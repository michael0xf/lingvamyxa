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
Structure versus Array/METHOD referents. RESOLVED by the user on2026-09-11:
"отдельный идентификатор для каждого типа элемента T   :)".
Use a distinct concrete type identifier for each element type T in the typed
service-range metadata. Classify an array reference by its descriptor address;
one typed pool can have multiple stable block ranges with that same type ID.
Do not substitute one undifferentiated reference-array type. The descriptor
still contains only len/data; no per-element tag, added Structure schema,
global payload registry or ownership link follows from this decision.
This settles the pending type-admission direction. Grok can continue concrete
reference-array admission/tracing after his primitive-array collector checkpoint.
Empty representation and other unrelated section25 decisions remain open.

Grok88a457e/e15aa90 reviewed read-only: ten Exec source/toolchain/executable
manifest hashes match, native exit0 and primitive/reference-array stderr cases
verified. Distinct ARRAY_OF_LMX/DESC/METHOD descriptors and KIND_REF backing
with corresponding T are admitted; collector walks the three reference kinds.
This is direct arena_collect coverage, not complete scheduled end_turn or
arbitrary source T admission. Existing type numbers unchanged.

Codex accepted the five-file reference constructor slice215200 via220400.
Implemented lmx_array_ref_new_positive_owned for those three IDs, positive
count only, private descriptor/backing preparation then atomic admission.
PASS array_ref_owned/20260911_220321_200_a5c81482/evidence.json:
453 checks,66 allocations/releases, four support objects, stable compiler
unchanged. Typed/null cells, ranges, all twelve OOM cases, rejection/retry,
same-T multiple ranges, paired transfer and disposal verified. See
LMX_ARRAY_REF_OWNED.txt. No compiler or Message integration in this slice;
Grok owns integration/collector. Next independent slice needs agreement.

Grok ae5e60c now integrates the real reference constructor, adds a rooted DESC
cycle/shared descriptor and verifies an unrooted referencing array is freed.
01d984d adds direct end_turn CHAR retention/INT reclamation and root drop.
Source diffs reviewed; ten current Exec hashes match, native exit0 and both
stderr cases observed. The end_turn test calls the API directly with a stack
root; it does not establish full scheduler/active evaluation coverage.

Codex has_ptr slice agreed in220600/220900: existing visit header/module,
selftest/note only, runner unchanged. lmx_msg_visit_has_ptr scans raw addresses,
null-safe and searchable after OOM, without allocation/mutation. PASS
msg_visit/20260911_220800_011_5def6510/evidence.json148checks,3frees,10reallocs,
C99/O2, stable unchanged. Grok can replace local lmx_msg_seen_ptr in collector;
no root/mark policy change. Next ownership must be agreed after handoff.

424d995 integration accepted read-only: collector calls visit_has_ptr and local
scan is removed; ten Exec hashes match, nativeexit0 and cycle/shared case passes.
Grok221000 proposes source CHAR/INT constructor emission. Investigation found
only c.array foreign local support in l2_array_local/l2_emit_loc_stmt; no source
[] graph-array admission exists. Codex221300 asks exact first fixture/body and
graph-host/lifetime subset before edits. Need parser/own metadata/emitter plus
run_l2trans linking/focused runner ownership, not merely a test splicer call.
Spec6.5 says runtime [] construction creates descriptor/backing; an Lmx field
holding it is separate. Do not silently substitute c.array, static allocation
or handwritten test L1 for source-array support.

221300/221600 RESOLVED the exact six-file slice: top-level method own []:
int/char NAME positive-decimal COUNT, existing unit-child placement, descriptor
in leaf.data and constructor in synthetic entry. IMPLEMENTED and verified.
Own metadata4/5, preserved count through growth, conditional array predef,
15th support object; no scalar cache, array indexing/rebinding or empty ABI.
PASS l2_message_root/20260911_222127_766_c9e25dab/evidence.json197stages,
34modes/2693assertions,55compiler-OOM cases,15source rejects,12overlays.
Two actual source arrays/type/extent/zero/publication, all8constructor faults,
laterMETHODfailure cleanup; six-array metadata growth and all allocation
faults leave live0/existing output intact. Previous focused outputs pass.
This stays eaac7c5 snapshot+owned overlays, not newest runtime/scheduler or
full selfhost. See L2_MESSAGE_ROOT.txt. Grok owns runtime integration; next
compiler slice must be explicitly coordinated, no repeated helper builds.

Grok222056 identified explicit native roots beyond graph as the next runtime
gap. Ticket222500 activates it after222400 integration: owner-local attach/
release for graph/Array references surviving end_turn, domain tracing, failure
atomicity and teardown, with real end_turn retained/released/unrelated-root
tests. No global owner registry/per-Lmx field or whole-arena retention. Grok
owns runtime/API/harness; this native-root foundation is not complete compiler
activation/result/continuation integration. Pin those sites after it lands.

Follow-up22:55: Grok0b5a79a reviewed; ten saved Exec manifest hashes match,
nativeexit0 and explicit INT/root-drop/emitted-shape markers verified. The
publication test is handwritten C matching emitted layout, not actual L2 output
linked with latest runtime. Native roots attach/release ARRAY/CHILDREN addresses;
current case covers primitive descriptor only. Ticket225400 asks remaining
graph/ref-array/OOM/teardown cases and audits storage transfer with source roots.
No redundant permanent descriptor roots when already reachable through graph.

Bounds fork is resolved by existing spec around8046: out-of-range indexing is
throw: Bounds, never return0/assert/sentinel. Codex225400 proposes exact compiler
ownership for a statically proven literal-index INT own-array subset; reject
dynamic/negative/OOB until actual Bounds machinery, no fake runtime fallback.
Agreement received in225400.progress and completed reply; implementation below.
Grok keeps native runtime/collector.
Grok225056 SHA224C8C9E2B481609A0F611F556CA3E61FF9A5A9B5E3A484F751848D2B14FF2D8
is progress/next-assignment request, not new implementation evidence.

225400 reply SHACE69213359B508D281931F8E14D71BB80DD21E881C741EC3F8D074EF35F128E2
confirms exact Codex compiler slice and delivers0c714cd. Ten Exec hashes match,
nativeexit0; graph root/ref-DESC/attach-OOM/leftover teardown and transfer cases
source-reviewed. handoff_move_locked removes stale source roots after storage
move; destination does not inherit permanent retention. Existing eligibility
guards unchanged. Root set membership is not multiple independent leases;
real activation/continuation lifecycle registration remains unfinished.

Codex literal-index implementation committed/pushed52c15de, now PASS:
build/codex/l2_message_root/20260911_230127_765_2589b4ec/evidence.json,
224stages/35modes/2752assertions,13overlays/15supportobjects,55compiler-OOMs.
Actual INT source first/last stores7/9, reads return16, middle stays0, typed
descriptor and all tracked cleanup checked. First-cell7/cell-to-cell23/printf9
cases pass;14 unsupported-index plus15 prior array rejects. See updated
L2_MESSAGE_ROOT.txt. No general checked index, Bounds throw plumbing, CHAR,
rebind/escape or full newest-runtime/selfhost claim. Runtime integration must
compile generated code with matching current Message headers (roots changed
the layout); do not link old archived-layout objects into new runtime.
Ticket230400 delivers this stage and assigns Grok actual generated-code/runtime
retention/drop integration in his runner/harness files, then an exact proposal
for Bounds plumbing or the next active/result-root gap. Codex retains compiler
ownership pending the next non-overlapping agreement; no repeat helper build.

Grok230400 SHAFC6471A02DF6A9416D095AB3F23FC0B44BDD454EFBF6B87A85B2209D9C10A1D5
delivers4b50c97. Saved L1 exactly matches Codex230127 output, retranslated/
compiled against current Message headers. Ten Exec hashes match, generated
test nativeexit0/stderr verifies7/0/9 retained through end_turn, dead neighbour
reclaimed and graph-drop releases array. This closes the actual emitted-code
integration gap, not scheduled activation or full selfhosting.
Runner issue: normal Exec now depends on an untracked dated private build file;
its manifest omits generated fixture/harness/exe. Ticket231000 asks reproducible
explicit invocation or tracked generated provenance and matching saved hashes.

Bounds ABI decision is now ASKED OF USER in this task (pending): explicit status
plus typed output slots (recommended) versus cleanup-preserving non-local
transfer. Handoff25/spec21.G leave the physical carrier open; spec19.11 already
requires payload retention, dirty publish/finally/publish and caller catch,
never diagnostic-root escape. Do not invent an ABI or reuse cancel poll_escape.
While pending, Grok231000 activates a bounded actual result/failed-history root
lifetime site after runner repair. Codex proposes the same literal-index path
for CHAR own arrays in exact compiler/runner/driver/note/new fixture paths;
ownership confirmed231000; completed below. Native char maps to C char per spec8 bootstrap
map; no new encoding, intern-table provenance, dynamic Bounds or reference ABI.

231000 reply SHA5112D2C15BDD35F4B6537443C27E0DD341493091854B08EB22AF2687BAA0A0C6
delivers59a5e56: tracked authentic generated L1/provenance replaces private-run
dependency,15 manifest hashes match and both native exits0. Runner repair
accepted. New adopt_failed root is NOT accepted: allocation failure after move
is ignored and loses mandatory history; dropping child's separate roots also
loses failed grandchild history not reachable through child.graph. Ticket231500
requires preprepared atomic root/storage admission and nested failed P/C/G
retention, preserving ordinary adopted-but-unretained collection. Grok owns fix.

Codex CHAR literal-index slice implemented and PASS:
build/codex/l2_message_root/20260911_231614_144_a6b3bf90/evidence.json.
237stages/36modes/2812assertions and55compiler allocator failures.
Same static proof, CHAR pointer/load temps, mutable backing/no intern rebind.
Actual bytes65/0/62 and result127, typed byte range/descriptors, tracked cleanup;
printf62 and six additional bad-index cases pass with all existing INT/focused
checks.14overlays/15supportobjects, stable unchanged. No result/throw ABI chosen;
the user's carrier question stays pending. See L2_MESSAGE_ROOT.txt. Committed/
pushed13c51f8 and delivered231800 without interrupting Grok's runtime fix.

Grok231500 SHA6C8A0DB219A4E5FD06778490274B8458F2C285559F0EB37C983C88988FE23441
and231800 SHA6F2D25DFB4D4DCAC00128F020029B832D13A4C3141E4BB8EDCE1D3F952EF563F
deliverfec75be.15saved Exec hashes match,both exits0. History root preparation
now precedes storage move; injected root-OOM preserves owners and retry works.
Nested failed P/C/G case retains C=9/G=7, unrelated neighbour dies. Limited
acceptance of those paths; prepare_failed_history still copies all roots without
distinguishing history from ordinary temporary/native retention. Ticket232200
asks minimal role/lifetime distinction and a paired-role regression. Claimed
post-move init push limitation appears unreachable for valid exclusive state:
fresh detached non-null init block, acyclic moved list, nonallocating push.
Grok to document proof or supply admitted counterexample, not invent rollback.

Grok231800 proceeds with actual generated CHAR/current-runtime collect proof
using tracked authentic L1/provenance/manifest. Codex232200 proposes independent
length(own INT/CHAR array) lowering to the live descriptor's size_t len, while
preserving a declared source method named length. Exact compiler/runner/driver/
note plus tests/unit_own_array_length.lm2; confirmed232200 and completed below.
User's result/throw ABI choice is still pending; no assumption made.

232200 reply SHAFB91B22D91FF9E566E2A914634B694A155EC3EC5A6035781F85DE53EDB24FA8B
deliversbdd6a62. Source-reviewed HISTORY/RETAIN role bits share one address;
adopt_failed copies only history, ordinary release clears RETAIN only; commit
merges HISTORY with existing RETAIN.15Exec hashes match,both exits0 and role/
OOM/nested/unretained/generated-INT cases verified. Init-push proof documented.
232056 SHABB85617965BB2DF253C30917A8FE6613386FAE2EBED8E14F20FBD03301749DF3
is matched progress request/reply, not extra implementation evidence. Grok is
continuing actual generated-CHAR/runtime collect with tracked fixture/provenance.

Codex length slice PASS:
build/codex/l2_message_root/20260911_232721_400_ce47fb1b/evidence.json,
250stages/37modes/2888assertions,15overlays/15objects,55compiler-OOM. Builtin
length(ownINT/CHAR) emits live descriptor size_t len, no reference escape. INT3+
CHAR4 returns7; native-only descriptor shortening2/1 then same method returns3
(source resize not admitted). Source-defined length keeps normal dispatch,
returns9; six bad builtin argument/arity forms rejected, all prior focused
tests pass. Stable untouched; no Bounds/result ABI choice. See L2_MESSAGE_ROOT.

Committed/pushed bb4c634, delivered to Grok234500. No redundant runtime length
harness requested; his generated-CHAR/current-runtime integration remains active.
Next independent Codex proposal234500 awaits exact ownership confirmation:
port existing bdd6a62 history prepare/commit/discard C algorithm into new L1
lmx_msg_history_owned.h.lm1/.lm1, tests/lmx_msg_history_owned_selftest.lm1,
run_lmx_msg_history_owned.ps1 and LMX_MSG_HISTORY_OWNED.txt. Grok retains exec/
headers/lifecycle and integration after helper handoff. Preserve exact role
policy; focused later-allocation OOM rollback, deduplication and role merge,
test-only allocator wrapping. No helper edits until matched agreement. Physical
result/throw carrier is still pending user choice; independent migration proceeds.

234500 reply SHAA993C0C2A42289DBCA23CE2C7D0570F8809654F846ACEE87DBD55450D5009C0D
confirms exact history-helper split and delivers7d05282. Tracked CHAR L1 hash
617A8D311F9548EE5511F947A602B8102327F1A7EB7D8FE7246748A2DA6FE084 exactly
matches Codex231614 capture. All20 Exec manifest hashes match, all3native exits0;
harness checks actual generated65/0/62, retain through end_turn, dead neighbour
and graph-drop reclamation. This completes that bounded generated-CHAR proof.

Codex history L1 extraction IMPLEMENTED/PASS, exact five new files above:
build/codex/msg_history_owned/20260911_235051_624_8c67c678/evidence.json,
51checks/failures0/live0/malloc12/free9 (3 injected failures). Frozen compiler,
C99/O2/Werror; no mutable production storage, onlymalloc/free/range-find imports.
Every allocation position including partial-prefix failure rolls back privately;
owner heads/roles/payload remain intact, retry/duplicate/role merge/null/empty
and unclassified cases pass. Test-only per-object allocator substitution avoids
MinGW CRT allocations contaminating global linker-wrap counters. Full details
in l2src/LMX_MSG_HISTORY_OWNED.txt. Grok next integrates the3functions, removes
duplicate C code, preserves root-OOM regressions and verifies affected Exec
cases; no scheduler or result/throw semantics chosen by this extraction.

History helper committed/pushed9ca34e8, delivered235200. Grok235200 reply
SHA60678DF10BCF46E49E9A0C125B7DC7EA93134FBB72595FB0C568FD4C859EE570
delivers01a245b: runtime adopts L1 prepare/commit/dispose, duplicate C removed.
All20 Exec manifest hashes match, all3native exits0; role/nested/OOM/transfer
markers pass. Object evidence records helper C hash, headers and local TEST
malloc/free substitution using existing root-failure hook. No standalone rerun.

Same reply proposes exact5stale-root helper paths; Codex235800 accepts and
IMPLEMENTS lmx_msg_roots_drop_stale(m) in L1, preserving exact classification
policy (any classified address stays; roles/kind do not narrow that predicate).
PASS msg_roots_stale/20260911_235842_725_813b8336/evidence.json15stages,
27checks/failures0/eight distinct bookkeeping frees. Head/middle/adjacent/tail,
one-past, role/order/pointer preservation, repeat/empty/no-ranges and payload/
graph/storage integrity covered. No allocator/global mutable storage, only
free/range-find imports. See l2src/LMX_MSG_ROOTS_STALE.txt. Grok retains exec
splice/removal and affected runtime checks after handoff; no throw ABI choice.

Stale helper committed/pushed ef6c3c3, delivered000000. Grok000000 reply
SHAF54C4C3BE55A72A5711BB2D2E037D7D21EFFECE6F6C0EEB7A14F2E48D25202F2
delivers f8257a5. Both post-move sites call L1 helper, C duplicate deleted;
20Exec hashes match and3native exits0, history allocator setup preserved.
No additional standalone run. Grok proposed redundant generated length collect;
ticket000700 redirects to actual remaining runtime defect: signed cap*2 and
size multiplication in ready_grow/bind_grow lack representability checks.
Grok owns coherent checked-capacity/reservation fix and arithmetic/OOM/retry
regressions in runtime/runner/selftest; preserve8start/no arbitrary cap and
scan/failure semantics. May be part of larger D3 bind/ready migration. After
checkpoint, identify evidenced K3/K5 compiler gap/coherent split for Codex.
Only carrier-dependent dynamic Bounds is waiting for the user's ABI choice;
do not label all compiler/selfhosting work blocked or manufacture tiny helpers.

English colleague mailboxes and exact-path commits/pushes continue. The stable
compiler stays read-only. Watchers already target the replacement task; see
`work_chat/WATCHER_MIGRATION_20260911.md` for delivery evidence.
