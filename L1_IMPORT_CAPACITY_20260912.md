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

## Verification

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

## Remaining implementation work

This checkpoint fixes import storage. Historical 1040-byte temporary buffers
in path decoding/resolution/header naming, the separate depth > 16 import guard,
and other compiler fixed registries are still present. They are implementation
limitations, not new language rules. The next Codex stage audits/removes the
path-buffer limits using actual-length array/string storage. Full native MP3
composition is Claude's integration work. The whole-toolchain stable/bootstrap
promotion and full L2 self-hosting milestone are not claimed by this test run.
