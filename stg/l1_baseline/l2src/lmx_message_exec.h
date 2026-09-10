#ifndef LMX_MESSAGE_EXEC_H
#define LMX_MESSAGE_EXEC_H

#include "l2src/lmx_message.h"

int lmx_msg_exec_attach(LmxMsgRuntime *rt);
void lmx_msg_exec_detach(LmxMsgRuntime *rt);
int lmx_msg_exec_lock(LmxMsgRuntime *rt);
void lmx_msg_exec_unlock(LmxMsgRuntime *rt);
int lmx_msg_exec_holding_turn(LmxMsgRuntime *rt, LmxMsgAddr who);
int lmx_msg_exec_workers(LmxMsgRuntime *rt);
int lmx_msg_exec_holding_any(LmxMsgRuntime *rt);
int lmx_msg_tab_grow(LmxMsgRuntime *rt);
LmxMsg *lmx_msg_slot_new(void);
int lmx_msg_exec_bind(LmxMsgRuntime *rt, LmxMsgAddr addr, LmxMsgTurn turn, void *ctx, int affinity);
int lmx_msg_exec_start(LmxMsgRuntime *rt, int nworkers);
int lmx_msg_exec_ui_step(LmxMsgRuntime *rt);
int lmx_msg_exec_stop(LmxMsgRuntime *rt);
void lmx_msg_exec_ready(LmxMsgRuntime *rt, LmxMsgAddr addr);

#endif
