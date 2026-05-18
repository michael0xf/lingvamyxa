#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.lm1.h"

typedef struct LmTransName {
    LmP0Text text;
} LmTransName;

typedef struct LmTransNameSet {
    LmTransName *items;
    size_t count;
    size_t capacity;
} LmTransNameSet;

static int lm_trans_text_equals(LmP0Text text, const char *value) {
    size_t length;

    length = strlen(value);
    return text.length == length && memcmp(text.data, value, length) == 0;
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

static void lm_trans_names_destroy(LmTransNameSet *set) {
    if (set != NULL) {
        free(set->items);
        set->items = NULL;
        set->count = 0U;
        set->capacity = 0U;
    }
}

static int lm_trans_names_contains(const LmTransNameSet *set, LmP0Text name) {
    size_t i;

    if (set == NULL) {
        return 0;
    }

    for (i = 0U; i < set->count; ++i) {
        if (
            set->items[i].text.length == name.length &&
            memcmp(set->items[i].text.data, name.data, name.length) == 0
        ) {
            return 1;
        }
    }

    return 0;
}

static int lm_trans_names_add(LmTransNameSet *set, LmP0Text name) {
    LmTransName *items;
    size_t capacity;

    if (lm_trans_names_contains(set, name)) {
        return 0;
    }

    if (set->count == set->capacity) {
        capacity = set->capacity == 0U ? 16U : set->capacity * 2U;
        items = (LmTransName *)realloc(set->items, capacity * sizeof(*items));
        if (items == NULL) {
            return 1;
        }
        set->items = items;
        set->capacity = capacity;
    }

    set->items[set->count].text = name;
    ++set->count;
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

static int lm_trans_emit_expr_node(FILE *file, const LmP0Node *node);
static int lm_trans_emit_expr_fields(FILE *file, const LmP0Field *first);
static int lm_trans_emit_expr_range(FILE *file, const LmP0Field *first, const LmP0Field *stop);

static int lm_trans_atom_starts_string(LmP0Text text) {
    return text.length > 0U && (text.data[0] == '"' || text.data[0] == '\'');
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

    if (first == NULL) {
        return NULL;
    }

    field = first;
    previous_operand = NULL;

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
        field = field->next;
    } else {
        previous_operand = node;
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
        } else if (lm_trans_atom_is_infix_expr_operator(node->as.atom, node, previous_operand)) {
            operand = field->next;
            if (operand == NULL) {
                return field;
            }
            previous_operand = operand->value;
            field = operand->next;
        } else {
            break;
        }
    }

    return field;
}

static int lm_trans_emit_call_args(FILE *file, const LmP0Structure *body) {
    const LmP0Field *field;
    const LmP0Field *next;
    int first;

    first = 1;
    field = body->first_field;
    while (field != NULL) {
        if (!first && lm_trans_put(file, ", ") != 0) {
            return 1;
        }
        next = lm_trans_expr_segment_end(field);
        if (lm_trans_emit_expr_range(file, field, next) != 0) {
            return 1;
        }
        first = 0;
        field = next;
    }

    return 0;
}

static int lm_trans_emit_expr_frame(FILE *file, const LmP0Frame *frame) {
    if (lm_trans_emit_name(file, frame->head) != 0) {
        return 1;
    }
    if (lm_trans_put(file, "(") != 0) {
        return 1;
    }
    if (lm_trans_emit_call_args(file, &frame->body) != 0) {
        return 1;
    }
    return lm_trans_put(file, ")");
}

static int lm_trans_emit_expr_node(FILE *file, const LmP0Node *node) {
    if (node == NULL) {
        return 0;
    }

    if (node->kind == LM_P0_NODE_ATOM) {
        return lm_trans_emit_name(file, node->as.atom);
    }

    if (node->kind == LM_P0_NODE_FRAME) {
        return lm_trans_emit_expr_frame(file, &node->as.frame);
    }

    if (node->kind == LM_P0_NODE_STRUCTURE) {
        if (lm_trans_put(file, "(") != 0) {
            return 1;
        }
        if (lm_trans_emit_expr_fields(file, node->as.structure.first_field) != 0) {
            return 1;
        }
        return lm_trans_put(file, ")");
    }

    return 0;
}

static int lm_trans_atom_is_operand_like(LmP0Text text) {
    return
        !lm_trans_text_is_operator_atom(text) &&
        !lm_trans_text_equals(text, "@") &&
        !lm_trans_text_equals(text, "\\");
}

static int lm_trans_emit_expr_fields(FILE *file, const LmP0Field *first) {
    return lm_trans_emit_expr_range(file, first, NULL);
}

static int lm_trans_emit_expr_range(FILE *file, const LmP0Field *first, const LmP0Field *stop) {
    const LmP0Field *field;
    const LmP0Node *node;
    int wrote;
    const LmP0Node *previous_operand;

    wrote = 0;
    previous_operand = NULL;
    field = first;
    while (field != stop) {
        node = field->value;
        if (node != NULL && !(node->flags & LM_P0_NODE_INACTIVE)) {
            if (wrote && lm_trans_put(file, " ") != 0) {
                return 1;
            }

            if (node->kind == LM_P0_NODE_ATOM) {
                if (lm_trans_text_equals(node->as.atom, "=")) {
                    if (lm_trans_put(file, "==") != 0) {
                        return 1;
                    }
                    previous_operand = NULL;
                } else if (lm_trans_text_equals(node->as.atom, "@")) {
                    if (lm_trans_put(file, "&") != 0) {
                        return 1;
                    }
                    previous_operand = NULL;
                } else if (lm_trans_text_equals(node->as.atom, "\\")) {
                    if (previous_operand != NULL && lm_trans_nodes_touch(previous_operand, node)) {
                        if (lm_trans_put(file, "->") != 0) {
                            return 1;
                        }
                        previous_operand = NULL;
                    } else {
                        if (lm_trans_put(file, "*") != 0) {
                            return 1;
                        }
                        previous_operand = NULL;
                    }
                } else {
                    if (lm_trans_emit_name(file, node->as.atom) != 0) {
                        return 1;
                    }
                    previous_operand = lm_trans_atom_is_operand_like(node->as.atom) ? node : NULL;
                }
            } else {
                if (lm_trans_emit_expr_node(file, node) != 0) {
                    return 1;
                }
                previous_operand = node;
            }
            wrote = 1;
        }
        field = field->next;
    }

    return 0;
}

static int lm_trans_emit_type_and_name(
    FILE *file,
    const LmP0Node *type_node,
    LmP0Text name,
    size_t pointer_depth
) {
    size_t i;

    if (type_node == NULL) {
        return 1;
    }

    if (lm_trans_emit_expr_node(file, type_node) != 0) {
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

static int lm_trans_emit_param(FILE *file, const LmP0Node *node, LmTransNameSet *locals) {
    const LmP0Field *field0;
    const LmP0Field *field1;
    const LmP0Node *name_node;

    if (node == NULL || node->kind != LM_P0_NODE_FRAME) {
        fprintf(stderr, "trans L2 error: parameter must be a typed frame\n");
        return 1;
    }

    field0 = lm_trans_nth_field(&node->as.frame.body, 0U);
    field1 = lm_trans_nth_field(&node->as.frame.body, 1U);

    if (
        lm_trans_is_c_reference_name(node->as.frame.head) &&
        lm_trans_accepts_c_type_name(node->as.frame.head)
    ) {
        if (field0 == NULL || field0->value == NULL || field0->value->kind != LM_P0_NODE_ATOM) {
            fprintf(stderr, "trans L2 error: c.type parameter expects a name\n");
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
        return lm_trans_names_add(locals, name_node->as.atom);
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
                node->as.frame.head.length
            ) != 0
        ) {
            return 1;
        }
        return lm_trans_names_add(locals, name_node->as.atom);
    }

    fprintf(stderr, "trans L2 error: unsupported parameter receiver\n");
    return 1;
}

static int lm_trans_emit_statement_list(
    FILE *file,
    const LmP0Field *first,
    unsigned indent,
    LmTransNameSet *locals
);

static const LmP0Field *lm_trans_control_body_start(const LmP0Frame *frame) {
    const LmP0Field *field;

    field = frame->body.first_field;
    if (field == NULL) {
        return NULL;
    }

    if (field->value != NULL && field->value->kind == LM_P0_NODE_STRUCTURE) {
        return field->next;
    }

    while (
        field != NULL &&
        field->value != NULL &&
        field->value->kind == LM_P0_NODE_ATOM
    ) {
        field = field->next;
    }

    return field;
}

static int lm_trans_emit_control_condition(FILE *file, const LmP0Frame *frame) {
    const LmP0Field *body_start;
    const LmP0Field *first;

    first = frame->body.first_field;
    if (first == NULL) {
        return 0;
    }

    if (first->value != NULL && first->value->kind == LM_P0_NODE_STRUCTURE) {
        return lm_trans_emit_expr_fields(file, first->value->as.structure.first_field);
    }

    body_start = lm_trans_control_body_start(frame);
    return lm_trans_emit_expr_range(file, first, body_start);
}

static int lm_trans_emit_trailer_statement(
    FILE *file,
    const LmP0Trailer *trailer,
    unsigned indent,
    LmTransNameSet *locals
) {
    (void)locals;

    if (trailer == NULL) {
        return 0;
    }

    if (lm_trans_text_equals(trailer->spelling, "return")) {
        if (lm_trans_emit_indent(file, indent) != 0) {
            return 1;
        }
        if (lm_trans_put(file, "return ") != 0) {
            return 1;
        }
        if (lm_trans_emit_expr_fields(file, trailer->body.first_field) != 0) {
            return 1;
        }
        return lm_trans_put(file, ";\n");
    }

    return 0;
}

static int lm_trans_emit_control(
    FILE *file,
    const LmP0Frame *frame,
    unsigned indent,
    LmTransNameSet *locals,
    const char *keyword
) {
    const LmP0Field *body_start;

    body_start = lm_trans_control_body_start(frame);
    if (lm_trans_emit_indent(file, indent) != 0) {
        return 1;
    }
    if (lm_trans_put(file, keyword) != 0 || lm_trans_put(file, " (") != 0) {
        return 1;
    }
    if (lm_trans_emit_control_condition(file, frame) != 0) {
        return 1;
    }
    if (lm_trans_put(file, ") {\n") != 0) {
        return 1;
    }
    if (lm_trans_emit_statement_list(file, body_start, indent + 1U, locals) != 0) {
        return 1;
    }
    if (lm_trans_emit_indent(file, indent) != 0) {
        return 1;
    }
    return lm_trans_put(file, "}\n");
}

static int lm_trans_emit_else(
    FILE *file,
    const LmP0Frame *frame,
    unsigned indent,
    LmTransNameSet *locals
) {
    if (lm_trans_emit_indent(file, indent) != 0) {
        return 1;
    }
    if (lm_trans_put(file, "else {\n") != 0) {
        return 1;
    }
    if (lm_trans_emit_statement_list(file, frame->body.first_field, indent + 1U, locals) != 0) {
        return 1;
    }
    if (lm_trans_emit_indent(file, indent) != 0) {
        return 1;
    }
    return lm_trans_put(file, "}\n");
}

static int lm_trans_emit_declaration(
    FILE *file,
    const LmP0Frame *frame,
    unsigned indent,
    LmTransNameSet *locals
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
        if (lm_trans_emit_expr_fields(file, name_field->next) != 0) {
            return 1;
        }
    }
    if (lm_trans_put(file, ";\n") != 0) {
        return 1;
    }
    return lm_trans_names_add(locals, name_node->as.atom);
}

static int lm_trans_atom_can_be_new_binding_name(LmP0Text text) {
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

static int lm_trans_frame_looks_c_declaration(
    const LmP0Frame *frame,
    const LmTransNameSet *locals
) {
    const LmP0Field *name_field;

    if (
        frame == NULL ||
        !lm_trans_is_c_reference_name(frame->head) ||
        !lm_trans_accepts_c_type_name(frame->head)
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

    return
        lm_trans_atom_can_be_new_binding_name(name_field->value->as.atom) &&
        !lm_trans_names_contains(locals, name_field->value->as.atom);
}

static int lm_trans_emit_pointer_declaration(
    FILE *file,
    const LmP0Frame *frame,
    unsigned indent,
    LmTransNameSet *locals
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
    if (
        lm_trans_emit_type_and_name(
            file,
            type_field->value,
            name_node->as.atom,
            frame->head.length
        ) != 0
    ) {
        return 1;
    }
    if (name_field->next != NULL) {
        if (lm_trans_put(file, " = ") != 0) {
            return 1;
        }
        if (lm_trans_emit_expr_fields(file, name_field->next) != 0) {
            return 1;
        }
    }
    if (lm_trans_put(file, ";\n") != 0) {
        return 1;
    }
    return lm_trans_names_add(locals, name_node->as.atom);
}

static int lm_trans_emit_call_statement(
    FILE *file,
    const LmP0Frame *frame,
    unsigned indent
) {
    if (lm_trans_emit_indent(file, indent) != 0) {
        return 1;
    }
    if (lm_trans_emit_expr_frame(file, frame) != 0) {
        return 1;
    }
    return lm_trans_put(file, ";\n");
}

static int lm_trans_emit_assignment(
    FILE *file,
    const LmP0Frame *frame,
    unsigned indent
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
    if (lm_trans_emit_expr_fields(file, frame->body.first_field) != 0) {
        return 1;
    }
    return lm_trans_put(file, ";\n");
}

static int lm_trans_emit_statement(
    FILE *file,
    const LmP0Node *node,
    unsigned indent,
    LmTransNameSet *locals
) {
    int status;

    if (node == NULL || (node->flags & LM_P0_NODE_INACTIVE)) {
        return 0;
    }

    if (node->kind == LM_P0_NODE_ATOM) {
        if (lm_trans_text_equals(node->as.atom, "break") || lm_trans_text_equals(node->as.atom, "continue")) {
            if (lm_trans_emit_indent(file, indent) != 0) {
                return 1;
            }
            if (lm_trans_write_text(file, node->as.atom) != 0) {
                return 1;
            }
            return lm_trans_put(file, ";\n");
        }

        if (lm_trans_emit_indent(file, indent) != 0) {
            return 1;
        }
        if (lm_trans_emit_expr_node(file, node) != 0) {
            return 1;
        }
        return lm_trans_put(file, ";\n");
    }

    if (node->kind != LM_P0_NODE_FRAME) {
        return 0;
    }

    status = 0;
    if (lm_trans_text_equals(node->as.frame.head, "return")) {
        if (lm_trans_emit_indent(file, indent) != 0) {
            return 1;
        }
        if (lm_trans_put(file, "return ") != 0) {
            return 1;
        }
        status = lm_trans_emit_expr_fields(file, node->as.frame.body.first_field);
        if (status == 0) {
            status = lm_trans_put(file, ";\n");
        }
    } else if (lm_trans_text_equals(node->as.frame.head, "if")) {
        status = lm_trans_emit_control(file, &node->as.frame, indent, locals, "if");
    } else if (lm_trans_text_equals(node->as.frame.head, "while")) {
        status = lm_trans_emit_control(file, &node->as.frame, indent, locals, "while");
    } else if (lm_trans_text_equals(node->as.frame.head, "else")) {
        status = lm_trans_emit_else(file, &node->as.frame, indent, locals);
    } else if (lm_trans_text_equals(node->as.frame.head, "break")) {
        if (lm_trans_emit_indent(file, indent) != 0) {
            return 1;
        }
        status = lm_trans_put(file, "break;\n");
    } else if (lm_trans_text_equals(node->as.frame.head, "continue")) {
        if (lm_trans_emit_indent(file, indent) != 0) {
            return 1;
        }
        status = lm_trans_put(file, "continue;\n");
    } else if (lm_trans_frame_looks_c_declaration(&node->as.frame, locals)) {
        status = lm_trans_emit_declaration(file, &node->as.frame, indent, locals);
    } else if (lm_trans_text_all_char(node->as.frame.head, '@')) {
        status = lm_trans_emit_pointer_declaration(file, &node->as.frame, indent, locals);
    } else if (lm_trans_names_contains(locals, node->as.frame.head)) {
        status = lm_trans_emit_assignment(file, &node->as.frame, indent);
    } else {
        status = lm_trans_emit_call_statement(file, &node->as.frame, indent);
    }

    if (status != 0) {
        return status;
    }

    return lm_trans_emit_trailer_statement(file, node->as.frame.trailer, indent, locals);
}

static int lm_trans_emit_statement_list(
    FILE *file,
    const LmP0Field *first,
    unsigned indent,
    LmTransNameSet *locals
) {
    const LmP0Field *field;

    field = first;
    while (field != NULL) {
        if (lm_trans_emit_statement(file, field->value, indent, locals) != 0) {
            return 1;
        }
        field = field->next;
    }

    return 0;
}

static int lm_trans_emit_params(
    FILE *file,
    const LmP0Node *params,
    LmTransNameSet *locals
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
        if (lm_trans_emit_param(file, field->value, locals) != 0) {
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

static int lm_trans_emit_function(FILE *file, const LmP0Frame *frame, int is_sub) {
    const LmP0Field *name_field;
    const LmP0Field *params_field;
    const LmP0Field *return_field;
    const LmP0Field *body_start;
    LmTransNameSet locals;
    int status;

    memset(&locals, 0, sizeof(locals));
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

    if (is_sub) {
        if (lm_trans_put(file, "void ") != 0) {
            return 1;
        }
        body_start = params_field->next;
    } else {
        return_field = lm_trans_nth_field(&frame->body, 2U);
        if (return_field == NULL || return_field->value == NULL) {
            fprintf(stderr, "trans L2 error: fn expects return type\n");
            return 1;
        }
        if (lm_trans_emit_expr_node(file, return_field->value) != 0) {
            return 1;
        }
        if (lm_trans_put(file, " ") != 0) {
            return 1;
        }
        body_start = return_field->next;
    }

    if (lm_trans_write_text(file, name_field->value->as.atom) != 0) {
        return 1;
    }
    if (lm_trans_put(file, "(") != 0) {
        return 1;
    }
    if (lm_trans_emit_params(file, params_field->value, &locals) != 0) {
        lm_trans_names_destroy(&locals);
        return 1;
    }
    if (lm_trans_put(file, ") {\n") != 0) {
        lm_trans_names_destroy(&locals);
        return 1;
    }

    status = lm_trans_emit_statement_list(file, body_start, 1U, &locals);
    if (status == 0) {
        status = lm_trans_emit_trailer_statement(file, frame->trailer, 1U, &locals);
    }
    if (status == 0) {
        status = lm_trans_put(file, "}\n\n");
    }

    lm_trans_names_destroy(&locals);
    return status;
}

static int lm_trans_emit_l2_frame(FILE *file, const LmP0Frame *l2) {
    const LmP0Field *field;
    const LmP0Node *node;

    field = l2->body.first_field;
    while (field != NULL) {
        node = field->value;
        if (node != NULL && node->kind == LM_P0_NODE_FRAME) {
            if (lm_trans_text_equals(node->as.frame.head, "fn")) {
                if (lm_trans_emit_function(file, &node->as.frame, 0) != 0) {
                    return 1;
                }
            } else if (lm_trans_text_equals(node->as.frame.head, "sub")) {
                if (lm_trans_emit_function(file, &node->as.frame, 1) != 0) {
                    return 1;
                }
            } else {
                fprintf(stderr, "trans L2 error: top-level L2 field must be fn or sub\n");
                return 1;
            }
        }
        field = field->next;
    }

    return 0;
}

static int lm_trans_emit_raw_atom(FILE *output, const LmP0Node *node) {
    if (node == NULL || node->kind != LM_P0_NODE_ATOM) {
        fprintf(stderr, "trans error: L1 raw field must be atom text\n");
        return 1;
    }

    if (lm_trans_write_all(output, node->as.atom.data, node->as.atom.length) != 0) {
        return 1;
    }
    return lm_trans_write_all(output, "\n", 1U);
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
    const LmP0Field *field;
    const LmP0Node *node;

    field = l1->body.first_field;
    while (field != NULL) {
        node = field->value;
        if (node != NULL && !(node->flags & LM_P0_NODE_INACTIVE)) {
            if (node->kind == LM_P0_NODE_ATOM) {
                if (lm_trans_emit_raw_atom(output, node) != 0) {
                    return 1;
                }
                *emitted = 1;
            } else if (
                node->kind == LM_P0_NODE_FRAME &&
                lm_trans_text_equals(node->as.frame.head, "L2")
            ) {
                if (lm_trans_emit_l2_frame(output, &node->as.frame) != 0) {
                    return 1;
                }
                *emitted = 1;
            } else if (
                node->kind == LM_P0_NODE_FRAME &&
                lm_trans_is_end_target(&node->as.frame, "L1")
            ) {
                return 0;
            } else {
                fprintf(stderr, "trans error: L1 body may contain raw text or explicit L2 frames\n");
                return 1;
            }
        }
        field = field->next;
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

    if (fclose(output) != 0) {
        status = 1;
    }

    if (status != 0) {
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

