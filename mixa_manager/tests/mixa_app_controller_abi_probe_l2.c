/* ABI parity guard (ticket 20260913-151000). Prints sizeof and every
 * field offset of MixaAppController as seen through the L2 header's
 * own l1trans-generated C header (mixa_app_controller_l2.h.lm1 ->
 * mixa_app_controller_l2.lm1.h). Compiled and run alongside mixa_app_
 * controller_abi_probe_real.c; the runner diffs their output byte-for-
 * byte before ever building or running the actual parity harness.
 */
#include <stdio.h>
#include <stddef.h>
#include "mixa_manager/mixa_app_controller_l2.lm1.h"

int main(void) {
    printf("MixaAppController sizeof=%zu\n", sizeof(MixaAppController));
    printf("MixaAppController root=%zu\n", offsetof(MixaAppController, root));
    printf("MixaAppController loop=%zu\n", offsetof(MixaAppController, loop));
    printf("MixaAppController m=%zu\n", offsetof(MixaAppController, m));
    printf("MixaAppController ev=%zu\n", offsetof(MixaAppController, ev));
    printf("MixaAppController av=%zu\n", offsetof(MixaAppController, av));
    printf("MixaAppController vt=%zu\n", offsetof(MixaAppController, vt));
    printf("MixaAppController console_path=%zu\n", offsetof(MixaAppController, console_path));
    printf("MixaAppController console_file=%zu\n", offsetof(MixaAppController, console_file));
    printf("MixaAppController console_file_append=%zu\n", offsetof(MixaAppController, console_file_append));
    printf("MixaAppController view=%zu\n", offsetof(MixaAppController, view));
    printf("MixaAppController text_rect=%zu\n", offsetof(MixaAppController, text_rect));
    printf("MixaAppController upper_rect=%zu\n", offsetof(MixaAppController, upper_rect));
    printf("MixaAppController rgba=%zu\n", offsetof(MixaAppController, rgba));
    printf("MixaAppController width_px=%zu\n", offsetof(MixaAppController, width_px));
    printf("MixaAppController height_px=%zu\n", offsetof(MixaAppController, height_px));
    printf("MixaAppController rgba_size=%zu\n", offsetof(MixaAppController, rgba_size));
    printf("MixaAppController help_ctx=%zu\n", offsetof(MixaAppController, help_ctx));
    printf("MixaAppController help_btn_row=%zu\n", offsetof(MixaAppController, help_btn_row));
    printf("MixaAppController help_btn_col=%zu\n", offsetof(MixaAppController, help_btn_col));
    printf("MixaAppController help_btn_nrows=%zu\n", offsetof(MixaAppController, help_btn_nrows));
    printf("MixaAppController help_btn_ncols=%zu\n", offsetof(MixaAppController, help_btn_ncols));
    printf("MixaAppController cmdline=%zu\n", offsetof(MixaAppController, cmdline));
    printf("MixaAppController dispatch=%zu\n", offsetof(MixaAppController, dispatch));
    printf("MixaAppController cwd_owned=%zu\n", offsetof(MixaAppController, cwd_owned));
    printf("MixaAppController cwd_cur=%zu\n", offsetof(MixaAppController, cwd_cur));
    printf("MixaAppController pending_view=%zu\n", offsetof(MixaAppController, pending_view));
    printf("MixaAppController last_exit_code=%zu\n", offsetof(MixaAppController, last_exit_code));
    printf("MixaAppController fmpanel=%zu\n", offsetof(MixaAppController, fmpanel));
    printf("MixaAppController copy_sink=%zu\n", offsetof(MixaAppController, copy_sink));
    return 0;
}
