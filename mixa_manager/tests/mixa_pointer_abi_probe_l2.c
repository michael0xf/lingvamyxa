/* ABI parity guard (ticket 20260913-113000). Prints sizeof and every
 * field offset of MixaPointer as seen through the L2 header unit
 * (mixa_pointer_l2.h.lm1, translated to mixa_pointer_l2.lm1.h by the
 * runner before this file is compiled). Must be byte-identical to
 * mixa_pointer_abi_probe_real.c's own output.
 */
#include <stdio.h>
#include <stddef.h>
#include "mixa_manager/mixa_pointer_l2.lm1.h"

int main(void) {
    printf("MixaPointer sizeof=%zu\n", sizeof(MixaPointer));
    printf("MixaPointer is_open=%zu\n", offsetof(MixaPointer, is_open));
    printf("MixaPointer octant=%zu\n", offsetof(MixaPointer, octant));
    printf("MixaPointer row=%zu\n", offsetof(MixaPointer, row));
    printf("MixaPointer col=%zu\n", offsetof(MixaPointer, col));
    return 0;
}
