/* Overlapping Message turns. Mutex not held during turn_fn. */
#include "l2src/lmx_message_exec.h"
#include "l2src/lmx_msg_slots.lm1.h"
#include "l2src/lmx_msg_mail_chain.lm1.h"
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
} LmxMsgExecBind;

#if defined(LMX_MSG_EXEC_TEST)
void (*lmx_msg_test_mail_locked)(LmxMsg *m);
void (*lmx_msg_test_after_outbox_xfer)(LmxMsgRuntime *rt, LmxMsg *src, LmxMsgCopy *outb);
void (*lmx_msg_test_after_recv_pin)(LmxMsgRuntime *rt, LmxMsg *m);
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
    /* Decision 18, stage 3d: the UI lane, a Message-shaped mailbox owner
     * created at the first mapping request, outside the family lists and
     * rt->n until stage 5 makes it the root Message's child. Its inbox holds
     * the mapping requests; ui_step, its turn, drains them. */
    LmxMsg *ui_lane;
    LmxMsg *retire_head;
    LmxMsg *retire_tail;
    unsigned wait_serial;
    LmxMsgAddr unbound_held;
#if defined(LMX_MSG_EXEC_TEST)
    int test_fail_ctx;
    int test_fail_adopt_block;
    int test_fail_start_kicks;
    /* The thread that attached the executor (runtime_new's caller): the only
     * thread that may write a scheduler cell outside any turn (lane tripwire). */
#if defined(_WIN32)
    DWORD test_boot_tid;
#else
    pthread_t test_boot_tid;
#endif
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
    if (turn == 0) {
        /* Outside any turn only the bootstrap thread acts, with the parent's
         * authority; a write from any other thread outside a turn has no lane
         * (tripwire, 2026-09-15). A settled owner has no lane either (19.29.6). */
        LmxMsgExec *boot = (LmxMsgExec *)rt->exec;
        if (boot == 0) {
            return;
        }
#if defined(_WIN32)
        if (GetCurrentThreadId() == boot->test_boot_tid) {
            return;
        }
#else
        if (pthread_equal(pthread_self(), boot->test_boot_tid)) {
            return;
        }
#endif
        if (owner->handoff_ready != 0 && lmx_msg_running_load(owner) == 0) {
            return;
        }
        fprintf(stderr, "LANE WRITE FAIL site=%s owner=%u turn=none: a cell written outside any turn off the bootstrap thread (decision 18)\n",
            site, (unsigned)owner->addr);
        fflush(stderr);
        abort();
    }
    if (turn->owner_rt != rt || turn == owner) {
        return;
    }
    /* A settled owner (handoff-safe, not running) has no lane: whoever settles
     * it writes its cells (spec 19.29.6). A parent about to run its child's
     * turn on its own lane (the sequential mapping, 19.28.R2.2) is that
     * child's lane for the take. */
    if (owner->handoff_ready != 0 && lmx_msg_running_load(owner) == 0) {
        return;
    }
    if (owner->parent_msg == turn) {
        return;
    }
    fprintf(stderr, "LANE WRITE FAIL site=%s owner=%u turn=%u: a cell written off its owner's lane (decision 18)\n",
        site, (unsigned)owner->addr, (unsigned)turn->addr);
    fflush(stderr);
    abort();
}

/* The take's lane (19.28.R2.2 (3)): the lane that takes a Message's turn is that
 * Message's lane for the take and clears its ready flag; outside any turn that
 * lane is the thread holding the run claim (held_by). Only take_this calls this. */
static void lmx_msg_test_lane_take(LmxMsgRuntime *rt, LmxMsg *owner,
#if defined(_WIN32)
    DWORD held_by,
#else
    pthread_t held_by,
#endif
    const char *site) {
    if (lmx_msg_test_lane_check == 0 || rt == 0 || owner == 0) {
        return;
    }
    if (lmx_turn_msg == 0) {
#if defined(_WIN32)
        if (held_by == GetCurrentThreadId()) {
            return;
        }
#else
        if (pthread_equal(held_by, pthread_self())) {
            return;
        }
#endif
    }
    lmx_msg_test_lane_write(rt, owner, site);
}

/* The mapping cell's tripwire (19.28.R2.2 (2), 19.29.6): release_slot's unbind is
 * the settling lane's write, refused by construction never; a wrong-lane release
 * that reaches a refusal aborts under the lane check, silent otherwise. */
void lmx_msg_test_unbind_refused(LmxMsgRuntime *rt, LmxMsg *m, int st, const char *site) {
    LmxMsg *turn;
    if (lmx_msg_test_lane_check == 0 || st == LMX_MSG_OK || rt == 0 || m == 0) {
        return;
    }
    turn = lmx_turn_msg;
    fprintf(stderr, "release_slot: unbind refused site=%s owner=%u turn=%u\n",
        site, m->parent_msg != 0 ? (unsigned)m->parent_msg->addr : 0U,
        turn != 0 ? (unsigned)turn->addr : 0U);
    fflush(stderr);
    abort();
}
#else
#define lmx_msg_test_lane_take(r, o, h, s) ((void)0)
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

#if defined(LMX_MSG_EXEC_TEST)
/* Lock removal S2 (design d6/lock-removal 8f20d183): every walk that resolves an
 * address through the family tree is counted by the thread that made it. C1: a
 * turn walking to its own direct child (the parent holds that pointer); C2: a
 * turn walking to itself (the lane holds its own record); turn_other: any other
 * walk inside a turn (sends, liveness, not found); host: outside any turn (R0's
 * management and the embedder's queries). LMX_LOOKUP_COUNT prints the counts at
 * exit; run_port_message never sets it, so parity is unchanged. */
static long test_walk_c1;
static long test_walk_c2;
static long test_walk_turn_other;
static long test_walk_host;
static int test_walk_report_armed;

void lmx_msg_test_on_walk(LmxMsgRuntime *rt, LmxMsg *found) {
    LmxMsg *turn = lmx_turn_msg;
    if (turn == 0 || turn->owner_rt != rt) {
        __sync_fetch_and_add(&test_walk_host, 1);
    } else if (found != 0 && found == turn) {
        __sync_fetch_and_add(&test_walk_c2, 1);
    } else if (found != 0 && found->parent_msg == turn) {
        __sync_fetch_and_add(&test_walk_c1, 1);
    } else {
        __sync_fetch_and_add(&test_walk_turn_other, 1);
    }
}

static void test_walk_report(void) {
    fprintf(stderr, "lookup walks: C1=%ld C2=%ld turn_other=%ld host=%ld\n",
        test_walk_c1, test_walk_c2, test_walk_turn_other, test_walk_host);
    fflush(stderr);
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

/* Stage 5 (e): the clock is R0's management state, read from R0's policy record
 * (a late read changes nothing); the logical clock once lmx_msg_set_now set it. */
unsigned lmx_msg_now(LmxMsgRuntime *rt) {
    if (rt == 0) {
        return 0U;
    }
    if (lmx_root_record_clock_test(rt->root_record) != 0) {
        return lmx_root_record_clock(rt->root_record);
    }
#if defined(_WIN32)
    return GetTickCount();
#else
    return lmx_root_record_clock(rt->root_record);
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

/* Stage 5 (b): internal kinds are never a Message's input. INGRESS is the one that
 * can sit in a Message's inbox (STOP is consumed at admission, MAP lives in the UI
 * lane); the other two are named so the rule reads as one rule. */
static int mail_kind_internal(int kind) {
    return kind == LMX_MSG_KIND_INGRESS || kind == LMX_MSG_KIND_MAP || kind == LMX_MSG_KIND_STOP;
}

int lmx_msg_mail_inbox_has_input(LmxMsg *m) {
    LmxMsgCopy *n;
    int found = 0;
    if (m == 0) {
        return 0;
    }
    lmx_msg_mail_lock(m);
    for (n = m->inbox; n != 0 && found == 0; n = n->next) {
        found = mail_kind_internal(n->kind) == 0;
    }
    lmx_msg_mail_unlock(m);
    return found;
}

LmxMsgCopy *lmx_msg_mail_inbox_pop_input(LmxMsg *m) {
    LmxMsgCopy *prev = 0;
    LmxMsgCopy *n;
    if (m == 0) {
        return 0;
    }
    lmx_msg_mail_lock(m);
    n = m->inbox;
    while (n != 0 && mail_kind_internal(n->kind) != 0) {
        prev = n;
        n = n->next;
    }
    if (n != 0) {
        if (prev == 0) {
            m->inbox = n->next;
        } else {
            prev->next = n->next;
        }
        if (m->inbox_tail == n) {
            m->inbox_tail = prev;
        }
        n->next = 0;
    }
    lmx_msg_mail_unlock(m);
    return n;
}

void lmx_msg_mail_inbox_take_ingress(LmxMsg *m, LmxMsgCopy **out) {
    LmxMsgCopy *prev = 0;
    LmxMsgCopy *n;
    LmxMsgCopy *nxt;
    LmxMsgCopy *head = 0;
    LmxMsgCopy *tail = 0;
    if (out == 0) {
        return;
    }
    *out = 0;
    if (m == 0) {
        return;
    }
    lmx_msg_mail_lock(m);
    n = m->inbox;
    while (n != 0) {
        nxt = n->next;
        if (n->kind == LMX_MSG_KIND_INGRESS) {
            if (prev == 0) {
                m->inbox = nxt;
            } else {
                prev->next = nxt;
            }
            if (m->inbox_tail == n) {
                m->inbox_tail = prev;
            }
            n->next = 0;
            if (tail == 0) {
                head = n;
            } else {
                tail->next = n;
            }
            tail = n;
        } else {
            prev = n;
        }
        n = nxt;
    }
    lmx_msg_mail_unlock(m);
    *out = head;
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
        || m->first_child != 0) {
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
    e->test_boot_tid = lmx_tid();
    if (test_walk_report_armed == 0 && getenv("LMX_LOOKUP_COUNT") != 0) {
        test_walk_report_armed = 1;
        atexit(test_walk_report);
    }
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
    /* Stage 5 (d2): the UI lane is R0's child, a slot of the runtime; runtime_delete's
     * slot loop has already freed it and its MAP requests. */
    e->ui_lane = 0;
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

/* The thread's one turn identity (model 29) is lmx_turn_msg with lmx_turn_running,
 * shared by every runtime on the thread, while the held turn is per exec (e->tls).
 * A nested turn restores the exact identity it replaced, not one recomputed from its
 * own exec's old TLS: an R0 turn of another runtime run from inside a turn on this
 * thread (a library unit opened from a program's turn, stage 5 (a)) leaves the outer
 * turn the thread's turn again. */
static void restore_turn(LmxMsgExec *e, LmxMsgAddr old, LmxMsg *old_msg, uint_fast8_t *old_running) {
    set_tls(e, old);
    lmx_turn_msg = old_msg;
    lmx_turn_running = old_running;
}



/* Decision 18: the walk over every bound record, in family-tree order from
 * rt->root (iterative: first_child, else next_sibling, else climb parent_msg).
 * A record is its Message's own (exec_bind); a bound Message is always in its
 * family tree (3b-8) and an orphan is a root, so the walk meets every record.
 * A nonzero visitor result stops the walk and is returned. Caller holds the exec
 * lock; a visitor must not change the tree or drop the lock. */
typedef int (*LmxRecVisit)(LmxMsgExec *e, LmxMsgExecBind *rec, void *arg);

static int rec_walk_locked(LmxMsgExec *e, LmxRecVisit visit, void *arg) {
    LmxMsg *m;
    LmxMsgExecBind *rec;
    int st;
    if (e == 0 || e->rt == 0) {
        return 0;
    }
    m = e->rt->root;
    while (m != 0) {
        rec = bind_rec_locked(m);
        if (rec != 0) {
            st = visit(e, rec, arg);
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

static int ctx_visit_wake(LmxMsgExec *e, LmxMsgExecBind *rec, void *arg) {
    (void)e;
    (void)arg;
    if (rec->affinity != LMX_MSG_AFFINITY_UI) {
        bind_wait_signal(rec->wait);
    }
    return 0;
}

/* Stage 3b-7d: the number of bound records. */
static int ctx_visit_count(LmxMsgExec *e, LmxMsgExecBind *rec, void *arg) {
    (void)e;
    (void)rec;
    *(int *)arg += 1;
    return 0;
}

void lmx_msg_exec_wake_locked(LmxMsgRuntime *rt) {
    LmxMsgExec *e = exof(rt);
    if (e == 0) {
        return;
    }
    (void)rec_walk_locked(e, ctx_visit_wake, 0);
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

/* Stage 5 (d2): runtime_new hands the executor the UI lane it created as R0's child. */
void lmx_msg_exec_set_ui_lane(LmxMsgRuntime *rt, LmxMsg *lane) {
    LmxMsgExec *e = exof(rt);
    if (e == 0) {
        return;
    }
    lmx_msg_exec_lock(rt);
    e->ui_lane = lane;
    lmx_msg_exec_unlock(rt);
}

int lmx_msg_exec_has_ui_lane(LmxMsgRuntime *rt) {
    LmxMsgExec *e = exof(rt);
    int has;
    if (e == 0) {
        return 0;
    }
    lmx_msg_exec_lock(rt);
    has = e->ui_lane != 0;
    lmx_msg_exec_unlock(rt);
    return has;
}

/* Decision 18, stage 3d: a mapping request for a UI-mapped Message, admitted
 * into the UI lane's inbox under its mail lock (a mailbox admission, class 4).
 * The writer of the Message's readiness sends it, once per request
 * (ui_pending). The node is an internal control envelope (LMX_MSG_KIND_MAP),
 * never admitted to a handler. The lane is created at the first request.
 * Caller holds the exec lock. */
int lmx_msg_exec_ui_request_locked(LmxMsgRuntime *rt, LmxMsgAddr addr) {
    LmxMsgExec *e = exof(rt);
    LmxMsg *lane;
    LmxMsgCopy *node;
    if (e == 0 || addr == 0U) {
        return LMX_MSG_INVALID;
    }
    lane = e->ui_lane;
    node = (LmxMsgCopy *)calloc(1U, sizeof(LmxMsgCopy));
    if (node == 0) {
        return LMX_MSG_NOMEM;
    }
    node->kind = LMX_MSG_KIND_MAP;
    node->to = addr;
    lmx_msg_mail_lock(lane);
    if (lane->inbox_tail != 0) {
        lane->inbox_tail->next = node;
    } else {
        lane->inbox = node;
    }
    lane->inbox_tail = node;
    lmx_msg_mail_unlock(lane);
    return LMX_MSG_OK;
}

/* Stage 3d: a UI-mapped Message that is ready with no request outstanding
 * sends one (the failed UI->ANY launch path, whose request the lane may have
 * dropped while the Message was launching). Caller holds the exec lock. */
static void ui_request_if_ready_locked(LmxMsgRuntime *rt, LmxMsg *m) {
    LmxMsgExecBind *r = bind_rec_locked(m);
    if (r == 0 || r->affinity != LMX_MSG_AFFINITY_UI || m->ready == 0 || m->ui_pending != 0) {
        return;
    }
    m->ui_pending = 1;
    if (lmx_msg_exec_ui_request_locked(rt, m->addr) != LMX_MSG_OK) {
        m->ui_pending = 0;
    }
}

/* Decision 18, stage 3d: the UI lane's take. It drains the lane's inbox in
 * admission order. Each request clears its Message's ui_pending (the taking
 * lane); a request whose Message is gone, no longer bound to UI, held,
 * launching or not ready is dropped (its next readiness sends again), and a
 * ready Message that is no longer runnable has its flag cleared. The first
 * eligible Message is held, its ready flag cleared, and its turn taken. No
 * parent's cells are written. Caller holds the exec lock. */
unsigned lmx_msg_exec_take_ui_locked(LmxMsgRuntime *rt) {
    LmxMsgExec *e = exof(rt);
    LmxMsg *lane;
    LmxMsg *m;
    LmxMsgCopy *node;
    LmxMsgExecBind *rj;
    LmxMsgAddr addr;
    if (e == 0 || e->ui_lane == 0) {
        return 0U;
    }
    lane = e->ui_lane;
    for (;;) {
        lmx_msg_mail_lock(lane);
        node = lane->inbox;
        if (node != 0) {
            lane->inbox = node->next;
            if (lane->inbox == 0) {
                lane->inbox_tail = 0;
            }
            node->next = 0;
        }
        lmx_msg_mail_unlock(lane);
        if (node == 0) {
            return 0U;
        }
        addr = node->to;
        free(node);
        m = msg_at_addr(rt, addr);
        if (m == 0) {
            continue;
        }
        /* Stage 5 (d2b), decision 18 class 3: the take's writes on the served
         * Message are the UI lane's (the taking lane, R0's child mapped to R0's
         * lane), so the oracle is passed the writer, not the written Message. */
        lmx_msg_test_lane_write(rt, e->ui_lane, "take_ui:pending_clear");
        m->ui_pending = 0;
        rj = bind_rec_locked(m);
        if (rj == 0 || rj->gone != 0 || rj->affinity != LMX_MSG_AFFINITY_UI || m->ready == 0
            || rj->held != 0 || rj->launching != 0) {
            continue;
        }
        if (lmx_msg_exec_is_runnable_locked(rt, addr) == 0) {
            lmx_msg_test_lane_write(rt, e->ui_lane, "take_ui:stale_clear");
            m->ready = 0;
            continue;
        }
        rj->held = 1;
        rj->held_by = lmx_tid();
        lmx_msg_test_lane_write(rt, e->ui_lane, "take_ui:ready_clear");
        m->ready = 0;
        return addr;
    }
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



/* Decision 18: whether a bound non-UI Message's own ready flag is set. */
int lmx_msg_exec_map_queued(LmxMsgRuntime *rt, LmxMsgAddr addr) {
    LmxMsg *m;
    LmxMsgExecBind *r;
    int q = 0;
    if (rt == 0 || addr == 0U) {
        return 0;
    }
    lmx_msg_exec_lock(rt);
    m = msg_at_addr(rt, addr);
    r = bind_rec_locked(m);
    if (r != 0 && r->affinity != LMX_MSG_AFFINITY_UI) {
        q = m->ready;
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

/* Stage 3d: the number of mapping requests in the UI lane's inbox. */
int lmx_msg_exec_ui_nrequests(LmxMsgRuntime *rt) {
    LmxMsgExec *e = exof(rt);
    LmxMsgCopy *node;
    int n = 0;
    if (e == 0) {
        return 0;
    }
    lmx_msg_exec_lock(rt);
    if (e->ui_lane != 0) {
        lmx_msg_mail_lock(e->ui_lane);
        for (node = e->ui_lane->inbox; node != 0; node = node->next) {
            n += 1;
        }
        lmx_msg_mail_unlock(e->ui_lane);
    }
    lmx_msg_exec_unlock(rt);
    return n;
}




int lmx_msg_exec_bind_n(LmxMsgRuntime *rt) {
    LmxMsgExec *e = exof(rt);
    int n = 0;
    if (e == 0) {
        return 0;
    }
    lmx_msg_exec_lock(rt);
    (void)rec_walk_locked(e, ctx_visit_count, &n);
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


#endif

/* Stage 3a-2: the Message's own record while it is a counted table entry,
 * else 0. Caller holds the exec lock. */

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
            lmx_msg_test_on_walk(rt, m);
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
    lmx_msg_test_on_walk(rt, 0);
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
    old = rec->msg;
    rec->msg = 0;
    if (old != 0) {
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
    if (lmx_msg_mail_inbox_has_input(m) != 0) {
        return 1;
    }
    if (m->closing != 0) {
        return 1;
    }
    return 0;
}

/* The execution mapping is the parent's cell about its direct child (19.28.R2.2 (2)).
 * Its writers: the host outside any turn; the turn of m's parent; or, when that parent
 * is settled (handoff-ready, not running: no lane of its own), the turn of the nearest
 * unsettled ancestor, the lane that settles it (19.29.6). A root or an orphan has no
 * parent: the host only. A Message never maps itself. Exec lock held. */
static int mapping_authority_locked(LmxMsgRuntime *rt, LmxMsg *m) {
    LmxMsg *a;
    if (lmx_msg_host_is_owner(rt) != 0 && lmx_msg_exec_holding_any(rt) == 0) {
        return 1;
    }
    a = m != 0 ? m->parent_msg : 0;
    while (a != 0 && a->handoff_ready != 0 && lmx_msg_running_load(a) == 0) {
        a = a->parent_msg;
    }
    return a != 0 && lmx_msg_exec_holding_turn(rt, a->addr) != 0;
}

/* Stage 5 (d1b): launch = 0 binds without starting a context worker (the
 * host-sequential mapping); run_entry_turn and root_turn bind that way, so a
 * runtime with contexts started gains no worker for their turn. */
static int exec_bind_mode(LmxMsgRuntime *rt, LmxMsgAddr addr, LmxMsgTurn turn, void *ctx, int affinity, int launch) {
    LmxMsgExec *e = exof(rt);
    LmxMsg *m;
    LmxMsgExecBind *rec;
    int owner;
    int live;
    int kick = 0;
    if (e == 0 || addr == 0U || turn == 0) {
        return LMX_MSG_INVALID;
    }
    owner = lmx_msg_host_is_owner(rt) != 0 && lmx_msg_exec_holding_any(rt) == 0;
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
    if (mapping_authority_locked(rt, m) == 0) {
        lmx_msg_exec_unlock(rt);
        return LMX_MSG_INVALID;
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
                lmx_msg_test_lane_write(rt, m->parent_msg, "bind:affinity");
                rec->affinity = LMX_MSG_AFFINITY_UI;
                join_bind_worker(rt, rec);
                lmx_msg_exec_unlock(rt);
                lmx_msg_exec_flush_retire(rt);
                if (kick != 0) {
                    lmx_msg_exec_ready(rt, addr);
                }
                return LMX_MSG_OK;
            }
            /* UI->ANY: the ready flag is the Message's own and survives a failed launch. */
            need = launch != 0 && e->contexts_live != 0 && bind_has_worker(rec) == 0;
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
                lmx_msg_test_lane_write(rt, m->parent_msg, "bind:affinity");
                rec->affinity = affinity;
                bind_wait_launch_hold_locked(cap);
                lmx_msg_exec_unlock(rt);
                st = launch_ctx_thread(rt, addr);
                lmx_msg_exec_lock(rt);
                rec = rec_at_addr_locked(e, addr);
                if (st != LMX_MSG_OK) {
                    if (rec != 0 && launch_same_gen(rec, cap, gen) != 0
                        && bind_has_worker(rec) == 0) {
                        lmx_msg_test_lane_write(rt, rec->msg != 0 ? rec->msg->parent_msg : 0, "bind:affinity");
                        rec->affinity = old_aff;
                        rec->launching = 0;
                        ui_request_if_ready_locked(rt, rec->msg);
                    }
                    lmx_msg_exec_unlock(rt);
                    bind_wait_launch_release(rt, cap);
                    lmx_msg_exec_flush_retire(rt);
                    return st;
                }
                if (rec != 0 && launch_same_gen(rec, cap, gen) != 0) {
                    rec->launching = 0;
                }
                lmx_msg_exec_unlock(rt);
                bind_wait_launch_release(rt, cap);
                lmx_msg_exec_flush_retire(rt);
                if (kick != 0) {
                    lmx_msg_exec_ready(rt, addr);
                }
                return LMX_MSG_OK;
            }
            lmx_msg_test_lane_write(rt, m->parent_msg, "bind:affinity");
            rec->affinity = affinity;
            lmx_msg_exec_unlock(rt);
            lmx_msg_exec_flush_retire(rt);
            if (kick != 0) {
                lmx_msg_exec_ready(rt, addr);
            }
            return LMX_MSG_OK;
        }
        need = launch != 0 && e->contexts_live != 0 && affinity != LMX_MSG_AFFINITY_UI && bind_has_worker(rec) == 0;
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
    lmx_msg_test_lane_write(rt, m->parent_msg, "bind:affinity");
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
    live = launch != 0 ? e->contexts_live : 0;
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

int lmx_msg_exec_bind(LmxMsgRuntime *rt, LmxMsgAddr addr, LmxMsgTurn turn, void *ctx, int affinity) {
    return exec_bind_mode(rt, addr, turn, ctx, affinity, 1);
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
    LmxMsg *old_msg;
    uint_fast8_t *old_running;
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
    old_msg = lmx_turn_msg;
    old_running = lmx_turn_running;
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
        restore_turn(e, old, old_msg, old_running);
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
        restore_turn(e, old, old_msg, old_running);
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
    restore_turn(e, old, old_msg, old_running);
    native_leave_addr(rt, snap->addr);
    requeue_if_runnable(rt, snap->addr);
    return st;
}

/* Stage 3b-7b: exec_start_map_kick's collection, over the bound records. */
typedef struct CtxKickCollect {
    LmxMsgAddr *kicks;
    LmxMsg **pars;
    LmxMsg **chs;
    int nk;
    int nu;
    int st;
} CtxKickCollect;

static int ctx_visit_kick(LmxMsgExec *e, LmxMsgExecBind *rec, void *arg) {
    CtxKickCollect *k = (CtxKickCollect *)arg;
    LmxMsg *cm = rec->msg;
    LmxMsg *par;
    (void)e;
    if (cm == 0 || cm->mapped != 0 || rec->addr == 0U) {
        return 0;
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
    (void)rec_walk_locked(e, ctx_visit_count, &nrec);
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
                (void)rec_walk_locked(e, ctx_visit_kick, &kc);
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
    if (lmx_msg_mail_inbox_has_input(m) == 0 && m->closing == 0) {
        return 0;
    }
    r->held = 1;
    r->held_by = lmx_tid();
    lmx_msg_test_lane_take(m->owner_rt, m, r->held_by, "take_this:ready_clear");
    m->ready = 0;
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
static int ctx_visit_first_launchable(LmxMsgExec *e, LmxMsgExecBind *rec, void *arg) {
    (void)e;
    if (rec->affinity != LMX_MSG_AFFINITY_UI && rec->gone == 0 && bind_has_worker(rec) == 0) {
        *(LmxMsgAddr *)arg = rec->addr;
        return 1;
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
        (void)rec_walk_locked(e, ctx_visit_first_launchable, &addr);
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
    /* Stage 5 (d3): the UI lane is R0's child, so its step is R0's act, run only
     * from R0's turn; the host outside any turn steps nothing. */
    if (rt->root == 0 || lmx_msg_exec_holding_turn(rt, rt->root->addr) == 0) {
        return LMX_MSG_INVALID;
    }
    /* Stage 5 (d2b): the UI step only takes; the drain is the host's maintenance
     * between turns (it refuses inside a turn). */
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
    LmxMsg *old_msg;
    uint_fast8_t *old_running;
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
    old_msg = lmx_turn_msg;
    old_running = lmx_turn_running;
    set_tls(e, addr);
    lmx_msg_exec_unlock(rt);
    st = lmx_msg_end_turn(rt, addr, 1);
    lmx_msg_exec_lock(rt);
    e->unbound_held = 0;
    restore_turn(e, old, old_msg, old_running);
    /* Stage 5 (d1d), decision 17 rule 1: the end-turn above is the bookkeeping of
     * a Message with no lane, written by the maintaining lane (drive); the borrowed
     * TLS identity is that bookkeeping's spelling, not a turn, and no handler runs.
     * It ends at run_one's boundary, so the closed Message is handoff-ready and its
     * parent's dispose or adopt settles it. */
    {
        LmxMsg *m = msg_at_addr(rt, addr);
        if (m != 0) {
            if (lmx_msg_success_load(m) != 0) {
                lmx_msg_running_store(m, 0);
            }
            if (lmx_msg_running_load(m) == 0 && m->native_users == 0) {
                m->handoff_ready = 1;
            }
        }
    }
    lmx_msg_exec_unlock(rt);
    return st;
}

/* Stage 3b-7b: lmx_msg_exec_stop's passes over the owners' lists. */
static int ctx_visit_stop_unmap(LmxMsgExec *e, LmxMsgExecBind *rec, void *arg) {
    (void)e;
    (void)arg;
    if (rec->msg != 0) {
        lmx_msg_test_lane_write(e->rt, rec->msg, "stop_unmap:ready_clear");
        rec->msg->ready = 0;
        rec->msg->mapped = 0;
    }
    return 0;
}

static int ctx_visit_stop_retire_waits(LmxMsgExec *e, LmxMsgExecBind *rec, void *arg) {
    (void)arg;
    if (rec->wait != 0) {
        rec->wait->retired = 1;
        bind_wait_signal(rec->wait);
        bind_reap_push(e, rec->wait);
        rec->wait = 0;
    }
    return 0;
}

static int ctx_visit_stop_reset(LmxMsgExec *e, LmxMsgExecBind *rec, void *arg) {
    (void)e;
    (void)arg;
    rec->held = 0;
    rec->held_by = 0;
    rec->launching = 0;
    rec->gone = 0;
    if (rec->msg != 0) {
        lmx_msg_test_lane_write(e->rt, rec->msg, "stop_reset:ready_clear");
        rec->msg->ready = 0;
        rec->msg->mapped = 0;
    }
    return 0;
}

int lmx_msg_exec_stop(LmxMsgRuntime *rt) {
    LmxMsgExec *e = exof(rt);
    LmxMsg *old_msg;
    uint_fast8_t *old_running;
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
    (void)rec_walk_locked(e, ctx_visit_stop_unmap, 0);
    (void)rec_walk_locked(e, ctx_visit_stop_retire_waits, 0);
#if defined(_WIN32)
    SetEvent(e->stop_ev);
#endif
    lmx_msg_exec_unlock(rt);
    lmx_msg_exec_flush_retire(rt);
    bind_reap_join_all(rt);
    e->nworkers = 0;
    lmx_msg_exec_lock(rt);
    /* The stop clears this exec's TLS and restores the thread's turn identity it
     * replaced: a stop run from inside another runtime's turn on this thread leaves
     * that turn the thread's turn (restore_turn). */
    old_msg = lmx_turn_msg;
    old_running = lmx_turn_running;
    restore_turn(e, 0, old_msg, old_running);
    (void)rec_walk_locked(e, ctx_visit_stop_reset, 0);
    e->unbound_held = 0;
    e->stopped = 1;
    lmx_msg_exec_unlock(rt);
    lmx_msg_exec_flush_retire(rt);
    return LMX_MSG_OK;
}

/* Stage 3b-7b: drop_binds takes the first record the walk meets, unbinds it and
 * restarts until the walk finds none (no allocation on the teardown path). */
static int ctx_visit_first_record(LmxMsgExec *e, LmxMsgExecBind *rec, void *arg) {
    (void)e;
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
        (void)rec_walk_locked(e, ctx_visit_first_record, &rec);
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
    LmxMsg *m;
    if (e == 0 || addr == 0U) {
        return LMX_MSG_INVALID;
    }
    lmx_msg_exec_lock(rt);
    /* The binding is the parent's mapping cell whichever way it is written: the same
     * writers as bind (a Message never unbinds itself). */
    m = msg_at_addr(rt, addr);
    if (mapping_authority_locked(rt, m) == 0) {
        lmx_msg_exec_unlock(rt);
        return LMX_MSG_INVALID;
    }
    r = rec_at_addr_locked(e, addr);
    if (r == 0) {
        lmx_msg_exec_unlock(rt);
        return LMX_MSG_OK;
    }
    lmx_msg_test_lane_write(rt, m != 0 ? m->parent_msg : 0, "unbind:record");
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

/* Stage 5 (d3), 19.28.R2.2 and model 29: one turn of a bound, unmapped child on
 * this thread (claim its record, run, release). A step is its parent's act on the
 * parent's lane: the child's parent turn must be held. R0 has no parent; its turn
 * is started only by the bootstrap, run_entry_turn, which has made its own checks
 * and passes bootstrap = 1. */
static int child_turn_core(LmxMsgRuntime *rt, LmxMsgAddr child, int bootstrap) {
    LmxMsg *m;
    LmxMsgExecBind snap;
    int st;
    LmxMsgExecBind *rec;
    lmx_msg_exec_lock(rt);
    m = msg_at_addr(rt, child);
    if (m == 0 || m->turn == 0 || m->mapped != 0) {
        lmx_msg_exec_unlock(rt);
        return LMX_MSG_INVALID;
    }
    if (bootstrap == 0 && (m->parent == 0U || lmx_msg_exec_holding_turn(rt, m->parent) == 0)) {
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
    lmx_msg_test_lane_write(rt, m, "run_child_turn:ready_clear");
    m->ready = 0;
    memset(&snap, 0, sizeof(snap));
    snap.addr = child;
    snap.turn = m->turn;
    snap.ctx = m->turn_ctx;
    lmx_msg_exec_unlock(rt);
    st = run_one(rt, &snap);
    /* Stage 5 (d1c), spec 19.29.6 (i)-(ii): the step settles none of the parent's
     * other children; settling is the parent's dispose or adopt. Stage 5 (c): an
     * orphan settled by this turn is reclaimed by the root's next maintenance
     * (lmx_msg_drive's sweep), on both paths, not here. */
    return st == 0 ? LMX_MSG_OK : st;
}

int lmx_msg_run_child_turn(LmxMsgRuntime *rt, LmxMsgAddr child) {
    if (exof(rt) == 0 || child == 0U) {
        return LMX_MSG_INVALID;
    }
    return child_turn_core(rt, child, 0);
}

/* Stage 5 step (a): the bootstrap of a process entry. On the host thread outside
 * any turn, bind addr to turn, run exactly one turn of it on this thread, and
 * unbind; returns the run's status. Stage 5 (d1): addr is R0 or an unbound direct
 * child of R0, the former parent-0 set; a deeper Message is refused. */
int lmx_msg_run_entry_turn(LmxMsgRuntime *rt, LmxMsgAddr addr, LmxMsgTurn turn, void *ctx) {
    LmxMsg *m;
    int top = 0;
    int st;
    if (exof(rt) == 0 || addr == 0U || turn == 0 || lmx_msg_host_is_owner(rt) == 0
        || lmx_msg_exec_holding_any(rt) != 0 || lmx_msg_exec_is_bound(rt, addr) != 0) {
        return LMX_MSG_INVALID;
    }
    lmx_msg_exec_lock(rt);
    m = msg_at_addr(rt, addr);
    if (m != 0) {
        top = m == rt->root || (rt->root != 0 && m->parent_msg == rt->root);
    }
    lmx_msg_exec_unlock(rt);
    if (m == 0 || top == 0) {
        return LMX_MSG_INVALID;
    }
    st = exec_bind_mode(rt, addr, turn, ctx, LMX_MSG_AFFINITY_ANY, 0);
    if (st != LMX_MSG_OK) {
        return st;
    }
    st = child_turn_core(rt, addr, 1);
    (void)lmx_msg_exec_unbind(rt, addr);
    return st;
}

/* Stage 5 (d1): exactly one turn of the runtime's root Message R0 on this thread,
 * bound and unbound around it; R0 is not ended. The helper the tests migrate to
 * until R0's own loop exists. */
int lmx_msg_root_turn(LmxMsgRuntime *rt, LmxMsgTurn turn, void *ctx) {
    if (rt == 0 || rt->root == 0) {
        return LMX_MSG_INVALID;
    }
    return lmx_msg_run_entry_turn(rt, rt->root->addr, turn, ctx);
}

int lmx_msg_map_child(LmxMsgRuntime *rt, LmxMsgAddr parent, LmxMsgAddr child) {
    LmxMsgExec *e = exof(rt);
    LmxMsg *p;
    LmxMsg *c;
    int st;
    if (e == 0 || parent == 0U || child == 0U) {
        return LMX_MSG_INVALID;
    }
    /* Stage 5 (d3), 19.28.R2.2: mapping a child to a context is its parent's act,
     * run only from the parent's own turn. */
    if (lmx_msg_exec_holding_turn(rt, parent) == 0) {
        return LMX_MSG_INVALID;
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
            /* Nothing fallible follows the move: lmx_msg_blocks_push refuses only a
             * malformed node, and prepared is a fresh detached block with a base. */
            (void)lmx_msg_blocks_push(&p->blocks, prepared);
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
        /* Stage 5 (d1b), spec 19.29.8: an orphan under R0 is settled only by the
         * sweep under the retention policy; the parent's settle neither adopts
         * nor disposes it. */
        if (ch->orphan != 0) {
            ch = ch->next_sibling;
            continue;
        }
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

/* Decision 17 (spec 19.29.8): the runtime's failed-orphan retention policy.
 * Stage 5 (e): it is R0's management state, written only on R0's lane (R0's own
 * turn, or until (f) the host outside any turn); refused, nothing is written. */
int lmx_msg_set_orphan_retain(LmxMsgRuntime *rt, unsigned retain) {
    int st;
    if (rt == 0 || rt->root == 0) {
        return LMX_MSG_INVALID;
    }
    if (lmx_msg_exec_holding_turn(rt, rt->root->addr) == 0
        && (lmx_msg_host_is_owner(rt) == 0 || lmx_msg_exec_holding_any(rt) != 0)) {
        return LMX_MSG_INVALID;
    }
    /* Created at the first set on R0's lane, as a parent's scheduler record is at
     * its first step: runtime_new cannot call the generated unit, whose library
     * open creates a runtime of its own. */
    if (rt->root_record == 0) {
        rt->root_record = lmx_root_record_new(rt->root);
        if (rt->root_record == 0) {
            return LMX_MSG_NOMEM;
        }
    }
    lmx_msg_exec_lock(rt);
    st = lmx_root_record_set_orphan_retain(rt->root_record, retain);
    lmx_msg_exec_unlock(rt);
    return st;
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
