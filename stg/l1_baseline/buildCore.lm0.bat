@echo off
rem buildCore.lm0.bat for the stg\l1_baseline build root.
rem
rem Own script, not a copy of the repo-root one. The root script serves the live
rem L1 chain and changes with it; this root has different needs:
rem   - it needs no cmake;
rem   - it builds only the four tools this root actually uses.
rem
rem It needs nothing of the old L2 chain (Mikhail 2026-09-15: the previous binary
rem always exists, and the old chain's files go): no trans.lm0.exe, no
rem printTree.lm0.exe, no libparser/libown archives. Everything it builds comes
rem from this root's committed generated C in lm1\build. The gen0 seed is gcc on
rem that same committed l1trans.lm1.c (tests\l1\run_seed.ps1), and the parser
rem oracle is committed goldens under tests\l1, so nothing here restores or
rem checks an old binary.
rem
rem Anchors at its own directory, like every runner here.

setlocal
set "BUILD_ROOT=%~dp0"
cd /d "%BUILD_ROOT%" || exit /b 1

if not defined LM_CC set "LM_CC=C:\Qt\Tools\mingw1310_64\bin\gcc.exe"
if not defined LM_THREAD_PROVIDER set "LM_THREAD_PROVIDER=single"

set "THREAD_PROVIDER="
if /I "%LM_THREAD_PROVIDER%"=="auto" set "THREAD_PROVIDER=win32"
if /I "%LM_THREAD_PROVIDER%"=="pthread" set "THREAD_PROVIDER=pthread"
if /I "%LM_THREAD_PROVIDER%"=="win32" set "THREAD_PROVIDER=win32"
if /I "%LM_THREAD_PROVIDER%"=="single" set "THREAD_PROVIDER=single"

if not defined THREAD_PROVIDER (
    echo buildCore.lm0.bat: unsupported LM_THREAD_PROVIDER: %LM_THREAD_PROVIDER% 1>&2
    echo Expected one of: auto, pthread, win32, single. 1>&2
    exit /b 1
)

if "%THREAD_PROVIDER%"=="pthread" (
    set "THREAD_FLAGS=-DLM_THREAD_PROVIDER=LM_THREAD_PROVIDER_PTHREAD -pthread"
) else if "%THREAD_PROVIDER%"=="win32" (
    set "THREAD_FLAGS=-DLM_THREAD_PROVIDER=LM_THREAD_PROVIDER_WIN32"
) else (
    set "THREAD_FLAGS=-DLM_THREAD_PROVIDER=LM_THREAD_PROVIDER_SINGLE"
)

if not exist "%LM_CC%" (
    echo buildCore.lm0.bat: C compiler not found: %LM_CC% 1>&2
    echo Set LM_CC to the gcc.exe path and retry. 1>&2
    exit /b 1
)

for %%S in (l1trans make finalize buildCore) do (
    if not exist "lm1\build\%%S.lm1.c" (
        echo buildCore.lm0.bat: source file not found: lm1\build\%%S.lm1.c 1>&2
        exit /b 1
    )
)

if not exist build\lm0 mkdir build\lm0 || exit /b 1
if not exist build\obj mkdir build\obj || exit /b 1

set "CFLAGS=-std=c99 -Wall -Wextra -Wpedantic %THREAD_FLAGS%"

"%LM_CC%" %CFLAGS% -I. -I lm1/build "lm1\build\l1trans.lm1.c"   -o build\lm0\l1trans.lm0.exe   || exit /b 1
"%LM_CC%" %CFLAGS%     "lm1\build\make.lm1.c"      -o build\lm0\make.lm0.exe      || exit /b 1
"%LM_CC%" %CFLAGS%     "lm1\build\finalize.lm1.c"  -o build\lm0\finalize.lm0.exe  || exit /b 1
"%LM_CC%" %CFLAGS% -I. "lm1\build\buildCore.lm1.c" -o build\lm0\buildCore.lm0.exe || exit /b 1

echo built build\lm0 bootstrap tools from lm1\build
