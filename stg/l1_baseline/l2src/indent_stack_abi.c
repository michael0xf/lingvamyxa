/* C ABI adapters: frozen parser symbols -> L2 l2_m{i} on the indent unit.
   parse_bytes in a stripped candidate parser calls these, not L1 bodies. */
#include "l2src/lmx.h"
#include "l1src/p0.h"
#include <stdio.h>

extern Lmx *l2_indent_unit;
int l2_indent_boot(void);

void l2_m0(Lmx *node, LmP0IndentStack *stack);
int l2_m1(Lmx *node, size_t *p, size_t v);
size_t l2_m2(Lmx *node, size_t *p);
int l2_m3(Lmx *node, LmP0Document *document, LmP0IndentStack *stack, size_t column, size_t line, size_t source_column);
int l2_m4(Lmx *node, LmP0Document *document, LmP0IndentStack *stack);
LmP0IndentStack *l2_m5(Lmx *node);
LmP0IndentStack *l2_m6(Lmx *node, LmP0Document *document);
void l2_m7(Lmx *node, LmP0IndentStack *stack);
int l2_m8(Lmx *node, LmP0Document *document, LmP0IndentStack *target, const LmP0IndentStack *source, size_t line, size_t column);
LmP0IndentStack *l2_m9(Lmx *node, LmP0Document *document, const LmP0IndentStack *source, size_t line, size_t column);
int l2_m10(Lmx *node, LmP0Document *document, LmP0IndentStack *stack, size_t column, size_t line, size_t source_column, size_t *out_level);

int l2_indent_level_calls = 0;

static void l2_indent_ensure(void) {
    if (l2_indent_unit == 0) {
        if (l2_indent_boot() != 0 || l2_indent_unit == 0) {
            fprintf(stderr, "l2_indent_boot failed\n");
        }
    }
}

void lm_p0_indent_stack_free(LmP0IndentStack *stack) {
    l2_indent_ensure();
    l2_m0(l2_indent_unit, stack);
}

void lm_p0_indent_stack_free_any(void *object) {
    lm_p0_indent_stack_free((LmP0IndentStack *)object);
}

int lm_p0_indent_stack_push(LmP0Document *document, LmP0IndentStack *stack, size_t column, size_t line, size_t source_column) {
    l2_indent_ensure();
    return l2_m3(l2_indent_unit, document, stack, column, line, source_column);
}

int lm_p0_indent_stack_init(LmP0Document *document, LmP0IndentStack *stack) {
    l2_indent_ensure();
    return l2_m4(l2_indent_unit, document, stack);
}

LmP0IndentStack *lm_p0_indent_stack_new_empty(void) {
    l2_indent_ensure();
    return l2_m5(l2_indent_unit);
}

LmP0IndentStack *lm_p0_indent_stack_new(LmP0Document *document) {
    l2_indent_ensure();
    return l2_m6(l2_indent_unit, document);
}

void lm_p0_indent_stack_delete(LmP0IndentStack *stack) {
    l2_indent_ensure();
    l2_m7(l2_indent_unit, stack);
}

int lm_p0_indent_stack_copy(LmP0Document *document, LmP0IndentStack *target, const LmP0IndentStack *source, size_t line, size_t column) {
    l2_indent_ensure();
    return l2_m8(l2_indent_unit, document, target, source, line, column);
}

LmP0IndentStack *lm_p0_indent_stack_clone(LmP0Document *document, const LmP0IndentStack *source, size_t line, size_t column) {
    l2_indent_ensure();
    return l2_m9(l2_indent_unit, document, source, line, column);
}

int lm_p0_indent_level_from_column(LmP0Document *document, LmP0IndentStack *stack, size_t column, size_t line, size_t source_column, size_t *out_level) {
    l2_indent_ensure();
    l2_indent_level_calls += 1;
    return l2_m10(l2_indent_unit, document, stack, column, line, source_column, out_level);
}
