/* C ABI: frozen parser trailer-role symbols -> their L2 callables. */
#include "l1src/p0.lm1.h"
#include "l2src/lmx.h"
#include "l2src/lmx_branch_owned.lm1.h"
#include <stddef.h>
#include <stdio.h>

extern Lmx *l2_trailer_unit;
int l2_trailer_boot(void);
int l2_trailer_m1(Lmx *, const char *, size_t, const char *, int);
LmP0TrailerRole l2_trailer_m2(Lmx *, const char *, size_t);
LmP0TrailerRole l2_trailer_m3(Lmx *, const char *);
char *l2_trailer_m4(Lmx *, LmP0TrailerRole);
int l2_trailer_m5(Lmx *, LmP0TrailerRole);
LmP0TrailerRole l2_trailer_m6(Lmx *, const char *, size_t);

int l2_trailer_role_calls = 0;

static Lmx *l2_trailer_own(size_t index) {
    if (l2_trailer_unit == 0) {
        if (l2_trailer_boot() != 0 || l2_trailer_unit == 0) {
            fprintf(stderr, "l2_trailer_boot failed\n");
        }
    }
    l2_trailer_role_calls += 1;
    return lmx_branch_struct_known(l2_trailer_unit, index);
}

int lm_p0_text_has_prefix_name(const char *text, size_t length,
    const char *name, int allow_bare) {
    return l2_trailer_m1(l2_trailer_own(1U), text, length, name, allow_bare);
}

LmP0TrailerRole lm_p0_legacy_trailer_role(const char *text, size_t length) {
    return l2_trailer_m2(l2_trailer_own(2U), text, length);
}

LmP0TrailerRole lm_p0_trailer_role_from_payload(const char *payload) {
    return l2_trailer_m3(l2_trailer_own(3U), payload);
}

const char *lm_p0_trailer_role_payload(LmP0TrailerRole role) {
    return l2_trailer_m4(l2_trailer_own(4U), role);
}

int lm_p0_trailer_role_is_tail_cutter(LmP0TrailerRole role) {
    return l2_trailer_m5(l2_trailer_own(5U), role);
}

LmP0TrailerRole lm_p0_trailer_role(const char *text, size_t length) {
    return l2_trailer_m6(l2_trailer_own(6U), text, length);
}
