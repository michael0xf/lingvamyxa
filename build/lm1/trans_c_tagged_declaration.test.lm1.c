struct LmTagProbeStruct {
    int value;
};

union LmTagProbeUnion {
    int value;
};

enum LmTagProbeEnum {
    LM_TAG_PROBE_OK = 7
};

int main(void) {
    struct LmTagProbeStruct item;
    union LmTagProbeUnion choice;
    enum LmTagProbeEnum kind;
    item.value = 3;
    choice.value = 4;
    kind = LM_TAG_PROBE_OK;
    return item.value != 3 || choice.value != 4 || kind != LM_TAG_PROBE_OK;
}
