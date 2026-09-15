/* LMX_MSG_HOST_INGRESS_V0 platform primitives only: owner id, mutex.
 * Copy/envelope/queue policy lives in lmx_message.lm1. S3 (Mikhail 2026-09-15):
 * no wait, no wake -- the owner loops, checked each round in its own caller. */
#include "l2src/lmx_message_host.h"
#include <stdlib.h>

#if defined(LMX_MSG_HOST_TEST)
int lmx_msg_host_test_nomem;
int lmx_msg_host_test_nowake;

int lmx_msg_host_test_get_nomem(void) {
    return lmx_msg_host_test_nomem;
}

void lmx_msg_host_test_set_nomem(int v) {
    lmx_msg_host_test_nomem = v;
}

void lmx_msg_host_test_set_nowake(int v) {
    lmx_msg_host_test_nowake = v;
}
#else
int lmx_msg_host_test_get_nomem(void) {
    return 0;
}
#endif

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
typedef struct LmxMsgHostSync {
    CRITICAL_SECTION lock;
    DWORD owner;
    int shutting_down;
} LmxMsgHostSync;
#else
#include <pthread.h>
#include <sched.h>
typedef struct LmxMsgHostSync {
    pthread_mutex_t lock;
    pthread_t owner;
    int shutting_down;
} LmxMsgHostSync;
#endif

int lmx_msg_host_attach(LmxMsgRuntime *rt) {
    LmxMsgHostSync *h;
    if (rt == 0) {
        return 1;
    }
    h = (LmxMsgHostSync *)calloc(1U, sizeof(LmxMsgHostSync));
    if (h == 0) {
        return 1;
    }
#if defined(_WIN32)
    InitializeCriticalSection(&h->lock);
    h->owner = GetCurrentThreadId();
#else
    if (pthread_mutex_init(&h->lock, 0) != 0) {
        free(h);
        return 1;
    }
    h->owner = pthread_self();
#endif
    rt->host_sync = h;
    return 0;
}

void lmx_msg_host_detach(LmxMsgRuntime *rt) {
    LmxMsgHostSync *h;
    if (rt == 0 || rt->host_sync == 0) {
        return;
    }
    h = (LmxMsgHostSync *)rt->host_sync;
#if defined(_WIN32)
    DeleteCriticalSection(&h->lock);
#else
    pthread_mutex_destroy(&h->lock);
#endif
    free(h);
    rt->host_sync = 0;
}

int lmx_msg_host_is_owner(LmxMsgRuntime *rt) {
    LmxMsgHostSync *h;
    if (rt == 0 || rt->host_sync == 0) {
        return 0;
    }
    h = (LmxMsgHostSync *)rt->host_sync;
#if defined(_WIN32)
    return h->owner == GetCurrentThreadId();
#else
    return pthread_equal(h->owner, pthread_self()) != 0;
#endif
}

int lmx_msg_host_lock(LmxMsgRuntime *rt) {
    LmxMsgHostSync *h;
    if (rt == 0 || rt->host_sync == 0) {
        return 1;
    }
    h = (LmxMsgHostSync *)rt->host_sync;
#if defined(_WIN32)
    EnterCriticalSection(&h->lock);
#else
    pthread_mutex_lock(&h->lock);
#endif
    return 0;
}

void lmx_msg_host_unlock(LmxMsgRuntime *rt) {
    LmxMsgHostSync *h;
    if (rt == 0 || rt->host_sync == 0) {
        return;
    }
    h = (LmxMsgHostSync *)rt->host_sync;
#if defined(_WIN32)
    LeaveCriticalSection(&h->lock);
#else
    pthread_mutex_unlock(&h->lock);
#endif
}

int lmx_msg_host_is_shutdown(LmxMsgRuntime *rt) {
    LmxMsgHostSync *h;
    if (rt == 0 || rt->host_sync == 0) {
        return 1;
    }
    h = (LmxMsgHostSync *)rt->host_sync;
    return h->shutting_down;
}

int lmx_msg_host_shutdown(LmxMsgRuntime *rt) {
    LmxMsgHostSync *h;
    if (rt == 0 || rt->host_sync == 0) {
        return LMX_MSG_INVALID;
    }
    if (lmx_msg_host_is_owner(rt) == 0) {
        return LMX_MSG_INVALID;
    }
    h = (LmxMsgHostSync *)rt->host_sync;
#if defined(_WIN32)
    EnterCriticalSection(&h->lock);
    h->shutting_down = 1;
    LeaveCriticalSection(&h->lock);
#else
    pthread_mutex_lock(&h->lock);
    h->shutting_down = 1;
    pthread_mutex_unlock(&h->lock);
#endif
    return LMX_MSG_OK;
}

/* S3 (Mikhail 2026-09-15): no round is empty. A round is the thread's end_turn
 * work, then a look into its mailbox, then a turn if there is one; for R0's host
 * drive that round already ran in the caller (its own maintenance work, then its
 * drain/recv looked into the mailbox) before this is called. This only reports
 * whether the host is still up. Only after a round that found neither mail nor
 * work does it yield here -- one implementation tick (SwitchToThread/sched_yield,
 * the coordinator's decision), not part of the model. */
int lmx_msg_host_wait(LmxMsgRuntime *rt, unsigned timeout_ms) {
    LmxMsgHostSync *h;
    int down;
    if (rt == 0 || rt->host_sync == 0) {
        return LMX_MSG_INVALID;
    }
    if (lmx_msg_host_is_owner(rt) == 0) {
        return LMX_MSG_INVALID;
    }
    if (timeout_ms == 0xFFFFFFFFU) {
        return LMX_MSG_INVALID;
    }
    h = (LmxMsgHostSync *)rt->host_sync;
#if defined(_WIN32)
    EnterCriticalSection(&h->lock);
    down = h->shutting_down;
    LeaveCriticalSection(&h->lock);
#else
    pthread_mutex_lock(&h->lock);
    down = h->shutting_down;
    pthread_mutex_unlock(&h->lock);
#endif
    if (down != 0) {
        return LMX_MSG_STOPPED;
    }
#if defined(_WIN32)
    SwitchToThread();
#else
    sched_yield();
#endif
    return LMX_MSG_EMPTY;
}
