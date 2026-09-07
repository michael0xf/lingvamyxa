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

* `gcc` reachable on `PATH`. New-generation runners (`run_gen` / `run_smoke` /
  `run_parser` / `run_expr` / `run_ifdef` / `run_define` / `build_l1`)
  invoke it with `-std=c99 -Wall -Wextra -Wpedantic
  -I .` and four hard guards: `-Werror=incompatible-pointer-types
  -Werror=discarded-qualifiers -Werror=implicit-function-declaration
  -Werror=implicit-int`. The seed runner uses the older hosted command
  (`-I lm1`, no those `-Werror=` flags) that produced the working gen0
  binary; hosted C may still warn (unused `extent_field`).
* `powershell` for the runner scripts.
* External old-chain artifacts (not built by these L1 scripts; not a
  source-only clean-machine bootstrap):
  * `build\lm0\trans.lm0.exe` — stage-0 L2 translator for the gen0 seed.
  * `build\lm0\libparser.lm0.a`, `build\lm0\libown.lm0.a` — linked into gen0.
  * `lm2\parser_abi.lm2` — `predef` of `lm2\l1trans.lm2`.
  * Default hosted registry profile (`run_seed`/`run_gen`/`run_parser`/`run_smoke`
    clear `LM_TRANS_REGISTRY`, `LM_P0_REGISTRY`, `LM_TRANS_REGISTRY_VIEW`, and
    `LM_P0_COMPARE_REGISTRY` for the whole hosted parse path, then restore): `lm2\core.lm2`, `lm2\primitive.lm2`,
    `lm2\convert.lm2`, `lm2\trans_registry.lm2`, `lm2\parser_registry.lm2`.
    View mode 0 (legacy). `convert_impl.lm2` is a convert-table impl path,
    not a bootstrap candidate of this profile.
  * `lm1\` — include directory used by the working seed `gcc` line.
  * `build\lm0\printTree.lm0.exe` — P0 oracle for `tests\l1\run_parser.ps1`.
* No Qt or CMake for this Windows checkpoint. New native generations
  do not link the L2 runtime; gen0 still uses the old bootstrap bridge.

## Commands (CWD = repo root)

Start of a new self-build:

```
powershell -NoProfile -ExecutionPolicy Bypass -File tests\l1\run_seed.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File tests\l1\run_gen.ps1
```

`run_seed.ps1` is the documented seed command. It refuses to treat an
existing `build\l1trans\gen0\l1trans.exe` as success. Output of this run
goes to a unique staging dir `build\obj\l1trans\gen0\seed-<stamp>` and
`build\l1trans\gen0\seed-<stamp>`:

```
build\lm0\trans.lm0.exe lm2\l1trans.lm2 build\obj\l1trans\gen0\seed-<stamp>\l1trans.c
gcc -std=c99 -Wall -Wextra -Wpedantic -I lm1 -o build\l1trans\gen0\seed-<stamp>\l1trans.exe ^
    build\obj\l1trans\gen0\seed-<stamp>\l1trans.c ^
    build\lm0\libparser.lm0.a build\lm0\libown.lm0.a
```

then translates/compiles/runs `tests\l1\integer_add.lm2` with that binary.
Only after those checks does it copy C and exe into the published gen0
names, as two separate copies (not an atomic pair). The previous published
gen0 is left in place until those checks pass. Missing `trans.lm0` / libs /
registry files / sources abort with the exact path.

`build\l1trans\logs\seed.log` records incoming and effective registry env,
CMD/EXIT of trans.lm0, seed gcc, integer_add translate/gcc/run, and SHA256
of: bridge exe/libs, gcc.exe, `lm2\l1trans.lm2`, `parser_abi.lm2`, the five
default registry files, integer_add source/C/exe, and the published seed
C/exe. That is the recorded set; it is not claimed as a complete machine
manifest. `port_parser.py` / `port_l1trans.py` are not invoked.

`run_gen.ps1` then requires that published seed (it does not build one
and does not fall back to a stale exe):

```
powershell -NoProfile -ExecutionPolicy Bypass -File tests\l1\run_gen.ps1
```

drives gen0 → gen1 → gen2 → gen3, requires `gen1 C == gen2 C == gen3 C`,
runs the L1 builder selftest, bootstraps gen3 tools, then re-runs the smoke
and parser suites with `L1_GEN=gen2`.

The suites can also be run alone against a chosen generation:

```
powershell -NoProfile -ExecutionPolicy Bypass -File tests\l1\run_smoke.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File tests\l1\run_parser.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File tests\l1\run_expr.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File tests\l1\run_ifdef.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File tests\l1\run_define.ps1
```

All default to `gen0`; set `L1_GEN` (for example `gen2`) to pick another.
`run_parser.ps1` compares this parser's `printTree` dumps byte-for-byte
against `printTree.lm0.exe` on 16 positive fixtures, then requires exit 1
from both on 5 malformed fixtures and exit 1 plus a `P0 parse error`
diagnostic on a missing file.

`run_expr.ps1` has 22 positive fixtures and one diagnostic case, verified on
gen0 and gen2; `run_gen.ps1` does not invoke it automatically. It checks
repeated C output, strict compilation and execution of nested calls,
index/field arguments, unary/binary operators, parentheses, both cast
spellings, prefix dereference, prefix/postfix `++`/`--` and their use in indices.
The bounded cast regression distinguishes 512 from the formerly misgrouped
result 384. With `*p == 4`, `expr_deref_mix.lm2` checks that
`add(\ @ \ p + 1, 1)` emits `add(*(&*(p)) + 1, 1)` and returns 6.
`expr_inc_arg.lm2` checks that `a + ++b` does not become `a++ + b`,
and that `take(- --i)` keeps its separate unary operators. The ported
`expr_arg_segments.lm2` retains the frozen nested-call results 22 and 21.

Five string fixtures cover triple/fence values in initializers, returns and
arguments, with independent byte-for-byte payload and final-NUL checks, plus
ordinary string/character regressions. Fences retain source provenance across
imports; quote runs and operator-like payload remain data. Raw `??/n` stays
bytes 63,63,47,110, and tab followed by `0` emits `\0110` without absorbing
the digit. Full ordinary-string recanonicalization and byte-blob declarations
are not claimed.

`run_ifdef.ps1` verifies four fixtures in 11 macro configurations on each
of gen0/gen2, plus eight malformed branch/end cases that must not publish C.
It covers `#if`/`#elif`/`#else`, nesting, quoted conditions, unit/function
contexts and matching `end:` targets. Undefined `FLAG` and `-DFLAG=0` select
default; `-DFLAG=1` selects the first branch. This suite is standalone, not
invoked by `run_gen.ps1`. Conditional-import policy is outside this checkpoint.

`run_define.ps1` is also standalone: four fixtures, seven compile/run
configurations and four malformed name/token/end cases per gen0/gen2.
It covers unit-level object-like macros, including inside `ifdef` branches:

```text
define: PRESENT
define: MSG "ok"
define: ADD `3 + 4`
`ifndef-default`: VALUE 7
```

An empty replacement stays empty; `ifndef-default` preserves a command-line
definition, including zero. String bytes/NUL, arithmetic, alias and character
values are checked at execution; `=` and `(void)` only as emitted tokens.
Function-like macros, hosted table commands and function-body placement are
outside this checkpoint. Results are in `logs/<gen>/define.log`.

## Where output goes

```
build\l1trans\gen0|gen1|gen2|gen3\   executables
build\obj\l1trans\gen<N>\            generated C and objects
build\l1trans\oracles\               printTree.lm0 reference dumps
build\l1trans\logs\                  seed.log, gen_accept.log, smoke.log,
                                     parser_accept.log, expr.log, ifdef.log, gcc_*.log,
                                     builder stdout/stderr, per-dump pt_*
```

Nothing here writes to `lm1\build`, `build\lm0`, or live tools.
Generation-specific suite logs are under `logs\<gen>\`.

## port_parser.py / port_l1trans.py

Manual one-shot migration utilities used while converting the frozen L2
sources into `L1:` form. They contain hard-coded absolute paths and are
**not** part of the native self-build. Do not run them for an ordinary
build; the `.lm2` files in this directory are the source of truth.

## Status

Working today, exercised by the runners above: the ported P0 parser, `own`,
`parser_text`, `printTree`, `make` and `l1trans` itself; a real
gen1 → gen2 → gen3 fixed point on identical generated C; the four strict C99
guards passing; gen2 compiler smoke and gen0/gen2 expression suites; and the
parser oracle comparison described above.

Not claimed: full coverage of the L1 contract in
`L1_language_and_translator_spec.txt`, the second Translator-L2, and the old
Qt/CMake `buildCore` gate — none of those are part of this checkpoint. A
clean-checkout build has not been verified. Custom `LM_P0_REGISTRY` and the
by-value `.` versus `->` question remain open. The Windows builder passes
both rounds with spaced seed/obj/bin paths, rejects truncated path joins,
and records builder stdout/stderr and stage exits. Non-Windows execution
of the build runner has not been verified.
