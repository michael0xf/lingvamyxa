/* mixa_backend.h - the one seam between Mixa Manager and a platform.
 *
 * Design: mixa_manager/BACKEND_SEAM.txt. Distribution model:
 * Mixa_Manager_DISTRIBUTION_MODEL.txt section 5.
 *
 * Every target reimplements exactly what is declared here and nothing else, so
 * this file is the whole porting cost. Keep it small.
 *
 * Written as a hand-written C header for now. Converting it to a .h.lm1 header
 * unit is H2, and it is blocked on two separate things rather than one:
 *
 *   1. the header-unit L1 lives in l1src and has not been promoted into
 *      stg/l1_baseline, which is the translator mixa_manager builds against on
 *      purpose. Probed 2026-09-09: the baseline rejects "struct:" outright,
 *      while the l1src build emits a correct .lm1.h for the same fixture.
 *   2. after that promotion, run_mixa.ps1 still has to translate .h.lm1 into
 *      .lm1.h before compiling the units that include it. Promotion alone does
 *      not implement the conversion - they are separate gaps.
 *
 * Codex confirmed on 2026-09-09 that the promotion is intended, in a scoped
 * integration window once the root L1 settles. Until both parts land, this file
 * stays hand-written, and that is a known exception to the hermeticity the
 * distribution model asks for, not the final shape.
 */
#ifndef MIXA_BACKEND_H
#define MIXA_BACKEND_H

#include <stddef.h>
#include "mixa_manager/mixa_overlay.h"   /* MixaU8 */

/* The backend owns its own state. Callers hold an opaque pointer and never see
 * a platform handle. */
typedef struct MixaBackend MixaBackend;

typedef enum MixaEventKind {
    MIXA_EVENT_NONE = 0,
    MIXA_EVENT_KEY,
    MIXA_EVENT_MOUSE,
    MIXA_EVENT_RESIZE,
    MIXA_EVENT_CLOSE
} MixaEventKind;

/* Non-text keys. Text arrives as a codepoint instead, never as one of these.
 * The split is deliberate: merging them would force every platform to invent
 * private values for the non-text keys, which is the private protocol the model
 * warns against in section 0.2. */
typedef enum MixaKeyCode {
    MIXA_KEY_NONE = 0,
    MIXA_KEY_UP,
    MIXA_KEY_DOWN,
    MIXA_KEY_LEFT,
    MIXA_KEY_RIGHT,
    MIXA_KEY_HOME,
    MIXA_KEY_END,
    MIXA_KEY_PAGE_UP,
    MIXA_KEY_PAGE_DOWN,
    MIXA_KEY_INSERT,
    MIXA_KEY_DELETE,
    MIXA_KEY_BACKSPACE,
    MIXA_KEY_TAB,
    MIXA_KEY_ENTER,
    MIXA_KEY_ESCAPE,
    MIXA_KEY_F1,
    MIXA_KEY_F2,
    MIXA_KEY_F3,
    MIXA_KEY_F4,
    MIXA_KEY_F5,
    MIXA_KEY_F6,
    MIXA_KEY_F7,
    MIXA_KEY_F8,
    MIXA_KEY_F9,
    MIXA_KEY_F10,
    MIXA_KEY_F11,
    MIXA_KEY_F12
} MixaKeyCode;

typedef enum MixaModifier {
    MIXA_MOD_SHIFT = 1,
    MIXA_MOD_CTRL = 2,
    MIXA_MOD_ALT = 4,
    MIXA_MOD_META = 8
} MixaModifier;

typedef enum MixaButton {
    MIXA_BUTTON_LEFT = 1,
    MIXA_BUTTON_MIDDLE = 2,
    MIXA_BUTTON_RIGHT = 4
} MixaButton;

/* One shape for every kind, so the seam does not grow a variant per platform.
 *
 * A key event carries EITHER a codepoint (text) OR a keycode (not text), never
 * both. Mouse coordinates are CELLS, not pixels: the backend knows the geometry
 * from open and converts once, so consumers cannot drift. */
typedef struct MixaEvent {
    int kind;               /* MixaEventKind */
    unsigned int codepoint; /* key: Unicode scalar, 0 when not text */
    int keycode;            /* key: MixaKeyCode, MIXA_KEY_NONE when text */
    unsigned int modifiers; /* key and mouse: MixaModifier bits */
    size_t row;             /* mouse: cell row */
    size_t col;             /* mouse: cell column */
    unsigned int buttons;   /* mouse: MixaButton bits */
    size_t cols;            /* resize: new geometry */
    size_t rows;
} MixaEvent;

/* The six operations. Resize is deliberately not a seventh - it arrives as an
 * event, because that is how every platform delivers it. See poll. */

/* Creates the surface. Cell geometry in, pixels derived.
 *
 * The caller owns the MixaBackend storage and MUST zero it before the first
 * open: open refuses a backend that is already open, but it cannot tell a
 * never-opened backend from uninitialised memory, and it does not initialise
 * the fields it has no value for yet. Sizing the struct needs the concrete
 * backend's own header, which is where the type is completed. */
int mixa_backend_open(MixaBackend *backend, size_t cols, size_t rows,
                      size_t cell_width, size_t cell_height);

/* One composited frame. The buffer is borrowed for the call and not retained.
 * Refused unless it holds at least a whole frame at the CURRENT geometry - see
 * poll, which is where the geometry can change. */
int mixa_backend_present(MixaBackend *backend, const MixaU8 *rgba, size_t bytes);

/* Next pending event, or MIXA_EVENT_NONE. Never blocks.
 *
 * Not blocking is the contract, not an omission. The loop belongs to the
 * Message: a running Message is an L3 Thread with a FIFO inbox and one serial
 * lane (Lingvamyxa_spec.txt 19.29.6), and the backend is an event source
 * feeding it, so poll is a drain. A blocking poll would move the wait inside
 * the event source, which would then be deciding how long to wait on behalf of
 * a Message whose other work it cannot see. BACKEND_SEAM.txt section 7.
 *
 * Delivering MIXA_EVENT_RESIZE also ADOPTS the new geometry, because by the
 * time a platform tells you it resized, its surface already has. poll re-derives
 * the frame from the event's cols/rows and the cell size fixed at open, so from
 * this call onward present validates against the new size. Frame contents after
 * an adoption are undefined: composite and present a full frame.
 *
 * This is what keeps resize from needing a seventh function - the event is the
 * resize, not a notification that one should be requested.
 *
 * Returns non-zero if the new size could not be adopted (it overflows, or the
 * allocation failed). The event is still written to out and the old surface is
 * left intact, so a caller that ignores the result keeps a working surface at
 * the old size rather than a half-resized one. */
int mixa_backend_poll(MixaBackend *backend, MixaEvent *out);

/* UTF-8 both ways.
 *
 * clipboard_get returns the clipboard length in bytes, excluding the
 * terminator, whether or not it fit. It writes out only when cap is at least
 * that length plus one, and leaves out untouched otherwise - never a silent
 * truncation. Returning the length needed rather than the number written is
 * what lets a caller tell an empty clipboard (0) from a buffer that was too
 * small (result + 1 > cap); both are ordinary conditions and they call for
 * opposite reactions.
 *
 * This carries more weight than its size suggests. It is the practical path for
 * entering text a keyboard cannot easily produce - copy a name from the panel,
 * paste it into the rename dialog - which is why full IME integration can be
 * deferred without making those locales unusable. */
size_t mixa_backend_clipboard_get(MixaBackend *backend, char *out, size_t cap);
int mixa_backend_clipboard_set(MixaBackend *backend, const char *text);

void mixa_backend_close(MixaBackend *backend);

#endif
