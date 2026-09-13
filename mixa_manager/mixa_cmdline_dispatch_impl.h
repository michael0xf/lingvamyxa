#ifndef MIXA_CMDLINE_DISPATCH_IMPL_H
#define MIXA_CMDLINE_DISPATCH_IMPL_H

/* Concrete MixaCmdDispatch; opaque to callers of mixa_cmdline_dispatch.h.
 * Owner holds MixaCmdDispatch* and releases it -- the same opaque-handle
 * split mixa_file.h/mixa_file_win32.h already establish. Included ONLY
 * by mixa_cmdline_dispatch.lm1.
 *
 * `struct MixaProcessMarkerScanner` is referenced here by TAG only
 * (never through the typedef name), so this header never needs to see
 * -- or forward-declare -- the typedef itself: a bare `struct TAG *`
 * field implicitly declares an incomplete struct in this scope, which
 * is all a pointer field ever needs. mixa_cmdline_dispatch.lm1's own
 * predef of mixa_process_marker.h.lm1 compiles the REAL, complete
 * `struct MixaProcessMarkerScanner { ... }` into the SAME translation
 * unit (confirmed by translating and compiling this exact combination);
 * every function in that .lm1 file that actually dereferences \field on
 * a scanner sees that complete definition, this header never needs to.
 */

#include "mixa_manager/mixa_cmdline_dispatch.h"
#include "mixa_manager/mixa_process.h"

struct MixaCmdDispatch {
    MixaProcess *proc;
    struct MixaProcessMarkerScanner *scanner;
    MixaFile *console_file;
    char *wrapper_path;
    int done;
    int exit_code;
    char *new_cwd;
};

#endif
