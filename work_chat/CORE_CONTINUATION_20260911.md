# Current core continuation

The architectural plan is `L2_CORE_SELFHOST_HANDOFF_20260911.md`, read in the
replacement Codex task on 2026-09-11. Its dated initial assignments are now
completed; do not restart them from the old snapshot.

## Verified checkpoints

- `d21aed6`: Codex's exact seven-file known-character-read slice, committed and
  pushed. Evidence: `build/codex/l2_message_root/20260911_210611_852_e08aab70/evidence.json`.
  76 stages, 19 driver modes / 908 assertions; char allocation remains legacy.
- `641f456`: Grok's `arena_collect` L1 extraction. Codex reviewed the three-file
  diff, matched all ten source/compiler/executable entries in saved
  `stg/l1_baseline/build/l1trans/logs/gen2/lmx_Exec/lmx_message_ctx.hashes.txt`,
  and read the suite/native exit-zero PASS. The local L1 visit predef resolves
  the reported type blocker. Sweep order and OOM skip/disposal are preserved.
  This is bounded extraction acceptance, not complete root/domain coverage.

## Remaining active-path gaps

| Area | Observed source evidence | Next boundary |
| --- | --- | --- |
| Character storage | `lmx_chars.lm1` defines global `lmx_chars_pool`; graph-unit emission still calls `lmx_chars_init` | Explicit owner-local table construction, then separately agreed compiler/context integration |
| Global range catalog | `lmx_range.lm1` defines table/count/capacity; actual own char fields still request `lmx_ranges_init` | Integrate Message-owned character storage before removing the remaining char dependency |
| Text/query adapters | Removed unused `lmx_size.lm1` import and P0/query-only range initialization; borrowed-data contracts unchanged | Focused P0/query/mixed-char evidence below; no general pointer-lifetime claim |
| Collector roots/domains | Current mark starts from `m->graph`, follows lexical/child edges, marks ARRAY descriptor/backing and METHOD pointers | Active/result/continuation roots and reference-valued array traversal still need exact integration evidence |
| Message runtime | Grok's 210800 reply identifies D3/D7 host bind/EXEC-ring prototype remnants | Grok retains Message/lifecycle/native runtime ownership and K2 completion |

No generic traversal policy, merge representation, post-bind address behavior or
source-method owner parameter was invented during this review. Open decisions
in handoff section 25 remain explicit; unrelated open corners do not block
independent storage work.

## Next split

Codex proposed five new `lmx_chars_owned` helper/test/runner/note paths in Grok
inbox `20260911-211100.txt`. The proposal is a pure owner-local table constructor
and explicit-table lookup with stable byte identity, failure cleanup and paired
storage transfer tests. It does not claim removal of generated-program globals.
Grok confirmed all five paths in the matching 211100 reply. The helper is now
implemented and its isolated run passed:
`build/codex/chars_owned/20260911_211338_746_fe7792a5/evidence.json`, 1585 checks,
9 allocations and 9 releases. Committed and pushed as `51959f8`.
The constructor still requires later integration
with the selected Message-local table; generated-program globals remain.
Grok retains collector, reference-ARRAY and Message lifecycle/runtime work.

Completed split, inbox/outbox `20260911-211500.txt`: remove the unused legacy
size/branch/char/range import chain from the pure text-hash adapter and the
unnecessary range initialization from P0/query-only generated units. Preserve
legacy character imports where actual char fields still need them; preserve
query/text behavior and borrowed-data contracts. Grok confirmed all paths;
implementation changes only hash predef/compiler emission, focused runner/note.
PASS `build/codex/l2_message_root/20260911_211950_819_818fdf57/evidence.json`:
88 stages, 19 driver modes / 908 assertions, four new P0/query/mixed-char cases.
One translator/thirteen support objects from eaac7c5 plus six hashed overlays.
No legacy catalogs/pools in char-free generated C. Mixed char still works.
No full parser/self-build or latest-collector integration claim.

Next boundary is canonical character table selection and Message/runtime
integration of the already-tested helper. That touches Grok-owned lifecycle
code and must be agreed explicitly; do not invent a hidden source-method owner
argument or global/TLS lookup to connect it. Existing user-facing section 25
semantic questions remain open. Runtime/collector/reference-ARRAY remain Grok's.

English colleague mailboxes and exact-path commits/pushes continue. The stable
compiler stays read-only. Watchers already target the replacement task; see
`work_chat/WATCHER_MIGRATION_20260911.md` for delivery evidence.
