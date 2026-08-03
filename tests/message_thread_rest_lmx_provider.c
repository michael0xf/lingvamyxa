#include <stddef.h>
#include <stdio.h>
#include <string.h>

typedef struct LmMessageThread LmMessageThread;
typedef struct LmMessageThreadRuntime LmMessageThreadRuntime;
typedef struct LmMessageThreadPool LmMessageThreadPool;
typedef struct LmMutex LmMutex;
typedef struct LmCondition LmCondition;
typedef void (*LmMessageThreadEntry)(LmMessageThread *thread, void *argument);
typedef int (*LmRestLmxPost)(
    void *context,
    const char *normalized_uri,
    const char *body,
    size_t length,
    unsigned *out_http_status
);
typedef void (*LmRestLmxDestroy)(void *context);

typedef struct LmRestLmxProviderOpsV1 {
    size_t abi_size;
    LmRestLmxPost post;
    LmRestLmxDestroy destroy;
} LmRestLmxProviderOpsV1;

enum {
    LM_MESSAGE_STATUS_ROUTE_NOT_FOUND = 64,
    LM_MESSAGE_STATUS_TRANSPORT_PROVIDER_NOT_CONFIGURED = 65,
    LM_MESSAGE_STATUS_INVALID_ADDRESS = 66,
    LM_MESSAGE_STATUS_TRANSPORT_FAILED = 67,
    LM_MESSAGE_STATUS_HTTP_REJECTED = 68,
    LM_MESSAGE_STATUS_TRANSPORT_PROTOCOL_ERROR = 69,
    LM_REST_LMX_MAX_CALLS = 8
};

LmMessageThreadRuntime *lm_message_thread_runtime_new(void);
int lm_message_thread_runtime_delete(LmMessageThreadRuntime *runtime);
int lm_message_thread_runtime_set_rest_lmx_provider(
    LmMessageThreadRuntime *runtime,
    const LmRestLmxProviderOpsV1 *ops,
    void *context
);
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
LmMessageThread *lm_message_thread_new(void);
LmMessageThreadPool *lm_message_thread_pool_new(
    LmMessageThreadRuntime *runtime,
    size_t worker_count
);
void lm_message_thread_pool_request_stop(LmMessageThreadPool *pool);
int lm_message_thread_pool_delete(LmMessageThreadPool *pool);
LmMessageThread *lm_message_thread_new_in(LmMessageThreadPool *pool);
void lm_message_thread_delete(LmMessageThread *thread);
int lm_message_thread_start(
    LmMessageThread *thread,
    LmMessageThreadEntry entry,
    void *argument
);
int lm_message_thread_join(LmMessageThread *thread, int *result);
int lm_message_thread_begin_turn(LmMessageThread *thread);
int lm_message_thread_end_turn(LmMessageThread *thread);
int lm_message_thread_send_lmx(
    LmMessageThread *sender,
    const char *endpoint,
    const char *route,
    const char *lmx,
    size_t length
);
size_t lm_message_thread_outbox_count(const LmMessageThread *thread);
void lm_message_thread_request_failure(LmMessageThread *thread, int status);
int lm_message_thread_request_exit(LmMessageThread *requester, int status);
void lm_message_thread_request_stop(LmMessageThread *thread, int status);
const char *lm_thread_provider_name(void);
LmMutex *lm_mutex_new(void);
void lm_mutex_delete(LmMutex *mutex);
int lm_mutex_lock(LmMutex *mutex);
int lm_mutex_unlock(LmMutex *mutex);
LmCondition *lm_condition_new(void);
void lm_condition_delete(LmCondition *condition);
int lm_condition_wait(LmCondition *condition, LmMutex *mutex);
int lm_condition_broadcast(LmCondition *condition);

typedef struct LmRestLmxResponse {
    int transport_status;
    unsigned http_status;
} LmRestLmxResponse;

typedef struct LmRestLmxFake {
    LmMessageThreadRuntime *runtime;
    const char *expected_uri[LM_REST_LMX_MAX_CALLS];
    const char *expected_body[LM_REST_LMX_MAX_CALLS];
    size_t expected_length[LM_REST_LMX_MAX_CALLS];
    LmRestLmxResponse response[LM_REST_LMX_MAX_CALLS];
    size_t expected_calls;
    size_t calls;
    size_t destroy_calls;
    size_t state_calls;
    int delete_on_destroy;
    int recursive_delete_result;
    int errors;
} LmRestLmxFake;

typedef struct LmRestLmxRootFixture {
    LmMessageThreadRuntime *runtime;
    LmMessageThread *root;
    int attached;
} LmRestLmxRootFixture;

static int lm_rest_lmx_fake_post(
    void *context,
    const char *normalized_uri,
    const char *body,
    size_t length,
    unsigned *out_http_status
) {
    LmRestLmxFake *fake = (LmRestLmxFake *)context;
    size_t index;
    int requested = -1;
    int ready = -1;
    int status = -1;

    if (fake == 0 || out_http_status == 0) {
        return 1;
    }
    index = fake->calls;
    fake->calls += 1U;
    if (fake->runtime == 0 ||
        lm_message_thread_runtime_exit_state(
            fake->runtime,
            &requested,
            &ready,
            &status
        ) != 0) {
        fake->errors |= 1;
    } else {
        fake->state_calls += 1U;
    }
    if (index >= fake->expected_calls ||
        fake->expected_uri[index] == 0 || normalized_uri == 0 ||
        strcmp(normalized_uri, fake->expected_uri[index]) != 0) {
        fake->errors |= 2;
    }
    if (index >= fake->expected_calls ||
        fake->expected_body[index] == 0 || body == 0 ||
        length != fake->expected_length[index] ||
        (length != 0U && memcmp(
            body,
            fake->expected_body[index],
            length
        ) != 0)) {
        fake->errors |= 4;
    }
    if (index >= fake->expected_calls) {
        *out_http_status = 0U;
        return 1;
    }
    *out_http_status = fake->response[index].http_status;
    return fake->response[index].transport_status;
}

static void lm_rest_lmx_fake_destroy(void *context) {
    LmRestLmxFake *fake = (LmRestLmxFake *)context;
    int requested = -1;
    int ready = -1;
    int status = -1;

    if (fake == 0) {
        return;
    }
    fake->destroy_calls += 1U;
    if (fake->delete_on_destroy && fake->runtime != 0) {
        fake->recursive_delete_result =
            lm_message_thread_runtime_delete(fake->runtime);
    }
    if (fake->runtime != 0 &&
        lm_message_thread_runtime_exit_state(
            fake->runtime,
            &requested,
            &ready,
            &status
        ) != 0) {
        fake->errors |= 8;
    }
}

static const LmRestLmxProviderOpsV1 lm_rest_lmx_fake_ops = {
    sizeof(LmRestLmxProviderOpsV1),
    lm_rest_lmx_fake_post,
    lm_rest_lmx_fake_destroy
};

static int lm_rest_lmx_fixture_open(
    LmRestLmxRootFixture *fixture,
    LmRestLmxFake *fake,
    int install_provider
) {
    memset(fixture, 0, sizeof(*fixture));
    fixture->runtime = lm_message_thread_runtime_new();
    fixture->root = lm_message_thread_new();
    if (fixture->runtime == 0 || fixture->root == 0) {
        return 1;
    }
    if (install_provider) {
        fake->runtime = fixture->runtime;
        if (lm_message_thread_runtime_set_rest_lmx_provider(
                fixture->runtime,
                &lm_rest_lmx_fake_ops,
                fake
            ) != 0) {
            return 1;
        }
    }
    if (lm_message_thread_runtime_attach_root(
            fixture->runtime,
            fixture->root
        ) != 0) {
        return 1;
    }
    fixture->attached = 1;
    return 0;
}

static int lm_rest_lmx_fixture_close(LmRestLmxRootFixture *fixture) {
    int status = 0;

    if (fixture->attached &&
        lm_message_thread_runtime_detach_root(
            fixture->runtime,
            fixture->root
        ) != 0) {
        status = 1;
    }
    fixture->attached = 0;
    lm_message_thread_delete(fixture->root);
    fixture->root = 0;
    if (fixture->runtime != 0 &&
        lm_message_thread_runtime_delete(fixture->runtime) != 0) {
        status = 1;
    }
    fixture->runtime = 0;
    return status;
}

static int lm_rest_lmx_provider_lifecycle_test(void) {
    LmMessageThreadRuntime *runtime = lm_message_thread_runtime_new();
    LmRestLmxFake first = {0};
    LmRestLmxFake second = {0};
    LmRestLmxFake third = {0};
    LmRestLmxProviderOpsV1 bad_size = lm_rest_lmx_fake_ops;
    LmRestLmxProviderOpsV1 bad_post = lm_rest_lmx_fake_ops;
    LmRestLmxProviderOpsV1 aliased_ops = lm_rest_lmx_fake_ops;
    int status = 0;

    if (runtime == 0) {
        return 1;
    }
    first.runtime = runtime;
    second.runtime = runtime;
    third.runtime = runtime;
    first.delete_on_destroy = 1;
    first.recursive_delete_result = -1;
    third.delete_on_destroy = 1;
    third.recursive_delete_result = -1;
    bad_size.abi_size -= 1U;
    bad_post.post = 0;
    aliased_ops.destroy = 0;
    if (lm_message_thread_runtime_set_rest_lmx_provider(
            runtime,
            &bad_size,
            &first
        ) == 0 || lm_message_thread_runtime_set_rest_lmx_provider(
            runtime,
            &bad_post,
            &first
        ) == 0 || lm_message_thread_runtime_set_rest_lmx_provider(
            runtime,
            0,
            &first
        ) == 0 || first.destroy_calls != 0U) {
        status = 2;
        goto cleanup;
    }
    if (lm_message_thread_runtime_set_rest_lmx_provider(
            runtime,
            &lm_rest_lmx_fake_ops,
            &first
        ) != 0 || lm_message_thread_runtime_set_rest_lmx_provider(
            runtime,
            &lm_rest_lmx_fake_ops,
            &first
        ) != 0 || first.destroy_calls != 0U ||
        lm_message_thread_runtime_set_rest_lmx_provider(
            runtime,
            &aliased_ops,
            &first
        ) == 0 || first.destroy_calls != 0U ||
        lm_message_thread_runtime_set_rest_lmx_provider(
            runtime,
            &lm_rest_lmx_fake_ops,
            &second
        ) != 0 || first.destroy_calls != 1U ||
        first.recursive_delete_result != 1 || first.errors != 0 ||
        lm_message_thread_runtime_set_rest_lmx_provider(
            runtime,
            0,
            0
        ) != 0 || second.destroy_calls != 1U || second.errors != 0 ||
        lm_message_thread_runtime_set_rest_lmx_provider(
            runtime,
            &lm_rest_lmx_fake_ops,
            &third
        ) != 0) {
        status = 3;
    }

cleanup:
    if (lm_message_thread_runtime_delete(runtime) != 0 && status == 0) {
        status = 4;
    }
    if (status == 0 &&
        (first.destroy_calls != 1U || second.destroy_calls != 1U ||
         third.destroy_calls != 1U || third.recursive_delete_result != 1 ||
         third.errors != 0)) {
        status = 5;
    }
    return status;
}

static int lm_rest_lmx_exact_copy_and_seal_test(void) {
    LmRestLmxRootFixture fixture;
    LmRestLmxFake fake = {0};
    LmRestLmxFake rejected = {0};
    char endpoint[] = "HTTP://Example.com:8443/base/";
    char route[] = "/v1/%41";
    char body[] = {'A', '\0', 'B', 'x'};
    static const char expected_body[] = {'A', '\0', 'B'};
    int status = 0;

    fake.expected_calls = 1U;
    fake.expected_uri[0] = "HTTP://Example.com:8443/base/v1/%41";
    fake.expected_body[0] = expected_body;
    fake.expected_length[0] = sizeof(expected_body);
    fake.response[0].http_status = 204U;
    if (lm_rest_lmx_fixture_open(&fixture, &fake, 1) != 0) {
        return 1;
    }
    rejected.runtime = fixture.runtime;
    if (lm_message_thread_runtime_set_rest_lmx_provider(
            fixture.runtime,
            &lm_rest_lmx_fake_ops,
            &rejected
        ) == 0 || rejected.destroy_calls != 0U ||
        lm_message_thread_begin_turn(fixture.root) != 1 ||
        lm_message_thread_send_lmx(
            fixture.root,
            endpoint,
            route,
            body,
            sizeof(expected_body)
        ) != 0) {
        status = 2;
        goto cleanup;
    }
    endpoint[7] = 'x';
    route[1] = 'X';
    body[0] = 'Z';
    body[2] = 'Y';
    if (lm_message_thread_end_turn(fixture.root) != 0 ||
        fake.calls != 1U || fake.state_calls != 1U || fake.errors != 0 ||
        lm_message_thread_runtime_detach_root(
            fixture.runtime,
            fixture.root
        ) != 0) {
        status = 3;
        goto cleanup;
    }
    fixture.attached = 0;
    if (lm_message_thread_runtime_set_rest_lmx_provider(
            fixture.runtime,
            0,
            0
        ) == 0 || fake.destroy_calls != 0U) {
        status = 4;
    }

cleanup:
    if (lm_rest_lmx_fixture_close(&fixture) != 0 && status == 0) {
        status = 5;
    }
    if (status == 0 &&
        (fake.destroy_calls != 1U || rejected.destroy_calls != 0U)) {
        status = 6;
    }
    return status;
}

static int lm_rest_lmx_one_delivery_test(
    int transport_status,
    unsigned http_status,
    int expected_status
) {
    LmRestLmxRootFixture fixture;
    LmRestLmxFake fake = {0};
    int status = 0;

    fake.expected_calls = 1U;
    fake.expected_uri[0] = "https://[2001:db8::1]:9443/api/message";
    fake.expected_body[0] = "message";
    fake.expected_length[0] = 7U;
    fake.response[0].transport_status = transport_status;
    fake.response[0].http_status = http_status;
    if (lm_rest_lmx_fixture_open(&fixture, &fake, 1) != 0) {
        return 1;
    }
    if (lm_message_thread_begin_turn(fixture.root) != 1 ||
        lm_message_thread_send_lmx(
            fixture.root,
            "https://[2001:db8::1]:9443/api/",
            "/message",
            "message",
            7U
        ) != 0 || lm_message_thread_end_turn(fixture.root) !=
            expected_status || fake.calls != 1U ||
        fake.state_calls != 1U || fake.errors != 0) {
        status = 2;
    }
    if (lm_rest_lmx_fixture_close(&fixture) != 0 && status == 0) {
        status = 3;
    }
    if (status == 0 && fake.destroy_calls != 1U) {
        status = 4;
    }
    return status;
}

static int lm_rest_lmx_status_mapping_test(void) {
    if (lm_rest_lmx_one_delivery_test(9, 204U,
            LM_MESSAGE_STATUS_TRANSPORT_FAILED) != 0) {
        return 1;
    }
    if (lm_rest_lmx_one_delivery_test(0, 201U, 0) != 0) {
        return 2;
    }
    if (lm_rest_lmx_one_delivery_test(0, 404U,
            LM_MESSAGE_STATUS_ROUTE_NOT_FOUND) != 0) {
        return 3;
    }
    if (lm_rest_lmx_one_delivery_test(0, 503U,
            LM_MESSAGE_STATUS_HTTP_REJECTED) != 0) {
        return 4;
    }
    if (lm_rest_lmx_one_delivery_test(0, 302U,
            LM_MESSAGE_STATUS_HTTP_REJECTED) != 0) {
        return 5;
    }
    if (lm_rest_lmx_one_delivery_test(0, 413U,
            LM_MESSAGE_STATUS_HTTP_REJECTED) != 0) {
        return 6;
    }
    if (lm_rest_lmx_one_delivery_test(0, 99U,
            LM_MESSAGE_STATUS_TRANSPORT_PROTOCOL_ERROR) != 0) {
        return 7;
    }
    if (lm_rest_lmx_one_delivery_test(0, 600U,
            LM_MESSAGE_STATUS_TRANSPORT_PROTOCOL_ERROR) != 0) {
        return 8;
    }
    return 0;
}

static int lm_rest_lmx_no_provider_isolation_test(void) {
    LmMessageThreadRuntime *configured_runtime =
        lm_message_thread_runtime_new();
    LmRestLmxFake configured = {0};
    LmRestLmxRootFixture fixture;
    int status = 0;

    if (configured_runtime == 0) {
        return 1;
    }
    configured.runtime = configured_runtime;
    if (lm_message_thread_runtime_set_rest_lmx_provider(
            configured_runtime,
            &lm_rest_lmx_fake_ops,
            &configured
        ) != 0 || lm_rest_lmx_fixture_open(&fixture, 0, 0) != 0) {
        status = 2;
        goto cleanup;
    }
    if (lm_message_thread_begin_turn(fixture.root) != 1 ||
        lm_message_thread_send_lmx(
            fixture.root,
            "http://example.test",
            "/remote",
            "x",
            1U
        ) != 0 || lm_message_thread_end_turn(fixture.root) !=
            LM_MESSAGE_STATUS_TRANSPORT_PROVIDER_NOT_CONFIGURED ||
        configured.calls != 0U) {
        status = 3;
    }
    if (lm_rest_lmx_fixture_close(&fixture) != 0 && status == 0) {
        status = 4;
    }

cleanup:
    if (lm_message_thread_runtime_delete(configured_runtime) != 0 &&
        status == 0) {
        status = 5;
    }
    if (status == 0 && configured.destroy_calls != 1U) {
        status = 6;
    }
    return status;
}

static int lm_rest_lmx_attempt_all_and_rollback_test(void) {
    LmRestLmxRootFixture fixture;
    LmRestLmxFake fake = {0};
    size_t index;
    int status = 0;

    fake.expected_calls = 3U;
    for (index = 0U; index < fake.expected_calls; index += 1U) {
        fake.expected_uri[index] = "http://example.test/base/queue";
    }
    fake.expected_body[0] = "0";
    fake.expected_body[1] = "1";
    fake.expected_body[2] = "2";
    fake.expected_length[0] = 1U;
    fake.expected_length[1] = 1U;
    fake.expected_length[2] = 1U;
    fake.response[0].transport_status = 1;
    fake.response[0].http_status = 204U;
    fake.response[1].http_status = 201U;
    fake.response[2].http_status = 404U;
    if (lm_rest_lmx_fixture_open(&fixture, &fake, 1) != 0) {
        return 1;
    }
    if (lm_message_thread_begin_turn(fixture.root) != 1) {
        status = 2;
        goto cleanup;
    }
    for (index = 0U; index < 3U; index += 1U) {
        const char bodies[] = {'0', '1', '2'};

        if (lm_message_thread_send_lmx(
                fixture.root,
                "http://example.test/base",
                "/queue",
                &bodies[index],
                1U
            ) != 0) {
            status = 3;
            goto cleanup;
        }
    }
    if (lm_message_thread_end_turn(fixture.root) !=
            LM_MESSAGE_STATUS_TRANSPORT_FAILED ||
        fake.calls != 3U || fake.state_calls != 3U || fake.errors != 0) {
        status = 4;
    }

cleanup:
    if (lm_rest_lmx_fixture_close(&fixture) != 0 && status == 0) {
        status = 5;
    }
    if (status != 0) {
        return status;
    }

    memset(&fake, 0, sizeof(fake));
    fake.expected_calls = 0U;
    if (lm_rest_lmx_fixture_open(&fixture, &fake, 1) != 0) {
        return 6;
    }
    if (lm_message_thread_begin_turn(fixture.root) != 1 ||
        lm_message_thread_send_lmx(
            fixture.root,
            "http://example.test",
            "/rollback",
            "discard",
            7U
        ) != 0) {
        status = 7;
        goto rollback_cleanup;
    }
    lm_message_thread_request_failure(fixture.root, 77);
    if (lm_message_thread_end_turn(fixture.root) != 77 ||
        fake.calls != 0U || lm_message_thread_outbox_count(fixture.root) != 0U) {
        status = 8;
    }

rollback_cleanup:
    if (lm_rest_lmx_fixture_close(&fixture) != 0 && status == 0) {
        status = 9;
    }
    return status;
}

typedef struct LmRestLmxExitCutState {
    LmMessageThreadRuntime *runtime;
    LmMutex *mutex;
    LmCondition *condition;
    int late_staged;
    int release_late;
    size_t post_calls;
    size_t state_calls;
    size_t destroy_calls;
    int errors;
} LmRestLmxExitCutState;

static void lm_rest_lmx_exit_cut_record_error(
    LmRestLmxExitCutState *state,
    int error
) {
    if (state == 0 || state->mutex == 0 ||
        lm_mutex_lock(state->mutex) != 0) {
        return;
    }
    state->errors |= error;
    (void)lm_condition_broadcast(state->condition);
    (void)lm_mutex_unlock(state->mutex);
}

static int lm_rest_lmx_exit_cut_post(
    void *context,
    const char *normalized_uri,
    const char *body,
    size_t length,
    unsigned *out_http_status
) {
    LmRestLmxExitCutState *state =
        (LmRestLmxExitCutState *)context;
    int requested = -1;
    int ready = -1;
    int status = -1;

    if (state == 0 || out_http_status == 0 ||
        normalized_uri == 0 ||
        strcmp(normalized_uri, "http://example.test/admitted") != 0 ||
        body == 0 || length != 8U || memcmp(body, "admitted", 8U) != 0) {
        if (state != 0) {
            lm_rest_lmx_exit_cut_record_error(state, 1);
        }
        return 1;
    }
    if (lm_message_thread_runtime_exit_state(
            state->runtime,
            &requested,
            &ready,
            &status
        ) != 0 || !requested || ready || status != 23) {
        lm_rest_lmx_exit_cut_record_error(state, 2);
    }
    if (lm_mutex_lock(state->mutex) != 0) {
        return 1;
    }
    state->post_calls += 1U;
    state->state_calls += 1U;
    (void)lm_condition_broadcast(state->condition);
    (void)lm_mutex_unlock(state->mutex);
    *out_http_status = 204U;
    return 0;
}

static void lm_rest_lmx_exit_cut_destroy(void *context) {
    LmRestLmxExitCutState *state =
        (LmRestLmxExitCutState *)context;

    if (state != 0) {
        state->destroy_calls += 1U;
    }
}

static const LmRestLmxProviderOpsV1 lm_rest_lmx_exit_cut_ops = {
    sizeof(LmRestLmxProviderOpsV1),
    lm_rest_lmx_exit_cut_post,
    lm_rest_lmx_exit_cut_destroy
};

static void lm_rest_lmx_exit_cut_late_entry(
    LmMessageThread *thread,
    void *argument
) {
    LmRestLmxExitCutState *state =
        (LmRestLmxExitCutState *)argument;

    if (lm_message_thread_send_lmx(
            thread,
            "http://example.test",
            "/after-ready",
            "late",
            4U
        ) != 0) {
        lm_rest_lmx_exit_cut_record_error(state, 4);
    }
    if (lm_mutex_lock(state->mutex) != 0) {
        lm_message_thread_request_stop(thread, 91);
        return;
    }
    state->late_staged = 1;
    (void)lm_condition_broadcast(state->condition);
    while (!state->release_late) {
        if (lm_condition_wait(state->condition, state->mutex) != 0) {
            state->errors |= 8;
            break;
        }
    }
    (void)lm_mutex_unlock(state->mutex);
    lm_message_thread_request_stop(thread, 0);
}

static void lm_rest_lmx_exit_cut_requester_entry(
    LmMessageThread *thread,
    void *argument
) {
    LmRestLmxExitCutState *state =
        (LmRestLmxExitCutState *)argument;

    if (lm_message_thread_send_lmx(
            thread,
            "http://example.test",
            "/admitted",
            "admitted",
            8U
        ) != 0) {
        lm_rest_lmx_exit_cut_record_error(state, 16);
    }
    if (lm_message_thread_request_exit(thread, 23) != 0) {
        lm_rest_lmx_exit_cut_record_error(state, 32);
        lm_message_thread_request_stop(thread, 92);
    }
}

static int lm_rest_lmx_exit_cut_test(void) {
    LmRestLmxExitCutState state = {0};
    LmMessageThreadRuntime *runtime = 0;
    LmMessageThreadPool *pool = 0;
    LmMessageThread *late = 0;
    LmMessageThread *requester = 0;
    int late_started = 0;
    int requester_started = 0;
    int late_joined = 0;
    int requester_joined = 0;
    int actor_status = -1;
    int requested = -1;
    int ready = -1;
    int exit_status = -1;
    int status = 0;

    if (strcmp(lm_thread_provider_name(), "pthread") != 0 &&
        strcmp(lm_thread_provider_name(), "win32") != 0) {
        return 0;
    }
    state.mutex = lm_mutex_new();
    state.condition = lm_condition_new();
    runtime = lm_message_thread_runtime_new();
    state.runtime = runtime;
    if (state.mutex == 0 || state.condition == 0 || runtime == 0 ||
        lm_message_thread_runtime_set_rest_lmx_provider(
            runtime,
            &lm_rest_lmx_exit_cut_ops,
            &state
        ) != 0) {
        status = 1;
        goto cleanup;
    }
    pool = lm_message_thread_pool_new(runtime, 2U);
    late = lm_message_thread_new_in(pool);
    requester = lm_message_thread_new_in(pool);
    if (pool == 0 || late == 0 || requester == 0 ||
        lm_message_thread_start(
            late,
            lm_rest_lmx_exit_cut_late_entry,
            &state
        ) != 0) {
        status = 2;
        goto cleanup;
    }
    late_started = 1;
    if (lm_mutex_lock(state.mutex) != 0) {
        status = 3;
        goto cleanup;
    }
    while (!state.late_staged && state.errors == 0) {
        if (lm_condition_wait(state.condition, state.mutex) != 0) {
            state.errors |= 64;
        }
    }
    (void)lm_mutex_unlock(state.mutex);
    if (state.errors != 0 || lm_message_thread_start(
            requester,
            lm_rest_lmx_exit_cut_requester_entry,
            &state
        ) != 0) {
        status = 4;
        goto cleanup;
    }
    requester_started = 1;
    if (lm_message_thread_join(requester, &actor_status) != 0 ||
        actor_status != 23) {
        status = 5;
        goto cleanup;
    }
    requester_joined = 1;
    if (lm_message_thread_runtime_exit_state(
            runtime,
            &requested,
            &ready,
            &exit_status
        ) != 0 || !requested || !ready || exit_status != 23 ||
        state.post_calls != 1U || state.state_calls != 1U ||
        state.errors != 0) {
        status = 6;
        goto cleanup;
    }
    if (lm_mutex_lock(state.mutex) != 0) {
        status = 7;
        goto cleanup;
    }
    state.release_late = 1;
    (void)lm_condition_broadcast(state.condition);
    (void)lm_mutex_unlock(state.mutex);
    if (lm_message_thread_join(late, &actor_status) != 0 ||
        actor_status != 0 || state.post_calls != 1U || state.errors != 0) {
        status = 8;
        goto cleanup;
    }
    late_joined = 1;

cleanup:
    if (state.mutex != 0 && lm_mutex_lock(state.mutex) == 0) {
        state.release_late = 1;
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
        if (late_started && !late_joined) {
            (void)lm_message_thread_join(late, &actor_status);
        }
        if (lm_message_thread_pool_delete(pool) != 0 && status == 0) {
            status = 9;
        }
    }
    if (runtime != 0 && lm_message_thread_runtime_delete(runtime) != 0 &&
        status == 0) {
        status = 10;
    }
    if (status == 0 && state.destroy_calls != 1U) {
        status = 11;
    }
    lm_condition_delete(state.condition);
    lm_mutex_delete(state.mutex);
    return status;
}

static int lm_rest_lmx_address_validation_test(void) {
    static const char *invalid_routes[] = {
        "relative", "//double", "/a//b", "/./a", "/a/../b",
        "/%", "/%0G", "/space here", "/back\\slash", "/query?x",
        "/fragment#x", "/non-ascii-\xc3\xa9"
    };
    static const char *invalid_endpoints[] = {
        "ftp://example.test", "http://", "http:///path",
        "http://user@example.test", "http://example.test:0",
        "http://example.test:65536", "http://example.test:abc",
        "http://[]", "http://[2001:::1]", "http://[2001:db8::1",
        "http://example.test/base//x", "http://example.test/./x",
        "http://example.test/path?x", "http://example.test/path#x",
        "http://example.test/back\\slash"
    };
    LmRestLmxRootFixture fixture;
    size_t index;
    int status = 0;

    if (lm_rest_lmx_fixture_open(&fixture, 0, 0) != 0 ||
        lm_message_thread_begin_turn(fixture.root) != 1) {
        return 1;
    }
    for (index = 0U;
         index < sizeof(invalid_routes) / sizeof(invalid_routes[0]);
         index += 1U) {
        if (lm_message_thread_send_lmx(
                fixture.root,
                "http://example.test",
                invalid_routes[index],
                "x",
                1U
            ) != LM_MESSAGE_STATUS_INVALID_ADDRESS) {
            status = 2;
            goto cleanup;
        }
    }
    for (index = 0U;
         index < sizeof(invalid_endpoints) / sizeof(invalid_endpoints[0]);
         index += 1U) {
        if (lm_message_thread_send_lmx(
                fixture.root,
                invalid_endpoints[index],
                "/route",
                "x",
                1U
            ) != LM_MESSAGE_STATUS_INVALID_ADDRESS) {
            status = 3;
            goto cleanup;
        }
    }
    if (lm_message_thread_outbox_count(fixture.root) != 0U ||
        lm_message_thread_end_turn(fixture.root) != 0) {
        status = 4;
    }

cleanup:
    if (status != 0 && lm_message_thread_outbox_count(fixture.root) == 0U) {
        (void)lm_message_thread_end_turn(fixture.root);
    }
    if (lm_rest_lmx_fixture_close(&fixture) != 0 && status == 0) {
        status = 5;
    }
    return status;
}

int main(void) {
    int status;

    status = lm_rest_lmx_provider_lifecycle_test();
    if (status != 0) {
        fprintf(stderr, "provider lifecycle failed: %d\n", status);
        return 10 + status;
    }
    status = lm_rest_lmx_exact_copy_and_seal_test();
    if (status != 0) {
        fprintf(stderr, "exact copy/seal failed: %d\n", status);
        return 20 + status;
    }
    status = lm_rest_lmx_status_mapping_test();
    if (status != 0) {
        fprintf(stderr, "status mapping failed: %d\n", status);
        return 30 + status;
    }
    status = lm_rest_lmx_no_provider_isolation_test();
    if (status != 0) {
        fprintf(stderr, "runtime isolation failed: %d\n", status);
        return 40 + status;
    }
    status = lm_rest_lmx_attempt_all_and_rollback_test();
    if (status != 0) {
        fprintf(stderr, "attempt-all/rollback failed: %d\n", status);
        return 50 + status;
    }
    status = lm_rest_lmx_exit_cut_test();
    if (status != 0) {
        fprintf(stderr, "exit cut failed: %d\n", status);
        return 60 + status;
    }
    status = lm_rest_lmx_address_validation_test();
    if (status != 0) {
        fprintf(stderr, "address validation failed: %d\n", status);
        return 70 + status;
    }
    return 0;
}
