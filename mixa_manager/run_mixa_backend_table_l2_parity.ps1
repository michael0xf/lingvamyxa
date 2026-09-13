# Real L1-vs-L2 parity gate for mixa_backend_table (share-and-backends
# ticket 20260913-192700, module 5 of 6). No dedicated ABI probe pair,
# and no L1 header unit to predef at all: mixa_backend.h has no .h.lm1
# counterpart anywhere in this codebase -- real, plain, hand-written C
# (its own top comment records converting it as planned future work,
# "H2", blocked on run_mixa.ps1's own .h.lm1 -> .lm1.h conversion step
# not yet being written). MixaBackend's own field is dereferenced via
# `\` with no L1-level struct: declaration, proven safe by the
# established "`\` defers to gcc" finding.
#
# Headless-only link (matching every other headless-backend-driven
# harness this segment): the existing accepted mixa_backend_table_
# selftest.lm1's own conditional win32 branch naturally never fires
# here (no win32 constructor registered) -- reported as-is, exactly
# the same shape the real selftest's own else-branch already covers.
#
#   1. ALWAYS builds and runs the ORACLE-side harness (table_valid
#      rejecting null/all-zero/each-required-slot-holed table; a real
#      mis-stamped concrete open refused without mutating the caller's
#      out slot, for both a null and a wrong vt; two real headless
#      handles live simultaneously with independent vt identity,
#      closing one leaving the other fully functional; the registry/
#      by_name/default_table resolving correctly).
#   2. Attempts to translate the COMPLETE mixa_backend_table.lm2.
#      - fails with an already-known barrier -> EXPECTED_CORE_BARRIER
#        (exit 2). NOT a pass.
#      - fails with any OTHER diagnostic -> UNEXPECTED_FAILURE (exit 1).
#      - SUCCEEDS -> builds/links/runs the L2-side harness and diffs its
#        stdout against the oracle trace byte-for-byte. PASS (exit 0)
#        only on an exact match, else PARITY_FAILURE (exit 1).
#
# mixa_backend.h/mixa_backend_table.lm1 and every real dependency
# (mixa_event_fifo, mixa_backend_headless, mixa_backend_ctors_headless)
# are the parity oracle (or real, unmodified dependencies) and are
# never touched. Nothing under stg/l1_baseline is modified, only read.
# Every input is built fresh in a unique run directory -- no stale
# objects.
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
$BaseDir = Join-Path $RepoRoot "build\mixa\claude\mixa_backend_table_l2_parity"
$RunDir = Join-Path $BaseDir "run_${RunTimestamp}_${RunGuid}"
$HeaderDir = Join-Path $RunDir "headers\mixa_manager"
New-Item -ItemType Directory -Force -Path $HeaderDir | Out-Null

function Invoke-Cmd([string]$exe, [string]$argsStr, [string]$outLog, [string]$errLog) {
    & cmd /c "$exe $argsStr > `"$outLog`" 2> `"$errLog`""
    return $LASTEXITCODE
}

function Invoke-HeaderTrans([string]$SrcRel, [string]$OutName) {
    Push-Location $RepoRoot
    $out = Join-Path $HeaderDir $OutName
    $o1 = Join-Path $RunDir "hdr_${OutName}_stdout.log"
    $o2 = Join-Path $RunDir "hdr_${OutName}_stderr.log"
    $rc = Invoke-Cmd $L1Trans "$SrcRel `"$out`"" $o1 $o2
    Pop-Location
    if ($rc -ne 0) { Get-Content $o2; throw "$SrcRel header translation failed" }
}

function Build-RealDep([string]$Name, [string]$SrcRel) {
    Push-Location $RepoRoot
    $depC = Join-Path $RunDir "$Name.c"
    $dLog1 = Join-Path $RunDir "${Name}_trans_stdout.log"
    $dLog2 = Join-Path $RunDir "${Name}_trans_stderr.log"
    $dExit = Invoke-Cmd $L1Trans "$SrcRel `"$depC`"" $dLog1 $dLog2
    Pop-Location
    if ($dExit -ne 0) { Get-Content $dLog2; throw "real $Name translation failed" }
    $depO = Join-Path $RunDir "$Name.o"
    $dcLog1 = Join-Path $RunDir "${Name}_compile_stdout.log"
    $dcLog2 = Join-Path $RunDir "${Name}_compile_stderr.log"
    $dcExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" -c `"$depC`" -o `"$depO`"" $dcLog1 $dcLog2
    if ($dcExit -ne 0) { Get-Content $dcLog2; throw "real $Name compile failed" }
    return $depO
}

# ---- Step 0: translate the L2 header (mixa_backend.h is plain C, no
# other real headers to translate). ----
Invoke-HeaderTrans "mixa_manager\mixa_backend_table_l2.h.lm1" "mixa_backend_table_l2.lm1.h"

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

# ---- Step 2: build the harness object (compiled ONCE). ----
Push-Location $RepoRoot
$harnessC = Join-Path $RunDir "harness.c"
$hLog1 = Join-Path $RunDir "harness_trans_stdout.log"
$hLog2 = Join-Path $RunDir "harness_trans_stderr.log"
$hExit = Invoke-Cmd $L1Trans "mixa_manager\tests\mixa_backend_table_parity_harness.lm1 `"$harnessC`"" $hLog1 $hLog2
Pop-Location
if ($hExit -ne 0) { Get-Content $hLog2; throw "harness translation failed" }

$harnessO = Join-Path $RunDir "harness.o"
$hcLog1 = Join-Path $RunDir "harness_compile_stdout.log"
$hcLog2 = Join-Path $RunDir "harness_compile_stderr.log"
$hcExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" -c `"$harnessC`" -o `"$harnessO`"" $hcLog1 $hcLog2
if ($hcExit -ne 0) { Get-Content $hcLog2; throw "harness compile failed" }

# ---- Step 3: build the shared headless-backend dependency set once
# (used by BOTH oracle and L2 sides -- mixa_backend_table.lm1 predefs
# nothing). ----
$eventFifoO = Build-RealDep "mixa_event_fifo" "mixa_manager\mixa_event_fifo.lm1"
$backendHeadlessO = Build-RealDep "mixa_backend_headless" "mixa_manager\mixa_backend_headless.lm1"
$backendCtorsO = Build-RealDep "mixa_backend_ctors_headless" "mixa_manager\mixa_backend_ctors_headless.lm1"

# ---- Step 4: ALWAYS build + run the ORACLE-side harness. ----
$oracleC = Join-Path $RunDir "mixa_backend_table_oracle.c"
$ocLog1 = Join-Path $RunDir "oracle_trans_stdout.log"
$ocLog2 = Join-Path $RunDir "oracle_trans_stderr.log"
Push-Location $RepoRoot
$ocExit = Invoke-Cmd $L1Trans "mixa_manager\mixa_backend_table.lm1 `"$oracleC`"" $ocLog1 $ocLog2
Pop-Location
if ($ocExit -ne 0) { Get-Content $ocLog2; throw "oracle mixa_backend_table.lm1 translation failed" }

$oracleO = Join-Path $RunDir "mixa_backend_table_oracle.o"
$occLog1 = Join-Path $RunDir "oracle_compile_stdout.log"
$occLog2 = Join-Path $RunDir "oracle_compile_stderr.log"
$occExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" -c `"$oracleC`" -o `"$oracleO`"" $occLog1 $occLog2
if ($occExit -ne 0) { Get-Content $occLog2; throw "oracle mixa_backend_table.lm1 compile failed" }

$oracleExe = Join-Path $RunDir "parity_oracle.exe"
$olLog1 = Join-Path $RunDir "oracle_link_stdout.log"
$olLog2 = Join-Path $RunDir "oracle_link_stderr.log"
$olExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" `"$harnessO`" `"$oracleO`" `"$eventFifoO`" `"$backendHeadlessO`" `"$backendCtorsO`" -o `"$oracleExe`"" $olLog1 $olLog2
if ($olExit -ne 0) { Get-Content $olLog2; throw "oracle harness link failed" }

$OracleFixtureDir = Join-Path $RunDir "oracle_fixtures"
New-Item -ItemType Directory -Force -Path $OracleFixtureDir | Out-Null
Push-Location $RepoRoot
$oracleRunOut = Join-Path $RunDir "oracle_run_stdout.log"
$oracleRunErr = Join-Path $RunDir "oracle_run_stderr.log"
$oracleRunExit = Invoke-Cmd "`"$oracleExe`"" "" $oracleRunOut $oracleRunErr
Pop-Location
$OracleTrace = Get-Content -LiteralPath $oracleRunOut -Raw

# ---- Step 5: attempt the COMPLETE mixa_backend_table.lm2 translation. ----
Push-Location $RepoRoot
$btSrc = "mixa_manager\mixa_backend_table.lm2"
$btOut = Join-Path $RunDir "mixa_backend_table_l2.lm1"
$btStdout = Join-Path $RunDir "bt_stdout.log"
$btStderr = Join-Path $RunDir "bt_stderr.log"
$BtExit = Invoke-Cmd "`"$l2exe`"" "`"$btSrc`" `"$btOut`"" $btStdout $btStderr
Pop-Location

$BtStdoutText = if (Test-Path -LiteralPath $btStdout) { Get-Content -LiteralPath $btStdout -Raw } else { "" }
$BtStderrText = if (Test-Path -LiteralPath $btStderr) { Get-Content -LiteralPath $btStderr -Raw } else { "" }
$BtSourceHash = (Get-FileHash -LiteralPath (Join-Path $RepoRoot $btSrc) -Algorithm SHA256).Hash
$RunnerHash = (Get-FileHash -LiteralPath $PSCommandPath -Algorithm SHA256).Hash

$Verdict = $null
$ExitCode = 1
$L2TraceText = ""
$DiffText = ""

$KnownBarrier = ($BtStderrText -match "unknown foreign type") -or ($BtStderrText -match "incompatible entry signature") -or ($BtStderrText -match "unsupported own array declaration")

if ($BtExit -ne 0 -and $KnownBarrier) {
    $Verdict = "EXPECTED_CORE_BARRIER"
    $ExitCode = 2
} elseif ($BtExit -ne 0) {
    $Verdict = "UNEXPECTED_FAILURE"
    $ExitCode = 1
} else {
    Push-Location $RepoRoot
    $l2BtC = Join-Path $RunDir "mixa_backend_table_l2.c"
    $l2ccLog1 = Join-Path $RunDir "l2bt_trans_stdout.log"
    $l2ccLog2 = Join-Path $RunDir "l2bt_trans_stderr.log"
    $l2ccExit = Invoke-Cmd $L1Trans "`"$btOut`" `"$l2BtC`"" $l2ccLog1 $l2ccLog2
    Pop-Location
    if ($l2ccExit -ne 0) {
        Get-Content $l2ccLog2
        $Verdict = "UNEXPECTED_FAILURE"
        $ExitCode = 1
    } else {
        $l2BtO = Join-Path $RunDir "mixa_backend_table_l2.o"
        $l2occLog1 = Join-Path $RunDir "l2bt_compile_stdout.log"
        $l2occLog2 = Join-Path $RunDir "l2bt_compile_stderr.log"
        $l2occExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" -c `"$l2BtC`" -o `"$l2BtO`"" $l2occLog1 $l2occLog2
        if ($l2occExit -ne 0) {
            Get-Content $l2occLog2
            $Verdict = "UNEXPECTED_FAILURE"
            $ExitCode = 1
        } else {
            $l2Exe = Join-Path $RunDir "parity_l2.exe"
            $l2olLog1 = Join-Path $RunDir "l2bt_link_stdout.log"
            $l2olLog2 = Join-Path $RunDir "l2bt_link_stderr.log"
            $l2olExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" `"$harnessO`" `"$l2BtO`" `"$eventFifoO`" `"$backendHeadlessO`" `"$backendCtorsO`" -o `"$l2Exe`"" $l2olLog1 $l2olLog2
            if ($l2olExit -ne 0) {
                Get-Content $l2olLog2
                $Verdict = "UNEXPECTED_FAILURE"
                $ExitCode = 1
            } else {
                $L2FixtureDir = Join-Path $RunDir "l2_fixtures"
                New-Item -ItemType Directory -Force -Path $L2FixtureDir | Out-Null
                Push-Location $RepoRoot
                $l2RunOut = Join-Path $RunDir "l2_run_stdout.log"
                $l2RunErr = Join-Path $RunDir "l2_run_stderr.log"
                $l2RunExit = Invoke-Cmd "`"$l2Exe`"" "" $l2RunOut $l2RunErr
                Pop-Location
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
BackendTable-L2-Header: mixa_manager\mixa_backend_table_l2.h.lm1
BackendTable-L2-Source: $btSrc
BackendTable-L2-Source-Sha256: $BtSourceHash
Harness-Source-Sha256: $((Get-FileHash -LiteralPath (Join-Path $RepoRoot "mixa_manager\tests\mixa_backend_table_parity_harness.lm1") -Algorithm SHA256).Hash)
Runner-Sha256: $RunnerHash
Oracle-Run-Exit-Code: $oracleRunExit
Oracle-Trace:
$OracleTrace
BackendTable-Translate-Exit-Code: $BtExit
BackendTable-Translate-Stdout:
$BtStdoutText
BackendTable-Translate-Stderr:
$BtStderrText
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
        "EXPECTED_CORE_BARRIER: full mixa_backend_table.lm2 translation stopped at an already-known barrier. This is NOT a pass -- the integrated frontend is not yet on main. The oracle-side harness DID run (see Oracle-Trace above)."
    }
    "UNEXPECTED_FAILURE" {
        "UNEXPECTED_FAILURE: translation or build failed with something OTHER than the already-known barriers. Inspect the logs under $RunDir."
    }
    "PARITY_FAILURE" {
        "PARITY_FAILURE: both implementations built and ran, but their traces differ (see Diff above) or one exited non-zero."
    }
    "PASS" {
        "PASS: full mixa_backend_table.lm2 translated, built, and ran; its trace is byte-identical to the L1 oracle's own trace."
    }
}

exit $ExitCode
