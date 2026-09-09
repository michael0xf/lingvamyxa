#ifndef MIXA_CORE_H
#define MIXA_CORE_H

#include <stddef.h>

/* Style bits for MixaCell.flags. Model section 1 says "style flags" with no
 * list; section 14 names bold, underline and inverse among glyph issues.
 * Only these three are defined here. Optional metadata is intentionally
 * omitted — the model marks it optional, and this header does not invent it.
 */
typedef enum MixaStyle {
    MIXA_STYLE_BOLD = 1,
    MIXA_STYLE_UNDERLINE = 2,
    MIXA_STYLE_REVERSE = 4
} MixaStyle;

typedef struct MixaCell {
    unsigned int codepoint;   /* Unicode scalar; 0 = empty cell */
    unsigned char fg;         /* palette index */
    unsigned char bg;         /* palette index */
    unsigned char flags;      /* MixaStyle bits */
    unsigned char width;      /* 1 or 2, see model section 14 */
    unsigned char alpha;      /* background alpha: 0=transparent, 255=opaque */
} MixaCell;

typedef struct MixaTextRect {
    size_t rows;
    size_t cols;
    MixaCell *cells;          /* rows*cols, ROW-MAJOR, contiguous */
} MixaTextRect;

#endif