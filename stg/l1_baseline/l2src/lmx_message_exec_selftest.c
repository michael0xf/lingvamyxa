/* Overlapping Message turns. Win32. */
#include "l2src/lmx_message.h"
#include "l2src/lmx_message_exec.h"
#include <stdio.h>
#include <string.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

typedef struct TurnCtx {
    HANDLE unblock;
    HANDLE started;
    volatile LONG done;
    DWORD t0;
    DWORD t1;
    DWORD t2;
    unsigned recvd;
    unsigned ui_recvd;
    LmxMsgRuntime *rt;
} TurnCtx;

static int turn_slow(LmxMsgRuntime *rt, LmxMsgAddr who, void *ctx) {
    TurnCtx *c = (TurnCtx *)ctx;
    LmxMsgEnv got;
    memset(&got, 0, sizeof(got));
    c->t0 = GetTickCount();
    SetEvent(c->started);
    if (lmx_msg_recv(rt, who, &got) != LMX_MSG_OK || got.n != 1 || got.bytes == 0 || got.bytes[0] != 1) {
        lmx_msg_env_release(&got);
        return 1;
    }
    c->recvd += 1;
    lmx_msg_env_release(&got);
    {
        int k;
        LmxMsgAddr ch = 0;
        uchar b = 1;
        for (k = 0; k < 20; k++) {
            ch = 0;
            if (lmx_msg_create(rt, who, (unsigned)(40 + k), &b, 1, &ch) != LMX_MSG_OK) {
                return 1;
            }
        }
    }
    Sleep(80);
    c->t1 = GetTickCount();
    InterlockedIncrement(&c->done);
    return lmx_msg_end_turn(rt, who, 1);
}

static int turn_ui(LmxMsgRuntime *rt, LmxMsgAddr who, void *ctx) {
    TurnCtx *c = (TurnCtx *)ctx;
    LmxMsgEnv got;
    memset(&got, 0, sizeof(got));
    if (lmx_msg_recv(rt, who, &got) != LMX_MSG_OK || got.n != 1 || got.bytes == 0 || got.bytes[0] != 7) {
        lmx_msg_env_release(&got);
        return 1;
    }
    c->ui_recvd += 1;
    lmx_msg_env_release(&got);
    return lmx_msg_end_turn(rt, who, 1);
}

static int turn_fast(LmxMsgRuntime *rt, LmxMsgAddr who, void *ctx) {
    TurnCtx *c = (TurnCtx *)ctx;
    LmxMsgEnv got;
    memset(&got, 0, sizeof(got));
    c->t2 = GetTickCount();
    if (lmx_msg_recv(rt, who, &got) != LMX_MSG_OK || got.n != 1 || got.bytes == 0 || got.bytes[0] != 1) {
        lmx_msg_env_release(&got);
        return 1;
    }
    c->recvd += 1;
    lmx_msg_env_release(&got);
    InterlockedIncrement(&c->done);
    return lmx_msg_end_turn(rt, who, 1);
}

int main(void) {
    LmxMsgRuntime *rt;
    LmxMsgAddr parent = 0, w1 = 0, w2 = 0, ui = 0;
    LmxMsgEnv env;
    uchar init[1];
    TurnCtx slow;
    TurnCtx fast;
    DWORD tui;
    FILE *ev;

    init[0] = 1;
    memset(&env, 0, sizeof(env));
    memset(&slow, 0, sizeof(slow));
    memset(&fast, 0, sizeof(fast));
    slow.unblock = CreateEventA(0, 1, 0, 0);
    slow.started = CreateEventA(0, 1, 0, 0);
    fast.unblock = slow.unblock;
    fast.started = CreateEventA(0, 1, 0, 0);
    rt = lmx_msg_runtime_new();
    if (rt == 0 || slow.unblock == 0) {
        return 1;
    }
    slow.rt = rt;
    fast.rt = rt;
    if (lmx_msg_create(rt, 0, 1, init, 1, &parent) != LMX_MSG_OK) {
        return 1;
    }
    if (lmx_msg_create(rt, parent, 2, init, 1, &w1) != LMX_MSG_OK) {
        return 1;
    }
    if (lmx_msg_create(rt, parent, 3, init, 1, &w2) != LMX_MSG_OK) {
        return 1;
    }
    if (lmx_msg_create(rt, parent, 4, init, 1, &ui) != LMX_MSG_OK) {
        return 1;
    }
    if (lmx_msg_end_turn(rt, parent, 1) != LMX_MSG_OK) {
        return 1;
    }
    if (lmx_msg_exec_bind(rt, w1, turn_slow, &slow, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK) {
        return 1;
    }
    if (lmx_msg_exec_bind(rt, w2, turn_fast, &fast, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK) {
        return 1;
    }
    if (lmx_msg_exec_bind(rt, ui, turn_ui, &slow, LMX_MSG_AFFINITY_UI) != LMX_MSG_OK) {
        return 1;
    }
    env.kind = LMX_MSG_KIND_BYTES;
    env.n = 1;
    env.bytes = init;
    env.id = 0;
    lmx_msg_send(rt, parent, w1, &env);
    lmx_msg_send(rt, parent, w2, &env);
    lmx_msg_end_turn(rt, parent, 1);
    lmx_msg_pump(rt);
    if (lmx_msg_exec_start(rt, 2) != LMX_MSG_OK) {
        return 1;
    }
    if (WaitForSingleObject(slow.started, 2000) != WAIT_OBJECT_0) {
        fprintf(stderr, "slow turn did not start\n");
        return 1;
    }
    {
        LmxMsgEnv hin;
        uchar hb = 7;
        memset(&hin, 0, sizeof(hin));
        hin.kind = LMX_MSG_KIND_BYTES;
        hin.n = 1;
        hin.bytes = &hb;
        if (lmx_msg_host_post(rt, ui, &hin) != LMX_MSG_STAGED) {
            return 1;
        }
    }
    tui = GetTickCount();
    if (lmx_msg_exec_ui_step(rt) != LMX_MSG_OK) {
        fprintf(stderr, "ui_step failed while slow active\n");
        return 1;
    }
    tui = GetTickCount() - tui;
    if (slow.ui_recvd != 1) {
        fprintf(stderr, "UI handler did not recv host_post\n");
        return 1;
    }
    if (tui > 50) {
        fprintf(stderr, "ui_step blocked on slow worker ms=%lu\n", (unsigned long)tui);
        return 1;
    }
    while (slow.done == 0 || fast.done == 0) {
        Sleep(10);
    }
    lmx_msg_exec_stop(rt);
    ev = fopen("build/l1trans/logs/gen2/lmx_message_exec_selftest.evidence.txt", "w");
    if (ev) {
        fprintf(ev, "slow t0=%lu t1=%lu recvd=%u\n", (unsigned long)slow.t0, (unsigned long)slow.t1, slow.recvd);
        fprintf(ev, "fast t2=%lu recvd=%u\n", (unsigned long)fast.t2, fast.recvd);
        fprintf(ev, "ui_step_ms=%lu extra_created=20\n", (unsigned long)tui);
        fclose(ev);
    }
    printf("lmx_message_exec ok slow=%lu..%lu fast=%lu ui_ms=%lu recvd=%u/%u\n",
        (unsigned long)slow.t0, (unsigned long)slow.t1, (unsigned long)fast.t2,
        (unsigned long)tui, slow.recvd, fast.recvd);
    if (slow.t1 - slow.t0 < 50) {
        fprintf(stderr, "slow turn did not block\n");
        lmx_msg_runtime_delete(rt);
        return 1;
    }
    if (fast.t2 < slow.t0 || fast.t2 > slow.t1) {
        fprintf(stderr, "fast turn did not overlap slow wait\n");
        lmx_msg_runtime_delete(rt);
        return 1;
    }
    if (slow.recvd != 1 || fast.recvd != 1) {
        fprintf(stderr, "payload recv mismatch\n");
        lmx_msg_runtime_delete(rt);
        return 1;
    }
    lmx_msg_runtime_delete(rt);
    CloseHandle(slow.unblock);
    CloseHandle(slow.started);
    CloseHandle(fast.started);
    return 0;
}
