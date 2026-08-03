@echo off
setlocal

set "PROJECT_ROOT=%~dp0"
cd /d "%PROJECT_ROOT%" || exit /b 1

if not defined LM_CMAKE set "LM_CMAKE=C:\Qt\Tools\CMake_64\bin\cmake.exe"
if not defined LM_CC set "LM_CC=C:\Qt\Tools\mingw1310_64\bin\gcc.exe"
if not defined LM_AR set "LM_AR=C:\Qt\Tools\mingw1310_64\bin\ar.exe"
if not defined LM_RANLIB set "LM_RANLIB=C:\Qt\Tools\mingw1310_64\bin\ranlib.exe"
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
    set "THREAD_COMPILE_FLAGS=-DLM_THREAD_PROVIDER=LM_THREAD_PROVIDER_PTHREAD -pthread"
    set "THREAD_LINK_FLAGS=-DLM_THREAD_PROVIDER=LM_THREAD_PROVIDER_PTHREAD -pthread"
) else if "%THREAD_PROVIDER%"=="win32" (
    set "THREAD_COMPILE_FLAGS=-DLM_THREAD_PROVIDER=LM_THREAD_PROVIDER_WIN32"
    set "THREAD_LINK_FLAGS=-DLM_THREAD_PROVIDER=LM_THREAD_PROVIDER_WIN32"
) else (
    set "THREAD_COMPILE_FLAGS=-DLM_THREAD_PROVIDER=LM_THREAD_PROVIDER_SINGLE"
    set "THREAD_LINK_FLAGS=-DLM_THREAD_PROVIDER=LM_THREAD_PROVIDER_SINGLE"
)

set "PARSER_SOURCE=lm1\build\parser.lm1.c"
set "OWN_SOURCE=lm1\build\own.lm1.c"
set "TRANS_SOURCE=lm1\build\trans.lm1.c"
set "MAKE_SOURCE=lm1\build\make.lm1.c"
set "FINALIZE_SOURCE=lm1\build\finalize.lm1.c"
set "BUILD_CORE_SOURCE=lm1\build\buildCore.lm1.c"

if not exist "%PARSER_SOURCE%" (
    echo buildCore.lm0.bat: source file not found: %PARSER_SOURCE% 1>&2
    exit /b 1
)

if not exist "%OWN_SOURCE%" (
    echo buildCore.lm0.bat: source file not found: %OWN_SOURCE% 1>&2
    exit /b 1
)

if not exist "%TRANS_SOURCE%" (
    echo buildCore.lm0.bat: source file not found: %TRANS_SOURCE% 1>&2
    exit /b 1
)

if not exist "%MAKE_SOURCE%" (
    echo buildCore.lm0.bat: source file not found: %MAKE_SOURCE% 1>&2
    exit /b 1
)

if not exist "%FINALIZE_SOURCE%" (
    echo buildCore.lm0.bat: source file not found: %FINALIZE_SOURCE% 1>&2
    exit /b 1
)

if not exist "%BUILD_CORE_SOURCE%" (
    echo buildCore.lm0.bat: source file not found: %BUILD_CORE_SOURCE% 1>&2
    exit /b 1
)

if not exist "%LM_CMAKE%" (
    echo buildCore.lm0.bat: cmake not found: %LM_CMAKE% 1>&2
    echo Set LM_CMAKE to the cmake.exe path and retry. 1>&2
    exit /b 1
)

if not exist "%LM_CC%" (
    echo buildCore.lm0.bat: C compiler not found: %LM_CC% 1>&2
    echo Set LM_CC to the gcc.exe path and retry. 1>&2
    exit /b 1
)

if not exist "%LM_AR%" (
    echo buildCore.lm0.bat: ar not found: %LM_AR% 1>&2
    echo Set LM_AR to the ar.exe path and retry. 1>&2
    exit /b 1
)

if not exist "%LM_RANLIB%" (
    echo buildCore.lm0.bat: ranlib not found: %LM_RANLIB% 1>&2
    echo Set LM_RANLIB to the ranlib.exe path and retry. 1>&2
    exit /b 1
)

"%LM_CMAKE%" -E make_directory build\lm0 || exit /b 1
"%LM_CMAKE%" -E make_directory build\obj || exit /b 1

"%LM_CC%" -std=c99 -Wall -Wextra -Wpedantic %THREAD_COMPILE_FLAGS% -Ilm1 -c "%PARSER_SOURCE%" -o build\obj\parser.lm1.o || exit /b 1
if exist build\lm0\libparser.lm0.a del /f /q build\lm0\libparser.lm0.a || exit /b 1
"%LM_AR%" rcs build\lm0\libparser.lm0.a build\obj\parser.lm1.o || exit /b 1
"%LM_RANLIB%" build\lm0\libparser.lm0.a || exit /b 1

"%LM_CC%" -std=c99 -Wall -Wextra -Wpedantic %THREAD_COMPILE_FLAGS% -Ilm1 -c "%OWN_SOURCE%" -o build\obj\own.lm1.o || exit /b 1
if exist build\lm0\libown.lm0.a del /f /q build\lm0\libown.lm0.a || exit /b 1
"%LM_AR%" rcs build\lm0\libown.lm0.a build\obj\own.lm1.o || exit /b 1
"%LM_RANLIB%" build\lm0\libown.lm0.a || exit /b 1

"%LM_CC%" -std=c99 -Wall -Wextra -Wpedantic %THREAD_LINK_FLAGS% -Ilm1 "%TRANS_SOURCE%" build\lm0\libparser.lm0.a build\lm0\libown.lm0.a -o build\lm0\trans.lm0.exe || exit /b 1
"%LM_CC%" -std=c99 -Wall -Wextra -Wpedantic %THREAD_LINK_FLAGS% "%MAKE_SOURCE%" build\lm0\libown.lm0.a -o build\lm0\make.lm0.exe || exit /b 1
"%LM_CC%" -std=c99 -Wall -Wextra -Wpedantic %THREAD_LINK_FLAGS% "%FINALIZE_SOURCE%" build\lm0\libown.lm0.a -o build\lm0\finalize.lm0.exe || exit /b 1
"%LM_CC%" -std=c99 -Wall -Wextra -Wpedantic %THREAD_LINK_FLAGS% -Ilm1 "%BUILD_CORE_SOURCE%" build\lm0\libown.lm0.a -o build\lm0\buildCore.lm0.exe || exit /b 1

echo built build\lm0 bootstrap tools
