/* ABI parity guard (ticket 20260913-152000). Prints sizeof and every
 * field offset of MixaAppPanelEntry/MixaAppPanel as seen through the
 * REAL header's own l1trans-generated C header (mixa_app_panel.h.lm1
 * -> mixa_app_panel.lm1.h). Unlike most modules ported this segment,
 * mixa_app_panel has no genuine hand-written plain-C header at all --
 * both its real and L2 headers are L1 .h.lm1 units, so "real" here
 * means the SAME stable l1trans translator applied to the real,
 * unmodified source instead of the L2 port's own header. The runner
 * compiles this AND mixa_app_panel_abi_probe_l2.c (which probes the L2
 * predef'd header's own generated C header instead) and diffs their
 * output byte-for-byte before ever building or running the actual
 * parity harness.
 */
#include <stdio.h>
#include <stddef.h>
#include "mixa_manager/mixa_app_panel.lm1.h"

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
