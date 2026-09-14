/* Overlapping Message turns. Mutex not held during turn_fn. */
#include "l2src/lmx_message_exec.h"
#include "l2src/lmx_msg_slots.lm1.h"
#include "l2src/lmx_msg_mail_chain.lm1.h"
#include "l2src/lmx_msg_sched_ready.lm1.h"
#include "l2src/lmx_msg_visit.lm1.h"
#include "l2src/lmx_msg_liveness.lm1.h"
#include "l2src/lmx_msg_history_owned.lm1.h"
#include "l2src/lmx_msg_roots_stale.lm1.h"
#include "l2src/lmx_message_host.h"
#include "l2src/lmx.h"
#include <stdlib.h>
#if defined(LMX_MSG_EXEC_TEST)
#include <stdio.h>
#endif
#include <string.h>
#include <stdint.h>
#include <limits.h>
#include <setjmp.h>
#include <time.h>

/* Growable. Not product caps. */

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
typedef DWORD LmxTid;
#define lmx_tid() GetCurrentThreadId()
#else
#include <pthread.h>
typedef pthread_t LmxTid;
#define lmx_tid() pthread_self()
#endif

typedef struct LmxMsgBindWait {
#if defined(_WIN32)
    HANDLE wait_ev;
    HANDLE worker;
#else
    pthread_cond_t cv;
    int sig;
    pthread_t worker;
    int worker_on;
#endif
    unsigned gen;
    int retired;
    int slot;
    int launch_n;
    int reaping;
    int on_reap;
    LmxTid owner_tid;
    struct LmxMsgBindWait *reap_next;
    /* Stage 3a-2: the bind record this generation serves while attached;
     * 0 once detached. Read and written under the exec lock. */
    struct LmxMsgExecBind *rec;
} LmxMsgBindWait;

typedef struct LmxMsgExecBind {
    LmxMsgAddr addr;
    LmxMsgTurn turn;
    void *ctx;
    LmxMsg *msg;
    int affinity;
    LmxTid held_by;
    int held;
    int last_st;
    int launching;
    int gone;
    /* Heap-stable wait generation. */
    LmxMsgBindWait *wait;
    /* Stage 3a-2/3b-7d: 1 while this record is bound: set by lmx_msg_exec_bind,
     * cleared where it is unbound (unbind_slot_locked). Read only under the exec
     * lock. */
    int in_table;
    /* Stage 3b-8: the next record on the context list of
     * ready_owner_of(msg). Under the exec lock. */
    struct LmxMsgExecBind *ctx_next;
} LmxMsgExecBind;

#if defined(LMX_MSG_EXEC_TEST)
void (*lmx_msg_test_mail_locked)(LmxMsg *m);
void (*lmx_msg_test_after_outbox_xfer)(LmxMsgRuntime *rt, LmxMsg *src, LmxMsgCopy *outb);
void (*lmx_msg_test_after_recv_pin)(LmxMsgRuntime *rt, LmxMsg *m);
void (*lmx_msg_test_after_sched_snap)(LmxMsgRuntime *rt, LmxMsg *p);
void (*lmx_msg_test_after_drive_snap)(LmxMsgRuntime *rt, LmxMsg *p);
void (*lmx_msg_exec_test_after_cleanup)(LmxMsgAddr who, int live, int st);
void (*lmx_msg_exec_test_after_bind_add)(LmxMsgRuntime *rt);
void (*lmx_msg_exec_test_during_launch)(LmxMsgRuntime *rt, LmxMsgAddr addr, int after_create);
void (*lmx_msg_exec_test_during_reap_kept)(LmxMsgRuntime *rt) = 0;
/* Stage 3b-9: fires in lmx_msg_release_slot after unbind, inside the exec lock
 * that covers the tree change (child_unlink and the root-list removal). */
void (*lmx_msg_exec_test_during_release_tree)(LmxMsgRuntime *rt, LmxMsg *m) = 0;
void lmx_msg_test_release_tree(LmxMsgRuntime *rt, LmxMsg *m) {
    if (lmx_msg_exec_test_during_release_tree != 0) {
        lmx_msg_exec_test_during_release_tree(rt, m);
    }
}
/* Decision 18 (2026-09-14): a Message's scheduler cell is written only on
 * its owner's lane, the thread that holds the owner's turn. The oracle is
 * armed by LMX_LANE_CHECK=1 in the environment (run_port_message -LaneCheck)
 * and aborts at the first write whose owner is not the current-turn Message;
 * the host outside any turn acts with the parent's authority and passes. */
int lmx_msg_test_lane_check;
static LmxMsgBindWait *test_launch_cap;
static unsigned test_launch_cap_gen;
static unsigned test_wait_destroy_n;
static unsigned test_wait_destroy_last_gen;
#endif

typedef struct LmxMsgExec {
#if defined(_WIN32)
    CRITICAL_SECTION lock;
    HANDLE stop_ev;
    DWORD tls;
#else
    pthread_mutex_t lock;
    pthread_key_t tls;
#endif
    int nworkers;
    LmxMsgBindWait *reap_head;
    int stopping;
    int stopped;
    int no_retire;
    int contexts_live;
    LmxMsgRuntime *rt;
    /* Stage 3b: a parent's ANY ready set is its own (map_ready); ANY children are
     * woken through their own context. The one cross-parent walk is the UI lane:
     * the parents raised for UI, in raise order. */
    int scan;
    LmxMsg *ui_map_own_head;
    LmxMsg *ui_map_own_tail;
#if defined(LMX_MSG_EXEC_TEST)
    int test_take_owners;
#endif
    LmxMsg *retire_head;
    LmxMsg *retire_tail;
    unsigned wait_serial;
    LmxMsgAddr unbound_held;
#if defined(LMX_MSG_EXEC_TEST)
    int test_fail_ctx;
    int test_fail_adopt_block;
    int test_fail_start_kicks;
#endif
} LmxMsgExec;

static LmxMsgBindWait *bind_wait_new(LmxMsgExec *e);
static void bind_wait_signal(LmxMsgBindWait *w);
static void bind_wait_destroy(LmxMsgBindWait *w);
static void bind_reap_join_all(LmxMsgRuntime *rt);
static int launch_ctx_thread(LmxMsgRuntime *rt, LmxMsgAddr addr);
static int launch_same_gen(const LmxMsgExecBind *b, LmxMsgBindWait *cap, unsigned gen);
#if defined(_WIN32)
static DWORD WINAPI context_worker(void *arg);
#endif
static LmxMsgExecBind *bind_rec_locked(LmxMsg *m);
static LmxMsgExecBind *rec_at_addr_locked(LmxMsgExec *e, LmxMsgAddr addr);
static void ctx_link_locked(LmxMsgExecBind *rec, LmxMsg *owner);
static void ctx_unlink_locked(LmxMsgExecBind *rec);
static int bind_has_worker(const LmxMsgExecBind *b);
static LmxMsg *msg_at_addr(LmxMsgRuntime *rt, LmxMsgAddr addr);
static void bind_wait_launch_hold_locked(LmxMsgBindWait *w);
static void bind_wait_launch_release(LmxMsgRuntime *rt, LmxMsgBindWait *w);

typedef struct LmxTurnRoot {
    jmp_buf jmp;
    int ready;
    struct LmxTurnRoot *prev;
} LmxTurnRoot;

#if defined(_MSC_VER)
static __declspec(thread) LmxMsg *lmx_turn_msg;
static __declspec(thread) LmxTurnRoot *lmx_turn_root;
__declspec(thread) uint_fast8_t *lmx_turn_running;
#else
static __thread LmxMsg *lmx_turn_msg;
static __thread LmxTurnRoot *lmx_turn_root;
__thread uint_fast8_t *lmx_turn_running;
#endif

#if defined(LMX_MSG_EXEC_TEST)
/* Decision 18 oracle body; its declaration and switch sit with the other test hooks above. */
void lmx_msg_test_lane_write(LmxMsgRuntime *rt, LmxMsg *owner, const char *site) {
    LmxMsg *turn;
    if (lmx_msg_test_lane_check == 0 || rt == 0 || owner == 0) {
        return;
    }
    turn = lmx_turn_msg;
    if (turn == 0 || turn->owner_rt != rt || turn == owner) {
        return;
    }
    fprintf(stderr, "LANE WRITE FAIL site=%s owner=%u turn=%u: a cell written off its owner's lane (decision 18)\n",
        site, (unsigned)owner->addr, (unsigned)turn->addr);
    fflush(stderr);
    abort();
}
#endif

#if defined(LMX_MSG_HOST_TEST) || defined(LMX_MSG_EXEC_TEST)
int lmx_msg_test_copy_fail;
int lmx_msg_test_copy_should_fail(void) {
    if (lmx_msg_test_copy_fail > 0) {
        lmx_msg_test_copy_fail -= 1;
        return 1;
    }
    return 0;
}
void lmx_msg_test_set_copy_fail(int n) {
    lmx_msg_test_copy_fail = n;
}
int lmx_msg_test_root_alloc_fail;
int lmx_msg_test_root_alloc_should_fail(void) {
    if (lmx_msg_test_root_alloc_fail > 0) {
        lmx_msg_test_root_alloc_fail -= 1;
        return 1;
    }
    return 0;
}
void lmx_msg_test_set_root_alloc_fail(int n) {
    lmx_msg_test_root_alloc_fail = n;
}
void *lmx_msg_history_test_malloc(size_t n) {
    if (lmx_msg_test_root_alloc_should_fail() != 0) {
        return 0;
    }
    return malloc(n);
}
void lmx_msg_history_test_free(void *p) {
    free(p);
}
#endif

int lmx_msg_poll_abort(void) {
    LmxTurnRoot *r = lmx_turn_root;
    if (r != 0 && r->ready != 0) {
        r->ready = 0;
        longjmp(r->jmp, 1);
    }
    return 1;
}

static void turn_root_pop(LmxTurnRoot *self, LmxTurnRoot *saved) {
    if (self != 0) {
        self->ready = 0;
    }
    lmx_turn_root = saved;
}

static LmxMsgExec *exof(LmxMsgRuntime *rt) {
    if (rt == 0) {
        return 0;
    }
    return (LmxMsgExec *)rt->exec;
}

#if defined(LMX_MSG_EXEC_TEST)
int lmx_msg_test_fail_retain;
int lmx_msg_test_fail_post_dead;
#endif

int lmx_msg_test_post_dead_fail(void) {
#if defined(LMX_MSG_EXEC_TEST)
    if (lmx_msg_test_fail_post_dead > 0) {
        lmx_msg_test_fail_post_dead -= 1;
        if (lmx_msg_test_fail_post_dead == 0) {
            return 1;
        }
    }
#endif
    return 0;
}

int lmx_msg_endp_retain(LmxMsg *m) {
#if defined(LMX_MSG_EXEC_TEST)
    if (lmx_msg_test_fail_retain > 0) {
        lmx_msg_test_fail_retain -= 1;
        return 0;
    }
#endif
#if defined(_WIN32)
    LONG old;
    LONG neu;
    if (m == 0) {
        return 0;
    }
    for (;;) {
        old = (LONG)m->refs;
        if (old < 1 || old == 2147483647) {
            return 0;
        }
        neu = old + 1;
        if (InterlockedCompareExchange((LONG *)&m->refs, neu, old) == old) {
            return 1;
        }
    }
#else
    if (m == 0 || m->refs < 1 || m->refs == 2147483647) {
        return 0;
    }
    m->refs = m->refs + 1;
    return 1;
#endif
}

void lmx_msg_endp_release(LmxMsg *m) {
#if defined(_WIN32)
    LONG old;
    LONG neu;
    if (m == 0) {
        return;
    }
    for (;;) {
        old = (LONG)m->refs;
        if (old < 1) {
            return;
        }
        neu = old - 1;
        if (InterlockedCompareExchange((LONG *)&m->refs, neu, old) == old) {
            if (neu == 0 && m->state == LMX_MSG_STATE_RELEASED && m->owner_rt != 0) {
                lmx_msg_endp_try_retire(m->owner_rt, m);
            }
            return;
        }
    }
#else
    if (m == 0 || m->refs < 1) {
        return;
    }
    m->refs = m->refs - 1;
    if (m->refs == 0 && m->state == LMX_MSG_STATE_RELEASED && m->owner_rt != 0) {
        lmx_msg_endp_try_retire(m->owner_rt, m);
    }
#endif
}

uint_fast8_t lmx_msg_running_load(const LmxMsg *m) {
    if (m == 0) {
        return 0;
    }
    return __atomic_load_n(&m->running, __ATOMIC_RELAXED);
}

void lmx_msg_running_store(LmxMsg *m, uint_fast8_t v) {
    if (m == 0) {
        return;
    }
    __atomic_store_n(&m->running, v, __ATOMIC_RELAXED);
}

uint_fast8_t lmx_msg_success_load(const LmxMsg *m) {
    if (m == 0) {
        return 0;
    }
    return __atomic_load_n(&m->success, __ATOMIC_RELAXED);
}

void lmx_msg_success_store(LmxMsg *m, uint_fast8_t v) {
    if (m == 0) {
        return;
    }
    __atomic_store_n(&m->success, v, __ATOMIC_RELAXED);
}

int lmx_msg_emergency_cancel(LmxMsgRuntime *rt, LmxMsgAddr who) {
    LmxMsg *m;
    if (rt == 0 || who == 0U) {
        return LMX_MSG_INVALID;
    }
    lmx_msg_exec_lock(rt);
    m = msg_at_addr(rt, who);
    if (m == 0) {
        lmx_msg_exec_unlock(rt);
        return LMX_MSG_INVALID;
    }
    lmx_msg_running_store(m, 0);
    m->closing = 1;
    lmx_msg_exec_unlock(rt);
    lmx_msg_exec_ready(rt, who);
    return LMX_MSG_OK;
}

unsigned lmx_msg_now(LmxMsgRuntime *rt) {
    if (rt == 0) {
        return 0U;
    }
    if (rt->clock_test != 0) {
        return rt->clock;
    }
#if defined(_WIN32)
    return GetTickCount();
#else
    return rt->clock;
#endif
}

int lmx_msg_tab_grow(LmxMsgRuntime *rt) {
    (void)rt;
    return 0;
}

LmxMsg *lmx_msg_slot_new(void) {
    LmxMsg *m = (LmxMsg *)calloc(1U, sizeof(LmxMsg));
    if (m == 0) {
        return 0;
    }
    m->refs = 1;
    m->running = 1;
    m->success = 0;
    m->tracked = 1;
#if defined(_WIN32)
    m->mail = calloc(1U, sizeof(CRITICAL_SECTION));
    if (m->mail == 0) {
        free(m);
        return 0;
    }
    InitializeCriticalSection((CRITICAL_SECTION *)m->mail);
#else
    m->mail = calloc(1U, sizeof(pthread_mutex_t));
    if (m->mail == 0) {
        free(m);
        return 0;
    }
    if (pthread_mutex_init((pthread_mutex_t *)m->mail, 0) != 0) {
        free(m->mail);
        m->mail = 0;
        free(m);
        return 0;
    }
#endif
    return m;
}

LmxMsg *lmx_msg_turn_self(LmxMsgRuntime *rt) {
    if (rt == 0 || lmx_turn_msg == 0 || lmx_turn_msg->owner_rt != rt) {
        return 0;
    }
    return lmx_turn_msg;
}

void lmx_msg_mail_lock(LmxMsg *m) {
    if (m == 0 || m->mail == 0) {
        return;
    }
#if defined(_WIN32)
    EnterCriticalSection((CRITICAL_SECTION *)m->mail);
#else
    pthread_mutex_lock((pthread_mutex_t *)m->mail);
#endif
#if defined(LMX_MSG_EXEC_TEST)
    if (lmx_msg_test_mail_locked != 0) {
        lmx_msg_test_mail_locked(m);
    }
#endif
}

void lmx_msg_mail_unlock(LmxMsg *m) {
    if (m == 0 || m->mail == 0) {
        return;
    }
#if defined(_WIN32)
    LeaveCriticalSection((CRITICAL_SECTION *)m->mail);
#else
    pthread_mutex_unlock((pthread_mutex_t *)m->mail);
#endif
}

int lmx_msg_mail_inbox_empty(LmxMsg *m) {
    int empty;
    if (m == 0) {
        return 1;
    }
    lmx_msg_mail_lock(m);
    empty = lmx_msg_mail_chain_empty(m->inbox);
    lmx_msg_mail_unlock(m);
    return empty;
}

int lmx_msg_mail_inbox_n(LmxMsg *m) {
    int n;
    if (m == 0) {
        return 0;
    }
    lmx_msg_mail_lock(m);
    n = lmx_msg_mail_chain_n(m->inbox);
    lmx_msg_mail_unlock(m);
    return n;
}

void lmx_msg_mail_inbox_take(LmxMsg *m, LmxMsgCopy **out) {
    if (out == 0) {
        return;
    }
    if (m == 0) {
        *out = 0;
        return;
    }
    lmx_msg_mail_lock(m);
    lmx_msg_mail_chain_take(&m->inbox, &m->inbox_tail, out);
    lmx_msg_mail_unlock(m);
}

int lmx_msg_mail_outbox_empty(LmxMsg *m) {
    int empty;
    if (m == 0) {
        return 1;
    }
    lmx_msg_mail_lock(m);
    empty = lmx_msg_mail_chain_empty(m->outbox);
    lmx_msg_mail_unlock(m);
    return empty;
}

void lmx_msg_sched_enqueue_child(LmxMsg *parent, LmxMsg *child) {
    if (parent == 0 || child == 0) {
        return;
    }
    lmx_msg_mail_lock(parent);
    lmx_msg_sched_ready_enqueue(&parent->sched_ready, &parent->sched_ready_tail, child);
    lmx_msg_mail_unlock(parent);
}

LmxMsg *lmx_msg_sched_dequeue_child(LmxMsg *parent) {
    LmxMsg *child;
    if (parent == 0) {
        return 0;
    }
    lmx_msg_mail_lock(parent);
    child = lmx_msg_sched_ready_dequeue(&parent->sched_ready, &parent->sched_ready_tail);
    lmx_msg_mail_unlock(parent);
    return child;
}

void lmx_msg_set_graph(LmxMsg *m, struct Lmx *unit) {
    if (m != 0) {
        m->graph = unit;
    }
}

struct Lmx *lmx_msg_graph(LmxMsg *m) {
    if (m == 0) {
        return 0;
    }
    return m->graph;
}

int lmx_msg_bootstrap_eternal_admit(LmxMsg *owner, void *address) {
    LmxOwnedRange *source;
    LmxOwnedRange *entry;
    uintptr_t lo;
    if (owner == 0 || address == 0) {
        return LMX_MSG_INVALID;
    }
    if (lmx_owned_ranges_find(owner->eternal_ranges, address) != 0) {
        return LMX_MSG_OK;
    }
    source = lmx_owned_ranges_find(owner->ranges, address);
    if (source == 0 || source->stride == 0) {
        return LMX_MSG_INVALID;
    }
    lo = (uintptr_t)address;
    if (lo > UINTPTR_MAX - source->stride) {
        return LMX_MSG_INVALID;
    }
    entry = (LmxOwnedRange *)calloc(1U, sizeof(*entry));
    if (entry == 0) {
        return LMX_MSG_NOMEM;
    }
    entry->lo = address;
    entry->hi = (void *)(lo + source->stride);
    entry->stride = source->stride;
    entry->kind = source->kind;
    entry->type = source->type;
    if (lmx_owned_ranges_add(&owner->eternal_ranges, entry) != LMX_OWNED_RANGES_OK) {
        free(entry);
        return LMX_MSG_INVALID;
    }
    return LMX_MSG_OK;
}

LmxOwnedRange *lmx_msg_eternal_ranges(LmxMsg *owner) {
    return owner != 0 ? owner->eternal_ranges : 0;
}

int lmx_msg_bootstrap_method_admit(LmxMsg *owner, void *address) {
    LmxOwnedRange *source;
    LmxOwnedRange *entry;
    uintptr_t lo;
    if (owner == 0 || address == 0) {
        return LMX_MSG_INVALID;
    }
    if (lmx_owned_ranges_find(owner->method_ranges, address) != 0) {
        return LMX_MSG_OK;
    }
    source = lmx_owned_ranges_find(owner->ranges, address);
    if (source == 0 || source->kind != LMX_KIND_METHOD || source->stride == 0) {
        return LMX_MSG_INVALID;
    }
    lo = (uintptr_t)address;
    if (lo > UINTPTR_MAX - source->stride) {
        return LMX_MSG_INVALID;
    }
    entry = (LmxOwnedRange *)calloc(1U, sizeof(*entry));
    if (entry == 0) {
        return LMX_MSG_NOMEM;
    }
    entry->lo = address;
    entry->hi = (void *)(lo + source->stride);
    entry->stride = source->stride;
    entry->kind = source->kind;
    entry->type = source->type;
    if (lmx_owned_ranges_add(&owner->method_ranges, entry) != LMX_OWNED_RANGES_OK) {
        free(entry);
        return LMX_MSG_INVALID;
    }
    return LMX_MSG_OK;
}

LmxOwnedRange *lmx_msg_method_ranges(LmxMsg *owner) {
    return owner != 0 ? owner->method_ranges : 0;
}

static void eternal_ranges_free(LmxOwnedRange *head) {
    LmxOwnedRange *next;
    while (head != 0) {
        next = head->next;
        free(head);
        head = next;
    }
}

int lmx_msg_eternal_clone(LmxMsg *dest, LmxOwnedRange *source) {
    LmxOwnedRange *slow;
    LmxOwnedRange *fast;
    LmxOwnedRange *prepared = 0;
    LmxOwnedRange *entry;
    if (dest == 0 || dest->eternal_ranges != 0) {
        return LMX_MSG_INVALID;
    }
    slow = source;
    fast = source;
    while (fast != 0 && fast->next != 0) {
        slow = slow->next;
        fast = fast->next->next;
        if (slow == fast) {
            return LMX_MSG_INVALID;
        }
    }
    while (source != 0) {
        entry = (LmxOwnedRange *)calloc(1U, sizeof(*entry));
        if (entry == 0) {
            eternal_ranges_free(prepared);
            return LMX_MSG_NOMEM;
        }
        entry->lo = source->lo;
        entry->hi = source->hi;
        entry->stride = source->stride;
        entry->kind = source->kind;
        entry->type = source->type;
        if (lmx_owned_ranges_add(&prepared, entry) != LMX_OWNED_RANGES_OK) {
            free(entry);
            eternal_ranges_free(prepared);
            return LMX_MSG_INVALID;
        }
        source = source->next;
    }
    dest->eternal_ranges = prepared;
    return LMX_MSG_OK;
}

int lmx_msg_method_clone(LmxMsg *dest, LmxOwnedRange *source) {
    LmxOwnedRange *slow;
    LmxOwnedRange *fast;
    LmxOwnedRange *prepared = 0;
    LmxOwnedRange *entry;
    if (dest == 0 || dest->method_ranges != 0) {
        return LMX_MSG_INVALID;
    }
    slow = source;
    fast = source;
    while (fast != 0 && fast->next != 0) {
        slow = slow->next;
        fast = fast->next->next;
        if (slow == fast) {
            return LMX_MSG_INVALID;
        }
    }
    while (source != 0) {
        if (source->kind != LMX_KIND_METHOD) {
            eternal_ranges_free(prepared);
            return LMX_MSG_INVALID;
        }
        entry = (LmxOwnedRange *)calloc(1U, sizeof(*entry));
        if (entry == 0) {
            eternal_ranges_free(prepared);
            return LMX_MSG_NOMEM;
        }
        entry->lo = source->lo;
        entry->hi = source->hi;
        entry->stride = source->stride;
        entry->kind = source->kind;
        entry->type = source->type;
        if (lmx_owned_ranges_add(&prepared, entry) != LMX_OWNED_RANGES_OK) {
            free(entry);
            eternal_ranges_free(prepared);
            return LMX_MSG_INVALID;
        }
        source = source->next;
    }
    dest->method_ranges = prepared;
    return LMX_MSG_OK;
}

void lmx_msg_sched_unlink_child(LmxMsg *parent, LmxMsg *child) {
    if (parent == 0 || child == 0) {
        return;
    }
    lmx_msg_mail_lock(parent);
    lmx_msg_sched_ready_unlink(&parent->sched_ready, &parent->sched_ready_tail, child);
    lmx_msg_mail_unlock(parent);
}

int lmx_msg_sched_pick_host_child(LmxMsgRuntime *rt, LmxMsgAddr parent, unsigned *out_addr) {
    LmxMsg *p;
    LmxMsg **tab;
    LmxMsg *ch;
    size_t cap = 0;
    size_t n = 0;
    size_t i;
    int pin_p;
    unsigned addr = 0U;
    if (out_addr != 0) {
        *out_addr = 0U;
    }
    if (rt == 0 || parent == 0U || out_addr == 0) {
        return LMX_MSG_INVALID;
    }
    lmx_msg_exec_lock(rt);
    p = msg_at_addr(rt, parent);
    if (p == 0) {
        lmx_msg_exec_unlock(rt);
        return LMX_MSG_INVALID;
    }
    for (ch = p->first_child; ch != 0; ch = ch->next_sibling) {
        if (cap == SIZE_MAX / sizeof(*tab)) {
            lmx_msg_exec_unlock(rt);
            return LMX_MSG_NOMEM;
        }
        cap += 1;
    }
    tab = 0;
    if (cap > 0) {
        tab = (LmxMsg **)malloc((size_t)cap * sizeof(LmxMsg *));
        if (tab == 0) {
            lmx_msg_exec_unlock(rt);
            return LMX_MSG_NOMEM;
        }
    }
    pin_p = lmx_msg_endp_retain(p);
    if (pin_p == 0) {
        free(tab);
        lmx_msg_exec_unlock(rt);
        return LMX_MSG_NOMEM;
    }
    for (ch = p->first_child; ch != 0; ch = ch->next_sibling) {
        if (ch->mapped == 0 && ch->turn != 0
            && ch->state != LMX_MSG_STATE_STOPPED
            && ch->state != LMX_MSG_STATE_DEAD
            && ch->state != LMX_MSG_STATE_RELEASED) {
            if (lmx_msg_endp_retain(ch) == 0) {
                while (n > 0) {
                    n -= 1;
                    lmx_msg_endp_release(tab[n]);
                }
                if (pin_p != 0) {
                    lmx_msg_endp_release(p);
                }
                free(tab);
                lmx_msg_exec_unlock(rt);
                return LMX_MSG_NOMEM;
            }
            tab[n] = ch;
            n += 1;
        }
    }
    lmx_msg_exec_unlock(rt);
#if defined(LMX_MSG_EXEC_TEST)
    if (lmx_msg_test_after_sched_snap != 0) {
        lmx_msg_test_after_sched_snap(rt, p);
    }
#endif
    for (i = 0; i < n && addr == 0U; i++) {
        int ok;
        int closing;
        unsigned a;
        ch = tab[i];
        lmx_msg_exec_lock(rt);
        ok = (ch->parent_msg == p && ch->mapped == 0 && ch->turn != 0
            && ch->state != LMX_MSG_STATE_STOPPED
            && ch->state != LMX_MSG_STATE_DEAD
            && ch->state != LMX_MSG_STATE_RELEASED);
        closing = ch->closing;
        a = ch->addr;
        lmx_msg_exec_unlock(rt);
        if (ok != 0 && (lmx_msg_mail_inbox_empty(ch) == 0 || closing != 0)) {
            addr = a;
        }
    }
    for (i = 0; i < n; i++) {
        lmx_msg_endp_release(tab[i]);
    }
    if (pin_p != 0) {
        lmx_msg_endp_release(p);
    }
    free(tab);
    if (out_addr != 0) {
        *out_addr = addr;
    }
    return LMX_MSG_OK;
}

int lmx_msg_drive_tree(LmxMsgRuntime *rt, LmxMsg *m);

static int drive_walk_list(LmxMsgRuntime *rt, LmxMsg *parent, LmxMsg *head) {
    LmxMsg **tab = 0;
    LmxMsg *ch;
    size_t cap = 0;
    size_t n = 0;
    size_t i;
    int pin_p = 0;
    int st = LMX_MSG_OK;
    if (rt == 0) {
        return LMX_MSG_OK;
    }
    for (ch = head; ch != 0; ch = ch->next_sibling) {
        if (cap == SIZE_MAX / sizeof(*tab)) {
            return LMX_MSG_NOMEM;
        }
        cap += 1;
    }
    if (cap > 0) {
        tab = (LmxMsg **)malloc(cap * sizeof(*tab));
        if (tab == 0) {
            return LMX_MSG_NOMEM;
        }
    }
    if (parent != 0) {
        pin_p = lmx_msg_endp_retain(parent);
        if (pin_p == 0) {
            free(tab);
            return LMX_MSG_NOMEM;
        }
    }
    for (ch = head; ch != 0; ch = ch->next_sibling) {
        if (lmx_msg_endp_retain(ch) == 0) {
            while (n > 0) {
                n -= 1;
                lmx_msg_endp_release(tab[n]);
            }
            if (pin_p != 0) {
                lmx_msg_endp_release(parent);
            }
            free(tab);
            return LMX_MSG_NOMEM;
        }
        tab[n] = ch;
        n += 1;
    }
    lmx_msg_exec_unlock(rt);
#if defined(LMX_MSG_EXEC_TEST)
    if (lmx_msg_test_after_drive_snap != 0) {
        lmx_msg_test_after_drive_snap(rt, parent);
    }
#endif
    for (i = 0; i < n && st == LMX_MSG_OK; i++) {
        int ok = 0;
        lmx_msg_exec_lock(rt);
        if (parent != 0) {
            ok = (tab[i]->parent_msg == parent
                && tab[i]->state != LMX_MSG_STATE_RELEASED);
        } else {
            for (ch = rt->root; ch != 0; ch = ch->next_sibling) {
                if (ch == tab[i]) {
                    ok = (tab[i]->state != LMX_MSG_STATE_RELEASED);
                    break;
                }
            }
        }
        if (ok != 0) {
            st = lmx_msg_drive_tree(rt, tab[i]);
        }
        lmx_msg_exec_unlock(rt);
        lmx_msg_endp_release(tab[i]);
        tab[i] = 0;
    }
    while (i < n) {
        lmx_msg_endp_release(tab[i]);
        i += 1;
    }
    if (pin_p != 0) {
        lmx_msg_endp_release(parent);
    }
    free(tab);
    lmx_msg_exec_lock(rt);
    return st;
}

int lmx_msg_drive_walk_children(LmxMsgRuntime *rt, LmxMsg *m) {
    if (rt == 0 || m == 0) {
        return LMX_MSG_OK;
    }
    return drive_walk_list(rt, m, m->first_child);
}

int lmx_msg_drive_walk_roots(LmxMsgRuntime *rt) {
    if (rt == 0) {
        return LMX_MSG_OK;
    }
    return drive_walk_list(rt, 0, rt->root);
}

void lmx_msg_mail_inbox_prepend(LmxMsg *m, LmxMsgCopy *chain) {
    LmxMsgCopy *t;
    if (m == 0 || chain == 0) {
        return;
    }
    t = chain;
    while (t->next != 0) {
        t = t->next;
    }
    lmx_msg_mail_lock(m);
    t->next = m->inbox;
    if (m->inbox_tail == 0) {
        m->inbox_tail = t;
    }
    m->inbox = chain;
    lmx_msg_mail_unlock(m);
}

void lmx_msg_mail_outbox_take(LmxMsg *m, LmxMsgCopy **out) {
    if (out == 0) {
        return;
    }
    if (m == 0) {
        *out = 0;
        return;
    }
    lmx_msg_mail_lock(m);
    lmx_msg_mail_chain_take(&m->outbox, &m->outbox_tail, out);
    lmx_msg_mail_unlock(m);
}

#if defined(LMX_MSG_EXEC_TEST)
int lmx_msg_test_stage(LmxMsgRuntime *rt, LmxMsgAddr from, LmxMsgAddr to, unsigned id) {
    LmxMsg *src;
    LmxMsg *dest;
    LmxMsgCopy *node;
    if (rt == 0 || from == 0U || to == 0U || id == 0U) {
        return LMX_MSG_INVALID;
    }
    node = (LmxMsgCopy *)calloc(1U, sizeof(LmxMsgCopy));
    if (node == 0) {
        return LMX_MSG_NOMEM;
    }
    node->id = id;
    node->from = from;
    node->to = to;
    node->kind = LMX_MSG_KIND_BYTES;
    lmx_msg_exec_lock(rt);
    src = msg_at_addr(rt, from);
    dest = msg_at_addr(rt, to);
    if (src == 0 || dest == 0 || src->state == LMX_MSG_STATE_DEAD || src->state == LMX_MSG_STATE_STOPPED) {
        lmx_msg_exec_unlock(rt);
        free(node);
        return LMX_MSG_INVALID;
    }
    if (lmx_msg_endp_retain(dest) == 0) {
        lmx_msg_exec_unlock(rt);
        free(node);
        return LMX_MSG_NOMEM;
    }
    node->dest_msg = dest;
    if (lmx_msg_endp_retain(src) == 0) {
        lmx_msg_exec_unlock(rt);
        node->dest_msg = 0;
        lmx_msg_endp_release(dest);
        free(node);
        return LMX_MSG_NOMEM;
    }
    lmx_msg_exec_unlock(rt);
    lmx_msg_mail_lock(src);
    node->next = 0;
    if (src->outbox_tail != 0) {
        src->outbox_tail->next = node;
    } else {
        src->outbox = node;
    }
    src->outbox_tail = node;
    lmx_msg_mail_unlock(src);
    lmx_msg_endp_release(src);
    return LMX_MSG_STAGED;
}
#endif

void lmx_msg_after_recv_pin(LmxMsgRuntime *rt, LmxMsg *m) {
#if defined(LMX_MSG_EXEC_TEST)
    if (lmx_msg_test_after_recv_pin != 0) {
        lmx_msg_test_after_recv_pin(rt, m);
    }
#else
    (void)rt;
    (void)m;
#endif
}

void lmx_msg_after_outbox_xfer(LmxMsgRuntime *rt, LmxMsg *src, LmxMsgCopy *outb) {
#if defined(LMX_MSG_EXEC_TEST)
    if (lmx_msg_test_after_outbox_xfer != 0) {
        lmx_msg_test_after_outbox_xfer(rt, src, outb);
    }
#else
    (void)rt;
    (void)src;
    (void)outb;
#endif
}

static void drop_ranges_locked(LmxMsg *m) {
    if (m == 0) {
        return;
    }
    while (m->ranges != 0) {
        if (lmx_owned_ranges_remove(&m->ranges, m->ranges) != LMX_OWNED_RANGES_OK) {
            m->ranges = 0;
            return;
        }
    }
}

static int handoff_move_locked(LmxMsg *dst, LmxMsg *src) {
    int graph_moves;
    if (dst == 0 || src == 0 || dst == src) {
        return LMX_MSG_INVALID;
    }
    graph_moves = src->graph != 0
        && lmx_owned_ranges_find(src->ranges, src->graph) != 0;
    if (lmx_msg_storage_can_move(&dst->blocks, &dst->ranges, &src->blocks, &src->ranges)
        != LMX_MSG_STORAGE_OK) {
        return LMX_MSG_INVALID;
    }
    if (lmx_msg_storage_move_all(&dst->blocks, &dst->ranges, &src->blocks, &src->ranges)
        != LMX_MSG_STORAGE_OK) {
        return LMX_MSG_INVALID;
    }
    /* Roots are owner-local retention, not storage. Do not move them. Drop
     * source entries whose addresses no longer classify here so they cannot
     * retain transferred payloads. Dest must attach if it wants retention. */
    if (graph_moves != 0) {
        src->graph = 0;
    }
    lmx_msg_roots_drop_stale(src);
    return LMX_MSG_OK;
}

void lmx_msg_slot_free(LmxMsg *m) {
    LmxMsgRoot *r;
    LmxMsgRoot *rn;
    LmxOwnedRange *eternal;
    if (m == 0) {
        return;
    }
    r = m->roots;
    m->roots = 0;
    while (r != 0) {
        rn = r->next;
        free(r);
        r = rn;
    }
    eternal = m->eternal_ranges;
    m->eternal_ranges = 0;
    eternal_ranges_free(eternal);
    eternal = m->method_ranges;
    m->method_ranges = 0;
    eternal_ranges_free(eternal);
    free(m->done_from);
    free(m->done_id);
    m->done_from = 0;
    m->done_id = 0;
    m->done_n = 0;
    m->done_cap = 0;
    free(m->exec_bind);
    m->exec_bind = 0;
    drop_ranges_locked(m);
    (void)lmx_msg_blocks_dispose_all(&m->blocks);
    if (m->mail != 0) {
#if defined(_WIN32)
        DeleteCriticalSection((CRITICAL_SECTION *)m->mail);
#else
        pthread_mutex_destroy((pthread_mutex_t *)m->mail);
#endif
        free(m->mail);
        m->mail = 0;
    }
    free(m);
}

int lmx_msg_endp_try_retire(LmxMsgRuntime *rt, LmxMsg *m) {
    LmxMsgExec *e;
    LmxMsg *prev;
    LmxMsg *cur;
    if (rt == 0 || m == 0) {
        return 0;
    }
    e = exof(rt);
    if (e != 0 && e->no_retire != 0) {
        return 0;
    }
    lmx_msg_exec_lock(rt);
    if (e != 0 && e->no_retire != 0) {
        lmx_msg_exec_unlock(rt);
        return 0;
    }
    if (m->owner_rt != rt || m->refs != 0 || m->state != LMX_MSG_STATE_RELEASED) {
        lmx_msg_exec_unlock(rt);
        return 0;
    }
    if (lmx_msg_mail_inbox_empty(m) == 0 || lmx_msg_mail_outbox_empty(m) == 0 || m->parent_msg != 0
        || m->first_child != 0 || m->map_ready != 0 || m->ui_map_ready != 0
        || m->ui_map_own_queued != 0) {
        lmx_msg_exec_unlock(rt);
        return 0;
    }
    /* A root uses next_sibling as the runtime root-list link.  It must leave
     * that index before its slot/storage is freed, otherwise concurrent
     * msg_at_addr() walks a dangling tree and may loop through reused memory. */
    prev = 0;
    cur = rt->root;
    while (cur != 0) {
        if (cur == m) {
            if (prev != 0) {
                prev->next_sibling = m->next_sibling;
            } else {
                rt->root = m->next_sibling;
            }
            m->next_sibling = 0;
            break;
        }
        prev = cur;
        cur = cur->next_sibling;
    }
    prev = 0;
    cur = rt->slots;
    while (cur != 0) {
        if (cur == m) {
            if (prev != 0) {
                prev->alloc_next = m->alloc_next;
            } else {
                rt->slots = m->alloc_next;
            }
            m->alloc_next = 0;
            if (rt->n > 0) {
                rt->n -= 1;
            }
            break;
        }
        prev = cur;
        cur = cur->alloc_next;
    }
    if (m->path != 0) {
        free(m->path);
        m->path = 0;
    }
    if (m->init != 0) {
        free(m->init);
        m->init = 0;
    }
    lmx_msg_exec_unlock(rt);
    lmx_msg_slot_free(m);
    return 1;
}

int lmx_msg_exec_attach(LmxMsgRuntime *rt) {
    LmxMsgExec *e;
    if (rt == 0) {
        return 1;
    }
    e = (LmxMsgExec *)calloc(1U, sizeof(LmxMsgExec));
    if (e == 0) {
        return 1;
    }
    e->rt = rt;
#if defined(LMX_MSG_EXEC_TEST)
    lmx_msg_test_lane_check = getenv("LMX_LANE_CHECK") != 0;
#endif
#if defined(_WIN32)
    InitializeCriticalSection(&e->lock);
    e->stop_ev = CreateEventA(0, 1, 0, 0);
    e->tls = TlsAlloc();
    if (e->stop_ev == 0 || e->tls == TLS_OUT_OF_INDEXES) {
        if (e->stop_ev) CloseHandle(e->stop_ev);
        if (e->tls != TLS_OUT_OF_INDEXES) TlsFree(e->tls);
        DeleteCriticalSection(&e->lock);
        free(e);
        return 1;
    }
#else
    pthread_mutexattr_t a;
    pthread_mutexattr_init(&a);
    pthread_mutexattr_settype(&a, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&e->lock, &a);
    pthread_mutexattr_destroy(&a);
    pthread_key_create(&e->tls, 0);
#endif
    rt->exec = e;
    return 0;
}

void lmx_msg_exec_detach(LmxMsgRuntime *rt) {
    LmxMsgExec *e = exof(rt);
    if (e == 0) {
        return;
    }
    if (e->stopped == 0) {
        lmx_msg_exec_stop(rt);
    }
    {
        LmxMsgBindWait *w = e->reap_head;
        while (w != 0) {
            LmxMsgBindWait *nxt = w->reap_next;
            bind_wait_destroy(w);
            w = nxt;
        }
        e->reap_head = 0;
    }
#if defined(_WIN32)
    CloseHandle(e->stop_ev);
    TlsFree(e->tls);
    DeleteCriticalSection(&e->lock);
#else
    pthread_mutex_destroy(&e->lock);
    pthread_key_delete(e->tls);
#endif
    free(e);
    rt->exec = 0;
}

int lmx_msg_exec_lock(LmxMsgRuntime *rt) {
    LmxMsgExec *e = exof(rt);
    if (e == 0) {
        return 1;
    }
#if defined(_WIN32)
    EnterCriticalSection(&e->lock);
#else
    pthread_mutex_lock(&e->lock);
#endif
    return 0;
}

void lmx_msg_exec_unlock(LmxMsgRuntime *rt) {
    LmxMsgExec *e = exof(rt);
    if (e == 0) {
        return;
    }
#if defined(_WIN32)
    LeaveCriticalSection(&e->lock);
#else
    pthread_mutex_unlock(&e->lock);
#endif
}

int lmx_msg_exec_holding_any(LmxMsgRuntime *rt) {
    LmxMsgExec *e = exof(rt);
    LmxMsgAddr cur;
    if (e == 0) {
        return 0;
    }
#if defined(_WIN32)
    cur = (LmxMsgAddr)(uintptr_t)TlsGetValue(e->tls);
#else
    cur = (LmxMsgAddr)(uintptr_t)pthread_getspecific(e->tls);
#endif
    return cur != 0U;
}

int lmx_msg_exec_workers(LmxMsgRuntime *rt) {
    LmxMsgExec *e = exof(rt);
    if (e == 0) {
        return 0;
    }
    return e->nworkers;
}

int lmx_msg_exec_contexts_live(LmxMsgRuntime *rt) {
    LmxMsgExec *e = exof(rt);
    if (e == 0) {
        return 0;
    }
    return e->contexts_live;
}

int lmx_msg_exec_holding_turn(LmxMsgRuntime *rt, LmxMsgAddr who) {
    LmxMsgExec *e = exof(rt);
    LmxMsgAddr cur;
    if (e == 0 || who == 0U) {
        return 0;
    }
#if defined(_WIN32)
    cur = (LmxMsgAddr)(uintptr_t)TlsGetValue(e->tls);
#else
    cur = (LmxMsgAddr)(uintptr_t)pthread_getspecific(e->tls);
#endif
    return cur == who;
}

static LmxMsgAddr get_tls(LmxMsgExec *e) {
#if defined(_WIN32)
    return (LmxMsgAddr)(uintptr_t)TlsGetValue(e->tls);
#else
    return (LmxMsgAddr)(uintptr_t)pthread_getspecific(e->tls);
#endif
}

static void set_tls(LmxMsgExec *e, LmxMsgAddr who) {
#if defined(_WIN32)
    TlsSetValue(e->tls, (void *)(uintptr_t)who);
#else
    pthread_setspecific(e->tls, (void *)(uintptr_t)who);
#endif
    if (e != 0 && e->rt != 0 && who != 0U) {
        lmx_turn_msg = msg_at_addr(e->rt, who);
        lmx_turn_running = lmx_turn_msg != 0 ? &lmx_turn_msg->running : 0;
    } else {
        lmx_turn_msg = 0;
        lmx_turn_running = 0;
    }
}

void lmx_msg_exec_set_scan_locked(LmxMsgRuntime *rt, int v) {
    LmxMsgExec *e = exof(rt);
    if (e != 0) {
        e->scan = v;
    }
}

int lmx_msg_exec_get_scan_locked(LmxMsgRuntime *rt) {
    LmxMsgExec *e = exof(rt);
    if (e == 0) {
        return 0;
    }
    return e->scan;
}

/* Stage 3b-7b: the one temporary cross-parent walk. Visits every Message
 * reachable from rt->root whose context list is non-empty, in tree order
 * (iterative: first_child, else next_sibling, else climb parent_msg), and hands
 * the callback the owner and its list; the callback iterates the records. A
 * nonzero callback result stops the walk and is returned. Caller holds the exec
 * lock; a callback must not unlink records or drop the lock. 3c-2 swaps this
 * walk for the parents' records. */
typedef int (*LmxCtxOwnerVisit)(LmxMsgExec *e, LmxMsg *owner, LmxMsgExecBind *head, void *arg);

static int ctx_walk_locked(LmxMsgExec *e, LmxCtxOwnerVisit visit, void *arg) {
    LmxMsg *m;
    int st;
    if (e == 0 || e->rt == 0) {
        return 0;
    }
    m = e->rt->root;
    while (m != 0) {
        if (m->ctx_head != 0) {
            st = visit(e, m, m->ctx_head, arg);
            if (st != 0) {
                return st;
            }
        }
        if (m->first_child != 0) {
            m = m->first_child;
            continue;
        }
        while (m != 0 && m->next_sibling == 0) {
            m = m->parent_msg;
        }
        if (m != 0) {
            m = m->next_sibling;
        }
    }
    return 0;
}

static int ctx_visit_wake(LmxMsgExec *e, LmxMsg *owner, LmxMsgExecBind *rec, void *arg) {
    (void)e;
    (void)owner;
    (void)arg;
    for (; rec != 0; rec = rec->ctx_next) {
        if (rec->affinity != LMX_MSG_AFFINITY_UI) {
            bind_wait_signal(rec->wait);
        }
    }
    return 0;
}

/* Stage 3b-7d: the number of bound records, over the owners' lists. */
static int ctx_visit_count(LmxMsgExec *e, LmxMsg *owner, LmxMsgExecBind *rec, void *arg) {
    (void)e;
    (void)owner;
    for (; rec != 0; rec = rec->ctx_next) {
        *(int *)arg += 1;
    }
    return 0;
}

void lmx_msg_exec_wake_locked(LmxMsgRuntime *rt) {
    LmxMsgExec *e = exof(rt);
    if (e == 0) {
        return;
    }
    (void)ctx_walk_locked(e, ctx_visit_wake, 0);
}

void lmx_msg_exec_wake_addr_locked(LmxMsgRuntime *rt, LmxMsgAddr addr) {
    LmxMsgExec *e = exof(rt);
    LmxMsgExecBind *r;
    if (e == 0 || addr == 0U) {
        return;
    }
    r = rec_at_addr_locked(e, addr);
    if (r != 0 && r->affinity != LMX_MSG_AFFINITY_UI) {
        bind_wait_signal(r->wait);
    }
}

/* Stage 3b: raise a parent on the UI lane (its UI set became non-empty) and lower it
 * (its UI set emptied, or the lane take rotates it). ui_map_own_queued is the raised bit. */
static void ui_owner_raise(LmxMsgExec *e, LmxMsg *owner) {
    if (e == 0 || owner == 0 || owner->ui_map_own_queued != 0) {
        return;
    }
    owner->ui_map_own_queued = 1;
    owner->ui_map_own_next = 0;
    if (e->ui_map_own_tail != 0) {
        e->ui_map_own_tail->ui_map_own_next = owner;
    } else {
        e->ui_map_own_head = owner;
    }
    e->ui_map_own_tail = owner;
}

static void ui_owner_lower(LmxMsgExec *e, LmxMsg *owner) {
    LmxMsg *prev = 0;
    LmxMsg *item;
    if (e == 0 || owner == 0 || owner->ui_map_own_queued == 0) {
        return;
    }
    item = e->ui_map_own_head;
    while (item != 0) {
        if (item == owner) {
            if (prev == 0) {
                e->ui_map_own_head = item->ui_map_own_next;
            } else {
                prev->ui_map_own_next = item->ui_map_own_next;
            }
            if (e->ui_map_own_tail == owner) {
                e->ui_map_own_tail = prev;
            }
            break;
        }
        prev = item;
        item = item->ui_map_own_next;
    }
    owner->ui_map_own_next = 0;
    owner->ui_map_own_queued = 0;
}

static void map_ready_pend_retire(LmxMsgExec *e, LmxMsg *owner) {
    if (e == 0 || owner == 0 || owner->retire_queued != 0) {
        return;
    }
    if (owner->state != LMX_MSG_STATE_RELEASED || owner->refs != 0
        || owner->parent_msg != 0 || owner->first_child != 0
        || owner->map_ready != 0 || owner->ui_map_ready != 0
        || owner->ui_map_own_queued != 0) {
        return;
    }
    owner->retire_queued = 1;
    owner->retire_next = 0;
    if (e->retire_tail != 0) {
        e->retire_tail->retire_next = owner;
    } else {
        e->retire_head = owner;
    }
    e->retire_tail = owner;
}

/* Stage 3b: the owner of a child's ready set is its parent, or the Message
 * itself when it has no parent (a top-level Message schedules itself). The one
 * place this convention lives; stage 5 may make every Message a child of the
 * root Message. */
static LmxMsg *ready_owner_of(LmxMsg *child) {
    return child->parent_msg != 0 ? child->parent_msg : child;
}

static void map_ready_enqueue_kind(LmxMsg *child, int ui) {
    LmxMsg *owner;
    LmxMsgExec *e;
    if (child == 0) {
        return;
    }
    e = child->owner_rt != 0 ? exof(child->owner_rt) : 0;
    if (ui != 0) {
        if (child->ui_map_queued != 0) {
            return;
        }
        owner = ready_owner_of(child);
        lmx_msg_test_lane_write(child->owner_rt, owner, "map_ready_enqueue:ui");
        child->ui_map_queued = 1;
        child->ui_map_next = 0;
        if (owner->ui_map_ready_tail != 0) {
            owner->ui_map_ready_tail->ui_map_next = child;
        } else {
            owner->ui_map_ready = child;
        }
        owner->ui_map_ready_tail = child;
        ui_owner_raise(e, owner);
        return;
    }
    if (child->map_queued != 0) {
        return;
    }
    owner = ready_owner_of(child);
    lmx_msg_test_lane_write(child->owner_rt, owner, "map_ready_enqueue:any");
    child->map_queued = 1;
    child->map_next = 0;
    if (owner->map_ready_tail != 0) {
        owner->map_ready_tail->map_next = child;
    } else {
        owner->map_ready = child;
    }
    owner->map_ready_tail = child;
}

static void map_ready_unlink_kind(LmxMsgExec *e, LmxMsg *child, int ui) {
    LmxMsg *owner;
    LmxMsg *prev;
    LmxMsg *item;
    if (child == 0) {
        return;
    }
    if (ui != 0) {
        if (child->ui_map_queued == 0) {
            return;
        }
        owner = ready_owner_of(child);
        prev = 0;
        item = owner != 0 ? owner->ui_map_ready : 0;
        while (item != 0) {
            if (item == child) {
                if (prev == 0) {
                    owner->ui_map_ready = child->ui_map_next;
                } else {
                    prev->ui_map_next = child->ui_map_next;
                }
                if (owner->ui_map_ready_tail == child) {
                    owner->ui_map_ready_tail = prev;
                }
                break;
            }
            prev = item;
            item = item->ui_map_next;
        }
        child->ui_map_next = 0;
        child->ui_map_queued = 0;
        if (owner != 0 && owner->ui_map_ready == 0) {
            ui_owner_lower(e, owner);
        }
        map_ready_pend_retire(e, owner);
        return;
    }
    if (child->map_queued == 0) {
        return;
    }
    owner = ready_owner_of(child);
    prev = 0;
    item = owner != 0 ? owner->map_ready : 0;
    while (item != 0) {
        if (item == child) {
            if (prev == 0) {
                owner->map_ready = child->map_next;
            } else {
                prev->map_next = child->map_next;
            }
            if (owner->map_ready_tail == child) {
                owner->map_ready_tail = prev;
            }
            break;
        }
        prev = item;
        item = item->map_next;
    }
    child->map_next = 0;
    child->map_queued = 0;
    map_ready_pend_retire(e, owner);
}

static void map_ready_unlink_msg(LmxMsg *child) {
    LmxMsgExec *e;
    if (child == 0) {
        return;
    }
    e = child->owner_rt != 0 ? exof(child->owner_rt) : 0;
    map_ready_unlink_kind(e, child, 0);
    map_ready_unlink_kind(e, child, 1);
}

int lmx_msg_exec_map_try_enqueue_locked(LmxMsgRuntime *rt, LmxMsgAddr addr) {
    LmxMsg *m;
    if (rt == 0 || addr == 0U) {
        return 1;
    }
    m = msg_at_addr(rt, addr);
    if (m == 0) {
        return 1;
    }
    map_ready_enqueue_kind(m, 0);
    return 0;
}

int lmx_msg_exec_ui_map_try_enqueue_locked(LmxMsgRuntime *rt, LmxMsgAddr addr) {
    LmxMsg *m;
    if (rt == 0 || addr == 0U) {
        return 1;
    }
    m = msg_at_addr(rt, addr);
    if (m == 0) {
        return 1;
    }
    map_ready_enqueue_kind(m, 1);
    return 0;
}

void lmx_msg_exec_map_unlink_locked(LmxMsgRuntime *rt, LmxMsgAddr addr) {
    LmxMsg *m;
    if (rt == 0 || addr == 0U) {
        return;
    }
    m = msg_at_addr(rt, addr);
    if (m != 0) {
        map_ready_unlink_msg(m);
    }
}

void lmx_msg_map_ready_unlink(LmxMsg *child) {
    LmxMsgRuntime *rt;
    if (child == 0) {
        return;
    }
    rt = child->owner_rt;
    if (rt != 0) {
        lmx_msg_exec_lock(rt);
    }
    map_ready_unlink_msg(child);
    if (rt != 0) {
        lmx_msg_exec_unlock(rt);
    }
}

void lmx_msg_exec_flush_retire(LmxMsgRuntime *rt) {
    LmxMsgExec *e = exof(rt);
    LmxMsg *head;
    LmxMsg *m;
    LmxMsg *nxt;
    if (e == 0) {
        return;
    }
    lmx_msg_exec_lock(rt);
    head = e->retire_head;
    e->retire_head = 0;
    e->retire_tail = 0;
    lmx_msg_exec_unlock(rt);
    m = head;
    while (m != 0) {
        nxt = m->retire_next;
        m->retire_next = 0;
        m->retire_queued = 0;
        (void)lmx_msg_endp_try_retire(rt, m);
        m = nxt;
    }
}

/* Stage 3b-5/3b-7b: the catch-up enqueue when scan is set (moved from
 * lmx_message.lm1), over the owners' context lists. Both kinds are enqueued
 * as before; the ANY enqueue has no consumer beyond retire lifecycle. */
static int ctx_visit_lane_scan(LmxMsgExec *e, LmxMsg *owner, LmxMsgExecBind *rec, void *arg) {
    LmxMsgRuntime *rt = e->rt;
    LmxMsgAddr a;
    (void)owner;
    (void)arg;
    for (; rec != 0; rec = rec->ctx_next) {
        a = rec->addr;
        if (a == 0U || rec->held != 0) {
            continue;
        }
        if (rec->affinity == LMX_MSG_AFFINITY_UI) {
            if (lmx_msg_exec_is_runnable_locked(rt, a) != 0) {
                (void)lmx_msg_exec_ui_map_try_enqueue_locked(rt, a);
            }
            continue;
        }
        if (lmx_msg_exec_is_runnable_locked(rt, a) != 0) {
            (void)lmx_msg_exec_map_try_enqueue_locked(rt, a);
        }
    }
    return 0;
}

static void lane_scan_ready_locked(LmxMsgRuntime *rt) {
    LmxMsgExec *e = exof(rt);
    if (e == 0 || e->scan == 0) {
        return;
    }
    e->scan = 0;
    (void)ctx_walk_locked(e, ctx_visit_lane_scan, 0);
}

/* Stage 3b: the UI lane take, the one cross-parent walk. Parents in raise order: the head
 * parent is lowered and raised again at the tail while its UI set is non-empty. Children
 * FIFO within a parent: held or launching stay queued; gone, not UI or not runnable are
 * unlinked; the first eligible one is held and returned. */
unsigned lmx_msg_exec_take_ui_map_locked(LmxMsgRuntime *rt) {
    LmxMsgExec *e = exof(rt);
    LmxMsgExecBind *rj;
    int empty;
    LmxMsg *owner;
    LmxMsg *guard;
    LmxMsg *child;
    LmxMsg *nxt;
    LmxMsgAddr took;
    if (e == 0) {
        return 0U;
    }
    lane_scan_ready_locked(rt);
    owner = e->ui_map_own_head;
    guard = owner;
    while (owner != 0) {
#if defined(LMX_MSG_EXEC_TEST)
        e->test_take_owners += 1;
#endif
        took = 0U;
        child = owner->ui_map_ready;
        while (child != 0) {
            nxt = child->ui_map_next;
            rj = bind_rec_locked(child);
            if (rj == 0 || rj->gone != 0 || rj->affinity != LMX_MSG_AFFINITY_UI) {
                map_ready_unlink_kind(e, child, 1);
                child = nxt;
                continue;
            }
            if (rj->held != 0 || rj->launching != 0) {
                child = nxt;
                continue;
            }
            if (lmx_msg_exec_is_runnable_locked(rt, child->addr) == 0) {
                map_ready_unlink_kind(e, child, 1);
                child = nxt;
                continue;
            }
            rj->held = 1;
            rj->held_by = lmx_tid();
            map_ready_unlink_kind(e, child, 1);
            took = child->addr;
            break;
        }
        empty = owner->ui_map_ready == 0;
        ui_owner_lower(e, owner);
        if (empty == 0) {
            ui_owner_raise(e, owner);
        }
        if (took != 0U) {
            return took;
        }
        owner = e->ui_map_own_head;
        if (owner == 0 || owner == guard) {
            return 0U;
        }
    }
    return 0U;
}



/* D1 allocation enumeration. Not the scheduler. Delegates to
 * Codex lmx_msg_slots; the lane catch-up must not use these. */
int lmx_msg_exec_tab_n_locked(LmxMsgRuntime *rt) {
    return lmx_msg_slots_n(rt);
}

LmxMsgAddr lmx_msg_exec_tab_addr_locked(LmxMsgRuntime *rt, int i) {
    return lmx_msg_slots_at(rt, i);
}

#if defined(LMX_MSG_EXEC_TEST)
void lmx_msg_exec_test_set_fail_start_kicks(LmxMsgRuntime *rt, int v) {
    LmxMsgExec *e = exof(rt);
    if (e == 0) {
        return;
    }
    lmx_msg_exec_lock(rt);
    e->test_fail_start_kicks = v;
    lmx_msg_exec_unlock(rt);
}

void lmx_msg_exec_test_set_fail_ctx(LmxMsgRuntime *rt, int v) {
    LmxMsgExec *e = exof(rt);
    if (e == 0) {
        return;
    }
    lmx_msg_exec_lock(rt);
    e->test_fail_ctx = v;
    lmx_msg_exec_unlock(rt);
}

void lmx_msg_exec_test_set_fail_adopt_block(LmxMsgRuntime *rt, int v) {
    LmxMsgExec *e = exof(rt);
    if (e == 0) {
        return;
    }
    lmx_msg_exec_lock(rt);
    e->test_fail_adopt_block = v;
    lmx_msg_exec_unlock(rt);
}



int lmx_msg_exec_map_queued(LmxMsgRuntime *rt, LmxMsgAddr addr) {
    LmxMsg *m;
    int q = 0;
    if (rt == 0 || addr == 0U) {
        return 0;
    }
    lmx_msg_exec_lock(rt);
    m = msg_at_addr(rt, addr);
    if (m != 0) {
        q = m->map_queued;
    }
    lmx_msg_exec_unlock(rt);
    return q;
}

int lmx_msg_exec_ui_map_queued(LmxMsgRuntime *rt, LmxMsgAddr addr) {
    LmxMsg *m;
    int q = 0;
    if (rt == 0 || addr == 0U) {
        return 0;
    }
    lmx_msg_exec_lock(rt);
    m = msg_at_addr(rt, addr);
    if (m != 0) {
        q = m->ui_map_queued;
    }
    lmx_msg_exec_unlock(rt);
    return q;
}

int lmx_msg_exec_retire_n(LmxMsgRuntime *rt) {
    LmxMsgExec *e = exof(rt);
    int n = 0;
    LmxMsg *m;
    if (e == 0) {
        return 0;
    }
    lmx_msg_exec_lock(rt);
    m = e->retire_head;
    while (m != 0) {
        n += 1;
        m = m->retire_next;
    }
    lmx_msg_exec_unlock(rt);
    return n;
}

int lmx_msg_exec_ui_map_nready(LmxMsgRuntime *rt) {
    LmxMsgExec *e = exof(rt);
    int n = 0;
    LmxMsg *owner;
    LmxMsg *child;
    if (e == 0) {
        return 0;
    }
    lmx_msg_exec_lock(rt);
    owner = e->ui_map_own_head;
    while (owner != 0) {
        child = owner->ui_map_ready;
        while (child != 0) {
            n += 1;
            child = child->ui_map_next;
        }
        owner = owner->ui_map_own_next;
    }
    lmx_msg_exec_unlock(rt);
    return n;
}

int lmx_msg_exec_get_scan(LmxMsgRuntime *rt) {
    LmxMsgExec *e = exof(rt);
    int s = 0;
    if (e == 0) {
        return 0;
    }
    lmx_msg_exec_lock(rt);
    s = e->scan;
    lmx_msg_exec_unlock(rt);
    return s;
}



int lmx_msg_exec_bind_n(LmxMsgRuntime *rt) {
    LmxMsgExec *e = exof(rt);
    int n = 0;
    if (e == 0) {
        return 0;
    }
    lmx_msg_exec_lock(rt);
    (void)ctx_walk_locked(e, ctx_visit_count, &n);
    lmx_msg_exec_unlock(rt);
    return n;
}

int lmx_msg_exec_bind_aff(LmxMsgRuntime *rt, LmxMsgAddr addr) {
    LmxMsgExec *e = exof(rt);
    LmxMsgExecBind *r;
    int aff = -1;
    if (e == 0 || addr == 0U) {
        return -1;
    }
    lmx_msg_exec_lock(rt);
    r = rec_at_addr_locked(e, addr);
    if (r != 0) {
        aff = r->affinity;
    }
    lmx_msg_exec_unlock(rt);
    return aff;
}

int lmx_msg_exec_bind_has_worker(LmxMsgRuntime *rt, LmxMsgAddr addr) {
    LmxMsgExec *e = exof(rt);
    LmxMsgExecBind *r;
    int h = 0;
    if (e == 0 || addr == 0U) {
        return 0;
    }
    lmx_msg_exec_lock(rt);
    r = rec_at_addr_locked(e, addr);
    if (r != 0) {
        h = bind_has_worker(r);
    }
    lmx_msg_exec_unlock(rt);
    return h;
}

unsigned lmx_msg_exec_test_wait_gen(LmxMsgRuntime *rt, LmxMsgAddr addr) {
    LmxMsgExec *e = exof(rt);
    LmxMsgExecBind *r;
    unsigned g = 0U;
    if (e == 0 || addr == 0U) {
        return 0U;
    }
    lmx_msg_exec_lock(rt);
    r = rec_at_addr_locked(e, addr);
    if (r != 0 && r->wait != 0) {
        g = r->wait->gen;
    }
    lmx_msg_exec_unlock(rt);
    return g;
}

void *lmx_msg_exec_test_worker_handle(LmxMsgRuntime *rt, LmxMsgAddr addr) {
    LmxMsgExec *e = exof(rt);
    LmxMsgExecBind *r;
    void *h = 0;
    if (e == 0 || addr == 0U) {
        return 0;
    }
    lmx_msg_exec_lock(rt);
    r = rec_at_addr_locked(e, addr);
    if (r != 0 && r->wait != 0) {
#if defined(_WIN32)
        h = (void *)r->wait->worker;
#else
        h = r->wait->worker_on != 0 ? (void *)(uintptr_t)1 : 0;
#endif
    }
    lmx_msg_exec_unlock(rt);
    return h;
}

int lmx_msg_exec_test_launching(LmxMsgRuntime *rt, LmxMsgAddr addr) {
    LmxMsgExec *e = exof(rt);
    LmxMsgExecBind *r;
    int v = 0;
    if (e == 0 || addr == 0U) {
        return 0;
    }
    lmx_msg_exec_lock(rt);
    r = rec_at_addr_locked(e, addr);
    if (r != 0) {
        v = r->launching;
    }
    lmx_msg_exec_unlock(rt);
    return v;
}

void *lmx_msg_exec_test_launch_cap(void) {
    return test_launch_cap;
}

unsigned lmx_msg_exec_test_launch_cap_gen(void) {
    return test_launch_cap_gen;
}

unsigned lmx_msg_exec_test_wait_gen_raw(const void *cap) {
    const LmxMsgBindWait *w = (const LmxMsgBindWait *)cap;
    return w != 0 ? w->gen : 0U;
}

int lmx_msg_exec_test_wait_launch_n(const void *cap) {
    const LmxMsgBindWait *w = (const LmxMsgBindWait *)cap;
    return w != 0 ? w->launch_n : -1;
}

unsigned lmx_msg_exec_test_wait_destroy_n(void) {
    return test_wait_destroy_n;
}

unsigned lmx_msg_exec_test_wait_destroy_last_gen(void) {
    return test_wait_destroy_last_gen;
}

int lmx_msg_exec_test_take_owners(LmxMsgRuntime *rt) {
    LmxMsgExec *e = exof(rt);
    int n = 0;
    if (e == 0) {
        return 0;
    }
    lmx_msg_exec_lock(rt);
    n = e->test_take_owners;
    lmx_msg_exec_unlock(rt);
    return n;
}

void lmx_msg_exec_test_take_owners_reset(LmxMsgRuntime *rt) {
    LmxMsgExec *e = exof(rt);
    if (e == 0) {
        return;
    }
    lmx_msg_exec_lock(rt);
    e->test_take_owners = 0;
    lmx_msg_exec_unlock(rt);
}
#endif

/* Stage 3a-2: the Message's own record while it is a counted table entry,
 * else 0. Caller holds the exec lock. */
/* Stage 3b-7a: a bound record joins the context list of its owner (FIFO). */
static void ctx_link_locked(LmxMsgExecBind *rec, LmxMsg *owner) {
    if (rec == 0 || owner == 0) {
        return;
    }
    rec->ctx_next = 0;
    if (owner->ctx_tail != 0) {
        owner->ctx_tail->ctx_next = rec;
    } else {
        owner->ctx_head = rec;
    }
    owner->ctx_tail = rec;
}

static void ctx_unlink_locked(LmxMsgExecBind *rec) {
    LmxMsg *owner;
    LmxMsgExecBind *prev = 0;
    LmxMsgExecBind *item;
    if (rec == 0 || rec->msg == 0) {
        return;
    }
    owner = ready_owner_of(rec->msg);
    item = owner->ctx_head;
    while (item != 0) {
        if (item == rec) {
            if (prev == 0) {
                owner->ctx_head = rec->ctx_next;
            } else {
                prev->ctx_next = rec->ctx_next;
            }
            if (owner->ctx_tail == rec) {
                owner->ctx_tail = prev;
            }
            break;
        }
        prev = item;
        item = item->ctx_next;
    }
}

static LmxMsgExecBind *bind_rec_locked(LmxMsg *m) {
    if (m == 0 || m->exec_bind == 0 || m->exec_bind->in_table == 0) {
        return 0;
    }
    return m->exec_bind;
}

/* Stage 3b-7c: the Message at addr, RELEASED Messages included (lmx_msg_find
 * skips them; a record lookup must not). Every bound Message is in its family
 * tree (3b-8) and the tree changes under the exec lock (3b-9). Caller holds the
 * exec lock. */
static LmxMsg *msg_find_any_locked(LmxMsgRuntime *rt, LmxMsgAddr addr) {
    LmxMsg *m;
    if (rt == 0 || addr == 0U) {
        return 0;
    }
    m = lmx_msg_turn_self(rt);
    if (m != 0 && m->addr == addr) {
        return m;
    }
    m = rt->root;
    while (m != 0) {
        if (m->addr == addr) {
            return m;
        }
        if (m->first_child != 0) {
            m = m->first_child;
            continue;
        }
        while (m != 0 && m->next_sibling == 0) {
            m = m->parent_msg;
        }
        if (m != 0) {
            m = m->next_sibling;
        }
    }
    return 0;
}

/* Stage 3b-7c: the record bound at addr, the Message's own. */
static LmxMsgExecBind *rec_at_addr_locked(LmxMsgExec *e, LmxMsgAddr addr) {
    LmxMsgExecBind *r;
    if (e == 0 || addr == 0U) {
        return 0;
    }
    r = bind_rec_locked(msg_find_any_locked(e->rt, addr));
    return r;
}

/* Stage 3b-8: whether m's record is in the table; lmx_msg_child_unlink refuses
 * a bound child (the family boundary contract). */
int lmx_msg_exec_msg_bound(LmxMsg *m) {
    LmxMsgRuntime *rt;
    int bound;
    if (m == 0) {
        return 0;
    }
    rt = m->owner_rt;
    if (rt != 0) {
        lmx_msg_exec_lock(rt);
    }
    bound = bind_rec_locked(m) != 0;
    if (rt != 0) {
        lmx_msg_exec_unlock(rt);
    }
    return bound;
}

/* Decision 17 rule 4: the executor side of a supervision handoff, the seam
 * 3c-2 replaces with the parents' records. Caller holds the exec lock. detach
 * runs while c->parent_msg is still the old parent (the ready sets, the context
 * list and the child scheduler list are found through it) and returns c's
 * memberships (1 ANY ready, 2 UI ready, 4 child scheduler); attach runs after
 * the family move and restores them on the new parent. */
int lmx_msg_exec_supervision_detach_locked(LmxMsg *c) {
    LmxMsgExecBind *rec;
    LmxMsg *p;
    int kept;
    if (c == 0 || c->parent_msg == 0) {
        return 0;
    }
    p = c->parent_msg;
    kept = (c->map_queued != 0 ? 1 : 0) | (c->ui_map_queued != 0 ? 2 : 0)
        | (c->sched_queued != 0 ? 4 : 0);
    map_ready_unlink_msg(c);
    rec = bind_rec_locked(c);
    if (rec != 0) {
        ctx_unlink_locked(rec);
    }
    lmx_msg_sched_unlink_child(p, c);
    return kept;
}

void lmx_msg_exec_supervision_attach_locked(LmxMsg *c, LmxMsg *old_parent, int kept) {
    LmxMsgExecBind *rec;
    LmxMsgExec *e;
    if (c == 0 || c->parent_msg == 0) {
        return;
    }
    rec = bind_rec_locked(c);
    if (rec != 0) {
        ctx_link_locked(rec, ready_owner_of(c));
    }
    if ((kept & 1) != 0) {
        map_ready_enqueue_kind(c, 0);
    }
    if ((kept & 2) != 0) {
        map_ready_enqueue_kind(c, 1);
    }
    if ((kept & 4) != 0) {
        lmx_msg_sched_enqueue_child(c->parent_msg, c);
    }
    e = c->owner_rt != 0 ? exof(c->owner_rt) : 0;
    map_ready_pend_retire(e, old_parent);
}

/* Decision 17 (spec 19.29.6 (iii)): c is re-rooted at the runtime as an
 * orphan. Its record joins its own context list (ready_owner_of(c) is c now)
 * and a kept ANY or UI readiness is re-raised into its own sets. A kept place
 * in the old parent's scheduler ready set has no root counterpart: the result
 * asks the caller for an exec_ready after the unlock. Decision 18: these
 * re-raises from the releaser's lane are class A until readiness is the
 * orphan's own flag. */
int lmx_msg_exec_orphan_attach_locked(LmxMsg *c, LmxMsg *old_parent, int kept) {
    LmxMsgExecBind *rec;
    LmxMsgExec *e;
    if (c == 0) {
        return 0;
    }
    rec = bind_rec_locked(c);
    if (rec != 0) {
        ctx_link_locked(rec, c);
    }
    if ((kept & 1) != 0) {
        map_ready_enqueue_kind(c, 0);
    }
    if ((kept & 2) != 0) {
        map_ready_enqueue_kind(c, 1);
    }
    e = c->owner_rt != 0 ? exof(c->owner_rt) : 0;
    map_ready_pend_retire(e, old_parent);
    return (kept & 4) != 0;
}

#if defined(LMX_MSG_EXEC_TEST)
/* Decision 17 rule 4 oracle: the Message whose context list (which 0), ANY
 * ready set (1) or UI ready set (2) holds addr's Message, and how many entries
 * hold it, over every Message reachable from rt->root. */
LmxMsgAddr lmx_msg_exec_test_list_owner(LmxMsgRuntime *rt, LmxMsgAddr addr, int which, int *count) {
    LmxMsg *m;
    LmxMsg *target;
    LmxMsgAddr owner = 0U;
    int n = 0;
    if (count != 0) {
        *count = 0;
    }
    if (rt == 0 || addr == 0U) {
        return 0U;
    }
    lmx_msg_exec_lock(rt);
    target = msg_find_any_locked(rt, addr);
    m = target != 0 ? rt->root : 0;
    while (m != 0) {
        int hit = 0;
        if (which == 0) {
            LmxMsgExecBind *r;
            for (r = m->ctx_head; r != 0; r = r->ctx_next) {
                if (r->msg == target) {
                    hit += 1;
                }
            }
        } else {
            LmxMsg *x = which == 1 ? m->map_ready : m->ui_map_ready;
            while (x != 0) {
                if (x == target) {
                    hit += 1;
                }
                x = which == 1 ? x->map_next : x->ui_map_next;
            }
        }
        if (hit != 0) {
            owner = m->addr;
            n += hit;
        }
        if (m->first_child != 0) {
            m = m->first_child;
            continue;
        }
        while (m != 0 && m->next_sibling == 0) {
            m = m->parent_msg;
        }
        if (m != 0) {
            m = m->next_sibling;
        }
    }
    lmx_msg_exec_unlock(rt);
    if (count != 0) {
        *count = n;
    }
    return owner;
}
#endif

/* Stage 3b-5: the routing read of a ready request, through the Message's own
 * record (the seam 3c-2 swaps for the parent's record). Returns 1 when m is
 * bound; *ui reports UI affinity and *pool a live worker. The caller resolves m
 * under the exec lock; a Message the live tree no longer finds is not routed. */
int lmx_msg_exec_route_locked(LmxMsg *m, int *ui, int *pool) {
    LmxMsgExecBind *rec = bind_rec_locked(m);
    if (ui != 0) {
        *ui = 0;
    }
    if (pool != 0) {
        *pool = 0;
    }
    if (rec == 0) {
        return 0;
    }
    if (ui != 0 && rec->affinity == LMX_MSG_AFFINITY_UI) {
        *ui = 1;
    }
    if (pool != 0 && bind_has_worker(rec) != 0) {
        *pool = 1;
    }
    return 1;
}

static int bind_has_worker(const LmxMsgExecBind *b) {
    if (b == 0 || b->wait == 0 || b->wait->retired != 0) {
        return 0;
    }
#if defined(_WIN32)
    return b->wait->worker != 0;
#else
    return b->wait->worker_on != 0;
#endif
}

static LmxMsgBindWait *bind_wait_new(LmxMsgExec *e) {
    LmxMsgBindWait *w = (LmxMsgBindWait *)calloc(1U, sizeof(LmxMsgBindWait));
    if (w == 0 || e == 0) {
        free(w);
        return 0;
    }
#if defined(_WIN32)
    w->wait_ev = CreateEventA(0, 0, 0, 0);
    if (w->wait_ev == 0) {
        free(w);
        return 0;
    }
#else
    if (pthread_cond_init(&w->cv, 0) != 0) {
        free(w);
        return 0;
    }
#endif
    e->wait_serial += 1U;
    if (e->wait_serial == 0U) {
        e->wait_serial = 1U;
    }
    w->gen = e->wait_serial;
    w->slot = 1;
    return w;
}

static void bind_wait_signal(LmxMsgBindWait *w) {
    if (w == 0) {
        return;
    }
#if defined(_WIN32)
    if (w->wait_ev != 0) {
        SetEvent(w->wait_ev);
    }
#else
    w->sig = 1;
    pthread_cond_signal(&w->cv);
#endif
}

static void bind_wait_destroy(LmxMsgBindWait *w) {
    if (w == 0) {
        return;
    }
#if defined(LMX_MSG_EXEC_TEST)
    test_wait_destroy_n += 1U;
    test_wait_destroy_last_gen = w->gen;
#endif
#if defined(_WIN32)
    if (w->wait_ev != 0) {
        CloseHandle(w->wait_ev);
        w->wait_ev = 0;
    }
#else
    pthread_cond_destroy(&w->cv);
#endif
    free(w);
}

static int wait_has_native(const LmxMsgBindWait *w) {
    if (w == 0) {
        return 0;
    }
#if defined(_WIN32)
    return w->worker != 0;
#else
    return w->worker_on != 0;
#endif
}

static int bind_wait_can_free(const LmxMsgBindWait *w) {
    return w != 0 && w->slot == 0 && w->launch_n == 0 && w->reaping == 0 && wait_has_native(w) == 0;
}

static void bind_wait_reap_unlink_locked(LmxMsgExec *e, LmxMsgBindWait *w) {
    LmxMsgBindWait **pp;
    if (e == 0 || w == 0) {
        return;
    }
    pp = &e->reap_head;
    while (*pp != 0) {
        if (*pp == w) {
            *pp = w->reap_next;
            w->reap_next = 0;
            w->on_reap = 0;
            return;
        }
        pp = &(*pp)->reap_next;
    }
}

static void bind_wait_maybe_free_locked(LmxMsgExec *e, LmxMsgBindWait *w) {
    if (bind_wait_can_free(w) == 0) {
        return;
    }
    bind_wait_reap_unlink_locked(e, w);
    bind_wait_destroy(w);
}

static void bind_wait_launch_hold_locked(LmxMsgBindWait *w) {
    if (w != 0) {
        w->launch_n += 1;
    }
}

static void bind_wait_launch_release(LmxMsgRuntime *rt, LmxMsgBindWait *w) {
    LmxMsgExec *e = exof(rt);
    if (e == 0 || w == 0) {
        return;
    }
    lmx_msg_exec_lock(rt);
    if (w->launch_n > 0) {
        w->launch_n -= 1;
    }
    bind_wait_maybe_free_locked(e, w);
    lmx_msg_exec_unlock(rt);
}

static int bind_wait_is_self(const LmxMsgBindWait *w) {
    if (w == 0) {
        return 0;
    }
#if defined(_WIN32)
    return w->owner_tid != 0 && w->owner_tid == GetCurrentThreadId();
#else
    return w->worker_on != 0 && pthread_equal(w->worker, pthread_self());
#endif
}

static int bind_wait_join(LmxMsgBindWait *w) {
    int had = 0;
    if (w == 0) {
        return 0;
    }
#if defined(_WIN32)
    if (w->worker != 0) {
        had = 1;
        WaitForSingleObject(w->worker, INFINITE);
        CloseHandle(w->worker);
        w->worker = 0;
    }
#else
    if (w->worker_on != 0) {
        had = 1;
        pthread_join(w->worker, 0);
        w->worker_on = 0;
    }
#endif
    return had;
}

static void bind_reap_push(LmxMsgExec *e, LmxMsgBindWait *w) {
    if (e == 0 || w == 0) {
        return;
    }
    w->retired = 1;
    w->slot = 0;
    if (w->on_reap == 0 && w->reaping == 0) {
        w->reap_next = e->reap_head;
        e->reap_head = w;
        w->on_reap = 1;
    }
    bind_wait_signal(w);
}

static void bind_reap_join_all(LmxMsgRuntime *rt) {
    LmxMsgExec *e = exof(rt);
    LmxMsgBindWait *head;
    LmxMsgBindWait *w;
    LmxMsgBindWait *nxt;
    int n;
    if (e == 0) {
        return;
    }
    lmx_msg_exec_lock(rt);
    head = e->reap_head;
    e->reap_head = 0;
    w = head;
    while (w != 0) {
        nxt = w->reap_next;
        w->on_reap = 0;
        w->reaping = 1;
        if (nxt == head) {
            w->reap_next = 0;
            nxt = 0;
        }
        w = nxt;
    }
    lmx_msg_exec_unlock(rt);
    w = head;
    n = 0;
    while (w != 0) {
        nxt = w->reap_next;
        w->reap_next = 0;
        if (bind_wait_is_self(w) != 0) {
            lmx_msg_exec_lock(rt);
            w->reaping = 1;
            w->on_reap = 1;
            w->reap_next = e->reap_head;
            e->reap_head = w;
            lmx_msg_exec_unlock(rt);
        } else {
            if (bind_wait_join(w) != 0) {
                n += 1;
            }
            lmx_msg_exec_lock(rt);
            w->reaping = 1;
            w->on_reap = 0;
#if defined(LMX_MSG_EXEC_TEST)
            if (lmx_msg_exec_test_during_reap_kept != 0
                && (w->launch_n > 0 || w->slot != 0)) {
                lmx_msg_exec_unlock(rt);
                lmx_msg_exec_test_during_reap_kept(rt);
                lmx_msg_exec_lock(rt);
            }
#endif
            w->reaping = 0;
            if (bind_wait_can_free(w) != 0) {
                bind_wait_destroy(w);
            } else {
                w->on_reap = 1;
                w->reap_next = e->reap_head;
                e->reap_head = w;
            }
            lmx_msg_exec_unlock(rt);
        }
        w = nxt;
    }
    lmx_msg_exec_lock(rt);
    if (n > 0) {
        if (e->nworkers >= n) {
            e->nworkers -= n;
        } else {
            e->nworkers = 0;
        }
    }
    lmx_msg_exec_unlock(rt);
}

static void unbind_slot_locked(LmxMsgExec *e, LmxMsgExecBind *rec) {
    LmxMsg *old;
    LmxMsgBindWait *w;
    if (e == 0 || rec == 0 || rec->in_table == 0) {
        return;
    }
    w = rec->wait;
    rec->wait = 0;
    if (w != 0) {
        w->slot = 0;
        bind_reap_push(e, w);
        bind_wait_maybe_free_locked(e, w);
    }
    ctx_unlink_locked(rec);
    old = rec->msg;
    rec->msg = 0;
    if (old != 0) {
        map_ready_unlink_msg(old);
        lmx_msg_endp_release(old);
    }
    rec->in_table = 0;
}

static void join_bind_worker(LmxMsgRuntime *rt, LmxMsgExecBind *rec) {
    LmxMsgExec *e = exof(rt);
    LmxMsgBindWait *w;
    int had;
    if (e == 0 || rec == 0) {
        return;
    }
    w = rec->wait;
    rec->wait = 0;
    if (w == 0) {
        return;
    }
    w->slot = 0;
    w->retired = 1;
    w->reaping = 1;
    bind_wait_signal(w);
    lmx_msg_exec_unlock(rt);
    had = bind_wait_join(w);
    lmx_msg_exec_lock(rt);
    w->reaping = 0;
    if (had != 0 && e->nworkers > 0) {
        e->nworkers -= 1;
    }
    bind_wait_maybe_free_locked(e, w);
}

static int bind_kick_needed_locked(LmxMsg *m) {
    if (m == 0) {
        return 0;
    }
    if (m->state == LMX_MSG_STATE_STOPPED || m->state == LMX_MSG_STATE_DEAD
        || m->state == LMX_MSG_STATE_RELEASED) {
        return 0;
    }
    if (lmx_msg_mail_inbox_empty(m) == 0) {
        return 1;
    }
    if (m->closing != 0) {
        return 1;
    }
    return 0;
}

int lmx_msg_exec_bind(LmxMsgRuntime *rt, LmxMsgAddr addr, LmxMsgTurn turn, void *ctx, int affinity) {
    LmxMsgExec *e = exof(rt);
    LmxMsg *m;
    LmxMsgExecBind *rec;
    int owner;
    int live;
    int kick = 0;
    if (e == 0 || addr == 0U || turn == 0) {
        return LMX_MSG_INVALID;
    }
    owner = lmx_msg_host_is_owner(rt);
    if (owner == 0 && lmx_msg_exec_holding_any(rt) == 0) {
        return LMX_MSG_INVALID;
    }
    bind_reap_join_all(rt);
    lmx_msg_exec_lock(rt);
    if (e->unbound_held == addr) {
        lmx_msg_exec_unlock(rt);
        return LMX_MSG_INVALID;
    }
    m = msg_at_addr(rt, addr);
    if (m == 0) {
        lmx_msg_exec_unlock(rt);
        return LMX_MSG_INVALID;
    }
    if (owner == 0) {
        if (lmx_msg_exec_holding_turn(rt, addr) == 0 && (m->parent == 0U || lmx_msg_exec_holding_turn(rt, m->parent) == 0)) {
            lmx_msg_exec_unlock(rt);
            return LMX_MSG_INVALID;
        }
    }
    rec = bind_rec_locked(m);
    if (rec != 0) {
        int old_aff;
        int need;
        int st;
        if (rec->held != 0) {
            lmx_msg_exec_unlock(rt);
            return LMX_MSG_INVALID;
        }
        if (rec->affinity != affinity && affinity == LMX_MSG_AFFINITY_UI && owner == 0) {
            lmx_msg_exec_unlock(rt);
            return LMX_MSG_INVALID;
        }
        old_aff = rec->affinity;
        rec->turn = turn;
        rec->ctx = ctx;
        rec->gone = 0;
        m->turn = turn;
        m->turn_ctx = ctx;
        kick = bind_kick_needed_locked(m);
        if (old_aff != affinity) {
            if (affinity == LMX_MSG_AFFINITY_UI) {
                rec->affinity = LMX_MSG_AFFINITY_UI;
                map_ready_unlink_msg(m);
                join_bind_worker(rt, rec);
                lmx_msg_exec_unlock(rt);
                lmx_msg_exec_flush_retire(rt);
                if (kick != 0) {
                    lmx_msg_exec_ready(rt, addr);
                }
                return LMX_MSG_OK;
            }
            /* UI->ANY: keep ui_map_ready until launch commits. */
            need = e->contexts_live != 0 && bind_has_worker(rec) == 0;
            if (need != 0) {
                LmxMsgBindWait *cap;
                unsigned gen;
                if (rec->wait == 0) {
                    rec->wait = bind_wait_new(e);
                    if (rec->wait == 0) {
                        lmx_msg_exec_unlock(rt);
                        return LMX_MSG_NOMEM;
                    }
                    rec->wait->rec = rec;
                }
                cap = rec->wait;
                gen = cap->gen;
                rec->launching = 1;
                rec->affinity = affinity;
                bind_wait_launch_hold_locked(cap);
                lmx_msg_exec_unlock(rt);
                st = launch_ctx_thread(rt, addr);
                lmx_msg_exec_lock(rt);
                rec = rec_at_addr_locked(e, addr);
                if (st != LMX_MSG_OK) {
                    if (rec != 0 && launch_same_gen(rec, cap, gen) != 0
                        && bind_has_worker(rec) == 0) {
                        rec->affinity = old_aff;
                        rec->launching = 0;
                        /* UI membership stays on ui_map_ready until a
                         * successful launch commit; do not restore the
                         * leftover host ui_ready[] ring. */
                    }
                    lmx_msg_exec_unlock(rt);
                    bind_wait_launch_release(rt, cap);
                    lmx_msg_exec_flush_retire(rt);
                    return st;
                }
                if (rec != 0 && launch_same_gen(rec, cap, gen) != 0) {
                    rec->launching = 0;
                    if (rec->msg != 0) {
                        map_ready_unlink_kind(e, rec->msg, 1);
                    }
                }
                lmx_msg_exec_unlock(rt);
                bind_wait_launch_release(rt, cap);
                lmx_msg_exec_flush_retire(rt);
                if (kick != 0) {
                    lmx_msg_exec_ready(rt, addr);
                }
                return LMX_MSG_OK;
            }
            rec->affinity = affinity;
            if (rec->msg != 0) {
                map_ready_unlink_kind(e, rec->msg, 1);
            }
            lmx_msg_exec_unlock(rt);
            lmx_msg_exec_flush_retire(rt);
            if (kick != 0) {
                lmx_msg_exec_ready(rt, addr);
            }
            return LMX_MSG_OK;
        }
        need = e->contexts_live != 0 && affinity != LMX_MSG_AFFINITY_UI && bind_has_worker(rec) == 0;
        lmx_msg_exec_unlock(rt);
        if (need != 0) {
            st = launch_ctx_thread(rt, addr);
            if (st != LMX_MSG_OK) {
                return st;
            }
        }
        if (kick != 0) {
            lmx_msg_exec_ready(rt, addr);
        }
        return LMX_MSG_OK;
    }
    if (m->exec_bind == 0) {
        m->exec_bind = (LmxMsgExecBind *)calloc(1U, sizeof(LmxMsgExecBind));
        if (m->exec_bind == 0) {
            lmx_msg_exec_unlock(rt);
            return LMX_MSG_NOMEM;
        }
    } else {
        memset(m->exec_bind, 0, sizeof(LmxMsgExecBind));
    }
    rec = m->exec_bind;
    rec->addr = addr;
    rec->turn = turn;
    rec->ctx = ctx;
    rec->affinity = affinity;
    if (lmx_msg_endp_retain(m) == 0) {
        lmx_msg_exec_unlock(rt);
        return LMX_MSG_NOMEM;
    }
    rec->msg = m;
    m->turn = turn;
    m->turn_ctx = ctx;
    {
        LmxMsgBindWait *w = bind_wait_new(e);
        if (w == 0) {
            rec->msg = 0;
            lmx_msg_endp_release(m);
            lmx_msg_exec_unlock(rt);
            return LMX_MSG_NOMEM;
        }
        rec->wait = w;
        w->rec = rec;
    }
    rec->in_table = 1;
    ctx_link_locked(rec, ready_owner_of(m));
    live = e->contexts_live;
    kick = bind_kick_needed_locked(m);
    {
        LmxMsgBindWait *cap = rec->wait;
        unsigned gen = cap != 0 ? cap->gen : 0U;
        if (live != 0 && affinity != LMX_MSG_AFFINITY_UI) {
            bind_wait_launch_hold_locked(cap);
        }
        lmx_msg_exec_unlock(rt);
#if defined(LMX_MSG_EXEC_TEST)
        if (lmx_msg_exec_test_after_bind_add != 0) {
            lmx_msg_exec_test_after_bind_add(rt);
        }
#endif
        if (live != 0 && affinity != LMX_MSG_AFFINITY_UI) {
            int st = launch_ctx_thread(rt, addr);
            if (st != LMX_MSG_OK) {
                lmx_msg_exec_lock(rt);
                rec = rec_at_addr_locked(e, addr);
                if (rec != 0 && launch_same_gen(rec, cap, gen) != 0
                    && bind_has_worker(rec) == 0) {
                    unbind_slot_locked(e, rec);
                }
                lmx_msg_exec_unlock(rt);
                bind_wait_launch_release(rt, cap);
                return st;
            }
            bind_wait_launch_release(rt, cap);
        }
    }
    if (kick != 0) {
        lmx_msg_exec_ready(rt, addr);
    }
    return LMX_MSG_OK;
}

static LmxMsg *msg_at_addr(LmxMsgRuntime *rt, LmxMsgAddr addr) {
    return lmx_msg_self_or_find(rt, addr);
}

static int take_ready(LmxMsgExec *e, LmxMsgExecBind *snap) {
    LmxMsgExecBind *r;
    LmxMsgAddr addr;
    addr = lmx_msg_exec_take_addr(e->rt);
    if (addr == 0U) {
        return 0;
    }
    r = rec_at_addr_locked(e, addr);
    if (r == 0) {
        return 0;
    }
    *snap = *r;
    return 1;
}

static void native_leave_addr(LmxMsgRuntime *rt, LmxMsgAddr addr) {
    LmxMsg *m;
    lmx_msg_exec_lock(rt);
    m = msg_at_addr(rt, addr);
    if (m != 0 && m->native_users > 0) {
        m->native_users -= 1;
    }
    if (m != 0 && lmx_msg_running_load(m) == 0 && m->native_users == 0) {
        m->handoff_ready = 1;
    }
    lmx_msg_exec_unlock(rt);
}

static void requeue_if_runnable(LmxMsgRuntime *rt, LmxMsgAddr addr) {
    LmxMsg *m;
    lmx_msg_exec_lock(rt);
    m = lmx_msg_self_or_find(rt, addr);
    if (m != 0 && lmx_msg_exec_is_runnable(rt, addr) != 0) {
        lmx_msg_exec_unlock(rt);
        lmx_msg_exec_ready(rt, addr);
        return;
    }
    lmx_msg_exec_unlock(rt);
}

static int run_one(LmxMsgRuntime *rt, LmxMsgExecBind *snap) {
    LmxMsgExec *e = exof(rt);
    LmxMsgAddr old;
    LmxMsg *m;
    int st;
    int live = 0;
    int clean;
    LmxTurnRoot root;
    LmxTurnRoot *saved;
    memset(&root, 0, sizeof(root));
    saved = lmx_turn_root;
    root.prev = saved;
    old = get_tls(e);
    set_tls(e, snap->addr);
    lmx_msg_exec_lock(rt);
    m = msg_at_addr(rt, snap->addr);
    if (m != 0) {
        m->native_users += 1;
    }
    lmx_msg_exec_unlock(rt);
    lmx_turn_root = &root;
    if (setjmp(root.jmp) != 0) {
        st = 0;
        live = 0;
        lmx_msg_exec_lock(rt);
        m = msg_at_addr(rt, snap->addr);
        if (m != 0) {
            m->closing = 1;
            live = m->exec_live;
        }
        lmx_msg_exec_unlock(rt);
        (void)lmx_msg_end_turn(rt, snap->addr, 0);
        lmx_msg_exec_lock(rt);
        {
            /* Stage 3a-2: the running Message's own record, not a table scan. */
            LmxMsgExecBind *rec = bind_rec_locked(msg_at_addr(rt, snap->addr));
            if (rec != 0 && rec->addr == snap->addr
                && (snap->wait == 0 || rec->wait == snap->wait)) {
                rec->held = 0;
                rec->held_by = 0;
                rec->last_st = st;
            }
        }
        lmx_msg_exec_unlock(rt);
        turn_root_pop(&root, saved);
        set_tls(e, old);
        native_leave_addr(rt, snap->addr);
        requeue_if_runnable(rt, snap->addr);
        return st;
    }
    root.ready = 1;
    lmx_msg_exec_lock(rt);
    m = msg_at_addr(rt, snap->addr);
    /* Mikhail 2026-09-14: a Message that declared its work done (success=1)
     * runs no further turn body; running=0 alone now means a stop request,
     * and running is cleared from success only in end_turn. Both settle here
     * without running the body. */
    if (m != 0 && (lmx_msg_running_load(m) == 0 || lmx_msg_success_load(m) != 0)) {
        m->closing = 1;
        lmx_msg_exec_unlock(rt);
        st = 0;
        lmx_msg_end_turn(rt, snap->addr, 0);
        lmx_msg_exec_lock(rt);
        {
            /* Stage 3a-2: the running Message's own record, not a table scan. */
            LmxMsgExecBind *rec = bind_rec_locked(msg_at_addr(rt, snap->addr));
            if (rec != 0 && rec->addr == snap->addr
                && (snap->wait == 0 || rec->wait == snap->wait)) {
                rec->held = 0;
                rec->held_by = 0;
                rec->last_st = st;
            }
        }
        lmx_msg_exec_unlock(rt);
        turn_root_pop(&root, saved);
        set_tls(e, old);
        native_leave_addr(rt, snap->addr);
        return st;
    }
    lmx_msg_exec_unlock(rt);
    st = snap->turn(rt, snap->addr, snap->ctx);
    lmx_msg_exec_lock(rt);
    m = msg_at_addr(rt, snap->addr);
    if (m != 0) {
        live = m->exec_live;
        /* Mikhail 2026-09-14: the one who executed the Message clears
         * running, at the turn boundary, from success=1. A body that neither
         * received nor ended its turn still reaches this boundary. */
        if (lmx_msg_success_load(m) != 0) {
            lmx_msg_running_store(m, 0);
        }
    }
    lmx_msg_exec_unlock(rt);
    if (live != 0) {
        clean = lmx_msg_end_turn(rt, snap->addr, 0);
        if (st == 0) {
            st = clean == LMX_MSG_OK ? 1 : clean;
        }
    }
#if defined(LMX_MSG_EXEC_TEST)
    if (lmx_msg_exec_test_after_cleanup != 0) {
        lmx_msg_exec_test_after_cleanup(snap->addr, live, st);
    }
#endif
    lmx_msg_exec_lock(rt);
    {
        /* Stage 3a-2: the running Message's own record, not a table scan. */
        LmxMsgExecBind *rec = bind_rec_locked(msg_at_addr(rt, snap->addr));
        if (rec != 0 && rec->addr == snap->addr
            && (snap->wait == 0 || rec->wait == snap->wait)) {
            rec->held = 0;
            rec->held_by = 0;
            rec->last_st = st;
        }
    }
    /* take_addr skips held without dequeue; waiters must re-scan. */
    lmx_msg_exec_wake_locked(rt);
    lmx_msg_exec_unlock(rt);
    turn_root_pop(&root, saved);
    set_tls(e, old);
    native_leave_addr(rt, snap->addr);
    requeue_if_runnable(rt, snap->addr);
    return st;
}

/* Stage 3b-7b: exec_start_map_kick's collection, over the owners' lists. */
typedef struct CtxKickCollect {
    LmxMsgAddr *kicks;
    LmxMsg **pars;
    LmxMsg **chs;
    int nk;
    int nu;
    int st;
} CtxKickCollect;

static int ctx_visit_kick(LmxMsgExec *e, LmxMsg *owner, LmxMsgExecBind *rec, void *arg) {
    CtxKickCollect *k = (CtxKickCollect *)arg;
    (void)e;
    (void)owner;
    for (; rec != 0; rec = rec->ctx_next) {
        LmxMsg *cm = rec->msg;
        LmxMsg *par;
        if (cm == 0 || cm->mapped != 0 || rec->addr == 0U) {
            continue;
        }
        par = cm->parent_msg;
        if (lmx_msg_endp_retain(cm) == 0) {
            k->st = LMX_MSG_NOMEM;
            return 1;
        }
        if (par != 0 && lmx_msg_endp_retain(par) == 0) {
            lmx_msg_endp_release(cm);
            k->st = LMX_MSG_NOMEM;
            return 1;
        }
        k->pars[k->nu] = par;
        k->chs[k->nu] = cm;
        k->kicks[k->nk] = rec->addr;
        k->nk += 1;
        k->nu += 1;
    }
    return 0;
}

static int exec_start_map_kick(LmxMsgRuntime *rt) {
    LmxMsgExec *e = exof(rt);
    LmxMsgAddr *kicks = 0;
    int nk = 0;
    int b;
    int nrec = 0;
    if (e == 0) {
        return LMX_MSG_INVALID;
    }
    lmx_msg_exec_lock(rt);
    (void)ctx_walk_locked(e, ctx_visit_count, &nrec);
    if (nrec > 0) {
#if defined(LMX_MSG_EXEC_TEST)
        if (e->test_fail_start_kicks != 0) {
            e->test_fail_start_kicks = 0;
            lmx_msg_exec_unlock(rt);
            return LMX_MSG_NOMEM;
        }
#endif
        kicks = (LmxMsgAddr *)calloc((size_t)nrec, sizeof(LmxMsgAddr));
        if (kicks == 0) {
            lmx_msg_exec_unlock(rt);
            return LMX_MSG_NOMEM;
        }
        {
            LmxMsg **pars = (LmxMsg **)calloc((size_t)nrec, sizeof(LmxMsg *));
            LmxMsg **chs = (LmxMsg **)calloc((size_t)nrec, sizeof(LmxMsg *));
            int nu = 0;
            if (pars == 0 || chs == 0) {
                free(pars);
                free(chs);
                free(kicks);
                lmx_msg_exec_unlock(rt);
                return LMX_MSG_NOMEM;
            }
            {
                CtxKickCollect kc;
                kc.kicks = kicks;
                kc.pars = pars;
                kc.chs = chs;
                kc.nk = 0;
                kc.nu = 0;
                kc.st = LMX_MSG_OK;
                (void)ctx_walk_locked(e, ctx_visit_kick, &kc);
                nk = kc.nk;
                nu = kc.nu;
                if (kc.st != LMX_MSG_OK) {
                    while (nu > 0) {
                        nu -= 1;
                        lmx_msg_endp_release(chs[nu]);
                        if (pars[nu] != 0) {
                            lmx_msg_endp_release(pars[nu]);
                        }
                    }
                    free(pars);
                    free(chs);
                    free(kicks);
                    lmx_msg_exec_unlock(rt);
                    return kc.st;
                }
            }
            lmx_msg_exec_unlock(rt);
            for (b = 0; b < nu; b++) {
                if (pars[b] != 0) {
                    lmx_msg_sched_unlink_child(pars[b], chs[b]);
                }
            }
            lmx_msg_exec_lock(rt);
            for (b = 0; b < nu; b++) {
                if (chs[b] != 0 && (pars[b] == 0 || chs[b]->parent_msg == pars[b]
                    || chs[b]->parent_msg == 0)) {
                    chs[b]->mapped = 1;
                }
            }
            lmx_msg_exec_unlock(rt);
            for (b = 0; b < nu; b++) {
                lmx_msg_endp_release(chs[b]);
                if (pars[b] != 0) {
                    lmx_msg_endp_release(pars[b]);
                }
            }
            lmx_msg_exec_lock(rt);
            free(pars);
            free(chs);
        }
    }
    lmx_msg_exec_unlock(rt);
    for (b = 0; b < nk; b++) {
        lmx_msg_exec_ready(rt, kicks[b]);
    }
    free(kicks);
    return LMX_MSG_OK;
}

typedef struct LmxMsgCtxPack {
    LmxMsgRuntime *rt;
    LmxMsgAddr addr;
    LmxMsgBindWait *wait;
    volatile int go;
#if defined(_WIN32)
    HANDLE gate;
#else
    pthread_mutex_t gm;
    pthread_cond_t gc;
#endif
} LmxMsgCtxPack;

static int pack_gate_init(LmxMsgCtxPack *p) {
    p->go = 0;
    p->wait = 0;
#if defined(_WIN32)
    p->gate = CreateEventA(0, 1, 0, 0);
    return p->gate != 0 ? 0 : 1;
#else
    if (pthread_mutex_init(&p->gm, 0) != 0) {
        return 1;
    }
    if (pthread_cond_init(&p->gc, 0) != 0) {
        pthread_mutex_destroy(&p->gm);
        return 1;
    }
    return 0;
#endif
}

static void pack_gate_wait(LmxMsgCtxPack *p) {
#if defined(_WIN32)
    WaitForSingleObject(p->gate, INFINITE);
#else
    pthread_mutex_lock(&p->gm);
    while (p->go == 0) {
        pthread_cond_wait(&p->gc, &p->gm);
    }
    pthread_mutex_unlock(&p->gm);
#endif
}

static void pack_gate_signal(LmxMsgCtxPack *p, int go) {
#if defined(_WIN32)
    p->go = go;
    SetEvent(p->gate);
#else
    pthread_mutex_lock(&p->gm);
    p->go = go;
    pthread_cond_signal(&p->gc);
    pthread_mutex_unlock(&p->gm);
#endif
}

static void pack_gate_destroy(LmxMsgCtxPack *p) {
#if defined(_WIN32)
    if (p->gate != 0) {
        CloseHandle(p->gate);
        p->gate = 0;
    }
#else
    pthread_cond_destroy(&p->gc);
    pthread_mutex_destroy(&p->gm);
#endif
}

#if defined(_WIN32)
static DWORD WINAPI context_worker(void *arg);
#else
static void *context_worker(void *arg);
#endif

static int launch_same_gen(const LmxMsgExecBind *b, LmxMsgBindWait *cap, unsigned gen) {
    return b != 0 && cap != 0 && b->wait == cap && cap->gen == gen && cap->retired == 0;
}

static void launch_clear_if_gen(LmxMsgRuntime *rt, LmxMsgAddr addr, LmxMsgBindWait *cap, unsigned gen) {
    LmxMsgExec *e = exof(rt);
    LmxMsgExecBind *r;
    if (e == 0) {
        return;
    }
    lmx_msg_exec_lock(rt);
    r = rec_at_addr_locked(e, addr);
    if (r != 0 && launch_same_gen(r, cap, gen) != 0) {
        r->launching = 0;
    }
    lmx_msg_exec_unlock(rt);
}

static int launch_ctx_thread(LmxMsgRuntime *rt, LmxMsgAddr addr) {
    LmxMsgExec *e = exof(rt);
    LmxMsgCtxPack *pack;
    LmxMsgBindWait *cap;
    unsigned gen;
    LmxMsgExecBind *r;
#if defined(_WIN32)
    HANDLE th;
#else
    pthread_t th;
    int pr;
#endif
    cap = 0;
    gen = 0U;
    if (e == 0) {
        return LMX_MSG_INVALID;
    }
    lmx_msg_exec_lock(rt);
#if defined(LMX_MSG_EXEC_TEST)
    if (e->test_fail_ctx > 0) {
        e->test_fail_ctx -= 1;
        if (e->test_fail_ctx == 0) {
            lmx_msg_exec_unlock(rt);
            return LMX_MSG_NOMEM;
        }
    }
#endif
    r = rec_at_addr_locked(e, addr);
    if (r == 0 || e->stopping != 0 || r->affinity == LMX_MSG_AFFINITY_UI) {
        lmx_msg_exec_unlock(rt);
        return LMX_MSG_INVALID;
    }
    if (bind_has_worker(r) != 0) {
        lmx_msg_exec_unlock(rt);
        return LMX_MSG_OK;
    }
    if (r->wait == 0) {
        r->wait = bind_wait_new(e);
        if (r->wait == 0) {
            lmx_msg_exec_unlock(rt);
            return LMX_MSG_NOMEM;
        }
        r->wait->rec = r;
    }
    r->launching = 1;
    cap = r->wait;
    gen = cap->gen;
    bind_wait_launch_hold_locked(cap);
#if defined(LMX_MSG_EXEC_TEST)
    test_launch_cap = cap;
    test_launch_cap_gen = gen;
#endif
    lmx_msg_exec_unlock(rt);
#if defined(LMX_MSG_EXEC_TEST)
    if (lmx_msg_exec_test_during_launch != 0) {
        lmx_msg_exec_test_during_launch(rt, addr, 0);
    }
#endif
    pack = (LmxMsgCtxPack *)malloc(sizeof(LmxMsgCtxPack));
    if (pack == 0) {
        launch_clear_if_gen(rt, addr, cap, gen);
        bind_wait_launch_release(rt, cap);
        return LMX_MSG_NOMEM;
    }
    memset(pack, 0, sizeof(*pack));
    pack->rt = rt;
    pack->addr = addr;
    if (pack_gate_init(pack) != 0) {
        free(pack);
        launch_clear_if_gen(rt, addr, cap, gen);
        bind_wait_launch_release(rt, cap);
        return LMX_MSG_NOMEM;
    }
#if defined(_WIN32)
    th = CreateThread(0, 0, context_worker, pack, 0, 0);
    if (th == 0) {
#else
    pr = pthread_create(&th, 0, context_worker, pack);
    if (pr != 0) {
#endif
        pack_gate_destroy(pack);
        free(pack);
        launch_clear_if_gen(rt, addr, cap, gen);
        bind_wait_launch_release(rt, cap);
        return LMX_MSG_NOMEM;
    }
    lmx_msg_exec_lock(rt);
    r = rec_at_addr_locked(e, addr);
    if (r == 0 || e->stopping != 0 || r->gone != 0
        || r->affinity == LMX_MSG_AFFINITY_UI
        || launch_same_gen(r, cap, gen) == 0
        || bind_has_worker(r) != 0) {
        if (r != 0 && launch_same_gen(r, cap, gen) != 0
            && bind_has_worker(r) == 0) {
            r->launching = 0;
        }
        lmx_msg_exec_unlock(rt);
        pack_gate_signal(pack, 2);
#if defined(_WIN32)
        WaitForSingleObject(th, INFINITE);
        CloseHandle(th);
#else
        pthread_join(th, 0);
#endif
        bind_wait_launch_release(rt, cap);
        return LMX_MSG_INVALID;
    }
#if defined(_WIN32)
    r->wait->worker = th;
#else
    r->wait->worker = th;
    r->wait->worker_on = 1;
#endif
    r->launching = 0;
    e->nworkers += 1;
    pack->wait = r->wait;
    bind_wait_signal(r->wait);
    lmx_msg_exec_unlock(rt);
    pack_gate_signal(pack, 1);
#if defined(LMX_MSG_EXEC_TEST)
    if (lmx_msg_exec_test_during_launch != 0) {
        lmx_msg_exec_test_during_launch(rt, addr, 1);
    }
#endif
    bind_wait_launch_release(rt, cap);
    return LMX_MSG_OK;
}

/* Stage 3a-2: take a turn of the worker's own record (reached through its
 * wait generation), with every refusal the address scan applied. */
static int take_this(LmxMsgExec *e, LmxMsgExecBind *r, LmxMsgAddr addr, LmxMsgExecBind *snap) {
    LmxMsg *m;
    if (e == 0 || r == 0 || addr == 0U || snap == 0) {
        return 0;
    }
    if (r->in_table == 0 || r->addr != addr) {
        return 0;
    }
    if (r->affinity == LMX_MSG_AFFINITY_UI) {
        return 0;
    }
    if (r->held != 0 || r->gone != 0) {
        return 0;
    }
    m = r->msg;
    if (m == 0 || m->addr != addr) {
        return 0;
    }
    if (m->state == LMX_MSG_STATE_STOPPED || m->state == LMX_MSG_STATE_DEAD || m->state == LMX_MSG_STATE_RELEASED) {
        return 0;
    }
    if (lmx_msg_mail_inbox_empty(m) != 0 && m->closing == 0) {
        return 0;
    }
    r->held = 1;
    r->held_by = lmx_tid();
    map_ready_unlink_kind(e, m, 0);
    *snap = *r;
    return 1;
}

#if defined(_WIN32)
static DWORD WINAPI context_worker(void *arg) {
    LmxMsgCtxPack *p = (LmxMsgCtxPack *)arg;
    LmxMsgRuntime *rt;
    LmxMsgAddr addr;
    LmxMsgExec *e;
    LmxMsgExecBind snap;
    HANDLE ev;
    HANDLE wh[2];
    LmxMsgBindWait *mine;
    LmxMsgExecBind *rec;
    int go;
    if (p == 0) {
        return 1;
    }
    rt = p->rt;
    addr = p->addr;
    pack_gate_wait(p);
    go = p->go;
    mine = p->wait;
    pack_gate_destroy(p);
    free(p);
    if (go != 1 || mine == 0 || mine->retired != 0) {
        return 0;
    }
    mine->owner_tid = lmx_tid();
    e = exof(rt);
    if (e == 0) {
        return 1;
    }
    for (;;) {
        ev = 0;
        lmx_msg_exec_lock(rt);
        if (e->stopping != 0) {
            lmx_msg_exec_unlock(rt);
            return 0;
        }
        if (mine != 0 && mine->retired != 0) {
            lmx_msg_exec_unlock(rt);
            return 0;
        }
        /* Stage 3a-2: this worker's own record, through its generation. Every
         * detach retires the generation under the lock, so after the retired
         * exit above rec is the live record. */
        rec = mine->rec;
        if (rec->affinity == LMX_MSG_AFFINITY_UI || rec->gone != 0) {
            lmx_msg_exec_unlock(rt);
            return 0;
        }
        ev = mine->wait_ev;
        if (take_this(e, rec, addr, &snap) != 0) {
            lmx_msg_exec_unlock(rt);
            run_one(rt, &snap);
            continue;
        }
        lmx_msg_exec_unlock(rt);
        if (ev == 0) {
            return 0;
        }
        wh[0] = e->stop_ev;
        wh[1] = ev;
        {
            LmxMsg *self;
            unsigned th = 0;
            unsigned now = 0;
            DWORD to = INFINITE;
            DWORD wr;
            lmx_msg_exec_lock(rt);
            self = msg_at_addr(rt, addr);
            if (self != 0) {
                th = self->live_wait_th;
            }
            lmx_msg_exec_unlock(rt);
            if (th != 0U) {
                to = 20;
            }
            wr = WaitForMultipleObjects(2, wh, 0, to);
            if (wr == WAIT_OBJECT_0) {
                return 0;
            }
            if (th != 0U && wr != WAIT_FAILED) {
                now = lmx_msg_now(rt);
                lmx_msg_exec_lock(rt);
                set_tls(e, addr);
                lmx_msg_exec_unlock(rt);
                lmx_msg_live_check(rt, addr, now, th);
                lmx_msg_exec_lock(rt);
                set_tls(e, 0);
                lmx_msg_exec_unlock(rt);
            }
        }
    }
}
#else
static void *context_worker(void *arg) {
    LmxMsgCtxPack *p = (LmxMsgCtxPack *)arg;
    LmxMsgRuntime *rt;
    LmxMsgAddr addr;
    LmxMsgExec *e;
    LmxMsgExecBind snap;
    LmxMsgBindWait *w;
    LmxMsgExecBind *rec;
    int gone;
    int ui;
    int go;
    if (p == 0) {
        return 0;
    }
    rt = p->rt;
    addr = p->addr;
    pack_gate_wait(p);
    go = p->go;
    w = p->wait;
    pack_gate_destroy(p);
    free(p);
    if (go != 1 || w == 0 || w->retired != 0) {
        return 0;
    }
    w->owner_tid = lmx_tid();
    e = exof(rt);
    if (e == 0) {
        return 0;
    }
    for (;;) {
        lmx_msg_exec_lock(rt);
        if (e->stopping != 0) {
            lmx_msg_exec_unlock(rt);
            return 0;
        }
        if (w != 0 && w->retired != 0) {
            lmx_msg_exec_unlock(rt);
            return 0;
        }
        /* Stage 3a-2: this worker's own record, through its generation (see
         * the Win32 worker). A detached generation exits on retired above; the
         * address scan used to keep waiting when no entry had the address. */
        rec = w->rec;
        gone = (rec->gone != 0);
        ui = (rec->affinity == LMX_MSG_AFFINITY_UI);
        if (ui != 0 || gone != 0) {
            lmx_msg_exec_unlock(rt);
            return 0;
        }
        if (take_this(e, rec, addr, &snap) != 0) {
            lmx_msg_exec_unlock(rt);
            run_one(rt, &snap);
            continue;
        }
        if (w == 0) {
            lmx_msg_exec_unlock(rt);
            return 0;
        }
        while (e->stopping == 0 && w->sig == 0) {
            pthread_cond_wait(&w->cv, &e->lock);
        }
        w->sig = 0;
        lmx_msg_exec_unlock(rt);
    }
}
#endif

/* Stage 3b-7b: start_contexts restarts the walk after each launch; a successful
 * launch records the worker under the lock, so the record is not picked again. */
static int ctx_visit_first_launchable(LmxMsgExec *e, LmxMsg *owner, LmxMsgExecBind *rec, void *arg) {
    (void)e;
    (void)owner;
    for (; rec != 0; rec = rec->ctx_next) {
        if (rec->affinity != LMX_MSG_AFFINITY_UI && rec->gone == 0 && bind_has_worker(rec) == 0) {
            *(LmxMsgAddr *)arg = rec->addr;
            return 1;
        }
    }
    return 0;
}

int lmx_msg_exec_start_contexts(LmxMsgRuntime *rt) {
    LmxMsgExec *e = exof(rt);
    LmxMsgAddr addr;
    int st;
    if (e == 0) {
        return LMX_MSG_INVALID;
    }
    if (lmx_msg_host_is_owner(rt) == 0) {
        return LMX_MSG_INVALID;
    }
    bind_reap_join_all(rt);
    lmx_msg_exec_lock(rt);
    if (e->contexts_live != 0) {
        lmx_msg_exec_unlock(rt);
        return LMX_MSG_INVALID;
    }
    e->stopping = 0;
    e->stopped = 0;
#if defined(_WIN32)
    ResetEvent(e->stop_ev);
#endif
    e->contexts_live = 1;
    lmx_msg_exec_unlock(rt);
    st = exec_start_map_kick(rt);
    if (st != LMX_MSG_OK) {
        lmx_msg_exec_stop(rt);
        return st;
    }
    for (;;) {
        addr = 0U;
        lmx_msg_exec_lock(rt);
        (void)ctx_walk_locked(e, ctx_visit_first_launchable, &addr);
        lmx_msg_exec_unlock(rt);
        if (addr == 0U) {
            break;
        }
        st = launch_ctx_thread(rt, addr);
        if (st != LMX_MSG_OK) {
            lmx_msg_exec_stop(rt);
            return st;
        }
    }
    return LMX_MSG_OK;
}

int lmx_msg_exec_ui_step(LmxMsgRuntime *rt) {
    LmxMsgExec *e = exof(rt);
    LmxMsgExecBind snap;
    if (e == 0) {
        return LMX_MSG_INVALID;
    }
    if (lmx_msg_host_is_owner(rt) == 0) {
        return LMX_MSG_INVALID;
    }
    lmx_msg_host_drain(rt);
    lmx_msg_exec_lock(rt);
    if (take_ready(e, &snap) == 0) {
        lmx_msg_exec_unlock(rt);
        return LMX_MSG_EMPTY;
    }
    lmx_msg_exec_unlock(rt);
    run_one(rt, &snap);
    return LMX_MSG_OK;
}

int lmx_msg_exec_last_status(LmxMsgRuntime *rt, LmxMsgAddr addr) {
    LmxMsgExec *e = exof(rt);
    LmxMsgExecBind *r;
    int st = LMX_MSG_INVALID;
    if (e == 0 || addr == 0U) {
        return LMX_MSG_INVALID;
    }
    lmx_msg_exec_lock(rt);
    r = rec_at_addr_locked(e, addr);
    if (r != 0) {
        st = r->last_st;
    }
    lmx_msg_exec_unlock(rt);
    return st;
}

int lmx_msg_exec_is_bound(LmxMsgRuntime *rt, LmxMsgAddr addr) {
    LmxMsgExec *e = exof(rt);
    int hit = 0;
    if (e == 0 || addr == 0U) {
        return 0;
    }
    lmx_msg_exec_lock(rt);
    hit = rec_at_addr_locked(e, addr) != 0;
    lmx_msg_exec_unlock(rt);
    return hit;
}

int lmx_msg_exec_unbound_close(LmxMsgRuntime *rt, LmxMsgAddr addr) {
    LmxMsgExec *e = exof(rt);
    int st;
    LmxMsgAddr old;
    if (e == 0 || addr == 0U) {
        return LMX_MSG_INVALID;
    }
    if (lmx_msg_host_is_owner(rt) == 0) {
        return LMX_MSG_INVALID;
    }
    lmx_msg_exec_lock(rt);
    if (rec_at_addr_locked(e, addr) != 0) {
        lmx_msg_exec_unlock(rt);
        lmx_msg_exec_ready(rt, addr);
        return LMX_MSG_OK;
    }
    if (e->unbound_held != 0) {
        lmx_msg_exec_unlock(rt);
        return LMX_MSG_INVALID;
    }
    e->unbound_held = addr;
    old = get_tls(e);
    set_tls(e, addr);
    lmx_msg_exec_unlock(rt);
    st = lmx_msg_end_turn(rt, addr, 1);
    lmx_msg_exec_lock(rt);
    e->unbound_held = 0;
    set_tls(e, old);
    lmx_msg_exec_unlock(rt);
    return st;
}

/* Stage 3b-7b: lmx_msg_exec_stop's passes over the owners' lists. */
static int ctx_visit_stop_unmap(LmxMsgExec *e, LmxMsg *owner, LmxMsgExecBind *rec, void *arg) {
    (void)e;
    (void)owner;
    (void)arg;
    for (; rec != 0; rec = rec->ctx_next) {
        if (rec->msg != 0) {
            map_ready_unlink_msg(rec->msg);
            rec->msg->mapped = 0;
        }
    }
    return 0;
}

static int ctx_visit_stop_retire_waits(LmxMsgExec *e, LmxMsg *owner, LmxMsgExecBind *rec, void *arg) {
    (void)owner;
    (void)arg;
    for (; rec != 0; rec = rec->ctx_next) {
        if (rec->wait != 0) {
            rec->wait->retired = 1;
            bind_wait_signal(rec->wait);
            bind_reap_push(e, rec->wait);
            rec->wait = 0;
        }
    }
    return 0;
}

static int ctx_visit_stop_reset(LmxMsgExec *e, LmxMsg *owner, LmxMsgExecBind *rec, void *arg) {
    (void)e;
    (void)owner;
    (void)arg;
    for (; rec != 0; rec = rec->ctx_next) {
        rec->held = 0;
        rec->held_by = 0;
        rec->launching = 0;
        rec->gone = 0;
        if (rec->msg != 0) {
            map_ready_unlink_msg(rec->msg);
            rec->msg->mapped = 0;
        }
    }
    return 0;
}

int lmx_msg_exec_stop(LmxMsgRuntime *rt) {
    LmxMsgExec *e = exof(rt);
    if (e == 0) {
        return LMX_MSG_INVALID;
    }
    if (lmx_msg_host_is_owner(rt) == 0) {
        return LMX_MSG_INVALID;
    }
    lmx_msg_exec_lock(rt);
    if (e->stopped != 0) {
        lmx_msg_exec_unlock(rt);
        return LMX_MSG_OK;
    }
    e->stopping = 1;
    e->contexts_live = 0;
    (void)ctx_walk_locked(e, ctx_visit_stop_unmap, 0);
    (void)ctx_walk_locked(e, ctx_visit_stop_retire_waits, 0);
#if defined(_WIN32)
    SetEvent(e->stop_ev);
#endif
    lmx_msg_exec_unlock(rt);
    lmx_msg_exec_flush_retire(rt);
    bind_reap_join_all(rt);
    e->nworkers = 0;
    lmx_msg_exec_lock(rt);
    set_tls(e, 0);
    (void)ctx_walk_locked(e, ctx_visit_stop_reset, 0);
    e->scan = 0;
    while (e->ui_map_own_head != 0) {
        ui_owner_lower(e, e->ui_map_own_head);
    }
    e->unbound_held = 0;
    e->stopped = 1;
    lmx_msg_exec_unlock(rt);
    lmx_msg_exec_flush_retire(rt);
    return LMX_MSG_OK;
}

/* Stage 3b-7b: drop_binds takes the first record the walk meets, unbinds it and
 * restarts until the walk finds none (no allocation on the teardown path). */
static int ctx_visit_first_record(LmxMsgExec *e, LmxMsg *owner, LmxMsgExecBind *rec, void *arg) {
    (void)e;
    (void)owner;
    *(LmxMsgExecBind **)arg = rec;
    return 1;
}

void lmx_msg_exec_drop_binds(LmxMsgRuntime *rt) {
    LmxMsgExec *e = exof(rt);
    LmxMsgExecBind *rec;
    if (e == 0) {
        return;
    }
    lmx_msg_exec_lock(rt);
    /* Stage 3a-1: the records are the Messages' own and die in
     * lmx_msg_slot_free, so an index entry retains its Message and leaves the
     * table before that retain is dropped (unbind_slot_locked). The old walk
     * released the Messages and kept the entries: runtime_delete's slot loop
     * then freed records the table still pointed at, and lmx_msg_exec_detach
     * read them (the executor selftest's first runtime_delete, 70 bound
     * Messages, access violation; lead's find, 2026-09-14). Drop = unbind
     * every record, the first the walk from rt->root meets, until none is left
     * (stage 3b-7b; unbinding unlinks it, so the walk restarts). */
    for (;;) {
        rec = 0;
        (void)ctx_walk_locked(e, ctx_visit_first_record, &rec);
        if (rec == 0) {
            break;
        }
        if (rec->msg != 0) {
            rec->msg->mapped = 0;
        }
        unbind_slot_locked(e, rec);
    }
    lmx_msg_exec_unlock(rt);
    lmx_msg_exec_flush_retire(rt);
}

void lmx_msg_exec_set_no_retire(LmxMsgRuntime *rt, int v) {
    LmxMsgExec *e = exof(rt);
    if (e == 0) {
        return;
    }
    e->no_retire = v;
}

int lmx_msg_exec_unbind(LmxMsgRuntime *rt, LmxMsgAddr addr) {
    LmxMsgExec *e = exof(rt);
    LmxMsgExecBind *r;
    if (e == 0 || addr == 0U) {
        return LMX_MSG_INVALID;
    }
    lmx_msg_exec_lock(rt);
    r = rec_at_addr_locked(e, addr);
    if (r == 0) {
        lmx_msg_exec_unlock(rt);
        return LMX_MSG_OK;
    }
    if (r->wait != 0) {
        LmxMsgBindWait *w = r->wait;
        r->wait = 0;
        bind_reap_push(e, w);
    }
    unbind_slot_locked(e, r);
    {
        int host = get_tls(e) == 0U;
        lmx_msg_exec_unlock(rt);
        if (host != 0) {
            bind_reap_join_all(rt);
        }
    }
    lmx_msg_exec_flush_retire(rt);
    return LMX_MSG_OK;
}

int lmx_msg_run_child_turn(LmxMsgRuntime *rt, LmxMsgAddr child) {
    LmxMsgExec *e = exof(rt);
    LmxMsg *m;
    LmxMsgExecBind snap;
    LmxMsgAddr par;
    int owner;
    int st;
    LmxMsgExecBind *rec;
    if (e == 0 || child == 0U) {
        return LMX_MSG_INVALID;
    }
    owner = lmx_msg_host_is_owner(rt);
    lmx_msg_exec_lock(rt);
    m = msg_at_addr(rt, child);
    if (m == 0 || m->turn == 0 || m->mapped != 0) {
        lmx_msg_exec_unlock(rt);
        return LMX_MSG_INVALID;
    }
    par = m->parent;
    if (par != 0U) {
        if (lmx_msg_exec_holding_turn(rt, par) == 0 && (owner == 0 || lmx_msg_exec_holding_any(rt) != 0)) {
            lmx_msg_exec_unlock(rt);
            return LMX_MSG_INVALID;
        }
    } else if (owner == 0) {
        lmx_msg_exec_unlock(rt);
        return LMX_MSG_INVALID;
    }
    /* Stage 3a-2: the child's own record (m resolved above). */
    rec = bind_rec_locked(m);
    if (rec == 0) {
        lmx_msg_exec_unlock(rt);
        return LMX_MSG_INVALID;
    }
    if (rec->held != 0) {
        lmx_msg_exec_unlock(rt);
        return LMX_MSG_INVALID;
    }
    rec->held = 1;
    rec->held_by = lmx_tid();
    memset(&snap, 0, sizeof(snap));
    snap.addr = child;
    snap.turn = m->turn;
    snap.ctx = m->turn_ctx;
    lmx_msg_exec_unlock(rt);
    st = run_one(rt, &snap);
    if (par != 0U && lmx_msg_exec_holding_turn(rt, par) != 0) {
        (void)lmx_msg_parent_settle(rt, par);
    }
    /* Decision 17: once the turn has left run_one, the host finishes an
     * orphan's end-turn (a successful orphan reclaims itself). */
    if (owner != 0 && lmx_msg_exec_holding_any(rt) == 0) {
        (void)lmx_msg_orphan_end(rt, child);
    }
    return st == 0 ? LMX_MSG_OK : st;
}

int lmx_msg_map_child(LmxMsgRuntime *rt, LmxMsgAddr parent, LmxMsgAddr child) {
    LmxMsgExec *e = exof(rt);
    LmxMsg *p;
    LmxMsg *c;
    int st;
    if (e == 0 || parent == 0U || child == 0U) {
        return LMX_MSG_INVALID;
    }
    if (lmx_msg_exec_holding_turn(rt, parent) == 0) {
        if (lmx_msg_host_is_owner(rt) == 0 || lmx_msg_exec_holding_any(rt) != 0) {
            return LMX_MSG_INVALID;
        }
    }
    lmx_msg_exec_lock(rt);
    p = msg_at_addr(rt, parent);
    c = msg_at_addr(rt, child);
    if (p == 0 || c == 0 || c->parent_msg != p) {
        lmx_msg_exec_unlock(rt);
        return LMX_MSG_INVALID;
    }
    {
        /* Stage 3a-2: the child's own record (c resolved above). */
        LmxMsgExecBind *rb = bind_rec_locked(c);
        LmxMsgBindWait *cap;
        unsigned gen;
        if (rb == 0) {
            lmx_msg_exec_unlock(rt);
            return LMX_MSG_INVALID;
        }
        if (rb->affinity == LMX_MSG_AFFINITY_UI) {
            lmx_msg_exec_unlock(rt);
            return LMX_MSG_INVALID;
        }
        if (c->mapped != 0) {
            lmx_msg_exec_unlock(rt);
            return LMX_MSG_OK;
        }
        c->mapped = 1;
        cap = rb->wait;
        gen = cap != 0 ? cap->gen : 0U;
        bind_wait_launch_hold_locked(cap);
        lmx_msg_exec_unlock(rt);
        st = launch_ctx_thread(rt, child);
        if (st != LMX_MSG_OK) {
            lmx_msg_exec_lock(rt);
            c = msg_at_addr(rt, child);
            rb = bind_rec_locked(c);
            if (c != 0) {
                if (rb == 0) {
                    c->mapped = 0;
                } else if (cap != 0 && launch_same_gen(rb, cap, gen) != 0
                    && bind_has_worker(rb) == 0) {
                    c->mapped = 0;
                } else if (cap == 0 && bind_has_worker(rb) == 0
                    && rb->wait == 0) {
                    c->mapped = 0;
                }
            }
            lmx_msg_exec_unlock(rt);
        }
        bind_wait_launch_release(rt, cap);
    }
    return st;
}

int lmx_msg_handoff_ready(LmxMsgRuntime *rt, LmxMsgAddr who) {
    LmxMsg *m = lmx_msg_self_or_find(rt, who);
    return m != 0 && m->handoff_ready != 0;
}

int lmx_msg_native_users(LmxMsgRuntime *rt, LmxMsgAddr who) {
    LmxMsg *m = lmx_msg_self_or_find(rt, who);
    return m != 0 ? m->native_users : -1;
}

#if defined(__GNUC__)
__attribute__((unused))
#endif
static int adopt_push(LmxMsg *p, void *base, size_t n) {
    LmxMsgBlock *b;
    if (base == 0) {
        return 0;
    }
    b = (LmxMsgBlock *)calloc(1U, sizeof(LmxMsgBlock));
    if (b == 0) {
        return 1;
    }
    b->base = base;
    b->n = n;
    if (lmx_msg_blocks_push(&p->blocks, b) != LMX_MSG_BLOCKS_OK) {
        free(b);
        return 1;
    }
    return 0;
}

static int lifecycle_authority(LmxMsgRuntime *rt, LmxMsgAddr who) {
    if (lmx_msg_exec_holding_turn(rt, who) != 0) {
        return 1;
    }
    if (lmx_msg_host_is_owner(rt) != 0 && lmx_msg_exec_holding_any(rt) == 0) {
        return 1;
    }
    return 0;
}

/* Decision 17 with spec 19.29.8: the C half of settling a failed direct child,
 * called by lmx_msg_settle_child after it checked authority and the child's own
 * settled children were settled into it. On refusal nothing has moved. */
int lmx_msg_exec_adopt_mark(LmxMsgRuntime *rt, LmxMsgAddr parent, LmxMsgAddr child) {
    LmxMsg *p;
    LmxMsg *c;
    if (rt == 0) {
        return LMX_MSG_INVALID;
    }
    lmx_msg_exec_lock(rt);
    p = lmx_msg_self_or_find(rt, parent);
    c = lmx_msg_self_or_find(rt, child);
    if (p == 0 || c == 0 || c->parent_msg != p || p->disposed != 0 || c->disposed != 0
        || c->native_users != 0 || lmx_msg_success_load(c) != 0
        || lmx_msg_running_load(c) != 0 || c->handoff_ready == 0) {
        lmx_msg_exec_unlock(rt);
        return LMX_MSG_INVALID;
    }
    if (c->init == 0 && c->blocks == 0 && c->ranges == 0) {
        lmx_msg_exec_unlock(rt);
        return LMX_MSG_INVALID;
    }
    if (lmx_msg_storage_can_move(&p->blocks, &p->ranges, &c->blocks, &c->ranges)
        != LMX_MSG_STORAGE_OK) {
        lmx_msg_exec_unlock(rt);
        return LMX_MSG_INVALID;
    }
    {
        LmxMsgBlock *prepared = 0;
        LmxMsgRoot *history = 0;
        if (lmx_msg_history_prepare(c, &history) != LMX_MSG_OK) {
            lmx_msg_exec_unlock(rt);
            return LMX_MSG_NOMEM;
        }
        if (c->init != 0) {
#if defined(LMX_MSG_EXEC_TEST)
            if (exof(rt) != 0 && exof(rt)->test_fail_adopt_block != 0) {
                lmx_msg_history_dispose(history);
                lmx_msg_exec_unlock(rt);
                return LMX_MSG_NOMEM;
            }
#endif
            prepared = (LmxMsgBlock *)calloc(1U, sizeof(LmxMsgBlock));
            if (prepared == 0) {
                lmx_msg_history_dispose(history);
                lmx_msg_exec_unlock(rt);
                return LMX_MSG_NOMEM;
            }
            prepared->base = c->init;
            prepared->n = c->init_n;
        }
        if (lmx_msg_storage_move_all(&p->blocks, &p->ranges, &c->blocks, &c->ranges)
            != LMX_MSG_STORAGE_OK) {
            lmx_msg_history_dispose(history);
            free(prepared);
            lmx_msg_exec_unlock(rt);
            return LMX_MSG_INVALID;
        }
        c->graph = 0;
        lmx_msg_roots_drop_stale(c);
        lmx_msg_history_commit(p, history);
        if (prepared != 0) {
            if (lmx_msg_blocks_push(&p->blocks, prepared) != LMX_MSG_BLOCKS_OK) {
                free(prepared);
                lmx_msg_exec_unlock(rt);
                return LMX_MSG_INVALID;
            }
            c->init = 0;
            c->init_n = 0U;
        }
    }
    c->disposed = 1;
    lmx_msg_exec_unlock(rt);
    return LMX_MSG_OK;
}

int lmx_msg_adopted_n(LmxMsgRuntime *rt, LmxMsgAddr who) {
    LmxMsg *m;
    LmxMsgBlock *a;
    int n = 0;
    lmx_msg_exec_lock(rt);
    m = lmx_msg_self_or_find(rt, who);
    if (m == 0) {
        lmx_msg_exec_unlock(rt);
        return -1;
    }
    for (a = m->blocks; a != 0; a = a->next) {
        n += 1;
    }
    lmx_msg_exec_unlock(rt);
    return n;
}

void *lmx_msg_adopted_base(LmxMsgRuntime *rt, LmxMsgAddr who, int i) {
    LmxMsg *m;
    LmxMsgBlock *a;
    int k = 0;
    void *base = 0;
    lmx_msg_exec_lock(rt);
    m = lmx_msg_self_or_find(rt, who);
    if (m == 0 || i < 0) {
        lmx_msg_exec_unlock(rt);
        return 0;
    }
    for (a = m->blocks; a != 0; a = a->next) {
        if (k == i) {
            base = a->base;
            break;
        }
        k += 1;
    }
    lmx_msg_exec_unlock(rt);
    return base;
}

static void drop_adopted_locked(LmxMsg *m) {
    if (m != 0) {
        drop_ranges_locked(m);
        (void)lmx_msg_blocks_dispose_all(&m->blocks);
    }
}

int lmx_msg_transfer_adopted(LmxMsgRuntime *rt, LmxMsgAddr from, LmxMsgAddr to) {
    LmxMsg *src;
    LmxMsg *dst;
    if (lifecycle_authority(rt, to) == 0) {
        return LMX_MSG_INVALID;
    }
    lmx_msg_exec_lock(rt);
    src = lmx_msg_self_or_find(rt, from);
    dst = lmx_msg_self_or_find(rt, to);
    if (src == 0 || dst == 0 || src == dst
        || (src->blocks == 0 && src->ranges == 0)
        || src->parent_msg != dst || src->native_users != 0
        || lmx_msg_running_load(src) != 0 || src->handoff_ready == 0
        || dst->disposed != 0
        || dst->state == LMX_MSG_STATE_DEAD || dst->state == LMX_MSG_STATE_RELEASED) {
        lmx_msg_exec_unlock(rt);
        return LMX_MSG_INVALID;
    }
    {
        LmxMsg *ch = src->first_child;
        while (ch != 0) {
            if (ch->disposed == 0) {
                lmx_msg_exec_unlock(rt);
                return LMX_MSG_INVALID;
            }
            ch = ch->next_sibling;
        }
    }
    if (handoff_move_locked(dst, src) != LMX_MSG_OK) {
        lmx_msg_exec_unlock(rt);
        return LMX_MSG_INVALID;
    }
    lmx_msg_exec_unlock(rt);
    return LMX_MSG_OK;
}

static int transfer_graph_locked_api(LmxMsgRuntime *rt, LmxMsgAddr from,
                                     LmxMsgAddr to, Lmx *root,
                                     int direct_parent_only,
                                     int complete_delivery) {
    LmxMsg *src;
    LmxMsg *dst;
    LmxMsg *ch;
    LmxOwnedRange *rg;
    LmxMsgRoot *prepared;
    LmxMsgRoot *cur;
    if (rt == 0 || root == 0 || lifecycle_authority(rt, to) == 0) {
        return LMX_MSG_INVALID;
    }
    lmx_msg_exec_lock(rt);
    src = lmx_msg_self_or_find(rt, from);
    dst = lmx_msg_self_or_find(rt, to);
    if (src == 0 || dst == 0 || src == dst
        || (direct_parent_only != 0 && src->parent_msg != dst)
        || src->native_users != 0
        || lmx_msg_running_load(src) != 0 || lmx_msg_success_load(src) == 0
        || src->handoff_ready == 0 || src->disposed != 0
        || dst->disposed != 0
        || dst->state == LMX_MSG_STATE_DEAD || dst->state == LMX_MSG_STATE_RELEASED) {
        lmx_msg_exec_unlock(rt);
        return LMX_MSG_INVALID;
    }
    ch = src->first_child;
    while (ch != 0) {
        if (ch->disposed == 0) {
            lmx_msg_exec_unlock(rt);
            return LMX_MSG_INVALID;
        }
        ch = ch->next_sibling;
    }
    rg = lmx_owned_ranges_find(src->ranges, root);
    if (rg == 0 || (rg->kind != LMX_KIND_STRUCT
        && rg->kind != LMX_KIND_ARRAY && rg->kind != LMX_KIND_CHILDREN)
        || lmx_msg_storage_can_move(&dst->blocks, &dst->ranges,
                                    &src->blocks, &src->ranges)
            != LMX_MSG_STORAGE_OK) {
        lmx_msg_exec_unlock(rt);
        return LMX_MSG_INVALID;
    }
    for (cur = dst->roots; cur != 0; cur = cur->next) {
        if (cur->p == root) {
            lmx_msg_exec_unlock(rt);
            return LMX_MSG_INVALID;
        }
    }
    if (lmx_msg_test_root_alloc_should_fail() != 0) {
        lmx_msg_exec_unlock(rt);
        return LMX_MSG_NOMEM;
    }
    prepared = (LmxMsgRoot *)malloc(sizeof(LmxMsgRoot));
    if (prepared == 0) {
        lmx_msg_exec_unlock(rt);
        return LMX_MSG_NOMEM;
    }
    prepared->p = root;
    prepared->roles = LMX_MSG_ROOT_RETAIN;
    prepared->next = 0;
    if (lmx_msg_storage_move_all(&dst->blocks, &dst->ranges,
                                 &src->blocks, &src->ranges)
        != LMX_MSG_STORAGE_OK) {
        free(prepared);
        lmx_msg_exec_unlock(rt);
        return LMX_MSG_INVALID;
    }
    if (src->graph != 0) {
        src->graph = 0;
    }
    lmx_msg_roots_drop_stale(src);
    prepared->next = dst->roots;
    dst->roots = prepared;
    if (complete_delivery != 0) {
        /* Delivery consumes this completed assignment.  Keep parent_msg as
         * the original lifecycle relation until its owner disposes the empty
         * child; the recipient never becomes a new supervisor. */
        src->tracked = 0;
    }
    lmx_msg_exec_unlock(rt);
    return LMX_MSG_OK;
}

int lmx_msg_transfer_graph(LmxMsgRuntime *rt, LmxMsgAddr from, LmxMsgAddr to,
                           Lmx *root) {
    return transfer_graph_locked_api(rt, from, to, root, 1, 0);
}

int lmx_msg_deliver_graph(LmxMsgRuntime *rt, LmxMsgAddr from, LmxMsgAddr to,
                          Lmx *root) {
    return transfer_graph_locked_api(rt, from, to, root, 0, 1);
}

/* Decision 17 with spec 19.29.8: the C half of settling a direct child whose
 * storage is reclaimed (a successful history, or nothing to adopt), called by
 * lmx_msg_settle_child after it checked authority. */
int lmx_msg_exec_dispose_mark(LmxMsgRuntime *rt, LmxMsgAddr parent, LmxMsgAddr child) {
    LmxMsg *p;
    LmxMsg *c;
    if (rt == 0) {
        return LMX_MSG_INVALID;
    }
    lmx_msg_exec_lock(rt);
    p = lmx_msg_self_or_find(rt, parent);
    c = lmx_msg_self_or_find(rt, child);
    if (p == 0 || c == 0 || c->parent_msg != p || p->disposed != 0 || c->native_users != 0
        || lmx_msg_running_load(c) != 0 || c->handoff_ready == 0 || c->disposed != 0) {
        lmx_msg_exec_unlock(rt);
        return LMX_MSG_INVALID;
    }
    if (c->init != 0) {
        free(c->init);
        c->init = 0;
        c->init_n = 0U;
    }
    drop_adopted_locked(c);
    c->disposed = 1;
    lmx_msg_exec_unlock(rt);
    return LMX_MSG_OK;
}

/* Decision 17: an orphan's storage is reclaimed at its own end. Nobody adopts
 * an orphan's arena; a failed one's retention has expired by then. */
int lmx_msg_exec_reclaim_mark(LmxMsgRuntime *rt, LmxMsg *m) {
    if (rt == 0 || m == 0) {
        return LMX_MSG_INVALID;
    }
    lmx_msg_exec_lock(rt);
    if (m->init != 0) {
        free(m->init);
        m->init = 0;
        m->init_n = 0U;
    }
    drop_adopted_locked(m);
    m->disposed = 1;
    lmx_msg_exec_unlock(rt);
    return LMX_MSG_OK;
}

int lmx_msg_parent_settle(LmxMsgRuntime *rt, LmxMsgAddr parent) {
    LmxMsg *p;
    LmxMsg *ch;
    LmxMsgAddr *buf;
    LmxMsgAddr *tmp;
    int n = 0;
    int cap = 8;
    int i;
    if (lifecycle_authority(rt, parent) == 0) {
        return LMX_MSG_INVALID;
    }
    buf = (LmxMsgAddr *)malloc((size_t)cap * sizeof(LmxMsgAddr));
    if (buf == 0) {
        return LMX_MSG_NOMEM;
    }
    lmx_msg_exec_lock(rt);
    p = lmx_msg_self_or_find(rt, parent);
    if (p == 0 || p->disposed != 0) {
        lmx_msg_exec_unlock(rt);
        free(buf);
        return LMX_MSG_INVALID;
    }
    ch = p->first_child;
    while (ch != 0) {
        if (n == cap) {
            cap *= 2;
            tmp = (LmxMsgAddr *)realloc(buf, (size_t)cap * sizeof(LmxMsgAddr));
            if (tmp == 0) {
                lmx_msg_exec_unlock(rt);
                free(buf);
                return LMX_MSG_NOMEM;
            }
            buf = tmp;
        }
        buf[n] = ch->addr;
        n += 1;
        ch = ch->next_sibling;
    }
    lmx_msg_exec_unlock(rt);
    for (i = 0; i < n; i++) {
        (void)lmx_msg_adopt_failed(rt, parent, buf[i]);
        (void)lmx_msg_dispose_child(rt, parent, buf[i]);
    }
    free(buf);
    return LMX_MSG_OK;
}

int lmx_msg_set_orphan_until(LmxMsgRuntime *rt, LmxMsgAddr who, unsigned until) {
    LmxMsg *m;
    lmx_msg_exec_lock(rt);
    m = lmx_msg_self_or_find(rt, who);
    if (m == 0) {
        lmx_msg_exec_unlock(rt);
        return LMX_MSG_INVALID;
    }
    m->orphan_until = until;
    lmx_msg_exec_unlock(rt);
    return LMX_MSG_OK;
}

/* Decision 17 (spec 19.29.8): the runtime's failed-orphan retention policy. */
int lmx_msg_set_orphan_retain(LmxMsgRuntime *rt, unsigned retain) {
    if (rt == 0) {
        return LMX_MSG_INVALID;
    }
    lmx_msg_exec_lock(rt);
    rt->orphan_retain = retain;
    lmx_msg_exec_unlock(rt);
    return LMX_MSG_OK;
}

int lmx_msg_orphan_expired(LmxMsgRuntime *rt, LmxMsgAddr who, unsigned now) {
    LmxMsg *m;
    LmxMsg *p;
    int exp = 0;
    lmx_msg_exec_lock(rt);
    m = lmx_msg_self_or_find(rt, who);
    if (m != 0 && m->orphan_until != 0U && now >= m->orphan_until
        && m->native_users == 0 && lmx_msg_success_load(m) == 0 && m->handoff_ready != 0) {
        p = m->parent_msg;
        if (p == 0 || p->state == LMX_MSG_STATE_DEAD || p->state == LMX_MSG_STATE_RELEASED) {
            exp = 1;
        }
    }
    lmx_msg_exec_unlock(rt);
    return exp;
}
