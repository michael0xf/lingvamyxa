#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#define LM_FINALIZE_EXE_SUFFIX ".exe"
#define LM_FINALIZE_PATH_SEP "\\"
#else
#define LM_FINALIZE_EXE_SUFFIX ""
#define LM_FINALIZE_PATH_SEP "/"
#endif

static int lm_finalize_copy_file(const char *source_path, const char *output_path) {
    unsigned char buffer[32768];
    FILE *source;
    FILE *output;
    size_t count;
    int status;

    source = fopen(source_path, "rb");
    if (source == NULL) {
        fprintf(stderr, "finalize.lm0: cannot open input file %s\n", source_path);
        return 1;
    }

    output = fopen(output_path, "wb");
    if (output == NULL) {
        fprintf(stderr, "finalize.lm0: cannot open output file %s\n", output_path);
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
        fprintf(stderr, "finalize.lm0: cannot copy %s to %s\n", source_path, output_path);
        return 1;
    }

    printf("finalize.lm0: copied %s to %s\n", source_path, output_path);
    return 0;
}

int main(int argc, char **argv) {
    char source_path[256];
    char output_path[256];

    if (argc != 1) {
        fprintf(stderr, "usage: finalize.lm0[.exe]\n");
        return 1;
    }

    (void)argv;

    snprintf(
        source_path,
        sizeof(source_path),
        "build%slm0%sbuildCore.lm0%s",
        LM_FINALIZE_PATH_SEP,
        LM_FINALIZE_PATH_SEP,
        LM_FINALIZE_EXE_SUFFIX
    );
    snprintf(
        output_path,
        sizeof(output_path),
        "lm0%sbuildCore.lm0%s",
        LM_FINALIZE_PATH_SEP,
        LM_FINALIZE_EXE_SUFFIX
    );
    return lm_finalize_copy_file(source_path, output_path);
}
