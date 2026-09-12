/* mixa_process.h - portable owner-local process seam (PROCESS_SEAM.txt
 * section 3). Six operations: spawn/read/write/status/kill/close.
 *
 * THE CONSOLE IS NOT A TERMINAL (PROCESS_SEAM 2): no PTY, no ConPTY, no
 * escape-sequence interpretation. The command line goes to the platform
 * shell as one string.
 *
 * Handles are opaque and owner-held; the owner calls close() exactly
 * once. read() never blocks (PROCESS_SEAM 3: the loop belongs to the
 * Message, not to this seam). No Message/thread/mutex/queue integration
 * lives here -- PROCESS_SEAM section 6 names that as later work.
 */
#ifndef MIXA_PROCESS_H
#define MIXA_PROCESS_H

#include <stddef.h>

typedef struct MixaProcess MixaProcess;

/* Explicit status: 0 success; nonzero is a failure class, never a silent
 * substitution. ERR_SPAWN means the child never started at all --
 * distinct from a child that started and later exited nonzero, which is
 * read back through status()'s own exit_code, not through spawn()'s
 * return value. */
#define MIXA_PROC_OK          0
#define MIXA_PROC_ERR_ARG     1
#define MIXA_PROC_ERR_SPAWN   2
#define MIXA_PROC_ERR_IO      3
#define MIXA_PROC_ERR_NOMEM   4
/* read() only: the child's output stream is confirmed closed (the child
 * exited and every write handle to the merged stdout+stderr pipe is
 * gone) -- no more bytes will ever arrive. Distinct from MIXA_PROC_OK
 * with *got==0, which means "nothing available right now, try again
 * later" (the child may still be running and quiet). */
#define MIXA_PROC_EOF         5

/* Starts command_line_utf8 with cwd_utf8 as its working directory
 * (required; the working directory is always explicit, per PROCESS_SEAM
 * 4). stdin_enabled selects the exact lifetime PROCESS_SEAM 4/ticket
 * item 2 asks for:
 *   0  the inherited write path is closed once the child is confirmed
 *      started, so a child that reads stdin (e.g. `cat`) sees EOF
 *      immediately rather than hanging forever waiting for input
 *      nobody will send. write() on such a handle always fails
 *      MIXA_PROC_ERR_ARG (there is no live pipe to write into).
 *   nonzero  the write path stays open for explicit write() calls,
 *      exactly as the caller sends them; the caller ends stdin by
 *      calling close() or kill() (there is no separate "close stdin
 *      only" operation -- both end the whole child's lifetime, matching
 *      the seam's own six-operation shape).
 * On success *out holds an owned handle; the caller calls close()
 * exactly once. On MIXA_PROC_ERR_SPAWN the child never started -- *out
 * is left null and every handle this call opened is already released.
 */
int mixa_process_spawn(MixaProcess **out, const char *command_line_utf8,
                        const char *cwd_utf8, int stdin_enabled);

/* Drains whatever of the merged stdout+stderr stream is already
 * available into buf, up to cap bytes. NEVER BLOCKS: a quiet running
 * child returns MIXA_PROC_OK with *got==0 promptly, not after waiting
 * for output. Returns MIXA_PROC_EOF (not MIXA_PROC_OK) once the stream
 * is confirmed closed and drained -- a caller sees this at most once per
 * process's own lifetime, after which further read() calls keep
 * returning MIXA_PROC_EOF with *got==0. */
int mixa_process_read(MixaProcess *p, void *buf, size_t cap, size_t *got);

/* Writes to the child's stdin. *wrote receives the count actually
 * written (may be short); a short write is not itself an error. Fails
 * MIXA_PROC_ERR_ARG if stdin was disabled at spawn() or the write path
 * is already closed (close()/kill() already called). */
int mixa_process_write(MixaProcess *p, const void *buf, size_t n, size_t *wrote);

/* *running is 1 while the child has not exited, 0 once it has.
 * *exit_code is only meaningful when *running==0, and is then the
 * child's own real exit code, preserved verbatim -- never compressed
 * into one generic failure value. */
int mixa_process_status(MixaProcess *p, int *running, int *exit_code);

/* Terminates the child AND every descendant it spawned (a Windows Job
 * Object with kill-on-job-close, or an equivalently proven process-tree
 * mechanism on other platforms) and mean it -- PROCESS_SEAM 3's own
 * wording. Safe to call on an already-exited child (a no-op status-wise,
 * still releases nothing by itself -- call close() afterward). */
int mixa_process_kill(MixaProcess *p);

/* Releases every OS handle this MixaProcess owns. Idempotent: a second
 * call, or a call on a process that failed mid-spawn, is a safe no-op.
 * Null-safe. After return the pointer must not be used. */
void mixa_process_close(MixaProcess *p);

#endif
