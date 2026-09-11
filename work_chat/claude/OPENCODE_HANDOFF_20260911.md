# OpenCode -> Claude: complete app handoff, 2026-09-11

## Authority and scope

The user has moved all remaining OpenCode work to Claude (reported Haiku 4.5).
OpenCode is quota-blocked and its monitoring is explicitly disabled. Do not
restart it, change models or spend paid balance. This is an ownership handoff,
not a request to wait for OpenCode to return.

Claude owns the mixa_manager app stream. Codex and Grok continue the independent
Message/L2 core stream. Coordinate through this Claude channel in English.
ASSIGNMENT.txt supersedes the old simple-tasks-only/quota-pause restriction.
This document supersedes old workflow/owner claims in STATUS.txt and old ticket
freezes; it does not supersede the current product specification.

## Complete correspondence, with no lost tickets

Local snapshot: work_chat/claude/transfers/opencode_20260911/

- inbox/: 101 files, including 100 final .txt tickets;
- outbox/: 85 files, including drafts;
- seen/: 97 claim files;
- manifest.json: 283 file hashes verified against the original on 2026-09-11.

OPENCODE_TICKET_INDEX_20260911.md indexes all 100 final tickets. The snapshot is
gitignored local correspondence; originals remain untouched in OpenCode's channel.
It contains no model credentials/server configuration. A .tmp reply is NOT a
published answer. A claim or an ACK is NOT evidence that a task is finished.

Read this queue first, then only the original request/review/reply needed for
the current stage. Do not load the entire correspondence repeatedly. Old pings,
superseded constraints and accepted micro-fixes are not a hundred new tasks.
If another genuinely unfinished stage is discovered, add it to this queue and
tell Codex; do not silently discard it.

## Working contract

All application algorithms and new tests are L1; new headers are name.h.lm1.
Do not substitute handwritten C to get around an inconvenient language feature.
Approved foreign libraries and narrow OS ABI seams are exceptions, not a route
for business logic or app-private synchronization.

Use the stable compiler read-only:
stg/l1_baseline/build/l1trans/gen2/l1trans.exe
SHA256: 65D5A5ED127CA1BAEBDD1D500A5B74CEEA63EC1985EAC52EDEF28EFEB261C936

Use unique build/mixa/claude stage outputs. Do not run shared self-build gates,
rewrite stable gen2, or edit active stg/l1_baseline/l2src files. Build once per
focused module checkpoint and reuse the result across its test cases.

Before editing, inspect git status and exact path diffs. Existing dirty backend,
draw, run_mixa.ps1, STATUS.txt and their tests are NOT implicitly free to overwrite.
Ask Codex for exact overlap ownership; work on clean App/audio paths meanwhile.
Preserve untracked modules, tests, vendor artifacts and recovery files. Never
git add -A, reset, stash away or delete colleagues' changes.

After every verified stage: commit and push exact owned paths, report changes,
test evidence, commit and limits, then continue the next agreed stage. Planning
and acknowledging mean WORKING. A blocker specifies the exact step, observed
error, attempted checks and needed decision. Do not stop at a proposed plan.

## Product source of truth

Current app requirements/deltas take precedence over ClearShell where different.
Read README, PORT_OF_CLEARSHELL.txt, UI_MODEL.txt, FIRST_VERSION.txt, FILE_SEAM.txt,
PROCESS_SEAM.txt and CODING_RULES.txt as needed for the current module.
SPEC_DIGEST.txt and REVIEW_20260911.txt carry useful investigation, not permission
to invent language behavior.

ClearShell reference is read-only; use mixa_manager/clearshell_reference (or the
user's equivalent C:\Nyasha_Planet\git\gitlab_lmx\mixa_manager\clearshell_reference).
Port its existing algorithms and button/panel behavior, not a generic manager.

Preserve reference colors and background transparency; text is fully opaque.
Bottom button panels stay fixed during vertical list scrolling. Each button
panel has independent horizontal scrolling; each list has vertical scrolling,
and a shared horizontal scroller moves the list windows. Input layer 1 lies
under layer 2. Missing features open a nested instance of the same app interface
with "нет функции" and one dismiss button, not a native MessageBox/console line.
Extra buttons are not a defect; F1 Help is one known addition, not a whitelist.

## Queue 0 -- close the nested-window stage without rewriting it

Original 20260911-153700; review 20260911-163600.
Base dc5a18d; correction 84eacef is already committed.
The correction reply exists only as outbox/20260911-163600.txt.tmp.

Codex verified read-only on 2026-09-11:
build/mixa/opencode/app_window/run_20260911_182557_329_fdb6638f/logs/
Header, implementation, test and runner hashes match the current files. Header
translation, translation, compilation and test exits are zero; test output:
9 cases 180 checks 0 failures.

Accepted bounded correction: reject message/button wider than saved interior;
restore parent after partial paint failure; arithmetic overflow guards.
Record this accepted evidence in your handoff response; no redundant build.
This proves the module stage, NOT integration into an interactive application.
Preserve its five files and later wire it into actual button actions.

## Queue 1 -- App / shortcuts: first coding stage

Original 20260911-154200; base cc6c44b; OPEN review 20260911-172200.
Read all three before editing. The old 63-check pass does not close the review.

Owned stage paths:
mixa_manager/mixa_app.h.lm1
mixa_manager/mixa_app_win32.h.lm1
mixa_manager/mixa_app_win32.lm1
mixa_manager/tests/mixa_app_selftest.lm1
mixa_manager/tests/mixa_app_fixture_invoke.c
mixa_manager/run_app_selftest.ps1
mixa_manager/mixa_app.txt
and a .lm1 replacement for the test fixture.

Required fixes:
- entry_to_text silently truncates through char line[128]; checked full-string
  appending, >127-character path/label round trips and Unicode coverage.
- Enumeration must distinguish FindNextFileW END (GLE18) from real failures,
  with deterministic invalid-handle/error coverage.
- entry_parse must length-check before the 17-byte header memcmp; test empty,
  one-byte and truncated inputs.
- Replace handwritten C fixture logic with L1 using the pinned compiler. Report
  an exact minimal compiler blocker if real; do not declare COM impossible.
- Keep the common API independent of Win32 types/includes through opaque
  handles/platform seams. Preserve original .lnk/.url launch references rather
  than flattening away target arguments or working directory.

Use test-owned fixtures only; do not invoke arbitrary installed apps or change
associations/start-menu settings. Registered/packaged identities and non-Windows
backends must have honest unsupported boundaries until implemented.
After module corrections, App button integration remains part of the original
task. Do not call the whole App feature done after the standalone selftest.

## Queue 2 -- MP3 / audio corrections, then button integration

Original 20260911-154950; base 3ea8c95; OPEN review 20260911-183400.
The old fake 67 / native 23 checks are bounded evidence, not completion.

Stage paths: mixa_audio.h.lm1, mixa_audio_win32.h.lm1, mixa_audio.lm1,
mixa_audio_win32.lm1, tests/mixa_audio_selftest.lm1,
tests/mixa_audio_native_selftest.lm1, run_audio_selftest.ps1,
run_audio_native_selftest.ps1, mixa_audio.txt, and only the relevant
PORT_OF_CLEARSHELL.txt section (all under mixa_manager).

Required fixes:
- Remove process-mutable g_mixa_audio_alias_seq and lazy global vtable/ready
  state. Own state per instance/Message; use native handles or OS identities.
  No singleton lock/atomic/once workaround.
- set_list/clear during playback must reconcile controller/device state.
  Preserve prior state if preparation fails; stop/unload correctly; report
  cleanup failures honestly. Add deterministic fake replace/clear/failure tests.
- Successful MCI open followed by failed time-format setup must close acquired
  resources. Validate command lengths and termination.
- Remove invented MAX_LIST=64; check allocation overflow and test >64 items.
- Preserve ClearShell's previous/next +/-1000ms behavior. It is not track skip;
  next_file/play_next are separate operations. Correct misleading old prose.
- Keep the common API OS-independent and backend limitations explicit.

Native smoke uses silent generated fixtures, not user media or system-volume
changes. Later MP3 UI wiring/event polling uses Message; module tests alone do
not prove concurrent integration.

## Queue 3 -- file manager itself, all original operations and panels

Overarching OPEN task 20260911-100100 and its follow-ups.
The directory seam stages 0fec820/cd0f19e are accepted low-level enumeration/error
work, NOT completion of the actual file manager.

Inventory already ported modules, then port the missing ClearShell operations,
selection-aware copy/move behavior, navigation, panel/button algorithms and their
tests in bounded stages. Implement independent algorithms now; do not wait for
Message. Wire completed modules into the actual application once the relevant
interface is available. Missing operations use the nested "нет функции" window.

## Queue 4 -- application integration, process/console and concurrency

Earlier 032400/033200 maps are plans, not implementation proof. In the reviewed
state, mixa_pump_next/drain appeared only in tests and run_mixa.ps1 was a harness
runner, not evidence of a running interactive application. Verify current source
before choosing the next integration cut.

Remaining integration includes nonblocking jobs/output, file-backed console,
button dispatch, nested windows, App and MP3 modules. Current FIRST_VERSION and
PROCESS_SEAM line-start-to-cursor behavior is intentional where it differs from
the original whole-line behavior.

Use the new approved Message runtime when Grok hands it over. Do NOT wire the
rejected lmx_message_exec global-manager prototype or introduce application C
threads/locks/atomics/shared mutable state. Message owns its state and direct-child
scheduling. OS ingress/harness synchronization is a distinct, explicit seam.

Before integration, address the vendor pin staleness trigger from Claude's
REVIEW_20260911.txt; do not silently repin to an arbitrary moving runtime.
For shared dirty UI paths agree exact ownership with Codex first; this does not
block the clean module corrections above.

## Queue 5 -- calculator later, not ahead of the file-manager work

Syntax task 20260911-091600 and recovery/acceptance 111600/112300 are completed
bounded parser work (43 checks). Preserve the existing accepted syntax.

Evaluator/decimal task 094800 remains future work. Do not convert decimal through
double or add fixed epsilon arithmetic. Use the approved core decNumber backend;
the existing third_party C library is an authorized dependency, not something to
rewrite in L1. Coordinate the shared decimal seam with Codex/Grok before editing
it. Keep lexical numeric spans intact. Unsupported variables/functions must be
reported, not guessed. Prefix degrees/postfix radians behavior is intentional.

## Preserve accepted earlier stages

- Pointer-bounds 190355 accepted at 221753: 8 scenarios, 196 member / 110 API
  checks, zero failures. Preserve currently untracked accepted files.
- Highlight/restore accepted 194700: 8160 member / 22 API checks; frozen result.
- Process-marker 222033 accepted 233417: 26 cases / 102 checks; preserve files.
- Console-window 052600/053300 follow-ups accepted 090900/091300 after runner
  corrections. Module evidence is not a finished editor or app integration.
- Directory error correction cd0f19e is accepted; do not redo it blindly.

Keep their sources, logs and focused tests. Do not restart historical diagnostics
or force every old test into each new mandatory build.

## Superseded/non-work correspondence

155100's button restriction is superseded by 155200: do not police a whitelist.
162400 ticket-state rules remain applicable (ACK/plan != DONE).
033000/033500/033900 are architectural map/context updates.
Repeated greeting/status probes are history, not separate coding assignments.
195900 source incident was recovered in 204000; preserve the recovery record.
Old no-git freezes for completed staging do not override the user's current
instruction to commit/push verified owned stages.

## Immediate next actions

1. Read this queue, claim the new Claude handoff ticket and verify/re-arm your
   inbox FSW + 30-minute fallback according to INBOX_WATCHERS.md.
2. Record acceptance of window 84eacef from the matching existing evidence.
3. Implement App review 172200 now; publish an actual tested commit, not just ACK.
4. Continue audio review 183400 and the remaining app queue without prompting.
