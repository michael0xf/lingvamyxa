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
static HANDLE g_cleanup_seen;
static HANDLE g_cleanup_go;
static volatile LONG g_cleanup_hits;

static void after_cleanup_gate(LmxMsgAddr who, int live, int st) {
    (void)who;
    (void)live;
    (void)st;
    if (InterlockedIncrement(&g_cleanup_hits) == 1) {
        SetEvent(g_cleanup_seen);
        if (WaitForSingleObject(g_cleanup_go, 2000) != WAIT_OBJECT_0) {
            ExitProcess(2);
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
    if (lmx_msg_create(rt, parent, 5, init, 1, &w3) != LMX_MSG_OK) {
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
    memset(&peerrec, 0, sizeof(peerrec));
    peerrec.expect = 42;
    if (lmx_msg_exec_bind(rt, w3, turn_mass, &peerrec, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK) {
        return 1;
    }
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
                return 1;
            }
            if (lmx_msg_exec_bind(rt, extra, turn_mass, &g_mass[k], LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK) {
                return 1;
            }
            env.bytes = &payload[k];
            if (lmx_msg_send(rt, parent, extra, &env) != LMX_MSG_STAGED) {
                return 1;
            }
        }
        lmx_msg_end_turn(rt, parent, 1);
        lmx_msg_pump(rt);
    }
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
                    (long)slow.done, (long)fast.done, mass_done, (long)peerrec.done);
                return 1;
            }
        }
    }
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
    lmx_msg_exec_stop(rt);
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
    if (fast.send_ui_st != LMX_MSG_STAGED || fast.send_peer_st != LMX_MSG_STAGED || peerrec.got != 42 || peerrec.recv_st != LMX_MSG_OK || peerrec.end_st != LMX_MSG_OK || slow.ui_bytes[0] != 7 || slow.ui_bytes[1] != 8) {
        fprintf(stderr, "cross-send ui_st=%d peer_st=%d got=%u recv=%d end=%d ui_bytes=%u,%u\n",
            fast.send_ui_st, fast.send_peer_st, peerrec.got, peerrec.recv_st, peerrec.end_st,
            slow.ui_bytes[0], slow.ui_bytes[1]);
        lmx_msg_runtime_delete(rt);
        return 1;
    }
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
    lmx_msg_runtime_delete(rt);
    CloseHandle(slow.unblock);
    CloseHandle(slow.started);
    CloseHandle(fast.started);

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
            return 1;
        }
        if (lmx_msg_create(rtf, 0, 1, &ini, 1, &pf) != LMX_MSG_OK) {
            return 1;
        }
        if (lmx_msg_create(rtf, pf, 2, &ini, 1, &wf) != LMX_MSG_OK) {
            return 1;
        }
        if (lmx_msg_end_turn(rtf, pf, 1) != LMX_MSG_OK) {
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
            return 1;
        }
        g_cleanup_seen = CreateEventA(0, 1, 0, 0);
        g_cleanup_go = CreateEventA(0, 1, 0, 0);
        g_cleanup_hits = 0;
        lmx_msg_exec_test_after_cleanup = after_cleanup_gate;
        if (lmx_msg_exec_start(rtf, 2) != LMX_MSG_OK) {
            return 1;
        }
        if (WaitForSingleObject(g_cleanup_seen, 2000) != WAIT_OBJECT_0) {
            fprintf(stderr, "cleanup gate not reached\n");
            return 1;
        }
        if (InterlockedCompareExchange(&failrec.done, 0, 0) != 1) {
            fprintf(stderr, "next turn started during cleanup done=%ld\n",
                (long)InterlockedCompareExchange(&failrec.done, 0, 0));
            return 1;
        }
        SetEvent(g_cleanup_go);
        dl = GetTickCount() + 3000;
        while (InterlockedCompareExchange(&failrec.done, 0, 0) < 2 && GetTickCount() < dl) {
            Sleep(10);
        }
        lmx_msg_exec_stop(rtf);
        lmx_msg_exec_test_after_cleanup = 0;
        if (InterlockedCompareExchange(&failrec.done, 0, 0) != 2 || failrec.fifo_a != 31 || failrec.fifo_b != 32 || InterlockedCompareExchange(&failrec.in_turn, 0, 0) != 0 || failrec.overlap != 0 || lmx_msg_exec_last_status(rtf, wf) != 1) {
            fprintf(stderr, "fail-handler race done=%ld got=%u,%u overlap=%d in_turn=%ld last=%d\n",
                (long)InterlockedCompareExchange(&failrec.done, 0, 0), failrec.fifo_a, failrec.fifo_b,
                failrec.overlap, (long)InterlockedCompareExchange(&failrec.in_turn, 0, 0),
                lmx_msg_exec_last_status(rtf, wf));
            lmx_msg_runtime_delete(rtf);
            return 1;
        }
        lmx_msg_runtime_delete(rtf);
        CloseHandle(g_cleanup_seen);
        CloseHandle(g_cleanup_go);
    }
    {
        LmxMsgRuntime *rte;
        LmxMsgAddr pe = 0, we = 0;
        LmxMsgEnv ee;
        uchar ini = 1, b = 9;
        static MassRec err_after;
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
            lmx_msg_runtime_delete(rte);
            return 1;
        }
        lmx_msg_runtime_delete(rte);
    }
    {
        LmxMsgRuntime *rto;
        LmxMsgAddr po = 0, wo = 0;
        LmxMsgEnv eo;
        uchar ini = 1, b = 5;
        static MassRec omit;
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
            lmx_msg_runtime_delete(rto);
            return 1;
        }
        lmx_msg_runtime_delete(rto);
    }
    {
        LmxMsgRuntime *rtb;
        LmxMsgAddr pb = 0, wb = 0, ub = 0;
        LmxMsgEnv eb;
        uchar ini = 1, hb = 7;
        TurnCtx busy;
        TurnCtx bui;
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
    ev = fopen("build/l1trans/logs/gen2/lmx_message_exec_selftest.evidence.txt", "w");
    if (ev) {
        fprintf(ev, "slow t0=%lu t1=%lu recvd=%u\n", (unsigned long)slow.t0, (unsigned long)slow.t1, slow.recvd);
        fprintf(ev, "fast t2=%lu recvd=%u fifo=%u,%u send_ui=%d send_peer=%d peer_got=%u\n",
            (unsigned long)fast.t2, fast.recvd, fast.fifo[0], fast.fifo[1],
            fast.send_ui_st, fast.send_peer_st, peerrec.got);
        fprintf(ev, "ui_step_ms=%lu cpu_busy_ui_ms=%lu mass_complete=70 fail_fifo=31,32 err_after=1 omit_end=1 ui_from_worker=%u peer=42\n",
            (unsigned long)tui, (unsigned long)tbusy_ui, slow.ui_recvd);
        fclose(ev);
    }
    printf("lmx_message_exec ok fifo=%u,%u mass=70 fail=31,32 err_after=1 omit_end=1 xsend_ui=%d xsend_peer=%d peer=%u ui_from_worker=%u ui_ms=%lu cpu_busy_ui_ms=%lu\n",
        fast.fifo[0], fast.fifo[1], fast.send_ui_st, fast.send_peer_st, peerrec.got,
        slow.ui_recvd, (unsigned long)tui, (unsigned long)tbusy_ui);
    return 0;
}
