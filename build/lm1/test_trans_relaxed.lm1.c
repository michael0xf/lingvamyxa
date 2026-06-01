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

/* STOP test_trans.lm2:
This file is the first clean L2 slice of trans.lm2: shared structures plus the
text helper functions.

The next helpers in trans.lm2 are still intentionally left for the next pass:

    lm_trans_write_all(FILE *file, const char *data, size_t length)
    lm_trans_put(FILE *file, const char *text)
    lm_trans_nth_field(const LmP0Structure *structure, size_t index)
    namespace functions over namespace_->items[index].field

They require finishing the small C-surface/details around file APIs and
array-element value-member access. Until that is nailed down, this file stays a
compiling checkpoint instead of pretending to be the full translator.
*/

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
    return lm_trans_text_equals(text, "+") || lm_trans_text_equals(text, "-") || lm_trans_text_equals(text, "*") || lm_trans_text_equals(text, "/") || lm_trans_text_equals(text, "%") || lm_trans_text_equals(text, "=") || lm_trans_text_equals(text, "!=") || lm_trans_text_equals(text, "<") || lm_trans_text_equals(text, "<=") || lm_trans_text_equals(text, ">") || lm_trans_text_equals(text, ">=") || lm_trans_text_equals(text, "&&") || lm_trans_text_equals(text, "||") || lm_trans_text_equals(text, "!") || lm_trans_text_equals(text, "[") || lm_trans_text_equals(text, "]");
}

int main(void) {
    return ! lm_trans_text_equals(& lm_test_hello, "hello") || lm_trans_text_same(& lm_test_hello, & lm_test_other) || ! lm_trans_text_starts_with(& lm_test_hello, "he") || lm_trans_text_is_operator_atom(& lm_test_hello);
}

