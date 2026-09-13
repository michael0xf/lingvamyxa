/* ABI parity guard (ticket 20260913-152000). Prints sizeof and every
 * field offset of MixaAppPanelEntry/MixaAppPanel as seen through the L2
 * header's own l1trans-generated C header (mixa_app_panel_l2.h.lm1 ->
 * mixa_app_panel_l2.lm1.h). Compiled and run alongside mixa_app_panel_
 * abi_probe_real.c; the runner diffs their output byte-for-byte before
 * ever building or running the actual parity harness.
 */
#include <stdio.h>
#include <stddef.h>
#include "mixa_manager/mixa_app_panel_l2.lm1.h"

int main(void) {
    printf("MixaAppPanelEntry sizeof=%zu\n", sizeof(MixaAppPanelEntry));
    printf("MixaAppPanelEntry kind=%zu\n", offsetof(MixaAppPanelEntry, kind));
    printf("MixaAppPanelEntry label=%zu\n", offsetof(MixaAppPanelEntry, label));
    printf("MixaAppPanelEntry launch_ref=%zu\n", offsetof(MixaAppPanelEntry, launch_ref));
    printf("MixaAppPanelEntry path=%zu\n", offsetof(MixaAppPanelEntry, path));

    printf("MixaAppPanel sizeof=%zu\n", sizeof(MixaAppPanel));
    printf("MixaAppPanel entries=%zu\n", offsetof(MixaAppPanel, entries));
    printf("MixaAppPanel entry_count=%zu\n", offsetof(MixaAppPanel, entry_count));
    printf("MixaAppPanel entry_cap=%zu\n", offsetof(MixaAppPanel, entry_cap));
    printf("MixaAppPanel panel=%zu\n", offsetof(MixaAppPanel, panel));
    printf("MixaAppPanel is_open=%zu\n", offsetof(MixaAppPanel, is_open));
    return 0;
}
