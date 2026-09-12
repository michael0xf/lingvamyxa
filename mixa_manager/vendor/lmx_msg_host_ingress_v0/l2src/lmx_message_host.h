/* LMX_MSG_HOST_INGRESS_V0 platform isolation. Not a general Message lock. */
#ifndef LMX_MESSAGE_HOST_H
#define LMX_MESSAGE_HOST_H

#include "l2src/lmx_message.h"

int lmx_msg_host_attach(LmxMsgRuntime *rt);
void lmx_msg_host_detach(LmxMsgRuntime *rt);
int lmx_msg_host_is_owner(LmxMsgRuntime *rt);
int lmx_msg_host_lock(LmxMsgRuntime *rt);
void lmx_msg_host_unlock(LmxMsgRuntime *rt);
int lmx_msg_host_is_shutdown(LmxMsgRuntime *rt);
int lmx_msg_host_wake(LmxMsgRuntime *rt);
int lmx_msg_host_shutdown(LmxMsgRuntime *rt);

/* Production: get_nomem is an immutable 0 stub. Setters exist only
 * in -DLMX_MSG_HOST_TEST builds. */
int lmx_msg_host_test_get_nomem(void);
#if defined(LMX_MSG_HOST_TEST)
extern int lmx_msg_host_test_nomem;
extern int lmx_msg_host_test_nowake;
void lmx_msg_host_test_set_nomem(int v);
void lmx_msg_host_test_set_nowake(int v);
#endif

#endif
