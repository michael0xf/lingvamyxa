#ifndef MIXA_DRAW_H
#define MIXA_DRAW_H

#include "mixa_manager/mixa_core.h"

/* Single drawing API (DRAWING.txt). Draws into a MixaTextRect cell surface.
 * Does not know which layer the surface belongs to.
 */

#define MIXA_DRAW_OK  0
#define MIXA_DRAW_ERR 1

#define MIXA_DRAW_SINGLE 0
#define MIXA_DRAW_DOUBLE 1

#define MIXA_DRAW_HORIZ 0
#define MIXA_DRAW_VERT 1

/* UTF-8 text from (row,col) with attrs. Clips at edges. OOB start -> ERR. */
int mixa_draw_text(MixaTextRect *rect, size_t row, size_t col,
                   const char *text, unsigned fg, unsigned bg,
                   unsigned flags, unsigned alpha);

/* Fill rectangle of cells. Extents must lie in-bounds; else ERR. */
int mixa_draw_fill(MixaTextRect *rect, size_t row, size_t col,
                   size_t nrows, size_t ncols, unsigned codepoint,
                   unsigned fg, unsigned bg, unsigned flags, unsigned alpha);

/* Horizontal or vertical line, single or double. Merges box stubs. */
int mixa_draw_line(MixaTextRect *rect, size_t row, size_t col,
                   size_t len, int orient, int style,
                   unsigned fg, unsigned bg, unsigned flags, unsigned alpha);

/* Rectangle border, single or double. Merges box stubs. nrows/ncols >= 2. */
int mixa_draw_frame(MixaTextRect *rect, size_t row, size_t col,
                    size_t nrows, size_t ncols, int style,
                    unsigned fg, unsigned bg, unsigned flags, unsigned alpha);

#endif