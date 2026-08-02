#include <stddef.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>

struct LmOwnArena;
struct LmMessageThread;
struct LmMessageThread *lm_message_thread_new(void);
void lm_message_thread_delete(struct LmMessageThread *thread);
struct LmOwnArena *lm_message_thread_owner(struct LmMessageThread *thread);
void *lm_message_thread_execution_context(struct LmMessageThread *thread);
void *lm_message_thread_set_execution_context(struct LmMessageThread *thread, void *context);
int lm_message_thread_begin_turn(struct LmMessageThread *thread);
int lm_message_thread_end_turn(struct LmMessageThread *thread);
void lm_message_thread_request_stop(struct LmMessageThread *thread, int status);
void lm_message_thread_request_failure(struct LmMessageThread *thread, int status);
int lm_message_thread_status(const struct LmMessageThread *thread);
int lm_message_thread_is_running(const struct LmMessageThread *thread);
size_t lm_message_thread_turn_count(const struct LmMessageThread *thread);
size_t lm_message_thread_collection_count(const struct LmMessageThread *thread);
void lm_own_arena_freeze(struct LmOwnArena *arena);
void *lm_own_arena_new_zero(struct LmOwnArena *arena, size_t size);
void *lm_own_arena_array_new_zero(struct LmOwnArena *arena, size_t element_size, size_t count, size_t rank, size_t level);
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
    LmMessageThreadExecutionContext context = {0};
    int status;
    if (entry == 0) {
        return 1;
    }
    thread = lm_message_thread_new();
    if (thread == 0) {
        return 1;
    }
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
    lm_message_thread_delete(thread);
    return status;
}
#undef LM_LMX_UNUSED_ENTRY_HELPER
#include <stddef.h>




typedef struct LmOwnPtrStack LmOwnPtrStack;
typedef struct LmOwnValueStack LmOwnValueStack;
typedef struct LmOwnAllocationDescriptor LmOwnAllocationDescriptor;
typedef struct LmOwnLazyEdge LmOwnLazyEdge;
typedef struct LmOwnArena LmOwnArena;
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
};
struct LmMessageThread {
    LmOwnArena * root_owner;
    LmMessageThreadState state;
    int stop_status;
    size_t turn_count;
    size_t collection_count;
    int collector_failed;
    void *execution_context;
};


#ifndef LM_LMX_TYPEDEF_DEFINED_LmOwnDestroyFields
#define LM_LMX_TYPEDEF_DEFINED_LmOwnDestroyFields 1
typedef void (*LmOwnDestroyFields)(void *object);
#endif
#ifndef LM_LMX_TYPEDEF_DEFINED_LmOwnDelete
#define LM_LMX_TYPEDEF_DEFINED_LmOwnDelete 1
typedef void (*LmOwnDelete)(void *object);
#endif


void * (lm_own_new_zero)(size_t size);
void * (lm_own_resize)(void *object, size_t size);
char * (lm_own_copy_bytes)(const char *source, size_t length);
void * (lm_own_array_new_zero)(size_t element_size, size_t count, size_t rank, size_t level);
const LmOwnAllocationDescriptor * (lm_own_allocation_descriptor)(const void *address);
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
LmOwnArena * (lm_own_arena_new)(void);
void (lm_own_arena_delete)(LmOwnArena *arena);
int (lm_own_arena_init)(LmOwnArena *arena);
void (lm_own_arena_destroy)(LmOwnArena *arena);
void * (lm_own_arena_new_zero)(LmOwnArena *arena, size_t size);
void * (lm_own_arena_array_new_zero)(LmOwnArena *arena, size_t element_size, size_t count, size_t rank, size_t level);
const LmOwnAllocationDescriptor * (lm_own_arena_allocation_descriptor)(const LmOwnArena *arena, const void *address);
char * (lm_own_arena_copy_bytes)(LmOwnArena *arena, const char *source, size_t length);
int (lm_own_arena_add_lazy_edge)(LmOwnArena *target, LmOwnArena *source, const void *source_ptr, size_t size, const void **patch_slot);
int (lm_own_arena_promote_lazy_edges)(LmOwnArena *arena);
int (lm_own_arena_absorb)(LmOwnArena *target, LmOwnArena *source);
void (lm_own_arena_freeze)(LmOwnArena *arena);
int (lm_own_arena_is_frozen)(const LmOwnArena *arena);
int (lm_own_tree_cut)(LmOwnArena *arena);
int (lm_own_tree_cut_promote_lazy_edges)(LmOwnArena *arena);
int (lm_message_thread_init)(LmMessageThread *thread);
void (lm_message_thread_destroy)(LmMessageThread *thread);
LmMessageThread * (lm_message_thread_new)(void);
void (lm_message_thread_delete)(LmMessageThread *thread);
int (lm_message_thread_begin_turn)(LmMessageThread *thread);
int (lm_message_thread_end_turn)(LmMessageThread *thread);
int (lm_message_thread_collect)(LmMessageThread *thread);
void (lm_message_thread_request_stop)(LmMessageThread *thread, int status);
void (lm_message_thread_request_failure)(LmMessageThread *thread, int status);
int (lm_message_thread_is_running)(const LmMessageThread *thread);
int (lm_message_thread_status)(const LmMessageThread *thread);
LmOwnArena * (lm_message_thread_owner)(LmMessageThread *thread);
void * (lm_message_thread_execution_context)(LmMessageThread *thread);
void * (lm_message_thread_set_execution_context)(LmMessageThread *thread, void *context);
size_t (lm_message_thread_turn_count)(const LmMessageThread *thread);
size_t (lm_message_thread_collection_count)(const LmMessageThread *thread);




























































































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
int lm_own_arena_init(LmOwnArena *arena);
void lm_own_arena_destroy(LmOwnArena *arena);
LmOwnArena * lm_own_arena_new(void);
void lm_own_arena_delete(LmOwnArena *arena);
void lm_own_arena_freeze(LmOwnArena *arena);
int lm_own_arena_is_frozen(const LmOwnArena *arena);
int lm_own_tree_cut(LmOwnArena *arena);
int lm_own_tree_cut_promote_lazy_edges(LmOwnArena *arena);
int lm_own_value_stack_push(LmOwnValueStack *stack, const void *item);
int lm_own_value_stack_resize_zero(LmOwnValueStack *stack, size_t count);
int lm_own_value_stack_pop(LmOwnValueStack *stack, void *out_item);
void * lm_own_value_stack_at(const LmOwnValueStack *stack, size_t index);
void * lm_own_value_stack_top(const LmOwnValueStack *stack);
static int lm_own_size_multiply(struct LmMessageThread *lm_lmx_message_thread, size_t left, size_t right, size_t *out);
static int lm_own_global_allocation_descriptors_init(struct LmMessageThread *lm_lmx_message_thread);
static const LmOwnAllocationDescriptor * lm_own_allocation_descriptor_find(struct LmMessageThread *lm_lmx_message_thread, const LmOwnPtrStack *descriptors, const void *address);
static int lm_own_allocation_descriptor_push(struct LmMessageThread *lm_lmx_message_thread, LmOwnPtrStack *descriptors, void *address, LmOwnArena *owner, size_t bytes, size_t element_size, size_t count, size_t rank, size_t level);
void * lm_own_array_new_zero(size_t element_size, size_t count, size_t rank, size_t level);
const LmOwnAllocationDescriptor * lm_own_allocation_descriptor(const void *address);
void * lm_own_arena_new_zero(LmOwnArena *arena, size_t size);
void * lm_own_arena_array_new_zero(LmOwnArena *arena, size_t element_size, size_t count, size_t rank, size_t level);
const LmOwnAllocationDescriptor * lm_own_arena_allocation_descriptor(const LmOwnArena *arena, const void *address);
char * lm_own_arena_copy_bytes(LmOwnArena *arena, const char *source, size_t length);
int lm_own_arena_add_lazy_edge(LmOwnArena *target, LmOwnArena *source, const void *source_ptr, size_t size, const void **patch_slot);
int lm_own_arena_promote_lazy_edges(LmOwnArena *arena);
int lm_own_arena_absorb(LmOwnArena *target, LmOwnArena *source);
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
void lm_message_thread_request_stop(LmMessageThread *thread, int status);
void lm_message_thread_request_failure(LmMessageThread *thread, int status);
int lm_message_thread_collect(LmMessageThread *thread);
int lm_message_thread_begin_turn(LmMessageThread *thread);
int lm_message_thread_end_turn(LmMessageThread *thread);

static LmOwnPtrStack * lm_own_global_allocation_descriptors;

static int lm_own_global_allocation_descriptors_ready;

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

int lm_own_arena_init(LmOwnArena *arena) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    if (arena == 0) {
        return 1;
    }
    arena->allocations = 0;
    arena->allocation_descriptors = 0;
    arena->lazy_edges = 0;
    arena->frozen = 1;
    arena->allocations = lm_own_new_zero(sizeof(LmOwnPtrStack));
    arena->allocation_descriptors = lm_own_new_zero(sizeof(LmOwnPtrStack));
    arena->lazy_edges = lm_own_new_zero(sizeof(LmOwnPtrStack));
    if (arena -> allocations == 0 || arena -> allocation_descriptors == 0 || arena -> lazy_edges == 0) {
        lm_own_delete(arena -> lazy_edges, 0);
        lm_own_delete(arena -> allocation_descriptors, 0);
        lm_own_delete(arena -> allocations, 0);
        arena->allocations = 0;
        arena->allocation_descriptors = 0;
        arena->lazy_edges = 0;
        return 1;
    }
    lm_own_ptr_stack_init(arena -> allocations, free);
    lm_own_ptr_stack_init(arena -> allocation_descriptors, lm_own_delete_plain);
    lm_own_ptr_stack_init(arena -> lazy_edges, lm_own_delete_plain);
    arena->frozen = 0;
    return 0;
}

void lm_own_arena_destroy(LmOwnArena *arena) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    if (arena != 0) {
        lm_own_ptr_stack_destroy(arena -> lazy_edges);
        lm_own_delete(arena -> lazy_edges, 0);
        lm_own_ptr_stack_destroy(arena -> allocation_descriptors);
        lm_own_delete(arena -> allocation_descriptors, 0);
        lm_own_ptr_stack_destroy(arena -> allocations);
        lm_own_delete(arena -> allocations, 0);
        arena->lazy_edges = 0;
        arena->allocation_descriptors = 0;
        arena->allocations = 0;
        arena->frozen = 0;
    }
}

LmOwnArena * lm_own_arena_new(void) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    LmOwnArena * arena;
    arena = lm_own_new_zero(sizeof(arena[0]));
    if (arena == 0) {
        return 0;
    }
    if (lm_own_arena_init(arena) != 0) {
        lm_own_delete(arena, 0);
        return 0;
    }
    return arena;
}

void lm_own_arena_delete(LmOwnArena *arena) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    if (arena != 0) {
        lm_own_arena_destroy(arena);
        lm_own_delete(arena, 0);
    }
}

void lm_own_arena_freeze(LmOwnArena *arena) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    if (arena != 0) {
        arena->frozen = 1;
    }
}

int lm_own_arena_is_frozen(const LmOwnArena *arena) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    return arena != 0 && arena -> frozen;
}

int lm_own_tree_cut(LmOwnArena *arena) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    return lm_own_tree_cut_promote_lazy_edges(arena);
}

int lm_own_tree_cut_promote_lazy_edges(LmOwnArena *arena) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    return lm_own_arena_promote_lazy_edges(arena);
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

static int lm_own_global_allocation_descriptors_init(struct LmMessageThread *lm_lmx_message_thread) {
    (void)lm_lmx_message_thread;
    if (lm_own_global_allocation_descriptors_ready == 0) {
        lm_own_global_allocation_descriptors = lm_own_new_zero(sizeof(LmOwnPtrStack));
        if (lm_own_global_allocation_descriptors == 0) {
            return 1;
        }
        lm_own_ptr_stack_init(lm_own_global_allocation_descriptors, lm_own_delete_plain);
        lm_own_global_allocation_descriptors_ready = 1;
    }
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

void * lm_own_array_new_zero(size_t element_size, size_t count, size_t rank, size_t level) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    void *object;
    size_t bytes;
    if (lm_own_size_multiply(lm_lmx_message_thread, element_size, count, &bytes) != 0) {
        return 0;
    }
    if (bytes == 0U) {
        return 0;
    }
    if (lm_own_global_allocation_descriptors_init(lm_lmx_message_thread) != 0) {
        return 0;
    }
    object = calloc(1U, bytes);
    if (object == 0) {
        return 0;
    }
    if (lm_own_allocation_descriptor_push(lm_lmx_message_thread, lm_own_global_allocation_descriptors, object, 0, bytes, element_size, count, rank, level) != 0) {
        free(object);
        return 0;
    }
    return object;
}

const LmOwnAllocationDescriptor * lm_own_allocation_descriptor(const void *address) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    if (lm_own_global_allocation_descriptors_ready == 0) {
        return 0;
    }
    return lm_own_allocation_descriptor_find(lm_lmx_message_thread, lm_own_global_allocation_descriptors, address);
}

void * lm_own_arena_new_zero(LmOwnArena *arena, size_t size) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    void *object;
    if (arena == 0 || arena -> frozen || arena -> allocations == 0 || arena -> allocation_descriptors == 0) {
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

void * lm_own_arena_array_new_zero(LmOwnArena *arena, size_t element_size, size_t count, size_t rank, size_t level) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    void *object;
    size_t bytes;
    if (arena == 0 || arena -> frozen || arena -> allocations == 0 || arena -> allocation_descriptors == 0) {
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

const LmOwnAllocationDescriptor * lm_own_arena_allocation_descriptor(const LmOwnArena *arena, const void *address) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    if (arena == 0 || arena -> allocation_descriptors == 0) {
        return 0;
    }
    return lm_own_allocation_descriptor_find(lm_lmx_message_thread, arena -> allocation_descriptors, address);
}

char * lm_own_arena_copy_bytes(LmOwnArena *arena, const char *source, size_t length) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    char *copy;
    copy = lm_own_arena_new_zero(arena, length + 1U);
    if (copy == 0) {
        return 0;
    }
    if (source != 0 && length > 0U) {
        memcpy(copy, source, length);
    }
    copy[length] = '\0';
    return copy;
}

int lm_own_arena_add_lazy_edge(LmOwnArena *target, LmOwnArena *source, const void *source_ptr, size_t size, const void **patch_slot) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    LmOwnLazyEdge * edge;
    int status;
    if (target == 0 || source == 0 || patch_slot == 0 || target -> frozen || target -> lazy_edges == 0) {
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

int lm_own_arena_promote_lazy_edges(LmOwnArena *arena) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    LmOwnLazyEdge * edge;
    void *copy;
    size_t i;
    if (arena == 0 || arena -> frozen || arena -> lazy_edges == 0) {
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
        copy = lm_own_arena_new_zero(arena, edge -> size + 1U);
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

int lm_own_arena_absorb(LmOwnArena *target, LmOwnArena *source) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
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
    if (target == 0 || source == 0 || target -> frozen) {
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
    if (thread == 0) {
        return 1;
    }
    memset(thread, 0, sizeof(thread[0]));
    thread->root_owner = lm_own_arena_new();
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
    if (thread != 0) {
        lm_own_arena_delete(thread -> root_owner);
        thread->root_owner = 0;
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
        lm_message_thread_destroy(thread);
        lm_own_delete(thread, 0);
    }
}

int lm_message_thread_is_running(const LmMessageThread *thread) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    return thread != 0 && thread -> state == LM_MESSAGE_THREAD_RUNNING;
}

int lm_message_thread_status(const LmMessageThread *thread) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    if (thread == 0) {
        return 1;
    }
    return thread -> stop_status;
}

LmOwnArena * lm_message_thread_owner(LmMessageThread *thread) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    if (thread == 0) {
        return 0;
    }
    return thread -> root_owner;
}

void * lm_message_thread_execution_context(LmMessageThread *thread) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    if (thread == 0) {
        return 0;
    }
    return thread -> execution_context;
}

void * lm_message_thread_set_execution_context(LmMessageThread *thread, void *context) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    void *previous;
    if (thread == 0) {
        return 0;
    }
    previous = thread -> execution_context;
    thread->execution_context = context;
    return previous;
}

size_t lm_message_thread_turn_count(const LmMessageThread *thread) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    if (thread == 0) {
        return 0U;
    }
    return thread -> turn_count;
}

size_t lm_message_thread_collection_count(const LmMessageThread *thread) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    if (thread == 0) {
        return 0U;
    }
    return thread -> collection_count;
}

void lm_message_thread_request_stop(LmMessageThread *thread, int status) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    if (thread != 0 && thread -> state == LM_MESSAGE_THREAD_RUNNING) {
        thread->stop_status = status;
        thread->state = LM_MESSAGE_THREAD_STOPPING;
    }
}

void lm_message_thread_request_failure(LmMessageThread *thread, int status) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    if (status == 0) {
        status = 1;
    }
    if (thread != 0 && (thread -> state == LM_MESSAGE_THREAD_RUNNING || thread -> state == LM_MESSAGE_THREAD_STOPPING)) {
        thread->stop_status = status;
        thread->state = LM_MESSAGE_THREAD_STOPPING;
    }
}

int lm_message_thread_collect(LmMessageThread *thread) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    if (thread == 0) {
        return 1;
    }
    thread->collection_count = thread -> collection_count + 1U;
    if (thread -> root_owner == 0) {
        thread->collector_failed = 1;
        lm_message_thread_request_failure(thread, 1);
        return 1;
    }
    if (lm_own_tree_cut(thread -> root_owner) != 0) {
        thread->collector_failed = 1;
        lm_message_thread_request_failure(thread, 1);
        return 1;
    }
    return 0;
}

int lm_message_thread_begin_turn(LmMessageThread *thread) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    if (lm_message_thread_is_running(thread) == 0) {
        return 0;
    }
    thread->turn_count = thread -> turn_count + 1U;
    return 1;
}

int lm_message_thread_end_turn(LmMessageThread *thread) {
    struct LmMessageThread *lm_lmx_message_thread = 0;
    (void)lm_lmx_message_thread;
    int status;
    if (thread == 0) {
        return 1;
    }
    status = lm_message_thread_collect(thread);
    if (thread -> state == LM_MESSAGE_THREAD_STOPPING) {
        thread->state = LM_MESSAGE_THREAD_STOPPED;
    }
    return status;
}
