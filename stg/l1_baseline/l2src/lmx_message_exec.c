/* Overlapping Message turns. Mutex not held during turn_fn. */
#include "l2src/lmx_message_exec.h"
#include "l2src/lmx_message_host.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#define LMX_MSG_EXEC_MAX_BIND 64
#define LMX_MSG_EXEC_MAX_READY 64
#define LMX_MSG_EXEC_MAX_WORKERS 8

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
    HANDLE workers[LMX_MSG_EXEC_MAX_WORKERS];
    DWORD tls;
#else
    pthread_mutex_t lock;
    pthread_cond_t ready_cv;
    pthread_t workers[LMX_MSG_EXEC_MAX_WORKERS];
    pthread_key_t tls;
    int ready_sig;
#endif
    int nworkers;
    int stopping;
    LmxMsgRuntime *rt;
    LmxMsgExecBind bind[LMX_MSG_EXEC_MAX_BIND];
    int nbind;
    LmxMsgAddr ready[LMX_MSG_EXEC_MAX_READY];
    int nready;
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
    if (e->nready < LMX_MSG_EXEC_MAX_READY) {
        e->ready[e->nready++] = addr;
#if defined(_WIN32)
        SetEvent(e->ready_ev);
#else
        e->ready_sig = 1;
        pthread_cond_signal(&e->ready_cv);
#endif
    }
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
            e->bind[i].turn = turn;
            e->bind[i].ctx = ctx;
            e->bind[i].affinity = affinity;
            lmx_msg_exec_unlock(rt);
            return LMX_MSG_OK;
        }
    }
    if (e->nbind >= LMX_MSG_EXEC_MAX_BIND) {
        lmx_msg_exec_unlock(rt);
        return LMX_MSG_NOMEM;
    }
    e->bind[e->nbind].addr = addr;
    e->bind[e->nbind].turn = turn;
    e->bind[e->nbind].ctx = ctx;
    e->bind[e->nbind].affinity = affinity;
    e->nbind += 1;
    lmx_msg_exec_unlock(rt);
    return LMX_MSG_OK;
}

static int take_ready(LmxMsgExec *e, int want_ui, LmxMsgExecBind **out) {
    int i;
    int j;
    LmxMsgAddr addr;
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
            e->nready -= 1;
            memmove(&e->ready[i], &e->ready[i + 1], (size_t)(e->nready - i) * sizeof(LmxMsgAddr));
            e->bind[j].held = 1;
            e->bind[j].held_by = lmx_tid();
            *out = &e->bind[j];
            return 1;
        }
    }
    return 0;
}

static int run_one(LmxMsgRuntime *rt, LmxMsgExecBind *b) {
    LmxMsgExec *e = exof(rt);
    int st;
    set_tls(e, b->addr);
    st = b->turn(rt, b->addr, b->ctx);
    set_tls(e, 0);
    lmx_msg_exec_lock(rt);
    b->held = 0;
    b->held_by = 0;
    lmx_msg_exec_unlock(rt);
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
    LmxMsgExecBind *b;
    for (;;) {
        if (e->stopping != 0) {
            break;
        }
        lmx_msg_exec_lock(rt);
        b = 0;
        if (take_ready(e, 0, &b) == 0) {
            lmx_msg_exec_unlock(rt);
            if (e->stopping != 0) {
                break;
            }
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
        run_one(rt, b);
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
    if (e == 0 || nworkers < 1 || nworkers > LMX_MSG_EXEC_MAX_WORKERS) {
        return LMX_MSG_INVALID;
    }
    if (lmx_msg_host_is_owner(rt) == 0) {
        return LMX_MSG_INVALID;
    }
    e->stopping = 0;
    e->nworkers = nworkers;
    for (i = 0; i < nworkers; i++) {
#if defined(_WIN32)
        e->workers[i] = CreateThread(0, 0, worker, rt, 0, 0);
        if (e->workers[i] == 0) {
            return LMX_MSG_NOMEM;
        }
#else
        if (pthread_create(&e->workers[i], 0, worker, rt) != 0) {
            return LMX_MSG_NOMEM;
        }
#endif
    }
    return LMX_MSG_OK;
}

int lmx_msg_exec_ui_step(LmxMsgRuntime *rt) {
    LmxMsgExec *e = exof(rt);
    LmxMsgExecBind *b;
    if (e == 0) {
        return LMX_MSG_INVALID;
    }
    if (lmx_msg_host_is_owner(rt) == 0) {
        return LMX_MSG_INVALID;
    }
    lmx_msg_host_drain(rt);
    lmx_msg_exec_lock(rt);
    b = 0;
    if (take_ready(e, 1, &b) == 0) {
        lmx_msg_exec_unlock(rt);
        return LMX_MSG_EMPTY;
    }
    lmx_msg_exec_unlock(rt);
    run_one(rt, b);
    return LMX_MSG_OK;
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
    e->stopping = 1;
#if defined(_WIN32)
    SetEvent(e->stop_ev);
    SetEvent(e->ready_ev);
    for (i = 0; i < e->nworkers; i++) {
        if (e->workers[i]) {
            WaitForSingleObject(e->workers[i], INFINITE);
            CloseHandle(e->workers[i]);
            e->workers[i] = 0;
        }
    }
#else
    pthread_cond_broadcast(&e->ready_cv);
    for (i = 0; i < e->nworkers; i++) {
        pthread_join(e->workers[i], 0);
    }
#endif
    e->nworkers = 0;
    return LMX_MSG_OK;
}
