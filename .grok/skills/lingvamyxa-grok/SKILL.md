---
name: lingvamyxa-grok
description: >
  Operating rules for Grok on the Lingvamyxa repo (Message exec, collector,
  D7, graph-ABI integration, grok file-drop chat). Use on this repo, any
  inbox/outbox ticket, collector/arena/drive_tree/Message work, or when a
  session starts cold. Slash: /lingvamyxa-grok
---

# Grok on Lingvamyxa

Read this before coding. Language model is
`L2_CORE_AND_MESSAGE_MODEL_20260912.md` (parts I–III) plus
`Lingvamyxa_spec.txt` and `struct_refactoring_version_2.txt`.
Do not treat a green old test or a C function as a language rule.

## Who you are

- **Grok** owns L2/core Message execution, D7 unnesting, collector/GC wiring,
  and integrating Fable's graph ABI into Message create/set_graph later.
- **Codex** plans, reviews evidence, edits SPEC/ABI docs. Do not fight
  documentation-only commits on main.
- **Fable** owns graph copy, ABI files, frontend/emitter, copier.
  Do not overwrite his active copier/ABI checkout.
- **Claude** owns `mixa_manager`. Never edit it.
- Frozen: `REF/parser.lm1`, live STG gen2 `l1trans` sha
  `65D5A5ED127CA1BAEBDD1D500A5B74CEEA63EC1985EAC52EDEF28EFEB261C936`
  at `stg/l1_baseline/build/l1trans/gen2/l1trans.exe`. Fable's original
  graph-abi checkout. Do not bootstrap l1trans for app/runtime edits.

## File-drop chat

Paths: `work_chat/grok/{inbox,seen,outbox}` plus `PROTOCOL.txt`,
`ASSIGNMENT.txt`, `work_chat/TICKET_RULES_EN.md`.

On NEW inbox `*.txt`: claim `seen\<basename>`, do the work, write
`outbox\<basename>` via temp+rename. Chat: `answered <basename>`.
Do not delete inbox. English colleague replies. Russian greetings still
get an English status outbox.

Heartbeat: FileSystemWatcher `watch_grok_inbox.ps1` (pid historically 18088).
Empty inbox is not a reason to invent work. Authorized remaining work is
not starvation if a ticket is in flight.

## Git

- Grok-owned commits only. Exact-path `git add`. Never `git add -A`.
- Never force-push. Never commit `mixa_manager`, vendor, or untracked junk.
- Active graph/collector/D7 work: worktree
  `C:\Nyasha_Planet\lingvamyxa-gabi` branch `grok/collector-slot-values`.
- Keep **main headers unchanged** until a combined stage is accepted for
  integration. Comment-only `lmx.h` on both trees is allowed when ticketed.
- Evidence under `build/grok/<slice>/<timestamp>/` in the main checkout.

## Architecture (do not invent)

- One Structure header `Lmx {node, len, data}`. `data` is `void **` slots.
  Only STRUCT has node/len/data. Child VALUE type comes from owned ranges
  (STRUCT/ARRAY/METHOD/PRIMITIVE/CHILDREN/REF).
- Methods: shared `{addr,sig}` descriptors, not cloned. Call supplies
  invoking Structure as node. Descriptor has no node. The OS-root
  Message owns a second `const: immutable` ARRAY of known descriptors
  until process exit (separate from the eternal-branch ARRAY). Placing
  records there instead of the creating child's arena is not done yet.
- Merge and Message-create use **one** source→copy map for all roots.
  Rewrite destination node/refs through that map. Independent is `node=0`.
- Eternal branch: `independent: const: immutable` (SPEC 9.1.4). The OS-root
  Message holds one ARRAY of references to **every** such branch in the
  OS process (all Messages), not the root's own lexical graph. Branch
  `node=0`. The ARRAY itself is `const: immutable`. Its membership is
  known at translation; it does not grow at runtime. Merge/create
  **substitute the same address** (no deep copy) and do not import or
  walk the ARRAY. Construction may still fill a branch with runtime
  values before publish. **Not implemented yet. Do not invent the ARRAY
  or a global pool.**
- Ordinary immutable/const is not automatically eternal or cross-Message.
- Local arena handoff (`storage_move_all`) is **not** merge. Same objects,
  same addresses, owner changes. Create/merge copy used graph into a new arena.
- Collector is nonmoving. Live L1/L2 entries never relocate, including at
  `end_turn`. Growth is by chunk. `arena_collect` is L1 with stack `LmxVisit`
  via `predef: "l2src/lmx_msg_visit.h.lm1"` (not only generated C include).
- Mark STRUCT via `mark_from` (header, node, data, CHILDREN slots). ARRAY
  via `mark_array`. METHOD `visit_add_ptr`. OOM on visit: do not sweep.
- GC once per `end_turn` after outbox publish or discard. Not on method
  return/retry/caught throw.
- `dest_msg` is a refcounted control capability. No process-wide Message table.
- Lock order: exec then mail, never mail then exec. Snapshot/retain under
  exec, drop exec, then mailbox or child turn, then revalidate.
- `running=1, success=0` at slot alloc (`lmx_msg_slot_new`). `success=1`
  only on actual assigned-work completion. `running=0` is stop/cancel, not
  success. Distinct from `state` (INACTIVE/RUNNING/STOPPED/DEAD/RELEASED).
- Path: child extends parent; first local id is 1. Create idempotent on
  `(parent, create_id)`.
- Current create copies **init bytes**, not the used graph. Do not claim
  create already runs Fable's copier.

## Tests and evidence

- Targeted Exec: `stg/l1_baseline/l2src/run_lmx.ps1 -Suite Exec`
  from `l1_baseline`, timeout often 120–180s (compile+suite).
- Passing retry does **not** fix the unexplained nest hang after `m0_acc`.
  Do not declare that hang fixed.
- Overlap tests must prove the competing lock is taken **while** the other
  lock is still held (`hook_got_go` / Wait timeout = fail). Eventual progress
  after timeout is a false PASS.
- Do not repeat an identical accepted Exec. Do not run Fable 95 fixtures
  or l1trans bootstrap for Message-only edits.
- POSIX runtime is compile-only unless a ticket says otherwise.

## Do not

- Implement merge/shallow copy, copier, or frontend lowering.
- Clone method descriptors or add per-node layout descriptors.
- Relocate live arena entries.
- Hold exec across `mail_inbox_empty` / user handler / child turn.
- Walk `first_child/next_sibling` after dropping exec without a retained snapshot.
- Skip DEAD parents in `drive_tree` (descendants still need close).
- Treat Codex/Fable worktrees as yours (`build/codex/fable-abi-review`,
  Fable graph-abi original).
- Invent language at a fork; ask one concrete question in outbox.

## Dated snapshot (2026-09-12; re-read git)

- gabi `grok/collector-slot-values` includes D7 through `040af5d2`,
  collector slot-values, STRUCT history_kind, drive snapshot, drive-close
  mail-outside-exec, overlap `hook_got_go` (`e5ba5119`).
- Fable copier reported `0763a2cf` (one map, `eternal_ranges`); Codex
  acceptance may lag. Coordinate, do not invent a second copy model.
- Method lifetime after source Message teardown is still open.
- Eternal retention ARRAY is documentation only.
