#include <stdio.h>

static int lm_test_counter = 3;

static int lm_test_next(void) {
    return lm_test_counter-- > 0;
}
int main(void) {
    int iterations = 0;
    if (printf("control-call\n") < 0) {
        return 1;
    }
    while (lm_test_next() != 0) {
        iterations = iterations + 1;
    }
    return iterations != 3;
}
