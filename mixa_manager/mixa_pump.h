/* mixa_pump.h - drain backend poll into a FIFO inbox, then process.
 *
 * BACKEND_SEAM 7.1 groundwork: never act on an event where it is polled.
 * One lane, FIFO order. No scheduler / priorities / blocking wait.
 * No staged outbox in this unit (later ticket).
 */
#ifndef MIXA_PUMP_H
#define MIXA_PUMP_H

#include <stddef.h>
#include "mixa_manager/mixa_backend.h"
#include "mixa_manager/mixa_event_fifo.h"

typedef struct MixaPump {
    MixaEventFifo inbox;
    int is_open;
} MixaPump;

int mixa_pump_open(MixaPump *pump);
void mixa_pump_release(MixaPump *pump);

/* Drain mixa_backend_poll until MIXA_EVENT_NONE into the inbox.
 * Returns 0 on success, non-zero on failure. How many were taken is
 * mixa_pump_pending after the call. On RESIZE adoption failure from poll,
 * the event is still pushed and counted, and drain returns failure. */
int mixa_pump_drain(MixaPump *pump, MixaBackend *backend);

int mixa_pump_next(MixaPump *pump, MixaEvent *out);
size_t mixa_pump_pending(const MixaPump *pump);

#endif
