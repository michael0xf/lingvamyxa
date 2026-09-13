# Real nine-scenario L1-vs-L2 parity gate for mixa_selection (ticket
# 20260913-083100, continuing 081200/075100). Builds the CURRENT L2
# frontend (stg/l1_baseline/l2src/l2trans.lm1) fresh from the pinned,
# read-only stable L1 translator, then:
#
#   1. ALWAYS builds and runs the ORACLE-side parity harness (the real
#      mixa_selection.lm1, linked with tests/mixa_selection_parity_
#      harness.lm1 and tests/mixa_selection_alloc_fault.c) -- this is
#      the "run all preparatory/oracle checks that are possible" half of
#      this ticket's own instruction, independent of whether L2
#      translation succeeds.
#   2. Attempts to translate the COMPLETE mixa_selection.lm2 (all 22
#      functions, the real typed API, not a minimal probe).
#      - fails with either already-known barrier diagnostic ("unknown
#        foreign type" -- no manager-owned struct type admission, or
#        "incompatible entry signature" -- no @@: return type on an
#        ordinary leaf function) -> EXPECTED_CORE_BARRIER (exit 2). NOT
#        a pass; oracle side still ran and is reported.
#      - fails with any OTHER diagnostic -> UNEXPECTED_FAILURE (exit 1),
#        a further barrier to investigate, never silently folded into
#        the expected ones.
#      - SUCCEEDS -> builds and runs the L2-side harness (the SAME
#        harness source, linked against the object l2trans.exe emitted
#        for mixa_selection.lm2 instead) and diffs its stdout against
#        the oracle run's stdout byte-for-byte. Identical output on both
#        + both exit 0 -> PASS (exit 0). Any difference or non-zero L2
#        run -> PARITY_FAILURE (exit 1).
#
# mixa_selection.h/mixa_selection.lm1 are the parity oracle and are
# never touched by this script. Nothing under stg/l1_baseline is
# modified, only read.
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

$guards = @(
    "-Werror=incompatible-pointer-types", "-Werror=discarded-qualifiers",
    "-Werror=implicit-function-declaration", "-Werror=implicit-int"
)
$GccStd = "-std=c99 -Wall -Wextra -Wpedantic $($guards -join ' ')"

$RunTimestamp = (Get-Date -Format "yyyyMMdd_HHmmss_fff")
$RunGuid = [GUID]::NewGuid().ToString().Substring(0, 8)
$BaseDir = Join-Path $RepoRoot "build\mixa\claude\mixa_selection_l2_parity"
$RunDir = Join-Path $BaseDir "run_${RunTimestamp}_${RunGuid}"
$HeaderDir = Join-Path $RunDir "headers\mixa_manager"
New-Item -ItemType Directory -Force -Path $HeaderDir | Out-Null

function Invoke-Cmd([string]$exe, [string]$argsStr, [string]$outLog, [string]$errLog) {
    # Established fix (ticket 20260913-063500, re-applied 075100/081200):
    # route through cmd /c so PowerShell's own NativeCommandError
    # machinery under EAP=Stop never wraps a native process's stderr.
    & cmd /c "$exe $argsStr > `"$outLog`" 2> `"$errLog`""
    return $LASTEXITCODE
}

# ---- Step 0: translate the shared header-unit once (used by both the
# oracle-side and L2-side harness compiles; field-identical to
# mixa_selection.h, so valid for either implementation object). ----
Push-Location $RepoRoot
$hdrLog1 = Join-Path $RunDir "header_trans_stdout.log"
$hdrLog2 = Join-Path $RunDir "header_trans_stderr.log"
$hdrOut = Join-Path $HeaderDir "mixa_selection_l2.lm1.h"
$hdrExit = Invoke-Cmd $L1Trans "mixa_manager\mixa_selection_l2.h.lm1 `"$hdrOut`"" $hdrLog1 $hdrLog2
Pop-Location
if ($hdrExit -ne 0) { throw "header translation failed: see $hdrLog2" }

# ---- Step 1: build l2trans.exe fresh. ----
$L2TransSourceRel = "l2src\l2trans.lm1"
$L2TransSource = Join-Path $L1Root $L2TransSourceRel
if (-not (Test-Path -LiteralPath $L2TransSource)) {
    throw "missing L2 frontend source (read-only, not modified by this script): $L2TransSource"
}
$L2TransSourceHash = (Get-FileHash -LiteralPath $L2TransSource -Algorithm SHA256).Hash

$l2exe = Join-Path $RunDir "l2trans.exe"
Push-Location $L1Root
try {
    $l2c = Join-Path $RunDir "l2trans.c"
    & $L1Trans $L2TransSourceRel $l2c
    if ($LASTEXITCODE -ne 0) { throw "l1trans failed translating l2trans.lm1 itself" }
    $gccBuildLog = Join-Path $RunDir "l2trans_build_stderr.log"
    $gccArgsStr = "$GccStd -I . -I lm1\build `"$l2c`" -o `"$l2exe`""
    $rc = Invoke-Cmd "gcc" $gccArgsStr (Join-Path $RunDir "l2trans_build_stdout.log") $gccBuildLog
    if ($rc -ne 0) { Get-Content $gccBuildLog; throw "gcc failed building l2trans.exe" }
} finally {
    Pop-Location
}
$L2TransExeHash = (Get-FileHash -LiteralPath $l2exe -Algorithm SHA256).Hash

# ---- Step 2: build the harness + alloc-fault objects (implementation-
# independent; used unmodified against whichever selection object is
# linked in). ----
Push-Location $RepoRoot
$harnessC = Join-Path $RunDir "harness.c"
$hLog1 = Join-Path $RunDir "harness_trans_stdout.log"
$hLog2 = Join-Path $RunDir "harness_trans_stderr.log"
$hExit = Invoke-Cmd $L1Trans "mixa_manager\tests\mixa_selection_parity_harness.lm1 `"$harnessC`"" $hLog1 $hLog2
Pop-Location
if ($hExit -ne 0) { Get-Content $hLog2; throw "harness translation failed" }

$harnessO = Join-Path $RunDir "harness.o"
$hcLog1 = Join-Path $RunDir "harness_compile_stdout.log"
$hcLog2 = Join-Path $RunDir "harness_compile_stderr.log"
$hcExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" -c `"$harnessC`" -o `"$harnessO`"" $hcLog1 $hcLog2
if ($hcExit -ne 0) { Get-Content $hcLog2; throw "harness compile failed" }

$faultO = Join-Path $RunDir "alloc_fault.o"
$faLog1 = Join-Path $RunDir "alloc_fault_compile_stdout.log"
$faLog2 = Join-Path $RunDir "alloc_fault_compile_stderr.log"
$faExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -c `"$RepoRoot\mixa_manager\tests\mixa_selection_alloc_fault.c`" -o `"$faultO`"" $faLog1 $faLog2
if ($faExit -ne 0) { Get-Content $faLog2; throw "alloc-fault helper compile failed" }

# ---- Step 3: ALWAYS build + run the ORACLE-side harness. ----
$oracleC = Join-Path $RunDir "mixa_selection_oracle.c"
$ocLog1 = Join-Path $RunDir "oracle_trans_stdout.log"
$ocLog2 = Join-Path $RunDir "oracle_trans_stderr.log"
Push-Location $RepoRoot
$ocExit = Invoke-Cmd $L1Trans "mixa_manager\mixa_selection.lm1 `"$oracleC`"" $ocLog1 $ocLog2
Pop-Location
if ($ocExit -ne 0) { Get-Content $ocLog2; throw "oracle mixa_selection.lm1 translation failed" }

$oracleO = Join-Path $RunDir "mixa_selection_oracle.o"
$occLog1 = Join-Path $RunDir "oracle_compile_stdout.log"
$occLog2 = Join-Path $RunDir "oracle_compile_stderr.log"
# -Dmalloc/-Dcalloc redirect ONLY this compiled object's own allocator
# calls to the counting wrappers (scenario 9); mixa_selection.lm1 itself
# is never edited -- only this test build's own compile command differs.
$occExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -Dmalloc=test_malloc -Dcalloc=test_calloc -c `"$oracleC`" -o `"$oracleO`"" $occLog1 $occLog2
if ($occExit -ne 0) { Get-Content $occLog2; throw "oracle mixa_selection.lm1 compile failed" }

$oracleExe = Join-Path $RunDir "parity_oracle.exe"
$olLog1 = Join-Path $RunDir "oracle_link_stdout.log"
$olLog2 = Join-Path $RunDir "oracle_link_stderr.log"
$olExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" `"$harnessO`" `"$oracleO`" `"$faultO`" -o `"$oracleExe`"" $olLog1 $olLog2
if ($olExit -ne 0) { Get-Content $olLog2; throw "oracle harness link failed" }

$oracleRunOut = Join-Path $RunDir "oracle_run_stdout.log"
$oracleRunErr = Join-Path $RunDir "oracle_run_stderr.log"
$oracleRunExit = Invoke-Cmd "`"$oracleExe`"" "" $oracleRunOut $oracleRunErr
$OracleTrace = Get-Content -LiteralPath $oracleRunOut -Raw

# ---- Step 4: attempt the COMPLETE mixa_selection.lm2 translation. ----
Push-Location $RepoRoot
$selSrc = "mixa_manager\mixa_selection.lm2"
$selOut = Join-Path $RunDir "mixa_selection_l2.lm1"
$selStdout = Join-Path $RunDir "selection_stdout.log"
$selStderr = Join-Path $RunDir "selection_stderr.log"
$SelExit = Invoke-Cmd "`"$l2exe`"" "`"$selSrc`" `"$selOut`"" $selStdout $selStderr
Pop-Location

$SelStdoutText = if (Test-Path -LiteralPath $selStdout) { Get-Content -LiteralPath $selStdout -Raw } else { "" }
$SelStderrText = if (Test-Path -LiteralPath $selStderr) { Get-Content -LiteralPath $selStderr -Raw } else { "" }
$SelSourceHash = (Get-FileHash -LiteralPath (Join-Path $RepoRoot $selSrc) -Algorithm SHA256).Hash
$SelHeaderHash = (Get-FileHash -LiteralPath (Join-Path $RepoRoot "mixa_manager\mixa_selection_l2.h.lm1") -Algorithm SHA256).Hash
$RunnerHash = (Get-FileHash -LiteralPath $PSCommandPath -Algorithm SHA256).Hash

$Verdict = $null
$ExitCode = 1
$L2TraceText = ""
$DiffText = ""

$KnownBarrier = ($SelStderrText -match "unknown foreign type") -or ($SelStderrText -match "incompatible entry signature")

if ($SelExit -ne 0 -and $KnownBarrier) {
    $Verdict = "EXPECTED_CORE_BARRIER"
    $ExitCode = 2
} elseif ($SelExit -ne 0) {
    $Verdict = "UNEXPECTED_FAILURE"
    $ExitCode = 1
} else {
    # ---- Step 5 (only reached once translation succeeds): build + run
    # the L2-side harness against the emitted mixa_selection_l2.lm1 and
    # diff byte-for-byte against the oracle trace. ----
    Push-Location $RepoRoot
    $l2SelC = Join-Path $RunDir "mixa_selection_l2.c"
    $l2scLog1 = Join-Path $RunDir "l2sel_trans_stdout.log"
    $l2scLog2 = Join-Path $RunDir "l2sel_trans_stderr.log"
    $l2scExit = Invoke-Cmd $L1Trans "`"$selOut`" `"$l2SelC`"" $l2scLog1 $l2scLog2
    Pop-Location
    if ($l2scExit -ne 0) {
        Get-Content $l2scLog2
        $Verdict = "UNEXPECTED_FAILURE"
        $ExitCode = 1
    } else {
        $l2SelO = Join-Path $RunDir "mixa_selection_l2.o"
        $l2occLog1 = Join-Path $RunDir "l2sel_compile_stdout.log"
        $l2occLog2 = Join-Path $RunDir "l2sel_compile_stderr.log"
        $l2occExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" -Dmalloc=test_malloc -Dcalloc=test_calloc -c `"$l2SelC`" -o `"$l2SelO`"" $l2occLog1 $l2occLog2
        if ($l2occExit -ne 0) {
            Get-Content $l2occLog2
            $Verdict = "UNEXPECTED_FAILURE"
            $ExitCode = 1
        } else {
            $l2Exe = Join-Path $RunDir "parity_l2.exe"
            $l2olLog1 = Join-Path $RunDir "l2sel_link_stdout.log"
            $l2olLog2 = Join-Path $RunDir "l2sel_link_stderr.log"
            $l2olExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" `"$harnessO`" `"$l2SelO`" `"$faultO`" -o `"$l2Exe`"" $l2olLog1 $l2olLog2
            if ($l2olExit -ne 0) {
                Get-Content $l2olLog2
                $Verdict = "UNEXPECTED_FAILURE"
                $ExitCode = 1
            } else {
                $l2RunOut = Join-Path $RunDir "l2_run_stdout.log"
                $l2RunErr = Join-Path $RunDir "l2_run_stderr.log"
                $l2RunExit = Invoke-Cmd "`"$l2Exe`"" "" $l2RunOut $l2RunErr
                $L2TraceText = Get-Content -LiteralPath $l2RunOut -Raw
                if ($l2RunExit -eq 0 -and $oracleRunExit -eq 0 -and $L2TraceText -eq $OracleTrace) {
                    $Verdict = "PASS"
                    $ExitCode = 0
                } else {
                    $Verdict = "PARITY_FAILURE"
                    $DiffText = Compare-Object -ReferenceObject ($OracleTrace -split "`n") -DifferenceObject ($L2TraceText -split "`n") | Out-String
                    $ExitCode = 1
                }
            }
        }
    }
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
Harness-Source-Sha256: $((Get-FileHash -LiteralPath (Join-Path $RepoRoot "mixa_manager\tests\mixa_selection_parity_harness.lm1") -Algorithm SHA256).Hash)
Alloc-Fault-Source-Sha256: $((Get-FileHash -LiteralPath (Join-Path $RepoRoot "mixa_manager\tests\mixa_selection_alloc_fault.c") -Algorithm SHA256).Hash)
Runner-Sha256: $RunnerHash
Oracle-Run-Exit-Code: $oracleRunExit
Oracle-Trace:
$OracleTrace
Selection-Translate-Exit-Code: $SelExit
Selection-Translate-Stdout:
$SelStdoutText
Selection-Translate-Stderr:
$SelStderrText
L2-Trace:
$L2TraceText
Diff (oracle vs L2, empty if identical):
$DiffText
Verdict: $Verdict
"@
Set-Content -LiteralPath (Join-Path $RunDir "run_summary.txt") -Value $Summary
$Summary
"Run directory: $RunDir"

switch ($Verdict) {
    "EXPECTED_CORE_BARRIER" {
        "EXPECTED_CORE_BARRIER: full mixa_selection.lm2 translation stopped at an already-measured barrier (`"unknown foreign type`" or `"incompatible entry signature`" -- core struct-type/@@: return-type admission not yet landed). This is NOT a pass. The oracle-side harness DID run (see Oracle-Trace above) -- all preparatory/oracle checks that are currently possible have been exercised."
    }
    "UNEXPECTED_FAILURE" {
        "UNEXPECTED_FAILURE: translation or build failed with something OTHER than the already-known barriers. Inspect the logs under $RunDir -- this may be a further, distinct compiler barrier; do not invent a manager-side workaround for it."
    }
    "PARITY_FAILURE" {
        "PARITY_FAILURE: both implementations built and ran, but their traces differ (see Diff above) or one exited non-zero. This is a real behavioral divergence between the L1 oracle and the L2 port -- investigate before reporting anything as a pass."
    }
    "PASS" {
        "PASS: full mixa_selection.lm2 translated, built, and ran; its trace is byte-identical to the L1 oracle's own trace across all nine scenarios."
    }
}

exit $ExitCode
