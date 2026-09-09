/* mixa_backend_headless.h - concrete MixaBackend for the headless build.
 *
 * Completes the opaque struct from mixa_backend.h and declares the two
 * test-only entry points that are not part of the seam.
 */
#ifndef MIXA_BACKEND_HEADLESS_H
#define MIXA_BACKEND_HEADLESS_H

#include "mixa_manager/mixa_backend.h"

struct MixaBackend {
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
    MixaEvent *queue;
    size_t queue_cap;
    size_t queue_head;
    size_t queue_count;
};

/* Headless-only: append a synthetic event. For MIXA_EVENT_MOUSE, incoming
 * col/row are PIXEL coordinates and are converted to cells here. */
int mixa_backend_push_event(MixaBackend *backend, const MixaEvent *ev);

/* Headless-only: copy one RGBA pixel of the last presented frame into out[4]. */
int mixa_backend_frame_at(MixaBackend *backend, size_t x, size_t y, MixaU8 *out);

#endif
