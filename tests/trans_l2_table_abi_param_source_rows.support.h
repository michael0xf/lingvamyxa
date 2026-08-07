#ifndef LM_TEST_TRANS_L2_TABLE_ABI_PARAM_SOURCE_ROWS_SUPPORT_H
#define LM_TEST_TRANS_L2_TABLE_ABI_PARAM_SOURCE_ROWS_SUPPORT_H

static const char *LmWideAbiParams(const char *text, int count) {
    return text[0] == 'o' && count == 7 ? text : 0;
}

static int LmAbiParamFallback(int value) {
    return value + 1;
}

#endif
