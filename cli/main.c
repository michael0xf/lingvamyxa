#include <stdio.h>

#include "lm_core.h"

int main(void) {
    printf("%s\n", lm_version());
    printf("lm_add(2, 3) = %d\n", lm_add(2, 3));

    return 0;
}
