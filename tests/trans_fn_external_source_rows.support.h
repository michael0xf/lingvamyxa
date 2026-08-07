#ifndef LM_TEST_TRANS_FN_EXTERNAL_SOURCE_ROWS_SUPPORT_H
#define LM_TEST_TRANS_FN_EXTERNAL_SOURCE_ROWS_SUPPORT_H

int lm_test_external_direct(int value);
void lm_test_external_fallback(int *value);
void lm_test_external_joined(void);
void lm_test_external_receiver_n2(void);
void lm_test_external_receiver_fallback(void);
int lm_test_external_calls_complete(void);

#endif
