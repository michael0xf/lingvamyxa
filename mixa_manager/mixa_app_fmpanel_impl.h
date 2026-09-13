#ifndef MIXA_APP_FMPANEL_IMPL_H
#define MIXA_APP_FMPANEL_IMPL_H

/* Concrete MixaAppFmPanel; opaque to callers of mixa_app_fmpanel.h.
 * Included ONLY by mixa_app_fmpanel.lm1 -- the same opaque-handle split
 * mixa_app_controller.h/_impl.h already establish.
 *
 * struct MixaSelection is referenced by STRUCT TAG ONLY (mixa_selection.h
 * is a plain C header with the real, complete definition already
 * available wherever it is include:'d -- referencing it here by tag only
 * keeps this header from needing to include it itself).
 */

#include "mixa_manager/mixa_app_fmpanel.h"
#include "mixa_manager/mixa_highlight.h"

#define MIXA_APP_FMPANEL_PATH_MAX 1024

struct MixaAppFmPanel {
    MixaFm *fm;
    size_t list_row;
    size_t list_col;
    size_t list_rows;
    size_t list_cols;
    size_t action_row;
    size_t action_col;

    /* Raw, buttons-free confirmation surface (see mixa_app_fmpanel.h's
     * own comment for why this does not reuse mixa_app_window). */
    int confirm_open;
    size_t confirm_row;
    size_t confirm_col;
    size_t confirm_rows;
    size_t confirm_cols;
    size_t cancel_row;
    size_t cancel_col;
    size_t cancel_w;
    size_t ok_row;
    size_t ok_col;
    size_t ok_w;
    MixaCell *saved;
    size_t saved_count;
    size_t saved_row;
    size_t saved_col;
    size_t saved_rows;
    size_t saved_cols;

    int invocations;
    int has_failure;
    int last_status;
    char failing_path[MIXA_APP_FMPANEL_PATH_MAX];

    /* Copy Here's own visible result (ticket 20260913-041656) -- the
     * SAME MixaFmCopyHereCtx.last_status the accepted action already
     * sets, never a second status channel. Persists until an explicit
     * dismiss click on the result text itself, or a later invocation
     * replaces it. */
    int has_copy_result;
    int copy_last_status;

    /* Vertical scrolling + keyboard highlight (ticket 20260913-044900).
     * view_top is the index of the first entry currently drawn in the
     * list area; highlight_idx is the keyboard-moved current entry, -1
     * meaning no keyboard highlight has been established yet (mouse-only
     * selection, from earlier tickets, still works with no highlight at
     * all). last_dir/has_last_dir detect a directory change between
     * renders so the view resets to the top of the NEW listing instead
     * of keeping an old scroll position that no longer means anything --
     * see mixa_app_fmpanel_clamp_view's own comment in the .lm1. */
    size_t view_top;
    int highlight_idx;
    char last_dir[MIXA_APP_FMPANEL_PATH_MAX];
    int has_last_dir;

    /* Scrollbar-thumb dragging (ticket 20260913-060000). drag_active is
     * nonzero from the tick the left button is pressed exactly on the
     * CURRENT thumb until the tick it is released (mixa_app_fmpanel_
     * release, called on every button-up mouse event, always clears it --
     * idempotent when no drag is active). drag_grab_offset is the row
     * within the thumb (0 = its own first/top row) where the press
     * landed, fixed for the whole drag so the thumb tracks the cursor at
     * that same relative row rather than snapping its top row under it --
     * requirement 2's own "does not jump when grabbed away from its
     * first row". Both are ordinary per-instance state, not globals. */
    int drag_active;
    size_t drag_grab_offset;

    /* Moving double-box keyboard focus frame (ticket 20260913-064500),
     * replacing the reverse-video stand-in Part IV disclosed. Embedded by
     * VALUE (MixaHighlight is a complete, plain-C struct, not opaque) so
     * open()/close() need no separate allocation for it -- mirrors how
     * this struct itself is embedded directly in the caller's own
     * MixaAppController rather than through a second pointer indirection.
     * See mixa_app_fmpanel.txt's own Part VII for why this needs a
     * dedicated blank "focus gutter" column pair rather than framing the
     * full entry row: mixa_draw_frame (which mixa_highlight_set calls)
     * refuses nrows<2, and a real list entry is exactly one text row
     * tall -- framing two real rows would overwrite a NEIGHBORING
     * entry's own text with border glyphs, which requirement 1's own "do
     * not... overwrite the underlying row fill" forbids. */
    MixaHighlight focus;
};

#endif
