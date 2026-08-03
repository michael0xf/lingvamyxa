#include <stddef.h>
#include <string.h>

typedef struct LmHostThread LmHostThread;
typedef struct LmMessageThread LmMessageThread;
typedef struct LmMessageThreadRuntime LmMessageThreadRuntime;
typedef struct LmMessageThreadPool LmMessageThreadPool;
typedef void *(*LmHostThreadEntry)(void *argument);
typedef void (*LmMessageThreadEntry)(
    LmMessageThread *thread,
    void *argument
);

enum {
    LM_MESSAGE_STATUS_ROUTE_NOT_FOUND = 64,
    LM_MESSAGE_STATUS_INVALID_ADDRESS = 66,
    LM_MESSAGE_STATUS_APPLICATION_STOPPING = 70,
    LM_INGRESS_FIFO_MESSAGES = 3,
    LM_INGRESS_PRODUCERS = 6,
    LM_INGRESS_MESSAGES_PER_PRODUCER = 48,
    LM_INGRESS_FIFO_STATUS = 711,
    LM_INGRESS_CONCURRENT_STATUS = 712
};

const char *lm_thread_provider_name(void);
LmHostThread *lm_host_thread_new(void);
void lm_host_thread_delete(LmHostThread *thread);
int lm_host_thread_start(
    LmHostThread *thread,
    LmHostThreadEntry entry,
    void *argument
);
int lm_host_thread_join(LmHostThread *thread, void **result);
LmMessageThreadRuntime *lm_message_thread_runtime_new(void);
int lm_message_thread_runtime_delete(LmMessageThreadRuntime *runtime);
int lm_message_thread_runtime_admit_lmx(
    LmMessageThreadRuntime *runtime,
    const char *route,
    const char *lmx,
    size_t length
);
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
size_t lm_message_thread_turn_count(const LmMessageThread *thread);
int lm_message_thread_begin_turn(LmMessageThread *thread);
int lm_message_thread_end_turn(LmMessageThread *thread);
int lm_message_thread_request_exit(
    LmMessageThread *requester,
    int status
);
void lm_message_thread_request_stop(LmMessageThread *thread, int status);

typedef struct LmIngressFifoState {
    const unsigned char *expected[LM_INGRESS_FIFO_MESSAGES];
    size_t expected_length[LM_INGRESS_FIFO_MESSAGES];
    size_t received;
    int errors;
} LmIngressFifoState;

static void lm_ingress_fifo_receive(
    LmMessageThread *thread,
    void *argument
) {
    LmIngressFifoState *state = (LmIngressFifoState *)argument;
    const char *lmx = 0;
    size_t length = 0U;
    size_t index;

    if (state == 0 || lm_message_thread_current_lmx(
            thread,
            &lmx,
            &length
        ) != 0 || lmx == 0) {
        if (state != 0) {
            state->errors |= 1;
        }
        lm_message_thread_request_stop(thread, 901);
        return;
    }
    index = state->received;
    if (index >= LM_INGRESS_FIFO_MESSAGES ||
        length != state->expected_length[index] ||
        (length != 0U && memcmp(
            lmx,
            state->expected[index],
            length
        ) != 0)) {
        state->errors |= 2;
    }
    state->received += 1U;
    if (state->received == LM_INGRESS_FIFO_MESSAGES) {
        lm_message_thread_request_stop(thread, LM_INGRESS_FIFO_STATUS);
    }
}

static int lm_ingress_fifo_test(void) {
    static const unsigned char expected_zero[] = {'a', 0U, 'b', 'c'};
    static const unsigned char expected_empty[] = {0U};
    static const unsigned char expected_two[] = {0U, 'x', 0U, 'y', 0xffU};
    unsigned char body_zero[] = {'a', 0U, 'b', 'c'};
    unsigned char body_empty[] = {0U};
    unsigned char body_two[] = {0U, 'x', 0U, 'y', 0xffU};
    LmIngressFifoState state = {{0}, {0U}, 0U, 0};
    LmMessageThreadRuntime *runtime = lm_message_thread_runtime_new();
    LmMessageThreadPool *pool = 0;
    LmMessageThread *receiver = 0;
    int receiver_started = 0;
    int receiver_joined = 0;
    int actor_status = 0;
    int status = 0;

    state.expected[0] = expected_zero;
    state.expected[1] = expected_empty;
    state.expected[2] = expected_two;
    state.expected_length[0] = sizeof(expected_zero);
    state.expected_length[1] = 0U;
    state.expected_length[2] = sizeof(expected_two);
    if (runtime == 0) {
        return 1;
    }
    pool = lm_message_thread_pool_new(runtime, 3U);
    receiver = lm_message_thread_new_in(pool);
    if (pool == 0 || receiver == 0 ||
        lm_message_thread_start_mailbox(
            receiver,
            lm_ingress_fifo_receive,
            &state
        ) != 0 || lm_message_thread_bind_route(
            receiver,
            "/rest/fifo"
        ) != 0) {
        status = 2;
        goto cleanup;
    }
    receiver_started = 1;

    if (lm_message_thread_runtime_admit_lmx(
            0,
            "/rest/fifo",
            "x",
            1U
        ) != 1 || lm_message_thread_runtime_admit_lmx(
            runtime,
            "rest/fifo",
            "x",
            1U
        ) != LM_MESSAGE_STATUS_INVALID_ADDRESS ||
        lm_message_thread_runtime_admit_lmx(
            runtime,
            "/missing",
            "x",
            1U
        ) != LM_MESSAGE_STATUS_ROUTE_NOT_FOUND ||
        lm_message_thread_runtime_admit_lmx(
            runtime,
            "/rest/fifo",
            0,
            0U
        ) != 1 || lm_message_thread_runtime_admit_lmx(
            runtime,
            "/rest/fifo",
            "x",
            (size_t)-1
        ) != 1) {
        status = 3;
        goto cleanup;
    }
    if (lm_message_thread_runtime_admit_lmx(
            runtime,
            "/rest/fifo",
            (const char *)body_zero,
            sizeof(body_zero)
        ) != 0) {
        status = 4;
        goto cleanup;
    }
    memset(body_zero, 0xee, sizeof(body_zero));
    if (lm_message_thread_runtime_admit_lmx(
            runtime,
            "/rest/fifo",
            (const char *)body_empty,
            0U
        ) != 0) {
        status = 5;
        goto cleanup;
    }
    body_empty[0] = 0xeeU;
    if (lm_message_thread_runtime_admit_lmx(
            runtime,
            "/rest/fifo",
            (const char *)body_two,
            sizeof(body_two)
        ) != 0) {
        status = 6;
        goto cleanup;
    }
    memset(body_two, 0xee, sizeof(body_two));

    if (lm_message_thread_join(receiver, &actor_status) != 0 ||
        actor_status != LM_INGRESS_FIFO_STATUS) {
        status = 7;
        goto cleanup;
    }
    receiver_joined = 1;
    if (lm_message_thread_runtime_admit_lmx(
            runtime,
            "/rest/fifo",
            "late",
            4U
        ) != LM_MESSAGE_STATUS_APPLICATION_STOPPING ||
        state.errors != 0 ||
        state.received != LM_INGRESS_FIFO_MESSAGES ||
        lm_message_thread_inbox_count(receiver) != 0U ||
        lm_message_thread_turn_count(receiver) != LM_INGRESS_FIFO_MESSAGES) {
        status = 8;
    }

cleanup:
    if (pool != 0) {
        lm_message_thread_pool_request_stop(pool);
        if (receiver_started && !receiver_joined) {
            (void)lm_message_thread_join(receiver, &actor_status);
        }
        if (lm_message_thread_pool_delete(pool) != 0 && status == 0) {
            status = 9;
        }
    }
    if (runtime != 0 && lm_message_thread_runtime_delete(runtime) != 0 &&
        status == 0) {
        status = 10;
    }
    return status;
}

typedef struct LmIngressConcurrentState {
    size_t next_sequence[LM_INGRESS_PRODUCERS];
    size_t received;
    int errors;
} LmIngressConcurrentState;

typedef struct LmIngressProducer {
    LmMessageThreadRuntime *runtime;
    size_t identifier;
    int status;
} LmIngressProducer;

static void lm_ingress_concurrent_receive(
    LmMessageThread *thread,
    void *argument
) {
    LmIngressConcurrentState *state =
        (LmIngressConcurrentState *)argument;
    const char *lmx = 0;
    const unsigned char *body;
    size_t length = 0U;
    size_t producer;
    size_t sequence;

    if (state == 0 || lm_message_thread_current_lmx(
            thread,
            &lmx,
            &length
        ) != 0 || lmx == 0 || length != 6U) {
        if (state != 0) {
            state->errors |= 1;
        }
        lm_message_thread_request_stop(thread, 902);
        return;
    }
    body = (const unsigned char *)lmx;
    producer = (size_t)(body[0] & 0x0fU);
    sequence = (size_t)body[2] | ((size_t)body[3] << 8U);
    if ((body[0] & 0xf0U) != 0xa0U || body[1] != 0U ||
        body[4] != 0x55U || body[5] != 0U ||
        producer >= LM_INGRESS_PRODUCERS ||
        sequence >= LM_INGRESS_MESSAGES_PER_PRODUCER) {
        state->errors |= 2;
    } else {
        if (sequence != state->next_sequence[producer]) {
            state->errors |= 4;
        }
        state->next_sequence[producer] += 1U;
    }
    state->received += 1U;
    if (state->received ==
        (size_t)LM_INGRESS_PRODUCERS *
        LM_INGRESS_MESSAGES_PER_PRODUCER) {
        lm_message_thread_request_stop(
            thread,
            LM_INGRESS_CONCURRENT_STATUS
        );
    }
}

static void *lm_ingress_produce(void *argument) {
    LmIngressProducer *producer = (LmIngressProducer *)argument;
    size_t sequence;

    if (producer == 0 || producer->runtime == 0 ||
        producer->identifier >= LM_INGRESS_PRODUCERS) {
        return 0;
    }
    for (sequence = 0U;
         sequence < LM_INGRESS_MESSAGES_PER_PRODUCER;
         sequence += 1U) {
        unsigned char body[6];

        body[0] = (unsigned char)(0xa0U | producer->identifier);
        body[1] = 0U;
        body[2] = (unsigned char)(sequence & 0xffU);
        body[3] = (unsigned char)((sequence >> 8U) & 0xffU);
        body[4] = 0x55U;
        body[5] = 0U;
        if (lm_message_thread_runtime_admit_lmx(
                producer->runtime,
                "/rest/concurrent",
                (const char *)body,
                sizeof(body)
            ) != 0) {
            producer->status = 1;
            return 0;
        }
        memset(body, 0xee, sizeof(body));
    }
    return producer;
}

static int lm_ingress_concurrent_test(void) {
    LmIngressConcurrentState state = {{0U}, 0U, 0};
    LmIngressProducer producers[LM_INGRESS_PRODUCERS] = {{0}};
    LmHostThread *hosts[LM_INGRESS_PRODUCERS] = {0};
    LmMessageThreadRuntime *runtime = lm_message_thread_runtime_new();
    LmMessageThreadPool *pool = 0;
    LmMessageThread *receiver = 0;
    size_t started = 0U;
    int receiver_started = 0;
    int receiver_joined = 0;
    int actor_status = 0;
    int status = 0;
    size_t index;

    if (runtime == 0) {
        return 1;
    }
    pool = lm_message_thread_pool_new(runtime, 4U);
    receiver = lm_message_thread_new_in(pool);
    if (pool == 0 || receiver == 0 ||
        lm_message_thread_start_mailbox(
            receiver,
            lm_ingress_concurrent_receive,
            &state
        ) != 0 || lm_message_thread_bind_route(
            receiver,
            "/rest/concurrent"
        ) != 0) {
        status = 2;
        goto cleanup;
    }
    receiver_started = 1;

    for (index = 0U; index < LM_INGRESS_PRODUCERS; index += 1U) {
        producers[index].runtime = runtime;
        producers[index].identifier = index;
        hosts[index] = lm_host_thread_new();
        if (hosts[index] == 0 || lm_host_thread_start(
                hosts[index],
                lm_ingress_produce,
                &producers[index]
            ) != 0) {
            status = 3;
            break;
        }
        started += 1U;
    }
    for (index = 0U; index < started; index += 1U) {
        void *result = 0;

        if (lm_host_thread_join(hosts[index], &result) != 0 ||
            result != &producers[index] || producers[index].status != 0) {
            status = 4;
        }
    }
    if (status != 0) {
        lm_message_thread_request_stop(receiver, 903);
    }
    if (lm_message_thread_join(receiver, &actor_status) != 0 ||
        (status == 0 && actor_status != LM_INGRESS_CONCURRENT_STATUS)) {
        if (status == 0) {
            status = 5;
        }
    } else {
        receiver_joined = 1;
    }
    if (status == 0 &&
        (state.errors != 0 || state.received !=
            (size_t)LM_INGRESS_PRODUCERS *
            LM_INGRESS_MESSAGES_PER_PRODUCER ||
         lm_message_thread_turn_count(receiver) != state.received)) {
        status = 6;
    }
    for (index = 0U; status == 0 && index < LM_INGRESS_PRODUCERS;
         index += 1U) {
        if (state.next_sequence[index] !=
            LM_INGRESS_MESSAGES_PER_PRODUCER) {
            status = 7;
        }
    }

cleanup:
    for (index = 0U; index < LM_INGRESS_PRODUCERS; index += 1U) {
        lm_host_thread_delete(hosts[index]);
    }
    if (pool != 0) {
        lm_message_thread_pool_request_stop(pool);
        if (receiver_started && !receiver_joined) {
            (void)lm_message_thread_join(receiver, &actor_status);
        }
        if (lm_message_thread_pool_delete(pool) != 0 && status == 0) {
            status = 8;
        }
    }
    if (runtime != 0 && lm_message_thread_runtime_delete(runtime) != 0 &&
        status == 0) {
        status = 9;
    }
    return status;
}

typedef struct LmIngressExitState {
    size_t calls;
} LmIngressExitState;

static void lm_ingress_exit_receive(
    LmMessageThread *thread,
    void *argument
) {
    LmIngressExitState *state = (LmIngressExitState *)argument;

    if (state != 0) {
        state->calls += 1U;
    }
    (void)thread;
}

static int lm_ingress_exit_cut_test(void) {
    LmIngressExitState state = {0U};
    LmMessageThreadRuntime *runtime = lm_message_thread_runtime_new();
    LmMessageThreadPool *pool = 0;
    LmMessageThread *root = lm_message_thread_new();
    LmMessageThread *receiver = 0;
    int attached = 0;
    int receiver_started = 0;
    int receiver_joined = 0;
    int actor_status = 0;
    int status = 0;

    if (runtime == 0 || root == 0) {
        status = 1;
        goto cleanup;
    }
    pool = lm_message_thread_pool_new(runtime, 2U);
    receiver = lm_message_thread_new_in(pool);
    if (pool == 0 || receiver == 0 ||
        lm_message_thread_start_mailbox(
            receiver,
            lm_ingress_exit_receive,
            &state
        ) != 0 || lm_message_thread_bind_route(
            receiver,
            "/rest/exit"
        ) != 0 || lm_message_thread_runtime_attach_root(
            runtime,
            root
        ) != 0) {
        status = 2;
        goto cleanup;
    }
    receiver_started = 1;
    attached = 1;
    if (lm_message_thread_begin_turn(root) != 1 ||
        lm_message_thread_request_exit(root, 19) != 0 ||
        lm_message_thread_runtime_admit_lmx(
            runtime,
            "/rest/exit",
            "remote",
            6U
        ) != LM_MESSAGE_STATUS_APPLICATION_STOPPING ||
        lm_message_thread_runtime_admit_lmx(
            runtime,
            "/missing",
            "remote",
            6U
        ) != LM_MESSAGE_STATUS_APPLICATION_STOPPING ||
        lm_message_thread_send_lmx(
            root,
            0,
            "/rest/exit",
            "local",
            5U
        ) != 0 || lm_message_thread_end_turn(root) != 0) {
        status = 3;
        goto cleanup;
    }
    if (lm_message_thread_runtime_detach_root(runtime, root) != 0) {
        status = 4;
        goto cleanup;
    }
    attached = 0;
    lm_message_thread_request_stop(receiver, 0);
    if (lm_message_thread_join(receiver, &actor_status) != 0 ||
        actor_status != 0) {
        status = 5;
        goto cleanup;
    }
    receiver_joined = 1;
    if (state.calls + lm_message_thread_inbox_count(receiver) != 1U ||
        state.calls > 1U) {
        status = 6;
    }

cleanup:
    if (attached) {
        (void)lm_message_thread_runtime_detach_root(runtime, root);
    }
    if (pool != 0) {
        lm_message_thread_pool_request_stop(pool);
        if (receiver_started && !receiver_joined) {
            (void)lm_message_thread_join(receiver, &actor_status);
        }
        if (lm_message_thread_pool_delete(pool) != 0 && status == 0) {
            status = 7;
        }
    }
    lm_message_thread_delete(root);
    if (runtime != 0 && lm_message_thread_runtime_delete(runtime) != 0 &&
        status == 0) {
        status = 8;
    }
    return status;
}

int main(void) {
    int status = lm_ingress_fifo_test();

    if (status != 0) {
        return 10 + status;
    }
    status = lm_ingress_exit_cut_test();
    if (status != 0) {
        return 30 + status;
    }
    if (strcmp(lm_thread_provider_name(), "single") != 0) {
        status = lm_ingress_concurrent_test();
        if (status != 0) {
            return 50 + status;
        }
    }
    return 0;
}
