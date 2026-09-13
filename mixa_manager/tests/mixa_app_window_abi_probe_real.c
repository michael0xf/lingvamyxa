/* ABI parity guard (ticket 20260913-153000). Prints sizeof and every
 * field offset of the REAL MixaAppWindow/MixaAppWindowStack structs,
 * as seen through the genuine production header (mixa_manager/mixa_
 * app_window.h.lm1's own l1trans-generated C header). The runner
 * compiles this AND mixa_app_window_abi_probe_l2.c (which probes the
 * L2 header's own generated C header instead) and diffs their output
 * byte-for-byte before ever building or running the actual parity
 * harness.
 */
#include <stdio.h>
#include <stddef.h>
#include "mixa_manager/mixa_app_window.lm1.h"

int main(void) {
    printf("MixaAppWindow sizeof=%zu\n", sizeof(MixaAppWindow));
    printf("MixaAppWindow saved=%zu\n", offsetof(MixaAppWindow, saved));
    printf("MixaAppWindow saved_count=%zu\n", offsetof(MixaAppWindow, saved_count));
    printf("MixaAppWindow panel=%zu\n", offsetof(MixaAppWindow, panel));
    printf("MixaAppWindow msg_row=%zu\n", offsetof(MixaAppWindow, msg_row));
    printf("MixaAppWindow msg_col=%zu\n", offsetof(MixaAppWindow, msg_col));
    printf("MixaAppWindow origin_row=%zu\n", offsetof(MixaAppWindow, origin_row));
    printf("MixaAppWindow origin_col=%zu\n", offsetof(MixaAppWindow, origin_col));
    printf("MixaAppWindow nrows=%zu\n", offsetof(MixaAppWindow, nrows));
    printf("MixaAppWindow ncols=%zu\n", offsetof(MixaAppWindow, ncols));
    printf("MixaAppWindow hb=%zu\n", offsetof(MixaAppWindow, hb));
    printf("MixaAppWindow is_open=%zu\n", offsetof(MixaAppWindow, is_open));

    printf("MixaAppWindowStack sizeof=%zu\n", sizeof(MixaAppWindowStack));
    printf("MixaAppWindowStack windows=%zu\n", offsetof(MixaAppWindowStack, windows));
    printf("MixaAppWindowStack count=%zu\n", offsetof(MixaAppWindowStack, count));
    printf("MixaAppWindowStack cap=%zu\n", offsetof(MixaAppWindowStack, cap));
    printf("MixaAppWindowStack is_open=%zu\n", offsetof(MixaAppWindowStack, is_open));
    return 0;
}
