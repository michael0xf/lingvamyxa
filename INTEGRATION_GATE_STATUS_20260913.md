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
    - Measured: the map-only mutation also PASSes run_port_message, so the
      added map_child check cannot fail. launch_ctx_thread refuses an
      unbound child through its own bind_index, and map_child's failure
      path re-reads the record through bind_rec_locked and resets mapped.
      The first record read is therefore not what refuses an unbound
      child, and for a bound child the record and the scan are the same
      entry.
    - Decision (d6): remove that check. 1b ships as a behaviour-neutral
      change evidenced by the gates, with no mutation-sensitive test until
      3b removes bind_index from launch_ctx_thread. The map_child refusal
      then becomes the record's job and gets its own red-first case.
    - Branch commit f8534687, with the check removed. Worktree gates, all
      green:
      - run_port_message PASS;
      - scenario36 49/0, 27/0, 32/0, 54/0, 24/0;
      - sched_record 35/0;
      - run_lmx -Suite Message ok;
      - history 65/0, roots_stale 27/0, visit 148/0, liveness 97/0,
        sched_ready 20/0.
      Merged into integration as f1a5e046. run_port_message on the merged
      tree PASS, and ctx_unbound_record appears 4 times in the runtime
      stderr. main merge: ae4f4f00.
- Stage 3a-2 increment 2 (in the worktree, not yet gated).
  - LmxMsgBindWait.rec is the record a wait generation serves while it is
    attached. It is set at the three places a new generation is attached:
    rebind, add, launch_ctx_thread.
  - It is cleared before each place a record drops its wait:
    unbind_slot_locked, join_bind_worker, lmx_msg_exec_stop,
    lmx_msg_exec_unbind. detach destroys the wait.
  - take_this takes the worker's record, and both context_worker bodies read
    mine->rec or w->rec instead of scanning e->bind by address. A replaced
    generation sees rec = 0 and exits, as the scan exited on
    entry->wait != mine.
  - Behaviour change, POSIX worker only: when no entry had its address at
    all, the scan kept waiting. A detached generation now exits, as the
    Win32 worker already did.
  - Red-first plan: the existing stale-launch and reap cases go through
    lmx_msg_exec_unbind during a launch. Leaving rec set at that detach
    site must turn them red. A new case is written only if they stay green.
  - Result: both rec-clear mutations stayed green, the unbind site and the
    unbind_slot_locked site alike (run_port_message PASS on each). The
    clears were not load-bearing. Every detach retires its wait under the
    exec lock: bind_reap_push sets retired, and so do join_bind_worker and
    stop. Both workers exit on retired at the top of the loop, before rec is
    read. The rec re-tests (in_table, addr, wait != mine, retired) duplicated
    that exit.
- Increment 2b: one generation invariant. A non-retired wait's rec is its
  live record.
  - Deleted: the four `rec = 0` clears, and the in_table / addr /
    wait != mine / retired terms in both worker guards.
  - Kept: gone and UI affinity. They refuse a turn of a live record; they
    do not test the generation.
  - Red-first: remove the loop-top retired exit from both workers. The
    stale-launch and reap cases must go red.
  - Result: run_port_message on 2b PASS. The mutation went red as a hang,
    not an assertion (build port_message/20260914_064914_687). reference.exe
    was killed, the runner reported exit 1, and exec.c was restored
    byte-exact.
  - Where the hang is. The stderr tail stops mid-word in "drop_binds
    retires two ready owners disti", but that is a single fprintf and
    stderr to a file is buffered. So the tail does not locate the hang.
  - Located with gdb, attached to the same binary rerun (the hang
    reproduces). The main thread is in main -> lmx_msg_exec_unbind ->
    bind_reap_join_all -> bind_wait_join, blocked in WaitForSingleObject on
    a retired generation's thread. Three context_worker threads sit in
    WaitForMultipleObjects. A retired worker no longer exits, so the join
    in unbind never returns.
  - This red is a timeout. It blocks inside a runtime call, so the case
    cannot bound it with its own wait. A selftest-wide watchdog thread
    could turn it into a failing line; that is not done.
  - Branch commit 7e6f5929 (increment 2 + 2b, exec.c only). Worktree gates,
    all green:
    - scenario36 49/0, 27/0, 32/0, 54/0, 24/0;
    - sched_record 35/0;
    - run_lmx -Suite Message ok;
    - history 65/0, roots_stale 27/0, visit 148/0, liveness 97/0,
      sched_ready 20/0 (at -CoreCommit HEAD).
  - Merged into integration as 4f44d192. run_port_message on the merged
    tree PASS.
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

## 22. Stage 3b — inventory, and what is deleted rather than moved

The contract is in L2_RUNTIME_PLAN (b82e0146). The inventory below is on
integration b4b6933a, with exec.c line numbers. Branch d6/exec-3b.

- Per-parent data already exists. It is kept, and the 3c-2 record mirrors it.
  - ANY ready set: owner->map_ready/map_ready_tail, and child
    map_next/map_owner/map_queued.
  - UI ready set: owner->ui_map_ready/ui_map_ready_tail, and child
    ui_map_next/ui_map_owner/ui_map_queued.
  - Helpers: map_ready_enqueue_kind (1523) and map_ready_unlink_kind (1563).
  - Lifecycle: lmx_msg_endp_try_retire (1094) and map_ready_pend_retire
    (1509) refuse to retire an owner while it has a queued child.
- Global, UI. This is the one cross-parent walk (contract point 2), and it
  is kept behind one function.
  - The walk is e->ui_map_own_head/tail. The per-parent "raised for UI" bit
    is ui_map_own_queued.
  - Consumer: lmx_msg_exec_ui_step (3550) -> take_ready(e, 1) -> lm1
    take_addr(1) -> scan_ready + take_map_kind_locked(rt, 1).
  - ui_step callers: the exec selftest and
    tests/lmx_msg_send_local_selftest.lm1 149-152. No production host in
    the repo calls it.
- Global, ANY. No production consumer, so it is deleted under decision 12.
  - What goes: e->map_own_head/tail, owner->map_own_next/map_own_queued,
    take_map_locked, and the take_addr(0) branch (lm1 761, lm2 802).
  - take_ready(e, 0) has no C caller; ui_step is the only caller of
    take_ready.
  - Users: selftest take_addr(rti, 0) at 6670-6672 and map_nready at about
    45 lines. These are rewritten on per-parent counts; a check that cannot
    move goes to e2 first.
  - e2 confirmed there is no other ANY consumer. Bound ANY children run on
    their own context worker (own wait, take_this). Unbound children run on
    the parent's thread through sched_step. Host-driven turns go through
    run_child_turn and drive.
- Global wake. It is write-only, so it is deleted under decision 12.
  - Win32 ready_ev (93): created 1160, closed 1212, SetEvent at 1387
    (wake_locked), 1406 (wake_addr_locked), 3673 (stop).
  - POSIX ready_cv (98) and ready_sig (100): broadcast at 1390, 1409, 3675;
    ready_sig reset at 3508.
  - Nothing waits on them or reads them: no WaitFor* and no
    pthread_cond_wait in exec.c, and 0 selftest hits.
  - wake_locked and wake_addr_locked keep their per-record bind_wait_signal
    loops. Those are the real ANY wake, each child's own context.
- Order to preserve (take_map_kind_locked 1727-1786). The UI walk keeps it,
  and 3c-2's dequeue is "first eligible" over it.
  - Parents: round-robin by the arrival order of their raise. Take the head,
    unlink it, and re-enqueue it at the tail while its set is non-empty.
    Stop at the first parent seen again, or when the list is empty.
  - Children: FIFO from the ready head. Held or launching children are
    skipped and stay queued. Gone, wrong-affinity or non-runnable children
    are unlinked. The first eligible child is marked held and returned.
- Tripwire before deleting. Not committed, and unconditional, because
  run_port_message builds both of its binaries with -DLMX_MSG_EXEC_TEST.
  - take_map_kind_locked(rt, 0) prints "TRIPWIRE 3b" and aborts.
  - run_port_message must go red at the selftest's take_addr(rti, 0) until
    those calls are rewritten, and then pass. scenario36 must pass
    throughout.
  - Result (wt3b at b4b6933a, run_port_message evidence
    build/port_message/20260914_070639_724). run_port_message exits 1.
    reference.1.stderr ends with "exec wait: owner B progresses while owner
    A stays runnable; 1 worker" and then the TRIPWIRE line. The tripwire
    fflushes stderr before abort, so this order is real. The next ANY take
    is the owner-scale ANY case, selftest 6670-6672, the only take_addr(rti,
    0) calls.
  - scenario36 with the same abort: 49/0, 27/0, 32/0, 54/0, 24/0, both runs
    agree, production runtime. exec.c was restored, hash checked.
- 3b-4 rewrite plan.
  - Delete the owner-scale ANY case (6630-6685). It pins a mechanism that is
    deleted; its UI twin (6687-6755) stays.
  - map_nready assertions become per-child reads of
    lmx_msg_exec_map_queued(rt, child), which each case can name: OOM 2285
    and 2402 (the wo[k] under po), stop 6121/6131, the "ANY set empty"
    checks in the UI cases 6235-6542, and 7673.
  - Diagnostic-only uses drop the count: 1322, 1333, 1516, 2266, 2283, 2335,
    2370, 6605, 8689.
- run_msg_exec_oom (0c's measurement, 2026-09-14). Red in both modes:
  - the pin b89c01cf: compile_exec_oom, header drift;
  - HEAD: selftest 2285, map_nready < 8. In six runs the value read at that
    check was 4 or 5, never 8. The count read just after the eight fills
    varied from 5 to 11. The failure line prints the earlier read as
    nready= and the checked value as map=. (0c corrected an earlier
    "nready 6", which was the earlier read.)
  Both of its mechanisms are 3b deletions: the global ANY count at 2285,
  and fail_grow, which injects into bind table growth (exec.c 1346-1352,
  nbind vs bind_cap) and goes with the table in 3b-7. It is retired in 3b
  under decision 12, unless the parent-owned context list keeps an
  allocation worth injecting into; that is decided in 3b-7. 0c commits the
  manifest port with the pin kept and both reds stated.
- Branch d6/exec-3b (worktree wt3b, off integration 71ece22d; it holds a copy
  of the pinned gen2 translator, 722AC86E).
  - 2f8e6abb, 3b-4a (selftest only). Proof: with the ANY take aborting
    unconditionally, run_port_message PASSES and scenario36 passes
    (build/port_message/20260914_071347_058). Before this commit the same
    tripwire was red at the old take_addr(rti, 0).
  - 8bde43bc, 3b-3 (exec.c only): ready_ev, ready_cv and ready_sig deleted.
    Gates all green: run_port_message PASS; scenario36 49/0, 27/0, 32/0,
    54/0, 24/0; sched_record 35/0; run_lmx Message ok; history 65/0,
    roots_stale 27/0, visit 148/0, liveness 97/0, sched_ready 20/0 (at HEAD);
    send_local 146/0.
  - 3b-4 touches lm1, lm2 and the runner pin, by e2's rule (a).
    - lm2 is the mechanical mirror of the lm1 hunk in the same commit.
    - run_port_message's pinned msg_exec_take_addr signature moves in that
      commit, because the ABI narrowed.
    - e2 reviews the lm2 hunk before the integration merge.
    - Renames: take_map_kind_locked(rt, want_ui) -> take_ui_map_locked(rt);
      owner_ready_* -> ui_owner_raise/lower; take_addr(rt, want_ui) ->
      take_addr(rt); take_ready(e, want_ui, snap) -> take_ready(e, snap).
    - Deleted: take_map_locked, map_nready, and the ANY owner fields.
    - Committed as 64c37c0a (6 files, +65/-155). The apply script checked
      that take_map_locked occurs 0 times in lm1, lm2, exec.c and
      lmx_message.h, and that want_ui occurs 0 times in the runner, lm2, lm1
      and exec.c.
    - Gates all green: run_port_message PASS (87 methods); scenario36 49/0,
      27/0, 32/0, 54/0, 24/0; sched_record 35/0; run_lmx Message ok; history
      65/0, roots_stale 27/0, visit 148/0, liveness 97/0, sched_ready 20/0
      (at HEAD); send_local 146/0.
    - Pushed on d6/exec-3b; e2 reviews the lm2 hunk before the integration
      merge.
- 3b-5 question (lmx_msg_exec_ready's by-position lookup through m).
  - lmx_msg_find_tree skips RELEASED Messages. So for a bound but RELEASED
    child, exec_ready sees m == 0 while the bind[] scan still says bound,
    and the path ends in wake_addr_locked.
  - LMX_MSG_AFFINITY_ANY is 0, so "bound" must stay its own read.
  - Tripwire: wake_addr_locked aborts on a bound address with no findable
    Message. If it stays green, a lookup through m loses nothing the gates
    exercise. If it goes red, the wake must be kept behind one exec.c
    function until 3b-7.
  - Result on 64c37c0a: green. run_port_message PASS, no TRIPWIRE line
    (build/port_message/20260914_072612_341). scenario36 49/0, 27/0, 32/0,
    54/0, 24/0. exec.c restored.
  - So 3b-5 routes exec_ready through m's own record, behind one exec.c
    entry (proposed lmx_msg_exec_route_locked(m, &ui, &pool)). With the lane
    take, that is 3c-2's seam.
  - 64c37c0a was reviewed by e2 (lm2 hunk line for line) and approved for
    the integration merge.
- 3b-6, eb879c23 (touches lm1 and lm2): lmx_msg_exec_drop_stale_ready, its
  exec.h prototype, the drop-stale-retire selftest case and the OOM
  housekeeping call are deleted. The apply script checked:
  drop_stale_ready occurs 0 times in lm1, lm2, exec.h, the selftest and
  exec.c. Gates running; e2 reviews the lm2 hunk.
  - Gates on eb879c23, all green: run_port_message PASS (86 methods);
    scenario36 49/0, 27/0, 32/0, 54/0, 24/0; sched_record 35/0; run_lmx
    Message ok; history 65/0, roots_stale 27/0, visit 148/0, liveness 97/0,
    sched_ready 20/0; send_local 146/0. e2 approved the lm2 hunk.
- Integration merge 924a6b69 = d6/exec-3b at eb879c23 (3b-4a, 3b-3, 3b-4,
  3b-6). On the merged tree: run_port_message PASS (86 methods),
  sched_record 37/0.
- 3b-5, widened with e2's agreement. lm1 and lm2 stop enumerating bind[]
  by position.
  - exec_ready routes through lmx_msg_exec_route_locked(m, &ui, &pool).
  - scan_ready's walk moves behind the lane take as the static
    lane_scan_ready_locked.
  - The by-position accessors stay declared, only for run_l2trans's
    own-local fixture and tests/l2_and_foreign_call_own_local.lm2.
  - e2's check: the accessors occur 0 times in lm1/lm2, route_locked is
    called once in each, and scan_ready occurs 0 times in lm1/lm2.
  - Then a separate commit adds ready_owner_of(child): one owner derivation
    (parent_msg or the Message itself), the line stage 5 may change.
  - Committed as d2b7ce61 (4 files, +61/-96) and pushed on d6/exec-3b.
    Checked on the committed tree: the accessors occur 0 times and
    route_locked once in lm1 and in lm2; scan_ready occurs 0 times in
    lm1/lm2 and twice in exec.c (definition and one call).
  - The first apply run stopped at an exec.c count of 4. Two comments named
    scan_ready: the new catch-up's comment and an older tab-accessor note.
    Both were reworded, the checks rerun, and then the commit made.
  - Gates running; e2 reviews the lm2 hunk.
  - ready_owner_of: the derivation occurs 4 times in exec.c (enqueue UI and
    ANY, unlink fallbacks UI and ANY) and 0 times in lm1, lm2 and
    lmx_message.h. 3b-5b replaces all four.
  - e2 reviewed and approved the d2b7ce61 lm2 hunk.
  - Gates on d2b7ce61, all green: run_port_message PASS (85 methods);
    scenario36 49/0, 27/0, 32/0, 54/0, 24/0; sched_record 35/0; run_lmx
    Message ok; history 65/0, roots_stale 27/0, visit 148/0, liveness 97/0,
    sched_ready 20/0; send_local 146/0.
- 3b-7 decisions (e2, 2026-09-14).
  - Option (A): a bound record is linked at bind time onto
    ready_owner_of(child), in LmxMsg.ctx_head / LmxMsgExecBind.ctx_next, and
    unlinked at unbind.
  - The plan's "parent that mapped the child" is the same Message, because
    map_child refuses c->parent_msg != p. start_contexts keeps reaching
    bound but unmapped children.
  - start_contexts, stop and drop_binds walk parents from rt->root and each
    parent's ctx list: the one cross-parent walk, in exec.c.
  - 3c-2 moves ctx_head/ctx_next into the record as a contexts ring.
  - The rebind branch in lmx_msg_exec_bind (rec->msg != m) is deleted. It
    cannot be true once the record is reached as m->exec_bind; it left with
    the scan.
  - Falsifiers: `e->bind[` drops to nothing but the listed walks, and
    "addresses are never reused" no longer appears in exec.c.
- 3b-5b, 51d026aa (exec.c only): ready_owner_of(child) replaces the four
  owner derivations. The derivation now occurs once; ready_owner_of occurs 5
  times. Gates all green: run_port_message PASS (85 methods); scenario36
  49/0, 27/0, 32/0, 54/0, 24/0; sched_record 35/0; run_lmx Message ok;
  history 65/0, roots_stale 27/0, visit 148/0, liveness 97/0, sched_ready
  20/0; send_local 146/0.
- 3b-7a owner storage. e2 asked what happens when a bound child's parent
  changes.
  - Measured on 51d026aa: parent_msg is written only at
    lmx_msg_child_link (lm1 591, at create) and lmx_msg_child_unlink (622,
    P -> 0, together with the sched and map unlink). The latter's only
    production caller is the child's own lmx_msg_release_slot.
  - CORRECTION (same day). The next two claims, that a bound Message cannot
    reach release_slot and that only the fabricated helper changes
    parent_msg while bound, were an inference, not a reading, and they are
    wrong.
    - lmx_msg_release_slot (lm1 1314-1358) first calls child_unlink
      (1320-1321, parent_msg -> 0), then later unbinds the Message itself
      (1356 lmx_msg_exec_unbind) and releases it (1357).
    - A production caller is the failed end_turn path (lm1 1425-1430),
      which releases uncommitted INACTIVE children; such a child can be
      bound.
    - So parent_msg does become 0 while the record is still in the table.
      An owner derived at unlink would search the wrong list and leave the
      record linked on the parent. This is why map_owner is recorded.
  - Superseded: a bound Message holds the table's retain, so it cannot
    reach release_slot while bound (wrong, see above). runtime_delete drops
    binds (903) before it frees slots (930). exec.c never assigns
    parent_msg.
  - The "map-reparent" selftest case does not reparent.
  - Superseded: only the fabricated helper detach_child_keep_ready changes
    parent_msg while bound (wrong, see above).
  - Consequence: the owner is recorded at link (ctx_owner, like map_owner),
    or release_slot must unbind before child_unlink (an lm1+lm2 change).
    Recommended to e2: record it. 3b-7a waits for e2's answer.
  - The path is pinned by the gate. The failed-turn case ("failed-turn
    uncommitted child unlinks map_ready; sibling kept", selftest
    ~7544-7631) binds the uncommitted kid and releases it through the
    failed end_turn.
- e2's decisions after the correction.
  - 3b-7a stores ctx_owner at link, with release_slot's order as the
    reason.
  - 3b-8, after 3b-7d and before e2's C half of 3c-2: lmx_msg_release_slot
    unbinds before child_unlink (lm1+lm2, rule (a)). In the same step
    map_owner, ui_map_owner and ctx_owner all go, derived from
    ready_owner_of. Oracle: the failed-turn case. Recorded in
    L2_RUNTIME_PLAN on main 2f3ad37a.
  - The two fabricated cases (stop-retire, drop_binds-retire) move to 3b-7b
    with the detach_child_keep_ready = 0 falsifier. Replacement assertions
    go to e2 before any deletion.
  - The redundant clears of ctx_owner/ctx_next at unlink go in a follow-up
    before the merge, so ctx_owner is assigned in one place.
- 3b-7a red-first, run_port_message on the applied tree:
  - link removed: exit 1, "CTX AGREE FAIL at bind: record 0 of 1 is on its
    owner list 0 times" (build/port_message/20260914_074853_818);
  - unlink removed: exit 1, "CTX AGREE FAIL at unbind: owner lists hold 74
    records, table 73" (20260914_074905_135);
  - exec.c restored by hash.
- 3b-7a, unmutated applied tree: run_port_message exit 1, "CTX AGREE FAIL
  at bind: record 1 of 3 is on its owner list 0 times", in parity.2 only
  (20260914_074917_401). Not committed: the commit step also failed,
  because a PowerShell here-string with double quotes split into git
  pathspecs.
  - Diagnosis. The abort follows 7817 "one owner on ANY+UI ready lists
    retires exactly once", so it is in the fabricated stop-retire case, at
    its restart bind (c1, c2, dummy: record 1 = c2).
  - detach_child_keep_ready takes c1/c2 out of their parents' families
    without child_unlink. stop then retires pm1/pm2 while c1/c2 stay bound
    and linked on them, and the check reads freed pm2. That is a
    use-after-free, seen only when dummy reuses pm2's memory.
  - Evidence that it is intermittent: the gate run over the same dirty
    tree minutes later passed run_port_message (85 methods), and all ten
    gates were green.
  - try_retire (exec.c 1100-1109) does not know ctx_head.
  - Production equivalent: a bound child leaving its parent through
    lmx_msg_child_unlink (the release_slot window). That hook,
    lmx_msg_map_ready_unlink, already moves the ready entries off the
    parent, but not the context.
  - Options sent to e2. (A) the retire gates read ctx_head, which turns
    red the three owner-retire cases that child_unlink a bound child. (B)
    the hook also moves the context onto the child, the check asserts
    owner in {Message, parent_msg}, and the two fabricated cases and the
    helper are deleted. Recommended: B.
  - e2 decided (B).
  - The owner check is "ctx_owner is the record's Message or its
    parent_msg", not strict ready_owner_of equality. The hook moves the
    context under the exec lock, and L2 clears parent_msg afterwards,
    outside the lock on the release_slot path (lm1 1429), so strict
    equality would abort on a correct state in that window.
  - The replacement case e2 sketched (released family, stop retires
    nothing, drop_binds retires the family exactly once) is not reachable.
    try_retire refuses parent_msg != 0 (exec.c 1104), so children released
    through drop_binds keep their family. The reachable shape goes through
    lmx_msg_release_slot (failed end_turn); it is designed in 3b-7b, with
    assertions to e2 first.
  - For 3b-8: once release_slot unbinds before child_unlink,
    lmx_msg_child_unlink of a bound child has no production caller, and
    (B)'s move becomes test-only. 3b-8 then decides whether child_unlink
    refuses a bound child, as a contract at the family boundary rather
    than a defensive check. If it does, the three owner-retire cases
    (7690, 7762, 7817) are rewritten to unbind first, and the owner check
    can become strict ready_owner_of equality.
  - A second reason for 3b-8 (e2): the lock gap itself. child_unlink's hook
    (the ready-entry unlink and the context move) runs under the exec lock,
    and L2 clears parent_msg after it, outside the lock on the release_slot
    path. Executor state and family membership then change in two steps
    that another thread can observe between.
  - e2 accepted both adjustments. The go for 3b-7a (B): three red-first
    mutations, each with the case it aborts after; the unmutated run; an -F
    commit; 10 gates.
- 3b-7a (B) red-first, run_port_message on the applied tree (proof_3b7aB;
  exec.c restored by hash):
  - link removed: exit 1, at the first bind after "end_turn returned 0":
    "CTX AGREE FAIL at bind: record 0 of 1 has an owner that is neither its
    Message nor its parent" (20260914_075656_387).
  - unlink removed: exit 1, at an unbind after "mass 70 ok": "owner lists
    hold 74 records, table 73" (20260914_075707_850).
  - move removed: exit 1, at an unbind after "two owners retire exactly
    once from one batched unlink" (inside the 7817 case, which
    child_unlinks a bound child by hand): "record 0 of 1 has an owner that
    is neither its Message nor its parent" (20260914_075719_930).
  - Why the failed-turn case does not catch a skipped move. release_slot
    unbinds the child right after child_unlink; the stored owner makes that
    unlink correct; and the check runs after the record has left the
    table. The move is reached by the cases that child_unlink a bound child
    and keep it bound.
- 3b-7a (B) committed as 4b726b7f (lmx_message.h, lmx_message_exec.c,
  selftest; +146/-171), via a BOM-free -F file with exact paths. Unmutated
  run_port_message PASS. Gates all green: run_port_message PASS (85
  methods); scenario36 49/0, 27/0, 32/0, 54/0, 24/0; sched_record 35/0;
  run_lmx Message ok; history 65/0, roots_stale 27/0, visit 148/0,
  liveness 97/0, sched_ready 20/0; send_local 146/0. The follow-up that
  removes the redundant clears at unlink comes before the merge.
- 3b-7a follow-up 5cdbd747 (exec.c, two lines removed: ctx_unlink_locked
  no longer clears the record's links, so ctx_owner is assigned only at the
  link). Gates all green: run_port_message PASS (85 methods); scenario36
  49/0, 27/0, 32/0, 54/0, 24/0; sched_record 35/0; run_lmx Message ok;
  history 65/0, roots_stale 27/0, visit 148/0, liveness 97/0, sched_ready
  20/0; send_local 146/0.
- Integration merge 45b62efb (d6/exec-3b at 5cdbd747). On the committed
  merge: run_port_message PASS (85 methods), sched_record 46/0. Main
  a7c0fe17.
- 3b-8 prep, drafted and dry-run on copies of 5cdbd747, not applied.
  - The reorder hunk is identical in lm1 and lm2.
  - map_owner and ui_map_owner are derived through ready_owner_of.
  - Deriving ctx_owner waits for e2's child_unlink contract: with B's move,
    the hook puts a context on the child while parent_msg stays set outside
    the lock.
- 3b-8 as decided with e2.
  - Reorder: release_slot unbinds right after the m = 0 guard, before
    child_unlink, in lm1 and lm2 (rule (a), e2 reviews the lm2 hunk).
  - Stored owners: map_owner, ui_map_owner and ctx_owner all go, derived
    through ready_owner_of. The (B) move goes. The TEST check becomes
    strict: each table record is on ready_owner_of(its Message)'s list
    exactly once.
  - Contract: lmx_msg_child_unlink becomes `fn: ... int` (C prototype
    int), LMX_MSG_INVALID for a bound child (lmx_msg_exec_msg_bound, under
    the exec lock), LMX_MSG_OK otherwise. release_slot keeps a bare call.
    The translators already accept bare statement calls to fn: methods:
    18 in each of lm1 and lm2, e.g. lm1 61 lmx_msg_mark_from.
  - Cases: 7690, 7762 and 7817 unbind before they unlink. sched-snap
    unbinds c2 before arming its hook, with its assertions unchanged. 7690
    first asserts the refusal, which is the contract's reaching test.
  - Red-first mutations: the reorder undone (failed-turn case), the refusal
    removed (7690's new assertion), unlink deriving the wrong owner (the
    strict check).
  - Risk named before measuring. After unbind-first, nothing may
    pend_retire a released parent whose last child then leaves the family
    (child_unlink's hook returns early for a child with no ready entry), so
    7690/7762/7817 may go red on their retire counts.
    - e2's model position: such a parent holds no edge and must retire.
    - If it goes red, the question is whether production reaches that
      state. If yes, the trigger at child_unlink's tail is its own step; if
      no, the cases move to the release_slot shape. Nothing is adapted
      inside 3b-8, and the counts go to e2 first.
  - Dry run on copies of 5cdbd747: all three scripts apply; map_owner,
    ui_map_owner and ctx_owner are 0 in lmx_message.h and exec.c;
    detach_child_keep_ready is 0; each mutation touches the expected files.
    The chain runs in wt3b and commits only if the unmutated run is green.
- 3b-8 measured (wt3b, 5cdbd747 + full apply; not committed):
  - Unmutated (build/port_message/20260914_081423_870): red at 7690, after
    the failed-turn case (which passes): "exec owner-retire did not free
    after child_unlink n=2 pend=0". The risk named above: after unbind-first
    nothing queues the released pm for retire. The parity runs did not
    start.
  - Refusal removed (081403_252): red at 7690's new assertion, "child_unlink
    accepted a bound child".
  - Wrong owner at unlink (081412_763): red, "CTX AGREE FAIL at unbind:
    owner lists hold 74 records, table 73".
  - Reorder undone (081353_910): red earlier than predicted, in the rollback
    case: "rolled-back bound child not retired n=3 bind=0". With the
    contract in place, a late unbind makes release_slot's child_unlink
    refuse the still-bound child.
  - Chain-script bug: `$null = Invoke-PortMessage` swallowed the verdict
    lines, and the return capture made any run look red. The verdicts above
    come from the logs and stderr.
  - Reading, not measured: production's only path out of a family is
    release_slot, whose only caller is the parent's own failed end_turn, so
    the parent is running, not RELEASED. A parent released later retires
    through its own endp_release. So the owner-retire cases' state looks
    unreachable in production.
  - Asked e2: delete 7690/7762/7817 and give the contract assertion its own
    case; or a child_unlink-tail retire trigger as its own step first; or a
    tripwire to measure reachability.
- e2's decision: (a), conditional on one measurement, because deleting
  cases under decision 12 needs a measured "no production path", not a
  reading.
  - Tripwire: "a child leaves a RELEASED parent", at child_unlink's C hook
    lmx_msg_map_ready_unlink (only callers lm1 621 and lm2 660, before
    parent_msg is cleared).
  - Runners: the production-runtime runners only (scenario36, family
    handoff with -CoreCommit on a local throwaway commit, liveness, run_lmx
    Message). The Exec selftest is excluded, because its three cases
    fabricate the state.
  - If silent: delete 7690/7762/7817; the contract's reaching assertion
    (child_unlink refuses a bound child, red-first with the refusal
    removed) gets its own small case; and this note says a child leaves the
    family only through release_slot from its parent's own failed end_turn,
    so a released parent never loses a child. If a future path adds one, the
    fix is a retire trigger at child_unlink's tail.
  - If it fires: the runner and scenario line go to e2, and the trigger
    becomes its own step before 3b-8.
  - The reorder-undone mutation is red by the contract in the rollback case
    ("rolled-back bound child not retired n=3 bind=0"), earlier than the
    failed-turn case. Both count as its reds.
- Model question answered by Mikhail (decision 17; spec 19.29.6, model
  section 32, LEAD_REVIEW, plan stage 4). Question: a released parent whose
  children are disposed but still linked never retired until
  runtime_delete. Ruling: a released branch is never retained, and release
  does not wait for the children.
  1. A parent's forced release of a child starts a chain down the subtree.
     Each Message closes and releases its own direct children, and that
     chain frees the branch's slots and arenas, never runtime_delete.
  2. A child's self-close on timeout starts the same chain for its subtree.
  3. The runtime never sets success on a parent whose children are
     running=1/success=0. A parent whose own algorithm sets it declares the
     children's work unneeded, and the chain closes them.
  4. Corrected by Mikhail, main 2c11fb5d. A running Message survives its
     parent's closing only through a handoff of supervision to another live
     parent. The closing parent picks the new parent among the capabilities
     it holds, not necessarily its own parent.
     - The handoff moves the child's parent capability and scheduler place.
       The child keeps its arena, mailbox and turn. The child may be bound:
       its record moves between the parents' scheduler records, and
       parent_msg changes, all under the exec lock (3b-9).
     - Storage adoption is a different operation. 19.29.7's "a running
       Message is not transferred" now covers storage only:
       transfer_adopted keeps handoff_ready, non-running and
       msg_bound == 0.
     - A child that is not handed over closes with the chain.
     - For the root the only new parent is the World Wide Mix stub, so a root
       handoff is refused until stage 5.
     - d6 brings the shape of the new operation to e2 before code. It gets
       its own acceptance once it exists; e2's three section 32 falsifiers
       cover the chain only.
  The acceptance asserts the end state after the chain has drained: slots
  gone, parent retired. A retire deferred to the next flush is acceptable;
  one that needs runtime_delete is not.
  After 3b-7d (sequence with e2):
  - e2 first adds three falsifiers to the section 32 test, red on today's
    runtime:
    - a disposed child leaves its family and the released parent retires;
    - a released parent's subtree is freed by the chain;
    - success with running children closes them.
  - d6 then implements in lm1/lm2/exec.c:
    - dispose_child and adopt_failed end by releasing the child's slot;
    - a parent's release closes and releases its subtree;
    - complete() on a parent with running children requests their close.
  - e2's 3c-2 C half touches disjoint functions: the lane take and ready
    sets, against release_slot, dispose_child, adopt_failed and try_retire.
- Measurement result, child-leaves-released tripwire. Throwaway worktree at
  5be1aaf4, local unpushed commit 99b792b4, removed afterwards.
  - Silent: scenario36 (49/27/32/54/24); family handoff (62/0,
    watched_frees=4); liveness (97/0); run_lmx MessageApi, Host and
    Production (all PASS, no marker).
  - The only fire was in run_lmx's executor selftest, right after the
    failed-turn case: "TRIPWIRE child-leaves-released: child 2 leaves
    released parent 1". That is 7690, the fabricated case e2 excluded.
- Standing note (e2): a child leaves its family only through release_slot,
  from its parent's own failed end_turn, so a released parent never loses a
  child. If a future path adds one, the fix is a retire trigger at
  child_unlink's tail, when first_child becomes 0.
- So under (a): 7690, 7762 and 7817 are deleted, and a new unlink-contract
  case pins the refusal. The chain runs in wt3b and commits only if the
  unmutated run is green.
- 3b-8 red-first on the final tree (proof_3b8b), run_port_message, the
  reference build's stderr, files restored by hash:
  - reorder undone: exit 1, "rolled-back bound child not retired n=3
    bind=0" after "ctx_real_clock" (build/port_message/20260914_082143_974);
  - refusal removed: exit 1, "exec unlink-contract bound child left its
    family" after the failed-turn case (082153_076);
  - unlink from the wrong owner: exit 1, "CTX AGREE FAIL at unbind: owner
    lists hold 74 records, table 73" after "mass 70 ok" (082202_742).
- 3b-8 committed on d6/exec-3b as 5fa4c9ad: lmx_message.h, lm1, lm2,
  exec.c, exec.h, selftest; +76/-234; -F commit with exact paths.
  - The unmutated run_port_message on the applied tree passed, and all four
    runs end at the same last case (20260914_082213_226).
  - Gates all green: run_port_message PASS (85 methods); scenario36 49/0,
    27/0, 32/0, 54/0, 24/0; sched_record 35/0; run_lmx Message ok; history
    65/0, roots_stale 27/0, visit 148/0, liveness 97/0, sched_ready 20/0;
    send_local 146/0.
  - e2 reviews the lm2 hunk before the integration merge.
- e2 reviewed 5fa4c9ad (lm2 hunks equal the lm1 hunks). Verified on the
  committed blobs: map_owner, ui_map_owner and ctx_owner are 0 in
  lmx_message.h, exec.c, lm1, lm2 and the selftest; lmx_msg_exec_msg_bound
  is defined once and called once in each of lm1 and lm2.
- Integration merge 9d3b4be6 (d6/exec-3b at 5fa4c9ad). On the committed
  merge: run_port_message PASS (85 methods), sched_record 46/0. Main
  4d6f69b2.
- 3b-7b design, approved by e2 with refinements.
  - One exec.c walk, ctx_walk_locked: iterative over the family trees from
    rt->root. Its unit is the owner: it hands (owner, owner's ctx list) to a
    callback that iterates the records, the shape 3c-2 swaps for parents
    with a record.
  - wake_locked, the lane catch-up and stop's three passes visit in place
    under the lock.
  - No allocation on start or teardown. drop_binds unbinds the first record
    the walk meets and restarts until none is left; start_contexts launches
    the first eligible record and restarts. Termination: a successful
    launch_ctx_thread records the worker under the lock before it unlocks
    (3362-3372), so bind_has_worker is true on the next walk.
    exec_start_map_kick keeps its collect-with-retains structure.
  - detach does not walk: runtime_delete frees every slot (lm1 921-931)
    before lmx_msg_exec_detach (937), and rt->root is not cleared, so a walk
    there would read freed Messages. Emptiness is checked in drop_binds
    instead (TEST: nbind is 0 when the walk finds no record).
  - Any case that depends on global bind order goes to e2. Red-first: a
    walk that visits only root-level Messages must go red through stop.
- Finding (d6, while checking 3b-7b's locking; already present on 9d3b4be6):
  lmx_msg_release_slot mutates the family tree without the exec lock.
  - lmx_msg_end_turn (lm1 1430-1436; lm2 at msg_release_slot's call, 1495)
    unlocks the exec lock around the release of each uncommitted INACTIVE
    child.
  - release_slot then rewrites the chain with no lock held:
    - lmx_msg_child_unlink (lm1 1327; body 601-631) changes parent->first_child,
      the predecessor's next_sibling, parent->last_child, child->next_sibling
      and child->parent_msg.
    - The root-list removal (lm1 1329-1343) changes rt->root or a root's
      next_sibling.
  - Every tree reader holds the exec lock: lmx_msg_find over rt->root and
    each family (lm1 541-553, reached through msg_at_addr), drive_walk_list
    (exec.c 856) and the root unlink in lmx_msg_exec_retire (1107-1124).
    After 3b-7b ctx_walk_locked is one more such reader.
  - The child is freed only by flush_retire under the lock, so a racing
    reader sees a short chain, not freed memory. After 3b-7b, stop and
    drop_binds depend on a complete walk, so a short walk there leaves a
    context unstopped or unbound. The watchdog would report that as a late
    hang with no line.
  - Unmeasured so far; no failure produced.
- e2's decision: fix it as its own step, 3b-9, after 3b-7b's merge and before
  3b-7c; not in 3c, so 3c-2 inherits a tree that stays consistent under the
  lock.
  - release_slot holds the exec lock around child_unlink and the root-list
    removal, in lm1 and lm2 (rule a, e2 reviews).
  - The exec lock is recursive (PTHREAD_MUTEX_RECURSIVE at exec.c 1180, a
    CRITICAL_SECTION on Win32), so child_unlink's callees can still take it.
  - Red-first: a TEST-only hook in release_slot, between the unlock and
    child_unlink. The test's second thread runs lmx_msg_find over the family
    and asserts the full sibling chain. On 9d3b4be6 it sees the chain half
    unlinked and goes red; with the fix the walker blocks until the chain is
    whole.
- 3b-7b committed as ada3f25d on d6/exec-3b (exec.c only, +228 -116).
  - Red-first: a walk that visits only root-level Messages turned
    run_port_message red on "live-cascade turns not started inbox p=0 c=1
    runnable p=0 c=1". Unmutated it passes (85 methods).
  - Gates on ada3f25d: port_message PASS; scenario36 49/27/32/54/24;
    sched_record 35/0; run_lmx Message ok; history 65; roots_stale 27;
    visit 148; liveness 97; sched_ready 20; send_local 146.
  - No order-dependent case surfaced: parity stderr agreed on both runs.
  - Integration merge a05d9a8c. On the merge: port_message PASS,
    sched_record 46/0. Main 7306d6ea.
- 3b-9 red-first as agreed with e2 (option b).
  - At the hook's spot the chain is still whole, so an unlocked reader alone
    cannot fail a chain check. The case asserts the invariant itself: no
    reader takes the exec lock inside release_slot's tree window.
  - The hook waits 300 ms for a reader that takes the exec lock and checks
    p's chain: the count, every parent_msg == p, last_child is the tail, c1
    absent. That chain check stays as the second assertion.
- 3b-9 committed as 57878394 on d6/exec-3b: lm1, lm2, exec.c, exec.h,
  selftest (+158).
  - Red-first: the hook and case without the lock turned run_port_message red
    on "release-tree window reader_in_window=1 chain_ok=1 c1_present=1 n=2"
    (reference build). The reader took the lock inside the window, while the
    chain was still whole.
  - With the lock: PASS (85 methods); the case prints "exec wait:
    release_slot changes the family tree under the exec lock" in all four
    runs.
  - Gates on 57878394: port_message PASS; scenario36 49/27/32/54/24;
    sched_record 35/0; run_lmx Message ok; history 65; roots_stale 27;
    visit 148; liveness 97; sched_ready 20; send_local 146.
  - The lm2 hunk is with e2 for review before the integration merge.
  - e2 reviewed 57878394: lm1 1323-1350 and lm2 1383-1410 add the same five
    lines in the same places; the only other difference is lm2's
    msg_child_unlink spelling. Approved.
  - Integration merge 9400105e. On the merge: port_message PASS,
    sched_record 46/0. Main 07a05ddc.
- 3b-7c committed as 119aa95c on d6/exec-3b (exec.c only, +263 -253).
  - The by-address lookups read the Message's own record through
    msg_find_any_locked, which does not skip RELEASED Messages.
  - The rebind branch, its "addresses are never reused" comment and
    bind_index are deleted.
  - Red-first, against run_port_message:
    - Lookup ignores in_table: red, "CTX FIND FAIL: addr 3 table record
      none, tree record other" (ctx_ui_any_rollback).
    - Find does not descend into children: red, "CTX FIND FAIL: addr 2
      table record set, tree record none".
    - Find skips RELEASED Messages: green. Only release_slot sets
      RELEASED, and since 3b-8 it unbinds first, so the difference is
      unreachable today. e2 accepted this and it is stated in the commit.
  - Gates on 119aa95c: port_message PASS; scenario36 49/27/32/54/24;
    sched_record 35/0; run_lmx Message ok; history 65; roots_stale 27;
    visit 148; liveness 97; sched_ready 20; send_local 146.
  - Integration merge db48f109. On the merge: port_message PASS,
    sched_record 46/0. Main 8f6ae80e.
- 3b-7d decisions with e2:
  - exec_oom: the scenario, its mode flags and the g_oom_* fields are
    deleted in 3b-7d under decision 12. Context lists are intrusive, so
    fail_grow has no allocation left to inject into.
  - Ticket to 0c, after the 3b-7d merge: delete run_msg_exec_oom.ps1 and
    LMX_MSG_EXEC_OOM_TEST.txt, and rewrite RUN_LMX_TESTS.txt:43.
  - The run_l2trans probe l2_and_foreign_call_own_local moves onto the
    D1 pair in its own commit, ahead of 3b-7d.
  - lmx_msg_exec_bind_n stays as a TEST-only count over the walk.
  - Falsifier: grep -c 'e->bind\[\|nbind\|bind_cap\|bind_grow' prints 0
    for exec.c, exec.h and the selftest. Ten gates green; sched_record 46
    on the merge.
- The run_l2trans probe was moved as its own commit, 9b99230d.
  - The probe calls tab_n_locked for n, child_at(rt, 0U, i) for a, and
    tab_addr_locked(rt, i) on the right of &&.
  - Red first: with the moved checks and the old fixture, run_l2trans threw
    "l2_and_foreign_call_own_local does not pass the actual i". With the new
    fixture it ends "l2trans gen2 ok".
  - The first green attempt stopped past the probe on "missing STG gen2
    printTree": wt3b's build held only l1trans.exe. printTree.exe was
    copied from the integration checkout's gen2, next to the same pinned
    l1trans (722AC86E).
- 3b-7d committed as da8076a9: exec.c, exec.h, the selftest and
  run_lmx.ps1 (+35 -763).
  - The falsifier as agreed could never reach 0: `nbind` is a substring of
    every `unbind`. The dry run on copies found it. The check uses
    `e->bind\[|\bnbind\b|bind_cap|bind_grow`, and it prints 0 for all
    three files at HEAD.
  - Also deleted under decision 12: the UI case whose only claim was that
    fail_grow does not strand a UI send, and run_lmx.ps1's two
    production-export checks for set_fail_grow and fail_hits.
  - Red-first, against run_port_message:
    - ctx_visit_count counting no record: red, "mid unroll n0=0 n1=0
      c1=0 c2=1".
    - unbind leaving in_table set: red, "mid unroll n0=1 n1=2 c1=1
      c2=1".
  - No warning in exec.c, exec.h or the selftest in the port_message build
    logs.
  - Gates on da8076a9: port_message PASS; scenario36 49/27/32/54/24;
    sched_record 35/0; run_lmx Message ok; history 65; roots_stale 27;
    visit 148; liveness 97; sched_ready 20; send_local 146.
  - Integration merge 266a6cc8. On the merge: port_message PASS,
    sched_record 46/0. Main 0fb56413.
  - 0c now deletes run_msg_exec_oom.ps1 and LMX_MSG_EXEC_OOM_TEST.txt and
    rewrites RUN_LMX_TESTS.txt:43. e2 takes the C half of 3c-2.
- 0c's two commits were cherry-picked onto integration after 29f80ce5.
  0c checked the patch-ids against its originals; they are the same content.
  - d1db42fd (was fc3cbdaf): run_msg_exec_oom.ps1 and
    LMX_MSG_EXEC_OOM_TEST.txt deleted; RUN_LMX_TESTS.txt:43 rewritten. git
    grep run_msg_exec_oom now matches only this file.
  - f7ba7372 (was a5a01949): l2src/run_gates.ps1, the ten core gates in d6's
    order and invocations. Each verdict is the child's exit code. The chain
    stops at the first red and prints one summary.
- run_gates.ps1 on f7ba7372 (integration checkout, dirty=0, pin 722AC86E
  matching L1_PIN): gates GREEN 10 of 10. Every result line equals d6's
  private chain: parity PASS 85 methods; core tests 49/27/32/54/24;
  sched record 46/0; selected=Message ok; history 65; stale 27; visit 148;
  liveness 97; sched_ready 20; send local 146. 0c's red-first stopped at
  sched_record with the later gates not run. d6 now uses run_gates.ps1
  instead of its private chain.
- Main's decision 17 docs and e2's acceptance test d7eef06b
  (tests/lmx_model_family_release_17_selftest.lm1, opt-in, 26 checks, 6
  failures on today's runtime; the red lines are in RUNTIME_L2_PORTS) are
  merged into integration. No runner default changes.
- Supervision handoff (decision 17 rule 4), shape approved by e2, now being
  written:
  lmx_msg_handoff_supervision(rt, old_parent, child, new_parent).
  - Refusals:
    - no authority of the old parent;
    - p, c or q unresolved;
    - c not a direct child of p;
    - c a root (World Wide Mix, until stage 5);
    - q == p or q == c;
    - q inside c's subtree (a cycle);
    - q STOPPED, DEAD, RELEASED, closing or disposed.
  - A nine-step move under one exec lock, through two exported entries,
    lmx_msg_exec_supervision_detach_locked and
    lmx_msg_exec_supervision_attach_locked. This is the seam where 3c-2
    later swaps the record move in.
  - Rulings:
    - c->parent (the address) moves together with parent_msg, since eight
      readers resolve the supervisor by address;
    - q starts a fresh liveness window: child_heard_at = now, the
      live_query_id/pend reset, tracked kept;
    - the path stays as the creation identity (rule 5: genesis vs
      supervision);
    - create_id is cleared to 0;
    - a bound running q is allowed;
    - addresses are the capability check until 19.28.R2.
  - The mailbox, arena, turn, held, wait and mapped state are unchanged.
- 0c's -FamilyRelease17 opt-in switch: fd075d30 (fast-forward), then the
  column fix and corrected usage comment cherry-picked as 564986e9 (was
  b021fd23). With the switch the chain stops red on "family release 17: 26
  checks, 6 failures" after the default ten.
- e2's fable/exec-3a merged as c7efa936: stage 3c-2a (l2units_build.ps1;
  run_lmx, run_model_scenario36 and run_port_message link the L2 runtime
  units) plus main's decision 17 docs up to 922f751e.
  - The one conflict was RUNTIME_L2_PORTS.txt: HEAD's side was empty and the
    3c-2a section was taken. The decision 17 test was byte-identical to
    d7eef06b.
  - 0c wires the remaining six runners.
  - 0c's 79c1ae5b, fast-forwarded onto integration: run_msg_send_local and
    run_msg_family_handoff link the L2 runtime units. Their archives now
    include l1src and lm1/build, l2units_build.ps1 is dot-sourced, and the
    unit objects are added per optimization level. e2 agreed the five module
    gates stay unwired.
    - Measured by 0c on 79c1ae5b: run_gates.ps1 -FamilyHandoff GREEN 11 of 11
      (family handoff 62/4).
    - nm shows lmx_sched_record_new once in every runtime link: port_message
      reference and parity, the five scenario36 selftests, run_lmx
      Message's six executables, send_local and family_handoff.
      send_local's count was 0 before this commit.
- Supervision handoff committed as 5f97128b on d6/exec-3b: lm1, lm2,
  exec.c, exec.h, lmx_message.h and the selftest (+368 -20). 87 methods
  (85 plus msg_child_chain_remove and msg_handoff_supervision).
  - msg_child_chain_remove is now the one internal detach, shared by
    child_unlink and the handoff. The 3b-8 bound refusal stays on
    child_unlink's public path only, so 3c-2b's swap of the two exec.c
    entries does not touch it (e2's note).
  - Red-first, against run_port_message:
    - Context record left on p: red, "handoff move st=0 ctx=1/1 map=2/1
      pn=0 qn=1 parent=2 create_id=0 path=2/2".
    - ANY membership not restored on q: red, "handoff move st=0 ctx=2/1
      map=0/0 ...".
    - c->parent not moved: red, "handoff move ... parent=1 ...".
    - The same mutation with the direct c->parent assertion removed from a
      scratch copy fails first on "handoff liveness closing=1 turn=0
      q_inbox=0 p_inbox=1": poll closed c through the stale address, and
      c's live_query went to p. Files restored, dirty=0.
  - run_gates.ps1 on 5f97128b: GREEN 10 of 10 (parity 87 methods; core
    tests 49/27/32/54/24; sched record 46/0; Message ok; history 65;
    stale 27; visit 148; liveness 97; sched_ready 20; send local 146).
  - e2 reviewed the lm2 hunk: it differs from lm1 only in lm2's closers.
    Approved.
  - Integration merge ee3af0bf, on top of c7efa936 and 564986e9.
    run_gates.ps1 on the merge: GREEN 10 of 10, the first run of the handoff
    with the L2 runtime units linked. Main b355365b.
  - Next: the decision 17 release chain. dispose_child and adopt_failed
    release the child's slot after the unlock; a parent's release cascades;
    complete() closes running children. The target is e2's six red lines.
- Release chain rulings (e2, 2026-09-14, from rule 1 as Mikhail stated it;
  a consequence Mikhail may overrule).
  - (a) corrected by e2 against spec 19.29.8 (the earlier "dispose drops a
    failed child's storage" reading is withdrawn). After final
    non-successful completion the child's arena is adopted into its direct
    parent without copying. Each closing Message settles only its direct
    children, bottom-up: G joins C, then C joins P. Successful histories are
    reclaimed by default.
    - dispose_child of a settled failed child neither refuses nor drops.
      It settles the child: adopt (blocks and ranges into the parent, the
      HISTORY root), then release the slot.
    - The old refusal goes as "the runtime adopts for you".
    - The selftest case "failure dispose must not drop history" becomes:
      dispose of a failed child adopts and the child is gone. Red first by
      reclaiming instead of adopting.
    - Open with e2: the cascade's two error branches, storage_can_move
      INVALID and history NOMEM. d6 proposes stopping and returning the
      status with nothing half-moved.
  - (b) dispose_child and adopt_failed become lm1/lm2 methods (89 unit
    methods). Each calls a C marker (today's body under the lock, as
    lmx_msg_exec_dispose_mark / _adopt_mark) and then release_slot after the
    unlock. The cascade lives in release_slot (children first, then unlink
    and release). Reason: run_port_message's -D redirect covers only the
    selftest and driver compiles, not exec.c, so a release_slot call from
    exec.c would never exercise the lm2 unit.
  - (c) The "every child disposed" precondition goes. The guards that
    stay: not running, handoff_ready, native_users == 0. Rule 3's close
    stays in end_turn's closing path.
  - Running descendants at release (follow-up step, after the settled chain):
    the cascade sets closing and an orphan mark. A successful orphan reclaims
    itself at its end-turn. A failed orphan keeps its handoff-safe arena
    under 19.29.8's orphan-retention timeout, then self-reclaims. e2 adds the
    red scenario to the release-17 test first.
- Release chain for settled subtrees committed as 5b05729f on d6/exec-3b. It
  folds in e2's b90fb3db and 67cd735d and moves run_msg_family_handoff's
  expected line to checks=61 watched_frees=4.
  - lmx_msg_settle_child (lm1, lm2) settles bottom-up:
    - it checks the direct-child guards under the lock before touching
      any grandchild;
    - a failed child with storage is adopted (lmx_msg_exec_adopt_mark);
    - any other child's storage is reclaimed
      (lmx_msg_exec_dispose_mark);
    - then release_slot.
  - dispose_child and adopt_failed are that settle, after one authority
    check (adopt keeps its refusals). A refused adopt inside the cascade
    returns its status with nothing half-moved: storage_can_move INVALID or
    history NOMEM, the first one reported.
  - Interim until the orphan step: when settle_child meets a still-running
    grandchild, first_settled_child skips it. The child is settled and
    released with that grandchild still linked, and try_retire keeps the
    child's slot until the grandchild stops. That window is what the orphan
    step closes (19.29.8's orphan retention; e2's fourth release-17
    scenario, red first).
  - Falsifier found on the way: the release-17 test was green without the
    cascade, because gone() is find by address and an unlinked branch is
    unreachable from rt->root whether or not its slots are retained. e2's
    67cd735d adds the slot count (rt\n) as the oracle. The executor
    selftest carries its own settle-branch case with the same oracle.
  - Red-first, first failing line each:
    - no cascade, release-17: 30 checks, 2 failures, on both slot-count
      lines;
    - no cascade, run_port_message: "settle branch st=0 n=3 n0=3
      adopted=1";
    - settle without release, release-17: 30 checks, 11 failures;
    - reclaim instead of adopt, run_port_message: "settle branch st=0
      n=1 n0=3 adopted=0".
  - Two chain runs were stopped before committing: first on the green
    no_cascade, then on a commit message naming a red line the rerun no
    longer measured. The commit was made from the verified applied state.
  - Integration merge 80e9342a. Main 0bf1e35d.
- Decision 18 (Mikhail, 2026-09-14): one arena, one lane, one writer. d6's
  audit of every write into a Message other than the writer's own lane went
  to e2. The ownership rule the acceptance enforces is that every write to
  a Message-owned cell happens on the lane of the cell's owner, where the
  lane is the current-turn Message (so run_child_turn on the host is the
  child's lane).
  - (1) A Message's own cells: its arena, scheduler record and cursor,
    ready-flag clear, mailbox except admission, native_users,
    handoff_ready, success, and its own running clear.
  - (2) The parent owns the supervision cells about its direct children:
    committed, tracked, child_heard_at, mapped, the family chain, the bind
    mapping (turn, turn_ctx, exec_bind) and the settle writes.
  - (3) Control flags with a designated cross-lane writer: running=0 and
    closing by the parent, ready=1 by the sender at admission.
  - (4) Primitives: mailbox admission, refs, bind_wait_signal.
  - (5) Runtime-level lists under the exec lock belong to no Message: the
    root list, slot list, retire queue, e->scan and the lane queue.
  - Removed in d6's first decision 18 commit, after the release chain:
    - A: pushes into the parent's sched_ready and map sets from the
      sender, the child or the catch-up;
    - B: the UI take writing the parents' sets and raising or lowering
      them;
    - C: a child unlinking itself from the parent's set;
    - D: the parent's ctx list written at bind and unbind.
    They are replaced by the child's ready flag, which the parent's step
    reads, and a walk of the parent's direct children.
  - e2's TEST oracle checks the current-turn Message against each cell's
    owner at every write site, red first at admit_one readying a child on
    the sender's lane.
- Integration merges after the release chain, all with scenario 4 of the
  release-17 test (9db57e84, e2's 5d9aa3bc) red first until the orphan step:
  - 6c0223a9 = fable/exec-3a at 12342ae3.
    - 45c56133 adds e2's lane-write oracle: lmx_msg_test_lane_write (TEST
      builds), armed by run_port_message -LaneCheck. It is hooked at
      exec_ready's sched_enqueue_child, with identical three lines in lm1 and
      lm2 (rule a), and at map_ready_enqueue_kind.
    - 12342ae3 retires the 3c-1 rings. lmx_sched_record is now the cursor
      and policy cells, written by the owner's step on its own lane.
    - Gates with -FamilyHandoff -FamilyRelease17: 11 PASS (sched record
      12/0, family handoff 61/4), family_release_17 red at "37 checks, 2
      failures".
    - Falsifier: -LaneCheck exits 1 with "LANE WRITE FAIL
      site=map_ready_enqueue:any owner=1 turn=2".
  - c32b84a0 = 0c's claude-0c/family-handoff-default at 1c6a5692 (local
    branch; run_gates.ps1 only).
    - family_handoff is the eleventh default gate and -FamilyHandoff is
      deleted.
    - -FamilyRelease17 stays opt-in until the orphan step.
    - run_gates -FamilyRelease17: 11 default gates PASS, family_release_17
      red at 37/2, "gates RED: stopped at family_release_17 after 298s".
  - Orphan step, design agreed with e2 (spec 19.29.6 (iii), model section
    32):
    - first_settled_child selects handoff-ready children.
    - A child still running when its parent is settled is re-rooted at the
      runtime as an orphan: parent_msg 0, parent 0U, create_id 0, orphan 1,
      appended to rt->root, its record in its own context list. The released
      parent retires at once.
    - The host finishes the orphan's end-turn after the turn leaves run_one:
      at run_child_turn's tail (lmx_msg_orphan_end) and in lmx_msg_drive
      (lmx_msg_orphan_sweep).
      - A successful orphan reclaims itself (lmx_msg_reclaim_orphan:
        settle, re-root, reclaim_mark, release_slot).
      - A failed one gets orphan_until = now + rt->orphan_retain
        (LMX_MSG_ORPHAN_RETAIN 30000 by default, lmx_msg_set_orphan_retain)
        and is reclaimed by the sweep once it expires.
    - Not end_turn or run_one's body: run_one keeps using the record,
      native_users and handoff_ready after the turn, and release_slot's
      exec_unbind joins reaps when get_tls is 0, which on a context worker
      is its own thread.
    - Decision 18: the orphan attach's ANY/UI re-raise and exec_ready from
      the releaser's lane are class A until readiness is the orphan's own
      flag, so -LaneCheck may be red there until then.
  - Orphan step committed as e09bc3f4 on d6/exec-3b, and e2 approved its lm2
    hunk (rule a). Three changes from the design were accepted by e2:
    - there is no orphan list;
    - retention is a per-runtime policy (rt->orphan_retain,
      LMX_MSG_ORPHAN_RETAIN 30000, lmx_msg_set_orphan_retain);
    - there is no retire trigger, because a re-rooted child leaves the
      parent's first_child empty.
  - A failed orphan's deadline is assigned at its end-turn on the host path:
    lmx_msg_orphan_end at run_child_turn's tail sets orphan_until. The sweep
    assigns it only to a mapped orphan whose turn ended on its own context.
  - Red-first on the applied tree, the first failing line each:
    - (a) settled test on running_load: 37 checks, 2 failures, on "R
      releases P4 at once";
    - (b) no re-rooting: 37 checks, 3 failures, on "C4's own algorithm
      completes";
    - (c) no orphan_end at run_child_turn's tail: 37 checks, 1 failure, on
      the slot-count line;
    - (d) no sweep in lm2's msg_drive: parity run 1 exit 1 with "orphan
      mapped reclaim n=2 n0=3 find_c=1".
  - Unmutated: release-17 37/0 and run_port_message PASS with 97 methods.
    Gates on e09bc3f4: "gates GREEN: 12 of 12".
  - e2's 84313e15 (fable/t17-sc5, on e09bc3f4) adds scenario 5, a failed
    orphan:
    - C5 runs and never completes;
    - R releases P5 and C5 is re-rooted;
    - retain is 100 on the logical clock;
    - C5's host-run closing turn at 5000 sets the deadline to 5100;
    - drives at 5000 and 5050 keep C5, and the drive at 5100 reclaims it
      (rt->n 1).
    Release-17 is 46/0. Red first by a sweep that never expires: 46 checks,
    1 failure. The test proves the host path and the reclaim by drive.
  - Integration merge 7cab28f8: integration fast-forwarded to 0c's f6a084e8
    (run_gates -LaneCheck switch), then merged fable/t17-sc5 at 84313e15.
    Gated with run_gates -FamilyRelease17: gates GREEN with release-17
    46/0. -FamilyRelease17 joins the default set next (0c's ticket).
- fbee9c77 (0c, a fast-forward): lmx_model_family_release_17_selftest is
  scenario36's sixth default test and -FamilyRelease17 is deleted. Gates
  GREEN, 11 of 11, with release-17 at 46/0.
- Decision 18 executor commit 0e57b685 (d6/exec-3b), design agreed with e2
  (q1-q4), lm2 hunk approved by e2 (rule a). Classes A-D are removed.
  - LmxMsg.ready is the Message's own flag (class 3).
    - lmx_msg_exec_ready sets it: the sender at admission, the closing
      requester, the bind kick.
    - The lane taking the Message's turn clears it: take_this,
      run_child_turn, the UI take.
  - The parent's step reads its direct children's flags after
    LmxMsg.sched_cursor, the parent's own cell (class 1), and wraps once:
    - on its own turn through lmx_msg_sched_pick (lm1 and lm2);
    - from the host outside any turn through sched_pick_host_child,
      which is the oracle's pass rule.
  - The cursor moves into lmx_sched_record once 0c routes the remaining
    runners through l2units_build.
  - The UI take walks the tree after e->ui_cursor (class 5) until 3d's
    mailbox.
  - Deleted: map_ready and ui_map_ready with the UI raise/lower, the lane
    scan, sched_enqueue/dequeue/unlink_child, the ctx lists (replaced by a
    tree walk over each Message's own exec_bind), the supervision and
    orphan attach helpers, and test_list_owner/test_take_owners.
  - try_retire gates on first_child alone.
  - sched_ready's LmxMsg fields and unit stay, unused, until 0c deletes the
    unit, its runners and gate.
  - Red-first, five mutations, each red:
    - the UI take without its cursor: "exec ui fifo second-turn first=2
      second=0";
    - take_this keeping the flag: "exec map-ready done ... nready=2
      map=2";
    - the host step without its cursor: "exec sched-cursor host order
      a=1,2,2,2 b=0,0,1,2";
    - lm2 exec_ready setting no flag, and lm2 pick without its cursor:
      parity run 1 exit 1.
  - Unmutated: parity 99 methods; wt3b gates GREEN, 11 of 11.
  - Integration merge 165c6d59.
- e2's lane oracle 67f0d319 (fable/d18-oracle), hooked at the surviving
  writes:
  - the cursor, with the owner the parent;
  - the ready clears, with the owner the Message.
  It adds the settled-owner and taking-lane clauses.
  - Red-first on the branch by two scratch mutations, each LANE WRITE FAIL
    exit 3: a child's turn writing its parent's cursor, and a child's turn
    clearing a sibling's flag.
  - Integration merge 0f395ba2, gated with run_gates -LaneCheck. -LaneCheck
    becomes a default gate next (0c).
- 5e (mixa lane): ticketed at Mikhail's request to finish parser-in-L2
  Stage c (p0_dump_alloc) and bring the Stage d plan to e2 before code.
  I merge sonnet/parser-l2 once run_port_parser is green on the merge.
- main 3cddf466: integration 40d94c61 merged onto e2's d7bf3bfd, through a
  temporary worktree. main's own commits were documents only. The first try
  pushed nothing because main moved during the merge.
- 4dd0c2af = fable/exec-3a at 900fd223, gated with run_gates -LaneCheck:
  GREEN 11 of 11.
  - e2's 3d acceptance test tests/lmx_model_ui_lane_3d_selftest.lm1
    (opt-in) is 22 checks, 2 failures on the interim take, on the
    readiness-order lines.
  - run_port_message's Mask covers "ctx_spawn_race st=N": 0c's flake,
    where the case's bind races the stop right after its release and the
    case accepts either status.
- Stage 3d commit 44ae8904 (d6/exec-3b). Design agreed with e2 (q1-q3);
  e2 approved the lm2 hunk and read the C part with no findings.
  - The UI lane is e->ui_lane: a Message-shaped mailbox owner created at
    the first request, outside rt->root and rt->n until stage 5.
  - exec_ready (lm1 and lm2) is the writer of a UI child's readiness. When
    ui_pending is 0 it sets it and admits one KIND_MAP request into the
    lane's inbox (class 4).
  - take_ui_locked drains the inbox in admission order:
    - it clears ui_pending (the taking lane);
    - it drops a request whose child is gone, not UI, held, launching or
      not ready;
    - it clears a stale flag;
    - it takes the first eligible child.
  - The interim ui_cursor and tree walk are gone. The failed UI->ANY
    launch re-requests.
  - Red-first:
    - no send: 3d test 22 checks, 12 failures;
    - an unrequested non-UI address: the same, so it degenerated into
      "no send";
    - LIFO inbox: 22 checks, 6 failures, on "the lane serves B first";
    - no dedupe in lm2: parity red, "exec ui fifo second-turn".
  - Unmutated: parity 99 methods with and without -LaneCheck;
    scenario36 six tests; 3d test 22/0. wt3b gates -LaneCheck GREEN, 11 of
    11.
  - Integration merge 6e522d75, gated with run_gates -LaneCheck and the 3d test
    (22/0). 0c promotes the 3d test into the defaults and rebases the
    -LaneCheck-default commit on it.
- 96f2a6bb, a fast-forward of 0c's claude-0c/lanecheck-default:
  - e9467a76 adds lmx_model_ui_lane_3d_selftest as scenario36's seventh
    default test;
  - 96f2a6bb folds the lane oracle into the port_message gate and deletes
    -LaneCheck.
  0c measured that exact commit twice with run_gates: GREEN 11 of 11,
  laneCheck=True, seven scenario36 tests. It was not re-gated here.
- Model section 34 audit (e2's request). Each gap mutation was measured on a
  scratch commit with the full run_gates.
  - (1) The storage move is link-only (no copy), blocks, ranges and roots
    move together, and the adopted child closes (rule 5). Pinned by the
    family handoff test's block, range, root and rule-5 labels.
  - (2) settle_child marks before release_slot. Pinned by "G to C": a
    release above the mark makes adopt_mark refuse.
  - (3) The successful history is released and the moved roots are
    exactly HISTORY. Pinned by K's labels and "P's HISTORY roots ...
    nothing else".
    - Making the moved history eternal turns scenario36 red, on "E is
      unaffected by B's failure" (49 checks, 1 failure).
    - My earlier claim that no gate caught it was wrong: it was read, not
      run.
  - (4) No forwarding to ancestors. Pinned by "the history is P's; P's
    parent did not inherit it".
  - (5) GAP: a failed orphan whose turn ends on its own context gets its
    deadline only from the sweep. Deleting that assignment in lm1 and lm2
    leaves every gate GREEN (11 of 11, 311s). e2 writes the acceptance
    from it.
  - Side note: if adopt_mark's init-block push fails after the storage move
    and the history commit, it returns INVALID with the storage already
    the parent's; no gate injects that failure.
- Merge 929b7bae = 5e's sonnet/parser-l2 at 8ea073f5: parser in L2, Stages a-c
  (text, scan, allocation, dump printer). run_port_parser.ps1 on the merge
  prints "run_port_parser ok", every stage 36/36 through both drivers.
  5e writes the Stage d plan for e2 before code.
- Merge c4806fbb = 0c's claude-0c/runner-routing at 1a410b54. It touches
  runners and RUNTIME_L2_PORTS only:
  - 18 run_port_* runners, run_graph_abi and run_l2trans link the L2 runtime
    units through l2units_build.ps1;
  - run_sched_record is the named exception.
  0c measured 1a410b54: run_gates GREEN 11 of 11, 18/18 run_port_* PASS. It
  was pushed ungated here; the section 34 merges below gate the result.
- Section 34 merges, gated once on 07cecca4:
  - 5f3ec6d0 = fable/exec-3a at 14c44ee6: e2's acceptance for gap (5),
    tests/lmx_model_orphan_mapped_17_selftest.lm1, plus eternal checks in
    the family handoff test. The conflict in RUNTIME_L2_PORTS.txt was
    notes only, resolved by keeping both sections.
  - 07cecca4 = d6/exec-3b at 326f227f: adopt_mark's init-block push failure
    branch deleted (the side note above). e2 ruled it dead:
    lmx_msg_blocks_push refuses only a malformed node, and the block is
    fresh and detached. Decision 12.
  - run_gates on 07cecca4: GREEN 11 of 11 in 308s; family handoff
    checks=63 failures=0; sched_record 12/0.
  - Falsifier: deleting the retention clause in lm1 makes the orphan
    mapped 17 test 17 checks, 2 failures, on "within the retention window
    the orphan is retained". lm1 was restored afterwards.
  - 0c promotes that test as scenario36's eighth default test on its
    rebased sched_ready retirement branch.
- Order after 3b-7a (e2, option iii): 3b-8, then 3b-7b, 3b-7c, 3b-7d, then
  e2's C half of 3c-2.
  - Reason: 3b-7b walks the family trees from rt->root, and release_slot
    today takes a bound Message out of its tree (child_unlink, and root-list
    removal for a parentless Message, lm1 1320-1337) before it unbinds
    (1356), with the lock dropped around the call (lm1 1429). A tree walk
    could miss a record the table still holds.
  - Rejected alternatives: a list of out-of-family bound Messages, or a
    walk of rt->slots. Both rebuild the table.
  - 3b-8 then checks its derived owner against the table while the table
    still exists.
  - e2's lean for 3b-8's contract: lmx_msg_child_unlink refuses a bound
    child. It is a `sub:` today (lm1 601, lm2), so that is a signature
    change in lm1+lm2 and the C prototype. The three owner-retire cases
    unbind before they unlink; any that cannot be rewritten goes to e2.
