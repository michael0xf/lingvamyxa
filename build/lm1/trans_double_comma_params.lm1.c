#include "parser.lm1.h"

static int doubleComma(const LmP0Text *text, const char *value) {
    return text != NULL && value != NULL;
}

int main(void) {
    return doubleComma(NULL, NULL);
}

