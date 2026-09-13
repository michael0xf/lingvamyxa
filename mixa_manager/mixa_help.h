#ifndef MIXA_HELP_H
#define MIXA_HELP_H

/* F1 Help (FIRST_VERSION.txt section 2/4/5, ticket 20260912-235847):
 * "one button on it: F1 help, which writes its text into the console,
 * ending with a newline" / "F1 here corresponds to ClearShell's About,
 * which calls mainHelp. Ours writes its help text into the console
 * rather than opening a dialog."
 *
 * The text is ClearShell's own mainHelp() message verbatim (ClearShell.
 * java, mainHelp(): the String shown as its About dialog's body) --
 * see MIXA_HELP_TEXT below. This is console OUTPUT, not a modal: the
 * action appends bytes to the already-open console file, the same file
 * the console view already reads and the same file a real child process
 * would write to (FIRST_VERSION: "the child gets a command and a place
 * to write, and that is all" -- Help writes to that same place).
 *
 * Two trigger paths reach the SAME action, mirroring the shape already
 * established by mixa_fm_copy_here_key_event/_hit for Ctrl+V/Copy Here:
 * a plain F1 key press (mixa_help_key_event) and a hit-test against the
 * button's own on-screen rect (mixa_help_hit). Both are pure functions
 * taking raw geometry (row/col/nrows/ncols), not a MixaButtonPanel --
 * same decoupling Copy Here's own hit function already uses, so this is
 * testable without constructing a real button panel or opening a
 * window.
 *
 * Deliberately excluded (out of this milestone, FIRST_VERSION section
 * 3): no terminal emulation, no modal/dialog, no new synchronization
 * primitive. The single mixa_file_append call this makes either writes
 * the whole message or writes nothing -- "no partial publication" is
 * enforced by treating a short write as a failure (never claimed as
 * success), not by inventing transactional semantics FILE_SEAM itself
 * does not offer.
 */

#include <stddef.h>
#include "mixa_manager/mixa_file.h"
#include "mixa_manager/mixa_backend.h"

#define MIXA_HELP_OK 0
#define MIXA_HELP_ERR_ARG 1
#define MIXA_HELP_ERR_NOMEM 2
#define MIXA_HELP_ERR_IO 3

#define MIXA_HELP_HIT_NONE 0
#define MIXA_HELP_HIT_ACTION 1

/* Verbatim from ClearShell.java's mainHelp(): the exact String passed to
 * AlertDialog.Builder.setMessage(m). No CRLF/newline of its own --
 * mixa_help_action appends "\r\n" itself, matching the ticket's own
 * instruction to end with CRLF. A function rather than an exported
 * array, matching mixa_app_panel_entry_label's own const-char-pointer-
 * return shape elsewhere in this codebase. */
const char *mixa_help_text(void);

typedef struct MixaHelpCtx {
    MixaFile *console_file; /* borrowed; not owned, not closed here */
    int last_status;
} MixaHelpCtx;

/* Appends MIXA_HELP_TEXT + "\r\n" to ctx->console_file in ONE mixa_file_
 * append call. Returns MIXA_HELP_OK only if every byte was accepted;
 * any short write or append failure is reported (MIXA_HELP_ERR_IO),
 * never silently treated as success. ctx->last_status mirrors the
 * return value for a caller that reached this indirectly through
 * mixa_help_key_event/_hit. */
int mixa_help_action(void *ctx_raw);

/* Recognizes a bare F1 key press on a MIXA_EVENT_KEY event (any
 * modifier state -- F1 is F1 regardless of incidental Shift/Alt/Ctrl/
 * Meta noise; no modifier gating is specified anywhere in FIRST_VERSION
 * or PORT_OF_CLEARSHELL, unlike Copy Here's own Ctrl+V, which explicitly
 * requires MIXA_MOD_CTRL). On a match, runs the action and returns
 * MIXA_HELP_HIT_ACTION; otherwise MIXA_HELP_HIT_NONE, touching nothing. */
int mixa_help_key_event(MixaHelpCtx *ctx, const MixaEvent *ev);

/* Rect hit-test in TEXT-LAYER cells (matching mixa_fm_copy_here_hit's own
 * shape exactly) -- a hit inside [btn_row, btn_row+btn_nrows) x
 * [btn_col, btn_col+btn_ncols) runs the action and returns MIXA_HELP_
 * HIT_ACTION; anything else returns MIXA_HELP_HIT_NONE untouched. */
int mixa_help_hit(MixaHelpCtx *ctx, size_t cell_row, size_t cell_col,
                  size_t btn_row, size_t btn_col, size_t btn_nrows,
                  size_t btn_ncols);

#endif
