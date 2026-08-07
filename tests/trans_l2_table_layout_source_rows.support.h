#ifndef LM_TEST_TRANS_L2_TABLE_LAYOUT_SOURCE_ROWS_SUPPORT_H
#define LM_TEST_TRANS_L2_TABLE_LAYOUT_SOURCE_ROWS_SUPPORT_H

typedef struct Plain {
    int value;
} Plain;

typedef struct Qualified {
    const char *text;
} Qualified;

typedef struct Partial {
    int value;
} Partial;

typedef struct Overlay {
    int value;
} Overlay;

typedef struct SplitOnly {
    int flag;
} SplitOnly;

typedef struct WrongSchema {
    int value;
} WrongSchema;

#endif
