#include <stdlib.h>

struct LmMessageThreadRuntime;

static unsigned lm_test_rest_lmx_install_calls;

static void lm_test_rest_lmx_verify_install(void) {
    if (lm_test_rest_lmx_install_calls != 1U) {
        abort();
    }
}

int lm_rest_lmx_http_client_install_default(
    struct LmMessageThreadRuntime *runtime
) {
    if (runtime == NULL || lm_test_rest_lmx_install_calls != 0U ||
        atexit(lm_test_rest_lmx_verify_install) != 0) {
        return 1;
    }

    lm_test_rest_lmx_install_calls = 1U;
    return getenv("LM_TEST_REST_LMX_INSTALL_FAIL") == NULL ? 0 : 23;
}
