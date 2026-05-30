#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "own.lm1.h"
#include "parser.lm1.h"

typedef enum LmTransSymbolKind {
    LM_TRANS_SYMBOL_VARIABLE = 1,
    LM_TRANS_SYMBOL_FUNCTION = 2,
    LM_TRANS_SYMBOL_PROCEDURE = 3,
    LM_TRANS_SYMBOL_LABEL = 4,
    LM_TRANS_SYMBOL_STRUCTURE = 5
} LmTransSymbolKind;

typedef struct LmTransRegistryRow {
    char *table;
    char *key;
    char *payload;
} LmTransRegistryRow;

typedef struct LmTransRegistry {
    LmOwnPtrStack rows;
    int loaded;
} LmTransRegistry;

static LmTransRegistry lm_trans_registry;

typedef struct LmTransSymbol {
    LmP0Text *name;
    LmTransSymbolKind kind;
    unsigned depth;
    LmOwnPtrStack param_names;
    int has_signature;
} LmTransSymbol;

typedef struct LmTransCleanup {
    unsigned id;
} LmTransCleanup;

typedef struct LmTransLoop {
    size_t cleanup_base;
} LmTransLoop;

typedef struct LmTransExprSegment {
    const LmP0Field *first;
    const LmP0Field *stop;
    int present;
} LmTransExprSegment;

typedef enum LmTransCallLoweringMode {
    LM_TRANS_CALL_LOWER_VALUE = 1,
    LM_TRANS_CALL_LOWER_STATEMENT = 2
} LmTransCallLoweringMode;

typedef struct LmTransCallLowering {
    LmP0Text name;
    const LmTransSymbol *signature;
} LmTransCallLowering;

typedef enum LmTransExprAtomLoweringKind {
    LM_TRANS_EXPR_ATOM_LOWER_VALUE = 1,
    LM_TRANS_EXPR_ATOM_LOWER_FIELD_NAME = 2,
    LM_TRANS_EXPR_ATOM_LOWER_C_FIELD_NAME = 3,
    LM_TRANS_EXPR_ATOM_LOWER_C_DOT = 4,
    LM_TRANS_EXPR_ATOM_LOWER_EQUAL = 5,
    LM_TRANS_EXPR_ATOM_LOWER_ADDRESS = 6,
    LM_TRANS_EXPR_ATOM_LOWER_DEREF = 7,
    LM_TRANS_EXPR_ATOM_LOWER_POINTER_FOLLOW = 8,
    LM_TRANS_EXPR_ATOM_LOWER_INDEX_OPERATOR = 9,
    LM_TRANS_EXPR_ATOM_LOWER_INFIX_OPERATOR = 10,
    LM_TRANS_EXPR_ATOM_LOWER_C_SURFACE = 11
} LmTransExprAtomLoweringKind;

typedef struct LmTransExprAtomLowering {
    LmTransExprAtomLoweringKind kind;
    LmP0Text text;
} LmTransExprAtomLowering;

typedef enum LmTransExprPieceKind {
    LM_TRANS_EXPR_PIECE_ATOM = 1,
    LM_TRANS_EXPR_PIECE_NODE = 2,
    LM_TRANS_EXPR_PIECE_INDEX = 3
} LmTransExprPieceKind;

typedef struct LmTransExprPiece {
    LmTransExprPieceKind kind;
    int leading_space;
    const LmP0Node *node;
    LmTransExprAtomLowering atom;
    const LmP0Field *first;
    const LmP0Field *stop;
} LmTransExprPiece;

typedef struct LmTransExprLoweredRange {
    LmOwnValueStack pieces;
    size_t index;
} LmTransExprLoweredRange;

typedef enum LmTransExprJobKind {
    LM_TRANS_EXPR_JOB_TEXT = 1,
    LM_TRANS_EXPR_JOB_NODE = 2,
    LM_TRANS_EXPR_JOB_FRAME = 3,
    LM_TRANS_EXPR_JOB_RANGE = 4,
    LM_TRANS_EXPR_JOB_CALL_ARGS = 5,
    LM_TRANS_EXPR_JOB_LOWERED_RANGE = 6
} LmTransExprJobKind;

typedef struct LmTransExprRangeJob {
    const LmP0Field *field;
    const LmP0Field *stop;
    int wrote;
    const LmP0Node *previous_operand;
    int expect_field_name;
    int expect_c_field_name;
    int c_dot_path;
} LmTransExprRangeJob;

typedef struct LmTransExprCallArgsJob {
    const LmP0Structure *body;
    const LmTransSymbol *callee;
} LmTransExprCallArgsJob;

typedef struct LmTransExprJob {
    LmTransExprJobKind kind;
    union {
        const char *text;
        const LmP0Node *node;
        const LmP0Frame *frame;
        LmTransExprRangeJob range;
        LmTransExprCallArgsJob call_args;
        LmTransExprLoweredRange *lowered_range;
    } as;
} LmTransExprJob;

typedef struct LmTransExprStack {
    LmOwnValueStack jobs;
} LmTransExprStack;

typedef enum LmTransStatementJobKind {
    LM_TRANS_STATEMENT_JOB_LIST = 1,
    LM_TRANS_STATEMENT_JOB_NODE = 2,
    LM_TRANS_STATEMENT_JOB_VALIDATE_END = 3,
    LM_TRANS_STATEMENT_JOB_TRAILER = 4,
    LM_TRANS_STATEMENT_JOB_LEAVE_SCOPE = 5,
    LM_TRANS_STATEMENT_JOB_LOOP_POP = 6,
    LM_TRANS_STATEMENT_JOB_CLEANUP_POP = 7,
    LM_TRANS_STATEMENT_JOB_INDENT_TEXT = 8,
    LM_TRANS_STATEMENT_JOB_SYNC_LEAVE = 9
} LmTransStatementJobKind;

typedef struct LmTransStatementListJob {
    const LmP0Field *field;
    unsigned indent;
    int unwrap_single_structure;
} LmTransStatementListJob;

typedef struct LmTransStatementNodeJob {
    const LmP0Node *node;
    unsigned indent;
} LmTransStatementNodeJob;

typedef struct LmTransStatementFrameJob {
    const LmP0Frame *frame;
    unsigned indent;
} LmTransStatementFrameJob;

typedef struct LmTransStatementTextJob {
    unsigned indent;
    const char *text;
} LmTransStatementTextJob;

typedef struct LmTransStatementSyncLeaveJob {
    unsigned indent;
    unsigned cleanup_id;
} LmTransStatementSyncLeaveJob;

typedef struct LmTransStatementJob {
    LmTransStatementJobKind kind;
    union {
        LmTransStatementListJob list;
        LmTransStatementNodeJob node;
        LmTransStatementFrameJob frame;
        LmTransStatementTextJob text;
        LmTransStatementSyncLeaveJob sync_leave;
    } as;
} LmTransStatementJob;

typedef struct LmTransStatementStack {
    LmOwnValueStack jobs;
} LmTransStatementStack;

typedef enum LmTransStatementLoweringKind {
    LM_TRANS_STATEMENT_LOWER_RETURN = 1,
    LM_TRANS_STATEMENT_LOWER_IF = 2,
    LM_TRANS_STATEMENT_LOWER_WHILE = 3,
    LM_TRANS_STATEMENT_LOWER_ELSE = 4,
    LM_TRANS_STATEMENT_LOWER_SYNCHRONIZED = 5,
    LM_TRANS_STATEMENT_LOWER_BREAK = 6,
    LM_TRANS_STATEMENT_LOWER_CONTINUE = 7,
    LM_TRANS_STATEMENT_LOWER_CONST_DECLARATION = 8,
    LM_TRANS_STATEMENT_LOWER_ARRAY_DECLARATION = 9,
    LM_TRANS_STATEMENT_LOWER_STORAGE_DECLARATION = 10,
    LM_TRANS_STATEMENT_LOWER_POINTER_DECLARATION = 11,
    LM_TRANS_STATEMENT_LOWER_TARGET_ASSIGNMENT = 12,
    LM_TRANS_STATEMENT_LOWER_LABEL = 13,
    LM_TRANS_STATEMENT_LOWER_CALL = 14,
    LM_TRANS_STATEMENT_LOWER_ASSIGNMENT = 15
} LmTransStatementLoweringKind;

typedef struct LmTransStatementLowering {
    LmTransStatementLoweringKind kind;
} LmTransStatementLowering;

typedef enum LmTransAtomStatementLoweringKind {
    LM_TRANS_ATOM_STATEMENT_LOWER_BREAK = 1,
    LM_TRANS_ATOM_STATEMENT_LOWER_CONTINUE = 2,
    LM_TRANS_ATOM_STATEMENT_LOWER_RETURN = 3,
    LM_TRANS_ATOM_STATEMENT_LOWER_EXPR = 4,
    LM_TRANS_ATOM_STATEMENT_LOWER_STRING_ERROR = 5
} LmTransAtomStatementLoweringKind;

typedef struct LmTransFunctionState {
    const LmP0Node *previous_return_type_node;
    int previous_return_type_is_struct;
    LmP0Text *previous_return_type_name;
    LmP0Text *current_return_type_name;
    unsigned previous_next_return_id;
    LmOwnPtrStack previous_cleanups;
    LmOwnPtrStack previous_loops;
    int has_previous_control_stacks;
} LmTransFunctionState;

typedef enum LmTransTopLevelItemKind {
    LM_TRANS_TOP_LEVEL_ITEM_NONE = 0,
    LM_TRANS_TOP_LEVEL_ITEM_FUNCTION = 1,
    LM_TRANS_TOP_LEVEL_ITEM_NAMED_STRUCTURE = 2,
    LM_TRANS_TOP_LEVEL_ITEM_L1 = 3
} LmTransTopLevelItemKind;

typedef struct LmTransFunctionHeader {
    const LmP0Frame *frame;
    LmP0Text name;
    const LmP0Node *params_node;
    const LmP0Node *return_node;
    const LmP0Field *body_start;
    LmTransSymbolKind symbol_kind;
    int is_sub;
    int is_struct_return;
    int is_external;
} LmTransFunctionHeader;

typedef struct LmTransTopLevelItem {
    LmTransTopLevelItemKind kind;
    const LmP0Frame *frame;
    LmTransFunctionHeader function;
} LmTransTopLevelItem;

typedef struct LmTransNamespace {
    LmOwnPtrStack items;
    unsigned depth;
    LmOwnPtrStack cleanups;
    LmOwnPtrStack loops;
    unsigned next_cleanup_id;
    const LmP0Node *return_type_node;
    int return_type_is_struct;
    LmP0Text *return_type_name;
    unsigned next_return_id;
} LmTransNamespace;

static int lm_trans_text_equals(LmP0Text text, const char *value) {
    size_t length;

    length = strlen(value);
    return text.length == length && memcmp(text.data, value, length) == 0;
}

static int lm_trans_text_same(LmP0Text left, LmP0Text right) {
    if (left.length != right.length) {
        return 0;
    }
    if (left.length == 0U) {
        return 1;
    }
    return memcmp(left.data, right.data, left.length) == 0;
}

static int lm_trans_text_starts_with(LmP0Text text, const char *prefix) {
    size_t length;

    length = strlen(prefix);
    return text.length >= length && memcmp(text.data, prefix, length) == 0;
}

static char *lm_trans_text_copy_cstr(LmP0Text text) {
    char *copy;

    copy = (char *)malloc(text.length + 1U);
    if (copy == 0) {
        return 0;
    }
    if (text.length > 0U) {
        memcpy(copy, text.data, text.length);
    }
    copy[text.length] = '\0';
    return copy;
}

static int lm_trans_registry_atom_payload(LmP0Text atom, LmP0Text *out_payload) {
    char quote;

    if (out_payload == 0) {
        return 0;
    }

    *out_payload = atom;
    if (atom.length < 2U) {
        return 1;
    }

    quote = atom.data[0];
    if (
        (quote == '`' || quote == '"' || quote == '\'') &&
        atom.data[atom.length - 1U] == quote
    ) {
        out_payload->data = atom.data + 1U;
        out_payload->length = atom.length - 2U;
    }

    return 1;
}

static char *lm_trans_registry_atom_copy_cstr(LmP0Text atom) {
    LmP0Text payload;

    if (!lm_trans_registry_atom_payload(atom, &payload)) {
        return 0;
    }
    return lm_trans_text_copy_cstr(payload);
}

static void lm_trans_registry_row_destroy_fields(LmTransRegistryRow *row) {
    if (row != 0) {
        free(row->table);
        free(row->key);
        free(row->payload);
        row->table = 0;
        row->key = 0;
        row->payload = 0;
    }
}

static void lm_trans_registry_row_destroy_any(void *object) {
    LmTransRegistryRow *row;

    row = (LmTransRegistryRow *)object;
    lm_trans_registry_row_destroy_fields(row);
    lm_own_delete(row, 0);
}

static int lm_trans_registry_push_row_atoms(
    LmP0Text table_atom,
    LmP0Text key_atom,
    LmP0Text payload_atom
) {
    LmTransRegistryRow *row;

    row = (LmTransRegistryRow *)lm_own_new_zero(sizeof(*row));
    if (row == 0) {
        return -1;
    }

    row->table = lm_trans_registry_atom_copy_cstr(table_atom);
    row->key = lm_trans_registry_atom_copy_cstr(key_atom);
    row->payload = lm_trans_registry_atom_copy_cstr(payload_atom);
    if (row->table == 0 || row->key == 0 || row->payload == 0) {
        lm_trans_registry_row_destroy_any(row);
        return -1;
    }

    if (lm_own_ptr_stack_push(&lm_trans_registry.rows, row) != 0) {
        lm_trans_registry_row_destroy_any(row);
        return -1;
    }

    return 0;
}

static const char *lm_trans_registry_lookup(LmP0Text key, const char *table) {
    size_t i;
    LmTransRegistryRow *row;

    if (table == 0) {
        return 0;
    }

    i = lm_trans_registry.rows.count;
    while (i > 0U) {
        --i;
        row = (LmTransRegistryRow *)lm_own_ptr_stack_at(&lm_trans_registry.rows, i);
        if (
            row != 0 &&
            row->table != 0 &&
            row->key != 0 &&
            strcmp(row->table, table) == 0 &&
            lm_trans_text_equals(key, row->key)
        ) {
            return row->payload;
        }
    }

    return 0;
}

static int lm_trans_registry_has(LmP0Text key, const char *table) {
    return lm_trans_registry_lookup(key, table) != 0;
}

static int lm_trans_registry_has_prefix(LmP0Text text, const char *table) {
    size_t i;
    LmTransRegistryRow *row;

    if (table == 0) {
        return 0;
    }

    i = lm_trans_registry.rows.count;
    while (i > 0U) {
        --i;
        row = (LmTransRegistryRow *)lm_own_ptr_stack_at(&lm_trans_registry.rows, i);
        if (
            row != 0 &&
            row->table != 0 &&
            row->key != 0 &&
            strcmp(row->table, table) == 0 &&
            lm_trans_text_starts_with(text, row->key)
        ) {
            return 1;
        }
    }

    return 0;
}

static int lm_trans_registry_compare_enabled(void) {
    const char *value;
    static int initialized = 0;
    static int enabled = 1;

    if (!initialized) {
        value = getenv("LM_TRANS_COMPARE_REGISTRY");
        enabled = value == 0 || strcmp(value, "0") != 0;
        initialized = 1;
    }

    return enabled;
}

static const char *lm_trans_payload_text(const char *payload) {
    return payload != 0 ? payload : "<none>";
}

static void lm_trans_registry_compare_fail(
    const char *table,
    LmP0Text key,
    const char *registry_payload,
    const char *legacy_payload
) {
    fprintf(
        stderr,
        "trans registry mismatch: table=%s key=\"%.*s\" registry=%s legacy=%s\n",
        table != 0 ? table : "<none>",
        (int)key.length,
        key.data,
        lm_trans_payload_text(registry_payload),
        lm_trans_payload_text(legacy_payload)
    );
    exit(2);
}

static const char *lm_trans_registry_lookup_compare(
    LmP0Text key,
    const char *table,
    const char *legacy_payload
) {
    const char *registry_payload;

    registry_payload = lm_trans_registry_lookup(key, table);
    if (
        lm_trans_registry_compare_enabled() &&
        (
            (registry_payload == 0 && legacy_payload != 0) ||
            (registry_payload != 0 && legacy_payload == 0) ||
            (registry_payload != 0 && legacy_payload != 0 && strcmp(registry_payload, legacy_payload) != 0)
        )
    ) {
        lm_trans_registry_compare_fail(table, key, registry_payload, legacy_payload);
    }

    return registry_payload;
}

static int lm_trans_registry_has_compare(
    LmP0Text key,
    const char *table,
    int legacy_result
) {
    const char *registry_payload;
    int registry_result;

    registry_payload = lm_trans_registry_lookup(key, table);
    registry_result = registry_payload != 0;
    if (lm_trans_registry_compare_enabled() && registry_result != legacy_result) {
        lm_trans_registry_compare_fail(table, key, registry_payload, legacy_result ? "<present>" : 0);
    }
    return registry_result;
}

static int lm_trans_bool_compare(
    const char *table,
    LmP0Text key,
    int registry_result,
    int legacy_result
) {
    if (lm_trans_registry_compare_enabled() && registry_result != legacy_result) {
        lm_trans_registry_compare_fail(
            table,
            key,
            registry_result ? "<present>" : 0,
            legacy_result ? "<present>" : 0
        );
    }
    return registry_result;
}

static const char *lm_trans_legacy_c_spelling(LmP0Text name) {
    if (lm_trans_text_equals(name, "u8")) {
        return "uint8_t";
    }
    if (lm_trans_text_equals(name, "u16")) {
        return "uint16_t";
    }
    if (lm_trans_text_equals(name, "u32")) {
        return "uint32_t";
    }
    if (lm_trans_text_equals(name, "u64")) {
        return "uint64_t";
    }
    if (lm_trans_text_equals(name, "i8")) {
        return "int8_t";
    }
    if (lm_trans_text_equals(name, "i16")) {
        return "int16_t";
    }
    if (lm_trans_text_equals(name, "i32")) {
        return "int32_t";
    }
    if (lm_trans_text_equals(name, "i64")) {
        return "int64_t";
    }

    return 0;
}

static int lm_trans_legacy_builtin_c_type_tail(LmP0Text name) {
    return
        lm_trans_legacy_c_spelling(name) != 0 ||
        lm_trans_text_equals(name, "void") ||
        lm_trans_text_equals(name, "char") ||
        lm_trans_text_equals(name, "short") ||
        lm_trans_text_equals(name, "int") ||
        lm_trans_text_equals(name, "long") ||
        lm_trans_text_equals(name, "float") ||
        lm_trans_text_equals(name, "double") ||
        lm_trans_text_equals(name, "signed") ||
        lm_trans_text_equals(name, "unsigned") ||
        lm_trans_text_equals(name, "size_t") ||
        lm_trans_text_equals(name, "FILE");
}

static int lm_trans_legacy_text_is_operator_atom(LmP0Text text) {
    return
        lm_trans_text_equals(text, "++") ||
        lm_trans_text_equals(text, "--") ||
        lm_trans_text_equals(text, "+") ||
        lm_trans_text_equals(text, "-") ||
        lm_trans_text_equals(text, "*") ||
        lm_trans_text_equals(text, "/") ||
        lm_trans_text_equals(text, "%") ||
        lm_trans_text_equals(text, "=") ||
        lm_trans_text_equals(text, "!=") ||
        lm_trans_text_equals(text, "<") ||
        lm_trans_text_equals(text, "<=") ||
        lm_trans_text_equals(text, ">") ||
        lm_trans_text_equals(text, ">=") ||
        lm_trans_text_equals(text, "&&") ||
        lm_trans_text_equals(text, "||") ||
        lm_trans_text_equals(text, "&") ||
        lm_trans_text_equals(text, "|") ||
        lm_trans_text_equals(text, "^") ||
        lm_trans_text_equals(text, "!") ||
        lm_trans_text_equals(text, "~") ||
        lm_trans_text_equals(text, "@") ||
        lm_trans_text_equals(text, "\\") ||
        lm_trans_text_equals(text, "[") ||
        lm_trans_text_equals(text, "]");
}

static int lm_trans_legacy_is_reserved_head_name(LmP0Text name) {
    return
        lm_trans_text_equals(name, "L1") ||
        lm_trans_text_equals(name, "L2") ||
        lm_trans_text_equals(name, "fn") ||
        lm_trans_text_equals(name, "fm") ||
        lm_trans_text_equals(name, "sub") ||
        lm_trans_text_equals(name, "if") ||
        lm_trans_text_equals(name, "else") ||
        lm_trans_text_equals(name, "while") ||
        lm_trans_text_equals(name, "return") ||
        lm_trans_text_equals(name, "break") ||
        lm_trans_text_equals(name, "continue") ||
        lm_trans_text_equals(name, "end") ||
        lm_trans_text_equals(name, "until") ||
        lm_trans_text_equals(name, "synchronized") ||
        lm_trans_text_equals(name, "const") ||
        lm_trans_text_equals(name, "external");
}

static int lm_trans_legacy_atom_is_prefix_expr_operator(LmP0Text text) {
    return
        lm_trans_text_equals(text, "++") ||
        lm_trans_text_equals(text, "--") ||
        lm_trans_text_equals(text, "+") ||
        lm_trans_text_equals(text, "-") ||
        lm_trans_text_equals(text, "@") ||
        lm_trans_text_equals(text, "\\") ||
        lm_trans_text_equals(text, "!") ||
        lm_trans_text_equals(text, "~");
}

static int lm_trans_legacy_atom_is_postfix_expr_operator(LmP0Text text) {
    return
        lm_trans_text_equals(text, "++") ||
        lm_trans_text_equals(text, "--");
}

static int lm_trans_legacy_atom_is_index_operator(LmP0Text text) {
    return lm_trans_text_equals(text, "[") || lm_trans_text_equals(text, "]");
}

static const char *lm_trans_legacy_expr_atom_payload(LmP0Text text) {
    if (lm_trans_text_equals(text, ".")) {
        return "emit.c-dot";
    }
    if (lm_trans_text_equals(text, "=")) {
        return "emit.==";
    }
    if (lm_trans_text_equals(text, "@")) {
        return "emit.&";
    }
    if (lm_trans_text_equals(text, "\\")) {
        return "emit.*-or-->";
    }
    return 0;
}

static const char *lm_trans_legacy_statement_payload(LmP0Text head) {
    if (lm_trans_text_equals(head, "return")) {
        return "return";
    }
    if (lm_trans_text_equals(head, "if")) {
        return "if";
    }
    if (lm_trans_text_equals(head, "while")) {
        return "while";
    }
    if (lm_trans_text_equals(head, "else")) {
        return "else";
    }
    if (lm_trans_text_equals(head, "synchronized")) {
        return "synchronized";
    }
    if (lm_trans_text_equals(head, "break")) {
        return "break";
    }
    if (lm_trans_text_equals(head, "continue")) {
        return "continue";
    }
    if (lm_trans_text_equals(head, "const")) {
        return "const";
    }
    if (lm_trans_text_equals(head, "[]")) {
        return "array-declaration";
    }
    return 0;
}

static const char *lm_trans_legacy_atom_statement_payload(LmP0Text atom) {
    if (lm_trans_text_equals(atom, "break")) {
        return "break";
    }
    if (lm_trans_text_equals(atom, "continue")) {
        return "continue";
    }
    if (lm_trans_text_equals(atom, "return")) {
        return "return";
    }
    return 0;
}

static const char *lm_trans_legacy_function_receiver_payload(LmP0Text head) {
    if (lm_trans_text_equals(head, "fn")) {
        return "function.primitive";
    }
    if (lm_trans_text_equals(head, "fm")) {
        return "function.struct";
    }
    if (lm_trans_text_equals(head, "sub")) {
        return "procedure";
    }
    return 0;
}

static const char *lm_trans_legacy_top_level_payload(LmP0Text head) {
    if (lm_trans_text_equals(head, "L1")) {
        return "lower.l1";
    }
    if (
        lm_trans_text_equals(head, "fn") ||
        lm_trans_text_equals(head, "fm") ||
        lm_trans_text_equals(head, "sub")
    ) {
        return "function";
    }
    if (lm_trans_text_equals(head, "external")) {
        return "external-function";
    }
    return 0;
}

static int lm_trans_legacy_frame_has_positional_name_argument(const LmP0Frame *frame) {
    if (frame == 0) {
        return 0;
    }

    return
        lm_trans_text_equals(frame->head, "fn") ||
        lm_trans_text_equals(frame->head, "fm") ||
        lm_trans_text_equals(frame->head, "sub") ||
        lm_trans_text_equals(frame->head, "synchronized") ||
        lm_trans_text_equals(frame->head, "[]") ||
        lm_trans_text_equals(frame->head, "entry");
}

static int lm_trans_text_all_char(LmP0Text text, char ch) {
    size_t i;

    if (text.length == 0U) {
        return 0;
    }

    for (i = 0U; i < text.length; ++i) {
        if (text.data[i] != ch) {
            return 0;
        }
    }

    return 1;
}

static LmP0Text *lm_trans_text_ref_new(LmP0Text text) {
    LmP0Text *copy;

    copy = (LmP0Text *)lm_own_new_zero(sizeof(*copy));
    if (copy != 0) {
        *copy = text;
    }
    return copy;
}

static void lm_trans_text_ref_destroy(LmP0Text **text) {
    if (text != 0 && *text != 0) {
        lm_own_delete(*text, 0);
        *text = 0;
    }
}

static void lm_trans_text_ref_delete_any(void *object) {
    lm_own_delete(object, 0);
}

static LmTransFunctionState *lm_trans_function_state_new(void) {
    return (LmTransFunctionState *)lm_own_new_zero(sizeof(LmTransFunctionState));
}

static void lm_trans_function_state_destroy(LmTransFunctionState *state) {
    if (state != 0) {
        lm_trans_text_ref_destroy(&state->current_return_type_name);
        if (state->has_previous_control_stacks) {
            lm_own_ptr_stack_destroy(&state->previous_cleanups);
            lm_own_ptr_stack_destroy(&state->previous_loops);
            state->has_previous_control_stacks = 0;
        }
    }
}

static void lm_trans_function_state_destroy_any(void *object) {
    lm_trans_function_state_destroy((LmTransFunctionState *)object);
}

static void lm_trans_function_state_delete(LmTransFunctionState *state) {
    lm_own_delete(state, lm_trans_function_state_destroy_any);
}

static int lm_trans_text_is_operator_atom(LmP0Text text) {
    return lm_trans_registry_has_compare(text, "operator.any", lm_trans_legacy_text_is_operator_atom(text));
}

static int lm_trans_write_all(FILE *file, const char *data, size_t length) {
    return fwrite(data, 1U, length, file) == length ? 0 : 1;
}

static int lm_trans_put(FILE *file, const char *text) {
    return fputs(text, file) < 0 ? 1 : 0;
}

static int lm_trans_write_text(FILE *file, LmP0Text text) {
    return lm_trans_write_all(file, text.data, text.length);
}

static int lm_trans_emit_indent(FILE *file, unsigned indent) {
    unsigned i;

    for (i = 0U; i < indent; ++i) {
        if (lm_trans_put(file, "    ") != 0) {
            return 1;
        }
    }

    return 0;
}

static const LmP0Field *lm_trans_nth_field(const LmP0Structure *structure, size_t index) {
    const LmP0Field *field;
    size_t i;

    field = structure->first_field;
    i = 0U;
    while (field != 0 && i < index) {
        field = field->next;
        ++i;
    }

    return field;
}

static int lm_trans_node_is_ignored(const LmP0Node *node) {
    return node == 0 || (node->flags & (LM_P0_NODE_INACTIVE | LM_P0_NODE_MIX)) != 0U;
}

static int lm_trans_trailer_single_atom(const LmP0Trailer *trailer, LmP0Text *out_text) {
    const LmP0Field *field;

    if (trailer == 0 || out_text == 0) {
        return 0;
    }

    field = trailer->body.first_field;
    if (
        field == 0 ||
        field->next != 0 ||
        field->value == 0 ||
        field->value->kind != LM_P0_NODE_ATOM
    ) {
        return 0;
    }

    *out_text = field->value->as.atom;
    return 1;
}

static const char *lm_trans_symbol_kind_name(LmTransSymbolKind kind) {
    switch (kind) {
    case LM_TRANS_SYMBOL_VARIABLE:
        return "variable";
    case LM_TRANS_SYMBOL_FUNCTION:
        return "function";
    case LM_TRANS_SYMBOL_PROCEDURE:
        return "procedure";
    case LM_TRANS_SYMBOL_LABEL:
        return "label";
    case LM_TRANS_SYMBOL_STRUCTURE:
        return "structure";
    default:
        return "symbol";
    }
}

static void lm_trans_symbol_destroy_fields(LmTransSymbol *symbol) {
    if (symbol != 0) {
        lm_trans_text_ref_destroy(&symbol->name);
        lm_own_ptr_stack_destroy(&symbol->param_names);
        symbol->has_signature = 0;
    }
}

static void lm_trans_symbol_destroy_fields_any(void *object) {
    lm_trans_symbol_destroy_fields((LmTransSymbol *)object);
}

static void lm_trans_symbol_destroy(LmTransSymbol *symbol) {
    lm_own_delete(symbol, lm_trans_symbol_destroy_fields_any);
}

static void lm_trans_symbol_delete_any(void *object) {
    lm_trans_symbol_destroy((LmTransSymbol *)object);
}

static LmTransSymbol *lm_trans_symbol_new(
    LmP0Text name,
    LmTransSymbolKind kind,
    unsigned depth
) {
    LmTransSymbol *symbol;

    symbol = (LmTransSymbol *)lm_own_new_zero(sizeof(*symbol));
    if (symbol == 0) {
        return 0;
    }

    symbol->name = lm_trans_text_ref_new(name);
    if (symbol->name == 0) {
        lm_trans_symbol_destroy(symbol);
        return 0;
    }

    lm_own_ptr_stack_init(&symbol->param_names, lm_trans_text_ref_delete_any);
    symbol->kind = kind;
    symbol->depth = depth;
    return symbol;
}

static LmTransCleanup *lm_trans_cleanup_new(unsigned id) {
    LmTransCleanup *cleanup;

    cleanup = (LmTransCleanup *)lm_own_new_zero(sizeof(*cleanup));
    if (cleanup != 0) {
        cleanup->id = id;
    }
    return cleanup;
}

static void lm_trans_cleanup_destroy(LmTransCleanup *cleanup) {
    lm_own_delete(cleanup, 0);
}

static void lm_trans_cleanup_delete_any(void *object) {
    lm_trans_cleanup_destroy((LmTransCleanup *)object);
}

static LmTransLoop *lm_trans_loop_new(size_t cleanup_base) {
    LmTransLoop *loop;

    loop = (LmTransLoop *)lm_own_new_zero(sizeof(*loop));
    if (loop != 0) {
        loop->cleanup_base = cleanup_base;
    }
    return loop;
}

static void lm_trans_loop_destroy(LmTransLoop *loop) {
    lm_own_delete(loop, 0);
}

static void lm_trans_loop_delete_any(void *object) {
    lm_trans_loop_destroy((LmTransLoop *)object);
}

static LmTransNamespace *lm_trans_namespace_new(void) {
    LmTransNamespace *namespace_;

    namespace_ = (LmTransNamespace *)lm_own_new_zero(sizeof(LmTransNamespace));
    if (namespace_ != 0) {
        lm_own_ptr_stack_init(&namespace_->items, lm_trans_symbol_delete_any);
        lm_own_ptr_stack_init(&namespace_->cleanups, lm_trans_cleanup_delete_any);
        lm_own_ptr_stack_init(&namespace_->loops, lm_trans_loop_delete_any);
    }
    return namespace_;
}

static void lm_trans_namespace_destroy(LmTransNamespace *namespace_) {
    if (namespace_ != 0) {
        lm_own_ptr_stack_destroy(&namespace_->items);
        lm_own_ptr_stack_destroy(&namespace_->cleanups);
        lm_own_ptr_stack_destroy(&namespace_->loops);
        namespace_->depth = 0U;
        namespace_->next_cleanup_id = 0U;
        namespace_->return_type_node = 0;
        namespace_->return_type_is_struct = 0;
        lm_trans_text_ref_destroy(&namespace_->return_type_name);
        namespace_->next_return_id = 0U;
    }
}

static void lm_trans_namespace_destroy_any(void *object) {
    lm_trans_namespace_destroy((LmTransNamespace *)object);
}

static void lm_trans_namespace_delete(LmTransNamespace *namespace_) {
    lm_own_delete(namespace_, lm_trans_namespace_destroy_any);
}

static void lm_trans_namespace_enter_scope(LmTransNamespace *namespace_) {
    if (namespace_ != 0) {
        ++namespace_->depth;
    }
}

static void lm_trans_namespace_leave_scope(LmTransNamespace *namespace_) {
    LmTransSymbol *symbol;

    if (namespace_ == 0) {
        return;
    }

    while (namespace_->items.count > 0U) {
        symbol = (LmTransSymbol *)lm_own_ptr_stack_top(&namespace_->items);
        if (symbol == 0 || symbol->depth != namespace_->depth) {
            break;
        }
        symbol = (LmTransSymbol *)lm_own_ptr_stack_pop(&namespace_->items);
        lm_trans_symbol_destroy(symbol);
    }

    if (namespace_->depth > 0U) {
        --namespace_->depth;
    }
}

static int lm_trans_cleanup_push(LmTransNamespace *namespace_, unsigned id) {
    LmTransCleanup *cleanup;

    if (namespace_ == 0) {
        return 1;
    }

    cleanup = lm_trans_cleanup_new(id);
    if (cleanup == 0) {
        return 1;
    }
    if (lm_own_ptr_stack_push(&namespace_->cleanups, cleanup) != 0) {
        lm_trans_cleanup_destroy(cleanup);
        return 1;
    }
    return 0;
}

static void lm_trans_cleanup_pop(LmTransNamespace *namespace_) {
    LmTransCleanup *cleanup;

    if (namespace_ != 0) {
        cleanup = (LmTransCleanup *)lm_own_ptr_stack_pop(&namespace_->cleanups);
        lm_trans_cleanup_destroy(cleanup);
    }
}

static int lm_trans_loop_push(LmTransNamespace *namespace_) {
    LmTransLoop *loop;

    if (namespace_ == 0) {
        return 1;
    }

    loop = lm_trans_loop_new(namespace_->cleanups.count);
    if (loop == 0) {
        return 1;
    }
    if (lm_own_ptr_stack_push(&namespace_->loops, loop) != 0) {
        lm_trans_loop_destroy(loop);
        return 1;
    }
    return 0;
}

static void lm_trans_loop_pop(LmTransNamespace *namespace_) {
    LmTransLoop *loop;

    if (namespace_ != 0) {
        loop = (LmTransLoop *)lm_own_ptr_stack_pop(&namespace_->loops);
        lm_trans_loop_destroy(loop);
    }
}

static size_t lm_trans_loop_cleanup_base(const LmTransNamespace *namespace_) {
    const LmTransLoop *loop;

    if (namespace_ == 0 || namespace_->loops.count == 0U) {
        return namespace_ != 0 ? namespace_->cleanups.count : 0U;
    }

    loop = (const LmTransLoop *)lm_own_ptr_stack_top(&namespace_->loops);
    return loop != 0 ? loop->cleanup_base : namespace_->cleanups.count;
}

static int lm_trans_emit_sync_name(FILE *file, unsigned id) {
    return fprintf(file, "lm_sync_%u", id) < 0 ? 1 : 0;
}

static int lm_trans_emit_return_name(FILE *file, unsigned id) {
    return fprintf(file, "lm_return_%u", id) < 0 ? 1 : 0;
}

static int lm_trans_symbol_name_same(const LmP0Text *left, LmP0Text right) {
    return left != 0 && lm_trans_text_same(*left, right);
}

static const LmTransSymbol *lm_trans_namespace_find(
    const LmTransNamespace *namespace_,
    LmP0Text name
) {
    size_t i;
    const LmTransSymbol *symbol;

    if (namespace_ == 0) {
        return 0;
    }

    i = namespace_->items.count;
    while (i > 0U) {
        --i;
        symbol = (const LmTransSymbol *)lm_own_ptr_stack_at(&namespace_->items, i);
        if (
            symbol != 0 &&
            lm_trans_symbol_name_same(symbol->name, name)
        ) {
            return symbol;
        }
    }

    return 0;
}

static LmTransSymbol *lm_trans_namespace_find_mutable(
    LmTransNamespace *namespace_,
    LmP0Text name
) {
    size_t i;
    LmTransSymbol *symbol;

    if (namespace_ == 0) {
        return 0;
    }

    i = namespace_->items.count;
    while (i > 0U) {
        --i;
        symbol = (LmTransSymbol *)lm_own_ptr_stack_at(&namespace_->items, i);
        if (
            symbol != 0 &&
            lm_trans_symbol_name_same(symbol->name, name)
        ) {
            return symbol;
        }
    }

    return 0;
}

static int lm_trans_is_c_reference_name(LmP0Text name);

static int lm_trans_is_reserved_head_name(LmP0Text name) {
    return lm_trans_registry_has_compare(name, "receiver.reserved", lm_trans_legacy_is_reserved_head_name(name));
}

static int lm_trans_namespace_declare(
    LmTransNamespace *namespace_,
    LmP0Text name,
    LmTransSymbolKind kind
) {
    LmTransSymbol *symbol;
    const LmTransSymbol *existing;

    if (namespace_ == 0) {
        return 1;
    }

    if (lm_trans_is_c_reference_name(name)) {
        fprintf(
            stderr,
            "trans L2 error: C-surface spelling \"%.*s\" cannot be declared as a Lingvamyxa name\n",
            (int)name.length,
            name.data
        );
        return 1;
    }

    if (lm_trans_is_reserved_head_name(name)) {
        fprintf(
            stderr,
            "trans L2 error: reserved head \"%.*s\" cannot be redeclared\n",
            (int)name.length,
            name.data
        );
        return 1;
    }

    existing = lm_trans_namespace_find(namespace_, name);
    if (existing != 0) {
        fprintf(
            stderr,
            "trans L2 error: name \"%.*s\" is already visible as %s\n",
            (int)name.length,
            name.data,
            lm_trans_symbol_kind_name(existing->kind)
        );
        return 1;
    }

    symbol = lm_trans_symbol_new(name, kind, namespace_->depth);
    if (symbol == 0) {
        return 1;
    }
    if (lm_own_ptr_stack_push(&namespace_->items, symbol) != 0) {
        lm_trans_symbol_destroy(symbol);
        return 1;
    }
    return 0;
}

static const char *lm_trans_c_spelling(LmP0Text name) {
    return lm_trans_registry_lookup_compare(name, "type.spelling", lm_trans_legacy_c_spelling(name));
}

static int lm_trans_builtin_c_type_tail(LmP0Text name) {
    int registry_result;

    registry_result =
        lm_trans_c_spelling(name) != 0 ||
        lm_trans_registry_has(name, "type.cTail");
    return lm_trans_bool_compare(
        "type.cTail/type.spelling",
        name,
        registry_result,
        lm_trans_legacy_builtin_c_type_tail(name)
    );
}

static int lm_trans_builtin_c_type_name(LmP0Text name) {
    LmP0Text tail;

    if (lm_trans_text_starts_with(name, "c.") && name.length > 2U) {
        tail.data = name.data + 2U;
        tail.length = name.length - 2U;
        return lm_trans_builtin_c_type_tail(tail);
    }

    return lm_trans_builtin_c_type_tail(name);
}

static int lm_trans_emit_name(FILE *file, LmP0Text name) {
    const char *spelling;
    LmP0Text tail;

    if (lm_trans_text_starts_with(name, "c.")) {
        tail.data = name.data + 2U;
        tail.length = name.length - 2U;
        spelling = lm_trans_c_spelling(tail);
        if (spelling != 0) {
            return lm_trans_put(file, spelling);
        }
        return lm_trans_write_text(file, tail);
    }

    return lm_trans_write_text(file, name);
}

static int lm_trans_emit_type_name(FILE *file, LmP0Text name) {
    const char *spelling;
    LmP0Text tail;

    if (lm_trans_text_starts_with(name, "c.") && name.length > 2U) {
        tail.data = name.data + 2U;
        tail.length = name.length - 2U;
        spelling = lm_trans_c_spelling(tail);
        if (spelling != 0) {
            return lm_trans_put(file, spelling);
        }
        return lm_trans_write_text(file, tail);
    }

    spelling = lm_trans_c_spelling(name);
    if (spelling != 0) {
        return lm_trans_put(file, spelling);
    }
    return lm_trans_write_text(file, name);
}

static int lm_trans_is_c_reference_name(LmP0Text name) {
    return lm_trans_bool_compare(
        "surface.prefix",
        name,
        lm_trans_registry_has_prefix(name, "surface.prefix") && name.length > 2U,
        lm_trans_text_starts_with(name, "c.") && name.length > 2U
    );
}

static int lm_trans_c_reference_has_path_dot(LmP0Text name) {
    size_t i;

    if (!lm_trans_is_c_reference_name(name)) {
        return 0;
    }

    i = 2U;
    while (i < name.length) {
        if (name.data[i] == '.') {
            return 1;
        }
        ++i;
    }

    return 0;
}

static int lm_trans_node_is_c_reference_atom(const LmP0Node *node) {
    return
        node != 0 &&
        node->kind == LM_P0_NODE_ATOM &&
        lm_trans_is_c_reference_name(node->as.atom);
}

static int lm_trans_emit_expr_node(FILE *file, const LmP0Node *node, const LmTransNamespace *namespace_);
static int lm_trans_emit_expr_fields(FILE *file, const LmP0Field *first, const LmTransNamespace *namespace_);
static int lm_trans_emit_expr_range(
    FILE *file,
    const LmP0Field *first,
    const LmP0Field *stop,
    const LmTransNamespace *namespace_
);

static int lm_trans_atom_starts_string(LmP0Text text) {
    return text.length > 0U && (text.data[0] == '"' || text.data[0] == '\'');
}

static int lm_trans_atom_is_identifier_like(LmP0Text text) {
    unsigned char ch;

    if (text.length == 0U || lm_trans_atom_starts_string(text)) {
        return 0;
    }

    if (
        lm_trans_text_is_operator_atom(text) ||
        lm_trans_text_equals(text, "@") ||
        lm_trans_text_equals(text, "\\")
    ) {
        return 0;
    }

    ch = (unsigned char)text.data[0];
    return
        (ch >= 'A' && ch <= 'Z') ||
        (ch >= 'a' && ch <= 'z') ||
        ch == '_' ||
        ch == '`';
}

static int lm_trans_name_argument_is_valid(LmP0Text text) {
    (void)text;
    return 1;
}

static int lm_trans_text_contains_char(LmP0Text text, char ch);

static int lm_trans_validate_expr_atom(
    LmP0Text atom,
    const LmTransNamespace *namespace_
) {
    const LmTransSymbol *symbol;

    if (lm_trans_is_c_reference_name(atom) || !lm_trans_atom_is_identifier_like(atom)) {
        return 0;
    }

    symbol = lm_trans_namespace_find(namespace_, atom);
    if (symbol == 0) {
        fprintf(
            stderr,
            "trans L2 error: unknown Lingvamyxa name \"%.*s\"; use c.%.*s for explicit C-surface spelling\n",
            (int)atom.length,
            atom.data,
            (int)atom.length,
            atom.data
        );
        return 1;
    }

    if (
        symbol->kind == LM_TRANS_SYMBOL_PROCEDURE ||
        symbol->kind == LM_TRANS_SYMBOL_LABEL ||
        symbol->kind == LM_TRANS_SYMBOL_STRUCTURE
    ) {
        fprintf(
            stderr,
            "trans L2 error: \"%.*s\" is %s, not an expression value\n",
            (int)atom.length,
            atom.data,
            lm_trans_symbol_kind_name(symbol->kind)
        );
        return 1;
    }

    return 0;
}

static int lm_trans_emit_expr_atom(
    FILE *file,
    LmP0Text atom,
    const LmTransNamespace *namespace_
) {
    if (lm_trans_validate_expr_atom(atom, namespace_) != 0) {
        return 1;
    }

    return lm_trans_emit_name(file, atom);
}

static int lm_trans_lower_call(
    LmP0Text head,
    const LmTransNamespace *namespace_,
    LmTransCallLoweringMode mode,
    LmTransCallLowering *out
) {
    const LmTransSymbol *symbol;
    const char *expected;

    if (out == 0) {
        return 1;
    }

    out->name = head;
    out->signature = 0;
    if (lm_trans_is_c_reference_name(head)) {
        return 0;
    }

    symbol = lm_trans_namespace_find(namespace_, head);
    if (symbol == 0) {
        expected = mode == LM_TRANS_CALL_LOWER_VALUE ? "function" : "callable";
        fprintf(
            stderr,
            "trans L2 error: unknown Lingvamyxa %s \"%.*s\"; use c.%.*s for explicit C-surface spelling\n",
            expected,
            (int)head.length,
            head.data,
            (int)head.length,
            head.data
        );
        return 1;
    }

    if (mode == LM_TRANS_CALL_LOWER_VALUE) {
        if (symbol->kind != LM_TRANS_SYMBOL_FUNCTION) {
            fprintf(
                stderr,
                "trans L2 error: \"%.*s\" is %s, not a value-returning function\n",
                (int)head.length,
                head.data,
                lm_trans_symbol_kind_name(symbol->kind)
            );
            return 1;
        }
    } else if (
        symbol->kind != LM_TRANS_SYMBOL_FUNCTION &&
        symbol->kind != LM_TRANS_SYMBOL_PROCEDURE
    ) {
        fprintf(
            stderr,
            "trans L2 error: \"%.*s\" is %s, not a callable\n",
            (int)head.length,
            head.data,
            lm_trans_symbol_kind_name(symbol->kind)
        );
        return 1;
    }

    out->signature = symbol;
    return 0;
}

static int lm_trans_atom_is_prefix_expr_operator(LmP0Text text) {
    return lm_trans_registry_has_compare(text, "operator.prefix", lm_trans_legacy_atom_is_prefix_expr_operator(text));
}

static int lm_trans_atom_is_postfix_expr_operator(LmP0Text text) {
    return lm_trans_registry_has_compare(text, "operator.postfix", lm_trans_legacy_atom_is_postfix_expr_operator(text));
}

static int lm_trans_nodes_touch(const LmP0Node *left, const LmP0Node *right) {
    return
        left != 0 &&
        right != 0 &&
        left->span.offset + left->span.length == right->span.offset;
}

static int lm_trans_legacy_atom_is_infix_expr_operator(
    LmP0Text text,
    const LmP0Node *operator_node,
    const LmP0Node *previous_operand
) {
    if (lm_trans_text_equals(text, "\\")) {
        return
            previous_operand != 0 &&
            lm_trans_nodes_touch(previous_operand, operator_node) &&
            !(previous_operand->kind == LM_P0_NODE_ATOM && lm_trans_atom_starts_string(previous_operand->as.atom));
    }

    return
        lm_trans_text_equals(text, "+") ||
        lm_trans_text_equals(text, "-") ||
        lm_trans_text_equals(text, "*") ||
        lm_trans_text_equals(text, "/") ||
        lm_trans_text_equals(text, "%") ||
        lm_trans_text_equals(text, "=") ||
        lm_trans_text_equals(text, "!=") ||
        lm_trans_text_equals(text, "<") ||
        lm_trans_text_equals(text, "<=") ||
        lm_trans_text_equals(text, ">") ||
        lm_trans_text_equals(text, ">=") ||
        lm_trans_text_equals(text, "&&") ||
        lm_trans_text_equals(text, "||") ||
        lm_trans_text_equals(text, "&") ||
        lm_trans_text_equals(text, "|") ||
        lm_trans_text_equals(text, "^");
}

static int lm_trans_atom_is_infix_expr_operator(
    LmP0Text text,
    const LmP0Node *operator_node,
    const LmP0Node *previous_operand
) {
    int registry_result;

    if (lm_trans_text_equals(text, "\\")) {
        if (!lm_trans_registry_has(text, "operator.infix")) {
            return lm_trans_bool_compare(
                "operator.infix",
                text,
                0,
                lm_trans_legacy_atom_is_infix_expr_operator(text, operator_node, previous_operand)
            );
        }
        registry_result =
            previous_operand != 0 &&
            lm_trans_nodes_touch(previous_operand, operator_node) &&
            !(previous_operand->kind == LM_P0_NODE_ATOM && lm_trans_atom_starts_string(previous_operand->as.atom));
        return lm_trans_bool_compare(
            "operator.infix",
            text,
            registry_result,
            lm_trans_legacy_atom_is_infix_expr_operator(text, operator_node, previous_operand)
        );
    }

    return lm_trans_registry_has_compare(
        text,
        "operator.infix",
        lm_trans_legacy_atom_is_infix_expr_operator(text, operator_node, previous_operand)
    );
}

static const LmP0Field *lm_trans_expr_segment_end(const LmP0Field *first) {
    const LmP0Field *field;
    const LmP0Field *operand;
    const LmP0Node *previous_operand;
    const LmP0Node *node;
    int bracket_depth;
    int c_dot_path;

    if (first == 0) {
        return 0;
    }

    field = first;
    previous_operand = 0;
    c_dot_path = 0;

    node = field->value;
    while (
        node != 0 &&
        node->kind == LM_P0_NODE_ATOM &&
        lm_trans_atom_is_prefix_expr_operator(node->as.atom)
    ) {
        field = field->next;
        if (field == 0) {
            return 0;
        }
        node = field->value;
    }
    previous_operand = node;
    c_dot_path = lm_trans_node_is_c_reference_atom(previous_operand);
    field = field->next;

    while (field != 0) {
        node = field->value;
        if (node == 0 || node->kind != LM_P0_NODE_ATOM) {
            break;
        }

        if (lm_trans_text_equals(node->as.atom, "[")) {
            bracket_depth = 1;
            field = field->next;
            while (field != 0 && bracket_depth > 0) {
                node = field->value;
                if (node != 0 && node->kind == LM_P0_NODE_ATOM) {
                    if (lm_trans_text_equals(node->as.atom, "[")) {
                        ++bracket_depth;
                    } else if (lm_trans_text_equals(node->as.atom, "]")) {
                        --bracket_depth;
                    }
                }
                previous_operand = node;
                field = field->next;
            }
        } else if (lm_trans_text_equals(node->as.atom, ".")) {
            if (
                !c_dot_path ||
                previous_operand == 0 ||
                !lm_trans_nodes_touch(previous_operand, node)
            ) {
                break;
            }
            operand = field->next;
            if (operand == 0) {
                return field;
            }
            previous_operand = operand->value;
            c_dot_path = 1;
            field = operand->next;
        } else if (lm_trans_atom_is_postfix_expr_operator(node->as.atom)) {
            if (previous_operand == 0) {
                break;
            }
            previous_operand = node;
            c_dot_path = 0;
            field = field->next;
        } else if (lm_trans_atom_is_infix_expr_operator(node->as.atom, node, previous_operand)) {
            operand = field->next;
            if (operand == 0) {
                return field;
            }
            while (
                operand->value != 0 &&
                operand->value->kind == LM_P0_NODE_ATOM &&
                lm_trans_atom_is_prefix_expr_operator(operand->value->as.atom)
            ) {
                operand = operand->next;
                if (operand == 0) {
                    return field;
                }
            }
            previous_operand = operand->value;
            c_dot_path = lm_trans_node_is_c_reference_atom(previous_operand);
            field = operand->next;
        } else {
            break;
        }
    }

    return field;
}

static int lm_trans_signature_param_index(
    const LmTransSymbol *callee,
    LmP0Text name,
    size_t *out_index
) {
    size_t i;
    const LmP0Text *param_name;

    if (callee == 0 || out_index == 0) {
        return 0;
    }

    for (i = 0U; i < callee->param_names.count; ++i) {
        param_name = (const LmP0Text *)lm_own_ptr_stack_at(&callee->param_names, i);
        if (param_name != 0 && lm_trans_text_same(*param_name, name)) {
            *out_index = i;
            return 1;
        }
    }

    return 0;
}

static int lm_trans_call_field_is_named_argument(
    const LmP0Field *field,
    const LmTransSymbol *callee,
    size_t *out_index
) {
    const LmP0Frame *frame;

    if (
        field == 0 ||
        field->value == 0 ||
        field->value->kind != LM_P0_NODE_FRAME ||
        callee == 0 ||
        out_index == 0
    ) {
        return 0;
    }

    frame = &field->value->as.frame;
    if ((frame->flags & LM_P0_FRAME_COLON) == 0U) {
        return 0;
    }

    return lm_trans_signature_param_index(callee, frame->head, out_index);
}

static LmTransExprLoweredRange *lm_trans_expr_lowered_range_new(void) {
    LmTransExprLoweredRange *range;

    range = (LmTransExprLoweredRange *)lm_own_new_zero(sizeof(LmTransExprLoweredRange));
    if (range != 0) {
        lm_own_value_stack_init(&range->pieces, sizeof(LmTransExprPiece));
    }
    return range;
}

static void lm_trans_expr_lowered_range_destroy(LmTransExprLoweredRange *range) {
    if (range != 0) {
        lm_own_value_stack_destroy(&range->pieces);
        range->index = 0U;
    }
}

static void lm_trans_expr_lowered_range_destroy_any(void *object) {
    lm_trans_expr_lowered_range_destroy((LmTransExprLoweredRange *)object);
}

static void lm_trans_expr_lowered_range_delete(LmTransExprLoweredRange *range) {
    lm_own_delete(range, lm_trans_expr_lowered_range_destroy_any);
}

static void lm_trans_expr_job_destroy(LmTransExprJob *job) {
    if (job != 0 && job->kind == LM_TRANS_EXPR_JOB_LOWERED_RANGE) {
        lm_trans_expr_lowered_range_delete(job->as.lowered_range);
        job->as.lowered_range = 0;
    }
}

static void lm_trans_expr_stack_destroy(LmTransExprStack *stack) {
    LmTransExprJob job;

    if (stack != 0) {
        while (lm_own_value_stack_pop(&stack->jobs, &job) == 0) {
            lm_trans_expr_job_destroy(&job);
        }
        lm_own_value_stack_destroy(&stack->jobs);
    }
}

static int lm_trans_expr_stack_push(LmTransExprStack *stack, LmTransExprJob job) {
    if (stack == 0) {
        return 1;
    }

    return lm_own_value_stack_push(&stack->jobs, &job);
}

static int lm_trans_expr_stack_push_text(LmTransExprStack *stack, const char *text) {
    LmTransExprJob job;

    job.kind = LM_TRANS_EXPR_JOB_TEXT;
    job.as.text = text;
    return lm_trans_expr_stack_push(stack, job);
}

static int lm_trans_expr_stack_push_node(LmTransExprStack *stack, const LmP0Node *node) {
    LmTransExprJob job;

    job.kind = LM_TRANS_EXPR_JOB_NODE;
    job.as.node = node;
    return lm_trans_expr_stack_push(stack, job);
}

static int lm_trans_expr_stack_push_frame(LmTransExprStack *stack, const LmP0Frame *frame) {
    LmTransExprJob job;

    job.kind = LM_TRANS_EXPR_JOB_FRAME;
    job.as.frame = frame;
    return lm_trans_expr_stack_push(stack, job);
}

static int lm_trans_expr_stack_push_range_state(
    LmTransExprStack *stack,
    LmTransExprRangeJob range
) {
    LmTransExprJob job;

    job.kind = LM_TRANS_EXPR_JOB_RANGE;
    job.as.range = range;
    return lm_trans_expr_stack_push(stack, job);
}

static int lm_trans_expr_stack_push_range(
    LmTransExprStack *stack,
    const LmP0Field *first,
    const LmP0Field *stop
) {
    LmTransExprRangeJob range;

    range.field = first;
    range.stop = stop;
    range.wrote = 0;
    range.previous_operand = 0;
    range.expect_field_name = 0;
    range.expect_c_field_name = 0;
    range.c_dot_path = 0;
    return lm_trans_expr_stack_push_range_state(stack, range);
}

static int lm_trans_expr_stack_push_call_args(
    LmTransExprStack *stack,
    const LmP0Structure *body,
    const LmTransSymbol *callee
) {
    LmTransExprJob job;

    job.kind = LM_TRANS_EXPR_JOB_CALL_ARGS;
    job.as.call_args.body = body;
    job.as.call_args.callee = callee;
    return lm_trans_expr_stack_push(stack, job);
}

static int lm_trans_expr_stack_push_lowered_range(
    LmTransExprStack *stack,
    LmTransExprLoweredRange *range
) {
    LmTransExprJob job;

    job.kind = LM_TRANS_EXPR_JOB_LOWERED_RANGE;
    job.as.lowered_range = range;
    return lm_trans_expr_stack_push(stack, job);
}

static void lm_trans_expr_segments_init(LmOwnValueStack *segments) {
    lm_own_value_stack_init(segments, sizeof(LmTransExprSegment));
}

static int lm_trans_expr_segments_append(
    LmOwnValueStack *segments,
    const LmP0Field *first,
    const LmP0Field *stop
) {
    LmTransExprSegment segment;

    segment.first = first;
    segment.stop = stop;
    segment.present = 1;
    return lm_own_value_stack_push(segments, &segment);
}

static int lm_trans_expr_segments_parse_fields(
    LmOwnValueStack *segments,
    const LmP0Field *first
) {
    const LmP0Field *field;
    const LmP0Field *next;

    field = first;
    while (field != 0) {
        next = lm_trans_expr_segment_end(field);
        if (lm_trans_expr_segments_append(segments, field, next) != 0) {
            return 1;
        }
        field = next;
    }

    return 0;
}

static int lm_trans_expr_stack_push_segments(
    LmTransExprStack *stack,
    const LmOwnValueStack *segments
) {
    size_t index;
    const LmTransExprSegment *segment;

    if (segments == 0) {
        return 0;
    }

    index = segments->count;
    while (index > 0U) {
        --index;
        segment = (const LmTransExprSegment *)lm_own_value_stack_at(segments, index);
        if (segment == 0) {
            return 1;
        }
        if (lm_trans_expr_stack_push_range(stack, segment->first, segment->stop) != 0) {
            return 1;
        }
        if (index > 0U && lm_trans_expr_stack_push_text(stack, ", ") != 0) {
            return 1;
        }
    }

    return 0;
}

static const LmP0Field *lm_trans_call_body_first_field(const LmP0Structure *body) {
    const LmP0Field *field;

    if (body == 0) {
        return 0;
    }

    field = body->first_field;
    if (
        field != 0 &&
        field->next == 0 &&
        field->value != 0 &&
        field->value->kind == LM_P0_NODE_STRUCTURE
    ) {
        field = field->value->as.structure.first_field;
    }

    return field;
}

static int lm_trans_call_args_layout_signature(
    LmOwnValueStack *segments,
    const LmP0Field *field,
    const LmTransSymbol *callee
) {
    const LmP0Field *next;
    LmTransExprSegment *segment;
    const LmP0Frame *named_frame;
    size_t index;
    size_t named_index;
    int named_started;
    int is_named;

    if (callee == 0) {
        return 1;
    }

    if (lm_own_value_stack_resize_zero(segments, callee->param_names.count) != 0) {
        return 1;
    }

    index = 0U;
    named_started = 0;
    while (field != 0) {
        is_named = lm_trans_call_field_is_named_argument(field, callee, &named_index);
        if (is_named) {
            named_started = 1;
            segment = (LmTransExprSegment *)lm_own_value_stack_at(segments, named_index);
            if (segment == 0) {
                return 1;
            }
            if (segment->present) {
                fprintf(stderr, "trans L2 error: duplicate named argument\n");
                return 1;
            }
            named_frame = &field->value->as.frame;
            segment->first = named_frame->body.first_field;
            segment->stop = 0;
            segment->present = 1;
            field = field->next;
            continue;
        }

        if (named_started) {
            fprintf(stderr, "trans L2 error: positional argument after named argument\n");
            return 1;
        }
        if (index >= callee->param_names.count) {
            fprintf(stderr, "trans L2 error: too many arguments\n");
            return 1;
        }
        next = lm_trans_expr_segment_end(field);
        segment = (LmTransExprSegment *)lm_own_value_stack_at(segments, index);
        if (segment == 0) {
            return 1;
        }
        segment->first = field;
        segment->stop = next;
        segment->present = 1;
        ++index;
        field = next;
    }

    for (index = 0U; index < callee->param_names.count; ++index) {
        segment = (LmTransExprSegment *)lm_own_value_stack_at(segments, index);
        if (segment == 0) {
            return 1;
        }
        if (!segment->present) {
            fprintf(stderr, "trans L2 error: missing function argument\n");
            return 1;
        }
    }

    return 0;
}

static int lm_trans_call_args_layout(
    LmOwnValueStack *segments,
    const LmP0Structure *body,
    const LmTransSymbol *callee
) {
    const LmP0Field *field;

    field = lm_trans_call_body_first_field(body);
    if (callee != 0 && callee->has_signature) {
        return lm_trans_call_args_layout_signature(segments, field, callee);
    }

    return lm_trans_expr_segments_parse_fields(segments, field);
}

static int lm_trans_expr_stack_schedule_call_args(
    LmTransExprStack *stack,
    const LmP0Structure *body,
    const LmTransSymbol *callee
) {
    LmOwnValueStack segments;
    int status;

    lm_trans_expr_segments_init(&segments);
    status = lm_trans_call_args_layout(&segments, body, callee);
    if (status == 0) {
        status = lm_trans_expr_stack_push_segments(stack, &segments);
    }
    lm_own_value_stack_destroy(&segments);
    return status;
}

static int lm_trans_expr_stack_emit_frame(
    FILE *file,
    LmTransExprStack *stack,
    const LmP0Frame *frame,
    const LmTransNamespace *namespace_
) {
    LmTransCallLowering call;

    if (frame == 0) {
        return 0;
    }

    if (lm_trans_lower_call(frame->head, namespace_, LM_TRANS_CALL_LOWER_VALUE, &call) != 0) {
        return 1;
    }

    if (lm_trans_emit_name(file, call.name) != 0) {
        return 1;
    }
    if (lm_trans_put(file, "(") != 0) {
        return 1;
    }
    if (lm_trans_expr_stack_push_text(stack, ")") != 0) {
        return 1;
    }
    return lm_trans_expr_stack_push_call_args(stack, &frame->body, call.signature);
}

static int lm_trans_expr_stack_emit_node(
    FILE *file,
    LmTransExprStack *stack,
    const LmP0Node *node,
    const LmTransNamespace *namespace_
) {
    if (node == 0) {
        return 0;
    }

    if (node->kind == LM_P0_NODE_ATOM) {
        return lm_trans_emit_expr_atom(file, node->as.atom, namespace_);
    }

    if (node->kind == LM_P0_NODE_FRAME) {
        return lm_trans_expr_stack_push_frame(stack, &node->as.frame);
    }

    if (node->kind == LM_P0_NODE_STRUCTURE) {
        if (lm_trans_expr_stack_push_text(stack, ")") != 0) {
            return 1;
        }
        if (lm_trans_expr_stack_push_range(stack, node->as.structure.first_field, 0) != 0) {
            return 1;
        }
        return lm_trans_expr_stack_push_text(stack, "(");
    }

    return 0;
}

static int lm_trans_atom_is_operand_like(LmP0Text text) {
    return
        !lm_trans_text_is_operator_atom(text) &&
        !lm_trans_text_equals(text, "@") &&
        !lm_trans_text_equals(text, "\\");
}

static int lm_trans_atom_is_index_operator(LmP0Text text) {
    return lm_trans_registry_has_compare(text, "operator.index", lm_trans_legacy_atom_is_index_operator(text));
}

static LmTransExprAtomLowering lm_trans_lower_expr_atom(
    const LmP0Node *node,
    const LmP0Node *previous_operand,
    int expect_field_name,
    int expect_c_field_name,
    int c_dot_path
) {
    LmTransExprAtomLowering lowering;
    const char *payload;

    lowering.kind = LM_TRANS_EXPR_ATOM_LOWER_VALUE;
    lowering.text = node->as.atom;
    payload = lm_trans_registry_lookup_compare(
        node->as.atom,
        "expr.atom",
        lm_trans_legacy_expr_atom_payload(node->as.atom)
    );

    if (expect_field_name) {
        lowering.kind = LM_TRANS_EXPR_ATOM_LOWER_FIELD_NAME;
    } else if (expect_c_field_name) {
        lowering.kind = LM_TRANS_EXPR_ATOM_LOWER_C_FIELD_NAME;
    } else if (lm_trans_is_c_reference_name(node->as.atom)) {
        lowering.kind = LM_TRANS_EXPR_ATOM_LOWER_C_SURFACE;
    } else if (payload != 0 && strcmp(payload, "emit.c-dot") == 0) {
        lowering.kind = LM_TRANS_EXPR_ATOM_LOWER_C_DOT;
    } else if (payload != 0 && strcmp(payload, "emit.==") == 0) {
        lowering.kind = LM_TRANS_EXPR_ATOM_LOWER_EQUAL;
    } else if (payload != 0 && strcmp(payload, "emit.&") == 0) {
        lowering.kind = LM_TRANS_EXPR_ATOM_LOWER_ADDRESS;
    } else if (payload != 0 && strcmp(payload, "emit.*-or-->") == 0) {
        if (previous_operand != 0 && lm_trans_nodes_touch(previous_operand, node)) {
            lowering.kind = LM_TRANS_EXPR_ATOM_LOWER_POINTER_FOLLOW;
        } else {
            lowering.kind = LM_TRANS_EXPR_ATOM_LOWER_DEREF;
        }
    } else if (lm_trans_atom_is_index_operator(node->as.atom)) {
        lowering.kind = LM_TRANS_EXPR_ATOM_LOWER_INDEX_OPERATOR;
    } else if (lm_trans_text_is_operator_atom(node->as.atom)) {
        lowering.kind = LM_TRANS_EXPR_ATOM_LOWER_INFIX_OPERATOR;
    }

    (void)c_dot_path;
    return lowering;
}

static int lm_trans_update_expr_atom_lowering_state(
    LmTransExprAtomLowering lowering,
    const LmP0Node *node,
    const LmP0Node **previous_operand,
    int *expect_field_name,
    int *expect_c_field_name,
    int *c_dot_path
) {
    if (lowering.kind == LM_TRANS_EXPR_ATOM_LOWER_FIELD_NAME) {
        if (!lm_trans_atom_is_identifier_like(lowering.text)) {
            fprintf(stderr, "trans L2 error: field-follow expects a field name\n");
            return 1;
        }
        *previous_operand = node;
        *expect_field_name = 0;
        *c_dot_path = 0;
    } else if (lowering.kind == LM_TRANS_EXPR_ATOM_LOWER_C_FIELD_NAME) {
        if (!lm_trans_atom_is_identifier_like(lowering.text)) {
            fprintf(stderr, "trans L2 error: C value-field dot expects a field name\n");
            return 1;
        }
        *previous_operand = node;
        *expect_c_field_name = 0;
        *c_dot_path = 1;
    } else if (lowering.kind == LM_TRANS_EXPR_ATOM_LOWER_C_DOT) {
        if (
            !*c_dot_path ||
            *previous_operand == 0 ||
            !lm_trans_nodes_touch(*previous_operand, node)
        ) {
            fprintf(stderr, "trans L2 error: C value-field dot must follow a c.name path\n");
            return 1;
        }
        *previous_operand = 0;
        *expect_c_field_name = 1;
    } else if (lowering.kind == LM_TRANS_EXPR_ATOM_LOWER_EQUAL) {
        *previous_operand = 0;
        *c_dot_path = 0;
    } else if (lowering.kind == LM_TRANS_EXPR_ATOM_LOWER_ADDRESS) {
        *previous_operand = 0;
        *c_dot_path = 0;
    } else if (lowering.kind == LM_TRANS_EXPR_ATOM_LOWER_POINTER_FOLLOW) {
        *previous_operand = 0;
        *expect_field_name = 1;
        *c_dot_path = 0;
    } else if (lowering.kind == LM_TRANS_EXPR_ATOM_LOWER_DEREF) {
        *previous_operand = 0;
        *c_dot_path = 0;
    } else if (
        lowering.kind == LM_TRANS_EXPR_ATOM_LOWER_INDEX_OPERATOR ||
        lowering.kind == LM_TRANS_EXPR_ATOM_LOWER_INFIX_OPERATOR
    ) {
        *previous_operand = 0;
        *c_dot_path = 0;
    } else if (lowering.kind == LM_TRANS_EXPR_ATOM_LOWER_C_SURFACE) {
        *previous_operand = node;
        *c_dot_path = 1;
    } else {
        *previous_operand = lm_trans_atom_is_operand_like(lowering.text) ? node : 0;
        *c_dot_path = lm_trans_node_is_c_reference_atom(*previous_operand);
    }

    return 0;
}

static int lm_trans_emit_expr_atom_lowering(
    FILE *file,
    LmTransExprAtomLowering lowering,
    const LmTransNamespace *namespace_
) {
    if (
        lowering.kind == LM_TRANS_EXPR_ATOM_LOWER_FIELD_NAME ||
        lowering.kind == LM_TRANS_EXPR_ATOM_LOWER_C_FIELD_NAME
    ) {
        return lm_trans_emit_name(file, lowering.text);
    }
    if (lowering.kind == LM_TRANS_EXPR_ATOM_LOWER_C_DOT) {
        return lm_trans_put(file, ".");
    }
    if (lowering.kind == LM_TRANS_EXPR_ATOM_LOWER_EQUAL) {
        return lm_trans_put(file, "==");
    }
    if (lowering.kind == LM_TRANS_EXPR_ATOM_LOWER_ADDRESS) {
        return lm_trans_put(file, "&");
    }
    if (lowering.kind == LM_TRANS_EXPR_ATOM_LOWER_POINTER_FOLLOW) {
        return lm_trans_put(file, "->");
    }
    if (lowering.kind == LM_TRANS_EXPR_ATOM_LOWER_DEREF) {
        return lm_trans_put(file, "*");
    }
    if (
        lowering.kind == LM_TRANS_EXPR_ATOM_LOWER_INDEX_OPERATOR ||
        lowering.kind == LM_TRANS_EXPR_ATOM_LOWER_INFIX_OPERATOR
    ) {
        return lm_trans_write_text(file, lowering.text);
    }
    if (lowering.kind == LM_TRANS_EXPR_ATOM_LOWER_C_SURFACE) {
        return lm_trans_emit_name(file, lowering.text);
    }
    return lm_trans_emit_expr_atom(file, lowering.text, namespace_);
}

static int lm_trans_expr_lowered_range_append_atom(
    LmTransExprLoweredRange *range,
    int leading_space,
    const LmP0Node *node,
    LmTransExprAtomLowering lowering
) {
    LmTransExprPiece piece;

    piece.kind = LM_TRANS_EXPR_PIECE_ATOM;
    piece.leading_space = leading_space;
    piece.node = node;
    piece.atom = lowering;
    piece.first = 0;
    piece.stop = 0;
    return lm_own_value_stack_push(&range->pieces, &piece);
}

static int lm_trans_expr_lowered_range_append_node(
    LmTransExprLoweredRange *range,
    int leading_space,
    const LmP0Node *node
) {
    LmTransExprPiece piece;

    piece.kind = LM_TRANS_EXPR_PIECE_NODE;
    piece.leading_space = leading_space;
    piece.node = node;
    piece.atom.kind = LM_TRANS_EXPR_ATOM_LOWER_VALUE;
    piece.atom.text.data = 0;
    piece.atom.text.length = 0U;
    piece.first = 0;
    piece.stop = 0;
    return lm_own_value_stack_push(&range->pieces, &piece);
}

static int lm_trans_expr_lowered_range_append_index(
    LmTransExprLoweredRange *range,
    const LmP0Field *first,
    const LmP0Field *stop
) {
    LmTransExprPiece piece;

    piece.kind = LM_TRANS_EXPR_PIECE_INDEX;
    piece.leading_space = 0;
    piece.node = 0;
    piece.atom.kind = LM_TRANS_EXPR_ATOM_LOWER_VALUE;
    piece.atom.text.data = 0;
    piece.atom.text.length = 0U;
    piece.first = first;
    piece.stop = stop;
    return lm_own_value_stack_push(&range->pieces, &piece);
}

static int lm_trans_field_is_atom(const LmP0Field *field, const char *spelling) {
    return
        field != 0 &&
        field->value != 0 &&
        field->value->kind == LM_P0_NODE_ATOM &&
        lm_trans_text_equals(field->value->as.atom, spelling);
}

static const LmP0Field *lm_trans_find_matching_index_close(
    const LmP0Field *open,
    const LmP0Field *stop
) {
    const LmP0Field *field;
    int depth;

    if (!lm_trans_field_is_atom(open, "[")) {
        return 0;
    }

    depth = 1;
    field = open->next;
    while (field != stop) {
        if (lm_trans_field_is_atom(field, "[")) {
            ++depth;
        } else if (lm_trans_field_is_atom(field, "]")) {
            --depth;
            if (depth == 0) {
                return field;
            }
        }
        field = field->next;
    }

    return 0;
}

static int lm_trans_expr_lowered_range_build(
    LmTransExprLoweredRange *lowered,
    LmTransExprRangeJob range
) {
    const LmP0Field *field;
    const LmP0Node *node;
    const LmP0Node *previous_operand;
    int expect_field_name;
    int expect_c_field_name;
    int c_dot_path;
    LmTransExprAtomLowering atom;

    if (lowered == 0) {
        return 1;
    }

    field = range.field;
    previous_operand = range.previous_operand;
    expect_field_name = range.expect_field_name;
    expect_c_field_name = range.expect_c_field_name;
    c_dot_path = range.c_dot_path;
    while (field != range.stop) {
        node = field->value;
        if (!lm_trans_node_is_ignored(node)) {
            if (lm_trans_field_is_atom(field, "[")) {
                const LmP0Field *close;

                if (previous_operand == 0) {
                    fprintf(stderr, "trans L2 error: index operator expects a target expression\n");
                    return 1;
                }
                close = lm_trans_find_matching_index_close(field, range.stop);
                if (close == 0) {
                    fprintf(stderr, "trans L2 error: unclosed index operator\n");
                    return 1;
                }
                if (lm_trans_expr_lowered_range_append_index(lowered, field->next, close) != 0) {
                    return 1;
                }
                previous_operand = close->value;
                expect_field_name = 0;
                expect_c_field_name = 0;
                c_dot_path = 0;
                range.wrote = 1;
                field = close->next;
                continue;
            } else if (node->kind == LM_P0_NODE_ATOM) {
                atom = lm_trans_lower_expr_atom(
                    node,
                    previous_operand,
                    expect_field_name,
                    expect_c_field_name,
                    c_dot_path
                );
                if (
                    lm_trans_update_expr_atom_lowering_state(
                        atom,
                        node,
                        &previous_operand,
                        &expect_field_name,
                        &expect_c_field_name,
                        &c_dot_path
                    ) != 0
                ) {
                    return 1;
                }
                if (
                    lm_trans_expr_lowered_range_append_atom(
                        lowered,
                        range.wrote,
                        node,
                        atom
                    ) != 0
                ) {
                    return 1;
                }
            } else {
                if (
                    lm_trans_expr_lowered_range_append_node(
                        lowered,
                        range.wrote,
                        node
                    ) != 0
                ) {
                    return 1;
                }
                previous_operand = node;
                expect_field_name = 0;
                expect_c_field_name = 0;
                c_dot_path = 0;
            }
            range.wrote = 1;
        }
        field = field->next;
    }

    if (expect_field_name) {
        fprintf(stderr, "trans L2 error: field-follow expects a field name\n");
        return 1;
    }
    if (expect_c_field_name) {
        fprintf(stderr, "trans L2 error: C value-field dot expects a field name\n");
        return 1;
    }

    return 0;
}

static int lm_trans_expr_stack_emit_lowered_range(
    FILE *file,
    LmTransExprStack *stack,
    LmTransExprLoweredRange *lowered,
    const LmTransNamespace *namespace_
) {
    const LmTransExprPiece *piece;
    LmTransExprJob discard;

    if (lowered == 0) {
        return 0;
    }

    while (lowered->index < lowered->pieces.count) {
        piece = (const LmTransExprPiece *)lm_own_value_stack_at(&lowered->pieces, lowered->index);
        if (piece == 0) {
            lm_trans_expr_lowered_range_delete(lowered);
            return 1;
        }
        if (piece->leading_space && lm_trans_put(file, " ") != 0) {
            lm_trans_expr_lowered_range_delete(lowered);
            return 1;
        }
        if (piece->kind == LM_TRANS_EXPR_PIECE_ATOM) {
            if (lm_trans_emit_expr_atom_lowering(file, piece->atom, namespace_) != 0) {
                lm_trans_expr_lowered_range_delete(lowered);
                return 1;
            }
            ++lowered->index;
        } else if (piece->kind == LM_TRANS_EXPR_PIECE_NODE) {
            ++lowered->index;
            if (lm_trans_expr_stack_push_lowered_range(stack, lowered) != 0) {
                lm_trans_expr_lowered_range_delete(lowered);
                return 1;
            }
            if (lm_trans_expr_stack_push_node(stack, piece->node) != 0) {
                (void)lm_own_value_stack_pop(&stack->jobs, &discard);
                lm_trans_expr_lowered_range_delete(lowered);
                return 1;
            }
            return 0;
        } else if (piece->kind == LM_TRANS_EXPR_PIECE_INDEX) {
            if (lm_trans_put(file, "[") != 0) {
                lm_trans_expr_lowered_range_delete(lowered);
                return 1;
            }
            ++lowered->index;
            if (lm_trans_expr_stack_push_lowered_range(stack, lowered) != 0) {
                lm_trans_expr_lowered_range_delete(lowered);
                return 1;
            }
            if (lm_trans_expr_stack_push_text(stack, "]") != 0) {
                (void)lm_own_value_stack_pop(&stack->jobs, &discard);
                lm_trans_expr_lowered_range_delete(lowered);
                return 1;
            }
            if (lm_trans_expr_stack_push_range(stack, piece->first, piece->stop) != 0) {
                (void)lm_own_value_stack_pop(&stack->jobs, &discard);
                (void)lm_own_value_stack_pop(&stack->jobs, &discard);
                lm_trans_expr_lowered_range_delete(lowered);
                return 1;
            }
            return 0;
        }
    }

    lm_trans_expr_lowered_range_delete(lowered);
    return 0;
}

static int lm_trans_expr_stack_emit_range(
    FILE *file,
    LmTransExprStack *stack,
    LmTransExprRangeJob range,
    const LmTransNamespace *namespace_
) {
    LmTransExprLoweredRange *lowered;

    lowered = lm_trans_expr_lowered_range_new();
    if (lowered == 0) {
        return 1;
    }
    if (lm_trans_expr_lowered_range_build(lowered, range) != 0) {
        lm_trans_expr_lowered_range_delete(lowered);
        return 1;
    }
    return lm_trans_expr_stack_emit_lowered_range(file, stack, lowered, namespace_);
}

static int lm_trans_emit_expr_stack_run(
    FILE *file,
    LmTransExprJob initial,
    const LmTransNamespace *namespace_
) {
    LmTransExprStack stack;
    LmTransExprJob job;
    int status;

    lm_own_value_stack_init(&stack.jobs, sizeof(LmTransExprJob));
    if (lm_trans_expr_stack_push(&stack, initial) != 0) {
        return 1;
    }

    status = 0;
    while (status == 0 && stack.jobs.count > 0U) {
        if (lm_own_value_stack_pop(&stack.jobs, &job) != 0) {
            status = 1;
            break;
        }
        if (job.kind == LM_TRANS_EXPR_JOB_TEXT) {
            status = lm_trans_put(file, job.as.text);
        } else if (job.kind == LM_TRANS_EXPR_JOB_NODE) {
            status = lm_trans_expr_stack_emit_node(file, &stack, job.as.node, namespace_);
        } else if (job.kind == LM_TRANS_EXPR_JOB_FRAME) {
            status = lm_trans_expr_stack_emit_frame(file, &stack, job.as.frame, namespace_);
        } else if (job.kind == LM_TRANS_EXPR_JOB_RANGE) {
            status = lm_trans_expr_stack_emit_range(file, &stack, job.as.range, namespace_);
        } else if (job.kind == LM_TRANS_EXPR_JOB_CALL_ARGS) {
            status = lm_trans_expr_stack_schedule_call_args(
                &stack,
                job.as.call_args.body,
                job.as.call_args.callee
            );
        } else if (job.kind == LM_TRANS_EXPR_JOB_LOWERED_RANGE) {
            status = lm_trans_expr_stack_emit_lowered_range(
                file,
                &stack,
                job.as.lowered_range,
                namespace_
            );
        }
    }

    lm_trans_expr_stack_destroy(&stack);
    return status;
}

static int lm_trans_emit_call_args(
    FILE *file,
    const LmP0Structure *body,
    const LmTransNamespace *namespace_,
    const LmTransSymbol *callee
) {
    LmTransExprJob job;

    job.kind = LM_TRANS_EXPR_JOB_CALL_ARGS;
    job.as.call_args.body = body;
    job.as.call_args.callee = callee;
    return lm_trans_emit_expr_stack_run(file, job, namespace_);
}

static int lm_trans_emit_expr_list(
    FILE *file,
    const LmP0Field *first,
    const LmTransNamespace *namespace_
) {
    const LmP0Field *field;
    const LmP0Field *next;
    int wrote;

    wrote = 0;
    field = first;
    while (field != 0) {
        if (wrote && lm_trans_put(file, ", ") != 0) {
            return 1;
        }
        next = lm_trans_expr_segment_end(field);
        if (lm_trans_emit_expr_range(file, field, next, namespace_) != 0) {
            return 1;
        }
        wrote = 1;
        field = next;
    }

    return 0;
}

static int lm_trans_emit_expr_node(FILE *file, const LmP0Node *node, const LmTransNamespace *namespace_) {
    LmTransExprJob job;

    job.kind = LM_TRANS_EXPR_JOB_NODE;
    job.as.node = node;
    return lm_trans_emit_expr_stack_run(file, job, namespace_);
}

static int lm_trans_emit_expr_fields(FILE *file, const LmP0Field *first, const LmTransNamespace *namespace_) {
    return lm_trans_emit_expr_range(file, first, 0, namespace_);
}

static int lm_trans_emit_expr_range(
    FILE *file,
    const LmP0Field *first,
    const LmP0Field *stop,
    const LmTransNamespace *namespace_
) {
    LmTransExprJob job;

    job.kind = LM_TRANS_EXPR_JOB_RANGE;
    job.as.range.field = first;
    job.as.range.stop = stop;
    job.as.range.wrote = 0;
    job.as.range.previous_operand = 0;
    job.as.range.expect_field_name = 0;
    job.as.range.expect_c_field_name = 0;
    job.as.range.c_dot_path = 0;
    return lm_trans_emit_expr_stack_run(file, job, namespace_);
}

static int lm_trans_emit_type_node(FILE *file, const LmP0Node *type_node) {
    const LmP0Field *field;
    size_t i;

    if (type_node == 0) {
        fprintf(stderr, "trans L2 error: type position expects a name\n");
        return 1;
    }

    if (type_node->kind == LM_P0_NODE_ATOM) {
        return lm_trans_emit_type_name(file, type_node->as.atom);
    }

    if (
        type_node->kind == LM_P0_NODE_FRAME &&
        lm_trans_text_equals(type_node->as.frame.head, "const")
    ) {
        field = type_node->as.frame.body.first_field;
        if (field == 0 || field->next != 0 || field->value == 0) {
            fprintf(stderr, "trans L2 error: const type qualifier expects exactly one type\n");
            return 1;
        }
        if (lm_trans_put(file, "const ") != 0) {
            return 1;
        }
        return lm_trans_emit_type_node(file, field->value);
    }

    if (
        type_node->kind == LM_P0_NODE_FRAME &&
        lm_trans_text_all_char(type_node->as.frame.head, '@')
    ) {
        field = type_node->as.frame.body.first_field;
        if (field == 0 || field->next != 0 || field->value == 0) {
            fprintf(stderr, "trans L2 error: pointer type expects exactly one type\n");
            return 1;
        }
        if (lm_trans_emit_type_node(file, field->value) != 0) {
            return 1;
        }
        if (lm_trans_put(file, " ") != 0) {
            return 1;
        }
        for (i = 0U; i < type_node->as.frame.head.length; ++i) {
            if (lm_trans_put(file, "*") != 0) {
                return 1;
            }
        }
        return 0;
    }

    fprintf(stderr, "trans L2 error: type position expects a name\n");
    return 1;
}

static int lm_trans_emit_function_return_struct_type_name(FILE *file, LmP0Text function_name) {
    if (lm_trans_write_text(file, function_name) != 0) {
        return 1;
    }
    return lm_trans_put(file, "Return");
}

static int lm_trans_emit_current_return_type(FILE *file, const LmTransNamespace *namespace_) {
    if (namespace_ != 0 && namespace_->return_type_is_struct) {
        if (namespace_->return_type_name == 0) {
            return 1;
        }
        return lm_trans_emit_function_return_struct_type_name(file, *namespace_->return_type_name);
    }
    return lm_trans_emit_type_node(file, namespace_ != 0 ? namespace_->return_type_node : 0);
}

static int lm_trans_emit_type_and_name(
    FILE *file,
    const LmP0Node *type_node,
    LmP0Text name,
    size_t pointer_depth,
    const LmTransNamespace *namespace_
) {
    size_t i;

    if (type_node == 0) {
        return 1;
    }

    (void)namespace_;

    if (lm_trans_emit_type_node(file, type_node) != 0) {
        return 1;
    }
    if (lm_trans_put(file, " ") != 0) {
        return 1;
    }
    for (i = 0U; i < pointer_depth; ++i) {
        if (lm_trans_put(file, "*") != 0) {
            return 1;
        }
    }
    return lm_trans_write_text(file, name);
}

static int lm_trans_head_can_declare_storage(
    LmP0Text head,
    const LmTransNamespace *namespace_
);

static int lm_trans_emit_param(FILE *file, const LmP0Node *node, LmTransNamespace *namespace_) {
    const LmP0Field *field0;
    const LmP0Field *field1;
    const LmP0Field *inner_field;
    const LmP0Node *param_node;
    const LmP0Node *name_node;

    if (node == 0 || node->kind != LM_P0_NODE_FRAME) {
        fprintf(stderr, "trans L2 error: parameter must be a typed frame\n");
        return 1;
    }

    if (lm_trans_text_equals(node->as.frame.head, "const")) {
        field0 = node->as.frame.body.first_field;
        param_node = 0;
        if (
            field0 != 0 &&
            field0->next == 0 &&
            field0->value != 0
        ) {
            if (field0->value->kind == LM_P0_NODE_FRAME) {
                param_node = field0->value;
            } else if (field0->value->kind == LM_P0_NODE_STRUCTURE) {
                inner_field = field0->value->as.structure.first_field;
                if (
                    inner_field != 0 &&
                    inner_field->next == 0 &&
                    inner_field->value != 0 &&
                    inner_field->value->kind == LM_P0_NODE_FRAME
                ) {
                    param_node = inner_field->value;
                }
            }
        }
        if (param_node == 0) {
            fprintf(stderr, "trans L2 error: const parameter expects exactly one parameter declaration\n");
            return 1;
        }
        if (lm_trans_put(file, "const ") != 0) {
            return 1;
        }
        return lm_trans_emit_param(file, param_node, namespace_);
    }

    field0 = lm_trans_nth_field(&node->as.frame.body, 0U);
    field1 = lm_trans_nth_field(&node->as.frame.body, 1U);

    if (lm_trans_head_can_declare_storage(node->as.frame.head, namespace_)) {
        if (field0 == 0 || field0->value == 0 || field0->value->kind != LM_P0_NODE_ATOM) {
            fprintf(stderr, "trans L2 error: typed parameter expects a name\n");
            return 1;
        }
        name_node = field0->value;
        if (lm_trans_emit_name(file, node->as.frame.head) != 0) {
            return 1;
        }
        if (lm_trans_put(file, " ") != 0) {
            return 1;
        }
        if (lm_trans_write_text(file, name_node->as.atom) != 0) {
            return 1;
        }
        return lm_trans_namespace_declare(namespace_, name_node->as.atom, LM_TRANS_SYMBOL_VARIABLE);
    }

    if (lm_trans_text_all_char(node->as.frame.head, '@')) {
        if (
            field0 == 0 ||
            field1 == 0 ||
            field1->value == 0 ||
            field1->value->kind != LM_P0_NODE_ATOM
        ) {
            fprintf(stderr, "trans L2 error: @ parameter expects type and name\n");
            return 1;
        }
        name_node = field1->value;
        if (
            lm_trans_emit_type_and_name(
                file,
                field0->value,
                name_node->as.atom,
                node->as.frame.head.length,
                namespace_
            ) != 0
        ) {
            return 1;
        }
        return lm_trans_namespace_declare(namespace_, name_node->as.atom, LM_TRANS_SYMBOL_VARIABLE);
    }

    fprintf(stderr, "trans L2 error: unsupported parameter head\n");
    return 1;
}

static int lm_trans_emit_statement_list(
    FILE *file,
    const LmP0Field *first,
    unsigned indent,
    LmTransNamespace *namespace_
);

static const LmP0Field *lm_trans_control_body_start(const LmP0Frame *frame) {
    const LmP0Field *end;

    if (frame == 0 || frame->body.first_field == 0) {
        return 0;
    }

    end = lm_trans_expr_segment_end(frame->body.first_field);
    if (end != 0 && end->value != 0 && end->value->kind == LM_P0_NODE_STRUCTURE) {
        return end;
    }

    return 0;
}

static int lm_trans_emit_control_condition(
    FILE *file,
    const LmP0Frame *frame,
    const LmTransNamespace *namespace_
) {
    const LmP0Field *body_start;
    const LmP0Field *first;

    first = frame->body.first_field;
    if (first == 0) {
        return 0;
    }

    body_start = lm_trans_control_body_start(frame);
    return lm_trans_emit_expr_range(file, first, body_start, namespace_);
}

static int lm_trans_frame_has_positional_name_argument(const LmP0Frame *frame) {
    int registry_result;

    if (frame == 0) {
        return 0;
    }

    registry_result = lm_trans_registry_has(frame->head, "receiver.positionalName");
    return lm_trans_bool_compare(
        "receiver.positionalName",
        frame->head,
        registry_result,
        lm_trans_legacy_frame_has_positional_name_argument(frame)
    );
}

static int lm_trans_name_argument_from_frame(const LmP0Frame *frame, LmP0Text *out_name) {
    const LmP0Field *field;
    const LmP0Field *name_field;
    const LmP0Frame *child_frame;

    if (frame == 0 || out_name == 0) {
        return 0;
    }

    field = frame->body.first_field;
    while (field != 0) {
        if (field->value != 0 && field->value->kind == LM_P0_NODE_FRAME) {
            child_frame = &field->value->as.frame;
            if (lm_trans_text_equals(child_frame->head, "name")) {
                name_field = child_frame->body.first_field;
                if (
                    name_field != 0 &&
                    name_field->next == 0 &&
                    name_field->value != 0 &&
                    name_field->value->kind == LM_P0_NODE_ATOM &&
                    lm_trans_name_argument_is_valid(name_field->value->as.atom)
                ) {
                    *out_name = name_field->value->as.atom;
                    return 1;
                }
            }
        }
        field = field->next;
    }

    if (!lm_trans_frame_has_positional_name_argument(frame)) {
        return 0;
    }

    field = lm_trans_nth_field(&frame->body, 0U);
    if (field == 0 || field->value == 0) {
        return 0;
    }

    if (
        field->value->kind == LM_P0_NODE_ATOM &&
        lm_trans_name_argument_is_valid(field->value->as.atom)
    ) {
        *out_name = field->value->as.atom;
        return 1;
    }

    return 0;
}

static int lm_trans_frame_close_target(const LmP0Frame *frame, LmP0Text *out_target) {
    if (frame == 0 || out_target == 0) {
        return 0;
    }

    if (lm_trans_name_argument_from_frame(frame, out_target)) {
        return 1;
    }

    *out_target = frame->head;
    return 1;
}

static int lm_trans_validate_end_trailer(const LmP0Frame *frame) {
    LmP0Text actual;
    LmP0Text expected;

    if (frame == 0 || frame->trailer == 0) {
        return 0;
    }

    if (!lm_trans_text_equals(frame->trailer->spelling, "end")) {
        return 0;
    }

    if (!lm_trans_trailer_single_atom(frame->trailer, &actual)) {
        fprintf(stderr, "trans error: end trailer expects exactly one target name\n");
        return 1;
    }

    if (!lm_trans_frame_close_target(frame, &expected)) {
        fprintf(
            stderr,
            "trans error: head \"%.*s\" does not expose a named close target\n",
            (int)frame->head.length,
            frame->head.data
        );
        return 1;
    }

    if (!lm_trans_text_same(actual, expected)) {
        fprintf(
            stderr,
            "trans error: end target \"%.*s\" does not match close target \"%.*s\"\n",
            (int)actual.length,
            actual.data,
            (int)expected.length,
            expected.data
        );
        return 1;
    }

    return 0;
}

static int lm_trans_emit_cleanups_until(
    FILE *file,
    unsigned indent,
    const LmTransNamespace *namespace_,
    size_t cleanup_base
) {
    size_t index;
    const LmTransCleanup *cleanup;

    if (namespace_ == 0 || cleanup_base > namespace_->cleanups.count) {
        return 0;
    }

    index = namespace_->cleanups.count;
    while (index > cleanup_base) {
        --index;
        cleanup = (const LmTransCleanup *)lm_own_ptr_stack_at(&namespace_->cleanups, index);
        if (cleanup == 0) {
            return 1;
        }
        if (lm_trans_emit_indent(file, indent) != 0) {
            return 1;
        }
        if (lm_trans_put(file, "lm_synchronized_leave(") != 0) {
            return 1;
        }
        if (lm_trans_emit_sync_name(file, cleanup->id) != 0) {
            return 1;
        }
        if (lm_trans_put(file, ");\n") != 0) {
            return 1;
        }
    }

    return 0;
}

static int lm_trans_emit_return_statement(
    FILE *file,
    const LmP0Field *return_fields,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    unsigned return_id;

    if (return_fields == 0) {
        if (lm_trans_emit_cleanups_until(file, indent, namespace_, 0U) != 0) {
            return 1;
        }
        if (lm_trans_emit_indent(file, indent) != 0) {
            return 1;
        }
        return lm_trans_put(file, "return;\n");
    }

    if (namespace_ != 0 && namespace_->cleanups.count > 0U && namespace_->return_type_node != 0) {
        return_id = namespace_->next_return_id++;
        if (lm_trans_emit_indent(file, indent) != 0) {
            return 1;
        }
        if (lm_trans_put(file, "{\n") != 0) {
            return 1;
        }
        if (lm_trans_emit_indent(file, indent + 1U) != 0) {
            return 1;
        }
        if (lm_trans_emit_current_return_type(file, namespace_) != 0) {
            return 1;
        }
        if (lm_trans_put(file, " ") != 0) {
            return 1;
        }
        if (lm_trans_emit_return_name(file, return_id) != 0) {
            return 1;
        }
        if (lm_trans_put(file, " = ") != 0) {
            return 1;
        }
        if (lm_trans_emit_expr_fields(file, return_fields, namespace_) != 0) {
            return 1;
        }
        if (lm_trans_put(file, ";\n") != 0) {
            return 1;
        }
        if (lm_trans_emit_cleanups_until(file, indent + 1U, namespace_, 0U) != 0) {
            return 1;
        }
        if (lm_trans_emit_indent(file, indent + 1U) != 0) {
            return 1;
        }
        if (lm_trans_put(file, "return ") != 0) {
            return 1;
        }
        if (lm_trans_emit_return_name(file, return_id) != 0) {
            return 1;
        }
        if (lm_trans_put(file, ";\n") != 0) {
            return 1;
        }
        if (lm_trans_emit_indent(file, indent) != 0) {
            return 1;
        }
        return lm_trans_put(file, "}\n");
    }

    if (lm_trans_emit_cleanups_until(file, indent, namespace_, 0U) != 0) {
        return 1;
    }
    if (lm_trans_emit_indent(file, indent) != 0) {
        return 1;
    }
    if (lm_trans_put(file, "return ") != 0) {
        return 1;
    }
    if (lm_trans_emit_expr_fields(file, return_fields, namespace_) != 0) {
        return 1;
    }
    return lm_trans_put(file, ";\n");
}

static int lm_trans_emit_trailer_statement(
    FILE *file,
    const LmP0Trailer *trailer,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    if (trailer == 0) {
        return 0;
    }

    if (lm_trans_text_equals(trailer->spelling, "return")) {
        return lm_trans_emit_return_statement(file, trailer->body.first_field, indent, namespace_);
    }

    if (lm_trans_text_equals(trailer->spelling, "end")) {
        return 0;
    }

    fprintf(
        stderr,
        "trans error: unsupported trailer \"%.*s\"\n",
        (int)trailer->spelling.length,
        trailer->spelling.data
    );
    return 1;
}

static int lm_trans_emit_declaration_with_qualifier(
    FILE *file,
    const LmP0Frame *frame,
    unsigned indent,
    LmTransNamespace *namespace_,
    const char *qualifier
) {
    const LmP0Field *name_field;
    const LmP0Node *name_node;

    name_field = lm_trans_nth_field(&frame->body, 0U);
    if (name_field == 0 || name_field->value == 0 || name_field->value->kind != LM_P0_NODE_ATOM) {
        fprintf(stderr, "trans L2 error: declaration expects a name\n");
        return 1;
    }

    name_node = name_field->value;
    if (lm_trans_emit_indent(file, indent) != 0) {
        return 1;
    }
    if (qualifier != 0 && lm_trans_put(file, qualifier) != 0) {
        return 1;
    }
    if (lm_trans_builtin_c_type_name(frame->head)) {
        if (lm_trans_emit_type_name(file, frame->head) != 0) {
            return 1;
        }
    } else {
        if (lm_trans_emit_name(file, frame->head) != 0) {
            return 1;
        }
    }
    if (lm_trans_put(file, " ") != 0) {
        return 1;
    }
    if (lm_trans_write_text(file, name_node->as.atom) != 0) {
        return 1;
    }
    if (name_field->next != 0) {
        if (lm_trans_put(file, " = ") != 0) {
            return 1;
        }
        if (lm_trans_emit_expr_fields(file, name_field->next, namespace_) != 0) {
            return 1;
        }
    }
    if (lm_trans_put(file, ";\n") != 0) {
        return 1;
    }
    return lm_trans_namespace_declare(namespace_, name_node->as.atom, LM_TRANS_SYMBOL_VARIABLE);
}

static int lm_trans_emit_declaration(
    FILE *file,
    const LmP0Frame *frame,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    return lm_trans_emit_declaration_with_qualifier(file, frame, indent, namespace_, "");
}

static int lm_trans_atom_can_be_new_binding_name(LmP0Text text) {
    return lm_trans_atom_is_identifier_like(text);
}

static int lm_trans_head_can_declare_storage(
    LmP0Text head,
    const LmTransNamespace *namespace_
) {
    const LmTransSymbol *symbol;

    if (lm_trans_builtin_c_type_name(head)) {
        return 1;
    }

    symbol = lm_trans_namespace_find(namespace_, head);
    return symbol != 0 && symbol->kind == LM_TRANS_SYMBOL_STRUCTURE;
}

static int lm_trans_frame_looks_storage_declaration(
    const LmP0Frame *frame,
    const LmTransNamespace *namespace_
) {
    const LmP0Field *name_field;

    if (
        frame == 0 ||
        !lm_trans_head_can_declare_storage(frame->head, namespace_)
    ) {
        return 0;
    }

    name_field = lm_trans_nth_field(&frame->body, 0U);
    if (
        name_field == 0 ||
        name_field->value == 0 ||
        name_field->value->kind != LM_P0_NODE_ATOM
    ) {
        return 0;
    }

    return lm_trans_atom_can_be_new_binding_name(name_field->value->as.atom);
}

static int lm_trans_emit_pointer_declaration_with_qualifier(
    FILE *file,
    const LmP0Frame *frame,
    unsigned indent,
    LmTransNamespace *namespace_,
    const char *qualifier
) {
    const LmP0Field *type_field;
    const LmP0Field *name_field;
    const LmP0Node *name_node;

    type_field = lm_trans_nth_field(&frame->body, 0U);
    name_field = lm_trans_nth_field(&frame->body, 1U);
    if (
        type_field == 0 ||
        name_field == 0 ||
        name_field->value == 0 ||
        name_field->value->kind != LM_P0_NODE_ATOM
    ) {
        fprintf(stderr, "trans L2 error: @ declaration expects type and name\n");
        return 1;
    }

    name_node = name_field->value;
    if (lm_trans_emit_indent(file, indent) != 0) {
        return 1;
    }
    if (qualifier != 0 && lm_trans_put(file, qualifier) != 0) {
        return 1;
    }
    if (
        lm_trans_emit_type_and_name(
            file,
            type_field->value,
            name_node->as.atom,
            frame->head.length,
            namespace_
        ) != 0
    ) {
        return 1;
    }
    if (name_field->next != 0) {
        if (lm_trans_put(file, " = ") != 0) {
            return 1;
        }
        if (lm_trans_emit_expr_fields(file, name_field->next, namespace_) != 0) {
            return 1;
        }
    }
    if (lm_trans_put(file, ";\n") != 0) {
        return 1;
    }
    return lm_trans_namespace_declare(namespace_, name_node->as.atom, LM_TRANS_SYMBOL_VARIABLE);
}

static int lm_trans_emit_pointer_declaration(
    FILE *file,
    const LmP0Frame *frame,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    return lm_trans_emit_pointer_declaration_with_qualifier(file, frame, indent, namespace_, "");
}

static int lm_trans_emit_array_declaration_with_qualifier(
    FILE *file,
    const LmP0Frame *frame,
    unsigned indent,
    LmTransNamespace *namespace_,
    const char *qualifier
) {
    const LmP0Field *field;
    const LmP0Field *type_field;
    const LmP0Field *name_field;
    const LmP0Field *size_field;
    const LmP0Node *name_node;
    size_t pointer_depth;

    field = frame->body.first_field;
    pointer_depth = 0U;
    while (
        field != 0 &&
        field->value != 0 &&
        field->value->kind == LM_P0_NODE_ATOM &&
        lm_trans_text_all_char(field->value->as.atom, '@')
    ) {
        pointer_depth += field->value->as.atom.length;
        field = field->next;
    }

    type_field = field;
    name_field = type_field != 0 ? type_field->next : 0;
    size_field = name_field != 0 ? name_field->next : 0;
    if (
        type_field == 0 ||
        name_field == 0 ||
        size_field == 0 ||
        name_field->value == 0 ||
        name_field->value->kind != LM_P0_NODE_ATOM
    ) {
        fprintf(stderr, "trans L2 error: [] declaration expects [@...] type name size\n");
        return 1;
    }

    name_node = name_field->value;
    if (lm_trans_emit_indent(file, indent) != 0) {
        return 1;
    }
    if (qualifier != 0 && lm_trans_put(file, qualifier) != 0) {
        return 1;
    }
    if (
        lm_trans_emit_type_and_name(
            file,
            type_field->value,
            name_node->as.atom,
            pointer_depth,
            namespace_
        ) != 0
    ) {
        return 1;
    }
    if (lm_trans_put(file, "[") != 0) {
        return 1;
    }
    if (lm_trans_emit_expr_node(file, size_field->value, namespace_) != 0) {
        return 1;
    }
    if (lm_trans_put(file, "]") != 0) {
        return 1;
    }
    if (size_field->next != 0) {
        if (lm_trans_put(file, " = {") != 0) {
            return 1;
        }
        if (lm_trans_emit_expr_list(file, size_field->next, namespace_) != 0) {
            return 1;
        }
        if (lm_trans_put(file, "}") != 0) {
            return 1;
        }
    }
    if (lm_trans_put(file, ";\n") != 0) {
        return 1;
    }
    return lm_trans_namespace_declare(namespace_, name_node->as.atom, LM_TRANS_SYMBOL_VARIABLE);
}

static int lm_trans_emit_array_declaration(
    FILE *file,
    const LmP0Frame *frame,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    return lm_trans_emit_array_declaration_with_qualifier(file, frame, indent, namespace_, "");
}

static int lm_trans_emit_const_statement(
    FILE *file,
    const LmP0Frame *frame,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    const LmP0Field *field;
    const LmP0Frame *inner;

    field = frame->body.first_field;
    if (
        field == 0 ||
        field->next != 0 ||
        field->value == 0 ||
        field->value->kind != LM_P0_NODE_FRAME
    ) {
        fprintf(stderr, "trans L2 error: const receiver expects exactly one declaration frame\n");
        return 1;
    }

    inner = &field->value->as.frame;
    if (lm_trans_text_equals(inner->head, "[]")) {
        return lm_trans_emit_array_declaration_with_qualifier(file, inner, indent, namespace_, "const ");
    }
    if (lm_trans_frame_looks_storage_declaration(inner, namespace_)) {
        return lm_trans_emit_declaration_with_qualifier(file, inner, indent, namespace_, "const ");
    }
    if (lm_trans_text_all_char(inner->head, '@')) {
        return lm_trans_emit_pointer_declaration_with_qualifier(file, inner, indent, namespace_, "const ");
    }

    fprintf(stderr, "trans L2 error: const receiver expects storage, pointer, or array declaration\n");
    return 1;
}

static int lm_trans_emit_call_statement(
    FILE *file,
    const LmP0Frame *frame,
    unsigned indent,
    const LmTransNamespace *namespace_
) {
    LmTransCallLowering call;

    if (lm_trans_lower_call(frame->head, namespace_, LM_TRANS_CALL_LOWER_STATEMENT, &call) != 0) {
        return 1;
    }

    if (lm_trans_emit_indent(file, indent) != 0) {
        return 1;
    }
    if (lm_trans_emit_name(file, call.name) != 0) {
        return 1;
    }
    if (lm_trans_put(file, "(") != 0) {
        return 1;
    }
    if (lm_trans_emit_call_args(file, &frame->body, namespace_, call.signature) != 0) {
        return 1;
    }
    if (lm_trans_put(file, ")") != 0) {
        return 1;
    }
    return lm_trans_put(file, ";\n");
}

static int lm_trans_text_contains_char(LmP0Text text, char ch) {
    size_t i;

    i = 0U;
    while (i < text.length) {
        if (text.data[i] == ch) {
            return 1;
        }
        ++i;
    }
    return 0;
}

static int lm_trans_head_looks_assignable_target(LmP0Text head) {
    return
        lm_trans_text_contains_char(head, '[') ||
        lm_trans_text_contains_char(head, '\\') ||
        lm_trans_c_reference_has_path_dot(head);
}

static int lm_trans_emit_assignment_target(FILE *file, LmP0Text target) {
    size_t i;

    if (!lm_trans_is_c_reference_name(target) && lm_trans_text_contains_char(target, '.')) {
        fprintf(stderr, "trans L2 error: dot belongs only to c.* ANSI C surface names; use \\ for Lingvamyxa structures\n");
        return 1;
    }

    i = 0U;
    if (lm_trans_is_c_reference_name(target)) {
        i = 2U;
    }
    while (i < target.length) {
        if (target.data[i] == '\\') {
            if (lm_trans_put(file, "->") != 0) {
                return 1;
            }
        } else if (lm_trans_write_all(file, target.data + i, 1U) != 0) {
            return 1;
        }
        ++i;
    }
    return 0;
}

static int lm_trans_emit_target_assignment(
    FILE *file,
    const LmP0Frame *frame,
    unsigned indent,
    const LmTransNamespace *namespace_
) {
    if (lm_trans_emit_indent(file, indent) != 0) {
        return 1;
    }
    if (lm_trans_emit_assignment_target(file, frame->head) != 0) {
        return 1;
    }
    if (lm_trans_put(file, " = ") != 0) {
        return 1;
    }
    if (lm_trans_emit_expr_fields(file, frame->body.first_field, namespace_) != 0) {
        return 1;
    }
    return lm_trans_put(file, ";\n");
}

static int lm_trans_emit_assignment(
    FILE *file,
    const LmP0Frame *frame,
    unsigned indent,
    const LmTransNamespace *namespace_
) {
    if (lm_trans_emit_indent(file, indent) != 0) {
        return 1;
    }
    if (lm_trans_write_text(file, frame->head) != 0) {
        return 1;
    }
    if (lm_trans_put(file, " = ") != 0) {
        return 1;
    }
    if (lm_trans_emit_expr_fields(file, frame->body.first_field, namespace_) != 0) {
        return 1;
    }
    return lm_trans_put(file, ";\n");
}

static int lm_trans_frame_looks_named_structure_declaration(const LmP0Frame *frame) {
    return
        frame != 0 &&
        !lm_trans_is_c_reference_name(frame->head) &&
        !lm_trans_is_reserved_head_name(frame->head) &&
        (frame->flags & LM_P0_FRAME_COLON) != 0U &&
        (frame->flags & LM_P0_FRAME_INLINE_BODY) == 0U;
}

static int lm_trans_emit_struct_field_with_qualifier(
    FILE *file,
    const LmP0Node *node,
    unsigned indent,
    const char *qualifier
) {
    const LmP0Frame *frame;
    const LmP0Field *type_field;
    const LmP0Field *name_field;
    const LmP0Node *name_node;

    if (lm_trans_node_is_ignored(node)) {
        return 0;
    }

    if (node->kind != LM_P0_NODE_FRAME) {
        fprintf(stderr, "trans L2 error: structure field must be a named field\n");
        return 1;
    }

    frame = &node->as.frame;
    if (lm_trans_text_equals(frame->head, "const")) {
        name_field = frame->body.first_field;
        if (
            name_field == 0 ||
            name_field->next != 0 ||
            name_field->value == 0 ||
            name_field->value->kind != LM_P0_NODE_FRAME
        ) {
            fprintf(stderr, "trans L2 error: const structure field expects exactly one field declaration\n");
            return 1;
        }
        (void)qualifier;
        return lm_trans_emit_struct_field_with_qualifier(file, name_field->value, indent, "const ");
    }

    if (lm_trans_text_all_char(frame->head, '@')) {
        type_field = lm_trans_nth_field(&frame->body, 0U);
        name_field = lm_trans_nth_field(&frame->body, 1U);
        if (
            type_field == 0 ||
            name_field == 0 ||
            name_field->next != 0 ||
            name_field->value == 0 ||
            name_field->value->kind != LM_P0_NODE_ATOM
        ) {
            fprintf(stderr, "trans L2 error: pointer structure field expects type and name\n");
            return 1;
        }
        name_node = name_field->value;
        if (lm_trans_emit_indent(file, indent) != 0) {
            return 1;
        }
        if (qualifier != 0 && lm_trans_put(file, qualifier) != 0) {
            return 1;
        }
        if (
            lm_trans_emit_type_and_name(
                file,
                type_field->value,
                name_node->as.atom,
                frame->head.length,
                0
            ) != 0
        ) {
            return 1;
        }
        return lm_trans_put(file, ";\n");
    }

    name_field = lm_trans_nth_field(&frame->body, 0U);
    if (
        name_field == 0 ||
        name_field->next != 0 ||
        name_field->value == 0 ||
        name_field->value->kind != LM_P0_NODE_ATOM
    ) {
        fprintf(stderr, "trans L2 error: structure field expects Type: name\n");
        return 1;
    }

    if (lm_trans_emit_indent(file, indent) != 0) {
        return 1;
    }
    if (qualifier != 0 && lm_trans_put(file, qualifier) != 0) {
        return 1;
    }
    if (lm_trans_emit_name(file, frame->head) != 0) {
        return 1;
    }
    if (lm_trans_put(file, " ") != 0) {
        return 1;
    }
    if (lm_trans_write_text(file, name_field->value->as.atom) != 0) {
        return 1;
    }
    return lm_trans_put(file, ";\n");
}

static int lm_trans_emit_struct_field(
    FILE *file,
    const LmP0Node *node,
    unsigned indent
) {
    return lm_trans_emit_struct_field_with_qualifier(file, node, indent, "");
}

static int lm_trans_emit_named_structure(
    FILE *file,
    const LmP0Frame *frame
) {
    const LmP0Field *field;

    if (lm_trans_put(file, "typedef struct ") != 0) {
        return 1;
    }
    if (lm_trans_write_text(file, frame->head) != 0) {
        return 1;
    }
    if (lm_trans_put(file, " ") != 0) {
        return 1;
    }
    if (lm_trans_write_text(file, frame->head) != 0) {
        return 1;
    }
    if (lm_trans_put(file, ";\n\nstruct ") != 0) {
        return 1;
    }
    if (lm_trans_write_text(file, frame->head) != 0) {
        return 1;
    }
    if (lm_trans_put(file, " {\n") != 0) {
        return 1;
    }

    field = frame->body.first_field;
    while (field != 0) {
        if (lm_trans_emit_struct_field(file, field->value, 1U) != 0) {
            return 1;
        }
        field = field->next;
    }

    if (lm_trans_put(file, "};\n") != 0) {
        return 1;
    }
    return lm_trans_validate_end_trailer(frame);
}

static int lm_trans_emit_function_return_structure(
    FILE *file,
    LmP0Text function_name,
    const LmP0Node *return_node
) {
    const LmP0Field *field;

    if (return_node == 0 || return_node->kind != LM_P0_NODE_STRUCTURE) {
        fprintf(stderr, "trans L2 error: fm expects a return Structure\n");
        return 1;
    }

    if (lm_trans_put(file, "typedef struct ") != 0) {
        return 1;
    }
    if (lm_trans_emit_function_return_struct_type_name(file, function_name) != 0) {
        return 1;
    }
    if (lm_trans_put(file, " ") != 0) {
        return 1;
    }
    if (lm_trans_emit_function_return_struct_type_name(file, function_name) != 0) {
        return 1;
    }
    if (lm_trans_put(file, ";\n\nstruct ") != 0) {
        return 1;
    }
    if (lm_trans_emit_function_return_struct_type_name(file, function_name) != 0) {
        return 1;
    }
    if (lm_trans_put(file, " {\n") != 0) {
        return 1;
    }

    field = return_node->as.structure.first_field;
    if (field == 0) {
        fprintf(stderr, "trans L2 error: fm return Structure must not be empty\n");
        return 1;
    }
    while (field != 0) {
        if (lm_trans_emit_struct_field(file, field->value, 1U) != 0) {
            return 1;
        }
        field = field->next;
    }

    return lm_trans_put(file, "};\n\n");
}

static int lm_trans_frame_looks_label_declaration(const LmP0Frame *frame) {
    return lm_trans_frame_looks_named_structure_declaration(frame);
}

static void lm_trans_statement_stack_destroy(LmTransStatementStack *stack) {
    if (stack != 0) {
        lm_own_value_stack_destroy(&stack->jobs);
    }
}

static int lm_trans_statement_stack_push(LmTransStatementStack *stack, LmTransStatementJob job) {
    if (stack == 0) {
        return 1;
    }

    return lm_own_value_stack_push(&stack->jobs, &job);
}

static int lm_trans_statement_stack_push_list(
    LmTransStatementStack *stack,
    const LmP0Field *field,
    unsigned indent,
    int unwrap_single_structure
) {
    LmTransStatementJob job;

    job.kind = LM_TRANS_STATEMENT_JOB_LIST;
    job.as.list.field = field;
    job.as.list.indent = indent;
    job.as.list.unwrap_single_structure = unwrap_single_structure;
    return lm_trans_statement_stack_push(stack, job);
}

static int lm_trans_statement_stack_push_node(
    LmTransStatementStack *stack,
    const LmP0Node *node,
    unsigned indent
) {
    LmTransStatementJob job;

    job.kind = LM_TRANS_STATEMENT_JOB_NODE;
    job.as.node.node = node;
    job.as.node.indent = indent;
    return lm_trans_statement_stack_push(stack, job);
}

static int lm_trans_statement_stack_push_frame_job(
    LmTransStatementStack *stack,
    LmTransStatementJobKind kind,
    const LmP0Frame *frame,
    unsigned indent
) {
    LmTransStatementJob job;

    job.kind = kind;
    job.as.frame.frame = frame;
    job.as.frame.indent = indent;
    return lm_trans_statement_stack_push(stack, job);
}

static int lm_trans_statement_stack_push_simple(LmTransStatementStack *stack, LmTransStatementJobKind kind) {
    LmTransStatementJob job;

    job.kind = kind;
    return lm_trans_statement_stack_push(stack, job);
}

static int lm_trans_statement_stack_push_indent_text(
    LmTransStatementStack *stack,
    unsigned indent,
    const char *text
) {
    LmTransStatementJob job;

    job.kind = LM_TRANS_STATEMENT_JOB_INDENT_TEXT;
    job.as.text.indent = indent;
    job.as.text.text = text;
    return lm_trans_statement_stack_push(stack, job);
}

static int lm_trans_statement_stack_push_sync_leave(
    LmTransStatementStack *stack,
    unsigned indent,
    unsigned cleanup_id
) {
    LmTransStatementJob job;

    job.kind = LM_TRANS_STATEMENT_JOB_SYNC_LEAVE;
    job.as.sync_leave.indent = indent;
    job.as.sync_leave.cleanup_id = cleanup_id;
    return lm_trans_statement_stack_push(stack, job);
}

static const LmP0Field *lm_trans_statement_list_first_field(
    const LmP0Field *first,
    int unwrap_single_structure
) {
    if (
        unwrap_single_structure &&
        first != 0 &&
        first->next == 0 &&
        first->value != 0 &&
        first->value->kind == LM_P0_NODE_STRUCTURE
    ) {
        return first->value->as.structure.first_field;
    }

    return first;
}

static int lm_trans_statement_stack_schedule_control(
    FILE *file,
    LmTransStatementStack *stack,
    const LmP0Frame *frame,
    unsigned indent,
    LmTransNamespace *namespace_,
    const char *keyword
) {
    const LmP0Field *body_start;
    int is_loop;
    int status;

    is_loop = strcmp(keyword, "while") == 0;
    body_start = lm_trans_control_body_start(frame);
    if (lm_trans_emit_indent(file, indent) != 0) {
        return 1;
    }
    if (lm_trans_put(file, keyword) != 0 || lm_trans_put(file, " (") != 0) {
        return 1;
    }
    if (lm_trans_emit_control_condition(file, frame, namespace_) != 0) {
        return 1;
    }
    if (lm_trans_put(file, ") {\n") != 0) {
        return 1;
    }

    lm_trans_namespace_enter_scope(namespace_);
    if (is_loop && lm_trans_loop_push(namespace_) != 0) {
        lm_trans_namespace_leave_scope(namespace_);
        return 1;
    }

    status =
        lm_trans_statement_stack_push_indent_text(stack, indent, "}\n") ||
        lm_trans_statement_stack_push_simple(stack, LM_TRANS_STATEMENT_JOB_LEAVE_SCOPE) ||
        (is_loop && lm_trans_statement_stack_push_simple(stack, LM_TRANS_STATEMENT_JOB_LOOP_POP)) ||
        lm_trans_statement_stack_push_list(stack, body_start, indent + 1U, 1);
    if (status != 0) {
        if (is_loop) {
            lm_trans_loop_pop(namespace_);
        }
        lm_trans_namespace_leave_scope(namespace_);
        return 1;
    }

    return 0;
}

static int lm_trans_statement_stack_schedule_synchronized(
    FILE *file,
    LmTransStatementStack *stack,
    const LmP0Frame *frame,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    const LmP0Field *body_start;
    unsigned cleanup_id;
    int status;

    if (frame == 0 || frame->body.first_field == 0) {
        fprintf(stderr, "trans L2 error: synchronized expects an object expression\n");
        return 1;
    }

    cleanup_id = namespace_->next_cleanup_id++;
    body_start = lm_trans_control_body_start(frame);
    if (lm_trans_emit_indent(file, indent) != 0) {
        return 1;
    }
    if (lm_trans_put(file, "void *") != 0) {
        return 1;
    }
    if (lm_trans_emit_sync_name(file, cleanup_id) != 0) {
        return 1;
    }
    if (lm_trans_put(file, " = (void *)(") != 0) {
        return 1;
    }
    if (lm_trans_emit_control_condition(file, frame, namespace_) != 0) {
        return 1;
    }
    if (lm_trans_put(file, ");\n") != 0) {
        return 1;
    }
    if (lm_trans_emit_indent(file, indent) != 0) {
        return 1;
    }
    if (lm_trans_put(file, "lm_synchronized_enter(") != 0) {
        return 1;
    }
    if (lm_trans_emit_sync_name(file, cleanup_id) != 0) {
        return 1;
    }
    if (lm_trans_put(file, ");\n") != 0) {
        return 1;
    }
    if (lm_trans_emit_indent(file, indent) != 0) {
        return 1;
    }
    if (lm_trans_put(file, "{\n") != 0) {
        return 1;
    }

    lm_trans_namespace_enter_scope(namespace_);
    if (lm_trans_cleanup_push(namespace_, cleanup_id) != 0) {
        lm_trans_namespace_leave_scope(namespace_);
        return 1;
    }

    status =
        lm_trans_statement_stack_push_sync_leave(stack, indent, cleanup_id) ||
        lm_trans_statement_stack_push_indent_text(stack, indent, "}\n") ||
        lm_trans_statement_stack_push_simple(stack, LM_TRANS_STATEMENT_JOB_LEAVE_SCOPE) ||
        lm_trans_statement_stack_push_simple(stack, LM_TRANS_STATEMENT_JOB_CLEANUP_POP) ||
        lm_trans_statement_stack_push_list(stack, body_start, indent + 1U, 1);
    if (status != 0) {
        lm_trans_cleanup_pop(namespace_);
        lm_trans_namespace_leave_scope(namespace_);
        return 1;
    }

    return 0;
}

static int lm_trans_statement_stack_schedule_else(
    FILE *file,
    LmTransStatementStack *stack,
    const LmP0Frame *frame,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    int status;

    if (lm_trans_emit_indent(file, indent) != 0) {
        return 1;
    }
    if (lm_trans_put(file, "else {\n") != 0) {
        return 1;
    }

    lm_trans_namespace_enter_scope(namespace_);
    status =
        lm_trans_statement_stack_push_indent_text(stack, indent, "}\n") ||
        lm_trans_statement_stack_push_simple(stack, LM_TRANS_STATEMENT_JOB_LEAVE_SCOPE) ||
        lm_trans_statement_stack_push_list(stack, frame->body.first_field, indent + 1U, 1);
    if (status != 0) {
        lm_trans_namespace_leave_scope(namespace_);
        return 1;
    }

    return 0;
}

static int lm_trans_statement_stack_schedule_label(
    FILE *file,
    LmTransStatementStack *stack,
    const LmP0Frame *frame,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    int status;

    if (lm_trans_namespace_declare(namespace_, frame->head, LM_TRANS_SYMBOL_LABEL) != 0) {
        return 1;
    }
    if (lm_trans_emit_indent(file, indent) != 0) {
        return 1;
    }
    if (lm_trans_write_text(file, frame->head) != 0) {
        return 1;
    }
    if (lm_trans_put(file, ":\n") != 0) {
        return 1;
    }
    if (lm_trans_emit_indent(file, indent) != 0) {
        return 1;
    }
    if (lm_trans_put(file, "{\n") != 0) {
        return 1;
    }

    lm_trans_namespace_enter_scope(namespace_);
    status =
        lm_trans_statement_stack_push_indent_text(stack, indent, "}\n") ||
        lm_trans_statement_stack_push_simple(stack, LM_TRANS_STATEMENT_JOB_LEAVE_SCOPE) ||
        lm_trans_statement_stack_push_list(stack, frame->body.first_field, indent + 1U, 1);
    if (status != 0) {
        lm_trans_namespace_leave_scope(namespace_);
        return 1;
    }

    return 0;
}

static int lm_trans_registry_statement_lowering(
    LmP0Text head,
    LmTransStatementLowering *out
) {
    const char *payload;

    if (out == 0) {
        return 0;
    }

    payload = lm_trans_registry_lookup_compare(head, "receiver.statement", lm_trans_legacy_statement_payload(head));
    if (payload == 0) {
        return 0;
    }

    if (strcmp(payload, "return") == 0) {
        out->kind = LM_TRANS_STATEMENT_LOWER_RETURN;
    } else if (strcmp(payload, "if") == 0) {
        out->kind = LM_TRANS_STATEMENT_LOWER_IF;
    } else if (strcmp(payload, "while") == 0) {
        out->kind = LM_TRANS_STATEMENT_LOWER_WHILE;
    } else if (strcmp(payload, "else") == 0) {
        out->kind = LM_TRANS_STATEMENT_LOWER_ELSE;
    } else if (strcmp(payload, "synchronized") == 0) {
        out->kind = LM_TRANS_STATEMENT_LOWER_SYNCHRONIZED;
    } else if (strcmp(payload, "break") == 0) {
        out->kind = LM_TRANS_STATEMENT_LOWER_BREAK;
    } else if (strcmp(payload, "continue") == 0) {
        out->kind = LM_TRANS_STATEMENT_LOWER_CONTINUE;
    } else if (strcmp(payload, "const") == 0) {
        out->kind = LM_TRANS_STATEMENT_LOWER_CONST_DECLARATION;
    } else if (strcmp(payload, "array-declaration") == 0) {
        out->kind = LM_TRANS_STATEMENT_LOWER_ARRAY_DECLARATION;
    } else {
        return 0;
    }

    return 1;
}

static int lm_trans_lower_statement_frame(
    const LmP0Frame *frame,
    const LmTransNamespace *namespace_,
    LmTransStatementLowering *out
) {
    const LmTransSymbol *symbol;

    if (frame == 0 || out == 0) {
        return 1;
    }

    if (lm_trans_registry_statement_lowering(frame->head, out)) {
        return 0;
    }
    if (lm_trans_frame_looks_storage_declaration(frame, namespace_)) {
        out->kind = LM_TRANS_STATEMENT_LOWER_STORAGE_DECLARATION;
        return 0;
    }
    if (lm_trans_text_all_char(frame->head, '@')) {
        out->kind = LM_TRANS_STATEMENT_LOWER_POINTER_DECLARATION;
        return 0;
    }
    if (lm_trans_head_looks_assignable_target(frame->head)) {
        out->kind = LM_TRANS_STATEMENT_LOWER_TARGET_ASSIGNMENT;
        return 0;
    }
    if (lm_trans_frame_looks_label_declaration(frame)) {
        out->kind = LM_TRANS_STATEMENT_LOWER_LABEL;
        return 0;
    }
    if (lm_trans_is_c_reference_name(frame->head)) {
        out->kind = LM_TRANS_STATEMENT_LOWER_CALL;
        return 0;
    }

    symbol = lm_trans_namespace_find(namespace_, frame->head);
    if (symbol == 0) {
        fprintf(
            stderr,
            "trans L2 error: unknown Lingvamyxa name \"%.*s\"; use c.%.*s for explicit C-surface spelling\n",
            (int)frame->head.length,
            frame->head.data,
            (int)frame->head.length,
            frame->head.data
        );
        return 1;
    }
    if (symbol->kind == LM_TRANS_SYMBOL_VARIABLE) {
        out->kind = LM_TRANS_STATEMENT_LOWER_ASSIGNMENT;
        return 0;
    }
    if (
        symbol->kind == LM_TRANS_SYMBOL_FUNCTION ||
        symbol->kind == LM_TRANS_SYMBOL_PROCEDURE
    ) {
        out->kind = LM_TRANS_STATEMENT_LOWER_CALL;
        return 0;
    }

    fprintf(
        stderr,
        "trans L2 error: \"%.*s\" is %s, not a statement head\n",
        (int)frame->head.length,
        frame->head.data,
        lm_trans_symbol_kind_name(symbol->kind)
    );
    return 1;
}

static int lm_trans_emit_loop_jump_statement(
    FILE *file,
    LmP0Text spelling,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    if (lm_trans_emit_cleanups_until(file, indent, namespace_, lm_trans_loop_cleanup_base(namespace_)) != 0) {
        return 1;
    }
    if (lm_trans_emit_indent(file, indent) != 0) {
        return 1;
    }
    if (lm_trans_write_text(file, spelling) != 0) {
        return 1;
    }
    return lm_trans_put(file, ";\n");
}

static int lm_trans_statement_stack_emit_frame_lowering(
    FILE *file,
    LmTransStatementStack *stack,
    const LmP0Frame *frame,
    unsigned indent,
    LmTransNamespace *namespace_,
    LmTransStatementLowering lowering
) {
    if (lowering.kind == LM_TRANS_STATEMENT_LOWER_RETURN) {
        return lm_trans_emit_return_statement(file, frame->body.first_field, indent, namespace_);
    }
    if (lowering.kind == LM_TRANS_STATEMENT_LOWER_IF) {
        return lm_trans_statement_stack_schedule_control(file, stack, frame, indent, namespace_, "if");
    }
    if (lowering.kind == LM_TRANS_STATEMENT_LOWER_WHILE) {
        return lm_trans_statement_stack_schedule_control(file, stack, frame, indent, namespace_, "while");
    }
    if (lowering.kind == LM_TRANS_STATEMENT_LOWER_ELSE) {
        return lm_trans_statement_stack_schedule_else(file, stack, frame, indent, namespace_);
    }
    if (lowering.kind == LM_TRANS_STATEMENT_LOWER_SYNCHRONIZED) {
        return lm_trans_statement_stack_schedule_synchronized(file, stack, frame, indent, namespace_);
    }
    if (
        lowering.kind == LM_TRANS_STATEMENT_LOWER_BREAK ||
        lowering.kind == LM_TRANS_STATEMENT_LOWER_CONTINUE
    ) {
        return lm_trans_emit_loop_jump_statement(file, frame->head, indent, namespace_);
    }
    if (lowering.kind == LM_TRANS_STATEMENT_LOWER_CONST_DECLARATION) {
        return lm_trans_emit_const_statement(file, frame, indent, namespace_);
    }
    if (lowering.kind == LM_TRANS_STATEMENT_LOWER_ARRAY_DECLARATION) {
        return lm_trans_emit_array_declaration(file, frame, indent, namespace_);
    }
    if (lowering.kind == LM_TRANS_STATEMENT_LOWER_STORAGE_DECLARATION) {
        return lm_trans_emit_declaration(file, frame, indent, namespace_);
    }
    if (lowering.kind == LM_TRANS_STATEMENT_LOWER_POINTER_DECLARATION) {
        return lm_trans_emit_pointer_declaration(file, frame, indent, namespace_);
    }
    if (lowering.kind == LM_TRANS_STATEMENT_LOWER_TARGET_ASSIGNMENT) {
        return lm_trans_emit_target_assignment(file, frame, indent, namespace_);
    }
    if (lowering.kind == LM_TRANS_STATEMENT_LOWER_LABEL) {
        return lm_trans_statement_stack_schedule_label(file, stack, frame, indent, namespace_);
    }
    if (lowering.kind == LM_TRANS_STATEMENT_LOWER_CALL) {
        return lm_trans_emit_call_statement(file, frame, indent, namespace_);
    }
    if (lowering.kind == LM_TRANS_STATEMENT_LOWER_ASSIGNMENT) {
        return lm_trans_emit_assignment(file, frame, indent, namespace_);
    }

    return 1;
}

static int lm_trans_statement_stack_emit_frame(
    FILE *file,
    LmTransStatementStack *stack,
    const LmP0Frame *frame,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    int status;
    LmTransStatementLowering lowering;

    status =
        lm_trans_statement_stack_push_frame_job(stack, LM_TRANS_STATEMENT_JOB_TRAILER, frame, indent) ||
        lm_trans_statement_stack_push_frame_job(stack, LM_TRANS_STATEMENT_JOB_VALIDATE_END, frame, indent);
    if (status != 0) {
        return 1;
    }

    if (lm_trans_lower_statement_frame(frame, namespace_, &lowering) != 0) {
        return 1;
    }
    return lm_trans_statement_stack_emit_frame_lowering(file, stack, frame, indent, namespace_, lowering);
}

static LmTransAtomStatementLoweringKind lm_trans_lower_atom_statement(LmP0Text atom) {
    const char *payload;

    payload = lm_trans_registry_lookup_compare(atom, "receiver.atomStatement", lm_trans_legacy_atom_statement_payload(atom));
    if (payload != 0 && strcmp(payload, "break") == 0) {
        return LM_TRANS_ATOM_STATEMENT_LOWER_BREAK;
    }
    if (payload != 0 && strcmp(payload, "continue") == 0) {
        return LM_TRANS_ATOM_STATEMENT_LOWER_CONTINUE;
    }
    if (payload != 0 && strcmp(payload, "return") == 0) {
        return LM_TRANS_ATOM_STATEMENT_LOWER_RETURN;
    }
    if (lm_trans_atom_starts_string(atom)) {
        return LM_TRANS_ATOM_STATEMENT_LOWER_STRING_ERROR;
    }
    return LM_TRANS_ATOM_STATEMENT_LOWER_EXPR;
}

static int lm_trans_emit_atom_statement_lowering(
    FILE *file,
    const LmP0Node *node,
    unsigned indent,
    LmTransNamespace *namespace_,
    LmTransAtomStatementLoweringKind lowering
) {
    if (
        lowering == LM_TRANS_ATOM_STATEMENT_LOWER_BREAK ||
        lowering == LM_TRANS_ATOM_STATEMENT_LOWER_CONTINUE
    ) {
        return lm_trans_emit_loop_jump_statement(file, node->as.atom, indent, namespace_);
    }
    if (lowering == LM_TRANS_ATOM_STATEMENT_LOWER_RETURN) {
        return lm_trans_emit_return_statement(file, 0, indent, namespace_);
    }
    if (lowering == LM_TRANS_ATOM_STATEMENT_LOWER_STRING_ERROR) {
        fprintf(stderr, "trans L2 error: standalone string field is not consumed by any L2 receiver\n");
        return 1;
    }

    if (lm_trans_emit_indent(file, indent) != 0) {
        return 1;
    }
    if (lm_trans_emit_expr_node(file, node, namespace_) != 0) {
        return 1;
    }
    return lm_trans_put(file, ";\n");
}

static int lm_trans_statement_stack_emit_node(
    FILE *file,
    LmTransStatementStack *stack,
    const LmP0Node *node,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    LmTransAtomStatementLoweringKind lowering;

    if (lm_trans_node_is_ignored(node)) {
        return 0;
    }

    if (node->kind == LM_P0_NODE_ATOM) {
        lowering = lm_trans_lower_atom_statement(node->as.atom);
        return lm_trans_emit_atom_statement_lowering(file, node, indent, namespace_, lowering);
    }

    if (node->kind == LM_P0_NODE_FRAME) {
        return lm_trans_statement_stack_emit_frame(file, stack, &node->as.frame, indent, namespace_);
    }

    return 0;
}

static int lm_trans_statement_stack_emit_sync_leave(
    FILE *file,
    unsigned indent,
    unsigned cleanup_id
) {
    if (lm_trans_emit_indent(file, indent) != 0) {
        return 1;
    }
    if (lm_trans_put(file, "lm_synchronized_leave(") != 0) {
        return 1;
    }
    if (lm_trans_emit_sync_name(file, cleanup_id) != 0) {
        return 1;
    }
    return lm_trans_put(file, ");\n");
}

static int lm_trans_emit_statement_list(
    FILE *file,
    const LmP0Field *first,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    LmTransStatementStack stack;
    LmTransStatementJob job;
    const LmP0Field *field;
    int status;

    lm_own_value_stack_init(&stack.jobs, sizeof(LmTransStatementJob));
    status = lm_trans_statement_stack_push_list(&stack, first, indent, 1);

    while (status == 0 && stack.jobs.count > 0U) {
        if (lm_own_value_stack_pop(&stack.jobs, &job) != 0) {
            status = 1;
            break;
        }
        if (job.kind == LM_TRANS_STATEMENT_JOB_LIST) {
            field = lm_trans_statement_list_first_field(
                job.as.list.field,
                job.as.list.unwrap_single_structure
            );
            if (field != 0) {
                if (field->next != 0) {
                    status = lm_trans_statement_stack_push_list(&stack, field->next, job.as.list.indent, 0);
                }
                if (status == 0) {
                    status = lm_trans_statement_stack_push_node(&stack, field->value, job.as.list.indent);
                }
            }
        } else if (job.kind == LM_TRANS_STATEMENT_JOB_NODE) {
            status = lm_trans_statement_stack_emit_node(
                file,
                &stack,
                job.as.node.node,
                job.as.node.indent,
                namespace_
            );
        } else if (job.kind == LM_TRANS_STATEMENT_JOB_VALIDATE_END) {
            status = lm_trans_validate_end_trailer(job.as.frame.frame);
        } else if (job.kind == LM_TRANS_STATEMENT_JOB_TRAILER) {
            status = lm_trans_emit_trailer_statement(
                file,
                job.as.frame.frame->trailer,
                job.as.frame.indent,
                namespace_
            );
        } else if (job.kind == LM_TRANS_STATEMENT_JOB_LEAVE_SCOPE) {
            lm_trans_namespace_leave_scope(namespace_);
        } else if (job.kind == LM_TRANS_STATEMENT_JOB_LOOP_POP) {
            lm_trans_loop_pop(namespace_);
        } else if (job.kind == LM_TRANS_STATEMENT_JOB_CLEANUP_POP) {
            lm_trans_cleanup_pop(namespace_);
        } else if (job.kind == LM_TRANS_STATEMENT_JOB_INDENT_TEXT) {
            if (lm_trans_emit_indent(file, job.as.text.indent) != 0) {
                status = 1;
            } else {
                status = lm_trans_put(file, job.as.text.text);
            }
        } else if (job.kind == LM_TRANS_STATEMENT_JOB_SYNC_LEAVE) {
            status = lm_trans_statement_stack_emit_sync_leave(
                file,
                job.as.sync_leave.indent,
                job.as.sync_leave.cleanup_id
            );
        }
    }

    lm_trans_statement_stack_destroy(&stack);
    return status;
}

static int lm_trans_emit_params(
    FILE *file,
    const LmP0Node *params,
    LmTransNamespace *namespace_
) {
    const LmP0Field *field;
    int first;

    if (params == 0 || params->kind != LM_P0_NODE_STRUCTURE) {
        fprintf(stderr, "trans L2 error: function parameters must be a Structure\n");
        return 1;
    }

    first = 1;
    field = params->as.structure.first_field;
    while (field != 0) {
        if (!first && lm_trans_put(file, ", ") != 0) {
            return 1;
        }
        if (lm_trans_emit_param(file, field->value, namespace_) != 0) {
            return 1;
        }
        first = 0;
        field = field->next;
    }

    if (first) {
        return lm_trans_put(file, "void");
    }

    return 0;
}

static int lm_trans_single_frame_node(const LmP0Node *node, const LmP0Frame **out_frame) {
    const LmP0Field *field;

    if (node == 0 || out_frame == 0) {
        return 0;
    }

    if (node->kind == LM_P0_NODE_FRAME) {
        *out_frame = &node->as.frame;
        return 1;
    }

    if (node->kind != LM_P0_NODE_STRUCTURE) {
        return 0;
    }

    field = node->as.structure.first_field;
    if (
        field != 0 &&
        field->next == 0 &&
        field->value != 0 &&
        field->value->kind == LM_P0_NODE_FRAME
    ) {
        *out_frame = &field->value->as.frame;
        return 1;
    }

    return 0;
}

static int lm_trans_formal_param_name(const LmP0Node *node, LmP0Text *out_name) {
    const LmP0Frame *frame;
    const LmP0Field *field;
    const LmP0Field *name_field;

    if (out_name == 0 || !lm_trans_single_frame_node(node, &frame)) {
        return 0;
    }

    if (lm_trans_text_equals(frame->head, "const")) {
        field = frame->body.first_field;
        if (field == 0 || field->next != 0 || field->value == 0) {
            return 0;
        }
        return lm_trans_formal_param_name(field->value, out_name);
    }

    if (lm_trans_text_all_char(frame->head, '@') || lm_trans_text_equals(frame->head, "[]")) {
        name_field = lm_trans_nth_field(&frame->body, 1U);
    } else {
        name_field = lm_trans_nth_field(&frame->body, 0U);
    }

    if (
        name_field == 0 ||
        name_field->value == 0 ||
        name_field->value->kind != LM_P0_NODE_ATOM
    ) {
        return 0;
    }

    *out_name = name_field->value->as.atom;
    return 1;
}

static int lm_trans_namespace_set_signature(
    LmTransNamespace *namespace_,
    LmP0Text name,
    const LmP0Frame *function_frame
) {
    LmTransSymbol *symbol;
    const LmP0Field *params_field;
    const LmP0Field *field;
    LmP0Text param_name;
    LmOwnPtrStack param_names;
    LmP0Text *param_name_ref;
    size_t index;
    size_t i;

    if (namespace_ == 0 || function_frame == 0) {
        return 1;
    }

    symbol = lm_trans_namespace_find_mutable(namespace_, name);
    if (symbol == 0) {
        return 1;
    }

    params_field = lm_trans_nth_field(&function_frame->body, 1U);
    if (
        params_field == 0 ||
        params_field->value == 0 ||
        params_field->value->kind != LM_P0_NODE_STRUCTURE
    ) {
        fprintf(stderr, "trans L2 error: fn/sub expects a parameter Structure\n");
        return 1;
    }

    lm_own_ptr_stack_init(&param_names, lm_trans_text_ref_delete_any);

    index = 0U;
    field = params_field->value->as.structure.first_field;
    while (field != 0) {
        if (!lm_trans_formal_param_name(field->value, &param_name)) {
            fprintf(stderr, "trans L2 error: function parameter must expose a binding name\n");
            lm_own_ptr_stack_destroy(&param_names);
            return 1;
        }
        for (i = 0U; i < index; ++i) {
            param_name_ref = (LmP0Text *)lm_own_ptr_stack_at(&param_names, i);
            if (param_name_ref != 0 && lm_trans_text_same(*param_name_ref, param_name)) {
                fprintf(stderr, "trans L2 error: duplicate function parameter name\n");
                lm_own_ptr_stack_destroy(&param_names);
                return 1;
            }
        }
        param_name_ref = lm_trans_text_ref_new(param_name);
        if (param_name_ref == 0) {
            lm_own_ptr_stack_destroy(&param_names);
            return 1;
        }
        if (lm_own_ptr_stack_push(&param_names, param_name_ref) != 0) {
            lm_trans_text_ref_delete_any(param_name_ref);
            lm_own_ptr_stack_destroy(&param_names);
            return 1;
        }
        ++index;
        field = field->next;
    }

    lm_own_ptr_stack_destroy(&symbol->param_names);
    symbol->param_names = param_names;
    symbol->has_signature = 1;
    return 0;
}

static int lm_trans_function_header_from_frame(
    const LmP0Frame *frame,
    int is_external,
    LmTransFunctionHeader *out
) {
    const LmP0Field *name_field;
    const LmP0Field *params_field;
    const LmP0Field *return_field;
    const LmP0Field *body_field;
    const char *receiver_payload;
    int is_sub;
    int is_struct_return;

    if (frame == 0 || out == 0) {
        return 0;
    }

    receiver_payload = lm_trans_registry_lookup_compare(
        frame->head,
        "receiver.function",
        lm_trans_legacy_function_receiver_payload(frame->head)
    );
    if (receiver_payload == 0) {
        return 0;
    }
    is_sub = strcmp(receiver_payload, "procedure") == 0;
    is_struct_return = strcmp(receiver_payload, "function.struct") == 0;

    name_field = lm_trans_nth_field(&frame->body, 0U);
    params_field = lm_trans_nth_field(&frame->body, 1U);

    if (
        name_field == 0 ||
        params_field == 0 ||
        name_field->value == 0 ||
        name_field->value->kind != LM_P0_NODE_ATOM
    ) {
        fprintf(stderr, "trans L2 error: fn/sub expects name and parameters\n");
        return -1;
    }

    memset(out, 0, sizeof(*out));
    out->frame = frame;
    out->name = name_field->value->as.atom;
    out->params_node = params_field->value;
    out->symbol_kind = is_sub ? LM_TRANS_SYMBOL_PROCEDURE : LM_TRANS_SYMBOL_FUNCTION;
    out->is_sub = is_sub;
    out->is_struct_return = is_struct_return;
    out->is_external = is_external;

    if (is_sub) {
        body_field = lm_trans_nth_field(&frame->body, 2U);
        if (
            body_field != 0 &&
            body_field->next == 0 &&
            body_field->value != 0 &&
            body_field->value->kind == LM_P0_NODE_STRUCTURE
        ) {
            out->body_start = body_field->value->as.structure.first_field;
        } else {
            out->body_start = params_field->next;
        }
        return 1;
    }

    return_field = lm_trans_nth_field(&frame->body, 2U);
    if (return_field == 0 || return_field->value == 0) {
        fprintf(stderr, "trans L2 error: fn expects return type\n");
        return -1;
    }

    out->return_node = return_field->value;
    body_field = lm_trans_nth_field(&frame->body, 3U);
    if (
        body_field != 0 &&
        body_field->next == 0 &&
        body_field->value != 0 &&
        body_field->value->kind == LM_P0_NODE_STRUCTURE
    ) {
        out->body_start = body_field->value->as.structure.first_field;
    } else {
        out->body_start = return_field->next;
    }

    return 1;
}

static int lm_trans_top_level_function_header(
    const LmP0Frame *frame,
    LmTransFunctionHeader *out
) {
    const LmP0Field *field;
    const LmP0Frame *inner;
    int status;

    if (frame == 0 || out == 0) {
        return 0;
    }

    status = lm_trans_function_header_from_frame(frame, 0, out);
    if (status != 0) {
        return status;
    }

    if (
        strcmp(
            lm_trans_payload_text(
                lm_trans_registry_lookup_compare(
                    frame->head,
                    "receiver.topLevel",
                    lm_trans_legacy_top_level_payload(frame->head)
                )
            ),
            "external-function"
        ) != 0
    ) {
        return 0;
    }

    field = frame->body.first_field;
    if (
        field == 0 ||
        field->next != 0 ||
        field->value == 0 ||
        field->value->kind != LM_P0_NODE_FRAME
    ) {
        fprintf(stderr, "trans L2 error: external receiver expects exactly one fn/sub frame\n");
        return -1;
    }

    inner = &field->value->as.frame;
    status = lm_trans_function_header_from_frame(inner, 1, out);
    if (status != 0) {
        return status;
    }

    fprintf(stderr, "trans L2 error: external receiver expects fn/sub frame\n");
    return -1;
}

static int lm_trans_lower_top_level_item(
    const LmP0Node *node,
    LmTransTopLevelItem *out
) {
    int function_status;

    if (out == 0) {
        return 1;
    }

    memset(out, 0, sizeof(*out));

    if (lm_trans_node_is_ignored(node)) {
        out->kind = LM_TRANS_TOP_LEVEL_ITEM_NONE;
        return 0;
    }

    if (node->kind != LM_P0_NODE_FRAME) {
        fprintf(stderr, "trans L2 error: top-level L2 field must be consumed by an L2 receiver\n");
        return 1;
    }

    out->frame = &node->as.frame;

    function_status = lm_trans_top_level_function_header(out->frame, &out->function);
    if (function_status < 0) {
        return 1;
    }
    if (function_status > 0) {
        out->kind = LM_TRANS_TOP_LEVEL_ITEM_FUNCTION;
        return 0;
    }

    if (lm_trans_frame_looks_named_structure_declaration(out->frame)) {
        out->kind = LM_TRANS_TOP_LEVEL_ITEM_NAMED_STRUCTURE;
        return 0;
    }

    if (
        strcmp(
            lm_trans_payload_text(
                lm_trans_registry_lookup_compare(
                    out->frame->head,
                    "receiver.topLevel",
                    lm_trans_legacy_top_level_payload(out->frame->head)
                )
            ),
            "lower.l1"
        ) == 0
    ) {
        out->kind = LM_TRANS_TOP_LEVEL_ITEM_L1;
        return 0;
    }

    fprintf(stderr, "trans L2 error: top-level L2 field must be fn, sub, external fn/sub, L1 frame, or named Structure\n");
    return 1;
}

static int lm_trans_emit_function(
    FILE *file,
    const LmTransFunctionHeader *function,
    LmTransNamespace *namespace_
) {
    const LmP0Frame *frame;
    LmTransFunctionState *state;
    int status;

    if (function == 0 || function->frame == 0) {
        return 1;
    }

    frame = function->frame;
    state = 0;

    if (function->is_sub) {
        if (!function->is_external) {
            if (lm_trans_put(file, "static ") != 0) {
                return 1;
            }
        }
        if (lm_trans_put(file, "void ") != 0) {
            return 1;
        }
    } else {
        if (function->is_struct_return) {
            if (lm_trans_emit_function_return_structure(file, function->name, function->return_node) != 0) {
                return 1;
            }
        } else {
            if (function->return_node->kind == LM_P0_NODE_STRUCTURE) {
                fprintf(stderr, "trans L2 error: fn expects a single-value return type; use fm for Structure return\n");
                return 1;
            }
        }
        if (!function->is_external) {
            if (lm_trans_put(file, "static ") != 0) {
                return 1;
            }
        }
        if (function->is_struct_return) {
            if (lm_trans_emit_function_return_struct_type_name(file, function->name) != 0) {
                return 1;
            }
        } else {
            if (lm_trans_emit_type_node(file, function->return_node) != 0) {
                return 1;
            }
        }
        if (lm_trans_put(file, " ") != 0) {
            return 1;
        }
    }

    if (lm_trans_write_text(file, function->name) != 0) {
        return 1;
    }
    if (lm_trans_put(file, "(") != 0) {
        return 1;
    }
    lm_trans_namespace_enter_scope(namespace_);
    if (lm_trans_emit_params(file, function->params_node, namespace_) != 0) {
        lm_trans_namespace_leave_scope(namespace_);
        return 1;
    }
    if (lm_trans_put(file, ") {\n") != 0) {
        lm_trans_namespace_leave_scope(namespace_);
        return 1;
    }

    state = lm_trans_function_state_new();
    if (state == 0) {
        lm_trans_namespace_leave_scope(namespace_);
        return 1;
    }

    if (function->is_struct_return) {
        state->current_return_type_name = lm_trans_text_ref_new(function->name);
        if (state->current_return_type_name == 0) {
            lm_trans_function_state_delete(state);
            lm_trans_namespace_leave_scope(namespace_);
            return 1;
        }
    }

    state->previous_return_type_node = namespace_->return_type_node;
    state->previous_return_type_is_struct = namespace_->return_type_is_struct;
    state->previous_return_type_name = namespace_->return_type_name;
    state->previous_next_return_id = namespace_->next_return_id;
    state->previous_cleanups = namespace_->cleanups;
    state->previous_loops = namespace_->loops;
    state->has_previous_control_stacks = 1;
    namespace_->return_type_node = function->is_sub ? 0 : function->return_node;
    namespace_->return_type_is_struct = function->is_struct_return;
    namespace_->return_type_name = function->is_struct_return ? state->current_return_type_name : state->previous_return_type_name;
    namespace_->next_return_id = 0U;
    lm_own_ptr_stack_init(&namespace_->cleanups, lm_trans_cleanup_delete_any);
    lm_own_ptr_stack_init(&namespace_->loops, lm_trans_loop_delete_any);

    status = lm_trans_emit_statement_list(file, function->body_start, 1U, namespace_);
    if (status == 0) {
        status = lm_trans_validate_end_trailer(frame);
    }
    if (status == 0) {
        status = lm_trans_emit_trailer_statement(file, frame->trailer, 1U, namespace_);
    }
    if (status == 0) {
        status = lm_trans_put(file, "}\n");
    }

    namespace_->return_type_node = state->previous_return_type_node;
    namespace_->return_type_is_struct = state->previous_return_type_is_struct;
    namespace_->return_type_name = state->previous_return_type_name;
    namespace_->next_return_id = state->previous_next_return_id;
    lm_own_ptr_stack_destroy(&namespace_->cleanups);
    lm_own_ptr_stack_destroy(&namespace_->loops);
    namespace_->cleanups = state->previous_cleanups;
    namespace_->loops = state->previous_loops;
    lm_own_ptr_stack_init(&state->previous_cleanups, 0);
    lm_own_ptr_stack_init(&state->previous_loops, 0);
    state->has_previous_control_stacks = 0;
    lm_trans_function_state_delete(state);
    lm_trans_namespace_leave_scope(namespace_);
    return status;
}

static int lm_trans_is_end_target(const LmP0Frame *frame, const char *target);
static int lm_trans_emit_l1_frame(FILE *output, const LmP0Frame *l1);
static int lm_trans_emit_l2_frame(FILE *file, const LmP0Frame *l2);

static int lm_trans_emit_l2_frame(FILE *file, const LmP0Frame *l2) {
    const LmP0Field *field;
    const LmP0Node *node;
    LmTransNamespace *namespace_;
    LmTransTopLevelItem item;
    int status;
    int emitted_top_level;

    namespace_ = lm_trans_namespace_new();
    if (namespace_ == 0) {
        return 1;
    }

    emitted_top_level = 0;
    field = l2->body.first_field;
    while (field != 0) {
        node = field->value;
        if (lm_trans_lower_top_level_item(node, &item) != 0) {
            lm_trans_namespace_delete(namespace_);
            return 1;
        }

        if (item.kind == LM_TRANS_TOP_LEVEL_ITEM_FUNCTION) {
            if (lm_trans_namespace_declare(namespace_, item.function.name, item.function.symbol_kind) != 0) {
                lm_trans_namespace_delete(namespace_);
                return 1;
            }
            if (lm_trans_namespace_set_signature(namespace_, item.function.name, item.function.frame) != 0) {
                lm_trans_namespace_delete(namespace_);
                return 1;
            }
        } else if (item.kind == LM_TRANS_TOP_LEVEL_ITEM_NAMED_STRUCTURE) {
            if (lm_trans_namespace_declare(namespace_, item.frame->head, LM_TRANS_SYMBOL_STRUCTURE) != 0) {
                lm_trans_namespace_delete(namespace_);
                return 1;
            }
        }

        field = field->next;
    }

    field = l2->body.first_field;
    while (field != 0) {
        node = field->value;
        if (lm_trans_lower_top_level_item(node, &item) != 0) {
            lm_trans_namespace_delete(namespace_);
            return 1;
        }

        if (item.kind == LM_TRANS_TOP_LEVEL_ITEM_L1) {
            if (emitted_top_level && lm_trans_put(file, "\n") != 0) {
                lm_trans_namespace_delete(namespace_);
                return 1;
            }
            status = lm_trans_emit_l1_frame(file, item.frame);
        } else if (item.kind == LM_TRANS_TOP_LEVEL_ITEM_NAMED_STRUCTURE) {
            if (emitted_top_level && lm_trans_put(file, "\n") != 0) {
                lm_trans_namespace_delete(namespace_);
                return 1;
            }
            status = lm_trans_emit_named_structure(file, item.frame);
        } else {
            status = 0;
        }

        if (status != 0) {
            lm_trans_namespace_delete(namespace_);
            return 1;
        }
        if (
            item.kind == LM_TRANS_TOP_LEVEL_ITEM_L1 ||
            item.kind == LM_TRANS_TOP_LEVEL_ITEM_NAMED_STRUCTURE
        ) {
            emitted_top_level = 1;
        }
        field = field->next;
    }

    field = l2->body.first_field;
    while (field != 0) {
        node = field->value;
        if (lm_trans_lower_top_level_item(node, &item) != 0) {
            lm_trans_namespace_delete(namespace_);
            return 1;
        }

        if (item.kind == LM_TRANS_TOP_LEVEL_ITEM_FUNCTION) {
            if (emitted_top_level && lm_trans_put(file, "\n") != 0) {
                lm_trans_namespace_delete(namespace_);
                return 1;
            }
            status = lm_trans_emit_function(file, &item.function, namespace_);
            if (status != 0) {
                lm_trans_namespace_delete(namespace_);
                return 1;
            }
            emitted_top_level = 1;
            if (
                item.function.is_external &&
                lm_trans_validate_end_trailer(item.frame) != 0
            ) {
                lm_trans_namespace_delete(namespace_);
                return 1;
            }
        }
        field = field->next;
    }

    status = lm_trans_validate_end_trailer(l2);
    lm_trans_namespace_delete(namespace_);
    return status;
}

static int lm_trans_inline_string_payload(LmP0Text text, LmP0Text *out_payload, size_t *out_run) {
    char quote;
    size_t open_run;
    size_t close_run;

    if (out_payload == 0 || out_run == 0 || text.length < 2U) {
        return 0;
    }

    quote = text.data[0];
    if (quote != '\'' && quote != '"') {
        return 0;
    }

    open_run = 0U;
    while (open_run < text.length && text.data[open_run] == quote) {
        ++open_run;
    }
    if (text.length < open_run * 2U) {
        return 0;
    }

    close_run = 0U;
    while (close_run < text.length && text.data[text.length - close_run - 1U] == quote) {
        ++close_run;
    }
    if (close_run < open_run) {
        return 0;
    }

    out_payload->data = text.data + open_run;
    out_payload->length = text.length - open_run * 2U;
    *out_run = open_run;
    return 1;
}

static int lm_trans_emit_l1_payload(FILE *output, LmP0Text text) {
    LmP0Text payload;
    size_t delimiter_run;
    size_t i;
    char ch;

    if (lm_trans_inline_string_payload(text, &payload, &delimiter_run)) {
        if (delimiter_run == 1U) {
            i = 0U;
            while (i < payload.length) {
                ch = payload.data[i++];
                if (ch == '\\' && i < payload.length) {
                    ch = payload.data[i++];
                    if (ch == 'n') {
                        ch = '\n';
                    } else if (ch == 'r') {
                        ch = '\r';
                    } else if (ch == 't') {
                        ch = '\t';
                    } else if (ch == '0') {
                        ch = '\0';
                    }
                }
                if (lm_trans_write_all(output, &ch, 1U) != 0) {
                    return 1;
                }
            }
            return 0;
        }

        return lm_trans_write_all(output, payload.data, payload.length);
    }

    return lm_trans_write_text(output, text);
}

static int lm_trans_emit_l1_node(FILE *output, const LmP0Node *node);

static int lm_trans_emit_l1_structure(FILE *output, const LmP0Structure *structure) {
    const LmP0Field *field;

    if (structure == 0) {
        return 0;
    }

    field = structure->first_field;
    while (field != 0) {
        if (lm_trans_emit_l1_node(output, field->value) != 0) {
            return 1;
        }
        field = field->next;
    }

    return 0;
}

static int lm_trans_emit_l1_node(FILE *output, const LmP0Node *node) {
    if (lm_trans_node_is_ignored(node)) {
        return 0;
    }

    if (node->kind == LM_P0_NODE_ATOM) {
        if (lm_trans_emit_l1_payload(output, node->as.atom) != 0) {
            return 1;
        }
        return lm_trans_write_all(output, "\n", 1U);
    }

    if (node->kind == LM_P0_NODE_STRUCTURE) {
        return lm_trans_emit_l1_structure(output, &node->as.structure);
    }

    if (node->kind == LM_P0_NODE_FRAME) {
        if (lm_trans_text_equals(node->as.frame.head, "L1")) {
            return lm_trans_emit_l1_frame(output, &node->as.frame);
        }
        if (lm_trans_text_equals(node->as.frame.head, "L2")) {
            return lm_trans_emit_l2_frame(output, &node->as.frame);
        }
        if (lm_trans_is_end_target(&node->as.frame, "L1")) {
            return 0;
        }

        fprintf(stderr, "trans error: unknown translator receiver inside L1\n");
        return 1;
    }

    return 0;
}

static int lm_trans_emit_l1_frame(FILE *output, const LmP0Frame *l1) {
    if (l1 == 0) {
        return 1;
    }

    if (lm_trans_emit_l1_structure(output, &l1->body) != 0) {
        return 1;
    }

    return lm_trans_validate_end_trailer(l1);
}

static int lm_trans_is_end_target(const LmP0Frame *frame, const char *target) {
    const LmP0Field *field;

    if (frame == 0 || !lm_trans_text_equals(frame->head, "end")) {
        return 0;
    }

    field = frame->body.first_field;
    return
        field != 0 &&
        field->next == 0 &&
        field->value != 0 &&
        field->value->kind == LM_P0_NODE_ATOM &&
        lm_trans_text_equals(field->value->as.atom, target);
}

static int lm_trans_emit_l1_body(FILE *output, const LmP0Frame *l1, int *emitted) {
    if (lm_trans_emit_l1_frame(output, l1) != 0) {
        return 1;
    }

    if (emitted != 0) {
        *emitted = 1;
    }
    return 0;
}

static int lm_trans_emit_root_sequence(FILE *output, const LmP0Node *root, int *emitted) {
    const LmP0Field *field;
    const LmP0Node *node;

    if (root == 0 || root->kind != LM_P0_NODE_STRUCTURE) {
        return 1;
    }

    field = root->as.structure.first_field;
    while (field != 0) {
        node = field->value;
        if (!lm_trans_node_is_ignored(node)) {
            if (node->kind == LM_P0_NODE_ATOM) {
                fprintf(stderr, "trans error: root raw text must be inside L1\n");
                return 1;
            } else if (node->kind == LM_P0_NODE_FRAME && lm_trans_text_equals(node->as.frame.head, "L1")) {
                if (lm_trans_emit_l1_body(output, &node->as.frame, emitted) != 0) {
                    return 1;
                }
                *emitted = 1;
            } else if (node->kind == LM_P0_NODE_FRAME && lm_trans_text_equals(node->as.frame.head, "L2")) {
                if (lm_trans_emit_l2_frame(output, &node->as.frame) != 0) {
                    return 1;
                }
                *emitted = 1;
            } else if (node->kind == LM_P0_NODE_FRAME && lm_trans_is_end_target(&node->as.frame, "L1")) {
                *emitted = 1;
            } else {
                fprintf(stderr, "trans error: root field must be L1, L2, raw L1 text, or end: L1\n");
                return 1;
            }
        }
        field = field->next;
    }

    return 0;
}

static void lm_trans_registry_destroy(void) {
    if (lm_trans_registry.loaded) {
        lm_own_ptr_stack_destroy(&lm_trans_registry.rows);
        lm_trans_registry.loaded = 0;
    }
}

static int lm_trans_registry_row_from_frame(const LmP0Frame *frame) {
    const LmP0Field *table_field;
    const LmP0Field *key_field;
    const LmP0Field *payload_field;

    if (frame == 0 || !lm_trans_text_equals(frame->head, "row")) {
        return 0;
    }

    table_field = lm_trans_nth_field(&frame->body, 0U);
    key_field = lm_trans_nth_field(&frame->body, 1U);
    payload_field = lm_trans_nth_field(&frame->body, 2U);
    if (
        table_field == 0 ||
        key_field == 0 ||
        payload_field == 0 ||
        payload_field->next != 0 ||
        table_field->value == 0 ||
        key_field->value == 0 ||
        payload_field->value == 0 ||
        table_field->value->kind != LM_P0_NODE_ATOM ||
        key_field->value->kind != LM_P0_NODE_ATOM ||
        payload_field->value->kind != LM_P0_NODE_ATOM
    ) {
        fprintf(stderr, "trans registry error: row expects exactly three atom fields\n");
        return -1;
    }

    if (
        lm_trans_registry_push_row_atoms(
            table_field->value->as.atom,
            key_field->value->as.atom,
            payload_field->value->as.atom
        ) != 0
    ) {
        return -1;
    }

    return 1;
}

static int lm_trans_registry_frame_single_atom(
    const LmP0Frame *frame,
    const char *head,
    LmP0Text *out_atom
) {
    const LmP0Field *field;

    if (frame == 0 || head == 0 || out_atom == 0) {
        return 0;
    }
    if (!lm_trans_text_equals(frame->head, head)) {
        return 0;
    }

    field = lm_trans_nth_field(&frame->body, 0U);
    if (
        field == 0 ||
        field->next != 0 ||
        field->value == 0 ||
        field->value->kind != LM_P0_NODE_ATOM
    ) {
        return -1;
    }

    *out_atom = field->value->as.atom;
    return 1;
}

static int lm_trans_registry_column_name(
    const LmP0Field *field,
    LmP0Text *out_name
) {
    const LmP0Node *node;

    if (field == 0 || out_name == 0) {
        return 0;
    }

    node = field->value;
    if (node == 0 || node->kind != LM_P0_NODE_FRAME) {
        return -1;
    }

    if (lm_trans_registry_frame_single_atom(&node->as.frame, "char@", out_name) > 0) {
        return 1;
    }
    if (lm_trans_registry_frame_single_atom(&node->as.frame, "int@", out_name) > 0) {
        return 1;
    }
    if (lm_trans_registry_frame_single_atom(&node->as.frame, "binding@", out_name) > 0) {
        return 1;
    }
    if (lm_trans_registry_frame_single_atom(&node->as.frame, "class@", out_name) > 0) {
        return 1;
    }

    return -1;
}

static int lm_trans_registry_columns_from_frame(
    const LmP0Frame *frame,
    size_t *out_count
) {
    const LmP0Field *field;
    LmP0Text first_name;
    LmP0Text column_name;
    size_t count;
    int status;

    if (frame == 0 || out_count == 0 || !lm_trans_text_equals(frame->head, "columns")) {
        return 0;
    }

    count = 0U;
    field = frame->body.first_field;
    while (field != 0) {
        if (field->value != 0 && !lm_trans_node_is_ignored(field->value)) {
            status = lm_trans_registry_column_name(field, &column_name);
            if (status <= 0) {
                fprintf(stderr, "trans registry error: columns expects typed column descriptors\n");
                return -1;
            }
            if (count == 0U) {
                first_name = column_name;
            }
            ++count;
        }
        field = field->next;
    }

    if (count != 2U) {
        fprintf(stderr, "trans registry error: bootstrap table expects exactly two columns\n");
        return -1;
    }
    if (!lm_trans_text_equals(first_name, "class")) {
        fprintf(stderr, "trans registry error: first table column must be class\n");
        return -1;
    }

    *out_count = count;
    return 1;
}

static int lm_trans_registry_rows_from_frame(
    const LmP0Frame *frame,
    LmP0Text table_name,
    size_t column_count
) {
    const LmP0Field *field;
    const LmP0Node *key_node;
    const LmP0Node *payload_node;
    size_t field_index;

    if (frame == 0 || !lm_trans_text_equals(frame->head, "rows")) {
        return 0;
    }
    if (column_count != 2U) {
        fprintf(stderr, "trans registry error: bootstrap rows require two columns\n");
        return -1;
    }

    field_index = 0U;
    field = frame->body.first_field;
    while (field != 0) {
        if (field->value != 0 && !lm_trans_node_is_ignored(field->value)) {
            if ((field_index % column_count) == 0U) {
                key_node = field->value;
                if (field->next == 0 || field->next->value == 0) {
                    fprintf(stderr, "trans registry error: rows field count is not divisible by column count\n");
                    return -1;
                }
                payload_node = field->next->value;
                if (
                    key_node->kind != LM_P0_NODE_ATOM ||
                    payload_node->kind != LM_P0_NODE_ATOM
                ) {
                    fprintf(stderr, "trans registry error: table rows currently expect atom cells\n");
                    return -1;
                }
                if (
                    lm_trans_registry_push_row_atoms(
                        table_name,
                        key_node->as.atom,
                        payload_node->as.atom
                    ) != 0
                ) {
                    return -1;
                }
            }
            ++field_index;
        }
        field = field->next;
    }

    if ((field_index % column_count) != 0U) {
        fprintf(stderr, "trans registry error: rows field count is not divisible by column count\n");
        return -1;
    }
    if (field_index == 0U) {
        fprintf(stderr, "trans registry error: table rows must not be empty\n");
        return -1;
    }

    return 1;
}

static int lm_trans_registry_validate_table_trailer(
    const LmP0Frame *frame,
    LmP0Text table_name
) {
    LmP0Text actual;
    LmP0Text actual_payload;
    LmP0Text expected_payload;

    if (frame == 0 || frame->trailer == 0) {
        return 0;
    }
    if (!lm_trans_text_equals(frame->trailer->spelling, "end")) {
        return 0;
    }
    if (!lm_trans_trailer_single_atom(frame->trailer, &actual)) {
        fprintf(stderr, "trans registry error: table end expects exactly one target name\n");
        return 1;
    }
    if (
        !lm_trans_registry_atom_payload(actual, &actual_payload) ||
        !lm_trans_registry_atom_payload(table_name, &expected_payload) ||
        !lm_trans_text_same(actual_payload, expected_payload)
    ) {
        fprintf(stderr, "trans registry error: table end target does not match table name\n");
        return 1;
    }

    return 0;
}

static int lm_trans_registry_table_from_frame(const LmP0Frame *frame) {
    const LmP0Field *field;
    const LmP0Node *node;
    LmP0Text table_name;
    size_t column_count;
    int have_name;
    int have_columns;
    int have_rows;
    int status;

    if (frame == 0 || !lm_trans_text_equals(frame->head, "table")) {
        return 0;
    }

    have_name = 0;
    have_columns = 0;
    have_rows = 0;
    column_count = 0U;
    field = frame->body.first_field;
    while (field != 0) {
        node = field->value;
        if (node != 0 && !lm_trans_node_is_ignored(node)) {
            if (node->kind != LM_P0_NODE_FRAME) {
                fprintf(stderr, "trans registry error: table body expects name/columns/rows frames\n");
                return -1;
            }
            status = lm_trans_registry_frame_single_atom(&node->as.frame, "name", &table_name);
            if (status < 0) {
                fprintf(stderr, "trans registry error: table name expects exactly one atom\n");
                return -1;
            }
            if (status > 0) {
                have_name = 1;
            } else if (lm_trans_text_equals(node->as.frame.head, "columns")) {
                status = lm_trans_registry_columns_from_frame(&node->as.frame, &column_count);
                if (status <= 0) {
                    return -1;
                }
                have_columns = 1;
            } else if (lm_trans_text_equals(node->as.frame.head, "rows")) {
                if (!have_name || !have_columns) {
                    fprintf(stderr, "trans registry error: table rows must appear after name and columns\n");
                    return -1;
                }
                status = lm_trans_registry_rows_from_frame(&node->as.frame, table_name, column_count);
                if (status <= 0) {
                    return -1;
                }
                have_rows = 1;
            } else {
                fprintf(stderr, "trans registry error: table body expects name/columns/rows frames\n");
                return -1;
            }
        }
        field = field->next;
    }

    if (!have_name || !have_columns || !have_rows) {
        fprintf(stderr, "trans registry error: table requires name, columns and rows\n");
        return -1;
    }
    if (lm_trans_registry_validate_table_trailer(frame, table_name) != 0) {
        return -1;
    }

    return 1;
}

static int lm_trans_registry_load_rows(const LmP0Structure *structure) {
    const LmP0Field *field;
    const LmP0Node *node;
    int status;

    if (structure == 0) {
        return 0;
    }

    field = structure->first_field;
    while (field != 0) {
        node = field->value;
        if (!lm_trans_node_is_ignored(node)) {
            if (node->kind != LM_P0_NODE_FRAME) {
                fprintf(stderr, "trans registry error: registry body expects table or row frames\n");
                return 1;
            }
            status = lm_trans_registry_row_from_frame(&node->as.frame);
            if (status <= 0) {
                if (status < 0) {
                    return 1;
                }
                status = lm_trans_registry_table_from_frame(&node->as.frame);
                if (status <= 0) {
                    if (status == 0) {
                        fprintf(stderr, "trans registry error: registry body expects table or row frames\n");
                    }
                    return 1;
                }
            }
        }
        field = field->next;
    }

    return 0;
}

static int lm_trans_registry_load_root(const LmP0Node *root) {
    const LmP0Field *field;
    const LmP0Node *node;
    int loaded;

    if (root == 0 || root->kind != LM_P0_NODE_STRUCTURE) {
        fprintf(stderr, "trans registry error: root must be a Structure\n");
        return 1;
    }

    loaded = 0;
    field = root->as.structure.first_field;
    while (field != 0) {
        node = field->value;
        if (!lm_trans_node_is_ignored(node)) {
            if (node->kind != LM_P0_NODE_FRAME) {
                fprintf(stderr, "trans registry error: root fields must be registry, table or row frames\n");
                return 1;
            }
            if (lm_trans_text_equals(node->as.frame.head, "registry")) {
                if (lm_trans_registry_load_rows(&node->as.frame.body) != 0) {
                    return 1;
                }
                if (lm_trans_validate_end_trailer(&node->as.frame) != 0) {
                    return 1;
                }
                loaded = 1;
            } else if (lm_trans_text_equals(node->as.frame.head, "row")) {
                if (lm_trans_registry_row_from_frame(&node->as.frame) <= 0) {
                    return 1;
                }
                loaded = 1;
            } else if (lm_trans_text_equals(node->as.frame.head, "table")) {
                if (lm_trans_registry_table_from_frame(&node->as.frame) <= 0) {
                    return 1;
                }
                loaded = 1;
            } else {
                fprintf(stderr, "trans registry error: root fields must be registry, table or row frames\n");
                return 1;
            }
        }
        field = field->next;
    }

    if (!loaded || lm_trans_registry.rows.count == 0U) {
        fprintf(stderr, "trans registry error: no rows loaded\n");
        return 1;
    }

    return 0;
}

static int lm_trans_registry_path_for_source(
    const char *source_path,
    char *buffer,
    size_t size
) {
    const char *override_path;
    size_t length;
    size_t i;
    size_t slash;
    const char *file_name;

    if (buffer == 0 || size == 0U) {
        return 1;
    }

    override_path = getenv("LM_TRANS_REGISTRY");
    if (override_path != 0 && override_path[0] != '\0') {
        if (strlen(override_path) >= size) {
            fprintf(stderr, "trans error: LM_TRANS_REGISTRY path is too long\n");
            return 1;
        }
        strcpy(buffer, override_path);
        return 0;
    }

    file_name = "trans_registry.lmx";
    if (source_path == 0 || source_path[0] == '\0') {
        if (strlen(file_name) >= size) {
            return 1;
        }
        strcpy(buffer, file_name);
        return 0;
    }

    length = strlen(source_path);
    slash = 0U;
    for (i = 0U; i < length; ++i) {
        if (source_path[i] == '/' || source_path[i] == '\\') {
            slash = i + 1U;
        }
    }

    if (slash == 0U) {
        if (strlen(file_name) >= size) {
            return 1;
        }
        strcpy(buffer, file_name);
        return 0;
    }

    if (slash + strlen(file_name) >= size) {
        fprintf(stderr, "trans error: registry path is too long\n");
        return 1;
    }
    memcpy(buffer, source_path, slash);
    strcpy(buffer + slash, file_name);
    return 0;
}

static int lm_trans_registry_load_for_source(const char *source_path) {
    char registry_path[4096];
    LmP0Document *document;
    const LmP0Diagnostic *diagnostic;
    const char *override_path;
    int override_enabled;
    int status;

    lm_trans_registry_destroy();
    lm_own_ptr_stack_init(&lm_trans_registry.rows, lm_trans_registry_row_destroy_any);
    lm_trans_registry.loaded = 1;

    if (lm_trans_registry_path_for_source(source_path, registry_path, sizeof(registry_path)) != 0) {
        lm_trans_registry_destroy();
        return 1;
    }

    document = 0;
    override_path = getenv("LM_TRANS_REGISTRY");
    override_enabled = override_path != 0 && override_path[0] != '\0';
    status = lm_p0_parse_file(registry_path, &document);
    diagnostic = status != 0 ? lm_p0_document_diagnostic(document) : 0;
    if (
        status != 0 &&
        diagnostic == 0 &&
        !override_enabled &&
        strcmp(registry_path, "lm2/trans_registry.lmx") != 0
    ) {
        lm_p0_document_destroy(document);
        document = 0;
        strcpy(registry_path, "lm2/trans_registry.lmx");
        status = lm_p0_parse_file(registry_path, &document);
    }

    if (status != 0) {
        diagnostic = lm_p0_document_diagnostic(document);
        if (diagnostic != 0) {
            fprintf(
                stderr,
                "trans registry parse error %d at %lu:%lu in %s: %s\n",
                diagnostic->code,
                (unsigned long)diagnostic->line,
                (unsigned long)diagnostic->column,
                registry_path,
                diagnostic->message
            );
        } else {
            fprintf(stderr, "trans registry error: cannot read %s\n", registry_path);
        }
        lm_p0_document_destroy(document);
        lm_trans_registry_destroy();
        return 1;
    }

    status = lm_trans_registry_load_root(lm_p0_document_root(document));
    lm_p0_document_destroy(document);
    if (status != 0) {
        lm_trans_registry_destroy();
        return 1;
    }

    return 0;
}

static int lm_trans_emit_document(const char *source_path, const char *output_path) {
    LmP0Document *document;
    const LmP0Diagnostic *diagnostic;
    const LmP0Node *root;
    FILE *output;
    int status;
    int close_status;
    int emitted;

    document = 0;
    if (lm_trans_registry_load_for_source(source_path) != 0) {
        return 1;
    }

    status = lm_p0_parse_file(source_path, &document);
    if (status != 0) {
        diagnostic = lm_p0_document_diagnostic(document);
        if (diagnostic != 0) {
            fprintf(
                stderr,
                "trans parse error %d at %lu:%lu: %s\n",
                diagnostic->code,
                (unsigned long)diagnostic->line,
                (unsigned long)diagnostic->column,
                diagnostic->message
            );
        } else {
            fprintf(stderr, "trans parse error while reading %s\n", source_path);
        }
        lm_p0_document_destroy(document);
        lm_trans_registry_destroy();
        return 1;
    }

    output = fopen(output_path, "wb");
    if (output == 0) {
        fprintf(stderr, "trans error: cannot open output file %s\n", output_path);
        lm_p0_document_destroy(document);
        lm_trans_registry_destroy();
        return 1;
    }

    root = lm_p0_document_root(document);
    emitted = 0;
    status = lm_trans_emit_root_sequence(output, root, &emitted);
    if (status == 0 && !emitted) {
        fprintf(stderr, "trans error: root L1 or L2 frame was not found\n");
        status = 1;
    }

    close_status = fclose(output);
    if (status != 0) {
        lm_p0_document_destroy(document);
        lm_trans_registry_destroy();
        return 1;
    }

    if (close_status != 0) {
        fprintf(stderr, "trans error: cannot write output file %s\n", output_path);
        lm_p0_document_destroy(document);
        lm_trans_registry_destroy();
        return 1;
    }

    lm_p0_document_destroy(document);
    lm_trans_registry_destroy();
    return 0;
}

int main(int argc, char **argv) {
    setvbuf(stdout, 0, _IONBF, 0);
    setvbuf(stderr, 0, _IONBF, 0);

    if (argc != 3) {
        fprintf(stderr, "usage: trans.lm0[.exe] <source.lm2> <output.lm1.c>\n");
        return 1;
    }

    return lm_trans_emit_document(argv[1], argv[2]);
}

