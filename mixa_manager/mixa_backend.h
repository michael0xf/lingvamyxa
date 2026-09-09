/* mixa_backend.h - the one seam between Mixa Manager and a platform.
 *
 * Design: mixa_manager/BACKEND_SEAM.txt. Distribution model:
 * Mixa_Manager_DISTRIBUTION_MODEL.txt section 5.
 *
 * Every target reimplements exactly what is declared here and nothing else, so
 * this file is the whole porting cost. Keep it small.
 *
 * Written as a hand-written C header for now. Converting it to a .h.lm1 header
 * unit is H2 and is not started.
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
 * event, because that is how every platform delivers it. */

int mixa_backend_open(MixaBackend *backend, size_t cols, size_t rows,
                      size_t cell_width, size_t cell_height);

/* One composited frame. The buffer is borrowed for the call and not retained. */
int mixa_backend_present(MixaBackend *backend, const MixaU8 *rgba, size_t bytes);

/* Next pending event, or MIXA_EVENT_NONE. Never blocks. Who owns the event loop
 * is a question for the first real backend, not for the seam. */
int mixa_backend_poll(MixaBackend *backend, MixaEvent *out);

/* UTF-8 both ways. Returns bytes written, excluding the terminator; 0 when the
 * buffer is too small - never a silent truncation.
 *
 * This carries more weight than its size suggests. It is the practical path for
 * entering text a keyboard cannot easily produce - copy a name from the panel,
 * paste it into the rename dialog - which is why full IME integration can be
 * deferred without making those locales unusable. */
size_t mixa_backend_clipboard_get(MixaBackend *backend, char *out, size_t cap);
int mixa_backend_clipboard_set(MixaBackend *backend, const char *text);

void mixa_backend_close(MixaBackend *backend);

#endif
