/* ABI parity guard (ticket 20260913-124500). Prints sizeof and every
 * field offset of the same structs, seen through the L2 header unit
 * (mixa_cmdline_l2.h.lm1, translated to mixa_cmdline_l2.lm1.h by the
 * runner). Must be byte-identical to mixa_cmdline_abi_probe_real.c's
 * own output.
 */
#include <stdio.h>
#include <stddef.h>
#include "mixa_manager/mixa_cmdline_l2.lm1.h"

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
