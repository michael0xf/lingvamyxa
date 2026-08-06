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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

extern int lm_message_thread_runtime_admit_lmx(
    struct LmMessageThreadRuntime *runtime,
    const char *route,
    const char *lmx,
    size_t length
);

#ifndef LM_MESSAGE_STATUS_ROUTE_NOT_FOUND
#define LM_MESSAGE_STATUS_ROUTE_NOT_FOUND 64
#endif
#ifndef LM_MESSAGE_STATUS_INVALID_ADDRESS
#define LM_MESSAGE_STATUS_INVALID_ADDRESS 66
#endif
#ifndef LM_MESSAGE_STATUS_APPLICATION_STOPPING
#define LM_MESSAGE_STATUS_APPLICATION_STOPPING 70
#endif

#ifndef LM_REST_LMX_SERVER_PROVIDER_NONE
#define LM_REST_LMX_SERVER_PROVIDER_NONE 0
#endif
#ifndef LM_REST_LMX_SERVER_PROVIDER_CIVETWEB
#define LM_REST_LMX_SERVER_PROVIDER_CIVETWEB 1
#endif

#ifndef LM_REST_LMX_SERVER_PROVIDER
#define LM_REST_LMX_SERVER_PROVIDER LM_REST_LMX_SERVER_PROVIDER_NONE
#endif

#if LM_REST_LMX_SERVER_PROVIDER != LM_REST_LMX_SERVER_PROVIDER_NONE && \
    LM_REST_LMX_SERVER_PROVIDER != LM_REST_LMX_SERVER_PROVIDER_CIVETWEB
#error "Unknown LM_REST_LMX_SERVER_PROVIDER value"
#endif

#if LM_REST_LMX_SERVER_PROVIDER == LM_REST_LMX_SERVER_PROVIDER_CIVETWEB

#include <civetweb.h>

struct LmRestLmxHttpServer {
    struct LmMessageThreadRuntime *runtime;
    struct mg_context *context;
    char *base_path;
    size_t base_path_length;
    size_t max_body_bytes;
    unsigned listening_port;
    int library_initialized;
};

static int lm_rest_lmx_http_ascii_is_hex(unsigned char value) {
    return (value >= (unsigned char)'0' && value <= (unsigned char)'9') ||
        (value >= (unsigned char)'a' && value <= (unsigned char)'f') ||
        (value >= (unsigned char)'A' && value <= (unsigned char)'F');
}

static unsigned char lm_rest_lmx_http_ascii_lower(unsigned char value) {
    if (value >= (unsigned char)'A' && value <= (unsigned char)'Z') {
        return (unsigned char)(value + ((unsigned char)'a' -
            (unsigned char)'A'));
    }
    return value;
}

static int lm_rest_lmx_http_ascii_equal(
    const char *left,
    const char *right
) {
    size_t index = 0U;

    if (left == 0 || right == 0) {
        return 0;
    }
    while (left[index] != '\0' && right[index] != '\0') {
        if (lm_rest_lmx_http_ascii_lower((unsigned char)left[index]) !=
            lm_rest_lmx_http_ascii_lower((unsigned char)right[index])) {
            return 0;
        }
        index += 1U;
    }
    return left[index] == '\0' && right[index] == '\0';
}

static int lm_rest_lmx_http_header_value_is(
    const char *value,
    const char *expected
) {
    const unsigned char *begin = (const unsigned char *)value;
    const unsigned char *end;
    size_t expected_length;
    size_t index;

    if (value == 0 || expected == 0) {
        return 0;
    }
    while (*begin == (unsigned char)' ' || *begin == (unsigned char)'\t') {
        begin += 1;
    }
    end = begin + strlen((const char *)begin);
    while (end > begin &&
        (end[-1] == (unsigned char)' ' || end[-1] == (unsigned char)'\t')) {
        end -= 1;
    }
    expected_length = strlen(expected);
    if ((size_t)(end - begin) != expected_length) {
        return 0;
    }
    for (index = 0U; index < expected_length; index += 1U) {
        if (lm_rest_lmx_http_ascii_lower(begin[index]) !=
            lm_rest_lmx_http_ascii_lower((unsigned char)expected[index])) {
            return 0;
        }
    }
    return 1;
}

static int lm_rest_lmx_http_parse_size(
    const char *text,
    size_t *out_value
) {
    const unsigned char *begin = (const unsigned char *)text;
    const unsigned char *end;
    size_t value = 0U;

    if (text == 0 || out_value == 0) {
        return 1;
    }
    while (*begin == (unsigned char)' ' || *begin == (unsigned char)'\t') {
        begin += 1;
    }
    end = begin + strlen((const char *)begin);
    while (end > begin &&
        (end[-1] == (unsigned char)' ' || end[-1] == (unsigned char)'\t')) {
        end -= 1;
    }
    if (begin == end) {
        return 1;
    }
    while (begin < end) {
        unsigned digit;

        if (*begin < (unsigned char)'0' || *begin > (unsigned char)'9') {
            return 1;
        }
        digit = (unsigned)(*begin - (unsigned char)'0');
        if (value > (SIZE_MAX - (size_t)digit) / 10U) {
            return 2;
        }
        value = value * 10U + (size_t)digit;
        begin += 1;
    }
    *out_value = value;
    return 0;
}

static int lm_rest_lmx_http_parse_unsigned(
    const char *text,
    unsigned *out_value
) {
    size_t value = 0U;
    int status;

    if (out_value == 0) {
        return 1;
    }
    status = lm_rest_lmx_http_parse_size(text, &value);
    if (status != 0 || value > (size_t)UINT_MAX) {
        return 1;
    }
    *out_value = (unsigned)value;
    return 0;
}

static int lm_rest_lmx_http_bind_address_is_valid(const char *address) {
    const unsigned char *text = (const unsigned char *)address;
    size_t index;
    size_t length;

    if (text == 0 || text[0] == 0U) {
        return 0;
    }
    length = strlen(address);
    if (length > 255U) {
        return 0;
    }
    for (index = 0U; index < length; index += 1U) {
        unsigned char value = text[index];

        if (!((value >= (unsigned char)'a' && value <= (unsigned char)'z') ||
              (value >= (unsigned char)'A' && value <= (unsigned char)'Z') ||
              (value >= (unsigned char)'0' && value <= (unsigned char)'9') ||
              value == (unsigned char)'.' || value == (unsigned char)'-' ||
              value == (unsigned char)':' || value == (unsigned char)'[' ||
              value == (unsigned char)']')) {
            return 0;
        }
    }
    return 1;
}

static int lm_rest_lmx_http_base_path_is_valid(const char *path) {
    const unsigned char *text = (const unsigned char *)path;
    size_t length;
    size_t index;
    size_t segment_start;

    if (text == 0) {
        return 0;
    }
    length = strlen(path);
    if (length == 0U || (length == 1U && text[0] == (unsigned char)'/')) {
        return 1;
    }
    if (text[0] != (unsigned char)'/' ||
        text[length - 1U] == (unsigned char)'/') {
        return 0;
    }
    segment_start = 1U;
    for (index = 1U; index < length; index += 1U) {
        unsigned char value = text[index];

        if (value <= 0x20U || value > 0x7eU ||
            value == (unsigned char)'\\' || value == (unsigned char)'?' ||
            value == (unsigned char)'#') {
            return 0;
        }
        if (value == (unsigned char)'/') {
            size_t segment_length = index - segment_start;

            if (segment_length == 0U ||
                (segment_length == 1U &&
                 text[segment_start] == (unsigned char)'.') ||
                (segment_length == 2U &&
                 text[segment_start] == (unsigned char)'.' &&
                 text[segment_start + 1U] == (unsigned char)'.')) {
                return 0;
            }
            segment_start = index + 1U;
        } else if (value == (unsigned char)'%') {
            if (index + 2U >= length ||
                !lm_rest_lmx_http_ascii_is_hex(text[index + 1U]) ||
                !lm_rest_lmx_http_ascii_is_hex(text[index + 2U])) {
                return 0;
            }
            index += 2U;
        }
    }
    if ((length - segment_start == 1U &&
         text[segment_start] == (unsigned char)'.') ||
        (length - segment_start == 2U &&
         text[segment_start] == (unsigned char)'.' &&
         text[segment_start + 1U] == (unsigned char)'.')) {
        return 0;
    }
    return 1;
}

static char *lm_rest_lmx_http_copy_text(const char *text) {
    size_t length;
    char *copy;

    if (text == 0) {
        return 0;
    }
    length = strlen(text);
    copy = (char *)malloc(length + 1U);
    if (copy == 0) {
        return 0;
    }
    memcpy(copy, text, length + 1U);
    return copy;
}

static int lm_rest_lmx_http_send_status(
    struct mg_connection *connection,
    unsigned status
) {
    if (status == 202U) {
        (void)mg_printf(
            connection,
            "HTTP/1.1 202 Accepted\r\n"
            "Content-Length: 0\r\n"
            "Connection: close\r\n\r\n"
        );
        return 202;
    }
    switch (status) {
        case 400U:
        case 404U:
        case 405U:
        case 411U:
        case 413U:
        case 415U:
        case 503U:
            break;
        default:
            status = 500U;
            break;
    }
    (void)mg_send_http_error(connection, (int)status, "%s", "");
    return (int)status;
}

static unsigned lm_rest_lmx_http_admission_status(int status) {
    if (status == 0) {
        return 202U;
    }
    if (status == LM_MESSAGE_STATUS_ROUTE_NOT_FOUND) {
        return 404U;
    }
    if (status == LM_MESSAGE_STATUS_INVALID_ADDRESS) {
        return 400U;
    }
    if (status == LM_MESSAGE_STATUS_APPLICATION_STOPPING) {
        return 503U;
    }
    return 500U;
}

static const char *lm_rest_lmx_http_route(
    const struct LmRestLmxHttpServer *server,
    const char *raw_uri,
    unsigned *out_status
) {
    size_t raw_length;

    if (out_status != 0) {
        *out_status = 400U;
    }
    if (server == 0 || raw_uri == 0 || out_status == 0) {
        return 0;
    }
    if (server->base_path_length == 0U) {
        return raw_uri;
    }
    raw_length = strlen(raw_uri);
    if (raw_length <= server->base_path_length ||
        memcmp(raw_uri, server->base_path, server->base_path_length) != 0 ||
        raw_uri[server->base_path_length] != '/') {
        *out_status = 404U;
        return 0;
    }
    return raw_uri + server->base_path_length;
}

static int lm_rest_lmx_http_begin_request(
    struct mg_connection *connection
) {
    const struct mg_request_info *request;
    struct LmRestLmxHttpServer *server;
    const char *content_length_text = 0;
    const char *content_type_text = 0;
    const char *route;
    char *body = 0;
    size_t content_length = 0U;
    size_t received = 0U;
    unsigned route_status = 400U;
    unsigned response_status;
    int content_length_count = 0;
    int transfer_encoding_count = 0;
    int content_type_count = 0;
    int index;
    int parse_status;
    int admission_status;

    if (connection == 0) {
        return 500;
    }
    request = mg_get_request_info(connection);
    if (request == 0 || request->user_data == 0) {
        return lm_rest_lmx_http_send_status(connection, 500U);
    }
    server = (struct LmRestLmxHttpServer *)request->user_data;
    if (!lm_rest_lmx_http_ascii_equal(request->request_method, "POST")) {
        return lm_rest_lmx_http_send_status(connection, 405U);
    }
    if (request->query_string != 0) {
        return lm_rest_lmx_http_send_status(connection, 400U);
    }
    route = lm_rest_lmx_http_route(
        server,
        request->local_uri_raw,
        &route_status
    );
    if (route == 0) {
        return lm_rest_lmx_http_send_status(connection, route_status);
    }

    for (index = 0; index < request->num_headers; index += 1) {
        const char *name = request->http_headers[index].name;
        const char *value = request->http_headers[index].value;

        if (lm_rest_lmx_http_ascii_equal(name, "Content-Length")) {
            content_length_count += 1;
            content_length_text = value;
        } else if (lm_rest_lmx_http_ascii_equal(
                name,
                "Transfer-Encoding"
            )) {
            transfer_encoding_count += 1;
        } else if (lm_rest_lmx_http_ascii_equal(name, "Content-Type")) {
            content_type_count += 1;
            content_type_text = value;
        }
    }
    if (transfer_encoding_count != 0 || content_length_count == 0) {
        return lm_rest_lmx_http_send_status(connection, 411U);
    }
    if (content_length_count != 1) {
        return lm_rest_lmx_http_send_status(connection, 400U);
    }
    if (content_type_count != 1 ||
        !lm_rest_lmx_http_header_value_is(
            content_type_text,
            "application/lmx"
        )) {
        return lm_rest_lmx_http_send_status(connection, 415U);
    }
    parse_status = lm_rest_lmx_http_parse_size(
        content_length_text,
        &content_length
    );
    if (parse_status == 2) {
        return lm_rest_lmx_http_send_status(connection, 413U);
    }
    if (parse_status != 0) {
        return lm_rest_lmx_http_send_status(connection, 400U);
    }
    if (content_length > server->max_body_bytes) {
        return lm_rest_lmx_http_send_status(connection, 413U);
    }

    body = (char *)malloc(content_length == 0U ? 1U : content_length);
    if (body == 0) {
        return lm_rest_lmx_http_send_status(connection, 500U);
    }
    while (received < content_length) {
        size_t remaining = content_length - received;
        size_t chunk = remaining > (size_t)INT_MAX
            ? (size_t)INT_MAX
            : remaining;
        int count = mg_read(connection, body + received, chunk);

        if (count <= 0 || (size_t)count > remaining) {
            free(body);
            return lm_rest_lmx_http_send_status(connection, 400U);
        }
        received += (size_t)count;
    }
    admission_status = lm_message_thread_runtime_admit_lmx(
        server->runtime,
        route,
        body,
        content_length
    );
    free(body);
    response_status = lm_rest_lmx_http_admission_status(admission_status);
    return lm_rest_lmx_http_send_status(connection, response_status);
}

static int lm_rest_lmx_http_server_options_are_valid(
    const LmRestLmxHttpServerOptionsV1 *options
) {
    return options != 0 && options->abi_size >= sizeof(*options) &&
        lm_rest_lmx_http_bind_address_is_valid(options->bind_address) &&
        options->port <= 65535U &&
        lm_rest_lmx_http_base_path_is_valid(options->base_path) &&
        options->max_body_bytes != 0U && options->worker_count != 0U &&
        options->worker_count <= 1024U &&
        options->request_timeout_ms != 0U;
}

const char *lm_rest_lmx_http_server_provider_name(void) {
    return "civetweb";
}

int lm_rest_lmx_http_server_start(
    struct LmMessageThreadRuntime *runtime,
    const LmRestLmxHttpServerOptionsV1 *options,
    struct LmRestLmxHttpServer **out_server
) {
    struct LmRestLmxHttpServer *server;
    struct mg_callbacks callbacks;
    struct mg_server_port ports[4];
    char listening_ports[320];
    char worker_count[32];
    char request_timeout[32];
    const char *configuration[13];
    const char *base_path;
    int configuration_index = 0;
    int port_count;
    int written;

    if (out_server == 0) {
        return 1;
    }
    *out_server = 0;
    if (runtime == 0 ||
        !lm_rest_lmx_http_server_options_are_valid(options)) {
        return 1;
    }
    base_path = strcmp(options->base_path, "/") == 0
        ? ""
        : options->base_path;
    server = (struct LmRestLmxHttpServer *)calloc(1U, sizeof(*server));
    if (server == 0) {
        return 1;
    }
    server->runtime = runtime;
    server->base_path = lm_rest_lmx_http_copy_text(base_path);
    if (server->base_path == 0) {
        free(server);
        return 1;
    }
    server->base_path_length = strlen(server->base_path);
    server->max_body_bytes = options->max_body_bytes;

    if (strchr(options->bind_address, ':') != 0 &&
        options->bind_address[0] != '[') {
        written = snprintf(
            listening_ports,
            sizeof(listening_ports),
            "[%s]:%u",
            options->bind_address,
            options->port
        );
    } else {
        written = snprintf(
            listening_ports,
            sizeof(listening_ports),
            "%s:%u",
            options->bind_address,
            options->port
        );
    }
    if (written < 0 || (size_t)written >= sizeof(listening_ports)) {
        free(server->base_path);
        free(server);
        return 1;
    }
    written = snprintf(
        worker_count,
        sizeof(worker_count),
        "%u",
        options->worker_count
    );
    if (written < 0 || (size_t)written >= sizeof(worker_count)) {
        free(server->base_path);
        free(server);
        return 1;
    }
    written = snprintf(
        request_timeout,
        sizeof(request_timeout),
        "%u",
        options->request_timeout_ms
    );
    if (written < 0 || (size_t)written >= sizeof(request_timeout)) {
        free(server->base_path);
        free(server);
        return 1;
    }

    configuration[configuration_index++] = "listening_ports";
    configuration[configuration_index++] = listening_ports;
    configuration[configuration_index++] = "num_threads";
    configuration[configuration_index++] = worker_count;
    configuration[configuration_index++] = "request_timeout_ms";
    configuration[configuration_index++] = request_timeout;
    configuration[configuration_index++] = "enable_keep_alive";
    configuration[configuration_index++] = "no";
    configuration[configuration_index++] = "decode_url";
    configuration[configuration_index++] = "no";
    configuration[configuration_index++] = "additional_header";
    configuration[configuration_index++] = "Allow: POST";
    configuration[configuration_index] = 0;

    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.begin_request = lm_rest_lmx_http_begin_request;
    (void)mg_init_library(MG_FEATURES_DEFAULT);
    server->library_initialized = 1;
    server->context = mg_start(&callbacks, server, configuration);
    if (server->context == 0) {
        (void)mg_exit_library();
        free(server->base_path);
        free(server);
        return 1;
    }
    memset(ports, 0, sizeof(ports));
    port_count = mg_get_server_ports(
        server->context,
        (int)(sizeof(ports) / sizeof(ports[0])),
        ports
    );
    if (port_count < 1 || ports[0].port <= 0 || ports[0].port > 65535 ||
        ports[0].is_ssl) {
        mg_stop(server->context);
        server->context = 0;
        (void)mg_exit_library();
        free(server->base_path);
        free(server);
        return 1;
    }
    server->listening_port = (unsigned)ports[0].port;
    *out_server = server;
    return 0;
}

int lm_rest_lmx_http_server_start_default(
    struct LmMessageThreadRuntime *runtime,
    struct LmRestLmxHttpServer **out_server
) {
    const char *bind_address = getenv("LM_REST_LMX_LISTEN_ADDRESS");
    const char *port_text = getenv("LM_REST_LMX_LISTEN_PORT");
    const char *base_path = getenv("LM_REST_LMX_BASE_PATH");
    const char *max_body_text = getenv("LM_REST_LMX_MAX_BODY_BYTES");
    const char *worker_count_text = getenv("LM_REST_LMX_WORKER_COUNT");
    const char *timeout_text = getenv("LM_REST_LMX_REQUEST_TIMEOUT_MS");
    LmRestLmxHttpServerOptionsV1 options;

    memset(&options, 0, sizeof(options));
    options.abi_size = sizeof(options);
    options.bind_address = bind_address == 0 ? "127.0.0.1" : bind_address;
    options.port = 8080U;
    options.base_path = base_path == 0 ? "" : base_path;
    options.max_body_bytes = 1024U * 1024U;
    options.worker_count = 4U;
    options.request_timeout_ms = 10000U;
    if ((port_text != 0 &&
         lm_rest_lmx_http_parse_unsigned(port_text, &options.port) != 0) ||
        (max_body_text != 0 &&
         lm_rest_lmx_http_parse_size(
             max_body_text,
             &options.max_body_bytes
         ) != 0) ||
        (worker_count_text != 0 &&
         lm_rest_lmx_http_parse_unsigned(
             worker_count_text,
             &options.worker_count
         ) != 0) ||
        (timeout_text != 0 &&
         lm_rest_lmx_http_parse_unsigned(
             timeout_text,
             &options.request_timeout_ms
         ) != 0)) {
        return 1;
    }
    return lm_rest_lmx_http_server_start(runtime, &options, out_server);
}

int lm_rest_lmx_http_server_stop(
    struct LmRestLmxHttpServer **server_slot
) {
    struct LmRestLmxHttpServer *server;

    if (server_slot == 0) {
        return 1;
    }
    server = *server_slot;
    if (server == 0) {
        return 0;
    }
    if (server->context != 0) {
        mg_stop(server->context);
        server->context = 0;
    }
    if (server->library_initialized) {
        (void)mg_exit_library();
        server->library_initialized = 0;
    }
    free(server->base_path);
    server->base_path = 0;
    server->runtime = 0;
    free(server);
    *server_slot = 0;
    return 0;
}

int lm_rest_lmx_http_server_port(
    const struct LmRestLmxHttpServer *server,
    unsigned *out_port
) {
    if (server == 0 || out_port == 0 || server->context == 0 ||
        server->listening_port == 0U) {
        return 1;
    }
    *out_port = server->listening_port;
    return 0;
}

#else

const char *lm_rest_lmx_http_server_provider_name(void) {
    return "none";
}

int lm_rest_lmx_http_server_start(
    struct LmMessageThreadRuntime *runtime,
    const LmRestLmxHttpServerOptionsV1 *options,
    struct LmRestLmxHttpServer **out_server
) {
    (void)options;
    if (out_server == 0) {
        return 1;
    }
    *out_server = 0;
    if (runtime == 0) {
        return 1;
    }
    return 0;
}

int lm_rest_lmx_http_server_start_default(
    struct LmMessageThreadRuntime *runtime,
    struct LmRestLmxHttpServer **out_server
) {
    return lm_rest_lmx_http_server_start(runtime, 0, out_server);
}

int lm_rest_lmx_http_server_stop(
    struct LmRestLmxHttpServer **server_slot
) {
    if (server_slot == 0) {
        return 1;
    }
    return *server_slot == 0 ? 0 : 1;
}

int lm_rest_lmx_http_server_port(
    const struct LmRestLmxHttpServer *server,
    unsigned *out_port
) {
    (void)server;
    (void)out_port;
    return 1;
}

#endif

