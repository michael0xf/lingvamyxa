/* ABI parity guard (ticket 20260913-154000). Prints sizeof and every
 * field offset of the REAL MixaButton/MixaButtonLine/MixaButtonPanel
 * structs, as seen through the genuine production header (mixa_
 * manager/mixa_buttons.h, a plain, hand-written C header). The runner
 * compiles this AND mixa_buttons_abi_probe_l2.c (which probes the L2
 * predef'd header instead) and diffs their output byte-for-byte before
 * ever building or running the actual parity harness.
 */
#include <stdio.h>
#include <stddef.h>
#include "mixa_manager/mixa_buttons.h"

int main(void) {
    printf("MixaButton sizeof=%zu\n", sizeof(MixaButton));
    printf("MixaButton label=%zu\n", offsetof(MixaButton, label));
    printf("MixaButton row=%zu\n", offsetof(MixaButton, row));
    printf("MixaButton col=%zu\n", offsetof(MixaButton, col));
    printf("MixaButton nrows=%zu\n", offsetof(MixaButton, nrows));
    printf("MixaButton ncols=%zu\n", offsetof(MixaButton, ncols));
    printf("MixaButton fg=%zu\n", offsetof(MixaButton, fg));
    printf("MixaButton bg=%zu\n", offsetof(MixaButton, bg));
    printf("MixaButton alpha=%zu\n", offsetof(MixaButton, alpha));
    printf("MixaButton is_sep=%zu\n", offsetof(MixaButton, is_sep));
    printf("MixaButton enlarge=%zu\n", offsetof(MixaButton, enlarge));

    printf("MixaButtonLine sizeof=%zu\n", sizeof(MixaButtonLine));
    printf("MixaButtonLine items=%zu\n", offsetof(MixaButtonLine, items));
    printf("MixaButtonLine count=%zu\n", offsetof(MixaButtonLine, count));
    printf("MixaButtonLine cap=%zu\n", offsetof(MixaButtonLine, cap));

    printf("MixaButtonPanel sizeof=%zu\n", sizeof(MixaButtonPanel));
    printf("MixaButtonPanel lines=%zu\n", offsetof(MixaButtonPanel, lines));
    printf("MixaButtonPanel line_count=%zu\n", offsetof(MixaButtonPanel, line_count));
    printf("MixaButtonPanel line_cap=%zu\n", offsetof(MixaButtonPanel, line_cap));
    printf("MixaButtonPanel hb=%zu\n", offsetof(MixaButtonPanel, hb));
    printf("MixaButtonPanel wb=%zu\n", offsetof(MixaButtonPanel, wb));
    printf("MixaButtonPanel margin=%zu\n", offsetof(MixaButtonPanel, margin));
    printf("MixaButtonPanel panel_w=%zu\n", offsetof(MixaButtonPanel, panel_w));
    printf("MixaButtonPanel panel_h=%zu\n", offsetof(MixaButtonPanel, panel_h));
    printf("MixaButtonPanel origin_row=%zu\n", offsetof(MixaButtonPanel, origin_row));
    printf("MixaButtonPanel origin_col=%zu\n", offsetof(MixaButtonPanel, origin_col));
    printf("MixaButtonPanel view_w=%zu\n", offsetof(MixaButtonPanel, view_w));
    printf("MixaButtonPanel scroll_x=%zu\n", offsetof(MixaButtonPanel, scroll_x));
    printf("MixaButtonPanel is_open=%zu\n", offsetof(MixaButtonPanel, is_open));
    return 0;
}
