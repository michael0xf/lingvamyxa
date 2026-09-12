/* Exercise the generated L1 import storage, including allocation failures. */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int fail_malloc;
static int fail_realloc;
static int live;

static void *checked_malloc(size_t size)
{
    void *p;
    if (fail_malloc) {
        fail_malloc = 0;
        return NULL;
    }
    p = malloc(size);
    if (p) ++live;
    return p;
}

static void *checked_realloc(void *old, size_t size)
{
    void *p;
    int fresh = old == NULL;
    if (fail_realloc) {
        fail_realloc = 0;
        return NULL;
    }
    p = realloc(old, size);
    if (p && fresh) ++live;
    return p;
}

static void checked_free(void *p)
{
    if (p) --live;
    free(p);
}

#define malloc checked_malloc
#define realloc checked_realloc
#define free checked_free
#define main imported_translator_main
#include L1_IMPORT_GENERATED_C
#undef main
#undef free
#undef realloc
#undef malloc

int main(void)
{
    int i, saved_n, saved_live;
    size_t saved_capacity;
    LmP0Text *saved_table;
    char path[64], long_path[8193];

    for (i = 0; i < 65; ++i) {
        snprintf(path, sizeof(path), "module-%d.lm1", i);
        assert(l1_imp_push(0, path) == 0);
    }
    assert(l1_imp_donen == 65);
    for (i = 0; i < 65; ++i) {
        snprintf(path, sizeof(path), "module-%d.lm1", i);
        assert(l1_imp_find(l1_imp_done, l1_imp_donen, path) == i);
        assert(l1_imp_done[i].length == strlen(path));
        assert(strcmp(l1_imp_done[i].data, path) == 0);
    }
    memset(long_path, 'x', sizeof(long_path) - 1);
    long_path[sizeof(long_path) - 1] = 0;
    assert(l1_imp_push(0, long_path) == 0);
    assert(l1_imp_done[65].length == 8192);
    assert(l1_imp_find(l1_imp_done, l1_imp_donen, long_path) == 65);
    long_path[0] = 'y';
    assert(l1_imp_done[65].data[0] == 'x');
    assert(l1_imp_find(l1_imp_done, l1_imp_donen, long_path) == -1);
    assert(l1_imp_push(1, "active-parent.lm1") == 0);
    assert(l1_imp_push(1, "active-child.lm1") == 0);
    l1_imp_pop();
    assert(l1_imp_actn == 1);
    assert(l1_imp_find(l1_imp_act, l1_imp_actn, "active-parent.lm1") == 0);
    assert(l1_imp_find(l1_imp_act, l1_imp_actn, "active-child.lm1") == -1);
    l1_imp_reset();
    assert(live == 0);
    assert(l1_imp_act == NULL && l1_imp_done == NULL);
    assert(l1_imp_actn == 0 && l1_imp_donen == 0);
    assert(l1_imp_actcap == 0 && l1_imp_donecap == 0);

    fail_malloc = 1;
    assert(l1_imp_push(0, "first.lm1") != 0);
    assert(live == 0 && l1_imp_done == NULL && l1_imp_donen == 0);
    fail_realloc = 1;
    assert(l1_imp_push(0, "first.lm1") != 0);
    assert(live == 0 && l1_imp_done == NULL && l1_imp_donecap == 0);
    assert(l1_imp_push(0, "first.lm1") == 0);
    while ((size_t)l1_imp_donen < l1_imp_donecap) {
        snprintf(path, sizeof(path), "fill-%d.lm1", l1_imp_donen);
        assert(l1_imp_push(0, path) == 0);
    }
    saved_n = l1_imp_donen;
    saved_table = l1_imp_done;
    saved_capacity = l1_imp_donecap;
    saved_live = live;
    fail_realloc = 1;
    assert(l1_imp_push(0, "growth.lm1") != 0);
    assert(l1_imp_done == saved_table && l1_imp_donen == saved_n);
    assert(l1_imp_donecap == saved_capacity && live == saved_live);
    assert(l1_imp_find(l1_imp_done, l1_imp_donen, "first.lm1") == 0);
    assert(l1_imp_find(l1_imp_done, l1_imp_donen, "growth.lm1") == -1);
    fail_malloc = 1;
    assert(l1_imp_push(0, "growth.lm1") != 0);
    assert(l1_imp_done == saved_table && l1_imp_donen == saved_n);
    assert(l1_imp_donecap == saved_capacity && live == saved_live);
    assert(l1_imp_push(0, "growth.lm1") == 0);
    assert(l1_imp_find(l1_imp_done, l1_imp_donen, "growth.lm1") == saved_n);
    l1_imp_reset();
    l1_imp_reset();
    l1_imp_pop();
    assert(live == 0);
    puts("import descriptor storage and OOM: PASS");
    return 0;
}
