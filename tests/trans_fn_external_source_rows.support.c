#include "trans_fn_external_source_rows.support.h"

static int lm_test_external_joined_called;
static int lm_test_external_receiver_n2_called;
static int lm_test_external_receiver_fallback_called;

int lm_test_external_direct(int value) {
    return value + 10;
}

void lm_test_external_fallback(int *value) {
    *value += 20;
}

void lm_test_external_joined(void) {
    lm_test_external_joined_called = 1;
}

void lm_test_external_receiver_n2(void) {
    lm_test_external_receiver_n2_called = 1;
}

void lm_test_external_receiver_fallback(void) {
    lm_test_external_receiver_fallback_called = 1;
}

int lm_test_external_calls_complete(void) {
    return lm_test_external_joined_called &&
        lm_test_external_receiver_n2_called &&
        lm_test_external_receiver_fallback_called;
}
