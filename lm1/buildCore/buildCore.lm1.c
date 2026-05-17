#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <direct.h>
#define LM_BUILD_EXE_SUFFIX ".exe"
#define LM_BUILD_PATH_SEP "\\"
#define LM_BUILD_CHDIR _chdir
#else
#include <unistd.h>
#define LM_BUILD_EXE_SUFFIX ""
#define LM_BUILD_PATH_SEP "/"
#define LM_BUILD_CHDIR chdir
#endif

typedef struct LmBuildOptions {
    int full_build;
} LmBuildOptions;

static int lm_build_is_path_separator(char value) {
    return value == '/' || value == '\\';
}

static int lm_build_trim_last_path_part(char *path) {
    size_t length;

    length = strlen(path);
    while (length > 0U && lm_build_is_path_separator(path[length - 1U])) {
        path[length - 1U] = '\0';
        --length;
    }

    while (length > 0U) {
        --length;
        if (lm_build_is_path_separator(path[length])) {
            path[length] = '\0';
            return 0;
        }
    }

    path[0] = '\0';
    return 0;
}

static int lm_build_enter_project_root(const char *program_path) {
    char root_path[1024];

    if (program_path == NULL || program_path[0] == '\0') {
        return 0;
    }
    if (strlen(program_path) >= sizeof(root_path)) {
        fprintf(stderr, "buildCore.lm0: executable path is too long\n");
        return 1;
    }

    strcpy(root_path, program_path);

    if (strchr(root_path, '/') == NULL && strchr(root_path, '\\') == NULL) {
        return 0;
    }

    lm_build_trim_last_path_part(root_path);
    lm_build_trim_last_path_part(root_path);
    lm_build_trim_last_path_part(root_path);

    if (root_path[0] == '\0') {
        return 0;
    }

    if (LM_BUILD_CHDIR(root_path) != 0) {
        fprintf(stderr, "buildCore.lm0: cannot enter project root %s\n", root_path);
        return 1;
    }

    return 0;
}

static const char *lm_build_env_or_default(const char *name, const char *fallback) {
    const char *value;

    value = getenv(name);
    if (value == NULL || value[0] == '\0') {
        return fallback;
    }

    return value;
}

static void lm_build_print_usage(void) {
    printf("usage: buildCore.lm0 [--full]\n");
    printf("  --full  refresh L0 tools, then run the full CMake/vcpkg build profile\n");
}

static int lm_build_parse_options(int argc, char **argv, LmBuildOptions *options) {
    int index;

    options->full_build = 0;

    for (index = 1; index < argc; ++index) {
        if (strcmp(argv[index], "--full") == 0) {
            options->full_build = 1;
        } else if (strcmp(argv[index], "--help") == 0 || strcmp(argv[index], "-h") == 0) {
            lm_build_print_usage();
            return 2;
        } else {
            fprintf(stderr, "buildCore.lm0: unknown option: %s\n", argv[index]);
            lm_build_print_usage();
            return 1;
        }
    }

    return 0;
}

static int lm_build_append(char *buffer, size_t size, size_t *used, const char *text) {
    size_t length;

    length = strlen(text);
    if (*used + length >= size) {
        fprintf(stderr, "buildCore.lm0: command line is too long\n");
        return 1;
    }

    memcpy(buffer + *used, text, length + 1U);
    *used += length;
    return 0;
}

static int lm_build_append_arg(char *buffer, size_t size, size_t *used, const char *arg) {
    if (lm_build_append(buffer, size, used, " \"") != 0) {
        return 1;
    }
    if (lm_build_append(buffer, size, used, arg) != 0) {
        return 1;
    }
    return lm_build_append(buffer, size, used, "\"");
}

static int lm_build_append_prefixed_arg(
    char *buffer,
    size_t size,
    size_t *used,
    const char *prefix,
    const char *value
) {
    char arg[2048];

    if (strlen(prefix) + strlen(value) >= sizeof(arg)) {
        fprintf(stderr, "buildCore.lm0: CMake argument is too long\n");
        return 1;
    }

    strcpy(arg, prefix);
    strcat(arg, value);

    return lm_build_append_arg(buffer, size, used, arg);
}

static int lm_build_run(const char *command) {
    int status;

    printf("%s\n", command);
    status = system(command);
    if (status != 0) {
        fprintf(stderr, "buildCore.lm0: command failed with status %d\n", status);
        return 1;
    }

    return 0;
}

static int lm_build_make(const char *make_tool, const char *operation, const char *args) {
    char command[8192];
    size_t used;

    used = 0U;
    command[0] = '\0';

    if (lm_build_append(command, sizeof(command), &used, make_tool) != 0) {
        return 1;
    }
    if (lm_build_append_arg(command, sizeof(command), &used, operation) != 0) {
        return 1;
    }
    if (args != NULL && args[0] != '\0') {
        if (lm_build_append(command, sizeof(command), &used, " ") != 0) {
            return 1;
        }
        if (lm_build_append(command, sizeof(command), &used, args) != 0) {
            return 1;
        }
    }

    return lm_build_run(command);
}

static int lm_build_trans(const char *trans_tool, const char *source_path, const char *output_path) {
    char command[4096];
    size_t used;

    used = 0U;
    command[0] = '\0';

    if (lm_build_append(command, sizeof(command), &used, trans_tool) != 0) {
        return 1;
    }
    if (lm_build_append_arg(command, sizeof(command), &used, source_path) != 0) {
        return 1;
    }
    if (lm_build_append_arg(command, sizeof(command), &used, output_path) != 0) {
        return 1;
    }

    return lm_build_run(command);
}

static int lm_build_generate_all(const char *trans_tool) {
    if (lm_build_trans(trans_tool, "lm2/trans/trans.lm2", "build/lm1/trans/trans.lm1.c") != 0) {
        return 1;
    }
    if (lm_build_trans(trans_tool, "lm2/parser/parser.lm2", "build/lm1/parser/parser.lm1.c") != 0) {
        return 1;
    }
    if (lm_build_trans(trans_tool, "lm2/printTree/printTree.lm2", "build/lm1/printTree/printTree.lm1.c") != 0) {
        return 1;
    }
    if (lm_build_trans(trans_tool, "lm2/finalize/finalize.lm2", "build/lm1/finalize/finalize.lm1.c") != 0) {
        return 1;
    }
    if (lm_build_trans(trans_tool, "lm2/make/make.lm2", "build/lm1/make/make.lm1.c") != 0) {
        return 1;
    }
    return 0;
}

static int lm_build_parser_library(const char *make_tool) {
    if (lm_build_make(make_tool, "cc", "-std=c99 -Wall -Wextra -Wpedantic -I\"lm1/parser\" -c \"build/lm1/parser/parser.lm1.c\" -o \"build/obj/parser.lm1.o\"") != 0) {
        return 1;
    }
    if (lm_build_make(make_tool, "ar", "qc \"build/lm0/libparser.lm0.a\" \"build/obj/parser.lm1.o\"") != 0) {
        return 1;
    }
    return lm_build_make(make_tool, "ranlib", "\"build/lm0/libparser.lm0.a\"");
}

static int lm_build_compile_trans(const char *make_tool, const char *parser_library) {
    char command[4096];

    snprintf(
        command,
        sizeof(command),
        "-std=c99 -Wall -Wextra -Wpedantic -I\"lm1/parser\" \"build/lm1/trans/trans.lm1.c\" \"%s\" -o \"build/lm0/trans.lm0%s\"",
        parser_library,
        LM_BUILD_EXE_SUFFIX
    );
    if (lm_build_make(make_tool, "link", command) != 0) {
        return 1;
    }

    return 0;
}

static int lm_build_compile_generated_tools(const char *make_tool) {
    char command[4096];

    snprintf(
        command,
        sizeof(command),
        "-std=c99 -Wall -Wextra -Wpedantic \"build/lm1/make/make.lm1.c\" -o \"build/lm0/make.next.lm0%s\"",
        LM_BUILD_EXE_SUFFIX
    );
    if (lm_build_make(make_tool, "link", command) != 0) {
        return 1;
    }

    snprintf(
        command,
        sizeof(command),
        "-std=c99 -Wall -Wextra -Wpedantic \"build/lm1/finalize/finalize.lm1.c\" -o \"build/lm0/finalize.lm0%s\"",
        LM_BUILD_EXE_SUFFIX
    );
    if (lm_build_make(make_tool, "link", command) != 0) {
        return 1;
    }

    snprintf(
        command,
        sizeof(command),
        "-std=c99 -Wall -Wextra -Wpedantic -I\"lm1/parser\" \"build/lm1/printTree/printTree.lm1.c\" \"build/lm0/libparser.lm0.a\" -o \"build/lm0/printTree.lm0%s\"",
        LM_BUILD_EXE_SUFFIX
    );
    if (lm_build_make(make_tool, "link", command) != 0) {
        return 1;
    }

    snprintf(
        command,
        sizeof(command),
        "-std=c99 -Wall -Wextra -Wpedantic \"lm1/buildCore/buildCore.lm1.c\" -o \"build/lm0/buildCore.next.lm0%s\"",
        LM_BUILD_EXE_SUFFIX
    );
    return lm_build_make(make_tool, "link", command);
}

static int lm_build_defer_finalize(void) {
    char command[256];

    snprintf(
        command,
        sizeof(command),
        "build%slm0%sfinalize.lm0%s --defer",
        LM_BUILD_PATH_SEP,
        LM_BUILD_PATH_SEP,
        LM_BUILD_EXE_SUFFIX
    );
    return lm_build_run(command);
}

static int lm_build_full_configure(void) {
    const char *cmake_tool;
    const char *build_dir;
    const char *build_type;
    const char *generator;
    const char *make_program;
    const char *cc;
    const char *cxx;
    const char *vcpkg_root;
    const char *vcpkg_toolchain;
    const char *vcpkg_manifest_dir;
    const char *vcpkg_target_triplet;
    const char *vcpkg_host_triplet;
    char default_toolchain[1024];
    char command[8192];
    size_t used;

    cmake_tool = lm_build_env_or_default("LM_CMAKE", "cmake");
    build_dir = lm_build_env_or_default("LM_FULL_BUILD_DIR", "build/full");
    build_type = lm_build_env_or_default("LM_FULL_BUILD_TYPE", "Debug");
    generator = lm_build_env_or_default("LM_CMAKE_GENERATOR", "");
    make_program = lm_build_env_or_default("LM_CMAKE_MAKE_PROGRAM", "");
    cc = lm_build_env_or_default("LM_CC", "");
    cxx = lm_build_env_or_default("LM_CXX", "");
    vcpkg_manifest_dir = lm_build_env_or_default("LM_VCPKG_MANIFEST_DIR", "cmake/vcpkg");
    vcpkg_target_triplet = lm_build_env_or_default("LM_VCPKG_TARGET_TRIPLET", "x64-mingw-dynamic");
    vcpkg_host_triplet = lm_build_env_or_default("LM_VCPKG_HOST_TRIPLET", "x64-mingw-dynamic");

#ifdef _WIN32
    vcpkg_root = lm_build_env_or_default("LM_VCPKG_ROOT", lm_build_env_or_default("VCPKG_ROOT", "C:/vcpkg"));
#else
    vcpkg_root = lm_build_env_or_default("LM_VCPKG_ROOT", lm_build_env_or_default("VCPKG_ROOT", ""));
#endif
    vcpkg_toolchain = lm_build_env_or_default("LM_VCPKG_TOOLCHAIN_FILE", "");
    if (vcpkg_toolchain[0] == '\0') {
        if (vcpkg_root[0] == '\0') {
            fprintf(stderr, "buildCore.lm0: --full needs LM_VCPKG_TOOLCHAIN_FILE or VCPKG_ROOT\n");
            return 1;
        }
        snprintf(
            default_toolchain,
            sizeof(default_toolchain),
            "%s/scripts/buildsystems/vcpkg.cmake",
            vcpkg_root
        );
        vcpkg_toolchain = default_toolchain;
    }

    used = 0U;
    command[0] = '\0';

    if (lm_build_append(command, sizeof(command), &used, cmake_tool) != 0) {
        return 1;
    }
    if (lm_build_append_arg(command, sizeof(command), &used, "-S") != 0) {
        return 1;
    }
    if (lm_build_append_arg(command, sizeof(command), &used, ".") != 0) {
        return 1;
    }
    if (lm_build_append_arg(command, sizeof(command), &used, "-B") != 0) {
        return 1;
    }
    if (lm_build_append_arg(command, sizeof(command), &used, build_dir) != 0) {
        return 1;
    }
    if (generator[0] != '\0') {
        if (lm_build_append_arg(command, sizeof(command), &used, "-G") != 0) {
            return 1;
        }
        if (lm_build_append_arg(command, sizeof(command), &used, generator) != 0) {
            return 1;
        }
    }
    if (lm_build_append_prefixed_arg(command, sizeof(command), &used, "-DCMAKE_BUILD_TYPE=", build_type) != 0) {
        return 1;
    }
    if (make_program[0] != '\0') {
        if (lm_build_append_prefixed_arg(command, sizeof(command), &used, "-DCMAKE_MAKE_PROGRAM=", make_program) != 0) {
            return 1;
        }
    }
    if (cc[0] != '\0') {
        if (lm_build_append_prefixed_arg(command, sizeof(command), &used, "-DCMAKE_C_COMPILER=", cc) != 0) {
            return 1;
        }
    }
    if (cxx[0] != '\0') {
        if (lm_build_append_prefixed_arg(command, sizeof(command), &used, "-DCMAKE_CXX_COMPILER=", cxx) != 0) {
            return 1;
        }
    }
    if (lm_build_append_prefixed_arg(command, sizeof(command), &used, "-DCMAKE_TOOLCHAIN_FILE=", vcpkg_toolchain) != 0) {
        return 1;
    }
    if (lm_build_append_prefixed_arg(command, sizeof(command), &used, "-DVCPKG_MANIFEST_DIR=", vcpkg_manifest_dir) != 0) {
        return 1;
    }
    if (lm_build_append_prefixed_arg(command, sizeof(command), &used, "-DVCPKG_TARGET_TRIPLET=", vcpkg_target_triplet) != 0) {
        return 1;
    }
    if (lm_build_append_prefixed_arg(command, sizeof(command), &used, "-DVCPKG_HOST_TRIPLET=", vcpkg_host_triplet) != 0) {
        return 1;
    }
    if (lm_build_append_arg(command, sizeof(command), &used, "-DLINGVAMYXA_ENABLE_EXTERNAL_DEPS=ON") != 0) {
        return 1;
    }
    if (lm_build_append_arg(command, sizeof(command), &used, "-DLINGVAMYXA_ENABLE_BUNDLED_THIRD_PARTY=ON") != 0) {
        return 1;
    }
    if (lm_build_append_arg(command, sizeof(command), &used, "-DBUILD_LINGVAMYXA_QT_APP=OFF") != 0) {
        return 1;
    }
    if (lm_build_append_arg(command, sizeof(command), &used, "-DBUILD_BUILD_CORE_LM0=OFF") != 0) {
        return 1;
    }
    if (lm_build_append_arg(command, sizeof(command), &used, "-DLINGVAMYXA_BUILDCORE_ARGS=") != 0) {
        return 1;
    }

    return lm_build_run(command);
}

static int lm_build_full_build(void) {
    const char *cmake_tool;
    const char *build_dir;
    char command[4096];
    size_t used;

    cmake_tool = lm_build_env_or_default("LM_CMAKE", "cmake");
    build_dir = lm_build_env_or_default("LM_FULL_BUILD_DIR", "build/full");

    used = 0U;
    command[0] = '\0';

    if (lm_build_append(command, sizeof(command), &used, cmake_tool) != 0) {
        return 1;
    }
    if (lm_build_append_arg(command, sizeof(command), &used, "--build") != 0) {
        return 1;
    }
    if (lm_build_append_arg(command, sizeof(command), &used, build_dir) != 0) {
        return 1;
    }

    return lm_build_run(command);
}

static int lm_build_full_project(void) {
    if (lm_build_full_configure() != 0) {
        return 1;
    }
    return lm_build_full_build();
}

int main(int argc, char **argv) {
    const char *trusted_make;
    char trusted_make_buffer[128];
    char built_trans_buffer[128];
    LmBuildOptions options;
    int parse_status;

    parse_status = lm_build_parse_options(argc, argv, &options);
    if (parse_status == 2) {
        return 0;
    }
    if (parse_status != 0) {
        return 1;
    }

    if (lm_build_enter_project_root(argv[0]) != 0) {
        return 1;
    }

    snprintf(
        trusted_make_buffer,
        sizeof(trusted_make_buffer),
        "build%slm0%smake.lm0%s",
        LM_BUILD_PATH_SEP,
        LM_BUILD_PATH_SEP,
        LM_BUILD_EXE_SUFFIX
    );
    snprintf(
        built_trans_buffer,
        sizeof(built_trans_buffer),
        "build%slm0%strans.lm0%s",
        LM_BUILD_PATH_SEP,
        LM_BUILD_PATH_SEP,
        LM_BUILD_EXE_SUFFIX
    );

    trusted_make = lm_build_env_or_default("LM_MAKE", trusted_make_buffer);

    if (lm_build_make(trusted_make, "mkdir", "\"build/lm1/trans\" \"build/lm1/parser\" \"build/lm1/printTree\" \"build/lm1/finalize\" \"build/lm1/make\" \"build/obj\" \"build/lm0\"") != 0) {
        return 1;
    }

    if (lm_build_compile_trans(trusted_make, "build/lm0/libparser.lm0.a") != 0) {
        return 1;
    }

    if (lm_build_generate_all(built_trans_buffer) != 0) {
        return 1;
    }

    if (lm_build_parser_library(trusted_make) != 0) {
        return 1;
    }

    if (lm_build_compile_trans(trusted_make, "build/lm0/libparser.lm0.a") != 0) {
        return 1;
    }

    if (lm_build_generate_all(built_trans_buffer) != 0) {
        return 1;
    }

    if (lm_build_parser_library(trusted_make) != 0) {
        return 1;
    }

    if (lm_build_compile_trans(trusted_make, "build/lm0/libparser.lm0.a") != 0) {
        return 1;
    }

    if (lm_build_compile_generated_tools(trusted_make) != 0) {
        return 1;
    }

    if (options.full_build) {
        if (lm_build_full_project() != 0) {
            return 1;
        }
        return lm_build_defer_finalize();
    }

    return lm_build_defer_finalize();
}
