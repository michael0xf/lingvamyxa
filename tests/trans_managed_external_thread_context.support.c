#include "trans_managed_external_thread_context.support.h"

int lm_test_imported_managed_external(
    struct LmMessageThread *lm_lmx_message_thread,
    void *expected_thread
) {
    return lm_lmx_message_thread != 0 &&
        lm_lmx_message_thread == expected_thread;
}

int lm_test_registry_managed_external(
    struct LmMessageThread *lm_lmx_message_thread,
    void *expected_thread
) {
    return lm_lmx_message_thread != 0 &&
        lm_lmx_message_thread == expected_thread;
}
