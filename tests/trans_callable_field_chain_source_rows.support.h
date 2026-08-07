#ifndef LM_TEST_TRANS_CALLABLE_FIELD_CHAIN_SOURCE_ROWS_SUPPORT_H
#define LM_TEST_TRANS_CALLABLE_FIELD_CHAIN_SOURCE_ROWS_SUPPORT_H

typedef struct LmFullCallHolder {
    LmUnaryCall callback;
} LmFullCallHolder;

typedef struct LmOverlayCallHolder {
    LmUnaryCall callback;
} LmOverlayCallHolder;

typedef struct LmSplitCallHolder {
    LmUnaryCall callback;
} LmSplitCallHolder;

typedef struct LmWrongDescriptorCallHolder {
    LmUnaryCall callback;
} LmWrongDescriptorCallHolder;

#endif
