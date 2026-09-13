/* C ABI: frozen parser symbol -> L2 compact-atom callable. */
#include "l2src/lmx.h"
#include "l2src/lmx_branch_owned.lm1.h"
#include <stddef.h>
#include <stdio.h>

extern Lmx *l2_registry_unit;
int l2_registry_boot(void);
size_t l2_registry_m0(Lmx *node, const char *text, size_t end_index, size_t start);

int l2_registry_compact_calls = 0;

static void l2_registry_ensure(void) {
    if (l2_registry_unit == 0) {
        if (l2_registry_boot() != 0 || l2_registry_unit == 0) {
            fprintf(stderr, "l2_registry_boot failed\n");
        }
    }
}

size_t lm_p0_scan_registry_compact_atom_piece(
    const char *text, size_t end_index, size_t start) {
    l2_registry_ensure();
    l2_registry_compact_calls += 1;
    return l2_registry_m0(
        lmx_branch_struct_known(l2_registry_unit, 0U), text, end_index, start);
}
