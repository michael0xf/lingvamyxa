# L2 runtime plan (2026-09-14)

Owner: review chat (`lingvamyxa-e2`, Fable 5.1). Confirmed by Mikhail on
2026-09-14 (LEAD_REVIEW_20260914.md §6, decisions 15 and 16). This replaces
§6.4 of CORE_LEAD_INSTRUCTION_20260913_en.md as the runtime lane's plan.

## 1. What changed

The runtime lane so far ported the L1 runtime modules to L2 one by one, each
as a library unit with a parity runner against the unchanged L1 module. That
work proved the translator expresses the runtime (fifteen units, seven
translator gaps found and closed, RUNTIME_L2_PORTS.txt) and it stays as that
evidence. It is not the L2 runtime.

Mikhail's ruling: L1 was deliberately left to the agents in a freer form,
because its purpose is preparation for a fairly direct translation to C. L2
is implemented exactly per the specification. The L1 runtime is the
temporary bootstrap that spec 19.29.6 allows ("temporarily L1/native
bootstrap"), not the oracle for L2.

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

Each stage is one or more L2 units under `stg/l1_baseline/l2src/` with the
`profile: runtime` directive (decision 14: the executor's own core is never
polled; checkpoint diagnostics stay), committed at the end of the stage
(decision 16), and accepted by the spec's own checks, never by the L1
selftests.

1. **Message as Structure data.** One Structure family for every Message,
   executed or not: its arena (blocks and ranges, one owner), running and
   success, the FIFO inbox and staged outbox, the parent capability, the
   direct-child list, and the parent-owned scheduler state for those
   children. Creation by merge only (a copy, its own arena; 19.29.6 "creation
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
4. **Close, liveness, failure.** stop as KIND_STOP admission setting closing
   only; family close per §32; liveness queries and timers per §33 as
   self-maintenance of every running Message; failure handoff per §34 with
   the HISTORY roots (lmx_msg_history_owned's contract).
5. **Root Message and bootstrap.** OS startup is the root Message; the
   external process entry runs in its turn loop. The L1 runtime remains the
   bootstrap underneath until the L2 runtime hosts itself; then the L1
   modules are retired stage by stage.

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

- Lead (`lingvamyxa-d6`): translator gaps found by this lane, with fixtures
  as before; merges through integration; the model's §31/§34 wording carries
  decision 14; the full gate on every merge.
- Helper (`lingvamyxa-5e`): parser in L2 (§6.5) unchanged.
- Review chat: this lane, stage by stage; decisions carried to Mikhail;
  spec clarifications committed as docs the day they are made.
