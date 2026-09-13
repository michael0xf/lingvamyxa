/* ABI parity guard (ticket 20260913-130500). Prints sizeof and every
 * field offset of the REAL MixaCmdDispatch struct, as seen through the
 * genuine production header (mixa_manager/mixa_cmdline_dispatch_impl.h,
 * a plain, hand-written C header -- unlike mixa_process_marker/mixa_
 * console_window, no l1trans translation is needed for the real side
 * here). The runner compiles this AND mixa_cmdline_dispatch_abi_probe_
 * l2.c (which probes the L2 predef'd header instead) and diffs their
 * output byte-for-byte before ever building or running the actual
 * parity harness.
 */
#include <stdio.h>
#include <stddef.h>
#include "mixa_manager/mixa_cmdline_dispatch_impl.h"

int main(void) {
    printf("MixaCmdDispatch sizeof=%zu\n", sizeof(MixaCmdDispatch));
    printf("MixaCmdDispatch proc=%zu\n", offsetof(MixaCmdDispatch, proc));
    printf("MixaCmdDispatch scanner=%zu\n", offsetof(MixaCmdDispatch, scanner));
    printf("MixaCmdDispatch console_file=%zu\n", offsetof(MixaCmdDispatch, console_file));
    printf("MixaCmdDispatch wrapper_path=%zu\n", offsetof(MixaCmdDispatch, wrapper_path));
    printf("MixaCmdDispatch done=%zu\n", offsetof(MixaCmdDispatch, done));
    printf("MixaCmdDispatch exit_code=%zu\n", offsetof(MixaCmdDispatch, exit_code));
    printf("MixaCmdDispatch new_cwd=%zu\n", offsetof(MixaCmdDispatch, new_cwd));
    return 0;
}
