l1_baseline snapshot
====================

Date (local): 2026-09-07 23:26:21
Purpose: Frozen self-build baseline of L1 sources + tests/l1 runners.
         Development continues in root l1src\; this tree is for later
         baseline-vs-dev comparison only. Do not treat as live build input.

What was copied
---------------
- l1src\  — full copy of repo-root l1src\, excluding __pycache__
  (robocopy /E /XD __pycache__). Includes .lm1 sources, p0.h,
  port_*.py (manual migration utils; not part of native self-build),
  and l1src\README.md.
- tests\l1\ — full copy of repo-root tests\l1\ (fixtures + run_*.ps1
  runners and subdirs bare_a, bare_b, bare_cycle, "bare space").

What was NOT copied (conscious omit)
------------------------------------
- build\ artifacts (build\l1trans\, build\lm0\, build\obj\, oracles,
  logs, published gen0–gen3 binaries). Per task default: omit build
  artifacts; self-build regenerates them via seed → gen.
- lm2\, lm1\, root l1src (live), stg\lm2-prev, stg\implementsABC —
  untouched; not part of this snapshot.
- __pycache__ under l1src.

Verification gate commands (per l1src\README.md; CWD = repo root)
-----------------------------------------------------------------
Self-build start:
  powershell -NoProfile -ExecutionPolicy Bypass -File tests\l1\run_seed.ps1
  powershell -NoProfile -ExecutionPolicy Bypass -File tests\l1\run_gen.ps1

Core suites (also driven by run_gen for smoke/parser with L1_GEN=gen2;
run alone against a chosen gen via L1_GEN, default gen0):
  powershell -NoProfile -ExecutionPolicy Bypass -File tests\l1\run_smoke.ps1
  powershell -NoProfile -ExecutionPolicy Bypass -File tests\l1\run_parser.ps1

Additional standalone suites documented in README (not automatic gate of
run_gen): run_expr, run_ifdef, run_define, run_scalar, run_decl_repeat,
run_ident, run_c_array; also present: run_control, run_import_bare.

Prerequisites (external; not snapshotted): gcc on PATH; build\lm0\
trans.lm0.exe + libparser/libown; lm2\ registry/predef files; lm1\
includes; build\lm0\printTree.lm0.exe for parser oracle — see
l1src\README.md Prerequisites.

Layout under this snapshot
--------------------------
  stg\l1_baseline\l1src\
  stg\l1_baseline\tests\l1\
  stg\l1_baseline\README.txt   (this file)