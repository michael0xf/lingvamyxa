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

void lm_own_delete(void *object, LmOwnDestroyFields destroy_fields);
void lm_own_pointer_array_delete(void **items, size_t count, LmOwnDelete delete_item);
void lm_own_ptr_stack_init(LmOwnPtrStack *stack, LmOwnDelete delete_item);
void lm_own_ptr_stack_destroy(LmOwnPtrStack *stack);
int lm_own_ptr_stack_push(LmOwnPtrStack *stack, void *item);
void *lm_own_ptr_stack_pop(LmOwnPtrStack *stack);
void *lm_own_ptr_stack_at(const LmOwnPtrStack *stack, size_t index);
void *lm_own_ptr_stack_top(const LmOwnPtrStack *stack);
void lm_own_ptr_stack_truncate(LmOwnPtrStack *stack, size_t count);

#endif
