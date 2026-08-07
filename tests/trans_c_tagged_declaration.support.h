#ifndef LM_TEST_TRANS_C_TAGGED_DECLARATION_SUPPORT_H
#define LM_TEST_TRANS_C_TAGGED_DECLARATION_SUPPORT_H

struct LmTagProbeStruct {
    int value;
};

union LmTagProbeUnion {
    int value;
};

enum LmTagProbeEnum {
    LM_TAG_PROBE_OK = 7
};

#endif
