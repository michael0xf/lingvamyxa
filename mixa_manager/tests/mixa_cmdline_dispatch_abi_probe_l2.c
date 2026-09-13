/* ABI parity guard (ticket 20260913-130500). Prints sizeof and every
 * field offset of MixaCmdDispatch as seen through the L2 header unit
 * (mixa_cmdline_dispatch_l2.h.lm1, translated to mixa_cmdline_
 * dispatch_l2.lm1.h by the runner). Must be byte-identical to mixa_
 * cmdline_dispatch_abi_probe_real.c's own output.
 */
#include <stdio.h>
#include <stddef.h>
#include "mixa_manager/mixa_cmdline_dispatch_l2.lm1.h"

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
