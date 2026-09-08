l2src — L2 kernel on L1, plus the first L2→L1 frontend
======================================================

Layout
------
  units 1–7     runtime helpers written in L1 (range/pool/chars/ref/branch/own)
  run_lmx.ps1   L1→C selftests of those units (not L2→L1)
  l2trans.lm1   first L2 frontend: P0 parse, subset check, emit .lm1
  run_l2trans.ps1
  tests/entry_*.lm2

l2trans slice (20260908-113800)
-------------------------------
Supported L2 input, and nothing else:

    L2:                          # optional; .lm2 is already implicit L2
        fn: main () int
            return: N            # N = non-negative decimal, no leading zeros
        end: main
    end: L2

That is the SPEC 1.7 bootstrap `main` adapter, with c.puts removed.
It is not a general L2 method, not L2 self-build, and not units 1–7.

Emit (L1):

    external:
        fn: main () int
            return: N
        end: main
    end: external

Then existing gen2 l1trans → C → gcc → exe. Artifacts: build\l2trans\
(not build\lm0). Do not run native finalize on this shared baseline.

Rejected with a diagnostic (old output is not a success):
  unsupported body, incompatible entry signature, several main,
  missing main, parse error, non-.lm2 path.

Runner: powershell -File l2src\run_l2trans.ps1
Open points for this unit: OPEN_POINTS.txt Unit 8.
