# Fable 5.1 temporary core handoff

## Role and clean starting boundary

Fable 5.1 temporarily owns all Lingvamyxa L2/core implementation while Grok is
paused for usage limits. Claude continues to own all `mixa_manager` work. Codex
only coordinates and reviews existing source/evidence read-only.

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

Merge representation is also settled: fields are stored in the exact order of
the `merge:` body. The physical value has only `lmx *node; int len; void *data;`;
`node` points to the Structure containing that receiver. Merge copies the void *
child pointers. Referenced objects and any Structure node pointers stay unchanged, so ordinary merge does not copy ancestors, reparent
or rewrite the tree. Do not add a lexical-skeleton or parallel membership
representation. Separately, when copying a value into another Message/arena,
copy its complete used closure into the destination, following required edges
and lexical `node` links until zero. `independent` stops the lexical walk with
its zero root. A whole-Structure use copies the whole relevant tree; never prune
the used closure merely because some branch appears unlikely. Copy mutable cells,
Array descriptors, Array backing and reference-valued elements as parts of that
closure, using an operation-local old-address to new-address map so aliases and
cycles keep their shape and no language-owned reference points back into the
source Message. Functions are already known; their compiled bodies are not
recursively copied. Moving their static `{addr,sig}` array (like other static
arrays) outside Messages or into an immutable Message is an optional arrangement
that the user explicitly allowed deferring. Do not treat it as a prerequisite or
replace the initial Message-local storage merely because the previous handoff
overstated this option. Raw OS handles are foreign-profile objects and need an
explicit foreign operation if they are ever admitted; they do not limit complete
copying of the language-owned tree.

## Additional settled user rules (2026-09-12)

L2 is a low-level language. Do not introduce L3 ownership, lifetime or mutation
restrictions into its ordinary pointer operations. The user will ask Fable to
review the answers after the documentation update; this text records answers,
not a new dispatched implementation or review ticket.

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

## First Fable stage after Grok releases ownership

Before continuing D7, remove the concrete L1 import-set capacity blocker exposed
by Claude's accepted MP3 components. The stable65D5 executable stays read-only;
produce a candidate translator from the canonical source using the existing
bootstrap workflow.

`l1_imp_act` and `l1_imp_done` are currently fixed at 16640 bytes, and
`l1_imp_push` rejects `n >= 16` while indexing 1040-byte paths. Do not merely
raise one magic number or confuse distinct imported-file count with the separate
maximum nesting-depth guard. Introduce one checked, coherent capacity contract
for both active and completed path sets, update every allocation/bound/index
site, preserve cycle detection and duplicate suppression, and fail cleanly on
real exhaustion/oversized paths. Do not alter import resolution or header type
registration semantics.

Acceptance must include Claude's exact seven-predef reproducer from
`mixa_manager/mixa_audio.txt`: the previous 17th-distinct-file failure must
translate successfully with the candidate. Also prove duplicate imports remain
deduplicated, a real cycle is still rejected, nesting beyond the documented
depth is still rejected independently, and the new maximum distinct-file
boundary succeeds at capacity and fails cleanly at capacity+1 without overwrite.
Run the smallest appropriate translator/bootstrap regressions and record source,
candidate and stable compiler hashes. Commit/push only owned core translator,
test, runner and documentation paths. This stage must not modify `mixa_manager`.

After Codex accepts this compiler checkpoint, Claude may compose scan+launch
against the new pinned candidate and Fable proceeds to the D7 stage below.

## Second Fable stage

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

After this first Fable stage is reviewed and accepted, Codex will issue the next
bounded stage. Grok receives ownership again only at a later clean committed
boundary chosen for the next quota rotation.
