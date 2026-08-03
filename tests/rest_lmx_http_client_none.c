#include <stddef.h>
#include <stdio.h>
#include <string.h>

typedef struct LmMessageThread LmMessageThread;
typedef struct LmMessageThreadRuntime LmMessageThreadRuntime;

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
    LM_MESSAGE_STATUS_TRANSPORT_PROVIDER_NOT_CONFIGURED = 65
};

const char *lm_rest_lmx_http_client_provider_name(void);
int lm_rest_lmx_http_client_install_default(LmMessageThreadRuntime *runtime);
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
LmMessageThread *lm_message_thread_new(void);
void lm_message_thread_delete(LmMessageThread *thread);
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

static unsigned lm_rest_lmx_none_destroy_count;

static int lm_rest_lmx_none_unexpected_post(
    void *context,
    const char *normalized_uri,
    const char *body,
    size_t length,
    unsigned *out_http_status
) {
    (void)context;
    (void)normalized_uri;
    (void)body;
    (void)length;
    if (out_http_status != 0) {
        *out_http_status = 204U;
    }
    return 0;
}

static void lm_rest_lmx_none_destroy(void *context) {
    unsigned *marker = (unsigned *)context;

    if (marker != 0) {
        *marker += 1U;
    }
    lm_rest_lmx_none_destroy_count += 1U;
}

int main(void) {
    LmMessageThreadRuntime *runtime = lm_message_thread_runtime_new();
    LmMessageThread *root = lm_message_thread_new();
    LmRestLmxProviderOpsV1 configured_ops;
    unsigned destroyed_marker = 0U;
    int attached = 0;
    int status = 0;

    configured_ops.abi_size = sizeof(configured_ops);
    configured_ops.post = lm_rest_lmx_none_unexpected_post;
    configured_ops.destroy = lm_rest_lmx_none_destroy;

    if (runtime == 0 || root == 0 ||
        strcmp(lm_rest_lmx_http_client_provider_name(), "none") != 0 ||
        lm_rest_lmx_http_client_install_default(0) == 0 ||
        lm_message_thread_runtime_set_rest_lmx_provider(
            runtime,
            &configured_ops,
            &destroyed_marker
        ) != 0 ||
        lm_rest_lmx_http_client_install_default(runtime) != 0 ||
        destroyed_marker != 1U || lm_rest_lmx_none_destroy_count != 1U ||
        lm_message_thread_runtime_attach_root(runtime, root) != 0) {
        status = 1;
        goto cleanup;
    }
    attached = 1;
    if (lm_message_thread_begin_turn(root) != 1 ||
        lm_message_thread_send_lmx(
            root,
            "https://example.test/api",
            "/message",
            "A\0B",
            3U
        ) != 0 || lm_message_thread_outbox_count(root) != 1U ||
        lm_message_thread_end_turn(root) !=
            LM_MESSAGE_STATUS_TRANSPORT_PROVIDER_NOT_CONFIGURED ||
        lm_message_thread_outbox_count(root) != 0U) {
        status = 2;
    }

cleanup:
    if (attached && lm_message_thread_runtime_detach_root(runtime, root) != 0 &&
        status == 0) {
        status = 3;
    }
    lm_message_thread_delete(root);
    if (runtime != 0 && lm_message_thread_runtime_delete(runtime) != 0 &&
        status == 0) {
        status = 4;
    }
    if (status != 0) {
        fprintf(stderr, "REST/LMX none provider test failed: %d\n", status);
    }
    return status;
}
