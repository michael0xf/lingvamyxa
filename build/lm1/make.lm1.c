#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static char * lm_make_env_or_default(char *name, char *fallback) {
    char *value;
    value = getenv(name);
    if (value == NULL || value [ 0 ] == '\0') {
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
        used = lm_make_append_arg(command, sizeof(command), used, argv [ index ]);
        if (used == sizeof(command)) {
            return 1;
        }
        index = index + 1;
    }
    return lm_make_run_command(command);
}

static int lm_make_copy_file(char *source_path, char *output_path) {
    char buffer[32768];
    FILE *source;
    FILE *output;
    size_t count;
    int status;
    source = fopen(source_path, "rb");
    if (source == NULL) {
        fprintf(stderr, "make.lm0: cannot open input file %s\n", source_path);
        return 1;
    }
    output = fopen(output_path, "wb");
    if (output == NULL) {
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
    if (strcmp(argv [ 1 ], "mkdir") == 0) {
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
            used = lm_make_append_arg(command, sizeof(command), used, argv [ index ]);
            if (used == sizeof(command)) {
                return 1;
            }
            index = index + 1;
        }
        return lm_make_run_command(command);
    }
    if (strcmp(argv [ 1 ], "cc") == 0 || strcmp(argv [ 1 ], "link") == 0) {
        return lm_make_run_tool(cc, argc, argv, 2);
    }
    if (strcmp(argv [ 1 ], "ar") == 0) {
        return lm_make_run_tool(ar, argc, argv, 2);
    }
    if (strcmp(argv [ 1 ], "ranlib") == 0) {
        return lm_make_run_tool(ranlib, argc, argv, 2);
    }
    if (strcmp(argv [ 1 ], "copy") == 0) {
        if (argc != 4) {
            lm_make_print_usage();
            return 1;
        }
        return lm_make_copy_file(argv [ 2 ], argv [ 3 ]);
    }
    lm_make_print_usage();
    return 1;
}
