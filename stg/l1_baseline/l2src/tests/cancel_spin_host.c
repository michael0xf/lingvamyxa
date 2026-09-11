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
int lmx_int_store(void *cell, int v);

typedef struct SpinCtx {
    Lmx *node;
    LmxMsgRuntime *rt;
    LmxMsgAddr self;
    LmxMsgAddr sib;
    LmxMsgAddr child;
    volatile LONG done;
    volatile LONG child_done;
    int send_st;
} SpinCtx;

typedef struct CancelArg {
    LmxMsgRuntime *rt;
    LmxMsgAddr who;
    Lmx *node;
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

static int field_at(Lmx *node, unsigned i) {
    Lmx *leaf = lmx_branch_child(node, i);
    if (leaf == 0 || leaf->data == 0) {
        return -1;
    }
    return lmx_int_value(leaf->data);
}

static int reset_fields(Lmx *node) {
    Lmx *hit = lmx_branch_child(node, 0U);
    Lmx *after = lmx_branch_child(node, 1U);
    if (hit == 0 || after == 0 || hit->data == 0 || after->data == 0) {
        return 1;
    }
    return lmx_int_store(hit->data, 0) != 0 || lmx_int_store(after->data, 0) != 0;
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
    (void)l2_m1(c->node);
    InterlockedIncrement(&c->done);
    return 0;
}

static int turn_child_end(LmxMsgRuntime *rt, LmxMsgAddr who, void *ctx) {
    SpinCtx *c = (SpinCtx *)ctx;
    LmxMsgEnv got;
    memset(&got, 0, sizeof(got));
    lmx_msg_recv(rt, who, &got);
    lmx_msg_env_release(&got);
    InterlockedIncrement(&c->child_done);
    return 0;
}

static int turn_parent_nested(LmxMsgRuntime *rt, LmxMsgAddr who, void *ctx) {
    SpinCtx *c = (SpinCtx *)ctx;
    LmxMsgEnv got;
    memset(&got, 0, sizeof(got));
    lmx_msg_recv(rt, who, &got);
    lmx_msg_env_release(&got);
    (void)lmx_msg_sched_step(rt, who);
    (void)l2_m1(c->node);
    InterlockedIncrement(&c->done);
    return 0;
}

static DWORD WINAPI cancel_after_settle(void *arg) {
    CancelArg *a = (CancelArg *)arg;
    Sleep(20);
    if (lmx_msg_emergency_cancel(a->rt, a->who) == LMX_MSG_OK) {
        InterlockedIncrement(&a->fired);
    }
    return 0;
}

typedef struct TwoCancelArg {
    LmxMsgRuntime *rt;
    LmxMsgAddr first;
    LmxMsgAddr second;
    volatile LONG fired1;
    volatile LONG fired2;
} TwoCancelArg;

static DWORD WINAPI cancel_child_then_parent(void *arg) {
    TwoCancelArg *a = (TwoCancelArg *)arg;
    Sleep(20);
    if (lmx_msg_emergency_cancel(a->rt, a->first) == LMX_MSG_OK) {
        InterlockedIncrement(&a->fired1);
    }
    Sleep(20);
    if (lmx_msg_emergency_cancel(a->rt, a->second) == LMX_MSG_OK) {
        InterlockedIncrement(&a->fired2);
    }
    return 0;
}

static Lmx *make_int_node(void) {
    Lmx *unit;
    Lmx *leaf;
    if (lmx_ranges_init(6U) != 0 || lmx_int_init() != 0) {
        return 0;
    }
    unit = (Lmx *)malloc(sizeof(Lmx));
    if (unit == 0) {
        return 0;
    }
    lmx_cell_init(unit, 0, 0);
    if (lmx_branch_open(unit, 2U) != 0) {
        return 0;
    }
    leaf = lmx_branch_child(unit, 0U);
    if (leaf == 0) {
        return 0;
    }
    leaf->data = lmx_int_take();
    leaf = lmx_branch_child(unit, 1U);
    if (leaf == 0) {
        return 0;
    }
    leaf->data = lmx_int_take();
    return unit;
}

static int spin_boot(LmxMsgRuntime **rt_out, LmxMsgAddr *p_out, LmxMsgAddr *c_out,
    LmxMsgAddr *sib_out, LmxMsgAddr *g_out, SpinCtx *ctx, Lmx *node, LmxMsgTurn child_turn) {
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
    ctx->child = c;
    if (lmx_msg_exec_bind(rt, c, child_turn, ctx, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK) {
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
    LmxMsgAddr g, SpinCtx *ctx, Lmx *node, const char *tag, int expect_parent_live) {
    LmxMsg *gm;
    LmxMsgEnv e;
    uchar ini = 1;
    gm = lmx_msg_find(rt, g);
    if (lmx_msg_state(rt, c) != LMX_MSG_STATE_STOPPED) {
        fprintf(stderr, "%s child state=%d hit=%d after=%d\n", tag, lmx_msg_state(rt, c),
            field_at(node, 0U), field_at(node, 1U));
        return 1;
    }
    if (expect_parent_live != 0) {
        if (lmx_msg_state(rt, p) == LMX_MSG_STATE_STOPPED || lmx_msg_state(rt, p) == LMX_MSG_STATE_DEAD) {
            fprintf(stderr, "%s parent stopped state=%d\n", tag, lmx_msg_state(rt, p));
            return 1;
        }
    }
    if (field_at(node, 0U) != 1) {
        fprintf(stderr, "%s inner loop not reached hit=%d\n", tag, field_at(node, 0U));
        return 1;
    }
    if (field_at(node, 1U) != 0) {
        fprintf(stderr, "%s nested L2 continuation wrote after=%d done=%ld\n", tag,
            field_at(node, 1U), (long)InterlockedCompareExchange(&ctx->done, 0, 0));
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
    if (reset_fields(node) != 0 || spin_boot(&rt, &p, &c, &sib, &g, &ctx, node, turn_spin) != 0) {
        fprintf(stderr, "spin-map boot\n");
        return 1;
    }
    if (lmx_msg_map_child(rt, p, c) != LMX_MSG_OK) {
        return fail_rt(rt, "spin-map map_child");
    }
    Sleep(20);
    if (lmx_msg_emergency_cancel(rt, c) != LMX_MSG_OK) {
        return fail_rt(rt, "spin-map cancel");
    }
    dl = GetTickCount() + 3000;
    while (lmx_msg_state(rt, c) != LMX_MSG_STATE_STOPPED && GetTickCount() < dl) {
        Sleep(10);
    }
    Sleep(50);
    if (check_aftermath(rt, p, c, sib, g, &ctx, node, "spin-map", 1) != 0) {
        return fail_rt(rt, "spin-map aftermath");
    }
    lmx_msg_exec_stop(rt);
    lmx_msg_runtime_delete(rt);
    fprintf(stderr, "cancel_spin_map ok hit=%d after=%d done=%ld send_st=%d\n",
        field_at(node, 0U), field_at(node, 1U),
        (long)InterlockedCompareExchange(&ctx.done, 0, 0), ctx.send_st);
    return 0;
}

static int run_step(Lmx *node) {
    LmxMsgRuntime *rt;
    LmxMsgAddr p = 0, c = 0, sib = 0, g = 0;
    SpinCtx ctx;
    CancelArg carg;
    HANDLE th;
    int st;
    if (reset_fields(node) != 0 || spin_boot(&rt, &p, &c, &sib, &g, &ctx, node, turn_spin) != 0) {
        fprintf(stderr, "spin-step boot\n");
        return 1;
    }
    memset(&carg, 0, sizeof(carg));
    carg.rt = rt;
    carg.who = c;
    carg.node = node;
    th = CreateThread(0, 0, cancel_after_settle, &carg, 0, 0);
    if (th == 0) {
        return fail_rt(rt, "spin-step thread");
    }
    st = lmx_msg_sched_step(rt, p);
    WaitForSingleObject(th, 3000);
    CloseHandle(th);
    if (st != LMX_MSG_OK && st != 1) {
        fprintf(stderr, "spin-step sched_step st=%d hit=%d fired=%ld\n", st,
            field_at(node, 0U), (long)InterlockedCompareExchange(&carg.fired, 0, 0));
        return fail_rt(rt, "spin-step sched");
    }
    if (InterlockedCompareExchange(&carg.fired, 0, 0) < 1) {
        return fail_rt(rt, "spin-step cancel not fired");
    }
    if (check_aftermath(rt, p, c, sib, g, &ctx, node, "spin-step", 1) != 0) {
        return fail_rt(rt, "spin-step aftermath");
    }
    lmx_msg_exec_stop(rt);
    lmx_msg_runtime_delete(rt);
    fprintf(stderr, "cancel_spin_step ok hit=%d after=%d done=%ld parent_continued=1\n",
        field_at(node, 0U), field_at(node, 1U),
        (long)InterlockedCompareExchange(&ctx.done, 0, 0));
    return 0;
}

static int run_nested(Lmx *node) {
    LmxMsgRuntime *rt;
    LmxMsgAddr p = 0, c = 0, sib = 0, g = 0;
    SpinCtx ctx;
    CancelArg carg;
    HANDLE th;
    int st;
    LmxMsgEnv e;
    uchar ini = 1;
    if (reset_fields(node) != 0 || spin_boot(&rt, &p, &c, &sib, &g, &ctx, node, turn_child_end) != 0) {
        fprintf(stderr, "spin-nested boot\n");
        return 1;
    }
    if (lmx_msg_exec_bind(rt, p, turn_parent_nested, &ctx, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK) {
        return fail_rt(rt, "spin-nested bind parent");
    }
    memset(&e, 0, sizeof(e));
    e.kind = LMX_MSG_KIND_BYTES;
    e.n = 1;
    e.bytes = &ini;
    if (lmx_msg_host_post(rt, p, &e) != LMX_MSG_STAGED) {
        return fail_rt(rt, "spin-nested post parent");
    }
    memset(&carg, 0, sizeof(carg));
    carg.rt = rt;
    carg.who = p;
    carg.node = node;
    th = CreateThread(0, 0, cancel_after_settle, &carg, 0, 0);
    if (th == 0) {
        return fail_rt(rt, "spin-nested thread");
    }
    st = lmx_msg_run_child_turn(rt, p);
    WaitForSingleObject(th, 3000);
    CloseHandle(th);
    if (InterlockedCompareExchange(&ctx.child_done, 0, 0) < 1) {
        fprintf(stderr, "spin-nested child did not return first child_done=%ld st=%d\n",
            (long)InterlockedCompareExchange(&ctx.child_done, 0, 0), st);
        return fail_rt(rt, "spin-nested child");
    }
    if (InterlockedCompareExchange(&carg.fired, 0, 0) < 1) {
        return fail_rt(rt, "spin-nested cancel not fired");
    }
    if (lmx_msg_state(rt, p) != LMX_MSG_STATE_STOPPED) {
        fprintf(stderr, "spin-nested parent state=%d\n", lmx_msg_state(rt, p));
        return fail_rt(rt, "spin-nested parent");
    }
    if (field_at(node, 0U) != 1 || field_at(node, 1U) != 0) {
        fprintf(stderr, "spin-nested hit=%d after=%d\n", field_at(node, 0U), field_at(node, 1U));
        return fail_rt(rt, "spin-nested graph");
    }
    if (InterlockedCompareExchange(&ctx.done, 0, 0) != 0) {
        return fail_rt(rt, "spin-nested host wrapper");
    }
    memset(&e, 0, sizeof(e));
    e.kind = LMX_MSG_KIND_BYTES;
    e.n = 1;
    e.bytes = &ini;
    if (lmx_msg_host_post(rt, sib, &e) != LMX_MSG_STAGED) {
        return fail_rt(rt, "spin-nested sibling");
    }
    lmx_msg_exec_stop(rt);
    lmx_msg_runtime_delete(rt);
    fprintf(stderr, "cancel_spin_nested ok child_done=1 hit=1 after=0 parent_root=1\n");
    return 0;
}

static int run_nested_child_cancel(Lmx *node) {
    LmxMsgRuntime *rt;
    LmxMsgAddr p = 0, c = 0, sib = 0, g = 0;
    SpinCtx ctx;
    TwoCancelArg carg;
    HANDLE th;
    int st;
    LmxMsgEnv e;
    uchar ini = 1;
    if (reset_fields(node) != 0 || spin_boot(&rt, &p, &c, &sib, &g, &ctx, node, turn_spin) != 0) {
        fprintf(stderr, "spin-ncc boot\n");
        return 1;
    }
    if (lmx_msg_exec_bind(rt, p, turn_parent_nested, &ctx, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK) {
        return fail_rt(rt, "spin-ncc bind parent");
    }
    memset(&e, 0, sizeof(e));
    e.kind = LMX_MSG_KIND_BYTES;
    e.n = 1;
    e.bytes = &ini;
    if (lmx_msg_host_post(rt, p, &e) != LMX_MSG_STAGED) {
        return fail_rt(rt, "spin-ncc post parent");
    }
    memset(&carg, 0, sizeof(carg));
    carg.rt = rt;
    carg.first = c;
    carg.second = p;
    th = CreateThread(0, 0, cancel_child_then_parent, &carg, 0, 0);
    if (th == 0) {
        return fail_rt(rt, "spin-ncc thread");
    }
    st = lmx_msg_run_child_turn(rt, p);
    WaitForSingleObject(th, 3000);
    CloseHandle(th);
    if (InterlockedCompareExchange(&carg.fired1, 0, 0) < 1 || InterlockedCompareExchange(&carg.fired2, 0, 0) < 1) {
        fprintf(stderr, "spin-ncc fired child=%ld parent=%ld st=%d\n",
            (long)InterlockedCompareExchange(&carg.fired1, 0, 0),
            (long)InterlockedCompareExchange(&carg.fired2, 0, 0), st);
        return fail_rt(rt, "spin-ncc cancels");
    }
    if (lmx_msg_state(rt, p) != LMX_MSG_STATE_STOPPED || field_at(node, 1U) != 0) {
        fprintf(stderr, "spin-ncc parent=%d after=%d st=%d\n", lmx_msg_state(rt, p), field_at(node, 1U), st);
        return fail_rt(rt, "spin-ncc parent root");
    }
    memset(&e, 0, sizeof(e));
    e.kind = LMX_MSG_KIND_BYTES;
    e.n = 1;
    e.bytes = &ini;
    if (lmx_msg_host_post(rt, sib, &e) != LMX_MSG_STAGED) {
        return fail_rt(rt, "spin-ncc sibling");
    }
    lmx_msg_exec_stop(rt);
    lmx_msg_runtime_delete(rt);
    fprintf(stderr, "cancel_spin_nested_child_cancel ok after=0 parent_root=1\n");
    return 0;
}

static int run_nested_precancel(Lmx *node) {
    LmxMsgRuntime *rt;
    LmxMsgAddr p = 0, c = 0, sib = 0, g = 0;
    SpinCtx ctx;
    CancelArg carg;
    HANDLE th;
    LmxMsgEnv e;
    uchar ini = 1;
    if (reset_fields(node) != 0 || spin_boot(&rt, &p, &c, &sib, &g, &ctx, node, turn_child_end) != 0) {
        fprintf(stderr, "spin-npc boot\n");
        return 1;
    }
    if (lmx_msg_exec_bind(rt, p, turn_parent_nested, &ctx, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK) {
        return fail_rt(rt, "spin-npc bind parent");
    }
    if (lmx_msg_emergency_cancel(rt, c) != LMX_MSG_OK) {
        return fail_rt(rt, "spin-npc precancel child");
    }
    memset(&e, 0, sizeof(e));
    e.kind = LMX_MSG_KIND_BYTES;
    e.n = 1;
    e.bytes = &ini;
    if (lmx_msg_host_post(rt, p, &e) != LMX_MSG_STAGED) {
        return fail_rt(rt, "spin-npc post parent");
    }
    memset(&carg, 0, sizeof(carg));
    carg.rt = rt;
    carg.who = p;
    carg.node = node;
    th = CreateThread(0, 0, cancel_after_settle, &carg, 0, 0);
    if (th == 0) {
        return fail_rt(rt, "spin-npc thread");
    }
    (void)lmx_msg_run_child_turn(rt, p);
    WaitForSingleObject(th, 3000);
    CloseHandle(th);
    if (lmx_msg_state(rt, p) != LMX_MSG_STATE_STOPPED || field_at(node, 1U) != 0) {
        fprintf(stderr, "spin-npc parent=%d after=%d\n", lmx_msg_state(rt, p), field_at(node, 1U));
        return fail_rt(rt, "spin-npc parent root");
    }
    memset(&e, 0, sizeof(e));
    e.kind = LMX_MSG_KIND_BYTES;
    e.n = 1;
    e.bytes = &ini;
    if (lmx_msg_host_post(rt, sib, &e) != LMX_MSG_STAGED) {
        return fail_rt(rt, "spin-npc sibling");
    }
    lmx_msg_exec_stop(rt);
    lmx_msg_runtime_delete(rt);
    fprintf(stderr, "cancel_spin_nested_precancel ok after=0 parent_root=1\n");
    return 0;
}

int main(void) {
    Lmx *node = make_int_node();
    if (node == 0) {
        fprintf(stderr, "spin node\n");
        return 1;
    }
    if (run_map(node) != 0 || run_step(node) != 0 || run_nested(node) != 0
        || run_nested_child_cancel(node) != 0 || run_nested_precancel(node) != 0) {
        return 1;
    }
    printf("cancel_spin ok\n");
    return 0;
}
