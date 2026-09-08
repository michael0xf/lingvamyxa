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
            c.puts: "..."        # optional, zero or more, quoted source spelling
            return: N            # N = 0..INT_MAX of the C99 target int
                                 # (no leading zeros; overflow is a diagnostic)
        end: main
    end: L2

That is the SPEC 1.7 bootstrap `main` adapter. String atoms are copied as
P0 source spelling into L1 (quotes and escapes kept; not decoded then
re-escaped). `c.puts` sequences are allowed; anything after `return` is
not. It is not a general L2 method, not L2 self-build, and not units 1–7.

Emit (L1), with `include: "<stdio.h>"` only when there is at least one puts:

    include: "<stdio.h>"
    external:
        fn: main () int
            c.puts: "..."
            return: N
        end: main
    end: external

Then existing gen2 l1trans → C → gcc → exe. Artifacts: build\l2trans\
(not build\lm0). Do not run native finalize on this shared baseline.

Rejected with a diagnostic (nonzero l2trans, no later stages, destination
unchanged). Planted negative files named .exe are text markers, not a
real executable and not a run of an old binary:
  unsupported body, incompatible entry signature, several main,
  missing main, parse error, non-.lm2 path,
  return literal not representable as int (e.g. 2147483648 on 32-bit int).
INT_MAX (2147483647 here) is admitted and emitted; OS exit status is
not the oracle for that value.

Runner: powershell -File l2src\run_l2trans.ps1
Open points for this unit: OPEN_POINTS.txt Unit 8.
