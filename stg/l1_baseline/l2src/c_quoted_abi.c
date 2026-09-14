/* C ABI: frozen parser quoted-token symbols -> their L2 callables. */
#include "l2src/lmx.h"
#include "l2src/lmx_branch_owned.lm1.h"
#include <stddef.h>
#include <stdio.h>

extern Lmx *l2_cquoted_unit;
int l2_cquoted_boot(void);
size_t l2_cquoted_m2(Lmx *, const char *, size_t, size_t);
int l2_cquoted_m3(Lmx *, const char *, size_t, size_t);
size_t l2_cquoted_m4(Lmx *, const char *, size_t, size_t);
size_t l2_cquoted_m5(Lmx *, const char *, size_t, size_t);

int l2_cquoted_calls = 0;

static void l2_cquoted_ensure(void) {
    if (l2_cquoted_unit == 0) {
        if (l2_cquoted_boot() != 0 || l2_cquoted_unit == 0) {
            fprintf(stderr, "l2_cquoted_boot failed\n");
        }
    }
}

size_t lm_p0_scan_c_quoted_token(
    const char *text, size_t end_index, size_t quote_index) {
    l2_cquoted_ensure();
    l2_cquoted_calls += 1;
    return l2_cquoted_m2(lmx_branch_struct_known(l2_cquoted_unit, 2U),
        text, end_index, quote_index);
}

int lm_p0_starts_c_prefixed_quote(
    const char *text, size_t end_index, size_t start) {
    l2_cquoted_ensure();
    l2_cquoted_calls += 1;
    return l2_cquoted_m3(lmx_branch_struct_known(l2_cquoted_unit, 3U),
        text, end_index, start);
}

size_t lm_p0_scan_c_char_token(
    const char *text, size_t end_index, size_t start) {
    l2_cquoted_ensure();
    l2_cquoted_calls += 1;
    return l2_cquoted_m4(lmx_branch_struct_known(l2_cquoted_unit, 4U),
        text, end_index, start);
}

size_t lm_p0_scan_c_prefixed_quote_token(
    const char *text, size_t end_index, size_t start) {
    l2_cquoted_ensure();
    l2_cquoted_calls += 1;
    return l2_cquoted_m5(lmx_branch_struct_known(l2_cquoted_unit, 5U),
        text, end_index, start);
}
