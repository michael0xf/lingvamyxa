/* p0_dump_driver -- parser-in-L2 port's own dump-printer acceptance
 * driver (e2, 2026-09-14). tests/p0_tree_contract/p0_meta_dump.c
 * deliberately does NOT exercise l1src/parser.lm1's own
 * lm_p0_dump_alloc/lm_p0_dump_run/lm_p0_dump_node family (it has its
 * own independent metadata walker; "Pretty printTree is not the
 * metadata oracle" per run_p0_meta.ps1's own comment) -- so the 36
 * tests/p0_tree_contract goldens prove nothing about that family
 * beyond compiling and linking.
 *
 * This driver is the reference-versus-port oracle for that family
 * specifically: parse each fixture the same way p0_meta_dump.c does
 * (lm_p0_parse_file), then call the oracle's own public dump entry
 * point, lm_p0_dump_alloc, and print its result verbatim. Built twice
 * by run_port_parser.ps1 -- once against the pristine oracle, once
 * against the patched-and-redirected copy plus this stage's L2 units
 * -- byte-identical output over all 36 tests/p0_tree_contract/*.lmx
 * inputs on both builds is the acceptance bar for the dump-printer
 * functions, the same reference-versus-port pattern as everything
 * else in this port, not a golden comparison.
 *
 * Only the current (non-620) parser ABI is relevant here -- the dump-
 * printer family is being ported now, not measured against a frozen
 * historical build.
 */
#include <stdio.h>
#include <stdlib.h>

typedef struct LmP0Document LmP0Document;

extern int lm_p0_parse_file(const char *path, LmP0Document **out);
extern void lm_p0_document_destroy(LmP0Document *document);
extern char *lm_p0_dump_alloc(const LmP0Document *document);

int main(int argc, char **argv) {
    LmP0Document *doc = 0;
    char *dump;
    if (argc != 2) {
        fputs("usage: p0_dump_driver <source.lmx>\n", stderr);
        return 2;
    }
    (void)lm_p0_parse_file(argv[1], &doc);
    dump = lm_p0_dump_alloc(doc);
    if (dump != 0) {
        fputs(dump, stdout);
        free(dump);
    }
    if (doc != 0) lm_p0_document_destroy(doc);
    return 0;
}
