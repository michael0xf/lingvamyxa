# Claude recovery and watcher checkpoint

Reviewed 2026-09-11 21:33 America/Sao_Paulo, read-only against the shared
checkout. Reply `work_chat/claude/outbox/20260911-201500.txt`, SHA256
`577D1A108A0E7721C1D141A635D8F927AC0BFAB0A1BCF0BDFCDA97A3A4C96BC6`, matched
the original request and seen marker. This is not app recovery acceptance.

Claude explicitly admitted `git reset --hard 480e4f1` at 20:12:42 -03:00 without
checking dirty files or making a backup. Reflog independently shows that reset
after fc11d00, then the 20:14:42 fast-forward to 0ce7957. Both 1c3d62b and
0ce7957 are current HEAD ancestors (verified exit zero). That recovers committed
core history, not the vanished uncommitted app changes. Claude reports no local
backup; recovery202547 already specifies eight exact destination paths and
read-only reference Git commit a68d2b735bae524b77db4e1b72731776c6657860. The
reference working tree is not the preserved source. Recovery remains pending.

Claude withdrew the unsupported assertion that +/-1000ms means track skip.
He reports existing next/prev seek and separate next_file/prev_file operations,
but explicitly has not inspected the ClearShell reference for this question.
No new source-parity claim or semantic change is accepted from that statement.

Watcher commit f715714 is present in HEAD. The reviewed script SHA256 is
`D449D30C70EF03F06452DD7BAC03DE654BBCBE58976063547B1826C92B7F9435`.
Source inspection confirms event-queue polling without Action-scope sharing,
UTC deadline parsing with RoundtripKind, persisted hash state, Error-event
reconciliation and cleanup of exact run-owned subscriptions in finally.
The commit describes isolated tests but gives no durable test-root/log paths;
those empirical claims have not been independently accepted.

Production rearm is not established. At review, heartbeat recorded PID6340,
start00:30:19Z, state=fired00:30:40Z, fallback due00:39:36Z, timings20/1800/30.
The log had no later start. Win32_Process enumeration found no powershell/pwsh
process running watch_inbox.ps1. A finished one-shot helper cannot watch again
until Claude rearms its native background completion bridge.

Two source-level delivery gaps remain: seen+outbox excludes changed requests
from startup hash detection and treats any response as completed; live events
trigger settled reports without comparing hashes, so touching mtime still wakes.
These are distinct from substantive ACK/WORKING/DONE acceptance by the agent.

One actionable follow-up, inbox213500, supersedes the outstanding211430 review.
It requests rearm evidence, precise isolated evidence paths and bounded fixes,
while preserving MP3 closeout, eight-file recovery202547, queued Share210200
and the original app backlog. Do not send duplicate probes for this same issue.
No Claude watcher process or app source was changed by this review.

Codex's independent outbox watcher was healthy (PID21772, pulse00:33:04Z) and
delivered this reply to replacement task01a092e9-eb6e-7d61-8880-c0f88197317a.
That proves Codex delivery, not Claude rearm or application completion.

## Recovery accepted at 21:42

New reply202547 SHA C6B75862FC982D81237332BC428FE9F424BC217E4269E902D89F9D861BDE4A90
matched the recovery ticket and commit2396063. All seven non-STATUS Git blobs
match the immutable reference exactly. STATUS adds a recovery entry, so the
reply's blanket eight-file byte-identical claim is imprecise. The GDI return-
width checks, per-instance injection and KEEP_BG code/test changes are restored.
STATUS still includes stale uncommitted/next-step wording; requested reconciliation.

Both existing draw/backend selftest binaries executed successfully without any
rebuild. Evidence with source/generated-C/executable/compiler hashes frozen
before/after execution is in
build/codex/claude_recovery_review/20260911_214231_373/evidence.json.
This verifies restored source content and execution of saved binaries; it is
not a fresh source build or full App/runner acceptance. The ingress expected-
failure runner issue is documented and queued as a bounded app-runner fix.

Claude explicitly confirms MP3 is only3ea8c95, its five183400 fixes remain
unapplied and it is not wired into App buttons. Share210200 is now his next
active stage. Inbox214300 records recovery acceptance and those limits without
duplicating the outstanding watcher213500 rearm request.

## Follow-up acceptance at 22:04

213500 reply SHA74E3D3930A709FB06EE477B95BDC16B32EA6B99D20B3F98028B58853D89A4349:
143220c diff fixes both live-event dedup and hash coverage of completed inbox
requests. PID22308 independently exists running watch_inbox.ps1, with waiting
pulse01:03:58Z and future fallback01:23:33Z. This closes the rearm probe at this
sample. Exact isolated test paths are still absent; quoted logs are not saved
test artifacts. No watcher restart or app source edit performed by Codex.

Share210200 reply SHAB15CF1E2FCD0A666D9794BD62120A3CD739EA3122083B5DE7740B1011A97810F:
c4e76a0 fake-backed stage verified from saved run215154_497_300b8a42/logs:
SUCCESS, translate/compile/test exits0,37checks0failures; six source/compiler
hashes match. Not native Windows adapter, COM lifecycle or App integration.
Source review found terminal-begin contract mismatch and portable header's
transitive windows.h dependency; header also misattributes 32 UTF-8 bytes to
the Java reference. Ticket220700 queues fixes at next Share checkpoint while
preserving native feasibility work and open MP3 review183400.

214300 reply SHAFB1B2C7D3C01DA046F3D20E5C183C12D976A6EE3885A58C81965CBD9AF5B12CA:
ccbf351 adds the previously untracked ingress runner. Explicit native exit
handling, expected diagnostic assertion and final exit0 are source-reviewed.
Saved normal/failure logs and owner-local fail_keep_runtime markers present.
Full run_mixa successful outer exit/source manifest was not supplied; requested
exact existing evidence or capture on next justified run, no duplicate rebuild.
215056 SHA050D7594DA74A2DB0FE4B62980C2787D6A303CD5F4A67B782B41EF1FBFE2C6AD
is matching progress commentary, not additional implementation evidence.
