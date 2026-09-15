/* mixa_ingress_host_harness.c
 * App-only Win32 foreign-event ingress demo for LMX_MSG_HOST_INGRESS_V0.
 * Uses the pinned vendor tree only; does not rewrite mixa_backend_win32 poll.
 *
 * Lifetime proof: owner waits on the test-only posted event (signaled only
 * AFTER foreign clears its stack) before first drain/recv. Joins are bounded.
 */
#include "l2src/lmx_message.h"
#include "l2src/lmx_message_host.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define POSTED_WAIT_MS 2000u
#define JOIN_WAIT_MS   5000u

static LmxMsgRuntime *g_rt;
static LmxMsgAddr g_dest;
static HANDLE g_posted_event;
static volatile LONG g_foreign_posted;
static volatile LONG g_foreign_late_stopped;
static volatile LONG g_handler_owner_tid;
static volatile LONG g_handler_ran;
static unsigned char g_expected[16];
static size_t g_expected_n;

typedef struct ForeignArg {
    int post_ok;
    int late_stopped;
    int unexpected;
} ForeignArg;

static DWORD WINAPI foreign_poster(void *arg)
{
    ForeignArg *fa = (ForeignArg *)arg;
    LmxMsgEnv env;
    unsigned char stack_payload[16];
    int st;
    DWORD tid = GetCurrentThreadId();

    memset(stack_payload, 0xA5, sizeof(stack_payload));
    memcpy(stack_payload, "mixa-ingress", 12);
    stack_payload[12] = 0x11;
    stack_payload[13] = 0x22;
    stack_payload[14] = 0x33;
    stack_payload[15] = 0x44;

    memset(&env, 0, sizeof(env));
    env.id = 0;
    env.kind = LMX_MSG_KIND_BYTES;
    env.bytes = stack_payload;
    env.n = sizeof(stack_payload);

    printf("evidence foreign_tid=%lu calls lmx_msg_host_post only\n", (unsigned long)tid);
    st = lmx_msg_host_post(g_rt, g_dest, &env);
    /* Prove seam owned a copy: mutate/clear stack after post returns. */
    memset(stack_payload, 0x00, sizeof(stack_payload));
    env.bytes = 0;
    env.n = 0;
    printf("evidence foreign cleared stack payload after post st=%d\n", st);

    if (st == LMX_MSG_STAGED) {
        fa->post_ok = 1;
        InterlockedExchange(&g_foreign_posted, 1);
        /* Signal only AFTER clear — owner must wait on this before drain. */
        SetEvent(g_posted_event);
    } else {
        fa->unexpected = 1;
        printf("evidence foreign unexpected post st=%d\n", st);
        SetEvent(g_posted_event);
    }
    return 0;
}

static DWORD WINAPI foreign_late_poster(void *arg)
{
    ForeignArg *fa = (ForeignArg *)arg;
    LmxMsgEnv env;
    unsigned char b[4];
    int st;

    memset(b, 0x5A, sizeof(b));
    memset(&env, 0, sizeof(env));
    env.kind = LMX_MSG_KIND_BYTES;
    env.bytes = b;
    env.n = sizeof(b);
    st = lmx_msg_host_post(g_rt, g_dest, &env);
    memset(b, 0, sizeof(b));
    if (st == LMX_MSG_STOPPED) {
        fa->late_stopped = 1;
        InterlockedExchange(&g_foreign_late_stopped, 1);
        printf("evidence late_post_after_shutdown -> STOPPED\n");
    } else {
        fa->unexpected = 1;
        printf("evidence late_post unexpected st=%d\n", st);
    }
    return 0;
}

/* Test-only hang poster: never exits; used by --force-join-timeout injection. */
static DWORD WINAPI hang_poster(void *arg)
{
    (void)arg;
    for (;;) {
        Sleep(60000u);
    }
    return 0;
}

/* Owner-local fail_keep_runtime exit: no shared-handle close, no poster fields. */
static int exit_fail_keep_runtime(DWORD owner_tid, const char *reason)
{
    FILE *evf;

    fprintf(stderr, "quiescence incomplete - leaving runtime (no delete while poster reachable)\n");
    evf = fopen("build/mixa/logs/mixa_ingress_host_harness.fail_keep_runtime.txt", "w");
    if (evf != 0) {
        fprintf(evf, "fail_keep_runtime=1 owner_tid=%lu fail=1 reason=%s\n",
            (unsigned long)owner_tid, reason != 0 ? reason : "join");
        fprintf(evf, "owner_local_only=1 no_close_posted_event=1 no_poster_field_inspect=1 no_runtime_cleanup=1\n");
        fclose(evf);
    }
    printf("mixa_ingress_host_harness FAIL\n");
    return 1;
}

/* Harness-only deterministic timeout injection (not application logic). */
static int run_force_join_timeout(void)
{
    HANDLE hang;
    DWORD wr;
    DWORD owner_tid = GetCurrentThreadId();

    printf("mixa_ingress_host_harness force_join_timeout injection owner_tid=%lu\n",
        (unsigned long)owner_tid);
    hang = CreateThread(0, 0, hang_poster, 0, 0, 0);
    if (hang == 0) {
        fprintf(stderr, "force_join_timeout CreateThread failed\n");
        return 1;
    }
    wr = WaitForSingleObject(hang, 1u);
    if (wr == WAIT_OBJECT_0) {
        fprintf(stderr, "force_join_timeout: hang thread exited unexpectedly\n");
        CloseHandle(hang);
        return 1;
    }
    printf("evidence force_join_timeout join_wr=%lu (expected timeout)\n", (unsigned long)wr);
    return exit_fail_keep_runtime(owner_tid, "force_join_timeout");
}

/* Bounded join: on failure, leave runtime alive (poster may still reach it). */
static int join_checked(HANDLE *thp, const char *label)
{
    DWORD wr;
    if (thp == 0 || *thp == 0) {
        return 0;
    }
    wr = WaitForSingleObject(*thp, JOIN_WAIT_MS);
    if (wr != WAIT_OBJECT_0) {
        fprintf(stderr, "%s join failed wr=%lu — not deleting runtime while poster reachable\n",
            label, (unsigned long)wr);
        return 1;
    }
    CloseHandle(*thp);
    *thp = 0;
    printf("evidence %s join bounded ok\n", label);
    return 0;
}

int main(int argc, char **argv)
{
    int ai;
    for (ai = 1; ai < argc; ai++) {
        if (argv[ai] != 0 && strcmp(argv[ai], "--force-join-timeout") == 0) {
            return run_force_join_timeout();
        }
    }

    LmxMsgAddr parent = 0;
    LmxMsgAddr child = 0;
    LmxMsgEnv got;
    uchar init[1];
    int st;
    int fail = 0;
    int ticks;
    int got_ok = 0;
    int posted_ready = 0;
    ForeignArg fa;
    ForeignArg late_fa;
    HANDLE th = 0;
    HANDLE late_th = 0;
    DWORD owner_tid = GetCurrentThreadId();
    DWORD wr;
    FILE *ev;
    int infinite_refused;

    init[0] = 1;
    memset(&got, 0, sizeof(got));
    memset(&fa, 0, sizeof(fa));
    memset(&late_fa, 0, sizeof(late_fa));
    memcpy(g_expected, "mixa-ingress", 12);
    g_expected[12] = 0x11;
    g_expected[13] = 0x22;
    g_expected[14] = 0x33;
    g_expected[15] = 0x44;
    g_expected_n = 16;

    printf("mixa_ingress_host_harness start owner_tid=%lu\n", (unsigned long)owner_tid);

    g_rt = lmx_msg_runtime_new();
    if (g_rt == 0) {
        fprintf(stderr, "runtime_new failed\n");
        return 1;
    }
    if (!lmx_msg_host_is_owner(g_rt)) {
        fprintf(stderr, "owner affinity missing after runtime_new\n");
        return 1;
    }

    st = lmx_msg_create(g_rt, 0, init, 1, &parent);
    if (st != LMX_MSG_OK) {
        fprintf(stderr, "create parent %d\n", st);
        return 1;
    }
    st = lmx_msg_create(g_rt, parent, init, 1, &child);
    if (st != LMX_MSG_OK) {
        fprintf(stderr, "create child %d\n", st);
        return 1;
    }
    st = lmx_msg_end_turn(g_rt, parent, 1);
    if (st != LMX_MSG_OK) {
        fprintf(stderr, "end_turn parent %d\n", st);
        return 1;
    }
    g_dest = child;
    printf("evidence dest=%u created on owner\n", (unsigned)g_dest);

    /* timeout_ms==0xFFFFFFFF must be refused (not INFINITE). */
    infinite_refused = lmx_msg_host_wait(g_rt, 0xFFFFFFFFu);
    if (infinite_refused != LMX_MSG_INVALID) {
        fprintf(stderr, "INFINITE wait not refused st=%d\n", infinite_refused);
        fail = 1;
    } else {
        printf("evidence host_wait(0xFFFFFFFF) -> INVALID (refused)\n");
    }

    g_posted_event = CreateEventA(0, 1, 0, 0);
    if (g_posted_event == 0) {
        fprintf(stderr, "CreateEvent failed\n");
        return 1;
    }

    th = CreateThread(0, 0, foreign_poster, &fa, 0, 0);
    if (th == 0) {
        fprintf(stderr, "CreateThread failed\n");
        return 1;
    }

    /* Coordination in the native fixture only: wait for signal AFTER clear. */
    wr = WaitForSingleObject(g_posted_event, POSTED_WAIT_MS);
    if (wr != WAIT_OBJECT_0) {
        fprintf(stderr, "posted_event wait failed wr=%lu (need signal after stack clear)\n",
            (unsigned long)wr);
        fail = 1;
    } else {
        posted_ready = 1;
        printf("evidence owner waited posted_event AFTER foreign stack clear (before first drain)\n");
    }

    /* Bounded owner loop: wait + drain + recv. Never INFINITE API timeout.
     * First drain only after posted_event — bytes observed only post-clear. */
    if (posted_ready) {
        for (ticks = 0; ticks < 200; ticks++) {
            st = lmx_msg_host_wait(g_rt, 5);
            if (st != LMX_MSG_OK && st != LMX_MSG_EMPTY) {
                fprintf(stderr, "host_wait bad %d\n", st);
                fail = 1;
                break;
            }
            st = lmx_msg_host_drain(g_rt);
            if (st != LMX_MSG_OK) {
                fprintf(stderr, "host_drain %d\n", st);
                fail = 1;
                break;
            }
            while (lmx_msg_recv(g_rt, child, &got) == LMX_MSG_OK) {
                InterlockedExchange(&g_handler_ran, 1);
                InterlockedExchange(&g_handler_owner_tid, (LONG)GetCurrentThreadId());
                if (got.kind != LMX_MSG_KIND_BYTES || got.n != g_expected_n || got.bytes == 0) {
                    fprintf(stderr, "bad envelope kind=%d n=%u\n", got.kind, (unsigned)got.n);
                    fail = 1;
                } else if (memcmp(got.bytes, g_expected, g_expected_n) != 0) {
                    fprintf(stderr, "bytes mismatch (copy failed?)\n");
                    fail = 1;
                } else {
                    got_ok = 1;
                    printf("evidence owner_handler tid=%lu bytes_match=1 (observed only after clear signal)\n",
                        (unsigned long)GetCurrentThreadId());
                }
                lmx_msg_env_release(&got);
            }
            if (got_ok) {
                break;
            }
        }
    }

    if (!got_ok) {
        fprintf(stderr, "owner did not receive copied bytes\n");
        fail = 1;
    }
    if (got_ok && (DWORD)g_handler_owner_tid != owner_tid) {
        fprintf(stderr, "handler not on owner tid got=%lu want=%lu\n",
            (unsigned long)g_handler_owner_tid, (unsigned long)owner_tid);
        fail = 1;
    } else if (got_ok) {
        printf("evidence handler_only_on_owner=1\n");
    }

    /* Quiescence: shutdown -> join ALL posters (bounded) -> then delete. */
    printf("evidence quiescence: shutdown then bounded join posters then delete\n");
    st = lmx_msg_runtime_shutdown(g_rt);
    if (st != LMX_MSG_OK) {
        fprintf(stderr, "runtime_shutdown %d\n", st);
        fail = 1;
    }

    if (join_checked(&th, "foreign_poster") != 0) {
        fail = 1;
        return exit_fail_keep_runtime(owner_tid, "foreign_poster_join");
    }

    late_th = CreateThread(0, 0, foreign_late_poster, &late_fa, 0, 0);
    if (late_th == 0) {
        fprintf(stderr, "late CreateThread failed\n");
        fail = 1;
        return exit_fail_keep_runtime(owner_tid, "late_CreateThread");
    }
    if (join_checked(&late_th, "foreign_late_poster") != 0) {
        fail = 1;
        return exit_fail_keep_runtime(owner_tid, "foreign_late_poster_join");
    }
    if (!late_fa.late_stopped) {
        fprintf(stderr, "late post did not return STOPPED\n");
        fail = 1;
    }

    /* Drain any leftovers on owner after joins. */
    lmx_msg_host_drain(g_rt);
    lmx_msg_runtime_delete(g_rt);
    g_rt = 0;

    /* write_evidence: only after quiescence completed (joins succeeded). */
    if (g_posted_event != 0) {
        CloseHandle(g_posted_event);
        g_posted_event = 0;
    }

    ev = fopen("build/mixa/logs/mixa_ingress_host_harness.evidence.txt", "w");
    if (ev != 0) {
        fprintf(ev, "foreign_post_ok=%d bytes_match=%d handler_owner=%d late_stopped=%d fail=%d\n",
            fa.post_ok, got_ok,
            (got_ok && ((DWORD)g_handler_owner_tid == owner_tid)) ? 1 : 0,
            late_fa.late_stopped, fail);
        fprintf(ev, "owner_tid=%lu handler_tid=%lu foreign_posted=%ld posted_ready=%d\n",
            (unsigned long)owner_tid,
            (unsigned long)g_handler_owner_tid,
            (long)g_foreign_posted,
            posted_ready);
        fprintf(ev, "coord=posted_event_after_clear joins=bounded_checked\n");
        fclose(ev);
    }

    if (fa.unexpected || late_fa.unexpected) {
        fail = 1;
    }
    if (fail) {
        printf("mixa_ingress_host_harness FAIL\n");
        return 1;
    }
    printf("mixa_ingress_host_harness ok\n");
    return 0;
}
