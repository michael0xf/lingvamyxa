/* ABI parity guard (ticket 20260913-115500). Prints sizeof and every
 * field offset of the REAL MixaProcessMarkerFaultVTable/Gen/Sink/
 * Scanner structs, as seen through the real production header (mixa_
 * process_marker.h.lm1, translated to mixa_process_marker_real.lm1.h
 * by the runner). There is no separate plain-C production header for
 * this module (unlike mixa_tiles/mixa_file_win32) -- both this probe
 * and mixa_process_marker_abi_probe_l2.c compile against l1trans-
 * generated output, one from the real header, one from the L2 header,
 * and the runner diffs their output byte-for-byte before ever building
 * or running the actual parity harness.
 */
#include <stdio.h>
#include <stddef.h>
#include "mixa_manager/mixa_process_marker.lm1.h"

int main(void) {
    printf("MixaProcessMarkerFaultVTable sizeof=%zu\n", sizeof(MixaProcessMarkerFaultVTable));
    printf("MixaProcessMarkerFaultVTable on_alloc=%zu\n", offsetof(MixaProcessMarkerFaultVTable, on_alloc));
    printf("MixaProcessMarkerFaultVTable on_collide=%zu\n", offsetof(MixaProcessMarkerFaultVTable, on_collide));
    printf("MixaProcessMarkerFaultVTable ctx=%zu\n", offsetof(MixaProcessMarkerFaultVTable, ctx));

    printf("MixaProcessMarkerGen sizeof=%zu\n", sizeof(MixaProcessMarkerGen));
    printf("MixaProcessMarkerGen counter=%zu\n", offsetof(MixaProcessMarkerGen, counter));

    printf("MixaProcessMarkerSink sizeof=%zu\n", sizeof(MixaProcessMarkerSink));
    printf("MixaProcessMarkerSink on_ordinary=%zu\n", offsetof(MixaProcessMarkerSink, on_ordinary));
    printf("MixaProcessMarkerSink ctx=%zu\n", offsetof(MixaProcessMarkerSink, ctx));

    printf("MixaProcessMarkerScanner sizeof=%zu\n", sizeof(MixaProcessMarkerScanner));
    printf("MixaProcessMarkerScanner marker=%zu\n", offsetof(MixaProcessMarkerScanner, marker));
    printf("MixaProcessMarkerScanner marker_len=%zu\n", offsetof(MixaProcessMarkerScanner, marker_len));
    printf("MixaProcessMarkerScanner state=%zu\n", offsetof(MixaProcessMarkerScanner, state));
    printf("MixaProcessMarkerScanner pending=%zu\n", offsetof(MixaProcessMarkerScanner, pending));
    printf("MixaProcessMarkerScanner pending_len=%zu\n", offsetof(MixaProcessMarkerScanner, pending_len));
    printf("MixaProcessMarkerScanner pending_cap=%zu\n", offsetof(MixaProcessMarkerScanner, pending_cap));
    printf("MixaProcessMarkerScanner status_start=%zu\n", offsetof(MixaProcessMarkerScanner, status_start));
    printf("MixaProcessMarkerScanner tab2_pos=%zu\n", offsetof(MixaProcessMarkerScanner, tab2_pos));
    printf("MixaProcessMarkerScanner exit_code=%zu\n", offsetof(MixaProcessMarkerScanner, exit_code));
    printf("MixaProcessMarkerScanner cwd=%zu\n", offsetof(MixaProcessMarkerScanner, cwd));
    printf("MixaProcessMarkerScanner cwd_len=%zu\n", offsetof(MixaProcessMarkerScanner, cwd_len));
    printf("MixaProcessMarkerScanner at_line_start=%zu\n", offsetof(MixaProcessMarkerScanner, at_line_start));
    printf("MixaProcessMarkerScanner fault=%zu\n", offsetof(MixaProcessMarkerScanner, fault));
    return 0;
}
