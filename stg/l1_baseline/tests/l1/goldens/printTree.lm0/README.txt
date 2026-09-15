printTree.lm0 goldens

What: the tree dump of the lm2 chain's printTree.lm0.exe for each fixture that
tests\l1\run_parser.ps1 and tests\l1\run_legacy_p0.ps1 compare against, generated
once so those runners no longer need the old chain's binary.

Oracle: printTree.lm0.exe, 505210 bytes, sha256
8FA2A40396ED7A4DE1262D76DA3EA1244AF9F68F5E707D970155963E29D37717. It is the lm2
chain's printTree built by CMake from lm1/build/printTree.lm1.c generated from
lm2/printTree.lm2 (root CMakeLists.txt L607-616 and L753-759), copied from the main
checkout's build\lm0.

Layout: <key> is the fixture's repo-relative path with \ and / replaced by _.
  <key>.exit    the oracle's exit code, one line
  <key>.stdout  the oracle's stdout, raw bytes (".gitattributes: * -text" keeps them exact)
  <key>.p0      "code@line:col" from the oracle's "P0 parse error" diagnostic, non-zero exits only

Fixtures: run_parser's 16 positive and 5 malformed fixtures, and every entry of
tests\l1\legacy_p0_manifest.txt, deduplicated: 134 files (103 exit 0, 31 exit 1).

Command, from the repository root, per fixture <src>:
  cmd /c "printTree.lm0.exe <src> > <key>.stdout 2> <stderr>"
  exit code -> <key>.exit; "P0 parse error C at L:COL" in <stderr> -> <key>.p0

Generated 2026-09-15 (0c, seed re-base, 6f's ruling). A disagreement between a
runner and these files is a question, never a silent regeneration.
