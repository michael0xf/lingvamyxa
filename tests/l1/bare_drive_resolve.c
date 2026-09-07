/* Focused harness for l1_resolve_bare_import drive-relative prefix.
 * No D: filesystem access. Same join rules as l1src/l1trans.lm1. */
#include <stdio.h>
#include <string.h>
#include <stddef.h>

static int l1_path_is_bare(const char *s)
{
    size_t i = 0U;
    if (s == 0 || s[0] == 0)
        return 0;
    if (s[0] == 47 || s[0] == 92)
        return 0;
    if (s[1] != 0 && s[1] == 58)
        return 0;
    while (s[i] != 0) {
        if (s[i] == 47 || s[i] == 92)
            return 0;
        i = i + 1U;
    }
    return 1;
}

static int l1_resolve_bare_import(char *buf, size_t cap, const char *source_path, const char *file_name)
{
    size_t length = 0U;
    size_t slash = 0U;
    size_t i = 0U;
    size_t n = 0U;
    if (buf == 0 || cap < 2U || file_name == 0)
        return 1;
    n = strlen(file_name);
    if (source_path == 0 || source_path[0] == 0) {
        if (n + 1U >= cap)
            return 1;
        strcpy(buf, file_name);
        return 0;
    }
    length = strlen(source_path);
    while (i < length) {
        if (source_path[i] == 47 || source_path[i] == 92)
            slash = i + 1U;
        i = i + 1U;
    }
    if (slash == 0U) {
        if (source_path[1] != 0 && source_path[1] == 58) {
            if (2U + n >= cap)
                return 1;
            memcpy(buf, source_path, 2U);
            strcpy(buf + 2U, file_name);
            return 0;
        }
        if (n + 1U >= cap)
            return 1;
        strcpy(buf, file_name);
        return 0;
    }
    if (slash + n >= cap)
        return 1;
    memcpy(buf, source_path, slash);
    strcpy(buf + slash, file_name);
    return 0;
}

static int expect(const char *got, const char *want)
{
    if (strcmp(got, want) != 0) {
        fprintf(stderr, "got '%s' want '%s'\n", got, want);
        return 1;
    }
    return 0;
}

int main(void)
{
    char buf[1040];
    if (l1_path_is_bare("helper.lm2") == 0)
        return 1;
    if (l1_path_is_bare("D:helper.lm2") != 0)
        return 2;
    if (l1_path_is_bare("l1src/add.lm2") != 0)
        return 3;
    if (l1_resolve_bare_import(buf, 1040U, "D:entry.lm2", "helper.lm2") != 0)
        return 4;
    if (expect(buf, "D:helper.lm2") != 0)
        return 5;
    if (l1_resolve_bare_import(buf, 1040U, "tests/l1/mod.lm2", "helper.lm2") != 0)
        return 6;
    if (expect(buf, "tests/l1/helper.lm2") != 0)
        return 7;
    if (l1_resolve_bare_import(buf, 1040U, "entry.lm2", "helper.lm2") != 0)
        return 8;
    if (expect(buf, "helper.lm2") != 0)
        return 9;
    if (l1_resolve_bare_import(buf, 3U, "D:entry.lm2", "helper.lm2") == 0)
        return 10;
    return 0;
}
