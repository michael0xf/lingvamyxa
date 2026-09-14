# Narrow, manager-owned evidence gate for the SECOND L2 compiler barrier
# found while completing ticket 20260913-081200 (mixa_selection full
# .lm2 translation attempt): a bare `@@:` (pointer-to-pointer) RETURN
# type on an ordinary (non-main) leaf function is unconditionally
# rejected by l2trans.lm1's own general function-signature checker
# (l2_collect_decls, around line 5103: only a bare int/size_t/
# LmP0TrailerRole atom or a single `@:` pointer frame is recognized as a
# return type there; `@@:` has no branch at all and falls through to
# "incompatible entry signature"). This is a DIFFERENT diagnostic and a
# DIFFERENT code path from the already-known "unknown foreign type"
# struct-type barrier (mixa_selection_l2_port.txt, ticket
# 20260913-075100) -- confirmed by reading l2trans.lm1 directly, not
# assumed. Note `@@:` IS accepted as a PARAMETER type (l2_typed_formal,
# types 17-20 for char/void/int/size_t) -- only the RETURN position is
# affected.
#
# Builds l2trans.exe fresh from the pinned, read-only stable L1
# translator and the CURRENT l2trans.lm1 (read-only, untouched),
# matching every other runner in this ticket family. Translates
# mixa_entrysig_probe.lm2 (one function, no custom struct type anywhere,
# `@@: char` return only) and records the exact diagnostic.
param()
$ErrorActionPreference = "Stop"

$RepoRoot = Split-Path -Parent $PSScriptRoot
$L1Root = Join-Path $RepoRoot "stg\l1_baseline"
$L1Trans = Join-Path $L1Root "build\l1trans\gen2\l1trans.exe"
. (Join-Path $PSScriptRoot "lib_l2_runtime_support.ps1")
$ExpectedL1Hash = Get-L1Pin -L1Root $L1Root

$ActualL1Hash = Assert-PinnedL1Translator -L1Trans $L1Trans -L1Root $L1Root

$RunTimestamp = (Get-Date -Format "yyyyMMdd_HHmmss_fff")
$RunGuid = [GUID]::NewGuid().ToString().Substring(0, 8)
$BaseDir = Join-Path $RepoRoot "build\mixa\claude\mixa_entrysig_probe"
$RunDir = Join-Path $BaseDir "run_${RunTimestamp}_${RunGuid}"
New-Item -ItemType Directory -Force -Path $RunDir | Out-Null

$L2TransSourceRel = "l2src\l2trans.lm1"
$L2TransSource = Join-Path $L1Root $L2TransSourceRel
if (-not (Test-Path -LiteralPath $L2TransSource)) {
    throw "missing L2 frontend source (read-only, not modified by this script): $L2TransSource"
}
$L2TransSourceHash = (Get-FileHash -LiteralPath $L2TransSource -Algorithm SHA256).Hash

$guards = @(
    "-Werror=incompatible-pointer-types", "-Werror=discarded-qualifiers",
    "-Werror=implicit-function-declaration", "-Werror=implicit-int"
)

Push-Location $L1Root
try {
    $l2c = Join-Path $RunDir "l2trans.c"
    $l2exe = Join-Path $RunDir "l2trans.exe"
    & $L1Trans $L2TransSourceRel $l2c
    if ($LASTEXITCODE -ne 0) { throw "l1trans failed translating l2trans.lm1 itself" }
    $gccBuildLog = Join-Path $RunDir "l2trans_build_stderr.log"
    $gccArgsStr = "-std=c99 -Wall -Wextra -Wpedantic -I . -I lm1\build $($guards -join ' ') `"$l2c`" -o `"$l2exe`""
    & cmd /c "gcc $gccArgsStr > `"$gccBuildLog`" 2>&1"
    if ($LASTEXITCODE -ne 0) {
        Get-Content $gccBuildLog
        throw "gcc failed building l2trans.exe"
    }
    $L2TransExeHash = (Get-FileHash -LiteralPath $l2exe -Algorithm SHA256).Hash

    Pop-Location
    Push-Location $RepoRoot
    $probeSrc = "mixa_manager\mixa_entrysig_probe.lm2"
    $probeOut = Join-Path $RunDir "mixa_entrysig_probe.lm1"
    $probeStdout = Join-Path $RunDir "probe_stdout.log"
    $probeStderr = Join-Path $RunDir "probe_stderr.log"
    & cmd /c "`"$l2exe`" `"$probeSrc`" `"$probeOut`" > `"$probeStdout`" 2> `"$probeStderr`""
    $ProbeExit = $LASTEXITCODE
} finally {
    Pop-Location
}

$ProbeStdoutText = if (Test-Path -LiteralPath $probeStdout) { Get-Content -LiteralPath $probeStdout -Raw } else { "" }
$ProbeStderrText = if (Test-Path -LiteralPath $probeStderr) { Get-Content -LiteralPath $probeStderr -Raw } else { "" }
$ProbeSourceHash = (Get-FileHash -LiteralPath (Join-Path $RepoRoot $probeSrc) -Algorithm SHA256).Hash
$RunnerHash = (Get-FileHash -LiteralPath $PSCommandPath -Algorithm SHA256).Hash

$Summary = @"
Stable-L1-Translator: $L1Trans
Stable-L1-Translator-Sha256: $ActualL1Hash
L2trans-Source (informational, not pinned): $L2TransSource
L2trans-Source-Sha256 (informational, not pinned): $L2TransSourceHash
L2trans-Exe-Sha256 (rebuilt fresh this run, not a stable artifact): $L2TransExeHash
Probe-Source: $probeSrc
Probe-Source-Sha256: $ProbeSourceHash
Runner-Sha256: $RunnerHash
Probe-Exit-Code: $ProbeExit
Probe-Stdout:
$ProbeStdoutText
Probe-Stderr:
$ProbeStderrText
"@
Set-Content -LiteralPath (Join-Path $RunDir "run_summary.txt") -Value $Summary
$Summary
"Run directory: $RunDir"

if ($ProbeExit -eq 0) {
    "NOTE: the probe translated successfully -- this barrier may no longer be present in the current l2trans.lm1."
} elseif ($ProbeStderrText -match "incompatible entry signature") {
    "Reproduced the expected barrier: L2 frontend rejects a bare @@: (pointer-to-pointer) return type on an ordinary leaf function (`"incompatible entry signature`")."
} else {
    "Probe failed, but NOT with the expected `"incompatible entry signature`" diagnostic -- inspect probe_stderr.log."
}
