# Model selection for background inbox turns

User authorized Codex to manage model choices. Current user-selected baseline:
`opencode/claude-haiku-4-5`, Default (no explicit variant sent).

`model-selection.json` is read by the inbox watcher at each dispatch. Change
providerID/modelID and optionally variant there to select the NEXT notification
turn without restarting the agent/server/watcher. Invalid configuration blocks
delivery rather than silently choosing another model. This does not change a
currently running turn or the interactive TUI's selection.

Do not switch models mid-task, create duplicate sessions, enable auto-reload,
buy credits, or connect another provider as part of routine orchestration.
Confirm catalog availability and cost before selecting a different model.
Complex core L2 tasks remain with Grok; OpenCode owns bounded app/test tasks.

2026-09-10: Codex verified actual session messages already used Haiku, tested
explicit model/default payload with an offline HTTP mock (no billable request),
and restarted only the inbox watcher to load this support. Agent/server stayed
running. Pending tickets are rechecked against seen/outbox before dispatch.
