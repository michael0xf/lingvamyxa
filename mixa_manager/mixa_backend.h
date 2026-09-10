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
 * integration window once the root L1 settles, and that the timing is
 * coordinated rather than promised. An earlier revision of this comment said
 * the new parser had been found not to match the old one and that the
 * translator would be wholesale REPLACED; Codex corrected both, and the
 * correction is recorded here rather than quietly dropped. What is actually
 * established as of 2026-09-09: a parity effort is running against
 * lingvamyxa_old_worked_version, the 119 historical fixtures AGREE across the
 * available runners, coverage is incomplete rather than proof of equivalence,
 * and some divergence from the old parser is a DELIBERATE grammar change by
 * the language owner - empty colon frames must now fail - not a regression.
 *
 * So this file stays hand-written until both parts above land, which is a known
 * exception to the hermeticity the distribution model asks for, not the final
 * shape.
 *
 * What does follow, and is the reason run_mixa.ps1 reports translator identity:
 * mixa_manager builds against a translator it does not own and cannot pin. Any
 * change to that floor, coordinated or not, lands here as a suite result, so
 * the suite has to stay fast, complete, and explicit about what produced a
 * green run.
 */
#ifndef MIXA_BACKEND_H
#define MIXA_BACKEND_H

#include <stddef.h>
#include "mixa_manager/mixa_overlay.h"   /* MixaU8 */

/* The backend owns its own state, including its own allocation. Callers hold a
 * pointer to this base and never see a platform handle.
 *
 * MixaBackend is COMPLETE and carries exactly one field: the dispatch table the
 * handle was opened with. Every concrete backend declares its own struct with
 * the same pointer as its FIRST member and casts between the two, which is what
 * lets several backends link into one binary and be chosen at startup - see
 * BACKEND_SEAM.txt section 11. The six non-open operations below are plain
 * forwards through this field, so their call sites did not have to change when
 * dispatch arrived; they read backend->vt and nothing else. */
typedef struct MixaBackendVTable MixaBackendVTable;
typedef struct MixaBackend MixaBackend;

struct MixaBackend {
    const MixaBackendVTable *vt; /* never null between open and close */
};

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
 * both.
 *
 * Mouse coordinates are HALF-CELLS of the TEXT layer, not pixels: the backend
 * knows the geometry from open and converts once, so consumers cannot drift.
 *
 * Half, rather than whole, because the pointer moves on a lattice of half a
 * text cell (UI_MODEL 7). That is the finest step at which BOTH grids have
 * defined positions - a text cell is two steps, an upper cell is also two,
 * offset by one - so the mapping is arithmetic with no rounding and no floating
 * point anywhere:
 *
 *     text cell  = half >> 1
 *     upper cell = (half - 1) >> 1, and half == 0 lies outside the upper layer
 *
 * The unit is the TEXT layer's cell even when the upper layer runs a different
 * font, because the text layer is the full screen and the upper one is inset
 * and derived from it. */
typedef struct MixaEvent {
    int kind;               /* MixaEventKind */
    unsigned int codepoint; /* key: Unicode scalar, 0 when not text */
    int keycode;            /* key: MixaKeyCode, MIXA_KEY_NONE when text */
    unsigned int modifiers; /* key and mouse: MixaModifier bits */
    size_t row;             /* mouse: HALF-cell row, see below */
    size_t col;             /* mouse: HALF-cell column */
    unsigned int buttons;   /* mouse: MixaButton bits */
    size_t cols;            /* resize: new geometry */
    size_t rows;
} MixaEvent;

/* Seam operations. Resize arrives as an event (not a function). Glyph is the
 * seventh operation - see MixaGlyph below / BACKEND_SEAM 9.4. */

/* Cell metrics for every layer, reported by open.
 *
 * Both layers are CELL GRIDS - see BACKEND_SEAM.txt section 10. The upper layer
 * is not a raster: it is a second grid with a larger monospace font, drawn
 * semi-transparently over the text layer by the same pseudographics drawer.
 *
 * A struct rather than out-parameters because a third layer for images is
 * explicitly anticipated, and a struct grows without breaking this signature
 * again.
 *
 * A backend with only one font reports the same metrics twice. That is a
 * legitimate answer, not a failure. */
typedef struct MixaCellMetrics {
    size_t text_cell_width;
    size_t text_cell_height;
    size_t upper_cell_width;
    size_t upper_cell_height;
    /* The mouse pointer's font. Not a third cell grid - see UI_MODEL 7.3. The
     * pointer is one glyph at a position on a half-cell lattice, so this sizes
     * the glyph and nothing else; a pointer larger than its half step is
     * allowed and expected. Target is the text size or slightly above, chosen
     * by the rule in BACKEND_SEAM 10.3 - the available system size nearest the
     * target, used as it comes. */
    size_t pointer_cell_width;
    size_t pointer_cell_height;
} MixaCellMetrics;

/* Creates the surface. Cols and rows in; the CELL METRICS ARE IN/OUT.
 *
 * The caller fills in the cells it would like and the backend writes back the
 * cells it actually has. A backend with a font overwrites the request with that
 * font's native metrics, because the rendering doctrine forbids scaling a font
 * to fit a request - see BACKEND_SEAM.txt section 9. A backend without a font,
 * such as the headless one, honours the request exactly, which is what keeps it
 * usable as a fixture on a machine with no font at all.
 *
 * In/out rather than a separate metrics query, so that ignoring the answer is
 * awkward: a caller that states a wish and never reads back what it got is
 * precisely the mistake that produces stretched, blurred text.
 *
 * The metrics pointer is required and no value in it may be zero.
 *
 * THE BACKEND ALLOCATES. vt selects which one; out receives the handle and is
 * written only on success.
 *
 * An earlier revision had the CALLER own the storage and zero it before the
 * first open. That cannot survive dispatch: owning the storage means knowing
 * the concrete size, knowing the concrete size means including one concrete
 * backend's header, and that include is exactly the hard binding to a platform
 * that section 11 exists to remove. A table cannot undo it - the #ifdef simply
 * moves from the call to the allocation.
 *
 * It also retires a footgun that revision documented against itself: open could
 * not tell a never-opened backend from uninitialised memory. Now there is
 * nothing to mistake.
 *
 * close frees the handle. Every successful open is paired with exactly one
 * close, and close is a no-op on a null pointer. */
int mixa_backend_open(const MixaBackendVTable *vt, MixaBackend **out,
                      size_t cols, size_t rows, MixaCellMetrics *metrics);

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
size_t mixa_backend_clipboard_get(const MixaBackend *backend, char *out, size_t cap);
int mixa_backend_clipboard_set(MixaBackend *backend, const char *text);

void mixa_backend_close(MixaBackend *backend);

/* Seventh seam operation: one glyph at the font's NATIVE size.
 *
 * Design: BACKEND_SEAM.txt section 9.4 / Mixa_Manager_RENDERING_DOCTRINE.txt.
 * Returns an 8-bit alpha COVERAGE bitmap - never scaled - plus bearing to place
 * it in the cell and advance width so the application can detect a WIDE glyph
 * (two cells). The backend reports; the application decides.
 *
 * layer selects which cell-grid font (text vs upper). Cache is per-backend and
 * keyed by (codepoint, layer); coverage storage is owned by the backend.
 *
 * A codepoint the platform cannot render is reported missing (out->missing != 0)
 * rather than silently substituted - font fallback stays an application choice.
 */
typedef enum MixaGlyphLayer {
    MIXA_LAYER_TEXT = 0,
    MIXA_LAYER_UPPER = 1,
    MIXA_LAYER_POINTER = 2
} MixaGlyphLayer;

typedef struct MixaGlyph {
    size_t width;           /* bitmap width in pixels */
    size_t height;          /* bitmap height in pixels */
    int bearing_x;          /* offset from cell left to bitmap left */
    int bearing_y;          /* offset from cell top to bitmap top */
    size_t advance;         /* pen advance in pixels */
    const MixaU8 *coverage; /* width*height bytes, backend-owned; NULL if missing */
    int missing;            /* non-zero: platform has no glyph for this codepoint */
} MixaGlyph;

/* Rasterize one codepoint for the given layer into out. out may be stack-
 * allocated; coverage points into the backend cache and must not be freed by
 * the caller. Returns non-zero on hard failure (null args / not open); a
 * missing codepoint is a successful call with out->missing set. */
int mixa_backend_glyph(MixaBackend *backend, int layer, unsigned int codepoint,
                       MixaGlyph *out);


/* ---- Dispatch: one table per backend, chosen at startup ----------------- *
 *
 * Design: BACKEND_SEAM.txt section 11. The seam already said a target
 * reimplements what mixa_backend.h declares and nothing else; the table is that
 * sentence made checkable. A new platform adds a unit and a constructor. It
 * does not edit a call site, an #ifdef, or this header.
 *
 * One typedef per operation rather than inline function-pointer syntax: the
 * table then reads as a list of operations, and a reviewer can hold it beside
 * the declarations above and see a missing one. Slot order matches declaration
 * order for the same reason. */
typedef int    (*MixaBackendOpenFn)(MixaBackend **out, size_t cols, size_t rows,
                                    MixaCellMetrics *metrics);
typedef int    (*MixaBackendPresentFn)(MixaBackend *backend, const MixaU8 *rgba,
                                       size_t bytes);
typedef int    (*MixaBackendPollFn)(MixaBackend *backend, MixaEvent *out);
typedef size_t (*MixaBackendClipboardGetFn)(const MixaBackend *backend,
                                            char *out, size_t cap);
typedef int    (*MixaBackendClipboardSetFn)(MixaBackend *backend,
                                            const char *text);
typedef int    (*MixaBackendGlyphFn)(MixaBackend *backend, int layer,
                                     unsigned int codepoint, MixaGlyph *out);
typedef void   (*MixaBackendCloseFn)(MixaBackend *backend);

/* name is for selection and diagnostics: short, lowercase, stable ("headless",
 * "win32"). It is compared, so it is part of the contract, not a comment. */
struct MixaBackendVTable {
    const char *name;
    MixaBackendOpenFn open;
    MixaBackendPresentFn present;
    MixaBackendPollFn poll;
    MixaBackendClipboardGetFn clipboard_get;
    MixaBackendClipboardSetFn clipboard_set;
    MixaBackendGlyphFn glyph;
    MixaBackendCloseFn close;
};

/* All-or-nothing. Non-zero when name is present and non-empty AND every slot is
 * filled; zero otherwise, with no attempt to say WHICH slot is missing.
 *
 * Partial tables are the failure this exists to stop, and they are a
 * PORTING-TIME mistake, not a runtime condition: someone adds an operation to
 * the seam and one backend does not grow it. A table that half works fails
 * later, in whatever feature happened to reach the empty slot, and the port
 * looks finished until then. Validated once at selection, it fails at startup
 * with the backend named.
 *
 * Non-zero means VALID. A predicate named _valid that returns 0 for valid is a
 * trap for every call site that reads like English. */
int mixa_backend_table_valid(const MixaBackendVTable *vt);

/* The tables compiled into THIS build, in the order the build listed them.
 * count is required; the array is static and outlives every caller. */
const MixaBackendVTable *const *mixa_backend_tables(size_t *count);

/* Selection at startup. Both return NULL rather than a partial or unknown
 * table, and neither ever returns one that mixa_backend_table_valid rejects.
 *
 * by_name is the explicit choice - a command line flag, or a test picking the
 * headless fixture on a machine that also has a real window. default_table is
 * the first valid entry, which is why the build lists the real backend before
 * the fixture. */
const MixaBackendVTable *mixa_backend_table_by_name(const char *name);
const MixaBackendVTable *mixa_backend_default_table(void);

/* Each backend exports exactly one public symbol. Declared in the backend's own
 * concrete header, not here, so that adding a platform does not touch the seam:
 *
 *     const MixaBackendVTable *mixa_backend_headless_table(void);
 *     const MixaBackendVTable *mixa_backend_win32_table(void);
 *
 * A constructor returns a validated singleton. It does not fill a caller's
 * table: a table is immutable once built, and handing out a pointer to the one
 * copy is both cheaper and harder to corrupt than copying seven pointers into
 * whatever storage the caller happened to provide. */

#endif
