/* ABI parity guard (ticket 20260913-121500). Prints sizeof and every
 * field offset of the same structs, seen through the L2 header unit
 * (mixa_console_window_l2.h.lm1, translated to mixa_console_window_l2.
 * lm1.h by the runner). Must be byte-identical to mixa_console_window_
 * abi_probe_real.c's own output.
 */
#include <stdio.h>
#include <stddef.h>
#include "mixa_manager/mixa_console_window_l2.lm1.h"

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
