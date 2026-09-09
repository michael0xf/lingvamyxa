#include "hdr_immutable.lm1.h"
int main(void) {
    Box b;
    const char *p;
    b.mut = 1;
    p = b.name;
    b.name = p;
    (void)b.n;
    (void)b.xs[0];
    (void)b.data;
    return b.mut != 1;
}
