#include "l2src/lmx.h"
#include "l2src/lmx_message.h"
#include "l2src/lmx_branch_owned.lm1.h"
#include "l2src/lmx_value_owned.lm1.h"
#include "l2src/lmx_msg_blocks.lm1.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

int l2_m0(Lmx *node);
int l2_m1(Lmx *node);

static LmxMsgBlock *g_graph_blocks;
static LmxOwnedRange *g_graph_ranges;

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

static int fail_rt(LmxMsgRuntime *rt, const char *msg) {
    fprintf(stderr, "%s\n", msg);
    if (rt != 0) {
        lmx_msg_exec_stop(rt);
        lmx_msg_runtime_delete(rt);
    }
    return 1;
}

static int join_canceler(HANDLE th, LmxMsgRuntime *rt, const char *tag) {
    DWORD w;
    if (th == 0) {
        return fail_rt(rt, tag);
    }
    w = WaitForSingleObject(th, 3000);
    if (w != WAIT_OBJECT_0) {
        WaitForSingleObject(th, INFINITE);
        CloseHandle(th);
        return fail_rt(rt, tag);
    }
    CloseHandle(th);
    return 0;
}

static int field_at(Lmx *node, unsigned i) {
    Lmx *callable;
    void *cell;
    if (node == 0 || node->node == 0 || i > 1U) {
        return -1;
    }
    callable = i == 0U ? lmx_branch_struct_known(node->node, 0U) : node;
    cell = lmx_branch_child_known(callable, 1U);
    if (cell == 0) {
        return -1;
    }
    return lmx_int_value_known(cell);
}

static int reset_fields(Lmx *node) {
    Lmx *inner;
    void *hit;
    void *after;
    if (node == 0 || node->node == 0) {
        return 1;
    }
    inner = lmx_branch_struct_known(node->node, 0U);
    hit = lmx_branch_child_known(inner, 1U);
    after = lmx_branch_child_known(node, 1U);
    if (hit == 0 || after == 0) {
        return 1;
    }
    return lmx_int_store_known(hit, 0) != 0 || lmx_int_store_known(after, 0) != 0;
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
    out.from_msg = lmx_msg_turn_self(rt);
    /* AD pair: to_msg CANNOT — no handle */
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

/* S4, ruled 2026-09-15: no lane cancels R0 mid-turn; the fixture cancels
 * R0's child. p here is R0 itself (spin_boot's first create on a fresh
 * runtime, lmx_message.lm1:1198-1206): R0 has no parent, so mapping_
 * authority_locked's only route to it is the host outside any turn, and
 * the host is inside this very call for as long as it runs. A spawned
 * thread can no longer cancel p mid-spin under S4's guard -- that was
 * the old prototype's host cancel, not a model operation. The spin
 * moves to a real child instead: spin_boot's own exec_bind (launch=1)
 * never actually launches c's worker here, since contexts_live is 0
 * (this file never calls exec_start_contexts) -- only lmx_msg_map_child
 * calls launch_ctx_thread_rec unconditionally (exec.c:2635), and it
 * requires holding_turn(parent), so p's own turn is where c actually
 * gets mapped and launched, same act run_map's turn_map_child performs.
 * The host cancels c once outside any turn, same as run_map's c. */
static int turn_parent_spin(LmxMsgRuntime *rt, LmxMsgAddr who, void *ctx) {
    SpinCtx *c = (SpinCtx *)ctx;
    LmxMsgEnv got;
    memset(&got, 0, sizeof(got));
    lmx_msg_recv(rt, who, &got);
    lmx_msg_env_release(&got);
    if (c == 0 || lmx_msg_map_child(rt, who, c->child) != LMX_MSG_OK) {
        return 1;
    }
    return 0;
}

/* M: a parent's own act on its child runs in the parent's own turn; no turn runs
 * another Message's turn. turn_map_child is the composite turn of a parent whose
 * act is a map: the cell holds [0] the child's address, [1] map_child's status. */
static int turn_map_child(LmxMsgRuntime *rt, LmxMsgAddr who, void *ctx) {
    unsigned *cell = (unsigned *)ctx;
    if (cell == 0) {
        return 1;
    }
    cell[1] = (unsigned)lmx_msg_map_child(rt, who, cell[0]);
    return lmx_msg_end_turn(rt, who, 1);
}

static unsigned g_cell_parent[2];

/* parent, R0's unbound child, runs its own composite turn acting on child through
 * the bootstrap on this thread. Returns run_entry_turn's status, else the act's. */
static int own_turn_entry(LmxMsgRuntime *rt, LmxMsgAddr parent, LmxMsgTurn turn, LmxMsgAddr child) {
    int st;
    g_cell_parent[0] = child;
    g_cell_parent[1] = (unsigned)LMX_MSG_INVALID;
    st = lmx_msg_run_entry_turn(rt, parent, turn, g_cell_parent);
    return st != LMX_MSG_OK ? st : (int)g_cell_parent[1];
}

static Lmx *make_int_node(void) {
    Lmx *unit;
    Lmx *inner;
    Lmx *inner_body;
    Lmx *outer;
    LmxMethod *inner_method;
    LmxMethod *outer_method;
    void *hit;
    void *after;
    unit = lmx_node_new_owned(&g_graph_blocks, &g_graph_ranges);
    if (unit == 0 || lmx_branch_open_owned(unit, 2U, &g_graph_blocks, &g_graph_ranges) != 0) {
        return 0;
    }
    inner = lmx_struct_new_owned(unit, &g_graph_blocks, &g_graph_ranges);
    inner_body = lmx_struct_new_owned(inner, &g_graph_blocks, &g_graph_ranges);
    outer = lmx_struct_new_owned(unit, &g_graph_blocks, &g_graph_ranges);
    if (inner == 0 || inner_body == 0 || outer == 0
        || lmx_branch_open_owned(inner, 3U, &g_graph_blocks, &g_graph_ranges) != 0
        || lmx_branch_open_owned(outer, 2U, &g_graph_blocks, &g_graph_ranges) != 0
        || lmx_branch_store_known(unit, 0U, inner) != 0
        || lmx_branch_store_known(unit, 1U, outer) != 0) {
        return 0;
    }
    inner_method = lmx_method_new_owned(&g_graph_blocks, &g_graph_ranges);
    outer_method = lmx_method_new_owned(&g_graph_blocks, &g_graph_ranges);
    if (inner_method == 0 || outer_method == 0) {
        return 0;
    }
    inner_method->addr = (LmxEntry)l2_m0;
    inner_method->sig = 1U;
    outer_method->addr = (LmxEntry)l2_m1;
    outer_method->sig = 1U;
    hit = lmx_int_new_owned(&g_graph_blocks, &g_graph_ranges);
    after = lmx_int_new_owned(&g_graph_blocks, &g_graph_ranges);
    if (hit == 0 || after == 0
        || lmx_branch_store_known(inner, 0U, inner_method) != 0
        || lmx_branch_store_known(outer, 0U, outer_method) != 0
        || lmx_branch_store_known(inner, 1U, hit) != 0
        || lmx_branch_store_known(inner, 2U, inner_body) != 0
        || lmx_branch_store_known(outer, 1U, after) != 0) {
        return 0;
    }
    return outer;
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
    if (rt == 0 || lmx_msg_create(rt, 0, &ini, 1, &p) != LMX_MSG_OK) {
        return 1;
    }
    if (lmx_msg_create(rt, p, &ini, 1, &sib) != LMX_MSG_OK || lmx_msg_end_turn(rt, p, 1) != LMX_MSG_OK) {
        lmx_msg_runtime_delete(rt);
        return 1;
    }
    if (lmx_msg_create(rt, p, &ini, 1, &c) != LMX_MSG_OK || lmx_msg_end_turn(rt, p, 1) != LMX_MSG_OK) {
        lmx_msg_runtime_delete(rt);
        return 1;
    }
    if (lmx_msg_create(rt, c, &ini, 1, &g) != LMX_MSG_OK || lmx_msg_end_turn(rt, c, 1) != LMX_MSG_OK) {
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
    e.from_msg = lmx_msg_turn_self(rt);
    /* AD pair: to_msg CANNOT — no handle */
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
    if (reset_fields(node) != 0 || spin_boot(&rt, &p, &c, &sib, &g, &ctx, node, turn_spin) != 0) {
        fprintf(stderr, "spin-map boot\n");
        return 1;
    }
    if (own_turn_entry(rt, p, turn_map_child, c) != LMX_MSG_OK) {
        return fail_rt(rt, "spin-map map_child");
    }
    /* M: yield rounds reading flags, no wall clock: the child's L2 loop is
     * entered, then the cancel stops it. */
    fprintf(stderr, "reading: spin-map: the child's inner loop entered\n");
    fflush(stderr);
    while (field_at(node, 0U) != 1) {
        SwitchToThread();
    }
    if (lmx_msg_emergency_cancel(rt, c) != LMX_MSG_OK) {
        return fail_rt(rt, "spin-map cancel");
    }
    fprintf(stderr, "reading: spin-map: the cancelled child stopped\n");
    fflush(stderr);
    while (lmx_msg_state(rt, c) != LMX_MSG_STATE_STOPPED) {
        SwitchToThread();
    }
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

static int run_nested(Lmx *node) {
    LmxMsgRuntime *rt;
    LmxMsgAddr p = 0, c = 0, sib = 0, g = 0;
    SpinCtx ctx;
    int st;
    LmxMsgEnv e;
    uchar ini = 1;
    /* p is R0 (spin_boot's first create on a fresh runtime); no lane cancels
     * R0 mid-turn (turn_parent_spin's own comment), so c -- spin_boot's
     * lawfully mapped child, bound with turn_spin -- carries the spin, and
     * the host cancels c once p's own (now brief) entry turn returns. */
    if (reset_fields(node) != 0 || spin_boot(&rt, &p, &c, &sib, &g, &ctx, node, turn_spin) != 0) {
        fprintf(stderr, "spin-nested boot\n");
        return 1;
    }
    memset(&e, 0, sizeof(e));
    e.kind = LMX_MSG_KIND_BYTES;
    e.n = 1;
    e.bytes = &ini;
    if (lmx_msg_host_post(rt, p, &e) != LMX_MSG_STAGED) {
        return fail_rt(rt, "spin-nested post parent");
    }
    st = lmx_msg_run_entry_turn(rt, p, turn_parent_spin, &ctx);
    if (st != LMX_MSG_OK) {
        return fail_rt(rt, "spin-nested entry turn");
    }
    /* p (R0) is never cancelled -- it just received its posted mail and
     * returned, still running; only its lawfully mapped child c is
     * cancelled below (check_aftermath's own expect_parent_live). */
    /* M: yield rounds reading flags, no wall clock: the child's L2 loop is
     * entered, then the cancel stops it (same pattern as run_map's own). */
    fprintf(stderr, "reading: spin-nested: the child's inner loop entered\n");
    fflush(stderr);
    while (field_at(node, 0U) != 1) {
        SwitchToThread();
    }
    if (lmx_msg_emergency_cancel(rt, c) != LMX_MSG_OK) {
        return fail_rt(rt, "spin-nested cancel");
    }
    fprintf(stderr, "reading: spin-nested: the cancelled child stopped\n");
    fflush(stderr);
    while (lmx_msg_state(rt, c) != LMX_MSG_STATE_STOPPED) {
        SwitchToThread();
    }
    if (check_aftermath(rt, p, c, sib, g, &ctx, node, "spin-nested", 1) != 0) {
        return fail_rt(rt, "spin-nested aftermath");
    }
    lmx_msg_exec_stop(rt);
    lmx_msg_runtime_delete(rt);
    fprintf(stderr, "cancel_spin_parent ok hit=%d after=%d parent_live=1\n",
        field_at(node, 0U), field_at(node, 1U));
    return 0;
}

/* S4 guard acceptance (LOCK_REMOVAL_S4_SITES.txt site 8): a lane that is
 * neither the target's parent's own lane nor the host outside any turn --
 * a spawned thread, settled onto neither -- must be refused. Written
 * red-first against emergency_cancel's own missing caller-identity check;
 * green now that the guard (mapping_authority_locked, reused directly)
 * lands in lmx_msg_emergency_cancel. */
typedef struct WrongLaneArg {
    LmxMsgRuntime *rt;
    LmxMsgAddr who;
    volatile LONG got;
} WrongLaneArg;

static DWORD WINAPI wrong_lane_cancel_worker(void *arg) {
    WrongLaneArg *a = (WrongLaneArg *)arg;
    InterlockedExchange(&a->got, (LONG)lmx_msg_emergency_cancel(a->rt, a->who));
    return 0;
}

static int run_s4_guard_emergency_cancel(void) {
    LmxMsgRuntime *rt;
    LmxMsgAddr p = 0, c = 0;
    uchar ini = 1;
    WrongLaneArg arg;
    HANDLE th;
    int got;
    rt = lmx_msg_runtime_new();
    if (rt == 0 || lmx_msg_create(rt, 0, &ini, 1, &p) != LMX_MSG_OK
        || lmx_msg_create(rt, p, &ini, 1, &c) != LMX_MSG_OK
        || lmx_msg_end_turn(rt, p, 1) != LMX_MSG_OK) {
        return fail_rt(rt, "s4 guard emergency_cancel: boot");
    }
    memset(&arg, 0, sizeof(arg));
    arg.rt = rt;
    arg.who = c;
    arg.got = (LONG)LMX_MSG_INVALID;
    /* This spawned thread is not c's parent p's own lane (p never runs a
     * turn here) and not the host outside any turn (main() is a different
     * thread): the guard's own two legitimate callers, neither. */
    th = CreateThread(0, 0, wrong_lane_cancel_worker, &arg, 0, 0);
    if (th == 0 || join_canceler(th, rt, "s4 guard emergency_cancel: thread") != 0) {
        return 1;
    }
    got = (int)InterlockedCompareExchange(&arg.got, 0, 0);
    lmx_msg_exec_stop(rt);
    lmx_msg_runtime_delete(rt);
    if (got != LMX_MSG_INVALID) {
        fprintf(stderr,
            "S4 guard check FAILED: emergency_cancel from neither the parent's lane nor "
            "the host outside any turn returned %d, want LMX_MSG_INVALID=%d "
            "(mapping_authority_locked's own guard missing or broken)\n",
            got, LMX_MSG_INVALID);
        return 1;
    }
    printf("s4 guard emergency_cancel ok (refused from a spawned thread)\n");
    return 0;
}

int main(void) {
    Lmx *node = make_int_node();
    if (node == 0) {
        fprintf(stderr, "spin node\n");
        (void)lmx_msg_blocks_dispose_all(&g_graph_blocks);
        g_graph_ranges = 0;
        return 1;
    }
    if (run_map(node) != 0 || run_nested(node) != 0 || run_s4_guard_emergency_cancel() != 0) {
        (void)lmx_msg_blocks_dispose_all(&g_graph_blocks);
        g_graph_ranges = 0;
        return 1;
    }
    (void)lmx_msg_blocks_dispose_all(&g_graph_blocks);
    g_graph_ranges = 0;
    printf("cancel_spin ok\n");
    return 0;
}
