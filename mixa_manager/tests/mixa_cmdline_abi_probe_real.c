/* ABI parity guard (ticket 20260913-124500). Prints sizeof and every
 * field offset of the REAL MixaCmdLineFaultVTable/MixaCmdLine structs,
 * as seen through the genuine production header (mixa_manager/mixa_
 * cmdline.h -- a plain, hand-written C header, unlike mixa_process_
 * marker/mixa_console_window). The runner compiles this AND mixa_
 * cmdline_abi_probe_l2.c (which probes the L2 predef'd header instead)
 * and diffs their output byte-for-byte before ever building or running
 * the actual parity harness.
 */
#include <stdio.h>
#include <stddef.h>
#include "mixa_manager/mixa_cmdline.h"

int main(void) {
    printf("MixaCmdLineFaultVTable sizeof=%zu\n", sizeof(MixaCmdLineFaultVTable));
    printf("MixaCmdLineFaultVTable on_alloc=%zu\n", offsetof(MixaCmdLineFaultVTable, on_alloc));
    printf("MixaCmdLineFaultVTable ctx=%zu\n", offsetof(MixaCmdLineFaultVTable, ctx));

    printf("MixaCmdLine sizeof=%zu\n", sizeof(MixaCmdLine));
    printf("MixaCmdLine cps=%zu\n", offsetof(MixaCmdLine, cps));
    printf("MixaCmdLine len=%zu\n", offsetof(MixaCmdLine, len));
    printf("MixaCmdLine cap=%zu\n", offsetof(MixaCmdLine, cap));
    printf("MixaCmdLine cursor=%zu\n", offsetof(MixaCmdLine, cursor));
    return 0;
}
