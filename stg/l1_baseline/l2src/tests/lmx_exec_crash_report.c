/* Crash report for the executor selftest builds of run_port_message.ps1.
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
 * Compiled with -DLMX_EXEC_CRASH_REPORT_MAIN it also provides main() for the
 * reference build, whose selftest is compiled with -Dmain=exec_selftest_main. */
#include <stdio.h>

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

void lmx_exec_crash_report_install(void) {
    SetUnhandledExceptionFilter(lmx_exec_crash_filter);
}
#else
void lmx_exec_crash_report_install(void) {
}
#endif

#if defined(LMX_EXEC_CRASH_REPORT_MAIN)
int exec_selftest_main(int argc, char **argv);
int main(int argc, char **argv) {
    lmx_exec_crash_report_install();
    return exec_selftest_main(argc, argv);
}
#endif
