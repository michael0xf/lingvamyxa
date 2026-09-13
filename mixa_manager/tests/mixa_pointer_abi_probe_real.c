/* ABI parity guard (ticket 20260913-113000). Prints sizeof and every
 * field offset of the REAL MixaPointer struct, as seen through the
 * genuine production header (mixa_manager/mixa_pointer.h). The runner
 * compiles this AND mixa_pointer_abi_probe_l2.c (which probes the L2
 * predef'd header instead) and diffs their output byte-for-byte before
 * ever building or running the actual parity harness. MixaPointer has
 * no dependency on MixaCell/MixaTextRect or any other manager struct,
 * so this probe pair is dedicated to this module alone (does not reuse
 * the tiles ABI probes).
 */
#include <stdio.h>
#include <stddef.h>
#include "mixa_manager/mixa_pointer.h"

int main(void) {
    printf("MixaPointer sizeof=%zu\n", sizeof(MixaPointer));
    printf("MixaPointer is_open=%zu\n", offsetof(MixaPointer, is_open));
    printf("MixaPointer octant=%zu\n", offsetof(MixaPointer, octant));
    printf("MixaPointer row=%zu\n", offsetof(MixaPointer, row));
    printf("MixaPointer col=%zu\n", offsetof(MixaPointer, col));
    return 0;
}
