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

int main(int argc, char **argv) {
    const char *trusted_make;
    char trusted_make_buffer[128];
    char built_trans_buffer[128];

    (void)argc;

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

    return lm_build_defer_finalize();
}
