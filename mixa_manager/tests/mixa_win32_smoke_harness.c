/* mixa_win32_smoke_harness.c -- external, out-of-process smoke test for
 * the REAL Win32 mixa_app_main.exe entrypoint (ticket 20260913-013609).
 *
 * This does not link against any mixa_manager L1 object: it drives the
 * production executable exactly the way a real user's window manager
 * and keyboard would -- CreateProcess to launch it, PostMessage(WM_CHAR/
 * WM_KEYDOWN/WM_CLOSE) against its real HWND to type/dispatch/close, and
 * polls the SAME production console file (<root>\mixa-console.log) the
 * app itself writes through mixa_app_controller's already-accepted
 * wiring to observe results. No second orchestration path is created;
 * this harness contains no mixa_manager application logic of its own.
 *
 * Every wait below is a bounded poll on OBSERVABLE state (a window
 * existing/visible, a substring appearing in the console file, a real
 * child process appearing/disappearing) with its own timeout used ONLY
 * as a test failure bound, never as a semantic delay standing in for a
 * real signal -- matching mixa_ingress_host_harness.c's own established
 * style for this project's native (non-L1) test harnesses.
 */

#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_checks = 0;
static int g_failures = 0;

static void check(int cond, const char *what) {
    g_checks++;
    if (!cond) {
        g_failures++;
        fprintf(stderr, "FAIL: %s\n", what);
    } else {
        printf("ok: %s\n", what);
    }
}

typedef struct FindCtx {
    DWORD pid;
    HWND hwnd;
} FindCtx;

static BOOL CALLBACK find_window_proc(HWND hwnd, LPARAM lparam) {
    FindCtx *ctx = (FindCtx *)lparam;
    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);
    if (pid == ctx->pid) {
        char cls[64];
        cls[0] = 0;
        GetClassNameA(hwnd, cls, sizeof(cls));
        if (strcmp(cls, "MixaBackendWin32") == 0) {
            ctx->hwnd = hwnd;
            return FALSE;
        }
    }
    return TRUE;
}

static HWND wait_for_window(DWORD pid, DWORD timeout_ms) {
    DWORD start = GetTickCount();
    for (;;) {
        FindCtx ctx;
        ctx.pid = pid;
        ctx.hwnd = 0;
        EnumWindows(find_window_proc, (LPARAM)&ctx);
        if (ctx.hwnd != 0) {
            return ctx.hwnd;
        }
        if (GetTickCount() - start > timeout_ms) {
            return 0;
        }
        Sleep(50);
    }
}

static char *read_whole_file(const char *path, long *out_len) {
    FILE *f = fopen(path, "rb");
    char *buf;
    long sz;
    size_t got;
    if (!f) {
        return NULL;
    }
    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return NULL;
    }
    sz = ftell(f);
    if (sz < 0) {
        fclose(f);
        return NULL;
    }
    if (fseek(f, 0, SEEK_SET) != 0) {
        fclose(f);
        return NULL;
    }
    buf = (char *)malloc((size_t)sz + 1);
    if (!buf) {
        fclose(f);
        return NULL;
    }
    got = fread(buf, 1, (size_t)sz, f);
    buf[got] = 0;
    fclose(f);
    if (out_len) {
        *out_len = (long)got;
    }
    return buf;
}

static int count_occurrences(const char *hay, const char *needle) {
    int n = 0;
    const char *p = hay;
    size_t nlen = strlen(needle);
    if (nlen == 0) {
        return 0;
    }
    while ((p = strstr(p, needle)) != NULL) {
        n++;
        p += nlen;
    }
    return n;
}

static int wait_file_occurrences_at_least(const char *path, const char *needle, int want, DWORD timeout_ms) {
    DWORD start = GetTickCount();
    for (;;) {
        char *buf = read_whole_file(path, NULL);
        if (buf) {
            int n = count_occurrences(buf, needle);
            free(buf);
            if (n >= want) {
                return 1;
            }
        }
        if (GetTickCount() - start > timeout_ms) {
            return 0;
        }
        Sleep(50);
    }
}

/* mixa_process_marker_build_wrapper spawns each command through an
 * intermediate shell (the marker wrapper), so the real command's own
 * process (e.g. PING.EXE) is a GRANDCHILD of the app, not a direct
 * child of it -- these two helpers walk the WHOLE descendant tree
 * (any depth, bounded so a cycle or snapshot anomaly cannot loop
 * forever) rather than assuming a fixed process-tree shape. */
#define MIXA_SMOKE_MAX_PROCS 4096

static int find_descendant(DWORD root_pid, const char *image_name_ci, DWORD *out_pid) {
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    static DWORD pids[MIXA_SMOKE_MAX_PROCS];
    static DWORD parents[MIXA_SMOKE_MAX_PROCS];
    static char names[MIXA_SMOKE_MAX_PROCS][MAX_PATH];
    int n = 0;
    PROCESSENTRY32 pe;
    DWORD queue[MIXA_SMOKE_MAX_PROCS];
    int qhead = 0, qtail = 0, qseen = 0;
    int i;

    if (snap == INVALID_HANDLE_VALUE) {
        return -1;
    }
    pe.dwSize = sizeof(pe);
    if (Process32First(snap, &pe)) {
        do {
            if (n < MIXA_SMOKE_MAX_PROCS) {
                pids[n] = pe.th32ProcessID;
                parents[n] = pe.th32ParentProcessID;
                strncpy(names[n], pe.szExeFile, MAX_PATH - 1);
                names[n][MAX_PATH - 1] = 0;
                n++;
            }
        } while (Process32Next(snap, &pe));
    }
    CloseHandle(snap);

    queue[qtail++] = root_pid;
    while (qhead < qtail && qseen < MIXA_SMOKE_MAX_PROCS) {
        DWORD cur = queue[qhead++];
        for (i = 0; i < n; i++) {
            if (parents[i] == cur) {
                qseen++;
                if (image_name_ci == NULL || _stricmp(names[i], image_name_ci) == 0) {
                    if (out_pid) {
                        *out_pid = pids[i];
                    }
                    return 1;
                }
                if (qtail < MIXA_SMOKE_MAX_PROCS) {
                    queue[qtail++] = pids[i];
                }
            }
        }
    }
    return 0;
}

static int child_present(DWORD parent_pid, const char *image_name_ci) {
    return find_descendant(parent_pid, image_name_ci, NULL);
}

static DWORD find_child_pid(DWORD parent_pid, const char *image_name_ci) {
    DWORD pid = 0;
    if (find_descendant(parent_pid, image_name_ci, &pid) == 1) {
        return pid;
    }
    return 0;
}

static int wait_child_state(DWORD parent_pid, const char *image_name_ci, int want_present, DWORD timeout_ms) {
    DWORD start = GetTickCount();
    for (;;) {
        int r = child_present(parent_pid, image_name_ci);
        if (want_present ? (r == 1) : (r == 0)) {
            return 1;
        }
        if (GetTickCount() - start > timeout_ms) {
            return 0;
        }
        Sleep(50);
    }
}

static void type_ascii(HWND hwnd, const char *s) {
    for (; *s; s++) {
        PostMessageA(hwnd, WM_CHAR, (WPARAM)(unsigned char)*s, 0);
    }
}

int main(int argc, char **argv) {
    const char *exe_path;
    const char *root;
    char console_path[MAX_PATH];
    char cmdline[1024];
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    HWND hwnd;
    DWORD app_exit = 1;
    DWORD ping_pid;

    if (argc < 3) {
        fprintf(stderr, "usage: mixa_win32_smoke_harness <mixa_app_main.exe> <fixture root dir>\n");
        return 2;
    }
    exe_path = argv[1];
    root = argv[2];

    CreateDirectoryA(root, NULL);
    _snprintf(console_path, sizeof(console_path) - 1, "%s\\mixa-console.log", root);
    console_path[sizeof(console_path) - 1] = 0;

    _snprintf(cmdline, sizeof(cmdline) - 1, "\"%s\" \"%s\"", exe_path, root);
    cmdline[sizeof(cmdline) - 1] = 0;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    check(CreateProcessA(NULL, cmdline, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi) != 0,
          "spawned the real mixa_app_main.exe as a child process");
    if (pi.hProcess == NULL) {
        printf("win32 smoke selftest: %d checks, %d failures\n", g_checks, g_failures);
        return g_failures ? 1 : 0;
    }

    hwnd = wait_for_window(pi.dwProcessId, 10000);
    check(hwnd != 0, "the real MixaBackendWin32 window appeared for the spawned process (window creation)");
    if (hwnd == 0) {
        TerminateProcess(pi.hProcess, 1);
        WaitForSingleObject(pi.hProcess, 5000);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        printf("win32 smoke selftest: %d checks, %d failures\n", g_checks, g_failures);
        return 1;
    }
    check(IsWindow(hwnd) != 0, "the created window is a real, live top-level window (IsWindow)");
    /* ticket 20260913-015530: mixa_backend_win32.lm1's "win32" table (the
     * one mixa_app_main.lm1 gets from mixa_backend_default_table()) now
     * opens with SW_SHOW; only the explicit, test-only "win32-hidden"
     * table stays hidden. The real production entrypoint must therefore
     * show a real, visible window -- asserted here directly rather than
     * assumed. */
    check(IsWindowVisible(hwnd) != 0,
          "the real production window is actually visible (ticket 20260913-015530: mixa_app_main's \"win32\" table now opens with SW_SHOW; only the explicit test-only \"win32-hidden\" table stays hidden)");

    /* Scenario A: typed character input + Enter dispatch of a short real
     * command; its echoed command text AND its real output must both
     * land, in order, in the production console file. */
    type_ascii(hwnd, "echo win32smoke");
    PostMessageA(hwnd, WM_KEYDOWN, VK_RETURN, 0);
    check(wait_file_occurrences_at_least(console_path, "echo win32smoke", 1, 5000),
          "typed command echoed verbatim into the production console file");
    check(wait_file_occurrences_at_least(console_path, "win32smoke", 2, 8000),
          "real spawned command's own output landed in the production console file (echo line + output line)");

    /* Scenario B: F1 help responsiveness -- appends ClearShell's own
     * verbatim help text (mixa_help_text()) to the SAME console file. */
    PostMessageA(hwnd, WM_KEYDOWN, VK_F1, 0);
    check(wait_file_occurrences_at_least(console_path, "mtkravchenko@gmail.com", 1, 5000),
          "F1 help text appended to the production console file");

    /* Scenario C: UI/F1 responsiveness WHILE a real command is still
     * running, then completion/cwd-status recovery proven by a
     * follow-up command succeeding afterward. */
    type_ascii(hwnd, "ping -n 4 127.0.0.1");
    PostMessageA(hwnd, WM_KEYDOWN, VK_RETURN, 0);
    check(wait_child_state(pi.dwProcessId, "PING.EXE", 1, 5000),
          "a real child process (ping) is running under the app while a command executes");
    /* F1 while the child is alive: if the message pump were blocked on
     * the live child, this second help-text append would never land. */
    PostMessageA(hwnd, WM_KEYDOWN, VK_F1, 0);
    check(wait_file_occurrences_at_least(console_path, "mtkravchenko@gmail.com", 2, 5000),
          "F1 handled a second time WHILE a real command is still running (UI/F1 responsiveness)");
    check(wait_child_state(pi.dwProcessId, "PING.EXE", 0, 10000),
          "the running command actually completed and its child process exited");
    type_ascii(hwnd, "echo after-ping");
    PostMessageA(hwnd, WM_KEYDOWN, VK_RETURN, 0);
    check(wait_file_occurrences_at_least(console_path, "after-ping", 2, 5000),
          "a follow-up command after completion succeeded (completion marker hidden, cwd/status recovered)");

    /* Scenario D: orderly close with NO surviving child. Start a real,
     * deliberately long-running command, confirm it is alive, close the
     * window, and confirm both the app process AND its child are gone. */
    type_ascii(hwnd, "ping -n 30 127.0.0.1");
    PostMessageA(hwnd, WM_KEYDOWN, VK_RETURN, 0);
    check(wait_child_state(pi.dwProcessId, "PING.EXE", 1, 5000),
          "a real long-running child process is alive before close");
    ping_pid = find_child_pid(pi.dwProcessId, "PING.EXE");
    check(ping_pid != 0, "captured the long-running child's own PID before close");

    PostMessageA(hwnd, WM_CLOSE, 0, 0);
    check(WaitForSingleObject(pi.hProcess, 10000) == WAIT_OBJECT_0,
          "the app process exited promptly after WM_CLOSE (orderly close)");
    if (GetExitCodeProcess(pi.hProcess, &app_exit)) {
        check(app_exit == 0, "the app process exited with code 0");
    } else {
        check(0, "GetExitCodeProcess succeeded");
    }

    if (ping_pid != 0) {
        HANDLE hChild = OpenProcess(SYNCHRONIZE, FALSE, ping_pid);
        if (hChild == NULL) {
            check(1, "the long-running child no longer exists after close (kill-and-mean-it)");
        } else {
            DWORD wr = WaitForSingleObject(hChild, 5000);
            check(wr == WAIT_OBJECT_0, "the long-running child was actually terminated promptly after close (kill-and-mean-it)");
            CloseHandle(hChild);
        }
    }
    check(wait_child_state(pi.dwProcessId, "PING.EXE", 0, 5000),
          "no PING.EXE child of the (now-closed) app remains (no surviving child)");

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    printf("win32 smoke selftest: %d checks, %d failures\n", g_checks, g_failures);
    return g_failures ? 1 : 0;
}
