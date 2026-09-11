# Implementation tickets and acceptance

These rules apply to Codex as coordinator and to active collaborators. They
clarify the existing implementation request; they do not broaden authority,
override file ownership or authorize a new architecture. A request to implement
already means working code, not merely a plan.

## Four status rules

1. **Plan or acknowledgement: WORKING.** Neither closes the task. Continue the
   authorized implementation. A `seen` marker, a delivered reply, or a live
   watcher is not evidence that implementation is complete.
2. **BLOCKED:** name the exact obstacle, the attempts and their observed results,
   and the smallest decision, dependency or help required. Continue independent
   authorized work where possible. Do not describe an unexplained wait as a
   blocker. Ask rather than inventing language semantics at a genuine fork.
3. **DONE:** identify the changes, actual verification and evidence, commit/push
   status, and remaining limitations. Never claim an action before performing
   it. Failed checks, missing integration or a partial implementation must be
   stated explicitly; completing a bounded module is not completing the project.
4. **STAGE DONE:** checkpoint the verified stage, then proceed to the next
   already agreed stage without waiting for a reminder. If it needs another
   owner's integration, hand off the exact artifact/API and take an independent
   agreed slice. If no authorized next step exists, ask one concrete question
   rather than manufacturing work or crossing ownership boundaries.

## A short ticket needs

- Objective and reference behavior / agreed decisions.
- Exact owned files or module boundary; dependencies and files not to touch.
- Observable acceptance checks, with failure cases proportional to risk.
- Deliverable: changes, test evidence, focused commit/push, limitations.
- The next agreed stage or explicit handoff recipient and boundary.

Reference these rules instead of repeating an expanding list of anti-idle
phrases in every ticket. A queued stage must not interrupt active work.
Do not issue duplicate tickets or request acknowledgement-only replies.

## Coordinator acceptance and continuity

Read each reply against its request. Classify its substantive status instead
of treating an outbox file as completion. Verify the claimed diff, commit and
saved evidence; preserve colleagues' active checkouts/builds. Claiming tests
passed requires successful exits for the stated source revision, not an old
green log or a still-running process.

On completion, activate the next queued stage; an empty inbox with authorized
work remaining is task starvation, not a worker failure. On unexplained lack
of progress, ask once for the exact current step/blocker and follow up using
evidence. A heartbeat, acknowledgement or elapsed time alone proves neither
progress nor a stall. Inspect advancing logs, source changes and actual running
commands. A failed test stuck in cleanup is a concrete failure to diagnose,
not an excuse to send repeated "are you alive?" messages.

Keep unfinished scope explicit. Commit/push each verified stage using only
owned paths; never include unrelated shared changes. New architecture, unsafe
cleanup, paid services, or changes outside the agreed task need their own
authority. Do not create extra tests or refactors merely to appear busy.
