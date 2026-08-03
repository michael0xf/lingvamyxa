#include <stddef.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>

#ifndef LM_UNUSED
#define LM_UNUSED(value) ((void)(value))
#endif
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
typedef struct LmBuildOptions {
    int full_build;
    int next_build;
} LmBuildOptions;


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
    fputs("if (-not $p.WaitForExit(120000)) { $p.Kill(); exit 124 }\n", file);
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

static int lm_build_write_platform_tests_script(struct LmMessageThread *lm_lmx_message_thread, FILE *file, char *output_dir, char *parser_library, char *own_library) {
    (void)lm_lmx_message_thread;
    fputs("$ErrorActionPreference = 'Continue'\n", file);
    fputs("function Invoke-LmTestWithTimeout([string]$Path, [string]$Name) {\n", file);
    fputs("    $process = Start-Process -FilePath (Resolve-Path -LiteralPath $Path).Path -WorkingDirectory (Get-Location).Path -PassThru\n", file);
    fputs("    if (-not $process.WaitForExit(30000)) { $process.Kill(); throw ($Name + ' timed out') }\n", file);
    fputs("    if ($process.ExitCode -ne 0) { throw ($Name + ' failed with exit ' + $process.ExitCode) }\n", file);
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
    fputs("foreach ($testFile in Get-ChildItem -LiteralPath 'tests' -File | Where-Object { $_.Name -like 'trans_*' -and ($_.Extension -eq '.lm2' -or $_.Extension -eq '.lmx') } | Sort-Object Name) {\n", file);
    fputs("    if ($transSkip -contains $testFile.Name) { continue }\n", file);
    fputs("    $cPath = Join-Path 'build/obj/tests' ($testFile.BaseName + '.c')\n", file);
    fputs("    $exePath = Join-Path 'build/obj/tests' ($testFile.BaseName + '.exe')\n", file);
    fputs("    if ($testFile.Name -like 'trans_invalid_*') {\n", file);
    fputs("        $previousViewMode = $env:LM_TRANS_REGISTRY_VIEW\n", file);
    fputs("        try {\n", file);
    fputs("            foreach ($viewMode in @('legacy', '1', 'view')) {\n", file);
    fputs("                $env:LM_TRANS_REGISTRY_VIEW = $viewMode\n", file);
    fputs("                Remove-Item -LiteralPath $cPath -Force -ErrorAction SilentlyContinue\n", file);
    fputs("                & $trans $testFile.FullName $cPath *> $null\n", file);
    fputs("                $code = $LASTEXITCODE\n", file);
    fputs("                if ($code -ne 1) { throw ('negative trans test expected exit 1: ' + $testFile.Name + ' [' + $viewMode + '] got ' + $code) }\n", file);
    fputs("            }\n", file);
    fputs("        }\n", file);
    fputs("        finally {\n", file);
    fputs("            $env:LM_TRANS_REGISTRY_VIEW = $previousViewMode\n", file);
    fputs("            Remove-Item -LiteralPath $cPath -Force -ErrorAction SilentlyContinue\n", file);
    fputs("        }\n", file);
    fputs("        continue\n", file);
    fputs("    }\n", file);
    fputs("    if ($testFile.Name -eq 'trans_registry_view_parity.lmx') {\n", file);
    fputs("        $legacyPath = $cPath + '.legacy'\n", file);
    fputs("        $previousViewMode = $env:LM_TRANS_REGISTRY_VIEW\n", file);
    fputs("        $env:LM_TRANS_REGISTRY_VIEW = 'legacy'\n", file);
    fputs("        & $trans $testFile.FullName $legacyPath\n", file);
    fputs("        $legacyCode = $LASTEXITCODE\n", file);
    fputs("        $env:LM_TRANS_REGISTRY_VIEW = 'view'\n", file);
    fputs("        & $trans $testFile.FullName $cPath\n", file);
    fputs("        $viewCode = $LASTEXITCODE\n", file);
    fputs("        $env:LM_TRANS_REGISTRY_VIEW = $previousViewMode\n", file);
    fputs("        if ($legacyCode -ne 0) { throw 'legacy registry parity translation failed' }\n", file);
    fputs("        if ($viewCode -ne 0) { throw 'new registry view parity translation failed' }\n", file);
    fputs("        if ((Get-FileHash -Algorithm SHA256 $legacyPath).Hash -ne (Get-FileHash -Algorithm SHA256 $cPath).Hash) { throw 'legacy/new registry outputs differ' }\n", file);
    fputs("        Remove-Item -LiteralPath $legacyPath -Force\n", file);
    fputs("    }\n", file);
    fputs("    else {\n", file);
    fputs("        $previousViewMode = $env:LM_TRANS_REGISTRY_VIEW\n", file);
    fputs("        $env:LM_TRANS_REGISTRY_VIEW = 'view'\n", file);
    fputs("        & $trans $testFile.FullName $cPath\n", file);
    fputs("        $viewCode = $LASTEXITCODE\n", file);
    fputs("        $env:LM_TRANS_REGISTRY_VIEW = $previousViewMode\n", file);
    fputs("        if ($viewCode -ne 0) { throw ('trans smoke translation failed: ' + $testFile.Name) }\n", file);
    fputs("    }\n", file);
    fputs("    if ($transTranslationOnly -contains $testFile.Name) { continue }\n", file);
    fputs("    & $make 'link' '-std=c99' '-Wall' '-Wextra' '-Wpedantic' '-Ilm1' $cPath $parserLib $ownLib '-o' $exePath\n", file);
    fputs("    if ($LASTEXITCODE -ne 0) { throw ('trans smoke link failed: ' + $testFile.Name) }\n", file);
    fputs("    & (Resolve-Path -LiteralPath $exePath).Path\n", file);
    fputs("    if ($LASTEXITCODE -ne 0) { throw ('trans smoke run failed: ' + $testFile.Name) }\n", file);
    fputs("}\n", file);
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
    fputs("Write-Host 'lm0 staged tests passed'\n", file);
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

static int lm_build_write_platform_tests_script(struct LmMessageThread *lm_lmx_message_thread, FILE *file, char *output_dir, char *parser_library, char *own_library) {
    (void)lm_lmx_message_thread;
    fputs("set -eu\n", file);
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
    fputs("legacy_registry_fixture='build/obj/tests/parser_registry_source_tables.lm4'\n", file);
    fputs("cp tests/fixtures/parser_registry_source_tables.lm2 \"$legacy_registry_fixture\"\n", file);
    fputs("if LM_P0_REGISTRY=\"$legacy_registry_fixture\" \"$registry_selftest\" >/dev/null 2>&1; then legacy_code=0; else legacy_code=$?; fi\n", file);
    fputs("rm -f \"$legacy_registry_fixture\"\n", file);
    fputs("if [ \"$legacy_code\" -ne 1 ]; then echo \"legacy .lm4 parser registry rejection expected exit 1, got $legacy_code\" >&2; exit 1; fi\n", file);
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
    fputs("for src in tests/trans_*.lm2 tests/trans_*.lmx; do\n", file);
    fputs("    [ -e \"$src\" ] || continue\n", file);
    fputs("    name=${src##*/}\n", file);
    fputs("    base=${name%.*}\n", file);
    fputs("    c_path=\"build/obj/tests/$base.c\"\n", file);
    fputs("    exe_path=\"build/obj/tests/$base\"\n", file);
    fputs("    case \"$name\" in\n", file);
    fputs("        trans_invalid_*)\n", file);
    fputs("            for view_mode in legacy 1 view; do\n", file);
    fputs("                rm -f \"$c_path\"\n", file);
    fputs("                if LM_TRANS_REGISTRY_VIEW=\"$view_mode\" \"$trans\" \"$src\" \"$c_path\" >/dev/null 2>&1; then code=0; else code=$?; fi\n", file);
    fputs("                if [ \"$code\" -ne 1 ]; then echo \"negative trans test expected exit 1: $name [$view_mode] got $code\" >&2; exit 1; fi\n", file);
    fputs("            done\n", file);
    fputs("            rm -f \"$c_path\"\n", file);
    fputs("            continue\n", file);
    fputs("            ;;\n", file);
    fputs("    esac\n", file);
    fputs("    case \"$name\" in\n", file);
    fputs("        trans_registry_view_parity.lmx)\n", file);
    fputs("            legacy_path=\"$c_path.legacy\"\n", file);
    fputs("            LM_TRANS_REGISTRY_VIEW=legacy \"$trans\" \"$src\" \"$legacy_path\"\n", file);
    fputs("            LM_TRANS_REGISTRY_VIEW=view \"$trans\" \"$src\" \"$c_path\"\n", file);
    fputs("            cmp \"$legacy_path\" \"$c_path\"\n", file);
    fputs("            rm -f \"$legacy_path\"\n", file);
    fputs("            ;;\n", file);
    fputs("        *) LM_TRANS_REGISTRY_VIEW=view \"$trans\" \"$src\" \"$c_path\" ;;\n", file);
    fputs("    esac\n", file);
    fputs("    case \"$name\" in trans_l4_abi_receivers.lm2|trans_message_thread_pool_single.lm2|trans_message_thread_mailbox_single.lm2) continue ;; esac\n", file);
    fputs("    \"$make_tool\" link -std=c99 -Wall -Wextra -Wpedantic -Ilm1 \"$c_path\" \"$parserLib\" \"$ownLib\" -o \"$exe_path\"\n", file);
    fputs("    \"$exe_path\"\n", file);
    fputs("done\n", file);
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
    fputs("echo 'lm0 staged tests passed'\n", file);
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




static inline int lm_message_thread_diagnostic_status(const LmMessageThreadExecutionContext *context) {
    if (context == 0 || context->diagnostic_code == 0) {
        return 1;
    }
    return context->diagnostic_code;
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

static void lm_build_print_usage(struct LmMessageThread *lm_lmx_message_thread) {
    (void)lm_lmx_message_thread;
    printf("usage: buildCore.lm0 [--build] [--full] [--next]\n");
    printf("  --build refresh the L0 bootstrap tools\n");
    printf("  --full  refresh L0 tools, then build the bundled third_party profile\n");
    printf("  --next  verify the staged L0 bootstrap tools without installing them\n");
}

static int lm_build_parse_options(struct LmMessageThread *lm_lmx_message_thread, int argc, char **argv, LmBuildOptions *options) {
    (void)lm_lmx_message_thread;
    int index;
    options->full_build = 0;
    options->next_build = 0;
    index = 1;
    while (index < argc) {
        if (strcmp(argv[index], "--build") == 0) {
        }
        if (strcmp(argv[index], "--full") == 0) {
            options->full_build = 1;
        }
        if (strcmp(argv[index], "--next") == 0) {
            options->next_build = 1;
        }
        if (strcmp(argv[index], "--help") == 0 || strcmp(argv[index], "-h") == 0) {
            lm_build_print_usage(lm_lmx_message_thread);
            return 2;
        }
        if (strcmp(argv[index], "--build") != 0 && strcmp(argv[index], "--full") != 0 && strcmp(argv[index], "--next") != 0 && strcmp(argv[index], "--help") != 0 && strcmp(argv[index], "-h") != 0) {
            fprintf(stderr, "buildCore.lm0: unknown option: %s\n", argv[index]);
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
    if (lm_build_trans(lm_lmx_message_thread, trans_tool, "lm2/trans.lm2", "lm1/build/trans.lm1.c") != 0) {
        return 1;
    }
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
    snprintf(command, sizeof(command), "-std=c99 -Wall -Wextra -Wpedantic \"lm1/build/finalize.lm1.c\" \"%s\" -o \"%s/finalize.lm0%s\"", own_library, output_dir, lm_build_exe_suffix(lm_lmx_message_thread));
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
    snprintf(command, sizeof(command), "-std=c99 -Wall -Wextra -Wpedantic -I\"lm1\" \"lm1/build/buildCore.lm1.c\" \"%s\" -o \"%s/buildCore.lm0%s\"", own_library, output_dir, lm_build_exe_suffix(lm_lmx_message_thread));
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
    LmMessageThreadExecutionContext lm_message_thread_main_context = {0};
    int lm_message_thread_exit_status;
    struct LmMessageThreadRuntime *lm_lmx_application_runtime = 0;
    int lm_lmx_application_root_attached = 0;
    int lm_lmx_thread_startup_failed = 0;
    int lm_lmx_thread_cleanup_failed = 0;
    int lm_lmx_application_controller_failure = 0;
    int lm_lmx_application_exit_requested = 0;
    int lm_lmx_application_exit_ready = 0;
    int lm_lmx_application_exit_status = 0;
    int lm_lmx_application_exit_snapshot_requested = 0;
    int lm_lmx_application_exit_snapshot_ready = 0;
    int lm_lmx_application_exit_snapshot_status = 0;
    lm_lmx_message_thread = lm_message_thread_new();
    if (lm_lmx_message_thread == 0) {
        return 1;
    }
    (void)lm_message_thread_set_execution_context(lm_lmx_message_thread, &lm_message_thread_main_context);
    lm_lmx_application_runtime = lm_message_thread_runtime_new();
    if (lm_lmx_application_runtime == 0) lm_lmx_thread_startup_failed = 1;
#ifdef LM_REST_LMX_INSTALL_DEFAULT_CLIENT
    if (!lm_lmx_thread_startup_failed && lm_rest_lmx_http_client_install_default(lm_lmx_application_runtime) != 0) {
        (void)lm_message_thread_runtime_delete(lm_lmx_application_runtime);
        lm_lmx_application_runtime = 0;
        lm_message_thread_delete(lm_lmx_message_thread);
        return 1;
    }
#endif
    if (!lm_lmx_thread_startup_failed && lm_message_thread_runtime_attach_root(lm_lmx_application_runtime, lm_lmx_message_thread) != 0) lm_lmx_thread_startup_failed = 1; else if (!lm_lmx_thread_startup_failed) lm_lmx_application_root_attached = 1;
    if (lm_lmx_thread_startup_failed) lm_message_thread_request_failure(lm_lmx_message_thread, 1);
    while (lm_message_thread_begin_turn(lm_lmx_message_thread)) {
        if (lm_lmx_application_controller_failure) {
            lm_message_thread_request_failure(lm_lmx_message_thread, 1);
            goto lm_message_thread_turn_end;
        }
        if (lm_lmx_application_exit_ready) {
            lm_message_thread_request_stop(lm_lmx_message_thread, lm_lmx_application_exit_status);
            goto lm_message_thread_turn_end;
        }
        lm_message_thread_main_context.diagnostic_code = 0;
        if (setjmp(lm_message_thread_main_context.diagnostic_root) == 0) {
            char *trusted_make;
            char trusted_make_buffer[128];
            char built_trans_buffer[128];
            LmBuildOptions * options;
            int parse_status;
            int result;
            options = lm_build_options_new(lm_lmx_message_thread);
            if (options == 0) {
                {
                    int lm_return_0 = 1;
                    lm_message_thread_request_stop(lm_lmx_message_thread, lm_return_0);
                    goto lm_message_thread_turn_end;
                }
            }
            parse_status = lm_build_parse_options(lm_lmx_message_thread, argc, argv, options);
            if (parse_status == 2) {
                lm_build_options_delete(lm_lmx_message_thread, options);
                {
                    int lm_return_1 = 0;
                    lm_message_thread_request_stop(lm_lmx_message_thread, lm_return_1);
                    goto lm_message_thread_turn_end;
                }
            }
            if (parse_status != 0) {
                lm_build_options_delete(lm_lmx_message_thread, options);
                {
                    int lm_return_2 = 1;
                    lm_message_thread_request_stop(lm_lmx_message_thread, lm_return_2);
                    goto lm_message_thread_turn_end;
                }
            }
            if (lm_build_enter_project_root(lm_lmx_message_thread, argv[0]) != 0) {
                lm_build_options_delete(lm_lmx_message_thread, options);
                {
                    int lm_return_3 = 1;
                    lm_message_thread_request_stop(lm_lmx_message_thread, lm_return_3);
                    goto lm_message_thread_turn_end;
                }
            }
            if (lm_build_thread_provider(lm_lmx_message_thread) == 0) {
                lm_build_options_delete(lm_lmx_message_thread, options);
                {
                    int lm_return_4 = 1;
                    lm_message_thread_request_stop(lm_lmx_message_thread, lm_return_4);
                    goto lm_message_thread_turn_end;
                }
            }
            snprintf(trusted_make_buffer, sizeof(trusted_make_buffer), "build%slm0%smake.lm0%s", lm_build_path_sep(lm_lmx_message_thread), lm_build_path_sep(lm_lmx_message_thread), lm_build_exe_suffix(lm_lmx_message_thread));
            snprintf(built_trans_buffer, sizeof(built_trans_buffer), "build%slm0%strans.lm0%s", lm_build_path_sep(lm_lmx_message_thread), lm_build_path_sep(lm_lmx_message_thread), lm_build_exe_suffix(lm_lmx_message_thread));
            if (options -> next_build) {
                snprintf(trusted_make_buffer, sizeof(trusted_make_buffer), "build%slm0%snext%smake.lm0%s", lm_build_path_sep(lm_lmx_message_thread), lm_build_path_sep(lm_lmx_message_thread), lm_build_path_sep(lm_lmx_message_thread), lm_build_exe_suffix(lm_lmx_message_thread));
                snprintf(built_trans_buffer, sizeof(built_trans_buffer), "build%slm0%snext%strans.lm0%s", lm_build_path_sep(lm_lmx_message_thread), lm_build_path_sep(lm_lmx_message_thread), lm_build_path_sep(lm_lmx_message_thread), lm_build_exe_suffix(lm_lmx_message_thread));
            }
            trusted_make = lm_build_env_or_default(lm_lmx_message_thread, "LM_MAKE", trusted_make_buffer);
            result = lm_build_run_bootstrap(lm_lmx_message_thread, options, trusted_make, built_trans_buffer);
            lm_build_options_delete(lm_lmx_message_thread, options);
            {
                int lm_return_5 = result;
                lm_message_thread_request_stop(lm_lmx_message_thread, lm_return_5);
                goto lm_message_thread_turn_end;
            }
        } else {
            lm_message_thread_request_failure(lm_lmx_message_thread, lm_message_thread_diagnostic_status(&lm_message_thread_main_context));
        }
    lm_message_thread_turn_end:
        (void)lm_message_thread_end_turn(lm_lmx_message_thread);
        lm_lmx_application_exit_snapshot_requested = 0;
        lm_lmx_application_exit_snapshot_ready = 0;
        lm_lmx_application_exit_snapshot_status = 0;
        if (lm_lmx_application_runtime != 0) {
            if (lm_message_thread_runtime_exit_state(lm_lmx_application_runtime, &lm_lmx_application_exit_snapshot_requested, &lm_lmx_application_exit_snapshot_ready, &lm_lmx_application_exit_snapshot_status) != 0) {
                lm_lmx_thread_cleanup_failed = 1;
                lm_lmx_application_controller_failure = 1;
            } else {
                lm_lmx_application_exit_requested = lm_lmx_application_exit_snapshot_requested;
                lm_lmx_application_exit_ready = lm_lmx_application_exit_snapshot_ready;
                lm_lmx_application_exit_status = lm_lmx_application_exit_snapshot_status;
            }
        }
    }
    lm_lmx_application_exit_snapshot_requested = 0;
    lm_lmx_application_exit_snapshot_ready = 0;
    lm_lmx_application_exit_snapshot_status = 0;
    if (lm_lmx_application_runtime != 0) {
        if (lm_message_thread_runtime_exit_state(lm_lmx_application_runtime, &lm_lmx_application_exit_snapshot_requested, &lm_lmx_application_exit_snapshot_ready, &lm_lmx_application_exit_snapshot_status) != 0) {
            lm_lmx_thread_cleanup_failed = 1;
            lm_lmx_application_controller_failure = 1;
        } else {
            lm_lmx_application_exit_requested = lm_lmx_application_exit_snapshot_requested;
            lm_lmx_application_exit_ready = lm_lmx_application_exit_snapshot_ready;
            lm_lmx_application_exit_status = lm_lmx_application_exit_snapshot_status;
        }
    }
    lm_message_thread_exit_status = lm_message_thread_status(lm_lmx_message_thread);
    if (lm_lmx_application_runtime != 0 && lm_lmx_application_root_attached) {
        if (lm_message_thread_runtime_detach_root(lm_lmx_application_runtime, lm_lmx_message_thread) != 0) lm_lmx_thread_cleanup_failed = 1; else lm_lmx_application_root_attached = 0;
    }
    if (lm_lmx_application_runtime != 0 && !lm_lmx_application_root_attached) {
        if (lm_message_thread_runtime_delete(lm_lmx_application_runtime) != 0) lm_lmx_thread_cleanup_failed = 1; else lm_lmx_application_runtime = 0;
    }
    if (lm_message_thread_exit_status == 0 && lm_lmx_application_exit_requested && lm_lmx_application_exit_status != 0) lm_message_thread_exit_status = lm_lmx_application_exit_status;
    if (lm_message_thread_exit_status == 0 && lm_lmx_thread_cleanup_failed) lm_message_thread_exit_status = 1;
    if (!lm_lmx_application_root_attached) lm_message_thread_delete(lm_lmx_message_thread);
    return lm_message_thread_exit_status;
}
