/* C ABI: frozen parser Python-string symbols -> their L2 callables. */
#include "l2src/lmx.h"
#include "l2src/lmx_branch_owned.lm1.h"
#include <stddef.h>
#include <stdio.h>

extern Lmx *l2_pystr_unit;
int l2_pystr_boot(void);
int l2_pystr_m1(Lmx *, const char *, size_t, size_t, size_t *);
size_t l2_pystr_m2(Lmx *, const char *, size_t, size_t);

int l2_pystr_calls = 0;

static void l2_pystr_ensure(void) {
    if (l2_pystr_unit == 0) {
        if (l2_pystr_boot() != 0 || l2_pystr_unit == 0) {
            fprintf(stderr, "l2_pystr_boot failed\n");
        }
    }
}

int lm_p0_find_python_string_end(
    const char *text, size_t length, size_t start, size_t *out_end) {
    l2_pystr_ensure();
    l2_pystr_calls += 1;
    return l2_pystr_m1(lmx_branch_struct_known(l2_pystr_unit, 1U),
        text, length, start, out_end);
}

size_t lm_p0_skip_python_string_unchecked(
    const char *text, size_t length, size_t start) {
    l2_pystr_ensure();
    l2_pystr_calls += 1;
    return l2_pystr_m2(lmx_branch_struct_known(l2_pystr_unit, 2U),
        text, length, start);
}
