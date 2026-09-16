/* lmx_message.h - first copy-only Message participant slice (19.29.7.1).
 *
 * Language names: create / send / stop. English "spawn" is not a parser
 * intrinsic. Send stages a transport-owned copy; the receiver materializes
 * a fresh copy at FIFO admission. Staging is not admission or execution.
 */
#ifndef LMX_MESSAGE_H
#define LMX_MESSAGE_H

struct Lmx;

#include "l2src/lmx_msg_blocks.lm1.h"
#include "l2src/lmx_owned_ranges.lm1.h"
#include "l2src/lmx_msg_storage.lm1.h"

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
/* S6-2: string address not deliverable yet (lmx_msg_send_address's body today). */
#define LMX_MSG_UNDELIVERABLE 9

#define LMX_MSG_KIND_NUMBER 0
#define LMX_MSG_KIND_BYTES 1
#define LMX_MSG_KIND_PROGRESS 2
#define LMX_MSG_KIND_ITEM 3
#define LMX_MSG_KIND_CANCELLED 4
#define LMX_MSG_KIND_DEAD 5
#define LMX_MSG_KIND_DONE 6
#define LMX_MSG_KIND_REJECTED 7
#define LMX_MSG_KIND_STOP 8
/* KIND_GRAPH: ordinary delivery of LMX text (SPEC 19.29.7): the envelope's
 * graph field carries the root of a Message created by the copier in its own
 * arena; recv moves that storage into the handler's arena. */
#define LMX_MSG_KIND_GRAPH 9
/* Stage 5 (b): an internal envelope in the root Message's inbox, a host post
 * waiting for the root's drain; `to` is its destination and ingress_kind its
 * own kind. recv never hands it out and readiness never counts it; lifecycle
 * reads (retire, close) do. */
#define LMX_MSG_KIND_INGRESS 11
/* S6-2 (SPEC 19.29.7): a registration letter to the mail service.  A child is
 * created on its PARENT's lane, so it cannot write the service's live set
 * itself; it posts this instead, carrying the new record in dest_msg, and the
 * service registers the address when it drains.  Kinds 0-11 were all taken, so
 * this is a new constant rather than a reused slot -- recorded as my choice,
 * the same way LMX_MSG_KIND_REJECTED's use was.
 * The letter is pushed BEFORE create returns the address, which with the FIFO
 * transport is what makes a registration precede any send that could have
 * learned the handle. */
#define LMX_MSG_KIND_REGISTER 12
/* S6-2 (SPEC 19.29.7, the removal ruling): an unregister-and-free letter to the
 * mail service, carrying the record in dest_msg and its id in to.  release_slot
 * runs on the RELEASING lane -- the parent's dispose, or the Message's own
 * end-turn -- so it can neither write the service's live set (a cross-lane write)
 * nor free the record itself (which would leave a live-set entry naming freed
 * memory until the service next drains).  It posts this instead; the service
 * removes the entry and calls slot_free, in that order, on its own lane.
 * The window between the release and that free is closed by what release_slot
 * already does: the state is RELEASED, written under the mailbox's monitor, and
 * admit_one refuses a RELEASED destination under that same monitor, so a send
 * arriving in the window is refused rather than admitted. */
#define LMX_MSG_KIND_UNREGISTER 13
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

/* Versioned host-ingress seam. Not a promise that create/send/pump/recv
 * are multi-thread safe. Only lmx_msg_host_post may run off the owner thread. */
#define LMX_MSG_HOST_INGRESS_VERSION 0
#define LMX_MSG_HOST_FROM 0U

typedef unsigned LmxMsgAddr;
typedef struct LmxMsgRuntime LmxMsgRuntime;
typedef int (*LmxMsgTurn)(LmxMsgRuntime *rt, LmxMsgAddr who, void *ctx);

/* Owner-local explicit root bookkeeping. Not an Lmx header field and not a
 * global/TLS registry. Nodes retain a native referent across end_turn;
 * release drops retention only and never disposes the referent. */
#define LMX_MSG_ROOT_RETAIN 1u
#define LMX_MSG_ROOT_HISTORY 2u

typedef struct LmxMsgRoot {
    void *p;
    unsigned roles;
    struct LmxMsgRoot *next;
} LmxMsgRoot;

typedef struct LmxMsgEnv {
    unsigned id;
    unsigned correlation;
    LmxMsgAddr reply_to;
    int kind;
    int number;
    const uchar *bytes;
    size_t n;
    struct Lmx *graph;
} LmxMsgEnv;

typedef struct LmxMsgCopy {
    unsigned id;
    unsigned correlation;
    LmxMsgAddr reply_to;
    LmxMsgAddr from;
    LmxMsgAddr to;
    int kind;
    /* Stage 5 (b): the envelope's own kind while kind is LMX_MSG_KIND_INGRESS. */
    int ingress_kind;
    int number;
    uchar *bytes;
    size_t n;
    int owned;
    struct LmxMsg *dest_msg;
    struct LmxMsg *delivered;
    struct LmxMsgCopy *next;
} LmxMsgCopy;

typedef struct LmxMsg {
    LmxMsgAddr addr;
    /* S6-2 (SPEC 19.29.7, Mikhail 2026-09-16, thirteenth line): THE C MIRROR OF AN
     * LMX FIELD, NOT A KERNEL FIELD.  The mail service's reference belongs to the
     * Message's LMX -- "поле уже LMX, а вот API надо сделать общим" -- and the
     * kernel record is running, success, handoff_safe, root, index.  Where the
     * bootstrap C keeps the mirror is an implementation detail; it is here, with
     * the same status as the other scaffolding fields the plan lists, and it goes
     * when the record is reduced.  Set from the parent at creation and
     * reassignable; R0's points at R0, which terminates delegation.  The precedent
     * for an LMX field is rt->root_record. */
    struct LmxMsg *service;
    /* The service's own live set: the records it will admit a letter to, sorted by
     * address as uintptr_t so membership is a binary search rather than a scan.
     * Registration at create, removal at release, on the service's own lane.  An
     * address absent from this set is REFUSED, never dereferenced -- which is what
     * makes delivery by memory address safe with no count of holders, and is this
     * stage's replacement for refs: one owner's data on one lane, instead of a
     * counter on every record written from every lane.  Only the service reads or
     * writes these; they are 0 on every Message that is not one.
     * EACH ENTRY IS A PAIR, the record pointer AND its id, and membership requires
     * BOTH to match: malloc reuses addresses, so a handle that outlived its record
     * could otherwise name a NEW record at the same address and pass the check.
     * The id (LmxMsgAddr) is minted monotonically and never reused, which is what
     * makes the pair decisive; stage AD keeps it for exactly this reason.
     * The pair is held as TWO PARALLEL ARRAYS rather than an array of structs, and
     * the reason is a measurement rather than a preference: neither core indexes an
     * array of structs and follows a field through the subscript anywhere (0 sites
     * against a control of 18 plain subscripts), so that spelling has no neighbour
     * to copy and would be verifiable only by a build.  live[i] and live_id[i] are
     * both forms the cores already use.  They are grown and shifted together and
     * always have the same length; live_n and live_cap describe both. */
    struct LmxMsg **live_m;
    LmxMsgAddr *live_id;
    /* AD (2026-09-16): the ID order, the same pair in a second order.  SPEC
     * 19.29.7 makes a capability "the target's id" and requires that a Message is
     * found by its id "in bounded time, never by scanning one by one" -- the
     * address order above answers "is this handle live", and this one answers
     * "which record does this id name" without a walk of R0's tree.  Ids are
     * minted monotonically and never reused, so the two orders differ and both are
     * needed.  byid_m[k] and byid_id[k] are one entry of the same set as
     * live_m[i]/live_id[i]; the three arrays are grown together and register and
     * unregister maintain both orders, so live_n and live_cap describe all of
     * them.  A slot whose record was freed is left in this order with byid_m 0 and
     * answers 0, which is the refusal the caller wants. */
    struct LmxMsg **byid_m;
    LmxMsgAddr *byid_id;
    int live_n;
    int live_cap;
    /* S6-2 (SPEC 19.29.7, Mikhail 2026-09-16): this Message's index at its parent
     * -- "the whole of what the hierarchical address a.b.c.d... adds at this
     * level".  The full address is composed by walking the parent links
     * (lmx_msg_get_address), so no array is stored: the arrays this replaces were
     * a materialized cache of that walk.  R0's index is 1 and its parent_msg is 0,
     * which terminates the walk. */
    unsigned index;
    int state;
    int committed;
    int closing;
    uchar *init;
    size_t init_n;
    LmxMsgCopy *inbox;
    LmxMsgCopy *inbox_tail;
    LmxMsgCopy *outbox;
    LmxMsgCopy *outbox_tail;
    unsigned *done_from;
    unsigned *done_id;
    int done_n;
    int done_cap;
    unsigned exec_id;
    unsigned exec_corr;
    LmxMsgAddr exec_from;
    LmxMsgAddr exec_reply;
    int exec_live;
    unsigned last_beat;
    unsigned child_seq;
    /* Direct-child list (CONTEXT_V0). Not a process-wide registry. */
    struct LmxMsg *parent_msg;
    struct LmxMsg *first_child;
    struct LmxMsg *last_child;
    struct LmxMsg *next_sibling;
    /* Decision 18 (2026-09-14): readiness is this Message's own control flag.
     * lmx_msg_exec_ready sets it (the sender at admission, the closing
     * requester, the bind kick); the lane that takes this Message's turn
     * clears it. Nothing is appended to a parent's cells from another lane. */
    int ready;
    LmxMsgTurn turn;
    void *turn_ctx;
    /* Stage 3a (L2_RUNTIME_PLAN_20260914.md): the executor's bind record is
     * this Message's own state; the executor's table only indexes it. */
    struct LmxMsgExecBind *exec_bind;
    int mapped;
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
    /* Decision 17 (spec 19.29.6 (iii)): re-rooted at the runtime because it was
     * still running when its parent was settled. */
    int orphan;
    int disposed;
    /* S6-2 (SPEC 19.29.7, respecified by Mikhail 2026-09-16): there is no
     * runtime-wide registry cell here and no owner's list of closed records
     * either.  A closed Message's slot and arena are freed by the release chain
     * as 19.29.6 says, so nothing needs to enumerate closed records: they do not
     * outlive their release.  A capability is the target's id, not a pointer, so
     * no sender holds this record and none has to be kept alive for one. */
    LmxMsgBlock *blocks;
    LmxOwnedRange *ranges;
    /* Message-owned, non-owning classification metadata for explicitly
     * admitted independent:const:immutable values.  Payload remains owned by
     * blocks/ranges; each metadata entry names exactly one typed value. */
    LmxOwnedRange *eternal_ranges;
    /* Separate non-owning classifier for root-Message-owned shared METHOD
     * records.  A copied callable keeps the descriptor address; this metadata
     * lets a later Message copy classify that address without claiming its
     * storage or conflating METHOD with an eternal branch. */
    LmxOwnedRange *method_ranges;
    struct Lmx *graph;
    LmxMsgRoot *roots;
} LmxMsg;

struct LmxMsgRuntime {
    LmxMsg *root;
    /* S6-2 (19.28 Revision 2): the slot list "served only the L1 delete loop and
     * its address lookup and goes without replacement". The lookup is the family
     * walk (lmx_msg_find -> find_tree from rt->root), and the delete loop is R0's
     * own storage -- its settled records and its tree -- walked by teardown. */
    LmxMsgCopy *transport;
    LmxMsgCopy *transport_tail;
    void *host_sync;
    void *exec;
    unsigned next_addr;
    /* Stage 5 (e): R0's policy record (l2src/lmx_root_record.lm2), Structure data
     * in R0's arena: the clock, its test flag and a failed orphan's retention
     * (decision 17, LMX_MSG_ORPHAN_RETAIN by default), written only on R0's lane.
     * 0 until the first policy set on R0's lane creates it (runtime_new cannot call
     * the generated unit, whose library open creates a runtime); readers take the
     * defaults while it is 0: the real clock, LMX_MSG_ORPHAN_RETAIN. */
    struct Lmx *root_record;
};

#define LMX_MSG_ORPHAN_RETAIN 30000U

LmxMsgRuntime *lmx_msg_runtime_new(void);
void lmx_msg_runtime_delete(LmxMsgRuntime *rt);

int lmx_msg_create(LmxMsgRuntime *rt, LmxMsgAddr parent, const uchar *init, size_t n, LmxMsgAddr *out);
/* Stage 5 (d1): declared because runtime_new, earlier in lmx_message, creates R0 through it. */
int lmx_msg_create_prepare(LmxMsgRuntime *rt, LmxMsgAddr parent,
                           struct Lmx *source, LmxOwnedRange *src_ranges,
                           LmxOwnedRange *eternal_ranges, LmxOwnedRange *method_ranges,
                           const uchar *init, size_t n, LmxMsgAddr *out);
/* Create with an explicit used-graph copy. The new Message stays private until
 * the complete copy and path preparation succeed; failure publishes no child. */
int lmx_msg_create_graph(LmxMsgRuntime *rt, LmxMsgAddr parent,
                         struct Lmx *source, LmxOwnedRange *src_ranges,
                         LmxOwnedRange *eternal_ranges,
                         LmxOwnedRange *method_ranges,
                         const uchar *init, size_t n, LmxMsgAddr *out);
int lmx_msg_send(LmxMsgRuntime *rt, LmxMsgAddr from, LmxMsgAddr to, const LmxMsgEnv *env);
int lmx_msg_send_graph(LmxMsgRuntime *rt, LmxMsgAddr from, LmxMsgAddr to, const LmxMsgEnv *env, struct Lmx *graph);
LmxMsg *lmx_msg_delivery_new(struct Lmx *graph, LmxOwnedRange *src_ranges, LmxOwnedRange *eternal_ranges, LmxOwnedRange *method_ranges);
void lmx_msg_delivery_dispose(LmxMsg *d);
int lmx_msg_delivery_receive(LmxMsg *m, LmxMsgCopy *node, LmxMsgEnv *out);
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
int lmx_msg_drive_tree(LmxMsgRuntime *rt, LmxMsg *m);
int lmx_msg_drive_walk_children(LmxMsgRuntime *rt, LmxMsg *m);
int lmx_msg_drive_walk_roots(LmxMsgRuntime *rt);
/* S6-2 (SPEC 19.29.7, Mikhail's fourteenth line): the COMMON MAIL API, written
 * now as scaffolding "because rewriting them later would be expensive".  Every
 * L3 Thread implements these; the default body is the stub that reads its own
 * service reference and delegates, and R0 carries the real body today.  A later
 * stage replaces bodies without touching a caller.
 *   register / unregister: the service's live set gains or loses a record.
 *   send_handle: deliver by the target's memory address -- the handle the sender
 *     already holds -- admitting under that mailbox's own monitor, or refusing
 *     with KIND_REJECTED when the address is not in the live set.
 * lmx_msg_service_of returns the service a Message delegates to (itself, when it
 * is the service). */
LmxMsg *lmx_msg_service_of(LmxMsg *m);
int lmx_msg_service_register(LmxMsg *svc, LmxMsg *m, LmxMsgAddr id);
int lmx_msg_service_unregister(LmxMsg *svc, LmxMsg *m, LmxMsgAddr id);
/* Membership requires BOTH the pointer and the id: a reused address with a
 * different id is not the record the sender meant, and is refused. */
int lmx_msg_service_is_live(LmxMsg *svc, LmxMsg *m, LmxMsgAddr id);
/* S6-2 (SPEC 19.29.7): compose a Message's hierarchical address by walking the
 * parent links to the root.  Returns the number of indices, or -1 if the Message
 * is not reachable or its chain changed between the count and the fill (a
 * handoff or an orphaning on another lane); the fill never writes past cap.  Fills out[0..n-1] root-first when out is non-zero and cap is
 * at least n; with a smaller cap (or out == 0) it returns the count and writes
 * nothing, so a caller can size its buffer first.  Nothing is allocated, so there
 * is nothing to free -- the shape lmx_msg_poll already uses. */
int lmx_msg_get_address(LmxMsgRuntime *rt, LmxMsgAddr who, unsigned *out, int cap);
/* S6-2 (SPEC 19.29.7): the common API's second entry, send by string address
 * (addr[0..addr_n-1], root-first, the shape lmx_msg_get_address fills).  The
 * default body delegates to the sender's service.  R0's body today refuses: a
 * KIND_REJECTED letter to the sender and LMX_MSG_UNDELIVERABLE returned -- the
 * hop-by-hop chain waits for the stage whose executor runs every holder's round. */
int lmx_msg_service_send_address(LmxMsgRuntime *rt, LmxMsg *svc, LmxMsgAddr from, const unsigned *addr, int addr_n, const LmxMsgEnv *env);
int lmx_msg_send_address(LmxMsgRuntime *rt, LmxMsgAddr from, const unsigned *addr, int addr_n, const LmxMsgEnv *env);
int lmx_msg_child_n(LmxMsgRuntime *rt, LmxMsgAddr who);
LmxMsgAddr lmx_msg_child_at(LmxMsgRuntime *rt, LmxMsgAddr who, int i);
/* Decision 17 rule 4: hand the supervision of old_parent's direct child to the
 * live new_parent. Mailbox, arena, turn and record stay; parent_msg,
 * scheduler place and liveness window move, and the index is minted afresh by
 * new_parent (S6-2, SPEC 19.29.7: "when a Message changes parent its address
 * changes"), so lmx_msg_get_address reads the new parent's address plus it. */
int lmx_msg_handoff_supervision(LmxMsgRuntime *rt, LmxMsgAddr old_parent, LmxMsgAddr child, LmxMsgAddr new_parent);

int lmx_msg_runtime_shutdown(LmxMsgRuntime *rt);
int lmx_msg_host_post(LmxMsgRuntime *rt, LmxMsgAddr dest, const LmxMsgEnv *env);
int lmx_msg_host_drain(LmxMsgRuntime *rt);
int lmx_msg_host_wait(LmxMsgRuntime *rt, unsigned timeout_ms);

#define LMX_MSG_AFFINITY_ANY 0
int lmx_msg_exec_bind(LmxMsgRuntime *rt, LmxMsgAddr addr, LmxMsgTurn turn, void *ctx, int affinity);
/* Stage 5 step (a): generated units call these two. The entry adapter refuses
 * outside its own Message's turn; the bootstrap runs that turn. */
int lmx_msg_exec_holding_turn(LmxMsgRuntime *rt, LmxMsgAddr who);
int lmx_msg_run_entry_turn(LmxMsgRuntime *rt, LmxMsgAddr addr, LmxMsgTurn turn, void *ctx);
/* Stage 5 (d1): R0, the runtime's root Message, and one turn of it on this thread. */
unsigned lmx_msg_root_addr(LmxMsgRuntime *rt);
int lmx_msg_root_turn(LmxMsgRuntime *rt, LmxMsgTurn turn, void *ctx);
int lmx_msg_exec_unbind(LmxMsgRuntime *rt, LmxMsgAddr addr);
int lmx_msg_exec_start_contexts(LmxMsgRuntime *rt);
int lmx_msg_exec_stop(LmxMsgRuntime *rt);
void lmx_msg_exec_drop_binds(LmxMsgRuntime *rt);
void lmx_msg_exec_set_no_retire(LmxMsgRuntime *rt, int v);
/* Stage 5 (e): R0's policy record, l2src/lmx_root_record.lm2, an L2 runtime unit
 * (real symbols, linked by every runner that links the executor). Declared here
 * so the L1 core and the executor call it through c. without a generated header;
 * the unit's own C includes this header, so a drift in these signatures fails
 * its compile. */
struct Lmx *lmx_root_record_new(LmxMsg *owner);
unsigned lmx_root_record_clock(struct Lmx *rec);
int lmx_root_record_set_clock(struct Lmx *rec, unsigned now);
int lmx_root_record_clock_test(struct Lmx *rec);
int lmx_root_record_set_clock_test(struct Lmx *rec, int on);
unsigned lmx_root_record_orphan_retain(struct Lmx *rec);
int lmx_root_record_set_orphan_retain(struct Lmx *rec, unsigned retain);
int lmx_msg_map_child(LmxMsgRuntime *rt, LmxMsgAddr parent, LmxMsgAddr child);
int lmx_msg_live_query(LmxMsgRuntime *rt, LmxMsgAddr who);
int lmx_msg_live_handle(LmxMsgRuntime *rt, LmxMsgAddr who, const LmxMsgEnv *env);
int lmx_msg_live_check(LmxMsgRuntime *rt, LmxMsgAddr who, unsigned now, unsigned threshold);
int lmx_msg_live_test_set_seq(LmxMsgRuntime *rt, LmxMsgAddr who, unsigned v);
int lmx_msg_live_test_set_wait_th(LmxMsgRuntime *rt, LmxMsgAddr who, unsigned th);
int lmx_msg_set_orphan_retain(LmxMsgRuntime *rt, unsigned retain);
unsigned lmx_msg_now(LmxMsgRuntime *rt);
/* S5: the next Message address from the runtime's order-free atomic counter. */
LmxMsgAddr lmx_msg_addr_take(LmxMsgRuntime *rt);
/* S6-2 (SPEC 19.29.7, "there is no count of holders"): endp_retain, endp_release,
 * endp_try_retire and endp_refs are deleted, and so is LmxMsg.refs itself. The
 * count answered exactly one question -- "can this record be freed under me?" --
 * and the settle answers it instead: a closing Message becomes its parent's
 * storage, and nothing frees a record except R0's teardown. */
int lmx_msg_child_unlink(LmxMsg *parent, LmxMsg *child);
int lmx_msg_send_cap(LmxMsgRuntime *rt, LmxMsgAddr from, LmxMsg *dest, const LmxMsgEnv *env);
void lmx_msg_mail_lock(LmxMsg *m);
void lmx_msg_mail_unlock(LmxMsg *m);
int lmx_msg_mail_inbox_empty(LmxMsg *m);
int lmx_msg_mail_inbox_n(LmxMsg *m);
/* Stage 5 (b): readiness reads skip internal kinds; lifecycle reads use
 * lmx_msg_mail_inbox_empty. pop_input unlinks the first non-internal node;
 * take_ingress unlinks every INGRESS node in admission order. */
int lmx_msg_mail_inbox_has_input(LmxMsg *m);
LmxMsgCopy *lmx_msg_mail_inbox_pop_input(LmxMsg *m);
void lmx_msg_mail_inbox_take_ingress(LmxMsg *m, LmxMsgCopy **out);
void lmx_msg_mail_inbox_take(LmxMsg *m, LmxMsgCopy **out);
int lmx_msg_mail_outbox_empty(LmxMsg *m);
void lmx_msg_mail_outbox_take(LmxMsg *m, LmxMsgCopy **out);
void lmx_msg_mail_inbox_prepend(LmxMsg *m, LmxMsgCopy *chain);
void lmx_msg_after_outbox_xfer(LmxMsgRuntime *rt, LmxMsg *src, LmxMsgCopy *outb);
void lmx_msg_after_recv_pin(LmxMsgRuntime *rt, LmxMsg *m);
int lmx_msg_test_post_dead_fail(void);
LmxMsg *lmx_msg_turn_self(LmxMsgRuntime *rt);
/* O1: the turn Message and its arena, reached without a runtime handle. The
 * allocations are blocks of the turn Message's own arena; 0 outside a turn. */
LmxMsg *lmx_msg_turn_current(void);
void *lmx_msg_turn_new_zero(size_t size);
char *lmx_msg_turn_copy_bytes(const char *source, size_t length);
void lmx_msg_slot_free(LmxMsg *m);
/* Integer resolver for remaining addr APIs. Runtime-owned endpoint list, not a directory. */
LmxMsg *lmx_msg_find(LmxMsgRuntime *rt, LmxMsgAddr addr);
void lmx_msg_set_graph(LmxMsg *m, struct Lmx *unit);
struct Lmx *lmx_msg_graph(LmxMsg *m);
/* Bootstrap-only mutation: call while the initial Message graph is being
 * built under exclusive ownership, before user turns can observe it. */
int lmx_msg_bootstrap_eternal_admit(LmxMsg *owner, void *address);
LmxOwnedRange *lmx_msg_eternal_ranges(LmxMsg *owner);
int lmx_msg_bootstrap_method_admit(LmxMsg *owner, void *address);
LmxOwnedRange *lmx_msg_method_ranges(LmxMsg *owner);
/* Internal Message-create seam. Clones classification records only; payload
 * and the root retention array are neither copied nor exposed. */
int lmx_msg_eternal_clone(LmxMsg *dest, LmxOwnedRange *source);
int lmx_msg_method_clone(LmxMsg *dest, LmxOwnedRange *source);
int lmx_msg_root_attach(LmxMsg *m, void *p);
int lmx_msg_root_release(LmxMsg *m, void *p);
LmxMsg *lmx_msg_self_or_find(LmxMsgRuntime *rt, LmxMsgAddr addr);
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
int lmx_msg_transfer_adopted(LmxMsgRuntime *rt, LmxMsgAddr from, LmxMsgAddr to);
/* Move a handoff-safe successful direct child's existing arena into its parent
 * without copying and retain exactly the selected owned graph root there.
 * The source control record remains until dispose_child; its moved graph and
 * owner-local roots are cleared by the transfer. */
int lmx_msg_transfer_graph(LmxMsgRuntime *rt, LmxMsgAddr from, LmxMsgAddr to,
                           struct Lmx *root);
/* Ordinary local delivery of a completed, handoff-safe graph to any live
 * recipient in the same runtime.  The existing blocks/ranges move without a
 * copy or pointer rewrite; the source keeps its original lifecycle parent but
 * is no longer an active tracked assignment.  The recipient does not become
 * the source Message's supervisor. */
int lmx_msg_deliver_graph(LmxMsgRuntime *rt, LmxMsgAddr from, LmxMsgAddr to,
                          struct Lmx *root);
int lmx_msg_dispose_child(LmxMsgRuntime *rt, LmxMsgAddr parent, LmxMsgAddr child);
int lmx_msg_parent_settle(LmxMsgRuntime *rt, LmxMsgAddr parent);
int lmx_msg_send_owned(LmxMsgRuntime *rt, LmxMsgAddr from, LmxMsgAddr to, LmxMsgEnv *env);
int lmx_msg_set_orphan_until(LmxMsgRuntime *rt, LmxMsgAddr who, unsigned until);
int lmx_msg_orphan_expired(LmxMsgRuntime *rt, LmxMsgAddr who, unsigned now);
#if defined(LMX_MSG_HOST_TEST) || defined(LMX_MSG_EXEC_TEST)
extern int lmx_msg_test_copy_fail;
int lmx_msg_test_copy_should_fail(void);
void lmx_msg_test_set_copy_fail(int n);
extern int lmx_msg_test_root_alloc_fail;
int lmx_msg_test_root_alloc_should_fail(void);
void lmx_msg_test_set_root_alloc_fail(int n);
#else
#define lmx_msg_test_copy_should_fail() 0
#define lmx_msg_test_root_alloc_should_fail() 0
#endif

#endif
