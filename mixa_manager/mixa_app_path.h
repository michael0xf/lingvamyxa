#ifndef MIXA_APP_PATH_H
#define MIXA_APP_PATH_H

/* Exact, checked path construction for the real app entrypoint (ticket
 * 20260912-233811). mixa_app_main.lm1 previously copied argv[1] into a
 * fixed char[1040] and formatted the console path into a fixed
 * char[1100] via snprintf -- both SILENTLY TRUNCATING, and both
 * arbitrary capacity constants standing in as semantic limits. This unit
 * replaces them with exact byte storage sized from strlen plus the
 * suffix, with every size addition overflow-checked before allocation.
 *
 * A plain hand-written C header (the same form mixa_process.h /
 * mixa_process_win32.h already use) rather than a .h.lm1 predef, for a
 * measured reason: mixa_app_main.lm1's own comment records that its
 * import path table is already at the limit (the 15-file fm_copy chain
 * plus the console header fits at exactly 16, and both full chains
 * together overflow). An `include:` of a plain C header costs that table
 * nothing, so the entrypoint can call these without giving up the
 * console render hook. The bodies live in mixa_app_path.lm1, compiled as
 * its own translation unit and linked in -- ordinary separate
 * compilation, the same way mixa_console_window.lm1 already reaches this
 * entrypoint.
 *
 * NO path policy of any kind lives here: no normalization, no separator
 * rewriting, no MAX_PATH opinion, no Win32 long-path prefixing. Bytes in
 * are bytes out. FILE_SEAM and the Win32 long-path question are
 * deliberately untouched by this unit.
 */

#include <stddef.h>

#define MIXA_APP_PATH_OK 0
#define MIXA_APP_PATH_ERR_ARG 1
#define MIXA_APP_PATH_ERR_NOMEM 2
#define MIXA_APP_PATH_ERR_OVERFLOW 3

/* Deterministic allocation-fault injection, the same borrowed-vtable
 * idiom already established by mixa_fileio's MixaFioFaultVTable and
 * mixa_process_marker's MixaProcessMarkerFaultVTable. Nullable: pass 0
 * (or a vtable with a null on_alloc) for real, uninjected allocation. A
 * nonzero return forces that ONE attempt to fail with
 * MIXA_APP_PATH_ERR_NOMEM, so the caller's own release path is provable
 * rather than hoped for. */
typedef int (*MixaAppPathFaultFn)(void *ctx, size_t requested_size);

typedef struct MixaAppPathFaultVTable {
    MixaAppPathFaultFn on_alloc;
    void *ctx;
} MixaAppPathFaultVTable;

/* The size arithmetic, split out as its own pure function precisely so
 * the overflow guard is DIRECTLY testable: a real join can never reach
 * the wrap point (it would need a ~SIZE_MAX-byte input string), but this
 * takes the two lengths as values, so SIZE_MAX cases are exercised for
 * real instead of merely asserted in a comment.
 *
 * *out_total receives a_len + b_len + 1 (the NUL) on success. Returns
 * MIXA_APP_PATH_ERR_OVERFLOW if that sum would wrap size_t, checked
 * BEFORE any addition is performed, and MIXA_APP_PATH_ERR_ARG if
 * out_total is null. *out_total is set to 0 on every failure. */
int mixa_app_path_total(size_t a_len, size_t b_len, size_t *out_total);

/* Allocates and returns, in *out, exactly a's bytes followed by b's
 * bytes plus a terminating NUL -- sized from their real strlen, never a
 * fixed capacity, never truncated. The caller owns *out and frees it
 * with free(). *out is set to 0 on every failure path, and nothing is
 * left allocated on any of them.
 *
 * An exact COPY of one string is join(src, "") -- that is the intended
 * use for argv[1], not a separate duplicate-me entry point.
 *
 * fault (nullable, borrowed) is consulted immediately before the one
 * allocation this function performs. */
int mixa_app_path_join(const char *a, const char *b,
                       const MixaAppPathFaultVTable *fault, char **out);

#endif
