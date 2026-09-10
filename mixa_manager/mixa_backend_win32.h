/* mixa_backend_win32.h - concrete Win32 backend; vt is first member.
 *
 * Test helper mixa_win32_frame_at is not part of the seam.
 * Constructor returns a singleton table.
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

typedef struct MixaWin32 {
    const MixaBackendVTable *vt; /* MUST be first - same layout as MixaBackend */
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
    size_t pointer_cell_width;
    size_t pointer_cell_height;
    size_t frame_w;
    size_t frame_h;
    size_t frame_bytes;
    HFONT hfont_text;
    HFONT hfont_upper;
    HFONT hfont_pointer;     /* third font size for the mouse glyph; not a grid */
    HDC hdc_glyph;           /* memory DC for GetGlyphOutline / metrics */
    int text_ascent;         /* tmAscent for text font (placement) */
    int upper_ascent;
    int pointer_ascent;
    MixaWin32GlyphEntry *glyph_cache;
} MixaWin32;

const MixaBackendVTable *mixa_backend_win32_table(void);

/* Win32-only test helper: copy one RGBA pixel of the last presented frame. */
int mixa_win32_frame_at(const MixaBackend *backend, size_t x, size_t y, MixaU8 *out);

#endif