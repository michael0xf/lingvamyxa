#include "own.lm1.h"

#include <stdlib.h>

void lm_own_delete(void *object, LmOwnDestroyFields destroy_fields) {
    if (object != NULL) {
        if (destroy_fields != NULL) {
            destroy_fields(object);
        }
        free(object);
    }
}

void lm_own_pointer_array_delete(void **items, size_t count, LmOwnDelete delete_item) {
    size_t index;

    if (items != NULL) {
        if (delete_item != NULL) {
            for (index = 0U; index < count; ++index) {
                if (items[index] != NULL) {
                    delete_item(items[index]);
                    items[index] = NULL;
                }
            }
        }
        free(items);
    }
}

void lm_own_ptr_stack_init(LmOwnPtrStack *stack, LmOwnDelete delete_item) {
    if (stack != NULL) {
        stack->items = NULL;
        stack->count = 0U;
        stack->capacity = 0U;
        stack->delete_item = delete_item;
    }
}

void lm_own_ptr_stack_destroy(LmOwnPtrStack *stack) {
    if (stack != NULL) {
        lm_own_pointer_array_delete(stack->items, stack->count, stack->delete_item);
        stack->items = NULL;
        stack->count = 0U;
        stack->capacity = 0U;
        stack->delete_item = NULL;
    }
}

int lm_own_ptr_stack_push(LmOwnPtrStack *stack, void *item) {
    void **items;
    size_t capacity;

    if (stack == NULL) {
        return 1;
    }

    if (stack->count == stack->capacity) {
        capacity = stack->capacity == 0U ? 8U : stack->capacity * 2U;
        items = (void **)realloc(stack->items, capacity * sizeof(*items));
        if (items == NULL) {
            return 1;
        }
        stack->items = items;
        stack->capacity = capacity;
    }

    stack->items[stack->count] = item;
    ++stack->count;
    return 0;
}

void *lm_own_ptr_stack_pop(LmOwnPtrStack *stack) {
    void *item;

    if (stack == NULL || stack->count == 0U) {
        return NULL;
    }

    --stack->count;
    item = stack->items[stack->count];
    stack->items[stack->count] = NULL;
    return item;
}

void *lm_own_ptr_stack_at(const LmOwnPtrStack *stack, size_t index) {
    if (stack == NULL || index >= stack->count) {
        return NULL;
    }

    return stack->items[index];
}

void *lm_own_ptr_stack_top(const LmOwnPtrStack *stack) {
    if (stack == NULL || stack->count == 0U) {
        return NULL;
    }

    return stack->items[stack->count - 1U];
}

void lm_own_ptr_stack_truncate(LmOwnPtrStack *stack, size_t count) {
    void *item;

    if (stack == NULL) {
        return;
    }

    while (stack->count > count) {
        item = lm_own_ptr_stack_pop(stack);
        if (item != NULL && stack->delete_item != NULL) {
            stack->delete_item(item);
        }
    }
}
