#ifndef L2SRC_L2_IMMUT_QUERY_H
#define L2SRC_L2_IMMUT_QUERY_H

#include <stddef.h>
#include <stdint.h>

/*
 * Explicit borrowed immutable-query adapter. Not LmP0Text and not an
 * Lmx Structure with a C layout.
 *
 * data points at caller-owned bytes that must stay unchanged and
 * alive for the lifetime of this object. A program-lifetime C string
 * literal satisfies that. A const char * formal does not prove it
 * for an arbitrary buffer.
 *
 * The adapter does not own or copy the bytes. hash is FNV-1a 64 of
 * those bytes, computed once by l2_immut_query_fill. live==0 means
 * invalid: compares must not read data/hash.
 */
typedef struct L2ImmutQuery {
    const char *data;
    size_t length;
    uint64_t hash;
    int live;
} L2ImmutQuery;

#endif
