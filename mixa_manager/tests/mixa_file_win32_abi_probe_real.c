/* ABI parity guard (ticket 20260913-114500). Prints sizeof and every
 * field offset of the REAL MixaFile struct, as seen through the
 * genuine production header (mixa_manager/mixa_file_win32.h). The
 * runner compiles this AND mixa_file_win32_abi_probe_l2.c (which
 * probes the L2 predef'd header instead) and diffs their output
 * byte-for-byte before ever building or running the actual parity
 * harness.
 */
#include <stdio.h>
#include <stddef.h>
#include "mixa_manager/mixa_file_win32.h"

int main(void) {
    printf("MixaFile sizeof=%zu\n", sizeof(MixaFile));
    printf("MixaFile handle=%zu\n", offsetof(MixaFile, handle));
    printf("MixaFile mode=%zu\n", offsetof(MixaFile, mode));
    printf("MixaFile is_open=%zu\n", offsetof(MixaFile, is_open));
    return 0;
}
