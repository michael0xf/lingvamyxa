/* lmx_message.h - first copy-only Message participant slice (19.29.7.1).
 *
 * Language names: create / send / stop. English "spawn" is not a parser
 * intrinsic. Send stages a transport-owned copy; the receiver materializes
 * a fresh copy at FIFO admission. Staging is not admission or execution.
 */
#ifndef LMX_MESSAGE_H
#define LMX_MESSAGE_H

#include <stddef.h>

typedef unsigned char uchar;

#define LMX_MSG_OK 0
#define LMX_MSG_STAGED 1
#define LMX_MSG_INVALID 2
#define LMX_MSG_NOMEM 3
#define LMX_MSG_STOPPED 4
#define LMX_MSG_DEAD 5
#define LMX_MSG_DUPLICATE 6
#define LMX_MSG_GONE 7
#define LMX_MSG_EMPTY 8

#define LMX_MSG_KIND_NUMBER 0
#define LMX_MSG_KIND_BYTES 1
#define LMX_MSG_KIND_PROGRESS 2
#define LMX_MSG_KIND_ITEM 3
#define LMX_MSG_KIND_CANCELLED 4
#define LMX_MSG_KIND_DEAD 5
#define LMX_MSG_KIND_DONE 6
#define LMX_MSG_KIND_REJECTED 7
#define LMX_MSG_KIND_STOP 8
/* KIND_STOP is internal close control. KIND_CANCELLED is ordinary result data. */

#define LMX_MSG_STATE_INACTIVE 0
#define LMX_MSG_STATE_RUNNING 1
#define LMX_MSG_STATE_STOPPED 2
#define LMX_MSG_STATE_DEAD 3
#define LMX_MSG_STATE_RELEASED 4

/* Prototype path storage. Not a language-level depth or width limit. */
#define LMX_MSG_PATH_CAP 16

/* Versioned host-ingress seam. Not a promise that create/send/pump/recv
 * are multi-thread safe. Only lmx_msg_host_post may run off the owner thread. */
#define LMX_MSG_HOST_INGRESS_VERSION 0
#define LMX_MSG_HOST_FROM 0U

typedef unsigned LmxMsgAddr;

typedef struct LmxMsgEnv {
    unsigned id;
    unsigned correlation;
    LmxMsgAddr reply_to;
    int kind;
    int number;
    const uchar *bytes;
    size_t n;
} LmxMsgEnv;

typedef struct LmxMsgCopy {
    unsigned id;
    unsigned correlation;
    LmxMsgAddr reply_to;
    LmxMsgAddr from;
    LmxMsgAddr to;
    int kind;
    int number;
    uchar *bytes;
    size_t n;
    struct LmxMsgCopy *next;
} LmxMsgCopy;

typedef struct LmxMsg {
    LmxMsgAddr addr;
    LmxMsgAddr parent;
    unsigned create_id;
    int state;
    int committed;
    int closing;
    uchar *init;
    size_t init_n;
    LmxMsgCopy *inbox;
    LmxMsgCopy *inbox_tail;
    LmxMsgCopy *outbox;
    LmxMsgCopy *outbox_tail;
    unsigned done_from[32];
    unsigned done_id[32];
    int done_n;
    unsigned exec_id;
    unsigned exec_corr;
    LmxMsgAddr exec_from;
    LmxMsgAddr exec_reply;
    int exec_live;
    unsigned last_beat;
    unsigned path[LMX_MSG_PATH_CAP];
    int path_n;
    unsigned child_seq;
} LmxMsg;

typedef struct LmxMsgRuntime {
    LmxMsg **tab;
    int n;
    int cap;
    LmxMsgCopy *transport;
    LmxMsgCopy *transport_tail;
    LmxMsgCopy *host_head;
    LmxMsgCopy *host_tail;
    void *host_sync;
    void *exec;
    unsigned next_addr;
    unsigned clock;
    unsigned root_seq;
} LmxMsgRuntime;

LmxMsgRuntime *lmx_msg_runtime_new(void);
void lmx_msg_runtime_delete(LmxMsgRuntime *rt);

int lmx_msg_create(LmxMsgRuntime *rt, LmxMsgAddr parent, unsigned create_id, const uchar *init, size_t n, LmxMsgAddr *out);
int lmx_msg_send(LmxMsgRuntime *rt, LmxMsgAddr from, LmxMsgAddr to, const LmxMsgEnv *env);
int lmx_msg_stop(LmxMsgRuntime *rt, LmxMsgAddr from, LmxMsgAddr to);
int lmx_msg_end_turn(LmxMsgRuntime *rt, LmxMsgAddr who, int success);
int lmx_msg_pump(LmxMsgRuntime *rt);
int lmx_msg_recv(LmxMsgRuntime *rt, LmxMsgAddr who, LmxMsgEnv *out);
void lmx_msg_env_release(LmxMsgEnv *env);
int lmx_msg_fail(LmxMsgRuntime *rt, LmxMsgAddr who);
int lmx_msg_state(LmxMsgRuntime *rt, LmxMsgAddr who);
int lmx_msg_inbox_n(LmxMsgRuntime *rt, LmxMsgAddr who);
int lmx_msg_init_copy(LmxMsgRuntime *rt, LmxMsgAddr who, LmxMsgEnv *out);
int lmx_msg_set_now(LmxMsgRuntime *rt, unsigned now);
int lmx_msg_poll(LmxMsgRuntime *rt, LmxMsgAddr who, unsigned now, unsigned threshold, LmxMsgAddr *out, int cap);
int lmx_msg_drive(LmxMsgRuntime *rt, unsigned now, unsigned threshold);
int lmx_msg_path_n(LmxMsgRuntime *rt, LmxMsgAddr who);
int lmx_msg_path_seg(LmxMsgRuntime *rt, LmxMsgAddr who, int i, unsigned *out);

int lmx_msg_runtime_shutdown(LmxMsgRuntime *rt);
int lmx_msg_host_post(LmxMsgRuntime *rt, LmxMsgAddr dest, const LmxMsgEnv *env);
int lmx_msg_host_drain(LmxMsgRuntime *rt);
int lmx_msg_host_wait(LmxMsgRuntime *rt, unsigned timeout_ms);

#define LMX_MSG_AFFINITY_ANY 0
#define LMX_MSG_AFFINITY_UI 1
typedef int (*LmxMsgTurn)(LmxMsgRuntime *rt, LmxMsgAddr who, void *ctx);
int lmx_msg_exec_bind(LmxMsgRuntime *rt, LmxMsgAddr addr, LmxMsgTurn turn, void *ctx, int affinity);
int lmx_msg_exec_start(LmxMsgRuntime *rt, int nworkers);
int lmx_msg_exec_ui_step(LmxMsgRuntime *rt);
int lmx_msg_exec_stop(LmxMsgRuntime *rt);

#endif
