#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.lm1.h"

typedef enum LmTransSymbolKind {
    LM_TRANS_SYMBOL_VARIABLE = 1,
    LM_TRANS_SYMBOL_FUNCTION = 2,
    LM_TRANS_SYMBOL_PROCEDURE = 3,
    LM_TRANS_SYMBOL_LABEL = 4,
    LM_TRANS_SYMBOL_STRUCTURE = 5
} LmTransSymbolKind;

static LmP0Text lm_test_hello = { "hello", 5U };
static LmP0Text lm_test_other = { "other", 5U };

typedef struct LmTransSymbol LmTransSymbol;

struct LmTransSymbol {
    LmP0Text name;
    LmTransSymbolKind kind;
    unsigned depth;
    int has_c_name;
    LmP0Text c_name;
    LmP0Text *param_names;
    size_t param_count;
    int has_signature;
};

typedef struct LmTransCleanup LmTransCleanup;

struct LmTransCleanup {
    unsigned id;
};

typedef struct LmTransLoop LmTransLoop;

struct LmTransLoop {
    size_t cleanup_base;
};

typedef struct LmTransArgumentRange LmTransArgumentRange;

struct LmTransArgumentRange {
    const LmP0Field *first;
    const LmP0Field *stop;
    int present;
};

typedef struct LmTransNamespace LmTransNamespace;

struct LmTransNamespace {
    LmTransSymbol *items;
    size_t count;
    size_t capacity;
    unsigned depth;
    LmTransCleanup *cleanups;
    size_t cleanup_count;
    size_t cleanup_capacity;
    LmTransLoop *loops;
    size_t loop_count;
    size_t loop_capacity;
    unsigned next_cleanup_id;
    const LmP0Node *return_type_node;
    unsigned next_return_id;
};

static int lm_trans_text_equals(const LmP0Text *text, const char *value) {
    size_t length;
    length = strlen(value);
    return text -> length == length && memcmp(text -> data, value, length) == 0;
}

static int lm_trans_text_same(const LmP0Text *left, const LmP0Text *right) {
    if (left -> length != right -> length) {
        return 0;
    }
    if (left -> length == 0) {
        return 1;
    }
    return memcmp(left -> data, right -> data, left -> length) == 0;
}

static int lm_trans_text_starts_with(const LmP0Text *text, const char *prefix) {
    size_t length;
    length = strlen(prefix);
    return text -> length >= length && memcmp(text -> data, prefix, length) == 0;
}

static int lm_trans_text_all_char(const LmP0Text *text, char ch) {
    size_t i = 0;
    if (text -> length == 0) {
        return 0;
    }
    while (i < text -> length) {
        if (text -> data [ i ] != ch) {
            return 0;
        }
        i = i + 1;
    }
    return 1;
}

static int lm_trans_text_is_operator_atom(const LmP0Text *text) {
    return (lm_trans_text_equals(text, "+") ||) (lm_trans_text_equals(text, "-") ||) (lm_trans_text_equals(text, "*") ||) (lm_trans_text_equals(text, "/") ||) (lm_trans_text_equals(text, "%") ||) (lm_trans_text_equals(text, "=") ||) (lm_trans_text_equals(text, "!=") ||) (lm_trans_text_equals(text, "<") ||) (lm_trans_text_equals(text, "<=") ||) (lm_trans_text_equals(text, ">") ||) (lm_trans_text_equals(text, ">=") ||) (lm_trans_text_equals(text, "&&") ||) (lm_trans_text_equals(text, "||") ||) (lm_trans_text_equals(text, "!") ||) (lm_trans_text_equals(text, "[") ||) lm_trans_text_equals(text, "]");
}

static int lm_trans_write_all(FILE *file, const char *bytes, size_t length) {
    if () {
        fwrite bytes = 1 length file;
