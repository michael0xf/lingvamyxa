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

typedef struct LmTransSymbol {
    LmP0Text *name;
    LmTransSymbolKind kind;
    unsigned depth;
    int has_c_name;
    LmP0Text *c_name;
    LmOwnPtrStack param_names;
    int has_signature;
} LmTransSymbol;

typedef struct LmTransCleanup {
    unsigned id;
} LmTransCleanup;

typedef struct LmTransLoop {
    size_t cleanup_base;
} LmTransLoop;

typedef struct LmTransArgumentRange {
    const LmP0Field *first;
    const LmP0Field *stop;
    int present;
} LmTransArgumentRange;

typedef enum LmTransExprJobKind {
    LM_TRANS_EXPR_JOB_TEXT = 1,
    LM_TRANS_EXPR_JOB_NODE = 2,
    LM_TRANS_EXPR_JOB_FRAME = 3,
    LM_TRANS_EXPR_JOB_RANGE = 4,
    LM_TRANS_EXPR_JOB_CALL_ARGS = 5
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
    if (copy != NULL) {
        *copy = text;
    }
    return copy;
}

static void lm_trans_text_ref_destroy(LmP0Text **text) {
    if (text != NULL && *text != NULL) {
        lm_own_delete(*text, NULL);
        *text = NULL;
    }
}

static void lm_trans_text_ref_delete_any(void *object) {
    lm_own_delete(object, NULL);
}

static int lm_trans_text_ref_set(LmP0Text **target, LmP0Text text) {
    LmP0Text *copy;

    if (target == NULL) {
        return 1;
    }

    copy = lm_trans_text_ref_new(text);
    if (copy == NULL) {
        return 1;
    }

    lm_trans_text_ref_destroy(target);
    *target = copy;
    return 0;
}

static LmTransFunctionState *lm_trans_function_state_new(void) {
    return (LmTransFunctionState *)lm_own_new_zero(sizeof(LmTransFunctionState));
}

static void lm_trans_function_state_destroy(LmTransFunctionState *state) {
    if (state != NULL) {
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
        lm_trans_text_equals(text, "!") ||
        lm_trans_text_equals(text, "[") ||
        lm_trans_text_equals(text, "]");
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
    while (field != NULL && i < index) {
        field = field->next;
        ++i;
    }

    return field;
}

static int lm_trans_trailer_single_atom(const LmP0Trailer *trailer, LmP0Text *out_text) {
    const LmP0Field *field;

    if (trailer == NULL || out_text == NULL) {
        return 0;
    }

    field = trailer->body.first_field;
    if (
        field == NULL ||
        field->next != NULL ||
        field->value == NULL ||
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
    if (symbol != NULL) {
        lm_trans_text_ref_destroy(&symbol->name);
        lm_trans_text_ref_destroy(&symbol->c_name);
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
    if (symbol == NULL) {
        return NULL;
    }

    symbol->name = lm_trans_text_ref_new(name);
    if (symbol->name == NULL) {
        lm_trans_symbol_destroy(symbol);
        return NULL;
    }

    lm_own_ptr_stack_init(&symbol->param_names, lm_trans_text_ref_delete_any);
    symbol->kind = kind;
    symbol->depth = depth;
    return symbol;
}

static LmTransCleanup *lm_trans_cleanup_new(unsigned id) {
    LmTransCleanup *cleanup;

    cleanup = (LmTransCleanup *)lm_own_new_zero(sizeof(*cleanup));
    if (cleanup != NULL) {
        cleanup->id = id;
    }
    return cleanup;
}

static void lm_trans_cleanup_destroy(LmTransCleanup *cleanup) {
    lm_own_delete(cleanup, NULL);
}

static void lm_trans_cleanup_delete_any(void *object) {
    lm_trans_cleanup_destroy((LmTransCleanup *)object);
}

static LmTransLoop *lm_trans_loop_new(size_t cleanup_base) {
    LmTransLoop *loop;

    loop = (LmTransLoop *)lm_own_new_zero(sizeof(*loop));
    if (loop != NULL) {
        loop->cleanup_base = cleanup_base;
    }
    return loop;
}

static void lm_trans_loop_destroy(LmTransLoop *loop) {
    lm_own_delete(loop, NULL);
}

static void lm_trans_loop_delete_any(void *object) {
    lm_trans_loop_destroy((LmTransLoop *)object);
}

static LmTransNamespace *lm_trans_namespace_new(void) {
    LmTransNamespace *namespace_;

    namespace_ = (LmTransNamespace *)lm_own_new_zero(sizeof(LmTransNamespace));
    if (namespace_ != NULL) {
        lm_own_ptr_stack_init(&namespace_->items, lm_trans_symbol_delete_any);
        lm_own_ptr_stack_init(&namespace_->cleanups, lm_trans_cleanup_delete_any);
        lm_own_ptr_stack_init(&namespace_->loops, lm_trans_loop_delete_any);
    }
    return namespace_;
}

static void lm_trans_namespace_destroy(LmTransNamespace *namespace_) {
    if (namespace_ != NULL) {
        lm_own_ptr_stack_destroy(&namespace_->items);
        lm_own_ptr_stack_destroy(&namespace_->cleanups);
        lm_own_ptr_stack_destroy(&namespace_->loops);
        namespace_->depth = 0U;
        namespace_->next_cleanup_id = 0U;
        namespace_->return_type_node = NULL;
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
    if (namespace_ != NULL) {
        ++namespace_->depth;
    }
}

static void lm_trans_namespace_leave_scope(LmTransNamespace *namespace_) {
    LmTransSymbol *symbol;

    if (namespace_ == NULL) {
        return;
    }

    while (namespace_->items.count > 0U) {
        symbol = (LmTransSymbol *)lm_own_ptr_stack_top(&namespace_->items);
        if (symbol == NULL || symbol->depth != namespace_->depth) {
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

    if (namespace_ == NULL) {
        return 1;
    }

    cleanup = lm_trans_cleanup_new(id);
    if (cleanup == NULL) {
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

    if (namespace_ != NULL) {
        cleanup = (LmTransCleanup *)lm_own_ptr_stack_pop(&namespace_->cleanups);
        lm_trans_cleanup_destroy(cleanup);
    }
}

static int lm_trans_loop_push(LmTransNamespace *namespace_) {
    LmTransLoop *loop;

    if (namespace_ == NULL) {
        return 1;
    }

    loop = lm_trans_loop_new(namespace_->cleanups.count);
    if (loop == NULL) {
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

    if (namespace_ != NULL) {
        loop = (LmTransLoop *)lm_own_ptr_stack_pop(&namespace_->loops);
        lm_trans_loop_destroy(loop);
    }
}

static size_t lm_trans_loop_cleanup_base(const LmTransNamespace *namespace_) {
    const LmTransLoop *loop;

    if (namespace_ == NULL || namespace_->loops.count == 0U) {
        return namespace_ != NULL ? namespace_->cleanups.count : 0U;
    }

    loop = (const LmTransLoop *)lm_own_ptr_stack_top(&namespace_->loops);
    return loop != NULL ? loop->cleanup_base : namespace_->cleanups.count;
}

static int lm_trans_emit_sync_name(FILE *file, unsigned id) {
    return fprintf(file, "lm_sync_%u", id) < 0 ? 1 : 0;
}

static int lm_trans_emit_return_name(FILE *file, unsigned id) {
    return fprintf(file, "lm_return_%u", id) < 0 ? 1 : 0;
}

static int lm_trans_symbol_name_same(const LmP0Text *left, LmP0Text right) {
    return left != NULL && lm_trans_text_same(*left, right);
}

static const LmTransSymbol *lm_trans_namespace_find(
    const LmTransNamespace *namespace_,
    LmP0Text name
) {
    size_t i;
    const LmTransSymbol *symbol;

    if (namespace_ == NULL) {
        return NULL;
    }

    i = namespace_->items.count;
    while (i > 0U) {
        --i;
        symbol = (const LmTransSymbol *)lm_own_ptr_stack_at(&namespace_->items, i);
        if (
            symbol != NULL &&
            lm_trans_symbol_name_same(symbol->name, name)
        ) {
            return symbol;
        }
    }

    return NULL;
}

static LmTransSymbol *lm_trans_namespace_find_mutable(
    LmTransNamespace *namespace_,
    LmP0Text name
) {
    size_t i;
    LmTransSymbol *symbol;

    if (namespace_ == NULL) {
        return NULL;
    }

    i = namespace_->items.count;
    while (i > 0U) {
        --i;
        symbol = (LmTransSymbol *)lm_own_ptr_stack_at(&namespace_->items, i);
        if (
            symbol != NULL &&
            lm_trans_symbol_name_same(symbol->name, name)
        ) {
            return symbol;
        }
    }

    return NULL;
}

static int lm_trans_is_c_reference_name(LmP0Text name);

static int lm_trans_is_reserved_head_name(LmP0Text name) {
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

static int lm_trans_namespace_declare(
    LmTransNamespace *namespace_,
    LmP0Text name,
    LmTransSymbolKind kind
) {
    LmTransSymbol *symbol;
    const LmTransSymbol *existing;

    if (namespace_ == NULL) {
        return 1;
    }

    if (lm_trans_is_c_reference_name(name)) {
        fprintf(
            stderr,
            "trans L2 error: C name \"%.*s\" belongs to the ANSI C namespace and cannot be declared in Lingvamyxa namespace\n",
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
    if (existing != NULL) {
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
    if (symbol == NULL) {
        return 1;
    }
    if (lm_own_ptr_stack_push(&namespace_->items, symbol) != 0) {
        lm_trans_symbol_destroy(symbol);
        return 1;
    }
    return 0;
}

static int lm_trans_namespace_bind_c_reference(
    LmTransNamespace *namespace_,
    LmP0Text name,
    LmP0Text c_name
) {
    LmTransSymbol *symbol;

    if (namespace_ == NULL) {
        return 1;
    }

    if (lm_trans_is_c_reference_name(name)) {
        fprintf(
            stderr,
            "trans L2 error: C name \"%.*s\" belongs to the ANSI C namespace and cannot be rebound in Lingvamyxa namespace\n",
            (int)name.length,
            name.data
        );
        return 1;
    }

    if (lm_trans_is_reserved_head_name(name)) {
        fprintf(
            stderr,
            "trans L2 error: reserved head \"%.*s\" cannot be rebound\n",
            (int)name.length,
            name.data
        );
        return 1;
    }

    symbol = lm_trans_namespace_find_mutable(namespace_, name);
    if (symbol == NULL) {
        if (lm_trans_namespace_declare(namespace_, name, LM_TRANS_SYMBOL_VARIABLE) != 0) {
            return 1;
        }
        symbol = (LmTransSymbol *)lm_own_ptr_stack_top(&namespace_->items);
    } else if (symbol->kind != LM_TRANS_SYMBOL_VARIABLE) {
        fprintf(
            stderr,
            "trans L2 error: name \"%.*s\" is already visible as %s and cannot be rebound to a C value\n",
            (int)name.length,
            name.data,
            lm_trans_symbol_kind_name(symbol->kind)
        );
        return 1;
    }

    if (lm_trans_text_ref_set(&symbol->c_name, c_name) != 0) {
        return 1;
    }
    symbol->has_c_name = 1;
    return 0;
}

static const char *lm_trans_c_alias(LmP0Text name) {
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

    return NULL;
}

static int lm_trans_emit_name(FILE *file, LmP0Text name) {
    const char *alias;
    LmP0Text tail;

    if (lm_trans_text_starts_with(name, "c.")) {
        tail.data = name.data + 2U;
        tail.length = name.length - 2U;
        alias = lm_trans_c_alias(tail);
        if (alias != NULL) {
            return lm_trans_put(file, alias);
        }
        return lm_trans_write_text(file, tail);
    }

    return lm_trans_write_text(file, name);
}

static int lm_trans_accepts_c_type_name(LmP0Text name) {
    (void)name;
    return 1;
}

static int lm_trans_is_c_reference_name(LmP0Text name) {
    return lm_trans_text_starts_with(name, "c.") && name.length > 2U;
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
        node != NULL &&
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
    if (symbol == NULL) {
        fprintf(
            stderr,
            "trans L2 error: unknown Lingvamyxa name \"%.*s\"; use c.%.*s for ANSI C namespace access\n",
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
    const LmTransSymbol *symbol;

    if (lm_trans_validate_expr_atom(atom, namespace_) != 0) {
        return 1;
    }

    symbol = lm_trans_namespace_find(namespace_, atom);
    if (symbol != NULL && symbol->has_c_name) {
        return lm_trans_emit_name(file, *symbol->c_name);
    }

    return lm_trans_emit_name(file, atom);
}

static int lm_trans_atom_is_prefix_expr_operator(LmP0Text text) {
    return
        lm_trans_text_equals(text, "@") ||
        lm_trans_text_equals(text, "\\") ||
        lm_trans_text_equals(text, "!");
}

static int lm_trans_nodes_touch(const LmP0Node *left, const LmP0Node *right) {
    return
        left != NULL &&
        right != NULL &&
        left->span.offset + left->span.length == right->span.offset;
}

static int lm_trans_atom_is_infix_expr_operator(
    LmP0Text text,
    const LmP0Node *operator_node,
    const LmP0Node *previous_operand
) {
    if (lm_trans_text_equals(text, "\\")) {
        return
            previous_operand != NULL &&
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
        lm_trans_text_equals(text, "||");
}

static const LmP0Field *lm_trans_expr_segment_end(const LmP0Field *first) {
    const LmP0Field *field;
    const LmP0Field *operand;
    const LmP0Node *previous_operand;
    const LmP0Node *node;
    int bracket_depth;
    int c_dot_path;

    if (first == NULL) {
        return NULL;
    }

    field = first;
    previous_operand = NULL;
    c_dot_path = 0;

    node = field->value;
    if (
        node != NULL &&
        node->kind == LM_P0_NODE_ATOM &&
        lm_trans_atom_is_prefix_expr_operator(node->as.atom)
    ) {
        field = field->next;
        if (field == NULL) {
            return NULL;
        }
        previous_operand = field->value;
        c_dot_path = lm_trans_node_is_c_reference_atom(previous_operand);
        field = field->next;
    } else {
        previous_operand = node;
        c_dot_path = lm_trans_node_is_c_reference_atom(previous_operand);
        field = field->next;
    }

    while (field != NULL) {
        node = field->value;
        if (node == NULL || node->kind != LM_P0_NODE_ATOM) {
            break;
        }

        if (lm_trans_text_equals(node->as.atom, "[")) {
            bracket_depth = 1;
            field = field->next;
            while (field != NULL && bracket_depth > 0) {
                node = field->value;
                if (node != NULL && node->kind == LM_P0_NODE_ATOM) {
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
                previous_operand == NULL ||
                !lm_trans_nodes_touch(previous_operand, node)
            ) {
                break;
            }
            operand = field->next;
            if (operand == NULL) {
                return field;
            }
            previous_operand = operand->value;
            c_dot_path = 1;
            field = operand->next;
        } else if (lm_trans_atom_is_infix_expr_operator(node->as.atom, node, previous_operand)) {
            operand = field->next;
            if (operand == NULL) {
                return field;
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

    if (callee == NULL || out_index == NULL) {
        return 0;
    }

    for (i = 0U; i < callee->param_names.count; ++i) {
        param_name = (const LmP0Text *)lm_own_ptr_stack_at(&callee->param_names, i);
        if (param_name != NULL && lm_trans_text_same(*param_name, name)) {
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
        field == NULL ||
        field->value == NULL ||
        field->value->kind != LM_P0_NODE_FRAME ||
        callee == NULL ||
        out_index == NULL
    ) {
        return 0;
    }

    frame = &field->value->as.frame;
    if ((frame->flags & LM_P0_FRAME_COLON) == 0U) {
        return 0;
    }

    return lm_trans_signature_param_index(callee, frame->head, out_index);
}

static void lm_trans_expr_stack_destroy(LmTransExprStack *stack) {
    if (stack != NULL) {
        lm_own_value_stack_destroy(&stack->jobs);
    }
}

static int lm_trans_expr_stack_push(LmTransExprStack *stack, LmTransExprJob job) {
    if (stack == NULL) {
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
    range.previous_operand = NULL;
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

static int lm_trans_argument_ranges_append(
    LmOwnValueStack *ranges,
    const LmP0Field *first,
    const LmP0Field *stop
) {
    LmTransArgumentRange range;

    range.first = first;
    range.stop = stop;
    range.present = 1;
    return lm_own_value_stack_push(ranges, &range);
}

static int lm_trans_expr_stack_push_argument_ranges(
    LmTransExprStack *stack,
    const LmOwnValueStack *ranges
) {
    size_t index;
    const LmTransArgumentRange *range;

    if (ranges == NULL) {
        return 0;
    }

    index = ranges->count;
    while (index > 0U) {
        --index;
        range = (const LmTransArgumentRange *)lm_own_value_stack_at(ranges, index);
        if (range == NULL) {
            return 1;
        }
        if (lm_trans_expr_stack_push_range(stack, range->first, range->stop) != 0) {
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

    if (body == NULL) {
        return NULL;
    }

    field = body->first_field;
    if (
        field != NULL &&
        field->next == NULL &&
        field->value != NULL &&
        field->value->kind == LM_P0_NODE_STRUCTURE
    ) {
        field = field->value->as.structure.first_field;
    }

    return field;
}

static int lm_trans_expr_stack_schedule_call_args(
    LmTransExprStack *stack,
    const LmP0Structure *body,
    const LmTransSymbol *callee
) {
    const LmP0Field *field;
    const LmP0Field *next;
    LmOwnValueStack ranges;
    LmTransArgumentRange *range;
    const LmP0Frame *named_frame;
    size_t index;
    size_t named_index;
    int named_started;
    int is_named;
    int status;

    field = lm_trans_call_body_first_field(body);
    lm_own_value_stack_init(&ranges, sizeof(LmTransArgumentRange));

    if (callee != NULL && callee->has_signature) {
        if (lm_own_value_stack_resize_zero(&ranges, callee->param_names.count) != 0) {
            lm_own_value_stack_destroy(&ranges);
            return 1;
        }

        index = 0U;
        named_started = 0;
        while (field != NULL) {
            is_named = lm_trans_call_field_is_named_argument(field, callee, &named_index);
            if (is_named) {
                named_started = 1;
                range = (LmTransArgumentRange *)lm_own_value_stack_at(&ranges, named_index);
                if (range == NULL) {
                    lm_own_value_stack_destroy(&ranges);
                    return 1;
                }
                if (range->present) {
                    fprintf(stderr, "trans L2 error: duplicate named argument\n");
                    lm_own_value_stack_destroy(&ranges);
                    return 1;
                }
                named_frame = &field->value->as.frame;
                range->first = named_frame->body.first_field;
                range->stop = NULL;
                range->present = 1;
                field = field->next;
                continue;
            }

            if (named_started) {
                fprintf(stderr, "trans L2 error: positional argument after named argument\n");
                lm_own_value_stack_destroy(&ranges);
                return 1;
            }
            if (index >= callee->param_names.count) {
                fprintf(stderr, "trans L2 error: too many arguments\n");
                lm_own_value_stack_destroy(&ranges);
                return 1;
            }
            next = lm_trans_expr_segment_end(field);
            range = (LmTransArgumentRange *)lm_own_value_stack_at(&ranges, index);
            if (range == NULL) {
                lm_own_value_stack_destroy(&ranges);
                return 1;
            }
            range->first = field;
            range->stop = next;
            range->present = 1;
            ++index;
            field = next;
        }

        for (index = 0U; index < callee->param_names.count; ++index) {
            range = (LmTransArgumentRange *)lm_own_value_stack_at(&ranges, index);
            if (range == NULL) {
                lm_own_value_stack_destroy(&ranges);
                return 1;
            }
            if (!range->present) {
                fprintf(stderr, "trans L2 error: missing function argument\n");
                lm_own_value_stack_destroy(&ranges);
                return 1;
            }
        }

        status = lm_trans_expr_stack_push_argument_ranges(stack, &ranges);
        lm_own_value_stack_destroy(&ranges);
        return status;
    }

    while (field != NULL) {
        next = lm_trans_expr_segment_end(field);
        if (lm_trans_argument_ranges_append(&ranges, field, next) != 0) {
            lm_own_value_stack_destroy(&ranges);
            return 1;
        }
        field = next;
    }

    status = lm_trans_expr_stack_push_argument_ranges(stack, &ranges);
    lm_own_value_stack_destroy(&ranges);
    return status;
}

static int lm_trans_expr_stack_emit_frame(
    FILE *file,
    LmTransExprStack *stack,
    const LmP0Frame *frame,
    const LmTransNamespace *namespace_
) {
    const LmTransSymbol *symbol;
    const LmTransSymbol *callee_symbol;
    LmP0Text callee;

    if (frame == NULL) {
        return 0;
    }

    callee_symbol = NULL;
    callee = frame->head;
    if (!lm_trans_is_c_reference_name(frame->head)) {
        symbol = lm_trans_namespace_find(namespace_, frame->head);
        if (symbol == NULL) {
            fprintf(
                stderr,
                "trans L2 error: unknown Lingvamyxa function \"%.*s\"; use c.%.*s for ANSI C namespace access\n",
                (int)frame->head.length,
                frame->head.data,
                (int)frame->head.length,
                frame->head.data
            );
            return 1;
        }
        if (symbol->has_c_name) {
            callee = *symbol->c_name;
        } else if (symbol->kind != LM_TRANS_SYMBOL_FUNCTION) {
            fprintf(
                stderr,
                "trans L2 error: \"%.*s\" is %s, not a value-returning function\n",
                (int)frame->head.length,
                frame->head.data,
                lm_trans_symbol_kind_name(symbol->kind)
            );
            return 1;
        } else {
            callee_symbol = symbol;
        }
    }

    if (lm_trans_emit_name(file, callee) != 0) {
        return 1;
    }
    if (lm_trans_put(file, "(") != 0) {
        return 1;
    }
    if (lm_trans_expr_stack_push_text(stack, ")") != 0) {
        return 1;
    }
    return lm_trans_expr_stack_push_call_args(stack, &frame->body, callee_symbol);
}

static int lm_trans_expr_stack_emit_node(
    FILE *file,
    LmTransExprStack *stack,
    const LmP0Node *node,
    const LmTransNamespace *namespace_
) {
    if (node == NULL) {
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
        if (lm_trans_expr_stack_push_range(stack, node->as.structure.first_field, NULL) != 0) {
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

static int lm_trans_expr_stack_emit_range(
    FILE *file,
    LmTransExprStack *stack,
    LmTransExprRangeJob range,
    const LmTransNamespace *namespace_
) {
    const LmP0Field *field;
    const LmP0Node *node;
    const LmP0Node *previous_operand;
    int expect_field_name;
    int expect_c_field_name;
    int c_dot_path;
    LmTransExprRangeJob continuation;

    field = range.field;
    previous_operand = range.previous_operand;
    expect_field_name = range.expect_field_name;
    expect_c_field_name = range.expect_c_field_name;
    c_dot_path = range.c_dot_path;
    while (field != range.stop) {
        node = field->value;
        if (node != NULL && !(node->flags & LM_P0_NODE_INACTIVE)) {
            if (range.wrote && lm_trans_put(file, " ") != 0) {
                return 1;
            }

            if (node->kind == LM_P0_NODE_ATOM) {
                if (expect_field_name) {
                    if (!lm_trans_atom_is_identifier_like(node->as.atom)) {
                        fprintf(stderr, "trans L2 error: field-follow expects a field name\n");
                        return 1;
                    }
                    if (lm_trans_emit_name(file, node->as.atom) != 0) {
                        return 1;
                    }
                    previous_operand = node;
                    expect_field_name = 0;
                    c_dot_path = 0;
                } else if (expect_c_field_name) {
                    if (!lm_trans_atom_is_identifier_like(node->as.atom)) {
                        fprintf(stderr, "trans L2 error: C value-field dot expects a field name\n");
                        return 1;
                    }
                    if (lm_trans_emit_name(file, node->as.atom) != 0) {
                        return 1;
                    }
                    previous_operand = node;
                    expect_c_field_name = 0;
                    c_dot_path = 1;
                } else if (lm_trans_text_equals(node->as.atom, ".")) {
                    if (
                        !c_dot_path ||
                        previous_operand == NULL ||
                        !lm_trans_nodes_touch(previous_operand, node)
                    ) {
                        fprintf(stderr, "trans L2 error: C value-field dot must follow a c.name path\n");
                        return 1;
                    }
                    if (lm_trans_put(file, ".") != 0) {
                        return 1;
                    }
                    previous_operand = NULL;
                    expect_c_field_name = 1;
                } else if (lm_trans_text_equals(node->as.atom, "=")) {
                    if (lm_trans_put(file, "==") != 0) {
                        return 1;
                    }
                    previous_operand = NULL;
                    c_dot_path = 0;
                } else if (lm_trans_text_equals(node->as.atom, "@")) {
                    if (lm_trans_put(file, "&") != 0) {
                        return 1;
                    }
                    previous_operand = NULL;
                    c_dot_path = 0;
                } else if (lm_trans_text_equals(node->as.atom, "\\")) {
                    if (previous_operand != NULL && lm_trans_nodes_touch(previous_operand, node)) {
                        if (lm_trans_put(file, "->") != 0) {
                            return 1;
                        }
                        previous_operand = NULL;
                        expect_field_name = 1;
                        c_dot_path = 0;
                    } else {
                        if (lm_trans_put(file, "*") != 0) {
                            return 1;
                        }
                        previous_operand = NULL;
                        c_dot_path = 0;
                    }
                } else {
                    if (lm_trans_emit_expr_atom(file, node->as.atom, namespace_) != 0) {
                        return 1;
                    }
                    previous_operand = lm_trans_atom_is_operand_like(node->as.atom) ? node : NULL;
                    c_dot_path = lm_trans_node_is_c_reference_atom(previous_operand);
                }
            } else {
                continuation.field = field->next;
                continuation.stop = range.stop;
                continuation.wrote = 1;
                continuation.previous_operand = node;
                continuation.expect_field_name = 0;
                continuation.expect_c_field_name = 0;
                continuation.c_dot_path = 0;
                if (lm_trans_expr_stack_push_range_state(stack, continuation) != 0) {
                    return 1;
                }
                if (lm_trans_expr_stack_push_node(stack, node) != 0) {
                    return 1;
                }
                return 0;
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
    while (field != NULL) {
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
    return lm_trans_emit_expr_range(file, first, NULL, namespace_);
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
    job.as.range.previous_operand = NULL;
    job.as.range.expect_field_name = 0;
    job.as.range.expect_c_field_name = 0;
    job.as.range.c_dot_path = 0;
    return lm_trans_emit_expr_stack_run(file, job, namespace_);
}

static int lm_trans_emit_type_node(FILE *file, const LmP0Node *type_node) {
    const LmP0Field *field;
    size_t i;

    if (type_node == NULL) {
        fprintf(stderr, "trans L2 error: type position expects a name\n");
        return 1;
    }

    if (type_node->kind == LM_P0_NODE_ATOM) {
        return lm_trans_emit_name(file, type_node->as.atom);
    }

    if (
        type_node->kind == LM_P0_NODE_FRAME &&
        lm_trans_text_equals(type_node->as.frame.head, "const")
    ) {
        field = type_node->as.frame.body.first_field;
        if (field == NULL || field->next != NULL || field->value == NULL) {
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
        if (field == NULL || field->next != NULL || field->value == NULL) {
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
    if (namespace_ != NULL && namespace_->return_type_is_struct) {
        if (namespace_->return_type_name == NULL) {
            return 1;
        }
        return lm_trans_emit_function_return_struct_type_name(file, *namespace_->return_type_name);
    }
    return lm_trans_emit_type_node(file, namespace_ != NULL ? namespace_->return_type_node : NULL);
}

static int lm_trans_emit_type_and_name(
    FILE *file,
    const LmP0Node *type_node,
    LmP0Text name,
    size_t pointer_depth,
    const LmTransNamespace *namespace_
) {
    size_t i;

    if (type_node == NULL) {
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

    if (node == NULL || node->kind != LM_P0_NODE_FRAME) {
        fprintf(stderr, "trans L2 error: parameter must be a typed frame\n");
        return 1;
    }

    if (lm_trans_text_equals(node->as.frame.head, "const")) {
        field0 = node->as.frame.body.first_field;
        param_node = NULL;
        if (
            field0 != NULL &&
            field0->next == NULL &&
            field0->value != NULL
        ) {
            if (field0->value->kind == LM_P0_NODE_FRAME) {
                param_node = field0->value;
            } else if (field0->value->kind == LM_P0_NODE_STRUCTURE) {
                inner_field = field0->value->as.structure.first_field;
                if (
                    inner_field != NULL &&
                    inner_field->next == NULL &&
                    inner_field->value != NULL &&
                    inner_field->value->kind == LM_P0_NODE_FRAME
                ) {
                    param_node = inner_field->value;
                }
            }
        }
        if (param_node == NULL) {
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
        if (field0 == NULL || field0->value == NULL || field0->value->kind != LM_P0_NODE_ATOM) {
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
            field0 == NULL ||
            field1 == NULL ||
            field1->value == NULL ||
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

    if (frame == NULL || frame->body.first_field == NULL) {
        return NULL;
    }

    end = lm_trans_expr_segment_end(frame->body.first_field);
    if (end != NULL && end->value != NULL && end->value->kind == LM_P0_NODE_STRUCTURE) {
        return end;
    }

    return NULL;
}

static int lm_trans_emit_control_condition(
    FILE *file,
    const LmP0Frame *frame,
    const LmTransNamespace *namespace_
) {
    const LmP0Field *body_start;
    const LmP0Field *first;

    first = frame->body.first_field;
    if (first == NULL) {
        return 0;
    }

    body_start = lm_trans_control_body_start(frame);
    return lm_trans_emit_expr_range(file, first, body_start, namespace_);
}

static int lm_trans_frame_has_positional_name_argument(const LmP0Frame *frame) {
    if (frame == NULL) {
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

static int lm_trans_name_argument_from_frame(const LmP0Frame *frame, LmP0Text *out_name) {
    const LmP0Field *field;
    const LmP0Field *name_field;
    const LmP0Frame *child_frame;

    if (frame == NULL || out_name == NULL) {
        return 0;
    }

    field = frame->body.first_field;
    while (field != NULL) {
        if (field->value != NULL && field->value->kind == LM_P0_NODE_FRAME) {
            child_frame = &field->value->as.frame;
            if (lm_trans_text_equals(child_frame->head, "name")) {
                name_field = child_frame->body.first_field;
                if (
                    name_field != NULL &&
                    name_field->next == NULL &&
                    name_field->value != NULL &&
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
    if (field == NULL || field->value == NULL) {
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
    if (frame == NULL || out_target == NULL) {
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

    if (frame == NULL || frame->trailer == NULL) {
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

    if (namespace_ == NULL || cleanup_base > namespace_->cleanups.count) {
        return 0;
    }

    index = namespace_->cleanups.count;
    while (index > cleanup_base) {
        --index;
        cleanup = (const LmTransCleanup *)lm_own_ptr_stack_at(&namespace_->cleanups, index);
        if (cleanup == NULL) {
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

    if (return_fields == NULL) {
        if (lm_trans_emit_cleanups_until(file, indent, namespace_, 0U) != 0) {
            return 1;
        }
        if (lm_trans_emit_indent(file, indent) != 0) {
            return 1;
        }
        return lm_trans_put(file, "return;\n");
    }

    if (namespace_ != NULL && namespace_->cleanups.count > 0U && namespace_->return_type_node != NULL) {
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
    if (trailer == NULL) {
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
    if (name_field == NULL || name_field->value == NULL || name_field->value->kind != LM_P0_NODE_ATOM) {
        fprintf(stderr, "trans L2 error: declaration expects a name\n");
        return 1;
    }

    name_node = name_field->value;
    if (lm_trans_emit_indent(file, indent) != 0) {
        return 1;
    }
    if (qualifier != NULL && lm_trans_put(file, qualifier) != 0) {
        return 1;
    }
    if (lm_trans_emit_name(file, frame->head) != 0) {
        return 1;
    }
    if (lm_trans_put(file, " ") != 0) {
        return 1;
    }
    if (lm_trans_write_text(file, name_node->as.atom) != 0) {
        return 1;
    }
    if (name_field->next != NULL) {
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

    if (lm_trans_is_c_reference_name(head) && lm_trans_accepts_c_type_name(head)) {
        return 1;
    }

    symbol = lm_trans_namespace_find(namespace_, head);
    return symbol != NULL && symbol->kind == LM_TRANS_SYMBOL_STRUCTURE;
}

static int lm_trans_frame_looks_storage_declaration(
    const LmP0Frame *frame,
    const LmTransNamespace *namespace_
) {
    const LmP0Field *name_field;

    if (
        frame == NULL ||
        !lm_trans_head_can_declare_storage(frame->head, namespace_)
    ) {
        return 0;
    }

    name_field = lm_trans_nth_field(&frame->body, 0U);
    if (
        name_field == NULL ||
        name_field->value == NULL ||
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
        type_field == NULL ||
        name_field == NULL ||
        name_field->value == NULL ||
        name_field->value->kind != LM_P0_NODE_ATOM
    ) {
        fprintf(stderr, "trans L2 error: @ declaration expects type and name\n");
        return 1;
    }

    name_node = name_field->value;
    if (lm_trans_emit_indent(file, indent) != 0) {
        return 1;
    }
    if (qualifier != NULL && lm_trans_put(file, qualifier) != 0) {
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
    if (name_field->next != NULL) {
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
        field != NULL &&
        field->value != NULL &&
        field->value->kind == LM_P0_NODE_ATOM &&
        lm_trans_text_all_char(field->value->as.atom, '@')
    ) {
        pointer_depth += field->value->as.atom.length;
        field = field->next;
    }

    type_field = field;
    name_field = type_field != NULL ? type_field->next : NULL;
    size_field = name_field != NULL ? name_field->next : NULL;
    if (
        type_field == NULL ||
        name_field == NULL ||
        size_field == NULL ||
        name_field->value == NULL ||
        name_field->value->kind != LM_P0_NODE_ATOM
    ) {
        fprintf(stderr, "trans L2 error: [] declaration expects [@...] type name size\n");
        return 1;
    }

    name_node = name_field->value;
    if (lm_trans_emit_indent(file, indent) != 0) {
        return 1;
    }
    if (qualifier != NULL && lm_trans_put(file, qualifier) != 0) {
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
    if (size_field->next != NULL) {
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
        field == NULL ||
        field->next != NULL ||
        field->value == NULL ||
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
    const LmTransSymbol *symbol;
    const LmTransSymbol *callee_symbol;
    LmP0Text callee;

    callee_symbol = NULL;
    callee = frame->head;
    if (!lm_trans_is_c_reference_name(frame->head)) {
        symbol = lm_trans_namespace_find(namespace_, frame->head);
        if (symbol == NULL) {
            fprintf(
                stderr,
                "trans L2 error: unknown Lingvamyxa callable \"%.*s\"; use c.%.*s for ANSI C namespace access\n",
                (int)frame->head.length,
                frame->head.data,
                (int)frame->head.length,
                frame->head.data
            );
            return 1;
        }
        if (symbol->has_c_name) {
            callee = *symbol->c_name;
        } else if (
            symbol->kind != LM_TRANS_SYMBOL_FUNCTION &&
            symbol->kind != LM_TRANS_SYMBOL_PROCEDURE
        ) {
            fprintf(
                stderr,
                "trans L2 error: \"%.*s\" is %s, not a callable\n",
                (int)frame->head.length,
                frame->head.data,
                lm_trans_symbol_kind_name(symbol->kind)
            );
            return 1;
        } else {
            callee_symbol = symbol;
        }
    }

    if (lm_trans_emit_indent(file, indent) != 0) {
        return 1;
    }
    if (lm_trans_emit_name(file, callee) != 0) {
        return 1;
    }
    if (lm_trans_put(file, "(") != 0) {
        return 1;
    }
    if (lm_trans_emit_call_args(file, &frame->body, namespace_, callee_symbol) != 0) {
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

static int lm_trans_frame_single_c_reference_body(
    const LmP0Frame *frame,
    LmP0Text *out_c_name
) {
    const LmP0Field *field;

    if (frame == NULL || out_c_name == NULL) {
        return 0;
    }

    field = frame->body.first_field;
    if (
        field == NULL ||
        field->next != NULL ||
        field->value == NULL ||
        field->value->kind != LM_P0_NODE_ATOM ||
        !lm_trans_is_c_reference_name(field->value->as.atom)
    ) {
        return 0;
    }

    *out_c_name = field->value->as.atom;
    return 1;
}

static int lm_trans_emit_c_reference_binding(
    const LmP0Frame *frame,
    LmTransNamespace *namespace_
) {
    LmP0Text c_name;

    if (!lm_trans_frame_single_c_reference_body(frame, &c_name)) {
        return 0;
    }

    return lm_trans_namespace_bind_c_reference(namespace_, frame->head, c_name);
}

static int lm_trans_frame_looks_named_structure_declaration(const LmP0Frame *frame) {
    return
        frame != NULL &&
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

    if (node == NULL || (node->flags & LM_P0_NODE_INACTIVE)) {
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
            name_field == NULL ||
            name_field->next != NULL ||
            name_field->value == NULL ||
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
            type_field == NULL ||
            name_field == NULL ||
            name_field->next != NULL ||
            name_field->value == NULL ||
            name_field->value->kind != LM_P0_NODE_ATOM
        ) {
            fprintf(stderr, "trans L2 error: pointer structure field expects type and name\n");
            return 1;
        }
        name_node = name_field->value;
        if (lm_trans_emit_indent(file, indent) != 0) {
            return 1;
        }
        if (qualifier != NULL && lm_trans_put(file, qualifier) != 0) {
            return 1;
        }
        if (
            lm_trans_emit_type_and_name(
                file,
                type_field->value,
                name_node->as.atom,
                frame->head.length,
                NULL
            ) != 0
        ) {
            return 1;
        }
        return lm_trans_put(file, ";\n");
    }

    name_field = lm_trans_nth_field(&frame->body, 0U);
    if (
        name_field == NULL ||
        name_field->next != NULL ||
        name_field->value == NULL ||
        name_field->value->kind != LM_P0_NODE_ATOM
    ) {
        fprintf(stderr, "trans L2 error: structure field expects Type: name\n");
        return 1;
    }

    if (lm_trans_emit_indent(file, indent) != 0) {
        return 1;
    }
    if (qualifier != NULL && lm_trans_put(file, qualifier) != 0) {
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
    while (field != NULL) {
        if (lm_trans_emit_struct_field(file, field->value, 1U) != 0) {
            return 1;
        }
        field = field->next;
    }

    if (lm_trans_put(file, "};\n\n") != 0) {
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

    if (return_node == NULL || return_node->kind != LM_P0_NODE_STRUCTURE) {
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
    if (field == NULL) {
        fprintf(stderr, "trans L2 error: fm return Structure must not be empty\n");
        return 1;
    }
    while (field != NULL) {
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
    if (stack != NULL) {
        lm_own_value_stack_destroy(&stack->jobs);
    }
}

static int lm_trans_statement_stack_push(LmTransStatementStack *stack, LmTransStatementJob job) {
    if (stack == NULL) {
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
        first != NULL &&
        first->next == NULL &&
        first->value != NULL &&
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

    if (frame == NULL || frame->body.first_field == NULL) {
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

static int lm_trans_statement_stack_emit_frame(
    FILE *file,
    LmTransStatementStack *stack,
    const LmP0Frame *frame,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    int status;
    const LmTransSymbol *symbol;
    LmP0Text c_name;

    status =
        lm_trans_statement_stack_push_frame_job(stack, LM_TRANS_STATEMENT_JOB_TRAILER, frame, indent) ||
        lm_trans_statement_stack_push_frame_job(stack, LM_TRANS_STATEMENT_JOB_VALIDATE_END, frame, indent);
    if (status != 0) {
        return 1;
    }

    if (lm_trans_text_equals(frame->head, "return")) {
        return lm_trans_emit_return_statement(file, frame->body.first_field, indent, namespace_);
    }
    if (lm_trans_text_equals(frame->head, "if")) {
        return lm_trans_statement_stack_schedule_control(file, stack, frame, indent, namespace_, "if");
    }
    if (lm_trans_text_equals(frame->head, "while")) {
        return lm_trans_statement_stack_schedule_control(file, stack, frame, indent, namespace_, "while");
    }
    if (lm_trans_text_equals(frame->head, "else")) {
        return lm_trans_statement_stack_schedule_else(file, stack, frame, indent, namespace_);
    }
    if (lm_trans_text_equals(frame->head, "synchronized")) {
        return lm_trans_statement_stack_schedule_synchronized(file, stack, frame, indent, namespace_);
    }
    if (lm_trans_text_equals(frame->head, "break")) {
        if (lm_trans_emit_cleanups_until(file, indent, namespace_, lm_trans_loop_cleanup_base(namespace_)) != 0) {
            return 1;
        }
        if (lm_trans_emit_indent(file, indent) != 0) {
            return 1;
        }
        return lm_trans_put(file, "break;\n");
    }
    if (lm_trans_text_equals(frame->head, "continue")) {
        if (lm_trans_emit_cleanups_until(file, indent, namespace_, lm_trans_loop_cleanup_base(namespace_)) != 0) {
            return 1;
        }
        if (lm_trans_emit_indent(file, indent) != 0) {
            return 1;
        }
        return lm_trans_put(file, "continue;\n");
    }
    if (lm_trans_text_equals(frame->head, "const")) {
        return lm_trans_emit_const_statement(file, frame, indent, namespace_);
    }
    if (lm_trans_text_equals(frame->head, "[]")) {
        return lm_trans_emit_array_declaration(file, frame, indent, namespace_);
    }
    if (lm_trans_frame_looks_storage_declaration(frame, namespace_)) {
        return lm_trans_emit_declaration(file, frame, indent, namespace_);
    }
    if (lm_trans_text_all_char(frame->head, '@')) {
        return lm_trans_emit_pointer_declaration(file, frame, indent, namespace_);
    }
    if (lm_trans_head_looks_assignable_target(frame->head)) {
        return lm_trans_emit_target_assignment(file, frame, indent, namespace_);
    }
    if (
        !lm_trans_is_c_reference_name(frame->head) &&
        !lm_trans_is_reserved_head_name(frame->head) &&
        lm_trans_frame_single_c_reference_body(frame, &c_name)
    ) {
        return lm_trans_emit_c_reference_binding(frame, namespace_);
    }
    if (lm_trans_frame_looks_label_declaration(frame)) {
        return lm_trans_statement_stack_schedule_label(file, stack, frame, indent, namespace_);
    }
    if (lm_trans_is_c_reference_name(frame->head)) {
        return lm_trans_emit_call_statement(file, frame, indent, namespace_);
    }

    symbol = lm_trans_namespace_find(namespace_, frame->head);
    if (symbol == NULL) {
        fprintf(
            stderr,
            "trans L2 error: unknown Lingvamyxa name \"%.*s\"; use c.%.*s for ANSI C namespace access\n",
            (int)frame->head.length,
            frame->head.data,
            (int)frame->head.length,
            frame->head.data
        );
        return 1;
    }
    if (symbol->kind == LM_TRANS_SYMBOL_VARIABLE) {
        if (symbol->has_c_name) {
            return lm_trans_emit_call_statement(file, frame, indent, namespace_);
        }
        return lm_trans_emit_assignment(file, frame, indent, namespace_);
    }
    if (
        symbol->kind == LM_TRANS_SYMBOL_FUNCTION ||
        symbol->kind == LM_TRANS_SYMBOL_PROCEDURE
    ) {
        return lm_trans_emit_call_statement(file, frame, indent, namespace_);
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

static int lm_trans_statement_stack_emit_node(
    FILE *file,
    LmTransStatementStack *stack,
    const LmP0Node *node,
    unsigned indent,
    LmTransNamespace *namespace_
) {
    if (node == NULL || (node->flags & LM_P0_NODE_INACTIVE)) {
        return 0;
    }

    if (node->kind == LM_P0_NODE_ATOM) {
        if (
            lm_trans_text_equals(node->as.atom, "break") ||
            lm_trans_text_equals(node->as.atom, "continue")
        ) {
            if (lm_trans_emit_cleanups_until(file, indent, namespace_, lm_trans_loop_cleanup_base(namespace_)) != 0) {
                return 1;
            }
            if (lm_trans_emit_indent(file, indent) != 0) {
                return 1;
            }
            if (lm_trans_write_text(file, node->as.atom) != 0) {
                return 1;
            }
            return lm_trans_put(file, ";\n");
        }

        if (lm_trans_text_equals(node->as.atom, "return")) {
            return lm_trans_emit_return_statement(file, NULL, indent, namespace_);
        }

        if (lm_trans_atom_starts_string(node->as.atom)) {
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
            if (field != NULL) {
                if (field->next != NULL) {
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

    if (params == NULL || params->kind != LM_P0_NODE_STRUCTURE) {
        fprintf(stderr, "trans L2 error: function parameters must be a Structure\n");
        return 1;
    }

    first = 1;
    field = params->as.structure.first_field;
    while (field != NULL) {
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

    if (node == NULL || out_frame == NULL) {
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
        field != NULL &&
        field->next == NULL &&
        field->value != NULL &&
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

    if (out_name == NULL || !lm_trans_single_frame_node(node, &frame)) {
        return 0;
    }

    if (lm_trans_text_equals(frame->head, "const")) {
        field = frame->body.first_field;
        if (field == NULL || field->next != NULL || field->value == NULL) {
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
        name_field == NULL ||
        name_field->value == NULL ||
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

    if (namespace_ == NULL || function_frame == NULL) {
        return 1;
    }

    symbol = lm_trans_namespace_find_mutable(namespace_, name);
    if (symbol == NULL) {
        return 1;
    }

    params_field = lm_trans_nth_field(&function_frame->body, 1U);
    if (
        params_field == NULL ||
        params_field->value == NULL ||
        params_field->value->kind != LM_P0_NODE_STRUCTURE
    ) {
        fprintf(stderr, "trans L2 error: fn/sub expects a parameter Structure\n");
        return 1;
    }

    lm_own_ptr_stack_init(&param_names, lm_trans_text_ref_delete_any);

    index = 0U;
    field = params_field->value->as.structure.first_field;
    while (field != NULL) {
        if (!lm_trans_formal_param_name(field->value, &param_name)) {
            fprintf(stderr, "trans L2 error: function parameter must expose a binding name\n");
            lm_own_ptr_stack_destroy(&param_names);
            return 1;
        }
        for (i = 0U; i < index; ++i) {
            param_name_ref = (LmP0Text *)lm_own_ptr_stack_at(&param_names, i);
            if (param_name_ref != NULL && lm_trans_text_same(*param_name_ref, param_name)) {
                fprintf(stderr, "trans L2 error: duplicate function parameter name\n");
                lm_own_ptr_stack_destroy(&param_names);
                return 1;
            }
        }
        param_name_ref = lm_trans_text_ref_new(param_name);
        if (param_name_ref == NULL) {
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

static int lm_trans_function_decl_name(const LmP0Frame *frame, LmP0Text *out_name) {
    const LmP0Field *name_field;

    if (frame == NULL || out_name == NULL) {
        return 0;
    }

    name_field = lm_trans_nth_field(&frame->body, 0U);
    if (
        name_field == NULL ||
        name_field->value == NULL ||
        name_field->value->kind != LM_P0_NODE_ATOM
    ) {
        return 0;
    }

    *out_name = name_field->value->as.atom;
    return 1;
}

static int lm_trans_top_level_function_frame_info(
    const LmP0Frame *frame,
    const LmP0Frame **out_function,
    LmTransSymbolKind *out_kind,
    int *out_external
) {
    const LmP0Field *field;
    const LmP0Frame *inner;

    if (
        frame == NULL ||
        out_function == NULL ||
        out_kind == NULL ||
        out_external == NULL
    ) {
        return 0;
    }

    if (
        lm_trans_text_equals(frame->head, "fn") ||
        lm_trans_text_equals(frame->head, "fm")
    ) {
        *out_function = frame;
        *out_kind = LM_TRANS_SYMBOL_FUNCTION;
        *out_external = 0;
        return 1;
    }

    if (lm_trans_text_equals(frame->head, "sub")) {
        *out_function = frame;
        *out_kind = LM_TRANS_SYMBOL_PROCEDURE;
        *out_external = 0;
        return 1;
    }

    if (!lm_trans_text_equals(frame->head, "external")) {
        return 0;
    }

    field = frame->body.first_field;
    if (
        field == NULL ||
        field->next != NULL ||
        field->value == NULL ||
        field->value->kind != LM_P0_NODE_FRAME
    ) {
        fprintf(stderr, "trans L2 error: external receiver expects exactly one fn/sub frame\n");
        return -1;
    }

    inner = &field->value->as.frame;
    if (
        lm_trans_text_equals(inner->head, "fn") ||
        lm_trans_text_equals(inner->head, "fm")
    ) {
        *out_function = inner;
        *out_kind = LM_TRANS_SYMBOL_FUNCTION;
        *out_external = 1;
        return 1;
    }

    if (lm_trans_text_equals(inner->head, "sub")) {
        *out_function = inner;
        *out_kind = LM_TRANS_SYMBOL_PROCEDURE;
        *out_external = 1;
        return 1;
    }

    fprintf(stderr, "trans L2 error: external receiver expects fn/sub frame\n");
    return -1;
}

static int lm_trans_emit_function(
    FILE *file,
    const LmP0Frame *frame,
    int is_sub,
    int is_external,
    LmTransNamespace *namespace_
) {
    const LmP0Field *name_field;
    const LmP0Field *params_field;
    const LmP0Field *return_field;
    const LmP0Field *body_field;
    const LmP0Field *body_start;
    LmTransFunctionState *state;
    int is_struct_return;
    int status;
    state = NULL;

    name_field = lm_trans_nth_field(&frame->body, 0U);
    params_field = lm_trans_nth_field(&frame->body, 1U);

    if (
        name_field == NULL ||
        params_field == NULL ||
        name_field->value == NULL ||
        name_field->value->kind != LM_P0_NODE_ATOM
    ) {
        fprintf(stderr, "trans L2 error: fn/sub expects name and parameters\n");
        return 1;
    }

    is_struct_return = !is_sub && lm_trans_text_equals(frame->head, "fm");

    if (is_sub) {
        if (!is_external) {
            if (lm_trans_put(file, "static ") != 0) {
                return 1;
            }
        }
        if (lm_trans_put(file, "void ") != 0) {
            return 1;
        }
        body_field = lm_trans_nth_field(&frame->body, 2U);
        if (
            body_field != NULL &&
            body_field->next == NULL &&
            body_field->value != NULL &&
            body_field->value->kind == LM_P0_NODE_STRUCTURE
        ) {
            body_start = body_field->value->as.structure.first_field;
        } else {
            body_start = params_field->next;
        }
    } else {
        return_field = lm_trans_nth_field(&frame->body, 2U);
        if (return_field == NULL || return_field->value == NULL) {
            fprintf(stderr, "trans L2 error: fn expects return type\n");
            return 1;
        }
        if (is_struct_return) {
            if (lm_trans_emit_function_return_structure(file, name_field->value->as.atom, return_field->value) != 0) {
                return 1;
            }
        } else {
            if (return_field->value->kind == LM_P0_NODE_STRUCTURE) {
                fprintf(stderr, "trans L2 error: fn expects a single-value return type; use fm for Structure return\n");
                return 1;
            }
        }
        if (!is_external) {
            if (lm_trans_put(file, "static ") != 0) {
                return 1;
            }
        }
        if (is_struct_return) {
            if (lm_trans_emit_function_return_struct_type_name(file, name_field->value->as.atom) != 0) {
                return 1;
            }
        } else {
            if (lm_trans_emit_type_node(file, return_field->value) != 0) {
                return 1;
            }
        }
        if (lm_trans_put(file, " ") != 0) {
            return 1;
        }
        body_field = lm_trans_nth_field(&frame->body, 3U);
        if (
            body_field != NULL &&
            body_field->next == NULL &&
            body_field->value != NULL &&
            body_field->value->kind == LM_P0_NODE_STRUCTURE
        ) {
            body_start = body_field->value->as.structure.first_field;
        } else {
            body_start = return_field->next;
        }
    }

    if (lm_trans_write_text(file, name_field->value->as.atom) != 0) {
        return 1;
    }
    if (lm_trans_put(file, "(") != 0) {
        return 1;
    }
    lm_trans_namespace_enter_scope(namespace_);
    if (lm_trans_emit_params(file, params_field->value, namespace_) != 0) {
        lm_trans_namespace_leave_scope(namespace_);
        return 1;
    }
    if (lm_trans_put(file, ") {\n") != 0) {
        lm_trans_namespace_leave_scope(namespace_);
        return 1;
    }

    state = lm_trans_function_state_new();
    if (state == NULL) {
        lm_trans_namespace_leave_scope(namespace_);
        return 1;
    }

    if (is_struct_return) {
        state->current_return_type_name = lm_trans_text_ref_new(name_field->value->as.atom);
        if (state->current_return_type_name == NULL) {
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
    namespace_->return_type_node = is_sub ? NULL : return_field->value;
    namespace_->return_type_is_struct = is_struct_return;
    namespace_->return_type_name = is_struct_return ? state->current_return_type_name : state->previous_return_type_name;
    namespace_->next_return_id = 0U;
    lm_own_ptr_stack_init(&namespace_->cleanups, lm_trans_cleanup_delete_any);
    lm_own_ptr_stack_init(&namespace_->loops, lm_trans_loop_delete_any);

    status = lm_trans_emit_statement_list(file, body_start, 1U, namespace_);
    if (status == 0) {
        status = lm_trans_validate_end_trailer(frame);
    }
    if (status == 0) {
        status = lm_trans_emit_trailer_statement(file, frame->trailer, 1U, namespace_);
    }
    if (status == 0) {
        status = lm_trans_put(file, "}\n\n");
    }

    namespace_->return_type_node = state->previous_return_type_node;
    namespace_->return_type_is_struct = state->previous_return_type_is_struct;
    namespace_->return_type_name = state->previous_return_type_name;
    namespace_->next_return_id = state->previous_next_return_id;
    lm_own_ptr_stack_destroy(&namespace_->cleanups);
    lm_own_ptr_stack_destroy(&namespace_->loops);
    namespace_->cleanups = state->previous_cleanups;
    namespace_->loops = state->previous_loops;
    lm_own_ptr_stack_init(&state->previous_cleanups, NULL);
    lm_own_ptr_stack_init(&state->previous_loops, NULL);
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
    LmP0Text name;
    LmP0Text c_name;
    const LmP0Frame *function_frame;
    LmTransSymbolKind kind;
    int function_info;
    int is_external;
    int status;

    namespace_ = lm_trans_namespace_new();
    if (namespace_ == NULL) {
        return 1;
    }

    field = l2->body.first_field;
    while (field != NULL) {
        node = field->value;
        if (node == NULL || (node->flags & LM_P0_NODE_INACTIVE)) {
            field = field->next;
            continue;
        }

        if (node->kind != LM_P0_NODE_FRAME) {
            fprintf(stderr, "trans L2 error: top-level L2 field must be consumed by an L2 receiver\n");
            lm_trans_namespace_delete(namespace_);
            return 1;
        }

        if (node->kind == LM_P0_NODE_FRAME) {
            function_info = lm_trans_top_level_function_frame_info(
                &node->as.frame,
                &function_frame,
                &kind,
                &is_external
            );
            if (function_info < 0) {
                lm_trans_namespace_delete(namespace_);
                return 1;
            }

            if (function_info > 0) {
                if (!lm_trans_function_decl_name(function_frame, &name)) {
                    fprintf(stderr, "trans L2 error: fn/sub expects name and parameters\n");
                    lm_trans_namespace_delete(namespace_);
                    return 1;
                }
                if (lm_trans_namespace_declare(namespace_, name, kind) != 0) {
                    lm_trans_namespace_delete(namespace_);
                    return 1;
                }
                if (lm_trans_namespace_set_signature(namespace_, name, function_frame) != 0) {
                    lm_trans_namespace_delete(namespace_);
                    return 1;
                }
            } else if (
                !lm_trans_is_c_reference_name(node->as.frame.head) &&
                !lm_trans_is_reserved_head_name(node->as.frame.head) &&
                lm_trans_frame_single_c_reference_body(&node->as.frame, &c_name)
            ) {
                if (lm_trans_namespace_bind_c_reference(namespace_, node->as.frame.head, c_name) != 0) {
                    lm_trans_namespace_delete(namespace_);
                    return 1;
                }
                field = field->next;
                continue;
            } else if (lm_trans_frame_looks_named_structure_declaration(&node->as.frame)) {
                if (lm_trans_namespace_declare(namespace_, node->as.frame.head, LM_TRANS_SYMBOL_STRUCTURE) != 0) {
                    lm_trans_namespace_delete(namespace_);
                    return 1;
                }
                field = field->next;
                continue;
            } else if (lm_trans_text_equals(node->as.frame.head, "L1")) {
                field = field->next;
                continue;
            } else {
                fprintf(stderr, "trans L2 error: top-level L2 field must be fn, sub, external fn/sub, L1 frame, named Structure, or namespace binding to c.name\n");
                lm_trans_namespace_delete(namespace_);
                return 1;
            }
        }
        field = field->next;
    }

    field = l2->body.first_field;
    while (field != NULL) {
        node = field->value;
        if (
            node != NULL &&
            node->kind == LM_P0_NODE_FRAME &&
            (
                lm_trans_frame_looks_named_structure_declaration(&node->as.frame) ||
                lm_trans_text_equals(node->as.frame.head, "L1")
            )
        ) {
            if (lm_trans_text_equals(node->as.frame.head, "L1")) {
                status = lm_trans_emit_l1_frame(file, &node->as.frame);
            } else {
                status = lm_trans_emit_named_structure(file, &node->as.frame);
            }
            if (status != 0) {
                lm_trans_namespace_delete(namespace_);
                return 1;
            }
        }
        field = field->next;
    }

    field = l2->body.first_field;
    while (field != NULL) {
        node = field->value;
        if (node != NULL && node->kind == LM_P0_NODE_FRAME) {
            function_info = lm_trans_top_level_function_frame_info(
                &node->as.frame,
                &function_frame,
                &kind,
                &is_external
            );
            if (function_info < 0) {
                lm_trans_namespace_delete(namespace_);
                return 1;
            }
            if (function_info == 0) {
                field = field->next;
                continue;
            }

            if (
                lm_trans_emit_function(
                    file,
                    function_frame,
                    kind == LM_TRANS_SYMBOL_PROCEDURE,
                    is_external,
                    namespace_
                ) != 0
            ) {
                lm_trans_namespace_delete(namespace_);
                return 1;
            }
            if (is_external && lm_trans_validate_end_trailer(&node->as.frame) != 0) {
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

    if (out_payload == NULL || out_run == NULL || text.length < 2U) {
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

    if (structure == NULL) {
        return 0;
    }

    field = structure->first_field;
    while (field != NULL) {
        if (lm_trans_emit_l1_node(output, field->value) != 0) {
            return 1;
        }
        field = field->next;
    }

    return 0;
}

static int lm_trans_emit_l1_node(FILE *output, const LmP0Node *node) {
    if (node == NULL || (node->flags & LM_P0_NODE_INACTIVE)) {
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
    if (l1 == NULL) {
        return 1;
    }

    if (lm_trans_emit_l1_structure(output, &l1->body) != 0) {
        return 1;
    }

    return lm_trans_validate_end_trailer(l1);
}

static int lm_trans_is_end_target(const LmP0Frame *frame, const char *target) {
    const LmP0Field *field;

    if (frame == NULL || !lm_trans_text_equals(frame->head, "end")) {
        return 0;
    }

    field = frame->body.first_field;
    return
        field != NULL &&
        field->next == NULL &&
        field->value != NULL &&
        field->value->kind == LM_P0_NODE_ATOM &&
        lm_trans_text_equals(field->value->as.atom, target);
}

static int lm_trans_emit_l1_body(FILE *output, const LmP0Frame *l1, int *emitted) {
    if (lm_trans_emit_l1_frame(output, l1) != 0) {
        return 1;
    }

    if (emitted != NULL) {
        *emitted = 1;
    }
    return 0;
}

static int lm_trans_emit_root_sequence(FILE *output, const LmP0Node *root, int *emitted) {
    const LmP0Field *field;
    const LmP0Node *node;

    if (root == NULL || root->kind != LM_P0_NODE_STRUCTURE) {
        return 1;
    }

    field = root->as.structure.first_field;
    while (field != NULL) {
        node = field->value;
        if (node != NULL && !(node->flags & LM_P0_NODE_INACTIVE)) {
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

static int lm_trans_emit_document(const char *source_path, const char *output_path) {
    LmP0Document *document;
    const LmP0Diagnostic *diagnostic;
    const LmP0Node *root;
    FILE *output;
    int status;
    int close_status;
    int emitted;

    document = NULL;
    status = lm_p0_parse_file(source_path, &document);
    if (status != 0) {
        diagnostic = lm_p0_document_diagnostic(document);
        if (diagnostic != NULL) {
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
        return 1;
    }

    output = fopen(output_path, "wb");
    if (output == NULL) {
        fprintf(stderr, "trans error: cannot open output file %s\n", output_path);
        lm_p0_document_destroy(document);
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
        return 1;
    }

    if (close_status != 0) {
        fprintf(stderr, "trans error: cannot write output file %s\n", output_path);
        lm_p0_document_destroy(document);
        return 1;
    }

    lm_p0_document_destroy(document);
    return 0;
}

int main(int argc, char **argv) {
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    if (argc != 3) {
        fprintf(stderr, "usage: trans.lm0[.exe] <source.lm2> <output.lm1.c>\n");
        return 1;
    }

    return lm_trans_emit_document(argv[1], argv[2]);
}

