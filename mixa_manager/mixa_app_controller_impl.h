#ifndef MIXA_APP_CONTROLLER_IMPL_H
#define MIXA_APP_CONTROLLER_IMPL_H

/* Concrete MixaAppController; opaque to callers of mixa_app_controller.
 * h. Owner holds MixaAppController* and closes it -- the same opaque-
 * handle split mixa_file.h/mixa_file_win32.h and mixa_cmdline_dispatch.
 * h/_impl.h already establish. Included ONLY by mixa_app_controller.
 * lm1.
 *
 * struct MixaAppLoop / MixaConsoleView / MixaConsolePending /
 * MixaFmAllocVTable are referenced here by STRUCT TAG ONLY (never
 * through their own typedef names), exactly the mixa_cmdline_dispatch_
 * impl.h precedent for MixaProcessMarkerScanner: a bare `struct TAG *`
 * field implicitly forward-declares an incomplete struct in this scope,
 * needing no typedef and risking no redeclaration conflict once mixa_
 * app_controller.lm1's own predef of mixa_fm_copy.h.lm1/mixa_console_
 * window.h.lm1 compiles the REAL, complete definitions into the same
 * translation unit.
 */

#include "mixa_manager/mixa_app_controller.h"
#include "mixa_manager/mixa_file.h"
#include "mixa_manager/mixa_help.h"
#include "mixa_manager/mixa_cmdline.h"
#include "mixa_manager/mixa_cmdline_dispatch.h"

struct MixaAppController {
    char *root;
    struct MixaAppLoop *loop;
    MixaCellMetrics *m;
    MixaEvent *ev;
    struct MixaFmAllocVTable *av;
    const MixaBackendVTable *vt;
    char *console_path;
    MixaFile *console_file;
    MixaFile *console_file_append;
    struct MixaConsoleView *view;
    MixaTextRect *text_rect;
    MixaTextRect *upper_rect;
    MixaU8 *rgba;
    size_t width_px;
    size_t height_px;
    size_t rgba_size;
    MixaHelpCtx *help_ctx;
    size_t help_btn_row;
    size_t help_btn_col;
    size_t help_btn_nrows;
    size_t help_btn_ncols;
    MixaCmdLine *cmdline;
    MixaCmdDispatch *dispatch;
    char *cwd_owned;
    const char *cwd_cur;
    struct MixaConsolePending *pending_view;
    int last_exit_code;
};

#endif
