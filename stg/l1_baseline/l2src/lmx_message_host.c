/* LMX_MSG_HOST_INGRESS_V0: mutex+wake around the host ingress queue only.
 *
 * Choice: the thread that calls lmx_msg_runtime_new owns the runtime.
 * create/send/pump/recv/drive stay owner-only. Foreign threads may only
 * lmx_msg_host_post. No OS thread per Message. Linearization of posts is
 * mutex acquisition order (one FIFO); each producer is FIFO with itself.
 * Wake is optional; drain on the owner loop is the contract.
 */
#include "l2src/lmx_message_host.h"
#include <stdlib.h>
#include <string.h>

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

static uchar *host_copy_bytes(const uchar *src, size_t n) {
    uchar *p;
    if (n == 0U) {
        return 0;
    }
    if (src == 0) {
        return 0;
    }
    p = (uchar *)malloc(n);
    if (p == 0) {
        return 0;
    }
    memcpy(p, src, n);
    return p;
}

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

int lmx_msg_host_post(LmxMsgRuntime *rt, LmxMsgAddr dest, const LmxMsgEnv *env) {
    LmxMsgHostSync *h;
    LmxMsgCopy *node;
    uchar *bytes;
    if (rt == 0 || rt->host_sync == 0 || env == 0 || dest == 0U) {
        return LMX_MSG_INVALID;
    }
    h = (LmxMsgHostSync *)rt->host_sync;
#if defined(_WIN32)
    EnterCriticalSection(&h->lock);
#else
    pthread_mutex_lock(&h->lock);
#endif
    if (h->shutting_down != 0) {
#if defined(_WIN32)
        LeaveCriticalSection(&h->lock);
#else
        pthread_mutex_unlock(&h->lock);
#endif
        return LMX_MSG_STOPPED;
    }
    node = (LmxMsgCopy *)calloc(1U, sizeof(LmxMsgCopy));
    if (node == 0) {
#if defined(_WIN32)
        LeaveCriticalSection(&h->lock);
#else
        pthread_mutex_unlock(&h->lock);
#endif
        return LMX_MSG_NOMEM;
    }
    bytes = host_copy_bytes(env->bytes, env->n);
    if (env->n != 0U && bytes == 0) {
        free(node);
#if defined(_WIN32)
        LeaveCriticalSection(&h->lock);
#else
        pthread_mutex_unlock(&h->lock);
#endif
        return LMX_MSG_NOMEM;
    }
    node->id = env->id;
    node->correlation = env->correlation;
    node->reply_to = env->reply_to;
    node->from = LMX_MSG_HOST_FROM;
    node->to = dest;
    node->kind = env->kind;
    node->number = env->number;
    node->bytes = bytes;
    node->n = env->n;
    node->next = 0;
    if (rt->host_head == 0) {
        rt->host_head = node;
        rt->host_tail = node;
    } else {
        rt->host_tail->next = node;
        rt->host_tail = node;
    }
#if defined(_WIN32)
    SetEvent(h->wake);
    LeaveCriticalSection(&h->lock);
#else
    h->signaled = 1;
    pthread_cond_signal(&h->wake);
    pthread_mutex_unlock(&h->lock);
#endif
    return LMX_MSG_STAGED;
}

LmxMsgCopy *lmx_msg_host_steal(LmxMsgRuntime *rt) {
    LmxMsgHostSync *h;
    LmxMsgCopy *head;
    if (rt == 0 || rt->host_sync == 0) {
        return 0;
    }
    h = (LmxMsgHostSync *)rt->host_sync;
#if defined(_WIN32)
    EnterCriticalSection(&h->lock);
#else
    pthread_mutex_lock(&h->lock);
#endif
    head = rt->host_head;
    rt->host_head = 0;
    rt->host_tail = 0;
#if defined(_WIN32)
    LeaveCriticalSection(&h->lock);
#else
    pthread_mutex_unlock(&h->lock);
#endif
    return head;
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
    h = (LmxMsgHostSync *)rt->host_sync;
#if defined(_WIN32)
    WaitForSingleObject(h->wake, timeout_ms);
#else
    {
        struct timespec ts;
        pthread_mutex_lock(&h->lock);
        if (h->signaled == 0) {
            clock_gettime(CLOCK_REALTIME, &ts);
            ts.tv_sec += (time_t)(timeout_ms / 1000U);
            ts.tv_nsec += (long)(timeout_ms % 1000U) * 1000000L;
            if (ts.tv_nsec >= 1000000000L) {
                ts.tv_sec += 1;
                ts.tv_nsec -= 1000000000L;
            }
            pthread_cond_timedwait(&h->wake, &h->lock, &ts);
        }
        h->signaled = 0;
        pthread_mutex_unlock(&h->lock);
    }
#endif
    return LMX_MSG_OK;
}
