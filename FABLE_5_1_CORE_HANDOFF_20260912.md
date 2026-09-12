# Fable 5.1 temporary core handoff

## Role and clean starting boundary

CURRENT OWNERSHIP (2026-09-12, latest user instruction): Grok, Fable 5.1 and
Codex actively implement the core together. Grok's quota pause and Codex's
planning-only restriction are cancelled. Codex owns the current L1 import-capacity
stage; Grok owns Message exec/D7; Fable owns the graph ABI and L2 frontend.
Claude retains all mixa_manager. Read CORE_TEAM_PLAN_20260912.md for exact file
boundaries, settled model and integration sequence. Ask the user only about a
concrete logical contradiction in the model, not an already answered rule or an
ordinary implementation choice.

Grok completed inbox `20260912-072700.txt`, pushed the evidence correction in
`85f731e`, and released core ownership. That boundary is accepted; start from
that pushed revision while preserving newer accepted commits. Preserve every unrelated
untracked/shared file; never reset, stash, clean, force-push or bulk-stage.

Read first:

- `work_chat/CORE_CONTINUATION_20260911.md`
- `L2_CORE_SELFHOST_HANDOFF_20260911.md`
- `stg/l1_baseline/l2src/LMX_MSG_CONTEXT_V0.txt`
- `stg/l1_baseline/l2src/LMX_MSG_EXEC_HOST_V0.txt`
- `work_chat/TICKET_RULES_EN.md`

The stable compiler is read-only and must remain SHA256
`65D5A5ED127CA1BAEBDD1D500A5B74CEEA63EC1985EAC52EDEF28EFEB261C936`.
Use existing saved evidence and do not rebuild accepted stages merely to repeat
them.

User correction 2026-09-12: address -> short name is only a reference table for
strings, holding source Structure names. It is not a descriptor, execution
identity or runtime binding service. Strings need not correspond to the tree;
short-name collisions/duplicates are immaterial to core execution. Do not add
canonical name IDs or mandatory named/anonymous/positional registration as
construction, merge, copy or call prerequisites. The actual spec/refactoring
EN/RU definitions, acceptance criteria and OPEN_POINTS.txt are corrected in
place. Compiler symbol resolution and exact callable signatures are separate.
Structure.len is exclusively child count. data addresses an ordered array of
void * child pointers; classify each stored address by its membership in the
array/ranges for its type T. The slot address is not the child's type address.
Do not use inline Lmx child records or primitive Lmx wrappers. Only an actual
Structure target has node, len and data. Three child pointers means len = 3;
Array record length is separate. These are settled user decisions.
The existing `l2src/lmx.h` still has an obsolete comment declaring len undecided
and a `size_t len` field, whereas the user's documented header is `int len`.
This source/document mismatch belongs to the core owner. Also audit the current
lmx_branch inline-Lmx storage, child accessors, primitive wrappers, merge and
tracing against the void * child-array contract. Existing tests of inline-Lmx
children do not establish conformance. Correct comments and dependent layout/
access code in a bounded implementation stage before claiming ABI conformance. Do not ask the user to select len semantics again.
Codex has changed documentation only.

Latest merge correction (2026-09-12): merge copies the COMPLETE USED graph,
including the full used lexical tree to node = 0, by the SAME traversal as new
Message creation. Traverse required fields/payload references and node links;
independent supplies a zero root and cuts external lexical surroundings. A
whole-tree use copies the whole relevant tree; unknown use cannot justify pruning.

Destination child/payload pointers AND each copied Structure's node are explicitly
rewritten through the source-address -> destination-address map. Preserve aliases
and cycles; copy used mutable cells, Array records/backing and referenced values.
The result header remains lmx *node; int len; void *data, with ordered void * child
slots and len as child count. Result node identifies the Structure containing the
merge receiver. Required lexical ancestors may be present without becoming extra
visible result fields. Source objects are not overwritten by copying.

The previous pointer-only merge/no-ancestor-copy wording is WITHDRAWN. The A.x/R.x
question based on merge sharing the original mutable cell is also withdrawn:
the used cell is copied with the graph. Plain argument/reference passing remains
separate. Callable stores no node and receives it as an argument; known function
code is reused. Static record arrays may initially remain Message-local. Foreign
resources retain their explicit foreign operation if admitted.

Codex coordinates this urgent source-document correction; Fable owns graph ABI
and frontend implementation and must align merge with Message's copy traversal.
Grok continues Message exec/D7 and aligns the shared copy boundary. Urgent inbox
20260912-090821.txt delivered to Grok, Fable and Claude; user supplied the matching
Fable clarification in this chat. No new language question is needed here.

## Additional settled user rules (2026-09-12)

L2 is a low-level language. Do not introduce L3 ownership, lifetime or mutation
restrictions into its ordinary pointer operations. The answers below are now
part of the three-person implementation under CORE_TEAM_PLAN_20260912.md.

@ is ordinary address-taking in L2, forbidden in L3. Same-name own bind keeps
the same argument variable/address/lifetime and associates it with checkpoint.
The form @: char "hello" is a void * field pointing into the typed char * array,
whose pointers address raw strings. It has no String length or Array descriptor;
a length-bearing String would require a different explicit construction.

Field count/slots are fixed; nested code may change child void * references.
Checkpoint writes dirty values, using ordinary handling when field type changed.
Do not implement active-occurrence move/remove conflict errors. A failing
checkpoint store uses assert. Merge failure reports throws merge(args).

Lexical resolution may follow node parents to zero; independent supplies zero.
Remove the old blanket ban on implicit lexical lookup. Use the existing fn
rules for generated C names and own-node, lexical/dynamic and explicit inputs;
no nested C functions or runtime code generation are needed. Available failure
graphs follow ordinary graph retention/handoff. Remote codecs and budgets do
not block the local core. These are implementation instructions, not questions
to send back to the user.

## Accepted core state

- Matching-parenthesis parser port and reach proof are complete in `d38fae3`
  and `be4e13f`; do not restart them.
- Worker launch/rebind/reaper lifecycle corrections through `ca03089` are
  accepted at the documented Windows/runtime and POSIX compile boundary.
- Owner-ready selection no longer scans `bind[]`; `099ac48` is accepted.
- D7 phase 1 is accepted in `eac736e`: each POSIX Message owns a real checked
  heap-stable mutex; ordinary `send`, `send_owned`, `send_cap` and turn-self
  recv no longer nest the mail lock under exec.
- D7 phase 2 runtime changes are in `1e176de` and corrected by `508e22b`:
  end_turn transfer and pump/admit mailbox mutation occur outside exec; the
  destination lifetime pin precedes done mutation. The stronger `072700`
  evidence correction in `85f731e` is accepted; Grok is paused.

## L1 import-storage stage — completed by Codex

Implemented and verified in b41af667: root and baseline L1 translators now store
paths as existing LmP0Text array/string descriptors (data and length), with
actual-length owned bytes and growing tables. Neither a sixteen-entry maximum
nor a 1040-byte storage cell is a language rule. The former request to select
a new fixed maximum was a coordinator mistake.

24 command checks passed: candidate/self/next generated C equality; 17/65 import
native runs; duplicate/cycle/header behavior; allocation failure preservation,
retry and cleanup; exact MP3 reproducer. Stable65D5 remains read-only. See
L1_IMPORT_CAPACITY_20260912.md for the candidate, hashes and evidence.

The separate old import-depth guard and temporary path-resolution/header-name
buffers remain recorded implementation limitations, not accepted language
limits. Codex's next continuation removes incidental path-buffer limits using
actual-length array/string data. Claude can consume the verified private
candidate for the blocked MP3 composition after his active app checkpoint.

## D7 stage — now owned by Grok

Continue D7 with the remaining ordinary runtime paths, but keep the first Fable
commit bounded to **non-self recv plus fail/stop inbox walks**. Separate mailbox
queue traversal/mutation from the shared exec lock using explicit Message pins
and a single local owner for detached chains. Preserve exact receive FIFO,
status/error results, done tracking, ready membership, stop/fail delivery and
all cleanup. Establish one lock order: never acquire exec while holding mail,
and remove exec->mail nesting from the converted paths including OOM/error
fallbacks.

Do not include `sched_ready`, `release_slot`, `runtime_delete`, retirement,
`bind[]` redesign, parser/compiler work, throw-carrier semantics or u64 overflow
semantics in this first stage. The user decided these on 2026-09-12. Declared
throw uses an explicit C ABI: status return plus typed ordinary-result and
throw-payload out-parameters, with distinct declared-throw/runtime-failure
statuses and Message/activation-owned storage. A throwing assignment skips its
store. Keep `setjmp`/`longjmp` only for the diagnostic `assert` root; never copy
the historical process-static lm2 throw channel. Numeric operations use native
target semantics without a new checked/wrapping language layer: the C backend
follows C exactly, including unsigned `u64` modulo behavior, and VM-native
differences are accepted. Existing explicit memory-size/index/bounds checks
remain required.

Required deterministic Windows evidence:

1. Gate a non-self recv on mailbox A while unrelated mailbox B completes; B
   must not wait for A through exec.
2. Concurrent recv and fail/stop on one inbox must produce exact single
   ownership, FIFO for retained entries, exact terminal statuses, and no
   loss/duplicate.
3. Unbind/release between resolve/pin and detached-chain processing must be safe
   and release the pin exactly once.
4. Inject every new allocation/retain failure and prove exact rollback of queue
   counts, refs, done/ready state and error result.
5. Retain accepted D7 phase-2, owner-ready and reaper regressions.

Also provide a real POSIX `-pthread -Werror` compile object/log; if POSIX runtime
execution remains unavailable, state that exactly. Update the context document
with the remaining nesting inventory. Completion requires exact changed paths,
immutable evidence directory, markers, Git blob IDs, focused commit/push and
limitations.

The former quota rotation is cancelled. After the D7 checkpoint is verified,
Grok continues remaining D7 paths; Fable continues graph/frontend conformance,
with Codex coordinating integration and implementing L1 capacity.
