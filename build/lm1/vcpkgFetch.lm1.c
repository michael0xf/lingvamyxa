#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "own.lm1.h"

#if defined(_WIN32)
#include <direct.h>
#else
#include <unistd.h>
#endif

#if defined(_WIN32)
static char *lm_vcpkg_platform_getcwd(char *buffer, size_t size) {
    return _getcwd(buffer, (int)size);
}

static int lm_vcpkg_platform_chdir(const char *path) {
    return _chdir(path);
}

static char *lm_vcpkg_platform_path_sep(void) {
    return "\\";
}

static char *lm_vcpkg_platform_tool_file(void) {
    return "vcpkg.exe";
}

static char *lm_vcpkg_platform_default_triplet(void) {
    return "x64-mingw-dynamic";
}

static int lm_vcpkg_platform_is_drive_absolute(const char *path) {
    return
        path[0] != '\0' &&
        path[1] == ':' &&
        (path[2] == '/' || path[2] == '\\');
}

static int lm_vcpkg_platform_has_default_tool(void) {
    return 1;
}

static char *lm_vcpkg_platform_default_tool(void) {
    return "C:/vcpkg/vcpkg.exe";
}

static char *lm_vcpkg_platform_default_root(void) {
    return "C:/vcpkg";
}

static int lm_vcpkg_platform_wrap_command(void) {
    return 1;
}
#elif defined(__APPLE__)
static char *lm_vcpkg_platform_getcwd(char *buffer, size_t size) {
    return getcwd(buffer, size);
}

static int lm_vcpkg_platform_chdir(const char *path) {
    return chdir(path);
}

static char *lm_vcpkg_platform_path_sep(void) {
    return "/";
}

static char *lm_vcpkg_platform_tool_file(void) {
    return "vcpkg";
}

static char *lm_vcpkg_platform_default_triplet(void) {
    return "x64-osx";
}

static int lm_vcpkg_platform_is_drive_absolute(const char *path) {
    (void)path;
    return 0;
}

static int lm_vcpkg_platform_has_default_tool(void) {
    return 0;
}

static char *lm_vcpkg_platform_default_tool(void) {
    return "";
}

static char *lm_vcpkg_platform_default_root(void) {
    return "";
}

static int lm_vcpkg_platform_wrap_command(void) {
    return 0;
}
#else
static char *lm_vcpkg_platform_getcwd(char *buffer, size_t size) {
    return getcwd(buffer, size);
}

static int lm_vcpkg_platform_chdir(const char *path) {
    return chdir(path);
}

static char *lm_vcpkg_platform_path_sep(void) {
    return "/";
}

static char *lm_vcpkg_platform_tool_file(void) {
    return "vcpkg";
}

static char *lm_vcpkg_platform_default_triplet(void) {
    return "x64-linux";
}

static int lm_vcpkg_platform_is_drive_absolute(const char *path) {
    (void)path;
    return 0;
}

static int lm_vcpkg_platform_has_default_tool(void) {
    return 0;
}

static char *lm_vcpkg_platform_default_tool(void) {
    return "";
}

static char *lm_vcpkg_platform_default_root(void) {
    return "";
}

static int lm_vcpkg_platform_wrap_command(void) {
    return 0;
}
#endif

typedef struct LmVcpkgOptions LmVcpkgOptions;

struct LmVcpkgOptions {
    char *output_dir;
    char *manifest_root;
    char *triplet;
    char *host_triplet;
    char *vcpkg_tool;
    char *vcpkg_root;
    int dry_run;
};

static LmVcpkgOptions * lm_vcpkg_options_new(void) {
    return lm_own_new_zero(sizeof(LmVcpkgOptions));
}

static void lm_vcpkg_options_delete(LmVcpkgOptions *options) {
    lm_own_delete(options, 0);
}

static int lm_vcpkg_is_path_separator(char value) {
    return value == '/' || value == '\\';
}

static int lm_vcpkg_has_path_separator(char *path) {
    return strchr(path, '/') != 0 || strchr(path, '\\') != 0;
}

static int lm_vcpkg_is_absolute_path(char *path) {
    if (path == 0 || path[0] == '\0') {
        return 0;
    }
    if (lm_vcpkg_is_path_separator(path[0]) && lm_vcpkg_is_path_separator(path[1])) {
        return 1;
    }
    if (lm_vcpkg_platform_is_drive_absolute(path)) {
        return 1;
    }
    return 0;
}

static int lm_vcpkg_join_path(char *buffer, size_t size, char *base, char *tail) {
    size_t base_length;
    size_t tail_length;
    size_t used;
    size_t separator_length;
    base_length = strlen(base);
    tail_length = strlen(tail);
    used = base_length;
    separator_length = strlen(lm_vcpkg_platform_path_sep());
    if (base_length + tail_length + separator_length + 1U >= size) {
        fprintf(stderr, "vcpkgFetch.lm0: path is too long\n");
        return 1;
    }
    memcpy(buffer, base, base_length);
    if (base_length > 0U && tail_length > 0U && lm_vcpkg_is_path_separator(base[base_length - 1U]) == 0 && lm_vcpkg_is_path_separator(tail[0]) == 0) {
        memcpy(buffer + used, lm_vcpkg_platform_path_sep(), separator_length);
        used = used + separator_length;
    }
    memcpy(buffer + used, tail, tail_length + 1U);
    return 0;
}

static int lm_vcpkg_trim_last_path_part(char *path) {
    size_t length;
    length = strlen(path);
    while (length > 0U && lm_vcpkg_is_path_separator(path[length - 1U])) {
        path[length-1U] = '\0';
        length = length - 1U;
    }
    while (length > 0U) {
        length = length - 1U;
        if (lm_vcpkg_is_path_separator(path[length])) {
            path[length] = '\0';
            return 0;
        }
    }
    path[0] = '\0';
    return 0;
}

static int lm_vcpkg_file_exists(char *path) {
    FILE *file;
    file = fopen(path, "rb");
    if (file == 0) {
        return 0;
    }
    fclose(file);
    return 1;
}

static int lm_vcpkg_has_project_marker(char *path) {
    char marker_path[2048];
    if (lm_vcpkg_join_path(marker_path, sizeof(marker_path), path, "cmake/vcpkg-sources/vcpkg.json") != 0) {
        return 0;
    }
    return lm_vcpkg_file_exists(marker_path);
}

static int lm_vcpkg_enter_project_root(char *program_path) {
    char search_path[1024];
    char executable_path[1024];
    char cwd[1024];
    int depth;
    if (lm_vcpkg_platform_getcwd(cwd, sizeof(cwd)) == 0) {
        fprintf(stderr, "vcpkgFetch.lm0: cannot read current directory\n");
        return 1;
    }
    if (program_path != 0 && program_path[0] != '\0' && lm_vcpkg_has_path_separator(program_path)) {
        if (lm_vcpkg_is_absolute_path(program_path)) {
            if (strlen(program_path) >= sizeof(executable_path)) {
                fprintf(stderr, "vcpkgFetch.lm0: executable path is too long\n");
                return 1;
            }
            strcpy(executable_path, program_path);
        }
        if (lm_vcpkg_is_absolute_path(program_path) == 0) {
            if (lm_vcpkg_join_path(executable_path, sizeof(executable_path), cwd, program_path) != 0) {
                return 1;
            }
        }
        strcpy(search_path, executable_path);
        lm_vcpkg_trim_last_path_part(search_path);
    }
    if (program_path == 0 || program_path[0] == '\0' || lm_vcpkg_has_path_separator(program_path) == 0) {
        strcpy(search_path, cwd);
    }
    depth = 0;
    while (depth < 12 && search_path[0] != '\0') {
        if (lm_vcpkg_has_project_marker(search_path)) {
            if (lm_vcpkg_platform_chdir(search_path) != 0) {
                fprintf(stderr, "vcpkgFetch.lm0: cannot enter project root %s\n", search_path);
                return 1;
            }
            return 0;
        }
        lm_vcpkg_trim_last_path_part(search_path);
        depth = depth + 1;
    }
    fprintf(stderr, "vcpkgFetch.lm0: cannot locate project root from %s\n", cwd);
    return 1;
}

static char * lm_vcpkg_env_or_default(char *name, char *fallback) {
    char *value;
    value = getenv(name);
    if (value == 0 || value[0] == '\0') {
        return fallback;
    }
    return value;
}

static size_t lm_vcpkg_append(char *buffer, size_t size, size_t used, char *text) {
    size_t length;
    length = strlen(text);
    if (used + length >= size) {
        fprintf(stderr, "vcpkgFetch.lm0: command line is too long\n");
        return size;
    }
    memcpy(buffer + used, text, length + 1U);
    return used + length;
}

static size_t lm_vcpkg_append_quoted(char *buffer, size_t size, size_t used, char *arg, int leading_space) {
    if (leading_space != 0) {
        used = lm_vcpkg_append(buffer, size, used, " ");
        if (used == size) {
            return size;
        }
    }
    used = lm_vcpkg_append(buffer, size, used, "\"");
    if (used == size) {
        return size;
    }
    used = lm_vcpkg_append(buffer, size, used, arg);
    if (used == size) {
        return size;
    }
    return lm_vcpkg_append(buffer, size, used, "\"");
}

static size_t lm_vcpkg_append_prefixed_arg(char *buffer, size_t size, size_t used, char *prefix, char *value) {
    char arg[2048];
    if (strlen(prefix) + strlen(value) >= sizeof(arg)) {
        fprintf(stderr, "vcpkgFetch.lm0: vcpkg argument is too long\n");
        return size;
    }
    strcpy(arg, prefix);
    strcat(arg, value);
    return lm_vcpkg_append_quoted(buffer, size, used, arg, 1);
}

static int lm_vcpkg_discover_tool(char *buffer, size_t size, char **root_out) {
    char *tool_env;
    char *root_env;
    char candidate[1024];
    root_out[0] = 0;
    tool_env = getenv("LM_VCPKG");
    if (tool_env != 0 && tool_env[0] != '\0') {
        if (strlen(tool_env) >= size) {
            fprintf(stderr, "vcpkgFetch.lm0: LM_VCPKG is too long\n");
            return 1;
        }
        strcpy(buffer, tool_env);
        return 0;
    }
    root_env = getenv("VCPKG_ROOT");
    if (root_env != 0 && root_env[0] != '\0') {
        if (lm_vcpkg_join_path(candidate, sizeof(candidate), root_env, lm_vcpkg_platform_tool_file()) != 0) {
            return 1;
        }
        if (lm_vcpkg_file_exists(candidate)) {
            if (strlen(candidate) >= size) {
                fprintf(stderr, "vcpkgFetch.lm0: vcpkg path is too long\n");
                return 1;
            }
            strcpy(buffer, candidate);
            root_out[0] = root_env;
            return 0;
        }
    }
    if (lm_vcpkg_platform_has_default_tool() != 0 && lm_vcpkg_file_exists(lm_vcpkg_platform_default_tool())) {
        if (strlen(lm_vcpkg_platform_default_tool()) >= size) {
            fprintf(stderr, "vcpkgFetch.lm0: default vcpkg path is too long\n");
            return 1;
        }
        strcpy(buffer, lm_vcpkg_platform_default_tool());
        root_out[0] = lm_vcpkg_platform_default_root();
        return 0;
    }
    if (strlen("vcpkg") >= size) {
        fprintf(stderr, "vcpkgFetch.lm0: fallback vcpkg command is too long\n");
        return 1;
    }
    strcpy(buffer, "vcpkg");
    return 0;
}

static void lm_vcpkg_print_usage(void) {
    printf("usage: vcpkgFetch.lm0 [options]\n");
    printf("  --output <dir>          downloads-root directory, default: build/vcpkg-downloads\n");
    printf("  --manifest-root <dir>   vcpkg source catalog, default: cmake/vcpkg-sources\n");
    printf("  --triplet <triplet>     target triplet, default: platform-specific\n");
    printf("  --host-triplet <triplet> host triplet, default: target triplet\n");
    printf("  --vcpkg <path>          vcpkg executable, or set LM_VCPKG\n");
    printf("  --vcpkg-root <dir>      vcpkg root, or set VCPKG_ROOT\n");
    printf("  --dry-run               print vcpkg plan without downloading\n");
}

static int lm_vcpkg_parse_options(int argc, char **argv, LmVcpkgOptions *options) {
    int index;
    options->output_dir = lm_vcpkg_env_or_default("LM_VCPKG_DOWNLOADS", "build/vcpkg-downloads");
    options->manifest_root = lm_vcpkg_env_or_default("LM_VCPKG_MANIFEST_ROOT", "cmake/vcpkg-sources");
    options->triplet = lm_vcpkg_env_or_default("LM_VCPKG_TRIPLET", lm_vcpkg_platform_default_triplet());
    options->host_triplet = lm_vcpkg_env_or_default("LM_VCPKG_HOST_TRIPLET", "");
    options->vcpkg_tool = lm_vcpkg_env_or_default("LM_VCPKG", "");
    options->vcpkg_root = lm_vcpkg_env_or_default("VCPKG_ROOT", "");
    options->dry_run = 0;
    index = 1;
    while (index < argc) {
        if (strcmp(argv[index], "--output") == 0 || strcmp(argv[index], "-o") == 0) {
            if (index + 1 >= argc) {
                fprintf(stderr, "vcpkgFetch.lm0: --output expects a directory\n");
                return 1;
            }
            index = index + 1;
            options->output_dir = argv[index];
            index = index + 1;
            continue;
        }
        if (strcmp(argv[index], "--manifest-root") == 0) {
            if (index + 1 >= argc) {
                fprintf(stderr, "vcpkgFetch.lm0: --manifest-root expects a directory\n");
                return 1;
            }
            index = index + 1;
            options->manifest_root = argv[index];
            index = index + 1;
            continue;
        }
        if (strcmp(argv[index], "--triplet") == 0) {
            if (index + 1 >= argc) {
                fprintf(stderr, "vcpkgFetch.lm0: --triplet expects a value\n");
                return 1;
            }
            index = index + 1;
            options->triplet = argv[index];
            index = index + 1;
            continue;
        }
        if (strcmp(argv[index], "--host-triplet") == 0) {
            if (index + 1 >= argc) {
                fprintf(stderr, "vcpkgFetch.lm0: --host-triplet expects a value\n");
                return 1;
            }
            index = index + 1;
            options->host_triplet = argv[index];
            index = index + 1;
            continue;
        }
        if (strcmp(argv[index], "--vcpkg") == 0) {
            if (index + 1 >= argc) {
                fprintf(stderr, "vcpkgFetch.lm0: --vcpkg expects a path\n");
                return 1;
            }
            index = index + 1;
            options->vcpkg_tool = argv[index];
            index = index + 1;
            continue;
        }
        if (strcmp(argv[index], "--vcpkg-root") == 0) {
            if (index + 1 >= argc) {
                fprintf(stderr, "vcpkgFetch.lm0: --vcpkg-root expects a directory\n");
                return 1;
            }
            index = index + 1;
            options->vcpkg_root = argv[index];
            index = index + 1;
            continue;
        }
        if (strcmp(argv[index], "--dry-run") == 0) {
            options->dry_run = 1;
            index = index + 1;
            continue;
        }
        if (strcmp(argv[index], "--help") == 0 || strcmp(argv[index], "-h") == 0) {
            lm_vcpkg_print_usage();
            return 2;
        }
        fprintf(stderr, "vcpkgFetch.lm0: unknown option: %s\n", argv[index]);
        lm_vcpkg_print_usage();
        return 1;
    }
    if (options -> host_triplet[0] == '\0') {
        options->host_triplet = options -> triplet;
    }
    return 0;
}

static int lm_vcpkg_build_command(LmVcpkgOptions *options, char *command, size_t size) {
    char *discovered_root = 0;
    char discovered_tool[1024];
    size_t used;
    if (options -> vcpkg_tool[0] != '\0') {
        if (strlen(options -> vcpkg_tool) >= sizeof(discovered_tool)) {
            fprintf(stderr, "vcpkgFetch.lm0: vcpkg path is too long\n");
            return 1;
        }
        strcpy(discovered_tool, options -> vcpkg_tool);
    }
    if (options -> vcpkg_tool[0] == '\0') {
        if (lm_vcpkg_discover_tool(discovered_tool, sizeof(discovered_tool), & discovered_root) != 0) {
            return 1;
        }
    }
    used = 0U;
    command[0] = '\0';
    if (lm_vcpkg_platform_wrap_command() != 0) {
        used = lm_vcpkg_append(command, size, used, "\"");
        if (used == size) {
            return 1;
        }
    }
    used = lm_vcpkg_append_quoted(command, size, used, discovered_tool, 0);
    if (used == size) {
        return 1;
    }
    used = lm_vcpkg_append_quoted(command, size, used, "install", 1);
    if (used == size) {
        return 1;
    }
    used = lm_vcpkg_append_prefixed_arg(command, size, used, "--x-manifest-root=", options -> manifest_root);
    if (used == size) {
        return 1;
    }
    used = lm_vcpkg_append_prefixed_arg(command, size, used, "--downloads-root=", options -> output_dir);
    if (used == size) {
        return 1;
    }
    used = lm_vcpkg_append_prefixed_arg(command, size, used, "--triplet=", options -> triplet);
    if (used == size) {
        return 1;
    }
    used = lm_vcpkg_append_prefixed_arg(command, size, used, "--host-triplet=", options -> host_triplet);
    if (used == size) {
        return 1;
    }
    if (options -> vcpkg_root[0] != '\0') {
        used = lm_vcpkg_append_prefixed_arg(command, size, used, "--vcpkg-root=", options -> vcpkg_root);
        if (used == size) {
            return 1;
        }
    }
    if (options -> vcpkg_root[0] == '\0' && discovered_root != 0 && discovered_root[0] != '\0') {
        used = lm_vcpkg_append_prefixed_arg(command, size, used, "--vcpkg-root=", discovered_root);
        if (used == size) {
            return 1;
        }
    }
    if (options -> dry_run != 0) {
        used = lm_vcpkg_append_quoted(command, size, used, "--dry-run", 1);
        if (used == size) {
            return 1;
        }
    }
    used = lm_vcpkg_append_quoted(command, size, used, "--only-downloads", 1);
    if (used == size) {
        return 1;
    }
    used = lm_vcpkg_append_quoted(command, size, used, "--no-print-usage", 1);
    if (used == size) {
        return 1;
    }
    if (lm_vcpkg_platform_wrap_command() != 0) {
        used = lm_vcpkg_append(command, size, used, "\"");
        if (used == size) {
            return 1;
        }
    }
    return 0;
}

int main(int argc, char **argv) {
    LmVcpkgOptions *options;
    char command[8192];
    int status;
    options = lm_vcpkg_options_new();
    if (options == 0) {
        return 1;
    }
    status = lm_vcpkg_parse_options(argc, argv, options);
    if (status == 2) {
        lm_vcpkg_options_delete(options);
        return 0;
    }
    if (status != 0) {
        lm_vcpkg_options_delete(options);
        return 1;
    }
    if (lm_vcpkg_enter_project_root(argv[0]) != 0) {
        lm_vcpkg_options_delete(options);
        return 1;
    }
    if (lm_vcpkg_build_command(options, command, sizeof(command)) != 0) {
        lm_vcpkg_options_delete(options);
        return 1;
    }
    lm_vcpkg_options_delete(options);
    printf("%s\n", command);
    status = system(command);
    if (status != 0) {
        fprintf(stderr, "vcpkgFetch.lm0: command failed with status %d\n", status);
        return 1;
    }
    return 0;
}
