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
    if (fail_malloc && --fail_malloc == 0) {
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
    if (fail_realloc && --fail_realloc == 0) {
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


static void test_paths(void)
{
    char long_source[8193], normalized[8300];
    const char *bare = "\"leaf.lm1\"";
    LmP0Text atom = {bare, strlen(bare)};
    LmP0Text *text;
    int i;
    char *cwd;
    size_t max = (size_t)-1;

    assert(l1_path_capacity(max, 0) == 0);
    assert(l1_path_capacity(0, max) == 0);
    assert(l1_path_capacity(max - 2, 0) == max);
    assert(l1_path_capacity(max - 2, 1) == 0);
    strcpy(long_source, "C:/");
    memset(long_source + 3, 'x', 8174);
    strcpy(long_source + 8177, "/parent.lm1");
    text = l1_import_path_text(&atom, long_source);
    assert(text && text->length == 8186);
    assert(strcmp(text->data + 8177, "/leaf.lm1") == 0);
    assert(memcmp(text->data, long_source, 8177) == 0);
    l1_path_text_delete(text);
    assert(live == 0);
    for (i = 1; i <= 3; ++i) {
        fail_malloc = i;
        assert(l1_import_path_text(&atom, long_source) == NULL);
        assert(fail_malloc == 0 && live == 0);
    }
    atom.data = long_source;
    atom.length = strlen(long_source);
    text = l1_import_path_text(&atom, "ignored.lm1");
    assert(text && text->length == atom.length);
    assert(strcmp(text->data, long_source) == 0);
    l1_path_text_delete(text);
    for (i = 1; i <= 2; ++i) {
        fail_malloc = i;
        assert(l1_import_path_text(&atom, NULL) == NULL);
        assert(fail_malloc == 0 && live == 0);
    }
    atom.length = max;
    assert(l1_import_path_text(&atom, NULL) == NULL && live == 0);

    l1_unit_root = "C:/project";
    l1_unit_root_set = 1;
    assert(l1_unit_id_from_source(normalized, sizeof(normalized),
        "C:/project/a/../dir/./header.h.lm1", NULL, NULL) == 0);
    assert(strcmp(normalized, "dir/header.h.lm1") == 0);
    assert(live == 0);
    for (i = 1; i <= 3; ++i) {
        fail_malloc = i;
        assert(l1_unit_id_from_source(normalized, sizeof(normalized),
            "dir/header.h.lm1", NULL, NULL) != 0);
        assert(fail_malloc == 0 && live == 0);
    }
    long_source[8177] = 0;
    l1_unit_root = long_source;
    assert(l1_unit_id_from_source(normalized, sizeof(normalized),
        "dir/header.h.lm1", NULL, NULL) == 0);
    assert(strcmp(normalized, "dir/header.h.lm1") == 0 && live == 0);
    assert(l1_unit_id_from_source(normalized, sizeof(normalized),
        "../outside.h.lm1", NULL, NULL) != 0 && live == 0);
    assert(l1_path_norm_into(normalized, 2, "abc", NULL, NULL) != 0);
    assert(l1_path_norm_into(normalized, sizeof(normalized), "..", NULL, NULL) != 0);
    assert(l1_path_norm_into(normalized, sizeof(normalized), "C:/../bad", NULL, NULL) != 0);
    fail_realloc = 1;
    assert(l1_getcwd_owned() == NULL && live == 0);
    cwd = l1_getcwd_owned();
    assert(cwd && cwd[0]);
    checked_free(cwd);
    assert(live == 0);
    l1_unit_root = NULL;
    l1_unit_root_set = 0;
    puts("dynamic path descriptors, normalization and allocation failure cleanup: PASS");
}

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
    test_paths();
    return 0;
}
