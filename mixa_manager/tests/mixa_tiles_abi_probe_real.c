/* ABI parity guard (ticket 20260913-093000, matching the established
 * pattern from ticket 20260913-090200): prints sizeof and every field
 * offset of the REAL MixaCell/MixaTextRect structs, as seen through the
 * genuine production header (mixa_core.h). The runner compiles this AND
 * mixa_tiles_abi_probe_l2.c (which probes the L2 predef'd header
 * instead) and diffs their output byte-for-byte before ever building or
 * running the actual parity harness.
 */
#include <stdio.h>
#include <stddef.h>
#include "mixa_manager/mixa_core.h"

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
