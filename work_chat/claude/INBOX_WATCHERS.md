# Claude inbox: FileSystemWatcher + 30-minute fallback

Effective 2026-09-11. These instructions supersede the old five-minute timings
and exhausted 12-hour watcher assumptions in WAKING_CLAUDE.txt. Its central
warning remains valid: a filesystem event is not, by itself, an agent wakeup.

## Assignment

Restore reliable monitoring for this existing Claude session:

- Workspace: C:\Nyasha_Planet\lingvamyxa
- Watch only: work_chat\claude\inbox
- Replies: work_chat\claude\outbox
- Claims: work_chat\claude\seen
- Event batching: 20 seconds of quiet.
- Fallback reconciliation: every 1,800 seconds (30 minutes).
- Health pulse: every 30 seconds, even without filesystem events.

Keep OpenCode and Antigravity disabled. Do not modify colleagues' watchers,
models, credentials, permissions, or core compiler/runtime files.

## 1. Inspect before replacing

Read PROTOCOL.txt, ASSIGNMENT.txt, WAKING_CLAUDE.txt and the current handoff.
Inspect your actual background-task list, watcher script, heartbeat and logs.
Do not assume an old PID or "healthy" message is current.

The old heartbeat location is:
C:\Nyasha_Planet\lingvamyxa\build\claude_watcher_heartbeat.txt

Use a durable script in work_chat\claude\scripts rather than a temporary session
scratchpad. Put runtime state/logs in build\claude_watch so they cannot trigger
the watched inbox. If a working script already exists, repair/reuse it.
Do not start a second session or a second watcher just to check the first.

## 2. Connect the helper to THIS session

For the completion-based mechanism documented in WAKING_CLAUDE.txt:

1. Launch the actual PowerShell watcher as your native background shell task.
   Use the background execution option exposed by your current Claude tools.
2. The watcher waits until an actionable batch settles OR the fallback deadline
   arrives; then it prints one compact result and exits.
3. Its completion must produce a task notification in this existing session.
4. On notification, reconcile/claim work and re-arm the watcher before doing
   lengthy implementation. A startup scan catches files arriving during re-arm.

Do not detach it with Start-Process and let its parent command return
immediately: completion of that launcher is not delivery of future inbox events.
Do not assume periodic writes to watcher.log wake an agent.
Do not repeatedly call TaskOutput or inspect status every few seconds.

If your current environment exposes a native streaming Monitor tool, you may
use a persistent helper through that tool instead, but only after demonstrating
that one emitted batch starts an actual agent turn. Use one delivery mechanism,
not both. Do not invent an API or use Codex/OpenCode session pipes.

If native background completion/streaming does not wake this session, report the
exact missing capability. A detached Windows process cannot fix that by itself.

Suggested script interface to implement or adapt, not a claim that it exists:

    powershell.exe -NoProfile -File "C:\Nyasha_Planet\lingvamyxa\work_chat\claude\scripts\watch_inbox.ps1" -PollSeconds 1800 -QuietSeconds 20 -HeartbeatSeconds 30

Invoke PowerShell explicitly if the shell tool is actually Bash. Do not paste
PowerShell syntax directly into Bash. No broad execution-policy change is needed
as part of this watcher task; report a real launch restriction if encountered.

## 3. FileSystemWatcher requirements

Use System.IO.FileSystemWatcher on the exact inbox directory, non-recursively.
Handle Created, Changed and Renamed; also handle Error. In particular, detect
a finished file published by renaming .tmp to .txt.

Only completed .txt requests are actionable. Ignore .tmp files, directories,
editor scratch files and watcher state. Subscribe and enable notifications,
then immediately scan the inbox: do not rely on receiving historical events.

Treat events as "rescan needed", not as exactly-once deliveries. Aggregate a
burst and wait for 20 seconds of quiet before waking. Longer continuous writing
may delay the event batch, which is intentional. It must not starve the health
pulse or the independent 30-minute reconciliation deadline.

Before delivery, rescan actual files and deduplicate by ticket ID and content
hash. A timestamp-only touch must not repeat a completed task. If someone
changes a claimed/completed ticket's content, report the protocol conflict;
prefer a new ticket ID, not silent replay.

On watcher Error/buffer overflow, log the fault and reconcile the directory;
recreate the subscription if necessary. The 30-minute fallback must still work
if event notifications fail. Avoid long work in event callbacks.

## 4. Thirty-minute fallback and health pulse

Use a timer/deadline in the same helper; two triggers do not require two workers.
At each 30-minute deadline, notify the session once to reconcile:

- unseen .txt requests;
- claimed but interrupted/unanswered work;
- the current multi-stage assignment and its next agreed step;
- FSW health and delivery failures.

With nothing actionable, re-arm quietly. Never turn this into a 10-second model
poll or a recurring user-facing "still alive" report.

Keep the fallback due time across event-triggered re-arms so frequent events
cannot postpone it forever. Use elapsed/monotonic time while the process runs,
with a persisted timestamp for restart recovery.

Refresh health every 30 seconds independently of file events. A timed wait is
enough; no busy loop is needed. Record UTC time, PID, process start time, exact
script path, session/background-task identity when available, last event,
last completed reconciliation and current state (waiting/settling/fired/error).

A fresh pulse proves only that the helper runs, NOT that Claude receives tasks
or is making progress. Record last delivered/claimed/replied ticket separately.

## 5. Claims and crash recovery

Only the agent claims work; the watcher must not mark files seen before delivery.
Create seen\<ticket>.txt atomically when taking the task. A seen marker means
CLAIMED, not DONE. Do not blindly delete it or restart work after a timeout.

On startup/fallback, reconcile claims with replies, progress and current tasks.
Resume an interrupted stage from its actual files/evidence. Do not duplicate
an active stage or rerun accepted work. A reply containing only an ACK or plan
does not finish the assignment.

Use one watcher per channel (named mutex or equivalent exclusive ownership).
A duplicate launch should exit with a clear "already running" result, not kill
the existing worker or enter a restart loop.

Publish outbox replies as UTF-8 through a temporary file and a same-directory
rename to the final ticket name. Keep inbox files and history intact.

## 6. Required acceptance tests

Test the helper in isolated directories first, then the actual wake bridge:

1. Publish two uniquely named test requests as .tmp, then rename them to .txt
   in one burst. One settled notification must contain both IDs.
2. Show the actual Claude background notification and the matching outbox
   response(s). A log saying "event received" alone is NOT a pass.
3. Leave an incomplete .tmp file: no request processing.
4. Touch a completed test .txt without changing its bytes: no duplicate work.
5. Restart with an unseen request already present: startup scan finds it.
6. Simulate an interrupted claim: recovery reports/resumes it, not loses it.
7. In an isolated test configuration, shorten the fallback interval and suppress
   FSW delivery. The timer must still cause a scan/notification. Restore 1800
   seconds afterwards; report this as accelerated evidence, not a real 30-min run.
8. Leave the inbox idle and show two advancing health pulses.
9. Attempt a second watcher: only one active instance remains.
10. After handling a notification, show the re-armed task and its live pulse.

Use test IDs that cannot be mistaken for implementation tickets. Do not place
fake completed responses in a colleague's real outbox to manufacture proof.

## 7. Safe repair and ongoing work

Prefer a graceful stop marker. Before stopping any process, match PID, creation
time and exact script path; do not kill by a broad "powershell/watch" substring.
Never kill Claude, Grok, Codex, OpenCode servers or unrelated background jobs.

If the helper is alive but this Claude session cannot receive notifications,
describe that separately. If the session is quota-blocked or closed, say so:
FSW cannot restore model quota or guarantee session restart.

Finish with an English report: script path, current task/PID, actual intervals,
test evidence, receipt/re-arm proof and any remaining limitation. Then continue
OPENCODE_HANDOFF_20260911.md. Watcher setup is not a reason to stop after a plan.

Implementation references (verify against the tools available in THIS session):
- https://code.claude.com/docs/en/interactive-mode
- https://learn.microsoft.com/en-us/dotnet/api/system.io.filesystemwatcher
