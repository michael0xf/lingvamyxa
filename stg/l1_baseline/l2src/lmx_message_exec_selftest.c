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
    unsigned fifo[2];
    unsigned ui_bytes[2];
    unsigned peer_got;
    LmxMsgAddr ui;
    LmxMsgAddr peer;
    int send_ui_st;
    int send_peer_st;
    LmxMsgAddr extras[8];
    volatile LONG extra_n;
    LmxMsgRuntime *rt;
} TurnCtx;

static volatile LONG g_cpu_stop;

typedef struct MassRec {
    uchar expect;
    LONG in_turn;
    LONG done;
    int recv_st;
    int end_st;
    unsigned got;
    unsigned fifo_a;
    unsigned fifo_b;
    int overlap;
} MassRec;

static MassRec g_mass[70];
static int g_live_cascade;
static int g_factory_n;
static int g_factory_n_at_meta;
static int g_oom_n;
static int g_oom_hits;
static int g_oom_scan;
static unsigned g_oom_fifo_a;
static unsigned g_oom_fifo_b;
static HANDLE g_cleanup_seen;
static HANDLE g_cleanup_go;
static volatile LONG g_cleanup_hits;
static volatile LONG g_cleanup_gate_fail;
static int g_cleanup_live;

static void after_cleanup_gate(LmxMsgAddr who, int live, int st) {
    LONG n;
    DWORD gw;
    (void)who;
    (void)st;
    n = InterlockedIncrement(&g_cleanup_hits);
    if (n == 1) {
        g_cleanup_live = live;
        SetEvent(g_cleanup_seen);
        gw = WAIT_FAILED;
        if (g_cleanup_go != 0) {
            gw = WaitForSingleObject(g_cleanup_go, 5000);
        }
        if (gw != WAIT_OBJECT_0) {
            InterlockedExchange(&g_cleanup_gate_fail, 1);
            fprintf(stderr, "cleanup go watchdog wr=%lu live=%d\n",
                (unsigned long)gw, live);
            fflush(stderr);
        }
    }
}

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
    if (lmx_msg_recv(rt, who, &got) != LMX_MSG_OK || got.n != 1 || got.bytes == 0) {
        lmx_msg_env_release(&got);
        return 1;
    }
    if (c->ui_recvd < 2) {
        c->ui_bytes[c->ui_recvd] = got.bytes[0];
    }
    c->ui_recvd += 1;
    lmx_msg_env_release(&got);
    return lmx_msg_end_turn(rt, who, 1);
}

static int turn_held_live(LmxMsgRuntime *rt, LmxMsgAddr who, void *ctx) {
    TurnCtx *c = (TurnCtx *)ctx;
    LmxMsgEnv got;
    LmxMsgEnv init;
    unsigned seg = 0;
    memset(&got, 0, sizeof(got));
    memset(&init, 0, sizeof(init));
    if (lmx_msg_recv(rt, who, &got) != LMX_MSG_OK) {
        lmx_msg_env_release(&got);
        return 1;
    }
    lmx_msg_env_release(&got);
    SetEvent(c->started);
    if (lmx_msg_path_n(rt, who) < 1 || lmx_msg_path_seg(rt, who, 0, &seg) != LMX_MSG_OK || lmx_msg_init_copy(rt, who, &init) != LMX_MSG_OK) {
        lmx_msg_env_release(&init);
        return 1;
    }
    lmx_msg_env_release(&init);
    c->t0 = GetTickCount();
    while (InterlockedCompareExchange(&g_cpu_stop, 0, 0) == 0) {
    }
    InterlockedIncrement(&c->done);
    return lmx_msg_end_turn(rt, who, 1);
}

static int turn_factory(LmxMsgRuntime *rt, LmxMsgAddr who, void *ctx) {
    TurnCtx *c = (TurnCtx *)ctx;
    LmxMsgEnv got;
    uchar b = 1;
    int k;
    LmxMsgAddr extra = 0;
    memset(&got, 0, sizeof(got));
    if (lmx_msg_recv(rt, who, &got) != LMX_MSG_OK) {
        lmx_msg_env_release(&got);
        return 1;
    }
    lmx_msg_env_release(&got);
    for (k = 0; k < 8; k++) {
        extra = 0;
        if (lmx_msg_create(rt, who, (unsigned)(200 + k), &b, 1, &extra) != LMX_MSG_OK || extra == 0) {
            return 1;
        }
        c->extras[k] = extra;
        InterlockedExchange(&c->extra_n, k + 1);
        if (k == 0) {
            SetEvent(c->started);
            if (c->unblock == 0 || WaitForSingleObject(c->unblock, 5000) != WAIT_OBJECT_0) {
                return 1;
            }
        }
    }
    InterlockedIncrement(&c->done);
    return lmx_msg_end_turn(rt, who, 1);
}

static int turn_just_end(LmxMsgRuntime *rt, LmxMsgAddr who, void *ctx) {
    TurnCtx *c = (TurnCtx *)ctx;
    InterlockedIncrement(&c->done);
    return lmx_msg_end_turn(rt, who, 1);
}

static int turn_busy(LmxMsgRuntime *rt, LmxMsgAddr who, void *ctx) {
    TurnCtx *c = (TurnCtx *)ctx;
    LmxMsgEnv got;
    DWORD t0;
    memset(&got, 0, sizeof(got));
    c->t0 = GetTickCount();
    SetEvent(c->started);
    if (lmx_msg_recv(rt, who, &got) != LMX_MSG_OK) {
        lmx_msg_env_release(&got);
        return 1;
    }
    lmx_msg_env_release(&got);
    t0 = c->t0;
    (void)t0;
    while (InterlockedCompareExchange(&g_cpu_stop, 0, 0) == 0) {
    }
    c->t1 = GetTickCount();
    InterlockedIncrement(&c->done);
    return lmx_msg_end_turn(rt, who, 1);
}

static int turn_fast(LmxMsgRuntime *rt, LmxMsgAddr who, void *ctx) {
    TurnCtx *c = (TurnCtx *)ctx;
    LmxMsgEnv got;
    memset(&got, 0, sizeof(got));
    c->t2 = GetTickCount();
    if (lmx_msg_recv(rt, who, &got) != LMX_MSG_OK || got.n != 1 || got.bytes == 0) {
        lmx_msg_env_release(&got);
        return 1;
    }
    if (c->recvd < 2) {
        c->fifo[c->recvd] = got.bytes[0];
    }
    c->recvd += 1;
    if (c->recvd == 1 && c->ui != 0) {
        LmxMsgEnv to_ui;
        uchar b = 8;
        memset(&to_ui, 0, sizeof(to_ui));
        to_ui.kind = LMX_MSG_KIND_BYTES;
        to_ui.n = 1;
        to_ui.bytes = &b;
        c->send_ui_st = lmx_msg_send(rt, who, c->ui, &to_ui);
        if (c->send_ui_st != LMX_MSG_STAGED) {
            lmx_msg_env_release(&got);
            return 1;
        }
        if (c->peer != 0) {
            uchar p = 42;
            to_ui.bytes = &p;
            c->send_peer_st = lmx_msg_send(rt, who, c->peer, &to_ui);
            if (c->send_peer_st != LMX_MSG_STAGED) {
                lmx_msg_env_release(&got);
                return 1;
            }
        }
    }
    lmx_msg_env_release(&got);
    InterlockedIncrement(&c->done);
    return lmx_msg_end_turn(rt, who, 1);
}

static int turn_ok_no_end(LmxMsgRuntime *rt, LmxMsgAddr who, void *ctx) {
    MassRec *m = (MassRec *)ctx;
    LmxMsgEnv got;
    memset(&got, 0, sizeof(got));
    m->recv_st = lmx_msg_recv(rt, who, &got);
    lmx_msg_env_release(&got);
    InterlockedIncrement(&m->done);
    return 0;
}

static int turn_err_after_end(LmxMsgRuntime *rt, LmxMsgAddr who, void *ctx) {
    MassRec *m = (MassRec *)ctx;
    LmxMsgEnv got;
    memset(&got, 0, sizeof(got));
    m->recv_st = lmx_msg_recv(rt, who, &got);
    lmx_msg_env_release(&got);
    m->end_st = lmx_msg_end_turn(rt, who, 1);
    InterlockedIncrement(&m->done);
    return 1;
}

static int turn_fail_no_end(LmxMsgRuntime *rt, LmxMsgAddr who, void *ctx) {
    MassRec *m = (MassRec *)ctx;
    LmxMsgEnv got;
    memset(&got, 0, sizeof(got));
    if (InterlockedIncrement(&m->in_turn) != 1) {
        m->overlap = 1;
        return 1;
    }
    m->recv_st = lmx_msg_recv(rt, who, &got);
    if (m->recv_st == LMX_MSG_OK && got.n == 1 && got.bytes != 0) {
        if (InterlockedCompareExchange(&m->done, 0, 0) == 0) {
            m->fifo_a = got.bytes[0];
        } else {
            m->fifo_b = got.bytes[0];
        }
    }
    lmx_msg_env_release(&got);
    InterlockedDecrement(&m->in_turn);
    InterlockedIncrement(&m->done);
    return 1;
}

static int turn_two_ok(LmxMsgRuntime *rt, LmxMsgAddr who, void *ctx) {
    MassRec *m = (MassRec *)ctx;
    LmxMsgEnv got;
    LONG n;
    memset(&got, 0, sizeof(got));
    if (InterlockedIncrement(&m->in_turn) != 1) {
        m->overlap = 1;
        return 1;
    }
    m->recv_st = lmx_msg_recv(rt, who, &got);
    if (m->recv_st != LMX_MSG_OK || got.n != 1 || got.bytes == 0) {
        lmx_msg_env_release(&got);
        InterlockedDecrement(&m->in_turn);
        return 1;
    }
    n = InterlockedCompareExchange(&m->done, 0, 0);
    if (n == 0) {
        m->fifo_a = got.bytes[0];
    } else {
        m->fifo_b = got.bytes[0];
    }
    lmx_msg_env_release(&got);
    m->end_st = lmx_msg_end_turn(rt, who, 1);
    InterlockedDecrement(&m->in_turn);
    InterlockedIncrement(&m->done);
    return m->end_st == LMX_MSG_OK ? 0 : 1;
}

static int turn_mass(LmxMsgRuntime *rt, LmxMsgAddr who, void *ctx) {
    MassRec *m = (MassRec *)ctx;
    LmxMsgEnv got;
    memset(&got, 0, sizeof(got));
    if (InterlockedIncrement(&m->in_turn) != 1) {
        m->recv_st = 1;
        return 1;
    }
    m->recv_st = lmx_msg_recv(rt, who, &got);
    if (m->recv_st != LMX_MSG_OK || got.n != 1 || got.bytes == 0 || got.bytes[0] != m->expect) {
        lmx_msg_env_release(&got);
        InterlockedDecrement(&m->in_turn);
        return 1;
    }
    m->got = got.bytes[0];
    lmx_msg_env_release(&got);
    m->end_st = lmx_msg_end_turn(rt, who, 1);
    InterlockedDecrement(&m->in_turn);
    InterlockedIncrement(&m->done);
    return m->end_st == LMX_MSG_OK ? 0 : 1;
}

int main(void) {
    LmxMsgRuntime *rt;
    LmxMsgAddr parent = 0, w1 = 0, w2 = 0, ui = 0, w3 = 0;
    LmxMsgEnv env;
    uchar init[1];
    TurnCtx slow;
    TurnCtx fast;
    static MassRec peerrec;
    DWORD tui;
    DWORD tbusy_ui = 0;
    FILE *ev;

    fprintf(stderr, "boot\n");
    fflush(stderr);
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
        fprintf(stderr, "rt/event\n");
        return 1;
    }
    slow.rt = rt;
    fast.rt = rt;
    if (lmx_msg_create(rt, 0, 1, init, 1, &parent) != LMX_MSG_OK) {
        fprintf(stderr, "create parent\n");
        return 1;
    }
    if (lmx_msg_create(rt, parent, 2, init, 1, &w1) != LMX_MSG_OK) {
        fprintf(stderr, "create w1\n");
        return 1;
    }
    if (lmx_msg_create(rt, parent, 3, init, 1, &w2) != LMX_MSG_OK) {
        fprintf(stderr, "create w2\n");
        return 1;
    }
    if (lmx_msg_create(rt, parent, 4, init, 1, &ui) != LMX_MSG_OK) {
        fprintf(stderr, "create ui\n");
        return 1;
    }
    if (lmx_msg_create(rt, parent, 5, init, 1, &w3) != LMX_MSG_OK) {
        fprintf(stderr, "create w3\n");
        return 1;
    }
    fprintf(stderr, "calling end_turn parent=%u\n", parent);
    {
        int et = lmx_msg_end_turn(rt, parent, 1);
        fprintf(stderr, "end_turn returned %d\n", et);
        if (et != LMX_MSG_OK) {
            fprintf(stderr, "end_turn parent st=%d\n", et);
            return 1;
        }
    }
    if (lmx_msg_exec_bind(rt, w1, turn_slow, &slow, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK) {
        fprintf(stderr, "bind w1\n");
        return 1;
    }
    fprintf(stderr, "bind w1 ok\n");
    fflush(stderr);
    if (lmx_msg_exec_bind(rt, w2, turn_fast, &fast, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK) {
        fprintf(stderr, "bind w2\n");
        return 1;
    }
    if (lmx_msg_exec_bind(rt, ui, turn_ui, &slow, LMX_MSG_AFFINITY_UI) != LMX_MSG_OK) {
        fprintf(stderr, "bind ui\n");
        return 1;
    }
    memset(&peerrec, 0, sizeof(peerrec));
    peerrec.expect = 42;
    if (lmx_msg_exec_bind(rt, w3, turn_mass, &peerrec, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK) {
        fprintf(stderr, "bind w3\n");
        return 1;
    }
    fprintf(stderr, "binds ok\n");
    fflush(stderr);
    fast.ui = ui;
    fast.peer = w3;
    fast.send_ui_st = -1;
    fast.send_peer_st = -1;
    env.kind = LMX_MSG_KIND_BYTES;
    env.n = 1;
    env.bytes = init;
    env.id = 0;
    lmx_msg_send(rt, parent, w1, &env);
    {
        uchar a = 10, b = 20;
        env.bytes = &a;
        lmx_msg_send(rt, parent, w2, &env);
        env.bytes = &b;
        lmx_msg_send(rt, parent, w2, &env);
        env.bytes = init;
    }
    lmx_msg_end_turn(rt, parent, 1);
    lmx_msg_pump(rt);
    {
        int k;
        LmxMsgAddr extra = 0;
        uchar payload[70];
        memset(g_mass, 0, sizeof(g_mass));
        for (k = 0; k < 70; k++) {
            extra = 0;
            payload[k] = (uchar)(k + 1);
            g_mass[k].expect = payload[k];
            if (lmx_msg_create(rt, parent, (unsigned)(100 + k), init, 1, &extra) != LMX_MSG_OK) {
                fprintf(stderr, "mass create %d\n", k);
                return 1;
            }
            if (lmx_msg_exec_bind(rt, extra, turn_mass, &g_mass[k], LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK) {
                fprintf(stderr, "mass bind %d\n", k);
                return 1;
            }
            env.bytes = &payload[k];
            if (lmx_msg_send(rt, parent, extra, &env) != LMX_MSG_STAGED) {
                fprintf(stderr, "mass send %d\n", k);
                return 1;
            }
        }
        lmx_msg_end_turn(rt, parent, 1);
        lmx_msg_pump(rt);
    }
    fprintf(stderr, "mass staged\n");
    fflush(stderr);
    {
        LmxMsgEnv hin;
        uchar hb = 7;
        memset(&hin, 0, sizeof(hin));
        hin.kind = LMX_MSG_KIND_BYTES;
        hin.n = 1;
        hin.bytes = &hb;
        if (lmx_msg_host_post(rt, ui, &hin) != LMX_MSG_STAGED) {
            fprintf(stderr, "host_post\n");
            return 1;
        }
    }
    fprintf(stderr, "starting workers nready=%d cap=%d\n", lmx_msg_exec_nready(rt), lmx_msg_exec_ready_cap(rt));
    fflush(stderr);
    if (lmx_msg_exec_start(rt, 2) != LMX_MSG_OK) {
        fprintf(stderr, "exec_start\n");
        return 1;
    }
    fprintf(stderr, "workers started, waiting slow\n");
    fflush(stderr);
    fprintf(stderr, "waiting slow\n");
    fflush(stderr);
    if (WaitForSingleObject(slow.started, 5000) != WAIT_OBJECT_0) {
        fprintf(stderr, "slow turn did not start nready=%d\n", lmx_msg_exec_nready(rt));
        return 1;
    }
    fprintf(stderr, "slow started\n");
    fflush(stderr);
    fprintf(stderr, "ui_step...\n");
    fflush(stderr);
    tui = GetTickCount();
    {
        int us = lmx_msg_exec_ui_step(rt);
        fprintf(stderr, "ui_step st=%d recvd=%u\n", us, slow.ui_recvd);
        fflush(stderr);
        if (us != LMX_MSG_OK) {
            fprintf(stderr, "ui_step failed while slow active\n");
            return 1;
        }
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
    {
        DWORD deadline = GetTickCount() + 3000;
        while ((InterlockedCompareExchange(&slow.done, 0, 0) == 0 || InterlockedCompareExchange(&fast.done, 0, 0) < 2) && GetTickCount() < deadline) {
            Sleep(10);
        }
        {
            int k;
            int mass_done = 0;
            for (k = 0; k < 70; k++) {
                if (InterlockedCompareExchange(&g_mass[k].done, 0, 0) != 0) {
                    mass_done += 1;
                }
            }
            while ((mass_done < 70 || InterlockedCompareExchange(&peerrec.done, 0, 0) != 1) && GetTickCount() < deadline) {
                Sleep(10);
                mass_done = 0;
                for (k = 0; k < 70; k++) {
                    if (InterlockedCompareExchange(&g_mass[k].done, 0, 0) != 0) {
                        mass_done += 1;
                    }
                }
            }
            if (InterlockedCompareExchange(&slow.done, 0, 0) == 0 || InterlockedCompareExchange(&fast.done, 0, 0) < 2 || mass_done < 70 || InterlockedCompareExchange(&peerrec.done, 0, 0) != 1) {
                fprintf(stderr, "timeout slow=%ld fast=%ld mass_done=%d peer=%ld\n",
                    (long)InterlockedCompareExchange(&slow.done, 0, 0),
                    (long)InterlockedCompareExchange(&fast.done, 0, 0), mass_done,
                    (long)InterlockedCompareExchange(&peerrec.done, 0, 0));
                return 1;
            }
        }
    }
    fprintf(stderr, "mass wait ok\n");
    fflush(stderr);
    {
        DWORD udl = GetTickCount() + 2000;
        while (slow.ui_recvd < 2 && GetTickCount() < udl) {
            lmx_msg_exec_ui_step(rt);
            Sleep(5);
        }
        if (slow.ui_recvd < 2) {
            fprintf(stderr, "worker-to-UI missing ui_recvd=%u\n", slow.ui_recvd);
            return 1;
        }
    }
    fprintf(stderr, "ui drain recvd=%u\n", slow.ui_recvd);
    fflush(stderr);
    lmx_msg_exec_stop(rt);
    fprintf(stderr, "exec_stop ok\n");
    fflush(stderr);
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
    if (slow.recvd != 1 || fast.recvd != 2 || fast.fifo[0] != 10 || fast.fifo[1] != 20) {
        fprintf(stderr, "FIFO mismatch slow=%u fast=%u %u,%u\n",
            slow.recvd, fast.recvd, fast.fifo[0], fast.fifo[1]);
        lmx_msg_runtime_delete(rt);
        return 1;
    }
    if (fast.send_ui_st != LMX_MSG_STAGED || fast.send_peer_st != LMX_MSG_STAGED || peerrec.got != 42 || peerrec.recv_st != LMX_MSG_OK || peerrec.end_st != LMX_MSG_OK) {
        fprintf(stderr, "cross-send ui_st=%d peer_st=%d got=%u recv=%d end=%d\n",
            fast.send_ui_st, fast.send_peer_st, peerrec.got, peerrec.recv_st, peerrec.end_st);
        lmx_msg_runtime_delete(rt);
        return 1;
    }
    {
        int has7 = (slow.ui_bytes[0] == 7 || slow.ui_bytes[1] == 7);
        int has8 = (slow.ui_bytes[0] == 8 || slow.ui_bytes[1] == 8);
        if (slow.ui_recvd < 2 || has7 == 0 || has8 == 0) {
            fprintf(stderr, "UI bytes missing recvd=%u %u,%u\n", slow.ui_recvd, slow.ui_bytes[0], slow.ui_bytes[1]);
            lmx_msg_runtime_delete(rt);
            return 1;
        }
    }
    fprintf(stderr, "first scenario asserts ok\n");
    fflush(stderr);
    {
        int k;
        for (k = 0; k < 70; k++) {
            if (InterlockedCompareExchange(&g_mass[k].done, 0, 0) != 1 || g_mass[k].recv_st != LMX_MSG_OK || g_mass[k].end_st != LMX_MSG_OK || g_mass[k].got != g_mass[k].expect || InterlockedCompareExchange(&g_mass[k].in_turn, 0, 0) != 0) {
                fprintf(stderr, "mass[%d] done=%ld recv=%d end=%d got=%u expect=%u in_turn=%ld\n",
                    k, (long)g_mass[k].done, g_mass[k].recv_st, g_mass[k].end_st,
                    g_mass[k].got, g_mass[k].expect, (long)g_mass[k].in_turn);
                lmx_msg_runtime_delete(rt);
                return 1;
            }
        }
    }
    fprintf(stderr, "mass 70 ok\n");
    fflush(stderr);
    lmx_msg_runtime_delete(rt);
    CloseHandle(slow.unblock);
    CloseHandle(slow.started);
    CloseHandle(fast.started);
    fprintf(stderr, "first rt deleted\n");
    fflush(stderr);

    /* Failing handler + queued second item: latch held through cleanup. */
    {
        LmxMsgRuntime *rtf;
        LmxMsgAddr pf = 0, wf = 0;
        LmxMsgEnv e2;
        uchar p31 = 31, p32 = 32, ini = 1;
        DWORD dl;
        static MassRec failrec;
        memset(&failrec, 0, sizeof(failrec));
        failrec.expect = 31;
        rtf = lmx_msg_runtime_new();
        if (rtf == 0) {
            fprintf(stderr, "fail rt\n");
            return 1;
        }
        fprintf(stderr, "fail rt ok\n");
        fflush(stderr);
        if (lmx_msg_create(rtf, 0, 1, &ini, 1, &pf) != LMX_MSG_OK) {
            fprintf(stderr, "fail create pf\n");
            return 1;
        }
        if (lmx_msg_create(rtf, pf, 2, &ini, 1, &wf) != LMX_MSG_OK) {
            fprintf(stderr, "fail create wf\n");
            return 1;
        }
        if (lmx_msg_end_turn(rtf, pf, 1) != LMX_MSG_OK) {
            fprintf(stderr, "fail end_turn\n");
            return 1;
        }
        memset(&e2, 0, sizeof(e2));
        e2.kind = LMX_MSG_KIND_BYTES;
        e2.n = 1;
        e2.bytes = &p31;
        lmx_msg_send(rtf, pf, wf, &e2);
        e2.bytes = &p32;
        lmx_msg_send(rtf, pf, wf, &e2);
        lmx_msg_end_turn(rtf, pf, 1);
        lmx_msg_pump(rtf);
        if (lmx_msg_exec_bind(rtf, wf, turn_fail_no_end, &failrec, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK) {
            fprintf(stderr, "fail bind\n");
            return 1;
        }
        g_cleanup_seen = CreateEventA(0, 1, 0, 0);
        g_cleanup_go = CreateEventA(0, 1, 0, 0);
        g_cleanup_hits = 0;
        g_cleanup_gate_fail = 0;
        lmx_msg_exec_test_after_cleanup = after_cleanup_gate;
        fprintf(stderr, "fail start\n");
        fflush(stderr);
        if (lmx_msg_exec_start(rtf, 2) != LMX_MSG_OK) {
            fprintf(stderr, "fail exec_start\n");
            return 1;
        }
        fprintf(stderr, "fail waiting cleanup seen=%p nready=%d\n",
            (void *)g_cleanup_seen, lmx_msg_exec_nready(rtf));
        fflush(stderr);
        {
            DWORD wr = WaitForSingleObject(g_cleanup_seen, 3000);
            fprintf(stderr, "cleanup wait wr=%lu hits=%ld\n",
                (unsigned long)wr, (long)InterlockedCompareExchange(&g_cleanup_hits, 0, 0));
            fflush(stderr);
            if (wr != WAIT_OBJECT_0) {
                fprintf(stderr, "cleanup gate not reached\n");
                fflush(stderr);
                lmx_msg_exec_test_after_cleanup = 0;
                SetEvent(g_cleanup_go);
                lmx_msg_exec_stop(rtf);
                lmx_msg_runtime_delete(rtf);
                return 1;
            }
        }
        fprintf(stderr, "cleanup gate ok done=%ld live=%d\n",
            (long)InterlockedCompareExchange(&failrec.done, 0, 0), g_cleanup_live);
        fflush(stderr);
        if (InterlockedCompareExchange(&failrec.done, 0, 0) != 1) {
            fprintf(stderr, "next turn started during cleanup done=%ld\n",
                (long)InterlockedCompareExchange(&failrec.done, 0, 0));
            fflush(stderr);
            SetEvent(g_cleanup_go);
            lmx_msg_exec_test_after_cleanup = 0;
            lmx_msg_exec_stop(rtf);
            lmx_msg_runtime_delete(rtf);
            return 1;
        }
        SetEvent(g_cleanup_go);
        fprintf(stderr, "cleanup go signaled\n");
        fflush(stderr);
        dl = GetTickCount() + 3000;
        while (InterlockedCompareExchange(&failrec.done, 0, 0) < 2 && GetTickCount() < dl) {
            Sleep(10);
        }
        fprintf(stderr, "fail-handler before stop done=%ld\n",
            (long)InterlockedCompareExchange(&failrec.done, 0, 0));
        fflush(stderr);
        lmx_msg_exec_stop(rtf);
        lmx_msg_exec_test_after_cleanup = 0;
        fprintf(stderr, "fail-handler after stop done=%ld fifo=%u,%u overlap=%d in_turn=%ld last=%d gate_fail=%ld\n",
            (long)InterlockedCompareExchange(&failrec.done, 0, 0), failrec.fifo_a, failrec.fifo_b,
            failrec.overlap, (long)InterlockedCompareExchange(&failrec.in_turn, 0, 0),
            lmx_msg_exec_last_status(rtf, wf),
            (long)InterlockedCompareExchange(&g_cleanup_gate_fail, 0, 0));
        fflush(stderr);
        if (InterlockedCompareExchange(&g_cleanup_gate_fail, 0, 0) != 0) {
            fprintf(stderr, "cleanup go watchdog escaped into pass\n");
            fflush(stderr);
            lmx_msg_runtime_delete(rtf);
            return 1;
        }
        if (InterlockedCompareExchange(&failrec.done, 0, 0) != 2 || failrec.fifo_a != 31 || failrec.fifo_b != 32 || InterlockedCompareExchange(&failrec.in_turn, 0, 0) != 0 || failrec.overlap != 0 || lmx_msg_exec_last_status(rtf, wf) != 1) {
            fprintf(stderr, "fail-handler race done=%ld got=%u,%u overlap=%d in_turn=%ld last=%d\n",
                (long)InterlockedCompareExchange(&failrec.done, 0, 0), failrec.fifo_a, failrec.fifo_b,
                failrec.overlap, (long)InterlockedCompareExchange(&failrec.in_turn, 0, 0),
                lmx_msg_exec_last_status(rtf, wf));
            fflush(stderr);
            lmx_msg_runtime_delete(rtf);
            return 1;
        }
        lmx_msg_runtime_delete(rtf);
        CloseHandle(g_cleanup_seen);
        CloseHandle(g_cleanup_go);
        fprintf(stderr, "fail-handler ok\n");
        fflush(stderr);
    }
    {
        LmxMsgRuntime *rte;
        LmxMsgAddr pe = 0, we = 0;
        LmxMsgEnv ee;
        uchar ini = 1, b = 9;
        static MassRec err_after;
        fprintf(stderr, "err-after start\n");
        fflush(stderr);
        memset(&err_after, 0, sizeof(err_after));
        rte = lmx_msg_runtime_new();
        lmx_msg_create(rte, 0, 1, &ini, 1, &pe);
        lmx_msg_create(rte, pe, 2, &ini, 1, &we);
        lmx_msg_end_turn(rte, pe, 1);
        memset(&ee, 0, sizeof(ee));
        ee.kind = LMX_MSG_KIND_BYTES;
        ee.n = 1;
        ee.bytes = &b;
        lmx_msg_send(rte, pe, we, &ee);
        lmx_msg_end_turn(rte, pe, 1);
        lmx_msg_pump(rte);
        lmx_msg_exec_bind(rte, we, turn_err_after_end, &err_after, LMX_MSG_AFFINITY_ANY);
        lmx_msg_exec_start(rte, 1);
        {
            DWORD dl = GetTickCount() + 2000;
            while (InterlockedCompareExchange(&err_after.done, 0, 0) == 0 && GetTickCount() < dl) {
                Sleep(10);
            }
        }
        lmx_msg_exec_stop(rte);
        if (InterlockedCompareExchange(&err_after.done, 0, 0) != 1 || lmx_msg_exec_last_status(rte, we) != 1 || err_after.end_st != LMX_MSG_OK) {
            fprintf(stderr, "error-after-end_turn done=%ld last=%d end_st=%d\n",
                (long)InterlockedCompareExchange(&err_after.done, 0, 0),
                lmx_msg_exec_last_status(rte, we), err_after.end_st);
            fflush(stderr);
            lmx_msg_runtime_delete(rte);
            return 1;
        }
        lmx_msg_runtime_delete(rte);
        fprintf(stderr, "err-after ok\n");
        fflush(stderr);
    }
    {
        LmxMsgRuntime *rto;
        LmxMsgAddr po = 0, wo = 0;
        LmxMsgEnv eo;
        uchar ini = 1, b = 5;
        static MassRec omit;
        fprintf(stderr, "omit-end start\n");
        fflush(stderr);
        memset(&omit, 0, sizeof(omit));
        rto = lmx_msg_runtime_new();
        lmx_msg_create(rto, 0, 1, &ini, 1, &po);
        lmx_msg_create(rto, po, 2, &ini, 1, &wo);
        lmx_msg_end_turn(rto, po, 1);
        memset(&eo, 0, sizeof(eo));
        eo.kind = LMX_MSG_KIND_BYTES;
        eo.n = 1;
        eo.bytes = &b;
        lmx_msg_send(rto, po, wo, &eo);
        lmx_msg_end_turn(rto, po, 1);
        lmx_msg_pump(rto);
        lmx_msg_exec_bind(rto, wo, turn_ok_no_end, &omit, LMX_MSG_AFFINITY_ANY);
        lmx_msg_exec_start(rto, 1);
        {
            DWORD dl = GetTickCount() + 2000;
            while (InterlockedCompareExchange(&omit.done, 0, 0) == 0 && GetTickCount() < dl) {
                Sleep(10);
            }
        }
        lmx_msg_exec_stop(rto);
        if (InterlockedCompareExchange(&omit.done, 0, 0) != 1 || omit.recv_st != LMX_MSG_OK || lmx_msg_exec_last_status(rto, wo) != 1) {
            fprintf(stderr, "ok-omit-end_turn done=%ld recv=%d last=%d\n",
                (long)InterlockedCompareExchange(&omit.done, 0, 0), omit.recv_st,
                lmx_msg_exec_last_status(rto, wo));
            fflush(stderr);
            lmx_msg_runtime_delete(rto);
            return 1;
        }
        lmx_msg_runtime_delete(rto);
        fprintf(stderr, "omit-end ok\n");
        fflush(stderr);
    }
    {
        LmxMsgRuntime *rtb;
        LmxMsgAddr pb = 0, wb = 0, ub = 0;
        LmxMsgEnv eb;
        uchar ini = 1, hb = 7;
        TurnCtx busy;
        TurnCtx bui;
        fprintf(stderr, "cpu-busy start\n");
        fflush(stderr);
        InterlockedExchange(&g_cpu_stop, 0);
        memset(&busy, 0, sizeof(busy));
        memset(&bui, 0, sizeof(bui));
        busy.started = CreateEventA(0, 1, 0, 0);
        rtb = lmx_msg_runtime_new();
        if (rtb == 0 || busy.started == 0) {
            return 1;
        }
        if (lmx_msg_create(rtb, 0, 1, &ini, 1, &pb) != LMX_MSG_OK) {
            return 1;
        }
        if (lmx_msg_create(rtb, pb, 2, &ini, 1, &wb) != LMX_MSG_OK) {
            return 1;
        }
        if (lmx_msg_create(rtb, pb, 3, &ini, 1, &ub) != LMX_MSG_OK) {
            return 1;
        }
        if (lmx_msg_end_turn(rtb, pb, 1) != LMX_MSG_OK) {
            return 1;
        }
        if (lmx_msg_exec_bind(rtb, wb, turn_busy, &busy, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK) {
            return 1;
        }
        if (lmx_msg_exec_bind(rtb, ub, turn_ui, &bui, LMX_MSG_AFFINITY_UI) != LMX_MSG_OK) {
            return 1;
        }
        memset(&eb, 0, sizeof(eb));
        eb.kind = LMX_MSG_KIND_BYTES;
        eb.n = 1;
        eb.bytes = &ini;
        if (lmx_msg_send(rtb, pb, wb, &eb) != LMX_MSG_STAGED) {
            return 1;
        }
        if (lmx_msg_end_turn(rtb, pb, 1) != LMX_MSG_OK) {
            return 1;
        }
        lmx_msg_pump(rtb);
        if (lmx_msg_exec_start(rtb, 1) != LMX_MSG_OK) {
            return 1;
        }
        if (WaitForSingleObject(busy.started, 2000) != WAIT_OBJECT_0) {
            fprintf(stderr, "busy turn did not start\n");
            return 1;
        }
        eb.bytes = &hb;
        if (lmx_msg_host_post(rtb, ub, &eb) != LMX_MSG_STAGED) {
            return 1;
        }
        tbusy_ui = GetTickCount();
        if (lmx_msg_exec_ui_step(rtb) != LMX_MSG_OK || bui.ui_recvd != 1 || bui.ui_bytes[0] != 7) {
            fprintf(stderr, "ui_step failed during CPU-busy recvd=%u byte=%u\n", bui.ui_recvd, bui.ui_bytes[0]);
            return 1;
        }
        tbusy_ui = GetTickCount() - tbusy_ui;
        if (InterlockedCompareExchange(&busy.done, 0, 0) != 0) {
            fprintf(stderr, "CPU-busy already finished before UI handled\n");
            return 1;
        }
        if (tbusy_ui > 50) {
            fprintf(stderr, "ui_step blocked on CPU-busy ms=%lu\n", (unsigned long)tbusy_ui);
            return 1;
        }
        InterlockedExchange(&g_cpu_stop, 1);
        {
            DWORD dl = GetTickCount() + 2000;
            while (InterlockedCompareExchange(&busy.done, 0, 0) == 0 && GetTickCount() < dl) {
                Sleep(10);
            }
        }
        lmx_msg_exec_stop(rtb);
        if (InterlockedCompareExchange(&busy.done, 0, 0) != 1) {
            fprintf(stderr, "CPU-busy did not finish done=%ld\n",
                (long)InterlockedCompareExchange(&busy.done, 0, 0));
            lmx_msg_runtime_delete(rtb);
            return 1;
        }
        lmx_msg_runtime_delete(rtb);
        CloseHandle(busy.started);
    }
    {
        LmxMsgRuntime *rtc;
        LmxMsgAddr pc = 0, uc = 0, cc = 0, bc = 0;
        LmxMsgEnv ec;
        uchar ini = 1;
        TurnCtx uictx;
        DWORD dl;
        fprintf(stderr, "close-path start\n");
        fflush(stderr);
        rtc = lmx_msg_runtime_new();
        memset(&uictx, 0, sizeof(uictx));
        lmx_msg_create(rtc, 0, 1, &ini, 1, &pc);
        lmx_msg_create(rtc, pc, 2, &ini, 1, &uc);
        lmx_msg_create(rtc, pc, 3, &ini, 1, &cc);
        lmx_msg_end_turn(rtc, pc, 1);
        if (lmx_msg_path_n(rtc, uc) < 1 || lmx_msg_init_copy(rtc, uc, &ec) != LMX_MSG_OK) {
            fprintf(stderr, "path/init_copy owner serial failed\n");
            return 1;
        }
        lmx_msg_env_release(&ec);
        lmx_msg_stop(rtc, pc, uc);
        lmx_msg_stop(rtc, pc, cc);
        lmx_msg_end_turn(rtc, pc, 1);
        lmx_msg_pump(rtc);
        lmx_msg_exec_bind(rtc, uc, turn_just_end, &uictx, LMX_MSG_AFFINITY_UI);
        if (lmx_msg_exec_start(rtc, 1) != LMX_MSG_OK) {
            return 1;
        }
        if (lmx_msg_fail(rtc, uc) != LMX_MSG_INVALID) {
            fprintf(stderr, "owner fail of worker-bound msg should be INVALID\n");
            return 1;
        }
        lmx_msg_drive(rtc, 0, 0);
        dl = GetTickCount() + 2000;
        while (InterlockedCompareExchange(&uictx.done, 0, 0) == 0 && GetTickCount() < dl) {
            lmx_msg_exec_ui_step(rtc);
            Sleep(5);
        }
        if (InterlockedCompareExchange(&uictx.done, 0, 0) != 1 || lmx_msg_state(rtc, uc) != LMX_MSG_STATE_STOPPED) {
            fprintf(stderr, "UI closer done=%ld state=%d\n",
                (long)InterlockedCompareExchange(&uictx.done, 0, 0), lmx_msg_state(rtc, uc));
            lmx_msg_runtime_delete(rtc);
            return 1;
        }
        lmx_msg_drive(rtc, 0, 0);
        if (lmx_msg_state(rtc, cc) != LMX_MSG_STATE_STOPPED) {
            fprintf(stderr, "unbound child state=%d\n", lmx_msg_state(rtc, cc));
            lmx_msg_runtime_delete(rtc);
            return 1;
        }
        lmx_msg_exec_stop(rtc);
        /* parent-loss: fail parent (workers stopped so owner turn fallback), poll descendants */
        lmx_msg_create(rtc, pc, 4, &ini, 1, &bc);
        lmx_msg_end_turn(rtc, pc, 1);
        if (lmx_msg_fail(rtc, pc) != LMX_MSG_OK) {
            fprintf(stderr, "fail parent after exec_stop\n");
            lmx_msg_runtime_delete(rtc);
            return 1;
        }
        lmx_msg_poll(rtc, 0, 0, 0, 0, 0);
        lmx_msg_drive(rtc, 0, 0);
        if (lmx_msg_state(rtc, bc) != LMX_MSG_STATE_STOPPED && lmx_msg_state(rtc, bc) != LMX_MSG_STATE_DEAD) {
            fprintf(stderr, "parent-loss child state=%d\n", lmx_msg_state(rtc, bc));
            lmx_msg_runtime_delete(rtc);
            return 1;
        }
        lmx_msg_runtime_delete(rtc);
    }
    {
        LmxMsgRuntime *rtl;
        LmxMsgAddr rl = 0, pl = 0, cl = 0, gl = 0, ul = 0, fl = 0;
        LmxMsgEnv el;
        uchar ini = 1, hb = 7;
        TurnCtx pctx, cctx, uctx, fctx;
        unsigned seg = 0;
        int pn;
        int nclose;
        DWORD dl;
        fprintf(stderr, "live-cascade start\n");
        fflush(stderr);
        InterlockedExchange(&g_cpu_stop, 0);
        memset(&pctx, 0, sizeof(pctx));
        memset(&cctx, 0, sizeof(cctx));
        memset(&uctx, 0, sizeof(uctx));
        memset(&fctx, 0, sizeof(fctx));
        pctx.started = CreateEventA(0, 1, 0, 0);
        cctx.started = CreateEventA(0, 1, 0, 0);
        fctx.started = CreateEventA(0, 1, 0, 0);
        fctx.unblock = CreateEventA(0, 1, 0, 0);
        rtl = lmx_msg_runtime_new();
        if (rtl == 0 || pctx.started == 0 || cctx.started == 0 || fctx.started == 0 || fctx.unblock == 0) {
            fprintf(stderr, "live-cascade setup runtime\n");
            return 1;
        }
        if (lmx_msg_create(rtl, 0, 1, &ini, 1, &rl) != LMX_MSG_OK || rl == 0) {
            fprintf(stderr, "live-cascade create root\n");
            return 1;
        }
        if (lmx_msg_create(rtl, rl, 2, &ini, 1, &pl) != LMX_MSG_OK || pl == 0) {
            fprintf(stderr, "live-cascade create parent\n");
            return 1;
        }
        if (lmx_msg_create(rtl, pl, 3, &ini, 1, &cl) != LMX_MSG_OK || cl == 0) {
            fprintf(stderr, "live-cascade create child\n");
            return 1;
        }
        if (lmx_msg_create(rtl, cl, 4, &ini, 1, &gl) != LMX_MSG_OK || gl == 0) {
            fprintf(stderr, "live-cascade create grandchild\n");
            return 1;
        }
        if (lmx_msg_create(rtl, rl, 5, &ini, 1, &ul) != LMX_MSG_OK || ul == 0) {
            fprintf(stderr, "live-cascade create ui\n");
            return 1;
        }
        if (lmx_msg_create(rtl, rl, 6, &ini, 1, &fl) != LMX_MSG_OK || fl == 0) {
            fprintf(stderr, "live-cascade create factory\n");
            return 1;
        }
        if (lmx_msg_end_turn(rtl, rl, 1) != LMX_MSG_OK) {
            return 1;
        }
        if (lmx_msg_end_turn(rtl, pl, 1) != LMX_MSG_OK) {
            return 1;
        }
        if (lmx_msg_end_turn(rtl, cl, 1) != LMX_MSG_OK) {
            return 1;
        }
        if (lmx_msg_exec_bind(rtl, pl, turn_held_live, &pctx, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK) {
            return 1;
        }
        if (lmx_msg_exec_bind(rtl, cl, turn_held_live, &cctx, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK) {
            return 1;
        }
        if (lmx_msg_exec_bind(rtl, ul, turn_ui, &uctx, LMX_MSG_AFFINITY_UI) != LMX_MSG_OK) {
            return 1;
        }
        if (lmx_msg_exec_bind(rtl, fl, turn_factory, &fctx, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK) {
            return 1;
        }
        memset(&el, 0, sizeof(el));
        el.kind = LMX_MSG_KIND_BYTES;
        el.n = 1;
        el.bytes = &ini;
        if (lmx_msg_send(rtl, rl, pl, &el) != LMX_MSG_STAGED) {
            return 1;
        }
        if (lmx_msg_send(rtl, rl, cl, &el) != LMX_MSG_STAGED) {
            return 1;
        }
        if (lmx_msg_send(rtl, rl, fl, &el) != LMX_MSG_STAGED) {
            return 1;
        }
        if (lmx_msg_end_turn(rtl, rl, 1) != LMX_MSG_OK) {
            return 1;
        }
        lmx_msg_pump(rtl);
        if (lmx_msg_inbox_n(rtl, pl) < 1 || lmx_msg_inbox_n(rtl, cl) < 1 || lmx_msg_inbox_n(rtl, fl) < 1) {
            fprintf(stderr, "live-cascade inbox p=%d c=%d f=%d state p=%d c=%d\n",
                lmx_msg_inbox_n(rtl, pl), lmx_msg_inbox_n(rtl, cl), lmx_msg_inbox_n(rtl, fl),
                lmx_msg_state(rtl, pl), lmx_msg_state(rtl, cl));
            return 1;
        }
        if (lmx_msg_exec_start(rtl, 3) != LMX_MSG_OK) {
            fprintf(stderr, "live-cascade start\n");
            return 1;
        }
        {
            DWORD wp = WaitForSingleObject(pctx.started, 2000);
            DWORD wc = WaitForSingleObject(cctx.started, 2000);
            DWORD wf = WaitForSingleObject(fctx.started, 2000);
            if (wp != WAIT_OBJECT_0 || wc != WAIT_OBJECT_0 || wf != WAIT_OBJECT_0) {
                SetEvent(fctx.unblock);
                fprintf(stderr, "live-cascade turns not started inbox p=%d c=%d runnable p=%d c=%d\n",
                    lmx_msg_inbox_n(rtl, pl), lmx_msg_inbox_n(rtl, cl),
                    lmx_msg_exec_is_runnable(rtl, pl), lmx_msg_exec_is_runnable(rtl, cl));
                return 1;
            }
        }
        g_factory_n_at_meta = (int)InterlockedCompareExchange(&fctx.extra_n, 0, 0);
        if (g_factory_n_at_meta < 1 || g_factory_n_at_meta >= 8) {
            SetEvent(fctx.unblock);
            fprintf(stderr, "factory not in create phase at meta n=%d\n", g_factory_n_at_meta);
            return 1;
        }
        pn = lmx_msg_path_n(rtl, cl);
        if (pn < 1 || lmx_msg_path_seg(rtl, cl, 0, &seg) != LMX_MSG_OK || lmx_msg_init_copy(rtl, cl, &el) != LMX_MSG_OK) {
            SetEvent(fctx.unblock);
            fprintf(stderr, "held-child path/init_copy during factory create phase\n");
            return 1;
        }
        lmx_msg_env_release(&el);
        SetEvent(fctx.unblock);
        if (lmx_msg_state(rtl, cl) != LMX_MSG_STATE_RUNNING || lmx_msg_state(rtl, gl) == LMX_MSG_STATE_RELEASED) {
            fprintf(stderr, "arena invalid while child held state=%d g=%d\n", lmx_msg_state(rtl, cl), lmx_msg_state(rtl, gl));
            return 1;
        }
        lmx_msg_set_now(rtl, 10000);
        nclose = lmx_msg_poll(rtl, 0, 10000, 1, 0, 0);
        if (nclose < 1) {
            fprintf(stderr, "timer parent-loss poll n=%d\n", nclose);
            return 1;
        }
        el.kind = LMX_MSG_KIND_BYTES;
        el.n = 1;
        el.bytes = &hb;
        if (lmx_msg_host_post(rtl, ul, &el) != LMX_MSG_STAGED) {
            return 1;
        }
        if (lmx_msg_exec_ui_step(rtl) != LMX_MSG_OK || uctx.ui_recvd != 1) {
            fprintf(stderr, "UI did not continue during live parent-loss recvd=%u\n", uctx.ui_recvd);
            return 1;
        }
        if (InterlockedCompareExchange(&cctx.done, 0, 0) != 0) {
            fprintf(stderr, "child finished before release\n");
            return 1;
        }
        InterlockedExchange(&g_cpu_stop, 1);
        dl = GetTickCount() + 3000;
        while ((InterlockedCompareExchange(&cctx.done, 0, 0) == 0 || InterlockedCompareExchange(&pctx.done, 0, 0) == 0) && GetTickCount() < dl) {
            Sleep(10);
        }
        lmx_msg_drive(rtl, 10000, 1);
        dl = GetTickCount() + 2000;
        while (lmx_msg_state(rtl, cl) == LMX_MSG_STATE_RUNNING && GetTickCount() < dl) {
            lmx_msg_drive(rtl, 10000, 1);
            Sleep(10);
        }
        if (lmx_msg_state(rtl, cl) != LMX_MSG_STATE_STOPPED) {
            fprintf(stderr, "live child not stopped at boundary state=%d\n", lmx_msg_state(rtl, cl));
            lmx_msg_runtime_delete(rtl);
            return 1;
        }
        lmx_msg_drive(rtl, 10000, 1);
        if (lmx_msg_state(rtl, gl) != LMX_MSG_STATE_STOPPED && lmx_msg_state(rtl, gl) != LMX_MSG_STATE_DEAD) {
            fprintf(stderr, "grandchild not terminated state=%d\n", lmx_msg_state(rtl, gl));
            lmx_msg_runtime_delete(rtl);
            return 1;
        }
        dl = GetTickCount() + 2000;
        while (InterlockedCompareExchange(&fctx.done, 0, 0) == 0 && GetTickCount() < dl) {
            Sleep(10);
        }
        if (InterlockedCompareExchange(&fctx.done, 0, 0) != 1 || InterlockedCompareExchange(&fctx.extra_n, 0, 0) != 8) {
            fprintf(stderr, "factory incomplete done=%ld n=%ld at_meta=%d\n",
                (long)InterlockedCompareExchange(&fctx.done, 0, 0),
                (long)InterlockedCompareExchange(&fctx.extra_n, 0, 0), g_factory_n_at_meta);
            lmx_msg_runtime_delete(rtl);
            return 1;
        }
        g_factory_n = (int)InterlockedCompareExchange(&fctx.extra_n, 0, 0);
        {
            int k;
            for (k = 0; k < 8; k++) {
                if (fctx.extras[k] == 0 || lmx_msg_state(rtl, fctx.extras[k]) == LMX_MSG_STATE_RELEASED) {
                    fprintf(stderr, "factory extra %d missing\n", k);
                    lmx_msg_runtime_delete(rtl);
                    return 1;
                }
            }
        }
        lmx_msg_exec_stop(rtl);
        CloseHandle(pctx.started);
        CloseHandle(cctx.started);
        CloseHandle(fctx.started);
        CloseHandle(fctx.unblock);
        lmx_msg_runtime_delete(rtl);
        g_live_cascade = 1;
    }
    {
        LmxMsgRuntime *rto;
        LmxMsgAddr po = 0, bs0 = 0, bs1 = 0, wo[11], wf = 0;
        LmxMsgEnv eo;
        uchar ini = 1;
        uchar payload[11];
        uchar f10 = 10, f20 = 20;
        static MassRec oom[11];
        static MassRec oomfifo;
        TurnCtx spin0, spin1;
        int k;
        int cap;
        int nrd;
        int hits;
        int sc;
        DWORD dl;
        fprintf(stderr, "oom start\n");
        fflush(stderr);
        InterlockedExchange(&g_cpu_stop, 0);
        memset(&spin0, 0, sizeof(spin0));
        memset(&spin1, 0, sizeof(spin1));
        memset(oom, 0, sizeof(oom));
        memset(&oomfifo, 0, sizeof(oomfifo));
        memset(wo, 0, sizeof(wo));
        spin0.started = CreateEventA(0, 1, 0, 0);
        spin1.started = CreateEventA(0, 1, 0, 0);
        rto = lmx_msg_runtime_new();
        if (rto == 0 || spin0.started == 0 || spin1.started == 0) {
            fprintf(stderr, "oom rt\n");
            return 1;
        }
        if (lmx_msg_create(rto, 0, 1, &ini, 1, &po) != LMX_MSG_OK) {
            fprintf(stderr, "oom create po\n");
            fflush(stderr);
            return 1;
        }
        if (lmx_msg_create(rto, po, 2, &ini, 1, &bs0) != LMX_MSG_OK) {
            fprintf(stderr, "oom create bs0\n");
            fflush(stderr);
            return 1;
        }
        if (lmx_msg_create(rto, po, 3, &ini, 1, &bs1) != LMX_MSG_OK) {
            fprintf(stderr, "oom create bs1\n");
            fflush(stderr);
            return 1;
        }
        for (k = 0; k < 11; k++) {
            payload[k] = (uchar)(50 + k);
            oom[k].expect = payload[k];
            if (lmx_msg_create(rto, po, (unsigned)(k + 10), &ini, 1, &wo[k]) != LMX_MSG_OK || wo[k] == 0) {
                fprintf(stderr, "oom create %d\n", k);
                return 1;
            }
            if (lmx_msg_exec_bind(rto, wo[k], turn_mass, &oom[k], LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK) {
                fprintf(stderr, "oom bind %d\n", k);
                fflush(stderr);
                return 1;
            }
        }
        if (lmx_msg_create(rto, po, 40, &ini, 1, &wf) != LMX_MSG_OK || wf == 0) {
            fprintf(stderr, "oom create wf\n");
            fflush(stderr);
            return 1;
        }
        if (lmx_msg_exec_bind(rto, wf, turn_two_ok, &oomfifo, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK) {
            fprintf(stderr, "oom bind wf\n");
            fflush(stderr);
            return 1;
        }
        if (lmx_msg_exec_bind(rto, bs0, turn_busy, &spin0, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK) {
            fprintf(stderr, "oom bind bs0\n");
            fflush(stderr);
            return 1;
        }
        if (lmx_msg_exec_bind(rto, bs1, turn_busy, &spin1, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK) {
            fprintf(stderr, "oom bind bs1\n");
            fflush(stderr);
            return 1;
        }
        if (lmx_msg_end_turn(rto, po, 1) != LMX_MSG_OK) {
            fprintf(stderr, "oom end_turn commit\n");
            fflush(stderr);
            return 1;
        }
        lmx_msg_exec_drop_stale_ready(rto);
        memset(&eo, 0, sizeof(eo));
        eo.kind = LMX_MSG_KIND_BYTES;
        eo.n = 1;
        eo.bytes = &ini;
        if (lmx_msg_send(rto, po, bs0, &eo) != LMX_MSG_STAGED || lmx_msg_send(rto, po, bs1, &eo) != LMX_MSG_STAGED) {
            fprintf(stderr, "oom send busy\n");
            fflush(stderr);
            return 1;
        }
        if (lmx_msg_end_turn(rto, po, 1) != LMX_MSG_OK) {
            fprintf(stderr, "oom end_turn busy\n");
            fflush(stderr);
            return 1;
        }
        lmx_msg_pump(rto);
        if (lmx_msg_exec_start(rto, 2) != LMX_MSG_OK) {
            fprintf(stderr, "oom exec_start\n");
            fflush(stderr);
            return 1;
        }
        if (WaitForSingleObject(spin0.started, 2000) != WAIT_OBJECT_0 || WaitForSingleObject(spin1.started, 2000) != WAIT_OBJECT_0) {
            fprintf(stderr, "oom busy workers not started\n");
            fflush(stderr);
            return 1;
        }
        fprintf(stderr, "oom busy running nready=%d cap=%d\n",
            lmx_msg_exec_nready(rto), lmx_msg_exec_ready_cap(rto));
        fflush(stderr);
        for (k = 0; k < 8; k++) {
            eo.bytes = &payload[k];
            if (lmx_msg_host_post(rto, wo[k], &eo) != LMX_MSG_STAGED) {
                fprintf(stderr, "oom host_post fill %d\n", k);
                fflush(stderr);
                return 1;
            }
        }
        if (lmx_msg_host_drain(rto) != LMX_MSG_OK) {
            fprintf(stderr, "oom drain fill\n");
            fflush(stderr);
            return 1;
        }
        cap = lmx_msg_exec_ready_cap(rto);
        nrd = lmx_msg_exec_nready(rto);
        fprintf(stderr, "oom filled cap=%d nready=%d\n", cap, nrd);
        fflush(stderr);
        if (cap != nrd || nrd < 8) {
            fprintf(stderr, "oom not at capacity cap=%d nready=%d\n", cap, nrd);
            fflush(stderr);
            lmx_msg_runtime_delete(rto);
            return 1;
        }
        for (k = 8; k < 11; k++) {
            if (lmx_msg_exec_ready_has(rto, wo[k]) != 0) {
                fprintf(stderr, "oom wo[%d] already in ready before overflow\n", k);
                fflush(stderr);
                lmx_msg_runtime_delete(rto);
                return 1;
            }
        }
        if (lmx_msg_exec_ready_has(rto, wf) != 0) {
            fprintf(stderr, "oom fifo addr already in ready before overflow\n");
            fflush(stderr);
            lmx_msg_runtime_delete(rto);
            return 1;
        }
        lmx_msg_exec_test_set_fail_grow(rto, 1);
        for (k = 8; k < 11; k++) {
            eo.bytes = &payload[k];
            if (lmx_msg_host_post(rto, wo[k], &eo) != LMX_MSG_STAGED) {
                fprintf(stderr, "oom host_post overflow %d\n", k);
                fflush(stderr);
                return 1;
            }
        }
        eo.bytes = &f10;
        if (lmx_msg_host_post(rto, wf, &eo) != LMX_MSG_STAGED) {
            fprintf(stderr, "oom host_post fifo 10\n");
            fflush(stderr);
            return 1;
        }
        eo.bytes = &f20;
        if (lmx_msg_host_post(rto, wf, &eo) != LMX_MSG_STAGED) {
            fprintf(stderr, "oom host_post fifo 20\n");
            fflush(stderr);
            return 1;
        }
        if (lmx_msg_host_drain(rto) != LMX_MSG_OK) {
            fprintf(stderr, "oom drain overflow\n");
            fflush(stderr);
            return 1;
        }
        hits = lmx_msg_exec_test_fail_hits(rto);
        sc = lmx_msg_exec_get_scan(rto);
        fprintf(stderr, "oom inject hits=%d scan=%d nready=%d was=%d\n",
            hits, sc, lmx_msg_exec_nready(rto), nrd);
        fflush(stderr);
        if (hits <= 0 || sc != 1 || lmx_msg_exec_nready(rto) != nrd) {
            fprintf(stderr, "oom inject not real hits=%d scan=%d nready=%d was=%d\n",
                hits, sc, lmx_msg_exec_nready(rto), nrd);
            fflush(stderr);
            lmx_msg_runtime_delete(rto);
            return 1;
        }
        for (k = 8; k < 11; k++) {
            if (lmx_msg_exec_ready_has(rto, wo[k]) != 0 || InterlockedCompareExchange(&oom[k].done, 0, 0) != 0) {
                fprintf(stderr, "oom overflow delivered under fail_grow k=%d ready=%d done=%ld\n",
                    k, lmx_msg_exec_ready_has(rto, wo[k]), (long)oom[k].done);
                lmx_msg_runtime_delete(rto);
                return 1;
            }
        }
        if (lmx_msg_exec_ready_has(rto, wf) != 0 || InterlockedCompareExchange(&oomfifo.done, 0, 0) != 0) {
            fprintf(stderr, "oom fifo delivered under fail_grow ready=%d done=%ld\n",
                lmx_msg_exec_ready_has(rto, wf), (long)oomfifo.done);
            lmx_msg_runtime_delete(rto);
            return 1;
        }
        lmx_msg_exec_test_set_fail_grow(rto, 0);
        InterlockedExchange(&g_cpu_stop, 1);
        dl = GetTickCount() + 5000;
        {
            int got = 0;
            LONG fifo_done = 0;
            while (GetTickCount() < dl) {
                got = 0;
                for (k = 0; k < 11; k++) {
                    if (InterlockedCompareExchange(&oom[k].done, 0, 0) == 1) {
                        got += 1;
                    }
                }
                fifo_done = InterlockedCompareExchange(&oomfifo.done, 0, 0);
                if (got == 11 && fifo_done == 2) {
                    break;
                }
                Sleep(10);
            }
            if (got != 11 || fifo_done != 2) {
                fprintf(stderr, "oom scan delivered %d/11 fifo_done=%ld hits=%d nready=%d scan=%d\n",
                    got, (long)fifo_done, hits, lmx_msg_exec_nready(rto), lmx_msg_exec_get_scan(rto));
                fflush(stderr);
                InterlockedExchange(&g_cpu_stop, 1);
                lmx_msg_exec_stop(rto);
                lmx_msg_runtime_delete(rto);
                return 1;
            }
        }
        lmx_msg_exec_stop(rto);
        for (k = 0; k < 11; k++) {
            if (oom[k].got != oom[k].expect || oom[k].recv_st != LMX_MSG_OK || oom[k].end_st != LMX_MSG_OK || InterlockedCompareExchange(&oom[k].done, 0, 0) != 1 || InterlockedCompareExchange(&oom[k].in_turn, 0, 0) != 0) {
                fprintf(stderr, "oom[%d] got=%u expect=%u done=%ld recv=%d end=%d in_turn=%ld\n",
                    k, oom[k].got, oom[k].expect, (long)oom[k].done, oom[k].recv_st, oom[k].end_st,
                    (long)InterlockedCompareExchange(&oom[k].in_turn, 0, 0));
                fflush(stderr);
                lmx_msg_runtime_delete(rto);
                return 1;
            }
            if (lmx_msg_exec_ready_has(rto, wo[k]) != 0 || lmx_msg_inbox_n(rto, wo[k]) != 0 || lmx_msg_exec_is_runnable(rto, wo[k]) != 0) {
                fprintf(stderr, "oom[%d] not drained ready=%d inbox=%d runnable=%d\n",
                    k, lmx_msg_exec_ready_has(rto, wo[k]), lmx_msg_inbox_n(rto, wo[k]),
                    lmx_msg_exec_is_runnable(rto, wo[k]));
                fflush(stderr);
                lmx_msg_runtime_delete(rto);
                return 1;
            }
        }
        if (oomfifo.fifo_a != 10 || oomfifo.fifo_b != 20 || oomfifo.recv_st != LMX_MSG_OK || oomfifo.end_st != LMX_MSG_OK || InterlockedCompareExchange(&oomfifo.done, 0, 0) != 2 || InterlockedCompareExchange(&oomfifo.in_turn, 0, 0) != 0 || oomfifo.overlap != 0) {
            fprintf(stderr, "oom fifo %u,%u done=%ld recv=%d end=%d overlap=%d in_turn=%ld hits=%d scan_was=%d\n",
                oomfifo.fifo_a, oomfifo.fifo_b, (long)InterlockedCompareExchange(&oomfifo.done, 0, 0),
                oomfifo.recv_st, oomfifo.end_st, oomfifo.overlap,
                (long)InterlockedCompareExchange(&oomfifo.in_turn, 0, 0), hits, sc);
            fflush(stderr);
            lmx_msg_runtime_delete(rto);
            return 1;
        }
        if (lmx_msg_exec_ready_has(rto, wf) != 0 || lmx_msg_inbox_n(rto, wf) != 0 || lmx_msg_exec_is_runnable(rto, wf) != 0 || lmx_msg_exec_nready(rto) != 0) {
            fprintf(stderr, "oom fifo not drained ready=%d inbox=%d runnable=%d nready=%d\n",
                lmx_msg_exec_ready_has(rto, wf), lmx_msg_inbox_n(rto, wf),
                lmx_msg_exec_is_runnable(rto, wf), lmx_msg_exec_nready(rto));
            fflush(stderr);
            lmx_msg_runtime_delete(rto);
            return 1;
        }
        g_oom_n = 12;
        g_oom_hits = hits;
        g_oom_scan = sc;
        g_oom_fifo_a = oomfifo.fifo_a;
        g_oom_fifo_b = oomfifo.fifo_b;
        CloseHandle(spin0.started);
        CloseHandle(spin1.started);
        lmx_msg_runtime_delete(rto);
    }
    {
        LmxMsgRuntime *rtc;
        LmxMsgAddr p = 0, a = 0, b = 0;
        LmxMsgEnv e;
        uchar ini = 1, pa = 2, pb = 3;
        static MassRec ra;
        static MassRec rb;
        DWORD dl;
        memset(&ra, 0, sizeof(ra));
        memset(&rb, 0, sizeof(rb));
        rtc = lmx_msg_runtime_new();
        if (rtc == 0 || lmx_msg_create(rtc, 0, 1, &ini, 1, &p) != LMX_MSG_OK) {
            fprintf(stderr, "ctx family setup\n");
            return 1;
        }
        if (lmx_msg_create(rtc, p, 2, &ini, 1, &a) != LMX_MSG_OK || lmx_msg_create(rtc, p, 3, &ini, 1, &b) != LMX_MSG_OK) {
            fprintf(stderr, "ctx children\n");
            return 1;
        }
        if (lmx_msg_exec_bind(rtc, a, turn_mass, &ra, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK || lmx_msg_exec_bind(rtc, b, turn_mass, &rb, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK) {
            fprintf(stderr, "ctx bind\n");
            return 1;
        }
        if (lmx_msg_end_turn(rtc, p, 1) != LMX_MSG_OK) {
            fprintf(stderr, "ctx commit\n");
            return 1;
        }
        memset(&e, 0, sizeof(e));
        e.kind = LMX_MSG_KIND_BYTES;
        e.n = 1;
        e.bytes = &pa;
        ra.expect = 2;
        if (lmx_msg_send(rtc, p, a, &e) != LMX_MSG_STAGED) {
            fprintf(stderr, "ctx send a\n");
            return 1;
        }
        e.bytes = &pb;
        rb.expect = 3;
        if (lmx_msg_send(rtc, p, b, &e) != LMX_MSG_STAGED) {
            fprintf(stderr, "ctx send b\n");
            return 1;
        }
        if (lmx_msg_end_turn(rtc, p, 1) != LMX_MSG_OK) {
            fprintf(stderr, "ctx publish\n");
            return 1;
        }
        lmx_msg_pump(rtc);
        if (lmx_msg_exec_start_contexts(rtc) != LMX_MSG_OK) {
            fprintf(stderr, "ctx start\n");
            return 1;
        }
        dl = GetTickCount() + 3000;
        while ((InterlockedCompareExchange(&ra.done, 0, 0) == 0 || InterlockedCompareExchange(&rb.done, 0, 0) == 0) && GetTickCount() < dl) {
            Sleep(10);
        }
        lmx_msg_exec_stop(rtc);
        if (InterlockedCompareExchange(&ra.done, 0, 0) != 1 || InterlockedCompareExchange(&rb.done, 0, 0) != 1) {
            fprintf(stderr, "ctx parallel done a=%ld b=%ld\n",
                (long)InterlockedCompareExchange(&ra.done, 0, 0),
                (long)InterlockedCompareExchange(&rb.done, 0, 0));
            lmx_msg_runtime_delete(rtc);
            return 1;
        }
        lmx_msg_runtime_delete(rtc);
        fprintf(stderr, "ctx_parallel a=%ld b=%ld\n",
            (long)InterlockedCompareExchange(&ra.done, 0, 0),
            (long)InterlockedCompareExchange(&rb.done, 0, 0));
        fflush(stderr);
    }
    ev = fopen("build/l1trans/logs/gen2/lmx_message_exec_selftest.evidence.txt", "w");
    if (ev) {
        fprintf(ev, "slow t0=%lu t1=%lu recvd=%u\n", (unsigned long)slow.t0, (unsigned long)slow.t1, slow.recvd);
        fprintf(ev, "fast t2=%lu recvd=%u fifo=%u,%u send_ui=%d send_peer=%d peer_got=%u\n",
            (unsigned long)fast.t2, fast.recvd, fast.fifo[0], fast.fifo[1],
            fast.send_ui_st, fast.send_peer_st, peerrec.got);
        fprintf(ev, "ui_step_ms=%lu cpu_busy_ui_ms=%lu mass_complete=70 fail_fifo=31,32 err_after=1 omit_end=1 ui_from_worker=%u peer=42\n",
            (unsigned long)tui, (unsigned long)tbusy_ui, slow.ui_recvd);
        fprintf(ev, "live_cascade=%d factory_n=%d factory_create_phase_n=%d held_child_meta=1 oom_n=%d oom_hits=%d oom_scan=%d oom_fifo=%u,%u ctx_parallel=1\n",
            g_live_cascade, g_factory_n, g_factory_n_at_meta, g_oom_n, g_oom_hits, g_oom_scan, g_oom_fifo_a, g_oom_fifo_b);
        fclose(ev);
    }
    printf("lmx_message_exec ok fifo=%u,%u mass=70 fail=31,32 err_after=1 omit_end=1 xsend_ui=%d xsend_peer=%d peer=%u ui_from_worker=%u ui_ms=%lu cpu_busy_ui_ms=%lu live_cascade=%d factory_n=%d factory_create_phase_n=%d oom_n=%d oom_hits=%d oom_scan=%d oom_fifo=%u,%u\n",
        fast.fifo[0], fast.fifo[1], fast.send_ui_st, fast.send_peer_st, peerrec.got,
        slow.ui_recvd, (unsigned long)tui, (unsigned long)tbusy_ui, g_live_cascade, g_factory_n,
        g_factory_n_at_meta, g_oom_n, g_oom_hits, g_oom_scan, g_oom_fifo_a, g_oom_fifo_b);
    return 0;
}
