/* ABI parity guard (ticket 20260913-150500). Prints sizeof and every
 * field offset of MixaAppFmPanel as seen through the L2 header unit
 * (mixa_app_fmpanel_l2.h.lm1, translated to mixa_app_fmpanel_l2.lm1.h
 * by the runner). Must be byte-identical to mixa_app_fmpanel_abi_
 * probe_real.c's own output.
 */
#include <stdio.h>
#include <stddef.h>
#include "mixa_manager/mixa_app_fmpanel_l2.lm1.h"

int main(void) {
    printf("MixaAppFmPanel sizeof=%zu\n", sizeof(MixaAppFmPanel));
    printf("MixaAppFmPanel fm=%zu\n", offsetof(MixaAppFmPanel, fm));
    printf("MixaAppFmPanel list_row=%zu\n", offsetof(MixaAppFmPanel, list_row));
    printf("MixaAppFmPanel list_col=%zu\n", offsetof(MixaAppFmPanel, list_col));
    printf("MixaAppFmPanel list_rows=%zu\n", offsetof(MixaAppFmPanel, list_rows));
    printf("MixaAppFmPanel list_cols=%zu\n", offsetof(MixaAppFmPanel, list_cols));
    printf("MixaAppFmPanel action_row=%zu\n", offsetof(MixaAppFmPanel, action_row));
    printf("MixaAppFmPanel action_col=%zu\n", offsetof(MixaAppFmPanel, action_col));
    printf("MixaAppFmPanel confirm_open=%zu\n", offsetof(MixaAppFmPanel, confirm_open));
    printf("MixaAppFmPanel confirm_row=%zu\n", offsetof(MixaAppFmPanel, confirm_row));
    printf("MixaAppFmPanel confirm_col=%zu\n", offsetof(MixaAppFmPanel, confirm_col));
    printf("MixaAppFmPanel confirm_rows=%zu\n", offsetof(MixaAppFmPanel, confirm_rows));
    printf("MixaAppFmPanel confirm_cols=%zu\n", offsetof(MixaAppFmPanel, confirm_cols));
    printf("MixaAppFmPanel cancel_row=%zu\n", offsetof(MixaAppFmPanel, cancel_row));
    printf("MixaAppFmPanel cancel_col=%zu\n", offsetof(MixaAppFmPanel, cancel_col));
    printf("MixaAppFmPanel cancel_w=%zu\n", offsetof(MixaAppFmPanel, cancel_w));
    printf("MixaAppFmPanel ok_row=%zu\n", offsetof(MixaAppFmPanel, ok_row));
    printf("MixaAppFmPanel ok_col=%zu\n", offsetof(MixaAppFmPanel, ok_col));
    printf("MixaAppFmPanel ok_w=%zu\n", offsetof(MixaAppFmPanel, ok_w));
    printf("MixaAppFmPanel saved=%zu\n", offsetof(MixaAppFmPanel, saved));
    printf("MixaAppFmPanel saved_count=%zu\n", offsetof(MixaAppFmPanel, saved_count));
    printf("MixaAppFmPanel saved_row=%zu\n", offsetof(MixaAppFmPanel, saved_row));
    printf("MixaAppFmPanel saved_col=%zu\n", offsetof(MixaAppFmPanel, saved_col));
    printf("MixaAppFmPanel saved_rows=%zu\n", offsetof(MixaAppFmPanel, saved_rows));
    printf("MixaAppFmPanel saved_cols=%zu\n", offsetof(MixaAppFmPanel, saved_cols));
    printf("MixaAppFmPanel invocations=%zu\n", offsetof(MixaAppFmPanel, invocations));
    printf("MixaAppFmPanel has_failure=%zu\n", offsetof(MixaAppFmPanel, has_failure));
    printf("MixaAppFmPanel last_status=%zu\n", offsetof(MixaAppFmPanel, last_status));
    printf("MixaAppFmPanel failing_path=%zu\n", offsetof(MixaAppFmPanel, failing_path));
    printf("MixaAppFmPanel has_copy_result=%zu\n", offsetof(MixaAppFmPanel, has_copy_result));
    printf("MixaAppFmPanel copy_last_status=%zu\n", offsetof(MixaAppFmPanel, copy_last_status));
    printf("MixaAppFmPanel view_top=%zu\n", offsetof(MixaAppFmPanel, view_top));
    printf("MixaAppFmPanel highlight_idx=%zu\n", offsetof(MixaAppFmPanel, highlight_idx));
    printf("MixaAppFmPanel last_dir=%zu\n", offsetof(MixaAppFmPanel, last_dir));
    printf("MixaAppFmPanel has_last_dir=%zu\n", offsetof(MixaAppFmPanel, has_last_dir));
    printf("MixaAppFmPanel drag_active=%zu\n", offsetof(MixaAppFmPanel, drag_active));
    printf("MixaAppFmPanel drag_grab_offset=%zu\n", offsetof(MixaAppFmPanel, drag_grab_offset));
    printf("MixaAppFmPanel focus=%zu\n", offsetof(MixaAppFmPanel, focus));
    return 0;
}
