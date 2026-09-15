/* C ABI: frozen parser layout comparison -> its L2 callable. */
#include "l2src/lmx.h"
#include "l2src/lmx_branch_owned.lm1.h"
#include <stddef.h>
#include <stdio.h>

extern Lmx *l2_deeper_unit;
int l2_deeper_boot(void);
int l2_deeper_m0(Lmx *, size_t, size_t, size_t, size_t);

int l2_layout_deeper_calls = 0;

int lm_p0_layout_prefix_is_deeper(size_t indent_column, size_t dot_level,
    size_t base_indent_column, size_t base_dot_level) {
    if (l2_deeper_unit == 0) {
        if (l2_deeper_boot() != 0 || l2_deeper_unit == 0) {
            fprintf(stderr, "l2_deeper_boot failed\n");
        }
    }
    l2_layout_deeper_calls += 1;
    return l2_deeper_m0(lmx_branch_struct_known(l2_deeper_unit, 0U),
        indent_column, dot_level, base_indent_column, base_dot_level);
}
