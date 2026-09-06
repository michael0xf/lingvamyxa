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
#ifndef lm_lmx_module_private_1_typedef_defined_getenv
#define lm_lmx_module_private_1_typedef_defined_getenv 1
#define lm_lmx_module_private_1_typedef_id_a_getenv 0x494fd8239b6d99b9ULL
#define lm_lmx_module_private_1_typedef_id_b_getenv 0xfa33a67e7d98250aULL
typedef struct getenvClosure *getenv;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_getenv) || !defined(lm_lmx_module_private_1_typedef_id_b_getenv) || lm_lmx_module_private_1_typedef_id_a_getenv != 0x494fd8239b6d99b9ULL || lm_lmx_module_private_1_typedef_id_b_getenv != 0xfa33a67e7d98250aULL
#error "Lingvamyxa conflicting typedef projection for getenv"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_getenvClosure
#define lm_lmx_module_private_1_typedef_defined_getenvClosure 1
#define lm_lmx_module_private_1_typedef_id_a_getenvClosure 0x0a0dd0b30a73ca8dULL
#define lm_lmx_module_private_1_typedef_id_b_getenvClosure 0x1a3d12d044616494ULL
typedef struct getenvClosure getenvClosure;
struct getenvClosure {
    LmLmxStructure *descriptor;
    char * (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, const char *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_getenv = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_getenvClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_getenvClosure) || lm_lmx_module_private_1_typedef_id_a_getenvClosure != 0x0a0dd0b30a73ca8dULL || lm_lmx_module_private_1_typedef_id_b_getenvClosure != 0x1a3d12d044616494ULL
#error "Lingvamyxa conflicting typedef projection for getenvClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_strlen
#define lm_lmx_module_private_1_typedef_defined_strlen 1
#define lm_lmx_module_private_1_typedef_id_a_strlen 0xcd88205ced3755c3ULL
#define lm_lmx_module_private_1_typedef_id_b_strlen 0x3aeeb86a40a37ab4ULL
typedef struct strlenClosure *strlen;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_strlen) || !defined(lm_lmx_module_private_1_typedef_id_b_strlen) || lm_lmx_module_private_1_typedef_id_a_strlen != 0xcd88205ced3755c3ULL || lm_lmx_module_private_1_typedef_id_b_strlen != 0x3aeeb86a40a37ab4ULL
#error "Lingvamyxa conflicting typedef projection for strlen"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_strlenClosure
#define lm_lmx_module_private_1_typedef_defined_strlenClosure 1
#define lm_lmx_module_private_1_typedef_id_a_strlenClosure 0x3c36d443d6587b43ULL
#define lm_lmx_module_private_1_typedef_id_b_strlenClosure 0xd46e0141dda5117eULL
typedef struct strlenClosure strlenClosure;
struct strlenClosure {
    LmLmxStructure *descriptor;
    size_t (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, const char *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_strlen = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_strlenClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_strlenClosure) || lm_lmx_module_private_1_typedef_id_a_strlenClosure != 0x3c36d443d6587b43ULL || lm_lmx_module_private_1_typedef_id_b_strlenClosure != 0xd46e0141dda5117eULL
#error "Lingvamyxa conflicting typedef projection for strlenClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_strchr
#define lm_lmx_module_private_1_typedef_defined_strchr 1
#define lm_lmx_module_private_1_typedef_id_a_strchr 0x8ef99a7a28348ec7ULL
#define lm_lmx_module_private_1_typedef_id_b_strchr 0xf38193d8aded3d54ULL
typedef struct strchrClosure *strchr;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_strchr) || !defined(lm_lmx_module_private_1_typedef_id_b_strchr) || lm_lmx_module_private_1_typedef_id_a_strchr != 0x8ef99a7a28348ec7ULL || lm_lmx_module_private_1_typedef_id_b_strchr != 0xf38193d8aded3d54ULL
#error "Lingvamyxa conflicting typedef projection for strchr"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_strchrClosure
#define lm_lmx_module_private_1_typedef_defined_strchrClosure 1
#define lm_lmx_module_private_1_typedef_id_a_strchrClosure 0x4e991240d58f3cd0ULL
#define lm_lmx_module_private_1_typedef_id_b_strchrClosure 0xf8cc928a714cad61ULL
typedef struct strchrClosure strchrClosure;
struct strchrClosure {
    LmLmxStructure *descriptor;
    char * (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, const char *, int);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_strchr = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_strchrClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_strchrClosure) || lm_lmx_module_private_1_typedef_id_a_strchrClosure != 0x4e991240d58f3cd0ULL || lm_lmx_module_private_1_typedef_id_b_strchrClosure != 0xf8cc928a714cad61ULL
#error "Lingvamyxa conflicting typedef projection for strchrClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_memcpy
#define lm_lmx_module_private_1_typedef_defined_memcpy 1
#define lm_lmx_module_private_1_typedef_id_a_memcpy 0x7f5a55088d75c281ULL
#define lm_lmx_module_private_1_typedef_id_b_memcpy 0x24ab06c42177549aULL
typedef struct memcpyClosure *memcpy;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_memcpy) || !defined(lm_lmx_module_private_1_typedef_id_b_memcpy) || lm_lmx_module_private_1_typedef_id_a_memcpy != 0x7f5a55088d75c281ULL || lm_lmx_module_private_1_typedef_id_b_memcpy != 0x24ab06c42177549aULL
#error "Lingvamyxa conflicting typedef projection for memcpy"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_memcpyClosure
#define lm_lmx_module_private_1_typedef_defined_memcpyClosure 1
#define lm_lmx_module_private_1_typedef_id_a_memcpyClosure 0xf564b1b845e99c1bULL
#define lm_lmx_module_private_1_typedef_id_b_memcpyClosure 0xe008560d8e0aadf6ULL
typedef struct memcpyClosure memcpyClosure;
struct memcpyClosure {
    LmLmxStructure *descriptor;
    void * (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, void *, const void *, size_t);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_memcpy = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_memcpyClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_memcpyClosure) || lm_lmx_module_private_1_typedef_id_a_memcpyClosure != 0xf564b1b845e99c1bULL || lm_lmx_module_private_1_typedef_id_b_memcpyClosure != 0xe008560d8e0aadf6ULL
#error "Lingvamyxa conflicting typedef projection for memcpyClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_strcpy
#define lm_lmx_module_private_1_typedef_defined_strcpy 1
#define lm_lmx_module_private_1_typedef_id_a_strcpy 0xd70e86a4acaf9001ULL
#define lm_lmx_module_private_1_typedef_id_b_strcpy 0x8440cbdc9f5407daULL
typedef struct strcpyClosure *strcpy;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_strcpy) || !defined(lm_lmx_module_private_1_typedef_id_b_strcpy) || lm_lmx_module_private_1_typedef_id_a_strcpy != 0xd70e86a4acaf9001ULL || lm_lmx_module_private_1_typedef_id_b_strcpy != 0x8440cbdc9f5407daULL
#error "Lingvamyxa conflicting typedef projection for strcpy"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_strcpyClosure
#define lm_lmx_module_private_1_typedef_defined_strcpyClosure 1
#define lm_lmx_module_private_1_typedef_id_a_strcpyClosure 0x42fc06f9184e897bULL
#define lm_lmx_module_private_1_typedef_id_b_strcpyClosure 0x4b5fbc37faf8fe56ULL
typedef struct strcpyClosure strcpyClosure;
struct strcpyClosure {
    LmLmxStructure *descriptor;
    char * (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, char *, const char *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_strcpy = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_strcpyClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_strcpyClosure) || lm_lmx_module_private_1_typedef_id_a_strcpyClosure != 0x42fc06f9184e897bULL || lm_lmx_module_private_1_typedef_id_b_strcpyClosure != 0x4b5fbc37faf8fe56ULL
#error "Lingvamyxa conflicting typedef projection for strcpyClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_strcat
#define lm_lmx_module_private_1_typedef_defined_strcat 1
#define lm_lmx_module_private_1_typedef_id_a_strcat 0x70d3184093fcf779ULL
#define lm_lmx_module_private_1_typedef_id_b_strcat 0xa322e00ea301a4b2ULL
typedef struct strcatClosure *strcat;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_strcat) || !defined(lm_lmx_module_private_1_typedef_id_b_strcat) || lm_lmx_module_private_1_typedef_id_a_strcat != 0x70d3184093fcf779ULL || lm_lmx_module_private_1_typedef_id_b_strcat != 0xa322e00ea301a4b2ULL
#error "Lingvamyxa conflicting typedef projection for strcat"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_strcatClosure
#define lm_lmx_module_private_1_typedef_defined_strcatClosure 1
#define lm_lmx_module_private_1_typedef_id_a_strcatClosure 0x6d839dd5679b3423ULL
#define lm_lmx_module_private_1_typedef_id_b_strcatClosure 0x28ffb1e8c5c8dee6ULL
typedef struct strcatClosure strcatClosure;
struct strcatClosure {
    LmLmxStructure *descriptor;
    char * (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, char *, const char *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_strcat = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_strcatClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_strcatClosure) || lm_lmx_module_private_1_typedef_id_a_strcatClosure != 0x6d839dd5679b3423ULL || lm_lmx_module_private_1_typedef_id_b_strcatClosure != 0x28ffb1e8c5c8dee6ULL
#error "Lingvamyxa conflicting typedef projection for strcatClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_system
#define lm_lmx_module_private_1_typedef_defined_system 1
#define lm_lmx_module_private_1_typedef_id_a_system 0xb69b036c63493e1dULL
#define lm_lmx_module_private_1_typedef_id_b_system 0xd694a5e3a8bb4b22ULL
typedef struct systemClosure *system;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_system) || !defined(lm_lmx_module_private_1_typedef_id_b_system) || lm_lmx_module_private_1_typedef_id_a_system != 0xb69b036c63493e1dULL || lm_lmx_module_private_1_typedef_id_b_system != 0xd694a5e3a8bb4b22ULL
#error "Lingvamyxa conflicting typedef projection for system"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_systemClosure
#define lm_lmx_module_private_1_typedef_defined_systemClosure 1
#define lm_lmx_module_private_1_typedef_id_a_systemClosure 0xe68b472095cb825cULL
#define lm_lmx_module_private_1_typedef_id_b_systemClosure 0xc61660e0eac79781ULL
typedef struct systemClosure systemClosure;
struct systemClosure {
    LmLmxStructure *descriptor;
    int (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, const char *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_system = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_systemClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_systemClosure) || lm_lmx_module_private_1_typedef_id_a_systemClosure != 0xe68b472095cb825cULL || lm_lmx_module_private_1_typedef_id_b_systemClosure != 0xc61660e0eac79781ULL
#error "Lingvamyxa conflicting typedef projection for systemClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_remove
#define lm_lmx_module_private_1_typedef_defined_remove 1
#define lm_lmx_module_private_1_typedef_id_a_remove 0x9671c71dbfc1aa67ULL
#define lm_lmx_module_private_1_typedef_id_b_remove 0x089695e8bf310900ULL
typedef struct removeClosure *remove;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_remove) || !defined(lm_lmx_module_private_1_typedef_id_b_remove) || lm_lmx_module_private_1_typedef_id_a_remove != 0x9671c71dbfc1aa67ULL || lm_lmx_module_private_1_typedef_id_b_remove != 0x089695e8bf310900ULL
#error "Lingvamyxa conflicting typedef projection for remove"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_removeClosure
#define lm_lmx_module_private_1_typedef_defined_removeClosure 1
#define lm_lmx_module_private_1_typedef_id_a_removeClosure 0x9d526cbabcff1d58ULL
#define lm_lmx_module_private_1_typedef_id_b_removeClosure 0x23bc443d2d466355ULL
typedef struct removeClosure removeClosure;
struct removeClosure {
    LmLmxStructure *descriptor;
    int (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, const char *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_remove = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_removeClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_removeClosure) || lm_lmx_module_private_1_typedef_id_a_removeClosure != 0x9d526cbabcff1d58ULL || lm_lmx_module_private_1_typedef_id_b_removeClosure != 0x23bc443d2d466355ULL
#error "Lingvamyxa conflicting typedef projection for removeClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_fopen
#define lm_lmx_module_private_1_typedef_defined_fopen 1
#define lm_lmx_module_private_1_typedef_id_a_fopen 0x050d9ca69d1b97e1ULL
#define lm_lmx_module_private_1_typedef_id_b_fopen 0x156e53a98e77c2eaULL
typedef struct fopenClosure *fopen;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_fopen) || !defined(lm_lmx_module_private_1_typedef_id_b_fopen) || lm_lmx_module_private_1_typedef_id_a_fopen != 0x050d9ca69d1b97e1ULL || lm_lmx_module_private_1_typedef_id_b_fopen != 0x156e53a98e77c2eaULL
#error "Lingvamyxa conflicting typedef projection for fopen"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_fopenClosure
#define lm_lmx_module_private_1_typedef_defined_fopenClosure 1
#define lm_lmx_module_private_1_typedef_id_a_fopenClosure 0x42cfefbfcc3f6ec6ULL
#define lm_lmx_module_private_1_typedef_id_b_fopenClosure 0xdace37da03ef3dbfULL
typedef struct fopenClosure fopenClosure;
struct fopenClosure {
    LmLmxStructure *descriptor;
    FILE * (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, const char *, const char *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_fopen = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_fopenClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_fopenClosure) || lm_lmx_module_private_1_typedef_id_a_fopenClosure != 0x42cfefbfcc3f6ec6ULL || lm_lmx_module_private_1_typedef_id_b_fopenClosure != 0xdace37da03ef3dbfULL
#error "Lingvamyxa conflicting typedef projection for fopenClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_fclose
#define lm_lmx_module_private_1_typedef_defined_fclose 1
#define lm_lmx_module_private_1_typedef_id_a_fclose 0x9bf7e91d437e8a7fULL
#define lm_lmx_module_private_1_typedef_id_b_fclose 0x2a69b5cb6a59f7d4ULL
typedef struct fcloseClosure *fclose;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_fclose) || !defined(lm_lmx_module_private_1_typedef_id_b_fclose) || lm_lmx_module_private_1_typedef_id_a_fclose != 0x9bf7e91d437e8a7fULL || lm_lmx_module_private_1_typedef_id_b_fclose != 0x2a69b5cb6a59f7d4ULL
#error "Lingvamyxa conflicting typedef projection for fclose"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_fcloseClosure
#define lm_lmx_module_private_1_typedef_defined_fcloseClosure 1
#define lm_lmx_module_private_1_typedef_id_a_fcloseClosure 0x94a4c4392a0264d5ULL
#define lm_lmx_module_private_1_typedef_id_b_fcloseClosure 0x9b31bb8ec49c2262ULL
typedef struct fcloseClosure fcloseClosure;
struct fcloseClosure {
    LmLmxStructure *descriptor;
    int (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, FILE *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_fclose = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_fcloseClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_fcloseClosure) || lm_lmx_module_private_1_typedef_id_a_fcloseClosure != 0x94a4c4392a0264d5ULL || lm_lmx_module_private_1_typedef_id_b_fcloseClosure != 0x9b31bb8ec49c2262ULL
#error "Lingvamyxa conflicting typedef projection for fcloseClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_fread
#define lm_lmx_module_private_1_typedef_defined_fread 1
#define lm_lmx_module_private_1_typedef_id_a_fread 0x695af7ce31489fb5ULL
#define lm_lmx_module_private_1_typedef_id_b_fread 0x1bb5f90bfea519e6ULL
typedef struct freadClosure *fread;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_fread) || !defined(lm_lmx_module_private_1_typedef_id_b_fread) || lm_lmx_module_private_1_typedef_id_a_fread != 0x695af7ce31489fb5ULL || lm_lmx_module_private_1_typedef_id_b_fread != 0x1bb5f90bfea519e6ULL
#error "Lingvamyxa conflicting typedef projection for fread"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_freadClosure
#define lm_lmx_module_private_1_typedef_defined_freadClosure 1
#define lm_lmx_module_private_1_typedef_id_a_freadClosure 0xb8b7ba1568dbf1c8ULL
#define lm_lmx_module_private_1_typedef_id_b_freadClosure 0x0ba0d125e1204071ULL
typedef struct freadClosure freadClosure;
struct freadClosure {
    LmLmxStructure *descriptor;
    size_t (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, void *, size_t, size_t, FILE *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_fread = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_freadClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_freadClosure) || lm_lmx_module_private_1_typedef_id_a_freadClosure != 0xb8b7ba1568dbf1c8ULL || lm_lmx_module_private_1_typedef_id_b_freadClosure != 0x0ba0d125e1204071ULL
#error "Lingvamyxa conflicting typedef projection for freadClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_fwrite
#define lm_lmx_module_private_1_typedef_defined_fwrite 1
#define lm_lmx_module_private_1_typedef_id_a_fwrite 0x07a6b003a12ad9b1ULL
#define lm_lmx_module_private_1_typedef_id_b_fwrite 0xcadf07055d6cabf2ULL
typedef struct fwriteClosure *fwrite;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_fwrite) || !defined(lm_lmx_module_private_1_typedef_id_b_fwrite) || lm_lmx_module_private_1_typedef_id_a_fwrite != 0x07a6b003a12ad9b1ULL || lm_lmx_module_private_1_typedef_id_b_fwrite != 0xcadf07055d6cabf2ULL
#error "Lingvamyxa conflicting typedef projection for fwrite"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_fwriteClosure
#define lm_lmx_module_private_1_typedef_defined_fwriteClosure 1
#define lm_lmx_module_private_1_typedef_id_a_fwriteClosure 0x6ba6235b0b2b379fULL
#define lm_lmx_module_private_1_typedef_id_b_fwriteClosure 0xb1a389c160fd65d6ULL
typedef struct fwriteClosure fwriteClosure;
struct fwriteClosure {
    LmLmxStructure *descriptor;
    size_t (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, const void *, size_t, size_t, FILE *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_fwrite = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_fwriteClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_fwriteClosure) || lm_lmx_module_private_1_typedef_id_a_fwriteClosure != 0x6ba6235b0b2b379fULL || lm_lmx_module_private_1_typedef_id_b_fwriteClosure != 0xb1a389c160fd65d6ULL
#error "Lingvamyxa conflicting typedef projection for fwriteClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_ferror
#define lm_lmx_module_private_1_typedef_defined_ferror 1
#define lm_lmx_module_private_1_typedef_id_a_ferror 0x7dd8f8e18cdc0e9bULL
#define lm_lmx_module_private_1_typedef_id_b_ferror 0x27c93d7a82e3530cULL
typedef struct ferrorClosure *ferror;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_ferror) || !defined(lm_lmx_module_private_1_typedef_id_b_ferror) || lm_lmx_module_private_1_typedef_id_a_ferror != 0x7dd8f8e18cdc0e9bULL || lm_lmx_module_private_1_typedef_id_b_ferror != 0x27c93d7a82e3530cULL
#error "Lingvamyxa conflicting typedef projection for ferror"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_ferrorClosure
#define lm_lmx_module_private_1_typedef_defined_ferrorClosure 1
#define lm_lmx_module_private_1_typedef_id_a_ferrorClosure 0x65c2b796165379c9ULL
#define lm_lmx_module_private_1_typedef_id_b_ferrorClosure 0xc2e5ae79a80b906eULL
typedef struct ferrorClosure ferrorClosure;
struct ferrorClosure {
    LmLmxStructure *descriptor;
    int (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, FILE *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_ferror = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_ferrorClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_ferrorClosure) || lm_lmx_module_private_1_typedef_id_a_ferrorClosure != 0x65c2b796165379c9ULL || lm_lmx_module_private_1_typedef_id_b_ferrorClosure != 0xc2e5ae79a80b906eULL
#error "Lingvamyxa conflicting typedef projection for ferrorClosure"
#endif
#endif

#ifndef lm_lmx_module_private_1_typedef_defined_strcmp
#define lm_lmx_module_private_1_typedef_defined_strcmp 1
#define lm_lmx_module_private_1_typedef_id_a_strcmp 0xf710f35444dc2f51ULL
#define lm_lmx_module_private_1_typedef_id_b_strcmp 0x6feca92f76b4c4b2ULL
typedef struct strcmpClosure *strcmp;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_strcmp) || !defined(lm_lmx_module_private_1_typedef_id_b_strcmp) || lm_lmx_module_private_1_typedef_id_a_strcmp != 0xf710f35444dc2f51ULL || lm_lmx_module_private_1_typedef_id_b_strcmp != 0x6feca92f76b4c4b2ULL
#error "Lingvamyxa conflicting typedef projection for strcmp"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_strcmpClosure
#define lm_lmx_module_private_1_typedef_defined_strcmpClosure 1
#define lm_lmx_module_private_1_typedef_id_a_strcmpClosure 0x56eff41635f44a95ULL
#define lm_lmx_module_private_1_typedef_id_b_strcmpClosure 0xf6bb2f95615e67beULL
typedef struct strcmpClosure strcmpClosure;
struct strcmpClosure {
    LmLmxStructure *descriptor;
    int (*call)(void *env, struct LmMessageThread *lm_lmx_message_thread, const char *, const char *);
    void *env;
    void (*destroy)(void *env);
};
static LmLmxStructure lm_lmx_callable_desc_strcmp = {&lm_lmx_structure_descriptor};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_strcmpClosure) || !defined(lm_lmx_module_private_1_typedef_id_b_strcmpClosure) || lm_lmx_module_private_1_typedef_id_a_strcmpClosure != 0x56eff41635f44a95ULL || lm_lmx_module_private_1_typedef_id_b_strcmpClosure != 0xf6bb2f95615e67beULL
#error "Lingvamyxa conflicting typedef projection for strcmpClosure"
#endif
#endif



#if defined(_WIN32)
/* no POSIX feature macro on Windows */
#else
#define _POSIX_C_SOURCE 200809L
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char * (getenv)(const char *name);
size_t (strlen)(const char *text);
char * (strchr)(const char *text, int value);
void * (memcpy)(void *target, const void *source, size_t length);
char * (strcpy)(char *target, const char *source);
char * (strcat)(char *target, const char *source);
int (system)(const char *command);
int (remove)(const char *path);
FILE * (fopen)(const char *path, const char *mode);
int (fclose)(FILE *file);
size_t (fread)(void *buffer, size_t item_size, size_t item_count, FILE *file);
size_t (fwrite)(const void *buffer, size_t item_size, size_t item_count, FILE *file);
int (ferror)(FILE *file);
int (strcmp)(const char *left, const char *right);


#if defined(_WIN32)
#include <direct.h>
#include <windows.h>
#else
#include <time.h>
#include <unistd.h>
#endif

#if defined(_WIN32)
static char * lm_finalize_platform_getcwd(struct LmMessageThread *lm_lmx_message_thread, char *buffer, size_t size);
static int lm_finalize_platform_chdir(struct LmMessageThread *lm_lmx_message_thread, const char *path);
static const char * lm_finalize_platform_exe_suffix(struct LmMessageThread *lm_lmx_message_thread);
static const char * lm_finalize_platform_path_sep(struct LmMessageThread *lm_lmx_message_thread);
static int lm_finalize_platform_is_drive_absolute(struct LmMessageThread *lm_lmx_message_thread, const char *path);
static void lm_finalize_platform_sleep_retry(struct LmMessageThread *lm_lmx_message_thread);
static const char * lm_finalize_platform_defer_command_format(struct LmMessageThread *lm_lmx_message_thread);

static char * lm_finalize_platform_getcwd(struct LmMessageThread *lm_lmx_message_thread, char *buffer, size_t size) {
    (void)lm_lmx_message_thread;
    return _getcwd(buffer, size);
}

static int lm_finalize_platform_chdir(struct LmMessageThread *lm_lmx_message_thread, const char *path) {
    (void)lm_lmx_message_thread;
    return _chdir(path);
}

static const char * lm_finalize_platform_exe_suffix(struct LmMessageThread *lm_lmx_message_thread) {
    (void)lm_lmx_message_thread;
    return ".exe";
}

static const char * lm_finalize_platform_path_sep(struct LmMessageThread *lm_lmx_message_thread) {
    (void)lm_lmx_message_thread;
    return "\\";
}

static int lm_finalize_platform_is_drive_absolute(struct LmMessageThread *lm_lmx_message_thread, const char *path) {
    (void)lm_lmx_message_thread;
    return path[0] != '\0' && path[1] == ':';
}

static void lm_finalize_platform_sleep_retry(struct LmMessageThread *lm_lmx_message_thread) {
    (void)lm_lmx_message_thread;
    Sleep(250U);
}

static const char * lm_finalize_platform_defer_command_format(struct LmMessageThread *lm_lmx_message_thread) {
    (void)lm_lmx_message_thread;
    return "cd /d \"%s\" && start \"\" /B \"%s\" --copy";
}
#else
static char * lm_finalize_platform_getcwd(struct LmMessageThread *lm_lmx_message_thread, char *buffer, size_t size);
static int lm_finalize_platform_chdir(struct LmMessageThread *lm_lmx_message_thread, const char *path);
static const char * lm_finalize_platform_exe_suffix(struct LmMessageThread *lm_lmx_message_thread);
static const char * lm_finalize_platform_path_sep(struct LmMessageThread *lm_lmx_message_thread);
static int lm_finalize_platform_is_drive_absolute(struct LmMessageThread *lm_lmx_message_thread, const char *path);
static void lm_finalize_platform_sleep_retry(struct LmMessageThread *lm_lmx_message_thread);
static const char * lm_finalize_platform_defer_command_format(struct LmMessageThread *lm_lmx_message_thread);

static char * lm_finalize_platform_getcwd(struct LmMessageThread *lm_lmx_message_thread, char *buffer, size_t size) {
    (void)lm_lmx_message_thread;
    return getcwd(buffer, size);
}

static int lm_finalize_platform_chdir(struct LmMessageThread *lm_lmx_message_thread, const char *path) {
    (void)lm_lmx_message_thread;
    return chdir(path);
}

static const char * lm_finalize_platform_exe_suffix(struct LmMessageThread *lm_lmx_message_thread) {
    (void)lm_lmx_message_thread;
    return "";
}

static const char * lm_finalize_platform_path_sep(struct LmMessageThread *lm_lmx_message_thread) {
    (void)lm_lmx_message_thread;
    return "/";
}

static int lm_finalize_platform_is_drive_absolute(struct LmMessageThread *lm_lmx_message_thread, const char *path) {
    (void)lm_lmx_message_thread;
    return path != 0 && 0;
}

static void lm_finalize_platform_sleep_retry(struct LmMessageThread *lm_lmx_message_thread) {
    (void)lm_lmx_message_thread;
    int started;
    started = clock();
    while (started >= 0 && clock() - started < CLOCKS_PER_SEC / 4) {
    }
}

static const char * lm_finalize_platform_defer_command_format(struct LmMessageThread *lm_lmx_message_thread) {
    (void)lm_lmx_message_thread;
    return "cd \"%s\" && \"%s\" \"--copy\" >/dev/null 2>&1 &";
}
#endif
static int lm_finalize_is_path_separator(struct LmMessageThread *lm_lmx_message_thread, char value);
static int lm_finalize_has_path_separator(struct LmMessageThread *lm_lmx_message_thread, char *path);
static int lm_finalize_is_absolute_path(struct LmMessageThread *lm_lmx_message_thread, char *path);
static int lm_finalize_file_exists(struct LmMessageThread *lm_lmx_message_thread, char *path);
static int lm_finalize_join_path(struct LmMessageThread *lm_lmx_message_thread, char *buffer, size_t size, char *base, char *tail);
static int lm_finalize_has_project_marker(struct LmMessageThread *lm_lmx_message_thread, char *path);
static int lm_finalize_trim_last_path_part(struct LmMessageThread *lm_lmx_message_thread, char *path);
static int lm_finalize_enter_project_root(struct LmMessageThread *lm_lmx_message_thread, char *program_path);
static void lm_finalize_log(struct LmMessageThread *lm_lmx_message_thread, char *message);
static void lm_finalize_sleep_retry(struct LmMessageThread *lm_lmx_message_thread);
static void lm_finalize_live_tool_path(struct LmMessageThread *lm_lmx_message_thread, char *path, size_t size, char *tool_name);
static void lm_finalize_next_tool_path(struct LmMessageThread *lm_lmx_message_thread, char *path, size_t size, char *tool_name);
static void lm_finalize_legacy_next_tool_path(struct LmMessageThread *lm_lmx_message_thread, char *path, size_t size, char *tool_name);
static void lm_finalize_live_artifact_path(struct LmMessageThread *lm_lmx_message_thread, char *path, size_t size, char *artifact_name);
static void lm_finalize_next_artifact_path(struct LmMessageThread *lm_lmx_message_thread, char *path, size_t size, char *artifact_name);
static int lm_finalize_copy_once(struct LmMessageThread *lm_lmx_message_thread, char *source_path, char *output_path, int quiet);
static int lm_finalize_copy_with_retry(struct LmMessageThread *lm_lmx_message_thread, char *source_path, char *output_path);
static int lm_finalize_install_next_tool(struct LmMessageThread *lm_lmx_message_thread, char *tool_name);
static int lm_finalize_install_legacy_next_tool(struct LmMessageThread *lm_lmx_message_thread, char *tool_name);
static int lm_finalize_install_next_artifact(struct LmMessageThread *lm_lmx_message_thread, char *artifact_name);
static int lm_finalize_defer(struct LmMessageThread *lm_lmx_message_thread);
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


static int lm_finalize_is_path_separator(struct LmMessageThread *lm_lmx_message_thread, char value) {
    (void)lm_lmx_message_thread;
    return value == '/' || value == '\\';
}

static int lm_finalize_has_path_separator(struct LmMessageThread *lm_lmx_message_thread, char *path) {
    (void)lm_lmx_message_thread;
    return strchr(path, '/') != 0 || strchr(path, '\\') != 0;
}

static int lm_finalize_is_absolute_path(struct LmMessageThread *lm_lmx_message_thread, char *path) {
    (void)lm_lmx_message_thread;
    if (path == 0 || path[0] == '\0') {
        return 0;
    }
    if (lm_finalize_is_path_separator(lm_lmx_message_thread, path[0])) {
        return 1;
    }
    if (lm_finalize_platform_is_drive_absolute(lm_lmx_message_thread, path)) {
        return 1;
    }
    return 0;
}

static int lm_finalize_file_exists(struct LmMessageThread *lm_lmx_message_thread, char *path) {
    (void)lm_lmx_message_thread;
    FILE * file;
    file = fopen(path, "rb");
    if (file == 0) {
        return 0;
    }
    fclose(file);
    return 1;
}

static int lm_finalize_join_path(struct LmMessageThread *lm_lmx_message_thread, char *buffer, size_t size, char *base, char *tail) {
    (void)lm_lmx_message_thread;
    size_t base_length;
    size_t tail_length;
    size_t used;
    base_length = strlen(base);
    tail_length = strlen(tail);
    used = base_length;
    if (base_length + tail_length + 2U >= size) {
        fprintf(stderr, "finalize.lm0: path is too long\n");
        return 1;
    }
    memcpy(buffer, base, base_length);
    if (base_length > 0U && tail_length > 0U && lm_finalize_is_path_separator(lm_lmx_message_thread, base[base_length - 1U]) == 0 && lm_finalize_is_path_separator(lm_lmx_message_thread, tail[0]) == 0) {
        memcpy(buffer + used, lm_finalize_platform_path_sep(lm_lmx_message_thread), strlen(lm_finalize_platform_path_sep(lm_lmx_message_thread)));
        used = used + strlen(lm_finalize_platform_path_sep(lm_lmx_message_thread));
    }
    memcpy(buffer + used, tail, tail_length + 1U);
    return 0;
}

static int lm_finalize_has_project_marker(struct LmMessageThread *lm_lmx_message_thread, char *path) {
    (void)lm_lmx_message_thread;
    char marker_path[2048];
    if (lm_finalize_join_path(lm_lmx_message_thread, marker_path, sizeof(marker_path), path, "lm2/buildCore.lmx") != 0) {
        return 0;
    }
    if (lm_finalize_file_exists(lm_lmx_message_thread, marker_path)) {
        return 1;
    }
    return 0;
}

static int lm_finalize_trim_last_path_part(struct LmMessageThread *lm_lmx_message_thread, char *path) {
    (void)lm_lmx_message_thread;
    size_t length;
    length = strlen(path);
    while (length > 0U && lm_finalize_is_path_separator(lm_lmx_message_thread, path[length - 1U])) {
        path[length - 1U] = '\0';
        length = length - 1U;
    }
    while (length > 0U) {
        length = length - 1U;
        if (lm_finalize_is_path_separator(lm_lmx_message_thread, path[length])) {
            path[length] = '\0';
            return 0;
        }
    }
    path[0] = '\0';
    return 0;
}

static int lm_finalize_enter_project_root(struct LmMessageThread *lm_lmx_message_thread, char *program_path) {
    (void)lm_lmx_message_thread;
    char search_path[1024];
    char executable_path[1024];
    char cwd[1024];
    int depth;
    if (lm_finalize_platform_getcwd(lm_lmx_message_thread, cwd, sizeof(cwd)) == 0) {
        fprintf(stderr, "finalize.lm0: cannot read current directory\n");
        return 1;
    }
    if (program_path != 0 && program_path[0] != '\0' && lm_finalize_has_path_separator(lm_lmx_message_thread, program_path)) {
        if (lm_finalize_is_absolute_path(lm_lmx_message_thread, program_path)) {
            if (strlen(program_path) >= sizeof(executable_path)) {
                fprintf(stderr, "finalize.lm0: executable path is too long\n");
                return 1;
            }
            strcpy(executable_path, program_path);
        }
        if (lm_finalize_is_absolute_path(lm_lmx_message_thread, program_path) == 0) {
            if (lm_finalize_join_path(lm_lmx_message_thread, executable_path, sizeof(executable_path), cwd, program_path) != 0) {
                return 1;
            }
        }
        strcpy(search_path, executable_path);
        lm_finalize_trim_last_path_part(lm_lmx_message_thread, search_path);
    }
    if (program_path == 0 || program_path[0] == '\0' || lm_finalize_has_path_separator(lm_lmx_message_thread, program_path) == 0) {
        strcpy(search_path, cwd);
    }
    depth = 0;
    while (depth < 12 && search_path[0] != '\0') {
        if (lm_finalize_has_project_marker(lm_lmx_message_thread, search_path)) {
            if (lm_finalize_platform_chdir(lm_lmx_message_thread, search_path) != 0) {
                fprintf(stderr, "finalize.lm0: cannot enter project root %s\n", search_path);
                return 1;
            }
            return 0;
        }
        lm_finalize_trim_last_path_part(lm_lmx_message_thread, search_path);
        depth = depth + 1;
    }
    fprintf(stderr, "finalize.lm0: cannot locate project root from %s\n", cwd);
    return 1;
    return 0;
}

static void lm_finalize_log(struct LmMessageThread *lm_lmx_message_thread, char *message) {
    (void)lm_lmx_message_thread;
    FILE * log_file;
    log_file = fopen("build/lm0/finalize.log", "a");
    if (log_file == 0) {
        return;
    }
    fprintf(log_file, "%s\n", message);
    fclose(log_file);
}

static void lm_finalize_sleep_retry(struct LmMessageThread *lm_lmx_message_thread) {
    (void)lm_lmx_message_thread;
    lm_finalize_platform_sleep_retry(lm_lmx_message_thread);
}

static void lm_finalize_live_tool_path(struct LmMessageThread *lm_lmx_message_thread, char *path, size_t size, char *tool_name) {
    (void)lm_lmx_message_thread;
    snprintf(path, size, "build%slm0%s%s.lm0%s", lm_finalize_platform_path_sep(lm_lmx_message_thread), lm_finalize_platform_path_sep(lm_lmx_message_thread), tool_name, lm_finalize_platform_exe_suffix(lm_lmx_message_thread));
}

static void lm_finalize_next_tool_path(struct LmMessageThread *lm_lmx_message_thread, char *path, size_t size, char *tool_name) {
    (void)lm_lmx_message_thread;
    snprintf(path, size, "build%slm0%snext%s%s.lm0%s", lm_finalize_platform_path_sep(lm_lmx_message_thread), lm_finalize_platform_path_sep(lm_lmx_message_thread), lm_finalize_platform_path_sep(lm_lmx_message_thread), tool_name, lm_finalize_platform_exe_suffix(lm_lmx_message_thread));
}

static void lm_finalize_legacy_next_tool_path(struct LmMessageThread *lm_lmx_message_thread, char *path, size_t size, char *tool_name) {
    (void)lm_lmx_message_thread;
    snprintf(path, size, "build%slm0%s%s.next.lm0%s", lm_finalize_platform_path_sep(lm_lmx_message_thread), lm_finalize_platform_path_sep(lm_lmx_message_thread), tool_name, lm_finalize_platform_exe_suffix(lm_lmx_message_thread));
}

static void lm_finalize_live_artifact_path(struct LmMessageThread *lm_lmx_message_thread, char *path, size_t size, char *artifact_name) {
    (void)lm_lmx_message_thread;
    snprintf(path, size, "build%slm0%s%s", lm_finalize_platform_path_sep(lm_lmx_message_thread), lm_finalize_platform_path_sep(lm_lmx_message_thread), artifact_name);
}

static void lm_finalize_next_artifact_path(struct LmMessageThread *lm_lmx_message_thread, char *path, size_t size, char *artifact_name) {
    (void)lm_lmx_message_thread;
    snprintf(path, size, "build%slm0%snext%s%s", lm_finalize_platform_path_sep(lm_lmx_message_thread), lm_finalize_platform_path_sep(lm_lmx_message_thread), lm_finalize_platform_path_sep(lm_lmx_message_thread), artifact_name);
}

static int lm_finalize_copy_once(struct LmMessageThread *lm_lmx_message_thread, char *source_path, char *output_path, int quiet) {
    (void)lm_lmx_message_thread;
    char buffer[32768];
    FILE * source;
    FILE * output;
    size_t count;
    int status;
    source = fopen(source_path, "rb");
    if (source == 0) {
        if (quiet == 0) {
            fprintf(stderr, "finalize.lm0: cannot open input file %s\n", source_path);
        }
        return 1;
    }
    output = fopen(output_path, "wb");
    if (output == 0) {
        if (quiet == 0) {
            fprintf(stderr, "finalize.lm0: cannot open output file %s\n", output_path);
        }
        fclose(source);
        return 1;
    }
    status = 0;
    count = fread(buffer, 1U, sizeof(buffer), source);
    while (count > 0U) {
        if (fwrite(buffer, 1U, count, output) != count) {
            status = 1;
            break;
        }
        count = fread(buffer, 1U, sizeof(buffer), source);
    }
    if (ferror(source)) {
        status = 1;
    }
    if (fclose(output) != 0) {
        status = 1;
    }
    fclose(source);
    if (status != 0) {
        if (quiet == 0) {
            fprintf(stderr, "finalize.lm0: cannot copy %s to %s\n", source_path, output_path);
        }
        return 1;
    }
    return 0;
}

static int lm_finalize_copy_with_retry(struct LmMessageThread *lm_lmx_message_thread, char *source_path, char *output_path) {
    (void)lm_lmx_message_thread;
    int attempt = 0;
    while (attempt < 40) {
        if (lm_finalize_copy_once(lm_lmx_message_thread, source_path, output_path, 1) == 0) {
            printf("finalize.lm0: copied %s to %s\n", source_path, output_path);
            return 0;
        }
        lm_finalize_sleep_retry(lm_lmx_message_thread);
        attempt = attempt + 1;
    }
    if (lm_finalize_copy_once(lm_lmx_message_thread, source_path, output_path, 0) != 0) {
        return 1;
    }
    printf("finalize.lm0: copied %s to %s\n", source_path, output_path);
    return 0;
}

static int lm_finalize_install_next_tool(struct LmMessageThread *lm_lmx_message_thread, char *tool_name) {
    (void)lm_lmx_message_thread;
    char source_path[256];
    char output_path[256];
    lm_finalize_next_tool_path(lm_lmx_message_thread, source_path, sizeof(source_path), tool_name);
    lm_finalize_live_tool_path(lm_lmx_message_thread, output_path, sizeof(output_path), tool_name);
    if (lm_finalize_copy_with_retry(lm_lmx_message_thread, source_path, output_path) != 0) {
        lm_finalize_log(lm_lmx_message_thread, "copy failed");
        return 1;
    }
    return 0;
}

static int lm_finalize_install_legacy_next_tool(struct LmMessageThread *lm_lmx_message_thread, char *tool_name) {
    (void)lm_lmx_message_thread;
    char source_path[256];
    char output_path[256];
    lm_finalize_legacy_next_tool_path(lm_lmx_message_thread, source_path, sizeof(source_path), tool_name);
    lm_finalize_live_tool_path(lm_lmx_message_thread, output_path, sizeof(output_path), tool_name);
    if (lm_finalize_copy_with_retry(lm_lmx_message_thread, source_path, output_path) != 0) {
        lm_finalize_log(lm_lmx_message_thread, "copy failed");
        return 1;
    }
    if (remove(source_path) != 0) {
        fprintf(stderr, "finalize.lm0: cannot remove temporary file %s\n", source_path);
        lm_finalize_log(lm_lmx_message_thread, "remove failed");
        return 1;
    }
    return 0;
}

static int lm_finalize_install_next_artifact(struct LmMessageThread *lm_lmx_message_thread, char *artifact_name) {
    (void)lm_lmx_message_thread;
    char source_path[256];
    char output_path[256];
    lm_finalize_next_artifact_path(lm_lmx_message_thread, source_path, sizeof(source_path), artifact_name);
    lm_finalize_live_artifact_path(lm_lmx_message_thread, output_path, sizeof(output_path), artifact_name);
    if (lm_finalize_copy_with_retry(lm_lmx_message_thread, source_path, output_path) != 0) {
        lm_finalize_log(lm_lmx_message_thread, "copy failed");
        return 1;
    }
    return 0;
}

static int lm_finalize_defer(struct LmMessageThread *lm_lmx_message_thread) {
    (void)lm_lmx_message_thread;
    char root_path[1024];
    char finalize_path[1400];
    char command[4096];
    int status;
    if (lm_finalize_platform_getcwd(lm_lmx_message_thread, root_path, sizeof(root_path)) == 0) {
        fprintf(stderr, "finalize.lm0: cannot get current directory\n");
        return 1;
    }
    snprintf(finalize_path, sizeof(finalize_path), "build%slm0%snext%sfinalize.lm0%s", lm_finalize_platform_path_sep(lm_lmx_message_thread), lm_finalize_platform_path_sep(lm_lmx_message_thread), lm_finalize_platform_path_sep(lm_lmx_message_thread), lm_finalize_platform_exe_suffix(lm_lmx_message_thread));
    if (lm_finalize_file_exists(lm_lmx_message_thread, finalize_path) == 0) {
        lm_finalize_live_tool_path(lm_lmx_message_thread, finalize_path, sizeof(finalize_path), "finalize");
    }
    snprintf(command, sizeof(command), lm_finalize_platform_defer_command_format(lm_lmx_message_thread), root_path, finalize_path);
    status = system(command);
    if (status != 0) {
        fprintf(stderr, "finalize.lm0: cannot schedule deferred finalize step\n");
        lm_finalize_log(lm_lmx_message_thread, "defer failed");
        return 1;
    }
    lm_finalize_log(lm_lmx_message_thread, "defer scheduled");
    printf("finalize.lm0: scheduled deferred staged install\n");
    return 0;
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
            char staged_build_core_path[256];
            if (lm_finalize_enter_project_root(lm_lmx_module_private_1_main_lm_lmx_message_thread, argv[0]) != 0) {
                {
                    int lm_return_0 = 1;
                    lm_message_thread_request_stop(lm_lmx_module_private_1_main_lm_lmx_message_thread, lm_return_0);
                    goto lm_lmx_module_private_1_main_lm_message_thread_turn_end;
                }
            }
            if (argc == 2 && strcmp(argv[1], "--defer") == 0) {
                {
                    int lm_return_1 = lm_finalize_defer(lm_lmx_module_private_1_main_lm_lmx_message_thread);
                    lm_message_thread_request_stop(lm_lmx_module_private_1_main_lm_lmx_message_thread, lm_return_1);
                    goto lm_lmx_module_private_1_main_lm_message_thread_turn_end;
                }
            }
            if (argc != 1 && (argc != 2 || strcmp(argv[1], "--copy") != 0)) {
                fprintf(stderr, "usage: finalize.lm0[.exe] [--defer|--copy]\n");
                {
                    int lm_return_2 = 1;
                    lm_message_thread_request_stop(lm_lmx_module_private_1_main_lm_lmx_message_thread, lm_return_2);
                    goto lm_lmx_module_private_1_main_lm_message_thread_turn_end;
                }
            }
            lm_finalize_log(lm_lmx_module_private_1_main_lm_lmx_message_thread, "copy started");
            lm_finalize_next_tool_path(lm_lmx_module_private_1_main_lm_lmx_message_thread, staged_build_core_path, sizeof(staged_build_core_path), "buildCore");
            if (lm_finalize_file_exists(lm_lmx_module_private_1_main_lm_lmx_message_thread, staged_build_core_path) == 0) {
                if (lm_finalize_install_legacy_next_tool(lm_lmx_module_private_1_main_lm_lmx_message_thread, "make") != 0) {
                    {
                        int lm_return_3 = 1;
                        lm_message_thread_request_stop(lm_lmx_module_private_1_main_lm_lmx_message_thread, lm_return_3);
                        goto lm_lmx_module_private_1_main_lm_message_thread_turn_end;
                    }
                }
                if (lm_finalize_install_legacy_next_tool(lm_lmx_module_private_1_main_lm_lmx_message_thread, "buildCore") != 0) {
                    {
                        int lm_return_4 = 1;
                        lm_message_thread_request_stop(lm_lmx_module_private_1_main_lm_lmx_message_thread, lm_return_4);
                        goto lm_lmx_module_private_1_main_lm_message_thread_turn_end;
                    }
                }
                lm_finalize_log(lm_lmx_module_private_1_main_lm_lmx_message_thread, "legacy copy completed");
                {
                    int lm_return_5 = 0;
                    lm_message_thread_request_stop(lm_lmx_module_private_1_main_lm_lmx_message_thread, lm_return_5);
                    goto lm_lmx_module_private_1_main_lm_message_thread_turn_end;
                }
            }
            if (lm_finalize_install_next_artifact(lm_lmx_module_private_1_main_lm_lmx_message_thread, "libparser.lm0.a") != 0) {
                {
                    int lm_return_6 = 1;
                    lm_message_thread_request_stop(lm_lmx_module_private_1_main_lm_lmx_message_thread, lm_return_6);
                    goto lm_lmx_module_private_1_main_lm_message_thread_turn_end;
                }
            }
            if (lm_finalize_install_next_artifact(lm_lmx_module_private_1_main_lm_lmx_message_thread, "libown.lm0.a") != 0) {
                {
                    int lm_return_7 = 1;
                    lm_message_thread_request_stop(lm_lmx_module_private_1_main_lm_lmx_message_thread, lm_return_7);
                    goto lm_lmx_module_private_1_main_lm_message_thread_turn_end;
                }
            }
            if (lm_finalize_install_next_tool(lm_lmx_module_private_1_main_lm_lmx_message_thread, "make") != 0) {
                {
                    int lm_return_8 = 1;
                    lm_message_thread_request_stop(lm_lmx_module_private_1_main_lm_lmx_message_thread, lm_return_8);
                    goto lm_lmx_module_private_1_main_lm_message_thread_turn_end;
                }
            }
            if (lm_finalize_install_next_tool(lm_lmx_module_private_1_main_lm_lmx_message_thread, "trans") != 0) {
                {
                    int lm_return_9 = 1;
                    lm_message_thread_request_stop(lm_lmx_module_private_1_main_lm_lmx_message_thread, lm_return_9);
                    goto lm_lmx_module_private_1_main_lm_message_thread_turn_end;
                }
            }
            if (lm_finalize_install_next_tool(lm_lmx_module_private_1_main_lm_lmx_message_thread, "vcpkgFetch") != 0) {
                {
                    int lm_return_10 = 1;
                    lm_message_thread_request_stop(lm_lmx_module_private_1_main_lm_lmx_message_thread, lm_return_10);
                    goto lm_lmx_module_private_1_main_lm_message_thread_turn_end;
                }
            }
            if (lm_finalize_install_next_tool(lm_lmx_module_private_1_main_lm_lmx_message_thread, "printTree") != 0) {
                {
                    int lm_return_11 = 1;
                    lm_message_thread_request_stop(lm_lmx_module_private_1_main_lm_lmx_message_thread, lm_return_11);
                    goto lm_lmx_module_private_1_main_lm_message_thread_turn_end;
                }
            }
            if (lm_finalize_install_next_tool(lm_lmx_module_private_1_main_lm_lmx_message_thread, "finalize") != 0) {
                {
                    int lm_return_12 = 1;
                    lm_message_thread_request_stop(lm_lmx_module_private_1_main_lm_lmx_message_thread, lm_return_12);
                    goto lm_lmx_module_private_1_main_lm_message_thread_turn_end;
                }
            }
            if (lm_finalize_install_next_tool(lm_lmx_module_private_1_main_lm_lmx_message_thread, "buildCore") != 0) {
                {
                    int lm_return_13 = 1;
                    lm_message_thread_request_stop(lm_lmx_module_private_1_main_lm_lmx_message_thread, lm_return_13);
                    goto lm_lmx_module_private_1_main_lm_message_thread_turn_end;
                }
            }
            lm_finalize_log(lm_lmx_module_private_1_main_lm_lmx_message_thread, "copy completed");
            {
                int lm_return_14 = 0;
                lm_message_thread_request_stop(lm_lmx_module_private_1_main_lm_lmx_message_thread, lm_return_14);
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
