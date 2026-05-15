#include "p0.lm1.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum LmP0LineKind {
    LM_P0_LINE_ITEM = 1,
    LM_P0_LINE_DELIM = 2
} LmP0LineKind;

typedef struct LmP0LineEvent {
    LmP0LineKind kind;
    size_t level;
    const char *text;
    size_t text_length;
    size_t line;
    size_t column;
    size_t offset;
} LmP0LineEvent;

struct LmP0Document {
    char *source;
    size_t source_length;
    LmP0Node *root;
    LmP0Diagnostic diagnostic;
};

typedef struct LmP0EventList {
    LmP0LineEvent *items;
    size_t count;
    size_t capacity;
} LmP0EventList;

typedef struct LmP0Stack {
    LmP0Structure **parents;
    LmP0Node **owners;
    unsigned char *hard;
    size_t capacity;
} LmP0Stack;

typedef struct LmP0Dump {
    char *data;
    size_t length;
    size_t capacity;
    int failed;
} LmP0Dump;

typedef enum LmP0TrailerRole {
    LM_P0_TRAILER_ROLE_NONE = 0,
    LM_P0_TRAILER_ROLE_TAIL_CUTTER = 1
} LmP0TrailerRole;

static void lm_p0_set_diagnostic(
    LmP0Document *document,
    int code,
    size_t line,
    size_t column,
    const char *format,
    ...
) {
    va_list args;

    if (document->diagnostic.code != 0) {
        return;
    }

    document->diagnostic.code = code;
    document->diagnostic.line = line;
    document->diagnostic.column = column;

    va_start(args, format);
    vsnprintf(document->diagnostic.message, sizeof(document->diagnostic.message), format, args);
    va_end(args);
}

static char *lm_p0_copy_bytes(const char *source, size_t length) {
    char *copy;

    copy = (char *)malloc(length + 1U);
    if (copy == NULL) {
        return NULL;
    }

    if (length > 0U) {
        memcpy(copy, source, length);
    }
    copy[length] = '\0';
    return copy;
}

static void lm_p0_trim_right(const char **text, size_t *length) {
    (void)text;
    while (*length > 0U && ((*text)[*length - 1U] == ' ' || (*text)[*length - 1U] == '\r')) {
        --(*length);
    }
}

static int lm_p0_event_list_push(
    LmP0Document *document,
    LmP0EventList *list,
    const LmP0LineEvent *event
) {
    LmP0LineEvent *items;
    size_t new_capacity;

    if (list->count == list->capacity) {
        new_capacity = list->capacity == 0U ? 32U : list->capacity * 2U;
        items = (LmP0LineEvent *)realloc(list->items, new_capacity * sizeof(*items));
        if (items == NULL) {
            lm_p0_set_diagnostic(document, 1, event->line, event->column, "out of memory while storing source events");
            return 0;
        }
        list->items = items;
        list->capacity = new_capacity;
    }

    list->items[list->count++] = *event;
    return 1;
}

static int lm_p0_normalize_lines(LmP0Document *document, LmP0EventList *events) {
    const char *source;
    size_t length;
    size_t offset;
    size_t line;

    source = document->source;
    length = document->source_length;
    offset = 0U;
    line = 1U;

    while (offset <= length) {
        size_t line_start;
        size_t line_end;
        size_t raw_length;
        size_t p;
        size_t level;
        const char *text;
        size_t text_length;
        LmP0LineEvent event;

        line_start = offset;
        line_end = offset;
        while (line_end < length && source[line_end] != '\n') {
            ++line_end;
        }

        raw_length = line_end - line_start;
        if (raw_length > 0U && source[line_start + raw_length - 1U] == '\r') {
            --raw_length;
        }

        if (raw_length == 0U) {
            if (line_end == length) {
                break;
            }
            offset = line_end + 1U;
            ++line;
            continue;
        }

        for (p = line_start; p < line_start + raw_length; ++p) {
            if (source[p] == '\t') {
                lm_p0_set_diagnostic(document, 2, line, p - line_start + 1U, "tabs are not accepted by the first dotted P0 parser");
                return 0;
            }
        }

        p = line_start;
        level = 0U;

        if (line == 1U && raw_length >= 3U &&
            (unsigned char)source[p] == 0xEFU &&
            (unsigned char)source[p + 1U] == 0xBBU &&
            (unsigned char)source[p + 2U] == 0xBFU) {
            p += 3U;
        }

        if (source[p] == ' ') {
            lm_p0_set_diagnostic(document, 3, line, 1U, "leading spaces are not accepted by the first dotted P0 parser");
            return 0;
        }

        while (p < line_start + raw_length && source[p] == '.') {
            ++level;
            ++p;
            while (p < line_start + raw_length && source[p] == ' ') {
                ++p;
            }
        }

        text = source + p;
        text_length = (line_start + raw_length) - p;
        lm_p0_trim_right(&text, &text_length);

        if (text_length == 0U && level == 0U) {
            if (line_end == length) {
                break;
            }
            offset = line_end + 1U;
            ++line;
            continue;
        }

        if (text_length > 0U && text[0] == '#') {
            if (line_end == length) {
                break;
            }
            offset = line_end + 1U;
            ++line;
            continue;
        }

        memset(&event, 0, sizeof(event));
        event.level = level;
        event.text = text;
        event.text_length = text_length;
        event.line = line;
        event.column = (size_t)(text - (source + line_start)) + 1U;
        event.offset = (size_t)(text - source);

        if (text_length == 0U && level > 0U) {
            event.kind = LM_P0_LINE_DELIM;
        } else {
            event.kind = LM_P0_LINE_ITEM;
        }

        if (!lm_p0_event_list_push(document, events, &event)) {
            return 0;
        }

        if (line_end == length) {
            break;
        }
        offset = line_end + 1U;
        ++line;
    }

    return 1;
}

static LmP0Node *lm_p0_new_node(LmP0Document *document, LmP0NodeKind kind) {
    LmP0Node *node;

    node = (LmP0Node *)calloc(1U, sizeof(*node));
    if (node == NULL) {
        lm_p0_set_diagnostic(document, 1, 0U, 0U, "out of memory while allocating parser node");
        return NULL;
    }

    node->kind = kind;
    return node;
}

static int lm_p0_append_field(LmP0Document *document, LmP0Structure *structure, LmP0Node *node) {
    LmP0Field *field;

    field = (LmP0Field *)calloc(1U, sizeof(*field));
    if (field == NULL) {
        lm_p0_set_diagnostic(document, 1, node != NULL ? node->span.line : 0U, node != NULL ? node->span.column : 0U, "out of memory while allocating parser field");
        return 0;
    }

    field->value = node;
    if (structure->last_field == NULL) {
        structure->first_field = field;
    } else {
        structure->last_field->next = field;
    }
    structure->last_field = field;
    ++structure->field_count;
    return 1;
}

static void lm_p0_free_node(LmP0Node *node);
static void lm_p0_free_structure_fields(LmP0Structure *structure);

static void lm_p0_free_trailer(LmP0Trailer *trailer) {
    if (trailer == NULL) {
        return;
    }
    lm_p0_free_structure_fields(&trailer->body);
    free(trailer);
}

static void lm_p0_free_structure_fields(LmP0Structure *structure) {
    LmP0Field *field;

    field = structure->first_field;
    while (field != NULL) {
        LmP0Field *next;

        next = field->next;
        lm_p0_free_node(field->value);
        free(field);
        field = next;
    }
    structure->first_field = NULL;
    structure->last_field = NULL;
    structure->field_count = 0U;
    lm_p0_free_trailer(structure->trailer);
    structure->trailer = NULL;
}

static void lm_p0_free_node(LmP0Node *node) {
    if (node == NULL) {
        return;
    }

    if (node->kind == LM_P0_NODE_STRUCTURE) {
        lm_p0_free_structure_fields(&node->as.structure);
    } else if (node->kind == LM_P0_NODE_FRAME) {
        lm_p0_free_structure_fields(&node->as.frame.body);
        lm_p0_free_trailer(node->as.frame.trailer);
        node->as.frame.trailer = NULL;
    }

    free(node);
}

static void lm_p0_skip_spaces(const char *text, size_t length, size_t *index) {
    while (*index < length && text[*index] == ' ') {
        ++(*index);
    }
}

static int lm_p0_is_operator_char(char c) {
    return c == '+' || c == '-' || c == '*' || c == '/' || c == '%' ||
           c == '=' || c == '<' || c == '>' || c == '!' || c == '&' ||
           c == '|';
}

static int lm_p0_text_has_expression_operator(const char *text, size_t length) {
    size_t i;

    for (i = 0U; i < length; ++i) {
        if (text[i] == '"') {
            ++i;
            while (i < length) {
                if (text[i] == '\\') {
                    i += 2U;
                    continue;
                }
                if (text[i] == '"') {
                    break;
                }
                ++i;
            }
            continue;
        }
        if (text[i] == '`') {
            ++i;
            while (i < length) {
                if (text[i] == '`' && i + 1U < length && text[i + 1U] == '`') {
                    i += 2U;
                    continue;
                }
                if (text[i] == '`') {
                    break;
                }
                ++i;
            }
            continue;
        }
        if (lm_p0_is_operator_char(text[i])) {
            return 1;
        }
    }
    return 0;
}

static int lm_p0_scan_quoted(
    LmP0Document *document,
    const char *text,
    size_t length,
    size_t *index,
    char quote,
    size_t line,
    size_t base_column
) {
    size_t i;

    i = *index + 1U;
    while (i < length) {
        if (quote == '"' && text[i] == '\\') {
            i += 2U;
            continue;
        }
        if (quote == '`' && text[i] == '`' && i + 1U < length && text[i + 1U] == '`') {
            i += 2U;
            continue;
        }
        if (text[i] == quote) {
            *index = i + 1U;
            return 1;
        }
        ++i;
    }

    lm_p0_set_diagnostic(document, 4, line, base_column + *index, "unterminated quoted token");
    return 0;
}

static int lm_p0_find_matching_paren(
    LmP0Document *document,
    const char *text,
    size_t length,
    size_t open_index,
    size_t line,
    size_t base_column,
    size_t *close_index
) {
    size_t i;
    size_t depth;

    i = open_index;
    depth = 0U;
    while (i < length) {
        if (text[i] == '"' || text[i] == '`') {
            if (!lm_p0_scan_quoted(document, text, length, &i, text[i], line, base_column)) {
                return 0;
            }
            continue;
        }
        if (text[i] == '(') {
            ++depth;
        } else if (text[i] == ')') {
            --depth;
            if (depth == 0U) {
                *close_index = i;
                return 1;
            }
        }
        ++i;
    }

    lm_p0_set_diagnostic(document, 5, line, base_column + open_index, "unclosed parenthesized form");
    return 0;
}

static int lm_p0_find_colon(const char *text, size_t length, size_t *colon_index) {
    size_t i;
    size_t depth;

    depth = 0U;
    i = 0U;
    while (i < length) {
        if (text[i] == '"' || text[i] == '`') {
            char quote;

            quote = text[i++];
            while (i < length) {
                if (quote == '"' && text[i] == '\\') {
                    i += 2U;
                    continue;
                }
                if (quote == '`' && text[i] == '`' && i + 1U < length && text[i + 1U] == '`') {
                    i += 2U;
                    continue;
                }
                if (text[i++] == quote) {
                    break;
                }
            }
            continue;
        }
        if (text[i] == '(' || text[i] == '[') {
            ++depth;
        } else if ((text[i] == ')' || text[i] == ']') && depth > 0U) {
            --depth;
        } else if (text[i] == ':' && depth == 0U) {
            *colon_index = i;
            return 1;
        }
        ++i;
    }

    return 0;
}

static LmP0Node *lm_p0_parse_line_item(
    LmP0Document *document,
    const char *text,
    size_t length,
    size_t line,
    size_t column,
    size_t offset
);

static int lm_p0_parse_fields_into(
    LmP0Document *document,
    LmP0Structure *structure,
    const char *text,
    size_t length,
    size_t line,
    size_t column,
    size_t offset
) {
    size_t i;

    i = 0U;
    while (i < length) {
        size_t start;
        size_t close_index;
        LmP0Node *node;

        lm_p0_skip_spaces(text, length, &i);
        if (i >= length) {
            break;
        }
        if (text[i] == ',') {
            ++i;
            continue;
        }

        start = i;
        node = NULL;

        if (text[i] == '(') {
            if (!lm_p0_find_matching_paren(document, text, length, i, line, column, &close_index)) {
                return 0;
            }

            if (lm_p0_text_has_expression_operator(text + i + 1U, close_index - i - 1U)) {
                node = lm_p0_new_node(document, LM_P0_NODE_EXPR);
                if (node == NULL) {
                    return 0;
                }
                node->as.expr.data = text + i;
                node->as.expr.length = close_index - i + 1U;
            } else {
                node = lm_p0_new_node(document, LM_P0_NODE_STRUCTURE);
                if (node == NULL) {
                    return 0;
                }
                if (!lm_p0_parse_fields_into(
                        document,
                        &node->as.structure,
                        text + i + 1U,
                        close_index - i - 1U,
                        line,
                        column + i + 1U,
                        offset + i + 1U
                    )) {
                    return 0;
                }
            }
            node->span.line = line;
            node->span.column = column + i;
            node->span.offset = offset + i;
            node->span.length = close_index - i + 1U;
            i = close_index + 1U;
        } else {
            size_t head_end;

            while (i < length && text[i] != ' ' && text[i] != ',' && text[i] != '(') {
                if (text[i] == '"' || text[i] == '`') {
                    if (!lm_p0_scan_quoted(document, text, length, &i, text[i], line, column)) {
                        return 0;
                    }
                    continue;
                }
                ++i;
            }
            head_end = i;

            if (i < length && text[i] == '(' && head_end > start) {
                node = lm_p0_new_node(document, LM_P0_NODE_FRAME);
                if (node == NULL) {
                    return 0;
                }
                node->as.frame.head.data = text + start;
                node->as.frame.head.length = head_end - start;
                node->as.frame.flags = LM_P0_FRAME_COMPACT;
                if (!lm_p0_find_matching_paren(document, text, length, i, line, column, &close_index)) {
                    return 0;
                }
                if (!lm_p0_parse_fields_into(
                        document,
                        &node->as.frame.body,
                        text + i + 1U,
                        close_index - i - 1U,
                        line,
                        column + i + 1U,
                        offset + i + 1U
                    )) {
                    return 0;
                }
                node->span.line = line;
                node->span.column = column + start;
                node->span.offset = offset + start;
                node->span.length = close_index - start + 1U;
                i = close_index + 1U;
            } else {
                if (head_end == start) {
                    lm_p0_set_diagnostic(document, 6, line, column + start, "unexpected character in field list");
                    return 0;
                }
                node = lm_p0_new_node(document, lm_p0_text_has_expression_operator(text + start, head_end - start) ? LM_P0_NODE_EXPR : LM_P0_NODE_ATOM);
                if (node == NULL) {
                    return 0;
                }
                if (node->kind == LM_P0_NODE_EXPR) {
                    node->as.expr.data = text + start;
                    node->as.expr.length = head_end - start;
                } else {
                    node->as.atom.data = text + start;
                    node->as.atom.length = head_end - start;
                }
                node->span.line = line;
                node->span.column = column + start;
                node->span.offset = offset + start;
                node->span.length = head_end - start;
            }
        }

        if (!lm_p0_append_field(document, structure, node)) {
            return 0;
        }
    }

    return 1;
}

static LmP0Node *lm_p0_parse_line_item(
    LmP0Document *document,
    const char *text,
    size_t length,
    size_t line,
    size_t column,
    size_t offset
) {
    size_t colon_index;
    LmP0Node *node;

    if (lm_p0_find_colon(text, length, &colon_index)) {
        size_t head_length;
        size_t body_start;

        head_length = colon_index;
        while (head_length > 0U && text[head_length - 1U] == ' ') {
            --head_length;
        }
        if (head_length == 0U) {
            lm_p0_set_diagnostic(document, 7, line, column, "frame head is empty");
            return NULL;
        }

        node = lm_p0_new_node(document, LM_P0_NODE_FRAME);
        if (node == NULL) {
            return NULL;
        }
        node->span.line = line;
        node->span.column = column;
        node->span.offset = offset;
        node->span.length = length;
        node->as.frame.head.data = text;
        node->as.frame.head.length = head_length;
        node->as.frame.flags = LM_P0_FRAME_COLON;

        body_start = colon_index + 1U;
        while (body_start < length && text[body_start] == ' ') {
            ++body_start;
        }
        if (!lm_p0_parse_fields_into(
                document,
                &node->as.frame.body,
                text + body_start,
                length - body_start,
                line,
                column + body_start,
                offset + body_start
            )) {
            lm_p0_free_node(node);
            return NULL;
        }
        return node;
    }

    node = lm_p0_new_node(document, LM_P0_NODE_STRUCTURE);
    if (node == NULL) {
        return NULL;
    }
    node->span.line = line;
    node->span.column = column;
    node->span.offset = offset;
    node->span.length = length;

    if (!lm_p0_parse_fields_into(document, &node->as.structure, text, length, line, column, offset)) {
        lm_p0_free_node(node);
        return NULL;
    }

    if (node->as.structure.field_count == 1U) {
        LmP0Field *field;
        LmP0Node *single;

        field = node->as.structure.first_field;
        single = field->value;
        free(field);
        free(node);
        return single;
    }

    return node;
}

static int lm_p0_stack_ensure(LmP0Document *document, LmP0Stack *stack, size_t level) {
    LmP0Structure **parents;
    LmP0Node **owners;
    unsigned char *hard;
    size_t old_capacity;
    size_t new_capacity;
    size_t i;

    if (level < stack->capacity) {
        return 1;
    }

    old_capacity = stack->capacity;
    new_capacity = old_capacity == 0U ? 8U : old_capacity;
    while (new_capacity <= level) {
        new_capacity *= 2U;
    }

    parents = (LmP0Structure **)realloc(stack->parents, new_capacity * sizeof(*parents));
    if (parents == NULL) {
        lm_p0_set_diagnostic(document, 1, 0U, 0U, "out of memory while growing parser stack");
        return 0;
    }
    owners = (LmP0Node **)realloc(stack->owners, new_capacity * sizeof(*owners));
    if (owners == NULL) {
        lm_p0_set_diagnostic(document, 1, 0U, 0U, "out of memory while growing parser stack");
        stack->parents = parents;
        return 0;
    }
    hard = (unsigned char *)realloc(stack->hard, new_capacity * sizeof(*hard));
    if (hard == NULL) {
        lm_p0_set_diagnostic(document, 1, 0U, 0U, "out of memory while growing parser stack");
        stack->parents = parents;
        stack->owners = owners;
        return 0;
    }

    stack->parents = parents;
    stack->owners = owners;
    stack->hard = hard;
    stack->capacity = new_capacity;
    for (i = old_capacity; i < new_capacity; ++i) {
        stack->parents[i] = NULL;
        stack->owners[i] = NULL;
        stack->hard[i] = 0U;
    }

    return 1;
}

static void lm_p0_stack_truncate_deeper(LmP0Stack *stack, size_t level) {
    size_t i;

    if (stack->capacity <= level + 1U) {
        return;
    }

    for (i = level + 1U; i < stack->capacity; ++i) {
        stack->parents[i] = NULL;
        stack->owners[i] = NULL;
        stack->hard[i] = 0U;
    }
}

static size_t lm_p0_stack_top_level(const LmP0Stack *stack) {
    size_t i;

    i = stack->capacity;
    while (i > 0U) {
        --i;
        if (stack->parents[i] != NULL) {
            return i;
        }
    }
    return 0U;
}

static size_t lm_p0_stack_collapse_soft_to_event(LmP0Stack *stack, size_t event_level) {
    size_t top_level;

    top_level = lm_p0_stack_top_level(stack);
    while (top_level > 0U && event_level < top_level && stack->hard[top_level] == 0U) {
        lm_p0_stack_truncate_deeper(stack, top_level - 1U);
        top_level = lm_p0_stack_top_level(stack);
    }
    return top_level;
}

static int lm_p0_text_has_prefix_name(
    const char *text,
    size_t length,
    const char *name,
    int allow_bare
) {
    size_t name_length;

    name_length = strlen(name);
    if (length < name_length) {
        return 0;
    }
    if (memcmp(text, name, name_length) != 0) {
        return 0;
    }
    if (length == name_length) {
        return allow_bare;
    }
    return text[name_length] == ':';
}

static LmP0TrailerRole lm_p0_trailer_role(const char *text, size_t length) {
    if (length >= 3U && memcmp(text, "---", 3U) == 0 &&
        (length == 3U || text[3U] == ' ' || text[3U] == ':')) {
        return LM_P0_TRAILER_ROLE_TAIL_CUTTER;
    }
    if (lm_p0_text_has_prefix_name(text, length, "end", 0)) {
        return LM_P0_TRAILER_ROLE_TAIL_CUTTER;
    }
    if (lm_p0_text_has_prefix_name(text, length, "return", 1)) {
        return LM_P0_TRAILER_ROLE_TAIL_CUTTER;
    }
    if (lm_p0_text_has_prefix_name(text, length, "until", 0)) {
        return LM_P0_TRAILER_ROLE_TAIL_CUTTER;
    }
    return LM_P0_TRAILER_ROLE_NONE;
}

static void lm_p0_stack_free(LmP0Stack *stack) {
    free(stack->parents);
    free(stack->owners);
    free(stack->hard);
    stack->parents = NULL;
    stack->owners = NULL;
    stack->hard = NULL;
    stack->capacity = 0U;
}

static int lm_p0_node_can_own_children(LmP0Node *node) {
    return node->kind == LM_P0_NODE_FRAME || node->kind == LM_P0_NODE_STRUCTURE;
}

static LmP0Structure *lm_p0_node_child_structure(LmP0Node *node) {
    if (node->kind == LM_P0_NODE_FRAME) {
        return &node->as.frame.body;
    }
    if (node->kind == LM_P0_NODE_STRUCTURE) {
        return &node->as.structure;
    }
    return NULL;
}

static LmP0Trailer **lm_p0_node_trailer_slot(LmP0Node *node) {
    if (node->kind == LM_P0_NODE_FRAME) {
        return &node->as.frame.trailer;
    }
    if (node->kind == LM_P0_NODE_STRUCTURE) {
        return &node->as.structure.trailer;
    }
    return NULL;
}

static LmP0Trailer *lm_p0_attach_trailer(
    LmP0Document *document,
    LmP0Node *node,
    const char *spelling,
    size_t spelling_length,
    unsigned flags,
    size_t line,
    size_t column
) {
    LmP0Trailer **slot;
    LmP0Trailer *trailer;

    slot = lm_p0_node_trailer_slot(node);
    if (slot == NULL) {
        lm_p0_set_diagnostic(document, 10, line, column, "this parser node cannot receive a trailer");
        return NULL;
    }
    if (*slot != NULL) {
        lm_p0_set_diagnostic(document, 11, line, column, "parser node already has a trailer");
        return NULL;
    }

    trailer = (LmP0Trailer *)calloc(1U, sizeof(*trailer));
    if (trailer == NULL) {
        lm_p0_set_diagnostic(document, 1, line, column, "out of memory while allocating parser trailer");
        return NULL;
    }
    trailer->spelling.data = spelling;
    trailer->spelling.length = spelling_length;
    trailer->flags = flags;
    *slot = trailer;
    return trailer;
}

static int lm_p0_parse_trailer_item(
    LmP0Document *document,
    LmP0Node *target,
    const char *text,
    size_t length,
    size_t line,
    size_t column,
    size_t offset,
    unsigned flags,
    LmP0Structure **out_body
) {
    size_t colon_index;
    size_t spelling_length;
    size_t body_start;
    LmP0Trailer *trailer;

    *out_body = NULL;

    if (lm_p0_find_colon(text, length, &colon_index)) {
        spelling_length = colon_index;
        while (spelling_length > 0U && text[spelling_length - 1U] == ' ') {
            --spelling_length;
        }
        body_start = colon_index + 1U;
        while (body_start < length && text[body_start] == ' ') {
            ++body_start;
        }
    } else {
        body_start = 0U;
        if (body_start < length && (text[body_start] == '"' || text[body_start] == '`')) {
            if (!lm_p0_scan_quoted(document, text, length, &body_start, text[body_start], line, column)) {
                return 0;
            }
            spelling_length = body_start;
        } else {
            while (body_start < length && text[body_start] != ' ' && text[body_start] != ',') {
                ++body_start;
            }
            spelling_length = body_start;
        }
        while (body_start < length && text[body_start] == ' ') {
            ++body_start;
        }
    }

    if (spelling_length == 0U) {
        lm_p0_set_diagnostic(document, 12, line, column, "trailer spelling is empty");
        return 0;
    }

    trailer = lm_p0_attach_trailer(document, target, text, spelling_length, flags, line, column);
    if (trailer == NULL) {
        return 0;
    }

    if (!lm_p0_parse_fields_into(
            document,
            &trailer->body,
            text + body_start,
            length - body_start,
            line,
            column + body_start,
            offset + body_start
        )) {
        return 0;
    }

    *out_body = &trailer->body;
    return 1;
}

static int lm_p0_parse_events(LmP0Document *document, const LmP0EventList *events) {
    LmP0Stack stack;
    size_t i;

    memset(&stack, 0, sizeof(stack));

    document->root = lm_p0_new_node(document, LM_P0_NODE_STRUCTURE);
    if (document->root == NULL) {
        return 0;
    }

    if (!lm_p0_stack_ensure(document, &stack, 0U)) {
        lm_p0_stack_free(&stack);
        return 0;
    }
    stack.parents[0] = &document->root->as.structure;
    stack.owners[0] = document->root;

    for (i = 0U; i < events->count; ++i) {
        const LmP0LineEvent *event;
        size_t top_level;
        LmP0TrailerRole trailer_role;

        event = &events->items[i];
        if (!lm_p0_stack_ensure(document, &stack, event->level + 1U)) {
            lm_p0_stack_free(&stack);
            return 0;
        }

        trailer_role = event->kind == LM_P0_LINE_ITEM
            ? lm_p0_trailer_role(event->text, event->text_length)
            : LM_P0_TRAILER_ROLE_NONE;

        top_level = lm_p0_stack_top_level(&stack);
        if (trailer_role != LM_P0_TRAILER_ROLE_TAIL_CUTTER ||
            event->level + 1U > top_level ||
            stack.owners[event->level + 1U] == NULL) {
            top_level = lm_p0_stack_collapse_soft_to_event(&stack, event->level);
            if (event->level == top_level && stack.hard[top_level] == 0U) {
                stack.hard[top_level] = 1U;
            }
        }

        if (event->kind == LM_P0_LINE_DELIM) {
            LmP0Structure *parent;

            if (event->level < top_level) {
                if (event->level + 1U != top_level) {
                    lm_p0_set_diagnostic(document, 13, event->line, event->column, "dotted point cannot close more than one open source level");
                    lm_p0_stack_free(&stack);
                    return 0;
                }
                lm_p0_stack_truncate_deeper(&stack, event->level);
            }

            parent = stack.parents[event->level];
            if (parent == NULL && event->level == 1U) {
                parent = stack.parents[0];
            }
            if (parent == NULL) {
                lm_p0_set_diagnostic(document, 8, event->line, event->column, "source level has no open parent structure");
                lm_p0_stack_free(&stack);
                return 0;
            }

            if (i + 1U < events->count && events->items[i + 1U].level == event->level + 1U) {
                LmP0Node *anonymous_node;

                anonymous_node = lm_p0_new_node(document, LM_P0_NODE_STRUCTURE);
                if (anonymous_node == NULL) {
                    lm_p0_stack_free(&stack);
                    return 0;
                }
                anonymous_node->span.line = event->line;
                anonymous_node->span.column = event->column;
                anonymous_node->span.offset = event->offset;
                anonymous_node->span.length = event->text_length;

                if (!lm_p0_append_field(document, parent, anonymous_node)) {
                    lm_p0_free_node(anonymous_node);
                    lm_p0_stack_free(&stack);
                    return 0;
                }
                stack.parents[event->level + 1U] = &anonymous_node->as.structure;
                stack.owners[event->level + 1U] = anonymous_node;
                stack.hard[event->level + 1U] = 1U;
                lm_p0_stack_truncate_deeper(&stack, event->level + 1U);
            } else {
                lm_p0_stack_truncate_deeper(&stack, event->level);
            }
        } else {
            LmP0Node *node;
            LmP0Structure *parent;

            if (trailer_role == LM_P0_TRAILER_ROLE_TAIL_CUTTER &&
                event->level + 1U <= top_level &&
                stack.owners[event->level + 1U] != NULL) {
                LmP0Structure *trailer_body;
                LmP0Node *target;
                size_t target_level;

                target_level = event->level + 1U;
                target = stack.owners[target_level];
                lm_p0_stack_truncate_deeper(&stack, target_level);

                if (!lm_p0_parse_trailer_item(
                        document,
                        target,
                        event->text,
                        event->text_length,
                        event->line,
                        event->column,
                        event->offset,
                        LM_P0_TRAILER_TAIL_CUTTER,
                        &trailer_body
                    )) {
                    lm_p0_stack_free(&stack);
                    return 0;
                }
                stack.parents[target_level] = trailer_body;
                stack.owners[target_level] = target;
                stack.hard[target_level] = 0U;
                if (target_level > 0U) {
                    stack.hard[target_level - 1U] = 0U;
                }
                lm_p0_stack_truncate_deeper(&stack, target_level);
                continue;
            }

            if (event->level < top_level) {
                lm_p0_set_diagnostic(
                    document,
                    16,
                    event->line,
                    event->column,
                    "name is not an accepted block trailer; expected end, ---, return, or until"
                );
                lm_p0_stack_free(&stack);
                return 0;
            }

            parent = stack.parents[event->level];
            if (parent == NULL) {
                lm_p0_set_diagnostic(document, 8, event->line, event->column, "source level has no open parent structure");
                lm_p0_stack_free(&stack);
                return 0;
            }

            node = lm_p0_parse_line_item(
                document,
                event->text,
                event->text_length,
                event->line,
                event->column,
                event->offset
            );
            if (node == NULL) {
                lm_p0_stack_free(&stack);
                return 0;
            }

            if (!lm_p0_append_field(document, parent, node)) {
                lm_p0_free_node(node);
                lm_p0_stack_free(&stack);
                return 0;
            }

            if (lm_p0_node_can_own_children(node)) {
                stack.parents[event->level + 1U] = lm_p0_node_child_structure(node);
                stack.owners[event->level + 1U] = node;
                if (node->kind == LM_P0_NODE_FRAME) {
                    stack.hard[event->level + 1U] = node->as.frame.body.field_count == 0U ? 1U : 0U;
                } else {
                    stack.hard[event->level + 1U] = node->as.structure.field_count == 0U ? 1U : 0U;
                }
            } else {
                stack.parents[event->level + 1U] = NULL;
                stack.owners[event->level + 1U] = NULL;
                stack.hard[event->level + 1U] = 0U;
            }
            lm_p0_stack_truncate_deeper(&stack, event->level + 1U);
        }
    }

    lm_p0_stack_free(&stack);
    return document->diagnostic.code == 0;
}

static int lm_p0_validate_nonempty_colon_frames_in_structure(
    LmP0Document *document,
    const LmP0Structure *structure
);

static int lm_p0_validate_nonempty_colon_frames_in_trailer(
    LmP0Document *document,
    const LmP0Trailer *trailer
) {
    if (trailer == NULL) {
        return 1;
    }
    return lm_p0_validate_nonempty_colon_frames_in_structure(document, &trailer->body);
}

static int lm_p0_validate_nonempty_colon_frames_in_node(LmP0Document *document, const LmP0Node *node) {
    if (node == NULL) {
        return 1;
    }

    if (node->kind == LM_P0_NODE_FRAME) {
        if ((node->as.frame.flags & LM_P0_FRAME_COLON) != 0U && node->as.frame.body.field_count == 0U) {
            lm_p0_set_diagnostic(
                document,
                9,
                node->span.line,
                node->span.column,
                "colon frame must have inline or vertical body fields"
            );
            return 0;
        }
        if (!lm_p0_validate_nonempty_colon_frames_in_structure(document, &node->as.frame.body)) {
            return 0;
        }
        return lm_p0_validate_nonempty_colon_frames_in_trailer(document, node->as.frame.trailer);
    }

    if (node->kind == LM_P0_NODE_STRUCTURE) {
        if (!lm_p0_validate_nonempty_colon_frames_in_structure(document, &node->as.structure)) {
            return 0;
        }
        return lm_p0_validate_nonempty_colon_frames_in_trailer(document, node->as.structure.trailer);
    }

    return 1;
}

static int lm_p0_validate_nonempty_colon_frames_in_structure(
    LmP0Document *document,
    const LmP0Structure *structure
) {
    const LmP0Field *field;

    field = structure->first_field;
    while (field != NULL) {
        if (!lm_p0_validate_nonempty_colon_frames_in_node(document, field->value)) {
            return 0;
        }
        field = field->next;
    }

    return 1;
}

int lm_p0_parse_string(const char *source, LmP0Document **out_document) {
    LmP0Document *document;
    LmP0EventList events;

    if (out_document == NULL) {
        return 1;
    }
    *out_document = NULL;

    document = (LmP0Document *)calloc(1U, sizeof(*document));
    if (document == NULL) {
        return 1;
    }

    if (source == NULL) {
        source = "";
    }

    document->source_length = strlen(source);
    document->source = lm_p0_copy_bytes(source, document->source_length);
    if (document->source == NULL) {
        free(document);
        return 1;
    }

    memset(&events, 0, sizeof(events));
    if (lm_p0_normalize_lines(document, &events)) {
        if (lm_p0_parse_events(document, &events)) {
            (void)lm_p0_validate_nonempty_colon_frames_in_node(document, document->root);
        }
    }
    free(events.items);

    *out_document = document;
    return document->diagnostic.code == 0 ? 0 : document->diagnostic.code;
}

int lm_p0_parse_file(const char *path, LmP0Document **out_document) {
    FILE *file;
    long size;
    char *buffer;
    size_t read_size;
    int status;

    if (out_document == NULL) {
        return 1;
    }
    *out_document = NULL;

    file = fopen(path, "rb");
    if (file == NULL) {
        return 1;
    }

    if (fseek(file, 0L, SEEK_END) != 0) {
        fclose(file);
        return 1;
    }
    size = ftell(file);
    if (size < 0L) {
        fclose(file);
        return 1;
    }
    if (fseek(file, 0L, SEEK_SET) != 0) {
        fclose(file);
        return 1;
    }

    buffer = (char *)malloc((size_t)size + 1U);
    if (buffer == NULL) {
        fclose(file);
        return 1;
    }
    read_size = fread(buffer, 1U, (size_t)size, file);
    fclose(file);
    if (read_size != (size_t)size) {
        free(buffer);
        return 1;
    }
    buffer[size] = '\0';

    status = lm_p0_parse_string(buffer, out_document);
    free(buffer);
    return status;
}

void lm_p0_document_destroy(LmP0Document *document) {
    if (document == NULL) {
        return;
    }
    lm_p0_free_node(document->root);
    free(document->source);
    free(document);
}

const LmP0Node *lm_p0_document_root(const LmP0Document *document) {
    return document != NULL ? document->root : NULL;
}

const LmP0Diagnostic *lm_p0_document_diagnostic(const LmP0Document *document) {
    if (document == NULL || document->diagnostic.code == 0) {
        return NULL;
    }
    return &document->diagnostic;
}

void lm_p0_free(void *ptr) {
    free(ptr);
}

static int lm_p0_dump_reserve(LmP0Dump *dump, size_t extra) {
    char *data;
    size_t new_capacity;

    if (dump->failed) {
        return 0;
    }
    if (dump->length + extra + 1U <= dump->capacity) {
        return 1;
    }

    new_capacity = dump->capacity == 0U ? 256U : dump->capacity;
    while (new_capacity < dump->length + extra + 1U) {
        new_capacity *= 2U;
    }

    data = (char *)realloc(dump->data, new_capacity);
    if (data == NULL) {
        dump->failed = 1;
        return 0;
    }
    dump->data = data;
    dump->capacity = new_capacity;
    return 1;
}

static void lm_p0_dump_append(LmP0Dump *dump, const char *text, size_t length) {
    if (!lm_p0_dump_reserve(dump, length)) {
        return;
    }
    memcpy(dump->data + dump->length, text, length);
    dump->length += length;
    dump->data[dump->length] = '\0';
}

static void lm_p0_dump_append_cstr(LmP0Dump *dump, const char *text) {
    lm_p0_dump_append(dump, text, strlen(text));
}

static void lm_p0_dump_appendf(LmP0Dump *dump, const char *format, ...) {
    char stack_buffer[256];
    va_list args;
    int needed;

    va_start(args, format);
    needed = vsnprintf(stack_buffer, sizeof(stack_buffer), format, args);
    va_end(args);

    if (needed < 0) {
        dump->failed = 1;
        return;
    }

    if ((size_t)needed < sizeof(stack_buffer)) {
        lm_p0_dump_append(dump, stack_buffer, (size_t)needed);
    } else {
        char *heap_buffer;

        heap_buffer = (char *)malloc((size_t)needed + 1U);
        if (heap_buffer == NULL) {
            dump->failed = 1;
            return;
        }
        va_start(args, format);
        (void)vsnprintf(heap_buffer, (size_t)needed + 1U, format, args);
        va_end(args);
        lm_p0_dump_append(dump, heap_buffer, (size_t)needed);
        free(heap_buffer);
    }
}

static void lm_p0_dump_indent(LmP0Dump *dump, size_t indent) {
    size_t i;

    for (i = 0U; i < indent; ++i) {
        lm_p0_dump_append_cstr(dump, "  ");
    }
}

static void lm_p0_dump_text(LmP0Dump *dump, LmP0Text text) {
    lm_p0_dump_append_cstr(dump, "\"");
    lm_p0_dump_append(dump, text.data, text.length);
    lm_p0_dump_append_cstr(dump, "\"");
}

static void lm_p0_dump_structure(LmP0Dump *dump, const LmP0Structure *structure, size_t indent);

static void lm_p0_dump_trailer(LmP0Dump *dump, const LmP0Trailer *trailer, size_t indent) {
    if (trailer == NULL) {
        return;
    }

    lm_p0_dump_indent(dump, indent);
    if ((trailer->flags & LM_P0_TRAILER_TAIL_CUTTER) != 0U) {
        lm_p0_dump_append_cstr(dump, "Tail cutter trailer spelling=");
    } else {
        lm_p0_dump_append_cstr(dump, "Trailer spelling=");
    }
    lm_p0_dump_text(dump, trailer->spelling);
    lm_p0_dump_appendf(dump, " fields=%lu\n", (unsigned long)trailer->body.field_count);
    lm_p0_dump_structure(dump, &trailer->body, indent + 1U);
}

static void lm_p0_dump_node(LmP0Dump *dump, const LmP0Node *node, size_t indent) {
    if (node == NULL) {
        return;
    }

    lm_p0_dump_indent(dump, indent);
    if (node->kind == LM_P0_NODE_STRUCTURE) {
        lm_p0_dump_appendf(dump, "Structure fields=%lu\n", (unsigned long)node->as.structure.field_count);
        lm_p0_dump_structure(dump, &node->as.structure, indent + 1U);
        lm_p0_dump_trailer(dump, node->as.structure.trailer, indent + 1U);
    } else if (node->kind == LM_P0_NODE_FRAME) {
        lm_p0_dump_append_cstr(dump, "Frame head=");
        lm_p0_dump_text(dump, node->as.frame.head);
        lm_p0_dump_appendf(dump, " fields=%lu\n", (unsigned long)node->as.frame.body.field_count);
        lm_p0_dump_structure(dump, &node->as.frame.body, indent + 1U);
        lm_p0_dump_trailer(dump, node->as.frame.trailer, indent + 1U);
    } else if (node->kind == LM_P0_NODE_ATOM) {
        lm_p0_dump_append_cstr(dump, "Atom ");
        lm_p0_dump_text(dump, node->as.atom);
        lm_p0_dump_append_cstr(dump, "\n");
    } else if (node->kind == LM_P0_NODE_EXPR) {
        lm_p0_dump_append_cstr(dump, "Expr ");
        lm_p0_dump_text(dump, node->as.expr);
        lm_p0_dump_append_cstr(dump, "\n");
    }
}

static void lm_p0_dump_structure(LmP0Dump *dump, const LmP0Structure *structure, size_t indent) {
    const LmP0Field *field;

    field = structure->first_field;
    while (field != NULL) {
        lm_p0_dump_node(dump, field->value, indent);
        field = field->next;
    }
}

char *lm_p0_dump_alloc(const LmP0Document *document) {
    LmP0Dump dump;

    memset(&dump, 0, sizeof(dump));
    if (document == NULL || document->root == NULL) {
        lm_p0_dump_append_cstr(&dump, "");
        return dump.data;
    }

    lm_p0_dump_node(&dump, document->root, 0U);
    if (dump.failed) {
        free(dump.data);
        return NULL;
    }
    return dump.data;
}
