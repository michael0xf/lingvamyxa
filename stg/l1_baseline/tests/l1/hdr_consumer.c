#include "hdr_combo.lm1.h"
int main(void) {
    Cell c;
    Rect r;
    CellVisitor v;
    c.codepoint = 0U;
    c.flags = 0;
    r.rows = 0;
    r.cols = 0;
    r.cells = 0;
    v = 0;
    (void)c; (void)r; (void)v;
    return rect_open ? 0 : 0;
}