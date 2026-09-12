# Codex restart: watchers, ownership, core and mixa_manager work plan

Current merge rule, user correction 2026-09-12: merge copies the complete USED
lexical tree/graph to node = 0 by the SAME traversal as new Message creation.
Copy used fields/payload and explicitly remap node and references through the
copy map; independent cuts external lexical surroundings. Pointer-only merge
and the ban on ancestor copying are withdrawn. Read current SPEC 2.3 and ABI
8.2 before acting on old checkpoints. Plain non-copying arena handoff is separate.

Eternal-branch clarification, SPEC 9.1.4: the OS-root Message retains an ARRAY
of independent: const: immutable branches until process exit. It need not include
them in its visible graph/lexical tree. Branch node=0; references occur where
declared and in other explicitly chosen graph locations. Retention never
reparents them. Merge/Message creation keep admitted eternal branch addresses;
ordinary mutable state still copies. Other Messages do not automatically see
the retention array or all root settings. Do not revive the old blanket ban on
immutable sharing, or treat const alone as eternal lifetime. Implementation and
acceptance are pending; this paragraph is not evidence of a completed runtime.


CURRENT OWNERSHIP (2026-09-12, latest user instruction): Grok, Fable 5.1 and
Codex actively implement the core together. Grok's quota pause and Codex's
planning-only restriction are cancelled. Codex owns the current L1 import-capacity
stage; Grok owns Message exec/D7; Fable owns the graph ABI and L2 frontend.
Claude retains all mixa_manager. Read CORE_TEAM_PLAN_20260912.md for exact file
boundaries, settled model and integration sequence. Ask the user only about a
concrete logical contradiction in the model, not an already answered rule or an
ordinary implementation choice.

Prepared 2026-09-11, America/Sao_Paulo, for moving coordination into a NEW Codex
chat. This is a detailed handoff and implementation plan, not a new language
specification. Current user decisions and the current spec take precedence over
older summaries and proposals. Recheck facts that can change while colleagues work.

## 0. Start here; do not reconstruct the project from scratch

Migration checkpoint: the new Codex task is `01a092e9-eb6e-7d61-8880-c0f88197317a`.
The heartbeat and FSW now target it; actual test delivery and remaining limits
are recorded in [the migration report](work_chat/WATCHER_MIGRATION_20260911.md).
Older PID/target descriptions below remain the pre-migration snapshot.

The user suspects the old long chat may be malfunctioning and wants a new one.
The migration itself is not a request to redesign the project or stop colleagues.

1. Work in `C:\Nyasha_Planet\lingvamyxa`; read this file and the root share ticket
   `NEXT_CLAUDE_TICKET_FILE_SHARE_20260911.md`.
2. Inspect Git branch, status, index and recent log. Read any applicable AGENTS.md.
   At this snapshot the branch is `main`, remote is
   `https://github.com/michael0xf/lingvamyxa.git`, latest observed code commit
   `18bbf81` (Grok's collect_block port). Numerous app files are untracked.
   Do not delete or stage them wholesale; they include real implemented modules.
3. Read the most recent Grok/Claude replies with matching inbox/seen requests,
   then their active work ownership. Do not read the entire historical mailbox.
4. Migrate the Codex heartbeat AND the FSW delivery target to the new chat as in
   sections 3–6. A process that still wakes the old chat is not restored monitoring.
5. Tell Grok and Claude in English that coordination moved to the new Codex chat,
   without interrupting their active jobs or asking them to restart completed work.
6. Resume the agreed Codex char-read slice (section 10) and Grok's Message lane in
   parallel with Claude's app work. Coding is authorized, not just coordination.

No core/app implementation was changed merely to prepare this handoff. Preparing
this file does not prove a new chat has received an FSW event or that watchers
have already migrated. It also does not prove the full Message or L2 milestone.

## 1. Goal, division of labor and completion criteria

The agreed long-term deliverable is the entire project implemented in L2 and
fully self-hosting through **L2 -> L1 -> C -> native executable**. L2 itself is
C-bound; do not revive an alternate L2-direct-to-C frontend. The L1 translator
must ultimately also have L2 source. Handwritten L1 remains only where it is
deliberately part of L1 tests/intermediate-tool support, not as a permanently
unported replacement for the promised L2 core. Generated L1 is the intermediate.

Preserve the old working bootstrap algorithm: a minimal, tracked portable C
snapshot builds a seed on another platform; that executable can build the LM
sources, perform `next`, `check`, tests and the same further self-build. Historical
binaries alone are not a portable bootstrap and are not the final oracle.

Current priorities:

1. Finish the Message-first concurrent runtime and Message-owned graph/method
   machinery without drifting into a conventional global C runtime manager.
2. Hand the accepted Message interface and an explicitly verified pin to the app.
3. Complete the required L2 semantics and port remaining project components,
   including build tools, parser and translators, toward genuine self-hosting.
4. In parallel, port the actual ClearShell-based mixa_manager in modular L1 and
   integrate it through Message. Do not wait for Message to implement pure modules.

Owners:

- **Grok** (`work_chat/grok`): complex core, `lmx_message*`, lifecycle, scheduling,
  mailbox integration and agreed L2 work. Delegate substantial independent work
  to him; the user values his code/spec reviews and lower usage cost.
- **Codex**: actively code alongside Grok in explicitly agreed non-overlapping
  files, particularly storage/range/visit helpers and compiler integration slices;
  integrate evidence, ask semantic questions early, keep progress moving.
- **Claude / Cloudy / Cloude** (`work_chat/claude`, not a new `cloude` directory):
  now owns ALL remaining mixa_manager work transferred from OpenCode. The user
  reports Haiku 4.5 and a working session/log. Reconcile actual work, not the model
  name. Claude also has valuable historical/spec knowledge.
- **OpenCode** and **Antigravity**: monitoring explicitly DISABLED. Do not restart,
  query APIs/models, send tickets or spend paid balance without a new request.
- **grok_bot**: quota-paused, no active assignment from this restart. Do not
  reactivate it just because old configuration still names its outbox.

English to colleagues; Russian to the user. Use the existing file mailboxes.
Do not spawn extra internal agents as a substitute for the requested colleagues.

## 2. Paths and source-of-truth map

| Purpose | Exact location / interpretation |
| --- | --- |
| Working repository | `C:\Nyasha_Planet\lingvamyxa` |
| Current L2 frontend/runtime development | `C:\Nyasha_Planet\lingvamyxa\stg\l1_baseline\l2src` |
| Active L2 frontend source | `stg/l1_baseline/l2src/l2trans.lm1` — L2 translator WRITTEN IN L1, not the L1 translator |
| Root L1 development sources | `C:\Nyasha_Planet\lingvamyxa\l1src` |
| Stable build-root / snapshot sources | `C:\Nyasha_Planet\lingvamyxa\stg\l1_baseline\l1src` |
| Stable compiler used by colleagues | `C:\Nyasha_Planet\lingvamyxa\stg\l1_baseline\build\l1trans\gen2\l1trans.exe` |
| App | `C:\Nyasha_Planet\lingvamyxa\mixa_manager` (actual spelling is mixa) |
| Main language specification | `C:\Nyasha_Planet\lingvamyxa\Lingvamyxa_spec.txt` |
| L1 specification | `C:\Nyasha_Planet\lingvamyxa\L1_spec.txt` |
| Additional language docs | `C:\Nyasha_Planet\lingvamyxa\include_languages` and `Lingvamyxa_handoff_notes.txt` |
| Parser reference | `C:\Nyasha_Planet\lingvamyxa_old_worked_version` — preserve as oracle; inspect its exact state before use |
| Secondary recovery repository | `C:\Nyasha_Planet\git\gitlab_lmx` — read-only recovery source, see section 15 |
| Current ClearShell reference | `mixa_manager/clearshell_reference/com/mtk/shell/ClearShell.java` and adjoining classes |
| Codex isolated artifacts | `C:\Nyasha_Planet\lingvamyxa\build\codex` |
| Claude isolated artifacts | `C:\Nyasha_Planet\lingvamyxa\build\mixa\claude` |
| Historical accepted app artifacts | `build/mixa/opencode` (reading saved evidence is not restarting OpenCode) |

Stable compiler SHA256 checked on 2026-09-11:

`65D5A5ED127CA1BAEBDD1D500A5B74CEEA63EC1985EAC52EDEF28EFEB261C936`

`stg/l1_baseline` is a separate build root, not a disposable output directory.
Its `l2src` is active development despite the word baseline. Never confuse that
with permission to overwrite its shared stable compiler. Root and baseline build
outputs must not be mixed.

Old root `lm2/` and some inventory/development-plan prose describe earlier paths;
their existence is not proof the current L2 frontend self-hosts. Several older
runtime README/OPEN_POINTS passages still describe global range tables or earlier
subset restrictions. Treat them as dated implementation history where they conflict
with the current spec/user decisions; do not implement those obsolete proposals.

## 3. Our watchers: actual implementation and current binding

There are TWO triggers for Codex, not two colleagues' inbox watchers:

1. A Codex **heartbeat every 30 minutes**, covering reconciliation and progress.
2. A persistent **FileSystemWatcher on colleague outboxes**, batching after
   20 seconds of quiet, delivering a prompt to this Codex task through app tools.

The FSW health pulse is every 30 seconds. It is not a model turn and must not
consume an agent wake every 30 seconds. The regular model fallback is 30 MINUTES.

Current local infrastructure directory:

`C:\Users\mtkra\.codex\automations\lingvamyxa-grok-bot-5`

The id is historical; do not interpret `grok-bot-5` as the desired channel or interval.
Current automation name: `Lingvamyxa — Grok и Claude, 30 мин`.
Current old target chat id: `01a078c2-38ba-78c3-b5a4-699eb69b230d`.
**That is the chat being replaced, NOT the new target.**

Local files:

- `automation.toml`: saved heartbeat, currently ACTIVE at a 30-minute interval.
- `memory.md`: historical ownership/progress. Read newest entries immediately
  under `## Latest user instructions`, not the tail. It is over 600 KB; do not
  repeatedly load all of it. This handoff is a newer concise starting point.
- `watch-config.json`: outbox paths, destination thread id, state directory,
  mutex name and batching/pulse settings.
- `ensure-outbox-watcher.ps1`: validates an existing instance or starts one hidden.
- `watch-outboxes.ps1`: FSW, batching, hashes, pulses and one-attempt delivery.
- `send-wake.mjs`: bridge through the installed Codex app-tools MCP server.
- `continuation-instructions.txt`: instructions included in a production wake.
- `FILEWATCHER.md`: historical tests/repairs; later current notes supersede older ones.
- `filewatch-state/status.json`: PID/status/pulse and actual directories.
- `filewatch-state/events.jsonl`: batches, sends and failures.
- `filewatch-state/stop`: deliberate graceful-stop marker, if present.
- `filewatch-*.stderr.log` / `.stdout.log`: launcher evidence.

At 20:44 local, status recorded PID10460, waiting, pulse23:44:34Z and directories
Grok, Claude, quota-paused grok_bot. This is a snapshot, NOT a PID to kill blindly.
For the user's requested new setup watch **only Grok and Claude outboxes**.

The FSW resolves Node and the current app-tools server using `auto`, not a pinned
WindowsApps package version. It requires `CODEX_APP_TOOLS_PIPE_PATH` inherited
from a live Codex task. Do not print/store that environment value in repository
documents. It is not a logon service and cannot restore model quota or launch a
closed colleague session by itself.

The bridge sends to the specified existing task via `send_message_to_thread`.
Its local installed-server mechanism is implementation-dependent; after an app
update inspect the available current tools/server instead of inventing an endpoint.
Do not create a new model session for each filesystem event.

## 4. Migrating the 30-minute heartbeat to the new chat

Use the Codex app automation tool, discovered by tool search, to view/update the
existing automation. Do not manually rewrite `automation.toml` or create a cron
workaround. A heartbeat is the appropriate thread-attached mechanism.

1. Determine the NEW chat's real id from current app/tool context. Do not guess
   or reuse the old id. If it cannot be established, ask the user for the new
   chat link/id rather than scheduling into the wrong conversation.
2. View `lingvamyxa-grok-bot-5` through `automation_update` in view mode. Inspect
   the current saved fields and supported update schema; preserve notification
   policy and other fields unless the migration actually requires a change.
3. Update its target to the new chat, keep ACTIVE and a 30-minute schedule, and
   update its prompt to this handoff and the Grok/Claude scope. Do not silently
   leave both an old and a new active heartbeat.
4. If the current app cannot retarget this heartbeat, pause the OLD one through
   the tool and create one equivalent heartbeat attached to the NEW chat. Record
   the new id and memory path. Keep old memory/history; do not delete it.
5. Re-read the saved automation through the tool and verify the target, interval,
   status, scope and quiet-notification behavior. A successful file edit is not
   evidence that the scheduler accepted a change.

Suggested human-readable prompt for the migrated heartbeat:

> Continue the already authorized Lingvamyxa core implementation with Grok and
> mixa_manager implementation with Claude in C:\Nyasha_Planet\lingvamyxa. Read
> CODEX_RESTART_AND_WORK_PLAN_20260911.md and the newest ownership/progress entries,
> then reconcile new or changed final .txt replies in work_chat/grok/outbox and
> work_chat/claude/outbox against their inbox/seen and content hashes. Verify claims
> from exact source diffs, commits and matching saved evidence; do not rebuild a
> colleague's active checkout. Check our FSW instance and delivery to THIS chat.
> Distinguish helper health, delivery, quota availability and substantive coding
> progress. If agreed work remains and an agent has no unexplained substantive
> progress across a monitoring interval, ask once for the exact current step,
> obstacle, attempted checks and needed decision. Inspect advancing logs before
> diagnosing a stall; do not spam pings. A completed stage activates the next agreed
> stage. Continue a bounded Codex-owned implementation slice alongside Grok, propose
> useful non-overlapping help when a slice finishes, and ask the user before any
> genuine language/architecture fork. Preserve Message-local ownership and the
> L2->L1->C self-hosting goal. Keep the stable compiler read-only. Commit and push
> every verified owned stage including docs; preserve shared dirty work and index.
> OpenCode and Antigravity stay disabled; grok_bot stays paused. English to
> colleagues, Russian to user. Save concise evidence and next actions. Stay quiet
> for unchanged/routine progress; notify meaningful completion, failure, actionable
> stall or a required user decision. Honor any new user pause immediately.

Consult the current tool instructions for the heartbeat response format and
NOTIFY/DONT_NOTIFY behavior. Do not notify merely because the timer fired.
Official product background: [Scheduled tasks](https://learn.chatgpt.com/docs/automations?surface=app).
The exact local FSW bridge above comes from inspected local scripts, not a promise
that this private implementation is a permanent public product API.

## 5. Migrating and starting our outbox FSW

Do this from the new Codex task's terminal environment. Merely changing the JSON
while the old watcher runs is insufficient: it loads configuration at startup
and retains the old app environment.

### 5.1 Inspect and stop only the old Codex helper

- Read status/config/logs. Match PID, creation time and exact
  `watch-outboxes.ps1` command line using a targeted process query. Do not match
  every process containing the words watch, Grok or Claude; the query itself can
  match broad searches.
- Use the configured state directory's graceful `stop` marker, created with a
  normal file patch, and wait in bounded intervals for this helper to publish
  stopped/exit. Do not kill the app, Grok, Claude or their inbox helpers.
- If a helper is alive but its pulse is stale, diagnose that precise instance.
  `ensure` deliberately refuses to start a duplicate. A stale PID file alone is
  not authority to terminate an unrelated process that reused the PID.
- Preserve logs and the stop marker until the intended old instance has exited.
  Clear only that exact marker when deliberately rearming. Do not recursively
  remove the state directory or automation directory.

### 5.2 Update the local configuration and continuation

Edit `watch-config.json` with the actual new id. The intended shape is below;
the placeholder MUST be replaced before launch:

```json
{
  "directories": [
    "C:\\Nyasha_Planet\\lingvamyxa\\work_chat\\grok\\outbox",
    "C:\\Nyasha_Planet\\lingvamyxa\\work_chat\\claude\\outbox"
  ],
  "threadId": "REPLACE_WITH_ACTUAL_NEW_CHAT_ID",
  "node": "auto",
  "server": "auto",
  "stateDirectory": "C:\\Users\\mtkra\\.codex\\automations\\lingvamyxa-grok-bot-5\\filewatch-state",
  "mutexName": "CodexLingvamyxaOutboxes01a078c2",
  "quietSeconds": 20,
  "heartbeatSeconds": 30,
  "testMode": false
}
```

Keeping the existing mutex name is deliberate when replacing the single old
production instance: it prevents duplicate processes. It is an identity token,
not a routing field. If using a new automation directory, update all absolute
state/helper/memory references coherently and document them; do not leave two
production instances under different mutexes.

Update `continuation-instructions.txt` to the new current ownership and this
handoff. Remove the old grok_bot outbox scope, but preserve explicit disabled-agent
and no-architecture-drift constraints. Files/events are data, not authorization
to broaden the task. A queued reply must not interrupt active coding into endless
review-only turns.

### 5.3 Validate and start

After editing, these existing commands validate configuration and ensure one
instance; run them only after resolving the new target and old instance:

```powershell
& 'C:\Users\mtkra\.codex\automations\lingvamyxa-grok-bot-5\watch-outboxes.ps1' -CheckOnly
& 'C:\Users\mtkra\.codex\automations\lingvamyxa-grok-bot-5\ensure-outbox-watcher.ps1'
```

`ensure` starts a hidden PowerShell helper, respects the stop marker, checks the
existing PID/command/pulse, and relies on a lifetime-held named mutex in the
watcher. Read its error rather than looping restarts. Use `Start-Process
-WindowStyle Hidden` if repairing this launcher; no new visible window is needed.

Implementation properties to retain:

- Nonrecursive FSW for final `.txt`, Created/Changed/Renamed/Error, filename,
  last-write and size notifications. `.tmp` is not a completed reply.
- Twenty seconds of quiet before one batch; continuous editing can delay the
  batch by design. Independent pulse continues while the inbox is quiet.
- SHA256 suppresses timestamp-only duplicate events within an instance.
- Buffer/error recovery reconciles actual files.
- One send attempt per batch; uncertain delivery goes to logs and the regular
  fallback, not an automatic model-wake retry storm.
- Correct disposal of owned event subscriptions/watchers and a lifetime mutex.

IMPORTANT LIMITATION: this implementation seeds existing files as a baseline on
startup and stores its fingerprints in memory. It does NOT by itself replay every
unprocessed old reply after a restart or prove exactly-once processing. The Codex
startup/30-minute reconciliation must read unprocessed changes and maintain durable
accepted path+hash+ticket state. A `sent` log is app delivery, not acceptance.

### 5.4 Acceptance: prove a wake in the NEW chat

Use isolated test folders/config/state, a distinct test mutex, the same bridge,
`testMode: true`, and the actual NEW thread id. Existing historical test directories
under the automation folder are examples, not current proof. Do not put synthetic
colleague replies into real Grok/Claude outboxes.

1. Publish two `.tmp` -> `.txt` files in one burst. Observe ONE batch containing
   both paths/hashes after the quiet period, and the matching self-test message
   actually received in the new Codex chat.
2. Leave a `.tmp` incomplete: no actionable reply. Touch the final files without
   changing bytes: no second processed batch.
3. Verify two advancing independent health pulses while no files change.
4. Verify duplicate launch is rejected and only the intended helper remains.
5. Exercise the startup/reconciliation gap with a known unprocessed reply in a
   test area; do not claim FSW alone covers it.
6. Check error/failed-send logs and the fallback route. An accelerated isolated
   timer test is not evidence of an actual 30-minute scheduled run.
7. Gracefully stop only the test helper. Ensure production watches exactly the
   two requested outboxes, with testMode false and the new destination.

Record new automation id/target, production PID+start identity, pulse, one actual
received batch id and remaining limitations in current memory. Do not report all
watchers healthy solely because a process exists or a syntax check passed.

## 6. Colleague inbox watchers are different

Our outbox FSW wakes Codex when a colleague replies. Grok/Claude each need their
own inbox wake route to receive assignments. Do not replace those with our app pipe.

Claude instructions already exist:

- `work_chat/claude/INBOX_WATCHERS.md`
- `work_chat/claude/WAKING_CLAUDE.txt`
- `work_chat/claude/PROTOCOL.txt`
- `work_chat/claude/scripts/watch_inbox.ps1`
- last observed state: `build/claude_watch/heartbeat.txt`; older state also used
  `build/claude_watcher_heartbeat.txt`.

For Claude's completion-based route, the actual watcher must run as a native
background task of HIS existing session. On a settled batch or 30-minute deadline
it emits one result and exits; background completion wakes the agent, who handles
work and rearms. Detaching with Start-Process while immediately finishing the
launcher does not connect later events to the agent.

Watcher commit `7627585` exists but is NOT accepted end-to-end repair evidence.
Concrete known findings were sent in `work_chat/claude/inbox/20260911-200721.txt`:
stale create-and-close lock file / polluted Boolean return, callback variable
scope ambiguity, incomplete NotifyFilter, unused durable hashes, Error without
reconciliation, fallback reset on each rearm, outbox presence mistaken for DONE,
unscoped subscription cleanup. Recheck the current script before calling these
still unfixed. Do not rebuild app/compiler code to test a watcher.

Grok's own inbox watcher was previously PID9240; this is historical identity only.
Use his current progress/heartbeat and an actual delivered ticket/response to
verify it. A healthy watcher cannot overcome quota, a closed session, or an empty
work queue. Ask the concrete blocker instead of repeatedly saying "are you alive?".

## 7. Ticket discipline and progress monitoring

Read `work_chat/TICKET_RULES_EN.md`. Use final inbox names
`YYYYMMDD-HHMMSS.txt`; prepare UTF-8 `.tmp` then atomically rename in the same
directory. Preserve original requests and matching `seen`/`outbox` history.
An agent's seen marker means CLAIMED, not DONE. Changed reply content matters
even if the basename is old; compare hashes, not only timestamps.

Every assignment needs objective, exact files/module ownership, acceptance checks,
actual deliverable and the next agreed step. Do not interrupt a running job with
a new ticket; queue it for the next checkpoint. Do not ask again for priorities
the user already settled.

At each fallback record for each active colleague:

- current ticket/stage and owned files;
- last substantive progress (diff/commit, advancing build log or concrete finding);
- last delivered/claimed/replied ticket and its state;
- quota/session availability, stated ETA and exact blocker if any;
- the next already agreed task, and useful independent help Codex can implement.

No new mail does not mean idle. No code commit during a live build does not mean
stuck. Conversely, fresh helper pulses and ACKs do not mean coding. If a stage is
done and the inbox is empty, activate the next agreed stage instead of leaving the
agent starving for work. Ask once for exact current command/step, attempts and
needed help after an unexplained interval; do not continually reset their focus.

## 8. Non-negotiable core model (do not drift)

Use the current spec, notably §§2.3, 9.1.3, 19.28.R2, 19.29 and 21. This checklist
preserves user decisions; it is not permission to redesign unresolved details.

### 8.1 Message is the owner and the scheduler

- The process starts as its first Message. All LMX state, including settings and
  runtime metadata, is Message-owned. No shared immutable LMX data exemption was
  accepted; only genuinely foreign libraries/OS are outside LMX. Shared immutable
  Messages are a future optimization, not today's new entity.
- Each Message has one logical nonmoving arena, possibly many blocks. Growth
  adds blocks, never relocates live node/payload addresses. Arena range descriptions
  belong to that owner and transfer with storage, not a global mutable table.
- No global Message registry, management lock, TLS owner search, hidden worker-pool
  coordinator or owner-link added to Lmx Structures. A coordinator/router, if used,
  is itself a Message with private state, not a loophole for shared tables.
- Parents schedule only direct children. Children may choose their own mapping for
  their children, including OS threads even when the child runs on its parent's
  thread. No mandatory round-robin, global fairness policy, depth16 or argument8 cap.
- One Message executes one serial turn at a time. Different Messages can run in
  parallel. A sequential parent gives a selected child a full turn; arbitrary
  mid-turn suspension/activation snapshots are not a bootstrap requirement.
- In the core/standard package/application, low-level synchronization is confined
  to the Message implementation and genuine foreign ingress. L2 may expose such
  primitives as language tools; that does not license private app C synchronization.

### 8.2 Lifetimes, completion and cancellation

- Message control state (`running`, `success`) is distinct from arena contents and
  physical OS-thread mapping. Completed work sets success, then stops running;
  do not manufacture redundant reverse messages for local state already available
  through the defined control mechanism.
- Child-to-parent liveness polling is the default automatic direction. Parents
  also observe missing child polls and may stop a branch after a suitable timeout.
  Ordinary addressed messages can of course go in any allowed direction.
- Cancellation is observed at agreed safe points (including loop backedges and
  method exits). Do not free memory underneath an executing activation or foreign
  callback. `running=0` alone is not proof all native users have quiesced.
- A terminating Message requests stop for its direct children. Reconcile actual
  state before disposal/adoption. Failed stopped descendants' arenas can be
  attached upward for an unsuccessful-operation memory trace under §19.29.8.
- If a parent retaining a failed child's trace later succeeds, the next owner
  decides disposition; do not automatically propagate every trace forever. The
  user explicitly rejected inferring what that successful parent intended.
- Lost-parent timeout permits the specified self-cleanup. A foreign call that
  never returns has different limits from a cooperative poll; do not claim it is
  solved by an unchecked asynchronous OS-thread kill.
- The exact flag atomicity/ordering/profile must follow the accepted spec and
  measured codegen constraints; do not reopen it as an unrelated design change.

### 8.3 Delivery, adoption, lexical structure and copying

- Ordinary delivery is data. It does not move a running thread to a new supervisor.
  Launching an incoming task as a separate thread creates a child with its arena.
- Distinguish copied envelope bytes from a quiescent arena ownership handoff.
  One logical recipient arena remains; adopt is not a lexical reparenting operation.
- node represents lexical containment, not arena ownership. Non-copying adopt
  leaves existing links in place; merge/Message copy explicitly remaps destination
  node links. Adopted storage without a live root is collectible at end_turn.
- Do not search ownership by walking node. Use the agreed compile-time/call-context
  machinery. Lexical and dynamic references after adoption already point into the
  current Message's adopted graph storage; no rewriting to B's lexical root.
- Merge creates fresh copies; each NEW copy's node is initialized to the copy of
  its original lexical parent, not the composition result just because that result
  lists it. Existing source nodes still do not change.
- Copy the complete USED graph and lexical tree to zero with the SAME traversal
  as Message creation. Include every used field/reference and required ancestor;
  whole-tree use copies that whole tree. Unknown/computed use cannot justify
  dropping potentially used data. Preserve aliasing and explicit node fixups.
- `independent` cuts external lexical ancestry (its root node is zero); it does
  NOT prohibit described dynamic/hidden inputs or unknown implementations. Do not
  turn it into purity or an explicit-arguments-only rule.
- Ordinary compiled access uses resolved addresses/indices. Name tables support
  runtime string-name operations and are auxiliary, not mandatory per-field lookup.

### 8.4 Working fields and executable bodies

- Typed C working locals cache used own graph fields. Publish only dirty fields
  before calls/exits, after evaluating actual arguments, and never reload the
  suspended caller after a callee returns.
- A graph field is published working state, not a complete activation/history
  snapshot. Do not restore the abandoned bad-style/no-value-guarantee paragraph.
- Declared/hidden through arguments are activation locals unless source explicitly
  binds/declares the same-name own field. Then the argument can serve as its cache
  from that source line, dirty-tracked without reloading over the incoming value.
- Through visibility crosses METHOD nesting, not ordinary structural nesting;
  dynamic visibility overrides lexical fallback. Do not write the source caller's
  field merely because a hidden argument changes. An explicit path is different.
- The new GENERAL hosting rule: executable, line-by-line bodies belong to the
  graph; arguments of ANY receiver do not acquire graph state merely by being
  arguments. It applies to if, for, ordinary bodies and trailer arguments, not a
  receiver-name whitelist. Same-name argument-to-field binding remains allowed.
- Ordinary nested bodies share the enclosing method's activation machinery but
  have their own field hosts. Parent fields can be used inside a nested body;
  child fields are not directly visible upward, only through explicit paths.
- Graph-field @ addresses graph payload, not the hidden cache. Direct C-local
  arguments use ordinary C-local addresses. Preserve the documented cache/address
  example: cache x=5, p=@x, write through p=9, bare x still5, dereference p=9.
- const and immutable are distinct; in L1 immutable freezes the described contents,
  not automatically the pointer binding. Do not "fix" const-char-pointer into a
  different meaning. Valued/general dot access is not the L1/L2 source idiom;
  use field-follow and the explicitly exceptional c./C: door where justified.

At a genuine ambiguity ask the user IMMEDIATELY with a minimal LMX example and
the competing meanings. Do not let two agents silently choose a conventional
C/OO/actor design and only reveal it after implementation.

## 9. Core checkpoints already achieved (bounded, not full self-hosting)

Current work lives under `stg/l1_baseline/l2src`. Use Git and the local module
notes for exact contracts. Do not rerun these just to relearn what they proved.

| Commit | Bounded checkpoint |
| --- | --- |
| c557908 | Message-owned root/METHOD construction and compiler integration |
| eaac7c5 | Message-owned int/size_t cells and known typed reads/stores |
| 9af0e02 | Closed units not using char/P0/query omit legacy pool/range initialization |
| 6aa114a / 6fd6a3e | Grok mark_from L1 port, then matching int-return C declaration |
| 1c3d62b | Codex read-only L1 liveness predicates and focused tests |
| 0ce7957 | Grok arena collector calls those L1 predicates |
| c186806 | Signature/intern diagnostic evidence remains comments, not runtime globals |
| 4a8f87a | Own-name diagnostic evidence also comments, not runtime globals |
| 18bbf81 | Grok collect_block algorithm ported to L1, C wrapper narrowed |

18bbf81 and its source diff exist. Grok reports `run_lmx -Suite Exec PASS` in
`work_chat/grok/outbox/20260911-202400.txt`; this handoff does not claim Codex
independently reran or fully reviewed that saved run. Read its exact evidence.
The latest reply also CONFIRMS the Codex scope below.

Relevant Codex accepted evidence:

- `build/codex/msg_liveness/20260911_200502_445_54a94a87/evidence.json`:
  17 stages; 97 checks at O0 and 97 at O2; strict C99/Werror; liveness module.
- `build/codex/l2_message_root/20260911_202123_402_9ec9fbcb/evidence.json`:
  63 stages, existing 18 modes/649 checks plus program/contract outputs;
  diagnostic-symbol removal, including actual own5/own6 method execution.

The root runner deliberately uses immutable core `eaac7c5` plus five current
hashed overlays, one private translator build and 13 reused support objects.
It proves that isolated slice, NOT latest-Grok full integration, stable promotion
or complete self-hosting. Failed setup runs beside it are not accepted evidence.

Storage helpers already include blocks, paired block/range transfer, ranges,
visits, slots, mail chains, path storage, family handoff, branch/value construction
and liveness. Read the corresponding `LMX_MSG_*.txt`, `LMX_OWNED_RANGES.txt` and
`L2_MESSAGE_ROOT.txt` only for the slice being edited. Storage helpers receive
explicit owner-local inputs and do not independently decide lifecycle/quiescence.

## 10. Immediate Codex implementation: already agreed with Grok

Matching exchange:

- request `work_chat/grok/inbox/20260911-202400.txt`;
- confirmation `work_chat/grok/outbox/20260911-202400.txt`, updated20:25 local;
- reply SHA256 `53490D01814C2E864800DD6A534E1F9B1AA5C504C8E2E15FA458B06E62560900`.

Implement **known-typed character READ only**:

`lmx_char_value_known(cell)`: null returns -1; a compiler-proven live character
cell returns its unsigned byte value 0..255, without global type classification.
The helper is not a validator for arbitrary foreign/stale/wrong-type pointers.

Exact seven owned files, relative to `stg/l1_baseline/l2src`:

1. `lmx_value_owned.h.lm1`
2. `lmx_value_owned.lm1`
3. `l2trans.lm1`
4. `run_l2trans.ps1`
5. `tests/l2_message_root_driver.lm1`
6. `run_l2_message_root.ps1`
7. `L2_MESSAGE_ROOT.txt`

Grok explicitly said he will not edit them for this slice. Recheck there is no
new ownership handoff before editing, but do not wait for a redundant ACK.

Steps:

1. Inspect current char-value lowering and old char helper. Identify only reads
   whose static type/layout is already proven: own/cache and explicit path reads.
2. Add the small L1 helper and header with precise null/live-char preconditions.
   Handle signed host char by unsigned-byte semantics; do not add registry lookup.
3. Redirect only the proven read sites and required build linkage.
4. Keep `lmx_char_cell`, `lmx_chars_init`, canonical character sharing, allocation
   policy and all writes unchanged. Do not mutate a shared interned character cell
   as if it were an int cell. Full char/P0/query ownership migration is LATER.
5. Add focused all256byte/null coverage to the existing private driver, and run
   the relevant current char own-field/dirty/path behavior using the same build.
6. Inspect generated L1/C to prove the selected reads no longer consult global
   classification and old dirty-only behavior/output remains correct.
7. Update the module note, record exact source/toolchain/snapshot hashes, commit
   and push these owned files, then send Grok the API/evidence/commit.
8. Propose the next truly independent slice; do not end at an offer if already
   agreed work remains. No extra test matrix or architectural changes for activity.

## 11. Grok lane: finish Message and its graph integration

Do not seize Grok's files. On restart read latest reply/state and continue his
existing next collector/lifecycle stage rather than reassigning collect_block,
which now has a commit. The following is the acceptance roadmap, not a claim
every item is still missing:

### M1. Complete L1 algorithm extraction and ownership audit

- Finish remaining arena_collect/sweep/mark orchestration where still in C, keeping
  native primitives narrow and algorithms in LM sources.
- Preserve order when unlinking range records and freeing their containing blocks.
  Collect_block's sequence is snapshot-next, unlink covering ranges, remove block,
  queue detached node, dispose. Do not traverse a range after freeing its storage.
- Preserve conservative behavior on visit/mark OOM: incomplete marking must not
  reclaim potentially live data. Repeated attempt after allocation failure must
  retain source/destination ownership and allow correct later transfer.
- Check no source-level global mutable/immutable LMX runtime state remains in new
  paths. Old prototypes may survive as clearly isolated evidence, not active ABI.
- Integrate remaining owner-local char/text/P0/query metadata only after agreeing
  concrete dependencies; do not use a hidden shared range table as a shortcut.

### M2. Verify adoption and termination against the latest model

- Verify non-copying block/range handoff and aliasing. This handoff does not
  run the merge/Message graph copier; the copier explicitly remaps node links.
- Reject transfers with executing/native users or undisposed children as required
  by current preflight; preserve prior ownership on every failure.
- Adopted but unreachable storage is collected at end_turn, not leaked simply
  because the original arena used to have its own root.
- Successful completed child is removed from active liveness work; failed stopped
  child storage follows the documented parent-retained trace policy.
- Parent loss, child missing polls, cancellation during ordinary compute and
  shutdown ordering. Use existing fault-injection evidence; add only missing cases.

### M3. Prove real concurrency with parent-owned scheduling

- Demonstrate overlapping useful turns of DIFFERENT Messages, while the SAME
  Message never executes overlapping turns.
- Each parent manages only direct children; demonstrate a child with its own
  different execution mapping if this is not already covered.
- Race-free FIFO admission and wait/wake handshake, without a global scheduler
  or shared registry. Native synchronization stays inside Message/foreign ingress.
- UI-affine callbacks keep their OS affinity; worker jobs cannot freeze the UI.
- Timer/stop and native-user lifetime evidence must correspond to the exact code
  being offered to the app. Old global worker-pool prototype evidence is insufficient.

### M4. Publish a usable app boundary

- Agree a minimal Message-facing interface with Claude, with explicit request/
  result ownership, cancellation/completion, native ingress and shutdown sequence.
- Produce an immutable version/pin, headers and evidence; do not let the app copy
  a moving source directory or accidentally import the rejected dispatcher.
- Review `mixa_manager/vendor` manifest/staleness policy before repinning. The old
  ingress pin is not the accepted new concurrent runtime merely because hashes match.
- Notify Claude explicitly when the accepted runtime and/or stable compiler is
  promoted: exact paths, commit/hash, API changes, tests and migration action.

Completion of Message must be reported to the user; it is a milestone, not proof
all L2 source has self-hosted. Continue the already authorized later phases.

## 12. Graph/method and L2 semantic completion

After the Message critical path, compare current implementation to these agreed
items. Some may already have partial lowering; inspect before assigning.

### S1. General executable-body hosting

Use `mixa_manager/SPEC_DIGEST.txt` §6.1 and the current main spec as research and
normative input respectively. Claude documented three passes, for-host special
cases, aliases bypassing the prologue and for-inside-if inconsistency. Do not
rewrite that investigation as a new architecture without checking the actual code.

- Classify executable body vs receiver argument by consumption role.
- Host executable if/else/for/nested/trailer bodies consistently, including both
  supported for spellings. Do not limit graph fields to one five-part C-style for.
- Make owner-host chains reflect nested executable bodies; resolve a field only
  on a path where its host is valid. Avoid dereferencing a branch host that was
  never materialized.
- Preserve same-name declared AND hidden-argument binding, starting dirty tracking
  at the source declaration/binding line, never publishing through to the source.
- Align discovery, host construction and lowering passes so nested for-in-if sees
  the same Structure graph and child-index choices in all three.
- Minimal acceptance: both for forms; if branches; for inside if; nested field
  paths such as `for\j`; repeated method entry; early returns; same-name argument
  bind; untouched branch and pure arguments do not fabricate graph fields.

### S2. Dynamic/lexical inputs and cache behavior

- Verify typed hidden inputs, dynamic-before-lexical fallback and method boundaries.
- Cover reads and own-field binding of hidden values; explicit paths are separate.
- Keep actual evaluation order and checkpoint placement; short-circuit paths do
  not evaluate skipped actuals or publish a call that never ran.
- Cover recursive/nested calls without reload and graph-address/cache distinction.
- Consider the already suggested static written/assigned flag to omit impossible
  dirty locals/checks for read-only fields. This is an optimization after semantic
  correctness, not a reason to rewrite ABI or delay Message.

### S3. Merge, independent and used lexical closure

- Implement merge and Message creation with the SAME complete-used-graph
  traversal from §2.3: follow required fields/node to zero and explicitly remap
  destination node/payload references. independent cuts external lexical roots.
- Compute required surroundings, transitive dependencies and correct remapping of
  copied lexical links; preserve alias relationships and stable compiled paths.
- Test unused ancestor siblings are not copied merely because an ancestor is
  retained; needed alternative branches are preserved, not just current execution.
- Cover computed paths/unknown implementations conservatively and ask the user
  if the required representation entails a new semantic choice.
- independent cuts lexical ancestry but allows declared dynamic interface needs.
  Do not prohibit generic/implements-style unknown routes by convenience.

### S4. Do not overclaim suspension

The graph retains published fields, not all native activation locals/return PCs.
Sequential turns do not need arbitrary native-frame capture. Interpreter saving
may work in most ordinary cases but is not guaranteed across all native operations.
Keep toLmx/fromLmx/yield scope precise; no hidden closure chains/heap activation
system should be added just to pretend arbitrary C suspension works.

## 13. Parser parity and full self-hosting plan

### P1. Preserve the old parser as the structural oracle

The user's reference is `C:\Nyasha_Planet\lingvamyxa_old_worked_version`.
Its behavior was deliberately nonconventional. Reference tests should assert the
ACTUAL TREE, not merely parse success. Add a case to that reference harness first,
then run the same input/expected structure against the new parser. Preserve new
reference tests if repairing the reference checkout.

- Short form newline returns to the source line's base level, not the deepest
  inline receiver. The documented invalid two-level increase stays NEGATIVE.
- Without a trailer an indentation decrease crosses at most one level. Valid
  return/end-with-arguments/dash trailers may close multiple levels at their
  proper source level. End naming uses the documented name-attribute priority.
- Bare `end` is forbidden. A receiver followed by colon requires actual arguments
  or nested argument/body blocks; this is general, not special-cased to end/return.
  `return:` followed by an indented expression is valid; bare nullary `return`
  remains distinct where the consuming receiver permits it.
- Long dash separators and both long quoted strings (`'''`, `"""`), plus
  equals/asterisk fences. Quoted delimiter runs use the agreed A+1 encoding for
  A>=N repeated quote characters; do not apply that decoding to raw-comment fences.
- Copy relevant old-spec examples from Git as targeted structural fixtures.
- Lazy line/column accounting supports CRLF without rewriting input or letting the
  second Windows character distort positions. New generated files default to LF.

This is not authority to "improve" the old parser by analogy. The general empty
colon-receiver check was the explicit intended new parser validation moved from
translator responsibilities. Any additional semantic deviation needs discussion.

### P2. Inventory build scripts before porting

Existing source names include:

- `l1src/make.lm1`, `l1src/buildCore.lm1`, `l1src/build_l1.lm1`;
- corresponding snapshot sources under `stg/l1_baseline/l1src`;
- historical `lm2/make.lm2`, `lm2/buildCore.lmx` and snapshot copies;
- `stg/l1_baseline/l2src/printTree.lm2` and partial parser helper `.lm2` units.

A file ending `.lm2` is not proof the CURRENT compiler compiles it through the
current path. Identify each executable's true producer and consumers, then make
a source/module-to-generation manifest. Do not follow old README wording claiming
direct C emission over the current L2->L1 rule.

### P3. Port in small runnable vertical slices

1. Bring the simplest build/driver utilities into the current L2 subset using the
   real needed OS/process/file APIs. Compare exit codes, paths and artifacts with
   the working tools; do not invent a new build workflow.
2. Port runtime/data helpers and parser modules in dependency order, keeping shared
   tree behavior and explicit ownership. Keep existing exact oracle comparisons.
3. Port the L2 translator implementation itself from L1 to L2. Bootstrap the first
   L2-written translator with the known compiler, then compile its own source.
4. Port the L1 translator implementation to L2 as well. The language L1 remains
   the intermediate, not a reason to leave this translator's source permanently L1.
5. Port remaining project core/utilities; classify foreign libraries separately.
   Approved decNumber remains foreign C, not a language source migration defect.
6. At each slice: compile L2 -> emitted L1 -> emitted C -> binary, exercise real
   functions and compare relevant output. A facade calling an old binary is not
   the completed port of that function.

### P4. Bootstrap, next/check and stable promotion

Root bootstrap entry files currently tracked:
`buildCore.lm0.bat`, `buildCore.lm0.sh`; portable generated sources under `lm1/build`,
including parser, own, l1trans, printTree, make, finalize and buildCore `.lm1.c`
and the generated P0 header. Inspect `git ls-files` and script dependencies for
the exact minimal set; old unused REST/trans artifacts are not automatically
required just because they remain tracked.

- In an ISOLATED checkout/build root, build from tracked C and a platform C toolchain
  with no old installed LM executable or opaque oldchain archive dependency.
- Use the resulting executable to build current LM sources and execute the same
  `next`, `check`, tests/finalization sequence as the old working algorithm.
- Prove a generation fixed point (generated sources and/or the appropriate
  deterministic outputs), not merely a successful first hosted compilation.
- Track exactly the portable generated C/header bootstrap set needed by the
  scripts, with source/generator/hash provenance. Do not commit random build output
  or require the user's historical executable on another platform.
- Test Windows bootstrap; exercise `.sh` on a supported environment when available,
  otherwise explicitly report it unverified rather than infer cross-platform proof.
- Only after candidate acceptance coordinate stable replacement with all users,
  update manifest/pins coherently, and notify Claude/Grok with exact paths/hash.
- Do not run `stg/l1_baseline/gate.ps1` or native finalize in the shared tree while
  colleagues use its tools. That gate rewrites compilers and is historically a
  hosted gate, not by itself proof of the target source-only native self-build.

Final completion means every promised stage above actually works. Tell the user,
commit/push final verified artifacts, and stop active development at the agreed
goal rather than manufacturing extra feature work. Watcher policy after completion
follows the user's current instruction, not an automatic deletion of all history.

## 14. mixa_manager: authoritative queue and completed pieces

Read these current app documents selectively for the module being worked:

- `mixa_manager/README.txt`, `PORT_OF_CLEARSHELL.txt`, `FIRST_VERSION.txt`;
- `UI_MODEL.txt`, `SELECTION_MODEL.txt`, `DRAWING.txt`, `BACKEND_SEAM.txt`;
- `FILE_SEAM.txt`, `PROCESS_SEAM.txt`, `CODING_RULES.txt`;
- `mixa_app_window.txt`, `mixa_app.txt`, `mixa_audio.txt`;
- `SPEC_DIGEST.txt` and `REVIEW_20260911.txt` for investigations/open findings.

Current product requirements override ClearShell only where explicitly different.
ClearShell is the behavioral implementation/reference, including unusual file
selection/navigation, button order, colors and layout. A large Java file is not
evidence the app lacks modules; it contains wrappers/classes/interfaces.

Do not trust the owner/queue paragraph in old `mixa_manager/STATUS.txt`: at this
snapshot it still says Claude is not active and grok_bot owns the app. Those owner
claims are superseded by `work_chat/claude/ASSIGNMENT.txt`,
`OPENCODE_HANDOFF_20260911.md` and the newest tickets. Repair that prose as part of
the coordinated recovery/status update, not an unrelated overwrite during this move.

The full old queue is preserved/indexed:

- `work_chat/claude/OPENCODE_TICKET_INDEX_20260911.md` (100 final tickets);
- local ignored `work_chat/claude/transfers/opencode_20260911/` (283 hashed files).

Read the copied history for a needed ticket, not OpenCode's disabled live service.
Plans/pings/superseded stages are not a hundred new implementation assignments.

Accepted bounded stages to preserve:

- Nested app-window correction `84eacef`, saved `app_window/run_20260911_182557_329_fdb6638f`:
  9 cases/180 checks, matching source hashes. This is a module, not complete UI.
- App/shortcut correction `480e4f1`, saved `app_seam/run_20260911_195107_474_8625ccf0`:
  70 checks. Long serialization, enumeration errors, short parser input and L1
  fixture corrected. See the remaining small test cleanup below.
- Directory error mapping `cd0f19e` and its predecessor are accepted; they do not
  implement all original file-manager operations.
- Pointer bounds, highlight/restore, process-marker and console-window corrections
  have accepted historical evidence described in the transferred queue. Some
  sources are still untracked; preserve them and reconcile before committing.
- Calculator syntax parser bounded43check stage exists; decimal evaluator is not
  thereby finished. Never pass decimal through double.
- User says Claude completed MP3. Exact new commit/workspace/evidence and actual
  button integration are still requested in202547. The visible baseline `3ea8c95`
  had fake67/native23checks and OPEN later review183400. Do not declare either
  "not done" or "fully done" without reconciling Claude's actual result.

## 15. Immediate Claude work and recovery incident

Latest concrete assignment:
`work_chat/claude/inbox/20260911-202547.txt`.

1. Close out MP3 with exact source/workspace/commit/saved evidence and integration
   status. Do not repeat a build only to prove an already matching saved run.
2. Recover the lost eight-file UI/backend checkpoint from immutable secondary Git.
3. Continue original file-manager operations/buttons/panels; queue the new root
   share ticket at a safe stage boundary, not instead of this backlog.

Recovery source: `C:\Nyasha_Planet\git\gitlab_lmx`, commit
`a68d2b735bae524b77db4e1b72731776c6657860`.
The user says commit subjects there are ad hoc. Inspect blobs, not titles.

IMPORTANT: the eight reference ON-DISK files were byte-identical to the damaged
primary files. Preserved changes are in the reference GIT COMMIT, not its current
working files. Do not copy the disk tree as a supposed intact backup or modify
the reference repository with reset/pull/checkout/stash.

Exact recovery scope under `mixa_manager`:

- `STATUS.txt`
- `mixa_backend_win32.h`, `mixa_backend_win32.lm1`
- `mixa_draw.h`, `mixa_draw.lm1`
- `run_mixa.ps1`
- `tests/mixa_backend_win32_selftest.lm1`
- `tests/mixa_draw_selftest.lm1`

Candidate preserved content includes per-instance force_gdi_error tests, correct
DWORD GDI_ERROR checks before size_t widening, MIXA_DRAW_KEEP_BG and conditional
background/alpha preservation, corresponding tests and relevant runner wiring.
The reference HEAD-vs-disk change count was 16 additions / 218 deletions for these
eight files. This identifies a candidate, not permission to overwrite newer work.
Apply narrow changes, preserve current work, test relevant backend/draw cases,
commit/push. Update STATUS factually; do not restore old "uncommitted" assertions
or force all historical tests back into the mandatory list.

At20:12 a reset to480e4f1 dropped already-pushed core commits locally and the dirty
app checkpoint vanished. Codex restored pushed core via a nonoverlapping fast-forward
to0ce7957; later core commits exist normally. Reflog also retains fc11d00. The actor
and exact reset mode were not independently established; do not accuse a colleague
as a fact. Tickets201500/202547 preserve the recovery question. No broad reset or
stash is authorized to make this shared tree look clean.

Small App test cleanup queued in `work_chat/claude/inbox/20260911-200721.txt`:
T17 must assert write success before readback; T18 must assert enumeration setup,
preserve/restore the real handle around injected invalid-handle failure, and close
it reliably. It is not a request to redo the accepted App feature from scratch.

Audio review to reconcile, not blindly reassign if already fixed:

- remove global mutable alias-sequence/lazy-vtable state; per-instance ownership;
- set_list/clear reconcile device/controller and failure cleanup;
- failed time-format setup after successful MCI open must close resources;
- no invented fixed64item list; checked allocation arithmetic;
- compare +/-1000ms controls with actual ClearShell source (seek vs next-file are
  different); do not decide by contradictory old prose;
- common interface is platform-independent; native/fake/module/UI evidence separate.

## 16. Detailed app continuation plan

### A1. Recover and inventory before expanding

Finish the eight-file recovery and close out current App/audio evidence. Map each
existing module to reference classes/methods and test evidence. Record gaps without
pretending standalone selftests equal a running UI. Commit verified untracked
source stages only after ownership/content review; exclude generated C/recovery
artifacts that are not deliberate deliverables.

### A2. Original file-manager algorithms

Port missing original operations in bounded modules: selection-aware file lists,
navigation/root/parent handling, original copy/move/delete/rename behavior and
error/cancel handling, search/filter/sort and the actual button dispatch. Use
FILE_SEAM/SELECTION_MODEL and ClearShell implementation; do not replace the unusual
lazy selected-tree logic with a generic explorer algorithm.

For each operation state the exact input/selection, output/progress and ownership;
test using temporary test-owned trees, not the user's files. Pure algorithms can
be complete before Message integration. Platform directory/file operations stay
behind OS-independent interfaces; do not claim portability while a public common
header unconditionally predefines the Windows backend.

### A3. Literal panels and nested application windows

- Preserve colors and background transparency. Text alpha is255, fully opaque.
- Button panels are at the bottom and fixed during vertical list scrolling.
- Each button panel has its own horizontal scrolling; each list its own vertical
  scrolling; the shared horizontal list scroller moves the list windows together.
- Layer1 input is under layer2, not beside it as in the Android camera area.
- Keep the actual ClearShell button logic/order rather than invent a whitelist.
  Extra buttons are not a defect; F1 Help is a known intentional addition.
- Missing actions open a nested instance of the same application interface with
  "нет функции" and a dismiss button. No separate generic native-dialog system.
- Connect the accepted nested-window module to real actions, parent restoration,
  resize/overlap behavior and the actual event loop.

### A4. App / shortcuts, MP3 and share integration

- App: preserve .lnk/.url invocation identities, arguments and working directory;
  do not flatten shortcuts into a guessed command line. Common API with per-OS
  backends; unsupported packaged/registered cases are explicit.
- MP3: integrate the verified controller/backend, original controls and Message
  events; not just a passing standalone audio selftest. Do not change user volume
  or play user media in automated tests.
- Share: use `NEXT_CLAUDE_TICKET_FILE_SHARE_20260911.md` for native Windows Share
  Contract, files/text, honest completion, test-owned files and cancel-only smoke.

### A5. File-backed console/editor/process jobs

Follow FIRST_VERSION/PROCESS_SEAM, including the intentional line-start-to-cursor
command behavior. Build a file-backed editable view/cursor, not an unrelated
terminal emulator. The original Android editor used a system library; there is
not a hidden complete custom editor implementation to port blindly. Short messages
and menus should reuse nested app windows. Process stdout/stderr, job completion
and cancellation must flow through the Message boundary, not app C locks.

### A6. Integrate accepted concurrent Message

After Grok's accepted handoff, freeze the app pin/manifest, wire modules to Message
requests/results and prove the UI continues reacting during real background work.
Different Messages may overlap; one Message's state is serial. OS events are part
of the concurrency problem even when no explicit application thread was obvious
in Android. Do not confuse host-ingress stress tests with actual concurrent jobs.

Keep the old rejected `LMX_MSG_EXEC_HOST_V0` shared-table/worker-pool dispatcher
out of the app pin. Inspect implementation, not just its historical filename:
`lmx_message_exec.c` has since been narrowed and edited, so filename alone cannot
determine whether a new version has the old architecture.

### A7. Decimal calculator and remaining parity

Use the accepted original calculator syntax; compare reference algorithms.
The user identified a historical decimal->double precision bug: do not port that
bug. Use approved `third_party/decNumber.zip` / current `LMX_DEC.txt` integration,
with exact lexical numeric spans, rounding/errors per the actual contract. Foreign
decNumber stays C; no reason to rewrite it in L1. Coordinate core ABI with Grok.

Complete supported evaluator functions and UI wiring in separate verified slices.
Do not let calculator work preempt the explicitly requested file-manager work.
Later replacing its core with an L3 interpreter is a future stage, not permission
to claim today's numeric engine is already L3.

## 17. Verification, Git and avoiding repeated work

The user explicitly requests commit+push of every verified stage and documentation
update because the machine is unreliable. Do not leave a whole day of accepted
work dirty. Conversely do not commit failing/incomplete work as verified.

- Check shared status AND index before staging. Stage exact owned files only;
  recheck staged names/diff, commit, push normally. No `git add -A`, force push,
  reset-to-safe-version, checkout-all, clean-all or stash-all.
- Never overwrite/remove colleagues' files to make a test pass or an index clean.
  Coordinate overlaps. A moving shared HEAD is normal; a broad reset is not.
- Use `apply_patch` for edits, UTF-8 and LF. Preserve third-party binary/source
  archives and generated-bootstrap exceptions deliberately.
- For docs-only work check links/paths, facts and diff; do not run compiler gates.
- For local source changes select focused checks. Build one translator/support set
  per source revision AND compiler/target/options/defines combination, reuse it for
  compatible tests. Different instrumentation defines need different objects.
- Keep test sources even when removing redundant tests from an always-on suite.
  A real failing test is not fixed by changing its tier or suppressing its marker.
- Saved evidence needs matching source/toolchain/config hashes and expected exits.
  A FAIL marker intentionally injected and asserted can be correct; inspect its
  runner. A successful launcher or old green log is not evidence a new test passed.
- Broad regression, parser differential suites, cross-generation/self-build and
  source-only bootstrap belong at justified integration/promotion milestones, not
  every independent module edit.
- Send completed APIs/evidence to the other owner promptly and advance the next
  already authorized stage. Keep small ownership/progress notes, not huge repeated
  spec dumps that exhaust usage without implementing anything.

## 18. First new-chat report should be concrete

After startup, give the user a short Russian report containing:

1. Which new chat receives the heartbeat and the FSW, intervals and an actual
   received test batch (or a precise unverified/blocking part).
2. Grok's current actual code stage and Codex's agreed implementation slice.
3. Claude's current actual app stage, whether MP3/recovery evidence was reconciled,
   and whether the share ticket has been published to his inbox.
4. Any genuine question requiring the user, without reopening settled semantics.

Then implement. Do not stop at "read the handoff / watchers configured / sent a
plan" when the user has already authorized the next bounded development stage.
