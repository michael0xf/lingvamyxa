/* ABI parity guard (ticket 20260913-093000): prints sizeof and every
 * field offset of MixaCell/MixaTextRect as seen through the L2 header
 * unit (mixa_tiles_l2.h.lm1, translated to mixa_tiles_l2.lm1.h by the
 * runner before this file is compiled). Must be byte-identical to
 * mixa_tiles_abi_probe_real.c's own output -- see that file's own
 * comment for why this pair exists, and mixa_tiles_l2.h.lm1's own
 * comment for why fg/bg/flags/width/alpha are declared as 1-element
 * char arrays here rather than scalar unsigned char fields (this probe
 * is exactly what proves that substitution is still byte/offset-exact).
 */
#include <stdio.h>
#include <stddef.h>
#include "mixa_manager/mixa_tiles_l2.lm1.h"

int main(void) {
    printf("MixaCell sizeof=%zu\n", sizeof(MixaCell));
    printf("MixaCell codepoint=%zu\n", offsetof(MixaCell, codepoint));
    printf("MixaCell fg=%zu\n", offsetof(MixaCell, fg));
    printf("MixaCell bg=%zu\n", offsetof(MixaCell, bg));
    printf("MixaCell flags=%zu\n", offsetof(MixaCell, flags));
    printf("MixaCell width=%zu\n", offsetof(MixaCell, width));
    printf("MixaCell alpha=%zu\n", offsetof(MixaCell, alpha));
    printf("MixaTextRect sizeof=%zu\n", sizeof(MixaTextRect));
    printf("MixaTextRect rows=%zu\n", offsetof(MixaTextRect, rows));
    printf("MixaTextRect cols=%zu\n", offsetof(MixaTextRect, cols));
    printf("MixaTextRect cells=%zu\n", offsetof(MixaTextRect, cells));
    return 0;
}
