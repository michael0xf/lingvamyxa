/* ABI parity guard (ticket 20260913-145500). Prints sizeof and every
 * field offset of MixaHelpCtx as seen through the L2 header unit
 * (mixa_help_l2.h.lm1, translated to mixa_help_l2.lm1.h by the
 * runner). Must be byte-identical to mixa_help_abi_probe_real.c's own
 * output.
 */
#include <stdio.h>
#include <stddef.h>
#include "mixa_manager/mixa_help_l2.lm1.h"

int main(void) {
    printf("MixaHelpCtx sizeof=%zu\n", sizeof(MixaHelpCtx));
    printf("MixaHelpCtx console_file=%zu\n", offsetof(MixaHelpCtx, console_file));
    printf("MixaHelpCtx last_status=%zu\n", offsetof(MixaHelpCtx, last_status));
    return 0;
}
