# Core team implementation plan — 2026-09-12

## Current user authorization

The user restored Grok to active core work and authorized Codex to code alongside
Grok and Fable 5.1. Usage conservation and the old quota pause are no longer work
constraints. Claude retains all mixa_manager work. This supersedes older
planning-only/exclusive-Fable/quota-rotation instructions. Use the named existing
mailboxes, not substitute internal agents.

Documentation checkpoint 7ed53b03 contains the settled L2 answers. Read actual
SPEC and refactoring EN/RU source sections, not an old question list. Raise a
user question only for a concrete logical contradiction in the language model:
show the two incompatible rules and one minimal program/outcome. A missing
implementation, old test expectation or ordinary backend choice is engineering
work. Resolve documentation omissions from the user's recorded decisions.

## Non-overlapping current stages

| Owner | Implementation boundary | Current deliverable |
| --- | --- | --- |
| Codex | l1src/l1trans.lm1 and its baseline mirror; new import-capacity tests/runner and stage notes | Import tables, path buffers and depth guard corrected through 5704f616 (34 checks); urgent merge docs correction and integration review; no stable promotion |
| Grok | stg/l1_baseline/l2src/lmx_message_exec.c/.h and exec selftest, related D7 runner/context notes | D7 non-self recv and fail/stop inbox traversal outside exec lock; pins, FIFO, exact cleanup; then remaining D7 lifecycle paths |
| Fable 5.1 | L2 graph representation and frontend: lmx.h, branch/primitive/method/own/range graph APIs, l2trans.lm1, their direct fixtures and dedicated runner/notes | Coherent void * child-pointer representation through construction/access/ordinary merge/typed lookup and emitted code; isolated integration candidate with precise remaining migration inventory |
| Claude | mixa_manager | Continue existing agreed app work, consume new candidate only after integration verification |

Keep code changes in isolated worktrees/checkouts when shared header/runner
changes would disrupt another owner's active build. Build into owner-specific
paths. Shared main/index and unrelated untracked files must be preserved.
Fable owns graph ABI; Grok may read but does not edit lmx.h/branch APIs in this
stage. Codex owns L1 translator capacity and does not edit Fable's l2trans.lm1.
No owner may bulk-stage/reset/stash, overwrite the stable compiler, or replace
another owner's in-flight code. Coordinate any boundary change explicitly.

## Model each stage must preserve

- Structure is Lmx *node; int len; void *data. len counts fixed child slots;
  data addresses void * child values. Type comes from the stored target address
  in its typed array/range. Only Structure targets have the common Lmx header.
- Merge and new Message creation use the SAME traversal to copy the COMPLETE
  USED graph, including all required fields/references and lexical node chains
  to zero. Explicitly rewrite destination node AND child/payload pointers through
  the copy map; preserve aliases/cycles and field order. independent cuts outer
  lexical surroundings with node = 0. Whole-tree use copies the whole relevant
  tree. The earlier pointer-only merge/no-ancestor-copy policy is withdrawn.
  Result root node identifies the Structure containing the merge receiver;
  source objects are not overwritten. A copied ancestor need not be a visible
  result field. The scalar-sharing question based on shallow merge is withdrawn.

- @ is ordinary L2 address-taking, forbidden in L3. @: char "hello" is a child
  pointer into a char * array whose values point directly to C strings; no
  String/Array length descriptor. Bind does not change address or lifetime.
- Child references may change, including inside nested calls. Checkpoint stores
  dirty values; type changes use ordinary handling. Store failure is assert.
  Merge failure is throws merge(args). No active-occurrence conflict policy.
- node lexical lookup may traverse ancestors to zero; independent supplies zero.
  Existing dynamic caller precedence remains. fn names ahead-of-time C functions
  and supplies own node, required lexical inputs, dynamic inputs, explicit args.
- All methods and their immutable `{addr, sig}` descriptors are shared. Graph
  copying retains those descriptor references while remapping Structure/node
  and ordinary payload references. A callable child stores no node. The reserved node argument is the Structure through whose child array
  the call is made; after ordinary merge that is the merge result. Lexical
  fallback follows that Structure's own `node` field (user decision 2026-09-12,
  given to Fable; stale "retains its original node" wording corrected in spec
  21.8, ABI 3.2/6.3/6.6/8.4/13.3 and the L2 handoff scenario 3).
- Result/throw uses status and typed out-parameters, activation/Message storage,
  no ordinary result store on throw. longjmp is diagnostic assert only.
- Arithmetic follows native backend behavior, exactly C on C, including u64.
  The optional address-to-short-source-name string table is not execution state.
  Existing graph retention/local handoff does not wait for remote codecs/budgets.

## Integration and continuation

First show the concrete source-to-runtime trace and affected surfaces for a
representation change. Preserve meaningful accepted runtime behavior while
replacing tests that encode the rejected inline-Lmx child model. A passing old
fixture alone is not conformance. No new architecture to make a fixture green.

Use work_chat/TICKET_RULES_EN.md. ACK/seen is WORKING, not completion. Each stage
ends with exact changed paths, actual test exits and immutable evidence/source
hashes, focused commit/push and honest limits. Review each other's integration
interfaces and useful failure cases. A completed bounded stage is not a claim
that the whole L2 core/self-hosting milestone is done.

Codex completed import descriptor storage in b41af667 (24 checks; exact MP3
reproducer translates). See L1_IMPORT_CAPACITY_20260912.md. Descriptor length
is array/string length, distinct from Structure child count. The old 16-entry
and 1040-byte storage-cell limits were bootstrap artifacts, not model rules.
Codex removed temporary path-buffer limits in 3cacecc2 and the import-depth
guard in 5704f616 (34 checks). Codex now reviews integration and corrects merge
source documents under the user's urgent 2026-09-12 used-graph-copy instruction.
After that candidate is verified, Claude can use it for the blocked composed
MP3 unit. Grok proceeds through D7's remaining sched_ready/release/delete paths;
Fable completes graph ABI consumers and frontend semantics. Integrate only
compatible committed stages, run proportional integration checks, then progress
toward the existing full L2 -> L1 -> C and self-hosting acceptance criteria.
