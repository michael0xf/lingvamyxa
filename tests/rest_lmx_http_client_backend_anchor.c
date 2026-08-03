#include <stdio.h>
#include <string.h>

typedef struct LmMessageThreadRuntime LmMessageThreadRuntime;

#ifndef LM_REST_LMX_CLIENT_PROVIDER_NONE
#define LM_REST_LMX_CLIENT_PROVIDER_NONE 0
#endif
#ifndef LM_REST_LMX_CLIENT_PROVIDER_LIBCURL
#define LM_REST_LMX_CLIENT_PROVIDER_LIBCURL 1
#endif
#ifndef LM_REST_LMX_CLIENT_PROVIDER_WINHTTP
#define LM_REST_LMX_CLIENT_PROVIDER_WINHTTP 2
#endif
#ifndef LM_REST_LMX_CLIENT_PROVIDER
#define LM_REST_LMX_CLIENT_PROVIDER LM_REST_LMX_CLIENT_PROVIDER_NONE
#endif

const char *lm_rest_lmx_http_client_provider_name(void);
int lm_rest_lmx_http_client_install_default(LmMessageThreadRuntime *runtime);
LmMessageThreadRuntime *lm_message_thread_runtime_new(void);
int lm_message_thread_runtime_delete(LmMessageThreadRuntime *runtime);

static const char *lm_rest_lmx_expected_provider(void) {
#if LM_REST_LMX_CLIENT_PROVIDER == LM_REST_LMX_CLIENT_PROVIDER_LIBCURL
    return "libcurl";
#elif LM_REST_LMX_CLIENT_PROVIDER == LM_REST_LMX_CLIENT_PROVIDER_WINHTTP
    return "winhttp";
#else
    return "none";
#endif
}

int main(void) {
    LmMessageThreadRuntime *runtime = lm_message_thread_runtime_new();
    int status = 0;

    if (runtime == 0 || strcmp(
            lm_rest_lmx_http_client_provider_name(),
            lm_rest_lmx_expected_provider()
        ) != 0 || lm_rest_lmx_http_client_install_default(runtime) != 0) {
        status = 1;
    }
    if (runtime != 0 && lm_message_thread_runtime_delete(runtime) != 0 &&
        status == 0) {
        status = 2;
    }
    if (status != 0) {
        fprintf(stderr, "REST/LMX backend anchor failed: %d\n", status);
    }
    return status;
}
