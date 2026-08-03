#include <stddef.h>
#include <stdio.h>
#include <string.h>

typedef struct LmMessageThread LmMessageThread;
typedef struct LmMessageThreadRuntime LmMessageThreadRuntime;
typedef struct LmMessageThreadPool LmMessageThreadPool;
typedef struct LmMutex LmMutex;
typedef struct LmCondition LmCondition;
typedef void (*LmMessageThreadEntry)(LmMessageThread *thread, void *argument);

enum {
    LM_MESSAGE_STATUS_ROUTE_NOT_FOUND = 64,
    LM_MESSAGE_STATUS_TRANSPORT_PROVIDER_NOT_CONFIGURED = 65,
    LM_MESSAGE_STATUS_INVALID_ADDRESS = 66
};

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
LmMessageThreadPool *lm_message_thread_pool_new(
    LmMessageThreadRuntime *runtime,
    size_t worker_count
);
void lm_message_thread_pool_request_stop(LmMessageThreadPool *pool);
void lm_message_thread_pool_request_stop_when_idle(
    LmMessageThreadPool *pool
);
int lm_message_thread_pool_delete(LmMessageThreadPool *pool);
LmMessageThread *lm_message_thread_new(void);
LmMessageThread *lm_message_thread_new_in(LmMessageThreadPool *pool);
void lm_message_thread_delete(LmMessageThread *thread);
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
int lm_message_thread_current_lmx(
    LmMessageThread *thread,
    const char **out_lmx,
    size_t *out_length
);
size_t lm_message_thread_inbox_count(const LmMessageThread *thread);
size_t lm_message_thread_outbox_count(const LmMessageThread *thread);
int lm_message_thread_begin_turn(LmMessageThread *thread);
int lm_message_thread_end_turn(LmMessageThread *thread);
void lm_message_thread_request_failure(
    LmMessageThread *thread,
    int status
);
int lm_message_thread_status(const LmMessageThread *thread);
size_t lm_message_thread_turn_count(const LmMessageThread *thread);
size_t lm_message_thread_collection_count(const LmMessageThread *thread);
LmMutex *lm_mutex_new(void);
void lm_mutex_delete(LmMutex *mutex);
int lm_mutex_lock(LmMutex *mutex);
int lm_mutex_unlock(LmMutex *mutex);
LmCondition *lm_condition_new(void);
void lm_condition_delete(LmCondition *condition);
int lm_condition_wait(LmCondition *condition, LmMutex *mutex);
int lm_condition_broadcast(LmCondition *condition);

typedef struct LmRootDrainState {
    size_t received;
    size_t failed_received;
    int error;
    int release_failing;
    LmMutex *gate_mutex;
    LmCondition *gate_condition;
} LmRootDrainState;

typedef struct LmRootPartialState {
    size_t received;
    int error;
} LmRootPartialState;

static void lm_root_partial_receiver(
    LmMessageThread *thread,
    void *argument
) {
    LmRootPartialState *state = (LmRootPartialState *)argument;
    const char *lmx = 0;
    size_t length = 0U;

    if (state == 0 || lm_message_thread_current_lmx(
            thread,
            &lmx,
            &length
        ) != 0 || length != 1U || lmx == 0 || lmx[0] != 'x') {
        if (state != 0) {
            state->error = 1;
        }
        lm_message_thread_request_failure(thread, 96);
        return;
    }
    state->received += 1U;
}

static void lm_root_drain_receiver(
    LmMessageThread *thread,
    void *argument
) {
    LmRootDrainState *state = (LmRootDrainState *)argument;
    const char *lmx = 0;
    size_t length = 0U;
    int value;

    if (state == 0 || lm_message_thread_current_lmx(
            thread,
            &lmx,
            &length
        ) != 0 || length != 1U || lmx == 0 ||
        lmx[0] < '0' || lmx[0] > '9') {
        if (state != 0) {
            state->error = 1;
        }
        lm_message_thread_request_failure(thread, 91);
        return;
    }
    value = lmx[0] - '0';
    state->received += 1U;
    if (value > 0) {
        char next[2];

        next[0] = (char)('0' + value - 1);
        next[1] = '\0';
        if (lm_message_thread_send_lmx(
                thread,
                0,
                "/cascade",
                next,
                1U
            ) != 0) {
            state->error = 2;
            lm_message_thread_request_failure(thread, 92);
        }
    }
}

static void lm_root_drain_failing_receiver(
    LmMessageThread *thread,
    void *argument
) {
    LmRootDrainState *state = (LmRootDrainState *)argument;
    const char *lmx = 0;
    size_t length = 0U;

    if (state == 0 || lm_message_thread_current_lmx(
            thread,
            &lmx,
            &length
        ) != 0 || length != 1U || lmx == 0 || lmx[0] != 'f') {
        if (state != 0) {
            state->error = 3;
        }
        lm_message_thread_request_failure(thread, 93);
        return;
    }
    if (lm_mutex_lock(state->gate_mutex) != 0) {
        state->error = 5;
        lm_message_thread_request_failure(thread, 94);
        return;
    }
    while (!state->release_failing) {
        if (lm_condition_wait(
                state->gate_condition,
                state->gate_mutex
            ) != 0) {
            state->error = 6;
            (void)lm_mutex_unlock(state->gate_mutex);
            lm_message_thread_request_failure(thread, 95);
            return;
        }
    }
    (void)lm_mutex_unlock(state->gate_mutex);
    state->failed_received += 1U;
    if (lm_message_thread_send_lmx(
            thread,
            0,
            "/cascade",
            "9",
            1U
        ) != 0) {
        state->error = 4;
    }
    lm_message_thread_request_failure(thread, 78);
}

static int lm_root_address_status_test(
    const char *endpoint,
    const char *route,
    int expected_status
) {
    LmMessageThreadRuntime *runtime = lm_message_thread_runtime_new();
    LmMessageThread *root = lm_message_thread_new();
    int status = 0;

    if (runtime == 0 || root == 0) {
        status = 1;
        goto cleanup;
    }
    if (lm_message_thread_runtime_attach_root(runtime, root) != 0 ||
        lm_message_thread_runtime_delete(runtime) == 0 ||
        lm_message_thread_send_lmx(root, endpoint, route, "x", 1U) == 0 ||
        lm_message_thread_begin_turn(root) != 1 ||
        lm_message_thread_begin_turn(root) != 0 ||
        lm_message_thread_runtime_detach_root(runtime, root) == 0) {
        status = 2;
        goto cleanup;
    }
    if (lm_message_thread_send_lmx(root, endpoint, route, "x", 1U) != 0 ||
        lm_message_thread_outbox_count(root) != 1U ||
        lm_message_thread_end_turn(root) != expected_status ||
        lm_message_thread_outbox_count(root) != 0U ||
        lm_message_thread_status(root) != expected_status) {
        status = 3;
        goto cleanup;
    }
    if (lm_message_thread_runtime_detach_root(runtime, root) != 0) {
        status = 4;
    }

cleanup:
    if (runtime != 0 && root != 0) {
        (void)lm_message_thread_runtime_detach_root(runtime, root);
    }
    lm_message_thread_delete(root);
    if (runtime != 0 && lm_message_thread_runtime_delete(runtime) != 0 &&
        status == 0) {
        status = 5;
    }
    return status;
}

static int lm_root_invalid_address_test(void) {
    LmMessageThreadRuntime *runtime = lm_message_thread_runtime_new();
    LmMessageThread *root = lm_message_thread_new();
    int status = 0;

    if (runtime == 0 || root == 0 ||
        lm_message_thread_runtime_attach_root(runtime, root) != 0 ||
        lm_message_thread_begin_turn(root) != 1) {
        status = 1;
        goto cleanup;
    }
    if (lm_message_thread_send_lmx(
            root,
            0,
            "/bad\\route",
            "x",
            1U
        ) != LM_MESSAGE_STATUS_INVALID_ADDRESS ||
        lm_message_thread_send_lmx(
            root,
            "file://host",
            "/route",
            "x",
            1U
        ) != LM_MESSAGE_STATUS_INVALID_ADDRESS ||
        lm_message_thread_outbox_count(root) != 0U ||
        lm_message_thread_end_turn(root) != 0) {
        status = 2;
        goto cleanup;
    }
    if (lm_message_thread_runtime_detach_root(runtime, root) != 0) {
        status = 3;
    }

cleanup:
    if (runtime != 0 && root != 0) {
        (void)lm_message_thread_runtime_detach_root(runtime, root);
    }
    lm_message_thread_delete(root);
    if (runtime != 0 && lm_message_thread_runtime_delete(runtime) != 0 &&
        status == 0) {
        status = 4;
    }
    return status;
}

static int lm_root_partial_commit_test(void) {
    LmRootPartialState state = {0};
    LmMessageThreadRuntime *runtime = lm_message_thread_runtime_new();
    LmMessageThreadPool *pool = 0;
    LmMessageThread *root = lm_message_thread_new();
    LmMessageThread *receiver = 0;
    int root_attached = 0;
    int receiver_started = 0;
    int receiver_joined = 0;
    int actor_status = -1;
    int status = 0;

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
    root_attached = 1;
    receiver = lm_message_thread_new_in(pool);
    if (receiver == 0 || lm_message_thread_start_mailbox(
            receiver,
            lm_root_partial_receiver,
            &state
        ) != 0 || lm_message_thread_bind_route(
            receiver,
            "/partial"
        ) != 0) {
        status = 3;
        goto cleanup;
    }
    receiver_started = 1;
    if (lm_message_thread_begin_turn(root) != 1 ||
        lm_message_thread_send_lmx(
            root,
            0,
            "/partial",
            "x",
            1U
        ) != 0 || lm_message_thread_send_lmx(
            root,
            "https://example.test",
            "/remote",
            "y",
            1U
        ) != 0 || lm_message_thread_end_turn(root) !=
            LM_MESSAGE_STATUS_TRANSPORT_PROVIDER_NOT_CONFIGURED) {
        status = 4;
        goto cleanup;
    }
    if (lm_message_thread_runtime_detach_root(runtime, root) != 0) {
        status = 5;
        goto cleanup;
    }
    root_attached = 0;
    lm_message_thread_pool_request_stop_when_idle(pool);
    if (lm_message_thread_join(receiver, &actor_status) != 0 ||
        actor_status != 0) {
        status = 6;
        goto cleanup;
    }
    receiver_joined = 1;
    if (state.error != 0 || state.received != 1U ||
        lm_message_thread_turn_count(receiver) != 1U) {
        status = 7;
    }

cleanup:
    if (pool != 0) {
        if (receiver_started && !receiver_joined) {
            lm_message_thread_pool_request_stop(pool);
            (void)lm_message_thread_join(receiver, &actor_status);
        }
        if (lm_message_thread_pool_delete(pool) != 0 && status == 0) {
            status = 8;
        }
    }
    if (root_attached) {
        (void)lm_message_thread_runtime_detach_root(runtime, root);
    }
    lm_message_thread_delete(root);
    if (runtime != 0 && lm_message_thread_runtime_delete(runtime) != 0 &&
        status == 0) {
        status = 9;
    }
    return status;
}

static int lm_root_drain_test(void) {
    LmRootDrainState state = {0};
    LmMessageThreadRuntime *runtime = 0;
    LmMessageThreadPool *pool = 0;
    LmMessageThread *root = 0;
    LmMessageThread *receiver = 0;
    LmMessageThread *failing_receiver = 0;
    int actor_status = -1;
    int receiver_started = 0;
    int receiver_joined = 0;
    int failing_receiver_started = 0;
    int failing_receiver_joined = 0;
    int status = 0;

    state.gate_mutex = lm_mutex_new();
    state.gate_condition = lm_condition_new();
    if (state.gate_mutex == 0 || state.gate_condition == 0) {
        status = 14;
        goto cleanup;
    }
    runtime = lm_message_thread_runtime_new();
    root = lm_message_thread_new();
    pool = lm_message_thread_pool_new(runtime, 2U);
    if (runtime == 0 || root == 0 || pool == 0 ||
        lm_message_thread_runtime_attach_root(runtime, root) != 0) {
        status = 1;
        goto cleanup;
    }
    receiver = lm_message_thread_new_in(pool);
    if (receiver == 0 || lm_message_thread_start_mailbox(
            receiver,
            lm_root_drain_receiver,
            &state
        ) != 0 || lm_message_thread_bind_route(
            receiver,
            "/cascade"
        ) != 0) {
        status = 2;
        goto cleanup;
    }
    receiver_started = 1;
    failing_receiver = lm_message_thread_new_in(pool);
    if (failing_receiver == 0 || lm_message_thread_start_mailbox(
            failing_receiver,
            lm_root_drain_failing_receiver,
            &state
        ) != 0 || lm_message_thread_bind_route(
            failing_receiver,
            "/fail"
        ) != 0) {
        status = 12;
        goto cleanup;
    }
    failing_receiver_started = 1;

    if (lm_message_thread_begin_turn(root) != 1 ||
        lm_message_thread_send_lmx(
            root,
            0,
            "/cascade",
            "4",
            1U
        ) != 0 || lm_message_thread_send_lmx(
            root,
            0,
            "/fail",
            "f",
            1U
        ) != 0 || lm_message_thread_send_lmx(
            root,
            0,
            "/fail",
            "f",
            1U
        ) != 0 || lm_message_thread_end_turn(root) != 0) {
        status = 3;
        goto cleanup;
    }
    if (lm_mutex_lock(state.gate_mutex) != 0) {
        status = 15;
        goto cleanup;
    }
    state.release_failing = 1;
    if (lm_condition_broadcast(state.gate_condition) != 0) {
        status = 16;
    }
    (void)lm_mutex_unlock(state.gate_mutex);
    if (status != 0) {
        goto cleanup;
    }
    if (lm_message_thread_begin_turn(root) != 1 ||
        lm_message_thread_send_lmx(
            root,
            0,
            "/cascade",
            "9",
            1U
        ) != 0) {
        status = 4;
        goto cleanup;
    }
    lm_message_thread_request_failure(root, 77);
    if (lm_message_thread_end_turn(root) != 77 ||
        lm_message_thread_status(root) != 77 ||
        lm_message_thread_outbox_count(root) != 0U) {
        status = 5;
        goto cleanup;
    }

    lm_message_thread_pool_request_stop_when_idle(pool);
    if (lm_message_thread_new_in(pool) != 0 ||
        lm_message_thread_bind_route(receiver, "/after-drain") == 0) {
        status = 6;
        goto cleanup;
    }
    if (lm_message_thread_join(receiver, &actor_status) != 0 ||
        actor_status != 0) {
        status = 7;
        goto cleanup;
    }
    receiver_joined = 1;
    if (lm_message_thread_join(failing_receiver, &actor_status) != 0 ||
        actor_status != 78) {
        status = 13;
        goto cleanup;
    }
    failing_receiver_joined = 1;
    if (state.error != 0 || state.received != 5U ||
        state.failed_received != 1U ||
        lm_message_thread_turn_count(receiver) != 5U ||
        lm_message_thread_collection_count(receiver) < 5U ||
        lm_message_thread_inbox_count(receiver) != 0U ||
        lm_message_thread_outbox_count(receiver) != 0U ||
        lm_message_thread_turn_count(failing_receiver) != 1U ||
        lm_message_thread_inbox_count(failing_receiver) != 1U ||
        lm_message_thread_outbox_count(failing_receiver) != 0U) {
        status = 8;
    }

cleanup:
    if (state.gate_mutex != 0 &&
        lm_mutex_lock(state.gate_mutex) == 0) {
        state.release_failing = 1;
        if (state.gate_condition != 0) {
            (void)lm_condition_broadcast(state.gate_condition);
        }
        (void)lm_mutex_unlock(state.gate_mutex);
    }
    if (pool != 0) {
        if (receiver_started && !receiver_joined) {
            lm_message_thread_pool_request_stop(pool);
            (void)lm_message_thread_join(receiver, &actor_status);
        }
        if (failing_receiver_started && !failing_receiver_joined) {
            lm_message_thread_pool_request_stop(pool);
            (void)lm_message_thread_join(failing_receiver, &actor_status);
        }
        if (lm_message_thread_pool_delete(pool) != 0 && status == 0) {
            status = 9;
        }
    }
    if (runtime != 0 && root != 0 &&
        lm_message_thread_runtime_detach_root(runtime, root) != 0 &&
        status == 0) {
        status = 10;
    }
    lm_message_thread_delete(root);
    if (runtime != 0 && lm_message_thread_runtime_delete(runtime) != 0 &&
        status == 0) {
        status = 11;
    }
    lm_condition_delete(state.gate_condition);
    lm_mutex_delete(state.gate_mutex);
    return status;
}

int main(void) {
    int status;

    status = lm_root_address_status_test(
        0,
        "/missing",
        LM_MESSAGE_STATUS_ROUTE_NOT_FOUND
    );
    if (status != 0) {
        fprintf(stderr, "root local status test failed: %d\n", status);
        return 10 + status;
    }
    status = lm_root_address_status_test(
        "https://example.test",
        "/remote",
        LM_MESSAGE_STATUS_TRANSPORT_PROVIDER_NOT_CONFIGURED
    );
    if (status != 0) {
        fprintf(stderr, "root remote status test failed: %d\n", status);
        return 20 + status;
    }
    status = lm_root_invalid_address_test();
    if (status != 0) {
        fprintf(stderr, "root invalid-address test failed: %d\n", status);
        return 30 + status;
    }
    status = lm_root_partial_commit_test();
    if (status != 0) {
        fprintf(stderr, "root partial-commit test failed: %d\n", status);
        return 40 + status;
    }
    status = lm_root_drain_test();
    if (status != 0) {
        fprintf(
            stderr,
            "%s root/drain test failed: %d\n",
            lm_thread_provider_name(),
            status
        );
        return 60 + status;
    }
    return 0;
}
