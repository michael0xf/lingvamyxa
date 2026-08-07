#ifndef LM_TEST_L2_PREDEF_CONDITIONAL_DESCRIPTOR_SUPPORT_H
#define LM_TEST_L2_PREDEF_CONDITIONAL_DESCRIPTOR_SUPPORT_H

#ifndef LM_TEST_PREDEF_DESCRIPTOR_BRANCH
#define LM_TEST_PREDEF_DESCRIPTOR_BRANCH 0
#endif

#if LM_TEST_PREDEF_DESCRIPTOR_BRANCH
static const void *lm_test_predef_conditional_target(const void *value) {
    return value;
}
#define LM_TEST_PREDEF_DESCRIPTOR_INVOKE(call) ((call)(0) != 0)
#else
static int lm_test_predef_conditional_target(int value) {
    return value + 1;
}
#define LM_TEST_PREDEF_DESCRIPTOR_INVOKE(call) ((call)(41) != 42)
#endif

#endif
