#ifndef MIXA_HIGHLIGHT_H
#define MIXA_HIGHLIGHT_H

#include <stddef.h>
#include "mixa_manager/mixa_core.h"
#include "mixa_manager/mixa_draw.h"

/* One moving double-box focus frame (DRAWING.txt 5.4 / PORT_OF_CLEARSHELL 2.8).
 * Overlays outermost cells; restores them when focus moves or clears.
 *
 * Lifetime / redraw contract while active:
 *   - The target MixaTextRect's cell storage must stay live and unchanged in
 *     identity (same cells pointer) and geometry (rows/cols).
 *   - Clear (or move on the same rect) before repainting underlying perimeter
 *     content, or before destroying/resizing the rectangle. Otherwise a later
 *     restore can overwrite newer content, or a mismatched target is rejected.
 *   - release() frees bookkeeping only; it does not restore cells. Call clear
 *     on the bound target first if a visible overlay must be removed.
 */

#define MIXA_HIGHLIGHT_OK  0
#define MIXA_HIGHLIGHT_ERR 1

typedef struct MixaHighlight {
    int is_open;
    int is_active;
    size_t row;
    size_t col;
    size_t nrows;
    size_t ncols;
    unsigned fg;
    MixaCell *saved;
    size_t saved_count;
    size_t saved_cap;
    /* Bound while active: reject restore/move if target identity drifts. */
    MixaTextRect *target_rect;
    MixaCell *target_cells;
    size_t target_rows;
    size_t target_cols;
} MixaHighlight;

int mixa_highlight_init(MixaHighlight *hl);
void mixa_highlight_release(MixaHighlight *hl);

/* Place or move the double frame on the same bound target. Restores any
 * previous perimeter first (same rect only). Uses MIXA_DRAW_KEEP_BG so
 * element backgrounds stay. fg is monochrome palette.
 * Rejects if an active snapshot is bound to a different rect/storage/geometry.
 */
int mixa_highlight_set(MixaHighlight *hl, MixaTextRect *rect,
                       size_t row, size_t col, size_t nrows, size_t ncols,
                       unsigned fg);

/* Restore saved perimeter cells and clear active state.
 * Rejects (leaves active intact) if rect is not the bound target or its
 * storage/geometry changed, or if restore bounds cannot be preflighted.
 */
int mixa_highlight_clear(MixaHighlight *hl, MixaTextRect *rect);

#endif