/* ABI parity guard (ticket 20260913-121500). Prints sizeof and every
 * field offset of the REAL MixaConsoleView/MixaConsolePending structs,
 * as seen through the real production header (mixa_console_window.h.
 * lm1, translated to mixa_console_window.lm1.h by the runner). There
 * is no separate plain-C production header for this module (like
 * mixa_process_marker) -- both this probe and mixa_console_window_abi_
 * probe_l2.c compile against l1trans-generated output, one from the
 * real header, one from the L2 header, and the runner diffs their
 * output byte-for-byte before ever building or running the actual
 * parity harness.
 */
#include <stdio.h>
#include <stddef.h>
#include "mixa_manager/mixa_console_window.lm1.h"

int main(void) {
    printf("MixaConsoleView sizeof=%zu\n", sizeof(MixaConsoleView));
    printf("MixaConsoleView file=%zu\n", offsetof(MixaConsoleView, file));
    printf("MixaConsoleView rows=%zu\n", offsetof(MixaConsoleView, rows));
    printf("MixaConsoleView cols=%zu\n", offsetof(MixaConsoleView, cols));
    printf("MixaConsoleView top_offset=%zu\n", offsetof(MixaConsoleView, top_offset));
    printf("MixaConsoleView cursor_row=%zu\n", offsetof(MixaConsoleView, cursor_row));
    printf("MixaConsoleView cursor_col=%zu\n", offsetof(MixaConsoleView, cursor_col));
    printf("MixaConsoleView file_size=%zu\n", offsetof(MixaConsoleView, file_size));

    printf("MixaConsolePending sizeof=%zu\n", sizeof(MixaConsolePending));
    printf("MixaConsolePending cps=%zu\n", offsetof(MixaConsolePending, cps));
    printf("MixaConsolePending len=%zu\n", offsetof(MixaConsolePending, len));
    printf("MixaConsolePending cursor=%zu\n", offsetof(MixaConsolePending, cursor));
    return 0;
}
