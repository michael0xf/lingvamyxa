/* mixa_backend_headless.h - concrete headless backend; vt is first member.
 *
 * Test helpers are not part of the seam. Constructor returns a singleton table.
 */
#ifndef MIXA_BACKEND_HEADLESS_H
#define MIXA_BACKEND_HEADLESS_H

#include "mixa_manager/mixa_backend.h"
#include "mixa_manager/mixa_event_fifo.h"

typedef struct MixaHeadless {
    const MixaBackendVTable *vt; /* MUST be first - same layout as MixaBackend */
    int is_open;
    size_t cols;
    size_t rows;
    size_t cell_width;
    size_t cell_height;
    size_t frame_w;
    size_t frame_h;
    size_t frame_bytes;
    MixaU8 *frame;
    char *clipboard;
    size_t clipboard_len; /* bytes, excluding NUL */
    /* Heap fifo so L1 can pass it without taking & of an embedded field.
     * Allocated and inited in open; released in close. */
    MixaEventFifo *inbox;
} MixaHeadless;

const MixaBackendVTable *mixa_backend_headless_table(void);

/* Headless-only: append a synthetic event. For MIXA_EVENT_MOUSE, incoming
 * col/row are PIXEL coordinates and are converted to HALF-cells of the text
 * layer here (BACKEND_SEAM 10.5). */
int mixa_headless_push_event(MixaBackend *backend, const MixaEvent *ev);

/* Headless-only: copy one RGBA pixel of the last presented frame into out[4]. */
int mixa_headless_frame_at(const MixaBackend *backend, size_t x, size_t y, MixaU8 *out);

#endif