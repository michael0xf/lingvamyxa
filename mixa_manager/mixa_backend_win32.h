/* mixa_backend_win32.h - concrete MixaBackend for the Win32 build.
 *
 * Completes the opaque struct from mixa_backend.h. Windows only.
 * Test-only: mixa_backend_frame_at (pixel readback of the DIB).
 */
#ifndef MIXA_BACKEND_WIN32_H
#define MIXA_BACKEND_WIN32_H

#include <windows.h>
#include "mixa_manager/mixa_backend.h"

struct MixaBackend {
    int is_open;
    HWND hwnd;
    HDC hdc_mem;
    HBITMAP hbmp;
    HBITMAP hbmp_old;
    MixaU8 *bits; /* DIB section bits, BGRA32 top-down; owned by hbmp */
    size_t cols;
    size_t rows;
    size_t cell_width;
    size_t cell_height;
    size_t frame_w;
    size_t frame_h;
    size_t frame_bytes;
};

/* Win32-only test helper: copy one RGBA pixel of the last presented frame. */
int mixa_backend_frame_at(const MixaBackend *backend, size_t x, size_t y, MixaU8 *out);

#endif