/* Overlapping Message turns. Win32. */
#include "l2src/lmx_message.h"
#include "l2src/lmx_message_exec.h"
#include "l2src/lmx.h"
#include "l2src/lmx_chars_owned.lm1.h"
#include "l2src/lmx_array_owned.lm1.h"
#include "l2src/lmx_array_ref_owned.lm1.h"
#include "l2src/lmx_branch_owned.lm1.h"
#include "l2src/lmx_value_owned.lm1.h"
#include "l2src/lmx_msg_storage.lm1.h"
#include <stdio.h>
#include <stdlib.h>
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
static int g_ctx_overlap;
static int g_ctx_restart;
static int g_ctx_child;
static int g_ctx_rebind_ui;
static int g_ctx_fail_retry;
static int g_ctx_busy_ui;
static int g_ctx_rebind_auth;
static int g_ctx_ui_any_rb;
static int g_ctx_mid_unroll;
static int g_ctx_spawn_race;
static int g_ctx_mix_map;
static int g_ctx_map_fail;
static int g_ctx_child_timer;
static int g_ctx_real_clock;
static int g_ctx_bind_rollback;
static int g_ctx_idle_rollback;
static int g_ctx_cancel_idle;
static int g_ctx_complete_idle;
static int g_m0_acc;
static int g_m0_overlap;
static unsigned g_m0_admit0;
static unsigned g_m0_admit1;
static unsigned g_m0_apply0;
static unsigned g_m0_apply1;
static LmxMsgAddr g_admit_dest;
static unsigned g_admit_log[8];
static volatile LONG g_admit_n;

void lmx_msg_test_on_admit(LmxMsgAddr dest, const LmxMsgCopy *fresh) {
    LONG n;
    if (g_admit_dest == 0U || dest != g_admit_dest || fresh == 0 || fresh->n == 0U || fresh->bytes == 0) {
        return;
    }
    n = InterlockedIncrement(&g_admit_n) - 1;
    if (n >= 0 && n < 8) {
        g_admit_log[n] = fresh->bytes[0];
    }
}
static int g_ctx_bind_held;
static int g_ctx_bind_ctx;
static LmxMsgAddr g_hook_extra;
static TurnCtx g_hook_ctx;

typedef struct RendezRec {
    HANDLE entered;
    HANDLE peer;
    volatile LONG in_turn;
    volatile LONG done;
    DWORD tid;
    int recv_st;
} RendezRec;

typedef struct SpawnRec {
    LmxMsgRuntime *rt;
    LmxMsgAddr child;
    MassRec *child_rec;
    HANDLE started;
    HANDLE go;
    volatile LONG done;
    int st;
} SpawnRec;

typedef struct TryUiRec {
    LmxMsgAddr other;
    void *other_ctx;
    int st;
    int aff_after;
    volatile LONG done;
} TryUiRec;

typedef struct MixRec {
    RendezRec *a1;
    RendezRec *a2;
    DWORD tid;
    volatile LONG done;
    int st;
} MixRec;
static TurnCtx g_self_new;
static int g_self_unbind_st;
static int g_self_bind_st;
static int g_self_aff_after;
static int g_self_bound_after;
static int turn_recv_end(LmxMsgRuntime *rt, LmxMsgAddr who, void *ctx);
static int turn_just_end(LmxMsgRuntime *rt, LmxMsgAddr who, void *ctx);
/* The mapping cell is the parent's (19.28.R2.2 (2)): a sibling's turn, even on the host
 * thread, can neither rebind nor unbind it. */
/* Stage 5 (b): counts the turns a root is given while only INGRESS is pending. */
static volatile LONG g_ingress_root_turns;
static int turn_count_root(LmxMsgRuntime *rt, LmxMsgAddr who, void *ctx) {
    LmxMsgEnv got;
    (void)ctx;
    InterlockedIncrement(&g_ingress_root_turns);
    memset(&got, 0, sizeof(got));
    if (lmx_msg_recv(rt, who, &got) == LMX_MSG_OK) {
        lmx_msg_env_release(&got);
    }
    return lmx_msg_end_turn(rt, who, 1);
}
typedef struct SiblingMapRec {
    LmxMsgAddr other;
    int bind_st;
    int unbind_st;
} SiblingMapRec;
static int turn_map_sibling(LmxMsgRuntime *rt, LmxMsgAddr who, void *ctx) {
    SiblingMapRec *s = (SiblingMapRec *)ctx;
    LmxMsgEnv got;
    memset(&got, 0, sizeof(got));
    (void)lmx_msg_recv(rt, who, &got);
    lmx_msg_env_release(&got);
    s->bind_st = lmx_msg_exec_bind(rt, s->other, turn_recv_end, 0, LMX_MSG_AFFINITY_UI);
    s->unbind_st = lmx_msg_exec_unbind(rt, s->other);
    return lmx_msg_end_turn(rt, who, 1);
}
/* P's turn releases its settled child C, whose settled child G is settled into it first:
 * G's unbind runs on P's lane, the nearest unsettled ancestor (19.29.6). */
typedef struct DisposeInTurnRec {
    LmxMsgAddr child;
    int st;
} DisposeInTurnRec;
static int turn_dispose_settled(LmxMsgRuntime *rt, LmxMsgAddr who, void *ctx) {
    DisposeInTurnRec *d = (DisposeInTurnRec *)ctx;
    d->st = lmx_msg_dispose_child(rt, who, d->child);
    return lmx_msg_end_turn(rt, who, 1);
}
/* Stage 5 (c): drive is the root's maintenance between its turns; a turn's call
 * records the refusal. */
typedef struct {
    unsigned now;
    int st;
} DriveInTurnRec;
static int turn_drive_in_turn(LmxMsgRuntime *rt, LmxMsgAddr who, void *ctx) {
    DriveInTurnRec *d = (DriveInTurnRec *)ctx;
    d->st = lmx_msg_drive(rt, d->now, 0U);
    return lmx_msg_end_turn(rt, who, 1);
}
/* A Message never maps itself (19.28.R2.2 (2): the binding is its parent's cell):
 * from inside its own turn both unbind and rebind refuse and leave the binding and
 * its affinity as they were. */
static int turn_self_rebind(LmxMsgRuntime *rt, LmxMsgAddr who, void *ctx) {
    TurnCtx *c = (TurnCtx *)ctx;
    LmxMsgEnv got;
    memset(&got, 0, sizeof(got));
    (void)lmx_msg_recv(rt, who, &got);
    lmx_msg_env_release(&got);
    g_self_unbind_st = lmx_msg_exec_unbind(rt, who);
    g_self_bind_st = lmx_msg_exec_bind(rt, who, turn_recv_end, &g_self_new, LMX_MSG_AFFINITY_UI);
    g_self_aff_after = lmx_msg_exec_bind_aff(rt, who);
    g_self_bound_after = lmx_msg_exec_is_bound(rt, who);
    InterlockedIncrement(&c->done);
    return lmx_msg_end_turn(rt, who, 1);
}

static LmxMsgAddr g_launch_unb;
static int g_launch_phase;
static TurnCtx g_stale_g2;
static unsigned g_stale_gen;
static unsigned g_stale_old_gen;
static void *g_stale_wh;
static int g_stale_launching;
static int g_stale_workers;
static int g_stale_alive;
static int g_stale_launch_n;
static unsigned g_stale_destroy_at_unbind;
static int g_stale_want_ui;
static void launch_unbind_hook(LmxMsgRuntime *rt, LmxMsgAddr addr, int after) {
    if (addr == g_launch_unb && after == g_launch_phase) {
        (void)lmx_msg_exec_unbind(rt, addr);
    }
}
static void stale_launch_hook(LmxMsgRuntime *rt, LmxMsgAddr addr, int after) {
    void *cap;
    unsigned gen;
    if (addr != g_launch_unb || after != 0) {
        return;
    }
    lmx_msg_exec_test_during_launch = 0;
    cap = lmx_msg_exec_test_launch_cap();
    gen = lmx_msg_exec_test_launch_cap_gen();
    g_stale_old_gen = gen;
    (void)lmx_msg_exec_unbind(rt, addr);
    g_stale_alive = (cap != 0 && lmx_msg_exec_test_wait_gen_raw(cap) == gen);
    g_stale_launch_n = lmx_msg_exec_test_wait_launch_n(cap);
    g_stale_destroy_at_unbind = lmx_msg_exec_test_wait_destroy_n();
    if (g_stale_want_ui != 0) {
        (void)lmx_msg_exec_bind(rt, addr, turn_recv_end, &g_stale_g2, LMX_MSG_AFFINITY_UI);
    } else {
        (void)lmx_msg_exec_bind(rt, addr, turn_recv_end, &g_stale_g2, LMX_MSG_AFFINITY_ANY);
    }
    g_stale_gen = lmx_msg_exec_test_wait_gen(rt, addr);
    g_stale_wh = lmx_msg_exec_test_worker_handle(rt, addr);
    g_stale_launching = lmx_msg_exec_test_launching(rt, addr);
    g_stale_workers = lmx_msg_exec_workers(rt);
}

static LmxMsgRuntime *g_reap_rt;

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

static volatile LONG *g_fair_peer_done;
static LmxMsgAddr g_mail_gate_addr;
static HANDLE g_mail_entered;
static HANDLE g_mail_go;
static LmxMsgAddr g_mail_dest;
static volatile LONG g_mail_gate_armed;
static volatile LONG g_mail_in_send;
static volatile LONG g_mail_gate_any;
static void mail_gate_hook(LmxMsg *m) {
    if (m == 0 || m->addr != g_mail_gate_addr) {
        return;
    }
    if (InterlockedCompareExchange(&g_mail_gate_any, 0, 0) == 0
        && InterlockedCompareExchange(&g_mail_in_send, 0, 0) == 0) {
        return;
    }
    if (InterlockedCompareExchange(&g_mail_gate_armed, 0, 1) != 1) {
        return;
    }
    SetEvent(g_mail_entered);
    (void)WaitForSingleObject(g_mail_go, 5000);
}
static void dest_pin_fail_after_outbox(LmxMsgRuntime *rt, LmxMsg *src, LmxMsgCopy *outb) {
    (void)rt;
    (void)src;
    (void)outb;
    lmx_msg_test_after_outbox_xfer = 0;
    lmx_msg_test_fail_retain = 1;
}
static void dest_stop_after_outbox(LmxMsgRuntime *rt, LmxMsg *src, LmxMsgCopy *outb) {
    LmxMsg *d;
    (void)src;
    lmx_msg_test_after_outbox_xfer = 0;
    if (rt == 0 || outb == 0 || outb->dest_msg == 0) {
        return;
    }
    d = outb->dest_msg;
    lmx_msg_exec_lock(rt);
    d->state = LMX_MSG_STATE_STOPPED;
    lmx_msg_exec_unlock(rt);
}
typedef struct StageJob {
    LmxMsgRuntime *rt;
    LmxMsgAddr from;
    LmxMsgAddr to;
    unsigned id0;
    unsigned id1;
} StageJob;
static LmxMsgAddr g_nself_from;
static LmxMsg *g_sched_drop;
static LmxMsg *g_drive_drop;
static LmxMsgRuntime *g_drive_mail_rt;
static volatile LONG g_drive_exec_ok;
static volatile LONG g_drive_hook_got_go;
/* Stage 3b-9: no reader takes the exec lock inside release_slot's tree window. */
static LmxMsgRuntime *g_rel_rt;
static LmxMsgAddr g_rel_parent;
static LmxMsgAddr g_rel_child;
static volatile LONG g_rel_armed;
static volatile LONG g_rel_reader_in_window;
static volatile LONG g_rel_chain_ok;
static volatile LONG g_rel_child_present;
static volatile LONG g_rel_count;
static HANDLE g_rel_entered;
static HANDLE g_rel_done;
static void release_tree_hook(LmxMsgRuntime *rt, LmxMsg *m) {
    (void)rt;
    if (m == 0 || m->addr != g_rel_child) {
        return;
    }
    if (InterlockedCompareExchange(&g_rel_armed, 0, 1) != 1) {
        return;
    }
    SetEvent(g_rel_entered);
    InterlockedExchange(&g_rel_reader_in_window,
        WaitForSingleObject(g_rel_done, 300) == WAIT_OBJECT_0 ? 1 : 0);
}
static DWORD WINAPI release_tree_reader(void *arg) {
    LmxMsg *pm;
    LmxMsg *ch;
    LmxMsg *last = 0;
    LONG n = 0;
    LONG ok = 1;
    LONG present = 0;
    (void)arg;
    if (WaitForSingleObject(g_rel_entered, 5000) != WAIT_OBJECT_0) {
        return 1;
    }
    lmx_msg_exec_lock(g_rel_rt);
    pm = lmx_msg_find(g_rel_rt, g_rel_parent);
    if (pm == 0) {
        ok = 0;
    } else {
        for (ch = pm->first_child; ch != 0; ch = ch->next_sibling) {
            if (ch->parent_msg != pm) {
                ok = 0;
            }
            if (ch->addr == g_rel_child) {
                present = 1;
            }
            last = ch;
            n += 1;
            if (n > 64) {
                ok = 0;
                break;
            }
        }
        if (pm->last_child != last) {
            ok = 0;
        }
    }
    InterlockedExchange(&g_rel_chain_ok, ok);
    InterlockedExchange(&g_rel_child_present, present);
    InterlockedExchange(&g_rel_count, n);
    SetEvent(g_rel_done);
    lmx_msg_exec_unlock(g_rel_rt);
    return 0;
}
static DWORD WINAPI drive_mail_overlap_helper(void *arg) {
    (void)arg;
    if (WaitForSingleObject(g_mail_entered, 5000) != WAIT_OBJECT_0) {
        SetEvent(g_mail_go);
        return 1;
    }
    if (g_drive_mail_rt != 0) {
        lmx_msg_exec_lock(g_drive_mail_rt);
        InterlockedExchange(&g_drive_exec_ok, 1);
        lmx_msg_exec_unlock(g_drive_mail_rt);
    }
    SetEvent(g_mail_go);
    return 0;
}
static void drive_close_mail_hook(LmxMsg *m) {
    DWORD w;
    if (m == 0 || m->addr != g_mail_gate_addr) {
        return;
    }
    if (InterlockedCompareExchange(&g_mail_gate_armed, 0, 1) != 1) {
        return;
    }
    SetEvent(g_mail_entered);
    w = WaitForSingleObject(g_mail_go, 5000);
    InterlockedExchange(&g_drive_hook_got_go, w == WAIT_OBJECT_0 ? 1 : 0);
}
static void sched_snap_drop_hook(LmxMsgRuntime *rt, LmxMsg *p) {
    lmx_msg_test_after_sched_snap = 0;
    (void)rt;
    if (p != 0 && g_sched_drop != 0) {
        lmx_msg_child_unlink(p, g_sched_drop);
        g_sched_drop = 0;
    }
}
static void drive_snap_drop_hook(LmxMsgRuntime *rt, LmxMsg *p) {
    (void)rt;
    if (p == 0 || g_drive_drop == 0 || g_drive_drop->parent_msg != p) {
        return;
    }
    lmx_msg_test_after_drive_snap = 0;
    lmx_msg_child_unlink(p, g_drive_drop);
    g_drive_drop = 0;
}
static void recv_fail_overlap_hook(LmxMsgRuntime *rt, LmxMsg *m) {
    lmx_msg_test_after_recv_pin = 0;
    if (rt != 0 && m != 0) {
        (void)lmx_msg_fail(rt, m->addr);
    }
}
static void recv_unbind_overlap_hook(LmxMsgRuntime *rt, LmxMsg *m) {
    lmx_msg_test_after_recv_pin = 0;
    if (rt != 0 && m != 0) {
        (void)lmx_msg_exec_unbind(rt, m->addr);
    }
}
static void nself_recv_pin_hook(LmxMsgRuntime *rt, LmxMsg *m) {
    (void)rt;
    (void)m;
    lmx_msg_test_after_recv_pin = 0;
    SetEvent(g_mail_entered);
    (void)WaitForSingleObject(g_mail_go, 5000);
}
static DWORD WINAPI nself_stage_thread(void *arg) {
    volatile LONG *ok = (volatile LONG *)arg;
    if (WaitForSingleObject(g_mail_entered, 5000) != WAIT_OBJECT_0) {
        SetEvent(g_mail_go);
        return 1;
    }
    if (g_reap_rt == 0 || lmx_msg_test_stage(g_reap_rt, g_nself_from, g_mail_dest, 99U) != LMX_MSG_STAGED) {
        SetEvent(g_mail_go);
        return 1;
    }
    if (ok != 0) {
        InterlockedExchange(ok, 1);
    }
    SetEvent(g_mail_go);
    return 0;
}
static DWORD WINAPI stage_prod_thread(void *arg) {
    StageJob *j = (StageJob *)arg;
    if (j == 0 || lmx_msg_test_stage(j->rt, j->from, j->to, j->id0) != LMX_MSG_STAGED
        || lmx_msg_test_stage(j->rt, j->from, j->to, j->id1) != LMX_MSG_STAGED) {
        return 1;
    }
    return 0;
}
static unsigned g_fifo_ids[8];
static int g_fifo_n;
static void outbox_fifo_hook(LmxMsgRuntime *rt, LmxMsg *src, LmxMsgCopy *outb) {
    LmxMsgCopy *n;
    (void)rt;
    (void)src;
    g_fifo_n = 0;
    n = outb;
    while (n != 0 && g_fifo_n < 8) {
        g_fifo_ids[g_fifo_n] = n->id;
        g_fifo_n += 1;
        n = n->next;
    }
    lmx_msg_test_after_outbox_xfer = 0;
}
static int turn_send_two(LmxMsgRuntime *rt, LmxMsgAddr who, void *ctx) {
    TurnCtx *c = (TurnCtx *)ctx;
    LmxMsgEnv e;
    uchar b = 1;
    memset(&e, 0, sizeof(e));
    (void)lmx_msg_recv(rt, who, &e);
    lmx_msg_env_release(&e);
    memset(&e, 0, sizeof(e));
    e.kind = LMX_MSG_KIND_BYTES;
    e.n = 1;
    e.bytes = &b;
    e.id = 11U;
    (void)lmx_msg_send(rt, who, g_mail_dest, &e);
    e.id = 22U;
    (void)lmx_msg_send(rt, who, g_mail_dest, &e);
    InterlockedIncrement(&c->done);
    return lmx_msg_end_turn(rt, who, 1);
}
static int turn_send_once(LmxMsgRuntime *rt, LmxMsgAddr who, void *ctx) {
    TurnCtx *c = (TurnCtx *)ctx;
    LmxMsgEnv e;
    uchar b = 1;
    memset(&e, 0, sizeof(e));
    (void)lmx_msg_recv(rt, who, &e);
    lmx_msg_env_release(&e);
    memset(&e, 0, sizeof(e));
    e.kind = LMX_MSG_KIND_BYTES;
    e.n = 1;
    e.bytes = &b;
    e.id = 7U;
    InterlockedExchange(&g_mail_in_send, 1);
    (void)lmx_msg_send(rt, who, g_mail_dest, &e);
    InterlockedExchange(&g_mail_in_send, 0);
    InterlockedIncrement(&c->done);
    return lmx_msg_end_turn(rt, who, 1);
}
static int turn_hold_until_peer(LmxMsgRuntime *rt, LmxMsgAddr who, void *ctx) {
    TurnCtx *c = (TurnCtx *)ctx;
    DWORD dl;
    InterlockedIncrement(&c->done);
    dl = GetTickCount() + 2000;
    while (g_fair_peer_done != 0 && InterlockedCompareExchange(g_fair_peer_done, 0, 0) == 0
        && GetTickCount() < dl) {
        Sleep(5);
    }
    return lmx_msg_end_turn(rt, who, 1);
}

/* Decision 18: an ANY Message's readiness is its own flag; a check names the Messages it
 * expects ready and reads each one's flag. */
static int any_queued_n(LmxMsgRuntime *rt, const LmxMsgAddr *a, int n) {
    int k;
    int q = 0;
    for (k = 0; k < n; k++) {
        if (a[k] != 0U && lmx_msg_exec_map_queued(rt, a[k]) != 0) {
            q += 1;
        }
    }
    return q;
}

static int any_queued2(LmxMsgRuntime *rt, LmxMsgAddr a, LmxMsgAddr b) {
    LmxMsgAddr two[2];
    two[0] = a;
    two[1] = b;
    return any_queued_n(rt, two, 2);
}

static int turn_recv_end(LmxMsgRuntime *rt, LmxMsgAddr who, void *ctx) {
    TurnCtx *c = (TurnCtx *)ctx;
    LmxMsgEnv got;
    memset(&got, 0, sizeof(got));
    (void)lmx_msg_recv(rt, who, &got);
    lmx_msg_env_release(&got);
    InterlockedIncrement(&c->done);
    return lmx_msg_end_turn(rt, who, 1);
}

static int turn_end_complete(LmxMsgRuntime *rt, LmxMsgAddr who, void *ctx) {
    TurnCtx *c = (TurnCtx *)ctx;
    int st;
    InterlockedIncrement(&c->done);
    st = lmx_msg_end_turn(rt, who, 1);
    if (st != LMX_MSG_OK) {
        return st;
    }
    return lmx_msg_complete(rt, who);
}

typedef struct NestUsers {
    LmxMsgAddr p;
    LmxMsgAddr c;
    int p_during;
    int c_during;
    int p_after;
    int c_after;
} NestUsers;

static int turn_fail_end(LmxMsgRuntime *rt, LmxMsgAddr who, void *ctx) {
    (void)ctx;
    return lmx_msg_end_turn(rt, who, 0);
}

static int turn_child_users(LmxMsgRuntime *rt, LmxMsgAddr who, void *ctx) {
    NestUsers *n = (NestUsers *)ctx;
    n->p_during = lmx_msg_native_users(rt, n->p);
    n->c_during = lmx_msg_native_users(rt, who);
    return 0;
}

static int turn_parent_nested(LmxMsgRuntime *rt, LmxMsgAddr who, void *ctx) {
    NestUsers *n = (NestUsers *)ctx;
    (void)who;
    (void)lmx_msg_run_child_turn(rt, n->c);
    n->p_after = lmx_msg_native_users(rt, n->p);
    n->c_after = lmx_msg_native_users(rt, n->c);
    return 0;
}

typedef struct OwnSend {
    LmxMsgAddr dest;
    void *orig;
    void *got;
} OwnSend;

static int turn_owned_send(LmxMsgRuntime *rt, LmxMsgAddr who, void *ctx) {
    OwnSend *o = (OwnSend *)ctx;
    LmxMsgEnv env;
    uchar *b = (uchar *)malloc(4U);
    if (b == 0) {
        return 1;
    }
    b[0] = 1;
    b[1] = 2;
    b[2] = 3;
    b[3] = 4;
    o->orig = b;
    memset(&env, 0, sizeof(env));
    env.kind = LMX_MSG_KIND_BYTES;
    env.n = 4;
    env.bytes = 0;
    if (lmx_msg_send_owned(rt, who, o->dest, &env) != LMX_MSG_INVALID || env.n != 4 || env.bytes != 0) {
        return 1;
    }
    env.kind = 99;
    env.n = 0;
    if (lmx_msg_send_owned(rt, who, o->dest, &env) != LMX_MSG_INVALID) {
        return 1;
    }
    env.kind = LMX_MSG_KIND_BYTES;
    env.n = 4;
    env.bytes = b;
    if (lmx_msg_send_owned(rt, who, o->dest, &env) != LMX_MSG_STAGED || env.bytes != 0) {
        return 1;
    }
    return lmx_msg_end_turn(rt, who, 1) == LMX_MSG_OK ? 0 : 1;
}

static int turn_owned_recv(LmxMsgRuntime *rt, LmxMsgAddr who, void *ctx) {
    OwnSend *o = (OwnSend *)ctx;
    LmxMsgEnv got;
    memset(&got, 0, sizeof(got));
    if (lmx_msg_recv(rt, who, &got) != LMX_MSG_OK) {
        return 1;
    }
    o->got = (void *)got.bytes;
    if (o->got != o->orig) {
        lmx_msg_env_release(&got);
        return 1;
    }
    lmx_msg_env_release(&got);
    return 0;
}

/* Decision 17 orphan case: the turn takes its message, reports that it is
 * inside, waits for go, then completes and ends its turn. */
typedef struct OrphanGate {
    LONG entered;
    LONG go;
} OrphanGate;

static int turn_orphan_gate(LmxMsgRuntime *rt, LmxMsgAddr who, void *ctx) {
    OrphanGate *g = (OrphanGate *)ctx;
    LmxMsgEnv got;
    DWORD dl;
    memset(&got, 0, sizeof(got));
    if (lmx_msg_recv(rt, who, &got) == LMX_MSG_OK) {
        lmx_msg_env_release(&got);
    }
    InterlockedIncrement(&g->entered);
    dl = GetTickCount() + 5000;
    while (InterlockedCompareExchange(&g->go, 0, 0) == 0 && GetTickCount() < dl) {
        Sleep(2);
    }
    if (lmx_msg_complete(rt, who) != LMX_MSG_OK) {
        return 1;
    }
    return lmx_msg_end_turn(rt, who, 1) == LMX_MSG_OK ? 0 : 1;
}

/* Decision 18 cursor case: the parent's own turn takes its input and runs its
 * scheduler step twice, recording which child each step ran (1 A, 2 B, 0 none). */
typedef struct StepCtx {
    TurnCtx *a;
    TurnCtx *b;
    int order[4];
    int n;
    int st;
} StepCtx;

static int turn_parent_steps(LmxMsgRuntime *rt, LmxMsgAddr who, void *ctx) {
    StepCtx *s = (StepCtx *)ctx;
    LmxMsgEnv got;
    int k;
    memset(&got, 0, sizeof(got));
    if (lmx_msg_recv(rt, who, &got) == LMX_MSG_OK) {
        lmx_msg_env_release(&got);
    }
    for (k = 0; k < 2 && s->n < 4; k++) {
        long a0 = InterlockedCompareExchange(&s->a->done, 0, 0);
        long b0 = InterlockedCompareExchange(&s->b->done, 0, 0);
        int st = lmx_msg_sched_step(rt, who);
        if (st != LMX_MSG_OK) {
            s->st = st;
            break;
        }
        s->order[s->n] = InterlockedCompareExchange(&s->a->done, 0, 0) > a0 ? 1
            : (InterlockedCompareExchange(&s->b->done, 0, 0) > b0 ? 2 : 0);
        s->n += 1;
    }
    return lmx_msg_end_turn(rt, who, 1) == LMX_MSG_OK ? 0 : 1;
}

static int turn_complete_self(LmxMsgRuntime *rt, LmxMsgAddr who, void *ctx) {
    NestUsers *n = (NestUsers *)ctx;
    n->p_during = lmx_msg_native_users(rt, who);
    if (lmx_msg_complete(rt, who) != LMX_MSG_OK) {
        return 1;
    }
    n->p_after = lmx_msg_adopted_n(rt, who);
    return 0;
}

static int turn_bind_then_omit(LmxMsgRuntime *rt, LmxMsgAddr who, void *ctx) {
    SpawnRec *s = (SpawnRec *)ctx;
    LmxMsgEnv got;
    uchar ini = 1;
    int end_st;
    memset(&got, 0, sizeof(got));
    if (lmx_msg_recv(rt, who, &got) != LMX_MSG_OK) {
        lmx_msg_env_release(&got);
        InterlockedIncrement(&s->done);
        return 1;
    }
    lmx_msg_env_release(&got);
    s->st = lmx_msg_create(rt, who, 9, &ini, 1, &s->child);
    if (s->st == LMX_MSG_OK) {
        s->st = lmx_msg_exec_bind(rt, s->child, turn_just_end, &g_hook_ctx, LMX_MSG_AFFINITY_ANY);
    }
    end_st = lmx_msg_end_turn(rt, who, 0);
    if (s->st == LMX_MSG_OK) {
        s->st = end_st;
    }
    InterlockedIncrement(&s->done);
    return end_st;
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

typedef struct AccRec {
    int value;
    unsigned apply[8];
    int n_apply;
    LmxMsgAddr m0;
    uchar delta;
    volatile LONG done;
    LONG in_turn;
    volatile LONG overlap;
} AccRec;

static int turn_m0_add(LmxMsgRuntime *rt, LmxMsgAddr who, void *ctx) {
    AccRec *a = (AccRec *)ctx;
    LmxMsgEnv got;
    unsigned d;
    memset(&got, 0, sizeof(got));
    if (InterlockedIncrement(&a->in_turn) != 1) {
        InterlockedExchange(&a->overlap, 1);
        return 1;
    }
    if (lmx_msg_recv(rt, who, &got) != LMX_MSG_OK || got.n != 1 || got.bytes == 0) {
        InterlockedDecrement(&a->in_turn);
        lmx_msg_env_release(&got);
        return 1;
    }
    d = got.bytes[0];
    lmx_msg_env_release(&got);
    a->value = a->value + (int)d;
    if (a->n_apply < 8) {
        a->apply[a->n_apply] = d;
    }
    a->n_apply = a->n_apply + 1;
    InterlockedDecrement(&a->in_turn);
    if (lmx_msg_end_turn(rt, who, 1) != LMX_MSG_OK) {
        return 1;
    }
    InterlockedIncrement(&a->done);
    return 0;
}

static int turn_send_delta(LmxMsgRuntime *rt, LmxMsgAddr who, void *ctx) {
    AccRec *a = (AccRec *)ctx;
    LmxMsgEnv got;
    LmxMsgEnv out;
    memset(&got, 0, sizeof(got));
    lmx_msg_recv(rt, who, &got);
    lmx_msg_env_release(&got);
    memset(&out, 0, sizeof(out));
    out.kind = LMX_MSG_KIND_BYTES;
    out.n = 1;
    out.bytes = &a->delta;
    if (lmx_msg_send(rt, who, a->m0, &out) != LMX_MSG_STAGED) {
        return 1;
    }
    if (lmx_msg_end_turn(rt, who, 1) != LMX_MSG_OK) {
        return 1;
    }
    InterlockedIncrement(&a->done);
    return 0;
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

static int turn_rendez(LmxMsgRuntime *rt, LmxMsgAddr who, void *ctx);

static int turn_mix_parent(LmxMsgRuntime *rt, LmxMsgAddr who, void *ctx) {
    MixRec *m = (MixRec *)ctx;
    LmxMsgEnv got;
    LmxMsgEnv env;
    uchar ini = 1;
    LmxMsgAddr c1 = 0, c2 = 0;
    memset(&got, 0, sizeof(got));
    memset(&env, 0, sizeof(env));
    m->tid = GetCurrentThreadId();
    if (lmx_msg_recv(rt, who, &got) != LMX_MSG_OK) {
        lmx_msg_env_release(&got);
        return 1;
    }
    lmx_msg_env_release(&got);
    if (lmx_msg_create(rt, who, 11, &ini, 1, &c1) != LMX_MSG_OK || lmx_msg_create(rt, who, 12, &ini, 1, &c2) != LMX_MSG_OK) {
        return 1;
    }
    if (lmx_msg_exec_bind(rt, c1, turn_rendez, m->a1, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK || lmx_msg_exec_bind(rt, c2, turn_rendez, m->a2, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK) {
        return 1;
    }
    m->st = lmx_msg_map_child(rt, who, c1);
    if (m->st != LMX_MSG_OK || lmx_msg_map_child(rt, who, c2) != LMX_MSG_OK) {
        return 1;
    }
    env.kind = LMX_MSG_KIND_BYTES;
    env.n = 1;
    env.bytes = &ini;
    if (lmx_msg_send(rt, who, c1, &env) != LMX_MSG_STAGED || lmx_msg_send(rt, who, c2, &env) != LMX_MSG_STAGED) {
        return 1;
    }
    InterlockedIncrement(&m->done);
    return lmx_msg_end_turn(rt, who, 1);
}

static int turn_rendez(LmxMsgRuntime *rt, LmxMsgAddr who, void *ctx) {
    RendezRec *c = (RendezRec *)ctx;
    LmxMsgEnv got;
    memset(&got, 0, sizeof(got));
    c->recv_st = lmx_msg_recv(rt, who, &got);
    if (c->recv_st != LMX_MSG_OK) {
        lmx_msg_env_release(&got);
        return 1;
    }
    lmx_msg_env_release(&got);
    c->tid = GetCurrentThreadId();
    InterlockedIncrement(&c->in_turn);
    SetEvent(c->entered);
    if (c->peer == 0 || WaitForSingleObject(c->peer, 2000) != WAIT_OBJECT_0) {
        InterlockedDecrement(&c->in_turn);
        return 1;
    }
    Sleep(20);
    InterlockedDecrement(&c->in_turn);
    InterlockedIncrement(&c->done);
    return lmx_msg_end_turn(rt, who, 1);
}

static int turn_live_wait(LmxMsgRuntime *rt, LmxMsgAddr who, void *ctx) {
    TurnCtx *c = (TurnCtx *)ctx;
    LmxMsgEnv got;
    memset(&got, 0, sizeof(got));
    lmx_msg_recv(rt, who, &got);
    lmx_msg_env_release(&got);
    if (InterlockedCompareExchange(&c->done, 0, 0) == 0) {
        if (lmx_msg_live_query(rt, who) != LMX_MSG_STAGED) {
            return 1;
        }
    }
    InterlockedIncrement(&c->done);
    return lmx_msg_end_turn(rt, who, 1);
}

static int turn_tid(LmxMsgRuntime *rt, LmxMsgAddr who, void *ctx) {
    TurnCtx *c = (TurnCtx *)ctx;
    LmxMsgEnv got;
    memset(&got, 0, sizeof(got));
    c->t0 = GetCurrentThreadId();
    if (lmx_msg_recv(rt, who, &got) != LMX_MSG_OK) {
        lmx_msg_env_release(&got);
        return 1;
    }
    lmx_msg_env_release(&got);
    InterlockedIncrement(&c->done);
    return lmx_msg_end_turn(rt, who, 1);
}

static int turn_spawn(LmxMsgRuntime *rt, LmxMsgAddr who, void *ctx) {
    SpawnRec *s = (SpawnRec *)ctx;
    LmxMsgEnv got;
    LmxMsgEnv env;
    uchar ini = 1;
    uchar b = 9;
    memset(&got, 0, sizeof(got));
    memset(&env, 0, sizeof(env));
    if (lmx_msg_recv(rt, who, &got) != LMX_MSG_OK) {
        lmx_msg_env_release(&got);
        return 1;
    }
    lmx_msg_env_release(&got);
    if (lmx_msg_create(rt, who, 9, &ini, 1, &s->child) != LMX_MSG_OK || s->child == 0) {
        return 1;
    }
    s->child_rec->expect = 9;
    s->st = lmx_msg_exec_bind(rt, s->child, turn_mass, s->child_rec, LMX_MSG_AFFINITY_ANY);
    if (s->st != LMX_MSG_OK) {
        return 1;
    }
    env.kind = LMX_MSG_KIND_BYTES;
    env.n = 1;
    env.bytes = &b;
    if (lmx_msg_send(rt, who, s->child, &env) != LMX_MSG_STAGED) {
        return 1;
    }
    InterlockedIncrement(&s->done);
    return lmx_msg_end_turn(rt, who, 1);
}

static int turn_try_ui(LmxMsgRuntime *rt, LmxMsgAddr who, void *ctx) {
    TryUiRec *c = (TryUiRec *)ctx;
    LmxMsgEnv got;
    memset(&got, 0, sizeof(got));
    if (lmx_msg_recv(rt, who, &got) != LMX_MSG_OK) {
        lmx_msg_env_release(&got);
        return 1;
    }
    lmx_msg_env_release(&got);
    c->st = lmx_msg_exec_bind(rt, c->other, turn_tid, c->other_ctx, LMX_MSG_AFFINITY_UI);
    c->aff_after = lmx_msg_exec_bind_aff(rt, c->other);
    InterlockedIncrement(&c->done);
    return lmx_msg_end_turn(rt, who, 1);
}

static int turn_spawn_race(LmxMsgRuntime *rt, LmxMsgAddr who, void *ctx) {
    SpawnRec *s = (SpawnRec *)ctx;
    LmxMsgEnv got;
    LmxMsgEnv env;
    uchar ini = 1;
    uchar b = 9;
    memset(&got, 0, sizeof(got));
    memset(&env, 0, sizeof(env));
    if (lmx_msg_recv(rt, who, &got) != LMX_MSG_OK) {
        lmx_msg_env_release(&got);
        return 1;
    }
    lmx_msg_env_release(&got);
    if (s->started != 0) {
        SetEvent(s->started);
    }
    if (s->go != 0 && WaitForSingleObject(s->go, 2000) != WAIT_OBJECT_0) {
        return 1;
    }
    if (lmx_msg_create(rt, who, 9, &ini, 1, &s->child) != LMX_MSG_OK) {
        return 1;
    }
    s->child_rec->expect = 9;
    s->st = lmx_msg_exec_bind(rt, s->child, turn_mass, s->child_rec, LMX_MSG_AFFINITY_ANY);
    if (s->st == LMX_MSG_OK) {
        env.kind = LMX_MSG_KIND_BYTES;
        env.n = 1;
        env.bytes = &b;
        if (lmx_msg_send(rt, who, s->child, &env) != LMX_MSG_STAGED) {
            return 1;
        }
    }
    InterlockedIncrement(&s->done);
    return lmx_msg_end_turn(rt, who, 1);
}

static void hook_bind_extra(LmxMsgRuntime *rt) {
    lmx_msg_exec_test_after_bind_add = 0;
    if (g_hook_extra != 0) {
        lmx_msg_exec_bind(rt, g_hook_extra, turn_just_end, &g_hook_ctx, LMX_MSG_AFFINITY_ANY);
    }
}

/* Test-only owned payload: embedded range then aligned payload. Not a product constructor. */
static void *test_owned_prepare(size_t count, size_t stride, int kind, int type,
    LmxMsgBlock **blocks, LmxOwnedRange **ranges)
{
    size_t offset;
    size_t padding;
    size_t bytes;
    char *allocation;
    void *payload;
    LmxMsgBlock *block;
    LmxOwnedRange *range;
    if (count == 0 || stride == 0 || blocks == 0 || ranges == 0) {
        return 0;
    }
    offset = sizeof(LmxOwnedRange);
    padding = (stride - (offset % stride)) % stride;
    bytes = offset + padding + count * stride;
    block = (LmxMsgBlock *)malloc(sizeof(LmxMsgBlock));
    if (block == 0) {
        return 0;
    }
    allocation = (char *)malloc(bytes);
    if (allocation == 0) {
        free(block);
        return 0;
    }
    payload = allocation + offset + padding;
    memset(payload, 0, count * stride);
    range = (LmxOwnedRange *)allocation;
    range->lo = payload;
    range->hi = allocation + bytes;
    range->stride = stride;
    range->kind = kind;
    range->type = type;
    range->next = *ranges;
    block->base = allocation;
    block->n = bytes;
    block->class = (unsigned)type;
    block->dispose = 0;
    block->next = *blocks;
    *ranges = range;
    *blocks = block;
    return payload;
}

int main(int argc, char **argv) {
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

    (void)argv;
    if (argc != 1) {
        fprintf(stderr, "usage: exec_selftest\n");
        return 2;
    }

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
    fprintf(stderr, "starting workers\n");
    fflush(stderr);
    if (lmx_msg_exec_start_contexts(rt) != LMX_MSG_OK) {
        fprintf(stderr, "exec_start\n");
        return 1;
    }
    fprintf(stderr, "workers started, waiting slow\n");
    fflush(stderr);
    fprintf(stderr, "waiting slow\n");
    fflush(stderr);
    if (WaitForSingleObject(slow.started, 5000) != WAIT_OBJECT_0) {
        fprintf(stderr, "slow turn did not start\n");
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
        if (lmx_msg_exec_start_contexts(rtf) != LMX_MSG_OK) {
            fprintf(stderr, "fail exec_start\n");
            return 1;
        }
        fprintf(stderr, "fail waiting cleanup seen=%p\n",
            (void *)g_cleanup_seen);
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
        lmx_msg_exec_start_contexts(rte);
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
        lmx_msg_exec_start_contexts(rto);
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
        if (lmx_msg_exec_start_contexts(rtb) != LMX_MSG_OK) {
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
        if (lmx_msg_exec_start_contexts(rtc) != LMX_MSG_OK) {
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
        LmxMsgRuntime *rtd;
        LmxMsgAddr dummy = 0, p = 0, c1 = 0, c2 = 0;
        uchar ini = 1;
        LmxMsg *pm;
        LmxMsg *cm2;
        rtd = lmx_msg_runtime_new();
        if (rtd == 0 || lmx_msg_create(rtd, 0, 1, &ini, 1, &dummy) != LMX_MSG_OK
            || lmx_msg_create(rtd, dummy, 2, &ini, 1, &p) != LMX_MSG_OK
            || lmx_msg_end_turn(rtd, dummy, 1) != LMX_MSG_OK
            || lmx_msg_create(rtd, p, 3, &ini, 1, &c1) != LMX_MSG_OK
            || lmx_msg_create(rtd, p, 4, &ini, 1, &c2) != LMX_MSG_OK
            || lmx_msg_end_turn(rtd, p, 1) != LMX_MSG_OK) {
            fprintf(stderr, "drive-snap create\n");
            if (rtd != 0) {
                lmx_msg_runtime_delete(rtd);
            }
            return 1;
        }
        pm = lmx_msg_find(rtd, p);
        cm2 = lmx_msg_find(rtd, c2);
        if (pm == 0 || cm2 == 0) {
            fprintf(stderr, "drive-snap find\n");
            lmx_msg_runtime_delete(rtd);
            return 1;
        }
        g_drive_drop = cm2;
        lmx_msg_test_after_drive_snap = drive_snap_drop_hook;
        if (lmx_msg_drive(rtd, 0, 0) != LMX_MSG_OK
            || g_drive_drop != 0) {
            fprintf(stderr, "drive-snap overlap drop_fired=%d\n",
                g_drive_drop == 0);
            lmx_msg_test_after_drive_snap = 0;
            g_drive_drop = 0;
            lmx_msg_runtime_delete(rtd);
            return 1;
        }
        lmx_msg_test_after_drive_snap = 0;
        g_drive_drop = 0;
        fprintf(stderr, "exec wait: drive_tree snap survives sibling unlink overlap\n");
        lmx_msg_runtime_delete(rtd);
    }
    {
        LmxMsgRuntime *rtd;
        LmxMsgAddr dummy = 0, closer = 0;
        LmxMsg *cm;
        HANDLE th;
        DWORD tid;
        uchar ini = 1;
        rtd = lmx_msg_runtime_new();
        g_mail_entered = CreateEventA(0, 1, 0, 0);
        g_mail_go = CreateEventA(0, 1, 0, 0);
        if (rtd == 0 || g_mail_entered == 0 || g_mail_go == 0
            || lmx_msg_create(rtd, 0, 1, &ini, 1, &dummy) != LMX_MSG_OK
            || lmx_msg_end_turn(rtd, dummy, 1) != LMX_MSG_OK
            || lmx_msg_create(rtd, 0, 2, &ini, 1, &closer) != LMX_MSG_OK
            || lmx_msg_end_turn(rtd, closer, 1) != LMX_MSG_OK) {
            fprintf(stderr, "drive-mail create\n");
            if (g_mail_entered != 0) {
                CloseHandle(g_mail_entered);
            }
            if (g_mail_go != 0) {
                CloseHandle(g_mail_go);
            }
            if (rtd != 0) {
                lmx_msg_runtime_delete(rtd);
            }
            return 1;
        }
        cm = lmx_msg_find(rtd, closer);
        if (cm == 0) {
            fprintf(stderr, "drive-mail find\n");
            CloseHandle(g_mail_entered);
            CloseHandle(g_mail_go);
            lmx_msg_runtime_delete(rtd);
            return 1;
        }
        cm->closing = 1;
        g_drive_mail_rt = rtd;
        g_mail_gate_addr = closer;
        InterlockedExchange(&g_drive_exec_ok, 0);
        InterlockedExchange(&g_drive_hook_got_go, 0);
        InterlockedExchange(&g_mail_gate_armed, 1);
        ResetEvent(g_mail_entered);
        ResetEvent(g_mail_go);
        lmx_msg_test_mail_locked = drive_close_mail_hook;
        th = CreateThread(0, 0, drive_mail_overlap_helper, 0, 0, &tid);
        if (th == 0) {
            fprintf(stderr, "drive-mail thread\n");
            lmx_msg_test_mail_locked = 0;
            InterlockedExchange(&g_mail_gate_armed, 0);
            g_drive_mail_rt = 0;
            CloseHandle(g_mail_entered);
            CloseHandle(g_mail_go);
            lmx_msg_runtime_delete(rtd);
            return 1;
        }
        if (lmx_msg_drive(rtd, 0, 0) != LMX_MSG_OK
            || WaitForSingleObject(th, 5000) != WAIT_OBJECT_0
            || InterlockedCompareExchange(&g_drive_exec_ok, 0, 0) != 1
            || InterlockedCompareExchange(&g_drive_hook_got_go, 0, 0) != 1) {
            fprintf(stderr, "drive-mail overlap exec_ok=%ld hook_got_go=%ld\n",
                (long)InterlockedCompareExchange(&g_drive_exec_ok, 0, 0),
                (long)InterlockedCompareExchange(&g_drive_hook_got_go, 0, 0));
            lmx_msg_test_mail_locked = 0;
            InterlockedExchange(&g_mail_gate_armed, 0);
            g_mail_gate_addr = 0;
            g_drive_mail_rt = 0;
            SetEvent(g_mail_go);
            WaitForSingleObject(th, 2000);
            CloseHandle(th);
            CloseHandle(g_mail_entered);
            CloseHandle(g_mail_go);
            lmx_msg_runtime_delete(rtd);
            return 1;
        }
        lmx_msg_test_mail_locked = 0;
        InterlockedExchange(&g_mail_gate_armed, 0);
        g_mail_gate_addr = 0;
        g_drive_mail_rt = 0;
        CloseHandle(th);
        CloseHandle(g_mail_entered);
        CloseHandle(g_mail_go);
        fprintf(stderr, "exec wait: drive close mail does not hold exec\n");
        lmx_msg_runtime_delete(rtd);
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
        if (lmx_msg_exec_start_contexts(rtl) != LMX_MSG_OK) {
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
        LmxMsgRuntime *rtc;
        LmxMsgAddr p = 0, a = 0, b = 0;
        LmxMsgEnv e;
        uchar ini = 1, pa = 2, pb = 3;
        static RendezRec ra;
        static RendezRec rb;
        static MassRec serial;
        DWORD owner_tid = GetCurrentThreadId();
        DWORD dl;
        memset(&ra, 0, sizeof(ra));
        memset(&rb, 0, sizeof(rb));
        memset(&serial, 0, sizeof(serial));
        ra.entered = CreateEventA(0, 1, 0, 0);
        rb.entered = CreateEventA(0, 1, 0, 0);
        ra.peer = rb.entered;
        rb.peer = ra.entered;
        rtc = lmx_msg_runtime_new();
        if (rtc == 0 || ra.entered == 0 || rb.entered == 0 || lmx_msg_create(rtc, 0, 1, &ini, 1, &p) != LMX_MSG_OK) {
            fprintf(stderr, "ctx family setup\n");
            return 1;
        }
        if (lmx_msg_create(rtc, p, 2, &ini, 1, &a) != LMX_MSG_OK || lmx_msg_create(rtc, p, 3, &ini, 1, &b) != LMX_MSG_OK) {
            fprintf(stderr, "ctx children\n");
            return 1;
        }
        if (lmx_msg_exec_bind(rtc, a, turn_rendez, &ra, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK || lmx_msg_exec_bind(rtc, b, turn_rendez, &rb, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK) {
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
        if (lmx_msg_send(rtc, p, a, &e) != LMX_MSG_STAGED) {
            fprintf(stderr, "ctx send a\n");
            return 1;
        }
        e.bytes = &pb;
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
        if (InterlockedCompareExchange(&ra.done, 0, 0) != 1 || InterlockedCompareExchange(&rb.done, 0, 0) != 1 || ra.tid == 0 || rb.tid == 0 || ra.tid == rb.tid) {
            fprintf(stderr, "ctx overlap done a=%ld b=%ld ta=%lu tb=%lu\n",
                (long)InterlockedCompareExchange(&ra.done, 0, 0),
                (long)InterlockedCompareExchange(&rb.done, 0, 0),
                (unsigned long)ra.tid, (unsigned long)rb.tid);
            lmx_msg_exec_stop(rtc);
            lmx_msg_runtime_delete(rtc);
            return 1;
        }
        g_ctx_overlap = 1;
        lmx_msg_exec_stop(rtc);
        InterlockedExchange(&ra.done, 0);
        InterlockedExchange(&rb.done, 0);
        ResetEvent(ra.entered);
        ResetEvent(rb.entered);
        e.bytes = &pa;
        if (lmx_msg_send(rtc, p, a, &e) != LMX_MSG_STAGED || (e.bytes = &pb, lmx_msg_send(rtc, p, b, &e) != LMX_MSG_STAGED)) {
            fprintf(stderr, "ctx restart send\n");
            lmx_msg_runtime_delete(rtc);
            return 1;
        }
        if (lmx_msg_end_turn(rtc, p, 1) != LMX_MSG_OK) {
            fprintf(stderr, "ctx restart publish\n");
            return 1;
        }
        lmx_msg_pump(rtc);
        if (lmx_msg_exec_start_contexts(rtc) != LMX_MSG_OK) {
            fprintf(stderr, "ctx restart start\n");
            lmx_msg_runtime_delete(rtc);
            return 1;
        }
        dl = GetTickCount() + 3000;
        while ((InterlockedCompareExchange(&ra.done, 0, 0) == 0 || InterlockedCompareExchange(&rb.done, 0, 0) == 0) && GetTickCount() < dl) {
            Sleep(10);
        }
        lmx_msg_exec_stop(rtc);
        if (InterlockedCompareExchange(&ra.done, 0, 0) != 1 || InterlockedCompareExchange(&rb.done, 0, 0) != 1) {
            fprintf(stderr, "ctx restart done a=%ld b=%ld\n",
                (long)InterlockedCompareExchange(&ra.done, 0, 0),
                (long)InterlockedCompareExchange(&rb.done, 0, 0));
            lmx_msg_runtime_delete(rtc);
            return 1;
        }
        g_ctx_restart = 1;
        serial.expect = 4;
        if (lmx_msg_exec_bind(rtc, a, turn_mass, &serial, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK) {
            fprintf(stderr, "ctx serial rebind\n");
            lmx_msg_runtime_delete(rtc);
            return 1;
        }
        {
            uchar s1 = 4, s2 = 4;
            e.bytes = &s1;
            if (lmx_msg_send(rtc, p, a, &e) != LMX_MSG_STAGED) {
                fprintf(stderr, "ctx serial send1\n");
                return 1;
            }
            e.bytes = &s2;
            if (lmx_msg_send(rtc, p, a, &e) != LMX_MSG_STAGED) {
                fprintf(stderr, "ctx serial send2\n");
                return 1;
            }
        }
        if (lmx_msg_end_turn(rtc, p, 1) != LMX_MSG_OK) {
            fprintf(stderr, "ctx serial publish\n");
            return 1;
        }
        lmx_msg_pump(rtc);
        if (lmx_msg_exec_start_contexts(rtc) != LMX_MSG_OK) {
            fprintf(stderr, "ctx serial start\n");
            return 1;
        }
        dl = GetTickCount() + 3000;
        while (InterlockedCompareExchange(&serial.done, 0, 0) < 2 && GetTickCount() < dl) {
            Sleep(10);
        }
        lmx_msg_exec_stop(rtc);
        if (InterlockedCompareExchange(&serial.done, 0, 0) != 2 || serial.overlap != 0) {
            fprintf(stderr, "ctx same-context done=%ld overlap=%d\n",
                (long)InterlockedCompareExchange(&serial.done, 0, 0), serial.overlap);
            lmx_msg_runtime_delete(rtc);
            return 1;
        }
        CloseHandle(ra.entered);
        CloseHandle(rb.entered);
        lmx_msg_runtime_delete(rtc);
        (void)owner_tid;
        fprintf(stderr, "ctx_overlap restart serial\n");
        fflush(stderr);
    }
    {
        LmxMsgRuntime *rts;
        LmxMsgAddr p = 0, parent = 0;
        LmxMsgEnv e;
        uchar ini = 1;
        static SpawnRec spawn;
        static MassRec child;
        DWORD dl;
        memset(&spawn, 0, sizeof(spawn));
        memset(&child, 0, sizeof(child));
        spawn.child_rec = &child;
        rts = lmx_msg_runtime_new();
        if (rts == 0 || lmx_msg_create(rts, 0, 1, &ini, 1, &p) != LMX_MSG_OK) {
            fprintf(stderr, "spawn family\n");
            return 1;
        }
        if (lmx_msg_create(rts, p, 2, &ini, 1, &parent) != LMX_MSG_OK) {
            fprintf(stderr, "spawn parent\n");
            return 1;
        }
        if (lmx_msg_end_turn(rts, p, 1) != LMX_MSG_OK) {
            return 1;
        }
        if (lmx_msg_exec_bind(rts, parent, turn_spawn, &spawn, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK) {
            fprintf(stderr, "spawn bind\n");
            return 1;
        }
        memset(&e, 0, sizeof(e));
        e.kind = LMX_MSG_KIND_BYTES;
        e.n = 1;
        e.bytes = &ini;
        if (lmx_msg_send(rts, p, parent, &e) != LMX_MSG_STAGED || lmx_msg_end_turn(rts, p, 1) != LMX_MSG_OK) {
            fprintf(stderr, "spawn send\n");
            return 1;
        }
        lmx_msg_pump(rts);
        if (lmx_msg_exec_start_contexts(rts) != LMX_MSG_OK) {
            fprintf(stderr, "spawn start\n");
            return 1;
        }
        dl = GetTickCount() + 3000;
        while ((InterlockedCompareExchange(&spawn.done, 0, 0) == 0 || InterlockedCompareExchange(&child.done, 0, 0) == 0) && GetTickCount() < dl) {
            Sleep(10);
        }
        lmx_msg_exec_stop(rts);
        if (InterlockedCompareExchange(&spawn.done, 0, 0) != 1 || InterlockedCompareExchange(&child.done, 0, 0) != 1 || spawn.st != LMX_MSG_OK || spawn.child == 0) {
            fprintf(stderr, "spawn child done p=%ld c=%ld st=%d ch=%u\n",
                (long)InterlockedCompareExchange(&spawn.done, 0, 0),
                (long)InterlockedCompareExchange(&child.done, 0, 0), spawn.st, spawn.child);
            lmx_msg_runtime_delete(rts);
            return 1;
        }
        g_ctx_child = 1;
        lmx_msg_runtime_delete(rts);
        fprintf(stderr, "ctx_child_launch\n");
        fflush(stderr);
    }
    {
        LmxMsgRuntime *rtu;
        LmxMsgAddr p = 0, w = 0;
        LmxMsgEnv e;
        uchar ini = 1, b = 5;
        TurnCtx rec;
        DWORD owner = GetCurrentThreadId();
        DWORD dl;
        memset(&rec, 0, sizeof(rec));
        rtu = lmx_msg_runtime_new();
        if (rtu == 0 || lmx_msg_create(rtu, 0, 1, &ini, 1, &p) != LMX_MSG_OK) {
            return 1;
        }
        if (lmx_msg_create(rtu, p, 2, &ini, 1, &w) != LMX_MSG_OK || lmx_msg_end_turn(rtu, p, 1) != LMX_MSG_OK) {
            return 1;
        }
        if (lmx_msg_exec_bind(rtu, w, turn_tid, &rec, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK) {
            fprintf(stderr, "rebind bind\n");
            return 1;
        }
        memset(&e, 0, sizeof(e));
        e.kind = LMX_MSG_KIND_BYTES;
        e.n = 1;
        e.bytes = &ini;
        if (lmx_msg_send(rtu, p, w, &e) != LMX_MSG_STAGED || lmx_msg_end_turn(rtu, p, 1) != LMX_MSG_OK) {
            return 1;
        }
        lmx_msg_pump(rtu);
        if (lmx_msg_exec_start_contexts(rtu) != LMX_MSG_OK) {
            fprintf(stderr, "rebind start\n");
            return 1;
        }
        dl = GetTickCount() + 3000;
        while (InterlockedCompareExchange(&rec.done, 0, 0) == 0 && GetTickCount() < dl) {
            Sleep(10);
        }
        if (InterlockedCompareExchange(&rec.done, 0, 0) != 1 || rec.t0 == owner) {
            fprintf(stderr, "rebind first tid=%lu owner=%lu done=%ld\n",
                (unsigned long)rec.t0, (unsigned long)owner,
                (long)InterlockedCompareExchange(&rec.done, 0, 0));
            lmx_msg_exec_stop(rtu);
            lmx_msg_runtime_delete(rtu);
            return 1;
        }
        if (lmx_msg_exec_bind(rtu, w, turn_tid, &rec, LMX_MSG_AFFINITY_UI) != LMX_MSG_OK) {
            fprintf(stderr, "rebind to ui\n");
            lmx_msg_exec_stop(rtu);
            lmx_msg_runtime_delete(rtu);
            return 1;
        }
        InterlockedExchange(&rec.done, 0);
        rec.t0 = 0;
        e.bytes = &b;
        if (lmx_msg_host_post(rtu, w, &e) != LMX_MSG_STAGED) {
            fprintf(stderr, "rebind post\n");
            return 1;
        }
        if (lmx_msg_exec_ui_step(rtu) != LMX_MSG_OK || InterlockedCompareExchange(&rec.done, 0, 0) != 1 || rec.t0 != owner) {
            fprintf(stderr, "rebind ui tid=%lu owner=%lu done=%ld\n",
                (unsigned long)rec.t0, (unsigned long)owner,
                (long)InterlockedCompareExchange(&rec.done, 0, 0));
            lmx_msg_exec_stop(rtu);
            lmx_msg_runtime_delete(rtu);
            return 1;
        }
        g_ctx_rebind_ui = 1;
        lmx_msg_exec_stop(rtu);
        lmx_msg_runtime_delete(rtu);
        fprintf(stderr, "ctx_rebind_ui\n");
        fflush(stderr);
    }
    {
        LmxMsgRuntime *rtf;
        LmxMsgAddr p = 0, a = 0, b = 0;
        LmxMsgEnv e;
        uchar ini = 1, pa = 2, pb = 3;
        static RendezRec fa;
        static RendezRec fb;
        DWORD dl;
        memset(&fa, 0, sizeof(fa));
        memset(&fb, 0, sizeof(fb));
        fa.entered = CreateEventA(0, 1, 0, 0);
        fb.entered = CreateEventA(0, 1, 0, 0);
        fa.peer = fb.entered;
        fb.peer = fa.entered;
        rtf = lmx_msg_runtime_new();
        if (rtf == 0 || fa.entered == 0 || lmx_msg_create(rtf, 0, 1, &ini, 1, &p) != LMX_MSG_OK) {
            return 1;
        }
        if (lmx_msg_create(rtf, p, 2, &ini, 1, &a) != LMX_MSG_OK || lmx_msg_create(rtf, p, 3, &ini, 1, &b) != LMX_MSG_OK) {
            return 1;
        }
        if (lmx_msg_end_turn(rtf, p, 1) != LMX_MSG_OK) {
            return 1;
        }
        if (lmx_msg_exec_bind(rtf, a, turn_rendez, &fa, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK || lmx_msg_exec_bind(rtf, b, turn_rendez, &fb, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK) {
            return 1;
        }
        memset(&e, 0, sizeof(e));
        e.kind = LMX_MSG_KIND_BYTES;
        e.n = 1;
        e.bytes = &pa;
        if (lmx_msg_send(rtf, p, a, &e) != LMX_MSG_STAGED) {
            return 1;
        }
        e.bytes = &pb;
        if (lmx_msg_send(rtf, p, b, &e) != LMX_MSG_STAGED || lmx_msg_end_turn(rtf, p, 1) != LMX_MSG_OK) {
            return 1;
        }
        lmx_msg_pump(rtf);
        lmx_msg_exec_test_set_fail_ctx(rtf, 2);
        if (lmx_msg_exec_start_contexts(rtf) != LMX_MSG_NOMEM) {
            fprintf(stderr, "ctx fail expected NOMEM after one launch\n");
            lmx_msg_runtime_delete(rtf);
            return 1;
        }
        if (lmx_msg_exec_workers(rtf) != 0) {
            fprintf(stderr, "ctx fail leftover workers=%d\n", lmx_msg_exec_workers(rtf));
            lmx_msg_runtime_delete(rtf);
            return 1;
        }
        lmx_msg_exec_test_set_fail_ctx(rtf, 0);
        if (lmx_msg_exec_start_contexts(rtf) != LMX_MSG_OK) {
            fprintf(stderr, "ctx fail retry start\n");
            lmx_msg_runtime_delete(rtf);
            return 1;
        }
        dl = GetTickCount() + 3000;
        while ((InterlockedCompareExchange(&fa.done, 0, 0) == 0 || InterlockedCompareExchange(&fb.done, 0, 0) == 0) && GetTickCount() < dl) {
            Sleep(10);
        }
        lmx_msg_exec_stop(rtf);
        if (InterlockedCompareExchange(&fa.done, 0, 0) != 1 || InterlockedCompareExchange(&fb.done, 0, 0) != 1) {
            fprintf(stderr, "ctx fail retry done a=%ld b=%ld\n",
                (long)InterlockedCompareExchange(&fa.done, 0, 0),
                (long)InterlockedCompareExchange(&fb.done, 0, 0));
            lmx_msg_runtime_delete(rtf);
            return 1;
        }
        g_ctx_fail_retry = 1;
        CloseHandle(fa.entered);
        CloseHandle(fb.entered);
        lmx_msg_runtime_delete(rtf);
        fprintf(stderr, "ctx_fail_retry\n");
        fflush(stderr);
    }
    {
        LmxMsgRuntime *rta;
        LmxMsgAddr p = 0, a = 0, b = 0;
        LmxMsgEnv e;
        uchar ini = 1;
        TryUiRec tryui;
        TurnCtx other;
        DWORD owner = GetCurrentThreadId();
        DWORD dl;
        memset(&tryui, 0, sizeof(tryui));
        memset(&other, 0, sizeof(other));
        rta = lmx_msg_runtime_new();
        if (rta == 0 || lmx_msg_create(rta, 0, 1, &ini, 1, &p) != LMX_MSG_OK) {
            return 1;
        }
        if (lmx_msg_create(rta, p, 2, &ini, 1, &a) != LMX_MSG_OK || lmx_msg_create(rta, p, 3, &ini, 1, &b) != LMX_MSG_OK) {
            return 1;
        }
        if (lmx_msg_end_turn(rta, p, 1) != LMX_MSG_OK) {
            return 1;
        }
        tryui.other = b;
        tryui.other_ctx = &other;
        if (lmx_msg_exec_bind(rta, a, turn_try_ui, &tryui, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK || lmx_msg_exec_bind(rta, b, turn_tid, &other, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK) {
            return 1;
        }
        memset(&e, 0, sizeof(e));
        e.kind = LMX_MSG_KIND_BYTES;
        e.n = 1;
        e.bytes = &ini;
        if (lmx_msg_send(rta, p, a, &e) != LMX_MSG_STAGED || lmx_msg_end_turn(rta, p, 1) != LMX_MSG_OK) {
            return 1;
        }
        lmx_msg_pump(rta);
        if (lmx_msg_exec_start_contexts(rta) != LMX_MSG_OK) {
            fprintf(stderr, "auth start\n");
            return 1;
        }
        dl = GetTickCount() + 3000;
        while (InterlockedCompareExchange(&tryui.done, 0, 0) == 0 && GetTickCount() < dl) {
            Sleep(10);
        }
        if (tryui.st != LMX_MSG_INVALID || tryui.aff_after != LMX_MSG_AFFINITY_ANY) {
            fprintf(stderr, "auth rebind st=%d aff=%d\n", tryui.st, tryui.aff_after);
            lmx_msg_exec_stop(rta);
            lmx_msg_runtime_delete(rta);
            return 1;
        }
        if (lmx_msg_host_post(rta, b, &e) != LMX_MSG_STAGED) {
            fprintf(stderr, "auth post\n");
            lmx_msg_exec_stop(rta);
            lmx_msg_runtime_delete(rta);
            return 1;
        }
        lmx_msg_host_drain(rta);
        dl = GetTickCount() + 3000;
        while (InterlockedCompareExchange(&other.done, 0, 0) == 0 && GetTickCount() < dl) {
            Sleep(10);
        }
        lmx_msg_exec_stop(rta);
        if (InterlockedCompareExchange(&other.done, 0, 0) != 1 || other.t0 == 0 || other.t0 == owner) {
            fprintf(stderr, "auth other tid=%lu owner=%lu done=%ld\n",
                (unsigned long)other.t0, (unsigned long)owner,
                (long)InterlockedCompareExchange(&other.done, 0, 0));
            lmx_msg_runtime_delete(rta);
            return 1;
        }
        g_ctx_rebind_auth = 1;
        lmx_msg_runtime_delete(rta);
        fprintf(stderr, "ctx_rebind_auth\n");
        fflush(stderr);
    }
    {
        LmxMsgRuntime *rtu;
        LmxMsgAddr p = 0, w = 0;
        LmxMsgEnv e;
        uchar ini = 1;
        TurnCtx rec;
        DWORD dl;
        memset(&rec, 0, sizeof(rec));
        rtu = lmx_msg_runtime_new();
        if (rtu == 0 || lmx_msg_create(rtu, 0, 1, &ini, 1, &p) != LMX_MSG_OK) {
            return 1;
        }
        if (lmx_msg_create(rtu, p, 2, &ini, 1, &w) != LMX_MSG_OK || lmx_msg_end_turn(rtu, p, 1) != LMX_MSG_OK) {
            return 1;
        }
        if (lmx_msg_exec_bind(rtu, w, turn_tid, &rec, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK) {
            return 1;
        }
        memset(&e, 0, sizeof(e));
        e.kind = LMX_MSG_KIND_BYTES;
        e.n = 1;
        e.bytes = &ini;
        if (lmx_msg_send(rtu, p, w, &e) != LMX_MSG_STAGED || lmx_msg_end_turn(rtu, p, 1) != LMX_MSG_OK) {
            return 1;
        }
        lmx_msg_pump(rtu);
        if (lmx_msg_exec_start_contexts(rtu) != LMX_MSG_OK) {
            return 1;
        }
        dl = GetTickCount() + 3000;
        while (InterlockedCompareExchange(&rec.done, 0, 0) == 0 && GetTickCount() < dl) {
            Sleep(10);
        }
        if (lmx_msg_exec_bind(rtu, w, turn_tid, &rec, LMX_MSG_AFFINITY_UI) != LMX_MSG_OK) {
            fprintf(stderr, "ui-any first to ui\n");
            lmx_msg_exec_stop(rtu);
            return 1;
        }
        InterlockedExchange(&rec.done, 0);
        rec.t0 = 0;
        lmx_msg_exec_test_set_fail_ctx(rtu, 1);
        if (lmx_msg_exec_bind(rtu, w, turn_tid, &rec, LMX_MSG_AFFINITY_ANY) != LMX_MSG_NOMEM) {
            fprintf(stderr, "ui-any expected NOMEM\n");
            lmx_msg_exec_stop(rtu);
            lmx_msg_runtime_delete(rtu);
            return 1;
        }
        if (lmx_msg_exec_bind_aff(rtu, w) != LMX_MSG_AFFINITY_UI || lmx_msg_exec_bind_has_worker(rtu, w) != 0) {
            fprintf(stderr, "ui-any rollback aff=%d worker=%d\n",
                lmx_msg_exec_bind_aff(rtu, w), lmx_msg_exec_bind_has_worker(rtu, w));
            lmx_msg_exec_stop(rtu);
            lmx_msg_runtime_delete(rtu);
            return 1;
        }
        lmx_msg_exec_test_set_fail_ctx(rtu, 0);
        if (lmx_msg_exec_bind(rtu, w, turn_tid, &rec, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK || lmx_msg_exec_bind_has_worker(rtu, w) == 0) {
            fprintf(stderr, "ui-any retry\n");
            lmx_msg_exec_stop(rtu);
            lmx_msg_runtime_delete(rtu);
            return 1;
        }
        g_ctx_ui_any_rb = 1;
        lmx_msg_exec_stop(rtu);
        lmx_msg_runtime_delete(rtu);
        fprintf(stderr, "ctx_ui_any_rollback\n");
        fflush(stderr);
    }
    {
        LmxMsgRuntime *rth;
        LmxMsgAddr p = 0, a = 0, c1 = 0, c2 = 0;
        LmxMsgEnv e;
        uchar ini = 1;
        TurnCtx keep;
        int n0;
        int n1;
        memset(&keep, 0, sizeof(keep));
        memset(&g_hook_ctx, 0, sizeof(g_hook_ctx));
        rth = lmx_msg_runtime_new();
        if (rth == 0 || lmx_msg_create(rth, 0, 1, &ini, 1, &p) != LMX_MSG_OK) {
            return 1;
        }
        if (lmx_msg_create(rth, p, 2, &ini, 1, &a) != LMX_MSG_OK || lmx_msg_create(rth, p, 3, &ini, 1, &c1) != LMX_MSG_OK || lmx_msg_create(rth, p, 4, &ini, 1, &c2) != LMX_MSG_OK) {
            return 1;
        }
        if (lmx_msg_end_turn(rth, p, 1) != LMX_MSG_OK) {
            return 1;
        }
        if (lmx_msg_exec_bind(rth, a, turn_just_end, &keep, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK) {
            return 1;
        }
        memset(&e, 0, sizeof(e));
        e.kind = LMX_MSG_KIND_BYTES;
        e.n = 1;
        e.bytes = &ini;
        if (lmx_msg_end_turn(rth, p, 1) != LMX_MSG_OK) {
            return 1;
        }
        if (lmx_msg_exec_start_contexts(rth) != LMX_MSG_OK) {
            fprintf(stderr, "mid start\n");
            return 1;
        }
        n0 = lmx_msg_exec_bind_n(rth);
        g_hook_extra = c2;
        lmx_msg_exec_test_after_bind_add = hook_bind_extra;
        lmx_msg_exec_test_set_fail_ctx(rth, 2);
        if (lmx_msg_exec_bind(rth, c1, turn_just_end, &keep, LMX_MSG_AFFINITY_ANY) != LMX_MSG_NOMEM) {
            fprintf(stderr, "mid expected NOMEM n=%d\n", lmx_msg_exec_bind_n(rth));
            lmx_msg_exec_test_after_bind_add = 0;
            lmx_msg_exec_stop(rth);
            lmx_msg_runtime_delete(rth);
            return 1;
        }
        lmx_msg_exec_test_after_bind_add = 0;
        n1 = lmx_msg_exec_bind_n(rth);
        if (lmx_msg_exec_is_bound(rth, c1) != 0 || lmx_msg_exec_is_bound(rth, c2) == 0 || n1 != n0 + 1) {
            fprintf(stderr, "mid unroll n0=%d n1=%d c1=%d c2=%d\n",
                n0, n1, lmx_msg_exec_is_bound(rth, c1), lmx_msg_exec_is_bound(rth, c2));
            lmx_msg_exec_stop(rth);
            lmx_msg_runtime_delete(rth);
            return 1;
        }
        g_ctx_mid_unroll = 1;
        lmx_msg_exec_stop(rth);
        lmx_msg_runtime_delete(rth);
        fprintf(stderr, "ctx_mid_unroll\n");
        fflush(stderr);
    }
    {
        LmxMsgRuntime *rtr;
        LmxMsgAddr p = 0, parent = 0;
        LmxMsgEnv e;
        uchar ini = 1;
        static SpawnRec spawn;
        static MassRec child;
        DWORD dl;
        memset(&spawn, 0, sizeof(spawn));
        memset(&child, 0, sizeof(child));
        spawn.child_rec = &child;
        spawn.started = CreateEventA(0, 1, 0, 0);
        spawn.go = CreateEventA(0, 1, 0, 0);
        rtr = lmx_msg_runtime_new();
        if (rtr == 0 || spawn.started == 0 || spawn.go == 0 || lmx_msg_create(rtr, 0, 1, &ini, 1, &p) != LMX_MSG_OK) {
            return 1;
        }
        if (lmx_msg_create(rtr, p, 2, &ini, 1, &parent) != LMX_MSG_OK || lmx_msg_end_turn(rtr, p, 1) != LMX_MSG_OK) {
            return 1;
        }
        if (lmx_msg_exec_bind(rtr, parent, turn_spawn_race, &spawn, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK) {
            return 1;
        }
        memset(&e, 0, sizeof(e));
        e.kind = LMX_MSG_KIND_BYTES;
        e.n = 1;
        e.bytes = &ini;
        if (lmx_msg_send(rtr, p, parent, &e) != LMX_MSG_STAGED || lmx_msg_end_turn(rtr, p, 1) != LMX_MSG_OK) {
            return 1;
        }
        lmx_msg_pump(rtr);
        if (lmx_msg_exec_start_contexts(rtr) != LMX_MSG_OK) {
            fprintf(stderr, "race start\n");
            return 1;
        }
        if (WaitForSingleObject(spawn.started, 2000) != WAIT_OBJECT_0) {
            fprintf(stderr, "race started\n");
            lmx_msg_exec_stop(rtr);
            return 1;
        }
        SetEvent(spawn.go);
        lmx_msg_exec_stop(rtr);
        dl = GetTickCount() + 2000;
        while (InterlockedCompareExchange(&spawn.done, 0, 0) == 0 && GetTickCount() < dl) {
            Sleep(10);
        }
        if (InterlockedCompareExchange(&spawn.done, 0, 0) != 1 || lmx_msg_exec_workers(rtr) != 0) {
            fprintf(stderr, "race done=%ld workers=%d st=%d\n",
                (long)InterlockedCompareExchange(&spawn.done, 0, 0),
                lmx_msg_exec_workers(rtr), spawn.st);
            lmx_msg_runtime_delete(rtr);
            return 1;
        }
        g_ctx_spawn_race = 1;
        CloseHandle(spawn.started);
        CloseHandle(spawn.go);
        lmx_msg_runtime_delete(rtr);
        fprintf(stderr, "ctx_spawn_race st=%d\n", spawn.st);
        fflush(stderr);
    }
    {
        LmxMsgRuntime *rtb;
        LmxMsgAddr pb = 0, wb = 0, ub = 0;
        LmxMsgEnv eb;
        uchar ini = 1, hb = 7;
        TurnCtx busy;
        TurnCtx bui;
        DWORD tctx_ui;
        InterlockedExchange(&g_cpu_stop, 0);
        memset(&busy, 0, sizeof(busy));
        memset(&bui, 0, sizeof(bui));
        busy.started = CreateEventA(0, 1, 0, 0);
        rtb = lmx_msg_runtime_new();
        if (rtb == 0 || busy.started == 0) {
            return 1;
        }
        if (lmx_msg_create(rtb, 0, 1, &ini, 1, &pb) != LMX_MSG_OK || lmx_msg_create(rtb, pb, 2, &ini, 1, &wb) != LMX_MSG_OK || lmx_msg_create(rtb, pb, 3, &ini, 1, &ub) != LMX_MSG_OK) {
            return 1;
        }
        if (lmx_msg_end_turn(rtb, pb, 1) != LMX_MSG_OK) {
            return 1;
        }
        if (lmx_msg_exec_bind(rtb, wb, turn_busy, &busy, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK || lmx_msg_exec_bind(rtb, ub, turn_ui, &bui, LMX_MSG_AFFINITY_UI) != LMX_MSG_OK) {
            return 1;
        }
        memset(&eb, 0, sizeof(eb));
        eb.kind = LMX_MSG_KIND_BYTES;
        eb.n = 1;
        eb.bytes = &ini;
        if (lmx_msg_send(rtb, pb, wb, &eb) != LMX_MSG_STAGED || lmx_msg_end_turn(rtb, pb, 1) != LMX_MSG_OK) {
            return 1;
        }
        lmx_msg_pump(rtb);
        if (lmx_msg_exec_start_contexts(rtb) != LMX_MSG_OK) {
            fprintf(stderr, "ctx busy start\n");
            return 1;
        }
        if (WaitForSingleObject(busy.started, 2000) != WAIT_OBJECT_0) {
            fprintf(stderr, "ctx busy turn did not start\n");
            return 1;
        }
        eb.bytes = &hb;
        if (lmx_msg_host_post(rtb, ub, &eb) != LMX_MSG_STAGED) {
            return 1;
        }
        tctx_ui = GetTickCount();
        if (lmx_msg_exec_ui_step(rtb) != LMX_MSG_OK || bui.ui_recvd != 1 || bui.ui_bytes[0] != 7) {
            fprintf(stderr, "ctx ui_step during busy recvd=%u\n", bui.ui_recvd);
            InterlockedExchange(&g_cpu_stop, 1);
            lmx_msg_exec_stop(rtb);
            lmx_msg_runtime_delete(rtb);
            return 1;
        }
        tctx_ui = GetTickCount() - tctx_ui;
        if (InterlockedCompareExchange(&busy.done, 0, 0) != 0 || tctx_ui > 50) {
            fprintf(stderr, "ctx ui blocked or busy already done ms=%lu\n", (unsigned long)tctx_ui);
            InterlockedExchange(&g_cpu_stop, 1);
            lmx_msg_exec_stop(rtb);
            lmx_msg_runtime_delete(rtb);
            return 1;
        }
        InterlockedExchange(&g_cpu_stop, 1);
        {
            DWORD dlb = GetTickCount() + 2000;
            while (InterlockedCompareExchange(&busy.done, 0, 0) == 0 && GetTickCount() < dlb) {
                Sleep(10);
            }
        }
        lmx_msg_exec_stop(rtb);
        CloseHandle(busy.started);
        lmx_msg_runtime_delete(rtb);
        g_ctx_busy_ui = 1;
        fprintf(stderr, "ctx_busy_ui ms=%lu\n", (unsigned long)tctx_ui);
        fflush(stderr);
    }
    {
        LmxMsgRuntime *rtp;
        LmxMsgAddr p = 0, a = 0;
        LmxMsgEnv e;
        uchar ini = 1, b1 = 2, b2 = 3;
        static MixRec mix;
        static RendezRec a1;
        static RendezRec a2;
        DWORD dl;
        DWORD owner = GetCurrentThreadId();
        memset(&mix, 0, sizeof(mix));
        memset(&a1, 0, sizeof(a1));
        memset(&a2, 0, sizeof(a2));
        a1.entered = CreateEventA(0, 1, 0, 0);
        a2.entered = CreateEventA(0, 1, 0, 0);
        a1.peer = a2.entered;
        a2.peer = a1.entered;
        mix.a1 = &a1;
        mix.a2 = &a2;
        rtp = lmx_msg_runtime_new();
        if (rtp == 0 || a1.entered == 0 || lmx_msg_create(rtp, 0, 1, &ini, 1, &p) != LMX_MSG_OK) {
            fprintf(stderr, "mix family\n");
            return 1;
        }
        if (lmx_msg_create(rtp, p, 2, &ini, 1, &a) != LMX_MSG_OK || lmx_msg_end_turn(rtp, p, 1) != LMX_MSG_OK) {
            return 1;
        }
        if (lmx_msg_exec_bind(rtp, a, turn_mix_parent, &mix, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK) {
            fprintf(stderr, "mix bind A\n");
            return 1;
        }
        memset(&e, 0, sizeof(e));
        e.kind = LMX_MSG_KIND_BYTES;
        e.n = 1;
        e.bytes = &ini;
        if (lmx_msg_send(rtp, p, a, &e) != LMX_MSG_STAGED || lmx_msg_end_turn(rtp, p, 1) != LMX_MSG_OK) {
            return 1;
        }
        lmx_msg_pump(rtp);
        dl = GetTickCount() + 3000;
        while (InterlockedCompareExchange(&mix.done, 0, 0) == 0 && GetTickCount() < dl) {
            if (lmx_msg_sched_step(rtp, p) == LMX_MSG_INVALID) {
                Sleep(1);
            }
        }
        dl = GetTickCount() + 3000;
        while ((InterlockedCompareExchange(&a1.done, 0, 0) == 0 || InterlockedCompareExchange(&a2.done, 0, 0) == 0) && GetTickCount() < dl) {
            Sleep(10);
        }
        lmx_msg_exec_stop(rtp);
        if (InterlockedCompareExchange(&mix.done, 0, 0) != 1 || InterlockedCompareExchange(&a1.done, 0, 0) != 1 || InterlockedCompareExchange(&a2.done, 0, 0) != 1 || a1.tid == 0 || a2.tid == 0 || a1.tid == a2.tid || mix.tid == 0 || mix.tid != owner) {
            fprintf(stderr, "mix map done A=%ld a1=%ld a2=%ld ta1=%lu ta2=%lu tA=%lu owner=%lu st=%d\n",
                (long)InterlockedCompareExchange(&mix.done, 0, 0),
                (long)InterlockedCompareExchange(&a1.done, 0, 0),
                (long)InterlockedCompareExchange(&a2.done, 0, 0),
                (unsigned long)a1.tid, (unsigned long)a2.tid,
                (unsigned long)mix.tid, (unsigned long)owner, mix.st);
            lmx_msg_runtime_delete(rtp);
            return 1;
        }
        g_ctx_mix_map = 1;
        CloseHandle(a1.entered);
        CloseHandle(a2.entered);
        lmx_msg_runtime_delete(rtp);
        fprintf(stderr, "ctx_mix_map\n");
        fflush(stderr);
        (void)b1;
        (void)b2;
    }
    {
        LmxMsgRuntime *rtm;
        LmxMsgAddr p = 0, c = 0;
        LmxMsgEnv e;
        uchar ini = 1;
        TurnCtx rec;
        DWORD owner = GetCurrentThreadId();
        DWORD dl;
        memset(&rec, 0, sizeof(rec));
        rtm = lmx_msg_runtime_new();
        if (rtm == 0 || lmx_msg_create(rtm, 0, 1, &ini, 1, &p) != LMX_MSG_OK) {
            return 1;
        }
        if (lmx_msg_create(rtm, p, 2, &ini, 1, &c) != LMX_MSG_OK || lmx_msg_end_turn(rtm, p, 1) != LMX_MSG_OK) {
            return 1;
        }
        if (lmx_msg_exec_bind(rtm, c, turn_tid, &rec, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK) {
            return 1;
        }
        lmx_msg_exec_test_set_fail_ctx(rtm, 1);
        if (lmx_msg_map_child(rtm, p, c) != LMX_MSG_NOMEM) {
            fprintf(stderr, "map fail expected NOMEM\n");
            lmx_msg_runtime_delete(rtm);
            return 1;
        }
        lmx_msg_exec_test_set_fail_ctx(rtm, 0);
        memset(&e, 0, sizeof(e));
        e.kind = LMX_MSG_KIND_BYTES;
        e.n = 1;
        e.bytes = &ini;
        if (lmx_msg_send(rtm, p, c, &e) != LMX_MSG_STAGED || lmx_msg_end_turn(rtm, p, 1) != LMX_MSG_OK) {
            return 1;
        }
        lmx_msg_pump(rtm);
        dl = GetTickCount() + 3000;
        while (InterlockedCompareExchange(&rec.done, 0, 0) == 0 && GetTickCount() < dl) {
            if (lmx_msg_sched_step(rtm, p) == LMX_MSG_INVALID) {
                Sleep(1);
            }
        }
        if (InterlockedCompareExchange(&rec.done, 0, 0) != 1 || rec.t0 != owner) {
            fprintf(stderr, "map fail sequential done=%ld tid=%lu owner=%lu\n",
                (long)InterlockedCompareExchange(&rec.done, 0, 0),
                (unsigned long)rec.t0, (unsigned long)owner);
            lmx_msg_runtime_delete(rtm);
            return 1;
        }
        InterlockedExchange(&rec.done, 0);
        rec.t0 = 0;
        if (lmx_msg_send(rtm, p, c, &e) != LMX_MSG_STAGED || lmx_msg_end_turn(rtm, p, 1) != LMX_MSG_OK) {
            fprintf(stderr, "map retry send/end_turn\n");
            fflush(stderr);
            return 1;
        }
        lmx_msg_pump(rtm);
        {
            int rst = lmx_msg_map_child(rtm, p, c);
            if (rst != LMX_MSG_OK) {
                fprintf(stderr, "map retry\n");
                lmx_msg_exec_stop(rtm);
                lmx_msg_runtime_delete(rtm);
                return 1;
            }
        }
        dl = GetTickCount() + 3000;
        while (InterlockedCompareExchange(&rec.done, 0, 0) == 0 && GetTickCount() < dl) {
            Sleep(10);
        }
        if (InterlockedCompareExchange(&rec.done, 0, 0) != 1 || rec.t0 == 0 || rec.t0 == owner) {
            fprintf(stderr, "map retry delivery tid=%lu owner=%lu done=%ld\n",
                (unsigned long)rec.t0, (unsigned long)owner,
                (long)InterlockedCompareExchange(&rec.done, 0, 0));
            lmx_msg_exec_stop(rtm);
            lmx_msg_runtime_delete(rtm);
            return 1;
        }
        g_ctx_map_fail = 1;
        lmx_msg_exec_stop(rtm);
        lmx_msg_runtime_delete(rtm);
        fprintf(stderr, "ctx_map_fail_retry\n");
        fflush(stderr);
    }
    {
        LmxMsgRuntime *rtl;
        LmxMsgAddr p = 0, c = 0;
        LmxMsgEnv e;
        uchar ini = 1;
        TurnCtx rec;
        DWORD dl;
        memset(&rec, 0, sizeof(rec));
        rtl = lmx_msg_runtime_new();
        if (rtl == 0) {
            return 1;
        }
        if (lmx_msg_set_now(rtl, 1000U) != LMX_MSG_OK) {
            return 1;
        }
        if (lmx_msg_create(rtl, 0, 1, &ini, 1, &p) != LMX_MSG_OK) {
            return 1;
        }
        if (lmx_msg_create(rtl, p, 2, &ini, 1, &c) != LMX_MSG_OK || lmx_msg_end_turn(rtl, p, 1) != LMX_MSG_OK) {
            return 1;
        }
        if (lmx_msg_exec_bind(rtl, c, turn_live_wait, &rec, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK) {
            return 1;
        }
        memset(&e, 0, sizeof(e));
        e.kind = LMX_MSG_KIND_BYTES;
        e.n = 1;
        e.bytes = &ini;
        if (lmx_msg_send(rtl, p, c, &e) != LMX_MSG_STAGED || lmx_msg_end_turn(rtl, p, 1) != LMX_MSG_OK) {
            return 1;
        }
        lmx_msg_pump(rtl);
        if (lmx_msg_live_test_set_wait_th(rtl, c, 5U) != LMX_MSG_OK) {
            return 1;
        }
        if (lmx_msg_map_child(rtl, p, c) != LMX_MSG_OK) {
            fprintf(stderr, "child-timer map\n");
            return 1;
        }
        dl = GetTickCount() + 3000;
        while (InterlockedCompareExchange(&rec.done, 0, 0) == 0 && GetTickCount() < dl) {
            Sleep(10);
        }
        if (lmx_msg_state(rtl, c) == LMX_MSG_STATE_STOPPED) {
            fprintf(stderr, "test-clock expired before deadline\n");
            lmx_msg_exec_stop(rtl);
            lmx_msg_runtime_delete(rtl);
            return 1;
        }
        lmx_msg_set_now(rtl, 1008U);
        dl = GetTickCount() + 3000;
        while (lmx_msg_state(rtl, c) != LMX_MSG_STATE_STOPPED && GetTickCount() < dl) {
            Sleep(10);
        }
        if (lmx_msg_state(rtl, c) != LMX_MSG_STATE_STOPPED) {
            fprintf(stderr, "child own timer state=%d done=%ld\n", lmx_msg_state(rtl, c),
                (long)InterlockedCompareExchange(&rec.done, 0, 0));
            lmx_msg_exec_stop(rtl);
            lmx_msg_runtime_delete(rtl);
            return 1;
        }
        g_ctx_child_timer = 1;
        lmx_msg_exec_stop(rtl);
        lmx_msg_runtime_delete(rtl);
        fprintf(stderr, "ctx_child_timer\n");
        fflush(stderr);
    }
    {
        LmxMsgRuntime *rtr;
        LmxMsgAddr p = 0, c = 0;
        LmxMsgEnv e;
        uchar ini = 1;
        TurnCtx rec;
        DWORD dl;
        memset(&rec, 0, sizeof(rec));
        rtr = lmx_msg_runtime_new();
        if (rtr == 0 || lmx_msg_create(rtr, 0, 1, &ini, 1, &p) != LMX_MSG_OK) {
            return 1;
        }
        if (lmx_msg_create(rtr, p, 2, &ini, 1, &c) != LMX_MSG_OK || lmx_msg_end_turn(rtr, p, 1) != LMX_MSG_OK) {
            return 1;
        }
        if (lmx_msg_exec_bind(rtr, c, turn_live_wait, &rec, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK) {
            return 1;
        }
        memset(&e, 0, sizeof(e));
        e.kind = LMX_MSG_KIND_BYTES;
        e.n = 1;
        e.bytes = &ini;
        if (lmx_msg_send(rtr, p, c, &e) != LMX_MSG_STAGED || lmx_msg_end_turn(rtr, p, 1) != LMX_MSG_OK) {
            return 1;
        }
        lmx_msg_pump(rtr);
        if (lmx_msg_live_test_set_wait_th(rtr, c, 80U) != LMX_MSG_OK || lmx_msg_map_child(rtr, p, c) != LMX_MSG_OK) {
            fprintf(stderr, "real-clock map\n");
            return 1;
        }
        dl = GetTickCount() + 3000;
        while (InterlockedCompareExchange(&rec.done, 0, 0) == 0 && GetTickCount() < dl) {
            Sleep(5);
        }
        Sleep(20);
        if (lmx_msg_state(rtr, c) == LMX_MSG_STATE_STOPPED) {
            fprintf(stderr, "real-clock expired before deadline\n");
            lmx_msg_exec_stop(rtr);
            lmx_msg_runtime_delete(rtr);
            return 1;
        }
        dl = GetTickCount() + 3000;
        while (lmx_msg_state(rtr, c) != LMX_MSG_STATE_STOPPED && GetTickCount() < dl) {
            Sleep(20);
        }
        if (lmx_msg_state(rtr, c) != LMX_MSG_STATE_STOPPED) {
            fprintf(stderr, "real-clock did not expire state=%d\n", lmx_msg_state(rtr, c));
            lmx_msg_exec_stop(rtr);
            lmx_msg_runtime_delete(rtr);
            return 1;
        }
        g_ctx_real_clock = 1;
        lmx_msg_exec_stop(rtr);
        lmx_msg_runtime_delete(rtr);
        fprintf(stderr, "ctx_real_clock\n");
        fflush(stderr);
    }
    {
        LmxMsgRuntime *rtb;
        LmxMsgAddr p = 0, c1 = 0, c2 = 0;
        LmxMsgEnv e;
        LmxMsg *held;
        uchar ini = 1;
        int n0;
        TurnCtx rec;
        memset(&rec, 0, sizeof(rec));
        rtb = lmx_msg_runtime_new();
        if (rtb == 0 || lmx_msg_create(rtb, 0, 1, &ini, 1, &p) != LMX_MSG_OK) {
            return 1;
        }
        if (lmx_msg_create(rtb, p, 2, &ini, 1, &c2) != LMX_MSG_OK || lmx_msg_end_turn(rtb, p, 1) != LMX_MSG_OK) {
            return 1;
        }
        if (lmx_msg_create(rtb, p, 3, &ini, 1, &c1) != LMX_MSG_OK) {
            return 1;
        }
        n0 = rtb->n;
        if (lmx_msg_exec_bind(rtb, c1, turn_just_end, &rec, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK) {
            fprintf(stderr, "rollback bind\n");
            return 1;
        }
        if (lmx_msg_end_turn(rtb, p, 0) != LMX_MSG_OK) {
            return 1;
        }
        if (lmx_msg_find(rtb, c1) != 0 || rtb->n != n0 - 1 || lmx_msg_exec_bind_n(rtb) != 0) {
            fprintf(stderr, "rolled-back bound child not retired n=%d bind=%d\n", rtb->n, lmx_msg_exec_bind_n(rtb));
            lmx_msg_runtime_delete(rtb);
            return 1;
        }
        memset(&e, 0, sizeof(e));
        e.kind = LMX_MSG_KIND_BYTES;
        e.n = 1;
        e.bytes = &ini;
        if (lmx_msg_send(rtb, p, c2, &e) != LMX_MSG_STAGED || lmx_msg_end_turn(rtb, p, 1) != LMX_MSG_OK) {
            fprintf(stderr, "sibling after rollback\n");
            lmx_msg_runtime_delete(rtb);
            return 1;
        }
        if (lmx_msg_exec_stop(rtb) != LMX_MSG_OK) {
            fprintf(stderr, "stop after rollback bind\n");
            lmx_msg_runtime_delete(rtb);
            return 1;
        }
        g_ctx_bind_rollback = 1;
        lmx_msg_runtime_delete(rtb);
        fprintf(stderr, "ctx_bind_rollback\n");
        fflush(stderr);

        /* Stage 3b-9: end_turn(p, 0) releases the uncommitted child c1 with the
         * exec lock dropped. release_slot changes the family tree under the exec
         * lock, so a reader waiting on that lock inside the window (the hook,
         * before child_unlink) gets it only after the chain is whole again. */
        {
            LmxMsgRuntime *rtt;
            LmxMsgAddr tp = 0, tc1 = 0, tc2 = 0;
            HANDLE th;
            rtt = lmx_msg_runtime_new();
            if (rtt == 0 || lmx_msg_create(rtt, 0, 1, &ini, 1, &tp) != LMX_MSG_OK
                || lmx_msg_create(rtt, tp, 2, &ini, 1, &tc2) != LMX_MSG_OK
                || lmx_msg_end_turn(rtt, tp, 1) != LMX_MSG_OK
                || lmx_msg_create(rtt, tp, 3, &ini, 1, &tc1) != LMX_MSG_OK) {
                fprintf(stderr, "release-tree create\n");
                if (rtt != 0) {
                    lmx_msg_runtime_delete(rtt);
                }
                return 1;
            }
            g_rel_entered = CreateEvent(0, TRUE, FALSE, 0);
            g_rel_done = CreateEvent(0, TRUE, FALSE, 0);
            g_rel_rt = rtt;
            g_rel_parent = tp;
            g_rel_child = tc1;
            InterlockedExchange(&g_rel_reader_in_window, 0);
            InterlockedExchange(&g_rel_chain_ok, 0);
            InterlockedExchange(&g_rel_child_present, 1);
            InterlockedExchange(&g_rel_count, -1);
            InterlockedExchange(&g_rel_armed, 1);
            lmx_msg_exec_test_during_release_tree = release_tree_hook;
            th = CreateThread(0, 0, release_tree_reader, 0, 0, 0);
            if (g_rel_entered == 0 || g_rel_done == 0 || th == 0
                || lmx_msg_end_turn(rtt, tp, 0) != LMX_MSG_OK
                || WaitForSingleObject(th, 2000) != WAIT_OBJECT_0
                || InterlockedCompareExchange(&g_rel_reader_in_window, 0, 0) != 0
                || InterlockedCompareExchange(&g_rel_chain_ok, 0, 0) != 1
                || InterlockedCompareExchange(&g_rel_child_present, 0, 0) != 0
                || InterlockedCompareExchange(&g_rel_count, 0, 0) != 1
                || lmx_msg_find(rtt, tc1) != 0) {
                fprintf(stderr, "release-tree window reader_in_window=%ld chain_ok=%ld c1_present=%ld n=%ld\n",
                    (long)InterlockedCompareExchange(&g_rel_reader_in_window, 0, 0),
                    (long)InterlockedCompareExchange(&g_rel_chain_ok, 0, 0),
                    (long)InterlockedCompareExchange(&g_rel_child_present, 0, 0),
                    (long)InterlockedCompareExchange(&g_rel_count, 0, 0));
                lmx_msg_exec_test_during_release_tree = 0;
                InterlockedExchange(&g_rel_armed, 0);
                if (g_rel_entered != 0) {
                    SetEvent(g_rel_entered);
                }
                if (th != 0) {
                    WaitForSingleObject(th, 2000);
                    CloseHandle(th);
                }
                if (g_rel_entered != 0) {
                    CloseHandle(g_rel_entered);
                }
                if (g_rel_done != 0) {
                    CloseHandle(g_rel_done);
                }
                g_rel_rt = 0;
                lmx_msg_runtime_delete(rtt);
                return 1;
            }
            lmx_msg_exec_test_during_release_tree = 0;
            CloseHandle(th);
            CloseHandle(g_rel_entered);
            CloseHandle(g_rel_done);
            g_rel_rt = 0;
            lmx_msg_runtime_delete(rtt);
            fprintf(stderr, "exec wait: release_slot changes the family tree under the exec lock\n");
            fflush(stderr);
        }

        /* Decision 17 rule 4: a supervision handoff moves c's record, its ANY
         * ready membership, its family link and its liveness supervisor from p
         * to q, keeps its path, clears its create_id, and the refusals hold. A
         * failed move may leave the lists inconsistent, so its failure path
         * returns without runtime_delete (whose drop would walk them). */
        {
            LmxMsgRuntime *rtv;
            LmxMsgAddr vp = 0, vq = 0, vc = 0, vg = 0;
            LmxMsg *vpm;
            LmxMsg *vcm;
            LmxMsgEnv venv;
            TurnCtx vctx;
            unsigned seg_before = 0U;
            unsigned seg_after = 0U;
            int path_before;
            int n_ctx = -1;
            int n_map = -1;
            int vst;
            int vturn;
            int vclosing;
            int vstate;
            memset(&vctx, 0, sizeof(vctx));
            memset(&venv, 0, sizeof(venv));
            venv.kind = LMX_MSG_KIND_BYTES;
            venv.n = 1;
            venv.bytes = &ini;
            rtv = lmx_msg_runtime_new();
            if (rtv == 0 || lmx_msg_create(rtv, 0, 1, &ini, 1, &vp) != LMX_MSG_OK
                || lmx_msg_create(rtv, 0, 2, &ini, 1, &vq) != LMX_MSG_OK
                || lmx_msg_create(rtv, vp, 3, &ini, 1, &vc) != LMX_MSG_OK
                || lmx_msg_end_turn(rtv, vp, 1) != LMX_MSG_OK
                || lmx_msg_exec_bind(rtv, vc, turn_live_wait, &vctx, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK
                || lmx_msg_send(rtv, vp, vc, &venv) != LMX_MSG_STAGED
                || lmx_msg_end_turn(rtv, vp, 1) != LMX_MSG_OK
                || lmx_msg_create(rtv, vc, 4, &ini, 1, &vg) != LMX_MSG_OK) {
                fprintf(stderr, "handoff create\n");
                if (rtv != 0) {
                    lmx_msg_runtime_delete(rtv);
                }
                return 1;
            }
            lmx_msg_exec_ready(rtv, vc);
            path_before = lmx_msg_path_n(rtv, vc);
            (void)lmx_msg_path_seg(rtv, vc, 0, &seg_before);
            n_ctx = lmx_msg_exec_bind_n(rtv);
            n_map = lmx_msg_exec_map_queued(rtv, vc);
            if (n_ctx != 1 || n_map != 1) {
                fprintf(stderr, "handoff oracle before move binds=%d ready=%d\n", n_ctx, n_map);
                lmx_msg_runtime_delete(rtv);
                return 1;
            }
            vst = lmx_msg_handoff_supervision(rtv, vp, vc, vq);
            n_ctx = lmx_msg_exec_bind_n(rtv);
            n_map = lmx_msg_exec_map_queued(rtv, vc);
            vcm = lmx_msg_find(rtv, vc);
            vpm = lmx_msg_find(rtv, vp);
            (void)lmx_msg_path_seg(rtv, vc, 0, &seg_after);
            if (vst != LMX_MSG_OK || vcm == 0 || vpm == 0
                || n_ctx != 1 || n_map != 1
                || lmx_msg_child_n(rtv, vp) != 0 || lmx_msg_child_n(rtv, vq) != 1
                || lmx_msg_child_at(rtv, vq, 0) != vc
                || vcm->parent != vq || vcm->parent_msg != lmx_msg_find(rtv, vq)
                || vcm->create_id != 0U
                || lmx_msg_path_n(rtv, vc) != path_before || seg_after != seg_before) {
                fprintf(stderr, "handoff move st=%d binds=%d ready=%d pn=%d qn=%d parent=%u create_id=%u path=%d/%d\n",
                    vst, n_ctx, n_map,
                    lmx_msg_child_n(rtv, vp), lmx_msg_child_n(rtv, vq),
                    vcm != 0 ? (unsigned)vcm->parent : 0U, vcm != 0 ? vcm->create_id : 0U,
                    lmx_msg_path_n(rtv, vc), path_before);
                return 1;
            }
            if (lmx_msg_handoff_supervision(rtv, vp, vc, vq) != LMX_MSG_INVALID
                || lmx_msg_handoff_supervision(rtv, vq, vc, vc) != LMX_MSG_INVALID
                || lmx_msg_handoff_supervision(rtv, vq, vc, vg) != LMX_MSG_INVALID
                || lmx_msg_handoff_supervision(rtv, vp, vq, vc) != LMX_MSG_INVALID) {
                fprintf(stderr, "handoff refusals\n");
                return 1;
            }
            lmx_msg_exec_lock(rtv);
            vstate = vpm->state;
            vpm->state = LMX_MSG_STATE_STOPPED;
            lmx_msg_exec_unlock(rtv);
            (void)lmx_msg_poll(rtv, vc, 1000U, 0U, 0, 0);
            lmx_msg_exec_lock(rtv);
            vclosing = vcm->closing;
            vpm->state = vstate;
            lmx_msg_exec_unlock(rtv);
            vturn = lmx_msg_run_child_turn(rtv, vc);
            lmx_msg_pump(rtv);
            if (vclosing != 0 || (vturn != LMX_MSG_OK && vturn != 1)
                || lmx_msg_inbox_n(rtv, vq) < 1 || lmx_msg_inbox_n(rtv, vp) != 0) {
                fprintf(stderr, "handoff liveness closing=%d turn=%d q_inbox=%d p_inbox=%d\n",
                    vclosing, vturn, lmx_msg_inbox_n(rtv, vq), lmx_msg_inbox_n(rtv, vp));
                return 1;
            }
            lmx_msg_runtime_delete(rtv);
            fprintf(stderr, "exec wait: supervision handoff moves record, ready set, family and liveness supervisor\n");
            fflush(stderr);
        }

        /* Decision 17 with spec 19.29.8: disposing a failed branch settles it
         * bottom-up. R disposes P while P's failed child C is only stopped: C's
         * arena joins P, P's joins R, and both slots are freed, so the slot count
         * drops by two. Finding a Message by address cannot tell: an unlinked
         * branch is unreachable from rt->root whether or not it is retained. A
         * failed settle may leave the lists inconsistent, so its failure path
         * returns without runtime_delete. */
        {
            LmxMsgRuntime *rtq;
            LmxMsgAddr qr = 0, qp = 0, qc = 0;
            int qn0;
            int qadopted0;
            int qst;
            rtq = lmx_msg_runtime_new();
            if (rtq == 0 || lmx_msg_create(rtq, 0, 1, &ini, 1, &qr) != LMX_MSG_OK
                || lmx_msg_create(rtq, qr, 2, &ini, 1, &qp) != LMX_MSG_OK
                || lmx_msg_end_turn(rtq, qr, 1) != LMX_MSG_OK
                || lmx_msg_create(rtq, qp, 3, &ini, 1, &qc) != LMX_MSG_OK
                || lmx_msg_end_turn(rtq, qp, 1) != LMX_MSG_OK
                || lmx_msg_exec_bind(rtq, qp, turn_fail_end, 0, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK
                || lmx_msg_exec_bind(rtq, qc, turn_fail_end, 0, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK
                || lmx_msg_emergency_cancel(rtq, qc) != LMX_MSG_OK
                || lmx_msg_emergency_cancel(rtq, qp) != LMX_MSG_OK) {
                fprintf(stderr, "settle branch create\n");
                if (rtq != 0) {
                    lmx_msg_runtime_delete(rtq);
                }
                return 1;
            }
            (void)lmx_msg_run_child_turn(rtq, qc);
            (void)lmx_msg_run_child_turn(rtq, qp);
            qn0 = rtq->n;
            qadopted0 = lmx_msg_adopted_n(rtq, qr);
            qst = lmx_msg_dispose_child(rtq, qr, qp);
            if (qst != LMX_MSG_OK || lmx_msg_find(rtq, qp) != 0 || lmx_msg_find(rtq, qc) != 0
                || lmx_msg_child_n(rtq, qr) != 0 || rtq->n != qn0 - 2
                || lmx_msg_adopted_n(rtq, qr) < qadopted0 + 2) {
                fprintf(stderr, "settle branch st=%d n=%d n0=%d adopted=%d adopted0=%d\n",
                    qst, rtq->n, qn0, lmx_msg_adopted_n(rtq, qr), qadopted0);
                return 1;
            }
            lmx_msg_runtime_delete(rtq);
            fprintf(stderr, "exec wait: dispose settles a failed branch bottom-up; both slots freed\n");
            fflush(stderr);
        }

        /* Decision 17 with spec 19.29.6 (iii) and 19.29.8: R releases P while
         * P's mapped child C is still inside its turn on its own context. The
         * release returns at once: P's slot is freed and C is re-rooted at the
         * runtime as an orphan, still bound. When C's
         * turn completes, the host's drive reclaims it. rt->n is the oracle;
         * find cannot tell a retained unreachable subtree from a freed one. */
        {
            LmxMsgRuntime *rto;
            LmxMsgAddr orr = 0, op = 0, oc = 0;
            LmxMsgEnv oe;
            OrphanGate og;
            int on0, ost, ocnt = 0, ofp, ofc, on1, ochn;
            DWORD odl;
            memset(&og, 0, sizeof(og));
            memset(&oe, 0, sizeof(oe));
            oe.kind = LMX_MSG_KIND_BYTES;
            oe.n = 1;
            oe.bytes = &ini;
            rto = lmx_msg_runtime_new();
            if (rto == 0 || lmx_msg_create(rto, 0, 1, &ini, 1, &orr) != LMX_MSG_OK
                || lmx_msg_create(rto, orr, 2, &ini, 1, &op) != LMX_MSG_OK
                || lmx_msg_end_turn(rto, orr, 1) != LMX_MSG_OK
                || lmx_msg_create(rto, op, 3, &ini, 1, &oc) != LMX_MSG_OK
                || lmx_msg_send(rto, op, oc, &oe) != LMX_MSG_STAGED
                || lmx_msg_end_turn(rto, op, 1) != LMX_MSG_OK
                || lmx_msg_exec_bind(rto, op, turn_fail_end, 0, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK
                || lmx_msg_exec_bind(rto, oc, turn_orphan_gate, &og, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK) {
                fprintf(stderr, "orphan mapped create\n");
                return 1;
            }
            lmx_msg_pump(rto);
            if (lmx_msg_map_child(rto, op, oc) != LMX_MSG_OK) {
                fprintf(stderr, "orphan mapped map\n");
                return 1;
            }
            odl = GetTickCount() + 3000;
            while (InterlockedCompareExchange(&og.entered, 0, 0) == 0 && GetTickCount() < odl) {
                Sleep(2);
            }
            if (InterlockedCompareExchange(&og.entered, 0, 0) == 0
                || lmx_msg_emergency_cancel(rto, op) != LMX_MSG_OK) {
                fprintf(stderr, "orphan mapped entered=%ld\n", (long)InterlockedCompareExchange(&og.entered, 0, 0));
                InterlockedIncrement(&og.go);
                return 1;
            }
            (void)lmx_msg_run_child_turn(rto, op);
            on0 = rto->n;
            ost = lmx_msg_dispose_child(rto, orr, op);
            ofp = lmx_msg_find(rto, op) != 0;
            ofc = lmx_msg_find(rto, oc) != 0;
            on1 = rto->n;
            ochn = lmx_msg_child_n(rto, orr);
            ocnt = lmx_msg_exec_bind_n(rto);
            InterlockedIncrement(&og.go);
            if (ost != LMX_MSG_OK || ofp != 0 || ofc == 0 || ochn != 0 || on1 != on0 - 1
                || ocnt != 1) {
                fprintf(stderr, "orphan mapped release st=%d find_p=%d find_c=%d child_n=%d n=%d n0=%d binds=%d\n",
                    ost, ofp, ofc, ochn, on1, on0, ocnt);
                return 1;
            }
            odl = GetTickCount() + 5000;
            while (rto->n != on0 - 2 && GetTickCount() < odl) {
                (void)lmx_msg_drive(rto, 0, 0);
                Sleep(5);
            }
            if (rto->n != on0 - 2 || lmx_msg_find(rto, oc) != 0) {
                fprintf(stderr, "orphan mapped reclaim n=%d n0=%d find_c=%d\n",
                    rto->n, on0, lmx_msg_find(rto, oc) != 0);
                return 1;
            }
            lmx_msg_exec_stop(rto);
            lmx_msg_runtime_delete(rto);
            fprintf(stderr, "exec wait: a mapped orphan is re-rooted at release and reclaimed by drive after its turn\n");
            fflush(stderr);
        }

        rtb = lmx_msg_runtime_new();
        p = 0;
        c1 = 0;
        c2 = 0;
        memset(&rec, 0, sizeof(rec));
        if (rtb == 0 || lmx_msg_create(rtb, 0, 1, &ini, 1, &p) != LMX_MSG_OK) {
            return 1;
        }
        if (lmx_msg_create(rtb, p, 2, &ini, 1, &c2) != LMX_MSG_OK || lmx_msg_end_turn(rtb, p, 1) != LMX_MSG_OK) {
            return 1;
        }
        if (lmx_msg_create(rtb, p, 3, &ini, 1, &c1) != LMX_MSG_OK) {
            return 1;
        }
        held = lmx_msg_find(rtb, c1);
        n0 = rtb->n;
        if (held == 0 || lmx_msg_endp_retain(held) == 0) {
            fprintf(stderr, "held retain\n");
            return 1;
        }
        if (lmx_msg_exec_bind(rtb, c1, turn_just_end, &rec, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK) {
            fprintf(stderr, "held bind\n");
            lmx_msg_endp_release(held);
            return 1;
        }
        memset(&e, 0, sizeof(e));
        e.kind = LMX_MSG_KIND_BYTES;
        e.n = 1;
        e.bytes = &ini;
        if (lmx_msg_send_cap(rtb, p, held, &e) != LMX_MSG_STAGED) {
            fprintf(stderr, "held send_cap\n");
            lmx_msg_endp_release(held);
            lmx_msg_runtime_delete(rtb);
            return 1;
        }
        if (lmx_msg_end_turn(rtb, p, 0) != LMX_MSG_OK) {
            lmx_msg_endp_release(held);
            lmx_msg_runtime_delete(rtb);
            return 1;
        }
        if (lmx_msg_find(rtb, c1) != 0 || rtb->n != n0) {
            fprintf(stderr, "held cap retired early n=%d\n", rtb->n);
            lmx_msg_endp_release(held);
            lmx_msg_runtime_delete(rtb);
            return 1;
        }
        lmx_msg_endp_release(held);
        if (lmx_msg_find(rtb, c1) != 0 || rtb->n != n0 - 1) {
            fprintf(stderr, "final held-cap did not retire n=%d\n", rtb->n);
            lmx_msg_runtime_delete(rtb);
            return 1;
        }
        if (lmx_msg_send(rtb, p, c2, &e) != LMX_MSG_STAGED || lmx_msg_end_turn(rtb, p, 1) != LMX_MSG_OK) {
            fprintf(stderr, "sibling after held retire\n");
            lmx_msg_runtime_delete(rtb);
            return 1;
        }
        if (lmx_msg_exec_stop(rtb) != LMX_MSG_OK) {
            fprintf(stderr, "stop after held retire\n");
            lmx_msg_runtime_delete(rtb);
            return 1;
        }
        g_ctx_bind_held = 1;
        lmx_msg_runtime_delete(rtb);
        fprintf(stderr, "ctx_bind_held\n");
        fflush(stderr);

        /* Stage 3a-2 (d6): run_child_turn reads the child's own bind record.
         * After unbind the record stays on the Message (it is freed only in
         * lmx_msg_slot_free) but is no longer a table entry, so it must not
         * admit a turn. A lookup that trusted m->exec_bind without in_table
         * would hold and run the unbound child here. */
        {
            LmxMsgRuntime *rtu;
            LmxMsgAddr pu = 0, cu = 0;
            uchar iniu = 1;
            TurnCtx recu;
            LmxMsg *cmu;
            int stu;
            memset(&recu, 0, sizeof(recu));
            rtu = lmx_msg_runtime_new();
            if (rtu == 0 || lmx_msg_create(rtu, 0, 1, &iniu, 1, &pu) != LMX_MSG_OK
                || lmx_msg_create(rtu, pu, 2, &iniu, 1, &cu) != LMX_MSG_OK) {
                fprintf(stderr, "unbound record setup\n");
                return 1;
            }
            if (lmx_msg_exec_bind(rtu, cu, turn_just_end, &recu, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK) {
                fprintf(stderr, "unbound record bind\n");
                lmx_msg_runtime_delete(rtu);
                return 1;
            }
            if (lmx_msg_exec_unbind(rtu, cu) != LMX_MSG_OK) {
                fprintf(stderr, "unbound record unbind\n");
                lmx_msg_runtime_delete(rtu);
                return 1;
            }
            cmu = lmx_msg_find(rtu, cu);
            stu = lmx_msg_run_child_turn(rtu, cu);
            if (cmu == 0 || cmu->exec_bind == 0 || stu != LMX_MSG_INVALID
                || InterlockedCompareExchange(&recu.done, 0, 0) != 0) {
                fprintf(stderr, "unbound record admitted a turn msg=%p rec=%p st=%d done=%ld\n",
                    (void *)cmu, cmu != 0 ? (void *)cmu->exec_bind : (void *)0, stu,
                    (long)InterlockedCompareExchange(&recu.done, 0, 0));
                lmx_msg_runtime_delete(rtu);
                return 1;
            }
            lmx_msg_runtime_delete(rtu);
            fprintf(stderr, "ctx_unbound_record\n");
            fflush(stderr);
        }

        rtb = lmx_msg_runtime_new();
        p = 0;
        c1 = 0;
        c2 = 0;
        {
            static SpawnRec omit;
            DWORD dl;
            memset(&omit, 0, sizeof(omit));
            omit.rt = rtb;
            if (rtb == 0 || lmx_msg_create(rtb, 0, 1, &ini, 1, &p) != LMX_MSG_OK) {
                return 1;
            }
            if (lmx_msg_create(rtb, p, 2, &ini, 1, &c2) != LMX_MSG_OK || lmx_msg_end_turn(rtb, p, 1) != LMX_MSG_OK) {
                return 1;
            }
            n0 = rtb->n;
            if (lmx_msg_exec_bind(rtb, p, turn_bind_then_omit, &omit, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK) {
                fprintf(stderr, "rollback ctx parent bind\n");
                lmx_msg_runtime_delete(rtb);
                return 1;
            }
            memset(&e, 0, sizeof(e));
            e.kind = LMX_MSG_KIND_BYTES;
            e.n = 1;
            e.bytes = &ini;
            if (lmx_msg_send(rtb, p, p, &e) != LMX_MSG_STAGED || lmx_msg_end_turn(rtb, p, 1) != LMX_MSG_OK) {
                fprintf(stderr, "rollback ctx publish\n");
                lmx_msg_runtime_delete(rtb);
                return 1;
            }
            if (lmx_msg_exec_start_contexts(rtb) != LMX_MSG_OK) {
                fprintf(stderr, "rollback start_contexts\n");
                lmx_msg_runtime_delete(rtb);
                return 1;
            }
            dl = GetTickCount() + 3000;
            while (InterlockedCompareExchange(&omit.done, 0, 0) == 0 && GetTickCount() < dl) {
                Sleep(10);
            }
            Sleep(20);
            c1 = omit.child;
            if (InterlockedCompareExchange(&omit.done, 0, 0) != 1 || omit.st != LMX_MSG_OK || c1 == 0) {
                fprintf(stderr, "rollback ctx turn st=%d child=%u done=%ld\n",
                    omit.st, c1, (long)InterlockedCompareExchange(&omit.done, 0, 0));
                lmx_msg_exec_stop(rtb);
                lmx_msg_runtime_delete(rtb);
                return 1;
            }
            if (lmx_msg_find(rtb, c1) != 0 || rtb->n != n0) {
                fprintf(stderr, "ctx rolled-back child not retired n=%d find=%d\n",
                    rtb->n, lmx_msg_find(rtb, c1) != 0);
                lmx_msg_exec_stop(rtb);
                lmx_msg_runtime_delete(rtb);
                return 1;
            }
            if (lmx_msg_exec_stop(rtb) != LMX_MSG_OK) {
                fprintf(stderr, "stop after ctx rollback\n");
                lmx_msg_runtime_delete(rtb);
                return 1;
            }
            if (lmx_msg_send(rtb, p, c2, &e) != LMX_MSG_STAGED || lmx_msg_end_turn(rtb, p, 1) != LMX_MSG_OK) {
                fprintf(stderr, "sibling after ctx rollback\n");
                lmx_msg_runtime_delete(rtb);
                return 1;
            }
            g_ctx_bind_ctx = 1;
            lmx_msg_runtime_delete(rtb);
            fprintf(stderr, "ctx_bind_ctx\n");
            fflush(stderr);
        }

        rtb = lmx_msg_runtime_new();
        p = 0;
        c1 = 0;
        c2 = 0;
        memset(&rec, 0, sizeof(rec));
        if (rtb == 0 || lmx_msg_create(rtb, 0, 1, &ini, 1, &p) != LMX_MSG_OK) {
            return 1;
        }
        if (lmx_msg_create(rtb, p, 2, &ini, 1, &c2) != LMX_MSG_OK || lmx_msg_end_turn(rtb, p, 1) != LMX_MSG_OK) {
            return 1;
        }
        if (lmx_msg_create(rtb, p, 3, &ini, 1, &c1) != LMX_MSG_OK) {
            return 1;
        }
        if (lmx_msg_exec_bind(rtb, c1, turn_just_end, &rec, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK) {
            return 1;
        }
        {
            DWORD dl = GetTickCount() + 3000;
            if (lmx_msg_end_turn(rtb, p, 0) != LMX_MSG_OK) {
                fprintf(stderr, "idle-rollback end_turn\n");
                lmx_msg_exec_stop(rtb);
                lmx_msg_runtime_delete(rtb);
                return 1;
            }
            if (GetTickCount() > dl) {
                fprintf(stderr, "idle-rollback timeout\n");
                lmx_msg_exec_stop(rtb);
                lmx_msg_runtime_delete(rtb);
                return 1;
            }
        }
        if (lmx_msg_find(rtb, c1) != 0) {
            fprintf(stderr, "idle-rollback child still present\n");
            lmx_msg_exec_stop(rtb);
            lmx_msg_runtime_delete(rtb);
            return 1;
        }
        memset(&e, 0, sizeof(e));
        e.kind = LMX_MSG_KIND_BYTES;
        e.n = 1;
        e.bytes = &ini;
        if (lmx_msg_send(rtb, p, c2, &e) != LMX_MSG_STAGED || lmx_msg_end_turn(rtb, p, 1) != LMX_MSG_OK) {
            fprintf(stderr, "idle-rollback sibling\n");
            lmx_msg_exec_stop(rtb);
            lmx_msg_runtime_delete(rtb);
            return 1;
        }
        if (lmx_msg_exec_stop(rtb) != LMX_MSG_OK) {
            fprintf(stderr, "idle-rollback stop\n");
            lmx_msg_runtime_delete(rtb);
            return 1;
        }
        g_ctx_idle_rollback = 1;
        lmx_msg_runtime_delete(rtb);
        fprintf(stderr, "ctx_idle_rollback\n");
        fflush(stderr);
    }
    {
        LmxMsgRuntime *rtc;
        LmxMsgAddr p = 0, c1 = 0, c2 = 0, g = 0;
        LmxMsgEnv e;
        uchar ini = 1;
        TurnCtx rec;
        DWORD dl;
        LmxMsg *gm;
        memset(&rec, 0, sizeof(rec));
        rtc = lmx_msg_runtime_new();
        if (rtc == 0 || lmx_msg_create(rtc, 0, 1, &ini, 1, &p) != LMX_MSG_OK) {
            return 1;
        }
        if (lmx_msg_create(rtc, p, 2, &ini, 1, &c2) != LMX_MSG_OK || lmx_msg_end_turn(rtc, p, 1) != LMX_MSG_OK) {
            return 1;
        }
        if (lmx_msg_create(rtc, p, 3, &ini, 1, &c1) != LMX_MSG_OK || lmx_msg_end_turn(rtc, p, 1) != LMX_MSG_OK) {
            return 1;
        }
        if (lmx_msg_create(rtc, c1, 4, &ini, 1, &g) != LMX_MSG_OK || lmx_msg_end_turn(rtc, c1, 1) != LMX_MSG_OK) {
            return 1;
        }
        if (lmx_msg_exec_bind(rtc, c1, turn_just_end, &rec, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK) {
            return 1;
        }
        if (lmx_msg_map_child(rtc, p, c1) != LMX_MSG_OK) {
            fprintf(stderr, "cancel-idle map\n");
            lmx_msg_exec_stop(rtc);
            lmx_msg_runtime_delete(rtc);
            return 1;
        }
        if (lmx_msg_emergency_cancel(rtc, c1) != LMX_MSG_OK) {
            fprintf(stderr, "cancel-idle\n");
            lmx_msg_exec_stop(rtc);
            lmx_msg_runtime_delete(rtc);
            return 1;
        }
        dl = GetTickCount() + 3000;
        while (lmx_msg_state(rtc, c1) != LMX_MSG_STATE_STOPPED && GetTickCount() < dl) {
            Sleep(10);
        }
        Sleep(50);
        gm = lmx_msg_find(rtc, g);
        if (lmx_msg_state(rtc, c1) != LMX_MSG_STATE_STOPPED || InterlockedCompareExchange(&rec.done, 0, 0) != 0 || gm == 0 || lmx_msg_running_load(gm) != 0) {
            fprintf(stderr, "cancel-idle state=%d done=%ld g_run=%d\n",
                lmx_msg_state(rtc, c1), (long)InterlockedCompareExchange(&rec.done, 0, 0),
                gm ? (int)lmx_msg_running_load(gm) : -1);
            lmx_msg_exec_stop(rtc);
            lmx_msg_runtime_delete(rtc);
            return 1;
        }
        memset(&e, 0, sizeof(e));
        e.kind = LMX_MSG_KIND_BYTES;
        e.n = 1;
        e.bytes = &ini;
        if (lmx_msg_host_post(rtc, c2, &e) != LMX_MSG_STAGED) {
            fprintf(stderr, "cancel-idle sibling\n");
            lmx_msg_exec_stop(rtc);
            lmx_msg_runtime_delete(rtc);
            return 1;
        }
        g_ctx_cancel_idle = 1;
        lmx_msg_exec_stop(rtc);
        lmx_msg_runtime_delete(rtc);
        fprintf(stderr, "ctx_cancel_idle\n");
        fflush(stderr);
    }
    {
        LmxMsgRuntime *rtp;
        LmxMsgAddr p = 0, c1 = 0, c2 = 0;
        LmxMsgEnv e;
        uchar ini = 1;
        TurnCtx rec;
        DWORD dl;
        memset(&rec, 0, sizeof(rec));
        rtp = lmx_msg_runtime_new();
        if (rtp == 0 || lmx_msg_create(rtp, 0, 1, &ini, 1, &p) != LMX_MSG_OK) {
            return 1;
        }
        if (lmx_msg_create(rtp, p, 2, &ini, 1, &c2) != LMX_MSG_OK || lmx_msg_end_turn(rtp, p, 1) != LMX_MSG_OK) {
            return 1;
        }
        if (lmx_msg_create(rtp, p, 3, &ini, 1, &c1) != LMX_MSG_OK || lmx_msg_end_turn(rtp, p, 1) != LMX_MSG_OK) {
            return 1;
        }
        if (lmx_msg_exec_bind(rtp, c1, turn_just_end, &rec, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK || lmx_msg_map_child(rtp, p, c1) != LMX_MSG_OK) {
            lmx_msg_exec_stop(rtp);
            lmx_msg_runtime_delete(rtp);
            return 1;
        }
        if (lmx_msg_complete(rtp, c1) != LMX_MSG_OK) {
            fprintf(stderr, "complete-idle\n");
            lmx_msg_exec_stop(rtp);
            lmx_msg_runtime_delete(rtp);
            return 1;
        }
        dl = GetTickCount() + 3000;
        while (lmx_msg_state(rtp, c1) != LMX_MSG_STATE_STOPPED && GetTickCount() < dl) {
            Sleep(10);
        }
        Sleep(50);
        if (lmx_msg_state(rtp, c1) != LMX_MSG_STATE_STOPPED || InterlockedCompareExchange(&rec.done, 0, 0) != 0) {
            fprintf(stderr, "complete-idle state=%d done=%ld\n",
                lmx_msg_state(rtp, c1), (long)InterlockedCompareExchange(&rec.done, 0, 0));
            lmx_msg_exec_stop(rtp);
            lmx_msg_runtime_delete(rtp);
            return 1;
        }
        memset(&e, 0, sizeof(e));
        e.kind = LMX_MSG_KIND_BYTES;
        e.n = 1;
        e.bytes = &ini;
        if (lmx_msg_host_post(rtp, c2, &e) != LMX_MSG_STAGED) {
            fprintf(stderr, "complete-idle sibling\n");
            lmx_msg_exec_stop(rtp);
            lmx_msg_runtime_delete(rtp);
            return 1;
        }
        g_ctx_complete_idle = 1;
        lmx_msg_exec_stop(rtp);
        lmx_msg_runtime_delete(rtp);
        fprintf(stderr, "ctx_complete_idle\n");
        fflush(stderr);
    }
    {
        LmxMsgRuntime *rta;
        LmxMsgAddr dummy = 0, m0 = 0, m1 = 0, m2 = 0;
        LmxMsgEnv e;
        uchar ini = 1;
        AccRec acc, s1, s2;
        DWORD dl;
        memset(&acc, 0, sizeof(acc));
        memset(&s1, 0, sizeof(s1));
        memset(&s2, 0, sizeof(s2));
        g_admit_dest = 0;
        g_admit_n = 0;
        memset(g_admit_log, 0, sizeof(g_admit_log));
        rta = lmx_msg_runtime_new();
        if (rta == 0 || lmx_msg_create(rta, 0, 1, &ini, 1, &dummy) != LMX_MSG_OK) {
            return 1;
        }
        if (lmx_msg_create(rta, dummy, 2, &ini, 1, &m0) != LMX_MSG_OK
            || lmx_msg_create(rta, dummy, 3, &ini, 1, &m1) != LMX_MSG_OK
            || lmx_msg_create(rta, dummy, 4, &ini, 1, &m2) != LMX_MSG_OK
            || lmx_msg_end_turn(rta, dummy, 1) != LMX_MSG_OK) {
            return 1;
        }
        acc.m0 = m0;
        s1.m0 = m0;
        s1.delta = 2;
        s2.m0 = m0;
        s2.delta = 5;
        g_admit_dest = m0;
        if (lmx_msg_exec_bind(rta, m0, turn_m0_add, &acc, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK
            || lmx_msg_exec_bind(rta, m1, turn_send_delta, &s1, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK
            || lmx_msg_exec_bind(rta, m2, turn_send_delta, &s2, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK) {
            return 1;
        }
        if (lmx_msg_map_child(rta, dummy, m1) != LMX_MSG_OK
            || lmx_msg_map_child(rta, dummy, m2) != LMX_MSG_OK) {
            fprintf(stderr, "m0 map senders\n");
            lmx_msg_runtime_delete(rta);
            return 1;
        }
        memset(&e, 0, sizeof(e));
        e.kind = LMX_MSG_KIND_BYTES;
        e.n = 1;
        e.bytes = &ini;
        if (lmx_msg_host_post(rta, m1, &e) != LMX_MSG_STAGED || lmx_msg_host_post(rta, m2, &e) != LMX_MSG_STAGED) {
            fprintf(stderr, "m0 sender post\n");
            lmx_msg_exec_stop(rta);
            lmx_msg_runtime_delete(rta);
            return 1;
        }
        if (lmx_msg_host_drain(rta) != LMX_MSG_OK) {
            fprintf(stderr, "m0 drain senders\n");
            lmx_msg_exec_stop(rta);
            lmx_msg_runtime_delete(rta);
            return 1;
        }
        dl = GetTickCount() + 3000;
        while ((InterlockedCompareExchange(&s1.done, 0, 0) == 0 || InterlockedCompareExchange(&s2.done, 0, 0) == 0) && GetTickCount() < dl) {
            lmx_msg_pump(rta);
            Sleep(5);
        }
        lmx_msg_pump(rta);
        if (InterlockedCompareExchange(&s1.done, 0, 0) == 0 || InterlockedCompareExchange(&s2.done, 0, 0) == 0
            || lmx_msg_inbox_n(rta, m0) != 2 || acc.value != 0 || acc.n_apply != 0) {
            fprintf(stderr, "m0 senders done=%ld,%ld inbox=%d value=%d\n",
                (long)InterlockedCompareExchange(&s1.done, 0, 0),
                (long)InterlockedCompareExchange(&s2.done, 0, 0),
                lmx_msg_inbox_n(rta, m0), acc.value);
            lmx_msg_exec_stop(rta);
            lmx_msg_runtime_delete(rta);
            return 1;
        }
        {
            int t1 = lmx_msg_run_child_turn(rta, m0);
            int t2 = lmx_msg_run_child_turn(rta, m0);
            if ((t1 != LMX_MSG_OK && t1 != 1) || (t2 != LMX_MSG_OK && t2 != 1)) {
                fprintf(stderr, "m0 turns st=%d,%d\n", t1, t2);
                lmx_msg_exec_stop(rta);
                lmx_msg_runtime_delete(rta);
                return 1;
            }
        }
        lmx_msg_exec_stop(rta);
        if (acc.value != 7 || InterlockedCompareExchange(&acc.overlap, 0, 0) != 0
            || InterlockedCompareExchange(&g_admit_n, 0, 0) != 2 || acc.n_apply != 2
            || g_admit_log[0] != acc.apply[0] || g_admit_log[1] != acc.apply[1]
            || !((acc.apply[0] == 2 && acc.apply[1] == 5) || (acc.apply[0] == 5 && acc.apply[1] == 2))) {
            fprintf(stderr, "m0 conc value=%d n=%d ov=%ld admit_n=%ld admit=%u,%u apply=%u,%u s1=%ld s2=%ld\n",
                acc.value, acc.n_apply, (long)InterlockedCompareExchange(&acc.overlap, 0, 0),
                (long)InterlockedCompareExchange(&g_admit_n, 0, 0), g_admit_log[0], g_admit_log[1],
                acc.apply[0], acc.apply[1],
                (long)InterlockedCompareExchange(&s1.done, 0, 0),
                (long)InterlockedCompareExchange(&s2.done, 0, 0));
            lmx_msg_runtime_delete(rta);
            return 1;
        }
        g_m0_acc = acc.value;
        g_m0_overlap = (int)InterlockedCompareExchange(&acc.overlap, 0, 0);
        g_m0_admit0 = g_admit_log[0];
        g_m0_admit1 = g_admit_log[1];
        g_m0_apply0 = acc.apply[0];
        g_m0_apply1 = acc.apply[1];
        /* This hook is scoped to the m0 scenario.  Leaving the destroyed
         * runtime's address armed lets a later runtime that reuses the same
         * numeric Message address enter the preceding scenario's observer. */
        g_admit_dest = 0;
        lmx_msg_runtime_delete(rta);
        fprintf(stderr, "m0_acc=7 admit=%u,%u apply=%u,%u overlap=0 concurrent=1\n",
            g_m0_admit0, g_m0_admit1, g_m0_apply0, g_m0_apply1);
        fflush(stderr);
    }
    {
        LmxMsgRuntime *rth;
        LmxMsgAddr dummy = 0, p = 0, c = 0, c2 = 0, g = 0;
        uchar ini = 7, ini2 = 8, ini3 = 9;
        NestUsers nu;
        void *gbase;
        void *cbase;
        void *c2base;
        rth = lmx_msg_runtime_new();
        memset(&nu, 0, sizeof(nu));
        if (rth == 0 || lmx_msg_create(rth, 0, 1, &ini, 1, &dummy) != LMX_MSG_OK) {
            return 1;
        }
        if (lmx_msg_create(rth, dummy, 2, &ini, 1, &p) != LMX_MSG_OK || lmx_msg_end_turn(rth, dummy, 1) != LMX_MSG_OK) {
            return 1;
        }
        if (lmx_msg_create(rth, p, 3, &ini, 1, &c) != LMX_MSG_OK || lmx_msg_create(rth, p, 4, &ini2, 1, &c2) != LMX_MSG_OK || lmx_msg_end_turn(rth, p, 1) != LMX_MSG_OK) {
            return 1;
        }
        if (lmx_msg_create(rth, c, 5, &ini3, 1, &g) != LMX_MSG_OK || lmx_msg_end_turn(rth, c, 1) != LMX_MSG_OK) {
            return 1;
        }
        gbase = lmx_msg_find(rth, g)->init;
        cbase = lmx_msg_find(rth, c)->init;
        c2base = lmx_msg_find(rth, c2)->init;
        nu.p = p;
        nu.c = c;
        if (lmx_msg_exec_bind(rth, c, turn_child_users, &nu, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK
            || lmx_msg_exec_bind(rth, p, turn_parent_nested, &nu, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK) {
            fprintf(stderr, "nest bind\n");
            return 1;
        }
        {
            LmxMsgEnv e;
            int pst;
            memset(&e, 0, sizeof(e));
            e.kind = LMX_MSG_KIND_BYTES;
            e.n = 1;
            e.bytes = &ini;
            if (lmx_msg_host_post(rth, p, &e) != LMX_MSG_STAGED || lmx_msg_host_post(rth, c, &e) != LMX_MSG_STAGED) {
                fprintf(stderr, "nest post\n");
                lmx_msg_runtime_delete(rth);
                return 1;
            }
            (void)lmx_msg_host_drain(rth);
            pst = lmx_msg_run_child_turn(rth, p);
            if (pst != LMX_MSG_OK && pst != 1) {
                fprintf(stderr, "nest P turn st=%d\n", pst);
                lmx_msg_runtime_delete(rth);
                return 1;
            }
        }
        if (nu.p_during != 1 || nu.c_during != 1 || nu.p_after != 1 || nu.c_after != 0) {
            fprintf(stderr, "nest users p_d=%d c_d=%d p_a=%d c_a=%d\n", nu.p_during, nu.c_during, nu.p_after, nu.c_after);
            lmx_msg_runtime_delete(rth);
            return 1;
        }
        if (lmx_msg_exec_bind(rth, g, turn_fail_end, 0, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK
            || lmx_msg_exec_bind(rth, c, turn_fail_end, 0, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK
            || lmx_msg_exec_bind(rth, c2, turn_fail_end, 0, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK) {
            fprintf(stderr, "fail bind\n");
            lmx_msg_runtime_delete(rth);
            return 1;
        }
        if (lmx_msg_emergency_cancel(rth, g) != LMX_MSG_OK || lmx_msg_emergency_cancel(rth, c) != LMX_MSG_OK || lmx_msg_emergency_cancel(rth, c2) != LMX_MSG_OK) {
            fprintf(stderr, "fail cancel\n");
            lmx_msg_runtime_delete(rth);
            return 1;
        }
        (void)lmx_msg_run_child_turn(rth, g);
        (void)lmx_msg_run_child_turn(rth, c);
        (void)lmx_msg_run_child_turn(rth, c2);
        if (lmx_msg_adopt_failed(rth, c2, g) != LMX_MSG_INVALID || lmx_msg_find(rth, g)->init != gbase) {
            fprintf(stderr, "sibling must not adopt G\n");
            lmx_msg_runtime_delete(rth);
            return 1;
        }
        if (lmx_msg_transfer_adopted(rth, c, p) != LMX_MSG_INVALID) {
            fprintf(stderr, "transfer C while G undisposed\n");
            lmx_msg_runtime_delete(rth);
            return 1;
        }
        if (lmx_msg_adopt_failed(rth, c, g) != LMX_MSG_OK || lmx_msg_adopted_n(rth, c) != 1 || lmx_msg_adopted_base(rth, c, 0) != gbase) {
            fprintf(stderr, "G->C n=%d\n", lmx_msg_adopted_n(rth, c));
            lmx_msg_runtime_delete(rth);
            return 1;
        }
        if (lmx_msg_adopt_failed(rth, p, c) != LMX_MSG_OK || lmx_msg_adopt_failed(rth, p, c2) != LMX_MSG_OK) {
            fprintf(stderr, "C->P\n");
            lmx_msg_runtime_delete(rth);
            return 1;
        }
        if (lmx_msg_adopted_n(rth, p) != 3) {
            fprintf(stderr, "P adopted n=%d\n", lmx_msg_adopted_n(rth, p));
            lmx_msg_runtime_delete(rth);
            return 1;
        }
        if (lmx_msg_adopt_failed(rth, p, c) != LMX_MSG_INVALID) {
            fprintf(stderr, "repeat adopt\n");
            lmx_msg_runtime_delete(rth);
            return 1;
        }
        if (lmx_msg_success_load(lmx_msg_find(rth, p)) != 0) {
            fprintf(stderr, "P success from children\n");
            lmx_msg_runtime_delete(rth);
            return 1;
        }
        {
            LmxMsgAddr live = 0;
            LmxMsgAddr drop = 0;
            int n_before;
            void *live_init;
            LmxMsgEnv e;
            int pst;
            if (lmx_msg_create(rth, p, 9, &ini, 1, &live) != LMX_MSG_OK || lmx_msg_create(rth, p, 10, &ini, 1, &drop) != LMX_MSG_OK
                || lmx_msg_end_turn(rth, p, 1) != LMX_MSG_OK) {
                fprintf(stderr, "live create\n");
                lmx_msg_runtime_delete(rth);
                return 1;
            }
            live_init = lmx_msg_find(rth, live)->init;
            if (lmx_msg_adopt_failed(rth, p, live) != LMX_MSG_INVALID || lmx_msg_find(rth, live)->init != live_init) {
                fprintf(stderr, "live idle adopt must reject and keep init\n");
                lmx_msg_runtime_delete(rth);
                return 1;
            }
            if (lmx_msg_exec_bind(rth, p, turn_complete_self, &nu, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK) {
                fprintf(stderr, "complete bind\n");
                lmx_msg_runtime_delete(rth);
                return 1;
            }
            memset(&e, 0, sizeof(e));
            e.kind = LMX_MSG_KIND_BYTES;
            e.n = 1;
            e.bytes = &ini;
            if (lmx_msg_host_post(rth, p, &e) != LMX_MSG_STAGED) {
                fprintf(stderr, "complete post\n");
                lmx_msg_runtime_delete(rth);
                return 1;
            }
            (void)lmx_msg_host_drain(rth);
            pst = lmx_msg_run_child_turn(rth, p);
            if ((pst != LMX_MSG_OK && pst != 1) || nu.p_during < 1 || nu.p_after != 3
                || lmx_msg_adopted_n(rth, p) != 3 || lmx_msg_success_load(lmx_msg_find(rth, p)) == 0) {
                fprintf(stderr, "complete during turn users=%d n=%d after=%d\n",
                    nu.p_during, lmx_msg_adopted_n(rth, p), nu.p_after);
                lmx_msg_runtime_delete(rth);
                return 1;
            }
            if (lmx_msg_exec_bind(rth, live, turn_fail_end, 0, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK
                || lmx_msg_emergency_cancel(rth, live) != LMX_MSG_OK
                || lmx_msg_exec_bind(rth, drop, turn_fail_end, 0, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK
                || lmx_msg_emergency_cancel(rth, drop) != LMX_MSG_OK) {
                fprintf(stderr, "live settle\n");
                lmx_msg_runtime_delete(rth);
                return 1;
            }
            (void)lmx_msg_run_child_turn(rth, live);
            (void)lmx_msg_run_child_turn(rth, drop);
            n_before = lmx_msg_adopted_n(rth, p);
            /* Decision 17 with spec 19.29.8: disposing a settled failed child
             * adopts its arena into the parent and releases the child's slot. */
            if (lmx_msg_dispose_child(rth, p, drop) != LMX_MSG_OK || lmx_msg_find(rth, drop) != 0
                || lmx_msg_adopted_n(rth, p) != n_before + 1) {
                fprintf(stderr, "failure dispose adopts the history and releases the child n=%d before=%d\n",
                    lmx_msg_adopted_n(rth, p), n_before);
                lmx_msg_runtime_delete(rth);
                return 1;
            }
            if (lmx_msg_adopt_failed(rth, p, live) != LMX_MSG_OK || lmx_msg_find(rth, live) != 0
                || lmx_msg_adopted_n(rth, p) != n_before + 2) {
                fprintf(stderr, "adopt live fail\n");
                lmx_msg_runtime_delete(rth);
                return 1;
            }
            if (lmx_msg_transfer_adopted(rth, p, dummy) != LMX_MSG_OK || lmx_msg_adopted_n(rth, dummy) != 5 || lmx_msg_adopted_n(rth, p) != 0) {
                fprintf(stderr, "transfer result n dummy=%d p=%d\n", lmx_msg_adopted_n(rth, dummy), lmx_msg_adopted_n(rth, p));
                lmx_msg_runtime_delete(rth);
                return 1;
            }
            if (lmx_msg_dispose_child(rth, dummy, p) != LMX_MSG_OK || lmx_msg_adopted_n(rth, dummy) != 5) {
                fprintf(stderr, "dispose after settle dummy=%d\n", lmx_msg_adopted_n(rth, dummy));
                lmx_msg_runtime_delete(rth);
                return 1;
            }
        }
        (void)gbase;
        (void)cbase;
        (void)c2base;
        if (lmx_msg_set_orphan_until(rth, dummy, 1U) != LMX_MSG_OK || lmx_msg_orphan_expired(rth, dummy, 99U) != 0) {
            fprintf(stderr, "idle orphan must not expire\n");
            lmx_msg_runtime_delete(rth);
            return 1;
        }
        fprintf(stderr, "handoff nest users G-C-P n=3 complete-keeps fail-dispose-reject transfer-survives\n");
        lmx_msg_runtime_delete(rth);
    }
    {
        LmxMsgRuntime *rto;
        LmxMsgAddr dummy = 0, src = 0, dst = 0;
        uchar ini = 1;
        OwnSend os;
        LmxMsgEnv e;
        memset(&os, 0, sizeof(os));
        rto = lmx_msg_runtime_new();
        if (rto == 0 || lmx_msg_create(rto, 0, 1, &ini, 1, &dummy) != LMX_MSG_OK) {
            return 1;
        }
        if (lmx_msg_create(rto, dummy, 2, &ini, 1, &src) != LMX_MSG_OK || lmx_msg_create(rto, dummy, 3, &ini, 1, &dst) != LMX_MSG_OK
            || lmx_msg_end_turn(rto, dummy, 1) != LMX_MSG_OK) {
            return 1;
        }
        os.dest = dst;
        if (lmx_msg_exec_bind(rto, src, turn_owned_send, &os, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK
            || lmx_msg_exec_bind(rto, dst, turn_owned_recv, &os, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK) {
            fprintf(stderr, "owned bind\n");
            return 1;
        }
        memset(&e, 0, sizeof(e));
        e.kind = LMX_MSG_KIND_BYTES;
        e.n = 1;
        e.bytes = &ini;
        if (lmx_msg_host_post(rto, src, &e) != LMX_MSG_STAGED) {
            fprintf(stderr, "owned post\n");
            lmx_msg_runtime_delete(rto);
            return 1;
        }
        (void)lmx_msg_host_drain(rto);
        (void)lmx_msg_run_child_turn(rto, src);
        lmx_msg_pump(rto);
        (void)lmx_msg_run_child_turn(rto, dst);
        if (os.orig == 0 || os.got != os.orig) {
            fprintf(stderr, "owned move orig=%p got=%p\n", os.orig, os.got);
            lmx_msg_runtime_delete(rto);
            return 1;
        }
        fprintf(stderr, "send_owned same-ptr env_release-once\n");
        lmx_msg_runtime_delete(rto);
    }
    {
        LmxMsgRuntime *rts;
        LmxMsgAddr dummy = 0, p = 0, kids[33];
        uchar ini = 1;
        int i;
        rts = lmx_msg_runtime_new();
        if (rts == 0 || lmx_msg_create(rts, 0, 1, &ini, 1, &dummy) != LMX_MSG_OK) {
            return 1;
        }
        if (lmx_msg_create(rts, dummy, 2, &ini, 1, &p) != LMX_MSG_OK || lmx_msg_end_turn(rts, dummy, 1) != LMX_MSG_OK) {
            return 1;
        }
        memset(kids, 0, sizeof(kids));
        for (i = 0; i < 33; i++) {
            if (lmx_msg_create(rts, p, (unsigned)(3 + i), &ini, 1, &kids[i]) != LMX_MSG_OK) {
                fprintf(stderr, "settle33 create %d\n", i);
                lmx_msg_runtime_delete(rts);
                return 1;
            }
        }
        if (lmx_msg_end_turn(rts, p, 1) != LMX_MSG_OK) {
            return 1;
        }
        for (i = 0; i < 33; i++) {
            if (lmx_msg_exec_bind(rts, kids[i], turn_fail_end, 0, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK
                || lmx_msg_emergency_cancel(rts, kids[i]) != LMX_MSG_OK) {
                fprintf(stderr, "settle33 bind %d\n", i);
                lmx_msg_runtime_delete(rts);
                return 1;
            }
            (void)lmx_msg_run_child_turn(rts, kids[i]);
        }
        for (i = 0; i < 32; i++) {
            if (lmx_msg_adopt_failed(rts, p, kids[i]) != LMX_MSG_OK) {
                fprintf(stderr, "settle33 adopt %d\n", i);
                lmx_msg_runtime_delete(rts);
                return 1;
            }
        }
        if (lmx_msg_parent_settle(rts, p) != LMX_MSG_OK || lmx_msg_adopted_n(rts, p) != 33) {
            fprintf(stderr, "settle33 n=%d\n", lmx_msg_adopted_n(rts, p));
            lmx_msg_runtime_delete(rts);
            return 1;
        }
        fprintf(stderr, "parent_settle 33 kids\n");
        lmx_msg_runtime_delete(rts);
    }
    {
        LmxMsgRuntime *rtr;
        LmxMsgAddr dummy = 0, p = 0, c = 0;
        uchar ini = 7;
        uchar store[32];
        LmxOwnedRange *pr;
        LmxOwnedRange *cr;
        LmxMsg *pm;
        LmxMsg *cm;
        void *init_keep;
        rtr = lmx_msg_runtime_new();
        if (rtr == 0 || lmx_msg_create(rtr, 0, 1, &ini, 1, &dummy) != LMX_MSG_OK
            || lmx_msg_create(rtr, dummy, 2, &ini, 1, &p) != LMX_MSG_OK
            || lmx_msg_end_turn(rtr, dummy, 1) != LMX_MSG_OK
            || lmx_msg_create(rtr, p, 3, &ini, 1, &c) != LMX_MSG_OK
            || lmx_msg_end_turn(rtr, p, 1) != LMX_MSG_OK) {
            fprintf(stderr, "range lifecycle create\n");
            return 1;
        }
        pr = (LmxOwnedRange *)calloc(1U, sizeof(LmxOwnedRange));
        cr = (LmxOwnedRange *)calloc(1U, sizeof(LmxOwnedRange));
        if (pr == 0 || cr == 0) {
            return 1;
        }
        pr->lo = store;
        pr->hi = store + 16;
        pr->stride = 1U;
        pr->kind = 1;
        pr->type = 1;
        cr->lo = store + 8;
        cr->hi = store + 24;
        cr->stride = 1U;
        cr->kind = 1;
        cr->type = 1;
        pm = lmx_msg_find(rtr, p);
        cm = lmx_msg_find(rtr, c);
        if (pm == 0 || cm == 0
            || lmx_owned_ranges_add(&pm->ranges, pr) != LMX_OWNED_RANGES_OK
            || lmx_owned_ranges_add(&cm->ranges, cr) != LMX_OWNED_RANGES_OK) {
            fprintf(stderr, "range lifecycle add\n");
            lmx_msg_runtime_delete(rtr);
            return 1;
        }
        init_keep = cm->init;
        if (lmx_msg_exec_bind(rtr, c, turn_fail_end, 0, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK
            || lmx_msg_emergency_cancel(rtr, c) != LMX_MSG_OK) {
            fprintf(stderr, "range overlap bind\n");
            lmx_msg_runtime_delete(rtr);
            return 1;
        }
        (void)lmx_msg_run_child_turn(rtr, c);
        if (lmx_msg_adopt_failed(rtr, p, c) != LMX_MSG_INVALID
            || cm->init != init_keep || cm->ranges != cr || pm->ranges != pr
            || pm->blocks != 0) {
            fprintf(stderr, "overlap must reject without moving blocks/ranges\n");
            lmx_msg_runtime_delete(rtr);
            return 1;
        }
        if (lmx_owned_ranges_remove(&pm->ranges, pr) != LMX_OWNED_RANGES_OK) {
            fprintf(stderr, "range overlap remove parent\n");
            lmx_msg_runtime_delete(rtr);
            return 1;
        }
        free(pr);
        if (lmx_msg_adopt_failed(rtr, p, c) != LMX_MSG_OK
            || lmx_msg_find(rtr, c) != 0 || pm->ranges != cr
            || cr->lo != store + 8 || cr->hi != store + 24 || cr->stride != 1U
            || lmx_owned_ranges_find(pm->ranges, store + 8) != cr
            || lmx_msg_adopted_n(rtr, p) != 1 || lmx_msg_adopted_base(rtr, p, 0) != init_keep) {
            fprintf(stderr, "typed range handoff identity n=%d\n", lmx_msg_adopted_n(rtr, p));
            lmx_msg_runtime_delete(rtr);
            return 1;
        }
        fprintf(stderr, "range overlap reject + typed handoff identity\n");
        lmx_msg_runtime_delete(rtr);
        free(cr);
    }
    {
        LmxMsgRuntime *rta;
        LmxMsgAddr dummy = 0, p = 0, c = 0;
        uchar ini = 9;
        uchar store[8];
        LmxOwnedRange *cr;
        LmxMsgBlock *pb;
        LmxMsgBlock *cb;
        void *pbase;
        void *cbase;
        LmxMsg *pm;
        LmxMsg *cm;
        void *init_keep;
        LmxOwnedRange *range_keep;
        rta = lmx_msg_runtime_new();
        if (rta == 0 || lmx_msg_create(rta, 0, 1, &ini, 1, &dummy) != LMX_MSG_OK
            || lmx_msg_create(rta, dummy, 2, &ini, 1, &p) != LMX_MSG_OK
            || lmx_msg_end_turn(rta, dummy, 1) != LMX_MSG_OK
            || lmx_msg_create(rta, p, 3, &ini, 1, &c) != LMX_MSG_OK
            || lmx_msg_end_turn(rta, p, 1) != LMX_MSG_OK) {
            fprintf(stderr, "adopt nomem create\n");
            return 1;
        }
        cr = (LmxOwnedRange *)calloc(1U, sizeof(LmxOwnedRange));
        pb = (LmxMsgBlock *)calloc(1U, sizeof(LmxMsgBlock));
        cb = (LmxMsgBlock *)calloc(1U, sizeof(LmxMsgBlock));
        pbase = malloc(8U);
        cbase = malloc(8U);
        if (cr == 0 || pb == 0 || cb == 0 || pbase == 0 || cbase == 0) {
            return 1;
        }
        cr->lo = store;
        cr->hi = store + 8;
        cr->stride = 1U;
        cr->kind = 1;
        cr->type = 1;
        pb->base = pbase;
        pb->n = 8U;
        cb->base = cbase;
        cb->n = 8U;
        pm = lmx_msg_find(rta, p);
        cm = lmx_msg_find(rta, c);
        if (pm == 0 || cm == 0
            || lmx_owned_ranges_add(&cm->ranges, cr) != LMX_OWNED_RANGES_OK
            || lmx_msg_blocks_push(&pm->blocks, pb) != LMX_MSG_BLOCKS_OK
            || lmx_msg_blocks_push(&cm->blocks, cb) != LMX_MSG_BLOCKS_OK) {
            fprintf(stderr, "adopt nomem add range/blocks\n");
            lmx_msg_runtime_delete(rta);
            return 1;
        }
        init_keep = cm->init;
        range_keep = cm->ranges;
        if (lmx_msg_exec_bind(rta, c, turn_fail_end, 0, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK
            || lmx_msg_emergency_cancel(rta, c) != LMX_MSG_OK) {
            fprintf(stderr, "adopt nomem bind\n");
            lmx_msg_runtime_delete(rta);
            return 1;
        }
        (void)lmx_msg_run_child_turn(rta, c);
        lmx_msg_exec_test_set_fail_adopt_block(rta, 1);
        if (lmx_msg_adopt_failed(rta, p, c) != LMX_MSG_NOMEM
            || cm->init != init_keep || cm->ranges != range_keep || cm->disposed != 0
            || pm->ranges != 0 || pm->blocks != pb || pb->next != 0 || pb->base != pbase
            || cm->blocks != cb || cb->next != 0 || cb->base != cbase) {
            fprintf(stderr, "adopt nomem must leave both owners' blocks/ranges/init\n");
            lmx_msg_runtime_delete(rta);
            return 1;
        }
        lmx_msg_exec_test_set_fail_adopt_block(rta, 0);
        if (lmx_msg_adopt_failed(rta, p, c) != LMX_MSG_OK
            || lmx_msg_find(rta, c) != 0
            || pm->ranges != range_keep || pm->blocks == 0
            || pm->blocks->base != init_keep || pm->blocks->next != cb
            || cb->next != pb || pb->next != 0 || cb->base != cbase || pb->base != pbase
            || lmx_msg_adopted_n(rta, p) != 3 || lmx_msg_adopted_base(rta, p, 0) != init_keep
            || lmx_msg_adopted_base(rta, p, 1) != cbase || lmx_msg_adopted_base(rta, p, 2) != pbase) {
            fprintf(stderr, "adopt after nomem retry n=%d\n", lmx_msg_adopted_n(rta, p));
            lmx_msg_runtime_delete(rta);
            return 1;
        }
        fprintf(stderr, "adopt nomem leaves both owners' existing blocks; retry moves all\n");
        lmx_msg_runtime_delete(rta);
        free(cr);
    }
    {
        LmxMsgRuntime *rtc;
        LmxMsgAddr a = 0, b = 0;
        uchar ini = 3;
        LmxMsg *ma;
        LmxMsg *mb;
        char *ta;
        char *tb;
        void *cell;
        void *re;
        Lmx *unit;
        rtc = lmx_msg_runtime_new();
        if (rtc == 0 || lmx_msg_create(rtc, 0, 1, &ini, 1, &a) != LMX_MSG_OK
            || lmx_msg_create(rtc, 0, 2, &ini, 1, &b) != LMX_MSG_OK) {
            fprintf(stderr, "chars collect create\n");
            return 1;
        }
        ma = lmx_msg_find(rtc, a);
        mb = lmx_msg_find(rtc, b);
        ta = lmx_chars_new_owned(&ma->blocks, &ma->ranges);
        tb = lmx_chars_new_owned(&mb->blocks, &mb->ranges);
        cell = lmx_char_cell_known(ta, 65);
        unit = (ma == 0) ? 0 : lmx_node_new_owned(&ma->blocks, &ma->ranges);
        if (ma == 0 || mb == 0 || ta == 0 || tb == 0 || cell == 0 || unit == 0
            || lmx_char_cell_known(tb, 65) == cell
            || lmx_branch_open_owned(unit, 1U, &ma->blocks, &ma->ranges) != 0
            || lmx_branch_store_known(unit, 0U, cell) != 0) {
            fprintf(stderr, "chars collect tables\n");
            lmx_msg_runtime_delete(rtc);
            return 1;
        }
        lmx_msg_set_graph(ma, unit);
        lmx_msg_arena_collect(ma);
        if (lmx_owned_ranges_find(ma->ranges, cell) == 0) {
            fprintf(stderr, "rooted char table collected\n");
            lmx_msg_runtime_delete(rtc);
            return 1;
        }
        re = lmx_char_rebind_known(cell, 66);
        if (re != lmx_char_cell_known(ta, 66)) {
            fprintf(stderr, "rebind identity\n");
            lmx_msg_runtime_delete(rtc);
            return 1;
        }
        if (lmx_msg_storage_move_all(&mb->blocks, &mb->ranges, &ma->blocks, &ma->ranges)
            != LMX_MSG_STORAGE_OK || ma->blocks != 0 || ma->ranges != 0) {
            fprintf(stderr, "chars table move\n");
            lmx_msg_runtime_delete(rtc);
            return 1;
        }
        if (lmx_char_rebind_known(cell, 67) != lmx_char_cell_known(ta, 67)
            || lmx_char_rebind_known(cell, 67) == lmx_char_cell_known(tb, 67)) {
            fprintf(stderr, "adopted table recanonicalized\n");
            lmx_msg_runtime_delete(rtc);
            return 1;
        }
        lmx_msg_set_graph(ma, 0);
        lmx_msg_set_graph(mb, unit);
        lmx_msg_arena_collect(mb);
        if (lmx_owned_ranges_find(mb->ranges, cell) == 0) {
            fprintf(stderr, "transferred char table died while rooted\n");
            lmx_msg_runtime_delete(rtc);
            return 1;
        }
        if (lmx_owned_ranges_find(mb->ranges, tb) != 0) {
            fprintf(stderr, "unrooted recipient table immortal\n");
            lmx_msg_runtime_delete(rtc);
            return 1;
        }
        lmx_msg_set_graph(mb, 0);
        lmx_msg_arena_collect(mb);
        if (lmx_owned_ranges_find(mb->ranges, cell) != 0
            || lmx_owned_ranges_find(mb->ranges, tb) != 0 || mb->blocks != 0) {
            fprintf(stderr, "unrooted char tables immortal\n");
            lmx_msg_runtime_delete(rtc);
            return 1;
        }
        fprintf(stderr, "rooted char table lives; unroot reclaims range-before-block\n");
        lmx_msg_runtime_delete(rtc);
    }
    {
        LmxMsgRuntime *rtv;
        LmxMsgAddr a = 0;
        uchar ini = 9;
        LmxMsg *ma;
        Lmx *root;
        Lmx *inner;
        void *icell;
        LmxMethod *rec;
        LmxArrayDesc *arr;
        LmxArrayDesc *dead;
        LmxOwnedRange *rg;
        rtv = lmx_msg_runtime_new();
        if (rtv == 0 || lmx_msg_create(rtv, 0, 1, &ini, 1, &a) != LMX_MSG_OK) {
            fprintf(stderr, "slot-value collect create\n");
            return 1;
        }
        ma = lmx_msg_find(rtv, a);
        root = (ma == 0) ? 0 : lmx_node_new_owned(&ma->blocks, &ma->ranges);
        if (ma == 0 || root == 0
            || lmx_branch_open_owned(root, 4U, &ma->blocks, &ma->ranges) != 0) {
            fprintf(stderr, "slot-value collect root\n");
            lmx_msg_runtime_delete(rtv);
            return 1;
        }
        icell = lmx_int_new_owned(&ma->blocks, &ma->ranges);
        rec = lmx_method_new_owned(&ma->blocks, &ma->ranges);
        inner = lmx_struct_new_owned(root, &ma->blocks, &ma->ranges);
        arr = lmx_array_new_positive_owned(LMX_TYPE_ARRAY_OF_INT, 2U, &ma->blocks, &ma->ranges);
        dead = lmx_array_new_positive_owned(LMX_TYPE_ARRAY_OF_CHAR, 2U, &ma->blocks, &ma->ranges);
        if (icell == 0 || rec == 0 || inner == 0 || arr == 0 || dead == 0
            || lmx_branch_open_owned(inner, 1U, &ma->blocks, &ma->ranges) != 0
            || lmx_branch_store_known(inner, 0U, icell) != 0
            || lmx_branch_store_known(root, 0U, icell) != 0
            || lmx_branch_store_known(root, 1U, rec) != 0
            || lmx_branch_store_known(root, 2U, inner) != 0
            || lmx_branch_store_known(root, 3U, arr) != 0) {
            fprintf(stderr, "slot-value collect graph\n");
            lmx_msg_runtime_delete(rtv);
            return 1;
        }
        rec->sig = 3U;
        lmx_msg_set_graph(ma, root);
        lmx_msg_arena_collect(ma);
        rg = lmx_owned_ranges_find(ma->ranges, root);
        if (rg == 0 || rg->kind != LMX_KIND_STRUCT
            || lmx_owned_ranges_find(ma->ranges, inner) == 0
            || lmx_owned_ranges_find(ma->ranges, icell) == 0
            || lmx_owned_ranges_find(ma->ranges, rec) == 0
            || lmx_owned_ranges_find(ma->ranges, arr) == 0
            || lmx_owned_ranges_find(ma->ranges, dead) != 0) {
            fprintf(stderr, "slot-value collect live/dead\n");
            lmx_msg_runtime_delete(rtv);
            return 1;
        }
        if (lmx_owned_ranges_find(ma->ranges, rec)->kind != LMX_KIND_METHOD
            || rec->sig != 3U) {
            fprintf(stderr, "slot-value collect method\n");
            lmx_msg_runtime_delete(rtv);
            return 1;
        }
        fprintf(stderr, "slot-value collect: nested STRUCT+int+method+array live; unrooted array dies\n");
        if (lmx_msg_root_attach(ma, root) != LMX_MSG_OK) {
            fprintf(stderr, "struct root attach\n");
            lmx_msg_runtime_delete(rtv);
            return 1;
        }
        lmx_msg_set_graph(ma, 0);
        lmx_msg_arena_collect(ma);
        if (lmx_owned_ranges_find(ma->ranges, root) == 0
            || lmx_owned_ranges_find(ma->ranges, inner) == 0
            || lmx_owned_ranges_find(ma->ranges, icell) == 0) {
            fprintf(stderr, "struct retain lost subtree\n");
            lmx_msg_runtime_delete(rtv);
            return 1;
        }
        if (lmx_msg_root_release(ma, root) != LMX_MSG_OK) {
            fprintf(stderr, "struct root release\n");
            lmx_msg_runtime_delete(rtv);
            return 1;
        }
        lmx_msg_arena_collect(ma);
        if (lmx_owned_ranges_find(ma->ranges, root) != 0
            || lmx_owned_ranges_find(ma->ranges, inner) != 0
            || lmx_owned_ranges_find(ma->ranges, icell) != 0) {
            fprintf(stderr, "struct retain immortal after release\n");
            lmx_msg_runtime_delete(rtv);
            return 1;
        }
        fprintf(stderr, "explicit STRUCT retain keeps subtree without graph; release reclaims\n");
        lmx_msg_runtime_delete(rtv);
    }
    {
        LmxMsgRuntime *rta;
        LmxMsgAddr a = 0, b = 0;
        uchar ini = 4;
        LmxMsg *ma;
        LmxMsg *mb;
        LmxArrayDesc *da;
        LmxArrayDesc *db;
        void *back_a;
        Lmx *unit;
        rta = lmx_msg_runtime_new();
        if (rta == 0 || lmx_msg_create(rta, 0, 1, &ini, 1, &a) != LMX_MSG_OK
            || lmx_msg_create(rta, 0, 2, &ini, 1, &b) != LMX_MSG_OK) {
            fprintf(stderr, "array collect create\n");
            return 1;
        }
        ma = lmx_msg_find(rta, a);
        mb = lmx_msg_find(rta, b);
        da = lmx_array_new_positive_owned(LMX_TYPE_ARRAY_OF_CHAR, 3U, &ma->blocks, &ma->ranges);
        db = lmx_array_new_positive_owned(LMX_TYPE_ARRAY_OF_INT, 2U, &mb->blocks, &mb->ranges);
        unit = (ma == 0) ? 0 : lmx_node_new_owned(&ma->blocks, &ma->ranges);
        if (ma == 0 || mb == 0 || da == 0 || db == 0 || da->data == 0 || db->data == 0 || unit == 0
            || lmx_branch_open_owned(unit, 1U, &ma->blocks, &ma->ranges) != 0
            || lmx_branch_store_known(unit, 0U, da) != 0) {
            fprintf(stderr, "array collect new\n");
            lmx_msg_runtime_delete(rta);
            return 1;
        }
        back_a = da->data;
        ((char *)back_a)[0] = 'Q';
        lmx_msg_set_graph(ma, unit);
        lmx_msg_arena_collect(ma);
        if (lmx_owned_ranges_find(ma->ranges, da) == 0
            || lmx_owned_ranges_find(ma->ranges, back_a) == 0
            || ((char *)back_a)[0] != 'Q') {
            fprintf(stderr, "rooted array collected\n");
            lmx_msg_runtime_delete(rta);
            return 1;
        }
        if (lmx_msg_storage_move_all(&mb->blocks, &mb->ranges, &ma->blocks, &ma->ranges)
            != LMX_MSG_STORAGE_OK || ma->blocks != 0 || ma->ranges != 0) {
            fprintf(stderr, "array move\n");
            lmx_msg_runtime_delete(rta);
            return 1;
        }
        if (da->data != back_a || ((char *)back_a)[0] != 'Q'
            || lmx_owned_ranges_find(mb->ranges, db) == 0) {
            fprintf(stderr, "array identity/recipient lost\n");
            lmx_msg_runtime_delete(rta);
            return 1;
        }
        lmx_msg_set_graph(ma, 0);
        lmx_msg_set_graph(mb, unit);
        lmx_msg_arena_collect(mb);
        if (lmx_owned_ranges_find(mb->ranges, da) == 0
            || lmx_owned_ranges_find(mb->ranges, back_a) == 0) {
            fprintf(stderr, "transferred array died while rooted\n");
            lmx_msg_runtime_delete(rta);
            return 1;
        }
        if (lmx_owned_ranges_find(mb->ranges, db) != 0
            || lmx_owned_ranges_find(mb->ranges, db->data) != 0) {
            fprintf(stderr, "unrooted recipient array immortal\n");
            lmx_msg_runtime_delete(rta);
            return 1;
        }
        lmx_msg_set_graph(mb, 0);
        lmx_msg_arena_collect(mb);
        if (lmx_owned_ranges_find(mb->ranges, da) != 0
            || lmx_owned_ranges_find(mb->ranges, back_a) != 0 || mb->blocks != 0) {
            fprintf(stderr, "unrooted array immortal\n");
            lmx_msg_runtime_delete(rta);
            return 1;
        }
        fprintf(stderr, "rooted array descriptor keeps backing; unroot reclaims both\n");
        lmx_msg_runtime_delete(rta);
    }
    {
        LmxMsgRuntime *rtr;
        LmxMsgAddr a = 0;
        uchar ini = 5;
        LmxMsg *ma;
        LmxArrayDesc *chars;
        LmxArrayDesc *ints;
        LmxArrayDesc *of_lmx;
        LmxArrayDesc *of_desc;
        LmxArrayDesc *of_cycle;
        LmxArrayDesc *of_share;
        LmxArrayDesc *of_meth;
        LmxMethod *rec;
        LmxOwnedRange *rg;
        Lmx *unit;
        Lmx *s_desc;
        Lmx *s_meth;
        void *char_back;
        void *int_back;
        void *share_back;
        rtr = lmx_msg_runtime_new();
        if (rtr == 0 || lmx_msg_create(rtr, 0, 1, &ini, 1, &a) != LMX_MSG_OK) {
            fprintf(stderr, "ref array collect create\n");
            return 1;
        }
        ma = lmx_msg_find(rtr, a);
        if (ma == 0) {
            fprintf(stderr, "ref array collect find\n");
            lmx_msg_runtime_delete(rtr);
            return 1;
        }
        chars = lmx_array_new_positive_owned(LMX_TYPE_ARRAY_OF_CHAR, 2U, &ma->blocks, &ma->ranges);
        ints = lmx_array_new_positive_owned(LMX_TYPE_ARRAY_OF_INT, 2U, &ma->blocks, &ma->ranges);
        rec = (LmxMethod *)test_owned_prepare(1U, sizeof(LmxMethod),
            LMX_KIND_METHOD, LMX_TYPE_METHOD, &ma->blocks, &ma->ranges);
        of_lmx = lmx_array_ref_new_positive_owned(LMX_TYPE_ARRAY_OF_LMX, 2U, &ma->blocks, &ma->ranges);
        of_desc = lmx_array_ref_new_positive_owned(LMX_TYPE_ARRAY_OF_DESC, 2U, &ma->blocks, &ma->ranges);
        of_cycle = lmx_array_ref_new_positive_owned(LMX_TYPE_ARRAY_OF_DESC, 2U, &ma->blocks, &ma->ranges);
        of_share = lmx_array_ref_new_positive_owned(LMX_TYPE_ARRAY_OF_DESC, 1U, &ma->blocks, &ma->ranges);
        of_meth = lmx_array_ref_new_positive_owned(LMX_TYPE_ARRAY_OF_METHOD, 1U, &ma->blocks, &ma->ranges);
        if (chars == 0 || ints == 0 || rec == 0 || of_lmx == 0 || of_desc == 0 || of_cycle == 0
            || of_share == 0 || of_meth == 0
            || chars->data == 0 || ints->data == 0 || of_lmx->data == 0
            || of_desc->data == 0 || of_cycle->data == 0 || of_share->data == 0
            || of_meth->data == 0) {
            fprintf(stderr, "ref array collect new\n");
            lmx_msg_runtime_delete(rtr);
            return 1;
        }
        char_back = chars->data;
        int_back = ints->data;
        share_back = of_share->data;
        ((char *)char_back)[0] = 'R';
        rec->sig = 9U;
        unit = lmx_node_new_owned(&ma->blocks, &ma->ranges);
        s_desc = lmx_struct_new_owned(unit, &ma->blocks, &ma->ranges);
        s_meth = lmx_struct_new_owned(unit, &ma->blocks, &ma->ranges);
        if (unit == 0 || s_desc == 0 || s_meth == 0
            || lmx_branch_open_owned(unit, 1U, &ma->blocks, &ma->ranges) != 0
            || lmx_branch_open_owned(s_desc, 1U, &ma->blocks, &ma->ranges) != 0
            || lmx_branch_open_owned(s_meth, 1U, &ma->blocks, &ma->ranges) != 0
            || lmx_branch_store_known(s_desc, 0U, of_desc) != 0
            || lmx_branch_store_known(s_meth, 0U, of_meth) != 0
            || lmx_branch_store_known(unit, 0U, of_lmx) != 0) {
            fprintf(stderr, "ref array collect graph\n");
            lmx_msg_runtime_delete(rtr);
            return 1;
        }
        ((Lmx **)of_lmx->data)[0] = s_desc;
        ((Lmx **)of_lmx->data)[1] = s_meth;
        ((LmxArrayDesc **)of_desc->data)[0] = chars;
        ((LmxArrayDesc **)of_desc->data)[1] = of_cycle;
        ((LmxArrayDesc **)of_cycle->data)[0] = of_desc;
        ((LmxArrayDesc **)of_cycle->data)[1] = chars;
        ((LmxArrayDesc **)of_share->data)[0] = chars;
        ((LmxMethod **)of_meth->data)[0] = rec;
        lmx_msg_set_graph(ma, unit);
        lmx_msg_arena_collect(ma);
        rg = lmx_owned_ranges_find(ma->ranges, of_lmx);
        if (rg == 0 || rg->kind != LMX_KIND_ARRAY || rg->type != LMX_TYPE_ARRAY_OF_LMX) {
            fprintf(stderr, "ARRAY_OF_LMX descriptor type\n");
            lmx_msg_runtime_delete(rtr);
            return 1;
        }
        rg = lmx_owned_ranges_find(ma->ranges, of_lmx->data);
        if (rg == 0 || rg->kind != LMX_KIND_REF || rg->type != LMX_TYPE_LMX
            || rg->stride != sizeof(void *)) {
            fprintf(stderr, "LMX ref-cell type/stride\n");
            lmx_msg_runtime_delete(rtr);
            return 1;
        }
        rg = lmx_owned_ranges_find(ma->ranges, of_desc);
        if (rg == 0 || rg->type != LMX_TYPE_ARRAY_OF_DESC) {
            fprintf(stderr, "ARRAY_OF_DESC descriptor type\n");
            lmx_msg_runtime_delete(rtr);
            return 1;
        }
        rg = lmx_owned_ranges_find(ma->ranges, of_desc->data);
        if (rg == 0 || rg->kind != LMX_KIND_REF || rg->type != LMX_TYPE_DESC) {
            fprintf(stderr, "DESC ref-cell type\n");
            lmx_msg_runtime_delete(rtr);
            return 1;
        }
        rg = lmx_owned_ranges_find(ma->ranges, of_meth);
        if (rg == 0 || rg->type != LMX_TYPE_ARRAY_OF_METHOD) {
            fprintf(stderr, "ARRAY_OF_METHOD descriptor type\n");
            lmx_msg_runtime_delete(rtr);
            return 1;
        }
        rg = lmx_owned_ranges_find(ma->ranges, of_meth->data);
        if (rg == 0 || rg->kind != LMX_KIND_REF || rg->type != LMX_TYPE_METHOD) {
            fprintf(stderr, "METHOD ref-cell type\n");
            lmx_msg_runtime_delete(rtr);
            return 1;
        }
        rg = lmx_owned_ranges_find(ma->ranges, chars);
        if (rg == 0 || rg->type != LMX_TYPE_ARRAY_OF_CHAR
            || rg->type == LMX_TYPE_ARRAY_OF_DESC) {
            fprintf(stderr, "CHAR array type collapsed into DESC\n");
            lmx_msg_runtime_delete(rtr);
            return 1;
        }
        if (lmx_owned_ranges_find(ma->ranges, char_back) == 0
            || lmx_owned_ranges_find(ma->ranges, rec) == 0
            || ((char *)char_back)[0] != 'R' || rec->sig != 9U) {
            fprintf(stderr, "rooted ref-array referents collected\n");
            lmx_msg_runtime_delete(rtr);
            return 1;
        }
        if (lmx_owned_ranges_find(ma->ranges, of_cycle) == 0
            || lmx_owned_ranges_find(ma->ranges, of_cycle->data) == 0) {
            fprintf(stderr, "nested DESC cycle collected\n");
            lmx_msg_runtime_delete(rtr);
            return 1;
        }
        if (lmx_owned_ranges_find(ma->ranges, of_share) != 0
            || lmx_owned_ranges_find(ma->ranges, share_back) != 0) {
            fprintf(stderr, "unrooted shared-DESC array immortal\n");
            lmx_msg_runtime_delete(rtr);
            return 1;
        }
        if (lmx_owned_ranges_find(ma->ranges, ints) != 0
            || lmx_owned_ranges_find(ma->ranges, int_back) != 0) {
            fprintf(stderr, "unrooted primitive array immortal beside refs\n");
            lmx_msg_runtime_delete(rtr);
            return 1;
        }
        lmx_msg_set_graph(ma, 0);
        lmx_msg_arena_collect(ma);
        if (ma->blocks != 0 || ma->ranges != 0
            || lmx_owned_ranges_find(ma->ranges, of_lmx) != 0
            || lmx_owned_ranges_find(ma->ranges, of_cycle) != 0
            || lmx_owned_ranges_find(ma->ranges, chars) != 0
            || lmx_owned_ranges_find(ma->ranges, rec) != 0) {
            fprintf(stderr, "unrooted ref arrays immortal\n");
            lmx_msg_runtime_delete(rtr);
            return 1;
        }
        fprintf(stderr, "ref arrays: constructor+cycle+shared DESC; unrooted INT/share die; unroot reclaims\n");
        lmx_msg_runtime_delete(rtr);
    }
    {
        LmxMsgRuntime *rte;
        LmxMsgAddr a = 0;
        uchar ini = 6;
        LmxMsg *ma;
        LmxArrayDesc *chars;
        LmxArrayDesc *ints;
        Lmx *unit;
        void *char_back;
        void *int_back;
        rte = lmx_msg_runtime_new();
        if (rte == 0 || lmx_msg_create(rte, 0, 1, &ini, 1, &a) != LMX_MSG_OK) {
            fprintf(stderr, "end_turn array create\n");
            return 1;
        }
        ma = lmx_msg_find(rte, a);
        if (ma == 0) {
            fprintf(stderr, "end_turn array find\n");
            lmx_msg_runtime_delete(rte);
            return 1;
        }
        chars = lmx_array_new_positive_owned(LMX_TYPE_ARRAY_OF_CHAR, 2U, &ma->blocks, &ma->ranges);
        ints = lmx_array_new_positive_owned(LMX_TYPE_ARRAY_OF_INT, 2U, &ma->blocks, &ma->ranges);
        if (chars == 0 || ints == 0 || chars->data == 0 || ints->data == 0) {
            fprintf(stderr, "end_turn array new\n");
            lmx_msg_runtime_delete(rte);
            return 1;
        }
        char_back = chars->data;
        int_back = ints->data;
        ((char *)char_back)[0] = 'E';
        unit = lmx_node_new_owned(&ma->blocks, &ma->ranges);
        if (unit == 0 || lmx_branch_open_owned(unit, 1U, &ma->blocks, &ma->ranges) != 0
            || lmx_branch_store_known(unit, 0U, chars) != 0) {
            fprintf(stderr, "end_turn array graph\n");
            lmx_msg_runtime_delete(rte);
            return 1;
        }
        lmx_msg_set_graph(ma, unit);
        if (lmx_msg_end_turn(rte, a, 1) != LMX_MSG_OK) {
            fprintf(stderr, "end_turn array first\n");
            lmx_msg_runtime_delete(rte);
            return 1;
        }
        if (lmx_owned_ranges_find(ma->ranges, chars) == 0
            || lmx_owned_ranges_find(ma->ranges, char_back) == 0
            || ((char *)char_back)[0] != 'E') {
            fprintf(stderr, "end_turn dropped rooted CHAR array\n");
            lmx_msg_runtime_delete(rte);
            return 1;
        }
        if (lmx_owned_ranges_find(ma->ranges, ints) != 0
            || lmx_owned_ranges_find(ma->ranges, int_back) != 0) {
            fprintf(stderr, "end_turn kept unrooted INT array\n");
            lmx_msg_runtime_delete(rte);
            return 1;
        }
        lmx_msg_set_graph(ma, 0);
        if (lmx_msg_end_turn(rte, a, 1) != LMX_MSG_OK) {
            fprintf(stderr, "end_turn array second\n");
            lmx_msg_runtime_delete(rte);
            return 1;
        }
        if (ma->blocks != 0 || ma->ranges != 0
            || lmx_owned_ranges_find(ma->ranges, chars) != 0
            || lmx_owned_ranges_find(ma->ranges, char_back) != 0) {
            fprintf(stderr, "end_turn unroot left CHAR array\n");
            lmx_msg_runtime_delete(rte);
            return 1;
        }
        fprintf(stderr, "end_turn collect: rooted CHAR lives; unrooted INT dies; unroot reclaims\n");
        lmx_msg_runtime_delete(rte);
    }
    {
        LmxMsgRuntime *rtg;
        LmxMsgAddr a = 0;
        uchar ini = 7;
        LmxMsg *ma;
        Lmx *unit;
        LmxArrayDesc *buf;
        LmxArrayDesc *letters;
        LmxArrayDesc *dead;
        LmxOwnedRange *rg;
        void *buf_back;
        void *letters_back;
        void *dead_back;
        rtg = lmx_msg_runtime_new();
        if (rtg == 0 || lmx_msg_create(rtg, 0, 1, &ini, 1, &a) != LMX_MSG_OK) {
            fprintf(stderr, "emit array create\n");
            return 1;
        }
        ma = lmx_msg_find(rtg, a);
        if (ma == 0) {
            fprintf(stderr, "emit array find\n");
            lmx_msg_runtime_delete(rtg);
            return 1;
        }
        unit = lmx_node_new_owned(&ma->blocks, &ma->ranges);
        if (unit == 0 || lmx_branch_open_owned(unit, 2U, &ma->blocks, &ma->ranges) != 0) {
            fprintf(stderr, "emit array unit\n");
            lmx_msg_runtime_delete(rtg);
            return 1;
        }
        buf = lmx_array_new_positive_owned(LMX_TYPE_ARRAY_OF_INT, 3U, &ma->blocks, &ma->ranges);
        letters = lmx_array_new_positive_owned(LMX_TYPE_ARRAY_OF_CHAR, 4U, &ma->blocks, &ma->ranges);
        dead = lmx_array_new_positive_owned(LMX_TYPE_ARRAY_OF_INT, 2U, &ma->blocks, &ma->ranges);
        if (buf == 0 || letters == 0 || dead == 0
            || buf->data == 0 || letters->data == 0 || dead->data == 0
            || lmx_branch_store_known(unit, 0U, buf) != 0
            || lmx_branch_store_known(unit, 1U, letters) != 0) {
            fprintf(stderr, "emit array fields\n");
            lmx_msg_runtime_delete(rtg);
            return 1;
        }
        buf_back = buf->data;
        letters_back = letters->data;
        dead_back = dead->data;
        lmx_msg_set_graph(ma, unit);
        if (lmx_msg_end_turn(rtg, a, 1) != LMX_MSG_OK) {
            fprintf(stderr, "emit array end_turn\n");
            lmx_msg_runtime_delete(rtg);
            return 1;
        }
        rg = lmx_owned_ranges_find(ma->ranges, buf);
        if (rg == 0 || rg->kind != LMX_KIND_ARRAY || rg->type != LMX_TYPE_ARRAY_OF_INT
            || buf->len != 3U || ((int *)buf_back)[0] != 0) {
            fprintf(stderr, "emit INT field dropped\n");
            lmx_msg_runtime_delete(rtg);
            return 1;
        }
        rg = lmx_owned_ranges_find(ma->ranges, letters);
        if (rg == 0 || rg->type != LMX_TYPE_ARRAY_OF_CHAR || letters->len != 4U
            || lmx_owned_ranges_find(ma->ranges, letters_back) == 0) {
            fprintf(stderr, "emit CHAR field dropped\n");
            lmx_msg_runtime_delete(rtg);
            return 1;
        }
        if (lmx_owned_ranges_find(ma->ranges, dead) != 0
            || lmx_owned_ranges_find(ma->ranges, dead_back) != 0) {
            fprintf(stderr, "emit unrooted neighbour immortal\n");
            lmx_msg_runtime_delete(rtg);
            return 1;
        }
        lmx_msg_set_graph(ma, 0);
        if (lmx_msg_end_turn(rtg, a, 1) != LMX_MSG_OK) {
            fprintf(stderr, "emit array unroot end_turn\n");
            lmx_msg_runtime_delete(rtg);
            return 1;
        }
        if (lmx_owned_ranges_find(ma->ranges, buf) != 0
            || lmx_owned_ranges_find(ma->ranges, letters) != 0) {
            fprintf(stderr, "emit unroot left fields\n");
            lmx_msg_runtime_delete(rtg);
            return 1;
        }
        fprintf(stderr, "emit own-array fields: INT3/CHAR4 live through end_turn; unroot reclaims\n");
        lmx_msg_runtime_delete(rtg);
    }
    {
        LmxMsgRuntime *rtr;
        LmxMsgAddr a = 0;
        uchar ini = 8;
        LmxMsg *ma;
        LmxArrayDesc *keep;
        LmxArrayDesc *drop;
        void *keep_back;
        void *drop_back;
        void *foreign;
        rtr = lmx_msg_runtime_new();
        if (rtr == 0 || lmx_msg_create(rtr, 0, 1, &ini, 1, &a) != LMX_MSG_OK) {
            fprintf(stderr, "explicit root create\n");
            return 1;
        }
        ma = lmx_msg_find(rtr, a);
        keep = (ma == 0) ? 0 : lmx_array_new_positive_owned(LMX_TYPE_ARRAY_OF_INT, 3U, &ma->blocks, &ma->ranges);
        drop = (ma == 0) ? 0 : lmx_array_new_positive_owned(LMX_TYPE_ARRAY_OF_CHAR, 4U, &ma->blocks, &ma->ranges);
        foreign = malloc(8U);
        if (ma == 0 || keep == 0 || drop == 0 || keep->data == 0 || drop->data == 0 || foreign == 0) {
            fprintf(stderr, "explicit root new\n");
            free(foreign);
            lmx_msg_runtime_delete(rtr);
            return 1;
        }
        keep_back = keep->data;
        drop_back = drop->data;
        lmx_msg_set_graph(ma, 0);
        if (lmx_msg_root_attach(ma, 0) != LMX_MSG_INVALID
            || lmx_msg_root_attach(ma, foreign) != LMX_MSG_INVALID
            || ma->roots != 0
            || lmx_owned_ranges_find(ma->ranges, keep) == 0) {
            fprintf(stderr, "explicit root reject mutated storage\n");
            free(foreign);
            lmx_msg_runtime_delete(rtr);
            return 1;
        }
        if (lmx_msg_root_attach(ma, keep) != LMX_MSG_OK
            || lmx_msg_root_attach(ma, keep) != LMX_MSG_DUPLICATE) {
            fprintf(stderr, "explicit root attach\n");
            free(foreign);
            lmx_msg_runtime_delete(rtr);
            return 1;
        }
        if (lmx_msg_end_turn(rtr, a, 1) != LMX_MSG_OK
            || lmx_owned_ranges_find(ma->ranges, keep) == 0
            || lmx_owned_ranges_find(ma->ranges, keep_back) == 0
            || keep->len != 3U) {
            fprintf(stderr, "explicit root dropped attached INT\n");
            free(foreign);
            lmx_msg_runtime_delete(rtr);
            return 1;
        }
        if (lmx_owned_ranges_find(ma->ranges, drop) != 0
            || lmx_owned_ranges_find(ma->ranges, drop_back) != 0) {
            fprintf(stderr, "explicit root kept unrooted CHAR\n");
            free(foreign);
            lmx_msg_runtime_delete(rtr);
            return 1;
        }
        if (lmx_msg_root_release(ma, keep) != LMX_MSG_OK || ma->roots != 0) {
            fprintf(stderr, "explicit root release\n");
            free(foreign);
            lmx_msg_runtime_delete(rtr);
            return 1;
        }
        if (lmx_msg_end_turn(rtr, a, 1) != LMX_MSG_OK
            || lmx_owned_ranges_find(ma->ranges, keep) != 0
            || lmx_owned_ranges_find(ma->ranges, keep_back) != 0
            || ma->blocks != 0) {
            fprintf(stderr, "explicit root release left payload\n");
            free(foreign);
            lmx_msg_runtime_delete(rtr);
            return 1;
        }
        if (lmx_msg_end_turn(rtr, a, 1) != LMX_MSG_OK || ma->blocks != 0 || ma->roots != 0) {
            fprintf(stderr, "explicit root second end_turn\n");
            free(foreign);
            lmx_msg_runtime_delete(rtr);
            return 1;
        }
        free(foreign);
        fprintf(stderr, "explicit root: graph unset keeps attached INT; release reclaims; no double-free\n");
        lmx_msg_runtime_delete(rtr);
    }
    {
        LmxMsgRuntime *rtg;
        LmxMsgAddr a = 0;
        uchar ini = 9;
        LmxMsg *ma;
        Lmx *unit;
        void *children;
        LmxArrayDesc *buf;
        void *buf_back;
        rtg = lmx_msg_runtime_new();
        if (rtg == 0 || lmx_msg_create(rtg, 0, 1, &ini, 1, &a) != LMX_MSG_OK) {
            fprintf(stderr, "graph root create\n");
            return 1;
        }
        ma = lmx_msg_find(rtg, a);
        unit = (ma == 0) ? 0 : lmx_node_new_owned(&ma->blocks, &ma->ranges);
        if (ma == 0 || unit == 0 || lmx_branch_open_owned(unit, 1U, &ma->blocks, &ma->ranges) != 0) {
            fprintf(stderr, "graph root unit\n");
            lmx_msg_runtime_delete(rtg);
            return 1;
        }
        children = unit->data;
        buf = lmx_array_new_positive_owned(LMX_TYPE_ARRAY_OF_INT, 3U, &ma->blocks, &ma->ranges);
        if (buf == 0 || buf->data == 0
            || lmx_branch_store_known(unit, 0U, buf) != 0) {
            fprintf(stderr, "graph root fields\n");
            lmx_msg_runtime_delete(rtg);
            return 1;
        }
        buf_back = buf->data;
        lmx_msg_set_graph(ma, 0);
        if (lmx_msg_root_attach(ma, children) != LMX_MSG_OK) {
            fprintf(stderr, "children root attach\n");
            lmx_msg_runtime_delete(rtg);
            return 1;
        }
        if (lmx_msg_end_turn(rtg, a, 1) != LMX_MSG_OK
            || lmx_owned_ranges_find(ma->ranges, unit) != 0
            || lmx_owned_ranges_find(ma->ranges, children) == 0
            || lmx_owned_ranges_find(ma->ranges, buf) == 0
            || lmx_owned_ranges_find(ma->ranges, buf_back) == 0) {
            fprintf(stderr, "bare CHILDREN root dropped a referenced Array\n");
            lmx_msg_runtime_delete(rtg);
            return 1;
        }
        if (lmx_msg_root_release(ma, children) != LMX_MSG_OK
            || lmx_msg_end_turn(rtg, a, 1) != LMX_MSG_OK
            || lmx_owned_ranges_find(ma->ranges, children) != 0
            || lmx_owned_ranges_find(ma->ranges, buf) != 0
            || lmx_owned_ranges_find(ma->ranges, buf_back) != 0
            || ma->blocks != 0) {
            fprintf(stderr, "bare CHILDREN root release left payload\n");
            lmx_msg_runtime_delete(rtg);
            return 1;
        }
        fprintf(stderr, "explicit root: bare CHILDREN keeps Array descriptor/backing; release reclaims\n");
        lmx_msg_runtime_delete(rtg);
    }
    {
        LmxMsgRuntime *rtd;
        LmxMsgAddr a = 0;
        uchar ini = 10;
        LmxMsg *ma;
        LmxArrayDesc *of_desc;
        LmxArrayDesc *chars;
        LmxArrayDesc *dead;
        void *char_back;
        void *dead_back;
        rtd = lmx_msg_runtime_new();
        if (rtd == 0 || lmx_msg_create(rtd, 0, 1, &ini, 1, &a) != LMX_MSG_OK) {
            fprintf(stderr, "ref root create\n");
            return 1;
        }
        ma = lmx_msg_find(rtd, a);
        of_desc = (ma == 0) ? 0 : lmx_array_ref_new_positive_owned(LMX_TYPE_ARRAY_OF_DESC, 1U, &ma->blocks, &ma->ranges);
        chars = (ma == 0) ? 0 : lmx_array_new_positive_owned(LMX_TYPE_ARRAY_OF_CHAR, 4U, &ma->blocks, &ma->ranges);
        dead = (ma == 0) ? 0 : lmx_array_new_positive_owned(LMX_TYPE_ARRAY_OF_INT, 2U, &ma->blocks, &ma->ranges);
        if (ma == 0 || of_desc == 0 || chars == 0 || dead == 0
            || of_desc->data == 0 || chars->data == 0 || dead->data == 0) {
            fprintf(stderr, "ref root new\n");
            lmx_msg_runtime_delete(rtd);
            return 1;
        }
        ((LmxArrayDesc **)of_desc->data)[0] = chars;
        char_back = chars->data;
        dead_back = dead->data;
        lmx_msg_set_graph(ma, 0);
        if (lmx_msg_root_attach(ma, of_desc) != LMX_MSG_OK
            || lmx_msg_end_turn(rtd, a, 1) != LMX_MSG_OK
            || lmx_owned_ranges_find(ma->ranges, of_desc) == 0
            || lmx_owned_ranges_find(ma->ranges, chars) == 0
            || lmx_owned_ranges_find(ma->ranges, char_back) == 0) {
            fprintf(stderr, "ref root dropped DESC referents\n");
            lmx_msg_runtime_delete(rtd);
            return 1;
        }
        if (lmx_owned_ranges_find(ma->ranges, dead) != 0
            || lmx_owned_ranges_find(ma->ranges, dead_back) != 0) {
            fprintf(stderr, "ref root kept unrooted INT\n");
            lmx_msg_runtime_delete(rtd);
            return 1;
        }
        fprintf(stderr, "explicit root: ARRAY_OF_DESC keeps CHAR referent; unrooted INT dies\n");
        lmx_msg_runtime_delete(rtd);
    }
    {
        LmxMsgRuntime *rto;
        LmxMsgAddr a = 0;
        uchar ini = 11;
        LmxMsg *ma;
        LmxArrayDesc *keep;
        LmxMsgRoot *saved;
        rto = lmx_msg_runtime_new();
        if (rto == 0 || lmx_msg_create(rto, 0, 1, &ini, 1, &a) != LMX_MSG_OK) {
            fprintf(stderr, "root oom create\n");
            return 1;
        }
        ma = lmx_msg_find(rto, a);
        keep = (ma == 0) ? 0 : lmx_array_new_positive_owned(LMX_TYPE_ARRAY_OF_INT, 3U, &ma->blocks, &ma->ranges);
        if (ma == 0 || keep == 0) {
            fprintf(stderr, "root oom new\n");
            lmx_msg_runtime_delete(rto);
            return 1;
        }
        saved = ma->roots;
        lmx_msg_test_set_root_alloc_fail(1);
        if (lmx_msg_root_attach(ma, keep) != LMX_MSG_NOMEM
            || ma->roots != saved
            || lmx_owned_ranges_find(ma->ranges, keep) == 0) {
            fprintf(stderr, "root oom mutated heads\n");
            lmx_msg_test_set_root_alloc_fail(0);
            lmx_msg_runtime_delete(rto);
            return 1;
        }
        lmx_msg_test_set_root_alloc_fail(0);
        if (lmx_msg_root_attach(ma, keep) != LMX_MSG_OK) {
            fprintf(stderr, "root oom retry\n");
            lmx_msg_runtime_delete(rto);
            return 1;
        }
        fprintf(stderr, "explicit root: attach OOM preserves heads; leftover teardown\n");
        lmx_msg_runtime_delete(rto);
    }
    {
        LmxMsgRuntime *rtt;
        LmxMsgAddr dummy = 0, p = 0, c = 0;
        uchar ini = 12;
        LmxMsg *child;
        LmxMsg *parent;
        LmxArrayDesc *keep;
        LmxMsgEnv e;
        TurnCtx tctx;
        memset(&tctx, 0, sizeof(tctx));
        rtt = lmx_msg_runtime_new();
        if (rtt == 0 || lmx_msg_create(rtt, 0, 1, &ini, 1, &dummy) != LMX_MSG_OK
            || lmx_msg_create(rtt, dummy, 2, &ini, 1, &p) != LMX_MSG_OK
            || lmx_msg_end_turn(rtt, dummy, 1) != LMX_MSG_OK
            || lmx_msg_create(rtt, p, 3, &ini, 1, &c) != LMX_MSG_OK
            || lmx_msg_end_turn(rtt, p, 1) != LMX_MSG_OK) {
            fprintf(stderr, "root transfer create\n");
            if (rtt != 0) {
                lmx_msg_runtime_delete(rtt);
            }
            return 1;
        }
        if (lmx_msg_exec_bind(rtt, c, turn_end_complete, &tctx, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK) {
            fprintf(stderr, "root transfer bind\n");
            lmx_msg_runtime_delete(rtt);
            return 1;
        }
        memset(&e, 0, sizeof(e));
        e.kind = LMX_MSG_KIND_BYTES;
        e.n = 1;
        e.bytes = &ini;
        if (lmx_msg_host_post(rtt, c, &e) != LMX_MSG_STAGED) {
            fprintf(stderr, "root transfer post\n");
            lmx_msg_runtime_delete(rtt);
            return 1;
        }
        (void)lmx_msg_host_drain(rtt);
        (void)lmx_msg_run_child_turn(rtt, c);
        child = lmx_msg_find(rtt, c);
        parent = lmx_msg_find(rtt, p);
        keep = (child == 0) ? 0 : lmx_array_new_positive_owned(LMX_TYPE_ARRAY_OF_INT, 3U, &child->blocks, &child->ranges);
        if (child == 0 || parent == 0 || keep == 0
            || lmx_msg_root_attach(child, keep) != LMX_MSG_OK
            || child->roots == 0) {
            fprintf(stderr, "root transfer attach\n");
            lmx_msg_runtime_delete(rtt);
            return 1;
        }
        if (lmx_msg_transfer_adopted(rtt, c, p) != LMX_MSG_OK) {
            fprintf(stderr, "root transfer move ready=%d users=%d run=%u parent=%d blocks=%d ranges=%d\n",
                child->handoff_ready, child->native_users, (unsigned)lmx_msg_running_load(child),
                child->parent_msg == parent, child->blocks != 0, child->ranges != 0);
            lmx_msg_runtime_delete(rtt);
            return 1;
        }
        if (child->roots != 0 || child->blocks != 0 || child->ranges != 0
            || lmx_owned_ranges_find(parent->ranges, keep) == 0
            || parent->roots != 0) {
            fprintf(stderr, "root transfer stale source or inherited dest roots\n");
            lmx_msg_runtime_delete(rtt);
            return 1;
        }
        lmx_msg_set_graph(parent, 0);
        if (lmx_msg_end_turn(rtt, p, 1) != LMX_MSG_OK
            || lmx_owned_ranges_find(parent->ranges, keep) != 0) {
            fprintf(stderr, "root transfer dest kept unrooted payload\n");
            lmx_msg_runtime_delete(rtt);
            return 1;
        }
        fprintf(stderr, "explicit root: handoff drops stale source roots; dest does not inherit\n");
        lmx_msg_runtime_delete(rtt);
    }
    {
        LmxMsgRuntime *rtg;
        LmxMsgAddr dummy = 0, p = 0, c = 0;
        uchar ini = 17;
        LmxMsg *child;
        LmxMsg *parent;
        Lmx *root;
        Lmx *inner;
        LmxArrayDesc *array;
        int *cells;
        int foreign = 0;
        LmxMsgBlock *child_blocks;
        LmxOwnedRange *child_ranges;
        LmxMsgBlock *parent_blocks;
        LmxOwnedRange *parent_ranges;
        LmxMsgRoot *child_roots;
        LmxMsgRoot *parent_roots;
        LmxMsgEnv e;
        TurnCtx tctx;
        memset(&tctx, 0, sizeof(tctx));
        rtg = lmx_msg_runtime_new();
        if (rtg == 0 || lmx_msg_create(rtg, 0, 1, &ini, 1, &dummy) != LMX_MSG_OK
            || lmx_msg_create(rtg, dummy, 2, &ini, 1, &p) != LMX_MSG_OK
            || lmx_msg_end_turn(rtg, dummy, 1) != LMX_MSG_OK
            || lmx_msg_create(rtg, p, 3, &ini, 1, &c) != LMX_MSG_OK
            || lmx_msg_end_turn(rtg, p, 1) != LMX_MSG_OK
            || lmx_msg_exec_bind(rtg, c, turn_end_complete, &tctx,
                                 LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK) {
            fprintf(stderr, "graph transfer create/bind\n");
            if (rtg != 0) {
                lmx_msg_runtime_delete(rtg);
            }
            return 1;
        }
        memset(&e, 0, sizeof(e));
        e.kind = LMX_MSG_KIND_BYTES;
        e.n = 1;
        e.bytes = &ini;
        if (lmx_msg_host_post(rtg, c, &e) != LMX_MSG_STAGED) {
            fprintf(stderr, "graph transfer post\n");
            lmx_msg_runtime_delete(rtg);
            return 1;
        }
        (void)lmx_msg_host_drain(rtg);
        (void)lmx_msg_run_child_turn(rtg, c);
        child = lmx_msg_find(rtg, c);
        parent = lmx_msg_find(rtg, p);
        root = child == 0 ? 0 : lmx_node_new_owned(&child->blocks, &child->ranges);
        inner = child == 0 ? 0 : lmx_struct_new_owned(root, &child->blocks, &child->ranges);
        array = child == 0 ? 0 : lmx_array_new_positive_owned(
            LMX_TYPE_ARRAY_OF_INT, 3U, &child->blocks, &child->ranges);
        if (child == 0 || parent == 0 || root == 0 || inner == 0 || array == 0
            || array->data == 0
            || lmx_branch_open_owned(root, 2U, &child->blocks, &child->ranges) != 0
            || lmx_branch_open_owned(inner, 1U, &child->blocks, &child->ranges) != 0
            || lmx_branch_store_known(root, 0U, inner) != 0
            || lmx_branch_store_known(root, 1U, array) != 0
            || lmx_branch_store_known(inner, 0U, root) != 0
            || lmx_msg_root_attach(child, inner) != LMX_MSG_OK) {
            fprintf(stderr, "graph transfer fixture\n");
            lmx_msg_runtime_delete(rtg);
            return 1;
        }
        cells = (int *)array->data;
        cells[0] = 11;
        cells[2] = 29;
        lmx_msg_set_graph(child, root);
        child_blocks = child->blocks;
        child_ranges = child->ranges;
        parent_blocks = parent->blocks;
        parent_ranges = parent->ranges;
        child_roots = child->roots;
        parent_roots = parent->roots;
        lmx_msg_test_set_root_alloc_fail(1);
        if (lmx_msg_transfer_graph(rtg, c, p, root) != LMX_MSG_NOMEM
            || child->blocks != child_blocks || child->ranges != child_ranges
            || child->graph != root || child->roots != child_roots
            || parent->blocks != parent_blocks || parent->ranges != parent_ranges
            || parent->roots != parent_roots) {
            fprintf(stderr, "graph transfer OOM mutated owners\n");
            lmx_msg_test_set_root_alloc_fail(0);
            lmx_msg_runtime_delete(rtg);
            return 1;
        }
        lmx_msg_test_set_root_alloc_fail(0);
        if (lmx_msg_transfer_graph(rtg, c, p, (Lmx *)&foreign) != LMX_MSG_INVALID
            || lmx_msg_transfer_graph(rtg, c, dummy, root) != LMX_MSG_INVALID
            || child->blocks != child_blocks || child->ranges != child_ranges) {
            fprintf(stderr, "graph transfer invalid mutated owners\n");
            lmx_msg_runtime_delete(rtg);
            return 1;
        }
        if (lmx_msg_transfer_graph(rtg, c, p, root) != LMX_MSG_OK
            || child->blocks != 0 || child->ranges != 0 || child->graph != 0
            || child->roots != 0
            || lmx_owned_ranges_find(parent->ranges, root) == 0
            || lmx_owned_ranges_find(parent->ranges, inner) == 0
            || lmx_owned_ranges_find(parent->ranges, array) == 0
            || lmx_owned_ranges_find(parent->ranges, array->data) == 0
            || parent->roots == 0 || parent->roots->p != root
            || (parent->roots->roles & LMX_MSG_ROOT_RETAIN) == 0
            || lmx_branch_child_known(root, 0U) != inner
            || lmx_branch_child_known(root, 1U) != array
            || lmx_branch_child_known(inner, 0U) != root
            || cells[0] != 11 || cells[2] != 29) {
            fprintf(stderr, "graph transfer changed graph identity/shape\n");
            lmx_msg_runtime_delete(rtg);
            return 1;
        }
        if (lmx_msg_dispose_child(rtg, p, c) != LMX_MSG_OK
            || lmx_msg_end_turn(rtg, p, 1) != LMX_MSG_OK
            || lmx_owned_ranges_find(parent->ranges, root) == 0
            || lmx_msg_root_release(parent, root) != LMX_MSG_OK
            || lmx_msg_end_turn(rtg, p, 1) != LMX_MSG_OK
            || lmx_owned_ranges_find(parent->ranges, root) != 0) {
            fprintf(stderr, "graph transfer retain/release lifecycle\n");
            lmx_msg_runtime_delete(rtg);
            return 1;
        }
        fprintf(stderr, "graph transfer: exact addresses retained, released once\n");
        lmx_msg_runtime_delete(rtg);
    }
    {
        LmxMsgRuntime *rtd;
        LmxMsgAddr top = 0, p = 0, srca = 0, dsta = 0;
        uchar ini = 23;
        LmxMsg *src;
        LmxMsg *dst;
        LmxMsg *old_parent;
        Lmx *root;
        Lmx *inner;
        LmxMsgEnv e;
        TurnCtx tctx;
        memset(&tctx, 0, sizeof(tctx));
        rtd = lmx_msg_runtime_new();
        if (rtd == 0 || lmx_msg_create(rtd, 0, 1, &ini, 1, &top) != LMX_MSG_OK
            || lmx_msg_create(rtd, top, 2, &ini, 1, &p) != LMX_MSG_OK
            || lmx_msg_end_turn(rtd, top, 1) != LMX_MSG_OK
            || lmx_msg_create(rtd, p, 3, &ini, 1, &srca) != LMX_MSG_OK
            || lmx_msg_create(rtd, p, 4, &ini, 1, &dsta) != LMX_MSG_OK
            || lmx_msg_end_turn(rtd, p, 1) != LMX_MSG_OK
            || lmx_msg_exec_bind(rtd, srca, turn_end_complete, &tctx,
                                 LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK) {
            fprintf(stderr, "graph delivery create/bind\n");
            if (rtd != 0) {
                lmx_msg_runtime_delete(rtd);
            }
            return 1;
        }
        memset(&e, 0, sizeof(e));
        e.kind = LMX_MSG_KIND_BYTES;
        e.n = 1;
        e.bytes = &ini;
        if (lmx_msg_host_post(rtd, srca, &e) != LMX_MSG_STAGED) {
            fprintf(stderr, "graph delivery post\n");
            lmx_msg_runtime_delete(rtd);
            return 1;
        }
        (void)lmx_msg_host_drain(rtd);
        (void)lmx_msg_run_child_turn(rtd, srca);
        src = lmx_msg_find(rtd, srca);
        dst = lmx_msg_find(rtd, dsta);
        old_parent = lmx_msg_find(rtd, p);
        root = src == 0 ? 0 : lmx_node_new_owned(&src->blocks, &src->ranges);
        inner = src == 0 ? 0 : lmx_struct_new_owned(root, &src->blocks, &src->ranges);
        if (src == 0 || dst == 0 || old_parent == 0 || root == 0 || inner == 0
            || lmx_branch_open_owned(root, 1U, &src->blocks, &src->ranges) != 0
            || lmx_branch_store_known(root, 0U, inner) != 0) {
            fprintf(stderr, "graph delivery fixture\n");
            lmx_msg_runtime_delete(rtd);
            return 1;
        }
        lmx_msg_set_graph(src, root);
        if (lmx_msg_deliver_graph(rtd, srca, dsta, root) != LMX_MSG_OK
            || src->blocks != 0 || src->ranges != 0 || src->graph != 0
            || src->roots != 0 || src->tracked != 0
            || src->parent_msg != old_parent || dst->parent_msg != old_parent
            || lmx_owned_ranges_find(dst->ranges, root) == 0
            || lmx_owned_ranges_find(dst->ranges, inner) == 0
            || dst->roots == 0 || dst->roots->p != root
            || lmx_branch_child_known(root, 0U) != inner
            || inner->node != root) {
            fprintf(stderr, "graph delivery identity/ownership\n");
            lmx_msg_runtime_delete(rtd);
            return 1;
        }
        if (lmx_msg_dispose_child(rtd, p, srca) != LMX_MSG_OK
            || lmx_msg_root_release(dst, root) != LMX_MSG_OK
            || lmx_msg_end_turn(rtd, dsta, 1) != LMX_MSG_OK
            || lmx_owned_ranges_find(dst->ranges, root) != 0) {
            fprintf(stderr, "graph delivery lifecycle\n");
            lmx_msg_runtime_delete(rtd);
            return 1;
        }
        fprintf(stderr, "graph delivery: sibling receives exact addresses; lifecycle parent unchanged\n");
        lmx_msg_runtime_delete(rtd);
    }
    {
        LmxMsgRuntime *rth;
        LmxMsgAddr dummy = 0, p = 0, c = 0;
        uchar ini = 13;
        LmxMsg *child;
        LmxMsg *parent;
        Lmx *unit;
        LmxArrayDesc *hist;
        LmxArrayDesc *dead;
        void *hist_back;
        void *dead_back;
        int *cells;
        rth = lmx_msg_runtime_new();
        if (rth == 0 || lmx_msg_create(rth, 0, 1, &ini, 1, &dummy) != LMX_MSG_OK
            || lmx_msg_create(rth, dummy, 2, &ini, 1, &p) != LMX_MSG_OK
            || lmx_msg_end_turn(rth, dummy, 1) != LMX_MSG_OK
            || lmx_msg_create(rth, p, 3, &ini, 1, &c) != LMX_MSG_OK
            || lmx_msg_end_turn(rth, p, 1) != LMX_MSG_OK) {
            fprintf(stderr, "fail-history create\n");
            if (rth != 0) {
                lmx_msg_runtime_delete(rth);
            }
            return 1;
        }
        child = lmx_msg_find(rth, c);
        parent = lmx_msg_find(rth, p);
        unit = (child == 0) ? 0 : lmx_node_new_owned(&child->blocks, &child->ranges);
        if (child == 0 || parent == 0 || unit == 0
            || lmx_branch_open_owned(unit, 1U, &child->blocks, &child->ranges) != 0) {
            fprintf(stderr, "fail-history unit\n");
            lmx_msg_runtime_delete(rth);
            return 1;
        }
        hist = lmx_array_new_positive_owned(LMX_TYPE_ARRAY_OF_INT, 1U, &child->blocks, &child->ranges);
        if (hist == 0 || hist->data == 0
            || lmx_branch_store_known(unit, 0U, hist) != 0) {
            fprintf(stderr, "fail-history field\n");
            lmx_msg_runtime_delete(rth);
            return 1;
        }
        cells = (int *)hist->data;
        cells[0] = 7;
        hist_back = hist->data;
        lmx_msg_set_graph(child, unit);
        if (lmx_msg_exec_bind(rth, c, turn_fail_end, 0, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK
            || lmx_msg_emergency_cancel(rth, c) != LMX_MSG_OK
            || lmx_msg_run_child_turn(rth, c) != LMX_MSG_OK
            || lmx_msg_adopt_failed(rth, p, c) != LMX_MSG_OK) {
            fprintf(stderr, "fail-history adopt\n");
            lmx_msg_runtime_delete(rth);
            return 1;
        }
        dead = lmx_array_new_positive_owned(LMX_TYPE_ARRAY_OF_INT, 2U, &parent->blocks, &parent->ranges);
        if (dead == 0 || dead->data == 0) {
            fprintf(stderr, "fail-history neighbour\n");
            lmx_msg_runtime_delete(rth);
            return 1;
        }
        dead_back = dead->data;
        lmx_msg_set_graph(parent, 0);
        if (parent->roots == 0
            || lmx_msg_end_turn(rth, p, 1) != LMX_MSG_OK
            || lmx_owned_ranges_find(parent->ranges, hist) == 0
            || lmx_owned_ranges_find(parent->ranges, hist_back) == 0
            || cells[0] != 7) {
            fprintf(stderr, "fail-history dropped\n");
            lmx_msg_runtime_delete(rth);
            return 1;
        }
        if (lmx_owned_ranges_find(parent->ranges, dead) != 0
            || lmx_owned_ranges_find(parent->ranges, dead_back) != 0) {
            fprintf(stderr, "fail-history neighbour immortal\n");
            lmx_msg_runtime_delete(rth);
            return 1;
        }
        fprintf(stderr, "adopt_failed history: parent root keeps failed graph 7; neighbour dies\n");
        lmx_msg_runtime_delete(rth);
    }
    {
        LmxMsgRuntime *rtp;
        LmxMsgAddr dummy = 0, p = 0, c = 0;
        uchar ini = 23;
        LmxMsg *child;
        LmxMsg *parent;
        int *value;
        int *dead;
        LmxMsgRoot *history;
        rtp = lmx_msg_runtime_new();
        if (rtp == 0 || lmx_msg_create(rtp, 0, 1, &ini, 1, &dummy) != LMX_MSG_OK
            || lmx_msg_create(rtp, dummy, 2, &ini, 1, &p) != LMX_MSG_OK
            || lmx_msg_end_turn(rtp, dummy, 1) != LMX_MSG_OK
            || lmx_msg_create(rtp, p, 3, &ini, 1, &c) != LMX_MSG_OK
            || lmx_msg_end_turn(rtp, p, 1) != LMX_MSG_OK) {
            fprintf(stderr, "primitive history create\n");
            if (rtp != 0) {
                lmx_msg_runtime_delete(rtp);
            }
            return 1;
        }
        child = lmx_msg_find(rtp, c);
        parent = lmx_msg_find(rtp, p);
        value = (child == 0) ? 0 : lmx_int_new_owned(&child->blocks, &child->ranges);
        if (child == 0 || parent == 0 || value == 0) {
            fprintf(stderr, "primitive history value\n");
            lmx_msg_runtime_delete(rtp);
            return 1;
        }
        *value = 29;
        lmx_msg_set_graph(child, (Lmx *)value);
        if (lmx_msg_exec_bind(rtp, c, turn_fail_end, 0, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK
            || lmx_msg_emergency_cancel(rtp, c) != LMX_MSG_OK
            || lmx_msg_run_child_turn(rtp, c) != LMX_MSG_OK
            || lmx_msg_adopt_failed(rtp, p, c) != LMX_MSG_OK) {
            fprintf(stderr, "primitive history adopt\n");
            lmx_msg_runtime_delete(rtp);
            return 1;
        }
        history = parent->roots;
        while (history != 0 && history->p != value) {
            history = history->next;
        }
        dead = lmx_int_new_owned(&parent->blocks, &parent->ranges);
        if (history == 0 || (history->roles & LMX_MSG_ROOT_HISTORY) == 0 || dead == 0) {
            fprintf(stderr, "primitive history root\n");
            lmx_msg_runtime_delete(rtp);
            return 1;
        }
        *dead = 31;
        lmx_msg_set_graph(parent, 0);
        if (lmx_msg_end_turn(rtp, p, 1) != LMX_MSG_OK
            || lmx_owned_ranges_find(parent->ranges, value) == 0
            || *value != 29
            || lmx_owned_ranges_find(parent->ranges, dead) != 0) {
            fprintf(stderr, "primitive history was not retained selectively\n");
            lmx_msg_runtime_delete(rtp);
            return 1;
        }
        fprintf(stderr, "adopt_failed history: primitive-only graph 29 retained; neighbour dies\n");
        lmx_msg_runtime_delete(rtp);
    }
    {
        LmxMsgRuntime *rtc;
        LmxMsgAddr a = 0;
        uchar ini = 24;
        LmxMsg *owner;
        int *eternal;
        int *dead;
        LmxMethod *method;
        rtc = lmx_msg_runtime_new();
        if (rtc == 0 || lmx_msg_create(rtc, 0, 1, &ini, 1, &a) != LMX_MSG_OK) {
            fprintf(stderr, "classifier retention create\n");
            if (rtc != 0) {
                lmx_msg_runtime_delete(rtc);
            }
            return 1;
        }
        owner = lmx_msg_find(rtc, a);
        eternal = owner == 0 ? 0 : lmx_int_new_owned(&owner->blocks, &owner->ranges);
        method = owner == 0 ? 0 : lmx_method_new_owned(&owner->blocks, &owner->ranges);
        dead = owner == 0 ? 0 : lmx_int_new_owned(&owner->blocks, &owner->ranges);
        if (owner == 0 || eternal == 0 || method == 0 || dead == 0
            || lmx_msg_bootstrap_eternal_admit(owner, eternal) != LMX_MSG_OK
            || lmx_msg_bootstrap_method_admit(owner, method) != LMX_MSG_OK) {
            fprintf(stderr, "classifier retention fixture\n");
            lmx_msg_runtime_delete(rtc);
            return 1;
        }
        *eternal = 37;
        *dead = 41;
        lmx_msg_set_graph(owner, 0);
        if (lmx_msg_end_turn(rtc, a, 1) != LMX_MSG_OK
            || lmx_owned_ranges_find(owner->ranges, eternal) == 0
            || lmx_owned_ranges_find(owner->ranges, method) == 0
            || lmx_owned_ranges_find(owner->ranges, dead) != 0
            || *eternal != 37) {
            fprintf(stderr, "classifier roots were not retained selectively\n");
            lmx_msg_runtime_delete(rtc);
            return 1;
        }
        fprintf(stderr, "classifier roots: eternal and METHOD retained; neighbour dies\n");
        lmx_msg_runtime_delete(rtc);
    }
    {
        LmxMsgRuntime *rtrp;
        LmxMsgAddr a = 0;
        uchar ini = 25;
        LmxMsg *owner;
        int *primitive;
        int *dead;
        LmxMethod *method;
        rtrp = lmx_msg_runtime_new();
        if (rtrp == 0 || lmx_msg_create(rtrp, 0, 1, &ini, 1, &a) != LMX_MSG_OK) {
            fprintf(stderr, "primitive root create\n");
            if (rtrp != 0) {
                lmx_msg_runtime_delete(rtrp);
            }
            return 1;
        }
        owner = lmx_msg_find(rtrp, a);
        primitive = owner == 0 ? 0 : lmx_int_new_owned(&owner->blocks, &owner->ranges);
        method = owner == 0 ? 0 : lmx_method_new_owned(&owner->blocks, &owner->ranges);
        dead = owner == 0 ? 0 : lmx_int_new_owned(&owner->blocks, &owner->ranges);
        if (owner == 0 || primitive == 0 || method == 0 || dead == 0
            || lmx_msg_root_attach(owner, primitive) != LMX_MSG_OK
            || lmx_msg_root_attach(owner, method) != LMX_MSG_OK) {
            fprintf(stderr, "primitive root attach\n");
            lmx_msg_runtime_delete(rtrp);
            return 1;
        }
        *primitive = 43;
        *dead = 47;
        lmx_msg_set_graph(owner, 0);
        if (lmx_msg_end_turn(rtrp, a, 1) != LMX_MSG_OK
            || lmx_owned_ranges_find(owner->ranges, primitive) == 0
            || lmx_owned_ranges_find(owner->ranges, method) == 0
            || lmx_owned_ranges_find(owner->ranges, dead) != 0
            || *primitive != 43
            || lmx_msg_root_release(owner, primitive) != LMX_MSG_OK
            || lmx_msg_root_release(owner, method) != LMX_MSG_OK
            || lmx_msg_end_turn(rtrp, a, 1) != LMX_MSG_OK
            || lmx_owned_ranges_find(owner->ranges, primitive) != 0
            || lmx_owned_ranges_find(owner->ranges, method) != 0) {
            fprintf(stderr, "primitive root lifecycle\n");
            lmx_msg_runtime_delete(rtrp);
            return 1;
        }
        fprintf(stderr, "explicit roots: primitive and METHOD retain/release\n");
        lmx_msg_runtime_delete(rtrp);
    }
    {
        LmxMsgRuntime *rto;
        LmxMsgAddr dummy = 0, p = 0, c = 0;
        uchar ini = 14;
        LmxMsg *child;
        LmxMsg *parent;
        Lmx *unit;
        LmxArrayDesc *hist;
        LmxMsgBlock *child_blocks;
        LmxOwnedRange *child_ranges;
        LmxMsgRoot *parent_roots;
        LmxMsgBlock *parent_blocks;
        LmxOwnedRange *parent_ranges;
        int *cells;
        rto = lmx_msg_runtime_new();
        if (rto == 0 || lmx_msg_create(rto, 0, 1, &ini, 1, &dummy) != LMX_MSG_OK
            || lmx_msg_create(rto, dummy, 2, &ini, 1, &p) != LMX_MSG_OK
            || lmx_msg_end_turn(rto, dummy, 1) != LMX_MSG_OK
            || lmx_msg_create(rto, p, 3, &ini, 1, &c) != LMX_MSG_OK
            || lmx_msg_end_turn(rto, p, 1) != LMX_MSG_OK) {
            fprintf(stderr, "fail-history oom create\n");
            if (rto != 0) {
                lmx_msg_runtime_delete(rto);
            }
            return 1;
        }
        child = lmx_msg_find(rto, c);
        parent = lmx_msg_find(rto, p);
        unit = (child == 0) ? 0 : lmx_node_new_owned(&child->blocks, &child->ranges);
        if (child == 0 || parent == 0 || unit == 0
            || lmx_branch_open_owned(unit, 1U, &child->blocks, &child->ranges) != 0) {
            fprintf(stderr, "fail-history oom unit\n");
            lmx_msg_runtime_delete(rto);
            return 1;
        }
        hist = lmx_array_new_positive_owned(LMX_TYPE_ARRAY_OF_INT, 1U, &child->blocks, &child->ranges);
        if (hist == 0 || hist->data == 0
            || lmx_branch_store_known(unit, 0U, hist) != 0) {
            fprintf(stderr, "fail-history oom field\n");
            lmx_msg_runtime_delete(rto);
            return 1;
        }
        cells = (int *)hist->data;
        cells[0] = 7;
        lmx_msg_set_graph(child, unit);
        if (lmx_msg_exec_bind(rto, c, turn_fail_end, 0, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK
            || lmx_msg_emergency_cancel(rto, c) != LMX_MSG_OK
            || lmx_msg_run_child_turn(rto, c) != LMX_MSG_OK) {
            fprintf(stderr, "fail-history oom fail\n");
            lmx_msg_runtime_delete(rto);
            return 1;
        }
        child_blocks = child->blocks;
        child_ranges = child->ranges;
        parent_roots = parent->roots;
        parent_blocks = parent->blocks;
        parent_ranges = parent->ranges;
        lmx_msg_test_set_root_alloc_fail(1);
        if (lmx_msg_adopt_failed(rto, p, c) != LMX_MSG_NOMEM
            || child->disposed != 0
            || child->blocks != child_blocks || child->ranges != child_ranges
            || child->graph != unit
            || parent->roots != parent_roots
            || parent->blocks != parent_blocks || parent->ranges != parent_ranges) {
            fprintf(stderr, "fail-history oom mutated owners\n");
            lmx_msg_test_set_root_alloc_fail(0);
            lmx_msg_runtime_delete(rto);
            return 1;
        }
        lmx_msg_test_set_root_alloc_fail(0);
        if (lmx_msg_adopt_failed(rto, p, c) != LMX_MSG_OK
            || lmx_msg_end_turn(rto, p, 1) != LMX_MSG_OK
            || lmx_owned_ranges_find(parent->ranges, hist) == 0
            || cells[0] != 7) {
            fprintf(stderr, "fail-history oom retry dropped\n");
            lmx_msg_runtime_delete(rto);
            return 1;
        }
        fprintf(stderr, "adopt_failed history: OOM leaves both owners; retry keeps 7\n");
        lmx_msg_runtime_delete(rto);
    }
    {
        LmxMsgRuntime *rtn;
        LmxMsgAddr dummy = 0, p = 0, c = 0, g = 0;
        uchar ini = 15;
        LmxMsg *pm;
        LmxMsg *cm;
        LmxMsg *gm;
        Lmx *cunit;
        Lmx *gunit;
        LmxArrayDesc *cbuf;
        LmxArrayDesc *gbuf;
        LmxArrayDesc *dead;
        void *dead_back;
        int *ccells;
        int *gcells;
        rtn = lmx_msg_runtime_new();
        if (rtn == 0 || lmx_msg_create(rtn, 0, 1, &ini, 1, &dummy) != LMX_MSG_OK
            || lmx_msg_create(rtn, dummy, 2, &ini, 1, &p) != LMX_MSG_OK
            || lmx_msg_end_turn(rtn, dummy, 1) != LMX_MSG_OK
            || lmx_msg_create(rtn, p, 3, &ini, 1, &c) != LMX_MSG_OK
            || lmx_msg_end_turn(rtn, p, 1) != LMX_MSG_OK
            || lmx_msg_create(rtn, c, 4, &ini, 1, &g) != LMX_MSG_OK
            || lmx_msg_end_turn(rtn, c, 1) != LMX_MSG_OK) {
            fprintf(stderr, "nested history create\n");
            if (rtn != 0) {
                lmx_msg_runtime_delete(rtn);
            }
            return 1;
        }
        pm = lmx_msg_find(rtn, p);
        cm = lmx_msg_find(rtn, c);
        gm = lmx_msg_find(rtn, g);
        cunit = (cm == 0) ? 0 : lmx_node_new_owned(&cm->blocks, &cm->ranges);
        gunit = (gm == 0) ? 0 : lmx_node_new_owned(&gm->blocks, &gm->ranges);
        if (pm == 0 || cm == 0 || gm == 0 || cunit == 0 || gunit == 0
            || lmx_branch_open_owned(cunit, 1U, &cm->blocks, &cm->ranges) != 0
            || lmx_branch_open_owned(gunit, 1U, &gm->blocks, &gm->ranges) != 0) {
            fprintf(stderr, "nested history unit\n");
            lmx_msg_runtime_delete(rtn);
            return 1;
        }
        cbuf = lmx_array_new_positive_owned(LMX_TYPE_ARRAY_OF_INT, 1U, &cm->blocks, &cm->ranges);
        gbuf = lmx_array_new_positive_owned(LMX_TYPE_ARRAY_OF_INT, 1U, &gm->blocks, &gm->ranges);
        if (cbuf == 0 || gbuf == 0
            || cbuf->data == 0 || gbuf->data == 0
            || lmx_branch_store_known(cunit, 0U, cbuf) != 0
            || lmx_branch_store_known(gunit, 0U, gbuf) != 0) {
            fprintf(stderr, "nested history fields\n");
            lmx_msg_runtime_delete(rtn);
            return 1;
        }
        ccells = (int *)cbuf->data;
        gcells = (int *)gbuf->data;
        ccells[0] = 9;
        gcells[0] = 7;
        lmx_msg_set_graph(cm, cunit);
        lmx_msg_set_graph(gm, gunit);
        if (lmx_msg_exec_bind(rtn, g, turn_fail_end, 0, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK
            || lmx_msg_emergency_cancel(rtn, g) != LMX_MSG_OK
            || lmx_msg_run_child_turn(rtn, g) != LMX_MSG_OK
            || lmx_msg_adopt_failed(rtn, c, g) != LMX_MSG_OK) {
            fprintf(stderr, "nested history G->C\n");
            lmx_msg_runtime_delete(rtn);
            return 1;
        }
        if (lmx_msg_exec_bind(rtn, c, turn_fail_end, 0, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK
            || lmx_msg_emergency_cancel(rtn, c) != LMX_MSG_OK
            || lmx_msg_run_child_turn(rtn, c) != LMX_MSG_OK
            || lmx_msg_adopt_failed(rtn, p, c) != LMX_MSG_OK) {
            fprintf(stderr, "nested history C->P\n");
            lmx_msg_runtime_delete(rtn);
            return 1;
        }
        dead = lmx_array_new_positive_owned(LMX_TYPE_ARRAY_OF_INT, 2U, &pm->blocks, &pm->ranges);
        if (dead == 0 || dead->data == 0) {
            fprintf(stderr, "nested history neighbour\n");
            lmx_msg_runtime_delete(rtn);
            return 1;
        }
        dead_back = dead->data;
        lmx_msg_set_graph(pm, 0);
        if (lmx_msg_end_turn(rtn, p, 1) != LMX_MSG_OK
            || lmx_owned_ranges_find(pm->ranges, cbuf) == 0
            || lmx_owned_ranges_find(pm->ranges, gbuf) == 0
            || ccells[0] != 9 || gcells[0] != 7) {
            fprintf(stderr, "nested history dropped C or G\n");
            lmx_msg_runtime_delete(rtn);
            return 1;
        }
        if (lmx_owned_ranges_find(pm->ranges, dead) != 0
            || lmx_owned_ranges_find(pm->ranges, dead_back) != 0) {
            fprintf(stderr, "nested history neighbour immortal\n");
            lmx_msg_runtime_delete(rtn);
            return 1;
        }
        fprintf(stderr, "adopt_failed nested: P keeps C=9 and G=7; neighbour dies\n");
        lmx_msg_runtime_delete(rtn);
    }
    {
        LmxMsgRuntime *rtr;
        LmxMsgAddr dummy = 0, p = 0, c = 0, g = 0;
        uchar ini = 16;
        LmxMsg *pm;
        LmxMsg *cm;
        LmxMsg *gm;
        Lmx *cunit;
        Lmx *gunit;
        LmxArrayDesc *cbuf;
        LmxArrayDesc *gbuf;
        LmxArrayDesc *temp;
        LmxMsgRoot *rg;
        void *temp_back;
        int *ccells;
        int *gcells;
        rtr = lmx_msg_runtime_new();
        if (rtr == 0 || lmx_msg_create(rtr, 0, 1, &ini, 1, &dummy) != LMX_MSG_OK
            || lmx_msg_create(rtr, dummy, 2, &ini, 1, &p) != LMX_MSG_OK
            || lmx_msg_end_turn(rtr, dummy, 1) != LMX_MSG_OK
            || lmx_msg_create(rtr, p, 3, &ini, 1, &c) != LMX_MSG_OK
            || lmx_msg_end_turn(rtr, p, 1) != LMX_MSG_OK
            || lmx_msg_create(rtr, c, 4, &ini, 1, &g) != LMX_MSG_OK
            || lmx_msg_end_turn(rtr, c, 1) != LMX_MSG_OK) {
            fprintf(stderr, "role history create\n");
            if (rtr != 0) {
                lmx_msg_runtime_delete(rtr);
            }
            return 1;
        }
        pm = lmx_msg_find(rtr, p);
        cm = lmx_msg_find(rtr, c);
        gm = lmx_msg_find(rtr, g);
        cunit = (cm == 0) ? 0 : lmx_node_new_owned(&cm->blocks, &cm->ranges);
        gunit = (gm == 0) ? 0 : lmx_node_new_owned(&gm->blocks, &gm->ranges);
        if (pm == 0 || cm == 0 || gm == 0 || cunit == 0 || gunit == 0
            || lmx_branch_open_owned(cunit, 1U, &cm->blocks, &cm->ranges) != 0
            || lmx_branch_open_owned(gunit, 1U, &gm->blocks, &gm->ranges) != 0) {
            fprintf(stderr, "role history unit\n");
            lmx_msg_runtime_delete(rtr);
            return 1;
        }
        cbuf = lmx_array_new_positive_owned(LMX_TYPE_ARRAY_OF_INT, 1U, &cm->blocks, &cm->ranges);
        gbuf = lmx_array_new_positive_owned(LMX_TYPE_ARRAY_OF_INT, 1U, &gm->blocks, &gm->ranges);
        temp = lmx_array_new_positive_owned(LMX_TYPE_ARRAY_OF_INT, 2U, &cm->blocks, &cm->ranges);
        if (cbuf == 0 || gbuf == 0 || temp == 0
            || cbuf->data == 0 || gbuf->data == 0 || temp->data == 0
            || lmx_branch_store_known(cunit, 0U, cbuf) != 0
            || lmx_branch_store_known(gunit, 0U, gbuf) != 0) {
            fprintf(stderr, "role history fields\n");
            lmx_msg_runtime_delete(rtr);
            return 1;
        }
        ccells = (int *)cbuf->data;
        gcells = (int *)gbuf->data;
        temp_back = temp->data;
        ccells[0] = 9;
        gcells[0] = 7;
        lmx_msg_set_graph(cm, cunit);
        lmx_msg_set_graph(gm, gunit);
        if (lmx_msg_exec_bind(rtr, g, turn_fail_end, 0, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK
            || lmx_msg_emergency_cancel(rtr, g) != LMX_MSG_OK
            || lmx_msg_run_child_turn(rtr, g) != LMX_MSG_OK
            || lmx_msg_adopt_failed(rtr, c, g) != LMX_MSG_OK
            || lmx_msg_root_attach(cm, temp) != LMX_MSG_OK
            || lmx_msg_root_attach(cm, gunit) != LMX_MSG_OK) {
            fprintf(stderr, "role history G->C attach\n");
            lmx_msg_runtime_delete(rtr);
            return 1;
        }
        if (lmx_msg_root_release(cm, gunit) != LMX_MSG_OK) {
            fprintf(stderr, "role history release retain\n");
            lmx_msg_runtime_delete(rtr);
            return 1;
        }
        rg = cm->roots;
        while (rg != 0 && rg->p != (void *)gunit) {
            rg = rg->next;
        }
        if (rg == 0 || (rg->roles & LMX_MSG_ROOT_HISTORY) == 0
            || (rg->roles & LMX_MSG_ROOT_RETAIN) != 0) {
            fprintf(stderr, "role history G lost HISTORY after RETAIN release\n");
            lmx_msg_runtime_delete(rtr);
            return 1;
        }
        if (lmx_msg_exec_bind(rtr, c, turn_fail_end, 0, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK
            || lmx_msg_emergency_cancel(rtr, c) != LMX_MSG_OK
            || lmx_msg_run_child_turn(rtr, c) != LMX_MSG_OK
            || lmx_msg_adopt_failed(rtr, p, c) != LMX_MSG_OK) {
            fprintf(stderr, "role history C->P\n");
            lmx_msg_runtime_delete(rtr);
            return 1;
        }
        lmx_msg_set_graph(pm, 0);
        if (lmx_msg_end_turn(rtr, p, 1) != LMX_MSG_OK
            || lmx_owned_ranges_find(pm->ranges, cbuf) == 0
            || lmx_owned_ranges_find(pm->ranges, gbuf) == 0
            || ccells[0] != 9 || gcells[0] != 7) {
            fprintf(stderr, "role history dropped C or G\n");
            lmx_msg_runtime_delete(rtr);
            return 1;
        }
        if (lmx_owned_ranges_find(pm->ranges, temp) != 0
            || lmx_owned_ranges_find(pm->ranges, temp_back) != 0) {
            fprintf(stderr, "role history copied RETAIN temp\n");
            lmx_msg_runtime_delete(rtr);
            return 1;
        }
        fprintf(stderr, "root roles: HISTORY G/C travel; RETAIN temp dies; release one role keeps the other\n");
        lmx_msg_runtime_delete(rtr);
    }
    {
        LmxMsgRuntime *rts;
        LmxMsgAddr dummy = 0;
        LmxMsgAddr a = 0;
        LmxMsgAddr b = 0;
        uchar ini = 18;
        LmxMsg *ma;
        LmxMsg *mb;
        TurnCtx tctx;
        memset(&tctx, 0, sizeof(tctx));
        rts = lmx_msg_runtime_new();
        if (rts == 0 || lmx_msg_create(rts, 0, 1, &ini, 1, &dummy) != LMX_MSG_OK
            || lmx_msg_end_turn(rts, dummy, 1) != LMX_MSG_OK
            || lmx_msg_create(rts, dummy, 2, &ini, 1, &a) != LMX_MSG_OK
            || lmx_msg_create(rts, dummy, 3, &ini, 1, &b) != LMX_MSG_OK
            || lmx_msg_exec_bind(rts, a, turn_just_end, &tctx, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK
            || lmx_msg_exec_bind(rts, b, turn_just_end, &tctx, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK) {
            fprintf(stderr, "exec_start claim create\n");
            if (rts != 0) {
                lmx_msg_runtime_delete(rts);
            }
            return 1;
        }
        ma = lmx_msg_find(rts, a);
        mb = lmx_msg_find(rts, b);
        if (ma == 0 || mb == 0 || ma->mapped != 0 || mb->mapped != 0) {
            fprintf(stderr, "exec_start claim pre-map\n");
            lmx_msg_runtime_delete(rts);
            return 1;
        }
        if (lmx_msg_exec_start_contexts(rts) != LMX_MSG_OK || ma->mapped == 0 || mb->mapped == 0) {
            fprintf(stderr, "exec_start claim missed bound children\n");
            if (rts != 0) {
                lmx_msg_exec_stop(rts);
                lmx_msg_runtime_delete(rts);
            }
            return 1;
        }
        lmx_msg_exec_stop(rts);
        fprintf(stderr, "exec_start: snapshot-claims both bound unmapped children\n");
        lmx_msg_runtime_delete(rts);
    }
    {
        LmxMsgRuntime *rtk;
        LmxMsgAddr dummy = 0;
        LmxMsgAddr a = 0;
        uchar ini = 19;
        LmxMsg *ma;
        TurnCtx tctx;
        memset(&tctx, 0, sizeof(tctx));
        rtk = lmx_msg_runtime_new();
        if (rtk == 0 || lmx_msg_create(rtk, 0, 1, &ini, 1, &dummy) != LMX_MSG_OK
            || lmx_msg_end_turn(rtk, dummy, 1) != LMX_MSG_OK
            || lmx_msg_create(rtk, dummy, 2, &ini, 1, &a) != LMX_MSG_OK
            || lmx_msg_exec_bind(rtk, a, turn_just_end, &tctx, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK) {
            fprintf(stderr, "exec_start kicks-fail create\n");
            if (rtk != 0) {
                lmx_msg_runtime_delete(rtk);
            }
            return 1;
        }
        ma = lmx_msg_find(rtk, a);
        if (ma == 0) {
            fprintf(stderr, "exec_start kicks-fail find\n");
            lmx_msg_runtime_delete(rtk);
            return 1;
        }
        lmx_msg_exec_test_set_fail_start_kicks(rtk, 1);
        if (lmx_msg_exec_start_contexts(rtk) != LMX_MSG_NOMEM || ma->mapped != 0) {
            fprintf(stderr, "exec_start kicks-fail did not stop clean\n");
            lmx_msg_exec_stop(rtk);
            lmx_msg_runtime_delete(rtk);
            return 1;
        }
        if (lmx_msg_exec_start_contexts(rtk) != LMX_MSG_OK || ma->mapped == 0) {
            fprintf(stderr, "exec_start kicks-fail retry\n");
            lmx_msg_exec_stop(rtk);
            lmx_msg_runtime_delete(rtk);
            return 1;
        }
        lmx_msg_exec_stop(rtk);
        fprintf(stderr, "exec_start: kick-calloc fail stops pool; retry claims child\n");
        lmx_msg_runtime_delete(rtk);
    }
    {
        LmxMsgRuntime *rtr;
        LmxMsgAddr dummy = 0;
        uchar ini = 20;
        LmxMsg *ma;
        TurnCtx tctx;
        memset(&tctx, 0, sizeof(tctx));
        rtr = lmx_msg_runtime_new();
        if (rtr == 0 || lmx_msg_create(rtr, 0, 1, &ini, 1, &dummy) != LMX_MSG_OK
            || lmx_msg_exec_bind(rtr, dummy, turn_just_end, &tctx, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK) {
            fprintf(stderr, "exec_stop ready create\n");
            if (rtr != 0) {
                lmx_msg_runtime_delete(rtr);
            }
            return 1;
        }
        lmx_msg_exec_ready(rtr, dummy);
        if (lmx_msg_exec_map_queued(rtr, dummy) == 0) {
            fprintf(stderr, "exec_stop map empty before stop map=%d\n",
                lmx_msg_exec_map_queued(rtr, dummy));
            if (rtr != 0) {
                lmx_msg_runtime_delete(rtr);
            }
            return 1;
        }
        ma = lmx_msg_find(rtr, dummy);
        if (lmx_msg_exec_stop(rtr) != LMX_MSG_OK
            || lmx_msg_exec_map_queued(rtr, dummy) != 0
            || lmx_msg_exec_map_queued(rtr, dummy) != 0
            || (ma != 0 && (ma->mapped != 0 || ma->ready != 0))) {
            fprintf(stderr, "exec_stop left ready/map\n");
            lmx_msg_runtime_delete(rtr);
            return 1;
        }
        if (lmx_msg_exec_start_contexts(rtr) != LMX_MSG_OK || ma == 0 || ma->mapped == 0) {
            fprintf(stderr, "exec_stop retry start\n");
            lmx_msg_exec_stop(rtr);
            lmx_msg_runtime_delete(rtr);
            return 1;
        }
        lmx_msg_exec_stop(rtr);
        fprintf(stderr, "exec_stop: set ready flag cleared; retry start remaps child\n");
        lmx_msg_runtime_delete(rtr);
    }
    {
        LmxMsgRuntime *rti;
        LmxMsgAddr dummy = 0;
        LmxMsgAddr ui = 0;
        LmxMsgAddr any = 0;
        uchar ini = 21;
        LmxMsgEnv env;
        TurnCtx ui_ctx;
        TurnCtx any_ctx;
        DWORD tstop;
        DWORD dl;
        memset(&ui_ctx, 0, sizeof(ui_ctx));
        memset(&any_ctx, 0, sizeof(any_ctx));
        memset(&env, 0, sizeof(env));
        env.kind = LMX_MSG_KIND_BYTES;
        env.n = 1;
        env.bytes = &ini;
        rti = lmx_msg_runtime_new();
        if (rti == 0 || lmx_msg_create(rti, 0, 1, &ini, 1, &dummy) != LMX_MSG_OK
            || lmx_msg_create(rti, dummy, 2, &ini, 1, &ui) != LMX_MSG_OK
            || lmx_msg_create(rti, dummy, 3, &ini, 1, &any) != LMX_MSG_OK
            || lmx_msg_exec_bind(rti, ui, turn_just_end, &ui_ctx, LMX_MSG_AFFINITY_UI) != LMX_MSG_OK
            || lmx_msg_exec_bind(rti, any, turn_just_end, &any_ctx, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK
            || lmx_msg_send(rti, dummy, ui, &env) != LMX_MSG_STAGED
            || lmx_msg_send(rti, dummy, any, &env) != LMX_MSG_STAGED
            || lmx_msg_end_turn(rti, dummy, 1) != LMX_MSG_OK) {
            fprintf(stderr, "exec wait ineligible create\n");
            if (rti != 0) {
                lmx_msg_runtime_delete(rti);
            }
            return 1;
        }
        lmx_msg_pump(rti);
        if (lmx_msg_exec_start_contexts(rti) != LMX_MSG_OK) {
            fprintf(stderr, "exec wait ineligible start\n");
            lmx_msg_runtime_delete(rti);
            return 1;
        }
        dl = GetTickCount() + 2000;
        while (InterlockedCompareExchange(&any_ctx.done, 0, 0) == 0 && GetTickCount() < dl) {
            Sleep(10);
        }
        Sleep(30);
        if (InterlockedCompareExchange(&any_ctx.done, 0, 0) < 1
            || InterlockedCompareExchange(&ui_ctx.done, 0, 0) != 0
            || lmx_msg_exec_map_queued(rti, ui) != 0) {
            fprintf(stderr, "exec wait ineligible any=%ld ui=%ld ui_on_worker=%d\n",
                (long)InterlockedCompareExchange(&any_ctx.done, 0, 0),
                (long)InterlockedCompareExchange(&ui_ctx.done, 0, 0),
                lmx_msg_exec_map_queued(rti, ui));
            lmx_msg_exec_stop(rti);
            lmx_msg_runtime_delete(rti);
            return 1;
        }
        if (lmx_msg_exec_ui_step(rti) != LMX_MSG_OK
            || InterlockedCompareExchange(&ui_ctx.done, 0, 0) < 1) {
            fprintf(stderr, "exec wait ineligible ui_step\n");
            lmx_msg_exec_stop(rti);
            lmx_msg_runtime_delete(rti);
            return 1;
        }
        lmx_msg_exec_stop(rti);
        fprintf(stderr, "exec wait: UI off worker ring; ANY ran; ui_step took UI; 2 workers\n");
        lmx_msg_runtime_delete(rti);
        rti = lmx_msg_runtime_new();
        dummy = 0;
        ui = 0;
        any = 0;
        memset(&ui_ctx, 0, sizeof(ui_ctx));
        memset(&any_ctx, 0, sizeof(any_ctx));
        if (rti == 0 || lmx_msg_create(rti, 0, 1, &ini, 1, &dummy) != LMX_MSG_OK
            || lmx_msg_create(rti, dummy, 2, &ini, 1, &ui) != LMX_MSG_OK
            || lmx_msg_create(rti, dummy, 3, &ini, 1, &any) != LMX_MSG_OK
            || lmx_msg_exec_bind(rti, ui, turn_just_end, &ui_ctx, LMX_MSG_AFFINITY_UI) != LMX_MSG_OK
            || lmx_msg_exec_bind(rti, any, turn_just_end, &any_ctx, LMX_MSG_AFFINITY_UI) != LMX_MSG_OK
            || lmx_msg_send(rti, dummy, ui, &env) != LMX_MSG_STAGED
            || lmx_msg_send(rti, dummy, ui, &env) != LMX_MSG_STAGED
            || lmx_msg_send(rti, dummy, ui, &env) != LMX_MSG_STAGED
            || lmx_msg_send(rti, dummy, any, &env) != LMX_MSG_STAGED
            || lmx_msg_end_turn(rti, dummy, 1) != LMX_MSG_OK) {
            fprintf(stderr, "exec ui fifo create\n");
            if (rti != 0) {
                lmx_msg_runtime_delete(rti);
            }
            return 1;
        }
        lmx_msg_pump(rti);
        if (any_queued2(rti, ui, any) != 0 || lmx_msg_exec_ui_nrequests(rti) < 2) {
            fprintf(stderr, "exec ui fifo ring nready=%d nui=%d\n",
                any_queued2(rti, ui, any), lmx_msg_exec_ui_nrequests(rti));
            lmx_msg_runtime_delete(rti);
            return 1;
        }
        if (lmx_msg_exec_ui_step(rti) != LMX_MSG_OK
            || InterlockedCompareExchange(&ui_ctx.done, 0, 0) != 1
            || InterlockedCompareExchange(&any_ctx.done, 0, 0) != 0
            || lmx_msg_exec_ui_nrequests(rti) < 1) {
            fprintf(stderr, "exec ui fifo first-turn first=%ld second=%ld nui=%d\n",
                (long)InterlockedCompareExchange(&ui_ctx.done, 0, 0),
                (long)InterlockedCompareExchange(&any_ctx.done, 0, 0),
                lmx_msg_exec_ui_nrequests(rti));
            lmx_msg_runtime_delete(rti);
            return 1;
        }
        if (lmx_msg_exec_ui_step(rti) != LMX_MSG_OK
            || InterlockedCompareExchange(&any_ctx.done, 0, 0) != 1
            || any_queued2(rti, ui, any) != 0) {
            fprintf(stderr, "exec ui fifo second-turn first=%ld second=%ld nready=%d\n",
                (long)InterlockedCompareExchange(&ui_ctx.done, 0, 0),
                (long)InterlockedCompareExchange(&any_ctx.done, 0, 0),
                any_queued2(rti, ui, any));
            lmx_msg_runtime_delete(rti);
            return 1;
        }
        fprintf(stderr, "exec wait: two UI FIFO order; first still runnable when second runs\n");
        lmx_msg_runtime_delete(rti);
        {
            /* Stage 3d: two parents map UI children, and the UI lane serves them
             * in admission order: cb's input is admitted before ca's, so cb runs
             * first although ca comes first in the family tree. */
            LmxMsgRuntime *rtw;
            LmxMsgAddr pa = 0, pb = 0, ca = 0, cb = 0;
            TurnCtx cta;
            TurnCtx ctb;
            int wst1;
            int wst2;
            long a1;
            long b1;
            memset(&cta, 0, sizeof(cta));
            memset(&ctb, 0, sizeof(ctb));
            rtw = lmx_msg_runtime_new();
            if (rtw == 0 || lmx_msg_create(rtw, 0, 1, &ini, 1, &pa) != LMX_MSG_OK
                || lmx_msg_end_turn(rtw, pa, 1) != LMX_MSG_OK
                || lmx_msg_create(rtw, pa, 2, &ini, 1, &ca) != LMX_MSG_OK
                || lmx_msg_end_turn(rtw, pa, 1) != LMX_MSG_OK
                || lmx_msg_create(rtw, 0, 3, &ini, 1, &pb) != LMX_MSG_OK
                || lmx_msg_end_turn(rtw, pb, 1) != LMX_MSG_OK
                || lmx_msg_create(rtw, pb, 4, &ini, 1, &cb) != LMX_MSG_OK
                || lmx_msg_end_turn(rtw, pb, 1) != LMX_MSG_OK
                || lmx_msg_exec_bind(rtw, ca, turn_recv_end, &cta, LMX_MSG_AFFINITY_UI) != LMX_MSG_OK
                || lmx_msg_exec_bind(rtw, cb, turn_recv_end, &ctb, LMX_MSG_AFFINITY_UI) != LMX_MSG_OK
                || lmx_msg_host_post(rtw, cb, &env) != LMX_MSG_STAGED
                || lmx_msg_host_post(rtw, ca, &env) != LMX_MSG_STAGED
                || lmx_msg_host_drain(rtw) != LMX_MSG_OK) {
                fprintf(stderr, "exec ui two-parent create\n");
                if (rtw != 0) {
                    lmx_msg_runtime_delete(rtw);
                }
                return 1;
            }
            wst1 = lmx_msg_exec_ui_step(rtw);
            a1 = InterlockedCompareExchange(&cta.done, 0, 0);
            b1 = InterlockedCompareExchange(&ctb.done, 0, 0);
            wst2 = lmx_msg_exec_ui_step(rtw);
            if (wst1 != LMX_MSG_OK || wst2 != LMX_MSG_OK || a1 != 0 || b1 != 1
                || InterlockedCompareExchange(&cta.done, 0, 0) != 1
                || InterlockedCompareExchange(&ctb.done, 0, 0) != 1
                || lmx_msg_exec_ui_step(rtw) != LMX_MSG_EMPTY) {
                fprintf(stderr, "exec ui two-parent order st=%d/%d first a=%ld b=%ld\n", wst1, wst2, a1, b1);
                lmx_msg_runtime_delete(rtw);
                return 1;
            }
            lmx_msg_runtime_delete(rtw);
            fprintf(stderr, "exec wait: the UI lane serves two parents' UI children in admission order\n");
        }
        rti = lmx_msg_runtime_new();
        dummy = 0;
        ui = 0;
        memset(&ui_ctx, 0, sizeof(ui_ctx));
        if (rti == 0 || lmx_msg_create(rti, 0, 1, &ini, 1, &dummy) != LMX_MSG_OK
            || lmx_msg_create(rti, dummy, 2, &ini, 1, &ui) != LMX_MSG_OK
            || lmx_msg_exec_bind(rti, ui, turn_just_end, &ui_ctx, LMX_MSG_AFFINITY_UI) != LMX_MSG_OK
            || lmx_msg_send(rti, dummy, ui, &env) != LMX_MSG_STAGED
            || lmx_msg_end_turn(rti, dummy, 1) != LMX_MSG_OK) {
            fprintf(stderr, "exec ui restart create\n");
            if (rti != 0) {
                lmx_msg_runtime_delete(rti);
            }
            return 1;
        }
        lmx_msg_pump(rti);
        if (lmx_msg_exec_start_contexts(rti) != LMX_MSG_OK) {
            fprintf(stderr, "exec ui restart start\n");
            lmx_msg_runtime_delete(rti);
            return 1;
        }
        if (lmx_msg_exec_stop(rti) != LMX_MSG_OK
            || lmx_msg_exec_start_contexts(rti) != LMX_MSG_OK
            || lmx_msg_exec_ui_step(rti) != LMX_MSG_OK
            || InterlockedCompareExchange(&ui_ctx.done, 0, 0) < 1
            || lmx_msg_exec_map_queued(rti, ui) != 0) {
            fprintf(stderr, "exec ui restart lost work done=%ld nready=%d\n",
                (long)InterlockedCompareExchange(&ui_ctx.done, 0, 0),
                lmx_msg_exec_map_queued(rti, ui));
            lmx_msg_exec_stop(rti);
            lmx_msg_runtime_delete(rti);
            return 1;
        }
        lmx_msg_exec_stop(rti);
        fprintf(stderr, "exec wait: UI pending survives stop/restart without new send\n");
        lmx_msg_runtime_delete(rti);
        rti = lmx_msg_runtime_new();
        dummy = 0;
        ui = 0;
        memset(&ui_ctx, 0, sizeof(ui_ctx));
        if (rti == 0 || lmx_msg_create(rti, 0, 1, &ini, 1, &dummy) != LMX_MSG_OK
            || lmx_msg_create(rti, dummy, 2, &ini, 1, &ui) != LMX_MSG_OK
            || lmx_msg_exec_bind(rti, ui, turn_just_end, &ui_ctx, LMX_MSG_AFFINITY_UI) != LMX_MSG_OK
            || lmx_msg_send(rti, dummy, ui, &env) != LMX_MSG_STAGED
            || lmx_msg_end_turn(rti, dummy, 1) != LMX_MSG_OK) {
            fprintf(stderr, "exec ui rebind create\n");
            if (rti != 0) {
                lmx_msg_runtime_delete(rti);
            }
            return 1;
        }
        lmx_msg_pump(rti);
        if (lmx_msg_exec_start_contexts(rti) != LMX_MSG_OK) {
            fprintf(stderr, "exec ui rebind contexts\n");
            lmx_msg_runtime_delete(rti);
            return 1;
        }
        lmx_msg_exec_test_set_fail_ctx(rti, 1);
        if (lmx_msg_exec_bind(rti, ui, turn_just_end, &ui_ctx, LMX_MSG_AFFINITY_ANY) != LMX_MSG_NOMEM
            || lmx_msg_exec_bind_aff(rti, ui) != LMX_MSG_AFFINITY_UI
            || lmx_msg_exec_map_queued(rti, ui) != 0
            || lmx_msg_exec_ui_nrequests(rti) < 1) {
            fprintf(stderr, "exec ui rebind rollback nready=%d nui=%d aff=%d\n",
                lmx_msg_exec_map_queued(rti, ui), lmx_msg_exec_ui_nrequests(rti),
                lmx_msg_exec_bind_aff(rti, ui));
            lmx_msg_exec_test_set_fail_ctx(rti, 0);
            lmx_msg_exec_stop(rti);
            lmx_msg_runtime_delete(rti);
            return 1;
        }
        lmx_msg_exec_test_set_fail_ctx(rti, 0);
        if (lmx_msg_exec_ui_step(rti) != LMX_MSG_OK
            || InterlockedCompareExchange(&ui_ctx.done, 0, 0) < 1
            || lmx_msg_exec_map_queued(rti, ui) != 0) {
            fprintf(stderr, "exec ui rebind recover done=%ld nready=%d\n",
                (long)InterlockedCompareExchange(&ui_ctx.done, 0, 0),
                lmx_msg_exec_map_queued(rti, ui));
            lmx_msg_exec_stop(rti);
            lmx_msg_runtime_delete(rti);
            return 1;
        }
        lmx_msg_exec_stop(rti);
        fprintf(stderr, "exec wait: UI->ANY launch fail keeps the UI ready flag; no worker residue\n");
        lmx_msg_runtime_delete(rti);
        rti = lmx_msg_runtime_new();
        dummy = 0;
        ui = 0;
        any = 0;
        memset(&ui_ctx, 0, sizeof(ui_ctx));
        memset(&any_ctx, 0, sizeof(any_ctx));
        if (rti == 0 || lmx_msg_create(rti, 0, 1, &ini, 1, &dummy) != LMX_MSG_OK
            || lmx_msg_create(rti, dummy, 2, &ini, 1, &ui) != LMX_MSG_OK
            || lmx_msg_create(rti, dummy, 3, &ini, 1, &any) != LMX_MSG_OK
            || lmx_msg_exec_bind(rti, ui, turn_just_end, &ui_ctx, LMX_MSG_AFFINITY_UI) != LMX_MSG_OK
            || lmx_msg_exec_bind(rti, any, turn_just_end, &any_ctx, LMX_MSG_AFFINITY_UI) != LMX_MSG_OK
            || lmx_msg_send(rti, dummy, ui, &env) != LMX_MSG_STAGED
            || lmx_msg_send(rti, dummy, any, &env) != LMX_MSG_STAGED
            || lmx_msg_end_turn(rti, dummy, 1) != LMX_MSG_OK) {
            fprintf(stderr, "exec ui head-fail create\n");
            if (rti != 0) {
                lmx_msg_runtime_delete(rti);
            }
            return 1;
        }
        lmx_msg_pump(rti);
        if (lmx_msg_exec_ui_nrequests(rti) < 2) {
            fprintf(stderr, "exec ui head-fail nui=%d\n", lmx_msg_exec_ui_nrequests(rti));
            lmx_msg_runtime_delete(rti);
            return 1;
        }
        if (lmx_msg_exec_start_contexts(rti) != LMX_MSG_OK) {
            fprintf(stderr, "exec ui head-fail contexts\n");
            lmx_msg_runtime_delete(rti);
            return 1;
        }
        lmx_msg_exec_test_set_fail_ctx(rti, 1);
        if (lmx_msg_exec_bind(rti, ui, turn_just_end, &ui_ctx, LMX_MSG_AFFINITY_ANY) != LMX_MSG_NOMEM
            || lmx_msg_exec_bind_aff(rti, ui) != LMX_MSG_AFFINITY_UI
            || any_queued2(rti, ui, any) != 0
            || lmx_msg_exec_ui_nrequests(rti) < 2) {
            fprintf(stderr, "exec ui head-fail rollback nready=%d nui=%d aff=%d\n",
                any_queued2(rti, ui, any), lmx_msg_exec_ui_nrequests(rti),
                lmx_msg_exec_bind_aff(rti, ui));
            lmx_msg_exec_test_set_fail_ctx(rti, 0);
            lmx_msg_exec_stop(rti);
            lmx_msg_runtime_delete(rti);
            return 1;
        }
        lmx_msg_exec_test_set_fail_ctx(rti, 0);
        if (lmx_msg_exec_ui_step(rti) != LMX_MSG_OK
            || InterlockedCompareExchange(&ui_ctx.done, 0, 0) != 1
            || InterlockedCompareExchange(&any_ctx.done, 0, 0) != 0
            || lmx_msg_exec_ui_nrequests(rti) < 1) {
            fprintf(stderr, "exec ui head-fail first-turn first=%ld second=%ld nui=%d\n",
                (long)InterlockedCompareExchange(&ui_ctx.done, 0, 0),
                (long)InterlockedCompareExchange(&any_ctx.done, 0, 0),
                lmx_msg_exec_ui_nrequests(rti));
            lmx_msg_exec_stop(rti);
            lmx_msg_runtime_delete(rti);
            return 1;
        }
        if (lmx_msg_exec_ui_step(rti) != LMX_MSG_OK
            || InterlockedCompareExchange(&any_ctx.done, 0, 0) != 1
            || InterlockedCompareExchange(&ui_ctx.done, 0, 0) != 1
            || any_queued2(rti, ui, any) != 0) {
            fprintf(stderr, "exec ui head-fail second-turn first=%ld second=%ld nready=%d\n",
                (long)InterlockedCompareExchange(&ui_ctx.done, 0, 0),
                (long)InterlockedCompareExchange(&any_ctx.done, 0, 0),
                any_queued2(rti, ui, any));
            lmx_msg_exec_stop(rti);
            lmx_msg_runtime_delete(rti);
            return 1;
        }
        lmx_msg_exec_stop(rti);
        fprintf(stderr, "exec wait: failed UI->ANY of head keeps FIFO; exact-once; no new send\n");
        lmx_msg_runtime_delete(rti);
        rti = lmx_msg_runtime_new();
        dummy = 0;
        ui = 0;
        any = 0;
        memset(&ui_ctx, 0, sizeof(ui_ctx));
        memset(&any_ctx, 0, sizeof(any_ctx));
        if (rti == 0 || lmx_msg_create(rti, 0, 1, &ini, 1, &dummy) != LMX_MSG_OK
            || lmx_msg_create(rti, dummy, 2, &ini, 1, &ui) != LMX_MSG_OK
            || lmx_msg_create(rti, dummy, 3, &ini, 1, &any) != LMX_MSG_OK
            || lmx_msg_exec_bind(rti, ui, turn_recv_end, &ui_ctx, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK
            || lmx_msg_exec_bind(rti, any, turn_recv_end, &any_ctx, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK
            || lmx_msg_send(rti, dummy, ui, &env) != LMX_MSG_STAGED
            || lmx_msg_send(rti, dummy, any, &env) != LMX_MSG_STAGED
            || lmx_msg_end_turn(rti, dummy, 1) != LMX_MSG_OK) {
            fprintf(stderr, "exec map-ready create\n");
            if (rti != 0) {
                lmx_msg_runtime_delete(rti);
            }
            return 1;
        }
        lmx_msg_pump(rti);
        if (lmx_msg_exec_start_contexts(rti) != LMX_MSG_OK) {
            fprintf(stderr, "exec map-ready start\n");
            lmx_msg_runtime_delete(rti);
            return 1;
        }
        dl = GetTickCount() + 2000;
        while ((InterlockedCompareExchange(&ui_ctx.done, 0, 0) == 0
            || InterlockedCompareExchange(&any_ctx.done, 0, 0) == 0)
            && GetTickCount() < dl) {
            Sleep(10);
        }
        if (InterlockedCompareExchange(&ui_ctx.done, 0, 0) != 1
            || InterlockedCompareExchange(&any_ctx.done, 0, 0) != 1
            || any_queued2(rti, ui, any) != 0
            || any_queued2(rti, ui, any) != 0) {
            fprintf(stderr, "exec map-ready done first=%ld second=%ld nready=%d map=%d\n",
                (long)InterlockedCompareExchange(&ui_ctx.done, 0, 0),
                (long)InterlockedCompareExchange(&any_ctx.done, 0, 0),
                any_queued2(rti, ui, any), any_queued2(rti, ui, any));
            lmx_msg_exec_stop(rti);
            lmx_msg_runtime_delete(rti);
            return 1;
        }
        lmx_msg_exec_stop(rti);
        fprintf(stderr, "exec wait: mapped ANY ready by its own flag; exact-once\n");
        lmx_msg_runtime_delete(rti);
        rti = lmx_msg_runtime_new();
        dummy = 0;
        ui = 0;
        any = 0;
        memset(&ui_ctx, 0, sizeof(ui_ctx));
        memset(&any_ctx, 0, sizeof(any_ctx));
        if (rti == 0 || lmx_msg_create(rti, 0, 1, &ini, 1, &dummy) != LMX_MSG_OK
            || lmx_msg_create(rti, dummy, 2, &ini, 1, &ui) != LMX_MSG_OK
            || lmx_msg_create(rti, 0, 3, &ini, 1, &any) != LMX_MSG_OK
            || lmx_msg_exec_bind(rti, ui, turn_recv_end, &ui_ctx, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK
            || lmx_msg_exec_bind(rti, any, turn_recv_end, &any_ctx, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK
            || lmx_msg_send(rti, dummy, ui, &env) != LMX_MSG_STAGED
            || lmx_msg_end_turn(rti, dummy, 1) != LMX_MSG_OK
            || lmx_msg_host_post(rti, any, &env) != LMX_MSG_STAGED) {
            fprintf(stderr, "exec map-isol create\n");
            if (rti != 0) {
                lmx_msg_runtime_delete(rti);
            }
            return 1;
        }
        lmx_msg_pump(rti);
        (void)lmx_msg_host_drain(rti);
        if (lmx_msg_exec_start_contexts(rti) != LMX_MSG_OK) {
            fprintf(stderr, "exec map-isol start\n");
            lmx_msg_runtime_delete(rti);
            return 1;
        }
        dl = GetTickCount() + 2000;
        while ((InterlockedCompareExchange(&ui_ctx.done, 0, 0) == 0
            || InterlockedCompareExchange(&any_ctx.done, 0, 0) == 0)
            && GetTickCount() < dl) {
            Sleep(10);
        }
        if (InterlockedCompareExchange(&ui_ctx.done, 0, 0) != 1
            || InterlockedCompareExchange(&any_ctx.done, 0, 0) != 1
            || any_queued2(rti, ui, any) != 0) {
            fprintf(stderr, "exec map-isol done a=%ld b=%ld nready=%d\n",
                (long)InterlockedCompareExchange(&ui_ctx.done, 0, 0),
                (long)InterlockedCompareExchange(&any_ctx.done, 0, 0),
                any_queued2(rti, ui, any));
            lmx_msg_exec_stop(rti);
            lmx_msg_runtime_delete(rti);
            return 1;
        }
        lmx_msg_exec_stop(rti);
        fprintf(stderr, "exec wait: two Message contexts isolated, each ready by its own flag\n");
        lmx_msg_runtime_delete(rti);
        rti = lmx_msg_runtime_new();
        {
            LmxMsgAddr pa = 0;
            LmxMsgAddr pb = 0;
            LmxMsgAddr ca = 0;
            LmxMsgAddr cb = 0;
            memset(&ui_ctx, 0, sizeof(ui_ctx));
            memset(&any_ctx, 0, sizeof(any_ctx));
            if (rti == 0 || lmx_msg_create(rti, 0, 1, &ini, 1, &pa) != LMX_MSG_OK
                || lmx_msg_end_turn(rti, pa, 1) != LMX_MSG_OK
                || lmx_msg_create(rti, pa, 2, &ini, 1, &ca) != LMX_MSG_OK
                || lmx_msg_end_turn(rti, pa, 1) != LMX_MSG_OK
                || lmx_msg_create(rti, 0, 3, &ini, 1, &pb) != LMX_MSG_OK
                || lmx_msg_end_turn(rti, pb, 1) != LMX_MSG_OK
                || lmx_msg_create(rti, pb, 4, &ini, 1, &cb) != LMX_MSG_OK
                || lmx_msg_end_turn(rti, pb, 1) != LMX_MSG_OK
                || lmx_msg_exec_bind(rti, ca, turn_hold_until_peer, &ui_ctx, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK
                || lmx_msg_exec_bind(rti, cb, turn_recv_end, &any_ctx, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK
                || lmx_msg_send(rti, pa, ca, &env) != LMX_MSG_STAGED
                || lmx_msg_send(rti, pb, cb, &env) != LMX_MSG_STAGED
                || lmx_msg_end_turn(rti, pa, 1) != LMX_MSG_OK
                || lmx_msg_end_turn(rti, pb, 1) != LMX_MSG_OK) {
                fprintf(stderr, "exec map-fair create\n");
                if (rti != 0) {
                    lmx_msg_runtime_delete(rti);
                }
                return 1;
            }
            lmx_msg_pump(rti);
            g_fair_peer_done = &any_ctx.done;
            if (lmx_msg_exec_start_contexts(rti) != LMX_MSG_OK) {
                g_fair_peer_done = 0;
                fprintf(stderr, "exec map-fair start\n");
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            dl = GetTickCount() + 2000;
            while ((InterlockedCompareExchange(&any_ctx.done, 0, 0) == 0
                || InterlockedCompareExchange(&ui_ctx.done, 0, 0) == 0)
                && GetTickCount() < dl) {
                Sleep(10);
            }
            g_fair_peer_done = 0;
            if (InterlockedCompareExchange(&any_ctx.done, 0, 0) != 1
                || InterlockedCompareExchange(&ui_ctx.done, 0, 0) < 1
                || lmx_msg_exec_map_queued(rti, cb) != 0
                || lmx_msg_exec_retire_n(rti) != 0
                || lmx_msg_exec_workers(rti) < 1) {
                fprintf(stderr, "exec map-fair starve B doneA=%ld doneB=%ld qA=%d qB=%d pend=%d\n",
                    (long)InterlockedCompareExchange(&ui_ctx.done, 0, 0),
                    (long)InterlockedCompareExchange(&any_ctx.done, 0, 0),
                    lmx_msg_exec_map_queued(rti, ca),
                    lmx_msg_exec_map_queued(rti, cb),
                    lmx_msg_exec_retire_n(rti));
                lmx_msg_exec_stop(rti);
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            lmx_msg_exec_stop(rti);
            fprintf(stderr, "exec wait: owner B progresses while owner A stays runnable; 1 worker\n");
            lmx_msg_runtime_delete(rti);
        }
        rti = lmx_msg_runtime_new();
        {
            enum { NIDLE = 40 };
            LmxMsgAddr p = 0, pa = 0, pb = 0, ca = 0, cb = 0;
            LmxMsgAddr idle[NIDLE];
            TurnCtx idle_ctx[NIDLE];
            int i;
            int steps;
            memset(&ui_ctx, 0, sizeof(ui_ctx));
            memset(&any_ctx, 0, sizeof(any_ctx));
            memset(idle, 0, sizeof(idle));
            memset(idle_ctx, 0, sizeof(idle_ctx));
            if (rti == 0 || lmx_msg_create(rti, 0, 1, &ini, 1, &p) != LMX_MSG_OK
                || lmx_msg_end_turn(rti, p, 1) != LMX_MSG_OK) {
                fprintf(stderr, "exec owner-scale ui p\n");
                if (rti != 0) {
                    lmx_msg_runtime_delete(rti);
                }
                return 1;
            }
            for (i = 0; i < NIDLE; i++) {
                if (lmx_msg_create(rti, p, (unsigned)(10 + i), &ini, 1, &idle[i]) != LMX_MSG_OK
                    || lmx_msg_end_turn(rti, p, 1) != LMX_MSG_OK
                    || lmx_msg_exec_bind(rti, idle[i], turn_just_end, &idle_ctx[i], LMX_MSG_AFFINITY_UI) != LMX_MSG_OK) {
                    fprintf(stderr, "exec owner-scale ui idle i=%d\n", i);
                    lmx_msg_runtime_delete(rti);
                    return 1;
                }
            }
            if (lmx_msg_create(rti, 0, 2, &ini, 1, &pa) != LMX_MSG_OK
                || lmx_msg_end_turn(rti, pa, 1) != LMX_MSG_OK
                || lmx_msg_create(rti, pa, 3, &ini, 1, &ca) != LMX_MSG_OK
                || lmx_msg_end_turn(rti, pa, 1) != LMX_MSG_OK
                || lmx_msg_create(rti, 0, 4, &ini, 1, &pb) != LMX_MSG_OK
                || lmx_msg_end_turn(rti, pb, 1) != LMX_MSG_OK
                || lmx_msg_create(rti, pb, 5, &ini, 1, &cb) != LMX_MSG_OK
                || lmx_msg_end_turn(rti, pb, 1) != LMX_MSG_OK
                || lmx_msg_exec_bind(rti, ca, turn_recv_end, &ui_ctx, LMX_MSG_AFFINITY_UI) != LMX_MSG_OK
                || lmx_msg_exec_bind(rti, cb, turn_recv_end, &any_ctx, LMX_MSG_AFFINITY_UI) != LMX_MSG_OK
                || lmx_msg_send(rti, pa, ca, &env) != LMX_MSG_STAGED
                || lmx_msg_send(rti, pb, cb, &env) != LMX_MSG_STAGED
                || lmx_msg_end_turn(rti, pa, 1) != LMX_MSG_OK
                || lmx_msg_end_turn(rti, pb, 1) != LMX_MSG_OK) {
                fprintf(stderr, "exec owner-scale ui ready\n");
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            lmx_msg_pump(rti);
            steps = 0;
            dl = GetTickCount() + 2000;
            while ((InterlockedCompareExchange(&ui_ctx.done, 0, 0) == 0
                || InterlockedCompareExchange(&any_ctx.done, 0, 0) == 0)
                && GetTickCount() < dl && steps < 64) {
                (void)lmx_msg_exec_ui_step(rti);
                steps += 1;
            }
            if (InterlockedCompareExchange(&ui_ctx.done, 0, 0) != 1
                || InterlockedCompareExchange(&any_ctx.done, 0, 0) != 1
                || steps >= 64) {
                fprintf(stderr, "exec owner-scale UI doneA=%ld doneB=%ld steps=%d\n",
                    (long)InterlockedCompareExchange(&ui_ctx.done, 0, 0),
                    (long)InterlockedCompareExchange(&any_ctx.done, 0, 0), steps);
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            fprintf(stderr, "exec wait: ready UI Messages run past %d idle UI binds\n", NIDLE);
            lmx_msg_runtime_delete(rti);
        }
        rti = lmx_msg_runtime_new();
        {
            LmxMsgAddr p = 0, a = 0, b = 0, d = 0;
            memset(&ui_ctx, 0, sizeof(ui_ctx));
            memset(&any_ctx, 0, sizeof(any_ctx));
            g_mail_entered = CreateEventA(0, 1, 0, 0);
            g_mail_go = CreateEventA(0, 1, 0, 0);
            if (rti == 0 || g_mail_entered == 0 || g_mail_go == 0
                || lmx_msg_create(rti, 0, 1, &ini, 1, &p) != LMX_MSG_OK
                || lmx_msg_end_turn(rti, p, 1) != LMX_MSG_OK
                || lmx_msg_create(rti, p, 2, &ini, 1, &a) != LMX_MSG_OK
                || lmx_msg_create(rti, p, 3, &ini, 1, &b) != LMX_MSG_OK
                || lmx_msg_create(rti, p, 4, &ini, 1, &d) != LMX_MSG_OK
                || lmx_msg_end_turn(rti, p, 1) != LMX_MSG_OK) {
                fprintf(stderr, "exec mail-gate create\n");
                if (g_mail_entered != 0) {
                    CloseHandle(g_mail_entered);
                }
                if (g_mail_go != 0) {
                    CloseHandle(g_mail_go);
                }
                if (rti != 0) {
                    lmx_msg_runtime_delete(rti);
                }
                return 1;
            }
            g_mail_gate_addr = a;
            g_mail_dest = d;
            ResetEvent(g_mail_entered);
            ResetEvent(g_mail_go);
            lmx_msg_test_mail_locked = mail_gate_hook;
            if (lmx_msg_exec_bind(rti, a, turn_send_once, &ui_ctx, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK
                || lmx_msg_exec_bind(rti, b, turn_send_once, &any_ctx, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK
                || lmx_msg_exec_start_contexts(rti) != LMX_MSG_OK) {
                fprintf(stderr, "exec mail-gate start\n");
                lmx_msg_test_mail_locked = 0;
                InterlockedExchange(&g_mail_gate_armed, 0);
                g_mail_gate_addr = 0;
                SetEvent(g_mail_go);
                lmx_msg_exec_stop(rti);
                CloseHandle(g_mail_entered);
                CloseHandle(g_mail_go);
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            Sleep(20);
            InterlockedExchange(&g_mail_gate_armed, 1);
            if (lmx_msg_host_post(rti, a, &env) != LMX_MSG_STAGED
                || lmx_msg_host_drain(rti) != LMX_MSG_OK) {
                fprintf(stderr, "exec mail-gate start\n");
                lmx_msg_test_mail_locked = 0;
                InterlockedExchange(&g_mail_gate_armed, 0);
                g_mail_gate_addr = 0;
                SetEvent(g_mail_go);
                lmx_msg_exec_stop(rti);
                CloseHandle(g_mail_entered);
                CloseHandle(g_mail_go);
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            if (WaitForSingleObject(g_mail_entered, 2000) != WAIT_OBJECT_0) {
                fprintf(stderr, "exec mail-gate enter\n");
                lmx_msg_test_mail_locked = 0;
                InterlockedExchange(&g_mail_gate_armed, 0);
                g_mail_gate_addr = 0;
                SetEvent(g_mail_go);
                lmx_msg_exec_stop(rti);
                CloseHandle(g_mail_entered);
                CloseHandle(g_mail_go);
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            if (lmx_msg_host_post(rti, b, &env) != LMX_MSG_STAGED
                || lmx_msg_host_drain(rti) != LMX_MSG_OK) {
                fprintf(stderr, "exec mail-gate post B\n");
                lmx_msg_test_mail_locked = 0;
                InterlockedExchange(&g_mail_gate_armed, 0);
                g_mail_gate_addr = 0;
                SetEvent(g_mail_go);
                lmx_msg_exec_stop(rti);
                CloseHandle(g_mail_entered);
                CloseHandle(g_mail_go);
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            dl = GetTickCount() + 2000;
            while (InterlockedCompareExchange(&any_ctx.done, 0, 0) == 0 && GetTickCount() < dl) {
                Sleep(10);
            }
            if (InterlockedCompareExchange(&any_ctx.done, 0, 0) != 1
                || InterlockedCompareExchange(&ui_ctx.done, 0, 0) != 0) {
                fprintf(stderr, "exec mail-gate B blocked doneA=%ld doneB=%ld\n",
                    (long)InterlockedCompareExchange(&ui_ctx.done, 0, 0),
                    (long)InterlockedCompareExchange(&any_ctx.done, 0, 0));
                lmx_msg_test_mail_locked = 0;
                InterlockedExchange(&g_mail_gate_armed, 0);
                g_mail_gate_addr = 0;
                SetEvent(g_mail_go);
                lmx_msg_exec_stop(rti);
                CloseHandle(g_mail_entered);
                CloseHandle(g_mail_go);
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            SetEvent(g_mail_go);
            dl = GetTickCount() + 2000;
            while (InterlockedCompareExchange(&ui_ctx.done, 0, 0) == 0 && GetTickCount() < dl) {
                Sleep(10);
            }
            lmx_msg_test_mail_locked = 0;
            InterlockedExchange(&g_mail_gate_armed, 0);
            g_mail_gate_addr = 0;
            g_mail_dest = 0;
            if (InterlockedCompareExchange(&ui_ctx.done, 0, 0) != 1) {
                fprintf(stderr, "exec mail-gate A stuck\n");
                lmx_msg_exec_stop(rti);
                CloseHandle(g_mail_entered);
                CloseHandle(g_mail_go);
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            lmx_msg_exec_stop(rti);
            CloseHandle(g_mail_entered);
            CloseHandle(g_mail_go);
            fprintf(stderr, "exec wait: mail lock on A does not block B send/recv\n");
            lmx_msg_runtime_delete(rti);
        }
        rti = lmx_msg_runtime_new();
        {
            LmxMsgAddr p = 0, a = 0, d = 0;
            int refs0;
            int st;
            memset(&any_ctx, 0, sizeof(any_ctx));
            if (rti == 0 || lmx_msg_create(rti, 0, 1, &ini, 1, &p) != LMX_MSG_OK
                || lmx_msg_end_turn(rti, p, 1) != LMX_MSG_OK
                || lmx_msg_create(rti, p, 2, &ini, 1, &a) != LMX_MSG_OK
                || lmx_msg_create(rti, p, 3, &ini, 1, &d) != LMX_MSG_OK
                || lmx_msg_end_turn(rti, p, 1) != LMX_MSG_OK
                || lmx_msg_exec_bind(rti, a, turn_send_once, &any_ctx, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK) {
                fprintf(stderr, "exec mail-oom create\n");
                if (rti != 0) {
                    lmx_msg_runtime_delete(rti);
                }
                return 1;
            }
            g_mail_dest = d;
            refs0 = lmx_msg_endp_refs(rti, d);
            if (lmx_msg_host_post(rti, a, &env) != LMX_MSG_STAGED
                || lmx_msg_host_drain(rti) != LMX_MSG_OK) {
                fprintf(stderr, "exec mail-oom post\n");
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            lmx_msg_test_set_copy_fail(1);
            st = lmx_msg_run_child_turn(rti, a);
            lmx_msg_test_set_copy_fail(0);
            if (st != LMX_MSG_OK || lmx_msg_endp_refs(rti, d) != refs0
                || lmx_msg_inbox_n(rti, d) != 0) {
                fprintf(stderr, "exec mail-oom st=%d refs=%d refs0=%d inbox=%d\n",
                    st, lmx_msg_endp_refs(rti, d), refs0, lmx_msg_inbox_n(rti, d));
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            g_mail_dest = 0;
            fprintf(stderr, "exec wait: send copy OOM leaves dest refs and inbox unchanged\n");
            lmx_msg_runtime_delete(rti);
        }
        rti = lmx_msg_runtime_new();
        {
            LmxMsgAddr p = 0, a = 0, b = 0, d = 0, eaddr = 0;
            memset(&ui_ctx, 0, sizeof(ui_ctx));
            memset(&any_ctx, 0, sizeof(any_ctx));
            g_mail_entered = CreateEventA(0, 1, 0, 0);
            g_mail_go = CreateEventA(0, 1, 0, 0);
            if (rti == 0 || g_mail_entered == 0 || g_mail_go == 0
                || lmx_msg_create(rti, 0, 1, &ini, 1, &p) != LMX_MSG_OK
                || lmx_msg_end_turn(rti, p, 1) != LMX_MSG_OK
                || lmx_msg_create(rti, p, 2, &ini, 1, &a) != LMX_MSG_OK
                || lmx_msg_create(rti, p, 3, &ini, 1, &b) != LMX_MSG_OK
                || lmx_msg_create(rti, p, 4, &ini, 1, &d) != LMX_MSG_OK
                || lmx_msg_create(rti, p, 5, &ini, 1, &eaddr) != LMX_MSG_OK
                || lmx_msg_end_turn(rti, p, 1) != LMX_MSG_OK
                || lmx_msg_exec_bind(rti, a, turn_send_once, &ui_ctx, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK
                || lmx_msg_exec_bind(rti, b, turn_send_once, &any_ctx, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK
                || lmx_msg_exec_start_contexts(rti) != LMX_MSG_OK) {
                fprintf(stderr, "exec admit-gate create\n");
                if (g_mail_entered != 0) {
                    CloseHandle(g_mail_entered);
                }
                if (g_mail_go != 0) {
                    CloseHandle(g_mail_go);
                }
                if (rti != 0) {
                    lmx_msg_runtime_delete(rti);
                }
                return 1;
            }
            Sleep(20);
            g_mail_gate_addr = d;
            g_mail_dest = d;
            InterlockedExchange(&g_mail_in_send, 0);
            InterlockedExchange(&g_mail_gate_any, 1);
            InterlockedExchange(&g_mail_gate_armed, 1);
            ResetEvent(g_mail_entered);
            ResetEvent(g_mail_go);
            lmx_msg_test_mail_locked = mail_gate_hook;
            if (lmx_msg_host_post(rti, a, &env) != LMX_MSG_STAGED
                || lmx_msg_host_drain(rti) != LMX_MSG_OK
                || WaitForSingleObject(g_mail_entered, 2000) != WAIT_OBJECT_0) {
                fprintf(stderr, "exec admit-gate enter\n");
                lmx_msg_test_mail_locked = 0;
                InterlockedExchange(&g_mail_gate_armed, 0);
                SetEvent(g_mail_go);
                lmx_msg_exec_stop(rti);
                CloseHandle(g_mail_entered);
                CloseHandle(g_mail_go);
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            g_mail_dest = eaddr;
            if (lmx_msg_host_post(rti, b, &env) != LMX_MSG_STAGED
                || lmx_msg_host_drain(rti) != LMX_MSG_OK) {
                fprintf(stderr, "exec admit-gate post B\n");
                lmx_msg_test_mail_locked = 0;
                SetEvent(g_mail_go);
                lmx_msg_exec_stop(rti);
                CloseHandle(g_mail_entered);
                CloseHandle(g_mail_go);
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            dl = GetTickCount() + 2000;
            while (InterlockedCompareExchange(&any_ctx.done, 0, 0) == 0 && GetTickCount() < dl) {
                Sleep(10);
            }
            if (InterlockedCompareExchange(&any_ctx.done, 0, 0) != 1) {
                fprintf(stderr, "exec admit-gate B blocked doneA=%ld doneB=%ld\n",
                    (long)InterlockedCompareExchange(&ui_ctx.done, 0, 0),
                    (long)InterlockedCompareExchange(&any_ctx.done, 0, 0));
                lmx_msg_test_mail_locked = 0;
                SetEvent(g_mail_go);
                lmx_msg_exec_stop(rti);
                CloseHandle(g_mail_entered);
                CloseHandle(g_mail_go);
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            SetEvent(g_mail_go);
            dl = GetTickCount() + 2000;
            while (InterlockedCompareExchange(&ui_ctx.done, 0, 0) == 0 && GetTickCount() < dl) {
                Sleep(10);
            }
            lmx_msg_test_mail_locked = 0;
            g_mail_gate_addr = 0;
            g_mail_dest = 0;
            lmx_msg_exec_stop(rti);
            CloseHandle(g_mail_entered);
            CloseHandle(g_mail_go);
            if (InterlockedCompareExchange(&ui_ctx.done, 0, 0) != 1) {
                fprintf(stderr, "exec admit-gate A stuck\n");
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            fprintf(stderr, "exec wait: dest A mail during admit does not block B end_turn\n");
            lmx_msg_runtime_delete(rti);
        }
        rti = lmx_msg_runtime_new();
        {
            LmxMsgAddr p = 0, a = 0, d = 0;
            int refs0;
            int refs1;
            memset(&any_ctx, 0, sizeof(any_ctx));
            if (rti == 0 || lmx_msg_create(rti, 0, 1, &ini, 1, &p) != LMX_MSG_OK
                || lmx_msg_end_turn(rti, p, 1) != LMX_MSG_OK
                || lmx_msg_create(rti, p, 2, &ini, 1, &a) != LMX_MSG_OK
                || lmx_msg_create(rti, p, 3, &ini, 1, &d) != LMX_MSG_OK
                || lmx_msg_end_turn(rti, p, 1) != LMX_MSG_OK
                || lmx_msg_exec_bind(rti, a, turn_send_once, &any_ctx, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK) {
                fprintf(stderr, "exec outbox-gone create\n");
                if (rti != 0) {
                    lmx_msg_runtime_delete(rti);
                }
                return 1;
            }
            g_mail_dest = d;
            refs0 = lmx_msg_endp_refs(rti, d);
            if (lmx_msg_host_post(rti, a, &env) != LMX_MSG_STAGED
                || lmx_msg_host_drain(rti) != LMX_MSG_OK) {
                fprintf(stderr, "exec outbox-gone post\n");
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            lmx_msg_test_after_outbox_xfer = dest_stop_after_outbox;
            if (lmx_msg_run_child_turn(rti, a) != LMX_MSG_OK) {
                lmx_msg_test_after_outbox_xfer = 0;
                fprintf(stderr, "exec outbox-gone turn\n");
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            lmx_msg_test_after_outbox_xfer = 0;
            refs1 = lmx_msg_endp_refs(rti, d);
            g_mail_dest = 0;
            if (lmx_msg_inbox_n(rti, d) != 0 || refs1 > refs0 + 1) {
                fprintf(stderr, "exec outbox-gone inbox n=%d refs0=%d refs1=%d\n",
                    lmx_msg_inbox_n(rti, d), refs0, refs1);
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            fprintf(stderr, "exec wait: dest stop after outbox take drops GONE; inbox empty\n");
            lmx_msg_runtime_delete(rti);
        }
        rti = lmx_msg_runtime_new();
        {
            LmxMsgAddr p = 0, a = 0, d = 0;
            int refs0;
            LmxMsgEnv got;
            memset(&any_ctx, 0, sizeof(any_ctx));
            memset(&got, 0, sizeof(got));
            if (rti == 0 || lmx_msg_create(rti, 0, 1, &ini, 1, &p) != LMX_MSG_OK
                || lmx_msg_end_turn(rti, p, 1) != LMX_MSG_OK
                || lmx_msg_create(rti, p, 2, &ini, 1, &a) != LMX_MSG_OK
                || lmx_msg_create(rti, p, 3, &ini, 1, &d) != LMX_MSG_OK
                || lmx_msg_end_turn(rti, p, 1) != LMX_MSG_OK
                || lmx_msg_exec_bind(rti, a, turn_send_once, &any_ctx, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK) {
                fprintf(stderr, "exec pin-oom create\n");
                if (rti != 0) {
                    lmx_msg_runtime_delete(rti);
                }
                return 1;
            }
            g_mail_dest = d;
            refs0 = lmx_msg_endp_refs(rti, d);
            if (lmx_msg_host_post(rti, a, &env) != LMX_MSG_STAGED
                || lmx_msg_host_drain(rti) != LMX_MSG_OK) {
                fprintf(stderr, "exec pin-oom post\n");
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            lmx_msg_test_after_outbox_xfer = dest_pin_fail_after_outbox;
            (void)lmx_msg_run_child_turn(rti, a);
            lmx_msg_test_after_outbox_xfer = 0;
            lmx_msg_test_fail_retain = 0;
            if (lmx_msg_inbox_n(rti, d) != 0 || lmx_msg_endp_refs(rti, d) != refs0) {
                fprintf(stderr, "exec pin-oom residue inbox=%d refs=%d refs0=%d fail=%d\n",
                    lmx_msg_inbox_n(rti, d), lmx_msg_endp_refs(rti, d), refs0, lmx_msg_test_fail_retain);
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            if (lmx_msg_host_post(rti, a, &env) != LMX_MSG_STAGED
                || lmx_msg_host_drain(rti) != LMX_MSG_OK
                || lmx_msg_run_child_turn(rti, a) != LMX_MSG_OK
                || lmx_msg_inbox_n(rti, d) != 1) {
                fprintf(stderr, "exec pin-oom retry inbox=%d\n", lmx_msg_inbox_n(rti, d));
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            memset(&ui_ctx, 0, sizeof(ui_ctx));
            if (lmx_msg_exec_bind(rti, d, turn_recv_end, &ui_ctx, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK
                || lmx_msg_run_child_turn(rti, d) != LMX_MSG_OK
                || lmx_msg_inbox_n(rti, d) != 0
                || InterlockedCompareExchange(&ui_ctx.done, 0, 0) != 1) {
                fprintf(stderr, "exec pin-oom second copy inbox=%d done=%ld\n",
                    lmx_msg_inbox_n(rti, d), (long)InterlockedCompareExchange(&ui_ctx.done, 0, 0));
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            g_mail_dest = 0;
            fprintf(stderr, "exec wait: dest pin OOM rolls back; retry admits exactly once\n");
            lmx_msg_runtime_delete(rti);
        }
        rti = lmx_msg_runtime_new();
        {
            LmxMsgAddr p = 0, a = 0, d = 0;
            LmxMsgEnv got;
            unsigned r1 = 0, r2 = 0;
            memset(&any_ctx, 0, sizeof(any_ctx));
            memset(&got, 0, sizeof(got));
            g_fifo_n = 0;
            g_fifo_ids[0] = 0;
            g_fifo_ids[1] = 0;
            if (rti == 0 || lmx_msg_create(rti, 0, 1, &ini, 1, &p) != LMX_MSG_OK
                || lmx_msg_end_turn(rti, p, 1) != LMX_MSG_OK
                || lmx_msg_create(rti, p, 2, &ini, 1, &a) != LMX_MSG_OK
                || lmx_msg_create(rti, p, 3, &ini, 1, &d) != LMX_MSG_OK
                || lmx_msg_end_turn(rti, p, 1) != LMX_MSG_OK
                || lmx_msg_exec_bind(rti, a, turn_send_two, &any_ctx, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK) {
                fprintf(stderr, "exec fifo-xfer create\n");
                if (rti != 0) {
                    lmx_msg_runtime_delete(rti);
                }
                return 1;
            }
            g_mail_dest = d;
            lmx_msg_test_after_outbox_xfer = outbox_fifo_hook;
            if (lmx_msg_host_post(rti, a, &env) != LMX_MSG_STAGED
                || lmx_msg_host_drain(rti) != LMX_MSG_OK
                || lmx_msg_run_child_turn(rti, a) != LMX_MSG_OK) {
                lmx_msg_test_after_outbox_xfer = 0;
                fprintf(stderr, "exec fifo-xfer turn\n");
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            lmx_msg_test_after_outbox_xfer = 0;
            if (g_fifo_n != 2 || g_fifo_ids[0] != 11U || g_fifo_ids[1] != 22U
                || lmx_msg_inbox_n(rti, d) != 2) {
                fprintf(stderr, "exec fifo-xfer n=%d ids=%u,%u inbox=%d\n",
                    g_fifo_n, g_fifo_ids[0], g_fifo_ids[1], lmx_msg_inbox_n(rti, d));
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            if (lmx_msg_exec_bind(rti, d, turn_recv_end, &ui_ctx, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK
                || lmx_msg_run_child_turn(rti, d) != LMX_MSG_OK) {
                fprintf(stderr, "exec fifo-xfer recv1\n");
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            memset(&got, 0, sizeof(got));
            /* turn_recv_end already recvd one; second remains */
            if (lmx_msg_inbox_n(rti, d) != 1) {
                fprintf(stderr, "exec fifo-xfer after first recv inbox=%d\n", lmx_msg_inbox_n(rti, d));
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            (void)r1;
            (void)r2;
            g_mail_dest = 0;
            fprintf(stderr, "exec wait: end_turn splice FIFO 11 then 22; one owner per node\n");
            lmx_msg_runtime_delete(rti);
        }
        rti = lmx_msg_runtime_new();
        {
            LmxMsgAddr p = 0, a = 0, d = 0;
            StageJob ja, jb;
            HANDLE tha, thb;
            int has11, has12, has21, has22, i;
            memset(&any_ctx, 0, sizeof(any_ctx));
            if (rti == 0 || lmx_msg_create(rti, 0, 1, &ini, 1, &p) != LMX_MSG_OK
                || lmx_msg_end_turn(rti, p, 1) != LMX_MSG_OK
                || lmx_msg_create(rti, p, 2, &ini, 1, &a) != LMX_MSG_OK
                || lmx_msg_create(rti, p, 3, &ini, 1, &d) != LMX_MSG_OK
                || lmx_msg_end_turn(rti, p, 1) != LMX_MSG_OK
                || lmx_msg_exec_bind(rti, a, turn_just_end, &any_ctx, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK) {
                fprintf(stderr, "exec two-prod create\n");
                if (rti != 0) {
                    lmx_msg_runtime_delete(rti);
                }
                return 1;
            }
            ja.rt = rti; ja.from = a; ja.to = d; ja.id0 = 11U; ja.id1 = 12U;
            jb.rt = rti; jb.from = a; jb.to = d; jb.id0 = 21U; jb.id1 = 22U;
            tha = CreateThread(0, 0, stage_prod_thread, &ja, 0, 0);
            thb = CreateThread(0, 0, stage_prod_thread, &jb, 0, 0);
            if (tha == 0 || thb == 0
                || WaitForSingleObject(tha, 2000) != WAIT_OBJECT_0
                || WaitForSingleObject(thb, 2000) != WAIT_OBJECT_0) {
                fprintf(stderr, "exec two-prod stage threads\n");
                if (tha != 0) {
                    CloseHandle(tha);
                }
                if (thb != 0) {
                    CloseHandle(thb);
                }
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            CloseHandle(tha);
            CloseHandle(thb);
            g_fifo_n = 0;
            lmx_msg_test_after_outbox_xfer = outbox_fifo_hook;
            if (lmx_msg_run_child_turn(rti, a) != LMX_MSG_OK) {
                lmx_msg_test_after_outbox_xfer = 0;
                fprintf(stderr, "exec two-prod end_turn\n");
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            lmx_msg_test_after_outbox_xfer = 0;
            has11 = has12 = has21 = has22 = 0;
            for (i = 0; i < g_fifo_n; i++) {
                if (g_fifo_ids[i] == 11U) has11 = i + 1;
                if (g_fifo_ids[i] == 12U) has12 = i + 1;
                if (g_fifo_ids[i] == 21U) has21 = i + 1;
                if (g_fifo_ids[i] == 22U) has22 = i + 1;
            }
            if (g_fifo_n != 4 || has11 == 0 || has12 == 0 || has21 == 0 || has22 == 0
                || has11 > has12 || has21 > has22
                || lmx_msg_inbox_n(rti, d) != 4
                || lmx_msg_mail_outbox_empty(lmx_msg_find(rti, a)) == 0) {
                fprintf(stderr, "exec two-prod n=%d inbox=%d\n", g_fifo_n, lmx_msg_inbox_n(rti, d));
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            fprintf(stderr, "exec wait: two producers same source outbox; splice owns all four ids once\n");
            lmx_msg_runtime_delete(rti);
        }
        rti = lmx_msg_runtime_new();
        {
            LmxMsgAddr p = 0, a = 0, b = 0, d = 0;
            LmxMsgEnv got;
            HANDLE th;
            volatile LONG t2ok = 0;
            memset(&got, 0, sizeof(got));
            g_mail_entered = CreateEventA(0, 1, 0, 0);
            g_mail_go = CreateEventA(0, 1, 0, 0);
            if (rti == 0 || g_mail_entered == 0 || g_mail_go == 0
                || lmx_msg_create(rti, 0, 1, &ini, 1, &p) != LMX_MSG_OK
                || lmx_msg_end_turn(rti, p, 1) != LMX_MSG_OK
                || lmx_msg_create(rti, p, 2, &ini, 1, &a) != LMX_MSG_OK
                || lmx_msg_create(rti, p, 3, &ini, 1, &b) != LMX_MSG_OK
                || lmx_msg_create(rti, p, 4, &ini, 1, &d) != LMX_MSG_OK
                || lmx_msg_end_turn(rti, p, 1) != LMX_MSG_OK
                || lmx_msg_host_post(rti, a, &env) != LMX_MSG_STAGED
                || lmx_msg_host_drain(rti) != LMX_MSG_OK) {
                fprintf(stderr, "exec nself-recv create\n");
                if (g_mail_entered != 0) {
                    CloseHandle(g_mail_entered);
                }
                if (g_mail_go != 0) {
                    CloseHandle(g_mail_go);
                }
                if (rti != 0) {
                    lmx_msg_runtime_delete(rti);
                }
                return 1;
            }
            g_reap_rt = rti;
            g_nself_from = b;
            g_mail_dest = d;
            ResetEvent(g_mail_entered);
            ResetEvent(g_mail_go);
            lmx_msg_test_after_recv_pin = nself_recv_pin_hook;
            th = CreateThread(0, 0, nself_stage_thread, (void *)&t2ok, 0, 0);
            if (th == 0 || lmx_msg_recv(rti, a, &got) != LMX_MSG_OK
                || WaitForSingleObject(th, 2000) != WAIT_OBJECT_0
                || InterlockedCompareExchange(&t2ok, 0, 0) == 0) {
                fprintf(stderr, "exec nself-recv gate t2=%ld\n", (long)InterlockedCompareExchange(&t2ok, 0, 0));
                lmx_msg_test_after_recv_pin = 0;
                SetEvent(g_mail_go);
                if (th != 0) {
                    WaitForSingleObject(th, 1000);
                    CloseHandle(th);
                }
                CloseHandle(g_mail_entered);
                CloseHandle(g_mail_go);
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            CloseHandle(th);
            lmx_msg_test_after_recv_pin = 0;
            g_reap_rt = 0;
            CloseHandle(g_mail_entered);
            CloseHandle(g_mail_go);
            lmx_msg_env_release(&got);
            fprintf(stderr, "exec wait: non-self recv mail on A does not hold exec against B stage\n");
            lmx_msg_runtime_delete(rti);
        }
        rti = lmx_msg_runtime_new();
        {
            LmxMsgAddr p = 0, a = 0;
            LmxMsgEnv got;
            memset(&got, 0, sizeof(got));
            if (rti == 0 || lmx_msg_create(rti, 0, 1, &ini, 1, &p) != LMX_MSG_OK
                || lmx_msg_end_turn(rti, p, 1) != LMX_MSG_OK
                || lmx_msg_create(rti, p, 2, &ini, 1, &a) != LMX_MSG_OK
                || lmx_msg_end_turn(rti, p, 1) != LMX_MSG_OK
                || lmx_msg_host_post(rti, a, &env) != LMX_MSG_STAGED
                || lmx_msg_host_post(rti, a, &env) != LMX_MSG_STAGED
                || lmx_msg_host_drain(rti) != LMX_MSG_OK
                || lmx_msg_inbox_n(rti, a) != 2) {
                fprintf(stderr, "exec fail-walk create\n");
                if (rti != 0) {
                    lmx_msg_runtime_delete(rti);
                }
                return 1;
            }
            if (lmx_msg_recv(rti, a, &got) != LMX_MSG_OK || lmx_msg_inbox_n(rti, a) != 1) {
                fprintf(stderr, "exec fail-walk recv n=%d\n", lmx_msg_inbox_n(rti, a));
                lmx_msg_env_release(&got);
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            lmx_msg_env_release(&got);
            if (lmx_msg_fail(rti, a) != LMX_MSG_OK || lmx_msg_inbox_n(rti, a) != 0
                || lmx_msg_recv(rti, a, &got) != LMX_MSG_EMPTY) {
                fprintf(stderr, "exec fail-walk after fail n=%d\n", lmx_msg_inbox_n(rti, a));
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            fprintf(stderr, "exec wait: recv then fail: one delivered, remainder detached, EMPTY after\n");
            lmx_msg_runtime_delete(rti);
        }
        rti = lmx_msg_runtime_new();
        {
            LmxMsgAddr p = 0, a = 0, r1 = 0, r2 = 0;
            LmxMsgEnv e1;
            memset(&e1, 0, sizeof(e1));
            e1.kind = LMX_MSG_KIND_BYTES;
            if (rti == 0 || lmx_msg_create(rti, 0, 1, &ini, 1, &p) != LMX_MSG_OK
                || lmx_msg_end_turn(rti, p, 1) != LMX_MSG_OK
                || lmx_msg_create(rti, p, 2, &ini, 1, &a) != LMX_MSG_OK
                || lmx_msg_create(rti, p, 3, &ini, 1, &r1) != LMX_MSG_OK
                || lmx_msg_create(rti, p, 4, &ini, 1, &r2) != LMX_MSG_OK
                || lmx_msg_end_turn(rti, p, 1) != LMX_MSG_OK) {
                fprintf(stderr, "exec fail-oom create\n");
                if (rti != 0) {
                    lmx_msg_runtime_delete(rti);
                }
                return 1;
            }
            e1.reply_to = r1;
            e1.id = 11U;
            if (lmx_msg_host_post(rti, a, &e1) != LMX_MSG_STAGED) {
                fprintf(stderr, "exec fail-oom post1\n");
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            e1.reply_to = r2;
            e1.id = 22U;
            if (lmx_msg_host_post(rti, a, &e1) != LMX_MSG_STAGED
                || lmx_msg_host_drain(rti) != LMX_MSG_OK
                || lmx_msg_inbox_n(rti, a) != 2) {
                fprintf(stderr, "exec fail-oom drain n=%d\n", lmx_msg_inbox_n(rti, a));
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            lmx_msg_test_fail_post_dead = 2;
            if (lmx_msg_fail(rti, a) != LMX_MSG_NOMEM || lmx_msg_inbox_n(rti, a) != 1) {
                fprintf(stderr, "exec fail-oom first st inbox=%d failn=%d\n",
                    lmx_msg_inbox_n(rti, a), lmx_msg_test_fail_post_dead);
                lmx_msg_test_fail_post_dead = 0;
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            lmx_msg_test_fail_post_dead = 0;
            if (lmx_msg_fail(rti, a) != LMX_MSG_OK || lmx_msg_inbox_n(rti, a) != 0) {
                fprintf(stderr, "exec fail-oom retry n=%d\n", lmx_msg_inbox_n(rti, a));
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            {
                LmxMsgEnv d1;
                LmxMsgEnv d2;
                memset(&d1, 0, sizeof(d1));
                memset(&d2, 0, sizeof(d2));
                if (lmx_msg_recv(rti, r1, &d1) != LMX_MSG_OK || d1.kind != LMX_MSG_KIND_DEAD
                    || d1.correlation != 11U || lmx_msg_inbox_n(rti, r1) != 0
                    || lmx_msg_recv(rti, r2, &d2) != LMX_MSG_OK || d2.kind != LMX_MSG_KIND_DEAD
                    || d2.correlation != 22U || lmx_msg_inbox_n(rti, r2) != 0
                    || lmx_msg_recv(rti, r1, &d1) != LMX_MSG_EMPTY
                    || lmx_msg_recv(rti, r2, &d2) != LMX_MSG_EMPTY) {
                    fprintf(stderr, "exec fail-oom recipients k1=%d c1=%u k2=%d c2=%u\n",
                        d1.kind, d1.correlation, d2.kind, d2.correlation);
                    lmx_msg_env_release(&d1);
                    lmx_msg_env_release(&d2);
                    lmx_msg_runtime_delete(rti);
                    return 1;
                }
                lmx_msg_env_release(&d1);
                lmx_msg_env_release(&d2);
            }
            fprintf(stderr, "exec wait: fail post_dead OOM keeps unnotified inbox; retry drains once\n");
            lmx_msg_runtime_delete(rti);
        }
        rti = lmx_msg_runtime_new();
        {
            LmxMsgAddr p = 0, a = 0;
            LmxMsgEnv got;
            memset(&got, 0, sizeof(got));
            if (rti == 0 || lmx_msg_create(rti, 0, 1, &ini, 1, &p) != LMX_MSG_OK
                || lmx_msg_end_turn(rti, p, 1) != LMX_MSG_OK
                || lmx_msg_create(rti, p, 2, &ini, 1, &a) != LMX_MSG_OK
                || lmx_msg_end_turn(rti, p, 1) != LMX_MSG_OK
                || lmx_msg_host_post(rti, a, &env) != LMX_MSG_STAGED
                || lmx_msg_host_drain(rti) != LMX_MSG_OK) {
                fprintf(stderr, "exec recv-fail-ov create\n");
                if (rti != 0) {
                    lmx_msg_runtime_delete(rti);
                }
                return 1;
            }
            lmx_msg_test_after_recv_pin = recv_fail_overlap_hook;
            if (lmx_msg_recv(rti, a, &got) != LMX_MSG_EMPTY || lmx_msg_inbox_n(rti, a) != 0) {
                fprintf(stderr, "exec recv-fail-ov st inbox=%d\n", lmx_msg_inbox_n(rti, a));
                lmx_msg_test_after_recv_pin = 0;
                lmx_msg_env_release(&got);
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            lmx_msg_test_after_recv_pin = 0;
            fprintf(stderr, "exec wait: recv/fail overlap: fail detaches before pop; recv EMPTY\n");
            lmx_msg_runtime_delete(rti);
        }
        rti = lmx_msg_runtime_new();
        {
            LmxMsgAddr p = 0, a = 0;
            LmxMsgEnv got;
            memset(&got, 0, sizeof(got));
            if (rti == 0 || lmx_msg_create(rti, 0, 1, &ini, 1, &p) != LMX_MSG_OK
                || lmx_msg_end_turn(rti, p, 1) != LMX_MSG_OK
                || lmx_msg_create(rti, p, 2, &ini, 1, &a) != LMX_MSG_OK
                || lmx_msg_end_turn(rti, p, 1) != LMX_MSG_OK
                || lmx_msg_exec_bind(rti, a, turn_recv_end, &any_ctx, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK
                || lmx_msg_host_post(rti, a, &env) != LMX_MSG_STAGED
                || lmx_msg_host_drain(rti) != LMX_MSG_OK) {
                fprintf(stderr, "exec recv-unbind-ov create\n");
                if (rti != 0) {
                    lmx_msg_runtime_delete(rti);
                }
                return 1;
            }
            lmx_msg_test_after_recv_pin = recv_unbind_overlap_hook;
            if (lmx_msg_recv(rti, a, &got) != LMX_MSG_OK || lmx_msg_inbox_n(rti, a) != 0) {
                fprintf(stderr, "exec recv-unbind-ov inbox=%d\n", lmx_msg_inbox_n(rti, a));
                lmx_msg_test_after_recv_pin = 0;
                lmx_msg_env_release(&got);
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            lmx_msg_test_after_recv_pin = 0;
            lmx_msg_env_release(&got);
            fprintf(stderr, "exec wait: unbind between pin and pop; recv still owns the node\n");
            lmx_msg_runtime_delete(rti);
        }
        rti = lmx_msg_runtime_new();
        {
            LmxMsgAddr p = 0, a = 0;
            LmxMsgEnv got;
            int refs0;
            memset(&got, 0, sizeof(got));
            if (rti == 0 || lmx_msg_create(rti, 0, 1, &ini, 1, &p) != LMX_MSG_OK
                || lmx_msg_end_turn(rti, p, 1) != LMX_MSG_OK
                || lmx_msg_create(rti, p, 2, &ini, 1, &a) != LMX_MSG_OK
                || lmx_msg_end_turn(rti, p, 1) != LMX_MSG_OK
                || lmx_msg_host_post(rti, a, &env) != LMX_MSG_STAGED
                || lmx_msg_host_drain(rti) != LMX_MSG_OK) {
                fprintf(stderr, "exec nself-pin create\n");
                if (rti != 0) {
                    lmx_msg_runtime_delete(rti);
                }
                return 1;
            }
            refs0 = lmx_msg_endp_refs(rti, a);
            lmx_msg_test_fail_retain = 1;
            if (lmx_msg_recv(rti, a, &got) != LMX_MSG_OK || lmx_msg_inbox_n(rti, a) != 0
                || lmx_msg_endp_refs(rti, a) != refs0) {
                fprintf(stderr, "exec nself-pin st inbox=%d refs=%d refs0=%d\n",
                    lmx_msg_inbox_n(rti, a), lmx_msg_endp_refs(rti, a), refs0);
                lmx_msg_test_fail_retain = 0;
                lmx_msg_env_release(&got);
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            lmx_msg_test_fail_retain = 0;
            lmx_msg_env_release(&got);
            fprintf(stderr, "exec wait: non-self recv retain fail still pops; pin released\n");
            lmx_msg_runtime_delete(rti);
        }
        rti = lmx_msg_runtime_new();
        {
            LmxMsgAddr p = 0, c1 = 0, c2 = 0;
            LmxMsg *cm2;
            memset(&any_ctx, 0, sizeof(any_ctx));
            memset(&ui_ctx, 0, sizeof(ui_ctx));
            if (rti == 0 || lmx_msg_create(rti, 0, 1, &ini, 1, &p) != LMX_MSG_OK
                || lmx_msg_end_turn(rti, p, 1) != LMX_MSG_OK
                || lmx_msg_create(rti, p, 2, &ini, 1, &c1) != LMX_MSG_OK
                || lmx_msg_create(rti, p, 3, &ini, 1, &c2) != LMX_MSG_OK
                || lmx_msg_end_turn(rti, p, 1) != LMX_MSG_OK
                || lmx_msg_exec_bind(rti, c1, turn_recv_end, &any_ctx, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK
                || lmx_msg_exec_bind(rti, c2, turn_recv_end, &ui_ctx, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK
                || lmx_msg_host_post(rti, c1, &env) != LMX_MSG_STAGED
                || lmx_msg_host_drain(rti) != LMX_MSG_OK) {
                fprintf(stderr, "exec sched-snap create\n");
                if (rti != 0) {
                    lmx_msg_runtime_delete(rti);
                }
                return 1;
            }
            lmx_msg_exec_unbind(rti, c2);
            cm2 = lmx_msg_find(rti, c2);
            g_sched_drop = cm2;
            lmx_msg_test_after_sched_snap = sched_snap_drop_hook;
            {
                int sst = lmx_msg_sched_step(rti, p);
                if (sst != LMX_MSG_OK
                    || InterlockedCompareExchange(&any_ctx.done, 0, 0) != 1) {
                    fprintf(stderr, "exec sched-snap step st=%d done=%ld\n",
                        sst, (long)InterlockedCompareExchange(&any_ctx.done, 0, 0));
                    lmx_msg_test_after_sched_snap = 0;
                    g_sched_drop = 0;
                    lmx_msg_runtime_delete(rti);
                    return 1;
                }
            }
            lmx_msg_test_after_sched_snap = 0;
            g_sched_drop = 0;
            fprintf(stderr, "exec wait: sched_step snap survives sibling unlink overlap\n");
            lmx_msg_runtime_delete(rti);
        }
        rti = lmx_msg_runtime_new();
        {
            enum { NIDLE = 64 };
            LmxMsgAddr p = 0, kids[NIDLE + 1];
            TurnCtx idle_ctx[NIDLE + 1];
            int i;
            memset(&any_ctx, 0, sizeof(any_ctx));
            memset(kids, 0, sizeof(kids));
            memset(idle_ctx, 0, sizeof(idle_ctx));
            if (rti == 0 || lmx_msg_create(rti, 0, 1, &ini, 1, &p) != LMX_MSG_OK
                || lmx_msg_end_turn(rti, p, 1) != LMX_MSG_OK) {
                fprintf(stderr, "exec sched-65 create p\n");
                if (rti != 0) {
                    lmx_msg_runtime_delete(rti);
                }
                return 1;
            }
            for (i = 0; i < NIDLE + 1; i++) {
                if (lmx_msg_create(rti, p, (unsigned)(10 + i), &ini, 1, &kids[i]) != LMX_MSG_OK
                    || lmx_msg_end_turn(rti, p, 1) != LMX_MSG_OK
                    || lmx_msg_exec_bind(rti, kids[i],
                        i < NIDLE ? turn_just_end : turn_recv_end,
                        i < NIDLE ? &idle_ctx[i] : &any_ctx,
                        LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK) {
                    fprintf(stderr, "exec sched-65 child i=%d\n", i);
                    lmx_msg_runtime_delete(rti);
                    return 1;
                }
            }
            if (lmx_msg_host_post(rti, kids[NIDLE], &env) != LMX_MSG_STAGED
                || lmx_msg_host_drain(rti) != LMX_MSG_OK
                || lmx_msg_sched_step(rti, p) != LMX_MSG_OK
                || InterlockedCompareExchange(&any_ctx.done, 0, 0) != 1) {
                fprintf(stderr, "exec sched-65 done=%ld\n",
                    (long)InterlockedCompareExchange(&any_ctx.done, 0, 0));
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            fprintf(stderr, "exec wait: sched_step runs 65th child when first 64 inboxes are empty\n");
            lmx_msg_runtime_delete(rti);
        }
        rti = lmx_msg_runtime_new();
        {
            /* Decision 18: the parent's step continues after its cursor, the
             * direct child it last gave a turn. A and B each hold two inputs;
             * four host steps (the parent holds no turn) run A, B, A, B. */
            LmxMsgAddr p = 0, ka = 0, kb = 0;
            long da[4];
            long db[4];
            int k;
            memset(&any_ctx, 0, sizeof(any_ctx));
            memset(&ui_ctx, 0, sizeof(ui_ctx));
            if (rti == 0 || lmx_msg_create(rti, 0, 1, &ini, 1, &p) != LMX_MSG_OK
                || lmx_msg_end_turn(rti, p, 1) != LMX_MSG_OK
                || lmx_msg_create(rti, p, 2, &ini, 1, &ka) != LMX_MSG_OK
                || lmx_msg_create(rti, p, 3, &ini, 1, &kb) != LMX_MSG_OK
                || lmx_msg_end_turn(rti, p, 1) != LMX_MSG_OK
                || lmx_msg_exec_bind(rti, ka, turn_recv_end, &any_ctx, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK
                || lmx_msg_exec_bind(rti, kb, turn_recv_end, &ui_ctx, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK
                || lmx_msg_host_post(rti, ka, &env) != LMX_MSG_STAGED
                || lmx_msg_host_post(rti, ka, &env) != LMX_MSG_STAGED
                || lmx_msg_host_post(rti, kb, &env) != LMX_MSG_STAGED
                || lmx_msg_host_post(rti, kb, &env) != LMX_MSG_STAGED
                || lmx_msg_host_drain(rti) != LMX_MSG_OK) {
                fprintf(stderr, "exec sched-cursor host create\n");
                if (rti != 0) {
                    lmx_msg_runtime_delete(rti);
                }
                return 1;
            }
            for (k = 0; k < 4; k++) {
                if (lmx_msg_sched_step(rti, p) != LMX_MSG_OK) {
                    fprintf(stderr, "exec sched-cursor host step k=%d\n", k);
                    lmx_msg_runtime_delete(rti);
                    return 1;
                }
                da[k] = InterlockedCompareExchange(&any_ctx.done, 0, 0);
                db[k] = InterlockedCompareExchange(&ui_ctx.done, 0, 0);
            }
            if (da[0] != 1 || db[0] != 0 || da[1] != 1 || db[1] != 1
                || da[2] != 2 || db[2] != 1 || da[3] != 2 || db[3] != 2) {
                fprintf(stderr, "exec sched-cursor host order a=%ld,%ld,%ld,%ld b=%ld,%ld,%ld,%ld\n",
                    da[0], da[1], da[2], da[3], db[0], db[1], db[2], db[3]);
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            fprintf(stderr, "exec wait: host parent step continues after its cursor: A, B, A, B\n");
            lmx_msg_runtime_delete(rti);
        }
        rti = lmx_msg_runtime_new();
        {
            /* Decision 18: the same order when the step runs on the parent's own
             * lane, inside its turn (lmx_msg_sched_pick in lm1 and lm2). */
            LmxMsgAddr p = 0, ka = 0, kb = 0;
            StepCtx sc;
            int st1;
            int st2;
            memset(&any_ctx, 0, sizeof(any_ctx));
            memset(&ui_ctx, 0, sizeof(ui_ctx));
            memset(&sc, 0, sizeof(sc));
            sc.a = &any_ctx;
            sc.b = &ui_ctx;
            if (rti == 0 || lmx_msg_create(rti, 0, 1, &ini, 1, &p) != LMX_MSG_OK
                || lmx_msg_end_turn(rti, p, 1) != LMX_MSG_OK
                || lmx_msg_create(rti, p, 2, &ini, 1, &ka) != LMX_MSG_OK
                || lmx_msg_create(rti, p, 3, &ini, 1, &kb) != LMX_MSG_OK
                || lmx_msg_end_turn(rti, p, 1) != LMX_MSG_OK
                || lmx_msg_exec_bind(rti, ka, turn_recv_end, &any_ctx, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK
                || lmx_msg_exec_bind(rti, kb, turn_recv_end, &ui_ctx, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK
                || lmx_msg_exec_bind(rti, p, turn_parent_steps, &sc, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK
                || lmx_msg_host_post(rti, ka, &env) != LMX_MSG_STAGED
                || lmx_msg_host_post(rti, ka, &env) != LMX_MSG_STAGED
                || lmx_msg_host_post(rti, kb, &env) != LMX_MSG_STAGED
                || lmx_msg_host_post(rti, kb, &env) != LMX_MSG_STAGED
                || lmx_msg_host_post(rti, p, &env) != LMX_MSG_STAGED
                || lmx_msg_host_post(rti, p, &env) != LMX_MSG_STAGED
                || lmx_msg_host_drain(rti) != LMX_MSG_OK) {
                fprintf(stderr, "exec sched-cursor turn create\n");
                if (rti != 0) {
                    lmx_msg_runtime_delete(rti);
                }
                return 1;
            }
            st1 = lmx_msg_run_child_turn(rti, p);
            st2 = lmx_msg_run_child_turn(rti, p);
            if ((st1 != LMX_MSG_OK && st1 != 1) || (st2 != LMX_MSG_OK && st2 != 1) || sc.st != 0 || sc.n != 4
                || sc.order[0] != 1 || sc.order[1] != 2 || sc.order[2] != 1 || sc.order[3] != 2) {
                fprintf(stderr, "exec sched-cursor turn st=%d/%d step_st=%d n=%d order=%d,%d,%d,%d\n",
                    st1, st2, sc.st, sc.n, sc.order[0], sc.order[1], sc.order[2], sc.order[3]);
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            fprintf(stderr, "exec wait: parent step in its own turn continues after its cursor: A, B, A, B\n");
            lmx_msg_runtime_delete(rti);
        }
        rti = lmx_msg_runtime_new();
        {
            LmxMsgAddr p = 0;
            LmxMsgAddr sib = 0;
            LmxMsgAddr kid = 0;
            LmxMsg *pm;
            LmxMsg *sm;
            LmxMsg *km;
            memset(&ui_ctx, 0, sizeof(ui_ctx));
            memset(&any_ctx, 0, sizeof(any_ctx));
            if (rti == 0 || lmx_msg_create(rti, 0, 1, &ini, 1, &p) != LMX_MSG_OK
                || lmx_msg_create(rti, p, 2, &ini, 1, &sib) != LMX_MSG_OK
                || lmx_msg_end_turn(rti, p, 1) != LMX_MSG_OK
                || lmx_msg_create(rti, p, 3, &ini, 1, &kid) != LMX_MSG_OK
                || lmx_msg_exec_bind(rti, sib, turn_recv_end, &ui_ctx, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK
                || lmx_msg_exec_bind(rti, kid, turn_recv_end, &any_ctx, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK
                || lmx_msg_send(rti, p, sib, &env) != LMX_MSG_STAGED
                || lmx_msg_send(rti, p, kid, &env) != LMX_MSG_STAGED) {
                fprintf(stderr, "exec map-reparent create\n");
                if (rti != 0) {
                    lmx_msg_runtime_delete(rti);
                }
                return 1;
            }
            lmx_msg_pump(rti);
            pm = lmx_msg_find(rti, p);
            sm = lmx_msg_find(rti, sib);
            km = lmx_msg_find(rti, kid);
            if (pm == 0 || sm == 0 || km == 0) {
                fprintf(stderr, "exec map-reparent find\n");
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            sm->mapped = 1;
            km->mapped = 1;
            lmx_msg_exec_ready(rti, sib);
            lmx_msg_exec_ready(rti, kid);
            if (lmx_msg_exec_map_queued(rti, sib) == 0 || lmx_msg_exec_map_queued(rti, kid) == 0) {
                fprintf(stderr, "exec map-reparent not queued\n");
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            if (lmx_msg_end_turn(rti, p, 0) != LMX_MSG_OK
                || lmx_msg_find(rti, kid) != 0
                || lmx_msg_exec_map_queued(rti, sib) == 0) {
                fprintf(stderr, "exec map-reparent sibling ready=%d find_kid=%d\n",
                    lmx_msg_exec_map_queued(rti, sib), lmx_msg_find(rti, kid) != 0);
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            if (lmx_msg_exec_start_contexts(rti) != LMX_MSG_OK) {
                fprintf(stderr, "exec map-reparent start\n");
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            if (lmx_msg_host_post(rti, sib, &env) != LMX_MSG_STAGED
                || lmx_msg_host_drain(rti) != LMX_MSG_OK) {
                fprintf(stderr, "exec map-reparent host_post\n");
                lmx_msg_exec_stop(rti);
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            lmx_msg_exec_ready(rti, sib);
            dl = GetTickCount() + 2000;
            while (InterlockedCompareExchange(&ui_ctx.done, 0, 0) == 0 && GetTickCount() < dl) {
                Sleep(10);
            }
            if (InterlockedCompareExchange(&ui_ctx.done, 0, 0) != 1
                || InterlockedCompareExchange(&any_ctx.done, 0, 0) != 0
                || lmx_msg_exec_map_queued(rti, sib) != 0) {
                fprintf(stderr, "exec map-reparent sibling done=%ld released=%ld nready=%d\n",
                    (long)InterlockedCompareExchange(&ui_ctx.done, 0, 0),
                    (long)InterlockedCompareExchange(&any_ctx.done, 0, 0),
                    lmx_msg_exec_map_queued(rti, sib));
                lmx_msg_exec_stop(rti);
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            lmx_msg_exec_stop(rti);
            fprintf(stderr, "exec wait: failed-turn uncommitted child released; sibling keeps its ready flag\n");
            lmx_msg_runtime_delete(rti);
        }
        rti = lmx_msg_runtime_new();
        {
            /* Stage 3b-8: the family boundary contract. A bound child cannot leave its
             * family (lmx_msg_child_unlink returns INVALID and nothing moves); once
             * unbound it leaves (OK, the family links cleared). */
            LmxMsgAddr p = 0, kid = 0;
            LmxMsg *pm, *km;
            memset(&ui_ctx, 0, sizeof(ui_ctx));
            if (rti == 0 || lmx_msg_create(rti, 0, 1, &ini, 1, &p) != LMX_MSG_OK
                || lmx_msg_end_turn(rti, p, 1) != LMX_MSG_OK
                || lmx_msg_create(rti, p, 2, &ini, 1, &kid) != LMX_MSG_OK
                || lmx_msg_end_turn(rti, p, 1) != LMX_MSG_OK
                || lmx_msg_exec_bind(rti, kid, turn_recv_end, &ui_ctx, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK) {
                fprintf(stderr, "exec unlink-contract create\n");
                if (rti != 0) {
                    lmx_msg_runtime_delete(rti);
                }
                return 1;
            }
            pm = lmx_msg_find(rti, p);
            km = lmx_msg_find(rti, kid);
            if (pm == 0 || km == 0
                || lmx_msg_child_unlink(pm, km) != LMX_MSG_INVALID
                || pm->first_child != km || km->parent_msg != pm) {
                fprintf(stderr, "exec unlink-contract bound child left its family\n");
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            if (lmx_msg_exec_unbind(rti, kid) != LMX_MSG_OK
                || lmx_msg_child_unlink(pm, km) != LMX_MSG_OK
                || pm->first_child != 0 || km->parent_msg != 0) {
                fprintf(stderr, "exec unlink-contract unbound child did not leave\n");
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            fprintf(stderr, "exec wait: child_unlink refuses a bound child; the unbound child leaves\n");
            lmx_msg_runtime_delete(rti);
        }
        rti = lmx_msg_runtime_new();
        {
            LmxMsgAddr p = 0, c0 = 0, extras[16];
            int k;
            memset(&ui_ctx, 0, sizeof(ui_ctx));
            memset(extras, 0, sizeof(extras));
            if (rti == 0 || lmx_msg_create(rti, 0, 1, &ini, 1, &p) != LMX_MSG_OK
                || lmx_msg_end_turn(rti, p, 1) != LMX_MSG_OK
                || lmx_msg_create(rti, p, 2, &ini, 1, &c0) != LMX_MSG_OK) {
                fprintf(stderr, "exec wait-grow create\n");
                if (rti != 0) {
                    lmx_msg_runtime_delete(rti);
                }
                return 1;
            }
            for (k = 0; k < 16; k++) {
                if (lmx_msg_create(rti, p, (unsigned)(10 + k), &ini, 1, &extras[k]) != LMX_MSG_OK) {
                    fprintf(stderr, "exec wait-grow extra create %d\n", k);
                    lmx_msg_runtime_delete(rti);
                    return 1;
                }
            }
            if (lmx_msg_end_turn(rti, p, 1) != LMX_MSG_OK
                || lmx_msg_exec_bind(rti, c0, turn_recv_end, &ui_ctx, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK
                || lmx_msg_exec_start_contexts(rti) != LMX_MSG_OK) {
                fprintf(stderr, "exec wait-grow start\n");
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            for (k = 0; k < 16; k++) {
                if (lmx_msg_exec_bind(rti, extras[k], turn_recv_end, &any_ctx, LMX_MSG_AFFINITY_UI) != LMX_MSG_OK) {
                    fprintf(stderr, "exec wait-grow extra %d\n", k);
                    lmx_msg_exec_stop(rti);
                    lmx_msg_runtime_delete(rti);
                    return 1;
                }
            }
            if (lmx_msg_host_post(rti, c0, &env) != LMX_MSG_STAGED
                || lmx_msg_host_drain(rti) != LMX_MSG_OK) {
                fprintf(stderr, "exec wait-grow post\n");
                lmx_msg_exec_stop(rti);
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            dl = GetTickCount() + 2000;
            while (InterlockedCompareExchange(&ui_ctx.done, 0, 0) == 0 && GetTickCount() < dl) {
                Sleep(10);
            }
            if (InterlockedCompareExchange(&ui_ctx.done, 0, 0) != 1) {
                fprintf(stderr, "exec wait-grow done=%ld\n",
                    (long)InterlockedCompareExchange(&ui_ctx.done, 0, 0));
                lmx_msg_exec_stop(rti);
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            if (lmx_msg_exec_stop(rti) != LMX_MSG_OK
                || lmx_msg_exec_start_contexts(rti) != LMX_MSG_OK) {
                fprintf(stderr, "exec wait-grow restart\n");
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            lmx_msg_exec_stop(rti);
            fprintf(stderr, "exec wait: context sleeps across bind grow 8->16; exact-once after wake; stop/restart\n");
            lmx_msg_runtime_delete(rti);
        }
        rti = lmx_msg_runtime_new();
        {
            LmxMsgAddr p = 0, a = 0, b = 0, c = 0;
            TurnCtx c_ctx;
            int bound0;
            int loop;
            memset(&ui_ctx, 0, sizeof(ui_ctx));
            memset(&any_ctx, 0, sizeof(any_ctx));
            memset(&c_ctx, 0, sizeof(c_ctx));
            if (rti == 0 || lmx_msg_create(rti, 0, 1, &ini, 1, &p) != LMX_MSG_OK
                || lmx_msg_end_turn(rti, p, 1) != LMX_MSG_OK
                || lmx_msg_create(rti, p, 2, &ini, 1, &a) != LMX_MSG_OK
                || lmx_msg_create(rti, p, 3, &ini, 1, &b) != LMX_MSG_OK
                || lmx_msg_create(rti, p, 4, &ini, 1, &c) != LMX_MSG_OK
                || lmx_msg_end_turn(rti, p, 1) != LMX_MSG_OK
                || lmx_msg_exec_bind(rti, a, turn_recv_end, &ui_ctx, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK
                || lmx_msg_exec_bind(rti, b, turn_recv_end, &any_ctx, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK
                || lmx_msg_exec_bind(rti, c, turn_recv_end, &c_ctx, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK
                || lmx_msg_exec_start_contexts(rti) != LMX_MSG_OK) {
                fprintf(stderr, "exec wait-compact create\n");
                if (rti != 0) {
                    lmx_msg_runtime_delete(rti);
                }
                return 1;
            }
            if (lmx_msg_exec_unbind(rti, b) != LMX_MSG_OK) {
                fprintf(stderr, "exec wait-compact unbind mid\n");
                lmx_msg_exec_stop(rti);
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            if (lmx_msg_host_post(rti, a, &env) != LMX_MSG_STAGED
                || lmx_msg_host_post(rti, c, &env) != LMX_MSG_STAGED
                || lmx_msg_host_drain(rti) != LMX_MSG_OK) {
                fprintf(stderr, "exec wait-compact post\n");
                lmx_msg_exec_stop(rti);
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            dl = GetTickCount() + 2000;
            while ((InterlockedCompareExchange(&ui_ctx.done, 0, 0) == 0
                || InterlockedCompareExchange(&c_ctx.done, 0, 0) == 0)
                && GetTickCount() < dl) {
                Sleep(10);
            }
            if (InterlockedCompareExchange(&ui_ctx.done, 0, 0) != 1
                || InterlockedCompareExchange(&c_ctx.done, 0, 0) != 1
                || InterlockedCompareExchange(&any_ctx.done, 0, 0) != 0) {
                fprintf(stderr, "exec wait-compact a=%ld b=%ld c=%ld\n",
                    (long)InterlockedCompareExchange(&ui_ctx.done, 0, 0),
                    (long)InterlockedCompareExchange(&any_ctx.done, 0, 0),
                    (long)InterlockedCompareExchange(&c_ctx.done, 0, 0));
                lmx_msg_exec_stop(rti);
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            InterlockedExchange(&any_ctx.done, 0);
            if (lmx_msg_exec_bind(rti, b, turn_recv_end, &any_ctx, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK
                || lmx_msg_host_post(rti, b, &env) != LMX_MSG_STAGED
                || lmx_msg_host_drain(rti) != LMX_MSG_OK) {
                fprintf(stderr, "exec wait-compact rebind\n");
                lmx_msg_exec_stop(rti);
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            dl = GetTickCount() + 2000;
            while (InterlockedCompareExchange(&any_ctx.done, 0, 0) == 0 && GetTickCount() < dl) {
                Sleep(10);
            }
            if (InterlockedCompareExchange(&any_ctx.done, 0, 0) != 1) {
                fprintf(stderr, "exec wait-compact rebind done=%ld\n",
                    (long)InterlockedCompareExchange(&any_ctx.done, 0, 0));
                lmx_msg_exec_stop(rti);
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            bound0 = lmx_msg_exec_bind_n(rti);
            for (loop = 0; loop < 8; loop++) {
                InterlockedExchange(&any_ctx.done, 0);
                if (lmx_msg_exec_unbind(rti, b) != LMX_MSG_OK
                    || lmx_msg_exec_bind(rti, b, turn_recv_end, &any_ctx, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK
                    || lmx_msg_exec_bind_n(rti) != bound0) {
                    fprintf(stderr, "exec wait-compact loop %d bound=%d want=%d\n",
                        loop, lmx_msg_exec_bind_n(rti), bound0);
                    lmx_msg_exec_stop(rti);
                    lmx_msg_runtime_delete(rti);
                    return 1;
                }
            }
            lmx_msg_exec_stop(rti);
            fprintf(stderr, "exec wait: unbind same-addr rebind exact-once; slots do not accumulate\n");
            lmx_msg_runtime_delete(rti);
        }
        rti = lmx_msg_runtime_new();
        {
            LmxMsgAddr p = 0, any = 0, ui = 0;
            int w0;
            memset(&ui_ctx, 0, sizeof(ui_ctx));
            memset(&any_ctx, 0, sizeof(any_ctx));
            if (rti == 0 || lmx_msg_create(rti, 0, 1, &ini, 1, &p) != LMX_MSG_OK
                || lmx_msg_end_turn(rti, p, 1) != LMX_MSG_OK
                || lmx_msg_create(rti, p, 2, &ini, 1, &any) != LMX_MSG_OK
                || lmx_msg_create(rti, p, 3, &ini, 1, &ui) != LMX_MSG_OK
                || lmx_msg_end_turn(rti, p, 1) != LMX_MSG_OK
                || lmx_msg_exec_bind(rti, any, turn_recv_end, &any_ctx, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK
                || lmx_msg_exec_bind(rti, ui, turn_recv_end, &ui_ctx, LMX_MSG_AFFINITY_UI) != LMX_MSG_OK
                || lmx_msg_exec_start_contexts(rti) != LMX_MSG_OK) {
                fprintf(stderr, "exec ui-reap create\n");
                if (rti != 0) {
                    lmx_msg_runtime_delete(rti);
                }
                return 1;
            }
            w0 = lmx_msg_exec_workers(rti);
            if (w0 < 1 || lmx_msg_exec_unbind(rti, ui) != LMX_MSG_OK
                || lmx_msg_exec_workers(rti) != w0) {
                fprintf(stderr, "exec ui-reap workers %d -> %d\n", w0, lmx_msg_exec_workers(rti));
                lmx_msg_exec_stop(rti);
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            if (lmx_msg_host_post(rti, any, &env) != LMX_MSG_STAGED
                || lmx_msg_host_drain(rti) != LMX_MSG_OK) {
                fprintf(stderr, "exec ui-reap post\n");
                lmx_msg_exec_stop(rti);
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            dl = GetTickCount() + 2000;
            while (InterlockedCompareExchange(&any_ctx.done, 0, 0) == 0 && GetTickCount() < dl) {
                Sleep(10);
            }
            if (InterlockedCompareExchange(&any_ctx.done, 0, 0) != 1) {
                fprintf(stderr, "exec ui-reap any done=%ld\n",
                    (long)InterlockedCompareExchange(&any_ctx.done, 0, 0));
                lmx_msg_exec_stop(rti);
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            lmx_msg_exec_stop(rti);
            fprintf(stderr, "exec wait: UI unbind does not undercount live ANY workers\n");
            lmx_msg_runtime_delete(rti);
        }
        rti = lmx_msg_runtime_new();
        {
            LmxMsgAddr p = 0, a = 0;
            int phase;
            memset(&any_ctx, 0, sizeof(any_ctx));
            if (rti == 0 || lmx_msg_create(rti, 0, 1, &ini, 1, &p) != LMX_MSG_OK
                || lmx_msg_end_turn(rti, p, 1) != LMX_MSG_OK
                || lmx_msg_create(rti, p, 2, &ini, 1, &a) != LMX_MSG_OK
                || lmx_msg_end_turn(rti, p, 1) != LMX_MSG_OK
                || lmx_msg_exec_start_contexts(rti) != LMX_MSG_OK) {
                fprintf(stderr, "exec launch-gate create\n");
                if (rti != 0) {
                    lmx_msg_runtime_delete(rti);
                }
                return 1;
            }
            for (phase = 0; phase < 2; phase++) {
                InterlockedExchange(&any_ctx.done, 0);
                g_launch_unb = a;
                g_launch_phase = phase;
                lmx_msg_exec_test_during_launch = launch_unbind_hook;
                (void)lmx_msg_exec_bind(rti, a, turn_recv_end, &any_ctx, LMX_MSG_AFFINITY_ANY);
                lmx_msg_exec_test_during_launch = 0;
                g_launch_unb = 0;
                if (lmx_msg_exec_bind(rti, a, turn_recv_end, &any_ctx, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK
                    || lmx_msg_host_post(rti, a, &env) != LMX_MSG_STAGED
                    || lmx_msg_host_drain(rti) != LMX_MSG_OK) {
                    fprintf(stderr, "exec launch-gate rebind phase=%d\n", phase);
                    lmx_msg_exec_stop(rti);
                    lmx_msg_runtime_delete(rti);
                    return 1;
                }
                dl = GetTickCount() + 2000;
                while (InterlockedCompareExchange(&any_ctx.done, 0, 0) == 0 && GetTickCount() < dl) {
                    Sleep(10);
                }
                if (InterlockedCompareExchange(&any_ctx.done, 0, 0) != 1) {
                    fprintf(stderr, "exec launch-gate done phase=%d v=%ld\n", phase,
                        (long)InterlockedCompareExchange(&any_ctx.done, 0, 0));
                    lmx_msg_exec_stop(rti);
                    lmx_msg_runtime_delete(rti);
                    return 1;
                }
                if (lmx_msg_exec_unbind(rti, a) != LMX_MSG_OK) {
                    fprintf(stderr, "exec launch-gate cleanup unbind\n");
                    lmx_msg_exec_stop(rti);
                    lmx_msg_runtime_delete(rti);
                    return 1;
                }
            }
            lmx_msg_exec_stop(rti);
            fprintf(stderr, "exec wait: unbind during launch both sides; same-addr rebind exact-once\n");
            lmx_msg_runtime_delete(rti);
        }
        rti = lmx_msg_runtime_new();
        {
            LmxMsgAddr p = 0, a = 0;
            int w0;
            memset(&any_ctx, 0, sizeof(any_ctx));
            memset(&g_stale_g2, 0, sizeof(g_stale_g2));
            if (rti == 0 || lmx_msg_create(rti, 0, 1, &ini, 1, &p) != LMX_MSG_OK
                || lmx_msg_end_turn(rti, p, 1) != LMX_MSG_OK
                || lmx_msg_create(rti, p, 2, &ini, 1, &a) != LMX_MSG_OK
                || lmx_msg_end_turn(rti, p, 1) != LMX_MSG_OK
                || lmx_msg_exec_start_contexts(rti) != LMX_MSG_OK) {
                fprintf(stderr, "exec stale-launch create\n");
                if (rti != 0) {
                    lmx_msg_runtime_delete(rti);
                }
                return 1;
            }
            g_launch_unb = a;
            g_stale_gen = 0U;
            g_stale_old_gen = 0U;
            g_stale_wh = 0;
            g_stale_launching = -1;
            g_stale_workers = 0;
            g_stale_alive = 0;
            g_stale_launch_n = 0;
            g_stale_destroy_at_unbind = 0U;
            g_stale_want_ui = 0;
            lmx_msg_exec_test_during_launch = stale_launch_hook;
            (void)lmx_msg_exec_bind(rti, a, turn_just_end, &any_ctx, LMX_MSG_AFFINITY_ANY);
            lmx_msg_exec_test_during_launch = 0;
            g_launch_unb = 0;
            w0 = lmx_msg_exec_workers(rti);
            if (w0 != 1 || w0 != g_stale_workers
                || g_stale_gen == 0U || g_stale_wh == 0
                || g_stale_alive == 0 || g_stale_launch_n < 1
                || lmx_msg_exec_test_wait_destroy_n() != g_stale_destroy_at_unbind + 1U
                || lmx_msg_exec_test_wait_destroy_last_gen() != g_stale_old_gen
                || lmx_msg_exec_test_wait_gen(rti, a) != g_stale_gen
                || lmx_msg_exec_test_worker_handle(rti, a) != g_stale_wh
                || lmx_msg_exec_test_launching(rti, a) != 0
                || g_stale_launching != 0
                || InterlockedCompareExchange(&any_ctx.done, 0, 0) != 0) {
                fprintf(stderr, "exec stale-launch workers=%d hookw=%d gen=%u alive=%d ln=%d dn=%u last=%u launching=%d olddone=%ld\n",
                    w0, g_stale_workers, g_stale_gen, g_stale_alive, g_stale_launch_n,
                    lmx_msg_exec_test_wait_destroy_n(), lmx_msg_exec_test_wait_destroy_last_gen(),
                    lmx_msg_exec_test_launching(rti, a),
                    (long)InterlockedCompareExchange(&any_ctx.done, 0, 0));
                lmx_msg_exec_stop(rti);
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            if (lmx_msg_host_post(rti, a, &env) != LMX_MSG_STAGED
                || lmx_msg_host_drain(rti) != LMX_MSG_OK) {
                fprintf(stderr, "exec stale-launch post\n");
                lmx_msg_exec_stop(rti);
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            dl = GetTickCount() + 2000;
            while (InterlockedCompareExchange(&g_stale_g2.done, 0, 0) == 0 && GetTickCount() < dl) {
                Sleep(10);
            }
            if (InterlockedCompareExchange(&g_stale_g2.done, 0, 0) != 1
                || InterlockedCompareExchange(&any_ctx.done, 0, 0) != 0
                || lmx_msg_exec_workers(rti) != 1
                || lmx_msg_exec_test_wait_gen(rti, a) != g_stale_gen
                || lmx_msg_exec_test_worker_handle(rti, a) != g_stale_wh) {
                fprintf(stderr, "exec stale-launch g2=%ld old=%ld workers=%d\n",
                    (long)InterlockedCompareExchange(&g_stale_g2.done, 0, 0),
                    (long)InterlockedCompareExchange(&any_ctx.done, 0, 0),
                    lmx_msg_exec_workers(rti));
                lmx_msg_exec_stop(rti);
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            lmx_msg_exec_stop(rti);
            if (lmx_msg_exec_workers(rti) != 0) {
                fprintf(stderr, "exec stale-launch stop residue workers=%d\n", lmx_msg_exec_workers(rti));
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            fprintf(stderr, "exec wait: stale launcher aborts; G2 exact-once; worker count exact\n");
            lmx_msg_runtime_delete(rti);
        }
        rti = lmx_msg_runtime_new();
        {
            LmxMsgAddr p = 0, a = 0;
            unsigned dn0;
            memset(&any_ctx, 0, sizeof(any_ctx));
            memset(&g_stale_g2, 0, sizeof(g_stale_g2));
            if (rti == 0 || lmx_msg_create(rti, 0, 1, &ini, 1, &p) != LMX_MSG_OK
                || lmx_msg_end_turn(rti, p, 1) != LMX_MSG_OK
                || lmx_msg_create(rti, p, 2, &ini, 1, &a) != LMX_MSG_OK
                || lmx_msg_end_turn(rti, p, 1) != LMX_MSG_OK
                || lmx_msg_exec_start_contexts(rti) != LMX_MSG_OK) {
                fprintf(stderr, "exec stale-launch ui-g2 create\n");
                if (rti != 0) {
                    lmx_msg_runtime_delete(rti);
                }
                return 1;
            }
            g_launch_unb = a;
            g_stale_gen = 0U;
            g_stale_old_gen = 0U;
            g_stale_wh = (void *)1;
            g_stale_launching = -1;
            g_stale_workers = -1;
            g_stale_alive = 0;
            g_stale_launch_n = 0;
            g_stale_destroy_at_unbind = 0U;
            g_stale_want_ui = 1;
            lmx_msg_exec_test_during_launch = stale_launch_hook;
            (void)lmx_msg_exec_bind(rti, a, turn_just_end, &any_ctx, LMX_MSG_AFFINITY_ANY);
            lmx_msg_exec_test_during_launch = 0;
            g_launch_unb = 0;
            g_stale_want_ui = 0;
            dn0 = g_stale_destroy_at_unbind;
            if (g_stale_alive == 0 || g_stale_launch_n < 1
                || lmx_msg_exec_test_wait_destroy_n() != dn0 + 1U
                || lmx_msg_exec_test_wait_destroy_last_gen() != g_stale_old_gen
                || lmx_msg_exec_is_bound(rti, a) == 0
                || lmx_msg_exec_bind_aff(rti, a) != LMX_MSG_AFFINITY_UI
                || lmx_msg_exec_test_launching(rti, a) != 0
                || g_stale_launching != 0
                || lmx_msg_exec_bind_has_worker(rti, a) != 0
                || g_stale_wh != 0
                || lmx_msg_exec_workers(rti) != 0
                || lmx_msg_exec_test_wait_gen(rti, a) == 0U
                || lmx_msg_exec_test_wait_gen(rti, a) == g_stale_old_gen
                || InterlockedCompareExchange(&any_ctx.done, 0, 0) != 0
                || InterlockedCompareExchange(&g_stale_g2.done, 0, 0) != 0) {
                fprintf(stderr, "exec stale-launch ui-g2 alive=%d ln=%d dn=%u last=%u aff=%d launch=%d wr=%d w=%d gen=%u old=%u\n",
                    g_stale_alive, g_stale_launch_n, lmx_msg_exec_test_wait_destroy_n(),
                    lmx_msg_exec_test_wait_destroy_last_gen(), lmx_msg_exec_bind_aff(rti, a),
                    lmx_msg_exec_test_launching(rti, a), lmx_msg_exec_bind_has_worker(rti, a),
                    lmx_msg_exec_workers(rti), lmx_msg_exec_test_wait_gen(rti, a), g_stale_old_gen);
                lmx_msg_exec_stop(rti);
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            lmx_msg_exec_stop(rti);
            if (lmx_msg_exec_workers(rti) != 0) {
                fprintf(stderr, "exec stale-launch ui-g2 stop residue workers=%d\n", lmx_msg_exec_workers(rti));
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            fprintf(stderr, "exec wait: stale launcher keeps G wait until release; UI G2 untouched\n");
            lmx_msg_runtime_delete(rti);
        }
        /* Retired with the bind/unbind authority (19.28.R2.2 (2) with 19.29.6): the reap-kept
         * race case unbound A from a turn-less thread while the host sat inside its own bind's
         * launch, and the reap it guarded ran only in that thread's unbind (bind_reap_join_all
         * joins when the caller holds no turn). Only the host outside any turn, here the one
         * blocked in bind, and the parent's lane, which runs with a turn and never takes the
         * join branch, may unbind now, so that interleaving is unreachable; the committed
         * self and sibling refusal cases and the authority line back the claim. */
        rti = lmx_msg_runtime_new();
        {
            LmxMsgAddr p = 0, a = 0;
            LmxMsg *cm;
            memset(&any_ctx, 0, sizeof(any_ctx));
            if (rti == 0 || lmx_msg_create(rti, 0, 1, &ini, 1, &p) != LMX_MSG_OK
                || lmx_msg_end_turn(rti, p, 1) != LMX_MSG_OK
                || lmx_msg_create(rti, p, 2, &ini, 1, &a) != LMX_MSG_OK
                || lmx_msg_end_turn(rti, p, 1) != LMX_MSG_OK
                || lmx_msg_exec_bind(rti, a, turn_recv_end, &any_ctx, LMX_MSG_AFFINITY_UI) != LMX_MSG_OK) {
                fprintf(stderr, "exec map-child ui create\n");
                if (rti != 0) {
                    lmx_msg_runtime_delete(rti);
                }
                return 1;
            }
            cm = lmx_msg_find(rti, a);
            if (cm == 0 || lmx_msg_map_child(rti, p, a) != LMX_MSG_INVALID || cm->mapped != 0) {
                fprintf(stderr, "exec map-child ui mapped=%d\n", cm != 0 ? cm->mapped : -1);
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            fprintf(stderr, "exec wait: map_child UI is INVALID and leaves mapped=0\n");
            lmx_msg_runtime_delete(rti);
        }
        rti = lmx_msg_runtime_new();
        {
            LmxMsgAddr p = 0, a = 0;
            memset(&any_ctx, 0, sizeof(any_ctx));
            memset(&g_self_new, 0, sizeof(g_self_new));
            g_self_unbind_st = -1;
            g_self_bind_st = -1;
            g_self_aff_after = -1;
            g_self_bound_after = -1;
            if (rti == 0 || lmx_msg_create(rti, 0, 1, &ini, 1, &p) != LMX_MSG_OK
                || lmx_msg_end_turn(rti, p, 1) != LMX_MSG_OK
                || lmx_msg_create(rti, p, 2, &ini, 1, &a) != LMX_MSG_OK
                || lmx_msg_end_turn(rti, p, 1) != LMX_MSG_OK
                || lmx_msg_exec_bind(rti, a, turn_self_rebind, &any_ctx, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK
                || lmx_msg_host_post(rti, a, &env) != LMX_MSG_STAGED
                || lmx_msg_host_drain(rti) != LMX_MSG_OK
                || lmx_msg_exec_start_contexts(rti) != LMX_MSG_OK) {
                fprintf(stderr, "exec self-rebind create\n");
                if (rti != 0) {
                    lmx_msg_runtime_delete(rti);
                }
                return 1;
            }
            dl = GetTickCount() + 2000;
            while (InterlockedCompareExchange(&any_ctx.done, 0, 0) == 0 && GetTickCount() < dl) {
                Sleep(10);
            }
            if (InterlockedCompareExchange(&any_ctx.done, 0, 0) != 1) {
                fprintf(stderr, "exec self-rebind old=%ld\n",
                    (long)InterlockedCompareExchange(&any_ctx.done, 0, 0));
                lmx_msg_exec_stop(rti);
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            if (g_self_unbind_st != LMX_MSG_INVALID || g_self_bind_st != LMX_MSG_INVALID
                || g_self_aff_after != LMX_MSG_AFFINITY_ANY || g_self_bound_after != 1) {
                fprintf(stderr, "exec self-rebind refusals unbind=%d bind=%d aff=%d bound=%d\n",
                    g_self_unbind_st, g_self_bind_st, g_self_aff_after, g_self_bound_after);
                lmx_msg_exec_stop(rti);
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            /* The parent's authority (the host outside any turn) rebinds A. */
            if (lmx_msg_exec_unbind(rti, a) != LMX_MSG_OK
                || lmx_msg_exec_bind(rti, a, turn_recv_end, &g_self_new, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK) {
                fprintf(stderr, "exec self-rebind host rebind\n");
                lmx_msg_exec_stop(rti);
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            if (lmx_msg_host_post(rti, a, &env) != LMX_MSG_STAGED
                || lmx_msg_host_drain(rti) != LMX_MSG_OK) {
                fprintf(stderr, "exec self-rebind post\n");
                lmx_msg_exec_stop(rti);
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            dl = GetTickCount() + 2000;
            while (InterlockedCompareExchange(&g_self_new.done, 0, 0) == 0 && GetTickCount() < dl) {
                Sleep(10);
            }
            if (InterlockedCompareExchange(&g_self_new.done, 0, 0) != 1) {
                fprintf(stderr, "exec self-rebind new=%ld\n",
                    (long)InterlockedCompareExchange(&g_self_new.done, 0, 0));
                lmx_msg_exec_stop(rti);
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            lmx_msg_exec_stop(rti);
            fprintf(stderr, "exec wait: self-unbind and self-rebind refused in the turn; host rebind then new gen exact-once\n");
            lmx_msg_runtime_delete(rti);
        }
        rti = lmx_msg_runtime_new();
        {
            LmxMsgAddr p = 0, a = 0, b = 0;
            SiblingMapRec sib;
            int ui0;
            int st;
            memset(&any_ctx, 0, sizeof(any_ctx));
            memset(&sib, 0, sizeof(sib));
            sib.bind_st = -1;
            sib.unbind_st = -1;
            if (rti == 0 || lmx_msg_create(rti, 0, 1, &ini, 1, &p) != LMX_MSG_OK
                || lmx_msg_end_turn(rti, p, 1) != LMX_MSG_OK
                || lmx_msg_create(rti, p, 2, &ini, 1, &a) != LMX_MSG_OK
                || lmx_msg_create(rti, p, 3, &ini, 1, &b) != LMX_MSG_OK
                || lmx_msg_end_turn(rti, p, 1) != LMX_MSG_OK) {
                fprintf(stderr, "exec sibling-map create\n");
                if (rti != 0) {
                    lmx_msg_runtime_delete(rti);
                }
                return 1;
            }
            sib.other = b;
            if (lmx_msg_exec_bind(rti, a, turn_map_sibling, &sib, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK
                || lmx_msg_exec_bind(rti, b, turn_recv_end, &any_ctx, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK
                || lmx_msg_host_post(rti, a, &env) != LMX_MSG_STAGED
                || lmx_msg_host_drain(rti) != LMX_MSG_OK) {
                fprintf(stderr, "exec sibling-map bind\n");
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            ui0 = lmx_msg_exec_ui_nrequests(rti);
            st = lmx_msg_run_child_turn(rti, a);
            if ((st != LMX_MSG_OK && st != 1) || sib.bind_st != LMX_MSG_INVALID || sib.unbind_st != LMX_MSG_INVALID
                || lmx_msg_exec_bind_aff(rti, b) != LMX_MSG_AFFINITY_ANY || lmx_msg_exec_is_bound(rti, b) != 1
                || lmx_msg_exec_ui_nrequests(rti) != ui0) {
                fprintf(stderr, "exec sibling-map turn=%d bind=%d unbind=%d aff=%d bound=%d ui=%d/%d\n",
                    st, sib.bind_st, sib.unbind_st, lmx_msg_exec_bind_aff(rti, b), lmx_msg_exec_is_bound(rti, b),
                    lmx_msg_exec_ui_nrequests(rti), ui0);
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            fprintf(stderr, "exec wait: a host-thread child's turn cannot rebind or unbind its sibling; binding and affinity intact, no UI request\n");
            lmx_msg_runtime_delete(rti);
        }
        rti = lmx_msg_runtime_new();
        {
            LmxMsgAddr p = 0, c = 0, g = 0;
            DisposeInTurnRec dz;
            int st;
            int n0;
            memset(&dz, 0, sizeof(dz));
            dz.st = -1;
            if (rti == 0 || lmx_msg_create(rti, 0, 1, &ini, 1, &p) != LMX_MSG_OK
                || lmx_msg_end_turn(rti, p, 1) != LMX_MSG_OK
                || lmx_msg_create(rti, p, 2, &ini, 1, &c) != LMX_MSG_OK
                || lmx_msg_end_turn(rti, p, 1) != LMX_MSG_OK
                || lmx_msg_create(rti, c, 3, &ini, 1, &g) != LMX_MSG_OK
                || lmx_msg_end_turn(rti, c, 1) != LMX_MSG_OK
                || lmx_msg_exec_bind(rti, c, turn_just_end, &any_ctx, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK
                || lmx_msg_exec_bind(rti, g, turn_just_end, &any_ctx, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK) {
                fprintf(stderr, "exec dispose-in-turn create\n");
                if (rti != 0) {
                    lmx_msg_runtime_delete(rti);
                }
                return 1;
            }
            if (lmx_msg_complete(rti, g) != LMX_MSG_OK
                || ((st = lmx_msg_run_child_turn(rti, g)) != LMX_MSG_OK && st != 1)
                || lmx_msg_handoff_ready(rti, g) == 0
                || lmx_msg_complete(rti, c) != LMX_MSG_OK
                || ((st = lmx_msg_run_child_turn(rti, c)) != LMX_MSG_OK && st != 1)
                || lmx_msg_handoff_ready(rti, c) == 0
                || lmx_msg_find(rti, g) == 0 || lmx_msg_exec_is_bound(rti, g) != 1) {
                fprintf(stderr, "exec dispose-in-turn settle g_ready=%d c_ready=%d g_bound=%d\n",
                    lmx_msg_handoff_ready(rti, g), lmx_msg_handoff_ready(rti, c), lmx_msg_exec_is_bound(rti, g));
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            dz.child = c;
            n0 = rti->n;
            if (lmx_msg_exec_bind(rti, p, turn_dispose_settled, &dz, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK) {
                fprintf(stderr, "exec dispose-in-turn bind p\n");
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            st = lmx_msg_run_child_turn(rti, p);
            if ((st != LMX_MSG_OK && st != 1) || dz.st != LMX_MSG_OK
                || lmx_msg_find(rti, g) != 0 || lmx_msg_find(rti, c) != 0
                || lmx_msg_child_n(rti, p) != 0 || rti->n != n0 - 2) {
                fprintf(stderr, "exec dispose-in-turn turn=%d dispose=%d g=%p c=%p kids=%d n=%d/%d\n",
                    st, dz.st, (void *)lmx_msg_find(rti, g), (void *)lmx_msg_find(rti, c),
                    lmx_msg_child_n(rti, p), rti->n, n0);
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            fprintf(stderr, "exec wait: P's turn disposes C with G settled under it; G unbound on P's lane, both slots gone\n");
            lmx_msg_runtime_delete(rti);
        }
        rti = lmx_msg_runtime_new();
        {
            /* Stage 5 (b): recv never hands out an internal kind. */
            LmxMsgAddr r = 0, c = 0;
            LmxMsgEnv ing;
            LmxMsgEnv sent;
            LmxMsgEnv got;
            int st;
            /* Stage 5 (d1): the ingress root is R0, so r is R0 and c its child. */
            if (rti == 0 || (r = lmx_msg_root_addr(rti)) == 0U
                || lmx_msg_create(rti, r, 2, &ini, 1, &c) != LMX_MSG_OK
                || lmx_msg_end_turn(rti, r, 1) != LMX_MSG_OK) {
                fprintf(stderr, "exec ingress recv create\n");
                if (rti != 0) {
                    lmx_msg_runtime_delete(rti);
                }
                return 1;
            }
            memset(&ing, 0, sizeof(ing));
            ing.kind = LMX_MSG_KIND_NUMBER;
            ing.number = 7;
            if (lmx_msg_host_post(rti, c, &ing) != LMX_MSG_STAGED || lmx_msg_inbox_n(rti, r) != 1 || lmx_msg_inbox_n(rti, c) != 0) {
                fprintf(stderr, "exec ingress recv post r=%d c=%d\n", lmx_msg_inbox_n(rti, r), lmx_msg_inbox_n(rti, c));
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            memset(&sent, 0, sizeof(sent));
            sent.kind = LMX_MSG_KIND_NUMBER;
            sent.number = 41;
            if (lmx_msg_send(rti, c, r, &sent) != LMX_MSG_STAGED || lmx_msg_end_turn(rti, c, 1) != LMX_MSG_OK
                || lmx_msg_inbox_n(rti, r) != 2) {
                fprintf(stderr, "exec ingress recv behind r=%d\n", lmx_msg_inbox_n(rti, r));
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            memset(&got, 0, sizeof(got));
            st = lmx_msg_recv(rti, r, &got);
            if (st != LMX_MSG_OK || got.kind != LMX_MSG_KIND_NUMBER || got.number != 41 || lmx_msg_inbox_n(rti, r) != 1) {
                fprintf(stderr, "exec ingress recv skip st=%d kind=%d number=%d r=%d\n", st, got.kind, got.number, lmx_msg_inbox_n(rti, r));
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            lmx_msg_env_release(&got);
            memset(&got, 0, sizeof(got));
            st = lmx_msg_recv(rti, r, &got);
            if (st != LMX_MSG_EMPTY || lmx_msg_inbox_n(rti, r) != 1) {
                fprintf(stderr, "exec ingress recv alone st=%d kind=%d r=%d\n", st, got.kind, lmx_msg_inbox_n(rti, r));
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            if (lmx_msg_end_turn(rti, r, 1) != LMX_MSG_OK || lmx_msg_host_drain(rti) != LMX_MSG_OK
                || lmx_msg_inbox_n(rti, r) != 0 || lmx_msg_inbox_n(rti, c) != 1) {
                fprintf(stderr, "exec ingress recv drain r=%d c=%d\n", lmx_msg_inbox_n(rti, r), lmx_msg_inbox_n(rti, c));
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            fprintf(stderr, "exec wait: recv skips a pending INGRESS for the input behind it and gives EMPTY when only INGRESS is left; the drain forwards it\n");
            lmx_msg_runtime_delete(rti);
        }
        rti = lmx_msg_runtime_new();
        {
            /* Stage 5 (b): readiness ignores internal kinds. */
            LmxMsgAddr r = 0, c = 0;
            LmxMsgEnv ing;
            DWORD until;
            /* Stage 5 (d1): the ingress root is R0, so r is R0 and c its child. */
            if (rti == 0 || (r = lmx_msg_root_addr(rti)) == 0U
                || lmx_msg_create(rti, r, 2, &ini, 1, &c) != LMX_MSG_OK
                || lmx_msg_end_turn(rti, r, 1) != LMX_MSG_OK) {
                fprintf(stderr, "exec ingress ready create\n");
                if (rti != 0) {
                    lmx_msg_runtime_delete(rti);
                }
                return 1;
            }
            memset(&ing, 0, sizeof(ing));
            ing.kind = LMX_MSG_KIND_NUMBER;
            ing.number = 7;
            if (lmx_msg_host_post(rti, c, &ing) != LMX_MSG_STAGED || lmx_msg_inbox_n(rti, r) != 1 || lmx_msg_inbox_n(rti, c) != 0) {
                fprintf(stderr, "exec ingress ready post r=%d c=%d\n", lmx_msg_inbox_n(rti, r), lmx_msg_inbox_n(rti, c));
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            InterlockedExchange(&g_ingress_root_turns, 0);
            if (lmx_msg_exec_is_runnable(rti, r) != 0
                || lmx_msg_exec_bind(rti, r, turn_count_root, 0, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK
                || lmx_msg_exec_start_contexts(rti) != LMX_MSG_OK) {
                fprintf(stderr, "exec ingress ready runnable=%d\n", lmx_msg_exec_is_runnable(rti, r));
                lmx_msg_exec_stop(rti);
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            until = GetTickCount() + 200;
            while (GetTickCount() < until) {
                Sleep(10);
            }
            if (InterlockedCompareExchange(&g_ingress_root_turns, 0, 0) != 0 || lmx_msg_inbox_n(rti, r) != 1) {
                fprintf(stderr, "exec ingress ready turns=%ld r=%d\n", (long)InterlockedCompareExchange(&g_ingress_root_turns, 0, 0), lmx_msg_inbox_n(rti, r));
                lmx_msg_exec_stop(rti);
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            lmx_msg_exec_stop(rti);
            fprintf(stderr, "exec wait: a root holding only INGRESS is not runnable and a started context gives it no turn\n");
            lmx_msg_runtime_delete(rti);
        }
        rti = lmx_msg_runtime_new();
        {
            /* Stage 5 (b): lifecycle reads count internal kinds. */
            LmxMsgAddr r = 0, c = 0;
            LmxMsgEnv ing;
            /* Stage 5 (d1): the ingress root is R0, so r is R0 and c its child. */
            if (rti == 0 || (r = lmx_msg_root_addr(rti)) == 0U
                || lmx_msg_create(rti, r, 2, &ini, 1, &c) != LMX_MSG_OK
                || lmx_msg_end_turn(rti, r, 1) != LMX_MSG_OK) {
                fprintf(stderr, "exec ingress close create\n");
                if (rti != 0) {
                    lmx_msg_runtime_delete(rti);
                }
                return 1;
            }
            memset(&ing, 0, sizeof(ing));
            ing.kind = LMX_MSG_KIND_NUMBER;
            ing.number = 7;
            if (lmx_msg_host_post(rti, c, &ing) != LMX_MSG_STAGED || lmx_msg_inbox_n(rti, r) != 1 || lmx_msg_inbox_n(rti, c) != 0) {
                fprintf(stderr, "exec ingress close post r=%d c=%d\n", lmx_msg_inbox_n(rti, r), lmx_msg_inbox_n(rti, c));
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            if (lmx_msg_emergency_cancel(rti, r) != LMX_MSG_OK || lmx_msg_drive(rti, 0U, 0U) != LMX_MSG_OK
                || lmx_msg_state(rti, r) == LMX_MSG_STATE_STOPPED || lmx_msg_inbox_n(rti, r) != 1) {
                fprintf(stderr, "exec ingress close-over state=%d r=%d\n", lmx_msg_state(rti, r), lmx_msg_inbox_n(rti, r));
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            if (lmx_msg_host_drain(rti) != LMX_MSG_OK || lmx_msg_inbox_n(rti, r) != 0 || lmx_msg_inbox_n(rti, c) != 1
                || lmx_msg_drive(rti, 0U, 0U) != LMX_MSG_OK || lmx_msg_state(rti, r) != LMX_MSG_STATE_STOPPED) {
                fprintf(stderr, "exec ingress close-after state=%d r=%d c=%d\n", lmx_msg_state(rti, r), lmx_msg_inbox_n(rti, r), lmx_msg_inbox_n(rti, c));
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            fprintf(stderr, "exec wait: drive does not close a closing root over undrained INGRESS; after the drain it does\n");
            lmx_msg_runtime_delete(rti);
        }
        rti = lmx_msg_runtime_new();
        {
            /* Stage 5 (c): a successful orphan settled on the host path waits for the
             * root's next maintenance; drive from inside a turn refuses and reclaims
             * nothing; outside any turn it reclaims the orphan (its parent's slot went
             * at the release). */
            LmxMsgAddr r = 0, p = 0, c = 0;
            DriveInTurnRec dv;
            int st;
            int n0;
            memset(&dv, 0, sizeof(dv));
            dv.st = -1;
            if (rti == 0 || lmx_msg_create(rti, 0, 1, &ini, 1, &r) != LMX_MSG_OK
                || lmx_msg_create(rti, r, 2, &ini, 1, &p) != LMX_MSG_OK
                || lmx_msg_end_turn(rti, r, 1) != LMX_MSG_OK
                || lmx_msg_create(rti, p, 3, &ini, 1, &c) != LMX_MSG_OK
                || lmx_msg_end_turn(rti, p, 1) != LMX_MSG_OK
                || lmx_msg_exec_bind(rti, p, turn_just_end, &any_ctx, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK
                || lmx_msg_exec_bind(rti, c, turn_just_end, &any_ctx, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK) {
                fprintf(stderr, "exec maintain create\n");
                if (rti != 0) {
                    lmx_msg_runtime_delete(rti);
                }
                return 1;
            }
            if (lmx_msg_emergency_cancel(rti, p) != LMX_MSG_OK
                || ((st = lmx_msg_run_child_turn(rti, p)) != LMX_MSG_OK && st != 1)) {
                fprintf(stderr, "exec maintain stop p\n");
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            n0 = rti->n;
            if (lmx_msg_dispose_child(rti, r, p) != LMX_MSG_OK || lmx_msg_complete(rti, c) != LMX_MSG_OK
                || ((st = lmx_msg_run_child_turn(rti, c)) != LMX_MSG_OK && st != 1)
                || lmx_msg_find(rti, c) == 0 || lmx_msg_find(rti, p) != 0 || rti->n != n0 - 1) {
                fprintf(stderr, "exec maintain orphan end-turn c=%p p=%p n=%d/%d\n", (void *)lmx_msg_find(rti, c), (void *)lmx_msg_find(rti, p), rti->n, n0);
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            dv.now = lmx_msg_now(rti);
            if (lmx_msg_exec_bind(rti, r, turn_drive_in_turn, &dv, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK) {
                fprintf(stderr, "exec maintain bind r\n");
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            st = lmx_msg_run_child_turn(rti, r);
            if ((st != LMX_MSG_OK && st != 1) || dv.st != LMX_MSG_INVALID || lmx_msg_find(rti, c) == 0 || rti->n != n0 - 1) {
                fprintf(stderr, "exec maintain in-turn turn=%d drive=%d c=%p n=%d/%d\n", st, dv.st, (void *)lmx_msg_find(rti, c), rti->n, n0);
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            if (lmx_msg_drive(rti, dv.now, 0U) != LMX_MSG_OK || lmx_msg_find(rti, c) != 0 || rti->n != n0 - 2) {
                fprintf(stderr, "exec maintain outside c=%p n=%d/%d\n", (void *)lmx_msg_find(rti, c), rti->n, n0);
                lmx_msg_runtime_delete(rti);
                return 1;
            }
            fprintf(stderr, "exec wait: a turn's drive refuses and the settled successful orphan waits; the next maintenance outside any turn reclaims it\n");
            lmx_msg_runtime_delete(rti);
        }
        rti = lmx_msg_runtime_new();
        if (rti == 0 || lmx_msg_exec_start_contexts(rti) != LMX_MSG_OK) {
            fprintf(stderr, "exec wait idle start\n");
            if (rti != 0) {
                lmx_msg_runtime_delete(rti);
            }
            return 1;
        }
        tstop = GetTickCount();
        if (lmx_msg_exec_stop(rti) != LMX_MSG_OK) {
            fprintf(stderr, "exec wait idle stop\n");
            lmx_msg_runtime_delete(rti);
            return 1;
        }
        tstop = GetTickCount() - tstop;
        if (tstop > 500) {
            fprintf(stderr, "exec wait idle stop slow ms=%lu\n", (unsigned long)tstop);
            lmx_msg_runtime_delete(rti);
            return 1;
        }
        fprintf(stderr, "exec wait: stop while idle joined ms=%lu\n", (unsigned long)tstop);
        lmx_msg_runtime_delete(rti);
        rti = lmx_msg_runtime_new();
        dummy = 0;
        memset(&any_ctx, 0, sizeof(any_ctx));
        if (rti == 0 || lmx_msg_exec_start_contexts(rti) != LMX_MSG_OK
            || lmx_msg_create(rti, 0, 1, &ini, 1, &dummy) != LMX_MSG_OK
            || lmx_msg_exec_bind(rti, dummy, turn_just_end, &any_ctx, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK
            || lmx_msg_host_post(rti, dummy, &env) != LMX_MSG_STAGED) {
            fprintf(stderr, "exec wait boundary create\n");
            if (rti != 0) {
                lmx_msg_exec_stop(rti);
                lmx_msg_runtime_delete(rti);
            }
            return 1;
        }
        Sleep(30);
        (void)lmx_msg_host_drain(rti);
        lmx_msg_exec_ready(rti, dummy);
        dl = GetTickCount() + 2000;
        while (InterlockedCompareExchange(&any_ctx.done, 0, 0) == 0 && GetTickCount() < dl) {
            Sleep(10);
        }
        if (InterlockedCompareExchange(&any_ctx.done, 0, 0) < 1) {
            fprintf(stderr, "exec wait boundary lost wake\n");
            lmx_msg_exec_stop(rti);
            lmx_msg_runtime_delete(rti);
            return 1;
        }
        lmx_msg_exec_stop(rti);
        fprintf(stderr, "exec wait: ready arrival at idle wait boundary delivered\n");
        lmx_msg_runtime_delete(rti);
    }
    ev = fopen("build/l1trans/logs/gen2/lmx_message_exec_selftest.evidence.txt", "w");
    if (ev) {
        fprintf(ev, "slow t0=%lu t1=%lu recvd=%u\n", (unsigned long)slow.t0, (unsigned long)slow.t1, slow.recvd);
        fprintf(ev, "fast t2=%lu recvd=%u fifo=%u,%u send_ui=%d send_peer=%d peer_got=%u\n",
            (unsigned long)fast.t2, fast.recvd, fast.fifo[0], fast.fifo[1],
            fast.send_ui_st, fast.send_peer_st, peerrec.got);
        fprintf(ev, "ui_step_ms=%lu cpu_busy_ui_ms=%lu mass_complete=70 fail_fifo=31,32 err_after=1 omit_end=1 ui_from_worker=%u peer=42\n",
            (unsigned long)tui, (unsigned long)tbusy_ui, slow.ui_recvd);
        fprintf(ev, "live_cascade=%d factory_n=%d factory_create_phase_n=%d held_child_meta=1 ctx_overlap=%d ctx_restart=%d ctx_child=%d ctx_rebind_ui=%d ctx_fail_retry=%d ctx_busy_ui=%d ctx_rebind_auth=%d ctx_ui_any_rb=%d ctx_mid_unroll=%d ctx_spawn_race=%d ctx_mix_map=%d ctx_map_fail=%d ctx_child_timer=%d ctx_real_clock=%d ctx_bind_rollback=%d ctx_bind_held=%d ctx_bind_ctx=%d ctx_idle_rollback=%d ctx_cancel_idle=%d ctx_complete_idle=%d m0_acc=%d m0_overlap=%d m0_admit=%u,%u m0_apply=%u,%u\n",
            g_live_cascade, g_factory_n, g_factory_n_at_meta,
            g_ctx_overlap, g_ctx_restart, g_ctx_child, g_ctx_rebind_ui, g_ctx_fail_retry, g_ctx_busy_ui,
            g_ctx_rebind_auth, g_ctx_ui_any_rb, g_ctx_mid_unroll, g_ctx_spawn_race, g_ctx_mix_map, g_ctx_map_fail, g_ctx_child_timer, g_ctx_real_clock, g_ctx_bind_rollback, g_ctx_bind_held, g_ctx_bind_ctx, g_ctx_idle_rollback, g_ctx_cancel_idle, g_ctx_complete_idle,
            g_m0_acc, g_m0_overlap, g_m0_admit0, g_m0_admit1, g_m0_apply0, g_m0_apply1);
        fclose(ev);
    }
    printf("lmx_message_exec ok fifo=%u,%u mass=70 fail=31,32 err_after=1 omit_end=1 xsend_ui=%d xsend_peer=%d peer=%u ui_from_worker=%u ui_ms=%lu cpu_busy_ui_ms=%lu live_cascade=%d factory_n=%d factory_create_phase_n=%d ctx_overlap=%d restart=%d child=%d rebind_ui=%d fail_retry=%d busy_ui=%d rebind_auth=%d ui_any_rb=%d mid_unroll=%d spawn_race=%d mix_map=%d map_fail=%d child_timer=%d real_clock=%d bind_rollback=%d bind_held=%d bind_ctx=%d idle_rollback=%d cancel_idle=%d complete_idle=%d\n",
        fast.fifo[0], fast.fifo[1], fast.send_ui_st, fast.send_peer_st, peerrec.got,
        slow.ui_recvd, (unsigned long)tui, (unsigned long)tbusy_ui, g_live_cascade, g_factory_n,
        g_factory_n_at_meta,
        g_ctx_overlap, g_ctx_restart, g_ctx_child, g_ctx_rebind_ui, g_ctx_fail_retry, g_ctx_busy_ui,
        g_ctx_rebind_auth, g_ctx_ui_any_rb, g_ctx_mid_unroll, g_ctx_spawn_race, g_ctx_mix_map, g_ctx_map_fail, g_ctx_child_timer, g_ctx_real_clock, g_ctx_bind_rollback, g_ctx_bind_held, g_ctx_bind_ctx, g_ctx_idle_rollback, g_ctx_cancel_idle, g_ctx_complete_idle);
    return 0;
}
