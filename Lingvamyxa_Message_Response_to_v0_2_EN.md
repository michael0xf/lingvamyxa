# Response to Message-Owned Scheduling and Synchronization v0.2

11 September 2026. Discussion clarification, not an implementation-completion report. Read alongside the current `Lingvamyxa_spec.txt` and `Lingvamyxa_Message_Architecture_Response_EN.md`.

Version 0.2 correctly reflects the main model: every running Message owns its scheduling/lifecycle state; a parent schedules only its direct children; policies and physical execution mappings are replaceable. There is no global Message manager, shared global management table or global management lock. One implementation may serve many separately owned instances. The following clarifications should guide the next revision.

## 1. Wakeup correctness belongs to the synchronized mailbox

Message2's executing thread delivers Message1 to Message0's internally synchronized queue. That same queue/wait mechanism must handle concurrent admission and the transition into waiting correctly.

Protecting insertion alone would be insufficient if an empty check and sleep could lose an intervening notification. Coupling those operations is an implementation obligation of the existing Message mailbox, not justification for another scheduler, synchronization service or application-level lock. No particular native primitive is prescribed here.

## 2. Delivery does not create references into foreign mutable arenas

There are no ordinary graph references into another live Message's mutable arena. A delivery address or capability identifies a recipient; it does not expose that recipient's graph.

A non-executing Message can be consumed at end-turn and its storage adopted into the consuming Message's one logical arena. Ownership transfers; block addresses remain stable; there is no concurrently active source owner. Do not describe this as two live owners retaining cross-arena graph pointers.

Admission racing with recipient closure still needs safe handling, but that is a mailbox/control-endpoint lifetime problem, not a reason to permit shared mutable arenas. The physical solution must preserve this distinction.

## 3. The outgoing Message describes timeout and next action

An outgoing Message carries its timeout and what-to-do-next policy, directly or through an explicit policy link. No universal timeout duration, retry count or failure action is imposed.

The policy identifies the timed phase and its starting event: waiting for publication, admission or the operation's result are different expectations. A staged request is not yet published, and a request discarded with a failed turn must not be mistaken for one awaiting a recipient response.

Timeout does not prove completion, roll back external effects or make an unpublished request visible. Nor does a deadline execute itself: the responsible Message must obtain an execution opportunity. A handler cannot solve a wait for its own unpublished request merely by adding timeout data to that request.

## 4. Supervision is symmetric; the default probe comes from the child

The child sends a periodic query and waits for the parent's response. Prolonged absence of that response leads to the child's orderly self-close request. The parent waits for the child's periodic queries and may request the child's closure after prolonged silence. No second mandatory automatic probe stream is needed.

Each side owns its own expectation/deadline state. Queries, responses, admission receipts and operation-completion results are distinct events.

An independently executing parent can detect silence and request closure. If the parent synchronously entered a child turn on the same physical thread and that turn is stuck, the parent cannot execute its timer on that blocked thread. An independently executing ancestor may detect silence from its direct-child branch and request closure of that branch; it need not scan grandchildren.

Requesting closure is not completed safe teardown. Normal closure occurs at end-turn. A permanently stuck native turn requires the separately specified exceptional failure mechanism; a timeout alone does not authorize freeing its executing arena. This does not introduce mandatory dedicated threads, mid-turn preemption or a global watchdog.

## 5. The compilation-route contradiction is corrected

The C-machine route is exclusively:

```text
L2 -> Translator-L2 -> L1 -> Translator-L1 -> C99
```

The conflicting direct-L2-to-C99 allowance in specification section 1.2 has been removed. L2's machine surface is C-bound.

As described in `include_languages/vm_porting_plan_en.txt`, MIR, WASM linear memory and RISC-V targets reuse the generated C99 through suitable target toolchains. They do not bypass L1. These are planned targets, not three completed ports.

A separate L3 backend may emit managed-VM bytecode, for example L3 -> JVM classfiles, preserving L3 semantics with its target runtime. This is not an alternative low-level L2 compilation route.

## Verification and scope

We agree with testing per-recipient FIFO, non-overlapping own turns, mixed mappings, wake races, causal constraints and safe independent reclamation rather than imposing one total trace on independent Messages.

The permit-service example remains an illustration, not approval of a new mandatory semaphore subsystem. Experimental prototype code and passing individual tests do not establish completion of the required Message architecture, application integration or full L2 self-hosting.
