/* Evidence: candidate parse_bytes reaches L2 indent_level_from_column. */
#include "l1src/p0.h"
#include <stdio.h>
#include <string.h>

extern int l2_indent_level_calls;
int lm_p0_parse_bytes(const char *source, size_t source_length, LmP0Document **out_document);
void lm_p0_document_destroy(LmP0Document *document);

int main(void) {
    static const char src[] = "a\n    b\n        c\n";
    LmP0Document *document = 0;
    int status;
    l2_indent_level_calls = 0;
    status = lm_p0_parse_bytes(src, strlen(src), &document);
    printf("parse=%d hits=%d\n", status, l2_indent_level_calls);
    if (document != 0) {
        lm_p0_document_destroy(document);
    }
    if (status != 0) {
        return 1;
    }
    if (l2_indent_level_calls <= 0) {
        fprintf(stderr, "parse_bytes did not call L2 indent_level_from_column\n");
        return 1;
    }
    return 0;
}
