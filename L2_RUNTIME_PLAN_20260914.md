# L2 runtime plan (2026-09-14)

Owner: review chat (`lingvamyxa-e2`, Fable 5.1). Confirmed by Mikhail on
2026-09-14 (LEAD_REVIEW_20260914.md §6, decisions 15 and 16). This replaces
§6.4 of CORE_LEAD_INSTRUCTION_20260913_en.md as the runtime lane's plan.

## 1. What changed

The runtime lane so far ported the L1 runtime modules to L2 one by one, each
as a library unit with a parity runner against the unchanged L1 module. That
work proved the translator expresses the runtime (fifteen units, seven
translator gaps found and closed, RUNTIME_L2_PORTS.txt), and the runners are
the mechanism by which core code moves from L1 to L2.

Mikhail's ruling, in two parts. (a) The target is the core described in
L2_CORE_AND_MESSAGE_MODEL_20260912.md and the specification; intermediate L1
implementations are not permanent, and wherever the L1 prototype differs from
the model (the number/byte envelope, the exec host V0 table, the flag order
before decision 14) the model wins. (b) Since L2 is translated into L1 first,
the core may be finished in L1, where the form is freer, and then moved to
L2 by the port machinery; there is no confusion between the two: L1 is the
workbench and the bootstrap that spec 19.29.6 allows, L2 is where the core
lives when done. Acceptance of the core itself is the model's scenarios and
the spec's own checks; acceptance of each move to L2 is parity with the L1
module that was brought to the model first.

## 2. What the specification requires (the sources)

- 19.28.R2.2 Message Thread: management state is ordinary Structure data
  implemented in L2, not an external C control model. Each live Message
  owns its FIFO inbox/outbox, a parent capability (except the root) and the
  list of its direct children. The control flags (running, success) belong
  to the Message. No shared global Message registry, no global management
  lock, no language scheduler above Messages; each parent contains the
  scheduler for its direct children as private Structure data. Native
  thread, wait/wake and per-mailbox primitives support Message and do not
  own its policy or family tree.
- 19.29.6 Running Message (L3 Thread): one logical arena, FIFO queues, one
  serial execution lane, at most one turn at a time; different Messages may
  execute concurrently; concurrent senders are linearized at the recipient's
  FIFO admission; OS process startup is already the root Message. Its
  implementation/integration checks are the acceptance of this plan.
- 19.29.7 and 19.29.7.1 Message boundary: ordinary delivery adds LMX text to
  the receiver's one arena with ownership transfer, no deep copy, no identity
  rewrite. "Copy-only" is merge's copy and the Message's own arena, true for
  every Message (clarification added 2026-09-14). The number/byte envelope,
  the host ingress V0 and the exec host V0 are prototype history.
- Model L2_CORE_AND_MESSAGE_MODEL_20260912.md §29-§36: parent-owned
  scheduling, running/success and stop (§31, with decision 14: complete sets
  only success; running=0 follows success=1 only at the turn boundary, set
  by whoever executed the Message), family close (§32), liveness (§33),
  failure handoff (§34), the executed-argument-as-own bind and callable
  recursion.
- 9.1.4 eternal branches, 21 name resolution: unchanged and not part of this
  lane.

## 3. What is built, in stages

Mikhail (2026-09-14): the level at which a stage is first written is the
review chat's choice. Each stage is written in L1 where that is faster (the
workbench) and moved to L2 by a parity runner, or directly in L2 where that
is no slower; in L2 it is a unit with the `profile: runtime` directive
(decision 14: the executor's own core is never polled; checkpoint
diagnostics stay). Every stage is committed when it lands (decision 16). A
stage is accepted by the model's scenarios and the spec's own checks; the
L1 selftests are regression evidence for the bootstrap, not acceptance of
the core. The order below follows the gaps between the model and the L1
prototype, largest first.

1. **Message and L3 Thread as Structure data.** Mikhail (2026-09-14), the
   two kinds and their relation: a Message is simply an isolated LMX graph.
   An L3 Thread is a Message that can attach and/or execute any Message;
   every L3 Thread is a Message, not every Message is an L3 Thread (a
   template and a letter have no turn, a launched child does). The mailbox
   and the provision for a turn exist only at the L3 Thread, even when it is
   executed from outside and no OS thread of its own is started; therefore
   only the varieties of L3 Thread carry a scheduler. So, two families:
   - **Message**: its arena (blocks and ranges, one owner), the running and
     success flags (every Message has them, spec 19.29.7.1), and the
     capability of its owner/parent.
   - **L3 Thread**: a Message that additionally holds the FIFO inbox and
     staged outbox, the turn state, the list of its direct children and the
     scheduling of what it executes: its launched children and the Messages
     it attached after receiving them (the consumer schedules those itself,
     including executing one as part of itself), with the worker mapping and
     ready queues, all as its own isolated graph in its own arena. Mikhail
     (2026-09-14): that scheduler does not manage launched children as a
     whole. Its management is only the allocation of execution (selecting a
     child's turn) and removal on timeout, judged by the checks that come
     from the children themselves. The primary mechanism, as agreed with
     Astra: the child polls the parent, not the other way round, and closes
     itself when the parent has been absent too long. Closing a child from
     above is the second, emergency mechanism. In the normal mode children
     close themselves, and each child schedules its own children the same
     way (model §29, §33; spec 19.28.R2.2, 19.29.7.1). The executor's
     per-thread C state in exec.c (LmxMsgExec, bind table, map queues) and
     the per-Message mailbox/turn fields of LmxMsg are the prototype of this
     family. Creation by merge only (a copy, its own arena; 19.29.6 "creation
   through merge"). No integer address table: a delivery address is a
   capability the sender holds.
2. **Mailbox and delivery.** send stages a Message created by merge in the
   sender's outbox; publication at the sender's successful end-turn; admission
   transfers the arena into the recipient's inbox, FIFO, serialized against
   other admissions (the only place native synchronization lives); recv at
   turn entry materializes it into the handler's own arena by ownership
   transfer. Dedup key (recipient, sender, id), late replies rejected after
   stop, queue pressure as admission failure, as 19.29.7.1 lists them.
3. **Turns and the parent's scheduler.** One serial lane per Message; the
   parent selects a child's whole turn before another (19.28.R2.2 sequential
   mapping); a child may map its own children to workers. The escape
   (running=0 with success=0) unwinds only a requested stop. complete sets
   success; the turn boundary clears running from success. Physical workers
   are the parent's mapping, reached through `c.` (threads, events, waits
   stay C behind their existing functions).

   Design (2026-09-14, after stages 1-2 and the five core tests).
   *Today, the prototype (exec host V0, "evidence, not the accepted
   architecture", spec 19.29.7.1):* one `LmxMsgExec` per runtime holds a
   global lock, a global bind table (addr, turn function, ctx, affinity,
   held/held_by, launching, gone, a context record `LmxMsgBindWait` with the
   thread and its event), global owner-ready lists for ANY and UI (the
   parents whose children are ready), a retire list, and thread-local state
   (the turn Message, the turn root, the running-flag pointer). Contexts are
   one thread per bound non-UI Message; each loops take_ready (a child of a
   ready owner, through the global lists and the bind table) and run_one
   (turn root, body, boundary, native_leave, requeue). Per-Message state
   already exists on `LmxMsg`: the mailbox and its lock, turn/turn_ctx/
   mapped, parent and direct children, the parent-local readiness list of
   children (`sched_ready`, module lmx_msg_sched_ready, ported), the
   membership fields of the global lists (map_*), refs/native_users/
   handoff_ready, the liveness timers, running and success.
   *Target (19.28.R2.2, 19.29.6, model §25 and §29):* each running Message
   is an L3 Thread Message: its own mailbox and turn state, its own direct
   children, and its own scheduler state for those children as Structure
   data: readiness (exists), the mapping of children to physical workers
   (today the global bind table and contexts), the parent-local wake
   primitive (today the global ready event). No global bind table, no global
   owner-ready lists, no global management lock for policy; the runtime's
   internal synchronization stays where the model allows it (mailbox
   admission, lifecycle, transfer). Native threads, events, TLS and the
   setjmp turn root remain C and are called through `c.`.
   *Steps, each keeping run_lmx -Suite Message and the five core tests green
   and committed on its own:*
   - 3a. Bind state moves onto the child Message: affinity, held, held_by,
     last_st, launching, gone and the context record become fields of
     `LmxMsg` beside turn/turn_ctx/mapped; bind_index lookups become field
     reads; bind_grow goes; the table survives only as an enumeration index
     for start_contexts and drop_stale, then goes.
   - 3b. Owner-ready lists become per parent: a parent's ready children are
     its sched_ready list plus a per-parent "ready for a physical worker"
     signal; contexts belong to the parent that mapped the child (a
     parent-owned context list); the global map_own lists and the global
     ready event go; unbound children keep running on the parent's thread
     through sched_step.
   - 3c. The per-parent scheduler record (context list, mapping policy,
     ready heads) becomes an L2 Structure allocated in the parent's arena and
     driven by L2 code with own fields; exec.c's C structs become that
     Structure's cells. This is the port step (a `profile: runtime` unit),
     verified by parity against 3b.
   - 3d. UI affinity is a mapping policy of the parent that owns the UI
     worker (an L3 Thread whose lane is the UI thread), not a global class.
     Design (2026-09-14, after decision 18 landed; shape fixed with the
     lead the same day): the UI lane is a Message of its own, created lazily
     by the executor at the first UI bind, outside the family lists and the
     slot count until stage 5 makes it the root Message's child, whose lane
     is the UI thread and whose mailbox is the only way work reaches it. No
     lane writes a child into any UI structure: the writer of a child's
     readiness (exec_ready: the sender at admission, the closing requester,
     the bind kick) sends a mapping request under the parent's policy (the
     child's bind affinity until the policy cell of lmx_sched_record takes
     over) to the UI lane's mailbox, as an admission (class 4): an internal
     control envelope like KIND_STOP carrying the child's address, never
     handler-visible work, one outstanding request per child (a pending
     flag of the child, class 3, cleared by the taking lane) so the lane's
     FIFO is by first readiness. ui_step,
     the UI lane's turn, drains its inbox: for each request whose child is
     still bound to UI, eligible and ready, it takes the child (the taking
     lane clears the flag) and runs the child's turn on the UI thread; a
     request for a child that is gone, no longer UI or not ready is
     dropped, and the parent sends again when the child is next ready. The
     runtime-level ui_cursor and the tree walk of the interim UI take go;
     the mailbox's admission lock is the one synchronization (decision 18).
     Ruling on the "parent's policy" (2026-09-14, the lead's design question
     before moving the bind affinity into the record): spec 19.28.R2.2,
     ownership of cells, item (2), names "the execution mapping written at
     bind" as a supervision cell of the child owned by the parent, so the
     mapping is per child, chosen by the parent's lane at bind (today the
     affinity argument of lmx_msg_exec_bind, kept), and the parent's policy
     is that choice applied child by child; a per-parent cell cannot express
     a mixed family (the executor selftest's w1-w3 on contexts beside a UI
     child under one parent) and restructuring families to fit one would
     serve a plan sentence, not the spec. lmx_sched_record's policy cell
     keeps its meaning as the parent's step policy (sequential today) and
     takes no UI value; the earlier sentence here saying it would is
     withdrawn. What remains of the item: the affinity write at bind and
     rebind is a parent-lane write under the lane oracle (a hook at both
     sites), reads stay wherever they are (a late read changes nothing),
     and the item is then closed. Found by the lead while placing the hook
     (2026-09-14): lmx_msg_exec_bind's authority was the host thread alone,
     so a child's turn run by the host's sequential mapping could rebind its
     sibling; ruled and closed at the source in the same commit: bind's
     authority is the host outside any turn or the turn of the target's
     parent, a self-rebind and a sibling's rebind refuse with INVALID and
     the cell unchanged (committed pins), the hook's owner is the parent
     (the pass set for owner = child would admit the self-rebind), and any
     selftest case that rebound from a child's own turn moves to the
     parent's turn or the host in that commit, named in the note. The same
     rule and hook for unbind (the same cell), with the clause the settle
     chain needs (found by the lead reading run_child_turn -> parent_settle
     -> settle_child's recursion, 2026-09-14): when the target's parent is
     itself settled (handoff_ready, not running: no lane), the authority is
     the turn of the nearest unsettled ancestor, the lane doing the settling
     (19.29.6: the lane that settles a Message writes its cells), a walk up
     parent_msg under the exec lock; release_slot keeps ignoring the
     unbind's status in production, since every entry into the chain
     carries lifecycle authority and the clause covers the recursion, and a
     TEST tripwire under the lane check aborts if a refusal ever reaches
     it; the selftest's bare-thread unbind during a launch (reap_unbind)
     moves to the parent's lane on a context worker, the race unchanged.
     Landed 2026-09-14: a24a5949 (d6/bind-authority) merged onto integration
     as b7ef2080 (run_gates 11 of 11 with no lane-write failure and no
     tripwire; run_lmx, run_l2trans, run_l2_message_root green): one rule
     for bind and unbind through mapping_authority_locked, the five
     affinity writes and the record's removal hooked with the parent as
     owner, the TEST tripwire in release_slot (lm1 and lm2 identical, 7
     lines), the self-rebind case moved to the host, two committed refusals
     (a host-thread sibling; P's turn disposing C with G settled under it),
     the reap-kept race retired with kept-wait coverage measured at 6 hits
     per selftest run, four reds measured one by one (RUNTIME_L2_PORTS,
     87e27652). Beside it, LmxMsg's four unused sched_ready fields deleted
     (7cbc23dc, merged as b4646252; the old fixture red at gcc "'LmxMsg' has
     no member named 'sched_queued'"), closing 3c-2b's field drop.
     Acceptance: the 19.29.6 checks and the executor selftest's UI
     cases unchanged in outcome; a new case where two parents map UI
     children and the UI lane serves them in admission order; the lane
     oracle armed; red-first by dropping the request send (the UI child
     never runs) and by taking a child not requested (the oracle or the
     order case). The lead implements on the take_ui seam; the review chat
     writes the acceptance first.
     Landed 2026-09-14: 44ae8904 (the lead, d6/exec-3b), merged onto
     integration as 6e522d75. The UI lane is a Message-shaped mailbox owner
     created at the first request (exec.c, e->ui_lane), LmxMsg.ui_pending
     is the child's outstanding-request flag (class 3, cleared by the taking
     lane), LMX_MSG_KIND_MAP the request envelope; lmx_msg_exec_ready
     (lm1/lm2, nine identical lines) sends; take_ui drains in admission
     order. Acceptance: tests/lmx_model_ui_lane_3d_selftest.lm1 (7dda2f05,
     written first: 22/2 on the interim take, red on the readiness-order
     lines; 22/0 on 44ae8904), the executor selftest's two-parent case,
     run_gates -LaneCheck 11/11 on the merge; red-first by the dropped send
     (22/12), the LIFO push (22/6, reproduced by the review chat on a
     detached worktree) and the dropped dedupe (parity red). 0c promotes
     the test into scenario36's default set on the merge. Still the bind
     affinity as the policy until lmx_sched_record's policy cell takes over
     (3c-2b's last step).
   Acceptance per step: run_lmx -Suite Message; the five core tests (the
   19.29.6 checks pin parallel execution and no overlap); the executor
   parity of lmx_message; a tripwire per step.

   Division (2026-09-14, agreed with the lead): the lead takes 3a-2 and 3b
   (exec.c and lmx_message.lm1's by-position uses, the Exec selftest's index
   API rewritten, contexts owned by the mapping parent); the review chat
   takes 3c (both halves) and each step's acceptance tests, and mirrors the
   lead's lmx_message.lm1 edits into lmx_message.lm2 so the parity runner
   stays green at every commit. 3a-1 (7d1d1a40 on fable/exec-3a) is the
   lead's to merge after the lane branch.

   3b contract (2026-09-14, agreed with the lead on integration 6d12b222,
   after 3a-2). Shape: (1) per parent: a scheduler record handle and, over
   it, enqueue/dequeue/remove/has of a direct child and the list of
   contexts the parent mapped; (2) executor side: a per-parent wake ("this
   parent has a child ready for a physical worker"), and the lane loop
   enumerates parents with a raised wake, never children; (3) nothing in
   lmx_message.lm1 enumerates bound records across parents. The
   enumeration that survives is each parent's own children: scan_ready
   asks the parent's record for its next ready child; drop_stale walks the
   parent's first_child/next_sibling and removes the stale ones from that
   parent's record. The two globals are thrown away: a walk of rt->slots
   (every Message of every tree) and an intrusive bound list (the bind
   table by another name). The one cross-parent walk left, the lane's
   enumeration of parents with a raised wake, lives in exec.c behind one
   function so 3c-2 replaces one call. Evidence (the lead's grep of the
   callers): scan_ready's only caller is take_addr from the lane loop, with
   no parent in hand, which is (2); drop_stale_ready has no production
   caller and goes; the Exec selftest cases that fabricated a ready,
   mapped child no parent lists (detach_child_keep_ready) are rewritten on
   real parents or deleted, with retire-exactly-once kept pinned, and a
   rewrite that cannot reach the property without the fabrication comes to
   the review chat before deletion.

   3b-7 and 3b-8 (2026-09-14, agreed with the lead during 3b). 3b-7, in
   four gated steps: (a) a per-parent context list (ctx_head/ctx_tail on
   the parent, ctx_next and a stored ctx_owner on the record), linked at
   bind onto ready_owner_of(child) and unlinked in unbind, behaviour-neutral,
   proven by a TEST-build agreement check against the table with red-first
   mutations on link and unlink; (b) start_contexts, stop, drop_binds,
   detach, wake and the lane catch-up walk parents from rt->root and their
   context lists, behind one exec.c function; (c) the by-address lookups
   read m->exec_bind through a find that does not skip RELEASED Messages,
   and the unreachable rebind branch goes; (d) the table, bind_grow/bind_cap
   and the by-position accessors go. The owner of a context is the owner of
   the ready entry: ready_owner_of(child), the child's parent or the child
   itself when parentless. ctx_owner is stored, like map_owner, for one
   reason only: lmx_msg_release_slot clears parent_msg (child_unlink) before
   it unbinds. That order is the defect by 19.28.R2.2 (a child's executor
   state belongs to its parent's scheduler record and must be torn down
   while the child is still that parent's child), and it also puts a bound
   parentless Message outside every family tree between its root-list
   unlink and its unbind, which a tree walk cannot see. So 3b-8 comes
   right after 3b-7a and BEFORE 3b-7b (order fixed 2026-09-14 with the
   lead): release_slot unbinds first, then child_unlink; lm1 and lm2 in one
   commit; map_owner, ui_map_owner and ctx_owner go, derived from
   ready_owner_of; the contract of child_unlink for a bound child is decided
   and stated there (lean: refused, a contract at the family boundary), and
   the TEST agreement check of 3b-7a turns strict while the table is still
   present as its oracle. Its reaching test is the failed-turn path releasing
   a bound uncommitted child. Then 3b-7b (the walks over the family tree,
   owner by owner, restart-to-fixpoint on start and teardown, no
   allocation), 3b-9 (found during 3b-7b: release_slot mutates the family
   tree with the exec lock dropped while every reader walks it under that
   lock; the lock is held around child_unlink and the root-list removal,
   lm1 and lm2 in one commit, red-first through a TEST hook between the
   unlock and child_unlink with a second thread walking the family), 3b-7c,
   3b-7d, then 3c-2's C half.

   3c-2 in two halves (2026-09-14). 3c-2a, the build: the production
   runtime built by every runner is the L1 modules plus every L2 unit under
   l2src whose first line is `profile: runtime` (lmx_sched_record.lm2
   first); one shared fragment, l2src/l2units_build.ps1
   (Build-L2RuntimeUnits: l2trans from the tree with the pinned l1trans,
   then per unit its header, the generated lm1 with the two checks of
   run_sched_record, l1trans to C, gcc -c), appended to each runner's
   runtime link; behaviour-neutral, the symbols linked and unused;
   acceptance: the ten gates green with lmx_sched_record_new defined in
   every runtime link (review chat wires run_lmx, run_port_message and
   run_model_scenario36; 0c the rest and run_gates). 3c-2b, re-cut by decision 18 (one
   lane, one writer; 2026-09-14): readiness is the child's own control
   flag, set at admission and by the closing request, cleared by the child
   when its turn is taken; the parent's scheduler step reads its direct
   children's flags and chooses (round-robin by a cursor of the parent's
   own), and the intrusive per-parent ready lists appended from children's
   threads (map_ready, ui_map_ready, the UI raise/lower of parents) go from
   exec.c, in C, not only in the port; retire gating reads the children's
   flags instead of a list. The UI lane is 3d, folded in: the UI worker is
   a lane with a mailbox; a parent's step that maps a UI child sends a
   mapping request to that mailbox, and ui_step drains its inbox and runs
   the requested turns; no thread writes another lane's data. The lead
   makes these C changes on the 3b seams (lane take, route, supervision
   detach/attach); the review chat writes the acceptance (a TEST oracle
   that every write to a parent's scheduler cells is on the parent's lane,
   keyed on the current-turn Message, red on today's push: 45c56133,
   -LaneCheck) and the parent's record as fixed cells of its own arena
   (cursor, policy: 12342ae3), so 3c-1's growing ring is retired: nothing
   appends to a parent's structure from outside its lane. Sequencing fixed
   2026-09-14 with the lead's design review: the lead's decision-18 commit
   puts the cursor on LmxMsg (the parent's own cell) because some twenty
   runners still link exec.c without the L2 unit; 0c then routes those
   runners through l2units_build and retires lmx_msg_sched_ready (unit,
   runners, gate), the lead drops its fields, and the review chat moves the
   cursor into lmx_sched_record as 3c-2b's last step (done 2026-09-14:
   routing 1a410b54 by 0c, the cursor move e4c1dc70 on fable/cursor-3c2b:
   LmxMsg.sched_rec is the record, created lazily by the parent's first step
   on its lane, the cursor read and written only there, the C host pick
   taking it as an argument; the unit's prototypes declared in
   lmx_message.h; acceptance in lmx_sched_record_selftest, 22 checks; the
   retirement e8f7eb58 by 0c rebased behind the routing fix for
   run_port_parser and run_l2_message_root, found unrouted afterwards). The UI take walks the
   tree with a runtime-level cursor until 3d, a separate commit, gives the
   UI lane its mailbox (done: 44ae8904, on integration as 6e522d75). The orphan step (e09bc3f4) and scenarios 4 and 5 of
   the release-17 test are done; the settle chain (541cad03) too.
   3c design (drafted before 3b; the record of 3c-1 and the contract above
   fix it). The parent's
   scheduler record is an ordinary Structure allocated in the parent's
   arena by the parent's own Message code, reachable from the parent's graph
   (its root retains it), with these slots: the ready list of direct
   children (head, tail; today LmxMsg.sched_ready/sched_ready_tail),
   the list of contexts the parent mapped (today the per-bind
   LmxMsgBindWait records reached through the global table), the mapping
   policy (sequential on the parent's lane, one context per child, UI lane
   for a child with UI affinity), and the wake primitive handle of the
   parent's lane (today the global ready event). The L2 unit
   (`profile: runtime`) implements the operations over that Structure with
   the branch/value calls on the parent's arena: enqueue/dequeue a ready
   child, map/unmap a child to a context, select the next child turn
   (sequential mapping), request the children's close, and reports the
   19.28.R2.2 invariants the tests pin (a whole child turn before another;
   no shared list across parents). The C half keeps only what L2 cannot
   spell: thread start, event wait/signal, TLS of the current turn, the
   setjmp turn root, called through `c.`. Migration: the record is filled
   from the existing LmxMsg fields first (a view), then those fields move
   into it, then the executor reads only the record.
4. **Close, liveness, failure.** stop as KIND_STOP admission setting closing
   only; family close per §32; liveness queries and timers per §33 as
   self-maintenance of every running Message; failure handoff per §34 with
   the HISTORY roots (lmx_msg_history_owned's contract). Decision 17
   (2026-09-14): the family release chain. Today stopped and disposed
   children stay linked to the parent until runtime_delete (found during
   3b-8: a released parent never retires while first_child is set). The
   runtime must release a closed branch by the chain: orderly, a STOPPED
   child waits only for its parent's adopt/dispose, then its slot is
   released and unlinked and the parent retires once it is released itself;
   forced, a parent's release closes and releases its subtree; a child's
   self-close does the same for its subtree; a parent's success with running
   children closes them. Acceptance first (review chat): the §32 test
   extended with three falsifiers that are red on today's runtime; then the
   lead implements in lm1/lm2/exec.c after 3b-7d, beside 3c-2's C half
   (disjoint functions: release_slot, dispose_child, adopt_failed, try_retire
   against the lane take and the ready sets). Rule (4) of decision 17 (corrected
   2026-09-14): a running child survives its parent's closing only by a
   handoff of supervision to another live parent the closing parent chooses
   among the capabilities it holds, not necessarily its own parent; the
   child keeps its arena, mailbox and turn, only the parent capability and
   the scheduler place move (a new runtime operation, distinct from
   transfer_adopted, to be specified with the chain: the child's record
   leaves the old parent's scheduler record and enters the new one under the
   exec lock, as 3b-9 requires); a child not handed over closes with the
   chain; the root's only new parent is the virtual World Wide Mix ancestor
   (OS-process level), a stub until stage 5, which then implements it as
   "launch an OS process". Rule (5): adoption closes the adopted, so
   adopt_failed and transfer_adopted end with the source's slot released
   (the same release_slot step as dispose), and a Message spawned by the
   adopter from adopted content is the adopter's child by construction
   (lmx_msg_create under the adopter); no runtime state may say "adopted and
   alive".
   Section 34 audited (2026-09-14, the lead, on integration c2a5bf4f; the
   review chat's acceptance on fable/exec-3a 14c44ee6): (1) the move without
   a deep copy that closes the adopted, (2) nothing released before the
   parent holds the graph, (4) no forwarding of history at the parent's
   success: implemented, pinned by tests/lmx_msg_family_handoff_selftest.lm1;
   (3) a transferred block is not made eternal: implemented, caught by
   scenario36 ("E is unaffected by B's failure") and now directly by two
   section-34 checks in the handoff test (63 checks); (5) a failed orphan
   whose turn ended on its own context gets its deadline only from
   lmx_msg_orphan_sweep: a coverage gap (the three deadline lines could be
   deleted with run_gates green), closed by
   tests/lmx_model_orphan_mapped_17_selftest.lm1 (17 checks, red 2 under
   that deletion), to be promoted into scenario36's default set. Ruling on
   the audit's side note: lmx_msg_exec_adopt_mark's init-block push after
   the storage move cannot fail (lmx_msg_blocks_push refuses only a
   malformed argument and receives a fresh detached node), so its failure
   branch is dead and goes under decision 12; nothing fallible follows the
   move.
5. **Root Message and bootstrap.** OS startup is the root Message; the
   external process entry runs in its turn loop. The L1 runtime remains the
   bootstrap underneath until the L2 runtime hosts itself; then the L1
   modules are retired stage by stage.
   Design (the lead's draft, 2026-09-14, reviewed by the review chat the same
   day; its five questions were answered from spec 19.29.6, 9.1.4 and 1.7 and
   written there as "Stage 5 clarifications", none forwarded to Mikhail):
   R0 is the only entry of rt->root (address 1, no parent capability), its
   lane the main thread, its management state (scheduler record, liveness
   and orphan-retention settings, the 9.1.4 arrays) in its own arena and
   created with the runtime; the process entry runs inside R0's turns with
   the settings and argc/argv as R0's initial Message, one admitted input per
   turn, and between turns R0's self-maintenance (ingress drained into its
   mailbox, liveness poll, the orphan sweep with orphan_end folded in, the
   step over its direct children, the wait on its lane's wake); the UI lane
   becomes R0's child with its mailbox unchanged, mapped to R0's lane, its
   ui_step the child's turn, and it enters rt->n; parent-0 creations and
   re-rooted orphans become R0's children (an orphan keeps its retention
   policy under R0); the handoff from R0 to the World Wide Mix ancestor is a
   refusing stub with its own status, the call site and the address fixed.
   What leaves the L1 host loop, in order, one commit each, every commit
   moving one authority from "the host outside any turn" to holding R0's
   turn, updating its callers and the lane oracle's pass rule, red-first by
   calling it from outside R0's turn (it must refuse): (a) l2_program_entry
   into R0's first turn (the generated main shrinks to runtime_new, the
   bootstrap bind and R0's loop); (b) host_post as an admission to R0's
   mailbox and host_drain as R0's drain between turns; (c) drive as R0's
   self-maintenance; (d) the step over R0's children, the UI lane child and
   run_child_turn's host path; (e) set_now and the other host setters as
   R0's own cells; (f) runtime_delete as R0's close, the decision-17 chain
   bottom-up, then R0's slot. Migration of the tests: the executor selftest
   and the core tests drive the runtime as the host today; a bootstrap
   helper that runs a test's driver as R0's turn (the host thread is R0's
   lane by definition, model section 29) makes that migration mechanical
   and lands with step (a). Slot-count oracles gain R0 and the UI lane in
   the step that moves them (d).
   Step (a) as designed by the lead and accepted (2026-09-14): the helper
   lmx_msg_run_entry_turn(rt, addr, turn, ctx), on the host thread outside
   any turn, binds the parent-0 Message addr to turn, runs exactly one turn
   of it on this thread, unbinds, and returns the run's status (INVALID when
   the caller holds a turn, addr is not parent-0 or is already bound);
   l2trans emits three names: l2_program_body (the source body, its return
   emission unchanged), l2_program_entry (the adapter: INVALID unless inside
   its own Message's turn, otherwise the body's result into the result cell
   and 0, so the program's value never travels in the status) and
   l2_program_turn (the LmxMsgTurn trampoline the bootstrap runs); the generated main and l2_library_open (each on its own
   runtime, so a library opened from inside a program's turn still opens,
   holding_any being per runtime) become runtime_new, the create of R0, the
   helper, the existing tail; the parse driver's two direct entry calls
   switch to the helper in the same commit, and 5e leaves that driver alone
   until it merges; runtime_new creating R0 and the slot counts stay for
   (d). Red-first in the commit: the adapter outside a turn returns INVALID
   with no graph installed, the helper is INVALID inside a turn and on a
   bound address, the emitted main has no direct l2_program_entry call.
   Landed 2026-09-14: d6/stage5a-r2 f8cc5e4e (3812c5ef rebased on 0c's
   re-founded gate 40d1ff97, plus 0ba95971, the abort section), merged onto
   integration as 892e5d3f after 5e's Stage d slice 1 (e58f3ec1).
   Acceptance run_entry_turn.ps1 with tests/lmx_entry_turn_selftest.lm1:
   19 checks (the adapter's refusal outside a turn with no graph and no
   result; the bootstrap running the turn, returning the value and
   unbinding; the bootstrap's refusals of a bound address, a Message with a
   parent and a call from inside a turn of the same runtime; a library
   opened lazily from inside another runtime's turn; the generated main
   returning the program's value; a body that aborts under the turn root
   leaving main's result at 1 with the runtime deleted, and the same
   program as a process exiting 1), the emitted L1 shape pinned (body and
   adapter each called once, the refusal line present, main and
   l2_library_open reaching the entry through the bootstrap alone); ten
   reds measured one by one. Gates on the merge: run_gates 11 of 11 with
   c_scanners, run_l2_message_root, run_graph_abi, run_lmx, run_l2trans
   gen2 green; the parse driver's two calls go through the bootstrap and
   the re-founded run_candidate_c_scanners (40d1ff97: it reads the tree, no
   frozen snapshot, no build-directory overlay; the driver's tracking table
   grows without a cap, 4316 entries flat across parses) measures it, with
   the old driver red at the adapter's refusal; a stale one-argument L1
   prototype of the adapter links by name and fails only at run, because
   the pinned l1trans does not check a call's arity, so 0c's 36bccd8c pins
   the prototype's shape in that gate. The FABLE_GRAPH_ABI "INTEGRATION
   GAP" note for the entry is closed.
   Step (b) as designed by the lead and accepted (2026-09-14; acceptance
   written first, fable/stage5b-acceptance 3d1312c9: tests/
   lmx_model_root_ingress_5b_selftest.lm1, 11 checks, red 2 on the host
   ring): the ingress root before (d) is rt->root's first non-orphan entry,
   no runtime field; host_post keeps its name, signature and any-thread
   contract and appends an internal-kind envelope LMX_MSG_KIND_INGRESS (the
   original kind in a copy field, `to` the destination, from = HOST_FROM)
   into the root's inbox under the root's mail lock, waking the root's
   lane, GONE when no parent-0 Message exists; the drain is the root's
   between-turns work on its lane (the host outside any turn while the
   bootstrap is the host, R0's lane between turns after (c)), moving the
   INGRESS nodes in admission order onto the transport with their kinds
   restored and pumping, INVALID from any other thread and, by the loop's
   shape rather than a spec mandate, from inside the root's own turn; the
   root's recv never hands out an internal kind (STOP, MAP, INGRESS) and
   returns EMPTY when only INGRESS nodes are pending; an internal kind is
   never an input, so the five readiness reads (is_runnable, sched_eligible,
   the host pick, the bind kick, the claim for a turn) move to a new read
   lmx_msg_mail_inbox_has_input, true only for a node of a non-internal
   kind, while the two lifecycle reads (try_retire, drive's should-close)
   keep the raw inbox_empty so a root never retires or closes over
   undrained ingress (readiness ignores internal kinds, lifecycle does not:
   the lead's precision, 2026-09-14), a rule (c) and (d) keep because a
   post can land at any moment;
   rt->host_head and host_tail, the ring's host-lock use and post's
   host_wake retire, host_wait stays as the bootstrap lane's wait signalled
   by an admission into the root's mailbox; the shutdown flag and
   host_is_owner stay until (d)/(f); lm1 and lm2 hunks mirrored (rule a);
   5b promoted into scenario36's defaults in the same commit.
   Landed 2026-09-14: the acceptance merged as 82cc9974 (3d1312c9 --no-ff),
   the code as 22deeb73 (d6/stage5b 066b7a26: msg_ingress_root, host_post
   into the root's inbox, the drain between turns, recv through pop_input,
   the five readiness reads on has_input, the ring gone from
   runtime_delete; lm1 and lm2 identical after normalizing unit names),
   the promotion f3e518c5 (5b the ninth default), the notes 573ab89b (the
   lock order host -> exec -> mail checked by grep with every host-lock
   site, the interim caveat, three committed cases, five reds). Gates on
   f3e518c5: run_gates 11 of 11 with 5b at 11/0, run_lmx, run_l2trans,
   run_l2_message_root green; run_port_message parity at 100 methods.
   Step (c) as designed by the lead and accepted (2026-09-14; measured on
   573ab89b: drive's authority was require_owner alone, so a host-thread
   turn could drive; orphan_end had one caller, run_child_turn's tail; the
   sweep already does everything orphan_end does): lmx_msg_drive keeps its
   name and order (set_now, the liveness poll, the orphan sweep, the walk
   of roots, the pump) and takes the drain's authority (owner outside any
   turn, INVALID from any other thread and from inside any turn, the
   root's own included); orphan_end and the tail call go, so the sweep is
   the one settlement path on both the host path and the mapped path, and
   a successful orphan is reclaimed at the root's next maintenance point
   (answer (a); the spec and model sentences clarified accordingly), the
   release-17 line that pinned the end-turn reclaim split into "still
   found after its turn" and "reclaimed by the next drive"; nothing else
   moves; lm1 and lm2 mirrored. Red-first: the sweep's three deadline lines
   deleted turn release-17's scenario 5 red as well as orphan_mapped_17
   (before the fold only the mapped test goes red, measured); the
   holding_any clause deleted prints the drive-from-a-turn case's refusal
   text; the split line's first half red if a tail reclaim were kept.
   Landed 2026-09-14: d6/stage5c f66fb2bc merged onto integration as
   babc02d5, notes ace75b2e (lm1 and lm2 code identical: drive's clause,
   orphan_end deleted, the sweep untouched; the exec tail call and the
   prototype gone; release-17 at 47 checks with the split line;
   run_port_message parity at 99 methods). Reds as measured: the three
   deadline lines deleted red on both release-17 and orphan_mapped_17; the
   clause deleted caught by the bind-authority tripwire ("release_slot:
   unbind refused site=release_slot owner=0 turn=1") before the case's own
   line; the tail reclaim kept red on the split line and the exec maintain
   case. Gates on babc02d5: run_gates 11 of 11 (release-17 47/0), run_lmx,
   run_l2trans, run_l2_message_root green.
   Step (d) as designed by the lead and accepted (2026-09-14; measured on
   babc02d5: runtime_new creates no Message, parent-0 creates and re-rooted
   orphans chain on rt->root, the UI lane is a bare slot outside slots and
   n, no test compares an address to a literal, eleven absolute slot-count
   literals in release-17 and orphan_mapped_17), three commits, lm1 and
   lm2 mirrored, red first each: (d1) R0 exists and is the one top level,
   created by runtime_new (address 1, committed, running, rt->root = R0,
   n = 1); a parent-0 create links under R0 and orphan_children re-roots
   under R0, the sweep walks R0's children, the ingress root is R0; the
   walks keep their shapes (one family); run_entry_turn binds R0 itself and
   refuses any other address, l2trans's two sites drop the emitted create
   and use lmx_msg_root_addr; the migration helper lmx_msg_root_turn(rt,
   fn, ctx) lands here: exactly one R0 turn on the calling thread, bound
   without launching a context, ended and unbound, R0 not ended; the
   authority stays dual until d3 (R0's turn or the host outside any turn).
   (d2) the UI lane is R0's child created in runtime_new right after R0
   (address 2, n = 2 from the start): in slots and n, the MAP-only inbox as
   today, take_ui plus run_one as its turn, exec_ui_step's name kept; recv
   never hands out a MAP node. Between d1/d2 and d3 the test call sites of
   run_child_turn, map_child, sched_step and ui_step (about 150, from 0c's
   inventory recounted on the tip) move into root_turn callbacks as 0c's
   commits, each green under the dual authority. (d3) a step is its parent's
   act: run_child_turn, sched_step, map_child and exec_ui_step require the
   turn of the stepped Message's parent (R0's turn for R0's children, P's
   turn for P's), the host-outside-any-turn branch goes outright for every
   parent (corrected 2026-09-14 from "on a child of R0": after (d) the host
   between turns is R0's lane for maintenance and the drain only, never a
   step, 19.28.R2.2), no test moves; reds: each called from main returns
   INVALID with nothing stepped. The lane
   oracle's turn == 0 pass stays, since the host between turns is R0's lane
   for drive's maintenance writes; d3's refusals live in the APIs. Slot
   oracles: the literals gain +1 in d1 and +1 in d2, absolute. Not in (d):
   runtime_delete as R0's close (f), the setters (e), the World Wide Mix
   stub fixed at its call site only.
   d1 precisions from the lead's reading of the tree (2026-09-14):
   run_entry_turn accepts R0 or an unbound direct child of R0 (exactly the
   former parent-0 set: the parse driver's two owners, the entry-turn
   selftest's A/B/D/E, the three tracked goldens keep working), a deeper
   Message refused; the l2trans emitter change (main and l2_library_open
   on lmx_msg_root_addr instead of an emitted create, the three goldens
   regenerated, message_root's driver checks) is its own commit after d1,
   gated by run_l2trans, run_lmx and run_l2_message_root; a liveness query
   is answered only in the parent's own turn (live_handle), never by drive,
   so a child of R0 is answered by an R0 turn run through root_turn, pinned
   in liveness_33, whose "a root has no parent to poll" moves to R0 itself;
   orphan_settled drops its parent_msg = 0 clause (the orphan flag carries
   the meaning) and the two tests read parent_msg = R0 on an orphan; the
   six readers assuming a top-level path of depth 1 (lmx_message_selftest,
   run_port_message's warm-up) read depth 2; the seven (b) inbox reads move
   from the created root to R0. Extra red: the orphan_settled clause kept
   turns release-17's scenario 5 and orphan_mapped_17 red on their reclaim
   lines.
   d1 landed 2026-09-14: d6/stage5d1 67a61fa2 merged as f8dd6015 over
   fd88bd8a, notes 666720fc (six reds with texts; gates: run_gates 11 of 11
   with release-17 47/0, liveness_33 61/0, root_ingress_5b 11/0;
   run_port_message parity 100 methods; run_entry_turn 22/0; run_lmx,
   run_l2trans, run_l2_message_root green). Two rules the lead found while
   handing the migration to 0c, fixed in one small commit before the
   migration reaches the tests they touch: (1) an orphan under R0 is
   settled only by the sweep under the retention policy (19.29.8), so
   parent_settle and the settle chain skip a child whose orphan flag is
   set and R0's turn stepping its children neither adopts nor disposes an
   orphan (red: a failed orphan under R0 with its window open survives one
   stepping root_turn with R0's adopted count unchanged and is reclaimed
   by the sweep past its deadline); (2) root_turn and run_entry_turn bind
   without launching a context (an internal bind mode the launch path
   ignores), so lmx_msg_exec_workers is unchanged across a root_turn in a
   runtime with contexts started. Migration order for 0c: files without
   orphans or contexts first, the rest after that commit.
   d1b landed 2026-09-14: d6/stage5d1b f823ac9a merged as 85a26af8, notes
   09da4678 (first_settled_child and parent_settle skip an orphan;
   exec_bind_mode with launch, the bootstrap on launch = 0; release-17 at
   51 with the orphan-under-R0 case; the root_turn-with-contexts case;
   two reds; run_gates 11 of 11, parity 100 methods, entry_turn 22/0,
   run_lmx, run_l2trans, run_l2_message_root green).
   Migration shape, ruled 2026-09-14 on 0c's finding that inside R0's turn
   run_child_turn, map_child and sched_step accept only R0's direct
   children while the tests step Messages at depths 2 to 4 from the host:
   no ancestor rule (R0's turn stepping any descendant would be the L1
   host's cross-family stepping that decision 18 removed; model 29, each
   parent services its direct children) and no flattened fixtures (the
   chain tests need their depths); the migration is nested turns: from
   main a root_turn, inside it R0 runs its child P's turn through
   run_child_turn, P's bound turn runs C's, and so on down, with drive in
   main between root_turns; intermediate parents, unbound today because
   the host ran their children directly, get a test-side helper turn that
   runs the child named in its ctx cell (defined once in the shared test
   include, bound from inside R0's turn under the parent-lane rule), never
   a runtime API for it; the lead specifies the helper's shape in the
   migration ticket; ui_lane_3d migrates as written (done, 563578f1: 13
   sites, 22/0, the callback falsifier red, the d3 flip red unmigrated and
   green migrated); adopt_unrooted held. Found by 0c's nested migration of
   scenario36 (2026-09-14): with B's cancel step run from inside P's turn,
   "B leaves its native activation, handoff-safe" and "P adopts B's
   retained state" go red. Measured by 0c and the lead: the helper's
   end_turn is not involved; run_child_turn's tail (since 3b12267a) runs
   parent_settle(par) only when the stepping lane holds the parent's turn,
   so the parent's-turn path ADDS an automatic settle that adopts B at the
   step's tail before the test's explicit adopt, while the host path never
   settled. Ruled (the review chat's first reading was inverted and is
   withdrawn): the spec puts settling at the parent's explicit dispose or
   adopt or the chain those start (19.29.6 consequences with 19.29.8: a
   STOPPED child waits only for its parent's adopt/dispose), and nothing
   makes a step settle the parent's other settled children, so d1c deletes
   the tail settle on every lane and the nested shape equals the host
   shape with the chain tests' lifecycle lines verbatim; whatever
   3b12267a's envelope move relied on (transfer requires disposed
   children) is corrected by an explicit dispose or adopt where the spec
   puts it, named in the note. Red-first: scenario36's two lines red in
   the nested shape before, green after; the gate run of the deletion is
   the second red if anything depended on it. The tests that step under a
   stopped or settled child are held until d1c; family_close_32's stopped
   C1 and G1 (a stopped parent has no turn to step its children) read
   after it. First wave landed 2026-09-14 as 0def609d over 09da4678 (0c's
   563578f1 ui_lane_3d, a6e30a70 sched_record, 971a740d send_local, then
   f31d42eb deleting send_local's never-called legacy case; each with the
   callback falsifier red and the bootstrap-aware d3 flip red unmigrated
   and green migrated, measured on scratch commits where the gate archives
   HEAD; run_gates 11 of 11 on the merge); second wave in progress
   (root_ingress_5b, adopt_unrooted after the lead lifted its d1b hold,
   the context tests), checks_19_29_6 has no stepping site. d1c measured:
   nothing depended on the tail settle (the nine, parity, run_gates 11 of
   11 on the deletion; 3b12267a's dispose-before-transfer is explicit in
   its callers). A second test helper for a parent that is already
   cancelled (family_handoff's C stepping G): turn_step_child_open steps
   the child and returns without recv or end_turn, a turn shape the
   executor defines (run_one reaches the boundary, ends the turn only if
   the body made the Message exec_live), so the cancelled parent's closing
   turn is not spent on the step; condition: no recv inside, "C not
   STOPPED right after G's step" pinned with a recv falsifier, the helper
   outside turn_step_child's equality check; a test-side shape, not a
   runtime change. orphan_mapped_17's migration depends on d1c,
   liveness_33's does not; both committed after d1c lands. d1c landed as
   b8f1b2aa, notes 768ac22a (three measurements: the case's red, nothing
   depending on the tail, 0c's nested scenario36 from 2 failures to PASS
   with its text unchanged). Found by 0c's family_handoff migration and
   ruled from the spec (2026-09-14): a stopped parent's children get no
   turn from it (a stopped Message has no lane), so after d3 their closing
   turns come through the chain: an empty-inbox child is closed by the
   maintenance (liveness_33 section 4), a child on its own context closes
   itself there, and a child with pending input waits until its parent is
   disposed, is re-rooted under R0 by the chain, and has its closing turn
   run by R0's step; none of the candidates (the settle path stepping, the
   sweep stepping, a stopped parent keeping authority) is the model; the
   open-turn helper is moot for a cancelled parent (run_one settles a
   cancelled Message bodyless at its first turn). family_handoff and
   family_close_32 therefore reorder: C stops, P disposes C from its turn,
   G is R0's orphan child, root_turn steps G's closing turn, the sweep
   reclaims it; their pinned counts move with the commits.
   d3 as designed by the lead and accepted (2026-09-14), one commit after
   the migration completes: run_child_turn keeps a static core (claim,
   run_one, release) with two authority front-ends, the public one on
   holding_turn(par) only and run_entry_turn's on its own three checks
   (host outside any turn, R0 or an unbound direct child of R0, bound by
   itself with launch 0), so the bootstrap never passes through the public
   step; only the bootstrap starts R0's turn, a public run_child_turn on
   R0 from main is INVALID (a recount of direct run_child_turn(R0) sites
   first, expected none); map_child and sched_step (lm1 and lm2 mirrored)
   on holding_turn(parent) only; exec_ui_step on holding_turn(R0); reds:
   run_child_turn on a child of R0, map_child on a child of R0, sched_step
   on R0 and exec_ui_step from main each INVALID with nothing stepped, a
   library open through the bootstrap green; unchanged: bind authority,
   drive and host_drain outside any turn, the oracle's turn == 0 pass.
   d3 landed 2026-09-14: d6/stage5d3 83f751b3 merged as f096e4fc over
   864cd580, note 090704f7 (lm1 and lm2 identical, 11 lines; exec.c:
   child_turn_core with the bootstrap flag, run_child_turn on it with 0,
   run_entry_turn with 1, exec_ui_step on holding_turn(R0), map_child on
   holding_turn(parent); reds "exec host step refused child=0 sched=8 ui=8
   root=0 map=0 turns=2" before and "child=0 sched=2 ui=2 root=0 map=2"
   with the host branch back; gates: run_gates 11 of 11, parity 100
   methods with "from main the host steps nothing", entry_turn 24,
   run_lmx, run_l2trans, run_l2_message_root green). Stage 5 (d) is
   complete: R0 exists with the runtime and is the one top level, the UI
   lane is its child, every step is its parent's act from the parent's
   own turn, and the host outside any turn keeps only the maintenance (c),
   the drain (b) and the bootstrap until (f).
   d1d, from 0c's drive measurement in the chain order (2026-09-14): a
   drive-closed unbound child (exec_unbound_close: end_turn under the
   Message's identity from the host) was STOPPED but never handoff-ready,
   since handoff_ready is written only by run_one's native_leave, so its
   parent could never dispose or adopt it and it stayed until
   runtime_delete, unreached by any test (liveness_33 section 4 never
   disposes F). Ruled: the unbound close ends with the boundary run_one's
   tail applies (handoff-ready), and it is maintenance, not a step (no
   handler, no lane; the end-turn bookkeeping of a Message without a lane,
   written by the maintaining lane), so it stays with drive after d3; one
   small commit before 0c's unbound cases; pin: a drive-closed unbound
   child is STOPPED and handoff-ready after one drive and its parent's
   dispose takes its slot; falsifier: the handoff_ready write removed.
   d1d landed 2026-09-14: d897c16e merged as 39934758 over ce0bd65d, note
   f9a25865 (run_gates 11 of 11; parity 100 methods; entry_turn 22;
   run_lmx, run_l2trans, run_l2_message_root green). Migration detail
   ruled by the lead, consistent with the chain ruling: the (c) maintain
   case steps the re-rooted orphan C from R0's turn (orphan_children
   re-roots C under R0 since d1, parent_settle skips it since d1b), so it
   still measures that a settled successful orphan waits for maintenance.
   family_handoff's section-34 half (adopt G into C, then C into P, with
   the per-step identity checks) stays verbatim through d3 (ruled
   2026-09-14): d3 leaves lifecycle authority unchanged (the parent's turn
   or the host outside any turn while the bootstrap is the host), and
   adopt_failed and dispose_child are lifecycle operations, not steps, so
   the host's adopt on behalf of the stopped C is the bootstrap host as the
   settling lane; the migrated shape unbinds G by the lane settling C,
   drive closes it, d1d makes it handoff-ready, then "G to C" and "C to P"
   as written. The strict shape is (f)'s: the lane that settles C is P's
   own dispose or adopt in P's turn, "G to C" the chain's first step inside
   it, and the per-step identity checks then read the end state or observe
   the chain's intermediate through a TEST-only hook in settle_child;
   decided at (f)'s design.
   d2 as designed by the lead and accepted (2026-09-14; measured on d1b's
   tree: the lane a bare slot created lazily at the first UI request,
   freed by exec_detach; UI records skipped by the wake paths, the worker
   loop, the launch walk and the pool claims; parent-0 creates dedup by
   create_id among R0's children, tests using ids 1 to 7): one commit;
   runtime_new creates the UI child right after R0 through create_prepare
   with the reserved create_id LMX_MSG_UI_LANE_ID = 0xFFFFFFFF (refused for
   any other create: the reserved id's contract, since an alias would hand
   a test the lane) and hands it to the executor (lmx_msg_exec_set_ui_lane),
   address 2, parent R0, in slots and n, the MAP-only mailbox unchanged;
   the child stays unbound, its step exec_ui_step's body (d3 flips that to
   R0's turn; whether the child is later bound to a turn adapter so R0's
   ordinary step can pick it is the loop step's decision after (f)); the
   lazy creation goes from ui_request, exec_detach stops freeing the slot
   (runtime_delete's slot loop owns it), nrequests and take_ui read one
   pointer; the eleven slot literals gain +1 again; the UI child takes path
   [1, 1] so every later child of R0 shifts one sequence number and the
   path readers move; ui_lane_3d unchanged until d3. Reds: find(rt, 2) is
   the lane with parent R0 and n = 2 after runtime_new; the reserved id
   refused; exec_detach freeing the slot again is a double free under the
   crash filter.
   d2 landed 2026-09-14: e81180ea plus dc3f6b18 (the message_root driver
   reading R0's last child) merged as 0c0a34e8 over f9a25865, note
   24fb94c7 (run_gates 11 of 11; parity 100 methods; entry_turn 24;
   run_lmx, run_l2trans, run_l2_message_root green).
   d2b, from 0c's exec-selftest batch under the lane oracle (2026-09-14):
   (A) take_ui's writes of the served child's ready and ui_pending are the
   taking lane's by decision 18, and the taking lane is the UI lane, whose
   lane is R0's by its mapping and which after d2 is R0's child; so the
   oracle hooks in take_ui pass the UI child as owner (the writer, not the
   written Message), and the existing pass set admits R0's turn as the UI
   lane's lane with no new rule ("LANE WRITE FAIL site=take_ui:pending_clear
   owner=5 turn=1" on the batch before, gone after; ui_lane_3d measured
   under the oracle both ways). (B) the drain is maintenance and leaves
   exec_ui_step: the UI step only takes, the host drains between root_turns
   (a migrated site that relied on the step's drain drains before its
   root_turn), after (f) R0's loop drains between turns (the worker-to-UI
   case's "ui_recvd=1" timeout inside R0's turn before, green after). One
   small commit after d2, before the exec-selftest batch and d3.
   Wave 3 landed as 8bd284f5 (liveness_33, orphan_mapped_17, the nested
   scenario36; run_gates 11 of 11); cancel_spin_host.c landed as ce0bd65d.
   d2b is 3b704fbb on d2's dc3f6b18 (the three take_ui hooks pass the UI
   lane as the writer; the drain gone from exec_ui_step; the "exec ui step"
   case; the ui_step_drained test helper for the 18 older calls that relied
   on the step's drain; red first on the oracle line, the drain back in as
   the falsifier), approved. The executor selftest's "sched-cursor host"
   case (four host steps of an unturned parent) is deleted in the
   migration commit rather than migrated, decision 12: it pinned the host
   path d3 removes, and "sched-cursor turn" pins the same order on P's own
   lane. Landed 2026-09-14, in order: family_handoff shape (a) 7c2ac7de as
   733d65f1 (section 34 verbatim; red on ce0bd65d, green on d1d, 63
   checks); 0c's wave 3 a9fce04d as a4e11bb5 (11 test-only commits, the
   executor selftest's batches 2-7 with the strengthened orphan end-turn
   check; run_gates 11 of 11, parity PASS); d2b 2f9be580 as 2bfce366 with
   note ebdd4362 (the selftest conflict against d1c/d1d resolved by
   keeping both blocks, no code line changed; run_gates 11 of 11, parity
   100 methods, entry_turn 24, run_lmx, run_l2trans, run_l2_message_root
   green). Stage 5 (d) through d2b is on integration. Remaining before d3:
   0c's wave-3 note, the exec shape-(a) cases, the exec_ui_step redo on
   ui_step_drained; the run_port_parser branch 49e5db6d (the cmd /c cut,
   the mismatch lines printing before the throw, the runtime trio and its
   headers rebuilt every run after a warm dir was found linking its first
   build's runtime: warm red at the link in 7 s, warm green 148 s, cold
   156 s before and 155 s after); 5e's c05e1178 and d3 landed cold.
   Landed since: the port_parser branch 49e5db6d as a8127b79 (cold 145 s
   on the merge); 0c's wave-3 note d7f7e813; exec-final 2892fac1 as
   8a010a81 (the exec shape-(a) cases, the 18 UI steps in R0's turn with
   no drain, family_close_32 under reading (1)) and 9fa7211d as d925e600
   (family_release_17 from R0's turn, check 158 strengthened with
   handoff_ready(c4), tests/l2/sched_snapshot.c and its Python runner
   deleted: not compiling since e4c1dc70, ungated, its unique claims all
   the C host pick's); the wave-4 note 1d609227. The migration from main
   is complete: no stg test steps a Message from main (0c's git grep over
   the four APIs shows every hit inside a turn body). Open beside d3: the
   C host pick in sched_step is still reached inside a nested turn (the
   exec selftest's mix family: holding_turn(P) true while turn_slot(P) is
   0), one turn identity read two ways; the lead finds the cause before
   the pick's fate and the sched_snapshot claims' move are decided. Cause
   found (the lead, 2026-09-14, a print on every set_tls of the thread):
   lmx_turn_msg and lmx_turn_running are one thread-local shared by every
   runtime on the thread while the held turn is per executor, and a nested
   turn of a foreign runtime on the same thread (the library unit's entry
   turn on its own runtime, run by sched_step's first lmx_sched_record_new)
   restores that runtime's old TLS (0) on exit, nulling the outer turn's
   identity; after it turn_self, turn_slot and generated code's
   running-flag polls see no turn for the rest of the outer turn. Fix as
   its own small commit before the pick question: run_one and
   exec_unbound_close save the two thread-locals on entry and restore
   those exact values on exit (the thread's turn identity is the innermost
   turn and returns to the outer one), red first with a case where a
   second runtime runs one root_turn inside P's turn and turn_self(rt)
   must still be P with sched_step picking P's child; (a)'s "a library
   opened from inside a turn still opens" line was blind to the corruption
   after the open, and this case is its missing half. Then the pick
   tripwire again and the deletion if nothing reaches it. 5e's slices
   c05e1178 and dd067e06 landed as a40f3d52 (run_port_parser ok cold).
   The fix is d6/stage5-turnmsg 84657ae3 plus e1b5303f (approved
   2026-09-14): restore_turn sets the executor's TLS back and then the
   exact lmx_turn_msg and lmx_turn_running saved on entry, at run_one's
   three exits, around exec_unbound_close's borrowed identity and in
   exec_stop's clear (the live-check loop's pair on a context thread
   unchanged, no outer turn there); cases "exec foreign turn" and "exec
   foreign stop" red on the unchanged code ("before=3 after=0") and green
   after; the host-pick tripwire then unreached by port_message and
   lmx -Suite Message, so the pick's deletion follows as its own commit
   with the tripwire's measurement. 5e's slice 4 (185bb9d3, d_trailer_
   resolve, 14 stages, the falsifier at 6 mismatches on three inputs) and
   the notes for slices 2 and 3 (03d9aeff) are pushed for the lead's merge.
   Landed after the machine's reboot (2026-09-14; every session came back
   under a new name, the roles re-identified, nothing pushed was lost):
   the turn-identity pair 75216d52 merged as ac861a5b over d7835dc7
   (run_gates 11 of 11; parity with the foreign-turn and foreign-stop
   lines; entry_turn 24; run_lmx, run_l2trans, run_l2_message_root green);
   5e's slice 5 with its note (9a50dae1) merged as b3b13a86, so the parser
   port's Stage d (39 functions, one unit) is complete on integration and
   Stage e's plan is accepted; the host-pick deletion 10523360 (approved:
   lm1 and lm2 identical, no reference to the deleted names left) in its
   merge measure; 0c's never-run runners fixed (ee35e67a: path_storage,
   storage, owned_ranges refused an empty -ArgumentList since their first
   commits on 09-11) and the l2trans int-alias local fix (a table of
   foreign int aliases with their p0.h lines, consulted everywhere the
   classifier and the formal and return codes need it; the single-name
   form refused) queued behind it. Found by 0c's cold survey: two parity
   runners outside run_gates' defaults were red unnoticed, run_port_msg_
   blocks (an extra free on the parity side: the bootstrap's bind-wait
   record reaped at run_entry_turn's unbind since stage 5 (a), counted by
   the process-wide free wrap of a link that opens a library unit; the
   module's disposal identical) and run_port_msg_path_storage (the
   pre-test allocation count moved with R0 and the UI child); both fixed
   runner-only by isolating the module under test; the survey's section
   (e) decides which non-default runners join run_gates.
   The host pick deleted 2026-09-14: 10523360 merged as 849f6bdd over
   b3b13a86 (lm1 and lm2 sched_step identical: the `p = 0` branch gone, a
   missing slot INVALID, the in-turn sibling pick unconditional;
   lmx_msg_sched_pick_host_child with its cursor argument, the
   after_sched_snap hook and the exec selftest's snap case deleted; no
   deleted name left in code; run_gates 11 of 11, parity 100 methods,
   entry_turn 24, run_lmx, run_l2trans, run_l2_message_root green). 3c-2b's
   remaining C in the parent's step is gone with it.
   Step (e) as designed by the lead and accepted (2026-09-14; measured on
   the host-pick tree: the runtime-level policy cells host setters write
   are rt->clock with clock_test (set_now, host_is_owner alone, 16 callers
   in main) and rt->orphan_retain (set_orphan_retain, no check, 2 callers),
   read by lmx_msg_now and lmx_msg_orphan_deadline; no runtime liveness
   deadline cell exists, the poll takes its threshold as an argument):
   a new runtime unit lmx_root_record.lm2 with three cells (clock,
   clock_test, orphan_retain) as owned cells in R0's arena rooted there,
   accessors shaped like lmx_sched_record, created lazily on R0's lane at
   the first successful set_now or set_orphan_retain (Q2 (b), corrected
   2026-09-14 after (a) was measured impossible: an L2 runtime unit's
   external wrapper opens a library runtime through runtime_new on its
   first call, so no function of a runtime unit may run on runtime_new's
   or runtime_delete's path, the rule lmx_sched_record already states for
   its own open path and the reason the scheduler record is created at the
   parent's first step; measured as "library open failed ...
   lmx_root_record_new" in the executor selftest); while the record is 0
   the readers take today's defaults (the real clock, LMX_MSG_ORPHAN_RETAIN)
   and run_port_msg_path_storage's pre-test pin stays at 3; set_now and
   set_orphan_retain keep their names and refuse unless R0's lane holds
   (holding_turn(R0), or the host outside any turn while the bootstrap is
   the host), nothing written on refusal; the readers go through the
   record; the three LmxMsgRuntime fields go; clock_test stays a cell (a
   clock of 0 meaning "real" would be one number with two meanings);
   lm1 and lm2 mirrored where lm1 changes. Acceptance written first:
   fable/stage5e-acceptance 980ebc3a, tests/lmx_model_root_record_5e_
   selftest.lm1, "15 checks, 5 failures" on 4a92659b (red on the
   child-turn refusals, the worker's set_orphan_retain and the
   unchanged-clock lines; the cells' location lines added by the lead with
   the accessors). The l2trans int-alias local fix landed as a4680548 over
   4a92659b (the table l2_int_alias with each alias's p0.h line, consulted
   by the classifier and the formal and return codes; the repro with two
   aliases, each removal red only at its own local; the historical input
   pin at 140; run_gates 11 of 11, parity, entry_turn 24, run_lmx,
   run_l2trans, run_l2_message_root 140 inputs, run_port_parser cold). (e)
   is 6cb55982 on d6/stage5e (reviewed: lm1 and lm2 identical, 22 lines;
   the acceptance the tenth default; lane falsifier "15 checks, 6
   failures"), with a fix-up commit pending for the unit's stale header
   comment, the acceptance's location lines with the root_attach
   falsifier, and one gate fix found on it: run_sched_record links its own
   copy of lmx_sched_record and no other runtime unit, so a second unit
   called by lmx_message left "undefined reference to lmx_root_record_*";
   Build-L2RuntimeUnits gains -Exclude and run_sched_record passes its own
   unit (one copy of each unit in the link).
   (e) landed 2026-09-14: d6/stage5e 275b50f9 (the acceptance merge,
   6cb55982, the fix-up) merged as f59a79c7 over c0d743b3 (5e's Stage e
   slices 1 and 2). Measured on 275b50f9: the acceptance 22/0 with the
   location lines; the root_attach falsifier "22 checks, 5 failures" (the
   RETAIN-root line, the post-collection line and its read-back among
   them); the lane falsifier "22 checks, 6 failures" (both setters from
   the child's turn and from the worker's, the clock twice); run_gates 11
   of 11. On the merge: run_gates 11 of 11, parity 100 methods with no
   library-open failure, scenario36's ten with root_record_5e 22/0 and
   release-17 51/0, run_port_msg_path_storage 541/0 (pin 3), entry_turn
   24, run_lmx, run_l2trans, run_l2_message_root 140 inputs; no clock,
   clock_test or orphan_retain runtime field left (a tripwired regex).
   The landing's field tripwire matched rt\clock only: tests/lmx_msg_slots_
   selftest.lm1 L59 wrote other\clock: 97U as the sentinel for L76's
   "unrelated owner unchanged" memcmp, and neither slots runner is a
   run_gates default, so run_port_msg_slots and the L1 run_msg_slots both
   compiled red unnoticed (found 2026-09-15 by 0c's step-2 cold run of the
   32-gate default set on 9ad7e6bf: 26 green before it, the 5 skipped ones
   green separately). Fixed by d6/slots-sentinel c57634f1 off 402b2919
   (other\root_seq: 97U; measured red at 402b2919 on both slots runners,
   278/0 green on both after; lmx_msg_slots reads neither root nor
   root_seq, so the sentinel's job is a non-zero byte a zeroing write would
   flip, and its falsifier is slots_n zeroing its argument's root_seq, that
   memcmp red). Rulings (2026-09-15): the grep for a deleted runtime field
   names the field on any receiver, not rt alone (the lead's tree grep,
   tripwired with non-rt receivers); run_msg_slots stays opt-in, since the
   parity runner's reference build compiles the same test against the L1
   module, so the step-2 default set is 32 with run_port_msg_slots among
   them (falsifier: without it the run reports 31). Lands after
   IndentStack and sizeof in the lead's chain; 0c's step-2 time is
   measured cold on that integration.
   Step (f) scope given to the lead (2026-09-14, design to follow in the (a)
   format after (e) lands): runtime_delete becomes R0's close, the
   decision-17 chain from R0 down (its whole subtree released, running
   descendants and retained orphans included, never at process end), then
   R0's slot, then the runtime; today's slot loop over rt->slots is the
   measure of what the chain must reach (a residue is a leak, not a loop
   to keep); the host forms retire: host_is_owner as an authority (each
   API says what R0's lane means without it and what stays a host-thread
   check for OS primitives), the host lock and host_wait ((b)'s Q4 revisited:
   R0's lane wait or the loop's), the ingress V0 flag and runtime_shutdown
   if R0's close covers them; the lifecycle authority's host form goes, so
   family_handoff's section-34 half takes the strict form (the chain's
   first step inside P's adopt of C, the per-step identities read from the
   end state or a TEST-only hook in settle_child); (f) leaves runtime_new
   and runtime_delete as the only host calls, for the R0 loop step after
   it (one input per turn, drain and maintenance between turns, the UI
   child's turn adapter, the bootstrap main inside R0's loop). Acceptance
   first: a running tree (a mapped child on its context, a retained failed
   orphan, a pending UI request) deleted with every slot and arena freed by
   the chain, the free wrap counting exactly the tree's blocks, the worker
   joined; runtime_delete from inside a turn refusing; (b) and (e)
   unchanged.
   The (f) acceptance written first (2026-09-15): fable/stage5f-acceptance
   ffeb1094 off 402b2919, two tests and one runner hook. tests/lmx_model_
   runtime_close_5f_selftest.lm1: R0 with P1 (C mapped onto its own context
   by P1's turn and inside its turn until it sees its closing flag), a
   failed orphan Q retained under the policy (P2 stopped by R0 and released
   from R0's own turn, the strict lifecycle form, clock 5000, retain 100,
   deadline 5100 never reached), U mapped to the UI lane with one request
   pending in the lane's mailbox; runtime_delete with no exec_stop before
   it, two cycles; measured "45 checks, 4 failures" on 402b2919, red twice
   each at "C's turn ended because the chain set its closing flag, not by
   its own deadline" and "P1's closing turn ran on R0's lane before its
   storage went"; the join line (C's done flag set 50 ms after its
   end_turn), the UI line and the balance line are green today, so the
   balance is (f)'s guard against residue, not a red. The balance is ld
   --wrap on malloc, calloc, realloc and free over every linked object,
   measured over two cycles because the first pays the L2 library runtime's
   open: the live count after the second delete equals the count after the
   first, exact with no magic number. tests/lmx_model_runtime_close_5f_
   turn_selftest.lm1: delete from inside R0's turn on the host thread and
   from a child's turn on a worker refuses INVALID with the runtime alive,
   from the host outside any turn OK; translates on 402b2919 and fails gcc
   "void value not ignored as it ought to be" at its three sites
   (runtime_delete is a sub today). run_model_scenario36.ps1: a test that
   defines __wrap_NAME is linked with -Wl,--wrap=NAME for each such name (a
   missing hook is a red link). Questions the acceptance settles, for the
   lead's design in the (a) format: Q1 runtime_delete returns a status,
   OK from the host outside any turn, INVALID from inside any turn with
   nothing touched (every caller today is a statement; the sub form is
   unobservable); Q2 a retained failed orphan at R0's close is reclaimed by
   the chain, no adopter remaining once R0 closes and its arena handoff-safe
   (waiting for a retention deadline on a clock nobody advances is a hang);
   Q3 runtime_delete is R0's maintenance with the drain's authority, stepping
   its children until the tree settles (a host-run child's closing turn is
   the red line; dropping binds and freeing slots is what decision 17
   forbids); Q4 the mapped child's turn ends on its worker after the chain
   sets its closing flag and the worker is joined before the runtime is
   freed (green today through exec_stop, must stay green when the host
   forms retire). Left to the design: what stays callable from main once
   the host forms retire (the acceptance calls create with parent 0,
   host_post, host_drain, set_now, set_orphan_retain, exec_bind, drive,
   emergency_cancel and root_turn from main; whatever moves into R0's turn,
   the test moves as d's steps did), and the order of R0's own slot, the UI
   lane and the runtime record at the end. Falsifiers on the (f) tree: the
   join deleted before the runtime's free makes the join line red; one
   free dropped on the close path (a slot's path array) makes the balance
   line red by that block.

## 4. Acceptance

- The 19.29.6 implementation/integration checks, as L2 tests: concurrent
  add requests to one owner produce the expected total; an execution trace
  follows the recorded admission order; own turns never overlap;
  independently mapped Messages execute in parallel. No fixed race winner is
  asserted, no success is fabricated from staging, no application lock or
  global manager is added to pass them.
- Model scenarios §31-§36: success/stop/failure, a parent with running=0
  still acting as lifecycle endpoint, orderly close of a family, liveness
  timeout closing a child, failure handoff of HISTORY roots.
- Every test has a tripwire that goes red (RUNTIME_L2_PORTS.txt discipline).
- The full self-build (`buildCore`, `run_lmx.ps1 -Suite Full`, the lead's
  gate list) passes on every merge; the whole project is committed each time
  it does (decision 16).

## 5. What the existing ports are for now

`lmx_message.lm2` and the fourteen module units stay in the tree as
translator evidence with their runners, and as the L1-shaped bootstrap while
stage 1-3 land. They are not extended. When a stage of the new runtime
replaces a bootstrap module, that module's runner is retired in the same
commit and the notes say why.

## 6. Coordination

Gate policy (2026-09-14, from 0c's runner survey, l2src/RUNNER_HAZARDS.txt):
run_gates' default set is the eleven of decision-18 days plus the 18 port
parity runners, run_entry_turn, run_graph_abi and run_turn_step_child_copies,
because two parity runners outside the defaults had been red unnoticed and a
gate that runs nowhere is not a gate; the L1 module selftest runners stay
opt-in as the port runners' reference half (retired under decision 12 where
the duplication is exact); run_l2trans, run_port_parser and
run_l2_message_root stay outside run_gates and required in the lead's landing
chains. Runner rules: no build step skipped on the presence or timestamp of
an output (rebuild, or a content-hash key); a missing golden or oracle is red
with its name, never a pass; launches through cmd /c with $LASTEXITCODE where
a runner spends its time in launches; a pin of the runtime's own bookkeeping
(a pre-test allocation or free count) moves with the runtime commit that
moves it, never silently. Fix order: the reuse sites, then the default set
with its measured time, then the launches. Added 2026-09-15: the tripwire for
a deleted runtime field greps the field on any receiver, not rt alone (the
slots sentinel other\clock survived (e)'s landing, see item 5 (e)); and
run_port_parser's link lines go through a gcc response file per link, written
by Invoke-Gcc itself beside the stage's objects and kept as evidence, because
cmd.exe refuses a line of 8192 chars (measured by 0c: 8152 runs, 8192 fails
before gcc starts; e_appenders' dumplink is 7048 chars with 99 -D redirects,
so the limit sits near 121 entries and e4b, e5, e6, f and g are still to
come). A generated #define header was rejected: a second source of truth for
the redirects, seen after the unit's own includes where -D applies before its
first line, the order Stage d's oracle-named headers rely on. 0c's runner
change, landed via the lead before e4b, red first with a permanent padding
stage whose inline line exceeds 8192 chars (red with "The command line is too
long." before, green after, kept as the guard at one gcc call).

Translator repairs driven by the parser port's Stage e (2026-09-14/15, the
lead, each a leaf in run_l2trans's historical set with its pin, red on the
previous translator and green after, in landing order): the int-alias
local (a table of foreign int aliases with their p0.h lines, pin 140); the
nested index in an actual (`text[index[0]]` had split into four actuals:
l2_index_span counts a balanced index at the head and operand sites of
l2_expr_span, pin 141); LmP0IndentStack retired from its legacy codes 13/14/15,
field table and sizeof/new_zero cases to the foreign pointer path spelled as
written, the dead code-13 comparisons deleted, `stack\columns[idx]` a leaf,
pin 142; `c.sizeof(name[0])` renaming the leading identifier of a formal or
slot and keeping the index, pin 143; the lm_own_resize, lm_own_copy_bytes and
lm_own_delete paths splitting actuals with l2_expr_span instead of one field
per actual, which had silently truncated an unparenthesized length (latent in
landed code, hit by the oracle's usual spelling at the first port; the
emitted text and a read-back as the reds), pin 144; the own-allocation temp
typed as the prototype's `@: void` with the source's cast the only type
(U4: the "size_t" text sniff gone; an uncast call typed by its assignment),
pin 145; the unreachable lm_p0_set_diagnostic handler deleted under
decision 12. Rule learnt: every special-cased call form in l2trans that
re-parses actuals on its own is a defect waiting; actuals go through
l2_expr_span. Named for its own ticket: U4's remaining default that casts
any other lm_own_new_zero argument to `@: LmP0Text`; a probe's
`lm_own_resize(stack\columns, ...)` failing with no located diagnostic.
The uchar branch (d6/uchar-local off ea7fa1ce, three commits, 5e's leaf
entering with commit 1 at pin 145 and U4 moving to 146; commits 1 and 2 red
on that leaf by design, measured and landed as a whole at its tip): commit 1
the uchar local (an unsigned-backed cache spelled uchar), commit 2 the
bare-atom cast `(cast: uchar c)`, commit 3 ruled 2026-09-15 as `"<ctype.h>"`
joining the unit writer's fixed include list beside stdlib and string (the
flags l2_need_string/l2_need_stdlib feed only the entry-program writer;
units get their includes from l2_emit_unit's fixed line, and nothing brought
ctype), no name table and no flag under decision 12, the prototype's
l2_is_known ctype table out with it; if run_l2trans's historical set compares
emitted text, the goldens' regeneration is in commit 3 with one include-line
hunk per golden and nothing else. Two more gaps from b5, queued after the
uchar branch and before U4 and the handler deletion, in this order: the
expression length (sonnet/parser-l2 af150cd6, the oracle's 318-character `||`
condition in lm_p0_scan_builtin_compact_atom_piece fails "expression too
long" at l2_cat's fixed 256-byte buffer; blocks e4b; pin 147; ruled: the
buffer is sized from the span it concatenates, not a bigger constant, and a
doubled condition in a scratch probe must still translate), then the
zero-argument cross-unit prototype with a pointer return (10f294a2,
`block_event: p0_stream_event_new()` "unsupported body" cross-unit while the
same call works in-unit and an int return works; blocks e5, WIP on a side
branch; pin 148; the leaf's note names which difference drives it, the zero
formals or the pointer return, measured by two scratch probes before the
fix). U4 and the handler deletion follow at 149.

- Lead (`lingvamyxa-d6`): translator gaps found by this lane, with fixtures
  as before; merges through integration; the model's §31/§34 wording carries
  decision 14; the full gate on every merge.
- Helper (`lingvamyxa-5e`): parser in L2 (§6.5) unchanged.
- Review chat: this lane, stage by stage; decisions carried to Mikhail;
  spec clarifications committed as docs the day they are made.
