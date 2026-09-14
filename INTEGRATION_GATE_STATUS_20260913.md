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
