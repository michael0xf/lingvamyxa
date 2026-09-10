#ifndef MIXA_BUTTONS_H
#define MIXA_BUTTONS_H

#include <stddef.h>
#include "mixa_manager/mixa_core.h"
#include "mixa_manager/mixa_tiles.h"

/* Application button panel — ClearShell freshLayout / normalize rules
 * (UI_MODEL.txt section 5). Sizes in CELLS. Micro-gap is a tile concern.
 */

#define MIXA_BUTTONS_OK   0
#define MIXA_BUTTONS_ERR  1
#define MIXA_BUTTON_NONE  (-1)

typedef struct MixaButton {
    char *label;       /* owned copy; NULL if separator */
    size_t row;        /* content-cell origin after layout */
    size_t col;
    size_t nrows;      /* = panel hb */
    size_t ncols;      /* label-driven width */
    unsigned fg;
    unsigned bg;
    unsigned alpha;
    int is_sep;        /* non-zero: separator, advances x by hb>>1 */
    int enlarge;       /* non-zero: label length < 3 on square path */
} MixaButton;

typedef struct MixaButtonLine {
    MixaButton *items;
    size_t count;
    size_t cap;
} MixaButtonLine;

typedef struct MixaButtonPanel {
    MixaButtonLine *lines;
    size_t line_count;
    size_t line_cap;
    size_t hb;          /* button height in cells */
    size_t wb;          /* (hb<<1)-(hb>>2) */
    size_t margin;      /* hb>>5 cell gap between buttons (may be 0) */
    size_t panel_w;     /* content maxima after layout */
    size_t panel_h;
    size_t origin_row;  /* panel position on the surface (cells) */
    size_t origin_col;
    size_t view_w;      /* visible width; overflow scrolls horizontally */
    size_t scroll_x;    /* horizontal scroll in cells */
    int is_open;
} MixaButtonPanel;

int mixa_button_panel_init(MixaButtonPanel *panel, size_t hb);
void mixa_button_panel_release(MixaButtonPanel *panel);

/* Label-driven width (UI_MODEL 5.2 / ClearShell normalize). */
size_t mixa_button_width(size_t hb, size_t wb, const char *label);

/* Append a button on line (0-based). Creates the line if needed. */
int mixa_button_panel_add(MixaButtonPanel *panel, size_t line,
                          const char *label, unsigned fg, unsigned bg,
                          unsigned alpha);

/* Append a null-separator on line. */
int mixa_button_panel_add_sep(MixaButtonPanel *panel, size_t line);

/* Run freshLayout: positions, panel_w/panel_h from maxima. */
int mixa_button_panel_layout(MixaButtonPanel *panel);

/* Hit-test surface cell coords -> button index among non-separators
 * (0..n-1 in add order), or MIXA_BUTTON_NONE. Gap / outside -> NONE.
 */
int mixa_button_panel_hit(const MixaButtonPanel *panel,
                          size_t cell_row, size_t cell_col);

/* Draw all non-sep buttons' tiles into rect at origin + layout - scroll. */
int mixa_button_panel_draw(const MixaButtonPanel *panel, MixaTextRect *rect);

/* Count non-separator buttons (hit-test index domain). */
size_t mixa_button_panel_count(const MixaButtonPanel *panel);

#endif
