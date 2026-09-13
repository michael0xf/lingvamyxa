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
