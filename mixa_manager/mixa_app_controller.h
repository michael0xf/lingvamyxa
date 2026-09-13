#ifndef MIXA_APP_CONTROLLER_H
#define MIXA_APP_CONTROLLER_H

/* The reusable app-controller seam (ticket 20260913-010249). FIRST_
 * VERSION.txt's own feature list (glyphs, process seam, console, Enter,
 * F1) is now real, but every previous ticket wired it directly into
 * mixa_app_main.lm1's own main() -- build-only, since that file opens a
 * real window and can never run in an automated suite. This module
 * extracts that SAME orchestration (open the app loop, the console
 * view/file, F1 Help, the editable command line and its Enter dispatch,
 * the render surfaces; one non-blocking tick; release everything) into
 * its own callable seam, so the identical production logic can be
 * driven by EITHER a real Win32 backend (the real entrypoint, now a
 * thin owner that only supplies vt/root_utf8 and drives the loop) OR
 * the existing headless backend with a real temporary file (an
 * automated end-to-end test) -- never a parallel test-only copy of the
 * orchestration.
 *
 * MixaAppController is opaque: a plain C header, reached via include:
 * not predef:, for the same measured import-table reason mixa_app_path/
 * mixa_help/mixa_cmdline_dispatch already are. Its concrete fields live
 * in mixa_app_controller_impl.h, included ONLY by mixa_app_controller.
 * lm1 -- the same opaque-handle split mixa_file.h/mixa_file_win32.h and
 * mixa_cmdline_dispatch.h/_impl.h already establish.
 *
 * Driving shape: open() once, then the CALLER's own while loop calls
 * step() repeatedly while running() is nonzero, then close() once. This
 * function does not loop internally, which is exactly what lets the
 * SAME step() implementation serve a real window's own message pump
 * (Win32) and a synthetic, test-injected event stream (headless) with
 * no branching on which backend is in use anywhere in this seam. */

#include <stddef.h>
#include "mixa_manager/mixa_backend.h"
#include "mixa_manager/mixa_core.h"

typedef struct MixaAppController MixaAppController;
typedef struct MixaFm MixaFm;

#define MIXA_APP_CONTROLLER_OK 0
#define MIXA_APP_CONTROLLER_ERR_ARG 1
#define MIXA_APP_CONTROLLER_ERR_OPEN 2

/* Opens everything the production loop needs. vt selects the backend
 * (win32 for the real entrypoint, headless for an automated end-to-end
 * test); root_utf8 is joined verbatim into an exact, owned copy (no
 * normalization, no truncation -- mixa_app_path's own already-tested
 * contract). On success *out is a new, owned handle; on any failure
 * *out is left null and everything this call itself acquired is
 * already released. */
int mixa_app_controller_open(MixaAppController **out,
                             const MixaBackendVTable *vt,
                             const char *root_utf8);

/* One tick: polls any in-flight command (never blocking -- at most one
 * real process read), drains the backend's own pump for at most one
 * event, and dispatches it through Copy Here / F1 Help / command-line
 * editing / Enter exactly as the production loop always has, then
 * presents one frame whenever either the event or the command poll made
 * progress (an idle tick with neither sleeps 1ms). *out_had_error is
 * set nonzero the one tick the underlying step reports a real error
 * (the caller should stop calling step() once running() goes to 0
 * afterward). The caller's own loop decides when to stop -- this
 * function never loops internally. */
int mixa_app_controller_step(MixaAppController *c, int *out_had_error);

/* Nonzero while the underlying app loop has not yet seen a CLOSE event
 * (mirrors mixa_app_loop_running exactly). */
int mixa_app_controller_running(const MixaAppController *c);

/* Releases everything this controller owns, in the reverse of
 * acquisition order. Null-safe. */
void mixa_app_controller_close(MixaAppController *c);

/* --- Introspection, all borrowed and read-only. Lets a headless
 * end-to-end test inject synthetic events and inspect exactly what the
 * SAME production code rendered/decided, without a parallel test-only
 * copy of any orchestration logic. */
MixaBackend *mixa_app_controller_backend(const MixaAppController *c);
const MixaTextRect *mixa_app_controller_text_rect(const MixaAppController *c);
const MixaTextRect *mixa_app_controller_upper_rect(const MixaAppController *c);
int mixa_app_controller_command_running(const MixaAppController *c);
int mixa_app_controller_last_exit_code(const MixaAppController *c);
const char *mixa_app_controller_cwd(const MixaAppController *c);
size_t mixa_app_controller_cursor_row(const MixaAppController *c);
size_t mixa_app_controller_cursor_col(const MixaAppController *c);
size_t mixa_app_controller_file_size(const MixaAppController *c);

/* Borrowed, read-only introspection for the same reason as the block
 * above (ticket 20260913-032000's own real production-controller test):
 * the MixaFm the app-loop already owns, so a test can select real
 * entries through the real mixa_fm_select API before driving a synthetic
 * click on the visible Delete/Copy Here row -- never a second, test-only
 * selection model. The caller must predef mixa_file_manager.h.lm1 itself
 * to reach mixa_fm_select/_count/_name's own prototypes; this header
 * only forward-declares the opaque type, exactly like mixa_app_fmpanel.h
 * already does for the identical reason. */
MixaFm *mixa_app_controller_fm(const MixaAppController *c);

/* Same reasoning, for the vertical-scroll/keyboard-highlight state
 * (ticket 20260913-044900): a test can read mixa_app_fmpanel_view_top/
 * _highlight_index on the SAME real panel the production controller
 * renders, never a second copy of that state. Declared with the bare
 * struct tag (no typedef here) so this header does not need to know
 * about MixaAppFmPanel at all beyond the pointer type -- the caller
 * includes mixa_app_fmpanel.h itself for the real typedef and the
 * getter prototypes, exactly like the mixa_fm_select() note above. */
struct MixaAppFmPanel *mixa_app_controller_fmpanel(const MixaAppController *c);

#endif
