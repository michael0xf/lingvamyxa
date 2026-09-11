/* Overlapping Message turns. Mutex not held during turn_fn. */
#include "l2src/lmx_message_exec.h"
#include "l2src/lmx_message_host.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
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
#endif
} LmxMsgExec;

static int launch_ctx_thread(LmxMsgRuntime *rt, LmxMsgAddr addr);
#if defined(_WIN32)
static DWORD WINAPI context_worker(void *arg);
#endif
static int bind_index(LmxMsgExec *e, LmxMsgAddr addr);
static int bind_has_worker(const LmxMsgExecBind *b);
static LmxMsg *msg_at_addr(LmxMsgRuntime *rt, LmxMsgAddr addr);

static LmxMsgExec *exof(LmxMsgRuntime *rt) {
    if (rt == 0) {
        return 0;
    }
    return (LmxMsgExec *)rt->exec;
}

int lmx_msg_path_grow(LmxMsg *slot, int need) {
    int cap;
    unsigned *p;
    if (slot == 0 || need < 1) {
        return LMX_MSG_INVALID;
    }
    if (slot->path_cap >= need) {
        return LMX_MSG_OK;
    }
    cap = slot->path_cap < 1 ? LMX_MSG_PATH_CHUNK : slot->path_cap;
    while (cap < need) {
        if (cap > 2147483647 / 2) {
            return LMX_MSG_NOMEM;
        }
        cap *= 2;
    }
    if ((size_t)cap > ((size_t)-1) / sizeof(unsigned)) {
        return LMX_MSG_NOMEM;
    }
    p = (unsigned *)realloc(slot->path, (size_t)cap * sizeof(unsigned));
    if (p == 0) {
        return LMX_MSG_NOMEM;
    }
    slot->path = p;
    slot->path_cap = cap;
    return LMX_MSG_OK;
}

int lmx_msg_tab_grow(LmxMsgRuntime *rt) {
    int cap;
    LmxMsg **tab;
    if (rt == 0) {
        return 1;
    }
    if (rt->n < rt->cap) {
        return 0;
    }
    cap = rt->cap == 0 ? 8 : rt->cap * 2;
    tab = (LmxMsg **)realloc(rt->tab, (size_t)cap * sizeof(LmxMsg *));
    if (tab == 0) {
        return 1;
    }
    rt->tab = tab;
    rt->cap = cap;
    return 0;
}

LmxMsg *lmx_msg_slot_new(void) {
    LmxMsg *m = (LmxMsg *)calloc(1U, sizeof(LmxMsg));
    if (m != 0) {
        m->refs = 1;
    }
    return m;
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
    lmx_msg_exec_stop(rt);
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

int lmx_msg_exec_tab_n_locked(LmxMsgRuntime *rt) {
    if (rt == 0) {
        return 0;
    }
    return rt->n;
}

LmxMsgAddr lmx_msg_exec_tab_addr_locked(LmxMsgRuntime *rt, int i) {
    if (rt == 0 || i < 0 || i >= rt->n || rt->tab == 0 || rt->tab[i] == 0) {
        return 0;
    }
    return rt->tab[i]->addr;
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
    if (e == 0 || i < 0 || i >= e->nbind) {
        return;
    }
#if defined(_WIN32)
    if (e->bind[i].wait_ev != 0) {
        CloseHandle(e->bind[i].wait_ev);
        e->bind[i].wait_ev = 0;
    }
#endif
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

int lmx_msg_exec_bind(LmxMsgRuntime *rt, LmxMsgAddr addr, LmxMsgTurn turn, void *ctx, int affinity) {
    LmxMsgExec *e = exof(rt);
    LmxMsg *m;
    int i;
    int owner;
    int live;
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
            e->bind[i].msg = m;
            m->turn = turn;
            m->turn_ctx = ctx;
            if (old_aff != affinity) {
                if (affinity == LMX_MSG_AFFINITY_UI) {
                    e->bind[i].affinity = LMX_MSG_AFFINITY_UI;
                    join_bind_worker(rt, i);
                    lmx_msg_exec_unlock(rt);
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
                return LMX_MSG_OK;
            }
            need = e->contexts_live != 0 && affinity != LMX_MSG_AFFINITY_UI && bind_has_worker(&e->bind[i]) == 0;
            lmx_msg_exec_unlock(rt);
            if (need != 0) {
                return launch_ctx_thread(rt, addr);
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
    e->bind[e->nbind].msg = m;
    e->bind[e->nbind].affinity = affinity;
    m->turn = turn;
    m->turn_ctx = ctx;
#if defined(_WIN32)
    e->bind[e->nbind].wait_ev = CreateEventA(0, 0, 0, 0);
    if (e->bind[e->nbind].wait_ev == 0) {
        lmx_msg_exec_unlock(rt);
        return LMX_MSG_NOMEM;
    }
#endif
    e->nbind += 1;
    live = e->contexts_live;
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
    return LMX_MSG_OK;
}

static LmxMsg *msg_at_addr(LmxMsgRuntime *rt, LmxMsgAddr addr) {
    int i;
    LmxMsg *m;
    for (i = 0; i < rt->n; i++) {
        m = rt->tab[i];
        if (m != 0 && m->addr == addr) {
            return m;
        }
    }
    return 0;
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

static void requeue_if_runnable(LmxMsgRuntime *rt, LmxMsgAddr addr) {
    int i;
    LmxMsg *m;
    lmx_msg_exec_lock(rt);
    for (i = 0; i < rt->n; i++) {
        m = rt->tab[i];
        if (m != 0 && m->addr == addr && lmx_msg_exec_is_runnable(rt, addr) != 0) {
            lmx_msg_exec_unlock(rt);
            lmx_msg_exec_ready(rt, addr);
            return;
        }
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
    old = get_tls(e);
    set_tls(e, snap->addr);
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
    set_tls(e, old);
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
    e->nworkers = 0;
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
        if (e->bind[j].held != 0) {
            return 0;
        }
        m = e->bind[j].msg;
        if (m == 0) {
            m = msg_at_addr(e->rt, addr);
            e->bind[j].msg = m;
        }
        if (m == 0) {
            return 0;
        }
        if (m->state == LMX_MSG_STATE_STOPPED || m->state == LMX_MSG_STATE_DEAD || m->state == LMX_MSG_STATE_RELEASED) {
            return 0;
        }
        if (m->inbox == 0 && m->closing == 0) {
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
                if (e->bind[i].affinity == LMX_MSG_AFFINITY_UI) {
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
            DWORD wr = WaitForMultipleObjects(2, wh, 0, 20);
            LmxMsg *self;
            unsigned th = 0;
            unsigned now = 0;
            if (wr == WAIT_OBJECT_0) {
                return 0;
            }
            lmx_msg_exec_lock(rt);
            self = msg_at_addr(rt, addr);
            if (self != 0) {
                th = self->live_wait_th;
                now = rt->clock;
            }
            lmx_msg_exec_unlock(rt);
            if (th != 0U && wr == WAIT_TIMEOUT) {
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
        if (e->bind[i].affinity == LMX_MSG_AFFINITY_UI || e->bind[i].worker != 0) {
            lmx_msg_exec_unlock(rt);
            continue;
        }
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
