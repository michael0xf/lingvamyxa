/* ABI parity guard (ticket 20260913-142800). Prints sizeof and every
 * field offset of the REAL MixaAppPathFaultVTable struct, as seen
 * through the genuine production header (mixa_manager/mixa_app_path.h
 * -- a plain, hand-written C header). The runner compiles this AND
 * mixa_app_path_abi_probe_l2.c (which probes the L2 predef'd header
 * instead) and diffs their output byte-for-byte before ever building
 * or running the actual parity harness.
 */
#include <stdio.h>
#include <stddef.h>
#include "mixa_manager/mixa_app_path.h"

int main(void) {
    printf("MixaAppPathFaultVTable sizeof=%zu\n", sizeof(MixaAppPathFaultVTable));
    printf("MixaAppPathFaultVTable on_alloc=%zu\n", offsetof(MixaAppPathFaultVTable, on_alloc));
    printf("MixaAppPathFaultVTable ctx=%zu\n", offsetof(MixaAppPathFaultVTable, ctx));
    return 0;
}
