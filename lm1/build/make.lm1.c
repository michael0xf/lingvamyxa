#include <stddef.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>


















#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char * (getenv)(const char *name);
size_t (strlen)(const char *text);
char * (strchr)(const char *text, int value);
void * (memcpy)(void *target, const void *source, size_t length);
char * (strcpy)(char *target, const char *source);
char * (strcat)(char *target, const char *source);
int (system)(const char *command);
int (remove)(const char *path);
FILE * (fopen)(const char *path, const char *mode);
int (fclose)(FILE *file);
size_t (fread)(void *buffer, size_t item_size, size_t item_count, FILE *file);
size_t (fwrite)(const void *buffer, size_t item_size, size_t item_count, FILE *file);
int (ferror)(FILE *file);
int (strcmp)(const char *left, const char *right);

static char * lm_make_env_or_default(char *name, char *fallback);
static size_t lm_make_append(char *buffer, size_t size, size_t used, char *text);
static size_t lm_make_append_arg(char *buffer, size_t size, size_t used, char *arg);
static int lm_make_run_command(char *command);
static int lm_make_run_tool(char *tool, int argc, char **argv, int start);
static int lm_make_copy_file(char *source_path, char *output_path);
static void lm_make_print_usage(void);
int main(int argc, char **argv);

struct LmOwnArena;
struct LmOwnArena *lm_own_arena_new(void);
void lm_own_arena_delete(struct LmOwnArena *arena);
void *lm_own_arena_new_zero(struct LmOwnArena *arena, size_t size);
typedef struct LmL5ExecutionContext LmL5ExecutionContext;
typedef struct LmL5Thread LmL5Thread;
struct LmL5ExecutionContext {
    jmp_buf diagnostic_root;
    int diagnostic_code;
    const char *diagnostic_label;
    const char *diagnostic_file;
    int diagnostic_line;
    const char *diagnostic_expr;
};
struct LmL5Thread {
    LmL5ExecutionContext main_context;
    LmL5ExecutionContext *current;
    struct LmOwnArena *root_owner;
};
static LmL5Thread lm_l5_main_thread_storage;
static int lm_l5_main_thread_owner_cleanup_registered;
static void lm_l5_main_thread_owner_destroy(void) {
    if (lm_l5_main_thread_storage.root_owner != 0) {
        lm_own_arena_delete(lm_l5_main_thread_storage.root_owner);
        lm_l5_main_thread_storage.root_owner = 0;
    }
}
static inline LmL5Thread *lm_l5_main_thread(void) {
    LmL5Thread *thread = &lm_l5_main_thread_storage;
    if (thread->root_owner == 0) {
        thread->root_owner = lm_own_arena_new();
        if (thread->root_owner == 0) {
            abort();
        }
        if (!lm_l5_main_thread_owner_cleanup_registered) {
            if (atexit(lm_l5_main_thread_owner_destroy) != 0) {
                lm_l5_main_thread_owner_destroy();
                abort();
            }
            lm_l5_main_thread_owner_cleanup_registered = 1;
        }
    }
    return thread;
}
static inline int lm_l5_thread_diagnostic_exit_code(const LmL5Thread *thread) {
    if (thread == 0 || thread->current == 0 || thread->current->diagnostic_code == 0) {
        return 1;
    }
    return thread->current->diagnostic_code;
}

static char * lm_make_env_or_default(char *name, char *fallback) {
    char *value;
    value = getenv(name);
    if (value == 0 || value[0] == '\0') {
        return fallback;
    }
    return value;
}

static size_t lm_make_append(char *buffer, size_t size, size_t used, char *text) {
    size_t length;
    length = strlen(text);
    if (used + length >= size) {
        fprintf(stderr, "make.lm0: command line is too long\n");
        return size;
    }
    memcpy(buffer + used, text, length + 1U);
    return used + length;
}

static size_t lm_make_append_arg(char *buffer, size_t size, size_t used, char *arg) {
    used = lm_make_append(buffer, size, used, " \"");
    if (used == size) {
        return size;
    }
    used = lm_make_append(buffer, size, used, arg);
    if (used == size) {
        return size;
    }
    return lm_make_append(buffer, size, used, "\"");
}

static int lm_make_run_command(char *command) {
    int status;
    printf("%s\n", command);
    status = system(command);
    if (status != 0) {
        fprintf(stderr, "make.lm0: command failed with status %d\n", status);
        return 1;
    }
    return 0;
}

static int lm_make_run_tool(char *tool, int argc, char **argv, int start) {
    char command[8192];
    size_t used = 0U;
    int index;
    index = start;
    command[0] = '\0';
    used = lm_make_append(command, sizeof(command), used, tool);
    if (used == sizeof(command)) {
        return 1;
    }
    while (index < argc) {
        used = lm_make_append_arg(command, sizeof(command), used, argv[index]);
        if (used == sizeof(command)) {
            return 1;
        }
        index = index + 1;
    }
    return lm_make_run_command(command);
}

static int lm_make_copy_file(char *source_path, char *output_path) {
    char buffer[32768];
    FILE * source;
    FILE * output;
    size_t count;
    int status;
    source = fopen(source_path, "rb");
    if (source == 0) {
        fprintf(stderr, "make.lm0: cannot open input file %s\n", source_path);
        return 1;
    }
    output = fopen(output_path, "wb");
    if (output == 0) {
        fprintf(stderr, "make.lm0: cannot open output file %s\n", output_path);
        fclose(source);
        return 1;
    }
    status = 0;
    count = fread(buffer, 1U, sizeof(buffer), source);
    while (count > 0U) {
        if (fwrite(buffer, 1U, count, output) != count) {
            status = 1;
            break;
        }
        count = fread(buffer, 1U, sizeof(buffer), source);
    }
    if (ferror(source)) {
        status = 1;
    }
    if (fclose(output) != 0) {
        status = 1;
    }
    fclose(source);
    if (status != 0) {
        fprintf(stderr, "make.lm0: cannot copy %s to %s\n", source_path, output_path);
        return 1;
    }
    return 0;
}

static void lm_make_print_usage(void) {
    fprintf(stderr, "usage:\n");
    fprintf(stderr, "  make.lm0 mkdir <dir>...\n");
    fprintf(stderr, "  make.lm0 cc <arg>...\n");
    fprintf(stderr, "  make.lm0 link <arg>...\n");
    fprintf(stderr, "  make.lm0 ar <arg>...\n");
    fprintf(stderr, "  make.lm0 ranlib <arg>...\n");
    fprintf(stderr, "  make.lm0 copy <source> <output>\n");
}

int main(int argc, char **argv) {
    LmL5Thread *lm_l5_thread = lm_l5_main_thread();
    lm_l5_thread->current = &lm_l5_thread->main_context;
    lm_l5_thread->main_context.diagnostic_code = 0;
    if (setjmp(lm_l5_thread->main_context.diagnostic_root) != 0) {
        return lm_l5_thread_diagnostic_exit_code(lm_l5_thread);
    }
    char *cmake;
    char *cc;
    char *ar;
    char *ranlib;
    if (argc < 2) {
        lm_make_print_usage();
        return 1;
    }
    cmake = lm_make_env_or_default("LM_CMAKE", "cmake");
    cc = lm_make_env_or_default("LM_CC", "gcc");
    ar = lm_make_env_or_default("LM_AR", "ar");
    ranlib = lm_make_env_or_default("LM_RANLIB", "ranlib");
    if (strcmp(argv[1], "mkdir") == 0) {
        char command[8192];
        size_t used = 0U;
        int index = 2;
        if (argc < 3) {
            lm_make_print_usage();
            return 1;
        }
        command[0] = '\0';
        used = lm_make_append(command, sizeof(command), used, cmake);
        if (used == sizeof(command)) {
            return 1;
        }
        used = lm_make_append(command, sizeof(command), used, " -E make_directory");
        if (used == sizeof(command)) {
            return 1;
        }
        while (index < argc) {
            used = lm_make_append_arg(command, sizeof(command), used, argv[index]);
            if (used == sizeof(command)) {
                return 1;
            }
            index = index + 1;
        }
        return lm_make_run_command(command);
    }
    if (strcmp(argv[1], "cc") == 0 || strcmp(argv[1], "link") == 0) {
        return lm_make_run_tool(cc, argc, argv, 2);
    }
    if (strcmp(argv[1], "ar") == 0) {
        return lm_make_run_tool(ar, argc, argv, 2);
    }
    if (strcmp(argv[1], "ranlib") == 0) {
        return lm_make_run_tool(ranlib, argc, argv, 2);
    }
    if (strcmp(argv[1], "copy") == 0) {
        if (argc != 4) {
            lm_make_print_usage();
            return 1;
        }
        return lm_make_copy_file(argv[2], argv[3]);
    }
    lm_make_print_usage();
    return 1;
}
