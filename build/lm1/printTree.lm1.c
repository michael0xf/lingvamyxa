#include <stdio.h>
#include "parser.lm1.h"

int main(int argc, char **argv) {
    setvbuf(stdout, 0, _IONBF, 0);
    setvbuf(stderr, 0, _IONBF, 0);
    if (argc == 2) {
        LmP0Document *document = 0;
        const LmP0Diagnostic *diagnostic;
        char *dump;
        int status;
        status = lm_p0_parse_file(argv[1], & document);
        if (status != 0) {
            diagnostic = lm_p0_document_diagnostic(document);
            if (diagnostic != 0) {
                fprintf(stderr, "P0 parse error %d at %zu:%zu: %s\n", diagnostic -> code, diagnostic -> line, diagnostic -> column, diagnostic -> message);
                lm_p0_document_destroy(document);
                return 1;
            }
            fprintf(stderr, "P0 parse error while reading %s\n", argv[1]);
            lm_p0_document_destroy(document);
            return 1;
        }
        dump = lm_p0_dump_alloc(document);
        if (dump == 0) {
            fprintf(stderr, "P0 dump allocation failed\n");
            lm_p0_document_destroy(document);
            return 1;
        }
        fputs(dump, stdout);
        fflush(stdout);
        lm_p0_free(dump);
        lm_p0_document_destroy(document);
        return 0;
    }
    printf("usage: printTree.lm0[.exe] <source.lmx>\n");
    return 0;
}
