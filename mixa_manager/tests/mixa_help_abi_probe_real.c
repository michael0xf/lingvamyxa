/* ABI parity guard (ticket 20260913-145500). Prints sizeof and every
 * field offset of the REAL MixaHelpCtx struct, as seen through the
 * genuine production header (mixa_manager/mixa_help.h -- a plain,
 * hand-written C header). The runner compiles this AND mixa_help_abi_
 * probe_l2.c (which probes the L2 predef'd header instead) and diffs
 * their output byte-for-byte before ever building or running the
 * actual parity harness.
 */
#include <stdio.h>
#include <stddef.h>
#include "mixa_manager/mixa_help.h"

int main(void) {
    printf("MixaHelpCtx sizeof=%zu\n", sizeof(MixaHelpCtx));
    printf("MixaHelpCtx console_file=%zu\n", offsetof(MixaHelpCtx, console_file));
    printf("MixaHelpCtx last_status=%zu\n", offsetof(MixaHelpCtx, last_status));
    return 0;
}
