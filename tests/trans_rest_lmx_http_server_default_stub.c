#include <stddef.h>
#include <stdlib.h>

struct LmMessageThreadRuntime;

typedef struct LmRestLmxHttpServer {
    struct LmMessageThreadRuntime *runtime;
} LmRestLmxHttpServer;

int lm_message_thread_runtime_admit_lmx(
    struct LmMessageThreadRuntime *runtime,
    const char *route,
    const char *lmx,
    size_t length
);
int lm_message_thread_runtime_exit_state(
    struct LmMessageThreadRuntime *runtime,
    int *out_requested,
    int *out_ready,
    int *out_status
);

enum {
    LM_MESSAGE_STATUS_APPLICATION_STOPPING = 70
};

static unsigned lm_test_server_start_calls;
static unsigned lm_test_server_stop_calls;
static int lm_test_server_start_admit_status = -1;
static int lm_test_server_stop_probe_status = -1;
static int lm_test_server_stop_runtime_status = -1;
static int lm_test_server_stop_exit_requested = -1;
static int lm_test_server_stop_exit_ready = -1;
static int lm_test_server_live_handles;
static int lm_test_server_errors;

static void lm_test_rest_lmx_verify_server_lifecycle(void) {
    if (lm_test_server_start_calls != 1U ||
        lm_test_server_stop_calls != 1U ||
        lm_test_server_start_admit_status != 0 ||
        lm_test_server_stop_runtime_status != 0 ||
        (lm_test_server_stop_probe_status != 0 &&
         lm_test_server_stop_probe_status !=
            LM_MESSAGE_STATUS_APPLICATION_STOPPING) ||
        (lm_test_server_stop_exit_requested != 0 &&
         lm_test_server_stop_exit_requested != 1) ||
        (lm_test_server_stop_exit_ready != 0 &&
         lm_test_server_stop_exit_ready != 1) ||
        lm_test_server_live_handles != 0 ||
        lm_test_server_errors != 0) {
        abort();
    }
    if (getenv("LM_TEST_REST_LMX_SERVER_START_FAIL") == NULL) {
        if (lm_test_server_stop_probe_status !=
                LM_MESSAGE_STATUS_APPLICATION_STOPPING ||
            lm_test_server_stop_exit_requested != 1 ||
            lm_test_server_stop_exit_ready != 1) {
            abort();
        }
    } else if (lm_test_server_stop_exit_requested == 0 &&
               lm_test_server_stop_probe_status != 0) {
        abort();
    }
}

int lm_rest_lmx_http_server_start_default(
    struct LmMessageThreadRuntime *runtime,
    LmRestLmxHttpServer **out_server
) {
    static const char body[] = {'{', '\0', '}'};
    LmRestLmxHttpServer *server;

    if (runtime == NULL || out_server == NULL || *out_server != NULL ||
        lm_test_server_start_calls != 0U) {
        return 1;
    }
    lm_test_server_start_calls += 1U;
    if (atexit(lm_test_rest_lmx_verify_server_lifecycle) != 0) {
        return 1;
    }
    server = (LmRestLmxHttpServer *)calloc(1U, sizeof(*server));
    if (server == NULL) {
        return 1;
    }
    server->runtime = runtime;
    *out_server = server;
    lm_test_server_live_handles += 1;
    lm_test_server_start_admit_status =
        lm_message_thread_runtime_admit_lmx(
            runtime,
            "/rest-lmx-http-server",
            body,
            sizeof(body)
        );
    if (lm_test_server_start_admit_status != 0) {
        lm_test_server_errors |= 1;
        return 1;
    }
    return getenv("LM_TEST_REST_LMX_SERVER_START_FAIL") == NULL ? 0 : 23;
}

int lm_rest_lmx_http_server_stop(LmRestLmxHttpServer **server_slot) {
    static const char probe[] = "stop";
    LmRestLmxHttpServer *server;
    int exit_status = -1;

    lm_test_server_stop_calls += 1U;
    if (server_slot == NULL || *server_slot == NULL ||
        lm_test_server_stop_calls != 1U) {
        lm_test_server_errors |= 2;
        return 1;
    }
    server = *server_slot;
    lm_test_server_stop_runtime_status =
        lm_message_thread_runtime_exit_state(
            server->runtime,
            &lm_test_server_stop_exit_requested,
            &lm_test_server_stop_exit_ready,
            &exit_status
        );
    lm_test_server_stop_probe_status =
        lm_message_thread_runtime_admit_lmx(
            server->runtime,
            "/rest-lmx-http-server",
            probe,
            sizeof(probe) - 1U
        );
    free(server);
    *server_slot = NULL;
    lm_test_server_live_handles -= 1;
    return 0;
}
