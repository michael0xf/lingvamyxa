#include "own.lm1.h"
#include <stdlib.h>
#include <string.h>

void *lm_own_new_zero(size_t size) {
    return calloc(1U, size);
}

void lm_own_delete(void *object, LmOwnDestroyFields destroy_fields) {
    if (object != 0) {
        if (destroy_fields != 0) {
            destroy_fields(object);
        }
        free(object);
    }
}

void lm_own_pointer_array_delete(void **items, size_t count, LmOwnDelete delete_item) {
    size_t index;

    if (items != 0) {
        if (delete_item != 0) {
            for (index = 0U; index < count; ++index) {
                if (items[index] != 0) {
                    delete_item(items[index]);
                    items[index] = 0;
                }
            }
        }
        free(items);
    }
}

void lm_own_ptr_stack_init(LmOwnPtrStack *stack, LmOwnDelete delete_item) {
    if (stack != 0) {
        stack->items = 0;
        stack->count = 0U;
        stack->capacity = 0U;
        stack->delete_item = delete_item;
    }
}

void lm_own_ptr_stack_destroy(LmOwnPtrStack *stack) {
    if (stack != 0) {
        lm_own_pointer_array_delete(stack->items, stack->count, stack->delete_item);
        stack->items = 0;
        stack->count = 0U;
        stack->capacity = 0U;
        stack->delete_item = 0;
    }
}

int lm_own_ptr_stack_push(LmOwnPtrStack *stack, void *item) {
    void **items;
    size_t capacity;

    if (stack == 0) {
        return 1;
    }

    if (stack->count == stack->capacity) {
        capacity = stack->capacity == 0U ? 8U : stack->capacity * 2U;
        items = (void **)realloc(stack->items, capacity * sizeof(*items));
        if (items == 0) {
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

    if (stack == 0 || stack->count == 0U) {
        return 0;
    }

    --stack->count;
    item = stack->items[stack->count];
    stack->items[stack->count] = 0;
    return item;
}

void *lm_own_ptr_stack_at(const LmOwnPtrStack *stack, size_t index) {
    if (stack == 0 || index >= stack->count) {
        return 0;
    }

    return stack->items[index];
}

void *lm_own_ptr_stack_top(const LmOwnPtrStack *stack) {
    if (stack == 0 || stack->count == 0U) {
        return 0;
    }

    return stack->items[stack->count - 1U];
}

void lm_own_ptr_stack_truncate(LmOwnPtrStack *stack, size_t count) {
    void *item;

    if (stack == 0) {
        return;
    }

    while (stack->count > count) {
        item = lm_own_ptr_stack_pop(stack);
        if (item != 0 && stack->delete_item != 0) {
            stack->delete_item(item);
        }
    }
}

void lm_own_value_stack_init(LmOwnValueStack *stack, size_t item_size) {
    if (stack != 0) {
        stack->items = 0;
        stack->count = 0U;
        stack->capacity = 0U;
        stack->item_size = item_size;
    }
}

void lm_own_value_stack_destroy(LmOwnValueStack *stack) {
    if (stack != 0) {
        free(stack->items);
        stack->items = 0;
        stack->count = 0U;
        stack->capacity = 0U;
        stack->item_size = 0U;
    }
}

int lm_own_value_stack_push(LmOwnValueStack *stack, const void *item) {
    unsigned char *items;
    size_t capacity;

    if (stack == 0 || item == 0 || stack->item_size == 0U) {
        return 1;
    }

    if (stack->count == stack->capacity) {
        capacity = stack->capacity == 0U ? 8U : stack->capacity * 2U;
        items = (unsigned char *)realloc(stack->items, capacity * stack->item_size);
        if (items == 0) {
            return 1;
        }
        stack->items = items;
        stack->capacity = capacity;
    }

    memcpy((unsigned char *)stack->items + stack->count * stack->item_size, item, stack->item_size);
    ++stack->count;
    return 0;
}

int lm_own_value_stack_resize_zero(LmOwnValueStack *stack, size_t count) {
    unsigned char *items;
    size_t capacity;
    size_t previous_count;

    if (stack == 0 || stack->item_size == 0U) {
        return 1;
    }

    if (count > stack->capacity) {
        capacity = stack->capacity == 0U ? 8U : stack->capacity;
        while (capacity < count) {
            capacity *= 2U;
        }
        items = (unsigned char *)realloc(stack->items, capacity * stack->item_size);
        if (items == 0) {
            return 1;
        }
        stack->items = items;
        stack->capacity = capacity;
    }

    previous_count = stack->count;
    if (count > previous_count) {
        memset(
            (unsigned char *)stack->items + previous_count * stack->item_size,
            0,
            (count - previous_count) * stack->item_size
        );
    }
    stack->count = count;
    return 0;
}

int lm_own_value_stack_pop(LmOwnValueStack *stack, void *out_item) {
    unsigned char *source;

    if (stack == 0 || stack->count == 0U || stack->item_size == 0U) {
        return 1;
    }

    --stack->count;
    source = (unsigned char *)stack->items + stack->count * stack->item_size;
    if (out_item != 0) {
        memcpy(out_item, source, stack->item_size);
    }
    return 0;
}

void *lm_own_value_stack_at(const LmOwnValueStack *stack, size_t index) {
    if (stack == 0 || index >= stack->count || stack->item_size == 0U) {
        return 0;
    }

    return (unsigned char *)stack->items + index * stack->item_size;
}

void *lm_own_value_stack_top(const LmOwnValueStack *stack) {
    if (stack == 0 || stack->count == 0U || stack->item_size == 0U) {
        return 0;
    }

    return (unsigned char *)stack->items + (stack->count - 1U) * stack->item_size;
}

void lm_own_value_stack_truncate(LmOwnValueStack *stack, size_t count) {
    if (stack != 0 && count < stack->count) {
        stack->count = count;
    }
}

void lm_own_arena_init(LmOwnArena *arena) {
    if (arena != 0) {
        lm_own_ptr_stack_init(&arena->allocations, free);
        lm_own_value_stack_init(&arena->lazy_edges, sizeof(LmOwnLazyEdge));
        arena->frozen = 0;
    }
}

void lm_own_arena_destroy(LmOwnArena *arena) {
    if (arena != 0) {
        lm_own_value_stack_destroy(&arena->lazy_edges);
        lm_own_ptr_stack_destroy(&arena->allocations);
        arena->frozen = 0;
    }
}

void *lm_own_arena_new_zero(LmOwnArena *arena, size_t size) {
    void *object;

    if (arena == 0 || arena->frozen) {
        return 0;
    }

    object = calloc(1U, size);
    if (object == 0) {
        return 0;
    }
    if (lm_own_ptr_stack_push(&arena->allocations, object) != 0) {
        free(object);
        return 0;
    }
    return object;
}

char *lm_own_arena_copy_bytes(LmOwnArena *arena, const char *source, size_t length) {
    char *copy;

    copy = (char *)lm_own_arena_new_zero(arena, length + 1U);
    if (copy == 0) {
        return 0;
    }
    if (source != 0 && length > 0U) {
        memcpy(copy, source, length);
    }
    copy[length] = '\0';
    return copy;
}

int lm_own_arena_add_lazy_edge(
    LmOwnArena *target,
    LmOwnArena *source,
    const void *source_ptr,
    size_t size,
    const void **patch_slot
) {
    LmOwnLazyEdge edge;

    if (target == 0 || source == 0 || patch_slot == 0 || target->frozen) {
        return 1;
    }
    if (size == 0U) {
        return 0;
    }
    if (source_ptr == 0) {
        return 1;
    }

    edge.kind = LM_OWN_EDGE_LAZY_OWNED;
    edge.source_owner = source;
    edge.target_owner = target;
    edge.source = source_ptr;
    edge.size = size;
    edge.patch_slot = patch_slot;
    return lm_own_value_stack_push(&target->lazy_edges, &edge);
}

int lm_own_arena_promote_lazy_edges(LmOwnArena *arena) {
    LmOwnLazyEdge *edge;
    void *copy;
    size_t i;

    if (arena == 0 || arena->frozen) {
        return 1;
    }

    for (i = 0U; i < arena->lazy_edges.count; ++i) {
        edge = (LmOwnLazyEdge *)lm_own_value_stack_at(&arena->lazy_edges, i);
        if (edge == 0 || edge->kind != LM_OWN_EDGE_LAZY_OWNED) {
            continue;
        }
        if (edge->patch_slot == 0 || edge->source == 0 || edge->size == 0U) {
            return 1;
        }

        copy = lm_own_arena_new_zero(arena, edge->size + 1U);
        if (copy == 0) {
            return 1;
        }
        memcpy(copy, edge->source, edge->size);
        *edge->patch_slot = copy;
        edge->source = copy;
        edge->source_owner = arena;
        edge->target_owner = arena;
        edge->kind = LM_OWN_EDGE_OWNED;
    }

    return 0;
}

int lm_own_arena_absorb(LmOwnArena *target, LmOwnArena *source) {
    void **items;
    size_t new_count;
    size_t capacity;
    size_t lazy_base;
    size_t lazy_new_count;
    size_t i;
    LmOwnLazyEdge *edge;
    LmOwnLazyEdge *target_edges;

    if (target == 0 || source == 0 || target->frozen || source == target) {
        return target == source ? 0 : 1;
    }

    lazy_base = target->lazy_edges.count;
    lazy_new_count = lazy_base + source->lazy_edges.count;
    if (source->lazy_edges.count > 0U) {
        if (lm_own_value_stack_resize_zero(&target->lazy_edges, lazy_new_count) != 0) {
            return 1;
        }
        target->lazy_edges.count = lazy_base;
    }

    if (source->allocations.count > 0U) {
        new_count = target->allocations.count + source->allocations.count;
        capacity = target->allocations.capacity == 0U ? 8U : target->allocations.capacity;
        while (capacity < new_count) {
            capacity *= 2U;
        }
        items = (void **)realloc(target->allocations.items, capacity * sizeof(*items));
        if (items == 0) {
            return 1;
        }
        target->allocations.items = items;
        target->allocations.capacity = capacity;
        memcpy(
            target->allocations.items + target->allocations.count,
            source->allocations.items,
            source->allocations.count * sizeof(*source->allocations.items)
        );
        target->allocations.count = new_count;

        free(source->allocations.items);
        source->allocations.items = 0;
        source->allocations.count = 0U;
        source->allocations.capacity = 0U;
    }

    if (source->lazy_edges.count > 0U) {
        target_edges = (LmOwnLazyEdge *)target->lazy_edges.items;
        memcpy(
            target_edges + lazy_base,
            source->lazy_edges.items,
            source->lazy_edges.count * sizeof(*target_edges)
        );
        target->lazy_edges.count = lazy_new_count;
        for (i = lazy_base; i < target->lazy_edges.count; ++i) {
            edge = (LmOwnLazyEdge *)lm_own_value_stack_at(&target->lazy_edges, i);
            if (edge != 0 && edge->target_owner == source) {
                edge->target_owner = target;
            }
        }
        source->lazy_edges.count = 0U;
    }

    return 0;
}

void lm_own_arena_freeze(LmOwnArena *arena) {
    if (arena != 0) {
        arena->frozen = 1;
    }
}

int lm_own_arena_is_frozen(const LmOwnArena *arena) {
    return arena != 0 && arena->frozen;
}

int lm_own_tree_cut(LmOwnArena *arena) {
    return lm_own_tree_cut_promote_lazy_edges(arena);
}

int lm_own_tree_cut_promote_lazy_edges(LmOwnArena *arena) {
    return lm_own_arena_promote_lazy_edges(arena);
}
