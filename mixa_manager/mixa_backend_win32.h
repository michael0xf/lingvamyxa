/* mixa_backend_win32.h - concrete MixaBackend for the Win32 build.
 *
 * Completes the opaque struct from mixa_backend.h. Windows only.
 * Test-only: mixa_backend_frame_at (pixel readback of the DIB).
 */
#ifndef MIXA_BACKEND_WIN32_H
#define MIXA_BACKEND_WIN32_H

#include <windows.h>
#include "mixa_manager/mixa_backend.h"

/* Per-backend glyph cache entry: keyed by (codepoint, layer). */
typedef struct MixaWin32GlyphEntry {
    unsigned int codepoint;
    int layer;
    int missing;
    size_t width;
    size_t height;
    int bearing_x;
    int bearing_y;
    size_t advance;
    MixaU8 *coverage; /* owned; NULL when missing */
    struct MixaWin32GlyphEntry *next;
} MixaWin32GlyphEntry;

struct MixaBackend {
    int is_open;
    HWND hwnd;
    HDC hdc_mem;
    HBITMAP hbmp;
    HBITMAP hbmp_old;
    MixaU8 *bits; /* DIB section bits, BGRA32 top-down; owned by hbmp */
    size_t cols;
    size_t rows;
    size_t cell_width;       /* text-layer cell; drives frame + mouse/resize */
    size_t cell_height;
    size_t upper_cell_width;
    size_t upper_cell_height;
    size_t frame_w;
    size_t frame_h;
    size_t frame_bytes;
    HFONT hfont_text;
    HFONT hfont_upper;
    HDC hdc_glyph;           /* memory DC for GetGlyphOutline / metrics */
    int text_ascent;         /* tmAscent for text font (placement) */
    int upper_ascent;
    MixaWin32GlyphEntry *glyph_cache;
};

/* Win32-only test helper: copy one RGBA pixel of the last presented frame. */
int mixa_backend_frame_at(const MixaBackend *backend, size_t x, size_t y, MixaU8 *out);

#endif
