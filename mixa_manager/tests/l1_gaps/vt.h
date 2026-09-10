#ifndef L1GAPS_VT_H
#define L1GAPS_VT_H
#include <stddef.h>
typedef struct VT { int x; } VT;

/* The bridge: name the inner pointer type, then qualify the ALIAS. */
typedef const VT *VTRef;

/* The exact prototype the bridge must match. */
const VT *const *reg_tables(size_t *count);
#endif
