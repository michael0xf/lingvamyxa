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
| Character storage | Graph-unit entry now constructs an owned table; method writes rebind within the target field's table | Verify newest collector/end_turn retention and reclamation in Grok's integration lane |
| Global range catalog | Focused graph-unit char and non-char C no longer imports legacy catalog/pools; old paths remain separate | Audit remaining legacy leaf/foreign paths independently; no full toolchain claim |
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
The graph-unit constructor integration is now completed below; this earlier
helper-only evidence is not itself compiler or collector acceptance.
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

The following exchange resolved the compiler's character table integration
without changes to Grok-owned lifecycle code. Existing user-facing section 25
semantic questions remain open. Runtime/collector/reference-ARRAY remain Grok's.

Grok replied 212100: retain tables through real graph cell references, no hidden
Message singleton; transferred blocks/ranges keep addresses, unrooted tables
remain collectible. This is integration guidance, not implementation evidence.
Source audit found process_message exists only in the synthetic entry; ordinary
methods receive node plus declared/dynamic inputs, not a table/context pointer.
Do not implement the reply's 'passing table' by silently extending source ABI.

Follow-up 212500 proposes a precise representation-based solution for agreement:
after entry initializes every char field from the complete owned byte table,
rebind a proven table cell using base = cell - unsigned_byte_at_cell and select
the new byte within that same allocation. This would require no owner search,
registry, new graph field or source parameter, and preserve original table
identity after transfer. It requires stronger provenance than a generic live
char pointer. Exact helper/compiler/runner/test paths and acceptance checks
are in the ticket. Grok confirmed the contract and exact files in reply 212500,
SHA D54559273017AF589296A8902B5EE879DBA1A97FB974438A80C4DDB9305EAF49.
The agreed implementation is now verified: helper rebind; synthetic entry owned
table; own/nested initial cells; three publication emit sites; fourteenth support
object; focused/legacy-drive compatibility. No lmx_message* changes were needed.

PASS helper `build/codex/chars_owned/20260911_212728_928_3df774e1/evidence.json`:
67892 checks, nine allocations/releases, all 65536 old/new pairs, no mutation/
allocation, post-transfer identity. PASS compiler
`build/codex/l2_message_root/20260911_212942_437_6cc83220/evidence.json`:
105 stages, 24 modes/2135 assertions, eight overlays on eaac7c5, one translator/
fourteen objects. Two simultaneously live Messages, both table allocation
failures and later root/METHOD failures clean up; all old outputs plus three
historical char binding/node drives pass. No legacy catalogs/pools in focused
generated char C. Stable unchanged. This is not latest collector/root/ARRAY,
full historical runner or self-host acceptance. Hand off actual collector
retention/drop evidence and the next non-overlapping integration split to Grok.

English colleague mailboxes and exact-path commits/pushes continue. The stable
compiler stays read-only. Watchers already target the replacement task; see
`work_chat/WATCHER_MIGRATION_20260911.md` for delivery evidence.
