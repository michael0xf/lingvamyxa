#ifndef MIXA_CMDLINE_H
#define MIXA_CMDLINE_H

/* Editable command line (ticket 20260913-002013, FIRST_VERSION.txt
 * section 2/4: "edits lines, wrapping BY CHARACTER, never by word - as
 * a console does" / "a cursor in the lower layer" / "Enter takes the
 * line from its start UP TO THE CURSOR, sends it plus a newline to the
 * OS"). This is the PENDING line only -- text typed but not yet sent.
 * It is never written into the console file itself (FILE_SEAM is
 * append-only; a file cannot represent backspace), and the console's
 * own scrollback stays exactly what it always was: committed history,
 * "not a buffer" (FIRST_VERSION line 39). Only mixa_cmdline_extract_to_
 * cursor_utf8's own OUTPUT -- the line up to the cursor, at Enter -- is
 * ever appended to the file, by the caller, once.
 *
 * Codepoints, not bytes: the key event seam already delivers text as a
 * Unicode scalar per keystroke (mixa_backend.h's own MixaEvent.
 * codepoint), so this stores an array of those directly. Insert/delete/
 * cursor-move are therefore trivial index arithmetic with no UTF-8
 * boundary logic anywhere in the EDITING path -- UTF-8 encoding happens
 * exactly once, at extract time, for exactly the bytes about to leave
 * this module.
 *
 * No fixed capacity anywhere: the codepoint array grows by checked
 * doubling (mixa_cmdline_check_fault, mirroring mixa_process_marker's
 * own established fault-injection idiom) and extract's own output
 * buffer is sized exactly from the substring's real UTF-8 length.
 *
 * Wrapping: mixa_cmdline_char_position is pure arithmetic answering
 * "where does character index i land, given the line starts at
 * (start_row, start_col) and the screen is screen_cols wide" -- BY
 * CHARACTER, never by word, per FIRST_VERSION's own explicit
 * instruction. It does not touch a MixaTextRect or draw anything; the
 * caller (the real entrypoint, which cannot be exercised by an
 * automated suite) uses the returned coordinates to write cells
 * directly, the same way it already draws the F1 Help button. Rows
 * that would fall below screen_rows are reported as such rather than
 * silently wrapping into a location that does not exist -- this
 * milestone does not implement scrolling the PENDING line (only
 * committed file content scrolls, via the already-accepted console
 * view's own recompute); a line long enough to need that is named as a
 * known limitation, not silently mishandled.
 */

#include <stddef.h>

#define MIXA_CMDLINE_OK 0
#define MIXA_CMDLINE_ERR_ARG 1
#define MIXA_CMDLINE_ERR_NOMEM 2

typedef int (*MixaCmdLineFaultFn)(void *ctx, size_t requested_size);

typedef struct MixaCmdLineFaultVTable {
    MixaCmdLineFaultFn on_alloc;
    void *ctx;
} MixaCmdLineFaultVTable;

typedef struct MixaCmdLine {
    unsigned int *cps;
    size_t len;
    size_t cap;
    size_t cursor;
} MixaCmdLine;

/* Zeroes the struct; no allocation happens until the first insert. */
void mixa_cmdline_init(MixaCmdLine *cl);

/* Frees the owned codepoint array and zeroes the struct. Safe on an
 * already-released or never-inserted-into line. */
void mixa_cmdline_release(MixaCmdLine *cl);

/* Inserts codepoint at the cursor, shifting anything after it right,
 * and advances the cursor past the inserted character. fault is
 * nullable (see MixaCmdLineFaultVTable). */
int mixa_cmdline_insert(MixaCmdLine *cl, const MixaCmdLineFaultVTable *fault,
                        unsigned int codepoint);

/* Removes the character immediately BEFORE the cursor and moves the
 * cursor back by one. A no-op (not an error) at cursor 0. */
void mixa_cmdline_backspace(MixaCmdLine *cl);

/* Removes the character immediately AT the cursor (the next one to the
 * right), cursor unchanged. A no-op (not an error) at the line's end. */
void mixa_cmdline_delete_forward(MixaCmdLine *cl);

void mixa_cmdline_move_left(MixaCmdLine *cl);
void mixa_cmdline_move_right(MixaCmdLine *cl);
void mixa_cmdline_move_home(MixaCmdLine *cl);
void mixa_cmdline_move_end(MixaCmdLine *cl);

/* Resets to an empty line at cursor 0 WITHOUT freeing the owned array
 * (cheap reuse for the very next command) -- call after Enter commits
 * the extracted prefix. */
void mixa_cmdline_clear(MixaCmdLine *cl);

/* Encodes cps[0..cursor) as UTF-8 into a freshly allocated, exactly
 * sized, NUL-terminated buffer the caller owns and frees with free().
 * This is FIRST_VERSION's own "line from its start UP TO THE CURSOR" --
 * characters after the cursor are never included, matching the spec
 * verbatim rather than sending the whole line. *out is set to 0 on
 * every failure path. */
int mixa_cmdline_extract_to_cursor_utf8(const MixaCmdLine *cl,
                                        const MixaCmdLineFaultVTable *fault,
                                        char **out);

/* Pure arithmetic, BY CHARACTER (never by word): character index i of a
 * line beginning at (start_row, start_col) on a screen screen_cols wide
 * lands at column (start_col + i) % screen_cols, row start_row +
 * (start_col + i) / screen_cols. Returns MIXA_CMDLINE_OK with *out_row/
 * *out_col set when that row is < screen_rows; returns
 * MIXA_CMDLINE_ERR_ARG (out_row/out_col left untouched) when the
 * character would land at or past screen_rows -- this milestone does
 * not scroll the pending line, so such a character is reported, never
 * drawn at a wrong or wrapped-around location. */
int mixa_cmdline_char_position(size_t start_row, size_t start_col,
                               size_t screen_cols, size_t screen_rows,
                               size_t index, size_t *out_row,
                               size_t *out_col);

#endif
