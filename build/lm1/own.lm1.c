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
