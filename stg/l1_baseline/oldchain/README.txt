oldchain — frozen L2 bootstrap prerequisites for the gen0 seed
=============================================================

Why this exists
---------------
tests\l1\run_seed.ps1 links the gen0 seed C (produced by build\lm0\trans.lm0.exe
from lm2\l1trans.lm2, i.e. L2-shaped) against build\lm0\libparser.lm0.a and
build\lm0\libown.lm0.a. That seed C calls lm_message_thread_* .

Commit ecf010e ("Port staged L1 self-build") repointed buildCore.lm0.bat at the
new L1-profile lm1\build\parser.lm1.c / own.lm1.c, which by design carry no
MessageThread -- but kept writing the SAME two archive names. Running the current
buildCore.lm0.bat therefore overwrites the seed prerequisite with archives that
do not define lm_message_thread_*, and run_seed dies at link:

    undefined reference to `lm_message_thread_new'   (and ~12 more)

Note: the current buildCore.lm0.bat builds those two archives and then links
nothing against them -- every exe on its link lines is built without any .a.
They are pure collateral output.

Contents
--------
lib\libparser.lm0.a   copied verbatim from repo build\tmp\ (2026-09-05 18:56)
lib\libown.lm0.a      same

Chosen because build\tmp\ is the copy closest in time to build\lm0\trans.lm0.exe
(2026-09-05 19:59), the stage-0 translator run_seed actually uses.
libown.lm0.a there defines 43 T lm_message_thread_* symbols.

Other copies found in the repo (do not use without re-checking):
  build\lm0\next\        2026-09-06 03:26  parser.o identical, own.o DIFFERENT
  build\lm0\next\check\  2026-09-06 03:05  same as next\
  build\lm0\            2026-09-07 23:40  L1-profile, no MessageThread -- broken for the seed

Not reconstructible from git
----------------------------
git show ecf010e^:lm1/build/parser.lm1.c does not compile: ~10 errors of the form
"'lm_own_delete' redeclared as different kind of symbol" (a prototype and a
typedef of the same name emitted together). So the last committed L2-profile
generated C cannot rebuild these archives; they are kept as binaries.

Restore procedure: copy lib\*.a over build\lm0\*.a after any buildCore.lm0.bat run.
Scope: local to stg\l1_baseline. Nothing above this directory is touched.
