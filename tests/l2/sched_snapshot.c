#include "l2src/lmx_message.h"
#include "l2src/lmx_message_exec.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Only exec.c is compiled with malloc redirected here. */
static int fail_alloc;
static int hook_calls;
static int checks;
static int failures;
/* Admission observer required by the reused Exec-instrumented Message object. */
void lmx_msg_test_on_admit(LmxMsgAddr destination, const LmxMsgCopy *fresh) {
    (void)destination; (void)fresh;
}
void *lmx_sched_snapshot_test_malloc(size_t bytes) {
    if (fail_alloc) { fail_alloc = 0; return NULL; }
    return malloc(bytes);
}
static void check(int ok, const char *label) {
    ++checks;
    if (!ok) { ++failures; fprintf(stderr, "FAIL %s\n", label); }
}
static void snap_hook(LmxMsgRuntime *rt, LmxMsg *parent) {
    (void)rt; (void)parent; ++hook_calls;
}
static int turn(LmxMsgRuntime *rt, LmxMsgAddr who, void *context) {
    LmxMsgEnv e;
    int st;
    memset(&e, 0, sizeof(e));
    st = lmx_msg_recv(rt, who, &e);
    if (st != LMX_MSG_OK) return st;
    lmx_msg_env_release(&e);
    ++*(int *)context;
    return lmx_msg_end_turn(rt, who, 1);
}
static int run_case(size_t count) {
    LmxMsgRuntime *rt = lmx_msg_runtime_new();
    LmxMsgAddr parent = 0, last = 0, first = 0;
    LmxMsg *p, *child;
    LmxMsgEnv e;
    unsigned selected = 0;
    unsigned char initial = 0;
    int done = 0, refs, child_refs, st;
    size_t i;
    if (!rt || lmx_msg_create(rt, 0, 1, &initial, 1, &parent) != LMX_MSG_OK
        || lmx_msg_end_turn(rt, parent, 1) != LMX_MSG_OK) return 2;
    for (i = 0; i < count; ++i) {
        if (lmx_msg_create(rt, parent, (unsigned)i + 2, &initial, 1, &last) != LMX_MSG_OK
            || lmx_msg_exec_bind(rt, last, turn, &done, LMX_MSG_AFFINITY_ANY) != LMX_MSG_OK) return 2;
        if (!i) first = last;
    }
    if (lmx_msg_end_turn(rt, parent, 1) != LMX_MSG_OK) return 2;
    p = lmx_msg_find(rt, parent);
    child = lmx_msg_find(rt, first);
    refs = p->refs;
    child_refs = child->refs;
    if (count == 1) {
        check(lmx_msg_sched_pick_host_child(rt, parent, NULL) == LMX_MSG_INVALID, "null output is invalid");
        selected = 123;
        check(lmx_msg_sched_pick_host_child(rt, UINT_MAX, &selected) == LMX_MSG_INVALID && selected == 0, "missing parent is invalid");
        fail_alloc = 1;
        check(lmx_msg_sched_pick_host_child(rt, parent, &selected) == LMX_MSG_NOMEM && selected == 0, "allocation failure is NOMEM");
        fail_alloc = 0;
        hook_calls = 0;
        lmx_msg_test_after_sched_snap = snap_hook;
        lmx_msg_test_fail_retain = 1;
        st = lmx_msg_sched_pick_host_child(rt, parent, &selected);
        lmx_msg_test_fail_retain = 0;
        lmx_msg_test_after_sched_snap = NULL;
        check(st == LMX_MSG_NOMEM && selected == 0, "parent retain failure is NOMEM");
        check(hook_calls == 0 && p->refs == refs && child->refs == child_refs, "failed parent retain skips hook and preserves refs");
        /* Saturation uses the real retain refusal, after the parent pin succeeds. */
        child->refs = INT_MAX;
        st = lmx_msg_sched_pick_host_child(rt, parent, &selected);
        child->refs = child_refs;
        check(st == LMX_MSG_NOMEM && selected == 0 && p->refs == refs, "child retain failure releases parent");
    }
    check(lmx_msg_sched_step(rt, parent) == LMX_MSG_EMPTY, "all empty children yield EMPTY");
    memset(&e, 0, sizeof(e)); e.kind = LMX_MSG_KIND_BYTES; e.id = 1;
    if (lmx_msg_host_post(rt, last, &e) != LMX_MSG_STAGED || lmx_msg_host_drain(rt) != LMX_MSG_OK) return 2;
    st = lmx_msg_sched_step(rt, parent);
    check(st == LMX_MSG_OK && done == 1, "last eligible child runs");
    check(lmx_msg_sched_step(rt, parent) == LMX_MSG_EMPTY && done == 1, "no duplicate run");
    check(p->refs == refs && child->refs == child_refs, "snapshot pins balanced");
    printf("children=%zu done=%d status=%d\n", count, done, st);
    lmx_msg_runtime_delete(rt);
    return 0;
}
int main(void) {
    const size_t counts[] = {1, 65, 129, 257};
    size_t i;
    for (i = 0; i < sizeof(counts) / sizeof(counts[0]); ++i)
        if (run_case(counts[i])) return 2;
    printf("snapshot: %d checks, %d failures\n", checks, failures);
    return failures ? 1 : 0;
}
