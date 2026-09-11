# Reply: Message-owned scheduling, not a mandatory execution framework

**Discussion response · 11 September 2026**

This is an updated response to *Message-Preserving Synchronization Lowering*, version 0.1. It supersedes the earlier response's overly restrictive treatment of scheduling and physical threads. Read it alongside the current `Lingvamyxa_spec.txt`, especially sections 1.3, 19.28.R2.2, 19.29.6 and 21.10.

The architectural decisions below are agreed. Implementation alternatives and unfinished work are identified separately; this is not a claim that the prototype already implements the target.

## 1. The language is a toolkit

Lingvamyxa supplies composable tools, not a mandatory application framework. A program chooses the receivers, modules and execution mechanisms it needs. An L1 program can run native C code without using Message at all.

Choosing Message does, however, select its ownership and execution contract. A backend cannot silently replace that contract with a different global management system.

Our implementation policy is also explicit: the language core and complete standard distribution coordinate through Message. Low-level synchronization within that package is confined to implementing Message itself. Low-level synchronization remains available as an L2 language capability; it is not removed from the language.

## 2. Every running Message has its own scheduling mechanism

**A parent Message contains the scheduler for its direct children. Each child can choose another scheduling implementation for its own children.**

The scheduler's policy and management state belong to the parent's ordinary Structure data. There is no separate language scheduler above the Messages, no shared global Message registry, and no shared global management lock.

This mechanism is not limited to Messages that already have children. Every running Message has its own local scheduling/lifecycle state: even a childless non-root Message periodically queries its parent and requests its own orderly closure after prolonged absence of a response. Managing direct children is an additional responsibility of that same mechanism. The initial implementation may reuse one implementation for every running Message, with separate state for each instance; it need not generate different scheduler code for each Message. Local ownership does not imply a dedicated OS thread or preemption of an unfinished turn.

Consider this family:

```text
P
├── A
│   ├── A1
│   └── A2
└── B
```

P schedules A and B. A schedules A1 and A2. P does not walk or schedule its grandchildren. Scheduling responsibility is recursive through ownership, not through a central traversal of all descendants.

Each live context has its own arena, mailbox, parent relationship and direct-child list. Parent/child relationships do not restrict all communication to the family tree: addressed messages can use other held recipient capabilities under the existing authority and transfer rules.

## 3. Message is not synonymous with an OS thread

A Message may be inactive. A running Message has a logical serial execution lane, but it is not guaranteed or required to have a dedicated OS thread.

Physical execution is a mapping selected by the parent-owned implementation. For example:

```text
P executes A's turns on P's current execution thread.
A uses its own scheduler to start threads for A1 and A2.
A1 and A2 can execute in parallel.
```

A does not need a dedicated thread of its own to manage those children. A branch does not have to inherit its parent's physical execution strategy.

Native thread start/join, wait/wake and destination-mailbox synchronization are necessary implementation primitives. They support Message; they do not become an external L1/C policy engine that owns the family tree. The OS still schedules its native threads.

## 4. Whole turns in sequential mode

When a parent chooses sequential iteration on one execution thread, the selected child retains control until that child's turn completes. The parent does not switch siblings halfway through that unfinished turn.

This mode therefore does not require a new mechanism for capturing and restoring arbitrary active call chains. It is distinct from the language's explicit activation-level `yield` contract and from OS scheduling of physical threads.

Selection order, batch size, fairness and resource allocation are replaceable policies. Round-robin versus draining a child's inbox is not a language-design blocker and is not a new mandatory rule. An initial implementation can use a simple policy without making it universal. Resource requests can later be expressed through ordinary parent/child Message protocols where needed.

Serial own turns and FIFO admission within a mailbox remain part of the selected Message contract. Replacing the order of selecting direct children is not the same as silently changing those guarantees.

## 5. Interpreter state: useful possibility, not a universal promise

An interpreter is expected to be able to preserve execution state for suspension and resumption in most ordinary interpreted cases. This is not guaranteed at every execution point or across every native operation, and it is not a prerequisite for the initial sequential scheduler.

The absence of a universal suspension guarantee means that some suspension points or operations may be unsupported. It does not permit resuming a supported operation with incorrect values.

Published method fields are not, by themselves, a complete record of every active invocation. Under the current cache/publication rules, an active call may retain values different from the graph's current values. Declared and hidden arguments, return locations and intermediate results also matter. An interpreter that supports such suspension must preserve the required activation state in addition to the published graph.

`toLmx`/`fromLmx` are not currently specified as automatic portable serialization of arbitrary native stacks and OS handles. Their codec role should not be silently enlarged to claim that this problem is already solved.

## 6. How the original proposal can help

The proposal's central principle is useful: the compiler preserves receiver-defined behavior rather than choosing semantics merely from a spelling such as `wait`.

The Slang analogy is likewise useful: a restricted implementation language can implement a richer machine and generate C without making the target language's execution arrangements the source language's ontology. The authoritative implementation direction remains LMX/L2 -> L1 -> C, with generated C snapshots retained for portable bootstrap. The [Slang architecture documentation](https://github.com/pharo-project/pharo-vm/wiki/Slang) supports this distinction; it does not require us to import Smalltalk's scheduler.

The proposal should be adapted as follows:

| Proposal element | Interpretation for Lingvamyxa |
| --- | --- |
| One owner mutates synchronization state. | A particular Message owns that state. Do not introduce a separate domain owner above the Message family. |
| Validate an operation before executing it. | Keep acceptance, execution and semantic completion distinct. Admission is not completion. |
| Queue pending requests and correlate completion. | Use private protocol records in the owning Message where needed. Other Messages do not mutate them directly. |
| Prevent lost wakeups and duplicate completion. | Preserve these invariants in Message protocols and the narrow mailbox implementation. |
| Select ready work. | The parent selects direct-child work under its replaceable policy, not a global scheduler over all tasks. |
| Preserve a continuation. | Use explicit protocol state or an appropriate supported backend mechanism when actually needed; do not require transformed call chains for whole-turn iteration. |
| Semaphore/mutex cancellation rules. | Potential tools or protocol-specific designs, not automatically new universal Message semantics. |
| Compare execution traces. | Check required outcomes and ordering constraints, not one global schedule for independent Messages. |

The proposal's semaphore example currently lets a domain owner manipulate waiter task records and a ready queue. That is not yet the required parent-owned implementation. A corresponding Message service would own its own pending requests and issue replies; it would not thereby acquire access to arbitrary task frames or a global ready table.

Similarly, a native wake and a semantic Message are not mutually exclusive alternatives. Admission of an ordinary message may use a native wake internally. The physical implementation must not introduce a second source-visible coordination system.

## 7. Example: concurrent application work

A UI-associated parent can delegate work to a child Message. That child can manage its own worker children using another scheduling implementation, even if the child's own turns are executed by its parent.

For example, a copy-job Message owns job state and worker relationships. Workers report progress and results through messages; the job owner updates its own records. The UI processes its own turns independently of workers mapped onto other execution threads.

The scheduler does not invent the application's completion meaning. For a copy request, completion means the copy completed; for a delivery request, it means the specified delivery completed. Enqueuing a request is not that semantic result.

A handler must also respect the existing staging/publication rules: merely staging a request and then blocking for a reply does not make the request visible. Protocols must allow their completion messages to be admitted and processed. This is not a reason to require a new global scheduler or an implicit `await` transformation.

## 8. Local lifetime and address depth

Parents manage direct children; children manage their descendants. Normal closure propagates through that responsibility. Automatic child-to-parent liveness polling and cooperative end-turn closure retain their agreed roles. They do not require a process-wide scan.

The default probe originates at the child, but timeout supervision is symmetric. The child waits for its parent's response and requests its own orderly close after prolonged silence. The parent may track expected communication from the child, including its periodic queries, and request that child's orderly close if those cease. No second automatic probe stream is necessary. Each side owns its expectation and deadline state; lack of a response is not proof of death or completion. An unresponsive handler that cannot reach end-turn still requires the exceptional failure path, not a claim that cooperative closure has already finished.

A finished child's arena must be reclaimable independently of unrelated live family members. In-flight sends and retained recipient capabilities still require safe control-state lifetime handling. Keeping all contexts until a global runtime is destroyed is not the target solution.

The prototype's path-depth limit of 16 has no foundation in the language model and must be removed from the replacement. The path appends one parent-local child counter segment:

```text
34.3 -> 34.3.1 -> 34.3.1.1
```

Storage sized for the required path is one straightforward implementation option. Checked resource and representation failures are real limits; an arbitrary depth constant must not define the language's topology. Published addresses must remain stable.

## 9. Current work and remaining questions

The reviewed prototype has Windows tests for overlapping execution, whole-context serialization, restart, dynamic child launch and UI activity during worker execution. It still contains central table/lookup/lock structures. Those remain deviations to remove, not an accepted Message-first implementation or an application delivery.

Grok's core task is to implement the actual parent-owned mechanism, validate mixed branch mappings and local lifetime, and deliver it for application integration. Full L2-authored self-hosting remains the subsequent goal. OpenCode can develop bounded application modules independently, without introducing a private concurrency framework or binding the application to the rejected prototype.

Before implementing a genuinely unclear ownership, lifetime or observable execution rule, we will ask the language author. We will not treat every overridable policy choice as a new normative decision that blocks work.

## Conclusion

**Message is a self-sufficient, composable tool for coordination. Its parent owns its scheduling relationship, and it can independently organize execution of its own children.**

The right implementation is neither a mandatory OS thread for every Message nor an external universal scheduler. It is a replaceable, parent-owned arrangement expressed through the language's tools, with narrow native support underneath.
