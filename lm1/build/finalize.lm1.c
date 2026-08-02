#include <stddef.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>


















#if defined(_WIN32)
/* no POSIX feature macro on Windows */
#else
#define _POSIX_C_SOURCE 199309L
#endif

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


#if defined(_WIN32)
#include <direct.h>
#include <windows.h>
#else
#include <time.h>
#include <unistd.h>
#endif

#if defined(_WIN32)
static char * lm_finalize_platform_getcwd(char *buffer, size_t size);
static int lm_finalize_platform_chdir(const char *path);
static const char * lm_finalize_platform_exe_suffix(void);
static const char * lm_finalize_platform_path_sep(void);
static int lm_finalize_platform_is_drive_absolute(const char *path);
static void lm_finalize_platform_sleep_retry(void);
static const char * lm_finalize_platform_defer_command_format(void);

static char * lm_finalize_platform_getcwd(char *buffer, size_t size) {
    return _getcwd(buffer, size);
}

static int lm_finalize_platform_chdir(const char *path) {
    return _chdir(path);
}

static const char * lm_finalize_platform_exe_suffix(void) {
    return ".exe";
}

static const char * lm_finalize_platform_path_sep(void) {
    return "\\";
}

static int lm_finalize_platform_is_drive_absolute(const char *path) {
    return path[0] != '\0' && path[1] == ':';
}

static void lm_finalize_platform_sleep_retry(void) {
    Sleep(250U);
}

static const char * lm_finalize_platform_defer_command_format(void) {
    return "cd /d \"%s\" && start \"\" /B \"%s\" --copy";
}
#else
static char * lm_finalize_platform_getcwd(char *buffer, size_t size);
static int lm_finalize_platform_chdir(const char *path);
static const char * lm_finalize_platform_exe_suffix(void);
static const char * lm_finalize_platform_path_sep(void);
static int lm_finalize_platform_is_drive_absolute(const char *path);
static void lm_finalize_platform_sleep_retry(void);
static const char * lm_finalize_platform_defer_command_format(void);

static char * lm_finalize_platform_getcwd(char *buffer, size_t size) {
    return getcwd(buffer, size);
}

static int lm_finalize_platform_chdir(const char *path) {
    return chdir(path);
}

static const char * lm_finalize_platform_exe_suffix(void) {
    return "";
}

static const char * lm_finalize_platform_path_sep(void) {
    return "/";
}

static int lm_finalize_platform_is_drive_absolute(const char *path) {
    return path != 0 && 0;
}

static void lm_finalize_platform_sleep_retry(void) {
    struct timespec request;
    request.tv_sec = 0;
    request.tv_nsec = 250000000L;
    nanosleep(&request, 0);
}

static const char * lm_finalize_platform_defer_command_format(void) {
    return "cd \"%s\" && \"%s\" \"--copy\" >/dev/null 2>&1 &";
}
#endif
static int lm_finalize_is_path_separator(char value);
static int lm_finalize_has_path_separator(char *path);
static int lm_finalize_is_absolute_path(char *path);
static int lm_finalize_file_exists(char *path);
static int lm_finalize_join_path(char *buffer, size_t size, char *base, char *tail);
static int lm_finalize_has_project_marker(char *path);
static int lm_finalize_trim_last_path_part(char *path);
static int lm_finalize_enter_project_root(char *program_path);
static void lm_finalize_log(char *message);
static void lm_finalize_sleep_retry(void);
static void lm_finalize_live_tool_path(char *path, size_t size, char *tool_name);
static void lm_finalize_next_tool_path(char *path, size_t size, char *tool_name);
static void lm_finalize_legacy_next_tool_path(char *path, size_t size, char *tool_name);
static void lm_finalize_live_artifact_path(char *path, size_t size, char *artifact_name);
static void lm_finalize_next_artifact_path(char *path, size_t size, char *artifact_name);
static int lm_finalize_copy_once(char *source_path, char *output_path, int quiet);
static int lm_finalize_copy_with_retry(char *source_path, char *output_path);
static int lm_finalize_install_next_tool(char *tool_name);
static int lm_finalize_install_legacy_next_tool(char *tool_name);
static int lm_finalize_install_next_artifact(char *artifact_name);
static int lm_finalize_defer(void);
int main(int argc, char **argv);

struct LmOwnArena;
struct LmMessageThread;
struct LmMessageThread *lm_message_thread_current(void);
struct LmMessageThread *lm_message_thread_new(void);
void lm_message_thread_delete(struct LmMessageThread *thread);
struct LmMessageThread *lm_message_thread_set_current(struct LmMessageThread *thread);
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
void lm_own_arena_freeze(struct LmOwnArena *arena);
void *lm_own_arena_new_zero(struct LmOwnArena *arena, size_t size);
void *lm_own_arena_array_new_zero(struct LmOwnArena *arena, size_t element_size, size_t count, size_t rank, size_t level);
typedef struct LmMessageThreadExecutionContext LmMessageThreadExecutionContext;
struct LmMessageThreadExecutionContext {
    jmp_buf diagnostic_root;
    int diagnostic_code;
    const char *diagnostic_label;
    const char *diagnostic_file;
    int diagnostic_line;
    const char *diagnostic_expr;
};
static LmMessageThreadExecutionContext lm_message_thread_main_context_storage;
static inline int lm_message_thread_diagnostic_status(const LmMessageThreadExecutionContext *context) {
    if (context == 0 || context->diagnostic_code == 0) {
        return 1;
    }
    return context->diagnostic_code;
}

static int lm_finalize_is_path_separator(char value) {
    return value == '/' || value == '\\';
}

static int lm_finalize_has_path_separator(char *path) {
    return strchr(path, '/') != 0 || strchr(path, '\\') != 0;
}

static int lm_finalize_is_absolute_path(char *path) {
    if (path == 0 || path[0] == '\0') {
        return 0;
    }
    if (lm_finalize_is_path_separator(path[0])) {
        return 1;
    }
    if (lm_finalize_platform_is_drive_absolute(path)) {
        return 1;
    }
    return 0;
}

static int lm_finalize_file_exists(char *path) {
    FILE * file;
    file = fopen(path, "rb");
    if (file == 0) {
        return 0;
    }
    fclose(file);
    return 1;
}

static int lm_finalize_join_path(char *buffer, size_t size, char *base, char *tail) {
    size_t base_length;
    size_t tail_length;
    size_t used;
    base_length = strlen(base);
    tail_length = strlen(tail);
    used = base_length;
    if (base_length + tail_length + 2U >= size) {
        fprintf(stderr, "finalize.lm0: path is too long\n");
        return 1;
    }
    memcpy(buffer, base, base_length);
    if (base_length > 0U && tail_length > 0U && lm_finalize_is_path_separator(base[base_length - 1U]) == 0 && lm_finalize_is_path_separator(tail[0]) == 0) {
        memcpy(buffer + used, lm_finalize_platform_path_sep(), strlen(lm_finalize_platform_path_sep()));
        used = used + strlen(lm_finalize_platform_path_sep());
    }
    memcpy(buffer + used, tail, tail_length + 1U);
    return 0;
}

static int lm_finalize_has_project_marker(char *path) {
    char marker_path[2048];
    if (lm_finalize_join_path(marker_path, sizeof(marker_path), path, "lm2/buildCore.lmx") != 0) {
        return 0;
    }
    if (lm_finalize_file_exists(marker_path)) {
        return 1;
    }
    return 0;
}

static int lm_finalize_trim_last_path_part(char *path) {
    size_t length;
    length = strlen(path);
    while (length > 0U && lm_finalize_is_path_separator(path[length - 1U])) {
        path[length-1U] = '\0';
        length = length - 1U;
    }
    while (length > 0U) {
        length = length - 1U;
        if (lm_finalize_is_path_separator(path[length])) {
            path[length] = '\0';
            return 0;
        }
    }
    path[0] = '\0';
    return 0;
}

static int lm_finalize_enter_project_root(char *program_path) {
    char search_path[1024];
    char executable_path[1024];
    char cwd[1024];
    int depth;
    if (lm_finalize_platform_getcwd(cwd, sizeof(cwd)) == 0) {
        fprintf(stderr, "finalize.lm0: cannot read current directory\n");
        return 1;
    }
    if (program_path != 0 && program_path[0] != '\0' && lm_finalize_has_path_separator(program_path)) {
        if (lm_finalize_is_absolute_path(program_path)) {
            if (strlen(program_path) >= sizeof(executable_path)) {
                fprintf(stderr, "finalize.lm0: executable path is too long\n");
                return 1;
            }
            strcpy(executable_path, program_path);
        }
        if (lm_finalize_is_absolute_path(program_path) == 0) {
            if (lm_finalize_join_path(executable_path, sizeof(executable_path), cwd, program_path) != 0) {
                return 1;
            }
        }
        strcpy(search_path, executable_path);
        lm_finalize_trim_last_path_part(search_path);
    }
    if (program_path == 0 || program_path[0] == '\0' || lm_finalize_has_path_separator(program_path) == 0) {
        strcpy(search_path, cwd);
    }
    depth = 0;
    while (depth < 12 && search_path[0] != '\0') {
        if (lm_finalize_has_project_marker(search_path)) {
            if (lm_finalize_platform_chdir(search_path) != 0) {
                fprintf(stderr, "finalize.lm0: cannot enter project root %s\n", search_path);
                return 1;
            }
            return 0;
        }
        lm_finalize_trim_last_path_part(search_path);
        depth = depth + 1;
    }
    fprintf(stderr, "finalize.lm0: cannot locate project root from %s\n", cwd);
    return 1;
    return 0;
}

static void lm_finalize_log(char *message) {
    FILE * log_file;
    log_file = fopen("build/lm0/finalize.log", "a");
    if (log_file == 0) {
        return;
    }
    fprintf(log_file, "%s\n", message);
    fclose(log_file);
}

static void lm_finalize_sleep_retry(void) {
    lm_finalize_platform_sleep_retry();
}

static void lm_finalize_live_tool_path(char *path, size_t size, char *tool_name) {
    snprintf(path, size, "build%slm0%s%s.lm0%s", lm_finalize_platform_path_sep(), lm_finalize_platform_path_sep(), tool_name, lm_finalize_platform_exe_suffix());
}

static void lm_finalize_next_tool_path(char *path, size_t size, char *tool_name) {
    snprintf(path, size, "build%slm0%snext%s%s.lm0%s", lm_finalize_platform_path_sep(), lm_finalize_platform_path_sep(), lm_finalize_platform_path_sep(), tool_name, lm_finalize_platform_exe_suffix());
}

static void lm_finalize_legacy_next_tool_path(char *path, size_t size, char *tool_name) {
    snprintf(path, size, "build%slm0%s%s.next.lm0%s", lm_finalize_platform_path_sep(), lm_finalize_platform_path_sep(), tool_name, lm_finalize_platform_exe_suffix());
}

static void lm_finalize_live_artifact_path(char *path, size_t size, char *artifact_name) {
    snprintf(path, size, "build%slm0%s%s", lm_finalize_platform_path_sep(), lm_finalize_platform_path_sep(), artifact_name);
}

static void lm_finalize_next_artifact_path(char *path, size_t size, char *artifact_name) {
    snprintf(path, size, "build%slm0%snext%s%s", lm_finalize_platform_path_sep(), lm_finalize_platform_path_sep(), lm_finalize_platform_path_sep(), artifact_name);
}

static int lm_finalize_copy_once(char *source_path, char *output_path, int quiet) {
    char buffer[32768];
    FILE * source;
    FILE * output;
    size_t count;
    int status;
    source = fopen(source_path, "rb");
    if (source == 0) {
        if (quiet == 0) {
            fprintf(stderr, "finalize.lm0: cannot open input file %s\n", source_path);
        }
        return 1;
    }
    output = fopen(output_path, "wb");
    if (output == 0) {
        if (quiet == 0) {
            fprintf(stderr, "finalize.lm0: cannot open output file %s\n", output_path);
        }
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
        if (quiet == 0) {
            fprintf(stderr, "finalize.lm0: cannot copy %s to %s\n", source_path, output_path);
        }
        return 1;
    }
    return 0;
}

static int lm_finalize_copy_with_retry(char *source_path, char *output_path) {
    int attempt = 0;
    while (attempt < 40) {
        if (lm_finalize_copy_once(source_path, output_path, 1) == 0) {
            printf("finalize.lm0: copied %s to %s\n", source_path, output_path);
            return 0;
        }
        lm_finalize_sleep_retry();
        attempt = attempt + 1;
    }
    if (lm_finalize_copy_once(source_path, output_path, 0) != 0) {
        return 1;
    }
    printf("finalize.lm0: copied %s to %s\n", source_path, output_path);
    return 0;
}

static int lm_finalize_install_next_tool(char *tool_name) {
    char source_path[256];
    char output_path[256];
    lm_finalize_next_tool_path(source_path, sizeof(source_path), tool_name);
    lm_finalize_live_tool_path(output_path, sizeof(output_path), tool_name);
    if (lm_finalize_copy_with_retry(source_path, output_path) != 0) {
        lm_finalize_log("copy failed");
        return 1;
    }
    return 0;
}

static int lm_finalize_install_legacy_next_tool(char *tool_name) {
    char source_path[256];
    char output_path[256];
    lm_finalize_legacy_next_tool_path(source_path, sizeof(source_path), tool_name);
    lm_finalize_live_tool_path(output_path, sizeof(output_path), tool_name);
    if (lm_finalize_copy_with_retry(source_path, output_path) != 0) {
        lm_finalize_log("copy failed");
        return 1;
    }
    if (remove(source_path) != 0) {
        fprintf(stderr, "finalize.lm0: cannot remove temporary file %s\n", source_path);
        lm_finalize_log("remove failed");
        return 1;
    }
    return 0;
}

static int lm_finalize_install_next_artifact(char *artifact_name) {
    char source_path[256];
    char output_path[256];
    lm_finalize_next_artifact_path(source_path, sizeof(source_path), artifact_name);
    lm_finalize_live_artifact_path(output_path, sizeof(output_path), artifact_name);
    if (lm_finalize_copy_with_retry(source_path, output_path) != 0) {
        lm_finalize_log("copy failed");
        return 1;
    }
    return 0;
}

static int lm_finalize_defer(void) {
    char root_path[1024];
    char finalize_path[1400];
    char command[4096];
    int status;
    if (lm_finalize_platform_getcwd(root_path, sizeof(root_path)) == 0) {
        fprintf(stderr, "finalize.lm0: cannot get current directory\n");
        return 1;
    }
    snprintf(finalize_path, sizeof(finalize_path), "build%slm0%snext%sfinalize.lm0%s", lm_finalize_platform_path_sep(), lm_finalize_platform_path_sep(), lm_finalize_platform_path_sep(), lm_finalize_platform_exe_suffix());
    if (lm_finalize_file_exists(finalize_path) == 0) {
        lm_finalize_live_tool_path(finalize_path, sizeof(finalize_path), "finalize");
    }
    snprintf(command, sizeof(command), lm_finalize_platform_defer_command_format(), root_path, finalize_path);
    status = system(command);
    if (status != 0) {
        fprintf(stderr, "finalize.lm0: cannot schedule deferred finalize step\n");
        lm_finalize_log("defer failed");
        return 1;
    }
    lm_finalize_log("defer scheduled");
    printf("finalize.lm0: scheduled deferred staged install\n");
    return 0;
}

int main(int argc, char **argv) {
    struct LmMessageThread *lm_message_thread = lm_message_thread_current();
    void *lm_message_thread_previous_context;
    if (lm_message_thread == 0) {
        return 1;
    }
    lm_message_thread_previous_context = lm_message_thread_set_execution_context(lm_message_thread, &lm_message_thread_main_context_storage);
    while (lm_message_thread_begin_turn(lm_message_thread)) {
        lm_message_thread_main_context_storage.diagnostic_code = 0;
        if (setjmp(lm_message_thread_main_context_storage.diagnostic_root) == 0) {
            char staged_build_core_path[256];
            if (lm_finalize_enter_project_root(argv[0]) != 0) {
                {
                    int lm_return_0 = 1;
                    lm_message_thread_request_stop(lm_message_thread, lm_return_0);
                    goto lm_message_thread_turn_end;
                }
            }
            if (argc == 2 && strcmp(argv[1], "--defer") == 0) {
                {
                    int lm_return_1 = lm_finalize_defer();
                    lm_message_thread_request_stop(lm_message_thread, lm_return_1);
                    goto lm_message_thread_turn_end;
                }
            }
            if (argc != 1 && (argc != 2 || strcmp(argv[1], "--copy") != 0)) {
                fprintf(stderr, "usage: finalize.lm0[.exe] [--defer|--copy]\n");
                {
                    int lm_return_2 = 1;
                    lm_message_thread_request_stop(lm_message_thread, lm_return_2);
                    goto lm_message_thread_turn_end;
                }
            }
            lm_finalize_log("copy started");
            lm_finalize_next_tool_path(staged_build_core_path, sizeof(staged_build_core_path), "buildCore");
            if (lm_finalize_file_exists(staged_build_core_path) == 0) {
                if (lm_finalize_install_legacy_next_tool("make") != 0) {
                    {
                        int lm_return_3 = 1;
                        lm_message_thread_request_stop(lm_message_thread, lm_return_3);
                        goto lm_message_thread_turn_end;
                    }
                }
                if (lm_finalize_install_legacy_next_tool("buildCore") != 0) {
                    {
                        int lm_return_4 = 1;
                        lm_message_thread_request_stop(lm_message_thread, lm_return_4);
                        goto lm_message_thread_turn_end;
                    }
                }
                lm_finalize_log("legacy copy completed");
                {
                    int lm_return_5 = 0;
                    lm_message_thread_request_stop(lm_message_thread, lm_return_5);
                    goto lm_message_thread_turn_end;
                }
            }
            if (lm_finalize_install_next_artifact("libparser.lm0.a") != 0) {
                {
                    int lm_return_6 = 1;
                    lm_message_thread_request_stop(lm_message_thread, lm_return_6);
                    goto lm_message_thread_turn_end;
                }
            }
            if (lm_finalize_install_next_artifact("libown.lm0.a") != 0) {
                {
                    int lm_return_7 = 1;
                    lm_message_thread_request_stop(lm_message_thread, lm_return_7);
                    goto lm_message_thread_turn_end;
                }
            }
            if (lm_finalize_install_next_tool("make") != 0) {
                {
                    int lm_return_8 = 1;
                    lm_message_thread_request_stop(lm_message_thread, lm_return_8);
                    goto lm_message_thread_turn_end;
                }
            }
            if (lm_finalize_install_next_tool("trans") != 0) {
                {
                    int lm_return_9 = 1;
                    lm_message_thread_request_stop(lm_message_thread, lm_return_9);
                    goto lm_message_thread_turn_end;
                }
            }
            if (lm_finalize_install_next_tool("vcpkgFetch") != 0) {
                {
                    int lm_return_10 = 1;
                    lm_message_thread_request_stop(lm_message_thread, lm_return_10);
                    goto lm_message_thread_turn_end;
                }
            }
            if (lm_finalize_install_next_tool("printTree") != 0) {
                {
                    int lm_return_11 = 1;
                    lm_message_thread_request_stop(lm_message_thread, lm_return_11);
                    goto lm_message_thread_turn_end;
                }
            }
            if (lm_finalize_install_next_tool("finalize") != 0) {
                {
                    int lm_return_12 = 1;
                    lm_message_thread_request_stop(lm_message_thread, lm_return_12);
                    goto lm_message_thread_turn_end;
                }
            }
            if (lm_finalize_install_next_tool("buildCore") != 0) {
                {
                    int lm_return_13 = 1;
                    lm_message_thread_request_stop(lm_message_thread, lm_return_13);
                    goto lm_message_thread_turn_end;
                }
            }
            lm_finalize_log("copy completed");
            {
                int lm_return_14 = 0;
                lm_message_thread_request_stop(lm_message_thread, lm_return_14);
                goto lm_message_thread_turn_end;
            }
        } else {
            lm_message_thread_request_failure(lm_message_thread, lm_message_thread_diagnostic_status(&lm_message_thread_main_context_storage));
        }
    lm_message_thread_turn_end:
        (void)lm_message_thread_end_turn(lm_message_thread);
    }
    lm_message_thread_set_execution_context(lm_message_thread, lm_message_thread_previous_context);
    return lm_message_thread_status(lm_message_thread);
}
