/* P0 metadata dump. Compile against one parser/own pair; do not mix 620 and current.
 *
 * 620:  gcc -std=c99 -DLM_THREAD_PROVIDER=LM_THREAD_PROVIDER_SINGLE -DP0_META_ABI_620
 *         p0_meta_dump.c <620>/lm1/build/parser.lm1.c <620>/lm1/build/own.lm1.c
 * current: gcc -std=c99 -DLM_THREAD_PROVIDER=LM_THREAD_PROVIDER_SINGLE
 *         p0_meta_dump.c lm1/build/parser.lm1.c lm1/build/own.lm1.c
 *
 * Output is pointer-free: kinds, flags, spans, spelling bytes/length, null/empty.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#ifdef P0_META_ABI_620
#include <setjmp.h>
#endif

typedef int LmP0NodeKind;
typedef struct LmP0Text { const char *data; size_t length; } LmP0Text;
typedef struct LmP0Span { size_t line; size_t column; size_t offset; size_t length; } LmP0Span;
typedef struct LmP0Diagnostic { int code; size_t line; size_t column; char message[256]; } LmP0Diagnostic;
typedef struct LmP0Field LmP0Field;
typedef struct LmP0Trailer LmP0Trailer;
typedef struct LmP0Structure {
    LmP0Field *first_field;
    LmP0Field *last_field;
    size_t field_count;
    LmP0Trailer *trailer;
} LmP0Structure;
struct LmP0Trailer {
    LmP0Text *spelling;
    unsigned flags;
    LmP0Structure *body;
};
typedef struct LmP0Frame {
    LmP0Text *head;
    unsigned flags;
    LmP0Structure *body;
    LmP0Trailer *trailer;
} LmP0Frame;
typedef struct LmP0NodeAs {
    LmP0Structure *structure;
    LmP0Frame *frame;
    LmP0Text *atom;
} LmP0NodeAs;
typedef struct LmP0Node {
    LmP0NodeKind kind;
    unsigned flags;
    LmP0Span *span;
    LmP0NodeAs *as;
} LmP0Node;
struct LmP0Field {
    LmP0Node *value;
    LmP0Field *next;
};
typedef struct LmP0Document {
    char *source;
    size_t source_length;
    LmP0Node *root;
    LmP0Diagnostic *diagnostic;
    void *source_owner;
    void *token_arena;
    void *tree_arena;
    void *diagnostic_arena;
    int owners_initialized;
    int frozen;
} LmP0Document;

#define LM_P0_NODE_STRUCTURE 1
#define LM_P0_NODE_FRAME 2
#define LM_P0_NODE_ATOM 3
#define LM_P0_NODE_DISABLED 4
#define LM_P0_TRAILER_COLON 2U

#ifdef P0_META_ABI_620
struct LmMessageThread;
struct LmMessageThreadRuntime;
struct LmMessageThread *lm_message_thread_new(void);
void lm_message_thread_delete(struct LmMessageThread *thread);
int lm_message_thread_begin_turn(struct LmMessageThread *thread);
int lm_message_thread_end_turn(struct LmMessageThread *thread);
void lm_message_thread_request_stop(struct LmMessageThread *thread, int status);
void *lm_message_thread_set_execution_context(struct LmMessageThread *thread, void *context);
struct LmMessageThreadRuntime *lm_message_thread_runtime_new(void);
int lm_message_thread_runtime_attach_root(struct LmMessageThreadRuntime *runtime, struct LmMessageThread *thread);
int lm_message_thread_runtime_detach_root(struct LmMessageThreadRuntime *runtime, struct LmMessageThread *thread);
int lm_message_thread_runtime_delete(struct LmMessageThreadRuntime *runtime);
int lm_p0_parse_file(struct LmMessageThread *thread, const char *path, LmP0Document **out);
const LmP0Diagnostic *lm_p0_document_diagnostic(struct LmMessageThread *thread, const LmP0Document *document);
void lm_p0_document_destroy(struct LmMessageThread *thread, LmP0Document *document);
typedef struct LmMessageThreadExecutionContext {
    jmp_buf diagnostic_root;
    int diagnostic_code;
    const char *diagnostic_label;
    const char *diagnostic_file;
    int diagnostic_line;
    const char *diagnostic_expr;
} LmMessageThreadExecutionContext;
#else
int lm_p0_parse_file(const char *path, LmP0Document **out);
const LmP0Diagnostic *lm_p0_document_diagnostic(const LmP0Document *document);
void lm_p0_document_destroy(LmP0Document *document);
#endif

static const char *kind_name(int k) {
    if (k == LM_P0_NODE_STRUCTURE) return "structure";
    if (k == LM_P0_NODE_FRAME) return "frame";
    if (k == LM_P0_NODE_ATOM) return "atom";
    if (k == LM_P0_NODE_DISABLED) return "disabled";
    return "unknown";
}

static void indent_f(FILE *out, int n) {
    int i;
    for (i = 0; i < n; i++) fputc(' ', out);
}

static void dump_bytes(FILE *out, const char *p, size_t n) {
    size_t i;
    fputs(" bytes=", out);
    if (p == 0) {
        fputs("null", out);
        return;
    }
    for (i = 0; i < n; i++) fprintf(out, "%02X", (unsigned char)p[i]);
}

static void dump_text(FILE *out, const char *label, const LmP0Text *t) {
    fprintf(out, " %s", label);
    if (t == 0) {
        fputs("=null", out);
        return;
    }
    fprintf(out, "_len=%zu", t->length);
    dump_bytes(out, t->data, t->length);
}

static void dump_span(FILE *out, const LmP0Span *s) {
    if (s == 0) {
        fputs(" span=null", out);
        return;
    }
    fprintf(out, " span=%zu:%zu+%zu:%zu", s->line, s->column, s->offset, s->length);
}

static void dump_node(FILE *out, const LmP0Node *node, int ind);

static void dump_trailer(FILE *out, const LmP0Trailer *tr, int ind) {
    indent_f(out, ind);
    if (tr == 0) {
        fputs("trailer=null\n", out);
        return;
    }
    /* flags_wo_colon: 620 has no TRAILER_COLON bit; current sets bit 2. Version that bit separately. */
    fprintf(out, "trailer flags=0x%X colon_bit=%u body=%s",
            tr->flags & ~LM_P0_TRAILER_COLON,
            (tr->flags & LM_P0_TRAILER_COLON) ? 1U : 0U,
            tr->body ? "struct" : "null");
    dump_text(out, "spelling", tr->spelling);
    fputc('\n', out);
    if (tr->body) {
        indent_f(out, ind + 2);
        fprintf(out, "trailer_body field_count=%zu\n", tr->body->field_count);
        {
            LmP0Field *f = tr->body->first_field;
            size_t i = 0;
            while (f) {
                indent_f(out, ind + 2);
                fprintf(out, "[%zu] value=%s\n", i, f->value ? "node" : "null");
                dump_node(out, f->value, ind + 4);
                f = f->next;
                i++;
            }
        }
        dump_trailer(out, tr->body->trailer, ind + 2);
    }
}

static void dump_struct(FILE *out, const LmP0Structure *st, int ind) {
    LmP0Field *f;
    size_t i = 0;
    indent_f(out, ind);
    if (st == 0) {
        fputs("struct=null\n", out);
        return;
    }
    fprintf(out, "struct field_count=%zu trailer=%s\n", st->field_count, st->trailer ? "yes" : "null");
    f = st->first_field;
    while (f) {
        indent_f(out, ind);
        fprintf(out, "[%zu] value=%s\n", i, f->value ? "node" : "null");
        dump_node(out, f->value, ind + 2);
        f = f->next;
        i++;
    }
    dump_trailer(out, st->trailer, ind);
}

static void dump_node(FILE *out, const LmP0Node *node, int ind) {
    indent_f(out, ind);
    if (node == 0) {
        fputs("node=null\n", out);
        return;
    }
    fprintf(out, "node kind=%s flags=0x%X as=%s",
            kind_name(node->kind), node->flags, node->as ? "yes" : "null");
    dump_span(out, node->span);
    fputc('\n', out);
    if (node->as == 0) return;
    if (node->kind == LM_P0_NODE_STRUCTURE) dump_struct(out, node->as->structure, ind + 2);
    else if (node->kind == LM_P0_NODE_FRAME) {
        LmP0Frame *fr = node->as->frame;
        indent_f(out, ind + 2);
        if (fr == 0) {
            fputs("frame=null\n", out);
            return;
        }
        fprintf(out, "frame flags=0x%X body=%s trailer=%s",
                fr->flags, fr->body ? "struct" : "null", fr->trailer ? "yes" : "null");
        dump_text(out, "head", fr->head);
        fputc('\n', out);
        dump_struct(out, fr->body, ind + 4);
        dump_trailer(out, fr->trailer, ind + 4);
    } else if (node->kind == LM_P0_NODE_ATOM || node->kind == LM_P0_NODE_DISABLED) {
        indent_f(out, ind + 2);
        dump_text(out, "atom", node->as->atom);
        fputc('\n', out);
    }
}

static int dump_document(FILE *out, LmP0Document *doc, const LmP0Diagnostic *diag) {
    fputs("# p0-meta v1\n", out);
    if (diag != 0) {
        fprintf(out, "REJECT code=%d line=%zu col=%zu msg=%s\n",
                diag->code, diag->line, diag->column, diag->message);
        return 1;
    }
    if (doc == 0) {
        fputs("document=null\n", out);
        return 1;
    }
    fprintf(out, "document source_len=%zu frozen=%d root=%s diagnostic_slot=%s\n",
            doc->source_length, doc->frozen, doc->root ? "yes" : "null",
            doc->diagnostic ? "yes" : "null");
    dump_node(out, doc->root, 0);
    return 0;
}

#ifdef P0_META_ABI_620
static int run_620(const char *path) {
    struct LmMessageThread *thread;
    struct LmMessageThreadRuntime *runtime = 0;
    LmMessageThreadExecutionContext ctx;
    LmP0Document *doc = 0;
    const LmP0Diagnostic *diag;
    int status;
    int rc = 1;
    memset(&ctx, 0, sizeof(ctx));
    thread = lm_message_thread_new();
    if (thread == 0) return 1;
    lm_message_thread_set_execution_context(thread, &ctx);
    runtime = lm_message_thread_runtime_new();
    if (runtime != 0) (void)lm_message_thread_runtime_attach_root(runtime, thread);
    while (lm_message_thread_begin_turn(thread)) {
        if (setjmp(ctx.diagnostic_root) == 0) {
            status = lm_p0_parse_file(thread, path, &doc);
            diag = 0;
            if (status != 0) diag = lm_p0_document_diagnostic(thread, doc);
            rc = dump_document(stdout, doc, diag);
            if (doc) lm_p0_document_destroy(thread, doc);
        } else {
            fputs("# p0-meta v1\nLONGJMP\n", stdout);
            rc = 1;
        }
        lm_message_thread_request_stop(thread, rc);
        (void)lm_message_thread_end_turn(thread);
        break;
    }
    if (runtime) {
        (void)lm_message_thread_runtime_detach_root(runtime, thread);
        (void)lm_message_thread_runtime_delete(runtime);
    }
    lm_message_thread_delete(thread);
    return rc;
}
#else
static int run_current(const char *path) {
    LmP0Document *doc = 0;
    const LmP0Diagnostic *diag = 0;
    int status;
    int rc;
    status = lm_p0_parse_file(path, &doc);
    if (status != 0) diag = lm_p0_document_diagnostic(doc);
    rc = dump_document(stdout, doc, diag);
    if (doc) lm_p0_document_destroy(doc);
    return rc;
}
#endif

int main(int argc, char **argv) {
    if (argc != 2) {
        fputs("usage: p0_meta_dump <source.lmx>\n", stderr);
        return 2;
    }
#ifdef P0_META_ABI_620
    return run_620(argv[1]);
#else
    return run_current(argv[1]);
#endif
}
