# Core team implementation plan — 2026-09-12

Current work/evidence reconciliation — 2026-09-12 12:06:
The user reports Fable next takes independent: const: immutable branches.
f12ea87f and 0c2494df are pushed DOCUMENTATION-ONLY commits. They close false
model/dependency questions, not the callable emitter defect. Task115036 for
per-callable Structure/selection is still assigned/unverified, alongside the
branch/array work. Do not label either implemented from those two commits.
See the quick answer map near the start of L2_CORE_AND_MESSAGE_MODEL_20260912.md.


Latest callable clarification (2026-09-12, user relayed by Fable114853):
The graph callable is ordinary Structure M; physical child slot0 points to the
shared immutable METHOD {addr,sig}, further children hold M's own fields/body.
Only the descriptor lacks node: M has its ordinary lexical-parent node. A call
selecting M through R passes M itself as the reserved own argument, not R.
Physical slot0 is not occurrence[0]name. Copy M with its internal order and
remap node/ordinary references; keep the METHOD address. One published M per
occurrence, separate C activations for recursion, dirty-only spill/no reload.
Model40 is CLOSED as flat-unit emitter error; Fable115036 implements callable
Structure/selection and repairs stale outer-container path ordinals. Proven
internal offsets/descriptor slot0 are not forbidden. No new layout/name table,
method clone, refusal, or own-field-location ABI workaround is introduced.
Grok remains CLOSED: no new inbox messages until explicit user resume.


CURRENT USER AVAILABILITY — 20260912-114331: the user CLOSED Grok. No new tickets,
coding tasks, reminders or document notifications to Grok until the user
explicitly resumes him. Do not restart his closed session or automatically
rearm its helper. Preserve unfinished112047 nested Exec hang work and evidence.
Codex/Fable continue their owned scope; Codex continuity/outbox receiver stays
active. This supersedes the earlier11:20 ACTIVE instruction below.


Latest method-storage clarification (2026-09-12, user relayed by Fable):
The first, OS-root Message owns TWO separate const: immutable arrays: one
retains the translation-known independent: const: immutable branches, and the
other contains the known method descriptors. Descriptor storage remains valid
until OS-process termination; finishing a borrowing/source child Message never
reclaims it. This is root-Message-owned storage, not ownerless storage and not
allocation in each invoking child arena. Merge/Message copy retain admitted
branch references and method-descriptor addresses as their respective terminals.
The METHOD descriptor stores no node; placement does not change the callable
Structure supplied as the own argument.

The translator builds the initial graph belonging to the first Message and
fills its two const: immutable arrays. It does not implicitly spawn further
Messages. Subsequent Message creation is an explicit operation copying all
required used graph state/references with the common copy map. Inputs may
explicitly include a reference to an entire admitted immutable root array, one
qualified branch, one METHOD descriptor or a system value. Receiving one branch
does not automatically expose its retention array or unrelated root settings.
There is no requirement for a child to discover a global root accessor. In the
current emitter process_message already denotes this first Message; descriptor
ownership there is correct. The remaining frontend work is dedicated array
shape, qualifier emission and matching copy metadata, not relocation from a
child arena or a blocked executor API.


Latest eternal-array clarification (2026-09-12, user's Grok discussion): the
OS-root Message ARRAY contains references to ALL independent: const: immutable
branches of ALL Messages in the running OS process. The set is known at
translation; the ARRAY itself is const: immutable and never grows/appends at
runtime. Merge/create substitute the same admitted branch address and stop;
they do not deep-copy those branches or register new entries. Retention is not
lexical parenting or an implicit view of root settings. See SPEC 9.1.4/model20.


Latest body/bind clarification (2026-09-12): ALL executable bodies belong to the
graph; callable and return arguments do not become graph fields merely by being
arguments. An executed arg: 5 in the body makes arg an own field FROM THAT POINT,
with the same working variable/address/lifetime and dirty checkpoint publication.
Preparing fixed slots does not activate the binding before that line. See model
section 11, SPEC 21.5/21.5.1 and Revision 2 section 6.5.

Latest user instruction, 2026-09-12 11:20: Grok is ACTIVE again. Keep his FSW
and 30-minute watcher running; do not auto-pause based on usage. The user will
close Grok when the limit is exhausted and notify Codex. Current assigned task:
work_chat/grok/inbox/20260912-112047-nested-exec-hang.txt — diagnose and fix the
repeated nested Message Exec hang after m0_acc, with a focused causal regression.
Fable keeps graph ABI/frontend; Codex coordinates integration and documentation.

## Current documentation/integration checkpoint — 2026-09-12 11:05

Read [L2_CORE_AND_MESSAGE_MODEL_20260912.md](L2_CORE_AND_MESSAGE_MODEL_20260912.md)
before selecting work: complete model first, implementation/evidence next,
ordered work plan last. The latest SPEC/ABI definitions supersede historical
questionnaires. One copy map spans every merge operand; METHOD and admitted
eternal branches are terminals. The root owns an ARRAY retaining eternal branches,
not their common lexical parent or a shared settings namespace (SPEC 9.1.4).

Fable reported copier 0763a2cf: 55/0 copy, 63/0 ABI and 95/95 fixtures. Codex
acceptance is pending. Model40 is resolved: callable Structure was collapsed into a flat unit in the
emitter. Fable115036 implements own Structure/selection before merge integration.
Grok reported e5ba5119 strengthening the permanent overlap assertion. Its
evidence is pending independent review; accepted production close remains
70759d0e. Earlier nested Exec timeouts remain unexplained, not fixed by a retry.
Do not repeat identical full builds. Codex is completing documentation, then
reviews these exact slices and continues the integration sequence in the model.

The following 10:39 checkpoint and older entries are dated history, not the
latest instruction to start merge lowering or repeat an accepted stage.


## Latest integration checkpoint — 10:39

Fable resumed at 10:32 and is implementing the shared used-graph copier on
fable/graph-copy in build/fable/graph-abi, based on Codex candidate e06966ee.
Its new lmx_graph_copy_owned files and direct tests are Fable-owned; merge
lowering follows, and Grok integrates the copier into Message creation.
The earlier 63/0 and 95/95 ABI evidence is accepted; do not rerun it unchanged.

Codex completed historical-runner prerequisites in 2c4a12d1 and the scanner
parity driver in e06966ee. Production support builds 19 objects; the eight-method
check passes. Scanner parity passes 118 cases and exact cleanup of 902 tracked
allocations. Remaining Codex integration includes handwritten splice bodies,
root/cancellation drivers and explicit reusable runner inputs. No main ABI
promotion yet; private-candidate checks used the older D7 production runtime.

Grok's pushed 7fd9f1bb combines the ABI with main D7 through 040af5d2. The
clean selected Exec run passed with exact hashes reconciled. Its two earlier
timeouts remain unexplained; no hang fix is claimed. The subsequent 598487cc
drive_tree snapshot stage also has verified successful evidence. Grok then
fixed drive_should_close's mailbox check under exec in 70759d0e. Codex's
independent ordering probe rejects the old path and passes the fixed path;
Grok still needs to retain this ordering assertion in the permanent suite
(104430/104506). The settled D4 node/merge wording is corrected on both the
candidate and main documentation. These corrections do not explain the
earlier timeouts. Claude retains mixa_manager; no resumption inferred.

Evidence indexes: build/codex/combined_exec_20260912_102718/review.json,
build/codex/drive_tree_598487cc_review.json, and the driver evidence linked
from stg/l1_baseline/l2src/FABLE_GRAPH_ABI.txt on the candidate branch.

## Current user authorization

The user authorized Codex to code alongside Grok and Fable 5.1. The user has closed Grok; no new assignments until explicit resume. Preserve
his nested Exec hang work and do not auto-restart his session. Fable and Codex
continue their active scope. Claude retains all mixa_manager work. This supersedes older
planning-only/exclusive-Fable/quota-rotation instructions. Use the named existing
mailboxes, not substitute internal agents.

Documentation checkpoints 3a9f8037 and 46b9aba9 contain the latest merge/node/
shared-method corrections and supersede earlier wording in 7ed53b03/7c5aec15.
Read actual SPEC and refactoring EN/RU source sections, not an old question list.
Raise a user question only for a concrete logical contradiction in the language model:
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

- Eternal storage decision (SPEC 9.1.4): the OS-root Message owns an ARRAY of
  retained independent: const: immutable branches, not one mandatory lexical
  tree. Branch roots have node=0; links remain at declaration sites and other
  explicitly selected graph locations. Retention never reparents them. Published
  branches are nonmoving and retained until process exit; other Messages get
  explicit branch references, not implicit root/array visibility. Merge and
  Message creation retain those addresses as copy terminals. This is a new
  implementation requirement, not evidence that the candidate already has it.
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
- METHOD {addr,sig} and native code are shared. The callable graph occurrence
  is an ordinary Structure M, descriptor in physical child[0], own/body fields
  in its other child slots. A call selecting M through R supplies M as own
  argument; M.node is lexical parent. Copy M/internal children and remap node,
  retain descriptor address. This corrects the former flat-unit assumption.

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


## Integration review checkpoint — 2026-09-12 09:29

- Codex completed the documentation correction in 3a9f8037 and 46b9aba9:
  full used-graph/node copying and unchanged shared method-descriptor pointers.
  The residual-rule audit read 235 tracked UTF-8 documents; evidence is
  build/codex/node_shared_methods_audit_20260912.json. Normative doc ownership
  is released; coordinate any overlapping future changes with Fable.
- Claude 7e4315b624e5a9a272f54094f2e6471cd4aa39fd is accepted for selection
  walking and MP3 composition. Codex verified exact committed module/test/runner
  hashes, compiler hashes and zero translate/compile/test exits for 11 saved
  runs: walk 100/0 twice, MP3 36/0 twice, audio 85/0, native MCI 30/0,
  scan 16/0, launch 45/0, panel 45/0, button 33/0 and file_manager 144/0.
  Exact paths/hashes are in build/codex/claude_7e4315b6_acceptance.json.
  The root-snapshot change is defensive: the hypothesized lost-root sequence
  did not reproduce through the public API. MP3 uses a fake device in this
  composition test; real MP3 playback and a whole-app event pump are not claimed.
  Claude's 94bfc408 is copy design only. Ticket 20260912-092435 resolves the
  implementation prerequisite: a separate tracked mixa_fileio seam, then the
  first copy-only operation on owned fixtures. Untracked legacy mixa_file*
  remains untouched. MP3 keeps candidate 24A1B57B; no stable promotion.
- Grok D7 17ef86df is pushed but acceptance remains open. Saved Windows Exec
  exits and runtime/test source hashes match that revision. POSIX is still
  compile-only. Ticket 20260912-092648 requests proof/fix of child-list lifetime
  and synchronization after exec is dropped in the host sched_step scan; only
  the parent is currently retained. Startup saved pointers also need a concrete
  quiescence/lifetime argument. Continue the remaining drive_tree lock work.
  Codex's independent active-request/queued-request OOM/retry probe passed
  against f8012796 runtime objects: each recipient gets one DEAD with the right
  correlation. Evidence: build/codex/d7_review/evidence.json and probe.log.
  This did not establish the initially suspected duplicate-notification bug.
  The old 091609 selected-Exec receipt has a selftest-source hash different from
  committed f8012796 (a diagnostic marker was reported); do not label that
  whole run exact-commit acceptance. The 092030 run matches the committed test.
- Fable's graph ABI/frontend work remains isolated in build/fable/graph-abi.
  Its shared used-graph copy helper is not yet delivered; Grok's Message
  create/set_graph integration follows that interface. The core/full self-host
  milestone is not completed by these bounded acceptance checkpoints.
