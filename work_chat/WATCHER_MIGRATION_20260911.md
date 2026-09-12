# Codex watcher migration verified, 2026-09-11

Current task: `01a092e9-eb6e-7d61-8880-c0f88197317a`.
Previous task: `01a078c2-38ba-78c3-b5a4-699eb69b230d`.

The existing heartbeat `lingvamyxa-grok-bot-5` was retargeted through the app
automation tool, remains ACTIVE every 30 minutes, and was viewed again after
the update. The saved target agrees. No second automation was created.
The prompt and FSW continuation now use the restart plan and Grok/Claude scope,
continue agreed implementation, and stay quiet for routine/unchanged progress.

Local infrastructure remains
`C:\Users\mtkra\.codex\automations\lingvamyxa-grok-bot-5`.
The verified old helper PID 10460 was gracefully stopped before rearming.
Production PID 21772 started at 21:02:06.302 local (America/Sao_Paulo), with
the exact `watch-outboxes.ps1 -ConfigPath watch-config.json` command in that
directory. Configuration targets the current task and only Grok/Claude outboxes,
20 seconds of quiet per batch, 30-second health pulse, testMode false.
Independent waiting pulses advanced at 00:02:39, 00:03:09 and 00:03:39 UTC.
The health pulse is not a model wake; the model fallback remains 30 minutes.

## Delivery acceptance

Isolated evidence: `migration-test-20260911-210200/state/events.jsonl` under
the infrastructure directory. Same scripts/bridge, isolated folder/state/mutex.

- Two `.tmp` to `.txt` publications produced one two-file batch after 20 seconds.
- Batch `bde9b7cc48b34b05a3142eea1d06cf34` was actually received in the current
  task; the bridge logged successful app delivery at 00:02:57 UTC.
- The incomplete `.tmp` and timestamp-only touches produced no extra batch.
- Duplicate helper launch failed at the lifetime-held mutex.
- Quiet health pulses advanced independently; one batch, one send, zero errors.
- The isolated helper PID 9344 stopped gracefully at 00:04:07 UTC. Production
  remained healthy, confirmed again at 00:05:09 UTC.
- A file created before startup was seeded without an event and found by explicit
  reconciliation. Restart gaps still require the heartbeat's mailbox/hash review.

No actual 30-minute scheduled firing has yet been observed. Scheduler acceptance,
FSW delivery, colleague availability and completed implementation are separate.
No error was injected into the app bridge; failed-send handling is unchanged.

## Colleague continuity

Published English tickets `20260911-210200.txt` atomically in both inboxes.
Grok retains collector ownership; his 205056 reply reports the LmxVisit local
declaration blocker. Codex retains the seven known-character-read files agreed
in 202400. The ticket points to the existing L1 visit header as a concrete lead.

Claude's ticket queues `NEXT_CLAUDE_TICKET_FILE_SHARE_20260911.md` after MP3
closeout/eight-file recovery in 202547, with the entire app backlog preserved.
At publication those pending tickets had no Claude claim/reply; his own watcher
state was still fired at 23:13:50 UTC. MP3/recovery completion and his inbox wake
are not established by Codex's working outbox watcher. Rearming is requested
through his existing session, without launching another model.

OpenCode/Antigravity remain disabled; grok_bot remains paused. No colleague
process was stopped, and no unrelated work was staged or reset.
