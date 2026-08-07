#ifndef LM_TEST_TRANS_MANAGED_EXTERNAL_THREAD_CONTEXT_SUPPORT_H
#define LM_TEST_TRANS_MANAGED_EXTERNAL_THREAD_CONTEXT_SUPPORT_H

struct LmMessageThread;

int lm_test_imported_managed_external(
    struct LmMessageThread *lm_lmx_message_thread,
    void *expected_thread
);

int lm_test_registry_managed_external(
    struct LmMessageThread *lm_lmx_message_thread,
    void *expected_thread
);

#endif
