# l1src — L1-profiled sources for Translator-L1

These are the `L1:` sources the new Translator-L1 compiles. They live here,
not in `lm2/`, so that building them can never clobber the old chain's
tracked `lm1/build/*.lm1.c`. Everything under `build/l1trans/` is generated
and gitignored.

Truth is the files in this directory and the logs under `build/l1trans/`.
Reports in `work_chat/` describe past runs; they are not instructions.

## What is here

Ported from frozen `lingvamyxa_old_worked_version` @ `620db86`, converted to
the `L1:` profile with no L2 tables, no MessageThread and no registry load:

| file | origin |
| --- | --- |
| `parser.lm2` | frozen `lm2/parser.lm2` — P0 parser |
| `parser_text.lm2` | frozen `lm2/parser_text.lm2` |
| `own.lm2` | frozen `lm2/own.lm2`, reduced to malloc + ptr stack + arena |
| `p0.h` | C99 layouts and constants for the above |
| `printTree.lm2` | thin P0 client: parse → dump → free |
| `make.lm2` | L1-profiled build driver |
| `libc_abi.lm2`, `add.lm2` | declaration prelude and import example |

Written for L1 (not ported):

| file | role |
| --- | --- |
| `l1trans.lm2` | the translator itself, in L1 |
| `build_l1.lm2` | L1 builder: generate, compile, second pass |

`lm2/l1trans.lm2` is a separate, L2-shaped copy kept only to produce the
gen0 seed with the old stage-0 `trans.lm0`. It is not the L1 source.

## Prerequisites

* `gcc` reachable on `PATH`. The runners invoke it bare, with
  `-std=c99 -Wall -Wextra -Wpedantic -I .` and four hard guards:
  `-Werror=incompatible-pointer-types -Werror=discarded-qualifiers
  -Werror=implicit-function-declaration -Werror=implicit-int`.
* `powershell` for the three runner scripts.
* Pre-existing artifacts from the old chain, already in the working tree:
  * `build\lm0\printTree.lm0.exe` — the P0 oracle. `tests\l1\run_parser.ps1`
    aborts without it.
  * `build\lm0\trans.lm0.exe` — used once, outside these scripts, to make
    the gen0 seed.
* No Qt or CMake for this Windows checkpoint. New native generations
  do not link the L2 runtime; gen0 still uses the old bootstrap bridge.

## Commands (CWD = repo root)

The gen0 seed is **not** produced by any script in the tree.
`tests\l1\run_gen.ps1` requires `build\l1trans\gen0\l1trans.exe` to exist and
throws `missing seed` otherwise. That seed is built once from the L2-shaped
`lm2\l1trans.lm2` with the old `build\lm0\trans.lm0.exe` plus `gcc`;
`build\l1trans\logs\build_gen0.log` records only the source and exe hashes of
that step, not its command line.

With the seed present:

```
powershell -NoProfile -ExecutionPolicy Bypass -File tests\l1\run_gen.ps1
```

drives gen0 → gen1 → gen2 → gen3, requires `gen1 C == gen2 C == gen3 C`,
runs the L1 builder selftest, bootstraps gen3 tools, then re-runs the smoke
and parser suites with `L1_GEN=gen2`.

The two suites can also be run alone against a chosen generation:

```
powershell -NoProfile -ExecutionPolicy Bypass -File tests\l1\run_smoke.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File tests\l1\run_parser.ps1
```

Both default to `gen0`; set `L1_GEN` (for example `gen2`) to pick another.
`run_parser.ps1` compares this parser's `printTree` dumps byte-for-byte
against `printTree.lm0.exe` on 16 positive fixtures, then requires exit 1
from both on 5 malformed fixtures and exit 1 plus a `P0 parse error`
diagnostic on a missing file.

## Where output goes

```
build\l1trans\gen0|gen1|gen2|gen3\   executables
build\obj\l1trans\gen<N>\            generated C and objects
build\l1trans\oracles\               printTree.lm0 reference dumps
build\l1trans\logs\                  gen_accept.log, smoke.log,
                                     parser_accept.log, gcc_*.log,
                                     per-dump pt_l1_* / pt_lm0_*
```

Nothing here writes to `lm1\build`, `build\lm0`, or live tools.

## port_parser.py / port_l1trans.py

Manual one-shot migration utilities used while converting the frozen L2
sources into `L1:` form. They contain hard-coded absolute paths and are
**not** part of the native self-build. Do not run them for an ordinary
build; the `.lm2` files in this directory are the source of truth.

## Status

Working today, exercised by the runners above: the ported P0 parser, `own`,
`parser_text`, `printTree`, `make` and `l1trans` itself; a real
gen1 → gen2 → gen3 fixed point on identical generated C; the four strict C99
guards passing; the gen2 compiler smoke suite; and the parser oracle
comparison described above.

Not claimed: full coverage of the L1 contract in
`L1_language_and_translator_spec.txt`, the second Translator-L2, and the old
Qt/CMake `buildCore` gate — none of those are part of this checkpoint. A
clean-checkout build has not been verified. Custom `LM_P0_REGISTRY` and the
by-value `.` versus `->` question remain open. The Windows builder passes
both rounds with spaced seed/obj/bin paths, rejects truncated path joins,
and records builder stdout/stderr and stage exits. Non-Windows execution
of the build runner has not been verified.
