#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.lm1.h"

static int lm_trans_text_equals(LmP0Text text, const char *value) {
    size_t length;

    length = strlen(value);
    return text.length == length && memcmp(text.data, value, length) == 0;
}

static const LmP0Node *lm_trans_find_root_frame(
    const LmP0Document *document,
    const char *name
) {
    const LmP0Node *root;
    const LmP0Field *field;
    const LmP0Node *node;

    root = lm_p0_document_root(document);
    if (root == NULL || root->kind != LM_P0_NODE_STRUCTURE) {
        return NULL;
    }

    field = root->as.structure.first_field;
    while (field != NULL) {
        node = field->value;
        if (
            node != NULL &&
            node->kind == LM_P0_NODE_FRAME &&
            lm_trans_text_equals(node->as.frame.head, name)
        ) {
            return node;
        }
        field = field->next;
    }

    return NULL;
}

static int lm_trans_write_all(FILE *file, const char *data, size_t length) {
    return fwrite(data, 1U, length, file) == length ? 0 : 1;
}

static int lm_trans_emit_l1_body(const char *source_path, const char *output_path) {
    LmP0Document *document;
    const LmP0Diagnostic *diagnostic;
    const LmP0Node *l1;
    const LmP0Field *body_field;
    const LmP0Node *body;
    FILE *output;
    int status;

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

    l1 = lm_trans_find_root_frame(document, "L1");
    if (l1 == NULL) {
        fprintf(stderr, "trans error: root L1 frame was not found\n");
        lm_p0_document_destroy(document);
        return 1;
    }

    body_field = l1->as.frame.body.first_field;
    if (body_field == NULL || body_field->next != NULL) {
        fprintf(stderr, "trans error: L1 receiver expects exactly one raw body field\n");
        lm_p0_document_destroy(document);
        return 1;
    }

    body = body_field->value;
    if (body == NULL || body->kind != LM_P0_NODE_ATOM) {
        fprintf(stderr, "trans error: L1 receiver expects raw body text as its first field\n");
        lm_p0_document_destroy(document);
        return 1;
    }

    output = fopen(output_path, "wb");
    if (output == NULL) {
        fprintf(stderr, "trans error: cannot open output file %s\n", output_path);
        lm_p0_document_destroy(document);
        return 1;
    }

    status = lm_trans_write_all(output, body->as.atom.data, body->as.atom.length);
    if (status == 0) {
        status = lm_trans_write_all(output, "\n", 1U);
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

    return lm_trans_emit_l1_body(argv[1], argv[2]);
}

