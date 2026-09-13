/* C ABI: frozen parser physical-line symbols -> their L2 callables. */
#include "l2src/lmx.h"
#include "l2src/lmx_branch_owned.lm1.h"
#include <stddef.h>
#include <stdio.h>

extern Lmx *l2_physical_unit;
int l2_physical_boot(void);
int l2_physical_m2(Lmx *, const char *, size_t);
size_t l2_physical_m3(Lmx *, const char *, size_t, size_t);
int l2_physical_m4(Lmx *, const char *, size_t, size_t);
size_t l2_physical_m6(Lmx *, const char *, size_t, size_t);
size_t l2_physical_m9(Lmx *, const char *, size_t, size_t);
void l2_physical_m11(Lmx *, const char *, size_t, size_t, size_t,
    size_t *, size_t *);

int l2_physical_line_calls = 0;

static void l2_physical_ensure(void) {
    if (l2_physical_unit == 0) {
        if (l2_physical_boot() != 0 || l2_physical_unit == 0) {
            fprintf(stderr, "l2_physical_boot failed\n");
        }
    }
}

int lm_p0_index_is_line_start(const char *text, size_t index) {
    l2_physical_ensure();
    l2_physical_line_calls += 1;
    return l2_physical_m2(lmx_branch_struct_known(l2_physical_unit, 2U),
        text, index);
}

size_t lm_p0_find_physical_line_end(
    const char *source, size_t length, size_t start) {
    l2_physical_ensure();
    l2_physical_line_calls += 1;
    return l2_physical_m3(lmx_branch_struct_known(l2_physical_unit, 3U),
        source, length, start);
}

int lm_p0_line_rest_is_horizontal_space(
    const char *source, size_t start, size_t end) {
    l2_physical_ensure();
    l2_physical_line_calls += 1;
    return l2_physical_m4(lmx_branch_struct_known(l2_physical_unit, 4U),
        source, start, end);
}

size_t lm_p0_count_line_breaks(
    const char *source, size_t start, size_t end) {
    l2_physical_ensure();
    l2_physical_line_calls += 1;
    return l2_physical_m6(lmx_branch_struct_known(l2_physical_unit, 6U),
        source, start, end);
}

size_t lm_p0_visual_column_between(
    const char *source, size_t start, size_t end) {
    l2_physical_ensure();
    l2_physical_line_calls += 1;
    return l2_physical_m9(lmx_branch_struct_known(l2_physical_unit, 9U),
        source, start, end);
}

void lm_p0_advance_layout_line(const char *source, size_t length,
    size_t line_start, size_t line_end, size_t *offset, size_t *line) {
    l2_physical_ensure();
    l2_physical_line_calls += 1;
    l2_physical_m11(lmx_branch_struct_known(l2_physical_unit, 11U),
        source, length, line_start, line_end, offset, line);
}
