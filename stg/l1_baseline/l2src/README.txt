l2src — L2 kernel on L1, plus the first L2→L1 frontend
======================================================

Layout
------
  units 1–7     runtime helpers written in L1 (range/pool/chars/ref/branch/own)
  run_lmx.ps1   L1→C selftests of those units (not L2→L1)
  l2trans.lm1   L2 frontend: P0 parse, subset check, emit .lm1
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
not. Several no-throw methods in one .lm2 are real §21.8
callables of one unit Structure: intern table shared across the
unit, child-index paths, typed `l2_m{i}(node, ...)`, if/else, direct
calls including forward references (L1 prototype:). Backend method
symbols are `l2_m{i}`; intern and source lookup keep original names.
`c.malloc` stays the foreign door. OwnUsed and DynRequired stay
empty (declared formals only). rec.sig is the interned full
L2_SIG_V0 + ordered source-name/type contract. Runtime does not
memcmp. Closed-unit devirtualization; actuals are temps before
the call. Recursion is an implementation limit. At most 8 methods;
`lmx_ranges_init` is 1+N. Formals are hygienic `l2_p{i}_{j}`.
char/int/size_t and `const @(char)` formals, arity 1–4, int or
size_t result. Pure &&/|| stay L1 C expressions. Calls inside
&&/|| use guarded if/temps and yield int 0/1, not an operand.
Non-logical size_t results stay size_t. Actuals once
left-to-right, checkpoint after actuals immediately before the
executed call; skipped RHS does not run nested actuals.
Dest replace writes tmp, then a unique dest.bak / dest.bak.N so
an existing foreign bak is kept; rollback failure reports the
backup path and does not claim dest was restored. Not atomic.
`char: quote` in a method body is an OwnUsed graph field of
`callable->node` (SPEC 21.5–21.6): typed C cache, all_chars cell
in the graph, dirty-only publish before calls and return, no
reload. Explicit `node\quote` writes the graph child, not the
cache. parser_text_predicates.lm2, parser_text_line_break.lm2
and parser_text_starts_python.lm2 are partial ports, not
replacements of l1src. `main` remains the §1.7 adapter. Not L2
self-build. Single emitter source: l2trans.lm1.

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
