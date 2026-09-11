/* Overlapping Message turns. Mutex not held during turn_fn. */
#include "l2src/lmx_message_exec.h"
#include "l2src/lmx_msg_slots.lm1.h"
#include "l2src/lmx_msg_mail_chain.lm1.h"
#include "l2src/lmx_msg_sched_ready.lm1.h"
#include "l2src/lmx_message_host.h"
#include "l2src/lmx.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
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
#if defined(_WIN32)
    HANDLE wait_ev;
    HANDLE worker;
#else
    int wait_sig;
#endif
} LmxMsgExecBind;

#if defined(LMX_MSG_EXEC_TEST)
void (*lmx_msg_exec_test_after_cleanup)(LmxMsgAddr who, int live, int st);
void (*lmx_msg_exec_test_after_bind_add)(LmxMsgRuntime *rt);
#endif

typedef struct LmxMsgExec {
#if defined(_WIN32)
    CRITICAL_SECTION lock;
    HANDLE ready_ev;
    HANDLE stop_ev;
    HANDLE *wh;
    DWORD tls;
#else
    pthread_mutex_t lock;
    pthread_cond_t ready_cv;
    pthread_t *wh;
    pthread_key_t tls;
    int ready_sig;
#endif
    int nworkers;
    int workers_cap;
    int stopping;
    int stopped;
    int no_retire;
    int contexts_live;
    LmxMsgRuntime *rt;
    LmxMsgExecBind *bind;
    int nbind;
    int bind_cap;
    LmxMsgAddr *ready;
    int nready;
    int ready_cap;
    int scan;
    LmxMsgAddr unbound_held;
    int test_fail_grow;
    int test_fail_hits;
#if defined(LMX_MSG_EXEC_TEST)
    int test_fail_ctx;
    int test_fail_adopt_block;
#endif
} LmxMsgExec;

static int launch_ctx_thread(LmxMsgRuntime *rt, LmxMsgAddr addr);
#if defined(_WIN32)
static DWORD WINAPI context_worker(void *arg);
#endif
static int bind_index(LmxMsgExec *e, LmxMsgAddr addr);
static int bind_has_worker(const LmxMsgExecBind *b);
static LmxMsg *msg_at_addr(LmxMsgRuntime *rt, LmxMsgAddr addr);

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

int lmx_msg_endp_retain(LmxMsg *m) {
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
#if defined(_WIN32)
    if (m != 0 && m->mail != 0) {
        EnterCriticalSection((CRITICAL_SECTION *)m->mail);
    }
#else
    (void)m;
#endif
}

void lmx_msg_mail_unlock(LmxMsg *m) {
#if defined(_WIN32)
    if (m != 0 && m->mail != 0) {
        LeaveCriticalSection((CRITICAL_SECTION *)m->mail);
    }
#else
    (void)m;
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

void lmx_msg_sched_unlink_child(LmxMsg *parent, LmxMsg *child) {
    if (parent == 0 || child == 0) {
        return;
    }
    lmx_msg_mail_lock(parent);
    lmx_msg_sched_ready_unlink(&parent->sched_ready, &parent->sched_ready_tail, child);
    lmx_msg_mail_unlock(parent);
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
    if (dst == 0 || src == 0 || dst == src) {
        return LMX_MSG_INVALID;
    }
    if (lmx_msg_storage_can_move(&dst->blocks, &dst->ranges, &src->blocks, &src->ranges)
        != LMX_MSG_STORAGE_OK) {
        return LMX_MSG_INVALID;
    }
    if (lmx_msg_storage_move_all(&dst->blocks, &dst->ranges, &src->blocks, &src->ranges)
        != LMX_MSG_STORAGE_OK) {
        return LMX_MSG_INVALID;
    }
    return LMX_MSG_OK;
}

void lmx_msg_slot_free(LmxMsg *m) {
    if (m == 0) {
        return;
    }
    drop_ranges_locked(m);
    (void)lmx_msg_blocks_dispose_all(&m->blocks);
#if defined(_WIN32)
    if (m->mail != 0) {
        DeleteCriticalSection((CRITICAL_SECTION *)m->mail);
        free(m->mail);
        m->mail = 0;
    }
#endif
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
    if (lmx_msg_mail_inbox_empty(m) == 0 || lmx_msg_mail_outbox_empty(m) == 0 || m->parent_msg != 0) {
        lmx_msg_exec_unlock(rt);
        return 0;
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
#if defined(_WIN32)
    InitializeCriticalSection(&e->lock);
    e->ready_ev = CreateEventA(0, 0, 0, 0);
    e->stop_ev = CreateEventA(0, 1, 0, 0);
    e->tls = TlsAlloc();
    if (e->ready_ev == 0 || e->stop_ev == 0 || e->tls == TLS_OUT_OF_INDEXES) {
        if (e->ready_ev) CloseHandle(e->ready_ev);
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
    pthread_cond_init(&e->ready_cv, 0);
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
#if defined(_WIN32)
    if (e->bind != 0) {
        int bi;
        for (bi = 0; bi < e->nbind; bi++) {
            if (e->bind[bi].wait_ev != 0) {
                CloseHandle(e->bind[bi].wait_ev);
                e->bind[bi].wait_ev = 0;
            }
        }
    }
#endif
    free(e->bind);
    free(e->ready);
#if defined(_WIN32)
    CloseHandle(e->ready_ev);
    CloseHandle(e->stop_ev);
    TlsFree(e->tls);
    DeleteCriticalSection(&e->lock);
#else
    pthread_cond_destroy(&e->ready_cv);
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

static int ready_grow(LmxMsgExec *e) {
    int cap;
    LmxMsgAddr *p;
    if (e->nready < e->ready_cap) {
        return 0;
    }
    if (e->test_fail_grow != 0) {
        e->test_fail_hits += 1;
        return 1;
    }
    cap = e->ready_cap == 0 ? 8 : e->ready_cap * 2;
    p = (LmxMsgAddr *)realloc(e->ready, (size_t)cap * sizeof(LmxMsgAddr));
    if (p == 0) {
        return 1;
    }
    e->ready = p;
    e->ready_cap = cap;
    return 0;
}

static int bind_grow(LmxMsgExec *e) {
    int cap;
    LmxMsgExecBind *p;
    if (e->nbind < e->bind_cap) {
        return 0;
    }
    cap = e->bind_cap == 0 ? 8 : e->bind_cap * 2;
    p = (LmxMsgExecBind *)realloc(e->bind, (size_t)cap * sizeof(LmxMsgExecBind));
    if (p == 0) {
        return 1;
    }
    e->bind = p;
    e->bind_cap = cap;
    return 0;
}

int lmx_msg_exec_ready_has_locked(LmxMsgRuntime *rt, LmxMsgAddr addr) {
    LmxMsgExec *e = exof(rt);
    int i;
    if (e == 0) {
        return 0;
    }
    for (i = 0; i < e->nready; i++) {
        if (e->ready[i] == addr) {
            return 1;
        }
    }
    return 0;
}

int lmx_msg_exec_ready_try_push_locked(LmxMsgRuntime *rt, LmxMsgAddr addr) {
    LmxMsgExec *e = exof(rt);
    if (e == 0) {
        return 1;
    }
    if (ready_grow(e) != 0) {
        return 1;
    }
    e->ready[e->nready++] = addr;
    return 0;
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

void lmx_msg_exec_wake_locked(LmxMsgRuntime *rt) {
    LmxMsgExec *e = exof(rt);
    int i;
    if (e == 0) {
        return;
    }
#if defined(_WIN32)
    SetEvent(e->ready_ev);
    for (i = 0; i < e->nbind; i++) {
        if (e->bind[i].wait_ev != 0 && e->bind[i].affinity != LMX_MSG_AFFINITY_UI) {
            SetEvent(e->bind[i].wait_ev);
        }
    }
#else
    e->ready_sig = 1;
    pthread_cond_signal(&e->ready_cv);
#endif
}

void lmx_msg_exec_wake_addr_locked(LmxMsgRuntime *rt, LmxMsgAddr addr) {
    LmxMsgExec *e = exof(rt);
    int i;
    if (e == 0 || addr == 0U) {
        return;
    }
#if defined(_WIN32)
    SetEvent(e->ready_ev);
    for (i = 0; i < e->nbind; i++) {
        if (e->bind[i].addr == addr && e->bind[i].wait_ev != 0 && e->bind[i].affinity != LMX_MSG_AFFINITY_UI) {
            SetEvent(e->bind[i].wait_ev);
        }
    }
#else
    e->ready_sig = 1;
    pthread_cond_signal(&e->ready_cv);
#endif
}

int lmx_msg_exec_nready_locked(LmxMsgRuntime *rt) {
    LmxMsgExec *e = exof(rt);
    if (e == 0) {
        return 0;
    }
    return e->nready;
}

LmxMsgAddr lmx_msg_exec_ready_at_locked(LmxMsgRuntime *rt, int i) {
    LmxMsgExec *e = exof(rt);
    if (e == 0 || i < 0 || i >= e->nready) {
        return 0;
    }
    return e->ready[i];
}

void lmx_msg_exec_ready_remove_locked(LmxMsgRuntime *rt, int i) {
    LmxMsgExec *e = exof(rt);
    if (e == 0 || i < 0 || i >= e->nready) {
        return;
    }
    e->nready -= 1;
    memmove(&e->ready[i], &e->ready[i + 1], (size_t)(e->nready - i) * sizeof(LmxMsgAddr));
}

int lmx_msg_exec_bind_n_locked(LmxMsgRuntime *rt) {
    LmxMsgExec *e = exof(rt);
    if (e == 0) {
        return 0;
    }
    return e->nbind;
}

LmxMsgAddr lmx_msg_exec_bind_addr_locked(LmxMsgRuntime *rt, int i) {
    LmxMsgExec *e = exof(rt);
    if (e == 0 || i < 0 || i >= e->nbind) {
        return 0;
    }
    return e->bind[i].addr;
}

int lmx_msg_exec_bind_aff_locked(LmxMsgRuntime *rt, int i) {
    LmxMsgExec *e = exof(rt);
    if (e == 0 || i < 0 || i >= e->nbind) {
        return 0;
    }
    return e->bind[i].affinity;
}

int lmx_msg_exec_bind_held_locked(LmxMsgRuntime *rt, int i) {
    LmxMsgExec *e = exof(rt);
    if (e == 0 || i < 0 || i >= e->nbind) {
        return 1;
    }
    return e->bind[i].held;
}

void lmx_msg_exec_bind_set_held_locked(LmxMsgRuntime *rt, int i, int held) {
    LmxMsgExec *e = exof(rt);
    if (e == 0 || i < 0 || i >= e->nbind) {
        return;
    }
    e->bind[i].held = held;
    if (held != 0) {
        e->bind[i].held_by = lmx_tid();
    } else {
        e->bind[i].held_by = 0;
    }
}

/* D1 allocation enumeration. Not the scheduler. Delegates to
 * Codex lmx_msg_slots; scan_ready must not use these. */
int lmx_msg_exec_tab_n_locked(LmxMsgRuntime *rt) {
    return lmx_msg_slots_n(rt);
}

LmxMsgAddr lmx_msg_exec_tab_addr_locked(LmxMsgRuntime *rt, int i) {
    return lmx_msg_slots_at(rt, i);
}

#if defined(LMX_MSG_EXEC_TEST)
void lmx_msg_exec_test_set_fail_grow(LmxMsgRuntime *rt, int v) {
    LmxMsgExec *e = exof(rt);
    if (e == 0) {
        return;
    }
    lmx_msg_exec_lock(rt);
    e->test_fail_grow = v;
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

int lmx_msg_exec_ready_cap(LmxMsgRuntime *rt) {
    LmxMsgExec *e = exof(rt);
    int cap = 0;
    if (e == 0) {
        return 0;
    }
    lmx_msg_exec_lock(rt);
    cap = e->ready_cap;
    lmx_msg_exec_unlock(rt);
    return cap;
}

int lmx_msg_exec_nready(LmxMsgRuntime *rt) {
    LmxMsgExec *e = exof(rt);
    int n = 0;
    if (e == 0) {
        return 0;
    }
    lmx_msg_exec_lock(rt);
    n = e->nready;
    lmx_msg_exec_unlock(rt);
    return n;
}

int lmx_msg_exec_test_fail_hits(LmxMsgRuntime *rt) {
    LmxMsgExec *e = exof(rt);
    int n = 0;
    if (e == 0) {
        return 0;
    }
    lmx_msg_exec_lock(rt);
    n = e->test_fail_hits;
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

int lmx_msg_exec_ready_has(LmxMsgRuntime *rt, LmxMsgAddr addr) {
    int h = 0;
    if (rt == 0 || addr == 0U) {
        return 0;
    }
    lmx_msg_exec_lock(rt);
    h = lmx_msg_exec_ready_has_locked(rt, addr);
    lmx_msg_exec_unlock(rt);
    return h;
}

int lmx_msg_exec_bind_n(LmxMsgRuntime *rt) {
    LmxMsgExec *e = exof(rt);
    int n = 0;
    if (e == 0) {
        return 0;
    }
    lmx_msg_exec_lock(rt);
    n = e->nbind;
    lmx_msg_exec_unlock(rt);
    return n;
}

int lmx_msg_exec_bind_aff(LmxMsgRuntime *rt, LmxMsgAddr addr) {
    LmxMsgExec *e = exof(rt);
    int i;
    int aff = -1;
    if (e == 0 || addr == 0U) {
        return -1;
    }
    lmx_msg_exec_lock(rt);
    i = bind_index(e, addr);
    if (i >= 0) {
        aff = e->bind[i].affinity;
    }
    lmx_msg_exec_unlock(rt);
    return aff;
}

int lmx_msg_exec_bind_has_worker(LmxMsgRuntime *rt, LmxMsgAddr addr) {
    LmxMsgExec *e = exof(rt);
    int i;
    int h = 0;
    if (e == 0 || addr == 0U) {
        return 0;
    }
    lmx_msg_exec_lock(rt);
    i = bind_index(e, addr);
    if (i >= 0) {
        h = bind_has_worker(&e->bind[i]);
    }
    lmx_msg_exec_unlock(rt);
    return h;
}
#endif

static int bind_index(LmxMsgExec *e, LmxMsgAddr addr) {
    int i;
    if (e == 0) {
        return -1;
    }
    for (i = 0; i < e->nbind; i++) {
        if (e->bind[i].addr == addr) {
            return i;
        }
    }
    return -1;
}

static int bind_has_worker(const LmxMsgExecBind *b) {
#if defined(_WIN32)
    return b != 0 && b->worker != 0;
#else
    (void)b;
    return 0;
#endif
}

static void unbind_slot_locked(LmxMsgExec *e, int i) {
    LmxMsg *old;
    if (e == 0 || i < 0 || i >= e->nbind) {
        return;
    }
#if defined(_WIN32)
    if (e->bind[i].wait_ev != 0) {
        CloseHandle(e->bind[i].wait_ev);
        e->bind[i].wait_ev = 0;
    }
#endif
    old = e->bind[i].msg;
    e->bind[i].msg = 0;
    if (old != 0) {
        lmx_msg_endp_release(old);
    }
    if (i < e->nbind - 1) {
        memmove(&e->bind[i], &e->bind[i + 1], (size_t)(e->nbind - 1 - i) * sizeof(LmxMsgExecBind));
    }
    e->nbind -= 1;
}

static void join_bind_worker(LmxMsgRuntime *rt, int i) {
    LmxMsgExec *e = exof(rt);
#if defined(_WIN32)
    HANDLE th;
    if (e == 0 || i < 0 || i >= e->nbind) {
        return;
    }
    th = e->bind[i].worker;
    e->bind[i].worker = 0;
    if (th == 0) {
        return;
    }
    if (e->bind[i].wait_ev != 0) {
        SetEvent(e->bind[i].wait_ev);
    }
    lmx_msg_exec_unlock(rt);
    WaitForSingleObject(th, INFINITE);
    CloseHandle(th);
    lmx_msg_exec_lock(rt);
    if (e->nworkers > 0) {
        e->nworkers -= 1;
    }
#else
    (void)rt;
    (void)i;
    (void)e;
#endif
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
    int i;
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
    for (i = 0; i < e->nbind; i++) {
        if (e->bind[i].addr == addr) {
            int old_aff;
            int need;
            int st;
            if (e->bind[i].held != 0) {
                lmx_msg_exec_unlock(rt);
                return LMX_MSG_INVALID;
            }
            if (e->bind[i].affinity != affinity && affinity == LMX_MSG_AFFINITY_UI && owner == 0) {
                lmx_msg_exec_unlock(rt);
                return LMX_MSG_INVALID;
            }
            old_aff = e->bind[i].affinity;
            e->bind[i].turn = turn;
            e->bind[i].ctx = ctx;
            e->bind[i].gone = 0;
            if (e->bind[i].msg != m) {
                if (e->bind[i].msg != 0) {
                    lmx_msg_endp_release(e->bind[i].msg);
                    e->bind[i].msg = 0;
                }
                if (lmx_msg_endp_retain(m) == 0) {
                    lmx_msg_exec_unlock(rt);
                    return LMX_MSG_NOMEM;
                }
                e->bind[i].msg = m;
            }
            m->turn = turn;
            m->turn_ctx = ctx;
            kick = bind_kick_needed_locked(m);
            if (old_aff != affinity) {
                if (affinity == LMX_MSG_AFFINITY_UI) {
                    e->bind[i].affinity = LMX_MSG_AFFINITY_UI;
                    join_bind_worker(rt, i);
                    lmx_msg_exec_unlock(rt);
                    if (kick != 0) {
                        lmx_msg_exec_ready(rt, addr);
                    }
                    return LMX_MSG_OK;
                }
                e->bind[i].affinity = affinity;
                need = e->contexts_live != 0 && bind_has_worker(&e->bind[i]) == 0;
                lmx_msg_exec_unlock(rt);
                if (need != 0) {
                    st = launch_ctx_thread(rt, addr);
                    if (st != LMX_MSG_OK) {
                        lmx_msg_exec_lock(rt);
                        i = bind_index(e, addr);
                        if (i >= 0 && bind_has_worker(&e->bind[i]) == 0) {
                            e->bind[i].affinity = old_aff;
                        }
                        lmx_msg_exec_unlock(rt);
                        return st;
                    }
                }
                if (kick != 0) {
                    lmx_msg_exec_ready(rt, addr);
                }
                return LMX_MSG_OK;
            }
            need = e->contexts_live != 0 && affinity != LMX_MSG_AFFINITY_UI && bind_has_worker(&e->bind[i]) == 0;
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
    }
    if (bind_grow(e) != 0) {
        lmx_msg_exec_unlock(rt);
        return LMX_MSG_NOMEM;
    }
    memset(&e->bind[e->nbind], 0, sizeof(LmxMsgExecBind));
    e->bind[e->nbind].addr = addr;
    e->bind[e->nbind].turn = turn;
    e->bind[e->nbind].ctx = ctx;
    e->bind[e->nbind].affinity = affinity;
    if (lmx_msg_endp_retain(m) == 0) {
        lmx_msg_exec_unlock(rt);
        return LMX_MSG_NOMEM;
    }
    e->bind[e->nbind].msg = m;
    m->turn = turn;
    m->turn_ctx = ctx;
#if defined(_WIN32)
    e->bind[e->nbind].wait_ev = CreateEventA(0, 0, 0, 0);
    if (e->bind[e->nbind].wait_ev == 0) {
        e->bind[e->nbind].msg = 0;
        lmx_msg_endp_release(m);
        lmx_msg_exec_unlock(rt);
        return LMX_MSG_NOMEM;
    }
#endif
    e->nbind += 1;
    live = e->contexts_live;
    kick = bind_kick_needed_locked(m);
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
            i = bind_index(e, addr);
            if (i >= 0 && bind_has_worker(&e->bind[i]) == 0) {
                unbind_slot_locked(e, i);
            }
            lmx_msg_exec_unlock(rt);
            return st;
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

static int take_ready(LmxMsgExec *e, int want_ui, LmxMsgExecBind *snap) {
    int j;
    LmxMsgAddr addr;
    addr = lmx_msg_exec_take_addr(e->rt, want_ui);
    if (addr == 0U) {
        return 0;
    }
    for (j = 0; j < e->nbind; j++) {
        if (e->bind[j].addr == addr) {
            *snap = e->bind[j];
            return 1;
        }
    }
    return 0;
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
    int i;
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
        for (i = 0; i < e->nbind; i++) {
            if (e->bind[i].addr == snap->addr) {
                e->bind[i].held = 0;
                e->bind[i].held_by = 0;
                e->bind[i].last_st = st;
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
    if (m != 0 && lmx_msg_running_load(m) == 0) {
        m->closing = 1;
        lmx_msg_exec_unlock(rt);
        st = 0;
        lmx_msg_end_turn(rt, snap->addr, 0);
        lmx_msg_exec_lock(rt);
        for (i = 0; i < e->nbind; i++) {
            if (e->bind[i].addr == snap->addr) {
                e->bind[i].held = 0;
                e->bind[i].held_by = 0;
                e->bind[i].last_st = st;
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
    for (i = 0; i < e->nbind; i++) {
        if (e->bind[i].addr == snap->addr) {
            e->bind[i].held = 0;
            e->bind[i].held_by = 0;
            e->bind[i].last_st = st;
        }
    }
    lmx_msg_exec_unlock(rt);
    turn_root_pop(&root, saved);
    set_tls(e, old);
    native_leave_addr(rt, snap->addr);
    requeue_if_runnable(rt, snap->addr);
    return st;
}

#if defined(_WIN32)
static DWORD WINAPI worker(void *arg)
#else
static void *worker(void *arg)
#endif
{
    LmxMsgRuntime *rt = (LmxMsgRuntime *)arg;
    LmxMsgExec *e = exof(rt);
    LmxMsgExecBind snap;
    for (;;) {
        lmx_msg_exec_lock(rt);
        if (e->stopping != 0) {
            lmx_msg_exec_unlock(rt);
            break;
        }
        if (take_ready(e, 0, &snap) == 0) {
            lmx_msg_exec_unlock(rt);
#if defined(_WIN32)
            WaitForSingleObject(e->ready_ev, 20);
#else
            {
                struct timespec ts;
                lmx_msg_exec_lock(rt);
                clock_gettime(CLOCK_REALTIME, &ts);
                ts.tv_nsec += 20000000L;
                if (ts.tv_nsec >= 1000000000L) {
                    ts.tv_sec += 1;
                    ts.tv_nsec -= 1000000000L;
                }
                pthread_cond_timedwait(&e->ready_cv, &e->lock, &ts);
                lmx_msg_exec_unlock(rt);
            }
#endif
            continue;
        }
        lmx_msg_exec_unlock(rt);
        run_one(rt, &snap);
    }
#if defined(_WIN32)
    return 0;
#else
    return 0;
#endif
}

int lmx_msg_exec_start(LmxMsgRuntime *rt, int nworkers) {
    LmxMsgExec *e = exof(rt);
    int i;
    if (e == 0 || nworkers < 1) {
        return LMX_MSG_INVALID;
    }
    if (lmx_msg_host_is_owner(rt) == 0) {
        return LMX_MSG_INVALID;
    }
    lmx_msg_exec_lock(rt);
    if (e->nworkers != 0 || e->contexts_live != 0) {
        lmx_msg_exec_unlock(rt);
        return LMX_MSG_INVALID;
    }
#if defined(_WIN32)
    e->wh = (HANDLE *)calloc((size_t)nworkers, sizeof(HANDLE));
#else
    e->wh = (pthread_t *)calloc((size_t)nworkers, sizeof(pthread_t));
#endif
    if (e->wh == 0) {
        lmx_msg_exec_unlock(rt);
        return LMX_MSG_NOMEM;
    }
    e->workers_cap = nworkers;
    e->stopping = 0;
    e->stopped = 0;
    e->nworkers = 0;
#if defined(_WIN32)
    ResetEvent(e->stop_ev);
    ResetEvent(e->ready_ev);
#endif
    lmx_msg_exec_unlock(rt);
    for (i = 0; i < nworkers; i++) {
#if defined(_WIN32)
        e->wh[i] = CreateThread(0, 0, worker, rt, 0, 0);
        if (e->wh[i] == 0) {
            lmx_msg_exec_lock(rt);
            e->nworkers = i;
            lmx_msg_exec_unlock(rt);
            lmx_msg_exec_stop(rt);
            return LMX_MSG_NOMEM;
        }
#else
        if (pthread_create(&e->wh[i], 0, worker, rt) != 0) {
            lmx_msg_exec_lock(rt);
            e->nworkers = i;
            lmx_msg_exec_unlock(rt);
            lmx_msg_exec_stop(rt);
            return LMX_MSG_NOMEM;
        }
#endif
        lmx_msg_exec_lock(rt);
        e->nworkers = i + 1;
        lmx_msg_exec_unlock(rt);
    }
    return LMX_MSG_OK;
}

typedef struct LmxMsgCtxPack {
    LmxMsgRuntime *rt;
    LmxMsgAddr addr;
} LmxMsgCtxPack;

static int launch_ctx_thread(LmxMsgRuntime *rt, LmxMsgAddr addr) {
#if !defined(_WIN32)
    (void)rt;
    (void)addr;
    return LMX_MSG_INVALID;
#else
    LmxMsgExec *e = exof(rt);
    LmxMsgCtxPack *pack;
    HANDLE th;
    int i;
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
    i = bind_index(e, addr);
    if (i < 0 || e->stopping != 0 || e->bind[i].affinity == LMX_MSG_AFFINITY_UI) {
        lmx_msg_exec_unlock(rt);
        return LMX_MSG_INVALID;
    }
    if (e->bind[i].worker != 0 || e->bind[i].launching != 0) {
        lmx_msg_exec_unlock(rt);
        return LMX_MSG_OK;
    }
    e->bind[i].launching = 1;
    lmx_msg_exec_unlock(rt);
    pack = (LmxMsgCtxPack *)malloc(sizeof(LmxMsgCtxPack));
    if (pack == 0) {
        lmx_msg_exec_lock(rt);
        i = bind_index(e, addr);
        if (i >= 0) {
            e->bind[i].launching = 0;
        }
        lmx_msg_exec_unlock(rt);
        return LMX_MSG_NOMEM;
    }
    pack->rt = rt;
    pack->addr = addr;
    th = CreateThread(0, 0, context_worker, pack, 0, 0);
    if (th == 0) {
        free(pack);
        lmx_msg_exec_lock(rt);
        i = bind_index(e, addr);
        if (i >= 0) {
            e->bind[i].launching = 0;
        }
        lmx_msg_exec_unlock(rt);
        return LMX_MSG_NOMEM;
    }
    lmx_msg_exec_lock(rt);
    i = bind_index(e, addr);
    if (i < 0 || e->stopping != 0) {
        if (i >= 0) {
            e->bind[i].launching = 0;
        }
        lmx_msg_exec_unlock(rt);
        WaitForSingleObject(th, INFINITE);
        CloseHandle(th);
        return LMX_MSG_INVALID;
    }
    e->bind[i].worker = th;
    e->bind[i].launching = 0;
    e->nworkers += 1;
    lmx_msg_exec_unlock(rt);
    return LMX_MSG_OK;
#endif
}

static int take_this(LmxMsgExec *e, LmxMsgAddr addr, LmxMsgExecBind *snap) {
    int j;
    LmxMsg *m;
    if (e == 0 || addr == 0U || snap == 0) {
        return 0;
    }
    for (j = 0; j < e->nbind; j++) {
        if (e->bind[j].addr != addr) {
            continue;
        }
        if (e->bind[j].affinity == LMX_MSG_AFFINITY_UI) {
            return 0;
        }
        if (e->bind[j].held != 0 || e->bind[j].gone != 0) {
            return 0;
        }
        m = e->bind[j].msg;
        if (m == 0 || m->addr != addr) {
            return 0;
        }
        if (m->state == LMX_MSG_STATE_STOPPED || m->state == LMX_MSG_STATE_DEAD || m->state == LMX_MSG_STATE_RELEASED) {
            return 0;
        }
        if (lmx_msg_mail_inbox_empty(m) != 0 && m->closing == 0 && lmx_msg_running_load(m) != 0) {
            return 0;
        }
        e->bind[j].held = 1;
        e->bind[j].held_by = lmx_tid();
        *snap = e->bind[j];
        return 1;
    }
    return 0;
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
    int i;
    if (p == 0) {
        return 1;
    }
    rt = p->rt;
    addr = p->addr;
    free(p);
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
        for (i = 0; i < e->nbind; i++) {
            if (e->bind[i].addr == addr) {
                if (e->bind[i].affinity == LMX_MSG_AFFINITY_UI || e->bind[i].gone != 0) {
                    lmx_msg_exec_unlock(rt);
                    return 0;
                }
                ev = e->bind[i].wait_ev;
                break;
            }
        }
        if (take_this(e, addr, &snap) != 0) {
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
#endif

int lmx_msg_exec_start_contexts(LmxMsgRuntime *rt) {
    LmxMsgExec *e = exof(rt);
    int i;
    int nbind;
    LmxMsgAddr addr;
    int st;
    if (e == 0) {
        return LMX_MSG_INVALID;
    }
    if (lmx_msg_host_is_owner(rt) == 0) {
        return LMX_MSG_INVALID;
    }
#if !defined(_WIN32)
    return LMX_MSG_INVALID;
#else
    lmx_msg_exec_lock(rt);
    if (e->contexts_live != 0 || e->wh != 0) {
        lmx_msg_exec_unlock(rt);
        return LMX_MSG_INVALID;
    }
    e->stopping = 0;
    e->stopped = 0;
    ResetEvent(e->stop_ev);
    e->contexts_live = 1;
    nbind = e->nbind;
    lmx_msg_exec_unlock(rt);
    for (i = 0; i < nbind; i++) {
        lmx_msg_exec_lock(rt);
        if (i >= e->nbind) {
            lmx_msg_exec_unlock(rt);
            break;
        }
        if (e->bind[i].affinity == LMX_MSG_AFFINITY_UI) {
            lmx_msg_exec_unlock(rt);
            continue;
        }
        if (e->bind[i].gone != 0) {
            lmx_msg_exec_unlock(rt);
            continue;
        }
#if defined(_WIN32)
        if (e->bind[i].worker != 0) {
            lmx_msg_exec_unlock(rt);
            continue;
        }
#endif
        addr = e->bind[i].addr;
        lmx_msg_exec_unlock(rt);
        st = launch_ctx_thread(rt, addr);
        if (st != LMX_MSG_OK) {
            lmx_msg_exec_stop(rt);
            return st;
        }
        lmx_msg_exec_lock(rt);
        nbind = e->nbind;
        lmx_msg_exec_unlock(rt);
    }
    return LMX_MSG_OK;
#endif
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
    if (take_ready(e, 1, &snap) == 0) {
        lmx_msg_exec_unlock(rt);
        return LMX_MSG_EMPTY;
    }
    lmx_msg_exec_unlock(rt);
    run_one(rt, &snap);
    return LMX_MSG_OK;
}

int lmx_msg_exec_last_status(LmxMsgRuntime *rt, LmxMsgAddr addr) {
    LmxMsgExec *e = exof(rt);
    int i;
    int st = LMX_MSG_INVALID;
    if (e == 0 || addr == 0U) {
        return LMX_MSG_INVALID;
    }
    lmx_msg_exec_lock(rt);
    for (i = 0; i < e->nbind; i++) {
        if (e->bind[i].addr == addr) {
            st = e->bind[i].last_st;
            break;
        }
    }
    lmx_msg_exec_unlock(rt);
    return st;
}

int lmx_msg_exec_is_bound(LmxMsgRuntime *rt, LmxMsgAddr addr) {
    LmxMsgExec *e = exof(rt);
    int i;
    int hit = 0;
    if (e == 0 || addr == 0U) {
        return 0;
    }
    lmx_msg_exec_lock(rt);
    for (i = 0; i < e->nbind; i++) {
        if (e->bind[i].addr == addr) {
            hit = 1;
            break;
        }
    }
    lmx_msg_exec_unlock(rt);
    return hit;
}

int lmx_msg_exec_unbound_close(LmxMsgRuntime *rt, LmxMsgAddr addr) {
    LmxMsgExec *e = exof(rt);
    int st;
    int i;
    LmxMsgAddr old;
    if (e == 0 || addr == 0U) {
        return LMX_MSG_INVALID;
    }
    if (lmx_msg_host_is_owner(rt) == 0) {
        return LMX_MSG_INVALID;
    }
    lmx_msg_exec_lock(rt);
    for (i = 0; i < e->nbind; i++) {
        if (e->bind[i].addr == addr) {
            lmx_msg_exec_unlock(rt);
            lmx_msg_exec_ready(rt, addr);
            return LMX_MSG_OK;
        }
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

int lmx_msg_exec_stop(LmxMsgRuntime *rt) {
    LmxMsgExec *e = exof(rt);
    int i;
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
    for (i = 0; i < e->nbind; i++) {
        if (e->bind[i].msg != 0) {
            e->bind[i].msg->mapped = 0;
        }
    }
#if defined(_WIN32)
    for (i = 0; i < e->nbind; i++) {
        if (e->bind[i].wait_ev != 0) {
            SetEvent(e->bind[i].wait_ev);
        }
    }
#endif
    lmx_msg_exec_unlock(rt);
#if defined(_WIN32)
    SetEvent(e->stop_ev);
    SetEvent(e->ready_ev);
    for (i = 0; i < e->nbind; i++) {
        HANDLE th;
        lmx_msg_exec_lock(rt);
        if (i >= e->nbind) {
            lmx_msg_exec_unlock(rt);
            break;
        }
        th = e->bind[i].worker;
        e->bind[i].worker = 0;
        lmx_msg_exec_unlock(rt);
        if (th != 0) {
            WaitForSingleObject(th, INFINITE);
            CloseHandle(th);
        }
    }
    for (i = 0; i < e->nworkers; i++) {
        if (e->wh != 0 && e->wh[i] != 0) {
            WaitForSingleObject(e->wh[i], INFINITE);
            CloseHandle(e->wh[i]);
            e->wh[i] = 0;
        }
    }
    free(e->wh);
    e->wh = 0;
#else
    pthread_cond_broadcast(&e->ready_cv);
    for (i = 0; i < e->nworkers; i++) {
        if (e->wh != 0) {
            pthread_join(e->wh[i], 0);
        }
    }
    free(e->wh);
    e->wh = 0;
#endif
    e->nworkers = 0;
    e->workers_cap = 0;
    lmx_msg_exec_lock(rt);
    set_tls(e, 0);
    for (i = 0; i < e->nbind; i++) {
#if defined(_WIN32)
        e->bind[i].worker = 0;
#endif
        if (e->bind[i].msg != 0) {
            e->bind[i].msg->mapped = 0;
        }
    }
    e->stopped = 1;
    lmx_msg_exec_unlock(rt);
    return LMX_MSG_OK;
}

void lmx_msg_exec_drop_binds(LmxMsgRuntime *rt) {
    LmxMsgExec *e = exof(rt);
    int i;
    LmxMsg *old;
    if (e == 0) {
        return;
    }
    lmx_msg_exec_lock(rt);
    for (i = 0; i < e->nbind; i++) {
        old = e->bind[i].msg;
        e->bind[i].msg = 0;
        if (old != 0) {
            old->mapped = 0;
            lmx_msg_endp_release(old);
        }
    }
    lmx_msg_exec_unlock(rt);
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
    int i;
    LmxMsg *old;
    if (e == 0 || addr == 0U) {
        return LMX_MSG_INVALID;
    }
    lmx_msg_exec_lock(rt);
    i = bind_index(e, addr);
    if (i < 0) {
        lmx_msg_exec_unlock(rt);
        return LMX_MSG_OK;
    }
    /* Nested unbind (failed parent end_turn) cannot join: the outer
     * exec lock would stay held and the wait-thread could not exit.
     * Drop the cached endpoint; keep wait_ev until exec_stop joins. */
    if (bind_has_worker(&e->bind[i]) != 0 || e->bind[i].launching != 0) {
        old = e->bind[i].msg;
        e->bind[i].msg = 0;
        e->bind[i].gone = 1;
#if defined(_WIN32)
        if (e->bind[i].wait_ev != 0) {
            SetEvent(e->bind[i].wait_ev);
        }
#endif
        if (old != 0) {
            lmx_msg_endp_release(old);
        }
        lmx_msg_exec_unlock(rt);
        return LMX_MSG_OK;
    }
    unbind_slot_locked(e, i);
    lmx_msg_exec_unlock(rt);
    return LMX_MSG_OK;
}

int lmx_msg_run_child_turn(LmxMsgRuntime *rt, LmxMsgAddr child) {
    LmxMsgExec *e = exof(rt);
    LmxMsg *m;
    LmxMsgExecBind snap;
    LmxMsgAddr par;
    int owner;
    int st;
    int i;
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
    i = bind_index(e, child);
    if (i < 0) {
        lmx_msg_exec_unlock(rt);
        return LMX_MSG_INVALID;
    }
    if (e->bind[i].held != 0) {
        lmx_msg_exec_unlock(rt);
        return LMX_MSG_INVALID;
    }
    e->bind[i].held = 1;
    e->bind[i].held_by = lmx_tid();
    memset(&snap, 0, sizeof(snap));
    snap.addr = child;
    snap.turn = m->turn;
    snap.ctx = m->turn_ctx;
    lmx_msg_exec_unlock(rt);
    st = run_one(rt, &snap);
    if (par != 0U && lmx_msg_exec_holding_turn(rt, par) != 0) {
        (void)lmx_msg_parent_settle(rt, par);
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
    if (bind_index(e, child) < 0) {
        lmx_msg_exec_unlock(rt);
        return LMX_MSG_INVALID;
    }
    if (c->mapped != 0) {
        lmx_msg_exec_unlock(rt);
        return LMX_MSG_OK;
    }
    c->mapped = 1;
    lmx_msg_exec_unlock(rt);
    st = launch_ctx_thread(rt, child);
    if (st != LMX_MSG_OK) {
        int i;
        lmx_msg_exec_lock(rt);
        c = msg_at_addr(rt, child);
        i = bind_index(e, child);
        if (c != 0 && (i < 0 || bind_has_worker(&e->bind[i]) == 0)) {
            c->mapped = 0;
        }
        lmx_msg_exec_unlock(rt);
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

int lmx_msg_adopt_failed(LmxMsgRuntime *rt, LmxMsgAddr parent, LmxMsgAddr child) {
    LmxMsg *p;
    LmxMsg *c;
    LmxMsg *ch;
    if (rt == 0 || lifecycle_authority(rt, parent) == 0) {
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
    ch = c->first_child;
    while (ch != 0) {
        if (ch->disposed == 0) {
            lmx_msg_exec_unlock(rt);
            return LMX_MSG_INVALID;
        }
        ch = ch->next_sibling;
    }
    if (lmx_msg_storage_can_move(&p->blocks, &p->ranges, &c->blocks, &c->ranges)
        != LMX_MSG_STORAGE_OK) {
        lmx_msg_exec_unlock(rt);
        return LMX_MSG_INVALID;
    }
    {
        LmxMsgBlock *prepared = 0;
        if (c->init != 0) {
#if defined(LMX_MSG_EXEC_TEST)
            if (exof(rt) != 0 && exof(rt)->test_fail_adopt_block != 0) {
                lmx_msg_exec_unlock(rt);
                return LMX_MSG_NOMEM;
            }
#endif
            prepared = (LmxMsgBlock *)calloc(1U, sizeof(LmxMsgBlock));
            if (prepared == 0) {
                lmx_msg_exec_unlock(rt);
                return LMX_MSG_NOMEM;
            }
            prepared->base = c->init;
            prepared->n = c->init_n;
        }
        if (lmx_msg_storage_move_all(&p->blocks, &p->ranges, &c->blocks, &c->ranges)
            != LMX_MSG_STORAGE_OK) {
            free(prepared);
            lmx_msg_exec_unlock(rt);
            return LMX_MSG_INVALID;
        }
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

int lmx_msg_dispose_child(LmxMsgRuntime *rt, LmxMsgAddr parent, LmxMsgAddr child) {
    LmxMsg *p;
    LmxMsg *c;
    LmxMsg *ch;
    if (lifecycle_authority(rt, parent) == 0) {
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
    if (lmx_msg_success_load(c) == 0 && (c->init != 0 || c->blocks != 0 || c->ranges != 0)) {
        lmx_msg_exec_unlock(rt);
        return LMX_MSG_INVALID;
    }
    ch = c->first_child;
    while (ch != 0) {
        if (ch->disposed == 0) {
            lmx_msg_exec_unlock(rt);
            return LMX_MSG_INVALID;
        }
        ch = ch->next_sibling;
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
