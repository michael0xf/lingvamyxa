# Fable 5.1: current graph ABI/frontend handoff

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

Updated 2026-09-12, 11:05 local. This replaces the temporary quota-rotation
assignment. Grok, Fable and Codex implement the core together; Claude owns
mixa_manager. Grok owns Message exec/D7, Fable owns graph ABI/frontend/copy,
Codex owns L1 and integration/review. The planning-only rule is withdrawn; current Grok availability is stated above.

## Read the model before selecting work

Read [L2_CORE_AND_MESSAGE_MODEL_20260912.md](L2_CORE_AND_MESSAGE_MODEL_20260912.md),
parts I-III in full. Part IV separates evidence and implementation gaps; its
last part gives the ordered work plan. Consult the actual corresponding source
sections in Lingvamyxa_spec.txt and struct_refactoring_version_2.txt before
changing a mechanism. This file is a role/status handoff, not another spec.

Only a concrete contradiction between current model rules warrants a user
question. Missing code, stale tests and ordinary backend choices are engineering
work. Follow work_chat/TICKET_RULES_EN.md; ACK/seen is WORKING, not completion.
Preserve shared dirt and others' files; no reset/stash/clean/force/bulk staging.

## Settled constraints to keep together

- L2 is low level. Structure is only Lmx *node; int len; void *data.
  len counts direct children; data is ordered void * slots. Classify the stored
  TARGET address in its typed range. Only STRUCT has the common header.
- Array descriptor len/data is separate. @: char "hello" refers through a
  typed char * entry to raw string bytes, not a hidden length descriptor.
- @ is ordinary address-taking, forbidden in L3. Same-name own bind keeps the
  same argument variable/address/lifetime. No invented ownership conversion.
- Fields have fixed slots; their references may change. Own publication is
  actual-dirty only, no post-call reload; checkpoint-store failure uses assert.
- Merge and Message creation copy the COMPLETE USED graph/lexical tree to zero
  by one common traversal. Rewrite copied child/payload AND node links; one
  operation-wide map spans ALL operands/roots and preserves aliases/cycles.
  independent supplies zero. Source objects are unchanged; copies have fixups.
- Result fields follow operand order then body. Result.node identifies the
  Structure containing merge. A call through the result selects callable M
  and passes M as own argument; its child[0] shares the node-less METHOD.
- SPEC 9.1.4: the OS-root Message owns an ARRAY retaining branches qualified
  independent: const: immutable until process exit. Branch root node=0;
  declaration-site and explicit references keep their positions. No common
  lexical tree, implicit settings access or mandatory content interning.
  Admitted eternal references are copy terminals; the retention array is not
  imported. Merely immutable/const state is not automatically eternal.
- fn supplies own node, lexical, dynamic and explicit inputs. Typed status plus
  result/throw outputs is settled; merge failure is throws merge(args), never
  successful return 0. longjmp is for assert. Arithmetic follows native C/VM.
- Names compile out; address -> short source name is auxiliary string data,
  not execution identity. Collisions and mandatory registration are irrelevant.
- One Message arena may have many nonmoving blocks. No relocating live cells
  or arbitrary 16/1040/64/128 capacity rule. Non-copying ownership handoff is a
  distinct operation with stable addresses, ranges and explicit roots.

## Accepted foundation and current report

The original graph ABI stage rescued as 00500bbb has accepted 63/0 selftest
and 95/95 historical fixtures twice. Candidate e06966ee adds accepted collector,
Array-driver, historical-runner dependency and native scanner migration slices.
See central model part IV for exact evidence; this does not claim all modules
or self-hosting have migrated. Main's old representation is not the candidate ABI.

Fable owns build/fable/graph-abi on fable/graph-copy, based on e06966ee.
Reported 0763a2cf extends 3a374fba with lmx_graph_copy_many_owned, one map across
all roots, explicit eternal_ranges, raw-target rejection and 55/0 copy checks.
Single-root copy is the wrapper. Codex independent acceptance is pending;
reported success is not silently relabelled accepted. The eternal retention
array and separate method-descriptor array still need their emitted graph shape
and qualifier/copy metadata. Records already belong to the first Message; no
blocked root-accessor/Grok API is required. See the latest clarification above.

Current helper restrictions, including zero-length Array coverage and refusal
of an eternal root as a direct source, do not define new language restrictions.
Do not let an unsupported child/target escape as a silently retained own pointer.

The old model40 conflict is closed by the callable Structure/descriptor
separation. Fable115036 implements per-callable M, METHOD at physical slot0,
own/body fields, selected M as reserved argument and correct occurrence paths.
Do not mistake an external lexical field for an own slot in M. Internal field
order survives copying; top-level merge order is separate. No field-location
ABI workaround, copied B receiver or method clone is required.

Fable continues the owned frontend/copy stage with focused checks. Grok is closed;
Fable no longer owns the old temporary non-self-recv/fail-stop assignment.

## Integration and verification

Codex completed L1 import storage b41af667, path-buffer removal 3cacecc2 and
depth-16 removal 5704f616 (34 checks). See L1_IMPORT_CAPACITY_20260912.md;
the earlier 24-check stage is historical, not the current remaining task.

Stable L1 is read-only, SHA256
65D5A5ED127CA1BAEBDD1D500A5B74CEEA63EC1985EAC52EDEF28EFEB261C936.
Do not repeat an identical accepted build. Reuse compatible support objects only
with exact source/toolchain/flags/defines hashes. Broad historical/self-host
checks belong at an actual integration boundary. Report changed paths, commit,
evidence, successful exits, limitations and the next owned slice.

Share copy/ABI boundary changes with Grok and Codex through the existing named
mailboxes. Keep LMX_MSG_CONTEXT_V0/EXEC_HOST_V0 and active handoffs aligned when
the implementation actually changes. Review the central model before resuming
after a quota pause or model/context switch.
