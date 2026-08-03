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
#include <stddef.h>




typedef struct LmOwnPtrStack LmOwnPtrStack;
typedef struct LmOwnValueStack LmOwnValueStack;
typedef struct LmOwnAllocationDescriptor LmOwnAllocationDescriptor;
typedef struct LmOwnLazyEdge LmOwnLazyEdge;
typedef struct LmOwnArena LmOwnArena;
typedef struct LmHostThread LmHostThread;
typedef struct LmMutex LmMutex;
typedef struct LmCondition LmCondition;
typedef struct LmMessage LmMessage;
typedef struct LmMessageOutboxEntry LmMessageOutboxEntry;
typedef struct LmMessageRoute LmMessageRoute;
typedef struct LmMessageThreadRuntime LmMessageThreadRuntime;
typedef struct LmRestLmxProviderOpsV1 LmRestLmxProviderOpsV1;
typedef struct LmMessageThreadPool LmMessageThreadPool;
typedef struct LmMessageThreadComponent LmMessageThreadComponent;
typedef struct LmMessageThread LmMessageThread;


typedef int LmOwnEdgeKind;
typedef int LmMessageThreadState;


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

typedef struct LmSlice {
    void *ptr;
    size_t length;
} LmSlice;
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


#ifndef LM_LMX_TYPEDEF_DEFINED_LmOwnDestroyFields
#define LM_LMX_TYPEDEF_DEFINED_LmOwnDestroyFields 1
typedef void (*LmOwnDestroyFields)(void *object);
#endif
#ifndef LM_LMX_TYPEDEF_DEFINED_LmOwnDelete
#define LM_LMX_TYPEDEF_DEFINED_LmOwnDelete 1
typedef void (*LmOwnDelete)(void *object);
#endif
#ifndef LM_LMX_TYPEDEF_DEFINED_LmHostThreadEntry
#define LM_LMX_TYPEDEF_DEFINED_LmHostThreadEntry 1
typedef void * (*LmHostThreadEntry)(void *argument);
#endif
#ifndef LM_LMX_TYPEDEF_DEFINED_LmMessageThreadEntry
#define LM_LMX_TYPEDEF_DEFINED_LmMessageThreadEntry 1
typedef void (*LmMessageThreadEntry)(struct LmMessageThread *lm_lmx_message_thread, void *argument);
#endif
#ifndef LM_LMX_TYPEDEF_DEFINED_LmMessageThreadComponentDestroy
#define LM_LMX_TYPEDEF_DEFINED_LmMessageThreadComponentDestroy 1
typedef void (*LmMessageThreadComponentDestroy)(struct LmMessageThread *lm_lmx_message_thread, void *component);
#endif
#ifndef LM_LMX_TYPEDEF_DEFINED_LmRestLmxPost
#define LM_LMX_TYPEDEF_DEFINED_LmRestLmxPost 1
typedef int (*LmRestLmxPost)(void *context, const char *normalized_uri, const char *body, size_t length, unsigned *out_http_status);
#endif
#ifndef LM_LMX_TYPEDEF_DEFINED_LmRestLmxDestroy
#define LM_LMX_TYPEDEF_DEFINED_LmRestLmxDestroy 1
typedef void (*LmRestLmxDestroy)(void *context);
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











































































































































































#include <stdlib.h>
#include <string.h>
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

#include <stdlib.h>

#define LM_THREAD_PROVIDER_AUTO 0
#define LM_THREAD_PROVIDER_PTHREAD 1
#define LM_THREAD_PROVIDER_WIN32 2
#define LM_THREAD_PROVIDER_SINGLE 3

#define LM_MESSAGE_STATUS_EXIT_DISCARDED (-1)

#ifndef LM_THREAD_PROVIDER
#define LM_THREAD_PROVIDER LM_THREAD_PROVIDER_SINGLE
#endif

#if LM_THREAD_PROVIDER == LM_THREAD_PROVIDER_AUTO
#if defined(_WIN32)
#define LM_THREAD_PROVIDER_SELECTED LM_THREAD_PROVIDER_WIN32
#elif defined(__APPLE__) || defined(__unix__)
#define LM_THREAD_PROVIDER_SELECTED LM_THREAD_PROVIDER_PTHREAD
#else
#define LM_THREAD_PROVIDER_SELECTED LM_THREAD_PROVIDER_SINGLE
#endif
#else
#define LM_THREAD_PROVIDER_SELECTED LM_THREAD_PROVIDER
#endif

typedef void *(*LmNativeHostThreadEntry)(void *argument);

#if LM_THREAD_PROVIDER_SELECTED == LM_THREAD_PROVIDER_PTHREAD

#if defined(_WIN32)
#error "LM_THREAD_PROVIDER_PTHREAD is not supported by the Windows backend"
#endif

#include <pthread.h>

typedef struct LmNativeHostThread {
    pthread_t handle;
} LmNativeHostThread;

typedef struct LmNativeThreadIdentity {
    pthread_t handle;
} LmNativeThreadIdentity;

static void *lm_native_thread_identity_new_current(void) {
    LmNativeThreadIdentity *identity =
        (LmNativeThreadIdentity *)calloc(1U, sizeof(*identity));

    if (identity != 0) {
        identity->handle = pthread_self();
    }
    return identity;
}

static int lm_native_thread_identity_is_current(const void *implementation) {
    const LmNativeThreadIdentity *identity =
        (const LmNativeThreadIdentity *)implementation;

    return identity != 0 && pthread_equal(identity->handle, pthread_self()) != 0;
}

static void lm_native_thread_identity_delete(void *implementation) {
    free(implementation);
}

static const char *lm_native_thread_provider_name(void) {
    return "pthread";
}

static void *lm_native_host_thread_start(
    LmNativeHostThreadEntry entry,
    void *argument,
    int *status
) {
    LmNativeHostThread *thread;

    if (status == 0) {
        return 0;
    }
    *status = 1;
    if (entry == 0) {
        return 0;
    }

    thread = (LmNativeHostThread *)calloc(1U, sizeof(*thread));
    if (thread == 0) {
        return 0;
    }
    if (pthread_create(&thread->handle, 0, entry, argument) != 0) {
        free(thread);
        return 0;
    }
    *status = 0;
    return thread;
}

static int lm_native_host_thread_join(void *implementation, void **result) {
    LmNativeHostThread *thread = (LmNativeHostThread *)implementation;

    if (thread == 0) {
        return 1;
    }
    if (pthread_join(thread->handle, result) != 0) {
        return 1;
    }
    free(thread);
    return 0;
}

static void *lm_native_mutex_new(void) {
    pthread_mutex_t *mutex = (pthread_mutex_t *)calloc(1U, sizeof(*mutex));

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
    pthread_mutex_t *mutex = (pthread_mutex_t *)implementation;

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
    if (implementation == 0) {
        return 1;
    }
    return pthread_mutex_lock((pthread_mutex_t *)implementation);
}

static int lm_native_mutex_unlock(void *implementation) {
    if (implementation == 0) {
        return 1;
    }
    return pthread_mutex_unlock((pthread_mutex_t *)implementation);
}

static void *lm_native_condition_new(void) {
    pthread_cond_t *condition = (pthread_cond_t *)calloc(1U, sizeof(*condition));

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
    pthread_cond_t *condition = (pthread_cond_t *)implementation;

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
    if (condition == 0 || mutex == 0) {
        return 1;
    }
    return pthread_cond_wait(
        (pthread_cond_t *)condition,
        (pthread_mutex_t *)mutex
    );
}

static int lm_native_condition_signal(void *implementation) {
    if (implementation == 0) {
        return 1;
    }
    return pthread_cond_signal((pthread_cond_t *)implementation);
}

static int lm_native_condition_broadcast(void *implementation) {
    if (implementation == 0) {
        return 1;
    }
    return pthread_cond_broadcast((pthread_cond_t *)implementation);
}

#elif LM_THREAD_PROVIDER_SELECTED == LM_THREAD_PROVIDER_WIN32

#if !defined(_WIN32)
#error "LM_THREAD_PROVIDER_WIN32 requires a Windows target"
#endif

#include <process.h>
#include <stdint.h>
#include <windows.h>

typedef struct LmNativeHostThread {
    HANDLE handle;
    unsigned thread_id;
    LmNativeHostThreadEntry entry;
    void *argument;
    void *result;
} LmNativeHostThread;

typedef struct LmNativeThreadIdentity {
    DWORD thread_id;
} LmNativeThreadIdentity;

static void *lm_native_thread_identity_new_current(void) {
    LmNativeThreadIdentity *identity =
        (LmNativeThreadIdentity *)calloc(1U, sizeof(*identity));

    if (identity != 0) {
        identity->thread_id = GetCurrentThreadId();
    }
    return identity;
}

static int lm_native_thread_identity_is_current(const void *implementation) {
    const LmNativeThreadIdentity *identity =
        (const LmNativeThreadIdentity *)implementation;

    return identity != 0 && identity->thread_id == GetCurrentThreadId();
}

static void lm_native_thread_identity_delete(void *implementation) {
    free(implementation);
}

static unsigned __stdcall lm_native_host_thread_entry(void *argument) {
    LmNativeHostThread *thread = (LmNativeHostThread *)argument;

    thread->result = thread->entry(thread->argument);
    return 0U;
}

static const char *lm_native_thread_provider_name(void) {
    return "win32";
}

static void *lm_native_host_thread_start(
    LmNativeHostThreadEntry entry,
    void *argument,
    int *status
) {
    LmNativeHostThread *thread;
    uintptr_t handle;

    if (status == 0) {
        return 0;
    }
    *status = 1;
    if (entry == 0) {
        return 0;
    }

    thread = (LmNativeHostThread *)calloc(1U, sizeof(*thread));
    if (thread == 0) {
        return 0;
    }
    thread->entry = entry;
    thread->argument = argument;
    handle = _beginthreadex(
        0,
        0U,
        lm_native_host_thread_entry,
        thread,
        0U,
        &thread->thread_id
    );
    if (handle == 0U) {
        free(thread);
        return 0;
    }
    thread->handle = (HANDLE)handle;
    *status = 0;
    return thread;
}

static int lm_native_host_thread_join(void *implementation, void **result) {
    LmNativeHostThread *thread = (LmNativeHostThread *)implementation;

    if (thread == 0) {
        return 1;
    }
    if (WaitForSingleObject(thread->handle, INFINITE) != WAIT_OBJECT_0) {
        return 1;
    }
    if (result != 0) {
        *result = thread->result;
    }
    (void)CloseHandle(thread->handle);
    free(thread);
    return 0;
}

static void *lm_native_mutex_new(void) {
    CRITICAL_SECTION *mutex = (CRITICAL_SECTION *)calloc(1U, sizeof(*mutex));

    if (mutex == 0) {
        return 0;
    }
    InitializeCriticalSection(mutex);
    return mutex;
}

static int lm_native_mutex_delete(void *implementation) {
    CRITICAL_SECTION *mutex = (CRITICAL_SECTION *)implementation;

    if (mutex == 0) {
        return 0;
    }
    DeleteCriticalSection(mutex);
    free(mutex);
    return 0;
}

static int lm_native_mutex_lock(void *implementation) {
    if (implementation == 0) {
        return 1;
    }
    EnterCriticalSection((CRITICAL_SECTION *)implementation);
    return 0;
}

static int lm_native_mutex_unlock(void *implementation) {
    if (implementation == 0) {
        return 1;
    }
    LeaveCriticalSection((CRITICAL_SECTION *)implementation);
    return 0;
}

static void *lm_native_condition_new(void) {
    CONDITION_VARIABLE *condition =
        (CONDITION_VARIABLE *)calloc(1U, sizeof(*condition));

    if (condition == 0) {
        return 0;
    }
    InitializeConditionVariable(condition);
    return condition;
}

static int lm_native_condition_delete(void *implementation) {
    free(implementation);
    return 0;
}

static int lm_native_condition_wait(void *condition, void *mutex) {
    if (condition == 0 || mutex == 0) {
        return 1;
    }
    return SleepConditionVariableCS(
        (CONDITION_VARIABLE *)condition,
        (CRITICAL_SECTION *)mutex,
        INFINITE
    ) ? 0 : 1;
}

static int lm_native_condition_signal(void *implementation) {
    if (implementation == 0) {
        return 1;
    }
    WakeConditionVariable((CONDITION_VARIABLE *)implementation);
    return 0;
}

static int lm_native_condition_broadcast(void *implementation) {
    if (implementation == 0) {
        return 1;
    }
    WakeAllConditionVariable((CONDITION_VARIABLE *)implementation);
    return 0;
}

#elif LM_THREAD_PROVIDER_SELECTED == LM_THREAD_PROVIDER_SINGLE

typedef struct LmNativeHostThread {
    void *result;
} LmNativeHostThread;

static void *lm_native_thread_identity_new_current(void) {
    return calloc(1U, 1U);
}

static int lm_native_thread_identity_is_current(const void *implementation) {
    return implementation != 0;
}

static void lm_native_thread_identity_delete(void *implementation) {
    free(implementation);
}

static const char *lm_native_thread_provider_name(void) {
    return "single";
}

static void *lm_native_host_thread_start(
    LmNativeHostThreadEntry entry,
    void *argument,
    int *status
) {
    LmNativeHostThread *thread;

    if (status == 0) {
        return 0;
    }
    *status = 1;
    if (entry == 0) {
        return 0;
    }

    thread = (LmNativeHostThread *)calloc(1U, sizeof(*thread));
    if (thread == 0) {
        return 0;
    }
    thread->result = entry(argument);
    *status = 0;
    return thread;
}

static int lm_native_host_thread_join(void *implementation, void **result) {
    LmNativeHostThread *thread = (LmNativeHostThread *)implementation;

    if (thread == 0) {
        return 1;
    }
    if (result != 0) {
        *result = thread->result;
    }
    free(thread);
    return 0;
}

static void *lm_native_mutex_new(void) {
    return calloc(1U, 1U);
}

static int lm_native_mutex_delete(void *implementation) {
    free(implementation);
    return 0;
}

static int lm_native_mutex_lock(void *implementation) {
    return implementation == 0;
}

static int lm_native_mutex_unlock(void *implementation) {
    return implementation == 0;
}

static void *lm_native_condition_new(void) {
    return calloc(1U, 1U);
}

static int lm_native_condition_delete(void *implementation) {
    free(implementation);
    return 0;
}

static int lm_native_condition_wait(void *condition, void *mutex) {
    (void)condition;
    (void)mutex;
    return 1;
}

static int lm_native_condition_signal(void *implementation) {
    return implementation == 0;
}

static int lm_native_condition_broadcast(void *implementation) {
    return implementation == 0;
}

#else
#error "unknown LM_THREAD_PROVIDER value"
#endif

static int lm_native_message_thread_state_lock(
    const LmMessageThread *thread
) {
    if (thread == 0) {
        return 1;
    }
    if (thread->state_mutex == 0) {
        return 0;
    }
    return lm_mutex_lock(thread->state_mutex);
}

static int lm_native_message_thread_state_unlock(
    const LmMessageThread *thread
) {
    if (thread == 0) {
        return 1;
    }
    if (thread->state_mutex == 0) {
        return 0;
    }
    return lm_mutex_unlock(thread->state_mutex);
}

static int lm_native_message_thread_is_current(
    const LmMessageThread *thread
) {
    const LmMessageThreadPool *pool;

    if (thread == 0) {
        return 0;
    }
    if (thread->pool == 0) {
        return thread->runtime != 0 &&
            thread->runtime->root_thread == thread &&
            thread->turn_active &&
            lm_native_thread_identity_is_current(thread->runtime->identity);
    }
    if (!thread->executing) {
        return 0;
    }
    pool = thread->pool;
    if (pool->single_mode) {
        return pool->runtime != 0 &&
            pool->runtime->single_execution_depth != 0U &&
            pool->runtime->single_active_thread == thread;
    }
    return lm_native_thread_identity_is_current(thread->execution_identity);
}

static char *lm_native_message_thread_copy_cstr(const char *source) {
    size_t length;
    char *copy;

    if (source == 0) {
        return 0;
    }
    length = strlen(source);
    if (length == (size_t)-1) {
        return 0;
    }
    copy = (char *)malloc(length + 1U);
    if (copy != 0) {
        memcpy(copy, source, length + 1U);
    }
    return copy;
}

static int lm_native_message_thread_ascii_equal_ignore_case(
    unsigned char left,
    unsigned char right
) {
    if (left >= (unsigned char)'A' && left <= (unsigned char)'Z') {
        left = (unsigned char)(left + ((unsigned char)'a' -
            (unsigned char)'A'));
    }
    if (right >= (unsigned char)'A' && right <= (unsigned char)'Z') {
        right = (unsigned char)(right + ((unsigned char)'a' -
            (unsigned char)'A'));
    }
    return left == right;
}

static int lm_native_message_thread_ascii_prefix_ignore_case(
    const char *text,
    const char *prefix,
    size_t length
) {
    size_t index;

    if (text == 0 || prefix == 0) {
        return 0;
    }
    for (index = 0U; index < length; index += 1U) {
        if (text[index] == '\0' ||
            !lm_native_message_thread_ascii_equal_ignore_case(
                (unsigned char)text[index],
                (unsigned char)prefix[index]
            )) {
            return 0;
        }
    }
    return 1;
}

static int lm_native_message_thread_ascii_is_hex(unsigned char value) {
    return (value >= (unsigned char)'0' && value <= (unsigned char)'9') ||
        (value >= (unsigned char)'a' && value <= (unsigned char)'f') ||
        (value >= (unsigned char)'A' && value <= (unsigned char)'F');
}

static int lm_native_message_thread_ascii_is_unreserved(
    unsigned char value
) {
    return (value >= (unsigned char)'a' && value <= (unsigned char)'z') ||
        (value >= (unsigned char)'A' && value <= (unsigned char)'Z') ||
        (value >= (unsigned char)'0' && value <= (unsigned char)'9') ||
        value == (unsigned char)'-' || value == (unsigned char)'.' ||
        value == (unsigned char)'_' || value == (unsigned char)'~';
}

static int lm_native_message_thread_ascii_is_sub_delim(
    unsigned char value
) {
    return value == (unsigned char)'!' || value == (unsigned char)'$' ||
        value == (unsigned char)'&' || value == (unsigned char)'\'' ||
        value == (unsigned char)'(' || value == (unsigned char)')' ||
        value == (unsigned char)'*' || value == (unsigned char)'+' ||
        value == (unsigned char)',' || value == (unsigned char)';' ||
        value == (unsigned char)'=';
}

static int lm_native_message_thread_percent_encoded_is_valid(
    const unsigned char *text,
    size_t length,
    size_t *index
) {
    size_t current = *index;

    if (current + 2U >= length ||
        !lm_native_message_thread_ascii_is_hex(text[current + 1U]) ||
        !lm_native_message_thread_ascii_is_hex(text[current + 2U])) {
        return 0;
    }
    *index = current + 2U;
    return 1;
}

static int lm_native_message_thread_path_is_valid(
    const char *path,
    int require_leading_slash
) {
    const unsigned char *text = (const unsigned char *)path;
    size_t length;
    size_t index;
    size_t segment_start;

    if (text == 0) {
        return 0;
    }
    length = strlen(path);
    if ((require_leading_slash &&
         (length == 0U || text[0] != (unsigned char)'/')) ||
        (!require_leading_slash && length != 0U &&
         text[0] != (unsigned char)'/')) {
        return 0;
    }
    segment_start = 0U;
    for (index = 0U; index < length; index += 1U) {
        unsigned char value = text[index];

        if (value <= 0x20U || value > 0x7eU ||
            value == (unsigned char)'\\' ||
            value == (unsigned char)'?' ||
            value == (unsigned char)'#') {
            return 0;
        }
        if (value == (unsigned char)'/') {
            size_t segment_length = index - segment_start;

            if (index != 0U && text[index - 1U] == (unsigned char)'/') {
                return 0;
            }
            if ((segment_length == 1U &&
                 text[segment_start] == (unsigned char)'.') ||
                (segment_length == 2U &&
                 text[segment_start] == (unsigned char)'.' &&
                 text[segment_start + 1U] == (unsigned char)'.')) {
                return 0;
            }
            segment_start = index + 1U;
        } else if (value == (unsigned char)'%') {
            if (!lm_native_message_thread_percent_encoded_is_valid(
                    text,
                    length,
                    &index
                )) {
                return 0;
            }
        } else if (!lm_native_message_thread_ascii_is_unreserved(value) &&
                   !lm_native_message_thread_ascii_is_sub_delim(value) &&
                   value != (unsigned char)':' &&
                   value != (unsigned char)'@') {
            return 0;
        }
    }
    if (segment_start < length) {
        size_t segment_length = length - segment_start;

        if ((segment_length == 1U &&
             text[segment_start] == (unsigned char)'.') ||
            (segment_length == 2U &&
             text[segment_start] == (unsigned char)'.' &&
             text[segment_start + 1U] == (unsigned char)'.')) {
            return 0;
        }
    }
    return 1;
}

static int lm_native_message_thread_ipv4_is_valid(
    const unsigned char *text,
    size_t length
) {
    size_t index = 0U;
    size_t component = 0U;

    while (index < length && component < 4U) {
        size_t digits = 0U;
        unsigned value = 0U;

        while (index < length && text[index] != (unsigned char)'.') {
            if (text[index] < (unsigned char)'0' ||
                text[index] > (unsigned char)'9' || digits == 3U) {
                return 0;
            }
            value = value * 10U +
                (unsigned)(text[index] - (unsigned char)'0');
            digits += 1U;
            index += 1U;
        }
        if (digits == 0U || value > 255U ||
            (digits > 1U && text[index - digits] == (unsigned char)'0')) {
            return 0;
        }
        component += 1U;
        if (index < length) {
            index += 1U;
            if (index == length) {
                return 0;
            }
        }
    }
    return index == length && component == 4U;
}

static int lm_native_message_thread_ipv6_is_valid(
    const unsigned char *text,
    size_t length
) {
    size_t index = 0U;
    size_t groups = 0U;
    int compressed = 0;

    if (length == 0U) {
        return 0;
    }
    if (text[0] == (unsigned char)'v' ||
        text[0] == (unsigned char)'V') {
        index = 1U;
        while (index < length &&
               lm_native_message_thread_ascii_is_hex(text[index])) {
            index += 1U;
        }
        if (index == 1U || index >= length ||
            text[index] != (unsigned char)'.') {
            return 0;
        }
        index += 1U;
        if (index == length) {
            return 0;
        }
        while (index < length) {
            if (!lm_native_message_thread_ascii_is_unreserved(text[index]) &&
                !lm_native_message_thread_ascii_is_sub_delim(text[index]) &&
                text[index] != (unsigned char)':') {
                return 0;
            }
            index += 1U;
        }
        return 1;
    }
    if (text[0] == (unsigned char)':') {
        if (length < 2U || text[1] != (unsigned char)':') {
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

        while (index < length && text[index] != (unsigned char)':') {
            if (text[index] == (unsigned char)'.') {
                contains_dot = 1;
            }
            index += 1U;
        }
        segment_length = index - segment_start;
        if (segment_length == 0U) {
            return 0;
        }
        if (contains_dot) {
            if (index != length || groups > 6U ||
                !lm_native_message_thread_ipv4_is_valid(
                    text + segment_start,
                    segment_length
                )) {
                return 0;
            }
            groups += 2U;
        } else {
            if (segment_length > 4U) {
                return 0;
            }
            for (scan = segment_start; scan < index; scan += 1U) {
                if (!lm_native_message_thread_ascii_is_hex(text[scan])) {
                    return 0;
                }
            }
            groups += 1U;
        }
        if (groups > 8U || index == length) {
            break;
        }
        if (index + 1U < length &&
            text[index + 1U] == (unsigned char)':') {
            if (compressed) {
                return 0;
            }
            compressed = 1;
            index += 2U;
            if (index == length) {
                break;
            }
        } else {
            index += 1U;
            if (index == length) {
                return 0;
            }
        }
    }
    return compressed ? groups < 8U : groups == 8U;
}

static int lm_native_message_thread_reg_name_is_valid(
    const unsigned char *text,
    size_t length
) {
    size_t index;

    if (length == 0U) {
        return 0;
    }
    for (index = 0U; index < length; index += 1U) {
        unsigned char value = text[index];

        if (value == (unsigned char)'%') {
            if (!lm_native_message_thread_percent_encoded_is_valid(
                    text,
                    length,
                    &index
                )) {
                return 0;
            }
        } else if (!lm_native_message_thread_ascii_is_unreserved(value) &&
                   !lm_native_message_thread_ascii_is_sub_delim(value)) {
            return 0;
        }
    }
    return 1;
}

static int lm_native_message_thread_port_is_valid(
    const unsigned char *text,
    size_t length
) {
    size_t index;
    unsigned value = 0U;

    if (length == 0U) {
        return 0;
    }
    for (index = 0U; index < length; index += 1U) {
        if (text[index] < (unsigned char)'0' ||
            text[index] > (unsigned char)'9') {
            return 0;
        }
        value = value * 10U +
            (unsigned)(text[index] - (unsigned char)'0');
        if (value > 65535U) {
            return 0;
        }
    }
    return value != 0U;
}

static int lm_native_message_thread_route_is_valid(const char *route) {
    return lm_native_message_thread_path_is_valid(route, 1);
}

static int lm_native_message_thread_endpoint_is_http(const char *endpoint) {
    const unsigned char *text = (const unsigned char *)endpoint;
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
    if (lm_native_message_thread_ascii_prefix_ignore_case(
            endpoint,
            "http://",
            7U
        )) {
        authority = text + 7U;
    } else if (lm_native_message_thread_ascii_prefix_ignore_case(
                   endpoint,
                   "https://",
                   8U
               )) {
        authority = text + 8U;
    } else {
        return 0;
    }
    path = authority;
    while (*path != 0U && *path != (unsigned char)'/') {
        if (*path <= 0x20U || *path > 0x7eU ||
            *path == (unsigned char)'\\' ||
            *path == (unsigned char)'?' ||
            *path == (unsigned char)'#' ||
            *path == (unsigned char)'@') {
            return 0;
        }
        path += 1;
    }
    authority_end = path;
    authority_length = (size_t)(authority_end - authority);
    if (authority_length == 0U) {
        return 0;
    }
    if (authority[0] == (unsigned char)'[') {
        host_end = authority + 1U;
        while (host_end < authority_end &&
               *host_end != (unsigned char)']') {
            host_end += 1;
        }
        if (host_end == authority_end || host_end == authority + 1U ||
            !lm_native_message_thread_ipv6_is_valid(
                authority + 1U,
                (size_t)(host_end - authority - 1U)
            )) {
            return 0;
        }
        host_end += 1U;
        if (host_end < authority_end) {
            if (*host_end != (unsigned char)':') {
                return 0;
            }
            port = host_end + 1U;
            port_length = (size_t)(authority_end - port);
        }
    } else {
        host_end = authority_end;
        for (index = 0U; index < authority_length; index += 1U) {
            if (authority[index] == (unsigned char)'[' ||
                authority[index] == (unsigned char)']') {
                return 0;
            }
            if (authority[index] == (unsigned char)':') {
                if (port != 0) {
                    return 0;
                }
                host_end = authority + index;
                port = host_end + 1U;
                port_length = (size_t)(authority_end - port);
            }
        }
        host_length = (size_t)(host_end - authority);
        if (!lm_native_message_thread_reg_name_is_valid(
                authority,
                host_length
            )) {
            return 0;
        }
    }
    if (port != 0 &&
        !lm_native_message_thread_port_is_valid(port, port_length)) {
        return 0;
    }
    return lm_native_message_thread_path_is_valid((const char *)path, 0);
}

static char *lm_native_message_thread_join_uri(
    const char *endpoint,
    const char *route
) {
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
    if (endpoint_length != 0U && route_length != 0U &&
        endpoint[endpoint_length - 1U] == '/' && route[0] == '/') {
        copied_endpoint_length -= 1U;
    }
    if (route_length > (size_t)-1 - copied_endpoint_length - 1U) {
        return 0;
    }
    uri = (char *)malloc(copied_endpoint_length + route_length + 1U);
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

static LmMessage *lm_native_message_thread_message_new(
    const char *lmx,
    size_t length
) {
    LmMessage *message;

    if (lmx == 0 || length == (size_t)-1) {
        return 0;
    }
    message = (LmMessage *)calloc(1U, sizeof(*message));
    if (message == 0) {
        return 0;
    }
    message->lmx = (char *)malloc(length + 1U);
    if (message->lmx == 0) {
        free(message);
        return 0;
    }
    if (length != 0U) {
        memcpy(message->lmx, lmx, length);
    }
    message->lmx[length] = '\0';
    message->length = length;
    return message;
}

static void lm_native_message_thread_message_delete(LmMessage *message) {
    if (message != 0) {
        free(message->lmx);
        message->lmx = 0;
        free(message);
    }
}

static void lm_native_message_thread_outbox_entry_delete(
    LmMessageOutboxEntry *entry
) {
    if (entry != 0) {
        free(entry->endpoint);
        free(entry->route);
        lm_native_message_thread_message_delete(entry->message);
        free(entry);
    }
}

static void lm_native_message_thread_mailboxes_destroy(
    LmMessageThread *thread
) {
    LmMessage *message;
    LmMessageOutboxEntry *entry;

    if (thread == 0) {
        return;
    }
    message = thread->inbox_head;
    while (message != 0) {
        LmMessage *next = message->next;

        lm_native_message_thread_message_delete(message);
        message = next;
    }
    entry = thread->outbox_head;
    while (entry != 0) {
        LmMessageOutboxEntry *next = entry->next;

        lm_native_message_thread_outbox_entry_delete(entry);
        entry = next;
    }
    lm_native_message_thread_message_delete(thread->current_message);
    thread->inbox_head = 0;
    thread->inbox_tail = 0;
    thread->inbox_count = 0U;
    thread->outbox_head = 0;
    thread->outbox_tail = 0;
    thread->outbox_count = 0U;
    thread->current_message = 0;
}

static void lm_native_message_thread_ready_push_locked(
    LmMessageThreadPool *pool,
    LmMessageThread *thread
) {
    thread->ready_next = 0;
    thread->scheduled = 1;
    if (pool->ready_tail == 0) {
        pool->ready_head = thread;
    } else {
        pool->ready_tail->ready_next = thread;
    }
    pool->ready_tail = thread;
}

static LmMessageThread *lm_native_message_thread_ready_pop_locked(
    LmMessageThreadPool *pool
) {
    LmMessageThread *thread = pool->ready_head;

    if (thread == 0) {
        return 0;
    }
    pool->ready_head = thread->ready_next;
    if (pool->ready_head == 0) {
        pool->ready_tail = 0;
    }
    thread->ready_next = 0;
    return thread;
}

static void lm_native_message_thread_schedule_stop_locked(
    LmMessageThreadPool *pool,
    LmMessageThread *thread
) {
    if (!thread->scheduled && !thread->executing) {
        lm_native_message_thread_ready_push_locked(pool, thread);
        (void)lm_condition_signal(pool->work_ready);
    }
}

static int lm_native_message_thread_pool_is_controller(
    const LmMessageThreadPool *pool
) {
    return pool != 0 && pool->runtime != 0 &&
        lm_native_thread_identity_is_current(pool->runtime->identity);
}

static int lm_native_message_thread_pool_lifecycle_allowed(
    const LmMessageThreadPool *pool
) {
    return lm_native_message_thread_pool_is_controller(pool) &&
        pool->runtime->single_execution_depth == 0U &&
        (pool->runtime->root_thread == 0 ||
         !pool->runtime->root_thread->turn_active);
}

static int lm_native_message_thread_control_allowed_locked(
    const LmMessageThreadPool *pool,
    const LmMessageThread *thread
) {
    if (lm_native_message_thread_pool_lifecycle_allowed(pool)) {
        return 1;
    }
    if (pool->single_mode) {
        return pool->runtime->single_active_thread == thread;
    }
    return thread != 0 && thread->executing &&
        lm_native_thread_identity_is_current(thread->execution_identity);
}

static int lm_native_message_thread_invoke(
    LmMessageThread *thread,
    LmMessageThreadEntry entry,
    void *argument
) {
    LmMessageThreadExecutionContext context = {0};
    void *previous;
    int status = 0;

    previous = lm_message_thread_set_execution_context(thread, &context);
    if (setjmp(context.diagnostic_root) == 0) {
        entry(thread, argument);
    } else {
        status = context.diagnostic_code == 0 ? 1 : context.diagnostic_code;
        lm_message_thread_request_failure(
            thread,
            status
        );
    }
    (void)lm_message_thread_set_execution_context(thread, previous);
    return status;
}

static void lm_native_message_thread_pool_stop_locked(
    LmMessageThreadPool *pool
) {
    LmMessageThread *thread;

    pool->stop_requested = 1;
    thread = pool->member_head;
    while (thread != 0) {
        if (lm_native_message_thread_state_lock(thread) == 0) {
            if (thread->state == LM_MESSAGE_THREAD_RUNNING) {
                thread->stop_status = 0;
                thread->state = LM_MESSAGE_THREAD_STOPPING;
                lm_native_message_thread_schedule_stop_locked(pool, thread);
            } else if (thread->state == LM_MESSAGE_THREAD_NEW) {
                thread->state = LM_MESSAGE_THREAD_STOPPED;
                (void)lm_condition_broadcast(thread->stopped_condition);
            }
            (void)lm_native_message_thread_state_unlock(thread);
        }
        thread = thread->pool_next;
    }
}

static int lm_native_message_thread_pool_is_quiescent_locked(
    LmMessageThreadPool *pool
) {
    LmMessageThread *thread;

    if (pool == 0 || pool->ready_head != 0) {
        return 0;
    }
    thread = pool->member_head;
    while (thread != 0) {
        int busy;

        if (lm_native_message_thread_state_lock(thread) != 0) {
            abort();
        }
        busy = thread->scheduled || thread->executing ||
            thread->current_message != 0 || thread->outbox_head != 0 ||
            (thread->state == LM_MESSAGE_THREAD_RUNNING &&
             thread->mailbox_mode && thread->inbox_head != 0);
        (void)lm_native_message_thread_state_unlock(thread);
        if (busy) {
            return 0;
        }
        thread = thread->pool_next;
    }
    return 1;
}

static void lm_native_message_thread_pool_maybe_finish_drain_locked(
    LmMessageThreadPool *pool
) {
    if (pool != 0 && pool->drain_requested && !pool->stop_requested &&
        lm_native_message_thread_pool_is_quiescent_locked(pool)) {
        lm_native_message_thread_pool_stop_locked(pool);
        (void)lm_condition_broadcast(pool->work_ready);
    }
}

static int lm_native_message_thread_pool_begin_turn(
    LmMessageThread *thread
) {
    if (lm_native_message_thread_state_lock(thread) != 0) {
        abort();
    }
    if (thread->state != LM_MESSAGE_THREAD_RUNNING) {
        (void)lm_native_message_thread_state_unlock(thread);
        return 0;
    }
    if (thread->mailbox_mode) {
        LmMessage *message = thread->inbox_head;

        if (message == 0 || thread->current_message != 0) {
            (void)lm_native_message_thread_state_unlock(thread);
            return 0;
        }
        thread->inbox_head = message->next;
        if (thread->inbox_head == 0) {
            thread->inbox_tail = 0;
        }
        thread->inbox_count -= 1U;
        message->next = 0;
        thread->current_message = message;
    }
    thread->turn_active = 1;
    thread->turn_failed = 0;
    thread->turn_count += 1U;
    (void)lm_native_message_thread_state_unlock(thread);
    return 1;
}

static int lm_native_message_thread_pool_collect(
    LmMessageThread *thread
) {
    if (lm_native_message_thread_state_lock(thread) != 0) {
        abort();
    }
    thread->collection_count += 1U;
    (void)lm_native_message_thread_state_unlock(thread);

    if (thread->root_owner == 0 ||
        thread->root_owner->owner_thread != thread ||
        thread->arena_head != thread->root_owner ||
        thread->root_owner->registry_previous != 0 ||
        thread->arena_tail == 0 ||
        thread->arena_tail->registry_next != 0 ||
        thread->arena_count == 0U) {
        if (lm_native_message_thread_state_lock(thread) != 0) {
            abort();
        }
        thread->collector_failed = 1;
        (void)lm_native_message_thread_state_unlock(thread);
        lm_message_thread_request_failure(thread, 1);
        return 1;
    }
    if (lm_own_tree_cut(thread, thread->root_owner) != 0) {
        if (lm_native_message_thread_state_lock(thread) != 0) {
            abort();
        }
        thread->collector_failed = 1;
        (void)lm_native_message_thread_state_unlock(thread);
        lm_message_thread_request_failure(thread, 1);
        return 1;
    }
    return 0;
}

static int lm_native_message_thread_pool_end_turn(
    LmMessageThread *thread
) {
    int status = lm_native_message_thread_pool_collect(thread);

    if (lm_native_message_thread_state_lock(thread) != 0) {
        abort();
    }
    thread->turn_active = 0;
    if (thread->state == LM_MESSAGE_THREAD_STOPPING) {
        thread->state = LM_MESSAGE_THREAD_STOPPED;
    }
    if (thread->state == LM_MESSAGE_THREAD_STOPPED &&
        thread->stopped_condition != 0) {
        (void)lm_condition_broadcast(thread->stopped_condition);
    }
    (void)lm_native_message_thread_state_unlock(thread);
    return status;
}

static int lm_native_message_thread_mark_exit_ready(
    LmMessageThread *thread
) {
    LmMessageThreadRuntime *runtime;
    int effective_status;
    int turn_failed;
    int became_ready = 0;

    if (thread == 0) {
        return 0;
    }
    if (lm_native_message_thread_state_lock(thread) != 0) {
        return 0;
    }
    runtime = thread->runtime;
    effective_status = thread->stop_status;
    turn_failed = thread->turn_failed;
    (void)lm_native_message_thread_state_unlock(thread);
    if (runtime == 0) {
        return 0;
    }
    if (runtime->route_mutex == 0 ||
        lm_mutex_lock(runtime->route_mutex) != 0) {
        return 0;
    }
    if (runtime->exit_requested && !runtime->exit_ready &&
        runtime->exit_requester == thread) {
        if (turn_failed) {
            runtime->exit_status = effective_status;
        }
        runtime->exit_ready = 1;
        runtime->exit_requester = 0;
        became_ready = 1;
    }
    (void)lm_mutex_unlock(runtime->route_mutex);
    return became_ready;
}

static int lm_native_message_thread_deliver_remote(
    LmMessageThreadRuntime *runtime,
    const char *endpoint,
    const char *route,
    const LmMessage *message
) {
    LmRestLmxPost post;
    void *context;
    char *normalized_uri;
    unsigned http_status = 0U;
    int transport_status;

    if (runtime == 0 || message == 0 || runtime->route_mutex == 0 ||
        lm_mutex_lock(runtime->route_mutex) != 0) {
        return 1;
    }
    if (runtime->exit_ready) {
        (void)lm_mutex_unlock(runtime->route_mutex);
        return LM_MESSAGE_STATUS_EXIT_DISCARDED;
    }
    post = runtime->rest_lmx_post;
    context = runtime->rest_lmx_context;
    (void)lm_mutex_unlock(runtime->route_mutex);
    if (post == 0) {
        return LM_MESSAGE_STATUS_TRANSPORT_PROVIDER_NOT_CONFIGURED;
    }

    normalized_uri = lm_native_message_thread_join_uri(endpoint, route);
    if (normalized_uri == 0) {
        return 1;
    }
    transport_status = post(
        context,
        normalized_uri,
        message->lmx,
        message->length,
        &http_status
    );
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

static int lm_native_message_thread_deliver_local(
    LmMessageThreadRuntime *runtime,
    const char *route,
    LmMessage *message,
    int reject_exit_requested
) {
    LmMessageRoute *binding;
    LmMessageThread *target;
    LmMessageThreadPool *pool;
    int status = LM_MESSAGE_STATUS_ROUTE_NOT_FOUND;

    if (runtime == 0 || runtime->route_mutex == 0 ||
        lm_mutex_lock(runtime->route_mutex) != 0) {
        return 1;
    }
    if (reject_exit_requested && runtime->exit_requested) {
        (void)lm_mutex_unlock(runtime->route_mutex);
        return LM_MESSAGE_STATUS_APPLICATION_STOPPING;
    }
    if (runtime->exit_ready) {
        (void)lm_mutex_unlock(runtime->route_mutex);
        return LM_MESSAGE_STATUS_EXIT_DISCARDED;
    }
    binding = runtime->route_head;
    while (binding != 0 && strcmp(binding->route, route) != 0) {
        binding = binding->next;
    }
    if (binding == 0) {
        (void)lm_mutex_unlock(runtime->route_mutex);
        return status;
    }
    if (binding->target == 0 || binding->target->pool == 0) {
        (void)lm_mutex_unlock(runtime->route_mutex);
        return reject_exit_requested
            ? LM_MESSAGE_STATUS_APPLICATION_STOPPING
            : status;
    }

    target = binding->target;
    pool = target->pool;
    if (pool->runtime != runtime || lm_mutex_lock(pool->mutex) != 0) {
        (void)lm_mutex_unlock(runtime->route_mutex);
        return 1;
    }
    if (lm_native_message_thread_state_lock(target) != 0) {
        (void)lm_mutex_unlock(pool->mutex);
        (void)lm_mutex_unlock(runtime->route_mutex);
        return 1;
    }
    if (!pool->stop_requested && !pool->deleting &&
        (!reject_exit_requested || !pool->drain_requested) &&
        target->pool == pool &&
        target->mailbox_mode && target->started &&
        target->state == LM_MESSAGE_THREAD_RUNNING &&
        target->inbox_count != (size_t)-1) {
        message->next = 0;
        if (target->inbox_tail == 0) {
            target->inbox_head = message;
        } else {
            target->inbox_tail->next = message;
        }
        target->inbox_tail = message;
        target->inbox_count += 1U;
        if (!target->scheduled && !target->executing) {
            lm_native_message_thread_ready_push_locked(pool, target);
            (void)lm_condition_signal(pool->work_ready);
        }
        status = 0;
    } else if (reject_exit_requested &&
        (pool->stop_requested || pool->drain_requested || pool->deleting ||
         target->pool != pool || !target->mailbox_mode || !target->started ||
         target->state != LM_MESSAGE_THREAD_RUNNING)) {
        status = LM_MESSAGE_STATUS_APPLICATION_STOPPING;
    }
    (void)lm_native_message_thread_state_unlock(target);
    (void)lm_mutex_unlock(pool->mutex);
    (void)lm_mutex_unlock(runtime->route_mutex);
    return status;
}

static LmMessageOutboxEntry *lm_native_message_thread_outbox_take(
    LmMessageThread *thread
) {
    LmMessageOutboxEntry *head;

    if (lm_native_message_thread_state_lock(thread) != 0) {
        return 0;
    }
    head = thread->outbox_head;
    thread->outbox_head = 0;
    thread->outbox_tail = 0;
    thread->outbox_count = 0U;
    (void)lm_native_message_thread_state_unlock(thread);
    return head;
}

static int lm_native_message_thread_outbox_finish(
    LmMessageThread *thread,
    int commit
) {
    LmMessageOutboxEntry *entry =
        lm_native_message_thread_outbox_take(thread);
    int first_status = 0;

    while (entry != 0) {
        LmMessageOutboxEntry *next = entry->next;
        int status = 0;

        entry->next = 0;
        if (commit) {
            if (entry->endpoint == 0 || entry->endpoint[0] == '\0') {
                status = lm_native_message_thread_deliver_local(
                    thread->runtime,
                    entry->route,
                    entry->message,
                    0
                );
                if (status == 0) {
                    entry->message = 0;
                }
            } else {
                status = lm_native_message_thread_deliver_remote(
                    thread->runtime,
                    entry->endpoint,
                    entry->route,
                    entry->message
                );
            }
        }
        if (first_status == 0 && status != 0 &&
            status != LM_MESSAGE_STATUS_EXIT_DISCARDED) {
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

static void lm_native_message_thread_current_message_finish(
    LmMessageThread *thread
) {
    LmMessage *message;

    if (lm_native_message_thread_state_lock(thread) != 0) {
        abort();
    }
    message = thread->current_message;
    thread->current_message = 0;
    (void)lm_native_message_thread_state_unlock(thread);
    lm_native_message_thread_message_delete(message);
}

static void lm_native_message_thread_execute(
    LmMessageThreadPool *pool,
    LmMessageThread *thread
) {
    LmMessageThreadRuntime *runtime = 0;
    LmMessageThread *previous_active_thread = 0;
    int run_entry;
    int invoke_status = 0;
    int commit_allowed = 0;
    int exit_became_ready = 0;

    if (pool->single_mode) {
        runtime = pool->runtime;
        previous_active_thread = runtime->single_active_thread;
        runtime->single_active_thread = thread;
        runtime->single_execution_depth += 1U;
    }
    run_entry = lm_native_message_thread_pool_begin_turn(thread);
    if (run_entry) {
        if (thread->entry == 0) {
            lm_message_thread_request_failure(thread, 1);
            invoke_status = 1;
        } else {
            invoke_status = lm_native_message_thread_invoke(
                thread,
                thread->entry,
                thread->entry_argument
            );
        }
    }
    if (lm_native_message_thread_state_lock(thread) != 0) {
        abort();
    }
    commit_allowed = run_entry && invoke_status == 0 &&
        !thread->turn_failed;
    (void)lm_native_message_thread_state_unlock(thread);
    (void)lm_native_message_thread_outbox_finish(thread, commit_allowed);
    lm_native_message_thread_current_message_finish(thread);
    (void)lm_native_message_thread_pool_end_turn(thread);
    exit_became_ready = lm_native_message_thread_mark_exit_ready(thread);
    if (runtime != 0) {
        runtime->single_execution_depth -= 1U;
        runtime->single_active_thread = previous_active_thread;
    }

    if (lm_mutex_lock(pool->mutex) != 0) {
        abort();
    }
    if (lm_native_message_thread_state_lock(thread) != 0) {
        abort();
    }

    thread->execution_identity = 0;
    thread->executing = 0;
    (void)lm_native_message_thread_state_unlock(thread);
    if (exit_became_ready) {
        lm_native_message_thread_pool_stop_locked(pool);
    }
    if (lm_native_message_thread_state_lock(thread) != 0) {
        abort();
    }
    if (pool->stop_requested &&
        thread->state == LM_MESSAGE_THREAD_RUNNING) {
        thread->stop_status = 0;
        thread->state = LM_MESSAGE_THREAD_STOPPING;
    }
    if (thread->state == LM_MESSAGE_THREAD_STOPPING &&
        !thread->scheduled) {
        lm_native_message_thread_schedule_stop_locked(pool, thread);
    }
    if (thread->state == LM_MESSAGE_THREAD_RUNNING &&
        ((thread->mailbox_mode && thread->inbox_head != 0) ||
         (!thread->mailbox_mode && !pool->drain_requested))) {
        if (!thread->scheduled) {
            lm_native_message_thread_ready_push_locked(pool, thread);
            (void)lm_condition_signal(pool->work_ready);
        }
    } else {
        (void)lm_condition_broadcast(thread->stopped_condition);
    }

    (void)lm_native_message_thread_state_unlock(thread);
    lm_native_message_thread_pool_maybe_finish_drain_locked(pool);
    (void)lm_mutex_unlock(pool->mutex);
}

static void *lm_native_message_thread_worker(void *argument) {
    LmMessageThreadPool *pool = (LmMessageThreadPool *)argument;
    void *worker_identity = lm_native_thread_identity_new_current();

    if (worker_identity == 0) {
        abort();
    }

    for (;;) {
        LmMessageThread *thread;

        if (lm_mutex_lock(pool->mutex) != 0) {
            abort();
        }
        while (pool->ready_head == 0 && !pool->stop_requested) {
            if (lm_condition_wait(pool->work_ready, pool->mutex) != 0) {
                abort();
            }
        }
        if (pool->ready_head == 0 && pool->stop_requested) {
            (void)lm_mutex_unlock(pool->mutex);
            lm_native_thread_identity_delete(worker_identity);
            return 0;
        }

        thread = lm_native_message_thread_ready_pop_locked(pool);
        if (thread == 0 ||
            lm_native_message_thread_state_lock(thread) != 0) {
            abort();
        }
        thread->scheduled = 0;
        thread->executing = 1;
        thread->execution_identity = worker_identity;
        (void)lm_native_message_thread_state_unlock(thread);
        (void)lm_mutex_unlock(pool->mutex);

        lm_native_message_thread_execute(pool, thread);
    }
}

static int lm_native_message_thread_pool_pump_one(
    LmMessageThreadPool *pool
) {
    LmMessageThread *thread;

    if (pool == 0 || !pool->single_mode) {
        return -1;
    }
    if (lm_mutex_lock(pool->mutex) != 0) {
        abort();
    }
    thread = lm_native_message_thread_ready_pop_locked(pool);
    if (thread == 0) {
        (void)lm_mutex_unlock(pool->mutex);
        return 0;
    }
    if (lm_native_message_thread_state_lock(thread) != 0) {
        abort();
    }
    thread->scheduled = 0;
    thread->executing = 1;
    (void)lm_native_message_thread_state_unlock(thread);
    (void)lm_mutex_unlock(pool->mutex);

    lm_native_message_thread_execute(pool, thread);
    return 1;
}

size_t lm_message_thread_pool_pump(
    LmMessageThreadPool *pool,
    size_t max_turns
) {
    size_t completed = 0U;

    if (pool == 0 || max_turns == 0U || !pool->single_mode ||
        pool->deleting ||
        !lm_native_message_thread_pool_lifecycle_allowed(pool)) {
        return 0U;
    }
    while (completed < max_turns &&
           lm_native_message_thread_pool_pump_one(pool) > 0) {
        completed += 1U;
    }
    return completed;
}

LmMessageThreadRuntime *lm_message_thread_runtime_new(void) {
    LmMessageThreadRuntime *runtime =
        (LmMessageThreadRuntime *)calloc(1U, sizeof(*runtime));

    if (runtime == 0) {
        return 0;
    }
    runtime->identity = lm_native_thread_identity_new_current();
    runtime->route_mutex = lm_mutex_new();
    if (runtime->identity == 0 || runtime->route_mutex == 0) {
        lm_mutex_delete(runtime->route_mutex);
        lm_native_thread_identity_delete(runtime->identity);
        free(runtime);
        return 0;
    }
    return runtime;
}

int lm_message_thread_runtime_set_rest_lmx_provider(
    LmMessageThreadRuntime *runtime,
    const LmRestLmxProviderOpsV1 *ops,
    void *context
) {
    LmRestLmxPost new_post = 0;
    LmRestLmxDestroy new_destroy = 0;
    LmRestLmxPost old_post;
    LmRestLmxDestroy old_destroy;
    void *old_context;
    int destroy_old = 0;

    if (runtime == 0 || runtime->route_mutex == 0 ||
        !lm_native_thread_identity_is_current(runtime->identity)) {
        return 1;
    }
    if (ops == 0) {
        if (context != 0) {
            return 1;
        }
    } else {
        if (ops->abi_size != sizeof(*ops) || ops->post == 0) {
            return 1;
        }
        new_post = ops->post;
        new_destroy = ops->destroy;
    }
    if (lm_mutex_lock(runtime->route_mutex) != 0) {
        return 1;
    }
    if (runtime->rest_lmx_provider_sealed ||
        runtime->rest_lmx_provider_transition || runtime->deleting ||
        runtime->pool_count != 0U ||
        runtime->single_execution_depth != 0U ||
        runtime->route_count != 0U || runtime->route_head != 0 ||
        runtime->root_thread != 0 || runtime->exit_requester != 0 ||
        runtime->exit_requested || runtime->exit_ready) {
        (void)lm_mutex_unlock(runtime->route_mutex);
        return 1;
    }
    old_post = runtime->rest_lmx_post;
    old_destroy = runtime->rest_lmx_destroy;
    old_context = runtime->rest_lmx_context;
    if (old_post == new_post && old_destroy == new_destroy &&
        old_context == context) {
        (void)lm_mutex_unlock(runtime->route_mutex);
        return 0;
    }
    if (old_post != 0 && old_context != 0 && old_context == context) {
        (void)lm_mutex_unlock(runtime->route_mutex);
        return 1;
    }
    destroy_old = old_post != 0 && old_destroy != 0;
    runtime->rest_lmx_provider_transition = destroy_old;
    runtime->rest_lmx_post = new_post;
    runtime->rest_lmx_destroy = new_destroy;
    runtime->rest_lmx_context = context;
    (void)lm_mutex_unlock(runtime->route_mutex);

    if (destroy_old) {
        old_destroy(old_context);
        if (lm_mutex_lock(runtime->route_mutex) != 0) {
            abort();
        }
        if (!runtime->rest_lmx_provider_transition) {
            abort();
        }
        runtime->rest_lmx_provider_transition = 0;
        (void)lm_mutex_unlock(runtime->route_mutex);
    }
    return 0;
}

int lm_message_thread_runtime_admit_lmx(
    LmMessageThreadRuntime *runtime,
    const char *route,
    const char *lmx,
    size_t length
) {
    LmMessage *message;
    int status;

    if (!lm_native_message_thread_route_is_valid(route)) {
        return LM_MESSAGE_STATUS_INVALID_ADDRESS;
    }
    if (runtime == 0 || lmx == 0 || length == (size_t)-1) {
        return 1;
    }
    message = lm_native_message_thread_message_new(lmx, length);
    if (message == 0) {
        return 1;
    }
    status = lm_native_message_thread_deliver_local(
        runtime,
        route,
        message,
        1
    );
    if (status != 0) {
        lm_native_message_thread_message_delete(message);
    }
    return status;
}

int lm_message_thread_request_exit(
    LmMessageThread *requester,
    int status
) {
    LmMessageThreadRuntime *runtime;
    int valid = 0;
    int selected_status;

    if (requester == 0 ||
        lm_native_message_thread_state_lock(requester) != 0) {
        return 1;
    }
    runtime = requester->runtime;
    if (runtime != 0 && lm_native_message_thread_is_current(requester) &&
        ((requester->pool == 0 && runtime->root_thread == requester) ||
         (requester->pool != 0 && requester->pool->runtime == runtime))) {
        valid = 1;
    }
    (void)lm_native_message_thread_state_unlock(requester);
    if (!valid || runtime->route_mutex == 0 ||
        lm_mutex_lock(runtime->route_mutex) != 0) {
        return 1;
    }
    if (!runtime->exit_requested) {
        runtime->exit_requester = requester;
        runtime->exit_requested = 1;
        runtime->exit_ready = 0;
        runtime->exit_status = status;
    }
    selected_status = runtime->exit_status;
    (void)lm_mutex_unlock(runtime->route_mutex);
    lm_message_thread_request_stop(requester, selected_status);
    return 0;
}

int lm_message_thread_runtime_exit_state(
    LmMessageThreadRuntime *runtime,
    int *out_requested,
    int *out_ready,
    int *out_status
) {
    if (out_requested == 0 || out_ready == 0 || out_status == 0) {
        return 1;
    }
    *out_requested = 0;
    *out_ready = 0;
    *out_status = 0;
    if (runtime == 0 || runtime->route_mutex == 0 ||
        lm_mutex_lock(runtime->route_mutex) != 0) {
        return 1;
    }
    *out_requested = runtime->exit_requested;
    *out_ready = runtime->exit_ready;
    *out_status = runtime->exit_status;
    (void)lm_mutex_unlock(runtime->route_mutex);
    return 0;
}

int lm_message_thread_runtime_attach_root(
    LmMessageThreadRuntime *runtime,
    LmMessageThread *thread
) {
    int status = 1;

    if (runtime == 0 || thread == 0 || runtime->route_mutex == 0 ||
        !lm_native_thread_identity_is_current(runtime->identity) ||
        lm_mutex_lock(runtime->route_mutex) != 0) {
        return 1;
    }
    if (runtime->deleting || runtime->rest_lmx_provider_transition) {
        (void)lm_mutex_unlock(runtime->route_mutex);
        return 1;
    }
    if (lm_native_message_thread_state_lock(thread) == 0) {
        if (runtime->root_thread == 0 && thread->runtime == 0 &&
            thread->pool == 0 && !thread->started && !thread->turn_active &&
            !thread->executing && thread->state == LM_MESSAGE_THREAD_RUNNING &&
            thread->outbox_head == 0 && thread->current_message == 0) {
            runtime->root_thread = thread;
            runtime->rest_lmx_provider_sealed = 1;
            thread->runtime = runtime;
            status = 0;
        }
        (void)lm_native_message_thread_state_unlock(thread);
    }
    (void)lm_mutex_unlock(runtime->route_mutex);
    return status;
}

int lm_message_thread_runtime_detach_root(
    LmMessageThreadRuntime *runtime,
    LmMessageThread *thread
) {
    int status = 1;

    if (runtime == 0 || thread == 0 || runtime->route_mutex == 0 ||
        !lm_native_thread_identity_is_current(runtime->identity) ||
        lm_mutex_lock(runtime->route_mutex) != 0) {
        return 1;
    }
    if (lm_native_message_thread_state_lock(thread) == 0) {
        if (runtime->root_thread == thread && thread->runtime == runtime &&
            thread->pool == 0 && !thread->turn_active &&
            !thread->executing && thread->outbox_head == 0) {
            runtime->root_thread = 0;
            thread->runtime = 0;
            status = 0;
        }
        (void)lm_native_message_thread_state_unlock(thread);
    }
    (void)lm_mutex_unlock(runtime->route_mutex);
    return status;
}

int lm_message_thread_runtime_delete(
    LmMessageThreadRuntime *runtime
) {
    LmRestLmxPost old_post;
    LmRestLmxDestroy old_destroy;
    void *old_context;

    if (runtime == 0) {
        return 0;
    }
    if (!lm_native_thread_identity_is_current(runtime->identity) ||
        runtime->route_mutex == 0 ||
        lm_mutex_lock(runtime->route_mutex) != 0) {
        return 1;
    }
    if (runtime->deleting || runtime->rest_lmx_provider_transition ||
        runtime->pool_count != 0U ||
        runtime->single_execution_depth != 0U ||
        runtime->route_count != 0U || runtime->route_head != 0 ||
        runtime->root_thread != 0) {
        (void)lm_mutex_unlock(runtime->route_mutex);
        return 1;
    }
    old_post = runtime->rest_lmx_post;
    old_destroy = runtime->rest_lmx_destroy;
    old_context = runtime->rest_lmx_context;
    runtime->rest_lmx_post = 0;
    runtime->rest_lmx_destroy = 0;
    runtime->rest_lmx_context = 0;
    runtime->rest_lmx_provider_sealed = 1;
    runtime->deleting = 1;
    (void)lm_mutex_unlock(runtime->route_mutex);
    if (old_post != 0 && old_destroy != 0) {
        old_destroy(old_context);
    }
    lm_mutex_delete(runtime->route_mutex);
    runtime->route_mutex = 0;
    lm_native_thread_identity_delete(runtime->identity);
    runtime->identity = 0;
    free(runtime);
    return 0;
}

static int lm_native_message_thread_runtime_acquire_pool(
    LmMessageThreadRuntime *runtime
) {
    if (runtime == 0 || runtime->route_mutex == 0 ||
        lm_mutex_lock(runtime->route_mutex) != 0) {
        return 1;
    }
    if (runtime->deleting || runtime->rest_lmx_provider_transition ||
        runtime->pool_count == (size_t)-1) {
        (void)lm_mutex_unlock(runtime->route_mutex);
        return 1;
    }
    runtime->rest_lmx_provider_sealed = 1;
    runtime->pool_count += 1U;
    (void)lm_mutex_unlock(runtime->route_mutex);
    return 0;
}

static void lm_native_message_thread_runtime_release_pool(
    LmMessageThreadRuntime *runtime
) {
    if (runtime == 0 || runtime->route_mutex == 0 ||
        lm_mutex_lock(runtime->route_mutex) != 0) {
        abort();
    }
    if (runtime->pool_count == 0U) {
        abort();
    }
    runtime->pool_count -= 1U;
    (void)lm_mutex_unlock(runtime->route_mutex);
}

LmMessageThreadPool *lm_message_thread_pool_new(
    LmMessageThreadRuntime *runtime,
    size_t worker_count
) {
    LmMessageThreadPool *pool;
    size_t index;

    if (runtime == 0 || worker_count == 0U ||
        !lm_native_thread_identity_is_current(runtime->identity)) {
        return 0;
    }
    if (runtime->single_execution_depth != 0U) {
        return 0;
    }
    pool = (LmMessageThreadPool *)calloc(1U, sizeof(*pool));
    if (pool == 0) {
        return 0;
    }
    pool->runtime = runtime;
    pool->mutex = lm_mutex_new();
    pool->work_ready = lm_condition_new();
    pool->single_mode = strcmp(lm_thread_provider_name(), "single") == 0;
    if (pool->mutex == 0 || pool->work_ready == 0) {
        lm_condition_delete(pool->work_ready);
        lm_mutex_delete(pool->mutex);
        free(pool);
        return 0;
    }
    if (lm_native_message_thread_runtime_acquire_pool(runtime) != 0) {
        lm_condition_delete(pool->work_ready);
        lm_mutex_delete(pool->mutex);
        free(pool);
        return 0;
    }
    if (pool->single_mode) {
        return pool;
    }

    pool->worker_count = worker_count;
    pool->workers = (LmHostThread **)calloc(
        worker_count,
        sizeof(pool->workers[0])
    );
    if (pool->workers == 0) {
        lm_condition_delete(pool->work_ready);
        lm_mutex_delete(pool->mutex);
        lm_native_message_thread_runtime_release_pool(runtime);
        free(pool);
        return 0;
    }

    for (index = 0U; index < worker_count; index += 1U) {
        pool->workers[index] = lm_host_thread_new();
        if (pool->workers[index] == 0 ||
            lm_host_thread_start(
                pool->workers[index],
                lm_native_message_thread_worker,
                pool
            ) != 0) {
            size_t cleanup_index;
            int cleanup_failed = 0;

            if (lm_mutex_lock(pool->mutex) != 0) {
                return 0;
            }
            lm_native_message_thread_pool_stop_locked(pool);
            (void)lm_condition_broadcast(pool->work_ready);
            (void)lm_mutex_unlock(pool->mutex);
            for (cleanup_index = 0U;
                 cleanup_index < pool->started_worker_count;
                 cleanup_index += 1U) {
                if (lm_host_thread_join(
                        pool->workers[cleanup_index],
                        0
                    ) != 0) {
                    cleanup_failed = 1;
                }
            }
            if (cleanup_failed) {
                return 0;
            }
            for (cleanup_index = 0U;
                 cleanup_index <= index;
                 cleanup_index += 1U) {
                lm_host_thread_delete(pool->workers[cleanup_index]);
            }
            free(pool->workers);
            lm_condition_delete(pool->work_ready);
            lm_mutex_delete(pool->mutex);
            lm_native_message_thread_runtime_release_pool(runtime);
            free(pool);
            return 0;
        }
        pool->started_worker_count += 1U;
    }
    return pool;
}

void lm_message_thread_pool_request_stop(LmMessageThreadPool *pool) {
    if (pool == 0 || lm_mutex_lock(pool->mutex) != 0) {
        return;
    }
    if (pool->deleting ||
        !lm_native_message_thread_pool_lifecycle_allowed(pool)) {
        (void)lm_mutex_unlock(pool->mutex);
        return;
    }
    lm_native_message_thread_pool_stop_locked(pool);
    (void)lm_condition_broadcast(pool->work_ready);
    (void)lm_mutex_unlock(pool->mutex);
}

void lm_message_thread_pool_request_stop_when_idle(
    LmMessageThreadPool *pool
) {
    if (pool == 0 || lm_mutex_lock(pool->mutex) != 0) {
        return;
    }
    if (pool->deleting ||
        !lm_native_message_thread_pool_lifecycle_allowed(pool)) {
        (void)lm_mutex_unlock(pool->mutex);
        return;
    }
    pool->drain_requested = 1;
    lm_native_message_thread_pool_maybe_finish_drain_locked(pool);
    (void)lm_condition_broadcast(pool->work_ready);
    (void)lm_mutex_unlock(pool->mutex);
}

LmMessageThread *lm_message_thread_new_in(LmMessageThreadPool *pool) {
    LmMessageThread *thread;

    if (pool == 0 ||
        !lm_native_message_thread_pool_lifecycle_allowed(pool)) {
        return 0;
    }
    thread = lm_message_thread_new();
    if (thread == 0) {
        return 0;
    }
    thread->state_mutex = lm_mutex_new();
    thread->stopped_condition = lm_condition_new();
    if (thread->state_mutex == 0 || thread->stopped_condition == 0) {
        lm_message_thread_delete(thread);
        return 0;
    }
    thread->state = LM_MESSAGE_THREAD_NEW;

    if (lm_mutex_lock(pool->mutex) != 0) {
        lm_message_thread_delete(thread);
        return 0;
    }
    if (pool->stop_requested || pool->drain_requested || pool->deleting) {
        (void)lm_mutex_unlock(pool->mutex);
        lm_message_thread_delete(thread);
        return 0;
    }
    thread->pool = pool;
    thread->runtime = pool->runtime;
    thread->pool_previous = pool->member_tail;
    if (pool->member_tail == 0) {
        pool->member_head = thread;
    } else {
        pool->member_tail->pool_next = thread;
    }
    pool->member_tail = thread;
    pool->member_count += 1U;
    (void)lm_mutex_unlock(pool->mutex);
    return thread;
}

static int lm_native_message_thread_start(
    LmMessageThread *thread,
    LmMessageThreadEntry entry,
    void *argument,
    int mailbox_mode
) {
    LmMessageThreadPool *pool;

    if (thread == 0 || entry == 0 || thread->pool == 0) {
        return 1;
    }
    pool = thread->pool;
    if (!lm_native_message_thread_pool_lifecycle_allowed(pool)) {
        return 1;
    }
    if (lm_mutex_lock(pool->mutex) != 0) {
        return 1;
    }
    if (lm_native_message_thread_state_lock(thread) != 0) {
        (void)lm_mutex_unlock(pool->mutex);
        return 1;
    }
    if (thread->pool != pool || pool->stop_requested ||
        pool->drain_requested || pool->deleting ||
        thread->state != LM_MESSAGE_THREAD_NEW || thread->started ||
        thread->scheduled || thread->executing) {
        (void)lm_native_message_thread_state_unlock(thread);
        (void)lm_mutex_unlock(pool->mutex);
        return 1;
    }

    thread->entry = entry;
    thread->entry_argument = argument;
    thread->mailbox_mode = mailbox_mode;
    thread->started = 1;
    thread->state = LM_MESSAGE_THREAD_RUNNING;
    if (!mailbox_mode) {
        lm_native_message_thread_ready_push_locked(pool, thread);
        (void)lm_condition_signal(pool->work_ready);
    }
    (void)lm_native_message_thread_state_unlock(thread);
    (void)lm_mutex_unlock(pool->mutex);
    return 0;
}

int lm_message_thread_start(
    LmMessageThread *thread,
    LmMessageThreadEntry entry,
    void *argument
) {
    return lm_native_message_thread_start(thread, entry, argument, 0);
}

int lm_message_thread_start_mailbox(
    LmMessageThread *thread,
    LmMessageThreadEntry entry,
    void *argument
) {
    return lm_native_message_thread_start(thread, entry, argument, 1);
}

int lm_message_thread_bind_route(
    LmMessageThread *thread,
    const char *route
) {
    LmMessageThreadPool *pool;
    LmMessageThreadRuntime *runtime;
    LmMessageRoute *binding;
    LmMessageRoute *cursor;

    if (thread == 0 || thread->pool == 0 ||
        !lm_native_message_thread_route_is_valid(route)) {
        return LM_MESSAGE_STATUS_INVALID_ADDRESS;
    }
    pool = thread->pool;
    runtime = pool->runtime;
    if (runtime == 0 ||
        !lm_native_message_thread_pool_lifecycle_allowed(pool)) {
        return 1;
    }
    binding = (LmMessageRoute *)calloc(1U, sizeof(*binding));
    if (binding == 0) {
        return 1;
    }
    binding->route = lm_native_message_thread_copy_cstr(route);
    if (binding->route == 0) {
        free(binding);
        return 1;
    }
    binding->target = thread;

    if (lm_mutex_lock(runtime->route_mutex) != 0) {
        free(binding->route);
        free(binding);
        return 1;
    }
    cursor = runtime->route_head;
    while (cursor != 0) {
        if (strcmp(cursor->route, route) == 0) {
            (void)lm_mutex_unlock(runtime->route_mutex);
            free(binding->route);
            free(binding);
            return 1;
        }
        cursor = cursor->next;
    }
    if (lm_mutex_lock(pool->mutex) != 0) {
        (void)lm_mutex_unlock(runtime->route_mutex);
        free(binding->route);
        free(binding);
        return 1;
    }
    if (lm_native_message_thread_state_lock(thread) != 0) {
        (void)lm_mutex_unlock(pool->mutex);
        (void)lm_mutex_unlock(runtime->route_mutex);
        free(binding->route);
        free(binding);
        return 1;
    }
    if (pool->runtime != runtime || pool->stop_requested ||
        pool->drain_requested || pool->deleting ||
        thread->pool != pool || !thread->started || !thread->mailbox_mode ||
        thread->state != LM_MESSAGE_THREAD_RUNNING ||
        runtime->route_count == (size_t)-1) {
        (void)lm_native_message_thread_state_unlock(thread);
        (void)lm_mutex_unlock(pool->mutex);
        (void)lm_mutex_unlock(runtime->route_mutex);
        free(binding->route);
        free(binding);
        return 1;
    }
    binding->next = runtime->route_head;
    runtime->route_head = binding;
    runtime->route_count += 1U;
    (void)lm_native_message_thread_state_unlock(thread);
    (void)lm_mutex_unlock(pool->mutex);
    (void)lm_mutex_unlock(runtime->route_mutex);
    return 0;
}

int lm_message_thread_send_lmx(
    LmMessageThread *sender,
    const char *endpoint,
    const char *route,
    const char *lmx,
    size_t length
) {
    LmMessageOutboxEntry *entry;

    if (!lm_native_message_thread_route_is_valid(route) ||
        (endpoint != 0 && endpoint[0] != '\0' &&
         !lm_native_message_thread_endpoint_is_http(endpoint))) {
        return LM_MESSAGE_STATUS_INVALID_ADDRESS;
    }
    entry = (LmMessageOutboxEntry *)calloc(1U, sizeof(*entry));
    if (entry == 0) {
        return 1;
    }
    entry->route = lm_native_message_thread_copy_cstr(route);
    if (endpoint != 0 && endpoint[0] != '\0') {
        entry->endpoint = lm_native_message_thread_copy_cstr(endpoint);
    }
    entry->message = lm_native_message_thread_message_new(lmx, length);
    if (entry->route == 0 || entry->message == 0 ||
        (endpoint != 0 && endpoint[0] != '\0' && entry->endpoint == 0)) {
        lm_native_message_thread_outbox_entry_delete(entry);
        return 1;
    }
    if (sender == 0 || lm_native_message_thread_state_lock(sender) != 0) {
        lm_native_message_thread_outbox_entry_delete(entry);
        return 1;
    }
    if (!lm_native_message_thread_is_current(sender) || sender->runtime == 0 ||
        (sender->pool != 0 && !sender->started) ||
        (sender->pool == 0 && sender->runtime->root_thread != sender) ||
        (sender->state != LM_MESSAGE_THREAD_RUNNING &&
         sender->state != LM_MESSAGE_THREAD_STOPPING) ||
        sender->outbox_count == (size_t)-1) {
        (void)lm_native_message_thread_state_unlock(sender);
        lm_native_message_thread_outbox_entry_delete(entry);
        return 1;
    }
    entry->next = 0;
    if (sender->outbox_tail == 0) {
        sender->outbox_head = entry;
    } else {
        sender->outbox_tail->next = entry;
    }
    sender->outbox_tail = entry;
    sender->outbox_count += 1U;
    (void)lm_native_message_thread_state_unlock(sender);
    return 0;
}

int lm_message_thread_current_lmx(
    LmMessageThread *thread,
    const char **out_lmx,
    size_t *out_length
) {
    if (out_lmx == 0 || out_length == 0) {
        return 1;
    }
    *out_lmx = 0;
    *out_length = 0U;
    if (thread == 0 || lm_native_message_thread_state_lock(thread) != 0) {
        return 1;
    }
    if (!lm_native_message_thread_is_current(thread) ||
        thread->current_message == 0) {
        (void)lm_native_message_thread_state_unlock(thread);
        return 1;
    }
    *out_lmx = thread->current_message->lmx;
    *out_length = thread->current_message->length;
    (void)lm_native_message_thread_state_unlock(thread);
    return 0;
}

size_t lm_message_thread_inbox_count(const LmMessageThread *thread) {
    size_t result;

    if (thread == 0 || lm_native_message_thread_state_lock(thread) != 0) {
        return 0U;
    }
    result = thread->inbox_count;
    (void)lm_native_message_thread_state_unlock(thread);
    return result;
}

size_t lm_message_thread_outbox_count(const LmMessageThread *thread) {
    size_t result;

    if (thread == 0 || lm_native_message_thread_state_lock(thread) != 0) {
        return 0U;
    }
    result = thread->outbox_count;
    (void)lm_native_message_thread_state_unlock(thread);
    return result;
}

int lm_message_thread_join(LmMessageThread *thread, int *result) {
    LmMessageThreadPool *pool;

    if (thread == 0 || result == 0 || thread->pool == 0) {
        return 1;
    }
    pool = thread->pool;
    if (!lm_native_message_thread_pool_lifecycle_allowed(pool)) {
        return 1;
    }
    if (lm_mutex_lock(pool->mutex) != 0) {
        return 1;
    }
    if (lm_native_message_thread_state_lock(thread) != 0) {
        (void)lm_mutex_unlock(pool->mutex);
        return 1;
    }
    if (thread->pool != pool || pool->deleting ||
        !thread->started ||
        thread->joining || thread->joined) {
        (void)lm_native_message_thread_state_unlock(thread);
        (void)lm_mutex_unlock(pool->mutex);
        return 1;
    }
    thread->joining = 1;
    (void)lm_mutex_unlock(pool->mutex);

    if (!pool->single_mode) {
        while (thread->state != LM_MESSAGE_THREAD_STOPPED ||
               thread->scheduled || thread->executing) {
            if (lm_condition_wait(
                    thread->stopped_condition,
                    thread->state_mutex
                ) != 0) {
                thread->joining = 0;
                (void)lm_condition_broadcast(thread->stopped_condition);
                (void)lm_native_message_thread_state_unlock(thread);
                return 1;
            }
        }
        *result = thread->stop_status;
        thread->joined = 1;
        thread->joining = 0;
        (void)lm_condition_broadcast(thread->stopped_condition);
        (void)lm_native_message_thread_state_unlock(thread);
        return 0;
    }

    (void)lm_native_message_thread_state_unlock(thread);
    for (;;) {
        int stopped;
        int pump_status;

        if (lm_native_message_thread_state_lock(thread) != 0) {
            return 1;
        }
        stopped = thread->state == LM_MESSAGE_THREAD_STOPPED &&
            !thread->scheduled && !thread->executing;
        if (stopped) {
            *result = thread->stop_status;
            thread->joined = 1;
            thread->joining = 0;
            (void)lm_condition_broadcast(thread->stopped_condition);
            (void)lm_native_message_thread_state_unlock(thread);
            return 0;
        }
        (void)lm_native_message_thread_state_unlock(thread);

        pump_status = lm_native_message_thread_pool_pump_one(pool);
        if (pump_status <= 0) {
            if (lm_native_message_thread_state_lock(thread) == 0) {
                thread->joining = 0;
                (void)lm_condition_broadcast(thread->stopped_condition);
                (void)lm_native_message_thread_state_unlock(thread);
            }
            return 1;
        }
    }
}

static void lm_native_message_thread_remove_pool_routes_locked(
    LmMessageThreadRuntime *runtime,
    LmMessageThreadPool *pool
) {
    LmMessageRoute **slot = &runtime->route_head;

    while (*slot != 0) {
        LmMessageRoute *binding = *slot;

        if (binding->target != 0 && binding->target->pool == pool) {
            *slot = binding->next;
            free(binding->route);
            free(binding);
            runtime->route_count -= 1U;
        } else {
            slot = &binding->next;
        }
    }
}

int lm_message_thread_pool_delete(LmMessageThreadPool *pool) {
    LmMessageThread *thread;
    LmMessageThreadRuntime *runtime;
    int lifecycle_active = 0;
    int join_failed = 0;
    size_t index;

    if (pool == 0) {
        return 0;
    }
    if (!lm_native_message_thread_pool_lifecycle_allowed(pool)) {
        return 1;
    }
    runtime = pool->runtime;
    if (runtime == 0 || lm_mutex_lock(runtime->route_mutex) != 0) {
        return 1;
    }
    if (lm_mutex_lock(pool->mutex) != 0) {
        (void)lm_mutex_unlock(runtime->route_mutex);
        return 1;
    }
    if (pool->deleting) {
        (void)lm_mutex_unlock(pool->mutex);
        (void)lm_mutex_unlock(runtime->route_mutex);
        return 1;
    }
    lm_native_message_thread_pool_stop_locked(pool);
    thread = pool->member_head;
    while (thread != 0) {
        if (lm_native_message_thread_state_lock(thread) != 0) {
            abort();
        }
        if (thread->executing || thread->joining) {
            lifecycle_active = 1;
        }
        (void)lm_native_message_thread_state_unlock(thread);
        thread = thread->pool_next;
    }
    (void)lm_condition_broadcast(pool->work_ready);
    if (lifecycle_active) {
        (void)lm_mutex_unlock(pool->mutex);
        (void)lm_mutex_unlock(runtime->route_mutex);
        return 1;
    }
    pool->deleting = 1;
    lm_native_message_thread_remove_pool_routes_locked(runtime, pool);
    (void)lm_mutex_unlock(pool->mutex);
    (void)lm_mutex_unlock(runtime->route_mutex);

    if (pool->single_mode) {
        while (lm_native_message_thread_pool_pump_one(pool) > 0) {
        }
    } else {
        for (index = 0U;
             index < pool->started_worker_count;
             index += 1U) {
            if (pool->workers[index] != 0 &&
                !pool->workers[index]->joined &&
                lm_host_thread_join(pool->workers[index], 0) != 0) {
                join_failed = 1;
            }
        }
    }

    if (join_failed) {
        if (lm_mutex_lock(pool->mutex) == 0) {
            pool->deleting = 0;
            (void)lm_mutex_unlock(pool->mutex);
        }
        return 1;
    }

    for (index = 0U; index < pool->worker_count; index += 1U) {
        lm_host_thread_delete(pool->workers[index]);
    }
    free(pool->workers);
    pool->workers = 0;

    thread = pool->member_head;
    pool->member_head = 0;
    pool->member_tail = 0;
    pool->member_count = 0U;
    while (thread != 0) {
        LmMessageThread *next = thread->pool_next;

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

    lm_condition_delete(pool->work_ready);
    lm_mutex_delete(pool->mutex);
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
    return arena -> allocations == 0 && arena -> allocation_descriptors == 0 && arena -> lazy_edges == 0 && arena -> frozen == 0 && arena -> owner_thread == 0 && arena -> registry_previous == 0 && arena -> registry_next == 0 && arena -> runtime_owned_shell == 0;
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
    if (arena == 0 || owner_thread == 0 || lm_lmx_message_thread == 0 || owner_thread != lm_lmx_message_thread || lm_own_arena_shell_is_zero(lm_lmx_message_thread, arena) == 0) {
        return 1;
    }
    allocations = lm_own_new_zero(sizeof(allocations[0]));
    allocation_descriptors = lm_own_new_zero(sizeof(allocation_descriptors[0]));
    lazy_edges = lm_own_new_zero(sizeof(lazy_edges[0]));
    if (allocations == 0 || allocation_descriptors == 0 || lazy_edges == 0) {
        lm_own_delete(lazy_edges, 0);
        lm_own_delete(allocation_descriptors, 0);
        lm_own_delete(allocations, 0);
        return 1;
    }
    lm_own_ptr_stack_init(allocations, free);
    lm_own_ptr_stack_init(allocation_descriptors, lm_own_delete_plain);
    lm_own_ptr_stack_init(lazy_edges, lm_own_delete_plain);
    arena->allocations = allocations;
    arena->allocation_descriptors = allocation_descriptors;
    arena->lazy_edges = lazy_edges;
    arena->frozen = 0;
    arena->runtime_owned_shell = runtime_owned_shell != 0;
    if (lm_own_arena_register(lm_lmx_message_thread, owner_thread, arena) != 0) {
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
