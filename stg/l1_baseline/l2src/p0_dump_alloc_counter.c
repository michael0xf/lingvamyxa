/* Counted calloc/realloc/free for p0_dump_driver.c's leak-balance check
   (lingvamyxa-e2, 2026-09-14: dump-printer Slice 4's falsifier shape for
   p0_dump_delete needs a counted allocator in the driver, not the port).
   Compiled WITHOUT the stage's -D<lm_p0_name>=p0_<name> / calloc-redirect
   flags -- if it were compiled WITH -Dcalloc=p0_counted_calloc etc, its
   own calls to the real calloc/realloc/free below would rewrite to
   themselves and recurse forever. run_port_parser.ps1 compiles this file
   once, plain, and links the resulting object into both the Ref and Port
   p0_dump_driver executables; only l1src/own.lm1's calloc/realloc/free
   call sites (inlined into lm1/build/parser.lm1.c, i.e. Ref's pristine
   copy and every stage's own patched copy) get redirected to these names,
   via three defines added alongside the stage's existing lm_p0_*
   redirects. */

#include <stdlib.h>

long p0_dump_alloc_balance = 0;

void *p0_counted_calloc(size_t count, size_t size) {
    void *p = calloc(count, size);
    if (p != 0) {
        p0_dump_alloc_balance++;
    }
    return p;
}

void *p0_counted_realloc(void *object, size_t size) {
    void *p = realloc(object, size);
    if (object == 0 && p != 0) {
        p0_dump_alloc_balance++;
    }
    return p;
}

void p0_counted_free(void *object) {
    if (object != 0) {
        p0_dump_alloc_balance--;
    }
    free(object);
}
