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
#include <stddef.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
#ifndef lm_lmx_module_private_1_typedef_defined_LmP0Node
#define lm_lmx_module_private_1_typedef_defined_LmP0Node 1
#define lm_lmx_module_private_1_typedef_id_a_LmP0Node 0x65d13d8ed2a28e86ULL
#define lm_lmx_module_private_1_typedef_id_b_LmP0Node 0x38e3b62fc61a92a7ULL
typedef struct LmP0Node LmP0Node;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmP0Node) || !defined(lm_lmx_module_private_1_typedef_id_b_LmP0Node) || lm_lmx_module_private_1_typedef_id_a_LmP0Node != 0x65d13d8ed2a28e86ULL || lm_lmx_module_private_1_typedef_id_b_LmP0Node != 0x38e3b62fc61a92a7ULL
#error "Lingvamyxa conflicting typedef projection for LmP0Node"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_LmP0Field
#define lm_lmx_module_private_1_typedef_defined_LmP0Field 1
#define lm_lmx_module_private_1_typedef_id_a_LmP0Field 0x3c706f0dce7650f0ULL
#define lm_lmx_module_private_1_typedef_id_b_LmP0Field 0x57dad0631f0f3e29ULL
typedef struct LmP0Field LmP0Field;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmP0Field) || !defined(lm_lmx_module_private_1_typedef_id_b_LmP0Field) || lm_lmx_module_private_1_typedef_id_a_LmP0Field != 0x3c706f0dce7650f0ULL || lm_lmx_module_private_1_typedef_id_b_LmP0Field != 0x57dad0631f0f3e29ULL
#error "Lingvamyxa conflicting typedef projection for LmP0Field"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_LmP0Trailer
#define lm_lmx_module_private_1_typedef_defined_LmP0Trailer 1
#define lm_lmx_module_private_1_typedef_id_a_LmP0Trailer 0xec450340d436d084ULL
#define lm_lmx_module_private_1_typedef_id_b_LmP0Trailer 0x1afaddaf2279fc75ULL
typedef struct LmP0Trailer LmP0Trailer;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmP0Trailer) || !defined(lm_lmx_module_private_1_typedef_id_b_LmP0Trailer) || lm_lmx_module_private_1_typedef_id_a_LmP0Trailer != 0xec450340d436d084ULL || lm_lmx_module_private_1_typedef_id_b_LmP0Trailer != 0x1afaddaf2279fc75ULL
#error "Lingvamyxa conflicting typedef projection for LmP0Trailer"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_LmP0Document
#define lm_lmx_module_private_1_typedef_defined_LmP0Document 1
#define lm_lmx_module_private_1_typedef_id_a_LmP0Document 0x938784e99c22e6f8ULL
#define lm_lmx_module_private_1_typedef_id_b_LmP0Document 0xbec33c28c32b4af1ULL
typedef struct LmP0Document LmP0Document;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmP0Document) || !defined(lm_lmx_module_private_1_typedef_id_b_LmP0Document) || lm_lmx_module_private_1_typedef_id_a_LmP0Document != 0x938784e99c22e6f8ULL || lm_lmx_module_private_1_typedef_id_b_LmP0Document != 0xbec33c28c32b4af1ULL
#error "Lingvamyxa conflicting typedef projection for LmP0Document"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_LmRegistrySourceColumn
#define lm_lmx_module_private_1_typedef_defined_LmRegistrySourceColumn 1
#define lm_lmx_module_private_1_typedef_id_a_LmRegistrySourceColumn 0xa32fc3c9d43d5912ULL
#define lm_lmx_module_private_1_typedef_id_b_LmRegistrySourceColumn 0x63fd9c4e6e892bb3ULL
typedef struct LmRegistrySourceColumn LmRegistrySourceColumn;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmRegistrySourceColumn) || !defined(lm_lmx_module_private_1_typedef_id_b_LmRegistrySourceColumn) || lm_lmx_module_private_1_typedef_id_a_LmRegistrySourceColumn != 0xa32fc3c9d43d5912ULL || lm_lmx_module_private_1_typedef_id_b_LmRegistrySourceColumn != 0x63fd9c4e6e892bb3ULL
#error "Lingvamyxa conflicting typedef projection for LmRegistrySourceColumn"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_LmRegistrySourceLoader
#define lm_lmx_module_private_1_typedef_defined_LmRegistrySourceLoader 1
#define lm_lmx_module_private_1_typedef_id_a_LmRegistrySourceLoader 0xbc8982de0073e1a8ULL
#define lm_lmx_module_private_1_typedef_id_b_LmRegistrySourceLoader 0xd8ba5bcf9116f911ULL
typedef struct LmRegistrySourceLoader LmRegistrySourceLoader;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmRegistrySourceLoader) || !defined(lm_lmx_module_private_1_typedef_id_b_LmRegistrySourceLoader) || lm_lmx_module_private_1_typedef_id_a_LmRegistrySourceLoader != 0xbc8982de0073e1a8ULL || lm_lmx_module_private_1_typedef_id_b_LmRegistrySourceLoader != 0xd8ba5bcf9116f911ULL
#error "Lingvamyxa conflicting typedef projection for LmRegistrySourceLoader"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_LmTableColumnDescriptor
#define lm_lmx_module_private_1_typedef_defined_LmTableColumnDescriptor 1
#define lm_lmx_module_private_1_typedef_id_a_LmTableColumnDescriptor 0x95116a69540c0a28ULL
#define lm_lmx_module_private_1_typedef_id_b_LmTableColumnDescriptor 0x7b4e72dbee2794a1ULL
typedef struct LmTableColumnDescriptor LmTableColumnDescriptor;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmTableColumnDescriptor) || !defined(lm_lmx_module_private_1_typedef_id_b_LmTableColumnDescriptor) || lm_lmx_module_private_1_typedef_id_a_LmTableColumnDescriptor != 0x95116a69540c0a28ULL || lm_lmx_module_private_1_typedef_id_b_LmTableColumnDescriptor != 0x7b4e72dbee2794a1ULL
#error "Lingvamyxa conflicting typedef projection for LmTableColumnDescriptor"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_LmTableCell
#define lm_lmx_module_private_1_typedef_defined_LmTableCell 1
#define lm_lmx_module_private_1_typedef_id_a_LmTableCell 0x835df3c36d9b469cULL
#define lm_lmx_module_private_1_typedef_id_b_LmTableCell 0x5011abfbe83c7e45ULL
typedef struct LmTableCell LmTableCell;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmTableCell) || !defined(lm_lmx_module_private_1_typedef_id_b_LmTableCell) || lm_lmx_module_private_1_typedef_id_a_LmTableCell != 0x835df3c36d9b469cULL || lm_lmx_module_private_1_typedef_id_b_LmTableCell != 0x5011abfbe83c7e45ULL
#error "Lingvamyxa conflicting typedef projection for LmTableCell"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_LmTableRow
#define lm_lmx_module_private_1_typedef_defined_LmTableRow 1
#define lm_lmx_module_private_1_typedef_id_a_LmTableRow 0x2dff0d4404a01232ULL
#define lm_lmx_module_private_1_typedef_id_b_LmTableRow 0x2d9ba751e8374c03ULL
typedef struct LmTableRow LmTableRow;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmTableRow) || !defined(lm_lmx_module_private_1_typedef_id_b_LmTableRow) || lm_lmx_module_private_1_typedef_id_a_LmTableRow != 0x2dff0d4404a01232ULL || lm_lmx_module_private_1_typedef_id_b_LmTableRow != 0x2d9ba751e8374c03ULL
#error "Lingvamyxa conflicting typedef projection for LmTableRow"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_LmRegistryViewRow
#define lm_lmx_module_private_1_typedef_defined_LmRegistryViewRow 1
#define lm_lmx_module_private_1_typedef_id_a_LmRegistryViewRow 0x19214f6b9cd5f7fcULL
#define lm_lmx_module_private_1_typedef_id_b_LmRegistryViewRow 0x2c08662d7dc05f05ULL
typedef struct LmRegistryViewRow LmRegistryViewRow;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmRegistryViewRow) || !defined(lm_lmx_module_private_1_typedef_id_b_LmRegistryViewRow) || lm_lmx_module_private_1_typedef_id_a_LmRegistryViewRow != 0x19214f6b9cd5f7fcULL || lm_lmx_module_private_1_typedef_id_b_LmRegistryViewRow != 0x2c08662d7dc05f05ULL
#error "Lingvamyxa conflicting typedef projection for LmRegistryViewRow"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_LmTableDescriptor
#define lm_lmx_module_private_1_typedef_defined_LmTableDescriptor 1
#define lm_lmx_module_private_1_typedef_id_a_LmTableDescriptor 0xddbe1d712abaae38ULL
#define lm_lmx_module_private_1_typedef_id_b_LmTableDescriptor 0x00d2f0f0f077de01ULL
typedef struct LmTableDescriptor LmTableDescriptor;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmTableDescriptor) || !defined(lm_lmx_module_private_1_typedef_id_b_LmTableDescriptor) || lm_lmx_module_private_1_typedef_id_a_LmTableDescriptor != 0xddbe1d712abaae38ULL || lm_lmx_module_private_1_typedef_id_b_LmTableDescriptor != 0x00d2f0f0f077de01ULL
#error "Lingvamyxa conflicting typedef projection for LmTableDescriptor"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_LmRegistryView
#define lm_lmx_module_private_1_typedef_defined_LmRegistryView 1
#define lm_lmx_module_private_1_typedef_id_a_LmRegistryView 0x823913542ef61912ULL
#define lm_lmx_module_private_1_typedef_id_b_LmRegistryView 0xabec1d6f1a471913ULL
typedef struct LmRegistryView LmRegistryView;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmRegistryView) || !defined(lm_lmx_module_private_1_typedef_id_b_LmRegistryView) || lm_lmx_module_private_1_typedef_id_a_LmRegistryView != 0x823913542ef61912ULL || lm_lmx_module_private_1_typedef_id_b_LmRegistryView != 0xabec1d6f1a471913ULL
#error "Lingvamyxa conflicting typedef projection for LmRegistryView"
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
#ifndef lm_lmx_module_private_1_typedef_defined_LmP0NodeKind
#define lm_lmx_module_private_1_typedef_defined_LmP0NodeKind 1
#define lm_lmx_module_private_1_typedef_id_a_LmP0NodeKind 0x1d04aff0a5eb398bULL
#define lm_lmx_module_private_1_typedef_id_b_LmP0NodeKind 0x5e96f30ca10f9108ULL
typedef int LmP0NodeKind;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmP0NodeKind) || !defined(lm_lmx_module_private_1_typedef_id_b_LmP0NodeKind) || lm_lmx_module_private_1_typedef_id_a_LmP0NodeKind != 0x1d04aff0a5eb398bULL || lm_lmx_module_private_1_typedef_id_b_LmP0NodeKind != 0x5e96f30ca10f9108ULL
#error "Lingvamyxa conflicting typedef projection for LmP0NodeKind"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_LmP0FrameFlags
#define lm_lmx_module_private_1_typedef_defined_LmP0FrameFlags 1
#define lm_lmx_module_private_1_typedef_id_a_LmP0FrameFlags 0xe175a4f4618db837ULL
#define lm_lmx_module_private_1_typedef_id_b_LmP0FrameFlags 0xe17661ffbedd718aULL
typedef unsigned LmP0FrameFlags;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmP0FrameFlags) || !defined(lm_lmx_module_private_1_typedef_id_b_LmP0FrameFlags) || lm_lmx_module_private_1_typedef_id_a_LmP0FrameFlags != 0xe175a4f4618db837ULL || lm_lmx_module_private_1_typedef_id_b_LmP0FrameFlags != 0xe17661ffbedd718aULL
#error "Lingvamyxa conflicting typedef projection for LmP0FrameFlags"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_LmP0NodeFlags
#define lm_lmx_module_private_1_typedef_defined_LmP0NodeFlags 1
#define lm_lmx_module_private_1_typedef_id_a_LmP0NodeFlags 0xe03579aac09def90ULL
#define lm_lmx_module_private_1_typedef_id_b_LmP0NodeFlags 0xa4615392bb0d31d9ULL
typedef unsigned LmP0NodeFlags;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmP0NodeFlags) || !defined(lm_lmx_module_private_1_typedef_id_b_LmP0NodeFlags) || lm_lmx_module_private_1_typedef_id_a_LmP0NodeFlags != 0xe03579aac09def90ULL || lm_lmx_module_private_1_typedef_id_b_LmP0NodeFlags != 0xa4615392bb0d31d9ULL
#error "Lingvamyxa conflicting typedef projection for LmP0NodeFlags"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_LmP0TrailerFlags
#define lm_lmx_module_private_1_typedef_defined_LmP0TrailerFlags 1
#define lm_lmx_module_private_1_typedef_id_a_LmP0TrailerFlags 0xe8790f378efe9145ULL
#define lm_lmx_module_private_1_typedef_id_b_LmP0TrailerFlags 0xc62eb19a69bf38a8ULL
typedef unsigned LmP0TrailerFlags;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmP0TrailerFlags) || !defined(lm_lmx_module_private_1_typedef_id_b_LmP0TrailerFlags) || lm_lmx_module_private_1_typedef_id_a_LmP0TrailerFlags != 0xe8790f378efe9145ULL || lm_lmx_module_private_1_typedef_id_b_LmP0TrailerFlags != 0xc62eb19a69bf38a8ULL
#error "Lingvamyxa conflicting typedef projection for LmP0TrailerFlags"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_LmP0StreamEventKind
#define lm_lmx_module_private_1_typedef_defined_LmP0StreamEventKind 1
#define lm_lmx_module_private_1_typedef_id_a_LmP0StreamEventKind 0xc13ead69ebc8a04fULL
#define lm_lmx_module_private_1_typedef_id_b_LmP0StreamEventKind 0x88d537e66fa1235eULL
typedef int LmP0StreamEventKind;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmP0StreamEventKind) || !defined(lm_lmx_module_private_1_typedef_id_b_LmP0StreamEventKind) || lm_lmx_module_private_1_typedef_id_a_LmP0StreamEventKind != 0xc13ead69ebc8a04fULL || lm_lmx_module_private_1_typedef_id_b_LmP0StreamEventKind != 0x88d537e66fa1235eULL
#error "Lingvamyxa conflicting typedef projection for LmP0StreamEventKind"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_LmP0TrailerRole
#define lm_lmx_module_private_1_typedef_defined_LmP0TrailerRole 1
#define lm_lmx_module_private_1_typedef_id_a_LmP0TrailerRole 0x5ab7762f4926c0a6ULL
#define lm_lmx_module_private_1_typedef_id_b_LmP0TrailerRole 0x64aa764e346c7647ULL
typedef int LmP0TrailerRole;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmP0TrailerRole) || !defined(lm_lmx_module_private_1_typedef_id_b_LmP0TrailerRole) || lm_lmx_module_private_1_typedef_id_a_LmP0TrailerRole != 0x5ab7762f4926c0a6ULL || lm_lmx_module_private_1_typedef_id_b_LmP0TrailerRole != 0x64aa764e346c7647ULL
#error "Lingvamyxa conflicting typedef projection for LmP0TrailerRole"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_LmP0DashFenceStatus
#define lm_lmx_module_private_1_typedef_defined_LmP0DashFenceStatus 1
#define lm_lmx_module_private_1_typedef_id_a_LmP0DashFenceStatus 0x5851001e0927925eULL
#define lm_lmx_module_private_1_typedef_id_b_LmP0DashFenceStatus 0xbe652b32f0516a9fULL
typedef int LmP0DashFenceStatus;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmP0DashFenceStatus) || !defined(lm_lmx_module_private_1_typedef_id_b_LmP0DashFenceStatus) || lm_lmx_module_private_1_typedef_id_a_LmP0DashFenceStatus != 0x5851001e0927925eULL || lm_lmx_module_private_1_typedef_id_b_LmP0DashFenceStatus != 0xbe652b32f0516a9fULL
#error "Lingvamyxa conflicting typedef projection for LmP0DashFenceStatus"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_LmP0FieldParseFlags
#define lm_lmx_module_private_1_typedef_defined_LmP0FieldParseFlags 1
#define lm_lmx_module_private_1_typedef_id_a_LmP0FieldParseFlags 0x543e57ccdac83045ULL
#define lm_lmx_module_private_1_typedef_id_b_LmP0FieldParseFlags 0xcd568a0520fb25f6ULL
typedef unsigned LmP0FieldParseFlags;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmP0FieldParseFlags) || !defined(lm_lmx_module_private_1_typedef_id_b_LmP0FieldParseFlags) || lm_lmx_module_private_1_typedef_id_a_LmP0FieldParseFlags != 0x543e57ccdac83045ULL || lm_lmx_module_private_1_typedef_id_b_LmP0FieldParseFlags != 0xcd568a0520fb25f6ULL
#error "Lingvamyxa conflicting typedef projection for LmP0FieldParseFlags"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_LmP0FieldParseLoopContinuation
#define lm_lmx_module_private_1_typedef_defined_LmP0FieldParseLoopContinuation 1
#define lm_lmx_module_private_1_typedef_id_a_LmP0FieldParseLoopContinuation 0x0c13d33f8364e0f3ULL
#define lm_lmx_module_private_1_typedef_id_b_LmP0FieldParseLoopContinuation 0xfa4c010e8007e384ULL
typedef int LmP0FieldParseLoopContinuation;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmP0FieldParseLoopContinuation) || !defined(lm_lmx_module_private_1_typedef_id_b_LmP0FieldParseLoopContinuation) || lm_lmx_module_private_1_typedef_id_a_LmP0FieldParseLoopContinuation != 0x0c13d33f8364e0f3ULL || lm_lmx_module_private_1_typedef_id_b_LmP0FieldParseLoopContinuation != 0xfa4c010e8007e384ULL
#error "Lingvamyxa conflicting typedef projection for LmP0FieldParseLoopContinuation"
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
#define LM_P0_NODE_STRUCTURE 1
#define LM_P0_NODE_FRAME 2
#define LM_P0_NODE_ATOM 3
#define LM_P0_NODE_DISABLED 4
#define LM_P0_FRAME_COLON 1U
#define LM_P0_FRAME_COMPACT 2U
#define LM_P0_FRAME_INLINE_BODY 4U
#define LM_P0_FRAME_SEPARATOR_CLOSED 8U
#define LM_P0_NODE_INACTIVE 1U
#define LM_P0_NODE_MIX 2U
#define LM_P0_NODE_POSITIONAL_SKIP 4U
#define LM_P0_TRAILER_TAIL_CUTTER 1U
#define LM_P0_STREAM_EVENT_ITEM 1
#define LM_P0_STREAM_EVENT_DELIM 2
#define LM_P0_STREAM_EVENT_BLOCK_STRING 3
#define LM_P0_STREAM_EVENT_DISABLED_BLOCK 4
#define LM_P0_STREAM_EVENT_MIX 5
#define LM_P0_TRAILER_ROLE_NONE 0
#define LM_P0_TRAILER_ROLE_DASH_CUTTER 1
#define LM_P0_TRAILER_ROLE_END 2
#define LM_P0_TRAILER_ROLE_RETURN 3
#define LM_P0_TRAILER_ROLE_UNTIL 4
#define LM_P0_DASH_FENCE_NONE 0
#define LM_P0_DASH_FENCE_VALID 1
#define LM_P0_DASH_FENCE_TOO_LONG 2
#define LM_P0_DASH_FENCE_TRAILING_TEXT 3
#define LM_P0_FIELD_PARSE_STOP_ON_SEMICOLON 1U
#define LM_P0_FIELD_PARSE_STOP_ON_SOURCE_LEVEL 2U
#define LM_P0_FIELD_PARSE_REQUIRE_BOUNDED_SOURCE_LEVEL 4U
#define LM_P0_FIELD_PARSE_ALLOW_EMPTY_FIELDS 8U
#define LM_P0_FIELD_PARSE_ALLOW_HEADLESS_AFTER_SEPARATOR 16U
#define LM_P0_MAX_FENCE_LENGTH 80U
#define LM_P0_LAYOUT_DELIMITER_STACK_LIMIT 256U
#define LM_P0_FIELD_PARSE_LOOP_HEADLESS_GROUP 0
#define LM_P0_FIELD_PARSE_LOOP_COLON_FRAME_BODY 1
#define LM_P0_FIELD_PARSE_LOOP_BOUNDED_STRUCTURE 2
#define LM_P0_FIELD_PARSE_LOOP_COMPACT_FRAME_BODY 3
#define LM_P0_POSTPROCESS_NODE 1
#define LM_P0_POSTPROCESS_STRUCTURE 2
#define LM_P0_POSTPROCESS_TRAILER 3
#define LM_P0_POSTPROCESS_FRAME_WRAP 4
#define LM_P0_DUMP_NODE 1
#define LM_P0_DUMP_STRUCTURE 2
#define LM_P0_DUMP_TRAILER 3


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
#ifndef lm_lmx_module_private_1_typedef_defined_LmP0Text
#define lm_lmx_module_private_1_typedef_defined_LmP0Text 1
#define lm_lmx_module_private_1_typedef_id_a_LmP0Text 0xd63d738ac661665dULL
#define lm_lmx_module_private_1_typedef_id_b_LmP0Text 0x21d3a49ca50ca808ULL
typedef struct LmP0Text {
    const char *data;
    size_t length;
} LmP0Text;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmP0Text) || !defined(lm_lmx_module_private_1_typedef_id_b_LmP0Text) || lm_lmx_module_private_1_typedef_id_a_LmP0Text != 0xd63d738ac661665dULL || lm_lmx_module_private_1_typedef_id_b_LmP0Text != 0x21d3a49ca50ca808ULL
#error "Lingvamyxa conflicting typedef projection for LmP0Text"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_LmP0Span
#define lm_lmx_module_private_1_typedef_defined_LmP0Span 1
#define lm_lmx_module_private_1_typedef_id_a_LmP0Span 0xd9ffd728192a8257ULL
#define lm_lmx_module_private_1_typedef_id_b_LmP0Span 0x0a372f58d22dff48ULL
typedef struct LmP0Span {
    size_t line;
    size_t column;
    size_t offset;
    size_t length;
} LmP0Span;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmP0Span) || !defined(lm_lmx_module_private_1_typedef_id_b_LmP0Span) || lm_lmx_module_private_1_typedef_id_a_LmP0Span != 0xd9ffd728192a8257ULL || lm_lmx_module_private_1_typedef_id_b_LmP0Span != 0x0a372f58d22dff48ULL
#error "Lingvamyxa conflicting typedef projection for LmP0Span"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_LmP0Diagnostic
#define lm_lmx_module_private_1_typedef_defined_LmP0Diagnostic 1
#define lm_lmx_module_private_1_typedef_id_a_LmP0Diagnostic 0xb485573811a5ba87ULL
#define lm_lmx_module_private_1_typedef_id_b_LmP0Diagnostic 0x9d0338c229428460ULL
typedef struct LmP0Diagnostic {
    int code;
    size_t line;
    size_t column;
    char message[256U];
} LmP0Diagnostic;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmP0Diagnostic) || !defined(lm_lmx_module_private_1_typedef_id_b_LmP0Diagnostic) || lm_lmx_module_private_1_typedef_id_a_LmP0Diagnostic != 0xb485573811a5ba87ULL || lm_lmx_module_private_1_typedef_id_b_LmP0Diagnostic != 0x9d0338c229428460ULL
#error "Lingvamyxa conflicting typedef projection for LmP0Diagnostic"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_LmP0Structure
#define lm_lmx_module_private_1_typedef_defined_LmP0Structure 1
#define lm_lmx_module_private_1_typedef_id_a_LmP0Structure 0xa8e18fda676aeb2dULL
#define lm_lmx_module_private_1_typedef_id_b_LmP0Structure 0x00a63744a2dc502aULL
typedef struct LmP0Structure {
    LmP0Field * first_field;
    LmP0Field * last_field;
    size_t field_count;
    LmP0Trailer * trailer;
} LmP0Structure;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmP0Structure) || !defined(lm_lmx_module_private_1_typedef_id_b_LmP0Structure) || lm_lmx_module_private_1_typedef_id_a_LmP0Structure != 0xa8e18fda676aeb2dULL || lm_lmx_module_private_1_typedef_id_b_LmP0Structure != 0x00a63744a2dc502aULL
#error "Lingvamyxa conflicting typedef projection for LmP0Structure"
#endif
#endif
struct LmP0Trailer {
    LmP0Text * spelling;
    unsigned flags;
    LmP0Structure * body;
};
#ifndef lm_lmx_module_private_1_typedef_defined_LmP0Frame
#define lm_lmx_module_private_1_typedef_defined_LmP0Frame 1
#define lm_lmx_module_private_1_typedef_id_a_LmP0Frame 0x2f36dcfe17b1bbc5ULL
#define lm_lmx_module_private_1_typedef_id_b_LmP0Frame 0x27340e3f3602fb92ULL
typedef struct LmP0Frame {
    LmP0Text * head;
    unsigned flags;
    LmP0Structure * body;
    LmP0Trailer * trailer;
} LmP0Frame;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmP0Frame) || !defined(lm_lmx_module_private_1_typedef_id_b_LmP0Frame) || lm_lmx_module_private_1_typedef_id_a_LmP0Frame != 0x2f36dcfe17b1bbc5ULL || lm_lmx_module_private_1_typedef_id_b_LmP0Frame != 0x27340e3f3602fb92ULL
#error "Lingvamyxa conflicting typedef projection for LmP0Frame"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_LmP0NodeAs
#define lm_lmx_module_private_1_typedef_defined_LmP0NodeAs 1
#define lm_lmx_module_private_1_typedef_id_a_LmP0NodeAs 0x4f1adbbf96ef759eULL
#define lm_lmx_module_private_1_typedef_id_b_LmP0NodeAs 0xdca9032e01d9954fULL
typedef struct LmP0NodeAs {
    LmP0Structure * structure;
    LmP0Frame * frame;
    LmP0Text * atom;
} LmP0NodeAs;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmP0NodeAs) || !defined(lm_lmx_module_private_1_typedef_id_b_LmP0NodeAs) || lm_lmx_module_private_1_typedef_id_a_LmP0NodeAs != 0x4f1adbbf96ef759eULL || lm_lmx_module_private_1_typedef_id_b_LmP0NodeAs != 0xdca9032e01d9954fULL
#error "Lingvamyxa conflicting typedef projection for LmP0NodeAs"
#endif
#endif
struct LmP0Node {
    LmP0NodeKind kind;
    unsigned flags;
    LmP0Span * span;
    LmP0NodeAs * as;
};
struct LmP0Field {
    LmP0Node * value;
    LmP0Field * next;
};
struct LmRegistrySourceColumn {
    const LmP0Text * name;
    const LmP0Text * descriptors[16U];
    size_t descriptor_count;
    const LmP0Text * type_name;
    size_t address_depth;
    size_t array_rank;
    int is_const;
};
struct LmRegistrySourceLoader {
    const char *error_prefix;
    int (*push_column_metadata)(struct LmMessageThread *lm_lmx_message_thread, void *context, const LmP0Text *table_name, LmRegistrySourceColumn **columns, size_t column_count);
    int (*push_table_row)(struct LmMessageThread *lm_lmx_message_thread, void *context, const LmP0Text *table_name, LmRegistrySourceColumn **columns, size_t column_count, const LmP0Node **cells);
    int (*join_table)(struct LmMessageThread *lm_lmx_message_thread, void *context, const LmP0Text *source_table, const LmP0Text *target_table);
    int (*formal_param_unwrap_index)(struct LmMessageThread *lm_lmx_message_thread, const LmRegistrySourceLoader *loader, void *context, const LmP0Frame *frame, size_t *out_index);
    int (*positional_name_index)(struct LmMessageThread *lm_lmx_message_thread, const LmRegistrySourceLoader *loader, void *context, const LmP0Frame *frame, size_t *out_index);
};
struct LmTableColumnDescriptor {
    char *name;
    char *descriptor;
    LmOwnPtrStack * descriptors;
    char *type_name;
    size_t index;
    size_t address_depth;
    size_t array_rank;
    int is_const;
};
struct LmTableCell {
    char *atom;
    char *value;
    const void *node;
    const void *source;
    int explicit_none;
};
struct LmTableRow {
    LmOwnPtrStack * cells;
    const LmTableRow * source;
    size_t local_sequence;
    int source_native;
};
struct LmRegistryViewRow {
    char *table;
    char *key;
    char *payload;
    const void *payload_node;
    const void *source;
    size_t local_sequence;
};
struct LmTableDescriptor {
    char *name;
    LmOwnPtrStack * columns;
    LmOwnPtrStack * rows;
    LmOwnPtrStack * source_rows;
    LmOwnPtrStack * materialized_rows;
};
struct LmRegistryView {
    const LmRegistryView * parent;
    LmOwnPtrStack * tables;
    LmOwnPtrStack * facts;
    size_t local_fact_count;
    LmOwnPtrStack * source_tables;
    LmOwnPtrStack * class_names;
    size_t mutation_generation;
};
#ifndef lm_lmx_module_private_1_typedef_defined_LmP0StreamEvent
#define lm_lmx_module_private_1_typedef_defined_LmP0StreamEvent 1
#define lm_lmx_module_private_1_typedef_id_a_LmP0StreamEvent 0xfd769ceb50750273ULL
#define lm_lmx_module_private_1_typedef_id_b_LmP0StreamEvent 0x9be9db4f27b6780cULL
typedef struct LmP0StreamEvent {
    LmP0StreamEventKind kind;
    unsigned node_flags;
    size_t level;
    const char *text;
    size_t text_length;
    size_t line;
    size_t column;
    size_t offset;
} LmP0StreamEvent;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmP0StreamEvent) || !defined(lm_lmx_module_private_1_typedef_id_b_LmP0StreamEvent) || lm_lmx_module_private_1_typedef_id_a_LmP0StreamEvent != 0xfd769ceb50750273ULL || lm_lmx_module_private_1_typedef_id_b_LmP0StreamEvent != 0x9be9db4f27b6780cULL
#error "Lingvamyxa conflicting typedef projection for LmP0StreamEvent"
#endif
#endif
struct LmP0Document {
    char *source;
    size_t source_length;
    LmP0Node * root;
    LmP0Diagnostic * diagnostic;
    LmOwnArena * source_owner;
    LmOwnArena * token_arena;
    LmOwnArena * tree_arena;
    LmOwnArena * diagnostic_arena;
    int owners_initialized;
    int frozen;
};
#ifndef lm_lmx_module_private_1_typedef_defined_LmP0PendingDelimiter
#define lm_lmx_module_private_1_typedef_defined_LmP0PendingDelimiter 1
#define lm_lmx_module_private_1_typedef_id_a_LmP0PendingDelimiter 0x699549bfbe22579aULL
#define lm_lmx_module_private_1_typedef_id_b_LmP0PendingDelimiter 0xb13b62fb51a8ea9bULL
typedef struct LmP0PendingDelimiter {
    int active;
    LmP0StreamEvent * event;
} LmP0PendingDelimiter;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmP0PendingDelimiter) || !defined(lm_lmx_module_private_1_typedef_id_b_LmP0PendingDelimiter) || lm_lmx_module_private_1_typedef_id_a_LmP0PendingDelimiter != 0x699549bfbe22579aULL || lm_lmx_module_private_1_typedef_id_b_LmP0PendingDelimiter != 0xb13b62fb51a8ea9bULL
#error "Lingvamyxa conflicting typedef projection for LmP0PendingDelimiter"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_LmP0PendingMix
#define lm_lmx_module_private_1_typedef_defined_LmP0PendingMix 1
#define lm_lmx_module_private_1_typedef_id_a_LmP0PendingMix 0x8255bc8056b3d5f4ULL
#define lm_lmx_module_private_1_typedef_id_b_LmP0PendingMix 0x305ae20718ed45edULL
typedef struct LmP0PendingMix {
    LmP0StreamEvent * events;
    size_t count;
    size_t capacity;
} LmP0PendingMix;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmP0PendingMix) || !defined(lm_lmx_module_private_1_typedef_id_b_LmP0PendingMix) || lm_lmx_module_private_1_typedef_id_a_LmP0PendingMix != 0x8255bc8056b3d5f4ULL || lm_lmx_module_private_1_typedef_id_b_LmP0PendingMix != 0x305ae20718ed45edULL
#error "Lingvamyxa conflicting typedef projection for LmP0PendingMix"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_LmP0IndentStack
#define lm_lmx_module_private_1_typedef_defined_LmP0IndentStack 1
#define lm_lmx_module_private_1_typedef_id_a_LmP0IndentStack 0x88e51ccef18321d3ULL
#define lm_lmx_module_private_1_typedef_id_b_LmP0IndentStack 0x30ffbd3854c4dd24ULL
typedef struct LmP0IndentStack {
    size_t *columns;
    size_t count;
    size_t capacity;
} LmP0IndentStack;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmP0IndentStack) || !defined(lm_lmx_module_private_1_typedef_id_b_LmP0IndentStack) || lm_lmx_module_private_1_typedef_id_a_LmP0IndentStack != 0x88e51ccef18321d3ULL || lm_lmx_module_private_1_typedef_id_b_LmP0IndentStack != 0x30ffbd3854c4dd24ULL
#error "Lingvamyxa conflicting typedef projection for LmP0IndentStack"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_LmP0DisabledState
#define lm_lmx_module_private_1_typedef_defined_LmP0DisabledState 1
#define lm_lmx_module_private_1_typedef_id_a_LmP0DisabledState 0x45f200122e91531cULL
#define lm_lmx_module_private_1_typedef_id_b_LmP0DisabledState 0x04ab474012b749e9ULL
typedef struct LmP0DisabledState {
    int body_started;
    int pending_item;
    size_t base_level;
    size_t top_level;
    size_t pending_level;
} LmP0DisabledState;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmP0DisabledState) || !defined(lm_lmx_module_private_1_typedef_id_b_LmP0DisabledState) || lm_lmx_module_private_1_typedef_id_a_LmP0DisabledState != 0x45f200122e91531cULL || lm_lmx_module_private_1_typedef_id_b_LmP0DisabledState != 0x04ab474012b749e9ULL
#error "Lingvamyxa conflicting typedef projection for LmP0DisabledState"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_LmP0Stack
#define lm_lmx_module_private_1_typedef_defined_LmP0Stack 1
#define lm_lmx_module_private_1_typedef_id_a_LmP0Stack 0xbe44d1bc66b6db10ULL
#define lm_lmx_module_private_1_typedef_id_b_LmP0Stack 0x20d07afe859fdc15ULL
typedef struct LmP0Stack {
    LmP0Structure * *parents;
    LmP0Node * *owners;
    unsigned char *hard;
    size_t capacity;
} LmP0Stack;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmP0Stack) || !defined(lm_lmx_module_private_1_typedef_id_b_LmP0Stack) || lm_lmx_module_private_1_typedef_id_a_LmP0Stack != 0xbe44d1bc66b6db10ULL || lm_lmx_module_private_1_typedef_id_b_LmP0Stack != 0x20d07afe859fdc15ULL
#error "Lingvamyxa conflicting typedef projection for LmP0Stack"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_LmP0Dump
#define lm_lmx_module_private_1_typedef_defined_LmP0Dump 1
#define lm_lmx_module_private_1_typedef_id_a_LmP0Dump 0x690a02684e65dc0cULL
#define lm_lmx_module_private_1_typedef_id_b_LmP0Dump 0x423960f1c9c11751ULL
typedef struct LmP0Dump {
    char *data;
    size_t length;
    size_t capacity;
    int failed;
} LmP0Dump;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmP0Dump) || !defined(lm_lmx_module_private_1_typedef_id_b_LmP0Dump) || lm_lmx_module_private_1_typedef_id_a_LmP0Dump != 0x690a02684e65dc0cULL || lm_lmx_module_private_1_typedef_id_b_LmP0Dump != 0x423960f1c9c11751ULL
#error "Lingvamyxa conflicting typedef projection for LmP0Dump"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_LmP0DumpFrame
#define lm_lmx_module_private_1_typedef_defined_LmP0DumpFrame 1
#define lm_lmx_module_private_1_typedef_id_a_LmP0DumpFrame 0xed20b8d3153c8f20ULL
#define lm_lmx_module_private_1_typedef_id_b_LmP0DumpFrame 0x3c6ba3645ea9996dULL
typedef struct LmP0DumpFrame {
    int phase;
    const LmP0Node * node;
    const LmP0Structure * structure;
    const LmP0Trailer * trailer;
    const LmP0Field * field;
    size_t indent;
} LmP0DumpFrame;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmP0DumpFrame) || !defined(lm_lmx_module_private_1_typedef_id_b_LmP0DumpFrame) || lm_lmx_module_private_1_typedef_id_a_LmP0DumpFrame != 0xed20b8d3153c8f20ULL || lm_lmx_module_private_1_typedef_id_b_LmP0DumpFrame != 0x3c6ba3645ea9996dULL
#error "Lingvamyxa conflicting typedef projection for LmP0DumpFrame"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_LmP0FieldParseLoopFrame
#define lm_lmx_module_private_1_typedef_defined_LmP0FieldParseLoopFrame 1
#define lm_lmx_module_private_1_typedef_id_a_LmP0FieldParseLoopFrame 0x3c04bc45aadbb849ULL
#define lm_lmx_module_private_1_typedef_id_b_LmP0FieldParseLoopFrame 0xec38e2975b3afdacULL
typedef struct LmP0FieldParseLoopFrame {
    LmP0IndentStack * indent_stack;
    LmP0Structure * structure;
    const char *text;
    size_t length;
    size_t line;
    size_t column;
    size_t offset;
    unsigned flags;
    size_t short_source_level;
    size_t layout_base_level;
    size_t i;
    size_t current_source_level;
    int allow_empty_fields;
    int expect_field;
    int headless_group_after_separator;
    int indent_stack_owned;
    LmP0FieldParseLoopContinuation continuation;
    LmP0Node * node;
    size_t start;
    size_t close_index;
} LmP0FieldParseLoopFrame;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmP0FieldParseLoopFrame) || !defined(lm_lmx_module_private_1_typedef_id_b_LmP0FieldParseLoopFrame) || lm_lmx_module_private_1_typedef_id_a_LmP0FieldParseLoopFrame != 0x3c04bc45aadbb849ULL || lm_lmx_module_private_1_typedef_id_b_LmP0FieldParseLoopFrame != 0xec38e2975b3afdacULL
#error "Lingvamyxa conflicting typedef projection for LmP0FieldParseLoopFrame"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_LmP0Registry
#define lm_lmx_module_private_1_typedef_defined_LmP0Registry 1
#define lm_lmx_module_private_1_typedef_id_a_LmP0Registry 0xd11ffac02365514eULL
#define lm_lmx_module_private_1_typedef_id_b_LmP0Registry 0xd0d10fef974e0687ULL
typedef struct LmP0Registry {
    LmRegistryView * view;
    int loaded;
    int loading;
} LmP0Registry;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmP0Registry) || !defined(lm_lmx_module_private_1_typedef_id_b_LmP0Registry) || lm_lmx_module_private_1_typedef_id_a_LmP0Registry != 0xd11ffac02365514eULL || lm_lmx_module_private_1_typedef_id_b_LmP0Registry != 0xd0d10fef974e0687ULL
#error "Lingvamyxa conflicting typedef projection for LmP0Registry"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_LmP0PostprocessFrame
#define lm_lmx_module_private_1_typedef_defined_LmP0PostprocessFrame 1
#define lm_lmx_module_private_1_typedef_id_a_LmP0PostprocessFrame 0x67b6ed598cdbeb13ULL
#define lm_lmx_module_private_1_typedef_id_b_LmP0PostprocessFrame 0x49eccb1939adc852ULL
typedef struct LmP0PostprocessFrame {
    int phase;
    LmP0Node * node;
    LmP0Structure * structure;
    LmP0Trailer * trailer;
    LmP0Field * field;
} LmP0PostprocessFrame;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmP0PostprocessFrame) || !defined(lm_lmx_module_private_1_typedef_id_b_LmP0PostprocessFrame) || lm_lmx_module_private_1_typedef_id_a_LmP0PostprocessFrame != 0x67b6ed598cdbeb13ULL || lm_lmx_module_private_1_typedef_id_b_LmP0PostprocessFrame != 0x49eccb1939adc852ULL
#error "Lingvamyxa conflicting typedef projection for LmP0PostprocessFrame"
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
#ifndef lm_lmx_module_private_1_typedef_defined_LmRegistrySourcePushTableRow
#define lm_lmx_module_private_1_typedef_defined_LmRegistrySourcePushTableRow 1
#define lm_lmx_module_private_1_typedef_id_a_LmRegistrySourcePushTableRow 0xa376a26ae91d70ebULL
#define lm_lmx_module_private_1_typedef_id_b_LmRegistrySourcePushTableRow 0x8b6702b05893a338ULL
typedef int (*LmRegistrySourcePushTableRow)(struct LmMessageThread *lm_lmx_message_thread, void *context, const LmP0Text *table_name, LmRegistrySourceColumn **columns, size_t column_count, const LmP0Node **cells);
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmRegistrySourcePushTableRow) || !defined(lm_lmx_module_private_1_typedef_id_b_LmRegistrySourcePushTableRow) || lm_lmx_module_private_1_typedef_id_a_LmRegistrySourcePushTableRow != 0xa376a26ae91d70ebULL || lm_lmx_module_private_1_typedef_id_b_LmRegistrySourcePushTableRow != 0x8b6702b05893a338ULL
#error "Lingvamyxa conflicting typedef projection for LmRegistrySourcePushTableRow"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_LmRegistrySourcePushColumnMetadata
#define lm_lmx_module_private_1_typedef_defined_LmRegistrySourcePushColumnMetadata 1
#define lm_lmx_module_private_1_typedef_id_a_LmRegistrySourcePushColumnMetadata 0x407ffac901b60465ULL
#define lm_lmx_module_private_1_typedef_id_b_LmRegistrySourcePushColumnMetadata 0x9732008bda932faeULL
typedef int (*LmRegistrySourcePushColumnMetadata)(struct LmMessageThread *lm_lmx_message_thread, void *context, const LmP0Text *table_name, LmRegistrySourceColumn **columns, size_t column_count);
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmRegistrySourcePushColumnMetadata) || !defined(lm_lmx_module_private_1_typedef_id_b_LmRegistrySourcePushColumnMetadata) || lm_lmx_module_private_1_typedef_id_a_LmRegistrySourcePushColumnMetadata != 0x407ffac901b60465ULL || lm_lmx_module_private_1_typedef_id_b_LmRegistrySourcePushColumnMetadata != 0x9732008bda932faeULL
#error "Lingvamyxa conflicting typedef projection for LmRegistrySourcePushColumnMetadata"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_LmRegistrySourceJoinTable
#define lm_lmx_module_private_1_typedef_defined_LmRegistrySourceJoinTable 1
#define lm_lmx_module_private_1_typedef_id_a_LmRegistrySourceJoinTable 0x89fa1694748a1529ULL
#define lm_lmx_module_private_1_typedef_id_b_LmRegistrySourceJoinTable 0x1966b230f990d48eULL
typedef int (*LmRegistrySourceJoinTable)(struct LmMessageThread *lm_lmx_message_thread, void *context, const LmP0Text *source_table, const LmP0Text *target_table);
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmRegistrySourceJoinTable) || !defined(lm_lmx_module_private_1_typedef_id_b_LmRegistrySourceJoinTable) || lm_lmx_module_private_1_typedef_id_a_LmRegistrySourceJoinTable != 0x89fa1694748a1529ULL || lm_lmx_module_private_1_typedef_id_b_LmRegistrySourceJoinTable != 0x1966b230f990d48eULL
#error "Lingvamyxa conflicting typedef projection for LmRegistrySourceJoinTable"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_LmRegistrySourceFrameIndexRule
#define lm_lmx_module_private_1_typedef_defined_LmRegistrySourceFrameIndexRule 1
#define lm_lmx_module_private_1_typedef_id_a_LmRegistrySourceFrameIndexRule 0x0233d8fbc9c0120fULL
#define lm_lmx_module_private_1_typedef_id_b_LmRegistrySourceFrameIndexRule 0xe81561fa08dccf34ULL
typedef int (*LmRegistrySourceFrameIndexRule)(struct LmMessageThread *lm_lmx_message_thread, const LmRegistrySourceLoader *loader, void *context, const LmP0Frame *frame, size_t *out_index);
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmRegistrySourceFrameIndexRule) || !defined(lm_lmx_module_private_1_typedef_id_b_LmRegistrySourceFrameIndexRule) || lm_lmx_module_private_1_typedef_id_a_LmRegistrySourceFrameIndexRule != 0x0233d8fbc9c0120fULL || lm_lmx_module_private_1_typedef_id_b_LmRegistrySourceFrameIndexRule != 0xe81561fa08dccf34ULL
#error "Lingvamyxa conflicting typedef projection for LmRegistrySourceFrameIndexRule"
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
int (lm_p0_parse_string)(struct LmMessageThread *lm_lmx_message_thread, const char *source, LmP0Document **out_document);
int (lm_p0_parse_bytes)(struct LmMessageThread *lm_lmx_message_thread, const char *source, size_t source_length, LmP0Document **out_document);
int (lm_p0_parse_file)(struct LmMessageThread *lm_lmx_message_thread, const char *path, LmP0Document **out_document);
void (lm_p0_document_destroy)(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document);
const LmP0Node * (lm_p0_document_root)(struct LmMessageThread *lm_lmx_message_thread, const LmP0Document *document);
LmP0Node * (lm_p0_document_mutable_root)(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document);
const LmP0Diagnostic * (lm_p0_document_diagnostic)(struct LmMessageThread *lm_lmx_message_thread, const LmP0Document *document);
const LmP0Structure * (lm_p0_node_structure)(struct LmMessageThread *lm_lmx_message_thread, const LmP0Node *node);
const LmP0Frame * (lm_p0_node_frame)(struct LmMessageThread *lm_lmx_message_thread, const LmP0Node *node);
const LmP0Text * (lm_p0_node_atom)(struct LmMessageThread *lm_lmx_message_thread, const LmP0Node *node);
const LmP0Trailer * (lm_p0_structure_trailer)(struct LmMessageThread *lm_lmx_message_thread, const LmP0Structure *structure);
const LmP0Text * (lm_p0_frame_head)(struct LmMessageThread *lm_lmx_message_thread, const LmP0Frame *frame);
const LmP0Structure * (lm_p0_frame_body)(struct LmMessageThread *lm_lmx_message_thread, const LmP0Frame *frame);
const LmP0Trailer * (lm_p0_frame_trailer)(struct LmMessageThread *lm_lmx_message_thread, const LmP0Frame *frame);
const LmP0Text * (lm_p0_trailer_spelling)(struct LmMessageThread *lm_lmx_message_thread, const LmP0Trailer *trailer);
const LmP0Structure * (lm_p0_trailer_body)(struct LmMessageThread *lm_lmx_message_thread, const LmP0Trailer *trailer);
const char * (lm_p0_node_kind_class_name)(struct LmMessageThread *lm_lmx_message_thread, LmP0NodeKind kind);
char * (lm_p0_dump_alloc)(struct LmMessageThread *lm_lmx_message_thread, const LmP0Document *document);
void (lm_p0_free)(struct LmMessageThread *lm_lmx_message_thread, void *ptr);
static int lm_registry_source_load_root(struct LmMessageThread *lm_lmx_message_thread, const LmRegistrySourceLoader *loader, void *context, const LmP0Node *root);










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


































































static int lm_registry_source_text_equals(struct LmMessageThread *lm_lmx_message_thread, const LmP0Text *text, const char *value);
static int lm_registry_source_text_slice_equals(struct LmMessageThread *lm_lmx_message_thread, const char *data, size_t length, const char *value);
static int lm_registry_source_text_slice_same(struct LmMessageThread *lm_lmx_message_thread, const char *left_data, size_t left_length, const char *right_data, size_t right_length);
static int lm_registry_source_node_is_ignored(struct LmMessageThread *lm_lmx_message_thread, const LmP0Node *node);
static const LmP0Structure * lm_registry_source_node_structure(struct LmMessageThread *lm_lmx_message_thread, const LmP0Node *node);
static const LmP0Frame * lm_registry_source_node_frame(struct LmMessageThread *lm_lmx_message_thread, const LmP0Node *node);
static const LmP0Text * lm_registry_source_node_atom(struct LmMessageThread *lm_lmx_message_thread, const LmP0Node *node);
static const LmP0Text * lm_registry_source_frame_head(struct LmMessageThread *lm_lmx_message_thread, const LmP0Frame *frame);
static const LmP0Structure * lm_registry_source_frame_body(struct LmMessageThread *lm_lmx_message_thread, const LmP0Frame *frame);
static const LmP0Text * lm_registry_source_trailer_spelling(struct LmMessageThread *lm_lmx_message_thread, const LmP0Trailer *trailer);
static const LmP0Structure * lm_registry_source_trailer_body(struct LmMessageThread *lm_lmx_message_thread, const LmP0Trailer *trailer);
static const char * lm_registry_source_error_prefix(struct LmMessageThread *lm_lmx_message_thread, const LmRegistrySourceLoader *loader);
static void lm_registry_source_error(struct LmMessageThread *lm_lmx_message_thread, const LmRegistrySourceLoader *loader, const char *message);
static const LmP0Field * lm_registry_source_nth_field(struct LmMessageThread *lm_lmx_message_thread, const LmP0Structure *structure, size_t index);
static int lm_registry_source_trailer_single_atom(struct LmMessageThread *lm_lmx_message_thread, const LmP0Trailer *trailer, const LmP0Text **out_text);
static int lm_registry_source_identifier_payload(struct LmMessageThread *lm_lmx_message_thread, const LmP0Text *atom, const char **out_data, size_t *out_length);
static int lm_registry_source_identifier_same(struct LmMessageThread *lm_lmx_message_thread, const LmP0Text *left, const LmP0Text *right);
static int lm_registry_source_text_all_char(struct LmMessageThread *lm_lmx_message_thread, const LmP0Text *text, char ch);
static int lm_registry_source_text_is_array_receiver_head(struct LmMessageThread *lm_lmx_message_thread, const LmP0Text *head);
static int lm_registry_source_structure_single_visible_node(struct LmMessageThread *lm_lmx_message_thread, const LmP0Structure *structure, const LmP0Node **out_node);
static int lm_registry_source_single_frame_node(struct LmMessageThread *lm_lmx_message_thread, const LmP0Node *node, const LmP0Frame **out_frame);
static int lm_registry_source_frame_formal_param_unwrap_index(struct LmMessageThread *lm_lmx_message_thread, const LmRegistrySourceLoader *loader, void *context, const LmP0Frame *frame, size_t *out_index);
static int lm_registry_source_frame_positional_name_index(struct LmMessageThread *lm_lmx_message_thread, const LmRegistrySourceLoader *loader, void *context, const LmP0Frame *frame, size_t *out_index);
static int lm_registry_source_formal_param_name(struct LmMessageThread *lm_lmx_message_thread, const LmRegistrySourceLoader *loader, void *context, const LmP0Node *node, const LmP0Text **out_name);
static int lm_registry_source_column_descriptor_from_param_node(struct LmMessageThread *lm_lmx_message_thread, const LmRegistrySourceLoader *loader, void *context, const LmP0Node *node, const LmP0Text **out_descriptor);
static size_t lm_registry_source_array_receiver_rank(struct LmMessageThread *lm_lmx_message_thread, const LmP0Text *head);
static int lm_registry_source_column_type_shape(struct LmMessageThread *lm_lmx_message_thread, const LmRegistrySourceLoader *loader, void *context, const LmP0Node *node, LmRegistrySourceColumn *out_column);
static int lm_registry_source_column_name_from_param_node(struct LmMessageThread *lm_lmx_message_thread, const LmRegistrySourceLoader *loader, void *context, const LmP0Node *node, LmRegistrySourceColumn *out_column);
static int lm_registry_source_structure_single_visible_frame(struct LmMessageThread *lm_lmx_message_thread, const LmP0Structure *structure, const LmP0Frame **out_frame);
static int lm_registry_source_frame_single_atom(struct LmMessageThread *lm_lmx_message_thread, const LmP0Frame *frame, const char *head, const LmP0Text **out_atom);
static int lm_registry_source_column_name(struct LmMessageThread *lm_lmx_message_thread, const LmRegistrySourceLoader *loader, void *context, const LmP0Field *field, LmRegistrySourceColumn *out_column);
static void lm_registry_source_columns_destroy(struct LmMessageThread *lm_lmx_message_thread, LmRegistrySourceColumn **columns, size_t count);
static int lm_registry_source_columns_from_frame(struct LmMessageThread *lm_lmx_message_thread, const LmRegistrySourceLoader *loader, void *context, const LmP0Frame *frame, LmRegistrySourceColumn **columns, size_t columns_capacity, size_t *out_count);
static int lm_registry_source_validate_named_trailer(struct LmMessageThread *lm_lmx_message_thread, const LmRegistrySourceLoader *loader, const LmP0Frame *frame, const LmP0Text *expected_name);
static int lm_registry_source_rows_from_frame(struct LmMessageThread *lm_lmx_message_thread, const LmRegistrySourceLoader *loader, void *context, const LmP0Frame *frame, const LmP0Text *table_name, LmRegistrySourceColumn **columns, size_t column_count);
static int lm_registry_source_body_first_after_optional_source(struct LmMessageThread *lm_lmx_message_thread, const LmRegistrySourceLoader *loader, const LmP0Structure *body, const char *marker_error, const LmP0Field **out_first);
static int lm_registry_source_table_from_frame(struct LmMessageThread *lm_lmx_message_thread, const LmRegistrySourceLoader *loader, void *context, const LmP0Frame *frame);
static int lm_registry_source_seen_table_add(struct LmMessageThread *lm_lmx_message_thread, LmOwnPtrStack *seen, const LmP0Text *table_name);
static int lm_registry_source_check_table_frame_unique(struct LmMessageThread *lm_lmx_message_thread, const LmRegistrySourceLoader *loader, const LmP0Frame *frame, LmOwnPtrStack *seen);
static const LmP0Field * lm_registry_source_next_present_field(struct LmMessageThread *lm_lmx_message_thread, const LmP0Field *field);
static int lm_registry_source_join_header(struct LmMessageThread *lm_lmx_message_thread, const LmP0Frame *frame, const LmP0Structure **out_sources, const LmP0Text **out_target, const LmP0Structure **out_body);
static int lm_registry_source_join_sources_into_target(struct LmMessageThread *lm_lmx_message_thread, const LmRegistrySourceLoader *loader, void *context, const LmP0Structure *sources, const LmP0Text *target_name);
static int lm_registry_source_join_from_frame(struct LmMessageThread *lm_lmx_message_thread, const LmRegistrySourceLoader *loader, void *context, const LmP0Frame *frame);
static int lm_registry_source_check_join_frame_unique(struct LmMessageThread *lm_lmx_message_thread, const LmRegistrySourceLoader *loader, const LmP0Frame *frame, LmOwnPtrStack *seen);
static int lm_registry_source_load_table_frame(struct LmMessageThread *lm_lmx_message_thread, const LmRegistrySourceLoader *loader, void *context, const LmP0Frame *frame, LmOwnPtrStack *seen);
static int lm_registry_source_load_join_frame(struct LmMessageThread *lm_lmx_message_thread, const LmRegistrySourceLoader *loader, void *context, const LmP0Frame *frame, LmOwnPtrStack *seen);
static int lm_registry_source_load_root(struct LmMessageThread *lm_lmx_message_thread, const LmRegistrySourceLoader *loader, void *context, const LmP0Node *root);


static int lm_registry_source_text_equals(struct LmMessageThread *lm_lmx_message_thread, const LmP0Text *text, const char *value) {
    (void)lm_lmx_message_thread;
    size_t length;
    if (text == 0 || value == 0) {
        return 0;
    }
    length = strlen(value);
    return text -> length == length && memcmp(text -> data, value, length) == 0;
}

static int lm_registry_source_text_slice_equals(struct LmMessageThread *lm_lmx_message_thread, const char *data, size_t length, const char *value) {
    (void)lm_lmx_message_thread;
    size_t value_length;
    if (data == 0 || value == 0) {
        return 0;
    }
    value_length = strlen(value);
    return length == value_length && memcmp(data, value, length) == 0;
}

static int lm_registry_source_text_slice_same(struct LmMessageThread *lm_lmx_message_thread, const char *left_data, size_t left_length, const char *right_data, size_t right_length) {
    (void)lm_lmx_message_thread;
    if (left_data == 0 || right_data == 0) {
        return 0;
    }
    if (left_length != right_length) {
        return 0;
    }
    if (left_length == 0U) {
        return 1;
    }
    return memcmp(left_data, right_data, left_length) == 0;
}

static int lm_registry_source_node_is_ignored(struct LmMessageThread *lm_lmx_message_thread, const LmP0Node *node) {
    (void)lm_lmx_message_thread;
    return node == 0 || (node -> flags & (LM_P0_NODE_INACTIVE | LM_P0_NODE_MIX)) != 0U;
}

static const LmP0Structure * lm_registry_source_node_structure(struct LmMessageThread *lm_lmx_message_thread, const LmP0Node *node) {
    (void)lm_lmx_message_thread;
    if (node == 0 || node -> kind != LM_P0_NODE_STRUCTURE) {
        return 0;
    }
    return node -> as -> structure;
}

static const LmP0Frame * lm_registry_source_node_frame(struct LmMessageThread *lm_lmx_message_thread, const LmP0Node *node) {
    (void)lm_lmx_message_thread;
    if (node == 0 || node -> kind != LM_P0_NODE_FRAME) {
        return 0;
    }
    return node -> as -> frame;
}

static const LmP0Text * lm_registry_source_node_atom(struct LmMessageThread *lm_lmx_message_thread, const LmP0Node *node) {
    (void)lm_lmx_message_thread;
    if (node == 0 || (node -> kind != LM_P0_NODE_ATOM && node -> kind != LM_P0_NODE_DISABLED)) {
        return 0;
    }
    return node -> as -> atom;
}

static const LmP0Text * lm_registry_source_frame_head(struct LmMessageThread *lm_lmx_message_thread, const LmP0Frame *frame) {
    (void)lm_lmx_message_thread;
    if (frame == 0) {
        return 0;
    }
    return frame -> head;
}

static const LmP0Structure * lm_registry_source_frame_body(struct LmMessageThread *lm_lmx_message_thread, const LmP0Frame *frame) {
    (void)lm_lmx_message_thread;
    if (frame == 0) {
        return 0;
    }
    return frame -> body;
}

static const LmP0Text * lm_registry_source_trailer_spelling(struct LmMessageThread *lm_lmx_message_thread, const LmP0Trailer *trailer) {
    (void)lm_lmx_message_thread;
    if (trailer == 0) {
        return 0;
    }
    return trailer -> spelling;
}

static const LmP0Structure * lm_registry_source_trailer_body(struct LmMessageThread *lm_lmx_message_thread, const LmP0Trailer *trailer) {
    (void)lm_lmx_message_thread;
    if (trailer == 0) {
        return 0;
    }
    return trailer -> body;
}

static const char * lm_registry_source_error_prefix(struct LmMessageThread *lm_lmx_message_thread, const LmRegistrySourceLoader *loader) {
    (void)lm_lmx_message_thread;
    if (loader != 0 && loader -> error_prefix != 0) {
        return loader -> error_prefix;
    }
    return "registry source";
}

static void lm_registry_source_error(struct LmMessageThread *lm_lmx_message_thread, const LmRegistrySourceLoader *loader, const char *message) {
    (void)lm_lmx_message_thread;
    fprintf(stderr, "%s registry error: %s\n", lm_registry_source_error_prefix(lm_lmx_message_thread, loader), message);
}

static const LmP0Field * lm_registry_source_nth_field(struct LmMessageThread *lm_lmx_message_thread, const LmP0Structure *structure, size_t index) {
    (void)lm_lmx_message_thread;
    const LmP0Field * field;
    size_t i;
    if (structure == 0) {
        return 0;
    }
    field = structure -> first_field;
    i = 0U;
    while (field != 0 && i < index) {
        field = field -> next;
        i = i + 1U;
    }
    return field;
}

static int lm_registry_source_trailer_single_atom(struct LmMessageThread *lm_lmx_message_thread, const LmP0Trailer *trailer, const LmP0Text **out_text) {
    (void)lm_lmx_message_thread;
    const LmP0Field * field;
    const LmP0Text * atom;
    if (trailer == 0 || out_text == 0) {
        return 0;
    }
    field = lm_registry_source_trailer_body(lm_lmx_message_thread, trailer) -> first_field;
    if (field == 0 || field -> next != 0 || field -> value == 0 || field -> value -> kind != LM_P0_NODE_ATOM) {
        return 0;
    }
    atom = lm_registry_source_node_atom(lm_lmx_message_thread, field -> value);
    if (atom == 0) {
        return 0;
    }
    *(out_text) = atom;
    return 1;
}

static int lm_registry_source_identifier_payload(struct LmMessageThread *lm_lmx_message_thread, const LmP0Text *atom, const char **out_data, size_t *out_length) {
    (void)lm_lmx_message_thread;
    if (atom == 0 || out_data == 0 || out_length == 0 || atom -> data == 0) {
        return 0;
    }
    if (atom -> length >= 2U && atom -> data[0] == '`' && atom -> data[atom -> length - 1U] == '`') {
        *(out_data) = atom -> data + 1U;
        *(out_length) = atom -> length - 2U;
        return 1;
    }
    *(out_data) = atom -> data;
    *(out_length) = atom -> length;
    return 1;
}

static int lm_registry_source_identifier_same(struct LmMessageThread *lm_lmx_message_thread, const LmP0Text *left, const LmP0Text *right) {
    (void)lm_lmx_message_thread;
    const char *left_data;
    const char *right_data;
    size_t left_length;
    size_t right_length;
    if (lm_registry_source_identifier_payload(lm_lmx_message_thread, left, &left_data, &left_length) == 0 || lm_registry_source_identifier_payload(lm_lmx_message_thread, right, &right_data, &right_length) == 0) {
        return 0;
    }
    return lm_registry_source_text_slice_same(lm_lmx_message_thread, left_data, left_length, right_data, right_length);
}

static int lm_registry_source_text_all_char(struct LmMessageThread *lm_lmx_message_thread, const LmP0Text *text, char ch) {
    (void)lm_lmx_message_thread;
    size_t i;
    if (text == 0 || text -> length == 0U) {
        return 0;
    }
    i = 0U;
    while (i < text -> length) {
        if (text -> data[i] != ch) {
            return 0;
        }
        i = i + 1U;
    }
    return 1;
}

static int lm_registry_source_text_is_array_receiver_head(struct LmMessageThread *lm_lmx_message_thread, const LmP0Text *head) {
    (void)lm_lmx_message_thread;
    size_t i;
    size_t count;
    if (head == 0 || head -> length == 0U) {
        return 0;
    }
    i = 0U;
    count = 0U;
    while (i < head -> length) {
        if (head -> data[i] != '[') {
            return 0;
        }
        i = i + 1U;
        while (i < head -> length && head -> data[i] != ']') {
            i = i + 1U;
        }
        if (i >= head -> length) {
            return 0;
        }
        i = i + 1U;
        count = count + 1U;
    }
    return count > 0U;
}

static int lm_registry_source_structure_single_visible_node(struct LmMessageThread *lm_lmx_message_thread, const LmP0Structure *structure, const LmP0Node **out_node) {
    (void)lm_lmx_message_thread;
    const LmP0Field * field;
    const LmP0Field * next_field;
    if (structure == 0 || out_node == 0) {
        return 0;
    }
    field = structure -> first_field;
    while (field != 0 && lm_registry_source_node_is_ignored(lm_lmx_message_thread, field -> value) != 0) {
        field = field -> next;
    }
    if (field == 0 || field -> value == 0) {
        return 0;
    }
    next_field = field -> next;
    while (next_field != 0 && lm_registry_source_node_is_ignored(lm_lmx_message_thread, next_field -> value) != 0) {
        next_field = next_field -> next;
    }
    if (next_field != 0) {
        return 0;
    }
    *(out_node) = field -> value;
    return 1;
}

static int lm_registry_source_single_frame_node(struct LmMessageThread *lm_lmx_message_thread, const LmP0Node *node, const LmP0Frame **out_frame) {
    (void)lm_lmx_message_thread;
    const LmP0Node * inner;
    if (node == 0 || out_frame == 0) {
        return 0;
    }
    if (node -> kind == LM_P0_NODE_FRAME) {
        *(out_frame) = node -> as -> frame;
        return 1;
    }
    if (node -> kind == LM_P0_NODE_STRUCTURE) {
        if (lm_registry_source_structure_single_visible_node(lm_lmx_message_thread, node -> as -> structure, &inner) == 0 || inner -> kind != LM_P0_NODE_FRAME) {
            return 0;
        }
        *(out_frame) = inner -> as -> frame;
        return 1;
    }
    return 0;
}

static int lm_registry_source_frame_formal_param_unwrap_index(struct LmMessageThread *lm_lmx_message_thread, const LmRegistrySourceLoader *loader, void *context, const LmP0Frame *frame, size_t *out_index) {
    (void)lm_lmx_message_thread;
    if (frame == 0 || out_index == 0) {
        return 0;
    }
    if (loader != 0 && loader -> formal_param_unwrap_index != 0 && (loader->formal_param_unwrap_index)(lm_lmx_message_thread, loader, context, frame, out_index) != 0) {
        return 1;
    }
    if (lm_registry_source_text_equals(lm_lmx_message_thread, lm_registry_source_frame_head(lm_lmx_message_thread, frame), "const")) {
        *(out_index) = 0U;
        return 1;
    }
    return 0;
}

static int lm_registry_source_frame_positional_name_index(struct LmMessageThread *lm_lmx_message_thread, const LmRegistrySourceLoader *loader, void *context, const LmP0Frame *frame, size_t *out_index) {
    (void)lm_lmx_message_thread;
    if (frame == 0 || out_index == 0) {
        return 0;
    }
    if (loader != 0 && loader -> positional_name_index != 0 && (loader->positional_name_index)(lm_lmx_message_thread, loader, context, frame, out_index) != 0) {
        return 1;
    }
    if (lm_registry_source_text_all_char(lm_lmx_message_thread, lm_registry_source_frame_head(lm_lmx_message_thread, frame), '@') != 0 || lm_registry_source_text_is_array_receiver_head(lm_lmx_message_thread, lm_registry_source_frame_head(lm_lmx_message_thread, frame)) != 0) {
        *(out_index) = 1U;
        return 1;
    }
    return 0;
}

static int lm_registry_source_formal_param_name(struct LmMessageThread *lm_lmx_message_thread, const LmRegistrySourceLoader *loader, void *context, const LmP0Node *node, const LmP0Text **out_name) {
    (void)lm_lmx_message_thread;
    const LmP0Node * current;
    const LmP0Frame * frame;
    const LmP0Field * field;
    const LmP0Field * name_field;
    size_t name_index;
    size_t unwrap_index;
    if (out_name == 0) {
        return 0;
    }
    current = node;
    while (current != 0) {
        if (current -> kind == LM_P0_NODE_STRUCTURE) {
            if (lm_registry_source_structure_single_visible_node(lm_lmx_message_thread, current -> as -> structure, &current) == 0) {
                return 0;
            }
            continue;
        }
        if (lm_registry_source_single_frame_node(lm_lmx_message_thread, current, &frame) == 0) {
            break;
        }
        if (lm_registry_source_frame_formal_param_unwrap_index(lm_lmx_message_thread, loader, context, frame, &unwrap_index) == 0) {
            break;
        }
        field = lm_registry_source_nth_field(lm_lmx_message_thread, frame -> body, unwrap_index);
        if (field == 0 || field -> value == 0) {
            return 0;
        }
        current = field -> value;
    }
    if (lm_registry_source_single_frame_node(lm_lmx_message_thread, current, &frame) == 0) {
        return 0;
    }
    if (lm_registry_source_frame_positional_name_index(lm_lmx_message_thread, loader, context, frame, &name_index) == 0) {
        name_index = 0U;
    }
    name_field = lm_registry_source_nth_field(lm_lmx_message_thread, frame -> body, name_index);
    if (name_field == 0 || name_field -> value == 0 || name_field -> value -> kind != LM_P0_NODE_ATOM) {
        return 0;
    }
    *(out_name) = name_field -> value -> as -> atom;
    return 1;
}

static int lm_registry_source_column_descriptor_from_param_node(struct LmMessageThread *lm_lmx_message_thread, const LmRegistrySourceLoader *loader, void *context, const LmP0Node *node, const LmP0Text **out_descriptor) {
    (void)lm_lmx_message_thread;
    const LmP0Node * current;
    const LmP0Frame * frame;
    const LmP0Field * type_field;
    size_t name_index;
    size_t unwrap_index;
    if (out_descriptor == 0) {
        return 0;
    }
    current = node;
    while (current != 0) {
        if (current -> kind == LM_P0_NODE_STRUCTURE) {
            if (lm_registry_source_structure_single_visible_node(lm_lmx_message_thread, current -> as -> structure, &current) == 0) {
                return 0;
            }
            continue;
        }
        if (lm_registry_source_single_frame_node(lm_lmx_message_thread, current, &frame) == 0) {
            break;
        }
        if (lm_registry_source_frame_formal_param_unwrap_index(lm_lmx_message_thread, loader, context, frame, &unwrap_index) == 0) {
            break;
        }
        type_field = lm_registry_source_nth_field(lm_lmx_message_thread, frame -> body, unwrap_index);
        if (type_field == 0 || type_field -> value == 0) {
            return 0;
        }
        current = type_field -> value;
    }
    if (lm_registry_source_single_frame_node(lm_lmx_message_thread, current, &frame) == 0) {
        return 0;
    }
    if (lm_registry_source_frame_positional_name_index(lm_lmx_message_thread, loader, context, frame, &name_index) == 0 || name_index == 0U) {
        *(out_descriptor) = lm_registry_source_frame_head(lm_lmx_message_thread, frame);
        return 1;
    }
    type_field = lm_registry_source_nth_field(lm_lmx_message_thread, frame -> body, 0U);
    if (type_field != 0 && type_field -> value != 0) {
        if (type_field -> value -> kind == LM_P0_NODE_ATOM) {
            *(out_descriptor) = type_field -> value -> as -> atom;
            return 1;
        }
        if (type_field -> value -> kind == LM_P0_NODE_FRAME) {
            *(out_descriptor) = type_field -> value -> as -> frame -> head;
            return 1;
        }
        if (type_field -> value -> kind == LM_P0_NODE_STRUCTURE && lm_registry_source_structure_single_visible_frame(lm_lmx_message_thread, type_field -> value -> as -> structure, &frame) != 0) {
            *(out_descriptor) = frame -> head;
            return 1;
        }
    }
    return 0;
}

static size_t lm_registry_source_array_receiver_rank(struct LmMessageThread *lm_lmx_message_thread, const LmP0Text *head) {
    (void)lm_lmx_message_thread;
    size_t i;
    size_t rank;
    if (head == 0) {
        return 0U;
    }
    i = 0U;
    rank = 0U;
    while (i < head -> length) {
        if (head -> data[i] != '[') {
            return 0U;
        }
        i = i + 1U;
        while (i < head -> length && head -> data[i] != ']') {
            i = i + 1U;
        }
        if (i >= head -> length) {
            return 0U;
        }
        i = i + 1U;
        rank = rank + 1U;
    }
    return rank;
}

static int lm_registry_source_column_type_shape(struct LmMessageThread *lm_lmx_message_thread, const LmRegistrySourceLoader *loader, void *context, const LmP0Node *node, LmRegistrySourceColumn *out_column) {
    (void)lm_lmx_message_thread;
    const LmP0Node * current;
    const LmP0Frame * frame;
    const LmP0Field * field;
    size_t index;
    size_t rank;
    if (node == 0 || out_column == 0) {
        return 0;
    }
    current = node;
    while (current != 0) {
        if (current -> kind == LM_P0_NODE_STRUCTURE) {
            if (lm_registry_source_structure_single_visible_node(lm_lmx_message_thread, current -> as -> structure, &current) == 0) {
                return 1;
            }
            continue;
        }
        if (current -> kind == LM_P0_NODE_ATOM) {
            out_column->type_name = current -> as -> atom;
            return 1;
        }
        if (current -> kind != LM_P0_NODE_FRAME) {
            return 1;
        }
        frame = current -> as -> frame;
        if (lm_registry_source_text_equals(lm_lmx_message_thread, frame -> head, "const") != 0) {
            out_column->is_const = 1;
            if (lm_registry_source_frame_formal_param_unwrap_index(lm_lmx_message_thread, loader, context, frame, &index) == 0) {
                index = 0U;
            }
            field = lm_registry_source_nth_field(lm_lmx_message_thread, frame -> body, index);
            if (field == 0 || field -> value == 0) {
                return 0;
            }
            current = field -> value;
            continue;
        }
        if (lm_registry_source_text_all_char(lm_lmx_message_thread, frame -> head, '@') != 0) {
            out_column->address_depth = out_column -> address_depth + frame -> head -> length;
            field = lm_registry_source_nth_field(lm_lmx_message_thread, frame -> body, 0U);
            if (field == 0 || field -> value == 0) {
                return 0;
            }
            current = field -> value;
            continue;
        }
        rank = lm_registry_source_array_receiver_rank(lm_lmx_message_thread, frame -> head);
        if (rank != 0U) {
            out_column->array_rank = out_column -> array_rank + rank;
            out_column->address_depth = out_column -> address_depth + 1U;
            field = lm_registry_source_nth_field(lm_lmx_message_thread, frame -> body, 0U);
            if (field == 0 || field -> value == 0) {
                return 0;
            }
            current = field -> value;
            continue;
        }
        out_column->type_name = frame -> head;
        return 1;
    }
    return 1;
}

static int lm_registry_source_column_name_from_param_node(struct LmMessageThread *lm_lmx_message_thread, const LmRegistrySourceLoader *loader, void *context, const LmP0Node *node, LmRegistrySourceColumn *out_column) {
    (void)lm_lmx_message_thread;
    const LmP0Text * atom;
    if (node == 0 || out_column == 0) {
        return 0;
    }
    if (lm_registry_source_formal_param_name(lm_lmx_message_thread, loader, context, node, &atom) == 0) {
        return -1;
    }
    out_column->name = atom;
    if (lm_registry_source_column_descriptor_from_param_node(lm_lmx_message_thread, loader, context, node, &atom) != 0) {
        out_column->descriptors[0] = atom;
        out_column->descriptor_count = 1U;
    }
    if (lm_registry_source_column_type_shape(lm_lmx_message_thread, loader, context, node, out_column) == 0) {
        return -1;
    }
    return 1;
}

static int lm_registry_source_structure_single_visible_frame(struct LmMessageThread *lm_lmx_message_thread, const LmP0Structure *structure, const LmP0Frame **out_frame) {
    (void)lm_lmx_message_thread;
    const LmP0Field * field;
    const LmP0Field * next_field;
    if (structure == 0 || out_frame == 0) {
        return 0;
    }
    field = structure -> first_field;
    while (field != 0 && lm_registry_source_node_is_ignored(lm_lmx_message_thread, field -> value) != 0) {
        field = field -> next;
    }
    if (field == 0 || field -> value == 0 || field -> value -> kind != LM_P0_NODE_FRAME) {
        return 0;
    }
    next_field = field -> next;
    while (next_field != 0 && lm_registry_source_node_is_ignored(lm_lmx_message_thread, next_field -> value) != 0) {
        next_field = next_field -> next;
    }
    if (next_field != 0) {
        return 0;
    }
    *(out_frame) = lm_registry_source_node_frame(lm_lmx_message_thread, field -> value);
    return 1;
}

static int lm_registry_source_frame_single_atom(struct LmMessageThread *lm_lmx_message_thread, const LmP0Frame *frame, const char *head, const LmP0Text **out_atom) {
    (void)lm_lmx_message_thread;
    const LmP0Field * field;
    const LmP0Text * frame_head;
    const LmP0Text * atom;
    if (frame == 0 || out_atom == 0) {
        return 0;
    }
    frame_head = lm_registry_source_frame_head(lm_lmx_message_thread, frame);
    if (head != 0 && (frame_head == 0 || lm_registry_source_text_equals(lm_lmx_message_thread, frame_head, head) == 0)) {
        return 0;
    }
    field = lm_registry_source_nth_field(lm_lmx_message_thread, lm_registry_source_frame_body(lm_lmx_message_thread, frame), 0U);
    if (field == 0 || field -> next != 0 || field -> value == 0 || field -> value -> kind != LM_P0_NODE_ATOM) {
        return -1;
    }
    atom = lm_registry_source_node_atom(lm_lmx_message_thread, field -> value);
    if (atom == 0) {
        return -1;
    }
    *(out_atom) = atom;
    return 1;
}

static int lm_registry_source_column_name(struct LmMessageThread *lm_lmx_message_thread, const LmRegistrySourceLoader *loader, void *context, const LmP0Field *field, LmRegistrySourceColumn *out_column) {
    (void)lm_lmx_message_thread;
    const LmP0Node * node;
    const LmP0Frame * node_frame;
    const LmP0Structure * node_structure;
    const LmP0Field * body_field;
    const LmP0Text * atom;
    size_t descriptor_count;
    int status;
    if (field == 0 || out_column == 0) {
        return 0;
    }
    memset(out_column, 0, sizeof(out_column[0]));
    node = field -> value;
    if (node == 0) {
        return -1;
    }
    if (node -> kind == LM_P0_NODE_ATOM) {
        atom = lm_registry_source_node_atom(lm_lmx_message_thread, node);
        if (atom == 0) {
            return -1;
        }
        out_column->name = atom;
        return 1;
    }
    if (node -> kind == LM_P0_NODE_FRAME) {
        return lm_registry_source_column_name_from_param_node(lm_lmx_message_thread, loader, context, node, out_column);
    }
    if (node -> kind != LM_P0_NODE_STRUCTURE) {
        return -1;
    }
    node_structure = lm_registry_source_node_structure(lm_lmx_message_thread, node);
    if (node_structure == 0) {
        return -1;
    }
    status = lm_registry_source_structure_single_visible_frame(lm_lmx_message_thread, node_structure, &node_frame);
    if (status != 0) {
        return lm_registry_source_column_name_from_param_node(lm_lmx_message_thread, loader, context, node, out_column);
    }
    body_field = node_structure -> first_field;
    while (body_field != 0 && lm_registry_source_node_is_ignored(lm_lmx_message_thread, body_field -> value) != 0) {
        body_field = body_field -> next;
    }
    if (body_field == 0 || body_field -> value == 0 || body_field -> value -> kind != LM_P0_NODE_ATOM) {
        return -1;
    }
    atom = lm_registry_source_node_atom(lm_lmx_message_thread, body_field -> value);
    if (atom == 0) {
        return -1;
    }
    out_column->name = atom;
    descriptor_count = 0U;
    body_field = body_field -> next;
    while (body_field != 0) {
        if (body_field -> value != 0 && lm_registry_source_node_is_ignored(lm_lmx_message_thread, body_field -> value) == 0) {
            if (body_field -> value -> kind != LM_P0_NODE_ATOM) {
                return -1;
            }
            if (descriptor_count >= sizeof(out_column -> descriptors) / sizeof(out_column -> descriptors[0])) {
                return -1;
            }
            atom = lm_registry_source_node_atom(lm_lmx_message_thread, body_field -> value);
            if (atom == 0) {
                return -1;
            }
            out_column->descriptors[descriptor_count] = atom;
            descriptor_count = descriptor_count + 1U;
        }
        body_field = body_field -> next;
    }
    out_column->descriptor_count = descriptor_count;
    return 1;
}

static void lm_registry_source_columns_destroy(struct LmMessageThread *lm_lmx_message_thread, LmRegistrySourceColumn **columns, size_t count) {
    (void)lm_lmx_message_thread;
    size_t index;
    if (columns == 0) {
        return;
    }
    index = 0U;
    while (index < count) {
        lm_own_delete(columns[index], 0);
        index = index + 1U;
    }
    lm_own_delete(columns, 0);
}

static int lm_registry_source_columns_from_frame(struct LmMessageThread *lm_lmx_message_thread, const LmRegistrySourceLoader *loader, void *context, const LmP0Frame *frame, LmRegistrySourceColumn **columns, size_t columns_capacity, size_t *out_count) {
    (void)lm_lmx_message_thread;
    const LmP0Field * field;
    LmRegistrySourceColumn * column;
    size_t count;
    int status;
    if (frame == 0 || columns == 0 || out_count == 0 || columns_capacity == 0U || lm_registry_source_text_equals(lm_lmx_message_thread, lm_registry_source_frame_head(lm_lmx_message_thread, frame), "columns") == 0) {
        return 0;
    }
    *(out_count) = 0U;
    count = 0U;
    field = lm_registry_source_frame_body(lm_lmx_message_thread, frame) -> first_field;
    while (field != 0) {
        if (field -> value != 0 && lm_registry_source_node_is_ignored(lm_lmx_message_thread, field -> value) == 0) {
            if (count >= columns_capacity) {
                lm_registry_source_error(lm_lmx_message_thread, loader, "table has too many columns");
                return -1;
            }
            column = lm_own_new_zero(sizeof(LmRegistrySourceColumn));
            if (column == 0) {
                lm_registry_source_error(lm_lmx_message_thread, loader, "out of memory while reading table columns");
                return -1;
            }
            status = lm_registry_source_column_name(lm_lmx_message_thread, loader, context, field, column);
            if (status <= 0) {
                lm_own_delete(column, 0);
                lm_registry_source_error(lm_lmx_message_thread, loader, "columns expects atoms or anonymous descriptor structures");
                return -1;
            }
            columns[count] = column;
            count = count + 1U;
            *(out_count) = count;
        }
        field = field -> next;
    }
    *(out_count) = count;
    return 1;
}

static int lm_registry_source_validate_named_trailer(struct LmMessageThread *lm_lmx_message_thread, const LmRegistrySourceLoader *loader, const LmP0Frame *frame, const LmP0Text *expected_name) {
    (void)lm_lmx_message_thread;
    const LmP0Text * actual;
    if (frame == 0 || frame -> trailer == 0) {
        return 0;
    }
    if (lm_registry_source_text_equals(lm_lmx_message_thread, lm_registry_source_trailer_spelling(lm_lmx_message_thread, frame -> trailer), "end") == 0) {
        return 0;
    }
    if (lm_registry_source_trailer_single_atom(lm_lmx_message_thread, frame -> trailer, &actual) == 0) {
        lm_registry_source_error(lm_lmx_message_thread, loader, "end trailer expects exactly one target name");
        return 1;
    }
    if (lm_registry_source_identifier_same(lm_lmx_message_thread, actual, expected_name) == 0 && lm_registry_source_identifier_same(lm_lmx_message_thread, actual, lm_registry_source_frame_head(lm_lmx_message_thread, frame)) == 0) {
        lm_registry_source_error(lm_lmx_message_thread, loader, "end trailer target does not match head/name");
        return 1;
    }
    return 0;
}

static int lm_registry_source_rows_from_frame(struct LmMessageThread *lm_lmx_message_thread, const LmRegistrySourceLoader *loader, void *context, const LmP0Frame *frame, const LmP0Text *table_name, LmRegistrySourceColumn **columns, size_t column_count) {
    (void)lm_lmx_message_thread;
    const LmP0Field * field;
    const LmP0Node * *row_cells;
    size_t field_index;
    size_t column_index;
    if (frame == 0 || lm_registry_source_text_equals(lm_lmx_message_thread, lm_registry_source_frame_head(lm_lmx_message_thread, frame), "rows") == 0) {
        return 0;
    }
    if (loader == 0 || loader -> push_table_row == 0) {
        lm_registry_source_error(lm_lmx_message_thread, loader, "table row consumer is not configured");
        return -1;
    }
    if (columns == 0 || column_count == 0U) {
        field = lm_registry_source_frame_body(lm_lmx_message_thread, frame) -> first_field;
        while (field != 0) {
            if (field -> value != 0 && lm_registry_source_node_is_ignored(lm_lmx_message_thread, field -> value) == 0) {
                lm_registry_source_error(lm_lmx_message_thread, loader, "rows require columns before cells");
                return -1;
            }
            field = field -> next;
        }
        return 1;
    }
    row_cells = lm_own_new_zero(column_count * sizeof(row_cells[0]));
    if (row_cells == 0) {
        lm_registry_source_error(lm_lmx_message_thread, loader, "out of memory while reading table rows");
        return -1;
    }
    field_index = 0U;
    field = lm_registry_source_frame_body(lm_lmx_message_thread, frame) -> first_field;
    while (field != 0) {
        if (field -> value != 0 && lm_registry_source_node_is_ignored(lm_lmx_message_thread, field -> value) == 0) {
            column_index = field_index % column_count;
            row_cells[column_index] = field -> value;
            if (column_index == 0U) {
                if (field -> value -> kind != LM_P0_NODE_ATOM || lm_registry_source_node_atom(lm_lmx_message_thread, field -> value) == 0) {
                    lm_registry_source_error(lm_lmx_message_thread, loader, "table rows currently expect atom cells in the key column");
                    lm_own_delete(row_cells, 0);
                    return -1;
                }
            }
            if (column_index + 1U == column_count && (loader->push_table_row)(lm_lmx_message_thread, context, table_name, columns, column_count, row_cells) != 0) {
                lm_own_delete(row_cells, 0);
                return -1;
            }
            field_index = field_index + 1U;
        }
        field = field -> next;
    }
    if ((field_index % column_count) != 0U) {
        if (table_name != 0 && table_name -> data != 0) {
            fprintf(stderr, "%s registry error: \"%.*s\" rows have %zu fields, not divisible by %zu columns (%zu leftover); quote operator characters or use explicit None\n", lm_registry_source_error_prefix(lm_lmx_message_thread, loader), (((int)table_name -> length)), table_name -> data, field_index, column_count, field_index % column_count);
        }
        else {
            fprintf(stderr, "%s registry error: rows have %zu fields, not divisible by %zu columns (%zu leftover); quote operator characters or use explicit None\n", lm_registry_source_error_prefix(lm_lmx_message_thread, loader), field_index, column_count, field_index % column_count);
        }
        lm_own_delete(row_cells, 0);
        return -1;
    }
    lm_own_delete(row_cells, 0);
    return 1;
}

static int lm_registry_source_body_first_after_optional_source(struct LmMessageThread *lm_lmx_message_thread, const LmRegistrySourceLoader *loader, const LmP0Structure *body, const char *marker_error, const LmP0Field **out_first) {
    (void)lm_lmx_message_thread;
    const LmP0Field * field;
    const LmP0Field * first;
    const LmP0Node * node;
    int source_seen;
    if (out_first == 0) {
        return 1;
    }
    out_first[0] = 0;
    first = 0;
    source_seen = 0;
    field = 0;
    if (body != 0) {
        field = body -> first_field;
    }
    while (field != 0) {
        node = field -> value;
        if (node != 0 && lm_registry_source_node_is_ignored(lm_lmx_message_thread, node) == 0) {
            if (node -> kind == LM_P0_NODE_ATOM && lm_registry_source_text_equals(lm_lmx_message_thread, lm_registry_source_node_atom(lm_lmx_message_thread, node), "source") != 0) {
                if (source_seen != 0 || first != 0) {
                    lm_registry_source_error(lm_lmx_message_thread, loader, marker_error);
                    return 1;
                }
                source_seen = 1;
            }
            else {
                if (first == 0) {
                    first = field;
                }
            }
        }
        field = field -> next;
    }
    out_first[0] = first;
    return 0;
}

static int lm_registry_source_table_from_frame(struct LmMessageThread *lm_lmx_message_thread, const LmRegistrySourceLoader *loader, void *context, const LmP0Frame *frame) {
    (void)lm_lmx_message_thread;
    const LmP0Field * field;
    const LmP0Node * node;
    const LmP0Frame * node_frame;
    const LmP0Structure * body;
    LmRegistrySourceColumn * *columns;
    const LmP0Text * table_name;
    size_t column_count;
    int have_name;
    int have_columns;
    int have_rows;
    int status;
    if (frame == 0 || lm_registry_source_text_equals(lm_lmx_message_thread, lm_registry_source_frame_head(lm_lmx_message_thread, frame), "table") == 0) {
        return 0;
    }
    body = lm_registry_source_frame_body(lm_lmx_message_thread, frame);
    field = 0;
    if (lm_registry_source_body_first_after_optional_source(lm_lmx_message_thread, loader, body, "table source marker must be the first present table body field and appear exactly once", &field) != 0) {
        return -1;
    }
    columns = lm_own_new_zero(128U * sizeof(columns[0]));
    if (columns == 0) {
        lm_registry_source_error(lm_lmx_message_thread, loader, "out of memory while reading table columns");
        return -1;
    }
    have_name = 0;
    have_columns = 0;
    have_rows = 0;
    column_count = 0U;
    table_name = 0;
    while (field != 0) {
        node = field -> value;
        if (node != 0 && lm_registry_source_node_is_ignored(lm_lmx_message_thread, node) == 0) {
            if (node -> kind != LM_P0_NODE_FRAME) {
                lm_registry_source_error(lm_lmx_message_thread, loader, "table body expects name/columns/rows frames");
                lm_registry_source_columns_destroy(lm_lmx_message_thread, columns, column_count);
                return -1;
            }
            node_frame = lm_registry_source_node_frame(lm_lmx_message_thread, node);
            status = lm_registry_source_frame_single_atom(lm_lmx_message_thread, node_frame, "name", &table_name);
            if (status < 0) {
                lm_registry_source_error(lm_lmx_message_thread, loader, "table name expects exactly one atom");
                lm_registry_source_columns_destroy(lm_lmx_message_thread, columns, column_count);
                return -1;
            }
            if (status > 0) {
                have_name = 1;
                field = field -> next;
                continue;
            }
            if (lm_registry_source_text_equals(lm_lmx_message_thread, lm_registry_source_frame_head(lm_lmx_message_thread, node_frame), "columns") != 0) {
                if (have_name == 0) {
                    lm_registry_source_error(lm_lmx_message_thread, loader, "table columns must appear after name");
                    lm_registry_source_columns_destroy(lm_lmx_message_thread, columns, column_count);
                    return -1;
                }
                status = lm_registry_source_columns_from_frame(lm_lmx_message_thread, loader, context, node_frame, columns, 128U, &column_count);
                if (status <= 0) {
                    lm_registry_source_columns_destroy(lm_lmx_message_thread, columns, column_count);
                    return -1;
                }
                if (loader != 0 && loader -> push_column_metadata != 0 && (loader->push_column_metadata)(lm_lmx_message_thread, context, table_name, columns, column_count) != 0) {
                    lm_registry_source_error(lm_lmx_message_thread, loader, "cannot store table column metadata");
                    lm_registry_source_columns_destroy(lm_lmx_message_thread, columns, column_count);
                    return -1;
                }
                have_columns = 1;
                field = field -> next;
                continue;
            }
            if (lm_registry_source_text_equals(lm_lmx_message_thread, lm_registry_source_frame_head(lm_lmx_message_thread, node_frame), "rows") != 0) {
                if (have_name == 0 || have_columns == 0) {
                    lm_registry_source_error(lm_lmx_message_thread, loader, "table rows must appear after name and columns");
                    lm_registry_source_columns_destroy(lm_lmx_message_thread, columns, column_count);
                    return -1;
                }
                status = lm_registry_source_rows_from_frame(lm_lmx_message_thread, loader, context, node_frame, table_name, columns, column_count);
                if (status <= 0) {
                    lm_registry_source_columns_destroy(lm_lmx_message_thread, columns, column_count);
                    return -1;
                }
                have_rows = 1;
                field = field -> next;
                continue;
            }
            lm_registry_source_error(lm_lmx_message_thread, loader, "table body expects name/columns/rows frames");
            lm_registry_source_columns_destroy(lm_lmx_message_thread, columns, column_count);
            return -1;
        }
        field = field -> next;
    }
    if (have_name == 0 || have_columns == 0 || have_rows == 0) {
        lm_registry_source_error(lm_lmx_message_thread, loader, "table requires name, columns and rows");
        lm_registry_source_columns_destroy(lm_lmx_message_thread, columns, column_count);
        return -1;
    }
    if (lm_registry_source_validate_named_trailer(lm_lmx_message_thread, loader, frame, table_name) != 0) {
        lm_registry_source_columns_destroy(lm_lmx_message_thread, columns, column_count);
        return -1;
    }
    lm_registry_source_columns_destroy(lm_lmx_message_thread, columns, column_count);
    return 1;
}

static int lm_registry_source_seen_table_add(struct LmMessageThread *lm_lmx_message_thread, LmOwnPtrStack *seen, const LmP0Text *table_name) {
    (void)lm_lmx_message_thread;
    const char *payload_data;
    char *name;
    size_t i;
    size_t payload_length;
    char *existing;
    if (seen == 0) {
        return -1;
    }
    if (lm_registry_source_identifier_payload(lm_lmx_message_thread, table_name, &payload_data, &payload_length) == 0) {
        return -1;
    }
    i = 0U;
    while (i < seen -> count) {
        existing = lm_own_ptr_stack_at(seen, i);
        if (existing != 0 && lm_registry_source_text_slice_equals(lm_lmx_message_thread, payload_data, payload_length, existing) != 0) {
            return 1;
        }
        i = i + 1U;
    }
    name = lm_own_new_zero(payload_length + 1U);
    if (name == 0) {
        return -1;
    }
    if (payload_length > 0U) {
        memcpy(name, payload_data, payload_length);
    }
    name[payload_length] = '\0';
    if (lm_own_ptr_stack_push(seen, name) != 0) {
        lm_own_delete(name, 0);
        return -1;
    }
    return 0;
}

static int lm_registry_source_check_table_frame_unique(struct LmMessageThread *lm_lmx_message_thread, const LmRegistrySourceLoader *loader, const LmP0Frame *frame, LmOwnPtrStack *seen) {
    (void)lm_lmx_message_thread;
    const LmP0Field * field;
    const LmP0Node * node;
    const LmP0Frame * node_frame;
    const LmP0Text * table_name;
    int status;
    if (frame == 0 || lm_registry_source_text_equals(lm_lmx_message_thread, lm_registry_source_frame_head(lm_lmx_message_thread, frame), "table") == 0) {
        return 0;
    }
    status = 0;
    table_name = 0;
    field = lm_registry_source_frame_body(lm_lmx_message_thread, frame) -> first_field;
    while (field != 0) {
        node = field -> value;
        if (node == 0 || lm_registry_source_node_is_ignored(lm_lmx_message_thread, node) != 0 || node -> kind != LM_P0_NODE_FRAME) {
            field = field -> next;
            continue;
        }
        node_frame = lm_registry_source_node_frame(lm_lmx_message_thread, node);
        status = lm_registry_source_frame_single_atom(lm_lmx_message_thread, node_frame, "name", &table_name);
        if (status < 0) {
            lm_registry_source_error(lm_lmx_message_thread, loader, "table name expects exactly one atom");
            return -1;
        }
        if (status > 0) {
            break;
        }
        field = field -> next;
    }
    if (status == 0) {
        return 0;
    }
    status = lm_registry_source_seen_table_add(lm_lmx_message_thread, seen, table_name);
    if (status < 0) {
        lm_registry_source_error(lm_lmx_message_thread, loader, "cannot record table name");
        return -1;
    }
    if (status > 0) {
        lm_registry_source_error(lm_lmx_message_thread, loader, "duplicate table in one registry source module");
        return -1;
    }
    return 0;
}

static const LmP0Field * lm_registry_source_next_present_field(struct LmMessageThread *lm_lmx_message_thread, const LmP0Field *field) {
    (void)lm_lmx_message_thread;
    while (field != 0 && (field -> value == 0 || lm_registry_source_node_is_ignored(lm_lmx_message_thread, field -> value) != 0)) {
        field = field -> next;
    }
    return field;
}

static int lm_registry_source_join_header(struct LmMessageThread *lm_lmx_message_thread, const LmP0Frame *frame, const LmP0Structure **out_sources, const LmP0Text **out_target, const LmP0Structure **out_body) {
    (void)lm_lmx_message_thread;
    const LmP0Field * field;
    const LmP0Structure * frame_body;
    const LmP0Structure * sources;
    const LmP0Text * target;
    const LmP0Structure * body;
    if (frame == 0 || out_sources == 0 || out_target == 0 || out_body == 0 || lm_registry_source_text_equals(lm_lmx_message_thread, lm_registry_source_frame_head(lm_lmx_message_thread, frame), "join") == 0) {
        return 0;
    }
    *(out_sources) = 0;
    *(out_target) = 0;
    *(out_body) = 0;
    frame_body = lm_registry_source_frame_body(lm_lmx_message_thread, frame);
    if (frame_body == 0) {
        return -1;
    }
    field = lm_registry_source_next_present_field(lm_lmx_message_thread, frame_body -> first_field);
    if (field == 0 || field -> value == 0 || field -> value -> kind != LM_P0_NODE_STRUCTURE) {
        return -1;
    }
    sources = lm_registry_source_node_structure(lm_lmx_message_thread, field -> value);
    if (sources == 0 || lm_registry_source_next_present_field(lm_lmx_message_thread, sources -> first_field) == 0) {
        return -1;
    }
    field = lm_registry_source_next_present_field(lm_lmx_message_thread, field -> next);
    if (field == 0 || field -> value == 0 || field -> value -> kind != LM_P0_NODE_ATOM) {
        return -1;
    }
    target = lm_registry_source_node_atom(lm_lmx_message_thread, field -> value);
    if (target == 0) {
        return -1;
    }
    field = lm_registry_source_next_present_field(lm_lmx_message_thread, field -> next);
    if (field == 0 || field -> value == 0 || field -> value -> kind != LM_P0_NODE_STRUCTURE) {
        return -1;
    }
    body = lm_registry_source_node_structure(lm_lmx_message_thread, field -> value);
    if (body == 0) {
        return -1;
    }
    *(out_sources) = sources;
    *(out_target) = target;
    *(out_body) = body;
    return 1;
}

static int lm_registry_source_join_sources_into_target(struct LmMessageThread *lm_lmx_message_thread, const LmRegistrySourceLoader *loader, void *context, const LmP0Structure *sources, const LmP0Text *target_name) {
    (void)lm_lmx_message_thread;
    const LmP0Field * field;
    const LmP0Node * node;
    const LmP0Text * source_name;
    if (loader == 0 || loader -> join_table == 0) {
        lm_registry_source_error(lm_lmx_message_thread, loader, "join consumer is not configured");
        return -1;
    }
    if (sources == 0 || target_name == 0) {
        return -1;
    }
    field = sources -> first_field;
    while (field != 0) {
        node = field -> value;
        if (node != 0 && lm_registry_source_node_is_ignored(lm_lmx_message_thread, node) == 0) {
            if (node -> kind != LM_P0_NODE_ATOM) {
                lm_registry_source_error(lm_lmx_message_thread, loader, "join source list currently expects table name atoms");
                return -1;
            }
            source_name = lm_registry_source_node_atom(lm_lmx_message_thread, node);
            if (source_name == 0 || (loader->join_table)(lm_lmx_message_thread, context, source_name, target_name) != 0) {
                return -1;
            }
        }
        field = field -> next;
    }
    return 0;
}

static int lm_registry_source_join_from_frame(struct LmMessageThread *lm_lmx_message_thread, const LmRegistrySourceLoader *loader, void *context, const LmP0Frame *frame) {
    (void)lm_lmx_message_thread;
    const LmP0Field * field;
    const LmP0Node * node;
    const LmP0Frame * node_frame;
    const LmP0Structure * sources;
    const LmP0Structure * body;
    LmRegistrySourceColumn * *columns;
    const LmP0Text * target_name;
    const LmP0Frame * rows_frame;
    size_t column_count;
    int have_columns;
    int have_rows;
    int status;
    if (frame == 0 || lm_registry_source_text_equals(lm_lmx_message_thread, lm_registry_source_frame_head(lm_lmx_message_thread, frame), "join") == 0) {
        return 0;
    }
    sources = 0;
    body = 0;
    target_name = 0;
    field = 0;
    status = lm_registry_source_join_header(lm_lmx_message_thread, frame, &sources, &target_name, &body);
    if (status <= 0) {
        if (status == 0) {
            return 0;
        }
        lm_registry_source_error(lm_lmx_message_thread, loader, "join expects (sourceTables...) targetName and a table fragment body");
        return -1;
    }
    if (lm_registry_source_body_first_after_optional_source(lm_lmx_message_thread, loader, body, "join source marker must be the first present join body field and appear exactly once", &field) != 0) {
        return -1;
    }
    columns = lm_own_new_zero(128U * sizeof(columns[0]));
    if (columns == 0) {
        lm_registry_source_error(lm_lmx_message_thread, loader, "out of memory while reading join columns");
        return -1;
    }
    have_columns = 0;
    have_rows = 0;
    rows_frame = 0;
    column_count = 0U;
    while (field != 0) {
        node = field -> value;
        if (node != 0 && lm_registry_source_node_is_ignored(lm_lmx_message_thread, node) == 0) {
            if (node -> kind != LM_P0_NODE_FRAME) {
                lm_registry_source_error(lm_lmx_message_thread, loader, "join body expects columns/rows frames");
                lm_registry_source_columns_destroy(lm_lmx_message_thread, columns, column_count);
                return -1;
            }
            node_frame = lm_registry_source_node_frame(lm_lmx_message_thread, node);
            if (lm_registry_source_text_equals(lm_lmx_message_thread, lm_registry_source_frame_head(lm_lmx_message_thread, node_frame), "columns") != 0) {
                if (have_columns != 0) {
                    lm_registry_source_error(lm_lmx_message_thread, loader, "join expects one columns frame");
                    lm_registry_source_columns_destroy(lm_lmx_message_thread, columns, column_count);
                    return -1;
                }
                status = lm_registry_source_columns_from_frame(lm_lmx_message_thread, loader, context, node_frame, columns, 128U, &column_count);
                if (status <= 0) {
                    lm_registry_source_columns_destroy(lm_lmx_message_thread, columns, column_count);
                    return -1;
                }
                if (loader != 0 && loader -> push_column_metadata != 0 && (loader->push_column_metadata)(lm_lmx_message_thread, context, target_name, columns, column_count) != 0) {
                    lm_registry_source_error(lm_lmx_message_thread, loader, "cannot store join target column metadata");
                    lm_registry_source_columns_destroy(lm_lmx_message_thread, columns, column_count);
                    return -1;
                }
                have_columns = 1;
                field = lm_registry_source_next_present_field(lm_lmx_message_thread, field -> next);
                continue;
            }
            if (lm_registry_source_text_equals(lm_lmx_message_thread, lm_registry_source_frame_head(lm_lmx_message_thread, node_frame), "rows") != 0) {
                if (have_rows != 0) {
                    lm_registry_source_error(lm_lmx_message_thread, loader, "join expects at most one rows frame");
                    lm_registry_source_columns_destroy(lm_lmx_message_thread, columns, column_count);
                    return -1;
                }
                rows_frame = node_frame;
                have_rows = 1;
                field = lm_registry_source_next_present_field(lm_lmx_message_thread, field -> next);
                continue;
            }
            lm_registry_source_error(lm_lmx_message_thread, loader, "join body expects columns/rows frames");
            lm_registry_source_columns_destroy(lm_lmx_message_thread, columns, column_count);
            return -1;
        }
        field = lm_registry_source_next_present_field(lm_lmx_message_thread, field -> next);
    }
    if (have_columns == 0) {
        lm_registry_source_error(lm_lmx_message_thread, loader, "join requires target columns");
        lm_registry_source_columns_destroy(lm_lmx_message_thread, columns, column_count);
        return -1;
    }
    if (lm_registry_source_join_sources_into_target(lm_lmx_message_thread, loader, context, sources, target_name) != 0) {
        lm_registry_source_columns_destroy(lm_lmx_message_thread, columns, column_count);
        return -1;
    }
    if (have_rows != 0) {
        status = lm_registry_source_rows_from_frame(lm_lmx_message_thread, loader, context, rows_frame, target_name, columns, column_count);
        if (status <= 0) {
            lm_registry_source_columns_destroy(lm_lmx_message_thread, columns, column_count);
            return -1;
        }
    }
    if (lm_registry_source_validate_named_trailer(lm_lmx_message_thread, loader, frame, target_name) != 0) {
        lm_registry_source_columns_destroy(lm_lmx_message_thread, columns, column_count);
        return -1;
    }
    lm_registry_source_columns_destroy(lm_lmx_message_thread, columns, column_count);
    return 1;
}

static int lm_registry_source_check_join_frame_unique(struct LmMessageThread *lm_lmx_message_thread, const LmRegistrySourceLoader *loader, const LmP0Frame *frame, LmOwnPtrStack *seen) {
    (void)lm_lmx_message_thread;
    const LmP0Structure * sources;
    const LmP0Structure * body;
    const LmP0Text * target_name;
    int status;
    if (frame == 0 || lm_registry_source_text_equals(lm_lmx_message_thread, lm_registry_source_frame_head(lm_lmx_message_thread, frame), "join") == 0) {
        return 0;
    }
    sources = 0;
    body = 0;
    target_name = 0;
    status = lm_registry_source_join_header(lm_lmx_message_thread, frame, &sources, &target_name, &body);
    if (status <= 0) {
        if (status < 0) {
            lm_registry_source_error(lm_lmx_message_thread, loader, "join expects (sourceTables...) targetName and a table fragment body");
            return -1;
        }
        return 0;
    }
    status = lm_registry_source_seen_table_add(lm_lmx_message_thread, seen, target_name);
    if (status < 0) {
        lm_registry_source_error(lm_lmx_message_thread, loader, "cannot record join target name");
        return -1;
    }
    if (status > 0) {
        lm_registry_source_error(lm_lmx_message_thread, loader, "duplicate table in one registry source module");
        return -1;
    }
    return 0;
}

static int lm_registry_source_load_table_frame(struct LmMessageThread *lm_lmx_message_thread, const LmRegistrySourceLoader *loader, void *context, const LmP0Frame *frame, LmOwnPtrStack *seen) {
    (void)lm_lmx_message_thread;
    int status;
    if (lm_registry_source_check_table_frame_unique(lm_lmx_message_thread, loader, frame, seen) != 0) {
        return 1;
    }
    status = lm_registry_source_table_from_frame(lm_lmx_message_thread, loader, context, frame);
    if (status <= 0) {
        if (status == 0) {
            lm_registry_source_error(lm_lmx_message_thread, loader, "table receiver expects table frame");
        }
        return 1;
    }
    return 0;
}

static int lm_registry_source_load_join_frame(struct LmMessageThread *lm_lmx_message_thread, const LmRegistrySourceLoader *loader, void *context, const LmP0Frame *frame, LmOwnPtrStack *seen) {
    (void)lm_lmx_message_thread;
    int status;
    if (lm_registry_source_check_join_frame_unique(lm_lmx_message_thread, loader, frame, seen) != 0) {
        return 1;
    }
    status = lm_registry_source_join_from_frame(lm_lmx_message_thread, loader, context, frame);
    if (status <= 0) {
        if (status == 0) {
            lm_registry_source_error(lm_lmx_message_thread, loader, "join receiver expects join frame");
        }
        return 1;
    }
    return 0;
}

static int lm_registry_source_load_root(struct LmMessageThread *lm_lmx_message_thread, const LmRegistrySourceLoader *loader, void *context, const LmP0Node *root) {
    (void)lm_lmx_message_thread;
    const LmP0Field * field;
    const LmP0Node * node;
    const LmP0Frame * frame;
    LmOwnPtrStack * seen;
    int loaded;
    int status;
    if (root == 0 || root -> kind != LM_P0_NODE_STRUCTURE) {
        lm_registry_source_error(lm_lmx_message_thread, loader, "root must be a Structure");
        return 1;
    }
    seen = lm_own_new_zero(sizeof(seen[0]));
    if (seen == 0) {
        lm_registry_source_error(lm_lmx_message_thread, loader, "cannot allocate table duplicate tracker");
        return 1;
    }
    lm_own_ptr_stack_init(seen, lm_own_delete_plain);
    loaded = 0;
    status = 0;
    field = lm_registry_source_node_structure(lm_lmx_message_thread, root) -> first_field;
    while (field != 0 && status == 0) {
        node = field -> value;
        if (lm_registry_source_node_is_ignored(lm_lmx_message_thread, node) != 0) {
            field = field -> next;
            continue;
        }
        if (node -> kind != LM_P0_NODE_FRAME) {
            lm_registry_source_error(lm_lmx_message_thread, loader, "root fields must be table or join frames");
            status = 1;
            field = field -> next;
            continue;
        }
        frame = lm_registry_source_node_frame(lm_lmx_message_thread, node);
        if (lm_registry_source_text_equals(lm_lmx_message_thread, lm_registry_source_frame_head(lm_lmx_message_thread, frame), "table") != 0) {
            status = lm_registry_source_load_table_frame(lm_lmx_message_thread, loader, context, frame, seen);
        }
        else {
            if (lm_registry_source_text_equals(lm_lmx_message_thread, lm_registry_source_frame_head(lm_lmx_message_thread, frame), "join") != 0) {
                status = lm_registry_source_load_join_frame(lm_lmx_message_thread, loader, context, frame, seen);
            }
            else {
                lm_registry_source_error(lm_lmx_message_thread, loader, "root fields must be table or join frames");
                status = 1;
            }
        }
        if (status == 0) {
            loaded = 1;
        }
        field = field -> next;
    }
    if (status == 0 && loaded == 0) {
        lm_registry_source_error(lm_lmx_message_thread, loader, "no rows loaded");
        status = 1;
    }
    lm_own_ptr_stack_destroy(seen);
    lm_own_delete(seen, 0);
    return status;
}





#ifndef lm_lmx_module_private_1_typedef_defined_lm_p0_parse_string
#define lm_lmx_module_private_1_typedef_defined_lm_p0_parse_string 1
#define lm_lmx_module_private_1_typedef_id_a_lm_p0_parse_string 0xc9e1874ed5fe7f93ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_p0_parse_string 0x7255c4d21d55d02cULL
typedef struct lm_p0_parse_stringClosure *lm_p0_parse_string;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_p0_parse_string) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_p0_parse_string) || lm_lmx_module_private_1_typedef_id_a_lm_p0_parse_string != 0xc9e1874ed5fe7f93ULL || lm_lmx_module_private_1_typedef_id_b_lm_p0_parse_string != 0x7255c4d21d55d02cULL
#error "Lingvamyxa conflicting typedef projection for lm_p0_parse_string"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_p0_parse_stringClosure
#define lm_lmx_module_private_1_typedef_defined_lm_p0_parse_stringClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_p0_parse_stringClosure 0xa5f47d3394e1d680ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_p0_parse_stringClosure 0x8d1812de23b6b9c5ULL
typedef struct lm_p0_parse_stringClosure lm_p0_parse_stringClosure;
struct lm_p0_parse_stringClosure {
    LmLmxStructure *descriptor;
    int (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, const char *, LmP0Document **);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_p0_parse_string = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_p0_parse_stringClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_p0_parse_stringClosure) || lm_lmx_module_private_1_typedef_id_a_lm_p0_parse_stringClosure != 0xa5f47d3394e1d680ULL || lm_lmx_module_private_1_typedef_id_b_lm_p0_parse_stringClosure != 0x8d1812de23b6b9c5ULL
#error "Lingvamyxa conflicting typedef projection for lm_p0_parse_stringClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_p0_parse_bytes
#define lm_lmx_module_private_1_typedef_defined_lm_p0_parse_bytes 1
#define lm_lmx_module_private_1_typedef_id_a_lm_p0_parse_bytes 0x85eefd722ed8bb99ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_p0_parse_bytes 0x4d45c3fdaebe3b3aULL
typedef struct lm_p0_parse_bytesClosure *lm_p0_parse_bytes;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_p0_parse_bytes) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_p0_parse_bytes) || lm_lmx_module_private_1_typedef_id_a_lm_p0_parse_bytes != 0x85eefd722ed8bb99ULL || lm_lmx_module_private_1_typedef_id_b_lm_p0_parse_bytes != 0x4d45c3fdaebe3b3aULL
#error "Lingvamyxa conflicting typedef projection for lm_p0_parse_bytes"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_p0_parse_bytesClosure
#define lm_lmx_module_private_1_typedef_defined_lm_p0_parse_bytesClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_p0_parse_bytesClosure 0x7aa09fdd8c4665d8ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_p0_parse_bytesClosure 0xc99034b48042d999ULL
typedef struct lm_p0_parse_bytesClosure lm_p0_parse_bytesClosure;
struct lm_p0_parse_bytesClosure {
    LmLmxStructure *descriptor;
    int (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, const char *, size_t, LmP0Document **);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_p0_parse_bytes = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_p0_parse_bytesClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_p0_parse_bytesClosure) || lm_lmx_module_private_1_typedef_id_a_lm_p0_parse_bytesClosure != 0x7aa09fdd8c4665d8ULL || lm_lmx_module_private_1_typedef_id_b_lm_p0_parse_bytesClosure != 0xc99034b48042d999ULL
#error "Lingvamyxa conflicting typedef projection for lm_p0_parse_bytesClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_p0_parse_file
#define lm_lmx_module_private_1_typedef_defined_lm_p0_parse_file 1
#define lm_lmx_module_private_1_typedef_id_a_lm_p0_parse_file 0x6a248aa6ca686e99ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_p0_parse_file 0xf418a91f06a8420eULL
typedef struct lm_p0_parse_fileClosure *lm_p0_parse_file;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_p0_parse_file) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_p0_parse_file) || lm_lmx_module_private_1_typedef_id_a_lm_p0_parse_file != 0x6a248aa6ca686e99ULL || lm_lmx_module_private_1_typedef_id_b_lm_p0_parse_file != 0xf418a91f06a8420eULL
#error "Lingvamyxa conflicting typedef projection for lm_p0_parse_file"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_p0_parse_fileClosure
#define lm_lmx_module_private_1_typedef_defined_lm_p0_parse_fileClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_p0_parse_fileClosure 0x2a40062c3348c1f2ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_p0_parse_fileClosure 0x43d18004f8b85213ULL
typedef struct lm_p0_parse_fileClosure lm_p0_parse_fileClosure;
struct lm_p0_parse_fileClosure {
    LmLmxStructure *descriptor;
    int (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, const char *, LmP0Document **);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_p0_parse_file = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_p0_parse_fileClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_p0_parse_fileClosure) || lm_lmx_module_private_1_typedef_id_a_lm_p0_parse_fileClosure != 0x2a40062c3348c1f2ULL || lm_lmx_module_private_1_typedef_id_b_lm_p0_parse_fileClosure != 0x43d18004f8b85213ULL
#error "Lingvamyxa conflicting typedef projection for lm_p0_parse_fileClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_p0_document_destroy
#define lm_lmx_module_private_1_typedef_defined_lm_p0_document_destroy 1
#define lm_lmx_module_private_1_typedef_id_a_lm_p0_document_destroy 0xbcd4b273124a3c59ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_p0_document_destroy 0x067aa088cb6e5dc2ULL
typedef struct lm_p0_document_destroyClosure *lm_p0_document_destroy;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_p0_document_destroy) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_p0_document_destroy) || lm_lmx_module_private_1_typedef_id_a_lm_p0_document_destroy != 0xbcd4b273124a3c59ULL || lm_lmx_module_private_1_typedef_id_b_lm_p0_document_destroy != 0x067aa088cb6e5dc2ULL
#error "Lingvamyxa conflicting typedef projection for lm_p0_document_destroy"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_p0_document_destroyClosure
#define lm_lmx_module_private_1_typedef_defined_lm_p0_document_destroyClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_p0_document_destroyClosure 0x352b8fc9f82fb5e8ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_p0_document_destroyClosure 0x9b0539aba2cbf839ULL
typedef struct lm_p0_document_destroyClosure lm_p0_document_destroyClosure;
struct lm_p0_document_destroyClosure {
    LmLmxStructure *descriptor;
    void (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, LmP0Document *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_p0_document_destroy = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_p0_document_destroyClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_p0_document_destroyClosure) || lm_lmx_module_private_1_typedef_id_a_lm_p0_document_destroyClosure != 0x352b8fc9f82fb5e8ULL || lm_lmx_module_private_1_typedef_id_b_lm_p0_document_destroyClosure != 0x9b0539aba2cbf839ULL
#error "Lingvamyxa conflicting typedef projection for lm_p0_document_destroyClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_p0_document_root
#define lm_lmx_module_private_1_typedef_defined_lm_p0_document_root 1
#define lm_lmx_module_private_1_typedef_id_a_lm_p0_document_root 0x3e87c354e8da9c39ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_p0_document_root 0xe5f0349dc62b03a2ULL
typedef struct lm_p0_document_rootClosure *lm_p0_document_root;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_p0_document_root) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_p0_document_root) || lm_lmx_module_private_1_typedef_id_a_lm_p0_document_root != 0x3e87c354e8da9c39ULL || lm_lmx_module_private_1_typedef_id_b_lm_p0_document_root != 0xe5f0349dc62b03a2ULL
#error "Lingvamyxa conflicting typedef projection for lm_p0_document_root"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_p0_document_rootClosure
#define lm_lmx_module_private_1_typedef_defined_lm_p0_document_rootClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_p0_document_rootClosure 0x832975dfe36e9ff9ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_p0_document_rootClosure 0xde04e2b993756be0ULL
typedef struct lm_p0_document_rootClosure lm_p0_document_rootClosure;
struct lm_p0_document_rootClosure {
    LmLmxStructure *descriptor;
    const LmP0Node * (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, const LmP0Document *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_p0_document_root = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_p0_document_rootClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_p0_document_rootClosure) || lm_lmx_module_private_1_typedef_id_a_lm_p0_document_rootClosure != 0x832975dfe36e9ff9ULL || lm_lmx_module_private_1_typedef_id_b_lm_p0_document_rootClosure != 0xde04e2b993756be0ULL
#error "Lingvamyxa conflicting typedef projection for lm_p0_document_rootClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_p0_document_mutable_root
#define lm_lmx_module_private_1_typedef_defined_lm_p0_document_mutable_root 1
#define lm_lmx_module_private_1_typedef_id_a_lm_p0_document_mutable_root 0xc0e8e10081db2ff1ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_p0_document_mutable_root 0xf8cc0dff77e315faULL
typedef struct lm_p0_document_mutable_rootClosure *lm_p0_document_mutable_root;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_p0_document_mutable_root) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_p0_document_mutable_root) || lm_lmx_module_private_1_typedef_id_a_lm_p0_document_mutable_root != 0xc0e8e10081db2ff1ULL || lm_lmx_module_private_1_typedef_id_b_lm_p0_document_mutable_root != 0xf8cc0dff77e315faULL
#error "Lingvamyxa conflicting typedef projection for lm_p0_document_mutable_root"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_p0_document_mutable_rootClosure
#define lm_lmx_module_private_1_typedef_defined_lm_p0_document_mutable_rootClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_p0_document_mutable_rootClosure 0xbb84fa22fa5d7d9dULL
#define lm_lmx_module_private_1_typedef_id_b_lm_p0_document_mutable_rootClosure 0x138f1329665d1bf4ULL
typedef struct lm_p0_document_mutable_rootClosure lm_p0_document_mutable_rootClosure;
struct lm_p0_document_mutable_rootClosure {
    LmLmxStructure *descriptor;
    LmP0Node * (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, LmP0Document *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_p0_document_mutable_root = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_p0_document_mutable_rootClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_p0_document_mutable_rootClosure) || lm_lmx_module_private_1_typedef_id_a_lm_p0_document_mutable_rootClosure != 0xbb84fa22fa5d7d9dULL || lm_lmx_module_private_1_typedef_id_b_lm_p0_document_mutable_rootClosure != 0x138f1329665d1bf4ULL
#error "Lingvamyxa conflicting typedef projection for lm_p0_document_mutable_rootClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_p0_document_diagnostic
#define lm_lmx_module_private_1_typedef_defined_lm_p0_document_diagnostic 1
#define lm_lmx_module_private_1_typedef_id_a_lm_p0_document_diagnostic 0x2de730244ed083a5ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_p0_document_diagnostic 0x5231bd2da722c726ULL
typedef struct lm_p0_document_diagnosticClosure *lm_p0_document_diagnostic;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_p0_document_diagnostic) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_p0_document_diagnostic) || lm_lmx_module_private_1_typedef_id_a_lm_p0_document_diagnostic != 0x2de730244ed083a5ULL || lm_lmx_module_private_1_typedef_id_b_lm_p0_document_diagnostic != 0x5231bd2da722c726ULL
#error "Lingvamyxa conflicting typedef projection for lm_p0_document_diagnostic"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_p0_document_diagnosticClosure
#define lm_lmx_module_private_1_typedef_defined_lm_p0_document_diagnosticClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_p0_document_diagnosticClosure 0x7e226fcf617b2646ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_p0_document_diagnosticClosure 0xe5583cd4325be7afULL
typedef struct lm_p0_document_diagnosticClosure lm_p0_document_diagnosticClosure;
struct lm_p0_document_diagnosticClosure {
    LmLmxStructure *descriptor;
    const LmP0Diagnostic * (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, const LmP0Document *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_p0_document_diagnostic = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_p0_document_diagnosticClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_p0_document_diagnosticClosure) || lm_lmx_module_private_1_typedef_id_a_lm_p0_document_diagnosticClosure != 0x7e226fcf617b2646ULL || lm_lmx_module_private_1_typedef_id_b_lm_p0_document_diagnosticClosure != 0xe5583cd4325be7afULL
#error "Lingvamyxa conflicting typedef projection for lm_p0_document_diagnosticClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_p0_node_structure
#define lm_lmx_module_private_1_typedef_defined_lm_p0_node_structure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_p0_node_structure 0x1732da245bb0c899ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_p0_node_structure 0xef7e57a667e3ecaaULL
typedef struct lm_p0_node_structureClosure *lm_p0_node_structure;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_p0_node_structure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_p0_node_structure) || lm_lmx_module_private_1_typedef_id_a_lm_p0_node_structure != 0x1732da245bb0c899ULL || lm_lmx_module_private_1_typedef_id_b_lm_p0_node_structure != 0xef7e57a667e3ecaaULL
#error "Lingvamyxa conflicting typedef projection for lm_p0_node_structure"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_p0_node_structureClosure
#define lm_lmx_module_private_1_typedef_defined_lm_p0_node_structureClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_p0_node_structureClosure 0x57b90b3424807a29ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_p0_node_structureClosure 0x6c86e207e903e61eULL
typedef struct lm_p0_node_structureClosure lm_p0_node_structureClosure;
struct lm_p0_node_structureClosure {
    LmLmxStructure *descriptor;
    const LmP0Structure * (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, const LmP0Node *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_p0_node_structure = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_p0_node_structureClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_p0_node_structureClosure) || lm_lmx_module_private_1_typedef_id_a_lm_p0_node_structureClosure != 0x57b90b3424807a29ULL || lm_lmx_module_private_1_typedef_id_b_lm_p0_node_structureClosure != 0x6c86e207e903e61eULL
#error "Lingvamyxa conflicting typedef projection for lm_p0_node_structureClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_p0_node_frame
#define lm_lmx_module_private_1_typedef_defined_lm_p0_node_frame 1
#define lm_lmx_module_private_1_typedef_id_a_lm_p0_node_frame 0x91ff609be2f58df9ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_p0_node_frame 0x8ea909fcbbf65beaULL
typedef struct lm_p0_node_frameClosure *lm_p0_node_frame;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_p0_node_frame) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_p0_node_frame) || lm_lmx_module_private_1_typedef_id_a_lm_p0_node_frame != 0x91ff609be2f58df9ULL || lm_lmx_module_private_1_typedef_id_b_lm_p0_node_frame != 0x8ea909fcbbf65beaULL
#error "Lingvamyxa conflicting typedef projection for lm_p0_node_frame"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_p0_node_frameClosure
#define lm_lmx_module_private_1_typedef_defined_lm_p0_node_frameClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_p0_node_frameClosure 0x035955c6f5639079ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_p0_node_frameClosure 0x95435e838cc21776ULL
typedef struct lm_p0_node_frameClosure lm_p0_node_frameClosure;
struct lm_p0_node_frameClosure {
    LmLmxStructure *descriptor;
    const LmP0Frame * (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, const LmP0Node *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_p0_node_frame = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_p0_node_frameClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_p0_node_frameClosure) || lm_lmx_module_private_1_typedef_id_a_lm_p0_node_frameClosure != 0x035955c6f5639079ULL || lm_lmx_module_private_1_typedef_id_b_lm_p0_node_frameClosure != 0x95435e838cc21776ULL
#error "Lingvamyxa conflicting typedef projection for lm_p0_node_frameClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_p0_node_atom
#define lm_lmx_module_private_1_typedef_defined_lm_p0_node_atom 1
#define lm_lmx_module_private_1_typedef_id_a_lm_p0_node_atom 0xb4f89d242b921ba1ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_p0_node_atom 0x61f5763bba9ff88aULL
typedef struct lm_p0_node_atomClosure *lm_p0_node_atom;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_p0_node_atom) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_p0_node_atom) || lm_lmx_module_private_1_typedef_id_a_lm_p0_node_atom != 0xb4f89d242b921ba1ULL || lm_lmx_module_private_1_typedef_id_b_lm_p0_node_atom != 0x61f5763bba9ff88aULL
#error "Lingvamyxa conflicting typedef projection for lm_p0_node_atom"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_p0_node_atomClosure
#define lm_lmx_module_private_1_typedef_defined_lm_p0_node_atomClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_p0_node_atomClosure 0x60a1d0fe2ccfdf29ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_p0_node_atomClosure 0xd1eff3b67f2510b8ULL
typedef struct lm_p0_node_atomClosure lm_p0_node_atomClosure;
struct lm_p0_node_atomClosure {
    LmLmxStructure *descriptor;
    const LmP0Text * (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, const LmP0Node *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_p0_node_atom = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_p0_node_atomClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_p0_node_atomClosure) || lm_lmx_module_private_1_typedef_id_a_lm_p0_node_atomClosure != 0x60a1d0fe2ccfdf29ULL || lm_lmx_module_private_1_typedef_id_b_lm_p0_node_atomClosure != 0xd1eff3b67f2510b8ULL
#error "Lingvamyxa conflicting typedef projection for lm_p0_node_atomClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_p0_structure_trailer
#define lm_lmx_module_private_1_typedef_defined_lm_p0_structure_trailer 1
#define lm_lmx_module_private_1_typedef_id_a_lm_p0_structure_trailer 0xfc2c8b7fd77d87d5ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_p0_structure_trailer 0xe8968d243183b756ULL
typedef struct lm_p0_structure_trailerClosure *lm_p0_structure_trailer;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_p0_structure_trailer) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_p0_structure_trailer) || lm_lmx_module_private_1_typedef_id_a_lm_p0_structure_trailer != 0xfc2c8b7fd77d87d5ULL || lm_lmx_module_private_1_typedef_id_b_lm_p0_structure_trailer != 0xe8968d243183b756ULL
#error "Lingvamyxa conflicting typedef projection for lm_p0_structure_trailer"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_p0_structure_trailerClosure
#define lm_lmx_module_private_1_typedef_defined_lm_p0_structure_trailerClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_p0_structure_trailerClosure 0x2c9111412d62c2feULL
#define lm_lmx_module_private_1_typedef_id_b_lm_p0_structure_trailerClosure 0xcf40587b033d6dc7ULL
typedef struct lm_p0_structure_trailerClosure lm_p0_structure_trailerClosure;
struct lm_p0_structure_trailerClosure {
    LmLmxStructure *descriptor;
    const LmP0Trailer * (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, const LmP0Structure *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_p0_structure_trailer = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_p0_structure_trailerClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_p0_structure_trailerClosure) || lm_lmx_module_private_1_typedef_id_a_lm_p0_structure_trailerClosure != 0x2c9111412d62c2feULL || lm_lmx_module_private_1_typedef_id_b_lm_p0_structure_trailerClosure != 0xcf40587b033d6dc7ULL
#error "Lingvamyxa conflicting typedef projection for lm_p0_structure_trailerClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_p0_frame_head
#define lm_lmx_module_private_1_typedef_defined_lm_p0_frame_head 1
#define lm_lmx_module_private_1_typedef_id_a_lm_p0_frame_head 0xd8e524b27be9c661ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_p0_frame_head 0x290ec5437c5f80f2ULL
typedef struct lm_p0_frame_headClosure *lm_p0_frame_head;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_p0_frame_head) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_p0_frame_head) || lm_lmx_module_private_1_typedef_id_a_lm_p0_frame_head != 0xd8e524b27be9c661ULL || lm_lmx_module_private_1_typedef_id_b_lm_p0_frame_head != 0x290ec5437c5f80f2ULL
#error "Lingvamyxa conflicting typedef projection for lm_p0_frame_head"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_p0_frame_headClosure
#define lm_lmx_module_private_1_typedef_defined_lm_p0_frame_headClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_p0_frame_headClosure 0xb005c804634f9094ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_p0_frame_headClosure 0xe8c79753c9c3bcd1ULL
typedef struct lm_p0_frame_headClosure lm_p0_frame_headClosure;
struct lm_p0_frame_headClosure {
    LmLmxStructure *descriptor;
    const LmP0Text * (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, const LmP0Frame *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_p0_frame_head = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_p0_frame_headClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_p0_frame_headClosure) || lm_lmx_module_private_1_typedef_id_a_lm_p0_frame_headClosure != 0xb005c804634f9094ULL || lm_lmx_module_private_1_typedef_id_b_lm_p0_frame_headClosure != 0xe8c79753c9c3bcd1ULL
#error "Lingvamyxa conflicting typedef projection for lm_p0_frame_headClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_p0_frame_body
#define lm_lmx_module_private_1_typedef_defined_lm_p0_frame_body 1
#define lm_lmx_module_private_1_typedef_id_a_lm_p0_frame_body 0x492e90e4ee1a0fe9ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_p0_frame_body 0x5dc60892a3227622ULL
typedef struct lm_p0_frame_bodyClosure *lm_p0_frame_body;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_p0_frame_body) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_p0_frame_body) || lm_lmx_module_private_1_typedef_id_a_lm_p0_frame_body != 0x492e90e4ee1a0fe9ULL || lm_lmx_module_private_1_typedef_id_b_lm_p0_frame_body != 0x5dc60892a3227622ULL
#error "Lingvamyxa conflicting typedef projection for lm_p0_frame_body"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_p0_frame_bodyClosure
#define lm_lmx_module_private_1_typedef_defined_lm_p0_frame_bodyClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_p0_frame_bodyClosure 0x4cbfe36a31c72990ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_p0_frame_bodyClosure 0xa5965d9e4d58f0b9ULL
typedef struct lm_p0_frame_bodyClosure lm_p0_frame_bodyClosure;
struct lm_p0_frame_bodyClosure {
    LmLmxStructure *descriptor;
    const LmP0Structure * (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, const LmP0Frame *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_p0_frame_body = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_p0_frame_bodyClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_p0_frame_bodyClosure) || lm_lmx_module_private_1_typedef_id_a_lm_p0_frame_bodyClosure != 0x4cbfe36a31c72990ULL || lm_lmx_module_private_1_typedef_id_b_lm_p0_frame_bodyClosure != 0xa5965d9e4d58f0b9ULL
#error "Lingvamyxa conflicting typedef projection for lm_p0_frame_bodyClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_p0_frame_trailer
#define lm_lmx_module_private_1_typedef_defined_lm_p0_frame_trailer 1
#define lm_lmx_module_private_1_typedef_id_a_lm_p0_frame_trailer 0x7cd1a241c637ef3dULL
#define lm_lmx_module_private_1_typedef_id_b_lm_p0_frame_trailer 0xc2b0bb996f19a46eULL
typedef struct lm_p0_frame_trailerClosure *lm_p0_frame_trailer;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_p0_frame_trailer) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_p0_frame_trailer) || lm_lmx_module_private_1_typedef_id_a_lm_p0_frame_trailer != 0x7cd1a241c637ef3dULL || lm_lmx_module_private_1_typedef_id_b_lm_p0_frame_trailer != 0xc2b0bb996f19a46eULL
#error "Lingvamyxa conflicting typedef projection for lm_p0_frame_trailer"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_p0_frame_trailerClosure
#define lm_lmx_module_private_1_typedef_defined_lm_p0_frame_trailerClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_p0_frame_trailerClosure 0x547267029ea1996eULL
#define lm_lmx_module_private_1_typedef_id_b_lm_p0_frame_trailerClosure 0x611eed165390e067ULL
typedef struct lm_p0_frame_trailerClosure lm_p0_frame_trailerClosure;
struct lm_p0_frame_trailerClosure {
    LmLmxStructure *descriptor;
    const LmP0Trailer * (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, const LmP0Frame *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_p0_frame_trailer = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_p0_frame_trailerClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_p0_frame_trailerClosure) || lm_lmx_module_private_1_typedef_id_a_lm_p0_frame_trailerClosure != 0x547267029ea1996eULL || lm_lmx_module_private_1_typedef_id_b_lm_p0_frame_trailerClosure != 0x611eed165390e067ULL
#error "Lingvamyxa conflicting typedef projection for lm_p0_frame_trailerClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_p0_trailer_spelling
#define lm_lmx_module_private_1_typedef_defined_lm_p0_trailer_spelling 1
#define lm_lmx_module_private_1_typedef_id_a_lm_p0_trailer_spelling 0xab195fce3f0468f1ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_p0_trailer_spelling 0xddce2a5f72c90ffeULL
typedef struct lm_p0_trailer_spellingClosure *lm_p0_trailer_spelling;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_p0_trailer_spelling) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_p0_trailer_spelling) || lm_lmx_module_private_1_typedef_id_a_lm_p0_trailer_spelling != 0xab195fce3f0468f1ULL || lm_lmx_module_private_1_typedef_id_b_lm_p0_trailer_spelling != 0xddce2a5f72c90ffeULL
#error "Lingvamyxa conflicting typedef projection for lm_p0_trailer_spelling"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_p0_trailer_spellingClosure
#define lm_lmx_module_private_1_typedef_defined_lm_p0_trailer_spellingClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_p0_trailer_spellingClosure 0xe0827a0bf49c7756ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_p0_trailer_spellingClosure 0x4ab7ad9b938dbf7fULL
typedef struct lm_p0_trailer_spellingClosure lm_p0_trailer_spellingClosure;
struct lm_p0_trailer_spellingClosure {
    LmLmxStructure *descriptor;
    const LmP0Text * (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, const LmP0Trailer *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_p0_trailer_spelling = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_p0_trailer_spellingClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_p0_trailer_spellingClosure) || lm_lmx_module_private_1_typedef_id_a_lm_p0_trailer_spellingClosure != 0xe0827a0bf49c7756ULL || lm_lmx_module_private_1_typedef_id_b_lm_p0_trailer_spellingClosure != 0x4ab7ad9b938dbf7fULL
#error "Lingvamyxa conflicting typedef projection for lm_p0_trailer_spellingClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_p0_trailer_body
#define lm_lmx_module_private_1_typedef_defined_lm_p0_trailer_body 1
#define lm_lmx_module_private_1_typedef_id_a_lm_p0_trailer_body 0x3bcbb69e337a3581ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_p0_trailer_body 0x1168efb6a08a1292ULL
typedef struct lm_p0_trailer_bodyClosure *lm_p0_trailer_body;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_p0_trailer_body) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_p0_trailer_body) || lm_lmx_module_private_1_typedef_id_a_lm_p0_trailer_body != 0x3bcbb69e337a3581ULL || lm_lmx_module_private_1_typedef_id_b_lm_p0_trailer_body != 0x1168efb6a08a1292ULL
#error "Lingvamyxa conflicting typedef projection for lm_p0_trailer_body"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_p0_trailer_bodyClosure
#define lm_lmx_module_private_1_typedef_defined_lm_p0_trailer_bodyClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_p0_trailer_bodyClosure 0xbe48e83332902496ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_p0_trailer_bodyClosure 0x3ad453f3479c4fefULL
typedef struct lm_p0_trailer_bodyClosure lm_p0_trailer_bodyClosure;
struct lm_p0_trailer_bodyClosure {
    LmLmxStructure *descriptor;
    const LmP0Structure * (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, const LmP0Trailer *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_p0_trailer_body = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_p0_trailer_bodyClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_p0_trailer_bodyClosure) || lm_lmx_module_private_1_typedef_id_a_lm_p0_trailer_bodyClosure != 0xbe48e83332902496ULL || lm_lmx_module_private_1_typedef_id_b_lm_p0_trailer_bodyClosure != 0x3ad453f3479c4fefULL
#error "Lingvamyxa conflicting typedef projection for lm_p0_trailer_bodyClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_p0_node_kind_class_name
#define lm_lmx_module_private_1_typedef_defined_lm_p0_node_kind_class_name 1
#define lm_lmx_module_private_1_typedef_id_a_lm_p0_node_kind_class_name 0xfc335cfa4e56aef1ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_p0_node_kind_class_name 0x1f67ba4b0c1226a2ULL
typedef struct lm_p0_node_kind_class_nameClosure *lm_p0_node_kind_class_name;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_p0_node_kind_class_name) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_p0_node_kind_class_name) || lm_lmx_module_private_1_typedef_id_a_lm_p0_node_kind_class_name != 0xfc335cfa4e56aef1ULL || lm_lmx_module_private_1_typedef_id_b_lm_p0_node_kind_class_name != 0x1f67ba4b0c1226a2ULL
#error "Lingvamyxa conflicting typedef projection for lm_p0_node_kind_class_name"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_p0_node_kind_class_nameClosure
#define lm_lmx_module_private_1_typedef_defined_lm_p0_node_kind_class_nameClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_p0_node_kind_class_nameClosure 0x7ba54ba407a8b2d6ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_p0_node_kind_class_nameClosure 0x8abc9ce6773ab357ULL
typedef struct lm_p0_node_kind_class_nameClosure lm_p0_node_kind_class_nameClosure;
struct lm_p0_node_kind_class_nameClosure {
    LmLmxStructure *descriptor;
    const char * (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, LmP0NodeKind);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_p0_node_kind_class_name = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_p0_node_kind_class_nameClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_p0_node_kind_class_nameClosure) || lm_lmx_module_private_1_typedef_id_a_lm_p0_node_kind_class_nameClosure != 0x7ba54ba407a8b2d6ULL || lm_lmx_module_private_1_typedef_id_b_lm_p0_node_kind_class_nameClosure != 0x8abc9ce6773ab357ULL
#error "Lingvamyxa conflicting typedef projection for lm_p0_node_kind_class_nameClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_p0_dump_alloc
#define lm_lmx_module_private_1_typedef_defined_lm_p0_dump_alloc 1
#define lm_lmx_module_private_1_typedef_id_a_lm_p0_dump_alloc 0xb11badc0cee22231ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_p0_dump_alloc 0x48e14efd31831e6aULL
typedef struct lm_p0_dump_allocClosure *lm_p0_dump_alloc;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_p0_dump_alloc) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_p0_dump_alloc) || lm_lmx_module_private_1_typedef_id_a_lm_p0_dump_alloc != 0xb11badc0cee22231ULL || lm_lmx_module_private_1_typedef_id_b_lm_p0_dump_alloc != 0x48e14efd31831e6aULL
#error "Lingvamyxa conflicting typedef projection for lm_p0_dump_alloc"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_p0_dump_allocClosure
#define lm_lmx_module_private_1_typedef_defined_lm_p0_dump_allocClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_p0_dump_allocClosure 0x87d845ebc1da1947ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_p0_dump_allocClosure 0x0c079d80e18a74eaULL
typedef struct lm_p0_dump_allocClosure lm_p0_dump_allocClosure;
struct lm_p0_dump_allocClosure {
    LmLmxStructure *descriptor;
    char * (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, const LmP0Document *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_p0_dump_alloc = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_p0_dump_allocClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_p0_dump_allocClosure) || lm_lmx_module_private_1_typedef_id_a_lm_p0_dump_allocClosure != 0x87d845ebc1da1947ULL || lm_lmx_module_private_1_typedef_id_b_lm_p0_dump_allocClosure != 0x0c079d80e18a74eaULL
#error "Lingvamyxa conflicting typedef projection for lm_p0_dump_allocClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_lm_p0_free
#define lm_lmx_module_private_1_typedef_defined_lm_p0_free 1
#define lm_lmx_module_private_1_typedef_id_a_lm_p0_free 0xc848ac29d8f52ff9ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_p0_free 0xa003a6193712dbeeULL
typedef struct lm_p0_freeClosure *lm_p0_free;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_p0_free) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_p0_free) || lm_lmx_module_private_1_typedef_id_a_lm_p0_free != 0xc848ac29d8f52ff9ULL || lm_lmx_module_private_1_typedef_id_b_lm_p0_free != 0xa003a6193712dbeeULL
#error "Lingvamyxa conflicting typedef projection for lm_p0_free"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_p0_freeClosure
#define lm_lmx_module_private_1_typedef_defined_lm_p0_freeClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_p0_freeClosure 0x8d03fb357c7da5f0ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_p0_freeClosure 0x0ec1ef4a23d0fb25ULL
typedef struct lm_p0_freeClosure lm_p0_freeClosure;
struct lm_p0_freeClosure {
    LmLmxStructure *descriptor;
    void (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, void *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_p0_free = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_p0_freeClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_p0_freeClosure) || lm_lmx_module_private_1_typedef_id_a_lm_p0_freeClosure != 0x8d03fb357c7da5f0ULL || lm_lmx_module_private_1_typedef_id_b_lm_p0_freeClosure != 0x0ec1ef4a23d0fb25ULL
#error "Lingvamyxa conflicting typedef projection for lm_p0_freeClosure"
#endif
#endif










































#ifndef lm_lmx_module_private_1_typedef_defined_lm_registry_source_load_root
#define lm_lmx_module_private_1_typedef_defined_lm_registry_source_load_root 1
#define lm_lmx_module_private_1_typedef_id_a_lm_registry_source_load_root 0x3fa84080af8ecf79ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_registry_source_load_root 0x2e0bea579df77f06ULL
typedef struct lm_registry_source_load_rootClosure *lm_registry_source_load_root;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_registry_source_load_root) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_registry_source_load_root) || lm_lmx_module_private_1_typedef_id_a_lm_registry_source_load_root != 0x3fa84080af8ecf79ULL || lm_lmx_module_private_1_typedef_id_b_lm_registry_source_load_root != 0x2e0bea579df77f06ULL
#error "Lingvamyxa conflicting typedef projection for lm_registry_source_load_root"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_lm_registry_source_load_rootClosure
#define lm_lmx_module_private_1_typedef_defined_lm_registry_source_load_rootClosure 1
#define lm_lmx_module_private_1_typedef_id_a_lm_registry_source_load_rootClosure 0x2700977ef22a9c96ULL
#define lm_lmx_module_private_1_typedef_id_b_lm_registry_source_load_rootClosure 0x747099d3d017edcfULL
typedef struct lm_registry_source_load_rootClosure lm_registry_source_load_rootClosure;
struct lm_registry_source_load_rootClosure {
    LmLmxStructure *descriptor;
    int (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, const LmRegistrySourceLoader *, void *, const LmP0Node *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_lm_registry_source_load_root = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_lm_registry_source_load_rootClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_lm_registry_source_load_rootClosure) || lm_lmx_module_private_1_typedef_id_a_lm_registry_source_load_rootClosure != 0x2700977ef22a9c96ULL || lm_lmx_module_private_1_typedef_id_b_lm_registry_source_load_rootClosure != 0x747099d3d017edcfULL
#error "Lingvamyxa conflicting typedef projection for lm_registry_source_load_rootClosure"
#endif
#endif










































#include <string.h>
static char * lm_table_descriptor_copy_slice(struct LmMessageThread *lm_lmx_message_thread, const char *data, size_t length);
static char * lm_table_descriptor_copy_cstr(struct LmMessageThread *lm_lmx_message_thread, const char *value);
static int lm_table_descriptor_slice_equals(struct LmMessageThread *lm_lmx_message_thread, const char *left, size_t left_length, const char *right, size_t right_length);
static int lm_table_descriptor_cstr_equals_slice(struct LmMessageThread *lm_lmx_message_thread, const char *left, const char *right, size_t right_length);
static LmOwnPtrStack * lm_table_descriptor_stack_new(struct LmMessageThread *lm_lmx_message_thread, LmOwnDelete delete_item);
static void lm_table_column_descriptor_delete_any(void *object);
static void lm_table_cell_delete_any(void *object);
static void lm_table_row_delete_any(void *object);
static void lm_registry_view_row_delete_any(void *object);
static void lm_table_descriptor_delete_any(void *object);
static LmTableColumnDescriptor * lm_table_column_descriptor_new_slice(struct LmMessageThread *lm_lmx_message_thread, const char *name, size_t name_length, const char *type_name, size_t type_name_length, size_t address_depth, size_t array_rank, int is_const);
static int lm_table_column_descriptor_add_descriptor_slice(struct LmMessageThread *lm_lmx_message_thread, LmTableColumnDescriptor *column, const char *descriptor, size_t descriptor_length);
static size_t lm_table_column_descriptor_descriptor_count(struct LmMessageThread *lm_lmx_message_thread, const LmTableColumnDescriptor *column);
static const char * lm_table_column_descriptor_descriptor_at(struct LmMessageThread *lm_lmx_message_thread, const LmTableColumnDescriptor *column, size_t index);
static int lm_table_column_descriptor_has_descriptor(struct LmMessageThread *lm_lmx_message_thread, const LmTableColumnDescriptor *column, const char *descriptor);
static int lm_table_descriptor_take_column(struct LmMessageThread *lm_lmx_message_thread, LmTableDescriptor *table, LmTableColumnDescriptor *column);
static int lm_table_descriptor_add_column_slices(struct LmMessageThread *lm_lmx_message_thread, LmTableDescriptor *table, const char *name, size_t name_length, const char *type_name, size_t type_name_length, const char **descriptor_data, const size_t *descriptor_lengths, size_t descriptor_count, size_t address_depth, size_t array_rank, int is_const);
static int lm_table_descriptor_add_column(struct LmMessageThread *lm_lmx_message_thread, LmTableDescriptor *table, const char *name, const char *descriptor);
static LmTableDescriptor * lm_table_descriptor_new_empty_slice(struct LmMessageThread *lm_lmx_message_thread, const char *name, size_t name_length);
static LmTableDescriptor * lm_table_descriptor_new_slice(struct LmMessageThread *lm_lmx_message_thread, const char *name, size_t name_length);
static size_t lm_table_descriptor_column_count(struct LmMessageThread *lm_lmx_message_thread, const LmTableDescriptor *table);
static const LmTableColumnDescriptor * lm_table_descriptor_column_at(struct LmMessageThread *lm_lmx_message_thread, const LmTableDescriptor *table, size_t index);
static size_t lm_table_descriptor_row_count(struct LmMessageThread *lm_lmx_message_thread, const LmTableDescriptor *table);
static const LmRegistryViewRow * lm_table_descriptor_row_at(struct LmMessageThread *lm_lmx_message_thread, const LmTableDescriptor *table, size_t index);
static LmTableCell * lm_table_cell_new_slice(struct LmMessageThread *lm_lmx_message_thread, const char *atom, size_t atom_length, const char *value, size_t value_length, const void *node, const void *source, int explicit_none);
static LmTableCell * lm_table_cell_new_cstr(struct LmMessageThread *lm_lmx_message_thread, const char *atom, const char *value, const void *node, const void *source, int explicit_none);
static LmTableRow * lm_table_row_new(struct LmMessageThread *lm_lmx_message_thread, const LmTableRow *source);
static int lm_table_row_take_cell(struct LmMessageThread *lm_lmx_message_thread, LmTableRow *row, LmTableCell *cell);
static int lm_table_row_take_cell_slice(struct LmMessageThread *lm_lmx_message_thread, LmTableRow *row, const char *atom, size_t atom_length, const char *value, size_t value_length, const void *node, const void *source, int explicit_none);
static size_t lm_table_row_cell_count(struct LmMessageThread *lm_lmx_message_thread, const LmTableRow *row);
static const LmTableCell * lm_table_row_cell_at(struct LmMessageThread *lm_lmx_message_thread, const LmTableRow *row, size_t index);
static int lm_table_descriptor_take_materialized_row(struct LmMessageThread *lm_lmx_message_thread, LmTableDescriptor *table, LmTableRow *row);
static size_t lm_table_descriptor_materialized_row_count(struct LmMessageThread *lm_lmx_message_thread, const LmTableDescriptor *table);
static const LmTableRow * lm_table_descriptor_materialized_row_at(struct LmMessageThread *lm_lmx_message_thread, const LmTableDescriptor *table, size_t index);
static int lm_table_descriptor_append_materialized_row_copy(struct LmMessageThread *lm_lmx_message_thread, LmTableDescriptor *table, const LmTableRow *source_row);
static int lm_table_descriptor_schema_same(struct LmMessageThread *lm_lmx_message_thread, const LmTableDescriptor *left, const LmTableDescriptor *right);
static int lm_table_descriptor_join_schema_compatible(struct LmMessageThread *lm_lmx_message_thread, const LmTableDescriptor *source, const LmTableDescriptor *target);
static LmRegistryView * lm_registry_view_new(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *parent);
static void lm_registry_view_note_mutation(struct LmMessageThread *lm_lmx_message_thread, LmRegistryView *view);
static void lm_registry_view_delete(struct LmMessageThread *lm_lmx_message_thread, LmRegistryView *view);
static int lm_registry_view_class_has_slice(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view, const char *name, size_t name_length);
static int lm_registry_view_class_has(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view, const char *name);
static int lm_registry_view_class_add(struct LmMessageThread *lm_lmx_message_thread, LmRegistryView *view, const char *name);
static size_t lm_registry_view_class_count(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view);
static const char * lm_registry_view_class_at(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view, size_t index);
static LmTableDescriptor * lm_registry_view_find_local_table_slice(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view, const char *table, size_t table_length);
static LmTableDescriptor * lm_registry_view_ensure_local_table_slice(struct LmMessageThread *lm_lmx_message_thread, LmRegistryView *view, const char *table, size_t table_length);
static LmTableDescriptor * lm_registry_view_find_local_source_table_slice(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view, const char *table, size_t table_length);
static LmTableDescriptor * lm_registry_view_find_source_table_slice(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view, const char *table, size_t table_length);
static int lm_registry_view_take_local_source_table(struct LmMessageThread *lm_lmx_message_thread, LmRegistryView *view, LmTableDescriptor *descriptor);
static size_t lm_registry_view_source_table_count(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view);
static const LmTableDescriptor * lm_registry_view_source_table_at(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view, size_t index);
static int lm_table_descriptor_source_path_column_matches(struct LmMessageThread *lm_lmx_message_thread, const LmTableDescriptor *descriptor, size_t column_index, const char *path, size_t path_length);
static int lm_registry_view_source_path_has_rows_slice_mode(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view, const char *path, size_t path_length, int projected_only, int *out_covered);
static int lm_registry_view_source_path_has_rows_slice(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view, const char *path, size_t path_length, int *out_covered);
static int lm_registry_view_source_path_has_rows(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view, const char *path, int *out_covered);
static int lm_registry_view_source_path_has_key_slice_mode(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view, const char *path, size_t path_length, const char *key, size_t key_length, int projected_only, int *out_covered);
static int lm_registry_view_source_path_has_key_slice(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view, const char *path, size_t path_length, const char *key, size_t key_length, int *out_covered);
static const LmTableCell * lm_registry_view_source_path_cell_at_slice_mode(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view, const char *path, size_t path_length, size_t index, int projected_only, const LmTableDescriptor **out_descriptor, size_t *out_column_index, const LmTableCell **out_key_cell, int *out_covered);
static const LmTableCell * lm_registry_view_source_path_cell_at_slice(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view, const char *path, size_t path_length, size_t index, const LmTableDescriptor **out_descriptor, size_t *out_column_index, const LmTableCell **out_key_cell, int *out_covered);
static const char * lm_registry_view_source_path_key_at_slice_mode(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view, const char *path, size_t path_length, size_t index, int projected_only, int *out_covered);
static const char * lm_registry_view_source_path_key_at_slice(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view, const char *path, size_t path_length, size_t index, int *out_covered);
static int lm_registry_view_append_materialized_rows_mode(struct LmMessageThread *lm_lmx_message_thread, LmRegistryView *view, LmTableDescriptor *target, const char *source_table, size_t source_table_length, int note_mutation, int require_compatible_source, size_t snapshot_source_count, const size_t *snapshot_row_counts);
static int lm_registry_view_append_materialized_rows(struct LmMessageThread *lm_lmx_message_thread, LmRegistryView *view, LmTableDescriptor *target, const char *source_table, size_t source_table_length);
static size_t lm_registry_view_fact_count(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view);
static const LmRegistryViewRow * lm_registry_view_fact_at(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view, size_t index);
static int lm_registry_view_push_relation(struct LmMessageThread *lm_lmx_message_thread, LmRegistryView *view, const char *table, const char *key, const char *payload, const void *payload_node, const void *source);
static const LmRegistryViewRow * lm_registry_view_lookup_local_exact_slice(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view, const char *key, size_t key_length, const char *table, size_t table_length);
static const LmRegistryViewRow * lm_registry_view_lookup_exact_slice(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view, const char *key, size_t key_length, const char *table, size_t table_length);
static const LmRegistryViewRow * lm_registry_view_lookup_exact(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view, const char *key, const char *table);
static const LmRegistryViewRow * lm_registry_view_lookup_default_slice(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view, const char *table, size_t table_length);
static const LmRegistryViewRow * lm_registry_view_lookup_default(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view, const char *table);
static const LmRegistryViewRow * lm_registry_view_lookup_slice(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view, const char *key, size_t key_length, const char *table, size_t table_length);
static const LmRegistryViewRow * lm_registry_view_lookup(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view, const char *key, const char *table);
static const LmRegistryViewRow * lm_registry_view_lookup_text_slice(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view, const char *key, size_t key_length, const char *table, size_t table_length);
static const LmRegistryViewRow * lm_registry_view_lookup_text(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view, const char *key, const char *table);
static size_t lm_registry_view_local_table_row_count_slice(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view, const char *table, size_t table_length);
static size_t lm_registry_view_table_row_count_slice(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view, const char *table, size_t table_length);
static size_t lm_registry_view_table_row_count(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view, const char *table);
static const LmRegistryViewRow * lm_registry_view_table_row_at_slice(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view, const char *table, size_t table_length, size_t index);
static const LmRegistryViewRow * lm_registry_view_table_row_at(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view, const char *table, size_t index);
static size_t lm_table_descriptor_matching_key_count_slice(struct LmMessageThread *lm_lmx_message_thread, const LmTableDescriptor *descriptor, const char *key, size_t key_length);
static const LmRegistryViewRow * lm_table_descriptor_matching_key_at_slice(struct LmMessageThread *lm_lmx_message_thread, const LmTableDescriptor *descriptor, const char *key, size_t key_length, size_t match_index);
static size_t lm_registry_view_matching_key_count_slice(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view, const char *table, size_t table_length, const char *key, size_t key_length);
static size_t lm_registry_view_matching_key_count(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view, const char *table, const char *key);
static const LmRegistryViewRow * lm_registry_view_matching_key_at_slice(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view, const char *table, size_t table_length, const char *key, size_t key_length, size_t match_index);
static const LmRegistryViewRow * lm_registry_view_matching_key_at(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view, const char *table, const char *key, size_t match_index);
static const LmOwnPtrStack * lm_registry_view_local_source_rows_slice(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view, const char *table, size_t table_length);
static int lm_registry_view_table_has_rows(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view, const char *table);


static int lm_p0_text_equals(struct LmMessageThread *lm_lmx_message_thread, const LmP0Text *text, const char *value);
static int lm_p0_identifier_payload(struct LmMessageThread *lm_lmx_message_thread, const LmP0Text *atom, LmP0Text *out_payload);
static int lm_p0_registry_identifier_value(struct LmMessageThread *lm_lmx_message_thread, const LmP0Text *atom, LmP0Text *out_payload);
static int lm_p0_registry_literal_value(struct LmMessageThread *lm_lmx_message_thread, const LmP0Text *atom, LmP0Text *out_payload);
static int lm_p0_registry_payload_is_null(struct LmMessageThread *lm_lmx_message_thread, const LmP0Text *atom);
static int lm_p0_is_horizontal_space(struct LmMessageThread *lm_lmx_message_thread, char value);
static int lm_p0_is_line_break(struct LmMessageThread *lm_lmx_message_thread, char value);
static size_t lm_p0_line_break_width_at(struct LmMessageThread *lm_lmx_message_thread, const char *source, size_t length, size_t index);
static int lm_p0_is_field_space(struct LmMessageThread *lm_lmx_message_thread, char value);
static int lm_p0_is_field_separator(struct LmMessageThread *lm_lmx_message_thread, char value);
static int lm_p0_is_short_form_separator(struct LmMessageThread *lm_lmx_message_thread, char value);
static int lm_p0_is_quoted_token_boundary(struct LmMessageThread *lm_lmx_message_thread, char value);
static int lm_p0_starts_python_string(struct LmMessageThread *lm_lmx_message_thread, const char *text, size_t length, size_t index);
static int lm_p0_is_decimal_digit(struct LmMessageThread *lm_lmx_message_thread, char value);
static char * lm_p0_copy_bytes(struct LmMessageThread *lm_lmx_message_thread, const char *source, size_t length);
static LmP0Text * lm_p0_text_view_new_cstr(struct LmMessageThread *lm_lmx_message_thread, const char *text);
static void lm_p0_text_view_delete(struct LmMessageThread *lm_lmx_message_thread, LmP0Text *text);
static LmP0Text * lm_p0_text_from_cstr(struct LmMessageThread *lm_lmx_message_thread, const char *text);


#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#ifndef LM_P0_ENABLE_REGISTRY_COMPARE
#define LM_P0_ENABLE_REGISTRY_COMPARE 1
#endif

static int lm_p0_registry_selftest_expect_path(struct LmMessageThread *lm_lmx_message_thread, const char *path, size_t index, const char *expected_key, const char *expected_value);
static int lm_p0_registry_selftest_expect_lookup(struct LmMessageThread *lm_lmx_message_thread, const char *path, const char *key, const char *expected_value);
static int lm_p0_registry_source_tables_selftest(struct LmMessageThread *lm_lmx_message_thread);


#if LM_P0_REGISTRY_SELFTEST
int main(void);

static int lm_lmx_module_private_1_init_all(struct LmMessageThread *lm_lmx_module_private_1_message_thread, struct LmOwnArena *lm_lmx_module_private_1_arena);
static inline int lm_message_thread_diagnostic_status(const LmMessageThreadExecutionContext *context) {
    if (context == 0 || context->diagnostic_code == 0) {
        return 1;
    }
    return context->diagnostic_code;
}
int main(void) {
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
            {
                int lm_return_0 = lm_p0_registry_source_tables_selftest(lm_lmx_module_private_1_main_lm_lmx_message_thread);
                lm_message_thread_request_stop(lm_lmx_module_private_1_main_lm_lmx_message_thread, lm_return_0);
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
#endif
static int lm_p0_document_init_owners(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document);
static int lm_p0_document_owners_belong_to_actor(struct LmMessageThread *lm_lmx_message_thread, const LmP0Document *document);
static void lm_p0_document_destroy_owners(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document);
static void lm_p0_document_freeze_tree(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document);
static void lm_p0_indent_stack_free(struct LmMessageThread *lm_lmx_message_thread, LmP0IndentStack *stack);
static void lm_p0_indent_stack_free_any(void *object);
static int lm_p0_indent_stack_push(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmP0IndentStack *stack, size_t column, size_t line, size_t source_column);
static int lm_p0_indent_stack_init(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmP0IndentStack *stack);
static LmP0IndentStack * lm_p0_indent_stack_new_empty(struct LmMessageThread *lm_lmx_message_thread);
static LmP0IndentStack * lm_p0_indent_stack_new(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document);
static void lm_p0_indent_stack_delete(struct LmMessageThread *lm_lmx_message_thread, LmP0IndentStack *stack);
static int lm_p0_indent_stack_copy(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmP0IndentStack *target, const LmP0IndentStack *source, size_t line, size_t column);
static LmP0IndentStack * lm_p0_indent_stack_clone(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, const LmP0IndentStack *source, size_t line, size_t column);
static size_t lm_p0_indent_tab_column(struct LmMessageThread *lm_lmx_message_thread, size_t column);
static void lm_p0_scan_indent_column(struct LmMessageThread *lm_lmx_message_thread, const char *source, size_t start, size_t end, size_t *out_offset, size_t *out_column);
static size_t lm_p0_visual_column_between(struct LmMessageThread *lm_lmx_message_thread, const char *source, size_t start, size_t end);
static const LmTableCell * lm_p0_registry_source_path_cell_at(struct LmMessageThread *lm_lmx_message_thread, const char *path, size_t index, const LmTableCell **out_key_cell);
static const LmTableCell * lm_p0_registry_source_path_lookup_cell_slice(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view, const char *path, size_t path_length, const char *key, size_t key_length, const LmTableCell **out_key_cell);
static const char * lm_p0_registry_lookup_in(struct LmMessageThread *lm_lmx_message_thread, LmP0Registry *registry, const LmP0Text *key, const char *table);
static const char * lm_p0_registry_lookup(struct LmMessageThread *lm_lmx_message_thread, const LmP0Text *key, const char *table);
static const char * lm_p0_registry_lookup_cstr_in(struct LmMessageThread *lm_lmx_message_thread, LmP0Registry *registry, const char *key, const char *table);
static const char * lm_p0_registry_lookup_cstr(struct LmMessageThread *lm_lmx_message_thread, const char *key, const char *table);
static int lm_p0_registry_source_path_has_rows(struct LmMessageThread *lm_lmx_message_thread, const char *table);
static int lm_p0_registry_table_has_rows(struct LmMessageThread *lm_lmx_message_thread, const char *table);
static int lm_p0_registry_table_has_rows_loaded_or_loading(struct LmMessageThread *lm_lmx_message_thread, const char *table);
static size_t lm_p0_count_line_breaks(struct LmMessageThread *lm_lmx_message_thread, const char *source, size_t start, size_t end);
static void lm_p0_position_in_slice(struct LmMessageThread *lm_lmx_message_thread, const char *text, size_t length, size_t index, size_t base_line, size_t base_column, size_t *out_line, size_t *out_column);
static void lm_p0_advance_layout_line(struct LmMessageThread *lm_lmx_message_thread, const char *source, size_t length, size_t line_start, size_t line_end, size_t *offset, size_t *line);
static int lm_p0_index_is_line_start(struct LmMessageThread *lm_lmx_message_thread, const char *text, size_t index);
static int lm_p0_line_rest_is_horizontal_space(struct LmMessageThread *lm_lmx_message_thread, const char *source, size_t start, size_t end);
static size_t lm_p0_find_physical_line_end(struct LmMessageThread *lm_lmx_message_thread, const char *source, size_t length, size_t start);
static int lm_p0_text_has_prefix_name(struct LmMessageThread *lm_lmx_message_thread, const char *text, size_t length, const char *name, int allow_bare);
static LmP0TrailerRole lm_p0_legacy_trailer_role(struct LmMessageThread *lm_lmx_message_thread, const char *text, size_t length);
static LmP0TrailerRole lm_p0_trailer_role_from_payload(struct LmMessageThread *lm_lmx_message_thread, const char *payload);
static const char * lm_p0_trailer_role_payload(struct LmMessageThread *lm_lmx_message_thread, LmP0TrailerRole role);
static int lm_p0_registry_trailer_allows_bare(struct LmMessageThread *lm_lmx_message_thread, const char *class_name);
static int lm_p0_trailer_role_is_tail_cutter(struct LmMessageThread *lm_lmx_message_thread, LmP0TrailerRole role);
static int lm_p0_node_head_is(struct LmMessageThread *lm_lmx_message_thread, const LmP0Node *node, const char *name);
static int lm_p0_trailer_role_accepts_target(struct LmMessageThread *lm_lmx_message_thread, LmP0TrailerRole role, const LmP0Node *target);
const LmP0Structure * lm_p0_node_structure(struct LmMessageThread *lm_lmx_message_thread, const LmP0Node *node);
const LmP0Frame * lm_p0_node_frame(struct LmMessageThread *lm_lmx_message_thread, const LmP0Node *node);
const LmP0Text * lm_p0_node_atom(struct LmMessageThread *lm_lmx_message_thread, const LmP0Node *node);
const LmP0Trailer * lm_p0_structure_trailer(struct LmMessageThread *lm_lmx_message_thread, const LmP0Structure *structure);
const LmP0Text * lm_p0_frame_head(struct LmMessageThread *lm_lmx_message_thread, const LmP0Frame *frame);
const LmP0Structure * lm_p0_frame_body(struct LmMessageThread *lm_lmx_message_thread, const LmP0Frame *frame);
const LmP0Trailer * lm_p0_frame_trailer(struct LmMessageThread *lm_lmx_message_thread, const LmP0Frame *frame);
const LmP0Text * lm_p0_trailer_spelling(struct LmMessageThread *lm_lmx_message_thread, const LmP0Trailer *trailer);
const LmP0Structure * lm_p0_trailer_body(struct LmMessageThread *lm_lmx_message_thread, const LmP0Trailer *trailer);
static int lm_p0_stream_event_is_tail_cutter(struct LmMessageThread *lm_lmx_message_thread, const LmP0StreamEvent *event);
static int lm_p0_find_python_string_end(struct LmMessageThread *lm_lmx_message_thread, const char *text, size_t length, size_t start, size_t *out_end);
static size_t lm_p0_skip_python_string_unchecked(struct LmMessageThread *lm_lmx_message_thread, const char *text, size_t length, size_t start);
static void lm_p0_scan_layout_prefix(struct LmMessageThread *lm_lmx_message_thread, const char *source, size_t length, size_t start, size_t *out_offset, size_t *out_indent_column, size_t *out_dot_level);
static int lm_p0_layout_prefix_is_deeper(struct LmMessageThread *lm_lmx_message_thread, size_t indent_column, size_t dot_level, size_t base_indent_column, size_t base_dot_level);
static LmP0DashFenceStatus lm_p0_dash_fence_status(struct LmMessageThread *lm_lmx_message_thread, const char *text, size_t length, size_t *out_dash_count);
static int lm_p0_validate_dash_fence_line(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, const char *text, size_t length, size_t line, size_t column);
static int lm_p0_match_block_string_fence_line(struct LmMessageThread *lm_lmx_message_thread, const char *source, size_t line_start, size_t line_end, size_t eq_count);
static int lm_p0_match_raw_comment_fence_line(struct LmMessageThread *lm_lmx_message_thread, const char *source, size_t line_start, size_t line_end, size_t star_count);
static void lm_p0_dump_append_size(struct LmMessageThread *lm_lmx_message_thread, LmP0Dump *dump, size_t value);
static void lm_p0_dump_append_field_count_line(struct LmMessageThread *lm_lmx_message_thread, LmP0Dump *dump, size_t field_count);
static void lm_p0_registry_private_api_anchor(struct LmMessageThread *lm_lmx_message_thread);
static void lm_p0_registry_component_destroy(struct LmMessageThread *lm_lmx_message_thread, void *component);
static LmP0Registry * lm_p0_registry_current(struct LmMessageThread *lm_lmx_message_thread);
static int lm_p0_registry_init(struct LmMessageThread *lm_lmx_message_thread);
static void lm_p0_registry_destroy(struct LmMessageThread *lm_lmx_message_thread);
static LmP0Text * lm_p0_text_ref_new_empty(struct LmMessageThread *lm_lmx_message_thread);
static void lm_p0_text_ref_delete(struct LmMessageThread *lm_lmx_message_thread, LmP0Text *text);
static int lm_p0_document_register_lazy_text(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, const char *source, size_t length, const char **patch_slot, size_t line, size_t column);
static LmP0Text * lm_p0_new_text(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, const char *source, size_t length, size_t line, size_t column);
static LmP0Structure * lm_p0_new_structure(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, size_t line, size_t column);
static LmP0Frame * lm_p0_new_frame(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, size_t line, size_t column);
static LmP0Trailer * lm_p0_new_trailer(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, const char *spelling, size_t spelling_length, size_t line, size_t column);
static void lm_p0_set_diagnostic(LmP0Document *document, int code, size_t line, size_t column, const char *message);
static int lm_p0_registry_column_has_descriptor(struct LmMessageThread *lm_lmx_message_thread, const LmRegistrySourceColumn *column, const char *descriptor);
static int lm_p0_registry_cell_none_cell_matches(struct LmMessageThread *lm_lmx_message_thread, LmP0Registry *registry, const LmP0Text *payload, const LmP0Text *class_atom);
static int lm_p0_registry_cell_is_null(struct LmMessageThread *lm_lmx_message_thread, LmP0Registry *registry, const LmP0Text *atom, const LmRegistrySourceColumn *column);
static int lm_p0_registry_cell_value(struct LmMessageThread *lm_lmx_message_thread, LmP0Registry *registry, const LmP0Text *atom, const LmRegistrySourceColumn *column, LmP0Text *out_value);
static LmTableDescriptor * lm_p0_registry_source_descriptor_new(struct LmMessageThread *lm_lmx_message_thread, const LmP0Text *table_name, LmRegistrySourceColumn **columns, size_t column_count);
static int lm_p0_registry_materialize_source_row(struct LmMessageThread *lm_lmx_message_thread, LmP0Registry *registry, const LmP0Text *table_name, LmRegistrySourceColumn **columns, size_t column_count, const LmP0Node **cells);
static int lm_p0_registry_compare_enabled(struct LmMessageThread *lm_lmx_message_thread);
static void lm_p0_registry_compare_fail(struct LmMessageThread *lm_lmx_message_thread, const char *table, const char *key, const char *registry_payload, const char *legacy_payload);
static void lm_p0_trim_right(struct LmMessageThread *lm_lmx_message_thread, const char **text, size_t *length);
static void lm_p0_trim_trailing_line_comment(struct LmMessageThread *lm_lmx_message_thread, const char **text, size_t *length);
static int lm_p0_indent_level_from_column(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmP0IndentStack *stack, size_t column, size_t line, size_t source_column, size_t *out_level);
static size_t lm_p0_skip_fence_block_unchecked(struct LmMessageThread *lm_lmx_message_thread, const char *text, size_t length, size_t start, char fence_char);
static size_t lm_p0_find_layout_line_end(struct LmMessageThread *lm_lmx_message_thread, const char *source, size_t length, size_t start);
static size_t lm_p0_scan_brace_mark_unchecked(const char *text, size_t length, size_t start, int *closed);
static int lm_p0_skip_brace_mark_ex(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmP0Structure *structure, const char *text, size_t length, size_t *index, size_t line, size_t column, int record);
static int lm_p0_skip_brace_mark(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, const char *text, size_t length, size_t *index, size_t line, size_t column);
static int lm_p0_consume_brace_mark(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmP0Structure *structure, const char *text, size_t length, size_t *index, size_t line, size_t column);
static int lm_p0_skip_leading_brace_marks_ex(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, const char *text, size_t length, size_t line, size_t column, size_t *index, int record);
static int lm_p0_skip_leading_brace_marks(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, const char *text, size_t length, size_t line, size_t column, size_t *index);
static int lm_p0_line_is_standalone_mix_run(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, const char *text, size_t length, size_t line, size_t column, int *out_is_mix);
static int lm_p0_scan_leading_mix_prefix(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, const char *text, size_t length, size_t line, size_t column, size_t *out_prefix_start, size_t *out_prefix_end, size_t *out_anchor);
static LmP0DashFenceStatus lm_p0_dash_fence_status_after_comment_trim(const char *text, size_t length, size_t *out_dash_count);
static int lm_p0_scan_raw_comment_block(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, const char *source, size_t length, size_t line_start, size_t line, size_t *next_offset, size_t *next_line);
static int lm_p0_scan_block_string_event(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, const char *source, size_t length, size_t line_start, size_t line, LmP0StreamEvent *event, size_t *next_offset, size_t *next_line);
static LmP0Node * lm_p0_new_node(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmP0NodeKind kind);
static int lm_p0_append_field(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmP0Structure *structure, LmP0Node *node);
static int lm_p0_pointer_source_offset(struct LmMessageThread *lm_lmx_message_thread, const LmP0Document *document, const char *pointer, size_t *out_offset);
static size_t lm_p0_offset_from_line_column(struct LmMessageThread *lm_lmx_message_thread, const char *text, size_t length, size_t line, size_t column);
static void lm_p0_copy_payload_diagnostic(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, const LmP0Document *payload_document, size_t payload_offset);
static void lm_p0_adjust_node_span_to_document(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmP0Node *node, size_t base_offset);
static void lm_p0_adjust_structure_spans_to_document(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmP0Structure *structure, size_t base_offset);
static int lm_p0_record_mix_mark(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmP0Structure *structure, const char *text, size_t length, size_t line, size_t column, size_t start, size_t end);
static int lm_p0_is_bare_identifier_start(struct LmMessageThread *lm_lmx_message_thread, char c);
static int lm_p0_is_bare_identifier_rest(struct LmMessageThread *lm_lmx_message_thread, char c);
static int lm_p0_is_hex_digit(struct LmMessageThread *lm_lmx_message_thread, char c);
static int lm_p0_scan_number_token(struct LmMessageThread *lm_lmx_message_thread, const char *text, size_t end_index, size_t start, size_t *out_end);
static size_t lm_p0_scan_c_quoted_token(struct LmMessageThread *lm_lmx_message_thread, const char *text, size_t end_index, size_t quote_index);
static int lm_p0_starts_c_prefixed_quote(struct LmMessageThread *lm_lmx_message_thread, const char *text, size_t end_index, size_t start);
static size_t lm_p0_scan_c_char_token(struct LmMessageThread *lm_lmx_message_thread, const char *text, size_t end_index, size_t start);
static size_t lm_p0_scan_c_prefixed_quote_token(struct LmMessageThread *lm_lmx_message_thread, const char *text, size_t end_index, size_t start);
static int lm_p0_starts_c_surface_atom(struct LmMessageThread *lm_lmx_message_thread, const char *text, size_t end_index, size_t start);
static int lm_p0_is_c_surface_top_boundary(struct LmMessageThread *lm_lmx_message_thread, char value);
static size_t lm_p0_scan_c_sizeof_surface_atom(struct LmMessageThread *lm_lmx_message_thread, const char *text, size_t end_index, size_t start);
static size_t lm_p0_scan_c_surface_atom(struct LmMessageThread *lm_lmx_message_thread, const char *text, size_t end_index, size_t start);
static int lm_p0_scan_c_char(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, const char *text, size_t length, size_t *index, size_t line, size_t base_column);
static int lm_p0_scan_c_prefixed_quote(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, const char *text, size_t length, size_t *index, size_t line, size_t base_column);
static size_t lm_p0_scan_builtin_compact_atom_piece(struct LmMessageThread *lm_lmx_message_thread, const char *text, size_t end_index, size_t start);
static size_t lm_p0_scan_registry_compact_atom_piece(struct LmMessageThread *lm_lmx_message_thread, const char *text, size_t end_index, size_t start);
static size_t lm_p0_scan_compact_atom_piece(struct LmMessageThread *lm_lmx_message_thread, const char *text, size_t end_index, size_t start, int allow_signed_number);
static int lm_p0_append_atom_slice(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmP0Structure *structure, const char *text, size_t length, size_t line, size_t column, size_t offset, size_t start, size_t end_index);
static int lm_p0_append_positional_skip(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmP0Structure *structure, const char *text, size_t length, size_t line, size_t column, size_t offset, size_t index);
static int lm_p0_append_compact_atom_pieces(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmP0Structure *structure, const char *text, size_t length, size_t line, size_t column, size_t offset, size_t start, size_t end_index);
static void lm_p0_free_node(struct LmMessageThread *lm_lmx_message_thread, LmP0Node *node);
static int lm_p0_relaxed_level_from_column(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmP0IndentStack *stack, size_t column, size_t base_level, size_t line, size_t source_column, size_t *out_level);
static int lm_p0_source_level_after_line_break(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmP0IndentStack *indent_stack, const char *text, size_t length, size_t index, size_t line, size_t column, size_t base_level, size_t *content_index, size_t *out_level);
static int lm_p0_skip_field_space(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmP0IndentStack *indent_stack, LmP0Structure *structure, const char *text, size_t length, size_t *index, size_t line, size_t column, unsigned flags, size_t short_source_level, size_t layout_base_level, size_t *current_source_level, int *stopped_by_source_level);
static int lm_p0_scan_python_string(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, const char *text, size_t length, size_t *index, size_t line, size_t base_column);
static int lm_p0_scan_quoted(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, const char *text, size_t length, size_t *index, char quote, size_t line, size_t base_column);
static int lm_p0_require_quoted_token_boundary(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, const char *text, size_t length, size_t index, size_t line, size_t column);
static int lm_p0_find_matching_paren(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, const char *text, size_t length, size_t open_index, size_t line, size_t base_column, size_t *close_index);
static int lm_p0_find_matching_bracket(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, const char *text, size_t length, size_t open_index, size_t line, size_t base_column, size_t *close_index);
static int lm_p0_find_colon(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, const char *text, size_t length, size_t line, size_t column, size_t *colon_index);
static int lm_p0_field_start_looks_explicit_frame(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, const char *text, size_t length, size_t index, size_t line, size_t column);
static void lm_p0_field_parse_loop_frame_delete_any(void *object);
static LmOwnPtrStack * lm_p0_field_parse_loop_stack_new(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, size_t line, size_t column);
static void lm_p0_field_parse_loop_stack_delete(struct LmMessageThread *lm_lmx_message_thread, LmOwnPtrStack *stack);
static int lm_p0_field_parse_loop_push(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmOwnPtrStack *stack, LmP0IndentStack *indent_stack, int indent_stack_owned, LmP0Structure *structure, const char *text, size_t length, size_t line, size_t column, size_t offset, unsigned flags, size_t short_source_level, size_t layout_base_level, size_t i, size_t current_source_level, int allow_empty_fields, int expect_field, int headless_group_after_separator, LmP0FieldParseLoopContinuation continuation, LmP0Node *node, size_t start, size_t close_index);
static int lm_p0_field_parse_fail(struct LmMessageThread *lm_lmx_message_thread, LmOwnPtrStack **parse_stack, LmP0IndentStack **indent_stack, int *indent_stack_owned);
static int lm_p0_parse_append_node_and_update(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmP0Structure *structure, LmP0Node *node, unsigned flags, int allow_empty_fields, int *expect_field, int *headless_group_after_separator);
static int lm_p0_parse_fields_until_with_layout(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmP0IndentStack *indent_stack, LmP0Structure *structure, const char *text, size_t length, size_t line, size_t column, size_t offset, unsigned flags, size_t short_source_level, size_t initial_source_level, size_t layout_base_level, size_t *index);
static int lm_p0_parse_fields_until(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmP0Structure *structure, const char *text, size_t length, size_t line, size_t column, size_t offset, unsigned flags, size_t short_source_level, size_t initial_source_level, size_t *index);
static int lm_p0_parse_fields_into(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmP0Structure *structure, const char *text, size_t length, size_t line, size_t column, size_t offset);
static int lm_p0_stack_ensure(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmP0Stack *stack, size_t level);
static void lm_p0_stack_truncate_deeper(struct LmMessageThread *lm_lmx_message_thread, LmP0Stack *stack, size_t level);
static size_t lm_p0_stack_top_level(struct LmMessageThread *lm_lmx_message_thread, const LmP0Stack *stack);
static int lm_p0_stack_level_is_trailer_body(struct LmMessageThread *lm_lmx_message_thread, const LmP0Stack *stack, size_t level);
static size_t lm_p0_stack_collapse_soft_to_event(struct LmMessageThread *lm_lmx_message_thread, LmP0Stack *stack, size_t event_level);
static LmP0TrailerRole lm_p0_registry_trailer_role(struct LmMessageThread *lm_lmx_message_thread, const char *text, size_t length);
static LmP0TrailerRole lm_p0_trailer_role(const char *text, size_t length);
static void lm_p0_stack_free(struct LmMessageThread *lm_lmx_message_thread, LmP0Stack *stack);
static void lm_p0_stack_free_any(void *object);
static LmP0Stack * lm_p0_stack_new(struct LmMessageThread *lm_lmx_message_thread);
static void lm_p0_stack_delete(struct LmMessageThread *lm_lmx_message_thread, LmP0Stack *stack);
static LmP0PendingDelimiter * lm_p0_pending_delimiter_new(struct LmMessageThread *lm_lmx_message_thread);
static LmP0StreamEvent * lm_p0_stream_event_new(struct LmMessageThread *lm_lmx_message_thread);
static LmP0StreamEvent * lm_p0_stream_event_new_copy(struct LmMessageThread *lm_lmx_message_thread, const LmP0StreamEvent *event);
static void lm_p0_stream_event_delete(struct LmMessageThread *lm_lmx_message_thread, LmP0StreamEvent *event);
static void lm_p0_pending_delimiter_clear(struct LmMessageThread *lm_lmx_message_thread, LmP0PendingDelimiter *pending);
static int lm_p0_pending_delimiter_set(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmP0PendingDelimiter *pending, const LmP0StreamEvent *event);
static void lm_p0_pending_delimiter_delete(struct LmMessageThread *lm_lmx_message_thread, LmP0PendingDelimiter *pending);
static LmP0PendingMix * lm_p0_pending_mix_new(struct LmMessageThread *lm_lmx_message_thread);
static void lm_p0_pending_mix_free(struct LmMessageThread *lm_lmx_message_thread, LmP0PendingMix *pending);
static void lm_p0_pending_mix_free_any(void *object);
static void lm_p0_pending_mix_delete(struct LmMessageThread *lm_lmx_message_thread, LmP0PendingMix *pending);
static int lm_p0_pending_mix_push(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmP0PendingMix *pending, const LmP0StreamEvent *event);
static LmP0DisabledState * lm_p0_disabled_state_new(struct LmMessageThread *lm_lmx_message_thread, size_t base_level);
static void lm_p0_disabled_state_delete(struct LmMessageThread *lm_lmx_message_thread, LmP0DisabledState *state);
static int lm_p0_node_keeps_source_child_level(struct LmMessageThread *lm_lmx_message_thread, LmP0Node *node);
static LmP0Structure * lm_p0_node_child_structure(struct LmMessageThread *lm_lmx_message_thread, LmP0Node *node);
static LmP0Node * lm_p0_structure_last_colon_frame(struct LmMessageThread *lm_lmx_message_thread, LmP0Structure *structure);
static int lm_p0_stack_install_node_lineage(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmP0Stack *stack, size_t base_level, LmP0Node *node);
static int lm_p0_stack_ensure_root_level_alias(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmP0Stack *stack, size_t level);
static int lm_p0_stack_open_implicit_anonymous(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmP0Stack *stack, size_t parent_level, size_t line, size_t column, size_t offset);
static LmP0Trailer * * lm_p0_node_trailer_slot(struct LmMessageThread *lm_lmx_message_thread, LmP0Node *node);
static LmP0Trailer * lm_p0_attach_trailer(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmP0Node *node, const char *spelling, size_t spelling_length, unsigned flags, size_t line, size_t column);
static int lm_p0_parse_trailer_item(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmP0Node *target, const char *text, size_t length, size_t line, size_t column, size_t offset, unsigned flags, LmP0Structure **out_body);
static int lm_p0_stream_resolve_pending_delimiter(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmP0Stack *stack, LmP0PendingDelimiter *pending, size_t next_level);
static int lm_p0_stream_apply_item_event(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmP0Stack *stack, const LmP0StreamEvent *event);
static int lm_p0_stream_apply_mix_event(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmP0Stack *stack, const LmP0StreamEvent *event);
static int lm_p0_stream_apply_event(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmP0Stack *stack, LmP0PendingDelimiter *pending, const LmP0StreamEvent *event);
static int lm_p0_pending_mix_flush(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmP0Stack *stack, LmP0PendingDelimiter *pending_delimiter, LmP0PendingMix *pending_mix, size_t level);
static size_t lm_p0_stream_block_string_level(struct LmMessageThread *lm_lmx_message_thread, const LmP0Stack *stack, const LmP0PendingDelimiter *pending);
static int lm_p0_validate_disabled_item_text(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, const char *text, size_t length, size_t line, size_t column);
static int lm_p0_disabled_scan_next_event(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmP0IndentStack *indent_stack, size_t *offset, size_t *line, LmP0StreamEvent *event, int *has_event);
static int lm_p0_disabled_event_is_tail_cutter(struct LmMessageThread *lm_lmx_message_thread, const LmP0StreamEvent *event);
static int lm_p0_disabled_state_accept_event(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmP0DisabledState *state, LmP0StreamEvent *event, int *done_after_event, int *done_before_event);
static int lm_p0_validate_disabled_block(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, const LmP0IndentStack *indent_stack, size_t first_next_offset, size_t first_next_line, size_t base_level, const char *header_text, size_t header_length, size_t header_line, size_t header_column, size_t *out_offset, size_t *out_line);
static int lm_p0_parse_stream(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document);
static void lm_p0_structure_recount(struct LmMessageThread *lm_lmx_message_thread, LmP0Structure *structure);
static LmP0PostprocessFrame * lm_p0_postprocess_frame_new(struct LmMessageThread *lm_lmx_message_thread, int phase);
static int lm_p0_postprocess_push_frame(struct LmMessageThread *lm_lmx_message_thread, LmOwnPtrStack *stack, LmP0PostprocessFrame *frame);
static int lm_p0_postprocess_push_node(struct LmMessageThread *lm_lmx_message_thread, LmOwnPtrStack *stack, LmP0Node *node);
static int lm_p0_postprocess_push_structure(struct LmMessageThread *lm_lmx_message_thread, LmOwnPtrStack *stack, LmP0Structure *structure);
static int lm_p0_postprocess_push_trailer(struct LmMessageThread *lm_lmx_message_thread, LmOwnPtrStack *stack, LmP0Trailer *trailer);
static int lm_p0_postprocess_push_frame_wrap(struct LmMessageThread *lm_lmx_message_thread, LmOwnPtrStack *stack, LmP0Node *node);
static LmOwnPtrStack * lm_p0_postprocess_stack_new(struct LmMessageThread *lm_lmx_message_thread);
static void lm_p0_postprocess_stack_delete(struct LmMessageThread *lm_lmx_message_thread, LmOwnPtrStack **stack);
static int lm_p0_postprocess_run(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmOwnPtrStack *stack);
static int lm_p0_postprocess_node(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmP0Node *node);
static int lm_p0_wrap_fields_from_line(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmP0Structure *structure, size_t head_line, size_t inline_event_end_offset);
static int lm_p0_validate_nonempty_colon_frames_in_trailer(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, const LmP0Trailer *trailer);
static int lm_p0_validate_nonempty_colon_frames_in_node(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, const LmP0Node *node);
static int lm_p0_validate_nonempty_colon_frames_in_structure(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, const LmP0Structure *structure);
int lm_p0_parse_bytes(struct LmMessageThread *lm_lmx_message_thread, const char *source, size_t source_length, LmP0Document **out_document);
int lm_p0_parse_string(struct LmMessageThread *lm_lmx_message_thread, const char *source, LmP0Document **out_document);
int lm_p0_parse_file(struct LmMessageThread *lm_lmx_message_thread, const char *path, LmP0Document **out_document);
static int lm_p0_registry_source_push_column_metadata(struct LmMessageThread *lm_lmx_message_thread, void *context, const LmP0Text *table_name, LmRegistrySourceColumn **columns, size_t column_count);
static int lm_p0_registry_source_push_table_row(struct LmMessageThread *lm_lmx_message_thread, void *context, const LmP0Text *table_name, LmRegistrySourceColumn **columns, size_t column_count, const LmP0Node **cells);
static int lm_p0_registry_source_join_table(struct LmMessageThread *lm_lmx_message_thread, void *context, const LmP0Text *source_table, const LmP0Text *target_table);
static int lm_p0_registry_source_text_all_char(struct LmMessageThread *lm_lmx_message_thread, const LmP0Text *text, char ch);
static int lm_p0_registry_source_text_is_array_receiver_head(struct LmMessageThread *lm_lmx_message_thread, const LmP0Text *head);
static const char * lm_p0_registry_source_frame_receiver_lookup(struct LmMessageThread *lm_lmx_message_thread, LmP0Registry *registry, const LmP0Frame *frame, const char *table);
static int lm_p0_registry_source_parse_size_payload(struct LmMessageThread *lm_lmx_message_thread, const char *payload, size_t *out_value);
static int lm_p0_registry_source_formal_param_unwrap_index(struct LmMessageThread *lm_lmx_message_thread, const LmRegistrySourceLoader *loader, void *context, const LmP0Frame *frame, size_t *out_index);
static int lm_p0_registry_source_positional_name_index(struct LmMessageThread *lm_lmx_message_thread, const LmRegistrySourceLoader *loader, void *context, const LmP0Frame *frame, size_t *out_index);
static LmRegistrySourceLoader * lm_p0_registry_source_loader_new(struct LmMessageThread *lm_lmx_message_thread);
static int lm_p0_path_has_extension(struct LmMessageThread *lm_lmx_message_thread, const char *path, const char *extension);
static int lm_p0_registry_require_source_only(struct LmMessageThread *lm_lmx_message_thread, LmP0Registry *registry, const char *phase);
static int lm_p0_registry_parse_unsigned_payload(struct LmMessageThread *lm_lmx_message_thread, const char *payload, unsigned *out_value);
static const char * lm_p0_registry_lookup_key_by_unsigned_payload(struct LmMessageThread *lm_lmx_message_thread, const char *table, unsigned value);
static int lm_p0_registry_validate_abi_constant(struct LmMessageThread *lm_lmx_message_thread, const char *table, const char *key, unsigned expected);
static int lm_p0_registry_validate_abi_constants(struct LmMessageThread *lm_lmx_message_thread);
const char * lm_p0_node_kind_class_name(struct LmMessageThread *lm_lmx_message_thread, LmP0NodeKind kind);
static int lm_p0_registry_load_default(struct LmMessageThread *lm_lmx_message_thread);
void lm_p0_document_destroy(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document);
const LmP0Node * lm_p0_document_root(struct LmMessageThread *lm_lmx_message_thread, const LmP0Document *document);
LmP0Node * lm_p0_document_mutable_root(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document);
const LmP0Diagnostic * lm_p0_document_diagnostic(struct LmMessageThread *lm_lmx_message_thread, const LmP0Document *document);
void lm_p0_free(struct LmMessageThread *lm_lmx_message_thread, void *ptr);
static int lm_p0_dump_reserve(struct LmMessageThread *lm_lmx_message_thread, LmP0Dump *dump, size_t extra);
static void lm_p0_dump_append(LmP0Dump *dump, const char *text, size_t length);
static void lm_p0_dump_append_cstr(LmP0Dump *dump, const char *text);
static void lm_p0_dump_indent(struct LmMessageThread *lm_lmx_message_thread, LmP0Dump *dump, size_t indent);
static void lm_p0_dump_text(struct LmMessageThread *lm_lmx_message_thread, LmP0Dump *dump, const LmP0Text *text);
static LmP0DumpFrame * lm_p0_dump_frame_new(struct LmMessageThread *lm_lmx_message_thread, int phase, size_t indent);
static int lm_p0_dump_push_frame(struct LmMessageThread *lm_lmx_message_thread, LmOwnPtrStack *stack, LmP0DumpFrame *frame);
static int lm_p0_dump_push_node(struct LmMessageThread *lm_lmx_message_thread, LmOwnPtrStack *stack, const LmP0Node *node, size_t indent);
static int lm_p0_dump_push_structure(struct LmMessageThread *lm_lmx_message_thread, LmOwnPtrStack *stack, const LmP0Structure *structure, size_t indent);
static int lm_p0_dump_push_trailer(struct LmMessageThread *lm_lmx_message_thread, LmOwnPtrStack *stack, const LmP0Trailer *trailer, size_t indent);
static LmOwnPtrStack * lm_p0_dump_stack_new(struct LmMessageThread *lm_lmx_message_thread);
static void lm_p0_dump_stack_delete(struct LmMessageThread *lm_lmx_message_thread, LmOwnPtrStack **stack);
static void lm_p0_dump_run(struct LmMessageThread *lm_lmx_message_thread, LmP0Dump *dump, LmOwnPtrStack *stack);
static void lm_p0_dump_node(struct LmMessageThread *lm_lmx_message_thread, LmP0Dump *dump, const LmP0Node *node, size_t indent);
static LmP0Dump * lm_p0_dump_new(struct LmMessageThread *lm_lmx_message_thread);
static char * lm_p0_dump_take_data(struct LmMessageThread *lm_lmx_message_thread, LmP0Dump *dump);
static void lm_p0_dump_delete(struct LmMessageThread *lm_lmx_message_thread, LmP0Dump *dump);
char * lm_p0_dump_alloc(struct LmMessageThread *lm_lmx_message_thread, const LmP0Document *document);















































#if defined(__GNUC__) || defined(__clang__)
__attribute__((unused))
#endif
static int lm_lmx_module_private_1_init_all(struct LmMessageThread *lm_lmx_module_private_1_message_thread, struct LmOwnArena *lm_lmx_module_private_1_arena) {
    (void)lm_lmx_module_private_1_message_thread;
    (void)lm_lmx_module_private_1_arena;
    return 0;
}






static char * lm_table_descriptor_copy_slice(struct LmMessageThread *lm_lmx_message_thread, const char *data, size_t length) {
    (void)lm_lmx_message_thread;
    if (data == 0 && length != 0U) {
        return 0;
    }
    return lm_own_copy_bytes(data, length);
}

static char * lm_table_descriptor_copy_cstr(struct LmMessageThread *lm_lmx_message_thread, const char *value) {
    (void)lm_lmx_message_thread;
    if (value == 0) {
        return 0;
    }
    return lm_table_descriptor_copy_slice(lm_lmx_message_thread, value, strlen(value));
}

static int lm_table_descriptor_slice_equals(struct LmMessageThread *lm_lmx_message_thread, const char *left, size_t left_length, const char *right, size_t right_length) {
    (void)lm_lmx_message_thread;
    if (left_length != right_length) {
        return 0;
    }
    if (left_length == 0U) {
        return 1;
    }
    if (left == 0 || right == 0) {
        return 0;
    }
    return memcmp(left, right, left_length) == 0;
}

static int lm_table_descriptor_cstr_equals_slice(struct LmMessageThread *lm_lmx_message_thread, const char *left, const char *right, size_t right_length) {
    (void)lm_lmx_message_thread;
    if (left == 0) {
        return 0;
    }
    return lm_table_descriptor_slice_equals(lm_lmx_message_thread, left, strlen(left), right, right_length);
}

static LmOwnPtrStack * lm_table_descriptor_stack_new(struct LmMessageThread *lm_lmx_message_thread, LmOwnDelete delete_item) {
    (void)lm_lmx_message_thread;
    LmOwnPtrStack * stack;
    stack = lm_own_new_zero(sizeof(stack[0]));
    if (stack != 0) {
        lm_own_ptr_stack_init(stack, delete_item);
    }
    return stack;
}

static void lm_table_column_descriptor_delete_any(void *object) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    LmTableColumnDescriptor * column;
    column = ((LmTableColumnDescriptor *)object);
    if (column == 0) {
        return;
    }
    lm_own_delete(column -> name, 0);
    lm_own_delete(column -> descriptor, 0);
    if (column -> descriptors != 0) {
        lm_own_ptr_stack_destroy(column -> descriptors);
        lm_own_delete(column -> descriptors, 0);
        column->descriptors = 0;
    }
    lm_own_delete(column -> type_name, 0);
    column->name = 0;
    column->descriptor = 0;
    column->type_name = 0;
    lm_own_delete(column, 0);
}

static void lm_table_cell_delete_any(void *object) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    LmTableCell * cell;
    cell = ((LmTableCell *)object);
    if (cell == 0) {
        return;
    }
    lm_own_delete(cell -> atom, 0);
    lm_own_delete(cell -> value, 0);
    cell->atom = 0;
    cell->value = 0;
    cell->node = 0;
    cell->source = 0;
    lm_own_delete(cell, 0);
}

static void lm_table_row_delete_any(void *object) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    LmTableRow * row;
    row = ((LmTableRow *)object);
    if (row == 0) {
        return;
    }
    if (row -> cells != 0) {
        lm_own_ptr_stack_destroy(row -> cells);
        lm_own_delete(row -> cells, 0);
        row->cells = 0;
    }
    row->source = 0;
    row->source_native = 0;
    lm_own_delete(row, 0);
}

static void lm_registry_view_row_delete_any(void *object) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    LmRegistryViewRow * row;
    row = ((LmRegistryViewRow *)object);
    if (row == 0) {
        return;
    }
    lm_own_delete(row -> table, 0);
    lm_own_delete(row -> key, 0);
    lm_own_delete(row -> payload, 0);
    row->table = 0;
    row->key = 0;
    row->payload = 0;
    row->payload_node = 0;
    row->source = 0;
    lm_own_delete(row, 0);
}

static void lm_table_descriptor_delete_any(void *object) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    LmTableDescriptor * table;
    table = ((LmTableDescriptor *)object);
    if (table == 0) {
        return;
    }
    if (table -> columns != 0) {
        lm_own_ptr_stack_destroy(table -> columns);
        lm_own_delete(table -> columns, 0);
        table->columns = 0;
    }
    if (table -> rows != 0) {
        lm_own_ptr_stack_destroy(table -> rows);
        lm_own_delete(table -> rows, 0);
        table->rows = 0;
    }
    if (table -> materialized_rows != 0) {
        lm_own_ptr_stack_destroy(table -> materialized_rows);
        lm_own_delete(table -> materialized_rows, 0);
        table->materialized_rows = 0;
    }
    if (table -> source_rows != 0) {
        lm_own_ptr_stack_destroy(table -> source_rows);
        lm_own_delete(table -> source_rows, 0);
        table->source_rows = 0;
    }
    lm_own_delete(table -> name, 0);
    table->name = 0;
    lm_own_delete(table, 0);
}

static LmTableColumnDescriptor * lm_table_column_descriptor_new_slice(struct LmMessageThread *lm_lmx_message_thread, const char *name, size_t name_length, const char *type_name, size_t type_name_length, size_t address_depth, size_t array_rank, int is_const) {
    (void)lm_lmx_message_thread;
    LmTableColumnDescriptor * column;
    if (name == 0) {
        return 0;
    }
    column = lm_own_new_zero(sizeof(column[0]));
    if (column == 0) {
        return 0;
    }
    column->name = lm_table_descriptor_copy_slice(lm_lmx_message_thread, name, name_length);
    column->descriptors = lm_table_descriptor_stack_new(lm_lmx_message_thread, lm_own_delete_plain);
    if (type_name != 0) {
        column->type_name = lm_table_descriptor_copy_slice(lm_lmx_message_thread, type_name, type_name_length);
    }
    column->address_depth = address_depth;
    column->array_rank = array_rank;
    column->is_const = is_const;
    if (column -> name == 0 || column -> descriptors == 0 || (type_name != 0 && column -> type_name == 0)) {
        lm_table_column_descriptor_delete_any(column);
        return 0;
    }
    return column;
}

static int lm_table_column_descriptor_add_descriptor_slice(struct LmMessageThread *lm_lmx_message_thread, LmTableColumnDescriptor *column, const char *descriptor, size_t descriptor_length) {
    (void)lm_lmx_message_thread;
    char *copy;
    if (column == 0 || column -> descriptors == 0 || descriptor == 0) {
        return 1;
    }
    copy = lm_table_descriptor_copy_slice(lm_lmx_message_thread, descriptor, descriptor_length);
    if (copy == 0) {
        return 1;
    }
    if (column -> descriptor == 0) {
        column->descriptor = lm_table_descriptor_copy_slice(lm_lmx_message_thread, descriptor, descriptor_length);
        if (column -> descriptor == 0) {
            lm_own_delete(copy, 0);
            return 1;
        }
    }
    if (lm_own_ptr_stack_push(column -> descriptors, copy) != 0) {
        lm_own_delete(copy, 0);
        return 1;
    }
    return 0;
}

static size_t lm_table_column_descriptor_descriptor_count(struct LmMessageThread *lm_lmx_message_thread, const LmTableColumnDescriptor *column) {
    (void)lm_lmx_message_thread;
    if (column == 0 || column -> descriptors == 0) {
        return 0U;
    }
    return column -> descriptors -> count;
}

static const char * lm_table_column_descriptor_descriptor_at(struct LmMessageThread *lm_lmx_message_thread, const LmTableColumnDescriptor *column, size_t index) {
    (void)lm_lmx_message_thread;
    if (column == 0 || column -> descriptors == 0) {
        return 0;
    }
    return lm_own_ptr_stack_at(column -> descriptors, index);
}

static int lm_table_column_descriptor_has_descriptor(struct LmMessageThread *lm_lmx_message_thread, const LmTableColumnDescriptor *column, const char *descriptor) {
    (void)lm_lmx_message_thread;
    const char *candidate;
    size_t index;
    if (column == 0 || descriptor == 0) {
        return 0;
    }
    index = 0U;
    while (index < lm_table_column_descriptor_descriptor_count(lm_lmx_message_thread, column)) {
        candidate = lm_table_column_descriptor_descriptor_at(lm_lmx_message_thread, column, index);
        if (candidate != 0 && strcmp(candidate, descriptor) == 0) {
            return 1;
        }
        index = index + 1U;
    }
    return 0;
}

static int lm_table_descriptor_take_column(struct LmMessageThread *lm_lmx_message_thread, LmTableDescriptor *table, LmTableColumnDescriptor *column) {
    (void)lm_lmx_message_thread;
    if (table == 0 || table -> columns == 0 || column == 0 || column -> name == 0) {
        return 1;
    }
    column->index = table -> columns -> count;
    if (lm_own_ptr_stack_push(table -> columns, column) != 0) {
        return 1;
    }
    return 0;
}

static int lm_table_descriptor_add_column_slices(struct LmMessageThread *lm_lmx_message_thread, LmTableDescriptor *table, const char *name, size_t name_length, const char *type_name, size_t type_name_length, const char **descriptor_data, const size_t *descriptor_lengths, size_t descriptor_count, size_t address_depth, size_t array_rank, int is_const) {
    (void)lm_lmx_message_thread;
    LmTableColumnDescriptor * column;
    size_t descriptor_index;
    if (table == 0 || name == 0 || (type_name == 0 && type_name_length != 0U) || (descriptor_count != 0U && (descriptor_data == 0 || descriptor_lengths == 0))) {
        return 1;
    }
    column = lm_table_column_descriptor_new_slice(lm_lmx_message_thread, name, name_length, type_name, type_name_length, address_depth, array_rank, is_const);
    if (column == 0) {
        return 1;
    }
    descriptor_index = 0U;
    while (descriptor_index < descriptor_count) {
        if (descriptor_data[descriptor_index] == 0 || lm_table_column_descriptor_add_descriptor_slice(lm_lmx_message_thread, column, descriptor_data[descriptor_index], descriptor_lengths[descriptor_index]) != 0) {
            lm_table_column_descriptor_delete_any(column);
            return 1;
        }
        descriptor_index = descriptor_index + 1U;
    }
    if (lm_table_descriptor_take_column(lm_lmx_message_thread, table, column) != 0) {
        lm_table_column_descriptor_delete_any(column);
        return 1;
    }
    return 0;
}

static int lm_table_descriptor_add_column(struct LmMessageThread *lm_lmx_message_thread, LmTableDescriptor *table, const char *name, const char *descriptor) {
    (void)lm_lmx_message_thread;
    const char *descriptor_data[1];
    size_t descriptor_lengths[1];
    if (name == 0) {
        return 1;
    }
    if (descriptor == 0) {
        return lm_table_descriptor_add_column_slices(lm_lmx_message_thread, table, name, strlen(name), 0, 0U, 0, 0, 0U, 0U, 0U, 0);
    }
    descriptor_data[0] = descriptor;
    descriptor_lengths[0] = strlen(descriptor);
    return lm_table_descriptor_add_column_slices(lm_lmx_message_thread, table, name, strlen(name), 0, 0U, descriptor_data, descriptor_lengths, 1U, 0U, 0U, 0);
}

static LmTableDescriptor * lm_table_descriptor_new_empty_slice(struct LmMessageThread *lm_lmx_message_thread, const char *name, size_t name_length) {
    (void)lm_lmx_message_thread;
    LmTableDescriptor * table;
    if (name == 0) {
        return 0;
    }
    table = lm_own_new_zero(sizeof(table[0]));
    if (table == 0) {
        return 0;
    }
    table->name = lm_table_descriptor_copy_slice(lm_lmx_message_thread, name, name_length);
    table->columns = lm_table_descriptor_stack_new(lm_lmx_message_thread, lm_table_column_descriptor_delete_any);
    table->rows = lm_table_descriptor_stack_new(lm_lmx_message_thread, lm_registry_view_row_delete_any);
    table->materialized_rows = lm_table_descriptor_stack_new(lm_lmx_message_thread, lm_table_row_delete_any);
    table->source_rows = lm_table_descriptor_stack_new(lm_lmx_message_thread, 0);
    if (table -> name == 0 || table -> columns == 0 || table -> rows == 0 || table -> materialized_rows == 0 || table -> source_rows == 0) {
        lm_table_descriptor_delete_any(table);
        return 0;
    }
    return table;
}

static LmTableDescriptor * lm_table_descriptor_new_slice(struct LmMessageThread *lm_lmx_message_thread, const char *name, size_t name_length) {
    (void)lm_lmx_message_thread;
    LmTableDescriptor * table;
    table = lm_table_descriptor_new_empty_slice(lm_lmx_message_thread, name, name_length);
    if (table == 0) {
        return 0;
    }
    if (lm_table_descriptor_add_column(lm_lmx_message_thread, table, "class", "key") != 0 || lm_table_descriptor_add_column(lm_lmx_message_thread, table, "value", "payload") != 0) {
        lm_table_descriptor_delete_any(table);
        return 0;
    }
    return table;
}

static size_t lm_table_descriptor_column_count(struct LmMessageThread *lm_lmx_message_thread, const LmTableDescriptor *table) {
    (void)lm_lmx_message_thread;
    if (table == 0 || table -> columns == 0) {
        return 0U;
    }
    return table -> columns -> count;
}

static const LmTableColumnDescriptor * lm_table_descriptor_column_at(struct LmMessageThread *lm_lmx_message_thread, const LmTableDescriptor *table, size_t index) {
    (void)lm_lmx_message_thread;
    if (table == 0 || table -> columns == 0) {
        return 0;
    }
    return lm_own_ptr_stack_at(table -> columns, index);
}

static size_t lm_table_descriptor_row_count(struct LmMessageThread *lm_lmx_message_thread, const LmTableDescriptor *table) {
    (void)lm_lmx_message_thread;
    if (table == 0 || table -> rows == 0) {
        return 0U;
    }
    return table -> rows -> count;
}

static const LmRegistryViewRow * lm_table_descriptor_row_at(struct LmMessageThread *lm_lmx_message_thread, const LmTableDescriptor *table, size_t index) {
    (void)lm_lmx_message_thread;
    if (table == 0 || table -> rows == 0) {
        return 0;
    }
    return lm_own_ptr_stack_at(table -> rows, index);
}

static LmTableCell * lm_table_cell_new_slice(struct LmMessageThread *lm_lmx_message_thread, const char *atom, size_t atom_length, const char *value, size_t value_length, const void *node, const void *source, int explicit_none) {
    (void)lm_lmx_message_thread;
    LmTableCell * cell;
    if (atom == 0 && value == 0 && node == 0 && explicit_none == 0) {
        return 0;
    }
    cell = lm_own_new_zero(sizeof(cell[0]));
    if (cell == 0) {
        return 0;
    }
    if (atom != 0) {
        cell->atom = lm_table_descriptor_copy_slice(lm_lmx_message_thread, atom, atom_length);
    }
    if (value != 0) {
        cell->value = lm_table_descriptor_copy_slice(lm_lmx_message_thread, value, value_length);
    }
    cell->node = node;
    cell->source = source;
    cell->explicit_none = explicit_none;
    if ((atom != 0 && cell -> atom == 0) || (value != 0 && cell -> value == 0)) {
        lm_table_cell_delete_any(cell);
        return 0;
    }
    return cell;
}

static LmTableCell * lm_table_cell_new_cstr(struct LmMessageThread *lm_lmx_message_thread, const char *atom, const char *value, const void *node, const void *source, int explicit_none) {
    (void)lm_lmx_message_thread;
    size_t atom_length;
    size_t value_length;
    atom_length = 0U;
    value_length = 0U;
    if (atom != 0) {
        atom_length = strlen(atom);
    }
    if (value != 0) {
        value_length = strlen(value);
    }
    return lm_table_cell_new_slice(lm_lmx_message_thread, atom, atom_length, value, value_length, node, source, explicit_none);
}

static LmTableRow * lm_table_row_new(struct LmMessageThread *lm_lmx_message_thread, const LmTableRow *source) {
    (void)lm_lmx_message_thread;
    LmTableRow * row;
    row = lm_own_new_zero(sizeof(row[0]));
    if (row == 0) {
        return 0;
    }
    row->cells = lm_table_descriptor_stack_new(lm_lmx_message_thread, lm_table_cell_delete_any);
    row->source = source;
    if (source != 0) {
        row->source_native = source -> source_native;
    }
    if (row -> cells == 0) {
        lm_table_row_delete_any(row);
        return 0;
    }
    return row;
}

static int lm_table_row_take_cell(struct LmMessageThread *lm_lmx_message_thread, LmTableRow *row, LmTableCell *cell) {
    (void)lm_lmx_message_thread;
    if (row == 0 || row -> cells == 0 || cell == 0) {
        return 1;
    }
    return lm_own_ptr_stack_push(row -> cells, cell);
}

static int lm_table_row_take_cell_slice(struct LmMessageThread *lm_lmx_message_thread, LmTableRow *row, const char *atom, size_t atom_length, const char *value, size_t value_length, const void *node, const void *source, int explicit_none) {
    (void)lm_lmx_message_thread;
    LmTableCell * cell;
    cell = lm_table_cell_new_slice(lm_lmx_message_thread, atom, atom_length, value, value_length, node, source, explicit_none);
    if (cell == 0) {
        return 1;
    }
    if (lm_table_row_take_cell(lm_lmx_message_thread, row, cell) != 0) {
        lm_table_cell_delete_any(cell);
        return 1;
    }
    return 0;
}

static size_t lm_table_row_cell_count(struct LmMessageThread *lm_lmx_message_thread, const LmTableRow *row) {
    (void)lm_lmx_message_thread;
    if (row == 0 || row -> cells == 0) {
        return 0U;
    }
    return row -> cells -> count;
}

static const LmTableCell * lm_table_row_cell_at(struct LmMessageThread *lm_lmx_message_thread, const LmTableRow *row, size_t index) {
    (void)lm_lmx_message_thread;
    if (row == 0 || row -> cells == 0) {
        return 0;
    }
    return lm_own_ptr_stack_at(row -> cells, index);
}

static int lm_table_descriptor_take_materialized_row(struct LmMessageThread *lm_lmx_message_thread, LmTableDescriptor *table, LmTableRow *row) {
    (void)lm_lmx_message_thread;
    if (table == 0 || table -> materialized_rows == 0 || row == 0 || row -> cells == 0) {
        return 1;
    }
    if (row -> cells -> count != lm_table_descriptor_column_count(lm_lmx_message_thread, table)) {
        return 1;
    }
    row->local_sequence = table -> materialized_rows -> count;
    return lm_own_ptr_stack_push(table -> materialized_rows, row);
}

static size_t lm_table_descriptor_materialized_row_count(struct LmMessageThread *lm_lmx_message_thread, const LmTableDescriptor *table) {
    (void)lm_lmx_message_thread;
    if (table == 0 || table -> materialized_rows == 0) {
        return 0U;
    }
    return table -> materialized_rows -> count;
}

static const LmTableRow * lm_table_descriptor_materialized_row_at(struct LmMessageThread *lm_lmx_message_thread, const LmTableDescriptor *table, size_t index) {
    (void)lm_lmx_message_thread;
    if (table == 0 || table -> materialized_rows == 0) {
        return 0;
    }
    return lm_own_ptr_stack_at(table -> materialized_rows, index);
}

static int lm_table_descriptor_append_materialized_row_copy(struct LmMessageThread *lm_lmx_message_thread, LmTableDescriptor *table, const LmTableRow *source_row) {
    (void)lm_lmx_message_thread;
    LmTableRow * row;
    const LmTableCell * source_cell;
    LmTableCell * cell;
    size_t atom_length;
    size_t value_length;
    size_t index;
    if (table == 0 || source_row == 0 || lm_table_row_cell_count(lm_lmx_message_thread, source_row) != lm_table_descriptor_column_count(lm_lmx_message_thread, table)) {
        return 1;
    }
    row = lm_table_row_new(lm_lmx_message_thread, source_row);
    if (row == 0) {
        return 1;
    }
    index = 0U;
    while (index < lm_table_row_cell_count(lm_lmx_message_thread, source_row)) {
        source_cell = lm_table_row_cell_at(lm_lmx_message_thread, source_row, index);
        if (source_cell == 0) {
            lm_table_row_delete_any(row);
            return 1;
        }
        if (source_cell -> atom == 0 && source_cell -> value == 0) {
            cell = lm_table_cell_new_cstr(lm_lmx_message_thread, 0, 0, source_cell -> node, source_cell -> source, source_cell -> explicit_none);
            if (cell == 0 || lm_table_row_take_cell(lm_lmx_message_thread, row, cell) != 0) {
                lm_table_cell_delete_any(cell);
                lm_table_row_delete_any(row);
                return 1;
            }
        }
        else {
            atom_length = 0U;
            value_length = 0U;
            if (source_cell -> atom != 0) {
                atom_length = strlen(source_cell -> atom);
            }
            if (source_cell -> value != 0) {
                value_length = strlen(source_cell -> value);
            }
            if (lm_table_row_take_cell_slice(lm_lmx_message_thread, row, source_cell -> atom, atom_length, source_cell -> value, value_length, source_cell -> node, source_cell -> source, source_cell -> explicit_none) != 0) {
                lm_table_row_delete_any(row);
                return 1;
            }
        }
        index = index + 1U;
    }
    if (lm_table_descriptor_take_materialized_row(lm_lmx_message_thread, table, row) != 0) {
        lm_table_row_delete_any(row);
        return 1;
    }
    return 0;
}

static int lm_table_descriptor_schema_same(struct LmMessageThread *lm_lmx_message_thread, const LmTableDescriptor *left, const LmTableDescriptor *right) {
    (void)lm_lmx_message_thread;
    const LmTableColumnDescriptor * left_column;
    const LmTableColumnDescriptor * right_column;
    const char *left_descriptor;
    const char *right_descriptor;
    size_t column_index;
    size_t descriptor_index;
    if (left == 0 || right == 0 || lm_table_descriptor_column_count(lm_lmx_message_thread, left) != lm_table_descriptor_column_count(lm_lmx_message_thread, right)) {
        return 0;
    }
    column_index = 0U;
    while (column_index < lm_table_descriptor_column_count(lm_lmx_message_thread, left)) {
        left_column = lm_table_descriptor_column_at(lm_lmx_message_thread, left, column_index);
        right_column = lm_table_descriptor_column_at(lm_lmx_message_thread, right, column_index);
        if (left_column == 0 || right_column == 0 || left_column -> name == 0 || right_column -> name == 0 || strcmp(left_column -> name, right_column -> name) != 0 || left_column -> address_depth != right_column -> address_depth || left_column -> array_rank != right_column -> array_rank || left_column -> is_const != right_column -> is_const || lm_table_column_descriptor_descriptor_count(lm_lmx_message_thread, left_column) != lm_table_column_descriptor_descriptor_count(lm_lmx_message_thread, right_column)) {
            return 0;
        }
        if ((left_column -> type_name == 0) != (right_column -> type_name == 0)) {
            return 0;
        }
        if (left_column -> type_name != 0 && strcmp(left_column -> type_name, right_column -> type_name) != 0) {
            return 0;
        }
        descriptor_index = 0U;
        while (descriptor_index < lm_table_column_descriptor_descriptor_count(lm_lmx_message_thread, left_column)) {
            left_descriptor = lm_table_column_descriptor_descriptor_at(lm_lmx_message_thread, left_column, descriptor_index);
            right_descriptor = lm_table_column_descriptor_descriptor_at(lm_lmx_message_thread, right_column, descriptor_index);
            if (left_descriptor == 0 || right_descriptor == 0 || lm_table_column_descriptor_has_descriptor(lm_lmx_message_thread, right_column, left_descriptor) == 0 || strcmp(left_descriptor, right_descriptor) != 0) {
                return 0;
            }
            descriptor_index = descriptor_index + 1U;
        }
        column_index = column_index + 1U;
    }
    return 1;
}

static int lm_table_descriptor_join_schema_compatible(struct LmMessageThread *lm_lmx_message_thread, const LmTableDescriptor *source, const LmTableDescriptor *target) {
    (void)lm_lmx_message_thread;
    if (lm_table_descriptor_schema_same(lm_lmx_message_thread, source, target) != 0) {
        return 1;
    }
    return lm_table_descriptor_column_count(lm_lmx_message_thread, source) == 2U && lm_table_descriptor_column_count(lm_lmx_message_thread, target) == 2U;
}

static LmRegistryView * lm_registry_view_new(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *parent) {
    (void)lm_lmx_message_thread;
    LmRegistryView * view;
    view = lm_own_new_zero(sizeof(view[0]));
    if (view == 0) {
        return 0;
    }
    view->parent = parent;
    view->tables = lm_table_descriptor_stack_new(lm_lmx_message_thread, lm_table_descriptor_delete_any);
    view->source_tables = lm_table_descriptor_stack_new(lm_lmx_message_thread, lm_table_descriptor_delete_any);
    view->facts = lm_table_descriptor_stack_new(lm_lmx_message_thread, 0);
    view->class_names = lm_table_descriptor_stack_new(lm_lmx_message_thread, lm_own_delete_plain);
    if (view -> tables == 0 || view -> source_tables == 0 || view -> facts == 0 || view -> class_names == 0) {
        if (view -> tables != 0) {
            lm_own_ptr_stack_destroy(view -> tables);
            lm_own_delete(view -> tables, 0);
        }
        if (view -> facts != 0) {
            lm_own_ptr_stack_destroy(view -> facts);
            lm_own_delete(view -> facts, 0);
        }
        if (view -> source_tables != 0) {
            lm_own_ptr_stack_destroy(view -> source_tables);
            lm_own_delete(view -> source_tables, 0);
        }
        if (view -> class_names != 0) {
            lm_own_ptr_stack_destroy(view -> class_names);
            lm_own_delete(view -> class_names, 0);
        }
        lm_own_delete(view, 0);
        return 0;
    }
    return view;
}

static void lm_registry_view_note_mutation(struct LmMessageThread *lm_lmx_message_thread, LmRegistryView *view) {
    (void)lm_lmx_message_thread;
    if (view != 0) {
        view->mutation_generation = view -> mutation_generation + 1U;
    }
}

static void lm_registry_view_delete(struct LmMessageThread *lm_lmx_message_thread, LmRegistryView *view) {
    (void)lm_lmx_message_thread;
    if (view == 0) {
        return;
    }
    if (view -> facts != 0) {
        lm_own_ptr_stack_destroy(view -> facts);
        lm_own_delete(view -> facts, 0);
        view->facts = 0;
    }
    if (view -> tables != 0) {
        lm_own_ptr_stack_destroy(view -> tables);
        lm_own_delete(view -> tables, 0);
        view->tables = 0;
    }
    if (view -> source_tables != 0) {
        lm_own_ptr_stack_destroy(view -> source_tables);
        lm_own_delete(view -> source_tables, 0);
        view->source_tables = 0;
    }
    if (view -> class_names != 0) {
        lm_own_ptr_stack_destroy(view -> class_names);
        lm_own_delete(view -> class_names, 0);
        view->class_names = 0;
    }
    view->parent = 0;
    view->local_fact_count = 0U;
    lm_own_delete(view, 0);
}

static int lm_registry_view_class_has_slice(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view, const char *name, size_t name_length) {
    (void)lm_lmx_message_thread;
    size_t index;
    const char *candidate;
    if (view == 0 || name == 0) {
        return 0;
    }
    if (view -> class_names != 0) {
        index = 0U;
        while (index < view -> class_names -> count) {
            candidate = lm_own_ptr_stack_at(view -> class_names, index);
            if (candidate != 0 && lm_table_descriptor_cstr_equals_slice(lm_lmx_message_thread, candidate, name, name_length) != 0) {
                return 1;
            }
            index = index + 1U;
        }
    }
    return lm_registry_view_class_has_slice(lm_lmx_message_thread, view -> parent, name, name_length);
}

static int lm_registry_view_class_has(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view, const char *name) {
    (void)lm_lmx_message_thread;
    if (name == 0) {
        return 0;
    }
    return lm_registry_view_class_has_slice(lm_lmx_message_thread, view, name, strlen(name));
}

static int lm_registry_view_class_add(struct LmMessageThread *lm_lmx_message_thread, LmRegistryView *view, const char *name) {
    (void)lm_lmx_message_thread;
    char *copy;
    if (view == 0 || view -> class_names == 0 || name == 0) {
        return 1;
    }
    if (lm_registry_view_class_has(lm_lmx_message_thread, view, name) != 0) {
        return 0;
    }
    copy = lm_table_descriptor_copy_cstr(lm_lmx_message_thread, name);
    if (copy == 0) {
        return 1;
    }
    if (lm_own_ptr_stack_push(view -> class_names, copy) != 0) {
        lm_own_delete(copy, 0);
        return 1;
    }
    lm_registry_view_note_mutation(lm_lmx_message_thread, view);
    return 0;
}

static size_t lm_registry_view_class_count(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view) {
    (void)lm_lmx_message_thread;
    size_t count;
    size_t index;
    const LmRegistryView * current;
    const char *candidate;
    count = 0U;
    current = view;
    while (current != 0) {
        if (current -> class_names != 0) {
            index = 0U;
            while (index < current -> class_names -> count) {
                candidate = lm_own_ptr_stack_at(current -> class_names, index);
                if (candidate != 0 && lm_registry_view_class_has(lm_lmx_message_thread, current -> parent, candidate) == 0) {
                    count = count + 1U;
                }
                index = index + 1U;
            }
        }
        current = current -> parent;
    }
    return count;
}

static const char * lm_registry_view_class_at(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view, size_t index) {
    (void)lm_lmx_message_thread;
    size_t parent_count;
    size_t local_index;
    const LmRegistryView * current;
    const char *candidate;
    current = view;
    while (current != 0) {
        parent_count = lm_registry_view_class_count(lm_lmx_message_thread, current -> parent);
        if (index < parent_count) {
            current = current -> parent;
            continue;
        }
        index = index - parent_count;
        if (current -> class_names == 0) {
            return 0;
        }
        local_index = 0U;
        while (local_index < current -> class_names -> count) {
            candidate = lm_own_ptr_stack_at(current -> class_names, local_index);
            if (candidate != 0 && lm_registry_view_class_has(lm_lmx_message_thread, current -> parent, candidate) == 0) {
                if (index == 0U) {
                    return candidate;
                }
                index = index - 1U;
            }
            local_index = local_index + 1U;
        }
        return 0;
    }
    return 0;
}

static LmTableDescriptor * lm_registry_view_find_local_table_slice(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view, const char *table, size_t table_length) {
    (void)lm_lmx_message_thread;
    size_t index;
    LmTableDescriptor * descriptor;
    if (view == 0 || view -> tables == 0 || table == 0) {
        return 0;
    }
    index = 0U;
    while (index < view -> tables -> count) {
        descriptor = lm_own_ptr_stack_at(view -> tables, index);
        if (descriptor != 0 && descriptor -> name != 0 && lm_table_descriptor_cstr_equals_slice(lm_lmx_message_thread, descriptor -> name, table, table_length) != 0) {
            return descriptor;
        }
        index = index + 1U;
    }
    return 0;
}

static LmTableDescriptor * lm_registry_view_ensure_local_table_slice(struct LmMessageThread *lm_lmx_message_thread, LmRegistryView *view, const char *table, size_t table_length) {
    (void)lm_lmx_message_thread;
    LmTableDescriptor * descriptor;
    descriptor = lm_registry_view_find_local_table_slice(lm_lmx_message_thread, view, table, table_length);
    if (descriptor != 0) {
        return descriptor;
    }
    if (view == 0 || view -> tables == 0) {
        return 0;
    }
    descriptor = lm_table_descriptor_new_slice(lm_lmx_message_thread, table, table_length);
    if (descriptor == 0) {
        return 0;
    }
    if (lm_own_ptr_stack_push(view -> tables, descriptor) != 0) {
        lm_table_descriptor_delete_any(descriptor);
        return 0;
    }
    return descriptor;
}

static LmTableDescriptor * lm_registry_view_find_local_source_table_slice(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view, const char *table, size_t table_length) {
    (void)lm_lmx_message_thread;
    size_t index;
    LmTableDescriptor * descriptor;
    if (view == 0 || view -> source_tables == 0 || table == 0) {
        return 0;
    }
    index = view -> source_tables -> count;
    while (index > 0U) {
        index = index - 1U;
        descriptor = lm_own_ptr_stack_at(view -> source_tables, index);
        if (descriptor != 0 && descriptor -> name != 0 && lm_table_descriptor_cstr_equals_slice(lm_lmx_message_thread, descriptor -> name, table, table_length) != 0) {
            return descriptor;
        }
    }
    return 0;
}

static LmTableDescriptor * lm_registry_view_find_source_table_slice(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view, const char *table, size_t table_length) {
    (void)lm_lmx_message_thread;
    LmTableDescriptor * descriptor;
    if (view == 0) {
        return 0;
    }
    descriptor = lm_registry_view_find_local_source_table_slice(lm_lmx_message_thread, view, table, table_length);
    if (descriptor != 0) {
        return descriptor;
    }
    return lm_registry_view_find_source_table_slice(lm_lmx_message_thread, view -> parent, table, table_length);
}

static int lm_registry_view_take_local_source_table(struct LmMessageThread *lm_lmx_message_thread, LmRegistryView *view, LmTableDescriptor *descriptor) {
    (void)lm_lmx_message_thread;
    if (view == 0 || view -> source_tables == 0 || descriptor == 0 || descriptor -> name == 0) {
        return 1;
    }
    if (lm_own_ptr_stack_push(view -> source_tables, descriptor) != 0) {
        return 1;
    }
    lm_registry_view_note_mutation(lm_lmx_message_thread, view);
    return 0;
}

static size_t lm_registry_view_source_table_count(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view) {
    (void)lm_lmx_message_thread;
    size_t count;
    if (view == 0) {
        return 0U;
    }
    count = 0U;
    if (view -> source_tables != 0) {
        count = view -> source_tables -> count;
    }
    if (view -> parent != 0) {
        count = count + lm_registry_view_source_table_count(lm_lmx_message_thread, view -> parent);
    }
    return count;
}

static const LmTableDescriptor * lm_registry_view_source_table_at(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view, size_t index) {
    (void)lm_lmx_message_thread;
    size_t parent_count;
    if (view == 0) {
        return 0;
    }
    parent_count = lm_registry_view_source_table_count(lm_lmx_message_thread, view -> parent);
    if (index < parent_count) {
        return lm_registry_view_source_table_at(lm_lmx_message_thread, view -> parent, index);
    }
    if (view -> source_tables == 0) {
        return 0;
    }
    return lm_own_ptr_stack_at(view -> source_tables, index - parent_count);
}

static int lm_table_descriptor_source_path_column_matches(struct LmMessageThread *lm_lmx_message_thread, const LmTableDescriptor *descriptor, size_t column_index, const char *path, size_t path_length) {
    (void)lm_lmx_message_thread;
    const LmTableColumnDescriptor * column;
    size_t table_name_length;
    size_t column_count;
    size_t column_name_length;
    if (descriptor == 0 || descriptor -> name == 0 || path == 0) {
        return 0;
    }
    column_count = lm_table_descriptor_column_count(lm_lmx_message_thread, descriptor);
    if (column_index == 0U || column_index >= column_count) {
        return 0;
    }
    column = lm_table_descriptor_column_at(lm_lmx_message_thread, descriptor, column_index);
    if (column == 0 || column -> name == 0) {
        return 0;
    }
    if (column_count == 2U) {
        return lm_table_descriptor_cstr_equals_slice(lm_lmx_message_thread, descriptor -> name, path, path_length);
    }
    table_name_length = strlen(descriptor -> name);
    column_name_length = strlen(column -> name);
    if (path_length != table_name_length + 1U + column_name_length) {
        return 0;
    }
    if (memcmp(path, descriptor -> name, table_name_length) != 0 || path[table_name_length] != '.') {
        return 0;
    }
    return memcmp(path + table_name_length + 1U, column -> name, column_name_length) == 0;
}

static int lm_registry_view_source_path_has_rows_slice_mode(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view, const char *path, size_t path_length, int projected_only, int *out_covered) {
    (void)lm_lmx_message_thread;
    const LmTableDescriptor * descriptor;
    const LmTableRow * row;
    const LmTableCell * cell;
    size_t table_count;
    size_t table_index;
    size_t column_count;
    size_t column_index;
    size_t row_count;
    size_t row_index;
    if (out_covered != 0) {
        out_covered[0] = 0;
    }
    if (view == 0 || path == 0 || out_covered == 0) {
        return 0;
    }
    table_count = lm_registry_view_source_table_count(lm_lmx_message_thread, view);
    table_index = 0U;
    while (table_index < table_count) {
        descriptor = lm_registry_view_source_table_at(lm_lmx_message_thread, view, table_index);
        if (descriptor != 0 && descriptor -> name != 0) {
            column_count = lm_table_descriptor_column_count(lm_lmx_message_thread, descriptor);
            column_index = 1U;
            while (column_index < column_count) {
                if (lm_table_descriptor_source_path_column_matches(lm_lmx_message_thread, descriptor, column_index, path, path_length) != 0) {
                    out_covered[0] = 1;
                    row_count = lm_table_descriptor_materialized_row_count(lm_lmx_message_thread, descriptor);
                    row_index = 0U;
                    while (row_index < row_count) {
                        row = lm_table_descriptor_materialized_row_at(lm_lmx_message_thread, descriptor, row_index);
                        if (row != 0 && (projected_only == 0 || row -> source_native == 0)) {
                            cell = lm_table_row_cell_at(lm_lmx_message_thread, row, column_index);
                            if (cell != 0 && (cell -> value != 0 || cell -> node != 0)) {
                                return 1;
                            }
                        }
                        row_index = row_index + 1U;
                    }
                }
                column_index = column_index + 1U;
            }
        }
        table_index = table_index + 1U;
    }
    return 0;
}

static int lm_registry_view_source_path_has_rows_slice(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view, const char *path, size_t path_length, int *out_covered) {
    (void)lm_lmx_message_thread;
    return lm_registry_view_source_path_has_rows_slice_mode(lm_lmx_message_thread, view, path, path_length, 0, out_covered);
}

static int lm_registry_view_source_path_has_rows(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view, const char *path, int *out_covered) {
    (void)lm_lmx_message_thread;
    if (path == 0) {
        if (out_covered != 0) {
            out_covered[0] = 0;
        }
        return 0;
    }
    return lm_registry_view_source_path_has_rows_slice(lm_lmx_message_thread, view, path, strlen(path), out_covered);
}

static int lm_registry_view_source_path_has_key_slice_mode(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view, const char *path, size_t path_length, const char *key, size_t key_length, int projected_only, int *out_covered) {
    (void)lm_lmx_message_thread;
    const LmTableDescriptor * descriptor;
    const LmTableRow * row;
    const LmTableCell * key_cell;
    const LmTableCell * payload_cell;
    size_t table_count;
    size_t table_index;
    size_t column_count;
    size_t column_index;
    size_t row_count;
    size_t row_index;
    if (out_covered != 0) {
        out_covered[0] = 0;
    }
    if (view == 0 || path == 0 || key == 0 || out_covered == 0) {
        return 0;
    }
    table_count = lm_registry_view_source_table_count(lm_lmx_message_thread, view);
    table_index = 0U;
    while (table_index < table_count) {
        descriptor = lm_registry_view_source_table_at(lm_lmx_message_thread, view, table_index);
        column_count = lm_table_descriptor_column_count(lm_lmx_message_thread, descriptor);
        column_index = 1U;
        while (column_index < column_count) {
            if (lm_table_descriptor_source_path_column_matches(lm_lmx_message_thread, descriptor, column_index, path, path_length) != 0) {
                out_covered[0] = 1;
                row_count = lm_table_descriptor_materialized_row_count(lm_lmx_message_thread, descriptor);
                row_index = 0U;
                while (row_index < row_count) {
                    row = lm_table_descriptor_materialized_row_at(lm_lmx_message_thread, descriptor, row_index);
                    if (row != 0 && (projected_only == 0 || row -> source_native == 0)) {
                        key_cell = lm_table_row_cell_at(lm_lmx_message_thread, row, 0U);
                        payload_cell = lm_table_row_cell_at(lm_lmx_message_thread, row, column_index);
                        if (key_cell != 0 && key_cell -> value != 0 && lm_table_descriptor_cstr_equals_slice(lm_lmx_message_thread, key_cell -> value, key, key_length) != 0 && payload_cell != 0 && (payload_cell -> value != 0 || payload_cell -> node != 0)) {
                            return 1;
                        }
                    }
                    row_index = row_index + 1U;
                }
            }
            column_index = column_index + 1U;
        }
        table_index = table_index + 1U;
    }
    return 0;
}

static int lm_registry_view_source_path_has_key_slice(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view, const char *path, size_t path_length, const char *key, size_t key_length, int *out_covered) {
    (void)lm_lmx_message_thread;
    return lm_registry_view_source_path_has_key_slice_mode(lm_lmx_message_thread, view, path, path_length, key, key_length, 0, out_covered);
}

static const LmTableCell * lm_registry_view_source_path_cell_at_slice_mode(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view, const char *path, size_t path_length, size_t index, int projected_only, const LmTableDescriptor **out_descriptor, size_t *out_column_index, const LmTableCell **out_key_cell, int *out_covered) {
    (void)lm_lmx_message_thread;
    const LmTableDescriptor * descriptor;
    const LmTableRow * row;
    const LmTableCell * key_cell;
    const LmTableCell * payload_cell;
    size_t table_count;
    size_t table_index;
    size_t column_count;
    size_t column_index;
    size_t row_count;
    size_t row_index;
    size_t fact_index;
    int descriptor_covered;
    if (out_descriptor != 0) {
        out_descriptor[0] = 0;
    }
    if (out_column_index != 0) {
        out_column_index[0] = 0U;
    }
    if (out_key_cell != 0) {
        out_key_cell[0] = 0;
    }
    if (out_covered != 0) {
        out_covered[0] = 0;
    }
    if (view == 0 || path == 0 || out_covered == 0) {
        return 0;
    }
    fact_index = 0U;
    table_count = lm_registry_view_source_table_count(lm_lmx_message_thread, view);
    table_index = 0U;
    while (table_index < table_count) {
        descriptor = lm_registry_view_source_table_at(lm_lmx_message_thread, view, table_index);
        column_count = lm_table_descriptor_column_count(lm_lmx_message_thread, descriptor);
        descriptor_covered = 0;
        column_index = 1U;
        while (column_index < column_count) {
            if (lm_table_descriptor_source_path_column_matches(lm_lmx_message_thread, descriptor, column_index, path, path_length) != 0) {
                descriptor_covered = 1;
                out_covered[0] = 1;
            }
            column_index = column_index + 1U;
        }
        if (descriptor_covered != 0) {
            row_count = lm_table_descriptor_materialized_row_count(lm_lmx_message_thread, descriptor);
            row_index = 0U;
            while (row_index < row_count) {
                row = lm_table_descriptor_materialized_row_at(lm_lmx_message_thread, descriptor, row_index);
                if (row != 0 && (projected_only == 0 || row -> source_native == 0)) {
                    key_cell = lm_table_row_cell_at(lm_lmx_message_thread, row, 0U);
                    column_index = 1U;
                    while (column_index < column_count) {
                        if (lm_table_descriptor_source_path_column_matches(lm_lmx_message_thread, descriptor, column_index, path, path_length) != 0) {
                            payload_cell = lm_table_row_cell_at(lm_lmx_message_thread, row, column_index);
                            if (key_cell != 0 && key_cell -> value != 0 && payload_cell != 0 && (payload_cell -> value != 0 || payload_cell -> node != 0)) {
                                if (fact_index == index) {
                                    if (out_descriptor != 0) {
                                        out_descriptor[0] = descriptor;
                                    }
                                    if (out_column_index != 0) {
                                        out_column_index[0] = column_index;
                                    }
                                    if (out_key_cell != 0) {
                                        out_key_cell[0] = key_cell;
                                    }
                                    return payload_cell;
                                }
                                fact_index = fact_index + 1U;
                            }
                        }
                        column_index = column_index + 1U;
                    }
                }
                row_index = row_index + 1U;
            }
        }
        table_index = table_index + 1U;
    }
    return 0;
}

static const LmTableCell * lm_registry_view_source_path_cell_at_slice(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view, const char *path, size_t path_length, size_t index, const LmTableDescriptor **out_descriptor, size_t *out_column_index, const LmTableCell **out_key_cell, int *out_covered) {
    (void)lm_lmx_message_thread;
    return lm_registry_view_source_path_cell_at_slice_mode(lm_lmx_message_thread, view, path, path_length, index, 0, out_descriptor, out_column_index, out_key_cell, out_covered);
}

static const char * lm_registry_view_source_path_key_at_slice_mode(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view, const char *path, size_t path_length, size_t index, int projected_only, int *out_covered) {
    (void)lm_lmx_message_thread;
    const LmTableCell * key_cell;
    key_cell = 0;
    if (projected_only == 0) {
        if (lm_registry_view_source_path_cell_at_slice(lm_lmx_message_thread, view, path, path_length, index, 0, 0, &key_cell, out_covered) == 0 || key_cell == 0) {
            return 0;
        }
    }
    else {
        if (lm_registry_view_source_path_cell_at_slice_mode(lm_lmx_message_thread, view, path, path_length, index, 1, 0, 0, &key_cell, out_covered) == 0 || key_cell == 0) {
            return 0;
        }
    }
    return key_cell -> value;
}

static const char * lm_registry_view_source_path_key_at_slice(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view, const char *path, size_t path_length, size_t index, int *out_covered) {
    (void)lm_lmx_message_thread;
    return lm_registry_view_source_path_key_at_slice_mode(lm_lmx_message_thread, view, path, path_length, index, 0, out_covered);
}

static int lm_registry_view_append_materialized_rows_mode(struct LmMessageThread *lm_lmx_message_thread, LmRegistryView *view, LmTableDescriptor *target, const char *source_table, size_t source_table_length, int note_mutation, int require_compatible_source, size_t snapshot_source_count, const size_t *snapshot_row_counts) {
    (void)lm_lmx_message_thread;
    const LmTableDescriptor * source;
    const LmTableRow * row;
    size_t source_count;
    size_t row_count;
    size_t original_row_count;
    size_t source_index;
    size_t index;
    int matched;
    int incompatible;
    if (view == 0 || target == 0 || source_table == 0) {
        return 1;
    }
    source = lm_registry_view_find_source_table_slice(lm_lmx_message_thread, view, source_table, source_table_length);
    if (source == 0) {
        return require_compatible_source != 0;
    }
    original_row_count = lm_table_descriptor_materialized_row_count(lm_lmx_message_thread, target);
    source_count = lm_registry_view_source_table_count(lm_lmx_message_thread, view);
    if (snapshot_row_counts != 0) {
        if (snapshot_source_count > source_count) {
            return 1;
        }
        source_count = snapshot_source_count;
    }
    source_index = 0U;
    matched = 0;
    incompatible = 0;
    while (source_index < source_count) {
        source = lm_registry_view_source_table_at(lm_lmx_message_thread, view, source_index);
        if (source != 0 && source -> name != 0 && lm_table_descriptor_cstr_equals_slice(lm_lmx_message_thread, source -> name, source_table, source_table_length) != 0) {
            if (lm_table_descriptor_join_schema_compatible(lm_lmx_message_thread, source, target) == 0) {
                incompatible = 1;
            }
            else {
                matched = 1;
                row_count = lm_table_descriptor_materialized_row_count(lm_lmx_message_thread, source);
                if (snapshot_row_counts != 0) {
                    if (snapshot_row_counts[source_index] > row_count) {
                        lm_own_ptr_stack_truncate(target -> materialized_rows, original_row_count);
                        return 1;
                    }
                    row_count = snapshot_row_counts[source_index];
                }
                else {
                    if (source == target) {
                        row_count = original_row_count;
                    }
                }
                index = 0U;
                while (index < row_count) {
                    row = lm_table_descriptor_materialized_row_at(lm_lmx_message_thread, source, index);
                    if (row == 0 || lm_table_descriptor_append_materialized_row_copy(lm_lmx_message_thread, target, row) != 0) {
                        lm_own_ptr_stack_truncate(target -> materialized_rows, original_row_count);
                        return 1;
                    }
                    index = index + 1U;
                }
            }
        }
        source_index = source_index + 1U;
    }
    if (require_compatible_source != 0 && (matched == 0 || incompatible != 0)) {
        lm_own_ptr_stack_truncate(target -> materialized_rows, original_row_count);
        return 1;
    }
    if (note_mutation != 0 && lm_table_descriptor_materialized_row_count(lm_lmx_message_thread, target) != original_row_count) {
        lm_registry_view_note_mutation(lm_lmx_message_thread, view);
    }
    return 0;
}

static int lm_registry_view_append_materialized_rows(struct LmMessageThread *lm_lmx_message_thread, LmRegistryView *view, LmTableDescriptor *target, const char *source_table, size_t source_table_length) {
    (void)lm_lmx_message_thread;
    return lm_registry_view_append_materialized_rows_mode(lm_lmx_message_thread, view, target, source_table, source_table_length, 1, 0, 0U, 0);
}

static size_t lm_registry_view_fact_count(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view) {
    (void)lm_lmx_message_thread;
    size_t count;
    if (view == 0) {
        return 0U;
    }
    count = view -> local_fact_count;
    if (view -> parent != 0) {
        count = count + lm_registry_view_fact_count(lm_lmx_message_thread, view -> parent);
    }
    return count;
}

static const LmRegistryViewRow * lm_registry_view_fact_at(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view, size_t index) {
    (void)lm_lmx_message_thread;
    size_t parent_count;
    if (view == 0) {
        return 0;
    }
    parent_count = lm_registry_view_fact_count(lm_lmx_message_thread, view -> parent);
    if (index < parent_count) {
        return lm_registry_view_fact_at(lm_lmx_message_thread, view -> parent, index);
    }
    if (view -> facts == 0) {
        return 0;
    }
    return lm_own_ptr_stack_at(view -> facts, index - parent_count);
}

static int lm_registry_view_push_relation(struct LmMessageThread *lm_lmx_message_thread, LmRegistryView *view, const char *table, const char *key, const char *payload, const void *payload_node, const void *source) {
    (void)lm_lmx_message_thread;
    LmTableDescriptor * descriptor;
    LmRegistryViewRow * row;
    void *popped;
    if (view == 0 || table == 0 || key == 0 || (payload == 0 && payload_node == 0)) {
        return 1;
    }
    descriptor = lm_registry_view_ensure_local_table_slice(lm_lmx_message_thread, view, table, strlen(table));
    if (descriptor == 0 || descriptor -> rows == 0 || view -> facts == 0) {
        return 1;
    }
    row = lm_own_new_zero(sizeof(row[0]));
    if (row == 0) {
        return 1;
    }
    row->table = lm_table_descriptor_copy_cstr(lm_lmx_message_thread, table);
    row->key = lm_table_descriptor_copy_cstr(lm_lmx_message_thread, key);
    if (payload != 0) {
        row->payload = lm_table_descriptor_copy_cstr(lm_lmx_message_thread, payload);
    }
    row->payload_node = payload_node;
    row->source = source;
    row->local_sequence = view -> local_fact_count;
    if (row -> table == 0 || row -> key == 0 || (payload != 0 && row -> payload == 0)) {
        lm_registry_view_row_delete_any(row);
        return 1;
    }
    if (lm_own_ptr_stack_push(descriptor -> rows, row) != 0) {
        lm_registry_view_row_delete_any(row);
        return 1;
    }
    if (lm_own_ptr_stack_push(descriptor -> source_rows, (((void *)source))) != 0) {
        popped = lm_own_ptr_stack_pop(descriptor -> rows);
        lm_registry_view_row_delete_any(popped);
        return 1;
    }
    if (lm_own_ptr_stack_push(view -> facts, row) != 0) {
        popped = lm_own_ptr_stack_pop(descriptor -> source_rows);
        popped = lm_own_ptr_stack_pop(descriptor -> rows);
        lm_registry_view_row_delete_any(popped);
        return 1;
    }
    if (payload != 0 && strcmp(table, "class.present") == 0 && lm_registry_view_class_add(lm_lmx_message_thread, view, key) != 0) {
        popped = lm_own_ptr_stack_pop(view -> facts);
        popped = lm_own_ptr_stack_pop(descriptor -> source_rows);
        popped = lm_own_ptr_stack_pop(descriptor -> rows);
        lm_registry_view_row_delete_any(popped);
        return 1;
    }
    view->local_fact_count = view -> local_fact_count + 1U;
    lm_registry_view_note_mutation(lm_lmx_message_thread, view);
    return 0;
}

static const LmRegistryViewRow * lm_registry_view_lookup_local_exact_slice(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view, const char *key, size_t key_length, const char *table, size_t table_length) {
    (void)lm_lmx_message_thread;
    LmTableDescriptor * descriptor;
    LmRegistryViewRow * row;
    size_t index;
    descriptor = lm_registry_view_find_local_table_slice(lm_lmx_message_thread, view, table, table_length);
    if (descriptor == 0 || descriptor -> rows == 0) {
        return 0;
    }
    index = descriptor -> rows -> count;
    while (index > 0U) {
        index = index - 1U;
        row = lm_own_ptr_stack_at(descriptor -> rows, index);
        if (row != 0 && row -> key != 0 && lm_table_descriptor_cstr_equals_slice(lm_lmx_message_thread, row -> key, key, key_length) != 0) {
            return row;
        }
    }
    return 0;
}

static const LmRegistryViewRow * lm_registry_view_lookup_exact_slice(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view, const char *key, size_t key_length, const char *table, size_t table_length) {
    (void)lm_lmx_message_thread;
    const LmRegistryViewRow * row;
    if (view == 0 || key == 0 || table == 0) {
        return 0;
    }
    row = lm_registry_view_lookup_local_exact_slice(lm_lmx_message_thread, view, key, key_length, table, table_length);
    if (row != 0) {
        return row;
    }
    return lm_registry_view_lookup_exact_slice(lm_lmx_message_thread, view -> parent, key, key_length, table, table_length);
}

static const LmRegistryViewRow * lm_registry_view_lookup_exact(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view, const char *key, const char *table) {
    (void)lm_lmx_message_thread;
    if (key == 0 || table == 0) {
        return 0;
    }
    return lm_registry_view_lookup_exact_slice(lm_lmx_message_thread, view, key, strlen(key), table, strlen(table));
}

static const LmRegistryViewRow * lm_registry_view_lookup_default_slice(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view, const char *table, size_t table_length) {
    (void)lm_lmx_message_thread;
    return lm_registry_view_lookup_exact_slice(lm_lmx_message_thread, view, "default", 7U, table, table_length);
}

static const LmRegistryViewRow * lm_registry_view_lookup_default(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view, const char *table) {
    (void)lm_lmx_message_thread;
    if (table == 0) {
        return 0;
    }
    return lm_registry_view_lookup_default_slice(lm_lmx_message_thread, view, table, strlen(table));
}

static const LmRegistryViewRow * lm_registry_view_lookup_slice(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view, const char *key, size_t key_length, const char *table, size_t table_length) {
    (void)lm_lmx_message_thread;
    const LmRegistryViewRow * row;
    row = lm_registry_view_lookup_exact_slice(lm_lmx_message_thread, view, key, key_length, table, table_length);
    if (row != 0) {
        return row;
    }
    return lm_registry_view_lookup_default_slice(lm_lmx_message_thread, view, table, table_length);
}

static const LmRegistryViewRow * lm_registry_view_lookup(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view, const char *key, const char *table) {
    (void)lm_lmx_message_thread;
    if (key == 0 || table == 0) {
        return 0;
    }
    return lm_registry_view_lookup_slice(lm_lmx_message_thread, view, key, strlen(key), table, strlen(table));
}

static const LmRegistryViewRow * lm_registry_view_lookup_text_slice(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view, const char *key, size_t key_length, const char *table, size_t table_length) {
    (void)lm_lmx_message_thread;
    const LmRegistryViewRow * row;
    row = lm_registry_view_lookup_exact_slice(lm_lmx_message_thread, view, key, key_length, table, table_length);
    if (row != 0 && row -> payload != 0) {
        return row;
    }
    return lm_registry_view_lookup_default_slice(lm_lmx_message_thread, view, table, table_length);
}

static const LmRegistryViewRow * lm_registry_view_lookup_text(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view, const char *key, const char *table) {
    (void)lm_lmx_message_thread;
    if (key == 0 || table == 0) {
        return 0;
    }
    return lm_registry_view_lookup_text_slice(lm_lmx_message_thread, view, key, strlen(key), table, strlen(table));
}

static size_t lm_registry_view_local_table_row_count_slice(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view, const char *table, size_t table_length) {
    (void)lm_lmx_message_thread;
    LmTableDescriptor * descriptor;
    descriptor = lm_registry_view_find_local_table_slice(lm_lmx_message_thread, view, table, table_length);
    return lm_table_descriptor_row_count(lm_lmx_message_thread, descriptor);
}

static size_t lm_registry_view_table_row_count_slice(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view, const char *table, size_t table_length) {
    (void)lm_lmx_message_thread;
    size_t count;
    if (view == 0 || table == 0) {
        return 0U;
    }
    count = lm_registry_view_local_table_row_count_slice(lm_lmx_message_thread, view, table, table_length);
    if (view -> parent != 0) {
        count = count + lm_registry_view_table_row_count_slice(lm_lmx_message_thread, view -> parent, table, table_length);
    }
    return count;
}

static size_t lm_registry_view_table_row_count(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view, const char *table) {
    (void)lm_lmx_message_thread;
    if (table == 0) {
        return 0U;
    }
    return lm_registry_view_table_row_count_slice(lm_lmx_message_thread, view, table, strlen(table));
}

static const LmRegistryViewRow * lm_registry_view_table_row_at_slice(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view, const char *table, size_t table_length, size_t index) {
    (void)lm_lmx_message_thread;
    size_t parent_count;
    LmTableDescriptor * descriptor;
    if (view == 0 || table == 0) {
        return 0;
    }
    parent_count = lm_registry_view_table_row_count_slice(lm_lmx_message_thread, view -> parent, table, table_length);
    if (index < parent_count) {
        return lm_registry_view_table_row_at_slice(lm_lmx_message_thread, view -> parent, table, table_length, index);
    }
    descriptor = lm_registry_view_find_local_table_slice(lm_lmx_message_thread, view, table, table_length);
    return lm_table_descriptor_row_at(lm_lmx_message_thread, descriptor, index - parent_count);
}

static const LmRegistryViewRow * lm_registry_view_table_row_at(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view, const char *table, size_t index) {
    (void)lm_lmx_message_thread;
    if (table == 0) {
        return 0;
    }
    return lm_registry_view_table_row_at_slice(lm_lmx_message_thread, view, table, strlen(table), index);
}

static size_t lm_table_descriptor_matching_key_count_slice(struct LmMessageThread *lm_lmx_message_thread, const LmTableDescriptor *descriptor, const char *key, size_t key_length) {
    (void)lm_lmx_message_thread;
    size_t count;
    size_t index;
    const LmRegistryViewRow * row;
    count = 0U;
    index = 0U;
    while (index < lm_table_descriptor_row_count(lm_lmx_message_thread, descriptor)) {
        row = lm_table_descriptor_row_at(lm_lmx_message_thread, descriptor, index);
        if (row != 0 && row -> key != 0 && lm_table_descriptor_cstr_equals_slice(lm_lmx_message_thread, row -> key, key, key_length) != 0) {
            count = count + 1U;
        }
        index = index + 1U;
    }
    return count;
}

static const LmRegistryViewRow * lm_table_descriptor_matching_key_at_slice(struct LmMessageThread *lm_lmx_message_thread, const LmTableDescriptor *descriptor, const char *key, size_t key_length, size_t match_index) {
    (void)lm_lmx_message_thread;
    size_t index;
    const LmRegistryViewRow * row;
    index = 0U;
    while (index < lm_table_descriptor_row_count(lm_lmx_message_thread, descriptor)) {
        row = lm_table_descriptor_row_at(lm_lmx_message_thread, descriptor, index);
        if (row != 0 && row -> key != 0 && lm_table_descriptor_cstr_equals_slice(lm_lmx_message_thread, row -> key, key, key_length) != 0) {
            if (match_index == 0U) {
                return row;
            }
            match_index = match_index - 1U;
        }
        index = index + 1U;
    }
    return 0;
}

static size_t lm_registry_view_matching_key_count_slice(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view, const char *table, size_t table_length, const char *key, size_t key_length) {
    (void)lm_lmx_message_thread;
    size_t count;
    LmTableDescriptor * descriptor;
    if (view == 0) {
        return 0U;
    }
    count = lm_registry_view_matching_key_count_slice(lm_lmx_message_thread, view -> parent, table, table_length, key, key_length);
    descriptor = lm_registry_view_find_local_table_slice(lm_lmx_message_thread, view, table, table_length);
    return count + lm_table_descriptor_matching_key_count_slice(lm_lmx_message_thread, descriptor, key, key_length);
}

static size_t lm_registry_view_matching_key_count(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view, const char *table, const char *key) {
    (void)lm_lmx_message_thread;
    if (table == 0 || key == 0) {
        return 0U;
    }
    return lm_registry_view_matching_key_count_slice(lm_lmx_message_thread, view, table, strlen(table), key, strlen(key));
}

static const LmRegistryViewRow * lm_registry_view_matching_key_at_slice(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view, const char *table, size_t table_length, const char *key, size_t key_length, size_t match_index) {
    (void)lm_lmx_message_thread;
    size_t parent_count;
    LmTableDescriptor * descriptor;
    if (view == 0) {
        return 0;
    }
    parent_count = lm_registry_view_matching_key_count_slice(lm_lmx_message_thread, view -> parent, table, table_length, key, key_length);
    if (match_index < parent_count) {
        return lm_registry_view_matching_key_at_slice(lm_lmx_message_thread, view -> parent, table, table_length, key, key_length, match_index);
    }
    descriptor = lm_registry_view_find_local_table_slice(lm_lmx_message_thread, view, table, table_length);
    return lm_table_descriptor_matching_key_at_slice(lm_lmx_message_thread, descriptor, key, key_length, match_index - parent_count);
}

static const LmRegistryViewRow * lm_registry_view_matching_key_at(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view, const char *table, const char *key, size_t match_index) {
    (void)lm_lmx_message_thread;
    if (table == 0 || key == 0) {
        return 0;
    }
    return lm_registry_view_matching_key_at_slice(lm_lmx_message_thread, view, table, strlen(table), key, strlen(key), match_index);
}

static const LmOwnPtrStack * lm_registry_view_local_source_rows_slice(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view, const char *table, size_t table_length) {
    (void)lm_lmx_message_thread;
    LmTableDescriptor * descriptor;
    descriptor = lm_registry_view_find_local_table_slice(lm_lmx_message_thread, view, table, table_length);
    if (descriptor == 0) {
        return 0;
    }
    return descriptor -> source_rows;
}

static int lm_registry_view_table_has_rows(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view, const char *table) {
    (void)lm_lmx_message_thread;
    return lm_registry_view_table_row_count(lm_lmx_message_thread, view, table) != 0U;
}


static int lm_p0_text_equals(struct LmMessageThread *lm_lmx_message_thread, const LmP0Text *text, const char *value) {
    (void)lm_lmx_message_thread;
    size_t value_length;
    if (text == 0 || value == 0) {
        return 0;
    }
    value_length = strlen(value);
    return text -> length == value_length && memcmp(text -> data, value, value_length) == 0;
}

static int lm_p0_identifier_payload(struct LmMessageThread *lm_lmx_message_thread, const LmP0Text *atom, LmP0Text *out_payload) {
    (void)lm_lmx_message_thread;
    if (atom == 0 || out_payload == 0 || atom -> data == 0) {
        return 0;
    }
    out_payload->data = atom -> data;
    out_payload->length = atom -> length;
    if (atom -> length >= 2U && atom -> data[0] == '`' && atom -> data[atom -> length - 1U] == '`') {
        out_payload->data = atom -> data + 1U;
        out_payload->length = atom -> length - 2U;
    }
    return 1;
}

static int lm_p0_registry_identifier_value(struct LmMessageThread *lm_lmx_message_thread, const LmP0Text *atom, LmP0Text *out_payload) {
    (void)lm_lmx_message_thread;
    if (atom == 0 || out_payload == 0) {
        return 0;
    }
    if (atom -> length > 0U && (atom -> data[0] == '"' || atom -> data[0] == '\'')) {
        return 0;
    }
    return lm_p0_identifier_payload(lm_lmx_message_thread, atom, out_payload);
}

static int lm_p0_registry_literal_value(struct LmMessageThread *lm_lmx_message_thread, const LmP0Text *atom, LmP0Text *out_payload) {
    (void)lm_lmx_message_thread;
    char quote;
    if (atom == 0 || out_payload == 0 || atom -> data == 0) {
        return 0;
    }
    out_payload->data = atom -> data;
    out_payload->length = atom -> length;
    if (atom -> length < 2U) {
        return 1;
    }
    quote = atom -> data[0];
    if ((quote == '"' || quote == '\'') && atom -> data[atom -> length - 1U] == quote) {
        out_payload->data = atom -> data + 1U;
        out_payload->length = atom -> length - 2U;
        return 1;
    }
    return lm_p0_identifier_payload(lm_lmx_message_thread, atom, out_payload);
}

static int lm_p0_registry_payload_is_null(struct LmMessageThread *lm_lmx_message_thread, const LmP0Text *atom) {
    (void)lm_lmx_message_thread;
    LmP0Text * payload;
    int is_null;
    payload = lm_p0_text_view_new_cstr(lm_lmx_message_thread, "");
    if (payload == 0) {
        return 0;
    }
    if (lm_p0_registry_identifier_value(lm_lmx_message_thread, atom, payload) == 0) {
        lm_p0_text_view_delete(lm_lmx_message_thread, payload);
        return 0;
    }
    is_null = payload -> length == 4U && memcmp(payload -> data, "None", 4U) == 0;
    lm_p0_text_view_delete(lm_lmx_message_thread, payload);
    return is_null;
}

static int lm_p0_is_horizontal_space(struct LmMessageThread *lm_lmx_message_thread, char value) {
    (void)lm_lmx_message_thread;
    return value == ' ' || value == '\t';
}

static int lm_p0_is_line_break(struct LmMessageThread *lm_lmx_message_thread, char value) {
    (void)lm_lmx_message_thread;
    return value == '\n' || value == '\r';
}

static size_t lm_p0_line_break_width_at(struct LmMessageThread *lm_lmx_message_thread, const char *source, size_t length, size_t index) {
    (void)lm_lmx_message_thread;
    if (index >= length) {
        return 0U;
    }
    if (source[index] == '\r') {
        if (index + 1U < length && source[index + 1U] == '\n') {
            return 2U;
        }
        return 1U;
    }
    if (source[index] == '\n') {
        return 1U;
    }
    return 0U;
}

static int lm_p0_is_field_space(struct LmMessageThread *lm_lmx_message_thread, char value) {
    (void)lm_lmx_message_thread;
    return lm_p0_is_horizontal_space(lm_lmx_message_thread, value) || lm_p0_is_line_break(lm_lmx_message_thread, value);
}

static int lm_p0_is_field_separator(struct LmMessageThread *lm_lmx_message_thread, char value) {
    (void)lm_lmx_message_thread;
    return value == ',' || value == ';';
}

static int lm_p0_is_short_form_separator(struct LmMessageThread *lm_lmx_message_thread, char value) {
    (void)lm_lmx_message_thread;
    return value == ';';
}

static int lm_p0_is_quoted_token_boundary(struct LmMessageThread *lm_lmx_message_thread, char value) {
    (void)lm_lmx_message_thread;
    return lm_p0_is_field_space(lm_lmx_message_thread, value) || lm_p0_is_field_separator(lm_lmx_message_thread, value) || value == '(' || value == ')' || value == '#';
}

static int lm_p0_starts_python_string(struct LmMessageThread *lm_lmx_message_thread, const char *text, size_t length, size_t index) {
    (void)lm_lmx_message_thread;
    char quote;
    if (index + 2U >= length) {
        return 0;
    }
    quote = text[index];
    if (quote != '"' && quote != '\'') {
        return 0;
    }
    return text[index + 1U] == quote && text[index + 2U] == quote;
}

static int lm_p0_is_decimal_digit(struct LmMessageThread *lm_lmx_message_thread, char value) {
    (void)lm_lmx_message_thread;
    return value >= '0' && value <= '9';
}

static char * lm_p0_copy_bytes(struct LmMessageThread *lm_lmx_message_thread, const char *source, size_t length) {
    (void)lm_lmx_message_thread;
    return lm_own_copy_bytes(source, length);
}

static LmP0Text * lm_p0_text_view_new_cstr(struct LmMessageThread *lm_lmx_message_thread, const char *text) {
    (void)lm_lmx_message_thread;
    LmP0Text * result;
    result = lm_own_new_zero(1U * sizeof(LmP0Text));
    if (result == 0) {
        return 0;
    }
    if (text != 0) {
        result->data = text;
    }
    if (text == 0) {
        result->data = "";
    }
    result->length = strlen(result -> data);
    return result;
}

static void lm_p0_text_view_delete(struct LmMessageThread *lm_lmx_message_thread, LmP0Text *text) {
    (void)lm_lmx_message_thread;
    lm_own_delete(text, 0);
}

static LmP0Text * lm_p0_text_from_cstr(struct LmMessageThread *lm_lmx_message_thread, const char *text) {
    (void)lm_lmx_message_thread;
    return lm_p0_text_view_new_cstr(lm_lmx_message_thread, text);
}

static int lm_p0_document_init_owners(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document) {
    (void)lm_lmx_message_thread;
    if (document == 0) {
        return 1;
    }
    if (document -> owners_initialized != 0) {
        return 0;
    }
    document->source_owner = lm_own_new_zero(sizeof(LmOwnArena));
    document->token_arena = lm_own_new_zero(sizeof(LmOwnArena));
    document->tree_arena = lm_own_new_zero(sizeof(LmOwnArena));
    document->diagnostic_arena = lm_own_new_zero(sizeof(LmOwnArena));
    if (document -> source_owner == 0 || document -> token_arena == 0 || document -> tree_arena == 0 || document -> diagnostic_arena == 0) {
        lm_own_delete(document -> diagnostic_arena, 0);
        lm_own_delete(document -> tree_arena, 0);
        lm_own_delete(document -> token_arena, 0);
        lm_own_delete(document -> source_owner, 0);
        document->diagnostic_arena = 0;
        document->tree_arena = 0;
        document->token_arena = 0;
        document->source_owner = 0;
        return 1;
    }
    document->owners_initialized = 1;
    if (lm_own_arena_init(lm_lmx_message_thread, document -> source_owner, lm_lmx_message_thread) != 0 || lm_own_arena_init(lm_lmx_message_thread, document -> token_arena, lm_lmx_message_thread) != 0 || lm_own_arena_init(lm_lmx_message_thread, document -> tree_arena, lm_lmx_message_thread) != 0 || lm_own_arena_init(lm_lmx_message_thread, document -> diagnostic_arena, lm_lmx_message_thread) != 0) {
        lm_p0_document_destroy_owners(lm_lmx_message_thread, document);
        return 1;
    }
    document->diagnostic = lm_own_arena_new_zero(lm_lmx_message_thread, document -> diagnostic_arena, sizeof(document -> diagnostic[0]));
    if (document -> diagnostic == 0) {
        lm_p0_document_destroy_owners(lm_lmx_message_thread, document);
        return 1;
    }
    return 0;
}

static int lm_p0_document_owners_belong_to_actor(struct LmMessageThread *lm_lmx_message_thread, const LmP0Document *document) {
    (void)lm_lmx_message_thread;
    if (document == 0 || document -> owners_initialized == 0) {
        return 1;
    }
    if ((document -> diagnostic_arena != 0 && document -> diagnostic_arena -> owner_thread != 0 && document -> diagnostic_arena -> owner_thread != lm_lmx_message_thread) || (document -> tree_arena != 0 && document -> tree_arena -> owner_thread != 0 && document -> tree_arena -> owner_thread != lm_lmx_message_thread) || (document -> token_arena != 0 && document -> token_arena -> owner_thread != 0 && document -> token_arena -> owner_thread != lm_lmx_message_thread) || (document -> source_owner != 0 && document -> source_owner -> owner_thread != 0 && document -> source_owner -> owner_thread != lm_lmx_message_thread)) {
        return 0;
    }
    return 1;
}

static void lm_p0_document_destroy_owners(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document) {
    (void)lm_lmx_message_thread;
    if (document != 0 && document -> owners_initialized != 0) {
        if (lm_p0_document_owners_belong_to_actor(lm_lmx_message_thread, document) == 0) {
            return;
        }
        lm_own_arena_destroy(lm_lmx_message_thread, document -> diagnostic_arena);
        lm_own_delete(document -> diagnostic_arena, 0);
        lm_own_arena_destroy(lm_lmx_message_thread, document -> tree_arena);
        lm_own_delete(document -> tree_arena, 0);
        lm_own_arena_destroy(lm_lmx_message_thread, document -> token_arena);
        lm_own_delete(document -> token_arena, 0);
        lm_own_arena_destroy(lm_lmx_message_thread, document -> source_owner);
        lm_own_delete(document -> source_owner, 0);
        document->diagnostic_arena = 0;
        document->tree_arena = 0;
        document->token_arena = 0;
        document->source_owner = 0;
        document->diagnostic = 0;
        document->owners_initialized = 0;
        document->frozen = 0;
    }
}

static void lm_p0_document_freeze_tree(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document) {
    (void)lm_lmx_message_thread;
    if (document != 0 && document -> owners_initialized != 0) {
        lm_own_arena_freeze(lm_lmx_message_thread, document -> tree_arena);
        lm_own_arena_freeze(lm_lmx_message_thread, document -> source_owner);
        document->frozen = 1;
    }
}

static void lm_p0_indent_stack_free(struct LmMessageThread *lm_lmx_message_thread, LmP0IndentStack *stack) {
    (void)lm_lmx_message_thread;
    lm_own_delete(stack -> columns, 0);
    stack->columns = 0;
    stack->count = 0U;
    stack->capacity = 0U;
}

static void lm_p0_indent_stack_free_any(void *object) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    LmP0IndentStack * stack;
    stack = ((LmP0IndentStack *)object);
    lm_p0_indent_stack_free(lm_lmx_message_thread, stack);
}

static int lm_p0_indent_stack_push(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmP0IndentStack *stack, size_t column, size_t line, size_t source_column) {
    (void)lm_lmx_message_thread;
    size_t new_capacity;
    size_t *columns;
    if (stack -> count == stack -> capacity) {
        if (stack -> capacity == 0U) {
            new_capacity = 8U;
        }
        if (stack -> capacity != 0U) {
            new_capacity = stack -> capacity * 2U;
        }
        columns = lm_own_resize(stack -> columns, new_capacity * sizeof(columns[0]));
        if (columns == 0) {
            lm_p0_set_diagnostic(document, 1, line, source_column, "out of memory while storing indentation levels");
            return 0;
        }
        stack->columns = columns;
        stack->capacity = new_capacity;
    }
    stack->columns[stack -> count] = column;
    stack->count = stack -> count + 1U;
    return 1;
}

static int lm_p0_indent_stack_init(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmP0IndentStack *stack) {
    (void)lm_lmx_message_thread;
    memset(stack, 0, sizeof(stack[0]));
    return lm_p0_indent_stack_push(lm_lmx_message_thread, document, stack, 0U, 0U, 0U);
}

static LmP0IndentStack * lm_p0_indent_stack_new_empty(struct LmMessageThread *lm_lmx_message_thread) {
    (void)lm_lmx_message_thread;
    return lm_own_new_zero(1U * sizeof(LmP0IndentStack));
}

static LmP0IndentStack * lm_p0_indent_stack_new(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document) {
    (void)lm_lmx_message_thread;
    LmP0IndentStack * stack;
    stack = lm_p0_indent_stack_new_empty(lm_lmx_message_thread);
    if (stack == 0) {
        lm_p0_set_diagnostic(document, 1, 0U, 0U, "out of memory while creating indentation stack");
        return 0;
    }
    if (lm_p0_indent_stack_init(lm_lmx_message_thread, document, stack) == 0) {
        lm_own_delete(stack, lm_p0_indent_stack_free_any);
        return 0;
    }
    return stack;
}

static void lm_p0_indent_stack_delete(struct LmMessageThread *lm_lmx_message_thread, LmP0IndentStack *stack) {
    (void)lm_lmx_message_thread;
    lm_own_delete(stack, lm_p0_indent_stack_free_any);
}

static int lm_p0_indent_stack_copy(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmP0IndentStack *target, const LmP0IndentStack *source, size_t line, size_t column) {
    (void)lm_lmx_message_thread;
    size_t capacity;
    memset(target, 0, sizeof(target[0]));
    if (source -> capacity > source -> count) {
        capacity = source -> capacity;
    }
    if (source -> capacity <= source -> count) {
        capacity = source -> count;
    }
    if (capacity == 0U) {
        return 1;
    }
    target->columns = lm_own_new_zero(capacity * sizeof(target->columns[0]));
    if (target -> columns == 0) {
        lm_p0_set_diagnostic(document, 1, line, column, "out of memory while copying indentation levels");
        return 0;
    }
    memcpy(target -> columns, source -> columns, source -> count * sizeof(target->columns[0]));
    target->count = source -> count;
    target->capacity = capacity;
    return 1;
}

static LmP0IndentStack * lm_p0_indent_stack_clone(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, const LmP0IndentStack *source, size_t line, size_t column) {
    (void)lm_lmx_message_thread;
    LmP0IndentStack * target;
    target = lm_p0_indent_stack_new_empty(lm_lmx_message_thread);
    if (target == 0) {
        lm_p0_set_diagnostic(document, 1, line, column, "out of memory while copying indentation levels");
        return 0;
    }
    if (lm_p0_indent_stack_copy(lm_lmx_message_thread, document, target, source, line, column) == 0) {
        lm_p0_indent_stack_delete(lm_lmx_message_thread, target);
        return 0;
    }
    return target;
}

static size_t lm_p0_indent_tab_column(struct LmMessageThread *lm_lmx_message_thread, size_t column) {
    (void)lm_lmx_message_thread;
    return ((column / 8U) + 1U) * 8U;
}

static void lm_p0_scan_indent_column(struct LmMessageThread *lm_lmx_message_thread, const char *source, size_t start, size_t end, size_t *out_offset, size_t *out_column) {
    (void)lm_lmx_message_thread;
    size_t p;
    size_t column;
    p = start;
    column = 0U;
    while (p < end && lm_p0_is_horizontal_space(lm_lmx_message_thread, source[p])) {
        if (source[p] == '\t') {
            column = lm_p0_indent_tab_column(lm_lmx_message_thread, column);
        }
        if (source[p] != '\t') {
            column = column + 1U;
        }
        p = p + 1U;
    }
    out_offset[0] = p;
    out_column[0] = column;
}

static size_t lm_p0_visual_column_between(struct LmMessageThread *lm_lmx_message_thread, const char *source, size_t start, size_t end) {
    (void)lm_lmx_message_thread;
    size_t p;
    size_t column;
    p = start;
    column = 0U;
    while (p < end) {
        if (source[p] == '\r' && p + 1U < end && source[p + 1U] == '\n') {
            p = p + 1U;
        }
        if (source[p] == '\r') {
            column = 0U;
            p = p + 1U;
            continue;
        }
        if (source[p] == '\n') {
            column = 0U;
            p = p + 1U;
            continue;
        }
        if (source[p] == '\t') {
            column = lm_p0_indent_tab_column(lm_lmx_message_thread, column);
            p = p + 1U;
            continue;
        }
        column = column + 1U;
        p = p + 1U;
    }
    return column;
}

static const LmTableCell * lm_p0_registry_source_path_cell_at(struct LmMessageThread *lm_lmx_message_thread, const char *path, size_t index, const LmTableCell **out_key_cell) {
    (void)lm_lmx_message_thread;
    LmP0Registry * registry;
    int covered;
    if (out_key_cell != 0) {
        out_key_cell[0] = 0;
    }
    registry = lm_p0_registry_current(lm_lmx_message_thread);
    if (path == 0 || registry == 0 || registry -> view == 0) {
        return 0;
    }
    covered = 0;
    return lm_registry_view_source_path_cell_at_slice(lm_lmx_message_thread, registry -> view, path, strlen(path), index, 0, 0, out_key_cell, &covered);
}

static const LmTableCell * lm_p0_registry_source_path_lookup_cell_slice(struct LmMessageThread *lm_lmx_message_thread, const LmRegistryView *view, const char *path, size_t path_length, const char *key, size_t key_length, const LmTableCell **out_key_cell) {
    (void)lm_lmx_message_thread;
    const LmTableDescriptor * descriptor;
    const LmTableRow * row;
    const LmTableCell * key_cell;
    const LmTableCell * payload_cell;
    size_t table_index;
    size_t row_index;
    size_t column_index;
    if (out_key_cell != 0) {
        out_key_cell[0] = 0;
    }
    if (view == 0 || path == 0 || key == 0) {
        return 0;
    }
    table_index = lm_registry_view_source_table_count(lm_lmx_message_thread, view);
    while (table_index > 0U) {
        table_index = table_index - 1U;
        descriptor = lm_registry_view_source_table_at(lm_lmx_message_thread, view, table_index);
        if (descriptor != 0) {
            row_index = lm_table_descriptor_materialized_row_count(lm_lmx_message_thread, descriptor);
            while (row_index > 0U) {
                row_index = row_index - 1U;
                row = lm_table_descriptor_materialized_row_at(lm_lmx_message_thread, descriptor, row_index);
                key_cell = lm_table_row_cell_at(lm_lmx_message_thread, row, 0U);
                if (key_cell != 0 && key_cell -> value != 0 && lm_table_descriptor_cstr_equals_slice(lm_lmx_message_thread, key_cell -> value, key, key_length) != 0) {
                    column_index = lm_table_descriptor_column_count(lm_lmx_message_thread, descriptor);
                    while (column_index > 1U) {
                        column_index = column_index - 1U;
                        if (lm_table_descriptor_source_path_column_matches(lm_lmx_message_thread, descriptor, column_index, path, path_length) != 0) {
                            payload_cell = lm_table_row_cell_at(lm_lmx_message_thread, row, column_index);
                            if (payload_cell != 0 && (payload_cell -> value != 0 || payload_cell -> node != 0)) {
                                if (out_key_cell != 0) {
                                    out_key_cell[0] = key_cell;
                                }
                                return payload_cell;
                            }
                        }
                    }
                }
            }
        }
    }
    return 0;
}

static const char * lm_p0_registry_lookup_in(struct LmMessageThread *lm_lmx_message_thread, LmP0Registry *registry, const LmP0Text *key, const char *table) {
    (void)lm_lmx_message_thread;
    LmP0Text * key_payload;
    const LmTableCell * payload_cell;
    if (table == 0 || registry == 0 || registry -> view == 0 || (registry -> loaded == 0 && registry -> loading == 0)) {
        return 0;
    }
    key_payload = lm_p0_text_from_cstr(lm_lmx_message_thread, "");
    if (key_payload == 0) {
        return 0;
    }
    if (lm_p0_identifier_payload(lm_lmx_message_thread, key, key_payload) == 0) {
        lm_p0_text_view_delete(lm_lmx_message_thread, key_payload);
        return 0;
    }
    payload_cell = lm_p0_registry_source_path_lookup_cell_slice(lm_lmx_message_thread, registry -> view, table, strlen(table), key_payload -> data, key_payload -> length, 0);
    lm_p0_text_view_delete(lm_lmx_message_thread, key_payload);
    if (payload_cell == 0) {
        return 0;
    }
    return payload_cell -> value;
}

static const char * lm_p0_registry_lookup(struct LmMessageThread *lm_lmx_message_thread, const LmP0Text *key, const char *table) {
    (void)lm_lmx_message_thread;
    return lm_p0_registry_lookup_in(lm_lmx_message_thread, lm_p0_registry_current(lm_lmx_message_thread), key, table);
}

static const char * lm_p0_registry_lookup_cstr_in(struct LmMessageThread *lm_lmx_message_thread, LmP0Registry *registry, const char *key, const char *table) {
    (void)lm_lmx_message_thread;
    LmP0Text * key_text;
    const char *payload;
    key_text = lm_p0_text_from_cstr(lm_lmx_message_thread, key);
    if (key_text == 0) {
        return 0;
    }
    payload = lm_p0_registry_lookup_in(lm_lmx_message_thread, registry, key_text, table);
    lm_p0_text_view_delete(lm_lmx_message_thread, key_text);
    return payload;
}

static const char * lm_p0_registry_lookup_cstr(struct LmMessageThread *lm_lmx_message_thread, const char *key, const char *table) {
    (void)lm_lmx_message_thread;
    return lm_p0_registry_lookup_cstr_in(lm_lmx_message_thread, lm_p0_registry_current(lm_lmx_message_thread), key, table);
}

static int lm_p0_registry_source_path_has_rows(struct LmMessageThread *lm_lmx_message_thread, const char *table) {
    (void)lm_lmx_message_thread;
    LmP0Registry * registry;
    int covered;
    registry = lm_p0_registry_current(lm_lmx_message_thread);
    if (table == 0 || registry == 0 || registry -> view == 0) {
        return 0;
    }
    covered = 0;
    return lm_registry_view_source_path_has_rows(lm_lmx_message_thread, registry -> view, table, &covered);
}

static int lm_p0_registry_table_has_rows(struct LmMessageThread *lm_lmx_message_thread, const char *table) {
    (void)lm_lmx_message_thread;
    LmP0Registry * registry;
    registry = lm_p0_registry_current(lm_lmx_message_thread);
    if (registry == 0 || registry -> loaded == 0) {
        return 0;
    }
    return lm_p0_registry_source_path_has_rows(lm_lmx_message_thread, table);
}

static int lm_p0_registry_table_has_rows_loaded_or_loading(struct LmMessageThread *lm_lmx_message_thread, const char *table) {
    (void)lm_lmx_message_thread;
    return lm_p0_registry_source_path_has_rows(lm_lmx_message_thread, table);
}

static size_t lm_p0_count_line_breaks(struct LmMessageThread *lm_lmx_message_thread, const char *source, size_t start, size_t end) {
    (void)lm_lmx_message_thread;
    size_t count;
    size_t i;
    size_t width;
    count = 0U;
    i = start;
    while (i < end) {
        width = lm_p0_line_break_width_at(lm_lmx_message_thread, source, end, i);
        if (width != 0U) {
            count = count + 1U;
            i = i + width;
            continue;
        }
        i = i + 1U;
    }
    return count;
}

static void lm_p0_position_in_slice(struct LmMessageThread *lm_lmx_message_thread, const char *text, size_t length, size_t index, size_t base_line, size_t base_column, size_t *out_line, size_t *out_column) {
    (void)lm_lmx_message_thread;
    size_t i;
    size_t line;
    size_t column;
    size_t width;
    line = base_line;
    column = base_column;
    if (index > length) {
        index = length;
    }
    i = 0U;
    while (i < index) {
        width = lm_p0_line_break_width_at(lm_lmx_message_thread, text, index, i);
        if (width != 0U) {
            line = line + 1U;
            column = 1U;
            i = i + width;
            continue;
        }
        column = column + 1U;
        i = i + 1U;
    }
    out_line[0] = line;
    out_column[0] = column;
}

static void lm_p0_advance_layout_line(struct LmMessageThread *lm_lmx_message_thread, const char *source, size_t length, size_t line_start, size_t line_end, size_t *offset, size_t *line) {
    (void)lm_lmx_message_thread;
    size_t next_offset;
    size_t line_breaks;
    size_t width;
    next_offset = line_end;
    line_breaks = lm_p0_count_line_breaks(lm_lmx_message_thread, source, line_start, line_end);
    width = lm_p0_line_break_width_at(lm_lmx_message_thread, source, length, next_offset);
    if (width != 0U) {
        line_breaks = line_breaks + 1U;
        next_offset = next_offset + width;
    }
    offset[0] = next_offset;
    line[0] = line[0] + line_breaks;
}

static int lm_p0_index_is_line_start(struct LmMessageThread *lm_lmx_message_thread, const char *text, size_t index) {
    (void)lm_lmx_message_thread;
    if (index == 0U) {
        return 1;
    }
    return text[index - 1U] == '\n' || text[index - 1U] == '\r';
}

static int lm_p0_line_rest_is_horizontal_space(struct LmMessageThread *lm_lmx_message_thread, const char *source, size_t start, size_t end) {
    (void)lm_lmx_message_thread;
    size_t i;
    i = start;
    while (i < end) {
        if (lm_p0_is_horizontal_space(lm_lmx_message_thread, source[i]) == 0) {
            return 0;
        }
        i = i + 1U;
    }
    return 1;
}

static size_t lm_p0_find_physical_line_end(struct LmMessageThread *lm_lmx_message_thread, const char *source, size_t length, size_t start) {
    (void)lm_lmx_message_thread;
    size_t i;
    i = start;
    while (i < length && lm_p0_is_line_break(lm_lmx_message_thread, source[i]) == 0) {
        i = i + 1U;
    }
    return i;
}

static int lm_p0_text_has_prefix_name(struct LmMessageThread *lm_lmx_message_thread, const char *text, size_t length, const char *name, int allow_bare) {
    (void)lm_lmx_message_thread;
    size_t name_length;
    name_length = strlen(name);
    if (length < name_length) {
        return 0;
    }
    if (memcmp(text, name, name_length) != 0) {
        return 0;
    }
    if (length == name_length) {
        return allow_bare;
    }
    if (allow_bare != 0 && (lm_p0_is_horizontal_space(lm_lmx_message_thread, text[name_length]) != 0 || text[name_length] == '#')) {
        return 1;
    }
    return text[name_length] == ':';
}

static LmP0TrailerRole lm_p0_legacy_trailer_role(struct LmMessageThread *lm_lmx_message_thread, const char *text, size_t length) {
    (void)lm_lmx_message_thread;
    if (lm_p0_text_has_prefix_name(lm_lmx_message_thread, text, length, "end", 0) != 0) {
        return LM_P0_TRAILER_ROLE_END;
    }
    if (lm_p0_text_has_prefix_name(lm_lmx_message_thread, text, length, "return", 1) != 0) {
        return LM_P0_TRAILER_ROLE_RETURN;
    }
    if (lm_p0_text_has_prefix_name(lm_lmx_message_thread, text, length, "until", 0) != 0) {
        return LM_P0_TRAILER_ROLE_UNTIL;
    }
    return LM_P0_TRAILER_ROLE_NONE;
}

static LmP0TrailerRole lm_p0_trailer_role_from_payload(struct LmMessageThread *lm_lmx_message_thread, const char *payload) {
    (void)lm_lmx_message_thread;
    if (payload == 0) {
        return LM_P0_TRAILER_ROLE_NONE;
    }
    if (strcmp(payload, "LM_P0_TRAILER_ROLE_END") == 0 || strcmp(payload, "trailer.end") == 0) {
        return LM_P0_TRAILER_ROLE_END;
    }
    if (strcmp(payload, "LM_P0_TRAILER_ROLE_RETURN") == 0 || strcmp(payload, "trailer.return") == 0) {
        return LM_P0_TRAILER_ROLE_RETURN;
    }
    if (strcmp(payload, "LM_P0_TRAILER_ROLE_UNTIL") == 0 || strcmp(payload, "trailer.until") == 0) {
        return LM_P0_TRAILER_ROLE_UNTIL;
    }
    if (strcmp(payload, "LM_P0_TRAILER_ROLE_DASH_CUTTER") == 0 || strcmp(payload, "trailer.dash-cutter") == 0) {
        return LM_P0_TRAILER_ROLE_DASH_CUTTER;
    }
    return LM_P0_TRAILER_ROLE_NONE;
}

static const char * lm_p0_trailer_role_payload(struct LmMessageThread *lm_lmx_message_thread, LmP0TrailerRole role) {
    (void)lm_lmx_message_thread;
    if (role == LM_P0_TRAILER_ROLE_END) {
        return "trailer.end";
    }
    if (role == LM_P0_TRAILER_ROLE_RETURN) {
        return "trailer.return";
    }
    if (role == LM_P0_TRAILER_ROLE_UNTIL) {
        return "trailer.until";
    }
    if (role == LM_P0_TRAILER_ROLE_DASH_CUTTER) {
        return "trailer.dash-cutter";
    }
    return 0;
}

static int lm_p0_registry_trailer_allows_bare(struct LmMessageThread *lm_lmx_message_thread, const char *class_name) {
    (void)lm_lmx_message_thread;
    const char *payload;
    payload = lm_p0_registry_lookup_cstr(lm_lmx_message_thread, class_name, "trailer.allow-bare");
    return payload != 0 && (strcmp(payload, "1") == 0 || strcmp(payload, "true") == 0);
}

static int lm_p0_trailer_role_is_tail_cutter(struct LmMessageThread *lm_lmx_message_thread, LmP0TrailerRole role) {
    (void)lm_lmx_message_thread;
    return role != LM_P0_TRAILER_ROLE_NONE;
}

static int lm_p0_node_head_is(struct LmMessageThread *lm_lmx_message_thread, const LmP0Node *node, const char *name) {
    (void)lm_lmx_message_thread;
    size_t name_length;
    if (node == 0 || node -> kind != LM_P0_NODE_FRAME) {
        return 0;
    }
    name_length = strlen(name);
    return node -> as -> frame -> head -> length == name_length && memcmp(node -> as -> frame -> head -> data, name, name_length) == 0;
}

static int lm_p0_trailer_role_accepts_target(struct LmMessageThread *lm_lmx_message_thread, LmP0TrailerRole role, const LmP0Node *target) {
    (void)lm_lmx_message_thread;
    const char *role_payload;
    const char *target_head;
    role_payload = lm_p0_trailer_role_payload(lm_lmx_message_thread, role);
    target_head = 0;
    if (role_payload != 0) {
        target_head = lm_p0_registry_lookup_cstr(lm_lmx_message_thread, role_payload, "trailer.target");
    }
    if (target_head != 0) {
        return lm_p0_node_head_is(lm_lmx_message_thread, target, target_head);
    }
    if (lm_p0_registry_table_has_rows(lm_lmx_message_thread, "trailer.target") == 0 && role == LM_P0_TRAILER_ROLE_RETURN) {
        return lm_p0_node_head_is(lm_lmx_message_thread, target, "fn");
    }
    return lm_p0_trailer_role_is_tail_cutter(lm_lmx_message_thread, role);
}

const LmP0Structure * lm_p0_node_structure(struct LmMessageThread *lm_lmx_message_thread, const LmP0Node *node) {
    (void)lm_lmx_message_thread;
    if (node == 0 || node -> kind != LM_P0_NODE_STRUCTURE) {
        return 0;
    }
    return node -> as -> structure;
}

const LmP0Frame * lm_p0_node_frame(struct LmMessageThread *lm_lmx_message_thread, const LmP0Node *node) {
    (void)lm_lmx_message_thread;
    if (node == 0 || node -> kind != LM_P0_NODE_FRAME) {
        return 0;
    }
    return node -> as -> frame;
}

const LmP0Text * lm_p0_node_atom(struct LmMessageThread *lm_lmx_message_thread, const LmP0Node *node) {
    (void)lm_lmx_message_thread;
    if (node == 0 || (node -> kind != LM_P0_NODE_ATOM && node -> kind != LM_P0_NODE_DISABLED)) {
        return 0;
    }
    return node -> as -> atom;
}

const LmP0Trailer * lm_p0_structure_trailer(struct LmMessageThread *lm_lmx_message_thread, const LmP0Structure *structure) {
    (void)lm_lmx_message_thread;
    if (structure == 0) {
        return 0;
    }
    return structure -> trailer;
}

const LmP0Text * lm_p0_frame_head(struct LmMessageThread *lm_lmx_message_thread, const LmP0Frame *frame) {
    (void)lm_lmx_message_thread;
    if (frame == 0) {
        return 0;
    }
    return frame -> head;
}

const LmP0Structure * lm_p0_frame_body(struct LmMessageThread *lm_lmx_message_thread, const LmP0Frame *frame) {
    (void)lm_lmx_message_thread;
    if (frame == 0) {
        return 0;
    }
    return frame -> body;
}

const LmP0Trailer * lm_p0_frame_trailer(struct LmMessageThread *lm_lmx_message_thread, const LmP0Frame *frame) {
    (void)lm_lmx_message_thread;
    if (frame == 0) {
        return 0;
    }
    return frame -> trailer;
}

const LmP0Text * lm_p0_trailer_spelling(struct LmMessageThread *lm_lmx_message_thread, const LmP0Trailer *trailer) {
    (void)lm_lmx_message_thread;
    if (trailer == 0) {
        return 0;
    }
    return trailer -> spelling;
}

const LmP0Structure * lm_p0_trailer_body(struct LmMessageThread *lm_lmx_message_thread, const LmP0Trailer *trailer) {
    (void)lm_lmx_message_thread;
    if (trailer == 0) {
        return 0;
    }
    return trailer -> body;
}

static int lm_p0_stream_event_is_tail_cutter(struct LmMessageThread *lm_lmx_message_thread, const LmP0StreamEvent *event) {
    (void)lm_lmx_message_thread;
    return event -> kind == LM_P0_STREAM_EVENT_ITEM && lm_p0_trailer_role_is_tail_cutter(lm_lmx_message_thread, lm_p0_trailer_role(event -> text, event -> text_length)) != 0;
}

static int lm_p0_find_python_string_end(struct LmMessageThread *lm_lmx_message_thread, const char *text, size_t length, size_t start, size_t *out_end) {
    (void)lm_lmx_message_thread;
    char quote;
    size_t i;
    size_t run_length;
    if (lm_p0_starts_python_string(lm_lmx_message_thread, text, length, start) == 0) {
        out_end[0] = start;
        return 0;
    }
    quote = text[start];
    i = start + 3U;
    while (i < length) {
        if (text[i] == quote) {
            run_length = 1U;
            while (i + run_length < length && text[i + run_length] == quote) {
                run_length = run_length + 1U;
            }
            if (run_length == 3U) {
                out_end[0] = i + 3U;
                return 1;
            }
            i = i + run_length;
            continue;
        }
        i = i + 1U;
    }
    out_end[0] = length;
    return 0;
}

static size_t lm_p0_skip_python_string_unchecked(struct LmMessageThread *lm_lmx_message_thread, const char *text, size_t length, size_t start) {
    (void)lm_lmx_message_thread;
    size_t end;
    if (lm_p0_find_python_string_end(lm_lmx_message_thread, text, length, start, &end) == 0) {
        return length;
    }
    return end;
}

static void lm_p0_scan_layout_prefix(struct LmMessageThread *lm_lmx_message_thread, const char *source, size_t length, size_t start, size_t *out_offset, size_t *out_indent_column, size_t *out_dot_level) {
    (void)lm_lmx_message_thread;
    size_t p;
    size_t indent_column;
    size_t dot_level;
    lm_p0_scan_indent_column(lm_lmx_message_thread, source, start, length, &p, &indent_column);
    dot_level = 0U;
    while (p < length && source[p] == '.') {
        dot_level = dot_level + 1U;
        p = p + 1U;
        while (p < length && lm_p0_is_horizontal_space(lm_lmx_message_thread, source[p]) != 0) {
            p = p + 1U;
        }
    }
    out_offset[0] = p;
    out_indent_column[0] = indent_column;
    out_dot_level[0] = dot_level;
}

static int lm_p0_layout_prefix_is_deeper(struct LmMessageThread *lm_lmx_message_thread, size_t indent_column, size_t dot_level, size_t base_indent_column, size_t base_dot_level) {
    (void)lm_lmx_message_thread;
    if (dot_level > 0U || base_dot_level > 0U) {
        return dot_level > base_dot_level;
    }
    return indent_column > base_indent_column;
}

static LmP0DashFenceStatus lm_p0_dash_fence_status(struct LmMessageThread *lm_lmx_message_thread, const char *text, size_t length, size_t *out_dash_count) {
    (void)lm_lmx_message_thread;
    size_t dash_count;
    size_t i;
    int closed;
    size_t brace_end;
    dash_count = 0U;
    while (dash_count < length && text[dash_count] == '-') {
        dash_count = dash_count + 1U;
    }
    if (out_dash_count != 0) {
        out_dash_count[0] = dash_count;
    }
    if (dash_count < 3U) {
        return LM_P0_DASH_FENCE_NONE;
    }
    if (dash_count > LM_P0_MAX_FENCE_LENGTH) {
        return LM_P0_DASH_FENCE_TOO_LONG;
    }
    i = dash_count;
    while (i < length) {
        if (lm_p0_is_horizontal_space(lm_lmx_message_thread, text[i]) != 0) {
            i = i + 1U;
            continue;
        }
        if (text[i] == '#') {
            return LM_P0_DASH_FENCE_VALID;
        }
        if (text[i] == '{') {
            brace_end = lm_p0_scan_brace_mark_unchecked(text, length, i, &closed);
            if (closed != 0) {
                i = brace_end;
                continue;
            }
        }
        return LM_P0_DASH_FENCE_TRAILING_TEXT;
    }
    return LM_P0_DASH_FENCE_VALID;
}

static int lm_p0_validate_dash_fence_line(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, const char *text, size_t length, size_t line, size_t column) {
    (void)lm_lmx_message_thread;
    LmP0DashFenceStatus status;
    status = lm_p0_dash_fence_status_after_comment_trim(text, length, 0);
    if (status == LM_P0_DASH_FENCE_TOO_LONG) {
        lm_p0_set_diagnostic(document, 25, line, column, "dash delimiter fence length exceeds 80 characters");
        return 0;
    }
    if (status == LM_P0_DASH_FENCE_TRAILING_TEXT) {
        lm_p0_set_diagnostic(document, 26, line, column, "dash delimiter line must contain only the dash fence, whitespace, or a line comment");
        return 0;
    }
    return 1;
}

static int lm_p0_match_block_string_fence_line(struct LmMessageThread *lm_lmx_message_thread, const char *source, size_t line_start, size_t line_end, size_t eq_count) {
    (void)lm_lmx_message_thread;
    size_t i;
    if (eq_count < 3U || eq_count > LM_P0_MAX_FENCE_LENGTH || line_start + eq_count > line_end) {
        return 0;
    }
    i = 0U;
    while (i < eq_count) {
        if (source[line_start + i] != '=') {
            return 0;
        }
        i = i + 1U;
    }
    if (line_start + eq_count < line_end && source[line_start + eq_count] == '=') {
        return 0;
    }
    i = line_start + eq_count;
    while (i < line_end) {
        if (lm_p0_is_horizontal_space(lm_lmx_message_thread, source[i]) == 0) {
            return 0;
        }
        i = i + 1U;
    }
    return 1;
}

static int lm_p0_match_raw_comment_fence_line(struct LmMessageThread *lm_lmx_message_thread, const char *source, size_t line_start, size_t line_end, size_t star_count) {
    (void)lm_lmx_message_thread;
    size_t i;
    if (star_count < 3U || star_count > LM_P0_MAX_FENCE_LENGTH || line_start + star_count > line_end) {
        return 0;
    }
    i = 0U;
    while (i < star_count) {
        if (source[line_start + i] != '*') {
            return 0;
        }
        i = i + 1U;
    }
    if (line_start + star_count < line_end && source[line_start + star_count] == '*') {
        return 0;
    }
    return lm_p0_line_rest_is_horizontal_space(lm_lmx_message_thread, source, line_start + star_count, line_end);
}

static void lm_p0_dump_append_size(struct LmMessageThread *lm_lmx_message_thread, LmP0Dump *dump, size_t value) {
    (void)lm_lmx_message_thread;
    char buffer[32];
    size_t index;
    if (value == 0U) {
        lm_p0_dump_append_cstr(dump, "0");
        return;
    }
    index = sizeof(buffer);
    while (value > 0U) {
        index = index - 1U;
        buffer[index] = '0' + value % 10U;
        value = value / 10U;
    }
    lm_p0_dump_append(dump, buffer + index, sizeof(buffer) - index);
}

static void lm_p0_dump_append_field_count_line(struct LmMessageThread *lm_lmx_message_thread, LmP0Dump *dump, size_t field_count) {
    (void)lm_lmx_message_thread;
    lm_p0_dump_append_cstr(dump, " fields=");
    lm_p0_dump_append_size(lm_lmx_message_thread, dump, field_count);
    lm_p0_dump_append_cstr(dump, "\n");
}

static void lm_p0_registry_private_api_anchor(struct LmMessageThread *lm_lmx_message_thread) {
    (void)lm_lmx_message_thread;
    LM_UNUSED(&lm_registry_view_class_at);
    LM_UNUSED(&lm_registry_view_source_path_key_at_slice);
    LM_UNUSED(&lm_registry_view_append_materialized_rows);
    LM_UNUSED(&lm_registry_view_fact_at);
    LM_UNUSED(&lm_registry_view_push_relation);
    LM_UNUSED(&lm_registry_view_lookup_exact);
    LM_UNUSED(&lm_registry_view_lookup_default);
    LM_UNUSED(&lm_registry_view_lookup);
    LM_UNUSED(&lm_registry_view_lookup_text);
    LM_UNUSED(&lm_registry_view_table_row_at);
    LM_UNUSED(&lm_registry_view_matching_key_count);
    LM_UNUSED(&lm_registry_view_matching_key_at);
    LM_UNUSED(&lm_registry_view_local_source_rows_slice);
    LM_UNUSED(&lm_registry_view_table_has_rows);
    LM_UNUSED(&lm_p0_registry_source_tables_selftest);
}

static void lm_p0_registry_component_destroy(struct LmMessageThread *lm_lmx_message_thread, void *component) {
    (void)lm_lmx_message_thread;
    LmP0Registry * registry;
    registry = (((LmP0Registry *)component));
    if (registry == 0) {
        return;
    }
    lm_registry_view_delete(lm_lmx_message_thread, registry -> view);
    registry->view = 0;
    registry->loaded = 0;
    registry->loading = 0;
    lm_own_delete(registry, 0);
}

static LmP0Registry * lm_p0_registry_current(struct LmMessageThread *lm_lmx_message_thread) {
    (void)lm_lmx_message_thread;
    return (((LmP0Registry *)lm_message_thread_component_get(lm_lmx_message_thread, &lm_p0_registry_component_destroy)));
}

static int lm_p0_registry_init(struct LmMessageThread *lm_lmx_message_thread) {
    (void)lm_lmx_message_thread;
    LmP0Registry * registry;
    lm_p0_registry_private_api_anchor(lm_lmx_message_thread);
    registry = lm_p0_registry_current(lm_lmx_message_thread);
    if (registry != 0) {
        return 0;
    }
    registry = (((LmP0Registry *)lm_own_new_zero(sizeof(registry[0]))));
    if (registry == 0) {
        return 1;
    }
    registry->view = lm_registry_view_new(lm_lmx_message_thread, 0);
    if (registry -> view == 0) {
        lm_p0_registry_component_destroy(lm_lmx_message_thread, registry);
        return 1;
    }
    if (lm_message_thread_component_attach(lm_lmx_message_thread, &lm_p0_registry_component_destroy, registry) != 0) {
        lm_p0_registry_component_destroy(lm_lmx_message_thread, registry);
        return 1;
    }
    return 0;
}

static void lm_p0_registry_destroy(struct LmMessageThread *lm_lmx_message_thread) {
    (void)lm_lmx_message_thread;
    if (lm_p0_registry_current(lm_lmx_message_thread) == 0) {
        return;
    }
    LM_UNUSED(lm_message_thread_component_remove(lm_lmx_message_thread, &lm_p0_registry_component_destroy));
}

static LmP0Text * lm_p0_text_ref_new_empty(struct LmMessageThread *lm_lmx_message_thread) {
    (void)lm_lmx_message_thread;
    return (((LmP0Text *)lm_own_new_zero(sizeof(LmP0Text))));
}

static void lm_p0_text_ref_delete(struct LmMessageThread *lm_lmx_message_thread, LmP0Text *text) {
    (void)lm_lmx_message_thread;
    lm_own_delete(text, 0);
}

static int lm_p0_document_register_lazy_text(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, const char *source, size_t length, const char **patch_slot, size_t line, size_t column) {
    (void)lm_lmx_message_thread;
    if (patch_slot == 0) {
        return 0;
    }
    if (length == 0U) {
        patch_slot[0] = "";
        return 1;
    }
    if (((document == 0) || (document -> owners_initialized == 0)) || (lm_own_arena_add_lazy_edge(lm_lmx_message_thread, document -> tree_arena, document -> source_owner, source, length, (((const void **)patch_slot))) != 0)) {
        if (document != 0) {
            lm_p0_set_diagnostic(document, 1, line, column, "out of memory while registering parser lazy text edge");
        }
        return 0;
    }
    return 1;
}

static LmP0Text * lm_p0_new_text(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, const char *source, size_t length, size_t line, size_t column) {
    (void)lm_lmx_message_thread;
    LmP0Text * text;
    text = (((LmP0Text *)lm_own_arena_new_zero(lm_lmx_message_thread, document -> tree_arena, sizeof(text[0]))));
    if (text == 0) {
        lm_p0_set_diagnostic(document, 1, line, column, "out of memory while allocating parser text");
        return 0;
    }
    if (source != 0) {
        text->data = source;
    }
    else {
        text->data = "";
    }
    if (source != 0) {
        text->length = length;
    }
    else {
        text->length = 0U;
    }
    if (lm_p0_document_register_lazy_text(lm_lmx_message_thread, document, text -> data, text -> length, &text -> data, line, column) == 0) {
        return 0;
    }
    return text;
}

static LmP0Structure * lm_p0_new_structure(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, size_t line, size_t column) {
    (void)lm_lmx_message_thread;
    LmP0Structure * structure;
    structure = (((LmP0Structure *)lm_own_arena_new_zero(lm_lmx_message_thread, document -> tree_arena, sizeof(structure[0]))));
    if (structure == 0) {
        lm_p0_set_diagnostic(document, 1, line, column, "out of memory while allocating parser structure");
        return 0;
    }
    return structure;
}

static LmP0Frame * lm_p0_new_frame(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, size_t line, size_t column) {
    (void)lm_lmx_message_thread;
    LmP0Frame * frame;
    frame = (((LmP0Frame *)lm_own_arena_new_zero(lm_lmx_message_thread, document -> tree_arena, sizeof(frame[0]))));
    if (frame == 0) {
        lm_p0_set_diagnostic(document, 1, line, column, "out of memory while allocating parser frame");
        return 0;
    }
    frame->head = lm_p0_new_text(lm_lmx_message_thread, document, "", 0U, line, column);
    if (frame -> head == 0) {
        return 0;
    }
    frame->body = lm_p0_new_structure(lm_lmx_message_thread, document, line, column);
    if (frame -> body == 0) {
        return 0;
    }
    return frame;
}

static LmP0Trailer * lm_p0_new_trailer(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, const char *spelling, size_t spelling_length, size_t line, size_t column) {
    (void)lm_lmx_message_thread;
    LmP0Trailer * trailer;
    trailer = (((LmP0Trailer *)lm_own_arena_new_zero(lm_lmx_message_thread, document -> tree_arena, sizeof(trailer[0]))));
    if (trailer == 0) {
        lm_p0_set_diagnostic(document, 1, line, column, "out of memory while allocating parser trailer");
        return 0;
    }
    trailer->spelling = lm_p0_new_text(lm_lmx_message_thread, document, spelling, spelling_length, line, column);
    if (trailer -> spelling == 0) {
        return 0;
    }
    trailer->body = lm_p0_new_structure(lm_lmx_message_thread, document, line, column);
    if (trailer -> body == 0) {
        return 0;
    }
    return trailer;
}

static void lm_p0_set_diagnostic(LmP0Document *document, int code, size_t line, size_t column, const char *message) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    if (document == 0 || document -> diagnostic == 0 || document -> diagnostic -> code != 0) {
        return;
    }
    if (message == 0) {
        message = "";
    }
    document->diagnostic->code = code;
    document->diagnostic->line = line;
    document->diagnostic->column = column;
    snprintf(document -> diagnostic -> message, sizeof(document -> diagnostic -> message), "%s", message);
}

static int lm_p0_registry_column_has_descriptor(struct LmMessageThread *lm_lmx_message_thread, const LmRegistrySourceColumn *column, const char *descriptor) {
    (void)lm_lmx_message_thread;
    size_t i;
    LmP0Text * payload;
    int result;
    if ((column == 0) || (descriptor == 0)) {
        return 0;
    }
    payload = lm_p0_text_ref_new_empty(lm_lmx_message_thread);
    if (payload == 0) {
        return 0;
    }
    result = 0;
    i = 0U;
    while (i < column -> descriptor_count) {
        if (((column -> descriptors[i] != 0) && lm_p0_registry_identifier_value(lm_lmx_message_thread, column -> descriptors[i], payload)) && lm_p0_text_equals(lm_lmx_message_thread, payload, descriptor)) {
            result = 1;
            break;
        }
        i = i + 1U;
    }
    lm_p0_text_ref_delete(lm_lmx_message_thread, payload);
    return result;
}

static int lm_p0_registry_cell_none_cell_matches(struct LmMessageThread *lm_lmx_message_thread, LmP0Registry *registry, const LmP0Text *payload, const LmP0Text *class_atom) {
    (void)lm_lmx_message_thread;
    LmP0Text * class_name;
    const char *none_value;
    int result;
    if (payload == 0 || class_atom == 0) {
        return 0;
    }
    class_name = lm_p0_text_ref_new_empty(lm_lmx_message_thread);
    if (class_name == 0) {
        return 0;
    }
    result = 0;
    if (lm_p0_registry_identifier_value(lm_lmx_message_thread, class_atom, class_name) != 0) {
        none_value = lm_p0_registry_lookup_in(lm_lmx_message_thread, registry, class_name, "None.cell");
        if (none_value != 0 && lm_p0_text_equals(lm_lmx_message_thread, payload, none_value)) {
            result = 1;
        }
    }
    lm_p0_text_ref_delete(lm_lmx_message_thread, class_name);
    return result;
}

static int lm_p0_registry_cell_is_null(struct LmMessageThread *lm_lmx_message_thread, LmP0Registry *registry, const LmP0Text *atom, const LmRegistrySourceColumn *column) {
    (void)lm_lmx_message_thread;
    LmP0Text * payload;
    size_t i;
    int result;
    if (lm_p0_registry_payload_is_null(lm_lmx_message_thread, atom)) {
        return 1;
    }
    if (atom == 0 || column == 0) {
        return 0;
    }
    payload = lm_p0_text_ref_new_empty(lm_lmx_message_thread);
    if (payload == 0) {
        return 0;
    }
    if (lm_p0_registry_identifier_value(lm_lmx_message_thread, atom, payload) == 0) {
        lm_p0_text_ref_delete(lm_lmx_message_thread, payload);
        return 0;
    }
    result = 0;
    if (column -> name != 0 && lm_p0_registry_cell_none_cell_matches(lm_lmx_message_thread, registry, payload, column -> name)) {
        result = 1;
    }
    i = 0U;
    while (result == 0 && i < column -> descriptor_count) {
        if (column -> descriptors[i] != 0 && lm_p0_registry_cell_none_cell_matches(lm_lmx_message_thread, registry, payload, column -> descriptors[i])) {
            result = 1;
        }
        i = i + 1U;
    }
    lm_p0_text_ref_delete(lm_lmx_message_thread, payload);
    return result;
}

static int lm_p0_registry_cell_value(struct LmMessageThread *lm_lmx_message_thread, LmP0Registry *registry, const LmP0Text *atom, const LmRegistrySourceColumn *column, LmP0Text *out_value) {
    (void)lm_lmx_message_thread;
    if (lm_p0_registry_cell_is_null(lm_lmx_message_thread, registry, atom, column)) {
        return 0;
    }
    if (lm_p0_registry_column_has_descriptor(lm_lmx_message_thread, column, "char")) {
        if (lm_p0_registry_literal_value(lm_lmx_message_thread, atom, out_value)) {
            return 1;
        }
        return -1;
    }
    if (lm_p0_registry_identifier_value(lm_lmx_message_thread, atom, out_value)) {
        return 1;
    }
    return -1;
}

static LmTableDescriptor * lm_p0_registry_source_descriptor_new(struct LmMessageThread *lm_lmx_message_thread, const LmP0Text *table_name, LmRegistrySourceColumn **columns, size_t column_count) {
    (void)lm_lmx_message_thread;
    LmP0Text * table_value;
    LmP0Text * column_value;
    LmP0Text * descriptor_value;
    LmP0Text * type_value;
    LmTableDescriptor * descriptor;
    const char *descriptor_data[16];
    size_t descriptor_lengths[16];
    const char *type_data;
    size_t type_length;
    size_t column_index;
    size_t descriptor_index;
    if (table_name == 0 || columns == 0 || column_count == 0U) {
        return 0;
    }
    table_value = lm_p0_text_ref_new_empty(lm_lmx_message_thread);
    column_value = lm_p0_text_ref_new_empty(lm_lmx_message_thread);
    descriptor_value = lm_p0_text_ref_new_empty(lm_lmx_message_thread);
    type_value = lm_p0_text_ref_new_empty(lm_lmx_message_thread);
    descriptor = 0;
    if (table_value == 0 || column_value == 0 || descriptor_value == 0 || type_value == 0 || lm_p0_registry_identifier_value(lm_lmx_message_thread, table_name, table_value) == 0) {
        lm_p0_text_ref_delete(lm_lmx_message_thread, table_value);
        lm_p0_text_ref_delete(lm_lmx_message_thread, column_value);
        lm_p0_text_ref_delete(lm_lmx_message_thread, descriptor_value);
        lm_p0_text_ref_delete(lm_lmx_message_thread, type_value);
        return 0;
    }
    descriptor = lm_table_descriptor_new_empty_slice(lm_lmx_message_thread, table_value -> data, table_value -> length);
    column_index = 0U;
    while (descriptor != 0 && column_index < column_count) {
        if (columns[column_index] == 0 || columns[column_index] -> name == 0 || lm_p0_registry_identifier_value(lm_lmx_message_thread, columns[column_index] -> name, column_value) == 0) {
            lm_table_descriptor_delete_any(descriptor);
            descriptor = 0;
            break;
        }
        type_data = 0;
        type_length = 0U;
        if (columns[column_index] -> type_name != 0) {
            if (lm_p0_registry_identifier_value(lm_lmx_message_thread, columns[column_index] -> type_name, type_value) == 0) {
                lm_table_descriptor_delete_any(descriptor);
                descriptor = 0;
                break;
            }
            type_data = type_value -> data;
            type_length = type_value -> length;
        }
        if (columns[column_index] -> descriptor_count > sizeof(descriptor_data) / sizeof(descriptor_data[0])) {
            lm_table_descriptor_delete_any(descriptor);
            descriptor = 0;
            break;
        }
        descriptor_index = 0U;
        while (descriptor_index < columns[column_index] -> descriptor_count) {
            if (columns[column_index] -> descriptors[descriptor_index] == 0 || lm_p0_registry_identifier_value(lm_lmx_message_thread, columns[column_index] -> descriptors[descriptor_index], descriptor_value) == 0) {
                lm_table_descriptor_delete_any(descriptor);
                descriptor = 0;
                break;
            }
            descriptor_data[descriptor_index] = descriptor_value -> data;
            descriptor_lengths[descriptor_index] = descriptor_value -> length;
            descriptor_index = descriptor_index + 1U;
        }
        if (descriptor == 0) {
            break;
        }
        if (lm_table_descriptor_add_column_slices(lm_lmx_message_thread, descriptor, column_value -> data, column_value -> length, type_data, type_length, descriptor_data, descriptor_lengths, columns[column_index] -> descriptor_count, columns[column_index] -> address_depth, columns[column_index] -> array_rank, columns[column_index] -> is_const) != 0) {
            lm_table_descriptor_delete_any(descriptor);
            descriptor = 0;
            break;
        }
        column_index = column_index + 1U;
    }
    lm_p0_text_ref_delete(lm_lmx_message_thread, table_value);
    lm_p0_text_ref_delete(lm_lmx_message_thread, column_value);
    lm_p0_text_ref_delete(lm_lmx_message_thread, descriptor_value);
    lm_p0_text_ref_delete(lm_lmx_message_thread, type_value);
    return descriptor;
}

static int lm_p0_registry_materialize_source_row(struct LmMessageThread *lm_lmx_message_thread, LmP0Registry *registry, const LmP0Text *table_name, LmRegistrySourceColumn **columns, size_t column_count, const LmP0Node **cells) {
    (void)lm_lmx_message_thread;
    LmP0Text * table_value;
    LmP0Text * decoded;
    LmTableDescriptor * descriptor;
    LmTableRow * row;
    const LmP0Node * node;
    const LmP0Text * atom;
    size_t column_index;
    int cell_status;
    int status;
    if (registry == 0 || registry -> view == 0 || table_name == 0 || columns == 0 || column_count == 0U || cells == 0) {
        return -1;
    }
    table_value = lm_p0_text_ref_new_empty(lm_lmx_message_thread);
    decoded = lm_p0_text_ref_new_empty(lm_lmx_message_thread);
    if (table_value == 0 || decoded == 0 || lm_p0_registry_identifier_value(lm_lmx_message_thread, table_name, table_value) == 0) {
        lm_p0_text_ref_delete(lm_lmx_message_thread, table_value);
        lm_p0_text_ref_delete(lm_lmx_message_thread, decoded);
        return -1;
    }
    descriptor = lm_registry_view_find_local_source_table_slice(lm_lmx_message_thread, registry -> view, table_value -> data, table_value -> length);
    if (descriptor == 0 || lm_table_descriptor_column_count(lm_lmx_message_thread, descriptor) != column_count) {
        lm_p0_text_ref_delete(lm_lmx_message_thread, table_value);
        lm_p0_text_ref_delete(lm_lmx_message_thread, decoded);
        return -1;
    }
    row = lm_table_row_new(lm_lmx_message_thread, 0);
    status = 0;
    column_index = 0U;
    while (status == 0 && column_index < column_count) {
        node = cells[column_index];
        if (columns[column_index] == 0 || node == 0 || node -> kind != LM_P0_NODE_ATOM) {
            status = -1;
        }
        else {
            atom = node -> as -> atom;
            if (atom == 0) {
                status = -1;
            }
            else {
                if (column_index == 0U) {
                    cell_status = lm_p0_registry_identifier_value(lm_lmx_message_thread, atom, decoded);
                }
                else {
                    cell_status = lm_p0_registry_cell_value(lm_lmx_message_thread, registry, atom, columns[column_index], decoded);
                }
                if (cell_status < 0 || (column_index == 0U && cell_status == 0)) {
                    status = -1;
                }
                else {
                    if (cell_status == 0) {
                        if (lm_table_row_take_cell_slice(lm_lmx_message_thread, row, atom -> data, atom -> length, 0, 0U, 0, 0, 1) != 0) {
                            status = -1;
                        }
                    }
                    else {
                        if (lm_table_row_take_cell_slice(lm_lmx_message_thread, row, atom -> data, atom -> length, decoded -> data, decoded -> length, 0, 0, 0) != 0) {
                            status = -1;
                        }
                    }
                }
            }
        }
        column_index = column_index + 1U;
    }
    lm_p0_text_ref_delete(lm_lmx_message_thread, table_value);
    lm_p0_text_ref_delete(lm_lmx_message_thread, decoded);
    if (status != 0 || row == 0) {
        lm_table_row_delete_any(row);
        return -1;
    }
    if (lm_table_descriptor_take_materialized_row(lm_lmx_message_thread, descriptor, row) != 0) {
        lm_table_row_delete_any(row);
        return -1;
    }
    lm_registry_view_note_mutation(lm_lmx_message_thread, registry -> view);
    return 0;
}

static int lm_p0_registry_compare_enabled(struct LmMessageThread *lm_lmx_message_thread) {
    (void)lm_lmx_message_thread;
    const char *value;
    if (LM_P0_ENABLE_REGISTRY_COMPARE == 0) {
        return 0;
    }
    value = getenv("LM_P0_COMPARE_REGISTRY");
    return value == 0 || strcmp(value, "0") != 0;
}

static void lm_p0_registry_compare_fail(struct LmMessageThread *lm_lmx_message_thread, const char *table, const char *key, const char *registry_payload, const char *legacy_payload) {
    (void)lm_lmx_message_thread;
    const char *lm_p0_tmp_1;
    if (table != 0) {
        lm_p0_tmp_1 = table;
    }
    else {
        lm_p0_tmp_1 = "<none>";
    }
    const char *lm_p0_tmp_2;
    if (key != 0) {
        lm_p0_tmp_2 = key;
    }
    else {
        lm_p0_tmp_2 = "<none>";
    }
    const char *lm_p0_tmp_3;
    if (registry_payload != 0) {
        lm_p0_tmp_3 = registry_payload;
    }
    else {
        lm_p0_tmp_3 = "<none>";
    }
    const char *lm_p0_tmp_4;
    if (legacy_payload != 0) {
        lm_p0_tmp_4 = legacy_payload;
    }
    else {
        lm_p0_tmp_4 = "<none>";
    }
    fprintf(stderr, "parser registry mismatch: table=%s key=\"%s\" registry=%s legacy=%s\n", lm_p0_tmp_1, lm_p0_tmp_2, lm_p0_tmp_3, lm_p0_tmp_4);
    exit(2);
}

static void lm_p0_trim_right(struct LmMessageThread *lm_lmx_message_thread, const char **text, size_t *length) {
    (void)lm_lmx_message_thread;
    while ((length[0] > 0U) && (lm_p0_is_horizontal_space(lm_lmx_message_thread, text[0][(length[0] - 1U)]) || (text[0][(length[0] - 1U)] == '\r'))) {
        length[0] = length[0] - 1U;
    }
}

static void lm_p0_trim_trailing_line_comment(struct LmMessageThread *lm_lmx_message_thread, const char **text, size_t *length) {
    (void)lm_lmx_message_thread;
    size_t i;
    i = 0U;
    while (i < length[0]) {
        size_t prefixed_end;
        if (lm_p0_starts_python_string(lm_lmx_message_thread, text[0], length[0], i)) {
            i = lm_p0_skip_python_string_unchecked(lm_lmx_message_thread, text[0], length[0], i);
            continue;
        }
        prefixed_end = lm_p0_scan_c_prefixed_quote_token(lm_lmx_message_thread, text[0], length[0], i);
        if (prefixed_end > i) {
            i = prefixed_end;
            continue;
        }
        if ((text[0][i] == '\'') && (lm_p0_starts_python_string(lm_lmx_message_thread, text[0], length[0], i) == 0)) {
            i = lm_p0_scan_c_char_token(lm_lmx_message_thread, text[0], length[0], i);
            continue;
        }
        if ((text[0][i] == '"') || (text[0][i] == '`')) {
            char quote;
            char lm_p0_tmp_5;
            lm_p0_tmp_5 = text[0][i];
            i = i + 1U;
            quote = lm_p0_tmp_5;
            while (i < length[0]) {
                if ((quote == '"') && (text[0][i] == '\\')) {
                    if ((i + 1U) < length[0]) {
                        i = i + 2U;
                    }
                    else {
                        i = i + 1U;
                    }
                    continue;
                }
                if ((((quote == '`') && (text[0][i] == '`')) && ((i + 1U) < length[0])) && (text[0][(i + 1U)] == '`')) {
                    i = i + 2U;
                    continue;
                }
                char lm_p0_tmp_6;
                lm_p0_tmp_6 = text[0][i];
                i = i + 1U;
                if (lm_p0_tmp_6 == quote) {
                    break;
                }
            }
            continue;
        }
        if (text[0][i] == '#') {
            size_t end_index;
            end_index = (i + 1U);
            while ((end_index < length[0]) && (lm_p0_is_line_break(lm_lmx_message_thread, text[0][end_index]) == 0)) {
                end_index = end_index + 1U;
            }
            if (end_index == length[0]) {
                length[0] = i;
                lm_p0_trim_right(lm_lmx_message_thread, text, length);
            }
            return;
        }
        i = i + 1U;
    }
}

static int lm_p0_indent_level_from_column(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmP0IndentStack *stack, size_t column, size_t line, size_t source_column, size_t *out_level) {
    (void)lm_lmx_message_thread;
    size_t top;
    top = stack -> columns[(stack -> count - 1U)];
    if (column == top) {
        out_level[0] = (stack -> count - 1U);
        return 1;
    }
    if (column > top) {
        if (lm_p0_indent_stack_push(lm_lmx_message_thread, document, stack, column, line, source_column) == 0) {
            return 0;
        }
        out_level[0] = (stack -> count - 1U);
        return 1;
    }
    while ((stack -> count > 0U) && (stack -> columns[(stack -> count - 1U)] > column)) {
        stack->count = stack -> count - 1U;
    }
    if ((stack -> count == 0U) || (stack -> columns[(stack -> count - 1U)] != column)) {
        lm_p0_set_diagnostic(document, 17, line, source_column, "unindent does not match any outer indentation level");
        return 0;
    }
    out_level[0] = (stack -> count - 1U);
    return 1;
}

static size_t lm_p0_skip_fence_block_unchecked(struct LmMessageThread *lm_lmx_message_thread, const char *text, size_t length, size_t start, char fence_char) {
    (void)lm_lmx_message_thread;
    size_t fence_count;
    size_t line_end;
    size_t scan_start;
    line_end = lm_p0_find_physical_line_end(lm_lmx_message_thread, text, length, start);
    fence_count = 0U;
    while (((start + fence_count) < line_end) && (text[(start + fence_count)] == fence_char)) {
        fence_count = fence_count + 1U;
    }
    if ((fence_count < 3U) || (fence_count > LM_P0_MAX_FENCE_LENGTH)) {
        return start;
    }
    if (fence_char == '=') {
        if (lm_p0_match_block_string_fence_line(lm_lmx_message_thread, text, start, line_end, fence_count) == 0) {
            return start;
        }
    }
    else {
        if (fence_char == '*') {
            if (lm_p0_match_raw_comment_fence_line(lm_lmx_message_thread, text, start, line_end, fence_count) == 0) {
                return start;
            }
        }
        else {
            return start;
        }
    }
    scan_start = line_end;
    if (scan_start < length) {
        scan_start = scan_start + lm_p0_line_break_width_at(lm_lmx_message_thread, text, length, scan_start);
    }
    while (scan_start <= length) {
        size_t current_end;
        current_end = lm_p0_find_physical_line_end(lm_lmx_message_thread, text, length, scan_start);
        if ((fence_char == '=') && lm_p0_match_block_string_fence_line(lm_lmx_message_thread, text, scan_start, current_end, fence_count)) {
            size_t lm_p0_tmp_7;
            if (current_end < length) {
                lm_p0_tmp_7 = lm_p0_line_break_width_at(lm_lmx_message_thread, text, length, current_end);
            }
            else {
                lm_p0_tmp_7 = 0U;
            }
            return (current_end + lm_p0_tmp_7);
        }
        if ((fence_char == '*') && lm_p0_match_raw_comment_fence_line(lm_lmx_message_thread, text, scan_start, current_end, fence_count)) {
            size_t lm_p0_tmp_8;
            if (current_end < length) {
                lm_p0_tmp_8 = lm_p0_line_break_width_at(lm_lmx_message_thread, text, length, current_end);
            }
            else {
                lm_p0_tmp_8 = 0U;
            }
            return (current_end + lm_p0_tmp_8);
        }
        if (current_end == length) {
            break;
        }
        scan_start = (current_end + lm_p0_line_break_width_at(lm_lmx_message_thread, text, length, current_end));
    }
    return length;
}

static size_t lm_p0_find_layout_line_end(struct LmMessageThread *lm_lmx_message_thread, const char *source, size_t length, size_t start) {
    (void)lm_lmx_message_thread;
    size_t i;
    size_t depth;
    size_t current_line_indent;
    size_t current_line_dot_level;
    size_t ignored_content_offset;
    size_t delimiter_indent_stack[LM_P0_LAYOUT_DELIMITER_STACK_LIMIT];
    size_t delimiter_dot_stack[LM_P0_LAYOUT_DELIMITER_STACK_LIMIT];
    char delimiter_stack[LM_P0_LAYOUT_DELIMITER_STACK_LIMIT];
    char quote;
    i = start;
    depth = 0U;
    quote = '\0';
    lm_p0_scan_layout_prefix(lm_lmx_message_thread, source, length, start, &ignored_content_offset, &current_line_indent, &current_line_dot_level);
    while (i < length) {
        if (quote != '\0') {
            if ((quote == '"') && (source[i] == '\\')) {
                size_t line_break_width;
                if ((i + 1U) < length) {
                    line_break_width = lm_p0_line_break_width_at(lm_lmx_message_thread, source, length, (i + 1U));
                }
                else {
                    line_break_width = 0U;
                }
                if (line_break_width > 0U) {
                    i = i + (1U + line_break_width);
                }
                else {
                    if ((i + 1U) < length) {
                        i = i + 2U;
                    }
                    else {
                        i = i + 1U;
                    }
                }
                continue;
            }
            if ((((quote == '`') && (source[i] == '`')) && ((i + 1U) < length)) && (source[(i + 1U)] == '`')) {
                i = i + 2U;
                continue;
            }
            if (source[i] == quote) {
                quote = '\0';
            }
            i = i + 1U;
            continue;
        }
        if (lm_p0_starts_python_string(lm_lmx_message_thread, source, length, i)) {
            i = lm_p0_skip_python_string_unchecked(lm_lmx_message_thread, source, length, i);
            continue;
        }
        if (lm_p0_scan_c_prefixed_quote_token(lm_lmx_message_thread, source, length, i) > i) {
            i = lm_p0_scan_c_prefixed_quote_token(lm_lmx_message_thread, source, length, i);
            continue;
        }
        if ((source[i] == '\'') && (lm_p0_starts_python_string(lm_lmx_message_thread, source, length, i) == 0)) {
            i = lm_p0_scan_c_char_token(lm_lmx_message_thread, source, length, i);
            continue;
        }
        if ((source[i] == '"') || (source[i] == '`')) {
            char lm_p0_tmp_9;
            lm_p0_tmp_9 = source[i];
            i = i + 1U;
            quote = lm_p0_tmp_9;
            continue;
        }
        if (source[i] == '#') {
            while ((i < length) && (lm_p0_is_line_break(lm_lmx_message_thread, source[i]) == 0)) {
                i = i + 1U;
            }
            continue;
        }
        if (source[i] == '{') {
            int closed;
            size_t brace_end;
            brace_end = lm_p0_scan_brace_mark_unchecked(source, length, i, &closed);
            if (closed) {
                i = brace_end;
                continue;
            }
        }
        if (lm_p0_is_line_break(lm_lmx_message_thread, source[i])) {
            size_t line_break_width;
            size_t next_line_start;
            size_t next_content_offset;
            size_t next_line_indent;
            size_t next_line_dot_level;
            size_t base_indent;
            size_t base_dot_level;
            int next_line_starts_with_matching_close;
            if (depth == 0U) {
                break;
            }
            line_break_width = lm_p0_line_break_width_at(lm_lmx_message_thread, source, length, i);
            next_line_start = (i + line_break_width);
            lm_p0_scan_layout_prefix(lm_lmx_message_thread, source, length, next_line_start, &next_content_offset, &next_line_indent, &next_line_dot_level);
            next_line_starts_with_matching_close = 0;
            if ((depth <= LM_P0_LAYOUT_DELIMITER_STACK_LIMIT) && (next_content_offset < length)) {
                char top_delimiter;
                top_delimiter = delimiter_stack[(depth - 1U)];
                next_line_starts_with_matching_close = (((top_delimiter == '(') && (source[next_content_offset] == ')')) || ((top_delimiter == '[') && (source[next_content_offset] == ']')));
            }
            if (depth <= LM_P0_LAYOUT_DELIMITER_STACK_LIMIT) {
                base_indent = delimiter_indent_stack[(depth - 1U)];
                base_dot_level = delimiter_dot_stack[(depth - 1U)];
                if ((next_line_starts_with_matching_close == 0) && (lm_p0_layout_prefix_is_deeper(lm_lmx_message_thread, next_line_indent, next_line_dot_level, base_indent, base_dot_level) == 0)) {
                    break;
                }
            }
            current_line_indent = next_line_indent;
            current_line_dot_level = next_line_dot_level;
            i = next_line_start;
            continue;
        }
        if ((source[i] == '(') || (source[i] == '[')) {
            if (depth < LM_P0_LAYOUT_DELIMITER_STACK_LIMIT) {
                delimiter_indent_stack[depth] = current_line_indent;
                delimiter_dot_stack[depth] = current_line_dot_level;
                delimiter_stack[depth] = source[i];
            }
            depth = depth + 1U;
        }
        else {
            if (((source[i] == ')') || (source[i] == ']')) && (depth > 0U)) {
                depth = depth - 1U;
            }
        }
        i = i + 1U;
    }
    return i;
}

static size_t lm_p0_scan_brace_mark_unchecked(const char *text, size_t length, size_t start, int *closed) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    size_t i;
    size_t depth;
    i = start;
    depth = 0U;
    closed[0] = 0;
    while (i < length) {
        size_t prefixed_end;
        if (lm_p0_index_is_line_start(lm_lmx_message_thread, text, i) && ((text[i] == '=') || (text[i] == '*'))) {
            size_t fence_end;
            fence_end = lm_p0_skip_fence_block_unchecked(lm_lmx_message_thread, text, length, i, text[i]);
            if (fence_end > i) {
                i = fence_end;
                continue;
            }
        }
        if (lm_p0_starts_python_string(lm_lmx_message_thread, text, length, i)) {
            i = lm_p0_skip_python_string_unchecked(lm_lmx_message_thread, text, length, i);
            continue;
        }
        prefixed_end = lm_p0_scan_c_prefixed_quote_token(lm_lmx_message_thread, text, length, i);
        if (prefixed_end > i) {
            i = prefixed_end;
            continue;
        }
        if ((text[i] == '\'') && (lm_p0_starts_python_string(lm_lmx_message_thread, text, length, i) == 0)) {
            i = lm_p0_scan_c_char_token(lm_lmx_message_thread, text, length, i);
            continue;
        }
        if ((text[i] == '"') || (text[i] == '`')) {
            char quote;
            char lm_p0_tmp_10;
            lm_p0_tmp_10 = text[i];
            i = i + 1U;
            quote = lm_p0_tmp_10;
            while (i < length) {
                if ((quote == '"') && (text[i] == '\\')) {
                    size_t line_break_width;
                    if ((i + 1U) < length) {
                        line_break_width = lm_p0_line_break_width_at(lm_lmx_message_thread, text, length, (i + 1U));
                    }
                    else {
                        line_break_width = 0U;
                    }
                    if (line_break_width > 0U) {
                        i = i + (1U + line_break_width);
                    }
                    else {
                        if ((i + 1U) < length) {
                            i = i + 2U;
                        }
                        else {
                            i = i + 1U;
                        }
                    }
                    continue;
                }
                if ((((quote == '`') && (text[i] == '`')) && ((i + 1U) < length)) && (text[(i + 1U)] == '`')) {
                    i = i + 2U;
                    continue;
                }
                char lm_p0_tmp_11;
                lm_p0_tmp_11 = text[i];
                i = i + 1U;
                if (lm_p0_tmp_11 == quote) {
                    break;
                }
            }
            continue;
        }
        if (text[i] == '#') {
            while ((i < length) && (lm_p0_is_line_break(lm_lmx_message_thread, text[i]) == 0)) {
                i = i + 1U;
            }
            continue;
        }
        if (text[i] == '{') {
            depth = depth + 1U;
            i = i + 1U;
            continue;
        }
        if (text[i] == '}') {
            i = i + 1U;
            if (depth > 0U) {
                depth = depth - 1U;
                if (depth == 0U) {
                    closed[0] = 1;
                    return i;
                }
            }
            continue;
        }
        i = i + 1U;
    }
    return i;
}

static int lm_p0_skip_brace_mark_ex(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmP0Structure *structure, const char *text, size_t length, size_t *index, size_t line, size_t column, int record) {
    (void)lm_lmx_message_thread;
    size_t start;
    size_t end_index;
    int closed;
    if ((index[0] >= length) || (text[index[0]] != '{')) {
        return 1;
    }
    start = index[0];
    end_index = lm_p0_scan_brace_mark_unchecked(text, length, start, &closed);
    index[0] = end_index;
    if (closed == 0) {
        size_t diagnostic_line;
        size_t diagnostic_column;
        lm_p0_position_in_slice(lm_lmx_message_thread, text, length, start, line, column, &diagnostic_line, &diagnostic_column);
        lm_p0_set_diagnostic(document, 31, diagnostic_line, diagnostic_column, "unterminated brace mark");
        return 0;
    }
    if (record && (lm_p0_record_mix_mark(lm_lmx_message_thread, document, structure, text, length, line, column, start, end_index) == 0)) {
        return 0;
    }
    return 1;
}

static int lm_p0_skip_brace_mark(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, const char *text, size_t length, size_t *index, size_t line, size_t column) {
    (void)lm_lmx_message_thread;
    return lm_p0_skip_brace_mark_ex(lm_lmx_message_thread, document, 0, text, length, index, line, column, 0);
}

static int lm_p0_consume_brace_mark(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmP0Structure *structure, const char *text, size_t length, size_t *index, size_t line, size_t column) {
    (void)lm_lmx_message_thread;
    return lm_p0_skip_brace_mark_ex(lm_lmx_message_thread, document, structure, text, length, index, line, column, 1);
}

static int lm_p0_skip_leading_brace_marks_ex(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, const char *text, size_t length, size_t line, size_t column, size_t *index, int record) {
    (void)lm_lmx_message_thread;
    while (index[0] < length) {
        while ((index[0] < length) && lm_p0_is_horizontal_space(lm_lmx_message_thread, text[index[0]])) {
            index[0] = index[0] + 1U;
        }
        if ((index[0] >= length) || (text[index[0]] != '{')) {
            return 1;
        }
        if (lm_p0_skip_brace_mark_ex(lm_lmx_message_thread, document, 0, text, length, index, line, column, record) == 0) {
            return 0;
        }
    }
    return 1;
}

static int lm_p0_skip_leading_brace_marks(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, const char *text, size_t length, size_t line, size_t column, size_t *index) {
    (void)lm_lmx_message_thread;
    return lm_p0_skip_leading_brace_marks_ex(lm_lmx_message_thread, document, text, length, line, column, index, 0);
}

static int lm_p0_line_is_standalone_mix_run(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, const char *text, size_t length, size_t line, size_t column, int *out_is_mix) {
    (void)lm_lmx_message_thread;
    size_t i;
    int saw_mix;
    i = 0U;
    saw_mix = 0;
    out_is_mix[0] = 0;
    while (i < length) {
        while ((i < length) && lm_p0_is_horizontal_space(lm_lmx_message_thread, text[i])) {
            i = i + 1U;
        }
        if (i >= length) {
            break;
        }
        if (text[i] == '#') {
            break;
        }
        if (text[i] != '{') {
            return 1;
        }
        if (lm_p0_skip_brace_mark(lm_lmx_message_thread, document, text, length, &i, line, column) == 0) {
            return 0;
        }
        saw_mix = 1;
    }
    out_is_mix[0] = saw_mix;
    return 1;
}

static int lm_p0_scan_leading_mix_prefix(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, const char *text, size_t length, size_t line, size_t column, size_t *out_prefix_start, size_t *out_prefix_end, size_t *out_anchor) {
    (void)lm_lmx_message_thread;
    size_t i;
    size_t prefix_start;
    size_t prefix_end;
    int saw_mix;
    i = 0U;
    while ((i < length) && lm_p0_is_horizontal_space(lm_lmx_message_thread, text[i])) {
        i = i + 1U;
    }
    if ((i >= length) || (text[i] != '{')) {
        return 1;
    }
    prefix_start = i;
    prefix_end = i;
    saw_mix = 0;
    while (i < length) {
        while ((i < length) && lm_p0_is_horizontal_space(lm_lmx_message_thread, text[i])) {
            i = i + 1U;
        }
        if ((i >= length) || (text[i] != '{')) {
            break;
        }
        if (lm_p0_skip_brace_mark(lm_lmx_message_thread, document, text, length, &i, line, column) == 0) {
            return 0;
        }
        prefix_end = i;
        saw_mix = 1;
    }
    while ((i < length) && lm_p0_is_horizontal_space(lm_lmx_message_thread, text[i])) {
        i = i + 1U;
    }
    if (((saw_mix == 0) || (i >= length)) || (text[i] == '#')) {
        return 1;
    }
    out_prefix_start[0] = prefix_start;
    out_prefix_end[0] = prefix_end;
    out_anchor[0] = i;
    return 1;
}

static LmP0DashFenceStatus lm_p0_dash_fence_status_after_comment_trim(const char *text, size_t length, size_t *out_dash_count) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    const char *trimmed_text;
    size_t trimmed_length;
    trimmed_text = text;
    trimmed_length = length;
    lm_p0_trim_trailing_line_comment(lm_lmx_message_thread, &trimmed_text, &trimmed_length);
    return lm_p0_dash_fence_status(lm_lmx_message_thread, trimmed_text, trimmed_length, out_dash_count);
}

static int lm_p0_scan_raw_comment_block(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, const char *source, size_t length, size_t line_start, size_t line, size_t *next_offset, size_t *next_line) {
    (void)lm_lmx_message_thread;
    size_t line_end;
    size_t star_count;
    size_t scan_start;
    size_t scan_line;
    if ((line_start >= length) || (source[line_start] != '*')) {
        return 0;
    }
    line_end = lm_p0_find_physical_line_end(lm_lmx_message_thread, source, length, line_start);
    star_count = 0U;
    while (((line_start + star_count) < line_end) && (source[(line_start + star_count)] == '*')) {
        star_count = star_count + 1U;
    }
    if (star_count < 3U) {
        return 0;
    }
    if (star_count > LM_P0_MAX_FENCE_LENGTH) {
        lm_p0_set_diagnostic(document, 23, line, 1U, "raw comment fence length exceeds 80 characters");
        return 0;
    }
    if (lm_p0_match_raw_comment_fence_line(lm_lmx_message_thread, source, line_start, line_end, star_count) == 0) {
        lm_p0_set_diagnostic(document, 27, line, 1U, "raw comment fence line must contain only the star fence and whitespace");
        return 0;
    }
    scan_start = line_end;
    if (scan_start < length) {
        scan_start = scan_start + lm_p0_line_break_width_at(lm_lmx_message_thread, source, length, scan_start);
    }
    scan_line = (line + 1U);
    while (scan_start <= length) {
        size_t current_end;
        size_t break_width;
        current_end = lm_p0_find_physical_line_end(lm_lmx_message_thread, source, length, scan_start);
        if (lm_p0_match_raw_comment_fence_line(lm_lmx_message_thread, source, scan_start, current_end, star_count)) {
            if (current_end < length) {
                break_width = lm_p0_line_break_width_at(lm_lmx_message_thread, source, length, current_end);
            }
            else {
                break_width = 0U;
            }
            next_offset[0] = (current_end + break_width);
            size_t lm_p0_tmp_12;
            if (break_width > 0U) {
                lm_p0_tmp_12 = 1U;
            }
            else {
                lm_p0_tmp_12 = 0U;
            }
            next_line[0] = (scan_line + lm_p0_tmp_12);
            return 1;
        }
        if (current_end == length) {
            break;
        }
        scan_start = (current_end + lm_p0_line_break_width_at(lm_lmx_message_thread, source, length, current_end));
        scan_line = scan_line + 1U;
    }
    lm_p0_set_diagnostic(document, 24, line, 1U, "unterminated raw comment block");
    return 0;
}

static int lm_p0_scan_block_string_event(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, const char *source, size_t length, size_t line_start, size_t line, LmP0StreamEvent *event, size_t *next_offset, size_t *next_line) {
    (void)lm_lmx_message_thread;
    size_t line_end;
    size_t eq_count;
    size_t content_start;
    size_t content_end;
    size_t scan_start;
    size_t scan_line;
    if ((line_start >= length) || (source[line_start] != '=')) {
        return 0;
    }
    line_end = lm_p0_find_physical_line_end(lm_lmx_message_thread, source, length, line_start);
    eq_count = 0U;
    while (((line_start + eq_count) < line_end) && (source[(line_start + eq_count)] == '=')) {
        eq_count = eq_count + 1U;
    }
    if (eq_count < 3U) {
        return 0;
    }
    if (eq_count > LM_P0_MAX_FENCE_LENGTH) {
        lm_p0_set_diagnostic(document, 23, line, 1U, "block string fence length exceeds 80 characters");
        return 0;
    }
    if (lm_p0_match_block_string_fence_line(lm_lmx_message_thread, source, line_start, line_end, eq_count) == 0) {
        lm_p0_set_diagnostic(document, 29, line, 1U, "block string fence line must contain only the equals fence and whitespace");
        return 0;
    }
    scan_start = line_end;
    if (scan_start < length) {
        scan_start = scan_start + lm_p0_line_break_width_at(lm_lmx_message_thread, source, length, scan_start);
    }
    content_start = scan_start;
    content_end = content_start;
    scan_line = (line + 1U);
    while (scan_start <= length) {
        size_t current_end;
        size_t break_width;
        current_end = lm_p0_find_physical_line_end(lm_lmx_message_thread, source, length, scan_start);
        if (lm_p0_match_block_string_fence_line(lm_lmx_message_thread, source, scan_start, current_end, eq_count)) {
            if (current_end < length) {
                break_width = lm_p0_line_break_width_at(lm_lmx_message_thread, source, length, current_end);
            }
            else {
                break_width = 0U;
            }
            memset(event, 0, sizeof(event[0]));
            event->kind = LM_P0_STREAM_EVENT_BLOCK_STRING;
            event->text = (source + content_start);
            event->text_length = (content_end - content_start);
            event->line = line;
            event->column = 1U;
            event->offset = line_start;
            next_offset[0] = (current_end + break_width);
            size_t lm_p0_tmp_13;
            if (break_width > 0U) {
                lm_p0_tmp_13 = 1U;
            }
            else {
                lm_p0_tmp_13 = 0U;
            }
            next_line[0] = (scan_line + lm_p0_tmp_13);
            return 1;
        }
        if (current_end == length) {
            break;
        }
        content_end = current_end;
        scan_start = (current_end + lm_p0_line_break_width_at(lm_lmx_message_thread, source, length, current_end));
        scan_line = scan_line + 1U;
    }
    lm_p0_set_diagnostic(document, 20, line, 1U, "unterminated block string literal");
    return 0;
}

static LmP0Node * lm_p0_new_node(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmP0NodeKind kind) {
    (void)lm_lmx_message_thread;
    LmP0Node * node;
    node = (((LmP0Node *)lm_own_arena_new_zero(lm_lmx_message_thread, document -> tree_arena, sizeof(node[0]))));
    if (node == 0) {
        lm_p0_set_diagnostic(document, 1, 0U, 0U, "out of memory while allocating parser node");
        return 0;
    }
    node->span = (((LmP0Span *)lm_own_arena_new_zero(lm_lmx_message_thread, document -> tree_arena, sizeof(node -> span[0]))));
    if (node -> span == 0) {
        lm_p0_set_diagnostic(document, 1, 0U, 0U, "out of memory while allocating parser node span");
        return 0;
    }
    node->as = (((LmP0NodeAs *)lm_own_arena_new_zero(lm_lmx_message_thread, document -> tree_arena, sizeof(node -> as[0]))));
    if (node -> as == 0) {
        lm_p0_set_diagnostic(document, 1, 0U, 0U, "out of memory while allocating parser node payload");
        return 0;
    }
    node->kind = kind;
    if (kind == LM_P0_NODE_STRUCTURE) {
        node->as->structure = lm_p0_new_structure(lm_lmx_message_thread, document, 0U, 0U);
        if (node -> as -> structure == 0) {
            return 0;
        }
    }
    else {
        if (kind == LM_P0_NODE_FRAME) {
            node->as->frame = lm_p0_new_frame(lm_lmx_message_thread, document, 0U, 0U);
            if (node -> as -> frame == 0) {
                return 0;
            }
        }
        else {
            if ((kind == LM_P0_NODE_ATOM) || (kind == LM_P0_NODE_DISABLED)) {
                node->as->atom = lm_p0_new_text(lm_lmx_message_thread, document, "", 0U, 0U, 0U);
                if (node -> as -> atom == 0) {
                    return 0;
                }
            }
        }
    }
    return node;
}

static int lm_p0_append_field(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmP0Structure *structure, LmP0Node *node) {
    (void)lm_lmx_message_thread;
    LmP0Field * field;
    field = (((LmP0Field *)lm_own_arena_new_zero(lm_lmx_message_thread, document -> tree_arena, sizeof(field[0]))));
    if (field == 0) {
        size_t lm_p0_tmp_14;
        if (node != 0) {
            lm_p0_tmp_14 = node -> span -> line;
        }
        else {
            lm_p0_tmp_14 = 0U;
        }
        size_t lm_p0_tmp_15;
        if (node != 0) {
            lm_p0_tmp_15 = node -> span -> column;
        }
        else {
            lm_p0_tmp_15 = 0U;
        }
        lm_p0_set_diagnostic(document, 1, lm_p0_tmp_14, lm_p0_tmp_15, "out of memory while allocating parser field");
        return 0;
    }
    field->value = node;
    if (structure -> last_field == 0) {
        structure->first_field = field;
    }
    else {
        structure->last_field->next = field;
    }
    structure->last_field = field;
    structure->field_count = structure -> field_count + 1U;
    return 1;
}

static int lm_p0_pointer_source_offset(struct LmMessageThread *lm_lmx_message_thread, const LmP0Document *document, const char *pointer, size_t *out_offset) {
    (void)lm_lmx_message_thread;
    const char *begin;
    const char *end_index;
    if (((document == 0) || (document -> source == 0)) || (pointer == 0)) {
        return 0;
    }
    begin = document -> source;
    end_index = (document -> source + document -> source_length);
    if ((pointer < begin) || (pointer > end_index)) {
        return 0;
    }
    out_offset[0] = (((size_t)(pointer - begin)));
    return 1;
}

static size_t lm_p0_offset_from_line_column(struct LmMessageThread *lm_lmx_message_thread, const char *text, size_t length, size_t line, size_t column) {
    (void)lm_lmx_message_thread;
    size_t i;
    size_t current_line;
    size_t current_column;
    i = 0U;
    current_line = 1U;
    current_column = 1U;
    while (i < length) {
        if ((current_line == line) && (current_column == column)) {
            return i;
        }
        if (text[i] == '\r') {
            current_line = current_line + 1U;
            current_column = 1U;
            if (((i + 1U) < length) && (text[(i + 1U)] == '\n')) {
                i = i + 1U;
            }
        }
        else {
            if (text[i] == '\n') {
                current_line = current_line + 1U;
                current_column = 1U;
            }
            else {
                current_column = current_column + 1U;
            }
        }
        i = i + 1U;
    }
    return i;
}

static void lm_p0_copy_payload_diagnostic(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, const LmP0Document *payload_document, size_t payload_offset) {
    (void)lm_lmx_message_thread;
    size_t local_offset;
    if (((((document == 0) || (document -> diagnostic == 0)) || (payload_document == 0)) || (payload_document -> diagnostic == 0)) || (payload_document -> diagnostic -> code == 0)) {
        return;
    }
    document->diagnostic[0] = payload_document -> diagnostic[0];
    local_offset = lm_p0_offset_from_line_column(lm_lmx_message_thread, payload_document -> source, payload_document -> source_length, payload_document -> diagnostic -> line, payload_document -> diagnostic -> column);
    lm_p0_position_in_slice(lm_lmx_message_thread, document -> source, document -> source_length, (payload_offset + local_offset), 1U, 1U, &document -> diagnostic -> line, &document -> diagnostic -> column);
}

static void lm_p0_adjust_node_span_to_document(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmP0Node *node, size_t base_offset) {
    (void)lm_lmx_message_thread;
    if (node == 0) {
        return;
    }
    node->span->offset = node -> span -> offset + base_offset;
    lm_p0_position_in_slice(lm_lmx_message_thread, document -> source, document -> source_length, node -> span -> offset, 1U, 1U, &node -> span -> line, &node -> span -> column);
    if (node -> kind == LM_P0_NODE_STRUCTURE) {
        lm_p0_adjust_structure_spans_to_document(lm_lmx_message_thread, document, node -> as -> structure, base_offset);
        if (node -> as -> structure -> trailer != 0) {
            lm_p0_adjust_structure_spans_to_document(lm_lmx_message_thread, document, node -> as -> structure -> trailer -> body, base_offset);
        }
    }
    else {
        if (node -> kind == LM_P0_NODE_FRAME) {
            lm_p0_adjust_structure_spans_to_document(lm_lmx_message_thread, document, node -> as -> frame -> body, base_offset);
            if (node -> as -> frame -> trailer != 0) {
                lm_p0_adjust_structure_spans_to_document(lm_lmx_message_thread, document, node -> as -> frame -> trailer -> body, base_offset);
            }
        }
    }
}

static void lm_p0_adjust_structure_spans_to_document(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmP0Structure *structure, size_t base_offset) {
    (void)lm_lmx_message_thread;
    LmP0Field * field;
    field = structure -> first_field;
    while (field != 0) {
        lm_p0_adjust_node_span_to_document(lm_lmx_message_thread, document, field -> value, base_offset);
        field = field -> next;
    }
}

static int lm_p0_record_mix_mark(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmP0Structure *structure, const char *text, size_t length, size_t line, size_t column, size_t start, size_t end) {
    (void)lm_lmx_message_thread;
    LmP0Node * node;
    size_t span_line;
    size_t span_column;
    size_t absolute_offset;
    size_t payload_offset;
    size_t payload_length;
    LmP0Document * payload_document;
    int status;
    node = lm_p0_new_node(lm_lmx_message_thread, document, LM_P0_NODE_STRUCTURE);
    if (node == 0) {
        return 0;
    }
    node->flags = node -> flags | LM_P0_NODE_MIX;
    lm_p0_position_in_slice(lm_lmx_message_thread, text, length, start, line, column, &span_line, &span_column);
    node->span->line = span_line;
    node->span->column = span_column;
    node->span->length = end - start;
    if (lm_p0_pointer_source_offset(lm_lmx_message_thread, document, text + start, &absolute_offset)) {
        node->span->offset = absolute_offset;
    }
    if (lm_p0_append_field(lm_lmx_message_thread, document, structure, node) == 0) {
        lm_p0_free_node(lm_lmx_message_thread, node);
        return 0;
    }
    payload_offset = node -> span -> offset + 1U;
    if (end > start + 2U) {
        payload_length = end - start - 2U;
    }
    else {
        payload_length = 0U;
    }
    if (payload_length > 0U) {
        payload_document = lm_own_new_zero(sizeof(payload_document[0]));
        if (payload_document == 0) {
            lm_p0_set_diagnostic(document, 1, span_line, span_column, "out of memory while creating MIX payload document");
            return 0;
        }
        status = 0;
        if (lm_p0_document_init_owners(lm_lmx_message_thread, payload_document) != 0) {
            lm_p0_set_diagnostic(document, 1, span_line, span_column, "out of memory while creating MIX payload owners");
        }
        else {
            payload_document->source = (((char *)(text + start + 1U)));
            payload_document->source_length = payload_length;
            if (lm_p0_parse_stream(lm_lmx_message_thread, payload_document) == 0) {
                lm_p0_copy_payload_diagnostic(lm_lmx_message_thread, document, payload_document, payload_offset);
            }
            else {
                if (lm_p0_postprocess_node(lm_lmx_message_thread, payload_document, payload_document -> root) == 0) {
                    lm_p0_copy_payload_diagnostic(lm_lmx_message_thread, document, payload_document, payload_offset);
                }
                else {
                    if (lm_p0_validate_nonempty_colon_frames_in_node(lm_lmx_message_thread, payload_document, payload_document -> root) == 0) {
                        lm_p0_copy_payload_diagnostic(lm_lmx_message_thread, document, payload_document, payload_offset);
                    }
                    else {
                        if (lm_own_tree_cut(lm_lmx_message_thread, payload_document -> tree_arena) != 0) {
                            lm_p0_set_diagnostic(document, 1, span_line, span_column, "out of memory while promoting MIX lazy text edges");
                        }
                        else {
                            node->as->structure = payload_document -> root -> as -> structure;
                            payload_document->root->as->structure = 0;
                            if (lm_own_arena_absorb(lm_lmx_message_thread, document -> tree_arena, payload_document -> tree_arena) != 0) {
                                lm_p0_set_diagnostic(document, 1, span_line, span_column, "out of memory while moving MIX tree into parser arena");
                            }
                            else {
                                lm_p0_adjust_structure_spans_to_document(lm_lmx_message_thread, document, node -> as -> structure, payload_offset);
                                status = 1;
                            }
                        }
                    }
                }
            }
        }
        lm_p0_document_destroy_owners(lm_lmx_message_thread, payload_document);
        lm_own_delete(payload_document, 0);
        if (status == 0) {
            return 0;
        }
    }
    return 1;
}

static int lm_p0_is_bare_identifier_start(struct LmMessageThread *lm_lmx_message_thread, char c) {
    (void)lm_lmx_message_thread;
    unsigned char value;
    value = (((unsigned char)c));
    return (isalpha(value) || (c == '_'));
}

static int lm_p0_is_bare_identifier_rest(struct LmMessageThread *lm_lmx_message_thread, char c) {
    (void)lm_lmx_message_thread;
    unsigned char value;
    value = (((unsigned char)c));
    return (isalnum(value) || (c == '_'));
}

static int lm_p0_is_hex_digit(struct LmMessageThread *lm_lmx_message_thread, char c) {
    (void)lm_lmx_message_thread;
    unsigned char value;
    value = (((unsigned char)c));
    return (isxdigit(value) != 0);
}

static int lm_p0_scan_number_token(struct LmMessageThread *lm_lmx_message_thread, const char *text, size_t end_index, size_t start, size_t *out_end) {
    (void)lm_lmx_message_thread;
    size_t i;
    int saw_digit;
    i = start;
    saw_digit = 0;
    if ((i < end_index) && ((text[i] == '+') || (text[i] == '-'))) {
        if (((i + 1U) < end_index) && lm_p0_is_decimal_digit(lm_lmx_message_thread, text[(i + 1U)])) {
            i = i + 1U;
        }
        else {
            out_end[0] = start;
            return 0;
        }
    }
    if ((((i + 1U) < end_index) && (text[i] == '0')) && ((text[(i + 1U)] == 'x') || (text[(i + 1U)] == 'X'))) {
        i = i + 2U;
        while ((i < end_index) && lm_p0_is_hex_digit(lm_lmx_message_thread, text[i])) {
            saw_digit = 1;
            i = i + 1U;
        }
        if ((i < end_index) && (text[i] == '.')) {
            i = i + 1U;
            while ((i < end_index) && lm_p0_is_hex_digit(lm_lmx_message_thread, text[i])) {
                saw_digit = 1;
                i = i + 1U;
            }
        }
        if ((saw_digit && (i < end_index)) && ((text[i] == 'p') || (text[i] == 'P'))) {
            size_t exponent_start;
            exponent_start = i;
            i = i + 1U;
            if ((i < end_index) && ((text[i] == '+') || (text[i] == '-'))) {
                i = i + 1U;
            }
            if ((i < end_index) && lm_p0_is_decimal_digit(lm_lmx_message_thread, text[i])) {
                while ((i < end_index) && lm_p0_is_decimal_digit(lm_lmx_message_thread, text[i])) {
                    i = i + 1U;
                }
            }
            else {
                i = exponent_start;
            }
        }
        while ((i < end_index) && lm_p0_is_bare_identifier_rest(lm_lmx_message_thread, text[i])) {
            i = i + 1U;
        }
        if (saw_digit) {
            out_end[0] = i;
        }
        else {
            out_end[0] = (start + 1U);
        }
        return 1;
    }
    while ((i < end_index) && lm_p0_is_decimal_digit(lm_lmx_message_thread, text[i])) {
        saw_digit = 1;
        i = i + 1U;
    }
    if ((i < end_index) && (text[i] == '.')) {
        i = i + 1U;
        while ((i < end_index) && lm_p0_is_decimal_digit(lm_lmx_message_thread, text[i])) {
            saw_digit = 1;
            i = i + 1U;
        }
    }
    if ((saw_digit && (i < end_index)) && ((text[i] == 'e') || (text[i] == 'E'))) {
        size_t exponent_start;
        exponent_start = i;
        i = i + 1U;
        if ((i < end_index) && ((text[i] == '+') || (text[i] == '-'))) {
            i = i + 1U;
        }
        if ((i < end_index) && lm_p0_is_decimal_digit(lm_lmx_message_thread, text[i])) {
            while ((i < end_index) && lm_p0_is_decimal_digit(lm_lmx_message_thread, text[i])) {
                i = i + 1U;
            }
        }
        else {
            i = exponent_start;
        }
    }
    while ((i < end_index) && lm_p0_is_bare_identifier_rest(lm_lmx_message_thread, text[i])) {
        i = i + 1U;
    }
    if (saw_digit) {
        out_end[0] = i;
    }
    else {
        out_end[0] = start;
    }
    return saw_digit;
}

static size_t lm_p0_scan_c_quoted_token(struct LmMessageThread *lm_lmx_message_thread, const char *text, size_t end_index, size_t quote_index) {
    (void)lm_lmx_message_thread;
    size_t i;
    char quote;
    if ((quote_index >= end_index) || ((text[quote_index] != '\'') && (text[quote_index] != '"'))) {
        return quote_index;
    }
    quote = text[quote_index];
    i = (quote_index + 1U);
    while (i < end_index) {
        if ((text[i] == '\\') && ((i + 1U) < end_index)) {
            size_t line_break_width;
            line_break_width = lm_p0_line_break_width_at(lm_lmx_message_thread, text, end_index, (i + 1U));
            if (line_break_width > 0U) {
                i = i + (1U + line_break_width);
            }
            else {
                i = i + 2U;
            }
            continue;
        }
        if (text[i] == quote) {
            return (i + 1U);
        }
        if (lm_p0_is_line_break(lm_lmx_message_thread, text[i])) {
            return (quote_index + 1U);
        }
        i = i + 1U;
    }
    return (quote_index + 1U);
}

static int lm_p0_starts_c_prefixed_quote(struct LmMessageThread *lm_lmx_message_thread, const char *text, size_t end_index, size_t start) {
    (void)lm_lmx_message_thread;
    return ((((start + 1U) < end_index) && (text[start] == 'L')) && ((text[(start + 1U)] == '\'') || (text[(start + 1U)] == '"')));
}

static size_t lm_p0_scan_c_char_token(struct LmMessageThread *lm_lmx_message_thread, const char *text, size_t end_index, size_t start) {
    (void)lm_lmx_message_thread;
    if ((start >= end_index) || (text[start] != '\'')) {
        return start;
    }
    return lm_p0_scan_c_quoted_token(lm_lmx_message_thread, text, end_index, start);
}

static size_t lm_p0_scan_c_prefixed_quote_token(struct LmMessageThread *lm_lmx_message_thread, const char *text, size_t end_index, size_t start) {
    (void)lm_lmx_message_thread;
    size_t quoted_end;
    if (lm_p0_starts_c_prefixed_quote(lm_lmx_message_thread, text, end_index, start) == 0) {
        return start;
    }
    quoted_end = lm_p0_scan_c_quoted_token(lm_lmx_message_thread, text, end_index, (start + 1U));
    if (quoted_end > (start + 2U)) {
        return quoted_end;
    }
    return (start + 1U);
}

static int lm_p0_starts_c_surface_atom(struct LmMessageThread *lm_lmx_message_thread, const char *text, size_t end_index, size_t start) {
    (void)lm_lmx_message_thread;
    return ((((start + 1U) < end_index) && (text[start] == 'c')) && (text[(start + 1U)] == '.'));
}

static int lm_p0_is_c_surface_top_boundary(struct LmMessageThread *lm_lmx_message_thread, char value) {
    (void)lm_lmx_message_thread;
    return ((((((lm_p0_is_field_space(lm_lmx_message_thread, value) || lm_p0_is_field_separator(lm_lmx_message_thread, value)) || (value == '(')) || (value == ')')) || (value == ':')) || (value == '#')) || (value == '{'));
}

static size_t lm_p0_scan_c_sizeof_surface_atom(struct LmMessageThread *lm_lmx_message_thread, const char *text, size_t end_index, size_t start) {
    (void)lm_lmx_message_thread;
    size_t i;
    size_t close_index;
    size_t depth;
    int raw_safe;
    if ((((start + 8U) >= end_index) || (memcmp((text + start), "c.sizeof", 8U) != 0)) || (text[(start + 8U)] != '(')) {
        return start;
    }
    i = (start + 9U);
    depth = 1U;
    while (i < end_index) {
        if (lm_p0_is_line_break(lm_lmx_message_thread, text[i])) {
            return start;
        }
        if (lm_p0_starts_c_prefixed_quote(lm_lmx_message_thread, text, end_index, i)) {
            size_t quoted_end;
            quoted_end = lm_p0_scan_c_prefixed_quote_token(lm_lmx_message_thread, text, end_index, i);
            if (quoted_end > (i + 1U)) {
                i = quoted_end;
                continue;
            }
        }
        if ((text[i] == '\'') || (text[i] == '"')) {
            size_t quoted_end;
            quoted_end = lm_p0_scan_c_quoted_token(lm_lmx_message_thread, text, end_index, i);
            if (quoted_end > (i + 1U)) {
                i = quoted_end;
                continue;
            }
        }
        if (text[i] == '(') {
            depth = depth + 1U;
        }
        else {
            if (text[i] == ')') {
                depth = depth - 1U;
                if (depth == 0U) {
                    break;
                }
            }
        }
        i = i + 1U;
    }
    if ((i >= end_index) || (text[i] != ')')) {
        return start;
    }
    close_index = i;
    raw_safe = 1;
    i = (start + 9U);
    while (i < close_index) {
        if (((text[i] == '\\') || (text[i] == '@')) || ((((i + 1U) < close_index) && (text[i] == 'c')) && (text[(i + 1U)] == '.'))) {
            raw_safe = 0;
            break;
        }
        i = i + 1U;
    }
    if (raw_safe) {
        return (close_index + 1U);
    }
    return start;
}

static size_t lm_p0_scan_c_surface_atom(struct LmMessageThread *lm_lmx_message_thread, const char *text, size_t end_index, size_t start) {
    (void)lm_lmx_message_thread;
    size_t i;
    size_t sizeof_end;
    size_t bracket_depth;
    if (lm_p0_starts_c_surface_atom(lm_lmx_message_thread, text, end_index, start) == 0) {
        return start;
    }
    sizeof_end = lm_p0_scan_c_sizeof_surface_atom(lm_lmx_message_thread, text, end_index, start);
    if (sizeof_end > start) {
        return sizeof_end;
    }
    i = (start + 2U);
    bracket_depth = 0U;
    while (i < end_index) {
        if (lm_p0_is_line_break(lm_lmx_message_thread, text[i])) {
            break;
        }
        if ((bracket_depth == 0U) && lm_p0_is_c_surface_top_boundary(lm_lmx_message_thread, text[i])) {
            break;
        }
        if (text[i] == '[') {
            bracket_depth = bracket_depth + 1U;
        }
        else {
            if ((text[i] == ']') && (bracket_depth > 0U)) {
                bracket_depth = bracket_depth - 1U;
            }
        }
        i = i + 1U;
    }
    return i;
}

static int lm_p0_scan_c_char(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, const char *text, size_t length, size_t *index, size_t line, size_t base_column) {
    (void)lm_lmx_message_thread;
    size_t end_index;
    end_index = lm_p0_scan_c_char_token(lm_lmx_message_thread, text, length, index[0]);
    if (end_index <= (index[0] + 1U)) {
        size_t diagnostic_line;
        size_t diagnostic_column;
        lm_p0_position_in_slice(lm_lmx_message_thread, text, length, index[0], line, base_column, &diagnostic_line, &diagnostic_column);
        lm_p0_set_diagnostic(document, 4, diagnostic_line, diagnostic_column, "unterminated C character literal");
        return 0;
    }
    index[0] = end_index;
    return 1;
}

static int lm_p0_scan_c_prefixed_quote(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, const char *text, size_t length, size_t *index, size_t line, size_t base_column) {
    (void)lm_lmx_message_thread;
    size_t end_index;
    end_index = lm_p0_scan_c_prefixed_quote_token(lm_lmx_message_thread, text, length, index[0]);
    if (end_index <= (index[0] + 1U)) {
        size_t diagnostic_line;
        size_t diagnostic_column;
        lm_p0_position_in_slice(lm_lmx_message_thread, text, length, index[0], line, base_column, &diagnostic_line, &diagnostic_column);
        lm_p0_set_diagnostic(document, 4, diagnostic_line, diagnostic_column, "unterminated C prefixed literal");
        return 0;
    }
    index[0] = end_index;
    return 1;
}

static size_t lm_p0_scan_builtin_compact_atom_piece(struct LmMessageThread *lm_lmx_message_thread, const char *text, size_t end_index, size_t start) {
    (void)lm_lmx_message_thread;
    if ((start + 1U) < end_index) {
        char first;
        char second;
        first = text[start];
        second = text[(start + 1U)];
        if (((((((((first == '!') && (second == '=')) || ((first == '<') && (second == '='))) || ((first == '>') && (second == '='))) || ((first == '&') && (second == '&'))) || ((first == '|') && (second == '|'))) || ((first == '+') && (second == '+'))) || ((first == '-') && (second == '-'))) || ((first == '[') && (second == ']'))) {
            return (start + 2U);
        }
    }
    return (start + 1U);
}

static size_t lm_p0_scan_registry_compact_atom_piece(struct LmMessageThread *lm_lmx_message_thread, const char *text, size_t end_index, size_t start) {
    (void)lm_lmx_message_thread;
    size_t i;
    size_t best_length;
    size_t token_length;
    const LmTableCell * key_cell;
    const LmTableCell * payload_cell;
    if (lm_p0_registry_table_has_rows_loaded_or_loading(lm_lmx_message_thread, "p0.compact-token") == 0) {
        return lm_p0_scan_builtin_compact_atom_piece(lm_lmx_message_thread, text, end_index, start);
    }
    best_length = 0U;
    i = 0U;
    payload_cell = lm_p0_registry_source_path_cell_at(lm_lmx_message_thread, "p0.compact-token", i, &key_cell);
    while (payload_cell != 0) {
        if (key_cell != 0 && key_cell -> value != 0) {
            token_length = strlen(key_cell -> value);
            if (((token_length > best_length) && ((start + token_length) <= end_index)) && (memcmp((text + start), key_cell -> value, token_length) == 0)) {
                best_length = token_length;
            }
        }
        i = i + 1U;
        payload_cell = lm_p0_registry_source_path_cell_at(lm_lmx_message_thread, "p0.compact-token", i, &key_cell);
    }
    if (best_length > 0U) {
        return (start + best_length);
    }
    return (start + 1U);
}

static size_t lm_p0_scan_compact_atom_piece(struct LmMessageThread *lm_lmx_message_thread, const char *text, size_t end_index, size_t start, int allow_signed_number) {
    (void)lm_lmx_message_thread;
    size_t i;
    if (start >= end_index) {
        return start;
    }
    if (lm_p0_starts_c_prefixed_quote(lm_lmx_message_thread, text, end_index, start)) {
        return lm_p0_scan_c_prefixed_quote_token(lm_lmx_message_thread, text, end_index, start);
    }
    if (lm_p0_starts_c_surface_atom(lm_lmx_message_thread, text, end_index, start)) {
        return lm_p0_scan_c_surface_atom(lm_lmx_message_thread, text, end_index, start);
    }
    if (lm_p0_is_bare_identifier_start(lm_lmx_message_thread, text[start])) {
        i = (start + 1U);
        while ((i < end_index) && lm_p0_is_bare_identifier_rest(lm_lmx_message_thread, text[i])) {
            i = i + 1U;
        }
        if (((((i == (start + 1U)) && (text[start] == 'c')) && ((i + 1U) < end_index)) && (text[i] == '.')) && lm_p0_is_bare_identifier_start(lm_lmx_message_thread, text[(i + 1U)])) {
            i = i + 2U;
            while ((i < end_index) && lm_p0_is_bare_identifier_rest(lm_lmx_message_thread, text[i])) {
                i = i + 1U;
            }
        }
        return i;
    }
    if (lm_p0_is_decimal_digit(lm_lmx_message_thread, text[start]) || (allow_signed_number && (((start + 1U) < end_index) && ((text[start] == '+') || (text[start] == '-')) && lm_p0_is_decimal_digit(lm_lmx_message_thread, text[(start + 1U)])))) {
        size_t number_end;
        if (lm_p0_scan_number_token(lm_lmx_message_thread, text, end_index, start, &number_end)) {
            return number_end;
        }
    }
    if ((text[start] == '\'') && (lm_p0_starts_python_string(lm_lmx_message_thread, text, end_index, start) == 0)) {
        return lm_p0_scan_c_char_token(lm_lmx_message_thread, text, end_index, start);
    }
    if ((start + 1U) < end_index) {
        char first;
        char second;
        first = text[start];
        second = text[(start + 1U)];
        if ((first == '@') && (second == '@')) {
            size_t at_end;
            at_end = (start + 2U);
            while ((at_end < end_index) && (text[at_end] == '@')) {
                at_end = at_end + 1U;
            }
            return at_end;
        }
    }
    return lm_p0_scan_registry_compact_atom_piece(lm_lmx_message_thread, text, end_index, start);
}

static int lm_p0_append_atom_slice(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmP0Structure *structure, const char *text, size_t length, size_t line, size_t column, size_t offset, size_t start, size_t end_index) {
    (void)lm_lmx_message_thread;
    LmP0Node * node;
    node = lm_p0_new_node(lm_lmx_message_thread, document, LM_P0_NODE_ATOM);
    if (node == 0) {
        return 0;
    }
    node->as->atom->data = (text + start);
    node->as->atom->length = (end_index - start);
    node->span->line = line;
    lm_p0_position_in_slice(lm_lmx_message_thread, text, length, start, line, column, &node -> span -> line, &node -> span -> column);
    node->span->offset = (offset + start);
    node->span->length = (end_index - start);
    if (lm_p0_document_register_lazy_text(lm_lmx_message_thread, document, node -> as -> atom -> data, node -> as -> atom -> length, &node -> as -> atom -> data, node -> span -> line, node -> span -> column) == 0) {
        return 0;
    }
    return lm_p0_append_field(lm_lmx_message_thread, document, structure, node);
}

static int lm_p0_append_positional_skip(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmP0Structure *structure, const char *text, size_t length, size_t line, size_t column, size_t offset, size_t index) {
    (void)lm_lmx_message_thread;
    LmP0Node * node;
    node = lm_p0_new_node(lm_lmx_message_thread, document, LM_P0_NODE_ATOM);
    if (node == 0) {
        return 0;
    }
    node->flags = node -> flags | LM_P0_NODE_POSITIONAL_SKIP;
    node->as->atom->data = "";
    node->as->atom->length = 0U;
    node->span->line = line;
    lm_p0_position_in_slice(lm_lmx_message_thread, text, length, index, line, column, &node -> span -> line, &node -> span -> column);
    node->span->offset = (offset + index);
    node->span->length = 0U;
    return lm_p0_append_field(lm_lmx_message_thread, document, structure, node);
}

static int lm_p0_append_compact_atom_pieces(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmP0Structure *structure, const char *text, size_t length, size_t line, size_t column, size_t offset, size_t start, size_t end_index) {
    (void)lm_lmx_message_thread;
    size_t i;
    int allow_signed_number;
    i = start;
    allow_signed_number = 1;
    while (i < end_index) {
        size_t piece_end;
        if (lm_p0_is_field_space(lm_lmx_message_thread, text[i])) {
            i = i + 1U;
            continue;
        }
        if (text[i] == '[') {
            size_t close_index;
            size_t inner_index;
            if (lm_p0_append_atom_slice(lm_lmx_message_thread, document, structure, text, length, line, column, offset, i, (i + 1U)) == 0) {
                return 0;
            }
            if (lm_p0_find_matching_bracket(lm_lmx_message_thread, document, text, end_index, i, line, column, &close_index) == 0) {
                return 0;
            }
            inner_index = 0U;
            if (lm_p0_parse_fields_until(lm_lmx_message_thread, document, structure, ((text + i) + 1U), ((close_index - i) - 1U), line, ((column + i) + 1U), ((offset + i) + 1U), 0U, 0U, 0U, &inner_index) == 0) {
                return 0;
            }
            if (lm_p0_append_atom_slice(lm_lmx_message_thread, document, structure, text, length, line, column, offset, close_index, (close_index + 1U)) == 0) {
                return 0;
            }
            i = (close_index + 1U);
            allow_signed_number = 0;
            continue;
        }
        piece_end = lm_p0_scan_compact_atom_piece(lm_lmx_message_thread, text, end_index, i, allow_signed_number);
        if (piece_end <= i) {
            piece_end = (i + 1U);
        }
        if (lm_p0_append_atom_slice(lm_lmx_message_thread, document, structure, text, length, line, column, offset, i, piece_end) == 0) {
            return 0;
        }
        i = piece_end;
        allow_signed_number = 0;
    }
    return 1;
}

static void lm_p0_free_node(struct LmMessageThread *lm_lmx_message_thread, LmP0Node *node) {
    (void)lm_lmx_message_thread;
    LM_UNUSED(node);
}

static int lm_p0_relaxed_level_from_column(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmP0IndentStack *stack, size_t column, size_t base_level, size_t line, size_t source_column, size_t *out_level) {
    (void)lm_lmx_message_thread;
    size_t i;
    size_t parent_level;
    if (column == 0U) {
        if (base_level == 0U) {
            out_level[0] = 0U;
        }
        else {
            out_level[0] = 1U;
        }
        return 1;
    }
    if (base_level == 0U) {
        parent_level = 0U;
    }
    else {
        parent_level = (base_level - 1U);
    }
    if (stack -> count == 0U) {
        if (lm_p0_indent_stack_push(lm_lmx_message_thread, document, stack, column, line, source_column) == 0) {
            return 0;
        }
        out_level[0] = base_level;
        return 1;
    }
    if (column > stack -> columns[(stack -> count - 1U)]) {
        if (lm_p0_indent_stack_push(lm_lmx_message_thread, document, stack, column, line, source_column) == 0) {
            return 0;
        }
        out_level[0] = ((base_level + stack -> count) - 1U);
        return 1;
    }
    if (column < stack -> columns[0]) {
        out_level[0] = parent_level;
        return 1;
    }
    while ((stack -> count > 0U) && (stack -> columns[(stack -> count - 1U)] > column)) {
        stack->count = stack -> count - 1U;
    }
    if (stack -> count == 0U) {
        out_level[0] = parent_level;
        return 1;
    }
    i = 0U;
    while (i < stack -> count) {
        if (stack -> columns[i] == column) {
            out_level[0] = (base_level + i);
            return 1;
        }
        i = i + 1U;
    }
    lm_p0_set_diagnostic(document, 17, line, source_column, "unindent does not match any outer indentation level");
    return 0;
}

static int lm_p0_source_level_after_line_break(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmP0IndentStack *indent_stack, const char *text, size_t length, size_t index, size_t line, size_t column, size_t base_level, size_t *content_index, size_t *out_level) {
    (void)lm_lmx_message_thread;
    size_t p;
    size_t indent_column;
    size_t dot_level;
    p = index;
    if ((((p < length) && (text[p] == '\r')) && ((p + 1U) < length)) && (text[(p + 1U)] == '\n')) {
        p = p + 2U;
    }
    else {
        if ((p < length) && lm_p0_is_line_break(lm_lmx_message_thread, text[p])) {
            p = p + 1U;
        }
    }
    indent_column = 0U;
    while ((p < length) && lm_p0_is_horizontal_space(lm_lmx_message_thread, text[p])) {
        if (text[p] == '\t') {
            indent_column = lm_p0_indent_tab_column(lm_lmx_message_thread, indent_column);
        }
        else {
            indent_column = indent_column + 1U;
        }
        p = p + 1U;
    }
    dot_level = 0U;
    while ((p < length) && (text[p] == '.')) {
        dot_level = dot_level + 1U;
        p = p + 1U;
        while ((p < length) && lm_p0_is_horizontal_space(lm_lmx_message_thread, text[p])) {
            p = p + 1U;
        }
    }
    content_index[0] = p;
    if (dot_level > 0U) {
        out_level[0] = dot_level;
        return 1;
    }
    size_t diagnostic_line;
    size_t diagnostic_column;
    lm_p0_position_in_slice(lm_lmx_message_thread, text, length, p, line, column, &diagnostic_line, &diagnostic_column);
    return lm_p0_relaxed_level_from_column(lm_lmx_message_thread, document, indent_stack, indent_column, base_level, diagnostic_line, diagnostic_column, out_level);
}

static int lm_p0_skip_field_space(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmP0IndentStack *indent_stack, LmP0Structure *structure, const char *text, size_t length, size_t *index, size_t line, size_t column, unsigned flags, size_t short_source_level, size_t layout_base_level, size_t *current_source_level, int *stopped_by_source_level) {
    (void)lm_lmx_message_thread;
    stopped_by_source_level[0] = 0;
    while (index[0] < length) {
        if (lm_p0_is_horizontal_space(lm_lmx_message_thread, text[index[0]])) {
            index[0] = index[0] + 1U;
            continue;
        }
        if (text[index[0]] == '#') {
            while ((index[0] < length) && (lm_p0_is_line_break(lm_lmx_message_thread, text[index[0]]) == 0)) {
                index[0] = index[0] + 1U;
            }
            continue;
        }
        if (text[index[0]] == '{') {
            if (lm_p0_consume_brace_mark(lm_lmx_message_thread, document, structure, text, length, index, line, column) == 0) {
                return 0;
            }
            continue;
        }
        if (lm_p0_is_line_break(lm_lmx_message_thread, text[index[0]])) {
            size_t content_index;
            size_t next_level;
            if (lm_p0_source_level_after_line_break(lm_lmx_message_thread, document, indent_stack, text, length, index[0], line, column, layout_base_level, &content_index, &next_level) == 0) {
                return 0;
            }
            if ((((((flags & LM_P0_FIELD_PARSE_REQUIRE_BOUNDED_SOURCE_LEVEL) != 0U) && (next_level < short_source_level)) && (content_index < length)) && (text[content_index] != ')')) && (text[content_index] != ']')) {
                size_t diagnostic_line;
                size_t diagnostic_column;
                lm_p0_position_in_slice(lm_lmx_message_thread, text, length, content_index, line, column, &diagnostic_line, &diagnostic_column);
                lm_p0_set_diagnostic(document, 13, diagnostic_line, diagnostic_column, "bounded form continuation must stay inside the form");
                return 0;
            }
            if (((flags & LM_P0_FIELD_PARSE_STOP_ON_SOURCE_LEVEL) != 0U) && (next_level <= short_source_level)) {
                stopped_by_source_level[0] = 1;
                return 1;
            }
            current_source_level[0] = next_level;
            index[0] = content_index;
            continue;
        }
        break;
    }
    return 1;
}

static int lm_p0_scan_python_string(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, const char *text, size_t length, size_t *index, size_t line, size_t base_column) {
    (void)lm_lmx_message_thread;
    size_t end_index;
    if (lm_p0_find_python_string_end(lm_lmx_message_thread, text, length, index[0], &end_index) == 0) {
        size_t diagnostic_line;
        size_t diagnostic_column;
        lm_p0_position_in_slice(lm_lmx_message_thread, text, length, index[0], line, base_column, &diagnostic_line, &diagnostic_column);
        lm_p0_set_diagnostic(document, 4, diagnostic_line, diagnostic_column, "unterminated python-like string literal");
        return 0;
    }
    index[0] = end_index;
    return 1;
}

static int lm_p0_scan_quoted(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, const char *text, size_t length, size_t *index, char quote, size_t line, size_t base_column) {
    (void)lm_lmx_message_thread;
    size_t i;
    if (lm_p0_starts_python_string(lm_lmx_message_thread, text, length, index[0])) {
        return lm_p0_scan_python_string(lm_lmx_message_thread, document, text, length, index, line, base_column);
    }
    i = (index[0] + 1U);
    while (i < length) {
        if (quote == '"') {
            if (text[i] == '\\') {
                size_t line_break_width;
                if ((i + 1U) < length) {
                    line_break_width = lm_p0_line_break_width_at(lm_lmx_message_thread, text, length, (i + 1U));
                }
                else {
                    line_break_width = 0U;
                }
                if (line_break_width > 0U) {
                    i = i + (1U + line_break_width);
                    continue;
                }
                if ((i + 1U) < length) {
                    i = i + 2U;
                }
                else {
                    i = i + 1U;
                }
                continue;
            }
            if (lm_p0_is_line_break(lm_lmx_message_thread, text[i])) {
                size_t diagnostic_line;
                size_t diagnostic_column;
                lm_p0_position_in_slice(lm_lmx_message_thread, text, length, i, line, base_column, &diagnostic_line, &diagnostic_column);
                lm_p0_set_diagnostic(document, 19, diagnostic_line, diagnostic_column, "unescaped newline in string literal");
                return 0;
            }
        }
        if ((quote == '`') && (text[i] == '\0')) {
            size_t diagnostic_line;
            size_t diagnostic_column;
            lm_p0_position_in_slice(lm_lmx_message_thread, text, length, i, line, base_column, &diagnostic_line, &diagnostic_column);
            lm_p0_set_diagnostic(document, 30, diagnostic_line, diagnostic_column, "NUL byte in exact quoted identifier");
            return 0;
        }
        if ((((quote == '`') && (text[i] == '`')) && ((i + 1U) < length)) && (text[(i + 1U)] == '`')) {
            i = i + 2U;
            continue;
        }
        if (text[i] == quote) {
            index[0] = (i + 1U);
            return 1;
        }
        i = i + 1U;
    }
    size_t diagnostic_line;
    size_t diagnostic_column;
    lm_p0_position_in_slice(lm_lmx_message_thread, text, length, index[0], line, base_column, &diagnostic_line, &diagnostic_column);
    lm_p0_set_diagnostic(document, 4, diagnostic_line, diagnostic_column, "unterminated quoted token");
    return 0;
}

static int lm_p0_require_quoted_token_boundary(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, const char *text, size_t length, size_t index, size_t line, size_t column) {
    (void)lm_lmx_message_thread;
    size_t diagnostic_line;
    size_t diagnostic_column;
    if ((index >= length) || lm_p0_is_quoted_token_boundary(lm_lmx_message_thread, text[index])) {
        return 1;
    }
    lm_p0_position_in_slice(lm_lmx_message_thread, text, length, index, line, column, &diagnostic_line, &diagnostic_column);
    lm_p0_set_diagnostic(document, 18, diagnostic_line, diagnostic_column, "missing separator after quoted token");
    return 0;
}

static int lm_p0_find_matching_paren(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, const char *text, size_t length, size_t open_index, size_t line, size_t base_column, size_t *close_index) {
    (void)lm_lmx_message_thread;
    size_t i;
    size_t depth;
    i = open_index;
    depth = 0U;
    while (i < length) {
        if (text[i] == '#') {
            while ((i < length) && (lm_p0_is_line_break(lm_lmx_message_thread, text[i]) == 0)) {
                i = i + 1U;
            }
            continue;
        }
        if (lm_p0_starts_python_string(lm_lmx_message_thread, text, length, i)) {
            if (lm_p0_scan_python_string(lm_lmx_message_thread, document, text, length, &i, line, base_column) == 0) {
                return 0;
            }
            continue;
        }
        if (lm_p0_starts_c_prefixed_quote(lm_lmx_message_thread, text, length, i)) {
            if (lm_p0_scan_c_prefixed_quote(lm_lmx_message_thread, document, text, length, &i, line, base_column) == 0) {
                return 0;
            }
            continue;
        }
        if ((text[i] == '\'') && (lm_p0_starts_python_string(lm_lmx_message_thread, text, length, i) == 0)) {
            if (lm_p0_scan_c_char(lm_lmx_message_thread, document, text, length, &i, line, base_column) == 0) {
                return 0;
            }
            continue;
        }
        if ((text[i] == '"') || (text[i] == '`')) {
            if (lm_p0_scan_quoted(lm_lmx_message_thread, document, text, length, &i, text[i], line, base_column) == 0) {
                return 0;
            }
            continue;
        }
        if (text[i] == '(') {
            depth = depth + 1U;
        }
        else {
            if (text[i] == ')') {
                depth = depth - 1U;
                if (depth == 0U) {
                    close_index[0] = i;
                    return 1;
                }
            }
        }
        i = i + 1U;
    }
    size_t diagnostic_line;
    size_t diagnostic_column;
    lm_p0_position_in_slice(lm_lmx_message_thread, text, length, open_index, line, base_column, &diagnostic_line, &diagnostic_column);
    lm_p0_set_diagnostic(document, 5, diagnostic_line, diagnostic_column, "unclosed parenthesized form");
    return 0;
}

static int lm_p0_find_matching_bracket(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, const char *text, size_t length, size_t open_index, size_t line, size_t base_column, size_t *close_index) {
    (void)lm_lmx_message_thread;
    size_t i;
    size_t bracket_depth;
    size_t paren_depth;
    i = open_index;
    bracket_depth = 0U;
    paren_depth = 0U;
    while (i < length) {
        if (text[i] == '{') {
            if (lm_p0_skip_brace_mark(lm_lmx_message_thread, document, text, length, &i, line, base_column) == 0) {
                return 0;
            }
            continue;
        }
        if (text[i] == '#') {
            while ((i < length) && (lm_p0_is_line_break(lm_lmx_message_thread, text[i]) == 0)) {
                i = i + 1U;
            }
            continue;
        }
        if (lm_p0_starts_python_string(lm_lmx_message_thread, text, length, i)) {
            if (lm_p0_scan_python_string(lm_lmx_message_thread, document, text, length, &i, line, base_column) == 0) {
                return 0;
            }
            continue;
        }
        if (lm_p0_starts_c_prefixed_quote(lm_lmx_message_thread, text, length, i)) {
            if (lm_p0_scan_c_prefixed_quote(lm_lmx_message_thread, document, text, length, &i, line, base_column) == 0) {
                return 0;
            }
            continue;
        }
        if ((text[i] == '\'') && (lm_p0_starts_python_string(lm_lmx_message_thread, text, length, i) == 0)) {
            if (lm_p0_scan_c_char(lm_lmx_message_thread, document, text, length, &i, line, base_column) == 0) {
                return 0;
            }
            continue;
        }
        if ((text[i] == '"') || (text[i] == '`')) {
            if (lm_p0_scan_quoted(lm_lmx_message_thread, document, text, length, &i, text[i], line, base_column) == 0) {
                return 0;
            }
            continue;
        }
        if (text[i] == '(') {
            paren_depth = paren_depth + 1U;
        }
        else {
            if ((text[i] == ')') && (paren_depth > 0U)) {
                paren_depth = paren_depth - 1U;
            }
            else {
                if (text[i] == '[') {
                    bracket_depth = bracket_depth + 1U;
                }
                else {
                    if (text[i] == ']') {
                        if (bracket_depth == 0U) {
                            break;
                        }
                        bracket_depth = bracket_depth - 1U;
                        if ((bracket_depth == 0U) && (paren_depth == 0U)) {
                            close_index[0] = i;
                            return 1;
                        }
                    }
                }
            }
        }
        i = i + 1U;
    }
    size_t diagnostic_line;
    size_t diagnostic_column;
    lm_p0_position_in_slice(lm_lmx_message_thread, text, length, open_index, line, base_column, &diagnostic_line, &diagnostic_column);
    lm_p0_set_diagnostic(document, 5, diagnostic_line, diagnostic_column, "unclosed index bracket form");
    return 0;
}

static int lm_p0_find_colon(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, const char *text, size_t length, size_t line, size_t column, size_t *colon_index) {
    (void)lm_lmx_message_thread;
    size_t i;
    size_t depth;
    depth = 0U;
    i = 0U;
    while (i < length) {
        if (text[i] == '{') {
            if (lm_p0_skip_brace_mark(lm_lmx_message_thread, document, text, length, &i, line, column) == 0) {
                return -1;
            }
            continue;
        }
        if (text[i] == '#') {
            while ((i < length) && (lm_p0_is_line_break(lm_lmx_message_thread, text[i]) == 0)) {
                i = i + 1U;
            }
            continue;
        }
        if (lm_p0_starts_c_surface_atom(lm_lmx_message_thread, text, length, i)) {
            i = lm_p0_scan_c_surface_atom(lm_lmx_message_thread, text, length, i);
            continue;
        }
        if (lm_p0_starts_python_string(lm_lmx_message_thread, text, length, i)) {
            i = lm_p0_skip_python_string_unchecked(lm_lmx_message_thread, text, length, i);
            continue;
        }
        if (lm_p0_starts_c_prefixed_quote(lm_lmx_message_thread, text, length, i)) {
            if (lm_p0_scan_c_prefixed_quote(lm_lmx_message_thread, document, text, length, &i, line, column) == 0) {
                return -1;
            }
            continue;
        }
        if ((text[i] == '\'') && (lm_p0_starts_python_string(lm_lmx_message_thread, text, length, i) == 0)) {
            if (lm_p0_scan_c_char(lm_lmx_message_thread, document, text, length, &i, line, column) == 0) {
                return -1;
            }
            continue;
        }
        if ((text[i] == '"') || (text[i] == '`')) {
            char quote;
            char lm_p0_tmp_16;
            lm_p0_tmp_16 = text[i];
            i = i + 1U;
            quote = lm_p0_tmp_16;
            while (i < length) {
                if ((quote == '"') && (text[i] == '\\')) {
                    i = i + 2U;
                    continue;
                }
                if ((((quote == '`') && (text[i] == '`')) && ((i + 1U) < length)) && (text[(i + 1U)] == '`')) {
                    i = i + 2U;
                    continue;
                }
                if ((quote == '`') && (text[i] == '\0')) {
                    size_t diagnostic_line;
                    size_t diagnostic_column;
                    lm_p0_position_in_slice(lm_lmx_message_thread, text, length, i, line, column, &diagnostic_line, &diagnostic_column);
                    lm_p0_set_diagnostic(document, 30, diagnostic_line, diagnostic_column, "NUL byte in exact quoted identifier");
                    return -1;
                }
                char lm_p0_tmp_17;
                lm_p0_tmp_17 = text[i];
                i = i + 1U;
                if (lm_p0_tmp_17 == quote) {
                    break;
                }
            }
            continue;
        }
        if ((text[i] == '(') || (text[i] == '[')) {
            depth = depth + 1U;
        }
        else {
            if (((text[i] == ')') || (text[i] == ']')) && (depth > 0U)) {
                depth = depth - 1U;
            }
            else {
                if ((text[i] == ':') && (depth == 0U)) {
                    colon_index[0] = i;
                    return 1;
                }
            }
        }
        i = i + 1U;
    }
    return 0;
}

static int lm_p0_field_start_looks_explicit_frame(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, const char *text, size_t length, size_t index, size_t line, size_t column) {
    (void)lm_lmx_message_thread;
    size_t i;
    size_t head_end;
    size_t close_index;
    if (index >= length) {
        return 0;
    }
    i = index;
    if (text[i] == '(') {
        if (lm_p0_find_matching_paren(lm_lmx_message_thread, document, text, length, i, line, column, &close_index) == 0) {
            return 0;
        }
        return (((close_index + 1U) < length) && (text[(close_index + 1U)] == ':'));
    }
    if (lm_p0_starts_c_surface_atom(lm_lmx_message_thread, text, length, i)) {
        i = lm_p0_scan_c_surface_atom(lm_lmx_message_thread, text, length, i);
        head_end = i;
    }
    else {
        if (lm_p0_starts_c_prefixed_quote(lm_lmx_message_thread, text, length, i)) {
            if (lm_p0_scan_c_prefixed_quote(lm_lmx_message_thread, document, text, length, &i, line, column) == 0) {
                return 0;
            }
            head_end = i;
        }
        else {
            if ((lm_p0_starts_python_string(lm_lmx_message_thread, text, length, i) || (text[i] == '"')) || (text[i] == '`')) {
                if (lm_p0_scan_quoted(lm_lmx_message_thread, document, text, length, &i, text[i], line, column) == 0) {
                    return 0;
                }
                head_end = i;
            }
            else {
                if (text[i] == '\'') {
                    if (lm_p0_scan_c_char(lm_lmx_message_thread, document, text, length, &i, line, column) == 0) {
                        return 0;
                    }
                    head_end = i;
                }
                else {
                    while ((((((((i < length) && (lm_p0_is_field_space(lm_lmx_message_thread, text[i]) == 0)) && (lm_p0_is_field_separator(lm_lmx_message_thread, text[i]) == 0)) && (text[i] != '(')) && (text[i] != ')')) && (text[i] != '#')) && (text[i] != '{')) && (text[i] != ':')) {
                        if (text[i] == '[') {
                            size_t bracket_close_index;
                            if (lm_p0_find_matching_bracket(lm_lmx_message_thread, document, text, length, i, line, column, &bracket_close_index) == 0) {
                                return 0;
                            }
                            i = (bracket_close_index + 1U);
                            continue;
                        }
                        if (text[i] == ']') {
                            break;
                        }
                        i = i + 1U;
                    }
                    head_end = i;
                }
            }
        }
    }
    return (((head_end > index) && (i < length)) && ((text[i] == ':') || (text[i] == '(')));
}

static void lm_p0_field_parse_loop_frame_delete_any(void *object) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    LmP0FieldParseLoopFrame * frame;
    frame = (((LmP0FieldParseLoopFrame *)object));
    if (frame == 0) {
        return;
    }
    if (frame -> indent_stack_owned) {
        lm_p0_indent_stack_delete(lm_lmx_message_thread, frame -> indent_stack);
    }
    lm_own_delete(frame, 0);
}

static LmOwnPtrStack * lm_p0_field_parse_loop_stack_new(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, size_t line, size_t column) {
    (void)lm_lmx_message_thread;
    LmOwnPtrStack * stack;
    stack = (((LmOwnPtrStack *)lm_own_new_zero(sizeof(stack[0]))));
    if (stack == 0) {
        lm_p0_set_diagnostic(document, 1, line, column, "out of memory while creating parser field stack");
        return 0;
    }
    lm_own_ptr_stack_init(stack, lm_p0_field_parse_loop_frame_delete_any);
    return stack;
}

static void lm_p0_field_parse_loop_stack_delete(struct LmMessageThread *lm_lmx_message_thread, LmOwnPtrStack *stack) {
    (void)lm_lmx_message_thread;
    if (stack == 0) {
        return;
    }
    lm_own_ptr_stack_destroy(stack);
    lm_own_delete(stack, 0);
}

static int lm_p0_field_parse_loop_push(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmOwnPtrStack *stack, LmP0IndentStack *indent_stack, int indent_stack_owned, LmP0Structure *structure, const char *text, size_t length, size_t line, size_t column, size_t offset, unsigned flags, size_t short_source_level, size_t layout_base_level, size_t i, size_t current_source_level, int allow_empty_fields, int expect_field, int headless_group_after_separator, LmP0FieldParseLoopContinuation continuation, LmP0Node *node, size_t start, size_t close_index) {
    (void)lm_lmx_message_thread;
    LmP0FieldParseLoopFrame * frame;
    frame = (((LmP0FieldParseLoopFrame *)lm_own_new_zero(sizeof(frame[0]))));
    if (frame == 0) {
        lm_p0_set_diagnostic(document, 1, line, column, "out of memory while growing parser field stack");
        return 0;
    }
    frame->indent_stack = indent_stack;
    frame->structure = structure;
    frame->text = text;
    frame->length = length;
    frame->line = line;
    frame->column = column;
    frame->offset = offset;
    frame->flags = flags;
    frame->short_source_level = short_source_level;
    frame->layout_base_level = layout_base_level;
    frame->i = i;
    frame->current_source_level = current_source_level;
    frame->allow_empty_fields = allow_empty_fields;
    frame->expect_field = expect_field;
    frame->headless_group_after_separator = headless_group_after_separator;
    frame->indent_stack_owned = indent_stack_owned;
    frame->continuation = continuation;
    frame->node = node;
    frame->start = start;
    frame->close_index = close_index;
    if (lm_own_ptr_stack_push(stack, frame) != 0) {
        lm_p0_field_parse_loop_frame_delete_any(frame);
        lm_p0_set_diagnostic(document, 1, line, column, "out of memory while growing parser field stack");
        return 0;
    }
    return 1;
}

static int lm_p0_field_parse_fail(struct LmMessageThread *lm_lmx_message_thread, LmOwnPtrStack **parse_stack, LmP0IndentStack **indent_stack, int *indent_stack_owned) {
    (void)lm_lmx_message_thread;
    if ((((indent_stack_owned != 0) && indent_stack_owned[0]) && (indent_stack != 0)) && (indent_stack[0] != 0)) {
        lm_p0_indent_stack_delete(lm_lmx_message_thread, indent_stack[0]);
        indent_stack[0] = 0;
        indent_stack_owned[0] = 0;
    }
    if (parse_stack != 0) {
        lm_p0_field_parse_loop_stack_delete(lm_lmx_message_thread, parse_stack[0]);
        parse_stack[0] = 0;
    }
    return 0;
}

static int lm_p0_parse_append_node_and_update(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmP0Structure *structure, LmP0Node *node, unsigned flags, int allow_empty_fields, int *expect_field, int *headless_group_after_separator) {
    (void)lm_lmx_message_thread;
    if (lm_p0_append_field(lm_lmx_message_thread, document, structure, node) == 0) {
        return 0;
    }
    expect_field[0] = 0;
    if ((allow_empty_fields && (node -> kind == LM_P0_NODE_FRAME)) && ((node -> as -> frame -> flags & LM_P0_FRAME_SEPARATOR_CLOSED) != 0U)) {
        expect_field[0] = 1;
    }
    if ((((flags & LM_P0_FIELD_PARSE_ALLOW_HEADLESS_AFTER_SEPARATOR) != 0U) && (node -> kind == LM_P0_NODE_FRAME)) && ((node -> as -> frame -> flags & LM_P0_FRAME_SEPARATOR_CLOSED) != 0U)) {
        headless_group_after_separator[0] = 1;
    }
    return 1;
}

static int lm_p0_parse_fields_until_with_layout(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmP0IndentStack *indent_stack, LmP0Structure *structure, const char *text, size_t length, size_t line, size_t column, size_t offset, unsigned flags, size_t short_source_level, size_t initial_source_level, size_t layout_base_level, size_t *index) {
    (void)lm_lmx_message_thread;
    size_t i;
    size_t start;
    size_t head_end;
    size_t close_index;
    size_t current_source_level;
    size_t child_index;
    LmOwnPtrStack * parse_stack;
    LmP0FieldParseLoopFrame * frame;
    LmP0IndentStack * child_indent_stack;
    LmP0Node * node;
    int allow_empty_fields;
    int expect_field;
    int headless_group_after_separator;
    int indent_stack_owned;
    int restart_parse_context;
    parse_stack = lm_p0_field_parse_loop_stack_new(lm_lmx_message_thread, document, line, column);
    if (parse_stack == 0) {
        return 0;
    }
    i = index[0];
    current_source_level = initial_source_level;
    allow_empty_fields = ((flags & LM_P0_FIELD_PARSE_ALLOW_EMPTY_FIELDS) != 0U);
    expect_field = allow_empty_fields;
    headless_group_after_separator = 0;
    indent_stack_owned = 0;
    restart_parse_context = 1;
    while (restart_parse_context) {
        restart_parse_context = 0;
        while (i < length) {
            int stopped_by_source_level;
            if (lm_p0_skip_field_space(lm_lmx_message_thread, document, indent_stack, structure, text, length, &i, line, column, flags, short_source_level, layout_base_level, &current_source_level, &stopped_by_source_level) == 0) {
                return lm_p0_field_parse_fail(lm_lmx_message_thread, &parse_stack, &indent_stack, &indent_stack_owned);
            }
            if (stopped_by_source_level) {
                break;
            }
            if (i >= length) {
                break;
            }
            if (((((headless_group_after_separator && (text[i] != '(')) && (text[i] != ')')) && (text[i] != '#')) && (lm_p0_is_field_separator(lm_lmx_message_thread, text[i]) == 0)) && (lm_p0_field_start_looks_explicit_frame(lm_lmx_message_thread, document, text, length, i, line, column) == 0)) {
                LmP0Node * group_node;
                group_node = lm_p0_new_node(lm_lmx_message_thread, document, LM_P0_NODE_STRUCTURE);
                if (group_node == 0) {
                    return lm_p0_field_parse_fail(lm_lmx_message_thread, &parse_stack, &indent_stack, &indent_stack_owned);
                }
                if (lm_p0_field_parse_loop_push(lm_lmx_message_thread, document, parse_stack, indent_stack, indent_stack_owned, structure, text, length, line, column, offset, flags, short_source_level, layout_base_level, i, current_source_level, allow_empty_fields, expect_field, headless_group_after_separator, LM_P0_FIELD_PARSE_LOOP_HEADLESS_GROUP, group_node, i, 0U) == 0) {
                    lm_p0_free_node(lm_lmx_message_thread, group_node);
                    return lm_p0_field_parse_fail(lm_lmx_message_thread, &parse_stack, &indent_stack, &indent_stack_owned);
                }
                structure = group_node -> as -> structure;
                flags = ((LM_P0_FIELD_PARSE_STOP_ON_SEMICOLON | LM_P0_FIELD_PARSE_STOP_ON_SOURCE_LEVEL) | LM_P0_FIELD_PARSE_ALLOW_EMPTY_FIELDS);
                short_source_level = current_source_level;
                current_source_level = (current_source_level + 1U);
                allow_empty_fields = 1;
                expect_field = 1;
                headless_group_after_separator = 0;
                indent_stack_owned = 0;
                restart_parse_context = 1;
                break;
            }
            else {
                if (headless_group_after_separator && (lm_p0_is_field_separator(lm_lmx_message_thread, text[i]) == 0)) {
                    headless_group_after_separator = 0;
                }
            }
            if (lm_p0_index_is_line_start(lm_lmx_message_thread, text, i)) {
                size_t field_line;
                size_t field_column;
                size_t next_offset;
                size_t next_line;
                LmP0StreamEvent * block_event;
                lm_p0_position_in_slice(lm_lmx_message_thread, text, length, i, line, column, &field_line, &field_column);
                if (lm_p0_scan_raw_comment_block(lm_lmx_message_thread, document, text, length, i, field_line, &next_offset, &next_line)) {
                    i = next_offset;
                    continue;
                }
                if (document -> diagnostic -> code != 0) {
                    return lm_p0_field_parse_fail(lm_lmx_message_thread, &parse_stack, &indent_stack, &indent_stack_owned);
                }
                block_event = lm_p0_stream_event_new(lm_lmx_message_thread);
                if (block_event == 0) {
                    lm_p0_set_diagnostic(document, 1, field_line, field_column, "out of memory while creating block string event");
                    return lm_p0_field_parse_fail(lm_lmx_message_thread, &parse_stack, &indent_stack, &indent_stack_owned);
                }
                if (lm_p0_scan_block_string_event(lm_lmx_message_thread, document, text, length, i, field_line, block_event, &next_offset, &next_line)) {
                    node = lm_p0_new_node(lm_lmx_message_thread, document, LM_P0_NODE_ATOM);
                    if (node == 0) {
                        lm_p0_stream_event_delete(lm_lmx_message_thread, block_event);
                        return lm_p0_field_parse_fail(lm_lmx_message_thread, &parse_stack, &indent_stack, &indent_stack_owned);
                    }
                    node->as->atom->data = block_event -> text;
                    node->as->atom->length = block_event -> text_length;
                    node->span->line = field_line;
                    node->span->column = field_column;
                    node->span->offset = (offset + i);
                    node->span->length = (next_offset - i);
                    if (lm_p0_document_register_lazy_text(lm_lmx_message_thread, document, node -> as -> atom -> data, node -> as -> atom -> length, &node -> as -> atom -> data, node -> span -> line, node -> span -> column) == 0) {
                        lm_p0_stream_event_delete(lm_lmx_message_thread, block_event);
                        return lm_p0_field_parse_fail(lm_lmx_message_thread, &parse_stack, &indent_stack, &indent_stack_owned);
                    }
                    if (lm_p0_append_field(lm_lmx_message_thread, document, structure, node) == 0) {
                        lm_p0_free_node(lm_lmx_message_thread, node);
                        lm_p0_stream_event_delete(lm_lmx_message_thread, block_event);
                        return lm_p0_field_parse_fail(lm_lmx_message_thread, &parse_stack, &indent_stack, &indent_stack_owned);
                    }
                    lm_p0_stream_event_delete(lm_lmx_message_thread, block_event);
                    expect_field = 0;
                    i = next_offset;
                    continue;
                }
                lm_p0_stream_event_delete(lm_lmx_message_thread, block_event);
                if (document -> diagnostic -> code != 0) {
                    return lm_p0_field_parse_fail(lm_lmx_message_thread, &parse_stack, &indent_stack, &indent_stack_owned);
                }
            }
            if (lm_p0_is_field_separator(lm_lmx_message_thread, text[i])) {
                char separator;
                size_t separator_index;
                separator_index = i;
                char lm_p0_tmp_18;
                lm_p0_tmp_18 = text[i];
                i = i + 1U;
                separator = lm_p0_tmp_18;
                if (lm_p0_is_short_form_separator(lm_lmx_message_thread, separator) && ((flags & LM_P0_FIELD_PARSE_STOP_ON_SEMICOLON) != 0U)) {
                    break;
                }
                if (allow_empty_fields && expect_field) {
                    if (lm_p0_append_positional_skip(lm_lmx_message_thread, document, structure, text, length, line, column, offset, separator_index) == 0) {
                        return lm_p0_field_parse_fail(lm_lmx_message_thread, &parse_stack, &indent_stack, &indent_stack_owned);
                    }
                }
                expect_field = allow_empty_fields;
                continue;
            }
            if (text[i] == ')') {
                break;
            }
            start = i;
            head_end = i;
            node = 0;
            if (text[i] == '(') {
                if (lm_p0_find_matching_paren(lm_lmx_message_thread, document, text, length, i, line, column, &close_index) == 0) {
                    return lm_p0_field_parse_fail(lm_lmx_message_thread, &parse_stack, &indent_stack, &indent_stack_owned);
                }
                if (((close_index == (i + 1U)) && ((close_index + 1U) < length)) && (text[(close_index + 1U)] == ':')) {
                    size_t body_index;
                    unsigned body_flags;
                    int has_inline_body;
                    node = lm_p0_new_node(lm_lmx_message_thread, document, LM_P0_NODE_FRAME);
                    if (node == 0) {
                        return lm_p0_field_parse_fail(lm_lmx_message_thread, &parse_stack, &indent_stack, &indent_stack_owned);
                    }
                    node->as->frame->head->data = (text + start);
                    node->as->frame->head->length = ((close_index - start) + 1U);
                    node->as->frame->flags = LM_P0_FRAME_COLON;
                    i = (close_index + 2U);
                    while ((i < length) && lm_p0_is_horizontal_space(lm_lmx_message_thread, text[i])) {
                        i = i + 1U;
                    }
                    body_index = i;
                    has_inline_body = (((((i < length) && (lm_p0_is_line_break(lm_lmx_message_thread, text[i]) == 0)) && (lm_p0_is_field_separator(lm_lmx_message_thread, text[i]) == 0)) && (text[i] != ')')) && (text[i] != '#'));
                    if (has_inline_body) {
                        node->as->frame->flags = node -> as -> frame -> flags | LM_P0_FRAME_INLINE_BODY;
                    }
                    body_flags = ((LM_P0_FIELD_PARSE_STOP_ON_SEMICOLON | LM_P0_FIELD_PARSE_STOP_ON_SOURCE_LEVEL) | LM_P0_FIELD_PARSE_ALLOW_EMPTY_FIELDS);
                    if (lm_p0_field_parse_loop_push(lm_lmx_message_thread, document, parse_stack, indent_stack, indent_stack_owned, structure, text, length, line, column, offset, flags, short_source_level, layout_base_level, i, current_source_level, allow_empty_fields, expect_field, headless_group_after_separator, LM_P0_FIELD_PARSE_LOOP_COLON_FRAME_BODY, node, start, close_index) == 0) {
                        return lm_p0_field_parse_fail(lm_lmx_message_thread, &parse_stack, &indent_stack, &indent_stack_owned);
                    }
                    structure = node -> as -> frame -> body;
                    flags = body_flags;
                    short_source_level = current_source_level;
                    current_source_level = (current_source_level + 1U);
                    i = body_index;
                    allow_empty_fields = 1;
                    expect_field = 1;
                    headless_group_after_separator = 0;
                    indent_stack_owned = 0;
                    restart_parse_context = 1;
                    break;
                }
                else {
                    node = lm_p0_new_node(lm_lmx_message_thread, document, LM_P0_NODE_STRUCTURE);
                    if (node == 0) {
                        return lm_p0_field_parse_fail(lm_lmx_message_thread, &parse_stack, &indent_stack, &indent_stack_owned);
                    }
                    child_indent_stack = lm_p0_indent_stack_new_empty(lm_lmx_message_thread);
                    if (child_indent_stack == 0) {
                        lm_p0_set_diagnostic(document, 1, line, column, "out of memory while creating indentation stack");
                        return lm_p0_field_parse_fail(lm_lmx_message_thread, &parse_stack, &indent_stack, &indent_stack_owned);
                    }
                    if (lm_p0_field_parse_loop_push(lm_lmx_message_thread, document, parse_stack, indent_stack, indent_stack_owned, structure, text, length, line, column, offset, flags, short_source_level, layout_base_level, i, current_source_level, allow_empty_fields, expect_field, headless_group_after_separator, LM_P0_FIELD_PARSE_LOOP_BOUNDED_STRUCTURE, node, start, close_index) == 0) {
                        lm_p0_indent_stack_delete(lm_lmx_message_thread, child_indent_stack);
                        return lm_p0_field_parse_fail(lm_lmx_message_thread, &parse_stack, &indent_stack, &indent_stack_owned);
                    }
                    structure = node -> as -> structure;
                    text = ((text + i) + 1U);
                    length = ((close_index - i) - 1U);
                    column = ((column + i) + 1U);
                    offset = ((offset + i) + 1U);
                    flags = (LM_P0_FIELD_PARSE_REQUIRE_BOUNDED_SOURCE_LEVEL | LM_P0_FIELD_PARSE_ALLOW_EMPTY_FIELDS);
                    short_source_level = (current_source_level + 1U);
                    layout_base_level = (current_source_level + 1U);
                    current_source_level = (current_source_level + 1U);
                    i = 0U;
                    allow_empty_fields = 1;
                    expect_field = 1;
                    headless_group_after_separator = 0;
                    indent_stack = child_indent_stack;
                    indent_stack_owned = 1;
                    restart_parse_context = 1;
                    break;
                }
            }
            else {
                if (((text[i] == '[') && (lm_p0_field_start_looks_explicit_frame(lm_lmx_message_thread, document, text, length, i, line, column) == 0)) && ((i == 0U) || (text[i - 1U] != ')'))) {
                    if (lm_p0_find_matching_bracket(lm_lmx_message_thread, document, text, length, i, line, column, &close_index) == 0) {
                        return lm_p0_field_parse_fail(lm_lmx_message_thread, &parse_stack, &indent_stack, &indent_stack_owned);
                    }
                    node = lm_p0_new_node(lm_lmx_message_thread, document, LM_P0_NODE_STRUCTURE);
                    if (node == 0) {
                        return lm_p0_field_parse_fail(lm_lmx_message_thread, &parse_stack, &indent_stack, &indent_stack_owned);
                    }
                    child_indent_stack = lm_p0_indent_stack_new_empty(lm_lmx_message_thread);
                    if (child_indent_stack == 0) {
                        lm_p0_set_diagnostic(document, 1, line, column, "out of memory while creating indentation stack");
                        return lm_p0_field_parse_fail(lm_lmx_message_thread, &parse_stack, &indent_stack, &indent_stack_owned);
                    }
                    if (lm_p0_field_parse_loop_push(lm_lmx_message_thread, document, parse_stack, indent_stack, indent_stack_owned, structure, text, length, line, column, offset, flags, short_source_level, layout_base_level, i, current_source_level, allow_empty_fields, expect_field, headless_group_after_separator, LM_P0_FIELD_PARSE_LOOP_BOUNDED_STRUCTURE, node, start, close_index) == 0) {
                        lm_p0_indent_stack_delete(lm_lmx_message_thread, child_indent_stack);
                        return lm_p0_field_parse_fail(lm_lmx_message_thread, &parse_stack, &indent_stack, &indent_stack_owned);
                    }
                    structure = node -> as -> structure;
                    text = ((text + i) + 1U);
                    length = ((close_index - i) - 1U);
                    column = ((column + i) + 1U);
                    offset = ((offset + i) + 1U);
                    flags = (LM_P0_FIELD_PARSE_REQUIRE_BOUNDED_SOURCE_LEVEL | LM_P0_FIELD_PARSE_ALLOW_EMPTY_FIELDS);
                    short_source_level = (current_source_level + 1U);
                    layout_base_level = (current_source_level + 1U);
                    current_source_level = (current_source_level + 1U);
                    i = 0U;
                    allow_empty_fields = 1;
                    expect_field = 1;
                    headless_group_after_separator = 0;
                    indent_stack = child_indent_stack;
                    indent_stack_owned = 1;
                    restart_parse_context = 1;
                    break;
                }
                else {
                    int quoted_head;
                    quoted_head = 0;
                    if (lm_p0_starts_c_surface_atom(lm_lmx_message_thread, text, length, i)) {
                        i = lm_p0_scan_c_surface_atom(lm_lmx_message_thread, text, length, i);
                        head_end = i;
                    }
                    else {
                        if (lm_p0_starts_c_prefixed_quote(lm_lmx_message_thread, text, length, i)) {
                            if (lm_p0_scan_c_prefixed_quote(lm_lmx_message_thread, document, text, length, &i, line, column) == 0) {
                                return lm_p0_field_parse_fail(lm_lmx_message_thread, &parse_stack, &indent_stack, &indent_stack_owned);
                            }
                            head_end = i;
                            quoted_head = 1;
                        }
                        else {
                            if ((lm_p0_starts_python_string(lm_lmx_message_thread, text, length, i) || (text[i] == '"')) || (text[i] == '`')) {
                                if (lm_p0_scan_quoted(lm_lmx_message_thread, document, text, length, &i, text[i], line, column) == 0) {
                                    return lm_p0_field_parse_fail(lm_lmx_message_thread, &parse_stack, &indent_stack, &indent_stack_owned);
                                }
                                head_end = i;
                                quoted_head = 1;
                            }
                            else {
                                if (text[i] == '\'') {
                                    if (lm_p0_scan_c_char(lm_lmx_message_thread, document, text, length, &i, line, column) == 0) {
                                        return lm_p0_field_parse_fail(lm_lmx_message_thread, &parse_stack, &indent_stack, &indent_stack_owned);
                                    }
                                    head_end = i;
                                    quoted_head = 1;
                                }
                                else {
                                    while ((((((((i < length) && (lm_p0_is_field_space(lm_lmx_message_thread, text[i]) == 0)) && (lm_p0_is_field_separator(lm_lmx_message_thread, text[i]) == 0)) && (text[i] != '(')) && (text[i] != ')')) && (text[i] != '#')) && (text[i] != '{')) && (text[i] != ':')) {
                                        if (text[i] == '[') {
                                            size_t bracket_close_index;
                                            if (lm_p0_find_matching_bracket(lm_lmx_message_thread, document, text, length, i, line, column, &bracket_close_index) == 0) {
                                                return lm_p0_field_parse_fail(lm_lmx_message_thread, &parse_stack, &indent_stack, &indent_stack_owned);
                                            }
                                            i = (bracket_close_index + 1U);
                                            continue;
                                        }
                                        if (text[i] == ']') {
                                            break;
                                        }
                                        if ((((lm_p0_starts_c_prefixed_quote(lm_lmx_message_thread, text, length, i) || lm_p0_starts_python_string(lm_lmx_message_thread, text, length, i)) || (text[i] == '"')) || (text[i] == '`')) || (text[i] == '\'')) {
                                            size_t diagnostic_line;
                                            size_t diagnostic_column;
                                            lm_p0_position_in_slice(lm_lmx_message_thread, text, length, i, line, column, &diagnostic_line, &diagnostic_column);
                                            lm_p0_set_diagnostic(document, 18, diagnostic_line, diagnostic_column, "missing separator before quoted token");
                                            return lm_p0_field_parse_fail(lm_lmx_message_thread, &parse_stack, &indent_stack, &indent_stack_owned);
                                        }
                                        i = i + 1U;
                                    }
                                    head_end = i;
                                }
                            }
                        }
                    }
                    if (((i < length) && (text[i] == ':')) && (head_end > start)) {
                        size_t body_index;
                        unsigned body_flags;
                        int has_inline_body;
                        node = lm_p0_new_node(lm_lmx_message_thread, document, LM_P0_NODE_FRAME);
                        if (node == 0) {
                            return lm_p0_field_parse_fail(lm_lmx_message_thread, &parse_stack, &indent_stack, &indent_stack_owned);
                        }
                        node->as->frame->head->data = (text + start);
                        node->as->frame->head->length = (head_end - start);
                        node->as->frame->flags = LM_P0_FRAME_COLON;
                        i = i + 1U;
                        while ((i < length) && lm_p0_is_horizontal_space(lm_lmx_message_thread, text[i])) {
                            i = i + 1U;
                        }
                        body_index = i;
                        has_inline_body = (((((i < length) && (lm_p0_is_line_break(lm_lmx_message_thread, text[i]) == 0)) && (lm_p0_is_field_separator(lm_lmx_message_thread, text[i]) == 0)) && (text[i] != ')')) && (text[i] != '#'));
                        if (has_inline_body) {
                            node->as->frame->flags = node -> as -> frame -> flags | LM_P0_FRAME_INLINE_BODY;
                        }
                        body_flags = ((LM_P0_FIELD_PARSE_STOP_ON_SEMICOLON | LM_P0_FIELD_PARSE_STOP_ON_SOURCE_LEVEL) | LM_P0_FIELD_PARSE_ALLOW_EMPTY_FIELDS);
                        if (lm_p0_field_parse_loop_push(lm_lmx_message_thread, document, parse_stack, indent_stack, indent_stack_owned, structure, text, length, line, column, offset, flags, short_source_level, layout_base_level, i, current_source_level, allow_empty_fields, expect_field, headless_group_after_separator, LM_P0_FIELD_PARSE_LOOP_COLON_FRAME_BODY, node, start, 0U) == 0) {
                            return lm_p0_field_parse_fail(lm_lmx_message_thread, &parse_stack, &indent_stack, &indent_stack_owned);
                        }
                        structure = node -> as -> frame -> body;
                        flags = body_flags;
                        short_source_level = current_source_level;
                        current_source_level = (current_source_level + 1U);
                        i = body_index;
                        allow_empty_fields = 1;
                        expect_field = 1;
                        headless_group_after_separator = 0;
                        indent_stack_owned = 0;
                        restart_parse_context = 1;
                        break;
                    }
                    else {
                        if (((i < length) && (text[i] == '(')) && (head_end > start)) {
                            node = lm_p0_new_node(lm_lmx_message_thread, document, LM_P0_NODE_FRAME);
                            if (node == 0) {
                                return lm_p0_field_parse_fail(lm_lmx_message_thread, &parse_stack, &indent_stack, &indent_stack_owned);
                            }
                            node->as->frame->head->data = (text + start);
                            node->as->frame->head->length = (head_end - start);
                            node->as->frame->flags = LM_P0_FRAME_COMPACT;
                            if (lm_p0_find_matching_paren(lm_lmx_message_thread, document, text, length, i, line, column, &close_index) == 0) {
                                return lm_p0_field_parse_fail(lm_lmx_message_thread, &parse_stack, &indent_stack, &indent_stack_owned);
                            }
                            child_indent_stack = lm_p0_indent_stack_new_empty(lm_lmx_message_thread);
                            if (child_indent_stack == 0) {
                                lm_p0_set_diagnostic(document, 1, line, column, "out of memory while creating indentation stack");
                                return lm_p0_field_parse_fail(lm_lmx_message_thread, &parse_stack, &indent_stack, &indent_stack_owned);
                            }
                            if (lm_p0_field_parse_loop_push(lm_lmx_message_thread, document, parse_stack, indent_stack, indent_stack_owned, structure, text, length, line, column, offset, flags, short_source_level, layout_base_level, i, current_source_level, allow_empty_fields, expect_field, headless_group_after_separator, LM_P0_FIELD_PARSE_LOOP_COMPACT_FRAME_BODY, node, start, close_index) == 0) {
                                lm_p0_indent_stack_delete(lm_lmx_message_thread, child_indent_stack);
                                return lm_p0_field_parse_fail(lm_lmx_message_thread, &parse_stack, &indent_stack, &indent_stack_owned);
                            }
                            structure = node -> as -> frame -> body;
                            text = ((text + i) + 1U);
                            length = ((close_index - i) - 1U);
                            column = ((column + i) + 1U);
                            offset = ((offset + i) + 1U);
                            flags = (LM_P0_FIELD_PARSE_REQUIRE_BOUNDED_SOURCE_LEVEL | LM_P0_FIELD_PARSE_ALLOW_EMPTY_FIELDS);
                            short_source_level = (current_source_level + 1U);
                            layout_base_level = (current_source_level + 1U);
                            current_source_level = (current_source_level + 1U);
                            i = 0U;
                            allow_empty_fields = 1;
                            expect_field = 1;
                            headless_group_after_separator = 0;
                            indent_stack = child_indent_stack;
                            indent_stack_owned = 1;
                            restart_parse_context = 1;
                            break;
                        }
                        else {
                            if (head_end == start) {
                                size_t diagnostic_line;
                                size_t diagnostic_column;
                                lm_p0_position_in_slice(lm_lmx_message_thread, text, length, start, line, column, &diagnostic_line, &diagnostic_column);
                                lm_p0_set_diagnostic(document, 6, diagnostic_line, diagnostic_column, "unexpected character in field list");
                                return lm_p0_field_parse_fail(lm_lmx_message_thread, &parse_stack, &indent_stack, &indent_stack_owned);
                            }
                            if (quoted_head && (lm_p0_require_quoted_token_boundary(lm_lmx_message_thread, document, text, length, head_end, line, column) == 0)) {
                                return lm_p0_field_parse_fail(lm_lmx_message_thread, &parse_stack, &indent_stack, &indent_stack_owned);
                            }
                            if (quoted_head == 0) {
                                i = head_end;
                                if (lm_p0_append_compact_atom_pieces(lm_lmx_message_thread, document, structure, text, length, line, column, offset, start, head_end) == 0) {
                                    return lm_p0_field_parse_fail(lm_lmx_message_thread, &parse_stack, &indent_stack, &indent_stack_owned);
                                }
                                expect_field = 0;
                                continue;
                            }
                            node = lm_p0_new_node(lm_lmx_message_thread, document, LM_P0_NODE_ATOM);
                            if (node == 0) {
                                return lm_p0_field_parse_fail(lm_lmx_message_thread, &parse_stack, &indent_stack, &indent_stack_owned);
                            }
                            node->as->atom->data = (text + start);
                            node->as->atom->length = (head_end - start);
                            node->span->line = line;
                            lm_p0_position_in_slice(lm_lmx_message_thread, text, length, start, line, column, &node -> span -> line, &node -> span -> column);
                            node->span->offset = (offset + start);
                            node->span->length = (head_end - start);
                            if (lm_p0_document_register_lazy_text(lm_lmx_message_thread, document, node -> as -> atom -> data, node -> as -> atom -> length, &node -> as -> atom -> data, node -> span -> line, node -> span -> column) == 0) {
                                return lm_p0_field_parse_fail(lm_lmx_message_thread, &parse_stack, &indent_stack, &indent_stack_owned);
                            }
                            i = head_end;
                        }
                    }
                }
            }
            if (lm_p0_parse_append_node_and_update(lm_lmx_message_thread, document, structure, node, flags, allow_empty_fields, &expect_field, &headless_group_after_separator) == 0) {
                return lm_p0_field_parse_fail(lm_lmx_message_thread, &parse_stack, &indent_stack, &indent_stack_owned);
            }
        }
        if (restart_parse_context) {
            continue;
        }
        child_index = i;
        if (indent_stack_owned) {
            lm_p0_indent_stack_delete(lm_lmx_message_thread, indent_stack);
            indent_stack = 0;
            indent_stack_owned = 0;
        }
        frame = (((LmP0FieldParseLoopFrame *)lm_own_ptr_stack_pop(parse_stack)));
        if (frame == 0) {
            index[0] = child_index;
            lm_p0_field_parse_loop_stack_delete(lm_lmx_message_thread, parse_stack);
            return 1;
        }
        indent_stack = frame -> indent_stack;
        structure = frame -> structure;
        text = frame -> text;
        length = frame -> length;
        line = frame -> line;
        column = frame -> column;
        offset = frame -> offset;
        flags = frame -> flags;
        short_source_level = frame -> short_source_level;
        layout_base_level = frame -> layout_base_level;
        i = frame -> i;
        current_source_level = frame -> current_source_level;
        allow_empty_fields = frame -> allow_empty_fields;
        expect_field = frame -> expect_field;
        headless_group_after_separator = frame -> headless_group_after_separator;
        indent_stack_owned = frame -> indent_stack_owned;
        node = frame -> node;
        start = frame -> start;
        close_index = frame -> close_index;
        if (frame -> continuation == LM_P0_FIELD_PARSE_LOOP_HEADLESS_GROUP) {
            if (child_index <= start) {
                lm_p0_free_node(lm_lmx_message_thread, node);
                headless_group_after_separator = 0;
            }
            else {
                node->span->line = line;
                lm_p0_position_in_slice(lm_lmx_message_thread, text, length, start, line, column, &node -> span -> line, &node -> span -> column);
                node->span->offset = (offset + start);
                node->span->length = (child_index - start);
                if (lm_p0_append_field(lm_lmx_message_thread, document, structure, node) == 0) {
                    lm_p0_free_node(lm_lmx_message_thread, node);
                    lm_own_delete(frame, 0);
                    return lm_p0_field_parse_fail(lm_lmx_message_thread, &parse_stack, &indent_stack, &indent_stack_owned);
                }
                expect_field = 0;
                headless_group_after_separator = ((((flags & LM_P0_FIELD_PARSE_ALLOW_HEADLESS_AFTER_SEPARATOR) != 0U) && (child_index > 0U)) && lm_p0_is_short_form_separator(lm_lmx_message_thread, text[(child_index - 1U)]));
                i = child_index;
            }
            lm_own_delete(frame, 0);
            restart_parse_context = 1;
            continue;
        }
        if (frame -> continuation == LM_P0_FIELD_PARSE_LOOP_COLON_FRAME_BODY) {
            if ((child_index > 0U) && lm_p0_is_short_form_separator(lm_lmx_message_thread, text[(child_index - 1U)])) {
                node->as->frame->flags = node -> as -> frame -> flags | LM_P0_FRAME_SEPARATOR_CLOSED;
            }
            node->span->line = line;
            lm_p0_position_in_slice(lm_lmx_message_thread, text, length, start, line, column, &node -> span -> line, &node -> span -> column);
            node->span->offset = (offset + start);
            node->span->length = (child_index - start);
            if (lm_p0_document_register_lazy_text(lm_lmx_message_thread, document, node -> as -> frame -> head -> data, node -> as -> frame -> head -> length, &node -> as -> frame -> head -> data, node -> span -> line, node -> span -> column) == 0) {
                lm_own_delete(frame, 0);
                return lm_p0_field_parse_fail(lm_lmx_message_thread, &parse_stack, &indent_stack, &indent_stack_owned);
            }
            i = child_index;
            if (lm_p0_parse_append_node_and_update(lm_lmx_message_thread, document, structure, node, flags, allow_empty_fields, &expect_field, &headless_group_after_separator) == 0) {
                lm_own_delete(frame, 0);
                return lm_p0_field_parse_fail(lm_lmx_message_thread, &parse_stack, &indent_stack, &indent_stack_owned);
            }
            lm_own_delete(frame, 0);
            restart_parse_context = 1;
            continue;
        }
        if (frame -> continuation == LM_P0_FIELD_PARSE_LOOP_BOUNDED_STRUCTURE) {
            node->span->line = line;
            lm_p0_position_in_slice(lm_lmx_message_thread, text, length, start, line, column, &node -> span -> line, &node -> span -> column);
            node->span->offset = (offset + start);
            node->span->length = ((close_index - start) + 1U);
            i = (close_index + 1U);
            if (lm_p0_parse_append_node_and_update(lm_lmx_message_thread, document, structure, node, flags, allow_empty_fields, &expect_field, &headless_group_after_separator) == 0) {
                lm_own_delete(frame, 0);
                return lm_p0_field_parse_fail(lm_lmx_message_thread, &parse_stack, &indent_stack, &indent_stack_owned);
            }
            lm_own_delete(frame, 0);
            restart_parse_context = 1;
            continue;
        }
        if (frame -> continuation == LM_P0_FIELD_PARSE_LOOP_COMPACT_FRAME_BODY) {
            node->span->line = line;
            lm_p0_position_in_slice(lm_lmx_message_thread, text, length, start, line, column, &node -> span -> line, &node -> span -> column);
            node->span->offset = (offset + start);
            node->span->length = ((close_index - start) + 1U);
            if (lm_p0_document_register_lazy_text(lm_lmx_message_thread, document, node -> as -> frame -> head -> data, node -> as -> frame -> head -> length, &node -> as -> frame -> head -> data, node -> span -> line, node -> span -> column) == 0) {
                lm_own_delete(frame, 0);
                return lm_p0_field_parse_fail(lm_lmx_message_thread, &parse_stack, &indent_stack, &indent_stack_owned);
            }
            i = (close_index + 1U);
            if (lm_p0_parse_append_node_and_update(lm_lmx_message_thread, document, structure, node, flags, allow_empty_fields, &expect_field, &headless_group_after_separator) == 0) {
                lm_own_delete(frame, 0);
                return lm_p0_field_parse_fail(lm_lmx_message_thread, &parse_stack, &indent_stack, &indent_stack_owned);
            }
            lm_own_delete(frame, 0);
            restart_parse_context = 1;
            continue;
        }
        lm_own_delete(frame, 0);
        lm_p0_set_diagnostic(document, 1, line, column, "internal parser field stack continuation error");
        return lm_p0_field_parse_fail(lm_lmx_message_thread, &parse_stack, &indent_stack, &indent_stack_owned);
    }
    return 0;
}

static int lm_p0_parse_fields_until(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmP0Structure *structure, const char *text, size_t length, size_t line, size_t column, size_t offset, unsigned flags, size_t short_source_level, size_t initial_source_level, size_t *index) {
    (void)lm_lmx_message_thread;
    LmP0IndentStack * indent_stack;
    int status;
    indent_stack = lm_p0_indent_stack_new_empty(lm_lmx_message_thread);
    if (indent_stack == 0) {
        lm_p0_set_diagnostic(document, 1, line, column, "out of memory while creating indentation stack");
        return 0;
    }
    status = lm_p0_parse_fields_until_with_layout(lm_lmx_message_thread, document, indent_stack, structure, text, length, line, column, offset, flags, short_source_level, initial_source_level, initial_source_level, index);
    lm_p0_indent_stack_delete(lm_lmx_message_thread, indent_stack);
    return status;
}

static int lm_p0_parse_fields_into(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmP0Structure *structure, const char *text, size_t length, size_t line, size_t column, size_t offset) {
    (void)lm_lmx_message_thread;
    size_t index;
    index = 0U;
    return lm_p0_parse_fields_until(lm_lmx_message_thread, document, structure, text, length, line, column, offset, LM_P0_FIELD_PARSE_ALLOW_HEADLESS_AFTER_SEPARATOR, 0U, 0U, &index);
}

static int lm_p0_stack_ensure(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmP0Stack *stack, size_t level) {
    (void)lm_lmx_message_thread;
    LmP0Structure * *parents;
    LmP0Node * *owners;
    unsigned char *hard;
    size_t old_capacity;
    size_t new_capacity;
    size_t i;
    if (level < stack -> capacity) {
        return 1;
    }
    old_capacity = stack -> capacity;
    if (old_capacity == 0U) {
        new_capacity = 8U;
    }
    else {
        new_capacity = old_capacity;
    }
    while (new_capacity <= level) {
        new_capacity = new_capacity * 2U;
    }
    parents = (((LmP0Structure * *)lm_own_resize(stack -> parents, (new_capacity * sizeof(parents[0])))));
    if (parents == 0) {
        lm_p0_set_diagnostic(document, 1, 0U, 0U, "out of memory while growing parser stack");
        return 0;
    }
    owners = (((LmP0Node * *)lm_own_resize(stack -> owners, (new_capacity * sizeof(owners[0])))));
    if (owners == 0) {
        lm_p0_set_diagnostic(document, 1, 0U, 0U, "out of memory while growing parser stack");
        stack->parents = parents;
        return 0;
    }
    hard = (((unsigned char *)lm_own_resize(stack -> hard, (new_capacity * sizeof(hard[0])))));
    if (hard == 0) {
        lm_p0_set_diagnostic(document, 1, 0U, 0U, "out of memory while growing parser stack");
        stack->parents = parents;
        stack->owners = owners;
        return 0;
    }
    stack->parents = parents;
    stack->owners = owners;
    stack->hard = hard;
    stack->capacity = new_capacity;
    i = old_capacity;
    while (i < new_capacity) {
        stack->parents[i] = 0;
        stack->owners[i] = 0;
        stack->hard[i] = 0U;
        i = i + 1U;
    }
    return 1;
}

static void lm_p0_stack_truncate_deeper(struct LmMessageThread *lm_lmx_message_thread, LmP0Stack *stack, size_t level) {
    (void)lm_lmx_message_thread;
    size_t i;
    if (stack -> capacity <= (level + 1U)) {
        return;
    }
    i = (level + 1U);
    while (i < stack -> capacity) {
        stack->parents[i] = 0;
        stack->owners[i] = 0;
        stack->hard[i] = 0U;
        i = i + 1U;
    }
}

static size_t lm_p0_stack_top_level(struct LmMessageThread *lm_lmx_message_thread, const LmP0Stack *stack) {
    (void)lm_lmx_message_thread;
    size_t i;
    i = stack -> capacity;
    while (i > 0U) {
        i = i - 1U;
        if (stack -> parents[i] != 0) {
            return i;
        }
    }
    return 0U;
}

static int lm_p0_stack_level_is_trailer_body(struct LmMessageThread *lm_lmx_message_thread, const LmP0Stack *stack, size_t level) {
    (void)lm_lmx_message_thread;
    LmP0Node * owner;
    if ((level >= stack -> capacity) || (stack -> parents[level] == 0)) {
        return 0;
    }
    owner = stack -> owners[level];
    if (owner == 0) {
        return 0;
    }
    if (((owner -> kind == LM_P0_NODE_FRAME) && (owner -> as -> frame -> trailer != 0)) && (stack -> parents[level] == owner -> as -> frame -> trailer -> body)) {
        return 1;
    }
    if (((owner -> kind == LM_P0_NODE_STRUCTURE) && (owner -> as -> structure -> trailer != 0)) && (stack -> parents[level] == owner -> as -> structure -> trailer -> body)) {
        return 1;
    }
    return 0;
}

static size_t lm_p0_stack_collapse_soft_to_event(struct LmMessageThread *lm_lmx_message_thread, LmP0Stack *stack, size_t event_level) {
    (void)lm_lmx_message_thread;
    size_t top_level;
    top_level = lm_p0_stack_top_level(lm_lmx_message_thread, stack);
    while (((top_level > 0U) && (event_level < top_level)) && (stack -> hard[top_level] == 0U)) {
        lm_p0_stack_truncate_deeper(lm_lmx_message_thread, stack, (top_level - 1U));
        top_level = lm_p0_stack_top_level(lm_lmx_message_thread, stack);
    }
    return top_level;
}

static LmP0TrailerRole lm_p0_registry_trailer_role(struct LmMessageThread *lm_lmx_message_thread, const char *text, size_t length) {
    (void)lm_lmx_message_thread;
    size_t i;
    const LmTableCell * key_cell;
    const LmTableCell * payload_cell;
    const char *role_payload;
    LmP0TrailerRole result;
    if (lm_p0_registry_table_has_rows(lm_lmx_message_thread, "trailer.role") == 0) {
        return LM_P0_TRAILER_ROLE_NONE;
    }
    result = LM_P0_TRAILER_ROLE_NONE;
    i = 0U;
    payload_cell = lm_p0_registry_source_path_cell_at(lm_lmx_message_thread, "namespace", i, &key_cell);
    while (payload_cell != 0) {
        if (key_cell != 0 && key_cell -> value != 0 && payload_cell -> value != 0) {
            role_payload = lm_p0_registry_lookup_cstr(lm_lmx_message_thread, payload_cell -> value, "trailer.role");
            if (role_payload != 0 && lm_p0_text_has_prefix_name(lm_lmx_message_thread, text, length, key_cell -> value, lm_p0_registry_trailer_allows_bare(lm_lmx_message_thread, payload_cell -> value))) {
                result = lm_p0_trailer_role_from_payload(lm_lmx_message_thread, role_payload);
            }
        }
        i = i + 1U;
        payload_cell = lm_p0_registry_source_path_cell_at(lm_lmx_message_thread, "namespace", i, &key_cell);
    }
    return result;
}

static LmP0TrailerRole lm_p0_trailer_role(const char *text, size_t length) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    LmP0TrailerRole legacy_role;
    LmP0TrailerRole registry_role;
    if (lm_p0_dash_fence_status_after_comment_trim(text, length, 0) == LM_P0_DASH_FENCE_VALID) {
        return LM_P0_TRAILER_ROLE_DASH_CUTTER;
    }
    legacy_role = lm_p0_legacy_trailer_role(lm_lmx_message_thread, text, length);
    if (lm_p0_registry_table_has_rows(lm_lmx_message_thread, "trailer.role") == 0) {
        return legacy_role;
    }
    registry_role = lm_p0_registry_trailer_role(lm_lmx_message_thread, text, length);
    if (lm_p0_registry_compare_enabled(lm_lmx_message_thread) && (registry_role != legacy_role)) {
        char *key;
        key = lm_p0_copy_bytes(lm_lmx_message_thread, text, length);
        lm_p0_registry_compare_fail(lm_lmx_message_thread, "trailer.role", key, lm_p0_trailer_role_payload(lm_lmx_message_thread, registry_role), lm_p0_trailer_role_payload(lm_lmx_message_thread, legacy_role));
    }
    return registry_role;
}

static void lm_p0_stack_free(struct LmMessageThread *lm_lmx_message_thread, LmP0Stack *stack) {
    (void)lm_lmx_message_thread;
    lm_own_delete(stack -> parents, 0);
    lm_own_delete(stack -> owners, 0);
    lm_own_delete(stack -> hard, 0);
    stack->parents = 0;
    stack->owners = 0;
    stack->hard = 0;
    stack->capacity = 0U;
}

static void lm_p0_stack_free_any(void *object) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    lm_p0_stack_free(lm_lmx_message_thread, ((LmP0Stack *)object));
}

static LmP0Stack * lm_p0_stack_new(struct LmMessageThread *lm_lmx_message_thread) {
    (void)lm_lmx_message_thread;
    return (((LmP0Stack *)lm_own_new_zero(1U * sizeof(LmP0Stack))));
}

static void lm_p0_stack_delete(struct LmMessageThread *lm_lmx_message_thread, LmP0Stack *stack) {
    (void)lm_lmx_message_thread;
    lm_own_delete(stack, lm_p0_stack_free_any);
}

static LmP0PendingDelimiter * lm_p0_pending_delimiter_new(struct LmMessageThread *lm_lmx_message_thread) {
    (void)lm_lmx_message_thread;
    return (((LmP0PendingDelimiter *)lm_own_new_zero(1U * sizeof(LmP0PendingDelimiter))));
}

static LmP0StreamEvent * lm_p0_stream_event_new(struct LmMessageThread *lm_lmx_message_thread) {
    (void)lm_lmx_message_thread;
    return (((LmP0StreamEvent *)lm_own_new_zero(1U * sizeof(LmP0StreamEvent))));
}

static LmP0StreamEvent * lm_p0_stream_event_new_copy(struct LmMessageThread *lm_lmx_message_thread, const LmP0StreamEvent *event) {
    (void)lm_lmx_message_thread;
    LmP0StreamEvent * copy;
    if (event == 0) {
        return 0;
    }
    copy = (((LmP0StreamEvent *)lm_own_new_zero(1U * sizeof(copy[0]))));
    if (copy != 0) {
        copy[0] = event[0];
    }
    return copy;
}

static void lm_p0_stream_event_delete(struct LmMessageThread *lm_lmx_message_thread, LmP0StreamEvent *event) {
    (void)lm_lmx_message_thread;
    lm_own_delete(event, 0);
}

static void lm_p0_pending_delimiter_clear(struct LmMessageThread *lm_lmx_message_thread, LmP0PendingDelimiter *pending) {
    (void)lm_lmx_message_thread;
    if (pending != 0) {
        lm_p0_stream_event_delete(lm_lmx_message_thread, pending -> event);
        pending->event = 0;
        pending->active = 0;
    }
}

static int lm_p0_pending_delimiter_set(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmP0PendingDelimiter *pending, const LmP0StreamEvent *event) {
    (void)lm_lmx_message_thread;
    if (pending == 0) {
        return 0;
    }
    lm_p0_pending_delimiter_clear(lm_lmx_message_thread, pending);
    pending->event = lm_p0_stream_event_new_copy(lm_lmx_message_thread, event);
    if (pending -> event == 0) {
        size_t lm_p0_tmp_19;
        if (event != 0) {
            lm_p0_tmp_19 = event -> line;
        }
        else {
            lm_p0_tmp_19 = 0U;
        }
        size_t lm_p0_tmp_20;
        if (event != 0) {
            lm_p0_tmp_20 = event -> column;
        }
        else {
            lm_p0_tmp_20 = 0U;
        }
        lm_p0_set_diagnostic(document, 1, lm_p0_tmp_19, lm_p0_tmp_20, "out of memory while storing pending delimiter");
        return 0;
    }
    pending->active = 1;
    return 1;
}

static void lm_p0_pending_delimiter_delete(struct LmMessageThread *lm_lmx_message_thread, LmP0PendingDelimiter *pending) {
    (void)lm_lmx_message_thread;
    if (pending != 0) {
        lm_p0_pending_delimiter_clear(lm_lmx_message_thread, pending);
        lm_own_delete(pending, 0);
    }
}

static LmP0PendingMix * lm_p0_pending_mix_new(struct LmMessageThread *lm_lmx_message_thread) {
    (void)lm_lmx_message_thread;
    return (((LmP0PendingMix *)lm_own_new_zero(1U * sizeof(LmP0PendingMix))));
}

static void lm_p0_pending_mix_free(struct LmMessageThread *lm_lmx_message_thread, LmP0PendingMix *pending) {
    (void)lm_lmx_message_thread;
    if (pending != 0) {
        lm_own_delete(pending -> events, 0);
        pending->events = 0;
        pending->count = 0U;
        pending->capacity = 0U;
    }
}

static void lm_p0_pending_mix_free_any(void *object) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    lm_p0_pending_mix_free(lm_lmx_message_thread, ((LmP0PendingMix *)object));
}

static void lm_p0_pending_mix_delete(struct LmMessageThread *lm_lmx_message_thread, LmP0PendingMix *pending) {
    (void)lm_lmx_message_thread;
    lm_own_delete(pending, lm_p0_pending_mix_free_any);
}

static int lm_p0_pending_mix_push(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmP0PendingMix *pending, const LmP0StreamEvent *event) {
    (void)lm_lmx_message_thread;
    size_t new_capacity;
    LmP0StreamEvent * events;
    if ((pending == 0) || (event == 0)) {
        return 0;
    }
    if (pending -> count == pending -> capacity) {
        if (pending -> capacity == 0U) {
            new_capacity = 4U;
        }
        else {
            new_capacity = (pending -> capacity * 2U);
        }
        events = (((LmP0StreamEvent *)lm_own_resize(pending -> events, (new_capacity * sizeof(events[0])))));
        if (events == 0) {
            lm_p0_set_diagnostic(document, 1, event -> line, event -> column, "out of memory while storing pending MIX marks");
            return 0;
        }
        pending->events = events;
        pending->capacity = new_capacity;
    }
    pending->events[pending -> count] = event[0];
    pending->count = pending -> count + 1U;
    return 1;
}

static LmP0DisabledState * lm_p0_disabled_state_new(struct LmMessageThread *lm_lmx_message_thread, size_t base_level) {
    (void)lm_lmx_message_thread;
    LmP0DisabledState * state;
    state = (((LmP0DisabledState *)lm_own_new_zero(1U * sizeof(state[0]))));
    if (state != 0) {
        state->base_level = base_level;
        state->top_level = base_level;
        state->pending_item = 1;
        state->pending_level = base_level;
    }
    return state;
}

static void lm_p0_disabled_state_delete(struct LmMessageThread *lm_lmx_message_thread, LmP0DisabledState *state) {
    (void)lm_lmx_message_thread;
    lm_own_delete(state, 0);
}

static int lm_p0_node_keeps_source_child_level(struct LmMessageThread *lm_lmx_message_thread, LmP0Node *node) {
    (void)lm_lmx_message_thread;
    if (node -> kind == LM_P0_NODE_FRAME) {
        return (((node -> as -> frame -> flags & LM_P0_FRAME_COLON) != 0U) && ((node -> as -> frame -> flags & LM_P0_FRAME_SEPARATOR_CLOSED) == 0U));
    }
    return 0;
}

static LmP0Structure * lm_p0_node_child_structure(struct LmMessageThread *lm_lmx_message_thread, LmP0Node *node) {
    (void)lm_lmx_message_thread;
    if (node -> kind == LM_P0_NODE_FRAME) {
        return node -> as -> frame -> body;
    }
    if (node -> kind == LM_P0_NODE_STRUCTURE) {
        return node -> as -> structure;
    }
    return 0;
}

static LmP0Node * lm_p0_structure_last_colon_frame(struct LmMessageThread *lm_lmx_message_thread, LmP0Structure *structure) {
    (void)lm_lmx_message_thread;
    LmP0Node * node;
    if ((structure -> last_field == 0) || (structure -> last_field -> value == 0)) {
        return 0;
    }
    node = structure -> last_field -> value;
    if (((node -> kind == LM_P0_NODE_FRAME) && ((node -> as -> frame -> flags & LM_P0_FRAME_COLON) != 0U)) && ((node -> as -> frame -> flags & LM_P0_FRAME_SEPARATOR_CLOSED) == 0U)) {
        return node;
    }
    return 0;
}

static int lm_p0_stack_install_node_lineage(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmP0Stack *stack, size_t base_level, LmP0Node *node) {
    (void)lm_lmx_message_thread;
    LmP0Node * owner;
    size_t level;
    if (lm_p0_node_keeps_source_child_level(lm_lmx_message_thread, node) == 0) {
        if (lm_p0_stack_ensure(lm_lmx_message_thread, document, stack, (base_level + 1U)) == 0) {
            return 0;
        }
        stack->parents[(base_level + 1U)] = 0;
        stack->owners[(base_level + 1U)] = 0;
        stack->hard[(base_level + 1U)] = 0U;
        lm_p0_stack_truncate_deeper(lm_lmx_message_thread, stack, (base_level + 1U));
        return 1;
    }
    owner = node;
    level = (base_level + 1U);
    while (lm_p0_node_keeps_source_child_level(lm_lmx_message_thread, owner)) {
        LmP0Structure * body;
        LmP0Node * next_owner;
        if (lm_p0_stack_ensure(lm_lmx_message_thread, document, stack, level) == 0) {
            return 0;
        }
        body = lm_p0_node_child_structure(lm_lmx_message_thread, owner);
        stack->parents[level] = body;
        stack->owners[level] = owner;
        if (body -> field_count == 0U) {
            stack->hard[level] = 1U;
        }
        else {
            stack->hard[level] = 0U;
        }
        next_owner = lm_p0_structure_last_colon_frame(lm_lmx_message_thread, body);
        if (next_owner == 0) {
            break;
        }
        owner = next_owner;
        level = level + 1U;
    }
    lm_p0_stack_truncate_deeper(lm_lmx_message_thread, stack, level);
    return 1;
}

static int lm_p0_stack_ensure_root_level_alias(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmP0Stack *stack, size_t level) {
    (void)lm_lmx_message_thread;
    if ((level != 1U) || (stack -> parents[1] != 0)) {
        return 1;
    }
    if (lm_p0_stack_ensure(lm_lmx_message_thread, document, stack, 1U) == 0) {
        return 0;
    }
    stack->parents[1] = stack -> parents[0];
    stack->owners[1] = stack -> owners[0];
    stack->hard[1] = 1U;
    return 1;
}

static int lm_p0_stack_open_implicit_anonymous(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmP0Stack *stack, size_t parent_level, size_t line, size_t column, size_t offset) {
    (void)lm_lmx_message_thread;
    LmP0Node * anonymous_node;
    LmP0Structure * parent;
    parent = stack -> parents[parent_level];
    if (parent == 0) {
        lm_p0_set_diagnostic(document, 8, line, column, "source level has no open parent structure");
        return 0;
    }
    anonymous_node = lm_p0_new_node(lm_lmx_message_thread, document, LM_P0_NODE_STRUCTURE);
    if (anonymous_node == 0) {
        return 0;
    }
    anonymous_node->span->line = line;
    anonymous_node->span->column = column;
    anonymous_node->span->offset = offset;
    anonymous_node->span->length = 0U;
    if (lm_p0_append_field(lm_lmx_message_thread, document, parent, anonymous_node) == 0) {
        lm_p0_free_node(lm_lmx_message_thread, anonymous_node);
        return 0;
    }
    if (lm_p0_stack_ensure(lm_lmx_message_thread, document, stack, (parent_level + 1U)) == 0) {
        return 0;
    }
    stack->parents[(parent_level + 1U)] = anonymous_node -> as -> structure;
    stack->owners[(parent_level + 1U)] = anonymous_node;
    stack->hard[(parent_level + 1U)] = 1U;
    lm_p0_stack_truncate_deeper(lm_lmx_message_thread, stack, (parent_level + 1U));
    return 1;
}

static LmP0Trailer * * lm_p0_node_trailer_slot(struct LmMessageThread *lm_lmx_message_thread, LmP0Node *node) {
    (void)lm_lmx_message_thread;
    if (node -> kind == LM_P0_NODE_FRAME) {
        return &node -> as -> frame -> trailer;
    }
    if (node -> kind == LM_P0_NODE_STRUCTURE) {
        return &node -> as -> structure -> trailer;
    }
    return 0;
}

static LmP0Trailer * lm_p0_attach_trailer(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmP0Node *node, const char *spelling, size_t spelling_length, unsigned flags, size_t line, size_t column) {
    (void)lm_lmx_message_thread;
    LmP0Trailer * *slot;
    LmP0Trailer * trailer;
    slot = lm_p0_node_trailer_slot(lm_lmx_message_thread, node);
    if (slot == 0) {
        lm_p0_set_diagnostic(document, 10, line, column, "this parser node cannot receive a trailer");
        return 0;
    }
    if (slot[0] != 0) {
        lm_p0_set_diagnostic(document, 11, line, column, "parser node already has a trailer");
        return 0;
    }
    trailer = lm_p0_new_trailer(lm_lmx_message_thread, document, spelling, spelling_length, line, column);
    if (trailer == 0) {
        return 0;
    }
    trailer->flags = flags;
    slot[0] = trailer;
    return trailer;
}

static int lm_p0_parse_trailer_item(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmP0Node *target, const char *text, size_t length, size_t line, size_t column, size_t offset, unsigned flags, LmP0Structure **out_body) {
    (void)lm_lmx_message_thread;
    size_t colon_index;
    size_t spelling_length;
    size_t body_start;
    LmP0Trailer * trailer;
    out_body[0] = 0;
    int colon_status;
    colon_status = lm_p0_find_colon(lm_lmx_message_thread, document, text, length, line, column, &colon_index);
    if (colon_status < 0) {
        return 0;
    }
    if (colon_status == 0) {
        colon_index = length;
    }
    if (colon_index < length) {
        spelling_length = colon_index;
        while ((spelling_length > 0U) && lm_p0_is_horizontal_space(lm_lmx_message_thread, text[(spelling_length - 1U)])) {
            spelling_length = spelling_length - 1U;
        }
        body_start = (colon_index + 1U);
        while ((body_start < length) && lm_p0_is_horizontal_space(lm_lmx_message_thread, text[body_start])) {
            body_start = body_start + 1U;
        }
    }
    else {
        body_start = 0U;
        if ((body_start < length) && lm_p0_starts_c_prefixed_quote(lm_lmx_message_thread, text, length, body_start)) {
            if (lm_p0_scan_c_prefixed_quote(lm_lmx_message_thread, document, text, length, &body_start, line, column) == 0) {
                return 0;
            }
            spelling_length = body_start;
        }
        else {
            if ((body_start < length) && (text[body_start] == '\'')) {
                if (lm_p0_scan_c_char(lm_lmx_message_thread, document, text, length, &body_start, line, column) == 0) {
                    return 0;
                }
                spelling_length = body_start;
            }
            else {
                if ((body_start < length) && ((lm_p0_starts_python_string(lm_lmx_message_thread, text, length, body_start) || (text[body_start] == '"')) || (text[body_start] == '`'))) {
                    if (lm_p0_scan_quoted(lm_lmx_message_thread, document, text, length, &body_start, text[body_start], line, column) == 0) {
                        return 0;
                    }
                    spelling_length = body_start;
                }
                else {
                    while ((((body_start < length) && (lm_p0_is_horizontal_space(lm_lmx_message_thread, text[body_start]) == 0)) && (lm_p0_is_field_separator(lm_lmx_message_thread, text[body_start]) == 0)) && (text[body_start] != '#')) {
                        body_start = body_start + 1U;
                    }
                    spelling_length = body_start;
                }
            }
        }
        while ((body_start < length) && lm_p0_is_horizontal_space(lm_lmx_message_thread, text[body_start])) {
            body_start = body_start + 1U;
        }
    }
    if (spelling_length == 0U) {
        lm_p0_set_diagnostic(document, 12, line, column, "trailer spelling is empty");
        return 0;
    }
    trailer = lm_p0_attach_trailer(lm_lmx_message_thread, document, target, text, spelling_length, flags, line, column);
    if (trailer == 0) {
        return 0;
    }
    if (lm_p0_parse_fields_into(lm_lmx_message_thread, document, trailer -> body, (text + body_start), (length - body_start), line, (column + body_start), (offset + body_start)) == 0) {
        return 0;
    }
    out_body[0] = trailer -> body;
    return 1;
}

static int lm_p0_stream_resolve_pending_delimiter(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmP0Stack *stack, LmP0PendingDelimiter *pending, size_t next_level) {
    (void)lm_lmx_message_thread;
    const LmP0StreamEvent * event;
    size_t top_level;
    LmP0Structure * parent;
    if ((pending -> active == 0) || (pending -> event == 0)) {
        return 1;
    }
    event = pending -> event;
    if (lm_p0_stack_ensure(lm_lmx_message_thread, document, stack, (event -> level + 1U)) == 0) {
        return 0;
    }
    top_level = lm_p0_stack_collapse_soft_to_event(lm_lmx_message_thread, stack, event -> level);
    if ((event -> level == top_level) && (stack -> hard[top_level] == 0U)) {
        stack->hard[top_level] = 1U;
    }
    if (event -> level < top_level) {
        lm_p0_stack_truncate_deeper(lm_lmx_message_thread, stack, event -> level);
        top_level = lm_p0_stack_top_level(lm_lmx_message_thread, stack);
    }
    parent = stack -> parents[event -> level];
    if ((parent == 0) && (event -> level == 1U)) {
        parent = stack -> parents[0];
    }
    if (parent == 0) {
        lm_p0_set_diagnostic(document, 8, event -> line, event -> column, "source level has no open parent structure");
        return 0;
    }
    if (next_level == (event -> level + 1U)) {
        LmP0Node * anonymous_node;
        anonymous_node = lm_p0_new_node(lm_lmx_message_thread, document, LM_P0_NODE_STRUCTURE);
        if (anonymous_node == 0) {
            return 0;
        }
        anonymous_node->span->line = event -> line;
        anonymous_node->span->column = event -> column;
        anonymous_node->span->offset = event -> offset;
        anonymous_node->span->length = event -> text_length;
        if (lm_p0_append_field(lm_lmx_message_thread, document, parent, anonymous_node) == 0) {
            lm_p0_free_node(lm_lmx_message_thread, anonymous_node);
            return 0;
        }
        stack->parents[(event -> level + 1U)] = anonymous_node -> as -> structure;
        stack->owners[(event -> level + 1U)] = anonymous_node;
        stack->hard[(event -> level + 1U)] = 1U;
        lm_p0_stack_truncate_deeper(lm_lmx_message_thread, stack, (event -> level + 1U));
    }
    else {
        lm_p0_stack_truncate_deeper(lm_lmx_message_thread, stack, event -> level);
    }
    lm_p0_pending_delimiter_clear(lm_lmx_message_thread, pending);
    return 1;
}

static int lm_p0_stream_apply_item_event(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmP0Stack *stack, const LmP0StreamEvent *event) {
    (void)lm_lmx_message_thread;
    LmP0TrailerRole trailer_role;
    size_t top_level;
    LmP0Structure * parent;
    LmP0Node * node;
    int trailer_target_available;
    int trailer_target_accepted;
    if (lm_p0_stack_ensure(lm_lmx_message_thread, document, stack, (event -> level + 1U)) == 0) {
        return 0;
    }
    if ((event -> kind == LM_P0_STREAM_EVENT_ITEM) && (lm_p0_validate_dash_fence_line(lm_lmx_message_thread, document, event -> text, event -> text_length, event -> line, event -> column) == 0)) {
        return 0;
    }
    if (event -> kind == LM_P0_STREAM_EVENT_ITEM) {
        trailer_role = lm_p0_trailer_role(event -> text, event -> text_length);
    }
    else {
        trailer_role = LM_P0_TRAILER_ROLE_NONE;
    }
    top_level = lm_p0_stack_top_level(lm_lmx_message_thread, stack);
    trailer_target_available = ((lm_p0_trailer_role_is_tail_cutter(lm_lmx_message_thread, trailer_role) && ((event -> level + 1U) <= top_level)) && (stack -> owners[(event -> level + 1U)] != 0));
    trailer_target_accepted = (trailer_target_available && lm_p0_trailer_role_accepts_target(lm_lmx_message_thread, trailer_role, stack -> owners[(event -> level + 1U)]));
    if ((lm_p0_trailer_role_is_tail_cutter(lm_lmx_message_thread, trailer_role) == 0) || (trailer_target_accepted == 0)) {
        top_level = lm_p0_stack_collapse_soft_to_event(lm_lmx_message_thread, stack, event -> level);
        if ((event -> level == top_level) && (stack -> hard[top_level] == 0U)) {
            stack->hard[top_level] = 1U;
        }
        trailer_target_available = ((lm_p0_trailer_role_is_tail_cutter(lm_lmx_message_thread, trailer_role) && ((event -> level + 1U) <= top_level)) && (stack -> owners[(event -> level + 1U)] != 0));
        trailer_target_accepted = (trailer_target_available && lm_p0_trailer_role_accepts_target(lm_lmx_message_thread, trailer_role, stack -> owners[(event -> level + 1U)]));
    }
    if ((lm_p0_trailer_role_is_tail_cutter(lm_lmx_message_thread, trailer_role) && ((event -> level + 1U) < top_level)) && (trailer_target_accepted == 0)) {
        lm_p0_set_diagnostic(document, 13, event -> line, event -> column, "tail-cutter target is not valid for this receiver");
        return 0;
    }
    if (lm_p0_trailer_role_is_tail_cutter(lm_lmx_message_thread, trailer_role) && trailer_target_accepted) {
        LmP0Structure * trailer_body;
        LmP0Node * target;
        size_t target_level;
        target_level = (event -> level + 1U);
        target = stack -> owners[target_level];
        lm_p0_stack_truncate_deeper(lm_lmx_message_thread, stack, target_level);
        if (lm_p0_parse_trailer_item(lm_lmx_message_thread, document, target, event -> text, event -> text_length, event -> line, event -> column, event -> offset, LM_P0_TRAILER_TAIL_CUTTER, &trailer_body) == 0) {
            return 0;
        }
        stack->parents[target_level] = trailer_body;
        stack->owners[target_level] = target;
        stack->hard[target_level] = 0U;
        if (target_level > 0U) {
            stack->hard[(target_level - 1U)] = 0U;
        }
        lm_p0_stack_truncate_deeper(lm_lmx_message_thread, stack, target_level);
        return 1;
    }
    if (event -> level < top_level) {
        lm_p0_stack_truncate_deeper(lm_lmx_message_thread, stack, event -> level);
        top_level = lm_p0_stack_top_level(lm_lmx_message_thread, stack);
    }
    if ((stack -> parents[event -> level] == 0) && (event -> level > top_level)) {
        if (event -> level != (top_level + 1U)) {
            lm_p0_set_diagnostic(document, 13, event -> line, event -> column, "source level jumps too deep");
            return 0;
        }
        if (lm_p0_stack_open_implicit_anonymous(lm_lmx_message_thread, document, stack, top_level, event -> line, event -> column, event -> offset) == 0) {
            return 0;
        }
    }
    if (lm_p0_stack_ensure_root_level_alias(lm_lmx_message_thread, document, stack, event -> level) == 0) {
        return 0;
    }
    parent = stack -> parents[event -> level];
    if (parent == 0) {
        lm_p0_set_diagnostic(document, 8, event -> line, event -> column, "source level has no open parent structure");
        return 0;
    }
    if (event -> kind == LM_P0_STREAM_EVENT_ITEM) {
        LmP0Field * previous_last;
        LmP0Field * field;
        previous_last = parent -> last_field;
        if (lm_p0_parse_fields_into(lm_lmx_message_thread, document, parent, event -> text, event -> text_length, event -> line, event -> column, event -> offset) == 0) {
            return 0;
        }
        if (previous_last != 0) {
            field = previous_last -> next;
        }
        else {
            field = parent -> first_field;
        }
        while (field != 0) {
            if (field -> value != 0) {
                field->value->flags = field -> value -> flags | event -> node_flags;
            }
            field = field -> next;
        }
        if ((parent -> last_field == previous_last) || (parent -> last_field == 0)) {
            return 1;
        }
        return lm_p0_stack_install_node_lineage(lm_lmx_message_thread, document, stack, event -> level, parent -> last_field -> value);
    }
    if (event -> kind == LM_P0_STREAM_EVENT_DISABLED_BLOCK) {
        node = lm_p0_new_node(lm_lmx_message_thread, document, LM_P0_NODE_DISABLED);
        if (node == 0) {
            return 0;
        }
        node->as->atom->data = event -> text;
        node->as->atom->length = event -> text_length;
        node->span->line = event -> line;
        node->span->column = event -> column;
        node->span->offset = event -> offset;
        node->span->length = event -> text_length;
        if (lm_p0_document_register_lazy_text(lm_lmx_message_thread, document, node -> as -> atom -> data, node -> as -> atom -> length, &node -> as -> atom -> data, node -> span -> line, node -> span -> column) == 0) {
            return 0;
        }
    }
    else {
        if (event -> kind != LM_P0_STREAM_EVENT_BLOCK_STRING) {
            lm_p0_set_diagnostic(document, 13, event -> line, event -> column, "unexpected stream item event kind");
            return 0;
        }
        node = lm_p0_new_node(lm_lmx_message_thread, document, LM_P0_NODE_ATOM);
        if (node == 0) {
            return 0;
        }
        node->as->atom->data = event -> text;
        node->as->atom->length = event -> text_length;
        node->span->line = event -> line;
        node->span->column = event -> column;
        node->span->offset = event -> offset;
        node->span->length = event -> text_length;
        if (lm_p0_document_register_lazy_text(lm_lmx_message_thread, document, node -> as -> atom -> data, node -> as -> atom -> length, &node -> as -> atom -> data, node -> span -> line, node -> span -> column) == 0) {
            return 0;
        }
    }
    node->flags = node -> flags | event -> node_flags;
    if (lm_p0_append_field(lm_lmx_message_thread, document, parent, node) == 0) {
        lm_p0_free_node(lm_lmx_message_thread, node);
        return 0;
    }
    if (lm_p0_stack_install_node_lineage(lm_lmx_message_thread, document, stack, event -> level, node) == 0) {
        return 0;
    }
    return 1;
}

static int lm_p0_stream_apply_mix_event(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmP0Stack *stack, const LmP0StreamEvent *event) {
    (void)lm_lmx_message_thread;
    size_t top_level;
    LmP0Structure * parent;
    LmP0Field * previous_last;
    LmP0Field * field;
    if (lm_p0_stack_ensure(lm_lmx_message_thread, document, stack, (event -> level + 1U)) == 0) {
        return 0;
    }
    top_level = lm_p0_stack_collapse_soft_to_event(lm_lmx_message_thread, stack, event -> level);
    if ((event -> level == top_level) && (stack -> hard[top_level] == 0U)) {
        stack->hard[top_level] = 1U;
    }
    if (event -> level < top_level) {
        lm_p0_stack_truncate_deeper(lm_lmx_message_thread, stack, event -> level);
        top_level = lm_p0_stack_top_level(lm_lmx_message_thread, stack);
    }
    if ((stack -> parents[event -> level] == 0) && (event -> level > top_level)) {
        if (event -> level != (top_level + 1U)) {
            lm_p0_set_diagnostic(document, 13, event -> line, event -> column, "source level jumps too deep");
            return 0;
        }
        if (lm_p0_stack_open_implicit_anonymous(lm_lmx_message_thread, document, stack, top_level, event -> line, event -> column, event -> offset) == 0) {
            return 0;
        }
    }
    if (lm_p0_stack_ensure_root_level_alias(lm_lmx_message_thread, document, stack, event -> level) == 0) {
        return 0;
    }
    parent = stack -> parents[event -> level];
    if (parent == 0) {
        lm_p0_set_diagnostic(document, 8, event -> line, event -> column, "source level has no open parent structure");
        return 0;
    }
    previous_last = parent -> last_field;
    if (lm_p0_parse_fields_into(lm_lmx_message_thread, document, parent, event -> text, event -> text_length, event -> line, event -> column, event -> offset) == 0) {
        return 0;
    }
    if (previous_last != 0) {
        field = previous_last -> next;
    }
    else {
        field = parent -> first_field;
    }
    while (field != 0) {
        if (field -> value != 0) {
            field->value->flags = field -> value -> flags | event -> node_flags;
        }
        field = field -> next;
    }
    return 1;
}

static int lm_p0_stream_apply_event(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmP0Stack *stack, LmP0PendingDelimiter *pending, const LmP0StreamEvent *event) {
    (void)lm_lmx_message_thread;
    if (event -> kind == LM_P0_STREAM_EVENT_DELIM) {
        if (lm_p0_stream_resolve_pending_delimiter(lm_lmx_message_thread, document, stack, pending, event -> level) == 0) {
            return 0;
        }
        return lm_p0_pending_delimiter_set(lm_lmx_message_thread, document, pending, event);
    }
    if (lm_p0_stream_resolve_pending_delimiter(lm_lmx_message_thread, document, stack, pending, event -> level) == 0) {
        return 0;
    }
    if (event -> kind == LM_P0_STREAM_EVENT_MIX) {
        return lm_p0_stream_apply_mix_event(lm_lmx_message_thread, document, stack, event);
    }
    if ((event -> kind == LM_P0_STREAM_EVENT_ITEM) && (lm_p0_dash_fence_status_after_comment_trim(event -> text, event -> text_length, 0) == LM_P0_DASH_FENCE_VALID)) {
        if (lm_p0_pending_delimiter_set(lm_lmx_message_thread, document, pending, event) == 0) {
            return 0;
        }
        pending->event->kind = LM_P0_STREAM_EVENT_DELIM;
        return 1;
    }
    return lm_p0_stream_apply_item_event(lm_lmx_message_thread, document, stack, event);
}

static int lm_p0_pending_mix_flush(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmP0Stack *stack, LmP0PendingDelimiter *pending_delimiter, LmP0PendingMix *pending_mix, size_t level) {
    (void)lm_lmx_message_thread;
    size_t i;
    if ((pending_mix == 0) || (pending_mix -> count == 0U)) {
        return 1;
    }
    i = 0U;
    while (i < pending_mix -> count) {
        LmP0StreamEvent * event;
        event = lm_p0_stream_event_new(lm_lmx_message_thread);
        if (event == 0) {
            lm_p0_set_diagnostic(document, 1, 0U, 0U, "out of memory while flushing pending MIX event");
            return 0;
        }
        event[0] = pending_mix -> events[i];
        event->level = level;
        if (lm_p0_stream_apply_event(lm_lmx_message_thread, document, stack, pending_delimiter, event) == 0) {
            lm_p0_stream_event_delete(lm_lmx_message_thread, event);
            return 0;
        }
        lm_p0_stream_event_delete(lm_lmx_message_thread, event);
        i = i + 1U;
    }
    pending_mix->count = 0U;
    return 1;
}

static size_t lm_p0_stream_block_string_level(struct LmMessageThread *lm_lmx_message_thread, const LmP0Stack *stack, const LmP0PendingDelimiter *pending) {
    (void)lm_lmx_message_thread;
    size_t level;
    if (pending -> active && (pending -> event != 0)) {
        return (pending -> event -> level + 1U);
    }
    level = lm_p0_stack_top_level(lm_lmx_message_thread, stack);
    while (level > 0U) {
        if ((stack -> parents[level] != 0) && (lm_p0_stack_level_is_trailer_body(lm_lmx_message_thread, stack, level) == 0)) {
            return level;
        }
        level = level - 1U;
    }
    return 0U;
}

static int lm_p0_validate_disabled_item_text(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, const char *text, size_t length, size_t line, size_t column) {
    (void)lm_lmx_message_thread;
    size_t i;
    i = 0U;
    if (lm_p0_validate_dash_fence_line(lm_lmx_message_thread, document, text, length, line, column) == 0) {
        return 0;
    }
    while (i < length) {
        if (text[i] == '{') {
            if (lm_p0_skip_brace_mark(lm_lmx_message_thread, document, text, length, &i, line, column) == 0) {
                return 0;
            }
            continue;
        }
        if (text[i] == '#') {
            while ((i < length) && (lm_p0_is_line_break(lm_lmx_message_thread, text[i]) == 0)) {
                i = i + 1U;
            }
            continue;
        }
        if (lm_p0_starts_python_string(lm_lmx_message_thread, text, length, i)) {
            if (lm_p0_scan_python_string(lm_lmx_message_thread, document, text, length, &i, line, column) == 0) {
                return 0;
            }
            continue;
        }
        if (lm_p0_starts_c_prefixed_quote(lm_lmx_message_thread, text, length, i)) {
            if (lm_p0_scan_c_prefixed_quote(lm_lmx_message_thread, document, text, length, &i, line, column) == 0) {
                return 0;
            }
            continue;
        }
        if ((text[i] == '\'') && (lm_p0_starts_python_string(lm_lmx_message_thread, text, length, i) == 0)) {
            if (lm_p0_scan_c_char(lm_lmx_message_thread, document, text, length, &i, line, column) == 0) {
                return 0;
            }
            continue;
        }
        if ((text[i] == '"') || (text[i] == '`')) {
            if (lm_p0_scan_quoted(lm_lmx_message_thread, document, text, length, &i, text[i], line, column) == 0) {
                return 0;
            }
            continue;
        }
        if (text[i] == '(') {
            size_t close_index;
            if (lm_p0_find_matching_paren(lm_lmx_message_thread, document, text, length, i, line, column, &close_index) == 0) {
                return 0;
            }
            i = (close_index + 1U);
            continue;
        }
        if (text[i] == ')') {
            size_t diagnostic_line;
            size_t diagnostic_column;
            lm_p0_position_in_slice(lm_lmx_message_thread, text, length, i, line, column, &diagnostic_line, &diagnostic_column);
            lm_p0_set_diagnostic(document, 6, diagnostic_line, diagnostic_column, "unmatched closing parenthesis");
            return 0;
        }
        i = i + 1U;
    }
    return 1;
}

static int lm_p0_disabled_scan_next_event(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmP0IndentStack *indent_stack, size_t *offset, size_t *line, LmP0StreamEvent *event, int *has_event) {
    (void)lm_lmx_message_thread;
    const char *source;
    size_t length;
    source = document -> source;
    length = document -> source_length;
    has_event[0] = 0;
    while (offset[0] <= length) {
        size_t line_start;
        size_t line_end;
        size_t raw_length;
        size_t p;
        size_t level;
        int dotted_level;
        const char *text;
        size_t text_length;
        line_start = offset[0];
        if (line_start >= length) {
            return 1;
        }
        if (lm_p0_scan_raw_comment_block(lm_lmx_message_thread, document, source, length, line_start, line[0], offset, line)) {
            continue;
        }
        if (document -> diagnostic -> code != 0) {
            return 0;
        }
        if (lm_p0_scan_block_string_event(lm_lmx_message_thread, document, source, length, line_start, line[0], event, offset, line)) {
            event->level = (((size_t)-1));
            has_event[0] = 1;
            return 1;
        }
        if (document -> diagnostic -> code != 0) {
            return 0;
        }
        line_end = lm_p0_find_layout_line_end(lm_lmx_message_thread, source, length, line_start);
        raw_length = (line_end - line_start);
        if ((raw_length > 0U) && (source[((line_start + raw_length) - 1U)] == '\r')) {
            raw_length = raw_length - 1U;
        }
        if (raw_length == 0U) {
            lm_p0_advance_layout_line(lm_lmx_message_thread, source, length, line_start, line_end, offset, line);
            continue;
        }
        p = line_start;
        level = 0U;
        dotted_level = 0;
        if (((((line[0] == 1U) && (raw_length >= 3U)) && ((((unsigned char)source[p])) == 0xEFU)) && ((((unsigned char)source[(p + 1U)])) == 0xBBU)) && ((((unsigned char)source[(p + 2U)])) == 0xBFU)) {
            p = p + 3U;
        }
        if ((p < (line_start + raw_length)) && (source[p] == '.')) {
            dotted_level = 1;
            while ((p < (line_start + raw_length)) && (source[p] == '.')) {
                level = level + 1U;
                p = p + 1U;
                while ((p < (line_start + raw_length)) && lm_p0_is_horizontal_space(lm_lmx_message_thread, source[p])) {
                    p = p + 1U;
                }
            }
        }
        else {
            size_t indent_column;
            size_t mark_skip;
            lm_p0_scan_indent_column(lm_lmx_message_thread, source, p, (line_start + raw_length), &p, &indent_column);
            text = (source + p);
            text_length = ((line_start + raw_length) - p);
            lm_p0_trim_right(lm_lmx_message_thread, &text, &text_length);
            mark_skip = 0U;
            if (lm_p0_skip_leading_brace_marks(lm_lmx_message_thread, document, text, text_length, line[0], ((((size_t)(text - (source + line_start)))) + 1U), &mark_skip) == 0) {
                return 0;
            }
            p = p + mark_skip;
            text = (source + p);
            text_length = ((line_start + raw_length) - p);
            lm_p0_trim_right(lm_lmx_message_thread, &text, &text_length);
            if ((text_length == 0U) || (text[0] == '#')) {
                lm_p0_advance_layout_line(lm_lmx_message_thread, source, length, line_start, line_end, offset, line);
                continue;
            }
            if (lm_p0_indent_level_from_column(lm_lmx_message_thread, document, indent_stack, indent_column, line[0], ((((size_t)(text - (source + line_start)))) + 1U), &level) == 0) {
                return 0;
            }
        }
        text = (source + p);
        text_length = ((line_start + raw_length) - p);
        lm_p0_trim_right(lm_lmx_message_thread, &text, &text_length);
        size_t mark_skip;
        mark_skip = 0U;
        if (lm_p0_skip_leading_brace_marks(lm_lmx_message_thread, document, text, text_length, line[0], ((((size_t)(text - (source + line_start)))) + 1U), &mark_skip) == 0) {
            return 0;
        }
        p = p + mark_skip;
        text = (source + p);
        text_length = ((line_start + raw_length) - p);
        lm_p0_trim_right(lm_lmx_message_thread, &text, &text_length);
        if ((text_length == 0U) || (text[0] == '#')) {
            if (dotted_level && (level > 0U)) {
                text_length = 0U;
            }
            else {
                lm_p0_advance_layout_line(lm_lmx_message_thread, source, length, line_start, line_end, offset, line);
                continue;
            }
        }
        memset(event, 0, sizeof(event[0]));
        event->level = level;
        event->text = text;
        event->text_length = text_length;
        event->line = line[0];
        event->column = ((((size_t)(text - (source + line_start)))) + 1U);
        event->offset = (((size_t)(text - source)));
        if ((text_length == 0U) && (level > 0U)) {
            event->kind = LM_P0_STREAM_EVENT_DELIM;
        }
        else {
            event->kind = LM_P0_STREAM_EVENT_ITEM;
        }
        lm_p0_advance_layout_line(lm_lmx_message_thread, source, length, line_start, line_end, offset, line);
        has_event[0] = 1;
        return 1;
    }
    return 1;
}

static int lm_p0_disabled_event_is_tail_cutter(struct LmMessageThread *lm_lmx_message_thread, const LmP0StreamEvent *event) {
    (void)lm_lmx_message_thread;
    if (event -> kind == LM_P0_STREAM_EVENT_DELIM) {
        return 1;
    }
    return lm_p0_stream_event_is_tail_cutter(lm_lmx_message_thread, event);
}

static int lm_p0_disabled_state_accept_event(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmP0DisabledState *state, LmP0StreamEvent *event, int *done_after_event, int *done_before_event) {
    (void)lm_lmx_message_thread;
    int is_tail_cutter;
    done_after_event[0] = 0;
    done_before_event[0] = 0;
    if (event -> level == (((size_t)-1))) {
        if (state -> pending_item) {
            event->level = (state -> pending_level + 1U);
        }
        else {
            event->level = state -> top_level;
        }
    }
    if ((event -> kind == LM_P0_STREAM_EVENT_ITEM) && (lm_p0_validate_disabled_item_text(lm_lmx_message_thread, document, event -> text, event -> text_length, event -> line, event -> column) == 0)) {
        return 0;
    }
    if (state -> pending_item) {
        if (event -> level == (state -> pending_level + 1U)) {
            state->body_started = 1;
            state->top_level = event -> level;
            state->pending_item = 0;
        }
        else {
            if (event -> level > (state -> pending_level + 1U)) {
                lm_p0_set_diagnostic(document, 13, event -> line, event -> column, "disabled block source level jumps too deep");
                return 0;
            }
            else {
                state->pending_item = 0;
            }
        }
    }
    is_tail_cutter = lm_p0_disabled_event_is_tail_cutter(lm_lmx_message_thread, event);
    if (state -> body_started == 0) {
        if ((event -> level == state -> base_level) && is_tail_cutter) {
            done_after_event[0] = 1;
            return 1;
        }
        if (event -> level <= state -> base_level) {
            done_before_event[0] = 1;
            return 1;
        }
        lm_p0_set_diagnostic(document, 8, event -> line, event -> column, "disabled block source level has no open parent");
        return 0;
    }
    if ((is_tail_cutter && (event -> level >= state -> base_level)) && ((event -> level + 1U) <= state -> top_level)) {
        state->top_level = event -> level;
        state->pending_item = 0;
        if (event -> level == state -> base_level) {
            done_after_event[0] = 1;
        }
        return 1;
    }
    if ((event -> level + 1U) < state -> top_level) {
        lm_p0_set_diagnostic(document, 13, event -> line, event -> column, "disabled block source level decrease must be one step unless a tail cutter is used");
        return 0;
    }
    if (event -> level <= state -> base_level) {
        done_before_event[0] = 1;
        return 1;
    }
    if (event -> level < state -> top_level) {
        state->top_level = event -> level;
    }
    if (event -> level > (state -> top_level + 1U)) {
        lm_p0_set_diagnostic(document, 8, event -> line, event -> column, "disabled block source level has no open parent");
        return 0;
    }
    if (event -> level == (state -> top_level + 1U)) {
        state->top_level = event -> level;
    }
    state->pending_item = 1;
    state->pending_level = event -> level;
    return 1;
}

static int lm_p0_validate_disabled_block(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, const LmP0IndentStack *indent_stack, size_t first_next_offset, size_t first_next_line, size_t base_level, const char *header_text, size_t header_length, size_t header_line, size_t header_column, size_t *out_offset, size_t *out_line) {
    (void)lm_lmx_message_thread;
    LmP0IndentStack * local_indent;
    LmP0DisabledState * state;
    LmP0StreamEvent * event;
    size_t offset;
    size_t line;
    int status;
    if (lm_p0_validate_disabled_item_text(lm_lmx_message_thread, document, header_text, header_length, header_line, header_column) == 0) {
        return 0;
    }
    local_indent = lm_p0_indent_stack_clone(lm_lmx_message_thread, document, indent_stack, header_line, header_column);
    if (local_indent == 0) {
        return 0;
    }
    state = lm_p0_disabled_state_new(lm_lmx_message_thread, base_level);
    if (state == 0) {
        lm_p0_set_diagnostic(document, 1, header_line, header_column, "out of memory while creating disabled block state");
        lm_p0_indent_stack_delete(lm_lmx_message_thread, local_indent);
        return 0;
    }
    event = lm_p0_stream_event_new(lm_lmx_message_thread);
    if (event == 0) {
        lm_p0_set_diagnostic(document, 1, header_line, header_column, "out of memory while creating disabled block event");
        lm_p0_disabled_state_delete(lm_lmx_message_thread, state);
        lm_p0_indent_stack_delete(lm_lmx_message_thread, local_indent);
        return 0;
    }
    offset = first_next_offset;
    line = first_next_line;
    status = 1;
    while (status && (offset <= document -> source_length)) {
        size_t event_offset;
        size_t event_line;
        int has_event;
        int done_after_event;
        int done_before_event;
        event_offset = offset;
        event_line = line;
        memset(event, 0, sizeof(event[0]));
        if (lm_p0_disabled_scan_next_event(lm_lmx_message_thread, document, local_indent, &offset, &line, event, &has_event) == 0) {
            status = 0;
            break;
        }
        if (has_event == 0) {
            if (state -> body_started && (state -> top_level > state -> base_level)) {
                lm_p0_set_diagnostic(document, 22, header_line, header_column, "unterminated disabled block");
                status = 0;
            }
            break;
        }
        if (lm_p0_disabled_state_accept_event(lm_lmx_message_thread, document, state, event, &done_after_event, &done_before_event) == 0) {
            status = 0;
            break;
        }
        if (done_before_event) {
            offset = event_offset;
            line = event_line;
            break;
        }
        if (done_after_event) {
            break;
        }
    }
    lm_p0_stream_event_delete(lm_lmx_message_thread, event);
    lm_p0_disabled_state_delete(lm_lmx_message_thread, state);
    lm_p0_indent_stack_delete(lm_lmx_message_thread, local_indent);
    out_offset[0] = offset;
    out_line[0] = line;
    return (status && (document -> diagnostic -> code == 0));
}

static int lm_p0_parse_stream(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document) {
    (void)lm_lmx_message_thread;
    const char *source;
    size_t length;
    size_t offset;
    size_t line;
    LmP0IndentStack * indent_stack;
    LmP0Stack * stack;
    LmP0PendingDelimiter * pending;
    LmP0PendingMix * pending_mix;
    LmP0StreamEvent * event;
    int status;
    int has_last_physical_level;
    size_t last_physical_level;
    source = document -> source;
    length = document -> source_length;
    offset = 0U;
    line = 1U;
    status = 1;
    has_last_physical_level = 0;
    last_physical_level = 0U;
    indent_stack = 0;
    stack = 0;
    pending = 0;
    pending_mix = 0;
    event = 0;
    document->root = lm_p0_new_node(lm_lmx_message_thread, document, LM_P0_NODE_STRUCTURE);
    if (document -> root == 0) {
        return 0;
    }
    document->root->span->offset = 0U;
    document->root->span->length = document -> source_length;
    indent_stack = lm_p0_indent_stack_new(lm_lmx_message_thread, document);
    stack = lm_p0_stack_new(lm_lmx_message_thread);
    pending = lm_p0_pending_delimiter_new(lm_lmx_message_thread);
    pending_mix = lm_p0_pending_mix_new(lm_lmx_message_thread);
    if (((((indent_stack == 0) || (stack == 0)) || (pending == 0)) || (pending_mix == 0)) || (lm_p0_stack_ensure(lm_lmx_message_thread, document, stack, 0U) == 0)) {
        if ((pending == 0) || (pending_mix == 0)) {
            lm_p0_set_diagnostic(document, 1, 0U, 0U, "out of memory while creating pending stream state");
        }
        lm_p0_indent_stack_delete(lm_lmx_message_thread, indent_stack);
        lm_p0_stack_delete(lm_lmx_message_thread, stack);
        lm_p0_pending_delimiter_delete(lm_lmx_message_thread, pending);
        lm_p0_pending_mix_delete(lm_lmx_message_thread, pending_mix);
        return 0;
    }
    stack->parents[0] = document -> root -> as -> structure;
    stack->owners[0] = document -> root;
    event = lm_p0_stream_event_new(lm_lmx_message_thread);
    if (event == 0) {
        lm_p0_set_diagnostic(document, 1, 0U, 0U, "out of memory while creating stream event");
        lm_p0_indent_stack_delete(lm_lmx_message_thread, indent_stack);
        lm_p0_stack_delete(lm_lmx_message_thread, stack);
        lm_p0_pending_delimiter_delete(lm_lmx_message_thread, pending);
        lm_p0_pending_mix_delete(lm_lmx_message_thread, pending_mix);
        return 0;
    }
    while (offset <= length) {
        size_t line_start;
        size_t line_end;
        size_t raw_length;
        size_t p;
        size_t level;
        int dotted_level;
        const char *text;
        size_t text_length;
        unsigned node_flags;
        int has_leading_mix_prefix;
        size_t mix_prefix_start;
        size_t mix_prefix_end;
        line_start = offset;
        if (lm_p0_scan_raw_comment_block(lm_lmx_message_thread, document, source, length, line_start, line, &offset, &line)) {
            continue;
        }
        if (document -> diagnostic -> code != 0) {
            status = 0;
            break;
        }
        if (lm_p0_scan_block_string_event(lm_lmx_message_thread, document, source, length, line_start, line, event, &offset, &line)) {
            event->level = lm_p0_stream_block_string_level(lm_lmx_message_thread, stack, pending);
            if (lm_p0_pending_mix_flush(lm_lmx_message_thread, document, stack, pending, pending_mix, event -> level) == 0) {
                status = 0;
                break;
            }
            if (lm_p0_stream_apply_event(lm_lmx_message_thread, document, stack, pending, event) == 0) {
                status = 0;
                break;
            }
            continue;
        }
        if (document -> diagnostic -> code != 0) {
            status = 0;
            break;
        }
        line_end = lm_p0_find_layout_line_end(lm_lmx_message_thread, source, length, offset);
        raw_length = (line_end - line_start);
        if ((raw_length > 0U) && (source[((line_start + raw_length) - 1U)] == '\r')) {
            raw_length = raw_length - 1U;
        }
        if (raw_length == 0U) {
            if (line_end == length) {
                break;
            }
            lm_p0_advance_layout_line(lm_lmx_message_thread, source, length, line_start, line_end, &offset, &line);
            continue;
        }
        p = line_start;
        level = 0U;
        dotted_level = 0;
        has_leading_mix_prefix = 0;
        mix_prefix_start = 0U;
        mix_prefix_end = 0U;
        if (((((line == 1U) && (raw_length >= 3U)) && ((((unsigned char)source[p])) == 0xEFU)) && ((((unsigned char)source[(p + 1U)])) == 0xBBU)) && ((((unsigned char)source[(p + 2U)])) == 0xBFU)) {
            p = p + 3U;
        }
        if (((p == line_start) && (raw_length > 0U)) && (source[p] == '{')) {
            int standalone_mix;
            standalone_mix = 0;
            if (lm_p0_line_is_standalone_mix_run(lm_lmx_message_thread, document, (source + p), ((line_start + raw_length) - p), line, ((((size_t)(p - line_start))) + 1U), &standalone_mix) == 0) {
                status = 0;
                break;
            }
            if (standalone_mix) {
                memset(event, 0, sizeof(event[0]));
                event->kind = LM_P0_STREAM_EVENT_MIX;
                event->text = (source + p);
                event->text_length = ((line_start + raw_length) - p);
                event->line = line;
                event->column = ((((size_t)(p - line_start))) + 1U);
                event->offset = p;
                if (lm_p0_pending_mix_push(lm_lmx_message_thread, document, pending_mix, event) == 0) {
                    status = 0;
                    break;
                }
                if (line_end == length) {
                    break;
                }
                lm_p0_advance_layout_line(lm_lmx_message_thread, source, length, line_start, line_end, &offset, &line);
                continue;
            }
        }
        size_t prefix_start;
        size_t prefix_end;
        size_t prefix_anchor;
        prefix_start = 0U;
        prefix_end = 0U;
        prefix_anchor = 0U;
        if (lm_p0_scan_leading_mix_prefix(lm_lmx_message_thread, document, (source + p), ((line_start + raw_length) - p), line, ((((size_t)(p - line_start))) + 1U), &prefix_start, &prefix_end, &prefix_anchor) == 0) {
            status = 0;
            break;
        }
        if (prefix_end > prefix_start) {
            has_leading_mix_prefix = 1;
            mix_prefix_start = (p + prefix_start);
            mix_prefix_end = (p + prefix_end);
            p = p + prefix_anchor;
        }
        if ((p < (line_start + raw_length)) && (source[p] == '.')) {
            dotted_level = 1;
            while ((p < (line_start + raw_length)) && (source[p] == '.')) {
                level = level + 1U;
                p = p + 1U;
                while ((p < (line_start + raw_length)) && lm_p0_is_horizontal_space(lm_lmx_message_thread, source[p])) {
                    p = p + 1U;
                }
            }
        }
        else {
            size_t indent_column;
            if (has_leading_mix_prefix) {
                indent_column = lm_p0_visual_column_between(lm_lmx_message_thread, source, line_start, p);
            }
            else {
                lm_p0_scan_indent_column(lm_lmx_message_thread, source, p, (line_start + raw_length), &p, &indent_column);
            }
            text = (source + p);
            text_length = ((line_start + raw_length) - p);
            lm_p0_trim_right(lm_lmx_message_thread, &text, &text_length);
            if ((text_length == 0U) || (text[0] == '#')) {
                if (line_end == length) {
                    break;
                }
                lm_p0_advance_layout_line(lm_lmx_message_thread, source, length, line_start, line_end, &offset, &line);
                continue;
            }
            if (lm_p0_indent_level_from_column(lm_lmx_message_thread, document, indent_stack, indent_column, line, ((((size_t)(text - (source + line_start)))) + 1U), &level) == 0) {
                status = 0;
                break;
            }
        }
        text = (source + p);
        text_length = ((line_start + raw_length) - p);
        lm_p0_trim_right(lm_lmx_message_thread, &text, &text_length);
        node_flags = 0U;
        if ((text_length == 0U) && (level == 0U)) {
            if (line_end == length) {
                break;
            }
            lm_p0_advance_layout_line(lm_lmx_message_thread, source, length, line_start, line_end, &offset, &line);
            continue;
        }
        if ((text_length > 0U) && (text[0] == '#')) {
            if (dotted_level && (level > 0U)) {
                text_length = 0U;
            }
            else {
                if (line_end == length) {
                    break;
                }
                lm_p0_advance_layout_line(lm_lmx_message_thread, source, length, line_start, line_end, &offset, &line);
                continue;
            }
        }
        if ((text_length > 0U) && (text[0] == '%')) {
            size_t marker_column;
            size_t header_column;
            size_t next_offset;
            size_t next_line;
            size_t disabled_next_offset;
            size_t disabled_next_line;
            size_t skip;
            node_flags = node_flags | LM_P0_NODE_INACTIVE;
            marker_column = ((((size_t)(text - (source + line_start)))) + 1U);
            skip = 1U;
            while ((skip < text_length) && lm_p0_is_horizontal_space(lm_lmx_message_thread, text[skip])) {
                skip = skip + 1U;
            }
            text = text + skip;
            text_length = text_length - skip;
            lm_p0_trim_trailing_line_comment(lm_lmx_message_thread, &text, &text_length);
            if ((text_length == 0U) || (text[0] == '#')) {
                lm_p0_set_diagnostic(document, 21, line, marker_column, "disabled marker must be followed by a source item");
                status = 0;
                break;
            }
            header_column = ((((size_t)(text - (source + line_start)))) + 1U);
            next_offset = line_start;
            next_line = line;
            lm_p0_advance_layout_line(lm_lmx_message_thread, source, length, line_start, line_end, &next_offset, &next_line);
            if (lm_p0_validate_disabled_block(lm_lmx_message_thread, document, indent_stack, next_offset, next_line, level, text, text_length, line, header_column, &disabled_next_offset, &disabled_next_line) == 0) {
                status = 0;
                break;
            }
            memset(event, 0, sizeof(event[0]));
            event->kind = LM_P0_STREAM_EVENT_DISABLED_BLOCK;
            event->level = level;
            event->node_flags = node_flags;
            event->text = text;
            event->text_length = text_length;
            event->line = line;
            event->column = header_column;
            event->offset = (((size_t)(text - source)));
            if (lm_p0_stream_apply_event(lm_lmx_message_thread, document, stack, pending, event) == 0) {
                status = 0;
                break;
            }
            offset = disabled_next_offset;
            line = disabled_next_line;
            continue;
        }
        memset(event, 0, sizeof(event[0]));
        event->level = level;
        event->node_flags = node_flags;
        event->text = text;
        event->text_length = text_length;
        event->line = line;
        event->column = ((((size_t)(text - (source + line_start)))) + 1U);
        event->offset = (((size_t)(text - source)));
        if ((text_length == 0U) && (level > 0U)) {
            event->kind = LM_P0_STREAM_EVENT_DELIM;
        }
        else {
            event->kind = LM_P0_STREAM_EVENT_ITEM;
        }
        if (has_last_physical_level) {
            if (level > (last_physical_level + 1U)) {
                lm_p0_set_diagnostic(document, 13, line, event -> column, "source level increase must be one step");
                status = 0;
                break;
            }
            if (((level + 1U) < last_physical_level) && (lm_p0_stream_event_is_tail_cutter(lm_lmx_message_thread, event) == 0)) {
                lm_p0_set_diagnostic(document, 13, line, event -> column, "source level decrease must be one step unless a tail cutter is used");
                status = 0;
                break;
            }
        }
        if (lm_p0_pending_mix_flush(lm_lmx_message_thread, document, stack, pending, pending_mix, level) == 0) {
            status = 0;
            break;
        }
        if (has_leading_mix_prefix) {
            LmP0StreamEvent * mix_event;
            mix_event = lm_p0_stream_event_new(lm_lmx_message_thread);
            if (mix_event == 0) {
                lm_p0_set_diagnostic(document, 1, line, ((((size_t)(mix_prefix_start - line_start))) + 1U), "out of memory while creating leading MIX event");
                status = 0;
                break;
            }
            mix_event->kind = LM_P0_STREAM_EVENT_MIX;
            mix_event->level = level;
            mix_event->text = (source + mix_prefix_start);
            mix_event->text_length = (mix_prefix_end - mix_prefix_start);
            mix_event->line = line;
            mix_event->column = ((((size_t)(mix_prefix_start - line_start))) + 1U);
            mix_event->offset = mix_prefix_start;
            if (lm_p0_stream_apply_event(lm_lmx_message_thread, document, stack, pending, mix_event) == 0) {
                lm_p0_stream_event_delete(lm_lmx_message_thread, mix_event);
                status = 0;
                break;
            }
            lm_p0_stream_event_delete(lm_lmx_message_thread, mix_event);
        }
        if (lm_p0_stream_apply_event(lm_lmx_message_thread, document, stack, pending, event) == 0) {
            status = 0;
            break;
        }
        has_last_physical_level = 1;
        last_physical_level = level;
        if (line_end == length) {
            break;
        }
        lm_p0_advance_layout_line(lm_lmx_message_thread, source, length, line_start, line_end, &offset, &line);
    }
    if (status && (lm_p0_pending_mix_flush(lm_lmx_message_thread, document, stack, pending, pending_mix, lm_p0_stream_block_string_level(lm_lmx_message_thread, stack, pending)) == 0)) {
        status = 0;
    }
    if (status && (lm_p0_stream_resolve_pending_delimiter(lm_lmx_message_thread, document, stack, pending, (((size_t)-1))) == 0)) {
        status = 0;
    }
    lm_p0_stream_event_delete(lm_lmx_message_thread, event);
    lm_p0_indent_stack_delete(lm_lmx_message_thread, indent_stack);
    lm_p0_stack_delete(lm_lmx_message_thread, stack);
    lm_p0_pending_delimiter_delete(lm_lmx_message_thread, pending);
    lm_p0_pending_mix_delete(lm_lmx_message_thread, pending_mix);
    return (status && (document -> diagnostic -> code == 0));
}

static void lm_p0_structure_recount(struct LmMessageThread *lm_lmx_message_thread, LmP0Structure *structure) {
    (void)lm_lmx_message_thread;
    LmP0Field * field;
    structure->field_count = 0U;
    structure->last_field = 0;
    field = structure -> first_field;
    while (field != 0) {
        structure->field_count = structure -> field_count + 1U;
        structure->last_field = field;
        field = field -> next;
    }
}

static LmP0PostprocessFrame * lm_p0_postprocess_frame_new(struct LmMessageThread *lm_lmx_message_thread, int phase) {
    (void)lm_lmx_message_thread;
    LmP0PostprocessFrame * frame;
    frame = (((LmP0PostprocessFrame *)lm_own_new_zero(sizeof(frame[0]))));
    if (frame != 0) {
        frame->phase = phase;
    }
    return frame;
}

static int lm_p0_postprocess_push_frame(struct LmMessageThread *lm_lmx_message_thread, LmOwnPtrStack *stack, LmP0PostprocessFrame *frame) {
    (void)lm_lmx_message_thread;
    if ((stack == 0) || (frame == 0)) {
        lm_own_delete(frame, 0);
        return 1;
    }
    if (lm_own_ptr_stack_push(stack, frame) != 0) {
        lm_own_delete(frame, 0);
        return 1;
    }
    return 0;
}

static int lm_p0_postprocess_push_node(struct LmMessageThread *lm_lmx_message_thread, LmOwnPtrStack *stack, LmP0Node *node) {
    (void)lm_lmx_message_thread;
    LmP0PostprocessFrame * frame;
    if (node == 0) {
        return 0;
    }
    frame = lm_p0_postprocess_frame_new(lm_lmx_message_thread, LM_P0_POSTPROCESS_NODE);
    if (frame != 0) {
        frame->node = node;
    }
    return lm_p0_postprocess_push_frame(lm_lmx_message_thread, stack, frame);
}

static int lm_p0_postprocess_push_structure(struct LmMessageThread *lm_lmx_message_thread, LmOwnPtrStack *stack, LmP0Structure *structure) {
    (void)lm_lmx_message_thread;
    LmP0PostprocessFrame * frame;
    if (structure == 0) {
        return 1;
    }
    frame = lm_p0_postprocess_frame_new(lm_lmx_message_thread, LM_P0_POSTPROCESS_STRUCTURE);
    if (frame != 0) {
        frame->structure = structure;
        frame->field = structure -> first_field;
    }
    return lm_p0_postprocess_push_frame(lm_lmx_message_thread, stack, frame);
}

static int lm_p0_postprocess_push_trailer(struct LmMessageThread *lm_lmx_message_thread, LmOwnPtrStack *stack, LmP0Trailer *trailer) {
    (void)lm_lmx_message_thread;
    LmP0PostprocessFrame * frame;
    if (trailer == 0) {
        return 0;
    }
    frame = lm_p0_postprocess_frame_new(lm_lmx_message_thread, LM_P0_POSTPROCESS_TRAILER);
    if (frame != 0) {
        frame->trailer = trailer;
    }
    return lm_p0_postprocess_push_frame(lm_lmx_message_thread, stack, frame);
}

static int lm_p0_postprocess_push_frame_wrap(struct LmMessageThread *lm_lmx_message_thread, LmOwnPtrStack *stack, LmP0Node *node) {
    (void)lm_lmx_message_thread;
    LmP0PostprocessFrame * frame;
    if (node == 0) {
        return 1;
    }
    frame = lm_p0_postprocess_frame_new(lm_lmx_message_thread, LM_P0_POSTPROCESS_FRAME_WRAP);
    if (frame != 0) {
        frame->node = node;
    }
    return lm_p0_postprocess_push_frame(lm_lmx_message_thread, stack, frame);
}

static LmOwnPtrStack * lm_p0_postprocess_stack_new(struct LmMessageThread *lm_lmx_message_thread) {
    (void)lm_lmx_message_thread;
    LmOwnPtrStack * stack;
    stack = (((LmOwnPtrStack *)lm_own_new_zero(sizeof(stack[0]))));
    if (stack != 0) {
        lm_own_ptr_stack_init(stack, lm_own_delete_plain);
    }
    return stack;
}

static void lm_p0_postprocess_stack_delete(struct LmMessageThread *lm_lmx_message_thread, LmOwnPtrStack **stack) {
    (void)lm_lmx_message_thread;
    if ((stack != 0) && (stack[0] != 0)) {
        lm_own_ptr_stack_destroy(stack[0]);
        lm_own_delete(stack[0], 0);
        stack[0] = 0;
    }
}

static int lm_p0_postprocess_run(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmOwnPtrStack *stack) {
    (void)lm_lmx_message_thread;
    LmP0PostprocessFrame * frame;
    LmP0Node * node;
    LmP0Structure * structure;
    int status;
    status = 1;
    while (((status != 0) && (stack != 0)) && (stack -> count != 0U)) {
        frame = (((LmP0PostprocessFrame *)lm_own_ptr_stack_top(stack)));
        if (frame == 0) {
            status = 0;
            break;
        }
        if (frame -> phase == LM_P0_POSTPROCESS_NODE) {
            node = frame -> node;
            lm_own_delete(lm_own_ptr_stack_pop(stack), 0);
            if (node == 0) {
                continue;
            }
            if (node -> kind == LM_P0_NODE_FRAME) {
                status = (((lm_p0_postprocess_push_trailer(lm_lmx_message_thread, stack, node -> as -> frame -> trailer) == 0) && (lm_p0_postprocess_push_frame_wrap(lm_lmx_message_thread, stack, node) == 0)) && (lm_p0_postprocess_push_structure(lm_lmx_message_thread, stack, node -> as -> frame -> body) == 0));
            }
            else {
                if (node -> kind == LM_P0_NODE_STRUCTURE) {
                    status = ((lm_p0_postprocess_push_trailer(lm_lmx_message_thread, stack, node -> as -> structure -> trailer) == 0) && (lm_p0_postprocess_push_structure(lm_lmx_message_thread, stack, node -> as -> structure) == 0));
                }
            }
            continue;
        }
        if (frame -> phase == LM_P0_POSTPROCESS_STRUCTURE) {
            while ((frame -> field != 0) && (frame -> field -> value == 0)) {
                frame->field = frame -> field -> next;
            }
            if (frame -> field == 0) {
                lm_p0_structure_recount(lm_lmx_message_thread, frame -> structure);
                lm_own_delete(lm_own_ptr_stack_pop(stack), 0);
                continue;
            }
            node = frame -> field -> value;
            frame->field = frame -> field -> next;
            status = (lm_p0_postprocess_push_node(lm_lmx_message_thread, stack, node) == 0);
            continue;
        }
        if (frame -> phase == LM_P0_POSTPROCESS_TRAILER) {
            if (frame -> trailer == 0) {
                lm_own_delete(lm_own_ptr_stack_pop(stack), 0);
                continue;
            }
            structure = frame -> trailer -> body;
            lm_own_delete(lm_own_ptr_stack_pop(stack), 0);
            status = (lm_p0_postprocess_push_structure(lm_lmx_message_thread, stack, structure) == 0);
            continue;
        }
        if (frame -> phase == LM_P0_POSTPROCESS_FRAME_WRAP) {
            node = frame -> node;
            lm_own_delete(lm_own_ptr_stack_pop(stack), 0);
            if (((node != 0) && ((node -> as -> frame -> flags & LM_P0_FRAME_INLINE_BODY) != 0U)) && (lm_p0_wrap_fields_from_line(lm_lmx_message_thread, document, node -> as -> frame -> body, node -> span -> line, (node -> span -> offset + node -> span -> length)) == 0)) {
                status = 0;
            }
            continue;
        }
        status = 0;
    }
    return status;
}

static int lm_p0_postprocess_node(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmP0Node *node) {
    (void)lm_lmx_message_thread;
    LmOwnPtrStack * stack;
    int status;
    if (node == 0) {
        return 1;
    }
    stack = lm_p0_postprocess_stack_new(lm_lmx_message_thread);
    if (stack == 0) {
        return 0;
    }
    status = ((lm_p0_postprocess_push_node(lm_lmx_message_thread, stack, node) == 0) && lm_p0_postprocess_run(lm_lmx_message_thread, document, stack));
    lm_p0_postprocess_stack_delete(lm_lmx_message_thread, &stack);
    return status;
}

static int lm_p0_wrap_fields_from_line(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, LmP0Structure *structure, size_t head_line, size_t inline_event_end_offset) {
    (void)lm_lmx_message_thread;
    LmP0Field * field;
    LmP0Field * previous;
    LmP0Field * group_first;
    LmP0Node * group_node;
    LmP0Field * move;
    previous = 0;
    field = structure -> first_field;
    while (field != 0) {
        if (((field -> value != 0) && (field -> value -> span -> line != head_line)) && (field -> value -> span -> offset >= inline_event_end_offset)) {
            break;
        }
        previous = field;
        field = field -> next;
    }
    if (field == 0) {
        return 1;
    }
    group_first = field;
    group_node = lm_p0_new_node(lm_lmx_message_thread, document, LM_P0_NODE_STRUCTURE);
    if (group_node == 0) {
        return 0;
    }
    group_node->span[0] = group_first -> value -> span[0];
    move = group_first;
    while (move != 0) {
        LmP0Field * next_move;
        LmP0Node * value;
        next_move = move -> next;
        value = move -> value;
        move->value = 0;
        if (lm_p0_append_field(lm_lmx_message_thread, document, group_node -> as -> structure, value) == 0) {
            lm_p0_free_node(lm_lmx_message_thread, group_node);
            return 0;
        }
        move = next_move;
    }
    group_first->value = group_node;
    group_first->next = 0;
    if (previous == 0) {
        structure->first_field = group_first;
    }
    else {
        previous->next = group_first;
    }
    lm_p0_structure_recount(lm_lmx_message_thread, structure);
    return 1;
}

static int lm_p0_validate_nonempty_colon_frames_in_trailer(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, const LmP0Trailer *trailer) {
    (void)lm_lmx_message_thread;
    if (trailer == 0) {
        return 1;
    }
    return lm_p0_validate_nonempty_colon_frames_in_structure(lm_lmx_message_thread, document, trailer -> body);
}

static int lm_p0_validate_nonempty_colon_frames_in_node(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, const LmP0Node *node) {
    (void)lm_lmx_message_thread;
    if (node == 0) {
        return 1;
    }
    if (node -> kind == LM_P0_NODE_FRAME) {
        if (lm_p0_validate_nonempty_colon_frames_in_structure(lm_lmx_message_thread, document, node -> as -> frame -> body) == 0) {
            return 0;
        }
        return lm_p0_validate_nonempty_colon_frames_in_trailer(lm_lmx_message_thread, document, node -> as -> frame -> trailer);
    }
    if (node -> kind == LM_P0_NODE_STRUCTURE) {
        if (lm_p0_validate_nonempty_colon_frames_in_structure(lm_lmx_message_thread, document, node -> as -> structure) == 0) {
            return 0;
        }
        return lm_p0_validate_nonempty_colon_frames_in_trailer(lm_lmx_message_thread, document, node -> as -> structure -> trailer);
    }
    return 1;
}

static int lm_p0_validate_nonempty_colon_frames_in_structure(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document, const LmP0Structure *structure) {
    (void)lm_lmx_message_thread;
    const LmP0Field * field;
    field = structure -> first_field;
    while (field != 0) {
        if (lm_p0_validate_nonempty_colon_frames_in_node(lm_lmx_message_thread, document, field -> value) == 0) {
            return 0;
        }
        field = field -> next;
    }
    return 1;
}

int lm_p0_parse_bytes(struct LmMessageThread *lm_lmx_message_thread, const char *source, size_t source_length, LmP0Document **out_document) {
    (void)lm_lmx_message_thread;
    LmP0Document * document;
    if (out_document == 0) {
        return 1;
    }
    out_document[0] = 0;
    document = (((LmP0Document *)lm_own_new_zero(sizeof(document[0]))));
    if (document == 0) {
        return 1;
    }
    if (lm_p0_document_init_owners(lm_lmx_message_thread, document) != 0) {
        lm_own_delete(document, 0);
        return 1;
    }
    if (source == 0) {
        source = "";
        source_length = 0U;
    }
    if (lm_p0_registry_load_default(lm_lmx_message_thread) != 0) {
        lm_p0_document_destroy_owners(lm_lmx_message_thread, document);
        lm_own_delete(document, 0);
        return 1;
    }
    document->source_length = source_length;
    document->source = lm_own_arena_copy_bytes(lm_lmx_message_thread, document -> source_owner, source, document -> source_length);
    if (document -> source == 0) {
        lm_p0_document_destroy_owners(lm_lmx_message_thread, document);
        lm_own_delete(document, 0);
        return 1;
    }
    if (lm_p0_parse_stream(lm_lmx_message_thread, document)) {
        if (lm_p0_postprocess_node(lm_lmx_message_thread, document, document -> root)) {
            lm_p0_validate_nonempty_colon_frames_in_node(lm_lmx_message_thread, document, document -> root);
        }
    }
    if (document -> diagnostic -> code == 0) {
        if (lm_own_tree_cut(lm_lmx_message_thread, document -> tree_arena) != 0) {
            lm_p0_set_diagnostic(document, 1, 0U, 0U, "out of memory while promoting parser lazy text edges");
        }
        else {
            lm_p0_document_freeze_tree(lm_lmx_message_thread, document);
        }
    }
    out_document[0] = document;
    if (document -> diagnostic -> code == 0) {
        return 0;
    }
    return document -> diagnostic -> code;
}

int lm_p0_parse_string(struct LmMessageThread *lm_lmx_message_thread, const char *source, LmP0Document **out_document) {
    (void)lm_lmx_message_thread;
    if (source == 0) {
        source = "";
    }
    return lm_p0_parse_bytes(lm_lmx_message_thread, source, strlen(source), out_document);
}

int lm_p0_parse_file(struct LmMessageThread *lm_lmx_message_thread, const char *path, LmP0Document **out_document) {
    (void)lm_lmx_message_thread;
    FILE * file;
    long size;
    char *buffer;
    size_t read_size;
    int status;
    if (out_document == 0) {
        return 1;
    }
    out_document[0] = 0;
    file = fopen(path, "rb");
    if (file == 0) {
        return 1;
    }
    if (fseek(file, 0L, SEEK_END) != 0) {
        fclose(file);
        return 1;
    }
    size = ftell(file);
    if (size < 0L) {
        fclose(file);
        return 1;
    }
    if (fseek(file, 0L, SEEK_SET) != 0) {
        fclose(file);
        return 1;
    }
    buffer = (((char *)lm_own_new_zero((((size_t)size)) + 1U)));
    if (buffer == 0) {
        fclose(file);
        return 1;
    }
    read_size = fread(buffer, 1U, (((size_t)size)), file);
    fclose(file);
    if (read_size != (((size_t)size))) {
        lm_own_delete(buffer, 0);
        return 1;
    }
    buffer[read_size] = '\0';
    status = lm_p0_parse_bytes(lm_lmx_message_thread, buffer, read_size, out_document);
    lm_own_delete(buffer, 0);
    return status;
}

static int lm_p0_registry_source_push_column_metadata(struct LmMessageThread *lm_lmx_message_thread, void *context, const LmP0Text *table_name, LmRegistrySourceColumn **columns, size_t column_count) {
    (void)lm_lmx_message_thread;
    LmP0Registry * registry;
    LmTableDescriptor * descriptor;
    registry = (((LmP0Registry *)context));
    if (registry == 0 || registry != lm_p0_registry_current(lm_lmx_message_thread) || registry -> view == 0 || table_name == 0) {
        return -1;
    }
    descriptor = lm_p0_registry_source_descriptor_new(lm_lmx_message_thread, table_name, columns, column_count);
    if (descriptor == 0) {
        return -1;
    }
    if (lm_registry_view_take_local_source_table(lm_lmx_message_thread, registry -> view, descriptor) != 0) {
        lm_table_descriptor_delete_any(descriptor);
        return -1;
    }
    return 0;
}

static int lm_p0_registry_source_push_table_row(struct LmMessageThread *lm_lmx_message_thread, void *context, const LmP0Text *table_name, LmRegistrySourceColumn **columns, size_t column_count, const LmP0Node **cells) {
    (void)lm_lmx_message_thread;
    LmP0Registry * registry;
    registry = (((LmP0Registry *)context));
    if (registry == 0 || registry != lm_p0_registry_current(lm_lmx_message_thread)) {
        return -1;
    }
    return lm_p0_registry_materialize_source_row(lm_lmx_message_thread, registry, table_name, columns, column_count, cells);
}

static int lm_p0_registry_source_join_table(struct LmMessageThread *lm_lmx_message_thread, void *context, const LmP0Text *source_table, const LmP0Text *target_table) {
    (void)lm_lmx_message_thread;
    LmP0Registry * registry;
    LmP0Text * source_name;
    LmP0Text * target_name;
    LmTableDescriptor * target;
    int status;
    registry = (((LmP0Registry *)context));
    if (registry == 0 || registry != lm_p0_registry_current(lm_lmx_message_thread) || registry -> view == 0 || source_table == 0 || target_table == 0) {
        return -1;
    }
    source_name = lm_p0_text_ref_new_empty(lm_lmx_message_thread);
    target_name = lm_p0_text_ref_new_empty(lm_lmx_message_thread);
    if ((source_name == 0) || (target_name == 0)) {
        lm_p0_text_ref_delete(lm_lmx_message_thread, source_name);
        lm_p0_text_ref_delete(lm_lmx_message_thread, target_name);
        return -1;
    }
    if ((lm_p0_registry_identifier_value(lm_lmx_message_thread, source_table, source_name) == 0) || (lm_p0_registry_identifier_value(lm_lmx_message_thread, target_table, target_name) == 0)) {
        lm_p0_text_ref_delete(lm_lmx_message_thread, source_name);
        lm_p0_text_ref_delete(lm_lmx_message_thread, target_name);
        return -1;
    }
    target = lm_registry_view_find_local_source_table_slice(lm_lmx_message_thread, registry -> view, target_name -> data, target_name -> length);
    status = -1;
    if (target != 0) {
        status = lm_registry_view_append_materialized_rows_mode(lm_lmx_message_thread, registry -> view, target, source_name -> data, source_name -> length, 1, 1, 0U, 0);
    }
    lm_p0_text_ref_delete(lm_lmx_message_thread, source_name);
    lm_p0_text_ref_delete(lm_lmx_message_thread, target_name);
    return status;
}

static int lm_p0_registry_source_text_all_char(struct LmMessageThread *lm_lmx_message_thread, const LmP0Text *text, char ch) {
    (void)lm_lmx_message_thread;
    size_t i;
    if (text == 0 || text -> length == 0U) {
        return 0;
    }
    i = 0U;
    while (i < text -> length) {
        if (text -> data[i] != ch) {
            return 0;
        }
        i = i + 1U;
    }
    return 1;
}

static int lm_p0_registry_source_text_is_array_receiver_head(struct LmMessageThread *lm_lmx_message_thread, const LmP0Text *head) {
    (void)lm_lmx_message_thread;
    size_t i;
    size_t count;
    if (head == 0 || head -> length == 0U) {
        return 0;
    }
    i = 0U;
    count = 0U;
    while (i < head -> length) {
        if (head -> data[i] != '[') {
            return 0;
        }
        i = i + 1U;
        while (i < head -> length && head -> data[i] != ']') {
            i = i + 1U;
        }
        if (i >= head -> length) {
            return 0;
        }
        i = i + 1U;
        count = count + 1U;
    }
    return count > 0U;
}

static const char * lm_p0_registry_source_frame_receiver_lookup(struct LmMessageThread *lm_lmx_message_thread, LmP0Registry *registry, const LmP0Frame *frame, const char *table) {
    (void)lm_lmx_message_thread;
    if (frame == 0) {
        return 0;
    }
    if (lm_p0_registry_source_text_all_char(lm_lmx_message_thread, frame -> head, '@') != 0) {
        return lm_p0_registry_lookup_cstr_in(lm_lmx_message_thread, registry, "@", table);
    }
    if (lm_p0_registry_source_text_is_array_receiver_head(lm_lmx_message_thread, frame -> head) != 0) {
        return lm_p0_registry_lookup_cstr_in(lm_lmx_message_thread, registry, "[]", table);
    }
    return lm_p0_registry_lookup_in(lm_lmx_message_thread, registry, frame -> head, table);
}

static int lm_p0_registry_source_parse_size_payload(struct LmMessageThread *lm_lmx_message_thread, const char *payload, size_t *out_value) {
    (void)lm_lmx_message_thread;
    char *end_index;
    unsigned long value;
    if (payload == 0 || payload[0] == '\0' || out_value == 0) {
        return 0;
    }
    end_index = 0;
    value = strtoul(payload, &end_index, 10);
    if (end_index == payload || end_index == 0 || end_index[0] != '\0') {
        return 0;
    }
    out_value[0] = ((size_t)value);
    return 1;
}

static int lm_p0_registry_source_formal_param_unwrap_index(struct LmMessageThread *lm_lmx_message_thread, const LmRegistrySourceLoader *loader, void *context, const LmP0Frame *frame, size_t *out_index) {
    (void)lm_lmx_message_thread;
    LmP0Registry * registry;
    const char *payload;
    LM_UNUSED(loader);
    registry = (((LmP0Registry *)context));
    if (registry == 0 || registry != lm_p0_registry_current(lm_lmx_message_thread)) {
        return 0;
    }
    payload = lm_p0_registry_source_frame_receiver_lookup(lm_lmx_message_thread, registry, frame, "receiver.formal-param.unwrap");
    if (payload == 0) {
        return 0;
    }
    return lm_p0_registry_source_parse_size_payload(lm_lmx_message_thread, payload, out_index);
}

static int lm_p0_registry_source_positional_name_index(struct LmMessageThread *lm_lmx_message_thread, const LmRegistrySourceLoader *loader, void *context, const LmP0Frame *frame, size_t *out_index) {
    (void)lm_lmx_message_thread;
    LmP0Registry * registry;
    const char *payload;
    LM_UNUSED(loader);
    registry = (((LmP0Registry *)context));
    if (registry == 0 || registry != lm_p0_registry_current(lm_lmx_message_thread)) {
        return 0;
    }
    payload = lm_p0_registry_source_frame_receiver_lookup(lm_lmx_message_thread, registry, frame, "receiver.positional-name.index");
    if (payload == 0) {
        payload = lm_p0_registry_source_frame_receiver_lookup(lm_lmx_message_thread, registry, frame, "receiver.positional-name.argument");
        if (payload == 0) {
            return 0;
        }
        out_index[0] = 0U;
        return 1;
    }
    return lm_p0_registry_source_parse_size_payload(lm_lmx_message_thread, payload, out_index);
}

static LmRegistrySourceLoader * lm_p0_registry_source_loader_new(struct LmMessageThread *lm_lmx_message_thread) {
    (void)lm_lmx_message_thread;
    LmRegistrySourceLoader * loader;
    loader = (((LmRegistrySourceLoader *)lm_own_new_zero(sizeof(loader[0]))));
    if (loader == 0) {
        return 0;
    }
    loader->error_prefix = "parser";
    loader->push_column_metadata = &lm_p0_registry_source_push_column_metadata;
    loader->push_table_row = &lm_p0_registry_source_push_table_row;
    loader->join_table = &lm_p0_registry_source_join_table;
    loader->formal_param_unwrap_index = &lm_p0_registry_source_formal_param_unwrap_index;
    loader->positional_name_index = &lm_p0_registry_source_positional_name_index;
    return loader;
}

static int lm_p0_path_has_extension(struct LmMessageThread *lm_lmx_message_thread, const char *path, const char *extension) {
    (void)lm_lmx_message_thread;
    size_t path_length;
    size_t extension_length;
    size_t i;
    char left;
    char right;
    if ((path == 0) || (extension == 0)) {
        return 0;
    }
    path_length = strlen(path);
    extension_length = strlen(extension);
    if (path_length < extension_length) {
        return 0;
    }
    i = 0U;
    while (i < extension_length) {
        left = path[((path_length - extension_length) + i)];
        right = extension[i];
        if ((left >= 'A') && (left <= 'Z')) {
            left = (((char)((left - 'A') + 'a')));
        }
        if ((right >= 'A') && (right <= 'Z')) {
            right = (((char)((right - 'A') + 'a')));
        }
        if (left != right) {
            return 0;
        }
        i = i + 1U;
    }
    return 1;
}

static int lm_p0_registry_require_source_only(struct LmMessageThread *lm_lmx_message_thread, LmP0Registry *registry, const char *phase) {
    (void)lm_lmx_message_thread;
    if (registry == 0 || registry -> view == 0) {
        return 1;
    }
    if (registry -> view -> parent != 0) {
        fprintf(stderr, "parser registry error: parser view is not isolated %s source-table load\n", phase);
        return 1;
    }
    if (lm_registry_view_fact_count(lm_lmx_message_thread, registry -> view) != 0U) {
        fprintf(stderr, "parser registry error: legacy facts appeared %s source-table load\n", phase);
        return 1;
    }
    return 0;
}

static int lm_p0_registry_parse_unsigned_payload(struct LmMessageThread *lm_lmx_message_thread, const char *payload, unsigned *out_value) {
    (void)lm_lmx_message_thread;
    char *end_index;
    unsigned long value;
    if ((payload == 0) || (out_value == 0)) {
        return 1;
    }
    end_index = 0;
    value = strtoul(payload, &end_index, 10);
    if ((((end_index == payload) || (end_index == 0)) || (end_index[0] != '\0')) || (value > (((unsigned long)UINT_MAX)))) {
        return 1;
    }
    out_value[0] = (((unsigned)value));
    return 0;
}

static const char * lm_p0_registry_lookup_key_by_unsigned_payload(struct LmMessageThread *lm_lmx_message_thread, const char *table, unsigned value) {
    (void)lm_lmx_message_thread;
    LmP0Registry * registry;
    size_t i;
    const LmTableCell * key_cell;
    const LmTableCell * payload_cell;
    const char *result;
    unsigned actual;
    registry = lm_p0_registry_current(lm_lmx_message_thread);
    if (table == 0 || registry == 0 || registry -> loaded == 0) {
        return 0;
    }
    result = 0;
    i = 0U;
    payload_cell = lm_p0_registry_source_path_cell_at(lm_lmx_message_thread, table, i, &key_cell);
    while (payload_cell != 0) {
        if (key_cell != 0 && key_cell -> value != 0 && payload_cell -> value != 0 && lm_p0_registry_parse_unsigned_payload(lm_lmx_message_thread, payload_cell -> value, &actual) == 0 && actual == value) {
            result = key_cell -> value;
        }
        i = i + 1U;
        payload_cell = lm_p0_registry_source_path_cell_at(lm_lmx_message_thread, table, i, &key_cell);
    }
    return result;
}

static int lm_p0_registry_validate_abi_constant(struct LmMessageThread *lm_lmx_message_thread, const char *table, const char *key, unsigned expected) {
    (void)lm_lmx_message_thread;
    const char *payload;
    unsigned actual;
    if (lm_p0_registry_table_has_rows(lm_lmx_message_thread, table) == 0) {
        return 0;
    }
    payload = lm_p0_registry_lookup_cstr(lm_lmx_message_thread, key, table);
    if (payload == 0) {
        fprintf(stderr, "parser registry error: missing ABI constant %s[%s]\n", table, key);
        return 1;
    }
    if (lm_p0_registry_parse_unsigned_payload(lm_lmx_message_thread, payload, &actual) != 0) {
        fprintf(stderr, "parser registry error: ABI constant %s[%s] expects unsigned integer payload\n", table, key);
        return 1;
    }
    if (actual != expected) {
        fprintf(stderr, "parser registry mismatch: ABI constant %s[%s] registry=%lu C=%lu\n", table, key, (((unsigned long)actual)), (((unsigned long)expected)));
        return 1;
    }
    return 0;
}

static int lm_p0_registry_validate_abi_constants(struct LmMessageThread *lm_lmx_message_thread) {
    (void)lm_lmx_message_thread;
    return (((((((((((lm_p0_registry_validate_abi_constant(lm_lmx_message_thread, "node.kind", "structure", LM_P0_NODE_STRUCTURE) || lm_p0_registry_validate_abi_constant(lm_lmx_message_thread, "node.kind", "frame", LM_P0_NODE_FRAME)) || lm_p0_registry_validate_abi_constant(lm_lmx_message_thread, "node.kind", "atom", LM_P0_NODE_ATOM)) || lm_p0_registry_validate_abi_constant(lm_lmx_message_thread, "node.kind", "disabled", LM_P0_NODE_DISABLED)) || lm_p0_registry_validate_abi_constant(lm_lmx_message_thread, "frame.flag", "colon", LM_P0_FRAME_COLON)) || lm_p0_registry_validate_abi_constant(lm_lmx_message_thread, "frame.flag", "compact", LM_P0_FRAME_COMPACT)) || lm_p0_registry_validate_abi_constant(lm_lmx_message_thread, "frame.flag", "inline-body", LM_P0_FRAME_INLINE_BODY)) || lm_p0_registry_validate_abi_constant(lm_lmx_message_thread, "frame.flag", "separator-closed", LM_P0_FRAME_SEPARATOR_CLOSED)) || lm_p0_registry_validate_abi_constant(lm_lmx_message_thread, "node.flag", "inactive", LM_P0_NODE_INACTIVE)) || lm_p0_registry_validate_abi_constant(lm_lmx_message_thread, "node.flag", "mix", LM_P0_NODE_MIX)) || lm_p0_registry_validate_abi_constant(lm_lmx_message_thread, "node.flag", "positional-skip", LM_P0_NODE_POSITIONAL_SKIP)) || lm_p0_registry_validate_abi_constant(lm_lmx_message_thread, "trailer.flag", "tail-cutter", LM_P0_TRAILER_TAIL_CUTTER));
}

const char * lm_p0_node_kind_class_name(struct LmMessageThread *lm_lmx_message_thread, LmP0NodeKind kind) {
    (void)lm_lmx_message_thread;
    const char *registry_name;
    registry_name = 0;
    if ((kind >= 0) && (lm_p0_registry_load_default(lm_lmx_message_thread) == 0)) {
        registry_name = lm_p0_registry_lookup_key_by_unsigned_payload(lm_lmx_message_thread, "node.kind", (((unsigned)kind)));
    }
    if (registry_name != 0) {
        return registry_name;
    }
    if (kind == LM_P0_NODE_STRUCTURE) {
        return "structure";
    }
    if (kind == LM_P0_NODE_FRAME) {
        return "frame";
    }
    if (kind == LM_P0_NODE_ATOM) {
        return "atom";
    }
    if (kind == LM_P0_NODE_DISABLED) {
        return "disabled";
    }
    return "unknown";
}

static int lm_p0_registry_load_default(struct LmMessageThread *lm_lmx_message_thread) {
    (void)lm_lmx_message_thread;
    LmP0Registry * registry;
    const char *override_path;
    const char *candidates[4];
    const char *registry_path;
    LmP0Document * document;
    const LmP0Diagnostic * diagnostic;
    LmRegistrySourceLoader * loader;
    int override_enabled;
    int status;
    size_t i;
    registry = lm_p0_registry_current(lm_lmx_message_thread);
    if (registry != 0 && (registry -> loaded || registry -> loading)) {
        return 0;
    }
    if (lm_p0_registry_init(lm_lmx_message_thread) != 0) {
        return 1;
    }
    registry = lm_p0_registry_current(lm_lmx_message_thread);
    if (registry == 0 || lm_p0_registry_require_source_only(lm_lmx_message_thread, registry, "before") != 0) {
        lm_p0_registry_destroy(lm_lmx_message_thread);
        return 1;
    }
    registry->loading = 1;
    if (lm_p0_registry_require_source_only(lm_lmx_message_thread, registry, "during") != 0) {
        lm_p0_registry_destroy(lm_lmx_message_thread);
        return 1;
    }
    override_path = getenv("LM_P0_REGISTRY");
    override_enabled = ((override_path != 0) && (override_path[0] != '\0'));
    if (override_enabled && lm_p0_path_has_extension(lm_lmx_message_thread, override_path, ".lm2") == 0) {
        fprintf(stderr, "parser registry error: LM_P0_REGISTRY accepts .lm2 source-table modules only: %s\n", override_path);
        lm_p0_registry_destroy(lm_lmx_message_thread);
        return 1;
    }
    if (override_enabled) {
        candidates[0] = override_path;
    }
    else {
        candidates[0] = "lm2/parser_registry.lm2";
    }
    candidates[1] = "../lm2/parser_registry.lm2";
    candidates[2] = "../../lm2/parser_registry.lm2";
    candidates[3] = 0;
    document = 0;
    registry_path = 0;
    i = 0U;
    while (candidates[i] != 0) {
        registry_path = candidates[i];
        status = lm_p0_parse_file(lm_lmx_message_thread, registry_path, &document);
        if (status == 0) {
            break;
        }
        diagnostic = lm_p0_document_diagnostic(lm_lmx_message_thread, document);
        if (diagnostic != 0) {
            fprintf(stderr, "parser registry parse error %d at %zu:%zu in %s: %s\n", diagnostic -> code, diagnostic -> line, diagnostic -> column, registry_path, diagnostic -> message);
            lm_p0_document_destroy(lm_lmx_message_thread, document);
            lm_p0_registry_destroy(lm_lmx_message_thread);
            return 1;
        }
        lm_p0_document_destroy(lm_lmx_message_thread, document);
        document = 0;
        if (override_enabled) {
            fprintf(stderr, "parser registry error: cannot read %s\n", registry_path);
            lm_p0_registry_destroy(lm_lmx_message_thread);
            return 1;
        }
        i = i + 1U;
    }
    if (document == 0) {
        if (lm_p0_registry_require_source_only(lm_lmx_message_thread, registry, "after") != 0) {
            lm_p0_registry_destroy(lm_lmx_message_thread);
            return 1;
        }
        registry->loaded = 1;
        registry->loading = 0;
        return 0;
    }
    loader = lm_p0_registry_source_loader_new(lm_lmx_message_thread);
    if (loader == 0) {
        lm_p0_document_destroy(lm_lmx_message_thread, document);
        lm_p0_registry_destroy(lm_lmx_message_thread);
        return 1;
    }
    status = lm_registry_source_load_root(lm_lmx_message_thread, loader, registry, lm_p0_document_root(lm_lmx_message_thread, document));
    lm_own_delete(loader, 0);
    lm_p0_document_destroy(lm_lmx_message_thread, document);
    if (status != 0) {
        lm_p0_registry_destroy(lm_lmx_message_thread);
        return 1;
    }
    if (lm_p0_registry_require_source_only(lm_lmx_message_thread, registry, "after") != 0) {
        lm_p0_registry_destroy(lm_lmx_message_thread);
        return 1;
    }
    registry->loaded = 1;
    registry->loading = 0;
    if (lm_p0_registry_validate_abi_constants(lm_lmx_message_thread) != 0) {
        lm_p0_registry_destroy(lm_lmx_message_thread);
        return 1;
    }
    if (lm_p0_registry_require_source_only(lm_lmx_message_thread, registry, "after validation") != 0) {
        lm_p0_registry_destroy(lm_lmx_message_thread);
        return 1;
    }
    return 0;
}

static int lm_p0_registry_selftest_expect_path(struct LmMessageThread *lm_lmx_message_thread, const char *path, size_t index, const char *expected_key, const char *expected_value) {
    (void)lm_lmx_message_thread;
    const LmTableCell * key_cell;
    const LmTableCell * payload_cell;
    payload_cell = lm_p0_registry_source_path_cell_at(lm_lmx_message_thread, path, index, &key_cell);
    if (payload_cell == 0 || key_cell == 0 || key_cell -> value == 0 || payload_cell -> value == 0) {
        return 1;
    }
    if (strcmp(key_cell -> value, expected_key) != 0 || strcmp(payload_cell -> value, expected_value) != 0) {
        return 1;
    }
    return 0;
}

static int lm_p0_registry_selftest_expect_lookup(struct LmMessageThread *lm_lmx_message_thread, const char *path, const char *key, const char *expected_value) {
    (void)lm_lmx_message_thread;
    LmP0Text * key_text;
    const char *actual;
    int status;
    key_text = lm_p0_text_from_cstr(lm_lmx_message_thread, key);
    if (key_text == 0) {
        return 1;
    }
    actual = lm_p0_registry_lookup(lm_lmx_message_thread, key_text, path);
    status = actual == 0 || strcmp(actual, expected_value) != 0;
    lm_p0_text_view_delete(lm_lmx_message_thread, key_text);
    return status;
}

static int lm_p0_registry_source_tables_selftest(struct LmMessageThread *lm_lmx_message_thread) {
    (void)lm_lmx_message_thread;
    LmP0Registry * registry;
    const LmTableDescriptor * alpha;
    const LmTableDescriptor * gamma;
    const LmTableDescriptor * synthetic;
    const LmTableColumnDescriptor * column;
    const LmTableRow * alpha_row;
    const LmTableRow * gamma_row;
    const LmTableRow * none_row;
    const LmTableRow * joined_none_row;
    const LmTableCell * alpha_cell;
    const LmTableCell * gamma_cell;
    const LmTableCell * key_cell;
    const LmTableCell * none_cell;
    LmP0Text * bare_key;
    LmP0Text * incompatible_source;
    LmP0Text * incompatible_target;
    size_t index;
    size_t synthetic_row_count;
    int covered;
    int status;
    lm_p0_registry_destroy(lm_lmx_message_thread);
    if (lm_p0_registry_load_default(lm_lmx_message_thread) != 0) {
        lm_p0_registry_destroy(lm_lmx_message_thread);
        return 1;
    }
    registry = lm_p0_registry_current(lm_lmx_message_thread);
    if (registry == 0 || registry -> view == 0) {
        lm_p0_registry_destroy(lm_lmx_message_thread);
        return 1;
    }
    status = 0;
    if (lm_registry_view_fact_count(lm_lmx_message_thread, registry -> view) != 0U || lm_registry_view_source_table_count(lm_lmx_message_thread, registry -> view) != 5U) {
        status = 1;
    }
    alpha = lm_registry_view_find_local_source_table_slice(lm_lmx_message_thread, registry -> view, "alpha", 5U);
    gamma = lm_registry_view_find_local_source_table_slice(lm_lmx_message_thread, registry -> view, "gamma", 5U);
    synthetic = lm_registry_view_find_local_source_table_slice(lm_lmx_message_thread, registry -> view, "synthetic", 9U);
    if (status == 0 && (alpha == 0 || gamma == 0 || synthetic == 0 || lm_table_descriptor_column_count(lm_lmx_message_thread, alpha) != 4U || lm_table_descriptor_materialized_row_count(lm_lmx_message_thread, alpha) != 4U || lm_table_descriptor_column_count(lm_lmx_message_thread, gamma) != 4U || lm_table_descriptor_materialized_row_count(lm_lmx_message_thread, gamma) != 6U || lm_table_descriptor_column_count(lm_lmx_message_thread, synthetic) != 2U || lm_table_descriptor_materialized_row_count(lm_lmx_message_thread, synthetic) != 1U)) {
        status = 1;
    }
    column = lm_table_descriptor_column_at(lm_lmx_message_thread, alpha, 0U);
    if (status == 0 && (column == 0 || column -> index != 0U || column -> name == 0 || column -> type_name == 0 || strcmp(column -> name, "class") != 0 || strcmp(column -> type_name, "class") != 0 || lm_table_column_descriptor_descriptor_count(lm_lmx_message_thread, column) != 1U || strcmp(lm_table_column_descriptor_descriptor_at(lm_lmx_message_thread, column, 0U), "class") != 0)) {
        status = 1;
    }
    column = lm_table_descriptor_column_at(lm_lmx_message_thread, alpha, 1U);
    if (status == 0 && (column == 0 || column -> index != 1U || column -> name == 0 || column -> type_name == 0 || strcmp(column -> name, "left") != 0 || strcmp(column -> type_name, "char") != 0 || column -> address_depth != 0U || column -> array_rank != 0U || column -> is_const != 0 || lm_table_column_descriptor_descriptor_count(lm_lmx_message_thread, column) != 1U || strcmp(lm_table_column_descriptor_descriptor_at(lm_lmx_message_thread, column, 0U), "char") != 0)) {
        status = 1;
    }
    column = lm_table_descriptor_column_at(lm_lmx_message_thread, alpha, 2U);
    if (status == 0 && (column == 0 || column -> index != 2U || column -> name == 0 || column -> type_name == 0 || strcmp(column -> name, "number") != 0 || strcmp(column -> type_name, "int") != 0 || lm_table_column_descriptor_descriptor_count(lm_lmx_message_thread, column) != 1U || strcmp(lm_table_column_descriptor_descriptor_at(lm_lmx_message_thread, column, 0U), "int") != 0)) {
        status = 1;
    }
    column = lm_table_descriptor_column_at(lm_lmx_message_thread, alpha, 3U);
    if (status == 0 && (column == 0 || column -> index != 3U || column -> name == 0 || column -> type_name == 0 || strcmp(column -> name, "enabled") != 0 || strcmp(column -> type_name, "boolean") != 0 || lm_table_column_descriptor_descriptor_count(lm_lmx_message_thread, column) != 1U || strcmp(lm_table_column_descriptor_descriptor_at(lm_lmx_message_thread, column, 0U), "boolean") != 0)) {
        status = 1;
    }
    covered = 1;
    if (status == 0 && (lm_registry_view_source_path_has_rows(lm_lmx_message_thread, registry -> view, "alpha", &covered) != 0 || covered != 0)) {
        status = 1;
    }
    covered = 0;
    if (status == 0 && (lm_registry_view_source_path_has_rows(lm_lmx_message_thread, registry -> view, "alpha.left", &covered) == 0 || covered == 0)) {
        status = 1;
    }
    covered = 0;
    if (status == 0 && (lm_registry_view_source_path_has_key_slice(lm_lmx_message_thread, registry -> view, "alpha.left", 10U, "noneKey", 7U, &covered) != 0 || covered == 0)) {
        status = 1;
    }
    if (status == 0 && (lm_p0_registry_selftest_expect_path(lm_lmx_message_thread, "alpha.left", 0U, "first", "left-old") != 0 || lm_p0_registry_selftest_expect_path(lm_lmx_message_thread, "alpha.left", 1U, "duplicate", "left-first") != 0 || lm_p0_registry_selftest_expect_path(lm_lmx_message_thread, "alpha.left", 2U, "duplicate", "left-last") != 0 || lm_p0_registry_source_path_cell_at(lm_lmx_message_thread, "alpha.left", 3U, &key_cell) != 0)) {
        status = 1;
    }
    if (status == 0 && (lm_p0_registry_selftest_expect_path(lm_lmx_message_thread, "gamma.left", 0U, "first", "left-old") != 0 || lm_p0_registry_selftest_expect_path(lm_lmx_message_thread, "gamma.left", 1U, "duplicate", "left-first") != 0 || lm_p0_registry_selftest_expect_path(lm_lmx_message_thread, "gamma.left", 2U, "duplicate", "left-last") != 0 || lm_p0_registry_selftest_expect_path(lm_lmx_message_thread, "gamma.left", 3U, "betaKey", "beta-left") != 0 || lm_p0_registry_selftest_expect_path(lm_lmx_message_thread, "gamma.left", 4U, "directKey", "direct-left") != 0 || lm_p0_registry_source_path_cell_at(lm_lmx_message_thread, "gamma.left", 5U, &key_cell) != 0)) {
        status = 1;
    }
    if (status == 0 && (lm_p0_registry_selftest_expect_lookup(lm_lmx_message_thread, "alpha.left", "duplicate", "left-last") != 0 || lm_p0_registry_selftest_expect_lookup(lm_lmx_message_thread, "alpha.number", "duplicate", "3") != 0 || lm_p0_registry_selftest_expect_lookup(lm_lmx_message_thread, "gamma.left", "duplicate", "left-last") != 0 || lm_p0_registry_selftest_expect_lookup(lm_lmx_message_thread, "columnOrder.value", "duplicateColumn", "last-column") != 0 || lm_p0_registry_selftest_expect_lookup(lm_lmx_message_thread, "synthetic", "explicitKey", "explicit-value") != 0)) {
        status = 1;
    }
    bare_key = lm_p0_text_from_cstr(lm_lmx_message_thread, "duplicate");
    if (status == 0 && (bare_key == 0 || lm_p0_registry_lookup(lm_lmx_message_thread, bare_key, "alpha") != 0)) {
        status = 1;
    }
    lm_p0_text_view_delete(lm_lmx_message_thread, bare_key);
    incompatible_source = lm_p0_text_from_cstr(lm_lmx_message_thread, "alpha");
    incompatible_target = lm_p0_text_from_cstr(lm_lmx_message_thread, "synthetic");
    synthetic_row_count = lm_table_descriptor_materialized_row_count(lm_lmx_message_thread, synthetic);
    if (status == 0 && (incompatible_source == 0 || incompatible_target == 0 || lm_p0_registry_source_join_table(lm_lmx_message_thread, registry, incompatible_source, incompatible_target) == 0 || lm_table_descriptor_materialized_row_count(lm_lmx_message_thread, synthetic) != synthetic_row_count)) {
        status = 1;
    }
    lm_p0_text_view_delete(lm_lmx_message_thread, incompatible_source);
    lm_p0_text_view_delete(lm_lmx_message_thread, incompatible_target);
    none_row = lm_table_descriptor_materialized_row_at(lm_lmx_message_thread, alpha, 2U);
    joined_none_row = lm_table_descriptor_materialized_row_at(lm_lmx_message_thread, gamma, 2U);
    index = 1U;
    while (status == 0 && index < 4U) {
        none_cell = lm_table_row_cell_at(lm_lmx_message_thread, none_row, index);
        if (none_cell == 0 || none_cell -> atom == 0 || strcmp(none_cell -> atom, "None") != 0 || none_cell -> value != 0 || none_cell -> node != 0 || none_cell -> source != 0 || none_cell -> explicit_none == 0) {
            status = 1;
        }
        none_cell = lm_table_row_cell_at(lm_lmx_message_thread, joined_none_row, index);
        if (none_cell == 0 || none_cell -> value != 0 || none_cell -> node != 0 || none_cell -> source != 0 || none_cell -> explicit_none == 0) {
            status = 1;
        }
        index = index + 1U;
    }
    alpha_row = lm_table_descriptor_materialized_row_at(lm_lmx_message_thread, alpha, 0U);
    gamma_row = lm_table_descriptor_materialized_row_at(lm_lmx_message_thread, gamma, 0U);
    alpha_cell = lm_table_row_cell_at(lm_lmx_message_thread, alpha_row, 1U);
    gamma_cell = lm_table_row_cell_at(lm_lmx_message_thread, gamma_row, 1U);
    if (status == 0 && (alpha_row == 0 || gamma_row == 0 || gamma_row == alpha_row || gamma_row -> source != alpha_row || alpha_cell == 0 || gamma_cell == 0 || alpha_cell -> atom == 0 || gamma_cell -> atom == 0 || alpha_cell -> value == 0 || gamma_cell -> value == 0 || gamma_cell == alpha_cell || alpha_cell -> atom == gamma_cell -> atom || alpha_cell -> value == gamma_cell -> value || gamma_cell -> source != 0 || strcmp(alpha_cell -> value, gamma_cell -> value) != 0)) {
        status = 1;
    }
    if (lm_registry_view_fact_count(lm_lmx_message_thread, registry -> view) != 0U || lm_p0_registry_require_source_only(lm_lmx_message_thread, registry, "after selftest lookups") != 0) {
        status = 1;
    }
    lm_p0_registry_destroy(lm_lmx_message_thread);
    return status;
}

void lm_p0_document_destroy(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document) {
    (void)lm_lmx_message_thread;
    if (document == 0) {
        return;
    }
    if (lm_p0_document_owners_belong_to_actor(lm_lmx_message_thread, document) == 0) {
        return;
    }
    lm_p0_document_destroy_owners(lm_lmx_message_thread, document);
    lm_own_delete(document, 0);
}

const LmP0Node * lm_p0_document_root(struct LmMessageThread *lm_lmx_message_thread, const LmP0Document *document) {
    (void)lm_lmx_message_thread;
    if (document != 0) {
        return document -> root;
    }
    return 0;
}

LmP0Node * lm_p0_document_mutable_root(struct LmMessageThread *lm_lmx_message_thread, LmP0Document *document) {
    (void)lm_lmx_message_thread;
    if (document != 0) {
        return document -> root;
    }
    return 0;
}

const LmP0Diagnostic * lm_p0_document_diagnostic(struct LmMessageThread *lm_lmx_message_thread, const LmP0Document *document) {
    (void)lm_lmx_message_thread;
    if (((document == 0) || (document -> diagnostic == 0)) || (document -> diagnostic -> code == 0)) {
        return 0;
    }
    return document -> diagnostic;
}

void lm_p0_free(struct LmMessageThread *lm_lmx_message_thread, void *ptr) {
    (void)lm_lmx_message_thread;
    lm_own_delete(ptr, 0);
}

static int lm_p0_dump_reserve(struct LmMessageThread *lm_lmx_message_thread, LmP0Dump *dump, size_t extra) {
    (void)lm_lmx_message_thread;
    char *data;
    size_t new_capacity;
    if (dump -> failed) {
        return 0;
    }
    if (((dump -> length + extra) + 1U) <= dump -> capacity) {
        return 1;
    }
    if (dump -> capacity == 0U) {
        new_capacity = 256U;
    }
    else {
        new_capacity = dump -> capacity;
    }
    while (new_capacity < ((dump -> length + extra) + 1U)) {
        new_capacity = new_capacity * 2U;
    }
    data = (((char *)lm_own_resize(dump -> data, new_capacity)));
    if (data == 0) {
        dump->failed = 1;
        return 0;
    }
    dump->data = data;
    dump->capacity = new_capacity;
    return 1;
}

static void lm_p0_dump_append(LmP0Dump *dump, const char *text, size_t length) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    if (lm_p0_dump_reserve(lm_lmx_message_thread, dump, length) == 0) {
        return;
    }
    memcpy((dump -> data + dump -> length), text, length);
    dump->length = dump -> length + length;
    dump->data[dump -> length] = '\0';
}

static void lm_p0_dump_append_cstr(LmP0Dump *dump, const char *text) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    lm_p0_dump_append(dump, text, strlen(text));
}

static void lm_p0_dump_indent(struct LmMessageThread *lm_lmx_message_thread, LmP0Dump *dump, size_t indent) {
    (void)lm_lmx_message_thread;
    size_t i;
    i = 0U;
    while (i < indent) {
        lm_p0_dump_append_cstr(dump, "  ");
        i = i + 1U;
    }
}

static void lm_p0_dump_text(struct LmMessageThread *lm_lmx_message_thread, LmP0Dump *dump, const LmP0Text *text) {
    (void)lm_lmx_message_thread;
    lm_p0_dump_append_cstr(dump, "\"");
    if (text != 0) {
        lm_p0_dump_append(dump, text -> data, text -> length);
    }
    lm_p0_dump_append_cstr(dump, "\"");
}

static LmP0DumpFrame * lm_p0_dump_frame_new(struct LmMessageThread *lm_lmx_message_thread, int phase, size_t indent) {
    (void)lm_lmx_message_thread;
    LmP0DumpFrame * frame;
    frame = (((LmP0DumpFrame *)lm_own_new_zero(sizeof(frame[0]))));
    if (frame != 0) {
        frame->phase = phase;
        frame->indent = indent;
    }
    return frame;
}

static int lm_p0_dump_push_frame(struct LmMessageThread *lm_lmx_message_thread, LmOwnPtrStack *stack, LmP0DumpFrame *frame) {
    (void)lm_lmx_message_thread;
    if ((stack == 0) || (frame == 0)) {
        lm_own_delete(frame, 0);
        return 1;
    }
    if (lm_own_ptr_stack_push(stack, frame) != 0) {
        lm_own_delete(frame, 0);
        return 1;
    }
    return 0;
}

static int lm_p0_dump_push_node(struct LmMessageThread *lm_lmx_message_thread, LmOwnPtrStack *stack, const LmP0Node *node, size_t indent) {
    (void)lm_lmx_message_thread;
    LmP0DumpFrame * frame;
    if (node == 0) {
        return 0;
    }
    frame = lm_p0_dump_frame_new(lm_lmx_message_thread, LM_P0_DUMP_NODE, indent);
    if (frame != 0) {
        frame->node = node;
    }
    return lm_p0_dump_push_frame(lm_lmx_message_thread, stack, frame);
}

static int lm_p0_dump_push_structure(struct LmMessageThread *lm_lmx_message_thread, LmOwnPtrStack *stack, const LmP0Structure *structure, size_t indent) {
    (void)lm_lmx_message_thread;
    LmP0DumpFrame * frame;
    if (structure == 0) {
        return 0;
    }
    frame = lm_p0_dump_frame_new(lm_lmx_message_thread, LM_P0_DUMP_STRUCTURE, indent);
    if (frame != 0) {
        frame->structure = structure;
        frame->field = structure -> first_field;
    }
    return lm_p0_dump_push_frame(lm_lmx_message_thread, stack, frame);
}

static int lm_p0_dump_push_trailer(struct LmMessageThread *lm_lmx_message_thread, LmOwnPtrStack *stack, const LmP0Trailer *trailer, size_t indent) {
    (void)lm_lmx_message_thread;
    LmP0DumpFrame * frame;
    if (trailer == 0) {
        return 0;
    }
    frame = lm_p0_dump_frame_new(lm_lmx_message_thread, LM_P0_DUMP_TRAILER, indent);
    if (frame != 0) {
        frame->trailer = trailer;
    }
    return lm_p0_dump_push_frame(lm_lmx_message_thread, stack, frame);
}

static LmOwnPtrStack * lm_p0_dump_stack_new(struct LmMessageThread *lm_lmx_message_thread) {
    (void)lm_lmx_message_thread;
    LmOwnPtrStack * stack;
    stack = (((LmOwnPtrStack *)lm_own_new_zero(sizeof(stack[0]))));
    if (stack != 0) {
        lm_own_ptr_stack_init(stack, lm_own_delete_plain);
    }
    return stack;
}

static void lm_p0_dump_stack_delete(struct LmMessageThread *lm_lmx_message_thread, LmOwnPtrStack **stack) {
    (void)lm_lmx_message_thread;
    if ((stack != 0) && (stack[0] != 0)) {
        lm_own_ptr_stack_destroy(stack[0]);
        lm_own_delete(stack[0], 0);
        stack[0] = 0;
    }
}

static void lm_p0_dump_run(struct LmMessageThread *lm_lmx_message_thread, LmP0Dump *dump, LmOwnPtrStack *stack) {
    (void)lm_lmx_message_thread;
    LmP0DumpFrame * frame;
    const LmP0Node * node;
    const LmP0Structure * structure;
    const LmP0Trailer * trailer;
    const char *structure_name;
    size_t indent;
    while ((((dump != 0) && (dump -> failed == 0)) && (stack != 0)) && (stack -> count != 0U)) {
        frame = (((LmP0DumpFrame *)lm_own_ptr_stack_top(stack)));
        if (frame == 0) {
            dump->failed = 1;
            return;
        }
        if (frame -> phase == LM_P0_DUMP_NODE) {
            node = frame -> node;
            indent = frame -> indent;
            lm_own_delete(lm_own_ptr_stack_pop(stack), 0);
            if (node == 0) {
                continue;
            }
            structure_name = lm_p0_node_kind_class_name(lm_lmx_message_thread, LM_P0_NODE_STRUCTURE);
            lm_p0_dump_indent(lm_lmx_message_thread, dump, indent);
            if ((node -> flags & LM_P0_NODE_INACTIVE) != 0U) {
                lm_p0_dump_append_cstr(dump, "Inactive ");
            }
            if ((node -> flags & LM_P0_NODE_MIX) != 0U) {
                lm_p0_dump_append_cstr(dump, "MIX ");
            }
            if ((node -> flags & LM_P0_NODE_POSITIONAL_SKIP) != 0U) {
                lm_p0_dump_append_cstr(dump, "PositionalSkip ");
            }
            if (node -> kind == LM_P0_NODE_STRUCTURE) {
                lm_p0_dump_append_cstr(dump, lm_p0_node_kind_class_name(lm_lmx_message_thread, node -> kind));
                lm_p0_dump_append_field_count_line(lm_lmx_message_thread, dump, node -> as -> structure -> field_count);
                if ((lm_p0_dump_push_trailer(lm_lmx_message_thread, stack, node -> as -> structure -> trailer, (indent + 1U)) != 0) || (lm_p0_dump_push_structure(lm_lmx_message_thread, stack, node -> as -> structure, (indent + 1U)) != 0)) {
                    dump->failed = 1;
                }
            }
            else {
                if (node -> kind == LM_P0_NODE_FRAME) {
                    lm_p0_dump_append_cstr(dump, lm_p0_node_kind_class_name(lm_lmx_message_thread, node -> kind));
                    lm_p0_dump_append_cstr(dump, " head=");
                    lm_p0_dump_text(lm_lmx_message_thread, dump, node -> as -> frame -> head);
                    lm_p0_dump_append_cstr(dump, " body=");
                    lm_p0_dump_append_cstr(dump, structure_name);
                    lm_p0_dump_append_field_count_line(lm_lmx_message_thread, dump, node -> as -> frame -> body -> field_count);
                    if ((lm_p0_dump_push_trailer(lm_lmx_message_thread, stack, node -> as -> frame -> trailer, (indent + 1U)) != 0) || (lm_p0_dump_push_structure(lm_lmx_message_thread, stack, node -> as -> frame -> body, (indent + 1U)) != 0)) {
                        dump->failed = 1;
                    }
                }
                else {
                    if (node -> kind == LM_P0_NODE_ATOM) {
                        lm_p0_dump_append_cstr(dump, lm_p0_node_kind_class_name(lm_lmx_message_thread, node -> kind));
                        lm_p0_dump_append_cstr(dump, " ");
                        lm_p0_dump_text(lm_lmx_message_thread, dump, node -> as -> atom);
                        lm_p0_dump_append_cstr(dump, "\n");
                    }
                    else {
                        if (node -> kind == LM_P0_NODE_DISABLED) {
                            lm_p0_dump_append_cstr(dump, lm_p0_node_kind_class_name(lm_lmx_message_thread, node -> kind));
                            lm_p0_dump_append_cstr(dump, " ");
                            lm_p0_dump_text(lm_lmx_message_thread, dump, node -> as -> atom);
                            lm_p0_dump_append_cstr(dump, "\n");
                        }
                        else {
                            lm_p0_dump_append_cstr(dump, lm_p0_node_kind_class_name(lm_lmx_message_thread, node -> kind));
                            lm_p0_dump_append_cstr(dump, " kind=");
                            lm_p0_dump_append_size(lm_lmx_message_thread, dump, (((size_t)node -> kind)));
                            lm_p0_dump_append_cstr(dump, "\n");
                        }
                    }
                }
            }
            continue;
        }
        if (frame -> phase == LM_P0_DUMP_STRUCTURE) {
            while ((frame -> field != 0) && (frame -> field -> value == 0)) {
                frame->field = frame -> field -> next;
            }
            if (frame -> field == 0) {
                lm_own_delete(lm_own_ptr_stack_pop(stack), 0);
                continue;
            }
            node = frame -> field -> value;
            frame->field = frame -> field -> next;
            if (lm_p0_dump_push_node(lm_lmx_message_thread, stack, node, frame -> indent) != 0) {
                dump->failed = 1;
            }
            continue;
        }
        if (frame -> phase == LM_P0_DUMP_TRAILER) {
            trailer = frame -> trailer;
            indent = frame -> indent;
            if (trailer != 0) {
                structure = trailer -> body;
            }
            else {
                structure = 0;
            }
            lm_own_delete(lm_own_ptr_stack_pop(stack), 0);
            if (trailer == 0) {
                continue;
            }
            lm_p0_dump_indent(lm_lmx_message_thread, dump, indent);
            if ((trailer -> flags & LM_P0_TRAILER_TAIL_CUTTER) != 0U) {
                lm_p0_dump_append_cstr(dump, "Tail cutter trailer spelling=");
            }
            else {
                lm_p0_dump_append_cstr(dump, "Trailer spelling=");
            }
            lm_p0_dump_text(lm_lmx_message_thread, dump, trailer -> spelling);
            lm_p0_dump_append_field_count_line(lm_lmx_message_thread, dump, trailer -> body -> field_count);
            if (lm_p0_dump_push_structure(lm_lmx_message_thread, stack, structure, (indent + 1U)) != 0) {
                dump->failed = 1;
            }
            continue;
        }
        dump->failed = 1;
    }
}

static void lm_p0_dump_node(struct LmMessageThread *lm_lmx_message_thread, LmP0Dump *dump, const LmP0Node *node, size_t indent) {
    (void)lm_lmx_message_thread;
    LmOwnPtrStack * stack;
    if (node == 0) {
        return;
    }
    stack = lm_p0_dump_stack_new(lm_lmx_message_thread);
    if (stack == 0) {
        dump->failed = 1;
        return;
    }
    if (lm_p0_dump_push_node(lm_lmx_message_thread, stack, node, indent) != 0) {
        dump->failed = 1;
    }
    else {
        lm_p0_dump_run(lm_lmx_message_thread, dump, stack);
    }
    lm_p0_dump_stack_delete(lm_lmx_message_thread, &stack);
}

static LmP0Dump * lm_p0_dump_new(struct LmMessageThread *lm_lmx_message_thread) {
    (void)lm_lmx_message_thread;
    return (((LmP0Dump *)lm_own_new_zero(1U * sizeof(LmP0Dump))));
}

static char * lm_p0_dump_take_data(struct LmMessageThread *lm_lmx_message_thread, LmP0Dump *dump) {
    (void)lm_lmx_message_thread;
    char *data;
    if (dump == 0) {
        return 0;
    }
    data = dump -> data;
    dump->data = 0;
    dump->length = 0U;
    dump->capacity = 0U;
    return data;
}

static void lm_p0_dump_delete(struct LmMessageThread *lm_lmx_message_thread, LmP0Dump *dump) {
    (void)lm_lmx_message_thread;
    if (dump != 0) {
        lm_own_delete(dump -> data, 0);
        lm_own_delete(dump, 0);
    }
}

char * lm_p0_dump_alloc(struct LmMessageThread *lm_lmx_message_thread, const LmP0Document *document) {
    (void)lm_lmx_message_thread;
    LmP0Dump * dump;
    char *data;
    dump = lm_p0_dump_new(lm_lmx_message_thread);
    if (dump == 0) {
        return 0;
    }
    if ((document == 0) || (document -> root == 0)) {
        lm_p0_dump_append_cstr(dump, "");
        data = lm_p0_dump_take_data(lm_lmx_message_thread, dump);
        lm_p0_dump_delete(lm_lmx_message_thread, dump);
        return data;
    }
    lm_p0_dump_node(lm_lmx_message_thread, dump, document -> root, 0U);
    if (dump -> failed) {
        lm_p0_dump_delete(lm_lmx_message_thread, dump);
        return 0;
    }
    data = lm_p0_dump_take_data(lm_lmx_message_thread, dump);
    lm_p0_dump_delete(lm_lmx_message_thread, dump);
    return data;
}
