l2src — L2 kernel on L1, plus the first L2→L1 frontend
======================================================

Layout
------
  units 1–7     runtime helpers written in L1 (range/pool/chars/ref/branch/own)
  lmx_size.lm1  mutable size_t primitive pool (own size_t fields)
  l2_text_hash.lm1  FNV-1a 64 + L2ImmutQuery fill/compare seam
  l2_immut_query.h  borrowed immutable-query adapter (not p0.h)
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
the call. Recursion is an implementation limit. A function closer may be
`end:<name>`, `end:fn`/`end:sub`, a `return:` trailer, or a
source-level cut (`---`). Nested `end: return` on a return
statement is still rejected. Method tables and
OwnUsed tables grow to the source count. Address slots stay at most
4 per method. `lmx_ranges_init` is 1+N. Formals are hygienic `l2_p{i}_{j}`.
char/int/size_t and `const @(char)` formals, declared arity grown to the
source count, int,
size_t, `@: char`, `@: LmP0Text`, `LmP0TrailerRole` (p0.lm1.h typedef int), or void (`sub:`) result.
At most 4 address slots `l2_s{i}_{j}` per method. Pure &&/|| stay L1 C expressions. Calls inside
&&/|| use guarded if/temps and yield int 0/1, not an operand.
Non-logical size_t results stay size_t. Actuals once
left-to-right, checkpoint after actuals immediately before the
executed call; skipped RHS does not run nested actuals.
Dest replace writes tmp, then a unique dest.bak / dest.bak.N so
an existing foreign bak is kept; rollback failure reports the
backup path and does not claim dest was restored. Not atomic.
`char: quote` / `size_t: value_length` in a method body is an
OwnUsed graph field of `callable->node` (SPEC 21.5–21.6): typed
C cache; char graph data is an all_chars cell; size_t graph data
is a unique LMX_TYPE_SIZE_T cell mutated in place (no interned
shared size_t, no int narrowing). Dirty-only publish before
calls and return, no reload. Explicit `node\name` writes the
graph child, not the cache. An explicit char/size_t argument
with the same name as an own field shares that parameter from
the own-decl line (not a second l2_q; no graph load over the
incoming value). Without that bind the argument is never
published. Own-decl inside if is rejected. Hidden through
arguments are not yet lowered. `const @(LmP0Text)` / `@: LmP0Text`
are the existing C ABI from generated l1src/p0.lm1.h (data, length), a limited
migration adapter: include and typed schema only, not a general
C header parser and not a silent C layout for new L2 Structures.
Lmx access stays child-index. Known C: c.strlen, c.memcmp,
l2_immut_query_fill, l2_hash_compare_q, c.sizeof(c.LmP0Text),
lm_own_new_zero, lm_own_copy_bytes, lm_own_delete; unknown c.*
is rejected. Stores through const LmP0Text* are
`const write`. parser_text_views.lm2 ports lm_p0_text_equals and
lm_p0_identifier_payload. `lm_p0_text_equals` is the mutable
fallback: strlen + length + memcmp, no extra FNV. Prepared
hashes use explicit borrowed `L2ImmutQuery` (l2src header, not
p0.lm1.h, not an Lmx Structure): `lm_p0_immut_query_make` fills
data/length/FNV-1a 64 once; `lm_p0_text_equals_query` hashes
current text bytes against that hash then memcmp. Backing of
the query must stay alive and unchanged; a C literal satisfies
that, a const formal does not prove it. Failed fill leaves
live=0; compare does not read garbage. identifier_payload
keeps aliasing. No process-global bind table.
Body `@: LmP0Text result` is an explicit 11.2.1 address-depth-1
slot (C `LmP0Text *`, symbol `l2_s`), not OwnUsed, not an Lmx
child, and not the lmx_own cache. `return: result` is the pointer
VALUE; `return: @result` would be the address of the slot
(dangling) and is not emitted. `result\data` / `result\length`
are raw foreign follow of the frozen p0.h ABI. `lm_own_*` comes
from frozen `l1src/own.lm1` (new_zero / copy_bytes / delete /
alloc_fails), imported as-is; that is heap allocation, not
lmx_own field cache. `c.sizeof` admits `LmP0Text` /
`c.LmP0Text` only. parser_text_heap.lm2 ports
lm_p0_copy_bytes, lm_p0_text_view_new_cstr,
lm_p0_text_view_delete (sub), and lm_p0_text_from_cstr.
Checkpoint still runs before call/exit when OwnUsed is empty.
parser_text_predicates.lm2,
parser_text_line_break.lm2, parser_text_starts_python.lm2,
parser_text_views.lm2, parser_text_heap.lm2 and
parser_fence_line.lm2 (block-string `=` / raw-comment `*`
line matchers; not Document, not dash-fence status) are partial
ports, not replacements of l1src. `main` remains the §1.7
adapter. Not L2 self-build.
Single emitter source: l2trans.lm1. Not the whole parser.

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
Hidden through-args (DynRequired) are the next bounded patch,
not implemented: OPEN_POINTS.txt Unit 12. Open points for this
unit: OPEN_POINTS.txt Units 8–12.
