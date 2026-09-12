#ifndef LMX_MESSAGE_EXEC_H
#define LMX_MESSAGE_EXEC_H

#include "l2src/lmx_message.h"

#if defined(LMX_MSG_EXEC_TEST)
void lmx_msg_test_on_admit(LmxMsgAddr dest, const LmxMsgCopy *fresh);
#else
#define lmx_msg_test_on_admit(d, f) ((void)0)
#endif
int lmx_msg_exec_attach(LmxMsgRuntime *rt);
void lmx_msg_exec_detach(LmxMsgRuntime *rt);
int lmx_msg_exec_lock(LmxMsgRuntime *rt);
void lmx_msg_exec_unlock(LmxMsgRuntime *rt);
int lmx_msg_exec_holding_turn(LmxMsgRuntime *rt, LmxMsgAddr who);
int lmx_msg_exec_workers(LmxMsgRuntime *rt);
int lmx_msg_exec_holding_any(LmxMsgRuntime *rt);
int lmx_msg_tab_grow(LmxMsgRuntime *rt);
LmxMsg *lmx_msg_slot_new(void);
int lmx_msg_run_child_turn(LmxMsgRuntime *rt, LmxMsgAddr child);
int lmx_msg_map_child(LmxMsgRuntime *rt, LmxMsgAddr parent, LmxMsgAddr child);
int lmx_msg_exec_bind(LmxMsgRuntime *rt, LmxMsgAddr addr, LmxMsgTurn turn, void *ctx, int affinity);
int lmx_msg_exec_unbind(LmxMsgRuntime *rt, LmxMsgAddr addr);
int lmx_msg_exec_start(LmxMsgRuntime *rt, int nworkers);
int lmx_msg_exec_start_contexts(LmxMsgRuntime *rt);
int lmx_msg_exec_ui_step(LmxMsgRuntime *rt);
int lmx_msg_exec_stop(LmxMsgRuntime *rt);
void lmx_msg_exec_drop_binds(LmxMsgRuntime *rt);
void lmx_msg_exec_set_no_retire(LmxMsgRuntime *rt, int v);
void lmx_msg_exec_ready(LmxMsgRuntime *rt, LmxMsgAddr addr);
int lmx_msg_arena_collect(LmxMsg *m);
void lmx_msg_sched_enqueue_child(LmxMsg *parent, LmxMsg *child);
LmxMsg *lmx_msg_sched_dequeue_child(LmxMsg *parent);
void lmx_msg_sched_unlink_child(LmxMsg *parent, LmxMsg *child);
int lmx_msg_exec_is_bound(LmxMsgRuntime *rt, LmxMsgAddr addr);
int lmx_msg_exec_unbound_close(LmxMsgRuntime *rt, LmxMsgAddr addr);
/* Last run_one status for this binding. A later turn overwrites it.
 * Not a per-request supervisor/result channel. */
int lmx_msg_exec_last_status(LmxMsgRuntime *rt, LmxMsgAddr addr);
int lmx_msg_exec_is_runnable(LmxMsgRuntime *rt, LmxMsgAddr addr);
int lmx_msg_exec_is_runnable_locked(LmxMsgRuntime *rt, LmxMsgAddr addr);
int lmx_msg_exec_ready_has_locked(LmxMsgRuntime *rt, LmxMsgAddr addr);
int lmx_msg_exec_ready_try_push_locked(LmxMsgRuntime *rt, LmxMsgAddr addr);
int lmx_msg_exec_map_try_enqueue_locked(LmxMsgRuntime *rt, LmxMsgAddr addr);
void lmx_msg_exec_map_unlink_locked(LmxMsgRuntime *rt, LmxMsgAddr addr);
unsigned lmx_msg_exec_take_map_locked(LmxMsgRuntime *rt);
int lmx_msg_exec_bind_launching_locked(LmxMsgRuntime *rt, int i);
void lmx_msg_exec_set_scan_locked(LmxMsgRuntime *rt, int v);
int lmx_msg_exec_get_scan_locked(LmxMsgRuntime *rt);
void lmx_msg_exec_wake_locked(LmxMsgRuntime *rt);
void lmx_msg_exec_wake_addr_locked(LmxMsgRuntime *rt, LmxMsgAddr addr);
/* D1 allocation walk of rt->slots. Not used by scan_ready. */
int lmx_msg_exec_tab_n_locked(LmxMsgRuntime *rt);
LmxMsgAddr lmx_msg_exec_tab_addr_locked(LmxMsgRuntime *rt, int i);
void lmx_msg_exec_scan_ready(LmxMsgRuntime *rt);
unsigned lmx_msg_exec_take_addr(LmxMsgRuntime *rt, int want_ui);
int lmx_msg_exec_nready_locked(LmxMsgRuntime *rt);
LmxMsgAddr lmx_msg_exec_ready_at_locked(LmxMsgRuntime *rt, int i);
void lmx_msg_exec_ready_remove_locked(LmxMsgRuntime *rt, int i);
int lmx_msg_exec_ui_nready_locked(LmxMsgRuntime *rt);
LmxMsgAddr lmx_msg_exec_ui_ready_at_locked(LmxMsgRuntime *rt, int i);
void lmx_msg_exec_ui_ready_remove_locked(LmxMsgRuntime *rt, int i);
int lmx_msg_exec_ui_ready_has_locked(LmxMsgRuntime *rt, LmxMsgAddr addr);
int lmx_msg_exec_ui_ready_try_push_locked(LmxMsgRuntime *rt, LmxMsgAddr addr);
void lmx_msg_exec_ready_remove_addr_locked(LmxMsgRuntime *rt, LmxMsgAddr addr);
void lmx_msg_exec_ui_ready_remove_addr_locked(LmxMsgRuntime *rt, LmxMsgAddr addr);
int lmx_msg_exec_bind_n_locked(LmxMsgRuntime *rt);
LmxMsgAddr lmx_msg_exec_bind_addr_locked(LmxMsgRuntime *rt, int i);
int lmx_msg_exec_bind_aff_locked(LmxMsgRuntime *rt, int i);
int lmx_msg_exec_bind_has_worker_locked(LmxMsgRuntime *rt, int i);
int lmx_msg_exec_bind_held_locked(LmxMsgRuntime *rt, int i);
void lmx_msg_exec_bind_set_held_locked(LmxMsgRuntime *rt, int i, int held);
void lmx_msg_exec_drop_stale_ready(LmxMsgRuntime *rt);
#if defined(LMX_MSG_EXEC_TEST)
void lmx_msg_exec_test_set_fail_grow(LmxMsgRuntime *rt, int v);
void lmx_msg_exec_test_set_fail_ctx(LmxMsgRuntime *rt, int v);
void lmx_msg_exec_test_set_fail_adopt_block(LmxMsgRuntime *rt, int v);
void lmx_msg_exec_test_set_fail_start_kicks(LmxMsgRuntime *rt, int v);
int lmx_msg_exec_test_fail_hits(LmxMsgRuntime *rt);
int lmx_msg_exec_get_scan(LmxMsgRuntime *rt);
int lmx_msg_exec_ready_has(LmxMsgRuntime *rt, LmxMsgAddr addr);
int lmx_msg_exec_ready_cap(LmxMsgRuntime *rt);
int lmx_msg_exec_nready(LmxMsgRuntime *rt);
int lmx_msg_exec_ui_nready(LmxMsgRuntime *rt);
int lmx_msg_exec_map_queued(LmxMsgRuntime *rt, LmxMsgAddr addr);
int lmx_msg_exec_map_nready(LmxMsgRuntime *rt);
int lmx_msg_exec_bind_cap(LmxMsgRuntime *rt);
int lmx_msg_exec_test_overflow_grow(LmxMsgRuntime *rt, int bind);
#endif
#if defined(LMX_MSG_EXEC_TEST)
extern void (*lmx_msg_exec_test_after_cleanup)(LmxMsgAddr who, int live, int st);
extern void (*lmx_msg_exec_test_after_bind_add)(LmxMsgRuntime *rt);
int lmx_msg_exec_bind_n(LmxMsgRuntime *rt);
int lmx_msg_exec_bind_aff(LmxMsgRuntime *rt, LmxMsgAddr addr);
int lmx_msg_exec_bind_has_worker(LmxMsgRuntime *rt, LmxMsgAddr addr);
#endif

#endif
