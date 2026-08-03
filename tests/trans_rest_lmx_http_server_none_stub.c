#include <stddef.h>
#include <stdlib.h>

struct LmMessageThreadRuntime;
typedef struct LmRestLmxHttpServer LmRestLmxHttpServer;

static unsigned lm_test_server_none_start_calls;
static unsigned lm_test_server_none_stop_calls;

static void lm_test_server_none_verify(void) {
    if (lm_test_server_none_start_calls != 1U ||
        lm_test_server_none_stop_calls != 0U) {
        abort();
    }
}

int lm_rest_lmx_http_server_start_default(
    struct LmMessageThreadRuntime *runtime,
    LmRestLmxHttpServer **out_server
) {
    if (runtime == NULL || out_server == NULL || *out_server != NULL ||
        lm_test_server_none_start_calls != 0U ||
        atexit(lm_test_server_none_verify) != 0) {
        return 1;
    }
    lm_test_server_none_start_calls = 1U;
    return 0;
}

int lm_rest_lmx_http_server_stop(LmRestLmxHttpServer **server) {
    (void)server;
    lm_test_server_none_stop_calls += 1U;
    return 1;
}
