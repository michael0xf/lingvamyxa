l1_baseline — frozen L1 slice, and a self-contained build root
==============================================================

Original snapshot note (2026-09-07 23:26) is kept as README.snapshot-origin.txt.
This file describes the state after 2026-09-08, when the tree was made
buildable on its own.

What this directory is
----------------------
A second build root. Every tests\l1\*.ps1 runner starts with

    Set-Location (Join-Path $PSScriptRoot "..\..")

and buildCore.lm0.bat uses PROJECT_ROOT=%~dp0. So the runners anchor all
relative paths at the directory two levels above themselves — here, at
stg\l1_baseline\ instead of the repo root.

The "l1src/..." strings inside the .lm1 sources (predef:, include:, and
lm_build_trans in buildCore.lm1 / build_l1.lm1) are therefore not a hard tie
to the repo root: they are build-root-relative and resolve to
stg\l1_baseline\l1src\. No source or runner was edited to achieve this.

Consequence: L1 development keeps building at the repo root against the live
l1src\, and L2 work builds here against the frozen slice. The two never share
build\l1trans, build\obj, build\lm0 or lm1\build.

Invariant this depends on
-------------------------
No absolute paths and no "..\" escaping the build root, anywhere in l1src\
or tests\l1\. Host-toolchain defaults are the one accepted exception:
l1src\buildCore.lm1 hardcodes C:/Qt/Tools/... for cmake/gcc/ar/ranlib. Those
are machine paths, not build-tree paths.

Layout
------
  l1src\            frozen L1 sources (unchanged since the snapshot)
  tests\l1\         frozen fixtures + the 13 run_*.ps1 runners
  tests\*.lmx       16 P0 fixtures run_parser.ps1 loads from the repo-root
                    tests\ (11 positive, 5 malformed). Added 2026-09-08;
                    without them run_gen dies with
                    "missing fixture tests\block_string.lmx".
  lm1\build\        the seven generated C files the buildCore chain compiles:
                    parser, own, l1trans, printTree, make, finalize, buildCore.
                    The five unused old-chain ones (trans, trans_library,
                    vcpkgFetch, rest_lmx_http_client, rest_lmx_http_server)
                    are deliberately not copied. The lm1\ directory must also
                    simply exist: run_seed.ps1 checks for it as the seed gcc
                    -I include dir.
  lm2\              copy of repo lm2\ — l1trans.lm2 seed source, parser_abi.lm2,
                    and the five default registry files
  buildCore.lm0.bat copy of the repo script (C bootstrap)
  buildCore.lm0.sh  copy
  oldchain\         frozen old-L2-chain seed prerequisites; see its README.txt
  build\            local outputs only: lm0\, l1trans\, obj\

Verified here (2026-09-08, gcc 13.1.0 MinGW, LM_THREAD_PROVIDER=single)
-----------------------------------------------------------------------
  buildCore.lm0.bat                                    exit 0
  tests\l1\run_seed.ps1                                exit 0  gen0 seed ok
  tests\l1\run_gen.ps1                                 exit 0
      gen1_c = gen2_c = gen3_c
      = 35F9726E54D7ED18C3ED0E5B179341CEDCBBD2A84D342CD4EA62C663475955E7
      build_l1 selftest 0, spaced-path route 0, gen3 bootstrap 0,
      gen2 smoke ok, gen2 parser accept ok
  gen0 standalone: expr ifdef define scalar decl_repeat ident c_array
                   control import_bare                all exit 0
  gen2 standalone: smoke parser expr ifdef define scalar decl_repeat ident
                   c_array control import_bare        all exit 0

Live tree isolation was checked after the run: no file newer than 23:45 under
repo build\l1trans, build\obj, build\lm0, lm1\build, l1src or tests.

One manual step after buildCore.lm0.bat
---------------------------------------
buildCore.lm0.bat overwrites build\lm0\libparser.lm0.a and libown.lm0.a with
L1-profile archives that carry no lm_message_thread_*, which the gen0 seed
needs. Restore them before run_seed:

    copy /Y oldchain\lib\libparser.lm0.a build\lm0\
    copy /Y oldchain\lib\libown.lm0.a    build\lm0\

See oldchain\README.txt. Reported upstream in
work_chat\cursor\inbox\20260907-235829.txt; cursor accepted fix (a) —
drop both ar/ranlib blocks from buildCore.lm0.bat and .sh, since nothing
links those archives any more — and queued it as TASK3 for grok_bot.

What git tracks here
--------------------
Same policy as the repo root: no binaries. See the .gitignore in this
directory. Tracked: l1src\, tests\, lm2\, the seven generated C files under
lm1\build\, the two buildCore scripts, the READMEs. Not tracked: build\
(everything under it is regenerated) and oldchain\lib\*.a.

So a fresh checkout of this directory does not build until the pinned
binaries are put back by hand:

    build\lm0\trans.lm0.exe, printTree.lm0.exe   from repo build\lm0\
    oldchain\lib\libparser.lm0.a, libown.lm0.a   from repo build\tmp\
    then copy oldchain\lib\*.a into build\lm0\

Promotion
---------
Agreed with cursor (work_chat\cursor\outbox\20260907-235829.txt): when an L1
slice is accepted, cursor writes "промоут: коммит <sha>" into that channel and
the copy into this directory is done from here. The self-contained set is:

    l1src\
    tests\l1\
    tests\*.lmx                                (the 16 listed above)
    lm1\build\*.lm1.c                          (the seven)
    lm2\
    build\lm0\trans.lm0.exe, printTree.lm0.exe (pinned, not rebuilt)
    oldchain\lib\*.a                           (old L2-profile, not the L1 ar)

Nothing in this directory writes above itself.
