#include <stdio.h>
#include <string.h>

int main (int argc, char ** argv)
{
    const char * marker = 0;
    int i;
    FILE * f;
    for (i = 1; i + 1 < argc; i++) {
        if (strcmp(argv[i], "-marker") == 0)
            marker = argv[i + 1];
    }
    if (marker == 0)
        return 2;
    if (strlen(marker) > 2000)
        return 3;
    f = fopen(marker, "w");
    if (f == 0)
        return 4;
    fprintf(f, "invoked\n");
    fclose(f);
    return 0;
}