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
    int affinity;
    LmxTid held_by;
    int held;
} LmxMsgExecBind;

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
    LmxMsgRuntime *rt;
    LmxMsgExecBind *bind;
    int nbind;
    int bind_cap;
    LmxMsgAddr *ready;
    int nready;
    int ready_cap;
    int scan;
} LmxMsgExec;

static LmxMsgExec *exof(LmxMsgRuntime *rt) {
    if (rt == 0) {
        return 0;
    }
    return (LmxMsgExec *)rt->exec;
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
    return (LmxMsg *)calloc(1U, sizeof(LmxMsg));
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

void lmx_msg_exec_ready(LmxMsgRuntime *rt, LmxMsgAddr addr) {
    LmxMsgExec *e = exof(rt);
    int i;
    if (e == 0 || addr == 0U) {
        return;
    }
    lmx_msg_exec_lock(rt);
    for (i = 0; i < e->nready; i++) {
        if (e->ready[i] == addr) {
            lmx_msg_exec_unlock(rt);
            return;
        }
    }
    if (ready_grow(e) != 0) {
        e->scan = 1;
        lmx_msg_exec_unlock(rt);
        return;
    }
    e->ready[e->nready++] = addr;
#if defined(_WIN32)
    SetEvent(e->ready_ev);
#else
    e->ready_sig = 1;
    pthread_cond_signal(&e->ready_cv);
#endif
    lmx_msg_exec_unlock(rt);
}

int lmx_msg_exec_bind(LmxMsgRuntime *rt, LmxMsgAddr addr, LmxMsgTurn turn, void *ctx, int affinity) {
    LmxMsgExec *e = exof(rt);
    int i;
    if (e == 0 || addr == 0U || turn == 0) {
        return LMX_MSG_INVALID;
    }
    if (lmx_msg_host_is_owner(rt) == 0) {
        return LMX_MSG_INVALID;
    }
    lmx_msg_exec_lock(rt);
    for (i = 0; i < e->nbind; i++) {
        if (e->bind[i].addr == addr) {
            if (e->bind[i].held != 0) {
                lmx_msg_exec_unlock(rt);
                return LMX_MSG_INVALID;
            }
            e->bind[i].turn = turn;
            e->bind[i].ctx = ctx;
            e->bind[i].affinity = affinity;
            lmx_msg_exec_unlock(rt);
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
    e->nbind += 1;
    lmx_msg_exec_unlock(rt);
    return LMX_MSG_OK;
}

static int ready_has(LmxMsgExec *e, LmxMsgAddr addr) {
    int i;
    for (i = 0; i < e->nready; i++) {
        if (e->ready[i] == addr) {
            return 1;
        }
    }
    return 0;
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

static int msg_runnable(LmxMsg *m) {
    if (m == 0) {
        return 0;
    }
    if (m->state == LMX_MSG_STATE_STOPPED || m->state == LMX_MSG_STATE_DEAD || m->state == LMX_MSG_STATE_RELEASED) {
        return 0;
    }
    if (m->inbox != 0) {
        return 1;
    }
    if (m->closing != 0 && m->state != LMX_MSG_STATE_STOPPED) {
        return 1;
    }
    return 0;
}

static void scan_runnable(LmxMsgRuntime *rt, LmxMsgExec *e) {
    int i;
    LmxMsg *m;
    if (e->scan == 0) {
        return;
    }
    e->scan = 0;
    for (i = 0; i < rt->n; i++) {
        m = rt->tab[i];
        if (msg_runnable(m) == 0 || ready_has(e, m->addr) != 0) {
            continue;
        }
        if (ready_grow(e) != 0) {
            e->scan = 1;
            return;
        }
        e->ready[e->nready++] = m->addr;
    }
}

static int take_ready(LmxMsgExec *e, int want_ui, LmxMsgExecBind *snap) {
    int i;
    int j;
    LmxMsgAddr addr;
    scan_runnable(e->rt, e);
    for (i = 0; i < e->nready; i++) {
        addr = e->ready[i];
        for (j = 0; j < e->nbind; j++) {
            if (e->bind[j].addr != addr) {
                continue;
            }
            if (want_ui && e->bind[j].affinity != LMX_MSG_AFFINITY_UI) {
                continue;
            }
            if (!want_ui && e->bind[j].affinity == LMX_MSG_AFFINITY_UI) {
                continue;
            }
            if (e->bind[j].held != 0) {
                continue;
            }
            if (msg_runnable(msg_at_addr(e->rt, addr)) == 0) {
                e->nready -= 1;
                memmove(&e->ready[i], &e->ready[i + 1], (size_t)(e->nready - i) * sizeof(LmxMsgAddr));
                i -= 1;
                break;
            }
            e->nready -= 1;
            memmove(&e->ready[i], &e->ready[i + 1], (size_t)(e->nready - i) * sizeof(LmxMsgAddr));
            e->bind[j].held = 1;
            e->bind[j].held_by = lmx_tid();
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
        if (m != 0 && m->addr == addr && m->state != LMX_MSG_STATE_STOPPED && m->state != LMX_MSG_STATE_DEAD && m->state != LMX_MSG_STATE_RELEASED && (m->inbox != 0 || m->closing != 0)) {
            lmx_msg_exec_unlock(rt);
            lmx_msg_exec_ready(rt, addr);
            return;
        }
    }
    lmx_msg_exec_unlock(rt);
}

static int run_one(LmxMsgRuntime *rt, LmxMsgExecBind *snap) {
    LmxMsgExec *e = exof(rt);
    int i;
    int st;
    set_tls(e, snap->addr);
    st = snap->turn(rt, snap->addr, snap->ctx);
    set_tls(e, 0);
    lmx_msg_exec_lock(rt);
    for (i = 0; i < e->nbind; i++) {
        if (e->bind[i].addr == snap->addr) {
            e->bind[i].held = 0;
            e->bind[i].held_by = 0;
        }
    }
    lmx_msg_exec_unlock(rt);
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
    if (e->nworkers != 0) {
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
    if (e == 0 || addr == 0U) {
        return LMX_MSG_INVALID;
    }
    if (lmx_msg_host_is_owner(rt) == 0) {
        return LMX_MSG_INVALID;
    }
    if (lmx_msg_exec_is_bound(rt, addr) != 0) {
        lmx_msg_exec_ready(rt, addr);
        return LMX_MSG_OK;
    }
    set_tls(e, addr);
    st = lmx_msg_end_turn(rt, addr, 1);
    set_tls(e, 0);
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
    lmx_msg_exec_unlock(rt);
#if defined(_WIN32)
    SetEvent(e->stop_ev);
    SetEvent(e->ready_ev);
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
