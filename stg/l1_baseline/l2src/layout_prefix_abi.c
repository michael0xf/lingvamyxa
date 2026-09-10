/* C ABI: frozen lm_p0_scan_layout_prefix -> L2 l2_layout_m3. */
#include "l2src/lmx.h"
#include <stdio.h>

extern Lmx *l2_layout_unit;
int l2_layout_boot(void);
void l2_layout_m3(Lmx *node, const char *source, size_t length, size_t start,
    size_t *out_offset, size_t *out_indent_column, size_t *out_dot_level);

int l2_layout_prefix_calls = 0;

static void l2_layout_ensure(void) {
    if (l2_layout_unit == 0) {
        if (l2_layout_boot() != 0 || l2_layout_unit == 0) {
            fprintf(stderr, "l2_layout_boot failed\n");
        }
    }
}

void lm_p0_scan_layout_prefix(const char *source, size_t length, size_t start,
    size_t *out_offset, size_t *out_indent_column, size_t *out_dot_level) {
    l2_layout_ensure();
    l2_layout_prefix_calls += 1;
    l2_layout_m3(l2_layout_unit, source, length, start, out_offset, out_indent_column, out_dot_level);
}
