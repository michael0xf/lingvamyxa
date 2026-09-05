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
void *lm_own_arena_copy_graph(struct LmMessageThread *lm_lmx_message_thread, struct LmOwnArena *arena, const void *source);
int lm_own_arena_pin(struct LmMessageThread *lm_lmx_message_thread, struct LmOwnArena *arena, void *address);
int lm_own_arena_root_add(struct LmMessageThread *lm_lmx_message_thread, struct LmOwnArena *arena, void *address);
int lm_own_arena_reclaim(struct LmMessageThread *lm_lmx_message_thread, struct LmOwnArena *arena);
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

#include <stddef.h>

#define LM_UNUSED (void)




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
    int marked;
    int pinned;
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
    LmOwnPtrStack * roots;
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
#ifndef lm_lmx_module_private_1_typedef_defined_LmBuildOptions
#define lm_lmx_module_private_1_typedef_defined_LmBuildOptions 1
#define lm_lmx_module_private_1_typedef_id_a_LmBuildOptions 0x00e08a2e2777177bULL
#define lm_lmx_module_private_1_typedef_id_b_LmBuildOptions 0x28289df227a734e4ULL
typedef struct LmBuildOptions {
    int full_build;
    int next_build;
    int test_suite;
} LmBuildOptions;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmBuildOptions) || !defined(lm_lmx_module_private_1_typedef_id_b_LmBuildOptions) || lm_lmx_module_private_1_typedef_id_a_LmBuildOptions != 0x00e08a2e2777177bULL || lm_lmx_module_private_1_typedef_id_b_LmBuildOptions != 0x28289df227a734e4ULL
#error "Lingvamyxa conflicting typedef projection for LmBuildOptions"
#endif
#endif


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
int (lm_own_arena_pin)(struct LmMessageThread *lm_lmx_message_thread, LmOwnArena *arena, void *address);
int (lm_own_arena_root_add)(struct LmMessageThread *lm_lmx_message_thread, LmOwnArena *arena, void *address);
void * (lm_own_arena_copy_graph)(struct LmMessageThread *lm_lmx_message_thread, LmOwnArena *arena, void *source);
int (lm_own_arena_reclaim)(struct LmMessageThread *lm_lmx_message_thread, LmOwnArena *arena);
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















































































































































































#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <direct.h>
static char * lm_build_exe_suffix(struct LmMessageThread *lm_lmx_message_thread);
static char * lm_build_path_sep(struct LmMessageThread *lm_lmx_message_thread);
static int lm_build_platform_chdir(struct LmMessageThread *lm_lmx_message_thread, char *path);
static char * lm_build_platform_getcwd(struct LmMessageThread *lm_lmx_message_thread, char *buffer, size_t size);
static int lm_build_platform_absolute(struct LmMessageThread *lm_lmx_message_thread, char *path);
static int lm_build_file_exists(struct LmMessageThread *lm_lmx_message_thread, char *path);
static int lm_build_has_qt_cmake(struct LmMessageThread *lm_lmx_message_thread);
static int lm_build_has_qt_mingw_make(struct LmMessageThread *lm_lmx_message_thread);
static int lm_build_has_qt_gcc(struct LmMessageThread *lm_lmx_message_thread);
static int lm_build_has_qt_gxx(struct LmMessageThread *lm_lmx_message_thread);
static char * lm_build_platform_canary_command_format(struct LmMessageThread *lm_lmx_message_thread);
static char * lm_build_platform_canary_script_path(struct LmMessageThread *lm_lmx_message_thread);
static char * lm_build_platform_canary_command_path(struct LmMessageThread *lm_lmx_message_thread, char *canary_path);
static int lm_build_prepare_platform_canary(struct LmMessageThread *lm_lmx_message_thread, char *canary_path);
static char * lm_build_platform_tests_script_path(struct LmMessageThread *lm_lmx_message_thread);
static char * lm_build_platform_tests_command_format(struct LmMessageThread *lm_lmx_message_thread);
static int lm_build_setenv(struct LmMessageThread *lm_lmx_message_thread, char *name, char *value);
static int lm_build_unsetenv(struct LmMessageThread *lm_lmx_message_thread, char *name);
static int lm_build_write_platform_tests_script(struct LmMessageThread *lm_lmx_message_thread, FILE *file, char *output_dir, char *parser_library, char *own_library);

static char * lm_build_exe_suffix(struct LmMessageThread *lm_lmx_message_thread) {
    (void)lm_lmx_message_thread;
    return ".exe";
}

static char * lm_build_path_sep(struct LmMessageThread *lm_lmx_message_thread) {
    (void)lm_lmx_message_thread;
    return "\\";
}

static int lm_build_platform_chdir(struct LmMessageThread *lm_lmx_message_thread, char *path) {
    (void)lm_lmx_message_thread;
    return _chdir(path);
}

static char * lm_build_platform_getcwd(struct LmMessageThread *lm_lmx_message_thread, char *buffer, size_t size) {
    (void)lm_lmx_message_thread;
    return _getcwd(buffer, size);
}

static int lm_build_platform_absolute(struct LmMessageThread *lm_lmx_message_thread, char *path) {
    (void)lm_lmx_message_thread;
    return ((path[0] == '/' || path[0] == '\\') && (path[1] == '/' || path[1] == '\\')) || (path[0] != '\0' && path[1] == ':' && (path[2] == '/' || path[2] == '\\'));
}

static int lm_build_file_exists(struct LmMessageThread *lm_lmx_message_thread, char *path) {
    (void)lm_lmx_message_thread;
    FILE * file;
    file = fopen(path, "rb");
    if (file == 0) {
        return 0;
    }
    fclose(file);
    return 1;
}

static int lm_build_has_qt_cmake(struct LmMessageThread *lm_lmx_message_thread) {
    (void)lm_lmx_message_thread;
    return lm_build_file_exists(lm_lmx_message_thread, "C:/Qt/Tools/CMake_64/bin/cmake.exe");
}

static int lm_build_has_qt_mingw_make(struct LmMessageThread *lm_lmx_message_thread) {
    (void)lm_lmx_message_thread;
    return lm_build_file_exists(lm_lmx_message_thread, "C:/Qt/Tools/mingw1310_64/bin/mingw32-make.exe");
}

static int lm_build_has_qt_gcc(struct LmMessageThread *lm_lmx_message_thread) {
    (void)lm_lmx_message_thread;
    return lm_build_file_exists(lm_lmx_message_thread, "C:/Qt/Tools/mingw1310_64/bin/gcc.exe");
}

static int lm_build_has_qt_gxx(struct LmMessageThread *lm_lmx_message_thread) {
    (void)lm_lmx_message_thread;
    return lm_build_file_exists(lm_lmx_message_thread, "C:/Qt/Tools/mingw1310_64/bin/g++.exe");
}

static char * lm_build_platform_canary_command_format(struct LmMessageThread *lm_lmx_message_thread) {
    (void)lm_lmx_message_thread;
    return "powershell -NoProfile -ExecutionPolicy Bypass -File \"%s\"";
}

static char * lm_build_platform_canary_script_path(struct LmMessageThread *lm_lmx_message_thread) {
    (void)lm_lmx_message_thread;
    return "build/obj/run_lm0_canary.ps1";
}

static char * lm_build_platform_canary_command_path(struct LmMessageThread *lm_lmx_message_thread, char *canary_path) {
    (void)lm_lmx_message_thread;
    LM_UNUSED(canary_path);
    return lm_build_platform_canary_script_path(lm_lmx_message_thread);
}

static int lm_build_prepare_platform_canary(struct LmMessageThread *lm_lmx_message_thread, char *canary_path) {
    (void)lm_lmx_message_thread;
    FILE * file;
    file = fopen(lm_build_platform_canary_script_path(lm_lmx_message_thread), "wb");
    if (file == 0) {
        fprintf(stderr, "buildCore.lm0: cannot write canary script %s\n", lm_build_platform_canary_script_path(lm_lmx_message_thread));
        return 1;
    }
    fprintf(file, "$exe = (Resolve-Path -LiteralPath '%s').Path\n", canary_path);
    fputs("$p = Start-Process -FilePath $exe -ArgumentList '--next' -WorkingDirectory (Get-Location).Path -PassThru\n", file);
    fputs("if (-not $p.WaitForExit(600000)) { $p.Kill(); exit 124 }\n", file);
    fputs("exit $p.ExitCode\n", file);
    if (fclose(file) != 0) {
        fprintf(stderr, "buildCore.lm0: cannot close canary script %s\n", lm_build_platform_canary_script_path(lm_lmx_message_thread));
        return 1;
    }
    return 0;
}

static char * lm_build_platform_tests_script_path(struct LmMessageThread *lm_lmx_message_thread) {
    (void)lm_lmx_message_thread;
    return "build/obj/tests/run_lm0_tests.ps1";
}

static char * lm_build_platform_tests_command_format(struct LmMessageThread *lm_lmx_message_thread) {
    (void)lm_lmx_message_thread;
    return "powershell -NoProfile -ExecutionPolicy Bypass -File \"%s\"";
}

static int lm_build_setenv(struct LmMessageThread *lm_lmx_message_thread, char *name, char *value) {
    (void)lm_lmx_message_thread;
    char assignment[256];
    if (name == 0) {
        return 1;
    }
    if (value == 0) {
        value = "";
    }
    snprintf(assignment, sizeof(assignment), "%s=%s", name, value);
    return _putenv(assignment);
}

static int lm_build_unsetenv(struct LmMessageThread *lm_lmx_message_thread, char *name) {
    (void)lm_lmx_message_thread;
    char assignment[256];
    if (name == 0) {
        return 1;
    }
    snprintf(assignment, sizeof(assignment), "%s=", name);
    return _putenv(assignment);
}

static int lm_build_write_platform_tests_script(struct LmMessageThread *lm_lmx_message_thread, FILE *file, char *output_dir, char *parser_library, char *own_library) {
    (void)lm_lmx_message_thread;
    fputs("$ErrorActionPreference = 'Continue'\n", file);
    fputs("$lmTestSuite = $env:LM_TEST_SUITE\n", file);
    fputs("if (-not $lmTestSuite) { $lmTestSuite = 'full' }\n", file);
    fputs("if ('smoke','core','full' -notcontains $lmTestSuite) { throw ('unknown LM_TEST_SUITE: ' + $lmTestSuite) }\n", file);
    fputs("$lmSmokeTrans = 'trans_integer_add.lm2','trans_include_receiver.lm2','trans_registry_view_parity.lmx','trans_getenv_index_probe.lm2','trans_invalid_anonymous_structure_shape.lm2'\n", file);
    fputs("Write-Host ('lm0 staged tests suite: ' + $lmTestSuite)\n", file);
    fputs("function Invoke-LmTestWithTimeout([string]$Path, [string]$Name) {\n", file);
    fputs("    $process = Start-Process -FilePath (Resolve-Path -LiteralPath $Path).Path -WorkingDirectory (Get-Location).Path -PassThru\n", file);
    fputs("    if (-not $process.WaitForExit(30000)) { $process.Kill(); throw ($Name + ' timed out') }\n", file);
    fputs("    if ($process.ExitCode -ne 0) { throw ($Name + ' failed with exit ' + $process.ExitCode) }\n", file);
    fputs("}\n", file);
    fputs("$lmTestJobs = $env:LM_TEST_JOBS\n", file);
    fputs("if (-not $lmTestJobs) { $lmTestJobs = [Environment]::ProcessorCount }\n", file);
    fputs("try { $lmTestJobs = [int]$lmTestJobs } catch { $lmTestJobs = 4 }\n", file);
    fputs("if ($lmTestJobs -lt 1) { $lmTestJobs = 1 }\n", file);
    fputs("Write-Host ('lm0 test jobs: ' + $lmTestJobs)\n", file);
    fputs("function ConvertTo-LmProcessArguments([string[]]$ArgumentList) {\n", file);
    fputs("    ($ArgumentList | ForEach-Object {\n", file);
    fputs("        if ($null -eq $_ -or $_ -eq '') { return }\n", file);
    fputs("        if ($_ -notmatch '[ \\t\"]') { return $_ }\n", file);
    fputs("        '\"' + $_.Replace('\\', '\\\\').Replace('\"', '\\\"') + '\"'\n", file);
    fputs("    }) -join ' '\n", file);
    fputs("}\n", file);
    fputs("function Start-LmJobProcess($Job) {\n", file);
    fputs("    $psi = New-Object System.Diagnostics.ProcessStartInfo\n", file);
    fputs("    $psi.FileName = (Resolve-Path -LiteralPath $Job.File).Path\n", file);
    fputs("    $psi.Arguments = ConvertTo-LmProcessArguments @($Job.Args)\n", file);
    fputs("    $psi.UseShellExecute = $false\n", file);
    fputs("    $psi.WorkingDirectory = (Get-Location).Path\n", file);
    fputs("    if ($Job.Quiet) {\n", file);
    fputs("        $psi.RedirectStandardOutput = $true\n", file);
    fputs("        $psi.RedirectStandardError = $true\n", file);
    fputs("    }\n", file);
    fputs("    if ($Job.Env) {\n", file);
    fputs("        foreach ($key in $Job.Env.Keys) {\n", file);
    fputs("            $psi.EnvironmentVariables[$key] = [string]$Job.Env[$key]\n", file);
    fputs("        }\n", file);
    fputs("    }\n", file);
    fputs("    $process = New-Object System.Diagnostics.Process\n", file);
    fputs("    $process.StartInfo = $psi\n", file);
    fputs("    $null = $process.Start()\n", file);
    fputs("    return $process\n", file);
    fputs("}\n", file);
    fputs("function Invoke-LmJobQueue($Jobs) {\n", file);
    fputs("    if ($null -eq $Jobs -or $Jobs.Count -eq 0) { return }\n", file);
    fputs("    $running = New-Object 'System.Collections.Generic.List[object]'\n", file);
    fputs("    $next = 0\n", file);
    fputs("    $failure = $null\n", file);
    fputs("    while ($next -lt $Jobs.Count -or $running.Count -gt 0) {\n", file);
    fputs("        while ($running.Count -lt $lmTestJobs -and $next -lt $Jobs.Count) {\n", file);
    fputs("            $job = $Jobs[$next]\n", file);
    fputs("            $next++\n", file);
    fputs("            $running.Add([pscustomobject]@{ Process = (Start-LmJobProcess $job); Job = $job })\n", file);
    fputs("        }\n", file);
    fputs("        Start-Sleep -Milliseconds 50\n", file);
    fputs("        $still = New-Object 'System.Collections.Generic.List[object]'\n", file);
    fputs("        foreach ($item in $running) {\n", file);
    fputs("            if (-not $item.Process.HasExited) { $still.Add($item); continue }\n", file);
    fputs("            if ($item.Job.Quiet) {\n", file);
    fputs("                $null = $item.Process.StandardOutput.ReadToEnd()\n", file);
    fputs("                $null = $item.Process.StandardError.ReadToEnd()\n", file);
    fputs("            }\n", file);
    fputs("            $expect = 0\n", file);
    fputs("            if ($null -ne $item.Job.Expect) { $expect = [int]$item.Job.Expect }\n", file);
    fputs("            if ($item.Process.ExitCode -ne $expect -and $null -eq $failure) {\n", file);
    fputs("                $failure = $item.Job.Stage + ': ' + $item.Job.Name + ' got ' + $item.Process.ExitCode\n", file);
    fputs("            }\n", file);
    fputs("            $item.Process.Dispose()\n", file);
    fputs("        }\n", file);
    fputs("        $running = $still\n", file);
    fputs("    }\n", file);
    fputs("    if ($null -ne $failure) { throw $failure }\n", file);
    fputs("}\n", file);
    fputs("$env:LM_P0_REGISTRY = 'lm2/parser_registry.lm2'\n", file);
    fprintf(file, "$printTree = '%s/printTree.lm0%s'\n", output_dir, lm_build_exe_suffix(lm_lmx_message_thread));
    fprintf(file, "$trans = '%s/trans.lm0%s'\n", output_dir, lm_build_exe_suffix(lm_lmx_message_thread));
    fprintf(file, "$make = '%s/make.lm0%s'\n", output_dir, lm_build_exe_suffix(lm_lmx_message_thread));
    fprintf(file, "$parserLib = '%s'\n", parser_library);
    fprintf(file, "$ownLib = '%s'\n", own_library);
    fputs("New-Item -ItemType Directory -Force 'build/obj/tests' | Out-Null\n", file);
    fputs("$registrySelftest = Join-Path 'build/obj/tests' 'parser_registry_source_tables_selftest.exe'\n", file);
    fputs("& $make 'link' '-std=c99' '-Wall' '-Wextra' '-Wpedantic' '-Werror' '-DLM_P0_REGISTRY_SELFTEST' '-Ilm1' 'lm1/build/parser.lm1.c' $ownLib '-o' $registrySelftest\n", file);
    fputs("if ($LASTEXITCODE -ne 0) { throw 'parser source-table selftest link failed' }\n", file);
    fputs("$previousP0Registry = $env:LM_P0_REGISTRY\n", file);
    fputs("try {\n", file);
    fputs("    $registryFixture = 'tests/fixtures/parser_registry_source_tables.lm2'\n", file);
    fputs("    $env:LM_P0_REGISTRY = $registryFixture\n", file);
    fputs("    & (Resolve-Path -LiteralPath $registrySelftest).Path\n", file);
    fputs("    if ($LASTEXITCODE -ne 0) { throw ('parser source-table selftest failed: ' + $registryFixture) }\n", file);
    fputs("    foreach ($invalidRegistryFixture in @('tests/fixtures/parser_registry_source_table_marker_order_invalid.lm2', 'tests/fixtures/parser_registry_source_table_marker_duplicate_invalid.lm2', 'tests/fixtures/parser_registry_source_join_marker_order_invalid.lm2', 'tests/fixtures/parser_registry_source_join_marker_duplicate_invalid.lm2')) {\n", file);
    fputs("        $env:LM_P0_REGISTRY = $invalidRegistryFixture\n", file);
    fputs("        & (Resolve-Path -LiteralPath $registrySelftest).Path *> $null\n", file);
    fputs("        $invalidRegistryCode = $LASTEXITCODE\n", file);
    fputs("        if ($invalidRegistryCode -ne 1) { throw ('invalid parser source marker expected exit 1: ' + $invalidRegistryFixture + ', got ' + $invalidRegistryCode) }\n", file);
    fputs("    }\n", file);
    fputs("    $legacyRegistryFixture = Join-Path 'build/obj/tests' 'parser_registry_source_tables.lm4'\n", file);
    fputs("    Copy-Item -LiteralPath $registryFixture -Destination $legacyRegistryFixture -Force\n", file);
    fputs("    try {\n", file);
    fputs("        $env:LM_P0_REGISTRY = $legacyRegistryFixture\n", file);
    fputs("        & (Resolve-Path -LiteralPath $registrySelftest).Path *> $null\n", file);
    fputs("        $legacyCode = $LASTEXITCODE\n", file);
    fputs("        if ($legacyCode -ne 1) { throw ('legacy .lm4 parser registry rejection expected exit 1, got ' + $legacyCode) }\n", file);
    fputs("    }\n", file);
    fputs("    finally { Remove-Item -LiteralPath $legacyRegistryFixture -Force -ErrorAction SilentlyContinue }\n", file);
    fputs("}\n", file);
    fputs("finally {\n", file);
    fputs("    if ($null -eq $previousP0Registry) { Remove-Item Env:LM_P0_REGISTRY -ErrorAction SilentlyContinue } else { $env:LM_P0_REGISTRY = $previousP0Registry }\n", file);
    fputs("}\n", file);
    fputs("$tableSourceMarkerC = Join-Path 'build/obj/tests' 'table_source_marker_roundtrip.c'\n", file);
    fputs("$tableSourceMarkerExe = Join-Path 'build/obj/tests' 'table_source_marker_roundtrip.exe'\n", file);
    fputs("& $trans 'lm2/table.lm2' $tableSourceMarkerC\n", file);
    fputs("if ($LASTEXITCODE -ne 0) { throw 'table source-marker roundtrip translation failed' }\n", file);
    fputs("& $make 'link' '-std=c99' '-Wall' '-Wextra' '-Wpedantic' '-Werror' '-Ilm1' $tableSourceMarkerC $parserLib $ownLib '-o' $tableSourceMarkerExe\n", file);
    fputs("if ($LASTEXITCODE -ne 0) { throw 'table source-marker roundtrip link failed' }\n", file);
    fputs("& (Resolve-Path -LiteralPath 'tests/table_source_marker_roundtrip.ps1').Path -TableTool (Resolve-Path -LiteralPath $tableSourceMarkerExe).Path\n", file);
    fputs("$unsupportedSourceFixture = Join-Path 'build/obj/tests' 'unsupported_source.lm4'\n", file);
    fputs("$unsupportedSourceOutput = Join-Path 'build/obj/tests' 'unsupported_source.c'\n", file);
    fputs("Copy-Item -LiteralPath 'tests/trans_mix_ignored.lm2' -Destination $unsupportedSourceFixture -Force -ErrorAction Stop\n", file);
    fputs("try {\n", file);
    fputs("    & $trans $unsupportedSourceFixture $unsupportedSourceOutput *> $null\n", file);
    fputs("    $unsupportedSourceCode = $LASTEXITCODE\n", file);
    fputs("    if ($unsupportedSourceCode -ne 1) { throw ('legacy .lm4 source rejection expected exit 1, got ' + $unsupportedSourceCode) }\n", file);
    fputs("}\n", file);
    fputs("finally {\n", file);
    fputs("    Remove-Item -LiteralPath $unsupportedSourceFixture -Force -ErrorAction SilentlyContinue\n", file);
    fputs("    Remove-Item -LiteralPath $unsupportedSourceOutput -Force -ErrorAction SilentlyContinue\n", file);
    fputs("}\n", file);
    fputs("$unsupportedTransRegistry = Join-Path 'build/obj/tests' 'unsupported_trans_registry.lm4'\n", file);
    fputs("$unsupportedTransRegistryOutput = Join-Path 'build/obj/tests' 'unsupported_trans_registry.c'\n", file);
    fputs("Copy-Item -LiteralPath 'lm2/trans_registry.lm2' -Destination $unsupportedTransRegistry -Force -ErrorAction Stop\n", file);
    fputs("$previousTransRegistry = $env:LM_TRANS_REGISTRY\n", file);
    fputs("try {\n", file);
    fputs("    $env:LM_TRANS_REGISTRY = $unsupportedTransRegistry\n", file);
    fputs("    & $trans 'tests/trans_include_receiver.lm2' $unsupportedTransRegistryOutput *> $null\n", file);
    fputs("    $unsupportedTransRegistryCode = $LASTEXITCODE\n", file);
    fputs("    if ($unsupportedTransRegistryCode -ne 1) { throw ('legacy .lm4 trans registry rejection expected exit 1, got ' + $unsupportedTransRegistryCode) }\n", file);
    fputs("}\n", file);
    fputs("finally {\n", file);
    fputs("    if ($null -eq $previousTransRegistry) { Remove-Item Env:LM_TRANS_REGISTRY -ErrorAction SilentlyContinue } else { $env:LM_TRANS_REGISTRY = $previousTransRegistry }\n", file);
    fputs("    Remove-Item -LiteralPath $unsupportedTransRegistry -Force -ErrorAction SilentlyContinue\n", file);
    fputs("    Remove-Item -LiteralPath $unsupportedTransRegistryOutput -Force -ErrorAction SilentlyContinue\n", file);
    fputs("}\n", file);
    fputs("$unsupportedUnitPayload = Join-Path 'build/obj/tests' 'unsupported_unit_payload.lm4'\n", file);
    fputs("$unsupportedUnitPayloadOutput = Join-Path 'build/obj/tests' 'unsupported_unit_payload.c'\n", file);
    fputs("Copy-Item -LiteralPath 'tests/fixtures/namespace_l2_payload_import.lm2' -Destination $unsupportedUnitPayload -Force -ErrorAction Stop\n", file);
    fputs("try {\n", file);
    fputs("    & $trans 'tests/fixtures/unsupported_unit_payload_import_probe.lm2' $unsupportedUnitPayloadOutput *> $null\n", file);
    fputs("    $unsupportedUnitPayloadCode = $LASTEXITCODE\n", file);
    fputs("    if ($unsupportedUnitPayloadCode -ne 1) { throw ('legacy .lm4 unit payload rejection expected exit 1, got ' + $unsupportedUnitPayloadCode) }\n", file);
    fputs("}\n", file);
    fputs("finally {\n", file);
    fputs("    Remove-Item -LiteralPath $unsupportedUnitPayload -Force -ErrorAction SilentlyContinue\n", file);
    fputs("    Remove-Item -LiteralPath $unsupportedUnitPayloadOutput -Force -ErrorAction SilentlyContinue\n", file);
    fputs("}\n", file);
    fputs("$parserSkip = @()\n", file);
    fputs("$transSkip = @()\n", file);
    fputs("$transTranslationOnly = @('trans_l4_abi_receivers.lm2', 'trans_message_thread_pool_single.lm2', 'trans_message_thread_mailbox_single.lm2')\n", file);
    fprintf(file, "$transLib = '%s/libtrans.lm0.a'\n", output_dir);
    fputs("if (-not (Test-Path -LiteralPath $transLib -PathType Leaf)) {\n", file);
    fputs("    & $make 'cc' '-std=c99' '-Wall' '-Wextra' '-Wpedantic' '-Ilm1' '-c' 'lm1/build/trans_library.lm1.c' '-o' 'build/obj/trans_library.lm1.o'\n", file);
    fputs("    if ($LASTEXITCODE -ne 0) { throw 'libtrans compile failed' }\n", file);
    fputs("    & $make 'ar' 'rcs' $transLib 'build/obj/trans_library.lm1.o'\n", file);
    fputs("    if ($LASTEXITCODE -ne 0) { throw 'libtrans archive failed' }\n", file);
    fputs("    & $make 'ranlib' $transLib\n", file);
    fputs("    if ($LASTEXITCODE -ne 0) { throw 'libtrans ranlib failed' }\n", file);
    fputs("}\n", file);
    fputs("foreach ($testFile in Get-ChildItem -LiteralPath 'tests' -File -Filter '*.lmx' | Sort-Object Name) {\n", file);
    fputs("    if ($testFile.Name -like 'trans_*') { continue }\n", file);
    fputs("    if ($parserSkip -contains $testFile.Name) { continue }\n", file);
    fputs("    & $printTree $testFile.FullName *> $null\n", file);
    fputs("    $code = $LASTEXITCODE\n", file);
    fputs("    if ($testFile.Name -like 'invalid_*') {\n", file);
    fputs("        if ($code -eq 0) { throw ('negative parser test unexpectedly passed: ' + $testFile.Name) }\n", file);
    fputs("    }\n", file);
    fputs("    elseif ($code -ne 0) { throw ('positive parser test failed: ' + $testFile.Name) }\n", file);
    fputs("}\n", file);
    fputs("$transJobs = New-Object 'System.Collections.Generic.List[object]'\n", file);
    fputs("$linkJobs = New-Object 'System.Collections.Generic.List[object]'\n", file);
    fputs("$runJobs = New-Object 'System.Collections.Generic.List[object]'\n", file);
    fputs("$parityLegacyPath = $null\n", file);
    fputs("$parityViewPath = $null\n", file);
    fputs("foreach ($testFile in Get-ChildItem -LiteralPath 'tests' -File | Where-Object { $_.Name -like 'trans_*' -and ($_.Extension -eq '.lm2' -or $_.Extension -eq '.lmx') } | Sort-Object Name) {\n", file);
    fputs("    if ($transSkip -contains $testFile.Name) { continue }\n", file);
    fputs("    if ($lmTestSuite -eq 'smoke' -and $lmSmokeTrans -notcontains $testFile.Name) { continue }\n", file);
    fputs("    $cPath = Join-Path 'build/obj/tests' ($testFile.BaseName + '.c')\n", file);
    fputs("    $exePath = Join-Path 'build/obj/tests' ($testFile.BaseName + '.exe')\n", file);
    fputs("    if ($testFile.Name -like 'trans_invalid_*') {\n", file);
    fputs("        $viewModes = @('view')\n", file);
    fputs("        if ($lmTestSuite -eq 'full') { $viewModes = @('legacy','1','view') }\n", file);
    fputs("        foreach ($viewMode in $viewModes) {\n", file);
    fputs("            $viewPath = $cPath + '.' + $viewMode\n", file);
    fputs("            $transJobs.Add([pscustomobject]@{ File = $trans; Args = @($testFile.FullName, $viewPath); Env = @{ LM_TRANS_REGISTRY_VIEW = $viewMode; LM_TRANS_LINK_LIBRARY = '0' }; Expect = 1; Quiet = $true; Name = ($testFile.Name + ' [' + $viewMode + ']'); Stage = 'negative trans test expected exit 1' })\n", file);
    fputs("        }\n", file);
    fputs("        continue\n", file);
    fputs("    }\n", file);
    fputs("    $extraLibs = @($parserLib, $ownLib)\n", file);
    fputs("    $transEnv = @{ LM_TRANS_REGISTRY_VIEW = 'view'; LM_TRANS_LINK_LIBRARY = '0' }\n", file);
    fputs("    if (Select-String -LiteralPath $testFile.FullName -Pattern 'import: \"lm2/trans_library.lm2\"' -Quiet) {\n", file);
    fputs("        $transEnv['LM_TRANS_LINK_LIBRARY'] = '1'\n", file);
    fputs("        $extraLibs = @($transLib, $parserLib, $ownLib)\n", file);
    fputs("    }\n", file);
    fputs("    if ($testFile.Name -eq 'trans_registry_view_parity.lmx') {\n", file);
    fputs("        $parityLegacyPath = $cPath + '.legacy'\n", file);
    fputs("        $parityViewPath = $cPath\n", file);
    fputs("        $legacyEnv = @{ LM_TRANS_REGISTRY_VIEW = 'legacy'; LM_TRANS_LINK_LIBRARY = $transEnv['LM_TRANS_LINK_LIBRARY'] }\n", file);
    fputs("        $transJobs.Add([pscustomobject]@{ File = $trans; Args = @($testFile.FullName, $parityLegacyPath); Env = $legacyEnv; Expect = 0; Quiet = $false; Name = 'legacy registry parity'; Stage = 'trans smoke translation failed' })\n", file);
    fputs("        $transJobs.Add([pscustomobject]@{ File = $trans; Args = @($testFile.FullName, $cPath); Env = $transEnv; Expect = 0; Quiet = $false; Name = 'new registry view parity'; Stage = 'trans smoke translation failed' })\n", file);
    fputs("    }\n", file);
    fputs("    else {\n", file);
    fputs("        $transJobs.Add([pscustomobject]@{ File = $trans; Args = @($testFile.FullName, $cPath); Env = $transEnv; Expect = 0; Quiet = $false; Name = $testFile.Name; Stage = 'trans smoke translation failed' })\n", file);
    fputs("    }\n", file);
    fputs("    if ($transTranslationOnly -contains $testFile.Name) { continue }\n", file);
    fputs("    $supportCPath = Join-Path 'tests' ($testFile.BaseName + '.support.c')\n", file);
    fputs("    $supportCSources = @()\n", file);
    fputs("    if (Test-Path -LiteralPath $supportCPath -PathType Leaf) { $supportCSources = @($supportCPath) }\n", file);
    fputs("    $warningErrorArgs = @()\n", file);
    fputs("    if ($testFile.Name -eq 'trans_l2_predef_conditional_descriptor.lm2') { $warningErrorArgs = @('-Werror') }\n", file);
    fputs("    $linkArgs = @('link', '-std=c99', '-Wall', '-Wextra', '-Wpedantic') + $warningErrorArgs + @('-Ilm1', '-Itests', $cPath) + $supportCSources + $extraLibs + @('-o', $exePath)\n", file);
    fputs("    $linkJobs.Add([pscustomobject]@{ File = $make; Args = $linkArgs; Env = @{}; Expect = 0; Quiet = $false; Name = $testFile.Name; Stage = 'trans smoke link failed' })\n", file);
    fputs("    $runJobs.Add($exePath)\n", file);
    fputs("    if ($testFile.Name -eq 'trans_l2_predef_conditional_descriptor.lm2') {\n", file);
    fputs("        $branchExePath = Join-Path 'build/obj/tests' ($testFile.BaseName + '_branch.exe')\n", file);
    fputs("        $branchArgs = @('link', '-std=c99', '-Wall', '-Wextra', '-Wpedantic', '-Werror', '-DLM_TEST_PREDEF_DESCRIPTOR_BRANCH=1', '-Ilm1', '-Itests', $cPath) + $supportCSources + @($parserLib, $ownLib) + @('-o', $branchExePath)\n", file);
    fputs("        $linkJobs.Add([pscustomobject]@{ File = $make; Args = $branchArgs; Env = @{}; Expect = 0; Quiet = $false; Name = 'conditional predef descriptor branch'; Stage = 'trans smoke link failed' })\n", file);
    fputs("        $runJobs.Add($branchExePath)\n", file);
    fputs("    }\n", file);
    fputs("}\n", file);
    fputs("Invoke-LmJobQueue $transJobs\n", file);
    fputs("if ($parityLegacyPath) {\n", file);
    fputs("    if ((Get-FileHash -Algorithm SHA256 $parityLegacyPath).Hash -ne (Get-FileHash -Algorithm SHA256 $parityViewPath).Hash) { throw 'legacy/new registry outputs differ' }\n", file);
    fputs("    Remove-Item -LiteralPath $parityLegacyPath -Force\n", file);
    fputs("}\n", file);
    fputs("Invoke-LmJobQueue $linkJobs\n", file);
    fputs("foreach ($exePath in $runJobs) {\n", file);
    fputs("    & (Resolve-Path -LiteralPath $exePath).Path\n", file);
    fputs("    if ($LASTEXITCODE -ne 0) { throw ('trans smoke run failed: ' + [IO.Path]::GetFileName($exePath)) }\n", file);
    fputs("}\n", file);
    fputs("if ($lmTestSuite -eq 'full') {\n", file);
    fputs("$httpClientDefaultC = Join-Path 'build/obj/tests' 'trans_rest_lmx_http_client_default.c'\n", file);
    fputs("$httpClientDefaultTest = Join-Path 'build/obj/tests' 'trans_rest_lmx_http_client_default.exe'\n", file);
    fputs("& $make 'link' '-std=c99' '-Wall' '-Wextra' '-Wpedantic' '-Werror' '-DLM_REST_LMX_INSTALL_DEFAULT_CLIENT=1' '-Ilm1' $httpClientDefaultC 'tests/trans_rest_lmx_http_client_default_stub.c' $parserLib $ownLib '-o' $httpClientDefaultTest\n", file);
    fputs("if ($LASTEXITCODE -ne 0) { throw 'REST/LMX default-client lowering test link failed' }\n", file);
    fputs("Invoke-LmTestWithTimeout $httpClientDefaultTest 'REST/LMX default-client lowering test'\n", file);
    fputs("$previousInstallFailure = $env:LM_TEST_REST_LMX_INSTALL_FAIL\n", file);
    fputs("try {\n", file);
    fputs("    $env:LM_TEST_REST_LMX_INSTALL_FAIL = '1'\n", file);
    fputs("    & (Resolve-Path -LiteralPath $httpClientDefaultTest).Path\n", file);
    fputs("    if ($LASTEXITCODE -ne 1) { throw ('REST/LMX installer failure returned ' + $LASTEXITCODE + ', expected 1') }\n", file);
    fputs("}\n", file);
    fputs("finally {\n", file);
    fputs("    if ($null -eq $previousInstallFailure) { Remove-Item Env:LM_TEST_REST_LMX_INSTALL_FAIL -ErrorAction SilentlyContinue } else { $env:LM_TEST_REST_LMX_INSTALL_FAIL = $previousInstallFailure }\n", file);
    fputs("}\n", file);
    fputs("$restLmxNoneTest = Join-Path 'build/obj/tests' 'rest_lmx_http_client_none.exe'\n", file);
    fputs("& $make 'link' '-std=c99' '-Wall' '-Wextra' '-Wpedantic' '-Werror' '-DLM_REST_LMX_CLIENT_PROVIDER=0' '-Ilm1' 'lm1/build/rest_lmx_http_client.lm1.c' 'tests/rest_lmx_http_client_none.c' $ownLib '-o' $restLmxNoneTest\n", file);
    fputs("if ($LASTEXITCODE -ne 0) { throw 'REST/LMX none-client test link failed' }\n", file);
    fputs("Invoke-LmTestWithTimeout $restLmxNoneTest 'REST/LMX none-client test'\n", file);
    fputs("$restLmxServerNoneTest = Join-Path 'build/obj/tests' 'rest_lmx_http_server_none.exe'\n", file);
    fputs("& $make 'link' '-std=c99' '-Wall' '-Wextra' '-Wpedantic' '-Werror' '-DLM_REST_LMX_SERVER_PROVIDER=0' '-Ilm1' 'lm1/build/rest_lmx_http_server.lm1.c' 'tests/rest_lmx_http_server_none.c' $ownLib '-o' $restLmxServerNoneTest\n", file);
    fputs("if ($LASTEXITCODE -ne 0) { throw 'REST/LMX none-server test link failed' }\n", file);
    fputs("Invoke-LmTestWithTimeout $restLmxServerNoneTest 'REST/LMX none-server test'\n", file);
    fputs("$httpServerDefaultC = Join-Path 'build/obj/tests' 'rest_lmx_http_server_default_source.c'\n", file);
    fputs("$httpServerDefaultTest = Join-Path 'build/obj/tests' 'rest_lmx_http_server_default.exe'\n", file);
    fputs("& $trans 'tests/rest_lmx_http_server_default_source.lm2' $httpServerDefaultC\n", file);
    fputs("if ($LASTEXITCODE -ne 0) { throw 'REST/LMX default-server lowering test translation failed' }\n", file);
    fputs("& $make 'link' '-std=c99' '-Wall' '-Wextra' '-Wpedantic' '-Werror' '-DLM_REST_LMX_INSTALL_DEFAULT_SERVER=1' '-Ilm1' $httpServerDefaultC 'tests/trans_rest_lmx_http_server_default_stub.c' $parserLib $ownLib '-o' $httpServerDefaultTest\n", file);
    fputs("if ($LASTEXITCODE -ne 0) { throw 'REST/LMX default-server lowering test link failed' }\n", file);
    fputs("Invoke-LmTestWithTimeout $httpServerDefaultTest 'REST/LMX default-server lowering test'\n", file);
    fputs("$previousServerStartFailure = $env:LM_TEST_REST_LMX_SERVER_START_FAIL\n", file);
    fputs("try {\n", file);
    fputs("    $env:LM_TEST_REST_LMX_SERVER_START_FAIL = '1'\n", file);
    fputs("    & (Resolve-Path -LiteralPath $httpServerDefaultTest).Path\n", file);
    fputs("    if ($LASTEXITCODE -ne 1) { throw ('REST/LMX server start failure returned ' + $LASTEXITCODE + ', expected 1') }\n", file);
    fputs("}\n", file);
    fputs("finally {\n", file);
    fputs("    if ($null -eq $previousServerStartFailure) { Remove-Item Env:LM_TEST_REST_LMX_SERVER_START_FAIL -ErrorAction SilentlyContinue } else { $env:LM_TEST_REST_LMX_SERVER_START_FAIL = $previousServerStartFailure }\n", file);
    fputs("}\n", file);
    fputs("$httpServerNoneC = Join-Path 'build/obj/tests' 'rest_lmx_http_server_none_source.c'\n", file);
    fputs("$httpServerNoneLifecycleTest = Join-Path 'build/obj/tests' 'rest_lmx_http_server_none_lifecycle.exe'\n", file);
    fputs("& $trans 'tests/rest_lmx_http_server_none_source.lm2' $httpServerNoneC\n", file);
    fputs("if ($LASTEXITCODE -ne 0) { throw 'REST/LMX none-server lifecycle translation failed' }\n", file);
    fputs("& $make 'link' '-std=c99' '-Wall' '-Wextra' '-Wpedantic' '-Werror' '-DLM_REST_LMX_INSTALL_DEFAULT_SERVER=1' '-Ilm1' $httpServerNoneC 'tests/trans_rest_lmx_http_server_none_stub.c' $parserLib $ownLib '-o' $httpServerNoneLifecycleTest\n", file);
    fputs("if ($LASTEXITCODE -ne 0) { throw 'REST/LMX none-server lifecycle test link failed' }\n", file);
    fputs("Invoke-LmTestWithTimeout $httpServerNoneLifecycleTest 'REST/LMX none-server lifecycle test'\n", file);
    fputs("$previousThreadProvider = $env:LM_THREAD_PROVIDER\n", file);
    fputs("try {\n", file);
    fputs("    $singlePoolTest = Join-Path 'build/obj/tests' 'trans_message_thread_pool_single.exe'\n", file);
    fputs("    $env:LM_THREAD_PROVIDER = 'single'\n", file);
    fputs("    & $make 'link' '-std=c99' '-Wall' '-Wextra' '-Wpedantic' '-Werror' '-Ilm1' 'lm1/build/own.lm1.c' 'build/obj/tests/trans_message_thread_pool_single.c' '-o' $singlePoolTest\n", file);
    fputs("    if ($LASTEXITCODE -ne 0) { throw 'single MessageThread pool test link failed' }\n", file);
    fputs("    Invoke-LmTestWithTimeout $singlePoolTest 'single MessageThread pool test'\n", file);
    fputs("    $singleMailboxTest = Join-Path 'build/obj/tests' 'trans_message_thread_mailbox_single.exe'\n", file);
    fputs("    & $make 'link' '-std=c99' '-Wall' '-Wextra' '-Wpedantic' '-Werror' '-Ilm1' 'lm1/build/own.lm1.c' 'build/obj/tests/trans_message_thread_mailbox_single.c' '-o' $singleMailboxTest\n", file);
    fputs("    if ($LASTEXITCODE -ne 0) { throw 'single MessageThread mailbox test link failed' }\n", file);
    fputs("    Invoke-LmTestWithTimeout $singleMailboxTest 'single MessageThread mailbox test'\n", file);
    fputs("    $singleRootDrainTest = Join-Path 'build/obj/tests' 'message_thread_root_drain_single.exe'\n", file);
    fputs("    & $make 'link' '-std=c99' '-Wall' '-Wextra' '-Wpedantic' '-Werror' '-Ilm1' 'lm1/build/own.lm1.c' 'tests/message_thread_root_drain.c' '-o' $singleRootDrainTest\n", file);
    fputs("    if ($LASTEXITCODE -ne 0) { throw 'single MessageThread root/drain test link failed' }\n", file);
    fputs("    Invoke-LmTestWithTimeout $singleRootDrainTest 'single MessageThread root/drain test'\n", file);
    fputs("    $singleApplicationExitTest = Join-Path 'build/obj/tests' 'message_thread_application_exit_single.exe'\n", file);
    fputs("    & $make 'link' '-std=c99' '-Wall' '-Wextra' '-Wpedantic' '-Werror' '-Ilm1' 'lm1/build/own.lm1.c' 'tests/message_thread_application_exit.c' '-o' $singleApplicationExitTest\n", file);
    fputs("    if ($LASTEXITCODE -ne 0) { throw 'single MessageThread application exit test link failed' }\n", file);
    fputs("    Invoke-LmTestWithTimeout $singleApplicationExitTest 'single MessageThread application exit test'\n", file);
    fputs("    $singleRestLmxProviderTest = Join-Path 'build/obj/tests' 'message_thread_rest_lmx_provider_single.exe'\n", file);
    fputs("    & $make 'link' '-std=c99' '-Wall' '-Wextra' '-Wpedantic' '-Werror' '-Ilm1' 'lm1/build/own.lm1.c' 'tests/message_thread_rest_lmx_provider.c' '-o' $singleRestLmxProviderTest\n", file);
    fputs("    if ($LASTEXITCODE -ne 0) { throw 'single REST/LMX provider test link failed' }\n", file);
    fputs("    Invoke-LmTestWithTimeout $singleRestLmxProviderTest 'single REST/LMX provider test'\n", file);
    fputs("    $singleRestLmxIngressTest = Join-Path 'build/obj/tests' 'message_thread_rest_lmx_ingress_single.exe'\n", file);
    fputs("    & $make 'link' '-std=c99' '-Wall' '-Wextra' '-Wpedantic' '-Werror' '-Ilm1' 'lm1/build/own.lm1.c' 'tests/message_thread_rest_lmx_ingress.c' '-o' $singleRestLmxIngressTest\n", file);
    fputs("    if ($LASTEXITCODE -ne 0) { throw 'single REST/LMX ingress test link failed' }\n", file);
    fputs("    Invoke-LmTestWithTimeout $singleRestLmxIngressTest 'single REST/LMX ingress test'\n", file);
    fputs("    $nativePoolTest = Join-Path 'build/obj/tests' 'message_thread_pool_native.exe'\n", file);
    fputs("    $env:LM_THREAD_PROVIDER = 'win32'\n", file);
    fputs("    & $make 'link' '-std=c99' '-Wall' '-Wextra' '-Wpedantic' '-Werror' '-Ilm1' 'lm1/build/own.lm1.c' 'tests/message_thread_pool_native.c' '-o' $nativePoolTest\n", file);
    fputs("    if ($LASTEXITCODE -ne 0) { throw 'Win32 MessageThread pool test link failed' }\n", file);
    fputs("    Invoke-LmTestWithTimeout $nativePoolTest 'Win32 MessageThread pool test'\n", file);
    fputs("    $nativeMailboxTest = Join-Path 'build/obj/tests' 'message_thread_mailbox_native.exe'\n", file);
    fputs("    & $make 'link' '-std=c99' '-Wall' '-Wextra' '-Wpedantic' '-Werror' '-Ilm1' 'lm1/build/own.lm1.c' 'tests/message_thread_mailbox_native.c' '-o' $nativeMailboxTest\n", file);
    fputs("    if ($LASTEXITCODE -ne 0) { throw 'Win32 MessageThread mailbox test link failed' }\n", file);
    fputs("    Invoke-LmTestWithTimeout $nativeMailboxTest 'Win32 MessageThread mailbox test'\n", file);
    fputs("    $nativeRootDrainTest = Join-Path 'build/obj/tests' 'message_thread_root_drain_native.exe'\n", file);
    fputs("    & $make 'link' '-std=c99' '-Wall' '-Wextra' '-Wpedantic' '-Werror' '-Ilm1' 'lm1/build/own.lm1.c' 'tests/message_thread_root_drain.c' '-o' $nativeRootDrainTest\n", file);
    fputs("    if ($LASTEXITCODE -ne 0) { throw 'Win32 MessageThread root/drain test link failed' }\n", file);
    fputs("    Invoke-LmTestWithTimeout $nativeRootDrainTest 'Win32 MessageThread root/drain test'\n", file);
    fputs("    $nativeApplicationExitTest = Join-Path 'build/obj/tests' 'message_thread_application_exit_native.exe'\n", file);
    fputs("    & $make 'link' '-std=c99' '-Wall' '-Wextra' '-Wpedantic' '-Werror' '-Ilm1' 'lm1/build/own.lm1.c' 'tests/message_thread_application_exit.c' '-o' $nativeApplicationExitTest\n", file);
    fputs("    if ($LASTEXITCODE -ne 0) { throw 'Win32 MessageThread application exit test link failed' }\n", file);
    fputs("    Invoke-LmTestWithTimeout $nativeApplicationExitTest 'Win32 MessageThread application exit test'\n", file);
    fputs("    $nativeRestLmxProviderTest = Join-Path 'build/obj/tests' 'message_thread_rest_lmx_provider_native.exe'\n", file);
    fputs("    & $make 'link' '-std=c99' '-Wall' '-Wextra' '-Wpedantic' '-Werror' '-Ilm1' 'lm1/build/own.lm1.c' 'tests/message_thread_rest_lmx_provider.c' '-o' $nativeRestLmxProviderTest\n", file);
    fputs("    if ($LASTEXITCODE -ne 0) { throw 'Win32 REST/LMX provider test link failed' }\n", file);
    fputs("    Invoke-LmTestWithTimeout $nativeRestLmxProviderTest 'Win32 REST/LMX provider test'\n", file);
    fputs("    $nativeRestLmxIngressTest = Join-Path 'build/obj/tests' 'message_thread_rest_lmx_ingress_native.exe'\n", file);
    fputs("    & $make 'link' '-std=c99' '-Wall' '-Wextra' '-Wpedantic' '-Werror' '-Ilm1' 'lm1/build/own.lm1.c' 'tests/message_thread_rest_lmx_ingress.c' '-o' $nativeRestLmxIngressTest\n", file);
    fputs("    if ($LASTEXITCODE -ne 0) { throw 'Win32 REST/LMX ingress test link failed' }\n", file);
    fputs("    Invoke-LmTestWithTimeout $nativeRestLmxIngressTest 'Win32 REST/LMX ingress test'\n", file);
    fputs("}\n", file);
    fputs("finally {\n", file);
    fputs("    if ($null -eq $previousThreadProvider) { Remove-Item Env:LM_THREAD_PROVIDER -ErrorAction SilentlyContinue } else { $env:LM_THREAD_PROVIDER = $previousThreadProvider }\n", file);
    fputs("}\n", file);
    fputs("}\n", file);
    fputs("Write-Host ('lm0 staged tests passed: ' + $lmTestSuite)\n", file);
    return 0;
}
#else
#include <unistd.h>
static char * lm_build_exe_suffix(struct LmMessageThread *lm_lmx_message_thread);
static char * lm_build_path_sep(struct LmMessageThread *lm_lmx_message_thread);
static int lm_build_platform_chdir(struct LmMessageThread *lm_lmx_message_thread, char *path);
static char * lm_build_platform_getcwd(struct LmMessageThread *lm_lmx_message_thread, char *buffer, size_t size);
static int lm_build_platform_absolute(struct LmMessageThread *lm_lmx_message_thread, char *path);
static int lm_build_file_exists(struct LmMessageThread *lm_lmx_message_thread, char *path);
static int lm_build_has_qt_cmake(struct LmMessageThread *lm_lmx_message_thread);
static int lm_build_has_qt_mingw_make(struct LmMessageThread *lm_lmx_message_thread);
static int lm_build_has_qt_gcc(struct LmMessageThread *lm_lmx_message_thread);
static int lm_build_has_qt_gxx(struct LmMessageThread *lm_lmx_message_thread);
static char * lm_build_platform_canary_command_format(struct LmMessageThread *lm_lmx_message_thread);
static char * lm_build_platform_canary_command_path(struct LmMessageThread *lm_lmx_message_thread, char *canary_path);
static int lm_build_prepare_platform_canary(struct LmMessageThread *lm_lmx_message_thread, char *canary_path);
static char * lm_build_platform_tests_script_path(struct LmMessageThread *lm_lmx_message_thread);
static char * lm_build_platform_tests_command_format(struct LmMessageThread *lm_lmx_message_thread);
static int lm_build_setenv(struct LmMessageThread *lm_lmx_message_thread, char *name, char *value);
static int lm_build_unsetenv(struct LmMessageThread *lm_lmx_message_thread, char *name);
static int lm_build_write_platform_tests_script(struct LmMessageThread *lm_lmx_message_thread, FILE *file, char *output_dir, char *parser_library, char *own_library);

static char * lm_build_exe_suffix(struct LmMessageThread *lm_lmx_message_thread) {
    (void)lm_lmx_message_thread;
    return "";
}

static char * lm_build_path_sep(struct LmMessageThread *lm_lmx_message_thread) {
    (void)lm_lmx_message_thread;
    return "/";
}

static int lm_build_platform_chdir(struct LmMessageThread *lm_lmx_message_thread, char *path) {
    (void)lm_lmx_message_thread;
    return chdir(path);
}

static char * lm_build_platform_getcwd(struct LmMessageThread *lm_lmx_message_thread, char *buffer, size_t size) {
    (void)lm_lmx_message_thread;
    return getcwd(buffer, size);
}

static int lm_build_platform_absolute(struct LmMessageThread *lm_lmx_message_thread, char *path) {
    (void)lm_lmx_message_thread;
    return path[0] == '/';
}

static int lm_build_file_exists(struct LmMessageThread *lm_lmx_message_thread, char *path) {
    (void)lm_lmx_message_thread;
    FILE * file;
    file = fopen(path, "rb");
    if (file == 0) {
        return 0;
    }
    fclose(file);
    return 1;
}

static int lm_build_has_qt_cmake(struct LmMessageThread *lm_lmx_message_thread) {
    (void)lm_lmx_message_thread;
    return 0;
}

static int lm_build_has_qt_mingw_make(struct LmMessageThread *lm_lmx_message_thread) {
    (void)lm_lmx_message_thread;
    return 0;
}

static int lm_build_has_qt_gcc(struct LmMessageThread *lm_lmx_message_thread) {
    (void)lm_lmx_message_thread;
    return 0;
}

static int lm_build_has_qt_gxx(struct LmMessageThread *lm_lmx_message_thread) {
    (void)lm_lmx_message_thread;
    return 0;
}

static char * lm_build_platform_canary_command_format(struct LmMessageThread *lm_lmx_message_thread) {
    (void)lm_lmx_message_thread;
    return "sh -c 'if command -v timeout >/dev/null 2>&1; then timeout 120s \"$1\" --next; elif command -v gtimeout >/dev/null 2>&1; then gtimeout 120s \"$1\" --next; else \"$1\" --next; fi' sh \"%s\"";
}

static char * lm_build_platform_canary_command_path(struct LmMessageThread *lm_lmx_message_thread, char *canary_path) {
    (void)lm_lmx_message_thread;
    return canary_path;
}

static int lm_build_prepare_platform_canary(struct LmMessageThread *lm_lmx_message_thread, char *canary_path) {
    (void)lm_lmx_message_thread;
    LM_UNUSED(canary_path);
    return 0;
}

static char * lm_build_platform_tests_script_path(struct LmMessageThread *lm_lmx_message_thread) {
    (void)lm_lmx_message_thread;
    return "build/obj/tests/run_lm0_tests.sh";
}

static char * lm_build_platform_tests_command_format(struct LmMessageThread *lm_lmx_message_thread) {
    (void)lm_lmx_message_thread;
    return "sh \"%s\"";
}

static int lm_build_setenv(struct LmMessageThread *lm_lmx_message_thread, char *name, char *value) {
    (void)lm_lmx_message_thread;
    if (name == 0) {
        return 1;
    }
    if (value == 0) {
        value = "";
    }
    return setenv(name, value, 1);
}

static int lm_build_unsetenv(struct LmMessageThread *lm_lmx_message_thread, char *name) {
    (void)lm_lmx_message_thread;
    if (name == 0) {
        return 1;
    }
    unsetenv(name);
    return 0;
}

static int lm_build_write_platform_tests_script(struct LmMessageThread *lm_lmx_message_thread, FILE *file, char *output_dir, char *parser_library, char *own_library) {
    (void)lm_lmx_message_thread;
    fputs("set -eu\n", file);
    fputs("lm_test_suite=${LM_TEST_SUITE:-full}\n", file);
    fputs("if [ \"$lm_test_suite\" != smoke ] && [ \"$lm_test_suite\" != core ] && [ \"$lm_test_suite\" != full ]; then echo \"unknown LM_TEST_SUITE: $lm_test_suite\" >&2; exit 1; fi\n", file);
    fputs("lm_smoke_trans=' trans_integer_add.lm2 trans_include_receiver.lm2 trans_registry_view_parity.lmx trans_getenv_index_probe.lm2 trans_invalid_anonymous_structure_shape.lm2 '\n", file);
    fputs("echo \"lm0 staged tests suite: $lm_test_suite\"\n", file);
    fputs("lm_run_with_watchdog() {\n", file);
    fputs("    if command -v timeout >/dev/null 2>&1; then\n", file);
    fputs("        if timeout 30s \"$@\"; then return 0; else lm_watchdog_status=$?; return \"$lm_watchdog_status\"; fi\n", file);
    fputs("    fi\n", file);
    fputs("    if command -v gtimeout >/dev/null 2>&1; then\n", file);
    fputs("        if gtimeout 30s \"$@\"; then return 0; else lm_watchdog_status=$?; return \"$lm_watchdog_status\"; fi\n", file);
    fputs("    fi\n", file);
    fputs("    \"$@\" &\n", file);
    fputs("    lm_watchdog_test_pid=$!\n", file);
    fputs("    (\n", file);
    fputs("        sleep 30\n", file);
    fputs("        if kill -0 \"$lm_watchdog_test_pid\" 2>/dev/null; then\n", file);
    fputs("            echo 'message-thread pool test timed out' >&2\n", file);
    fputs("            kill -TERM \"$lm_watchdog_test_pid\" 2>/dev/null || true\n", file);
    fputs("            sleep 2\n", file);
    fputs("            kill -KILL \"$lm_watchdog_test_pid\" 2>/dev/null || true\n", file);
    fputs("        fi\n", file);
    fputs("    ) &\n", file);
    fputs("    lm_watchdog_pid=$!\n", file);
    fputs("    if wait \"$lm_watchdog_test_pid\"; then lm_watchdog_status=0; else lm_watchdog_status=$?; fi\n", file);
    fputs("    kill \"$lm_watchdog_pid\" 2>/dev/null || true\n", file);
    fputs("    wait \"$lm_watchdog_pid\" 2>/dev/null || true\n", file);
    fputs("    return \"$lm_watchdog_status\"\n", file);
    fputs("}\n", file);
    fprintf(file, "printTree='%s/printTree.lm0%s'\n", output_dir, lm_build_exe_suffix(lm_lmx_message_thread));
    fprintf(file, "trans='%s/trans.lm0%s'\n", output_dir, lm_build_exe_suffix(lm_lmx_message_thread));
    fprintf(file, "make_tool='%s/make.lm0%s'\n", output_dir, lm_build_exe_suffix(lm_lmx_message_thread));
    fprintf(file, "parserLib='%s'\n", parser_library);
    fprintf(file, "ownLib='%s'\n", own_library);
    fputs("export LM_P0_REGISTRY='lm2/parser_registry.lm2'\n", file);
    fputs("mkdir -p build/obj/tests\n", file);
    fputs("registry_selftest='build/obj/tests/parser_registry_source_tables_selftest'\n", file);
    fputs("\"$make_tool\" link -std=c99 -Wall -Wextra -Wpedantic -Werror -DLM_P0_REGISTRY_SELFTEST -Ilm1 lm1/build/parser.lm1.c \"$ownLib\" -o \"$registry_selftest\"\n", file);
    fputs("LM_P0_REGISTRY='tests/fixtures/parser_registry_source_tables.lm2' \"$registry_selftest\"\n", file);
    fputs("for invalid_registry_fixture in tests/fixtures/parser_registry_source_table_marker_order_invalid.lm2 tests/fixtures/parser_registry_source_table_marker_duplicate_invalid.lm2 tests/fixtures/parser_registry_source_join_marker_order_invalid.lm2 tests/fixtures/parser_registry_source_join_marker_duplicate_invalid.lm2; do\n", file);
    fputs("    if LM_P0_REGISTRY=\"$invalid_registry_fixture\" \"$registry_selftest\" >/dev/null 2>&1; then invalid_registry_code=0; else invalid_registry_code=$?; fi\n", file);
    fputs("    if [ \"$invalid_registry_code\" -ne 1 ]; then echo \"invalid parser source marker expected exit 1: $invalid_registry_fixture, got $invalid_registry_code\" >&2; exit 1; fi\n", file);
    fputs("done\n", file);
    fputs("legacy_registry_fixture='build/obj/tests/parser_registry_source_tables.lm4'\n", file);
    fputs("cp tests/fixtures/parser_registry_source_tables.lm2 \"$legacy_registry_fixture\"\n", file);
    fputs("if LM_P0_REGISTRY=\"$legacy_registry_fixture\" \"$registry_selftest\" >/dev/null 2>&1; then legacy_code=0; else legacy_code=$?; fi\n", file);
    fputs("rm -f \"$legacy_registry_fixture\"\n", file);
    fputs("if [ \"$legacy_code\" -ne 1 ]; then echo \"legacy .lm4 parser registry rejection expected exit 1, got $legacy_code\" >&2; exit 1; fi\n", file);
    fputs("table_source_marker_c='build/obj/tests/table_source_marker_roundtrip.c'\n", file);
    fputs("table_source_marker_test='build/obj/tests/table_source_marker_roundtrip'\n", file);
    fputs("\"$trans\" lm2/table.lm2 \"$table_source_marker_c\"\n", file);
    fputs("\"$make_tool\" link -std=c99 -Wall -Wextra -Wpedantic -Werror -Ilm1 \"$table_source_marker_c\" \"$parserLib\" \"$ownLib\" -o \"$table_source_marker_test\"\n", file);
    fputs("tests/table_source_marker_roundtrip.sh \"$table_source_marker_test\"\n", file);
    fputs("unsupported_source_fixture='build/obj/tests/unsupported_source.lm4'\n", file);
    fputs("unsupported_source_output='build/obj/tests/unsupported_source.c'\n", file);
    fputs("cp tests/trans_mix_ignored.lm2 \"$unsupported_source_fixture\"\n", file);
    fputs("if \"$trans\" \"$unsupported_source_fixture\" \"$unsupported_source_output\" >/dev/null 2>&1; then unsupported_source_code=0; else unsupported_source_code=$?; fi\n", file);
    fputs("rm -f \"$unsupported_source_fixture\" \"$unsupported_source_output\"\n", file);
    fputs("if [ \"$unsupported_source_code\" -ne 1 ]; then echo \"legacy .lm4 source rejection expected exit 1, got $unsupported_source_code\" >&2; exit 1; fi\n", file);
    fputs("unsupported_trans_registry='build/obj/tests/unsupported_trans_registry.lm4'\n", file);
    fputs("unsupported_trans_registry_output='build/obj/tests/unsupported_trans_registry.c'\n", file);
    fputs("cp lm2/trans_registry.lm2 \"$unsupported_trans_registry\"\n", file);
    fputs("if LM_TRANS_REGISTRY=\"$unsupported_trans_registry\" \"$trans\" tests/trans_include_receiver.lm2 \"$unsupported_trans_registry_output\" >/dev/null 2>&1; then unsupported_trans_registry_code=0; else unsupported_trans_registry_code=$?; fi\n", file);
    fputs("rm -f \"$unsupported_trans_registry\" \"$unsupported_trans_registry_output\"\n", file);
    fputs("if [ \"$unsupported_trans_registry_code\" -ne 1 ]; then echo \"legacy .lm4 trans registry rejection expected exit 1, got $unsupported_trans_registry_code\" >&2; exit 1; fi\n", file);
    fputs("unsupported_unit_payload='build/obj/tests/unsupported_unit_payload.lm4'\n", file);
    fputs("unsupported_unit_payload_output='build/obj/tests/unsupported_unit_payload.c'\n", file);
    fputs("cp tests/fixtures/namespace_l2_payload_import.lm2 \"$unsupported_unit_payload\"\n", file);
    fputs("if \"$trans\" tests/fixtures/unsupported_unit_payload_import_probe.lm2 \"$unsupported_unit_payload_output\" >/dev/null 2>&1; then unsupported_unit_payload_code=0; else unsupported_unit_payload_code=$?; fi\n", file);
    fputs("rm -f \"$unsupported_unit_payload\" \"$unsupported_unit_payload_output\"\n", file);
    fputs("if [ \"$unsupported_unit_payload_code\" -ne 1 ]; then echo \"legacy .lm4 unit payload rejection expected exit 1, got $unsupported_unit_payload_code\" >&2; exit 1; fi\n", file);
    fputs("for src in tests/*.lmx; do\n", file);
    fputs("    [ -e \"$src\" ] || continue\n", file);
    fputs("    name=${src##*/}\n", file);
    fputs("    case \"$name\" in trans_*) continue ;; esac\n", file);
    fputs("    if \"$printTree\" \"$src\" >/dev/null 2>&1; then code=0; else code=$?; fi\n", file);
    fputs("    case \"$name\" in\n", file);
    fputs("        invalid_*) if [ \"$code\" -eq 0 ]; then echo \"negative parser test unexpectedly passed: $name\" >&2; exit 1; fi ;;\n", file);
    fputs("        *) if [ \"$code\" -ne 0 ]; then echo \"positive parser test failed: $name\" >&2; exit 1; fi ;;\n", file);
    fputs("    esac\n", file);
    fputs("done\n", file);
    fprintf(file, "transLib='%s/libtrans.lm0.a'\n", output_dir);
    fputs("if [ ! -f \"$transLib\" ]; then\n", file);
    fputs("    \"$make_tool\" cc -std=c99 -Wall -Wextra -Wpedantic -Ilm1 -c lm1/build/trans_library.lm1.c -o build/obj/trans_library.lm1.o\n", file);
    fputs("    \"$make_tool\" ar rcs \"$transLib\" build/obj/trans_library.lm1.o\n", file);
    fputs("    \"$make_tool\" ranlib \"$transLib\"\n", file);
    fputs("fi\n", file);
    fputs("lm_test_jobs=${LM_TEST_JOBS:-}\n", file);
    fputs("if [ -z \"$lm_test_jobs\" ]; then\n", file);
    fputs("    if command -v nproc >/dev/null 2>&1; then lm_test_jobs=$(nproc)\n", file);
    fputs("    elif command -v sysctl >/dev/null 2>&1; then lm_test_jobs=$(sysctl -n hw.ncpu 2>/dev/null || echo 4)\n", file);
    fputs("    else lm_test_jobs=4\n", file);
    fputs("    fi\n", file);
    fputs("fi\n", file);
    fputs("lm_run_link_jobs() {\n", file);
    fputs("    lm_job_n=0\n", file);
    fputs("    lm_job_fail=0\n", file);
    fputs("    lm_job_pids=\n", file);
    fputs("    while IFS= read -r lm_job_cmd; do\n", file);
    fputs("        [ -n \"$lm_job_cmd\" ] || continue\n", file);
    fputs("        eval \"$lm_job_cmd\" &\n", file);
    fputs("        lm_job_pids=\"$lm_job_pids $!\"\n", file);
    fputs("        lm_job_n=$((lm_job_n + 1))\n", file);
    fputs("        if [ \"$lm_job_n\" -ge \"$lm_test_jobs\" ]; then\n", file);
    fputs("            for lm_job_pid in $lm_job_pids; do if wait \"$lm_job_pid\"; then :; else lm_job_fail=1; fi; done\n", file);
    fputs("            lm_job_n=0\n", file);
    fputs("            lm_job_pids=\n", file);
    fputs("        fi\n", file);
    fputs("    done\n", file);
    fputs("    for lm_job_pid in $lm_job_pids; do if wait \"$lm_job_pid\"; then :; else lm_job_fail=1; fi; done\n", file);
    fputs("    return \"$lm_job_fail\"\n", file);
    fputs("}\n", file);
    fputs(": > build/obj/tests/link_jobs\n", file);
    fputs(": > build/obj/tests/run_jobs\n", file);
    fputs("for src in tests/trans_*.lm2 tests/trans_*.lmx; do\n", file);
    fputs("    [ -e \"$src\" ] || continue\n", file);
    fputs("    name=${src##*/}\n", file);
    fputs("    if [ \"$lm_test_suite\" = smoke ]; then\n", file);
    fputs("        case \"$lm_smoke_trans\" in *\" $name \"*) ;; *) continue ;; esac\n", file);
    fputs("    fi\n", file);
    fputs("    base=${name%.*}\n", file);
    fputs("    c_path=\"build/obj/tests/$base.c\"\n", file);
    fputs("    exe_path=\"build/obj/tests/$base\"\n", file);
    fputs("    case \"$name\" in\n", file);
    fputs("        trans_invalid_*)\n", file);
    fputs("            lm_invalid_views=view\n", file);
    fputs("            if [ \"$lm_test_suite\" = full ]; then lm_invalid_views='legacy 1 view'; fi\n", file);
    fputs("            for view_mode in $lm_invalid_views; do\n", file);
    fputs("                rm -f \"$c_path\"\n", file);
    fputs("                if LM_TRANS_REGISTRY_VIEW=\"$view_mode\" \"$trans\" \"$src\" \"$c_path\" >/dev/null 2>&1; then code=0; else code=$?; fi\n", file);
    fputs("                if [ \"$code\" -ne 1 ]; then echo \"negative trans test expected exit 1: $name [$view_mode] got $code\" >&2; exit 1; fi\n", file);
    fputs("            done\n", file);
    fputs("            rm -f \"$c_path\"\n", file);
    fputs("            continue\n", file);
    fputs("            ;;\n", file);
    fputs("    esac\n", file);
    fputs("    link_env=\n", file);
    fputs("    extra_libs=\"$parserLib $ownLib\"\n", file);
    fputs("    if grep -q 'import: \"lm2/trans_library.lm2\"' \"$src\" 2>/dev/null; then\n", file);
    fputs("        link_env='LM_TRANS_LINK_LIBRARY=1'\n", file);
    fputs("        extra_libs=\"$transLib $parserLib $ownLib\"\n", file);
    fputs("    fi\n", file);
    fputs("    case \"$name\" in\n", file);
    fputs("        trans_registry_view_parity.lmx)\n", file);
    fputs("            legacy_path=\"$c_path.legacy\"\n", file);
    fputs("            env LM_TRANS_REGISTRY_VIEW=legacy $link_env \"$trans\" \"$src\" \"$legacy_path\"\n", file);
    fputs("            env LM_TRANS_REGISTRY_VIEW=view $link_env \"$trans\" \"$src\" \"$c_path\"\n", file);
    fputs("            cmp \"$legacy_path\" \"$c_path\"\n", file);
    fputs("            rm -f \"$legacy_path\"\n", file);
    fputs("            ;;\n", file);
    fputs("        *) env LM_TRANS_REGISTRY_VIEW=view $link_env \"$trans\" \"$src\" \"$c_path\" ;;\n", file);
    fputs("    esac\n", file);
    fputs("    case \"$name\" in trans_l4_abi_receivers.lm2|trans_message_thread_pool_single.lm2|trans_message_thread_mailbox_single.lm2) continue ;; esac\n", file);
    fputs("    support_c=\n", file);
    fputs("    if [ -f \"tests/$base.support.c\" ]; then support_c=\"tests/$base.support.c\"; fi\n", file);
    fputs("    warning_error=\n", file);
    fputs("    if [ \"$name\" = trans_l2_predef_conditional_descriptor.lm2 ]; then warning_error=-Werror; fi\n", file);
    fputs("    printf '%s\\n' \"\\\"$make_tool\\\" link -std=c99 -Wall -Wextra -Wpedantic ${warning_error:+$warning_error} -Ilm1 -Itests \\\"$c_path\\\" ${support_c:+\\\"$support_c\\\"} $extra_libs -o \\\"$exe_path\\\"\" >> build/obj/tests/link_jobs\n", file);
    fputs("    printf '%s\\n' \"$exe_path\" >> build/obj/tests/run_jobs\n", file);
    fputs("    if [ \"$name\" = trans_l2_predef_conditional_descriptor.lm2 ]; then\n", file);
    fputs("        branch_exe_path=\"build/obj/tests/${base}_branch\"\n", file);
    fputs("        printf '%s\\n' \"\\\"$make_tool\\\" link -std=c99 -Wall -Wextra -Wpedantic -Werror -DLM_TEST_PREDEF_DESCRIPTOR_BRANCH=1 -Ilm1 -Itests \\\"$c_path\\\" ${support_c:+\\\"$support_c\\\"} $extra_libs -o \\\"$branch_exe_path\\\"\" >> build/obj/tests/link_jobs\n", file);
    fputs("        printf '%s\\n' \"$branch_exe_path\" >> build/obj/tests/run_jobs\n", file);
    fputs("    fi\n", file);
    fputs("done\n", file);
    fputs("if ! lm_run_link_jobs < build/obj/tests/link_jobs; then echo 'trans test link failed' >&2; exit 1; fi\n", file);
    fputs("while IFS= read -r exe_path; do\n", file);
    fputs("    [ -n \"$exe_path\" ] || continue\n", file);
    fputs("    \"$exe_path\"\n", file);
    fputs("done < build/obj/tests/run_jobs\n", file);
    fputs("if [ \"$lm_test_suite\" = full ]; then\n", file);
    fputs("http_client_default_c='build/obj/tests/trans_rest_lmx_http_client_default.c'\n", file);
    fputs("http_client_default_test='build/obj/tests/trans_rest_lmx_http_client_default'\n", file);
    fputs("\"$make_tool\" link -std=c99 -Wall -Wextra -Wpedantic -Werror -DLM_REST_LMX_INSTALL_DEFAULT_CLIENT=1 -Ilm1 \"$http_client_default_c\" tests/trans_rest_lmx_http_client_default_stub.c \"$parserLib\" \"$ownLib\" -o \"$http_client_default_test\"\n", file);
    fputs("lm_run_with_watchdog \"$http_client_default_test\"\n", file);
    fputs("if LM_TEST_REST_LMX_INSTALL_FAIL=1 lm_run_with_watchdog \"$http_client_default_test\"; then\n", file);
    fputs("    echo 'REST/LMX installer failure unexpectedly succeeded' >&2\n", file);
    fputs("    exit 1\n", file);
    fputs("else\n", file);
    fputs("    install_failure_status=$?\n", file);
    fputs("    if [ \"$install_failure_status\" -ne 1 ]; then echo \"REST/LMX installer failure returned $install_failure_status, expected 1\" >&2; exit 1; fi\n", file);
    fputs("fi\n", file);
    fputs("rest_lmx_none_test='build/obj/tests/rest_lmx_http_client_none'\n", file);
    fputs("\"$make_tool\" link -std=c99 -Wall -Wextra -Wpedantic -Werror -DLM_REST_LMX_CLIENT_PROVIDER=0 -Ilm1 lm1/build/rest_lmx_http_client.lm1.c tests/rest_lmx_http_client_none.c \"$ownLib\" -o \"$rest_lmx_none_test\"\n", file);
    fputs("lm_run_with_watchdog \"$rest_lmx_none_test\"\n", file);
    fputs("rest_lmx_server_none_test='build/obj/tests/rest_lmx_http_server_none'\n", file);
    fputs("\"$make_tool\" link -std=c99 -Wall -Wextra -Wpedantic -Werror -DLM_REST_LMX_SERVER_PROVIDER=0 -Ilm1 lm1/build/rest_lmx_http_server.lm1.c tests/rest_lmx_http_server_none.c \"$ownLib\" -o \"$rest_lmx_server_none_test\"\n", file);
    fputs("lm_run_with_watchdog \"$rest_lmx_server_none_test\"\n", file);
    fputs("http_server_default_c='build/obj/tests/rest_lmx_http_server_default_source.c'\n", file);
    fputs("http_server_default_test='build/obj/tests/rest_lmx_http_server_default'\n", file);
    fputs("\"$trans\" tests/rest_lmx_http_server_default_source.lm2 \"$http_server_default_c\"\n", file);
    fputs("\"$make_tool\" link -std=c99 -Wall -Wextra -Wpedantic -Werror -DLM_REST_LMX_INSTALL_DEFAULT_SERVER=1 -Ilm1 \"$http_server_default_c\" tests/trans_rest_lmx_http_server_default_stub.c \"$parserLib\" \"$ownLib\" -o \"$http_server_default_test\"\n", file);
    fputs("lm_run_with_watchdog \"$http_server_default_test\"\n", file);
    fputs("if LM_TEST_REST_LMX_SERVER_START_FAIL=1 lm_run_with_watchdog \"$http_server_default_test\"; then\n", file);
    fputs("    echo 'REST/LMX server start failure unexpectedly succeeded' >&2\n", file);
    fputs("    exit 1\n", file);
    fputs("else\n", file);
    fputs("    server_start_failure_status=$?\n", file);
    fputs("    if [ \"$server_start_failure_status\" -ne 1 ]; then echo \"REST/LMX server start failure returned $server_start_failure_status, expected 1\" >&2; exit 1; fi\n", file);
    fputs("fi\n", file);
    fputs("http_server_none_c='build/obj/tests/rest_lmx_http_server_none_source.c'\n", file);
    fputs("http_server_none_lifecycle_test='build/obj/tests/rest_lmx_http_server_none_lifecycle'\n", file);
    fputs("\"$trans\" tests/rest_lmx_http_server_none_source.lm2 \"$http_server_none_c\"\n", file);
    fputs("\"$make_tool\" link -std=c99 -Wall -Wextra -Wpedantic -Werror -DLM_REST_LMX_INSTALL_DEFAULT_SERVER=1 -Ilm1 \"$http_server_none_c\" tests/trans_rest_lmx_http_server_none_stub.c \"$parserLib\" \"$ownLib\" -o \"$http_server_none_lifecycle_test\"\n", file);
    fputs("lm_run_with_watchdog \"$http_server_none_lifecycle_test\"\n", file);
    fputs("single_pool_test='build/obj/tests/trans_message_thread_pool_single'\n", file);
    fputs("LM_THREAD_PROVIDER=single \"$make_tool\" link -std=c99 -Wall -Wextra -Wpedantic -Werror -Ilm1 lm1/build/own.lm1.c build/obj/tests/trans_message_thread_pool_single.c -o \"$single_pool_test\"\n", file);
    fputs("lm_run_with_watchdog \"$single_pool_test\"\n", file);
    fputs("single_mailbox_test='build/obj/tests/trans_message_thread_mailbox_single'\n", file);
    fputs("LM_THREAD_PROVIDER=single \"$make_tool\" link -std=c99 -Wall -Wextra -Wpedantic -Werror -Ilm1 lm1/build/own.lm1.c build/obj/tests/trans_message_thread_mailbox_single.c -o \"$single_mailbox_test\"\n", file);
    fputs("lm_run_with_watchdog \"$single_mailbox_test\"\n", file);
    fputs("single_root_drain_test='build/obj/tests/message_thread_root_drain_single'\n", file);
    fputs("LM_THREAD_PROVIDER=single \"$make_tool\" link -std=c99 -Wall -Wextra -Wpedantic -Werror -Ilm1 lm1/build/own.lm1.c tests/message_thread_root_drain.c -o \"$single_root_drain_test\"\n", file);
    fputs("lm_run_with_watchdog \"$single_root_drain_test\"\n", file);
    fputs("single_application_exit_test='build/obj/tests/message_thread_application_exit_single'\n", file);
    fputs("LM_THREAD_PROVIDER=single \"$make_tool\" link -std=c99 -Wall -Wextra -Wpedantic -Werror -Ilm1 lm1/build/own.lm1.c tests/message_thread_application_exit.c -o \"$single_application_exit_test\"\n", file);
    fputs("lm_run_with_watchdog \"$single_application_exit_test\"\n", file);
    fputs("single_rest_lmx_provider_test='build/obj/tests/message_thread_rest_lmx_provider_single'\n", file);
    fputs("LM_THREAD_PROVIDER=single \"$make_tool\" link -std=c99 -Wall -Wextra -Wpedantic -Werror -Ilm1 lm1/build/own.lm1.c tests/message_thread_rest_lmx_provider.c -o \"$single_rest_lmx_provider_test\"\n", file);
    fputs("lm_run_with_watchdog \"$single_rest_lmx_provider_test\"\n", file);
    fputs("single_rest_lmx_ingress_test='build/obj/tests/message_thread_rest_lmx_ingress_single'\n", file);
    fputs("LM_THREAD_PROVIDER=single \"$make_tool\" link -std=c99 -Wall -Wextra -Wpedantic -Werror -Ilm1 lm1/build/own.lm1.c tests/message_thread_rest_lmx_ingress.c -o \"$single_rest_lmx_ingress_test\"\n", file);
    fputs("lm_run_with_watchdog \"$single_rest_lmx_ingress_test\"\n", file);
    fputs("native_pool_test='build/obj/tests/message_thread_pool_native'\n", file);
    fputs("LM_THREAD_PROVIDER=pthread \"$make_tool\" link -std=c99 -Wall -Wextra -Wpedantic -Werror -Ilm1 lm1/build/own.lm1.c tests/message_thread_pool_native.c -o \"$native_pool_test\"\n", file);
    fputs("lm_run_with_watchdog \"$native_pool_test\"\n", file);
    fputs("native_mailbox_test='build/obj/tests/message_thread_mailbox_native'\n", file);
    fputs("LM_THREAD_PROVIDER=pthread \"$make_tool\" link -std=c99 -Wall -Wextra -Wpedantic -Werror -Ilm1 lm1/build/own.lm1.c tests/message_thread_mailbox_native.c -o \"$native_mailbox_test\"\n", file);
    fputs("lm_run_with_watchdog \"$native_mailbox_test\"\n", file);
    fputs("native_root_drain_test='build/obj/tests/message_thread_root_drain_native'\n", file);
    fputs("LM_THREAD_PROVIDER=pthread \"$make_tool\" link -std=c99 -Wall -Wextra -Wpedantic -Werror -Ilm1 lm1/build/own.lm1.c tests/message_thread_root_drain.c -o \"$native_root_drain_test\"\n", file);
    fputs("lm_run_with_watchdog \"$native_root_drain_test\"\n", file);
    fputs("native_application_exit_test='build/obj/tests/message_thread_application_exit_native'\n", file);
    fputs("LM_THREAD_PROVIDER=pthread \"$make_tool\" link -std=c99 -Wall -Wextra -Wpedantic -Werror -Ilm1 lm1/build/own.lm1.c tests/message_thread_application_exit.c -o \"$native_application_exit_test\"\n", file);
    fputs("lm_run_with_watchdog \"$native_application_exit_test\"\n", file);
    fputs("native_rest_lmx_provider_test='build/obj/tests/message_thread_rest_lmx_provider_native'\n", file);
    fputs("LM_THREAD_PROVIDER=pthread \"$make_tool\" link -std=c99 -Wall -Wextra -Wpedantic -Werror -Ilm1 lm1/build/own.lm1.c tests/message_thread_rest_lmx_provider.c -o \"$native_rest_lmx_provider_test\"\n", file);
    fputs("lm_run_with_watchdog \"$native_rest_lmx_provider_test\"\n", file);
    fputs("native_rest_lmx_ingress_test='build/obj/tests/message_thread_rest_lmx_ingress_native'\n", file);
    fputs("LM_THREAD_PROVIDER=pthread \"$make_tool\" link -std=c99 -Wall -Wextra -Wpedantic -Werror -Ilm1 lm1/build/own.lm1.c tests/message_thread_rest_lmx_ingress.c -o \"$native_rest_lmx_ingress_test\"\n", file);
    fputs("lm_run_with_watchdog \"$native_rest_lmx_ingress_test\"\n", file);
    fputs("fi\n", file);
    fputs("echo \"lm0 staged tests passed: $lm_test_suite\"\n", file);
    return 0;
}
#endif
static LmBuildOptions * lm_build_options_new(struct LmMessageThread *lm_lmx_message_thread);
static void lm_build_options_delete(struct LmMessageThread *lm_lmx_message_thread, LmBuildOptions *options);
static int lm_build_is_path_separator(struct LmMessageThread *lm_lmx_message_thread, char value);
static int lm_build_has_path_separator(struct LmMessageThread *lm_lmx_message_thread, char *path);
static int lm_build_is_absolute_path(struct LmMessageThread *lm_lmx_message_thread, char *path);
static int lm_build_join_path(struct LmMessageThread *lm_lmx_message_thread, char *buffer, size_t size, char *base, char *tail);
static int lm_build_trim_last_path_part(struct LmMessageThread *lm_lmx_message_thread, char *path);
static int lm_build_has_project_marker(struct LmMessageThread *lm_lmx_message_thread, char *path);
static int lm_build_enter_project_root(struct LmMessageThread *lm_lmx_message_thread, char *program_path);
static char * lm_build_env_or_default(struct LmMessageThread *lm_lmx_message_thread, char *name, char *fallback);
static char * lm_build_thread_provider(struct LmMessageThread *lm_lmx_message_thread);
static char * lm_build_default_cmake(struct LmMessageThread *lm_lmx_message_thread);
static char * lm_build_default_generator(struct LmMessageThread *lm_lmx_message_thread);
static char * lm_build_default_make_program(struct LmMessageThread *lm_lmx_message_thread);
static char * lm_build_default_cc(struct LmMessageThread *lm_lmx_message_thread);
static char * lm_build_default_cxx(struct LmMessageThread *lm_lmx_message_thread);
static char * lm_build_test_suite_name(struct LmMessageThread *lm_lmx_message_thread, int suite);
static void lm_build_print_usage(struct LmMessageThread *lm_lmx_message_thread);
static int lm_build_parse_options(struct LmMessageThread *lm_lmx_message_thread, int argc, char **argv, LmBuildOptions *options);
static char * lm_build_output_dir(struct LmMessageThread *lm_lmx_message_thread, LmBuildOptions *options);
static size_t lm_build_append(struct LmMessageThread *lm_lmx_message_thread, char *buffer, size_t size, size_t used, char *text);
static size_t lm_build_append_arg(struct LmMessageThread *lm_lmx_message_thread, char *buffer, size_t size, size_t used, char *arg);
static size_t lm_build_append_prefixed_arg(struct LmMessageThread *lm_lmx_message_thread, char *buffer, size_t size, size_t used, char *prefix, char *value);
static int lm_build_run(struct LmMessageThread *lm_lmx_message_thread, char *command);
static int lm_build_make(struct LmMessageThread *lm_lmx_message_thread, char *make_tool, char *operation, char *args);
static int lm_build_trans(struct LmMessageThread *lm_lmx_message_thread, char *trans_tool, char *source_path, char *output_path);
static int lm_build_generate_all(struct LmMessageThread *lm_lmx_message_thread, char *trans_tool);
static int lm_build_parser_library(struct LmMessageThread *lm_lmx_message_thread, char *make_tool, char *output_dir);
static int lm_build_own_library(struct LmMessageThread *lm_lmx_message_thread, char *make_tool, char *output_dir);
static int lm_build_trans_library(struct LmMessageThread *lm_lmx_message_thread, char *make_tool, char *output_dir);
static int lm_build_compile_trans(struct LmMessageThread *lm_lmx_message_thread, char *make_tool, char *output_dir, char *parser_library, char *own_library);
static int lm_build_compile_generated_tools(struct LmMessageThread *lm_lmx_message_thread, char *make_tool, char *output_dir, char *parser_library, char *own_library);
static int lm_build_run_canary(struct LmMessageThread *lm_lmx_message_thread);
static int lm_build_write_staged_tests_script(struct LmMessageThread *lm_lmx_message_thread, char *output_dir, char *parser_library, char *own_library);
static int lm_build_run_staged_tests(struct LmMessageThread *lm_lmx_message_thread, char *make_tool, char *output_dir, char *parser_library, char *own_library);
static int lm_build_defer_finalize(struct LmMessageThread *lm_lmx_message_thread);
static int lm_build_extract_third_party_zips(struct LmMessageThread *lm_lmx_message_thread);
static int lm_build_clear_full_cmake_cache(struct LmMessageThread *lm_lmx_message_thread, char *cmake_tool, char *build_dir);
static int lm_build_full_configure(struct LmMessageThread *lm_lmx_message_thread);
static int lm_build_full_build(struct LmMessageThread *lm_lmx_message_thread);
static int lm_build_full_project(struct LmMessageThread *lm_lmx_message_thread);
static int lm_build_run_bootstrap(struct LmMessageThread *lm_lmx_message_thread, LmBuildOptions *options, char *trusted_make, char *built_trans);
int main(int argc, char **argv);





static int lm_lmx_module_private_1_init_all(struct LmMessageThread *lm_lmx_module_private_1_message_thread, struct LmOwnArena *lm_lmx_module_private_1_arena);
static inline int lm_message_thread_diagnostic_status(const LmMessageThreadExecutionContext *context) {
    if (context == 0 || context->diagnostic_code == 0) {
        return 1;
    }
    return context->diagnostic_code;
}
#if defined(__GNUC__) || defined(__clang__)
__attribute__((unused))
#endif
static int lm_lmx_module_private_1_init_all(struct LmMessageThread *lm_lmx_module_private_1_message_thread, struct LmOwnArena *lm_lmx_module_private_1_arena) {
    (void)lm_lmx_module_private_1_message_thread;
    (void)lm_lmx_module_private_1_arena;
    return 0;
}


static LmBuildOptions * lm_build_options_new(struct LmMessageThread *lm_lmx_message_thread) {
    (void)lm_lmx_message_thread;
    return lm_own_new_zero(sizeof(LmBuildOptions));
}

static void lm_build_options_delete(struct LmMessageThread *lm_lmx_message_thread, LmBuildOptions *options) {
    (void)lm_lmx_message_thread;
    lm_own_delete(options, 0);
}

static int lm_build_is_path_separator(struct LmMessageThread *lm_lmx_message_thread, char value) {
    (void)lm_lmx_message_thread;
    return value == '/' || value == '\\';
}

static int lm_build_has_path_separator(struct LmMessageThread *lm_lmx_message_thread, char *path) {
    (void)lm_lmx_message_thread;
    return strchr(path, '/') != 0 || strchr(path, '\\') != 0;
}

static int lm_build_is_absolute_path(struct LmMessageThread *lm_lmx_message_thread, char *path) {
    (void)lm_lmx_message_thread;
    if (path == 0 || path[0] == '\0') {
        return 0;
    }
    if (lm_build_platform_absolute(lm_lmx_message_thread, path)) {
        return 1;
    }
    return 0;
}

static int lm_build_join_path(struct LmMessageThread *lm_lmx_message_thread, char *buffer, size_t size, char *base, char *tail) {
    (void)lm_lmx_message_thread;
    size_t base_length;
    size_t tail_length;
    size_t used;
    base_length = strlen(base);
    tail_length = strlen(tail);
    used = base_length;
    if (base_length + tail_length + 2U >= size) {
        fprintf(stderr, "buildCore.lm0: path is too long\n");
        return 1;
    }
    memcpy(buffer, base, base_length);
    if (base_length > 0U && tail_length > 0U && lm_build_is_path_separator(lm_lmx_message_thread, base[base_length - 1U]) == 0 && lm_build_is_path_separator(lm_lmx_message_thread, tail[0]) == 0) {
        memcpy(buffer + used, lm_build_path_sep(lm_lmx_message_thread), strlen(lm_build_path_sep(lm_lmx_message_thread)));
        used = used + strlen(lm_build_path_sep(lm_lmx_message_thread));
    }
    memcpy(buffer + used, tail, tail_length + 1U);
    return 0;
}

static int lm_build_trim_last_path_part(struct LmMessageThread *lm_lmx_message_thread, char *path) {
    (void)lm_lmx_message_thread;
    size_t length;
    length = strlen(path);
    while (length > 0U && lm_build_is_path_separator(lm_lmx_message_thread, path[length - 1U])) {
        path[length - 1U] = '\0';
        length = length - 1U;
    }
    while (length > 0U) {
        length = length - 1U;
        if (lm_build_is_path_separator(lm_lmx_message_thread, path[length])) {
            path[length] = '\0';
            return 0;
        }
    }
    path[0] = '\0';
    return 0;
}

static int lm_build_has_project_marker(struct LmMessageThread *lm_lmx_message_thread, char *path) {
    (void)lm_lmx_message_thread;
    char marker_path[2048];
    if (lm_build_join_path(lm_lmx_message_thread, marker_path, sizeof(marker_path), path, "lm2/buildCore.lmx") != 0) {
        return 0;
    }
    if (lm_build_file_exists(lm_lmx_message_thread, marker_path)) {
        return 1;
    }
    return 0;
}

static int lm_build_enter_project_root(struct LmMessageThread *lm_lmx_message_thread, char *program_path) {
    (void)lm_lmx_message_thread;
    char search_path[1024];
    char executable_path[1024];
    char cwd[1024];
    int depth;
    if (lm_build_platform_getcwd(lm_lmx_message_thread, cwd, sizeof(cwd)) == 0) {
        fprintf(stderr, "buildCore.lm0: cannot read current directory\n");
        return 1;
    }
    if (program_path != 0 && program_path[0] != '\0' && lm_build_has_path_separator(lm_lmx_message_thread, program_path)) {
        if (lm_build_is_absolute_path(lm_lmx_message_thread, program_path)) {
            if (strlen(program_path) >= sizeof(executable_path)) {
                fprintf(stderr, "buildCore.lm0: executable path is too long\n");
                return 1;
            }
            strcpy(executable_path, program_path);
        }
        if (lm_build_is_absolute_path(lm_lmx_message_thread, program_path) == 0) {
            if (lm_build_join_path(lm_lmx_message_thread, executable_path, sizeof(executable_path), cwd, program_path) != 0) {
                return 1;
            }
        }
        strcpy(search_path, executable_path);
        lm_build_trim_last_path_part(lm_lmx_message_thread, search_path);
    }
    if (program_path == 0 || program_path[0] == '\0' || lm_build_has_path_separator(lm_lmx_message_thread, program_path) == 0) {
        strcpy(search_path, cwd);
    }
    depth = 0;
    while (depth < 12 && search_path[0] != '\0') {
        if (lm_build_has_project_marker(lm_lmx_message_thread, search_path)) {
            if (lm_build_platform_chdir(lm_lmx_message_thread, search_path) != 0) {
                fprintf(stderr, "buildCore.lm0: cannot enter project root %s\n", search_path);
                return 1;
            }
            return 0;
        }
        lm_build_trim_last_path_part(lm_lmx_message_thread, search_path);
        depth = depth + 1;
    }
    fprintf(stderr, "buildCore.lm0: cannot locate project root from %s\n", cwd);
    return 1;
}

static char * lm_build_env_or_default(struct LmMessageThread *lm_lmx_message_thread, char *name, char *fallback) {
    (void)lm_lmx_message_thread;
    char *value;
    value = getenv(name);
    if (value == 0 || value[0] == '\0') {
        return fallback;
    }
    return value;
}

static char * lm_build_thread_provider(struct LmMessageThread *lm_lmx_message_thread) {
    (void)lm_lmx_message_thread;
    char *value;
    value = lm_build_env_or_default(lm_lmx_message_thread, "LM_THREAD_PROVIDER", "single");
    if (strcmp(value, "auto") == 0 || strcmp(value, "pthread") == 0 || strcmp(value, "win32") == 0 || strcmp(value, "single") == 0) {
        return value;
    }
    fprintf(stderr, "buildCore.lm0: unsupported LM_THREAD_PROVIDER '%s' (expected auto, pthread, win32 or single)\n", value);
    return 0;
}

static char * lm_build_default_cmake(struct LmMessageThread *lm_lmx_message_thread) {
    (void)lm_lmx_message_thread;
    if (lm_build_has_qt_cmake(lm_lmx_message_thread)) {
        return "C:/Qt/Tools/CMake_64/bin/cmake.exe";
    }
    return "cmake";
}

static char * lm_build_default_generator(struct LmMessageThread *lm_lmx_message_thread) {
    (void)lm_lmx_message_thread;
    if (lm_build_has_qt_mingw_make(lm_lmx_message_thread)) {
        return "MinGW Makefiles";
    }
    return "";
}

static char * lm_build_default_make_program(struct LmMessageThread *lm_lmx_message_thread) {
    (void)lm_lmx_message_thread;
    if (lm_build_has_qt_mingw_make(lm_lmx_message_thread)) {
        return "C:/Qt/Tools/mingw1310_64/bin/mingw32-make.exe";
    }
    return "";
}

static char * lm_build_default_cc(struct LmMessageThread *lm_lmx_message_thread) {
    (void)lm_lmx_message_thread;
    if (lm_build_has_qt_gcc(lm_lmx_message_thread)) {
        return "C:/Qt/Tools/mingw1310_64/bin/gcc.exe";
    }
    return "";
}

static char * lm_build_default_cxx(struct LmMessageThread *lm_lmx_message_thread) {
    (void)lm_lmx_message_thread;
    if (lm_build_has_qt_gxx(lm_lmx_message_thread)) {
        return "C:/Qt/Tools/mingw1310_64/bin/g++.exe";
    }
    return "";
}

static char * lm_build_test_suite_name(struct LmMessageThread *lm_lmx_message_thread, int suite) {
    (void)lm_lmx_message_thread;
    if (suite == 2) {
        return "smoke";
    }
    if (suite == 1) {
        return "core";
    }
    return "full";
}

static void lm_build_print_usage(struct LmMessageThread *lm_lmx_message_thread) {
    (void)lm_lmx_message_thread;
    printf("usage: buildCore.lm0 [--build] [--full] [--next] [--tests smoke core full]\n");
    printf("  --build refresh the L0 bootstrap tools\n");
    printf("  --full  refresh L0 tools, then build the bundled third_party profile\n");
    printf("  --next  verify the staged L0 bootstrap tools without installing them\n");
    printf("  --tests smoke, core or full  staged test suite (default full)\n");
}

static int lm_build_parse_options(struct LmMessageThread *lm_lmx_message_thread, int argc, char **argv, LmBuildOptions *options) {
    (void)lm_lmx_message_thread;
    int index;
    char *option;
    options->full_build = 0;
    options->next_build = 0;
    options->test_suite = 0;
    index = 1;
    while (index < argc) {
        option = argv[index];
        if (strcmp(option, "--build") == 0) {
        }
        if (strcmp(option, "--full") == 0) {
            options->full_build = 1;
        }
        if (strcmp(option, "--next") == 0) {
            options->next_build = 1;
        }
        if (strcmp(option, "--tests") == 0) {
            if (index + 1 >= argc) {
                fprintf(stderr, "buildCore.lm0: --tests requires smoke, core or full\n");
                lm_build_print_usage(lm_lmx_message_thread);
                return 1;
            }
            if (strcmp(argv[index + 1], "smoke") == 0) {
                options->test_suite = 2;
            }
            if (strcmp(argv[index + 1], "core") == 0) {
                options->test_suite = 1;
            }
            if (strcmp(argv[index + 1], "full") == 0) {
                options->test_suite = 0;
            }
            if (strcmp(argv[index + 1], "smoke") != 0 && strcmp(argv[index + 1], "core") != 0 && strcmp(argv[index + 1], "full") != 0) {
                fprintf(stderr, "buildCore.lm0: unknown test suite: %s\n", argv[index + 1]);
                lm_build_print_usage(lm_lmx_message_thread);
                return 1;
            }
            index = index + 1;
        }
        if (strcmp(option, "--help") == 0 || strcmp(option, "-h") == 0) {
            lm_build_print_usage(lm_lmx_message_thread);
            return 2;
        }
        if (strcmp(option, "--build") != 0 && strcmp(option, "--full") != 0 && strcmp(option, "--next") != 0 && strcmp(option, "--help") != 0 && strcmp(option, "-h") != 0 && strcmp(option, "--tests") != 0) {
            fprintf(stderr, "buildCore.lm0: unknown option: %s\n", option);
            lm_build_print_usage(lm_lmx_message_thread);
            return 1;
        }
        index = index + 1;
    }
    return 0;
}

static char * lm_build_output_dir(struct LmMessageThread *lm_lmx_message_thread, LmBuildOptions *options) {
    (void)lm_lmx_message_thread;
    if (options -> next_build) {
        return "build/lm0/next/check";
    }
    return "build/lm0/next";
}

static size_t lm_build_append(struct LmMessageThread *lm_lmx_message_thread, char *buffer, size_t size, size_t used, char *text) {
    (void)lm_lmx_message_thread;
    size_t length;
    length = strlen(text);
    if (used + length >= size) {
        fprintf(stderr, "buildCore.lm0: command line is too long\n");
        return size;
    }
    memcpy(buffer + used, text, length + 1U);
    return used + length;
}

static size_t lm_build_append_arg(struct LmMessageThread *lm_lmx_message_thread, char *buffer, size_t size, size_t used, char *arg) {
    (void)lm_lmx_message_thread;
    used = lm_build_append(lm_lmx_message_thread, buffer, size, used, " \"");
    if (used == size) {
        return size;
    }
    used = lm_build_append(lm_lmx_message_thread, buffer, size, used, arg);
    if (used == size) {
        return size;
    }
    return lm_build_append(lm_lmx_message_thread, buffer, size, used, "\"");
}

static size_t lm_build_append_prefixed_arg(struct LmMessageThread *lm_lmx_message_thread, char *buffer, size_t size, size_t used, char *prefix, char *value) {
    (void)lm_lmx_message_thread;
    char arg[2048];
    if (strlen(prefix) + strlen(value) >= sizeof(arg)) {
        fprintf(stderr, "buildCore.lm0: CMake argument is too long\n");
        return size;
    }
    strcpy(arg, prefix);
    strcat(arg, value);
    return lm_build_append_arg(lm_lmx_message_thread, buffer, size, used, arg);
}

static int lm_build_run(struct LmMessageThread *lm_lmx_message_thread, char *command) {
    (void)lm_lmx_message_thread;
    int status;
    printf("%s\n", command);
    status = system(command);
    if (status != 0) {
        fprintf(stderr, "buildCore.lm0: command failed with status %d\n", status);
        return 1;
    }
    return 0;
}

static int lm_build_make(struct LmMessageThread *lm_lmx_message_thread, char *make_tool, char *operation, char *args) {
    (void)lm_lmx_message_thread;
    char command[8192];
    size_t used;
    used = 0U;
    command[0] = '\0';
    used = lm_build_append(lm_lmx_message_thread, command, sizeof(command), used, make_tool);
    if (used == sizeof(command)) {
        return 1;
    }
    used = lm_build_append_arg(lm_lmx_message_thread, command, sizeof(command), used, operation);
    if (used == sizeof(command)) {
        return 1;
    }
    if (args != 0 && args[0] != '\0') {
        used = lm_build_append(lm_lmx_message_thread, command, sizeof(command), used, " ");
        if (used == sizeof(command)) {
            return 1;
        }
        used = lm_build_append(lm_lmx_message_thread, command, sizeof(command), used, args);
        if (used == sizeof(command)) {
            return 1;
        }
    }
    return lm_build_run(lm_lmx_message_thread, command);
}

static int lm_build_trans(struct LmMessageThread *lm_lmx_message_thread, char *trans_tool, char *source_path, char *output_path) {
    (void)lm_lmx_message_thread;
    char command[4096];
    size_t used;
    used = 0U;
    command[0] = '\0';
    used = lm_build_append(lm_lmx_message_thread, command, sizeof(command), used, trans_tool);
    if (used == sizeof(command)) {
        return 1;
    }
    used = lm_build_append_arg(lm_lmx_message_thread, command, sizeof(command), used, source_path);
    if (used == sizeof(command)) {
        return 1;
    }
    used = lm_build_append_arg(lm_lmx_message_thread, command, sizeof(command), used, output_path);
    if (used == sizeof(command)) {
        return 1;
    }
    return lm_build_run(lm_lmx_message_thread, command);
}

static int lm_build_generate_all(struct LmMessageThread *lm_lmx_message_thread, char *trans_tool) {
    (void)lm_lmx_message_thread;
    if (lm_build_trans(lm_lmx_message_thread, trans_tool, "lm2/own.lm2", "lm1/build/own.lm1.c") != 0) {
        return 1;
    }
    if (lm_build_trans(lm_lmx_message_thread, trans_tool, "lm2/rest_lmx_http_client.lm2", "lm1/build/rest_lmx_http_client.lm1.c") != 0) {
        return 1;
    }
    if (lm_build_trans(lm_lmx_message_thread, trans_tool, "lm2/rest_lmx_http_server.lm2", "lm1/build/rest_lmx_http_server.lm1.c") != 0) {
        return 1;
    }
    if (lm_build_trans(lm_lmx_message_thread, trans_tool, "lm2/trans.lm2", "lm1/build/trans.lm1.c") != 0) {
        return 1;
    }
    if (lm_build_setenv(lm_lmx_message_thread, "LM_TRANS_LIBRARY", "1") != 0) {
        return 1;
    }
    if (lm_build_trans(lm_lmx_message_thread, trans_tool, "lm2/trans_library.lm2", "lm1/build/trans_library.lm1.c") != 0) {
        lm_build_unsetenv(lm_lmx_message_thread, "LM_TRANS_LIBRARY");
        return 1;
    }
    lm_build_unsetenv(lm_lmx_message_thread, "LM_TRANS_LIBRARY");
    if (lm_build_trans(lm_lmx_message_thread, trans_tool, "lm2/parser.lm2", "lm1/build/parser.lm1.c") != 0) {
        return 1;
    }
    if (lm_build_trans(lm_lmx_message_thread, trans_tool, "lm2/printTree.lm2", "lm1/build/printTree.lm1.c") != 0) {
        return 1;
    }
    if (lm_build_trans(lm_lmx_message_thread, trans_tool, "lm2/finalize.lm2", "lm1/build/finalize.lm1.c") != 0) {
        return 1;
    }
    if (lm_build_trans(lm_lmx_message_thread, trans_tool, "lm2/make.lm2", "lm1/build/make.lm1.c") != 0) {
        return 1;
    }
    if (lm_build_trans(lm_lmx_message_thread, trans_tool, "lm2/vcpkgFetch.lm2", "lm1/build/vcpkgFetch.lm1.c") != 0) {
        return 1;
    }
    if (lm_build_trans(lm_lmx_message_thread, trans_tool, "lm2/buildCore.lmx", "lm1/build/buildCore.lm1.c") != 0) {
        return 1;
    }
    return 0;
}

static int lm_build_parser_library(struct LmMessageThread *lm_lmx_message_thread, char *make_tool, char *output_dir) {
    (void)lm_lmx_message_thread;
    char library_path[512];
    char command[4096];
    snprintf(library_path, sizeof(library_path), "%s/libparser.lm0.a", output_dir);
    if (lm_build_make(lm_lmx_message_thread, make_tool, "cc", "-std=c99 -Wall -Wextra -Wpedantic -I\"lm1\" -c \"lm1/build/parser.lm1.c\" -o \"build/obj/parser.lm1.o\"") != 0) {
        return 1;
    }
    remove(library_path);
    snprintf(command, sizeof(command), "rcs \"%s\" \"build/obj/parser.lm1.o\"", library_path);
    if (lm_build_make(lm_lmx_message_thread, make_tool, "ar", command) != 0) {
        return 1;
    }
    snprintf(command, sizeof(command), "\"%s\"", library_path);
    return lm_build_make(lm_lmx_message_thread, make_tool, "ranlib", command);
}

static int lm_build_own_library(struct LmMessageThread *lm_lmx_message_thread, char *make_tool, char *output_dir) {
    (void)lm_lmx_message_thread;
    char library_path[512];
    char command[4096];
    snprintf(library_path, sizeof(library_path), "%s/libown.lm0.a", output_dir);
    if (lm_build_make(lm_lmx_message_thread, make_tool, "cc", "-std=c99 -Wall -Wextra -Wpedantic -I\"lm1\" -c \"lm1/build/own.lm1.c\" -o \"build/obj/own.lm1.o\"") != 0) {
        return 1;
    }
    remove(library_path);
    snprintf(command, sizeof(command), "rcs \"%s\" \"build/obj/own.lm1.o\"", library_path);
    if (lm_build_make(lm_lmx_message_thread, make_tool, "ar", command) != 0) {
        return 1;
    }
    snprintf(command, sizeof(command), "\"%s\"", library_path);
    return lm_build_make(lm_lmx_message_thread, make_tool, "ranlib", command);
}

static int lm_build_trans_library(struct LmMessageThread *lm_lmx_message_thread, char *make_tool, char *output_dir) {
    (void)lm_lmx_message_thread;
    char library_path[512];
    char command[4096];
    snprintf(library_path, sizeof(library_path), "%s/libtrans.lm0.a", output_dir);
    if (lm_build_make(lm_lmx_message_thread, make_tool, "cc", "-std=c99 -Wall -Wextra -Wpedantic -I\"lm1\" -c \"lm1/build/trans_library.lm1.c\" -o \"build/obj/trans_library.lm1.o\"") != 0) {
        return 1;
    }
    remove(library_path);
    snprintf(command, sizeof(command), "rcs \"%s\" \"build/obj/trans_library.lm1.o\"", library_path);
    if (lm_build_make(lm_lmx_message_thread, make_tool, "ar", command) != 0) {
        return 1;
    }
    snprintf(command, sizeof(command), "\"%s\"", library_path);
    return lm_build_make(lm_lmx_message_thread, make_tool, "ranlib", command);
}

static int lm_build_compile_trans(struct LmMessageThread *lm_lmx_message_thread, char *make_tool, char *output_dir, char *parser_library, char *own_library) {
    (void)lm_lmx_message_thread;
    char command[4096];
    snprintf(command, sizeof(command), "-std=c99 -Wall -Wextra -Wpedantic -I\"lm1\" \"lm1/build/trans.lm1.c\" \"%s\" \"%s\" -o \"%s/trans.lm0%s\"", parser_library, own_library, output_dir, lm_build_exe_suffix(lm_lmx_message_thread));
    if (lm_build_make(lm_lmx_message_thread, make_tool, "link", command) != 0) {
        return 1;
    }
    return 0;
}

static int lm_build_compile_generated_tools(struct LmMessageThread *lm_lmx_message_thread, char *make_tool, char *output_dir, char *parser_library, char *own_library) {
    (void)lm_lmx_message_thread;
    char command[4096];
    snprintf(command, sizeof(command), "-std=c99 -Wall -Wextra -Wpedantic \"lm1/build/make.lm1.c\" \"%s\" -o \"%s/make.lm0%s\"", own_library, output_dir, lm_build_exe_suffix(lm_lmx_message_thread));
    if (lm_build_make(lm_lmx_message_thread, make_tool, "link", command) != 0) {
        return 1;
    }
    snprintf(command, sizeof(command), "-std=c99 -Wall -Wextra -Wpedantic -D_POSIX_C_SOURCE=200809L \"lm1/build/finalize.lm1.c\" \"%s\" -o \"%s/finalize.lm0%s\"", own_library, output_dir, lm_build_exe_suffix(lm_lmx_message_thread));
    if (lm_build_make(lm_lmx_message_thread, make_tool, "link", command) != 0) {
        return 1;
    }
    snprintf(command, sizeof(command), "-std=c99 -Wall -Wextra -Wpedantic -I\"lm1\" \"lm1/build/vcpkgFetch.lm1.c\" \"%s\" -o \"%s/vcpkgFetch.lm0%s\"", own_library, output_dir, lm_build_exe_suffix(lm_lmx_message_thread));
    if (lm_build_make(lm_lmx_message_thread, make_tool, "link", command) != 0) {
        return 1;
    }
    snprintf(command, sizeof(command), "-std=c99 -Wall -Wextra -Wpedantic -I\"lm1\" \"lm1/build/printTree.lm1.c\" \"%s\" \"%s\" -o \"%s/printTree.lm0%s\"", parser_library, own_library, output_dir, lm_build_exe_suffix(lm_lmx_message_thread));
    if (lm_build_make(lm_lmx_message_thread, make_tool, "link", command) != 0) {
        return 1;
    }
    snprintf(command, sizeof(command), "-std=c99 -Wall -Wextra -Wpedantic -D_POSIX_C_SOURCE=200809L -I\"lm1\" \"lm1/build/buildCore.lm1.c\" \"%s\" -o \"%s/buildCore.lm0%s\"", own_library, output_dir, lm_build_exe_suffix(lm_lmx_message_thread));
    return lm_build_make(lm_lmx_message_thread, make_tool, "link", command);
}

static int lm_build_run_canary(struct LmMessageThread *lm_lmx_message_thread) {
    (void)lm_lmx_message_thread;
    char canary_path[512];
    char *command_path;
    char command[4096];
    snprintf(canary_path, sizeof(canary_path), "build/lm0/next/buildCore.lm0%s", lm_build_exe_suffix(lm_lmx_message_thread));
    if (lm_build_prepare_platform_canary(lm_lmx_message_thread, canary_path) != 0) {
        return 1;
    }
    command_path = lm_build_platform_canary_command_path(lm_lmx_message_thread, canary_path);
    snprintf(command, sizeof(command), lm_build_platform_canary_command_format(lm_lmx_message_thread), command_path);
    return lm_build_run(lm_lmx_message_thread, command);
}

static int lm_build_write_staged_tests_script(struct LmMessageThread *lm_lmx_message_thread, char *output_dir, char *parser_library, char *own_library) {
    (void)lm_lmx_message_thread;
    FILE * file;
    file = fopen(lm_build_platform_tests_script_path(lm_lmx_message_thread), "wb");
    if (file == 0) {
        fprintf(stderr, "buildCore.lm0: cannot write staged tests script %s\n", lm_build_platform_tests_script_path(lm_lmx_message_thread));
        return 1;
    }
    if (lm_build_write_platform_tests_script(lm_lmx_message_thread, file, output_dir, parser_library, own_library) != 0) {
        fclose(file);
        return 1;
    }
    if (fclose(file) != 0) {
        fprintf(stderr, "buildCore.lm0: cannot close staged tests script %s\n", lm_build_platform_tests_script_path(lm_lmx_message_thread));
        return 1;
    }
    return 0;
}

static int lm_build_run_staged_tests(struct LmMessageThread *lm_lmx_message_thread, char *make_tool, char *output_dir, char *parser_library, char *own_library) {
    (void)lm_lmx_message_thread;
    char command[512];
    if (lm_build_make(lm_lmx_message_thread, make_tool, "mkdir", "\"build/obj/tests\"") != 0) {
        return 1;
    }
    if (lm_build_write_staged_tests_script(lm_lmx_message_thread, output_dir, parser_library, own_library) != 0) {
        return 1;
    }
    snprintf(command, sizeof(command), lm_build_platform_tests_command_format(lm_lmx_message_thread), lm_build_platform_tests_script_path(lm_lmx_message_thread));
    return lm_build_run(lm_lmx_message_thread, command);
}

static int lm_build_defer_finalize(struct LmMessageThread *lm_lmx_message_thread) {
    (void)lm_lmx_message_thread;
    char command[256];
    snprintf(command, sizeof(command), "build%slm0%snext%sfinalize.lm0%s --defer", lm_build_path_sep(lm_lmx_message_thread), lm_build_path_sep(lm_lmx_message_thread), lm_build_path_sep(lm_lmx_message_thread), lm_build_exe_suffix(lm_lmx_message_thread));
    return lm_build_run(lm_lmx_message_thread, command);
}

static int lm_build_extract_third_party_zips(struct LmMessageThread *lm_lmx_message_thread) {
    (void)lm_lmx_message_thread;
    char *cmake_tool;
    char command[4096];
    size_t used;
    cmake_tool = lm_build_env_or_default(lm_lmx_message_thread, "LM_CMAKE", lm_build_default_cmake(lm_lmx_message_thread));
    used = 0U;
    command[0] = '\0';
    used = lm_build_append(lm_lmx_message_thread, command, sizeof(command), used, cmake_tool);
    if (used == sizeof(command)) {
        return 1;
    }
    used = lm_build_append_arg(lm_lmx_message_thread, command, sizeof(command), used, "-DLM_THIRD_PARTY_ARCHIVE_DIR=third_party");
    if (used == sizeof(command)) {
        return 1;
    }
    used = lm_build_append_arg(lm_lmx_message_thread, command, sizeof(command), used, "-DLM_THIRD_PARTY_EXTRACT_DIR=build/third_party");
    if (used == sizeof(command)) {
        return 1;
    }
    used = lm_build_append_arg(lm_lmx_message_thread, command, sizeof(command), used, "-P");
    if (used == sizeof(command)) {
        return 1;
    }
    used = lm_build_append_arg(lm_lmx_message_thread, command, sizeof(command), used, "cmake/ExtractThirdPartyZips.cmake");
    if (used == sizeof(command)) {
        return 1;
    }
    return lm_build_run(lm_lmx_message_thread, command);
}

static int lm_build_clear_full_cmake_cache(struct LmMessageThread *lm_lmx_message_thread, char *cmake_tool, char *build_dir) {
    (void)lm_lmx_message_thread;
    char command[4096];
    char cache_path[2048];
    char files_path[2048];
    size_t used;
    if (strlen(build_dir) + 32U >= sizeof(cache_path)) {
        fprintf(stderr, "buildCore.lm0: full build directory is too long\n");
        return 1;
    }
    snprintf(cache_path, sizeof(cache_path), "%s/CMakeCache.txt", build_dir);
    snprintf(files_path, sizeof(files_path), "%s/CMakeFiles", build_dir);
    used = 0U;
    command[0] = '\0';
    used = lm_build_append(lm_lmx_message_thread, command, sizeof(command), used, cmake_tool);
    if (used == sizeof(command)) {
        return 1;
    }
    used = lm_build_append_arg(lm_lmx_message_thread, command, sizeof(command), used, "-E");
    if (used == sizeof(command)) {
        return 1;
    }
    used = lm_build_append_arg(lm_lmx_message_thread, command, sizeof(command), used, "rm");
    if (used == sizeof(command)) {
        return 1;
    }
    used = lm_build_append_arg(lm_lmx_message_thread, command, sizeof(command), used, "-f");
    if (used == sizeof(command)) {
        return 1;
    }
    used = lm_build_append_arg(lm_lmx_message_thread, command, sizeof(command), used, cache_path);
    if (used == sizeof(command)) {
        return 1;
    }
    if (lm_build_run(lm_lmx_message_thread, command) != 0) {
        return 1;
    }
    used = 0U;
    command[0] = '\0';
    used = lm_build_append(lm_lmx_message_thread, command, sizeof(command), used, cmake_tool);
    if (used == sizeof(command)) {
        return 1;
    }
    used = lm_build_append_arg(lm_lmx_message_thread, command, sizeof(command), used, "-E");
    if (used == sizeof(command)) {
        return 1;
    }
    used = lm_build_append_arg(lm_lmx_message_thread, command, sizeof(command), used, "rm");
    if (used == sizeof(command)) {
        return 1;
    }
    used = lm_build_append_arg(lm_lmx_message_thread, command, sizeof(command), used, "-rf");
    if (used == sizeof(command)) {
        return 1;
    }
    used = lm_build_append_arg(lm_lmx_message_thread, command, sizeof(command), used, files_path);
    if (used == sizeof(command)) {
        return 1;
    }
    return lm_build_run(lm_lmx_message_thread, command);
}

static int lm_build_full_configure(struct LmMessageThread *lm_lmx_message_thread) {
    (void)lm_lmx_message_thread;
    char *cmake_tool;
    char *build_dir;
    char *build_type;
    char *generator;
    char *make_program;
    char *cc;
    char *cxx;
    char *thread_provider;
    char command[8192];
    size_t used;
    cmake_tool = lm_build_env_or_default(lm_lmx_message_thread, "LM_CMAKE", lm_build_default_cmake(lm_lmx_message_thread));
    build_dir = lm_build_env_or_default(lm_lmx_message_thread, "LM_FULL_BUILD_DIR", "build/libs");
    build_type = lm_build_env_or_default(lm_lmx_message_thread, "LM_FULL_BUILD_TYPE", "Debug");
    generator = lm_build_env_or_default(lm_lmx_message_thread, "LM_CMAKE_GENERATOR", lm_build_default_generator(lm_lmx_message_thread));
    make_program = lm_build_env_or_default(lm_lmx_message_thread, "LM_CMAKE_MAKE_PROGRAM", lm_build_default_make_program(lm_lmx_message_thread));
    cc = lm_build_env_or_default(lm_lmx_message_thread, "LM_CC", lm_build_default_cc(lm_lmx_message_thread));
    cxx = lm_build_env_or_default(lm_lmx_message_thread, "LM_CXX", lm_build_default_cxx(lm_lmx_message_thread));
    thread_provider = lm_build_thread_provider(lm_lmx_message_thread);
    if (thread_provider == 0) {
        return 1;
    }
    if (lm_build_clear_full_cmake_cache(lm_lmx_message_thread, cmake_tool, build_dir) != 0) {
        return 1;
    }
    used = 0U;
    command[0] = '\0';
    used = lm_build_append(lm_lmx_message_thread, command, sizeof(command), used, cmake_tool);
    if (used == sizeof(command)) {
        return 1;
    }
    used = lm_build_append_arg(lm_lmx_message_thread, command, sizeof(command), used, "-S");
    if (used == sizeof(command)) {
        return 1;
    }
    used = lm_build_append_arg(lm_lmx_message_thread, command, sizeof(command), used, ".");
    if (used == sizeof(command)) {
        return 1;
    }
    used = lm_build_append_arg(lm_lmx_message_thread, command, sizeof(command), used, "-B");
    if (used == sizeof(command)) {
        return 1;
    }
    used = lm_build_append_arg(lm_lmx_message_thread, command, sizeof(command), used, build_dir);
    if (used == sizeof(command)) {
        return 1;
    }
    if (generator[0] != '\0') {
        used = lm_build_append_arg(lm_lmx_message_thread, command, sizeof(command), used, "-G");
        if (used == sizeof(command)) {
            return 1;
        }
        used = lm_build_append_arg(lm_lmx_message_thread, command, sizeof(command), used, generator);
        if (used == sizeof(command)) {
            return 1;
        }
    }
    used = lm_build_append_prefixed_arg(lm_lmx_message_thread, command, sizeof(command), used, "-DCMAKE_BUILD_TYPE=", build_type);
    if (used == sizeof(command)) {
        return 1;
    }
    used = lm_build_append_prefixed_arg(lm_lmx_message_thread, command, sizeof(command), used, "-DLM_THREAD_PROVIDER=", thread_provider);
    if (used == sizeof(command)) {
        return 1;
    }
    if (make_program[0] != '\0') {
        used = lm_build_append_prefixed_arg(lm_lmx_message_thread, command, sizeof(command), used, "-DCMAKE_MAKE_PROGRAM=", make_program);
        if (used == sizeof(command)) {
            return 1;
        }
    }
    if (cc[0] != '\0') {
        used = lm_build_append_prefixed_arg(lm_lmx_message_thread, command, sizeof(command), used, "-DCMAKE_C_COMPILER=", cc);
        if (used == sizeof(command)) {
            return 1;
        }
    }
    if (cxx[0] != '\0') {
        used = lm_build_append_prefixed_arg(lm_lmx_message_thread, command, sizeof(command), used, "-DCMAKE_CXX_COMPILER=", cxx);
        if (used == sizeof(command)) {
            return 1;
        }
    }
    used = lm_build_append_arg(lm_lmx_message_thread, command, sizeof(command), used, "-DLINGVAMYXA_ENABLE_EXTERNAL_DEPS=OFF");
    if (used == sizeof(command)) {
        return 1;
    }
    used = lm_build_append_arg(lm_lmx_message_thread, command, sizeof(command), used, "-DLINGVAMYXA_ENABLE_BUNDLED_THIRD_PARTY=ON");
    if (used == sizeof(command)) {
        return 1;
    }
    used = lm_build_append_arg(lm_lmx_message_thread, command, sizeof(command), used, "-DLM_THIRD_PARTY_SOURCE_DIR=build/third_party");
    if (used == sizeof(command)) {
        return 1;
    }
    used = lm_build_append_arg(lm_lmx_message_thread, command, sizeof(command), used, "-DBUILD_LINGVAMYXA_QT_APP=OFF");
    if (used == sizeof(command)) {
        return 1;
    }
    used = lm_build_append_arg(lm_lmx_message_thread, command, sizeof(command), used, "-DBUILD_BUILD_CORE_LM0=OFF");
    if (used == sizeof(command)) {
        return 1;
    }
    used = lm_build_append_arg(lm_lmx_message_thread, command, sizeof(command), used, "-DLINGVAMYXA_BUILDCORE_ARGS=");
    if (used == sizeof(command)) {
        return 1;
    }
    return lm_build_run(lm_lmx_message_thread, command);
}

static int lm_build_full_build(struct LmMessageThread *lm_lmx_message_thread) {
    (void)lm_lmx_message_thread;
    char *cmake_tool;
    char *build_dir;
    char command[4096];
    size_t used;
    cmake_tool = lm_build_env_or_default(lm_lmx_message_thread, "LM_CMAKE", lm_build_default_cmake(lm_lmx_message_thread));
    build_dir = lm_build_env_or_default(lm_lmx_message_thread, "LM_FULL_BUILD_DIR", "build/libs");
    used = 0U;
    command[0] = '\0';
    used = lm_build_append(lm_lmx_message_thread, command, sizeof(command), used, cmake_tool);
    if (used == sizeof(command)) {
        return 1;
    }
    used = lm_build_append_arg(lm_lmx_message_thread, command, sizeof(command), used, "--build");
    if (used == sizeof(command)) {
        return 1;
    }
    used = lm_build_append_arg(lm_lmx_message_thread, command, sizeof(command), used, build_dir);
    if (used == sizeof(command)) {
        return 1;
    }
    used = lm_build_append_arg(lm_lmx_message_thread, command, sizeof(command), used, "--target");
    if (used == sizeof(command)) {
        return 1;
    }
    used = lm_build_append_arg(lm_lmx_message_thread, command, sizeof(command), used, "third_party.lm0");
    if (used == sizeof(command)) {
        return 1;
    }
    return lm_build_run(lm_lmx_message_thread, command);
}

static int lm_build_full_project(struct LmMessageThread *lm_lmx_message_thread) {
    (void)lm_lmx_message_thread;
    if (lm_build_extract_third_party_zips(lm_lmx_message_thread) != 0) {
        return 1;
    }
    if (lm_build_full_configure(lm_lmx_message_thread) != 0) {
        return 1;
    }
    return lm_build_full_build(lm_lmx_message_thread);
}

static int lm_build_run_bootstrap(struct LmMessageThread *lm_lmx_message_thread, LmBuildOptions *options, char *trusted_make, char *built_trans) {
    (void)lm_lmx_message_thread;
    char *output_dir;
    char parser_library[512];
    char own_library[512];
    output_dir = lm_build_output_dir(lm_lmx_message_thread, options);
    snprintf(parser_library, sizeof(parser_library), "%s/libparser.lm0.a", output_dir);
    snprintf(own_library, sizeof(own_library), "%s/libown.lm0.a", output_dir);
    if (lm_build_make(lm_lmx_message_thread, trusted_make, "mkdir", "\"lm1/build\" \"build/obj\" \"build/lm0\" \"build/lm0/next\" \"build/lm0/next/check\"") != 0) {
        return 1;
    }
    if (lm_build_generate_all(lm_lmx_message_thread, built_trans) != 0) {
        return 1;
    }
    if (lm_build_parser_library(lm_lmx_message_thread, trusted_make, output_dir) != 0) {
        return 1;
    }
    if (lm_build_own_library(lm_lmx_message_thread, trusted_make, output_dir) != 0) {
        return 1;
    }
    if (lm_build_compile_trans(lm_lmx_message_thread, trusted_make, output_dir, parser_library, own_library) != 0) {
        return 1;
    }
    if (lm_build_generate_all(lm_lmx_message_thread, built_trans) != 0) {
        return 1;
    }
    if (lm_build_parser_library(lm_lmx_message_thread, trusted_make, output_dir) != 0) {
        return 1;
    }
    if (lm_build_own_library(lm_lmx_message_thread, trusted_make, output_dir) != 0) {
        return 1;
    }
    if (lm_build_trans_library(lm_lmx_message_thread, trusted_make, output_dir) != 0) {
        return 1;
    }
    if (lm_build_compile_trans(lm_lmx_message_thread, trusted_make, output_dir, parser_library, own_library) != 0) {
        return 1;
    }
    if (lm_build_compile_generated_tools(lm_lmx_message_thread, trusted_make, output_dir, parser_library, own_library) != 0) {
        return 1;
    }
    if (options -> next_build) {
        return 0;
    }
    if (lm_build_run_canary(lm_lmx_message_thread) != 0) {
        fprintf(stderr, "buildCore.lm0: staged bootstrap tools failed the --next rebuild; live tools were not overwritten\n");
        return 1;
    }
    fprintf(stderr, "buildCore.lm0: running staged tests (%s)\n", lm_build_test_suite_name(lm_lmx_message_thread, options -> test_suite));
    if (lm_build_setenv(lm_lmx_message_thread, "LM_TEST_SUITE", lm_build_test_suite_name(lm_lmx_message_thread, options -> test_suite)) != 0) {
        fprintf(stderr, "buildCore.lm0: cannot set LM_TEST_SUITE\n");
        return 1;
    }
    if (lm_build_run_staged_tests(lm_lmx_message_thread, trusted_make, output_dir, parser_library, own_library) != 0) {
        fprintf(stderr, "buildCore.lm0: staged bootstrap tools failed tests; live tools were not overwritten\n");
        return 1;
    }
    if (options -> full_build) {
        if (lm_build_full_project(lm_lmx_message_thread) != 0) {
            return 1;
        }
        return lm_build_defer_finalize(lm_lmx_message_thread);
    }
    return lm_build_defer_finalize(lm_lmx_message_thread);
}

int main(int argc, char **argv) {
    struct LmMessageThread *lm_lmx_message_thread;
    struct LmMessageThread *lm_lmx_module_private_1_main_lm_lmx_message_thread;
    LmMessageThreadExecutionContext lm_lmx_module_private_1_main_lm_message_thread_main_context = {0};
    int lm_lmx_module_private_1_main_lm_message_thread_exit_status;
    struct LmMessageThreadRuntime *lm_lmx_module_private_1_main_lm_lmx_application_runtime = 0;
    int lm_lmx_module_private_1_main_lm_lmx_application_root_attached = 0;
    int lm_lmx_module_private_1_main_lm_lmx_thread_startup_failed = 0;
    int lm_lmx_module_private_1_main_lm_lmx_thread_cleanup_failed = 0;
    int lm_lmx_module_private_1_main_lm_lmx_application_controller_failure = 0;
    int lm_lmx_module_private_1_main_lm_lmx_application_exit_requested = 0;
    int lm_lmx_module_private_1_main_lm_lmx_application_exit_ready = 0;
    int lm_lmx_module_private_1_main_lm_lmx_application_exit_status = 0;
    int lm_lmx_module_private_1_main_lm_lmx_application_exit_snapshot_requested = 0;
    int lm_lmx_module_private_1_main_lm_lmx_application_exit_snapshot_ready = 0;
    int lm_lmx_module_private_1_main_lm_lmx_application_exit_snapshot_status = 0;
    lm_lmx_module_private_1_main_lm_lmx_message_thread = lm_message_thread_new();
    if (lm_lmx_module_private_1_main_lm_lmx_message_thread == 0) {
        return 1;
    }
    lm_lmx_message_thread = lm_lmx_module_private_1_main_lm_lmx_message_thread;
    (void)lm_lmx_message_thread;
    (void)lm_message_thread_set_execution_context(lm_lmx_module_private_1_main_lm_lmx_message_thread, &lm_lmx_module_private_1_main_lm_message_thread_main_context);
    lm_lmx_module_private_1_main_lm_lmx_application_runtime = lm_message_thread_runtime_new();
    if (lm_lmx_module_private_1_main_lm_lmx_application_runtime == 0) lm_lmx_module_private_1_main_lm_lmx_thread_startup_failed = 1;
#ifdef LM_REST_LMX_INSTALL_DEFAULT_CLIENT
    if (!lm_lmx_module_private_1_main_lm_lmx_thread_startup_failed && lm_rest_lmx_http_client_install_default(lm_lmx_module_private_1_main_lm_lmx_application_runtime) != 0) {
        (void)lm_message_thread_runtime_delete(lm_lmx_module_private_1_main_lm_lmx_application_runtime);
        lm_lmx_module_private_1_main_lm_lmx_application_runtime = 0;
        lm_message_thread_delete(lm_lmx_module_private_1_main_lm_lmx_message_thread);
        return 1;
    }
#endif
    if (!lm_lmx_module_private_1_main_lm_lmx_thread_startup_failed && lm_message_thread_runtime_attach_root(lm_lmx_module_private_1_main_lm_lmx_application_runtime, lm_lmx_module_private_1_main_lm_lmx_message_thread) != 0) lm_lmx_module_private_1_main_lm_lmx_thread_startup_failed = 1; else if (!lm_lmx_module_private_1_main_lm_lmx_thread_startup_failed) lm_lmx_module_private_1_main_lm_lmx_application_root_attached = 1;
    if (!lm_lmx_module_private_1_main_lm_lmx_thread_startup_failed) {
        lm_lmx_module_private_1_main_lm_message_thread_main_context.diagnostic_code = 0;
        if (setjmp(lm_lmx_module_private_1_main_lm_message_thread_main_context.diagnostic_root) == 0) {
            if (lm_lmx_module_private_1_init_all(lm_lmx_module_private_1_main_lm_lmx_message_thread, lm_message_thread_owner(lm_lmx_module_private_1_main_lm_lmx_message_thread)) != 0) lm_lmx_module_private_1_main_lm_lmx_thread_startup_failed = 1;
        } else {
            lm_lmx_module_private_1_main_lm_lmx_thread_startup_failed = 1;
        }
    }
    if (lm_lmx_module_private_1_main_lm_lmx_thread_startup_failed) lm_message_thread_request_failure(lm_lmx_module_private_1_main_lm_lmx_message_thread, 1);
    while (lm_message_thread_begin_turn(lm_lmx_module_private_1_main_lm_lmx_message_thread)) {
        if (lm_lmx_module_private_1_main_lm_lmx_application_controller_failure) {
            lm_message_thread_request_failure(lm_lmx_module_private_1_main_lm_lmx_message_thread, 1);
            goto lm_lmx_module_private_1_main_lm_message_thread_turn_end;
        }
        if (lm_lmx_module_private_1_main_lm_lmx_application_exit_ready) {
            lm_message_thread_request_stop(lm_lmx_module_private_1_main_lm_lmx_message_thread, lm_lmx_module_private_1_main_lm_lmx_application_exit_status);
            goto lm_lmx_module_private_1_main_lm_message_thread_turn_end;
        }
        lm_lmx_module_private_1_main_lm_message_thread_main_context.diagnostic_code = 0;
        if (setjmp(lm_lmx_module_private_1_main_lm_message_thread_main_context.diagnostic_root) == 0) {
            char *trusted_make;
            char trusted_make_buffer[128];
            char built_trans_buffer[128];
            LmBuildOptions * options;
            int parse_status;
            int result;
            options = lm_build_options_new(lm_lmx_module_private_1_main_lm_lmx_message_thread);
            if (options == 0) {
                {
                    int lm_return_0 = 1;
                    lm_message_thread_request_stop(lm_lmx_module_private_1_main_lm_lmx_message_thread, lm_return_0);
                    goto lm_lmx_module_private_1_main_lm_message_thread_turn_end;
                }
            }
            parse_status = lm_build_parse_options(lm_lmx_module_private_1_main_lm_lmx_message_thread, argc, argv, options);
            if (parse_status == 2) {
                lm_build_options_delete(lm_lmx_module_private_1_main_lm_lmx_message_thread, options);
                {
                    int lm_return_1 = 0;
                    lm_message_thread_request_stop(lm_lmx_module_private_1_main_lm_lmx_message_thread, lm_return_1);
                    goto lm_lmx_module_private_1_main_lm_message_thread_turn_end;
                }
            }
            if (parse_status != 0) {
                lm_build_options_delete(lm_lmx_module_private_1_main_lm_lmx_message_thread, options);
                {
                    int lm_return_2 = 1;
                    lm_message_thread_request_stop(lm_lmx_module_private_1_main_lm_lmx_message_thread, lm_return_2);
                    goto lm_lmx_module_private_1_main_lm_message_thread_turn_end;
                }
            }
            if (lm_build_enter_project_root(lm_lmx_module_private_1_main_lm_lmx_message_thread, argv[0]) != 0) {
                lm_build_options_delete(lm_lmx_module_private_1_main_lm_lmx_message_thread, options);
                {
                    int lm_return_3 = 1;
                    lm_message_thread_request_stop(lm_lmx_module_private_1_main_lm_lmx_message_thread, lm_return_3);
                    goto lm_lmx_module_private_1_main_lm_message_thread_turn_end;
                }
            }
            if (lm_build_thread_provider(lm_lmx_module_private_1_main_lm_lmx_message_thread) == 0) {
                lm_build_options_delete(lm_lmx_module_private_1_main_lm_lmx_message_thread, options);
                {
                    int lm_return_4 = 1;
                    lm_message_thread_request_stop(lm_lmx_module_private_1_main_lm_lmx_message_thread, lm_return_4);
                    goto lm_lmx_module_private_1_main_lm_message_thread_turn_end;
                }
            }
            snprintf(trusted_make_buffer, sizeof(trusted_make_buffer), "build%slm0%smake.lm0%s", lm_build_path_sep(lm_lmx_module_private_1_main_lm_lmx_message_thread), lm_build_path_sep(lm_lmx_module_private_1_main_lm_lmx_message_thread), lm_build_exe_suffix(lm_lmx_module_private_1_main_lm_lmx_message_thread));
            snprintf(built_trans_buffer, sizeof(built_trans_buffer), "build%slm0%strans.lm0%s", lm_build_path_sep(lm_lmx_module_private_1_main_lm_lmx_message_thread), lm_build_path_sep(lm_lmx_module_private_1_main_lm_lmx_message_thread), lm_build_exe_suffix(lm_lmx_module_private_1_main_lm_lmx_message_thread));
            if (options -> next_build) {
                snprintf(trusted_make_buffer, sizeof(trusted_make_buffer), "build%slm0%snext%smake.lm0%s", lm_build_path_sep(lm_lmx_module_private_1_main_lm_lmx_message_thread), lm_build_path_sep(lm_lmx_module_private_1_main_lm_lmx_message_thread), lm_build_path_sep(lm_lmx_module_private_1_main_lm_lmx_message_thread), lm_build_exe_suffix(lm_lmx_module_private_1_main_lm_lmx_message_thread));
                snprintf(built_trans_buffer, sizeof(built_trans_buffer), "build%slm0%snext%strans.lm0%s", lm_build_path_sep(lm_lmx_module_private_1_main_lm_lmx_message_thread), lm_build_path_sep(lm_lmx_module_private_1_main_lm_lmx_message_thread), lm_build_path_sep(lm_lmx_module_private_1_main_lm_lmx_message_thread), lm_build_exe_suffix(lm_lmx_module_private_1_main_lm_lmx_message_thread));
            }
            trusted_make = lm_build_env_or_default(lm_lmx_module_private_1_main_lm_lmx_message_thread, "LM_MAKE", trusted_make_buffer);
            result = lm_build_run_bootstrap(lm_lmx_module_private_1_main_lm_lmx_message_thread, options, trusted_make, built_trans_buffer);
            lm_build_options_delete(lm_lmx_module_private_1_main_lm_lmx_message_thread, options);
            {
                int lm_return_5 = result;
                lm_message_thread_request_stop(lm_lmx_module_private_1_main_lm_lmx_message_thread, lm_return_5);
                goto lm_lmx_module_private_1_main_lm_message_thread_turn_end;
            }
        } else {
            lm_message_thread_request_failure(lm_lmx_module_private_1_main_lm_lmx_message_thread, lm_message_thread_diagnostic_status(&lm_lmx_module_private_1_main_lm_message_thread_main_context));
        }
    lm_lmx_module_private_1_main_lm_message_thread_turn_end:
        (void)lm_message_thread_end_turn(lm_lmx_module_private_1_main_lm_lmx_message_thread);
        lm_lmx_module_private_1_main_lm_lmx_application_exit_snapshot_requested = 0;
        lm_lmx_module_private_1_main_lm_lmx_application_exit_snapshot_ready = 0;
        lm_lmx_module_private_1_main_lm_lmx_application_exit_snapshot_status = 0;
        if (lm_lmx_module_private_1_main_lm_lmx_application_runtime != 0) {
            if (lm_message_thread_runtime_exit_state(lm_lmx_module_private_1_main_lm_lmx_application_runtime, &lm_lmx_module_private_1_main_lm_lmx_application_exit_snapshot_requested, &lm_lmx_module_private_1_main_lm_lmx_application_exit_snapshot_ready, &lm_lmx_module_private_1_main_lm_lmx_application_exit_snapshot_status) != 0) {
                lm_lmx_module_private_1_main_lm_lmx_thread_cleanup_failed = 1;
                lm_lmx_module_private_1_main_lm_lmx_application_controller_failure = 1;
            } else {
                lm_lmx_module_private_1_main_lm_lmx_application_exit_requested = lm_lmx_module_private_1_main_lm_lmx_application_exit_snapshot_requested;
                lm_lmx_module_private_1_main_lm_lmx_application_exit_ready = lm_lmx_module_private_1_main_lm_lmx_application_exit_snapshot_ready;
                lm_lmx_module_private_1_main_lm_lmx_application_exit_status = lm_lmx_module_private_1_main_lm_lmx_application_exit_snapshot_status;
            }
        }
    }
    lm_lmx_module_private_1_main_lm_lmx_application_exit_snapshot_requested = 0;
    lm_lmx_module_private_1_main_lm_lmx_application_exit_snapshot_ready = 0;
    lm_lmx_module_private_1_main_lm_lmx_application_exit_snapshot_status = 0;
    if (lm_lmx_module_private_1_main_lm_lmx_application_runtime != 0) {
        if (lm_message_thread_runtime_exit_state(lm_lmx_module_private_1_main_lm_lmx_application_runtime, &lm_lmx_module_private_1_main_lm_lmx_application_exit_snapshot_requested, &lm_lmx_module_private_1_main_lm_lmx_application_exit_snapshot_ready, &lm_lmx_module_private_1_main_lm_lmx_application_exit_snapshot_status) != 0) {
            lm_lmx_module_private_1_main_lm_lmx_thread_cleanup_failed = 1;
            lm_lmx_module_private_1_main_lm_lmx_application_controller_failure = 1;
        } else {
            lm_lmx_module_private_1_main_lm_lmx_application_exit_requested = lm_lmx_module_private_1_main_lm_lmx_application_exit_snapshot_requested;
            lm_lmx_module_private_1_main_lm_lmx_application_exit_ready = lm_lmx_module_private_1_main_lm_lmx_application_exit_snapshot_ready;
            lm_lmx_module_private_1_main_lm_lmx_application_exit_status = lm_lmx_module_private_1_main_lm_lmx_application_exit_snapshot_status;
        }
    }
    lm_lmx_module_private_1_main_lm_message_thread_exit_status = lm_message_thread_status(lm_lmx_module_private_1_main_lm_lmx_message_thread);
    if (lm_lmx_module_private_1_main_lm_lmx_application_runtime != 0 && lm_lmx_module_private_1_main_lm_lmx_application_root_attached) {
        if (lm_message_thread_runtime_detach_root(lm_lmx_module_private_1_main_lm_lmx_application_runtime, lm_lmx_module_private_1_main_lm_lmx_message_thread) != 0) lm_lmx_module_private_1_main_lm_lmx_thread_cleanup_failed = 1; else lm_lmx_module_private_1_main_lm_lmx_application_root_attached = 0;
    }
    if (lm_lmx_module_private_1_main_lm_lmx_application_runtime != 0 && !lm_lmx_module_private_1_main_lm_lmx_application_root_attached) {
        if (lm_message_thread_runtime_delete(lm_lmx_module_private_1_main_lm_lmx_application_runtime) != 0) lm_lmx_module_private_1_main_lm_lmx_thread_cleanup_failed = 1; else lm_lmx_module_private_1_main_lm_lmx_application_runtime = 0;
    }
    if (lm_lmx_module_private_1_main_lm_message_thread_exit_status == 0 && lm_lmx_module_private_1_main_lm_lmx_application_exit_requested && lm_lmx_module_private_1_main_lm_lmx_application_exit_status != 0) lm_lmx_module_private_1_main_lm_message_thread_exit_status = lm_lmx_module_private_1_main_lm_lmx_application_exit_status;
    if (lm_lmx_module_private_1_main_lm_message_thread_exit_status == 0 && lm_lmx_module_private_1_main_lm_lmx_thread_cleanup_failed) lm_lmx_module_private_1_main_lm_message_thread_exit_status = 1;
    if (!lm_lmx_module_private_1_main_lm_lmx_application_root_attached) lm_message_thread_delete(lm_lmx_module_private_1_main_lm_lmx_message_thread);
    return lm_lmx_module_private_1_main_lm_message_thread_exit_status;
}
