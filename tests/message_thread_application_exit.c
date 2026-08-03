#include <stddef.h>
#include <stdio.h>
#include <string.h>

typedef struct LmMessageThread LmMessageThread;
typedef struct LmMessageThreadRuntime LmMessageThreadRuntime;
typedef struct LmMessageThreadPool LmMessageThreadPool;
typedef struct LmMutex LmMutex;
typedef struct LmCondition LmCondition;
typedef void (*LmMessageThreadEntry)(LmMessageThread *thread, void *argument);

const char *lm_thread_provider_name(void);
LmMessageThreadRuntime *lm_message_thread_runtime_new(void);
int lm_message_thread_runtime_delete(LmMessageThreadRuntime *runtime);
int lm_message_thread_runtime_attach_root(
    LmMessageThreadRuntime *runtime,
    LmMessageThread *thread
);
int lm_message_thread_runtime_detach_root(
    LmMessageThreadRuntime *runtime,
    LmMessageThread *thread
);
int lm_message_thread_runtime_exit_state(
    LmMessageThreadRuntime *runtime,
    int *out_requested,
    int *out_ready,
    int *out_status
);
LmMessageThreadPool *lm_message_thread_pool_new(
    LmMessageThreadRuntime *runtime,
    size_t worker_count
);
void lm_message_thread_pool_request_stop(LmMessageThreadPool *pool);
size_t lm_message_thread_pool_pump(
    LmMessageThreadPool *pool,
    size_t max_turns
);
int lm_message_thread_pool_delete(LmMessageThreadPool *pool);
LmMessageThread *lm_message_thread_new(void);
LmMessageThread *lm_message_thread_new_in(LmMessageThreadPool *pool);
void lm_message_thread_delete(LmMessageThread *thread);
int lm_message_thread_start(
    LmMessageThread *thread,
    LmMessageThreadEntry entry,
    void *argument
);
int lm_message_thread_start_mailbox(
    LmMessageThread *thread,
    LmMessageThreadEntry entry,
    void *argument
);
int lm_message_thread_join(LmMessageThread *thread, int *result);
int lm_message_thread_bind_route(
    LmMessageThread *thread,
    const char *route
);
int lm_message_thread_send_lmx(
    LmMessageThread *sender,
    const char *endpoint,
    const char *route,
    const char *lmx,
    size_t length
);
size_t lm_message_thread_inbox_count(const LmMessageThread *thread);
size_t lm_message_thread_outbox_count(const LmMessageThread *thread);
int lm_message_thread_begin_turn(LmMessageThread *thread);
int lm_message_thread_end_turn(LmMessageThread *thread);
int lm_message_thread_request_exit(
    LmMessageThread *requester,
    int status
);
void lm_message_thread_request_stop(LmMessageThread *thread, int status);
int lm_message_thread_status(const LmMessageThread *thread);
size_t lm_message_thread_turn_count(const LmMessageThread *thread);
LmMutex *lm_mutex_new(void);
void lm_mutex_delete(LmMutex *mutex);
int lm_mutex_lock(LmMutex *mutex);
int lm_mutex_unlock(LmMutex *mutex);
LmCondition *lm_condition_new(void);
void lm_condition_delete(LmCondition *condition);
int lm_condition_wait(LmCondition *condition, LmMutex *mutex);
int lm_condition_broadcast(LmCondition *condition);

static int lm_exit_expect_state(
    LmMessageThreadRuntime *runtime,
    int expected_requested,
    int expected_ready,
    int expected_status
) {
    int requested = -1;
    int ready = -1;
    int status = -1;

    return lm_message_thread_runtime_exit_state(
        runtime,
        &requested,
        &ready,
        &status
    ) != 0 || requested != expected_requested ||
        ready != expected_ready || status != expected_status;
}

typedef struct LmRootExitReceiverState {
    size_t calls;
} LmRootExitReceiverState;

static void lm_root_exit_receiver(
    LmMessageThread *thread,
    void *argument
) {
    LmRootExitReceiverState *state =
        (LmRootExitReceiverState *)argument;

    if (state != 0) {
        state->calls += 1U;
    }
    lm_message_thread_request_stop(thread, 0);
}

static int lm_root_exit_test(void) {
    LmRootExitReceiverState state = {0};
    LmMessageThreadRuntime *runtime = lm_message_thread_runtime_new();
    LmMessageThreadPool *pool = 0;
    LmMessageThread *root = lm_message_thread_new();
    LmMessageThread *receiver = 0;
    int attached = 0;
    int receiver_started = 0;
    int receiver_joined = 0;
    int actor_status = -1;
    int status = 0;

    if (runtime == 0 || root == 0 ||
        lm_message_thread_runtime_exit_state(runtime, 0, 0, 0) == 0 ||
        lm_exit_expect_state(runtime, 0, 0, 0)) {
        status = 1;
        goto cleanup;
    }
    pool = lm_message_thread_pool_new(runtime, 1U);
    if (pool == 0 ||
        lm_message_thread_runtime_attach_root(runtime, root) != 0) {
        status = 2;
        goto cleanup;
    }
    attached = 1;
    if (lm_message_thread_request_exit(root, 9) == 0) {
        status = 3;
        goto cleanup;
    }
    receiver = lm_message_thread_new_in(pool);
    if (receiver == 0 || lm_message_thread_start_mailbox(
            receiver,
            lm_root_exit_receiver,
            &state
        ) != 0 || lm_message_thread_bind_route(
            receiver,
            "/root-exit"
        ) != 0) {
        status = 4;
        goto cleanup;
    }
    receiver_started = 1;

    if (lm_message_thread_begin_turn(root) != 1 ||
        lm_message_thread_pool_pump(pool, 1U) != 0U ||
        lm_message_thread_request_exit(root, 17) != 0 ||
        lm_message_thread_send_lmx(
            root,
            0,
            "/root-exit",
            "root",
            4U
        ) != 0 || lm_exit_expect_state(runtime, 1, 0, 17) ||
        lm_message_thread_request_exit(root, 19) != 0 ||
        lm_exit_expect_state(runtime, 1, 0, 17) ||
        lm_message_thread_end_turn(root) != 0 ||
        lm_message_thread_status(root) != 17 ||
        lm_message_thread_outbox_count(root) != 0U ||
        lm_exit_expect_state(runtime, 1, 1, 17)) {
        status = 5;
        goto cleanup;
    }
    if (lm_message_thread_runtime_delete(runtime) == 0 ||
        lm_message_thread_runtime_detach_root(runtime, root) != 0) {
        status = 6;
        goto cleanup;
    }
    attached = 0;
    if (lm_exit_expect_state(runtime, 1, 1, 17)) {
        status = 7;
        goto cleanup;
    }

    lm_message_thread_pool_request_stop(pool);
    if (lm_message_thread_join(receiver, &actor_status) != 0 ||
        actor_status != 0) {
        status = 8;
        goto cleanup;
    }
    receiver_joined = 1;
    if (state.calls + lm_message_thread_inbox_count(receiver) != 1U ||
        lm_message_thread_turn_count(receiver) > 1U) {
        status = 9;
    }

cleanup:
    if (pool != 0) {
        lm_message_thread_pool_request_stop(pool);
        if (receiver_started && !receiver_joined) {
            (void)lm_message_thread_join(receiver, &actor_status);
        }
        if (lm_message_thread_pool_delete(pool) != 0 && status == 0) {
            status = 10;
        }
    }
    if (attached) {
        (void)lm_message_thread_runtime_detach_root(runtime, root);
    }
    lm_message_thread_delete(root);
    if (runtime != 0 && lm_message_thread_runtime_delete(runtime) != 0 &&
        status == 0) {
        status = 11;
    }
    return status;
}

static int lm_root_exit_failure_test(void) {
    LmMessageThreadRuntime *runtime = lm_message_thread_runtime_new();
    LmMessageThread *root = lm_message_thread_new();
    int attached = 0;
    int status = 0;

    if (runtime == 0 || root == 0 ||
        lm_message_thread_runtime_attach_root(runtime, root) != 0) {
        status = 1;
        goto cleanup;
    }
    attached = 1;
    if (lm_message_thread_begin_turn(root) != 1 ||
        lm_message_thread_request_exit(root, 17) != 0 ||
        lm_message_thread_send_lmx(
            root,
            0,
            "/missing",
            "x",
            1U
        ) != 0 || lm_message_thread_end_turn(root) != 64 ||
        lm_message_thread_status(root) != 64 ||
        lm_exit_expect_state(runtime, 1, 1, 64)) {
        status = 2;
        goto cleanup;
    }
    if (lm_message_thread_runtime_detach_root(runtime, root) != 0) {
        status = 3;
        goto cleanup;
    }
    attached = 0;

cleanup:
    if (attached) {
        (void)lm_message_thread_runtime_detach_root(runtime, root);
    }
    lm_message_thread_delete(root);
    if (runtime != 0 && lm_message_thread_runtime_delete(runtime) != 0 &&
        status == 0) {
        status = 4;
    }
    return status;
}

static int lm_root_exit_zero_test(void) {
    LmMessageThreadRuntime *runtime = lm_message_thread_runtime_new();
    LmMessageThread *root = lm_message_thread_new();
    int attached = 0;
    int status = 0;

    if (runtime == 0 || root == 0 ||
        lm_message_thread_runtime_attach_root(runtime, root) != 0) {
        status = 1;
        goto cleanup;
    }
    attached = 1;
    if (lm_message_thread_begin_turn(root) != 1 ||
        lm_message_thread_request_exit(root, 0) != 0 ||
        lm_exit_expect_state(runtime, 1, 0, 0) ||
        lm_message_thread_end_turn(root) != 0 ||
        lm_message_thread_status(root) != 0 ||
        lm_exit_expect_state(runtime, 1, 1, 0)) {
        status = 2;
        goto cleanup;
    }
    if (lm_message_thread_runtime_detach_root(runtime, root) != 0) {
        status = 3;
        goto cleanup;
    }
    attached = 0;

cleanup:
    if (attached) {
        (void)lm_message_thread_runtime_detach_root(runtime, root);
    }
    lm_message_thread_delete(root);
    if (runtime != 0 && lm_message_thread_runtime_delete(runtime) != 0 &&
        status == 0) {
        status = 4;
    }
    return status;
}

static int lm_stop_before_exit_test(void) {
    LmMessageThreadRuntime *runtime = lm_message_thread_runtime_new();
    LmMessageThread *root = lm_message_thread_new();
    int attached = 0;
    int status = 0;

    if (runtime == 0 || root == 0 ||
        lm_message_thread_runtime_attach_root(runtime, root) != 0) {
        status = 1;
        goto cleanup;
    }
    attached = 1;
    if (lm_message_thread_begin_turn(root) != 1) {
        status = 2;
        goto cleanup;
    }
    lm_message_thread_request_stop(root, 3);
    if (lm_message_thread_request_exit(root, 5) != 0 ||
        lm_exit_expect_state(runtime, 1, 0, 5) ||
        lm_message_thread_end_turn(root) != 0 ||
        lm_message_thread_status(root) != 3 ||
        lm_exit_expect_state(runtime, 1, 1, 5)) {
        status = 3;
        goto cleanup;
    }
    if (lm_message_thread_runtime_detach_root(runtime, root) != 0) {
        status = 4;
        goto cleanup;
    }
    attached = 0;

cleanup:
    if (attached) {
        (void)lm_message_thread_runtime_detach_root(runtime, root);
    }
    lm_message_thread_delete(root);
    if (runtime != 0 && lm_message_thread_runtime_delete(runtime) != 0 &&
        status == 0) {
        status = 5;
    }
    return status;
}

typedef struct LmActorExitState {
    LmMessageThreadPool *pool;
    LmMessageThread *peer;
    size_t exit_calls;
    size_t peer_calls;
    size_t sink_calls;
    int invalid_peer_exit;
    size_t callback_pump_count;
    int send_status;
    int first_exit_status;
    int second_exit_status;
} LmActorExitState;

static void lm_actor_exit_requester(
    LmMessageThread *thread,
    void *argument
) {
    LmActorExitState *state = (LmActorExitState *)argument;

    state->exit_calls += 1U;
    state->send_status = lm_message_thread_send_lmx(
        thread,
        0,
        "/actor-exit-sink",
        "committed",
        9U
    );
    state->invalid_peer_exit = lm_message_thread_request_exit(
        state->peer,
        99
    );
    state->callback_pump_count = lm_message_thread_pool_pump(
        state->pool,
        1U
    );
    state->first_exit_status = lm_message_thread_request_exit(thread, 23);
    state->second_exit_status = lm_message_thread_request_exit(thread, 29);
}

static void lm_actor_exit_peer(
    LmMessageThread *thread,
    void *argument
) {
    LmActorExitState *state = (LmActorExitState *)argument;

    state->peer_calls += 1U;
    lm_message_thread_request_stop(thread, 81);
}

static void lm_actor_exit_sink(
    LmMessageThread *thread,
    void *argument
) {
    LmActorExitState *state = (LmActorExitState *)argument;

    state->sink_calls += 1U;
    lm_message_thread_request_stop(thread, 82);
}

static int lm_actor_exit_test(void) {
    LmActorExitState state;
    LmMessageThreadRuntime *runtime = lm_message_thread_runtime_new();
    LmMessageThreadPool *pool = 0;
    LmMessageThread *root = lm_message_thread_new();
    LmMessageThread *requester = 0;
    LmMessageThread *peer = 0;
    LmMessageThread *sink = 0;
    int attached = 0;
    int requester_started = 0;
    int peer_started = 0;
    int sink_started = 0;
    int requester_joined = 0;
    int peer_joined = 0;
    int sink_joined = 0;
    int actor_status = -1;
    int status = 0;

    memset(&state, 0, sizeof(state));
    state.invalid_peer_exit = -1;
    state.send_status = -1;
    state.first_exit_status = -1;
    state.second_exit_status = -1;
    if (runtime == 0 || root == 0) {
        status = 1;
        goto cleanup;
    }
    pool = lm_message_thread_pool_new(runtime, 1U);
    if (pool == 0 ||
        lm_message_thread_runtime_attach_root(runtime, root) != 0) {
        status = 2;
        goto cleanup;
    }
    attached = 1;
    state.pool = pool;
    requester = lm_message_thread_new_in(pool);
    sink = lm_message_thread_new_in(pool);
    peer = lm_message_thread_new_in(pool);
    state.peer = peer;
    if (requester == 0 || sink == 0 || peer == 0 ||
        lm_message_thread_start_mailbox(
            requester,
            lm_actor_exit_requester,
            &state
        ) != 0 || lm_message_thread_bind_route(
            requester,
            "/actor-exit"
        ) != 0) {
        status = 3;
        goto cleanup;
    }
    requester_started = 1;
    if (lm_message_thread_start_mailbox(
            sink,
            lm_actor_exit_sink,
            &state
        ) != 0 || lm_message_thread_bind_route(
            sink,
            "/actor-exit-sink"
        ) != 0) {
        status = 4;
        goto cleanup;
    }
    sink_started = 1;
    if (lm_message_thread_start_mailbox(
            peer,
            lm_actor_exit_peer,
            &state
        ) != 0 || lm_message_thread_bind_route(
            peer,
            "/actor-exit-peer"
        ) != 0) {
        status = 5;
        goto cleanup;
    }
    peer_started = 1;

    if (lm_message_thread_request_exit(requester, 7) == 0 ||
        lm_message_thread_begin_turn(root) != 1 ||
        lm_message_thread_send_lmx(
            root,
            0,
            "/actor-exit",
            "go",
            2U
        ) != 0 || lm_message_thread_end_turn(root) != 0) {
        status = 6;
        goto cleanup;
    }
    if (strcmp(lm_thread_provider_name(), "single") == 0 &&
        lm_message_thread_pool_pump(pool, 1U) != 1U) {
        status = 7;
        goto cleanup;
    }
    if (lm_message_thread_join(requester, &actor_status) != 0 ||
        actor_status != 23) {
        status = 8;
        goto cleanup;
    }
    requester_joined = 1;
    if (state.exit_calls != 1U || state.send_status != 0 ||
        state.invalid_peer_exit != 1 || state.callback_pump_count != 0U ||
        state.first_exit_status != 0 || state.second_exit_status != 0 ||
        lm_exit_expect_state(runtime, 1, 1, 23) ||
        lm_message_thread_outbox_count(requester) != 0U) {
        status = 9;
        goto cleanup;
    }
    if (strcmp(lm_thread_provider_name(), "single") == 0) {
        if (lm_message_thread_pool_pump(pool, 8U) != 2U) {
            status = 10;
            goto cleanup;
        }
    } else if (lm_message_thread_pool_pump(pool, 8U) != 0U) {
        status = 11;
        goto cleanup;
    }
    if (lm_message_thread_join(sink, &actor_status) != 0 ||
        actor_status != 0) {
        status = 12;
        goto cleanup;
    }
    sink_joined = 1;
    if (lm_message_thread_join(peer, &actor_status) != 0 ||
        actor_status != 0) {
        status = 13;
        goto cleanup;
    }
    peer_joined = 1;
    if (state.sink_calls != 0U || state.peer_calls != 0U ||
        lm_message_thread_inbox_count(sink) != 1U ||
        lm_message_thread_turn_count(sink) != 0U ||
        lm_message_thread_turn_count(peer) != 0U) {
        status = 14;
    }

cleanup:
    if (pool != 0) {
        lm_message_thread_pool_request_stop(pool);
        if (requester_started && !requester_joined) {
            (void)lm_message_thread_join(requester, &actor_status);
        }
        if (sink_started && !sink_joined) {
            (void)lm_message_thread_join(sink, &actor_status);
        }
        if (peer_started && !peer_joined) {
            (void)lm_message_thread_join(peer, &actor_status);
        }
        if (lm_message_thread_pool_delete(pool) != 0 && status == 0) {
            status = 15;
        }
    }
    if (attached) {
        (void)lm_message_thread_runtime_detach_root(runtime, root);
    }
    lm_message_thread_delete(root);
    if (runtime != 0 && lm_message_thread_runtime_delete(runtime) != 0 &&
        status == 0) {
        status = 16;
    }
    return status;
}

typedef struct LmExitCancelState {
    LmMutex *mutex;
    LmCondition *condition;
    int blocker_started;
    int release_blocker;
    int send_status_local;
    int send_status_remote;
    int exit_status;
} LmExitCancelState;

static void lm_exit_cancel_blocker(
    LmMessageThread *thread,
    void *argument
) {
    LmExitCancelState *state = (LmExitCancelState *)argument;

    state->send_status_local = lm_message_thread_send_lmx(
        thread,
        0,
        "/cancel-missing",
        "local",
        5U
    );
    state->send_status_remote = lm_message_thread_send_lmx(
        thread,
        "https://example.test",
        "/cancel-remote",
        "remote",
        6U
    );
    if (lm_mutex_lock(state->mutex) != 0) {
        lm_message_thread_request_stop(thread, 91);
        return;
    }
    state->blocker_started = 1;
    (void)lm_condition_broadcast(state->condition);
    while (!state->release_blocker) {
        if (lm_condition_wait(state->condition, state->mutex) != 0) {
            (void)lm_mutex_unlock(state->mutex);
            lm_message_thread_request_stop(thread, 92);
            return;
        }
    }
    (void)lm_mutex_unlock(state->mutex);
}

static void lm_exit_cancel_requester(
    LmMessageThread *thread,
    void *argument
) {
    LmExitCancelState *state = (LmExitCancelState *)argument;

    state->exit_status = lm_message_thread_request_exit(thread, 41);
}

static int lm_exit_cancel_native_test(void) {
    LmExitCancelState state;
    LmMessageThreadRuntime *runtime = 0;
    LmMessageThreadPool *pool = 0;
    LmMessageThread *root = 0;
    LmMessageThread *blocker = 0;
    LmMessageThread *requester = 0;
    int attached = 0;
    int blocker_started = 0;
    int requester_started = 0;
    int blocker_joined = 0;
    int requester_joined = 0;
    int actor_status = -1;
    int status = 0;

    if (strcmp(lm_thread_provider_name(), "single") == 0) {
        return 0;
    }
    memset(&state, 0, sizeof(state));
    state.send_status_local = -1;
    state.send_status_remote = -1;
    state.exit_status = -1;
    state.mutex = lm_mutex_new();
    state.condition = lm_condition_new();
    runtime = lm_message_thread_runtime_new();
    root = lm_message_thread_new();
    if (state.mutex == 0 || state.condition == 0 || runtime == 0 ||
        root == 0) {
        status = 1;
        goto cleanup;
    }
    pool = lm_message_thread_pool_new(runtime, 2U);
    if (pool == 0 ||
        lm_message_thread_runtime_attach_root(runtime, root) != 0) {
        status = 2;
        goto cleanup;
    }
    attached = 1;
    blocker = lm_message_thread_new_in(pool);
    requester = lm_message_thread_new_in(pool);
    if (blocker == 0 || requester == 0 || lm_message_thread_start(
            blocker,
            lm_exit_cancel_blocker,
            &state
        ) != 0) {
        status = 3;
        goto cleanup;
    }
    blocker_started = 1;
    if (lm_mutex_lock(state.mutex) != 0) {
        status = 4;
        goto cleanup;
    }
    while (!state.blocker_started) {
        if (lm_condition_wait(state.condition, state.mutex) != 0) {
            status = 5;
            break;
        }
    }
    (void)lm_mutex_unlock(state.mutex);
    if (status != 0) {
        goto cleanup;
    }
    if (lm_message_thread_start(
            requester,
            lm_exit_cancel_requester,
            &state
        ) != 0) {
        status = 6;
        goto cleanup;
    }
    requester_started = 1;
    if (lm_message_thread_join(requester, &actor_status) != 0 ||
        actor_status != 41 || state.exit_status != 0 ||
        lm_exit_expect_state(runtime, 1, 1, 41)) {
        status = 7;
        goto cleanup;
    }
    requester_joined = 1;
    if (lm_mutex_lock(state.mutex) != 0) {
        status = 8;
        goto cleanup;
    }
    state.release_blocker = 1;
    (void)lm_condition_broadcast(state.condition);
    (void)lm_mutex_unlock(state.mutex);
    if (lm_message_thread_join(blocker, &actor_status) != 0 ||
        actor_status != 0 || state.send_status_local != 0 ||
        state.send_status_remote != 0 ||
        lm_message_thread_outbox_count(blocker) != 0U ||
        lm_message_thread_status(blocker) != 0) {
        status = 9;
        goto cleanup;
    }
    blocker_joined = 1;

cleanup:
    if (state.mutex != 0 && lm_mutex_lock(state.mutex) == 0) {
        state.release_blocker = 1;
        if (state.condition != 0) {
            (void)lm_condition_broadcast(state.condition);
        }
        (void)lm_mutex_unlock(state.mutex);
    }
    if (pool != 0) {
        lm_message_thread_pool_request_stop(pool);
        if (requester_started && !requester_joined) {
            (void)lm_message_thread_join(requester, &actor_status);
        }
        if (blocker_started && !blocker_joined) {
            (void)lm_message_thread_join(blocker, &actor_status);
        }
        if (lm_message_thread_pool_delete(pool) != 0 && status == 0) {
            status = 10;
        }
    }
    if (attached) {
        (void)lm_message_thread_runtime_detach_root(runtime, root);
    }
    lm_message_thread_delete(root);
    if (runtime != 0 && lm_message_thread_runtime_delete(runtime) != 0 &&
        status == 0) {
        status = 11;
    }
    lm_condition_delete(state.condition);
    lm_mutex_delete(state.mutex);
    return status;
}

typedef struct LmExitFirstWinsState {
    LmMutex *mutex;
    LmCondition *condition;
    int winner_requested;
    int loser_done;
    int winner_result;
    int loser_result;
} LmExitFirstWinsState;

static void lm_exit_first_winner(
    LmMessageThread *thread,
    void *argument
) {
    LmExitFirstWinsState *state = (LmExitFirstWinsState *)argument;

    state->winner_result = lm_message_thread_request_exit(thread, 0);
    if (lm_mutex_lock(state->mutex) != 0) {
        lm_message_thread_request_stop(thread, 93);
        return;
    }
    state->winner_requested = 1;
    (void)lm_condition_broadcast(state->condition);
    while (!state->loser_done) {
        if (lm_condition_wait(state->condition, state->mutex) != 0) {
            (void)lm_mutex_unlock(state->mutex);
            lm_message_thread_request_stop(thread, 94);
            return;
        }
    }
    (void)lm_mutex_unlock(state->mutex);
}

static void lm_exit_first_loser(
    LmMessageThread *thread,
    void *argument
) {
    LmExitFirstWinsState *state = (LmExitFirstWinsState *)argument;

    if (lm_mutex_lock(state->mutex) != 0) {
        lm_message_thread_request_stop(thread, 95);
        return;
    }
    while (!state->winner_requested) {
        if (lm_condition_wait(state->condition, state->mutex) != 0) {
            (void)lm_mutex_unlock(state->mutex);
            lm_message_thread_request_stop(thread, 96);
            return;
        }
    }
    (void)lm_mutex_unlock(state->mutex);
    state->loser_result = lm_message_thread_request_exit(thread, 9);
    if (lm_mutex_lock(state->mutex) == 0) {
        state->loser_done = 1;
        (void)lm_condition_broadcast(state->condition);
        (void)lm_mutex_unlock(state->mutex);
    }
}

static int lm_exit_first_wins_native_test(void) {
    LmExitFirstWinsState state;
    LmMessageThreadRuntime *runtime = 0;
    LmMessageThreadPool *pool = 0;
    LmMessageThread *root = 0;
    LmMessageThread *winner = 0;
    LmMessageThread *loser = 0;
    int attached = 0;
    int winner_started = 0;
    int loser_started = 0;
    int winner_joined = 0;
    int loser_joined = 0;
    int actor_status = -1;
    int status = 0;

    if (strcmp(lm_thread_provider_name(), "single") == 0) {
        return 0;
    }
    memset(&state, 0, sizeof(state));
    state.winner_result = -1;
    state.loser_result = -1;
    state.mutex = lm_mutex_new();
    state.condition = lm_condition_new();
    runtime = lm_message_thread_runtime_new();
    root = lm_message_thread_new();
    if (state.mutex == 0 || state.condition == 0 || runtime == 0 ||
        root == 0) {
        status = 1;
        goto cleanup;
    }
    pool = lm_message_thread_pool_new(runtime, 2U);
    if (pool == 0 ||
        lm_message_thread_runtime_attach_root(runtime, root) != 0) {
        status = 2;
        goto cleanup;
    }
    attached = 1;
    winner = lm_message_thread_new_in(pool);
    loser = lm_message_thread_new_in(pool);
    if (winner == 0 || loser == 0 || lm_message_thread_start(
            winner,
            lm_exit_first_winner,
            &state
        ) != 0) {
        status = 3;
        goto cleanup;
    }
    winner_started = 1;
    if (lm_message_thread_start(
            loser,
            lm_exit_first_loser,
            &state
        ) != 0) {
        status = 4;
        goto cleanup;
    }
    loser_started = 1;
    if (lm_message_thread_join(winner, &actor_status) != 0 ||
        actor_status != 0) {
        status = 5;
        goto cleanup;
    }
    winner_joined = 1;
    if (lm_message_thread_join(loser, &actor_status) != 0 ||
        actor_status != 0) {
        status = 6;
        goto cleanup;
    }
    loser_joined = 1;
    if (state.winner_result != 0 || state.loser_result != 0 ||
        lm_exit_expect_state(runtime, 1, 1, 0)) {
        status = 7;
    }

cleanup:
    if (state.mutex != 0 && lm_mutex_lock(state.mutex) == 0) {
        state.winner_requested = 1;
        state.loser_done = 1;
        if (state.condition != 0) {
            (void)lm_condition_broadcast(state.condition);
        }
        (void)lm_mutex_unlock(state.mutex);
    }
    if (pool != 0) {
        lm_message_thread_pool_request_stop(pool);
        if (winner_started && !winner_joined) {
            (void)lm_message_thread_join(winner, &actor_status);
        }
        if (loser_started && !loser_joined) {
            (void)lm_message_thread_join(loser, &actor_status);
        }
        if (lm_message_thread_pool_delete(pool) != 0 && status == 0) {
            status = 8;
        }
    }
    if (attached) {
        (void)lm_message_thread_runtime_detach_root(runtime, root);
    }
    lm_message_thread_delete(root);
    if (runtime != 0 && lm_message_thread_runtime_delete(runtime) != 0 &&
        status == 0) {
        status = 9;
    }
    lm_condition_delete(state.condition);
    lm_mutex_delete(state.mutex);
    return status;
}

int main(void) {
    int status;

    status = lm_root_exit_test();
    if (status != 0) {
        fprintf(stderr, "root exit test failed: %d\n", status);
        return 10 + status;
    }
    status = lm_root_exit_failure_test();
    if (status != 0) {
        fprintf(stderr, "root exit failure test failed: %d\n", status);
        return 30 + status;
    }
    status = lm_root_exit_zero_test();
    if (status != 0) {
        fprintf(stderr, "root exit zero test failed: %d\n", status);
        return 40 + status;
    }
    status = lm_stop_before_exit_test();
    if (status != 0) {
        fprintf(stderr, "stop-before-exit test failed: %d\n", status);
        return 45 + status;
    }
    status = lm_actor_exit_test();
    if (status != 0) {
        fprintf(
            stderr,
            "%s actor exit test failed: %d\n",
            lm_thread_provider_name(),
            status
        );
        return 50 + status;
    }
    status = lm_exit_cancel_native_test();
    if (status != 0) {
        fprintf(stderr, "native exit cancellation test failed: %d\n", status);
        return 80 + status;
    }
    status = lm_exit_first_wins_native_test();
    if (status != 0) {
        fprintf(stderr, "native exit first-wins test failed: %d\n", status);
        return 100 + status;
    }
    return 0;
}
