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
   measured cold on that integration. Landed as 1542217d over 95042370
   (slots parity PASS 278/0 on both runs, O0 slots 278/0, run_gates 11 of
   11 in 352 s); 0c's step 2 measures on 1542217d.
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
   line red by that block. Edited as f79320de after Mikhail's I1 overrule
   (the join line relabelled "C's closing turn had ended before
   runtime_delete returned: the close settles C only once C is settled, and
   C's thread frees its own state and ends on its own" over the same
   check; the balance read once the live count is stable for 200 ms, 3 s
   cap), remeasured on 402b2919: 45 checks, 4 failures, the same two red
   lines; (f) builds on f79320de. Held 2026-09-15: Mikhail questions
   why R0 should wait for a child's settle at all ("what for?"; the
   executor frees its own memory; each L3 Thread has its own scheduler and
   GC; no shared locks, only the mail queue's own collection lock), which
   would make R0's close its own closing end_turn with no wait after it
   and contradict 995ff339's "settled children are reclaimed by the chain,
   and only then the root's own storage and the runtime go" and the
   acceptance's ordering line; the coordinator's reading put to him
   through the lead: R0 waits for no other L3 Thread, but the Messages
   without a thread of their own belong to R0's scheduler, which settles
   them before R0's thread frees what is R0's, and today's shared executor
   state (lock, TLS, slot table behind rt) is freed by the last thread
   that leaves it, by a count; if confirmed, the acceptance drops the
   ordering line, waits for C's own done flag before reading its reason,
   keeps the stable-window balance as the property itself, and the spec
   sentence is rewritten under his name. No code until his answer.
   The (f) design (d6/stage5f-design 18f9ba54, STAGE5F_DESIGN.txt on
   ffeb1094; measured on 402b2919: no closing turn runs in today's delete,
   the chain exists as request_children_close, settle_child, parent_settle,
   orphan_sweep and drive_one, root_turn is run_entry_turn(R0), the host
   forms have 2 + 4 test callers and host_is_owner 30 lines): runtime_delete
   returns int, INVALID from inside any turn or off R0's lane; the close is
   R0's maintenance: request_children_close(R0), then a loop of internal R0
   turns stepping closing children (host-run bound by run_child_turn on R0's
   lane, unbound by exec_unbound_close, mapped ones ending on their workers
   as they see closing), after each turn the reaped workers joined, settled
   non-orphans settled from R0's turn and settled orphans reclaimed
   regardless of orphan_until; end order the transport list, R0's own slot
   (its arena and the root record), exec_detach, free(rt), no host_detach;
   the host forms retire with their test callers; the lifecycle calls drop
   the host branch and 48 test sites move into the parent's turn; the
   main-callable set is R0's lane outside a turn (create with parent 0,
   host_post, host_drain, set_now, set_orphan_retain, exec_bind, drive,
   emergency_cancel, root_turn). Answered 2026-09-15, all the lead's
   recommendations: Q1 (a) a UI-bound child is unbound and closed by
   exec_unbound_close, no handler, the pending request going with the
   lane's mailbox (the UI child's turn adapter belongs to the R0 loop step);
   Q2 (a) block on the bind waits with no timeout (19.29.8: a timeout never
   grants permission to free memory still used by native code); Q3 (a) R0's
   mailbox refuses admissions with STOPPED under its own mail lock before
   the chain starts (19.29.6 (iii)), a post after the delete returns being
   the embedder's use of a freed handle; Q4 (a) runtime_shutdown deleted
   under decision 12; Q5 (a) the bootstrap-thread identity named once,
   lmx_msg_r0_lane, any-thread rejected for concurrent maintenance; Q6
   confirmed. Added measure: a mapped child blocked inside recv with an
   empty inbox must be woken by the close request as by an admission (a
   scratch probe; the fix, if missing, in request_children_close's ready).
   Falsifier for Q2 in the commit's note: a mapped child that never reads
   its closing flag holds runtime_delete (a 3 s watchdog reporting "held"),
   the same probe reading the flag returns. Conditions: spec quotes from
   origin/main before final; lm1 and lm2 mirrored in one commit; each
   migrated site keeps its label and count with its runner run on the
   landing; the two acceptance tests join scenario36's defaults; the
   tripwired grep at 0 for the retired host forms on any receiver. Lands
   after the chain and the response-file branch on its own integration
   merge. The design is final at d6/stage5f-design e5fd252a (2026-09-15;
   the spec quotes from origin/main including 995ff339's paragraph, the
   rulings, the Q2 falsifiers, the reshaped wake probe). The close-wake
   probe measured on 402b2919 plus ffeb1094: a mapped child whose first
   turn ended and whose worker idles (g_turns = 1 and exec_live = 0; the
   first predicate used running_load, which end_turn clears only on
   completion, and failed its own precondition) is woken by
   request_children_close's own exec_ready and runs its closing turn, 6/0
   in two agreeing runs; the falsifier, that ready call removed (13 to 12
   calls) in a copy under l2src so its imports resolve, reports "held", 6
   checks, 1 failure. No fix needed there. Ruled: the probe is committed
   with the (f) implementation as tests/lmx_model_close_wake_5f_selftest
   and joins scenario36's defaults, so the wake it measured cannot regress
   unguarded. The lead's queue after the chain (sizeof, slots): 0c's step
   3, port-parser-cwd 673c3475, actuals ea7fa1ce, uchar 80b0a77b, then the
   (f) implementation. Implementation notes before the code (10d313e7,
   nothing measured): I0, Mikhail's correction of 2026-09-15 ("running=0
   is set, and at the end of its turn the closing Message does the same to
   its children; that is all"): R0's close adds no path of its own, it
   marks its children exactly as end_turn's closing branch does (lm1
   1781-1790, 1828-1837: running=0 and ready on each child, then
   request_children_close), each child doing the same to its own children
   at its turn's end; I1 as overruled by Mikhail the same day ("the
   executor frees its own memory"; "each of our L3 Threads has its own
   scheduler and garbage collector"): the close waits for no thread and
   frees no thread's state; a context thread ends its closing turn, its
   Message settles, then the thread frees what is its own and ends; R0's
   close settles a child only once the child is settled and frees only
   what R0 owns; in today's C executor the lock and TLS live in the shared
   LmxMsgExec behind rt, so a thread's last touch of that state is the
   write that settles its Message (native_leave_addr, exec.c 2442-2453)
   with nothing read after it, which is false today (requeue_if_runnable
   re-locks, the worker loop re-locks at its top), so in (f) a thread whose
   Message is STOPPED, DEAD or RELEASED after its turn exits right after
   the settle without re-locking and frees its own wait state (take_this
   already refuses such a Message, 2958); Q2's "block on the bind waits" is
   superseded: the close waits on the child's settle signalled on R0's
   lane, never a thread handle, the falsifier unchanged; the acceptance's
   join line is relabelled "C's closing turn had ended before
   runtime_delete returned" over the same check, and its balance reading
   waits until the live count is stable for 200 ms (3 s cap), since the
   threads free their own state after the close returns; falsifiers: a
   re-lock after the settle in the exit path turns the acceptance red
   (balance or use after free), and a thread that never frees its wait
   state shows as one block in the balance; I2, the
   wake probe committed as tests/lmx_model_close_wake_5f_selftest in the
   defaults; I3, host_is_owner's remaining sites become lmx_msg_r0_lane
   (mapping_authority_locked, exec_bind_mode, start_contexts,
   unbound_close, exec_stop, run_entry_turn, set_orphan_retain,
   require_owner, require_turn, set_now), the five lifecycle calls and
   lifecycle_authority dropping the host branch; I4, the migration sites
   by grep: the executor selftest 40 lines, the message selftest 12,
   release_17 10, liveness_33 3, family_handoff 5, orphan_mapped_17,
   scenario36 and runtime_close_5f 1 each; runtime_delete's 598 statement
   calls keep their form; host_wait's four uses and runtime_shutdown's two
   migrate to the refusal and host_post's STOPPED. Uchar merged as 85c6fd0e
   (run_gates 11 of 11 in 222 s, the other runners still running), then a
   quiet window for 0c's 32-gate cold timing.

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
long." before, green after, kept as the guard at one gcc call). Implemented
by 0c as claude-0c/port-parser-rsp cd8c9028 (Invoke-Gcc writes the .rsp
beside the -o target, each argument quoted with \ and " escaped, runs gcc
@file; the guard at 8823 inline chars, the red now "gcc did not start: cmd
/c refused a 8876-character line (cmd.exe's limit is 8191)", the largest
real line 186 chars), with two commits on top: the working-directory rows
(L839, L876 absolute) and a quoted -D value checked by a negative-size
typedef; measured 2026-09-15: a dropped \ escape is caught by any
backslash path in the guard's own arguments (the mangled empty.c path, the
same red with or without the quoted entry), and the quoted entry alone
catches a dropped " escape (measured: gcc "<command-line>: error: stray
'\' in program" while parsing the define, before the size check is
evaluated; without the entry the run is green end to end, nothing else
carrying a quote), so the two failure texts are distinguishable in the
logs; the branch is claude-0c/port-parser-cwd 673c3475 on 7a1eb5fc on
cd8c9028, the guard line "cmdline guard ok: 137 arguments, 8855 characters
inline, through a response file", green from the worktree root in 318 s;
landed as 458c8875 over dbcc3f88 (run_port_parser cold from the baseline
exit 0 with the guard line and 18 stage ok lines; 57's two escape
falsifiers rerun on the merge). 0c's step 3 (64df50cc, the 12 runners
launched through cmd /c) landed as dbcc3f88 over 1542217d: the
history_owned falsifier 16 s before and 2 s after at "history checks=65
failures=0", each two-line runner printing its O0 and O2 lines at 0
failures (slots 278, storage 77, path storage 541, owned ranges 439), the
other eight at 0 failures, run_gates 11 of 11 in 221 s.

Translator repairs driven by the parser port's Stage e (2026-09-14/15, the
lead, each a leaf in run_l2trans's historical set with its pin, red on the
previous translator and green after, in landing order): the int-alias
local (a table of foreign int aliases with their p0.h lines, pin 140); the
nested index in an actual (`text[index[0]]` had split into four actuals:
l2_index_span counts a balanced index at the head and operand sites of
l2_expr_span, pin 141); LmP0IndentStack retired from its legacy codes 13/14/15,
field table and sizeof/new_zero cases to the foreign pointer path spelled as
written, the dead code-13 comparisons deleted, `stack\columns[idx]` a leaf,
pin 142, landed as fb58feb6 over 402b2919 with 609f5028 (run_graph_abi links
the foreign allocator by run_l2trans's exact rule; measured on the merge:
retired greps 0/0/0 tripwired against 19/9/3, run_gates 11 of 11 in 367 s,
graph_abi cold PASS with the linked count at least 1, l2trans ok,
message_root 142 inputs, port_parser cold ok); `c.sizeof(name[0])` renaming the leading identifier of a formal or
slot and keeping the index, pin 143, landed as 95042370 over fb58feb6
(run_gates 11 of 11 in 379 s, graph ABI 63/0 with 75 failure positions
exercised, l2trans ok, message_root 143 inputs, port_parser cold ok); the lm_own_resize, lm_own_copy_bytes and
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
hunk per golden and nothing else (measured: no runner stores a golden of
emitted unit text, nothing to regenerate). The branch as pushed,
d6/uchar-local 80b0a77b (1b2d4620 uchar as type code 42 through the
own-local paths with the leaf unit_uchar_local exit 10 and the historical
set at 145; 0f07f1de the four-line bare-atom cast; 80b0a77b the one token
in l2_emit_unit's fixed include line), reviewed 2026-09-15 and approved to
land as a whole: l2_ctype_word is pre-existing (multi-word C type words),
no ctype name table or flag remains; full run_l2trans on the tip 145 cases
"l2trans gen2 ok"; the reds per commit as ruled ("unsupported body,
frame=uchar" then "unsupported cast type" at 6:13, then gcc's implicit
declaration of isalpha, then green); the landing falsifier: "<ctype.h>"
removed from the fixed line turns exactly the uchar leaf red. Two more
gaps from b5, queued after the
uchar branch and before U4 and the handler deletion, in this order: the
expression length (sonnet/parser-l2 af150cd6, the oracle's 318-character `||`
condition in lm_p0_scan_builtin_compact_atom_piece fails "expression too
long" at l2_cat's fixed 256-byte buffer; blocks e4b; pin 147; ruled: the
buffer is sized from the span it concatenates, not a bigger constant, and a
doubled condition in a scratch probe must still translate; measured
2026-09-15: the reader takes the whole file, parser.lm1:4573 mirrored at
parser.lm2:5393, no line bound, a 74 KB one-line condition reaches l2_cat
whole and every length past 255 fails "expression too long" at 2:232, so
the shape is owned growable text with its capacity carried along the
expression path and freed on it, across 27 fixed 256-byte arrays in 15
functions, 37 l2_cat sites and 14 refusals, inventoried as 4 whole-expression
accumulators, about 9 carriers and 10 short pieces; the rule by origin: a
buffer stays fixed only if every byte in it is generated by the translator
itself, anything copying source text, identifiers included, is growable;
b5's data: slices f and g nothing over 200 characters, e5 six conditions up
to 345 with argument lists near 310, nothing over 512, so e5 needs 147 and
148 both; the stages' line ranges are in PARSER_L2_PORT.txt since
sonnet/parser-l2 b4a8de83, with an unassigned list (the registry family,
the trailer-role/scanning family, the capstone, the external API surface)
and e3 corrected into two clusters, 366-408 and 2081-2481; found on
c1dc9886 (g accepted): run_port_parser's $Stages carries neither f nor g on
any branch, so their "clean" was translate plus compile only; b5 wires
f_postprocess and g_validate, red-first for g without 80b0a77b, with one
mutation per stage proving the goldens reach the ported functions, or
reports that they do not (the validate family's only outer caller, oracle
1690, sits in unported disabled-block code); the leaf set adds a 2218-character condition and a 300-character
identifier probe measured today first: locals are unbounded, two
300-character names distinct with no truncation, so nothing to fix there;
method names and formal names are capped at 62 by three explicit bounds,
l2trans.lm1:5133 entry formals, 9609 method names, 9632 method formals,
refused "name too long" (frame=fn) with a located line, never truncated;
ruled 2026-09-15 as their own pin, not part of 147, because the storage
behind those bounds is a different mechanism from the expression buffers
and the refusal is honest and blocks nothing measured; its place: b5
measures the longest method name and formal name in l1src/parser.lm1 over
the slices still to port, over 62 puts the name pin at 149 right after 148,
otherwise it follows U4 and the handler deletion (measured by b5
2026-09-15 over e4b to g: the longest method name 49,
lm_p0_validate_nonempty_colon_frames_in_structure at parser.lm1:4512, the
longest formal 23, the longest port-header prototype 33, none at or above
63, so the name pin follows U4 and the handler deletion and blocks nobody);
field names and type names are probed in that pin's design; until it lands the 62-byte bound is
a labelled gap, not a language limit), then the
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

## 7. Removing the executor's shared lock (Mikhail, 2026-09-15)

Mikhail's order, relayed by the lead and confirmed by the coordinator the
same hour: "Stop everything immediately and put all effort into removing
this shared lock. It is not needed." Earlier the same day: "There must be no
shared locks. The mail queue has only its own internal collection lock,
that's all"; "each L3 Thread has its own scheduler and GC"; "the executor
frees its own memory." Everything else is held where it stands: the uchar
merge 85c6fd0e unpushed (integration stays c067bed9), stage 5 (f) unwritten
(d6/stage5f-impl ee8b80c9 empty, the acceptance f79320de unmerged), 0c's
step-2 timing, b5's leaf tickets. Size on c067bed9 (lmx_msg_exec_lock( and
*_locked( calls): lmx_message_exec.c 165, lmx_message.lm1 65 (lm2 65),
lmx_message_exec.h 9, the executor selftest 5, the model tests 29 across 7
files. Division: the lead inventories exec.c (LOCK_REMOVAL_INVENTORY.txt in
wt5f on c067bed9), b5 lmx_message.lm1 (LOCK_REMOVAL_INVENTORY_LM1.txt), 0c
the tests, runners and the -LaneCheck oracle with the gate per removal step
(LOCK_REMOVAL_INVENTORY_TESTS.txt), the coordinator the spec and model map
(l2src/LOCK_REMOVAL_SPEC_MAP.txt on main: where 19.28.R2.2, 19.29.6 and the
model's sections 25 and 29 already say who owns each piece, and where the
spec is silent). One format for every inventory line: file:line, enclosing
function, the state read and written under the lock, the lane today
(bootstrap/R0, the Message's own context, a parent's lane, any OS thread),
and the target owner by the rule (the Message's own fields on its own lane;
the parent's scheduler state as the parent's Structure data; cross-lane
only through a mailbox with its own collection lock) or "silent", the
silent lines being the design's questions. Rule: no lock line goes before
the inventories and the map are in and the design is written in the (a)
format, stage by stage, each stage red-first with the lane tripwire as its
proof (every write the removed lock covered becomes a lane write on its
owner's lane; -LaneCheck the oracle that no write moved off its lane); the
mail queue's own collection lock is the one lock that remains.
Added the same hour (Mikhail, verbatim, to the lead's line that children's
threads still use shared memory inside rt): "значит реализуйте полноценную
арену для каждого MEssage, как описано в спеке. Без этого Message не имеет
смысла" (so implement a full arena for each Message, as described in the
spec; without it a Message makes no sense). The two directives are one piece
of work: the inventories' target-owner column names the Message's own arena
where the state belongs, not "the Message's fields" in a calloc'd record.
On the end of the process (Mikhail, verbatim, to the lead's note that
children's closing turns still running are cut off with the process):
"подобное завершение детей это всё равно аварийный выход. Утечка памяи из ОС
будет или нет?" (such an ending of children is an abnormal exit anyway; will
there be a memory leak from the OS or not?); the lead's answer, confirmed:
no, the OS reclaims a process's memory, handles and threads at exit, so only
the unfinished closing work is lost. The lead's exec.c counting unit: 77
lmx_msg_exec_lock( acquisitions and 88 _locked( lines (22 helpers running
under a caller's lock), 165 together, one inventory line per acquisition and
one per helper.
Later the same day (Mikhail, verbatim): "блокировка только локальная у
коллекции сообщений может быть, больше блокировокне требуется даже локальных
-- их в проекте не длолжно быть больше вообще" (the only lock there may be is
the local lock of the message collection; no other locks, not even local ones,
anywhere in the project). The removal's target is therefore every lock in the
project except the message collection's own, not only the executor's.
His order on the spec's decision-18 item (5) ("Runtime-level structures under
the executor's lock belong to no Message: the root list, the slot list, the
retire queue, a lane's own queue"): find who wrote it, delete it at once, and
add to the docs only instructions agreed with him. Found: written 2026-09-14
10:10 -03 by the review chat (the coordinator's session, then e2) in commit
0c875197 as its own consequence of his one-arena, one-lane, one-writer
clarification, never his words; the lead's session copied it into
INTEGRATION_GATE_STATUS 4 s later (4baa4474). Deleted from the spec, the
model's section 29 and the gate status. Open for him, not deleted without his
word: item (4) of the same list (the coordinator's sentence, "Primitives:
mailbox admission under its lock, reference counts, wait signals"), and the
coordinator's consequences in the stage-5 (f) paragraph of spec 19.29.6 and
the model's section 32 marked "Mikhail may overrule" (the root's close
settling children before the root's storage and the runtime go, the UI child
closed without a handler, the root's mailbox refusing admissions, the
bootstrap thread as the root's lane, the strict lifecycle form, the retained
orphan reclaimed at the root's close, and the 19.29.8 cross-reference).
Docs rule (Mikhail, 2026-09-15, verbatim, on decision 18's ownership clause
(5), spec 11529-11530 and the model's section 29 (5)): "1) попробуй определить
кто и когда сделал это уточнение 2) удаляй это уточнение немедленно 3)
дополняй все доки, но только согласованными со мной инструкциями, после
дополнения пушай и коммить" (find out who made that clarification and when;
delete it immediately; add to all docs only instructions agreed with me,
then commit and push). The answer: 0c875197, 2026-09-14 10:10, the review
chat (the coordinator's session) wrote items (1)-(5) as consequences of his
one-arena-one-lane-one-writer clarification; (1)-(3) restate his words, (4)
and (5) were the coordinator's own sentences and never his. (5) is deleted
(657ce3f9, 07ad3679); item (4) and the coordinator's consequences in the
stage-5 (f) paragraph of the spec and the model stay until Mikhail answers
the lead's question about them, the coordinator's view being that they too
should go, leaving his sentences alone. His question to the coordinator,
"from which of my words did you conclude that a shared lock is needed?",
answered the same hour: from none; the coordinator wrote the L1 runtime's implementation
state (the root list, the slot list and the retire queue under the
executor's lock) into the spec as an exception to his rule, which his rule
excludes, and item (4) is of the same nature. His correction on the word
"prototype" (2026-09-15, verbatim): "Прототипом ядра является спецификация
ядра, а не какой-то код!!! Я это уже писал один раз" (the prototype of the
core is the core specification, not some code; I have written this once
already): no sentence of the spec or the model is derived from what the
runtime's code does; the code is checked against the spec and a mismatch
is a code defect with a ticket here. From now on the spec and the model take only his
sentences, verbatim with the date; every consequence the coordinator or the
lead derives lives in this plan or a design file as a question with options
until he agrees. The coordinator's spec-and-model map for the lock removal
(l2src/LOCK_REMOVAL_SPEC_MAP.txt) is written under this rule: the spec quoted
by sentence and line, today's state outside any arena by struct and
allocation call, and five questions for him (the owners of the root list,
the slot list, the retire queue and a lane's own queue; whether reference
counts and wait signals survive as primitives; the model's section 30 on
execution tables; which arena holds an admitted copy; whether R0's scheduler
steps its threadless children before R0's thread ends), each with a proposed
answer and an alternative, none of them in the spec.
Inventory 1 of 3 (0c, 2026-09-15): claude-0c/lock-inventory e71e64b8,
l2src/LOCK_REMOVAL_INVENTORY_TESTS.txt, 34 lines against the grep (24
exec_lock sites in the model tests and the executor selftest: checks_19_29_6
10, orphan_mapped_17 5, root_ingress_5b 2, root_record_5e 2, the selftest 5;
2 _locked name uses in l2_and_foreign_call_own_local; 8 in the two stage-5 (f)
tests on the impl branch, run by no default gate). Kinds as ruled: MAIL 15
(the worker's result reaches the test through its parent's mailbox, read on
the host lane), LANE 6, JOINED 3 (allowed only for threads the test itself
created; the runtime's workers are joined by no one), NAME 2, silent 8 (the
test-only Interlocked go flags and rendezvous that hold a worker inside its
turn until the host flips a cell: not a shape the model has, since a turn
runs to its end and waiting is between turns on the mailbox; each such test
is restructured into turns exchanging mail in the lead's design, or its
property dropped with a line saying what is lost). The executor selftest's
29/38/29 events and 385 Interlocked are harness state of a C selftest of the
C executor and go with that executor's rewrite; the host selftest's pthread
gate goes with the host forms. Measured gap in the proof tool: -LaneCheck
does not go red when a lane write moves to a thread with no turn (the oracle
returns when the thread-local turn is 0, exec.c 177; run_child_turn's
ready_clear moved onto a joined helper thread ran in both builds and both
runs with exit 0 and no LANE WRITE FAIL), so it never proved the one-lane
rule for that class; ruled: 0c writes the tripwire (the bootstrap thread
recorded at runtime_new; a lane write with turn == 0 on any other thread is
red), red on that mutation and green on the baseline, and no removal stage
cites -LaneCheck as its proof before the tripwire is measured; a lane write
from the bootstrap thread outside any turn must stay green.
Inventory 2 of 3 (b5, 2026-09-15): sonnet/lock-inventory df360bb0,
l2src/LOCK_REMOVAL_INVENTORY_LM1.txt, widened to the order's scope in the same
file: 72 lock-acquisition sites in lmx_message.lm1 (65 of the exec-lock
family, 1 host_lock, 6 mail_lock), lm1 and lm2 mirroring on every lock verb
by grep; 66 to go and 6 allowed (each mail_lock verified to guard only a
Message's own inbox or outbox push); of the 65 exec-lock lines 51 silent, 8
not applicable, 6 fitting the map's M or P, all grep-verified against the
file's own fields. Next for b5 on the same branch: the allocation inventory
of lmx_message.lm1 (every calloc, malloc, realloc and free site with the
record, the lane and the owner code), the arena half of the order.
Inventory 3 of 3 (the lead, 2026-09-15): d6/lock-removal 39435ed2,
l2src/LOCK_REMOVAL_INVENTORY.txt, 165 lines equal to the grep (77 LOCK
acquisitions, 24 DEF lines of *_locked helpers, 64 CALL lines), every line
with an owner code from the map: first codes M 25, P 58, Q 5, T 2, X 45;
30 lines silent alone (3 of them acquisitions), 50 naming a question: 5.1
on 41 lines, 5.2 on 13, 5.4 on 1. The one fact behind 35 of 5.1's 41 lines:
address resolution. msg_at_addr, self_or_find, msg_find_any_locked and
rec_at_addr_locked walk rt->root and every child list, and the lock exists
so that the tree does not change under the walk; the target reaches a
Message only through a capability the caller already holds (the parent's
child pointer, the taking lane's own record, the sender's capability,
19.28.R2.2 11447), so the design names that capability per call site. The
other six 5.1 lines: the root list in drive_walk_list, the retire queue
(try_retire, flush_retire, retire_n) and slot_free. 5.2 is the wake:
exec_wake_locked wakes every worker after any turn (run_one 2587),
exec_ready routes one wake, and walks take refs. Next for the lead on the
same branch: the exec.c allocation inventory, the arena half.
Mikhail's answers of 2026-09-15 on the map's question 5.2, verbatim: "Каких
сигналов ожидания? Снятие блокировки с сигналов ожидания чем-то может
нарушить смысл этих сигналов? Очевидно нет. Снимать всё и никогда больше не
редактируй спеку своими выводами -- всё спрашивай и только согласованное
вноси в документы. Каких ссылок? 1) Message не имеет никаких ссылок на общие
ресурсы кроме тех которые неизменяемые (independents: const: immutable:
хранятся в неизменяемых массивах у root). Либо на атомарные для которых
очередность не важна 2) Очередь сообщений -- синхронизированная коллекция"
(what wait signals? can removing the lock from the wait signals break their
meaning? obviously not; remove everything and never again edit the spec
with your conclusions, ask everything and enter only what is agreed; what
references? a Message has no references to shared resources except
immutable ones, independents stored in the root's immutable arrays, or
atomic ones for which ordering does not matter; the message queue is a
synchronized collection). The "reference counts" of the deleted item (4)
were the refs pins of the C record (walks and waits under the executor's
lock), a companion of the lock, not a Message's reference; they go with it.
Done the same hour, 668a5347: item (4) removed from both copies, the
stage-5 (f) paragraph reduced to his three sentences in both copies, the
19.29.8 line removed; the six remaining questions (the four structures'
owners, section 30, the admitted copy's arena, R0's threadless children, a
capability after its target closes, the coordinator's 2026-09-14
supervision-handoff sentence at 19.29.6 12588-12592) and the lead's held
point (lm2/own.lm2's unused mutex and condition wrappers) put to him
directly in the coordinator's chat.
The lead's allocation inventory and spine (d6/lock-removal 9e26a8c3):
LOCK_REMOVAL_ALLOC_INVENTORY.txt, 75 lines against the grep (23 ALLOC, 52
FREE), all to the C heap, none from a Message's arena, first codes M 30, P
8, Q 6, X 24, S 7; the silent lines are two facts, bind_wait_new's wait
object (question 5.2, answered above: the lock's removal does not touch the
signal's meaning) and launch_ctx_thread's LmxMsgCtxPack allocated on the
parent's lane and freed by the child's thread (a cross-lane free, a design
item of its own; the map's 3.5 corrected to cite it); adopt_push is dead
code (its only occurrence is its definition), deleted under decision 12 in
the first removal stage. LOCK_REMOVAL_SPINE.txt: 97 lookup call sites (exec.c
58 in 40 functions, lm1 39 in 35), each caller named; the three capabilities
do not cover every site: C4, the child's own parent capability (spec 11442:
the liveness queries live_check, live_query, live_handle, parent_gone,
context_worker's live_wait_th read); C5, R0's own management through its
child list (orphan_expired, set_orphan_until, run_entry_turn), C1 with R0 as
the parent; N, about 24 address queries from outside any Message (tests and
the embedder: state, inbox_n, path_n and seg, init_copy, child_n and at,
tracked, endp_refs, handoff_ready, native_users, adopted_*, exec_bind_*,
is_bound, last_status, map_queued, test_*) that hold no capability, each of
which becomes a read on the owner's lane or is deleted with its property
named; the UI take's MAP request carries an address and must carry the
capability (Q). The inputs for the design are complete: three lock
inventories, two allocation inventories, the spine, the map; the design's
first stage names the capability at every lookup site without changing
what is locked, so the tree walks go before the lock does.
0c's tripwire on the lane oracle (uncommitted, wt0c_lock on e71e64b8, exec.c
+34/-1 under LMX_MSG_EXEC_TEST: the bootstrap thread recorded at
exec_attach, a lane write with no turn on another thread aborts) is red on
the unmutated runtime at take_this:ready_clear (a worker marks the record
held_by itself and clears m->ready before it enters the turn identity);
ruled (b): the thread holding the run claim is the Message's lane for the
take by ownership item (3)'s own sentence, so the tripwire admits exactly
that site with held_by equal to the calling thread and nothing else;
whether the take and the turn identity become one write is the design's
question; then runs A (unmutated green), B (the bootstrap thread's writes
outside a turn green), C (the moved ready_clear red) before the commit.
Mikhail on question 5.1 (2026-09-15, verbatim, to the coordinator's proposed
owners): "1) зачем вам список корней?" (why do you need a root list?) and,
to the phrase "a registry of all Messages for the walk at deletion":
"откуда ты взял обход при удалении? Мы выше что обсуждали?" (where did you
get a walk at deletion from? what did we discuss above?). The coordinator's
answer, awaiting his confirmation: there is no root list, the root is one
(R0) and its children are an ordinary child list in R0's arena; there is
no walk at deletion, the close is R0's own closing end-turn as he said; the
slot list served only today's delete loop and the address lookup and goes
without replacement; the retire queue goes (a closed child's storage is the
parent's settle write); a lane's own queue is that Message's mailbox.
The lead's lock-removal design draft: d6/lock-removal 8f20d183,
l2src/LOCK_REMOVAL_DESIGN.txt, opening with Mikhail's verbatim sentences
only and stating it is a design, not the spec; the lock work sorted into
L1-L8 (address resolution, the family tree's writes, the mapping's cells,
the cross-lane control flags, the runtime-level lists, the wakes, the
storage moves at settle, test setters); stages each red-first: S0 the lane
tripwire as the proof tool; S1 dead code and the order's direct deletions;
S2 capabilities instead of address lookups for C1, C2 and C5 with locking
unchanged (red: a test-build lookup counter at 0 from those sites); S3 one
wake per lane (red: an idle lane's empty wakes counted); S4 the family tree
and the mapping written only on the owner's lane, then read without the
lock (red: the tripwire; C3 and C4 decided here; waits on 5.6 and N); S5
the runtime-level lists (waits on 5.1; reap and join go); S6 the lock
deleted (red: exec_lock and unlock built as abort, grep 0, only m->mail
left); A1-A3 the arena, records, bookkeeping and arrays from the Message's
own arena, A3 waiting on 5.4 (red: the two-cycle balance with heap
allocations counted). Reviewed and approved 2026-09-15 to start S1 and S2's
red on a branch off c067bed9, with S0 landing first and every landing from
S2 on running run_gates and run_port_message -LaneCheck with the tripwire.
Mikhail on the map's question 5.3 (2026-09-15, verbatim, to the proposed
rewrite of the model's section 30 as "Message synchronizes only the admission
of its mailbox; there are no execution tables", keeping the snapshot rule for
a parent reading its children's cells): "да, все описания L1 промежуточны и
их нужно приводить к спецификации L2" (yes, all L1 descriptions are interim
and must be brought to the L2 specification). Section 30 rewritten the same
hour as agreed, his words quoted in it; the lock-inversion test paragraph
and the prototype-locks paragraph, both descriptions of the L1
implementation, removed. Standing rule from his sentence: every description
of the L1 implementation in the model is interim and is brought to the L2
specification, never the reverse.
Mikhail on the map's question 5.4 (2026-09-15, verbatim): "какая копия? У нас
нет никаких копий message -- выше же с тобой обсуждали. Есть только он сам
и у него своя арена. Пускай это будет один alloc но это его собственный. При
получении Message который не L3 Thread присоединяется к арене получателя.
Всё" (what copy? there are no copies of a message, we discussed this
above; there is only the message itself, and it has its own arena, be it
one alloc, its own; on receipt a Message that is not an L3 Thread attaches
to the recipient's arena; that is all). Entered verbatim in spec 19.29.7
and the model's section 27 the same hour. Consequence for the design (the
lead's, not the spec's): the mailbox is a synchronized collection of
Messages, each with its own arena; LmxMsgCopy, copy_bytes and copy_dup are
today's artefact and go; A3 no longer waits. b5's next ticket: the
LmxMsgCopy census in lmx_message.lm1 (fields with writers and readers,
envelope kinds, byte copies, chain walkers) as the data for that
replacement.
S0 landed on 0c's side: claude-0c/lock-inventory 95fb6a62, the lane tripwire
as ruled (the attaching thread recorded; a no-turn write from any other
thread aborts; take_this admits the write only when held_by is the calling
thread, 19.28.R2.2 (3)); measured cold with run_port_message -LaneCheck: A
green on the committed bytes, P listing the admitted held take and the
bootstrap thread's five writes outside a turn (bind:affinity,
run_child_turn:ready_clear, unbind:record, stop_unmap:ready_clear,
stop_reset:ready_clear), F red without the held_by comparison, C red on the
moved ready_clear that was green before; the lead cherry-picks it onto
claude-0c/lane-tripwire and lands it with run_port_message -LaneCheck,
run_lmx -Suite Exec (plain and LMX_LANE_CHECK=1) and scenario36, which
builds exec.c without LMX_MSG_EXEC_TEST, so no model test arms the check.
b5's allocation inventory of lmx_message.lm1: sonnet/lock-inventory
e34468b3, 16 allocation and 34 free sites against the grep, lm2 equal; 26
M, 7 P, 3 X, 12 S, 2 provenance-dependent; release_slot's frees of a
released child's init and path are the parent's settle write (P) on the
disposing lane; done_grow's arrays are P but freed on admit_one's lane, a
cross-lane free like done_add's, a design item.
The lead's spine corrected (b57db1ad): exec.c has 57 lookup call sites in 39
functions (line 1114 is a comment naming msg_at_addr in endp_try_retire, which
makes no lookup), 96 in total with lm1's 39. S1 (adopt_push deleted) is
measuring on d6/lock-s1 off c067bed9; its first run was void for the
environment (a fresh worktree without the ignored pinned l1trans, every
runner stopping at "missing L1 translator"), rerun with the pinned exe
whose SHA-256 matches L1_PIN.txt.
S0's landing branch: d6/lane-tripwire 7c8336f9 (57's 95fb6a62 cherry-picked
with -x onto c067bed9, exec.c only, +64/-2); the landing script merges
--no-ff into integration, runs cold run_port_message plain and -LaneCheck,
run_lmx -Suite Exec plain and with LMX_LANE_CHECK=1, run_model_scenario36
and run_gates N of N, counts any "LANE WRITE FAIL" in a log as red, and
pushes only if all are green and origin is still c067bed9; it starts when
S1's measuring run finishes so the executor tests' timing windows do not
share the CPU. S1 so far green on send_local (146/0, owned_frees 1) and
family_handoff (63/0, watched_frees 4), the rest running; S1 commits onto
integration after S0 lands, its own merge. Design updated at 15567b9f with
Mikhail's 5.4 sentence verbatim, A3 no longer waiting, LmxMsgCopy at
admission marked as today's artefact.
Correction (0c, 2026-09-15): run_port_message -LaneCheck is already the
first of run_gates' 11 defaults (run_gates.ps1 34-35, "the decision 18
lane-write oracle is part of the gate"), so the proof for S2 on already
runs in every default chain once S0 lands; the coordinator's "a proof that
runs in no default gate" was wrong. Ruled: 0c hardens the row instead
(renamed lane_oracle, its verdict requiring the parity pass line and 0
"LANE WRITE FAIL" in the log, count and total unchanged, the RUNNER_HAZARDS
row for the no-turn gap in the same commit), on the integration hash with
S0 in; 0c's 32-name default-set commit merges over it afterwards.
Mikhail on the map's question 5.5 (2026-09-15, verbatim, to "does R0's
scheduler step its threadless children to their settle before R0's thread
frees what is R0's, or leave them ready with no scheduler?"): "мы же
обсудили миллион раз. Закрытие родителя это авария. _Каждый ребенок
закрывает себя сам и для его детей это такая же авария_. Если дети L3
Thread но реализация такая что у них нет своего потока, а они
последовательно выполняются, то это нюансы реализации. Разумеется в этом
случае закрытие родителя затягивается так как планировщик должен пройтись
и выполнить чужую работу -- но зачем вам вообще сейчас такие потоки если вы
не можете даже ядро архитектуры запилить точно по спеке?" (we discussed
this a million times; closing a parent is an emergency; each child closes
itself, and for its children it is the same emergency; if the children are
L3 Threads but the implementation gives them no thread of their own and
runs them sequentially, that is an implementation nuance; of course the
parent's close then drags on, since the scheduler has to go round and do
others' work; but why do you need such threads at all now, when you cannot
even build the core architecture exactly by the spec?). The coordinator's
reading, put to him for a one-word confirmation before any doc carries it:
for now every L3 Thread runs on its own OS thread, and the sequential
mapping (children without a thread stepped by the parent's thread:
run_child_turn, sched_step, the host lane) is removed from the runtime
until the core is exact by the spec.
Found by 0c (2026-09-15): RUNNER_HAZARDS.txt, cited by section 6 as the gate
policy's source, was never landed (absent at c067bed9, on integration and
on main; it exists only on claude-0c/runner-hazards c6b5ef27). Ruled: the
lane_oracle hardening commit lands the file as it stands plus the no-turn
row, allowlist run_gates.ps1 and that file, on the lead's integration hash
with S0 in (cf326822 is 57's S0 commit alone on claude-0c/lane-tripwire, not
a merge with S1; corrected by 57); the verdict requires the parity pass line and 0 "LANE WRITE
FAIL" in the gate log (the tripwire's abort line reaches the console log,
two lines in runs C and F); then S3's red test (one wake per lane,
test-only, 0c's, stacked on cf326822); step 2's default-set commit merges
over lane_oracle afterwards.
Mikhail confirmed the coordinator's reading of 5.5 (2026-09-15, "ДА"): for
now every L3 Thread runs on its own OS thread, and the sequential mapping
(children without a thread of their own, stepped by the parent's thread:
run_child_turn, sched_step, the host lane, the UI lane's stepping) is
removed from the runtime until the core is exact by the spec. Entered in
spec 19.28.R2.2 (after his 2026-09-15 clarification) and the model's
section 29 with his sentence verbatim. For the lock-removal design this is
a stage of its own, larger than S4: the host-run mapping and its tests
(stage 5 (d)'s turn_step_child migrations, the sequential scheduler
record's cursor, the UI lane as a stepped lane) are X; a child is launched
on its own thread or not at all; the lead re-plans S4-S6 on it.
Mikhail, 2026-09-15, verbatim, on the last open items (the coordinator's
2026-09-14 handoff-identity sentence in 19.29.6 and the model's section 32,
and lm2/own.lm2's unused mutex and condition wrappers): "Да всё удаляйте
потому что это совершенно очевидно лишнее. Одна арена на Message, арены
присоединяются при потреблении -- это ядро" (yes, delete all of it, it is
obviously superfluous; one arena per Message, arenas attach on consumption,
that is the core). Done the same hour: the handoff sentence removed from
both copies; his core sentence entered verbatim in spec 19.29.2 and the
model's section 25. His "delete" on own.lm2's wrappers answered a false
premise (the lead's correction the same hour): the wrappers are not dead,
lm2/own.lm2 carries a lock-based message-thread runtime of its own (route
and pool mutexes, condition variables for work and stop, 222 uses in
own.lm2, 41 in own_abi.lm2, C tests under lm2/tests, no runner building
them by grep); it is out of S1, needs its own inventory, and the question
with the real extent (goes whole, replaced by the one core, or converted;
the coordinator proposing whole, 19.28.R2.2 11582-11587) is put to him.
b5's LmxMsgCopy census (sonnet/lock-inventory db042997, section 3 of the
lm1 inventory): 39 LmxMsgCopy declaration and cast sites in lm1, 39 in lm2;
all 14 fields with every writer and reader by function and line; all 12
LMX_MSG_KIND_* constants, five of them (ITEM, CANCELLED, DONE, REJECTED,
MAP) created and consumed nowhere in lmx_message.lm1; 8 byte-copy sites
(copy_bytes 7, copy_dup 1), five into an LmxMsgCopy's bytes (host_post,
send, send_owned, send_cap), three into bare buffers (create_prepare's
init, init_copy's read-back twice), each a byte-for-byte duplicate where
one arena would move or attach storage; six functions walk a mailbox chain
by next (q_push, q_pop, copy_free, host_drain, end_turn's outbox transfer,
fail's inbox redirect); flagged for the design: KIND_DEAD is created but
never consumed by a kind check in this file, and KIND_GRAPH is detected by
node.delivered rather than its kind tag, so a replacement keyed on kind
must check what reads them above this file.
b5's next ticket (2026-09-15, in the order's scope, its own domain): the
vendored host ingress V0 in mixa_manager/vendor/lmx_msg_host_ingress_v0
carries the host lock again (3 host_lock calls, 2 EnterCriticalSection, 3
pthread_mutex_lock, 6 pthread_cond_ in the census) and cannot stay under "no
locks in the project at all except the message collection's own"; b5
inventories it as mixa_manager/HOST_INGRESS_V0_INVENTORY.txt on
sonnet/mixa-ingress-inventory off c067bed9 (every mixa_manager caller by
file and line, the harnesses that build it, what the runtime's own
admission into R0's mailbox gives each caller instead or "silent", the
copy's lock count against the census); the deletion is its own commit
after the lead's stage on the host forms names it.
Mikhail on the map's question 5.6 (2026-09-15, to the proposal "a capability
is the target's mailbox handle; a closing Message's mailbox is settled into
its parent with the rest of its storage; a late send lands in the parent's
mailbox and is refused on its lane with a status to the sender; no holder
count"): "давай так" (let us do it that way). Entered as agreed wording in
spec 19.29.7 and the model's section 27 the same hour; S4's C3/C4 unblocked.
On the coordinator's 2026-09-14 handoff sentence, verbatim: "зачем вообще
этот create id сейчас нужен? Снимайте все что не нужно" (why is this create
id needed at all now? remove everything that is not needed): the lead greps
the spec for create id; required nowhere, the field and its bookkeeping go
in the dead-code stage and the model's line about it is corrected to his
words; required somewhere, the section goes to him.
create id (the lead, 2026-09-15): the spec requires it. 19.29.7.1 "First
copy-only participant slice" (main dab1513a, 12872-12895): create/spawn(handler,
initialData, create_id) -> MessageAddress; a retry of the same (parent,
create_id) returns the same child and path and consumes no counter value;
an aborted reservation is not reused, a later retry gets a new slot and path
segment; while the child is live the retry returns it (no second committed
child); an aborted uncommitted reservation is released as a tombstone and a
retry must not return the freed handle. So create_id is the idempotence key
of a create retried within a live parent; the model repeats it at 309-310,
1260 and 1961; in code lmx_message.h 7 sites, lm1 and lm2 18 each, tests 5;
the UI lane's reserved LMX_MSG_UI_LANE_ID goes regardless once the UI lane
is an L3 Thread on its own thread. Question put to Mikhail directly: does
create-retry idempotence stay (and create_id with it), or go with the whole
retry clause, in which case those spec sentences go by his word and
create_id with its bookkeeping joins the dead-code stage. Nothing removed
until his answer.
Mikhail's last three answers (2026-09-15, verbatim): "1 - -да, 2 -- убирайте
3- да, удаляйте целиком" (1 yes; 2 remove; 3 yes, delete it whole). 1: the
5.1 wording confirmed and entered in spec 19.28.R2.2 and the model's
section 29 (no root list; no walk at deletion; the slot list and the retire
queue go without replacement; a lane's own queue is that Message's
mailbox). 2: create id goes: the create-retry idempotence keyed by (parent,
create_id) and its tombstone rule removed from 19.29.7.1 (the signature is
create/spawn(handler, initialData) -> MessageAddress) and from the model's
three mentions, each place carrying a one-line note of the removal by his
order; in code create_id, the retry lookup and their bookkeeping (header 7,
lm1 and lm2 18 each, tests 5) join the design's dead-code stage,
LMX_MSG_UI_LANE_ID going with the UI lane's change to an L3 Thread. 3:
lm2/own.lm2's lock-based message-thread runtime with own_abi.lm2 and its C
tests is deleted whole, replaced by the one core, its own stage in the
design with a grep at 0 for lm_mutex_, lm_condition_ and
lm_native_mutex_/condition_ as its proof. Every question of the map is now
answered.
b5's inventory of the vendored host ingress V0 (sonnet/mixa-ingress-inventory
589c8aae, mixa_manager/HOST_INGRESS_V0_INVENTORY.txt): 15 call sites across 11
of the copy's 28 exported names, all in tests/mixa_ingress_host_harness.c,
the other 17 names uncalled; the only builder run_ingress_harness.ps1 (the
share-native and win32 smoke harnesses do not reference it; their events
and Interlocked are harness-internal); 10 of the 11 called names have a
same-named live equivalent in lmx_message.lm1 (host_post admits into R0's
inbox under the mail lock, the copy's host-lock queue gone), host_wait
silent (lmx_message_host.c, the host forms' stage); the lock counts equal
to the census. Ruled 2026-09-15 under "снимайте все что не нужно": b5
deletes the vendored copy, its harness and its runner whole as a second
commit on the same branch, red-first by grep counts before and after (0
across mixa_manager, SPEC_DIGEST's mention of the live API the one remaining
match), mixa_manager's own gate cold and green; lands through the lead's
chain.
The lead's design at 5249bd82 (d6/lock-removal) carries the three stages
from Mikhail's answers: M, the sequential mapping removed (after S3, before
S4): run_child_turn and child_turn_core's step path, sched_step and the
scheduler record's cursor, exec_bind_mode's host mapping, the UI lane's
stepping (ui_step, take_ui, take_addr, the MAP request, AFFINITY_UI as a
bind affinity) go; R0's own turn on the bootstrap thread and map_child (a
child launched on its own thread) stay; the fourteen tests migrated onto
turn_step_child in stage 5 (d) are decided per test in M's commit by one
rule (a property of the mapping itself is deleted, a property of the child
moves onto its own thread); red a test-build counter of step entries,
green the grep at 0 with every converted test on its own thread and the
lane oracle. D, create_id and the create-retry idempotence deleted with
S1's dead-code kind (find_create, the tombstone rule, header 7, lm1 and
lm2 18 each, tests 5; UI_LANE_ID with M), proof the grep at 0 and every
runner building lmx_message green, each deleted check named. O, the
lock-based message-thread runtime in lm2/own.lm2 deleted whole with
own_abi.lm2 and its C tests, its inventory of builders and users the
stage's first line. Open in M for Mikhail, put to him by the coordinator:
the UI lane as an L3 Thread whose lane is the UI thread, taking its own
Messages from its own mailbox, work for the UI thread being a Message sent
to that lane (19.28.R2.2 11503-11506), no other Message's turn on the UI
thread. The lead's order: S1's measure (24 of 28 runners green, gates
last); S0 cf326822 lands; a quiet window for 57's lane_oracle and S3 red;
S1 merges; S2's red measured; b5's mixa deletion; then M, D, O per the
design; b5 holds the stage O inventory ticket after its current work.
Standing rule (Mikhail, 2026-09-15, verbatim): "в случае возникновения
каких-то багов или противоречий -- формулируйте, останавливайтесь и ждите
моего ответа. Синхронизация только локальная в почтовом ящике каждого L3
Thread -- больше в проекте нет (а исключением поддержки ресивера
\"synchronized\")" (in case of any bugs or contradictions, formulate, stop
and wait for my answer; synchronization is only local, in the mailbox of
each L3 Thread; there is none else in the project, except the support of
the receiver "synchronized"). Relayed to the lead, 0c and b5 the same
hour: on any bug or contradiction met in a stage, the stage stops with the
contradiction formulated in one paragraph and waits for his word through
the coordinator; the synchronization sentence entered in spec 19.28.R2.2
and the model's section 29.
Refined by Mikhail the same hour, verbatim: "\"при любом баге\" -- при любом
баге фикс которого потребовал бы введения других локов" ("at any bug" means
any bug whose fix would require introducing other locks). So the stop rule
covers contradictions and the bugs whose fix would need a lock outside a
Message's mailbox; other bugs are fixed in the ordinary way, red-first.
Relayed to the lead, 0c and b5.
b5's deletion of the vendored host ingress V0: sonnet/mixa-ingress-inventory
1a9f1ca8 on 589c8aae; before, the vendor path 6 files, the harness 15 call
sites, run_mixa.ps1 referencing run_ingress_harness.ps1 at line 183; deleted
the vendor directory, tests/mixa_ingress_host_harness.c,
run_ingress_harness.ps1 and the runner's invocation block in run_mixa.ps1
(lines 182-185), nothing else; after, 0 files, 0 call sites, 0 runner
references; run_mixa.ps1 cold exit 0 through "mixa backend table ok" with
no ingress line; the string lmx_msg_host_ingress_v0 remains in 5
documentation lines (the inventory file, 4; mixa_console_window.txt, 1),
reported as 5 rather than claimed 0. Ruled: the inventory file goes in a
third commit (the commit history and this plan hold the record), the
design note's mention stays and is named as the one match; lands through
the lead's chain after the S-stages in flight.
b5's branch sonnet/mixa-ingress-inventory is complete at 9b085558 (589c8aae
the inventory, 1a9f1ca8 the deletion, 9b085558 the inventory file removed;
the string lmx_msg_host_ingress_v0 remains once, mixa_console_window.txt:202,
a design note, named in the commit); it lands through the lead after the
S-stages in flight, allowlist mixa_manager. b5 is on the lead's own.lm2
inventory (stage O's first line).
S1 green: d6/lock-s1 aef95e1c (adopt_push deleted from exec.c, off c067bed9),
measured cold with the pinned l1trans: all 28 runners that compile exec.c at
their pass lines (send_local 146/0, family_handoff 63/0, port_message parity
PASS with 100 methods redirected, scenario36, entry_turn 24/0, sched_record
22/0, graph_abi, lmx and l2trans gen2 ok, the 18 port parity runners),
run_gates 11 of 11 in 210 s, no build log naming adopt_push. S0 is landing
(57's cf326822 merged into integration, the five runners cold); S1 lands
on S0's merge after 57's quiet window.
A contradiction raised by 0c (2026-09-15) and withdrawn by 0c the same hour:
the workers' wait events, stop_ev, the condition-variable signal and the
order-free Interlocked cells against "synchronization only in each L3
Thread's mailbox"; Mikhail's recorded words already cover them (wait
signals need no lock; atomic state whose order does not matter); what
remains reportable is each lock outside the mailbox, all already "to go":
the host selftest's g_go_mu, the pthread worker's pthread_cond_wait bound to
the exec lock (goes with it), the launch pack gate if it is a mutex. 0c's
inventory section 5 is rewritten against those quotes; the S3 red test
proceeds.
Found by 0c (2026-09-15): the launch pack gate is a mutex on POSIX
(LmxMsgCtxPack's pthread_mutex_t gm with a condition variable around the go
flag, exec.c 2789-2832; a one-shot event on Win32), a lock outside any
mailbox that the map's 3.5 had not named. No question open: under the
order it goes, replaced by the launched Message's own mailbox (the launch
parameters as the child's first Message, the thread waiting on its own
mailbox's wake); the map's 3.5 now names it, with the host lock
LmxMsgHostSync.lock as host_sync's X. 0c's inventory section 5 is rewritten
against Mikhail's two recorded sentences with the two production locks
added.
0c's inventory rewritten at claude-0c/lock-inventory aca90d74 (section 5c: the
"not locks" kind gone, each row citing one of Mikhail's two admitting
sentences or being a thread with no synchronization of its own; section 5d:
every production lock in l2src on cf326822 by grep, the executor lock with
the pthread worker's cond wait on it, the pack gate mutex, the host lock,
the mail lock allowed; own.lm2's 132 mutex lines and own_abi.lm2's 10
counted for stage O, not examined). Falsifier held: the file has no "not a
lock" row.
Mikhail on the UI lane (2026-09-15, verbatim, to the coordinator's (a)/(b)
question; on today's UI lane without a thread, stepped by R0's turn: "это
просто кривая реализация"): "Полоса на экране не может не быть L3 Thread. Message без L3 Thread это просто письмо. Полоса же это полноценный объект. Объектов у нас не так много как в SmallTalk но все равно много. Поэтому реализация L3 Thread планировщика который не запускает потоки детей а бегает по ним сам -- ближайшая задача для myxa_manager" (a lane on the screen cannot
fail to be an L3 Thread; a Message without an L3 Thread is just a letter; a
lane is a full object; we do not have as many objects as Smalltalk, but
still many; so the implementation of an L3 Thread scheduler that does not
launch its children's threads but runs over them itself is the nearest task
for mixa_manager). Read back to him and entered verbatim in spec
19.28.R2.2 and the model's sections 25 (iv) and 29: (1) the UI lane is a
full L3 Thread with its own thread (the UI thread) and its own mailbox;
today's threadless lane stepped by R0 goes (stage M); (2) a scheduler that
launches no threads for its children and runs over its attached Messages
itself is not a core mechanism but a variety of L3 Thread, and its
implementation is mixa_manager's nearest task, in L2, with the mailbox as
its only synchronization. Tickets: the lead's stage M carries (1); b5's
next ticket after the own.lm2 inventory is (2), design first.
S0 landed on integration as e88dab64 (57's lane tripwire cf326822 merged into
c067bed9): run_port_message plain and -LaneCheck parity PASS with 100
methods redirected, run_lmx -Suite Exec ok plain and with LMX_LANE_CHECK=1,
scenario36 core tests PASS (scenario36 49/0), run_gates 11 of 11 in 204 s,
no LANE WRITE FAIL in any log. 57's quiet window off e88dab64 for
lane_oracle and S3 red; then S1 aef95e1c lands on e88dab64, then S2's red is
measured. 0c's inventory header fixed at a21fa962 (the column's five values;
the "not a lock" grep prints nothing; the withdrawal record at L160 kept).
Stage O STOPPED under the stop rule (the lead, 2026-09-15), a contradiction
put to Mikhail by the coordinator: side 1, his "3- да, удаляйте целиком";
side 2, b5's own.lm2 inventory (sonnet/lock-inventory 19a95a26,
LOCK_REMOVAL_OWN_INVENTORY.txt): the L2-to-L1 translator's code generator
(lm2/trans_l1_statement.lm2) emits lm_message_thread_new/_begin_turn/
_end_turn and lm_own_arena_new_zero(thread, ...) into every translated
program's main and every generated heap allocation (123 matches of emitted
text), parser.lm2 keeps its P0 registry as a component of that thread, the
REST server and client use its providers with no substitute, 13 native C
tests and 33 translator fixtures build it, a CMake target refreshes it; the
L1 baseline has none of it, and the one core (lmx_message in l2src) has no
lm2 entry point today; so the arena-per-thread allocation of the lm2 chain
rides on the same records as the locks, and whole deletion breaks every
program the lm2 translator produces. Options put to him: (a) O waits until
the core serves the lm2 translator's emission and the emitted text switches
to it in the deletion's stage; (b) the translator stops emitting the
boilerplate now and its programs build without an arena until the core is
there, the REST providers and the registry component going with it; (c)
only the lock-bearing parts go now (the router and the pool with
route_mutex, the pool mutex, work_ready, state_mutex, stopped_condition),
the thread with its arena and mailbox staying as the lm2 chain's L3 Thread
until the core is available to it and then replaced, O completing then;
the coordinator proposing (c). S1-S3, M and D continue; nothing deleted.
Mikhail, 2026-09-15, verbatim, the frame for every stage: "Вы пишите на Си
на должны мыслить в категориях event-driven. В свете нашей \"почтовой\"
реализации, без семафоров SmallTakj" and "Точней вы пишите не на Си, на Си
пишет транслятор" (you must think in event-driven categories, in the light
of our mail implementation, without Smalltalk semaphores; more precisely
you do not write in C, the translator writes C). Relayed to the lead, 0c
and b5: an event is a Message in a mailbox and the reaction is the
recipient's turn; no waits on primitives, no pools with condition
variables, no semaphore-shaped design; the runtime's design and code are
L2 and L1, the C is the translator's emission and never the design
surface.
Second contradiction under the stop rule (the lead, 2026-09-15), put to
Mikhail by the coordinator: side 1, his "вы пишите не на Си, на Си пишет
транслятор" and "мыслить в категориях event-driven ... без семафоров";
side 2, the executor the lock-removal stages change is hand-written C,
stg/l1_baseline/l2src/lmx_message_exec.c, about 4,100 lines with the exec
lock, the context workers, wait events and conditions, launch_ctx_thread
and run_one; only lmx_message.lm1 (with its lm2 mirror) is translated; the
stages as designed edit that C directly (S1's deletion, S2's counter, S3's
hook, S4-S6's, M's and D's rewrites of the workers and records);
RUNTIME_L2_PORTS.txt ports parts of the runtime to L2 but not the executor
core. Options put to him: (a) the stages keep changing exec.c as the interim
implementation and the executor's port to L2/L1 is a later stage; (b) the
executor is first rewritten in L2/L1 as a translated unit (a thread, a
mailbox with its own wake, launch and close of children, one arena per
Message), event-driven without semaphores, and the lock removal is the
replacement of exec.c by that unit; (c) deletions in exec.c (S1, D, the M
and O removals) proceed while anything that adds or reshapes executor code
is written in L2/L1; the coordinator proposing (b) with (c)'s deletions.
Held until his word: S2's measure, S1's landing, 57's S3 red commit; stage
M's test reading and the design text continue.
S3's red reshaped (the coordinator, 2026-09-15, test shape, not spec): an
empty wake admits nothing and runs no turn, so no mailbox or turn
observable can go red and the empty-wake counter hook is the exec.c-internal
expectation the frame excludes; S3 carries the design change (the wake
addressed to the recipient's mailbox) and its proof is an oracle rule beside
the lane oracle: every wake of a lane must be issued by an admission into
that lane's mailbox, a wake with no admission is "LANE WAKE FAIL", red today
on run_one's wake-all and green after S3; the counters and the selftest's
Interlocked cells go, uncommitted. The hook is executor code and is held
with the exec.c contradiction before Mikhail.
Mikhail on stage O (2026-09-15, verbatim, choosing option (b)): "да, все имиации из кода немедленно убираем. Это не L2 код если он так себя ведет. Переводите его в L1 и помечайте в комментариях что механизм MEssage временно не используется. На самом деле пользовательский L2 может синхронизировать что хочет _но мы пишем в своей филососфии Message чтобы потом переводить на L3_" (yes, all the imitations are removed from the code at once; it is not L2 code if it behaves like that; translate it into L1 and mark in comments that the Message mechanism is temporarily not used; in fact user L2 may synchronize whatever it wants, but we write in our Message philosophy in order to translate to L3 later). So stage O is: the lm2 translator's code generator stops emitting the message-thread boilerplate (lm_message_thread_new/_begin_turn/_end_turn, lm_own_arena_new_zero(thread, ...)) now, its programs build without an arena context until the core is there; own.lm2's lock-based message-thread runtime, being no L2 code, is moved to L1 with the comment that the Message mechanism is temporarily unused, its locks gone with it; the REST providers and parser.lm2's registry component go with it; the 13 native C tests and 33 fixtures follow the emission. Stage O is unblocked and is the lead's to design in the (a) format (owner of the lm2 chain named in it); the exec.c question is still before him.
0c's lane_oracle: claude-0c/lane-oracle 38a91c86 on e88dab64; run_gates.ps1's
row renamed lane_oracle with a verdict red on exit 0 without the parity
line or with any LANE WRITE FAIL in the raw log, and RUNNER_HAZARDS.txt
landing (c6b5ef27's survey plus row (k), the no-turn gap closed by the
tripwire); cold 11 of 11 in 214 s, lane_oracle 55 s, 0 LANE WRITE FAIL;
falsifiers each "gates RED: stopped at lane_oracle" (mutation C naming
run_child_turn:ready_clear(moved); a stub with the parity line and one
FAIL line at exit 0; a stub with no parity line). No executor C, so not
held by the exec.c question; lands through the lead on e88dab64.
b5's UI-lane scheduler design (sonnet/ui-lane-scheduler-design 20178170 off
e88dab64, mixa_manager/UI_LANE_SCHEDULER_DESIGN.txt, no code): measured
first (mixa_manager has no AFFINITY_UI, UI_LANE, lmx_msg_ or
lm_message_thread_ reference; BACKEND_SEAM.txt section 7 already names the
architecture and leaves 7.2, how the loop idles, open; mixa_pump.lm1's
drain is a hand-rolled stand-in for a mailbox; the win32 backend's poll is
non-blocking for 7.2's sake; no central UI loop exists yet); the design:
the UI lane a full L3 Thread on the UI thread launched by R0, its own FIFO
mailbox the only cross-lane synchronization, the scheduler an L2 Structure
in the lane's arena holding the attached-Message list, one attached
Message's turn at a time in mailbox order, nothing pool-, mutex- or
condition-variable-shaped; the acceptance red-first (two attached
Messages, inputs admitted out of attachment order, turns in admission
order, one per call to the lane's turn-taking function, no OS thread or
wait primitive in the test). Reviewed 2026-09-15; its three questions
answered from main, not new rulings: Q3, "attached" is Mikhail's word
(19.28.R2.2 11481-11491: a consumer schedules the Messages it attached) and
attachment is his core sentence (arenas attach on consumption; on receipt
a non-L3 Message attaches to the recipient's arena), so the relation is
the lane's arena content, neither parent/child launch nor new; Q1, a
thread waits only for its own mailbox, so on the UI thread the lane's wait
is the platform's own thread wait and the mailbox's wake rides that queue,
no second handle; Q2 follows, admission wakes through the platform queue,
no separate primitive; both implementation within his words, not spec.
Next for b5: fold the answers, the acceptance red-first on e88dab64, then
the L2 code in mixa_manager's gate, merging through the lead.
Stage O designed on Mikhail's (b) at d6/lock-removal c6293c68 (his sentence
verbatim; facts from b5's 19a95a26): O1, trans_l1_statement.lm2 stops
emitting the message-thread boilerplate, red first on the fixtures' pinned
texts, green with the emission grep at 0 and the texts updated in the same
commit; O2, what remains of own.lm2's runtime moves to L1 with the comment
"the Message mechanism is temporarily not used here", its locks and
condition variables gone, a part that cannot run without a lock stopping
the stage as a lock-needing bug; O3, the deletions (the REST admit and
provider path, parser.lm2's registry component, the 13 C tests and the
module-only fixtures, the buildCore steps, CMake's refresh_own_lm1), each
test named; proof the grep at 0 for lm_mutex_, lm_condition_,
lm_native_mutex_, lm_native_condition_ and lm_message_thread_ and the lm2
chain green. Approved 2026-09-15; owner b5 (sonnet/stage-o off e88dab64,
one commit per step), landing the lead's with the allowlist lm2/,
stg/l1_baseline/lm2/, tests/ and CMakeLists.txt, nothing in l2src; order
for b5: O1-O3 before the UI-lane scheduler's code. lane_oracle's first
landing run stopped on the lead's own allowlist check (sort's locale
ordering), fixed with LC_ALL=C and relaunched.
Mikhail on the exec.c question (2026-09-15, verbatim): "это разве
противоречие или баг который требует синхронизации? Планируйте как вам
удобно" (is that a contradiction or a bug that needs synchronization? plan
as you find convenient). Not a stop item; the coordinator's operating rule
for every stage, (c): deletions and lock removals go directly in exec.c,
which only shrinks; anything new that is executor policy or state (a
record, a list, a scheduler cell, a wake owner) is written as an L2
runtime unit in its Message's arena on the lmx_sched_record pattern and
called through c., joining Build-L2RuntimeUnits and a parity runner as 3c
did; what stays in C is the native primitives the Target names (threads,
the mailbox's wake, TLS, the setjmp turn root); test-build hooks and
oracles (the tripwire, S3's wake rule) may live in exec.c under
LMX_MSG_EXEC_TEST as proof tools, not design. The holds are lifted: S1
lands on the lane_oracle merge, S2's red is measured, S3 proceeds as the
wake oracle rule.
lane_oracle landed on integration as d66a4ec1 (57's 38a91c86 merged into
e88dab64; the allowlist held exactly run_gates.ps1 and RUNNER_HAZARDS.txt):
run_gates 11 of 11 in 203 s with the row "lane_oracle PASS 54s". S1
(aef95e1c) landing on d66a4ec1 with run_port_message -LaneCheck,
send_local, family_handoff, scenario36 and run_gates; then S2's red
measure; then 57's window for the 32-gate default set and S3 on the S1
merge hash.
Stage O's premise corrected by Mikhail (2026-09-15, verbatim, on "its
programs build without an arena until the core is there"): "самосборка
подразумевает что код собирает себя сам, у нас нет стадии \"до появления
ядра\". Эта стадия появляется только при миграции на другую платформу и для
этого в gitignore есть несколько исключений для сгенерированных Си ядра,
иначе у нас всегда есть предыдущий бинарник" (self-hosting means the code
builds itself; there is no stage "before the core appears"; that stage
exists only when migrating to another platform, for which the gitignore
keeps exceptions for the core's generated C; otherwise there is always the
previous binary). So O as designed stops (b5 held, nothing committed) and
the lead rewrites it: O1, the lm2 translator's emission switches from
own.lm2's imitation to the core, the shape l2trans already emits (the entry
as R0's turn, allocations from the Message's arena, the core through c.),
the self-build through the previous translator binary with the core's
generated C as the gitignore's exceptions, the chain measured to build
itself on the new emission; O2/O3, the imitation deleted whole as he first
said; the first measured fact of the rewrite is what the lm2 chain's
emitted program needs from the core that l2trans's emission does not give.
Mikhail (2026-09-15): "C:\Nyasha_Planet\lingvamyxa_old_worked_version -- вот
этот проект собирает себя сам из своих исходников" (this project builds
itself from its own sources). Read there as the reference for stage O's
bootstrap order: .gitignore keeps the exceptions for the generated C of the
core and the translator (lm1/build/{buildCore,finalize,make,own,parser,
rest_lmx_http_client,rest_lmx_http_server,trans}.lm1.c, committed);
buildCore.lm0.sh compiles exactly those files into build/lm0 (libparser and
libown archives, trans, make, finalize, buildCore: "built build/lm0
bootstrap tools"), the previous binary; the lm0 tools then translate the
lm2 sources into new C that regenerates the same committed files, and a
self-build is green when the chain built from the regenerated C reproduces
it (the fixed point). The current repo keeps the script's shape with
l1trans.lm1.c in place of parser/own/trans (exceptions adding
l1trans.lm1.c, printTree.lm1.c and l1src/p0.lm1.h); the lm2 chain is
translated by CMakeLists.txt and stg/l1_baseline/gate.ps1. So O1's red is
the committed generated C carrying the boilerplate and its green is the
emission changed, the chain translated by the lm0 tools from the old C, the
regenerated C committed as the exceptions and a second self-build
reproducing it byte for byte; O2/O3 the deletions whole. Given to the lead
for the rewrite; b5 holds O and writes the scheduler's acceptance.
Mikhail (2026-09-15, verbatim, on the old working version): "работа над ним
требовала разделения на шаги с полной самосборкой -- чтобы были
промежуточные бинарники" (work on it required splitting into steps with a
full self-build, so that there were intermediate binaries). Rule for stage
O and for every change of the translator's emission: the work is split into
steps, each ending with a full self-build (the lm0 tools from the committed
generated C, the chain translated, the C regenerated and committed as the
gitignore exceptions, a second self-build reproducing it), so that an
intermediate binary exists after each step and the previous binary is never
lost; no step may leave the chain unable to build itself. Given to the
lead for O's rewrite.
Mikhail (2026-09-15, verbatim): "сгенерированные Си мы не правили никогда"
(we never edited the generated C). Rule: the committed generated C
(lm1/build/*.lm1.c and the other gitignore exceptions) is changed only by
regenerating it with the translator, never by hand; a diff of those files
in a commit is refutable by regenerating them from the same sources with
the same previous binary and comparing byte for byte. Relayed to the lead,
0c and b5.
b5's UI-lane scheduler acceptance: sonnet/ui-lane-scheduler-design 3d341969
(the design's three answers folded in 06aa3baa), mixa_manager/tests/
mixa_ui_lane_selftest.lm1: two Messages attached (10 then 20), inputs
admitted in the opposite order, take_turn returning 20 then 10 then 0 (one
turn per call, not a drain), no OS thread, wait primitive or runtime
stepping, the five functions (mixa_ui_lane_new/_delete/_attach/_admit/
_take_turn) declared by prototype only; measured red on e88dab64 at link
(the five undefined symbols matching the call counts), not yet in
run_mixa's gate. Ticket: implement the scheduler now in L2 in mixa_manager
(an L2 Structure in the lane's arena: the attached list, the mailbox FIFO,
the take cursor; admission the only mailbox write; nothing that waits), red
to green, then the test joins run_mixa's gate; anything the lane's thread
needs from the runtime that does not exist stops at that line and comes to
the coordinator; stage O takes precedence when its rewrite lands.
Stage O rewritten at d6/lock-removal d16bf30a on Mikhail's four sentences
(delete whole; no stage before the core; steps with intermediate binaries;
generated C never edited), approved 2026-09-15. Facts: F1, l2trans already
emits the core's entry (runtime_new, create, run_entry_turn with the program
as R0's turn, l2trans.lm1 14722-14779) but allocates with lm_own_new_zero
on the heap, no lm_own_arena in l2trans, LmxMsg owning no LmOwnArena and
the core having no accessor for the turn Message's arena, so the one thing
the core must add is that arena; F2, the kept generated C (.gitignore
70-83): trans.lm1.c 947 lm_message_thread_ matches, 98 of them the
boilerplate by b5's count, the REST files 56 each, own/parser/l1trans/
buildCore 0; F3, the lm2 chain's previous binary is CMake's trans.lm0 built
from the kept trans.lm1.c with parser.lm1 and own.lm1 (CMakeLists 515-526,
refresh targets 528-560, runBuildCore.lm0 with LM0_TRUSTED_DIR), L1_PIN's
gen2 belonging to the stg/l1_baseline chain; F4, gate.ps1 runs
buildCore.lm0.bat for the L1 chain only, so no gate self-builds the lm2
chain on integration today. Stages and owners: O0 (b5, now, measurement
only) the baseline fixed point of the lm2 chain, becoming a gate row of
0c's; O1 (the lead, the core lane) the turn Message's arena in
lmx_message.lm1/lm2 with its accessor, red-first (two Messages' turns with
distinct arenas; the two-cycle balance), landed with the full gate before
O2; O2 (b5) three steps each a full self-build with the fixed point as its
green (O2.1 allocations onto the turn arena with the old scaffold still
creating the thread, O2.2 the main scaffold as R0's entry turn, O2.3 the
REST C regenerated), the lm_message_thread_ grep in lm1/build falling by
the sites switched; O3 (b5) the deletions whole, each test named, the grep
at 0 for the five prefixes and the chain self-building twice. The
regeneration check (same sources, same previous binary, byte for byte) is
the proof against a hand edit on every O commit. Allowlists: O0/O2/O3
lm2/, stg/l1_baseline/lm2/, lm1/build/, tests/, CMakeLists.txt; O1
lmx_message.lm1/lm2 and its test. b5's scheduler continues until O1 lands.
Mikhail (2026-09-15, verbatim), the single goal: "Теперь у нас единственная
цель -- убрать из проекта все локи за пределами почтовых коллекций и
добиться самосборки. Это пока единственная задача" (now we have a single
goal: remove from the project every lock outside the mail collections and
achieve self-build; for now this is the only task). Applied the same
hour: b5 moves wholly onto the self-build (O0 now, O2 and O3 after the
lead's O1), the UI-lane scheduler paused at its red acceptance
(3d341969) until stage M needs it; the lead on O1 and the lock-removal
stages; 0c on the oracles, the lane and wake gates and the self-build's
fixed-point gate; nothing else is worked on.
S1 landed on integration as d7f3b569 (aef95e1c merged into d66a4ec1;
adopt_push gone from exec.c): run_port_message -LaneCheck parity PASS in
54 s with no LANE WRITE FAIL, send_local 146/0, family_handoff 63/0,
scenario36 core tests PASS, run_gates 11 of 11 in 210 s. S2's red is
measuring on d6/lock-s2-red (reference.exe twice with LMX_LOOKUP_COUNT=1);
then 57's quiet window on d7f3b569; then O1 (the turn Message's arena, the
red test written against lmx_message.lm1/lm2 and own.lm1's LmOwnArena) in
the lead's lane.
0c's S3 wake oracle (written in wt0c_s3o on d66a4ec1, uncommitted and
unbuilt until the quiet window on d7f3b569): per lane, take-wakes may never
exceed the admissions counted into its mailbox, both counts kept beside the
mail lock, aborting "LANE WAKE FAIL site=... owner=... admits=... wakes=...";
retire wakes named as unchecked; a probe run lists every violating site with
the Message's input and closing flags, and a Message failing with input
still in its inbox would expose a miscount in the oracle itself. The
generated-C regeneration gate and the lm2 chain's fixed point are planned
rows awaiting O0's runner and base.
S2's red measured on d6/lock-s2-red b3d16381 (the counter in the test build
only; parity unchanged): reference.exe twice with LMX_LOOKUP_COUNT=1, both
"lmx_message_exec ok", "lookup walks: C1=1812 C2=2 turn_other=426
host=327729" and "... host=305125" (host varying with the selftest's
address-polling wait loops from main; turn_other stable at 426; without the
variable no report line). C1 and C2 stable and nonzero are the red. Since
stage M removes run_child_turn and sched_step, which account for part of
C1, S2's threading waits for M and its green is measured after M on the C1
sites that remain. The machine is 57's (quiet on d7f3b569); the lead reads
for O1's design with no builds.
O1 concrete at d6/lock-removal 133f9f76, approved 2026-09-15: the core
already has the Message's arena as 19.29.2 describes (LmxMsg.blocks, a list
of LmxMsgBlock {base, n, class, dispose, next}, owner-local with no sync,
and LmxMsg.ranges for the domains; the owned constructors allocate into
that pair; settle moves it into the parent; slot free disposes it);
own.lm1's LmOwnArena is a different allocator the core does not link and
O1 leaves it out. The addition: lmx_msg_turn_current (C, a read of the
thread's turn identity, T) and lmx_msg_turn_new_zero(size) /
lmx_msg_turn_copy_bytes(source, length) in lm1 and lm2 in one commit, each
allocating one block pushed onto the turn Message's own list, 0 outside a
turn, the Message's own lane the writer (M), freed by what already frees
the arena. Red first: a model test with A allocating on its context worker
and B on R0's turn, each allocation in its own Message's blocks only, 0
outside a turn, the two-cycle balance; falsifier, pushing onto the parent's
list. Branch d6/o1-turn-arena off d7f3b569, edits only while 57 has the
machine. O2's order corrected by the coordinator from "0 outside a turn":
O2.1 the main scaffold becomes R0's entry turn with allocations still on
the heap, O2.2 the allocations onto the turn arena, O2.3 the REST C, each a
full self-build.
Two rulings on O (2026-09-15): O2.1 keeps every step with an arena, the
lead's alternative ruled in (the entry moves to R0's turn and inside it the
old thread object is created only as the arena holder for
lm_own_arena_new_zero, no mailbox and no turn loop used; O2.2 switches the
allocations to lmx_msg_turn_new_zero / _copy_bytes and drops the object),
the heap variant out by "без арены Message не имеет смысла". O1 keeps the
two-cycle balance: run_model_scenario36.ps1 on integration has no --wrap
support (that hook is f79320de's on the unlanded fable/stage5f-acceptance),
so the runner hunk is cherry-picked into d6/o1-turn-arena as its own commit
ahead of O1's red test, which is the hook's first user on integration and
proves it by linking; O1's allowlist gains that runner file; the list
checks (adopted_base in its own Message's blocks only, adopted_n rising by
one per allocation, 0 outside a turn) stay beside the balance.
O0 measured (b5, sonnet/stage-o 5ac167af, LOCK_REMOVAL_O0_MEASUREMENT.txt,
nothing changed): trans.lm0 does not compile from the committed
lm1/build/trans.lm1.c: cmake --build --target trans.lm0 twice, exit 2 both
times, 2011 errors, byte-identical logs, about 38 s each; 443 distinct
symbols "redeclared as different kind of symbol" (trans.lm1.c declares each
once as a plain extern function and again as a closure typedef of the same
name, 21 of own.lm1's 24 fn/sub names among them, a pre-existing chain
bug), 504 "expected expression" in the main scaffold (68890-68987) where
today's own.lm1 has no lm_message_thread_*, the rest cascades; no link, so
no regeneration and no fixed point. So the lm2 chain's committed generated
C is not consistent with today's L1 sources and the chain has no previous
binary in this repository, while the live self-build (l1trans from its
committed C, l2trans in l2src, the l2src core) builds and is gated. Put to
Mikhail by the coordinator: which chain is the project's self-build: (a)
the lm2 chain, its previous binary restored from
lingvamyxa_old_worked_version and O2/O3 continued on it; (b) the live
chain, lm2/own.lm2 and the unbuildable lm2 self-host deleted whole as dead,
"achieve self-build" meaning the live chain brought to a full self-build
without locks; the coordinator proposing (b). O2/O3 wait; O1, the S-stages
and M continue.
Mikhail (2026-09-15, verbatim): "и кстати сделайте синхронизацию в
почтовых коллекциях в точности как это делается в Java, причем максимально
упрощенно (ресивер synchronized в спеке описан), в старом проекте он был
даже до какой-то степени реализован C:\Nyasha_Planet\lingvamyxa_old_worked_version"
(and by the way, make the synchronization in the mail collections exactly
as it is done in Java, maximally simplified; the synchronized receiver is
described in the spec; in the old project it was even implemented to some
degree). So the mailbox's one lock is a Java-style monitor in its simplest
form, expressed through the language's synchronized receiver (spec 20.5.7,
lowered to lm_synchronized_enter/leave), the old project's emission and
runtime hooks the reference; given to the lead as the rule for the mailbox
and its wake (S3) with the spec's lines and the old project's files.
Mikhail (2026-09-15, verbatim, on which chain is the self-build): "да,
вариант б. Сначала добиваемся самосборки а потом уже опять делаем порт в L2 и
далее в L3" (yes, option (b); first we achieve the self-build, and only then
we do the port to L2 again and further to L3). Applied: the project's
self-build is the live chain (l1trans from its committed generated C,
l2trans in l2src, the l2src core); lm2/own.lm2 and the unbuildable lm2
self-host are deleted whole as dead (stage O reduces to that deletion, with
its CMake targets, tests and fixtures, each named; O0's measurement is the
record of why); the L2 port of the runtime and the parser waits until the
self-build without locks is achieved. Order under the single goal: the
lock-removal stages (S2 on, M, D) in the live chain; the lm2 chain deleted
whole (b5); the live chain's self-build fixed point as a gate (0c); O1 (the
turn Message's arena in the core) stays as a core primitive but the
switch of l2trans's allocations onto it is a stage after the locks are out.
The coordinator's operating rule (c) adjusted to his order: until the
self-build is achieved, new executor policy or state is written in L1
(lmx_message.lm1 with its lm2 mirror as today), not as new L2 units; the C
keeps shrinking; test hooks under the test define as before.
The design updated at d6/lock-removal e7d96d6f with Mikhail's (b): stage O
the whole deletion of the lm2 self-host (b5, red by the greps, green on the
live chain's gates, O0's measurement the why); O1 a core primitive with
l2trans's allocation switch after the locks; the operating rule until the
self-build (new executor state in L1 with the lm2 mirror, C shrinking,
hooks under the test define); stage Y, the mailbox monitor per his Java
order: Y1 the synchronized receiver in l2trans (red: the old
trans_synchronized_cleanup as a run_l2trans leaf), Y2 the native monitor
hook (reentrant enter and leave, one mutex and one condition per monitor,
wait and notify), Y3 the mailbox using it (notify on admission, wait in
take, replacing m->mail and the bind waits; red: 0c's S3 oracle). Ruling
(the coordinator, 2026-09-15, implementation order, Mikhail may overrule):
until the L2 port, the L1 mailbox calls Y2's hook as an explicit
enter/leave/wait/notify sequence, which 19.28.R2.2 at 11623 allows for the
bootstrap ("temporarily L1/native bootstrap"), 20.5.7's "not a raw
lock/unlock pair in source syntax" being said of the L2 receiver; Y1 is the
first item of the L2 port after the self-build; conditions: the hook is the
monitor exactly as Java's so the receiver's later lowering changes nothing,
and the hook is called only by the mailbox's own functions, a grep for its
names elsewhere at 0 being Y3's proof beside the S3 oracle.
Stage D sized and split (the lead, on d7f3b569): 487 create-API occurrences
(the executor selftest 290, lmx_message_selftest.lm1 79, the model tests 2-7
each, l2trans's emitted entry and library open 2). D1 keeps the signature
and deletes lmx_msg_find_create (lm1 765-783, lm2 808-827), LmxMsg.create_id
with its store and its handoff/orphan clears, and the UI lane reservation
(with M), plus the checks of those properties, each named
(lmx_message_selftest's "idempotent create changed sibling/path" 566-573,
entry_turn's "the UI lane's create_id is refused" 88, the executor
selftest's handoff "create_id != 0U" 3685-3690); D2 drops the parameter
from the API and every caller, l2trans's emission, the generated units and
run_l2trans's cases with the historical pin. O1 ready to measure on
d6/o1-turn-arena (24c4ecd6 the wrap hook; 2ac67ba0 and fb1c7221 the red
test in the defaults with its header note; 69255593 the addition):
measure_o1.sh does the red in a scratch worktree at fb1c7221 (must fail to
link), green on the O1 test and the defaults, the falsifier (pushing onto
parent_msg's blocks prints FAIL), run_port_message plain and -LaneCheck;
land_o1.sh with an allowlist of the six files; both waiting for 57's
"done".
Stage O stopped again at the deletion list's scope (b5, 2026-09-15, nothing
touched): lm2/own_abi.lm2 (1126 lines) is predef'd by lm2/parser.lm2,
l1trans.lm2, printTree.lm2, registry_control.lm2 and table.lm2 besides
own.lm2, and declares both the message-thread types (LmHostThread, LmMutex,
LmCondition, LmMessageThreadComponent) and the general ownership and arena
ABI (LmSlice, the own stacks, LmOwnAllocationDescriptor, LmOwnLazyEdge,
LmOwnArena) those files need; table_descriptor.lm2 is shared the same way;
nothing in the tree builds any lm2/*.lm2 except the dead trans.lm0; the
live chain reads none of them. Put to Mikhail: delete the whole lm2
self-host (lm2/ and its stg/l1_baseline/lm2 mirror, history keeping the
reference for the L2 port "again"), or keep those L2 sources live and split
own_abi.lm2 and table_descriptor.lm2 instead; the coordinator proposing the
whole deletion. Every group held; trans.lm2's closure (trans_library,
table_descriptor, trans_schema, trans_l1_registry, trans_l1_namespace,
trans_l1_expr, trans_l1_statement, trans_l1_top_level,
trans_l1_l4_registry, trans_text, trans_core, trans_internal_abi) listed.
0c's step 2 (the 32-gate default set): claude-0c/gates-default-set-32
ad3fc6b8 on d7f3b569, run_gates.ps1 only, merged over lane_oracle without
conflict; cold in a fresh worktree "gates GREEN: 32 of 32 in 535s", the
eleven core gates 213 s (lane_oracle 56 s), the 21 added 322 s (graph_abi
85 s, entry_turn 12 s, turn_step_child 0 s, the 18 port parity runners
11-20 s each), each pass line from the gate's own log; the S3 wake oracle
not among them until the lead's S3 change; falsifier: a deleted runner
file stops the chain at that gate by name with "not run". Accepted; lands
through the lead after O1, runner only; every landing chain then runs the
32 and reports the time.
D1 written and committed locally on d6/lock-d-create-id as 45f5e82c (off
d7f3b569, unmeasured, unpushed until the window): lmx_msg_find_create
deleted in lm1 and lm2 with its call in create_prepare and the unused
local; LmxMsg.create_id with its store and the clears in
handoff_supervision and orphan_children; two header comments; the checks
"idempotent create changed sibling/path" (lmx_message_selftest) and the
executor selftest handoff check's create_id clause; kept until D2 the
signature's parameter, and until M the UI lane's reservation with
entry_turn's "create_id is refused" check; grep for create_id and
find_create across stg/l1_baseline at 0. Landing order after 57's "done":
O1 (measure, push, land) -> ad3fc6b8 (the 32 gates) -> D1 (run_port_message
plain and -LaneCheck, lmx_message_selftest's runner, scenario36, run_gates)
-> b5's mixa deletion; S3's oracle waits for the S3 change.
0c's falsifier for ad3fc6b8 measured after it was sent (a correction of the
record): with run_port_message.ps1 moved aside, run_gates exits 1 in 1 s
with "gates RED: stopped at lane_oracle after 0s", the row "lane_oracle
FAIL exit=-196608 0s" and 31 of 32 rows "not run"; the file restored with
the same blob. Rule restated: a falsifier is run before it is reported.
0c's re-probe of the S3 wake oracle (ee5b2cb0, the four evidence stderr
files identical, each violating (site, owner) once, callers tagged): with
run_one's wake-all present, one violation is the wake-all itself
(ctx_visit_wake, owner 3, admits 0, wakes 1, input 0); every other line
originates in exec_start_map_kick (exec.c 2877): 61 lanes plus owner 5
woken a second time after their admission's wake with the input still
waiting (admits 1, wakes 2; owner 5 at 2/3), and owner 7 kicked with no
admission (admits 0, wakes 1, input 0, ready 1); with the wake-all removed
the same three exec_start_map_kick groups remain. A finding, not a
miscount: the start map kick wakes lanes regardless of admission, which
the model forbids (a lane's wake comes only from an admission into its
mailbox; a launched lane's first wake is its first Message); the kick's
removal folds into S3 or M. The probe caps at 64 pairs and hit the cap, so
sites beyond may exist; the landing note replaces ee5b2cb0's false "0
sites" line. Falsifier at ee5b2cb0 with the wake-all removed:
run_port_message -LaneCheck exits non-zero with "LANE WAKE FAIL
site=wake_addr_locked owner=4 admits=1 wakes=2" (19 s).
Corrected by 0c the same hour: the probe records only whether input is
present, not how many envelopes wait, so a miscount in the oracle is not
ruled out; the 61-lane group fits the kick re-waking one unconsumed
admission but does not prove it. Ordered: the strict check printing the
inbox length beside admits for every violating pair (length greater than
the counted admissions is a miscount; equal with wakes greater is the
kick's extra wake; owner 7 stands either way); the finding is "fits, not
proven" until its numbers are in.
Mikhail on the lm2 deletion's scope (2026-09-15, to "is splitting the shared
declarations while keeping the L2 sources very hard?", answered: no, a
mechanical cut of own_abi.lm2's message-thread declarations, table_descriptor
untouched, verifiable only by grep since nothing builds lm2/*.lm2 today):
"расщепляйте" (split). Stage O's scope: the L2 sources (parser.lm2,
l1trans.lm2, printTree.lm2, table.lm2, registry_control.lm2, mix/) and
table_descriptor.lm2 stay; own_abi.lm2 is split (the message-thread
declarations out, the ownership and arena ABI kept, the five predefs still
resolving by grep); own.lm2, the trans.lm2 chain (thirteen files), its kept
generated C (trans.lm1.c and the two REST .lm1.c with their gitignore
exceptions), its CMake targets, the 13 C tests and the fixtures go, each
named, mirrored in stg/l1_baseline/lm2; b5's, one commit per group on
sonnet/stage-o, red by the greps, green with the live chain's gates and
CMake configuring clean. The lead's O1 measure is running after 57's
"done" (red at fb1c7221 failing to link; green on 69255593 alone and in
the defaults; the parent_msg falsifier; run_port_message plain and
-LaneCheck), then landing on d7f3b569, then ad3fc6b8, then D1; the kick
finding folded into S3 in the design at 1f3f4dfd (the wake-all and
exec_start_map_kick go together), the strict check and an uncapped probe
with S3's green.
0c's self-build probe of the live L1 chain (cold on ad3fc6b8, scratch only,
9 s; proposed runner run_self_build.ps1: B0 the gcc of the committed
lm1/build/l1trans.lm1.c; pass 1 regenerating the 8 committed generated
files of buildCore.lm1's map, compared by git blob id; B1 from pass 1, pass
2; B2 from pass 2, pass 3; the fixed point pass 3 equal to pass 2): the
current l1src self-builds, fixed point 8 of 8; the committed C is stale:
against B0 7 of 8 equal, stg/l1_baseline/lm1/build/l1trans.lm1.c differing
from line 6403 (committed 2026-09-12 in 50067a64, its source changed
2026-09-13 in 7cc1c473); against B1 4 of 8 (own, parser, l1trans,
printTree differ). gcc's output is not reproducible here (the same C hashed
38C54956 and 66B2C2D0 in two runs), so binary hashes certify nothing and
L1_PIN's meaning is the source of the previous binary. Ruled: the fixed
point defines the gate, the row red until the stale files are regenerated
by the translator and committed in the lead's lane, never by hand.
Contradiction before Mikhail: the repository carries two diverged copies of
the L1 chain, the root's lm1/build and l1src (l1trans.lm1.c a978175a of
2026-09-13 from l1src/l1trans.lm1 c35f8991, plus the dead chain's
trans.lm1.c and REST files) and stg/l1_baseline's (90b22204 of 2026-09-12
from 1dbf2303), 31 insertions and 72 deletions apart in l1trans.lm1, the
other seven sources and their C identical; the core, l2src and every gate
live under stg/l1_baseline, whose buildCore.lm0.bat still needs the old
chain's binaries (trans.lm0.exe, printTree.lm0.exe, libparser/libown
.lm0.a) that the lm2 deletion removes. Put to him: which tree is the
project's self-build root and what stg is; the coordinator proposing
stg/l1_baseline as the live tree, its bootstrap rewritten onto the live
chain, the root copy brought to it. Until his word: b5 leaves the CMake
targets and the kept generated C alone (the split and the source
deletions proceed), 0c's row stays red on the stale files.
b5's second stop on stage O (2026-09-15, nothing touched): parser.lm2, one
of the kept L2 sources, calls the message-thread API directly in its own
text, not only through own_abi.lm2's declarations: lm_own_arena_init(...,
c.lm_lmx_message_thread) four times at 668, the owner_thread comparisons
against c.lm_lmx_message_thread at 680 (the ownership check gating the
document's owner destruction), and lm_message_thread_component_get /
_attach / _remove at 1401, 1417, 1426 (the per-thread P0 registry);
c.lm_lmx_message_thread is the global the deleted main scaffold populates;
l1trans.lm2, printTree.lm2, table.lm2 and registry_control.lm2 have none of
it. Ruled by the coordinator (Mikhail may overrule): parser.lm2 stays as
unbuilt reference for the L2 port and is not rewritten now (nothing can
translate or test a rewrite); the three sites are marked with the comment
that the Message mechanism is temporarily not used and that in the L2 port
the parsing Message's own arena and state replace the thread's arena and
the per-thread registry component, undeclared since own_abi.lm2's split;
the file's header says it does not translate until that port; the split
and the source deletions proceed, the CMake and generated-C groups stay
held on the tree question. 0c's strict wake probe (inbox beside admits, up
to 1024 pairs with a cap line, both variants) is ready and runs in a slot
after O1's measure.
Mikhail on the two trees (2026-09-15, verbatim): "stg/l1_baseline был у них
стабильной сборкой на которой они пилили stg\l1_baseline\l2src то есть и
stg\l1_baseline\l1src был стабильной самосборкой. А lingvamyxa\l1src была
разработка текущего L1. lingvamyxa\lm1 \lm2 это по-моему устаревшие
исходники" (stg/l1_baseline was their stable build on which they developed
stg/l1_baseline/l2src, so stg/l1_baseline/l1src was a stable self-build
too; lingvamyxa/l1src was the development of the current L1;
lingvamyxa/lm1 and lm2 are, in my opinion, obsolete sources). Measured
alongside: the root has l1src (the current L1), lm1/build (its generated C
plus the dead chain's trans, trans_library, rest and vcpkgFetch files) and
lm2/; stg/l1_baseline has its own l1src and lm1/build, l2src (the core,
l2trans, every gate), gate.ps1, buildCore.lm0.bat, oldchain/ and
README.snapshot-origin.txt; every runner references stg/l1_baseline's
l1src. The consequence is put to him for confirmation before anything
moves.
Read alongside his words: stg/l1_baseline/README.snapshot-origin.txt
(2026-09-07: a frozen self-build baseline of the L1 sources and the
tests/l1 runners; "development continues in root l1src; this tree is for
later baseline-vs-dev comparison only; do not treat as live build input")
and README.txt (after 2026-09-08 the tree is a second build root: every
tests/l1 runner and buildCore.lm0.bat anchor at stg/l1_baseline; "L1
development keeps building at the repo root against the live l1src, and L2
work builds here against the frozen slice; the two never share
build/l1trans, build/obj, build/lm0 or lm1/build"). Put to him as three
yes/no questions: (1) the self-build's target is the root's current l1src,
its committed generated C regenerated by the translator and committed;
(2) the frozen slice stg/l1_baseline/l1src refreshed from the root's l1src
at its fixed point (stg's lm1/build regenerated), keeping the two build
roots for now, one root after the self-build, versus collapsing to one root
now; (3) the root's lm2/ deleted whole (superseding "split", nothing left
to split) and the dead chain's files in the root's lm1/build (trans,
trans_library, rest_lmx_http_client/server, vcpkgFetch) with their
gitignore exceptions removed, the live chain's generated C kept.
Two facts from the lead for question 3: the snapshot README lists among its
prerequisites the root's lm2 registry and predef files, lm1 includes and
build/lm0/trans.lm0.exe, while a literal-path grep of stg/l1_baseline finds
one root read only (run_port_parser.ps1:31, tests/p0_tree_contract); not a
proof (joined paths and .lm1 includes not covered, no build run). Ordered
as question 3's falsifier whichever way he answers: a cold stg/l1_baseline
self-build (tests/l1 run_seed then run_gen, gate.ps1's buildCore.lm0.bat)
in a scratch worktree with the root's lm2/ and the dead chain's files in
root lm1/build moved aside, and with build/lm0/trans.lm0.exe absent; green
means those prerequisites are stale text; a red names the dependency by
file and line. O1 is 06ab81b0 (pushed, measure green), landing after 57's
inbox probe, then ad3fc6b8, then D1; none touch the bootstrap.
b5 on sonnet/stage-o before the hold arrived, under the "split" reading:
701a1b80 (own_abi.lm2 split, the message-thread declarations removed, the
ownership, arena and REST ABI kept), e09a457e (own.lm2 and the twelve-file
trans.lm2 chain deleted in lm2/ and stg/l1_baseline/lm2/,
table_descriptor.lm2 kept), 6c6aaa60 (parser.lm2's three undeclared call
sites marked with comments only); they stand and continue if "whole" wins.
Found while checking the tests group: the rest_lmx_http_server_civetweb.c,
rest_lmx_http_client_libcurl_loopback.c and the other rest_lmx_http_*.c
files under tests/ are REST's own backend implementations plugging into the
deleted runtime through the LmMessageThread family, not test fixtures;
ruled: they go as REST's own group under Mikhail's (b) ("REST-провайдеры и
реестр парсера уходят вместе с ним"), each named with its role, after a
grep for rest_lmx and LmMessageThread across the live chain (l2src, root
l1src, stg's l1src) finds no user.
b5's pre-check for the REST group: git grep for rest_lmx and LmMessageThread
across stg/l1_baseline/l2src, root l1src and stg/l1_baseline/l1src finds
one documentation hit (LOCK_REMOVAL_O0_MEASUREMENT.txt) and no code; the
live chain has no REST user; the group goes when the deletion resumes.
Mikhail's answers to the three questions (2026-09-15, verbatim): "1- да
2- обновить срез сейчас (дёшево, ворота остаются), один корень -- после
самосборки. 3- ну же не знал что вы не разобрались в их барахле который
кстати один чат вел вместе с ними. удаляйте" (1 yes; 2 refresh the slice
now, cheap, the gates stay; one root after the self-build; 3 I did not
know you had not sorted out their junk, which one chat led together with
them; delete). Applied: (1) the self-build's target is the root's current
l1src, its generated C in root lm1/build regenerated by the translator at
the fixed point and committed (the lead's lane, never by hand); (2)
stg/l1_baseline/l1src refreshed from the root's l1src at that fixed point,
stg's lm1/build regenerated, the full gate run on the refreshed slice,
stg's buildCore.lm0.bat rewritten off the old chain's binaries (the lead's
landing); the two build roots stay until the self-build is achieved, one
root after; (3) root lm2/ deleted whole (everything under lm2/ and
stg/l1_baseline/lm2/, the split own_abi.lm2 included, superseding "split"),
the dead chain's files in root lm1/build (trans.lm1.c, trans_library.lm1.c,
rest_lmx_http_client/server.lm1.c, vcpkgFetch.lm1.c) with their gitignore
exceptions, the CMake targets, the REST backends group, the 13 C tests and
the fixtures (b5's, one commit per group, red by the greps, green on the
live chain's gates); the lead's cold stg self-build probe with root lm2/
and those files moved aside gates the CMake and generated-C groups; 0c's
self-build row takes the root chain as its base (the fixed point and the
committed C equal to it) with a second row for the slice's equality to
the root's fixed point. Order: 0c's inbox probe, O1 lands, ad3fc6b8, D1,
the lead's stg probe, b5's groups, the regeneration commit, the slice
refresh with the full gate.
0c's strict wake probe (method corrected: the probe had credited one
admission after every violation, so only each lane's first line carried
true counts; first lines used): P2 with the wake-all removed (exit 0, 55 s,
151 identical lines in the four stderr files, the cap not reached, 76
lanes): inbox greater than admits 0 (no oracle miscount); inbox equal to
admits with wakes greater 74, of which 72 exec_start_map_kick (exec.c 2877)
at admits 1, wakes 2, inbox 1, owner 5 at 2/3/inbox 2, owner 1 at
site=launch 1/2/inbox 1 with an internal envelope; admits 0: owner 6 at
site=launch (inbox 0, ready 0) and owner 7 at the map kick (inbox 0, ready
1). P1 with the wake-all present: the same plus owner 3 (the wake-all) and
owner 1 re-woken by the wake-all with an already consumed admission. The
finding is proven: exec_start_map_kick's extra wake of admitted lanes and
its kick of empty lanes, the launch's wake of a lane with nothing
(site=launch, hidden by the old cap), and the wake-all; all three go with
S3 (or M where launch is); S3's green is the probe at 0 lines with the
oracle in the defaults.
0c's file survey for the slice-equality row (tracked files on d7f3b569):
root l1src 17 files, stg/l1_baseline/l1src 16, none only in stg; two
differ (l1trans.lm1: root c35f8991, stg 1dbf2303; p0.h, a tracked source
distinct from p0.h.lm1); one only in the root,
l1src/__pycache__/port_parser.cpython-311.pyc, tracked Python bytecode of a
manual migration utility. Ruled: the row compares every tracked file with
no exclusions; the .pyc is a defect and its removal from git with a
__pycache__/ gitignore line is named in the lead's refresh commit; the
self-build row on the root base as drafted (B0 root lm1/build/
l1trans.lm1.c, the eight sources and outputs tracked at the root, no
shadowing header), both unmeasured until the machine frees.
The lead's probe (a) written, five cold variants at the integration tip in
fresh worktrees: A control; B root lm2/ and root lm1/build's trans and the
two rest_lmx_http .lm1.c moved aside (trans_library and vcpkgFetch are
inside root lm2 or untracked); C root lm1/ and lm2/ moved aside whole; D
stg's pinned trans.lm0.exe absent; E the root route for (b), root
buildCore.lm0.bat plus root tests/l1/run_gen.ps1 with B's moves; A-D run
stg's buildCore, run_seed and run_gen. Read, not yet run: stg's scripts cd
into stg/l1_baseline and use stg's own lm2, lm1/build and build/lm0, so
the snapshot README's root prerequisites describe its origin; stg's
buildCore.lm0.bat (77-82) and run_seed.ps1 (48) need trans.lm0.exe and
run_seed the untracked oldchain archives (D expected red there; run_gen
boots natively from stg's l1trans.lm1.c, and (c)'s bootstrap rewrite
removes the need); root CMakeLists.txt reads root lm2 (343-355) and root
lm1/build's trans, vcpkgFetch and rest_lmx_http .lm1.c (331-341), so root
lm2's deletion lands with b5's CMake group; root tests/l1/run_seed.ps1
reads root lm2 (58-62) and trans.lm0, the old seed, and goes with it. b5's
grouping adjusted: CMake configuring clean is the branch's final check,
per group the greps and stg's gates; the branch lands whole through the
lead. S3's strict re-probe with the launch site is in the design.
O1 landed on integration as c94d3e31 (d6/o1-turn-arena 06ab81b0 merged
onto d7f3b569: the turn Message's arena through the core's blocks,
lmx_msg_turn_current, lmx_msg_turn_new_zero and _copy_bytes in lm1 and
lm2, the O1 test 23/0 in scenario36's defaults, scenario36's --wrap hook):
measured on the merge run_model_scenario36 PASS, run_port_message
-LaneCheck PASS, run_gates 11 of 11 in 209 s; before landing red, green,
the defaults, the parent_msg falsifier and plain port_message. Next: 57's
32-gate set ad3fc6b8 onto c94d3e31, then D1 (trial merge clean), then the
probe, the root regeneration and the slice refresh with the .pyc removal.
b5's stage-O branch sonnet/stage-o at 466d587c (groups 1-3 done: a6e3ca43
the whole of lm2/ and stg/l1_baseline/lm2/, the REST backends group, the C
tests and fixtures). Found by b5's cold gate run and stopped on:
stg/l1_baseline/tests/l1/run_smoke.ps1:175 checks the built l1trans.exe
for staleness by reading lm2/l1trans.lm2 (LastWriteTimeUtc and a content
hash), the dead chain's L2 source, deleted by group (1); the live
translator's source is l1src/l1trans.lm1 with lm1/build/l1trans.lm1.c.
Ruled: l1trans.lm2 does not come back; the check is a runner hazard (the
wrong target and a timestamp key) and 0c replaces it with a content-hash
check against the live source and its generated C, red-first, landing with
or before b5's branch. The cold worktree's gen0/lm0 misses are the seed
never run there (the oldchain archives exist only in the main checkout)
and stg's bootstrap need for the old binaries, which (c) removes; so b5
reports greps and file lists per group and the lead's landing chain
measures the gates in a warm worktree after (c), the branch landing whole.
The 32-gate set merged as 88934463 on c94d3e31, its gates running; D1 next,
then probe (a). (b)'s driver: the root's buildCore.lm0 is not a fixed-point
driver (its output_dir is build/lm0/next, 436-439, but both generate_all
calls use build/lm0/l1trans.lm0 at 825, 833 and 878, so its second pass
reruns the previous binary, and it compares nothing), so (b) uses a thin
driver regen_root.sh on d6/root-regen in a fresh worktree: T0 from gcc on
the committed lm1/build/l1trans.lm1.c; three passes, each translating the
eight files in place in lm_build_generate_all's order (p0.h.lm1 first,
since own and parser include l1src/p0.lm1.h from lm1/build) and building
the next T; green: pass 3 equal to pass 2 byte for byte on all eight, the
other tools compiling from the new C with buildCore's flags, root
tests/l1/run_gen.ps1 green, git status naming only the eight paths; on
green the eight committed with blob ids in the note and pushed, the
landing separate; the driver reports how many files pass 1 changed from
the committed C. Constraint for 0c's run_smoke fix: both hashes computed
at run time, no pinned value, or the check reddens on (b) and (c).
0c's contradiction on the run_smoke fix (2026-09-15): nothing records source
hashes when an l1trans.exe is built (run_gen logs exe hashes only and
keeps each gen's C in build/obj/l1trans/<gen>/l1trans.c); run_smoke
defaults to gen0 and skips its newer checks there because gen0 is built
from lm2/l1trans.lm2 by run_seed (L350-353); gate.ps1 builds the gen0 seed
through the old chain and runs every suite on gen0 and gen2 with a gen0
fail allowlist (L48-73); so the lm2 deletion removes the seed, run_smoke's
default generation and gate.ps1's gen0 half. Ruled: option (a) with the
seed re-based on Mikhail's "the previous binary always exists": run_seed
builds gen0 by gcc from the committed lm1/build/l1trans.lm1.c (the eight
committed files as the bootstrap, nothing from lm2, no trans.lm0, no
oldchain archives), gen1-gen3 as now, the fixed point gen3 equal to gen2
and after (b) gen2's C equal to the committed C; run_gen and run_seed
write build/l1trans/<gen>/l1trans.sources.txt (SHA256 of l1src/l1trans.lm1
and of the C the gen was compiled from), run_smoke compares at run time
naming the changed file and failing on a missing record, its lm2 branch
deleted, its default gen0 kept as the committed-C binary; gate.ps1's gen0
half stays on the new gen0 with its allowlist re-measured; allowlist
run_seed, run_gen, run_smoke, gate.ps1; red today (run_seed needs
trans.lm0 and the archives), green cold in a fresh worktree with root lm2
and the dead files aside. This is the runner half of (c), 0c's; the lead
keeps stg's buildCore.lm0.bat rewrite and the slice refresh; both land
together after (b) through the lead, then b5's branch.
(c)'s two points from the lead's reading, ruled 2026-09-15 (Mikhail may
overrule): (1) gen0 becomes the previous binary (gcc on the committed
lm1/build/l1trans.lm1.c), run_seed builds it, the gen0/gen2 double pass
stays, gate.ps1's $seedLimited empties, the gen0 skips in the eleven
tests/l1 runners (smoke 186/352, ifdef 109, expr 96, define 110, c_array
98, parser 49, control 89) go, the gate kept; 0c's, allowlist tests/l1/
run_*.ps1 and gate.ps1. (2) the parser oracle: run_parser.ps1:17 and
run_legacy_p0.ps1:15 compare L1's printTree against
build/lm0/printTree.lm0.exe, the old chain's binary; ruled (b), committed
goldens generated once from that exe while it exists, with the generating
command and the oracle's provenance in the goldens' README, the runners
comparing against the goldens, red-first, a later disagreement a
question and never a silent regeneration ((a) a pinned binary rejected,
gcc not reproducible; (c) a self-comparison rejected); 0c's, the same
branch as the seed, landing with the lead's buildCore rewrite after (b).
Mikhail's question on the mailbox monitor (2026-09-15, verbatim): "\"notify
при admission, wait при take\" -- а кого нотифицировать-то внутри одного
потока? Он же не выйдет \"проверить почту\"" (whom to notify inside one
thread? it will not come out to check the mail); answered by the
coordinator, awaiting his word: notify wakes the mailbox's owner thread
sleeping in wait() inside take between its turns, the sender being another
OS thread under the same monitor, Java's producer/consumer queue in its
simplest form; a self-send notifies nobody, harmlessly; no other wake in
the project.
The seed re-base's scope (0c's reading on d7f3b569, ruled): six more
tests/l1 suites skip checks on gen0 with the lm2 seed as the reason
(run_parser 48-50, run_expr 96-97, run_c_array 97-98, run_define 109-110,
run_ifdef 108-109, run_control 87-90); they join the branch, each skip
deleted with its justification quoted and the check re-measured on the
new gen0; run_smoke's second gen0 branch (186-187) is the same deletion;
cancel_spin.lm2 and tests/l1/invalid_*.lm2 are translator inputs by
extension and stay; gate.ps1's first step, stg's buildCore.lm0.bat, needs
the old chain's binaries (63-83), so the lead's buildCore rewrite comes
first as its own branch off the integration tip after the 32-gate landing,
0c stacks the seed branch on it and measures the cold green there with
root lm2 and the dead files moved aside, the two landing together after
(b), b5's branch after them.
Mikhail on the mailbox monitor (2026-09-15, verbatim): "тупейший вложенный
synchronized по локу на чтение и запись как в Java, без остальных
усложнений вообще. \"notify получает не «поток», а поток-владелец ящика\" --
ну а смысл его нотифицировать если он занят в turn?!!! Выйдет и посмотрит"
(the dumbest nested synchronized on the lock for reading and writing, as in
Java, with no other complications at all; what is the point of notifying
the owner thread if it is busy in a turn? it will come out and look). So
the mailbox's synchronization is synchronized on the mailbox object for
every read and write, reentrant as Java's, and nothing else: no wait, no
notify, no condition variable, no wake primitive; the owner thread looks
at its mailbox itself when it comes out of its turn. Stage Y: Y2 the
monitor's reentrant enter/leave only, Y3 the mailbox's reads and writes
under it; S3: the wake-all, the map kick and the launch wake go and
nothing replaces them; 0c's oracle becomes "no wake site exists" (any wake
red, the green 0 sites). Put to him by the coordinator: what the owner
thread does when its mailbox is empty after a turn (the thread ends and
the next admission launches the turn, or it waits on something); nothing
coded for the empty case until his word.
stg's buildCore.lm0.bat rewrite committed locally on d6/stg-buildcore (base
c94d3e31; 9 insertions, 25 deletions: the oldchain archive restore and the
trans.lm0.exe/printTree.lm0.exe check deleted, the four gcc builds from
stg's lm1/build kept, the header saying nothing of the old chain is
needed), to be moved onto the tip after the 32-gate landing, measured red
(the old bat in a fresh worktree without pinned inputs) and green (the new
bat cold), pushed for 0c to stack on; stg/l1_baseline/oldchain/ with its
README.txt goes in the same branch.
