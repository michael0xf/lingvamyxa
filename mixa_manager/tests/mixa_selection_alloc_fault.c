/* Allocation-fault injection for the mixa_selection parity harness
 * (ticket 20260913-083100). Linked ONLY into the parity-harness test
 * binaries, never into the real production build: mixa_selection.h and
 * mixa_selection.lm1 (and the L2 source they mirror) are compiled
 * completely unmodified for this test -- only the COMPILE COMMAND for
 * that one translation unit gets -Dmalloc=test_malloc
 * -Dcalloc=test_calloc, redirecting its own malloc/calloc calls to the
 * counting wrappers below. This file's own calls to the real malloc/
 * calloc are explicitly undef'd back to the libc originals first, so it
 * is unaffected by the same -D flags if a build script ever applies
 * them uniformly.
 */
#include <stdlib.h>
#undef malloc
#undef calloc

static int g_fail_at = -1;
static int g_call_count = 0;

void mixa_test_fault_set(int n) {
    g_fail_at = n;
    g_call_count = 0;
}

void mixa_test_fault_clear(void) {
    g_fail_at = -1;
    /* Deliberately does NOT reset g_call_count: mixa_test_fault_calls()
     * is meant to report how many allocator calls happened during the
     * just-finished fault_set(n)...fault_clear() window, for the caller
     * to sanity-check its own derivation of which call number failed. */
}

int mixa_test_fault_calls(void) {
    return g_call_count;
}

void *test_malloc(size_t n) {
    g_call_count = g_call_count + 1;
    if (g_fail_at >= 0 && g_call_count == g_fail_at) {
        return NULL;
    }
    return malloc(n);
}

void *test_calloc(size_t n, size_t s) {
    g_call_count = g_call_count + 1;
    if (g_fail_at >= 0 && g_call_count == g_fail_at) {
        return NULL;
    }
    return calloc(n, s);
}
