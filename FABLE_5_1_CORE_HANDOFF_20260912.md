# Fable 5.1: current graph ABI/frontend handoff

Latest body/bind clarification (2026-09-12): ALL executable bodies belong to the
graph; callable and return arguments do not become graph fields merely by being
arguments. An executed arg: 5 in the body makes arg an own field FROM THAT POINT,
with the same working variable/address/lifetime and dirty checkpoint publication.
Preparing fixed slots does not activate the binding before that line. See model
section 11, SPEC 21.5/21.5.1 and Revision 2 section 6.5.

Latest user availability instruction, 2026-09-12 11:07: Grok has 98% usage.
Do not give him new coding tasks, repeat-build requests or progress reminders.
His watchers are restored for questions/review of the shared core document;
do not pause them again. Fable and Codex continue their owned work, preserving
Grok's unfinished files. Receiving his review does not activate a new stage.

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
  Structure containing merge. A call through the result passes that result as
  its own node; callable records have no node. Methods/descriptors remain shared.
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
array itself and source METHOD lifetime after teardown are still integration work.

Current helper restrictions, including zero-length Array coverage and refusal
of an eternal root as a direct source, do not define new language restrictions.
Do not let an unsupported child/target escape as a silently retained own pointer.

Merge lowering is held for the concrete access case in central-model section 40:
C has two fields; B has x and m reading x; R=merge(C,B). A static index for B.x
cannot be reused blindly against R. Read the existing lexical/dynamic inputs,
sig and path rules before proposing an implementation. Do not silently switch
the agreed node from R to B', add a per-node layout descriptor or clone methods.
If the actual source constraints conflict, give their exact locations and the
minimal example; do not reopen the unrelated settled model questionnaire.

While that case is reviewed, Fable may finish the already owned legacy
branch/own/ref fixture migration and checkpoint-store assert. Grok continues D7;
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
