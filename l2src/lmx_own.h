/* lmx_own.h - own graph-backed fields as C working locals.
 *
 * Lingvamyxa_spec.txt 21.5, 21.6. Its own header rather than an addition to
 * lmx.h, so this unit is separable.
 *
 * 21.5: at entry the generated body loads only those own fields it actually
 * uses through unqualified own bindings into typed automatic C working locals,
 * and each local records the exact field occurrence it was loaded from. Under
 * the settled representation that occurrence is a slot of the containing
 * Structure's child array: `from` is &children[i], and the field's value is
 * the child pointer stored there. These C values are the active method state
 * between observable boundaries.
 *
 * 21.6: each has conceptual dirty state, set ONLY by an executed assignment
 * since the last successful checkpoint - not by comparison with the graph, not
 * by a branch that might have assigned, not by capability. Dirty locals are
 * published before every outbound call and every exit. Clean ones must not be
 * republished. There is no automatic reload after a call.
 */
#ifndef LMX_OWN_H
#define LMX_OWN_H

#include "l2src/lmx.h"

typedef struct LmxOwnLocal {
    void **from;    /* the exact slot this was loaded from (21.5) */
    void *value;    /* the C working value: the child pointer */
    int dirty;      /* set only by an executed write (21.6) */
} LmxOwnLocal;

#endif
