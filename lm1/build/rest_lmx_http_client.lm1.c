#include <stddef.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>

#include <string.h>
struct LmOwnArena;
struct LmMessageThread;
struct LmMessageThreadRuntime;
struct LmMessageThreadPool;
#ifndef lm_lmx_module_private_1_typedef_defined_LmSlice
#define lm_lmx_module_private_1_typedef_defined_LmSlice 1
#define lm_lmx_module_private_1_typedef_id_a_LmSlice 0xdd6442dffff43f92ULL
#define lm_lmx_module_private_1_typedef_id_b_LmSlice 0x21d9c62537977663ULL
typedef struct LmSlice {
    void *ptr;
    size_t length;
} LmSlice;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmSlice) || !defined(lm_lmx_module_private_1_typedef_id_b_LmSlice) || lm_lmx_module_private_1_typedef_id_a_LmSlice != 0xdd6442dffff43f92ULL || lm_lmx_module_private_1_typedef_id_b_LmSlice != 0x21d9c62537977663ULL
#error "Lingvamyxa conflicting typedef projection for LmSlice"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_IncomingMessage
#define lm_lmx_module_private_1_typedef_defined_IncomingMessage 1
#define lm_lmx_module_private_1_typedef_id_a_IncomingMessage 0x10eae12962f7f58fULL
#define lm_lmx_module_private_1_typedef_id_b_IncomingMessage 0xb2add056afdd41bcULL
typedef struct IncomingMessage IncomingMessage;
struct IncomingMessage {
    const char *lmx;
    size_t length;
};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_IncomingMessage) || !defined(lm_lmx_module_private_1_typedef_id_b_IncomingMessage) || lm_lmx_module_private_1_typedef_id_a_IncomingMessage != 0x10eae12962f7f58fULL || lm_lmx_module_private_1_typedef_id_b_IncomingMessage != 0xb2add056afdd41bcULL
#error "Lingvamyxa conflicting typedef projection for IncomingMessage"
#endif
#endif
struct LmMessageThread *lm_message_thread_new(void);
void lm_message_thread_delete(struct LmMessageThread *thread);
struct LmMessageThreadRuntime *lm_message_thread_runtime_new(void);
#ifdef LM_REST_LMX_INSTALL_DEFAULT_CLIENT
int lm_rest_lmx_http_client_install_default(struct LmMessageThreadRuntime *runtime);
#endif
#ifdef LM_REST_LMX_INSTALL_DEFAULT_SERVER
#ifndef lm_lmx_module_private_1_typedef_defined_LmRestLmxHttpServer
#define lm_lmx_module_private_1_typedef_defined_LmRestLmxHttpServer 1
#define lm_lmx_module_private_1_typedef_id_a_LmRestLmxHttpServer 0xc93320eec144d348ULL
#define lm_lmx_module_private_1_typedef_id_b_LmRestLmxHttpServer 0x25d8028910651f29ULL
typedef struct LmRestLmxHttpServer LmRestLmxHttpServer;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmRestLmxHttpServer) || !defined(lm_lmx_module_private_1_typedef_id_b_LmRestLmxHttpServer) || lm_lmx_module_private_1_typedef_id_a_LmRestLmxHttpServer != 0xc93320eec144d348ULL || lm_lmx_module_private_1_typedef_id_b_LmRestLmxHttpServer != 0x25d8028910651f29ULL
#error "Lingvamyxa conflicting typedef projection for LmRestLmxHttpServer"
#endif
#endif
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
#ifndef lm_lmx_module_private_1_typedef_defined_LmMessageThreadExecutionContext
#define lm_lmx_module_private_1_typedef_defined_LmMessageThreadExecutionContext 1
#define lm_lmx_module_private_1_typedef_id_a_LmMessageThreadExecutionContext 0xa01e9b4b1dd2bdb5ULL
#define lm_lmx_module_private_1_typedef_id_b_LmMessageThreadExecutionContext 0x5b5c3f9ba71f9c50ULL
typedef struct LmMessageThreadExecutionContext LmMessageThreadExecutionContext;
struct LmMessageThreadExecutionContext {
    jmp_buf diagnostic_root;
    int diagnostic_code;
    const char *diagnostic_label;
    const char *diagnostic_file;
    int diagnostic_line;
    const char *diagnostic_expr;
};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmMessageThreadExecutionContext) || !defined(lm_lmx_module_private_1_typedef_id_b_LmMessageThreadExecutionContext) || lm_lmx_module_private_1_typedef_id_a_LmMessageThreadExecutionContext != 0xa01e9b4b1dd2bdb5ULL || lm_lmx_module_private_1_typedef_id_b_LmMessageThreadExecutionContext != 0x5b5c3f9ba71f9c50ULL
#error "Lingvamyxa conflicting typedef projection for LmMessageThreadExecutionContext"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_LmLmxMessageThreadEntry
#define lm_lmx_module_private_1_typedef_defined_LmLmxMessageThreadEntry 1
#define lm_lmx_module_private_1_typedef_id_a_LmLmxMessageThreadEntry 0x645bb4e15cf3f304ULL
#define lm_lmx_module_private_1_typedef_id_b_LmLmxMessageThreadEntry 0xb3dc5cc6edc7d305ULL
typedef int (*LmLmxMessageThreadEntry)(struct LmMessageThread *thread);
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmLmxMessageThreadEntry) || !defined(lm_lmx_module_private_1_typedef_id_b_LmLmxMessageThreadEntry) || lm_lmx_module_private_1_typedef_id_a_LmLmxMessageThreadEntry != 0x645bb4e15cf3f304ULL || lm_lmx_module_private_1_typedef_id_b_LmLmxMessageThreadEntry != 0xb3dc5cc6edc7d305ULL
#error "Lingvamyxa conflicting typedef projection for LmLmxMessageThreadEntry"
#endif
#endif
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

#ifndef lm_lmx_module_private_1_typedef_defined_LmOwnDestroyFields
#define lm_lmx_module_private_1_typedef_defined_LmOwnDestroyFields 1
#define lm_lmx_module_private_1_typedef_id_a_LmOwnDestroyFields 0x87dae9040942b0a0ULL
#define lm_lmx_module_private_1_typedef_id_b_LmOwnDestroyFields 0x38ad69ed565e4a65ULL
typedef void (*LmOwnDestroyFields)(void *object);
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmOwnDestroyFields) || !defined(lm_lmx_module_private_1_typedef_id_b_LmOwnDestroyFields) || lm_lmx_module_private_1_typedef_id_a_LmOwnDestroyFields != 0x87dae9040942b0a0ULL || lm_lmx_module_private_1_typedef_id_b_LmOwnDestroyFields != 0x38ad69ed565e4a65ULL
#error "Lingvamyxa conflicting typedef projection for LmOwnDestroyFields"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_LmOwnDelete
#define lm_lmx_module_private_1_typedef_defined_LmOwnDelete 1
#define lm_lmx_module_private_1_typedef_id_a_LmOwnDelete 0xe0a897d278d341eeULL
#define lm_lmx_module_private_1_typedef_id_b_LmOwnDelete 0xca76caf7c88baa9fULL
typedef void (*LmOwnDelete)(void *object);
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmOwnDelete) || !defined(lm_lmx_module_private_1_typedef_id_b_LmOwnDelete) || lm_lmx_module_private_1_typedef_id_a_LmOwnDelete != 0xe0a897d278d341eeULL || lm_lmx_module_private_1_typedef_id_b_LmOwnDelete != 0xca76caf7c88baa9fULL
#error "Lingvamyxa conflicting typedef projection for LmOwnDelete"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_LmHostThreadEntry
#define lm_lmx_module_private_1_typedef_defined_LmHostThreadEntry 1
#define lm_lmx_module_private_1_typedef_id_a_LmHostThreadEntry 0x2bcd1683deffe5bbULL
#define lm_lmx_module_private_1_typedef_id_b_LmHostThreadEntry 0xbf88e44dfd79f8f2ULL
typedef void * (*LmHostThreadEntry)(void *argument);
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmHostThreadEntry) || !defined(lm_lmx_module_private_1_typedef_id_b_LmHostThreadEntry) || lm_lmx_module_private_1_typedef_id_a_LmHostThreadEntry != 0x2bcd1683deffe5bbULL || lm_lmx_module_private_1_typedef_id_b_LmHostThreadEntry != 0xbf88e44dfd79f8f2ULL
#error "Lingvamyxa conflicting typedef projection for LmHostThreadEntry"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_LmMessageThreadEntry
#define lm_lmx_module_private_1_typedef_defined_LmMessageThreadEntry 1
#define lm_lmx_module_private_1_typedef_id_a_LmMessageThreadEntry 0x8154b7f7ecc2154bULL
#define lm_lmx_module_private_1_typedef_id_b_LmMessageThreadEntry 0x29354c13a34b4916ULL
typedef void (*LmMessageThreadEntry)(struct LmMessageThread *lm_lmx_message_thread, void *argument);
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmMessageThreadEntry) || !defined(lm_lmx_module_private_1_typedef_id_b_LmMessageThreadEntry) || lm_lmx_module_private_1_typedef_id_a_LmMessageThreadEntry != 0x8154b7f7ecc2154bULL || lm_lmx_module_private_1_typedef_id_b_LmMessageThreadEntry != 0x29354c13a34b4916ULL
#error "Lingvamyxa conflicting typedef projection for LmMessageThreadEntry"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_LmMessageThreadComponentDestroy
#define lm_lmx_module_private_1_typedef_defined_LmMessageThreadComponentDestroy 1
#define lm_lmx_module_private_1_typedef_id_a_LmMessageThreadComponentDestroy 0x994bdd720ae8075eULL
#define lm_lmx_module_private_1_typedef_id_b_LmMessageThreadComponentDestroy 0xb551f3efe92f53b7ULL
typedef void (*LmMessageThreadComponentDestroy)(struct LmMessageThread *lm_lmx_message_thread, void *component);
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmMessageThreadComponentDestroy) || !defined(lm_lmx_module_private_1_typedef_id_b_LmMessageThreadComponentDestroy) || lm_lmx_module_private_1_typedef_id_a_LmMessageThreadComponentDestroy != 0x994bdd720ae8075eULL || lm_lmx_module_private_1_typedef_id_b_LmMessageThreadComponentDestroy != 0xb551f3efe92f53b7ULL
#error "Lingvamyxa conflicting typedef projection for LmMessageThreadComponentDestroy"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_LmRestLmxPost
#define lm_lmx_module_private_1_typedef_defined_LmRestLmxPost 1
#define lm_lmx_module_private_1_typedef_id_a_LmRestLmxPost 0xf02a42b4277dbd4aULL
#define lm_lmx_module_private_1_typedef_id_b_LmRestLmxPost 0x950c7db65a0b5dbbULL
typedef int (*LmRestLmxPost)(void *context, const char *normalized_uri, const char *body, size_t length, unsigned *out_http_status);
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmRestLmxPost) || !defined(lm_lmx_module_private_1_typedef_id_b_LmRestLmxPost) || lm_lmx_module_private_1_typedef_id_a_LmRestLmxPost != 0xf02a42b4277dbd4aULL || lm_lmx_module_private_1_typedef_id_b_LmRestLmxPost != 0x950c7db65a0b5dbbULL
#error "Lingvamyxa conflicting typedef projection for LmRestLmxPost"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_LmRestLmxDestroy
#define lm_lmx_module_private_1_typedef_defined_LmRestLmxDestroy 1
#define lm_lmx_module_private_1_typedef_id_a_LmRestLmxDestroy 0x4a99563ebb3cfe42ULL
#define lm_lmx_module_private_1_typedef_id_b_LmRestLmxDestroy 0xae9da72f8f4d2a73ULL
typedef void (*LmRestLmxDestroy)(void *context);
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmRestLmxDestroy) || !defined(lm_lmx_module_private_1_typedef_id_b_LmRestLmxDestroy) || lm_lmx_module_private_1_typedef_id_a_LmRestLmxDestroy != 0x4a99563ebb3cfe42ULL || lm_lmx_module_private_1_typedef_id_b_LmRestLmxDestroy != 0xae9da72f8f4d2a73ULL
#error "Lingvamyxa conflicting typedef projection for LmRestLmxDestroy"
#endif
#endif


































































































































































#include <stddef.h>

#define LM_UNUSED (void)
#define LM_REST_LMX_CLIENT_PROVIDER_NONE 0
#define LM_REST_LMX_CLIENT_PROVIDER_LIBCURL 1
#define LM_REST_LMX_CLIENT_PROVIDER_WINHTTP 2
#define LM_REST_LMX_WINHTTP_USER_AGENT L"Lingvamyxa REST/LMX/1"
#define LM_REST_LMX_WINHTTP_METHOD L"POST"
#define LM_REST_LMX_WINHTTP_ROOT_PATH L"/"
#define LM_REST_LMX_WINHTTP_HEADERS L"Content-Type: application/lmx\r\nAccept: application/lmx\r\n"


#ifndef lm_lmx_module_private_1_typedef_defined_LmOwnEdgeKind
#define lm_lmx_module_private_1_typedef_defined_LmOwnEdgeKind 1
#define lm_lmx_module_private_1_typedef_id_a_LmOwnEdgeKind 0x1b0ba13001d5c752ULL
#define lm_lmx_module_private_1_typedef_id_b_LmOwnEdgeKind 0x9d7b68ae3c8fab5bULL
typedef int LmOwnEdgeKind;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmOwnEdgeKind) || !defined(lm_lmx_module_private_1_typedef_id_b_LmOwnEdgeKind) || lm_lmx_module_private_1_typedef_id_a_LmOwnEdgeKind != 0x1b0ba13001d5c752ULL || lm_lmx_module_private_1_typedef_id_b_LmOwnEdgeKind != 0x9d7b68ae3c8fab5bULL
#error "Lingvamyxa conflicting typedef projection for LmOwnEdgeKind"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_LmMessageThreadState
#define lm_lmx_module_private_1_typedef_defined_LmMessageThreadState 1
#define lm_lmx_module_private_1_typedef_id_a_LmMessageThreadState 0x680d7f7cf5d18c05ULL
#define lm_lmx_module_private_1_typedef_id_b_LmMessageThreadState 0x4a157ef92e57affaULL
typedef int LmMessageThreadState;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmMessageThreadState) || !defined(lm_lmx_module_private_1_typedef_id_b_LmMessageThreadState) || lm_lmx_module_private_1_typedef_id_a_LmMessageThreadState != 0x680d7f7cf5d18c05ULL || lm_lmx_module_private_1_typedef_id_b_LmMessageThreadState != 0x4a157ef92e57affaULL
#error "Lingvamyxa conflicting typedef projection for LmMessageThreadState"
#endif
#endif


#ifndef lm_lmx_module_private_1_typedef_defined_LmOwnPtrStack
#define lm_lmx_module_private_1_typedef_defined_LmOwnPtrStack 1
#define lm_lmx_module_private_1_typedef_id_a_LmOwnPtrStack 0x9ae3c1a28ace4260ULL
#define lm_lmx_module_private_1_typedef_id_b_LmOwnPtrStack 0x2a7c240a466f8699ULL
typedef struct LmOwnPtrStack LmOwnPtrStack;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmOwnPtrStack) || !defined(lm_lmx_module_private_1_typedef_id_b_LmOwnPtrStack) || lm_lmx_module_private_1_typedef_id_a_LmOwnPtrStack != 0x9ae3c1a28ace4260ULL || lm_lmx_module_private_1_typedef_id_b_LmOwnPtrStack != 0x2a7c240a466f8699ULL
#error "Lingvamyxa conflicting typedef projection for LmOwnPtrStack"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_LmOwnValueStack
#define lm_lmx_module_private_1_typedef_defined_LmOwnValueStack 1
#define lm_lmx_module_private_1_typedef_id_a_LmOwnValueStack 0xa909010bae94e210ULL
#define lm_lmx_module_private_1_typedef_id_b_LmOwnValueStack 0xa4cd51ea3c5f8959ULL
typedef struct LmOwnValueStack LmOwnValueStack;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmOwnValueStack) || !defined(lm_lmx_module_private_1_typedef_id_b_LmOwnValueStack) || lm_lmx_module_private_1_typedef_id_a_LmOwnValueStack != 0xa909010bae94e210ULL || lm_lmx_module_private_1_typedef_id_b_LmOwnValueStack != 0xa4cd51ea3c5f8959ULL
#error "Lingvamyxa conflicting typedef projection for LmOwnValueStack"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_LmOwnAllocationDescriptor
#define lm_lmx_module_private_1_typedef_defined_LmOwnAllocationDescriptor 1
#define lm_lmx_module_private_1_typedef_id_a_LmOwnAllocationDescriptor 0x8af26fce7d92fafcULL
#define lm_lmx_module_private_1_typedef_id_b_LmOwnAllocationDescriptor 0xb93049b7294f551dULL
typedef struct LmOwnAllocationDescriptor LmOwnAllocationDescriptor;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmOwnAllocationDescriptor) || !defined(lm_lmx_module_private_1_typedef_id_b_LmOwnAllocationDescriptor) || lm_lmx_module_private_1_typedef_id_a_LmOwnAllocationDescriptor != 0x8af26fce7d92fafcULL || lm_lmx_module_private_1_typedef_id_b_LmOwnAllocationDescriptor != 0xb93049b7294f551dULL
#error "Lingvamyxa conflicting typedef projection for LmOwnAllocationDescriptor"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_LmOwnLazyEdge
#define lm_lmx_module_private_1_typedef_defined_LmOwnLazyEdge 1
#define lm_lmx_module_private_1_typedef_id_a_LmOwnLazyEdge 0xdcd1331c45e5a0b0ULL
#define lm_lmx_module_private_1_typedef_id_b_LmOwnLazyEdge 0xeb121590159143d9ULL
typedef struct LmOwnLazyEdge LmOwnLazyEdge;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmOwnLazyEdge) || !defined(lm_lmx_module_private_1_typedef_id_b_LmOwnLazyEdge) || lm_lmx_module_private_1_typedef_id_a_LmOwnLazyEdge != 0xdcd1331c45e5a0b0ULL || lm_lmx_module_private_1_typedef_id_b_LmOwnLazyEdge != 0xeb121590159143d9ULL
#error "Lingvamyxa conflicting typedef projection for LmOwnLazyEdge"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_LmOwnArena
#define lm_lmx_module_private_1_typedef_defined_LmOwnArena 1
#define lm_lmx_module_private_1_typedef_id_a_LmOwnArena 0x7c55fcb0a940aa3cULL
#define lm_lmx_module_private_1_typedef_id_b_LmOwnArena 0xcdfc77b6ba3bf135ULL
typedef struct LmOwnArena LmOwnArena;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmOwnArena) || !defined(lm_lmx_module_private_1_typedef_id_b_LmOwnArena) || lm_lmx_module_private_1_typedef_id_a_LmOwnArena != 0x7c55fcb0a940aa3cULL || lm_lmx_module_private_1_typedef_id_b_LmOwnArena != 0xcdfc77b6ba3bf135ULL
#error "Lingvamyxa conflicting typedef projection for LmOwnArena"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_LmHostThread
#define lm_lmx_module_private_1_typedef_defined_LmHostThread 1
#define lm_lmx_module_private_1_typedef_id_a_LmHostThread 0x0ee0cef52f19074eULL
#define lm_lmx_module_private_1_typedef_id_b_LmHostThread 0x9f6403534e9de8efULL
typedef struct LmHostThread LmHostThread;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmHostThread) || !defined(lm_lmx_module_private_1_typedef_id_b_LmHostThread) || lm_lmx_module_private_1_typedef_id_a_LmHostThread != 0x0ee0cef52f19074eULL || lm_lmx_module_private_1_typedef_id_b_LmHostThread != 0x9f6403534e9de8efULL
#error "Lingvamyxa conflicting typedef projection for LmHostThread"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_LmMutex
#define lm_lmx_module_private_1_typedef_defined_LmMutex 1
#define lm_lmx_module_private_1_typedef_id_a_LmMutex 0x26d0d7b596af7a34ULL
#define lm_lmx_module_private_1_typedef_id_b_LmMutex 0xa8c35a70241bcbcdULL
typedef struct LmMutex LmMutex;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmMutex) || !defined(lm_lmx_module_private_1_typedef_id_b_LmMutex) || lm_lmx_module_private_1_typedef_id_a_LmMutex != 0x26d0d7b596af7a34ULL || lm_lmx_module_private_1_typedef_id_b_LmMutex != 0xa8c35a70241bcbcdULL
#error "Lingvamyxa conflicting typedef projection for LmMutex"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_LmCondition
#define lm_lmx_module_private_1_typedef_defined_LmCondition 1
#define lm_lmx_module_private_1_typedef_id_a_LmCondition 0xb0d7d0c00a11e970ULL
#define lm_lmx_module_private_1_typedef_id_b_LmCondition 0x10616230e414eea1ULL
typedef struct LmCondition LmCondition;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmCondition) || !defined(lm_lmx_module_private_1_typedef_id_b_LmCondition) || lm_lmx_module_private_1_typedef_id_a_LmCondition != 0xb0d7d0c00a11e970ULL || lm_lmx_module_private_1_typedef_id_b_LmCondition != 0x10616230e414eea1ULL
#error "Lingvamyxa conflicting typedef projection for LmCondition"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_LmMessage
#define lm_lmx_module_private_1_typedef_defined_LmMessage 1
#define lm_lmx_module_private_1_typedef_id_a_LmMessage 0x7b0d63fc6cdc6820ULL
#define lm_lmx_module_private_1_typedef_id_b_LmMessage 0xe461bdf9461df589ULL
typedef struct LmMessage LmMessage;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmMessage) || !defined(lm_lmx_module_private_1_typedef_id_b_LmMessage) || lm_lmx_module_private_1_typedef_id_a_LmMessage != 0x7b0d63fc6cdc6820ULL || lm_lmx_module_private_1_typedef_id_b_LmMessage != 0xe461bdf9461df589ULL
#error "Lingvamyxa conflicting typedef projection for LmMessage"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_LmMessageOutboxEntry
#define lm_lmx_module_private_1_typedef_defined_LmMessageOutboxEntry 1
#define lm_lmx_module_private_1_typedef_id_a_LmMessageOutboxEntry 0xda71b412537e2bf6ULL
#define lm_lmx_module_private_1_typedef_id_b_LmMessageOutboxEntry 0x84fe1ad0603fee37ULL
typedef struct LmMessageOutboxEntry LmMessageOutboxEntry;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmMessageOutboxEntry) || !defined(lm_lmx_module_private_1_typedef_id_b_LmMessageOutboxEntry) || lm_lmx_module_private_1_typedef_id_a_LmMessageOutboxEntry != 0xda71b412537e2bf6ULL || lm_lmx_module_private_1_typedef_id_b_LmMessageOutboxEntry != 0x84fe1ad0603fee37ULL
#error "Lingvamyxa conflicting typedef projection for LmMessageOutboxEntry"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_LmMessageRoute
#define lm_lmx_module_private_1_typedef_defined_LmMessageRoute 1
#define lm_lmx_module_private_1_typedef_id_a_LmMessageRoute 0x590ea0a7fef33476ULL
#define lm_lmx_module_private_1_typedef_id_b_LmMessageRoute 0x5917a7ba09ea1f87ULL
typedef struct LmMessageRoute LmMessageRoute;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmMessageRoute) || !defined(lm_lmx_module_private_1_typedef_id_b_LmMessageRoute) || lm_lmx_module_private_1_typedef_id_a_LmMessageRoute != 0x590ea0a7fef33476ULL || lm_lmx_module_private_1_typedef_id_b_LmMessageRoute != 0x5917a7ba09ea1f87ULL
#error "Lingvamyxa conflicting typedef projection for LmMessageRoute"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_LmMessageThreadRuntime
#define lm_lmx_module_private_1_typedef_defined_LmMessageThreadRuntime 1
#define lm_lmx_module_private_1_typedef_id_a_LmMessageThreadRuntime 0xbcdbce07d745c668ULL
#define lm_lmx_module_private_1_typedef_id_b_LmMessageThreadRuntime 0x5267d0b311015cb1ULL
typedef struct LmMessageThreadRuntime LmMessageThreadRuntime;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmMessageThreadRuntime) || !defined(lm_lmx_module_private_1_typedef_id_b_LmMessageThreadRuntime) || lm_lmx_module_private_1_typedef_id_a_LmMessageThreadRuntime != 0xbcdbce07d745c668ULL || lm_lmx_module_private_1_typedef_id_b_LmMessageThreadRuntime != 0x5267d0b311015cb1ULL
#error "Lingvamyxa conflicting typedef projection for LmMessageThreadRuntime"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_LmRestLmxProviderOpsV1
#define lm_lmx_module_private_1_typedef_defined_LmRestLmxProviderOpsV1 1
#define lm_lmx_module_private_1_typedef_id_a_LmRestLmxProviderOpsV1 0xaadc7c363d31231cULL
#define lm_lmx_module_private_1_typedef_id_b_LmRestLmxProviderOpsV1 0x0e805817ebcbb71dULL
typedef struct LmRestLmxProviderOpsV1 LmRestLmxProviderOpsV1;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmRestLmxProviderOpsV1) || !defined(lm_lmx_module_private_1_typedef_id_b_LmRestLmxProviderOpsV1) || lm_lmx_module_private_1_typedef_id_a_LmRestLmxProviderOpsV1 != 0xaadc7c363d31231cULL || lm_lmx_module_private_1_typedef_id_b_LmRestLmxProviderOpsV1 != 0x0e805817ebcbb71dULL
#error "Lingvamyxa conflicting typedef projection for LmRestLmxProviderOpsV1"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_LmMessageThreadPool
#define lm_lmx_module_private_1_typedef_defined_LmMessageThreadPool 1
#define lm_lmx_module_private_1_typedef_id_a_LmMessageThreadPool 0xd41eb629c50c1a70ULL
#define lm_lmx_module_private_1_typedef_id_b_LmMessageThreadPool 0xe036b7fbdebf6a29ULL
typedef struct LmMessageThreadPool LmMessageThreadPool;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmMessageThreadPool) || !defined(lm_lmx_module_private_1_typedef_id_b_LmMessageThreadPool) || lm_lmx_module_private_1_typedef_id_a_LmMessageThreadPool != 0xd41eb629c50c1a70ULL || lm_lmx_module_private_1_typedef_id_b_LmMessageThreadPool != 0xe036b7fbdebf6a29ULL
#error "Lingvamyxa conflicting typedef projection for LmMessageThreadPool"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_LmMessageThreadComponent
#define lm_lmx_module_private_1_typedef_defined_LmMessageThreadComponent 1
#define lm_lmx_module_private_1_typedef_id_a_LmMessageThreadComponent 0x9adbfcfc5ede09deULL
#define lm_lmx_module_private_1_typedef_id_b_LmMessageThreadComponent 0x55c1ece857e1fb9fULL
typedef struct LmMessageThreadComponent LmMessageThreadComponent;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmMessageThreadComponent) || !defined(lm_lmx_module_private_1_typedef_id_b_LmMessageThreadComponent) || lm_lmx_module_private_1_typedef_id_a_LmMessageThreadComponent != 0x9adbfcfc5ede09deULL || lm_lmx_module_private_1_typedef_id_b_LmMessageThreadComponent != 0x55c1ece857e1fb9fULL
#error "Lingvamyxa conflicting typedef projection for LmMessageThreadComponent"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_LmMessageThread
#define lm_lmx_module_private_1_typedef_defined_LmMessageThread 1
#define lm_lmx_module_private_1_typedef_id_a_LmMessageThread 0xb24ebdc4cbfa0b14ULL
#define lm_lmx_module_private_1_typedef_id_b_LmMessageThread 0x198bda4c3705b0e5ULL
typedef struct LmMessageThread LmMessageThread;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmMessageThread) || !defined(lm_lmx_module_private_1_typedef_id_b_LmMessageThread) || lm_lmx_module_private_1_typedef_id_a_LmMessageThread != 0xb24ebdc4cbfa0b14ULL || lm_lmx_module_private_1_typedef_id_b_LmMessageThread != 0x198bda4c3705b0e5ULL
#error "Lingvamyxa conflicting typedef projection for LmMessageThread"
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
#ifndef lm_lmx_module_private_1_typedef_defined_LmRestLmxHttpClientContext
#define lm_lmx_module_private_1_typedef_defined_LmRestLmxHttpClientContext 1
#define lm_lmx_module_private_1_typedef_id_a_LmRestLmxHttpClientContext 0x0c111efa18728812ULL
#define lm_lmx_module_private_1_typedef_id_b_LmRestLmxHttpClientContext 0xed23b3601acf4b23ULL
typedef struct LmRestLmxHttpClientContext {
    int curl_initialized;
    void *session;
} LmRestLmxHttpClientContext;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmRestLmxHttpClientContext) || !defined(lm_lmx_module_private_1_typedef_id_b_LmRestLmxHttpClientContext) || lm_lmx_module_private_1_typedef_id_a_LmRestLmxHttpClientContext != 0x0c111efa18728812ULL || lm_lmx_module_private_1_typedef_id_b_LmRestLmxHttpClientContext != 0xed23b3601acf4b23ULL
#error "Lingvamyxa conflicting typedef projection for LmRestLmxHttpClientContext"
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
const char * (lm_rest_lmx_http_client_provider_name)(void);
int (lm_rest_lmx_http_client_install_default)(LmMessageThreadRuntime *runtime);


#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef LM_REST_LMX_CLIENT_PROVIDER
#define LM_REST_LMX_CLIENT_PROVIDER LM_REST_LMX_CLIENT_PROVIDER_NONE
#endif

#if LM_REST_LMX_CLIENT_PROVIDER != LM_REST_LMX_CLIENT_PROVIDER_NONE && LM_REST_LMX_CLIENT_PROVIDER != LM_REST_LMX_CLIENT_PROVIDER_LIBCURL && LM_REST_LMX_CLIENT_PROVIDER != LM_REST_LMX_CLIENT_PROVIDER_WINHTTP
static int lm_rest_lmx_http_client_invalid_provider_configuration(void);

static int lm_rest_lmx_http_client_invalid_provider_configuration(void) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    return LM_REST_LMX_CLIENT_PROVIDER_MUST_BE_NONE_LIBCURL_OR_WINHTTP;
}
#elif LM_REST_LMX_CLIENT_PROVIDER == LM_REST_LMX_CLIENT_PROVIDER_WINHTTP && !defined(_WIN32)
static int lm_rest_lmx_http_client_invalid_winhttp_target(void);

static int lm_rest_lmx_http_client_invalid_winhttp_target(void) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    return LM_REST_LMX_CLIENT_PROVIDER_WINHTTP_REQUIRES_WINDOWS;
}
#elif LM_REST_LMX_CLIENT_PROVIDER == LM_REST_LMX_CLIENT_PROVIDER_LIBCURL
#include <curl/curl.h>

#if LIBCURL_VERSION_NUM >= 0x075500
static int lm_rest_lmx_http_configure_protocols(CURL *easy);

static int lm_rest_lmx_http_configure_protocols(CURL *easy) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    if (curl_easy_setopt(easy, CURLOPT_PROTOCOLS_STR, "http,https") != CURLE_OK) {
        return 1;
    }
    return 0;
}
#else
static int lm_rest_lmx_http_configure_protocols(CURL *easy);

static int lm_rest_lmx_http_configure_protocols(CURL *easy) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    if (curl_easy_setopt(easy, CURLOPT_PROTOCOLS, (((long)(CURLPROTO_HTTP | CURLPROTO_HTTPS)))) != CURLE_OK) {
        return 1;
    }
    return 0;
}
#endif
static int lm_rest_lmx_http_ascii_prefix(const char *text, const char *prefix);
static int lm_rest_lmx_http_uri_is_supported(const char *uri);
static size_t lm_rest_lmx_http_discard_response(char *data, size_t size, size_t count, void *context);
static void lm_rest_lmx_http_client_destroy(void *opaque_context);
static int lm_rest_lmx_http_client_post(void *opaque_context, const char *normalized_uri, const char *body, size_t length, unsigned *out_http_status);
const char * lm_rest_lmx_http_client_provider_name(void);
int lm_rest_lmx_http_client_install_default(LmMessageThreadRuntime *runtime);

static int lm_rest_lmx_http_ascii_prefix(const char *text, const char *prefix) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    size_t index = 0U;
    unsigned char left;
    unsigned char right;
    if (text == 0 || prefix == 0) {
        return 0;
    }
    while (prefix[index] != '\0') {
        left = (((unsigned char)text[index]));
        right = (((unsigned char)prefix[index]));
        if (left >= (((unsigned char)'A')) && left <= (((unsigned char)'Z'))) {
            left = (((unsigned char)(left + ((((unsigned char)'a')) - (((unsigned char)'A'))))));
        }
        if (right >= (((unsigned char)'A')) && right <= (((unsigned char)'Z'))) {
            right = (((unsigned char)(right + ((((unsigned char)'a')) - (((unsigned char)'A'))))));
        }
        if (left == (((unsigned char)'\0')) || left != right) {
            return 0;
        }
        index = index + 1U;
    }
    return 1;
}

static int lm_rest_lmx_http_uri_is_supported(const char *uri) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    return lm_rest_lmx_http_ascii_prefix(uri, "http://") || lm_rest_lmx_http_ascii_prefix(uri, "https://");
}

static size_t lm_rest_lmx_http_discard_response(char *data, size_t size, size_t count, void *context) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    LM_UNUSED(data);
    LM_UNUSED(context);
    if (size != 0U && count > SIZE_MAX / size) {
        return 0U;
    }
    return size * count;
}

static void lm_rest_lmx_http_client_destroy(void *opaque_context) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    LmRestLmxHttpClientContext * context = 0;
    context = (((LmRestLmxHttpClientContext *)opaque_context));
    if (context == 0) {
        return;
    }
    if (context -> curl_initialized) {
        curl_global_cleanup();
        context->curl_initialized = 0;
    }
    free(context);
}

static int lm_rest_lmx_http_client_post(void *opaque_context, const char *normalized_uri, const char *body, size_t length, unsigned *out_http_status) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    LmRestLmxHttpClientContext * context = 0;
    CURL *easy = 0;
    struct curl_slist *headers = 0;
    struct curl_slist *updated_headers = 0;
    curl_off_t post_length;
    CURLcode result;
    long response_code = 0L;
    const char *post_body;
    context = (((LmRestLmxHttpClientContext *)opaque_context));
    if (out_http_status != 0) {
        out_http_status[0] = 0U;
    }
    if (context == 0 || context -> curl_initialized == 0 || normalized_uri == 0 || out_http_status == 0 || (body == 0 && length != 0U) || lm_rest_lmx_http_uri_is_supported(normalized_uri) == 0) {
        {
            int lm_return_0 = 1;
            curl_slist_free_all(headers);
            if (easy != 0) {
                curl_easy_cleanup(easy);
            }
            return lm_return_0;
        }
    }
    post_length = (((curl_off_t)length));
    if (post_length < (((curl_off_t)0)) || (((size_t)post_length)) != length) {
        {
            int lm_return_1 = 1;
            curl_slist_free_all(headers);
            if (easy != 0) {
                curl_easy_cleanup(easy);
            }
            return lm_return_1;
        }
    }
    easy = curl_easy_init();
    if (easy == 0) {
        {
            int lm_return_2 = 1;
            curl_slist_free_all(headers);
            if (easy != 0) {
                curl_easy_cleanup(easy);
            }
            return lm_return_2;
        }
    }
    headers = curl_slist_append(0, "Content-Type: application/lmx");
    if (headers == 0) {
        {
            int lm_return_3 = 1;
            curl_slist_free_all(headers);
            if (easy != 0) {
                curl_easy_cleanup(easy);
            }
            return lm_return_3;
        }
    }
    updated_headers = curl_slist_append(headers, "Accept: application/lmx");
    if (updated_headers == 0) {
        {
            int lm_return_4 = 1;
            curl_slist_free_all(headers);
            if (easy != 0) {
                curl_easy_cleanup(easy);
            }
            return lm_return_4;
        }
    }
    headers = updated_headers;
    post_body = body;
    if (post_body == 0) {
        post_body = "";
    }
    if (curl_easy_setopt(easy, CURLOPT_URL, normalized_uri) != CURLE_OK) {
        {
            int lm_return_5 = 1;
            curl_slist_free_all(headers);
            if (easy != 0) {
                curl_easy_cleanup(easy);
            }
            return lm_return_5;
        }
    }
    if (curl_easy_setopt(easy, CURLOPT_POST, 1L) != CURLE_OK) {
        {
            int lm_return_6 = 1;
            curl_slist_free_all(headers);
            if (easy != 0) {
                curl_easy_cleanup(easy);
            }
            return lm_return_6;
        }
    }
    if (curl_easy_setopt(easy, CURLOPT_POSTFIELDS, post_body) != CURLE_OK) {
        {
            int lm_return_7 = 1;
            curl_slist_free_all(headers);
            if (easy != 0) {
                curl_easy_cleanup(easy);
            }
            return lm_return_7;
        }
    }
    if (curl_easy_setopt(easy, CURLOPT_POSTFIELDSIZE_LARGE, post_length) != CURLE_OK) {
        {
            int lm_return_8 = 1;
            curl_slist_free_all(headers);
            if (easy != 0) {
                curl_easy_cleanup(easy);
            }
            return lm_return_8;
        }
    }
    if (curl_easy_setopt(easy, CURLOPT_HTTPHEADER, headers) != CURLE_OK) {
        {
            int lm_return_9 = 1;
            curl_slist_free_all(headers);
            if (easy != 0) {
                curl_easy_cleanup(easy);
            }
            return lm_return_9;
        }
    }
    if (curl_easy_setopt(easy, CURLOPT_FOLLOWLOCATION, 0L) != CURLE_OK) {
        {
            int lm_return_10 = 1;
            curl_slist_free_all(headers);
            if (easy != 0) {
                curl_easy_cleanup(easy);
            }
            return lm_return_10;
        }
    }
    if (curl_easy_setopt(easy, CURLOPT_MAXREDIRS, 0L) != CURLE_OK) {
        {
            int lm_return_11 = 1;
            curl_slist_free_all(headers);
            if (easy != 0) {
                curl_easy_cleanup(easy);
            }
            return lm_return_11;
        }
    }
    if (curl_easy_setopt(easy, CURLOPT_PATH_AS_IS, 1L) != CURLE_OK) {
        {
            int lm_return_12 = 1;
            curl_slist_free_all(headers);
            if (easy != 0) {
                curl_easy_cleanup(easy);
            }
            return lm_return_12;
        }
    }
    if (lm_rest_lmx_http_configure_protocols(easy) != 0) {
        {
            int lm_return_13 = 1;
            curl_slist_free_all(headers);
            if (easy != 0) {
                curl_easy_cleanup(easy);
            }
            return lm_return_13;
        }
    }
    if (curl_easy_setopt(easy, CURLOPT_NOSIGNAL, 1L) != CURLE_OK) {
        {
            int lm_return_14 = 1;
            curl_slist_free_all(headers);
            if (easy != 0) {
                curl_easy_cleanup(easy);
            }
            return lm_return_14;
        }
    }
    if (curl_easy_setopt(easy, CURLOPT_CONNECTTIMEOUT_MS, 10000L) != CURLE_OK) {
        {
            int lm_return_15 = 1;
            curl_slist_free_all(headers);
            if (easy != 0) {
                curl_easy_cleanup(easy);
            }
            return lm_return_15;
        }
    }
    if (curl_easy_setopt(easy, CURLOPT_TIMEOUT_MS, 30000L) != CURLE_OK) {
        {
            int lm_return_16 = 1;
            curl_slist_free_all(headers);
            if (easy != 0) {
                curl_easy_cleanup(easy);
            }
            return lm_return_16;
        }
    }
    if (curl_easy_setopt(easy, CURLOPT_SSL_VERIFYPEER, 1L) != CURLE_OK) {
        {
            int lm_return_17 = 1;
            curl_slist_free_all(headers);
            if (easy != 0) {
                curl_easy_cleanup(easy);
            }
            return lm_return_17;
        }
    }
    if (curl_easy_setopt(easy, CURLOPT_SSL_VERIFYHOST, 2L) != CURLE_OK) {
        {
            int lm_return_18 = 1;
            curl_slist_free_all(headers);
            if (easy != 0) {
                curl_easy_cleanup(easy);
            }
            return lm_return_18;
        }
    }
    if (curl_easy_setopt(easy, CURLOPT_WRITEFUNCTION, &lm_rest_lmx_http_discard_response) != CURLE_OK) {
        {
            int lm_return_19 = 1;
            curl_slist_free_all(headers);
            if (easy != 0) {
                curl_easy_cleanup(easy);
            }
            return lm_return_19;
        }
    }
    result = curl_easy_perform(easy);
    if (result != CURLE_OK || curl_easy_getinfo(easy, CURLINFO_RESPONSE_CODE, &response_code) != CURLE_OK || response_code < 0L || (((unsigned long)response_code)) > (((unsigned long)UINT_MAX))) {
        {
            int lm_return_20 = 1;
            curl_slist_free_all(headers);
            if (easy != 0) {
                curl_easy_cleanup(easy);
            }
            return lm_return_20;
        }
    }
    out_http_status[0] = (((unsigned)response_code));
    {
        int lm_return_21 = 0;
        curl_slist_free_all(headers);
        if (easy != 0) {
            curl_easy_cleanup(easy);
        }
        return lm_return_21;
    }
    curl_slist_free_all(headers);
    if (easy != 0) {
        curl_easy_cleanup(easy);
    }
}

const char * lm_rest_lmx_http_client_provider_name(void) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    return "libcurl";
}

int lm_rest_lmx_http_client_install_default(LmMessageThreadRuntime *runtime) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    LmRestLmxHttpClientContext * context = 0;
    LmRestLmxProviderOpsV1 * ops = 0;
    if (runtime == 0) {
        {
            int lm_return_0 = 1;
            free(ops);
            if (context != 0) {
                lm_rest_lmx_http_client_destroy(context);
            }
            return lm_return_0;
        }
    }
    context = (((LmRestLmxHttpClientContext *)calloc(1U, sizeof(context[0]))));
    ops = (((LmRestLmxProviderOpsV1 *)calloc(1U, sizeof(ops[0]))));
    if (context == 0 || ops == 0) {
        {
            int lm_return_1 = 1;
            free(ops);
            if (context != 0) {
                lm_rest_lmx_http_client_destroy(context);
            }
            return lm_return_1;
        }
    }
    if (curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK) {
        {
            int lm_return_2 = 1;
            free(ops);
            if (context != 0) {
                lm_rest_lmx_http_client_destroy(context);
            }
            return lm_return_2;
        }
    }
    context->curl_initialized = 1;
    ops->abi_size = sizeof(ops[0]);
    ops->post = &lm_rest_lmx_http_client_post;
    ops->destroy = &lm_rest_lmx_http_client_destroy;
    if (lm_message_thread_runtime_set_rest_lmx_provider(runtime, ops, context) != 0) {
        {
            int lm_return_3 = 1;
            free(ops);
            if (context != 0) {
                lm_rest_lmx_http_client_destroy(context);
            }
            return lm_return_3;
        }
    }
    context = 0;
    {
        int lm_return_4 = 0;
        free(ops);
        if (context != 0) {
            lm_rest_lmx_http_client_destroy(context);
        }
        return lm_return_4;
    }
    free(ops);
    if (context != 0) {
        lm_rest_lmx_http_client_destroy(context);
    }
}
#elif LM_REST_LMX_CLIENT_PROVIDER == LM_REST_LMX_CLIENT_PROVIDER_WINHTTP
#include <windows.h>
#include <winhttp.h>
static wchar_t * lm_rest_lmx_http_wide_uri(const char *uri);
static wchar_t * lm_rest_lmx_http_wide_slice(const wchar_t *source, DWORD length);
static void lm_rest_lmx_http_client_destroy(void *opaque_context);
static int lm_rest_lmx_http_client_post(void *opaque_context, const char *normalized_uri, const char *body, size_t length, unsigned *out_http_status);
const char * lm_rest_lmx_http_client_provider_name(void);
int lm_rest_lmx_http_client_install_default(LmMessageThreadRuntime *runtime);

static wchar_t * lm_rest_lmx_http_wide_uri(const char *uri) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    size_t length;
    int wide_length;
    wchar_t *wide_uri = 0;
    if (uri == 0) {
        return 0;
    }
    length = strlen(uri);
    if (length > (((size_t)INT_MAX)) - 1U) {
        return 0;
    }
    wide_length = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, uri, -1, 0, 0);
    if (wide_length <= 0 || (((size_t)wide_length)) > SIZE_MAX / sizeof(wide_uri[0])) {
        return 0;
    }
    wide_uri = (((wchar_t *)calloc((((size_t)wide_length)), sizeof(wide_uri[0]))));
    if (wide_uri == 0 || MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, uri, -1, wide_uri, wide_length) != wide_length) {
        free(wide_uri);
        return 0;
    }
    return wide_uri;
}

static wchar_t * lm_rest_lmx_http_wide_slice(const wchar_t *source, DWORD length) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    wchar_t *copy = 0;
    if (source == 0 || (((size_t)length)) > SIZE_MAX / sizeof(copy[0]) - 1U) {
        return 0;
    }
    copy = (((wchar_t *)calloc((((size_t)length)) + 1U, sizeof(copy[0]))));
    if (copy == 0) {
        return 0;
    }
    if (length != 0U) {
        memcpy(copy, source, (((size_t)length)) * sizeof(copy[0]));
    }
    return copy;
}

static void lm_rest_lmx_http_client_destroy(void *opaque_context) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    LmRestLmxHttpClientContext * context = 0;
    context = (((LmRestLmxHttpClientContext *)opaque_context));
    if (context == 0) {
        return;
    }
    if (context -> session != 0) {
        WinHttpCloseHandle(context -> session);
        context->session = 0;
    }
    free(context);
}

static int lm_rest_lmx_http_client_post(void *opaque_context, const char *normalized_uri, const char *body, size_t length, unsigned *out_http_status) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    LmRestLmxHttpClientContext * context = 0;
    wchar_t *wide_uri = 0;
    wchar_t *host = 0;
    wchar_t *path = 0;
    URL_COMPONENTS components;
    void *connection = 0;
    void *request = 0;
    void *request_body = 0;
    DWORD request_flags = WINHTTP_FLAG_ESCAPE_DISABLE;
    DWORD disabled_features = WINHTTP_DISABLE_REDIRECTS;
    DWORD body_length = (((DWORD)length));
    DWORD response_status = 0U;
    DWORD response_status_size = (((DWORD)sizeof(response_status)));
    context = (((LmRestLmxHttpClientContext *)opaque_context));
    if (out_http_status != 0) {
        out_http_status[0] = 0U;
    }
    if (context == 0 || context -> session == 0 || normalized_uri == 0 || out_http_status == 0 || (body == 0 && length != 0U) || (((size_t)body_length)) != length) {
        {
            int lm_return_0 = 1;
            if (request != 0) {
                WinHttpCloseHandle(request);
            }
            if (connection != 0) {
                WinHttpCloseHandle(connection);
            }
            free(path);
            free(host);
            free(wide_uri);
            return lm_return_0;
        }
    }
    wide_uri = lm_rest_lmx_http_wide_uri(normalized_uri);
    if (wide_uri == 0) {
        {
            int lm_return_1 = 1;
            if (request != 0) {
                WinHttpCloseHandle(request);
            }
            if (connection != 0) {
                WinHttpCloseHandle(connection);
            }
            free(path);
            free(host);
            free(wide_uri);
            return lm_return_1;
        }
    }
    memset(&components, 0, sizeof(components));
    components.dwStructSize = sizeof(components);
    components.dwSchemeLength = (((DWORD)-1));
    components.dwHostNameLength = (((DWORD)-1));
    components.dwUrlPathLength = (((DWORD)-1));
    components.dwExtraInfoLength = (((DWORD)-1));
    if (WinHttpCrackUrl(wide_uri, 0U, 0U, &components) == 0 || (components.nScheme != INTERNET_SCHEME_HTTP && components.nScheme != INTERNET_SCHEME_HTTPS) || components.lpszHostName == 0 || components.dwHostNameLength == 0U || components.dwExtraInfoLength != 0U) {
        {
            int lm_return_2 = 1;
            if (request != 0) {
                WinHttpCloseHandle(request);
            }
            if (connection != 0) {
                WinHttpCloseHandle(connection);
            }
            free(path);
            free(host);
            free(wide_uri);
            return lm_return_2;
        }
    }
    host = lm_rest_lmx_http_wide_slice(components.lpszHostName, components.dwHostNameLength);
    if (components.lpszUrlPath != 0 && components.dwUrlPathLength != 0U) {
        path = lm_rest_lmx_http_wide_slice(components.lpszUrlPath, components.dwUrlPathLength);
    }
    if (path == 0 && (components.lpszUrlPath == 0 || components.dwUrlPathLength == 0U)) {
        path = lm_rest_lmx_http_wide_slice(LM_REST_LMX_WINHTTP_ROOT_PATH, 1U);
    }
    if (host == 0 || path == 0) {
        {
            int lm_return_3 = 1;
            if (request != 0) {
                WinHttpCloseHandle(request);
            }
            if (connection != 0) {
                WinHttpCloseHandle(connection);
            }
            free(path);
            free(host);
            free(wide_uri);
            return lm_return_3;
        }
    }
    connection = WinHttpConnect(context -> session, host, components.nPort, 0U);
    if (connection == 0) {
        {
            int lm_return_4 = 1;
            if (request != 0) {
                WinHttpCloseHandle(request);
            }
            if (connection != 0) {
                WinHttpCloseHandle(connection);
            }
            free(path);
            free(host);
            free(wide_uri);
            return lm_return_4;
        }
    }
    if (components.nScheme == INTERNET_SCHEME_HTTPS) {
        request_flags = request_flags | WINHTTP_FLAG_SECURE;
    }
    request = WinHttpOpenRequest(connection, LM_REST_LMX_WINHTTP_METHOD, path, 0, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, request_flags);
    if (request == 0) {
        {
            int lm_return_5 = 1;
            if (request != 0) {
                WinHttpCloseHandle(request);
            }
            if (connection != 0) {
                WinHttpCloseHandle(connection);
            }
            free(path);
            free(host);
            free(wide_uri);
            return lm_return_5;
        }
    }
    if (body_length != 0U) {
        request_body = (((void *)body));
    }
    if (WinHttpSetOption(request, WINHTTP_OPTION_DISABLE_FEATURE, &disabled_features, sizeof(disabled_features)) == 0) {
        {
            int lm_return_6 = 1;
            if (request != 0) {
                WinHttpCloseHandle(request);
            }
            if (connection != 0) {
                WinHttpCloseHandle(connection);
            }
            free(path);
            free(host);
            free(wide_uri);
            return lm_return_6;
        }
    }
    if (WinHttpSetTimeouts(request, 10000, 10000, 30000, 30000) == 0) {
        {
            int lm_return_7 = 1;
            if (request != 0) {
                WinHttpCloseHandle(request);
            }
            if (connection != 0) {
                WinHttpCloseHandle(connection);
            }
            free(path);
            free(host);
            free(wide_uri);
            return lm_return_7;
        }
    }
    if (WinHttpAddRequestHeaders(request, LM_REST_LMX_WINHTTP_HEADERS, (((DWORD)-1)), WINHTTP_ADDREQ_FLAG_ADD | WINHTTP_ADDREQ_FLAG_REPLACE) == 0) {
        {
            int lm_return_8 = 1;
            if (request != 0) {
                WinHttpCloseHandle(request);
            }
            if (connection != 0) {
                WinHttpCloseHandle(connection);
            }
            free(path);
            free(host);
            free(wide_uri);
            return lm_return_8;
        }
    }
    if (WinHttpSendRequest(request, WINHTTP_NO_ADDITIONAL_HEADERS, 0U, request_body, body_length, body_length, 0U) == 0) {
        {
            int lm_return_9 = 1;
            if (request != 0) {
                WinHttpCloseHandle(request);
            }
            if (connection != 0) {
                WinHttpCloseHandle(connection);
            }
            free(path);
            free(host);
            free(wide_uri);
            return lm_return_9;
        }
    }
    if (WinHttpReceiveResponse(request, 0) == 0) {
        {
            int lm_return_10 = 1;
            if (request != 0) {
                WinHttpCloseHandle(request);
            }
            if (connection != 0) {
                WinHttpCloseHandle(connection);
            }
            free(path);
            free(host);
            free(wide_uri);
            return lm_return_10;
        }
    }
    if (WinHttpQueryHeaders(request, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, WINHTTP_HEADER_NAME_BY_INDEX, &response_status, &response_status_size, WINHTTP_NO_HEADER_INDEX) == 0) {
        {
            int lm_return_11 = 1;
            if (request != 0) {
                WinHttpCloseHandle(request);
            }
            if (connection != 0) {
                WinHttpCloseHandle(connection);
            }
            free(path);
            free(host);
            free(wide_uri);
            return lm_return_11;
        }
    }
    out_http_status[0] = (((unsigned)response_status));
    {
        int lm_return_12 = 0;
        if (request != 0) {
            WinHttpCloseHandle(request);
        }
        if (connection != 0) {
            WinHttpCloseHandle(connection);
        }
        free(path);
        free(host);
        free(wide_uri);
        return lm_return_12;
    }
    if (request != 0) {
        WinHttpCloseHandle(request);
    }
    if (connection != 0) {
        WinHttpCloseHandle(connection);
    }
    free(path);
    free(host);
    free(wide_uri);
}

const char * lm_rest_lmx_http_client_provider_name(void) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    return "winhttp";
}

int lm_rest_lmx_http_client_install_default(LmMessageThreadRuntime *runtime) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    LmRestLmxHttpClientContext * context = 0;
    LmRestLmxProviderOpsV1 * ops = 0;
    if (runtime == 0) {
        {
            int lm_return_0 = 1;
            free(ops);
            if (context != 0) {
                lm_rest_lmx_http_client_destroy(context);
            }
            return lm_return_0;
        }
    }
    context = (((LmRestLmxHttpClientContext *)calloc(1U, sizeof(context[0]))));
    ops = (((LmRestLmxProviderOpsV1 *)calloc(1U, sizeof(ops[0]))));
    if (context == 0 || ops == 0) {
        {
            int lm_return_1 = 1;
            free(ops);
            if (context != 0) {
                lm_rest_lmx_http_client_destroy(context);
            }
            return lm_return_1;
        }
    }
    context->session = WinHttpOpen(LM_REST_LMX_WINHTTP_USER_AGENT, WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0U);
    if (context -> session == 0) {
        {
            int lm_return_2 = 1;
            free(ops);
            if (context != 0) {
                lm_rest_lmx_http_client_destroy(context);
            }
            return lm_return_2;
        }
    }
    ops->abi_size = sizeof(ops[0]);
    ops->post = &lm_rest_lmx_http_client_post;
    ops->destroy = &lm_rest_lmx_http_client_destroy;
    if (lm_message_thread_runtime_set_rest_lmx_provider(runtime, ops, context) != 0) {
        {
            int lm_return_3 = 1;
            free(ops);
            if (context != 0) {
                lm_rest_lmx_http_client_destroy(context);
            }
            return lm_return_3;
        }
    }
    context = 0;
    {
        int lm_return_4 = 0;
        free(ops);
        if (context != 0) {
            lm_rest_lmx_http_client_destroy(context);
        }
        return lm_return_4;
    }
    free(ops);
    if (context != 0) {
        lm_rest_lmx_http_client_destroy(context);
    }
}
#else
const char * lm_rest_lmx_http_client_provider_name(void);
int lm_rest_lmx_http_client_install_default(LmMessageThreadRuntime *runtime);

const char * lm_rest_lmx_http_client_provider_name(void) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    return "none";
}

int lm_rest_lmx_http_client_install_default(LmMessageThreadRuntime *runtime) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    if (runtime == 0) {
        return 1;
    }
    return lm_message_thread_runtime_set_rest_lmx_provider(runtime, 0, 0);
}
#endif









