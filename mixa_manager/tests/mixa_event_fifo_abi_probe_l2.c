/* ABI parity guard (ticket 20260913-090200): prints sizeof and every
 * field offset of MixaEvent/MixaEventFifo as seen through the L2 header
 * unit (mixa_event_fifo_l2.h.lm1, translated to
 * mixa_event_fifo_l2.lm1.h by the runner before this file is compiled).
 * Must be byte-identical to mixa_event_fifo_abi_probe_real.c's own
 * output -- see that file's own comment for why this pair exists.
 */
#include <stdio.h>
#include <stddef.h>
#include "mixa_manager/mixa_event_fifo_l2.lm1.h"

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
