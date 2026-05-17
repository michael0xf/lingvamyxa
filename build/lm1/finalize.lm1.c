#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#define LM_FINALIZE_GETCWD _getcwd
#define LM_FINALIZE_CHDIR _chdir
#else
#include <unistd.h>
#define LM_FINALIZE_GETCWD getcwd
#define LM_FINALIZE_CHDIR chdir
#endif

#ifdef _WIN32
#define LM_FINALIZE_EXE_SUFFIX ".exe"
#define LM_FINALIZE_PATH_SEP "\\"
#else
#define LM_FINALIZE_EXE_SUFFIX ""
#define LM_FINALIZE_PATH_SEP "/"
#endif

static int lm_finalize_is_path_separator(char value) {
    return value == '/' || value == '\\';
}

static int lm_finalize_is_absolute_path(const char *path) {
    if (path == NULL || path[0] == '\0') {
        return 0;
    }

#ifdef _WIN32
    if (lm_finalize_is_path_separator(path[0])) {
        return 1;
    }
    return path[0] != '\0' && path[1] == ':';
#else
    return path[0] == '/';
#endif
}

static int lm_finalize_trim_last_path_part(char *path) {
    size_t length;

    length = strlen(path);
    while (length > 0U && lm_finalize_is_path_separator(path[length - 1U])) {
        path[length - 1U] = '\0';
        --length;
    }

    while (length > 0U) {
        --length;
        if (lm_finalize_is_path_separator(path[length])) {
            path[length] = '\0';
            return 0;
        }
    }

    path[0] = '\0';
    return 0;
}

static int lm_finalize_enter_project_root(const char *program_path) {
    char root_path[1024];

    if (!lm_finalize_is_absolute_path(program_path)) {
        return 0;
    }
    if (strlen(program_path) >= sizeof(root_path)) {
        fprintf(stderr, "finalize.lm0: executable path is too long\n");
        return 1;
    }

    strcpy(root_path, program_path);
    lm_finalize_trim_last_path_part(root_path);
    lm_finalize_trim_last_path_part(root_path);
    lm_finalize_trim_last_path_part(root_path);

    if (root_path[0] == '\0') {
        return 0;
    }

    if (LM_FINALIZE_CHDIR(root_path) != 0) {
        fprintf(stderr, "finalize.lm0: cannot enter project root %s\n", root_path);
        return 1;
    }

    return 0;
}

static void lm_finalize_log(const char *message) {
    FILE *log_file;

    log_file = fopen("build/lm0/finalize.log", "a");
    if (log_file == NULL) {
        return;
    }

    fprintf(log_file, "%s\n", message);
    fclose(log_file);
}

static void lm_finalize_sleep_retry(void) {
#ifdef _WIN32
    Sleep(250U);
#else
    usleep(250000U);
#endif
}

static void lm_finalize_tool_path(char *path, size_t size, const char *tool_name, const char *variant) {
    snprintf(
        path,
        size,
        "build%slm0%s%s%s.lm0%s",
        LM_FINALIZE_PATH_SEP,
        LM_FINALIZE_PATH_SEP,
        tool_name,
        variant,
        LM_FINALIZE_EXE_SUFFIX
    );
}

static int lm_finalize_copy_once(const char *source_path, const char *output_path, int quiet) {
    unsigned char buffer[32768];
    FILE *source;
    FILE *output;
    size_t count;
    int status;

    source = fopen(source_path, "rb");
    if (source == NULL) {
        if (!quiet) {
            fprintf(stderr, "finalize.lm0: cannot open input file %s\n", source_path);
        }
        return 1;
    }

    output = fopen(output_path, "wb");
    if (output == NULL) {
        if (!quiet) {
            fprintf(stderr, "finalize.lm0: cannot open output file %s\n", output_path);
        }
        fclose(source);
        return 1;
    }

    status = 0;
    while ((count = fread(buffer, 1U, sizeof(buffer), source)) > 0U) {
        if (fwrite(buffer, 1U, count, output) != count) {
            status = 1;
            break;
        }
    }

    if (ferror(source)) {
        status = 1;
    }
    if (fclose(output) != 0) {
        status = 1;
    }
    fclose(source);

    if (status != 0) {
        if (!quiet) {
            fprintf(stderr, "finalize.lm0: cannot copy %s to %s\n", source_path, output_path);
        }
        return 1;
    }

    return 0;
}

static int lm_finalize_copy_with_retry(const char *source_path, const char *output_path) {
    int attempt;

    for (attempt = 0; attempt < 40; ++attempt) {
        if (lm_finalize_copy_once(source_path, output_path, 1) == 0) {
            printf("finalize.lm0: copied %s to %s\n", source_path, output_path);
            return 0;
        }
        lm_finalize_sleep_retry();
    }

    if (lm_finalize_copy_once(source_path, output_path, 0) != 0) {
        return 1;
    }
    printf("finalize.lm0: copied %s to %s\n", source_path, output_path);
    return 0;
}

static int lm_finalize_install_next_tool(const char *tool_name) {
    char source_path[256];
    char output_path[256];

    lm_finalize_tool_path(source_path, sizeof(source_path), tool_name, ".next");
    lm_finalize_tool_path(output_path, sizeof(output_path), tool_name, "");

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

static int lm_finalize_defer(void) {
    char root_path[1024];
    char finalize_path[1400];
    char command[4096];
#ifdef _WIN32
    STARTUPINFOA startup_info;
    PROCESS_INFORMATION process_info;
#endif
    int status;

    if (LM_FINALIZE_GETCWD(root_path, sizeof(root_path)) == NULL) {
        fprintf(stderr, "finalize.lm0: cannot get current directory\n");
        return 1;
    }

    snprintf(
        finalize_path,
        sizeof(finalize_path),
        "%s%sbuild%slm0%sfinalize.lm0%s",
        root_path,
        LM_FINALIZE_PATH_SEP,
        LM_FINALIZE_PATH_SEP,
        LM_FINALIZE_PATH_SEP,
        LM_FINALIZE_EXE_SUFFIX
    );

#ifdef _WIN32
    snprintf(
        command,
        sizeof(command),
        "\"%s\" --copy",
        finalize_path
    );
    memset(&startup_info, 0, sizeof(startup_info));
    memset(&process_info, 0, sizeof(process_info));
    startup_info.cb = sizeof(startup_info);

    status = CreateProcessA(
        NULL,
        command,
        NULL,
        NULL,
        FALSE,
        CREATE_NO_WINDOW,
        NULL,
        root_path,
        &startup_info,
        &process_info
    ) ? 0 : 1;
    if (status == 0) {
        CloseHandle(process_info.hThread);
        CloseHandle(process_info.hProcess);
    }
#else
    snprintf(
        command,
        sizeof(command),
        "cd \"%s\" && \"%s\" \"--copy\" >/dev/null 2>&1 &",
        root_path,
        finalize_path
    );
    status = system(command);
#endif

    if (status != 0) {
        fprintf(stderr, "finalize.lm0: cannot schedule deferred finalize step\n");
        lm_finalize_log("defer failed");
        return 1;
    }

    lm_finalize_log("defer scheduled");
    printf("finalize.lm0: scheduled deferred buildCore install\n");
    return 0;
}

int main(int argc, char **argv) {
    if (lm_finalize_enter_project_root(argv[0]) != 0) {
        return 1;
    }

    if (argc == 2 && strcmp(argv[1], "--defer") == 0) {
        return lm_finalize_defer();
    }

    if (argc != 1 && !(argc == 2 && strcmp(argv[1], "--copy") == 0)) {
        fprintf(stderr, "usage: finalize.lm0[.exe] [--defer|--copy]\n");
        return 1;
    }

    lm_finalize_log("copy started");

    if (lm_finalize_install_next_tool("make") != 0) {
        return 1;
    }

    if (lm_finalize_install_next_tool("buildCore") != 0) {
        return 1;
    }

    lm_finalize_log("copy completed");
    return 0;
}
