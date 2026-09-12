# Fable 5.1 temporary core handoff

## Role and clean starting boundary

Fable 5.1 temporarily owns all Lingvamyxa L2/core implementation while Grok is
paused for usage limits. Claude continues to own all `mixa_manager` work. Codex
only coordinates and reviews existing source/evidence read-only.

Do not begin until Grok has completed inbox `20260912-072700.txt`, pushed its
focused evidence correction, reported the immutable evidence path, and released
core ownership. Start from that pushed clean revision. Preserve every unrelated
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
  destination lifetime pin precedes done mutation. Acceptance waits only for
  Grok's active `072700` stronger evidence correction.

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
semantics in this first stage. The latter two remain user decisions.

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
