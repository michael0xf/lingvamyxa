setlocal

set "PROJECT_ROOT=%~dp0"
cd /d "%PROJECT_ROOT%" || exit /b 1

if not defined LM_CMAKE set "LM_CMAKE=C:\Qt\Tools\CMake_64\bin\cmake.exe"
if not defined LM_CC set "LM_CC=C:\Qt\Tools\mingw1310_64\bin\gcc.exe"

set "SOURCE=build\lm1\buildCore\buildCore.lm1.c"
set "OUTPUT=build\lm0\buildCore.lm0.exe"

if not exist "%SOURCE%" (
    echo buildCore.lm0.bat: source file not found: %SOURCE% 1>&2
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

"%LM_CMAKE%" -E make_directory build\lm0 || exit /b 1
"%LM_CC%" -std=c99 -Wall -Wextra -Wpedantic "%SOURCE%" -o "%OUTPUT%"
if errorlevel 1 exit /b 1

echo built %OUTPUT%
