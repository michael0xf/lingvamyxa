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
void * (lm_own_arena_copy_graph)(struct LmMessageThread *lm_lmx_message_thread, LmOwnArena *arena, const void *source);
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











typedef struct LmLmxStructure LmLmxStructure;
struct LmLmxStructure {
    LmLmxStructure *descriptor;
};
static LmLmxStructure lm_lmx_structure_descriptor;
static void *lm_lmx_structure_new_in(struct LmMessageThread *thread, struct LmOwnArena *arena, LmLmxStructure *descriptor, size_t size)
{
    LmLmxStructure *chunk;
    if (size < sizeof(LmLmxStructure))
        size = sizeof(LmLmxStructure);
    if (lm_lmx_structure_descriptor.descriptor == 0)
        lm_lmx_structure_descriptor.descriptor = &lm_lmx_structure_descriptor;
    if (descriptor != 0 && descriptor->descriptor == 0)
        descriptor->descriptor = &lm_lmx_structure_descriptor;
    chunk = (LmLmxStructure *)lm_own_arena_new_zero(thread, arena, size);
    if (chunk == 0)
        abort();
    chunk->descriptor = descriptor;
    return chunk;
}
#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_new_zero
#define lm_lmx_module_private_1_typedef_defined_lm_own_new_zero 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_new_zero 0x1a5ee7e7a806ce25ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_new_zero 0x7d168d2170106866ULL
typedef struct lm_own_new_zeroClosure *lm_own_new_zero;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_new_zero) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_new_zero) || lm_lmx_module_private_1_typedef_id_a_lm_own_new_zero != 0x1a5ee7e7a806ce25ULL || lm_lmx_module_private_1_typedef_id_b_lm_own_new_zero != 0x7d168d2170106866ULL
#error "Lingvamyxa conflicting typedef projection for lm_own_new_zero"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_new_zeroClosure
#define lm_lmx_module_private_1_typedef_defined_lm_own_new_zeroClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_new_zeroClosure 0x17daa2511da8c8ecULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_new_zeroClosure 0x2596537755c821ddULL
typedef struct lm_own_new_zeroClosure lm_own_new_zeroClosure;
struct lm_own_new_zeroClosure {
    LmLmxStructure *descriptor;
    void * (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, size_t);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_own_new_zero = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_new_zeroClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_new_zeroClosure) || lm_lmx_module_private_1_typedef_id_a_lm_own_new_zeroClosure != 0x17daa2511da8c8ecULL || lm_lmx_module_private_1_typedef_id_b_lm_own_new_zeroClosure != 0x2596537755c821ddULL
#error "Lingvamyxa conflicting typedef projection for lm_own_new_zeroClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_resize
#define lm_lmx_module_private_1_typedef_defined_lm_own_resize 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_resize 0xb492ee947fec77a9ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_resize 0x20dda90a019fe6f2ULL
typedef struct lm_own_resizeClosure *lm_own_resize;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_resize) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_resize) || lm_lmx_module_private_1_typedef_id_a_lm_own_resize != 0xb492ee947fec77a9ULL || lm_lmx_module_private_1_typedef_id_b_lm_own_resize != 0x20dda90a019fe6f2ULL
#error "Lingvamyxa conflicting typedef projection for lm_own_resize"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_resizeClosure
#define lm_lmx_module_private_1_typedef_defined_lm_own_resizeClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_resizeClosure 0xe8190a6373d0ed62ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_resizeClosure 0x1bf90d02cbd7282bULL
typedef struct lm_own_resizeClosure lm_own_resizeClosure;
struct lm_own_resizeClosure {
    LmLmxStructure *descriptor;
    void * (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, void *, size_t);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_own_resize = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_resizeClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_resizeClosure) || lm_lmx_module_private_1_typedef_id_a_lm_own_resizeClosure != 0xe8190a6373d0ed62ULL || lm_lmx_module_private_1_typedef_id_b_lm_own_resizeClosure != 0x1bf90d02cbd7282bULL
#error "Lingvamyxa conflicting typedef projection for lm_own_resizeClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_copy_bytes
#define lm_lmx_module_private_1_typedef_defined_lm_own_copy_bytes 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_copy_bytes 0xa9dc4a66d0b209adULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_copy_bytes 0x9a61bda5bc2290e6ULL
typedef struct lm_own_copy_bytesClosure *lm_own_copy_bytes;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_copy_bytes) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_copy_bytes) || lm_lmx_module_private_1_typedef_id_a_lm_own_copy_bytes != 0xa9dc4a66d0b209adULL || lm_lmx_module_private_1_typedef_id_b_lm_own_copy_bytes != 0x9a61bda5bc2290e6ULL
#error "Lingvamyxa conflicting typedef projection for lm_own_copy_bytes"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_copy_bytesClosure
#define lm_lmx_module_private_1_typedef_defined_lm_own_copy_bytesClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_copy_bytesClosure 0xddf4f99ffcfd44bdULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_copy_bytesClosure 0x57de27f4757e4410ULL
typedef struct lm_own_copy_bytesClosure lm_own_copy_bytesClosure;
struct lm_own_copy_bytesClosure {
    LmLmxStructure *descriptor;
    char * (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, const char *, size_t);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_own_copy_bytes = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_copy_bytesClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_copy_bytesClosure) || lm_lmx_module_private_1_typedef_id_a_lm_own_copy_bytesClosure != 0xddf4f99ffcfd44bdULL || lm_lmx_module_private_1_typedef_id_b_lm_own_copy_bytesClosure != 0x57de27f4757e4410ULL
#error "Lingvamyxa conflicting typedef projection for lm_own_copy_bytesClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_delete
#define lm_lmx_module_private_1_typedef_defined_lm_own_delete 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_delete 0xeaefcb76a84f4a0dULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_delete 0xaf75653727db2ceeULL
typedef struct lm_own_deleteClosure *lm_own_delete;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_delete) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_delete) || lm_lmx_module_private_1_typedef_id_a_lm_own_delete != 0xeaefcb76a84f4a0dULL || lm_lmx_module_private_1_typedef_id_b_lm_own_delete != 0xaf75653727db2ceeULL
#error "Lingvamyxa conflicting typedef projection for lm_own_delete"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_deleteClosure
#define lm_lmx_module_private_1_typedef_defined_lm_own_deleteClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_deleteClosure 0x26dfe65340d9bff8ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_deleteClosure 0x4ea9d339ae1dc005ULL
typedef struct lm_own_deleteClosure lm_own_deleteClosure;
struct lm_own_deleteClosure {
    LmLmxStructure *descriptor;
    void (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, void *, LmOwnDestroyFields);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_own_delete = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_deleteClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_deleteClosure) || lm_lmx_module_private_1_typedef_id_a_lm_own_deleteClosure != 0x26dfe65340d9bff8ULL || lm_lmx_module_private_1_typedef_id_b_lm_own_deleteClosure != 0x4ea9d339ae1dc005ULL
#error "Lingvamyxa conflicting typedef projection for lm_own_deleteClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_delete_plain
#define lm_lmx_module_private_1_typedef_defined_lm_own_delete_plain 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_delete_plain 0x6a0d2cf1d2d268a1ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_delete_plain 0x1f1ba7195a536afaULL
typedef struct lm_own_delete_plainClosure *lm_own_delete_plain;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_delete_plain) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_delete_plain) || lm_lmx_module_private_1_typedef_id_a_lm_own_delete_plain != 0x6a0d2cf1d2d268a1ULL || lm_lmx_module_private_1_typedef_id_b_lm_own_delete_plain != 0x1f1ba7195a536afaULL
#error "Lingvamyxa conflicting typedef projection for lm_own_delete_plain"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_delete_plainClosure
#define lm_lmx_module_private_1_typedef_defined_lm_own_delete_plainClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_delete_plainClosure 0xbf36f0247e879f2eULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_delete_plainClosure 0x5218beb6f4cf5bafULL
typedef struct lm_own_delete_plainClosure lm_own_delete_plainClosure;
struct lm_own_delete_plainClosure {
    LmLmxStructure *descriptor;
    void (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, void *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_own_delete_plain = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_delete_plainClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_delete_plainClosure) || lm_lmx_module_private_1_typedef_id_a_lm_own_delete_plainClosure != 0xbf36f0247e879f2eULL || lm_lmx_module_private_1_typedef_id_b_lm_own_delete_plainClosure != 0x5218beb6f4cf5bafULL
#error "Lingvamyxa conflicting typedef projection for lm_own_delete_plainClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_pointer_array_delete
#define lm_lmx_module_private_1_typedef_defined_lm_own_pointer_array_delete 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_pointer_array_delete 0x0e22ca660367d2e9ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_pointer_array_delete 0xf81518d8003758a2ULL
typedef struct lm_own_pointer_array_deleteClosure *lm_own_pointer_array_delete;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_pointer_array_delete) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_pointer_array_delete) || lm_lmx_module_private_1_typedef_id_a_lm_own_pointer_array_delete != 0x0e22ca660367d2e9ULL || lm_lmx_module_private_1_typedef_id_b_lm_own_pointer_array_delete != 0xf81518d8003758a2ULL
#error "Lingvamyxa conflicting typedef projection for lm_own_pointer_array_delete"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_pointer_array_deleteClosure
#define lm_lmx_module_private_1_typedef_defined_lm_own_pointer_array_deleteClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_pointer_array_deleteClosure 0x0de78b6791fe80deULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_pointer_array_deleteClosure 0xbb3fc7fa4986e857ULL
typedef struct lm_own_pointer_array_deleteClosure lm_own_pointer_array_deleteClosure;
struct lm_own_pointer_array_deleteClosure {
    LmLmxStructure *descriptor;
    void (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, void **, size_t, LmOwnDelete);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_own_pointer_array_delete = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_pointer_array_deleteClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_pointer_array_deleteClosure) || lm_lmx_module_private_1_typedef_id_a_lm_own_pointer_array_deleteClosure != 0x0de78b6791fe80deULL || lm_lmx_module_private_1_typedef_id_b_lm_own_pointer_array_deleteClosure != 0xbb3fc7fa4986e857ULL
#error "Lingvamyxa conflicting typedef projection for lm_own_pointer_array_deleteClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_ptr_stack_init
#define lm_lmx_module_private_1_typedef_defined_lm_own_ptr_stack_init 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_ptr_stack_init 0x76158f89c1ea3619ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_ptr_stack_init 0xf20eb058d5e6ba72ULL
typedef struct lm_own_ptr_stack_initClosure *lm_own_ptr_stack_init;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_ptr_stack_init) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_ptr_stack_init) || lm_lmx_module_private_1_typedef_id_a_lm_own_ptr_stack_init != 0x76158f89c1ea3619ULL || lm_lmx_module_private_1_typedef_id_b_lm_own_ptr_stack_init != 0xf20eb058d5e6ba72ULL
#error "Lingvamyxa conflicting typedef projection for lm_own_ptr_stack_init"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_ptr_stack_initClosure
#define lm_lmx_module_private_1_typedef_defined_lm_own_ptr_stack_initClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_ptr_stack_initClosure 0x6243941a6ff43d7fULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_ptr_stack_initClosure 0x4076aa7f19ce5c0eULL
typedef struct lm_own_ptr_stack_initClosure lm_own_ptr_stack_initClosure;
struct lm_own_ptr_stack_initClosure {
    LmLmxStructure *descriptor;
    void (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, LmOwnPtrStack *, LmOwnDelete);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_own_ptr_stack_init = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_ptr_stack_initClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_ptr_stack_initClosure) || lm_lmx_module_private_1_typedef_id_a_lm_own_ptr_stack_initClosure != 0x6243941a6ff43d7fULL || lm_lmx_module_private_1_typedef_id_b_lm_own_ptr_stack_initClosure != 0x4076aa7f19ce5c0eULL
#error "Lingvamyxa conflicting typedef projection for lm_own_ptr_stack_initClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_ptr_stack_destroy
#define lm_lmx_module_private_1_typedef_defined_lm_own_ptr_stack_destroy 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_ptr_stack_destroy 0xefb16ee381898a19ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_ptr_stack_destroy 0x9608806b369d96aeULL
typedef struct lm_own_ptr_stack_destroyClosure *lm_own_ptr_stack_destroy;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_ptr_stack_destroy) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_ptr_stack_destroy) || lm_lmx_module_private_1_typedef_id_a_lm_own_ptr_stack_destroy != 0xefb16ee381898a19ULL || lm_lmx_module_private_1_typedef_id_b_lm_own_ptr_stack_destroy != 0x9608806b369d96aeULL
#error "Lingvamyxa conflicting typedef projection for lm_own_ptr_stack_destroy"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_ptr_stack_destroyClosure
#define lm_lmx_module_private_1_typedef_defined_lm_own_ptr_stack_destroyClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_ptr_stack_destroyClosure 0x9aa26a149fb85027ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_ptr_stack_destroyClosure 0x199da85b1a754c30ULL
typedef struct lm_own_ptr_stack_destroyClosure lm_own_ptr_stack_destroyClosure;
struct lm_own_ptr_stack_destroyClosure {
    LmLmxStructure *descriptor;
    void (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, LmOwnPtrStack *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_own_ptr_stack_destroy = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_ptr_stack_destroyClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_ptr_stack_destroyClosure) || lm_lmx_module_private_1_typedef_id_a_lm_own_ptr_stack_destroyClosure != 0x9aa26a149fb85027ULL || lm_lmx_module_private_1_typedef_id_b_lm_own_ptr_stack_destroyClosure != 0x199da85b1a754c30ULL
#error "Lingvamyxa conflicting typedef projection for lm_own_ptr_stack_destroyClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_ptr_stack_push
#define lm_lmx_module_private_1_typedef_defined_lm_own_ptr_stack_push 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_ptr_stack_push 0x0fe29c1d8b342399ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_ptr_stack_push 0x076482f49041ad42ULL
typedef struct lm_own_ptr_stack_pushClosure *lm_own_ptr_stack_push;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_ptr_stack_push) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_ptr_stack_push) || lm_lmx_module_private_1_typedef_id_a_lm_own_ptr_stack_push != 0x0fe29c1d8b342399ULL || lm_lmx_module_private_1_typedef_id_b_lm_own_ptr_stack_push != 0x076482f49041ad42ULL
#error "Lingvamyxa conflicting typedef projection for lm_own_ptr_stack_push"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_ptr_stack_pushClosure
#define lm_lmx_module_private_1_typedef_defined_lm_own_ptr_stack_pushClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_ptr_stack_pushClosure 0xff48c56fd57368faULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_ptr_stack_pushClosure 0x48297aa6aedf9ce3ULL
typedef struct lm_own_ptr_stack_pushClosure lm_own_ptr_stack_pushClosure;
struct lm_own_ptr_stack_pushClosure {
    LmLmxStructure *descriptor;
    int (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, LmOwnPtrStack *, void *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_own_ptr_stack_push = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_ptr_stack_pushClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_ptr_stack_pushClosure) || lm_lmx_module_private_1_typedef_id_a_lm_own_ptr_stack_pushClosure != 0xff48c56fd57368faULL || lm_lmx_module_private_1_typedef_id_b_lm_own_ptr_stack_pushClosure != 0x48297aa6aedf9ce3ULL
#error "Lingvamyxa conflicting typedef projection for lm_own_ptr_stack_pushClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_ptr_stack_pop
#define lm_lmx_module_private_1_typedef_defined_lm_own_ptr_stack_pop 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_ptr_stack_pop 0xf1f9057c26d2b4e3ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_ptr_stack_pop 0xd6c8cf695f6355c8ULL
typedef struct lm_own_ptr_stack_popClosure *lm_own_ptr_stack_pop;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_ptr_stack_pop) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_ptr_stack_pop) || lm_lmx_module_private_1_typedef_id_a_lm_own_ptr_stack_pop != 0xf1f9057c26d2b4e3ULL || lm_lmx_module_private_1_typedef_id_b_lm_own_ptr_stack_pop != 0xd6c8cf695f6355c8ULL
#error "Lingvamyxa conflicting typedef projection for lm_own_ptr_stack_pop"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_ptr_stack_popClosure
#define lm_lmx_module_private_1_typedef_defined_lm_own_ptr_stack_popClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_ptr_stack_popClosure 0xeb68b0a0813f31b1ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_ptr_stack_popClosure 0x5b7416e094a0ed5aULL
typedef struct lm_own_ptr_stack_popClosure lm_own_ptr_stack_popClosure;
struct lm_own_ptr_stack_popClosure {
    LmLmxStructure *descriptor;
    void * (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, LmOwnPtrStack *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_own_ptr_stack_pop = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_ptr_stack_popClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_ptr_stack_popClosure) || lm_lmx_module_private_1_typedef_id_a_lm_own_ptr_stack_popClosure != 0xeb68b0a0813f31b1ULL || lm_lmx_module_private_1_typedef_id_b_lm_own_ptr_stack_popClosure != 0x5b7416e094a0ed5aULL
#error "Lingvamyxa conflicting typedef projection for lm_own_ptr_stack_popClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_ptr_stack_at
#define lm_lmx_module_private_1_typedef_defined_lm_own_ptr_stack_at 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_ptr_stack_at 0x1aa08042e9f7b459ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_ptr_stack_at 0xaf9d3eaf0745d772ULL
typedef struct lm_own_ptr_stack_atClosure *lm_own_ptr_stack_at;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_ptr_stack_at) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_ptr_stack_at) || lm_lmx_module_private_1_typedef_id_a_lm_own_ptr_stack_at != 0x1aa08042e9f7b459ULL || lm_lmx_module_private_1_typedef_id_b_lm_own_ptr_stack_at != 0xaf9d3eaf0745d772ULL
#error "Lingvamyxa conflicting typedef projection for lm_own_ptr_stack_at"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_ptr_stack_atClosure
#define lm_lmx_module_private_1_typedef_defined_lm_own_ptr_stack_atClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_ptr_stack_atClosure 0x33b68d1fe618cf38ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_ptr_stack_atClosure 0x43449fa42f3a2c9dULL
typedef struct lm_own_ptr_stack_atClosure lm_own_ptr_stack_atClosure;
struct lm_own_ptr_stack_atClosure {
    LmLmxStructure *descriptor;
    void * (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, const LmOwnPtrStack *, size_t);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_own_ptr_stack_at = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_ptr_stack_atClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_ptr_stack_atClosure) || lm_lmx_module_private_1_typedef_id_a_lm_own_ptr_stack_atClosure != 0x33b68d1fe618cf38ULL || lm_lmx_module_private_1_typedef_id_b_lm_own_ptr_stack_atClosure != 0x43449fa42f3a2c9dULL
#error "Lingvamyxa conflicting typedef projection for lm_own_ptr_stack_atClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_ptr_stack_top
#define lm_lmx_module_private_1_typedef_defined_lm_own_ptr_stack_top 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_ptr_stack_top 0x2b33ea33328feb3bULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_ptr_stack_top 0xbd93078743c0e0c8ULL
typedef struct lm_own_ptr_stack_topClosure *lm_own_ptr_stack_top;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_ptr_stack_top) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_ptr_stack_top) || lm_lmx_module_private_1_typedef_id_a_lm_own_ptr_stack_top != 0x2b33ea33328feb3bULL || lm_lmx_module_private_1_typedef_id_b_lm_own_ptr_stack_top != 0xbd93078743c0e0c8ULL
#error "Lingvamyxa conflicting typedef projection for lm_own_ptr_stack_top"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_ptr_stack_topClosure
#define lm_lmx_module_private_1_typedef_defined_lm_own_ptr_stack_topClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_ptr_stack_topClosure 0xe46971c42b597238ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_ptr_stack_topClosure 0xa84a4657a4ba86b1ULL
typedef struct lm_own_ptr_stack_topClosure lm_own_ptr_stack_topClosure;
struct lm_own_ptr_stack_topClosure {
    LmLmxStructure *descriptor;
    void * (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, const LmOwnPtrStack *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_own_ptr_stack_top = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_ptr_stack_topClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_ptr_stack_topClosure) || lm_lmx_module_private_1_typedef_id_a_lm_own_ptr_stack_topClosure != 0xe46971c42b597238ULL || lm_lmx_module_private_1_typedef_id_b_lm_own_ptr_stack_topClosure != 0xa84a4657a4ba86b1ULL
#error "Lingvamyxa conflicting typedef projection for lm_own_ptr_stack_topClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_ptr_stack_truncate
#define lm_lmx_module_private_1_typedef_defined_lm_own_ptr_stack_truncate 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_ptr_stack_truncate 0x287116c823227811ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_ptr_stack_truncate 0x2a85472fd6a4b7aaULL
typedef struct lm_own_ptr_stack_truncateClosure *lm_own_ptr_stack_truncate;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_ptr_stack_truncate) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_ptr_stack_truncate) || lm_lmx_module_private_1_typedef_id_a_lm_own_ptr_stack_truncate != 0x287116c823227811ULL || lm_lmx_module_private_1_typedef_id_b_lm_own_ptr_stack_truncate != 0x2a85472fd6a4b7aaULL
#error "Lingvamyxa conflicting typedef projection for lm_own_ptr_stack_truncate"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_ptr_stack_truncateClosure
#define lm_lmx_module_private_1_typedef_defined_lm_own_ptr_stack_truncateClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_ptr_stack_truncateClosure 0x7c42d2fcc228b14dULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_ptr_stack_truncateClosure 0x0aa925115ab9bd06ULL
typedef struct lm_own_ptr_stack_truncateClosure lm_own_ptr_stack_truncateClosure;
struct lm_own_ptr_stack_truncateClosure {
    LmLmxStructure *descriptor;
    void (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, LmOwnPtrStack *, size_t);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_own_ptr_stack_truncate = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_ptr_stack_truncateClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_ptr_stack_truncateClosure) || lm_lmx_module_private_1_typedef_id_a_lm_own_ptr_stack_truncateClosure != 0x7c42d2fcc228b14dULL || lm_lmx_module_private_1_typedef_id_b_lm_own_ptr_stack_truncateClosure != 0x0aa925115ab9bd06ULL
#error "Lingvamyxa conflicting typedef projection for lm_own_ptr_stack_truncateClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_value_stack_init
#define lm_lmx_module_private_1_typedef_defined_lm_own_value_stack_init 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_value_stack_init 0x7633c9b615ecdb81ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_value_stack_init 0xe82050513f7e9dd2ULL
typedef struct lm_own_value_stack_initClosure *lm_own_value_stack_init;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_value_stack_init) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_value_stack_init) || lm_lmx_module_private_1_typedef_id_a_lm_own_value_stack_init != 0x7633c9b615ecdb81ULL || lm_lmx_module_private_1_typedef_id_b_lm_own_value_stack_init != 0xe82050513f7e9dd2ULL
#error "Lingvamyxa conflicting typedef projection for lm_own_value_stack_init"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_value_stack_initClosure
#define lm_lmx_module_private_1_typedef_defined_lm_own_value_stack_initClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_value_stack_initClosure 0x0bd70a916d45ae7cULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_value_stack_initClosure 0xc0cb68ec09bdfb6dULL
typedef struct lm_own_value_stack_initClosure lm_own_value_stack_initClosure;
struct lm_own_value_stack_initClosure {
    LmLmxStructure *descriptor;
    void (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, LmOwnValueStack *, size_t);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_own_value_stack_init = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_value_stack_initClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_value_stack_initClosure) || lm_lmx_module_private_1_typedef_id_a_lm_own_value_stack_initClosure != 0x0bd70a916d45ae7cULL || lm_lmx_module_private_1_typedef_id_b_lm_own_value_stack_initClosure != 0xc0cb68ec09bdfb6dULL
#error "Lingvamyxa conflicting typedef projection for lm_own_value_stack_initClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_value_stack_destroy
#define lm_lmx_module_private_1_typedef_defined_lm_own_value_stack_destroy 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_value_stack_destroy 0x86348bbe6299eaebULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_value_stack_destroy 0xbf066d02db504d10ULL
typedef struct lm_own_value_stack_destroyClosure *lm_own_value_stack_destroy;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_value_stack_destroy) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_value_stack_destroy) || lm_lmx_module_private_1_typedef_id_a_lm_own_value_stack_destroy != 0x86348bbe6299eaebULL || lm_lmx_module_private_1_typedef_id_b_lm_own_value_stack_destroy != 0xbf066d02db504d10ULL
#error "Lingvamyxa conflicting typedef projection for lm_own_value_stack_destroy"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_value_stack_destroyClosure
#define lm_lmx_module_private_1_typedef_defined_lm_own_value_stack_destroyClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_value_stack_destroyClosure 0xd42d51f07fe4c6e4ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_value_stack_destroyClosure 0xa7693bf357f6a991ULL
typedef struct lm_own_value_stack_destroyClosure lm_own_value_stack_destroyClosure;
struct lm_own_value_stack_destroyClosure {
    LmLmxStructure *descriptor;
    void (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, LmOwnValueStack *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_own_value_stack_destroy = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_value_stack_destroyClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_value_stack_destroyClosure) || lm_lmx_module_private_1_typedef_id_a_lm_own_value_stack_destroyClosure != 0xd42d51f07fe4c6e4ULL || lm_lmx_module_private_1_typedef_id_b_lm_own_value_stack_destroyClosure != 0xa7693bf357f6a991ULL
#error "Lingvamyxa conflicting typedef projection for lm_own_value_stack_destroyClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_value_stack_push
#define lm_lmx_module_private_1_typedef_defined_lm_own_value_stack_push 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_value_stack_push 0xc2564b6ccf5deb49ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_value_stack_push 0xa0b1f240b92d5262ULL
typedef struct lm_own_value_stack_pushClosure *lm_own_value_stack_push;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_value_stack_push) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_value_stack_push) || lm_lmx_module_private_1_typedef_id_a_lm_own_value_stack_push != 0xc2564b6ccf5deb49ULL || lm_lmx_module_private_1_typedef_id_b_lm_own_value_stack_push != 0xa0b1f240b92d5262ULL
#error "Lingvamyxa conflicting typedef projection for lm_own_value_stack_push"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_value_stack_pushClosure
#define lm_lmx_module_private_1_typedef_defined_lm_own_value_stack_pushClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_value_stack_pushClosure 0xdd26dc097cd09990ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_value_stack_pushClosure 0xcce7ae31108c88e1ULL
typedef struct lm_own_value_stack_pushClosure lm_own_value_stack_pushClosure;
struct lm_own_value_stack_pushClosure {
    LmLmxStructure *descriptor;
    int (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, LmOwnValueStack *, const void *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_own_value_stack_push = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_value_stack_pushClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_value_stack_pushClosure) || lm_lmx_module_private_1_typedef_id_a_lm_own_value_stack_pushClosure != 0xdd26dc097cd09990ULL || lm_lmx_module_private_1_typedef_id_b_lm_own_value_stack_pushClosure != 0xcce7ae31108c88e1ULL
#error "Lingvamyxa conflicting typedef projection for lm_own_value_stack_pushClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_value_stack_resize_zero
#define lm_lmx_module_private_1_typedef_defined_lm_own_value_stack_resize_zero 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_value_stack_resize_zero 0xb3ac4c9326824afdULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_value_stack_resize_zero 0x47b2c1ec071142b2ULL
typedef struct lm_own_value_stack_resize_zeroClosure *lm_own_value_stack_resize_zero;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_value_stack_resize_zero) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_value_stack_resize_zero) || lm_lmx_module_private_1_typedef_id_a_lm_own_value_stack_resize_zero != 0xb3ac4c9326824afdULL || lm_lmx_module_private_1_typedef_id_b_lm_own_value_stack_resize_zero != 0x47b2c1ec071142b2ULL
#error "Lingvamyxa conflicting typedef projection for lm_own_value_stack_resize_zero"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_value_stack_resize_zeroClosure
#define lm_lmx_module_private_1_typedef_defined_lm_own_value_stack_resize_zeroClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_value_stack_resize_zeroClosure 0x7b47d9967133facdULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_value_stack_resize_zeroClosure 0xe5da2db98fd8b138ULL
typedef struct lm_own_value_stack_resize_zeroClosure lm_own_value_stack_resize_zeroClosure;
struct lm_own_value_stack_resize_zeroClosure {
    LmLmxStructure *descriptor;
    int (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, LmOwnValueStack *, size_t);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_own_value_stack_resize_zero = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_value_stack_resize_zeroClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_value_stack_resize_zeroClosure) || lm_lmx_module_private_1_typedef_id_a_lm_own_value_stack_resize_zeroClosure != 0x7b47d9967133facdULL || lm_lmx_module_private_1_typedef_id_b_lm_own_value_stack_resize_zeroClosure != 0xe5da2db98fd8b138ULL
#error "Lingvamyxa conflicting typedef projection for lm_own_value_stack_resize_zeroClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_value_stack_pop
#define lm_lmx_module_private_1_typedef_defined_lm_own_value_stack_pop 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_value_stack_pop 0xb0075a8abfa4b19dULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_value_stack_pop 0x380f1329e93fe87aULL
typedef struct lm_own_value_stack_popClosure *lm_own_value_stack_pop;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_value_stack_pop) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_value_stack_pop) || lm_lmx_module_private_1_typedef_id_a_lm_own_value_stack_pop != 0xb0075a8abfa4b19dULL || lm_lmx_module_private_1_typedef_id_b_lm_own_value_stack_pop != 0x380f1329e93fe87aULL
#error "Lingvamyxa conflicting typedef projection for lm_own_value_stack_pop"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_value_stack_popClosure
#define lm_lmx_module_private_1_typedef_defined_lm_own_value_stack_popClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_value_stack_popClosure 0xa8a3d0053e487defULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_value_stack_popClosure 0xb0c2049b681fa4d6ULL
typedef struct lm_own_value_stack_popClosure lm_own_value_stack_popClosure;
struct lm_own_value_stack_popClosure {
    LmLmxStructure *descriptor;
    int (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, LmOwnValueStack *, void *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_own_value_stack_pop = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_value_stack_popClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_value_stack_popClosure) || lm_lmx_module_private_1_typedef_id_a_lm_own_value_stack_popClosure != 0xa8a3d0053e487defULL || lm_lmx_module_private_1_typedef_id_b_lm_own_value_stack_popClosure != 0xb0c2049b681fa4d6ULL
#error "Lingvamyxa conflicting typedef projection for lm_own_value_stack_popClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_value_stack_at
#define lm_lmx_module_private_1_typedef_defined_lm_own_value_stack_at 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_value_stack_at 0x65fde8f4e4e4200dULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_value_stack_at 0xa8612a4948b57bdeULL
typedef struct lm_own_value_stack_atClosure *lm_own_value_stack_at;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_value_stack_at) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_value_stack_at) || lm_lmx_module_private_1_typedef_id_a_lm_own_value_stack_at != 0x65fde8f4e4e4200dULL || lm_lmx_module_private_1_typedef_id_b_lm_own_value_stack_at != 0xa8612a4948b57bdeULL
#error "Lingvamyxa conflicting typedef projection for lm_own_value_stack_at"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_value_stack_atClosure
#define lm_lmx_module_private_1_typedef_defined_lm_own_value_stack_atClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_value_stack_atClosure 0x7f07e58e698e1e4dULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_value_stack_atClosure 0xc46dda20d4838e7aULL
typedef struct lm_own_value_stack_atClosure lm_own_value_stack_atClosure;
struct lm_own_value_stack_atClosure {
    LmLmxStructure *descriptor;
    void * (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, const LmOwnValueStack *, size_t);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_own_value_stack_at = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_value_stack_atClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_value_stack_atClosure) || lm_lmx_module_private_1_typedef_id_a_lm_own_value_stack_atClosure != 0x7f07e58e698e1e4dULL || lm_lmx_module_private_1_typedef_id_b_lm_own_value_stack_atClosure != 0xc46dda20d4838e7aULL
#error "Lingvamyxa conflicting typedef projection for lm_own_value_stack_atClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_value_stack_top
#define lm_lmx_module_private_1_typedef_defined_lm_own_value_stack_top 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_value_stack_top 0x74f79131dafd415dULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_value_stack_top 0x746e60352890fbf2ULL
typedef struct lm_own_value_stack_topClosure *lm_own_value_stack_top;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_value_stack_top) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_value_stack_top) || lm_lmx_module_private_1_typedef_id_a_lm_own_value_stack_top != 0x74f79131dafd415dULL || lm_lmx_module_private_1_typedef_id_b_lm_own_value_stack_top != 0x746e60352890fbf2ULL
#error "Lingvamyxa conflicting typedef projection for lm_own_value_stack_top"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_value_stack_topClosure
#define lm_lmx_module_private_1_typedef_defined_lm_own_value_stack_topClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_value_stack_topClosure 0x2700ad9dcb60ad2bULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_value_stack_topClosure 0xcd30de6d36d666b8ULL
typedef struct lm_own_value_stack_topClosure lm_own_value_stack_topClosure;
struct lm_own_value_stack_topClosure {
    LmLmxStructure *descriptor;
    void * (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, const LmOwnValueStack *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_own_value_stack_top = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_value_stack_topClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_value_stack_topClosure) || lm_lmx_module_private_1_typedef_id_a_lm_own_value_stack_topClosure != 0x2700ad9dcb60ad2bULL || lm_lmx_module_private_1_typedef_id_b_lm_own_value_stack_topClosure != 0xcd30de6d36d666b8ULL
#error "Lingvamyxa conflicting typedef projection for lm_own_value_stack_topClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_value_stack_truncate
#define lm_lmx_module_private_1_typedef_defined_lm_own_value_stack_truncate 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_value_stack_truncate 0x9bb4d10d9e7938a5ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_value_stack_truncate 0x95b0d25510a3a5feULL
typedef struct lm_own_value_stack_truncateClosure *lm_own_value_stack_truncate;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_value_stack_truncate) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_value_stack_truncate) || lm_lmx_module_private_1_typedef_id_a_lm_own_value_stack_truncate != 0x9bb4d10d9e7938a5ULL || lm_lmx_module_private_1_typedef_id_b_lm_own_value_stack_truncate != 0x95b0d25510a3a5feULL
#error "Lingvamyxa conflicting typedef projection for lm_own_value_stack_truncate"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_value_stack_truncateClosure
#define lm_lmx_module_private_1_typedef_defined_lm_own_value_stack_truncateClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_value_stack_truncateClosure 0x68e47d419df98520ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_value_stack_truncateClosure 0xa282007a59b33bd1ULL
typedef struct lm_own_value_stack_truncateClosure lm_own_value_stack_truncateClosure;
struct lm_own_value_stack_truncateClosure {
    LmLmxStructure *descriptor;
    void (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, LmOwnValueStack *, size_t);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_own_value_stack_truncate = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_value_stack_truncateClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_value_stack_truncateClosure) || lm_lmx_module_private_1_typedef_id_a_lm_own_value_stack_truncateClosure != 0x68e47d419df98520ULL || lm_lmx_module_private_1_typedef_id_b_lm_own_value_stack_truncateClosure != 0xa282007a59b33bd1ULL
#error "Lingvamyxa conflicting typedef projection for lm_own_value_stack_truncateClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_arena_new
#define lm_lmx_module_private_1_typedef_defined_lm_own_arena_new 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_arena_new 0xc0a7b3e36935e605ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_arena_new 0x27bf7f47b2021d8aULL
typedef struct lm_own_arena_newClosure *lm_own_arena_new;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_arena_new) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_arena_new) || lm_lmx_module_private_1_typedef_id_a_lm_own_arena_new != 0xc0a7b3e36935e605ULL || lm_lmx_module_private_1_typedef_id_b_lm_own_arena_new != 0x27bf7f47b2021d8aULL
#error "Lingvamyxa conflicting typedef projection for lm_own_arena_new"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_arena_newClosure
#define lm_lmx_module_private_1_typedef_defined_lm_own_arena_newClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_arena_newClosure 0xafac0f3dae774b40ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_arena_newClosure 0x8f833563f36bf971ULL
typedef struct lm_own_arena_newClosure lm_own_arena_newClosure;
struct lm_own_arena_newClosure {
    LmLmxStructure *descriptor;
    LmOwnArena * (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, LmMessageThread *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_own_arena_new = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_arena_newClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_arena_newClosure) || lm_lmx_module_private_1_typedef_id_a_lm_own_arena_newClosure != 0xafac0f3dae774b40ULL || lm_lmx_module_private_1_typedef_id_b_lm_own_arena_newClosure != 0x8f833563f36bf971ULL
#error "Lingvamyxa conflicting typedef projection for lm_own_arena_newClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_arena_delete
#define lm_lmx_module_private_1_typedef_defined_lm_own_arena_delete 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_arena_delete 0x1e02a65732c8e0d9ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_arena_delete 0x5fdd2c8a1b30dfa2ULL
typedef struct lm_own_arena_deleteClosure *lm_own_arena_delete;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_arena_delete) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_arena_delete) || lm_lmx_module_private_1_typedef_id_a_lm_own_arena_delete != 0x1e02a65732c8e0d9ULL || lm_lmx_module_private_1_typedef_id_b_lm_own_arena_delete != 0x5fdd2c8a1b30dfa2ULL
#error "Lingvamyxa conflicting typedef projection for lm_own_arena_delete"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_arena_deleteClosure
#define lm_lmx_module_private_1_typedef_defined_lm_own_arena_deleteClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_arena_deleteClosure 0x74379a9127553d0eULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_arena_deleteClosure 0x5b3f7507c04a19c7ULL
typedef struct lm_own_arena_deleteClosure lm_own_arena_deleteClosure;
struct lm_own_arena_deleteClosure {
    LmLmxStructure *descriptor;
    void (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, LmOwnArena *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_own_arena_delete = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_arena_deleteClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_arena_deleteClosure) || lm_lmx_module_private_1_typedef_id_a_lm_own_arena_deleteClosure != 0x74379a9127553d0eULL || lm_lmx_module_private_1_typedef_id_b_lm_own_arena_deleteClosure != 0x5b3f7507c04a19c7ULL
#error "Lingvamyxa conflicting typedef projection for lm_own_arena_deleteClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_arena_init
#define lm_lmx_module_private_1_typedef_defined_lm_own_arena_init 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_arena_init 0xa3b52b0dce94ed95ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_arena_init 0x0230cd5222d1665eULL
typedef struct lm_own_arena_initClosure *lm_own_arena_init;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_arena_init) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_arena_init) || lm_lmx_module_private_1_typedef_id_a_lm_own_arena_init != 0xa3b52b0dce94ed95ULL || lm_lmx_module_private_1_typedef_id_b_lm_own_arena_init != 0x0230cd5222d1665eULL
#error "Lingvamyxa conflicting typedef projection for lm_own_arena_init"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_arena_initClosure
#define lm_lmx_module_private_1_typedef_defined_lm_own_arena_initClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_arena_initClosure 0x801f199f88ee8599ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_arena_initClosure 0x2a5c8e1019ebce24ULL
typedef struct lm_own_arena_initClosure lm_own_arena_initClosure;
struct lm_own_arena_initClosure {
    LmLmxStructure *descriptor;
    int (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, LmOwnArena *, LmMessageThread *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_own_arena_init = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_arena_initClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_arena_initClosure) || lm_lmx_module_private_1_typedef_id_a_lm_own_arena_initClosure != 0x801f199f88ee8599ULL || lm_lmx_module_private_1_typedef_id_b_lm_own_arena_initClosure != 0x2a5c8e1019ebce24ULL
#error "Lingvamyxa conflicting typedef projection for lm_own_arena_initClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_arena_destroy
#define lm_lmx_module_private_1_typedef_defined_lm_own_arena_destroy 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_arena_destroy 0x3f33731600e6f751ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_arena_destroy 0xf85f5bdeaad21fd2ULL
typedef struct lm_own_arena_destroyClosure *lm_own_arena_destroy;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_arena_destroy) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_arena_destroy) || lm_lmx_module_private_1_typedef_id_a_lm_own_arena_destroy != 0x3f33731600e6f751ULL || lm_lmx_module_private_1_typedef_id_b_lm_own_arena_destroy != 0xf85f5bdeaad21fd2ULL
#error "Lingvamyxa conflicting typedef projection for lm_own_arena_destroy"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_arena_destroyClosure
#define lm_lmx_module_private_1_typedef_defined_lm_own_arena_destroyClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_arena_destroyClosure 0xb093150531e45786ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_arena_destroyClosure 0x1c91fc27f1420d2fULL
typedef struct lm_own_arena_destroyClosure lm_own_arena_destroyClosure;
struct lm_own_arena_destroyClosure {
    LmLmxStructure *descriptor;
    void (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, LmOwnArena *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_own_arena_destroy = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_arena_destroyClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_arena_destroyClosure) || lm_lmx_module_private_1_typedef_id_a_lm_own_arena_destroyClosure != 0xb093150531e45786ULL || lm_lmx_module_private_1_typedef_id_b_lm_own_arena_destroyClosure != 0x1c91fc27f1420d2fULL
#error "Lingvamyxa conflicting typedef projection for lm_own_arena_destroyClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_arena_new_zero
#define lm_lmx_module_private_1_typedef_defined_lm_own_arena_new_zero 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_arena_new_zero 0xa4acffda6d29a135ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_arena_new_zero 0x21a69dc55b2d6b3eULL
typedef struct lm_own_arena_new_zeroClosure *lm_own_arena_new_zero;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_arena_new_zero) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_arena_new_zero) || lm_lmx_module_private_1_typedef_id_a_lm_own_arena_new_zero != 0xa4acffda6d29a135ULL || lm_lmx_module_private_1_typedef_id_b_lm_own_arena_new_zero != 0x21a69dc55b2d6b3eULL
#error "Lingvamyxa conflicting typedef projection for lm_own_arena_new_zero"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_arena_new_zeroClosure
#define lm_lmx_module_private_1_typedef_defined_lm_own_arena_new_zeroClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_arena_new_zeroClosure 0x0f834fb84afa355aULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_arena_new_zeroClosure 0x046a5ca440c85c1bULL
typedef struct lm_own_arena_new_zeroClosure lm_own_arena_new_zeroClosure;
struct lm_own_arena_new_zeroClosure {
    LmLmxStructure *descriptor;
    void * (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, LmOwnArena *, size_t);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_own_arena_new_zero = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_arena_new_zeroClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_arena_new_zeroClosure) || lm_lmx_module_private_1_typedef_id_a_lm_own_arena_new_zeroClosure != 0x0f834fb84afa355aULL || lm_lmx_module_private_1_typedef_id_b_lm_own_arena_new_zeroClosure != 0x046a5ca440c85c1bULL
#error "Lingvamyxa conflicting typedef projection for lm_own_arena_new_zeroClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_arena_array_new_zero
#define lm_lmx_module_private_1_typedef_defined_lm_own_arena_array_new_zero 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_arena_array_new_zero 0x63b5651f62c00fa9ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_arena_array_new_zero 0x3b80140ff0bfd082ULL
typedef struct lm_own_arena_array_new_zeroClosure *lm_own_arena_array_new_zero;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_arena_array_new_zero) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_arena_array_new_zero) || lm_lmx_module_private_1_typedef_id_a_lm_own_arena_array_new_zero != 0x63b5651f62c00fa9ULL || lm_lmx_module_private_1_typedef_id_b_lm_own_arena_array_new_zero != 0x3b80140ff0bfd082ULL
#error "Lingvamyxa conflicting typedef projection for lm_own_arena_array_new_zero"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_arena_array_new_zeroClosure
#define lm_lmx_module_private_1_typedef_defined_lm_own_arena_array_new_zeroClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_arena_array_new_zeroClosure 0x87c46fa554382c44ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_arena_array_new_zeroClosure 0x8f9f39505ad05b11ULL
typedef struct lm_own_arena_array_new_zeroClosure lm_own_arena_array_new_zeroClosure;
struct lm_own_arena_array_new_zeroClosure {
    LmLmxStructure *descriptor;
    void * (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, LmOwnArena *, size_t, size_t, size_t, size_t);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_own_arena_array_new_zero = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_arena_array_new_zeroClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_arena_array_new_zeroClosure) || lm_lmx_module_private_1_typedef_id_a_lm_own_arena_array_new_zeroClosure != 0x87c46fa554382c44ULL || lm_lmx_module_private_1_typedef_id_b_lm_own_arena_array_new_zeroClosure != 0x8f9f39505ad05b11ULL
#error "Lingvamyxa conflicting typedef projection for lm_own_arena_array_new_zeroClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_arena_allocation_descriptor
#define lm_lmx_module_private_1_typedef_defined_lm_own_arena_allocation_descriptor 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_arena_allocation_descriptor 0x424330d38120f63dULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_arena_allocation_descriptor 0x4ca1b2125c50765aULL
typedef struct lm_own_arena_allocation_descriptorClosure *lm_own_arena_allocation_descriptor;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_arena_allocation_descriptor) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_arena_allocation_descriptor) || lm_lmx_module_private_1_typedef_id_a_lm_own_arena_allocation_descriptor != 0x424330d38120f63dULL || lm_lmx_module_private_1_typedef_id_b_lm_own_arena_allocation_descriptor != 0x4ca1b2125c50765aULL
#error "Lingvamyxa conflicting typedef projection for lm_own_arena_allocation_descriptor"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_arena_allocation_descriptorClosure
#define lm_lmx_module_private_1_typedef_defined_lm_own_arena_allocation_descriptorClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_arena_allocation_descriptorClosure 0xdac4173ab5328b11ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_arena_allocation_descriptorClosure 0x22c7af70951623f2ULL
typedef struct lm_own_arena_allocation_descriptorClosure lm_own_arena_allocation_descriptorClosure;
struct lm_own_arena_allocation_descriptorClosure {
    LmLmxStructure *descriptor;
    const LmOwnAllocationDescriptor * (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, const LmOwnArena *, const void *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_own_arena_allocation_descriptor = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_arena_allocation_descriptorClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_arena_allocation_descriptorClosure) || lm_lmx_module_private_1_typedef_id_a_lm_own_arena_allocation_descriptorClosure != 0xdac4173ab5328b11ULL || lm_lmx_module_private_1_typedef_id_b_lm_own_arena_allocation_descriptorClosure != 0x22c7af70951623f2ULL
#error "Lingvamyxa conflicting typedef projection for lm_own_arena_allocation_descriptorClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_arena_copy_bytes
#define lm_lmx_module_private_1_typedef_defined_lm_own_arena_copy_bytes 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_arena_copy_bytes 0xdeb8b87072cdc4fdULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_arena_copy_bytes 0x31ce9ca057e760b6ULL
typedef struct lm_own_arena_copy_bytesClosure *lm_own_arena_copy_bytes;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_arena_copy_bytes) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_arena_copy_bytes) || lm_lmx_module_private_1_typedef_id_a_lm_own_arena_copy_bytes != 0xdeb8b87072cdc4fdULL || lm_lmx_module_private_1_typedef_id_b_lm_own_arena_copy_bytes != 0x31ce9ca057e760b6ULL
#error "Lingvamyxa conflicting typedef projection for lm_own_arena_copy_bytes"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_arena_copy_bytesClosure
#define lm_lmx_module_private_1_typedef_defined_lm_own_arena_copy_bytesClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_arena_copy_bytesClosure 0x0491f90e7c317d37ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_arena_copy_bytesClosure 0x4c3b41ac74e6caaeULL
typedef struct lm_own_arena_copy_bytesClosure lm_own_arena_copy_bytesClosure;
struct lm_own_arena_copy_bytesClosure {
    LmLmxStructure *descriptor;
    char * (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, LmOwnArena *, const char *, size_t);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_own_arena_copy_bytes = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_arena_copy_bytesClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_arena_copy_bytesClosure) || lm_lmx_module_private_1_typedef_id_a_lm_own_arena_copy_bytesClosure != 0x0491f90e7c317d37ULL || lm_lmx_module_private_1_typedef_id_b_lm_own_arena_copy_bytesClosure != 0x4c3b41ac74e6caaeULL
#error "Lingvamyxa conflicting typedef projection for lm_own_arena_copy_bytesClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_arena_add_lazy_edge
#define lm_lmx_module_private_1_typedef_defined_lm_own_arena_add_lazy_edge 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_arena_add_lazy_edge 0x4d45b8e7ea7be3d5ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_arena_add_lazy_edge 0x960a7ba71a29ad6aULL
typedef struct lm_own_arena_add_lazy_edgeClosure *lm_own_arena_add_lazy_edge;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_arena_add_lazy_edge) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_arena_add_lazy_edge) || lm_lmx_module_private_1_typedef_id_a_lm_own_arena_add_lazy_edge != 0x4d45b8e7ea7be3d5ULL || lm_lmx_module_private_1_typedef_id_b_lm_own_arena_add_lazy_edge != 0x960a7ba71a29ad6aULL
#error "Lingvamyxa conflicting typedef projection for lm_own_arena_add_lazy_edge"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_arena_add_lazy_edgeClosure
#define lm_lmx_module_private_1_typedef_defined_lm_own_arena_add_lazy_edgeClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_arena_add_lazy_edgeClosure 0x1a6dc90316013e77ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_arena_add_lazy_edgeClosure 0xfe2a9f5e72ced6daULL
typedef struct lm_own_arena_add_lazy_edgeClosure lm_own_arena_add_lazy_edgeClosure;
struct lm_own_arena_add_lazy_edgeClosure {
    LmLmxStructure *descriptor;
    int (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, LmOwnArena *, LmOwnArena *, const void *, size_t, const void **);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_own_arena_add_lazy_edge = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_arena_add_lazy_edgeClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_arena_add_lazy_edgeClosure) || lm_lmx_module_private_1_typedef_id_a_lm_own_arena_add_lazy_edgeClosure != 0x1a6dc90316013e77ULL || lm_lmx_module_private_1_typedef_id_b_lm_own_arena_add_lazy_edgeClosure != 0xfe2a9f5e72ced6daULL
#error "Lingvamyxa conflicting typedef projection for lm_own_arena_add_lazy_edgeClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_arena_promote_lazy_edges
#define lm_lmx_module_private_1_typedef_defined_lm_own_arena_promote_lazy_edges 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_arena_promote_lazy_edges 0xa3bc9ccc3c4cfb49ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_arena_promote_lazy_edges 0x7a6e5974a8dca022ULL
typedef struct lm_own_arena_promote_lazy_edgesClosure *lm_own_arena_promote_lazy_edges;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_arena_promote_lazy_edges) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_arena_promote_lazy_edges) || lm_lmx_module_private_1_typedef_id_a_lm_own_arena_promote_lazy_edges != 0xa3bc9ccc3c4cfb49ULL || lm_lmx_module_private_1_typedef_id_b_lm_own_arena_promote_lazy_edges != 0x7a6e5974a8dca022ULL
#error "Lingvamyxa conflicting typedef projection for lm_own_arena_promote_lazy_edges"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_arena_promote_lazy_edgesClosure
#define lm_lmx_module_private_1_typedef_defined_lm_own_arena_promote_lazy_edgesClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_arena_promote_lazy_edgesClosure 0xb585abe98abb667bULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_arena_promote_lazy_edgesClosure 0x608f1e42953f5c84ULL
typedef struct lm_own_arena_promote_lazy_edgesClosure lm_own_arena_promote_lazy_edgesClosure;
struct lm_own_arena_promote_lazy_edgesClosure {
    LmLmxStructure *descriptor;
    int (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, LmOwnArena *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_own_arena_promote_lazy_edges = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_arena_promote_lazy_edgesClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_arena_promote_lazy_edgesClosure) || lm_lmx_module_private_1_typedef_id_a_lm_own_arena_promote_lazy_edgesClosure != 0xb585abe98abb667bULL || lm_lmx_module_private_1_typedef_id_b_lm_own_arena_promote_lazy_edgesClosure != 0x608f1e42953f5c84ULL
#error "Lingvamyxa conflicting typedef projection for lm_own_arena_promote_lazy_edgesClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_arena_absorb
#define lm_lmx_module_private_1_typedef_defined_lm_own_arena_absorb 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_arena_absorb 0xde9e92135ab904cdULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_arena_absorb 0x4592a5ed5b5caa36ULL
typedef struct lm_own_arena_absorbClosure *lm_own_arena_absorb;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_arena_absorb) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_arena_absorb) || lm_lmx_module_private_1_typedef_id_a_lm_own_arena_absorb != 0xde9e92135ab904cdULL || lm_lmx_module_private_1_typedef_id_b_lm_own_arena_absorb != 0x4592a5ed5b5caa36ULL
#error "Lingvamyxa conflicting typedef projection for lm_own_arena_absorb"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_arena_absorbClosure
#define lm_lmx_module_private_1_typedef_defined_lm_own_arena_absorbClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_arena_absorbClosure 0x3d8a3d455a216081ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_arena_absorbClosure 0x542357244aac9cdeULL
typedef struct lm_own_arena_absorbClosure lm_own_arena_absorbClosure;
struct lm_own_arena_absorbClosure {
    LmLmxStructure *descriptor;
    int (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, LmOwnArena *, LmOwnArena *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_own_arena_absorb = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_arena_absorbClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_arena_absorbClosure) || lm_lmx_module_private_1_typedef_id_a_lm_own_arena_absorbClosure != 0x3d8a3d455a216081ULL || lm_lmx_module_private_1_typedef_id_b_lm_own_arena_absorbClosure != 0x542357244aac9cdeULL
#error "Lingvamyxa conflicting typedef projection for lm_own_arena_absorbClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_arena_freeze
#define lm_lmx_module_private_1_typedef_defined_lm_own_arena_freeze 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_arena_freeze 0x04bf7441d7dc2b3dULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_arena_freeze 0x16d68b2ea8160976ULL
typedef struct lm_own_arena_freezeClosure *lm_own_arena_freeze;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_arena_freeze) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_arena_freeze) || lm_lmx_module_private_1_typedef_id_a_lm_own_arena_freeze != 0x04bf7441d7dc2b3dULL || lm_lmx_module_private_1_typedef_id_b_lm_own_arena_freeze != 0x16d68b2ea8160976ULL
#error "Lingvamyxa conflicting typedef projection for lm_own_arena_freeze"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_arena_freezeClosure
#define lm_lmx_module_private_1_typedef_defined_lm_own_arena_freezeClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_arena_freezeClosure 0x62b4d4b40aefea4eULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_arena_freezeClosure 0xbd0fb2cf46881f87ULL
typedef struct lm_own_arena_freezeClosure lm_own_arena_freezeClosure;
struct lm_own_arena_freezeClosure {
    LmLmxStructure *descriptor;
    void (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, LmOwnArena *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_own_arena_freeze = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_arena_freezeClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_arena_freezeClosure) || lm_lmx_module_private_1_typedef_id_a_lm_own_arena_freezeClosure != 0x62b4d4b40aefea4eULL || lm_lmx_module_private_1_typedef_id_b_lm_own_arena_freezeClosure != 0xbd0fb2cf46881f87ULL
#error "Lingvamyxa conflicting typedef projection for lm_own_arena_freezeClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_arena_is_frozen
#define lm_lmx_module_private_1_typedef_defined_lm_own_arena_is_frozen 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_arena_is_frozen 0xc82f3ec79b9a5c13ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_arena_is_frozen 0x14d7f3484f99bfbcULL
typedef struct lm_own_arena_is_frozenClosure *lm_own_arena_is_frozen;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_arena_is_frozen) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_arena_is_frozen) || lm_lmx_module_private_1_typedef_id_a_lm_own_arena_is_frozen != 0xc82f3ec79b9a5c13ULL || lm_lmx_module_private_1_typedef_id_b_lm_own_arena_is_frozen != 0x14d7f3484f99bfbcULL
#error "Lingvamyxa conflicting typedef projection for lm_own_arena_is_frozen"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_arena_is_frozenClosure
#define lm_lmx_module_private_1_typedef_defined_lm_own_arena_is_frozenClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_arena_is_frozenClosure 0x7df9778b278eb818ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_arena_is_frozenClosure 0xf05a3b5150d20089ULL
typedef struct lm_own_arena_is_frozenClosure lm_own_arena_is_frozenClosure;
struct lm_own_arena_is_frozenClosure {
    LmLmxStructure *descriptor;
    int (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, const LmOwnArena *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_own_arena_is_frozen = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_arena_is_frozenClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_arena_is_frozenClosure) || lm_lmx_module_private_1_typedef_id_a_lm_own_arena_is_frozenClosure != 0x7df9778b278eb818ULL || lm_lmx_module_private_1_typedef_id_b_lm_own_arena_is_frozenClosure != 0xf05a3b5150d20089ULL
#error "Lingvamyxa conflicting typedef projection for lm_own_arena_is_frozenClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_arena_owner_thread
#define lm_lmx_module_private_1_typedef_defined_lm_own_arena_owner_thread 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_arena_owner_thread 0xb14aa8dd4e64b3d1ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_arena_owner_thread 0x50ca2248d307fb9aULL
typedef struct lm_own_arena_owner_threadClosure *lm_own_arena_owner_thread;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_arena_owner_thread) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_arena_owner_thread) || lm_lmx_module_private_1_typedef_id_a_lm_own_arena_owner_thread != 0xb14aa8dd4e64b3d1ULL || lm_lmx_module_private_1_typedef_id_b_lm_own_arena_owner_thread != 0x50ca2248d307fb9aULL
#error "Lingvamyxa conflicting typedef projection for lm_own_arena_owner_thread"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_arena_owner_threadClosure
#define lm_lmx_module_private_1_typedef_defined_lm_own_arena_owner_threadClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_arena_owner_threadClosure 0x922095e79efb17d1ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_arena_owner_threadClosure 0x146e196b6f013896ULL
typedef struct lm_own_arena_owner_threadClosure lm_own_arena_owner_threadClosure;
struct lm_own_arena_owner_threadClosure {
    LmLmxStructure *descriptor;
    LmMessageThread * (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, const LmOwnArena *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_own_arena_owner_thread = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_arena_owner_threadClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_arena_owner_threadClosure) || lm_lmx_module_private_1_typedef_id_a_lm_own_arena_owner_threadClosure != 0x922095e79efb17d1ULL || lm_lmx_module_private_1_typedef_id_b_lm_own_arena_owner_threadClosure != 0x146e196b6f013896ULL
#error "Lingvamyxa conflicting typedef projection for lm_own_arena_owner_threadClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_tree_cut
#define lm_lmx_module_private_1_typedef_defined_lm_own_tree_cut 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_tree_cut 0xc5aaa75e461e9375ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_tree_cut 0xec2b2b8109126fd6ULL
typedef struct lm_own_tree_cutClosure *lm_own_tree_cut;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_tree_cut) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_tree_cut) || lm_lmx_module_private_1_typedef_id_a_lm_own_tree_cut != 0xc5aaa75e461e9375ULL || lm_lmx_module_private_1_typedef_id_b_lm_own_tree_cut != 0xec2b2b8109126fd6ULL
#error "Lingvamyxa conflicting typedef projection for lm_own_tree_cut"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_tree_cutClosure
#define lm_lmx_module_private_1_typedef_defined_lm_own_tree_cutClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_tree_cutClosure 0x8f1e778d977f97ebULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_tree_cutClosure 0x97274b7982fd2f14ULL
typedef struct lm_own_tree_cutClosure lm_own_tree_cutClosure;
struct lm_own_tree_cutClosure {
    LmLmxStructure *descriptor;
    int (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, LmOwnArena *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_own_tree_cut = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_tree_cutClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_tree_cutClosure) || lm_lmx_module_private_1_typedef_id_a_lm_own_tree_cutClosure != 0x8f1e778d977f97ebULL || lm_lmx_module_private_1_typedef_id_b_lm_own_tree_cutClosure != 0x97274b7982fd2f14ULL
#error "Lingvamyxa conflicting typedef projection for lm_own_tree_cutClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_tree_cut_promote_lazy_edges
#define lm_lmx_module_private_1_typedef_defined_lm_own_tree_cut_promote_lazy_edges 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_tree_cut_promote_lazy_edges 0xaeaa091442bcd3a1ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_tree_cut_promote_lazy_edges 0x9e090fc17f9c8382ULL
typedef struct lm_own_tree_cut_promote_lazy_edgesClosure *lm_own_tree_cut_promote_lazy_edges;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_tree_cut_promote_lazy_edges) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_tree_cut_promote_lazy_edges) || lm_lmx_module_private_1_typedef_id_a_lm_own_tree_cut_promote_lazy_edges != 0xaeaa091442bcd3a1ULL || lm_lmx_module_private_1_typedef_id_b_lm_own_tree_cut_promote_lazy_edges != 0x9e090fc17f9c8382ULL
#error "Lingvamyxa conflicting typedef projection for lm_own_tree_cut_promote_lazy_edges"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_tree_cut_promote_lazy_edgesClosure
#define lm_lmx_module_private_1_typedef_defined_lm_own_tree_cut_promote_lazy_edgesClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_tree_cut_promote_lazy_edgesClosure 0xb108b07db9a6e27bULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_tree_cut_promote_lazy_edgesClosure 0x1635ded70c052838ULL
typedef struct lm_own_tree_cut_promote_lazy_edgesClosure lm_own_tree_cut_promote_lazy_edgesClosure;
struct lm_own_tree_cut_promote_lazy_edgesClosure {
    LmLmxStructure *descriptor;
    int (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, LmOwnArena *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_own_tree_cut_promote_lazy_edges = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_tree_cut_promote_lazy_edgesClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_tree_cut_promote_lazy_edgesClosure) || lm_lmx_module_private_1_typedef_id_a_lm_own_tree_cut_promote_lazy_edgesClosure != 0xb108b07db9a6e27bULL || lm_lmx_module_private_1_typedef_id_b_lm_own_tree_cut_promote_lazy_edgesClosure != 0x1635ded70c052838ULL
#error "Lingvamyxa conflicting typedef projection for lm_own_tree_cut_promote_lazy_edgesClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_arena_pin
#define lm_lmx_module_private_1_typedef_defined_lm_own_arena_pin 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_arena_pin 0x4b67d864da2ef837ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_arena_pin 0xf86aa21e66fac430ULL
typedef struct lm_own_arena_pinClosure *lm_own_arena_pin;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_arena_pin) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_arena_pin) || lm_lmx_module_private_1_typedef_id_a_lm_own_arena_pin != 0x4b67d864da2ef837ULL || lm_lmx_module_private_1_typedef_id_b_lm_own_arena_pin != 0xf86aa21e66fac430ULL
#error "Lingvamyxa conflicting typedef projection for lm_own_arena_pin"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_arena_pinClosure
#define lm_lmx_module_private_1_typedef_defined_lm_own_arena_pinClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_arena_pinClosure 0x9bc2e291977b1c27ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_arena_pinClosure 0x894ae6650a273044ULL
typedef struct lm_own_arena_pinClosure lm_own_arena_pinClosure;
struct lm_own_arena_pinClosure {
    LmLmxStructure *descriptor;
    int (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, LmOwnArena *, void *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_own_arena_pin = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_arena_pinClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_arena_pinClosure) || lm_lmx_module_private_1_typedef_id_a_lm_own_arena_pinClosure != 0x9bc2e291977b1c27ULL || lm_lmx_module_private_1_typedef_id_b_lm_own_arena_pinClosure != 0x894ae6650a273044ULL
#error "Lingvamyxa conflicting typedef projection for lm_own_arena_pinClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_arena_root_add
#define lm_lmx_module_private_1_typedef_defined_lm_own_arena_root_add 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_arena_root_add 0x3a553d261214e179ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_arena_root_add 0xb3ea53fca2fa1d8aULL
typedef struct lm_own_arena_root_addClosure *lm_own_arena_root_add;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_arena_root_add) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_arena_root_add) || lm_lmx_module_private_1_typedef_id_a_lm_own_arena_root_add != 0x3a553d261214e179ULL || lm_lmx_module_private_1_typedef_id_b_lm_own_arena_root_add != 0xb3ea53fca2fa1d8aULL
#error "Lingvamyxa conflicting typedef projection for lm_own_arena_root_add"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_arena_root_addClosure
#define lm_lmx_module_private_1_typedef_defined_lm_own_arena_root_addClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_arena_root_addClosure 0xcf965b326b723dbfULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_arena_root_addClosure 0x1350d253d8d9cbc4ULL
typedef struct lm_own_arena_root_addClosure lm_own_arena_root_addClosure;
struct lm_own_arena_root_addClosure {
    LmLmxStructure *descriptor;
    int (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, LmOwnArena *, void *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_own_arena_root_add = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_arena_root_addClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_arena_root_addClosure) || lm_lmx_module_private_1_typedef_id_a_lm_own_arena_root_addClosure != 0xcf965b326b723dbfULL || lm_lmx_module_private_1_typedef_id_b_lm_own_arena_root_addClosure != 0x1350d253d8d9cbc4ULL
#error "Lingvamyxa conflicting typedef projection for lm_own_arena_root_addClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_arena_copy_graph
#define lm_lmx_module_private_1_typedef_defined_lm_own_arena_copy_graph 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_arena_copy_graph 0x8aaaa44b25a90275ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_arena_copy_graph 0x43b1285667588c5eULL
typedef struct lm_own_arena_copy_graphClosure *lm_own_arena_copy_graph;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_arena_copy_graph) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_arena_copy_graph) || lm_lmx_module_private_1_typedef_id_a_lm_own_arena_copy_graph != 0x8aaaa44b25a90275ULL || lm_lmx_module_private_1_typedef_id_b_lm_own_arena_copy_graph != 0x43b1285667588c5eULL
#error "Lingvamyxa conflicting typedef projection for lm_own_arena_copy_graph"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_arena_copy_graphClosure
#define lm_lmx_module_private_1_typedef_defined_lm_own_arena_copy_graphClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_arena_copy_graphClosure 0x73fd9f969d5eedafULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_arena_copy_graphClosure 0xcc9a00da90efe2daULL
typedef struct lm_own_arena_copy_graphClosure lm_own_arena_copy_graphClosure;
struct lm_own_arena_copy_graphClosure {
    LmLmxStructure *descriptor;
    void * (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, LmOwnArena *, const void *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_own_arena_copy_graph = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_arena_copy_graphClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_arena_copy_graphClosure) || lm_lmx_module_private_1_typedef_id_a_lm_own_arena_copy_graphClosure != 0x73fd9f969d5eedafULL || lm_lmx_module_private_1_typedef_id_b_lm_own_arena_copy_graphClosure != 0xcc9a00da90efe2daULL
#error "Lingvamyxa conflicting typedef projection for lm_own_arena_copy_graphClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_arena_reclaim
#define lm_lmx_module_private_1_typedef_defined_lm_own_arena_reclaim 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_arena_reclaim 0x03b12430a5f9c7c7ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_arena_reclaim 0x222444bd94d71614ULL
typedef struct lm_own_arena_reclaimClosure *lm_own_arena_reclaim;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_arena_reclaim) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_arena_reclaim) || lm_lmx_module_private_1_typedef_id_a_lm_own_arena_reclaim != 0x03b12430a5f9c7c7ULL || lm_lmx_module_private_1_typedef_id_b_lm_own_arena_reclaim != 0x222444bd94d71614ULL
#error "Lingvamyxa conflicting typedef projection for lm_own_arena_reclaim"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_own_arena_reclaimClosure
#define lm_lmx_module_private_1_typedef_defined_lm_own_arena_reclaimClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_own_arena_reclaimClosure 0x8a4780abf3f52563ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_own_arena_reclaimClosure 0x88553b57b1d03828ULL
typedef struct lm_own_arena_reclaimClosure lm_own_arena_reclaimClosure;
struct lm_own_arena_reclaimClosure {
    LmLmxStructure *descriptor;
    int (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, LmOwnArena *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_own_arena_reclaim = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_own_arena_reclaimClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_own_arena_reclaimClosure) || lm_lmx_module_private_1_typedef_id_a_lm_own_arena_reclaimClosure != 0x8a4780abf3f52563ULL || lm_lmx_module_private_1_typedef_id_b_lm_own_arena_reclaimClosure != 0x88553b57b1d03828ULL
#error "Lingvamyxa conflicting typedef projection for lm_own_arena_reclaimClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_thread_provider_name
#define lm_lmx_module_private_1_typedef_defined_lm_thread_provider_name 1
#define lm_lmx_module_private_1_typedef_id_a_lm_thread_provider_name 0xeccc8bfc004ed33dULL
#define lm_lmx_module_private_1_typedef_id_b_lm_thread_provider_name 0xb2335fe86e6afb2eULL
typedef struct lm_thread_provider_nameClosure *lm_thread_provider_name;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_thread_provider_name) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_thread_provider_name) || lm_lmx_module_private_1_typedef_id_a_lm_thread_provider_name != 0xeccc8bfc004ed33dULL || lm_lmx_module_private_1_typedef_id_b_lm_thread_provider_name != 0xb2335fe86e6afb2eULL
#error "Lingvamyxa conflicting typedef projection for lm_thread_provider_name"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_thread_provider_nameClosure
#define lm_lmx_module_private_1_typedef_defined_lm_thread_provider_nameClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_thread_provider_nameClosure 0x26920400c5aee723ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_thread_provider_nameClosure 0x4cd5889c00944aaaULL
typedef struct lm_thread_provider_nameClosure lm_thread_provider_nameClosure;
struct lm_thread_provider_nameClosure {
    LmLmxStructure *descriptor;
    const char * (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_thread_provider_name = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_thread_provider_nameClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_thread_provider_nameClosure) || lm_lmx_module_private_1_typedef_id_a_lm_thread_provider_nameClosure != 0x26920400c5aee723ULL || lm_lmx_module_private_1_typedef_id_b_lm_thread_provider_nameClosure != 0x4cd5889c00944aaaULL
#error "Lingvamyxa conflicting typedef projection for lm_thread_provider_nameClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_host_thread_new
#define lm_lmx_module_private_1_typedef_defined_lm_host_thread_new 1
#define lm_lmx_module_private_1_typedef_id_a_lm_host_thread_new 0x4e39b9fbeb67ce6fULL
#define lm_lmx_module_private_1_typedef_id_b_lm_host_thread_new 0x159d4adec13e269cULL
typedef struct lm_host_thread_newClosure *lm_host_thread_new;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_host_thread_new) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_host_thread_new) || lm_lmx_module_private_1_typedef_id_a_lm_host_thread_new != 0x4e39b9fbeb67ce6fULL || lm_lmx_module_private_1_typedef_id_b_lm_host_thread_new != 0x159d4adec13e269cULL
#error "Lingvamyxa conflicting typedef projection for lm_host_thread_new"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_host_thread_newClosure
#define lm_lmx_module_private_1_typedef_defined_lm_host_thread_newClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_host_thread_newClosure 0x229d3b6386a77713ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_host_thread_newClosure 0x5dac1c938338e5deULL
typedef struct lm_host_thread_newClosure lm_host_thread_newClosure;
struct lm_host_thread_newClosure {
    LmLmxStructure *descriptor;
    LmHostThread * (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_host_thread_new = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_host_thread_newClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_host_thread_newClosure) || lm_lmx_module_private_1_typedef_id_a_lm_host_thread_newClosure != 0x229d3b6386a77713ULL || lm_lmx_module_private_1_typedef_id_b_lm_host_thread_newClosure != 0x5dac1c938338e5deULL
#error "Lingvamyxa conflicting typedef projection for lm_host_thread_newClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_host_thread_delete
#define lm_lmx_module_private_1_typedef_defined_lm_host_thread_delete 1
#define lm_lmx_module_private_1_typedef_id_a_lm_host_thread_delete 0xa70cb8e6aa94cdd1ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_host_thread_delete 0xe8ab6e998ce153faULL
typedef struct lm_host_thread_deleteClosure *lm_host_thread_delete;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_host_thread_delete) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_host_thread_delete) || lm_lmx_module_private_1_typedef_id_a_lm_host_thread_delete != 0xa70cb8e6aa94cdd1ULL || lm_lmx_module_private_1_typedef_id_b_lm_host_thread_delete != 0xe8ab6e998ce153faULL
#error "Lingvamyxa conflicting typedef projection for lm_host_thread_delete"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_host_thread_deleteClosure
#define lm_lmx_module_private_1_typedef_defined_lm_host_thread_deleteClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_host_thread_deleteClosure 0x65d8f47a30793a15ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_host_thread_deleteClosure 0x0a98128b5e052048ULL
typedef struct lm_host_thread_deleteClosure lm_host_thread_deleteClosure;
struct lm_host_thread_deleteClosure {
    LmLmxStructure *descriptor;
    void (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, LmHostThread *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_host_thread_delete = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_host_thread_deleteClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_host_thread_deleteClosure) || lm_lmx_module_private_1_typedef_id_a_lm_host_thread_deleteClosure != 0x65d8f47a30793a15ULL || lm_lmx_module_private_1_typedef_id_b_lm_host_thread_deleteClosure != 0x0a98128b5e052048ULL
#error "Lingvamyxa conflicting typedef projection for lm_host_thread_deleteClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_host_thread_start
#define lm_lmx_module_private_1_typedef_defined_lm_host_thread_start 1
#define lm_lmx_module_private_1_typedef_id_a_lm_host_thread_start 0xc8c6e55ade4405abULL
#define lm_lmx_module_private_1_typedef_id_b_lm_host_thread_start 0x65690d399f665d70ULL
typedef struct lm_host_thread_startClosure *lm_host_thread_start;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_host_thread_start) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_host_thread_start) || lm_lmx_module_private_1_typedef_id_a_lm_host_thread_start != 0xc8c6e55ade4405abULL || lm_lmx_module_private_1_typedef_id_b_lm_host_thread_start != 0x65690d399f665d70ULL
#error "Lingvamyxa conflicting typedef projection for lm_host_thread_start"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_host_thread_startClosure
#define lm_lmx_module_private_1_typedef_defined_lm_host_thread_startClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_host_thread_startClosure 0x85c3ddb32081eedfULL
#define lm_lmx_module_private_1_typedef_id_b_lm_host_thread_startClosure 0x01d144558b15d37aULL
typedef struct lm_host_thread_startClosure lm_host_thread_startClosure;
struct lm_host_thread_startClosure {
    LmLmxStructure *descriptor;
    int (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, LmHostThread *, LmHostThreadEntry, void *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_host_thread_start = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_host_thread_startClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_host_thread_startClosure) || lm_lmx_module_private_1_typedef_id_a_lm_host_thread_startClosure != 0x85c3ddb32081eedfULL || lm_lmx_module_private_1_typedef_id_b_lm_host_thread_startClosure != 0x01d144558b15d37aULL
#error "Lingvamyxa conflicting typedef projection for lm_host_thread_startClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_host_thread_join
#define lm_lmx_module_private_1_typedef_defined_lm_host_thread_join 1
#define lm_lmx_module_private_1_typedef_id_a_lm_host_thread_join 0x2c886aab2295c0c9ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_host_thread_join 0x64e4aadf064064eaULL
typedef struct lm_host_thread_joinClosure *lm_host_thread_join;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_host_thread_join) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_host_thread_join) || lm_lmx_module_private_1_typedef_id_a_lm_host_thread_join != 0x2c886aab2295c0c9ULL || lm_lmx_module_private_1_typedef_id_b_lm_host_thread_join != 0x64e4aadf064064eaULL
#error "Lingvamyxa conflicting typedef projection for lm_host_thread_join"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_host_thread_joinClosure
#define lm_lmx_module_private_1_typedef_defined_lm_host_thread_joinClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_host_thread_joinClosure 0xfa504dc3457b2942ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_host_thread_joinClosure 0x576cede004209683ULL
typedef struct lm_host_thread_joinClosure lm_host_thread_joinClosure;
struct lm_host_thread_joinClosure {
    LmLmxStructure *descriptor;
    int (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, LmHostThread *, void **);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_host_thread_join = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_host_thread_joinClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_host_thread_joinClosure) || lm_lmx_module_private_1_typedef_id_a_lm_host_thread_joinClosure != 0xfa504dc3457b2942ULL || lm_lmx_module_private_1_typedef_id_b_lm_host_thread_joinClosure != 0x576cede004209683ULL
#error "Lingvamyxa conflicting typedef projection for lm_host_thread_joinClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_mutex_new
#define lm_lmx_module_private_1_typedef_defined_lm_mutex_new 1
#define lm_lmx_module_private_1_typedef_id_a_lm_mutex_new 0x89f5a60eecf4437fULL
#define lm_lmx_module_private_1_typedef_id_b_lm_mutex_new 0x5c5b1edb52cc5b1cULL
typedef struct lm_mutex_newClosure *lm_mutex_new;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_mutex_new) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_mutex_new) || lm_lmx_module_private_1_typedef_id_a_lm_mutex_new != 0x89f5a60eecf4437fULL || lm_lmx_module_private_1_typedef_id_b_lm_mutex_new != 0x5c5b1edb52cc5b1cULL
#error "Lingvamyxa conflicting typedef projection for lm_mutex_new"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_mutex_newClosure
#define lm_lmx_module_private_1_typedef_defined_lm_mutex_newClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_mutex_newClosure 0xbe769658427c40d8ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_mutex_newClosure 0x5268f8e8107b0071ULL
typedef struct lm_mutex_newClosure lm_mutex_newClosure;
struct lm_mutex_newClosure {
    LmLmxStructure *descriptor;
    LmMutex * (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_mutex_new = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_mutex_newClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_mutex_newClosure) || lm_lmx_module_private_1_typedef_id_a_lm_mutex_newClosure != 0xbe769658427c40d8ULL || lm_lmx_module_private_1_typedef_id_b_lm_mutex_newClosure != 0x5268f8e8107b0071ULL
#error "Lingvamyxa conflicting typedef projection for lm_mutex_newClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_mutex_delete
#define lm_lmx_module_private_1_typedef_defined_lm_mutex_delete 1
#define lm_lmx_module_private_1_typedef_id_a_lm_mutex_delete 0x1e54890da255e019ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_mutex_delete 0x511f6da2f8bfce0aULL
typedef struct lm_mutex_deleteClosure *lm_mutex_delete;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_mutex_delete) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_mutex_delete) || lm_lmx_module_private_1_typedef_id_a_lm_mutex_delete != 0x1e54890da255e019ULL || lm_lmx_module_private_1_typedef_id_b_lm_mutex_delete != 0x511f6da2f8bfce0aULL
#error "Lingvamyxa conflicting typedef projection for lm_mutex_delete"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_mutex_deleteClosure
#define lm_lmx_module_private_1_typedef_defined_lm_mutex_deleteClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_mutex_deleteClosure 0x6a332367598ea8caULL
#define lm_lmx_module_private_1_typedef_id_b_lm_mutex_deleteClosure 0xc4b0aaada8cec9c3ULL
typedef struct lm_mutex_deleteClosure lm_mutex_deleteClosure;
struct lm_mutex_deleteClosure {
    LmLmxStructure *descriptor;
    void (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, LmMutex *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_mutex_delete = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_mutex_deleteClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_mutex_deleteClosure) || lm_lmx_module_private_1_typedef_id_a_lm_mutex_deleteClosure != 0x6a332367598ea8caULL || lm_lmx_module_private_1_typedef_id_b_lm_mutex_deleteClosure != 0xc4b0aaada8cec9c3ULL
#error "Lingvamyxa conflicting typedef projection for lm_mutex_deleteClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_mutex_lock
#define lm_lmx_module_private_1_typedef_defined_lm_mutex_lock 1
#define lm_lmx_module_private_1_typedef_id_a_lm_mutex_lock 0xa8b987f683a67e0dULL
#define lm_lmx_module_private_1_typedef_id_b_lm_mutex_lock 0xa4f1b064e9829396ULL
typedef struct lm_mutex_lockClosure *lm_mutex_lock;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_mutex_lock) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_mutex_lock) || lm_lmx_module_private_1_typedef_id_a_lm_mutex_lock != 0xa8b987f683a67e0dULL || lm_lmx_module_private_1_typedef_id_b_lm_mutex_lock != 0xa4f1b064e9829396ULL
#error "Lingvamyxa conflicting typedef projection for lm_mutex_lock"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_mutex_lockClosure
#define lm_lmx_module_private_1_typedef_defined_lm_mutex_lockClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_mutex_lockClosure 0x5faec70c2af266e3ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_mutex_lockClosure 0xeb7daa78d9d75836ULL
typedef struct lm_mutex_lockClosure lm_mutex_lockClosure;
struct lm_mutex_lockClosure {
    LmLmxStructure *descriptor;
    int (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, LmMutex *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_mutex_lock = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_mutex_lockClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_mutex_lockClosure) || lm_lmx_module_private_1_typedef_id_a_lm_mutex_lockClosure != 0x5faec70c2af266e3ULL || lm_lmx_module_private_1_typedef_id_b_lm_mutex_lockClosure != 0xeb7daa78d9d75836ULL
#error "Lingvamyxa conflicting typedef projection for lm_mutex_lockClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_mutex_unlock
#define lm_lmx_module_private_1_typedef_defined_lm_mutex_unlock 1
#define lm_lmx_module_private_1_typedef_id_a_lm_mutex_unlock 0x022e476de862ee91ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_mutex_unlock 0x08ff3056769a53faULL
typedef struct lm_mutex_unlockClosure *lm_mutex_unlock;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_mutex_unlock) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_mutex_unlock) || lm_lmx_module_private_1_typedef_id_a_lm_mutex_unlock != 0x022e476de862ee91ULL || lm_lmx_module_private_1_typedef_id_b_lm_mutex_unlock != 0x08ff3056769a53faULL
#error "Lingvamyxa conflicting typedef projection for lm_mutex_unlock"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_mutex_unlockClosure
#define lm_lmx_module_private_1_typedef_defined_lm_mutex_unlockClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_mutex_unlockClosure 0x3995c2e360503319ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_mutex_unlockClosure 0x47d3a62b8cfcef24ULL
typedef struct lm_mutex_unlockClosure lm_mutex_unlockClosure;
struct lm_mutex_unlockClosure {
    LmLmxStructure *descriptor;
    int (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, LmMutex *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_mutex_unlock = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_mutex_unlockClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_mutex_unlockClosure) || lm_lmx_module_private_1_typedef_id_a_lm_mutex_unlockClosure != 0x3995c2e360503319ULL || lm_lmx_module_private_1_typedef_id_b_lm_mutex_unlockClosure != 0x47d3a62b8cfcef24ULL
#error "Lingvamyxa conflicting typedef projection for lm_mutex_unlockClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_condition_new
#define lm_lmx_module_private_1_typedef_defined_lm_condition_new 1
#define lm_lmx_module_private_1_typedef_id_a_lm_condition_new 0xd547ac3cad850177ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_condition_new 0x2306230ee9767f80ULL
typedef struct lm_condition_newClosure *lm_condition_new;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_condition_new) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_condition_new) || lm_lmx_module_private_1_typedef_id_a_lm_condition_new != 0xd547ac3cad850177ULL || lm_lmx_module_private_1_typedef_id_b_lm_condition_new != 0x2306230ee9767f80ULL
#error "Lingvamyxa conflicting typedef projection for lm_condition_new"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_condition_newClosure
#define lm_lmx_module_private_1_typedef_defined_lm_condition_newClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_condition_newClosure 0xc845dc2e57e68cceULL
#define lm_lmx_module_private_1_typedef_id_b_lm_condition_newClosure 0x06003e52df587bd7ULL
typedef struct lm_condition_newClosure lm_condition_newClosure;
struct lm_condition_newClosure {
    LmLmxStructure *descriptor;
    LmCondition * (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_condition_new = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_condition_newClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_condition_newClosure) || lm_lmx_module_private_1_typedef_id_a_lm_condition_newClosure != 0xc845dc2e57e68cceULL || lm_lmx_module_private_1_typedef_id_b_lm_condition_newClosure != 0x06003e52df587bd7ULL
#error "Lingvamyxa conflicting typedef projection for lm_condition_newClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_condition_delete
#define lm_lmx_module_private_1_typedef_defined_lm_condition_delete 1
#define lm_lmx_module_private_1_typedef_id_a_lm_condition_delete 0xbe5f0d8413cde9d1ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_condition_delete 0xfa42eddcd3dc57eaULL
typedef struct lm_condition_deleteClosure *lm_condition_delete;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_condition_delete) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_condition_delete) || lm_lmx_module_private_1_typedef_id_a_lm_condition_delete != 0xbe5f0d8413cde9d1ULL || lm_lmx_module_private_1_typedef_id_b_lm_condition_delete != 0xfa42eddcd3dc57eaULL
#error "Lingvamyxa conflicting typedef projection for lm_condition_delete"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_condition_deleteClosure
#define lm_lmx_module_private_1_typedef_defined_lm_condition_deleteClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_condition_deleteClosure 0x72f48ad6e76fdda0ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_condition_deleteClosure 0x1c1a5db0d0cf6db5ULL
typedef struct lm_condition_deleteClosure lm_condition_deleteClosure;
struct lm_condition_deleteClosure {
    LmLmxStructure *descriptor;
    void (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, LmCondition *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_condition_delete = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_condition_deleteClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_condition_deleteClosure) || lm_lmx_module_private_1_typedef_id_a_lm_condition_deleteClosure != 0x72f48ad6e76fdda0ULL || lm_lmx_module_private_1_typedef_id_b_lm_condition_deleteClosure != 0x1c1a5db0d0cf6db5ULL
#error "Lingvamyxa conflicting typedef projection for lm_condition_deleteClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_condition_wait
#define lm_lmx_module_private_1_typedef_defined_lm_condition_wait 1
#define lm_lmx_module_private_1_typedef_id_a_lm_condition_wait 0x6197b991480524f1ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_condition_wait 0x16b02780a77356e2ULL
typedef struct lm_condition_waitClosure *lm_condition_wait;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_condition_wait) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_condition_wait) || lm_lmx_module_private_1_typedef_id_a_lm_condition_wait != 0x6197b991480524f1ULL || lm_lmx_module_private_1_typedef_id_b_lm_condition_wait != 0x16b02780a77356e2ULL
#error "Lingvamyxa conflicting typedef projection for lm_condition_wait"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_condition_waitClosure
#define lm_lmx_module_private_1_typedef_defined_lm_condition_waitClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_condition_waitClosure 0x4299e6ebdeb2960fULL
#define lm_lmx_module_private_1_typedef_id_b_lm_condition_waitClosure 0xd8dfa63b76f98b88ULL
typedef struct lm_condition_waitClosure lm_condition_waitClosure;
struct lm_condition_waitClosure {
    LmLmxStructure *descriptor;
    int (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, LmCondition *, LmMutex *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_condition_wait = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_condition_waitClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_condition_waitClosure) || lm_lmx_module_private_1_typedef_id_a_lm_condition_waitClosure != 0x4299e6ebdeb2960fULL || lm_lmx_module_private_1_typedef_id_b_lm_condition_waitClosure != 0xd8dfa63b76f98b88ULL
#error "Lingvamyxa conflicting typedef projection for lm_condition_waitClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_condition_signal
#define lm_lmx_module_private_1_typedef_defined_lm_condition_signal 1
#define lm_lmx_module_private_1_typedef_id_a_lm_condition_signal 0xc808ed4b3deda515ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_condition_signal 0x15f6f9f5aa7157a6ULL
typedef struct lm_condition_signalClosure *lm_condition_signal;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_condition_signal) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_condition_signal) || lm_lmx_module_private_1_typedef_id_a_lm_condition_signal != 0xc808ed4b3deda515ULL || lm_lmx_module_private_1_typedef_id_b_lm_condition_signal != 0x15f6f9f5aa7157a6ULL
#error "Lingvamyxa conflicting typedef projection for lm_condition_signal"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_condition_signalClosure
#define lm_lmx_module_private_1_typedef_defined_lm_condition_signalClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_condition_signalClosure 0x8d6b3153b12ed6cfULL
#define lm_lmx_module_private_1_typedef_id_b_lm_condition_signalClosure 0xd2db45f281992e2eULL
typedef struct lm_condition_signalClosure lm_condition_signalClosure;
struct lm_condition_signalClosure {
    LmLmxStructure *descriptor;
    int (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, LmCondition *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_condition_signal = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_condition_signalClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_condition_signalClosure) || lm_lmx_module_private_1_typedef_id_a_lm_condition_signalClosure != 0x8d6b3153b12ed6cfULL || lm_lmx_module_private_1_typedef_id_b_lm_condition_signalClosure != 0xd2db45f281992e2eULL
#error "Lingvamyxa conflicting typedef projection for lm_condition_signalClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_condition_broadcast
#define lm_lmx_module_private_1_typedef_defined_lm_condition_broadcast 1
#define lm_lmx_module_private_1_typedef_id_a_lm_condition_broadcast 0x9f7725b4c3a1820dULL
#define lm_lmx_module_private_1_typedef_id_b_lm_condition_broadcast 0x81d88d4b74aaa5eaULL
typedef struct lm_condition_broadcastClosure *lm_condition_broadcast;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_condition_broadcast) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_condition_broadcast) || lm_lmx_module_private_1_typedef_id_a_lm_condition_broadcast != 0x9f7725b4c3a1820dULL || lm_lmx_module_private_1_typedef_id_b_lm_condition_broadcast != 0x81d88d4b74aaa5eaULL
#error "Lingvamyxa conflicting typedef projection for lm_condition_broadcast"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_condition_broadcastClosure
#define lm_lmx_module_private_1_typedef_defined_lm_condition_broadcastClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_condition_broadcastClosure 0x444b83176e6ad195ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_condition_broadcastClosure 0x59f03434918a0e30ULL
typedef struct lm_condition_broadcastClosure lm_condition_broadcastClosure;
struct lm_condition_broadcastClosure {
    LmLmxStructure *descriptor;
    int (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, LmCondition *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_condition_broadcast = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_condition_broadcastClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_condition_broadcastClosure) || lm_lmx_module_private_1_typedef_id_a_lm_condition_broadcastClosure != 0x444b83176e6ad195ULL || lm_lmx_module_private_1_typedef_id_b_lm_condition_broadcastClosure != 0x59f03434918a0e30ULL
#error "Lingvamyxa conflicting typedef projection for lm_condition_broadcastClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_runtime_new
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_runtime_new 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_runtime_new 0xb7aa2437a097bb21ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_runtime_new 0x1719aa4894988dbaULL
typedef struct lm_message_thread_runtime_newClosure *lm_message_thread_runtime_new;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_runtime_new) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_runtime_new) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_runtime_new != 0xb7aa2437a097bb21ULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_runtime_new != 0x1719aa4894988dbaULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_runtime_new"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_runtime_newClosure
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_runtime_newClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_runtime_newClosure 0xfe07c62294f6d272ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_runtime_newClosure 0x08d80b65e0c1b543ULL
typedef struct lm_message_thread_runtime_newClosure lm_message_thread_runtime_newClosure;
struct lm_message_thread_runtime_newClosure {
    LmLmxStructure *descriptor;
    LmMessageThreadRuntime * (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_message_thread_runtime_new = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_runtime_newClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_runtime_newClosure) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_runtime_newClosure != 0xfe07c62294f6d272ULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_runtime_newClosure != 0x08d80b65e0c1b543ULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_runtime_newClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_runtime_delete
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_runtime_delete 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_runtime_delete 0xab88fc2c6092fe69ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_runtime_delete 0xd6e4469e24d160d6ULL
typedef struct lm_message_thread_runtime_deleteClosure *lm_message_thread_runtime_delete;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_runtime_delete) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_runtime_delete) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_runtime_delete != 0xab88fc2c6092fe69ULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_runtime_delete != 0xd6e4469e24d160d6ULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_runtime_delete"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_runtime_deleteClosure
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_runtime_deleteClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_runtime_deleteClosure 0xef000e069fd1b849ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_runtime_deleteClosure 0xded442a376426366ULL
typedef struct lm_message_thread_runtime_deleteClosure lm_message_thread_runtime_deleteClosure;
struct lm_message_thread_runtime_deleteClosure {
    LmLmxStructure *descriptor;
    int (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, LmMessageThreadRuntime *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_message_thread_runtime_delete = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_runtime_deleteClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_runtime_deleteClosure) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_runtime_deleteClosure != 0xef000e069fd1b849ULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_runtime_deleteClosure != 0xded442a376426366ULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_runtime_deleteClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_runtime_set_rest_lmx_provider
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_runtime_set_rest_lmx_provider 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_runtime_set_rest_lmx_provider 0x98ba4e2323905379ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_runtime_set_rest_lmx_provider 0x9ff15b8f15525252ULL
typedef struct lm_message_thread_runtime_set_rest_lmx_providerClosure *lm_message_thread_runtime_set_rest_lmx_provider;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_runtime_set_rest_lmx_provider) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_runtime_set_rest_lmx_provider) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_runtime_set_rest_lmx_provider != 0x98ba4e2323905379ULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_runtime_set_rest_lmx_provider != 0x9ff15b8f15525252ULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_runtime_set_rest_lmx_provider"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_runtime_set_rest_lmx_providerClosure
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_runtime_set_rest_lmx_providerClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_runtime_set_rest_lmx_providerClosure 0x1dee7a1330b1e492ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_runtime_set_rest_lmx_providerClosure 0x00c032951ac6c4b3ULL
typedef struct lm_message_thread_runtime_set_rest_lmx_providerClosure lm_message_thread_runtime_set_rest_lmx_providerClosure;
struct lm_message_thread_runtime_set_rest_lmx_providerClosure {
    LmLmxStructure *descriptor;
    int (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, LmMessageThreadRuntime *, const LmRestLmxProviderOpsV1 *, void *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_message_thread_runtime_set_rest_lmx_provider = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_runtime_set_rest_lmx_providerClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_runtime_set_rest_lmx_providerClosure) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_runtime_set_rest_lmx_providerClosure != 0x1dee7a1330b1e492ULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_runtime_set_rest_lmx_providerClosure != 0x00c032951ac6c4b3ULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_runtime_set_rest_lmx_providerClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_runtime_admit_lmx
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_runtime_admit_lmx 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_runtime_admit_lmx 0xe265df049c37458dULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_runtime_admit_lmx 0x4fad177794668716ULL
typedef struct lm_message_thread_runtime_admit_lmxClosure *lm_message_thread_runtime_admit_lmx;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_runtime_admit_lmx) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_runtime_admit_lmx) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_runtime_admit_lmx != 0xe265df049c37458dULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_runtime_admit_lmx != 0x4fad177794668716ULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_runtime_admit_lmx"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_runtime_admit_lmxClosure
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_runtime_admit_lmxClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_runtime_admit_lmxClosure 0x7784694e52160327ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_runtime_admit_lmxClosure 0xcbd48263a29607c0ULL
typedef struct lm_message_thread_runtime_admit_lmxClosure lm_message_thread_runtime_admit_lmxClosure;
struct lm_message_thread_runtime_admit_lmxClosure {
    LmLmxStructure *descriptor;
    int (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, LmMessageThreadRuntime *, const char *, const char *, size_t);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_message_thread_runtime_admit_lmx = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_runtime_admit_lmxClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_runtime_admit_lmxClosure) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_runtime_admit_lmxClosure != 0x7784694e52160327ULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_runtime_admit_lmxClosure != 0xcbd48263a29607c0ULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_runtime_admit_lmxClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_runtime_attach_root
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_runtime_attach_root 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_runtime_attach_root 0x2ba40df3fc08253dULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_runtime_attach_root 0xfe98f5f132ccc416ULL
typedef struct lm_message_thread_runtime_attach_rootClosure *lm_message_thread_runtime_attach_root;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_runtime_attach_root) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_runtime_attach_root) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_runtime_attach_root != 0x2ba40df3fc08253dULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_runtime_attach_root != 0xfe98f5f132ccc416ULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_runtime_attach_root"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_runtime_attach_rootClosure
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_runtime_attach_rootClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_runtime_attach_rootClosure 0x719c638b78a148c9ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_runtime_attach_rootClosure 0xd95904739299ec60ULL
typedef struct lm_message_thread_runtime_attach_rootClosure lm_message_thread_runtime_attach_rootClosure;
struct lm_message_thread_runtime_attach_rootClosure {
    LmLmxStructure *descriptor;
    int (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, LmMessageThreadRuntime *, LmMessageThread *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_message_thread_runtime_attach_root = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_runtime_attach_rootClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_runtime_attach_rootClosure) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_runtime_attach_rootClosure != 0x719c638b78a148c9ULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_runtime_attach_rootClosure != 0xd95904739299ec60ULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_runtime_attach_rootClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_runtime_detach_root
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_runtime_detach_root 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_runtime_detach_root 0xe4f92016b95a20edULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_runtime_detach_root 0xc88038db69484a7eULL
typedef struct lm_message_thread_runtime_detach_rootClosure *lm_message_thread_runtime_detach_root;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_runtime_detach_root) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_runtime_detach_root) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_runtime_detach_root != 0xe4f92016b95a20edULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_runtime_detach_root != 0xc88038db69484a7eULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_runtime_detach_root"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_runtime_detach_rootClosure
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_runtime_detach_rootClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_runtime_detach_rootClosure 0x120c798ffd0efb79ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_runtime_detach_rootClosure 0xb5ca09c340390570ULL
typedef struct lm_message_thread_runtime_detach_rootClosure lm_message_thread_runtime_detach_rootClosure;
struct lm_message_thread_runtime_detach_rootClosure {
    LmLmxStructure *descriptor;
    int (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, LmMessageThreadRuntime *, LmMessageThread *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_message_thread_runtime_detach_root = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_runtime_detach_rootClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_runtime_detach_rootClosure) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_runtime_detach_rootClosure != 0x120c798ffd0efb79ULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_runtime_detach_rootClosure != 0xb5ca09c340390570ULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_runtime_detach_rootClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_runtime_exit_state
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_runtime_exit_state 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_runtime_exit_state 0x6824ffeb4963f893ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_runtime_exit_state 0x264cbd6de5d29f68ULL
typedef struct lm_message_thread_runtime_exit_stateClosure *lm_message_thread_runtime_exit_state;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_runtime_exit_state) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_runtime_exit_state) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_runtime_exit_state != 0x6824ffeb4963f893ULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_runtime_exit_state != 0x264cbd6de5d29f68ULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_runtime_exit_state"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_runtime_exit_stateClosure
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_runtime_exit_stateClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_runtime_exit_stateClosure 0xbdb5bfbaeced141aULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_runtime_exit_stateClosure 0xdcd6f9c22c90e673ULL
typedef struct lm_message_thread_runtime_exit_stateClosure lm_message_thread_runtime_exit_stateClosure;
struct lm_message_thread_runtime_exit_stateClosure {
    LmLmxStructure *descriptor;
    int (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, LmMessageThreadRuntime *, int *, int *, int *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_message_thread_runtime_exit_state = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_runtime_exit_stateClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_runtime_exit_stateClosure) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_runtime_exit_stateClosure != 0xbdb5bfbaeced141aULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_runtime_exit_stateClosure != 0xdcd6f9c22c90e673ULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_runtime_exit_stateClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_pool_new
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_pool_new 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_pool_new 0x62993521498bba4fULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_pool_new 0xc8c6ca60b7bf1d64ULL
typedef struct lm_message_thread_pool_newClosure *lm_message_thread_pool_new;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_pool_new) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_pool_new) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_pool_new != 0x62993521498bba4fULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_pool_new != 0xc8c6ca60b7bf1d64ULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_pool_new"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_pool_newClosure
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_pool_newClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_pool_newClosure 0xba17bd908bc93eb4ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_pool_newClosure 0xd359ed8df1d40985ULL
typedef struct lm_message_thread_pool_newClosure lm_message_thread_pool_newClosure;
struct lm_message_thread_pool_newClosure {
    LmLmxStructure *descriptor;
    LmMessageThreadPool * (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, LmMessageThreadRuntime *, size_t);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_message_thread_pool_new = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_pool_newClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_pool_newClosure) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_pool_newClosure != 0xba17bd908bc93eb4ULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_pool_newClosure != 0xd359ed8df1d40985ULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_pool_newClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_pool_request_stop
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_pool_request_stop 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_pool_request_stop 0x0a9565c13b3947a1ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_pool_request_stop 0xdacfdb3cc36e51caULL
typedef struct lm_message_thread_pool_request_stopClosure *lm_message_thread_pool_request_stop;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_pool_request_stop) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_pool_request_stop) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_pool_request_stop != 0x0a9565c13b3947a1ULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_pool_request_stop != 0xdacfdb3cc36e51caULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_pool_request_stop"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_pool_request_stopClosure
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_pool_request_stopClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_pool_request_stopClosure 0x85c40321e8d8f568ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_pool_request_stopClosure 0xa6e03be25da36631ULL
typedef struct lm_message_thread_pool_request_stopClosure lm_message_thread_pool_request_stopClosure;
struct lm_message_thread_pool_request_stopClosure {
    LmLmxStructure *descriptor;
    void (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, LmMessageThreadPool *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_message_thread_pool_request_stop = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_pool_request_stopClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_pool_request_stopClosure) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_pool_request_stopClosure != 0x85c40321e8d8f568ULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_pool_request_stopClosure != 0xa6e03be25da36631ULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_pool_request_stopClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_pool_request_stop_when_idle
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_pool_request_stop_when_idle 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_pool_request_stop_when_idle 0xcb730959ad21fcd9ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_pool_request_stop_when_idle 0xe4adcc1915a29392ULL
typedef struct lm_message_thread_pool_request_stop_when_idleClosure *lm_message_thread_pool_request_stop_when_idle;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_pool_request_stop_when_idle) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_pool_request_stop_when_idle) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_pool_request_stop_when_idle != 0xcb730959ad21fcd9ULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_pool_request_stop_when_idle != 0xe4adcc1915a29392ULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_pool_request_stop_when_idle"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_pool_request_stop_when_idleClosure
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_pool_request_stop_when_idleClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_pool_request_stop_when_idleClosure 0x24e9793cb67d9ec0ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_pool_request_stop_when_idleClosure 0x4c919e51954dbb21ULL
typedef struct lm_message_thread_pool_request_stop_when_idleClosure lm_message_thread_pool_request_stop_when_idleClosure;
struct lm_message_thread_pool_request_stop_when_idleClosure {
    LmLmxStructure *descriptor;
    void (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, LmMessageThreadPool *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_message_thread_pool_request_stop_when_idle = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_pool_request_stop_when_idleClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_pool_request_stop_when_idleClosure) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_pool_request_stop_when_idleClosure != 0x24e9793cb67d9ec0ULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_pool_request_stop_when_idleClosure != 0x4c919e51954dbb21ULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_pool_request_stop_when_idleClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_pool_pump
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_pool_pump 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_pool_pump 0x7f11483d1785e969ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_pool_pump 0xf5e9a0a41ab47b52ULL
typedef struct lm_message_thread_pool_pumpClosure *lm_message_thread_pool_pump;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_pool_pump) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_pool_pump) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_pool_pump != 0x7f11483d1785e969ULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_pool_pump != 0xf5e9a0a41ab47b52ULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_pool_pump"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_pool_pumpClosure
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_pool_pumpClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_pool_pumpClosure 0x20084cc4393fd50eULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_pool_pumpClosure 0x51931a9566a631afULL
typedef struct lm_message_thread_pool_pumpClosure lm_message_thread_pool_pumpClosure;
struct lm_message_thread_pool_pumpClosure {
    LmLmxStructure *descriptor;
    size_t (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, LmMessageThreadPool *, size_t);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_message_thread_pool_pump = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_pool_pumpClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_pool_pumpClosure) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_pool_pumpClosure != 0x20084cc4393fd50eULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_pool_pumpClosure != 0x51931a9566a631afULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_pool_pumpClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_pool_delete
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_pool_delete 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_pool_delete 0xa82a601d6b0b2671ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_pool_delete 0x6ded556349bf8802ULL
typedef struct lm_message_thread_pool_deleteClosure *lm_message_thread_pool_delete;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_pool_delete) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_pool_delete) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_pool_delete != 0xa82a601d6b0b2671ULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_pool_delete != 0x6ded556349bf8802ULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_pool_delete"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_pool_deleteClosure
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_pool_deleteClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_pool_deleteClosure 0xe90c4d385e5d7f2dULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_pool_deleteClosure 0xbd62d34a2cb1d1f8ULL
typedef struct lm_message_thread_pool_deleteClosure lm_message_thread_pool_deleteClosure;
struct lm_message_thread_pool_deleteClosure {
    LmLmxStructure *descriptor;
    int (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, LmMessageThreadPool *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_message_thread_pool_delete = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_pool_deleteClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_pool_deleteClosure) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_pool_deleteClosure != 0xe90c4d385e5d7f2dULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_pool_deleteClosure != 0xbd62d34a2cb1d1f8ULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_pool_deleteClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_init
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_init 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_init 0x0d933431eaf41169ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_init 0xbb0b3bdb175f859eULL
typedef struct lm_message_thread_initClosure *lm_message_thread_init;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_init) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_init) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_init != 0x0d933431eaf41169ULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_init != 0xbb0b3bdb175f859eULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_init"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_initClosure
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_initClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_initClosure 0x568bc3864233277bULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_initClosure 0xacd4f12fc023b44eULL
typedef struct lm_message_thread_initClosure lm_message_thread_initClosure;
struct lm_message_thread_initClosure {
    LmLmxStructure *descriptor;
    int (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, LmMessageThread *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_message_thread_init = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_initClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_initClosure) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_initClosure != 0x568bc3864233277bULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_initClosure != 0xacd4f12fc023b44eULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_initClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_destroy
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_destroy 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_destroy 0xc4c03650a2bc4131ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_destroy 0xa0a0e903292511aaULL
typedef struct lm_message_thread_destroyClosure *lm_message_thread_destroy;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_destroy) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_destroy) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_destroy != 0xc4c03650a2bc4131ULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_destroy != 0xa0a0e903292511aaULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_destroy"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_destroyClosure
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_destroyClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_destroyClosure 0x457504a012abe9aaULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_destroyClosure 0xd2d8f949e24482ebULL
typedef struct lm_message_thread_destroyClosure lm_message_thread_destroyClosure;
struct lm_message_thread_destroyClosure {
    LmLmxStructure *descriptor;
    void (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, LmMessageThread *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_message_thread_destroy = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_destroyClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_destroyClosure) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_destroyClosure != 0x457504a012abe9aaULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_destroyClosure != 0xd2d8f949e24482ebULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_destroyClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_new
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_new 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_new 0x5a0f04a45da193a9ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_new 0x35039a0f9aa6bf22ULL
typedef struct lm_message_thread_newClosure *lm_message_thread_new;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_new) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_new) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_new != 0x5a0f04a45da193a9ULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_new != 0x35039a0f9aa6bf22ULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_new"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_newClosure
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_newClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_newClosure 0x7921287cbabf8ffaULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_newClosure 0x6fda88dca299d83bULL
typedef struct lm_message_thread_newClosure lm_message_thread_newClosure;
struct lm_message_thread_newClosure {
    LmLmxStructure *descriptor;
    LmMessageThread * (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_message_thread_new = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_newClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_newClosure) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_newClosure != 0x7921287cbabf8ffaULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_newClosure != 0x6fda88dca299d83bULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_newClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_new_in
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_new_in 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_new_in 0x9f0ca334a61d0ae9ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_new_in 0x439e1e36e2cdabcaULL
typedef struct lm_message_thread_new_inClosure *lm_message_thread_new_in;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_new_in) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_new_in) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_new_in != 0x9f0ca334a61d0ae9ULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_new_in != 0x439e1e36e2cdabcaULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_new_in"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_new_inClosure
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_new_inClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_new_inClosure 0x64f727914053aa06ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_new_inClosure 0xbf226375d2b7e25fULL
typedef struct lm_message_thread_new_inClosure lm_message_thread_new_inClosure;
struct lm_message_thread_new_inClosure {
    LmLmxStructure *descriptor;
    LmMessageThread * (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, LmMessageThreadPool *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_message_thread_new_in = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_new_inClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_new_inClosure) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_new_inClosure != 0x64f727914053aa06ULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_new_inClosure != 0xbf226375d2b7e25fULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_new_inClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_delete
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_delete 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_delete 0xc7d9f4d616133ebbULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_delete 0x1ce29ec65ab94df4ULL
typedef struct lm_message_thread_deleteClosure *lm_message_thread_delete;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_delete) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_delete) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_delete != 0xc7d9f4d616133ebbULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_delete != 0x1ce29ec65ab94df4ULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_delete"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_deleteClosure
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_deleteClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_deleteClosure 0xf30e513715c14e30ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_deleteClosure 0x844e6c6e1f9bb6f1ULL
typedef struct lm_message_thread_deleteClosure lm_message_thread_deleteClosure;
struct lm_message_thread_deleteClosure {
    LmLmxStructure *descriptor;
    void (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, LmMessageThread *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_message_thread_delete = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_deleteClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_deleteClosure) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_deleteClosure != 0xf30e513715c14e30ULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_deleteClosure != 0x844e6c6e1f9bb6f1ULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_deleteClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_start
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_start 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_start 0xda4a2f9f529226a5ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_start 0xd3d1711db892124eULL
typedef struct lm_message_thread_startClosure *lm_message_thread_start;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_start) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_start) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_start != 0xda4a2f9f529226a5ULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_start != 0xd3d1711db892124eULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_start"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_startClosure
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_startClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_startClosure 0x0429fdff44a2e04dULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_startClosure 0x1cc487e8ec099b94ULL
typedef struct lm_message_thread_startClosure lm_message_thread_startClosure;
struct lm_message_thread_startClosure {
    LmLmxStructure *descriptor;
    int (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, LmMessageThread *, LmMessageThreadEntry, void *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_message_thread_start = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_startClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_startClosure) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_startClosure != 0x0429fdff44a2e04dULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_startClosure != 0x1cc487e8ec099b94ULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_startClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_start_mailbox
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_start_mailbox 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_start_mailbox 0x8520250e5a371efdULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_start_mailbox 0x4d10f860c235cc1eULL
typedef struct lm_message_thread_start_mailboxClosure *lm_message_thread_start_mailbox;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_start_mailbox) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_start_mailbox) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_start_mailbox != 0x8520250e5a371efdULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_start_mailbox != 0x4d10f860c235cc1eULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_start_mailbox"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_start_mailboxClosure
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_start_mailboxClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_start_mailboxClosure 0xfc9dd1dbf5632d23ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_start_mailboxClosure 0x9b592866845a59c6ULL
typedef struct lm_message_thread_start_mailboxClosure lm_message_thread_start_mailboxClosure;
struct lm_message_thread_start_mailboxClosure {
    LmLmxStructure *descriptor;
    int (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, LmMessageThread *, LmMessageThreadEntry, void *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_message_thread_start_mailbox = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_start_mailboxClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_start_mailboxClosure) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_start_mailboxClosure != 0xfc9dd1dbf5632d23ULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_start_mailboxClosure != 0x9b592866845a59c6ULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_start_mailboxClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_join
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_join 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_join 0x9d17bd0d5204e495ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_join 0x6728371a22fbac12ULL
typedef struct lm_message_thread_joinClosure *lm_message_thread_join;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_join) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_join) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_join != 0x9d17bd0d5204e495ULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_join != 0x6728371a22fbac12ULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_join"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_joinClosure
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_joinClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_joinClosure 0x6be6914cda2b520cULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_joinClosure 0x3e517b3d72bd9071ULL
typedef struct lm_message_thread_joinClosure lm_message_thread_joinClosure;
struct lm_message_thread_joinClosure {
    LmLmxStructure *descriptor;
    int (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, LmMessageThread *, int *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_message_thread_join = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_joinClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_joinClosure) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_joinClosure != 0x6be6914cda2b520cULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_joinClosure != 0x3e517b3d72bd9071ULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_joinClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_bind_route
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_bind_route 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_bind_route 0x845f45e8b433c66fULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_bind_route 0x57475692dd9b4b54ULL
typedef struct lm_message_thread_bind_routeClosure *lm_message_thread_bind_route;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_bind_route) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_bind_route) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_bind_route != 0x845f45e8b433c66fULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_bind_route != 0x57475692dd9b4b54ULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_bind_route"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_bind_routeClosure
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_bind_routeClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_bind_routeClosure 0x4bb235d59aed5d90ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_bind_routeClosure 0xcc84f886332c5265ULL
typedef struct lm_message_thread_bind_routeClosure lm_message_thread_bind_routeClosure;
struct lm_message_thread_bind_routeClosure {
    LmLmxStructure *descriptor;
    int (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, LmMessageThread *, const char *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_message_thread_bind_route = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_bind_routeClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_bind_routeClosure) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_bind_routeClosure != 0x4bb235d59aed5d90ULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_bind_routeClosure != 0xcc84f886332c5265ULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_bind_routeClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_send_lmx
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_send_lmx 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_send_lmx 0xdc20041ec424e5e5ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_send_lmx 0xd8f9c99ab621e732ULL
typedef struct lm_message_thread_send_lmxClosure *lm_message_thread_send_lmx;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_send_lmx) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_send_lmx) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_send_lmx != 0xdc20041ec424e5e5ULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_send_lmx != 0xd8f9c99ab621e732ULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_send_lmx"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_send_lmxClosure
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_send_lmxClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_send_lmxClosure 0xca455ea8618e680cULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_send_lmxClosure 0xea93c8958a51ab0dULL
typedef struct lm_message_thread_send_lmxClosure lm_message_thread_send_lmxClosure;
struct lm_message_thread_send_lmxClosure {
    LmLmxStructure *descriptor;
    int (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, LmMessageThread *, const char *, const char *, const char *, size_t);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_message_thread_send_lmx = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_send_lmxClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_send_lmxClosure) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_send_lmxClosure != 0xca455ea8618e680cULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_send_lmxClosure != 0xea93c8958a51ab0dULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_send_lmxClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_current_lmx
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_current_lmx 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_current_lmx 0x486e3cd979a98b15ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_current_lmx 0x60bd77f100416ffeULL
typedef struct lm_message_thread_current_lmxClosure *lm_message_thread_current_lmx;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_current_lmx) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_current_lmx) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_current_lmx != 0x486e3cd979a98b15ULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_current_lmx != 0x60bd77f100416ffeULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_current_lmx"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_current_lmxClosure
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_current_lmxClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_current_lmxClosure 0x7f979bf7a28246deULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_current_lmxClosure 0x1fdc881092436917ULL
typedef struct lm_message_thread_current_lmxClosure lm_message_thread_current_lmxClosure;
struct lm_message_thread_current_lmxClosure {
    LmLmxStructure *descriptor;
    int (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, LmMessageThread *, const char **, size_t *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_message_thread_current_lmx = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_current_lmxClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_current_lmxClosure) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_current_lmxClosure != 0x7f979bf7a28246deULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_current_lmxClosure != 0x1fdc881092436917ULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_current_lmxClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_inbox_count
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_inbox_count 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_inbox_count 0x205f949eeade43e9ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_inbox_count 0x36e79457392be6a2ULL
typedef struct lm_message_thread_inbox_countClosure *lm_message_thread_inbox_count;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_inbox_count) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_inbox_count) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_inbox_count != 0x205f949eeade43e9ULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_inbox_count != 0x36e79457392be6a2ULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_inbox_count"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_inbox_countClosure
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_inbox_countClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_inbox_countClosure 0x6517798e5665f2c9ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_inbox_countClosure 0x02127edcf319488aULL
typedef struct lm_message_thread_inbox_countClosure lm_message_thread_inbox_countClosure;
struct lm_message_thread_inbox_countClosure {
    LmLmxStructure *descriptor;
    size_t (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, const LmMessageThread *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_message_thread_inbox_count = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_inbox_countClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_inbox_countClosure) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_inbox_countClosure != 0x6517798e5665f2c9ULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_inbox_countClosure != 0x02127edcf319488aULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_inbox_countClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_outbox_count
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_outbox_count 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_outbox_count 0x43f7434dccb44943ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_outbox_count 0xd85278bb31c2b2f0ULL
typedef struct lm_message_thread_outbox_countClosure *lm_message_thread_outbox_count;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_outbox_count) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_outbox_count) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_outbox_count != 0x43f7434dccb44943ULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_outbox_count != 0xd85278bb31c2b2f0ULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_outbox_count"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_outbox_countClosure
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_outbox_countClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_outbox_countClosure 0xb4b4309c37298761ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_outbox_countClosure 0x4257b4948c621e82ULL
typedef struct lm_message_thread_outbox_countClosure lm_message_thread_outbox_countClosure;
struct lm_message_thread_outbox_countClosure {
    LmLmxStructure *descriptor;
    size_t (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, const LmMessageThread *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_message_thread_outbox_count = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_outbox_countClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_outbox_countClosure) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_outbox_countClosure != 0xb4b4309c37298761ULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_outbox_countClosure != 0x4257b4948c621e82ULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_outbox_countClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_begin_turn
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_begin_turn 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_begin_turn 0xaf00f2b0ed002bafULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_begin_turn 0x893fb39d28ac0408ULL
typedef struct lm_message_thread_begin_turnClosure *lm_message_thread_begin_turn;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_begin_turn) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_begin_turn) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_begin_turn != 0xaf00f2b0ed002bafULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_begin_turn != 0x893fb39d28ac0408ULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_begin_turn"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_begin_turnClosure
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_begin_turnClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_begin_turnClosure 0x7511251c941ed319ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_begin_turnClosure 0x90278485900ba0e4ULL
typedef struct lm_message_thread_begin_turnClosure lm_message_thread_begin_turnClosure;
struct lm_message_thread_begin_turnClosure {
    LmLmxStructure *descriptor;
    int (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, LmMessageThread *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_message_thread_begin_turn = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_begin_turnClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_begin_turnClosure) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_begin_turnClosure != 0x7511251c941ed319ULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_begin_turnClosure != 0x90278485900ba0e4ULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_begin_turnClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_end_turn
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_end_turn 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_end_turn 0x149411138f3896bbULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_end_turn 0xb6269dc3965ac424ULL
typedef struct lm_message_thread_end_turnClosure *lm_message_thread_end_turn;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_end_turn) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_end_turn) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_end_turn != 0x149411138f3896bbULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_end_turn != 0xb6269dc3965ac424ULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_end_turn"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_end_turnClosure
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_end_turnClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_end_turnClosure 0x547db359f180c439ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_end_turnClosure 0x25a0cf3ef01ac07cULL
typedef struct lm_message_thread_end_turnClosure lm_message_thread_end_turnClosure;
struct lm_message_thread_end_turnClosure {
    LmLmxStructure *descriptor;
    int (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, LmMessageThread *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_message_thread_end_turn = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_end_turnClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_end_turnClosure) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_end_turnClosure != 0x547db359f180c439ULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_end_turnClosure != 0x25a0cf3ef01ac07cULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_end_turnClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_collect
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_collect 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_collect 0x09a9f5331a6b1c99ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_collect 0xe296dbf6bc357132ULL
typedef struct lm_message_thread_collectClosure *lm_message_thread_collect;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_collect) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_collect) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_collect != 0x09a9f5331a6b1c99ULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_collect != 0xe296dbf6bc357132ULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_collect"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_collectClosure
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_collectClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_collectClosure 0x5f85c408db1e136fULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_collectClosure 0x610f1f855e32b0daULL
typedef struct lm_message_thread_collectClosure lm_message_thread_collectClosure;
struct lm_message_thread_collectClosure {
    LmLmxStructure *descriptor;
    int (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, LmMessageThread *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_message_thread_collect = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_collectClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_collectClosure) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_collectClosure != 0x5f85c408db1e136fULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_collectClosure != 0x610f1f855e32b0daULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_collectClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_request_stop
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_request_stop 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_request_stop 0x9a227e37f842c299ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_request_stop 0xd650ef22c1457f96ULL
typedef struct lm_message_thread_request_stopClosure *lm_message_thread_request_stop;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_request_stop) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_request_stop) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_request_stop != 0x9a227e37f842c299ULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_request_stop != 0xd650ef22c1457f96ULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_request_stop"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_request_stopClosure
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_request_stopClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_request_stopClosure 0xf44bff450e0e1d87ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_request_stopClosure 0x925d475fbe11e8daULL
typedef struct lm_message_thread_request_stopClosure lm_message_thread_request_stopClosure;
struct lm_message_thread_request_stopClosure {
    LmLmxStructure *descriptor;
    void (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, LmMessageThread *, int);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_message_thread_request_stop = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_request_stopClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_request_stopClosure) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_request_stopClosure != 0xf44bff450e0e1d87ULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_request_stopClosure != 0x925d475fbe11e8daULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_request_stopClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_request_failure
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_request_failure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_request_failure 0x5d4789d41d106339ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_request_failure 0xec7c56f01e5caa7aULL
typedef struct lm_message_thread_request_failureClosure *lm_message_thread_request_failure;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_request_failure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_request_failure) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_request_failure != 0x5d4789d41d106339ULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_request_failure != 0xec7c56f01e5caa7aULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_request_failure"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_request_failureClosure
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_request_failureClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_request_failureClosure 0x49e4f05292878bdfULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_request_failureClosure 0x4fbe4458922210a2ULL
typedef struct lm_message_thread_request_failureClosure lm_message_thread_request_failureClosure;
struct lm_message_thread_request_failureClosure {
    LmLmxStructure *descriptor;
    void (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, LmMessageThread *, int);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_message_thread_request_failure = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_request_failureClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_request_failureClosure) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_request_failureClosure != 0x49e4f05292878bdfULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_request_failureClosure != 0x4fbe4458922210a2ULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_request_failureClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_request_exit
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_request_exit 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_request_exit 0xa74ae989a1e5c479ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_request_exit 0x16cbba84c1fec77eULL
typedef struct lm_message_thread_request_exitClosure *lm_message_thread_request_exit;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_request_exit) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_request_exit) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_request_exit != 0xa74ae989a1e5c479ULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_request_exit != 0x16cbba84c1fec77eULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_request_exit"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_request_exitClosure
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_request_exitClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_request_exitClosure 0x7a25a8c7ee398136ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_request_exitClosure 0x550729f9bca9b137ULL
typedef struct lm_message_thread_request_exitClosure lm_message_thread_request_exitClosure;
struct lm_message_thread_request_exitClosure {
    LmLmxStructure *descriptor;
    int (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, LmMessageThread *, int);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_message_thread_request_exit = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_request_exitClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_request_exitClosure) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_request_exitClosure != 0x7a25a8c7ee398136ULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_request_exitClosure != 0x550729f9bca9b137ULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_request_exitClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_is_running
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_is_running 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_is_running 0xc0edbe30f2430b05ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_is_running 0xac253c9748f70892ULL
typedef struct lm_message_thread_is_runningClosure *lm_message_thread_is_running;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_is_running) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_is_running) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_is_running != 0xc0edbe30f2430b05ULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_is_running != 0xac253c9748f70892ULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_is_running"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_is_runningClosure
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_is_runningClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_is_runningClosure 0x8484befc42bc571eULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_is_runningClosure 0x3f31d80d3aa49ad7ULL
typedef struct lm_message_thread_is_runningClosure lm_message_thread_is_runningClosure;
struct lm_message_thread_is_runningClosure {
    LmLmxStructure *descriptor;
    int (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, const LmMessageThread *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_message_thread_is_running = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_is_runningClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_is_runningClosure) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_is_runningClosure != 0x8484befc42bc571eULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_is_runningClosure != 0x3f31d80d3aa49ad7ULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_is_runningClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_status
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_status 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_status 0x314a30b7448deae1ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_status 0x8ec9af9fa3ee3c02ULL
typedef struct lm_message_thread_statusClosure *lm_message_thread_status;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_status) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_status) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_status != 0x314a30b7448deae1ULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_status != 0x8ec9af9fa3ee3c02ULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_status"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_statusClosure
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_statusClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_statusClosure 0x6d540e76abf63886ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_statusClosure 0x824e10869eced3cfULL
typedef struct lm_message_thread_statusClosure lm_message_thread_statusClosure;
struct lm_message_thread_statusClosure {
    LmLmxStructure *descriptor;
    int (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, const LmMessageThread *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_message_thread_status = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_statusClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_statusClosure) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_statusClosure != 0x6d540e76abf63886ULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_statusClosure != 0x824e10869eced3cfULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_statusClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_owner
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_owner 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_owner 0x02fe85e230f69d25ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_owner 0xb5bc2609e19a55e6ULL
typedef struct lm_message_thread_ownerClosure *lm_message_thread_owner;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_owner) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_owner) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_owner != 0x02fe85e230f69d25ULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_owner != 0xb5bc2609e19a55e6ULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_owner"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_ownerClosure
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_ownerClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_ownerClosure 0x38d0151db53de562ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_ownerClosure 0xcd8bace9f56c6ea3ULL
typedef struct lm_message_thread_ownerClosure lm_message_thread_ownerClosure;
struct lm_message_thread_ownerClosure {
    LmLmxStructure *descriptor;
    LmOwnArena * (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, LmMessageThread *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_message_thread_owner = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_ownerClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_ownerClosure) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_ownerClosure != 0x38d0151db53de562ULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_ownerClosure != 0xcd8bace9f56c6ea3ULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_ownerClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_execution_context
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_execution_context 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_execution_context 0x3466f0839a9e25e1ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_execution_context 0x74c930972edd9772ULL
typedef struct lm_message_thread_execution_contextClosure *lm_message_thread_execution_context;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_execution_context) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_execution_context) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_execution_context != 0x3466f0839a9e25e1ULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_execution_context != 0x74c930972edd9772ULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_execution_context"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_execution_contextClosure
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_execution_contextClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_execution_contextClosure 0x97774bef14abb608ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_execution_contextClosure 0x189cd590f61fb90dULL
typedef struct lm_message_thread_execution_contextClosure lm_message_thread_execution_contextClosure;
struct lm_message_thread_execution_contextClosure {
    LmLmxStructure *descriptor;
    void * (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, LmMessageThread *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_message_thread_execution_context = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_execution_contextClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_execution_contextClosure) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_execution_contextClosure != 0x97774bef14abb608ULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_execution_contextClosure != 0x189cd590f61fb90dULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_execution_contextClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_set_execution_context
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_set_execution_context 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_set_execution_context 0x7cba9f631ed2af01ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_set_execution_context 0x1bc7cb96a12b98eaULL
typedef struct lm_message_thread_set_execution_contextClosure *lm_message_thread_set_execution_context;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_set_execution_context) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_set_execution_context) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_set_execution_context != 0x7cba9f631ed2af01ULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_set_execution_context != 0x1bc7cb96a12b98eaULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_set_execution_context"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_set_execution_contextClosure
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_set_execution_contextClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_set_execution_contextClosure 0xb941d4948cf358c0ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_set_execution_contextClosure 0x73f74fe4e9898d69ULL
typedef struct lm_message_thread_set_execution_contextClosure lm_message_thread_set_execution_contextClosure;
struct lm_message_thread_set_execution_contextClosure {
    LmLmxStructure *descriptor;
    void * (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, LmMessageThread *, void *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_message_thread_set_execution_context = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_set_execution_contextClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_set_execution_contextClosure) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_set_execution_contextClosure != 0xb941d4948cf358c0ULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_set_execution_contextClosure != 0x73f74fe4e9898d69ULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_set_execution_contextClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_turn_count
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_turn_count 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_turn_count 0x85ffb5db8818acc3ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_turn_count 0xf6e77b918042fa7cULL
typedef struct lm_message_thread_turn_countClosure *lm_message_thread_turn_count;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_turn_count) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_turn_count) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_turn_count != 0x85ffb5db8818acc3ULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_turn_count != 0xf6e77b918042fa7cULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_turn_count"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_turn_countClosure
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_turn_countClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_turn_countClosure 0x7968cddc1454e671ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_turn_countClosure 0x1f33e6a33fd1ff52ULL
typedef struct lm_message_thread_turn_countClosure lm_message_thread_turn_countClosure;
struct lm_message_thread_turn_countClosure {
    LmLmxStructure *descriptor;
    size_t (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, const LmMessageThread *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_message_thread_turn_count = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_turn_countClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_turn_countClosure) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_turn_countClosure != 0x7968cddc1454e671ULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_turn_countClosure != 0x1f33e6a33fd1ff52ULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_turn_countClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_collection_count
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_collection_count 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_collection_count 0x37e7a3a6457adf45ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_collection_count 0x842a7284fb0d8222ULL
typedef struct lm_message_thread_collection_countClosure *lm_message_thread_collection_count;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_collection_count) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_collection_count) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_collection_count != 0x37e7a3a6457adf45ULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_collection_count != 0x842a7284fb0d8222ULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_collection_count"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_collection_countClosure
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_collection_countClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_collection_countClosure 0x4c239f50bd28d2e1ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_collection_countClosure 0x2c075f8755602ad2ULL
typedef struct lm_message_thread_collection_countClosure lm_message_thread_collection_countClosure;
struct lm_message_thread_collection_countClosure {
    LmLmxStructure *descriptor;
    size_t (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, const LmMessageThread *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_message_thread_collection_count = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_collection_countClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_collection_countClosure) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_collection_countClosure != 0x4c239f50bd28d2e1ULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_collection_countClosure != 0x2c075f8755602ad2ULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_collection_countClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_arena_count
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_arena_count 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_arena_count 0xfc15df3a4ab3c095ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_arena_count 0x761ee69666a969b6ULL
typedef struct lm_message_thread_arena_countClosure *lm_message_thread_arena_count;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_arena_count) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_arena_count) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_arena_count != 0xfc15df3a4ab3c095ULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_arena_count != 0x761ee69666a969b6ULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_arena_count"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_arena_countClosure
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_arena_countClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_arena_countClosure 0x9fe32e5e90b04271ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_arena_countClosure 0xf6bf3e12130c5002ULL
typedef struct lm_message_thread_arena_countClosure lm_message_thread_arena_countClosure;
struct lm_message_thread_arena_countClosure {
    LmLmxStructure *descriptor;
    size_t (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, const LmMessageThread *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_message_thread_arena_count = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_arena_countClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_arena_countClosure) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_arena_countClosure != 0x9fe32e5e90b04271ULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_arena_countClosure != 0xf6bf3e12130c5002ULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_arena_countClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_component_attach
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_component_attach 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_component_attach 0x487171b81c8e9f03ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_component_attach 0xf5141fbd19b52ce0ULL
typedef struct lm_message_thread_component_attachClosure *lm_message_thread_component_attach;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_component_attach) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_component_attach) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_component_attach != 0x487171b81c8e9f03ULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_component_attach != 0xf5141fbd19b52ce0ULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_component_attach"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_component_attachClosure
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_component_attachClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_component_attachClosure 0xa52b6efbdf43a08eULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_component_attachClosure 0x901cdc7748a29a77ULL
typedef struct lm_message_thread_component_attachClosure lm_message_thread_component_attachClosure;
struct lm_message_thread_component_attachClosure {
    LmLmxStructure *descriptor;
    int (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, LmMessageThreadComponentDestroy, void *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_message_thread_component_attach = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_component_attachClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_component_attachClosure) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_component_attachClosure != 0xa52b6efbdf43a08eULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_component_attachClosure != 0x901cdc7748a29a77ULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_component_attachClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_component_get
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_component_get 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_component_get 0x4b9cd87902941689ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_component_get 0xf39d52ff89b4a73aULL
typedef struct lm_message_thread_component_getClosure *lm_message_thread_component_get;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_component_get) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_component_get) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_component_get != 0x4b9cd87902941689ULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_component_get != 0xf39d52ff89b4a73aULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_component_get"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_component_getClosure
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_component_getClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_component_getClosure 0xa2fbc20fd6ce9fd1ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_component_getClosure 0xa39fea920aa92d0aULL
typedef struct lm_message_thread_component_getClosure lm_message_thread_component_getClosure;
struct lm_message_thread_component_getClosure {
    LmLmxStructure *descriptor;
    void * (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, LmMessageThreadComponentDestroy);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_message_thread_component_get = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_component_getClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_component_getClosure) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_component_getClosure != 0xa2fbc20fd6ce9fd1ULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_component_getClosure != 0xa39fea920aa92d0aULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_component_getClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_component_remove
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_component_remove 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_component_remove 0x6c93e4c1c0de4c85ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_component_remove 0x64015c5eae797d92ULL
typedef struct lm_message_thread_component_removeClosure *lm_message_thread_component_remove;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_component_remove) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_component_remove) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_component_remove != 0x6c93e4c1c0de4c85ULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_component_remove != 0x64015c5eae797d92ULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_component_remove"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_component_removeClosure
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_component_removeClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_component_removeClosure 0xce2820eb17a9b620ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_component_removeClosure 0x782e4b1b08de2a45ULL
typedef struct lm_message_thread_component_removeClosure lm_message_thread_component_removeClosure;
struct lm_message_thread_component_removeClosure {
    LmLmxStructure *descriptor;
    int (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, LmMessageThreadComponentDestroy);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_message_thread_component_remove = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_component_removeClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_component_removeClosure) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_component_removeClosure != 0xce2820eb17a9b620ULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_component_removeClosure != 0x782e4b1b08de2a45ULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_component_removeClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_component_count
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_component_count 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_component_count 0x3743d1470f85fdb1ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_component_count 0xcef05d28fbb6c37aULL
typedef struct lm_message_thread_component_countClosure *lm_message_thread_component_count;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_component_count) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_component_count) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_component_count != 0x3743d1470f85fdb1ULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_component_count != 0xcef05d28fbb6c37aULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_component_count"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_message_thread_component_countClosure
#define lm_lmx_module_private_1_typedef_defined_lm_message_thread_component_countClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_message_thread_component_countClosure 0x43f5d041c12a9a01ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_message_thread_component_countClosure 0x4ebe5076c437b25aULL
typedef struct lm_message_thread_component_countClosure lm_message_thread_component_countClosure;
struct lm_message_thread_component_countClosure {
    LmLmxStructure *descriptor;
    size_t (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, const LmMessageThread *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_message_thread_component_count = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_message_thread_component_countClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_message_thread_component_countClosure) || lm_lmx_module_private_1_typedef_id_a_lm_message_thread_component_countClosure != 0x43f5d041c12a9a01ULL || lm_lmx_module_private_1_typedef_id_b_lm_message_thread_component_countClosure != 0x4ebe5076c437b25aULL
#error "Lingvamyxa conflicting typedef projection for lm_message_thread_component_countClosure"
#endif
#endif



































































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
