/* LMX_MSG_HOST_INGRESS_V0 platform primitives only: owner id, mutex, wake.
 * Copy/envelope/queue policy lives in lmx_message.lm1. */
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
    HANDLE wake;
    DWORD owner;
    int shutting_down;
} LmxMsgHostSync;
#else
#include <pthread.h>
#include <time.h>
typedef struct LmxMsgHostSync {
    pthread_mutex_t lock;
    pthread_cond_t wake;
    pthread_t owner;
    int shutting_down;
    int signaled;
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
    h->wake = CreateEventA(0, 0, 0, 0);
    if (h->wake == 0) {
        DeleteCriticalSection(&h->lock);
        free(h);
        return 1;
    }
    h->owner = GetCurrentThreadId();
#else
    if (pthread_mutex_init(&h->lock, 0) != 0) {
        free(h);
        return 1;
    }
    if (pthread_cond_init(&h->wake, 0) != 0) {
        pthread_mutex_destroy(&h->lock);
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
    CloseHandle(h->wake);
    DeleteCriticalSection(&h->lock);
#else
    pthread_cond_destroy(&h->wake);
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

int lmx_msg_host_wake(LmxMsgRuntime *rt) {
    LmxMsgHostSync *h;
    if (rt == 0 || rt->host_sync == 0) {
        return 1;
    }
#if defined(LMX_MSG_HOST_TEST)
    if (lmx_msg_host_test_nowake != 0) {
        return 1;
    }
#endif
    h = (LmxMsgHostSync *)rt->host_sync;
#if defined(_WIN32)
    if (SetEvent(h->wake) == 0) {
        return 1;
    }
#else
    h->signaled = 1;
    pthread_cond_signal(&h->wake);
#endif
    return 0;
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
    SetEvent(h->wake);
    LeaveCriticalSection(&h->lock);
#else
    pthread_mutex_lock(&h->lock);
    h->shutting_down = 1;
    h->signaled = 1;
    pthread_cond_broadcast(&h->wake);
    pthread_mutex_unlock(&h->lock);
#endif
    return LMX_MSG_OK;
}

int lmx_msg_host_wait(LmxMsgRuntime *rt, unsigned timeout_ms) {
    LmxMsgHostSync *h;
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
    {
        DWORD w = WaitForSingleObject(h->wake, timeout_ms);
        if (w == WAIT_OBJECT_0) {
            return LMX_MSG_OK;
        }
        if (w == WAIT_TIMEOUT) {
            return LMX_MSG_EMPTY;
        }
        return LMX_MSG_INVALID;
    }
#else
    {
        struct timespec ts;
        int rc;
        pthread_mutex_lock(&h->lock);
        if (h->signaled != 0) {
            h->signaled = 0;
            pthread_mutex_unlock(&h->lock);
            return LMX_MSG_OK;
        }
        if (timeout_ms == 0U) {
            pthread_mutex_unlock(&h->lock);
            return LMX_MSG_EMPTY;
        }
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += (time_t)(timeout_ms / 1000U);
        ts.tv_nsec += (long)(timeout_ms % 1000U) * 1000000L;
        if (ts.tv_nsec >= 1000000000L) {
            ts.tv_sec += 1;
            ts.tv_nsec -= 1000000000L;
        }
        rc = pthread_cond_timedwait(&h->wake, &h->lock, &ts);
        if (h->signaled != 0) {
            h->signaled = 0;
            pthread_mutex_unlock(&h->lock);
            return LMX_MSG_OK;
        }
        pthread_mutex_unlock(&h->lock);
        if (rc == 0) {
            return LMX_MSG_OK;
        }
        return LMX_MSG_EMPTY;
    }
#endif
}
