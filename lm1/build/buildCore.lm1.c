#include <stddef.h>
typedef struct LmOwnPtrStack LmOwnPtrStack;
typedef struct LmOwnValueStack LmOwnValueStack;
typedef struct LmOwnAllocationDescriptor LmOwnAllocationDescriptor;
typedef struct LmOwnLazyEdge LmOwnLazyEdge;
typedef struct LmOwnArena LmOwnArena;

typedef int LmOwnEdgeKind;

#define LM_OWN_EDGE_BORROWED 1
#define LM_OWN_EDGE_OWNED 2
#define LM_OWN_EDGE_LAZY_OWNED 3
#define LM_OWN_EDGE_EXTERNAL 4

#include <stddef.h>

struct LmOwnPtrStack {
    void **items;
    size_t count;
    size_t capacity;
    void (*delete_item)(void *object);
};
struct LmOwnValueStack {
    void *items;
    size_t count;
    size_t capacity;
    size_t item_size;
};
struct LmOwnAllocationDescriptor {
    void *address;
    LmOwnArena *owner;
    size_t bytes;
    size_t element_size;
    size_t count;
    size_t rank;
    size_t level;
};
struct LmOwnLazyEdge {
    LmOwnEdgeKind kind;
    LmOwnArena *source_owner;
    LmOwnArena *target_owner;
    const void *source;
    size_t size;
    const void **patch_slot;
};
struct LmOwnArena {
    LmOwnPtrStack allocations;
    LmOwnValueStack allocation_descriptors;
    LmOwnValueStack lazy_edges;
    int frozen;
};
typedef struct LmBuildOptions {
    int full_build;
} LmBuildOptions;

typedef void (*LmOwnDestroyFields)(void *object);
typedef void (*LmOwnDelete)(void *object);

void * lm_own_new_zero(size_t size);
void * lm_own_array_new_zero(size_t element_size, size_t count, size_t rank, size_t level);
const LmOwnAllocationDescriptor * lm_own_allocation_descriptor(const void *address);
void lm_own_delete(void *object, LmOwnDestroyFields destroy_fields);
void lm_own_pointer_array_delete(void **items, size_t count, LmOwnDelete delete_item);
void lm_own_ptr_stack_init(LmOwnPtrStack *stack, LmOwnDelete delete_item);
void lm_own_ptr_stack_destroy(LmOwnPtrStack *stack);
int lm_own_ptr_stack_push(LmOwnPtrStack *stack, void *item);
void * lm_own_ptr_stack_pop(LmOwnPtrStack *stack);
void * lm_own_ptr_stack_at(const LmOwnPtrStack *stack, size_t index);
void * lm_own_ptr_stack_top(const LmOwnPtrStack *stack);
void lm_own_ptr_stack_truncate(LmOwnPtrStack *stack, size_t count);
void lm_own_value_stack_init(LmOwnValueStack *stack, size_t item_size);
void lm_own_value_stack_destroy(LmOwnValueStack *stack);
int lm_own_value_stack_push(LmOwnValueStack *stack, const void *item);
int lm_own_value_stack_resize_zero(LmOwnValueStack *stack, size_t count);
int lm_own_value_stack_pop(LmOwnValueStack *stack, void *out_item);
void * lm_own_value_stack_at(const LmOwnValueStack *stack, size_t index);
void * lm_own_value_stack_top(const LmOwnValueStack *stack);
void lm_own_value_stack_truncate(LmOwnValueStack *stack, size_t count);
void lm_own_arena_init(LmOwnArena *arena);
void lm_own_arena_destroy(LmOwnArena *arena);
void * lm_own_arena_new_zero(LmOwnArena *arena, size_t size);
void * lm_own_arena_array_new_zero(LmOwnArena *arena, size_t element_size, size_t count, size_t rank, size_t level);
const LmOwnAllocationDescriptor * lm_own_arena_allocation_descriptor(const LmOwnArena *arena, const void *address);
char * lm_own_arena_copy_bytes(LmOwnArena *arena, const char *source, size_t length);
int lm_own_arena_add_lazy_edge(LmOwnArena *target, LmOwnArena *source, const void *source_ptr, size_t size, const void **patch_slot);
int lm_own_arena_promote_lazy_edges(LmOwnArena *arena);
int lm_own_arena_absorb(LmOwnArena *target, LmOwnArena *source);
void lm_own_arena_freeze(LmOwnArena *arena);
int lm_own_arena_is_frozen(const LmOwnArena *arena);
int lm_own_tree_cut(LmOwnArena *arena);
int lm_own_tree_cut_promote_lazy_edges(LmOwnArena *arena);


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <direct.h>
static char * lm_build_exe_suffix(void) {
    return ".exe";
}

static char * lm_build_path_sep(void) {
    return "\\";
}

static int lm_build_platform_chdir(char *path) {
    return _chdir(path);
}

static char * lm_build_platform_getcwd(char *buffer, size_t size) {
    return _getcwd(buffer, size);
}

static int lm_build_platform_absolute(char *path) {
    return ((path[0] == '/' || path[0] == '\\') && (path[1] == '/' || path[1] == '\\')) || (path[0] != '\0' && path[1] == ':' && (path[2] == '/' || path[2] == '\\'));
}

static int lm_build_file_exists(char *path) {
    FILE *file;
    file = fopen(path, "rb");
    if (file == 0) {
        return 0;
    }
    fclose(file);
    return 1;
}

static int lm_build_has_qt_cmake(void) {
    return lm_build_file_exists("C:/Qt/Tools/CMake_64/bin/cmake.exe");
}

static int lm_build_has_qt_mingw_make(void) {
    return lm_build_file_exists("C:/Qt/Tools/mingw1310_64/bin/mingw32-make.exe");
}

static int lm_build_has_qt_gcc(void) {
    return lm_build_file_exists("C:/Qt/Tools/mingw1310_64/bin/gcc.exe");
}

static int lm_build_has_qt_gxx(void) {
    return lm_build_file_exists("C:/Qt/Tools/mingw1310_64/bin/g++.exe");
}
#else
#include <unistd.h>
static char * lm_build_exe_suffix(void) {
    return "";
}

static char * lm_build_path_sep(void) {
    return "/";
}

static int lm_build_platform_chdir(char *path) {
    return chdir(path);
}

static char * lm_build_platform_getcwd(char *buffer, size_t size) {
    return getcwd(buffer, size);
}

static int lm_build_platform_absolute(char *path) {
    return path[0] == '/';
}

static int lm_build_file_exists(char *path) {
    FILE *file;
    file = fopen(path, "rb");
    if (file == 0) {
        return 0;
    }
    fclose(file);
    return 1;
}

static int lm_build_has_qt_cmake(void) {
    return 0;
}

static int lm_build_has_qt_mingw_make(void) {
    return 0;
}

static int lm_build_has_qt_gcc(void) {
    return 0;
}

static int lm_build_has_qt_gxx(void) {
    return 0;
}
#endif
static LmBuildOptions * lm_build_options_new(void) {
    return lm_own_new_zero(sizeof(LmBuildOptions));
}

static void lm_build_options_delete(LmBuildOptions *options) {
    lm_own_delete(options, 0);
}

static int lm_build_is_path_separator(char value) {
    return value == '/' || value == '\\';
}

static int lm_build_has_path_separator(char *path) {
    return strchr(path, '/') != 0 || strchr(path, '\\') != 0;
}

static int lm_build_is_absolute_path(char *path) {
    if (path == 0 || path[0] == '\0') {
        return 0;
    }
    if (lm_build_platform_absolute(path)) {
        return 1;
    }
    return 0;
}

static int lm_build_join_path(char *buffer, size_t size, char *base, char *tail) {
    size_t base_length;
    size_t tail_length;
    size_t used;
    base_length = strlen(base);
    tail_length = strlen(tail);
    used = base_length;
    if (base_length + tail_length + 2U >= size) {
        fprintf(stderr, "buildCore.lm0: path is too long\n");
        return 1;
    }
    memcpy(buffer, base, base_length);
    if (base_length > 0U && tail_length > 0U && lm_build_is_path_separator(base[base_length - 1U]) == 0 && lm_build_is_path_separator(tail[0]) == 0) {
        memcpy(buffer + used, lm_build_path_sep(), strlen(lm_build_path_sep()));
        used = used + strlen(lm_build_path_sep());
    }
    memcpy(buffer + used, tail, tail_length + 1U);
    return 0;
}

static int lm_build_trim_last_path_part(char *path) {
    size_t length;
    length = strlen(path);
    while (length > 0U && lm_build_is_path_separator(path[length - 1U])) {
        path[length - 1U] = '\0';
        length = length - 1U;
    }
    while (length > 0U) {
        length = length - 1U;
        if (lm_build_is_path_separator(path[length])) {
            path[length] = '\0';
            return 0;
        }
    }
    path[0] = '\0';
    return 0;
}

static int lm_build_has_project_marker(char *path) {
    char marker_path[2048];
    if (lm_build_join_path(marker_path, sizeof(marker_path), path, "lm2/buildCore.lmx") != 0) {
        return 0;
    }
    if (lm_build_file_exists(marker_path)) {
        return 1;
    }
    return 0;
}

static int lm_build_enter_project_root(char *program_path) {
    char search_path[1024];
    char executable_path[1024];
    char cwd[1024];
    int depth;
    if (lm_build_platform_getcwd(cwd, sizeof(cwd)) == 0) {
        fprintf(stderr, "buildCore.lm0: cannot read current directory\n");
        return 1;
    }
    if (program_path != 0 && program_path[0] != '\0' && lm_build_has_path_separator(program_path)) {
        if (lm_build_is_absolute_path(program_path)) {
            if (strlen(program_path) >= sizeof(executable_path)) {
                fprintf(stderr, "buildCore.lm0: executable path is too long\n");
                return 1;
            }
            strcpy(executable_path, program_path);
        }
        if (lm_build_is_absolute_path(program_path) == 0) {
            if (lm_build_join_path(executable_path, sizeof(executable_path), cwd, program_path) != 0) {
                return 1;
            }
        }
        strcpy(search_path, executable_path);
        lm_build_trim_last_path_part(search_path);
    }
    if (program_path == 0 || program_path[0] == '\0' || lm_build_has_path_separator(program_path) == 0) {
        strcpy(search_path, cwd);
    }
    depth = 0;
    while (depth < 12 && search_path[0] != '\0') {
        if (lm_build_has_project_marker(search_path)) {
            if (lm_build_platform_chdir(search_path) != 0) {
                fprintf(stderr, "buildCore.lm0: cannot enter project root %s\n", search_path);
                return 1;
            }
            return 0;
        }
        lm_build_trim_last_path_part(search_path);
        depth = depth + 1;
    }
    fprintf(stderr, "buildCore.lm0: cannot locate project root from %s\n", cwd);
    return 1;
}

static char * lm_build_env_or_default(char *name, char *fallback) {
    char *value;
    value = getenv(name);
    if (value == 0 || value[0] == '\0') {
        return fallback;
    }
    return value;
}

static char * lm_build_default_cmake(void) {
    if (lm_build_has_qt_cmake()) {
        return "C:/Qt/Tools/CMake_64/bin/cmake.exe";
    }
    return "cmake";
}

static char * lm_build_default_generator(void) {
    if (lm_build_has_qt_mingw_make()) {
        return "MinGW Makefiles";
    }
    return "";
}

static char * lm_build_default_make_program(void) {
    if (lm_build_has_qt_mingw_make()) {
        return "C:/Qt/Tools/mingw1310_64/bin/mingw32-make.exe";
    }
    return "";
}

static char * lm_build_default_cc(void) {
    if (lm_build_has_qt_gcc()) {
        return "C:/Qt/Tools/mingw1310_64/bin/gcc.exe";
    }
    return "";
}

static char * lm_build_default_cxx(void) {
    if (lm_build_has_qt_gxx()) {
        return "C:/Qt/Tools/mingw1310_64/bin/g++.exe";
    }
    return "";
}

static void lm_build_print_usage(void) {
    printf("usage: buildCore.lm0 [--full]\n");
    printf("  --full  refresh L0 tools, then build the bundled third_party profile\n");
}

static int lm_build_parse_options(int argc, char **argv, LmBuildOptions *options) {
    int index;
    options->full_build = 0;
    index = 1;
    while (index < argc) {
        if (strcmp(argv[index], "--full") == 0) {
            options->full_build = 1;
        }
        if (strcmp(argv[index], "--help") == 0 || strcmp(argv[index], "-h") == 0) {
            lm_build_print_usage();
            return 2;
        }
        if (strcmp(argv[index], "--full") != 0 && strcmp(argv[index], "--help") != 0 && strcmp(argv[index], "-h") != 0) {
            fprintf(stderr, "buildCore.lm0: unknown option: %s\n", argv[index]);
            lm_build_print_usage();
            return 1;
        }
        index = index + 1;
    }
    return 0;
}

static size_t lm_build_append(char *buffer, size_t size, size_t used, char *text) {
    size_t length;
    length = strlen(text);
    if (used + length >= size) {
        fprintf(stderr, "buildCore.lm0: command line is too long\n");
        return size;
    }
    memcpy(buffer + used, text, length + 1U);
    return used + length;
}

static size_t lm_build_append_arg(char *buffer, size_t size, size_t used, char *arg) {
    used = lm_build_append(buffer, size, used, " \"");
    if (used == size) {
        return size;
    }
    used = lm_build_append(buffer, size, used, arg);
    if (used == size) {
        return size;
    }
    return lm_build_append(buffer, size, used, "\"");
}

static size_t lm_build_append_prefixed_arg(char *buffer, size_t size, size_t used, char *prefix, char *value) {
    char arg[2048];
    if (strlen(prefix) + strlen(value) >= sizeof(arg)) {
        fprintf(stderr, "buildCore.lm0: CMake argument is too long\n");
        return size;
    }
    strcpy(arg, prefix);
    strcat(arg, value);
    return lm_build_append_arg(buffer, size, used, arg);
}

static int lm_build_run(char *command) {
    int status;
    printf("%s\n", command);
    status = system(command);
    if (status != 0) {
        fprintf(stderr, "buildCore.lm0: command failed with status %d\n", status);
        return 1;
    }
    return 0;
}

static int lm_build_make(char *make_tool, char *operation, char *args) {
    char command[8192];
    size_t used;
    used = 0U;
    command[0] = '\0';
    used = lm_build_append(command, sizeof(command), used, make_tool);
    if (used == sizeof(command)) {
        return 1;
    }
    used = lm_build_append_arg(command, sizeof(command), used, operation);
    if (used == sizeof(command)) {
        return 1;
    }
    if (args != 0 && args[0] != '\0') {
        used = lm_build_append(command, sizeof(command), used, " ");
        if (used == sizeof(command)) {
            return 1;
        }
        used = lm_build_append(command, sizeof(command), used, args);
        if (used == sizeof(command)) {
            return 1;
        }
    }
    return lm_build_run(command);
}

static int lm_build_trans(char *trans_tool, char *source_path, char *output_path) {
    char command[4096];
    size_t used;
    used = 0U;
    command[0] = '\0';
    used = lm_build_append(command, sizeof(command), used, trans_tool);
    if (used == sizeof(command)) {
        return 1;
    }
    used = lm_build_append_arg(command, sizeof(command), used, source_path);
    if (used == sizeof(command)) {
        return 1;
    }
    used = lm_build_append_arg(command, sizeof(command), used, output_path);
    if (used == sizeof(command)) {
        return 1;
    }
    return lm_build_run(command);
}

static int lm_build_generate_all(char *trans_tool) {
    if (lm_build_trans(trans_tool, "lm2/own.lm2", "lm1/build/own.lm1.c") != 0) {
        return 1;
    }
    if (lm_build_trans(trans_tool, "lm2/trans.lm2", "lm1/build/trans.lm1.c") != 0) {
        return 1;
    }
    if (lm_build_trans(trans_tool, "lm2/parser.lm2", "lm1/build/parser.lm1.c") != 0) {
        return 1;
    }
    if (lm_build_trans(trans_tool, "lm2/printTree.lm2", "lm1/build/printTree.lm1.c") != 0) {
        return 1;
    }
    if (lm_build_trans(trans_tool, "lm2/finalize.lm2", "lm1/build/finalize.lm1.c") != 0) {
        return 1;
    }
    if (lm_build_trans(trans_tool, "lm2/make.lm2", "lm1/build/make.lm1.c") != 0) {
        return 1;
    }
    if (lm_build_trans(trans_tool, "lm2/vcpkgFetch.lm2", "lm1/build/vcpkgFetch.lm1.c") != 0) {
        return 1;
    }
    if (lm_build_trans(trans_tool, "lm2/buildCore.lmx", "lm1/build/buildCore.lm1.c") != 0) {
        return 1;
    }
    return 0;
}

static int lm_build_parser_library(char *make_tool) {
    if (lm_build_make(make_tool, "cc", "-std=c99 -Wall -Wextra -Wpedantic -I\"lm1\" -c \"lm1/build/parser.lm1.c\" -o \"build/obj/parser.lm1.o\"") != 0) {
        return 1;
    }
    remove("build/lm0/libparser.lm0.a");
    if (lm_build_make(make_tool, "ar", "rcs \"build/lm0/libparser.lm0.a\" \"build/obj/parser.lm1.o\"") != 0) {
        return 1;
    }
    return lm_build_make(make_tool, "ranlib", "\"build/lm0/libparser.lm0.a\"");
}

static int lm_build_own_library(char *make_tool) {
    if (lm_build_make(make_tool, "cc", "-std=c99 -Wall -Wextra -Wpedantic -I\"lm1\" -c \"lm1/build/own.lm1.c\" -o \"build/obj/own.lm1.o\"") != 0) {
        return 1;
    }
    remove("build/lm0/libown.lm0.a");
    if (lm_build_make(make_tool, "ar", "rcs \"build/lm0/libown.lm0.a\" \"build/obj/own.lm1.o\"") != 0) {
        return 1;
    }
    return lm_build_make(make_tool, "ranlib", "\"build/lm0/libown.lm0.a\"");
}

static int lm_build_compile_trans(char *make_tool, char *parser_library, char *own_library) {
    char command[4096];
    snprintf(command, sizeof(command), "-std=c99 -Wall -Wextra -Wpedantic -I\"lm1\" \"lm1/build/trans.lm1.c\" \"%s\" \"%s\" -o \"build/lm0/trans.lm0%s\"", parser_library, own_library, lm_build_exe_suffix());
    if (lm_build_make(make_tool, "link", command) != 0) {
        return 1;
    }
    return 0;
}

static int lm_build_compile_generated_tools(char *make_tool) {
    char command[4096];
    snprintf(command, sizeof(command), "-std=c99 -Wall -Wextra -Wpedantic \"lm1/build/make.lm1.c\" -o \"build/lm0/make.next.lm0%s\"", lm_build_exe_suffix());
    if (lm_build_make(make_tool, "link", command) != 0) {
        return 1;
    }
    snprintf(command, sizeof(command), "-std=c99 -Wall -Wextra -Wpedantic \"lm1/build/finalize.lm1.c\" -o \"build/lm0/finalize.lm0%s\"", lm_build_exe_suffix());
    if (lm_build_make(make_tool, "link", command) != 0) {
        return 1;
    }
    snprintf(command, sizeof(command), "-std=c99 -Wall -Wextra -Wpedantic -I\"lm1\" \"lm1/build/vcpkgFetch.lm1.c\" \"build/lm0/libown.lm0.a\" -o \"build/lm0/vcpkgFetch.lm0%s\"", lm_build_exe_suffix());
    if (lm_build_make(make_tool, "link", command) != 0) {
        return 1;
    }
    snprintf(command, sizeof(command), "-std=c99 -Wall -Wextra -Wpedantic -I\"lm1\" \"lm1/build/printTree.lm1.c\" \"build/lm0/libparser.lm0.a\" \"build/lm0/libown.lm0.a\" -o \"build/lm0/printTree.lm0%s\"", lm_build_exe_suffix());
    if (lm_build_make(make_tool, "link", command) != 0) {
        return 1;
    }
    snprintf(command, sizeof(command), "-std=c99 -Wall -Wextra -Wpedantic -I\"lm1\" \"lm1/build/buildCore.lm1.c\" \"build/lm0/libown.lm0.a\" -o \"build/lm0/buildCore.next.lm0%s\"", lm_build_exe_suffix());
    return lm_build_make(make_tool, "link", command);
}

static int lm_build_defer_finalize(void) {
    char command[256];
    snprintf(command, sizeof(command), "build%slm0%sfinalize.lm0%s --defer", lm_build_path_sep(), lm_build_path_sep(), lm_build_exe_suffix());
    return lm_build_run(command);
}

static int lm_build_extract_third_party_zips(void) {
    char *cmake_tool;
    char command[4096];
    size_t used;
    cmake_tool = lm_build_env_or_default("LM_CMAKE", lm_build_default_cmake());
    used = 0U;
    command[0] = '\0';
    used = lm_build_append(command, sizeof(command), used, cmake_tool);
    if (used == sizeof(command)) {
        return 1;
    }
    used = lm_build_append_arg(command, sizeof(command), used, "-DLM_THIRD_PARTY_ARCHIVE_DIR=third_party");
    if (used == sizeof(command)) {
        return 1;
    }
    used = lm_build_append_arg(command, sizeof(command), used, "-DLM_THIRD_PARTY_EXTRACT_DIR=build/third_party");
    if (used == sizeof(command)) {
        return 1;
    }
    used = lm_build_append_arg(command, sizeof(command), used, "-P");
    if (used == sizeof(command)) {
        return 1;
    }
    used = lm_build_append_arg(command, sizeof(command), used, "cmake/ExtractThirdPartyZips.cmake");
    if (used == sizeof(command)) {
        return 1;
    }
    return lm_build_run(command);
}

static int lm_build_clear_full_cmake_cache(char *cmake_tool, char *build_dir) {
    char command[4096];
    char cache_path[2048];
    char files_path[2048];
    size_t used;
    if (strlen(build_dir) + 32U >= sizeof(cache_path)) {
        fprintf(stderr, "buildCore.lm0: full build directory is too long\n");
        return 1;
    }
    snprintf(cache_path, sizeof(cache_path), "%s/CMakeCache.txt", build_dir);
    snprintf(files_path, sizeof(files_path), "%s/CMakeFiles", build_dir);
    used = 0U;
    command[0] = '\0';
    used = lm_build_append(command, sizeof(command), used, cmake_tool);
    if (used == sizeof(command)) {
        return 1;
    }
    used = lm_build_append_arg(command, sizeof(command), used, "-E");
    if (used == sizeof(command)) {
        return 1;
    }
    used = lm_build_append_arg(command, sizeof(command), used, "rm");
    if (used == sizeof(command)) {
        return 1;
    }
    used = lm_build_append_arg(command, sizeof(command), used, "-f");
    if (used == sizeof(command)) {
        return 1;
    }
    used = lm_build_append_arg(command, sizeof(command), used, cache_path);
    if (used == sizeof(command)) {
        return 1;
    }
    if (lm_build_run(command) != 0) {
        return 1;
    }
    used = 0U;
    command[0] = '\0';
    used = lm_build_append(command, sizeof(command), used, cmake_tool);
    if (used == sizeof(command)) {
        return 1;
    }
    used = lm_build_append_arg(command, sizeof(command), used, "-E");
    if (used == sizeof(command)) {
        return 1;
    }
    used = lm_build_append_arg(command, sizeof(command), used, "rm");
    if (used == sizeof(command)) {
        return 1;
    }
    used = lm_build_append_arg(command, sizeof(command), used, "-rf");
    if (used == sizeof(command)) {
        return 1;
    }
    used = lm_build_append_arg(command, sizeof(command), used, files_path);
    if (used == sizeof(command)) {
        return 1;
    }
    return lm_build_run(command);
}

static int lm_build_full_configure(void) {
    char *cmake_tool;
    char *build_dir;
    char *build_type;
    char *generator;
    char *make_program;
    char *cc;
    char *cxx;
    char command[8192];
    size_t used;
    cmake_tool = lm_build_env_or_default("LM_CMAKE", lm_build_default_cmake());
    build_dir = lm_build_env_or_default("LM_FULL_BUILD_DIR", "build/libs");
    build_type = lm_build_env_or_default("LM_FULL_BUILD_TYPE", "Debug");
    generator = lm_build_env_or_default("LM_CMAKE_GENERATOR", lm_build_default_generator());
    make_program = lm_build_env_or_default("LM_CMAKE_MAKE_PROGRAM", lm_build_default_make_program());
    cc = lm_build_env_or_default("LM_CC", lm_build_default_cc());
    cxx = lm_build_env_or_default("LM_CXX", lm_build_default_cxx());
    if (lm_build_clear_full_cmake_cache(cmake_tool, build_dir) != 0) {
        return 1;
    }
    used = 0U;
    command[0] = '\0';
    used = lm_build_append(command, sizeof(command), used, cmake_tool);
    if (used == sizeof(command)) {
        return 1;
    }
    used = lm_build_append_arg(command, sizeof(command), used, "-S");
    if (used == sizeof(command)) {
        return 1;
    }
    used = lm_build_append_arg(command, sizeof(command), used, ".");
    if (used == sizeof(command)) {
        return 1;
    }
    used = lm_build_append_arg(command, sizeof(command), used, "-B");
    if (used == sizeof(command)) {
        return 1;
    }
    used = lm_build_append_arg(command, sizeof(command), used, build_dir);
    if (used == sizeof(command)) {
        return 1;
    }
    if (generator[0] != '\0') {
        used = lm_build_append_arg(command, sizeof(command), used, "-G");
        if (used == sizeof(command)) {
            return 1;
        }
        used = lm_build_append_arg(command, sizeof(command), used, generator);
        if (used == sizeof(command)) {
            return 1;
        }
    }
    used = lm_build_append_prefixed_arg(command, sizeof(command), used, "-DCMAKE_BUILD_TYPE=", build_type);
    if (used == sizeof(command)) {
        return 1;
    }
    if (make_program[0] != '\0') {
        used = lm_build_append_prefixed_arg(command, sizeof(command), used, "-DCMAKE_MAKE_PROGRAM=", make_program);
        if (used == sizeof(command)) {
            return 1;
        }
    }
    if (cc[0] != '\0') {
        used = lm_build_append_prefixed_arg(command, sizeof(command), used, "-DCMAKE_C_COMPILER=", cc);
        if (used == sizeof(command)) {
            return 1;
        }
    }
    if (cxx[0] != '\0') {
        used = lm_build_append_prefixed_arg(command, sizeof(command), used, "-DCMAKE_CXX_COMPILER=", cxx);
        if (used == sizeof(command)) {
            return 1;
        }
    }
    used = lm_build_append_arg(command, sizeof(command), used, "-DLINGVAMYXA_ENABLE_EXTERNAL_DEPS=OFF");
    if (used == sizeof(command)) {
        return 1;
    }
    used = lm_build_append_arg(command, sizeof(command), used, "-DLINGVAMYXA_ENABLE_BUNDLED_THIRD_PARTY=ON");
    if (used == sizeof(command)) {
        return 1;
    }
    used = lm_build_append_arg(command, sizeof(command), used, "-DLM_THIRD_PARTY_SOURCE_DIR=build/third_party");
    if (used == sizeof(command)) {
        return 1;
    }
    used = lm_build_append_arg(command, sizeof(command), used, "-DBUILD_LINGVAMYXA_QT_APP=OFF");
    if (used == sizeof(command)) {
        return 1;
    }
    used = lm_build_append_arg(command, sizeof(command), used, "-DBUILD_BUILD_CORE_LM0=OFF");
    if (used == sizeof(command)) {
        return 1;
    }
    used = lm_build_append_arg(command, sizeof(command), used, "-DLINGVAMYXA_BUILDCORE_ARGS=");
    if (used == sizeof(command)) {
        return 1;
    }
    return lm_build_run(command);
}

static int lm_build_full_build(void) {
    char *cmake_tool;
    char *build_dir;
    char command[4096];
    size_t used;
    cmake_tool = lm_build_env_or_default("LM_CMAKE", lm_build_default_cmake());
    build_dir = lm_build_env_or_default("LM_FULL_BUILD_DIR", "build/libs");
    used = 0U;
    command[0] = '\0';
    used = lm_build_append(command, sizeof(command), used, cmake_tool);
    if (used == sizeof(command)) {
        return 1;
    }
    used = lm_build_append_arg(command, sizeof(command), used, "--build");
    if (used == sizeof(command)) {
        return 1;
    }
    used = lm_build_append_arg(command, sizeof(command), used, build_dir);
    if (used == sizeof(command)) {
        return 1;
    }
    used = lm_build_append_arg(command, sizeof(command), used, "--target");
    if (used == sizeof(command)) {
        return 1;
    }
    used = lm_build_append_arg(command, sizeof(command), used, "third_party.lm0");
    if (used == sizeof(command)) {
        return 1;
    }
    return lm_build_run(command);
}

static int lm_build_full_project(void) {
    if (lm_build_extract_third_party_zips() != 0) {
        return 1;
    }
    if (lm_build_full_configure() != 0) {
        return 1;
    }
    return lm_build_full_build();
}

static int lm_build_run_bootstrap(LmBuildOptions *options, char *trusted_make, char *built_trans) {
    if (lm_build_make(trusted_make, "mkdir", "\"lm1/build\" \"build/obj\" \"build/lm0\"") != 0) {
        return 1;
    }
    if (lm_build_generate_all(built_trans) != 0) {
        return 1;
    }
    if (lm_build_parser_library(trusted_make) != 0) {
        return 1;
    }
    if (lm_build_own_library(trusted_make) != 0) {
        return 1;
    }
    if (lm_build_compile_trans(trusted_make, "build/lm0/libparser.lm0.a", "build/lm0/libown.lm0.a") != 0) {
        return 1;
    }
    if (lm_build_generate_all(built_trans) != 0) {
        return 1;
    }
    if (lm_build_parser_library(trusted_make) != 0) {
        return 1;
    }
    if (lm_build_own_library(trusted_make) != 0) {
        return 1;
    }
    if (lm_build_compile_trans(trusted_make, "build/lm0/libparser.lm0.a", "build/lm0/libown.lm0.a") != 0) {
        return 1;
    }
    if (lm_build_compile_generated_tools(trusted_make) != 0) {
        return 1;
    }
    if (options -> full_build) {
        if (lm_build_full_project() != 0) {
            return 1;
        }
        return lm_build_defer_finalize();
    }
    return lm_build_defer_finalize();
}

int main(int argc, char **argv) {
    char *trusted_make;
    char trusted_make_buffer[128];
    char built_trans_buffer[128];
    LmBuildOptions *options;
    int parse_status;
    int result;
    options = lm_build_options_new();
    if (options == 0) {
        return 1;
    }
    parse_status = lm_build_parse_options(argc, argv, options);
    if (parse_status == 2) {
        lm_build_options_delete(options);
        return 0;
    }
    if (parse_status != 0) {
        lm_build_options_delete(options);
        return 1;
    }
    if (lm_build_enter_project_root(argv[0]) != 0) {
        lm_build_options_delete(options);
        return 1;
    }
    snprintf(trusted_make_buffer, sizeof(trusted_make_buffer), "build%slm0%smake.lm0%s", lm_build_path_sep(), lm_build_path_sep(), lm_build_exe_suffix());
    snprintf(built_trans_buffer, sizeof(built_trans_buffer), "build%slm0%strans.lm0%s", lm_build_path_sep(), lm_build_path_sep(), lm_build_exe_suffix());
    trusted_make = lm_build_env_or_default("LM_MAKE", trusted_make_buffer);
    result = lm_build_run_bootstrap(options, trusted_make, built_trans_buffer);
    lm_build_options_delete(options);
    return result;
}
