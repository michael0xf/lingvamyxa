/* lmx.h - the one Lmx header and the registered address ranges.
 *
 * Lingvamyxa_spec.txt 2, 6.5, 12.1, 19.20. Aggregates come from a hand-written
 * C header pulled in with L1 `include:`, the same route l1src/p0.h uses; L1 has
 * no decided spelling for aggregate heads of its own (L1_spec.txt OPEN 10.4).
 *
 * The whole point: a field carries no type tag. `data` is a pointer, and its
 * type is whichever registered address range the pointer falls into.
 */
#ifndef LMX_H
#define LMX_H

#include <stddef.h>

typedef struct Lmx Lmx;

/* 2: the only universal node representation. node is the containment parent,
 * data is classified by range. Nothing else per node - no name, no type tag,
 * no descriptor prefix, no vtable.
 *
 * len is DECLARED but not yet MEANT. Its unit is open
 * (struct_refactoring_version_2.txt 14.2: child count, element count, byte
 * count or another explicit unit, one rule per range), and the encoding of an
 * empty value is open with it (14.3). No code here may read len until both
 * close. See l2src/OPEN_POINTS.txt. */
struct Lmx {
    Lmx *node;
    size_t len;
    void *data;
};

/* What a registered range means. 2 lists the service-entry interpretations;
 * these are the ones this unit registers. */
typedef enum LmxKind {
    LMX_KIND_NONE = 0,      /* not a classifiable high-level value */
    LMX_KIND_PRIMITIVE,     /* primitive pool entry, e.g. all_chars_array */
    LMX_KIND_METHOD,        /* all_methods_array {addr, sig} record */
    LMX_KIND_ARRAY,         /* typed Array pool entry {len, data} */
    LMX_KIND_CHILDREN       /* branch storage of a non-leaf occurrence */
} LmxKind;

/* 2: an immutable method record. addr is the generated C entry; sig fixes the
 * typed input list. Stored in a pool, never in the Lmx header. */
typedef void (*LmxEntry)(void);

typedef struct LmxMethod {
    LmxEntry addr;
    unsigned sig;
} LmxMethod;

/* 6.5: an Array descriptor has exactly two values and no Lmx node. */
typedef struct LmxArrayDesc {
    size_t len;
    void *data;
} LmxArrayDesc;

/* One registered range. Half-open [lo, hi). stride is the entry size, so a
 * classified pointer can also be turned back into an entry index. */
typedef struct LmxRange {
    void *lo;
    void *hi;
    size_t stride;
    int kind;
} LmxRange;

/* A typed service pool: one contiguous block of same-sized entries whose
 * address range is registered under one kind. 2 lists them - a primitive pool
 * such as all_chars_array, all_methods_array of {addr, sig} records, a typed
 * Array pool all_array_of_T of {len, data} descriptors. Pool storage and the
 * application storage an entry describes are different things (6.5).
 *
 * The pool header is not an Lmx node and is never handed out; only entry
 * addresses are, and an entry's type is read from the range it lands in. */
typedef struct LmxPool {
    void *base;
    size_t stride;
    size_t capacity;
    size_t count;
    int kind;
} LmxPool;

#define LMX_RANGE_MAX 16

typedef struct LmxRanges {
    LmxRange items[LMX_RANGE_MAX];
    size_t count;
} LmxRanges;

#endif
