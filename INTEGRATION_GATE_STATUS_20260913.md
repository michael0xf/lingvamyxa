# Integration gate status — 2026-09-13, branch `integration/main-absorbs-core`

What is measured here was run on the merged branch in the worktree
`build/fable/integration`, with the pinned 65D5 L1 building `l2trans` and the
merged tree's own freshly built L1 translating the generated L1. That second
half is not a convenience: the integrated compiler emits addresses of depth
three and positional backslashes, which the 65D5 pin cannot spell, so the pin
alone can no longer carry this tree end to end.

## 1. Core gates — green

| Gate | Result |
| --- | --- |
| `l2src/run_graph_abi.ps1` | PASS — selftest 63/0, copy 75/0 over 69 allocation-failure positions, pointer Array 21/0, merge 261/0 over 43 positions, fixtures 138/138, all 41 negatives |
| `l2src/run_l2trans.ps1` | `l2trans gen2 ok` |
| `l2src/run_lmx.ps1` | `lmx own selftest ok`, `l2 lmx gen2 ok` |
| `run_port_msg_storage.ps1` | PASS 77/0 |
| `run_port_msg_slots.ps1` | PASS 278/0 |
| `run_port_msg_path_storage.ps1` | PASS 541/0, 11 wrapped allocations |
| `run_port_owned_ranges.ps1` | PASS 439/0 |
| `run_port_msg_blocks.ps1` | PASS 143/0, 147 frees, 2 disposer callbacks |

## 2. Full L1 gate — four red, all inherited

`stg/l1_baseline/gate.ps1` on the merged branch: FAILED — `run_gen`,
`gen0 run_decl_repeat`, `gen0 run_ident`, `gen2 run_ident`, plus
`gen0 l2 run_lmx` which is red on `origin/main` as well.

The merged tree's L1 sources are byte-identical to `origin/codex/core-integration`,
and `origin/main` contains none of Codex's L1 commits, so none of this came from
the merge. `origin/main`'s own gate fails only on `gen0 l2 run_lmx`.

Two root causes, both established rather than guessed:

1. **The gen0 seed is one commit behind.** gen0 is built from
   `lm2/l1trans.lm2` by `trans.lm0`; commit `24a3e4c7` ("restore C surface
   assignment targets", 14:13) changed `l1src/l1trans.lm1` in both copies and
   never regenerated that seed. Evidence: gen1 C and gen2 C differ in exactly
   18 lines, all of the shape `stack->count` versus `stack -> count` inside an
   index — what that commit changes — while **gen2 C == gen3 C byte for byte**,
   so the current source does reach its own fixed point. The stale seed also
   explains `gen0 run_decl_repeat` and `gen0 run_ident`: gen0 is simply older
   than the tests now assume.
2. **`gen2 run_ident` is a real defect in the newer L1.** "quoted name leaked
   into C" on `` `value`: value + 1 ``, emitted as `` `value` = value + 1; ``.
   The same commit replaced `l1_emit_assign_head`'s call to `l1_c_ident` (which
   strips backticks) with a hand-rolled span walk that writes raw bytes, so the
   assignment target's root name keeps its quotes. `origin/main` passes this
   test.

Until both are closed the compiler pin cannot be promoted (decision 2 of
2026-09-13 gates promotion on a green gate), and self-hosting stays blocked
behind it.

## 3. mixa parity — 35 runners against the integrated compiler

Four pass outright: `app_fmpanel`, `app_path`, `help`, `selection` — exactly the
four Codex verified against the graph runtime on his branch. The rest is the
actionable queue, and it says plainly that **the merge alone does not unblock
the manager ports**: most modules still stop in the translator, several of them
further along the same line than before (`remove_confirm` moved from column 31
to column 89), which is progress, not completion.

```text
app_controller         exit=2  unknown foreign type               l2trans error: mixa_manager\mixa_app_controller.lm2:74:58: unknown foreign type
app_fmpanel            exit=0  no translator diagnostic           
app_main               exit=2  no translator diagnostic           
app_panel              exit=1  import root                        l1trans error: cannot read import l2src/lmx_array_ref_owned.h.lm1
app_path               exit=0  no translator diagnostic           
app_window             exit=1  no translator diagnostic           
button_dispatch        exit=2  incompatible entry signature       l2trans error: mixa_manager\mixa_button_dispatch.lm2:25:1: incompatible entry signature
buttons                exit=1  import root                        l1trans error: cannot read import l2src/lmx_array_ref_owned.h.lm1
cmdline                exit=1  import root                        l1trans error: cannot read import l2src/lmx_array_ref_owned.h.lm1
cmdline_dispatch       exit=2  unknown foreign type               l2trans error: mixa_manager\mixa_cmdline_dispatch.lm2:48:84: unknown foreign type
composite              exit=1  no translator diagnostic           
composite_glyphs       exit=1  no translator diagnostic           
console_window         exit=2  incompatible entry signature       l2trans error: mixa_manager\mixa_console_window.lm2:42:65: incompatible entry signature
copy                   exit=1  unsupported own array declaration  l2trans error: mixa_manager\mixa_copy.lm2:92:5: unsupported own array declaration
dir_win32              exit=2  unknown foreign type               l2trans error: mixa_manager\mixa_dir_win32.lm2:54:34: unknown foreign type
draw                   exit=1  no translator diagnostic           
event_fifo             exit=1  import root                        l1trans error: cannot read import l2src/lmx_array_ref_owned.h.lm1
file_manager           exit=1  unsupported own array declaration  l2trans error: mixa_manager\mixa_file_manager.lm2:181:5: unsupported own array declaration
file_win32             exit=1  unsupported own array declaration  l2trans error: mixa_manager\mixa_file_win32.lm2:122:5: unsupported own array declaration
fileio_win32           exit=2  incompatible entry signature       l2trans error: mixa_manager\mixa_fileio_win32.lm2:30:23: incompatible entry signature
fm_copy                exit=2  unknown foreign type               l2trans error: mixa_manager\mixa_fm_copy.lm2:98:38: unknown foreign type
fm_remove              exit=1  import root                        l1trans error: cannot read import l2src/lmx_array_ref_owned.h.lm1
help                   exit=0  no translator diagnostic           
highlight              exit=1  no translator diagnostic           
pointer                exit=2  incompatible entry signature       l2trans error: mixa_manager\mixa_pointer.lm2:60:26: incompatible entry signature
process_marker         exit=1  unsupported own array declaration  l2trans error: mixa_manager\mixa_process_marker.lm2:278:5: unsupported own array declaration
process_win32          exit=2  unknown foreign type               l2trans error: mixa_manager\mixa_process_win32.lm2:105:25: unknown foreign type
pump                   exit=2  unknown foreign type               l2trans error: mixa_manager\mixa_pump.lm2:7:21: unknown foreign type
remove                 exit=2  incompatible entry signature       l2trans error: mixa_manager\mixa_remove.lm2:15:25: incompatible entry signature
remove_confirm         exit=2  unknown foreign type               l2trans error: mixa_manager\mixa_remove_confirm.lm2:4:89: unknown foreign type
selection              exit=0  no translator diagnostic           
selection_walk         exit=2  unknown foreign type               l2trans error: mixa_manager\mixa_selection_walk.lm2:61:34: unknown foreign type
share                  exit=1  unsupported index                  l2trans error: mixa_manager\mixa_share.lm2:110:17: unsupported index
text_rect              exit=1  no translator diagnostic           
tiles                  exit=1  no translator diagnostic           

no translator diagnostic             12
unknown foreign type                 8
import root                          5
incompatible entry signature         5
unsupported own array declaration    4
unsupported index                    1
```

### What each bucket means

- **import root (5).** `l1trans error: cannot read import l2src/lmx_array_ref_owned.h.lm1`.
  These modules translate through `l2trans` cleanly; the generated L1 then
  imports a core header by a path relative to `stg/l1_baseline`, and the mixa
  runner invokes the L1 translator from the repository root. A runner/working
  directory question, not a language one, and the first thing to fix because it
  is hiding whatever those five modules would report next.
- **unknown foreign type (8).** Still the largest group. The aggregate pointer
  admission from `7d7ec87c` covers the shapes the runtime modules use, not
  these.
- **incompatible entry signature (5).** Includes the function-pointer-typed
  formal Claude isolated in `mixa_remove` (`MixaRemoveNodeFn: f`).
- **unsupported own array declaration (4).** `copy`, `file_win32`,
  `file_manager`, `process_marker`.
- **unsupported index (1).** `share`.
- **no translator diagnostic (12).** Four of them are the passes; the others
  fail later, in the harness build or the ABI probe, and each needs its own
  look. Nothing here should be called a compiler gap without reading its run
  directory first.

## 4. Order of work this implies

1. The L1 seed and the backtick regression (section 2) — they block the pin
   promotion, and the pin promotion blocks self-hosting.
2. The import root (one runner-side fix, unblocks five modules' real
   diagnostics).
3. The three remaining translator buckets, largest first.
4. Re-run all 35 and re-measure before telling Claude anything about a lifted
   barrier.


---

## 5. Update, 21:45 — one of the four closed, and the other three are one thing

`gen2 run_ident` is **green**. The backtick stripping on the assignment
target's root name is restored (commit on this branch); the fixture now reaches
C as `value = value + 1;` with no backtick anywhere in the output. That was a
real regression from `24a3e4c7` and it is gone.

The gate now reads: FAILED - `run_gen`, `gen0 run_decl_repeat`,
`gen0 run_ident`, `gen0 l2 run_lmx` (the last one red on `origin/main` too).

All three remaining are the **same** thing, and it is structural rather than a
defect to patch. Measured, not inferred:

    stg/l1_baseline> build/l1trans/gen0/l1trans.exe l1src/own.lm1 own_gen0.c
    stg/l1_baseline> build/l1trans/gen2/l1trans.exe l1src/own.lm1 own_gen2.c
    diff: 3 sites, e.g.
      gen0:  stack->items[stack -> count] = item;
      gen2:  stack->items[stack->count] = item;

gen0 is the seed. `buildCore.lm0.bat` says of its two archives, in its own
words, that they are "restored, never rebuilt" -- `libparser.lm0.a` and
`libown.lm0.a` are frozen old-chain artifacts, and `run_seed` calls them
external prerequisites. So gen0 embeds the FROZEN parser while gen1 onward
embed `l1src/parser.lm1`. The index content of an assignment target is
re-parsed by `l1_emit_assign_index` and emitted from the resulting tree, so its
rendering depends on how the linked parser splits `stack\count` -- one atom in
the current parser (compact `->`), separate fields in the frozen one (spaced).

Consequently `run_gen`'s gen1 == gen2 comparison cannot hold across ANY parser
change that reaches the l1trans source, because gen1 is produced by the frozen
parser and gen2 by the current one. It is green on `origin/main` only because
`origin/main` has no such change. The same explains `gen0 run_decl_repeat` and
`gen0 run_ident`: those suites run tests written for the current translator
against a translator that is by construction older.

What holds and what does not:

- `gen2 C == gen3 C` byte for byte. The current source reaches its own fixed
  point; that is the invariant that says the translator is self-consistent.
- `gen1 C == gen2 C` does not hold and cannot be made to hold by editing
  `lm2/l1trans.lm2`, because the divergence comes from the linked parser, not
  from that source.

Two honest ways out, and this is a contract decision about the bootstrap rather
than something to patch quietly:

1. Make the assignment target's index emission independent of parser
   granularity -- emit it from its bytes the way the identifier path does,
   instead of re-parsing it into a tree. This restores gen1 == gen2 and keeps
   the gate as strong as it was, at the cost of what a re-parsed index can
   express.
2. Re-found the check: treat gen1 as a bootstrap step and require the fixed
   point at gen2 == gen3, which is the property that actually certifies the
   translator, and say so in the gate with the reason written down.

Nothing here is weakened on my own judgement. Until this is settled the
compiler pin is not promoted and the 65D5 pin stays as it is.


---

## 6. Update, 22:05 — the import root is closed, and what it was hiding

`L2_RUNTIME_ROOT` plus source-relative predefs in the handwritten runtime
headers closed the import failure. Measured on the integration branch with
Claude's runner half (`6e8272e1`) merged in: of the six runners that carried the
`cannot read import` error, **none** still reports it. Five now fail one layer
further on, and that layer was always there -- it was simply unreachable before.

    mixa_app_panel_l2.c:2:10: fatal error:
      stg/l1_baseline/l2src/lmx_array_ref_owned.lm1.h: No such file or directory

A `predef:` becomes a C `#include` of the generated `<name>.lm1.h`, spelled with
the same path the predef used. The graph gate satisfies this by generating that
whole header set into a `headers/l2src/` tree inside its run directory and
compiling with `-I` pointing at it -- for example
`build/graph_abi/<run>/message_support/headers/l2src/lmx_array_ref_owned.lm1.h`.
The manager harness never built that set: its own `headers` directory holds only
`mixa_manager`, because translation always stopped before the runtime headers
were needed.

So the remaining work on this thread is in the manager harness, not the core:
generate or copy the runtime `.lm1.h` set the way the graph gate does and add
the matching `-I`. The graph gate is a working reference for exactly this, and
the ticket points at it.

This is the honest shape of the finding: fixing the import root did not make
five modules pass. It made five modules reach the next real obstacle, with an
exact diagnostic instead of a path error.


---

## 7. Correction, 22:20 — section 5 was wrong about the cause

Section 5 says the gen1/gen2 divergence comes from the linked parser and that no
edit to `lm2/l1trans.lm2` can close it. **That is wrong.** Mikhail asked whether
the difference was about `c.*` syntax, I went to check instead of answering from
the hypothesis, and the hypothesis did not survive.

What the parser actually does, both of them, on the text `stack\count`:

    build/lm0/printTree.lm0.exe        -> structure fields=3: "stack" "\" "count"
    build/l1trans/gen2/printTree.exe   -> structure fields=3: "stack" "\" "count"

Identical. So the frozen parser is not the cause, and `l1_emit_expr` is byte for
byte the same 153 lines in the seed and in the current source.

The real cause is a plain source difference, which is the ordinary "the seed is
one commit behind" story after all:

    l1_head_looks_assignable_target   current l1src: present   seed lm2: ABSENT
    l1_emit_assign_index              current l1src: 4 uses    seed lm2: 2 uses

`l1_head_looks_assignable_target` is the function `24a3e4c7` added. It decides
whether a colon frame is an assignment target at all -- an indexed or
field-follow head is one, and a `c.*` head is one when it carries a path dot
beyond the `c.` namespace. That decision changed the routing for ORDINARY
indexed targets too, so more statements now reach `l1_emit_assign_head`, which
renders the target compactly, where the seed still sends them down the generic
expression path, which separates fields with spaces.

And to answer the question that started this directly: no, the three differing
lines are not `c.*` constructs. The source is
`stack\items[stack\count]: item` in `l1src/own.lm1:125`, and not one of the
six differing C lines contains `c.` at all. But the commit that changed them is
exactly the C-surface one, because what it changed is the rule that decides what
an assignment target IS.

Consequence for the plan: the choice offered in section 5 is not the choice.
Porting `24a3e4c7` into `lm2/l1trans.lm2` should close `run_gen`, and the same
staleness is what `gen0 run_decl_repeat` and `gen0 run_ident` are reporting. No
gate needs relaxing and no language feature needs narrowing. That is the next
piece of work, and it is ordinary engineering rather than a contract decision.

The lesson is the one already written in this repository's own notes, and I
repeated it anyway: a hypothesis that explains the evidence is not a cause.
`printTree` was two commands away the whole time.


---

## 8. Second correction, 22:40 — what is actually measured, and one failed attempt

Sections 5 and 7 each named a cause and each was wrong. This section replaces
both. Everything below marked *measured* was run; everything marked *inferred*
is not, and is labelled.

### Measured

- The construct is `stack\items[stack\count]: item` (`l1src/own.lm1:125`). None
  of the six differing C lines contains `c.` at all.
- Both parsers split the index text the same way, through both entry points:
  `printTree` on a file and a C probe calling `lm_p0_parse_bytes` directly both
  return three atoms, `"stack"` `"\"` `"count"`. Section 5's "parser
  granularity" is not what happens.
- The current parser records those three atoms as adjacent:
  `off=0 len=5`, `off=5 len=1`, `off=6 len=5`.
- `l1_emit_expr` is identical in the seed and the current source -- 153 lines,
  differing only in L2 versus L1 type spelling. So is `l1_nodes_adjacent`, and
  so is `l1_emit_assign_index`.
- `l1_emit_expr` writes a compact `->` for a `\` atom only when
  `l1_nodes_adjacent(prev_node, node)` holds, and otherwise opens `(*`. That
  test reads `span->offset` and `span->length`.
- `24a3e4c7`'s routing hunk widens the assignment-target path only for heads
  that start with `c.`. A head like `stack\items[...]` takes the same route
  before and after it, so section 7's attribution to
  `l1_head_looks_assignable_target` does not explain these lines either.

### Inferred, not measured

Every ingredient above is identical between the two chains except the spans a
bytes parse records, so that is where the difference has to be. The frozen
`libparser.lm0.a` cannot be linked into the probe to confirm it -- it is the
L2-profile archive and the probe segfaults on its runtime -- so the frozen
side's offsets remain inferred from the emitted output rather than read.

### The attempt that failed

On the reading above I made the fragment path ignore recorded adjacency: a
counter raised around the `l1_emit_expr` call inside `l1_emit_assign_index`, and
the adjacency test relaxed to `... || l1_in_fragment != 0`, applied to the
current source AND the seed together. The intent was that both chains would then
emit the compact form.

Result: the divergence flipped instead of closing. After the change gen1 C held
the compact form and gen2 C the spaced one -- the opposite of before -- and a
direct probe showed gen0, gen1 and gen2 all emitting `stack -> count`. That is
not what forcing the compact branch should do, which means the spaces do not
come from the branch I changed. I reverted it rather than keep editing: the tree
is back at the previous commit and every green gate above still stands.

So the mechanism is still open. What is NOT open: it is not `c.*`, not the atom
split, and not the assignment-target routing rule. The next person should start
by finding where the SPACE around `->` is written, which is evidently not the
`->` branch of `l1_emit_expr`, and only then decide anything.

One more observation, unexplained and deliberately not built on: invoking gen0
by hand on `l1src/l1trans.lm1` produces C that does not compile (`'immutable'
undeclared`, `stray '@'`), while `run_gen` inside the gate does the same
translation successfully. The gate clears a registry environment before the
seed runs; that difference has not been explored.


---

## 9. Third pass, 23:05 — the numbers, and why the earlier passes kept missing

Two things went wrong in sections 5, 7 and 8. The first is that I took
measurements across a tree that was moving underneath them: `gate.ps1` rebuilds
gen0, gen1 and gen2 on every run, so a probe taken after one run and compared
with a diff from another run is comparing different binaries. Every number below
comes from a SINGLE clean run on the committed tree, with the probes taken
immediately after it and nothing edited in between.

In that single state:

    gen1 C line 143:  stack->items[stack->count] = item;
    gen2 C line 143:  stack->items[stack -> count] = item;
    the probe, same state, all three generations:
        gen0, gen1, gen2 -> stack->items[stack -> count] = item;

So the same gen0 binary emits the compact form for that line when it is inside
`l1trans.lm1` and the spaced form for the same construct in a small file. The
construct alone does not decide it; the surrounding statement does. That is why
every attempt to explain it from the construct -- parser granularity, `c.*`
routing, fragment adjacency -- explained nothing.

The second thing is that "the seed is one commit behind" was never true. Measured
with `l2src/tools_seed_drift.py`, comparing `stg/l1_baseline/lm2/l1trans.lm2`
against `l1src/l1trans.lm1` function by function:

    functions present only in the current source : 59
    functions present only in the seed           : 0
    common functions whose call set differs      : 26

The seed is missing whole subsystems, not a commit: the entire header-unit
emitter (`l1_emit_header_unit`, fourteen `l1_emit_hdr_*` / `l1_hdr_*`),
`l1_emit_data_decl`, `l1_emit_immutable`, `l1_pointer_depth`,
`l1_write_type_spelling`, the `l1_fn_register_*` set, and more. Among the
call-set differences is `l1_emit_stmt` itself -- the statement dispatcher --
which in the current source reaches `l1_head_looks_assignable_target`,
`l1_emit_data_decl`, `l1_emit_immutable`, `l1_fn_find`, `l1_head_is_unknown_type`
and `l1_pointer_depth`, and in the seed reaches `l1_emit_type_token` and
`l1_write_type_spelling` instead. A statement being routed to a different
emitter in the two chains is consistent with everything observed; that it is the
route taken by THIS statement is likely but not measured.

### What this does to the question

`run_gen` asks that a translator built from the seed and a translator built from
the current source produce byte-identical C for `l1trans.lm1`. With 59 functions
of drift between them, that requirement pins the bootstrap seed to the live
source permanently -- every future change to statement emission has to be
mirrored into an L2 source that no generator produces (`port_l1trans.py` runs
lm2 to lm1, and its output does not resemble the current lm1 at all: the seed
lacks even the `os:` blocks). That the two still differ in only 18 lines is the
surprising part.

So the choice is back, with numbers behind it now:

1. Mirror the relevant statement-emission behaviour into the seed by hand, and
   accept doing that again for every future change of that kind.
2. Re-found the check: gen1 is a bootstrap step, the fixed point that certifies
   the translator is `gen2 C == gen3 C`, which holds byte for byte today, and
   the reason goes into the gate in writing.

I am not choosing. What I will say is that the earlier framing -- "port one
commit into the seed and it closes" -- is measurably wrong, and nothing about
this blocks anything except the pin promotion.


---

## 10. 23:10 — five manager modules now compile through the integrated compiler

With Claude's runtime-header commit `5d8852d0` merged in (he had done that work;
his earlier outbox file was overwritten by a later report under the same name,
which is why section 6's ticket went back out -- the fault was in the reporting
channel, not the work), the six runners were re-run here on the integration
branch. Measured:

| runner | translate | C compile | link |
| --- | --- | --- | --- |
| app_panel | exit 0 | **0 errors** | 1039 undefined refs |
| buttons | exit 0 | **0 errors** | 1070 undefined refs |
| cmdline | exit 0 | **0 errors** | 634 undefined refs |
| event_fifo | exit 0 | **0 errors** | 347 undefined refs |
| fm_remove | exit 0 | **0 errors** | 59 undefined refs |
| cmdline_dispatch | barrier at 48:84 | -- | -- |

This is the first time a real manager module has gone through the integrated L2
compiler end to end into an object file. `app_panel`'s oracle side also reports
`mixa app panel parity harness ok` in the same run, so both halves exist; they
just have not been linked together yet.

Every missing symbol is the graph and Message runtime -- `lmx_branch_struct_known`,
`lmx_msg_create`, `lmx_int_new_owned`, `lmx_array_ref_new_positive_owned` and the
rest. The core gate builds exactly that set and links it: `run_graph_abi.ps1`
compiles the nineteen modules named in its own `$names` list into objects under
its run directory's `message_support` and passes them on the link line. The
manager harness compiles the generated object and links it without them.

So the remaining step on this thread is one more borrowing from the same
reference, and Claude has it. Nothing here is a core defect.


---

## 11. 23:35 — the L1 gate is green, and four manager modules pass real parity

Two results, both measured on the merged branch.

### The full L1 gate reads "all green"

`stg/l1_baseline/gate.ps1` exits 0: buildCore, run_seed, run_gen through the
gen3 fixed point and its integer_add run, and every suite on both generations.

Two changes got it there, both decided rather than assumed. `run_gen` no longer
requires gen1 C to equal gen2 C -- gen1 comes from the frozen bootstrap seed and
the fixed point that certifies the translator is gen2 C == gen3 C, which still
fails the gate when it breaks (proved by inverting it: the gate went red with
the tripwire message). Three gen0 suite steps report SEED instead of FAIL for
the same reason, each named with its diagnostic, each still run, each still
required to pass on gen2, and the gate tells you to remove an entry if it ever
starts passing on the seed. The full reasoning lives in `run_gen.ps1` and
`gate.ps1` at the point of each change, not only in this document.

The seed drift is reported, not hidden: the gate now prints
`gen1 and gen2 C differ in 18 lines` on every run.

### Four manager modules pass oracle-versus-L2 parity

With Claude's runtime link (`9a877c1a`) merged, the five runners that had
reached the link stage were re-run here:

| runner | verdict |
| --- | --- |
| fm_remove | **PASS** |
| event_fifo | **PASS** |
| cmdline | **PASS** |
| buttons | **PASS** |
| app_panel | PARITY_FAILURE |

Zero undefined references anywhere. This is the first time real manager modules
have been translated by the integrated L2 compiler, linked against the graph and
Message runtime, and compared behaviourally against their L1 oracle -- and four
of them agree.

`app_panel` is a genuine behavioural difference and the most useful result of
the five. Its L2 side fails from the first operation: `entry_create` returns 2
where the oracle returns 0, and no entry is created, after which every dependent
check follows. That is a real defect to find, not a barrier and not a harness
gap, and it is the next thing to work on.

### What this unblocks, and what it costs

Decision 2 of 2026-09-13 promotes the compiler pin once the full gate is green
on the merged branch. It is green. The promotion is therefore authorized -- but
it is not free: Claude's parity runners verify the stable translator by hash,
`65D5A5ED...`, in every one of them. Promoting a new pin breaks all of them at
once until he updates that constant. So the promotion needs to be sequenced with
him rather than done quietly, and it is not done here.


---

## 12. Correction, 23:55 — app_panel is a harness gap, not a core defect

Section 11 called app_panel's PARITY_FAILURE "a genuine behavioural difference,
not a barrier and not a harness gap". Wrong, and I had said the same to Claude
and told him to leave it to me. Measured in the run directory:

    <run>/fixtures/       ab.lnk  cd.lnk  ghost.lnk  marker_a.txt  marker_b.txt
    <run>/apppaneldir/    a.link  b.link
    <run>/l2run/          apppaneldir/ (empty), apppaneldir2/ (empty), emptydir/
    <run>/l2run/fixtures  does not exist

The runner prepares its real fixtures under `$FixtureDir = $RunDir/fixtures` and
runs the ORACLE with `$RunDir` as its fixture root. It then runs the L2 binary
with `$L2FixtureRoot = $RunDir/l2run`, a directory created fresh for that run,
where none of those fixtures exist. So the L2 side starts with empty
directories, `mixa_app_entry_create` returns `MIXA_APP_ERR_MISSING` -- which is
2, exactly the `got=2 want=0` in the diff -- and every dependent check follows
from that one absence. Nothing in the generated module was exercised at all.

The separate roots are the right design; only their contents are wrong. It is
Claude's file and he has it, with a note to check his other runners for the same
shape -- including the four that pass, so we know those passes are real and not
accidental.

What section 11 says about the gate and about the four passing modules stands.
What it says about app_panel does not, and the pattern is the same one that
produced the wrong causes in sections 5 and 7: a result that fit a story, taken
before the cheap check that would have contradicted it. Listing the two fixture
directories took one command.


---

## 13. 00:10 — app_panel behaves identically; only the printed root differs

Claude's fixture fix (`2cf0eb28`) gives each side its own populated root, and he
was right not to share one: the oracle's real invoke leaves marker files behind
and the harness requires them absent before any invoke, so a shared directory
would have failed that check instead. He audited the other runners and found the
same shape in app_win32 before it could bite.

app_panel went from every check failing to exactly four differing lines, and all
four are this:

    oracle: CHK entry0 ref got=...un_2026...ixturesb.lnk       want=(same) PASS
    L2    : CHK entry0 ref got=...un_2026...\l2runixturesb.lnk want=(same) PASS

Both sides say PASS. Each resolves its own launch reference correctly against
its own root. The generated module behaves identically to the oracle; what
differs is the absolute fixture root each trace prints, and the runner compares
traces as text, so two independent roots can never match.

The fix is to normalise each run's own root to a fixed token before comparing,
and it belongs somewhere every runner can reach, because the same shape will hit
any path-printing check wherever the two sides get separate roots. Claude has
it.

Stated in advance so it can be falsified: I expect app_panel to become the fifth
PASS with nothing in the generated module ever at fault.

So the count today: four manager modules pass real parity outright, and the
fifth has no known behavioural difference left -- only a comparison that cannot
see past two directory names.


---

## 14. 00:30 — app_panel is the fifth PASS; unknown foreign type mostly cleared

**app_panel: PASS, empty diff**, after Claude normalised each run's own fixture
root out of the traces (`11bc9c18`, applied to the 22 runners with two roots).
The prediction written in section 13 before the run held: nothing in the
generated module was at fault. Five manager modules now pass real
oracle-versus-L2 parity through the integrated compiler: fm_remove, event_fifo,
cmdline, buttons, app_panel.

**unknown foreign type** was a reachability gap -- a C typedef in a header
included by a predef'd `.h.lm1` was never scanned. Fixed with a gated fixture.
Of the eight modules it stopped, six moved and four left the diagnostic
entirely. The remaining two are a C `#include` one level further inside a C
header (process_win32, and pump's second formal) and a `<windows.h>` type
(dir_win32). The first is the next piece of compiler work; the second is out of
scope by design.

The host rebooted at 00:09 UTC. Afterwards, measured by live pid: my inbox and
peer watchers were restored and verified, Claude restored his own; Mikhail's
30-minute nudge monitor (`work_chat/monitor/mikhail_30m_nudge.ps1`) and
OpenCode's serve process are not running. Neither belongs to this chat, so they
are reported and not restarted.


---

## 15. 01:05 — seven manager modules pass real parity

Passing oracle-versus-L2 on the integration branch: **fm_remove, event_fifo,
cmdline, buttons, app_panel, fm_copy, pump**. fm_copy and pump passed once
Claude's shared runner helper (`790f0404`, `lib_l2_runtime_support.ps1`)
supplied the three harness pieces; pump also needed the compiler to find a C
function behind a `#include` inside a C header (`89f74224`).

Compiler work landed tonight, each with a fixture that failed first and every
core gate green:

- a C typedef reached through a predef'd header (`02997127`);
- a C typedef reached through a C `#include` inside that header (`7f99d33c`);
- the same walk for C function declarations (`89f74224`).

Of the eight modules once stopped at "unknown foreign type", seven have left it.
What remains, and who owns it:

| gap | modules | nature |
| --- | --- | --- |
| const-pointer return of a foreign struct | app_controller | the return branch admits only `char`; design question |
| `<windows.h>` type | dir_win32 | out of scope by design |
| own array with a named count (`define:` from a header) | remove_confirm, selection_walk, copy, file_manager | `l2_array_count_is` accepts only literals |
| `ulong` | process_win32, file_win32, process_marker | no `ulong` type in L2 at all -- a new value domain, like `unsigned` was |

The channel with the mixa_manager chat moved from `work_chat` files to direct
session messages at Mikhail's request; both sides' file watchers are stopped.

## 16. 02:30 — named own-array counts resolve; two gaps reclassified by the spec

**Own array with a `define:`-named count: closed in the compiler.** A count
that is not a literal now resolves through the unit's own `define:` table, then
through its predef'd `.h.lm1` chain, to the literal that reaches the constructor
and the bounds check. Design and patch procedure are in `FABLE_GRAPH_ABI.txt`.
Fixtures `entry_own_array_define` (runs, exit 0) and `own_array_define_oob`
(refused with the bounds diagnostic) failed first on the old translator. The
graph gate still passes 138/138 and the full `run_l2trans` passes (exit 0) with
both fixtures registered; its historical input pin moved 124 → 125, recorded in
`L2_MESSAGE_ROOT.txt`.

All four modules have left that barrier; each now stops somewhere new, not yet
diagnosed:

| module | was | now |
| --- | --- | --- |
| remove_confirm | 5:5 own array | 91:13 `unknown field path root` |
| selection_walk | 62:5 own array | 81:1 `incompatible entry signature` |
| copy | own array | 192:23 `incompatible entry signature` |
| file_manager | own array | 16:9 `unknown method` |

The seven PASS modules were re-run on this build and still pass.

file_win32 (122:5) and process_marker (278:5) still report
`unsupported own array declaration`, but they are not a missed case of this
change: both declare `[]: ulong … 1`, a literal count whose **element type** is
`ulong`. They sit in the `ulong` row below.

Claude's reading of the two unexpected stops (his, sent before any falsifying
run here): remove_confirm 91:13 is `g_remove_confirm_sink\on_file: …` on a
global declared `@: MixaRemoveSink` at line 72; file_manager 16:9 is
`return: f(n)` through a local typed `MixaFmAllocFn`. Both bodies match the
oracle `.lm1`. I asked him for the minimal commands that would falsify each
diagnosis before treating them as compiler gaps. selection_walk and copy now
stop at known barrier shapes (custom-struct formal, fnptr formal).

**Pin candidate.** `stg/l1_baseline/build/l1trans/gen3/l1trans.exe`, SHA256
`722AC86E256D28EB462EE244D92B5E7188792EC0A0F5B300957622672EBAB466` (297149
bytes), built from gen3 C `25DC4758…`, which is byte-identical to gen2 C (the
§11 fixed point). §6.2 names gen2's binary. That file was overwritten with the
65D5 pin afterwards so Claude's runners would keep working; gen3's binary comes
from the same C, built in the same gate run. No L1 source changed after that
gate (last `l1src` commit 18:41; the gate ran at 20:15; the 20:20 commit changed
only its checks). Not installed yet: Claude first moves the hash his runners
assert into one place, then the swap and the re-run follow (§17 when done).

`origin/main` (the review document, the mixa_audio_win32 port) merged into
integration as `4420684f`; integration is 0 commits behind main.

**Two rows of the §15 table were wrong about their nature.** The review lane
(`LEAD_REVIEW_20260914.md`) pointed at the spec; both citations checked:

| gap | modules | corrected nature |
| --- | --- | --- |
| const-pointer return of a foreign struct | app_controller | a compiler gap, not a design question: `Lingvamyxa_spec.txt` line 8002 (`const: @: T p` → `const T *p`) and 14614–14616 (20.2.2: C declarator qualifiers remain explicit receiver composition) |
| `ulong` | process_win32, file_win32, process_marker | a compiler gap, not a new domain to design: line 4822 maps `ulong -> C unsigned long` |
| `<windows.h>` type | dir_win32 | unchanged: out of scope by design |

**What "every module ported" does and does not mean.** Measured on this branch:
`mixa_manager` holds 53 `.lm2` units, 51 with a same-stem `.lm1` and 2 probes.
13 of the 51 are identical to their `.lm1` apart from `predef:` lines; the
largest difference is 77 lines (`mixa_draw`). None uses `merge:`, `lmx_msg`,
`own:` or an `L2:` wrapper. So the `.lm2` units are the L1 bodies moved onto the
L2 translator — the planned first test of library emission — and the port of the
app to the L2 model (Structures, Messages, graph ownership) has not started.

## 17. 03:30 — fnptr locals, type 31, and a runtime module the 65D5 pin miscompiles

**Authority.** Mikhail stated it first-hand in this conversation: Fable e2 is
#1, this chat (d6, Opus) #2, 5e (Sonnet) #3. e2 coordinates and ports the runtime
modules to L2. This chat keeps the compiler, runtime core, integration, pin
promotion and core documents. Merge mechanics go directly between d6 and 5e.

**Pin in one place.** The 21 core runners read `stg/l1_baseline/l2src/L1_PIN.txt`
(`7aafb954`). The 52 mixa runners read the same file through
`lib_l2_runtime_support.ps1` (5e, `2bd0159b`). Both sides checked that a wrong
hash throws before anything is built. The file still holds 65D5.

**Compiler, this commit.**
- A function-pointer local is called only in compact form (`f(n)`); `f: value`
  is an assignment. Before this, the split assignment `f: av\alloc` translated
  with exit 0 into `f(av\alloc)`, a call through an unset pointer, and
  `return: f(n)` was refused. file_manager has left 16:9 and now stops at its
  runner's import root.
- The public library signature spells `@@: LmxMsgCopy` (type 31), which blocked
  e2's lmx_msg_mail_chain port.
- Gates: run_l2trans exit 0, graph 138/138, seven PASS modules still PASS.

**run_port_msg_blocks: red on the 65D5 pin, green on the candidate.** The
module's generated L1 is byte-identical between the last green run (18:55) and
tonight's red one; only the C differs. The runner's default L1 is stg gen2,
which has held the 65D5 copy since 20:49. Re-run with the candidate
(`722AC86E…`) as `-TranslatorPath`: PASS, 143 checks, 0 failures, and its
generated C is byte-identical to the 18:55 green run. The pin miscompiles a
module-internal call (`m0(lmx_branch_struct_known(node->node, 0U)->l2_p1_0,
&tail)`); the current-source L1 does not. Until the swap, runtime-L2 runners
should pass the candidate as `-TranslatorPath`.

**Next compiler gaps, in order:**
1. `c.sizeof(@: void)` exits 1 with no diagnostic (e2: lmx_graph_copy_owned) —
   the silence first.
2. By-value local of a header struct type (`LmxCopyMap: ms`) is refused (e2;
   spec 20.2.2).
3. Field write through a foreign-pointer unit field (`g\on_file: x`,
   remove_confirm 91:13). The global is a program-Structure child, so the
   write needs the slot load `l2_emit_path_load` already emits.

Then the pin swap with 5e, the re-run, and the ff-merge to main.

## 18. 04:10 — the L1 pin is 722AC86E

**Promoted by §6.2.** The candidate `722AC86E256D28EB462EE244D92B5E7188792EC0A0F5B300957622672EBAB466`
(gen3 binary from gen3 C `25DC4758…`, byte-identical to gen2 C) now sits in
`stg/l1_baseline/build/l1trans/gen2/l1trans.exe` and in repo-root
`build/l1trans/gen3/l1trans.exe`. The swap script checked the hashes before and
after copying. The root gen3 was `EECAB10A…`, which is the gate's gen1 binary, not
a gen3. `L1_PIN.txt` holds the new hash. The 65D5 binary is kept at
`build/pin_65D5/l1trans.exe` for rollback. The documents that named 65D5 as the
current pin were updated: instruction §4.4 (ru, en), model §41 (which also records
Mikhail's confirmation of the gen2 C == gen3 C criterion), and the Fable handoff.
Dated evidence that names 65D5 stays as written.

**On the new pin, one worktree, one state:**

| gate | result |
| --- | --- |
| run_array_owned | exit 0, 929 checks, 0 failures |
| run_msg_blocks | exit 0, 143 checks, 0 failures |
| run_graph_abi | PASS, fixtures 138/138 |
| run_port_msg_blocks (default L1, no override) | PASS, 143 checks, both runs agree |
| run_l2trans | exit 0, "l2trans gen2 ok" |
| fm_copy, pump, fm_remove, event_fifo, cmdline, buttons, app_panel, help | PASS (eight modules) |
| file_manager | its runner's import root, as before |
| remove_confirm | 91:13 `unknown field path root`, as before |

help's PASS is new since §15: it passed on the merged runners before the swap,
and again after it.

**c.sizeof(@: void) silence: cause found, fix queued.** Traced under gdb on a
`-g` build. The frame form reaches `l2_emit_ccall`, not the `c.sizeof` branch,
and `l2_emit_ccall` gives up on the `@: void` argument without a diagnostic. Two
causes combine:
- **Generation order differs from checking.** Checking takes `c.sizeof` before
  the C-call door; generation tests the door first.
- **The door opens because `l2_quoted_has_function` matches text.** It strips
  `c.` and finds `sizeof(` in `l2src/lmx.h`, which the unit's predef header
  includes, so a C keyword counts as a declared function.

Without that header the same line fails loudly, differently:
`unresolved dynamic=void` from `l2_dyn_typed`.

The atom form is not a workaround: it translates to `c.sizeof(void)`, which is
`sizeof(void)` in C, 1 under GCC and not a pointer size.

Fix plan:
1. A C keyword is never a header function.
2. Generation handles `c.sizeof` before the C-call door.
3. The type inside `c.sizeof` is not a dynamic name.
4. A translation that fails with no diagnostic prints one.

**Also queued (e2):** a generated public wrapper returns 0 when
`l2_library_open` fails, so a status ABI such as `lmx_graph_copy_owned`
(0 = OK) reports an open failure as success.

## 19. 05:00 — c.sizeof(@: void) translates; no silent failures

The §18 plan is in:
- a C keyword is never a header function;
- emission takes `c.sizeof` before the C-call door;
- the type inside `c.sizeof` is not a free name;
- `main` reports any failure that printed no located diagnostic.

The net went in first, alone, as its own tripwire: the old silent case printed
the new line, a located failure got no extra line, and a success printed
nothing.

Fixture `library_sizeof_ptr_forms`, which predefs the header that caused the
false match, failed first (with the net's line). Now its C holds exactly two
`sizeof(void *)` and no `sizeof(void)`.

On pin 722AC86E: run_l2trans exit 0, graph 138/138, run_port_msg_blocks PASS,
the eight PASS modules still PASS.

**Next:**
- By-value header struct local (`LmxCopyMap: ms` plus `@ ms`), l2_ml type 41,
  mirroring the fnptr local at all its points. The patch dry-runs clean; its
  fixture is written.
- Then library-open failure reporting, then the unit-field field write
  (remove_confirm).
- Batch 1 of 5e's parameterized runner failed its success-branch check on
  integration (import root); his fix 838de1c4 is re-checked next.

## 20. 07:00 — struct local, open failure, and Stage A of "foreign C as written"

Landed since §19, each with a fixture that failed first:
- `a94461b2`: a by-value local of a header struct (`LmxCopyMap: map_storage`)
  and `@ name`. e2's copier then passed parity (75 checks).
- `9570123b`: a failed library open prints
  `lmx: library open failed: <unit> <operation>` on every call and follows the
  abort policy, never a silent zero. Recorded in model §14.

5e's runner batch 1 checked on integration with 838de1c4: event_fifo, cmdline
and pump PASS through the new engine. After the pin swap he re-verified all 19
modules in full, with 0 mismatches (4a13e40c). text_rect's harness bug (`p\fg[0]`
on a scalar) was his, and it was the same under both pins.

**Mikhail's rulings on foreign names** (through e2 and in this chat):
- Remove the declared-ness checks for c.*.
- Require c. for every foreign C call.
- Spell foreign types as written.
- A bare C call is a namespace bug, in L1 too.
- Delete any check the spec doesn't require, and write the deletion down.
- Authority order: e2 #1, d6 #2, 5e #3.

**Stage A (calls), in the tree:**
- c.name is emitted as written and gcc checks it.
- A bare name resolves only against unit methods and parsed `prototype:`
  declarations.
- Deleted: the C-header text scan, the closed known-name lists, and a duplicate
  declaration loop.
- Kept: fail-open include flags; the profile-installed adapter names (lm_p0_*,
  lm_own_*, l2_immut_query_fill, l2_hash_compare_q), which are LMX functions
  with their own lowering; and the c.-headed special forms (sizeof, puts,
  array, the byte and Message-move operations), outside the generic door.
- Falsifier (fixture unit_unknown_c): `c.no_such_function` passes l2trans and
  fails at gcc with "implicit declaration".
- Translate-only sweep of 333 .lm2 changed exactly the intended files.
- 5e declared the missing LMX prototypes in eight mixa headers (3ec09f90). Where
  a callee is implemented, not declared, decides c. or prototype.
- Measured on the Stage A tree with 3ec09f90 merged:
  - run_l2trans exit 0;
  - run_graph_abi 138/138 (the stale negative msg_bad_call rewritten to a bare,
    undeclared call);
  - run_port_msg_blocks PASS;
  - fm_copy, pump, fm_remove, event_fifo, cmdline, buttons, app_panel, help
    PASS.

**Next:**
- Stage B: foreign types as written.
- A ulong owned domain: e2 ruled that by-value numeric locals are graph cells
  (spec 21.5.1). It covers the cell, the array, and a unit field; e2 mirrors the
  copier case. Other numeric by-value locals are refused loudly until the corpus
  needs them.
- Stage C: bare C calls in .lm1 sources, then the l1trans rule and a pin
  promotion.

## 21. 08:30 — Stage A on main; the ulong owned domain

**Stage A** landed as `6d72b0d0` (main `ccaa8702`). A translate-only sweep of all
53 mixa .lm2 against the previous translator, with 5e's prototypes, changed
nothing.

**ulong storage, ruled by e2 from the spec** (reversible, reported to Mikhail):
a by-value local of a language numeric type is a Message-owned cell, like int
(spec 21.5.1, 6.6). Types whose domain is not implemented are refused by name,
never emitted as activation C storage.

- Runtime half `6e846894`: LMX_TYPE_ULONG / LMX_TYPE_ARRAY_OF_ULONG, the
  value_owned constructor and accessors, the Array stride case and the L1 copier
  cases. The copy selftest covers a ulong cell and a ulong Array: 78/0, with 75
  allocation-failure positions (was 69). e2 mirrored the cases in the L2 copier
  port; it went red without them and passed 78/0 with them.
- Translator half `38cc44dc` (main `43cbae5c`): own code 36 for a ulong local and 37 for a
  ulong Array. Code 9 was rejected because it is `@: size_t` among formal codes.
  The 12 unsigned emission branches got generated ulong twins, and the two Array
  sites, recognition and the numeric own-code lists were extended. Float, double,
  long, the fixed-width types and wchar_t by-value locals get "by-value <type>
  local not yet implemented". Fixtures unit_ulong_local (the 126th historical
  input; pin 9D7EE770…) and unit_float_local_refused both failed first.
- The candidate sweep moves audio_panel to translating, and file_win32 /
  process_marker / process_win32 to their next stops (`[]: long`, 30:5, and a
  foreign type).
- The first full run missed one site. The predef condition for
  lmx_array_owned did not list 37, so gcc failed on unit_ulong_local. After the
  fix: run_l2trans gen2 ok, graph ABI 139/139 (merge selftest 261/0),
  port_msg_blocks PASS, array_owned 929/0, the 8 mixa modules PASS.
- ulong in the signature, in gates next: formal and return code 36, the ulong
  own code, so a written ulong formal binds to its own field (SPEC 11.3.1).
  Fixtures unit_ulong_signature (built and run) and library_ulong (public
  signature). The 127th historical input, pin 333F3CA3. Gates: run_l2trans ok,
  graph ABI 140/140, port_msg_blocks PASS, array_owned 929/0, 8 mixa parity
  modules PASS. Mixa sweep, 54
  files, 2 changed: audio_win32 moves to 74:5 (`[]: wchar_t`), fileio_win32
  to 39:25 (Stage B). app_win32, audio and share_win32 stop earlier than
  their ulong signatures. Landed `612c338a` (main `9d38042b`).
- Public signature, in gates: l2_emit_public_sig now spells the seven formal
  codes that l2_emit_sig spells but it refused (4/6/7/10/13/14/15, the
  LmP0Text, L2ImmutQuery, LmP0TrailerRole, LmP0Document and LmP0IndentStack
  formals). 5e reported code 4 from the parser_text port; it blocked
  p0_text_equals and p0_identifier_payload. Fixture library_p0_text is 5e's
  repro: HEAD refuses it, the patched translator emits it. Landed `f29800c4`.
- e2's three runtime-port stops, in gates: `(cast: (@: int) ...)`, a
  `@: ulong` local (pointer codes 38/39), and `c.sizeof(<variable>)`. The last
  was copied as written from the parser's single surface atom, so formals and
  own fields reached C under their source names. It now lowers to l2_pM_K or
  to a zero temporary of the own type. Fixtures 128-130, pin 91C4D5A3. Landed
  `c926fb11` (main `1a8a82f1`); gates: run_l2trans ok, graph ABI 143/143,
  port_msg_blocks PASS, array_owned 929/0, 8 parity modules PASS.
- In gates: a c.sizeof operand of several fields (`s\v[0]`, from e2's
  lmx_msg_visit) is checked and emitted as an L2 expression. Fixture 131,
  unit_sizeof_expr, pin D6864BCF.
  Landed `9299501e`; gates as above, graph ABI 144/144.
- lmx_msg_history_owned_selftest had been red since 6dce6214 (found by e2).
  6dce6214 admitted LMX_KIND_PRIMITIVE roots to history, but the test still
  expected them omitted. Counts derived from the rule: prepare keeps cells[0],
  [1], [2] and [4], giving 4 private nodes and 4 OOM positions, with cells[4]
  present; commit frees the cells[1] duplicate, leaving live 3 and 4 roots. Now
  65/0. The five message-module runners are in the gate list from here: history
  65/0, roots_stale 27/0, visit 148/0, liveness 97/0, sched_ready 20/0.
  Landed `884e5111`.
- p0_meta goldens: 17 of the A-G `.meta.txt` files had been written from CRLF
  checkouts. Each was longer than its LF `.lmx` by exactly its line count, so
  run_p0_meta stopped at A_compact_f_paren (reported by e2/5e). They were
  regenerated through the runner's own P0_META_WRITE_GOLDEN=1 path: before
  red, after 36/36. The diff is 94 lines each way. With source_len and span
  values masked, both sides are identical, so no tree shape changed.
- Ruling on the l2_foreign_alloc fallback arena: lm_own_* are profile-installed
  LMX functions implemented in L1 (spec 9.0/9.2, lead decision 11). A unit
  declares them and links the implementation; it does not inline a second
  copy. That copy is what collided with l1src/own.lm1 in 5e's parser link.
  Goldens landed `58314636`. In gates: l2trans emits the p0 include and a
  four-entry prototype: block in place of the predef; run_l2trans links
  l2_foreign_alloc as a support object when the C calls lm_own_* without
  defining them; run_l2_message_root's check follows. No pin change.
  Landed `4abf4fba`. The run_l2trans, graph, port, foreign_alloc,
  message-module and parity gates all pass. run_l2_message_root is not
  verified. Its first direct `& gcc ... *>` call runs under EAP Stop, so in
  PS 5.1 gcc's unused-variable warnings become a terminating error, and it
  stops before any check. The same program.c through `cmd /c` exits 0 with
  warnings only. This is a defect in that runner, independent of 4abf4fba
  (last PASS 2026-09-12 in another checkout). A separate task makes its gcc
  calls tolerate warnings and runs it to a verdict, including the updated
  lm_own check.
- Ahead of Stage B: a foreign C call with no arguments (`c.rand()`,
  `c.abort()`; e2, five sites in lmx_message) was "unsupported body" as a
  value, a condition and a statement. The empty body reached l2_check_fields
  with 0 fields, and l2_emit_ccall required a body. Both now accept it, and it
  is emitted as written. Fixture unit_c_empty_call, built and run. Landed
  `a054b46b`, with the graph gate's abort count corrected to exclude the
  source's own `c.abort()`.
- Two miscompiles from e2's lmx_message port (C that did not compile), in
  gates:
  - a boxed C-call actual took its temporary name through the shared
    l2_tok buffer, which renamed the && condition temp;
  - index tokens spelled an own field bound to a formal as its absent working
    local l2_qN instead of the formal.
  Fixtures are e2's (038aae34), compiled with gcc -c through the new
  Invoke-CompileObject.
- Miscompiles landed `62233670`.
- Stage B step 1, in gates: the -2 admission in l2_foreign_intern is deleted,
  along with its eight "unknown foreign type" branches. A pointer type is
  spelled as written. A sweep of all 345 .lm2 files, HEAD vs patched, changed
  42, all forward: app_path, cmdline, help and selection_l2_probe now
  translate. graph msg_bad_type is deleted; unit_unknown_type is now a
  positive spelling check. Landed `06a940e5`.
- Stage B step 2a, in gates: a `c.sizeof` frame (a C call argument) takes
  any single operand as written, such as `c.wchar_t` (5e, fileio_win32), or
  lowers a formal or own field. The closed type list and the header lookup
  are deleted. Fixture unit_sizeof_arg is built and run. Landed `2620d3f5`.
- Stage B step 2b, in gates: by-value foreign formals and returns (`T: name`,
  `c.T: name`, `T` / `c.T` returns) are interned with depth 0 and spelled as
  written. A body temporary or local of T still needs L1 to know T, either
  built in (the LmP0* aliases) or through a `type:`/`struct:` in a predef'd
  .h.lm1. LmxMsgAddr is in neither; e2 is told. Fixture unit_byvalue_foreign,
  built and run. Sweep: 3 changed, all forward. Landed `642b6700`; the graph
  negative msg_bad_ret was deleted.
- Stage B step 3, in gates: LmP0Document is a foreign pointer type, so its
  fields resolve as a formal and as a local (5e, parser Stage c). Fixture
  unit_p0_document_field, built and run.
- New stops from 5e's own module runs after 06a940e5/2620d3f5, reported
  2026-09-14 (5e's STATUS fc3cd54b): "unsupported index" on `buf[off[0]]`
  through an out-parameter (app_win32 211:5, share 110:17); "unsupported
  cast type" (process_marker 212:71); "a callable field needs a method name"
  on a prototype (calculator_syntax 89:1). fileio_win32 and audio_button now
  PASS.
- Step 3 landed `f7390ded` (main `fb7ab80c`).
- 5e's three stops, in gates: a two-word cast type (`unsigned long`), an index
  that is itself an indexed load (`buf[off[0]]`), and a unit-level
  prototype: block (skipped). Fixtures unit_nested_index_cast and
  unit_unit_prototype, built and run. In scratch, calculator_syntax and
  process_marker translate completely.
- 5e's three stops landed `d24373ca`.
- Stage B, in gates: a const-pointer return of any foreign T keeps its
  qualifier on the return and on the call-result temporary. Fixture
  unit_const_foreign_return, built and run. Sweep: 3 changed, all forward;
  text_rect now translates.
- Still open in Stage B: the typedef text walk (l2_predef_has_type's fallback
  and the uchar own-Array typedef). A scratch sweep of its deletion changed 0
  of 351 .lm2.
- `profile: runtime` landed `73e01271` (main `eb5fbd19`).
- Merging e2's fable/runtime-l2 (212d51c3): the runtime L2 ports and
  runners, lmx_message.lm2, and the turn-flag ruling in lmx_message.lm1 and
  exec.c. The first gate run on the merge failed on a file collision.
  l2src/lmx_msg_mail_chain.lm2 became the clean-L2 port, while run_l2trans
  still used it as the historical leaf lmx_msg_mail_chain_l2. That leaf now
  reads a byte-exact frozen copy in l2src/tests (re-pin 05E9A373, 139 cases).
- Decisions 15-16 (Mikhail, relayed by e2, LEAD_REVIEW §6):
  - L1 was left to the agents in a freer form, as preparation for direct C
    translation. L2 is implemented exactly per the specification.
  - The runtime lane stops porting L1 modules. It builds the L2 runtime per
    SPEC 19.28.R2.2, 19.29.6 and 19.29.7 as Structure data, accepted by the
    spec's own 19.29.6 checks and the model's §31-§36 scenarios, never by L1
    selftests. The plan is L2_RUNTIME_PLAN_20260914.md, and it supersedes §6.4
    of d6's instruction.
  - The fifteen ports stay as translator evidence and bootstrap.
  - Working rules: commit all L2 code at every stage, and commit the whole
    project every time a full self-build passes.
- New gcc-level stops from 5e's runs (STATUS 7257bf25), awaiting 5e's minimal
  fixtures:
  - memcmp with too few arguments;
  - an off-by-one own working local, l2_q12 where only l2_q13 exists;
  - `p\fg[0]` on one-element char-array fields compiling as a scalar
    (text_rect, tiles), a suspected regression;
  - duplicated per-unit l2_fnv1a64 / l2_immut_query_fill / l2_hash_compare(_q)
    bodies when several units are linked.
- Escape polls, Mikhail's answer relayed by e2 and awaiting his confirmation
  of e2's reading. A child's own complete (success=1, then running=0) is the
  normal end of its work, so the escape fires only for a requested stop
  (running=0 with success=0). Runtime-profile units are not polled at all:
  the executor calls them during an unwind, so a polled end_turn would escape
  again. l2trans is not changed until he confirms; two variants are being
  prepared in scratch: a unit profile mark, and a success-aware poll for user
  units. Spellings agreed with e2 (Mikhail to confirm; decisions 12-13 in
  LEAD_REVIEW_20260914.md §6):
  - the mark is a top-level directive `profile: runtime`, never a file-name
    rule;
  - the user-unit poll calls `lmx_msg_poll_stop()`, declared in lmx.h beside
    lmx_msg_poll_escape. It returns nonzero, after lmx_msg_poll_abort, only
    when the turn's running flag is 0 AND its success flag is 0. The runtime
    side is e2's.
  - Checkpoint-failure paths (`if: c.lmx_msg_poll_abort() != 0 / c.abort()`)
    stay in both profiles (e2). A failed store is a real state failure, and a
    second poll_abort during an unwind finds ready = 0 and aborts the process.
  - RULED by Mikhail (relayed by e2), superseding the two-flag poll above.
    `complete` sets only success=1, and running=0 after success=1 is set only
    in end_turn. So running=0 during a turn always means a requested stop,
    user units keep the single-flag lmx_msg_poll_escape, and lmx_msg_poll_stop
    is dropped. The only translator change is `profile: runtime`, which
    suppresses l2_emit_poll's three spellings; the checkpoint aborts stay. In
    gates now, with fixture unit_profile_runtime. The runtime side
    (running_store leaves complete; end_turn sets running=0 when success=1)
    is e2's, in lmx_message.lm1 and lmx_message.lm2.
- The C files (lmx_message_exec.c, lmx_message_host.c), corrected the same
  day. e2's first reading, a provider contract with an opaque API, was struck
  by Mikhail. "A Message knows only its minimum" describes the model (a
  Message is created by merge); it is not a translator restriction. L2 sees
  all of C, and nothing additional is to be built; only L3 sees no C. The
  executor C files port to L2, calling their platform functions through c. as
  they are. The setjmp turn root, thread-local declarations and the
  Win32/pthread conditional blocks stay C behind existing functions. A
  Message is isolated three ways: creation by merge, validation by the
  receiving side, and libsodium when needed. N isolated OS processes are the
  top rung. Recorded as decision 13 in LEAD_REVIEW_20260914.md §6.
- Stage B scope, collected 2026-09-14 (foreign types as written; one change):
  - delete the -2 admission in l2_foreign_intern and the "unknown foreign type"
    family; delete the typedef text walk (l2_include_has_simple_typedef);
  - const-pointer return of any T; c.sizeof of any type operand, including
    `c.wchar_t` (5e, fileio_win32 20:64);
  - by-value `T: name` formals and `T` returns for a primitive, a type parsed
    from a .h.lm1 (5e: `type: X int` aliases such as LmP0NodeKind and
    LmP0FrameFlags; e2: `LmxMsgAddr: addr` and `c.LmxMsgAddr: addr`), or
    `c.T`;
  - field access on p0 types that have fixed formal codes: LmP0Document is
    13, so `doc\field` is "unknown foreign field" (5e, parser Stage c);
  - foreign by-value locals such as `@: LARGE_INTEGER pc` (process_marker
    30:5) and `[]: long` own Arrays (file_win32 173:5), the latter with the
    long domain;
  - fixtures: e2's `(cast: (@: int) ...)` (landed), the `@: no_such_type`
    falsifier, and the own-array uchar typedef via a parsed `type:`.
- Queued from e2/5e: (1) the l2_foreign_alloc.lm1 fallback arena collides with
  l1src/own.lm1 when a unit links both (5e strips it per stage); (2) the p0_meta
  goldens were written from CRLF checkouts. 17 A-G goldens are each longer than
  their LF .lmx by exactly the line count; A_compact_f_paren is the first to
  fail.
- 5e's two gcc-level miscompiles, reproduced from 5e's fixtures on
  sonnet/parser-l2 62863393, in gates:
  - memcmp with too few arguments. In value position, c.memcmp, c.strlen,
    c.memcpy, c.memset and the four storage-list calls had fixed-arity
    special cases that took one field per actual: `c.memcmp(expr + p\offset,
    expected, n)` became `c.memcmp(l2_p0_0, +, p\offset)`. The special cases
    and their exclusion from l2_c_door are deleted, so every C call is lowered
    by l2_emit_ccall. Deleting them exposed a second defect: l2_ccall_into is
    one global, and a C call nested in an actual (`c.malloc(c.strlen(s) +
    3U)`) cleared the outer call's destination. l2_emit_ccall now takes it at
    entry.
  - `buf[off[0]]` with an own Array `off` was spelled `l2_p0_0[l2_q0[0]]`
    (no l2_q0 exists). l2_index_token loads the element through its descriptor
    before the store, as the right-hand side already did.
  - Fixtures unit_ccall_value_expr and unit_index_own_array, compiled with gcc
    -c. Every new check fails on HEAD's output and passes on the patched one.
    The graph gate's unit_msg_storage_calls check had required the special
    case's typed temp; it now requires the call on two head slots, tested in
    place. Sweep of 369 .lm2: 15 changed. With temp numbers normalised, every
    difference is a call now inlined, plus mixa_share, whose two `buf[off[0]]`
    stores are the fix.
  - Gates:
    - run_l2trans: gen2 ok, pin unchanged.
    - graph ABI: 152/152.
    - port_msg_storage: 77/0, reference and generated agree.
    - port_msg_blocks: 143/0.
    - array_owned: 929/0.
    - message modules: history 65/0, roots_stale 27/0, visit 148/0, liveness
      97/0, sched_ready 20/0.
    - The eight parity modules pass: fm_copy, pump, fm_remove, event_fifo,
      cmdline and buttons through run_mixa_l2_parity -Module; the batch-2
      per-module scripts were deleted in 14d87253. app_panel and help also
      pass.
    - Extra, outside the gate list: app_path and selection pass.
      process_marker's runner stops in l1trans: "cannot read import
      l2src/lmx_array_owned.h.lm1". HEAD emits the same predef line, so the
      runner does not yet supply the own-Array import this module now needs.
      (5e: that script predated the module's move into the parameterized
      table; `run_mixa_l2_parity -Module process_marker` passes, and the
      stale script is deleted in mixa_manager b8ffe7fc.)
  - Landed `a274faec` (main `96f474e7`).
- Runtime struct fields as written (e2, stage 2 of the runtime plan), in
  gates. l2_raw_path accepted a field of LmxMsgRuntime (28), LmxMsg (29) or
  LmxMsgCopy (30) only if it was in l2_raw_fixed_field's per-type list, so
  every field a runtime struct gained was "unknown foreign field" until the
  list was edited (e2's `src\delivered`). The lists are deleted, and these
  codes spell any field as written, like every foreign type (>= 100); gcc
  checks it against lmx_message.h. Fixture unit_runtime_struct_field reads
  LmxMsg.sched_queued, which is in the header but was never in the list.
  HEAD refuses it at 7:9, and the patched translator emits
  `l2_p0_0\sched_queued`; it compiles with gcc -c. Sweep of 370 .lm2: that
  fixture is the only change. The graph gate's negative msg_bad_field
  (`rt\zzz` refused) went red with the deletion and is deleted too, with a
  retirement comment. Gates: run_l2trans gen2 ok; graph ABI 152/152. No
  other translated source changed. Landed `45166cd4` (main `57b5a178`).
- Merged e2's fable/runtime-l2 (93a240d4) as `c4a77e64`:
  - Runtime plan stage 1: the model's section 36 scenario as an executable
    test on the production runtime.
  - Stage 2: ordinary delivery of a graph as a Message of its own (SPEC
    19.29.7), with LmxMsgCopy.delivered, lmx_msg_send_graph and
    delivery new/dispose/receive.
  - Tests for the model's 32 and 33 and for the spec's own 19.29.6 checks.
  - The lmx_message.lm2 mirror, which needed runtime struct fields as written.
  - The merge brought no file conflicts.
  - Gates on the merge:
    - run_port_message: PASS, with the L2 mirror including `src\delivered`
      and 87 methods redirected.
    - run_model_scenario36: all five tests pass, each run twice with
      matching results on the production runtime: scenario36 49/0,
      delivery 27/0, checks_19_29_6 32/0, liveness_33 54/0,
      family_close_32 24/0.
    - Message-module runners: history 65/0, roots_stale 27/0, visit 148/0,
      liveness 97/0, sched_ready 20/0.
    - run_l2trans: gen2 ok.
    - graph ABI: 152/152.
  - run_model_scenario36.ps1 is in the gate list from here. Per decision 15
    it is the core's acceptance, run on any change to lmx_message.lm1 or
    exec.c. The model's 34 is covered by lmx_msg_family_handoff_selftest
    once run_msg_family_handoff is revived; 35 (remote) is out of scope.
- Runtime plan stage 3, division agreed with e2 on 2026-09-14. Mikhail was
  sent the same proposal.
  - e2 did 3a-1 on fable/exec-3a (93a240d4 base). Every bound Message owns
    its bind record, LmxMsg.exec_bind, and LmxMsgExec.bind becomes an index
    of pointers. This moves storage only.
  - d6 takes 3a-2 and 3b.
    - 3a-2: the by-addr lookups read the Message's own record. These are
      run_one's held updates, context_worker's self lookup, take_this,
      is_bound, last_status, unbound_close, wake_addr and bind's rebind path.
    - 3b: the global enumerations go through rt->slots first, then the
      parent's child list. These are stop, drop_binds, detach,
      start_contexts, scan_ready, wake_all and drop_stale. The by-position
      index API is reimplemented over the child list or retired; it has 33
      Exec selftest uses and 10 in lmx_message.lm1. Contexts become owned by
      the parent that mapped the child.
  - e2 takes 3c: the per-parent scheduler record as an L2 Structure in the
    parent's arena, driven by a `profile: runtime` unit. e2 also writes
    each step's acceptance tests and mirrors d6's lmx_message.lm1 changes
    into the L2 unit.
  - Each exec.c step runs the five core tests in run_model_scenario36.
  - d6's review of 3a-1 (ba3dea24, rebased onto main 35681313) found one
    use-after-free that the commit introduces. The case is a rebind that
    puts a different Message at an address already in the table. The table
    entry is now old->exec_bind, and the rebind releases old under the exec
    lock. That lock is recursive, so the release can retire old
    synchronously, and lmx_msg_slot_free then frees the record before the
    next `e->bind[i]->msg` write. The record also keeps the wrong owner:
    m->exec_bind would not be the table's record for 3a-2. Sent to e2 with a
    fix (move the record to m before releasing old) and a red-first
    acceptance test. 3a-1 merges after that fix.
  - e2 applied the fix as written but could not write the test honestly.
    rt->next_addr is monotonic and lmx_msg_self_or_find skips RELEASED
    Messages, so two different Messages never share an address. A rebind
    with `old != 0 && old != m` is therefore unreachable today. The only
    reachable `msg != m` case is an entry whose msg was cleared on an error
    path, where old == 0.
  - Decision (d6): take the fix without a test or an assertion. The
    error-path test would also pass on unfixed 3a-1, and an assertion that
    no current input can turn red is not evidence.
  - Precondition, recorded in the code comment and here: exec bind records
    rely on addresses never being reused. Any change that introduces
    address reuse must add a test that reaches this rebind branch.
  - The fix is e2's 6482c71d, exec.c only. The record is handed to m before
    old is released: old->exec_bind is cleared, a stale m->exec_bind is
    freed, and only then is old unlinked and released. Reviewed by d6.
  - Merging fable/exec-3a also brings 3c-1 (656bbd07): lmx_sched_record.lm2,
    the parent's scheduler record as Structure data in its arena, as a
    `profile: runtime` unit. It has a selftest and run_sched_record.ps1, with
    35/0 measured by e2 and a tripwire red with 9 failures. exec.c does not
    use the record until 3b's contract is in place.
  - Gate list from this merge: run_sched_record.ps1 and
    `run_lmx.ps1 -Suite Message` join run_port_message,
    run_model_scenario36 and the five message-module runners for any change
    to exec.c, lmx_message.lm1 or lmx_sched_record.
  - The merge (374ec250, local, not pushed) is RED in run_port_message.
    The executor selftest crashes against the handwritten modules with
    0xC0000005 in lmx_msg_runtime_delete, just after "mass 70 ok".
    - Cause: runtime_delete frees every slot (lmx_message.lm1 994-1005)
      before lmx_msg_exec_detach (1010). Since 3a-1, lmx_msg_slot_free frees
      m->exec_bind while that record is still in e->bind. detach then runs
      `bind_wait_destroy(e->bind[bi]->wait)` over freed records, and so does
      stop's walk when the executor was not stopped.
    - Before 3a-1 the records were inline in the table, so freeing slots
      first was harmless.
    - Green on the same merge: run_model_scenario36 5/5, run_sched_record
      35/0, run_lmx -Suite Message ok, history 65/0, roots_stale 27/0. Only
      run_port_message runs the full executor selftest. e2's measurement
      for 3a-1 had not included it.
    - Proposed fix, sent to e2: lmx_msg_slot_free removes the record from
      the executor table (by pointer, wait handed to the reaper) before
      freeing it, so the table never holds a record its Message no longer
      owns.
    - Acceptance is the existing red test. The merge stays unpushed until
      the fix lands and the whole chain is green.
    - The rest of that chain on 374ec250: visit 148/0, liveness 97/0,
      sched_ready 20/0, run_l2trans gen2 ok, graph ABI 152/152.
    - e2 reproduced the crash and fixed it index-side. The invariant is that
      an entry retains its Message and only unbind_slot_locked removes an
      entry. lmx_msg_exec_drop_binds released Messages but kept their
      entries, which broke it; drop now unbinds every entry. d6 accepted
      this over a second guard in slot_free, and pointed e2 at one more
      reaching path. After 6482c71d, the rebind branch's failed
      lmx_msg_endp_retain(m) returns NOMEM with the record handed to m, the
      entry still in the table, and m not retained.
    - e2's interim result, not committed: drop_binds fixed, then d6's rebind
      reorder added (retain m first, hand the record over after).
      run_port_message PASSed once. On a second run its reference died with
      the same access violation after "m0_acc=", before the handoff nest
      print, while d6's gate chain was loading the machine. The same binary
      passed 4/4 standalone, and unfixed 656bbd07 dies at that spot too. So
      a timing-dependent use-after-free survives the drop_binds fix. e2 is
      looping a -g -O0 build under gdb for the stack.
    - Held: merge 374ec250 stays local and unpushed until e2's commit and
      d6's full chain on it are green. Until then, d6's translator steps
      (l2trans.lm1, run_l2trans.ps1, fixtures and docs, no exec.c or
      lmx_message) are pushed by cherry-pick onto
      origin/integration/main-absorbs-core in a detached worktree. The
      shared checkout picks them up when the exec-3a merge lands.
    - e2 pushed a6ab4f72 on fable/exec-3a, touching exec.c and
      RUNTIME_L2_PORTS.txt only:
      - drop_binds unbinds every entry, last first, through
        unbind_slot_locked, so an entry leaves the index before its retain
        is dropped;
      - the rebind branch retains m before handing the record over, so a
        failed retain leaves the entry unchanged;
      - only unbind_slot_locked clears an entry's msg, with both spellings
        grepped.
    - e2's measurements on a6ab4f72: run_lmx Message ok, the five core
      tests and sched 35/0 are green. With the drop_binds fix alone,
      run_port_message PASSed. With both fixes, one reference run died
      after "m0_acc=" while d6's chain was compiling. That binary and -g
      rebuilds then ran 125 times clean:
      - 8 plain;
      - 25 under gdb;
      - 40 beside concurrent selftests;
      - 30 under CPU hogs with a quarantine allocator that poisons freed
        blocks;
      - 20 with a slow stderr reader.
      So none of those runs read a freed block. The handoff scenario is
      where the heap first detects an earlier corruption, and the planting
      site is still unknown. d6 merges a6ab4f72 after the current
      run_l2trans and runs the full chain, run_port_message first. A crash
      keeps its evidence directory, and the stderr tail before "handoff nest
      users" goes to e2.
    - Merged as fbb2a729 on 374ec250. Full chain, with no other load
      during run_port_message:
      - run_port_message: PASS, reference and generated agree on both
        runs, no crash;
      - run_model_scenario36: 49/0, 27/0, 32/0, 54/0, 24/0;
      - run_sched_record: 35/0;
      - run_lmx -Suite Message: ok;
      - message modules: history 65/0, roots_stale 27/0, visit 148/0,
        liveness 97/0, sched_ready 20/0;
      - run_l2trans: gen2 ok;
      - graph ABI: 152/152.
      The held merge is released and origin/integration is merged into
      that line. The cherry-picked translator steps are identical, and
      0c's runner commits come in. Pushed.
    - e2's 9e1dbc8c and 15895c66 are merged locally as 032c5f07 and
      pushed with the next step:
      - run_port_message's reference and parity builds link
        tests/lmx_exec_crash_report.c, an unhandled-exception filter. It
        prints "CRASH tid= code= access= rip_rva=" and the raw stack as
        module RVAs, and stays silent on a clean run.
      - RUNTIME_L2_PORTS records the residual crash as still open: 165
        clean standalone runs under every detector e2 tried.
      - run_port_message is rerun on it before the push.
- `@` on an own Array element is refused again (0c, run_l2_message_root
  element_address), in gates.
  - Spec 11.3 and 11.3.1 give `@x` only for an own graph field's payload,
    a declared or hidden through parameter, or an L2 address slot. For an
    Array field, `@` names the descriptor data, "not the element backing".
  - Spec 12.2 says a raw element-storage pointer comes only through an
    explicit adapter.
  - 485f15cc admitted `@` in flat-field position for foreign-pointer
    actuals (`@ tail`, `@ item\next`) and let every following expression
    through. So `return: @ buf[0]` emitted `return: @ l2_t2`, the address
    of a temporary copy, and `@ buf[0] + buf[1] + buf[2]` emitted
    `@ l2_t3 + ...`. 0c bisected it.
  - l2_check_fields now refuses `@` followed by an own Array element access
    (including the `for\` and `node\` forms): "address of an Array element
    needs an explicit adapter". Parameters, own scalar fields and
    foreign-pointer actuals are unchanged. `@ buf` on a whole Array field
    does not reach this refusal, but it is not correct either. It emits
    `@ l2_a0_data`, the address of a C local, not the descriptor address
    that 11.3.1 specifies. 0c's array_invalid sweep found it together with
    `return: buf`, the backing pointer used as a value. Those two are the
    next translator step. Two other accepted cases are stale gate:
    - array_invalid.zero: 770e83e6 lowers empty Arrays on purpose.
    - array_invalid.nested: an own Array declared inside `if:` is built in
      the if body's own Structure (`lmx_array_new_owned(...INT, 3U...)`
      under that body's struct), as the model's executable-body Structures
      require. The empty method body is correct.
    - Both move to the accepted group of run_l2_message_root.
  - Landed `3b00f53a` (main `cb53e7e9`), cherry-picked from the local
    `8ead0546` on top of the held exec-3a merge.
- Own Array field as a value (0c's scalar_read, address): the first patch
  deleted l2_check_expr's C-decay admission, and a sweep of 375 .lm2
  showed the admission is load-bearing. Eight files would turn into "own
  array use not yet supported": mixa app_path 52:43, audio 712:46,
  fileio_win32 51:42, process_marker 212:20, process_win32 135:61,
  selection 162:83, and the fixtures unit_nested_index_cast 18:17 and
  unit_ulong_ptr_local 18:35.
  - Every one passes an own Array as a call actual or as a cast operand:
    - L2 method out-parameters such as `[]: size_t total 1` passed to
      `mixa_app_path_total`;
    - C buffers such as `c.snprintf(cand_utf8, 96U, ...)`;
    - `(cast: (@: void) slot)`.
    None returns it or uses it in scalar arithmetic.
  - Decision (d6): narrow the admission instead of deleting it. An own
    Array decays to its backing pointer only as a call actual (L2 method or
    C call) or as a cast operand. That is the de facto low-level adapter
    spec 12.2 allows ("when admitted"). In any other value position it is
    refused, which covers `return: buf` and `return: @ buf`.
  - An explicit adapter spelling that replaces the positional one is a
    language question for Mikhail and e2. It is not needed to fix the gate.
  - The narrowed patch, in gates:
    - A counter, l2_decay_ok, opens the admission. Opening it are L2
      method actuals, the C-call, predef-function, fnptr and raw-head call
      bodies, and the cast operand, through l2_check_actual_fields and
      l2_check_actual_primary.
    - `@` followed by a whole own Array field is refused with "address of
      an Array field is not lowered to its descriptor yet".
    - Sweep of 375 .lm2: 0 changed. The six mixa modules and two fixtures
      that the outright deletion broke all translate unchanged.
    - Removal proof: HEAD translates and publishes array_field_value,
      address_array_field and 0c's two gate shapes built from
      unit_own_array_int. The patched translator exits 1 on all four,
      publishes nothing, and gives the two diagnostics above.
    - Gates: run_l2trans gen2 ok, both negatives included; graph ABI
      152/152.
  - Landed `cd7c7e21` (main `fb7b12d8`), cherry-picked from the local
    `487b4ce5`.
- run_msg_family_handoff revived: the model's section 34, family handoff.
  - The runner had hard-coded core 57b590f4's file and module list, so
    every newer core failed at the `message` stage: "cannot read import
    l2src/lmx_msg_liveness.h.lm1".
  - The module set cannot come from lmx_message.lm1's predef chain alone.
    lmx_msg_path_storage, _slots, _mail_chain and _sched_ready have left
    the chain but are still called by exec.c or lmx_message.lm1.
  - It now builds the production runtime of the selected revision, as
    run_model_scenario36 does. The modules are every lmx_*.lm1 with a
    matching .h.lm1, found by git ls-tree (19 at HEAD). The snapshot is
    the whole l2src subtree, and the default core is HEAD.
  - Landed `1b2a9671` (main `72ffab1e`).
  - Measured at 57b590f4: PASS "checks=53 failures=0 watched_frees=3",
    the old result reproduced.
  - Measured at 19d6e076 (no exec-3a) and at the shared checkout: the
    runtime builds and links, then the selftest stops at check 135,
    "FAIL successful arena freed once". The sequence is
    lmx_msg_complete(successful), run_child_turn and dispose_child; the
    check expects one watched free and k->ranges, blocks and init all 0.
  - So the runtime drifted between 57b590f4 and 19d6e076, and exec-3a is
    not the cause. The runner is committed red on purpose, because the red
    is real.
  - Next, a bisect through the runner itself (no checkout needed) over 81
    runtime-touching first-parent commits. Candidates to probe first:
    d1417062 (the turn-flag ruling), c4a77e64 (stage-2 delivery) and
    6e846894.
  - Bisected:
    - e2's candidates 2494848a and 1dd1b44b fail on their parents too.
    - Batch 1 was red at indices 20, 40 and 60. Batch 2 passed at 0, 1
      and 3, and failed at 5, 10 and 15. Index 4 fails.
    - First red: 8ab7387d (d6, 2026-09-11, "LmxMsg.graph is the Message
      unit; collect unrooted ranged blocks"). Its predecessor c0b5d8ba
      passes 53/0/3.
  - Mechanism, from the test source:
    - family_block builds K's block as a raw calloc'd block with an
      embedded range on K's blocks/ranges, never referenced from K's
      graph.
    - Since 8ab7387d, end_turn collects ranged blocks the graph does not
      hold, so K's closing turn in run_child_turn frees it.
    - The test arms its free watch only after run_child_turn, reading a
      freed block, and dispose_child then has nothing left to free.
  - Ruling (d6): the runtime is right, because the model collects unrooted
    data at end-turn. The test premise is stale. Fix planned: arm the
    watch before run_child_turn and keep "freed exactly once, K's blocks,
    ranges and init all 0 after dispose" without asserting which free did
    it. __wrap_free's "range list detached before embedded record is freed"
    then also checks the collector's order.
  - Revised with e2 to pin where the block dies instead of accepting either
    site:
    - after run_child_turn: watched_frees = 1 and the watch is cleared
      ("unrooted K block died at K's end-turn");
    - after dispose: still 1, and K's blocks, ranges and init are 0
      (check 135);
    - the comment names the collector rule and 8ab7387d.
    Committed on integration.
  - Measured:
    - At c0b5d8ba the test stops exactly at the new check, as designed.
    - At 28cc0d71 both pass, and the test reaches check 151, "cannot skip
      unsettled grandchild", which is still red. That check expects the
      failed branch's raw blocks bc and bg to stay on C and G after their
      closing turns.
    - Likely the same rule. That would put the whole failed-branch half of
      the fixture on unrooted raw blocks, which predates 8ab7387d's
      collector and bdd6a62a/0c714cd8's history-root adoption.
  - Proposed to e2 as the section 34 fixture rewrite, in e2's
    acceptance-test lane: build the payloads as rooted or history data and
    assert adoption per the current model. d6 keeps the runner.
- run_l2_message_root final gate on cd7c7e21 (0c, clean worktree, pin
  match): FAIL at unit_own_dirty_rhs.
  - Evidence:
    wt0c_fe000dd6/build/codex/l2_message_root/20260914_060052_156_76b6f819.
  - Green before it (534 stages):
    - all 336 refusals exit 1, including every array_invalid case;
    - array_zero and array_nested pass;
    - driver link, allocation-fault modes 0-48 and splice pass;
    - check 3: unit_text_heap and unit_foreign_resize translate, compile
      and run, and the lm_own_* prototype check passes;
    - the following primitive cases all pass.
  - The red: `65\n240`, expected `65\n88`, on 6 of 6 reruns.
    - In observe, `quote` is the hidden through parameter. The writes
      `quote: 88` (if body) and `quote: 11` (else body) became OwnUsed
      fields hosted in each body. That split was introduced in 043e1d41.
    - As a result M's child 1 is the if-body Structure, and the drive reads
      a byte of it.
    - 043e1d41 also moved run_l2trans's $d6 drive to read the if-host slot,
      which is why gen2 stayed green.
  - Ruling (d6): translator defect. A same-name parameter write inside a
    body is not a declaration; its OwnUsed field belongs to M. Only fields
    declared in a body are hosted there, and unit_bind_ifdecl is the
    control for that. The $d6 drive goes back to M child 1 in the same
    step.
  - Revisited, then decided from the spec. A dry sweep of the method-host
    fix showed 043e1d41 had hosted a same-name argument bind in each body
    on purpose: run_l2trans.ps1:1151 asserts "executed argument bind ...
    publish into its while-body host" on unit_body_hosts (`flag: 2` in an
    if body, `flag: 0` in a while body). The two readings:
    - (A) the bind publishes into its exact body host, so 0c's gate is
      stale;
    - (B) it publishes to M, so line 1151 is wrong.
    The spec settles it as (B):
    - 7794-7797: "The through-boundary is method nesting / activation, not
      Structure or `if` nesting ... Do not invent a shadow field for an
      assignment in an ordinary nested block that already sees the
      enclosing binding."
    - 7804-7808: "Once this method's own-field bind exists ... Subsequent
      bare reads and writes share that variable. Dirty-only publication
      writes this method's own node field".
    Body hosts keep only fields declared in the body. In the same step,
    unit_own_dirty_rhs's gate stays, run_l2trans's unit_body_hosts checks
    change to assert the method field, and the $d6 drive goes back to M
    child 1.
  - The method-host step, in gates:
    - The mechanism: l2_collect_asgn_body registered the OwnUsed field of a
      same-name parameter write under l2_scope_host(). Now it first
      resolves a field declared in an enclosing body (l2_own_find), and
      otherwise registers on the method: while l2_own_at_method is set,
      l2_own_add looks up and stamps host 0.
    - Declarations are collected before assignments. l2_translate_unit
      collects methods and body declarations, then parses. So a field
      declared in a body keeps its host; unit_bind_ifdecl is the control.
    - Runner changes:
      - the $d6 drive reads M child 1 again;
      - unit_body_hosts asserts `leaf, 6U` (was 5U), the flag bind
        published at `node, 1U`, and exactly three body-host publishes,
        which are the declared yes, no and loop;
      - line 1151's shadow-field assertion is gone.
    - Sweep of 377 .lm2: 15 files change L1, and no verdict changes. The
      15 are mixa_selection, lmx_message, lmx_msg_slots,
      parser_physical_line, parser_position, and the fixtures
      unit_arg_own_bind, unit_asgn_branch, unit_asgn_samename,
      unit_body_hosts, unit_break, unit_continue, unit_dash_emit,
      unit_msg_cursor, unit_own_dirty_rhs and unit_paren_and.
    - Removal proof on the sweep outputs: the HEAD output fails all four
      checks (open 6U, bind to M, only declared fields in body hosts,
      dirty_rhs publishing only to M), and the patched output passes all
      four.
    - First gate chain: graph 152/152, run_port_message PASS,
      scenario36 5/5, port_msg_slots 278/0 and selection parity
      byte-identical. run_l2trans stopped at "unit_asgn_branch drive exe
      failed".
      - Cause: 043e1d41 had also moved that drive's only_if read into the
        if-body host (line 2063), and the patched layout puts `x` at
        `method, 1U`.
      - Every 043e1d41 change to run_l2trans.ps1 is now accounted for:
        - the unit_body_hosts checks (rewritten for the method field);
        - the $d6 drive (restored);
        - unit_bind_ifdecl turned into a positive body-host case (kept,
          it is the declared-in-body control);
        - the unit_asgn_branch drive read (restored to `method, 1U`, as
          the other two methods in that drive always read).
    - Rerun after that: run_l2trans gen2 ok. It includes unit_own_dirty_rhs
      65/88, unit_asgn_branch, unit_bind_ifdecl 65 and the unit_body_hosts
      checks.
    - run_port_message was rerun on the tree with e2's crash report merged
      (032c5f07): PASS.
    - Landed `28cc0d71` (main `765d00dc`), together with that merge. 0c's
      final run_l2_message_root gate is rerun on it.
- run_l2_message_root: PASS on 28cc0d71 (0c, clean worktree, pin match).
  - Evidence: wt0c_fe000dd6/build/codex/l2_message_root/
    20260914_062816_505_ac3ac45c.
  - 561 stages ran. All 336 refusals exit 1, including the for_arrays,
    array metadata and nested_continue compiler OOM sweeps.
  - array_zero and array_nested pass.
  - The driver link, allocation-fault modes 0-48, normal exit, splice and
    nested branch pass.
  - Check 3 passes: unit_text_heap and unit_foreign_resize run with their
    expected outputs, plus the lm_own_* prototype check.
  - unit_own_dirty_rhs gives 65/88.
  - char_known_path and own5/own6 pass, and the signature contracts pass.
  - Native differentials: c_quoted 91150 checks, c_surface 1197191.
  - The owned-source, pin and runner-hash end checks pass. The historical
    catalog audit is off by default.
  - The gate had last passed on 2026-09-12. It was restored through 0c's
    201b43f0, b636afed, 3ea8c207 and 19d6e076, and d6's 3b00f53a,
    cd7c7e21 and 28cc0d71.
  - run_l2_message_root joins the gate list for any translator or
    message-root change.
- Stage 3a-2, increment 1 (d6), on branch d6/exec-3a2, based on bf1f1f35,
  built in a separate worktree. It touches exec.c and the Exec selftest
  only, not lmx_message.lm1, so no L2 mirror is needed.
  - LmxMsgExecBind.in_table is set where nbind counts the entry, and
    cleared in unbind_slot_locked and in detach.
  - bind_rec_locked(m) returns the Message's own record only while it is a
    counted entry, and runs under the exec lock.
  - run_one's three held updates and lmx_msg_run_child_turn read that
    record instead of scanning e->bind by address.
  - The exec selftest has no case that reaches the difference, so there is
    a new one, ctx_unbound_record: bind, unbind, then run_child_turn must
    be INVALID, the turn must not run, and the record stays on the
    Message.
  - Red-first by mutation, because HEAD also refuses: with bind_rec_locked
    ignoring in_table, the reference run fails "unbound record admitted a
    turn ... st=0 done=1".
  - With the increment, run_port_message PASSes, and ctx_unbound_record is
    printed by both reference and both parity runs.
  - Still running from the worktree: scenario36, sched_record, run_lmx
    -Suite Message and the five message modules.
  - Next: increment 1b (map_child and the rebind loop), then increment 2
    (take_this and context_worker through a record pointer on the wait).
  - Worktree gates on b63fbb2a: scenario36 49/0, 27/0, 32/0, 54/0, 24/0;
    sched_record 35/0; run_lmx -Suite Message ok; history 65/0,
    roots_stale 27/0, visit 148/0, liveness 97/0, sched_ready 20/0.
  - Merged into integration as `a545c04a`. The integration translator had
    changed lmx_message's L1 since the branch base, so run_port_message
    was rerun on the merged tree: PASS, and ctx_unbound_record printed by
    both reference and both parity runs.
  - Landed `a545c04a` (main `eb841ff6`).
  - Increment 1b in the worktree: lmx_msg_map_child reads the child's own
    record, both before the launch and on the launch-failure path.
    - The rebind loop in lmx_msg_exec_bind keeps its scan, because it
      passes the table index to join_bind_worker, which belongs to 3b's
      index API.
    - ctx_unbound_record adds a check that map_child refuses the unbound
      child without marking it mapped.
    - Red-first uses a separate mutation that affects map_child only:
      c->exec_bind without in_table. The earlier bind_rec_locked mutation
      stops at the run_child_turn check first, so it cannot prove this
      one.
  - 0c's pre-probe of the unreached tail on the same translator passes:
    - the signature contracts for add, entry_plus, entry_sum and
      entry_swap_formals, with all four cross-assertions;
    - parser_c_quoted and c_surface translate.
    Still uncovered: unit_char_known_path, unit_own5/6 and the native
    c_quoted/c_surface differentials.
  - 0c's bisection:
    - scalar_read comes from 7d7ec87c ("translate library units and native
      manager operations"). That commit put the decay admission in
      l2_check_primary, directly above the "own array use not yet
      supported" refusal, which it made dead code.
    - address comes from 485f15cc's flat-field `@` skip, on top of that
      admission.
    - nested comes from 043e1d41 ("host executable control bodies in
      graph"), which hosts it on purpose.
- 0c's run_l2_message_root commits, on origin/integration as a fast-forward
  from 3b00f53a:
  - b636afed: follows the lmx_array_new_owned constructor at all four
    sites. Red-first: the wrap on the old name fails array fault modes
    24-48, the renamed wrap passes 49/49, and with no wrap the link fails.
  - 3ea8c207: expected refusals are judged by exit code, through
    Invoke-RootRefusal, not by PS 5.1 stderr.
  - 19d6e076: array_zero and array_nested are accepted stages with
    constructor assertions. Red-first on 043e1d41~1 and 249e1c7d.
  - Remaining red in that runner: array_invalid scalar_read and address,
    until the narrowed decay step lands.
  - Measured by 0c on 19d6e076 (clean worktree, pin match): FAIL at
    array_invalid_scalar_read only.
    - Evidence:
      wt0c_fe000dd6/build/codex/l2_message_root/20260914_054754_455_a45407d4.
    - 303 stages ran; array_zero and array_nested pass.
    - Of 234 refusal stages, every exit is 1 except scalar_read's 0.
      address comes next and was not reached.
    - The driver's allocation-fault modes and check 3 were not reached.
  - Early evidence for check 3, outside the gate. This is 4abf4fba's
    lm_own_* prototype check.
    - 0c rebuilt its two fixtures exactly as the runner does:
      unit_text_heap from parser_text_heap.lm2, and unit_foreign_resize from
      the parser_indent_stack slice plus the runner's $resizeSource.
    - Both translate with the 19d6e076 translator.
    - Both emit the prototype: block for lm_own_new_zero, lm_own_resize,
      lm_own_copy_bytes and lm_own_delete, and neither emits a
      l2_foreign_alloc or l1src/own.lm1 predef.
    - The runner's exact condition passes for both. With the declaration
      renamed away, it fails for both.
    - Still unproven: compiling, running and matching expected output,
      which the full rerun covers.
  - Negative fixtures address_array_element and address_array_element_sum
    (Invoke-Negative) both translate on HEAD.
  - Sweep of 375 .lm2: only those two fixtures changed, from exit 0 to
    exit 1 at 9:13 with the new diagnostic. No accepted source uses `@`
    on an own Array element.
  - Gates: run_l2trans gen2 ok (both negatives included); graph ABI
    152/152. In 0c's worktree the patch let run_l2_message_root pass every
    index, char_index, length, for_paths and node_paths refusal group.
    The address variant of each group is now refused.
- Landed `c4a77e64` (main `35681313`).
- C99 octal and hexadecimal integer constants (0c), in gates. Spec 3.4.1
  makes numeric literals ANSI C / C99, and its examples list 0123. l2_num
  had refused a leading 0 since 62d0f4f4, and since 770e83e6 the own-Array
  extent goes through it. That broke run_l2_message_root's `[]: int buf
  003`, which 0c bisected. It also left seven l2src/tests fixtures refused
  with "unsupported own array declaration": unit_for_array_paths,
  unit_for_own_arrays, unit_node_array_paths, unit_own_array_char_index,
  unit_own_array_index, unit_own_array_int and unit_own_array_length.
  - l2_num and l2_signed_num accept 0 (or -0) followed by octal digits. `09`
    stays refused, as in C99.
  - l2_c99_value computes a hexadecimal, octal or decimal constant with an
    overflow status. l2_positive_decimal, l2_literal_value and
    l2_array_literal all compute through it. `010` now counts 8, and `0x3`
    counts 3; before, a hexadecimal extent counted 0.
  - l2_array_count_is tells a zero extent from an overflow by that status.
  - Fixture unit_octal_literals: extent 010 is `LMX_TYPE_ARRAY_OF_INT, 8U`,
    index 07 is `[7U]`, and extent 0x3 is `LMX_TYPE_ARRAY_OF_CHAR, 3U`.
    Compiled with gcc -c.
  - Sweep of 371 .lm2: the seven fixtures now translate, and nothing else
    changed. The gate's fixture and its `3U` assertion are unchanged.
  - Removal proof: HEAD refuses unit_octal_literals at 7:5, so none of its
    checks can pass there. `[]: int b 09` is refused by both translators.
  - Gates: run_l2trans gen2 ok; graph ABI 152/152.
- Landed `fe000dd6` (main `68d00f7d`).
- A function passed as a value is itself (5e, parser dump printer; repro on
  sonnet/parser-l2 7e0f3a9a), in gates. The shape is
  `lm_own_ptr_stack_init(stack, lm_own_delete_plain)`. Two defects:
  - A function declared in a predef'd prototype: block resolved in call
    position but was "unresolved name" as a value. l2_check_expr now
    accepts it, as it already accepted a method name.
  - The C call boxed every atom it could not type into an int temporary,
    so a function value became `int: l2_t0 / l2_t0: fn`. That included a
    same-unit L2 callable, which 5e saw translate but which handed C a
    function pointer in an int. l2_ccall_box_int passes a method or
    prototype function as itself.
  - Fixture unit_fnptr_prototype_value is 5e's repro, compiled with gcc -c.
    Both callbacks are emitted as `lm_own_ptr_stack_init(stack, <fn>)`, with
    no boxing temporary. HEAD refuses it at 32:38.
  - Sweep of 372 .lm2: the fixture is the only change.
  - Gates: run_l2trans gen2 ok; graph ABI 152/152.
- run_l2_message_root on fe000dd6 (0c, clean detached worktree): stages
  1-10 pass, including array_entry_L2_to_L1, where `003` now translates.
  The array_entry text check (runner line 106) then fails.
  - Cause: 770e83e6 moved every own-Array allocation to lmx_array_new_owned,
    so the translator no longer emits lmx_array_new_positive_owned. In the
    runtime, the latter is only a wrapper that returns 0 for count 0 and
    otherwise calls lmx_array_new_owned.
  - The runner and driver name the old constructor in four places: text
    counts at lines 106 and 231, `-Wl,--wrap` at 511, and the driver's
    __real_/__wrap_ pair.
  - Ruling (d6): the gate follows the rename and the translator keeps one
    constructor. 0c moves all four sites.
  - Before the change lands, 0c re-proves allocation-fault injection
    red-first. With the wrap gone or pointing at the old name, each array
    fault mode must fail; with the rename, each must pass.
  - Check 3 (lm_own_* prototypes) is still not reached.
- Queued, 2026-09-14:
  - Octal literals (0c, run_l2_message_root): done, see the entry below.
  - A function name as a value (5e): done, see the entry below.
