#ifndef MIXA_CMDLINE_DISPATCH_H
#define MIXA_CMDLINE_DISPATCH_H

/* Enter dispatch (ticket 20260913-002013). FIRST_VERSION.txt section 2 /
 * PROCESS_SEAM.txt section 2: "Pressing Enter takes the line from its
 * start up to the CURSOR, sends it plus a newline to the OS, and the
 * child's output arrives at the same place - as if the cursor itself
 * were writing. All of it lands in the file." That is PROCESS_SEAM's own
 * "Run OS Command": ONE FRESH PROCESS PER COMMAND LINE (the in-band
 * marker line records that command's own exit status and final working
 * directory, PROCESS_SEAM 4.1) -- not a persistent shell session.
 *
 * This module composes three ALREADY-ACCEPTED seams end to end: the
 * process-marker wrapper, the raw process seam, and the console file --
 * no new spawning, streaming or marker logic of its own, only real
 * wiring. A plain C header, opaque to callers, reached via include: --
 * NOT predef'd -- for the same measured reason mixa_app_path/mixa_help
 * are: this entrypoint's own import path table is already at its
 * documented limit (mixa_app_main.lm1's own comment), and predef'ing
 * even this module's own small header (which itself would need to
 * predef mixa_process_marker.h.lm1) was tried and measured to overflow
 * it directly ("import path table full" at mixa_process_marker.h.lm1
 * when added alongside the existing fm_copy+console_window chain).
 * MixaCmdDispatch's own concrete fields live in mixa_cmdline_dispatch_
 * impl.h, included ONLY by mixa_cmdline_dispatch.lm1 -- the same
 * opaque-handle split mixa_file.h/mixa_file_win32.h already establish.
 *
 * Ownership and lifetime: start() spawns a REAL child and returns a
 * handle the caller must poll() every tick until it reports DONE, then
 * release() (release() also KILLS the child if the caller abandons it
 * early -- PROCESS_SEAM's own "kill and mean it", never an orphaned
 * process). Exactly one command may be in flight per MixaCmdDispatch;
 * a caller wanting to refuse a second Enter while one is running (this
 * milestone's own choice, see this ticket's own outbox report) simply
 * does not call start() again until the previous handle reports DONE.
 *
 * Non-blocking (PROCESS_SEAM 3, this ticket's own explicit
 * "UI responsiveness/nonblocking behavior"): poll() performs AT MOST
 * ONE real mixa_process_read call and returns immediately either way,
 * mirroring the exact per-tick shape mixa_app_loop_step's own pump
 * drain already uses. It never blocks waiting for output. */

#include <stddef.h>
#include "mixa_manager/mixa_file.h"

typedef struct MixaCmdDispatch MixaCmdDispatch;

#define MIXA_CMD_DISPATCH_OK 0
#define MIXA_CMD_DISPATCH_ERR_ARG 1
#define MIXA_CMD_DISPATCH_ERR_NOMEM 2
#define MIXA_CMD_DISPATCH_ERR_IO 3
#define MIXA_CMD_DISPATCH_ERR_SPAWN 4

#define MIXA_CMD_DISPATCH_RUNNING 0
#define MIXA_CMD_DISPATCH_DONE 1

/* Spawns command_utf8 rooted at cwd_utf8 through the accepted process-
 * marker wrapper, and appends command_utf8 followed by CRLF to
 * console_file immediately (best-effort: a failure to echo does not
 * undo an already-started command, since the command is real and
 * running regardless). On success *out is a new, owned handle; on any
 * failure *out is left null and every resource this call itself
 * acquired is already released (MIXA_CMD_DISPATCH_ERR_SPAWN means the
 * platform shell itself never started, matching mixa_process_spawn's
 * own MIXA_PROC_ERR_SPAWN meaning exactly -- never a command that
 * started and failed, which is an ordinary running/exited child
 * observed through poll() like any other). console_file is BORROWED:
 * this module never closes it. */
int mixa_cmd_dispatch_start(const char *command_utf8, const char *cwd_utf8,
                            MixaFile *console_file, MixaCmdDispatch **out);

/* One non-blocking tick. Drains whatever output is currently available
 * (at most one real read) and appends every ordinary byte to
 * console_file as it is confirmed ordinary, in order -- exactly the
 * already-accepted marker-scanner sink contract. Returns MIXA_CMD_
 * DISPATCH_RUNNING while the command has not yet completed (out_
 * exit_code/out_new_cwd untouched) or MIXA_CMD_DISPATCH_DONE once a
 * complete marker record was recognized OR the child's own output
 * stream is confirmed closed with no marker ever found (PROCESS_SEAM
 * 4.1's own "a caller which finds no marker must treat the command as
 * having produced no status rather than inventing one" -- in that case
 * *out_exit_code is left at 0 and *out_new_cwd at NULL, exactly the
 * "no status" case, never a fabricated success). *out_new_cwd, when
 * non-null, is a freshly allocated string the CALLER owns and frees;
 * either out pointer may be null if the caller does not need it. Once
 * DONE is returned, further poll() calls are a no-op also returning
 * DONE with the SAME already-recorded status (idempotent; never
 * re-reads a closed process). */
int mixa_cmd_dispatch_poll(MixaCmdDispatch *d, int *out_exit_code,
                           char **out_new_cwd);

/* Nonzero while still running (a convenience wrapper a caller can check
 * without calling poll() -- e.g. to decide whether Enter should be
 * accepted this tick). */
int mixa_cmd_dispatch_running(const MixaCmdDispatch *d);

/* Releases everything: kills the child first if it is still running
 * (PROCESS_SEAM's own "kill and mean it" -- a caller must never be able
 * to abandon a live child by simply dropping this handle), then closes
 * the process handle, releases the scanner, and cleans up the wrapper
 * file. Null-safe. Does NOT touch console_file (borrowed, not owned). */
void mixa_cmd_dispatch_release(MixaCmdDispatch *d);

#endif
