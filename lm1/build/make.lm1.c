#include <stddef.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>

struct LmOwnArena;
struct LmMessageThread;
struct LmMessageThread *lm_message_thread_new(void);
void lm_message_thread_delete(struct LmMessageThread *thread);
struct LmOwnArena *lm_message_thread_owner(struct LmMessageThread *thread);
void *lm_message_thread_execution_context(struct LmMessageThread *thread);
void *lm_message_thread_set_execution_context(struct LmMessageThread *thread, void *context);
int lm_message_thread_begin_turn(struct LmMessageThread *thread);
int lm_message_thread_end_turn(struct LmMessageThread *thread);
void lm_message_thread_request_stop(struct LmMessageThread *thread, int status);
void lm_message_thread_request_failure(struct LmMessageThread *thread, int status);
int lm_message_thread_status(const struct LmMessageThread *thread);
int lm_message_thread_is_running(const struct LmMessageThread *thread);
size_t lm_message_thread_turn_count(const struct LmMessageThread *thread);
size_t lm_message_thread_collection_count(const struct LmMessageThread *thread);
void lm_own_arena_freeze(struct LmMessageThread *lm_lmx_message_thread, struct LmOwnArena *arena);
void *lm_own_arena_new_zero(struct LmMessageThread *lm_lmx_message_thread, struct LmOwnArena *arena, size_t size);
void *lm_own_arena_array_new_zero(struct LmMessageThread *lm_lmx_message_thread, struct LmOwnArena *arena, size_t element_size, size_t count, size_t rank, size_t level);
typedef struct LmMessageThreadExecutionContext LmMessageThreadExecutionContext;
struct LmMessageThreadExecutionContext {
    jmp_buf diagnostic_root;
    int diagnostic_code;
    const char *diagnostic_label;
    const char *diagnostic_file;
    int diagnostic_line;
    const char *diagnostic_expr;
};
typedef int (*LmLmxMessageThreadEntry)(struct LmMessageThread *thread);
#if defined(__GNUC__) || defined(__clang__)
#define LM_LMX_UNUSED_ENTRY_HELPER __attribute__((unused))
#else
#define LM_LMX_UNUSED_ENTRY_HELPER
#endif
static inline LM_LMX_UNUSED_ENTRY_HELPER int lm_lmx_message_thread_run_entry(LmLmxMessageThreadEntry entry) {
    struct LmMessageThread *thread;
    LmMessageThreadExecutionContext context = {0};
    int status;
    if (entry == 0) {
        return 1;
    }
    thread = lm_message_thread_new();
    if (thread == 0) {
        return 1;
    }
    (void)lm_message_thread_set_execution_context(thread, &context);
    while (lm_message_thread_begin_turn(thread)) {
        context.diagnostic_code = 0;
        if (setjmp(context.diagnostic_root) == 0) {
            lm_message_thread_request_stop(thread, entry(thread));
        } else {
            lm_message_thread_request_failure(thread, context.diagnostic_code == 0 ? 1 : context.diagnostic_code);
        }
        (void)lm_message_thread_end_turn(thread);
    }
    status = lm_message_thread_status(thread);
    lm_message_thread_delete(thread);
    return status;
}
#undef LM_LMX_UNUSED_ENTRY_HELPER

















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

static char * lm_make_env_or_default(struct LmMessageThread *lm_lmx_message_thread, char *name, char *fallback);
static size_t lm_make_append(struct LmMessageThread *lm_lmx_message_thread, char *buffer, size_t size, size_t used, char *text);
static size_t lm_make_append_arg(struct LmMessageThread *lm_lmx_message_thread, char *buffer, size_t size, size_t used, char *arg);
static int lm_make_run_command(struct LmMessageThread *lm_lmx_message_thread, char *command);
static int lm_make_run_tool(struct LmMessageThread *lm_lmx_message_thread, char *tool, int argc, char **argv, int start);
static int lm_make_copy_file(struct LmMessageThread *lm_lmx_message_thread, char *source_path, char *output_path);
static void lm_make_print_usage(struct LmMessageThread *lm_lmx_message_thread);
int main(int argc, char **argv);

static inline int lm_message_thread_diagnostic_status(const LmMessageThreadExecutionContext *context) {
    if (context == 0 || context->diagnostic_code == 0) {
        return 1;
    }
    return context->diagnostic_code;
}

static char * lm_make_env_or_default(struct LmMessageThread *lm_lmx_message_thread, char *name, char *fallback) {
    (void)lm_lmx_message_thread;
    char *value;
    value = getenv(name);
    if (value == 0 || value[0] == '\0') {
        return fallback;
    }
    return value;
}

static size_t lm_make_append(struct LmMessageThread *lm_lmx_message_thread, char *buffer, size_t size, size_t used, char *text) {
    (void)lm_lmx_message_thread;
    size_t length;
    length = strlen(text);
    if (used + length >= size) {
        fprintf(stderr, "make.lm0: command line is too long\n");
        return size;
    }
    memcpy(buffer + used, text, length + 1U);
    return used + length;
}

static size_t lm_make_append_arg(struct LmMessageThread *lm_lmx_message_thread, char *buffer, size_t size, size_t used, char *arg) {
    (void)lm_lmx_message_thread;
    used = lm_make_append(lm_lmx_message_thread, buffer, size, used, " \"");
    if (used == size) {
        return size;
    }
    used = lm_make_append(lm_lmx_message_thread, buffer, size, used, arg);
    if (used == size) {
        return size;
    }
    return lm_make_append(lm_lmx_message_thread, buffer, size, used, "\"");
}

static int lm_make_run_command(struct LmMessageThread *lm_lmx_message_thread, char *command) {
    (void)lm_lmx_message_thread;
    int status;
    printf("%s\n", command);
    status = system(command);
    if (status != 0) {
        fprintf(stderr, "make.lm0: command failed with status %d\n", status);
        return 1;
    }
    return 0;
}

static int lm_make_run_tool(struct LmMessageThread *lm_lmx_message_thread, char *tool, int argc, char **argv, int start) {
    (void)lm_lmx_message_thread;
    char command[8192];
    size_t used = 0U;
    int index;
    index = start;
    command[0] = '\0';
    used = lm_make_append(lm_lmx_message_thread, command, sizeof(command), used, tool);
    if (used == sizeof(command)) {
        return 1;
    }
    while (index < argc) {
        used = lm_make_append_arg(lm_lmx_message_thread, command, sizeof(command), used, argv[index]);
        if (used == sizeof(command)) {
            return 1;
        }
        index = index + 1;
    }
    return lm_make_run_command(lm_lmx_message_thread, command);
}

static int lm_make_copy_file(struct LmMessageThread *lm_lmx_message_thread, char *source_path, char *output_path) {
    (void)lm_lmx_message_thread;
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

static void lm_make_print_usage(struct LmMessageThread *lm_lmx_message_thread) {
    (void)lm_lmx_message_thread;
    fprintf(stderr, "usage:\n");
    fprintf(stderr, "  make.lm0 mkdir <dir>...\n");
    fprintf(stderr, "  make.lm0 cc <arg>...\n");
    fprintf(stderr, "  make.lm0 link <arg>...\n");
    fprintf(stderr, "  make.lm0 ar <arg>...\n");
    fprintf(stderr, "  make.lm0 ranlib <arg>...\n");
    fprintf(stderr, "  make.lm0 copy <source> <output>\n");
}

int main(int argc, char **argv) {
    struct LmMessageThread *lm_lmx_message_thread;
    LmMessageThreadExecutionContext lm_message_thread_main_context = {0};
    int lm_message_thread_exit_status;
    lm_lmx_message_thread = lm_message_thread_new();
    if (lm_lmx_message_thread == 0) {
        return 1;
    }
    (void)lm_message_thread_set_execution_context(lm_lmx_message_thread, &lm_message_thread_main_context);
    while (lm_message_thread_begin_turn(lm_lmx_message_thread)) {
        lm_message_thread_main_context.diagnostic_code = 0;
        if (setjmp(lm_message_thread_main_context.diagnostic_root) == 0) {
            char *cmake;
            char *cc;
            char *ar;
            char *ranlib;
            if (argc < 2) {
                lm_make_print_usage(lm_lmx_message_thread);
                {
                    int lm_return_0 = 1;
                    lm_message_thread_request_stop(lm_lmx_message_thread, lm_return_0);
                    goto lm_message_thread_turn_end;
                }
            }
            cmake = lm_make_env_or_default(lm_lmx_message_thread, "LM_CMAKE", "cmake");
            cc = lm_make_env_or_default(lm_lmx_message_thread, "LM_CC", "gcc");
            ar = lm_make_env_or_default(lm_lmx_message_thread, "LM_AR", "ar");
            ranlib = lm_make_env_or_default(lm_lmx_message_thread, "LM_RANLIB", "ranlib");
            if (strcmp(argv[1], "mkdir") == 0) {
                char command[8192];
                size_t used = 0U;
                int index = 2;
                if (argc < 3) {
                    lm_make_print_usage(lm_lmx_message_thread);
                    {
                        int lm_return_1 = 1;
                        lm_message_thread_request_stop(lm_lmx_message_thread, lm_return_1);
                        goto lm_message_thread_turn_end;
                    }
                }
                command[0] = '\0';
                used = lm_make_append(lm_lmx_message_thread, command, sizeof(command), used, cmake);
                if (used == sizeof(command)) {
                    {
                        int lm_return_2 = 1;
                        lm_message_thread_request_stop(lm_lmx_message_thread, lm_return_2);
                        goto lm_message_thread_turn_end;
                    }
                }
                used = lm_make_append(lm_lmx_message_thread, command, sizeof(command), used, " -E make_directory");
                if (used == sizeof(command)) {
                    {
                        int lm_return_3 = 1;
                        lm_message_thread_request_stop(lm_lmx_message_thread, lm_return_3);
                        goto lm_message_thread_turn_end;
                    }
                }
                while (index < argc) {
                    used = lm_make_append_arg(lm_lmx_message_thread, command, sizeof(command), used, argv[index]);
                    if (used == sizeof(command)) {
                        {
                            int lm_return_4 = 1;
                            lm_message_thread_request_stop(lm_lmx_message_thread, lm_return_4);
                            goto lm_message_thread_turn_end;
                        }
                    }
                    index = index + 1;
                }
                {
                    int lm_return_5 = lm_make_run_command(lm_lmx_message_thread, command);
                    lm_message_thread_request_stop(lm_lmx_message_thread, lm_return_5);
                    goto lm_message_thread_turn_end;
                }
            }
            if (strcmp(argv[1], "cc") == 0 || strcmp(argv[1], "link") == 0) {
                {
                    int lm_return_6 = lm_make_run_tool(lm_lmx_message_thread, cc, argc, argv, 2);
                    lm_message_thread_request_stop(lm_lmx_message_thread, lm_return_6);
                    goto lm_message_thread_turn_end;
                }
            }
            if (strcmp(argv[1], "ar") == 0) {
                {
                    int lm_return_7 = lm_make_run_tool(lm_lmx_message_thread, ar, argc, argv, 2);
                    lm_message_thread_request_stop(lm_lmx_message_thread, lm_return_7);
                    goto lm_message_thread_turn_end;
                }
            }
            if (strcmp(argv[1], "ranlib") == 0) {
                {
                    int lm_return_8 = lm_make_run_tool(lm_lmx_message_thread, ranlib, argc, argv, 2);
                    lm_message_thread_request_stop(lm_lmx_message_thread, lm_return_8);
                    goto lm_message_thread_turn_end;
                }
            }
            if (strcmp(argv[1], "copy") == 0) {
                if (argc != 4) {
                    lm_make_print_usage(lm_lmx_message_thread);
                    {
                        int lm_return_9 = 1;
                        lm_message_thread_request_stop(lm_lmx_message_thread, lm_return_9);
                        goto lm_message_thread_turn_end;
                    }
                }
                {
                    int lm_return_10 = lm_make_copy_file(lm_lmx_message_thread, argv[2], argv[3]);
                    lm_message_thread_request_stop(lm_lmx_message_thread, lm_return_10);
                    goto lm_message_thread_turn_end;
                }
            }
            lm_make_print_usage(lm_lmx_message_thread);
            {
                int lm_return_11 = 1;
                lm_message_thread_request_stop(lm_lmx_message_thread, lm_return_11);
                goto lm_message_thread_turn_end;
            }
        } else {
            lm_message_thread_request_failure(lm_lmx_message_thread, lm_message_thread_diagnostic_status(&lm_message_thread_main_context));
        }
    lm_message_thread_turn_end:
        (void)lm_message_thread_end_turn(lm_lmx_message_thread);
    }
    lm_message_thread_exit_status = lm_message_thread_status(lm_lmx_message_thread);
    lm_message_thread_delete(lm_lmx_message_thread);
    return lm_message_thread_exit_status;
}
