#include "own.lm1.h"
#include "parser.lm1.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



typedef enum LmP0StreamEventKind {
    LM_P0_STREAM_EVENT_ITEM = 1,
    LM_P0_STREAM_EVENT_DELIM = 2,
    LM_P0_STREAM_EVENT_BLOCK_STRING = 3,
    LM_P0_STREAM_EVENT_DISABLED_BLOCK = 4
} LmP0StreamEventKind;

typedef struct LmP0StreamEvent {
    LmP0StreamEventKind kind;
    unsigned node_flags;
    size_t level;
    const char *text;
    size_t text_length;
    size_t line;
    size_t column;
    size_t offset;
} LmP0StreamEvent;

struct LmP0Document {
    char *source;
    size_t source_length;
    LmP0Node *root;
    LmP0Diagnostic diagnostic;
};

typedef struct LmP0PendingDelimiter {
    int active;
    LmP0StreamEvent *event;
} LmP0PendingDelimiter;

typedef struct LmP0IndentStack {
    size_t *columns;
    size_t count;
    size_t capacity;
} LmP0IndentStack;

typedef struct LmP0DisabledState {
    int body_started;
    int pending_item;
    size_t base_level;
    size_t top_level;
    size_t pending_level;
} LmP0DisabledState;

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
    LM_P0_TRAILER_ROLE_DASH_CUTTER = 1,
    LM_P0_TRAILER_ROLE_END = 2,
    LM_P0_TRAILER_ROLE_RETURN = 3,
    LM_P0_TRAILER_ROLE_UNTIL = 4
} LmP0TrailerRole;

typedef enum LmP0DashFenceStatus {
    LM_P0_DASH_FENCE_NONE = 0,
    LM_P0_DASH_FENCE_VALID = 1,
    LM_P0_DASH_FENCE_TOO_LONG = 2,
    LM_P0_DASH_FENCE_TRAILING_TEXT = 3
} LmP0DashFenceStatus;

typedef enum LmP0FieldParseFlags {
    LM_P0_FIELD_PARSE_STOP_ON_COMMA = 1,
    LM_P0_FIELD_PARSE_STOP_ON_SOURCE_LEVEL = 2
} LmP0FieldParseFlags;

#define LM_P0_MAX_FENCE_LENGTH 80U

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

static int lm_p0_is_horizontal_space(char c) {
    return c == ' ' || c == '\t';
}

static int lm_p0_is_line_break(char c) {
    return c == '\n' || c == '\r';
}

static size_t lm_p0_line_break_width_at(const char *source, size_t length, size_t index) {
    if (index >= length) {
        return 0U;
    }
    if (source[index] == '\r') {
        return index + 1U < length && source[index + 1U] == '\n' ? 2U : 1U;
    }
    return source[index] == '\n' ? 1U : 0U;
}

static int lm_p0_is_field_space(char c) {
    return lm_p0_is_horizontal_space(c) || lm_p0_is_line_break(c);
}

static int lm_p0_is_quoted_token_boundary(char c) {
    return lm_p0_is_field_space(c) || c == ',' || c == '(' || c == ')' || c == '#';
}

static int lm_p0_starts_python_string(const char *text, size_t length, size_t index) {
    char quote;

    if (index + 2U >= length) {
        return 0;
    }
    quote = text[index];
    if (quote != '"' && quote != '\'') {
        return 0;
    }
    return text[index + 1U] == quote && text[index + 2U] == quote;
}

static int lm_p0_find_python_string_end(
    const char *text,
    size_t length,
    size_t start,
    size_t *out_end
) {
    char quote;
    size_t i;

    if (!lm_p0_starts_python_string(text, length, start)) {
        *out_end = start;
        return 0;
    }

    quote = text[start];
    i = start + 3U;
    while (i < length) {
        if (text[i] == quote) {
            size_t run_length;

            run_length = 1U;
            while (i + run_length < length && text[i + run_length] == quote) {
                ++run_length;
            }
            if (run_length == 3U) {
                *out_end = i + 3U;
                return 1;
            }
            i += run_length;
            continue;
        }
        ++i;
    }

    *out_end = length;
    return 0;
}

static size_t lm_p0_skip_python_string_unchecked(const char *text, size_t length, size_t start) {
    size_t end;

    if (!lm_p0_find_python_string_end(text, length, start, &end)) {
        return length;
    }
    return end;
}

static void lm_p0_trim_right(const char **text, size_t *length) {
    (void)text;
    while (*length > 0U && (lm_p0_is_horizontal_space((*text)[*length - 1U]) || (*text)[*length - 1U] == '\r')) {
        --(*length);
    }
}

static void lm_p0_trim_trailing_line_comment(const char **text, size_t *length) {
    size_t i;

    i = 0U;
    while (i < *length) {
        if (lm_p0_starts_python_string(*text, *length, i)) {
            i = lm_p0_skip_python_string_unchecked(*text, *length, i);
            continue;
        }
        if ((*text)[i] == '"' || (*text)[i] == '`') {
            char quote;

            quote = (*text)[i++];
            while (i < *length) {
                if (quote == '"' && (*text)[i] == '\\') {
                    i += i + 1U < *length ? 2U : 1U;
                    continue;
                }
                if (quote == '`' && (*text)[i] == '`' && i + 1U < *length && (*text)[i + 1U] == '`') {
                    i += 2U;
                    continue;
                }
                if ((*text)[i++] == quote) {
                    break;
                }
            }
            continue;
        }

        if ((*text)[i] == '#') {
            size_t end;

            end = i + 1U;
            while (end < *length && !lm_p0_is_line_break((*text)[end])) {
                ++end;
            }
            if (end == *length) {
                *length = i;
                lm_p0_trim_right(text, length);
            }
            return;
        }
        ++i;
    }
}

static void lm_p0_indent_stack_free(LmP0IndentStack *stack) {
    free(stack->columns);
    stack->columns = NULL;
    stack->count = 0U;
    stack->capacity = 0U;
}

static void lm_p0_indent_stack_free_any(void *object) {
    lm_p0_indent_stack_free((LmP0IndentStack *)object);
}

static int lm_p0_indent_stack_push(
    LmP0Document *document,
    LmP0IndentStack *stack,
    size_t column,
    size_t line,
    size_t source_column
) {
    size_t new_capacity;
    size_t *columns;

    if (stack->count == stack->capacity) {
        new_capacity = stack->capacity == 0U ? 8U : stack->capacity * 2U;
        columns = (size_t *)realloc(stack->columns, new_capacity * sizeof(*columns));
        if (columns == NULL) {
            lm_p0_set_diagnostic(document, 1, line, source_column, "out of memory while storing indentation levels");
            return 0;
        }
        stack->columns = columns;
        stack->capacity = new_capacity;
    }

    stack->columns[stack->count++] = column;
    return 1;
}

static int lm_p0_indent_stack_init(LmP0Document *document, LmP0IndentStack *stack) {
    memset(stack, 0, sizeof(*stack));
    return lm_p0_indent_stack_push(document, stack, 0U, 0U, 0U);
}

static LmP0IndentStack *lm_p0_indent_stack_new_empty(void) {
    return (LmP0IndentStack *)calloc(1U, sizeof(LmP0IndentStack));
}

static LmP0IndentStack *lm_p0_indent_stack_new(LmP0Document *document) {
    LmP0IndentStack *stack;

    stack = lm_p0_indent_stack_new_empty();
    if (stack == NULL) {
        lm_p0_set_diagnostic(document, 1, 0U, 0U, "out of memory while creating indentation stack");
        return NULL;
    }
    if (!lm_p0_indent_stack_init(document, stack)) {
        lm_own_delete(stack, lm_p0_indent_stack_free_any);
        return NULL;
    }
    return stack;
}

static void lm_p0_indent_stack_delete(LmP0IndentStack *stack) {
    lm_own_delete(stack, lm_p0_indent_stack_free_any);
}

static int lm_p0_indent_stack_copy(
    LmP0Document *document,
    LmP0IndentStack *target,
    const LmP0IndentStack *source,
    size_t line,
    size_t column
) {
    size_t capacity;

    memset(target, 0, sizeof(*target));
    capacity = source->capacity > source->count ? source->capacity : source->count;
    if (capacity == 0U) {
        return 1;
    }

    target->columns = (size_t *)malloc(capacity * sizeof(*target->columns));
    if (target->columns == NULL) {
        lm_p0_set_diagnostic(document, 1, line, column, "out of memory while copying indentation levels");
        return 0;
    }
    memcpy(target->columns, source->columns, source->count * sizeof(*target->columns));
    target->count = source->count;
    target->capacity = capacity;
    return 1;
}

static LmP0IndentStack *lm_p0_indent_stack_clone(
    LmP0Document *document,
    const LmP0IndentStack *source,
    size_t line,
    size_t column
) {
    LmP0IndentStack *target;

    target = lm_p0_indent_stack_new_empty();
    if (target == NULL) {
        lm_p0_set_diagnostic(document, 1, line, column, "out of memory while copying indentation levels");
        return NULL;
    }
    if (!lm_p0_indent_stack_copy(document, target, source, line, column)) {
        lm_p0_indent_stack_delete(target);
        return NULL;
    }
    return target;
}

static size_t lm_p0_indent_tab_column(size_t column) {
    return ((column / 8U) + 1U) * 8U;
}

static void lm_p0_scan_indent_column(
    const char *source,
    size_t start,
    size_t end,
    size_t *out_offset,
    size_t *out_column
) {
    size_t p;
    size_t column;

    p = start;
    column = 0U;
    while (p < end && lm_p0_is_horizontal_space(source[p])) {
        if (source[p] == '\t') {
            column = lm_p0_indent_tab_column(column);
        } else {
            ++column;
        }
        ++p;
    }

    *out_offset = p;
    *out_column = column;
}

static int lm_p0_indent_level_from_column(
    LmP0Document *document,
    LmP0IndentStack *stack,
    size_t column,
    size_t line,
    size_t source_column,
    size_t *out_level
) {
    size_t top;

    top = stack->columns[stack->count - 1U];
    if (column == top) {
        *out_level = stack->count - 1U;
        return 1;
    }

    if (column > top) {
        if (!lm_p0_indent_stack_push(document, stack, column, line, source_column)) {
            return 0;
        }
        *out_level = stack->count - 1U;
        return 1;
    }

    while (stack->count > 0U && stack->columns[stack->count - 1U] > column) {
        --stack->count;
    }
    if (stack->count == 0U || stack->columns[stack->count - 1U] != column) {
        lm_p0_set_diagnostic(document, 17, line, source_column, "unindent does not match any outer indentation level");
        return 0;
    }

    *out_level = stack->count - 1U;
    return 1;
}

static size_t lm_p0_scan_brace_mark_unchecked(
    const char *text,
    size_t length,
    size_t start,
    int *closed
);

static size_t lm_p0_find_layout_line_end(const char *source, size_t length, size_t start) {
    size_t i;
    size_t depth;
    char quote;

    i = start;
    depth = 0U;
    quote = '\0';
    while (i < length) {
        if (quote != '\0') {
            if (quote == '"' && source[i] == '\\') {
                size_t line_break_width;

                line_break_width = i + 1U < length
                    ? lm_p0_line_break_width_at(source, length, i + 1U)
                    : 0U;
                if (line_break_width > 0U) {
                    i += 1U + line_break_width;
                } else {
                    i += i + 1U < length ? 2U : 1U;
                }
                continue;
            }
            if (quote == '`' && source[i] == '`' && i + 1U < length && source[i + 1U] == '`') {
                i += 2U;
                continue;
            }
            if (source[i] == quote) {
                quote = '\0';
            }
            ++i;
            continue;
        }

        if (lm_p0_starts_python_string(source, length, i)) {
            i = lm_p0_skip_python_string_unchecked(source, length, i);
            continue;
        }
        if (source[i] == '"' || source[i] == '`') {
            quote = source[i++];
            continue;
        }
        if (source[i] == '#') {
            while (i < length && !lm_p0_is_line_break(source[i])) {
                ++i;
            }
            continue;
        }
        if (source[i] == '{') {
            int closed;
            size_t brace_end;

            brace_end = lm_p0_scan_brace_mark_unchecked(source, length, i, &closed);
            if (closed) {
                i = brace_end;
                continue;
            }
        }
        if (source[i] == '(') {
            ++depth;
        } else if (source[i] == ')' && depth > 0U) {
            --depth;
        } else if (lm_p0_is_line_break(source[i]) && depth == 0U) {
            break;
        }
        ++i;
    }

    return i;
}

static void lm_p0_position_in_slice(
    const char *text,
    size_t length,
    size_t index,
    size_t base_line,
    size_t base_column,
    size_t *out_line,
    size_t *out_column
);

static size_t lm_p0_scan_brace_mark_unchecked(
    const char *text,
    size_t length,
    size_t start,
    int *closed
) {
    size_t i;
    size_t depth;

    i = start;
    depth = 0U;
    *closed = 0;
    while (i < length) {
        if (text[i] == '{') {
            ++depth;
            ++i;
            continue;
        }
        if (text[i] == '}') {
            ++i;
            if (depth > 0U) {
                --depth;
                if (depth == 0U) {
                    *closed = 1;
                    return i;
                }
            }
            continue;
        }
        ++i;
    }

    return i;
}

static int lm_p0_skip_brace_mark(
    LmP0Document *document,
    const char *text,
    size_t length,
    size_t *index,
    size_t line,
    size_t column
) {
    size_t start;
    int closed;

    if (*index >= length || text[*index] != '{') {
        return 1;
    }

    start = *index;
    *index = lm_p0_scan_brace_mark_unchecked(text, length, start, &closed);
    if (!closed) {
        size_t diagnostic_line;
        size_t diagnostic_column;

        lm_p0_position_in_slice(text, length, start, line, column, &diagnostic_line, &diagnostic_column);
        lm_p0_set_diagnostic(document, 31, diagnostic_line, diagnostic_column, "unterminated brace mark");
        return 0;
    }

    return 1;
}

static int lm_p0_skip_leading_brace_marks(
    LmP0Document *document,
    const char *text,
    size_t length,
    size_t line,
    size_t column,
    size_t *index
) {
    while (*index < length) {
        while (*index < length && lm_p0_is_horizontal_space(text[*index])) {
            ++(*index);
        }
        if (*index >= length || text[*index] != '{') {
            return 1;
        }
        if (!lm_p0_skip_brace_mark(document, text, length, index, line, column)) {
            return 0;
        }
    }
    return 1;
}

static int lm_p0_line_rest_is_horizontal_space(const char *source, size_t start, size_t end) {
    size_t i;

    i = start;
    while (i < end) {
        if (!lm_p0_is_horizontal_space(source[i])) {
            return 0;
        }
        ++i;
    }
    return 1;
}

static LmP0DashFenceStatus lm_p0_dash_fence_status(
    const char *text,
    size_t length,
    size_t *out_dash_count
) {
    size_t dash_count;
    size_t i;

    dash_count = 0U;
    while (dash_count < length && text[dash_count] == '-') {
        ++dash_count;
    }
    if (out_dash_count != NULL) {
        *out_dash_count = dash_count;
    }
    if (dash_count < 3U) {
        return LM_P0_DASH_FENCE_NONE;
    }
    if (dash_count > LM_P0_MAX_FENCE_LENGTH) {
        return LM_P0_DASH_FENCE_TOO_LONG;
    }

    i = dash_count;
    while (i < length) {
        if (!lm_p0_is_horizontal_space(text[i])) {
            return LM_P0_DASH_FENCE_TRAILING_TEXT;
        }
        ++i;
    }
    return LM_P0_DASH_FENCE_VALID;
}

static LmP0DashFenceStatus lm_p0_dash_fence_status_after_comment_trim(
    const char *text,
    size_t length,
    size_t *out_dash_count
) {
    const char *trimmed_text;
    size_t trimmed_length;

    trimmed_text = text;
    trimmed_length = length;
    lm_p0_trim_trailing_line_comment(&trimmed_text, &trimmed_length);
    return lm_p0_dash_fence_status(trimmed_text, trimmed_length, out_dash_count);
}

static int lm_p0_validate_dash_fence_line(
    LmP0Document *document,
    const char *text,
    size_t length,
    size_t line,
    size_t column
) {
    LmP0DashFenceStatus status;

    status = lm_p0_dash_fence_status_after_comment_trim(text, length, NULL);
    if (status == LM_P0_DASH_FENCE_TOO_LONG) {
        lm_p0_set_diagnostic(document, 25, line, column, "dash delimiter fence length exceeds 80 characters");
        return 0;
    }
    if (status == LM_P0_DASH_FENCE_TRAILING_TEXT) {
        lm_p0_set_diagnostic(document, 26, line, column, "dash delimiter line must contain only the dash fence, whitespace, or a line comment");
        return 0;
    }
    return 1;
}

static size_t lm_p0_find_physical_line_end(const char *source, size_t length, size_t start) {
    size_t i;

    i = start;
    while (i < length && !lm_p0_is_line_break(source[i])) {
        ++i;
    }
    return i;
}

static int lm_p0_match_block_string_fence_line(
    const char *source,
    size_t line_start,
    size_t line_end,
    size_t eq_count
) {
    size_t i;

    if (eq_count < 3U || eq_count > LM_P0_MAX_FENCE_LENGTH || line_start + eq_count > line_end) {
        return 0;
    }
    for (i = 0U; i < eq_count; ++i) {
        if (source[line_start + i] != '=') {
            return 0;
        }
    }
    if (line_start + eq_count < line_end && source[line_start + eq_count] == '=') {
        return 0;
    }
    i = line_start + eq_count;
    while (i < line_end) {
        if (!lm_p0_is_horizontal_space(source[i])) {
            return 0;
        }
        ++i;
    }
    return 1;
}

static int lm_p0_match_raw_comment_fence_line(
    const char *source,
    size_t line_start,
    size_t line_end,
    size_t star_count
) {
    size_t i;

    if (star_count < 3U || star_count > LM_P0_MAX_FENCE_LENGTH || line_start + star_count > line_end) {
        return 0;
    }
    for (i = 0U; i < star_count; ++i) {
        if (source[line_start + i] != '*') {
            return 0;
        }
    }
    if (line_start + star_count < line_end && source[line_start + star_count] == '*') {
        return 0;
    }
    return lm_p0_line_rest_is_horizontal_space(source, line_start + star_count, line_end);
}

static int lm_p0_scan_raw_comment_block(
    LmP0Document *document,
    const char *source,
    size_t length,
    size_t line_start,
    size_t line,
    size_t *next_offset,
    size_t *next_line
) {
    size_t line_end;
    size_t star_count;
    size_t scan_start;
    size_t scan_line;

    if (line_start >= length || source[line_start] != '*') {
        return 0;
    }

    line_end = lm_p0_find_physical_line_end(source, length, line_start);
    star_count = 0U;
    while (line_start + star_count < line_end && source[line_start + star_count] == '*') {
        ++star_count;
    }
    if (star_count < 3U) {
        return 0;
    }
    if (star_count > LM_P0_MAX_FENCE_LENGTH) {
        lm_p0_set_diagnostic(document, 23, line, 1U, "raw comment fence length exceeds 80 characters");
        return 0;
    }
    if (!lm_p0_match_raw_comment_fence_line(source, line_start, line_end, star_count)) {
        lm_p0_set_diagnostic(document, 27, line, 1U, "raw comment fence line must contain only the star fence and whitespace");
        return 0;
    }

    scan_start = line_end;
    if (scan_start < length) {
        scan_start += lm_p0_line_break_width_at(source, length, scan_start);
    }
    scan_line = line + 1U;

    while (scan_start <= length) {
        size_t current_end;
        size_t break_width;

        current_end = lm_p0_find_physical_line_end(source, length, scan_start);
        if (lm_p0_match_raw_comment_fence_line(source, scan_start, current_end, star_count)) {
            break_width = current_end < length
                ? lm_p0_line_break_width_at(source, length, current_end)
                : 0U;
            *next_offset = current_end + break_width;
            *next_line = scan_line + (break_width > 0U ? 1U : 0U);
            return 1;
        }

        if (current_end == length) {
            break;
        }
        scan_start = current_end + lm_p0_line_break_width_at(source, length, current_end);
        ++scan_line;
    }

    lm_p0_set_diagnostic(document, 24, line, 1U, "unterminated raw comment block");
    return 0;
}

static int lm_p0_scan_block_string_event(
    LmP0Document *document,
    const char *source,
    size_t length,
    size_t line_start,
    size_t line,
    LmP0StreamEvent *event,
    size_t *next_offset,
    size_t *next_line
) {
    size_t line_end;
    size_t eq_count;
    size_t content_start;
    size_t content_end;
    size_t scan_start;
    size_t scan_line;

    if (line_start >= length || source[line_start] != '=') {
        return 0;
    }

    line_end = lm_p0_find_physical_line_end(source, length, line_start);
    eq_count = 0U;
    while (line_start + eq_count < line_end && source[line_start + eq_count] == '=') {
        ++eq_count;
    }
    if (eq_count < 3U) {
        return 0;
    }
    if (eq_count > LM_P0_MAX_FENCE_LENGTH) {
        lm_p0_set_diagnostic(document, 23, line, 1U, "block string fence length exceeds 80 characters");
        return 0;
    }
    if (!lm_p0_match_block_string_fence_line(source, line_start, line_end, eq_count)) {
        lm_p0_set_diagnostic(document, 29, line, 1U, "block string fence line must contain only the equals fence and whitespace");
        return 0;
    }

    scan_start = line_end;
    if (scan_start < length) {
        scan_start += lm_p0_line_break_width_at(source, length, scan_start);
    }
    content_start = scan_start;
    content_end = content_start;
    scan_line = line + 1U;

    while (scan_start <= length) {
        size_t current_end;
        size_t break_width;

        current_end = lm_p0_find_physical_line_end(source, length, scan_start);
        if (lm_p0_match_block_string_fence_line(source, scan_start, current_end, eq_count)) {
            break_width = current_end < length
                ? lm_p0_line_break_width_at(source, length, current_end)
                : 0U;

            memset(event, 0, sizeof(*event));
            event->kind = LM_P0_STREAM_EVENT_BLOCK_STRING;
            event->text = source + content_start;
            event->text_length = content_end - content_start;
            event->line = line;
            event->column = 1U;
            event->offset = line_start;

            *next_offset = current_end + break_width;
            *next_line = scan_line + (break_width > 0U ? 1U : 0U);
            return 1;
        }

        if (current_end == length) {
            break;
        }
        content_end = current_end;
        scan_start = current_end + lm_p0_line_break_width_at(source, length, current_end);
        ++scan_line;
    }

    lm_p0_set_diagnostic(document, 20, line, 1U, "unterminated block string literal");
    return 0;
}

static size_t lm_p0_count_line_breaks(const char *source, size_t start, size_t end) {
    size_t count;
    size_t i;

    count = 0U;
    i = start;
    while (i < end) {
        if (source[i] == '\r') {
            ++count;
            if (i + 1U < end && source[i + 1U] == '\n') {
                ++i;
            }
        } else if (source[i] == '\n') {
            ++count;
        }
        ++i;
    }
    return count;
}

static void lm_p0_position_in_slice(
    const char *text,
    size_t length,
    size_t index,
    size_t base_line,
    size_t base_column,
    size_t *out_line,
    size_t *out_column
) {
    size_t i;
    size_t line;
    size_t column;

    line = base_line;
    column = base_column;
    if (index > length) {
        index = length;
    }
    for (i = 0U; i < index; ++i) {
        if (text[i] == '\r') {
            ++line;
            column = 1U;
            if (i + 1U < index && text[i + 1U] == '\n') {
                ++i;
            }
        } else if (text[i] == '\n') {
            ++line;
            column = 1U;
        } else {
            ++column;
        }
    }

    *out_line = line;
    *out_column = column;
}

static void lm_p0_advance_layout_line(
    const char *source,
    size_t length,
    size_t line_start,
    size_t line_end,
    size_t *offset,
    size_t *line
) {
    size_t next_offset;
    size_t line_breaks;

    next_offset = line_end;
    line_breaks = lm_p0_count_line_breaks(source, line_start, line_end);
    if (next_offset < length && lm_p0_is_line_break(source[next_offset])) {
        ++line_breaks;
        if (source[next_offset] == '\r' && next_offset + 1U < length && source[next_offset + 1U] == '\n') {
            next_offset += 2U;
        } else {
            ++next_offset;
        }
    }

    *offset = next_offset;
    *line += line_breaks;
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

static int lm_p0_is_bare_identifier_start(char c) {
    unsigned char value;

    value = (unsigned char)c;
    return isalpha(value) || c == '_';
}

static int lm_p0_is_bare_identifier_rest(char c) {
    unsigned char value;

    value = (unsigned char)c;
    return isalnum(value) || c == '_';
}

static int lm_p0_is_hex_digit(char c) {
    unsigned char value;

    value = (unsigned char)c;
    return isxdigit(value) != 0;
}

static int lm_p0_is_decimal_digit(char c) {
    return c >= '0' && c <= '9';
}

static int lm_p0_scan_number_token(const char *text, size_t end, size_t start, size_t *out_end) {
    size_t i;
    int saw_digit;

    i = start;
    saw_digit = 0;

    if (i + 1U < end && text[i] == '0' && (text[i + 1U] == 'x' || text[i + 1U] == 'X')) {
        i += 2U;
        while (i < end && lm_p0_is_hex_digit(text[i])) {
            saw_digit = 1;
            ++i;
        }
        if (i < end && text[i] == '.') {
            ++i;
            while (i < end && lm_p0_is_hex_digit(text[i])) {
                saw_digit = 1;
                ++i;
            }
        }
        if (saw_digit && i < end && (text[i] == 'p' || text[i] == 'P')) {
            size_t exponent_start;

            exponent_start = i;
            ++i;
            if (i < end && (text[i] == '+' || text[i] == '-')) {
                ++i;
            }
            if (i < end && lm_p0_is_decimal_digit(text[i])) {
                while (i < end && lm_p0_is_decimal_digit(text[i])) {
                    ++i;
                }
            } else {
                i = exponent_start;
            }
        }
        while (i < end && lm_p0_is_bare_identifier_rest(text[i])) {
            ++i;
        }
        *out_end = saw_digit ? i : start + 1U;
        return 1;
    }

    while (i < end && lm_p0_is_decimal_digit(text[i])) {
        saw_digit = 1;
        ++i;
    }
    if (i < end && text[i] == '.') {
        ++i;
        while (i < end && lm_p0_is_decimal_digit(text[i])) {
            saw_digit = 1;
            ++i;
        }
    }
    if (saw_digit && i < end && (text[i] == 'e' || text[i] == 'E')) {
        size_t exponent_start;

        exponent_start = i;
        ++i;
        if (i < end && (text[i] == '+' || text[i] == '-')) {
            ++i;
        }
        if (i < end && lm_p0_is_decimal_digit(text[i])) {
            while (i < end && lm_p0_is_decimal_digit(text[i])) {
                ++i;
            }
        } else {
            i = exponent_start;
        }
    }
    while (i < end && lm_p0_is_bare_identifier_rest(text[i])) {
        ++i;
    }

    *out_end = saw_digit ? i : start;
    return saw_digit;
}

static size_t lm_p0_scan_c_char_token(const char *text, size_t end, size_t start) {
    size_t i;

    if (start >= end || text[start] != '\'') {
        return start;
    }

    i = start + 1U;
    while (i < end) {
        if (text[i] == '\\' && i + 1U < end) {
            i += 2U;
            continue;
        }
        if (text[i] == '\'') {
            return i + 1U;
        }
        if (lm_p0_is_line_break(text[i])) {
            return start + 1U;
        }
        ++i;
    }

    return start + 1U;
}

static size_t lm_p0_scan_compact_atom_piece(const char *text, size_t end, size_t start) {
    size_t i;

    if (start >= end) {
        return start;
    }

    if (start + 2U < end && text[start] == 'c' && text[start + 1U] == '.') {
        return end;
    }

    if (lm_p0_is_bare_identifier_start(text[start])) {
        i = start + 1U;
        while (i < end && lm_p0_is_bare_identifier_rest(text[i])) {
            ++i;
        }
        if (
            i == start + 1U &&
            text[start] == 'c' &&
            i + 1U < end &&
            text[i] == '.' &&
            lm_p0_is_bare_identifier_start(text[i + 1U])
        ) {
            i += 2U;
            while (i < end && lm_p0_is_bare_identifier_rest(text[i])) {
                ++i;
            }
        }
        return i;
    }

    if (lm_p0_is_decimal_digit(text[start])) {
        size_t number_end;

        if (lm_p0_scan_number_token(text, end, start, &number_end)) {
            return number_end;
        }
    }

    if (text[start] == '\'' && !lm_p0_starts_python_string(text, end, start)) {
        return lm_p0_scan_c_char_token(text, end, start);
    }

    if (start + 1U < end) {
        char first;
        char second;

        first = text[start];
        second = text[start + 1U];
        if (
            (first == '=' && second == '=') ||
            (first == '!' && second == '=') ||
            (first == '<' && second == '=') ||
            (first == '>' && second == '=') ||
            (first == '&' && second == '&') ||
            (first == '|' && second == '|') ||
            (first == '+' && second == '+') ||
            (first == '-' && second == '-') ||
            (first == '-' && second == '>') ||
            (first == '<' && second == '<') ||
            (first == '>' && second == '>') ||
            (first == '[' && second == ']')
        ) {
            return start + 2U;
        }
        if (first == '@' && second == '@') {
            size_t at_end;

            at_end = start + 2U;
            while (at_end < end && text[at_end] == '@') {
                ++at_end;
            }
            return at_end;
        }
    }

    return start + 1U;
}

static int lm_p0_append_atom_slice(
    LmP0Document *document,
    LmP0Structure *structure,
    const char *text,
    size_t length,
    size_t line,
    size_t column,
    size_t offset,
    size_t start,
    size_t end
) {
    LmP0Node *node;

    node = lm_p0_new_node(document, LM_P0_NODE_ATOM);
    if (node == NULL) {
        return 0;
    }
    node->as.atom.data = text + start;
    node->as.atom.length = end - start;
    node->span.line = line;
    lm_p0_position_in_slice(text, length, start, line, column, &node->span.line, &node->span.column);
    node->span.offset = offset + start;
    node->span.length = end - start;

    return lm_p0_append_field(document, structure, node);
}

static int lm_p0_append_compact_atom_pieces(
    LmP0Document *document,
    LmP0Structure *structure,
    const char *text,
    size_t length,
    size_t line,
    size_t column,
    size_t offset,
    size_t start,
    size_t end
) {
    size_t i;

    i = start;
    while (i < end) {
        size_t piece_end;

        piece_end = lm_p0_scan_compact_atom_piece(text, end, i);
        if (piece_end <= i) {
            piece_end = i + 1U;
        }
        if (!lm_p0_append_atom_slice(document, structure, text, length, line, column, offset, i, piece_end)) {
            return 0;
        }
        i = piece_end;
    }

    return 1;
}

static void lm_p0_free_node(LmP0Node *node);
static void lm_p0_free_structure_fields(LmP0Structure *structure);

static void lm_p0_free_trailer(LmP0Trailer *trailer) {
    if (trailer == NULL) {
        return;
    }
    lm_p0_free_structure_fields(&trailer->body);
    lm_own_delete(trailer, NULL);
}

static void lm_p0_free_structure_fields(LmP0Structure *structure) {
    LmP0Field *field;

    field = structure->first_field;
    while (field != NULL) {
        LmP0Field *next;

        next = field->next;
        lm_p0_free_node(field->value);
        lm_own_delete(field, NULL);
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

    lm_own_delete(node, NULL);
}

static int lm_p0_relaxed_level_from_column(
    LmP0Document *document,
    LmP0IndentStack *stack,
    size_t column,
    size_t base_level,
    size_t line,
    size_t source_column,
    size_t *out_level
) {
    size_t i;
    size_t parent_level;

    if (column == 0U) {
        *out_level = base_level == 0U ? 0U : 1U;
        return 1;
    }

    parent_level = base_level == 0U ? 0U : base_level - 1U;

    if (stack->count == 0U) {
        if (!lm_p0_indent_stack_push(document, stack, column, line, source_column)) {
            return 0;
        }
        *out_level = base_level;
        return 1;
    }

    if (column > stack->columns[stack->count - 1U]) {
        if (!lm_p0_indent_stack_push(document, stack, column, line, source_column)) {
            return 0;
        }
        *out_level = base_level + stack->count - 1U;
        return 1;
    }

    if (column < stack->columns[0]) {
        *out_level = parent_level;
        return 1;
    }

    while (stack->count > 0U && stack->columns[stack->count - 1U] > column) {
        --stack->count;
    }

    if (stack->count == 0U) {
        *out_level = parent_level;
        return 1;
    }

    for (i = 0U; i < stack->count; ++i) {
        if (stack->columns[i] == column) {
            *out_level = base_level + i;
            return 1;
        }
    }

    lm_p0_set_diagnostic(document, 17, line, source_column, "unindent does not match any outer indentation level");
    return 0;
}

static int lm_p0_source_level_after_line_break(
    LmP0Document *document,
    LmP0IndentStack *indent_stack,
    const char *text,
    size_t length,
    size_t index,
    size_t line,
    size_t column,
    size_t base_level,
    size_t *content_index,
    size_t *out_level
) {
    size_t p;
    size_t indent_column;
    size_t dot_level;

    p = index;
    if (p < length && text[p] == '\r' && p + 1U < length && text[p + 1U] == '\n') {
        p += 2U;
    } else if (p < length && lm_p0_is_line_break(text[p])) {
        ++p;
    }

    indent_column = 0U;
    while (p < length && lm_p0_is_horizontal_space(text[p])) {
        if (text[p] == '\t') {
            indent_column = lm_p0_indent_tab_column(indent_column);
        } else {
            ++indent_column;
        }
        ++p;
    }

    dot_level = 0U;
    while (p < length && text[p] == '.') {
        ++dot_level;
        ++p;
        while (p < length && lm_p0_is_horizontal_space(text[p])) {
            ++p;
        }
    }

    *content_index = p;
    if (dot_level > 0U) {
        *out_level = dot_level;
        return 1;
    }

    {
        size_t diagnostic_line;
        size_t diagnostic_column;

        lm_p0_position_in_slice(text, length, p, line, column, &diagnostic_line, &diagnostic_column);
        return lm_p0_relaxed_level_from_column(
            document,
            indent_stack,
            indent_column,
            base_level,
            diagnostic_line,
            diagnostic_column,
            out_level
        );
    }
}

static int lm_p0_skip_field_space(
    LmP0Document *document,
    LmP0IndentStack *indent_stack,
    const char *text,
    size_t length,
    size_t *index,
    size_t line,
    size_t column,
    unsigned flags,
    size_t short_source_level,
    size_t layout_base_level,
    size_t *current_source_level,
    int *stopped_by_source_level
) {
    *stopped_by_source_level = 0;
    while (*index < length) {
        if (lm_p0_is_horizontal_space(text[*index])) {
            ++(*index);
            continue;
        }

        if (text[*index] == '#') {
            while (*index < length && !lm_p0_is_line_break(text[*index])) {
                ++(*index);
            }
            continue;
        }

        if (text[*index] == '{') {
            if (!lm_p0_skip_brace_mark(document, text, length, index, line, column)) {
                return 0;
            }
            continue;
        }

        if (lm_p0_is_line_break(text[*index])) {
            size_t content_index;
            size_t next_level;

            if (!lm_p0_source_level_after_line_break(
                    document,
                    indent_stack,
                    text,
                    length,
                    *index,
                    line,
                    column,
                    layout_base_level,
                    &content_index,
                    &next_level
                )) {
                return 0;
            }
            if ((flags & LM_P0_FIELD_PARSE_STOP_ON_SOURCE_LEVEL) != 0U && next_level <= short_source_level) {
                *stopped_by_source_level = 1;
                return 1;
            }
            *current_source_level = next_level;
            *index = content_index;
            continue;
        }

        break;
    }
    return 1;
}

static int lm_p0_scan_python_string(
    LmP0Document *document,
    const char *text,
    size_t length,
    size_t *index,
    size_t line,
    size_t base_column
) {
    size_t end;

    if (!lm_p0_find_python_string_end(text, length, *index, &end)) {
        size_t diagnostic_line;
        size_t diagnostic_column;

        lm_p0_position_in_slice(text, length, *index, line, base_column, &diagnostic_line, &diagnostic_column);
        lm_p0_set_diagnostic(document, 4, diagnostic_line, diagnostic_column, "unterminated python-like string literal");
        return 0;
    }

    *index = end;
    return 1;
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

    if (lm_p0_starts_python_string(text, length, *index)) {
        return lm_p0_scan_python_string(document, text, length, index, line, base_column);
    }

    i = *index + 1U;
    while (i < length) {
        if (quote == '"') {
            if (text[i] == '\\') {
                size_t line_break_width;

                line_break_width = i + 1U < length
                    ? lm_p0_line_break_width_at(text, length, i + 1U)
                    : 0U;
                if (line_break_width > 0U) {
                    i += 1U + line_break_width;
                    continue;
                }
                i += i + 1U < length ? 2U : 1U;
                continue;
            }
            if (lm_p0_is_line_break(text[i])) {
                size_t diagnostic_line;
                size_t diagnostic_column;

                lm_p0_position_in_slice(text, length, i, line, base_column, &diagnostic_line, &diagnostic_column);
                lm_p0_set_diagnostic(
                    document,
                    19,
                    diagnostic_line,
                    diagnostic_column,
                    "unescaped newline in string literal"
                );
                return 0;
            }
        }
        if (quote == '`' && text[i] == '\0') {
            size_t diagnostic_line;
            size_t diagnostic_column;

            lm_p0_position_in_slice(text, length, i, line, base_column, &diagnostic_line, &diagnostic_column);
            lm_p0_set_diagnostic(
                document,
                30,
                diagnostic_line,
                diagnostic_column,
                "NUL byte in exact quoted identifier"
            );
            return 0;
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

    {
        size_t diagnostic_line;
        size_t diagnostic_column;

        lm_p0_position_in_slice(text, length, *index, line, base_column, &diagnostic_line, &diagnostic_column);
        lm_p0_set_diagnostic(document, 4, diagnostic_line, diagnostic_column, "unterminated quoted token");
    }
    return 0;
}

static int lm_p0_require_quoted_token_boundary(
    LmP0Document *document,
    const char *text,
    size_t length,
    size_t index,
    size_t line,
    size_t column
) {
    size_t diagnostic_line;
    size_t diagnostic_column;

    if (index >= length || lm_p0_is_quoted_token_boundary(text[index])) {
        return 1;
    }

    lm_p0_position_in_slice(text, length, index, line, column, &diagnostic_line, &diagnostic_column);
    lm_p0_set_diagnostic(
        document,
        18,
        diagnostic_line,
        diagnostic_column,
        "missing separator after quoted token"
    );
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
        if (text[i] == '#') {
            while (i < length && !lm_p0_is_line_break(text[i])) {
                ++i;
            }
            continue;
        }
        if (lm_p0_starts_python_string(text, length, i)) {
            if (!lm_p0_scan_python_string(document, text, length, &i, line, base_column)) {
                return 0;
            }
            continue;
        }
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

    {
        size_t diagnostic_line;
        size_t diagnostic_column;

        lm_p0_position_in_slice(text, length, open_index, line, base_column, &diagnostic_line, &diagnostic_column);
        lm_p0_set_diagnostic(document, 5, diagnostic_line, diagnostic_column, "unclosed parenthesized form");
    }
    return 0;
}

static int lm_p0_find_colon(
    LmP0Document *document,
    const char *text,
    size_t length,
    size_t line,
    size_t column,
    size_t *colon_index
) {
    size_t i;
    size_t depth;

    depth = 0U;
    i = 0U;
    while (i < length) {
        if (text[i] == '{') {
            if (!lm_p0_skip_brace_mark(document, text, length, &i, line, column)) {
                return -1;
            }
            continue;
        }
        if (text[i] == '#') {
            while (i < length && !lm_p0_is_line_break(text[i])) {
                ++i;
            }
            continue;
        }
        if (lm_p0_starts_python_string(text, length, i)) {
            i = lm_p0_skip_python_string_unchecked(text, length, i);
            continue;
        }
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
                if (quote == '`' && text[i] == '\0') {
                    size_t diagnostic_line;
                    size_t diagnostic_column;

                    lm_p0_position_in_slice(text, length, i, line, column, &diagnostic_line, &diagnostic_column);
                    lm_p0_set_diagnostic(
                        document,
                        30,
                        diagnostic_line,
                        diagnostic_column,
                        "NUL byte in exact quoted identifier"
                    );
                    return -1;
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

static int lm_p0_parse_fields_into(
    LmP0Document *document,
    LmP0Structure *structure,
    const char *text,
    size_t length,
    size_t line,
    size_t column,
    size_t offset
);

static int lm_p0_parse_fields_until(
    LmP0Document *document,
    LmP0Structure *structure,
    const char *text,
    size_t length,
    size_t line,
    size_t column,
    size_t offset,
    unsigned flags,
    size_t short_source_level,
    size_t initial_source_level,
    size_t *index
);

static int lm_p0_parse_fields_until_with_layout(
    LmP0Document *document,
    LmP0IndentStack *indent_stack,
    LmP0Structure *structure,
    const char *text,
    size_t length,
    size_t line,
    size_t column,
    size_t offset,
    unsigned flags,
    size_t short_source_level,
    size_t initial_source_level,
    size_t layout_base_level,
    size_t *index
) {
    size_t i;
    size_t current_source_level;

    i = *index;
    current_source_level = initial_source_level;
    while (i < length) {
        size_t start;
        size_t head_end;
        size_t close_index;
        int stopped_by_source_level;
        LmP0Node *node;

        if (!lm_p0_skip_field_space(
                document,
                indent_stack,
                text,
                length,
                &i,
                line,
                column,
                flags,
                short_source_level,
                layout_base_level,
                &current_source_level,
                &stopped_by_source_level
            )) {
            return 0;
        }
        if (stopped_by_source_level) {
            break;
        }
        if (i >= length) {
            break;
        }
        if (text[i] == ',') {
            ++i;
            if ((flags & LM_P0_FIELD_PARSE_STOP_ON_COMMA) != 0U) {
                break;
            }
            continue;
        }
        if (text[i] == ')') {
            break;
        }

        start = i;
        head_end = i;
        node = NULL;

        if (text[i] == '(') {
            size_t inner_index;

            if (!lm_p0_find_matching_paren(document, text, length, i, line, column, &close_index)) {
                return 0;
            }
            node = lm_p0_new_node(document, LM_P0_NODE_STRUCTURE);
            if (node == NULL) {
                return 0;
            }
            inner_index = 0U;
            if (!lm_p0_parse_fields_until(
                    document,
                    &node->as.structure,
                    text + i + 1U,
                    close_index - i - 1U,
                    line,
                    column + i + 1U,
                    offset + i + 1U,
                    0U,
                    0U,
                    current_source_level + 1U,
                    &inner_index
                )) {
                return 0;
            }
            node->span.line = line;
            lm_p0_position_in_slice(text, length, start, line, column, &node->span.line, &node->span.column);
            node->span.offset = offset + i;
            node->span.length = close_index - i + 1U;
            i = close_index + 1U;
        } else {
            int quoted_head;

            quoted_head = 0;
            if (lm_p0_starts_python_string(text, length, i) || text[i] == '"' || text[i] == '`') {
                if (!lm_p0_scan_quoted(document, text, length, &i, text[i], line, column)) {
                    return 0;
                }
                head_end = i;
                quoted_head = 1;
            } else {
                while (i < length &&
                       !lm_p0_is_field_space(text[i]) &&
                       text[i] != ',' &&
                       text[i] != '(' &&
                       text[i] != ')' &&
                       text[i] != '#' &&
                       text[i] != '{' &&
                       text[i] != ':') {
                    if (lm_p0_starts_python_string(text, length, i) || text[i] == '"' || text[i] == '`') {
                        size_t diagnostic_line;
                        size_t diagnostic_column;

                        lm_p0_position_in_slice(text, length, i, line, column, &diagnostic_line, &diagnostic_column);
                        lm_p0_set_diagnostic(document, 18, diagnostic_line, diagnostic_column, "missing separator before quoted token");
                        return 0;
                    }
                    ++i;
                }
                head_end = i;
            }

            if (i < length && text[i] == ':' && head_end > start) {
                size_t body_index;
                unsigned body_flags;
                int has_inline_body;

                node = lm_p0_new_node(document, LM_P0_NODE_FRAME);
                if (node == NULL) {
                    return 0;
                }
                node->as.frame.head.data = text + start;
                node->as.frame.head.length = head_end - start;
                node->as.frame.flags = LM_P0_FRAME_COLON;
                ++i;
                while (i < length && lm_p0_is_horizontal_space(text[i])) {
                    ++i;
                }
                body_index = i;
                has_inline_body = i < length &&
                    !lm_p0_is_line_break(text[i]) &&
                    text[i] != ',' &&
                    text[i] != ')' &&
                    text[i] != '#';
                if (has_inline_body) {
                    node->as.frame.flags |= LM_P0_FRAME_INLINE_BODY;
                }
                body_flags = LM_P0_FIELD_PARSE_STOP_ON_COMMA | LM_P0_FIELD_PARSE_STOP_ON_SOURCE_LEVEL;
                if (!lm_p0_parse_fields_until_with_layout(
                        document,
                        indent_stack,
                        &node->as.frame.body,
                        text,
                        length,
                        line,
                        column,
                        offset,
                        body_flags,
                        current_source_level,
                        current_source_level + 1U,
                        layout_base_level,
                        &body_index
                    )) {
                    return 0;
                }
                node->span.line = line;
                lm_p0_position_in_slice(text, length, start, line, column, &node->span.line, &node->span.column);
                node->span.offset = offset + start;
                node->span.length = body_index - start;
                i = body_index;
            } else if (i < length && text[i] == '(' && head_end > start) {
                size_t inner_index;

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
                inner_index = 0U;
                if (!lm_p0_parse_fields_until(
                        document,
                        &node->as.frame.body,
                        text + i + 1U,
                        close_index - i - 1U,
                        line,
                        column + i + 1U,
                        offset + i + 1U,
                        0U,
                        0U,
                        current_source_level + 1U,
                        &inner_index
                    )) {
                    return 0;
                }
                node->span.line = line;
                lm_p0_position_in_slice(text, length, start, line, column, &node->span.line, &node->span.column);
                node->span.offset = offset + start;
                node->span.length = close_index - start + 1U;
                i = close_index + 1U;
            } else {
                if (head_end == start) {
                    size_t diagnostic_line;
                    size_t diagnostic_column;

                    lm_p0_position_in_slice(text, length, start, line, column, &diagnostic_line, &diagnostic_column);
                    lm_p0_set_diagnostic(document, 6, diagnostic_line, diagnostic_column, "unexpected character in field list");
                    return 0;
                }
                if (quoted_head && !lm_p0_require_quoted_token_boundary(document, text, length, head_end, line, column)) {
                    return 0;
                }
                if (!quoted_head) {
                    i = head_end;
                    if (!lm_p0_append_compact_atom_pieces(
                            document,
                            structure,
                            text,
                            length,
                            line,
                            column,
                            offset,
                            start,
                            head_end
                        )) {
                        return 0;
                    }
                    continue;
                }
                node = lm_p0_new_node(document, LM_P0_NODE_ATOM);
                if (node == NULL) {
                    return 0;
                }
                node->as.atom.data = text + start;
                node->as.atom.length = head_end - start;
                node->span.line = line;
                lm_p0_position_in_slice(text, length, start, line, column, &node->span.line, &node->span.column);
                node->span.offset = offset + start;
                node->span.length = head_end - start;
                i = head_end;
            }
        }

        if (!lm_p0_append_field(document, structure, node)) {
            return 0;
        }
    }

    *index = i;
    return 1;
}

static int lm_p0_parse_fields_until(
    LmP0Document *document,
    LmP0Structure *structure,
    const char *text,
    size_t length,
    size_t line,
    size_t column,
    size_t offset,
    unsigned flags,
    size_t short_source_level,
    size_t initial_source_level,
    size_t *index
) {
    LmP0IndentStack *indent_stack;
    int status;

    indent_stack = lm_p0_indent_stack_new_empty();
    if (indent_stack == NULL) {
        lm_p0_set_diagnostic(document, 1, line, column, "out of memory while creating indentation stack");
        return 0;
    }
    status = lm_p0_parse_fields_until_with_layout(
        document,
        indent_stack,
        structure,
        text,
        length,
        line,
        column,
        offset,
        flags,
        short_source_level,
        initial_source_level,
        initial_source_level,
        index
    );
    lm_p0_indent_stack_delete(indent_stack);
    return status;
}

static int lm_p0_parse_fields_into(
    LmP0Document *document,
    LmP0Structure *structure,
    const char *text,
    size_t length,
    size_t line,
    size_t column,
    size_t offset
) {
    size_t index;

    index = 0U;
    return lm_p0_parse_fields_until(document, structure, text, length, line, column, offset, 0U, 0U, 0U, &index);
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

static int lm_p0_stack_level_is_trailer_body(const LmP0Stack *stack, size_t level) {
    LmP0Node *owner;

    if (level >= stack->capacity || stack->parents[level] == NULL) {
        return 0;
    }

    owner = stack->owners[level];
    if (owner == NULL) {
        return 0;
    }

    if (owner->kind == LM_P0_NODE_FRAME &&
        owner->as.frame.trailer != NULL &&
        stack->parents[level] == &owner->as.frame.trailer->body) {
        return 1;
    }

    if (owner->kind == LM_P0_NODE_STRUCTURE &&
        owner->as.structure.trailer != NULL &&
        stack->parents[level] == &owner->as.structure.trailer->body) {
        return 1;
    }

    return 0;
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
    if (allow_bare && (lm_p0_is_horizontal_space(text[name_length]) || text[name_length] == '#')) {
        return 1;
    }
    return text[name_length] == ':';
}

static LmP0TrailerRole lm_p0_trailer_role(const char *text, size_t length) {
    if (lm_p0_dash_fence_status_after_comment_trim(text, length, NULL) == LM_P0_DASH_FENCE_VALID) {
        return LM_P0_TRAILER_ROLE_DASH_CUTTER;
    }
    if (lm_p0_text_has_prefix_name(text, length, "end", 0)) {
        return LM_P0_TRAILER_ROLE_END;
    }
    if (lm_p0_text_has_prefix_name(text, length, "return", 1)) {
        return LM_P0_TRAILER_ROLE_RETURN;
    }
    if (lm_p0_text_has_prefix_name(text, length, "until", 0)) {
        return LM_P0_TRAILER_ROLE_UNTIL;
    }
    return LM_P0_TRAILER_ROLE_NONE;
}

static int lm_p0_trailer_role_is_tail_cutter(LmP0TrailerRole role) {
    return role != LM_P0_TRAILER_ROLE_NONE;
}

static int lm_p0_node_head_is(const LmP0Node *node, const char *name) {
    size_t name_length;

    if (node == NULL || node->kind != LM_P0_NODE_FRAME) {
        return 0;
    }

    name_length = strlen(name);
    return node->as.frame.head.length == name_length &&
        memcmp(node->as.frame.head.data, name, name_length) == 0;
}

static int lm_p0_trailer_role_accepts_target(LmP0TrailerRole role, const LmP0Node *target) {
    if (role == LM_P0_TRAILER_ROLE_RETURN) {
        return lm_p0_node_head_is(target, "fn");
    }
    return lm_p0_trailer_role_is_tail_cutter(role);
}

static int lm_p0_stream_event_is_tail_cutter(const LmP0StreamEvent *event) {
    return event->kind == LM_P0_STREAM_EVENT_ITEM &&
        lm_p0_trailer_role_is_tail_cutter(lm_p0_trailer_role(event->text, event->text_length));
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

static void lm_p0_stack_free_any(void *object) {
    lm_p0_stack_free((LmP0Stack *)object);
}

static LmP0Stack *lm_p0_stack_new(void) {
    return (LmP0Stack *)calloc(1U, sizeof(LmP0Stack));
}

static void lm_p0_stack_delete(LmP0Stack *stack) {
    lm_own_delete(stack, lm_p0_stack_free_any);
}

static LmP0PendingDelimiter *lm_p0_pending_delimiter_new(void) {
    return (LmP0PendingDelimiter *)calloc(1U, sizeof(LmP0PendingDelimiter));
}

static LmP0StreamEvent *lm_p0_stream_event_new_copy(const LmP0StreamEvent *event) {
    LmP0StreamEvent *copy;

    if (event == NULL) {
        return NULL;
    }
    copy = (LmP0StreamEvent *)calloc(1U, sizeof(*copy));
    if (copy != NULL) {
        *copy = *event;
    }
    return copy;
}

static void lm_p0_stream_event_delete(LmP0StreamEvent *event) {
    lm_own_delete(event, NULL);
}

static void lm_p0_pending_delimiter_clear(LmP0PendingDelimiter *pending) {
    if (pending != NULL) {
        lm_p0_stream_event_delete(pending->event);
        pending->event = NULL;
        pending->active = 0;
    }
}

static int lm_p0_pending_delimiter_set(
    LmP0Document *document,
    LmP0PendingDelimiter *pending,
    const LmP0StreamEvent *event
) {
    if (pending == NULL) {
        return 0;
    }

    lm_p0_pending_delimiter_clear(pending);
    pending->event = lm_p0_stream_event_new_copy(event);
    if (pending->event == NULL) {
        lm_p0_set_diagnostic(document, 1, event != NULL ? event->line : 0U, event != NULL ? event->column : 0U, "out of memory while storing pending delimiter");
        return 0;
    }
    pending->active = 1;
    return 1;
}

static void lm_p0_pending_delimiter_delete(LmP0PendingDelimiter *pending) {
    if (pending != NULL) {
        lm_p0_pending_delimiter_clear(pending);
        lm_own_delete(pending, NULL);
    }
}

static LmP0DisabledState *lm_p0_disabled_state_new(size_t base_level) {
    LmP0DisabledState *state;

    state = (LmP0DisabledState *)calloc(1U, sizeof(*state));
    if (state != NULL) {
        state->base_level = base_level;
        state->top_level = base_level;
        state->pending_item = 1;
        state->pending_level = base_level;
    }
    return state;
}

static void lm_p0_disabled_state_delete(LmP0DisabledState *state) {
    lm_own_delete(state, NULL);
}

static int lm_p0_node_keeps_source_child_level(LmP0Node *node) {
    if (node->kind == LM_P0_NODE_FRAME) {
        return (node->as.frame.flags & LM_P0_FRAME_COLON) != 0U;
    }
    return 0;
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

static LmP0Node *lm_p0_structure_last_colon_frame(LmP0Structure *structure) {
    LmP0Node *node;

    if (structure->last_field == NULL || structure->last_field->value == NULL) {
        return NULL;
    }

    node = structure->last_field->value;
    if (node->kind == LM_P0_NODE_FRAME &&
        (node->as.frame.flags & LM_P0_FRAME_COLON) != 0U) {
        return node;
    }

    return NULL;
}

static int lm_p0_stack_install_node_lineage(
    LmP0Document *document,
    LmP0Stack *stack,
    size_t base_level,
    LmP0Node *node
) {
    LmP0Node *owner;
    size_t level;

    if (!lm_p0_node_keeps_source_child_level(node)) {
        if (!lm_p0_stack_ensure(document, stack, base_level + 1U)) {
            return 0;
        }
        stack->parents[base_level + 1U] = NULL;
        stack->owners[base_level + 1U] = NULL;
        stack->hard[base_level + 1U] = 0U;
        lm_p0_stack_truncate_deeper(stack, base_level + 1U);
        return 1;
    }

    owner = node;
    level = base_level + 1U;
    while (lm_p0_node_keeps_source_child_level(owner)) {
        LmP0Structure *body;
        LmP0Node *next_owner;

        if (!lm_p0_stack_ensure(document, stack, level)) {
            return 0;
        }

        body = lm_p0_node_child_structure(owner);
        stack->parents[level] = body;
        stack->owners[level] = owner;
        stack->hard[level] = body->field_count == 0U ? 1U : 0U;

        next_owner = lm_p0_structure_last_colon_frame(body);
        if (next_owner == NULL) {
            break;
        }

        owner = next_owner;
        ++level;
    }

    lm_p0_stack_truncate_deeper(stack, level);
    return 1;
}

static int lm_p0_stack_ensure_root_level_alias(LmP0Document *document, LmP0Stack *stack, size_t level) {
    if (level != 1U || stack->parents[1] != NULL) {
        return 1;
    }
    if (!lm_p0_stack_ensure(document, stack, 1U)) {
        return 0;
    }
    stack->parents[1] = stack->parents[0];
    stack->owners[1] = stack->owners[0];
    stack->hard[1] = 1U;
    return 1;
}

static int lm_p0_stack_open_implicit_anonymous(
    LmP0Document *document,
    LmP0Stack *stack,
    size_t parent_level,
    size_t line,
    size_t column,
    size_t offset
) {
    LmP0Node *anonymous_node;
    LmP0Structure *parent;

    parent = stack->parents[parent_level];
    if (parent == NULL) {
        lm_p0_set_diagnostic(document, 8, line, column, "source level has no open parent structure");
        return 0;
    }

    anonymous_node = lm_p0_new_node(document, LM_P0_NODE_STRUCTURE);
    if (anonymous_node == NULL) {
        return 0;
    }
    anonymous_node->span.line = line;
    anonymous_node->span.column = column;
    anonymous_node->span.offset = offset;
    anonymous_node->span.length = 0U;

    if (!lm_p0_append_field(document, parent, anonymous_node)) {
        lm_p0_free_node(anonymous_node);
        return 0;
    }

    if (!lm_p0_stack_ensure(document, stack, parent_level + 1U)) {
        return 0;
    }
    stack->parents[parent_level + 1U] = &anonymous_node->as.structure;
    stack->owners[parent_level + 1U] = anonymous_node;
    stack->hard[parent_level + 1U] = 1U;
    lm_p0_stack_truncate_deeper(stack, parent_level + 1U);
    return 1;
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

    {
        int colon_status;

        colon_status = lm_p0_find_colon(document, text, length, line, column, &colon_index);
        if (colon_status < 0) {
            return 0;
        }
        if (colon_status == 0) {
            colon_index = length;
        }
    }

    if (colon_index < length) {
        spelling_length = colon_index;
        while (spelling_length > 0U && lm_p0_is_horizontal_space(text[spelling_length - 1U])) {
            --spelling_length;
        }
        body_start = colon_index + 1U;
        while (body_start < length && lm_p0_is_horizontal_space(text[body_start])) {
            ++body_start;
        }
    } else {
        body_start = 0U;
        if (body_start < length &&
            (lm_p0_starts_python_string(text, length, body_start) || text[body_start] == '"' || text[body_start] == '`')) {
            if (!lm_p0_scan_quoted(document, text, length, &body_start, text[body_start], line, column)) {
                return 0;
            }
            spelling_length = body_start;
        } else {
            while (body_start < length &&
                   !lm_p0_is_horizontal_space(text[body_start]) &&
                   text[body_start] != ',' &&
                   text[body_start] != '#') {
                ++body_start;
            }
            spelling_length = body_start;
        }
        while (body_start < length && lm_p0_is_horizontal_space(text[body_start])) {
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

static int lm_p0_stream_resolve_pending_delimiter(
    LmP0Document *document,
    LmP0Stack *stack,
    LmP0PendingDelimiter *pending,
    size_t next_level
) {
    const LmP0StreamEvent *event;
    size_t top_level;
    LmP0Structure *parent;

    if (!pending->active || pending->event == NULL) {
        return 1;
    }

    event = pending->event;
    if (!lm_p0_stack_ensure(document, stack, event->level + 1U)) {
        return 0;
    }

    top_level = lm_p0_stack_collapse_soft_to_event(stack, event->level);
    if (event->level == top_level && stack->hard[top_level] == 0U) {
        stack->hard[top_level] = 1U;
    }

    if (event->level < top_level) {
        lm_p0_stack_truncate_deeper(stack, event->level);
        top_level = lm_p0_stack_top_level(stack);
    }

    parent = stack->parents[event->level];
    if (parent == NULL && event->level == 1U) {
        parent = stack->parents[0];
    }
    if (parent == NULL) {
        lm_p0_set_diagnostic(document, 8, event->line, event->column, "source level has no open parent structure");
        return 0;
    }

    if (next_level == event->level + 1U) {
        LmP0Node *anonymous_node;

        anonymous_node = lm_p0_new_node(document, LM_P0_NODE_STRUCTURE);
        if (anonymous_node == NULL) {
            return 0;
        }
        anonymous_node->span.line = event->line;
        anonymous_node->span.column = event->column;
        anonymous_node->span.offset = event->offset;
        anonymous_node->span.length = event->text_length;

        if (!lm_p0_append_field(document, parent, anonymous_node)) {
            lm_p0_free_node(anonymous_node);
            return 0;
        }
        stack->parents[event->level + 1U] = &anonymous_node->as.structure;
        stack->owners[event->level + 1U] = anonymous_node;
        stack->hard[event->level + 1U] = 1U;
        lm_p0_stack_truncate_deeper(stack, event->level + 1U);
    } else {
        lm_p0_stack_truncate_deeper(stack, event->level);
    }

    lm_p0_pending_delimiter_clear(pending);
    return 1;
}

static int lm_p0_stream_apply_item_event(
    LmP0Document *document,
    LmP0Stack *stack,
    const LmP0StreamEvent *event
) {
    LmP0TrailerRole trailer_role;
    size_t top_level;
    LmP0Structure *parent;
    LmP0Node *node;
    int trailer_target_available;
    int trailer_target_accepted;

    if (!lm_p0_stack_ensure(document, stack, event->level + 1U)) {
        return 0;
    }

    if (event->kind == LM_P0_STREAM_EVENT_ITEM &&
        !lm_p0_validate_dash_fence_line(document, event->text, event->text_length, event->line, event->column)) {
        return 0;
    }

    trailer_role = event->kind == LM_P0_STREAM_EVENT_ITEM
        ? lm_p0_trailer_role(event->text, event->text_length)
        : LM_P0_TRAILER_ROLE_NONE;
    top_level = lm_p0_stack_top_level(stack);
    trailer_target_available = lm_p0_trailer_role_is_tail_cutter(trailer_role) &&
        event->level + 1U <= top_level &&
        stack->owners[event->level + 1U] != NULL;
    trailer_target_accepted = trailer_target_available &&
        lm_p0_trailer_role_accepts_target(trailer_role, stack->owners[event->level + 1U]);

    if (!lm_p0_trailer_role_is_tail_cutter(trailer_role) ||
        !trailer_target_accepted) {
        top_level = lm_p0_stack_collapse_soft_to_event(stack, event->level);
        if (event->level == top_level && stack->hard[top_level] == 0U) {
            stack->hard[top_level] = 1U;
        }
        trailer_target_available = lm_p0_trailer_role_is_tail_cutter(trailer_role) &&
            event->level + 1U <= top_level &&
            stack->owners[event->level + 1U] != NULL;
        trailer_target_accepted = trailer_target_available &&
            lm_p0_trailer_role_accepts_target(trailer_role, stack->owners[event->level + 1U]);
    }

    if (lm_p0_trailer_role_is_tail_cutter(trailer_role) &&
        event->level + 1U < top_level &&
        !trailer_target_accepted) {
        lm_p0_set_diagnostic(document, 13, event->line, event->column, "tail-cutter target is not valid for this receiver");
        return 0;
    }

    if (lm_p0_trailer_role_is_tail_cutter(trailer_role) &&
        trailer_target_accepted) {
        LmP0Structure *trailer_body;
        LmP0Node *target;
        size_t target_level;

        target_level = event->level + 1U;
        target = stack->owners[target_level];
        lm_p0_stack_truncate_deeper(stack, target_level);

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
            return 0;
        }

        stack->parents[target_level] = trailer_body;
        stack->owners[target_level] = target;
        stack->hard[target_level] = 0U;
        if (target_level > 0U) {
            stack->hard[target_level - 1U] = 0U;
        }
        lm_p0_stack_truncate_deeper(stack, target_level);
        return 1;
    }

    if (event->level < top_level) {
        lm_p0_stack_truncate_deeper(stack, event->level);
        top_level = lm_p0_stack_top_level(stack);
    }

    if (stack->parents[event->level] == NULL && event->level > top_level) {
        if (event->level != top_level + 1U) {
            lm_p0_set_diagnostic(document, 13, event->line, event->column, "source level jumps too deep");
            return 0;
        }
        if (!lm_p0_stack_open_implicit_anonymous(
                document,
                stack,
                top_level,
                event->line,
                event->column,
                event->offset
            )) {
            return 0;
        }
    }

    if (!lm_p0_stack_ensure_root_level_alias(document, stack, event->level)) {
        return 0;
    }

    parent = stack->parents[event->level];
    if (parent == NULL) {
        lm_p0_set_diagnostic(document, 8, event->line, event->column, "source level has no open parent structure");
        return 0;
    }

    if (event->kind == LM_P0_STREAM_EVENT_ITEM) {
        LmP0Field *previous_last;
        LmP0Field *field;

        previous_last = parent->last_field;
        if (!lm_p0_parse_fields_into(
                document,
                parent,
                event->text,
                event->text_length,
                event->line,
                event->column,
                event->offset
            )) {
            return 0;
        }

        field = previous_last != NULL ? previous_last->next : parent->first_field;
        while (field != NULL) {
            if (field->value != NULL) {
                field->value->flags |= event->node_flags;
            }
            field = field->next;
        }

        if (parent->last_field == previous_last || parent->last_field == NULL) {
            return 1;
        }
        return lm_p0_stack_install_node_lineage(document, stack, event->level, parent->last_field->value);
    }

    if (event->kind == LM_P0_STREAM_EVENT_DISABLED_BLOCK) {
        node = lm_p0_new_node(document, LM_P0_NODE_DISABLED);
        if (node == NULL) {
            return 0;
        }
        node->as.atom.data = event->text;
        node->as.atom.length = event->text_length;
        node->span.line = event->line;
        node->span.column = event->column;
        node->span.offset = event->offset;
        node->span.length = event->text_length;
    } else if (event->kind == LM_P0_STREAM_EVENT_BLOCK_STRING) {
        node = lm_p0_new_node(document, LM_P0_NODE_ATOM);
        if (node == NULL) {
            return 0;
        }
        node->as.atom.data = event->text;
        node->as.atom.length = event->text_length;
        node->span.line = event->line;
        node->span.column = event->column;
        node->span.offset = event->offset;
        node->span.length = event->text_length;
    }
    node->flags |= event->node_flags;

    if (!lm_p0_append_field(document, parent, node)) {
        lm_p0_free_node(node);
        return 0;
    }

    if (!lm_p0_stack_install_node_lineage(document, stack, event->level, node)) {
        return 0;
    }
    return 1;
}

static int lm_p0_stream_apply_event(
    LmP0Document *document,
    LmP0Stack *stack,
    LmP0PendingDelimiter *pending,
    const LmP0StreamEvent *event
) {
    if (event->kind == LM_P0_STREAM_EVENT_DELIM) {
        if (!lm_p0_stream_resolve_pending_delimiter(document, stack, pending, event->level)) {
            return 0;
        }
        return lm_p0_pending_delimiter_set(document, pending, event);
    }

    if (!lm_p0_stream_resolve_pending_delimiter(document, stack, pending, event->level)) {
        return 0;
    }
    if (event->kind == LM_P0_STREAM_EVENT_ITEM &&
        lm_p0_dash_fence_status_after_comment_trim(event->text, event->text_length, NULL) == LM_P0_DASH_FENCE_VALID) {
        if (!lm_p0_pending_delimiter_set(document, pending, event)) {
            return 0;
        }
        pending->event->kind = LM_P0_STREAM_EVENT_DELIM;
        return 1;
    }
    return lm_p0_stream_apply_item_event(document, stack, event);
}

static size_t lm_p0_stream_block_string_level(
    const LmP0Stack *stack,
    const LmP0PendingDelimiter *pending
) {
    size_t level;

    if (pending->active && pending->event != NULL) {
        return pending->event->level + 1U;
    }

    level = lm_p0_stack_top_level(stack);
    while (level > 0U) {
        if (stack->parents[level] != NULL && !lm_p0_stack_level_is_trailer_body(stack, level)) {
            return level;
        }
        --level;
    }

    return 0U;
}

static int lm_p0_validate_disabled_item_text(
    LmP0Document *document,
    const char *text,
    size_t length,
    size_t line,
    size_t column
) {
    size_t i;

    i = 0U;
    if (!lm_p0_validate_dash_fence_line(document, text, length, line, column)) {
        return 0;
    }
    while (i < length) {
        if (text[i] == '{') {
            if (!lm_p0_skip_brace_mark(document, text, length, &i, line, column)) {
                return 0;
            }
            continue;
        }
        if (text[i] == '#') {
            while (i < length && !lm_p0_is_line_break(text[i])) {
                ++i;
            }
            continue;
        }
        if (lm_p0_starts_python_string(text, length, i)) {
            if (!lm_p0_scan_python_string(document, text, length, &i, line, column)) {
                return 0;
            }
            continue;
        }
        if (text[i] == '"' || text[i] == '`') {
            if (!lm_p0_scan_quoted(document, text, length, &i, text[i], line, column)) {
                return 0;
            }
            continue;
        }
        if (text[i] == '(') {
            size_t close_index;

            if (!lm_p0_find_matching_paren(document, text, length, i, line, column, &close_index)) {
                return 0;
            }
            i = close_index + 1U;
            continue;
        }
        if (text[i] == ')') {
            size_t diagnostic_line;
            size_t diagnostic_column;

            lm_p0_position_in_slice(text, length, i, line, column, &diagnostic_line, &diagnostic_column);
            lm_p0_set_diagnostic(document, 6, diagnostic_line, diagnostic_column, "unmatched closing parenthesis");
            return 0;
        }
        ++i;
    }
    return 1;
}

static int lm_p0_disabled_scan_next_event(
    LmP0Document *document,
    LmP0IndentStack *indent_stack,
    size_t *offset,
    size_t *line,
    LmP0StreamEvent *event,
    int *has_event
) {
    const char *source;
    size_t length;

    source = document->source;
    length = document->source_length;
    *has_event = 0;

    while (*offset <= length) {
        size_t line_start;
        size_t line_end;
        size_t raw_length;
        size_t p;
        size_t level;
        int dotted_level;
        const char *text;
        size_t text_length;

        line_start = *offset;
        if (line_start >= length) {
            return 1;
        }

        if (lm_p0_scan_raw_comment_block(
                document,
                source,
                length,
                line_start,
                *line,
                offset,
                line
            )) {
            continue;
        }
        if (document->diagnostic.code != 0) {
            return 0;
        }

        if (lm_p0_scan_block_string_event(
                document,
                source,
                length,
                line_start,
                *line,
                event,
                offset,
                line
            )) {
            event->level = (size_t)-1;
            *has_event = 1;
            return 1;
        }
        if (document->diagnostic.code != 0) {
            return 0;
        }

        line_end = lm_p0_find_layout_line_end(source, length, line_start);
        raw_length = line_end - line_start;
        if (raw_length > 0U && source[line_start + raw_length - 1U] == '\r') {
            --raw_length;
        }

        if (raw_length == 0U) {
            lm_p0_advance_layout_line(source, length, line_start, line_end, offset, line);
            continue;
        }

        p = line_start;
        level = 0U;
        dotted_level = 0;
        if (*line == 1U && raw_length >= 3U &&
            (unsigned char)source[p] == 0xEFU &&
            (unsigned char)source[p + 1U] == 0xBBU &&
            (unsigned char)source[p + 2U] == 0xBFU) {
            p += 3U;
        }

        if (p < line_start + raw_length && source[p] == '.') {
            dotted_level = 1;
            while (p < line_start + raw_length && source[p] == '.') {
                ++level;
                ++p;
                while (p < line_start + raw_length && lm_p0_is_horizontal_space(source[p])) {
                    ++p;
                }
            }
        } else {
            size_t indent_column;
            size_t mark_skip;

            lm_p0_scan_indent_column(source, p, line_start + raw_length, &p, &indent_column);
            text = source + p;
            text_length = (line_start + raw_length) - p;
            lm_p0_trim_right(&text, &text_length);
            mark_skip = 0U;
            if (!lm_p0_skip_leading_brace_marks(
                    document,
                    text,
                    text_length,
                    *line,
                    (size_t)(text - (source + line_start)) + 1U,
                    &mark_skip
                )) {
                return 0;
            }
            p += mark_skip;
            text = source + p;
            text_length = (line_start + raw_length) - p;
            lm_p0_trim_right(&text, &text_length);

            if (text_length == 0U || text[0] == '#') {
                lm_p0_advance_layout_line(source, length, line_start, line_end, offset, line);
                continue;
            }

            if (!lm_p0_indent_level_from_column(
                    document,
                    indent_stack,
                    indent_column,
                    *line,
                    (size_t)(text - (source + line_start)) + 1U,
                    &level
                )) {
                return 0;
            }
        }

        text = source + p;
        text_length = (line_start + raw_length) - p;
        lm_p0_trim_right(&text, &text_length);
        {
            size_t mark_skip;

            mark_skip = 0U;
            if (!lm_p0_skip_leading_brace_marks(
                    document,
                    text,
                    text_length,
                    *line,
                    (size_t)(text - (source + line_start)) + 1U,
                    &mark_skip
                )) {
                return 0;
            }
            p += mark_skip;
            text = source + p;
            text_length = (line_start + raw_length) - p;
            lm_p0_trim_right(&text, &text_length);
        }

        if (text_length == 0U || text[0] == '#') {
            if (dotted_level && level > 0U) {
                text_length = 0U;
            } else {
                lm_p0_advance_layout_line(source, length, line_start, line_end, offset, line);
                continue;
            }
        }

        memset(event, 0, sizeof(*event));
        event->level = level;
        event->text = text;
        event->text_length = text_length;
        event->line = *line;
        event->column = (size_t)(text - (source + line_start)) + 1U;
        event->offset = (size_t)(text - source);
        event->kind = text_length == 0U && level > 0U
            ? LM_P0_STREAM_EVENT_DELIM
            : LM_P0_STREAM_EVENT_ITEM;

        lm_p0_advance_layout_line(source, length, line_start, line_end, offset, line);
        *has_event = 1;
        return 1;
    }

    return 1;
}

static int lm_p0_disabled_event_is_tail_cutter(const LmP0StreamEvent *event) {
    if (event->kind == LM_P0_STREAM_EVENT_DELIM) {
        return 1;
    }
    return lm_p0_stream_event_is_tail_cutter(event);
}

static int lm_p0_disabled_state_accept_event(
    LmP0Document *document,
    LmP0DisabledState *state,
    LmP0StreamEvent *event,
    int *done_after_event,
    int *done_before_event
) {
    int is_tail_cutter;

    *done_after_event = 0;
    *done_before_event = 0;

    if (event->level == (size_t)-1) {
        event->level = state->pending_item
            ? state->pending_level + 1U
            : state->top_level;
    }

    if (event->kind == LM_P0_STREAM_EVENT_ITEM &&
        !lm_p0_validate_disabled_item_text(
            document,
            event->text,
            event->text_length,
            event->line,
            event->column
        )) {
        return 0;
    }

    if (state->pending_item) {
        if (event->level == state->pending_level + 1U) {
            state->body_started = 1;
            state->top_level = event->level;
            state->pending_item = 0;
        } else if (event->level > state->pending_level + 1U) {
            lm_p0_set_diagnostic(document, 13, event->line, event->column, "disabled block source level jumps too deep");
            return 0;
        } else {
            state->pending_item = 0;
        }
    }

    is_tail_cutter = lm_p0_disabled_event_is_tail_cutter(event);
    if (!state->body_started) {
        if (event->level == state->base_level && is_tail_cutter) {
            *done_after_event = 1;
            return 1;
        }
        if (event->level <= state->base_level) {
            *done_before_event = 1;
            return 1;
        }
        lm_p0_set_diagnostic(document, 8, event->line, event->column, "disabled block source level has no open parent");
        return 0;
    }

    if (is_tail_cutter && event->level >= state->base_level && event->level + 1U <= state->top_level) {
        state->top_level = event->level;
        state->pending_item = 0;
        if (event->level == state->base_level) {
            *done_after_event = 1;
        }
        return 1;
    }

    if (event->level + 1U < state->top_level) {
        lm_p0_set_diagnostic(
            document,
            13,
            event->line,
            event->column,
            "disabled block source level decrease must be one step unless a tail cutter is used"
        );
        return 0;
    }

    if (event->level <= state->base_level) {
        *done_before_event = 1;
        return 1;
    }
    if (event->level < state->top_level) {
        state->top_level = event->level;
    }
    if (event->level > state->top_level + 1U) {
        lm_p0_set_diagnostic(document, 8, event->line, event->column, "disabled block source level has no open parent");
        return 0;
    }
    if (event->level == state->top_level + 1U) {
        state->top_level = event->level;
    }

    state->pending_item = 1;
    state->pending_level = event->level;
    return 1;
}

static int lm_p0_validate_disabled_block(
    LmP0Document *document,
    const LmP0IndentStack *indent_stack,
    size_t first_next_offset,
    size_t first_next_line,
    size_t base_level,
    const char *header_text,
    size_t header_length,
    size_t header_line,
    size_t header_column,
    size_t *out_offset,
    size_t *out_line
) {
    LmP0IndentStack *local_indent;
    LmP0DisabledState *state;
    size_t offset;
    size_t line;
    int status;

    if (!lm_p0_validate_disabled_item_text(document, header_text, header_length, header_line, header_column)) {
        return 0;
    }
    local_indent = lm_p0_indent_stack_clone(document, indent_stack, header_line, header_column);
    if (local_indent == NULL) {
        return 0;
    }

    state = lm_p0_disabled_state_new(base_level);
    if (state == NULL) {
        lm_p0_set_diagnostic(document, 1, header_line, header_column, "out of memory while creating disabled block state");
        lm_p0_indent_stack_delete(local_indent);
        return 0;
    }

    offset = first_next_offset;
    line = first_next_line;
    status = 1;

    while (status && offset <= document->source_length) {
        LmP0StreamEvent event;
        size_t event_offset;
        size_t event_line;
        int has_event;
        int done_after_event;
        int done_before_event;

        event_offset = offset;
        event_line = line;
        if (!lm_p0_disabled_scan_next_event(document, local_indent, &offset, &line, &event, &has_event)) {
            status = 0;
            break;
        }
        if (!has_event) {
            if (state->body_started && state->top_level > state->base_level) {
                lm_p0_set_diagnostic(document, 22, header_line, header_column, "unterminated disabled block");
                status = 0;
            }
            break;
        }

        if (!lm_p0_disabled_state_accept_event(
                document,
                state,
                &event,
                &done_after_event,
                &done_before_event
            )) {
            status = 0;
            break;
        }
        if (done_before_event) {
            offset = event_offset;
            line = event_line;
            break;
        }
        if (done_after_event) {
            break;
        }
    }

    lm_p0_disabled_state_delete(state);
    lm_p0_indent_stack_delete(local_indent);
    *out_offset = offset;
    *out_line = line;
    return status && document->diagnostic.code == 0;
}

static int lm_p0_parse_stream(LmP0Document *document) {
    const char *source;
    size_t length;
    size_t offset;
    size_t line;
    LmP0IndentStack *indent_stack;
    LmP0Stack *stack;
    LmP0PendingDelimiter *pending;
    int status;
    int has_last_physical_level;
    size_t last_physical_level;

    source = document->source;
    length = document->source_length;
    offset = 0U;
    line = 1U;
    status = 1;
    has_last_physical_level = 0;
    last_physical_level = 0U;
    indent_stack = NULL;
    stack = NULL;
    pending = NULL;

    document->root = lm_p0_new_node(document, LM_P0_NODE_STRUCTURE);
    if (document->root == NULL) {
        return 0;
    }

    indent_stack = lm_p0_indent_stack_new(document);
    stack = lm_p0_stack_new();
    pending = lm_p0_pending_delimiter_new();
    if (indent_stack == NULL ||
        stack == NULL ||
        pending == NULL ||
        !lm_p0_stack_ensure(document, stack, 0U)) {
        if (pending == NULL) {
            lm_p0_set_diagnostic(document, 1, 0U, 0U, "out of memory while creating pending delimiter");
        }
        lm_p0_indent_stack_delete(indent_stack);
        lm_p0_stack_delete(stack);
        lm_p0_pending_delimiter_delete(pending);
        return 0;
    }
    stack->parents[0] = &document->root->as.structure;
    stack->owners[0] = document->root;

    while (offset <= length) {
        size_t line_start;
        size_t line_end;
        size_t raw_length;
        size_t p;
        size_t level;
        int dotted_level;
        const char *text;
        size_t text_length;
        unsigned node_flags;
        LmP0StreamEvent event;

        line_start = offset;
        if (lm_p0_scan_raw_comment_block(
                document,
                source,
                length,
                line_start,
                line,
                &offset,
                &line
            )) {
            continue;
        }
        if (document->diagnostic.code != 0) {
            status = 0;
            break;
        }

        if (lm_p0_scan_block_string_event(
                document,
                source,
                length,
                line_start,
                line,
                &event,
                &offset,
                &line
            )) {
            event.level = lm_p0_stream_block_string_level(stack, pending);
            if (!lm_p0_stream_apply_event(document, stack, pending, &event)) {
                status = 0;
                break;
            }
            continue;
        }
        if (document->diagnostic.code != 0) {
            status = 0;
            break;
        }

        line_end = lm_p0_find_layout_line_end(source, length, offset);
        raw_length = line_end - line_start;
        if (raw_length > 0U && source[line_start + raw_length - 1U] == '\r') {
            --raw_length;
        }

        if (raw_length == 0U) {
            if (line_end == length) {
                break;
            }
            lm_p0_advance_layout_line(source, length, line_start, line_end, &offset, &line);
            continue;
        }

        p = line_start;
        level = 0U;
        dotted_level = 0;
        if (line == 1U && raw_length >= 3U &&
            (unsigned char)source[p] == 0xEFU &&
            (unsigned char)source[p + 1U] == 0xBBU &&
            (unsigned char)source[p + 2U] == 0xBFU) {
            p += 3U;
        }

        if (p < line_start + raw_length && source[p] == '.') {
            dotted_level = 1;
            while (p < line_start + raw_length && source[p] == '.') {
                ++level;
                ++p;
                while (p < line_start + raw_length && lm_p0_is_horizontal_space(source[p])) {
                    ++p;
                }
            }
        } else {
            size_t indent_column;
            size_t mark_skip;

            lm_p0_scan_indent_column(source, p, line_start + raw_length, &p, &indent_column);
            text = source + p;
            text_length = (line_start + raw_length) - p;
            lm_p0_trim_right(&text, &text_length);
            mark_skip = 0U;
            if (!lm_p0_skip_leading_brace_marks(
                    document,
                    text,
                    text_length,
                    line,
                    (size_t)(text - (source + line_start)) + 1U,
                    &mark_skip
                )) {
                status = 0;
                break;
            }
            p += mark_skip;
            text = source + p;
            text_length = (line_start + raw_length) - p;
            lm_p0_trim_right(&text, &text_length);

            if (text_length == 0U || text[0] == '#') {
                if (line_end == length) {
                    break;
                }
                lm_p0_advance_layout_line(source, length, line_start, line_end, &offset, &line);
                continue;
            }

            if (!lm_p0_indent_level_from_column(
                    document,
                    indent_stack,
                    indent_column,
                    line,
                    (size_t)(text - (source + line_start)) + 1U,
                    &level
                )) {
                status = 0;
                break;
            }
        }

        text = source + p;
        text_length = (line_start + raw_length) - p;
        lm_p0_trim_right(&text, &text_length);
        {
            size_t mark_skip;

            mark_skip = 0U;
            if (!lm_p0_skip_leading_brace_marks(
                    document,
                    text,
                    text_length,
                    line,
                    (size_t)(text - (source + line_start)) + 1U,
                    &mark_skip
                )) {
                status = 0;
                break;
            }
            p += mark_skip;
            text = source + p;
            text_length = (line_start + raw_length) - p;
            lm_p0_trim_right(&text, &text_length);
        }
        node_flags = 0U;

        if (text_length == 0U && level == 0U) {
            if (line_end == length) {
                break;
            }
            lm_p0_advance_layout_line(source, length, line_start, line_end, &offset, &line);
            continue;
        }
        if (text_length > 0U && text[0] == '#') {
            if (dotted_level && level > 0U) {
                text_length = 0U;
            } else {
                if (line_end == length) {
                    break;
                }
                lm_p0_advance_layout_line(source, length, line_start, line_end, &offset, &line);
                continue;
            }
        }
        if (text_length > 0U && text[0] == '%') {
            size_t marker_column;
            size_t header_column;
            size_t next_offset;
            size_t next_line;
            size_t disabled_next_offset;
            size_t disabled_next_line;
            size_t skip;

            node_flags |= LM_P0_NODE_INACTIVE;
            marker_column = (size_t)(text - (source + line_start)) + 1U;
            skip = 1U;
            while (skip < text_length && lm_p0_is_horizontal_space(text[skip])) {
                ++skip;
            }
            text += skip;
            text_length -= skip;
            lm_p0_trim_trailing_line_comment(&text, &text_length);
            if (text_length == 0U || text[0] == '#') {
                lm_p0_set_diagnostic(document, 21, line, marker_column, "disabled marker must be followed by a source item");
                status = 0;
                break;
            }

            header_column = (size_t)(text - (source + line_start)) + 1U;
            next_offset = line_start;
            next_line = line;
            lm_p0_advance_layout_line(source, length, line_start, line_end, &next_offset, &next_line);
            if (!lm_p0_validate_disabled_block(
                    document,
                    indent_stack,
                    next_offset,
                    next_line,
                    level,
                    text,
                    text_length,
                    line,
                    header_column,
                    &disabled_next_offset,
                    &disabled_next_line
                )) {
                status = 0;
                break;
            }

            memset(&event, 0, sizeof(event));
            event.kind = LM_P0_STREAM_EVENT_DISABLED_BLOCK;
            event.level = level;
            event.node_flags = node_flags;
            event.text = text;
            event.text_length = text_length;
            event.line = line;
            event.column = header_column;
            event.offset = (size_t)(text - source);

            if (!lm_p0_stream_apply_event(document, stack, pending, &event)) {
                status = 0;
                break;
            }

            offset = disabled_next_offset;
            line = disabled_next_line;
            continue;
        }

        memset(&event, 0, sizeof(event));
        event.level = level;
        event.node_flags = node_flags;
        event.text = text;
        event.text_length = text_length;
        event.line = line;
        event.column = (size_t)(text - (source + line_start)) + 1U;
        event.offset = (size_t)(text - source);
        event.kind = text_length == 0U && level > 0U
            ? LM_P0_STREAM_EVENT_DELIM
            : LM_P0_STREAM_EVENT_ITEM;

        if (has_last_physical_level) {
            if (level > last_physical_level + 1U) {
                lm_p0_set_diagnostic(document, 13, line, event.column, "source level increase must be one step");
                status = 0;
                break;
            }
            if (level + 1U < last_physical_level && !lm_p0_stream_event_is_tail_cutter(&event)) {
                lm_p0_set_diagnostic(
                    document,
                    13,
                    line,
                    event.column,
                    "source level decrease must be one step unless a tail cutter is used"
                );
                status = 0;
                break;
            }
        }

        if (!lm_p0_stream_apply_event(document, stack, pending, &event)) {
            status = 0;
            break;
        }
        has_last_physical_level = 1;
        last_physical_level = level;

        if (line_end == length) {
            break;
        }
        lm_p0_advance_layout_line(source, length, line_start, line_end, &offset, &line);
    }

    if (status && !lm_p0_stream_resolve_pending_delimiter(document, stack, pending, (size_t)-1)) {
        status = 0;
    }

    lm_p0_indent_stack_delete(indent_stack);
    lm_p0_stack_delete(stack);
    lm_p0_pending_delimiter_delete(pending);
    return status && document->diagnostic.code == 0;
}

static int lm_p0_validate_nonempty_colon_frames_in_structure(
    LmP0Document *document,
    const LmP0Structure *structure
);

static void lm_p0_structure_recount(LmP0Structure *structure) {
    LmP0Field *field;

    structure->field_count = 0U;
    structure->last_field = NULL;
    field = structure->first_field;
    while (field != NULL) {
        ++structure->field_count;
        structure->last_field = field;
        field = field->next;
    }
}

static int lm_p0_postprocess_structure(
    LmP0Document *document,
    LmP0Structure *structure
);

static int lm_p0_wrap_fields_from_line(
    LmP0Document *document,
    LmP0Structure *structure,
    size_t head_line
);

static int lm_p0_postprocess_trailer(LmP0Document *document, LmP0Trailer *trailer) {
    if (trailer == NULL) {
        return 1;
    }
    return lm_p0_postprocess_structure(document, &trailer->body);
}

static int lm_p0_postprocess_node(LmP0Document *document, LmP0Node *node) {
    if (node == NULL) {
        return 1;
    }

    if (node->kind == LM_P0_NODE_FRAME) {
        if (!lm_p0_postprocess_structure(document, &node->as.frame.body)) {
            return 0;
        }
        if ((node->as.frame.flags & LM_P0_FRAME_INLINE_BODY) != 0U &&
            !lm_p0_wrap_fields_from_line(document, &node->as.frame.body, node->span.line)) {
            return 0;
        }
        return lm_p0_postprocess_trailer(document, node->as.frame.trailer);
    }

    if (node->kind == LM_P0_NODE_STRUCTURE) {
        if (!lm_p0_postprocess_structure(document, &node->as.structure)) {
            return 0;
        }
        return lm_p0_postprocess_trailer(document, node->as.structure.trailer);
    }

    return 1;
}

static int lm_p0_wrap_fields_from_line(
    LmP0Document *document,
    LmP0Structure *structure,
    size_t head_line
) {
    LmP0Field *field;
    LmP0Field *previous;
    LmP0Field *group_first;
    LmP0Node *group_node;
    LmP0Field *move;

    previous = NULL;
    field = structure->first_field;
    while (field != NULL) {
        if (field->value != NULL && field->value->span.line != head_line) {
            break;
        }
        previous = field;
        field = field->next;
    }

    if (field == NULL) {
        return 1;
    }

    group_first = field;
    group_node = lm_p0_new_node(document, LM_P0_NODE_STRUCTURE);
    if (group_node == NULL) {
        return 0;
    }
    group_node->span = group_first->value->span;

    move = group_first;
    while (move != NULL) {
        LmP0Field *next_move;
        LmP0Node *value;

        next_move = move->next;
        value = move->value;
        move->value = NULL;
        if (!lm_p0_append_field(document, &group_node->as.structure, value)) {
            lm_p0_free_node(group_node);
            return 0;
        }
        if (move != group_first) {
            free(move);
        }
        move = next_move;
    }

    group_first->value = group_node;
    group_first->next = NULL;
    if (previous == NULL) {
        structure->first_field = group_first;
    } else {
        previous->next = group_first;
    }

    lm_p0_structure_recount(structure);
    return 1;
}

static int lm_p0_postprocess_structure(
    LmP0Document *document,
    LmP0Structure *structure
) {
    LmP0Field *field;

    field = structure->first_field;
    while (field != NULL) {
        if (field->value != NULL && !lm_p0_postprocess_node(document, field->value)) {
            return 0;
        }

        field = field->next;
    }

    lm_p0_structure_recount(structure);
    return 1;
}

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

int lm_p0_parse_bytes(
    const char *source,
    size_t source_length,
    LmP0Document **out_document
) {
    LmP0Document *document;

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
        source_length = 0U;
    }

    document->source_length = source_length;
    document->source = lm_p0_copy_bytes(source, document->source_length);
    if (document->source == NULL) {
        lm_own_delete(document, NULL);
        return 1;
    }

    if (lm_p0_parse_stream(document)) {
        if (lm_p0_postprocess_node(document, document->root)) {
            (void)lm_p0_validate_nonempty_colon_frames_in_node(document, document->root);
        }
    }

    *out_document = document;
    return document->diagnostic.code == 0 ? 0 : document->diagnostic.code;
}

int lm_p0_parse_string(const char *source, LmP0Document **out_document) {
    if (source == NULL) {
        source = "";
    }
    return lm_p0_parse_bytes(source, strlen(source), out_document);
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
    buffer[read_size] = '\0';

    status = lm_p0_parse_bytes(buffer, read_size, out_document);
    free(buffer);
    return status;
}

void lm_p0_document_destroy(LmP0Document *document) {
    if (document == NULL) {
        return;
    }
    lm_p0_free_node(document->root);
    lm_own_delete(document->source, NULL);
    lm_own_delete(document, NULL);
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
    lm_own_delete(ptr, NULL);
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
    if ((node->flags & LM_P0_NODE_INACTIVE) != 0U) {
        lm_p0_dump_append_cstr(dump, "Inactive ");
    }
    if (node->kind == LM_P0_NODE_STRUCTURE) {
        lm_p0_dump_appendf(dump, "Structure fields=%lu\n", (unsigned long)node->as.structure.field_count);
        lm_p0_dump_structure(dump, &node->as.structure, indent + 1U);
        lm_p0_dump_trailer(dump, node->as.structure.trailer, indent + 1U);
    } else if (node->kind == LM_P0_NODE_FRAME) {
        lm_p0_dump_append_cstr(dump, "Frame head=");
        lm_p0_dump_text(dump, node->as.frame.head);
        lm_p0_dump_appendf(dump, " body=Structure fields=%lu\n", (unsigned long)node->as.frame.body.field_count);
        lm_p0_dump_structure(dump, &node->as.frame.body, indent + 1U);
        lm_p0_dump_trailer(dump, node->as.frame.trailer, indent + 1U);
    } else if (node->kind == LM_P0_NODE_ATOM) {
        lm_p0_dump_append_cstr(dump, "Atom ");
        lm_p0_dump_text(dump, node->as.atom);
        lm_p0_dump_append_cstr(dump, "\n");
    } else if (node->kind == LM_P0_NODE_DISABLED) {
        lm_p0_dump_append_cstr(dump, "Block ");
        lm_p0_dump_text(dump, node->as.atom);
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

static LmP0Dump *lm_p0_dump_new(void) {
    return (LmP0Dump *)calloc(1U, sizeof(LmP0Dump));
}

static char *lm_p0_dump_take_data(LmP0Dump *dump) {
    char *data;

    if (dump == NULL) {
        return NULL;
    }
    data = dump->data;
    dump->data = NULL;
    dump->length = 0U;
    dump->capacity = 0U;
    return data;
}

static void lm_p0_dump_delete(LmP0Dump *dump) {
    if (dump != NULL) {
        lm_own_delete(dump->data, NULL);
        lm_own_delete(dump, NULL);
    }
}

char *lm_p0_dump_alloc(const LmP0Document *document) {
    LmP0Dump *dump;
    char *data;

    dump = lm_p0_dump_new();
    if (dump == NULL) {
        return NULL;
    }
    if (document == NULL || document->root == NULL) {
        lm_p0_dump_append_cstr(dump, "");
        data = lm_p0_dump_take_data(dump);
        lm_p0_dump_delete(dump);
        return data;
    }

    lm_p0_dump_node(dump, document->root, 0U);
    if (dump->failed) {
        lm_p0_dump_delete(dump);
        return NULL;
    }
    data = lm_p0_dump_take_data(dump);
    lm_p0_dump_delete(dump);
    return data;
}

