/* lmx_message.h - first copy-only Message participant slice (19.29.7.1).
 *
 * Language names: create / send / stop. English "spawn" is not a parser
 * intrinsic. Send stages a transport-owned copy; the receiver materializes
 * a fresh copy at FIFO admission. Staging is not admission or execution.
 */
#ifndef LMX_MESSAGE_H
#define LMX_MESSAGE_H

#include <stddef.h>
#include <stdint.h>

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
/* KIND_STOP is internal close control. KIND_CANCELLED is ordinary result data.
 * Implementation-only liveness profile on KIND_PROGRESS. Not language KINDs.
 * Ordinary progress number 1/2 must not match these. */
#define LMX_MSG_PROF_LIVE_Q 0x4C560001
#define LMX_MSG_PROF_LIVE_R 0x4C560002

#define LMX_MSG_STATE_INACTIVE 0
#define LMX_MSG_STATE_RUNNING 1
#define LMX_MSG_STATE_STOPPED 2
#define LMX_MSG_STATE_DEAD 3
#define LMX_MSG_STATE_RELEASED 4

/* Initial Mix-path chunk. Growable; not a language-level depth limit. */
#define LMX_MSG_PATH_CHUNK 4

/* Versioned host-ingress seam. Not a promise that create/send/pump/recv
 * are multi-thread safe. Only lmx_msg_host_post may run off the owner thread. */
#define LMX_MSG_HOST_INGRESS_VERSION 0
#define LMX_MSG_HOST_FROM 0U

typedef unsigned LmxMsgAddr;
typedef struct LmxMsgRuntime LmxMsgRuntime;
typedef int (*LmxMsgTurn)(LmxMsgRuntime *rt, LmxMsgAddr who, void *ctx);

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
    struct LmxMsg *dest_msg;
    struct LmxMsgCopy *next;
} LmxMsgCopy;

typedef struct LmxAdopted {
    void *base;
    size_t n;
    struct LmxAdopted *next;
} LmxAdopted;

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
    unsigned *path;
    int path_n;
    int path_cap;
    unsigned child_seq;
    /* Direct-child list (CONTEXT_V0). Not a process-wide registry. */
    struct LmxMsg *parent_msg;
    struct LmxMsg *first_child;
    struct LmxMsg *last_child;
    struct LmxMsg *next_sibling;
    LmxMsgTurn turn;
    void *turn_ctx;
    int mapped;
    int refs;
    uint_fast8_t running;
    uint_fast8_t success;
    int tracked;
    struct LmxMsgRuntime *owner_rt;
    void *mail;
    unsigned live_wait_th;
    unsigned live_query_id;
    unsigned live_query_at;
    unsigned live_query_pend_id;
    unsigned live_query_pend_at;
    unsigned live_seq;
    unsigned child_heard_at;
    int handoff_ready;
    int native_users;
    unsigned orphan_until;
    int retain_history;
    struct LmxAdopted *adopted;
} LmxMsg;

struct LmxMsgRuntime {
    LmxMsg *root;
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
    int clock_test;
    unsigned root_seq;
};

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
int lmx_msg_child_n(LmxMsgRuntime *rt, LmxMsgAddr who);
LmxMsgAddr lmx_msg_child_at(LmxMsgRuntime *rt, LmxMsgAddr who, int i);

int lmx_msg_runtime_shutdown(LmxMsgRuntime *rt);
int lmx_msg_host_post(LmxMsgRuntime *rt, LmxMsgAddr dest, const LmxMsgEnv *env);
int lmx_msg_host_drain(LmxMsgRuntime *rt);
int lmx_msg_host_wait(LmxMsgRuntime *rt, unsigned timeout_ms);

#define LMX_MSG_AFFINITY_ANY 0
#define LMX_MSG_AFFINITY_UI 1
int lmx_msg_exec_bind(LmxMsgRuntime *rt, LmxMsgAddr addr, LmxMsgTurn turn, void *ctx, int affinity);
int lmx_msg_exec_unbind(LmxMsgRuntime *rt, LmxMsgAddr addr);
int lmx_msg_exec_start(LmxMsgRuntime *rt, int nworkers);
int lmx_msg_exec_start_contexts(LmxMsgRuntime *rt);
int lmx_msg_exec_ui_step(LmxMsgRuntime *rt);
int lmx_msg_exec_stop(LmxMsgRuntime *rt);
void lmx_msg_exec_drop_binds(LmxMsgRuntime *rt);
void lmx_msg_exec_set_no_retire(LmxMsgRuntime *rt, int v);
int lmx_msg_sched_step(LmxMsgRuntime *rt, LmxMsgAddr parent);
int lmx_msg_map_child(LmxMsgRuntime *rt, LmxMsgAddr parent, LmxMsgAddr child);
int lmx_msg_run_child_turn(LmxMsgRuntime *rt, LmxMsgAddr child);
int lmx_msg_live_query(LmxMsgRuntime *rt, LmxMsgAddr who);
int lmx_msg_live_handle(LmxMsgRuntime *rt, LmxMsgAddr who, const LmxMsgEnv *env);
int lmx_msg_live_check(LmxMsgRuntime *rt, LmxMsgAddr who, unsigned now, unsigned threshold);
int lmx_msg_live_test_set_seq(LmxMsgRuntime *rt, LmxMsgAddr who, unsigned v);
int lmx_msg_live_test_set_wait_th(LmxMsgRuntime *rt, LmxMsgAddr who, unsigned th);
unsigned lmx_msg_now(LmxMsgRuntime *rt);
int lmx_msg_endp_retain(LmxMsg *m);
void lmx_msg_endp_release(LmxMsg *m);
int lmx_msg_endp_refs(LmxMsgRuntime *rt, LmxMsgAddr who);
int lmx_msg_endp_try_retire(LmxMsgRuntime *rt, LmxMsg *m);
int lmx_msg_send_cap(LmxMsgRuntime *rt, LmxMsgAddr from, LmxMsg *dest, const LmxMsgEnv *env);
void lmx_msg_mail_lock(LmxMsg *m);
void lmx_msg_mail_unlock(LmxMsg *m);
void lmx_msg_slot_free(LmxMsg *m);
/* Integer resolver for remaining addr APIs. Runtime-owned endpoint list, not a directory. */
LmxMsg *lmx_msg_find(LmxMsgRuntime *rt, LmxMsgAddr addr);
uint_fast8_t lmx_msg_running_load(const LmxMsg *m);
void lmx_msg_running_store(LmxMsg *m, uint_fast8_t v);
uint_fast8_t lmx_msg_success_load(const LmxMsg *m);
void lmx_msg_success_store(LmxMsg *m, uint_fast8_t v);
int lmx_msg_emergency_cancel(LmxMsgRuntime *rt, LmxMsgAddr who);
int lmx_msg_poll_abort(void);
int lmx_msg_complete(LmxMsgRuntime *rt, LmxMsgAddr who);
int lmx_msg_tracked(LmxMsgRuntime *rt, LmxMsgAddr who);
int lmx_msg_handoff_ready(LmxMsgRuntime *rt, LmxMsgAddr who);
int lmx_msg_native_users(LmxMsgRuntime *rt, LmxMsgAddr who);
int lmx_msg_adopt_failed(LmxMsgRuntime *rt, LmxMsgAddr parent, LmxMsgAddr child);
int lmx_msg_adopted_n(LmxMsgRuntime *rt, LmxMsgAddr who);
void *lmx_msg_adopted_base(LmxMsgRuntime *rt, LmxMsgAddr who, int i);
int lmx_msg_drop_adopted(LmxMsgRuntime *rt, LmxMsgAddr who);
int lmx_msg_retain_history(LmxMsgRuntime *rt, LmxMsgAddr who);
int lmx_msg_set_orphan_until(LmxMsgRuntime *rt, LmxMsgAddr who, unsigned until);
int lmx_msg_orphan_expired(LmxMsgRuntime *rt, LmxMsgAddr who, unsigned now);
#if defined(LMX_MSG_HOST_TEST) || defined(LMX_MSG_EXEC_TEST)
extern int lmx_msg_test_copy_fail;
int lmx_msg_test_copy_should_fail(void);
void lmx_msg_test_set_copy_fail(int n);
#else
#define lmx_msg_test_copy_should_fail() 0
#endif

#endif
