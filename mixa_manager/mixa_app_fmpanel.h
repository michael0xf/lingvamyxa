#ifndef MIXA_APP_FMPANEL_H
#define MIXA_APP_FMPANEL_H

/* First visible file-manager panel in the production controller (ticket
 * 20260913-032000). Renders MixaFm's own owned snapshot (current
 * directory + entry listing) and a small action row exposing the
 * already-accepted Delete operation (mixa_remove_selected_run/mixa_
 * dir_remove, reused -- their WALK-GLUE re-implemented locally here, see
 * mixa_app_fmpanel.txt for why) behind a real, drawn, same-app Cancel/OK
 * confirmation.
 *
 * Deliberately does NOT reuse mixa_app_window.h.lm1/mixa_buttons.h: this
 * module's own translation unit also needs mixa_backend.h (for MixaEvent,
 * reached transitively the same way mixa_app_controller.lm1 already
 * does), and mixa_backend.h's own MixaButton (an enum: MIXA_BUTTON_LEFT/
 * MIDDLE/RIGHT for mouse buttons) collides on the bare type name with
 * mixa_buttons.h's OWN MixaButton (a struct: one UI button) -- confirmed
 * as a real, reproducible compile error by actually trying it (see mixa_
 * app_fmpanel.txt), not assumed. Every "button" here is therefore a raw
 * drawn cell region with a raw bounds-check hit-test, the exact same
 * style mixa_help's own F1 button and mixa_fm_copy_here_hit's own rect
 * check already establish for the identical reason category (avoiding a
 * predef/type-collision cost inside the production controller's own
 * translation unit) -- not a new pattern invented for this ticket.
 *
 * Deliberately does NOT predef mixa_remove.h.lm1/mixa_fm_remove.h.lm1/
 * mixa_remove_confirm.h.lm1 either: mixa_app_controller.lm1's own import
 * table is already documented at its ceiling (measured directly: adding
 * either header alone overflows it), and even from a FRESH file, predef-
 * ing the REAL mixa_remove.lm1/mixa_fm_remove.lm1 implementations would
 * double-define mixa_fm_..., mixa_selection_..., mixa_dir_... symbols against
 * mixa_app_controller.o's own already-real copies of the exact same
 * functions (reached via ITS OWN existing mixa_fm_copy.lm1 predef chain).
 * This module instead predefs ONLY the relevant HEADERS (mixa_file_
 * manager.h.lm1, mixa_selection_walk.h.lm1) for types/prototypes, and
 * reimplements the small (~40-line) walk-glue (on_file/on_dir_done,
 * mirroring mixa_remove.lm1's own logic) directly against those already-
 * real, already-linked symbols -- a deliberate, small, disclosed local
 * duplication to escape a confirmed toolchain constraint, not a
 * divergent reimplementation of the operation's own semantics.
 */

#include <stddef.h>
#include "mixa_manager/mixa_core.h"

typedef struct MixaFm MixaFm;
typedef struct MixaAppFmPanel MixaAppFmPanel;

#define MIXA_APP_FMPANEL_OK 0
#define MIXA_APP_FMPANEL_ERR_ARG 1
#define MIXA_APP_FMPANEL_ERR_NOMEM 2

/* list_row/col/rows/cols: where the path + entry listing draws (rows
 * clipped to whatever is available -- no cap on fm's own entry count,
 * only on how many VISIBLE rows this geometry offers, exactly like the
 * console view's own real-screen-row bound). action_row/col: where the
 * "Delete"/"Copy Here" labels draw, one row, both reached via the
 * SAME rect. */
int mixa_app_fmpanel_open(MixaAppFmPanel **out, MixaFm *fm,
                          size_t list_row, size_t list_col,
                          size_t list_rows, size_t list_cols,
                          size_t action_row, size_t action_col);
void mixa_app_fmpanel_close(MixaAppFmPanel *p);

/* Redraws the whole panel (path, entries, action labels, and -- if a
 * confirmation is currently open -- the confirmation surface on top of
 * it) into rect. Call every present() tick, same cadence as the pending-
 * line refresh. */
int mixa_app_fmpanel_render(MixaAppFmPanel *p, MixaTextRect *rect);

/* One popped mouse-click cell coordinate (SAME upper-layer cell space
 * mixa_help_hit already uses). Returns:
 *   0 -- not mine; the caller's other hit-tests (F1 Help, console)
 *        should still see it.
 *   1 -- consumed: the Delete label (opens the nested confirmation), or
 *        -- while a confirmation is open -- ANY cell (input routes to
 *        the top nested surface only while one is open, mirroring
 *        mixa_app_window's own established contract; Cancel/OK inside
 *        it are both handled here).
 *   2 -- the Copy Here label was hit. This module never touches
 *        MixaFmCopyHereCtx (owned elsewhere); the caller runs the
 *        already-accepted mixa_fm_copy_here_action itself on seeing
 *        this, the exact same action its own Ctrl+V path already
 *        invokes today. */
int mixa_app_fmpanel_hit(MixaAppFmPanel *p, MixaTextRect *rect,
                         size_t cell_row, size_t cell_col);

int mixa_app_fmpanel_confirm_is_open(const MixaAppFmPanel *p);

/* Copy Here's own visible result (ticket 20260913-041656). Call this
 * with the EXACT status mixa_fm_copy_here_action (via MixaFmCopyHereCtx.
 * last_status) already returned -- never a synthesized or guessed one --
 * immediately after EITHER real invocation route: the mouse route (hit()
 * returning 2, the caller then calls the action itself and reports its
 * result here) or the keyboard route (mixa_app_loop_step returning
 * MIXA_COPY_HERE_HIT_ACTION -- the same signal that already means
 * "Ctrl+V/V just ran the action this tick"). The result persists across
 * unrelated events (no other event touches it) until either a later
 * invocation replaces it or a dismiss click (hit() on the result text
 * itself, a region distinct from Delete/Copy Here/entry rows/F1 Help)
 * clears it. */
void mixa_app_fmpanel_report_copy_result(MixaAppFmPanel *p, int status);

/* Vertical scrolling + keyboard highlight (ticket 20260913-044900). The
 * list's own rightmost column is a real, drawn scrollbar (UI_MODEL 1/5.7.2:
 * "a single list -- vertically, in its own bounds and its own scrollbar"),
 * clickable to move the view alone (hit() handles this; no separate entry
 * point). Keyboard Up/Down move a highlighted CURRENT entry -- there is no
 * such notion until the first Up/Down, matching mouse click-to-select
 * (established, unrelated) staying entirely selection-based, never
 * highlight-based. The view follows the highlight (never the reverse):
 * moving it into view when it would otherwise scroll off, exactly UI_MODEL
 * 5.7.1's own asymmetric rule ("the VIEW may leave the highlight [via a
 * mouse-only move]... any HIGHLIGHT movement brings the view back").
 *
 * At the time this was written, a mouse WHEEL was not in mixa_backend.h's
 * own MixaEvent contract at all -- only MIXA_EVENT_MOUSE with buttons/row/
 * col existed, no wheel delta. Closed by ticket 20260913-053000: see mixa_
 * app_fmpanel_wheel below. */
int mixa_app_fmpanel_key(MixaAppFmPanel *p, int keycode);

/* A real mouse-wheel notch (ticket 20260913-053000, MIXA_EVENT_MOUSE_WHEEL
 * in mixa_backend.h) at half-cell-derived upper-cell (cell_row, cell_col).
 * Same UI_MODEL 5.7.1 asymmetric rule as the scrollbar-track click above:
 * moves the view ALONE, touching neither the keyboard highlight nor any
 * selection. notches is signed (positive = away from the user / "scroll
 * up", negative = toward the user / "scroll down", the caller's own
 * MixaEvent.wheel_delta passed through unchanged) and may be more than one
 * per call on a fast wheel turn -- one row of view movement per notch, no
 * OS-configurable multiplier, a deliberate, disclosed simplification.
 * Returns 0 (not consumed, coordinates fall outside this panel's own list
 * area -- the caller's other handlers should still see the event) or 1
 * (consumed). Never touches the confirmation surface: like every other hit
 * region, a wheel event arriving while the confirmation is open is not
 * consumed here at all (the confirmation itself has no scrollable content,
 * so there is nothing for this function to do while it is open). */
int mixa_app_fmpanel_wheel(MixaAppFmPanel *p, size_t cell_row, size_t cell_col,
                           int notches);

/* Scrollbar-thumb dragging (ticket 20260913-060000). Pressing the left
 * button (hit() sees this the same way it already sees a click: a mouse
 * event with the left bit set) exactly on the CURRENT thumb starts a drag
 * instead of the existing jump-to-position behavior a track click elsewhere
 * still gets (requirement 5's own "preserve existing track-click"); no
 * separate entry point exists for starting one, matching every other
 * gesture this module owns. Once active, EVERY subsequent hit() call (an
 * ordinary mouse-move-while-held, indistinguishable at this seam from a
 * repeated press -- see mixa_app_fmpanel.txt's own Part VI for why that is
 * fine) moves the view following the cursor row, honoring the grab offset
 * captured at the press so the thumb does not jump to align its own first
 * row under the cursor. The mapping is recomputed fresh from the CURRENT
 * real entry count on every call, so a shrink/refresh mid-drag clamps
 * safely rather than leaving a stale view_top. Never touches the keyboard
 * highlight or any selection.
 *
 * mixa_app_fmpanel_release ends an active drag; call it on every mouse
 * event where the left button is NOT held (a real button-up, or an
 * ordinary hover-move with no button at all) -- idempotent when no drag is
 * active, so the caller does not need to track that state itself. */
int mixa_app_fmpanel_release(MixaAppFmPanel *p);

/* Introspection for tests (same reason mixa_app_controller_fm exists):
 * the index of the first entry currently drawn, and the keyboard-highlighted
 * entry's index or -1 if none has been established yet. */
size_t mixa_app_fmpanel_view_top(const MixaAppFmPanel *p);
int mixa_app_fmpanel_highlight_index(const MixaAppFmPanel *p);

/* Introspection for tests (ticket 20260913-060000): whether a scrollbar
 * drag is currently active. */
int mixa_app_fmpanel_drag_active(const MixaAppFmPanel *p);

#endif
