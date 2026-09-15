#ifndef LMX_MESSAGE_EXEC_H
#define LMX_MESSAGE_EXEC_H

#include "l2src/lmx_message.h"

#if defined(LMX_MSG_EXEC_TEST)
void lmx_msg_test_on_admit(LmxMsgAddr dest, const LmxMsgCopy *fresh);
void lmx_msg_test_release_tree(LmxMsgRuntime *rt, LmxMsg *m);
void lmx_msg_test_lane_write(LmxMsgRuntime *rt, LmxMsg *owner, const char *site);
void lmx_msg_test_unbind_refused(LmxMsgRuntime *rt, LmxMsg *m, int st, const char *site);
extern int lmx_msg_test_lane_check;
void lmx_msg_test_wake_site(const char *site, unsigned owner);
#else
#define lmx_msg_test_wake_site(s, o) ((void)0)
#define lmx_msg_test_on_admit(d, f) ((void)0)
#define lmx_msg_test_release_tree(r, m) ((void)0)
#define lmx_msg_test_lane_write(r, o, s) ((void)0)
#define lmx_msg_test_unbind_refused(r, m, st, s) ((void)(st))
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
int lmx_msg_exec_start_contexts(LmxMsgRuntime *rt);
int lmx_msg_exec_contexts_live(LmxMsgRuntime *rt);
int lmx_msg_exec_ui_step(LmxMsgRuntime *rt);
int lmx_msg_exec_stop(LmxMsgRuntime *rt);
void lmx_msg_exec_drop_binds(LmxMsgRuntime *rt);
void lmx_msg_exec_set_no_retire(LmxMsgRuntime *rt, int v);
void lmx_msg_exec_ready(LmxMsgRuntime *rt, LmxMsgAddr addr);
int lmx_msg_arena_collect(LmxMsg *m);
int lmx_msg_exec_is_bound(LmxMsgRuntime *rt, LmxMsgAddr addr);
int lmx_msg_exec_unbound_close(LmxMsgRuntime *rt, LmxMsgAddr addr);
/* Last run_one status for this binding. A later turn overwrites it.
 * Not a per-request supervisor/result channel. */
int lmx_msg_exec_last_status(LmxMsgRuntime *rt, LmxMsgAddr addr);
int lmx_msg_exec_is_runnable(LmxMsgRuntime *rt, LmxMsgAddr addr);
int lmx_msg_exec_is_runnable_locked(LmxMsgRuntime *rt, LmxMsgAddr addr);
void lmx_msg_exec_flush_retire(LmxMsgRuntime *rt);
unsigned lmx_msg_exec_take_ui_locked(LmxMsgRuntime *rt);
int lmx_msg_exec_ui_request_locked(LmxMsgRuntime *rt, LmxMsgAddr addr);
int lmx_msg_exec_route_locked(LmxMsg *m, int *ui, int *pool);
int lmx_msg_exec_msg_bound(LmxMsg *m);
int lmx_msg_exec_adopt_mark(LmxMsgRuntime *rt, LmxMsgAddr parent, LmxMsgAddr child);
int lmx_msg_exec_dispose_mark(LmxMsgRuntime *rt, LmxMsgAddr parent, LmxMsgAddr child);
int lmx_msg_exec_reclaim_mark(LmxMsgRuntime *rt, LmxMsg *m);
/* D1 allocation walk of rt->slots. */
int lmx_msg_exec_tab_n_locked(LmxMsgRuntime *rt);
LmxMsgAddr lmx_msg_exec_tab_addr_locked(LmxMsgRuntime *rt, int i);
unsigned lmx_msg_exec_take_addr(LmxMsgRuntime *rt);
#if defined(LMX_MSG_EXEC_TEST)
void lmx_msg_exec_test_set_fail_ctx(LmxMsgRuntime *rt, int v);
void lmx_msg_exec_test_set_fail_adopt_block(LmxMsgRuntime *rt, int v);
int lmx_msg_exec_map_queued(LmxMsgRuntime *rt, LmxMsgAddr addr);
int lmx_msg_exec_ui_nrequests(LmxMsgRuntime *rt);
int lmx_msg_exec_retire_n(LmxMsgRuntime *rt);
#endif
#if defined(LMX_MSG_EXEC_TEST)
extern void (*lmx_msg_exec_test_after_cleanup)(LmxMsgAddr who, int live, int st);
extern void (*lmx_msg_exec_test_after_bind_add)(LmxMsgRuntime *rt);
extern void (*lmx_msg_exec_test_during_release_tree)(LmxMsgRuntime *rt, LmxMsg *m);
extern void (*lmx_msg_test_mail_locked)(LmxMsg *m);
extern void (*lmx_msg_test_after_outbox_xfer)(LmxMsgRuntime *rt, LmxMsg *src, LmxMsgCopy *outb);
extern void (*lmx_msg_test_after_recv_pin)(LmxMsgRuntime *rt, LmxMsg *m);
extern void (*lmx_msg_test_after_drive_snap)(LmxMsgRuntime *rt, LmxMsg *p);
extern int lmx_msg_test_fail_retain;
extern int lmx_msg_test_fail_post_dead;
int lmx_msg_test_stage(LmxMsgRuntime *rt, LmxMsgAddr from, LmxMsgAddr to, unsigned id);
int lmx_msg_exec_bind_n(LmxMsgRuntime *rt);
int lmx_msg_exec_bind_aff(LmxMsgRuntime *rt, LmxMsgAddr addr);
int lmx_msg_exec_bind_has_worker(LmxMsgRuntime *rt, LmxMsgAddr addr);
#endif

#endif
