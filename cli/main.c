#include <stdio.h>

#include "lm_core.h"
#include "lm_p0.h"

int main(int argc, char **argv) {
    if (argc == 2) {
        LmP0Document *document;
        const LmP0Diagnostic *diagnostic;
        char *dump;
        int status;

        document = NULL;
        status = lm_p0_parse_file(argv[1], &document);
        if (status != 0) {
            diagnostic = lm_p0_document_diagnostic(document);
            if (diagnostic != NULL) {
                fprintf(
                    stderr,
                    "P0 parse error %d at %lu:%lu: %s\n",
                    diagnostic->code,
                    (unsigned long)diagnostic->line,
                    (unsigned long)diagnostic->column,
                    diagnostic->message
                );
            } else {
                fprintf(stderr, "P0 parse error while reading %s\n", argv[1]);
            }
            lm_p0_document_destroy(document);
            return 1;
        }

        dump = lm_p0_dump_alloc(document);
        if (dump == NULL) {
            fprintf(stderr, "P0 dump allocation failed\n");
            lm_p0_document_destroy(document);
            return 1;
        }

        fputs(dump, stdout);
        lm_p0_free(dump);
        lm_p0_document_destroy(document);
        return 0;
    }

    printf("%s\n", lm_version());
    printf("lm_add(2, 3) = %d\n", lm_add(2, 3));
    printf("usage: lingvamyxa_cli <source.lmx>\n");

    return 0;
}
