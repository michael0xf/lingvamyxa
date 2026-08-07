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
static inline LM_LMX_UNUSED_ENTRY_HELPER int lm_lmx_message_thread_invoke_diagnostic(
    struct LmMessageThread *thread,
    void (*entry)(struct LmMessageThread *, void *),
    void *argument
) {
    LmMessageThreadExecutionContext *context;
    void *previous;
    int status = 0;
    if (thread == 0 || entry == 0) {
        return 1;
    }
    context = (LmMessageThreadExecutionContext *)calloc(1U, sizeof(*context));
    if (context == 0) {
        lm_message_thread_request_failure(thread, 1);
        return 1;
    }
    previous = lm_message_thread_set_execution_context(thread, context);
    if (setjmp(context->diagnostic_root) == 0) {
        entry(thread, argument);
    } else {
        status = context->diagnostic_code == 0 ? 1 : context->diagnostic_code;
        lm_message_thread_request_failure(thread, status);
    }
    (void)lm_message_thread_set_execution_context(thread, previous);
    free(context);
    return status;
}
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

#ifndef LM_REST_LMX_SERVER_UNUSED
#define LM_REST_LMX_SERVER_UNUSED (void)
#endif

#include <stddef.h>

#define LmCivetwebRequestInfo struct mg_request_info


#ifndef LM_LMX_TYPEDEF_DEFINED_LmOwnEdgeKind
#define LM_LMX_TYPEDEF_DEFINED_LmOwnEdgeKind 1
#define LM_LMX_TYPEDEF_ID_A_LmOwnEdgeKind 0x1b0ba13001d5c752ULL
#define LM_LMX_TYPEDEF_ID_B_LmOwnEdgeKind 0x9d7b68ae3c8fab5bULL
typedef int LmOwnEdgeKind;
#else
#if !defined(LM_LMX_TYPEDEF_ID_A_LmOwnEdgeKind) || !defined(LM_LMX_TYPEDEF_ID_B_LmOwnEdgeKind) || LM_LMX_TYPEDEF_ID_A_LmOwnEdgeKind != 0x1b0ba13001d5c752ULL || LM_LMX_TYPEDEF_ID_B_LmOwnEdgeKind != 0x9d7b68ae3c8fab5bULL
#error "Lingvamyxa conflicting typedef projection for LmOwnEdgeKind"
#endif
#endif
#ifndef LM_LMX_TYPEDEF_DEFINED_LmMessageThreadState
#define LM_LMX_TYPEDEF_DEFINED_LmMessageThreadState 1
#define LM_LMX_TYPEDEF_ID_A_LmMessageThreadState 0x680d7f7cf5d18c05ULL
#define LM_LMX_TYPEDEF_ID_B_LmMessageThreadState 0x4a157ef92e57affaULL
typedef int LmMessageThreadState;
#else
#if !defined(LM_LMX_TYPEDEF_ID_A_LmMessageThreadState) || !defined(LM_LMX_TYPEDEF_ID_B_LmMessageThreadState) || LM_LMX_TYPEDEF_ID_A_LmMessageThreadState != 0x680d7f7cf5d18c05ULL || LM_LMX_TYPEDEF_ID_B_LmMessageThreadState != 0x4a157ef92e57affaULL
#error "Lingvamyxa conflicting typedef projection for LmMessageThreadState"
#endif
#endif


#ifndef LM_LMX_TYPEDEF_DEFINED_LmOwnPtrStack
#define LM_LMX_TYPEDEF_DEFINED_LmOwnPtrStack 1
#define LM_LMX_TYPEDEF_ID_A_LmOwnPtrStack 0x9ae3c1a28ace4260ULL
#define LM_LMX_TYPEDEF_ID_B_LmOwnPtrStack 0x2a7c240a466f8699ULL
typedef struct LmOwnPtrStack LmOwnPtrStack;
#else
#if !defined(LM_LMX_TYPEDEF_ID_A_LmOwnPtrStack) || !defined(LM_LMX_TYPEDEF_ID_B_LmOwnPtrStack) || LM_LMX_TYPEDEF_ID_A_LmOwnPtrStack != 0x9ae3c1a28ace4260ULL || LM_LMX_TYPEDEF_ID_B_LmOwnPtrStack != 0x2a7c240a466f8699ULL
#error "Lingvamyxa conflicting typedef projection for LmOwnPtrStack"
#endif
#endif
#ifndef LM_LMX_TYPEDEF_DEFINED_LmOwnValueStack
#define LM_LMX_TYPEDEF_DEFINED_LmOwnValueStack 1
#define LM_LMX_TYPEDEF_ID_A_LmOwnValueStack 0xa909010bae94e210ULL
#define LM_LMX_TYPEDEF_ID_B_LmOwnValueStack 0xa4cd51ea3c5f8959ULL
typedef struct LmOwnValueStack LmOwnValueStack;
#else
#if !defined(LM_LMX_TYPEDEF_ID_A_LmOwnValueStack) || !defined(LM_LMX_TYPEDEF_ID_B_LmOwnValueStack) || LM_LMX_TYPEDEF_ID_A_LmOwnValueStack != 0xa909010bae94e210ULL || LM_LMX_TYPEDEF_ID_B_LmOwnValueStack != 0xa4cd51ea3c5f8959ULL
#error "Lingvamyxa conflicting typedef projection for LmOwnValueStack"
#endif
#endif
#ifndef LM_LMX_TYPEDEF_DEFINED_LmOwnAllocationDescriptor
#define LM_LMX_TYPEDEF_DEFINED_LmOwnAllocationDescriptor 1
#define LM_LMX_TYPEDEF_ID_A_LmOwnAllocationDescriptor 0x8af26fce7d92fafcULL
#define LM_LMX_TYPEDEF_ID_B_LmOwnAllocationDescriptor 0xb93049b7294f551dULL
typedef struct LmOwnAllocationDescriptor LmOwnAllocationDescriptor;
#else
#if !defined(LM_LMX_TYPEDEF_ID_A_LmOwnAllocationDescriptor) || !defined(LM_LMX_TYPEDEF_ID_B_LmOwnAllocationDescriptor) || LM_LMX_TYPEDEF_ID_A_LmOwnAllocationDescriptor != 0x8af26fce7d92fafcULL || LM_LMX_TYPEDEF_ID_B_LmOwnAllocationDescriptor != 0xb93049b7294f551dULL
#error "Lingvamyxa conflicting typedef projection for LmOwnAllocationDescriptor"
#endif
#endif
#ifndef LM_LMX_TYPEDEF_DEFINED_LmOwnLazyEdge
#define LM_LMX_TYPEDEF_DEFINED_LmOwnLazyEdge 1
#define LM_LMX_TYPEDEF_ID_A_LmOwnLazyEdge 0xdcd1331c45e5a0b0ULL
#define LM_LMX_TYPEDEF_ID_B_LmOwnLazyEdge 0xeb121590159143d9ULL
typedef struct LmOwnLazyEdge LmOwnLazyEdge;
#else
#if !defined(LM_LMX_TYPEDEF_ID_A_LmOwnLazyEdge) || !defined(LM_LMX_TYPEDEF_ID_B_LmOwnLazyEdge) || LM_LMX_TYPEDEF_ID_A_LmOwnLazyEdge != 0xdcd1331c45e5a0b0ULL || LM_LMX_TYPEDEF_ID_B_LmOwnLazyEdge != 0xeb121590159143d9ULL
#error "Lingvamyxa conflicting typedef projection for LmOwnLazyEdge"
#endif
#endif
#ifndef LM_LMX_TYPEDEF_DEFINED_LmOwnArena
#define LM_LMX_TYPEDEF_DEFINED_LmOwnArena 1
#define LM_LMX_TYPEDEF_ID_A_LmOwnArena 0x7c55fcb0a940aa3cULL
#define LM_LMX_TYPEDEF_ID_B_LmOwnArena 0xcdfc77b6ba3bf135ULL
typedef struct LmOwnArena LmOwnArena;
#else
#if !defined(LM_LMX_TYPEDEF_ID_A_LmOwnArena) || !defined(LM_LMX_TYPEDEF_ID_B_LmOwnArena) || LM_LMX_TYPEDEF_ID_A_LmOwnArena != 0x7c55fcb0a940aa3cULL || LM_LMX_TYPEDEF_ID_B_LmOwnArena != 0xcdfc77b6ba3bf135ULL
#error "Lingvamyxa conflicting typedef projection for LmOwnArena"
#endif
#endif
#ifndef LM_LMX_TYPEDEF_DEFINED_LmHostThread
#define LM_LMX_TYPEDEF_DEFINED_LmHostThread 1
#define LM_LMX_TYPEDEF_ID_A_LmHostThread 0x0ee0cef52f19074eULL
#define LM_LMX_TYPEDEF_ID_B_LmHostThread 0x9f6403534e9de8efULL
typedef struct LmHostThread LmHostThread;
#else
#if !defined(LM_LMX_TYPEDEF_ID_A_LmHostThread) || !defined(LM_LMX_TYPEDEF_ID_B_LmHostThread) || LM_LMX_TYPEDEF_ID_A_LmHostThread != 0x0ee0cef52f19074eULL || LM_LMX_TYPEDEF_ID_B_LmHostThread != 0x9f6403534e9de8efULL
#error "Lingvamyxa conflicting typedef projection for LmHostThread"
#endif
#endif
#ifndef LM_LMX_TYPEDEF_DEFINED_LmMutex
#define LM_LMX_TYPEDEF_DEFINED_LmMutex 1
#define LM_LMX_TYPEDEF_ID_A_LmMutex 0x26d0d7b596af7a34ULL
#define LM_LMX_TYPEDEF_ID_B_LmMutex 0xa8c35a70241bcbcdULL
typedef struct LmMutex LmMutex;
#else
#if !defined(LM_LMX_TYPEDEF_ID_A_LmMutex) || !defined(LM_LMX_TYPEDEF_ID_B_LmMutex) || LM_LMX_TYPEDEF_ID_A_LmMutex != 0x26d0d7b596af7a34ULL || LM_LMX_TYPEDEF_ID_B_LmMutex != 0xa8c35a70241bcbcdULL
#error "Lingvamyxa conflicting typedef projection for LmMutex"
#endif
#endif
#ifndef LM_LMX_TYPEDEF_DEFINED_LmCondition
#define LM_LMX_TYPEDEF_DEFINED_LmCondition 1
#define LM_LMX_TYPEDEF_ID_A_LmCondition 0xb0d7d0c00a11e970ULL
#define LM_LMX_TYPEDEF_ID_B_LmCondition 0x10616230e414eea1ULL
typedef struct LmCondition LmCondition;
#else
#if !defined(LM_LMX_TYPEDEF_ID_A_LmCondition) || !defined(LM_LMX_TYPEDEF_ID_B_LmCondition) || LM_LMX_TYPEDEF_ID_A_LmCondition != 0xb0d7d0c00a11e970ULL || LM_LMX_TYPEDEF_ID_B_LmCondition != 0x10616230e414eea1ULL
#error "Lingvamyxa conflicting typedef projection for LmCondition"
#endif
#endif
#ifndef LM_LMX_TYPEDEF_DEFINED_LmMessage
#define LM_LMX_TYPEDEF_DEFINED_LmMessage 1
#define LM_LMX_TYPEDEF_ID_A_LmMessage 0x7b0d63fc6cdc6820ULL
#define LM_LMX_TYPEDEF_ID_B_LmMessage 0xe461bdf9461df589ULL
typedef struct LmMessage LmMessage;
#else
#if !defined(LM_LMX_TYPEDEF_ID_A_LmMessage) || !defined(LM_LMX_TYPEDEF_ID_B_LmMessage) || LM_LMX_TYPEDEF_ID_A_LmMessage != 0x7b0d63fc6cdc6820ULL || LM_LMX_TYPEDEF_ID_B_LmMessage != 0xe461bdf9461df589ULL
#error "Lingvamyxa conflicting typedef projection for LmMessage"
#endif
#endif
#ifndef LM_LMX_TYPEDEF_DEFINED_LmMessageOutboxEntry
#define LM_LMX_TYPEDEF_DEFINED_LmMessageOutboxEntry 1
#define LM_LMX_TYPEDEF_ID_A_LmMessageOutboxEntry 0xda71b412537e2bf6ULL
#define LM_LMX_TYPEDEF_ID_B_LmMessageOutboxEntry 0x84fe1ad0603fee37ULL
typedef struct LmMessageOutboxEntry LmMessageOutboxEntry;
#else
#if !defined(LM_LMX_TYPEDEF_ID_A_LmMessageOutboxEntry) || !defined(LM_LMX_TYPEDEF_ID_B_LmMessageOutboxEntry) || LM_LMX_TYPEDEF_ID_A_LmMessageOutboxEntry != 0xda71b412537e2bf6ULL || LM_LMX_TYPEDEF_ID_B_LmMessageOutboxEntry != 0x84fe1ad0603fee37ULL
#error "Lingvamyxa conflicting typedef projection for LmMessageOutboxEntry"
#endif
#endif
#ifndef LM_LMX_TYPEDEF_DEFINED_LmMessageRoute
#define LM_LMX_TYPEDEF_DEFINED_LmMessageRoute 1
#define LM_LMX_TYPEDEF_ID_A_LmMessageRoute 0x590ea0a7fef33476ULL
#define LM_LMX_TYPEDEF_ID_B_LmMessageRoute 0x5917a7ba09ea1f87ULL
typedef struct LmMessageRoute LmMessageRoute;
#else
#if !defined(LM_LMX_TYPEDEF_ID_A_LmMessageRoute) || !defined(LM_LMX_TYPEDEF_ID_B_LmMessageRoute) || LM_LMX_TYPEDEF_ID_A_LmMessageRoute != 0x590ea0a7fef33476ULL || LM_LMX_TYPEDEF_ID_B_LmMessageRoute != 0x5917a7ba09ea1f87ULL
#error "Lingvamyxa conflicting typedef projection for LmMessageRoute"
#endif
#endif
#ifndef LM_LMX_TYPEDEF_DEFINED_LmMessageThreadRuntime
#define LM_LMX_TYPEDEF_DEFINED_LmMessageThreadRuntime 1
#define LM_LMX_TYPEDEF_ID_A_LmMessageThreadRuntime 0xbcdbce07d745c668ULL
#define LM_LMX_TYPEDEF_ID_B_LmMessageThreadRuntime 0x5267d0b311015cb1ULL
typedef struct LmMessageThreadRuntime LmMessageThreadRuntime;
#else
#if !defined(LM_LMX_TYPEDEF_ID_A_LmMessageThreadRuntime) || !defined(LM_LMX_TYPEDEF_ID_B_LmMessageThreadRuntime) || LM_LMX_TYPEDEF_ID_A_LmMessageThreadRuntime != 0xbcdbce07d745c668ULL || LM_LMX_TYPEDEF_ID_B_LmMessageThreadRuntime != 0x5267d0b311015cb1ULL
#error "Lingvamyxa conflicting typedef projection for LmMessageThreadRuntime"
#endif
#endif
#ifndef LM_LMX_TYPEDEF_DEFINED_LmRestLmxProviderOpsV1
#define LM_LMX_TYPEDEF_DEFINED_LmRestLmxProviderOpsV1 1
#define LM_LMX_TYPEDEF_ID_A_LmRestLmxProviderOpsV1 0xaadc7c363d31231cULL
#define LM_LMX_TYPEDEF_ID_B_LmRestLmxProviderOpsV1 0x0e805817ebcbb71dULL
typedef struct LmRestLmxProviderOpsV1 LmRestLmxProviderOpsV1;
#else
#if !defined(LM_LMX_TYPEDEF_ID_A_LmRestLmxProviderOpsV1) || !defined(LM_LMX_TYPEDEF_ID_B_LmRestLmxProviderOpsV1) || LM_LMX_TYPEDEF_ID_A_LmRestLmxProviderOpsV1 != 0xaadc7c363d31231cULL || LM_LMX_TYPEDEF_ID_B_LmRestLmxProviderOpsV1 != 0x0e805817ebcbb71dULL
#error "Lingvamyxa conflicting typedef projection for LmRestLmxProviderOpsV1"
#endif
#endif
#ifndef LM_LMX_TYPEDEF_DEFINED_LmMessageThreadPool
#define LM_LMX_TYPEDEF_DEFINED_LmMessageThreadPool 1
#define LM_LMX_TYPEDEF_ID_A_LmMessageThreadPool 0xd41eb629c50c1a70ULL
#define LM_LMX_TYPEDEF_ID_B_LmMessageThreadPool 0xe036b7fbdebf6a29ULL
typedef struct LmMessageThreadPool LmMessageThreadPool;
#else
#if !defined(LM_LMX_TYPEDEF_ID_A_LmMessageThreadPool) || !defined(LM_LMX_TYPEDEF_ID_B_LmMessageThreadPool) || LM_LMX_TYPEDEF_ID_A_LmMessageThreadPool != 0xd41eb629c50c1a70ULL || LM_LMX_TYPEDEF_ID_B_LmMessageThreadPool != 0xe036b7fbdebf6a29ULL
#error "Lingvamyxa conflicting typedef projection for LmMessageThreadPool"
#endif
#endif
#ifndef LM_LMX_TYPEDEF_DEFINED_LmMessageThreadComponent
#define LM_LMX_TYPEDEF_DEFINED_LmMessageThreadComponent 1
#define LM_LMX_TYPEDEF_ID_A_LmMessageThreadComponent 0x9adbfcfc5ede09deULL
#define LM_LMX_TYPEDEF_ID_B_LmMessageThreadComponent 0x55c1ece857e1fb9fULL
typedef struct LmMessageThreadComponent LmMessageThreadComponent;
#else
#if !defined(LM_LMX_TYPEDEF_ID_A_LmMessageThreadComponent) || !defined(LM_LMX_TYPEDEF_ID_B_LmMessageThreadComponent) || LM_LMX_TYPEDEF_ID_A_LmMessageThreadComponent != 0x9adbfcfc5ede09deULL || LM_LMX_TYPEDEF_ID_B_LmMessageThreadComponent != 0x55c1ece857e1fb9fULL
#error "Lingvamyxa conflicting typedef projection for LmMessageThreadComponent"
#endif
#endif
#ifndef LM_LMX_TYPEDEF_DEFINED_LmMessageThread
#define LM_LMX_TYPEDEF_DEFINED_LmMessageThread 1
#define LM_LMX_TYPEDEF_ID_A_LmMessageThread 0xb24ebdc4cbfa0b14ULL
#define LM_LMX_TYPEDEF_ID_B_LmMessageThread 0x198bda4c3705b0e5ULL
typedef struct LmMessageThread LmMessageThread;
#else
#if !defined(LM_LMX_TYPEDEF_ID_A_LmMessageThread) || !defined(LM_LMX_TYPEDEF_ID_B_LmMessageThread) || LM_LMX_TYPEDEF_ID_A_LmMessageThread != 0xb24ebdc4cbfa0b14ULL || LM_LMX_TYPEDEF_ID_B_LmMessageThread != 0x198bda4c3705b0e5ULL
#error "Lingvamyxa conflicting typedef projection for LmMessageThread"
#endif
#endif
#ifndef LM_LMX_TYPEDEF_DEFINED_LmRestLmxHttpServer
#define LM_LMX_TYPEDEF_DEFINED_LmRestLmxHttpServer 1
#define LM_LMX_TYPEDEF_ID_A_LmRestLmxHttpServer 0xc93320eec144d348ULL
#define LM_LMX_TYPEDEF_ID_B_LmRestLmxHttpServer 0x25d8028910651f29ULL
typedef struct LmRestLmxHttpServer LmRestLmxHttpServer;
#else
#if !defined(LM_LMX_TYPEDEF_ID_A_LmRestLmxHttpServer) || !defined(LM_LMX_TYPEDEF_ID_B_LmRestLmxHttpServer) || LM_LMX_TYPEDEF_ID_A_LmRestLmxHttpServer != 0xc93320eec144d348ULL || LM_LMX_TYPEDEF_ID_B_LmRestLmxHttpServer != 0x25d8028910651f29ULL
#error "Lingvamyxa conflicting typedef projection for LmRestLmxHttpServer"
#endif
#endif


#define LM_OWN_EDGE_BORROWED 1
#define LM_OWN_EDGE_OWNED 2
#define LM_OWN_EDGE_LAZY_OWNED 3
#define LM_OWN_EDGE_EXTERNAL 4
#define LM_MESSAGE_THREAD_NEW 0
#define LM_MESSAGE_THREAD_RUNNING 1
#define LM_MESSAGE_THREAD_STOPPING 2
#define LM_MESSAGE_THREAD_STOPPED 3
#define LM_MESSAGE_STATUS_ROUTE_NOT_FOUND 64
#define LM_MESSAGE_STATUS_TRANSPORT_PROVIDER_NOT_CONFIGURED 65
#define LM_MESSAGE_STATUS_INVALID_ADDRESS 66
#define LM_MESSAGE_STATUS_TRANSPORT_FAILED 67
#define LM_MESSAGE_STATUS_HTTP_REJECTED 68
#define LM_MESSAGE_STATUS_TRANSPORT_PROTOCOL_ERROR 69
#define LM_MESSAGE_STATUS_APPLICATION_STOPPING 70


#include <stddef.h>

#ifndef LM_LMX_TYPEDEF_DEFINED_LmSlice
#define LM_LMX_TYPEDEF_DEFINED_LmSlice 1
#define LM_LMX_TYPEDEF_ID_A_LmSlice 0xdd6442dffff43f92ULL
#define LM_LMX_TYPEDEF_ID_B_LmSlice 0x21d9c62537977663ULL
typedef struct LmSlice {
    void *ptr;
    size_t length;
} LmSlice;
#else
#if !defined(LM_LMX_TYPEDEF_ID_A_LmSlice) || !defined(LM_LMX_TYPEDEF_ID_B_LmSlice) || LM_LMX_TYPEDEF_ID_A_LmSlice != 0xdd6442dffff43f92ULL || LM_LMX_TYPEDEF_ID_B_LmSlice != 0x21d9c62537977663ULL
#error "Lingvamyxa conflicting typedef projection for LmSlice"
#endif
#endif
struct LmOwnPtrStack {
    void **items;
    size_t count;
    size_t capacity;
    void (*delete_item)(void *object);
};
struct LmOwnValueStack {
    void *items;
    size_t count;
    size_t capacity;
    size_t item_size;
};
struct LmOwnAllocationDescriptor {
    void *address;
    LmOwnArena * owner;
    size_t bytes;
    size_t element_size;
    size_t count;
    size_t rank;
    size_t level;
};
struct LmOwnLazyEdge {
    LmOwnEdgeKind kind;
    LmOwnArena * source_owner;
    LmOwnArena * target_owner;
    const void *source;
    size_t size;
    const void **patch_slot;
};
struct LmOwnArena {
    LmOwnPtrStack * allocations;
    LmOwnPtrStack * allocation_descriptors;
    LmOwnPtrStack * lazy_edges;
    int frozen;
    LmMessageThread * owner_thread;
    LmOwnArena * registry_previous;
    LmOwnArena * registry_next;
    int runtime_owned_shell;
};
struct LmHostThread {
    void *implementation;
    int started;
    int joined;
    void *controller_identity;
    int starting;
};
struct LmMutex {
    void *implementation;
};
struct LmCondition {
    void *implementation;
};
struct LmMessage {
    char *lmx;
    size_t length;
    LmMessage * next;
};
struct LmMessageOutboxEntry {
    char *endpoint;
    char *route;
    LmMessage * message;
    LmMessageOutboxEntry * next;
};
struct LmMessageRoute {
    char *route;
    LmMessageThread * target;
    LmMessageRoute * next;
};
struct LmMessageThreadRuntime {
    void *identity;
    size_t pool_count;
    size_t single_execution_depth;
    LmMessageThread * single_active_thread;
    LmMutex * route_mutex;
    LmMessageRoute * route_head;
    size_t route_count;
    LmMessageThread * root_thread;
    LmMessageThread * exit_requester;
    int exit_requested;
    int exit_ready;
    int exit_status;
    int (*rest_lmx_post)(void *context, const char *normalized_uri, const char *body, size_t length, unsigned *out_http_status);
    void (*rest_lmx_destroy)(void *context);
    void *rest_lmx_context;
    int rest_lmx_provider_sealed;
    int rest_lmx_provider_transition;
    int deleting;
};
struct LmRestLmxProviderOpsV1 {
    size_t abi_size;
    int (*post)(void *context, const char *normalized_uri, const char *body, size_t length, unsigned *out_http_status);
    void (*destroy)(void *context);
};
struct LmMessageThreadPool {
    LmHostThread * *workers;
    size_t worker_count;
    size_t started_worker_count;
    LmMutex * mutex;
    LmCondition * work_ready;
    LmMessageThread * ready_head;
    LmMessageThread * ready_tail;
    LmMessageThread * member_head;
    LmMessageThread * member_tail;
    size_t member_count;
    int stop_requested;
    int drain_requested;
    int deleting;
    int single_mode;
    LmMessageThreadRuntime * runtime;
};
struct LmMessageThreadComponent {
    LmMessageThread * owner_thread;
    void *value;
    void (*destroy)(struct LmMessageThread *lm_lmx_message_thread, void *component);
    LmMessageThreadComponent * next;
};
struct LmMessageThread {
    LmOwnArena * root_owner;
    LmMessageThreadState state;
    int stop_status;
    size_t turn_count;
    size_t collection_count;
    int collector_failed;
    void *execution_context;
    LmOwnArena * arena_head;
    LmOwnArena * arena_tail;
    size_t arena_count;
    int arena_destroying;
    LmMessageThreadComponent * component_head;
    size_t component_count;
    int component_destroying;
    LmMessageThreadPool * pool;
    LmMessageThreadRuntime * runtime;
    void (*entry)(struct LmMessageThread *lm_lmx_message_thread, void *argument);
    void *entry_argument;
    LmMutex * state_mutex;
    LmCondition * stopped_condition;
    int started;
    int joining;
    int joined;
    int scheduled;
    int executing;
    void *execution_identity;
    LmMessageThread * ready_next;
    LmMessageThread * pool_previous;
    LmMessageThread * pool_next;
    int mailbox_mode;
    LmMessage * inbox_head;
    LmMessage * inbox_tail;
    size_t inbox_count;
    LmMessageOutboxEntry * outbox_head;
    LmMessageOutboxEntry * outbox_tail;
    size_t outbox_count;
    LmMessage * current_message;
    int turn_active;
    int turn_failed;
};
struct LmRestLmxHttpServer {
    LmMessageThreadRuntime * runtime;
    void *context;
    char *base_path;
    size_t base_path_length;
    size_t max_body_bytes;
    unsigned listening_port;
    int library_initialized;
};
#ifndef LM_LMX_TYPEDEF_DEFINED_LmRestLmxHttpServerOptionsV1
#define LM_LMX_TYPEDEF_DEFINED_LmRestLmxHttpServerOptionsV1 1
#define LM_LMX_TYPEDEF_ID_A_LmRestLmxHttpServerOptionsV1 0x5bef13d7496720b5ULL
#define LM_LMX_TYPEDEF_ID_B_LmRestLmxHttpServerOptionsV1 0x86b15cac0fea922eULL
typedef struct LmRestLmxHttpServerOptionsV1 {
    size_t abi_size;
    const char *bind_address;
    unsigned port;
    const char *base_path;
    size_t max_body_bytes;
    unsigned worker_count;
    unsigned request_timeout_ms;
} LmRestLmxHttpServerOptionsV1;
#else
#if !defined(LM_LMX_TYPEDEF_ID_A_LmRestLmxHttpServerOptionsV1) || !defined(LM_LMX_TYPEDEF_ID_B_LmRestLmxHttpServerOptionsV1) || LM_LMX_TYPEDEF_ID_A_LmRestLmxHttpServerOptionsV1 != 0x5bef13d7496720b5ULL || LM_LMX_TYPEDEF_ID_B_LmRestLmxHttpServerOptionsV1 != 0x86b15cac0fea922eULL
#error "Lingvamyxa conflicting typedef projection for LmRestLmxHttpServerOptionsV1"
#endif
#endif




void * (lm_own_new_zero)(size_t size);
void * (lm_own_resize)(void *object, size_t size);
char * (lm_own_copy_bytes)(const char *source, size_t length);
void (lm_own_delete)(void *object, LmOwnDestroyFields destroy_fields);
void (lm_own_delete_plain)(void *object);
void (lm_own_pointer_array_delete)(void **items, size_t count, LmOwnDelete delete_item);
void (lm_own_ptr_stack_init)(LmOwnPtrStack *stack, LmOwnDelete delete_item);
void (lm_own_ptr_stack_destroy)(LmOwnPtrStack *stack);
int (lm_own_ptr_stack_push)(LmOwnPtrStack *stack, void *item);
void * (lm_own_ptr_stack_pop)(LmOwnPtrStack *stack);
void * (lm_own_ptr_stack_at)(const LmOwnPtrStack *stack, size_t index);
void * (lm_own_ptr_stack_top)(const LmOwnPtrStack *stack);
void (lm_own_ptr_stack_truncate)(LmOwnPtrStack *stack, size_t count);
void (lm_own_value_stack_init)(LmOwnValueStack *stack, size_t item_size);
void (lm_own_value_stack_destroy)(LmOwnValueStack *stack);
int (lm_own_value_stack_push)(LmOwnValueStack *stack, const void *item);
int (lm_own_value_stack_resize_zero)(LmOwnValueStack *stack, size_t count);
int (lm_own_value_stack_pop)(LmOwnValueStack *stack, void *out_item);
void * (lm_own_value_stack_at)(const LmOwnValueStack *stack, size_t index);
void * (lm_own_value_stack_top)(const LmOwnValueStack *stack);
void (lm_own_value_stack_truncate)(LmOwnValueStack *stack, size_t count);
LmOwnArena * (lm_own_arena_new)(struct LmMessageThread *lm_lmx_message_thread, LmMessageThread *owner_thread);
void (lm_own_arena_delete)(struct LmMessageThread *lm_lmx_message_thread, LmOwnArena *arena);
int (lm_own_arena_init)(struct LmMessageThread *lm_lmx_message_thread, LmOwnArena *arena, LmMessageThread *owner_thread);
void (lm_own_arena_destroy)(struct LmMessageThread *lm_lmx_message_thread, LmOwnArena *arena);
void * (lm_own_arena_new_zero)(struct LmMessageThread *lm_lmx_message_thread, LmOwnArena *arena, size_t size);
void * (lm_own_arena_array_new_zero)(struct LmMessageThread *lm_lmx_message_thread, LmOwnArena *arena, size_t element_size, size_t count, size_t rank, size_t level);
const LmOwnAllocationDescriptor * (lm_own_arena_allocation_descriptor)(struct LmMessageThread *lm_lmx_message_thread, const LmOwnArena *arena, const void *address);
char * (lm_own_arena_copy_bytes)(struct LmMessageThread *lm_lmx_message_thread, LmOwnArena *arena, const char *source, size_t length);
int (lm_own_arena_add_lazy_edge)(struct LmMessageThread *lm_lmx_message_thread, LmOwnArena *target, LmOwnArena *source, const void *source_ptr, size_t size, const void **patch_slot);
int (lm_own_arena_promote_lazy_edges)(struct LmMessageThread *lm_lmx_message_thread, LmOwnArena *arena);
int (lm_own_arena_absorb)(struct LmMessageThread *lm_lmx_message_thread, LmOwnArena *target, LmOwnArena *source);
void (lm_own_arena_freeze)(struct LmMessageThread *lm_lmx_message_thread, LmOwnArena *arena);
int (lm_own_arena_is_frozen)(struct LmMessageThread *lm_lmx_message_thread, const LmOwnArena *arena);
LmMessageThread * (lm_own_arena_owner_thread)(struct LmMessageThread *lm_lmx_message_thread, const LmOwnArena *arena);
int (lm_own_tree_cut)(struct LmMessageThread *lm_lmx_message_thread, LmOwnArena *arena);
int (lm_own_tree_cut_promote_lazy_edges)(struct LmMessageThread *lm_lmx_message_thread, LmOwnArena *arena);
const char * (lm_thread_provider_name)(void);
LmHostThread * (lm_host_thread_new)(void);
void (lm_host_thread_delete)(LmHostThread *thread);
int (lm_host_thread_start)(LmHostThread *thread, LmHostThreadEntry entry, void *argument);
int (lm_host_thread_join)(LmHostThread *thread, void **result);
LmMutex * (lm_mutex_new)(void);
void (lm_mutex_delete)(LmMutex *mutex);
int (lm_mutex_lock)(LmMutex *mutex);
int (lm_mutex_unlock)(LmMutex *mutex);
LmCondition * (lm_condition_new)(void);
void (lm_condition_delete)(LmCondition *condition);
int (lm_condition_wait)(LmCondition *condition, LmMutex *mutex);
int (lm_condition_signal)(LmCondition *condition);
int (lm_condition_broadcast)(LmCondition *condition);
LmMessageThreadRuntime * (lm_message_thread_runtime_new)(void);
int (lm_message_thread_runtime_delete)(LmMessageThreadRuntime *runtime);
int (lm_message_thread_runtime_set_rest_lmx_provider)(LmMessageThreadRuntime *runtime, const LmRestLmxProviderOpsV1 *ops, void *context);
int (lm_message_thread_runtime_admit_lmx)(LmMessageThreadRuntime *runtime, const char *route, const char *lmx, size_t length);
int (lm_message_thread_runtime_attach_root)(LmMessageThreadRuntime *runtime, LmMessageThread *thread);
int (lm_message_thread_runtime_detach_root)(LmMessageThreadRuntime *runtime, LmMessageThread *thread);
int (lm_message_thread_runtime_exit_state)(LmMessageThreadRuntime *runtime, int *out_requested, int *out_ready, int *out_status);
LmMessageThreadPool * (lm_message_thread_pool_new)(LmMessageThreadRuntime *runtime, size_t worker_count);
void (lm_message_thread_pool_request_stop)(LmMessageThreadPool *pool);
void (lm_message_thread_pool_request_stop_when_idle)(LmMessageThreadPool *pool);
size_t (lm_message_thread_pool_pump)(LmMessageThreadPool *pool, size_t max_turns);
int (lm_message_thread_pool_delete)(LmMessageThreadPool *pool);
int (lm_message_thread_init)(LmMessageThread *thread);
void (lm_message_thread_destroy)(LmMessageThread *thread);
LmMessageThread * (lm_message_thread_new)(void);
LmMessageThread * (lm_message_thread_new_in)(LmMessageThreadPool *pool);
void (lm_message_thread_delete)(LmMessageThread *thread);
int (lm_message_thread_start)(LmMessageThread *thread, LmMessageThreadEntry entry, void *argument);
int (lm_message_thread_start_mailbox)(LmMessageThread *thread, LmMessageThreadEntry entry, void *argument);
int (lm_message_thread_join)(LmMessageThread *thread, int *result);
int (lm_message_thread_bind_route)(LmMessageThread *thread, const char *route);
int (lm_message_thread_send_lmx)(LmMessageThread *sender, const char *endpoint, const char *route, const char *lmx, size_t length);
int (lm_message_thread_current_lmx)(LmMessageThread *thread, const char **out_lmx, size_t *out_length);
size_t (lm_message_thread_inbox_count)(const LmMessageThread *thread);
size_t (lm_message_thread_outbox_count)(const LmMessageThread *thread);
int (lm_message_thread_begin_turn)(LmMessageThread *thread);
int (lm_message_thread_end_turn)(LmMessageThread *thread);
int (lm_message_thread_collect)(LmMessageThread *thread);
void (lm_message_thread_request_stop)(LmMessageThread *thread, int status);
void (lm_message_thread_request_failure)(LmMessageThread *thread, int status);
int (lm_message_thread_request_exit)(LmMessageThread *requester, int status);
int (lm_message_thread_is_running)(const LmMessageThread *thread);
int (lm_message_thread_status)(const LmMessageThread *thread);
LmOwnArena * (lm_message_thread_owner)(LmMessageThread *thread);
void * (lm_message_thread_execution_context)(LmMessageThread *thread);
void * (lm_message_thread_set_execution_context)(LmMessageThread *thread, void *context);
size_t (lm_message_thread_turn_count)(const LmMessageThread *thread);
size_t (lm_message_thread_collection_count)(const LmMessageThread *thread);
size_t (lm_message_thread_arena_count)(const LmMessageThread *thread);
int (lm_message_thread_component_attach)(struct LmMessageThread *lm_lmx_message_thread, LmMessageThreadComponentDestroy destroy, void *component);
void * (lm_message_thread_component_get)(struct LmMessageThread *lm_lmx_message_thread, LmMessageThreadComponentDestroy destroy);
int (lm_message_thread_component_remove)(struct LmMessageThread *lm_lmx_message_thread, LmMessageThreadComponentDestroy destroy);
size_t (lm_message_thread_component_count)(const LmMessageThread *thread);
const char * (lm_rest_lmx_http_server_provider_name)(void);
int (lm_rest_lmx_http_server_start)(LmMessageThreadRuntime *runtime, const LmRestLmxHttpServerOptionsV1 *options, LmRestLmxHttpServer **out_server);
int (lm_rest_lmx_http_server_start_default)(LmMessageThreadRuntime *runtime, LmRestLmxHttpServer **out_server);
int (lm_rest_lmx_http_server_stop)(LmRestLmxHttpServer **server);
int (lm_rest_lmx_http_server_port)(const LmRestLmxHttpServer *server, unsigned *out_port);


#if LM_REST_LMX_SERVER_PROVIDER != LM_REST_LMX_SERVER_PROVIDER_NONE && LM_REST_LMX_SERVER_PROVIDER != LM_REST_LMX_SERVER_PROVIDER_CIVETWEB
static char lm_rest_lmx_http_server_unknown_provider[-1];
#endif

#if LM_REST_LMX_SERVER_PROVIDER == LM_REST_LMX_SERVER_PROVIDER_CIVETWEB
#include <civetweb.h>
static int lm_rest_lmx_http_ascii_is_hex(unsigned char value);
static unsigned char lm_rest_lmx_http_ascii_lower(unsigned char value);
static int lm_rest_lmx_http_ascii_equal(const char *left, const char *right);
static int lm_rest_lmx_http_header_value_is(const char *value, const char *expected);
static int lm_rest_lmx_http_parse_size(const char *text, size_t *out_value);
static int lm_rest_lmx_http_parse_unsigned(const char *text, unsigned *out_value);
static int lm_rest_lmx_http_bind_address_is_valid(const char *address);
static int lm_rest_lmx_http_base_path_is_valid(const char *path);
static char * lm_rest_lmx_http_copy_text(const char *text);
static int lm_rest_lmx_http_send_status(struct mg_connection *connection, unsigned status);
static unsigned lm_rest_lmx_http_admission_status(int status);
static const char * lm_rest_lmx_http_route(const LmRestLmxHttpServer *server, const char *raw_uri, unsigned *out_status);
static int lm_rest_lmx_http_begin_request(struct mg_connection *connection);
static int lm_rest_lmx_http_server_options_are_valid(const LmRestLmxHttpServerOptionsV1 *options);
const char * lm_rest_lmx_http_server_provider_name(void);
int lm_rest_lmx_http_server_start(LmMessageThreadRuntime *runtime, const LmRestLmxHttpServerOptionsV1 *options, LmRestLmxHttpServer **out_server);
int lm_rest_lmx_http_server_start_default(LmMessageThreadRuntime *runtime, LmRestLmxHttpServer **out_server);
int lm_rest_lmx_http_server_stop(LmRestLmxHttpServer **server);
int lm_rest_lmx_http_server_port(const LmRestLmxHttpServer *server, unsigned *out_port);

static int lm_rest_lmx_http_ascii_is_hex(unsigned char value) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    return (value >= (((unsigned char)'0')) && value <= (((unsigned char)'9'))) || (value >= (((unsigned char)'a')) && value <= (((unsigned char)'f'))) || (value >= (((unsigned char)'A')) && value <= (((unsigned char)'F')));
}

static unsigned char lm_rest_lmx_http_ascii_lower(unsigned char value) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    if (value >= (((unsigned char)'A')) && value <= (((unsigned char)'Z'))) {
        return (((unsigned char)(value + ((((unsigned char)'a')) - (((unsigned char)'A'))))));
    }
    return value;
}

static int lm_rest_lmx_http_ascii_equal(const char *left, const char *right) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    size_t index = 0U;
    if (left == 0 || right == 0) {
        return 0;
    }
    while (left[index] != '\0' && right[index] != '\0') {
        if (lm_rest_lmx_http_ascii_lower(((unsigned char)left[index])) != lm_rest_lmx_http_ascii_lower(((unsigned char)right[index]))) {
            return 0;
        }
        index = index + 1U;
    }
    return left[index] == '\0' && right[index] == '\0';
}

static int lm_rest_lmx_http_header_value_is(const char *value, const char *expected) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    const unsigned char *begin = (((const unsigned char *)value));
    const unsigned char *end;
    size_t expected_length;
    size_t index;
    if (value == 0 || expected == 0) {
        return 0;
    }
    while (begin[0] == (((unsigned char)' ')) || begin[0] == (((unsigned char)'\t'))) {
        begin = begin + 1;
    }
    end = begin + strlen(((const char *)begin));
    while (end > begin && (end[-1] == (((unsigned char)' ')) || end[-1] == (((unsigned char)'\t')))) {
        end = end - 1;
    }
    expected_length = strlen(expected);
    if ((((size_t)(end - begin))) != expected_length) {
        return 0;
    }
    index = 0U;
    while (index < expected_length) {
        if (lm_rest_lmx_http_ascii_lower(begin[index]) != lm_rest_lmx_http_ascii_lower(((unsigned char)expected[index]))) {
            return 0;
        }
        index = index + 1U;
    }
    return 1;
}

static int lm_rest_lmx_http_parse_size(const char *text, size_t *out_value) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    const unsigned char *begin = (((const unsigned char *)text));
    const unsigned char *end;
    size_t value = 0U;
    if (text == 0 || out_value == 0) {
        return 1;
    }
    while (begin[0] == (((unsigned char)' ')) || begin[0] == (((unsigned char)'\t'))) {
        begin = begin + 1;
    }
    end = begin + strlen(((const char *)begin));
    while (end > begin && (end[-1] == (((unsigned char)' ')) || end[-1] == (((unsigned char)'\t')))) {
        end = end - 1;
    }
    if (begin == end) {
        return 1;
    }
    while (begin < end) {
        unsigned digit;
        if (begin[0] < (((unsigned char)'0')) || begin[0] > (((unsigned char)'9'))) {
            return 1;
        }
        digit = (((unsigned)(begin[0] - (((unsigned char)'0')))));
        if (value > (SIZE_MAX - (((size_t)digit))) / 10U) {
            return 2;
        }
        value = value * 10U + (((size_t)digit));
        begin = begin + 1;
    }
    out_value[0] = value;
    return 0;
}

static int lm_rest_lmx_http_parse_unsigned(const char *text, unsigned *out_value) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    size_t value = 0U;
    int status;
    if (out_value == 0) {
        return 1;
    }
    status = lm_rest_lmx_http_parse_size(text, &value);
    if (status != 0 || value > (((size_t)UINT_MAX))) {
        return 1;
    }
    out_value[0] = (((unsigned)value));
    return 0;
}

static int lm_rest_lmx_http_bind_address_is_valid(const char *address) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    const unsigned char *text = (((const unsigned char *)address));
    size_t index;
    size_t length;
    if (text == 0 || text[0] == 0U) {
        return 0;
    }
    length = strlen(address);
    if (length > 255U) {
        return 0;
    }
    index = 0U;
    while (index < length) {
        unsigned char value = text[index];
        if (((value >= (((unsigned char)'a')) && value <= (((unsigned char)'z'))) || (value >= (((unsigned char)'A')) && value <= (((unsigned char)'Z'))) || (value >= (((unsigned char)'0')) && value <= (((unsigned char)'9'))) || value == (((unsigned char)'.')) || value == (((unsigned char)'-')) || value == (((unsigned char)':')) || value == (((unsigned char)'[')) || value == (((unsigned char)']'))) == 0) {
            return 0;
        }
        index = index + 1U;
    }
    return 1;
}

static int lm_rest_lmx_http_base_path_is_valid(const char *path) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    const unsigned char *text = (((const unsigned char *)path));
    size_t length;
    size_t index;
    size_t segment_start;
    if (text == 0) {
        return 0;
    }
    length = strlen(path);
    if (length == 0U || (length == 1U && text[0] == (((unsigned char)'/')))) {
        return 1;
    }
    if (text[0] != (((unsigned char)'/')) || text[length - 1U] == (((unsigned char)'/'))) {
        return 0;
    }
    segment_start = 1U;
    index = 1U;
    while (index < length) {
        unsigned char value = text[index];
        if (value <= 0x20U || value > 0x7eU || value == (((unsigned char)'\\')) || value == (((unsigned char)'?')) || value == (((unsigned char)'#'))) {
            return 0;
        }
        if (value == (((unsigned char)'/'))) {
            size_t segment_length = index - segment_start;
            if (segment_length == 0U || (segment_length == 1U && text[segment_start] == (((unsigned char)'.'))) || (segment_length == 2U && text[segment_start] == (((unsigned char)'.')) && text[segment_start + 1U] == (((unsigned char)'.')))) {
                return 0;
            }
            segment_start = index + 1U;
        }
        else {
            if (value == (((unsigned char)'%'))) {
                if (index + 2U >= length || lm_rest_lmx_http_ascii_is_hex(text[index + 1U]) == 0 || lm_rest_lmx_http_ascii_is_hex(text[index + 2U]) == 0) {
                    return 0;
                }
                index = index + 2U;
            }
        }
        index = index + 1U;
    }
    if ((length - segment_start == 1U && text[segment_start] == (((unsigned char)'.'))) || (length - segment_start == 2U && text[segment_start] == (((unsigned char)'.')) && text[segment_start + 1U] == (((unsigned char)'.')))) {
        return 0;
    }
    return 1;
}

static char * lm_rest_lmx_http_copy_text(const char *text) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    size_t length;
    char *copy;
    if (text == 0) {
        return 0;
    }
    length = strlen(text);
    copy = (((char *)malloc(length + 1U)));
    if (copy == 0) {
        return 0;
    }
    memcpy(copy, text, length + 1U);
    return copy;
}

static int lm_rest_lmx_http_send_status(struct mg_connection *connection, unsigned status) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    if (status == 202U) {
        mg_printf(connection, "HTTP/1.1 202 Accepted\r\nContent-Length: 0\r\nConnection: close\r\n\r\n");
        return 202;
    }
    if (status != 400U && status != 404U && status != 405U && status != 411U && status != 413U && status != 415U && status != 503U) {
        status = 500U;
    }
    mg_send_http_error(connection, (((int)status)), "%s", "");
    return (((int)status));
}

static unsigned lm_rest_lmx_http_admission_status(int status) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
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

static const char * lm_rest_lmx_http_route(const LmRestLmxHttpServer *server, const char *raw_uri, unsigned *out_status) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    size_t raw_length;
    if (out_status != 0) {
        out_status[0] = 400U;
    }
    if (server == 0 || raw_uri == 0 || out_status == 0) {
        return 0;
    }
    if (server -> base_path_length == 0U) {
        return raw_uri;
    }
    raw_length = strlen(raw_uri);
    if (raw_length <= server -> base_path_length || memcmp(raw_uri, server -> base_path, server -> base_path_length) != 0 || raw_uri[server -> base_path_length] != '/') {
        out_status[0] = 404U;
        return 0;
    }
    return raw_uri + server -> base_path_length;
}

static int lm_rest_lmx_http_begin_request(struct mg_connection *connection) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    const LmCivetwebRequestInfo * request = 0;
    LmRestLmxHttpServer * server = 0;
    const char *content_length_text = 0;
    const char *content_type_text = 0;
    const char *route = 0;
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
    size_t allocation_size;
    if (connection == 0) {
        return 500;
    }
    request = mg_get_request_info(connection);
    if (request == 0 || request->user_data == 0) {
        return lm_rest_lmx_http_send_status(connection, 500U);
    }
    server = (((LmRestLmxHttpServer *)request->user_data));
    if (lm_rest_lmx_http_ascii_equal(request->request_method, "POST") == 0) {
        return lm_rest_lmx_http_send_status(connection, 405U);
    }
    if (request->query_string != 0) {
        return lm_rest_lmx_http_send_status(connection, 400U);
    }
    route = lm_rest_lmx_http_route(server, request->local_uri_raw, &route_status);
    if (route == 0) {
        return lm_rest_lmx_http_send_status(connection, route_status);
    }
    index = 0;
    while (index < request->num_headers) {
        const char *name = request->http_headers[index].name;
        const char *value = request->http_headers[index].value;
        if (lm_rest_lmx_http_ascii_equal(name, "Content-Length")) {
            content_length_count = content_length_count + 1;
            content_length_text = value;
        }
        else {
            if (lm_rest_lmx_http_ascii_equal(name, "Transfer-Encoding")) {
                transfer_encoding_count = transfer_encoding_count + 1;
            }
            else {
                if (lm_rest_lmx_http_ascii_equal(name, "Content-Type")) {
                    content_type_count = content_type_count + 1;
                    content_type_text = value;
                }
            }
        }
        index = index + 1;
    }
    if (transfer_encoding_count != 0 || content_length_count == 0) {
        return lm_rest_lmx_http_send_status(connection, 411U);
    }
    if (content_length_count != 1) {
        return lm_rest_lmx_http_send_status(connection, 400U);
    }
    if (content_type_count != 1 || lm_rest_lmx_http_header_value_is(content_type_text, "application/lmx") == 0) {
        return lm_rest_lmx_http_send_status(connection, 415U);
    }
    parse_status = lm_rest_lmx_http_parse_size(content_length_text, &content_length);
    if (parse_status == 2) {
        return lm_rest_lmx_http_send_status(connection, 413U);
    }
    if (parse_status != 0) {
        return lm_rest_lmx_http_send_status(connection, 400U);
    }
    if (content_length > server -> max_body_bytes) {
        return lm_rest_lmx_http_send_status(connection, 413U);
    }
    allocation_size = content_length;
    if (allocation_size == 0U) {
        allocation_size = 1U;
    }
    body = (((char *)malloc(allocation_size)));
    if (body == 0) {
        return lm_rest_lmx_http_send_status(connection, 500U);
    }
    while (received < content_length) {
        size_t remaining = content_length - received;
        size_t chunk = remaining;
        int count;
        if (chunk > (((size_t)INT_MAX))) {
            chunk = (((size_t)INT_MAX));
        }
        count = mg_read(connection, body + received, chunk);
        if (count <= 0 || (((size_t)count)) > remaining) {
            free(body);
            return lm_rest_lmx_http_send_status(connection, 400U);
        }
        received = received + (((size_t)count));
    }
    admission_status = lm_message_thread_runtime_admit_lmx(server -> runtime, route, body, content_length);
    free(body);
    response_status = lm_rest_lmx_http_admission_status(admission_status);
    return lm_rest_lmx_http_send_status(connection, response_status);
}

static int lm_rest_lmx_http_server_options_are_valid(const LmRestLmxHttpServerOptionsV1 *options) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    return options != 0 && options -> abi_size >= sizeof(options[0]) && lm_rest_lmx_http_bind_address_is_valid(options -> bind_address) && options -> port <= 65535U && lm_rest_lmx_http_base_path_is_valid(options -> base_path) && options -> max_body_bytes != 0U && options -> worker_count != 0U && options -> worker_count <= 1024U && options -> request_timeout_ms != 0U;
}

const char * lm_rest_lmx_http_server_provider_name(void) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    return "civetweb";
}

int lm_rest_lmx_http_server_start(LmMessageThreadRuntime *runtime, const LmRestLmxHttpServerOptionsV1 *options, LmRestLmxHttpServer **out_server) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    LmRestLmxHttpServer * server = 0;
    struct mg_callbacks callbacks;
    struct mg_server_port ports[4];
    char listening_ports[320];
    char worker_count[32];
    char request_timeout[32];
    const char *configuration[13];
    const char *base_path;
    int port_count;
    int written;
    if (out_server == 0) {
        return 1;
    }
    out_server[0] = 0;
    if (runtime == 0 || lm_rest_lmx_http_server_options_are_valid(options) == 0) {
        return 1;
    }
    base_path = options -> base_path;
    if (strcmp(base_path, "/") == 0) {
        base_path = "";
    }
    server = (((LmRestLmxHttpServer *)calloc(1U, sizeof(server[0]))));
    if (server == 0) {
        return 1;
    }
    server->runtime = runtime;
    server->base_path = lm_rest_lmx_http_copy_text(base_path);
    if (server -> base_path == 0) {
        free(server);
        return 1;
    }
    server->base_path_length = strlen(server -> base_path);
    server->max_body_bytes = options -> max_body_bytes;
    if (strchr(options -> bind_address, ':') != 0 && options -> bind_address[0] != '[') {
        written = snprintf(listening_ports, sizeof(listening_ports), "[%s]:%u", options -> bind_address, options -> port);
    }
    else {
        written = snprintf(listening_ports, sizeof(listening_ports), "%s:%u", options -> bind_address, options -> port);
    }
    if (written < 0 || (((size_t)written)) >= sizeof(listening_ports)) {
        free(server -> base_path);
        free(server);
        return 1;
    }
    written = snprintf(worker_count, sizeof(worker_count), "%u", options -> worker_count);
    if (written < 0 || (((size_t)written)) >= sizeof(worker_count)) {
        free(server -> base_path);
        free(server);
        return 1;
    }
    written = snprintf(request_timeout, sizeof(request_timeout), "%u", options -> request_timeout_ms);
    if (written < 0 || (((size_t)written)) >= sizeof(request_timeout)) {
        free(server -> base_path);
        free(server);
        return 1;
    }
    configuration[0] = "listening_ports";
    configuration[1] = listening_ports;
    configuration[2] = "num_threads";
    configuration[3] = worker_count;
    configuration[4] = "request_timeout_ms";
    configuration[5] = request_timeout;
    configuration[6] = "enable_keep_alive";
    configuration[7] = "no";
    configuration[8] = "decode_url";
    configuration[9] = "no";
    configuration[10] = "additional_header";
    configuration[11] = "Allow: POST";
    configuration[12] = 0;
    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.begin_request = &lm_rest_lmx_http_begin_request;
    mg_init_library(MG_FEATURES_DEFAULT);
    server->library_initialized = 1;
    server->context = mg_start(&callbacks, server, configuration);
    if (server -> context == 0) {
        mg_exit_library();
        free(server -> base_path);
        free(server);
        return 1;
    }
    memset(ports, 0, sizeof(ports));
    port_count = mg_get_server_ports(server -> context, (((int)(sizeof(ports) / sizeof(ports[0])))), ports);
    if (port_count < 1 || ports[0].port <= 0 || ports[0].port > 65535 || ports[0].is_ssl) {
        mg_stop(server -> context);
        server->context = 0;
        mg_exit_library();
        free(server -> base_path);
        free(server);
        return 1;
    }
    server->listening_port = (((unsigned)ports[0].port));
    out_server[0] = server;
    return 0;
}

int lm_rest_lmx_http_server_start_default(LmMessageThreadRuntime *runtime, LmRestLmxHttpServer **out_server) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    const char *bind_address = getenv("LM_REST_LMX_LISTEN_ADDRESS");
    const char *port_text = getenv("LM_REST_LMX_LISTEN_PORT");
    const char *base_path = getenv("LM_REST_LMX_BASE_PATH");
    const char *max_body_text = getenv("LM_REST_LMX_MAX_BODY_BYTES");
    const char *worker_count_text = getenv("LM_REST_LMX_WORKER_COUNT");
    const char *timeout_text = getenv("LM_REST_LMX_REQUEST_TIMEOUT_MS");
    struct LmRestLmxHttpServerOptionsV1 options_storage;
    LmRestLmxHttpServerOptionsV1 * options = (((LmRestLmxHttpServerOptionsV1 *)&options_storage));
    memset(options, 0, sizeof(options[0]));
    options->abi_size = sizeof(options[0]);
    options->bind_address = bind_address;
    if (options -> bind_address == 0) {
        options->bind_address = "127.0.0.1";
    }
    options->port = 8080U;
    options->base_path = base_path;
    if (options -> base_path == 0) {
        options->base_path = "";
    }
    options->max_body_bytes = 1024U * 1024U;
    options->worker_count = 4U;
    options->request_timeout_ms = 10000U;
    if ((port_text != 0 && lm_rest_lmx_http_parse_unsigned(port_text, &options -> port) != 0) || (max_body_text != 0 && lm_rest_lmx_http_parse_size(max_body_text, &options -> max_body_bytes) != 0) || (worker_count_text != 0 && lm_rest_lmx_http_parse_unsigned(worker_count_text, &options -> worker_count) != 0) || (timeout_text != 0 && lm_rest_lmx_http_parse_unsigned(timeout_text, &options -> request_timeout_ms) != 0)) {
        return 1;
    }
    return lm_rest_lmx_http_server_start(runtime, options, out_server);
}

int lm_rest_lmx_http_server_stop(LmRestLmxHttpServer **server) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    LmRestLmxHttpServer * instance = 0;
    if (server == 0) {
        return 1;
    }
    instance = server[0];
    if (instance == 0) {
        return 0;
    }
    if (instance -> context != 0) {
        mg_stop(instance -> context);
        instance->context = 0;
    }
    if (instance -> library_initialized) {
        mg_exit_library();
        instance->library_initialized = 0;
    }
    free(instance -> base_path);
    instance->base_path = 0;
    instance->runtime = 0;
    free(instance);
    server[0] = 0;
    return 0;
}

int lm_rest_lmx_http_server_port(const LmRestLmxHttpServer *server, unsigned *out_port) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    if (server == 0 || out_port == 0 || server -> context == 0 || server -> listening_port == 0U) {
        return 1;
    }
    out_port[0] = server -> listening_port;
    return 0;
}
#else
const char * lm_rest_lmx_http_server_provider_name(void);
int lm_rest_lmx_http_server_start(LmMessageThreadRuntime *runtime, const LmRestLmxHttpServerOptionsV1 *options, LmRestLmxHttpServer **out_server);
int lm_rest_lmx_http_server_start_default(LmMessageThreadRuntime *runtime, LmRestLmxHttpServer **out_server);
int lm_rest_lmx_http_server_stop(LmRestLmxHttpServer **server);
int lm_rest_lmx_http_server_port(const LmRestLmxHttpServer *server, unsigned *out_port);

const char * lm_rest_lmx_http_server_provider_name(void) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    return "none";
}

int lm_rest_lmx_http_server_start(LmMessageThreadRuntime *runtime, const LmRestLmxHttpServerOptionsV1 *options, LmRestLmxHttpServer **out_server) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    LM_REST_LMX_SERVER_UNUSED(options);
    if (out_server == 0) {
        return 1;
    }
    out_server[0] = 0;
    if (runtime == 0) {
        return 1;
    }
    return 0;
}

int lm_rest_lmx_http_server_start_default(LmMessageThreadRuntime *runtime, LmRestLmxHttpServer **out_server) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    return lm_rest_lmx_http_server_start(runtime, 0, out_server);
}

int lm_rest_lmx_http_server_stop(LmRestLmxHttpServer **server) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    if (server == 0) {
        return 1;
    }
    if (server[0] == 0) {
        return 0;
    }
    return 1;
}

int lm_rest_lmx_http_server_port(const LmRestLmxHttpServer *server, unsigned *out_port) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    LM_REST_LMX_SERVER_UNUSED(server);
    LM_REST_LMX_SERVER_UNUSED(out_port);
    return 1;
}
#endif











