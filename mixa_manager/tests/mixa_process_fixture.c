/* Harmless real child process for mixa_process_win32 selftest (ticket
 * 20260912-144000). Not L1 -- a plain, small, hand-written fixture, the
 * same precedent mixa_app_fixture_invoke.c already established for this
 * codebase's own process/launch tests.
 *
 * Modes (argv[1]):
 *   "echo"   reads stdin line by line, writes "E:<line>\n" to stdout for
 *            each one, immediately (unbuffered) so a caller sees each
 *            reply without waiting for EOF; exits 0 on EOF or on a line
 *            that is exactly "QUIT".
 *   "exit"   exits immediately with the code given in argv[2].
 *   "stdincheck"  attempts one raw ReadFile on its own STD_INPUT_HANDLE
 *            and prints exactly one of "DATA:<n>", "EOF" (a genuine
 *            pipe with no writer left, i.e. ERROR_BROKEN_PIPE or a
 *            zero-byte successful read) or "INVALID:<code>" (anything
 *            else, notably ERROR_INVALID_HANDLE) -- distinguishing a
 *            valid closed-pipe EOF from an invalid/missing standard
 *            handle, which a plain C stdio fgets() cannot tell apart.
 *   (none)   exits 0 immediately.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

int main(int argc, char **argv) {
    setvbuf(stdout, NULL, _IONBF, 0);
    if (argc >= 2 && strcmp(argv[1], "echo") == 0) {
        char line[4096];
        while (fgets(line, sizeof(line), stdin) != NULL) {
            size_t n = strlen(line);
            while (n > 0 && (line[n - 1] == '\n' || line[n - 1] == '\r')) {
                line[--n] = '\0';
            }
            if (strcmp(line, "QUIT") == 0) {
                return 0;
            }
            printf("E:%s\n", line);
        }
        return 0;
    }
    if (argc >= 3 && strcmp(argv[1], "exit") == 0) {
        return atoi(argv[2]);
    }
    if (argc >= 2 && strcmp(argv[1], "stdincheck") == 0) {
        HANDLE h = GetStdHandle(STD_INPUT_HANDLE);
        char buf[16];
        DWORD got = 0;
        BOOL ok = ReadFile(h, buf, (DWORD)sizeof(buf), &got, NULL);
        if (ok) {
            if (got == 0) {
                printf("EOF\n");
            } else {
                printf("DATA:%lu\n", (unsigned long)got);
            }
        } else {
            DWORD gle = GetLastError();
            if (gle == 109UL) { /* ERROR_BROKEN_PIPE */
                printf("EOF\n");
            } else {
                printf("INVALID:%lu\n", (unsigned long)gle);
            }
        }
        return 0;
    }
    return 0;
}
