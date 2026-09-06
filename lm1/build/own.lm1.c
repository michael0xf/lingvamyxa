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



































































#ifndef lm_lmx_module_private_1_typedef_defined_LmNativeHostThreadEntry
#define lm_lmx_module_private_1_typedef_defined_LmNativeHostThreadEntry 1
#define lm_lmx_module_private_1_typedef_id_a_LmNativeHostThreadEntry 0xea1de1a0bfeae0f8ULL
#define lm_lmx_module_private_1_typedef_id_b_LmNativeHostThreadEntry 0x71afef9044cba2c5ULL
typedef void * (*LmNativeHostThreadEntry)(void *argument);
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmNativeHostThreadEntry) || !defined(lm_lmx_module_private_1_typedef_id_b_LmNativeHostThreadEntry) || lm_lmx_module_private_1_typedef_id_a_LmNativeHostThreadEntry != 0xea1de1a0bfeae0f8ULL || lm_lmx_module_private_1_typedef_id_b_LmNativeHostThreadEntry != 0x71afef9044cba2c5ULL
#error "Lingvamyxa conflicting typedef projection for LmNativeHostThreadEntry"
#endif
#endif

#include <stddef.h>

#define LM_THREAD_PROVIDER_AUTO 0
#define LM_THREAD_PROVIDER_PTHREAD 1
#define LM_THREAD_PROVIDER_WIN32 2
#define LM_THREAD_PROVIDER_SINGLE 3
#define LM_MESSAGE_STATUS_EXIT_DISCARDED (-1)
#define LM_UNUSED (void)


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
#ifndef lm_lmx_module_private_1_typedef_defined_LmNativeHostThreadState
#define lm_lmx_module_private_1_typedef_defined_LmNativeHostThreadState 1
#define lm_lmx_module_private_1_typedef_id_a_LmNativeHostThreadState 0xf3e06584ec3dfce3ULL
#define lm_lmx_module_private_1_typedef_id_b_LmNativeHostThreadState 0x634ba89c3a68a716ULL
typedef struct LmNativeHostThreadState {
    void *handle;
    unsigned thread_id;
    void * (*entry)(void *argument);
    void *argument;
    void *result;
} LmNativeHostThreadState;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmNativeHostThreadState) || !defined(lm_lmx_module_private_1_typedef_id_b_LmNativeHostThreadState) || lm_lmx_module_private_1_typedef_id_a_LmNativeHostThreadState != 0xf3e06584ec3dfce3ULL || lm_lmx_module_private_1_typedef_id_b_LmNativeHostThreadState != 0x634ba89c3a68a716ULL
#error "Lingvamyxa conflicting typedef projection for LmNativeHostThreadState"
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


#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef LM_THREAD_PROVIDER
#define LM_THREAD_PROVIDER LM_THREAD_PROVIDER_SINGLE
#endif

#if LM_THREAD_PROVIDER == LM_THREAD_PROVIDER_PTHREAD || (LM_THREAD_PROVIDER == LM_THREAD_PROVIDER_AUTO && !defined(_WIN32) && (defined(__APPLE__) || defined(__unix__)))
#if defined(_WIN32)
static int lm_native_thread_provider_invalid_pthread_target(void);

static int lm_native_thread_provider_invalid_pthread_target(void) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    return LM_THREAD_PROVIDER_PTHREAD_IS_NOT_SUPPORTED_BY_WINDOWS;
}
#endif

#include <pthread.h>
static void * lm_native_thread_identity_new_current(void);
static int lm_native_thread_identity_is_current(const void *implementation);
static void lm_native_thread_identity_delete(void *implementation);
static const char * lm_native_thread_provider_name(void);
static void * lm_native_host_thread_start(LmNativeHostThreadEntry entry, void *argument, int *status);
static int lm_native_host_thread_join(void *implementation, void **result);
static void * lm_native_mutex_new(void);
static int lm_native_mutex_delete(void *implementation);
static int lm_native_mutex_lock(void *implementation);
static int lm_native_mutex_unlock(void *implementation);
static void * lm_native_condition_new(void);
static int lm_native_condition_delete(void *implementation);
static int lm_native_condition_wait(void *condition, void *mutex);
static int lm_native_condition_signal(void *implementation);
static int lm_native_condition_broadcast(void *implementation);

static void * lm_native_thread_identity_new_current(void) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    pthread_t *identity = calloc(1U, sizeof(identity[0]));
    if (identity != 0) {
        identity[0] = pthread_self();
    }
    return identity;
}

static int lm_native_thread_identity_is_current(const void *implementation) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    const pthread_t *identity = (((const pthread_t *)implementation));
    return identity != 0 && pthread_equal(identity[0], pthread_self()) != 0;
}

static void lm_native_thread_identity_delete(void *implementation) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    free(implementation);
}

static const char * lm_native_thread_provider_name(void) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    return "pthread";
}

static void * lm_native_host_thread_start(LmNativeHostThreadEntry entry, void *argument, int *status) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    pthread_t *thread;
    if (status == 0) {
        return 0;
    }
    status[0] = 1;
    if (entry == 0) {
        return 0;
    }
    thread = calloc(1U, sizeof(thread[0]));
    if (thread == 0) {
        return 0;
    }
    if (pthread_create(thread, 0, entry, argument) != 0) {
        free(thread);
        return 0;
    }
    status[0] = 0;
    return thread;
}

static int lm_native_host_thread_join(void *implementation, void **result) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    pthread_t *thread = (((pthread_t *)implementation));
    if (thread == 0) {
        return 1;
    }
    if (pthread_join(thread[0], result) != 0) {
        return 1;
    }
    free(thread);
    return 0;
}

static void * lm_native_mutex_new(void) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    pthread_mutex_t *mutex = calloc(1U, sizeof(mutex[0]));
    if (mutex == 0) {
        return 0;
    }
    if (pthread_mutex_init(mutex, 0) != 0) {
        free(mutex);
        return 0;
    }
    return mutex;
}

static int lm_native_mutex_delete(void *implementation) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    pthread_mutex_t *mutex = (((pthread_mutex_t *)implementation));
    if (mutex == 0) {
        return 0;
    }
    if (pthread_mutex_destroy(mutex) != 0) {
        return 1;
    }
    free(implementation);
    return 0;
}

static int lm_native_mutex_lock(void *implementation) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    if (implementation == 0) {
        return 1;
    }
    return pthread_mutex_lock(((pthread_mutex_t *)implementation));
}

static int lm_native_mutex_unlock(void *implementation) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    if (implementation == 0) {
        return 1;
    }
    return pthread_mutex_unlock(((pthread_mutex_t *)implementation));
}

static void * lm_native_condition_new(void) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    pthread_cond_t *condition = calloc(1U, sizeof(condition[0]));
    if (condition == 0) {
        return 0;
    }
    if (pthread_cond_init(condition, 0) != 0) {
        free(condition);
        return 0;
    }
    return condition;
}

static int lm_native_condition_delete(void *implementation) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    pthread_cond_t *condition = (((pthread_cond_t *)implementation));
    if (condition == 0) {
        return 0;
    }
    if (pthread_cond_destroy(condition) != 0) {
        return 1;
    }
    free(implementation);
    return 0;
}

static int lm_native_condition_wait(void *condition, void *mutex) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    if (condition == 0 || mutex == 0) {
        return 1;
    }
    return pthread_cond_wait((((pthread_cond_t *)condition)), (((pthread_mutex_t *)mutex)));
}

static int lm_native_condition_signal(void *implementation) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    if (implementation == 0) {
        return 1;
    }
    return pthread_cond_signal(((pthread_cond_t *)implementation));
}

static int lm_native_condition_broadcast(void *implementation) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    if (implementation == 0) {
        return 1;
    }
    return pthread_cond_broadcast(((pthread_cond_t *)implementation));
}
#elif LM_THREAD_PROVIDER == LM_THREAD_PROVIDER_WIN32 || (LM_THREAD_PROVIDER == LM_THREAD_PROVIDER_AUTO && defined(_WIN32))
#if !defined(_WIN32)
static int lm_native_thread_provider_invalid_win32_target(void);

static int lm_native_thread_provider_invalid_win32_target(void) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    return LM_THREAD_PROVIDER_WIN32_REQUIRES_A_WINDOWS_TARGET;
}
#endif

#include <process.h>
#include <windows.h>
static void * lm_native_thread_identity_new_current(void);
static int lm_native_thread_identity_is_current(const void *implementation);
static void lm_native_thread_identity_delete(void *implementation);
#ifndef LM_LMX_ABI_STDCALL
#if defined(_WIN32)
#define LM_LMX_ABI_STDCALL __stdcall
#else
#define LM_LMX_ABI_STDCALL
#endif
#endif
static unsigned LM_LMX_ABI_STDCALL lm_native_host_thread_entry(void *argument);
static const char * lm_native_thread_provider_name(void);
static void * lm_native_host_thread_start(LmNativeHostThreadEntry entry, void *argument, int *status);
static int lm_native_host_thread_join(void *implementation, void **result);
static void * lm_native_mutex_new(void);
static int lm_native_mutex_delete(void *implementation);
static int lm_native_mutex_lock(void *implementation);
static int lm_native_mutex_unlock(void *implementation);
static void * lm_native_condition_new(void);
static int lm_native_condition_delete(void *implementation);
static int lm_native_condition_wait(void *condition, void *mutex);
static int lm_native_condition_signal(void *implementation);
static int lm_native_condition_broadcast(void *implementation);

static void * lm_native_thread_identity_new_current(void) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    unsigned *identity = calloc(1U, sizeof(identity[0]));
    if (identity != 0) {
        identity[0] = (((unsigned)GetCurrentThreadId()));
    }
    return identity;
}

static int lm_native_thread_identity_is_current(const void *implementation) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    const unsigned *identity = (((const unsigned *)implementation));
    return identity != 0 && identity[0] == (((unsigned)GetCurrentThreadId()));
}

static void lm_native_thread_identity_delete(void *implementation) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    free(implementation);
}

static unsigned LM_LMX_ABI_STDCALL lm_native_host_thread_entry(void *argument) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    LmNativeHostThreadState * thread = (((LmNativeHostThreadState *)argument));
    thread->result = (thread->entry)(thread -> argument);
    return 0U;
}

static const char * lm_native_thread_provider_name(void) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    return "win32";
}

static void * lm_native_host_thread_start(LmNativeHostThreadEntry entry, void *argument, int *status) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    LmNativeHostThreadState * thread;
    uintptr_t handle;
    if (status == 0) {
        return 0;
    }
    status[0] = 1;
    if (entry == 0) {
        return 0;
    }
    thread = calloc(1U, sizeof(thread[0]));
    if (thread == 0) {
        return 0;
    }
    thread->entry = entry;
    thread->argument = argument;
    handle = (((uintptr_t)_beginthreadex(0, 0U, &lm_native_host_thread_entry, thread, 0U, &thread -> thread_id)));
    if (handle == 0U) {
        free(thread);
        return 0;
    }
    thread->handle = (((void *)handle));
    status[0] = 0;
    return thread;
}

static int lm_native_host_thread_join(void *implementation, void **result) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    LmNativeHostThreadState * thread = (((LmNativeHostThreadState *)implementation));
    if (thread == 0) {
        return 1;
    }
    if (WaitForSingleObject(thread -> handle, INFINITE) != WAIT_OBJECT_0) {
        return 1;
    }
    if (result != 0) {
        result[0] = thread -> result;
    }
    CloseHandle(thread -> handle);
    free(thread);
    return 0;
}

static void * lm_native_mutex_new(void) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    void *mutex = calloc(1U, sizeof(CRITICAL_SECTION));
    if (mutex == 0) {
        return 0;
    }
    InitializeCriticalSection(mutex);
    return mutex;
}

static int lm_native_mutex_delete(void *implementation) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    if (implementation == 0) {
        return 0;
    }
    DeleteCriticalSection(implementation);
    free(implementation);
    return 0;
}

static int lm_native_mutex_lock(void *implementation) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    if (implementation == 0) {
        return 1;
    }
    EnterCriticalSection(implementation);
    return 0;
}

static int lm_native_mutex_unlock(void *implementation) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    if (implementation == 0) {
        return 1;
    }
    LeaveCriticalSection(implementation);
    return 0;
}

static void * lm_native_condition_new(void) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    void *condition = calloc(1U, sizeof(CONDITION_VARIABLE));
    if (condition == 0) {
        return 0;
    }
    InitializeConditionVariable(condition);
    return condition;
}

static int lm_native_condition_delete(void *implementation) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    free(implementation);
    return 0;
}

static int lm_native_condition_wait(void *condition, void *mutex) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    if (condition == 0 || mutex == 0) {
        return 1;
    }
    if (SleepConditionVariableCS(condition, mutex, INFINITE)) {
        return 0;
    }
    return 1;
}

static int lm_native_condition_signal(void *implementation) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    if (implementation == 0) {
        return 1;
    }
    WakeConditionVariable(implementation);
    return 0;
}

static int lm_native_condition_broadcast(void *implementation) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    if (implementation == 0) {
        return 1;
    }
    WakeAllConditionVariable(implementation);
    return 0;
}
#elif LM_THREAD_PROVIDER == LM_THREAD_PROVIDER_SINGLE || (LM_THREAD_PROVIDER == LM_THREAD_PROVIDER_AUTO && !defined(_WIN32) && !defined(__APPLE__) && !defined(__unix__))
static void * lm_native_thread_identity_new_current(void);
static int lm_native_thread_identity_is_current(const void *implementation);
static void lm_native_thread_identity_delete(void *implementation);
static const char * lm_native_thread_provider_name(void);
static void * lm_native_host_thread_start(LmNativeHostThreadEntry entry, void *argument, int *status);
static int lm_native_host_thread_join(void *implementation, void **result);
static void * lm_native_mutex_new(void);
static int lm_native_mutex_delete(void *implementation);
static int lm_native_mutex_lock(void *implementation);
static int lm_native_mutex_unlock(void *implementation);
static void * lm_native_condition_new(void);
static int lm_native_condition_delete(void *implementation);
static int lm_native_condition_wait(void *condition, void *mutex);
static int lm_native_condition_signal(void *implementation);
static int lm_native_condition_broadcast(void *implementation);

static void * lm_native_thread_identity_new_current(void) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    return calloc(1U, 1U);
}

static int lm_native_thread_identity_is_current(const void *implementation) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    return implementation != 0;
}

static void lm_native_thread_identity_delete(void *implementation) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    free(implementation);
}

static const char * lm_native_thread_provider_name(void) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    return "single";
}

static void * lm_native_host_thread_start(LmNativeHostThreadEntry entry, void *argument, int *status) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    LmNativeHostThreadState * thread;
    if (status == 0) {
        return 0;
    }
    status[0] = 1;
    if (entry == 0) {
        return 0;
    }
    thread = calloc(1U, sizeof(thread[0]));
    if (thread == 0) {
        return 0;
    }
    thread->result = entry(argument);
    status[0] = 0;
    return thread;
}

static int lm_native_host_thread_join(void *implementation, void **result) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    LmNativeHostThreadState * thread = (((LmNativeHostThreadState *)implementation));
    if (thread == 0) {
        return 1;
    }
    if (result != 0) {
        result[0] = thread -> result;
    }
    free(thread);
    return 0;
}

static void * lm_native_mutex_new(void) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    return calloc(1U, 1U);
}

static int lm_native_mutex_delete(void *implementation) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    free(implementation);
    return 0;
}

static int lm_native_mutex_lock(void *implementation) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    return implementation == 0;
}

static int lm_native_mutex_unlock(void *implementation) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    return implementation == 0;
}

static void * lm_native_condition_new(void) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    return calloc(1U, 1U);
}

static int lm_native_condition_delete(void *implementation) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    free(implementation);
    return 0;
}

static int lm_native_condition_wait(void *condition, void *mutex) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    LM_UNUSED(condition);
    LM_UNUSED(mutex);
    return 1;
}

static int lm_native_condition_signal(void *implementation) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    return implementation == 0;
}

static int lm_native_condition_broadcast(void *implementation) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    return implementation == 0;
}
#else
static int lm_native_thread_provider_invalid_value(void);

static int lm_native_thread_provider_invalid_value(void) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    return LM_THREAD_PROVIDER_VALUE_IS_UNKNOWN;
}
#endif

#include <stdlib.h>
#include <string.h>
static int lm_native_message_thread_state_lock(const LmMessageThread *thread);
static int lm_native_message_thread_state_unlock(const LmMessageThread *thread);
static int lm_native_message_thread_is_current(const LmMessageThread *thread);
static char * lm_native_message_thread_copy_cstr(const char *source);
static int lm_native_message_thread_ascii_equal_ignore_case(unsigned char left, unsigned char right);
static int lm_native_message_thread_ascii_prefix_ignore_case(const char *text, const char *prefix, size_t length);
static int lm_native_message_thread_ascii_is_hex(unsigned char value);
static int lm_native_message_thread_ascii_is_unreserved(unsigned char value);
static int lm_native_message_thread_ascii_is_sub_delim(unsigned char value);
static int lm_native_message_thread_percent_encoded_is_valid(const unsigned char *text, size_t length, size_t *index);
static int lm_native_message_thread_path_is_valid(const char *path, int require_leading_slash);
static int lm_native_message_thread_ipv4_is_valid(const unsigned char *text, size_t length);
static int lm_native_message_thread_ipv6_is_valid(const unsigned char *text, size_t length);
static int lm_native_message_thread_reg_name_is_valid(const unsigned char *text, size_t length);
static int lm_native_message_thread_port_is_valid(const unsigned char *text, size_t length);
static int lm_native_message_thread_route_is_valid(const char *route);
static int lm_native_message_thread_endpoint_is_http(const char *endpoint);
static char * lm_native_message_thread_join_uri(const char *endpoint, const char *route);
static LmMessage * lm_native_message_thread_message_new(const char *lmx, size_t length);
static void lm_native_message_thread_message_delete(LmMessage *message);
static void lm_native_message_thread_outbox_entry_delete(LmMessageOutboxEntry *entry);
static void lm_native_message_thread_mailboxes_destroy(LmMessageThread *thread);
static void lm_native_message_thread_ready_push_locked(LmMessageThreadPool *pool, LmMessageThread *thread);
static LmMessageThread * lm_native_message_thread_ready_pop_locked(LmMessageThreadPool *pool);
static void lm_native_message_thread_schedule_stop_locked(LmMessageThreadPool *pool, LmMessageThread *thread);
static int lm_native_message_thread_pool_is_controller(const LmMessageThreadPool *pool);
static int lm_native_message_thread_pool_lifecycle_allowed(const LmMessageThreadPool *pool);
static int lm_native_message_thread_control_allowed_locked(const LmMessageThreadPool *pool, const LmMessageThread *thread);
static int lm_native_message_thread_invoke(LmMessageThread *thread, LmMessageThreadEntry entry, void *argument);
static void lm_native_message_thread_pool_stop_locked(LmMessageThreadPool *pool);
static int lm_native_message_thread_pool_is_quiescent_locked(LmMessageThreadPool *pool);
static void lm_native_message_thread_pool_maybe_finish_drain_locked(LmMessageThreadPool *pool);
static int lm_native_message_thread_pool_begin_turn(LmMessageThread *thread);
static int lm_native_message_thread_pool_collect(LmMessageThread *thread);
static int lm_native_message_thread_pool_end_turn(LmMessageThread *thread);
static int lm_native_message_thread_mark_exit_ready(LmMessageThread *thread);
static int lm_native_message_thread_deliver_remote(LmMessageThreadRuntime *runtime, const char *endpoint, const char *route, const LmMessage *message);
static int lm_native_message_thread_deliver_local(LmMessageThreadRuntime *runtime, const char *route, LmMessage *message, int reject_exit_requested);
static LmMessageOutboxEntry * lm_native_message_thread_outbox_take(LmMessageThread *thread);
static int lm_native_message_thread_outbox_finish(LmMessageThread *thread, int commit);
static void lm_native_message_thread_current_message_finish(LmMessageThread *thread);
static void lm_native_message_thread_execute(LmMessageThreadPool *pool, LmMessageThread *thread);
static void * lm_native_message_thread_worker(void *argument);
static int lm_native_message_thread_pool_pump_one(LmMessageThreadPool *pool);
size_t lm_message_thread_pool_pump(LmMessageThreadPool *pool, size_t max_turns);
int lm_message_thread_runtime_set_rest_lmx_provider(LmMessageThreadRuntime *runtime, const LmRestLmxProviderOpsV1 *ops, void *context);
int lm_message_thread_runtime_admit_lmx(LmMessageThreadRuntime *runtime, const char *route, const char *lmx, size_t length);
int lm_message_thread_request_exit(LmMessageThread *requester, int status);
int lm_message_thread_runtime_exit_state(LmMessageThreadRuntime *runtime, int *out_requested, int *out_ready, int *out_status);
int lm_message_thread_runtime_attach_root(LmMessageThreadRuntime *runtime, LmMessageThread *thread);
int lm_message_thread_runtime_detach_root(LmMessageThreadRuntime *runtime, LmMessageThread *thread);
int lm_message_thread_runtime_delete(LmMessageThreadRuntime *runtime);
static int lm_native_message_thread_runtime_acquire_pool(LmMessageThreadRuntime *runtime);
static void lm_native_message_thread_runtime_release_pool(LmMessageThreadRuntime *runtime);
LmMessageThreadPool * lm_message_thread_pool_new(LmMessageThreadRuntime *runtime, size_t worker_count);
void lm_message_thread_pool_request_stop(LmMessageThreadPool *pool);
void lm_message_thread_pool_request_stop_when_idle(LmMessageThreadPool *pool);
LmMessageThreadRuntime * lm_message_thread_runtime_new(void);
LmMessageThread * lm_message_thread_new_in(LmMessageThreadPool *pool);
static int lm_native_message_thread_start(LmMessageThread *thread, LmMessageThreadEntry entry, void *argument, int mailbox_mode);
int lm_message_thread_start(LmMessageThread *thread, LmMessageThreadEntry entry, void *argument);
int lm_message_thread_start_mailbox(LmMessageThread *thread, LmMessageThreadEntry entry, void *argument);
int lm_message_thread_bind_route(LmMessageThread *thread, const char *route);
int lm_message_thread_send_lmx(LmMessageThread *sender, const char *endpoint, const char *route, const char *lmx, size_t length);
int lm_message_thread_current_lmx(LmMessageThread *thread, const char **out_lmx, size_t *out_length);
size_t lm_message_thread_inbox_count(const LmMessageThread *thread);
size_t lm_message_thread_outbox_count(const LmMessageThread *thread);
int lm_message_thread_join(LmMessageThread *thread, int *result);
static void lm_native_message_thread_remove_pool_routes_locked(LmMessageThreadRuntime *runtime, LmMessageThreadPool *pool);
int lm_message_thread_pool_delete(LmMessageThreadPool *pool);
void * lm_own_new_zero(size_t size);
void * lm_own_resize(void *object, size_t size);
char * lm_own_copy_bytes(const char *source, size_t length);
void lm_own_delete(void *object, LmOwnDestroyFields destroy_fields);
void lm_own_delete_plain(void *object);
void lm_own_pointer_array_delete(void **items, size_t count, LmOwnDelete delete_item);
void lm_own_ptr_stack_init(LmOwnPtrStack *stack, LmOwnDelete delete_item);
void lm_own_ptr_stack_destroy(LmOwnPtrStack *stack);
int lm_own_ptr_stack_push(LmOwnPtrStack *stack, void *item);
void * lm_own_ptr_stack_pop(LmOwnPtrStack *stack);
void * lm_own_ptr_stack_at(const LmOwnPtrStack *stack, size_t index);
void * lm_own_ptr_stack_top(const LmOwnPtrStack *stack);
void lm_own_ptr_stack_truncate(LmOwnPtrStack *stack, size_t count);
void lm_own_value_stack_init(LmOwnValueStack *stack, size_t item_size);
void lm_own_value_stack_destroy(LmOwnValueStack *stack);
void lm_own_value_stack_truncate(LmOwnValueStack *stack, size_t count);
static LmMessageThreadComponent * lm_message_thread_component_find(struct LmMessageThread *lm_lmx_message_thread, LmMessageThread *owner_thread, LmMessageThreadComponentDestroy destroy, int *out_valid);
static int lm_message_thread_component_detach(struct LmMessageThread *lm_lmx_message_thread, LmMessageThread *owner_thread, LmMessageThreadComponent *component);
static int lm_message_thread_component_release(struct LmMessageThread *lm_lmx_message_thread, LmMessageThread *owner_thread, LmMessageThreadComponent *component);
int lm_message_thread_component_attach(struct LmMessageThread *lm_lmx_message_thread, LmMessageThreadComponentDestroy destroy, void *component);
void * lm_message_thread_component_get(struct LmMessageThread *lm_lmx_message_thread, LmMessageThreadComponentDestroy destroy);
int lm_message_thread_component_remove(struct LmMessageThread *lm_lmx_message_thread, LmMessageThreadComponentDestroy destroy);
size_t lm_message_thread_component_count(const LmMessageThread *thread);
static int lm_own_arena_shell_is_zero(struct LmMessageThread *lm_lmx_message_thread, const LmOwnArena *arena);
static int lm_own_arena_register(struct LmMessageThread *lm_lmx_message_thread, LmMessageThread *owner_thread, LmOwnArena *arena);
static int lm_own_arena_detach(struct LmMessageThread *lm_lmx_message_thread, LmMessageThread *owner_thread, LmOwnArena *arena);
static int lm_own_arena_init_registered(struct LmMessageThread *lm_lmx_message_thread, LmOwnArena *arena, LmMessageThread *owner_thread, int runtime_owned_shell);
static int lm_own_arena_release(struct LmMessageThread *lm_lmx_message_thread, LmMessageThread *owner_thread, LmOwnArena *arena, int delete_shell, int teardown_mode);
int lm_own_arena_init(struct LmMessageThread *lm_lmx_message_thread, LmOwnArena *arena, LmMessageThread *owner_thread);
void lm_own_arena_destroy(struct LmMessageThread *lm_lmx_message_thread, LmOwnArena *arena);
LmOwnArena * lm_own_arena_new(struct LmMessageThread *lm_lmx_message_thread, LmMessageThread *owner_thread);
void lm_own_arena_delete(struct LmMessageThread *lm_lmx_message_thread, LmOwnArena *arena);
void lm_own_arena_freeze(struct LmMessageThread *lm_lmx_message_thread, LmOwnArena *arena);
int lm_own_arena_is_frozen(struct LmMessageThread *lm_lmx_message_thread, const LmOwnArena *arena);
LmMessageThread * lm_own_arena_owner_thread(struct LmMessageThread *lm_lmx_message_thread, const LmOwnArena *arena);
int lm_own_tree_cut(struct LmMessageThread *lm_lmx_message_thread, LmOwnArena *arena);
int lm_own_tree_cut_promote_lazy_edges(struct LmMessageThread *lm_lmx_message_thread, LmOwnArena *arena);
const char * lm_thread_provider_name(void);
LmHostThread * lm_host_thread_new(void);
void lm_host_thread_delete(LmHostThread *thread);
int lm_host_thread_start(LmHostThread *thread, LmHostThreadEntry entry, void *argument);
int lm_host_thread_join(LmHostThread *thread, void **result);
LmMutex * lm_mutex_new(void);
void lm_mutex_delete(LmMutex *mutex);
int lm_mutex_lock(LmMutex *mutex);
int lm_mutex_unlock(LmMutex *mutex);
LmCondition * lm_condition_new(void);
void lm_condition_delete(LmCondition *condition);
int lm_condition_wait(LmCondition *condition, LmMutex *mutex);
int lm_condition_signal(LmCondition *condition);
int lm_condition_broadcast(LmCondition *condition);
int lm_own_value_stack_push(LmOwnValueStack *stack, const void *item);
int lm_own_value_stack_resize_zero(LmOwnValueStack *stack, size_t count);
int lm_own_value_stack_pop(LmOwnValueStack *stack, void *out_item);
void * lm_own_value_stack_at(const LmOwnValueStack *stack, size_t index);
void * lm_own_value_stack_top(const LmOwnValueStack *stack);
static int lm_own_size_multiply(struct LmMessageThread *lm_lmx_message_thread, size_t left, size_t right, size_t *out);
static const LmOwnAllocationDescriptor * lm_own_allocation_descriptor_find(struct LmMessageThread *lm_lmx_message_thread, const LmOwnPtrStack *descriptors, const void *address);
static int lm_own_allocation_descriptor_push(struct LmMessageThread *lm_lmx_message_thread, LmOwnPtrStack *descriptors, void *address, LmOwnArena *owner, size_t bytes, size_t element_size, size_t count, size_t rank, size_t level);
void * lm_own_arena_new_zero(struct LmMessageThread *lm_lmx_message_thread, LmOwnArena *arena, size_t size);
void * lm_own_arena_array_new_zero(struct LmMessageThread *lm_lmx_message_thread, LmOwnArena *arena, size_t element_size, size_t count, size_t rank, size_t level);
const LmOwnAllocationDescriptor * lm_own_arena_allocation_descriptor(struct LmMessageThread *lm_lmx_message_thread, const LmOwnArena *arena, const void *address);
char * lm_own_arena_copy_bytes(struct LmMessageThread *lm_lmx_message_thread, LmOwnArena *arena, const char *source, size_t length);
int lm_own_arena_add_lazy_edge(struct LmMessageThread *lm_lmx_message_thread, LmOwnArena *target, LmOwnArena *source, const void *source_ptr, size_t size, const void **patch_slot);
int lm_own_arena_promote_lazy_edges(struct LmMessageThread *lm_lmx_message_thread, LmOwnArena *arena);
static void * lm_own_copy_visit_find(struct LmMessageThread *lm_lmx_message_thread, const LmOwnPtrStack *sources, const LmOwnPtrStack *copies, void *source);
static void * lm_own_arena_copy_graph_into(struct LmMessageThread *lm_lmx_message_thread, LmOwnArena *arena, void *source, LmOwnPtrStack *sources, LmOwnPtrStack *copies);
static void lm_own_arena_mark(struct LmMessageThread *lm_lmx_message_thread, LmOwnArena *arena, void *object);
int lm_own_arena_pin(struct LmMessageThread *lm_lmx_message_thread, LmOwnArena *arena, void *address);
int lm_own_arena_root_add(struct LmMessageThread *lm_lmx_message_thread, LmOwnArena *arena, void *address);
void * lm_own_arena_copy_graph(struct LmMessageThread *lm_lmx_message_thread, LmOwnArena *arena, const void *source);
int lm_own_arena_reclaim(struct LmMessageThread *lm_lmx_message_thread, LmOwnArena *arena);
int lm_own_arena_absorb(struct LmMessageThread *lm_lmx_message_thread, LmOwnArena *target, LmOwnArena *source);
int lm_message_thread_init(LmMessageThread *thread);
void lm_message_thread_destroy(LmMessageThread *thread);
LmMessageThread * lm_message_thread_new(void);
void lm_message_thread_delete(LmMessageThread *thread);
int lm_message_thread_is_running(const LmMessageThread *thread);
int lm_message_thread_status(const LmMessageThread *thread);
LmOwnArena * lm_message_thread_owner(LmMessageThread *thread);
void * lm_message_thread_execution_context(LmMessageThread *thread);
void * lm_message_thread_set_execution_context(LmMessageThread *thread, void *context);
size_t lm_message_thread_turn_count(const LmMessageThread *thread);
size_t lm_message_thread_collection_count(const LmMessageThread *thread);
size_t lm_message_thread_arena_count(const LmMessageThread *thread);
void lm_message_thread_request_stop(LmMessageThread *thread, int status);
void lm_message_thread_request_failure(LmMessageThread *thread, int status);
int lm_message_thread_collect(LmMessageThread *thread);
int lm_message_thread_begin_turn(LmMessageThread *thread);
int lm_message_thread_end_turn(LmMessageThread *thread);











static int lm_native_message_thread_state_lock(const LmMessageThread *thread) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    if (thread == 0) {
        return 1;
    }
    if (thread -> state_mutex == 0) {
        return 0;
    }
    return lm_mutex_lock(thread -> state_mutex);
}

static int lm_native_message_thread_state_unlock(const LmMessageThread *thread) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    if (thread == 0) {
        return 1;
    }
    if (thread -> state_mutex == 0) {
        return 0;
    }
    return lm_mutex_unlock(thread -> state_mutex);
}

static int lm_native_message_thread_is_current(const LmMessageThread *thread) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    const LmMessageThreadPool * pool;
    if (thread == 0) {
        return 0;
    }
    if (thread -> pool == 0) {
        return thread -> runtime != 0 && thread -> runtime -> root_thread == thread && thread -> turn_active && lm_native_thread_identity_is_current(thread -> runtime -> identity);
    }
    if (! thread -> executing) {
        return 0;
    }
    pool = thread -> pool;
    if (pool -> single_mode) {
        return pool -> runtime != 0 && pool -> runtime -> single_execution_depth != 0U && pool -> runtime -> single_active_thread == thread;
    }
    return lm_native_thread_identity_is_current(thread -> execution_identity);
}

static char * lm_native_message_thread_copy_cstr(const char *source) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    size_t length;
    char *copy;
    if (source == 0) {
        return 0;
    }
    length = strlen(source);
    if (length == (((size_t)-1))) {
        return 0;
    }
    copy = malloc(length + 1U);
    if (copy != 0) {
        memcpy(copy, source, length + 1U);
    }
    return copy;
}

static int lm_native_message_thread_ascii_equal_ignore_case(unsigned char left, unsigned char right) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    if (left >= (((unsigned char)'A')) && left <= (((unsigned char)'Z'))) {
        left = (((unsigned char)(left + ((((unsigned char)'a')) - (((unsigned char)'A'))))));
    }
    if (right >= (((unsigned char)'A')) && right <= (((unsigned char)'Z'))) {
        right = (((unsigned char)(right + ((((unsigned char)'a')) - (((unsigned char)'A'))))));
    }
    return left == right;
}

static int lm_native_message_thread_ascii_prefix_ignore_case(const char *text, const char *prefix, size_t length) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    size_t index = 0U;
    if (text == 0 || prefix == 0) {
        return 0;
    }
    while (index < length) {
        if (text[index] == '\0' || lm_native_message_thread_ascii_equal_ignore_case((((unsigned char)text[index])), (((unsigned char)prefix[index]))) == 0) {
            return 0;
        }
        index = index + 1U;
    }
    return 1;
}

static int lm_native_message_thread_ascii_is_hex(unsigned char value) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    return (value >= (((unsigned char)'0')) && value <= (((unsigned char)'9'))) || (value >= (((unsigned char)'a')) && value <= (((unsigned char)'f'))) || (value >= (((unsigned char)'A')) && value <= (((unsigned char)'F')));
}

static int lm_native_message_thread_ascii_is_unreserved(unsigned char value) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    return (value >= (((unsigned char)'a')) && value <= (((unsigned char)'z'))) || (value >= (((unsigned char)'A')) && value <= (((unsigned char)'Z'))) || (value >= (((unsigned char)'0')) && value <= (((unsigned char)'9'))) || value == (((unsigned char)'-')) || value == (((unsigned char)'.')) || value == (((unsigned char)'_')) || value == (((unsigned char)'~'));
}

static int lm_native_message_thread_ascii_is_sub_delim(unsigned char value) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    return value == (((unsigned char)'!')) || value == (((unsigned char)'$')) || value == (((unsigned char)'&')) || value == (((unsigned char)'\'')) || value == (((unsigned char)'(')) || value == (((unsigned char)')')) || value == (((unsigned char)'*')) || value == (((unsigned char)'+')) || value == (((unsigned char)',')) || value == (((unsigned char)';')) || value == (((unsigned char)'='));
}

static int lm_native_message_thread_percent_encoded_is_valid(const unsigned char *text, size_t length, size_t *index) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    size_t current = index[0];
    if (current + 2U >= length || lm_native_message_thread_ascii_is_hex(text[current + 1U]) == 0 || lm_native_message_thread_ascii_is_hex(text[current + 2U]) == 0) {
        return 0;
    }
    index[0] = current + 2U;
    return 1;
}

static int lm_native_message_thread_path_is_valid(const char *path, int require_leading_slash) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    const unsigned char *text = (((const unsigned char *)path));
    size_t length;
    size_t index = 0U;
    size_t segment_start = 0U;
    if (text == 0) {
        return 0;
    }
    length = strlen(path);
    if ((require_leading_slash && (length == 0U || text[0] != (((unsigned char)'/')))) || (! require_leading_slash && length != 0U && text[0] != (((unsigned char)'/')))) {
        return 0;
    }
    while (index < length) {
        unsigned char value = text[index];
        if (value <= 0x20U || value > 0x7eU || value == (((unsigned char)'\\')) || value == (((unsigned char)'?')) || value == (((unsigned char)'#'))) {
            return 0;
        }
        if (value == (((unsigned char)'/'))) {
            size_t segment_length = index - segment_start;
            if (index != 0U && text[index - 1U] == (((unsigned char)'/'))) {
                return 0;
            }
            if ((segment_length == 1U && text[segment_start] == (((unsigned char)'.'))) || (segment_length == 2U && text[segment_start] == (((unsigned char)'.')) && text[segment_start + 1U] == (((unsigned char)'.')))) {
                return 0;
            }
            segment_start = index + 1U;
        }
        else {
            if (value == (((unsigned char)'%'))) {
                if (lm_native_message_thread_percent_encoded_is_valid(text, length, &index) == 0) {
                    return 0;
                }
            }
            else {
                if (lm_native_message_thread_ascii_is_unreserved(value) == 0 && lm_native_message_thread_ascii_is_sub_delim(value) == 0 && value != (((unsigned char)':')) && value != (((unsigned char)'@'))) {
                    return 0;
                }
            }
        }
        index = index + 1U;
    }
    if (segment_start < length) {
        size_t segment_length = length - segment_start;
        if ((segment_length == 1U && text[segment_start] == (((unsigned char)'.'))) || (segment_length == 2U && text[segment_start] == (((unsigned char)'.')) && text[segment_start + 1U] == (((unsigned char)'.')))) {
            return 0;
        }
    }
    return 1;
}

static int lm_native_message_thread_ipv4_is_valid(const unsigned char *text, size_t length) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    size_t index = 0U;
    size_t component = 0U;
    while (index < length && component < 4U) {
        size_t digits = 0U;
        unsigned value = 0U;
        while (index < length && text[index] != (((unsigned char)'.'))) {
            if (text[index] < (((unsigned char)'0')) || text[index] > (((unsigned char)'9')) || digits == 3U) {
                return 0;
            }
            value = value * 10U + (((unsigned)(text[index] - (((unsigned char)'0')))));
            digits = digits + 1U;
            index = index + 1U;
        }
        if (digits == 0U || value > 255U || (digits > 1U && text[index - digits] == (((unsigned char)'0')))) {
            return 0;
        }
        component = component + 1U;
        if (index < length) {
            index = index + 1U;
            if (index == length) {
                return 0;
            }
        }
    }
    return index == length && component == 4U;
}

static int lm_native_message_thread_ipv6_is_valid(const unsigned char *text, size_t length) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    size_t index = 0U;
    size_t groups = 0U;
    int compressed = 0;
    if (length == 0U) {
        return 0;
    }
    if (text[0] == (((unsigned char)'v')) || text[0] == (((unsigned char)'V'))) {
        index = 1U;
        while (index < length && lm_native_message_thread_ascii_is_hex(text[index])) {
            index = index + 1U;
        }
        if (index == 1U || index >= length || text[index] != (((unsigned char)'.'))) {
            return 0;
        }
        index = index + 1U;
        if (index == length) {
            return 0;
        }
        while (index < length) {
            if (lm_native_message_thread_ascii_is_unreserved(text[index]) == 0 && lm_native_message_thread_ascii_is_sub_delim(text[index]) == 0 && text[index] != (((unsigned char)':'))) {
                return 0;
            }
            index = index + 1U;
        }
        return 1;
    }
    if (text[0] == (((unsigned char)':'))) {
        if (length < 2U || text[1] != (((unsigned char)':'))) {
            return 0;
        }
        compressed = 1;
        index = 2U;
        if (index == length) {
            return 1;
        }
    }
    while (index < length) {
        size_t segment_start = index;
        size_t segment_length;
        size_t scan;
        int contains_dot = 0;
        while (index < length && text[index] != (((unsigned char)':'))) {
            if (text[index] == (((unsigned char)'.'))) {
                contains_dot = 1;
            }
            index = index + 1U;
        }
        segment_length = index - segment_start;
        if (segment_length == 0U) {
            return 0;
        }
        if (contains_dot) {
            if (index != length || groups > 6U || lm_native_message_thread_ipv4_is_valid(text + segment_start, segment_length) == 0) {
                return 0;
            }
            groups = groups + 2U;
        }
        else {
            if (segment_length > 4U) {
                return 0;
            }
            scan = segment_start;
            while (scan < index) {
                if (lm_native_message_thread_ascii_is_hex(text[scan]) == 0) {
                    return 0;
                }
                scan = scan + 1U;
            }
            groups = groups + 1U;
        }
        if (groups > 8U || index == length) {
            break;
        }
        if (index + 1U < length && text[index + 1U] == (((unsigned char)':'))) {
            if (compressed) {
                return 0;
            }
            compressed = 1;
            index = index + 2U;
            if (index == length) {
                break;
            }
        }
        else {
            index = index + 1U;
            if (index == length) {
                return 0;
            }
        }
    }
    if (compressed) {
        return groups < 8U;
    }
    return groups == 8U;
}

static int lm_native_message_thread_reg_name_is_valid(const unsigned char *text, size_t length) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    size_t index = 0U;
    if (length == 0U) {
        return 0;
    }
    while (index < length) {
        unsigned char value = text[index];
        if (value == (((unsigned char)'%'))) {
            if (lm_native_message_thread_percent_encoded_is_valid(text, length, &index) == 0) {
                return 0;
            }
        }
        else {
            if (lm_native_message_thread_ascii_is_unreserved(value) == 0 && lm_native_message_thread_ascii_is_sub_delim(value) == 0) {
                return 0;
            }
        }
        index = index + 1U;
    }
    return 1;
}

static int lm_native_message_thread_port_is_valid(const unsigned char *text, size_t length) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    size_t index = 0U;
    unsigned value = 0U;
    if (length == 0U) {
        return 0;
    }
    while (index < length) {
        if (text[index] < (((unsigned char)'0')) || text[index] > (((unsigned char)'9'))) {
            return 0;
        }
        value = value * 10U + (((unsigned)(text[index] - (((unsigned char)'0')))));
        if (value > 65535U) {
            return 0;
        }
        index = index + 1U;
    }
    return value != 0U;
}

static int lm_native_message_thread_route_is_valid(const char *route) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    return lm_native_message_thread_path_is_valid(route, 1);
}

static int lm_native_message_thread_endpoint_is_http(const char *endpoint) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    const unsigned char *text = (((const unsigned char *)endpoint));
    const unsigned char *authority;
    const unsigned char *path;
    const unsigned char *authority_end;
    const unsigned char *host_end;
    const unsigned char *port = 0;
    size_t authority_length;
    size_t host_length;
    size_t port_length = 0U;
    size_t index;
    if (endpoint == 0 || endpoint[0] == '\0') {
        return 0;
    }
    if (lm_native_message_thread_ascii_prefix_ignore_case(endpoint, "http://", 7U)) {
        authority = text + 7U;
    }
    else {
        if (lm_native_message_thread_ascii_prefix_ignore_case(endpoint, "https://", 8U)) {
            authority = text + 8U;
        }
        else {
            return 0;
        }
    }
    path = authority;
    while (path[0] != 0U && path[0] != (((unsigned char)'/'))) {
        if (path[0] <= 0x20U || path[0] > 0x7eU || path[0] == (((unsigned char)'\\')) || path[0] == (((unsigned char)'?')) || path[0] == (((unsigned char)'#')) || path[0] == (((unsigned char)'@'))) {
            return 0;
        }
        path = path + 1;
    }
    authority_end = path;
    authority_length = (((size_t)(authority_end - authority)));
    if (authority_length == 0U) {
        return 0;
    }
    if (authority[0] == (((unsigned char)'['))) {
        host_end = authority + 1U;
        while (host_end < authority_end && host_end[0] != (((unsigned char)']'))) {
            host_end = host_end + 1;
        }
        if (host_end == authority_end || host_end == authority + 1U || lm_native_message_thread_ipv6_is_valid(authority + 1U, (((size_t)(host_end - authority - 1U)))) == 0) {
            return 0;
        }
        host_end = host_end + 1U;
        if (host_end < authority_end) {
            if (host_end[0] != (((unsigned char)':'))) {
                return 0;
            }
            port = host_end + 1U;
            port_length = (((size_t)(authority_end - port)));
        }
    }
    else {
        host_end = authority_end;
        index = 0U;
        while (index < authority_length) {
            if (authority[index] == (((unsigned char)'[')) || authority[index] == (((unsigned char)']'))) {
                return 0;
            }
            if (authority[index] == (((unsigned char)':'))) {
                if (port != 0) {
                    return 0;
                }
                host_end = authority + index;
                port = host_end + 1U;
                port_length = (((size_t)(authority_end - port)));
            }
            index = index + 1U;
        }
        host_length = (((size_t)(host_end - authority)));
        if (lm_native_message_thread_reg_name_is_valid(authority, host_length) == 0) {
            return 0;
        }
    }
    if (port != 0 && lm_native_message_thread_port_is_valid(port, port_length) == 0) {
        return 0;
    }
    return lm_native_message_thread_path_is_valid((((const char *)path)), 0);
}

static char * lm_native_message_thread_join_uri(const char *endpoint, const char *route) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    size_t endpoint_length;
    size_t route_length;
    size_t copied_endpoint_length;
    char *uri;
    if (endpoint == 0 || route == 0) {
        return 0;
    }
    endpoint_length = strlen(endpoint);
    route_length = strlen(route);
    copied_endpoint_length = endpoint_length;
    if (endpoint_length != 0U && route_length != 0U && endpoint[endpoint_length - 1U] == '/' && route[0] == '/') {
        copied_endpoint_length = copied_endpoint_length - 1U;
    }
    if (route_length > (((size_t)-1)) - copied_endpoint_length - 1U) {
        return 0;
    }
    uri = malloc(copied_endpoint_length + route_length + 1U);
    if (uri == 0) {
        return 0;
    }
    if (copied_endpoint_length != 0U) {
        memcpy(uri, endpoint, copied_endpoint_length);
    }
    if (route_length != 0U) {
        memcpy(uri + copied_endpoint_length, route, route_length);
    }
    uri[copied_endpoint_length + route_length] = '\0';
    return uri;
}

static LmMessage * lm_native_message_thread_message_new(const char *lmx, size_t length) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    LmMessage * message;
    if (lmx == 0 || length == (((size_t)-1))) {
        return 0;
    }
    message = calloc(1U, sizeof(message[0]));
    if (message == 0) {
        return 0;
    }
    message->lmx = malloc(length + 1U);
    if (message -> lmx == 0) {
        free(message);
        return 0;
    }
    if (length != 0U) {
        memcpy(message -> lmx, lmx, length);
    }
    message->lmx[length] = '\0';
    message->length = length;
    return message;
}

static void lm_native_message_thread_message_delete(LmMessage *message) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    if (message != 0) {
        free(message -> lmx);
        message->lmx = 0;
        free(message);
    }
}

static void lm_native_message_thread_outbox_entry_delete(LmMessageOutboxEntry *entry) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    if (entry != 0) {
        free(entry -> endpoint);
        free(entry -> route);
        lm_native_message_thread_message_delete(entry -> message);
        free(entry);
    }
}

static void lm_native_message_thread_mailboxes_destroy(LmMessageThread *thread) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    LmMessage * message;
    LmMessageOutboxEntry * entry;
    if (thread == 0) {
        return;
    }
    message = thread -> inbox_head;
    while (message != 0) {
        LmMessage * next = message -> next;
        lm_native_message_thread_message_delete(message);
        message = next;
    }
    entry = thread -> outbox_head;
    while (entry != 0) {
        LmMessageOutboxEntry * next = entry -> next;
        lm_native_message_thread_outbox_entry_delete(entry);
        entry = next;
    }
    lm_native_message_thread_message_delete(thread -> current_message);
    thread->inbox_head = 0;
    thread->inbox_tail = 0;
    thread->inbox_count = 0U;
    thread->outbox_head = 0;
    thread->outbox_tail = 0;
    thread->outbox_count = 0U;
    thread->current_message = 0;
}

static void lm_native_message_thread_ready_push_locked(LmMessageThreadPool *pool, LmMessageThread *thread) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    thread->ready_next = 0;
    thread->scheduled = 1;
    if (pool -> ready_tail == 0) {
        pool->ready_head = thread;
    }
    else {
        pool->ready_tail->ready_next = thread;
    }
    pool->ready_tail = thread;
}

static LmMessageThread * lm_native_message_thread_ready_pop_locked(LmMessageThreadPool *pool) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    LmMessageThread * thread = pool -> ready_head;
    if (thread == 0) {
        return 0;
    }
    pool->ready_head = thread -> ready_next;
    if (pool -> ready_head == 0) {
        pool->ready_tail = 0;
    }
    thread->ready_next = 0;
    return thread;
}

static void lm_native_message_thread_schedule_stop_locked(LmMessageThreadPool *pool, LmMessageThread *thread) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    if (! thread -> scheduled && ! thread -> executing) {
        lm_native_message_thread_ready_push_locked(pool, thread);
        lm_condition_signal(pool -> work_ready);
    }
}

static int lm_native_message_thread_pool_is_controller(const LmMessageThreadPool *pool) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    return pool != 0 && pool -> runtime != 0 && lm_native_thread_identity_is_current(pool -> runtime -> identity);
}

static int lm_native_message_thread_pool_lifecycle_allowed(const LmMessageThreadPool *pool) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    return lm_native_message_thread_pool_is_controller(pool) && pool -> runtime -> single_execution_depth == 0U && (pool -> runtime -> root_thread == 0 || ! pool -> runtime -> root_thread -> turn_active);
}

static int lm_native_message_thread_control_allowed_locked(const LmMessageThreadPool *pool, const LmMessageThread *thread) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    if (lm_native_message_thread_pool_lifecycle_allowed(pool)) {
        return 1;
    }
    if (pool -> single_mode) {
        return pool -> runtime -> single_active_thread == thread;
    }
    return thread != 0 && thread -> executing && lm_native_thread_identity_is_current(thread -> execution_identity);
}

static int lm_native_message_thread_invoke(LmMessageThread *thread, LmMessageThreadEntry entry, void *argument) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    return lm_lmx_message_thread_invoke_diagnostic(thread, entry, argument);
}

static void lm_native_message_thread_pool_stop_locked(LmMessageThreadPool *pool) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    LmMessageThread * thread;
    pool->stop_requested = 1;
    thread = pool -> member_head;
    while (thread != 0) {
        if (lm_native_message_thread_state_lock(thread) == 0) {
            if (thread -> state == LM_MESSAGE_THREAD_RUNNING) {
                thread->stop_status = 0;
                thread->state = LM_MESSAGE_THREAD_STOPPING;
                lm_native_message_thread_schedule_stop_locked(pool, thread);
            }
            else {
                if (thread -> state == LM_MESSAGE_THREAD_NEW) {
                    thread->state = LM_MESSAGE_THREAD_STOPPED;
                    lm_condition_broadcast(thread -> stopped_condition);
                }
            }
            lm_native_message_thread_state_unlock(thread);
        }
        thread = thread -> pool_next;
    }
}

static int lm_native_message_thread_pool_is_quiescent_locked(LmMessageThreadPool *pool) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    LmMessageThread * thread;
    if (pool == 0 || pool -> ready_head != 0) {
        return 0;
    }
    thread = pool -> member_head;
    while (thread != 0) {
        int busy;
        if (lm_native_message_thread_state_lock(thread) != 0) {
            abort();
        }
        busy = thread -> scheduled || thread -> executing || thread -> current_message != 0 || thread -> outbox_head != 0 || (thread -> state == LM_MESSAGE_THREAD_RUNNING && thread -> mailbox_mode && thread -> inbox_head != 0);
        lm_native_message_thread_state_unlock(thread);
        if (busy) {
            return 0;
        }
        thread = thread -> pool_next;
    }
    return 1;
}

static void lm_native_message_thread_pool_maybe_finish_drain_locked(LmMessageThreadPool *pool) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    if (pool != 0 && pool -> drain_requested && ! pool -> stop_requested && lm_native_message_thread_pool_is_quiescent_locked(pool)) {
        lm_native_message_thread_pool_stop_locked(pool);
        lm_condition_broadcast(pool -> work_ready);
    }
}

static int lm_native_message_thread_pool_begin_turn(LmMessageThread *thread) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    if (lm_native_message_thread_state_lock(thread) != 0) {
        abort();
    }
    if (thread -> state != LM_MESSAGE_THREAD_RUNNING) {
        lm_native_message_thread_state_unlock(thread);
        return 0;
    }
    if (thread -> mailbox_mode) {
        LmMessage * message = thread -> inbox_head;
        if (message == 0 || thread -> current_message != 0) {
            lm_native_message_thread_state_unlock(thread);
            return 0;
        }
        thread->inbox_head = message -> next;
        if (thread -> inbox_head == 0) {
            thread->inbox_tail = 0;
        }
        thread->inbox_count = thread -> inbox_count - 1U;
        message->next = 0;
        thread->current_message = message;
    }
    thread->turn_active = 1;
    thread->turn_failed = 0;
    thread->turn_count = thread -> turn_count + 1U;
    lm_native_message_thread_state_unlock(thread);
    return 1;
}

static int lm_native_message_thread_pool_collect(LmMessageThread *thread) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    if (lm_native_message_thread_state_lock(thread) != 0) {
        abort();
    }
    thread->collection_count = thread -> collection_count + 1U;
    lm_native_message_thread_state_unlock(thread);
    if (thread -> root_owner == 0 || thread -> root_owner -> owner_thread != thread || thread -> arena_head != thread -> root_owner || thread -> root_owner -> registry_previous != 0 || thread -> arena_tail == 0 || thread -> arena_tail -> registry_next != 0 || thread -> arena_count == 0U) {
        if (lm_native_message_thread_state_lock(thread) != 0) {
            abort();
        }
        thread->collector_failed = 1;
        lm_native_message_thread_state_unlock(thread);
        lm_message_thread_request_failure(thread, 1);
        return 1;
    }
    if (lm_own_tree_cut(thread, thread -> root_owner) != 0) {
        if (lm_native_message_thread_state_lock(thread) != 0) {
            abort();
        }
        thread->collector_failed = 1;
        lm_native_message_thread_state_unlock(thread);
        lm_message_thread_request_failure(thread, 1);
        return 1;
    }
    if (lm_own_arena_reclaim(thread, thread -> root_owner) != 0) {
        if (lm_native_message_thread_state_lock(thread) != 0) {
            abort();
        }
        thread->collector_failed = 1;
        lm_native_message_thread_state_unlock(thread);
        lm_message_thread_request_failure(thread, 1);
        return 1;
    }
    return 0;
}

static int lm_native_message_thread_pool_end_turn(LmMessageThread *thread) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    int status = lm_native_message_thread_pool_collect(thread);
    if (lm_native_message_thread_state_lock(thread) != 0) {
        abort();
    }
    thread->turn_active = 0;
    if (thread -> state == LM_MESSAGE_THREAD_STOPPING) {
        thread->state = LM_MESSAGE_THREAD_STOPPED;
    }
    if (thread -> state == LM_MESSAGE_THREAD_STOPPED && thread -> stopped_condition != 0) {
        lm_condition_broadcast(thread -> stopped_condition);
    }
    lm_native_message_thread_state_unlock(thread);
    return status;
}

static int lm_native_message_thread_mark_exit_ready(LmMessageThread *thread) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    LmMessageThreadRuntime * runtime;
    int effective_status;
    int turn_failed;
    int became_ready = 0;
    if (thread == 0) {
        return 0;
    }
    if (lm_native_message_thread_state_lock(thread) != 0) {
        return 0;
    }
    runtime = thread -> runtime;
    effective_status = thread -> stop_status;
    turn_failed = thread -> turn_failed;
    lm_native_message_thread_state_unlock(thread);
    if (runtime == 0) {
        return 0;
    }
    if (runtime -> route_mutex == 0 || lm_mutex_lock(runtime -> route_mutex) != 0) {
        return 0;
    }
    if (runtime -> exit_requested && ! runtime -> exit_ready && runtime -> exit_requester == thread) {
        if (turn_failed) {
            runtime->exit_status = effective_status;
        }
        runtime->exit_ready = 1;
        runtime->exit_requester = 0;
        became_ready = 1;
    }
    lm_mutex_unlock(runtime -> route_mutex);
    return became_ready;
}

static int lm_native_message_thread_deliver_remote(LmMessageThreadRuntime *runtime, const char *endpoint, const char *route, const LmMessage *message) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    char *normalized_uri;
    unsigned http_status = 0U;
    int transport_status;
    if (runtime == 0 || message == 0 || runtime -> route_mutex == 0 || lm_mutex_lock(runtime -> route_mutex) != 0) {
        return 1;
    }
    if (runtime -> exit_ready) {
        lm_mutex_unlock(runtime -> route_mutex);
        return LM_MESSAGE_STATUS_EXIT_DISCARDED;
    }
    LmRestLmxPost post = runtime -> rest_lmx_post;
    void *context = runtime -> rest_lmx_context;
    lm_mutex_unlock(runtime -> route_mutex);
    if (post == 0) {
        return LM_MESSAGE_STATUS_TRANSPORT_PROVIDER_NOT_CONFIGURED;
    }
    normalized_uri = lm_native_message_thread_join_uri(endpoint, route);
    if (normalized_uri == 0) {
        return 1;
    }
    transport_status = post(context, normalized_uri, message -> lmx, message -> length, &http_status);
    free(normalized_uri);
    if (transport_status != 0) {
        return LM_MESSAGE_STATUS_TRANSPORT_FAILED;
    }
    if (http_status >= 200U && http_status <= 299U) {
        return 0;
    }
    if (http_status == 404U) {
        return LM_MESSAGE_STATUS_ROUTE_NOT_FOUND;
    }
    if (http_status >= 100U && http_status <= 599U) {
        return LM_MESSAGE_STATUS_HTTP_REJECTED;
    }
    return LM_MESSAGE_STATUS_TRANSPORT_PROTOCOL_ERROR;
}

static int lm_native_message_thread_deliver_local(LmMessageThreadRuntime *runtime, const char *route, LmMessage *message, int reject_exit_requested) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    LmMessageRoute * binding;
    LmMessageThread * target;
    LmMessageThreadPool * pool;
    int status = LM_MESSAGE_STATUS_ROUTE_NOT_FOUND;
    if (runtime == 0 || runtime -> route_mutex == 0 || lm_mutex_lock(runtime -> route_mutex) != 0) {
        return 1;
    }
    if (reject_exit_requested && runtime -> exit_requested) {
        lm_mutex_unlock(runtime -> route_mutex);
        return LM_MESSAGE_STATUS_APPLICATION_STOPPING;
    }
    if (runtime -> exit_ready) {
        lm_mutex_unlock(runtime -> route_mutex);
        return LM_MESSAGE_STATUS_EXIT_DISCARDED;
    }
    binding = runtime -> route_head;
    while (binding != 0 && strcmp(binding -> route, route) != 0) {
        binding = binding -> next;
    }
    if (binding == 0) {
        lm_mutex_unlock(runtime -> route_mutex);
        return status;
    }
    if (binding -> target == 0 || binding -> target -> pool == 0) {
        lm_mutex_unlock(runtime -> route_mutex);
        if (reject_exit_requested) {
            return LM_MESSAGE_STATUS_APPLICATION_STOPPING;
        }
        return status;
    }
    target = binding -> target;
    pool = target -> pool;
    if (pool -> runtime != runtime || lm_mutex_lock(pool -> mutex) != 0) {
        lm_mutex_unlock(runtime -> route_mutex);
        return 1;
    }
    if (lm_native_message_thread_state_lock(target) != 0) {
        lm_mutex_unlock(pool -> mutex);
        lm_mutex_unlock(runtime -> route_mutex);
        return 1;
    }
    if (! pool -> stop_requested && ! pool -> deleting && (! reject_exit_requested || ! pool -> drain_requested) && target -> pool == pool && target -> mailbox_mode && target -> started && target -> state == LM_MESSAGE_THREAD_RUNNING && target -> inbox_count != (((size_t)-1))) {
        message->next = 0;
        if (target -> inbox_tail == 0) {
            target->inbox_head = message;
        }
        else {
            target->inbox_tail->next = message;
        }
        target->inbox_tail = message;
        target->inbox_count = target -> inbox_count + 1U;
        if (! target -> scheduled && ! target -> executing) {
            lm_native_message_thread_ready_push_locked(pool, target);
            lm_condition_signal(pool -> work_ready);
        }
        status = 0;
    }
    else {
        if (reject_exit_requested && (pool -> stop_requested || pool -> drain_requested || pool -> deleting || target -> pool != pool || ! target -> mailbox_mode || ! target -> started || target -> state != LM_MESSAGE_THREAD_RUNNING)) {
            status = LM_MESSAGE_STATUS_APPLICATION_STOPPING;
        }
    }
    lm_native_message_thread_state_unlock(target);
    lm_mutex_unlock(pool -> mutex);
    lm_mutex_unlock(runtime -> route_mutex);
    return status;
}

static LmMessageOutboxEntry * lm_native_message_thread_outbox_take(LmMessageThread *thread) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    LmMessageOutboxEntry * head;
    if (lm_native_message_thread_state_lock(thread) != 0) {
        return 0;
    }
    head = thread -> outbox_head;
    thread->outbox_head = 0;
    thread->outbox_tail = 0;
    thread->outbox_count = 0U;
    lm_native_message_thread_state_unlock(thread);
    return head;
}

static int lm_native_message_thread_outbox_finish(LmMessageThread *thread, int commit) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    LmMessageOutboxEntry * entry = lm_native_message_thread_outbox_take(thread);
    int first_status = 0;
    while (entry != 0) {
        LmMessageOutboxEntry * next = entry -> next;
        int status = 0;
        entry->next = 0;
        if (commit) {
            if (entry -> endpoint == 0 || entry -> endpoint[0] == '\0') {
                status = lm_native_message_thread_deliver_local(thread -> runtime, entry -> route, entry -> message, 0);
                if (status == 0) {
                    entry->message = 0;
                }
            }
            else {
                status = lm_native_message_thread_deliver_remote(thread -> runtime, entry -> endpoint, entry -> route, entry -> message);
            }
        }
        if (first_status == 0 && status != 0 && status != LM_MESSAGE_STATUS_EXIT_DISCARDED) {
            first_status = status;
        }
        lm_native_message_thread_outbox_entry_delete(entry);
        entry = next;
    }
    if (first_status != 0) {
        lm_message_thread_request_failure(thread, first_status);
    }
    return first_status;
}

static void lm_native_message_thread_current_message_finish(LmMessageThread *thread) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    LmMessage * message;
    if (lm_native_message_thread_state_lock(thread) != 0) {
        abort();
    }
    message = thread -> current_message;
    thread->current_message = 0;
    lm_native_message_thread_state_unlock(thread);
    lm_native_message_thread_message_delete(message);
}

static void lm_native_message_thread_execute(LmMessageThreadPool *pool, LmMessageThread *thread) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    LmMessageThreadRuntime * runtime = 0;
    LmMessageThread * previous_active_thread = 0;
    int run_entry;
    int invoke_status = 0;
    int commit_allowed = 0;
    int exit_became_ready = 0;
    if (pool -> single_mode) {
        runtime = pool -> runtime;
        previous_active_thread = runtime -> single_active_thread;
        runtime->single_active_thread = thread;
        runtime->single_execution_depth = runtime -> single_execution_depth + 1U;
    }
    run_entry = lm_native_message_thread_pool_begin_turn(thread);
    if (run_entry) {
        if (thread -> entry == 0) {
            lm_message_thread_request_failure(thread, 1);
            invoke_status = 1;
        }
        else {
            invoke_status = lm_native_message_thread_invoke(thread, thread -> entry, thread -> entry_argument);
        }
    }
    if (lm_native_message_thread_state_lock(thread) != 0) {
        abort();
    }
    commit_allowed = run_entry && invoke_status == 0 && ! thread -> turn_failed;
    lm_native_message_thread_state_unlock(thread);
    lm_native_message_thread_outbox_finish(thread, commit_allowed);
    lm_native_message_thread_current_message_finish(thread);
    lm_native_message_thread_pool_end_turn(thread);
    exit_became_ready = lm_native_message_thread_mark_exit_ready(thread);
    if (runtime != 0) {
        runtime->single_execution_depth = runtime -> single_execution_depth - 1U;
        runtime->single_active_thread = previous_active_thread;
    }
    if (lm_mutex_lock(pool -> mutex) != 0) {
        abort();
    }
    if (lm_native_message_thread_state_lock(thread) != 0) {
        abort();
    }
    thread->execution_identity = 0;
    thread->executing = 0;
    lm_native_message_thread_state_unlock(thread);
    if (exit_became_ready) {
        lm_native_message_thread_pool_stop_locked(pool);
    }
    if (lm_native_message_thread_state_lock(thread) != 0) {
        abort();
    }
    if (pool -> stop_requested && thread -> state == LM_MESSAGE_THREAD_RUNNING) {
        thread->stop_status = 0;
        thread->state = LM_MESSAGE_THREAD_STOPPING;
    }
    if (thread -> state == LM_MESSAGE_THREAD_STOPPING && ! thread -> scheduled) {
        lm_native_message_thread_schedule_stop_locked(pool, thread);
    }
    if (thread -> state == LM_MESSAGE_THREAD_RUNNING && ((thread -> mailbox_mode && thread -> inbox_head != 0) || (! thread -> mailbox_mode && ! pool -> drain_requested))) {
        if (! thread -> scheduled) {
            lm_native_message_thread_ready_push_locked(pool, thread);
            lm_condition_signal(pool -> work_ready);
        }
    }
    else {
        lm_condition_broadcast(thread -> stopped_condition);
    }
    lm_native_message_thread_state_unlock(thread);
    lm_native_message_thread_pool_maybe_finish_drain_locked(pool);
    lm_mutex_unlock(pool -> mutex);
}

static void * lm_native_message_thread_worker(void *argument) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    LmMessageThreadPool * pool = (((LmMessageThreadPool *)argument));
    void *worker_identity = lm_native_thread_identity_new_current();
    if (worker_identity == 0) {
        abort();
    }
    while (1) {
        LmMessageThread * thread;
        if (lm_mutex_lock(pool -> mutex) != 0) {
            abort();
        }
        while (pool -> ready_head == 0 && ! pool -> stop_requested) {
            if (lm_condition_wait(pool -> work_ready, pool -> mutex) != 0) {
                abort();
            }
        }
        if (pool -> ready_head == 0 && pool -> stop_requested) {
            lm_mutex_unlock(pool -> mutex);
            lm_native_thread_identity_delete(worker_identity);
            return 0;
        }
        thread = lm_native_message_thread_ready_pop_locked(pool);
        if (thread == 0 || lm_native_message_thread_state_lock(thread) != 0) {
            abort();
        }
        thread->scheduled = 0;
        thread->executing = 1;
        thread->execution_identity = worker_identity;
        lm_native_message_thread_state_unlock(thread);
        lm_mutex_unlock(pool -> mutex);
        lm_native_message_thread_execute(pool, thread);
    }
    return 0;
}

static int lm_native_message_thread_pool_pump_one(LmMessageThreadPool *pool) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    LmMessageThread * thread;
    if (pool == 0 || ! pool -> single_mode) {
        return -1;
    }
    if (lm_mutex_lock(pool -> mutex) != 0) {
        abort();
    }
    thread = lm_native_message_thread_ready_pop_locked(pool);
    if (thread == 0) {
        lm_mutex_unlock(pool -> mutex);
        return 0;
    }
    if (lm_native_message_thread_state_lock(thread) != 0) {
        abort();
    }
    thread->scheduled = 0;
    thread->executing = 1;
    lm_native_message_thread_state_unlock(thread);
    lm_mutex_unlock(pool -> mutex);
    lm_native_message_thread_execute(pool, thread);
    return 1;
}

size_t lm_message_thread_pool_pump(LmMessageThreadPool *pool, size_t max_turns) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    size_t completed = 0U;
    if (pool == 0 || max_turns == 0U || ! pool -> single_mode || pool -> deleting || lm_native_message_thread_pool_lifecycle_allowed(pool) == 0) {
        return 0U;
    }
    while (completed < max_turns && lm_native_message_thread_pool_pump_one(pool) > 0) {
        completed = completed + 1U;
    }
    return completed;
}

int lm_message_thread_runtime_set_rest_lmx_provider(LmMessageThreadRuntime *runtime, const LmRestLmxProviderOpsV1 *ops, void *context) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    LmRestLmxPost new_post = 0;
    LmRestLmxDestroy new_destroy = 0;
    LmRestLmxPost *new_post_slot = &new_post;
    LmRestLmxDestroy *new_destroy_slot = &new_destroy;
    int destroy_old = 0;
    if (runtime == 0 || runtime -> route_mutex == 0 || lm_native_thread_identity_is_current(runtime -> identity) == 0) {
        return 1;
    }
    if (ops == 0) {
        if (context != 0) {
            return 1;
        }
    }
    else {
        if (ops -> abi_size != sizeof(ops[0]) || ops -> post == 0) {
            return 1;
        }
        new_post_slot[0] = ops -> post;
        new_destroy_slot[0] = ops -> destroy;
    }
    if (lm_mutex_lock(runtime -> route_mutex) != 0) {
        return 1;
    }
    if (runtime -> rest_lmx_provider_sealed || runtime -> rest_lmx_provider_transition || runtime -> deleting || runtime -> pool_count != 0U || runtime -> single_execution_depth != 0U || runtime -> route_count != 0U || runtime -> route_head != 0 || runtime -> root_thread != 0 || runtime -> exit_requester != 0 || runtime -> exit_requested || runtime -> exit_ready) {
        lm_mutex_unlock(runtime -> route_mutex);
        return 1;
    }
    LmRestLmxPost old_post = runtime -> rest_lmx_post;
    LmRestLmxDestroy old_destroy = runtime -> rest_lmx_destroy;
    void *old_context = runtime -> rest_lmx_context;
    if (old_post == new_post && old_destroy == new_destroy && old_context == context) {
        lm_mutex_unlock(runtime -> route_mutex);
        return 0;
    }
    if (old_post != 0 && old_context != 0 && old_context == context) {
        lm_mutex_unlock(runtime -> route_mutex);
        return 1;
    }
    destroy_old = old_post != 0 && old_destroy != 0;
    runtime->rest_lmx_provider_transition = destroy_old;
    runtime->rest_lmx_post = new_post;
    runtime->rest_lmx_destroy = new_destroy;
    runtime->rest_lmx_context = context;
    lm_mutex_unlock(runtime -> route_mutex);
    if (destroy_old) {
        old_destroy(old_context);
        if (lm_mutex_lock(runtime -> route_mutex) != 0) {
            abort();
        }
        if (! runtime -> rest_lmx_provider_transition) {
            abort();
        }
        runtime->rest_lmx_provider_transition = 0;
        lm_mutex_unlock(runtime -> route_mutex);
    }
    return 0;
}

int lm_message_thread_runtime_admit_lmx(LmMessageThreadRuntime *runtime, const char *route, const char *lmx, size_t length) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    LmMessage * message;
    int status;
    if (lm_native_message_thread_route_is_valid(route) == 0) {
        return LM_MESSAGE_STATUS_INVALID_ADDRESS;
    }
    if (runtime == 0 || lmx == 0 || length == (((size_t)-1))) {
        return 1;
    }
    message = lm_native_message_thread_message_new(lmx, length);
    if (message == 0) {
        return 1;
    }
    status = lm_native_message_thread_deliver_local(runtime, route, message, 1);
    if (status != 0) {
        lm_native_message_thread_message_delete(message);
    }
    return status;
}

int lm_message_thread_request_exit(LmMessageThread *requester, int status) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    LmMessageThreadRuntime * runtime;
    int valid = 0;
    int selected_status;
    if (requester == 0 || lm_native_message_thread_state_lock(requester) != 0) {
        return 1;
    }
    runtime = requester -> runtime;
    if (runtime != 0 && lm_native_message_thread_is_current(requester) && ((requester -> pool == 0 && runtime -> root_thread == requester) || (requester -> pool != 0 && requester -> pool -> runtime == runtime))) {
        valid = 1;
    }
    lm_native_message_thread_state_unlock(requester);
    if (! valid || runtime -> route_mutex == 0 || lm_mutex_lock(runtime -> route_mutex) != 0) {
        return 1;
    }
    if (! runtime -> exit_requested) {
        runtime->exit_requester = requester;
        runtime->exit_requested = 1;
        runtime->exit_ready = 0;
        runtime->exit_status = status;
    }
    selected_status = runtime -> exit_status;
    lm_mutex_unlock(runtime -> route_mutex);
    lm_message_thread_request_stop(requester, selected_status);
    return 0;
}

int lm_message_thread_runtime_exit_state(LmMessageThreadRuntime *runtime, int *out_requested, int *out_ready, int *out_status) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    if (out_requested == 0 || out_ready == 0 || out_status == 0) {
        return 1;
    }
    out_requested[0] = 0;
    out_ready[0] = 0;
    out_status[0] = 0;
    if (runtime == 0 || runtime -> route_mutex == 0 || lm_mutex_lock(runtime -> route_mutex) != 0) {
        return 1;
    }
    out_requested[0] = runtime -> exit_requested;
    out_ready[0] = runtime -> exit_ready;
    out_status[0] = runtime -> exit_status;
    lm_mutex_unlock(runtime -> route_mutex);
    return 0;
}

int lm_message_thread_runtime_attach_root(LmMessageThreadRuntime *runtime, LmMessageThread *thread) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    int status = 1;
    if (runtime == 0 || thread == 0 || runtime -> route_mutex == 0 || lm_native_thread_identity_is_current(runtime -> identity) == 0 || lm_mutex_lock(runtime -> route_mutex) != 0) {
        return 1;
    }
    if (runtime -> deleting || runtime -> rest_lmx_provider_transition) {
        lm_mutex_unlock(runtime -> route_mutex);
        return 1;
    }
    if (lm_native_message_thread_state_lock(thread) == 0) {
        if (runtime -> root_thread == 0 && thread -> runtime == 0 && thread -> pool == 0 && ! thread -> started && ! thread -> turn_active && ! thread -> executing && thread -> state == LM_MESSAGE_THREAD_RUNNING && thread -> outbox_head == 0 && thread -> current_message == 0) {
            runtime->root_thread = thread;
            runtime->rest_lmx_provider_sealed = 1;
            thread->runtime = runtime;
            status = 0;
        }
        lm_native_message_thread_state_unlock(thread);
    }
    lm_mutex_unlock(runtime -> route_mutex);
    return status;
}

int lm_message_thread_runtime_detach_root(LmMessageThreadRuntime *runtime, LmMessageThread *thread) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    int status = 1;
    if (runtime == 0 || thread == 0 || runtime -> route_mutex == 0 || lm_native_thread_identity_is_current(runtime -> identity) == 0 || lm_mutex_lock(runtime -> route_mutex) != 0) {
        return 1;
    }
    if (lm_native_message_thread_state_lock(thread) == 0) {
        if (runtime -> root_thread == thread && thread -> runtime == runtime && thread -> pool == 0 && ! thread -> turn_active && ! thread -> executing && thread -> outbox_head == 0) {
            runtime->root_thread = 0;
            thread->runtime = 0;
            status = 0;
        }
        lm_native_message_thread_state_unlock(thread);
    }
    lm_mutex_unlock(runtime -> route_mutex);
    return status;
}

int lm_message_thread_runtime_delete(LmMessageThreadRuntime *runtime) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    if (runtime == 0) {
        return 0;
    }
    if (lm_native_thread_identity_is_current(runtime -> identity) == 0 || runtime -> route_mutex == 0 || lm_mutex_lock(runtime -> route_mutex) != 0) {
        return 1;
    }
    if (runtime -> deleting || runtime -> rest_lmx_provider_transition || runtime -> pool_count != 0U || runtime -> single_execution_depth != 0U || runtime -> route_count != 0U || runtime -> route_head != 0 || runtime -> root_thread != 0) {
        lm_mutex_unlock(runtime -> route_mutex);
        return 1;
    }
    LmRestLmxPost old_post = runtime -> rest_lmx_post;
    LmRestLmxDestroy old_destroy = runtime -> rest_lmx_destroy;
    void *old_context = runtime -> rest_lmx_context;
    runtime->rest_lmx_post = 0;
    runtime->rest_lmx_destroy = 0;
    runtime->rest_lmx_context = 0;
    runtime->rest_lmx_provider_sealed = 1;
    runtime->deleting = 1;
    lm_mutex_unlock(runtime -> route_mutex);
    if (old_post != 0 && old_destroy != 0) {
        old_destroy(old_context);
    }
    lm_mutex_delete(runtime -> route_mutex);
    runtime->route_mutex = 0;
    lm_native_thread_identity_delete(runtime -> identity);
    runtime->identity = 0;
    free(runtime);
    return 0;
}

static int lm_native_message_thread_runtime_acquire_pool(LmMessageThreadRuntime *runtime) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    if (runtime == 0 || runtime -> route_mutex == 0 || lm_mutex_lock(runtime -> route_mutex) != 0) {
        return 1;
    }
    if (runtime -> deleting || runtime -> rest_lmx_provider_transition || runtime -> pool_count == (((size_t)-1))) {
        lm_mutex_unlock(runtime -> route_mutex);
        return 1;
    }
    runtime->rest_lmx_provider_sealed = 1;
    runtime->pool_count = runtime -> pool_count + 1U;
    lm_mutex_unlock(runtime -> route_mutex);
    return 0;
}

static void lm_native_message_thread_runtime_release_pool(LmMessageThreadRuntime *runtime) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    if (runtime == 0 || runtime -> route_mutex == 0 || lm_mutex_lock(runtime -> route_mutex) != 0) {
        abort();
    }
    if (runtime -> pool_count == 0U) {
        abort();
    }
    runtime->pool_count = runtime -> pool_count - 1U;
    lm_mutex_unlock(runtime -> route_mutex);
}

LmMessageThreadPool * lm_message_thread_pool_new(LmMessageThreadRuntime *runtime, size_t worker_count) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    LmMessageThreadPool * pool;
    size_t index;
    if (runtime == 0 || worker_count == 0U || lm_native_thread_identity_is_current(runtime -> identity) == 0) {
        return 0;
    }
    if (runtime -> single_execution_depth != 0U) {
        return 0;
    }
    pool = calloc(1U, sizeof(pool[0]));
    if (pool == 0) {
        return 0;
    }
    pool->runtime = runtime;
    pool->mutex = lm_mutex_new();
    pool->work_ready = lm_condition_new();
    pool->single_mode = strcmp(lm_thread_provider_name(), "single") == 0;
    if (pool -> mutex == 0 || pool -> work_ready == 0) {
        lm_condition_delete(pool -> work_ready);
        lm_mutex_delete(pool -> mutex);
        free(pool);
        return 0;
    }
    if (lm_native_message_thread_runtime_acquire_pool(runtime) != 0) {
        lm_condition_delete(pool -> work_ready);
        lm_mutex_delete(pool -> mutex);
        free(pool);
        return 0;
    }
    if (pool -> single_mode) {
        return pool;
    }
    pool->worker_count = worker_count;
    pool->workers = calloc(worker_count, sizeof(pool -> workers[0]));
    if (pool -> workers == 0) {
        lm_condition_delete(pool -> work_ready);
        lm_mutex_delete(pool -> mutex);
        lm_native_message_thread_runtime_release_pool(runtime);
        free(pool);
        return 0;
    }
    index = 0U;
    while (index < worker_count) {
        pool->workers[index] = lm_host_thread_new();
        if (pool -> workers[index] == 0 || lm_host_thread_start(pool -> workers[index], &lm_native_message_thread_worker, pool) != 0) {
            size_t cleanup_index;
            int cleanup_failed = 0;
            if (lm_mutex_lock(pool -> mutex) != 0) {
                return 0;
            }
            lm_native_message_thread_pool_stop_locked(pool);
            lm_condition_broadcast(pool -> work_ready);
            lm_mutex_unlock(pool -> mutex);
            cleanup_index = 0U;
            while (cleanup_index < pool -> started_worker_count) {
                if (lm_host_thread_join(pool -> workers[cleanup_index], 0) != 0) {
                    cleanup_failed = 1;
                }
                cleanup_index = cleanup_index + 1U;
            }
            if (cleanup_failed) {
                return 0;
            }
            cleanup_index = 0U;
            while (cleanup_index <= index) {
                lm_host_thread_delete(pool -> workers[cleanup_index]);
                cleanup_index = cleanup_index + 1U;
            }
            free(pool -> workers);
            lm_condition_delete(pool -> work_ready);
            lm_mutex_delete(pool -> mutex);
            lm_native_message_thread_runtime_release_pool(runtime);
            free(pool);
            return 0;
        }
        pool->started_worker_count = pool -> started_worker_count + 1U;
        index = index + 1U;
    }
    return pool;
}

void lm_message_thread_pool_request_stop(LmMessageThreadPool *pool) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    if (pool == 0 || lm_mutex_lock(pool -> mutex) != 0) {
        return;
    }
    if (pool -> deleting || lm_native_message_thread_pool_lifecycle_allowed(pool) == 0) {
        lm_mutex_unlock(pool -> mutex);
        return;
    }
    lm_native_message_thread_pool_stop_locked(pool);
    lm_condition_broadcast(pool -> work_ready);
    lm_mutex_unlock(pool -> mutex);
}

void lm_message_thread_pool_request_stop_when_idle(LmMessageThreadPool *pool) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    if (pool == 0 || lm_mutex_lock(pool -> mutex) != 0) {
        return;
    }
    if (pool -> deleting || lm_native_message_thread_pool_lifecycle_allowed(pool) == 0) {
        lm_mutex_unlock(pool -> mutex);
        return;
    }
    pool->drain_requested = 1;
    lm_native_message_thread_pool_maybe_finish_drain_locked(pool);
    lm_condition_broadcast(pool -> work_ready);
    lm_mutex_unlock(pool -> mutex);
}

LmMessageThreadRuntime * lm_message_thread_runtime_new(void) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    LmMessageThreadRuntime * runtime = calloc(1U, sizeof(runtime[0]));
    if (runtime == 0) {
        return 0;
    }
    runtime->identity = lm_native_thread_identity_new_current();
    runtime->route_mutex = lm_mutex_new();
    if (runtime -> identity == 0 || runtime -> route_mutex == 0) {
        lm_mutex_delete(runtime -> route_mutex);
        lm_native_thread_identity_delete(runtime -> identity);
        free(runtime);
        return 0;
    }
    return runtime;
}

LmMessageThread * lm_message_thread_new_in(LmMessageThreadPool *pool) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    LmMessageThread * thread;
    if (pool == 0 || lm_native_message_thread_pool_lifecycle_allowed(pool) == 0) {
        return 0;
    }
    thread = lm_message_thread_new();
    if (thread == 0) {
        return 0;
    }
    thread->state_mutex = lm_mutex_new();
    thread->stopped_condition = lm_condition_new();
    if (thread -> state_mutex == 0 || thread -> stopped_condition == 0) {
        lm_message_thread_delete(thread);
        return 0;
    }
    thread->state = LM_MESSAGE_THREAD_NEW;
    if (lm_mutex_lock(pool -> mutex) != 0) {
        lm_message_thread_delete(thread);
        return 0;
    }
    if (pool -> stop_requested || pool -> drain_requested || pool -> deleting) {
        lm_mutex_unlock(pool -> mutex);
        lm_message_thread_delete(thread);
        return 0;
    }
    thread->pool = pool;
    thread->runtime = pool -> runtime;
    thread->pool_previous = pool -> member_tail;
    if (pool -> member_tail == 0) {
        pool->member_head = thread;
    }
    else {
        pool->member_tail->pool_next = thread;
    }
    pool->member_tail = thread;
    pool->member_count = pool -> member_count + 1U;
    lm_mutex_unlock(pool -> mutex);
    return thread;
}

static int lm_native_message_thread_start(LmMessageThread *thread, LmMessageThreadEntry entry, void *argument, int mailbox_mode) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    LmMessageThreadPool * pool;
    if (thread == 0 || entry == 0 || thread -> pool == 0) {
        return 1;
    }
    pool = thread -> pool;
    if (lm_native_message_thread_pool_lifecycle_allowed(pool) == 0) {
        return 1;
    }
    if (lm_mutex_lock(pool -> mutex) != 0) {
        return 1;
    }
    if (lm_native_message_thread_state_lock(thread) != 0) {
        lm_mutex_unlock(pool -> mutex);
        return 1;
    }
    if (thread -> pool != pool || pool -> stop_requested || pool -> drain_requested || pool -> deleting || thread -> state != LM_MESSAGE_THREAD_NEW || thread -> started || thread -> scheduled || thread -> executing) {
        lm_native_message_thread_state_unlock(thread);
        lm_mutex_unlock(pool -> mutex);
        return 1;
    }
    thread->entry = entry;
    thread->entry_argument = argument;
    thread->mailbox_mode = mailbox_mode;
    thread->started = 1;
    thread->state = LM_MESSAGE_THREAD_RUNNING;
    if (! mailbox_mode) {
        lm_native_message_thread_ready_push_locked(pool, thread);
        lm_condition_signal(pool -> work_ready);
    }
    lm_native_message_thread_state_unlock(thread);
    lm_mutex_unlock(pool -> mutex);
    return 0;
}

int lm_message_thread_start(LmMessageThread *thread, LmMessageThreadEntry entry, void *argument) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    return lm_native_message_thread_start(thread, entry, argument, 0);
}

int lm_message_thread_start_mailbox(LmMessageThread *thread, LmMessageThreadEntry entry, void *argument) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    return lm_native_message_thread_start(thread, entry, argument, 1);
}

int lm_message_thread_bind_route(LmMessageThread *thread, const char *route) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    LmMessageThreadPool * pool;
    LmMessageThreadRuntime * runtime;
    LmMessageRoute * binding;
    LmMessageRoute * cursor;
    if (thread == 0 || thread -> pool == 0 || lm_native_message_thread_route_is_valid(route) == 0) {
        return LM_MESSAGE_STATUS_INVALID_ADDRESS;
    }
    pool = thread -> pool;
    runtime = pool -> runtime;
    if (runtime == 0 || lm_native_message_thread_pool_lifecycle_allowed(pool) == 0) {
        return 1;
    }
    binding = calloc(1U, sizeof(binding[0]));
    if (binding == 0) {
        return 1;
    }
    binding->route = lm_native_message_thread_copy_cstr(route);
    if (binding -> route == 0) {
        free(binding);
        return 1;
    }
    binding->target = thread;
    if (lm_mutex_lock(runtime -> route_mutex) != 0) {
        free(binding -> route);
        free(binding);
        return 1;
    }
    cursor = runtime -> route_head;
    while (cursor != 0) {
        if (strcmp(cursor -> route, route) == 0) {
            lm_mutex_unlock(runtime -> route_mutex);
            free(binding -> route);
            free(binding);
            return 1;
        }
        cursor = cursor -> next;
    }
    if (lm_mutex_lock(pool -> mutex) != 0) {
        lm_mutex_unlock(runtime -> route_mutex);
        free(binding -> route);
        free(binding);
        return 1;
    }
    if (lm_native_message_thread_state_lock(thread) != 0) {
        lm_mutex_unlock(pool -> mutex);
        lm_mutex_unlock(runtime -> route_mutex);
        free(binding -> route);
        free(binding);
        return 1;
    }
    if (pool -> runtime != runtime || pool -> stop_requested || pool -> drain_requested || pool -> deleting || thread -> pool != pool || ! thread -> started || ! thread -> mailbox_mode || thread -> state != LM_MESSAGE_THREAD_RUNNING || runtime -> route_count == (((size_t)-1))) {
        lm_native_message_thread_state_unlock(thread);
        lm_mutex_unlock(pool -> mutex);
        lm_mutex_unlock(runtime -> route_mutex);
        free(binding -> route);
        free(binding);
        return 1;
    }
    binding->next = runtime -> route_head;
    runtime->route_head = binding;
    runtime->route_count = runtime -> route_count + 1U;
    lm_native_message_thread_state_unlock(thread);
    lm_mutex_unlock(pool -> mutex);
    lm_mutex_unlock(runtime -> route_mutex);
    return 0;
}

int lm_message_thread_send_lmx(LmMessageThread *sender, const char *endpoint, const char *route, const char *lmx, size_t length) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    LmMessageOutboxEntry * entry;
    if (lm_native_message_thread_route_is_valid(route) == 0 || (endpoint != 0 && endpoint[0] != '\0' && lm_native_message_thread_endpoint_is_http(endpoint) == 0)) {
        return LM_MESSAGE_STATUS_INVALID_ADDRESS;
    }
    entry = calloc(1U, sizeof(entry[0]));
    if (entry == 0) {
        return 1;
    }
    entry->route = lm_native_message_thread_copy_cstr(route);
    if (endpoint != 0 && endpoint[0] != '\0') {
        entry->endpoint = lm_native_message_thread_copy_cstr(endpoint);
    }
    entry->message = lm_native_message_thread_message_new(lmx, length);
    if (entry -> route == 0 || entry -> message == 0 || (endpoint != 0 && endpoint[0] != '\0' && entry -> endpoint == 0)) {
        lm_native_message_thread_outbox_entry_delete(entry);
        return 1;
    }
    if (sender == 0 || lm_native_message_thread_state_lock(sender) != 0) {
        lm_native_message_thread_outbox_entry_delete(entry);
        return 1;
    }
    if (lm_native_message_thread_is_current(sender) == 0 || sender -> runtime == 0 || (sender -> pool != 0 && ! sender -> started) || (sender -> pool == 0 && sender -> runtime -> root_thread != sender) || (sender -> state != LM_MESSAGE_THREAD_RUNNING && sender -> state != LM_MESSAGE_THREAD_STOPPING) || sender -> outbox_count == (((size_t)-1))) {
        lm_native_message_thread_state_unlock(sender);
        lm_native_message_thread_outbox_entry_delete(entry);
        return 1;
    }
    entry->next = 0;
    if (sender -> outbox_tail == 0) {
        sender->outbox_head = entry;
    }
    else {
        sender->outbox_tail->next = entry;
    }
    sender->outbox_tail = entry;
    sender->outbox_count = sender -> outbox_count + 1U;
    lm_native_message_thread_state_unlock(sender);
    return 0;
}

int lm_message_thread_current_lmx(LmMessageThread *thread, const char **out_lmx, size_t *out_length) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    if (out_lmx == 0 || out_length == 0) {
        return 1;
    }
    out_lmx[0] = 0;
    out_length[0] = 0U;
    if (thread == 0 || lm_native_message_thread_state_lock(thread) != 0) {
        return 1;
    }
    if (lm_native_message_thread_is_current(thread) == 0 || thread -> current_message == 0) {
        lm_native_message_thread_state_unlock(thread);
        return 1;
    }
    out_lmx[0] = thread -> current_message -> lmx;
    out_length[0] = thread -> current_message -> length;
    lm_native_message_thread_state_unlock(thread);
    return 0;
}

size_t lm_message_thread_inbox_count(const LmMessageThread *thread) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    size_t result;
    if (thread == 0 || lm_native_message_thread_state_lock(thread) != 0) {
        return 0U;
    }
    result = thread -> inbox_count;
    lm_native_message_thread_state_unlock(thread);
    return result;
}

size_t lm_message_thread_outbox_count(const LmMessageThread *thread) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    size_t result;
    if (thread == 0 || lm_native_message_thread_state_lock(thread) != 0) {
        return 0U;
    }
    result = thread -> outbox_count;
    lm_native_message_thread_state_unlock(thread);
    return result;
}

int lm_message_thread_join(LmMessageThread *thread, int *result) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    LmMessageThreadPool * pool;
    if (thread == 0 || result == 0 || thread -> pool == 0) {
        return 1;
    }
    pool = thread -> pool;
    if (lm_native_message_thread_pool_lifecycle_allowed(pool) == 0) {
        return 1;
    }
    if (lm_mutex_lock(pool -> mutex) != 0) {
        return 1;
    }
    if (lm_native_message_thread_state_lock(thread) != 0) {
        lm_mutex_unlock(pool -> mutex);
        return 1;
    }
    if (thread -> pool != pool || pool -> deleting || ! thread -> started || thread -> joining || thread -> joined) {
        lm_native_message_thread_state_unlock(thread);
        lm_mutex_unlock(pool -> mutex);
        return 1;
    }
    thread->joining = 1;
    lm_mutex_unlock(pool -> mutex);
    if (! pool -> single_mode) {
        while (thread -> state != LM_MESSAGE_THREAD_STOPPED || thread -> scheduled || thread -> executing) {
            if (lm_condition_wait(thread -> stopped_condition, thread -> state_mutex) != 0) {
                thread->joining = 0;
                lm_condition_broadcast(thread -> stopped_condition);
                lm_native_message_thread_state_unlock(thread);
                return 1;
            }
        }
        result[0] = thread -> stop_status;
        thread->joined = 1;
        thread->joining = 0;
        lm_condition_broadcast(thread -> stopped_condition);
        lm_native_message_thread_state_unlock(thread);
        return 0;
    }
    lm_native_message_thread_state_unlock(thread);
    while (1) {
        int stopped;
        int pump_status;
        if (lm_native_message_thread_state_lock(thread) != 0) {
            return 1;
        }
        stopped = thread -> state == LM_MESSAGE_THREAD_STOPPED && ! thread -> scheduled && ! thread -> executing;
        if (stopped) {
            result[0] = thread -> stop_status;
            thread->joined = 1;
            thread->joining = 0;
            lm_condition_broadcast(thread -> stopped_condition);
            lm_native_message_thread_state_unlock(thread);
            return 0;
        }
        lm_native_message_thread_state_unlock(thread);
        pump_status = lm_native_message_thread_pool_pump_one(pool);
        if (pump_status <= 0) {
            if (lm_native_message_thread_state_lock(thread) == 0) {
                thread->joining = 0;
                lm_condition_broadcast(thread -> stopped_condition);
                lm_native_message_thread_state_unlock(thread);
            }
            return 1;
        }
    }
    return 1;
}

static void lm_native_message_thread_remove_pool_routes_locked(LmMessageThreadRuntime *runtime, LmMessageThreadPool *pool) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    LmMessageRoute * *slot = &runtime -> route_head;
    while (slot[0] != 0) {
        LmMessageRoute * binding = slot[0];
        if (binding -> target != 0 && binding -> target -> pool == pool) {
            slot[0] = binding -> next;
            free(binding -> route);
            free(binding);
            runtime->route_count = runtime -> route_count - 1U;
        }
        else {
            slot = &binding -> next;
        }
    }
}

int lm_message_thread_pool_delete(LmMessageThreadPool *pool) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    LmMessageThread * thread;
    LmMessageThreadRuntime * runtime;
    int lifecycle_active = 0;
    int join_failed = 0;
    size_t index;
    if (pool == 0) {
        return 0;
    }
    if (lm_native_message_thread_pool_lifecycle_allowed(pool) == 0) {
        return 1;
    }
    runtime = pool -> runtime;
    if (runtime == 0 || lm_mutex_lock(runtime -> route_mutex) != 0) {
        return 1;
    }
    if (lm_mutex_lock(pool -> mutex) != 0) {
        lm_mutex_unlock(runtime -> route_mutex);
        return 1;
    }
    if (pool -> deleting) {
        lm_mutex_unlock(pool -> mutex);
        lm_mutex_unlock(runtime -> route_mutex);
        return 1;
    }
    lm_native_message_thread_pool_stop_locked(pool);
    thread = pool -> member_head;
    while (thread != 0) {
        if (lm_native_message_thread_state_lock(thread) != 0) {
            abort();
        }
        if (thread -> executing || thread -> joining) {
            lifecycle_active = 1;
        }
        lm_native_message_thread_state_unlock(thread);
        thread = thread -> pool_next;
    }
    lm_condition_broadcast(pool -> work_ready);
    if (lifecycle_active) {
        lm_mutex_unlock(pool -> mutex);
        lm_mutex_unlock(runtime -> route_mutex);
        return 1;
    }
    pool->deleting = 1;
    lm_native_message_thread_remove_pool_routes_locked(runtime, pool);
    lm_mutex_unlock(pool -> mutex);
    lm_mutex_unlock(runtime -> route_mutex);
    if (pool -> single_mode) {
        while (lm_native_message_thread_pool_pump_one(pool) > 0) {
            LM_UNUSED(pool);
        }
    }
    else {
        index = 0U;
        while (index < pool -> started_worker_count) {
            if (pool -> workers[index] != 0 && ! pool -> workers[index] -> joined && lm_host_thread_join(pool -> workers[index], 0) != 0) {
                join_failed = 1;
            }
            index = index + 1U;
        }
    }
    if (join_failed) {
        if (lm_mutex_lock(pool -> mutex) == 0) {
            pool->deleting = 0;
            lm_mutex_unlock(pool -> mutex);
        }
        return 1;
    }
    index = 0U;
    while (index < pool -> worker_count) {
        lm_host_thread_delete(pool -> workers[index]);
        index = index + 1U;
    }
    free(pool -> workers);
    pool->workers = 0;
    thread = pool -> member_head;
    pool->member_head = 0;
    pool->member_tail = 0;
    pool->member_count = 0U;
    while (thread != 0) {
        LmMessageThread * next = thread -> pool_next;
        thread->pool = 0;
        thread->runtime = 0;
        thread->pool_previous = 0;
        thread->pool_next = 0;
        thread->ready_next = 0;
        thread->scheduled = 0;
        thread->executing = 0;
        thread->execution_identity = 0;
        lm_message_thread_delete(thread);
        thread = next;
    }
    lm_condition_delete(pool -> work_ready);
    lm_mutex_delete(pool -> mutex);
    lm_native_message_thread_runtime_release_pool(runtime);
    pool->runtime = 0;
    free(pool);
    return 0;
}

void * lm_own_new_zero(size_t size) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    return calloc(1U, size);
}

void * lm_own_resize(void *object, size_t size) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    if (size == 0U) {
        free(object);
        return 0;
    }
    return realloc(object, size);
}

char * lm_own_copy_bytes(const char *source, size_t length) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    char *copy;
    if (length == (((size_t)-1))) {
        return 0;
    }
    copy = lm_own_new_zero(length + 1U);
    if (copy == 0) {
        return 0;
    }
    if (source != 0 && length > 0U) {
        memcpy(copy, source, length);
    }
    copy[length] = '\0';
    return copy;
}

void lm_own_delete(void *object, LmOwnDestroyFields destroy_fields) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    if (object != 0) {
        if (destroy_fields != 0) {
            destroy_fields(object);
        }
        free(object);
    }
}

void lm_own_delete_plain(void *object) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    lm_own_delete(object, 0);
}

void lm_own_pointer_array_delete(void **items, size_t count, LmOwnDelete delete_item) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    size_t index;
    if (items != 0) {
        if (delete_item != 0) {
            index = 0U;
            while (index < count) {
                if (items[index] != 0) {
                    delete_item(items[index]);
                    items[index] = 0;
                }
                index = index + 1U;
            }
        }
        free(items);
    }
}

void lm_own_ptr_stack_init(LmOwnPtrStack *stack, LmOwnDelete delete_item) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    if (stack != 0) {
        stack->items = 0;
        stack->count = 0U;
        stack->capacity = 0U;
        stack->delete_item = delete_item;
    }
}

void lm_own_ptr_stack_destroy(LmOwnPtrStack *stack) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    if (stack != 0) {
        lm_own_pointer_array_delete(stack -> items, stack -> count, stack -> delete_item);
        stack->items = 0;
        stack->count = 0U;
        stack->capacity = 0U;
        stack->delete_item = 0;
    }
}

int lm_own_ptr_stack_push(LmOwnPtrStack *stack, void *item) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    void **items;
    size_t capacity;
    if (stack == 0) {
        return 1;
    }
    if (stack -> count == stack -> capacity) {
        if (stack -> capacity == 0U) {
            capacity = 8U;
        }
        if (stack -> capacity != 0U) {
            capacity = stack -> capacity * 2U;
        }
        items = realloc(stack -> items, capacity * sizeof(items[0]));
        if (items == 0) {
            return 1;
        }
        stack->items = items;
        stack->capacity = capacity;
    }
    stack->items[stack -> count] = item;
    stack->count = stack -> count + 1U;
    return 0;
}

void * lm_own_ptr_stack_pop(LmOwnPtrStack *stack) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    void *item;
    if (stack == 0 || stack -> count == 0U) {
        return 0;
    }
    stack->count = stack -> count - 1U;
    item = stack -> items[stack -> count];
    stack->items[stack -> count] = 0;
    return item;
}

void * lm_own_ptr_stack_at(const LmOwnPtrStack *stack, size_t index) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    if (stack == 0 || index >= stack -> count) {
        return 0;
    }
    return stack -> items[index];
}

void * lm_own_ptr_stack_top(const LmOwnPtrStack *stack) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    if (stack == 0 || stack -> count == 0U) {
        return 0;
    }
    return stack -> items[stack -> count - 1U];
}

void lm_own_ptr_stack_truncate(LmOwnPtrStack *stack, size_t count) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    void *item;
    if (stack == 0) {
        return;
    }
    while (stack -> count > count) {
        item = lm_own_ptr_stack_pop(stack);
        if (item != 0 && stack -> delete_item != 0) {
            stack->delete_item(item);
        }
    }
}

void lm_own_value_stack_init(LmOwnValueStack *stack, size_t item_size) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    if (stack != 0) {
        stack->items = 0;
        stack->count = 0U;
        stack->capacity = 0U;
        stack->item_size = item_size;
    }
}

void lm_own_value_stack_destroy(LmOwnValueStack *stack) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    if (stack != 0) {
        free(stack -> items);
        stack->items = 0;
        stack->count = 0U;
        stack->capacity = 0U;
        stack->item_size = 0U;
    }
}

void lm_own_value_stack_truncate(LmOwnValueStack *stack, size_t count) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    if (stack != 0 && count < stack -> count) {
        stack->count = count;
    }
}

static LmMessageThreadComponent * lm_message_thread_component_find(struct LmMessageThread *lm_lmx_message_thread, LmMessageThread *owner_thread, LmMessageThreadComponentDestroy destroy, int *out_valid) {
    (void)lm_lmx_message_thread;
    LmMessageThreadComponent * component;
    LmMessageThreadComponent * match;
    size_t visited;
    if (out_valid != 0) {
        out_valid[0] = 0;
    }
    if (owner_thread == 0 || destroy == 0 || out_valid == 0 || lm_lmx_message_thread == 0 || owner_thread != lm_lmx_message_thread) {
        return 0;
    }
    if (owner_thread -> component_count == 0U) {
        if (owner_thread -> component_head != 0) {
            return 0;
        }
        out_valid[0] = 1;
        return 0;
    }
    if (owner_thread -> component_head == 0) {
        return 0;
    }
    component = owner_thread -> component_head;
    match = 0;
    visited = 0U;
    while (component != 0) {
        if (visited >= owner_thread -> component_count || component -> owner_thread != owner_thread || component -> destroy == 0) {
            return 0;
        }
        if (component -> destroy == destroy) {
            if (match != 0) {
                return 0;
            }
            match = component;
        }
        component = component -> next;
        visited = visited + 1U;
    }
    if (visited != owner_thread -> component_count) {
        return 0;
    }
    out_valid[0] = 1;
    return match;
}

static int lm_message_thread_component_detach(struct LmMessageThread *lm_lmx_message_thread, LmMessageThread *owner_thread, LmMessageThreadComponent *component) {
    (void)lm_lmx_message_thread;
    LmMessageThreadComponent * current;
    LmMessageThreadComponent * previous;
    LmMessageThreadComponent * found_previous;
    size_t visited;
    if (owner_thread == 0 || component == 0 || lm_lmx_message_thread == 0 || owner_thread != lm_lmx_message_thread || component -> owner_thread != owner_thread || owner_thread -> component_count == 0U) {
        return 1;
    }
    current = owner_thread -> component_head;
    previous = 0;
    found_previous = 0;
    visited = 0U;
    while (current != 0) {
        if (visited >= owner_thread -> component_count || current -> owner_thread != owner_thread || current -> destroy == 0) {
            return 1;
        }
        if (current == component) {
            found_previous = previous;
        }
        previous = current;
        current = current -> next;
        visited = visited + 1U;
    }
    if (visited != owner_thread -> component_count || (component != owner_thread -> component_head && found_previous == 0)) {
        return 1;
    }
    if (found_previous != 0) {
        found_previous->next = component -> next;
    }
    else {
        owner_thread->component_head = component -> next;
    }
    owner_thread->component_count = owner_thread -> component_count - 1U;
    component->owner_thread = 0;
    component->next = 0;
    return 0;
}

static int lm_message_thread_component_release(struct LmMessageThread *lm_lmx_message_thread, LmMessageThread *owner_thread, LmMessageThreadComponent *component) {
    (void)lm_lmx_message_thread;
    void *value;
    if (owner_thread == 0 || component == 0 || lm_lmx_message_thread == 0 || owner_thread != lm_lmx_message_thread || component -> owner_thread != owner_thread || component -> destroy == 0) {
        return 1;
    }
    value = component -> value;
    if (lm_message_thread_component_detach(lm_lmx_message_thread, owner_thread, component) != 0) {
        return 1;
    }
    component->value = 0;
    if (component -> destroy != 0) {
        component->destroy(owner_thread, value);
    }
    component->destroy = 0;
    lm_own_delete(component, 0);
    return 0;
}

int lm_message_thread_component_attach(struct LmMessageThread *lm_lmx_message_thread, LmMessageThreadComponentDestroy destroy, void *component) {
    (void)lm_lmx_message_thread;
    LmMessageThreadComponent * entry;
    LmMessageThread * thread;
    int valid;
    thread = lm_lmx_message_thread;
    if (thread == 0 || destroy == 0 || component == 0 || thread -> state != LM_MESSAGE_THREAD_RUNNING || thread -> component_destroying || thread -> component_count == (((size_t)-1))) {
        return 1;
    }
    valid = 0;
    if (lm_message_thread_component_find(lm_lmx_message_thread, thread, destroy, &valid) != 0 || valid == 0) {
        return 1;
    }
    entry = lm_own_new_zero(sizeof(entry[0]));
    if (entry == 0) {
        return 1;
    }
    entry->owner_thread = thread;
    entry->value = component;
    entry->destroy = destroy;
    entry->next = thread -> component_head;
    thread->component_head = entry;
    thread->component_count = thread -> component_count + 1U;
    return 0;
}

void * lm_message_thread_component_get(struct LmMessageThread *lm_lmx_message_thread, LmMessageThreadComponentDestroy destroy) {
    (void)lm_lmx_message_thread;
    LmMessageThreadComponent * component;
    LmMessageThread * thread;
    int valid;
    thread = lm_lmx_message_thread;
    valid = 0;
    component = lm_message_thread_component_find(lm_lmx_message_thread, thread, destroy, &valid);
    if (valid == 0 || component == 0) {
        return 0;
    }
    return component -> value;
}

int lm_message_thread_component_remove(struct LmMessageThread *lm_lmx_message_thread, LmMessageThreadComponentDestroy destroy) {
    (void)lm_lmx_message_thread;
    LmMessageThreadComponent * component;
    LmMessageThread * thread;
    int valid;
    thread = lm_lmx_message_thread;
    if (thread == 0 || destroy == 0 || thread -> component_destroying) {
        return 1;
    }
    valid = 0;
    component = lm_message_thread_component_find(lm_lmx_message_thread, thread, destroy, &valid);
    if (valid == 0 || component == 0) {
        return 1;
    }
    return lm_message_thread_component_release(lm_lmx_message_thread, thread, component);
}

size_t lm_message_thread_component_count(const LmMessageThread *thread) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    if (thread == 0) {
        return 0U;
    }
    return thread -> component_count;
}

static int lm_own_arena_shell_is_zero(struct LmMessageThread *lm_lmx_message_thread, const LmOwnArena *arena) {
    (void)lm_lmx_message_thread;
    if (arena == 0) {
        return 0;
    }
    return arena -> allocations == 0 && arena -> allocation_descriptors == 0 && arena -> lazy_edges == 0 && arena -> frozen == 0 && arena -> owner_thread == 0 && arena -> registry_previous == 0 && arena -> registry_next == 0 && arena -> runtime_owned_shell == 0 && arena -> roots == 0;
}

static int lm_own_arena_register(struct LmMessageThread *lm_lmx_message_thread, LmMessageThread *owner_thread, LmOwnArena *arena) {
    (void)lm_lmx_message_thread;
    LmOwnArena * tail;
    if (owner_thread == 0 || arena == 0 || lm_lmx_message_thread == 0 || owner_thread != lm_lmx_message_thread || owner_thread -> arena_destroying) {
        return 1;
    }
    if (arena -> owner_thread != 0 || arena -> registry_previous != 0 || arena -> registry_next != 0) {
        return 1;
    }
    if (owner_thread -> arena_count == (((size_t)-1))) {
        return 1;
    }
    tail = owner_thread -> arena_tail;
    if (owner_thread -> arena_count == 0U) {
        if (owner_thread -> arena_head != 0 || tail != 0) {
            return 1;
        }
    }
    else {
        if (owner_thread -> arena_head == 0 || tail == 0 || owner_thread -> arena_head -> registry_previous != 0 || tail -> registry_next != 0 || owner_thread -> arena_head -> owner_thread != owner_thread || tail -> owner_thread != owner_thread) {
            return 1;
        }
    }
    arena->owner_thread = owner_thread;
    arena->registry_previous = tail;
    arena->registry_next = 0;
    if (tail != 0) {
        tail->registry_next = arena;
    }
    else {
        owner_thread->arena_head = arena;
    }
    owner_thread->arena_tail = arena;
    owner_thread->arena_count = owner_thread -> arena_count + 1U;
    return 0;
}

static int lm_own_arena_detach(struct LmMessageThread *lm_lmx_message_thread, LmMessageThread *owner_thread, LmOwnArena *arena) {
    (void)lm_lmx_message_thread;
    LmOwnArena * previous;
    LmOwnArena * next;
    if (owner_thread == 0 || arena == 0 || lm_lmx_message_thread == 0 || owner_thread != lm_lmx_message_thread || arena -> owner_thread != owner_thread || owner_thread -> arena_count == 0U) {
        return 1;
    }
    previous = arena -> registry_previous;
    next = arena -> registry_next;
    if (previous == 0) {
        if (owner_thread -> arena_head != arena) {
            return 1;
        }
    }
    else {
        if (previous -> owner_thread != owner_thread || previous -> registry_next != arena) {
            return 1;
        }
    }
    if (next == 0) {
        if (owner_thread -> arena_tail != arena) {
            return 1;
        }
    }
    else {
        if (next -> owner_thread != owner_thread || next -> registry_previous != arena) {
            return 1;
        }
    }
    if (previous != 0) {
        previous->registry_next = next;
    }
    else {
        owner_thread->arena_head = next;
    }
    if (next != 0) {
        next->registry_previous = previous;
    }
    else {
        owner_thread->arena_tail = previous;
    }
    owner_thread->arena_count = owner_thread -> arena_count - 1U;
    if (owner_thread -> root_owner == arena) {
        owner_thread->root_owner = 0;
    }
    arena->registry_previous = 0;
    arena->registry_next = 0;
    arena->owner_thread = 0;
    return 0;
}

static int lm_own_arena_init_registered(struct LmMessageThread *lm_lmx_message_thread, LmOwnArena *arena, LmMessageThread *owner_thread, int runtime_owned_shell) {
    (void)lm_lmx_message_thread;
    LmOwnPtrStack * allocations;
    LmOwnPtrStack * allocation_descriptors;
    LmOwnPtrStack * lazy_edges;
    LmOwnPtrStack * roots;
    if (arena == 0 || owner_thread == 0 || lm_lmx_message_thread == 0 || owner_thread != lm_lmx_message_thread || lm_own_arena_shell_is_zero(lm_lmx_message_thread, arena) == 0) {
        return 1;
    }
    allocations = lm_own_new_zero(sizeof(allocations[0]));
    allocation_descriptors = lm_own_new_zero(sizeof(allocation_descriptors[0]));
    lazy_edges = lm_own_new_zero(sizeof(lazy_edges[0]));
    roots = lm_own_new_zero(sizeof(roots[0]));
    if (allocations == 0 || allocation_descriptors == 0 || lazy_edges == 0 || roots == 0) {
        lm_own_delete(roots, 0);
        lm_own_delete(lazy_edges, 0);
        lm_own_delete(allocation_descriptors, 0);
        lm_own_delete(allocations, 0);
        return 1;
    }
    lm_own_ptr_stack_init(allocations, free);
    lm_own_ptr_stack_init(allocation_descriptors, lm_own_delete_plain);
    lm_own_ptr_stack_init(lazy_edges, lm_own_delete_plain);
    lm_own_ptr_stack_init(roots, 0);
    arena->allocations = allocations;
    arena->allocation_descriptors = allocation_descriptors;
    arena->lazy_edges = lazy_edges;
    arena->roots = roots;
    arena->frozen = 0;
    arena->runtime_owned_shell = runtime_owned_shell != 0;
    if (lm_own_arena_register(lm_lmx_message_thread, owner_thread, arena) != 0) {
        lm_own_ptr_stack_destroy(arena -> roots);
        lm_own_delete(arena -> roots, 0);
        lm_own_ptr_stack_destroy(arena -> lazy_edges);
        lm_own_delete(arena -> lazy_edges, 0);
        lm_own_ptr_stack_destroy(arena -> allocation_descriptors);
        lm_own_delete(arena -> allocation_descriptors, 0);
        lm_own_ptr_stack_destroy(arena -> allocations);
        lm_own_delete(arena -> allocations, 0);
        memset(arena, 0, sizeof(arena[0]));
        return 1;
    }
    return 0;
}

static int lm_own_arena_release(struct LmMessageThread *lm_lmx_message_thread, LmMessageThread *owner_thread, LmOwnArena *arena, int delete_shell, int teardown_mode) {
    (void)lm_lmx_message_thread;
    int runtime_owned_shell;
    if (owner_thread == 0 || arena == 0 || lm_lmx_message_thread == 0 || owner_thread != lm_lmx_message_thread || arena -> owner_thread != owner_thread) {
        return 1;
    }
    if (teardown_mode == 0) {
        if (arena == owner_thread -> root_owner) {
            lm_message_thread_request_failure(owner_thread, 1);
            return 1;
        }
        if ((delete_shell != 0) != (arena -> runtime_owned_shell != 0)) {
            return 1;
        }
    }
    runtime_owned_shell = arena -> runtime_owned_shell;
    if (lm_own_arena_detach(lm_lmx_message_thread, owner_thread, arena) != 0) {
        return 1;
    }
    lm_own_ptr_stack_destroy(arena -> roots);
    lm_own_delete(arena -> roots, 0);
    lm_own_ptr_stack_destroy(arena -> lazy_edges);
    lm_own_delete(arena -> lazy_edges, 0);
    lm_own_ptr_stack_destroy(arena -> allocation_descriptors);
    lm_own_delete(arena -> allocation_descriptors, 0);
    lm_own_ptr_stack_destroy(arena -> allocations);
    lm_own_delete(arena -> allocations, 0);
    memset(arena, 0, sizeof(arena[0]));
    if (runtime_owned_shell) {
        lm_own_delete(arena, 0);
    }
    return 0;
}

int lm_own_arena_init(struct LmMessageThread *lm_lmx_message_thread, LmOwnArena *arena, LmMessageThread *owner_thread) {
    (void)lm_lmx_message_thread;
    return lm_own_arena_init_registered(lm_lmx_message_thread, arena, owner_thread, 0);
}

void lm_own_arena_destroy(struct LmMessageThread *lm_lmx_message_thread, LmOwnArena *arena) {
    (void)lm_lmx_message_thread;
    if (arena != 0 && lm_lmx_message_thread != 0) {
        lm_own_arena_release(lm_lmx_message_thread, lm_lmx_message_thread, arena, 0, 0);
    }
}

LmOwnArena * lm_own_arena_new(struct LmMessageThread *lm_lmx_message_thread, LmMessageThread *owner_thread) {
    (void)lm_lmx_message_thread;
    LmOwnArena * arena;
    if (owner_thread == 0 || lm_lmx_message_thread == 0 || owner_thread != lm_lmx_message_thread) {
        return 0;
    }
    arena = lm_own_new_zero(sizeof(arena[0]));
    if (arena == 0) {
        return 0;
    }
    if (lm_own_arena_init_registered(lm_lmx_message_thread, arena, owner_thread, 1) != 0) {
        lm_own_delete(arena, 0);
        return 0;
    }
    return arena;
}

void lm_own_arena_delete(struct LmMessageThread *lm_lmx_message_thread, LmOwnArena *arena) {
    (void)lm_lmx_message_thread;
    if (arena != 0 && lm_lmx_message_thread != 0) {
        lm_own_arena_release(lm_lmx_message_thread, lm_lmx_message_thread, arena, 1, 0);
    }
}

void lm_own_arena_freeze(struct LmMessageThread *lm_lmx_message_thread, LmOwnArena *arena) {
    (void)lm_lmx_message_thread;
    if (arena != 0 && lm_lmx_message_thread != 0 && arena -> owner_thread == lm_lmx_message_thread) {
        arena->frozen = 1;
    }
}

int lm_own_arena_is_frozen(struct LmMessageThread *lm_lmx_message_thread, const LmOwnArena *arena) {
    (void)lm_lmx_message_thread;
    return arena != 0 && lm_lmx_message_thread != 0 && arena -> owner_thread == lm_lmx_message_thread && arena -> frozen;
}

LmMessageThread * lm_own_arena_owner_thread(struct LmMessageThread *lm_lmx_message_thread, const LmOwnArena *arena) {
    (void)lm_lmx_message_thread;
    if (arena == 0 || lm_lmx_message_thread == 0 || arena -> owner_thread != lm_lmx_message_thread) {
        return 0;
    }
    return arena -> owner_thread;
}

int lm_own_tree_cut(struct LmMessageThread *lm_lmx_message_thread, LmOwnArena *arena) {
    (void)lm_lmx_message_thread;
    if (arena == 0 || lm_lmx_message_thread == 0 || arena -> owner_thread != lm_lmx_message_thread) {
        return 1;
    }
    return lm_own_tree_cut_promote_lazy_edges(lm_lmx_message_thread, arena);
}

int lm_own_tree_cut_promote_lazy_edges(struct LmMessageThread *lm_lmx_message_thread, LmOwnArena *arena) {
    (void)lm_lmx_message_thread;
    if (arena == 0 || lm_lmx_message_thread == 0 || arena -> owner_thread != lm_lmx_message_thread) {
        return 1;
    }
    return lm_own_arena_promote_lazy_edges(lm_lmx_message_thread, arena);
}

const char * lm_thread_provider_name(void) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    return lm_native_thread_provider_name();
}

LmHostThread * lm_host_thread_new(void) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    LmHostThread * thread;
    thread = lm_own_new_zero(sizeof(LmHostThread));
    if (thread == 0) {
        return 0;
    }
    thread->controller_identity = lm_native_thread_identity_new_current();
    if (thread -> controller_identity == 0) {
        lm_own_delete(thread, 0);
        return 0;
    }
    return thread;
}

void lm_host_thread_delete(LmHostThread *thread) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    void *ignored_result;
    if (thread != 0 && lm_native_thread_identity_is_current(thread -> controller_identity)) {
        if (thread -> starting) {
            return;
        }
        if (thread -> started && thread -> joined == 0 && thread -> implementation != 0) {
            if (lm_native_host_thread_join(thread -> implementation, &ignored_result) == 0) {
                thread->implementation = 0;
                thread->joined = 1;
            }
        }
        if (thread -> implementation == 0) {
            lm_native_thread_identity_delete(thread -> controller_identity);
            thread->controller_identity = 0;
            lm_own_delete(thread, 0);
        }
    }
}

int lm_host_thread_start(LmHostThread *thread, LmHostThreadEntry entry, void *argument) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    int status;
    if (thread == 0 || entry == 0) {
        return 1;
    }
    if (lm_native_thread_identity_is_current(thread -> controller_identity) == 0) {
        return 1;
    }
    if (thread -> starting || thread -> started || thread -> implementation != 0) {
        return 1;
    }
    status = 1;
    thread->starting = 1;
    thread->implementation = lm_native_host_thread_start(entry, argument, &status);
    thread->starting = 0;
    if (status != 0 || thread -> implementation == 0) {
        thread->implementation = 0;
        return 1;
    }
    thread->started = 1;
    thread->joined = 0;
    return 0;
}

int lm_host_thread_join(LmHostThread *thread, void **result) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    int status;
    if (thread == 0) {
        return 1;
    }
    if (lm_native_thread_identity_is_current(thread -> controller_identity) == 0) {
        return 1;
    }
    if (thread -> starting || thread -> started == 0 || thread -> joined || thread -> implementation == 0) {
        return 1;
    }
    status = lm_native_host_thread_join(thread -> implementation, result);
    if (status != 0) {
        return status;
    }
    thread->implementation = 0;
    thread->joined = 1;
    return 0;
}

LmMutex * lm_mutex_new(void) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    LmMutex * mutex;
    mutex = lm_own_new_zero(sizeof(LmMutex));
    if (mutex == 0) {
        return 0;
    }
    mutex->implementation = lm_native_mutex_new();
    if (mutex -> implementation == 0) {
        lm_own_delete(mutex, 0);
        return 0;
    }
    return mutex;
}

void lm_mutex_delete(LmMutex *mutex) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    if (mutex != 0 && lm_native_mutex_delete(mutex -> implementation) == 0) {
        mutex->implementation = 0;
        lm_own_delete(mutex, 0);
    }
}

int lm_mutex_lock(LmMutex *mutex) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    if (mutex == 0) {
        return 1;
    }
    return lm_native_mutex_lock(mutex -> implementation);
}

int lm_mutex_unlock(LmMutex *mutex) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    if (mutex == 0) {
        return 1;
    }
    return lm_native_mutex_unlock(mutex -> implementation);
}

LmCondition * lm_condition_new(void) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    LmCondition * condition;
    condition = lm_own_new_zero(sizeof(LmCondition));
    if (condition == 0) {
        return 0;
    }
    condition->implementation = lm_native_condition_new();
    if (condition -> implementation == 0) {
        lm_own_delete(condition, 0);
        return 0;
    }
    return condition;
}

void lm_condition_delete(LmCondition *condition) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    if (condition != 0 && lm_native_condition_delete(condition -> implementation) == 0) {
        condition->implementation = 0;
        lm_own_delete(condition, 0);
    }
}

int lm_condition_wait(LmCondition *condition, LmMutex *mutex) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    if (condition == 0 || mutex == 0) {
        return 1;
    }
    return lm_native_condition_wait(condition -> implementation, mutex -> implementation);
}

int lm_condition_signal(LmCondition *condition) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    if (condition == 0) {
        return 1;
    }
    return lm_native_condition_signal(condition -> implementation);
}

int lm_condition_broadcast(LmCondition *condition) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    if (condition == 0) {
        return 1;
    }
    return lm_native_condition_broadcast(condition -> implementation);
}

int lm_own_value_stack_push(LmOwnValueStack *stack, const void *item) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    unsigned char *items;
    unsigned char *target;
    size_t capacity;
    if (stack == 0 || item == 0 || stack -> item_size == 0U) {
        return 1;
    }
    if (stack -> count == stack -> capacity) {
        if (stack -> capacity == 0U) {
            capacity = 8U;
        }
        if (stack -> capacity != 0U) {
            capacity = stack -> capacity * 2U;
        }
        items = ((unsigned char *)realloc(stack -> items, capacity * stack -> item_size));
        if (items == 0) {
            return 1;
        }
        stack->items = ((void *)items);
        stack->capacity = capacity;
    }
    target = (((unsigned char *)stack -> items)) + stack -> count * stack -> item_size;
    memcpy(target, item, stack -> item_size);
    stack->count = stack -> count + 1U;
    return 0;
}

int lm_own_value_stack_resize_zero(LmOwnValueStack *stack, size_t count) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    unsigned char *items;
    unsigned char *target;
    size_t capacity;
    size_t previous_count;
    if (stack == 0 || stack -> item_size == 0U) {
        return 1;
    }
    if (count > stack -> capacity) {
        if (stack -> capacity == 0U) {
            capacity = 8U;
        }
        if (stack -> capacity != 0U) {
            capacity = stack -> capacity;
        }
        while (capacity < count) {
            capacity = capacity * 2U;
        }
        items = ((unsigned char *)realloc(stack -> items, capacity * stack -> item_size));
        if (items == 0) {
            return 1;
        }
        stack->items = ((void *)items);
        stack->capacity = capacity;
    }
    previous_count = stack -> count;
    if (count > previous_count) {
        target = (((unsigned char *)stack -> items)) + previous_count * stack -> item_size;
        memset(target, 0, (count - previous_count) * stack -> item_size);
    }
    stack->count = count;
    return 0;
}

int lm_own_value_stack_pop(LmOwnValueStack *stack, void *out_item) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    unsigned char *source;
    if (stack == 0 || stack -> count == 0U || stack -> item_size == 0U) {
        return 1;
    }
    stack->count = stack -> count - 1U;
    source = (((unsigned char *)stack -> items)) + stack -> count * stack -> item_size;
    if (out_item != 0) {
        memcpy(out_item, source, stack -> item_size);
    }
    return 0;
}

void * lm_own_value_stack_at(const LmOwnValueStack *stack, size_t index) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    unsigned char *item;
    if (stack == 0 || index >= stack -> count || stack -> item_size == 0U) {
        return 0;
    }
    item = (((unsigned char *)stack -> items)) + index * stack -> item_size;
    return item;
}

void * lm_own_value_stack_top(const LmOwnValueStack *stack) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    unsigned char *item;
    if (stack == 0 || stack -> count == 0U || stack -> item_size == 0U) {
        return 0;
    }
    item = (((unsigned char *)stack -> items)) + (stack -> count - 1U) * stack -> item_size;
    return item;
}

static int lm_own_size_multiply(struct LmMessageThread *lm_lmx_message_thread, size_t left, size_t right, size_t *out) {
    (void)lm_lmx_message_thread;
    if (out == 0) {
        return 1;
    }
    if (left != 0U && right > (((size_t)-1)) / left) {
        return 1;
    }
    out[0] = left * right;
    return 0;
}

static const LmOwnAllocationDescriptor * lm_own_allocation_descriptor_find(struct LmMessageThread *lm_lmx_message_thread, const LmOwnPtrStack *descriptors, const void *address) {
    (void)lm_lmx_message_thread;
    const LmOwnAllocationDescriptor * descriptor;
    size_t index;
    if (descriptors == 0 || address == 0) {
        return 0;
    }
    index = descriptors -> count;
    while (index > 0U) {
        index = index - 1U;
        descriptor = lm_own_ptr_stack_at(descriptors, index);
        if (descriptor != 0 && descriptor -> address == address) {
            return descriptor;
        }
    }
    return 0;
}

static int lm_own_allocation_descriptor_push(struct LmMessageThread *lm_lmx_message_thread, LmOwnPtrStack *descriptors, void *address, LmOwnArena *owner, size_t bytes, size_t element_size, size_t count, size_t rank, size_t level) {
    (void)lm_lmx_message_thread;
    LmOwnAllocationDescriptor * descriptor;
    int status;
    if (descriptors == 0 || address == 0) {
        return 1;
    }
    descriptor = lm_own_new_zero(sizeof(LmOwnAllocationDescriptor));
    if (descriptor == 0) {
        return 1;
    }
    descriptor->address = address;
    descriptor->owner = owner;
    descriptor->bytes = bytes;
    descriptor->element_size = element_size;
    descriptor->count = count;
    descriptor->rank = rank;
    descriptor->level = level;
    descriptor->marked = 0;
    descriptor->pinned = 0;
    status = lm_own_ptr_stack_push(descriptors, descriptor);
    if (status != 0) {
        lm_own_delete(descriptor, 0);
    }
    return status;
}

void * lm_own_arena_new_zero(struct LmMessageThread *lm_lmx_message_thread, LmOwnArena *arena, size_t size) {
    (void)lm_lmx_message_thread;
    void *object;
    if (arena == 0 || lm_lmx_message_thread == 0 || arena -> owner_thread != lm_lmx_message_thread || arena -> frozen || arena -> allocations == 0 || arena -> allocation_descriptors == 0) {
        return 0;
    }
    object = calloc(1U, size);
    if (object == 0) {
        return 0;
    }
    if (lm_own_ptr_stack_push(arena -> allocations, object) != 0) {
        free(object);
        return 0;
    }
    if (lm_own_allocation_descriptor_push(lm_lmx_message_thread, arena -> allocation_descriptors, object, arena, size, size, 1U, 0U, 0U) != 0) {
        lm_own_ptr_stack_pop(arena -> allocations);
        free(object);
        return 0;
    }
    return object;
}

void * lm_own_arena_array_new_zero(struct LmMessageThread *lm_lmx_message_thread, LmOwnArena *arena, size_t element_size, size_t count, size_t rank, size_t level) {
    (void)lm_lmx_message_thread;
    void *object;
    size_t bytes;
    if (arena == 0 || lm_lmx_message_thread == 0 || arena -> owner_thread != lm_lmx_message_thread || arena -> frozen || arena -> allocations == 0 || arena -> allocation_descriptors == 0) {
        return 0;
    }
    if (lm_own_size_multiply(lm_lmx_message_thread, element_size, count, &bytes) != 0) {
        return 0;
    }
    if (bytes == 0U) {
        return 0;
    }
    object = calloc(1U, bytes);
    if (object == 0) {
        return 0;
    }
    if (lm_own_ptr_stack_push(arena -> allocations, object) != 0) {
        free(object);
        return 0;
    }
    if (lm_own_allocation_descriptor_push(lm_lmx_message_thread, arena -> allocation_descriptors, object, arena, bytes, element_size, count, rank, level) != 0) {
        lm_own_ptr_stack_pop(arena -> allocations);
        free(object);
        return 0;
    }
    return object;
}

const LmOwnAllocationDescriptor * lm_own_arena_allocation_descriptor(struct LmMessageThread *lm_lmx_message_thread, const LmOwnArena *arena, const void *address) {
    (void)lm_lmx_message_thread;
    if (arena == 0 || lm_lmx_message_thread == 0 || arena -> owner_thread != lm_lmx_message_thread || arena -> allocation_descriptors == 0) {
        return 0;
    }
    return lm_own_allocation_descriptor_find(lm_lmx_message_thread, arena -> allocation_descriptors, address);
}

char * lm_own_arena_copy_bytes(struct LmMessageThread *lm_lmx_message_thread, LmOwnArena *arena, const char *source, size_t length) {
    (void)lm_lmx_message_thread;
    char *copy;
    if (arena == 0 || lm_lmx_message_thread == 0 || arena -> owner_thread != lm_lmx_message_thread) {
        return 0;
    }
    copy = lm_own_arena_new_zero(lm_lmx_message_thread, arena, length + 1U);
    if (copy == 0) {
        return 0;
    }
    if (source != 0 && length > 0U) {
        memcpy(copy, source, length);
    }
    copy[length] = '\0';
    return copy;
}

int lm_own_arena_add_lazy_edge(struct LmMessageThread *lm_lmx_message_thread, LmOwnArena *target, LmOwnArena *source, const void *source_ptr, size_t size, const void **patch_slot) {
    (void)lm_lmx_message_thread;
    LmOwnLazyEdge * edge;
    int status;
    if (target == 0 || source == 0 || patch_slot == 0 || lm_lmx_message_thread == 0 || target -> owner_thread != lm_lmx_message_thread || source -> owner_thread != lm_lmx_message_thread || target -> frozen || target -> lazy_edges == 0) {
        return 1;
    }
    if (size == 0U) {
        return 0;
    }
    if (source_ptr == 0) {
        return 1;
    }
    edge = lm_own_new_zero(sizeof(LmOwnLazyEdge));
    if (edge == 0) {
        return 1;
    }
    edge->kind = LM_OWN_EDGE_LAZY_OWNED;
    edge->source_owner = source;
    edge->target_owner = target;
    edge->source = source_ptr;
    edge->size = size;
    edge->patch_slot = patch_slot;
    status = lm_own_ptr_stack_push(target -> lazy_edges, edge);
    if (status != 0) {
        lm_own_delete(edge, 0);
    }
    return status;
}

int lm_own_arena_promote_lazy_edges(struct LmMessageThread *lm_lmx_message_thread, LmOwnArena *arena) {
    (void)lm_lmx_message_thread;
    LmOwnLazyEdge * edge;
    void *copy;
    size_t i;
    if (arena == 0 || lm_lmx_message_thread == 0 || arena -> owner_thread != lm_lmx_message_thread || arena -> frozen || arena -> lazy_edges == 0) {
        return 1;
    }
    i = 0U;
    while (i < arena -> lazy_edges -> count) {
        edge = lm_own_ptr_stack_at(arena -> lazy_edges, i);
        if (edge == 0 || edge -> kind != LM_OWN_EDGE_LAZY_OWNED) {
            i = i + 1U;
            continue;
        }
        if (edge -> patch_slot == 0 || edge -> source == 0 || edge -> size == 0U) {
            return 1;
        }
        copy = lm_own_arena_new_zero(lm_lmx_message_thread, arena, edge -> size + 1U);
        if (copy == 0) {
            return 1;
        }
        memcpy(copy, edge -> source, edge -> size);
        edge->patch_slot[0] = copy;
        edge->source = copy;
        edge->source_owner = arena;
        edge->target_owner = arena;
        edge->kind = LM_OWN_EDGE_OWNED;
        i = i + 1U;
    }
    return 0;
}

static void * lm_own_copy_visit_find(struct LmMessageThread *lm_lmx_message_thread, const LmOwnPtrStack *sources, const LmOwnPtrStack *copies, void *source) {
    (void)lm_lmx_message_thread;
    size_t index;
    if (sources == 0 || copies == 0 || source == 0) {
        return 0;
    }
    index = 0U;
    while (index < sources -> count) {
        if (lm_own_ptr_stack_at(sources, index) == source) {
            return lm_own_ptr_stack_at(copies, index);
        }
        index = index + 1U;
    }
    return 0;
}

static void * lm_own_arena_copy_graph_into(struct LmMessageThread *lm_lmx_message_thread, LmOwnArena *arena, void *source, LmOwnPtrStack *sources, LmOwnPtrStack *copies) {
    (void)lm_lmx_message_thread;
    const LmOwnAllocationDescriptor * descriptor;
    void *copy;
    void *child;
    void *copied;
    unsigned char *raw;
    void *slot;
    size_t pointer_size;
    size_t offset;
    if (arena == 0 || source == 0) {
        return source;
    }
    copy = lm_own_copy_visit_find(lm_lmx_message_thread, sources, copies, source);
    if (copy != 0) {
        return copy;
    }
    descriptor = lm_own_allocation_descriptor_find(lm_lmx_message_thread, arena -> allocation_descriptors, source);
    if (descriptor == 0) {
        return source;
    }
    copy = lm_own_arena_new_zero(lm_lmx_message_thread, arena, descriptor -> bytes);
    if (copy == 0) {
        return 0;
    }
    memcpy(copy, source, descriptor -> bytes);
    if (lm_own_ptr_stack_push(sources, source) != 0 || lm_own_ptr_stack_push(copies, copy) != 0) {
        return 0;
    }
    pointer_size = sizeof(slot);
    if (pointer_size == 0U) {
        return copy;
    }
    raw = ((unsigned char *)copy);
    offset = 0U;
    while (offset + pointer_size <= descriptor -> bytes) {
        memcpy(&child, raw + offset, pointer_size);
        if (child != 0 && lm_own_allocation_descriptor_find(lm_lmx_message_thread, arena -> allocation_descriptors, child) != 0) {
            copied = lm_own_arena_copy_graph_into(lm_lmx_message_thread, arena, child, sources, copies);
            memcpy(raw + offset, &copied, pointer_size);
        }
        offset = offset + pointer_size;
    }
    return copy;
}

static void lm_own_arena_mark(struct LmMessageThread *lm_lmx_message_thread, LmOwnArena *arena, void *object) {
    (void)lm_lmx_message_thread;
    LmOwnAllocationDescriptor * descriptor;
    void *child;
    void *slot;
    unsigned char *raw;
    size_t pointer_size;
    size_t offset;
    if (arena == 0 || object == 0 || arena -> allocation_descriptors == 0) {
        return;
    }
    descriptor = ((LmOwnAllocationDescriptor *)lm_own_allocation_descriptor_find(lm_lmx_message_thread, arena -> allocation_descriptors, object));
    if (descriptor == 0 || descriptor -> marked) {
        return;
    }
    descriptor->marked = 1;
    pointer_size = sizeof(slot);
    if (pointer_size == 0U || descriptor -> bytes < pointer_size) {
        return;
    }
    raw = ((unsigned char *)object);
    offset = 0U;
    while (offset + pointer_size <= descriptor -> bytes) {
        memcpy(&child, raw + offset, pointer_size);
        if (child != 0) {
            lm_own_arena_mark(lm_lmx_message_thread, arena, child);
        }
        offset = offset + pointer_size;
    }
}

int lm_own_arena_pin(struct LmMessageThread *lm_lmx_message_thread, LmOwnArena *arena, void *address) {
    (void)lm_lmx_message_thread;
    LmOwnAllocationDescriptor * descriptor;
    if (arena == 0 || address == 0 || lm_lmx_message_thread == 0 || arena -> owner_thread != lm_lmx_message_thread || arena -> allocation_descriptors == 0) {
        return 1;
    }
    descriptor = ((LmOwnAllocationDescriptor *)lm_own_allocation_descriptor_find(lm_lmx_message_thread, arena -> allocation_descriptors, address));
    if (descriptor == 0) {
        return 1;
    }
    descriptor->pinned = 1;
    return 0;
}

int lm_own_arena_root_add(struct LmMessageThread *lm_lmx_message_thread, LmOwnArena *arena, void *address) {
    (void)lm_lmx_message_thread;
    size_t index;
    if (arena == 0 || address == 0 || lm_lmx_message_thread == 0 || arena -> owner_thread != lm_lmx_message_thread || arena -> roots == 0) {
        return 1;
    }
    index = 0U;
    while (index < arena -> roots -> count) {
        if (lm_own_ptr_stack_at(arena -> roots, index) == address) {
            return 0;
        }
        index = index + 1U;
    }
    return lm_own_ptr_stack_push(arena -> roots, address);
}

void * lm_own_arena_copy_graph(struct LmMessageThread *lm_lmx_message_thread, LmOwnArena *arena, const void *source) {
    (void)lm_lmx_message_thread;
    LmOwnPtrStack * sources;
    LmOwnPtrStack * copies;
    void *copy;
    void *mutable_source;
    if (arena == 0 || lm_lmx_message_thread == 0 || arena -> owner_thread != lm_lmx_message_thread || arena -> frozen) {
        return 0;
    }
    if (source == 0) {
        return 0;
    }
    mutable_source = ((void *)source);
    sources = lm_own_new_zero(sizeof(sources[0]));
    copies = lm_own_new_zero(sizeof(copies[0]));
    if (sources == 0 || copies == 0) {
        lm_own_delete(sources, 0);
        lm_own_delete(copies, 0);
        return 0;
    }
    lm_own_ptr_stack_init(sources, 0);
    lm_own_ptr_stack_init(copies, 0);
    copy = lm_own_arena_copy_graph_into(lm_lmx_message_thread, arena, mutable_source, sources, copies);
    lm_own_ptr_stack_destroy(sources);
    lm_own_ptr_stack_destroy(copies);
    lm_own_delete(sources, 0);
    lm_own_delete(copies, 0);
    return copy;
}

int lm_own_arena_reclaim(struct LmMessageThread *lm_lmx_message_thread, LmOwnArena *arena) {
    (void)lm_lmx_message_thread;
    LmOwnAllocationDescriptor * descriptor;
    void *object;
    void *item;
    size_t index;
    size_t keep;
    if (arena == 0 || lm_lmx_message_thread == 0 || arena -> owner_thread != lm_lmx_message_thread || arena -> frozen) {
        return 1;
    }
    if (arena -> allocations == 0 || arena -> allocation_descriptors == 0) {
        return 1;
    }
    if (lm_own_arena_promote_lazy_edges(lm_lmx_message_thread, arena) != 0) {
        return 1;
    }
    index = 0U;
    while (index < arena -> allocation_descriptors -> count) {
        descriptor = lm_own_ptr_stack_at(arena -> allocation_descriptors, index);
        if (descriptor != 0) {
            descriptor->marked = 0;
        }
        index = index + 1U;
    }
    if (arena -> roots != 0) {
        index = 0U;
        while (index < arena -> roots -> count) {
            lm_own_arena_mark(lm_lmx_message_thread, arena, lm_own_ptr_stack_at(arena -> roots, index));
            index = index + 1U;
        }
    }
    index = 0U;
    while (index < arena -> allocation_descriptors -> count) {
        descriptor = lm_own_ptr_stack_at(arena -> allocation_descriptors, index);
        if (descriptor != 0 && descriptor -> pinned) {
            lm_own_arena_mark(lm_lmx_message_thread, arena, descriptor -> address);
        }
        index = index + 1U;
    }
    keep = 0U;
    index = 0U;
    while (index < arena -> allocations -> count) {
        object = lm_own_ptr_stack_at(arena -> allocations, index);
        descriptor = lm_own_ptr_stack_at(arena -> allocation_descriptors, index);
        if (descriptor != 0 && (descriptor -> marked != 0 || descriptor -> pinned != 0)) {
            arena->allocations->items[keep] = object;
            arena->allocation_descriptors->items[keep] = descriptor;
            keep = keep + 1U;
        }
        else {
            if (object != 0) {
                free(object);
            }
            if (descriptor != 0) {
                lm_own_delete(descriptor, 0);
            }
        }
        index = index + 1U;
    }
    arena->allocations->count = keep;
    arena->allocation_descriptors->count = keep;
    if (arena -> roots != 0) {
        keep = 0U;
        index = 0U;
        while (index < arena -> roots -> count) {
            item = lm_own_ptr_stack_at(arena -> roots, index);
            if (item != 0 && lm_own_allocation_descriptor_find(lm_lmx_message_thread, arena -> allocation_descriptors, item) != 0) {
                arena->roots->items[keep] = item;
                keep = keep + 1U;
            }
            index = index + 1U;
        }
        arena->roots->count = keep;
    }
    return 0;
}

int lm_own_arena_absorb(struct LmMessageThread *lm_lmx_message_thread, LmOwnArena *target, LmOwnArena *source) {
    (void)lm_lmx_message_thread;
    void **items;
    size_t descriptor_base;
    size_t descriptor_new_count;
    size_t new_count;
    size_t capacity;
    size_t lazy_base;
    size_t lazy_new_count;
    size_t i;
    LmOwnAllocationDescriptor * descriptor;
    LmOwnLazyEdge * edge;
    if (target == 0 || source == 0 || lm_lmx_message_thread == 0 || target -> owner_thread != lm_lmx_message_thread || source -> owner_thread != lm_lmx_message_thread || target -> frozen) {
        return 1;
    }
    if (target -> allocations == 0 || target -> allocation_descriptors == 0 || target -> lazy_edges == 0) {
        return 1;
    }
    if (source -> allocations == 0 || source -> allocation_descriptors == 0 || source -> lazy_edges == 0) {
        return 1;
    }
    if (source == target) {
        return 0;
    }
    if (source -> allocations -> count > 0U) {
        new_count = target -> allocations -> count + source -> allocations -> count;
        if (target -> allocations -> capacity == 0U) {
            capacity = 8U;
        }
        if (target -> allocations -> capacity != 0U) {
            capacity = target -> allocations -> capacity;
        }
        while (capacity < new_count) {
            capacity = capacity * 2U;
        }
        items = realloc(target -> allocations -> items, capacity * sizeof(items[0]));
        if (items == 0) {
            return 1;
        }
        target->allocations->items = items;
        target->allocations->capacity = capacity;
        memcpy(target -> allocations -> items + target -> allocations -> count, source -> allocations -> items, source -> allocations -> count * sizeof(source -> allocations -> items[0]));
        target->allocations->count = new_count;
        free(source -> allocations -> items);
        source->allocations->items = 0;
        source->allocations->count = 0U;
        source->allocations->capacity = 0U;
    }
    descriptor_base = target -> allocation_descriptors -> count;
    descriptor_new_count = descriptor_base + source -> allocation_descriptors -> count;
    if (source -> allocation_descriptors -> count > 0U) {
        if (target -> allocation_descriptors -> capacity == 0U) {
            capacity = 8U;
        }
        if (target -> allocation_descriptors -> capacity != 0U) {
            capacity = target -> allocation_descriptors -> capacity;
        }
        while (capacity < descriptor_new_count) {
            capacity = capacity * 2U;
        }
        items = realloc(target -> allocation_descriptors -> items, capacity * sizeof(items[0]));
        if (items == 0) {
            return 1;
        }
        target->allocation_descriptors->items = items;
        target->allocation_descriptors->capacity = capacity;
        memcpy(target -> allocation_descriptors -> items + descriptor_base, source -> allocation_descriptors -> items, source -> allocation_descriptors -> count * sizeof(source -> allocation_descriptors -> items[0]));
        target->allocation_descriptors->count = descriptor_new_count;
        i = descriptor_base;
        while (i < target -> allocation_descriptors -> count) {
            descriptor = lm_own_ptr_stack_at(target -> allocation_descriptors, i);
            if (descriptor != 0 && descriptor -> owner == source) {
                descriptor->owner = target;
            }
            i = i + 1U;
        }
        free(source -> allocation_descriptors -> items);
        source->allocation_descriptors->items = 0;
        source->allocation_descriptors->count = 0U;
        source->allocation_descriptors->capacity = 0U;
    }
    lazy_base = target -> lazy_edges -> count;
    lazy_new_count = lazy_base + source -> lazy_edges -> count;
    if (source -> lazy_edges -> count > 0U) {
        if (target -> lazy_edges -> capacity == 0U) {
            capacity = 8U;
        }
        if (target -> lazy_edges -> capacity != 0U) {
            capacity = target -> lazy_edges -> capacity;
        }
        while (capacity < lazy_new_count) {
            capacity = capacity * 2U;
        }
        items = realloc(target -> lazy_edges -> items, capacity * sizeof(items[0]));
        if (items == 0) {
            return 1;
        }
        target->lazy_edges->items = items;
        target->lazy_edges->capacity = capacity;
        memcpy(target -> lazy_edges -> items + lazy_base, source -> lazy_edges -> items, source -> lazy_edges -> count * sizeof(source -> lazy_edges -> items[0]));
        target->lazy_edges->count = lazy_new_count;
        i = lazy_base;
        while (i < target -> lazy_edges -> count) {
            edge = lm_own_ptr_stack_at(target -> lazy_edges, i);
            if (edge != 0 && edge -> target_owner == source) {
                edge->target_owner = target;
            }
            i = i + 1U;
        }
        free(source -> lazy_edges -> items);
        source->lazy_edges->items = 0;
        source->lazy_edges->count = 0U;
        source->lazy_edges->capacity = 0U;
    }
    return 0;
}

int lm_message_thread_init(LmMessageThread *thread) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    if (thread == 0 || thread -> pool != 0 || thread -> runtime != 0) {
        return 1;
    }
    if (thread -> state == LM_MESSAGE_THREAD_RUNNING || thread -> state == LM_MESSAGE_THREAD_STOPPING || thread -> root_owner != 0 || thread -> arena_head != 0 || thread -> arena_tail != 0 || thread -> arena_count != 0U || thread -> arena_destroying || thread -> component_head != 0 || thread -> component_count != 0U || thread -> component_destroying || thread -> inbox_head != 0 || thread -> outbox_head != 0 || thread -> current_message != 0 || thread -> turn_active) {
        return 1;
    }
    memset(thread, 0, sizeof(thread[0]));
    thread->root_owner = lm_own_arena_new(thread, thread);
    if (thread -> root_owner == 0) {
        thread->state = LM_MESSAGE_THREAD_STOPPED;
        thread->collector_failed = 1;
        thread->stop_status = 1;
        return 1;
    }
    thread->state = LM_MESSAGE_THREAD_RUNNING;
    return 0;
}

void lm_message_thread_destroy(LmMessageThread *thread) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    LmOwnArena * arena;
    LmMessageThreadComponent * component;
    int components_released;
    if (thread != 0 && thread -> pool == 0 && thread -> runtime == 0) {
        lm_native_message_thread_mailboxes_destroy(thread);
        components_released = 1;
        thread->component_destroying = 1;
        while (thread -> component_head != 0) {
            component = thread -> component_head;
            if (lm_message_thread_component_release(thread, thread, component) != 0) {
                thread->collector_failed = 1;
                if (thread -> stop_status == 0) {
                    thread->stop_status = 1;
                }
                components_released = 0;
                break;
            }
        }
        if (thread -> component_count == 0U) {
            thread->component_head = 0;
        }
        thread->component_destroying = 0;
        if (components_released) {
            thread->arena_destroying = 1;
            while (thread -> arena_tail != 0) {
                arena = thread -> arena_tail;
                if (lm_own_arena_release(thread, thread, arena, arena -> runtime_owned_shell, 1) != 0) {
                    thread->collector_failed = 1;
                    if (thread -> stop_status == 0) {
                        thread->stop_status = 1;
                    }
                    break;
                }
            }
            if (thread -> arena_count == 0U) {
                thread->root_owner = 0;
                thread->arena_head = 0;
                thread->arena_tail = 0;
            }
            thread->arena_destroying = 0;
        }
        thread->execution_context = 0;
        thread->state = LM_MESSAGE_THREAD_STOPPED;
    }
}

LmMessageThread * lm_message_thread_new(void) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    LmMessageThread * thread;
    thread = lm_own_new_zero(sizeof(LmMessageThread));
    if (thread == 0) {
        return 0;
    }
    if (lm_message_thread_init(thread) != 0) {
        lm_own_delete(thread, 0);
        return 0;
    }
    return thread;
}

void lm_message_thread_delete(LmMessageThread *thread) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    if (thread != 0) {
        if (thread -> pool != 0) {
            return;
        }
        lm_message_thread_destroy(thread);
        if (thread -> arena_count == 0U && thread -> arena_head == 0 && thread -> arena_tail == 0 && thread -> component_count == 0U && thread -> component_head == 0) {
            lm_condition_delete(thread -> stopped_condition);
            thread->stopped_condition = 0;
            lm_mutex_delete(thread -> state_mutex);
            thread->state_mutex = 0;
            lm_own_delete(thread, 0);
        }
    }
}

int lm_message_thread_is_running(const LmMessageThread *thread) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    int result;
    if (thread == 0 || lm_native_message_thread_state_lock(thread) != 0) {
        return 0;
    }
    result = thread -> state == LM_MESSAGE_THREAD_RUNNING;
    lm_native_message_thread_state_unlock(thread);
    return result;
}

int lm_message_thread_status(const LmMessageThread *thread) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    int result;
    if (thread == 0 || lm_native_message_thread_state_lock(thread) != 0) {
        return 1;
    }
    result = thread -> stop_status;
    lm_native_message_thread_state_unlock(thread);
    return result;
}

LmOwnArena * lm_message_thread_owner(LmMessageThread *thread) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    if (thread == 0 || thread -> root_owner == 0 || thread -> root_owner -> owner_thread != thread || thread -> arena_head != thread -> root_owner || thread -> root_owner -> registry_previous != 0 || thread -> arena_count == 0U) {
        return 0;
    }
    return thread -> root_owner;
}

void * lm_message_thread_execution_context(LmMessageThread *thread) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    void *result;
    if (thread == 0 || lm_native_message_thread_state_lock(thread) != 0) {
        return 0;
    }
    result = thread -> execution_context;
    lm_native_message_thread_state_unlock(thread);
    return result;
}

void * lm_message_thread_set_execution_context(LmMessageThread *thread, void *context) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    void *previous;
    if (thread == 0 || lm_native_message_thread_state_lock(thread) != 0) {
        return 0;
    }
    previous = thread -> execution_context;
    thread->execution_context = context;
    lm_native_message_thread_state_unlock(thread);
    return previous;
}

size_t lm_message_thread_turn_count(const LmMessageThread *thread) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    size_t result;
    if (thread == 0 || lm_native_message_thread_state_lock(thread) != 0) {
        return 0U;
    }
    result = thread -> turn_count;
    lm_native_message_thread_state_unlock(thread);
    return result;
}

size_t lm_message_thread_collection_count(const LmMessageThread *thread) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    size_t result;
    if (thread == 0 || lm_native_message_thread_state_lock(thread) != 0) {
        return 0U;
    }
    result = thread -> collection_count;
    lm_native_message_thread_state_unlock(thread);
    return result;
}

size_t lm_message_thread_arena_count(const LmMessageThread *thread) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    size_t result;
    if (thread == 0 || lm_native_message_thread_state_lock(thread) != 0) {
        return 0U;
    }
    result = thread -> arena_count;
    lm_native_message_thread_state_unlock(thread);
    return result;
}

void lm_message_thread_request_stop(LmMessageThread *thread, int status) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    LmMessageThreadPool * pool;
    if (thread == 0) {
        return;
    }
    pool = thread -> pool;
    if (pool != 0) {
        if (lm_mutex_lock(pool -> mutex) != 0) {
            return;
        }
        if (pool -> deleting || lm_native_message_thread_control_allowed_locked(pool, thread) == 0) {
            lm_mutex_unlock(pool -> mutex);
            return;
        }
    }
    if (pool == 0 && thread -> runtime != 0) {
        if (thread -> runtime -> root_thread != thread || lm_native_thread_identity_is_current(thread -> runtime -> identity) == 0) {
            return;
        }
    }
    if (lm_native_message_thread_state_lock(thread) == 0) {
        if (thread -> state == LM_MESSAGE_THREAD_RUNNING) {
            thread->stop_status = status;
            thread->state = LM_MESSAGE_THREAD_STOPPING;
            if (pool != 0) {
                lm_native_message_thread_schedule_stop_locked(pool, thread);
            }
        }
        if (thread -> state == LM_MESSAGE_THREAD_NEW) {
            thread->stop_status = status;
            thread->state = LM_MESSAGE_THREAD_STOPPED;
            if (thread -> stopped_condition != 0) {
                lm_condition_broadcast(thread -> stopped_condition);
            }
        }
        lm_native_message_thread_state_unlock(thread);
    }
    if (pool != 0) {
        lm_mutex_unlock(pool -> mutex);
    }
}

void lm_message_thread_request_failure(LmMessageThread *thread, int status) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    LmMessageThreadPool * pool;
    if (status == 0) {
        status = 1;
    }
    if (thread == 0) {
        return;
    }
    pool = thread -> pool;
    if (pool != 0) {
        if (lm_mutex_lock(pool -> mutex) != 0) {
            return;
        }
        if (pool -> deleting || lm_native_message_thread_control_allowed_locked(pool, thread) == 0) {
            lm_mutex_unlock(pool -> mutex);
            return;
        }
    }
    if (pool == 0 && thread -> runtime != 0) {
        if (thread -> runtime -> root_thread != thread || lm_native_thread_identity_is_current(thread -> runtime -> identity) == 0) {
            return;
        }
    }
    if (lm_native_message_thread_state_lock(thread) == 0) {
        thread->turn_failed = 1;
        if (thread -> state == LM_MESSAGE_THREAD_RUNNING || thread -> state == LM_MESSAGE_THREAD_STOPPING) {
            thread->stop_status = status;
            thread->state = LM_MESSAGE_THREAD_STOPPING;
            if (pool != 0) {
                lm_native_message_thread_schedule_stop_locked(pool, thread);
            }
        }
        lm_native_message_thread_state_unlock(thread);
    }
    if (pool != 0) {
        lm_mutex_unlock(pool -> mutex);
    }
}

int lm_message_thread_collect(LmMessageThread *thread) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    if (thread == 0 || thread -> pool != 0) {
        return 1;
    }
    if (lm_native_message_thread_state_lock(thread) != 0) {
        return 1;
    }
    thread->collection_count = thread -> collection_count + 1U;
    lm_native_message_thread_state_unlock(thread);
    if (thread -> root_owner == 0 || thread -> root_owner -> owner_thread != thread || thread -> arena_head != thread -> root_owner || thread -> root_owner -> registry_previous != 0 || thread -> arena_tail == 0 || thread -> arena_tail -> registry_next != 0 || thread -> arena_count == 0U) {
        if (lm_native_message_thread_state_lock(thread) == 0) {
            thread->collector_failed = 1;
            lm_native_message_thread_state_unlock(thread);
        }
        lm_message_thread_request_failure(thread, 1);
        return 1;
    }
    if (lm_own_tree_cut(thread, thread -> root_owner) != 0) {
        if (lm_native_message_thread_state_lock(thread) == 0) {
            thread->collector_failed = 1;
            lm_native_message_thread_state_unlock(thread);
        }
        lm_message_thread_request_failure(thread, 1);
        return 1;
    }
    if (lm_own_arena_reclaim(thread, thread -> root_owner) != 0) {
        if (lm_native_message_thread_state_lock(thread) == 0) {
            thread->collector_failed = 1;
            lm_native_message_thread_state_unlock(thread);
        }
        lm_message_thread_request_failure(thread, 1);
        return 1;
    }
    return 0;
}

int lm_message_thread_begin_turn(LmMessageThread *thread) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    if (thread == 0 || thread -> pool != 0) {
        return 0;
    }
    if (thread -> runtime != 0 && (thread -> runtime -> root_thread != thread || lm_native_thread_identity_is_current(thread -> runtime -> identity) == 0)) {
        return 0;
    }
    if (lm_native_message_thread_state_lock(thread) != 0) {
        return 0;
    }
    if (thread -> state != LM_MESSAGE_THREAD_RUNNING || thread -> turn_active) {
        lm_native_message_thread_state_unlock(thread);
        return 0;
    }
    thread->turn_active = 1;
    thread->turn_failed = 0;
    thread->turn_count = thread -> turn_count + 1U;
    lm_native_message_thread_state_unlock(thread);
    return 1;
}

int lm_message_thread_end_turn(LmMessageThread *thread) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    int status;
    int collect_status;
    int failed_status;
    if (thread == 0 || thread -> pool != 0) {
        return 1;
    }
    if (thread -> runtime != 0 && (thread -> runtime -> root_thread != thread || lm_native_thread_identity_is_current(thread -> runtime -> identity) == 0)) {
        return 1;
    }
    if (lm_native_message_thread_state_lock(thread) != 0) {
        return 1;
    }
    if (thread -> turn_active == 0) {
        lm_native_message_thread_state_unlock(thread);
        return 1;
    }
    failed_status = 0;
    if (thread -> turn_failed) {
        failed_status = thread -> stop_status;
        if (failed_status == 0) {
            failed_status = 1;
        }
    }
    lm_native_message_thread_state_unlock(thread);
    status = lm_native_message_thread_outbox_finish(thread, failed_status == 0);
    collect_status = lm_message_thread_collect(thread);
    if (status == 0 && collect_status != 0) {
        status = collect_status;
    }
    if (status == 0 && failed_status != 0) {
        status = failed_status;
    }
    if (lm_native_message_thread_state_lock(thread) != 0) {
        return 1;
    }
    thread->turn_active = 0;
    if (thread -> state == LM_MESSAGE_THREAD_STOPPING) {
        thread->state = LM_MESSAGE_THREAD_STOPPED;
    }
    if (thread -> state == LM_MESSAGE_THREAD_STOPPED && thread -> stopped_condition != 0) {
        lm_condition_broadcast(thread -> stopped_condition);
    }
    lm_native_message_thread_state_unlock(thread);
    lm_native_message_thread_mark_exit_ready(thread);
    return status;
}
