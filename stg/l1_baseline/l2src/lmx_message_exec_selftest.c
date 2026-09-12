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
static int g_oom_n;
static int g_oom_hits;
static int g_oom_scan;
static unsigned g_oom_fifo_a;
static unsigned g_oom_fifo_b;
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
    int force_oom_cleanup = argc == 2 && strcmp(argv[1], "--oom-cleanup-failure") == 0;
    int oom_only = force_oom_cleanup || (argc == 2 && strcmp(argv[1], "--oom-only") == 0);
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

    if (argc != 1 && !oom_only) {
        fprintf(stderr, "usage: exec_selftest [--oom-only|--oom-cleanup-failure]\n");
        return 2;
    }
    if (oom_only) { goto oom_scenario; }

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
    /* Historical EXEC-ring growth test. Bind-without-map is not the current
     * parent-owned scheduler contract; keep its assertions opt-in and pin
     * that fixture's runtime in run_msg_exec_oom.ps1. */
    if (!oom_only) { goto current_context_scenarios; }
oom_scenario:
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
        int oom_failed = 1;
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
            goto oom_cleanup;
        }
        if (lmx_msg_create(rto, 0, 1, &ini, 1, &po) != LMX_MSG_OK) {
            fprintf(stderr, "oom create po\n");
            fflush(stderr);
            goto oom_cleanup;
        }
        if (lmx_msg_create(rto, po, 2, &ini, 1, &bs0) != LMX_MSG_OK) {
            fprintf(stderr, "oom create bs0\n");
            fflush(stderr);
            goto oom_cleanup;
        }
        if (lmx_msg_create(rto, po, 3, &ini, 1, &bs1) != LMX_MSG_OK) {
            fprintf(stderr, "oom create bs1\n");
            fflush(stderr);
            goto oom_cleanup;
        }
        for (k = 0; k < 11; k++) {
            payload[k] = (uchar)(50 + k);
            oom[k].expect = payload[k];
            if (lmx_msg_create(rto, po, (unsigned)(k + 10), &ini, 1, &wo[k]) != LMX_MSG_OK || wo[k] == 0) {
                fprintf(stderr, "oom create %d\n", k);
                goto oom_cleanup;
            }
            if (lmx_msg_exec_bind(rto, wo[k], turn_mass, &oom[k], LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK) {
                fprintf(stderr, "oom bind %d\n", k);
                fflush(stderr);
                goto oom_cleanup;
            }
        }
        if (lmx_msg_create(rto, po, 40, &ini, 1, &wf) != LMX_MSG_OK || wf == 0) {
            fprintf(stderr, "oom create wf\n");
            fflush(stderr);
            goto oom_cleanup;
        }
        if (lmx_msg_exec_bind(rto, wf, turn_two_ok, &oomfifo, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK) {
            fprintf(stderr, "oom bind wf\n");
            fflush(stderr);
            goto oom_cleanup;
        }
        if (lmx_msg_exec_bind(rto, bs0, turn_busy, &spin0, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK) {
            fprintf(stderr, "oom bind bs0\n");
            fflush(stderr);
            goto oom_cleanup;
        }
        if (lmx_msg_exec_bind(rto, bs1, turn_busy, &spin1, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK) {
            fprintf(stderr, "oom bind bs1\n");
            fflush(stderr);
            goto oom_cleanup;
        }
        if (lmx_msg_end_turn(rto, po, 1) != LMX_MSG_OK) {
            fprintf(stderr, "oom end_turn commit\n");
            fflush(stderr);
            goto oom_cleanup;
        }
        lmx_msg_exec_drop_stale_ready(rto);
        memset(&eo, 0, sizeof(eo));
        eo.kind = LMX_MSG_KIND_BYTES;
        eo.n = 1;
        eo.bytes = &ini;
        if (lmx_msg_send(rto, po, bs0, &eo) != LMX_MSG_STAGED || lmx_msg_send(rto, po, bs1, &eo) != LMX_MSG_STAGED) {
            fprintf(stderr, "oom send busy\n");
            fflush(stderr);
            goto oom_cleanup;
        }
        if (lmx_msg_end_turn(rto, po, 1) != LMX_MSG_OK) {
            fprintf(stderr, "oom end_turn busy\n");
            fflush(stderr);
            goto oom_cleanup;
        }
        lmx_msg_pump(rto);
        if (lmx_msg_exec_start(rto, 2) != LMX_MSG_OK) {
            fprintf(stderr, "oom exec_start\n");
            fflush(stderr);
            goto oom_cleanup;
        }
        if (WaitForSingleObject(spin0.started, 2000) != WAIT_OBJECT_0 || WaitForSingleObject(spin1.started, 2000) != WAIT_OBJECT_0) {
            fprintf(stderr, "oom busy workers not started\n");
            fflush(stderr);
            goto oom_cleanup;
        }
        fprintf(stderr, "oom busy running nready=%d cap=%d\n",
            lmx_msg_exec_nready(rto), lmx_msg_exec_ready_cap(rto));
        fflush(stderr);
        for (k = 0; k < 8; k++) {
            eo.bytes = &payload[k];
            if (lmx_msg_host_post(rto, wo[k], &eo) != LMX_MSG_STAGED) {
                fprintf(stderr, "oom host_post fill %d\n", k);
                fflush(stderr);
                goto oom_cleanup;
            }
        }
        if (lmx_msg_host_drain(rto) != LMX_MSG_OK) {
            fprintf(stderr, "oom drain fill\n");
            fflush(stderr);
            goto oom_cleanup;
        }
        cap = lmx_msg_exec_ready_cap(rto);
        nrd = lmx_msg_exec_nready(rto);
        fprintf(stderr, "oom filled cap=%d nready=%d\n", cap, nrd);
        fflush(stderr);
        if (force_oom_cleanup || cap != nrd || nrd < 8) {
            if (force_oom_cleanup) {
                fprintf(stderr, "forced oom cleanup failure\n");
            }
            fprintf(stderr, "oom not at capacity cap=%d nready=%d\n", cap, nrd);
            fflush(stderr);
            goto oom_cleanup;
        }
        for (k = 8; k < 11; k++) {
            if (lmx_msg_exec_ready_has(rto, wo[k]) != 0) {
                fprintf(stderr, "oom wo[%d] already in ready before overflow\n", k);
                fflush(stderr);
                goto oom_cleanup;
            }
        }
        if (lmx_msg_exec_ready_has(rto, wf) != 0) {
            fprintf(stderr, "oom fifo addr already in ready before overflow\n");
            fflush(stderr);
            goto oom_cleanup;
        }
        lmx_msg_exec_test_set_fail_grow(rto, 1);
        for (k = 8; k < 11; k++) {
            eo.bytes = &payload[k];
            if (lmx_msg_host_post(rto, wo[k], &eo) != LMX_MSG_STAGED) {
                fprintf(stderr, "oom host_post overflow %d\n", k);
                fflush(stderr);
                goto oom_cleanup;
            }
        }
        eo.bytes = &f10;
        if (lmx_msg_host_post(rto, wf, &eo) != LMX_MSG_STAGED) {
            fprintf(stderr, "oom host_post fifo 10\n");
            fflush(stderr);
            goto oom_cleanup;
        }
        eo.bytes = &f20;
        if (lmx_msg_host_post(rto, wf, &eo) != LMX_MSG_STAGED) {
            fprintf(stderr, "oom host_post fifo 20\n");
            fflush(stderr);
            goto oom_cleanup;
        }
        if (lmx_msg_host_drain(rto) != LMX_MSG_OK) {
            fprintf(stderr, "oom drain overflow\n");
            fflush(stderr);
            goto oom_cleanup;
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
            goto oom_cleanup;
        }
        for (k = 8; k < 11; k++) {
            if (lmx_msg_exec_ready_has(rto, wo[k]) != 0 || InterlockedCompareExchange(&oom[k].done, 0, 0) != 0) {
                fprintf(stderr, "oom overflow delivered under fail_grow k=%d ready=%d done=%ld\n",
                    k, lmx_msg_exec_ready_has(rto, wo[k]), (long)oom[k].done);
                goto oom_cleanup;
            }
        }
        if (lmx_msg_exec_ready_has(rto, wf) != 0 || InterlockedCompareExchange(&oomfifo.done, 0, 0) != 0) {
            fprintf(stderr, "oom fifo delivered under fail_grow ready=%d done=%ld\n",
                lmx_msg_exec_ready_has(rto, wf), (long)oomfifo.done);
            goto oom_cleanup;
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
                goto oom_cleanup;
            }
        }
        lmx_msg_exec_stop(rto);
        for (k = 0; k < 11; k++) {
            if (oom[k].got != oom[k].expect || oom[k].recv_st != LMX_MSG_OK || oom[k].end_st != LMX_MSG_OK || InterlockedCompareExchange(&oom[k].done, 0, 0) != 1 || InterlockedCompareExchange(&oom[k].in_turn, 0, 0) != 0) {
                fprintf(stderr, "oom[%d] got=%u expect=%u done=%ld recv=%d end=%d in_turn=%ld\n",
                    k, oom[k].got, oom[k].expect, (long)oom[k].done, oom[k].recv_st, oom[k].end_st,
                    (long)InterlockedCompareExchange(&oom[k].in_turn, 0, 0));
                fflush(stderr);
                goto oom_cleanup;
            }
            if (lmx_msg_exec_ready_has(rto, wo[k]) != 0 || lmx_msg_inbox_n(rto, wo[k]) != 0 || lmx_msg_exec_is_runnable(rto, wo[k]) != 0) {
                fprintf(stderr, "oom[%d] not drained ready=%d inbox=%d runnable=%d\n",
                    k, lmx_msg_exec_ready_has(rto, wo[k]), lmx_msg_inbox_n(rto, wo[k]),
                    lmx_msg_exec_is_runnable(rto, wo[k]));
                fflush(stderr);
                goto oom_cleanup;
            }
        }
        if (oomfifo.fifo_a != 10 || oomfifo.fifo_b != 20 || oomfifo.recv_st != LMX_MSG_OK || oomfifo.end_st != LMX_MSG_OK || InterlockedCompareExchange(&oomfifo.done, 0, 0) != 2 || InterlockedCompareExchange(&oomfifo.in_turn, 0, 0) != 0 || oomfifo.overlap != 0) {
            fprintf(stderr, "oom fifo %u,%u done=%ld recv=%d end=%d overlap=%d in_turn=%ld hits=%d scan_was=%d\n",
                oomfifo.fifo_a, oomfifo.fifo_b, (long)InterlockedCompareExchange(&oomfifo.done, 0, 0),
                oomfifo.recv_st, oomfifo.end_st, oomfifo.overlap,
                (long)InterlockedCompareExchange(&oomfifo.in_turn, 0, 0), hits, sc);
            fflush(stderr);
            goto oom_cleanup;
        }
        if (lmx_msg_exec_ready_has(rto, wf) != 0 || lmx_msg_inbox_n(rto, wf) != 0 || lmx_msg_exec_is_runnable(rto, wf) != 0 || lmx_msg_exec_nready(rto) != 0) {
            fprintf(stderr, "oom fifo not drained ready=%d inbox=%d runnable=%d nready=%d\n",
                lmx_msg_exec_ready_has(rto, wf), lmx_msg_inbox_n(rto, wf),
                lmx_msg_exec_is_runnable(rto, wf), lmx_msg_exec_nready(rto));
            fflush(stderr);
            goto oom_cleanup;
        }
        g_oom_n = 12;
        g_oom_hits = hits;
        g_oom_scan = sc;
        g_oom_fifo_a = oomfifo.fifo_a;
        g_oom_fifo_b = oomfifo.fifo_b;
        oom_failed = 0;
oom_cleanup:
        /* Release spin workers before any join, on EVERY scenario exit. */
        InterlockedExchange(&g_cpu_stop, 1);
        if (rto != 0) {
            lmx_msg_exec_test_set_fail_grow(rto, 0);
            lmx_msg_exec_stop(rto);
        }
        if (spin0.started != 0) { CloseHandle(spin0.started); }
        if (spin1.started != 0) { CloseHandle(spin1.started); }
        lmx_msg_runtime_delete(rto);
        fprintf(stderr, "oom cleanup complete failed=%d\n", oom_failed);
        if (oom_failed) { return 1; }
    }
    if (oom_only) {
        printf("exec oom-only ok count=%d hits=%d scan=%d fifo=%u,%u\n",
            g_oom_n, g_oom_hits, g_oom_scan, g_oom_fifo_a, g_oom_fifo_b);
        return 0;
    }
current_context_scenarios:
    fprintf(stderr, "legacy_ready_oom=SKIPPED (opt-in pinned run_msg_exec_oom.ps1)\n");
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
        if (lmx_msg_adopt_failed(rth, p, c) != LMX_MSG_INVALID
            || lmx_msg_find(rth, g)->init != gbase || lmx_msg_find(rth, c)->init != cbase) {
            fprintf(stderr, "C adopt must reject while G undisposed\n");
            lmx_msg_runtime_delete(rth);
            return 1;
        }
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
            void *live_init;
            LmxMsgEnv e;
            int pst;
            if (lmx_msg_create(rth, p, 9, &ini, 1, &live) != LMX_MSG_OK || lmx_msg_end_turn(rth, p, 1) != LMX_MSG_OK) {
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
            if (lmx_msg_dispose_child(rth, dummy, p) != LMX_MSG_INVALID) {
                fprintf(stderr, "dispose while live child unsettled\n");
                lmx_msg_runtime_delete(rth);
                return 1;
            }
            if (lmx_msg_exec_bind(rth, live, turn_fail_end, 0, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK
                || lmx_msg_emergency_cancel(rth, live) != LMX_MSG_OK) {
                fprintf(stderr, "live settle\n");
                lmx_msg_runtime_delete(rth);
                return 1;
            }
            (void)lmx_msg_run_child_turn(rth, live);
            if (lmx_msg_dispose_child(rth, p, live) != LMX_MSG_INVALID || lmx_msg_find(rth, live)->init != live_init) {
                fprintf(stderr, "failure dispose must not drop history\n");
                lmx_msg_runtime_delete(rth);
                return 1;
            }
            if (lmx_msg_adopt_failed(rth, p, live) != LMX_MSG_OK) {
                fprintf(stderr, "adopt live fail\n");
                lmx_msg_runtime_delete(rth);
                return 1;
            }
            if (lmx_msg_transfer_adopted(rth, p, dummy) != LMX_MSG_OK || lmx_msg_adopted_n(rth, dummy) != 4 || lmx_msg_adopted_n(rth, p) != 0) {
                fprintf(stderr, "transfer result n dummy=%d p=%d\n", lmx_msg_adopted_n(rth, dummy), lmx_msg_adopted_n(rth, p));
                lmx_msg_runtime_delete(rth);
                return 1;
            }
            if (lmx_msg_dispose_child(rth, dummy, p) != LMX_MSG_OK || lmx_msg_adopted_n(rth, dummy) != 4) {
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
            || cm->init != 0 || cm->ranges != 0 || pm->ranges != cr
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
            || cm->init != 0 || cm->ranges != 0 || cm->blocks != 0
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
        Lmx g;
        rtc = lmx_msg_runtime_new();
        memset(&g, 0, sizeof(g));
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
        if (ma == 0 || mb == 0 || ta == 0 || tb == 0 || cell == 0
            || lmx_char_cell_known(tb, 65) == cell) {
            fprintf(stderr, "chars collect tables\n");
            lmx_msg_runtime_delete(rtc);
            return 1;
        }
        g.data = cell;
        lmx_msg_set_graph(ma, &g);
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
        lmx_msg_set_graph(mb, &g);
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
        g.data = 0;
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
        LmxMsgRuntime *rta;
        LmxMsgAddr a = 0, b = 0;
        uchar ini = 4;
        LmxMsg *ma;
        LmxMsg *mb;
        LmxArrayDesc *da;
        LmxArrayDesc *db;
        void *back_a;
        Lmx g;
        rta = lmx_msg_runtime_new();
        memset(&g, 0, sizeof(g));
        if (rta == 0 || lmx_msg_create(rta, 0, 1, &ini, 1, &a) != LMX_MSG_OK
            || lmx_msg_create(rta, 0, 2, &ini, 1, &b) != LMX_MSG_OK) {
            fprintf(stderr, "array collect create\n");
            return 1;
        }
        ma = lmx_msg_find(rta, a);
        mb = lmx_msg_find(rta, b);
        da = lmx_array_new_positive_owned(LMX_TYPE_ARRAY_OF_CHAR, 3U, &ma->blocks, &ma->ranges);
        db = lmx_array_new_positive_owned(LMX_TYPE_ARRAY_OF_INT, 2U, &mb->blocks, &mb->ranges);
        if (ma == 0 || mb == 0 || da == 0 || db == 0 || da->data == 0 || db->data == 0) {
            fprintf(stderr, "array collect new\n");
            lmx_msg_runtime_delete(rta);
            return 1;
        }
        back_a = da->data;
        ((char *)back_a)[0] = 'Q';
        g.data = da;
        lmx_msg_set_graph(ma, &g);
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
        lmx_msg_set_graph(mb, &g);
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
        g.data = 0;
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
        Lmx g, n_desc, n_meth;
        void *char_back;
        void *int_back;
        void *share_back;
        rtr = lmx_msg_runtime_new();
        memset(&g, 0, sizeof(g));
        memset(&n_desc, 0, sizeof(n_desc));
        memset(&n_meth, 0, sizeof(n_meth));
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
        ((Lmx **)of_lmx->data)[0] = &n_desc;
        ((Lmx **)of_lmx->data)[1] = &n_meth;
        ((LmxArrayDesc **)of_desc->data)[0] = chars;
        ((LmxArrayDesc **)of_desc->data)[1] = of_cycle;
        ((LmxArrayDesc **)of_cycle->data)[0] = of_desc;
        ((LmxArrayDesc **)of_cycle->data)[1] = chars;
        ((LmxArrayDesc **)of_share->data)[0] = chars;
        ((LmxMethod **)of_meth->data)[0] = rec;
        n_desc.data = of_desc;
        n_meth.data = of_meth;
        g.data = of_lmx;
        lmx_msg_set_graph(ma, &g);
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
        g.data = 0;
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
        Lmx g;
        void *char_back;
        void *int_back;
        rte = lmx_msg_runtime_new();
        memset(&g, 0, sizeof(g));
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
        g.data = chars;
        lmx_msg_set_graph(ma, &g);
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
        g.data = 0;
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
        Lmx *leaf0;
        Lmx *leaf1;
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
        leaf0 = lmx_branch_child_known(unit, 0U);
        leaf1 = lmx_branch_child_known(unit, 1U);
        buf = lmx_array_new_positive_owned(LMX_TYPE_ARRAY_OF_INT, 3U, &ma->blocks, &ma->ranges);
        letters = lmx_array_new_positive_owned(LMX_TYPE_ARRAY_OF_CHAR, 4U, &ma->blocks, &ma->ranges);
        dead = lmx_array_new_positive_owned(LMX_TYPE_ARRAY_OF_INT, 2U, &ma->blocks, &ma->ranges);
        if (leaf0 == 0 || leaf1 == 0 || buf == 0 || letters == 0 || dead == 0
            || buf->data == 0 || letters->data == 0 || dead->data == 0) {
            fprintf(stderr, "emit array fields\n");
            lmx_msg_runtime_delete(rtg);
            return 1;
        }
        leaf0->data = buf;
        leaf1->data = letters;
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
        Lmx *leaf;
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
        leaf = lmx_branch_child_known(unit, 0U);
        buf = lmx_array_new_positive_owned(LMX_TYPE_ARRAY_OF_INT, 3U, &ma->blocks, &ma->ranges);
        if (leaf == 0 || buf == 0 || buf->data == 0) {
            fprintf(stderr, "graph root fields\n");
            lmx_msg_runtime_delete(rtg);
            return 1;
        }
        leaf->data = buf;
        buf_back = buf->data;
        lmx_msg_set_graph(ma, 0);
        if (lmx_msg_root_attach(ma, unit) != LMX_MSG_OK) {
            fprintf(stderr, "graph root attach\n");
            lmx_msg_runtime_delete(rtg);
            return 1;
        }
        if (lmx_msg_end_turn(rtg, a, 1) != LMX_MSG_OK
            || lmx_owned_ranges_find(ma->ranges, unit) == 0
            || lmx_owned_ranges_find(ma->ranges, buf) == 0
            || lmx_owned_ranges_find(ma->ranges, buf_back) == 0) {
            fprintf(stderr, "graph root dropped children\n");
            lmx_msg_runtime_delete(rtg);
            return 1;
        }
        if (lmx_msg_root_release(ma, unit) != LMX_MSG_OK
            || lmx_msg_end_turn(rtg, a, 1) != LMX_MSG_OK
            || lmx_owned_ranges_find(ma->ranges, buf) != 0) {
            fprintf(stderr, "graph root release left payload\n");
            lmx_msg_runtime_delete(rtg);
            return 1;
        }
        fprintf(stderr, "explicit root: CHILDREN unit keeps INT field; release reclaims\n");
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
        LmxMsgRuntime *rth;
        LmxMsgAddr dummy = 0, p = 0, c = 0;
        uchar ini = 13;
        LmxMsg *child;
        LmxMsg *parent;
        Lmx *unit;
        Lmx *leaf;
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
        leaf = lmx_branch_child_known(unit, 0U);
        hist = lmx_array_new_positive_owned(LMX_TYPE_ARRAY_OF_INT, 1U, &child->blocks, &child->ranges);
        if (leaf == 0 || hist == 0 || hist->data == 0) {
            fprintf(stderr, "fail-history field\n");
            lmx_msg_runtime_delete(rth);
            return 1;
        }
        leaf->data = hist;
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
    ev = fopen("build/l1trans/logs/gen2/lmx_message_exec_selftest.evidence.txt", "w");
    if (ev) {
        fprintf(ev, "slow t0=%lu t1=%lu recvd=%u\n", (unsigned long)slow.t0, (unsigned long)slow.t1, slow.recvd);
        fprintf(ev, "fast t2=%lu recvd=%u fifo=%u,%u send_ui=%d send_peer=%d peer_got=%u\n",
            (unsigned long)fast.t2, fast.recvd, fast.fifo[0], fast.fifo[1],
            fast.send_ui_st, fast.send_peer_st, peerrec.got);
        fprintf(ev, "ui_step_ms=%lu cpu_busy_ui_ms=%lu mass_complete=70 fail_fifo=31,32 err_after=1 omit_end=1 ui_from_worker=%u peer=42\n",
            (unsigned long)tui, (unsigned long)tbusy_ui, slow.ui_recvd);
        fprintf(ev, "live_cascade=%d factory_n=%d factory_create_phase_n=%d held_child_meta=1 oom_n=%d oom_hits=%d oom_scan=%d oom_fifo=%u,%u ctx_overlap=%d ctx_restart=%d ctx_child=%d ctx_rebind_ui=%d ctx_fail_retry=%d ctx_busy_ui=%d ctx_rebind_auth=%d ctx_ui_any_rb=%d ctx_mid_unroll=%d ctx_spawn_race=%d ctx_mix_map=%d ctx_map_fail=%d ctx_child_timer=%d ctx_real_clock=%d ctx_bind_rollback=%d ctx_bind_held=%d ctx_bind_ctx=%d ctx_idle_rollback=%d ctx_cancel_idle=%d ctx_complete_idle=%d m0_acc=%d m0_overlap=%d m0_admit=%u,%u m0_apply=%u,%u\n",
            g_live_cascade, g_factory_n, g_factory_n_at_meta, g_oom_n, g_oom_hits, g_oom_scan, g_oom_fifo_a, g_oom_fifo_b,
            g_ctx_overlap, g_ctx_restart, g_ctx_child, g_ctx_rebind_ui, g_ctx_fail_retry, g_ctx_busy_ui,
            g_ctx_rebind_auth, g_ctx_ui_any_rb, g_ctx_mid_unroll, g_ctx_spawn_race, g_ctx_mix_map, g_ctx_map_fail, g_ctx_child_timer, g_ctx_real_clock, g_ctx_bind_rollback, g_ctx_bind_held, g_ctx_bind_ctx, g_ctx_idle_rollback, g_ctx_cancel_idle, g_ctx_complete_idle,
            g_m0_acc, g_m0_overlap, g_m0_admit0, g_m0_admit1, g_m0_apply0, g_m0_apply1);
        fclose(ev);
    }
    printf("lmx_message_exec ok fifo=%u,%u mass=70 fail=31,32 err_after=1 omit_end=1 xsend_ui=%d xsend_peer=%d peer=%u ui_from_worker=%u ui_ms=%lu cpu_busy_ui_ms=%lu live_cascade=%d factory_n=%d factory_create_phase_n=%d oom_n=%d oom_hits=%d oom_scan=%d oom_fifo=%u,%u ctx_overlap=%d restart=%d child=%d rebind_ui=%d fail_retry=%d busy_ui=%d rebind_auth=%d ui_any_rb=%d mid_unroll=%d spawn_race=%d mix_map=%d map_fail=%d child_timer=%d real_clock=%d bind_rollback=%d bind_held=%d bind_ctx=%d idle_rollback=%d cancel_idle=%d complete_idle=%d\n",
        fast.fifo[0], fast.fifo[1], fast.send_ui_st, fast.send_peer_st, peerrec.got,
        slow.ui_recvd, (unsigned long)tui, (unsigned long)tbusy_ui, g_live_cascade, g_factory_n,
        g_factory_n_at_meta, g_oom_n, g_oom_hits, g_oom_scan, g_oom_fifo_a, g_oom_fifo_b,
        g_ctx_overlap, g_ctx_restart, g_ctx_child, g_ctx_rebind_ui, g_ctx_fail_retry, g_ctx_busy_ui,
        g_ctx_rebind_auth, g_ctx_ui_any_rb, g_ctx_mid_unroll, g_ctx_spawn_race, g_ctx_mix_map, g_ctx_map_fail, g_ctx_child_timer, g_ctx_real_clock, g_ctx_bind_rollback, g_ctx_bind_held, g_ctx_bind_ctx, g_ctx_idle_rollback, g_ctx_cancel_idle, g_ctx_complete_idle);
    return 0;
}
