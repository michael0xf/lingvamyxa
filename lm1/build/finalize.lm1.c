#include <stddef.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>

struct LmOwnArena;
struct LmMessageThread;
struct LmMessageThreadRuntime;
struct LmMessageThreadPool;
#ifndef LM_LMX_LAYOUT_DEFINED_IncomingMessage
#define LM_LMX_LAYOUT_DEFINED_IncomingMessage 1
typedef struct IncomingMessage IncomingMessage;
struct IncomingMessage {
    const char *lmx;
    size_t length;
};
#endif
struct LmMessageThread *lm_message_thread_new(void);
void lm_message_thread_delete(struct LmMessageThread *thread);
struct LmMessageThreadRuntime *lm_message_thread_runtime_new(void);
int lm_message_thread_runtime_attach_root(struct LmMessageThreadRuntime *runtime, struct LmMessageThread *thread);
int lm_message_thread_runtime_detach_root(struct LmMessageThreadRuntime *runtime, struct LmMessageThread *thread);
int lm_message_thread_runtime_exit_state(struct LmMessageThreadRuntime *runtime, int *requested, int *ready, int *status);
int lm_message_thread_runtime_delete(struct LmMessageThreadRuntime *runtime);
struct LmMessageThreadPool *lm_message_thread_pool_new(struct LmMessageThreadRuntime *runtime, size_t worker_count);
void lm_message_thread_pool_request_stop(struct LmMessageThreadPool *pool);
void lm_message_thread_pool_request_stop_when_idle(struct LmMessageThreadPool *pool);
size_t lm_message_thread_pool_pump(struct LmMessageThreadPool *pool, size_t max_turns);
int lm_message_thread_pool_delete(struct LmMessageThreadPool *pool);
struct LmMessageThread *lm_message_thread_new_in(struct LmMessageThreadPool *pool);
int lm_message_thread_start_mailbox(struct LmMessageThread *thread, void (*entry)(struct LmMessageThread *, void *), void *argument);
int lm_message_thread_join(struct LmMessageThread *thread, int *result);
int lm_message_thread_bind_route(struct LmMessageThread *thread, const char *route);
int lm_message_thread_current_lmx(struct LmMessageThread *thread, const char **out_lmx, size_t *out_length);
int lm_message_thread_send_lmx(struct LmMessageThread *thread, const char *endpoint, const char *route, const char *lmx, size_t length);
struct LmOwnArena *lm_message_thread_owner(struct LmMessageThread *thread);
void *lm_message_thread_execution_context(struct LmMessageThread *thread);
void *lm_message_thread_set_execution_context(struct LmMessageThread *thread, void *context);
int lm_message_thread_begin_turn(struct LmMessageThread *thread);
int lm_message_thread_end_turn(struct LmMessageThread *thread);
void lm_message_thread_request_stop(struct LmMessageThread *thread, int status);
void lm_message_thread_request_failure(struct LmMessageThread *thread, int status);
int lm_message_thread_request_exit(struct LmMessageThread *thread, int status);
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
    struct LmMessageThreadRuntime *runtime;
    LmMessageThreadExecutionContext context = {0};
    int status;
    int attached = 0;
    if (entry == 0) {
        return 1;
    }
    thread = lm_message_thread_new();
    if (thread == 0) {
        return 1;
    }
    runtime = lm_message_thread_runtime_new();
    if (runtime == 0) {
        lm_message_thread_delete(thread);
        return 1;
    }
    if (lm_message_thread_runtime_attach_root(runtime, thread) != 0) {
        (void)lm_message_thread_runtime_delete(runtime);
        lm_message_thread_delete(thread);
        return 1;
    }
    attached = 1;
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
    if (lm_message_thread_runtime_detach_root(runtime, thread) != 0) {
        if (status == 0) status = 1;
    } else {
        attached = 0;
    }
    if (!attached && lm_message_thread_runtime_delete(runtime) != 0 && status == 0) status = 1;
    if (!attached) lm_message_thread_delete(thread);
    return status;
}
#undef LM_LMX_UNUSED_ENTRY_HELPER

















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
static char * lm_finalize_platform_getcwd(struct LmMessageThread *lm_lmx_message_thread, char *buffer, size_t size);
static int lm_finalize_platform_chdir(struct LmMessageThread *lm_lmx_message_thread, const char *path);
static const char * lm_finalize_platform_exe_suffix(struct LmMessageThread *lm_lmx_message_thread);
static const char * lm_finalize_platform_path_sep(struct LmMessageThread *lm_lmx_message_thread);
static int lm_finalize_platform_is_drive_absolute(struct LmMessageThread *lm_lmx_message_thread, const char *path);
static void lm_finalize_platform_sleep_retry(struct LmMessageThread *lm_lmx_message_thread);
static const char * lm_finalize_platform_defer_command_format(struct LmMessageThread *lm_lmx_message_thread);

static char * lm_finalize_platform_getcwd(struct LmMessageThread *lm_lmx_message_thread, char *buffer, size_t size) {
    (void)lm_lmx_message_thread;
    return _getcwd(buffer, size);
}

static int lm_finalize_platform_chdir(struct LmMessageThread *lm_lmx_message_thread, const char *path) {
    (void)lm_lmx_message_thread;
    return _chdir(path);
}

static const char * lm_finalize_platform_exe_suffix(struct LmMessageThread *lm_lmx_message_thread) {
    (void)lm_lmx_message_thread;
    return ".exe";
}

static const char * lm_finalize_platform_path_sep(struct LmMessageThread *lm_lmx_message_thread) {
    (void)lm_lmx_message_thread;
    return "\\";
}

static int lm_finalize_platform_is_drive_absolute(struct LmMessageThread *lm_lmx_message_thread, const char *path) {
    (void)lm_lmx_message_thread;
    return path[0] != '\0' && path[1] == ':';
}

static void lm_finalize_platform_sleep_retry(struct LmMessageThread *lm_lmx_message_thread) {
    (void)lm_lmx_message_thread;
    Sleep(250U);
}

static const char * lm_finalize_platform_defer_command_format(struct LmMessageThread *lm_lmx_message_thread) {
    (void)lm_lmx_message_thread;
    return "cd /d \"%s\" && start \"\" /B \"%s\" --copy";
}
#else
static char * lm_finalize_platform_getcwd(struct LmMessageThread *lm_lmx_message_thread, char *buffer, size_t size);
static int lm_finalize_platform_chdir(struct LmMessageThread *lm_lmx_message_thread, const char *path);
static const char * lm_finalize_platform_exe_suffix(struct LmMessageThread *lm_lmx_message_thread);
static const char * lm_finalize_platform_path_sep(struct LmMessageThread *lm_lmx_message_thread);
static int lm_finalize_platform_is_drive_absolute(struct LmMessageThread *lm_lmx_message_thread, const char *path);
static void lm_finalize_platform_sleep_retry(struct LmMessageThread *lm_lmx_message_thread);
static const char * lm_finalize_platform_defer_command_format(struct LmMessageThread *lm_lmx_message_thread);

static char * lm_finalize_platform_getcwd(struct LmMessageThread *lm_lmx_message_thread, char *buffer, size_t size) {
    (void)lm_lmx_message_thread;
    return getcwd(buffer, size);
}

static int lm_finalize_platform_chdir(struct LmMessageThread *lm_lmx_message_thread, const char *path) {
    (void)lm_lmx_message_thread;
    return chdir(path);
}

static const char * lm_finalize_platform_exe_suffix(struct LmMessageThread *lm_lmx_message_thread) {
    (void)lm_lmx_message_thread;
    return "";
}

static const char * lm_finalize_platform_path_sep(struct LmMessageThread *lm_lmx_message_thread) {
    (void)lm_lmx_message_thread;
    return "/";
}

static int lm_finalize_platform_is_drive_absolute(struct LmMessageThread *lm_lmx_message_thread, const char *path) {
    (void)lm_lmx_message_thread;
    return path != 0 && 0;
}

static void lm_finalize_platform_sleep_retry(struct LmMessageThread *lm_lmx_message_thread) {
    (void)lm_lmx_message_thread;
    struct timespec request;
    request.tv_sec = 0;
    request.tv_nsec = 250000000L;
    nanosleep(&request, 0);
}

static const char * lm_finalize_platform_defer_command_format(struct LmMessageThread *lm_lmx_message_thread) {
    (void)lm_lmx_message_thread;
    return "cd \"%s\" && \"%s\" \"--copy\" >/dev/null 2>&1 &";
}
#endif
static int lm_finalize_is_path_separator(struct LmMessageThread *lm_lmx_message_thread, char value);
static int lm_finalize_has_path_separator(struct LmMessageThread *lm_lmx_message_thread, char *path);
static int lm_finalize_is_absolute_path(struct LmMessageThread *lm_lmx_message_thread, char *path);
static int lm_finalize_file_exists(struct LmMessageThread *lm_lmx_message_thread, char *path);
static int lm_finalize_join_path(struct LmMessageThread *lm_lmx_message_thread, char *buffer, size_t size, char *base, char *tail);
static int lm_finalize_has_project_marker(struct LmMessageThread *lm_lmx_message_thread, char *path);
static int lm_finalize_trim_last_path_part(struct LmMessageThread *lm_lmx_message_thread, char *path);
static int lm_finalize_enter_project_root(struct LmMessageThread *lm_lmx_message_thread, char *program_path);
static void lm_finalize_log(struct LmMessageThread *lm_lmx_message_thread, char *message);
static void lm_finalize_sleep_retry(struct LmMessageThread *lm_lmx_message_thread);
static void lm_finalize_live_tool_path(struct LmMessageThread *lm_lmx_message_thread, char *path, size_t size, char *tool_name);
static void lm_finalize_next_tool_path(struct LmMessageThread *lm_lmx_message_thread, char *path, size_t size, char *tool_name);
static void lm_finalize_legacy_next_tool_path(struct LmMessageThread *lm_lmx_message_thread, char *path, size_t size, char *tool_name);
static void lm_finalize_live_artifact_path(struct LmMessageThread *lm_lmx_message_thread, char *path, size_t size, char *artifact_name);
static void lm_finalize_next_artifact_path(struct LmMessageThread *lm_lmx_message_thread, char *path, size_t size, char *artifact_name);
static int lm_finalize_copy_once(struct LmMessageThread *lm_lmx_message_thread, char *source_path, char *output_path, int quiet);
static int lm_finalize_copy_with_retry(struct LmMessageThread *lm_lmx_message_thread, char *source_path, char *output_path);
static int lm_finalize_install_next_tool(struct LmMessageThread *lm_lmx_message_thread, char *tool_name);
static int lm_finalize_install_legacy_next_tool(struct LmMessageThread *lm_lmx_message_thread, char *tool_name);
static int lm_finalize_install_next_artifact(struct LmMessageThread *lm_lmx_message_thread, char *artifact_name);
static int lm_finalize_defer(struct LmMessageThread *lm_lmx_message_thread);
int main(int argc, char **argv);

static inline int lm_message_thread_diagnostic_status(const LmMessageThreadExecutionContext *context) {
    if (context == 0 || context->diagnostic_code == 0) {
        return 1;
    }
    return context->diagnostic_code;
}

static int lm_finalize_is_path_separator(struct LmMessageThread *lm_lmx_message_thread, char value) {
    (void)lm_lmx_message_thread;
    return value == '/' || value == '\\';
}

static int lm_finalize_has_path_separator(struct LmMessageThread *lm_lmx_message_thread, char *path) {
    (void)lm_lmx_message_thread;
    return strchr(path, '/') != 0 || strchr(path, '\\') != 0;
}

static int lm_finalize_is_absolute_path(struct LmMessageThread *lm_lmx_message_thread, char *path) {
    (void)lm_lmx_message_thread;
    if (path == 0 || path[0] == '\0') {
        return 0;
    }
    if (lm_finalize_is_path_separator(lm_lmx_message_thread, path[0])) {
        return 1;
    }
    if (lm_finalize_platform_is_drive_absolute(lm_lmx_message_thread, path)) {
        return 1;
    }
    return 0;
}

static int lm_finalize_file_exists(struct LmMessageThread *lm_lmx_message_thread, char *path) {
    (void)lm_lmx_message_thread;
    FILE * file;
    file = fopen(path, "rb");
    if (file == 0) {
        return 0;
    }
    fclose(file);
    return 1;
}

static int lm_finalize_join_path(struct LmMessageThread *lm_lmx_message_thread, char *buffer, size_t size, char *base, char *tail) {
    (void)lm_lmx_message_thread;
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
    if (base_length > 0U && tail_length > 0U && lm_finalize_is_path_separator(lm_lmx_message_thread, base[base_length - 1U]) == 0 && lm_finalize_is_path_separator(lm_lmx_message_thread, tail[0]) == 0) {
        memcpy(buffer + used, lm_finalize_platform_path_sep(lm_lmx_message_thread), strlen(lm_finalize_platform_path_sep(lm_lmx_message_thread)));
        used = used + strlen(lm_finalize_platform_path_sep(lm_lmx_message_thread));
    }
    memcpy(buffer + used, tail, tail_length + 1U);
    return 0;
}

static int lm_finalize_has_project_marker(struct LmMessageThread *lm_lmx_message_thread, char *path) {
    (void)lm_lmx_message_thread;
    char marker_path[2048];
    if (lm_finalize_join_path(lm_lmx_message_thread, marker_path, sizeof(marker_path), path, "lm2/buildCore.lmx") != 0) {
        return 0;
    }
    if (lm_finalize_file_exists(lm_lmx_message_thread, marker_path)) {
        return 1;
    }
    return 0;
}

static int lm_finalize_trim_last_path_part(struct LmMessageThread *lm_lmx_message_thread, char *path) {
    (void)lm_lmx_message_thread;
    size_t length;
    length = strlen(path);
    while (length > 0U && lm_finalize_is_path_separator(lm_lmx_message_thread, path[length - 1U])) {
        path[length - 1U] = '\0';
        length = length - 1U;
    }
    while (length > 0U) {
        length = length - 1U;
        if (lm_finalize_is_path_separator(lm_lmx_message_thread, path[length])) {
            path[length] = '\0';
            return 0;
        }
    }
    path[0] = '\0';
    return 0;
}

static int lm_finalize_enter_project_root(struct LmMessageThread *lm_lmx_message_thread, char *program_path) {
    (void)lm_lmx_message_thread;
    char search_path[1024];
    char executable_path[1024];
    char cwd[1024];
    int depth;
    if (lm_finalize_platform_getcwd(lm_lmx_message_thread, cwd, sizeof(cwd)) == 0) {
        fprintf(stderr, "finalize.lm0: cannot read current directory\n");
        return 1;
    }
    if (program_path != 0 && program_path[0] != '\0' && lm_finalize_has_path_separator(lm_lmx_message_thread, program_path)) {
        if (lm_finalize_is_absolute_path(lm_lmx_message_thread, program_path)) {
            if (strlen(program_path) >= sizeof(executable_path)) {
                fprintf(stderr, "finalize.lm0: executable path is too long\n");
                return 1;
            }
            strcpy(executable_path, program_path);
        }
        if (lm_finalize_is_absolute_path(lm_lmx_message_thread, program_path) == 0) {
            if (lm_finalize_join_path(lm_lmx_message_thread, executable_path, sizeof(executable_path), cwd, program_path) != 0) {
                return 1;
            }
        }
        strcpy(search_path, executable_path);
        lm_finalize_trim_last_path_part(lm_lmx_message_thread, search_path);
    }
    if (program_path == 0 || program_path[0] == '\0' || lm_finalize_has_path_separator(lm_lmx_message_thread, program_path) == 0) {
        strcpy(search_path, cwd);
    }
    depth = 0;
    while (depth < 12 && search_path[0] != '\0') {
        if (lm_finalize_has_project_marker(lm_lmx_message_thread, search_path)) {
            if (lm_finalize_platform_chdir(lm_lmx_message_thread, search_path) != 0) {
                fprintf(stderr, "finalize.lm0: cannot enter project root %s\n", search_path);
                return 1;
            }
            return 0;
        }
        lm_finalize_trim_last_path_part(lm_lmx_message_thread, search_path);
        depth = depth + 1;
    }
    fprintf(stderr, "finalize.lm0: cannot locate project root from %s\n", cwd);
    return 1;
    return 0;
}

static void lm_finalize_log(struct LmMessageThread *lm_lmx_message_thread, char *message) {
    (void)lm_lmx_message_thread;
    FILE * log_file;
    log_file = fopen("build/lm0/finalize.log", "a");
    if (log_file == 0) {
        return;
    }
    fprintf(log_file, "%s\n", message);
    fclose(log_file);
}

static void lm_finalize_sleep_retry(struct LmMessageThread *lm_lmx_message_thread) {
    (void)lm_lmx_message_thread;
    lm_finalize_platform_sleep_retry(lm_lmx_message_thread);
}

static void lm_finalize_live_tool_path(struct LmMessageThread *lm_lmx_message_thread, char *path, size_t size, char *tool_name) {
    (void)lm_lmx_message_thread;
    snprintf(path, size, "build%slm0%s%s.lm0%s", lm_finalize_platform_path_sep(lm_lmx_message_thread), lm_finalize_platform_path_sep(lm_lmx_message_thread), tool_name, lm_finalize_platform_exe_suffix(lm_lmx_message_thread));
}

static void lm_finalize_next_tool_path(struct LmMessageThread *lm_lmx_message_thread, char *path, size_t size, char *tool_name) {
    (void)lm_lmx_message_thread;
    snprintf(path, size, "build%slm0%snext%s%s.lm0%s", lm_finalize_platform_path_sep(lm_lmx_message_thread), lm_finalize_platform_path_sep(lm_lmx_message_thread), lm_finalize_platform_path_sep(lm_lmx_message_thread), tool_name, lm_finalize_platform_exe_suffix(lm_lmx_message_thread));
}

static void lm_finalize_legacy_next_tool_path(struct LmMessageThread *lm_lmx_message_thread, char *path, size_t size, char *tool_name) {
    (void)lm_lmx_message_thread;
    snprintf(path, size, "build%slm0%s%s.next.lm0%s", lm_finalize_platform_path_sep(lm_lmx_message_thread), lm_finalize_platform_path_sep(lm_lmx_message_thread), tool_name, lm_finalize_platform_exe_suffix(lm_lmx_message_thread));
}

static void lm_finalize_live_artifact_path(struct LmMessageThread *lm_lmx_message_thread, char *path, size_t size, char *artifact_name) {
    (void)lm_lmx_message_thread;
    snprintf(path, size, "build%slm0%s%s", lm_finalize_platform_path_sep(lm_lmx_message_thread), lm_finalize_platform_path_sep(lm_lmx_message_thread), artifact_name);
}

static void lm_finalize_next_artifact_path(struct LmMessageThread *lm_lmx_message_thread, char *path, size_t size, char *artifact_name) {
    (void)lm_lmx_message_thread;
    snprintf(path, size, "build%slm0%snext%s%s", lm_finalize_platform_path_sep(lm_lmx_message_thread), lm_finalize_platform_path_sep(lm_lmx_message_thread), lm_finalize_platform_path_sep(lm_lmx_message_thread), artifact_name);
}

static int lm_finalize_copy_once(struct LmMessageThread *lm_lmx_message_thread, char *source_path, char *output_path, int quiet) {
    (void)lm_lmx_message_thread;
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

static int lm_finalize_copy_with_retry(struct LmMessageThread *lm_lmx_message_thread, char *source_path, char *output_path) {
    (void)lm_lmx_message_thread;
    int attempt = 0;
    while (attempt < 40) {
        if (lm_finalize_copy_once(lm_lmx_message_thread, source_path, output_path, 1) == 0) {
            printf("finalize.lm0: copied %s to %s\n", source_path, output_path);
            return 0;
        }
        lm_finalize_sleep_retry(lm_lmx_message_thread);
        attempt = attempt + 1;
    }
    if (lm_finalize_copy_once(lm_lmx_message_thread, source_path, output_path, 0) != 0) {
        return 1;
    }
    printf("finalize.lm0: copied %s to %s\n", source_path, output_path);
    return 0;
}

static int lm_finalize_install_next_tool(struct LmMessageThread *lm_lmx_message_thread, char *tool_name) {
    (void)lm_lmx_message_thread;
    char source_path[256];
    char output_path[256];
    lm_finalize_next_tool_path(lm_lmx_message_thread, source_path, sizeof(source_path), tool_name);
    lm_finalize_live_tool_path(lm_lmx_message_thread, output_path, sizeof(output_path), tool_name);
    if (lm_finalize_copy_with_retry(lm_lmx_message_thread, source_path, output_path) != 0) {
        lm_finalize_log(lm_lmx_message_thread, "copy failed");
        return 1;
    }
    return 0;
}

static int lm_finalize_install_legacy_next_tool(struct LmMessageThread *lm_lmx_message_thread, char *tool_name) {
    (void)lm_lmx_message_thread;
    char source_path[256];
    char output_path[256];
    lm_finalize_legacy_next_tool_path(lm_lmx_message_thread, source_path, sizeof(source_path), tool_name);
    lm_finalize_live_tool_path(lm_lmx_message_thread, output_path, sizeof(output_path), tool_name);
    if (lm_finalize_copy_with_retry(lm_lmx_message_thread, source_path, output_path) != 0) {
        lm_finalize_log(lm_lmx_message_thread, "copy failed");
        return 1;
    }
    if (remove(source_path) != 0) {
        fprintf(stderr, "finalize.lm0: cannot remove temporary file %s\n", source_path);
        lm_finalize_log(lm_lmx_message_thread, "remove failed");
        return 1;
    }
    return 0;
}

static int lm_finalize_install_next_artifact(struct LmMessageThread *lm_lmx_message_thread, char *artifact_name) {
    (void)lm_lmx_message_thread;
    char source_path[256];
    char output_path[256];
    lm_finalize_next_artifact_path(lm_lmx_message_thread, source_path, sizeof(source_path), artifact_name);
    lm_finalize_live_artifact_path(lm_lmx_message_thread, output_path, sizeof(output_path), artifact_name);
    if (lm_finalize_copy_with_retry(lm_lmx_message_thread, source_path, output_path) != 0) {
        lm_finalize_log(lm_lmx_message_thread, "copy failed");
        return 1;
    }
    return 0;
}

static int lm_finalize_defer(struct LmMessageThread *lm_lmx_message_thread) {
    (void)lm_lmx_message_thread;
    char root_path[1024];
    char finalize_path[1400];
    char command[4096];
    int status;
    if (lm_finalize_platform_getcwd(lm_lmx_message_thread, root_path, sizeof(root_path)) == 0) {
        fprintf(stderr, "finalize.lm0: cannot get current directory\n");
        return 1;
    }
    snprintf(finalize_path, sizeof(finalize_path), "build%slm0%snext%sfinalize.lm0%s", lm_finalize_platform_path_sep(lm_lmx_message_thread), lm_finalize_platform_path_sep(lm_lmx_message_thread), lm_finalize_platform_path_sep(lm_lmx_message_thread), lm_finalize_platform_exe_suffix(lm_lmx_message_thread));
    if (lm_finalize_file_exists(lm_lmx_message_thread, finalize_path) == 0) {
        lm_finalize_live_tool_path(lm_lmx_message_thread, finalize_path, sizeof(finalize_path), "finalize");
    }
    snprintf(command, sizeof(command), lm_finalize_platform_defer_command_format(lm_lmx_message_thread), root_path, finalize_path);
    status = system(command);
    if (status != 0) {
        fprintf(stderr, "finalize.lm0: cannot schedule deferred finalize step\n");
        lm_finalize_log(lm_lmx_message_thread, "defer failed");
        return 1;
    }
    lm_finalize_log(lm_lmx_message_thread, "defer scheduled");
    printf("finalize.lm0: scheduled deferred staged install\n");
    return 0;
}

int main(int argc, char **argv) {
    struct LmMessageThread *lm_lmx_message_thread;
    LmMessageThreadExecutionContext lm_message_thread_main_context = {0};
    int lm_message_thread_exit_status;
    struct LmMessageThreadRuntime *lm_lmx_application_runtime = 0;
    int lm_lmx_application_root_attached = 0;
    int lm_lmx_thread_startup_failed = 0;
    int lm_lmx_thread_cleanup_failed = 0;
    int lm_lmx_application_controller_failure = 0;
    int lm_lmx_application_exit_requested = 0;
    int lm_lmx_application_exit_ready = 0;
    int lm_lmx_application_exit_status = 0;
    int lm_lmx_application_exit_snapshot_requested = 0;
    int lm_lmx_application_exit_snapshot_ready = 0;
    int lm_lmx_application_exit_snapshot_status = 0;
    lm_lmx_message_thread = lm_message_thread_new();
    if (lm_lmx_message_thread == 0) {
        return 1;
    }
    (void)lm_message_thread_set_execution_context(lm_lmx_message_thread, &lm_message_thread_main_context);
    lm_lmx_application_runtime = lm_message_thread_runtime_new();
    if (lm_lmx_application_runtime == 0) lm_lmx_thread_startup_failed = 1;
    if (!lm_lmx_thread_startup_failed && lm_message_thread_runtime_attach_root(lm_lmx_application_runtime, lm_lmx_message_thread) != 0) lm_lmx_thread_startup_failed = 1; else if (!lm_lmx_thread_startup_failed) lm_lmx_application_root_attached = 1;
    if (lm_lmx_thread_startup_failed) lm_message_thread_request_failure(lm_lmx_message_thread, 1);
    while (lm_message_thread_begin_turn(lm_lmx_message_thread)) {
        if (lm_lmx_application_controller_failure) {
            lm_message_thread_request_failure(lm_lmx_message_thread, 1);
            goto lm_message_thread_turn_end;
        }
        if (lm_lmx_application_exit_ready) {
            lm_message_thread_request_stop(lm_lmx_message_thread, lm_lmx_application_exit_status);
            goto lm_message_thread_turn_end;
        }
        lm_message_thread_main_context.diagnostic_code = 0;
        if (setjmp(lm_message_thread_main_context.diagnostic_root) == 0) {
            char staged_build_core_path[256];
            if (lm_finalize_enter_project_root(lm_lmx_message_thread, argv[0]) != 0) {
                {
                    int lm_return_0 = 1;
                    lm_message_thread_request_stop(lm_lmx_message_thread, lm_return_0);
                    goto lm_message_thread_turn_end;
                }
            }
            if (argc == 2 && strcmp(argv[1], "--defer") == 0) {
                {
                    int lm_return_1 = lm_finalize_defer(lm_lmx_message_thread);
                    lm_message_thread_request_stop(lm_lmx_message_thread, lm_return_1);
                    goto lm_message_thread_turn_end;
                }
            }
            if (argc != 1 && (argc != 2 || strcmp(argv[1], "--copy") != 0)) {
                fprintf(stderr, "usage: finalize.lm0[.exe] [--defer|--copy]\n");
                {
                    int lm_return_2 = 1;
                    lm_message_thread_request_stop(lm_lmx_message_thread, lm_return_2);
                    goto lm_message_thread_turn_end;
                }
            }
            lm_finalize_log(lm_lmx_message_thread, "copy started");
            lm_finalize_next_tool_path(lm_lmx_message_thread, staged_build_core_path, sizeof(staged_build_core_path), "buildCore");
            if (lm_finalize_file_exists(lm_lmx_message_thread, staged_build_core_path) == 0) {
                if (lm_finalize_install_legacy_next_tool(lm_lmx_message_thread, "make") != 0) {
                    {
                        int lm_return_3 = 1;
                        lm_message_thread_request_stop(lm_lmx_message_thread, lm_return_3);
                        goto lm_message_thread_turn_end;
                    }
                }
                if (lm_finalize_install_legacy_next_tool(lm_lmx_message_thread, "buildCore") != 0) {
                    {
                        int lm_return_4 = 1;
                        lm_message_thread_request_stop(lm_lmx_message_thread, lm_return_4);
                        goto lm_message_thread_turn_end;
                    }
                }
                lm_finalize_log(lm_lmx_message_thread, "legacy copy completed");
                {
                    int lm_return_5 = 0;
                    lm_message_thread_request_stop(lm_lmx_message_thread, lm_return_5);
                    goto lm_message_thread_turn_end;
                }
            }
            if (lm_finalize_install_next_artifact(lm_lmx_message_thread, "libparser.lm0.a") != 0) {
                {
                    int lm_return_6 = 1;
                    lm_message_thread_request_stop(lm_lmx_message_thread, lm_return_6);
                    goto lm_message_thread_turn_end;
                }
            }
            if (lm_finalize_install_next_artifact(lm_lmx_message_thread, "libown.lm0.a") != 0) {
                {
                    int lm_return_7 = 1;
                    lm_message_thread_request_stop(lm_lmx_message_thread, lm_return_7);
                    goto lm_message_thread_turn_end;
                }
            }
            if (lm_finalize_install_next_tool(lm_lmx_message_thread, "make") != 0) {
                {
                    int lm_return_8 = 1;
                    lm_message_thread_request_stop(lm_lmx_message_thread, lm_return_8);
                    goto lm_message_thread_turn_end;
                }
            }
            if (lm_finalize_install_next_tool(lm_lmx_message_thread, "trans") != 0) {
                {
                    int lm_return_9 = 1;
                    lm_message_thread_request_stop(lm_lmx_message_thread, lm_return_9);
                    goto lm_message_thread_turn_end;
                }
            }
            if (lm_finalize_install_next_tool(lm_lmx_message_thread, "vcpkgFetch") != 0) {
                {
                    int lm_return_10 = 1;
                    lm_message_thread_request_stop(lm_lmx_message_thread, lm_return_10);
                    goto lm_message_thread_turn_end;
                }
            }
            if (lm_finalize_install_next_tool(lm_lmx_message_thread, "printTree") != 0) {
                {
                    int lm_return_11 = 1;
                    lm_message_thread_request_stop(lm_lmx_message_thread, lm_return_11);
                    goto lm_message_thread_turn_end;
                }
            }
            if (lm_finalize_install_next_tool(lm_lmx_message_thread, "finalize") != 0) {
                {
                    int lm_return_12 = 1;
                    lm_message_thread_request_stop(lm_lmx_message_thread, lm_return_12);
                    goto lm_message_thread_turn_end;
                }
            }
            if (lm_finalize_install_next_tool(lm_lmx_message_thread, "buildCore") != 0) {
                {
                    int lm_return_13 = 1;
                    lm_message_thread_request_stop(lm_lmx_message_thread, lm_return_13);
                    goto lm_message_thread_turn_end;
                }
            }
            lm_finalize_log(lm_lmx_message_thread, "copy completed");
            {
                int lm_return_14 = 0;
                lm_message_thread_request_stop(lm_lmx_message_thread, lm_return_14);
                goto lm_message_thread_turn_end;
            }
        } else {
            lm_message_thread_request_failure(lm_lmx_message_thread, lm_message_thread_diagnostic_status(&lm_message_thread_main_context));
        }
    lm_message_thread_turn_end:
        (void)lm_message_thread_end_turn(lm_lmx_message_thread);
        lm_lmx_application_exit_snapshot_requested = 0;
        lm_lmx_application_exit_snapshot_ready = 0;
        lm_lmx_application_exit_snapshot_status = 0;
        if (lm_lmx_application_runtime != 0) {
            if (lm_message_thread_runtime_exit_state(lm_lmx_application_runtime, &lm_lmx_application_exit_snapshot_requested, &lm_lmx_application_exit_snapshot_ready, &lm_lmx_application_exit_snapshot_status) != 0) {
                lm_lmx_thread_cleanup_failed = 1;
                lm_lmx_application_controller_failure = 1;
            } else {
                lm_lmx_application_exit_requested = lm_lmx_application_exit_snapshot_requested;
                lm_lmx_application_exit_ready = lm_lmx_application_exit_snapshot_ready;
                lm_lmx_application_exit_status = lm_lmx_application_exit_snapshot_status;
            }
        }
    }
    lm_lmx_application_exit_snapshot_requested = 0;
    lm_lmx_application_exit_snapshot_ready = 0;
    lm_lmx_application_exit_snapshot_status = 0;
    if (lm_lmx_application_runtime != 0) {
        if (lm_message_thread_runtime_exit_state(lm_lmx_application_runtime, &lm_lmx_application_exit_snapshot_requested, &lm_lmx_application_exit_snapshot_ready, &lm_lmx_application_exit_snapshot_status) != 0) {
            lm_lmx_thread_cleanup_failed = 1;
            lm_lmx_application_controller_failure = 1;
        } else {
            lm_lmx_application_exit_requested = lm_lmx_application_exit_snapshot_requested;
            lm_lmx_application_exit_ready = lm_lmx_application_exit_snapshot_ready;
            lm_lmx_application_exit_status = lm_lmx_application_exit_snapshot_status;
        }
    }
    lm_message_thread_exit_status = lm_message_thread_status(lm_lmx_message_thread);
    if (lm_lmx_application_runtime != 0 && lm_lmx_application_root_attached) {
        if (lm_message_thread_runtime_detach_root(lm_lmx_application_runtime, lm_lmx_message_thread) != 0) lm_lmx_thread_cleanup_failed = 1; else lm_lmx_application_root_attached = 0;
    }
    if (lm_lmx_application_runtime != 0 && !lm_lmx_application_root_attached) {
        if (lm_message_thread_runtime_delete(lm_lmx_application_runtime) != 0) lm_lmx_thread_cleanup_failed = 1; else lm_lmx_application_runtime = 0;
    }
    if (lm_message_thread_exit_status == 0 && lm_lmx_application_exit_requested && lm_lmx_application_exit_status != 0) lm_message_thread_exit_status = lm_lmx_application_exit_status;
    if (lm_message_thread_exit_status == 0 && lm_lmx_thread_cleanup_failed) lm_message_thread_exit_status = 1;
    if (!lm_lmx_application_root_attached) lm_message_thread_delete(lm_lmx_message_thread);
    return lm_message_thread_exit_status;
}
