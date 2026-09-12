/* mixa_process_win32.h - concrete Win32 process handle; portable API in
 * mixa_process.h. Opaque to callers of that header.
 *
 * A Windows Job Object with JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE is the
 * process-tree mechanism (ticket 20260912-144000 item 3): the child is
 * spawned suspended, assigned to the job BEFORE it ever runs a single
 * instruction (eliminating the classic "child already spawned its own
 * descendant before assignment" race), then resumed. kill() calls
 * TerminateJobObject; closing the job handle also kills everything still
 * in it, so even a caller that only calls close() (skipping kill())
 * cannot leave an orphaned tree running.
 */
#ifndef MIXA_PROCESS_WIN32_H
#define MIXA_PROCESS_WIN32_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "mixa_manager/mixa_process.h"

struct MixaProcess {
    HANDLE h_process;
    HANDLE h_thread;
    HANDLE h_job;
    HANDLE h_out_read;  /* ours: read end of the merged stdout+stderr pipe */
    HANDLE h_in_write;  /* ours: write end of the stdin pipe; NULL if disabled or closed */
    int stdin_enabled;
    int eof_seen;
    int is_closed;
};

#endif
