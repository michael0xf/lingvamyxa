#include <stddef.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>

struct LmOwnArena;
struct LmMessageThread;
struct LmMessageThreadRuntime;
struct LmMessageThreadPool;
#ifndef LM_LMX_LAYOUT_DEFINED_IncomingMessage
#define LM_LMX_LAYOUT_DEFINED_IncomingMessage 1
typedef struct IncomingMessage IncomingMessage;
struct IncomingMessage {
    const char *lmx;
    size_t length;
};
#endif
struct LmMessageThread *lm_message_thread_new(void);
void lm_message_thread_delete(struct LmMessageThread *thread);
struct LmMessageThreadRuntime *lm_message_thread_runtime_new(void);
#ifdef LM_REST_LMX_INSTALL_DEFAULT_CLIENT
int lm_rest_lmx_http_client_install_default(struct LmMessageThreadRuntime *runtime);
#endif
#ifdef LM_REST_LMX_INSTALL_DEFAULT_SERVER
typedef struct LmRestLmxHttpServer LmRestLmxHttpServer;
int lm_rest_lmx_http_server_start_default(struct LmMessageThreadRuntime *runtime, LmRestLmxHttpServer **out_server);
int lm_rest_lmx_http_server_stop(LmRestLmxHttpServer **server);
#endif
int lm_message_thread_runtime_attach_root(struct LmMessageThreadRuntime *runtime, struct LmMessageThread *thread);
int lm_message_thread_runtime_detach_root(struct LmMessageThreadRuntime *runtime, struct LmMessageThread *thread);
int lm_message_thread_runtime_exit_state(struct LmMessageThreadRuntime *runtime, int *requested, int *ready, int *status);
int lm_message_thread_runtime_delete(struct LmMessageThreadRuntime *runtime);
struct LmMessageThreadPool *lm_message_thread_pool_new(struct LmMessageThreadRuntime *runtime, size_t worker_count);
void lm_message_thread_pool_request_stop(struct LmMessageThreadPool *pool);
void lm_message_thread_pool_request_stop_when_idle(struct LmMessageThreadPool *pool);
size_t lm_message_thread_pool_pump(struct LmMessageThreadPool *pool, size_t max_turns);
int lm_message_thread_pool_delete(struct LmMessageThreadPool *pool);
struct LmMessageThread *lm_message_thread_new_in(struct LmMessageThreadPool *pool);
int lm_message_thread_start_mailbox(struct LmMessageThread *thread, void (*entry)(struct LmMessageThread *, void *), void *argument);
int lm_message_thread_join(struct LmMessageThread *thread, int *result);
int lm_message_thread_bind_route(struct LmMessageThread *thread, const char *route);
int lm_message_thread_current_lmx(struct LmMessageThread *thread, const char **out_lmx, size_t *out_length);
int lm_message_thread_send_lmx(struct LmMessageThread *thread, const char *endpoint, const char *route, const char *lmx, size_t length);
struct LmOwnArena *lm_message_thread_owner(struct LmMessageThread *thread);
void *lm_message_thread_execution_context(struct LmMessageThread *thread);
void *lm_message_thread_set_execution_context(struct LmMessageThread *thread, void *context);
int lm_message_thread_begin_turn(struct LmMessageThread *thread);
int lm_message_thread_end_turn(struct LmMessageThread *thread);
void lm_message_thread_request_stop(struct LmMessageThread *thread, int status);
void lm_message_thread_request_failure(struct LmMessageThread *thread, int status);
int lm_message_thread_request_exit(struct LmMessageThread *thread, int status);
int lm_message_thread_status(const struct LmMessageThread *thread);
int lm_message_thread_is_running(const struct LmMessageThread *thread);
size_t lm_message_thread_turn_count(const struct LmMessageThread *thread);
size_t lm_message_thread_collection_count(const struct LmMessageThread *thread);
void lm_own_arena_freeze(struct LmMessageThread *lm_lmx_message_thread, struct LmOwnArena *arena);
void *lm_own_arena_new_zero(struct LmMessageThread *lm_lmx_message_thread, struct LmOwnArena *arena, size_t size);
void *lm_own_arena_array_new_zero(struct LmMessageThread *lm_lmx_message_thread, struct LmOwnArena *arena, size_t element_size, size_t count, size_t rank, size_t level);
typedef struct LmMessageThreadExecutionContext LmMessageThreadExecutionContext;
struct LmMessageThreadExecutionContext {
    jmp_buf diagnostic_root;
    int diagnostic_code;
    const char *diagnostic_label;
    const char *diagnostic_file;
    int diagnostic_line;
    const char *diagnostic_expr;
};
typedef int (*LmLmxMessageThreadEntry)(struct LmMessageThread *thread);
#if defined(__GNUC__) || defined(__clang__)
#define LM_LMX_UNUSED_ENTRY_HELPER __attribute__((unused))
#else
#define LM_LMX_UNUSED_ENTRY_HELPER
#endif
static inline LM_LMX_UNUSED_ENTRY_HELPER int lm_lmx_message_thread_run_entry(LmLmxMessageThreadEntry entry) {
    struct LmMessageThread *thread;
    struct LmMessageThreadRuntime *runtime;
    LmMessageThreadExecutionContext context = {0};
    int status;
    int attached = 0;
    if (entry == 0) {
        return 1;
    }
    thread = lm_message_thread_new();
    if (thread == 0) {
        return 1;
    }
    runtime = lm_message_thread_runtime_new();
    if (runtime == 0) {
        lm_message_thread_delete(thread);
        return 1;
    }
#ifdef LM_REST_LMX_INSTALL_DEFAULT_CLIENT
    if (lm_rest_lmx_http_client_install_default(runtime) != 0) {
        (void)lm_message_thread_runtime_delete(runtime);
        lm_message_thread_delete(thread);
        return 1;
    }
#endif
    if (lm_message_thread_runtime_attach_root(runtime, thread) != 0) {
        (void)lm_message_thread_runtime_delete(runtime);
        lm_message_thread_delete(thread);
        return 1;
    }
    attached = 1;
    (void)lm_message_thread_set_execution_context(thread, &context);
    while (lm_message_thread_begin_turn(thread)) {
        context.diagnostic_code = 0;
        if (setjmp(context.diagnostic_root) == 0) {
            lm_message_thread_request_stop(thread, entry(thread));
        } else {
            lm_message_thread_request_failure(thread, context.diagnostic_code == 0 ? 1 : context.diagnostic_code);
        }
        (void)lm_message_thread_end_turn(thread);
    }
    status = lm_message_thread_status(thread);
    if (lm_message_thread_runtime_detach_root(runtime, thread) != 0) {
        if (status == 0) status = 1;
    } else {
        attached = 0;
    }
    if (!attached && lm_message_thread_runtime_delete(runtime) != 0 && status == 0) status = 1;
    if (!attached) lm_message_thread_delete(thread);
    return status;
}
#undef LM_LMX_UNUSED_ENTRY_HELPER

#ifndef LM_LMX_TYPEDEF_DEFINED_LmOwnDestroyFields
#define LM_LMX_TYPEDEF_DEFINED_LmOwnDestroyFields 1
#define LM_LMX_TYPEDEF_ID_A_LmOwnDestroyFields 0x87dae9040942b0a0ULL
#define LM_LMX_TYPEDEF_ID_B_LmOwnDestroyFields 0x38ad69ed565e4a65ULL
typedef void (*LmOwnDestroyFields)(void *object);
#else
#if !defined(LM_LMX_TYPEDEF_ID_A_LmOwnDestroyFields) || !defined(LM_LMX_TYPEDEF_ID_B_LmOwnDestroyFields) || LM_LMX_TYPEDEF_ID_A_LmOwnDestroyFields != 0x87dae9040942b0a0ULL || LM_LMX_TYPEDEF_ID_B_LmOwnDestroyFields != 0x38ad69ed565e4a65ULL
#error "Lingvamyxa conflicting typedef projection for LmOwnDestroyFields"
#endif
#endif

#ifndef LM_LMX_TYPEDEF_DEFINED_LmOwnDelete
#define LM_LMX_TYPEDEF_DEFINED_LmOwnDelete 1
#define LM_LMX_TYPEDEF_ID_A_LmOwnDelete 0xe0a897d278d341eeULL
#define LM_LMX_TYPEDEF_ID_B_LmOwnDelete 0xca76caf7c88baa9fULL
typedef void (*LmOwnDelete)(void *object);
#else
#if !defined(LM_LMX_TYPEDEF_ID_A_LmOwnDelete) || !defined(LM_LMX_TYPEDEF_ID_B_LmOwnDelete) || LM_LMX_TYPEDEF_ID_A_LmOwnDelete != 0xe0a897d278d341eeULL || LM_LMX_TYPEDEF_ID_B_LmOwnDelete != 0xca76caf7c88baa9fULL
#error "Lingvamyxa conflicting typedef projection for LmOwnDelete"
#endif
#endif

#ifndef LM_LMX_TYPEDEF_DEFINED_LmHostThreadEntry
#define LM_LMX_TYPEDEF_DEFINED_LmHostThreadEntry 1
#define LM_LMX_TYPEDEF_ID_A_LmHostThreadEntry 0x2bcd1683deffe5bbULL
#define LM_LMX_TYPEDEF_ID_B_LmHostThreadEntry 0xbf88e44dfd79f8f2ULL
typedef void * (*LmHostThreadEntry)(void *argument);
#else
#if !defined(LM_LMX_TYPEDEF_ID_A_LmHostThreadEntry) || !defined(LM_LMX_TYPEDEF_ID_B_LmHostThreadEntry) || LM_LMX_TYPEDEF_ID_A_LmHostThreadEntry != 0x2bcd1683deffe5bbULL || LM_LMX_TYPEDEF_ID_B_LmHostThreadEntry != 0xbf88e44dfd79f8f2ULL
#error "Lingvamyxa conflicting typedef projection for LmHostThreadEntry"
#endif
#endif

#ifndef LM_LMX_TYPEDEF_DEFINED_LmMessageThreadEntry
#define LM_LMX_TYPEDEF_DEFINED_LmMessageThreadEntry 1
#define LM_LMX_TYPEDEF_ID_A_LmMessageThreadEntry 0x8154b7f7ecc2154bULL
#define LM_LMX_TYPEDEF_ID_B_LmMessageThreadEntry 0x29354c13a34b4916ULL
typedef void (*LmMessageThreadEntry)(struct LmMessageThread *lm_lmx_message_thread, void *argument);
#else
#if !defined(LM_LMX_TYPEDEF_ID_A_LmMessageThreadEntry) || !defined(LM_LMX_TYPEDEF_ID_B_LmMessageThreadEntry) || LM_LMX_TYPEDEF_ID_A_LmMessageThreadEntry != 0x8154b7f7ecc2154bULL || LM_LMX_TYPEDEF_ID_B_LmMessageThreadEntry != 0x29354c13a34b4916ULL
#error "Lingvamyxa conflicting typedef projection for LmMessageThreadEntry"
#endif
#endif

#ifndef LM_LMX_TYPEDEF_DEFINED_LmMessageThreadComponentDestroy
#define LM_LMX_TYPEDEF_DEFINED_LmMessageThreadComponentDestroy 1
#define LM_LMX_TYPEDEF_ID_A_LmMessageThreadComponentDestroy 0x994bdd720ae8075eULL
#define LM_LMX_TYPEDEF_ID_B_LmMessageThreadComponentDestroy 0xb551f3efe92f53b7ULL
typedef void (*LmMessageThreadComponentDestroy)(struct LmMessageThread *lm_lmx_message_thread, void *component);
#else
#if !defined(LM_LMX_TYPEDEF_ID_A_LmMessageThreadComponentDestroy) || !defined(LM_LMX_TYPEDEF_ID_B_LmMessageThreadComponentDestroy) || LM_LMX_TYPEDEF_ID_A_LmMessageThreadComponentDestroy != 0x994bdd720ae8075eULL || LM_LMX_TYPEDEF_ID_B_LmMessageThreadComponentDestroy != 0xb551f3efe92f53b7ULL
#error "Lingvamyxa conflicting typedef projection for LmMessageThreadComponentDestroy"
#endif
#endif

#ifndef LM_LMX_TYPEDEF_DEFINED_LmRestLmxPost
#define LM_LMX_TYPEDEF_DEFINED_LmRestLmxPost 1
#define LM_LMX_TYPEDEF_ID_A_LmRestLmxPost 0xf02a42b4277dbd4aULL
#define LM_LMX_TYPEDEF_ID_B_LmRestLmxPost 0x950c7db65a0b5dbbULL
typedef int (*LmRestLmxPost)(void *context, const char *normalized_uri, const char *body, size_t length, unsigned *out_http_status);
#else
#if !defined(LM_LMX_TYPEDEF_ID_A_LmRestLmxPost) || !defined(LM_LMX_TYPEDEF_ID_B_LmRestLmxPost) || LM_LMX_TYPEDEF_ID_A_LmRestLmxPost != 0xf02a42b4277dbd4aULL || LM_LMX_TYPEDEF_ID_B_LmRestLmxPost != 0x950c7db65a0b5dbbULL
#error "Lingvamyxa conflicting typedef projection for LmRestLmxPost"
#endif
#endif

#ifndef LM_LMX_TYPEDEF_DEFINED_LmRestLmxDestroy
#define LM_LMX_TYPEDEF_DEFINED_LmRestLmxDestroy 1
#define LM_LMX_TYPEDEF_ID_A_LmRestLmxDestroy 0x4a99563ebb3cfe42ULL
#define LM_LMX_TYPEDEF_ID_B_LmRestLmxDestroy 0xae9da72f8f4d2a73ULL
typedef void (*LmRestLmxDestroy)(void *context);
#else
#if !defined(LM_LMX_TYPEDEF_ID_A_LmRestLmxDestroy) || !defined(LM_LMX_TYPEDEF_ID_B_LmRestLmxDestroy) || LM_LMX_TYPEDEF_ID_A_LmRestLmxDestroy != 0x4a99563ebb3cfe42ULL || LM_LMX_TYPEDEF_ID_B_LmRestLmxDestroy != 0xae9da72f8f4d2a73ULL
#error "Lingvamyxa conflicting typedef projection for LmRestLmxDestroy"
#endif
#endif

































































































































































#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct LmRestLmxProviderOpsV1 {
    size_t abi_size;
    int (*post)(
        void *context,
        const char *normalized_uri,
        const char *body,
        size_t length,
        unsigned *out_http_status
    );
    void (*destroy)(void *context);
};

extern int lm_message_thread_runtime_set_rest_lmx_provider(
    struct LmMessageThreadRuntime *runtime,
    const struct LmRestLmxProviderOpsV1 *ops,
    void *context
);

#ifndef LM_REST_LMX_CLIENT_PROVIDER_NONE
#define LM_REST_LMX_CLIENT_PROVIDER_NONE 0
#endif
#ifndef LM_REST_LMX_CLIENT_PROVIDER_LIBCURL
#define LM_REST_LMX_CLIENT_PROVIDER_LIBCURL 1
#endif
#ifndef LM_REST_LMX_CLIENT_PROVIDER_WINHTTP
#define LM_REST_LMX_CLIENT_PROVIDER_WINHTTP 2
#endif

#ifndef LM_REST_LMX_CLIENT_PROVIDER
#define LM_REST_LMX_CLIENT_PROVIDER LM_REST_LMX_CLIENT_PROVIDER_NONE
#endif

#if LM_REST_LMX_CLIENT_PROVIDER != LM_REST_LMX_CLIENT_PROVIDER_NONE && \
    LM_REST_LMX_CLIENT_PROVIDER != LM_REST_LMX_CLIENT_PROVIDER_LIBCURL && \
    LM_REST_LMX_CLIENT_PROVIDER != LM_REST_LMX_CLIENT_PROVIDER_WINHTTP
#error "Unknown LM_REST_LMX_CLIENT_PROVIDER value"
#endif

#if LM_REST_LMX_CLIENT_PROVIDER == LM_REST_LMX_CLIENT_PROVIDER_LIBCURL

#include <curl/curl.h>

typedef struct LmRestLmxHttpClientContext {
    int curl_initialized;
} LmRestLmxHttpClientContext;

static int lm_rest_lmx_http_ascii_prefix(
    const char *text,
    const char *prefix
) {
    size_t index = 0U;

    if (text == 0 || prefix == 0) {
        return 0;
    }
    while (prefix[index] != '\0') {
        unsigned char left = (unsigned char)text[index];
        unsigned char right = (unsigned char)prefix[index];

        if (left >= (unsigned char)'A' && left <= (unsigned char)'Z') {
            left = (unsigned char)(left + ((unsigned char)'a' -
                (unsigned char)'A'));
        }
        if (right >= (unsigned char)'A' && right <= (unsigned char)'Z') {
            right = (unsigned char)(right + ((unsigned char)'a' -
                (unsigned char)'A'));
        }
        if (left == (unsigned char)'\0' || left != right) {
            return 0;
        }
        index += 1U;
    }
    return 1;
}

static int lm_rest_lmx_http_uri_is_supported(const char *uri) {
    return lm_rest_lmx_http_ascii_prefix(uri, "http://") ||
        lm_rest_lmx_http_ascii_prefix(uri, "https://");
}

static size_t lm_rest_lmx_http_discard_response(
    char *data,
    size_t size,
    size_t count,
    void *context
) {
    (void)data;
    (void)context;
    if (size != 0U && count > SIZE_MAX / size) {
        return 0U;
    }
    return size * count;
}

static void lm_rest_lmx_http_client_destroy(void *opaque_context) {
    LmRestLmxHttpClientContext *context =
        (LmRestLmxHttpClientContext *)opaque_context;

    if (context == 0) {
        return;
    }
    if (context->curl_initialized) {
        curl_global_cleanup();
        context->curl_initialized = 0;
    }
    free(context);
}

static int lm_rest_lmx_http_client_post(
    void *opaque_context,
    const char *normalized_uri,
    const char *body,
    size_t length,
    unsigned *out_http_status
) {
    LmRestLmxHttpClientContext *context =
        (LmRestLmxHttpClientContext *)opaque_context;
    CURL *easy = 0;
    struct curl_slist *headers = 0;
    struct curl_slist *updated_headers;
    curl_off_t post_length;
    CURLcode result;
    long response_code = 0L;
    int status = 1;

    if (out_http_status != 0) {
        *out_http_status = 0U;
    }
    if (context == 0 || !context->curl_initialized ||
        normalized_uri == 0 || out_http_status == 0 ||
        (body == 0 && length != 0U) ||
        !lm_rest_lmx_http_uri_is_supported(normalized_uri)) {
        return 1;
    }

    post_length = (curl_off_t)length;
    if (post_length < (curl_off_t)0 || (size_t)post_length != length) {
        return 1;
    }

    easy = curl_easy_init();
    if (easy == 0) {
        return 1;
    }
    headers = curl_slist_append(0, "Content-Type: application/lmx");
    if (headers == 0) {
        goto cleanup;
    }
    updated_headers = curl_slist_append(headers, "Accept: application/lmx");
    if (updated_headers == 0) {
        goto cleanup;
    }
    headers = updated_headers;

#define LM_REST_LMX_CURL_SETOPT(option, value) \
    do { \
        if (curl_easy_setopt(easy, (option), (value)) != CURLE_OK) { \
            goto cleanup; \
        } \
    } while (0)

    LM_REST_LMX_CURL_SETOPT(CURLOPT_URL, normalized_uri);
    LM_REST_LMX_CURL_SETOPT(CURLOPT_POST, 1L);
    LM_REST_LMX_CURL_SETOPT(CURLOPT_POSTFIELDS, body == 0 ? "" : body);
    LM_REST_LMX_CURL_SETOPT(CURLOPT_POSTFIELDSIZE_LARGE, post_length);
    LM_REST_LMX_CURL_SETOPT(CURLOPT_HTTPHEADER, headers);
    LM_REST_LMX_CURL_SETOPT(CURLOPT_FOLLOWLOCATION, 0L);
    LM_REST_LMX_CURL_SETOPT(CURLOPT_MAXREDIRS, 0L);
    LM_REST_LMX_CURL_SETOPT(CURLOPT_PATH_AS_IS, 1L);
#if LIBCURL_VERSION_NUM >= 0x075500
    LM_REST_LMX_CURL_SETOPT(CURLOPT_PROTOCOLS_STR, "http,https");
#else
    LM_REST_LMX_CURL_SETOPT(
        CURLOPT_PROTOCOLS,
        (long)(CURLPROTO_HTTP | CURLPROTO_HTTPS)
    );
#endif
    LM_REST_LMX_CURL_SETOPT(CURLOPT_NOSIGNAL, 1L);
    LM_REST_LMX_CURL_SETOPT(CURLOPT_CONNECTTIMEOUT_MS, 10000L);
    LM_REST_LMX_CURL_SETOPT(CURLOPT_TIMEOUT_MS, 30000L);
    LM_REST_LMX_CURL_SETOPT(CURLOPT_SSL_VERIFYPEER, 1L);
    LM_REST_LMX_CURL_SETOPT(CURLOPT_SSL_VERIFYHOST, 2L);
    LM_REST_LMX_CURL_SETOPT(
        CURLOPT_WRITEFUNCTION,
        lm_rest_lmx_http_discard_response
    );

#undef LM_REST_LMX_CURL_SETOPT

    result = curl_easy_perform(easy);
    if (result != CURLE_OK ||
        curl_easy_getinfo(easy, CURLINFO_RESPONSE_CODE, &response_code) !=
            CURLE_OK ||
        response_code < 0L ||
        (unsigned long)response_code > (unsigned long)UINT_MAX) {
        goto cleanup;
    }
    *out_http_status = (unsigned)response_code;
    status = 0;

cleanup:
    curl_slist_free_all(headers);
    curl_easy_cleanup(easy);
    return status;
}

const char *lm_rest_lmx_http_client_provider_name(void) {
    return "libcurl";
}

int lm_rest_lmx_http_client_install_default(
    struct LmMessageThreadRuntime *runtime
) {
    LmRestLmxHttpClientContext *context;
    struct LmRestLmxProviderOpsV1 ops;

    if (runtime == 0 || curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK) {
        return 1;
    }
    context = (LmRestLmxHttpClientContext *)calloc(1U, sizeof(*context));
    if (context == 0) {
        curl_global_cleanup();
        return 1;
    }
    context->curl_initialized = 1;
    ops.abi_size = sizeof(ops);
    ops.post = lm_rest_lmx_http_client_post;
    ops.destroy = lm_rest_lmx_http_client_destroy;
    if (lm_message_thread_runtime_set_rest_lmx_provider(
            runtime,
            &ops,
            context
        ) != 0) {
        lm_rest_lmx_http_client_destroy(context);
        return 1;
    }
    return 0;
}

#elif LM_REST_LMX_CLIENT_PROVIDER == LM_REST_LMX_CLIENT_PROVIDER_WINHTTP

#if !defined(_WIN32)
#error "LM_REST_LMX_CLIENT_PROVIDER_WINHTTP requires a Windows target"
#endif

#include <windows.h>
#include <winhttp.h>

typedef struct LmRestLmxHttpClientContext {
    HINTERNET session;
} LmRestLmxHttpClientContext;

static wchar_t *lm_rest_lmx_http_wide_uri(const char *uri) {
    size_t length;
    int wide_length;
    wchar_t *wide_uri;

    if (uri == 0) {
        return 0;
    }
    length = strlen(uri);
    if (length > (size_t)INT_MAX - 1U) {
        return 0;
    }
    wide_length = MultiByteToWideChar(
        CP_UTF8,
        MB_ERR_INVALID_CHARS,
        uri,
        -1,
        0,
        0
    );
    if (wide_length <= 0 ||
        (size_t)wide_length > SIZE_MAX / sizeof(*wide_uri)) {
        return 0;
    }
    wide_uri = (wchar_t *)calloc((size_t)wide_length, sizeof(*wide_uri));
    if (wide_uri == 0 || MultiByteToWideChar(
            CP_UTF8,
            MB_ERR_INVALID_CHARS,
            uri,
            -1,
            wide_uri,
            wide_length
        ) != wide_length) {
        free(wide_uri);
        return 0;
    }
    return wide_uri;
}

static wchar_t *lm_rest_lmx_http_wide_slice(
    const wchar_t *source,
    DWORD length
) {
    wchar_t *copy;

    if (source == 0 ||
        (size_t)length > SIZE_MAX / sizeof(*copy) - 1U) {
        return 0;
    }
    copy = (wchar_t *)calloc((size_t)length + 1U, sizeof(*copy));
    if (copy == 0) {
        return 0;
    }
    if (length != 0U) {
        memcpy(copy, source, (size_t)length * sizeof(*copy));
    }
    return copy;
}

static void lm_rest_lmx_http_client_destroy(void *opaque_context) {
    LmRestLmxHttpClientContext *context =
        (LmRestLmxHttpClientContext *)opaque_context;

    if (context == 0) {
        return;
    }
    if (context->session != 0) {
        (void)WinHttpCloseHandle(context->session);
        context->session = 0;
    }
    free(context);
}

static int lm_rest_lmx_http_client_post(
    void *opaque_context,
    const char *normalized_uri,
    const char *body,
    size_t length,
    unsigned *out_http_status
) {
    static const wchar_t headers[] =
        L"Content-Type: application/lmx\r\nAccept: application/lmx\r\n";
    LmRestLmxHttpClientContext *context =
        (LmRestLmxHttpClientContext *)opaque_context;
    wchar_t *wide_uri = 0;
    wchar_t *host = 0;
    wchar_t *path = 0;
    URL_COMPONENTS components;
    HINTERNET connection = 0;
    HINTERNET request = 0;
    DWORD request_flags = WINHTTP_FLAG_ESCAPE_DISABLE;
    DWORD disabled_features = WINHTTP_DISABLE_REDIRECTS;
    DWORD body_length = (DWORD)length;
    DWORD response_status = 0U;
    DWORD response_status_size = sizeof(response_status);
    int status = 1;

    if (out_http_status != 0) {
        *out_http_status = 0U;
    }
    if (context == 0 || context->session == 0 || normalized_uri == 0 ||
        out_http_status == 0 || (body == 0 && length != 0U) ||
        (size_t)body_length != length) {
        return 1;
    }
    wide_uri = lm_rest_lmx_http_wide_uri(normalized_uri);
    if (wide_uri == 0) {
        goto cleanup;
    }

    memset(&components, 0, sizeof(components));
    components.dwStructSize = sizeof(components);
    components.dwSchemeLength = (DWORD)-1;
    components.dwHostNameLength = (DWORD)-1;
    components.dwUrlPathLength = (DWORD)-1;
    components.dwExtraInfoLength = (DWORD)-1;
    if (!WinHttpCrackUrl(wide_uri, 0U, 0U, &components) ||
        (components.nScheme != INTERNET_SCHEME_HTTP &&
         components.nScheme != INTERNET_SCHEME_HTTPS) ||
        components.lpszHostName == 0 || components.dwHostNameLength == 0U ||
        components.dwExtraInfoLength != 0U) {
        goto cleanup;
    }
    host = lm_rest_lmx_http_wide_slice(
        components.lpszHostName,
        components.dwHostNameLength
    );
    if (components.lpszUrlPath != 0 && components.dwUrlPathLength != 0U) {
        path = lm_rest_lmx_http_wide_slice(
            components.lpszUrlPath,
            components.dwUrlPathLength
        );
    } else {
        path = lm_rest_lmx_http_wide_slice(L"/", 1U);
    }
    if (host == 0 || path == 0) {
        goto cleanup;
    }

    connection = WinHttpConnect(
        context->session,
        host,
        components.nPort,
        0U
    );
    if (connection == 0) {
        goto cleanup;
    }
    if (components.nScheme == INTERNET_SCHEME_HTTPS) {
        request_flags |= WINHTTP_FLAG_SECURE;
    }
    request = WinHttpOpenRequest(
        connection,
        L"POST",
        path,
        0,
        WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES,
        request_flags
    );
    if (request == 0 || !WinHttpSetOption(
            request,
            WINHTTP_OPTION_DISABLE_FEATURE,
            &disabled_features,
            sizeof(disabled_features)
        ) || !WinHttpSetTimeouts(request, 10000, 10000, 30000, 30000) ||
        !WinHttpAddRequestHeaders(
            request,
            headers,
            (DWORD)-1,
            WINHTTP_ADDREQ_FLAG_ADD | WINHTTP_ADDREQ_FLAG_REPLACE
        ) || !WinHttpSendRequest(
            request,
            WINHTTP_NO_ADDITIONAL_HEADERS,
            0U,
            body_length == 0U ? WINHTTP_NO_REQUEST_DATA : (LPVOID)body,
            body_length,
            body_length,
            0U
        ) || !WinHttpReceiveResponse(request, 0) ||
        !WinHttpQueryHeaders(
            request,
            WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
            WINHTTP_HEADER_NAME_BY_INDEX,
            &response_status,
            &response_status_size,
            WINHTTP_NO_HEADER_INDEX
        )) {
        goto cleanup;
    }
    *out_http_status = (unsigned)response_status;
    status = 0;

cleanup:
    if (request != 0) {
        (void)WinHttpCloseHandle(request);
    }
    if (connection != 0) {
        (void)WinHttpCloseHandle(connection);
    }
    free(path);
    free(host);
    free(wide_uri);
    return status;
}

const char *lm_rest_lmx_http_client_provider_name(void) {
    return "winhttp";
}

int lm_rest_lmx_http_client_install_default(
    struct LmMessageThreadRuntime *runtime
) {
    LmRestLmxHttpClientContext *context;
    struct LmRestLmxProviderOpsV1 ops;

    if (runtime == 0) {
        return 1;
    }
    context = (LmRestLmxHttpClientContext *)calloc(1U, sizeof(*context));
    if (context == 0) {
        return 1;
    }
    context->session = WinHttpOpen(
        L"Lingvamyxa REST/LMX/1",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS,
        0U
    );
    if (context->session == 0) {
        lm_rest_lmx_http_client_destroy(context);
        return 1;
    }
    ops.abi_size = sizeof(ops);
    ops.post = lm_rest_lmx_http_client_post;
    ops.destroy = lm_rest_lmx_http_client_destroy;
    if (lm_message_thread_runtime_set_rest_lmx_provider(
            runtime,
            &ops,
            context
        ) != 0) {
        lm_rest_lmx_http_client_destroy(context);
        return 1;
    }
    return 0;
}

#else

const char *lm_rest_lmx_http_client_provider_name(void) {
    return "none";
}

int lm_rest_lmx_http_client_install_default(
    struct LmMessageThreadRuntime *runtime
) {
    if (runtime == 0) {
        return 1;
    }
    return lm_message_thread_runtime_set_rest_lmx_provider(runtime, 0, 0);
}

#endif

