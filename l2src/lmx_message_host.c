/* LMX_MSG_HOST_INGRESS_V0 platform primitives only: owner id, mutex.
 * Copy/envelope/queue policy lives in lmx_message.lm1. S3 (Mikhail 2026-09-15):
 * no wait, no wake -- the owner loops, checked each round in its own caller. */
#include "l2src/lmx_message_host.h"
#include <stdlib.h>
#if defined(LMX_MSG_EXEC_TEST)
#include "l2src/lmx_message_exec.h"
#else
#define lmx_msg_test_wake_site(s, o) ((void)0)
#endif

#if defined(LMX_MSG_HOST_TEST)
int lmx_msg_host_test_nomem;

int lmx_msg_host_test_get_nomem(void) {
    return lmx_msg_host_test_nomem;
}

void lmx_msg_host_test_set_nomem(int v) {
    lmx_msg_host_test_nomem = v;
}
#else
int lmx_msg_host_test_get_nomem(void) {
    return 0;
}
#endif

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
/* S6 (c): no host lock. shutting_down is one atomic flag whose writer is the host
 * (runtime_shutdown sets it under R0's mailbox monitor); host_post checks it under
 * the same monitor, so a late post is refused there. */
typedef struct LmxMsgHostSync {
    DWORD owner;
    int shutting_down;
} LmxMsgHostSync;
#else
#include <pthread.h>
#include <sched.h>
typedef struct LmxMsgHostSync {
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
    h->owner = GetCurrentThreadId();
#else
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

int lmx_msg_host_is_shutdown(LmxMsgRuntime *rt) {
    LmxMsgHostSync *h;
    if (rt == 0 || rt->host_sync == 0) {
        return 1;
    }
    h = (LmxMsgHostSync *)rt->host_sync;
    return __atomic_load_n(&h->shutting_down, __ATOMIC_RELAXED);
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
    __atomic_store_n(&h->shutting_down, 1, __ATOMIC_RELAXED);
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
    down = __atomic_load_n(&h->shutting_down, __ATOMIC_RELAXED);
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
