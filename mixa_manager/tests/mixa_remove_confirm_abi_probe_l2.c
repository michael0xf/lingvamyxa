/* ABI parity guard (ticket 20260913-175200). Prints sizeof and every
 * field offset of MixaRemoveConfirmCtx as seen through the L2 header's
 * own l1trans-generated C header (mixa_remove_confirm_l2.h.lm1 ->
 * mixa_remove_confirm_l2.lm1.h). Compiled and run alongside mixa_
 * remove_confirm_abi_probe_real.c; the runner diffs their output
 * byte-for-byte before ever building or running the actual parity
 * harness.
 */
#include <stdio.h>
#include <stddef.h>
#include "mixa_manager/mixa_remove_confirm_l2.lm1.h"

int main(void) {
    printf("MixaRemoveConfirmCtx sizeof=%zu\n", sizeof(MixaRemoveConfirmCtx));
    printf("MixaRemoveConfirmCtx stack=%zu\n", offsetof(MixaRemoveConfirmCtx, stack));
    printf("MixaRemoveConfirmCtx rect=%zu\n", offsetof(MixaRemoveConfirmCtx, rect));
    printf("MixaRemoveConfirmCtx fm=%zu\n", offsetof(MixaRemoveConfirmCtx, fm));
    printf("MixaRemoveConfirmCtx window_row=%zu\n", offsetof(MixaRemoveConfirmCtx, window_row));
    printf("MixaRemoveConfirmCtx window_col=%zu\n", offsetof(MixaRemoveConfirmCtx, window_col));
    printf("MixaRemoveConfirmCtx nrows=%zu\n", offsetof(MixaRemoveConfirmCtx, nrows));
    printf("MixaRemoveConfirmCtx ncols=%zu\n", offsetof(MixaRemoveConfirmCtx, ncols));
    printf("MixaRemoveConfirmCtx hb=%zu\n", offsetof(MixaRemoveConfirmCtx, hb));
    printf("MixaRemoveConfirmCtx is_open=%zu\n", offsetof(MixaRemoveConfirmCtx, is_open));
    printf("MixaRemoveConfirmCtx invocations=%zu\n", offsetof(MixaRemoveConfirmCtx, invocations));
    printf("MixaRemoveConfirmCtx last_status=%zu\n", offsetof(MixaRemoveConfirmCtx, last_status));
    printf("MixaRemoveConfirmCtx has_failure=%zu\n", offsetof(MixaRemoveConfirmCtx, has_failure));
    printf("MixaRemoveConfirmCtx failing_path=%zu\n", offsetof(MixaRemoveConfirmCtx, failing_path));
    return 0;
}
