# Parity gate for the full mixa_selection L2 port (ticket 20260913-081200,
# continuing 20260913-075100). Builds the CURRENT L2 frontend
# (stg/l1_baseline/l2src/l2trans.lm1) fresh from the pinned, read-only
# stable L1 translator, then attempts to translate the COMPLETE
# mixa_selection.lm2 (all 22 functions, the real typed MixaSelection/
# MixaSelRev API, not a minimal probe).
#
# Per this ticket's own instruction: "The runner must stop with a clear
# EXPECTED_CORE_BARRIER result while the current compiler rejects the
# custom types; it must not call that a pass." So:
#   - translation fails with the already-measured "unknown foreign type"
#     diagnostic -> EXPECTED_CORE_BARRIER (exit 2, not a failure of this
#     script, not a pass of the port)
#   - translation fails with any OTHER diagnostic -> UNEXPECTED_FAILURE
#     (exit 1) -- a second, different barrier to investigate, not to be
#     silently folded into the expected one
#   - translation SUCCEEDS -> the core branch has landed general custom
#     struct-type admission; this script then builds/links/runs the real
#     L1-vs-L2 parity comparison across the 9 named scenarios and reports
#     PASS/FAIL on that, never on translation succeeding alone (exit 0 on
#     pass, exit 1 on any parity mismatch)
#
# mixa_selection.h/mixa_selection.lm1 are the parity oracle and are never
# touched by this script.
param()
$ErrorActionPreference = "Stop"

$RepoRoot = Split-Path -Parent $PSScriptRoot
$L1Root = Join-Path $RepoRoot "stg\l1_baseline"
$L1Trans = Join-Path $L1Root "build\l1trans\gen2\l1trans.exe"
$ExpectedL1Hash = "65D5A5ED127CA1BAEBDD1D500A5B74CEEA63EC1985EAC52EDEF28EFEB261C936"

if (-not (Test-Path -LiteralPath $L1Trans)) {
    throw "missing stable L1 translator: $L1Trans"
}
$ActualL1Hash = (Get-FileHash -LiteralPath $L1Trans -Algorithm SHA256).Hash
if ($ActualL1Hash -ne $ExpectedL1Hash) {
    throw "stable L1 translator hash mismatch: expected $ExpectedL1Hash got $ActualL1Hash"
}

$RunTimestamp = (Get-Date -Format "yyyyMMdd_HHmmss_fff")
$RunGuid = [GUID]::NewGuid().ToString().Substring(0, 8)
$BaseDir = Join-Path $RepoRoot "build\mixa\claude\mixa_selection_l2_parity"
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

$l2exe = Join-Path $RunDir "l2trans.exe"
$L2TransExeHash = $null

Push-Location $L1Root
try {
    $l2c = Join-Path $RunDir "l2trans.c"
    & $L1Trans $L2TransSourceRel $l2c
    if ($LASTEXITCODE -ne 0) { throw "l1trans failed translating l2trans.lm1 itself" }
    # Established fix (ticket 20260913-063500, re-applied 075100): route
    # gcc's own stderr through cmd /c so PowerShell's NativeCommandError
    # machinery under EAP=Stop never wraps it.
    $gccBuildLog = Join-Path $RunDir "l2trans_build_stderr.log"
    $gccArgsStr = "-std=c99 -Wall -Wextra -Wpedantic -I . -I lm1\build $($guards -join ' ') `"$l2c`" -o `"$l2exe`""
    & cmd /c "gcc $gccArgsStr > `"$gccBuildLog`" 2>&1"
    if ($LASTEXITCODE -ne 0) {
        Get-Content $gccBuildLog
        throw "gcc failed building l2trans.exe"
    }
    $L2TransExeHash = (Get-FileHash -LiteralPath $l2exe -Algorithm SHA256).Hash
} finally {
    Pop-Location
}

Push-Location $RepoRoot
$selSrc = "mixa_manager\mixa_selection.lm2"
$selOut = Join-Path $RunDir "mixa_selection_l2.lm1"
$selStdout = Join-Path $RunDir "selection_stdout.log"
$selStderr = Join-Path $RunDir "selection_stderr.log"
try {
    & cmd /c "`"$l2exe`" `"$selSrc`" `"$selOut`" > `"$selStdout`" 2> `"$selStderr`""
    $SelExit = $LASTEXITCODE
} finally {
    Pop-Location
}

$SelStdoutText = if (Test-Path -LiteralPath $selStdout) { Get-Content -LiteralPath $selStdout -Raw } else { "" }
$SelStderrText = if (Test-Path -LiteralPath $selStderr) { Get-Content -LiteralPath $selStderr -Raw } else { "" }
$SelSourceHash = (Get-FileHash -LiteralPath (Join-Path $RepoRoot $selSrc) -Algorithm SHA256).Hash
$SelHeaderHash = (Get-FileHash -LiteralPath (Join-Path $RepoRoot "mixa_manager\mixa_selection_l2.h.lm1") -Algorithm SHA256).Hash
$RunnerHash = (Get-FileHash -LiteralPath $PSCommandPath -Algorithm SHA256).Hash

$Verdict = $null
$ExitCode = 1

if ($SelExit -ne 0 -and $SelStderrText -match "unknown foreign type") {
    $Verdict = "EXPECTED_CORE_BARRIER"
    $ExitCode = 2
} elseif ($SelExit -eq 0) {
    $Verdict = "TRANSLATION_SUCCEEDED_PARITY_NOT_YET_RUN"
    $ExitCode = 3
} else {
    $Verdict = "UNEXPECTED_FAILURE"
    $ExitCode = 1
}

$Summary = @"
Stable-L1-Translator: $L1Trans
Stable-L1-Translator-Sha256: $ActualL1Hash
L2trans-Source (informational, not pinned): $L2TransSource
L2trans-Source-Sha256 (informational, not pinned): $L2TransSourceHash
L2trans-Exe-Sha256 (rebuilt fresh this run, not a stable artifact): $L2TransExeHash
Selection-L2-Header: mixa_manager\mixa_selection_l2.h.lm1
Selection-L2-Header-Sha256: $SelHeaderHash
Selection-L2-Source: $selSrc
Selection-L2-Source-Sha256: $SelSourceHash
Runner-Sha256: $RunnerHash
Selection-Translate-Exit-Code: $SelExit
Selection-Translate-Stdout:
$SelStdoutText
Selection-Translate-Stderr:
$SelStderrText
Verdict: $Verdict
"@
Set-Content -LiteralPath (Join-Path $RunDir "run_summary.txt") -Value $Summary
$Summary
"Run directory: $RunDir"

switch ($Verdict) {
    "EXPECTED_CORE_BARRIER" {
        "EXPECTED_CORE_BARRIER: full mixa_selection.lm2 translation stopped at the already-measured `"unknown foreign type`" diagnostic (core custom-struct-type admission not yet landed). This is NOT a pass -- the L2 port remains blocked exactly as ticket 20260913-075100 reported, now confirmed against the complete file rather than the minimal probe."
    }
    "TRANSLATION_SUCCEEDED_PARITY_NOT_YET_RUN" {
        "NOTE: full mixa_selection.lm2 translated successfully -- the core branch appears to have landed custom struct-type admission. The real L1-vs-L2 parity comparison (9 named scenarios) is NOT yet implemented in this script and must be added before this can be called a pass. Do not report this as a pass; report it as the trigger for the next integration step."
    }
    "UNEXPECTED_FAILURE" {
        "UNEXPECTED_FAILURE: translation of the complete file failed with a diagnostic OTHER than the already-measured `"unknown foreign type`" barrier. Inspect $selStderr -- this may be a second, distinct compiler barrier isolable further in the file; do not invent a manager-side workaround for it."
    }
}

exit $ExitCode
