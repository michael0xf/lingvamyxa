#ifndef MIXA_APP_FMPANEL_IMPL_H
#define MIXA_APP_FMPANEL_IMPL_H

/* Concrete MixaAppFmPanel; opaque to callers of mixa_app_fmpanel.h.
 * Included ONLY by mixa_app_fmpanel.lm1 -- the same opaque-handle split
 * mixa_app_controller.h/_impl.h already establish.
 *
 * struct MixaSelection is referenced by STRUCT TAG ONLY (mixa_selection.h
 * is a plain C header with the real, complete definition already
 * available wherever it is include:'d -- referencing it here by tag only
 * keeps this header from needing to include it itself).
 */

#include "mixa_manager/mixa_app_fmpanel.h"

#define MIXA_APP_FMPANEL_PATH_MAX 1024

struct MixaAppFmPanel {
    MixaFm *fm;
    size_t list_row;
    size_t list_col;
    size_t list_rows;
    size_t list_cols;
    size_t action_row;
    size_t action_col;

    /* Raw, buttons-free confirmation surface (see mixa_app_fmpanel.h's
     * own comment for why this does not reuse mixa_app_window). */
    int confirm_open;
    size_t confirm_row;
    size_t confirm_col;
    size_t confirm_rows;
    size_t confirm_cols;
    size_t cancel_row;
    size_t cancel_col;
    size_t cancel_w;
    size_t ok_row;
    size_t ok_col;
    size_t ok_w;
    MixaCell *saved;
    size_t saved_count;
    size_t saved_row;
    size_t saved_col;
    size_t saved_rows;
    size_t saved_cols;

    int invocations;
    int has_failure;
    int last_status;
    char failing_path[MIXA_APP_FMPANEL_PATH_MAX];
};

#endif
