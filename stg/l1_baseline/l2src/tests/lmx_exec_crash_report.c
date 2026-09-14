/* Crash report and watchdog for the executor selftest builds of
 * run_port_message.ps1.
 *
 * An access violation in a selftest process otherwise leaves only the exit
 * code (-1073741819) and the last stderr line, which on 2026-09-14 was not
 * enough to place a residual crash (RUNTIME_L2_PORTS.txt, stage 3a-1 notes).
 * lmx_exec_crash_report_install() sets an unhandled-exception filter that
 * prints, to stderr, the thread id, the exception code, the faulting access
 * address, and the raw stack as RVAs of this module (the exception dispatch
 * runs on the faulting thread's stack, so the frames below the dispatcher
 * are the faulting ones). Resolve an RVA with
 *   addr2line -f -p -e <exe> 0x<image base + rva>
 * (objdump -p <exe> | grep ImageBase). Silent on a clean run, so the
 * reference/generated stdout comparison is unaffected.
 *
 * It also starts a watchdog thread: a wait that never returns inside a
 * runtime call (the lead's 3a-2 red-first, 2026-09-14: a retired worker kept
 * waiting and lmx_msg_exec_unbind's join never returned) cannot be bounded
 * by the selftest case that made the call, and the runner's own timeout is
 * 900 s with no line from the test. After LMX_EXEC_WATCHDOG_SECONDS (300 by
 * default; override with -DLMX_EXEC_WATCHDOG_SECONDS=N) the watchdog flushes
 * stderr, so the main thread's buffered last line comes out, prints
 * "FAIL watchdog ..." and exits the process with code 3. A clean selftest
 * takes seconds; under a loaded machine well under a minute.
 *
 * Compiled with -DLMX_EXEC_CRASH_REPORT_MAIN it also provides main() for the
 * reference build, whose selftest is compiled with -Dmain=exec_selftest_main. */
#include <stdio.h>
#include <stdlib.h>

#ifndef LMX_EXEC_WATCHDOG_SECONDS
#define LMX_EXEC_WATCHDOG_SECONDS 300
#endif

#if defined(_WIN32)
#include <windows.h>

static LONG WINAPI lmx_exec_crash_filter(EXCEPTION_POINTERS *ep) {
    void *frames[64];
    USHORT n;
    USHORT i;
    unsigned char *base = (unsigned char *)GetModuleHandleA(0);
    unsigned char *rip = (unsigned char *)ep->ContextRecord->Rip;
    void *access = 0;
    if (ep->ExceptionRecord->NumberParameters >= 2) {
        access = (void *)ep->ExceptionRecord->ExceptionInformation[1];
    }
    fprintf(stderr, "CRASH tid=%lu code=%08lx access=%p rip_rva=%llx\n",
        (unsigned long)GetCurrentThreadId(),
        (unsigned long)ep->ExceptionRecord->ExceptionCode,
        access,
        (unsigned long long)(rip - base));
    n = RtlCaptureStackBackTrace(0, 64, frames, 0);
    for (i = 0; i < n; i++) {
        unsigned char *f = (unsigned char *)frames[i];
        if (f >= base && f < base + 0x4000000) {
            fprintf(stderr, "  frame %u rva=%llx\n", (unsigned)i, (unsigned long long)(f - base));
        } else {
            fprintf(stderr, "  frame %u ext=%p\n", (unsigned)i, (void *)f);
        }
    }
    fflush(stderr);
    return EXCEPTION_EXECUTE_HANDLER;
}

static DWORD WINAPI lmx_exec_watchdog(void *arg) {
    (void)arg;
    Sleep((DWORD)LMX_EXEC_WATCHDOG_SECONDS * 1000U);
    fflush(stderr);
    fprintf(stderr, "\nFAIL watchdog: the selftest has run for %d s; a wait never returned (attach gdb to the live process to place it)\n",
        (int)LMX_EXEC_WATCHDOG_SECONDS);
    fflush(stderr);
    ExitProcess(3);
    return 0;
}

void lmx_exec_crash_report_install(void) {
    HANDLE th;
    SetUnhandledExceptionFilter(lmx_exec_crash_filter);
    th = CreateThread(0, 0, lmx_exec_watchdog, 0, 0, 0);
    if (th != 0) {
        CloseHandle(th);
    }
}
#else
#include <pthread.h>
#include <unistd.h>

static void *lmx_exec_watchdog(void *arg) {
    (void)arg;
    sleep((unsigned)LMX_EXEC_WATCHDOG_SECONDS);
    fflush(stderr);
    fprintf(stderr, "\nFAIL watchdog: the selftest has run for %d s; a wait never returned\n", (int)LMX_EXEC_WATCHDOG_SECONDS);
    fflush(stderr);
    _exit(3);
    return 0;
}

void lmx_exec_crash_report_install(void) {
    pthread_t th;
    if (pthread_create(&th, 0, lmx_exec_watchdog, 0) == 0) {
        pthread_detach(th);
    }
}
#endif

#if defined(LMX_EXEC_CRASH_REPORT_MAIN)
int exec_selftest_main(int argc, char **argv);
int main(int argc, char **argv) {
    lmx_exec_crash_report_install();
    return exec_selftest_main(argc, argv);
}
#endif
