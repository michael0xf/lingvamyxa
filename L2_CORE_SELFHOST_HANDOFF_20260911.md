# L2 core: self-contained implementation handoff through full self-hosting

CURRENT USER OVERRIDE (2026-09-12): Grok owns ALL L2/core implementation,
including the former Codex parser/compiler/test/build lane. Claude owns all
mixa_manager coding. Codex ONLY plans, assigns/co-ordinates work and reviews
existing source/evidence; it does not code or run project builds/tests.
Planning/review/mailbox/monitoring records remain Codex-owned. This overrides
older role/coding instructions in this dated handoff. Current stage and
ownership are at the top of work_chat/CORE_CONTINUATION_20260911.md.

Prepared 2026-09-11 for a NEW Codex chat. The user intends to DELETE the old chat.
This document therefore preserves the decisions, rationale, current code state,
remaining implementation, questions and acceptance criteria needed to continue
WITHOUT that chat, its attachments or its private automation memory.

This is an implementation instruction and decision record, not a replacement
language specification. Normative references are the current repository files,
especially `Lingvamyxa_spec.txt` and `struct_refactoring_version_2.txt`. Where an
older inventory describes a rejected architecture, use the newer fixed contracts
recorded here and in the spec. Do not silently choose a new language rule to get
past an implementation problem.

Scope: CORE ONLY. App feature tickets and watcher setup are separate documents.
The app appears here only as a consumer of the concurrent Message API.

## 1. The actual goal

Implement the Lingvamyxa core and tools in L2 and achieve a genuine, repeatable,
portable self-build of the WHOLE promised toolchain:

```text
L2 source
  -> L2 translator
  -> generated L1 source
  -> L1 translator
  -> generated ANSI C99
  -> platform C compiler/linker
  -> executable able to repeat the same build / next / check / tests
```

The final L2 translator must be implemented in L2 and compile itself through
that chain. The L1 translator implementation must eventually also be L2 source;
L1 remains the intermediate language, not a permanent excuse to leave the kernel
in handwritten L1. Port the parser, runtime, build utilities and other core tools
too. L1 tests and deliberate intermediate-translator support can remain L1.
Approved foreign libraries, such as decNumber, remain foreign C.

Preserve the old working bootstrap algorithm exactly in its important behavior:
a small tracked portable generated-C source set builds a seed on another platform;
the seed builds the LM sources; the resulting executables support the same
self-build, next generation, check, tests and finalization. A Python/PowerShell
build of an L1-written translator is not the completed L2 self-build. A binary
oracle that only works on this machine is not the final portable bootstrap.

The user authorized Codex to WRITE CODE alongside Grok, not merely coordinate and
read specifications. Continue verified bounded stages until the agreed outcome;
ask promptly at genuine semantic forks. Finish with verified commits/pushes and
an honest report; do not start unrelated optimization projects after completion.

## 2. First actions in the new chat

Current continuation: [work_chat/CORE_CONTINUATION_20260911.md](work_chat/CORE_CONTINUATION_20260911.md).
The initial char-read and collector-extraction slices below are now committed
as `d21aed6` and `641f456`; keep the dated snapshot as history, not a duplicate
assignment. The continuation note records verification and the next split.

1. Read this document before editing core. Inspect applicable AGENTS.md, Git
   status/index/recent log and actual working paths. Do not reset a dirty tree.
2. Read Grok's latest outbox reply and matching inbox/seen request. Do not process
   only a timestamp or ACK. Current ownership comes from that matched exchange.
3. Reconcile any changes made since the dated snapshot in section 5. Grok attempted
   the collector port and reverted that unverified attempt; the concrete type-import
   diagnosis and a passing local-type test are recorded in section 5.3.
4. Tell Grok, in English via the existing inbox, that the new Codex chat continues
   the same work and ownership. Do not interrupt his active build or duplicate it.
5. Resume the already agreed Codex known-character-read slice in section 6.
6. Let Grok finish his collector/Message lane. Review saved evidence read-only and
   advance to the next real gap, rather than waiting for a new assignment each time.

The separate `CODEX_RESTART_AND_WORK_PLAN_20260911.md` documents the 30-minute
heartbeat and FSW migration. It is not necessary to recover the old chat to read
it. Its broad plan is supplemented/superseded by THIS document for detailed core
questions, especially graph traversal and body materialization.

## 3. Working roots, tools and people

All relative implementation paths below are relative to this working repository:

`C:\Nyasha_Planet\lingvamyxa`

| Role | Path |
| --- | --- |
| Active L2 frontend + runtime development | `stg/l1_baseline/l2src` |
| L2 translator currently written in L1 | `stg/l1_baseline/l2src/l2trans.lm1` |
| Root L1 development source | `l1src` |
| Stable L1 source/build root | `stg/l1_baseline/l1src` and `stg/l1_baseline` |
| Shared stable compiler, READ ONLY | `stg/l1_baseline/build/l1trans/gen2/l1trans.exe` |
| Main spec | `Lingvamyxa_spec.txt` |
| Detailed Revision-2 ABI/graph agreement | `struct_refactoring_version_2.txt` |
| L1 spec | `L1_spec.txt` |
| Supporting language documents | `include_languages`, `Lingvamyxa_handoff_notes.txt` |
| Parser oracle | `C:\Nyasha_Planet\lingvamyxa_old_worked_version` |
| Secondary Git recovery source | `C:\Nyasha_Planet\git\gitlab_lmx` |
| Grok channel | `work_chat/grok/inbox`, `outbox`, `seen` |
| Codex isolated verification outputs | `build/codex` |
| Concurrent app consumer | `mixa_manager`, owned by Claude via `work_chat/claude` |

Stable compiler SHA256 verified 2026-09-11:

`65D5A5ED127CA1BAEBDD1D500A5B74CEEA63EC1985EAC52EDEF28EFEB261C936`

Do not confuse similarly named receiver implementations: `l1src/l1trans.lm1`
implements L1; `stg/l1_baseline/l2src/l2trans.lm1` implements the NEW L2 frontend.
The word baseline does not mean its l2src is frozen; its compiler is shared/pinned.
The two build roots have separate outputs and must not write through one another.

Grok owns complex `lmx_message*`, lifecycle, scheduling, mailbox and agreed L2
integration. Codex owns exact agreed storage/helper/compiler slices. Do not start
editing another owner's current files because a reply is slow. Propose a concrete
non-overlapping file set and acceptance checks, then implement that agreed split.

Claude is the app owner and a useful specification/code reviewer, not a dependency
that may block independent core progress. OpenCode and Antigravity monitoring are
disabled by the user; grok_bot is paused. Do not revive those agents on your own.
English to colleagues, Russian to the user.

## 4. What counts as progress and completion

Follow `work_chat/TICKET_RULES_EN.md`:

- Plan/ACK: WORKING, not completion.
- BLOCKED: exact failing step, observed attempts, smallest needed decision/help.
- DONE: actual changes, matching tests/evidence, commit and push, limitations.
- STAGE DONE: checkpoint, hand off the API/evidence, continue the next agreed stage.

A live watcher, received ticket, line of activity or a long plan proves neither
coding progress nor finished semantics. Inspect current source, advancing logs,
commits and exact evidence. Do not re-run all tests to confirm a colleague's reply
when source hashes and a saved successful run already establish the claim.

Commit and push every verified source stage AND documentation update. Use exact
owned paths, inspect the shared index before and after staging, and preserve
others' uncommitted/untracked work. Never reset/stash/clean the shared repository
to make it appear safe. Do not force-push or silently replace the stable compiler.

## 5. Dated implementation snapshot, not a full-conformance claim

At handoff preparation the last committed core change was `18bbf81`; the earlier
root handoff documents were committed at `dfc20bf`. Grok attempted changes to:

- `stg/l1_baseline/l2src/lmx_message.lm1`
- `stg/l1_baseline/l2src/lmx_message_exec.c`
- `stg/l1_baseline/l2src/lmx_message_exec.h`

An intermediate inspection saw an L1 `lmx_msg_arena_collect` function in his working
tree. His subsequent `20260911-205056.txt` reply reports reverting that attempt after
an L1 type-validation error. The sweep driver therefore remains in C at this dated
checkpoint. A later status check found no tracked core modifications. Inspect again
before editing: neither the earlier WIP nor its later absence is a lasting lock.
Do not present the attempted port as accepted. Section 5.3 records concrete help.

### 5.1 Accepted bounded stages

| Commit | Scope |
| --- | --- |
| c557908 | Message-owned root/METHOD construction and emitted-program integration |
| eaac7c5 | Message-owned int/size_t cells, known typed reads/stores |
| 9af0e02 | Closed units without char/P0/query no longer initialize legacy pools/ranges |
| 6aa114a | Grok's mark_from L1 extraction |
| 6fd6a3e | Corrected its C declaration to match the L1 int result |
| 1c3d62b | Codex L1 read-only block-liveness predicates and focused tests |
| 0ce7957 | Collector uses the L1 liveness helpers |
| c186806 | Signature/intern diagnostic evidence emitted as comments, not runtime globals |
| 4a8f87a | Own-name diagnostics also comments; actual field/cache semantics unchanged |
| 18bbf81 | Grok collect_block L1 extraction with narrow native wrapper |

Grok's `work_chat/grok/outbox/20260911-202400.txt` reports Exec PASS for the
collect_block stage and confirms the next Codex char-read ownership. Read its
saved evidence to accept any newer integration; this document is not a rerun.

Relevant successful Codex evidence:

- `build/codex/msg_liveness/20260911_200502_445_54a94a87/evidence.json`:
  17 stages, 97 assertions at O0 and 97 at O2, strict C99/Werror.
- `build/codex/l2_message_root/20260911_202123_402_9ec9fbcb/evidence.json`:
  63 stages, 18 existing modes/649 checks plus program/contract outputs.

The second runner deliberately uses immutable core `eaac7c5` plus five hashed
owned overlays, one private translator build and 13 cached support objects.
It proves that isolated integration slice, not latest-Grok full integration,
stable promotion, a full parser port or L2 self-hosting. Adjacent failed setup
runs must not be cited as successful evidence.

### 5.2 Available building blocks

Read the relevant local notes, not every file on every turn:

- `LMX_MSG_BLOCKS.txt`, `LMX_MSG_STORAGE.txt`, `LMX_OWNED_RANGES.txt`:
  stable blocks, paired block/range admission/transfer, owner-local domains.
- `LMX_MSG_VISIT.txt`, `LMX_MSG_LIVENESS.txt`: traversal bookkeeping and conservative
  liveness queries; no ownership lookup or policy hidden in a small helper.
- `LMX_MSG_SLOTS.txt`, `LMX_MSG_MAIL_CHAIN.txt`, `LMX_MSG_PATH_STORAGE.txt`:
  local bookkeeping modules used by Message.
- `LMX_MSG_CONTEXT_V0.txt`: accepted per-context ownership direction, not an app pin.
- `L2_MESSAGE_ROOT.txt`: generated process-root binding, owned branches/values,
  diagnostics removal and the private focused integration harness.
- `LMX_DEC.txt`: existing approved decimal wrapper over vendored decNumber.
- `RUN_LMX_TESTS.txt` and runner parameter declarations: actual test entry points.

`LMX_MSG_EXEC_HOST_V0.txt` describes a REJECTED shared-table/worker-pool prototype
and retained evidence. It must not become the app architecture. Conversely the
current `lmx_message_exec.c` filename alone does not prove it is that old prototype;
inspect the current contents and ownership, not just the word exec.

Some README/OPEN_POINTS prose describes obsolete global tables and narrower old
frontend subsets. Existing code in a legacy lane is not a new normative rule.

### 5.3 Latest Grok blocker: LmxVisit source declaration visibility

Grok's `work_chat/grok/outbox/20260911-205056.txt`, also quoted by the user, reports:

```text
l1trans l2src/lmx_message.lm1:82:5: unknown type name
L1 cannot declare a stack LmxVisit even with generated lmx_msg_visit.lm1.h included.
mark_from and collect_block are L1; arena_collect sweep remains C.
No owner-link blocker. No quota error. Last accepted core stage: 18bbf81.
```

Do NOT conclude that L1 lacks local struct types or add a heap-allocation workaround
before checking the existing source-header import mechanism. Read-only inspection
found that `lmx_message.lm1` uses `include:` of the GENERATED C header, whereas the
already existing visit module and its selftest use:

```text
predef: "l2src/lmx_msg_visit.h.lm1"
```

That is the SOURCE header, containing `struct: LmxVisit`. L1_spec.txt sections 1.1
and 10 describe how predef imports its declarations for L1 validation and emits the
generated-header include. Merely including a C header does not parse its C typedefs
into the L1 validator. `tests/lmx_msg_visit_selftest.lm1` already declares:

```text
fn: main () int
    LmxVisit: state
    @: LmxVisit s (@ state)
```

Codex ran the existing isolated `run_msg_visit.ps1 -Optimization O2` after the
blocker report. Result: `visit checks=101 failures=0 frees=3 realloc_calls=10`.
Evidence: `build/codex/msg_visit/20260911_210212_774_7815d63d/evidence.json`.
This used the pinned stable compiler read-only, generated private outputs, and
performed no compiler self-build or edits to Grok's sources. It proves that this
local LmxVisit declaration works with the source predef; it does NOT prove the
full arena_collect port or all its header dependencies.

Next in Grok's OWN lane: use the source predef, reconcile duplicate includes and
any type/prototype dependencies, translate the restored collector, then run the
narrow integration evidence at its actual revision. Keep it stack-local. If another
type fails, provide that exact declaration/import chain rather than redesigning
ownership or moving temporary bookkeeping into graph storage. Codex sent these
findings in `work_chat/grok/inbox/20260911-210300.txt`; this is continuation help,
not a competing collector implementation. Check for his later result before acting.

## 6. Immediate Codex task already confirmed by Grok

Exchange: `work_chat/grok/inbox/20260911-202400.txt` and matching outbox, updated
20:25 local on 2026-09-11. Reply SHA256:

`53490D01814C2E864800DD6A534E1F9B1AA5C504C8E2E15FA458B06E62560900`

Implement **known-typed character READ ONLY**:

`lmx_char_value_known(cell)` returns -1 for null and 0..255 for the byte in an
already proven live character cell. No global type classification or owner lookup.
It is not safe validation of an arbitrary stale/wrong-type/foreign address.

Exactly these seven files under `stg/l1_baseline/l2src` are reserved for Codex:

1. `lmx_value_owned.h.lm1`
2. `lmx_value_owned.lm1`
3. `l2trans.lm1`
4. `run_l2trans.ps1`
5. `tests/l2_message_root_driver.lm1`
6. `run_l2_message_root.ps1`
7. `L2_MESSAGE_ROOT.txt`

Grok agreed not to edit them in this slice. Recheck for a NEW ownership change,
but do not wait for another acknowledgment of the existing agreement.

Work sequence:

- Inspect known own/cache and explicit-path char reads; redirect only sites with
  the existing proof of live character type/layout.
- Add the helper/header, unsigned-byte behavior even on signed-char targets,
  and required module linkage.
- Test null and all 256 values in the existing focused driver; preserve relevant
  own-field, path, dirty-publication and generated-program outputs.
- Inspect generated L1/C for removal of the classifier at those reads.
- Keep char allocation, interning, `lmx_char_cell`, `lmx_chars_init`, writes and
  canonical sharing unchanged. Mutating an interned character as if it were an
  ordinary unique int cell would be a new bug.
- Record hashes/evidence; commit/push; send Grok the exact contract and integration.

This does NOT finish Message-local characters/P0/query storage. That is a later
agreed slice with different ownership and domain-policy questions.

## 7. Fixed ontology and representation

### 7.1 One graph model, not generated per-name C classes

The ordinary Structure occurrence uses the common representation:

```c
typedef struct Lmx Lmx;
struct Lmx {
    Lmx *node;
    size_t len;
    void *data;
};
```

This is an explanatory ABI excerpt, not permission to add handwritten C algorithms.
Fields, method occurrences and executable-body hosts are ordinary graph occurrences.
Do not add a fourth owner/Namespace/class/descriptor word to every Lmx.
Classify payloads by the selected address domain; an address already known typed
can use the direct helper without another generic classification.

An ordinary non-leaf owns a sealed contiguous block of Lmx child occurrences.
The child count is fixed at construction. A runtime merge creates a NEW result
with a new child block; it does not grow/reallocate the old block in place.
Ordinary assignment changes a selected binding/payload, not the child count.

Array references address separate typed service descriptors (`len`, `data`), not
an Lmx header and not the element backing pointer. Backing is a contiguous
rectangle, not separately allocated rows. Descriptor-pool capacity, descriptor
length and backing size are different quantities. Reference-valued elements have
their own tracing/copy semantics; primitive elements do not need Lmx headers.

Callable occurrences have a payload identifying an immutable `{addr,sig}` method
record. The occurrence, its containing Structure, the compiled method body and
one C activation are different things. Never collapse them into an assumed
single method-instance object or a uniform one-pointer call ABI.

### 7.2 Everything LMX is inside a Message

OS process startup is ALREADY the root Message. There is no ownerless setup runtime
which later creates the first Message while retaining a global configuration,
method catalog, primitive pool or mutable/immutable range registry outside it.

The first Message receives system/user input and raises its settings as ordinary
graph data. A child constructed from a template receives the chosen settings by
explicit construction/merge copying, not the parent's entire state by default
and not a pointer to a shared settings singleton.

NO LMX data are shared across Messages, even immutable characters, symbols or
method records. Foreign libraries and the OS have their own external contracts;
do not disguise an LMX catalog as a foreign library to evade this rule. Sharing
compiled machine code where the ABI permits it does not license shared mutable
or immutable source-level method-record/state objects across Message arenas.

Interning can canonicalize suitable immutable values WITHIN one Message. It does
not collapse distinct mutable Structure identities merely because values compare
equal. A separate immutable-Message/shared-LMX-data model is deferred optimization,
not a prerequisite or an approved shortcut for this core.

### 7.3 Stable arenas and owner-local metadata

Each Message owns ONE logical arena, consisting of nonmoving blocks. Grow by
adding blocks; never relocate live nodes, child occurrences, descriptor entries
or live payloads. No GC compaction by moving addresses.

Owner-local ranges/type metadata travel with transferred storage. There is no
global mutable range/owner registry protected by a global lock. Lifetime metadata
for the Message endpoint is distinct from graph contents; endpoint/control storage
must survive possible observers until safely retired.

Do not conflate a block's physical capacity with a source arena or type tag.
Per-handler, per-branch, per-attempt and per-transaction source arenas were NOT
introduced. The allocator may use bump/slab/free-list mechanics only while
preserving stable addresses, one logical owner and end-turn reclamation.

## 8. Node, ordered edges and arena ownership are THREE different relations

This is the most important correction to preserve from the deleted chat.

1. **Lexical `node`:** fixed lexical parent of an occurrence, zero at a lexical
   root. Existing node NEVER changes on adoption, composition or inclusion in
   another result. Lexical links are acyclic.
2. **Ordered result children / graph references:** determine composition order,
   explicit paths and reachability. Inclusion in an ordered merge result does
   not make that result the lexical parent of the copied callable.
3. **Message arena ownership:** who may execute/use/reclaim the physical blocks
   and metadata now. One arena can contain MULTIPLE lexical trees after adoption.

After `adopt A -> B`, A's transferred lexical nodes and B's original caller nodes
both live in B's single logical arena. Neither needs a lexical rewrite. User code
chooses which references to retain; adoption itself does not attach the incoming
root as a child of B's graph or make all adopted blocks permanent roots.

Do NOT implement owner lookup by following node to the top. Do NOT add owner-link
to each Structure. Do NOT use dynamic caller visibility as a process-global/TLS
owner lookup mechanism. Binding selection is compile-time/admitted-call work;
native helpers receive explicit local context/range inputs under their agreed
internal ABI. If a remaining allocator needs an unselected way to recover that
context, expose that exact question before adding hidden source-method arguments.

For a selected callable, the method's reserved node argument is exactly
`callable->node`. `node\field` selects a child of THAT supplied node, without
another parent hop and without consulting the caller's same-named variable.
The caller selects the callable, not a replacement value for reserved node.

Existing local `LmxMsg.owner_rt`/endpoint bookkeeping is not automatically a fourth
Lmx field; inspect its purpose. It must not become a shared family registry or
permission for a source method to discover an owner by scanning all Messages.

## 9. Keep graph algorithms distinct

Do not reuse one indiscriminate "walk every pointer" policy for all operations.
The traversal engine may be reused, but its work items/edges/roots mean different
things in these algorithms:

| Operation | What determines the walk | What it must NOT infer |
| --- | --- | --- |
| GC mark | actual live roots and address-domain tracing edges | every arena block or reverse-name entry is a root |
| Merge/template dependency closure | requested output plus used lexical requirements/dependencies | reaching an ancestor means copy every sibling subtree |
| Merge physical copy/fixup | selected source occurrences/payloads and copy contexts | a global one-copy-per-address rule may collapse fresh occurrences |
| Source implements | exposed Consumer uses and exact used-call contracts | execute Consumer to discover the one future branch |
| RuntimeImplements | available current explicit requirements/Structures | unknown coverage is automatically true or a mandatory check everywhere |
| Call binding | OwnUsed/DynRequired, exact sig and caller/fallback order | climb node at runtime for ownership or ambient names |

YES: `implements(varA, varB, Consumer)` is another traversal, explicitly raised by
the user while this handoff was being prepared. Section 17 details its requirement
discovery and paired path comparison; do not omit it from the graph-work inventory.
The table distinguishes semantic jobs, not a requirement to duplicate every basic
iterator or allocate a new visited-set implementation for each job.

Grok's L1 `lmx_msg_mark_from` is a collector helper. It follows owned parent and
children edges and marks known Array/METHOD payloads in its current slice. Merely
porting that function to L1 does NOT implement merge's selective lexical closure,
all payload domains, all roots, or a complete reference-array tracer.

## 10. Runtime merge: FIXED semantic contract

Normative sources: main spec §2.3 and §19.17; ABI agreement §8 and §14.6.

### 10.1 Construction happens at execution time

`merge` is a normal L2/L3 structural operation. P0 parses a normal receiver Frame;
the translator emits generic runtime construction at that site. Only executing
that site performs the merge. Importing a unit or discovering a named declaration
must not eagerly instantiate/flatten it into a per-source-name C struct.

Merge operands can be resolved live references, Structure-producing expressions,
explicit paths or admitted call results, including a result of an earlier merge.
Do not require a literal-only operand or invoke the translator again at runtime.

### 10.2 Ordering and fresh identity

- Evaluate operand expressions ONCE, left to right. Their effects are real and
  are not silently rolled back if later copy work fails.
- Construct one NEW result root.
- Copy each operand ROOT'S DIRECT CHILD SEQUENCE and required subtrees in order;
  do not insert the operand root itself as an additional visible wrapper.
- Append children supplied by the merge's vertical body after operand children.
- Each requested containment occurrence is fresh. Do not mutate, rename, delete,
  reorder or replace a source occurrence.
- Preserve repeated names. Unqualified `name` is exactly `[0]name`, the FIRST
  matching occurrence in forward result order. Later parts do not override it.

For example, merging child sequences `[x=1, a]` and `[x=2, b]` produces ordered
`[x=1, a, x=2, b]`; `[0]x` is1 and `[1]x` is2. This is schematic notation, not
new LMX syntax. Do not implement JavaScript-style last-key-wins object merging.

### 10.3 Preserve original lexical ancestry through COPIES

For each NEW copied occurrence initialize its node once to the corresponding
COPY of its original lexical parent. Do not point it to the composition result
merely because it occurs in that result's ordered sequence. Source node pointers
are never rewritten. Links within the copied lexical environment must identify
the corresponding copies consistently and preserve alias relationships.

A copied callable uses the same admitted machine body, but receives its copied
lexical containing Structure. Its fallback state is that preserved lexical state,
not arbitrary fields on the composition root. Current callers can still override
required hidden values via ordinary dynamic-input selection.

This is not preserving an old C frame or a closure chain. It copies required graph
state, not activations, instruction pointers or the dynamic caller's biography.

### 10.4 What "copy only used ancestors" actually means

The explicit operand output is not pruned simply because one Consumer currently
does not use some output child. Selectivity concerns the ADDITIONAL surrounding
lexical environment required to keep copied code/data meaningful.

If copied code needs ancestor field `a`, retain the necessary lexical path and
the copied `a`/its required dependencies. Do not copy `b`, `c`, all their children
and all cousins merely because a retained ancestor also contains them.

If `a` refers to a large array/graph required by the operation, its size is real;
there is no promise copied environments can never be megabytes. Dependency closure
must preserve values, identity/alias relationships and domain policy, not a target
byte budget invented to make a test pass.

Uses include all relevant possible branches/calls of the copied code, not only
the branch executing when merge runs. Do not execute the code to discover a
smaller set. Transitive references and described Consumer/call requirements count.

Reaching an ancestor through node is NOT, by itself, a request for that ancestor's
complete child tree. This is why GC traversal is not the merge dependency rule.

### 10.5 Unknown paths and interfaces remain legal

An expression selecting an implementation at runtime can be fully described by
an expected call signature even though its final route is unknown. Do not reject
it merely to simplify copying. Likewise, lack of proof a field is unused is not
permission to drop it.

If an explicit whole-Structure use or a computed path exposes more of an ancestor,
conservatively include the potentially required part for that requirement. This
may require more data than a statically narrow path. It does NOT justify always
copying all ancestor descendants for every statically simple merge.

Concrete dependency metadata/encoding is implementation work. If deciding its
coverage would change admitted programs or their observable references, ask the
user with a small example BEFORE implementing that restriction.

## 11. Proposed merge implementation phases (not a new ABI decree)

This is a safe decomposition of the fixed semantics. It does not preselect open
payload copy/return/failure policies or a new per-node representation.

### Phase A — classify roles and requirements

At translation/admission, distinguish:

- requested ordered result occurrences;
- lexical skeleton needed to reach their original bindings;
- own fields required by copied methods;
- transitive references and domain-specific dependencies;
- explicit whole-object or uncertain path requirements;
- independent boundaries;
- activation-only inputs, which are NOT graph state to capture.

Use existing Consumer/call/OwnUsed/DynRequired analysis where meaningful, without
equating these sets blindly. A forwarded hidden argument is not automatically a
field to copy from an ancestor. An explicit graph reference remains graph data.

### Phase B — evaluate and retain source operands

Evaluate each source expression once in order, root results while construction
needs them, and obtain their live domain/structure information. Ordinary
same-Message serial execution avoids unrelated concurrent mutation, but calls
performed during operand evaluation still have their usual effects/checkpoints.
Do not cache an earlier selection then pretend later code was re-evaluated.

### Phase C — discover a finite required closure

Use a work list and a visited/requirement structure in the executing Message or
private operation storage with explicit lifetime. Graph cycles terminate by visited
identity; lexical links must remain acyclic. No process-global copy table.

Important distinctions:

- A visited entry for DISCOVERY avoids infinite traversal; it is not permission
  to collapse two requested output occurrences of the same source operand.
- A requirement may strengthen from one field to a wider exposed part of a
  Structure. Revisit/merge requirements as needed; do not stop at the first weak use.
- The metadata must distinguish original occurrence identity from a particular
  copied occurrence/context, where the same input contributes more than once.
- Keep path/occurrence selection valid if pruning surrounding fields. Compacting
  an ancestor's retained fields must not silently change a compiled child index
  or `[n]name` selection. The representation/remapping policy must be explicit.

### Phase D — allocate the new shape without relocating sources

Once required counts are known, allocate final-sized child blocks, new roots and
required lexical skeletons with checked arithmetic. Use Message-owned typed ranges
and existing nonmoving allocation helpers. Preserve the distinction between the
result's ordered children and original lexical-parent identities.

If using a temporary source-to-copy map, it is operation-local algorithm state,
not a persistent owner/Namespace field or a second general per-node metadata table.
Do not introduce a shared "object database" for this purpose.

### Phase E — initialize lexical links and payload/reference fixups

Initialize each new node to the right copied lexical parent. Resolve internal
references to their corresponding copies consistently. Do not mutate source
pointers in order to discover or install the mapping.

Apply explicit payload-domain copy policies:

- ordinary mutable cells/graphs must satisfy the selected copy contract;
- immutable atoms may share only where the WITHIN-Message domain contract permits;
- cross-Message copies may NOT leave LMX pointers into the source arena, even
  immutable ones;
- Array descriptors and backing/reference cells are separate objects;
- method records must have admitted exact signatures and local ownership;
- opaque handles require an explicit external contract, not byte-copy guessing.

The policy for every offered payload domain must be specified before accepting
it. Unknown foreign handles are not automatically deep-copyable, transferable or
safe to share. Report a concrete unsupported domain or ask for policy; do not
silently treat it as a source graph child.

### Phase F — register names, root and publish the completed result

Maintain the auxiliary occurrence-to-ShortNameId service for runtime textual
operations, with the chosen anonymous/positional ABI convention. Preserve name
identity and repeated occurrence order. This registration does not create a
runtime Namespace or replace ordinary direct child addressing.

Only expose a result when required links, domain metadata and lexical dependencies
are consistent. Temporary roots must protect allocations during construction and
transfer to the actual result root. Release operation bookkeeping under the chosen
failure/result policy. Do not imply a general rollback transaction for user code.

### Phase G — verify failure and repeatability

Test failure before/after each relevant allocation/admission group, consistent
cleanup/retention, sources unchanged, no half-built published graph and a correct
retry. The precise API result for failure remains an explicit choice; preserving
source identity and preventing invalid publication do not.

## 12. Merge and selective-copy acceptance scenarios

These are semantic scenarios; translate them to the supported LMX test syntax.
Do not label schematic notation as a fixture that already passed.

1. Two operands with repeated names: exact child order and `[0]`/`[1]` values;
   inputs unchanged and result occurrences fresh.
2. Merge of a previous merge result and a call-returned Structure; operand calls
   execute once, in order, only when the site executes.
3. Vertical body children appear after operand children, not as a separate wrapper.
4. A copied method uses a field from its original containing Structure. Merging
   it under a result with a conflicting same-name field must not change fallback
   to that result's field. A current dynamic caller can override the hidden input
   only through the already defined call rule.
5. Ancestor has `a`, unused `b` and unused huge `c`; copied code needs only `a`.
   Preserve needed lexical path/a dependencies; do not copy b/c just for ancestry.
6. Another possible branch needs b: b must survive even if the merge runs on the
   branch that currently reads only a. An execution trace is insufficient analysis.
7. Needed a explicitly references c: preserve that dependency according to its
   domain policy. The previous pruning example must not become an invalid pointer.
8. Two internal links alias the same needed graph object: the copied environment
   preserves required aliasing. A cyclic graph terminates discovery/fixup correctly.
9. The same operand used twice contributes distinct ordered occurrences. Do not
   accidentally deduplicate result nodes using a global source-address map.
10. Pruned lexical environment still selects the original intended child/path;
    fields are not shifted into another compiled selector by accident.
11. Runtime-selected callable with an admitted sig and described hidden inputs
    remains valid; an unresolved route alone is not a reason to reject it.
12. An explicit whole-environment consumer/computed path retains the broader state
    it may require; no unsafe optimistic pruning.
13. Template copy into another Message contains no implicit source-arena pointers.
    Later source mutation/destruction does not invalidate the accepted copy.
14. Runtime merge in the same Message still follows identity/domain policies;
    do not impose cross-Message cloning rules on every ordinary reference call.
15. Every source node byte remains unchanged. New lexical links target copies,
    not the composition root by default. Adopt of the finished result later
    preserves all of its native addresses and node links.
16. OOM/admission failure leaves no invalid published result, no double-free and
    no corruption of operand roots; later attempt works under the selected policy.

Do not run 16 full compiler bootstraps for these cases. Reuse one compatible
translator/runtime build, and use focused assertions on graph identity/order/
values and ownership. Source compilation alone is not acceptance.

## 13. independent: exact boundary and tests

Main spec §9.1.3 is normative. independent qualifies CONSTRUCTION of a Structure.
The new qualified root has node=0; surrounding lexical environment is not imported
merely because it was written there. It does not reset node on existing objects.

Its own fields and internal lexical relationships remain ordinary. If independent
S contains method f and own field n, f's node is S, S's node is0, and f can use n.
Do not clear every descendant's node and destroy internal lexical containment.

independent DOES NOT:

- prohibit caller-supplied hidden through-values;
- require all callees or expression routes to be statically known;
- restrict code to explicit formals only;
- make a method pure, freeze state, or create immutable sharing;
- shrink an explicitly supplied large Array or clone ordinary reference arguments;
- waive Message isolation or allow a borrowed reference into a foreign arena.

The interface is described by sig/Consumer requirements, not by a known route to
one implementation. An admitted callable can be chosen dynamically. Inputs are
supplied by the usual caller-first rule; missing required input fails normal
admission, not a new independent-specific error category.

Minimum tests:

- own n/internal f remains valid with root node0;
- caller supplies hidden x required by sig: call succeeds, source x unchanged on
  local rebinding, correct explicit own-bind publication if performed;
- x exists ONLY outside the independent root and not in caller/internal fallback:
  no access via an accidental outer lexical chain;
- runtime-selected callable with exact sig succeeds; wrong sig fails before the
  typed call; unavailable dynamic input fails without a guessed value;
- merge of independent S does not pull its former textual environment;
- internal and explicit reference dependencies still copy/retain as their real
  contracts require, including large data and cycles;
- no special P0 syntax rewrite or L1 semantic restriction is introduced merely
  to implement this L2/L3 qualifier.

## 14. Method analysis, dynamic visibility and exact invocation

### 14.1 Binding precedence is fixed

For a required free bare x at a call:

1. nearest current caller activation-local binding (own cache, declared formal/
   result or other receiver-defined local);
2. otherwise caller's dynamically inherited x;
3. otherwise direct `node\[0]x` of the SELECTED CALLEE'S supplied node;
4. otherwise incompatible call: reject statically when known or the specified
   runtime Type admission failure.

No runtime walk up `node->node` ancestors for fallback. No name-stack or hidden
closure-chain object. Explicit `node\x` is the structural field regardless of
dynamic caller x. Reserved node cannot be rebound or supplied as an ordinary name.

### 14.2 OwnUsed and DynRequired need a fixed point

OwnUsed includes graph-backed names read/written bare OR needed to supply a
callee's hidden input. DynRequired includes required free bare names and transitive
callee requirements not satisfied by own bindings. Explicit paths do not become
hidden parameters merely because their text contains a field name.

Propagate needs through direct/mutually recursive call graphs to a fixed point.
A method that never reads x itself but calls another method requiring x must still
retain/forward it. At a runtime-selected call, the expected sig describes the needs.
Do not infer a hidden field solely because a sibling Structure happens to have
the name or because a literal looks like a particular primitive type.

### 14.3 Exact sig, not same arity

Canonical signature identity covers ABI version/convention, reserved node,
declared names/positions/modes/types, ordered hidden names/types/pass modes,
result convention, declared throws and varargs shape/prohibition. Same arity is
not compatibility. Translation-unit-local intern numbers cannot be compared as
if they were one canonical linked identity.

The concrete record encoding is an ABI choice; exact equality and typed invocation
are fixed. Do not claim ISO C guarantees arbitrary void*-to-function-pointer casts;
use the selected documented backend ABI or a typed adapter representation.

### 14.4 Ordered lowering

1. Evaluate selected callable once and root it.
2. Validate expected exact sig.
3. Evaluate declared actuals once in their specified order into typed temporaries.
4. Select hidden values in sig order, caller-first/fallback, into typed temporaries.
5. Publish the caller's dirty own fields.
6. Invoke the exact typed entry with `callable->node`, declared and hidden values.
7. Continue the caller without reloading its working locals.

Nested calls during argument/callee evaluation use their own checkpoint rules.
Do not rely on C argument-expression evaluation order. A checkpoint cannot
retroactively change the previously selected callable or fallback actual.

Calls to const-qualified established callable paths may become direct C calls
after proof, but still evaluate actuals/hidden inputs correctly and checkpoint.
Do not replace current construction-time addresses with process-global static
LMX objects under the name of const optimization.

## 15. Executable-body hosting and floating arguments

Main spec §§21.5–21.5.1 and §11.3.1 settle the recent user correction:

**Code consumed as a line-by-line executable body is graph state. Arguments of
ANY receiver are not graph fields merely because they are arguments.**

This is a semantic consumption-role rule, not an if/for/trailer whitelist and not
"the last parsed argument is a Structure, therefore it is a body".

- Each executable if/else/for/while/nested body hosts its directly declared fields.
  Host the individual body, not a synthetic state field for the entire control
  statement. Nested hosts form their true hierarchy, not a flat list.
- Conditions, header arguments, Structure-valued parameters and return/trailer
  arguments are values. They may REFER to existing graph state or evaluate a call
  whose executable body is graph state; the argument occurrence itself is not
  automatically a persistent own field.
- A nested return expression such as `return:` followed by indented `5 + 5` is an
  argument expression, not automatically a new executable-body host.
- Ordinary nested bodies do not create C method activations or through-parameter
  boundaries. Through applies to method nesting/calls, not ordinary Structures.
- Inward use of an existing enclosing field updates that binding. Do not invent
  a shadow field for every assignment in a nested block. Child fields are visible
  upward only via explicit reference/path, not flattened into the parent's names.

### 15.1 Materialization vs execution vs binding: keep all three separate

Current §21.5 requires a normal builder to materialize declared own backing
occurrences, including nested executable-body hosts, BEFORE the containing callable
can be invoked. A missing required own field is admission failure, not an excuse
to reinterpret it as a dynamic input.

Graph presence does not execute an untaken branch or dirty its fields. The source
initialization/construction rule and subsequent executed writes must remain distinct.
The same-name argument bind line controls when that parameter becomes a cache;
it does NOT delay creating the graph field until that line is executed.

Do not use the earlier broad handoff's warning about an unmaterialized branch
as permission for graph hosts to be absent until a branch runs. Resolve/materialize
the required host graph consistently. Any physical laziness optimization must
preserve the observable graph required by the current spec; it is not a new rule.

### 15.2 Three-pass consistency work

Claude investigated this in `mixa_manager/SPEC_DIGEST.txt` §6.1. That file is a
repository source of analysis, not something recoverable only from his chat.
It identifies discovery/layout, validation and emission inconsistencies, special
C-style-for hosting, aliases loaded outside the prologue and for-inside-if gaps.

Unify the semantic role analysis so all passes see the same executable hosts and
fields. Both admitted for spellings need the same rule. Include nested fn/sub
discovery, not just top-level methods. Check actual implementation before declaring
each old observation still unfixed.

Tests: if/else hosts; both for forms; for in if; nested method vs nested body;
repeated calls; explicit body-field path such as `for\j`; missing required host;
untaken branch not dirty; receiver/trailer arguments not accidental graph fields.

## 16. Arguments, same-name own binding, cache and address semantics

### 16.1 Ordinary inputs remain ordinary activation values

Declared and hidden primitive arguments are ordinary typed C parameters. No
second hidden argument copy/boxing/frame heapification. Structure/Array arguments
copy a reference value, not the referent. Rebinding a local input does not rebind
the caller; explicit reference/index writes may mutate the same referent WITHIN
the same Message. No cross-Message graph alias follows from parameter passing.

### 16.2 Assignment can be declaration and an own-field bind

An incoming formal/hidden parameter with the same name as this method's own field
uses ONE C working variable. Before its own-field bind line it is activation-only.
From an EXECUTED typed declaration or assignment-as-declaration bind, that same
variable is the cache of the already materialized own field. Do not load the old
graph field over the incoming parameter. Later actual writes dirty it; publication
goes to this own field, never the caller/lexical fallback source.

An own-only field without such an input still loads normally at entry and does
not become caller-overridable. A hidden input is created for an actual free need
(including a use before binding), not just because a field with that name exists.
Apply the same rule to explicit, caller-supplied hidden and fallback hidden inputs.

### 16.3 Dirty-only, used-only and no reload

- Load only OwnUsed fields into typed working locals; keep exact backing
  occurrences rooted. Explicit paths are direct graph accesses, not hidden caches.
- Dirty records an executed assignment/update, including assignment of an equal
  value. It is not value-comparison with the graph or "this branch may assign".
- On each semantic external/callback/publishing call boundary and activation exit,
  publish dirty values in defined field order and clear each after successful
  store. Clean values MUST NOT overwrite a callee's explicit graph changes.
- Evaluate actuals before the outbound checkpoint. No reload after callee return.
- Private nonescaping bookkeeping helpers are not automatically semantic boundaries
  merely because the C compiler outlines them; uncertain external calls are.
- Return/throw/finally ordering must follow the spec: evaluate/root result once,
  checkpoint pre-cleanup, execute applicable cleanups, checkpoint cleanup writes,
  transfer result and destroy activation. No premature result unrooting.

Published state is the last state actually checkpointed by participating
activations, not a hidden per-call snapshot or simply a single recoverable caller
history. Recursive invocations share graph state while their C working values
remain separate. Do not restore the withdrawn "bad style/no portable value
guarantee" restriction as an excuse to avoid implementing dirty-only correctly.

### 16.4 Address distinction

For an own primitive graph field, @ resolves the typed payload, not `Lmx.data`
as a void-pointer slot and not the hidden C cache. A write through that address
changes graph payload, not the cache's dirty flag. Taking an address is not a
write and does not extend lifetime. Raw prefix dereference on a typed local
address uses ordinary L1/C dereference; no graph lookup is inserted.

For an UNBOUND declared/hidden parameter, @ is the C address of that parameter.
It dies with the activation; returning it does not create persistent backing.
A body-declared own field can return its graph-payload address if the referent
is kept alive by the actual lifetime contract. Nonmoving is not immortal.

Required cache example, using explicit state notation rather than claiming all
spellings are currently compiled:

```text
own working x = 5, clean; graph x = 5
p = @x              addresses graph payload
write through p = 9 graph x is now 9; cache remains 5 and clean
y = x               y becomes 5
z = dereference p   z becomes 9
exit                does not republish clean x; graph x remains 9
```

If the activation later actually assigns its working x, it becomes dirty and may
publish that newer working value. This is intentional, not a coherent-memory cache.

### 16.5 One address case remains explicitly OPEN

Main spec §11.3.1 still does not choose @ for a parameter AFTER a same-name own
bind, when the parameter itself is the working cache. Before bind its address
rule is C-local; ordinary own-field @ must not expose a cache. The spec names
the collision and says not to silently pick a result/lifetime policy.

Do not confuse this with the SETTLED same-name binding/publication rule. Before
lowering post-bind @, ask the user with the small example already in the spec
(address before bind, own declaration/bind, address after bind). Preserve any
already acquired address's actual referent; do not secretly retarget raw pointers.
This open corner must not block independent Message/merge work that does not use it.

## 17. implements, expression interfaces and names

### 17.1 A separate Consumer-guided compatibility walk

`implements(varA, varB, Consumer)` is a partial directional check of the paths and
operations Consumer uses. `RuntimeImplements` works over available current explicit
Structures/requirements. Neither runs Consumer to predict future branches.
Neither is a shallow field-name-presence test or "every possible graph reachable
anywhere must be structurally identical".

The spec spells the source call `implements(varA varB Consumer)`; commas here are
explanatory separators, not a proposed syntax change. There is no two-argument
form. Without Consumer there is no consumer-relative compatibility question.

Conceptually, the source predicate has two jobs, which may be fused or share already
computed requirements where correct:

1. Inspect the known Consumer code/data/P0 graph or its explicit known used-path
   list. Gather visible field-follow and consumption sites referring to varB,
   including alternative branches. Do not execute Consumer, expand arbitrary
   callees, evaluate computed names, or invent whole-heap alias/dataflow knowledge.
2. Follow the exposed required paths in varA against their role in varB. Require
   every covered path and admitted primitive leaf, and compare exact canonical
   signatures at callable leaves actually INVOKED by Consumer. Report known
   mismatches; preserve an explicit boundary around unresolved coverage.

For example, if Consumer reads `varB\x` and invokes `varB\worker`, varA must supply
those covered paths with their required leaf/call contracts. An unused enormous
`varB\archive` subtree does not become a compatibility requirement. A visible use
of `varB\y` in an alternative branch still belongs to the inspected requirements,
even when a particular execution would not enter that branch. Empty uses is true.

Direction matters: implements(A B Consumer) does not imply implements(B A Consumer)
or implements(A B AnotherConsumer). It does not compare unused values, field order,
duplicate count, unused nested contents, ownership, effects, mutability or C layout.
Repeated names obey the normal occurrence-selection rule; the predicate does not
invent a multiplicity requirement. It is NOT a complete mutation/ownership safety
validator merely because the compatibility check returned true.

RuntimeImplements obtains requirements from the CURRENT AVAILABLE Consumer graph
instead of unavailable native/source bodies. Success covers only inspected
requirements, not hidden behavior or all future execution. Message validation is
the first intended runtime application in the current spec. A known missing path
or incompatible invoked signature fails; an unknown implementation behind a known
signature remains legal and is checked at actual call admission as specified.

Shared traversal primitives are welcome, but keep separate policies: GC follows
live retention edges; merge chooses/copies lexical dependency closure; implements
compares a Consumer's covered requirements without copying or modifying operands.
In particular, GC visiting an ancestor does not license full ancestor copying,
and compatibility discovering a used path does not itself perform merge or GC.

Used callable invocations require exact sig. Opaque transport does not validate a
future call it is not performing. Dynamic inputs still must be supplied at actual
call admission. Both predicates leave data untouched and do not run converters;
consumption selects declared conversion receivers by the explicit merge key.

Do not silently install RuntimeImplements at every return/replacement/call. Its
additional sites/coverage need explicit specification. An unknown implementation
behind an interface is normal; independent does not forbid it.

Focused acceptance should cover: unused-field omission and empty uses; used-field
absence; alternative-branch uses; nested paths; directional substitution; exact
invoked signature mismatch versus an unused callable; dynamic/unknown route with
an expected signature and later actual admission; no operand mutation/converter
execution/Consumer execution; and explicit runtime coverage limitations. Reuse
already passing cases rather than adding duplicate mandatory full-build suites.

### 17.2 Auxiliary names and explicit evidence are not a global Namespace

Ordinary runtime LMX access is NAME-FREE: known child positions/direct addresses
or an explicitly supplied path referent. The occurrence -> ShortNameId reverse
index is auxiliary for runtime TEXTUAL name arguments and related diagnostics.
It is not a second forward Namespace and not an unconditional lookup at every
field access. Its canonical name identity must agree across linked modules;
unresolved encoding/collision/anonymous-name ABI choices must be recorded.

Registry/schema/evidence data are explicit Structures supplied to consumers.
Ordinary construction does not magically register a class/name. Receivers with
explicit data-materialization rules may build such tables; that is not a hidden
type system attached to every value. A resolver continuation key and a materialized
semantic descriptor must not be conflated solely because both are stored as keys.

## 18. Message scheduling and concurrent mutation

### 18.1 Local management, not a hidden global manager

Each live Message has its own FIFO mailbox, parent capability (root excepted),
list of DIRECT children, lifecycle/timer state and scheduling policy for those
children. The same implementation code can be reused with per-Message state.
No process-wide mutable Message table or global management lock.

Each child can choose another policy/mapping for its own children. A child run
on the parent's physical thread can still launch physical threads for its own
children. No mandatory OS thread per Message and no mandatory global worker pool.
An optional routing/sorting/coordinator service is a Message whose internal state
is only used by its own turn, not shared data behind a cosmetic Message facade.

One Message executes at most one turn at once and consumes according to its FIFO
contract. Different Messages can execute concurrently. A parent choosing serial
child execution gives each chosen child its WHOLE turn. No arbitrary mid-turn
switch/resume is required for this first native implementation. Ordering/fairness/
batch size are replaceable policies, not new language-wide rules.

Do not introduce a depth16 address/tree cap or eight-argument call limit. Grow
source-driven structures with checked resource failure rather than undocumented
semantic caps. Physical resource exhaustion is real; guessed hard limits are not
a replacement for the requested behavior.

### 18.2 Single owner M0, concurrent requesters Mn

To mutate M0, M1..Mn send the COMPLETE operation. M0 executes read-modify-write
serially inside its own turn. No requester locks M0 then directly writes its
arena. No extra parent permission/reservation is needed just because arrivals
are concurrent.

Admission serializes the actual arrival order into M0's FIFO. There is no required
random shuffle, timestamp sort or predetermined race winner. Order-sensitive
operations observe that admitted order. If another order is needed, explicitly
wrap them in a sorting/batching Message with a stated completion/late-input policy.

Separate read and later write requests are not one atomic read-modify-write;
serial turns do not repair a split protocol's lost update automatically. Likewise
FIFO is not external-effect rollback or durable exactly-once execution.

### 18.3 No lost wake and no application synchronization

The sending OS thread is executing some Message/foreign callback. Mailbox enqueue
and sleep/wake registration must form a correct handshake so a consumer cannot
sleep after missing a just-admitted item. A per-mailbox synchronized collection
and wait/wake primitive is the allowed low-level substrate, not an application
lock or a global scheduler. Test the race as a mailbox property.

No native locks/atomics/private worker frameworks in Mix, build logic or bundled
app algorithms. L2 exposes low-level synchronization as a language capability,
but the standard package uses it only inside Message and necessary foreign ingress.

## 19. Message construction, address, delivery and reliability

### 19.1 Create is not send; delivery is not launch

- Template/create constructs copied initial state in a distinct inactive child's
  arena. Child activation is committed by the creator's successful publication;
  do not expose an uncommitted reservation as a running child.
- Normal incoming LMX text/data are validated/executed by the recipient in its
  own serial context. Receiving does not automatically launch a separate thread.
- Explicit launch makes a child with its own assignment/arena.
- Running Messages do not change supervisor by being sent/adopted. Moving work
  means new delivered data/assignments, not transparent migration of live frames.

### 19.2 Hierarchical placement address

The selected bootstrap address is parent's path plus the next child counter
segment: parent34.3 gives34.3.1, then34.3.2; first child of34.3.1 is34.3.1.1.
It is concatenation, not arithmetic addition of whole paths and not a global
slot lookup. First allocated child is1; counter/retry behavior follows the
specified reservation/create-id contract. Do not reuse a released reservation
as if its handle were still live.

A Message capability is not a guessable integer granting access, and a Mix path
is not a raw pointer/OS worker id. Arbitrary recipients can be explicitly held
capabilities; parent/child are supervision relations, not the only send routes.
No fixed16segment source limit. Externally allocated worldwide addresses are a
future addressing/provider layer, not a reason to block current child counters.

### 19.3 Envelope prototype vs final arena delivery

The existing number/byte mailbox prototype stages/copies envelopes and materializes
recv data. Those copies are measured implementation status, NOT a new ontology
that every local delivery must deep-copy all graph state several times.

Ordinary local arena delivery transfers independently transferable, nonexecuting,
handoff-safe storage into the recipient's ONE logical arena, preserving pointers.
A running sender cannot retain simultaneous mutable aliases into transferred
storage. Ownership and range/name metadata move together. Received nodes are not
reparented or implicitly rooted forever.

A delivery-only Message can have no physical thread. Its executor marks completion
when that delivery is completed. Later downstream work is a different assignment.
After completion and removal from active tracking, the delivered Message has no
active supervising parent; the recipient owns its storage, not a reparented live
worker. Historical origin/path data may remain without a live supervision edge.

### 19.4 Publication and dedup phases

Staging status != recipient admission != application execution != remote delivery.
Successful end_turn publishes staged outgoing FIFO work; failed turn discards
uncommitted outbox, without undoing previously performed graph/external effects.
yield is not automatically end_turn/publication.

The existing in-memory envelope dedup scope is recipient+sender+logical request id.
Two senders' id1 must not collide. A duplicate admitted/executing request is not
new work; completed result reuse is possible only under the declared retention
policy, otherwise report late/rejected rather than re-execute silently.

Outgoing work describes timeout/expiry/attempt budget/backoff and what to do next,
including WHICH phase is timed and its start event. Do not time an unpublished
request as if the recipient had already accepted it. Queue pressure is explicit
failure/backpressure, not fabricated success. Do not silently forget dedup entries
to squeeze under a fixed-size historical table.

Memory-only identity/outbox does not survive process restart. Durable dedup and
idempotent external effects require explicit durable storage/protocol; they are
not guaranteed by FIFO or by a successful local control flag.

## 20. Liveness, running/success, stop and emergency escape

### 20.1 Supervision is local and observation symmetric

The default automatic liveness query is CHILD -> PARENT, at end_turn opportunities
or a configured local cadence. Every running Message has local self-maintenance,
even with no children. Parent observes expected child communication/control state
and per-child deadlines. Both sides can react to prolonged silence without
introducing a second mandatory parent->child polling stream.

Ordinary explicit messages may of course go either way. A live idle parent is
not dead merely because it has no user work. Native wake/bounded waits must allow
idle participants to observe stop/timer requests. A global all-context scan is
not the owner of these lifecycle decisions.

### 20.2 Meaning of control flags

Initially running=1, success=0, even if there is no executing OS thread.
running is permission to continue for this incarnation; setting it back to1 is
not an allowed resurrection of a stopped Message. Only actual completion sets
success=1; then running=0. Cancellation/timeouts do not manufacture success.

The parent checks direct children's control state on its own turns. Local
completion-only reverse messages are unnecessary when the defined flags suffice;
real result/progress data still travel through ordinary Message ownership rules.
Do not read a foreign graph merely because success was observed.

Deliver(task) completes on delivery; CopyFile(task) completes on completed copy.
An alive response does not complete a task. All delivery children reporting
success is not automatically proof all downstream effects completed.

### 20.3 Ordinary stop vs emergency cancellation

Ordinary stop requests close; the target finishes its current turn and exits its
loop at the specified boundary, requests its direct children close and performs
the normal publication/cleanup contract. Partial file/network effects are not
rolled back; logs/checkpoints may preserve progress explicitly.

Emergency cancellation can set running=0 before ordinary end_turn. Generated
code polls on method entry BEFORE loading used own fields, on loop backedges and
on exits. Observing0 escapes to the execution root of THAT L3 Thread, not an
unconditional OS-thread kill which would also kill parent/siblings using it.

The root requests running=0 for its DIRECT children, settles activations/native
resources/children and then reclaims or hands off its arena. It needs no
acknowledgment from itself; another observer still needs actual quiescence before
freeing memory. Polls must not escape while holding a native/mailbox lock or
unfinished protected operation. Discarded activations are not resumable snapshots.

### 20.4 Exact cheap-flag condition selected by the user

The fast path is an ordinary load/test of uint_fast8_t. Relaxed atomic load/store
is allowed ONLY if the actual target/compiler/options produce the SAME executable
instructions as the corresponding volatile load/store, for running AND success.
No extra lock, RMW, fence or helper call for these flag accesses. Check generated
code; do not merely assert x86 makes everything free.

If equivalence fails, report the unsupported fast path and ask the concrete
profile question. Do not silently weaken C threading correctness by claiming
plain volatile is a portable synchronization guarantee, or silently add cost the
user rejected. No immediate visibility/fixed cancellation-latency guarantee exists.

### 20.5 Termination, quiescence and control retirement are distinct

running=0, including together with success=0, is NOT final failure/quiescence.
Two independent loads are not an indivisible final-outcome snapshot. A runtime
handoff boundary establishes stopped execution, no remaining native users,
completed required cleanup, settled direct children and final outcome.

Only that boundary permits arena transfer/reclamation. The control block can
outlive its graph while observers/stop writers remain. No third SOURCE flag or
extra fence on every ordinary poll is implied; exact endpoint machinery remains
Message implementation work.

A sleeping worker needs wake/bounded wait. An uninstrumented native hang cannot
be interrupted by a poll it never reaches. If parent and child occupy the same
blocked physical thread, the parent's local timer cannot magically run either.
Do not claim arbitrary native hangs can be safely reclaimed by setting one byte.

## 21. Failure-arena retention and zero-copy adoption

Main spec §19.29.8 contains the selected policy; do not reopen it as an orphan
supervisor/global crash-manager design.

1. On final SUCCESS reclaim own execution history by default; retaining it needs
   explicit result/program roots or a specified transfer.
2. On final NON-SUCCESS, once handoff-safe, transfer the logical arena without
   copying to the DIRECT parent's available lifecycle endpoint.
3. Parent adopts blocks AND ranges/name metadata and retains a failure root at
   its own serialized end_turn/lifecycle boundary. Do not leave an unrooted
   interval between attaching storage and protecting its failed root.
4. Closing Message settles direct children first: successful histories can go;
   failed child arenas join it before its own failed arena travels upward.
5. P -> C -> G, both C and G failed: G joins C, then C joins P. No ancestor scans
   grandchildren, no pointer relocation, graph clone or implicit live reparent.
6. A stopped parent may still have a valid settling endpoint. running=0 does not
   mean that endpoint vanished or that child memory can be freed immediately.
7. If the parent endpoint is unavailable, retain the handoff-safe failed arena
   under a SEPARATE parent-loss/orphan-retention timeout, then self-reclaim when
   permitted. This is not the parent's operation timeout for a still-live child.
8. A native user that has not quiesced keeps the branch unreclaimed. Timeout is
   not permission to free memory still in use.

Required external-resource cleanup precedes handoff. A failure graph is available
published state, not live sockets/devices or a complete record of discarded C
locals. A new Message can use retained data for a new attempt; it is not resuming
the old activation or rolling back completed external effects.

If parent P collected failed child history and later P itself SUCCEEDS, P's parent
decides disposal of P's settled arena including that history. Do NOT automatically
forward the original failed descendants forever. P may explicitly preserve them
as data when that is its intent. If P itself fails, the aggregate follows the
normal upward failed-arena rule. The user explicitly confirmed this case.

## 22. GC and end_turn: what must still be proven

### 22.1 Exact collection boundary

One local collection pass at end_turn:

```text
determine turn outcome
  -> publish successful staged outbox OR discard failed staged outbox
  -> release completed inbox/turn roots
  -> collect THIS Message's arena
```

Method return/break/continue/retry/redo/caught throw are not extra GC points.
Uncaught turn failure reaches the same boundary. No moving compaction during
allocation; pressure can affect failure/policy or the next boundary, not violate
source lifetime. Final destruction releases the remaining owned arena safely.

### 22.2 Real roots and typed edges

Roots include Message graph, active node arguments, exact own occurrences kept
by activations, declared/hidden Structure/Array references, results, continuations,
scheduler/explicit handles and retained failed-history roots as applicable.
Any native/continuation reference surviving end_turn must be registered/rooted
until resume/close/cancel. A reverse-name index is NOT a root.

Trace according to domain: child blocks, required lexical parents, explicit graph
references, Array descriptor->backing and reference-valued elements, method/value
payload dependencies and explicit roots. Do not dereference arbitrary raw values
as Lmx pointers. Add coverage per domain before claiming a full collector.

Block-level sweeping may conservatively retain unused objects in a mixed live
block, but that implementation detail cannot justify a permanently rooted whole
adopted arena. Expose collector granularity; do not confuse within-block slack
with the user-visible guarantee for an otherwise dead transferred block.

### 22.3 Critical adopt-without-retain regression

Transfer an otherwise handoff-safe incoming graph to B, do NOT retain it in user
graph state, release delivery/turn temporary roots, then execute B.end_turn.

- Its otherwise empty/dead transferred block is freed exactly ONCE at that
  boundary, before B destruction.
- Dead range and reverse-name entries disappear before storage is reused.
- A second end_turn does not free again.
- The paired retained-root case keeps addresses/node/value alive until that root
  is released.
- Explicitly calling a disposer in the test is NOT proof end_turn collected it.

This was an explicit user acceptance requirement and must not be lost.

### 22.4 Failure safety and current extraction work

The L1 mark/collect_block extraction preserves order and visit-OOM behavior.
Resume the arena_collect port after the source-predef correction described in
section 5.3; its attempted version was reverted, not completed. Do not conflate
source-language extraction with full graph-domain correctness.

On incomplete mark due to OOM, conservatively skip unsafe sweep. Snapshot the
next range before removing an embedded record; never read it after its block was
freed. Remove dead domain/name metadata coherently. Paired range/block admission
or move failures must preserve both old owners and allow a later successful retry.

Evidence should cover null/empty graph, cycles/aliases, missing/failed mark,
registered and opaque domains, adopted unrooted/retained storage, reference arrays,
active/result roots, repeated collection and one-time disposal. Add only missing
cases rather than re-creating every already accepted helper matrix.

## 23. Remote representatives, World Wide Mix and rights

WorldWideMix.txt and the existing map implementation are context for the intended
tree of Message participants. They do not authorize importing its old Java
synchronization into the new core.

Each nonlocal participant is represented by a LOCAL Message which owns its
transport, timers and local control flags. HTTP/REST or another selected protocol
updates that representative; there is no cross-machine shared uint8 or global
remote scheduler. Transport silence is not proof of death/success/quiescence.

Remote transport sends bytes under an explicit codec, not physical RAM. Receiver
materializes owned local state; this is distinct from local zero-copy arena
handoff. Exporting a full remote failure graph is an explicit diagnostic/transport
contract, not automatic adoption across machines.

For multi-owner exclusion, the chosen managing PREFIX Message coordinates its
subtree via ordinary Messages. Each affected owner still mutates only its own
graph. No parent directly locks/writes a foreign arena or schedules grandchildren.
A reservation request stored at the prefix is not automatically enforced at all
owners; any selected protocol must settle in-flight work, enforce grants on all
conflicting routes and reject stale/revoked work. A grant/token API was discussed
as a POSSIBLE protocol, not a mandatory prerequisite for local Message runtime.

Synchronized execution/effective timestamps require explicit clock uncertainty,
late/missing request and partial-failure policies. They do not create simultaneous
atomic writes on two machines or automatically roll back external effects. Do not
add distributed transactions to ordinary same-owner FIFO updates.

Parent-child liveness/stop rights do not grant content-access rights. Authority
comes from the explicit verifier/capability/evidence policy in §19.32. libsodium
was mentioned as a possible native crypto provider, not as the L3 API or the
parent scheduler. Crypto/remote services must retain Message ownership and exact
provider contracts; do not delay local self-hosting to implement every future
distributed service or every libsodium API.

## 24. Native/foreign boundary, const and decimal

Keep C-facing code narrowly about OS/library primitives, ABI conversion, native
handle lifetime and required Message-internal synchronization. Algorithms that
will become L2 belong in LM source, currently L1 during bootstrap. New authored
headers use `name.h.lm1`, producing `name.lm1.h`; do not confuse generated headers
with authored source or add an inner h:/l1: wrapper to header units unnecessarily.

const fixes the described binding; immutable fixes described contents under its
tree/reference policy. `immutable: @: char name` -> const-char-pointer behavior
does not freeze the pointer itself and is not a defect. Avoid a blanket const
rewrite without verifying parameter-list grouping/lowering in L1.

Use const/static written-set proofs to remove unnecessary work only after the
semantics are correct. For read-only own fields a proven-never-written flag can
remove impossible dirty locals/checkpoints. Never use that proof to omit the
underlying graph field or ignore a possible alias/callback write contract.

Text equality should use meaningful hashes where reusable, then exact length/
byte comparison to resolve collisions. Mutable text cannot reuse a stale cached
hash without the selected invalidation/immutability proof. Do not replace exact
equality with hash equality or allocate process-global string catalogs.

Existing `LMX_DEC.txt` documents the foreign decNumber wrapper, not a need for a
second decimal engine. Current source directory:
`third_party/decNumber/decNumber-icu-368`; compile decNumber.c and decContext.c with
the selected DECNUMDIGITS=34 ABI. Wrapper context is owner-local and operation
status is distinguished from sticky history. Do not pass decimal through double.
Its native source remains C by user authorization; consumer wiring and inclusion
in the portable bootstrap dependency set must still be checked.

## 25. Explicit remaining decisions: ask BEFORE freezing these APIs

### Decisions supplied 2026-09-12

- Merge stores fields in exactly the order written in the `merge:` body. The
  physical value has only `lmx *node; int len; void *data;`. Its `node` points
  to the Structure whose body contains that `merge:` receiver. A merged child's
  own `node` pointer is not changed. Ordinary merge therefore does not copy an
  ancestor environment, reparent children, rewrite existing nodes or change the
  tree. Do not introduce a separate copied lexical skeleton or a second
  membership representation.
- Used-tree copying is a separate operation used when a value crosses into a
  destination Message/arena. Copy the complete used closure into that destination:
  follow the required data/reference edges and the lexical `node` chain until
  `node = 0`. An `independent` root stops that lexical walk naturally because
  its node is already zero. "Used" may be the whole selected Structure/tree;
  when it is, copy all of it. Do not prune anything in the used closure. The
  only excluded material is unrelated source graph state outside that closure.
  Unknown/runtime-selected use is resolved conservatively at runtime and may
  make the relevant whole Structure part of the used closure.
- The physical declared-throw/result carrier is the explicit C ABI variant:
  a status return plus typed result and throw-payload out-parameters. A throwing
  call does not publish/store its ordinary result. Declared throw and runtime
  failure have distinct status values. This shape is visible in prototypes,
  imports, external declarations and function-pointer descriptors. Payload
  storage is activation/Message-owned; do not use the historical process-static
  `throw_code` / fixed payload array from `lm2`. `setjmp`/`longjmp` remains the
  diagnostic `assert` mechanism and is not the declared-throw carrier.
- Native numeric operations have the target backend's native semantics without
  a Lingvamyxa checked/wrapping arithmetic extension. The C backend follows C
  exactly (`u64` therefore has C unsigned modulo behavior; other C numeric
  portability/undefined-behavior limits remain C limits). A VM backend may have
  its own native differences. Memory allocation, buffer bounds and indexing
  still require their existing explicit safety checks; those are not a new
  arithmetic language mode.

These decisions close the physical result/throw carrier and u64-overflow
questions. They do not by themselves settle the other rows below.

The following are real open details in the inspected spec/ABI, not excuses to
reopen settled Message/lexical/dirty-only rules. Read their current sections in
case a newer commit settles one. A later decision must be documented with examples.

| Question | Fixed boundary that any answer must respect |
| --- | --- |
| ~~Ordered merge-result membership vs copied lexical skeleton representation~~ | **DECIDED 2026-09-12:** fields follow `merge:` order; physical value is only `lmx *node; int len; void *data;`; `node` is the Structure containing the receiver; child `node` pointers and the tree do not change |
| ~~Encoding/discovery of selective lexical dependencies and unknown paths~~ | **SEMANTICS DECIDED 2026-09-12:** on cross-Message/arena copy, traverse and copy the complete used closure, including the required `node` chain to zero; `independent` supplies a zero root; a whole-Structure use copies the whole relevant tree; only the concrete metadata/work-list encoding remains implementation work |
| Per-domain payload copy policy incl mutable cells/arrays/opaque resources | no implicit foreign-arena LMX aliases; ordinary reference calls not cloned |
| ShortNameId encoding, collision handling, anonymous/positional registration | canonical linked identity; one auxiliary reverse-name service; first occurrence default |
| Empty representation and len unit per physical domain | absent/empty/value/descriptor not conflated |
| Physical result / declared-throw / runtime-failure carrier | exact sig and typed calls; correct cleanup/rooting; no per-name result structs or hidden Namespace |
| Post-bind @ for argument-as-own-cache | do not silently switch lifetime/storage; see §16.5 here and spec11.3.1 |
| Active own occurrence moved/removed by nested code | no silent relookup of new [0], reinsertion or stale freed target; bootstrap can reject mutation |
| Checkpoint-store failure | no outbound call before required publications; define visible stores/dirty state; no recursive failing epilogue or implicit rollback |
| Explicit source navigation above immediate node | no implicit ancestor fallback or extra node hop |
| Exact method-record/linker/function-pointer representation | canonical sig, reachable nested code, documented native portability |
| Merge failure carrier/temporary retention | sources not corrupted; no published half-built result; no invented general transaction |
| Retention budgets, remote codecs and full failure-graph export | direct-parent local handoff semantics and no timeout-as-quiescence remain fixed |

Ask a concise question with a minimal LMX scenario, two concrete outcomes and the
affected interface. Continue independent work meanwhile. Do not bury the question
in an agent-to-agent outbox where the user discovers it days later.

Historical sections 2.3 and 10.2--10.5 conflate ordinary merge membership with
cross-Message copy when they tell merge to copy an ancestor environment. The
decisions above supersede that prose: ordinary merge does not copy the ancestor
tree or rewrite child nodes; the used-tree/node-to-zero traversal belongs to
copying into another Message/arena. Correct those sections in one focused docs
checkpoint before implementing merge; do not treat the stale wording as authority.

## 26. Implementation roadmap after the current slices

The order is dependency-based, not permission to wait for every later feature.
Recheck which parts are already committed before assigning each stage.

### K1 — close current Message-local storage/collector slices

Codex: known-char read, exact seven files, focused evidence and handoff.
Grok: finish arena_collect extraction and current lifecycle integration, preserving
the successful collect_block/mark/liveness contracts. First resolve the concrete
source-header predef issue in section 5.3 using the already passing local-type test;
do not invent an owner-link fix or heap workaround. Jointly inventory remaining
legacy global char/P0/query/method/name domains, root omissions and source ABI gaps.

Acceptance: actual code through the active path, matching output/identity/cleanup,
no borrowed compiler globals leaking into emitted-program runtime, exact remaining
limits stated. A helper module PASS is not the whole kernel.

### K2 — prove the Message-first concurrency boundary

Reconcile current per-context implementation vs prototype compatibility helpers.
Verify no hidden global family table, direct-child management, parallel different
turns, same-Message serialization, mailbox wake race, stop/poll/failure handoff,
unrooted adoption GC, control retirement and OS-affine ingress.

Finish full local LMX-text/arena delivery where only number/byte copying remains.
Do not claim that the number/byte transport plus an unrelated adopt helper already
constitutes the complete language-facing delivery path.

Give Claude an accepted minimal API/pin and exact migration notes. This does not
wait for every future HTTP/Mix/crypto feature and does not authorize publishing
the old global dispatcher. Notify the user when this milestone actually closes.

### K3 — uniform graph-hosted executable bodies and complete typed calls

Bring discovery/layout/validation/emission into agreement; implement missing if/
for/body/trailer hosting and same-name hidden/declared binding. Complete transitive
DynRequired/OwnUsed and runtime-selected exact-signature calls needed by the real
programs. Retain direct const-call optimization only with correct proof/context.

This creates the accurate graph/code-use information needed for selective merge;
do not bolt merge onto a frontend that silently omits nested bodies/methods.

### K4 — generic runtime builders, selective merge, independent, implements

Use sections10–17 here as decomposition/acceptance. Resolve actual open ABI points
with the user. Keep generic Lmx operations instead of per-name C layouts or eager
module construction. Incrementally add supported payload domains and demonstrate
real graph results/calls after merge, not just pretty emitted C.

Integrate owner-local tracing/copy metadata and name-index maintenance. Test new
Messages built from a chosen template preserve only required lexical surroundings
while containing no source-arena aliases. Prove independent/unknown interface cases.

### K5 — complete source requirements driven by actual ports

Inventory real language constructs needed by parser/runtime/build/translators:
declarations/types/arrays, control/trailers, calls/returns/throws, const, imports,
foreign adapters and generic graph operations. Implement minimal COMPLETE rules
for those uses, not arbitrary one-off allowlists that happen to compile a single
sample. Add a regression for each real compiler defect; do not rewrite programs
into C to avoid exercising L2/L1.

### K6 — source-port the tools, then both translators

Port simple drivers/build utilities first where useful, then runtime/parser and
translator dependencies, then L2 translator, then L1 translator implementation.
Keep generated L1/C and reference outputs as evidence during transition, not as
permanent hidden implementations behind nominal L2 wrappers.

### K7 — full source-only bootstrap, generation fixed point, stable promotion

Implement and test the old build/next/check/finalize algorithm in the new chain.
Use an isolated tree, tracked portable generated C seed and no oldchain binary
dependency. Run the acceptance in sections28–30. Promote only a verified candidate,
with exact compiler/runtime hashes and communication to every consumer.

## 27. Parser compatibility: preserve the reference's deliberate syntax

The old worked-version parser is the user-designated oracle. Its counterintuitive
choices are often the design, not bugs. Preserve/extend TREE assertions on that
version first, then apply the SAME fixtures to the new parser. An error/no-error
comparison alone misses argument/block attachment regressions.

Required families:

- Same-line short receivers followed by indented lines. Newline resets the source
  nesting base to the line's starting receiver, not the deepest inline short form.
  The old rejected double-increase example stays negative with that explanation.
- Multiple short arguments followed by multiline blocks: assert exact order,
  attachment and receiver/trailer role, not just successful parse.
- Without a trailer, decrease nesting by at most one level. Valid return,
  end-with-arguments and correctly placed dash trailer may close multiple levels.
- End target uses the defined name-attribute priority and admitted receiver-name
  alternatives such as end:fn. Bare end is forbidden everywhere.
- A colon receiver requires actual arguments or nested blocks. This general
  validation was explicitly moved from translator to the NEW parser. It is not
  a special ban on return trailers. Nullary receivers use the allowed no-colon/
  empty-parentheses forms. `return:` with a following indented expression is valid.
- Long dash runs; BOTH single-quote and double-quote long strings; equals and
  asterisk fences. Preserve exact length/shorter-body/closing rules. Only quoted
  strings use A+1 encoding for A>=N delimiter characters; raw-comment fences do not
  silently reduce their interior runs.
- CRLF accounted lazily without replacing the source or letting the extra Windows
  character corrupt line/column/offsets. New files/output default to LF.

Mine important old-spec examples from Git, not memory. Freeze reference commit,
input bytes, oracle tree/error, toolchain and comparison normalization. The new
empty-colon validation is an intentional approved difference; other deviations
need evidence/user agreement, not "this is how conventional languages work".

Keep newly added oracle tests if repairing the old checkout after an accidental
pull. The secondary repository is also a recovery source, not a clean branch to
overwrite the current implementation wholesale.

## 28. Port and build dependency inventory

Current source/build names to examine, not proof they are all on the new path:

- `l1src/make.lm1`, `l1src/buildCore.lm1`, `l1src/build_l1.lm1`, corresponding
  `stg/l1_baseline/l1src` copies;
- historical `lm2/make.lm2`, `lm2/buildCore.lmx` and snapshot copies;
- `stg/l1_baseline/l2src/printTree.lm2` and partial parser helper `.lm2` ports;
- `l1src/l1trans.lm1` and current `stg/l1_baseline/l2src/l2trans.lm1`;
- root `buildCore.lm0.bat`, `buildCore.lm0.sh`;
- tracked generated bootstrap sources under `lm1/build`.

Create a concrete inventory during the port with columns:
logical component, current authoritative source, language, producer/generation,
headers/foreign dependencies, executable consumers, reference test, L2 port status.
Do not count old .lm2 files or facade wrappers as completed ports without proving
the active translator builds their actual implementations.

For a module port:

1. Pin its current observable behavior and API, including error and ownership.
2. Translate the implementation to L2, with generic graph/state where required.
3. Generate L1 with the current L2 translator, then C with the pinned/candidate
   L1 translator; compile/link once for compatible tests.
4. Exercise the actual callable/driver behavior against reference expectations.
5. Record generated output and dependencies; remove obsolete implementation
   reliance only after the replacement is accepted.
6. Commit/push and update the component inventory. Continue the next dependency.

Real useful first ports include make/buildCore/printTree or their reusable pieces,
but do not redesign their workflow around a new host scripting framework.

## 29. What full self-hosting must demonstrate

### 29.1 Naming the generations precisely

Record which binary builds which source, not just "gen2 passed". One possible
explanatory naming is seed -> candidate1 -> candidate2 -> candidate3; use the
project's existing names/commands when implementing.

- Build the first L2-written L2 translator using the currently trusted chain.
- Use that generated translator to translate its OWN L2 source to L1, then C,
  then another working translator.
- Do the equivalent for the L1 translator's final L2 implementation, so L1 output
  remains a real intermediate rather than a hidden old executable dependency.
- Build the parser/runtime/driver dependencies from the same source set.
- Compare deterministic generated sources and the required behavioral checks
  across further generations; explain any legitimate nondeterminism explicitly.

Do not require byte-identical EXEs if platform/linker metadata makes that irrelevant;
do require a meaningful fixed point of the selected deterministic artifacts and
behavior. A gen1 smoke test does not prove a gen2/gen3 fixed point.

### 29.2 Portable source bootstrap

The repository intentionally keeps a minimal generated C/header subset, historically
including `lm1/build/{parser,own,l1trans,printTree,make,finalize,buildCore}.lm1.c`
and the generated P0 header. Determine the EXACT final needed set from script
dependencies and `git ls-files`; do not copy every build artifact or assume old
REST/trans files are all required.

In a clean ISOLATED checkout/build area, without installed historical LM binaries:

1. Build that tracked C set with the platform C toolchain via buildCore.lm0 scripts.
2. Use the resulting executable(s) to build current LM sources.
3. Exercise self-build, next, check and the same finalization/test flow.
4. Prove no oldchain .exe/.a, local untracked generated C, Python translation stage
   or external opaque compiler executable is secretly required.
5. Preserve approved third-party library sources/license and explicit native
   dependencies in the source bootstrap manifest.

The normal C compiler/OS/library dependencies are not forbidden; undocumented
historical LMX executables are the dependency to remove. Do not confuse those.

### 29.3 Native build/next/check/finalize behavior

Port the OLD algorithm, not just a command named next:

- next builds the next candidate with the intended preceding generation;
- check compares/verifies the expected source/behavioral outputs and propagates
  failure honestly;
- tests use the correct generated tool, and failed stages stop promotion;
- finalize/install occurs only after checks, at the existing deferred safe phase;
- path handling/output names/exit codes allow the new executables to repeat the
  process on another platform using the published generated-C seed.

Read the old implementation to recover exact sequencing before changing scripts.
Do not overwrite the shared stable oracle during a test. Root and baseline have
different build roots; `stg/l1_baseline/gate.ps1` is historically hosted and can
rewrite the compiler. It is not a substitute for this isolated source-only proof.

### 29.4 Platforms and VM scope

Verify Windows with its real .bat path. Run the .sh path on an available supported
Unix environment before claiming that platform works; otherwise label it pending.
Do not infer portable success merely from ANSI-looking generated C.

The required native chain is L2->L1->C99. `include_languages/vm_porting_plan_en.txt`
also describes MIR/WASM-linear/RISC-V using generated C through existing tools.
Managed L3->JVM/WASM-GC/etc bytecode routes are future distinct backends. They are
not permission for a second L2 direct-C frontend, and not blockers for native L2
self-hosting. Keep capability/research status separate from implemented backends.

## 30. Release/stable promotion and minimal regression tiers

### Tier 1: edit-loop evidence

Smallest relevant unit/helper/compiler fixture set, one compatible translator/
support-object build. Check exact outputs, graph identity and error behavior.
Hash source/toolchain/options/defines. Different instrumentation defines require
different objects; do not reuse stale compiled code.

### Tier 2: integration checkpoint

After related slices land, run the relevant Message/graph/merge/call/parse integration
suite once on the combined revision. Verify all changes actually use the new path.
Read colleagues' matching saved evidence without rebuilding their active worktree.
Target a discovered gap rather than rerunning every historical check blindly.

### Tier 3: stable candidate

Isolated full regression, source-only bootstrap, next/check/fixed point, required
platform checks and dependent app API smoke. Preserve sources of tests removed
from per-edit mandatory lists, with an explicit way to run them. Do not hide a
real failure by reclassifying it as optional.

Only promote after proving the candidate and coordinating consumer downtime.
Publish source/generator/runtime/header hashes, exact new stable compiler path,
API changes and evidence. Notify Claude explicitly; he is waiting for a usable
Message boundary and must not silently copy moving core files into vendor.
Update pins coherently, never "fix" a mismatch by accepting whatever file exists.

Every executable/compiler/runtime accepted into stable needs provenance and
rebuild instructions. Every generated C file tracked as bootstrap is deliberate;
ordinary generated scratch C in source folders is not automatically a deliverable.

## 31. Final core acceptance checklist

Do not announce full success unless these claims can be backed by exact evidence:

- [ ] Root process and every LMX participant follow Message ownership; no shared
      global LMX pools/registry/manager remain on the active path.
- [ ] Different Messages execute in parallel; own turns are serial; mailbox
      admission/wake and direct-child scheduling work without external policy.
- [ ] Completion, ordinary stop, emergency escape, parent/child silence, native
      quiescence/control retirement and failed-arena retention follow the spec.
- [ ] Local delivery transfers handoff-safe storage without implicit copying or
      node rewriting; no forbidden foreign-arena aliases survive.
- [ ] End_turn really collects adopted-but-unretained dead storage exactly once;
      retained roots/domains remain valid; OOM does not cause unsafe sweep.
- [ ] Generic runtime construction and merge work on dynamic operands/results;
      order/fresh occurrences/first-name default/lexical closure are correct.
- [ ] Selective ancestor closure does not copy unrelated siblings by ancestry,
      does preserve transitive/alternative/unknown-route requirements and aliases.
- [ ] independent cuts external lexical surroundings but retains described dynamic
      inputs, internal fields and runtime-selected exact-signature calls.
- [ ] Executable bodies including if/for/trailers are hosted by consumption role;
      arguments do not become fields automatically; graph materialization,
      execution and same-name binding are distinct and correct.
- [ ] Used-only/dirty-only/no-reload behavior, same-name hidden/declared binds,
      actual evaluation order, result rooting and cleanup ordering are verified.
- [ ] implements/RuntimeImplements and auxiliary names obey their actual contract,
      not the old shallow presence/name-lookup implementation.
- [ ] Parser structural parity is checked against the old oracle with explicit
      approved new validation differences, not only success/error smoke tests.
- [ ] Both translators and promised core/build tools have authoritative L2 source
      and are built through L2->L1->C; no L2 facade delegates the real work to an
      unported historical executable.
- [ ] Tracked portable C bootstrap builds a seed without old LMX binaries; next,
      check, tests, finalization and generation fixed point work in isolation.
- [ ] Stable promotion is coordinated/documented, consumers notified, verified
      source/generated-bootstrap stages committed AND pushed.
- [ ] Any remaining open ABI/profile issue or untested platform is explicit; no
      incomplete module or research route is advertised as completed functionality.

## 32. How to continue without wasting the new chat

Do not reread this whole document on every wake. Once oriented, keep a concise
current stage/ownership/evidence/next-step note in the repository or current
automation memory; this document remains the durable architectural handoff.

Spend most working turns implementing the next bounded owned stage. Ask the user
early at real forks. Use Grok for substantial code and architectural review; help
him with concrete independent slices rather than generic offers. Completion of one
slice is a trigger to continue, not a reason to wait for another reminder.

The old chat can be deleted: no step above depends on retrieving it. Remaining
questions are intentionally recorded as questions, not invented resolutions.
