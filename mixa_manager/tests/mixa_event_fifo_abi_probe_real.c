/* ABI parity guard (ticket 20260913-090200): prints sizeof and every
 * field offset of the REAL MixaEvent/MixaEventFifo structs, as seen
 * through the genuine production headers. The runner compiles this
 * AND mixa_event_fifo_abi_probe_l2.c (which probes the L2 predef'd
 * header instead) and diffs their output byte-for-byte before ever
 * building or running the actual parity harness -- a copied struct
 * declaration that silently drifts from the real one must fail loudly
 * here, not surface as garbage field values deep in a scenario trace.
 */
#include <stdio.h>
#include <stddef.h>
#include "mixa_manager/mixa_backend.h"
#include "mixa_manager/mixa_event_fifo.h"

int main(void) {
    printf("MixaEvent sizeof=%zu\n", sizeof(MixaEvent));
    printf("MixaEvent kind=%zu\n", offsetof(MixaEvent, kind));
    printf("MixaEvent codepoint=%zu\n", offsetof(MixaEvent, codepoint));
    printf("MixaEvent keycode=%zu\n", offsetof(MixaEvent, keycode));
    printf("MixaEvent modifiers=%zu\n", offsetof(MixaEvent, modifiers));
    printf("MixaEvent row=%zu\n", offsetof(MixaEvent, row));
    printf("MixaEvent col=%zu\n", offsetof(MixaEvent, col));
    printf("MixaEvent buttons=%zu\n", offsetof(MixaEvent, buttons));
    printf("MixaEvent cols=%zu\n", offsetof(MixaEvent, cols));
    printf("MixaEvent rows=%zu\n", offsetof(MixaEvent, rows));
    printf("MixaEvent wheel_delta=%zu\n", offsetof(MixaEvent, wheel_delta));
    printf("MixaEventFifo sizeof=%zu\n", sizeof(MixaEventFifo));
    printf("MixaEventFifo items=%zu\n", offsetof(MixaEventFifo, items));
    printf("MixaEventFifo capacity=%zu\n", offsetof(MixaEventFifo, capacity));
    printf("MixaEventFifo head=%zu\n", offsetof(MixaEventFifo, head));
    printf("MixaEventFifo count=%zu\n", offsetof(MixaEventFifo, count));
    printf("MixaEventFifo is_open=%zu\n", offsetof(MixaEventFifo, is_open));
    return 0;
}
