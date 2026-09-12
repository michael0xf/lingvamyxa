/* lmx.h - the one Lmx header and the registered address ranges.
 *
 * Lingvamyxa_spec.txt 2, 6.5, 12.1, 19.20; struct_refactoring_version_2.txt
 * 3.1, 3.3, 3.6, 14.1, 14.2, 14.22. Aggregates come from a hand-written C
 * header pulled in with L1 `include:`, the same route l1src/p0.lm1.h uses; L1
 * has no decided spelling for aggregate heads of its own (L1_spec.txt OPEN 10.4).
 *
 * The whole point: a child carries no type tag. A Structure's data addresses
 * an ordered array of void * child values; the type of a child is whichever
 * registered address range its stored VALUE falls into. The address of the
 * slot itself identifies nothing.
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

/* SPEC 2 / ABI 3.1: the only universal Structure representation, settled by
 * the user on 2026-09-12. Field order is normative.
 *
 *   node  the lexical parent; NULL at a lexical root. Merge and new Message
 *         creation copy the used graph with the same traversal, rewriting
 *         destination node links through the source-to-copy map.
 *   len   exclusively the number of immediate children. Not bytes, not
 *         characters, not an Array length (that is LmxArrayDesc.len).
 *   data  points to an ordered array of len child pointers of type void *:
 *
 *             void **children = (void **)s->data;
 *             void *child = children[i];        0 <= i < s->len
 *
 * A child VALUE is the address of that child in the typed array/range of its
 * type T: a Structure child points at another Lmx header; a primitive child
 * points directly at its cell (an interned char is &all_chars['!']); an Array
 * child points at its {len, data} record; a known function child points at its
 * {addr, sig} record. Children are never inline Lmx records, and only a target
 * classified as a Structure has node/len/data. A callable stores nothing of its
 * own: the reserved node argument of a call is the Structure through whose
 * child array the callable pointer was invoked (SPEC 21.2, 21.8).
 *
 * The field count is fixed at construction (ABI 14.22). Nested code may replace
 * the void * references in the slots; it never adds, moves or removes slots.
 * Nothing else per node - no name, no type tag, no descriptor, no vtable. */
struct Lmx {
    Lmx *node;
    int len;
    void *data;
};

/* What a registered range means. 2 lists the child interpretations; these are
 * the ones this tree registers. */
/* 2: "There is a separate address domain for each element type T; a domain may
 * consist of multiple stable blocks." N typed service arrays, one per TYPE, not
 * per category - 6.5 spells it out for one of them: "For every used Array type
 * T the implementation provides a typed service pool, schematically
 * all_array_of_T."
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
    LMX_TYPE_BRANCH,            /* a Structure's void * child-pointer array */
    /* Distinct all_array_of_T identifiers. Not one ARRAY_OF_REF category.
     * An array is classified by its descriptor address; backing cells of the
     * same pool reuse this T. Graph / Array-descriptor / METHOD referents. */
    LMX_TYPE_LMX,               /* T = graph node (pointer to Lmx) */
    LMX_TYPE_DESC,              /* T = Array descriptor (pointer to LmxArrayDesc) */
    LMX_TYPE_ARRAY_OF_LMX,      /* all_array_of_T for T = Lmx* */
    LMX_TYPE_ARRAY_OF_DESC,     /* all_array_of_T for T = LmxArrayDesc* */
    LMX_TYPE_ARRAY_OF_METHOD,   /* all_array_of_T for T = LmxMethod* */
    LMX_TYPE_STRUCT,            /* an Lmx header: the Structure domain itself */
    /* SPEC 11.2.1 / ABI 3.3: the L2 form `@: char "hello"` is a child pointer
     * into a typed array of char * values, each pointing directly at a C
     * string. No length, no Array record. Kind PRIMITIVE, stride sizeof(char *). */
    LMX_TYPE_CHAR_PTR
} LmxType;

typedef enum LmxKind {
    LMX_KIND_NONE = 0,      /* not a classifiable high-level value */
    LMX_KIND_PRIMITIVE,     /* primitive pool entry, e.g. all_chars_array */
    LMX_KIND_METHOD,        /* all_methods_array {addr, sig} record */
    LMX_KIND_ARRAY,         /* typed Array pool entry {len, data} */
    LMX_KIND_CHILDREN,      /* a Structure's void * child-pointer array */
    LMX_KIND_REF,           /* pointer-valued array element; type is T */
    LMX_KIND_STRUCT         /* an Lmx header {node, len, data} */
} LmxKind;

/* 2: an immutable method record. addr is the generated C entry; sig fixes the
 * typed input list. Stored in a pool, never in the Lmx header. A child pointer
 * to a known function is the address of this record directly. Methods and their
 * immutable descriptors are shared: graph copying preserves this pointer,
 * while Structure/node links are remapped. The record stores no node. */
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
 * descriptors per element type, and one of Lmx headers.
 *
 * A non-const variable LIVES in the big array of its type, so that array has to
 * grow. It cannot grow by reallocating: live L1/L2 arena entries never
 * relocate, including at end_turn. A service array full of live cells is the
 * worst possible thing to move. So it grows by adding a CHUNK, and each chunk
 * registers as one more range carrying the same kind and type.
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
