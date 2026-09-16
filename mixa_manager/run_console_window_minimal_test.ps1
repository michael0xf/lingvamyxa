$ErrorActionPreference = "Stop"
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$workDir = Split-Path -Parent $scriptDir

$trans = Join-Path $workDir "stg\l1_baseline\build\l1trans\gen2\l1trans.exe"
$compilerSha = "65D5A5ED127CA1BAEBDD1D500A5B74CEEA63EC1985EAC52EDEF28EFEB261C936"

if (-not (Test-Path -LiteralPath $trans)) { exit 1 }
if ((Get-FileHash -LiteralPath $trans -Algorithm SHA256).Hash -ne $compilerSha) { exit 1 }

$timestamp = (Get-Date -Format "yyyyMMdd_HHmmss_fff")
$guid = [System.Guid]::NewGuid().ToString().Substring(0, 8)
$baseDir = Join-Path (Join-Path (Join-Path (Join-Path $workDir "build") "mixa") "opencode") "console_window"
New-Item -ItemType Directory -Force -Path $baseDir | Out-Null

$runDir = Join-Path $baseDir "run_minimal_${timestamp}_${guid}"
$logsDir = Join-Path $runDir "logs"
$headerDir = Join-Path (Join-Path $runDir "headers") "mixa_manager"
New-Item -ItemType Directory -Force -Path $logsDir | Out-Null
New-Item -ItemType Directory -Force -Path $headerDir | Out-Null

# Create fixture
$fixtureBytes = @(
    0x46, 0x49, 0x58, 0x54, 0x55, 0x52, 0x45, 0x30,
    0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38,
    0x41, 0x42, 0x43, 0x44, 0x00, 0x0D, 0x0A, 0xFF
)
$fixturePath = Join-Path $runDir "mixa_console_window_fixture.bin"
[System.IO.File]::WriteAllBytes($fixturePath, [byte[]]$fixtureBytes)

# Translate
Push-Location $workDir

# Header
& $trans "mixa_manager\mixa_console_window.h.lm1" (Join-Path $headerDir "mixa_console_window.lm1.h") 2>&1 | Out-Null
if ($LASTEXITCODE -ne 0) { Pop-Location; exit 1 }

# Implementation (via predef, not linked separately)
# File API (link separately)
$fileC = Join-Path $runDir "mixa_file_win32.c"
& $trans "mixa_manager\mixa_file_win32.lm1" $fileC 2>&1 | Out-Null
if ($LASTEXITCODE -ne 0) { Pop-Location; exit 1 }

# Test (predefs console module, links file impl)
$testC = Join-Path $runDir "test.c"
& $trans "mixa_manager\tests\mixa_console_window_minimal_test.lm1" $testC 2>&1 | Out-Null
if ($LASTEXITCODE -ne 0) { Pop-Location; exit 1 }

Pop-Location

# Compile: test + file impl (no console .c since predef'd in test)
$exe = Join-Path $runDir "test.exe"
Push-Location $workDir
& gcc -std=c99 -Wall -Wextra -Wpedantic -Werror=incompatible-pointer-types -I . -I (Join-Path $runDir "headers") $testC $fileC -o $exe 2>&1 | Tee-Object (Join-Path $logsDir "compile.log") | Out-Null
$compileExit = $LASTEXITCODE
Pop-Location

if ($compileExit -ne 0) {
    Write-Output "Compile failed: exit $compileExit"
    Get-Content (Join-Path $logsDir "compile.log") | Select-Object -Last 20
    exit 1
}

# Run
& $exe $fixturePath 2>&1 | Tee-Object (Join-Path $logsDir "output.log")
$testExit = $LASTEXITCODE

Write-Output ""
Write-Output "Run: $runDir"
Write-Output "Exit: $testExit"
exit $testExit
