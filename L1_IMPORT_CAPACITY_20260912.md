# L1 import storage: descriptor-based growing tables

The bootstrap limit came from 2b31a6cc (2026-09-06, lm2/l1trans.lm2), then
c1d1f05c copied it into l1src/l1trans.lm1. Two 16640-byte arrays and n >= 16
limited all imports, even shallow independent ones, to sixteen 1040-byte paths.
Neither numeric limit is a language model rule.

Both active and completed import tables now grow arrays of the existing
LmP0Text string descriptor: data and length. This is an array/string length,
not Structure.len (child count). Each entry copies only the actual path bytes
plus a terminating NUL for existing C file APIs. No fixed path-cell size or
fixed entry maximum remains in import storage. Initial allocation capacity is
only a growth seed. Machine size/count overflow and allocation failure remain
checked. Allocation failure leaves existing entries/count/capacity unchanged.

Active pop frees its path; the translation wrapper frees both tables after all
success/error paths. Lookup compares length and bytes. Import resolution,
canonicalization, header registration, duplicate handling and cycle detection
retain their existing behavior. Root and baseline sources receive the same
change without overwriting their unrelated differences.

## Import-storage checkpoint verification

Run from this checkout:

    C:/Python314/python.exe tests/l1/run_import_capacity.py --seed <pinned-l1trans.exe>

24 command checks passed, including generated C equality across candidate/self/
next translation, native compile/run with 17 and 65 imported files, repeated
import deduplication, 20 imported header units, ordinary/header cycles, missing
file and failed-output preservation. The descriptor allocator harness covers
an 8192-byte stored path, independent owned copies, pop/reset cleanup, initial
allocation failure and growth failure with no residue, followed by retry.
The exact seven-predef MP3 reproducer fails on the old seed and translates with
the candidate. The baseline mirror also builds and translates 65 imports.

Evidence (private, not a stable promotion):

    build/codex/l1-import-capacity/build/import_capacity/run_20260912_084844/

Candidate SHA256: 24A1B57B6C831C7B45630DF8CA61A7441376B4EB8EF946B1F08FC41957290B09
Stable seed SHA256: 65D5A5ED127CA1BAEBDD1D500A5B74CEEA63EC1985EAC52EDEF28EFEB261C936
Generated C SHA256: 4051C2702C530456BAC0FD523419BC1B6DF2A9B794835A4C7852A4BD3931AFE3

## Dynamic path-buffer checkpoint

All remaining 1040-byte path arrays in both translator sources are removed.
Import decoding/resolution uses owned LmP0Text string descriptors; header names,
unit identifiers, normalized paths and .tmp output names allocate checked sizes
from their input lengths. Current-directory retrieval grows its buffer; the
unit-root CLI argument is borrowed from argv for the synchronous invocation.
This is compiler string storage, not a Structure header or a reinterpretation
of L2 raw char * fields.

While moving normalization to allocated storage, review found the old dot/dot-dot
branch reading uninitialized output (including before its start). It now reads
source segments and removes completed output segments with checked bounds.
Calls consuming temporary output names are evaluated before finally cleanup,
as required by the pinned compiler's return lowering.

29 command checks passed, including the earlier storage/regression checks,
8192-byte descriptor/resolution and allocation-failure cleanup, normalization,
real imports exceeding 1200 characters followed by native compile/run, and
long header input/include/unit-root/output paths with failed-output preservation.
Generated C is identical across candidate/self/next translation. Both sources
build. All successful commands exited zero; expected negative cases exited one.

Evidence: build/codex/l1-import-capacity/build/import_capacity/run_20260912_085957/
Candidate SHA256: 189809FDDFAF4967A1CA2FF5DA987587134F4C219442AA7E923845EF1356B9F7
Generated C SHA256: 3DB9F17C3F16706F054EB0CF5D1AD63A9C5B4ACBDFA00DF2939847006A6A16CC
The manifest records source, runner, harness, headers, compiler and driver hashes.
The stable seed SHA256 remains 65D5A5ED127CA1BAEBDD1D500A5B74CEEA63EC1985EAC52EDEF28EFEB261C936.

On Windows, long fixtures use the extended path form. For long CLI arguments,
the test driver invokes the generated main with explicit argv to bypass MinGW
startup wildcard expansion of the '?' in that path form. The ordinary candidate
translates the long import fixture directly. This change removes compiler buffer
limits; it does not change CRT startup or promise different host file-API limits.

## Import depth checkpoint

Removed the two independent depth > 16 checks in import/header traversal.
Growing active-path storage already provides cycle detection at any reached
depth; there is no replacement language maximum. Native stack, allocation and
machine limits still apply. Other compiler fixed registries are separate work.

34 command checks passed. Added successful depth-17 and depth-65 import chains
(with native compile/run), a 33-level header chain, and a cycle reached after
more than sixteen imports which fails while preserving existing output.
The path, allocation and candidate/self/next checks above passed on this revision.

Evidence: build/codex/l1-import-capacity/build/import_capacity/run_20260912_090217/
Candidate SHA256: 920994CE06DC9648F7DE3EC5F751E4DE8A8D46E4588161B737E32089CD6B614A
Generated C SHA256: 1CD9431FC56502F3B96FE9AC775E8F3A9928FEB1F7F7E597F64C7D83E9F08CE6
Stable65D5 remains unchanged. The Windows startup qualification above still applies.

## Continuation

Codex continues integration review with Grok and Fable. Full native MP3
composition is Claude's integration work. Stable/bootstrap promotion and full
L2 self-hosting are not claimed by these test runs.
