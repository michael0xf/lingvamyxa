#ifndef LM_OWN_LM1_H
#define LM_OWN_LM1_H

#include <stddef.h>

typedef void (*LmOwnDestroyFields)(void *object);
typedef void (*LmOwnDelete)(void *object);

void lm_own_delete(void *object, LmOwnDestroyFields destroy_fields);
void lm_own_pointer_array_delete(void **items, size_t count, LmOwnDelete delete_item);

#endif
