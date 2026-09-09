#include "hdr_immutable.lm1.h"
int main(void) {
    Box b = {7, "ok", {3, 4}, "ab", 1};
    const char *p;
    if (b.n != 7) {
        return 1;
    }
    if (b.name == 0 || b.name[0] != 'o' || b.name[1] != 'k' || b.name[2] != 0) {
        return 2;
    }
    if (b.xs[0] != 3 || b.xs[1] != 4) {
        return 3;
    }
    if (b.data == 0 || b.data[0] != 'a' || b.data[1] != 'b') {
        return 4;
    }
    if (b.mut != 1) {
        return 5;
    }
    b.mut = 9;
    if (b.mut != 9) {
        return 6;
    }
    p = "zz";
    b.name = p;
    if (b.name == 0 || b.name[0] != 'z') {
        return 7;
    }
    return 0;
}
