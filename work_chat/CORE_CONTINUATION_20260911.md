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

Grok integration b67589c now reviewed read-only: all ten source/toolchain/exe
hashes in Exec/lmx_message_ctx.hashes.txt match, saved suite/native exit zero
and rooted-char stderr line observed. The new stack-hosted-root test calls
arena_collect directly, then storage_move_all beside a recipient table; it
checks rooted survival, original table identity and eventual reclamation.
This is bounded acceptance, not scheduled end_turn/adopt eligibility or ARRAY.

Completed Grok inbox/outbox213700: l2_need_own previously imported all
l1src/own.lm1, including three mutable fault counters and unused stack/absorb.
Now imports a narrow foreign allocation adapter for the four already admitted
new_zero/resize/copy_bytes/delete calls, preserving their ordinary foreign
allocation and callback contracts; test faults by native linker wrapping.
Exact new helper/test/runner/note plus compiler/focused-runner paths are in the
ticket. Grok confirmed exact ownership in reply SHA
3851611BCA88225D74F535F0DEEE78DD93A650005E486B075B55BD55D148082A.
PASS `build/codex/foreign_alloc/20260911_213847_839_dccb3ffe/evidence.json`:
44 assertions, no live allocations, native failure/callback checks, four exports
and no module data globals. PASS root checkpoint
`build/codex/l2_message_root/20260911_213949_720_0ffec604/evidence.json`:
111 stages, 24 modes/2135 assertions, nine overlays, one translator/14 objects.
Text heap alias/copy/delete and resize with real indent typed accessors pass;
all previous outputs pass, no old allocator counters/stacks. Initial213923
fixture indexed an unsupported local pointer slot; final reuses admitted
parameter-index accessors. No language extension. Full parser and L1 injection
suite were not run; ordinary foreign allocations remain caller-managed.
Do not redo completed P0/query cleanup86d1c50 or change L1 own/parser tests,
Message storage or source ABI. Next independent implementation split must be
coordinated with Grok's active runtime/ARRAY work.

Foreign fixture follow-up: root PASS214132_213_19ea0bfb, same111 stages/24 modes/
2135 assertions, additionally exercises the L2 text-heap OOM route through a
test-only calloc wrapper. Historical L2 heap drive now uses the new predef and
Message-entry splicer. L1 reference/fault tests remain unchanged. See
L2_FOREIGN_ALLOC.txt for exact evidence; do not reintroduce production counters.

Grok214300 proposed the next five-file primitive array storage helper. Codex
confirmed214600 and implemented lmx_array_new_positive_owned for the existing
CHAR/INT array IDs. Separate stable descriptor/backing allocations and ranges
are prepared privately, then admitted atomically into caller heads; failed
allocation/admission frees only prepared storage. This helper supports count>0,
explicitly leaving source empty representation open, with no compiler/collector
integration or new type/tag/owner field.
PASS `build/codex/array_owned/20260911_214707_321_25ee2fc7/evidence.json`:
909 assertions,22 allocations/releases, one four-object C99/O2/Werror build.
Types/strides/alignment/zeroed mutable elements, every allocation failure,
overflow/rejection/retry, paired transfer and one-time disposal verified.
Array CHAR backing is mutable and is NOT valid provenance for intern-table
rebind. See LMX_ARRAY_OWNED.txt. Stable compiler unchanged.

The proposed 'reference arrays versus char/int only' fork was checked against
spec6.5 and handoff22.2: reference-valued arrays/tracing are already required.
Grok was asked to identify any remaining concrete encoding/admission choice,
not reopen whether reference arrays exist. No reference type/stride was invented.

Grok214600 explicitly confirmed the positive-only helper and withdrew the
primitive-only fork. His remaining concern is native admission of T for
Structure versus Array/METHOD referents. User clarification is now pending:
distinct range type ID per concrete T, or general reference-array category
plus explicit T metadata. No answer has been received; do not choose on the
user's behalf or treat elapsed time as agreement. Existing primitive-array
collector integration can proceed independently in Grok's lane (handoff214900).

English colleague mailboxes and exact-path commits/pushes continue. The stable
compiler stays read-only. Watchers already target the replacement task; see
`work_chat/WATCHER_MIGRATION_20260911.md` for delivery evidence.
