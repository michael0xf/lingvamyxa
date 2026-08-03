#include <stddef.h>
#include <stdio.h>
#include <string.h>

typedef struct LmMessageThreadRuntime LmMessageThreadRuntime;
typedef struct LmRestLmxHttpServer LmRestLmxHttpServer;

typedef struct LmRestLmxHttpServerOptionsV1 {
    size_t abi_size;
    const char *bind_address;
    unsigned port;
    const char *base_path;
    size_t max_body_bytes;
    unsigned worker_count;
    unsigned request_timeout_ms;
} LmRestLmxHttpServerOptionsV1;

const char *lm_rest_lmx_http_server_provider_name(void);
int lm_rest_lmx_http_server_start(
    LmMessageThreadRuntime *runtime,
    const LmRestLmxHttpServerOptionsV1 *options,
    LmRestLmxHttpServer **out_server
);
int lm_rest_lmx_http_server_start_default(
    LmMessageThreadRuntime *runtime,
    LmRestLmxHttpServer **out_server
);
int lm_rest_lmx_http_server_stop(LmRestLmxHttpServer **server);
int lm_rest_lmx_http_server_port(
    const LmRestLmxHttpServer *server,
    unsigned *out_port
);
LmMessageThreadRuntime *lm_message_thread_runtime_new(void);
int lm_message_thread_runtime_delete(LmMessageThreadRuntime *runtime);

int main(void) {
    LmRestLmxHttpServerOptionsV1 options;
    LmMessageThreadRuntime *runtime = lm_message_thread_runtime_new();
    LmRestLmxHttpServer *server = 0;
    unsigned port = 0U;
    int status = 0;

    options.abi_size = sizeof(options);
    options.bind_address = "127.0.0.1";
    options.port = 0U;
    options.base_path = "/lmx";
    options.max_body_bytes = 1024U;
    options.worker_count = 1U;
    options.request_timeout_ms = 1000U;
    server = (LmRestLmxHttpServer *)(void *)&options;

    if (runtime == 0 ||
        strcmp(lm_rest_lmx_http_server_provider_name(), "none") != 0) {
        status = 1;
        goto cleanup;
    }
    if (lm_rest_lmx_http_server_start(
            runtime,
            &options,
            &server
        ) != 0 || server != 0) {
        status = 2;
        goto cleanup;
    }
    if (lm_rest_lmx_http_server_start_default(runtime, &server) != 0 ||
        server != 0) {
        status = 3;
        goto cleanup;
    }
    if (lm_rest_lmx_http_server_stop(&server) != 0 || server != 0 ||
        lm_rest_lmx_http_server_stop(&server) != 0 || server != 0) {
        status = 4;
        goto cleanup;
    }
    if (lm_rest_lmx_http_server_port(0, &port) == 0 ||
        lm_rest_lmx_http_server_port(0, 0) == 0) {
        status = 5;
    }

cleanup:
    if (server != 0 && lm_rest_lmx_http_server_stop(&server) != 0 &&
        status == 0) {
        status = 6;
    }
    if (runtime != 0 && lm_message_thread_runtime_delete(runtime) != 0 &&
        status == 0) {
        status = 7;
    }
    if (status != 0) {
        fprintf(stderr, "REST/LMX none server test failed: %d\n", status);
    }
    return status;
}
