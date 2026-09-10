/* LMX_MSG_HOST_INGRESS_V0 platform isolation. Not a general Message lock. */
#ifndef LMX_MESSAGE_HOST_H
#define LMX_MESSAGE_HOST_H

#include "l2src/lmx_message.h"

int lmx_msg_host_attach(LmxMsgRuntime *rt);
void lmx_msg_host_detach(LmxMsgRuntime *rt);
int lmx_msg_host_is_owner(LmxMsgRuntime *rt);
LmxMsgCopy *lmx_msg_host_steal(LmxMsgRuntime *rt);
int lmx_msg_host_shutdown(LmxMsgRuntime *rt);

#endif
