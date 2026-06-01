#include "parser.lm1.h"
static int doubleSemicolon(const LmP0Text *text, const char *value) {
    return text != 0 && value != 0;
}

int main(void) {
    return doubleSemicolon(0, 0);
}
