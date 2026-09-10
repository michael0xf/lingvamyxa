#ifndef MIXA_TILES_H
#define MIXA_TILES_H

#include <stddef.h>
#include "mixa_manager/mixa_core.h"
#include "mixa_manager/mixa_overlay.h"

/* Nine-slice button tiles at private-use U+E000..U+E00F (DRAWING.txt section 5).
 * Low 4 bits are gap flags: left/right/top/bottom. Compositor recognises the
 * range and paints procedurally; colour from cell bg + alpha; gaps stay
 * transparent. No MixaCell field added.
 *
 * Classic nine-slice uses nine of the sixteen flag combos; 1x1 and 1-row /
 * 1-col buttons need the other combos (notably all-four gaps for 1x1).
 */

#define MIXA_TILE_BASE   0xE000U
#define MIXA_TILE_LAST   0xE00FU

#define MIXA_TILE_GAP_L  0x1U
#define MIXA_TILE_GAP_R  0x2U
#define MIXA_TILE_GAP_T  0x4U
#define MIXA_TILE_GAP_B  0x8U

/* Named classic nine (and the 1x1 all-gap tile). */
#define MIXA_TILE_FILL   (MIXA_TILE_BASE)
#define MIXA_TILE_W      (MIXA_TILE_BASE | MIXA_TILE_GAP_L)
#define MIXA_TILE_E      (MIXA_TILE_BASE | MIXA_TILE_GAP_R)
#define MIXA_TILE_N      (MIXA_TILE_BASE | MIXA_TILE_GAP_T)
#define MIXA_TILE_S      (MIXA_TILE_BASE | MIXA_TILE_GAP_B)
#define MIXA_TILE_NW     (MIXA_TILE_BASE | MIXA_TILE_GAP_L | MIXA_TILE_GAP_T)
#define MIXA_TILE_NE     (MIXA_TILE_BASE | MIXA_TILE_GAP_R | MIXA_TILE_GAP_T)
#define MIXA_TILE_SW     (MIXA_TILE_BASE | MIXA_TILE_GAP_L | MIXA_TILE_GAP_B)
#define MIXA_TILE_SE     (MIXA_TILE_BASE | MIXA_TILE_GAP_R | MIXA_TILE_GAP_B)
#define MIXA_TILE_ALL    (MIXA_TILE_BASE | MIXA_TILE_GAP_L | MIXA_TILE_GAP_R | MIXA_TILE_GAP_T | MIXA_TILE_GAP_B)

/* Gap in pixels: cell_h >> 5, floored at 1. */
size_t mixa_tile_gap_px(size_t cell_h);

/* Non-zero if codepoint is in the private-use tile range. */
int mixa_tile_is(unsigned codepoint);

/* Gap flags from a tile codepoint (0 if not a tile). */
unsigned mixa_tile_gaps(unsigned codepoint);

/* Build a tile codepoint from gap flags (flags masked to 4 bits). */
unsigned mixa_tile_from_gaps(unsigned gaps);

/* 1 if local pixel (lx,ly) inside cell is SOLID for this tile; 0 if gap. */
int mixa_tile_pixel_solid(unsigned codepoint, size_t lx, size_t ly,
                          size_t cell_w, size_t cell_h);

/* Paint one cell's tile into an RGBA buffer. Solid pixels: palette[bg] with
 * cell alpha (src-over). Gap pixels untouched. Returns 0 ok, 1 err.
 */
int mixa_tile_paint_cell(unsigned codepoint, unsigned bg, unsigned alpha,
                         size_t cell_x, size_t cell_y, size_t cell_w, size_t cell_h,
                         MixaU8 *out_rgba, size_t width, size_t height);

/* Place nine-slice (or degenerate) tiles into a rect of cells. */
int mixa_tile_fill_button(MixaTextRect *rect, size_t row, size_t col,
                          size_t nrows, size_t ncols,
                          unsigned fg, unsigned bg, unsigned alpha);

#endif
