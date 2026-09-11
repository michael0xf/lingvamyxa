/* lmx.h - the one Lmx header and the registered address ranges.
 *
 * Lingvamyxa_spec.txt 2, 6.5, 12.1, 19.20. Aggregates come from a hand-written
 * C header pulled in with L1 `include:`, the same route l1src/p0.lm1.h uses; L1 has
 * no decided spelling for aggregate heads of its own (L1_spec.txt OPEN 10.4).
 *
 * The whole point: a field carries no type tag. `data` is a pointer, and its
 * type is whichever registered address range the pointer falls into.
 */
#ifndef LMX_H
#define LMX_H

#include <stddef.h>
#include <stdint.h>

#if defined(_MSC_VER)
extern __declspec(thread) uint_fast8_t *lmx_turn_running;
#else
extern __thread uint_fast8_t *lmx_turn_running;
#endif
int lmx_msg_poll_abort(void);
static inline int lmx_msg_poll_escape(void) {
    uint_fast8_t *p = lmx_turn_running;
    if (p == 0) {
        return 0;
    }
    if (__atomic_load_n(p, __ATOMIC_RELAXED) != 0) {
        return 0;
    }
    return lmx_msg_poll_abort();
}

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
/* 2: "The implementation has N typed service arrays." N is per TYPE, not per
 * category - 6.5 spells it out for one of them: "For every used Array type T the
 * implementation provides a typed service pool, schematically all_array_of_T."
 *
 * So a range carries a concrete type, and the address gives that type outright.
 * A descriptor from all_array_of_int and one from all_array_of_char are both
 * array descriptors, and are told apart by which range they land in, with
 * nothing stored on either.
 *
 * The kind below stays as the coarse reading of an entry - how to interpret its
 * shape, which is the column 2's table gives. The type says which one it is. */
typedef enum LmxType {
    LMX_TYPE_NONE = 0,
    LMX_TYPE_CHAR,              /* primitive pool, 2's all_chars_array */
    LMX_TYPE_INT,               /* primitive pool of ints */
    LMX_TYPE_SIZE_T,            /* mutable primitive pool of size_t */
    LMX_TYPE_METHOD,            /* 2's all_methods_array */
    LMX_TYPE_ARRAY_OF_CHAR,     /* 6.5's all_array_of_T for T = char */
    LMX_TYPE_ARRAY_OF_INT,      /* the same for T = int */
    LMX_TYPE_BRANCH             /* a branch block of Lmx children, 14.1 */
} LmxType;

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
    int type;
} LmxRange;

/* A typed service array. 2 lists them and 6.5 spells the Array one out: one per
 * primitive C type, one of {addr, sig} method records, one of {len, data} Array
 * descriptors per element type.
 *
 * A non-const variable LIVES in the big array of its type, so that array has to
 * grow. It cannot grow by reallocating: 2 requires live addresses to be stable
 * unless relocation atomically rewrites every affected reference, root and
 * index key, and a service array full of live cells is the worst possible thing
 * to relocate. So it grows by adding a CHUNK, and each chunk registers as one
 * more range carrying the same kind and type.
 *
 * A typed service array is therefore a set of registered ranges, not one range.
 * Classification is unaffected - an address falls in exactly one chunk, and
 * every chunk of the array answers with the same type.
 *
 * Two disciplines share this machinery, which is the point. Interned immutable
 * atoms are laid out once and addressed by value - all_chars_array has its 256
 * cells from the start and never grows (14.7's settled half). Mutable variable
 * cells are handed out one at a time and the array grows to fit them. What makes
 * an entry a char is the range it sits in, not which discipline produced it. */
typedef struct LmxChunk LmxChunk;

struct LmxChunk {
    void *base;
    size_t capacity;
    size_t count;
    LmxChunk *next;
};

typedef struct LmxPool {
    LmxChunk *head;
    LmxChunk *tail;
    size_t stride;
    size_t chunk_capacity;
    int kind;
    int type;
} LmxPool;

#define LMX_RANGE_MAX 16

typedef struct LmxRanges {
    LmxRange items[LMX_RANGE_MAX];
    size_t count;
} LmxRanges;

#endif
