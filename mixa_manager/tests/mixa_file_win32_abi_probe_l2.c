/* ABI parity guard (ticket 20260913-114500). Prints sizeof and every
 * field offset of MixaFile as seen through the L2 header unit (mixa_
 * file_win32_l2.h.lm1, translated to mixa_file_win32_l2.lm1.h by the
 * runner before this file is compiled). Must be byte-identical to
 * mixa_file_win32_abi_probe_real.c's own output -- see that file's own
 * comment and mixa_file_win32_l2.h.lm1's own comment for why the
 * opaque HANDLE field is declared as a plain `void *handle` here.
 */
#include <stdio.h>
#include <stddef.h>
#include "mixa_manager/mixa_file_win32_l2.lm1.h"

int main(void) {
    printf("MixaFile sizeof=%zu\n", sizeof(MixaFile));
    printf("MixaFile handle=%zu\n", offsetof(MixaFile, handle));
    printf("MixaFile mode=%zu\n", offsetof(MixaFile, mode));
    printf("MixaFile is_open=%zu\n", offsetof(MixaFile, is_open));
    return 0;
}
