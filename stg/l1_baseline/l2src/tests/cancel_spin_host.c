#include "l2src/lmx.h"
#include "l2src/lmx_message.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

int l2_m1(Lmx *node);
int lmx_ranges_init(size_t capacity);
int lmx_int_init(void);
void lmx_cell_init(Lmx *cell, Lmx *parent, void *data);
int lmx_branch_open(Lmx *parent, size_t children);
Lmx *lmx_branch_child(Lmx *parent, size_t index);
void *lmx_int_take(void);
int lmx_int_value(void *cell);

typedef struct SpinCtx {
    Lmx *node;
    LmxMsgRuntime *rt;
    LmxMsgAddr self;
    LmxMsgAddr sib;
    volatile LONG hits;
    volatile LONG done;
    int send_st;
} SpinCtx;

typedef struct CancelArg {
    LmxMsgRuntime *rt;
    LmxMsgAddr who;
    volatile LONG *hits;
    volatile LONG fired;
} CancelArg;

static int fail_rt(LmxMsgRuntime *rt, const char *msg) {
    fprintf(stderr, "%s\n", msg);
    if (rt != 0) {
        lmx_msg_exec_stop(rt);
        lmx_msg_runtime_delete(rt);
    }
    return 1;
}

static int turn_spin(LmxMsgRuntime *rt, LmxMsgAddr who, void *ctx) {
    SpinCtx *c = (SpinCtx *)ctx;
    LmxMsgEnv got;
    LmxMsgEnv out;
    uchar marker = 9;
    memset(&got, 0, sizeof(got));
    lmx_msg_recv(rt, who, &got);
    lmx_msg_env_release(&got);
    memset(&out, 0, sizeof(out));
    out.kind = LMX_MSG_KIND_BYTES;
    out.n = 1;
    out.bytes = &marker;
    c->send_st = lmx_msg_send(rt, who, c->sib, &out);
    InterlockedIncrement(&c->hits);
    (void)l2_m1(c->node);
    InterlockedIncrement(&c->done);
    return 0;
}

static int outer_after(Lmx *node) {
    Lmx *leaf = lmx_branch_child(node, 0U);
    if (leaf == 0 || leaf->data == 0) {
        return -1;
    }
    return lmx_int_value(leaf->data);
}

static DWORD WINAPI cancel_after_hit(void *arg) {
    CancelArg *a = (CancelArg *)arg;
    DWORD dl = GetTickCount() + 3000;
    while (InterlockedCompareExchange(a->hits, 0, 0) == 0 && GetTickCount() < dl) {
        Sleep(1);
    }
    if (InterlockedCompareExchange(a->hits, 0, 0) > 0) {
        Sleep(20);
    }
    if (lmx_msg_emergency_cancel(a->rt, a->who) == LMX_MSG_OK) {
        InterlockedIncrement(&a->fired);
    }
    return 0;
}

static Lmx *make_int_node(void) {
    Lmx *unit;
    Lmx *leaf;
    if (lmx_ranges_init(4U) != 0 || lmx_int_init() != 0) {
        return 0;
    }
    unit = (Lmx *)malloc(sizeof(Lmx));
    if (unit == 0) {
        return 0;
    }
    lmx_cell_init(unit, 0, 0);
    if (lmx_branch_open(unit, 1U) != 0) {
        return 0;
    }
    leaf = lmx_branch_child(unit, 0U);
    if (leaf == 0) {
        return 0;
    }
    leaf->data = lmx_int_take();
    return unit;
}

static int spin_boot(LmxMsgRuntime **rt_out, LmxMsgAddr *p_out, LmxMsgAddr *c_out,
    LmxMsgAddr *sib_out, LmxMsgAddr *g_out, SpinCtx *ctx, Lmx *node) {
    LmxMsgRuntime *rt;
    LmxMsgAddr p = 0, c = 0, sib = 0, g = 0;
    LmxMsgEnv e;
    uchar ini = 1;
    memset(ctx, 0, sizeof(*ctx));
    ctx->node = node;
    rt = lmx_msg_runtime_new();
    if (rt == 0 || lmx_msg_create(rt, 0, 1, &ini, 1, &p) != LMX_MSG_OK) {
        return 1;
    }
    if (lmx_msg_create(rt, p, 2, &ini, 1, &sib) != LMX_MSG_OK || lmx_msg_end_turn(rt, p, 1) != LMX_MSG_OK) {
        lmx_msg_runtime_delete(rt);
        return 1;
    }
    if (lmx_msg_create(rt, p, 3, &ini, 1, &c) != LMX_MSG_OK || lmx_msg_end_turn(rt, p, 1) != LMX_MSG_OK) {
        lmx_msg_runtime_delete(rt);
        return 1;
    }
    if (lmx_msg_create(rt, c, 4, &ini, 1, &g) != LMX_MSG_OK || lmx_msg_end_turn(rt, c, 1) != LMX_MSG_OK) {
        lmx_msg_runtime_delete(rt);
        return 1;
    }
    ctx->rt = rt;
    ctx->self = c;
    ctx->sib = sib;
    if (lmx_msg_exec_bind(rt, c, turn_spin, ctx, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK) {
        lmx_msg_runtime_delete(rt);
        return 1;
    }
    memset(&e, 0, sizeof(e));
    e.kind = LMX_MSG_KIND_BYTES;
    e.n = 1;
    e.bytes = &ini;
    if (lmx_msg_send(rt, p, c, &e) != LMX_MSG_STAGED || lmx_msg_end_turn(rt, p, 1) != LMX_MSG_OK) {
        lmx_msg_runtime_delete(rt);
        return 1;
    }
    lmx_msg_pump(rt);
    *rt_out = rt;
    *p_out = p;
    *c_out = c;
    *sib_out = sib;
    *g_out = g;
    return 0;
}

static int check_aftermath(LmxMsgRuntime *rt, LmxMsgAddr p, LmxMsgAddr c, LmxMsgAddr sib,
    LmxMsgAddr g, SpinCtx *ctx, Lmx *node, const char *tag) {
    LmxMsg *gm;
    LmxMsgEnv e;
    uchar ini = 1;
    gm = lmx_msg_find(rt, g);
    if (lmx_msg_state(rt, c) != LMX_MSG_STATE_STOPPED) {
        fprintf(stderr, "%s child state=%d hits=%ld\n", tag, lmx_msg_state(rt, c),
            (long)InterlockedCompareExchange(&ctx->hits, 0, 0));
        return 1;
    }
    if (lmx_msg_state(rt, p) == LMX_MSG_STATE_STOPPED || lmx_msg_state(rt, p) == LMX_MSG_STATE_DEAD) {
        fprintf(stderr, "%s parent stopped state=%d\n", tag, lmx_msg_state(rt, p));
        return 1;
    }
    if (InterlockedCompareExchange(&ctx->hits, 0, 0) < 1) {
        fprintf(stderr, "%s no hits\n", tag);
        return 1;
    }
    if (outer_after(node) != 0) {
        fprintf(stderr, "%s nested L2 continuation ran after=%d done=%ld\n", tag,
            outer_after(node), (long)InterlockedCompareExchange(&ctx->done, 0, 0));
        return 1;
    }
    if (InterlockedCompareExchange(&ctx->done, 0, 0) != 0) {
        fprintf(stderr, "%s host wrapper after L2 ran done=%ld\n", tag,
            (long)InterlockedCompareExchange(&ctx->done, 0, 0));
        return 1;
    }
    if (gm == 0 || lmx_msg_running_load(gm) != 0) {
        fprintf(stderr, "%s grandchild run=%d\n", tag, gm ? (int)lmx_msg_running_load(gm) : -1);
        return 1;
    }
    if (lmx_msg_inbox_n(rt, sib) != 0) {
        fprintf(stderr, "%s failed outbox published n=%d send_st=%d\n", tag,
            lmx_msg_inbox_n(rt, sib), ctx->send_st);
        return 1;
    }
    memset(&e, 0, sizeof(e));
    e.kind = LMX_MSG_KIND_BYTES;
    e.n = 1;
    e.bytes = &ini;
    if (lmx_msg_host_post(rt, sib, &e) != LMX_MSG_STAGED) {
        fprintf(stderr, "%s sibling\n", tag);
        return 1;
    }
    return 0;
}

static int run_map(Lmx *node) {
    LmxMsgRuntime *rt;
    LmxMsgAddr p = 0, c = 0, sib = 0, g = 0;
    SpinCtx ctx;
    DWORD dl;
    if (spin_boot(&rt, &p, &c, &sib, &g, &ctx, node) != 0) {
        fprintf(stderr, "spin-map boot\n");
        return 1;
    }
    if (lmx_msg_map_child(rt, p, c) != LMX_MSG_OK) {
        return fail_rt(rt, "spin-map map_child");
    }
    dl = GetTickCount() + 3000;
    while (InterlockedCompareExchange(&ctx.hits, 0, 0) == 0 && GetTickCount() < dl) {
        Sleep(1);
    }
    if (InterlockedCompareExchange(&ctx.hits, 0, 0) > 0) {
        Sleep(20);
    }
    if (lmx_msg_emergency_cancel(rt, c) != LMX_MSG_OK) {
        return fail_rt(rt, "spin-map cancel");
    }
    dl = GetTickCount() + 3000;
    while (lmx_msg_state(rt, c) != LMX_MSG_STATE_STOPPED && GetTickCount() < dl) {
        Sleep(10);
    }
    Sleep(50);
    if (check_aftermath(rt, p, c, sib, g, &ctx, node, "spin-map") != 0) {
        return fail_rt(rt, "spin-map aftermath");
    }
    lmx_msg_exec_stop(rt);
    lmx_msg_runtime_delete(rt);
    fprintf(stderr, "cancel_spin_map ok hits=%ld done=%ld send_st=%d after=%d\n",
        (long)InterlockedCompareExchange(&ctx.hits, 0, 0),
        (long)InterlockedCompareExchange(&ctx.done, 0, 0), ctx.send_st, outer_after(node));
    return 0;
}

static int run_step(Lmx *node) {
    LmxMsgRuntime *rt;
    LmxMsgAddr p = 0, c = 0, sib = 0, g = 0;
    SpinCtx ctx;
    CancelArg carg;
    HANDLE th;
    int st;
    if (spin_boot(&rt, &p, &c, &sib, &g, &ctx, node) != 0) {
        fprintf(stderr, "spin-step boot\n");
        return 1;
    }
    memset(&carg, 0, sizeof(carg));
    carg.rt = rt;
    carg.who = c;
    carg.hits = &ctx.hits;
    th = CreateThread(0, 0, cancel_after_hit, &carg, 0, 0);
    if (th == 0) {
        return fail_rt(rt, "spin-step thread");
    }
    st = lmx_msg_sched_step(rt, p);
    WaitForSingleObject(th, 3000);
    CloseHandle(th);
    /* run_one returns 1 when the turn returned 0 and end_turn cleanup ran. */
    if (st != LMX_MSG_OK && st != 1) {
        fprintf(stderr, "spin-step sched_step st=%d hits=%ld fired=%ld\n", st,
            (long)InterlockedCompareExchange(&ctx.hits, 0, 0),
            (long)InterlockedCompareExchange(&carg.fired, 0, 0));
        return fail_rt(rt, "spin-step sched");
    }
    if (InterlockedCompareExchange(&carg.fired, 0, 0) < 1) {
        return fail_rt(rt, "spin-step cancel not fired");
    }
    if (check_aftermath(rt, p, c, sib, g, &ctx, node, "spin-step") != 0) {
        return fail_rt(rt, "spin-step aftermath");
    }
    lmx_msg_exec_stop(rt);
    lmx_msg_runtime_delete(rt);
    fprintf(stderr, "cancel_spin_step ok hits=%ld done=%ld send_st=%d after=%d parent_continued=1\n",
        (long)InterlockedCompareExchange(&ctx.hits, 0, 0),
        (long)InterlockedCompareExchange(&ctx.done, 0, 0), ctx.send_st, outer_after(node));
    return 0;
}

int main(void) {
    Lmx *node = make_int_node();
    if (node == 0) {
        fprintf(stderr, "spin node\n");
        return 1;
    }
    if (run_map(node) != 0 || run_step(node) != 0) {
        return 1;
    }
    printf("cancel_spin ok\n");
    return 0;
}
