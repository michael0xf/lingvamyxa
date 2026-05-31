#ifndef LM_OWN_LM1_H
#define LM_OWN_LM1_H

#include <stddef.h>

typedef void (*LmOwnDestroyFields)(void *object);
typedef void (*LmOwnDelete)(void *object);

typedef struct LmOwnPtrStack {
    void **items;
    size_t count;
    size_t capacity;
    LmOwnDelete delete_item;
} LmOwnPtrStack;

typedef struct LmOwnValueStack {
    void *items;
    size_t count;
    size_t capacity;
    size_t item_size;
} LmOwnValueStack;

typedef int LmOwnEdgeKind;

#define LM_OWN_EDGE_BORROWED 1
#define LM_OWN_EDGE_OWNED 2
#define LM_OWN_EDGE_LAZY_OWNED 3
#define LM_OWN_EDGE_EXTERNAL 4

typedef struct LmOwnArena {
    LmOwnPtrStack allocations;
    int frozen;
} LmOwnArena;

void *lm_own_new_zero(size_t size);
void lm_own_delete(void *object, LmOwnDestroyFields destroy_fields);
void lm_own_pointer_array_delete(void **items, size_t count, LmOwnDelete delete_item);
void lm_own_ptr_stack_init(LmOwnPtrStack *stack, LmOwnDelete delete_item);
void lm_own_ptr_stack_destroy(LmOwnPtrStack *stack);
int lm_own_ptr_stack_push(LmOwnPtrStack *stack, void *item);
void *lm_own_ptr_stack_pop(LmOwnPtrStack *stack);
void *lm_own_ptr_stack_at(const LmOwnPtrStack *stack, size_t index);
void *lm_own_ptr_stack_top(const LmOwnPtrStack *stack);
void lm_own_ptr_stack_truncate(LmOwnPtrStack *stack, size_t count);
void lm_own_value_stack_init(LmOwnValueStack *stack, size_t item_size);
void lm_own_value_stack_destroy(LmOwnValueStack *stack);
int lm_own_value_stack_push(LmOwnValueStack *stack, const void *item);
int lm_own_value_stack_resize_zero(LmOwnValueStack *stack, size_t count);
int lm_own_value_stack_pop(LmOwnValueStack *stack, void *out_item);
void *lm_own_value_stack_at(const LmOwnValueStack *stack, size_t index);
void *lm_own_value_stack_top(const LmOwnValueStack *stack);
void lm_own_value_stack_truncate(LmOwnValueStack *stack, size_t count);
void lm_own_arena_init(LmOwnArena *arena);
void lm_own_arena_destroy(LmOwnArena *arena);
void *lm_own_arena_new_zero(LmOwnArena *arena, size_t size);
char *lm_own_arena_copy_bytes(LmOwnArena *arena, const char *source, size_t length);
int lm_own_arena_absorb(LmOwnArena *target, LmOwnArena *source);
void lm_own_arena_freeze(LmOwnArena *arena);
int lm_own_arena_is_frozen(const LmOwnArena *arena);
int lm_own_tree_cut(LmOwnArena *arena);
int lm_own_tree_cut_promote_lazy_edges(LmOwnArena *arena);

#endif
