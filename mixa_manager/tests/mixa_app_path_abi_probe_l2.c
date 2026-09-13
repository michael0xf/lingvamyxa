/* ABI parity guard (ticket 20260913-142800). Prints sizeof and every
 * field offset of MixaAppPathFaultVTable as seen through the L2 header
 * unit (mixa_app_path_l2.h.lm1, translated to mixa_app_path_l2.lm1.h
 * by the runner). Must be byte-identical to mixa_app_path_abi_probe_
 * real.c's own output.
 */
#include <stdio.h>
#include <stddef.h>
#include "mixa_manager/mixa_app_path_l2.lm1.h"

int main(void) {
    printf("MixaAppPathFaultVTable sizeof=%zu\n", sizeof(MixaAppPathFaultVTable));
    printf("MixaAppPathFaultVTable on_alloc=%zu\n", offsetof(MixaAppPathFaultVTable, on_alloc));
    printf("MixaAppPathFaultVTable ctx=%zu\n", offsetof(MixaAppPathFaultVTable, ctx));
    return 0;
}
