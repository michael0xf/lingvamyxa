#include <stddef.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>

#include <string.h>
struct LmOwnArena;
struct LmMessageThread;
struct LmMessageThreadRuntime;
struct LmMessageThreadPool;
#ifndef lm_lmx_module_private_1_typedef_defined_LmSlice
#define lm_lmx_module_private_1_typedef_defined_LmSlice 1
#define lm_lmx_module_private_1_typedef_id_a_LmSlice 0xdd6442dffff43f92ULL
#define lm_lmx_module_private_1_typedef_id_b_LmSlice 0x21d9c62537977663ULL
typedef struct LmSlice {
    void *ptr;
    size_t length;
} LmSlice;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmSlice) || !defined(lm_lmx_module_private_1_typedef_id_b_LmSlice) || lm_lmx_module_private_1_typedef_id_a_LmSlice != 0xdd6442dffff43f92ULL || lm_lmx_module_private_1_typedef_id_b_LmSlice != 0x21d9c62537977663ULL
#error "Lingvamyxa conflicting typedef projection for LmSlice"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_IncomingMessage
#define lm_lmx_module_private_1_typedef_defined_IncomingMessage 1
#define lm_lmx_module_private_1_typedef_id_a_IncomingMessage 0x10eae12962f7f58fULL
#define lm_lmx_module_private_1_typedef_id_b_IncomingMessage 0xb2add056afdd41bcULL
typedef struct IncomingMessage IncomingMessage;
struct IncomingMessage {
    const char *lmx;
    size_t length;
};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_IncomingMessage) || !defined(lm_lmx_module_private_1_typedef_id_b_IncomingMessage) || lm_lmx_module_private_1_typedef_id_a_IncomingMessage != 0x10eae12962f7f58fULL || lm_lmx_module_private_1_typedef_id_b_IncomingMessage != 0xb2add056afdd41bcULL
#error "Lingvamyxa conflicting typedef projection for IncomingMessage"
#endif
#endif
struct LmMessageThread *lm_message_thread_new(void);
void lm_message_thread_delete(struct LmMessageThread *thread);
struct LmMessageThreadRuntime *lm_message_thread_runtime_new(void);
#ifdef LM_REST_LMX_INSTALL_DEFAULT_CLIENT
int lm_rest_lmx_http_client_install_default(struct LmMessageThreadRuntime *runtime);
#endif
#ifdef LM_REST_LMX_INSTALL_DEFAULT_SERVER
#ifndef lm_lmx_module_private_1_typedef_defined_LmRestLmxHttpServer
#define lm_lmx_module_private_1_typedef_defined_LmRestLmxHttpServer 1
#define lm_lmx_module_private_1_typedef_id_a_LmRestLmxHttpServer 0xc93320eec144d348ULL
#define lm_lmx_module_private_1_typedef_id_b_LmRestLmxHttpServer 0x25d8028910651f29ULL
typedef struct LmRestLmxHttpServer LmRestLmxHttpServer;
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmRestLmxHttpServer) || !defined(lm_lmx_module_private_1_typedef_id_b_LmRestLmxHttpServer) || lm_lmx_module_private_1_typedef_id_a_LmRestLmxHttpServer != 0xc93320eec144d348ULL || lm_lmx_module_private_1_typedef_id_b_LmRestLmxHttpServer != 0x25d8028910651f29ULL
#error "Lingvamyxa conflicting typedef projection for LmRestLmxHttpServer"
#endif
#endif
int lm_rest_lmx_http_server_start_default(struct LmMessageThreadRuntime *runtime, LmRestLmxHttpServer **out_server);
int lm_rest_lmx_http_server_stop(LmRestLmxHttpServer **server);
#endif
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
#ifndef lm_lmx_module_private_1_typedef_defined_LmMessageThreadExecutionContext
#define lm_lmx_module_private_1_typedef_defined_LmMessageThreadExecutionContext 1
#define lm_lmx_module_private_1_typedef_id_a_LmMessageThreadExecutionContext 0xa01e9b4b1dd2bdb5ULL
#define lm_lmx_module_private_1_typedef_id_b_LmMessageThreadExecutionContext 0x5b5c3f9ba71f9c50ULL
typedef struct LmMessageThreadExecutionContext LmMessageThreadExecutionContext;
struct LmMessageThreadExecutionContext {
    jmp_buf diagnostic_root;
    int diagnostic_code;
    const char *diagnostic_label;
    const char *diagnostic_file;
    int diagnostic_line;
    const char *diagnostic_expr;
};
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmMessageThreadExecutionContext) || !defined(lm_lmx_module_private_1_typedef_id_b_LmMessageThreadExecutionContext) || lm_lmx_module_private_1_typedef_id_a_LmMessageThreadExecutionContext != 0xa01e9b4b1dd2bdb5ULL || lm_lmx_module_private_1_typedef_id_b_LmMessageThreadExecutionContext != 0x5b5c3f9ba71f9c50ULL
#error "Lingvamyxa conflicting typedef projection for LmMessageThreadExecutionContext"
#endif
#endif
#ifndef lm_lmx_module_private_1_typedef_defined_LmLmxMessageThreadEntry
#define lm_lmx_module_private_1_typedef_defined_LmLmxMessageThreadEntry 1
#define lm_lmx_module_private_1_typedef_id_a_LmLmxMessageThreadEntry 0x645bb4e15cf3f304ULL
#define lm_lmx_module_private_1_typedef_id_b_LmLmxMessageThreadEntry 0xb3dc5cc6edc7d305ULL
typedef int (*LmLmxMessageThreadEntry)(struct LmMessageThread *thread);
#else
#if !defined(lm_lmx_module_private_1_typedef_id_a_LmLmxMessageThreadEntry) || !defined(lm_lmx_module_private_1_typedef_id_b_LmLmxMessageThreadEntry) || lm_lmx_module_private_1_typedef_id_a_LmLmxMessageThreadEntry != 0x645bb4e15cf3f304ULL || lm_lmx_module_private_1_typedef_id_b_LmLmxMessageThreadEntry != 0xb3dc5cc6edc7d305ULL
#error "Lingvamyxa conflicting typedef projection for LmLmxMessageThreadEntry"
#endif
#endif
#if defined(__GNUC__) || defined(__clang__)
#define LM_LMX_UNUSED_ENTRY_HELPER __attribute__((unused))
#else
#define LM_LMX_UNUSED_ENTRY_HELPER
#endif
static inline LM_LMX_UNUSED_ENTRY_HELPER int lm_lmx_message_thread_invoke_diagnostic(
    struct LmMessageThread *thread,
    void (*entry)(struct LmMessageThread *, void *),
    void *argument
) {
    LmMessageThreadExecutionContext *context;
    void *previous;
    int status = 0;
    if (thread == 0 || entry == 0) {
        return 1;
    }
    context = (LmMessageThreadExecutionContext *)calloc(1U, sizeof(*context));
    if (context == 0) {
        lm_message_thread_request_failure(thread, 1);
        return 1;
    }
    previous = lm_message_thread_set_execution_context(thread, context);
    if (setjmp(context->diagnostic_root) == 0) {
        entry(thread, argument);
    } else {
        status = context->diagnostic_code == 0 ? 1 : context->diagnostic_code;
        lm_message_thread_request_failure(thread, status);
    }
    (void)lm_message_thread_set_execution_context(thread, previous);
    free(context);
    return status;
}
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
#ifdef LM_REST_LMX_INSTALL_DEFAULT_CLIENT
    if (lm_rest_lmx_http_client_install_default(runtime) != 0) {
        (void)lm_message_thread_runtime_delete(runtime);
        lm_message_thread_delete(thread);
        return 1;
    }
#endif
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
static char * lm_make_auto_thread_provider(struct LmMessageThread *lm_lmx_message_thread);

static char * lm_make_auto_thread_provider(struct LmMessageThread *lm_lmx_message_thread) {
    (void)lm_lmx_message_thread;
    return "win32";
}
#else
static char * lm_make_auto_thread_provider(struct LmMessageThread *lm_lmx_message_thread);

static char * lm_make_auto_thread_provider(struct LmMessageThread *lm_lmx_message_thread) {
    (void)lm_lmx_message_thread;
    return "pthread";
}
#endif
static char * lm_make_env_or_default(struct LmMessageThread *lm_lmx_message_thread, char *name, char *fallback);
static char * lm_make_resolve_thread_provider(struct LmMessageThread *lm_lmx_message_thread, char *requested);
static char * lm_make_thread_provider_define(struct LmMessageThread *lm_lmx_message_thread, char *provider);
static size_t lm_make_append(struct LmMessageThread *lm_lmx_message_thread, char *buffer, size_t size, size_t used, char *text);
static size_t lm_make_append_arg(struct LmMessageThread *lm_lmx_message_thread, char *buffer, size_t size, size_t used, char *arg);
static int lm_make_run_command(struct LmMessageThread *lm_lmx_message_thread, char *command);
static int lm_make_run_tool(struct LmMessageThread *lm_lmx_message_thread, char *tool, int argc, char **argv, int start);
static int lm_make_run_c_tool(struct LmMessageThread *lm_lmx_message_thread, char *tool, char *provider, int argc, char **argv, int start);
static int lm_make_copy_file(struct LmMessageThread *lm_lmx_message_thread, char *source_path, char *output_path);
static void lm_make_print_usage(struct LmMessageThread *lm_lmx_message_thread);
int main(int argc, char **argv);

static int lm_lmx_module_private_1_init_all(struct LmMessageThread *lm_lmx_module_private_1_message_thread, struct LmOwnArena *lm_lmx_module_private_1_arena);
static inline int lm_message_thread_diagnostic_status(const LmMessageThreadExecutionContext *context) {
    if (context == 0 || context->diagnostic_code == 0) {
        return 1;
    }
    return context->diagnostic_code;
}
#if defined(__GNUC__) || defined(__clang__)
__attribute__((unused))
#endif
static int lm_lmx_module_private_1_init_all(struct LmMessageThread *lm_lmx_module_private_1_message_thread, struct LmOwnArena *lm_lmx_module_private_1_arena) {
    (void)lm_lmx_module_private_1_message_thread;
    (void)lm_lmx_module_private_1_arena;
    return 0;
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

static char * lm_make_resolve_thread_provider(struct LmMessageThread *lm_lmx_message_thread, char *requested) {
    (void)lm_lmx_message_thread;
    if (strcmp(requested, "auto") == 0) {
        return lm_make_auto_thread_provider(lm_lmx_message_thread);
    }
    if (strcmp(requested, "pthread") == 0) {
        return "pthread";
    }
    if (strcmp(requested, "win32") == 0) {
        return "win32";
    }
    if (strcmp(requested, "single") == 0) {
        return "single";
    }
    fprintf(stderr, "make.lm0: unsupported LM_THREAD_PROVIDER '%s' (expected auto, pthread, win32 or single)\n", requested);
    return 0;
}

static char * lm_make_thread_provider_define(struct LmMessageThread *lm_lmx_message_thread, char *provider) {
    (void)lm_lmx_message_thread;
    if (strcmp(provider, "pthread") == 0) {
        return "-DLM_THREAD_PROVIDER=LM_THREAD_PROVIDER_PTHREAD";
    }
    if (strcmp(provider, "win32") == 0) {
        return "-DLM_THREAD_PROVIDER=LM_THREAD_PROVIDER_WIN32";
    }
    return "-DLM_THREAD_PROVIDER=LM_THREAD_PROVIDER_SINGLE";
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

static int lm_make_run_c_tool(struct LmMessageThread *lm_lmx_message_thread, char *tool, char *provider, int argc, char **argv, int start) {
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
    used = lm_make_append_arg(lm_lmx_message_thread, command, sizeof(command), used, lm_make_thread_provider_define(lm_lmx_message_thread, provider));
    if (used == sizeof(command)) {
        return 1;
    }
    if (strcmp(provider, "pthread") == 0) {
        used = lm_make_append_arg(lm_lmx_message_thread, command, sizeof(command), used, "-pthread");
        if (used == sizeof(command)) {
            return 1;
        }
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
    struct LmMessageThread *lm_lmx_module_private_1_main_lm_lmx_message_thread;
    LmMessageThreadExecutionContext lm_lmx_module_private_1_main_lm_message_thread_main_context = {0};
    int lm_lmx_module_private_1_main_lm_message_thread_exit_status;
    struct LmMessageThreadRuntime *lm_lmx_module_private_1_main_lm_lmx_application_runtime = 0;
    int lm_lmx_module_private_1_main_lm_lmx_application_root_attached = 0;
    int lm_lmx_module_private_1_main_lm_lmx_thread_startup_failed = 0;
    int lm_lmx_module_private_1_main_lm_lmx_thread_cleanup_failed = 0;
    int lm_lmx_module_private_1_main_lm_lmx_application_controller_failure = 0;
    int lm_lmx_module_private_1_main_lm_lmx_application_exit_requested = 0;
    int lm_lmx_module_private_1_main_lm_lmx_application_exit_ready = 0;
    int lm_lmx_module_private_1_main_lm_lmx_application_exit_status = 0;
    int lm_lmx_module_private_1_main_lm_lmx_application_exit_snapshot_requested = 0;
    int lm_lmx_module_private_1_main_lm_lmx_application_exit_snapshot_ready = 0;
    int lm_lmx_module_private_1_main_lm_lmx_application_exit_snapshot_status = 0;
    lm_lmx_module_private_1_main_lm_lmx_message_thread = lm_message_thread_new();
    if (lm_lmx_module_private_1_main_lm_lmx_message_thread == 0) {
        return 1;
    }
    lm_lmx_message_thread = lm_lmx_module_private_1_main_lm_lmx_message_thread;
    (void)lm_lmx_message_thread;
    (void)lm_message_thread_set_execution_context(lm_lmx_module_private_1_main_lm_lmx_message_thread, &lm_lmx_module_private_1_main_lm_message_thread_main_context);
    lm_lmx_module_private_1_main_lm_lmx_application_runtime = lm_message_thread_runtime_new();
    if (lm_lmx_module_private_1_main_lm_lmx_application_runtime == 0) lm_lmx_module_private_1_main_lm_lmx_thread_startup_failed = 1;
#ifdef LM_REST_LMX_INSTALL_DEFAULT_CLIENT
    if (!lm_lmx_module_private_1_main_lm_lmx_thread_startup_failed && lm_rest_lmx_http_client_install_default(lm_lmx_module_private_1_main_lm_lmx_application_runtime) != 0) {
        (void)lm_message_thread_runtime_delete(lm_lmx_module_private_1_main_lm_lmx_application_runtime);
        lm_lmx_module_private_1_main_lm_lmx_application_runtime = 0;
        lm_message_thread_delete(lm_lmx_module_private_1_main_lm_lmx_message_thread);
        return 1;
    }
#endif
    if (!lm_lmx_module_private_1_main_lm_lmx_thread_startup_failed && lm_message_thread_runtime_attach_root(lm_lmx_module_private_1_main_lm_lmx_application_runtime, lm_lmx_module_private_1_main_lm_lmx_message_thread) != 0) lm_lmx_module_private_1_main_lm_lmx_thread_startup_failed = 1; else if (!lm_lmx_module_private_1_main_lm_lmx_thread_startup_failed) lm_lmx_module_private_1_main_lm_lmx_application_root_attached = 1;
    if (!lm_lmx_module_private_1_main_lm_lmx_thread_startup_failed) {
        lm_lmx_module_private_1_main_lm_message_thread_main_context.diagnostic_code = 0;
        if (setjmp(lm_lmx_module_private_1_main_lm_message_thread_main_context.diagnostic_root) == 0) {
            if (lm_lmx_module_private_1_init_all(lm_lmx_module_private_1_main_lm_lmx_message_thread, lm_message_thread_owner(lm_lmx_module_private_1_main_lm_lmx_message_thread)) != 0) lm_lmx_module_private_1_main_lm_lmx_thread_startup_failed = 1;
        } else {
            lm_lmx_module_private_1_main_lm_lmx_thread_startup_failed = 1;
        }
    }
    if (lm_lmx_module_private_1_main_lm_lmx_thread_startup_failed) lm_message_thread_request_failure(lm_lmx_module_private_1_main_lm_lmx_message_thread, 1);
    while (lm_message_thread_begin_turn(lm_lmx_module_private_1_main_lm_lmx_message_thread)) {
        if (lm_lmx_module_private_1_main_lm_lmx_application_controller_failure) {
            lm_message_thread_request_failure(lm_lmx_module_private_1_main_lm_lmx_message_thread, 1);
            goto lm_lmx_module_private_1_main_lm_message_thread_turn_end;
        }
        if (lm_lmx_module_private_1_main_lm_lmx_application_exit_ready) {
            lm_message_thread_request_stop(lm_lmx_module_private_1_main_lm_lmx_message_thread, lm_lmx_module_private_1_main_lm_lmx_application_exit_status);
            goto lm_lmx_module_private_1_main_lm_message_thread_turn_end;
        }
        lm_lmx_module_private_1_main_lm_message_thread_main_context.diagnostic_code = 0;
        if (setjmp(lm_lmx_module_private_1_main_lm_message_thread_main_context.diagnostic_root) == 0) {
            char *cmake;
            char *cc;
            char *ar;
            char *ranlib;
            char *thread_provider;
            if (argc < 2) {
                lm_make_print_usage(lm_lmx_module_private_1_main_lm_lmx_message_thread);
                {
                    int lm_return_0 = 1;
                    lm_message_thread_request_stop(lm_lmx_module_private_1_main_lm_lmx_message_thread, lm_return_0);
                    goto lm_lmx_module_private_1_main_lm_message_thread_turn_end;
                }
            }
            cmake = lm_make_env_or_default(lm_lmx_module_private_1_main_lm_lmx_message_thread, "LM_CMAKE", "cmake");
            cc = lm_make_env_or_default(lm_lmx_module_private_1_main_lm_lmx_message_thread, "LM_CC", "gcc");
            ar = lm_make_env_or_default(lm_lmx_module_private_1_main_lm_lmx_message_thread, "LM_AR", "ar");
            ranlib = lm_make_env_or_default(lm_lmx_module_private_1_main_lm_lmx_message_thread, "LM_RANLIB", "ranlib");
            thread_provider = lm_make_resolve_thread_provider(lm_lmx_module_private_1_main_lm_lmx_message_thread, lm_make_env_or_default(lm_lmx_module_private_1_main_lm_lmx_message_thread, "LM_THREAD_PROVIDER", "single"));
            if (thread_provider == 0) {
                {
                    int lm_return_1 = 1;
                    lm_message_thread_request_stop(lm_lmx_module_private_1_main_lm_lmx_message_thread, lm_return_1);
                    goto lm_lmx_module_private_1_main_lm_message_thread_turn_end;
                }
            }
            if (strcmp(argv[1], "mkdir") == 0) {
                char command[8192];
                size_t used = 0U;
                int index = 2;
                if (argc < 3) {
                    lm_make_print_usage(lm_lmx_module_private_1_main_lm_lmx_message_thread);
                    {
                        int lm_return_2 = 1;
                        lm_message_thread_request_stop(lm_lmx_module_private_1_main_lm_lmx_message_thread, lm_return_2);
                        goto lm_lmx_module_private_1_main_lm_message_thread_turn_end;
                    }
                }
                command[0] = '\0';
                used = lm_make_append(lm_lmx_module_private_1_main_lm_lmx_message_thread, command, sizeof(command), used, cmake);
                if (used == sizeof(command)) {
                    {
                        int lm_return_3 = 1;
                        lm_message_thread_request_stop(lm_lmx_module_private_1_main_lm_lmx_message_thread, lm_return_3);
                        goto lm_lmx_module_private_1_main_lm_message_thread_turn_end;
                    }
                }
                used = lm_make_append(lm_lmx_module_private_1_main_lm_lmx_message_thread, command, sizeof(command), used, " -E make_directory");
                if (used == sizeof(command)) {
                    {
                        int lm_return_4 = 1;
                        lm_message_thread_request_stop(lm_lmx_module_private_1_main_lm_lmx_message_thread, lm_return_4);
                        goto lm_lmx_module_private_1_main_lm_message_thread_turn_end;
                    }
                }
                while (index < argc) {
                    used = lm_make_append_arg(lm_lmx_module_private_1_main_lm_lmx_message_thread, command, sizeof(command), used, argv[index]);
                    if (used == sizeof(command)) {
                        {
                            int lm_return_5 = 1;
                            lm_message_thread_request_stop(lm_lmx_module_private_1_main_lm_lmx_message_thread, lm_return_5);
                            goto lm_lmx_module_private_1_main_lm_message_thread_turn_end;
                        }
                    }
                    index = index + 1;
                }
                {
                    int lm_return_6 = lm_make_run_command(lm_lmx_module_private_1_main_lm_lmx_message_thread, command);
                    lm_message_thread_request_stop(lm_lmx_module_private_1_main_lm_lmx_message_thread, lm_return_6);
                    goto lm_lmx_module_private_1_main_lm_message_thread_turn_end;
                }
            }
            if (strcmp(argv[1], "cc") == 0 || strcmp(argv[1], "link") == 0) {
                {
                    int lm_return_7 = lm_make_run_c_tool(lm_lmx_module_private_1_main_lm_lmx_message_thread, cc, thread_provider, argc, argv, 2);
                    lm_message_thread_request_stop(lm_lmx_module_private_1_main_lm_lmx_message_thread, lm_return_7);
                    goto lm_lmx_module_private_1_main_lm_message_thread_turn_end;
                }
            }
            if (strcmp(argv[1], "ar") == 0) {
                {
                    int lm_return_8 = lm_make_run_tool(lm_lmx_module_private_1_main_lm_lmx_message_thread, ar, argc, argv, 2);
                    lm_message_thread_request_stop(lm_lmx_module_private_1_main_lm_lmx_message_thread, lm_return_8);
                    goto lm_lmx_module_private_1_main_lm_message_thread_turn_end;
                }
            }
            if (strcmp(argv[1], "ranlib") == 0) {
                {
                    int lm_return_9 = lm_make_run_tool(lm_lmx_module_private_1_main_lm_lmx_message_thread, ranlib, argc, argv, 2);
                    lm_message_thread_request_stop(lm_lmx_module_private_1_main_lm_lmx_message_thread, lm_return_9);
                    goto lm_lmx_module_private_1_main_lm_message_thread_turn_end;
                }
            }
            if (strcmp(argv[1], "copy") == 0) {
                if (argc != 4) {
                    lm_make_print_usage(lm_lmx_module_private_1_main_lm_lmx_message_thread);
                    {
                        int lm_return_10 = 1;
                        lm_message_thread_request_stop(lm_lmx_module_private_1_main_lm_lmx_message_thread, lm_return_10);
                        goto lm_lmx_module_private_1_main_lm_message_thread_turn_end;
                    }
                }
                {
                    int lm_return_11 = lm_make_copy_file(lm_lmx_module_private_1_main_lm_lmx_message_thread, argv[2], argv[3]);
                    lm_message_thread_request_stop(lm_lmx_module_private_1_main_lm_lmx_message_thread, lm_return_11);
                    goto lm_lmx_module_private_1_main_lm_message_thread_turn_end;
                }
            }
            lm_make_print_usage(lm_lmx_module_private_1_main_lm_lmx_message_thread);
            {
                int lm_return_12 = 1;
                lm_message_thread_request_stop(lm_lmx_module_private_1_main_lm_lmx_message_thread, lm_return_12);
                goto lm_lmx_module_private_1_main_lm_message_thread_turn_end;
            }
        } else {
            lm_message_thread_request_failure(lm_lmx_module_private_1_main_lm_lmx_message_thread, lm_message_thread_diagnostic_status(&lm_lmx_module_private_1_main_lm_message_thread_main_context));
        }
    lm_lmx_module_private_1_main_lm_message_thread_turn_end:
        (void)lm_message_thread_end_turn(lm_lmx_module_private_1_main_lm_lmx_message_thread);
        lm_lmx_module_private_1_main_lm_lmx_application_exit_snapshot_requested = 0;
        lm_lmx_module_private_1_main_lm_lmx_application_exit_snapshot_ready = 0;
        lm_lmx_module_private_1_main_lm_lmx_application_exit_snapshot_status = 0;
        if (lm_lmx_module_private_1_main_lm_lmx_application_runtime != 0) {
            if (lm_message_thread_runtime_exit_state(lm_lmx_module_private_1_main_lm_lmx_application_runtime, &lm_lmx_module_private_1_main_lm_lmx_application_exit_snapshot_requested, &lm_lmx_module_private_1_main_lm_lmx_application_exit_snapshot_ready, &lm_lmx_module_private_1_main_lm_lmx_application_exit_snapshot_status) != 0) {
                lm_lmx_module_private_1_main_lm_lmx_thread_cleanup_failed = 1;
                lm_lmx_module_private_1_main_lm_lmx_application_controller_failure = 1;
            } else {
                lm_lmx_module_private_1_main_lm_lmx_application_exit_requested = lm_lmx_module_private_1_main_lm_lmx_application_exit_snapshot_requested;
                lm_lmx_module_private_1_main_lm_lmx_application_exit_ready = lm_lmx_module_private_1_main_lm_lmx_application_exit_snapshot_ready;
                lm_lmx_module_private_1_main_lm_lmx_application_exit_status = lm_lmx_module_private_1_main_lm_lmx_application_exit_snapshot_status;
            }
        }
    }
    lm_lmx_module_private_1_main_lm_lmx_application_exit_snapshot_requested = 0;
    lm_lmx_module_private_1_main_lm_lmx_application_exit_snapshot_ready = 0;
    lm_lmx_module_private_1_main_lm_lmx_application_exit_snapshot_status = 0;
    if (lm_lmx_module_private_1_main_lm_lmx_application_runtime != 0) {
        if (lm_message_thread_runtime_exit_state(lm_lmx_module_private_1_main_lm_lmx_application_runtime, &lm_lmx_module_private_1_main_lm_lmx_application_exit_snapshot_requested, &lm_lmx_module_private_1_main_lm_lmx_application_exit_snapshot_ready, &lm_lmx_module_private_1_main_lm_lmx_application_exit_snapshot_status) != 0) {
            lm_lmx_module_private_1_main_lm_lmx_thread_cleanup_failed = 1;
            lm_lmx_module_private_1_main_lm_lmx_application_controller_failure = 1;
        } else {
            lm_lmx_module_private_1_main_lm_lmx_application_exit_requested = lm_lmx_module_private_1_main_lm_lmx_application_exit_snapshot_requested;
            lm_lmx_module_private_1_main_lm_lmx_application_exit_ready = lm_lmx_module_private_1_main_lm_lmx_application_exit_snapshot_ready;
            lm_lmx_module_private_1_main_lm_lmx_application_exit_status = lm_lmx_module_private_1_main_lm_lmx_application_exit_snapshot_status;
        }
    }
    lm_lmx_module_private_1_main_lm_message_thread_exit_status = lm_message_thread_status(lm_lmx_module_private_1_main_lm_lmx_message_thread);
    if (lm_lmx_module_private_1_main_lm_lmx_application_runtime != 0 && lm_lmx_module_private_1_main_lm_lmx_application_root_attached) {
        if (lm_message_thread_runtime_detach_root(lm_lmx_module_private_1_main_lm_lmx_application_runtime, lm_lmx_module_private_1_main_lm_lmx_message_thread) != 0) lm_lmx_module_private_1_main_lm_lmx_thread_cleanup_failed = 1; else lm_lmx_module_private_1_main_lm_lmx_application_root_attached = 0;
    }
    if (lm_lmx_module_private_1_main_lm_lmx_application_runtime != 0 && !lm_lmx_module_private_1_main_lm_lmx_application_root_attached) {
        if (lm_message_thread_runtime_delete(lm_lmx_module_private_1_main_lm_lmx_application_runtime) != 0) lm_lmx_module_private_1_main_lm_lmx_thread_cleanup_failed = 1; else lm_lmx_module_private_1_main_lm_lmx_application_runtime = 0;
    }
    if (lm_lmx_module_private_1_main_lm_message_thread_exit_status == 0 && lm_lmx_module_private_1_main_lm_lmx_application_exit_requested && lm_lmx_module_private_1_main_lm_lmx_application_exit_status != 0) lm_lmx_module_private_1_main_lm_message_thread_exit_status = lm_lmx_module_private_1_main_lm_lmx_application_exit_status;
    if (lm_lmx_module_private_1_main_lm_message_thread_exit_status == 0 && lm_lmx_module_private_1_main_lm_lmx_thread_cleanup_failed) lm_lmx_module_private_1_main_lm_message_thread_exit_status = 1;
    if (!lm_lmx_module_private_1_main_lm_lmx_application_root_attached) lm_message_thread_delete(lm_lmx_module_private_1_main_lm_lmx_message_thread);
    return lm_lmx_module_private_1_main_lm_message_thread_exit_status;
}
