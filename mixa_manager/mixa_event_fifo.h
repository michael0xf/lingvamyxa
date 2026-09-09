/* mixa_event_fifo.h - concrete growable FIFO of MixaEvent.
 *
 * One lane, FIFO order. Growth: seed 8, double; all three overflow guards
 * (addition, doubling, sizeof product) as in l1src/own.lm1. Replaces the
 * hand-rolled ring previously inside the headless backend.
 */
#ifndef MIXA_EVENT_FIFO_H
#define MIXA_EVENT_FIFO_H

#include <stddef.h>
#include "mixa_manager/mixa_backend.h"   /* MixaEvent */

typedef struct MixaEventFifo {
    MixaEvent *items;
    size_t capacity;
    size_t head;
    size_t count;
    int is_open;
} MixaEventFifo;

int mixa_event_fifo_init(MixaEventFifo *fifo);
void mixa_event_fifo_release(MixaEventFifo *fifo);
int mixa_event_fifo_push(MixaEventFifo *fifo, const MixaEvent *ev);
int mixa_event_fifo_pop(MixaEventFifo *fifo, MixaEvent *out);
size_t mixa_event_fifo_count(const MixaEventFifo *fifo);

#endif