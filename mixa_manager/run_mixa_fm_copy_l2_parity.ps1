# Real L1-vs-L2 parity gate for mixa_fm_copy (copy-chain ticket
# 20260913-185600, module 2 of 4). No dedicated ABI probe pair: this
# module declares no L2-only struct of its own (MixaFmCopyHereCtx/
# MixaAppLoop are already real, unmodified declarations reached via
# the REAL mixa_fm_copy.h.lm1 predef'd directly, identically on both
# the real oracle and this L2 port).
#
# Dependency-linking note: mixa_fm_copy.lm1 predefs the FULL bodies of
# mixa_file_manager.lm1 and mixa_copy.lm1 directly in ONE translation
# unit -- l1trans's own predef deduplication handles the fact that both
# of those further predef mixa_dir_win32.lm1/mixa_selection.lm1's real
# bodies (mixa_copy.lm1 via its own predef of mixa_selection_walk.lm1,
# which predefs them again) -- so the ORACLE side needs no separate
# real objects. The L2 header instead predefs ONLY mixa_fm_copy.h.lm1
# (header-only, no bodies), so the L2 side links against a test-owned
# wrapper (tests/mixa_fm_copy_l2_link.lm1) that predefs BOTH mixa_file_
# manager.lm1 and mixa_copy.lm1 together in one unit, for the same
# dedup reason -- compiling them as two SEPARATE objects and linking
# both would duplicate-define the shared dir_win32/selection symbols.
#
#   1. ALWAYS builds and runs the ORACLE-side harness (mixa_fm_copy_
#      selection's own refresh-on-match behavior, the real poison-probe
#      use-after-free regression on a borrowed destination pointer,
#      Copy Here's empty-selection no-op, rectangle hit-test bounds, an
#      existing-file conflict/cancellation/real I/O fault all leaving
#      the view usable, both accepted Ctrl+V spellings plus rejections,
#      pump-dispatch's own disjoint return domain, and the bounded
#      native-loop foundation's real FIFO event order/CLOSE/empty-queue/
#      teardown contract).
#   2. Attempts to translate the COMPLETE mixa_fm_copy.lm2.
#      - fails with an already-known barrier -> EXPECTED_CORE_BARRIER
#        (exit 2). NOT a pass.
#      - fails with any OTHER diagnostic -> UNEXPECTED_FAILURE (exit 1).
#      - SUCCEEDS -> builds/links/runs the L2-side harness and diffs its
#        stdout against the oracle trace byte-for-byte. PASS (exit 0)
#        only on an exact match, else PARITY_FAILURE (exit 1).
#
# mixa_fm_copy.h.lm1/.lm1 and every real dependency are the parity
# oracle (or real, unmodified dependencies) and are never touched.
# Nothing under stg/l1_baseline is modified, only read. Every input is
# built fresh in a unique run directory -- no stale objects.
param()
$ErrorActionPreference = "Stop"

$RepoRoot = Split-Path -Parent $PSScriptRoot
$L1Root = Join-Path $RepoRoot "stg\l1_baseline"
$L1Trans = Join-Path $L1Root "build\l1trans\gen2\l1trans.exe"
. (Join-Path $PSScriptRoot "lib_l2_runtime_support.ps1")
$ExpectedL1Hash = Get-L1Pin -L1Root $L1Root

$ActualL1Hash = Assert-PinnedL1Translator -L1Trans $L1Trans -L1Root $L1Root

$guards = @(
    "-Werror=incompatible-pointer-types", "-Werror=discarded-qualifiers",
    "-Werror=implicit-function-declaration", "-Werror=implicit-int"
)
$GccStd = "-std=c99 -Wall -Wextra -Wpedantic $($guards -join ' ')"

$RunTimestamp = (Get-Date -Format "yyyyMMdd_HHmmss_fff")
$RunGuid = [GUID]::NewGuid().ToString().Substring(0, 8)
$BaseDir = Join-Path $RepoRoot "build\mixa\claude\mixa_fm_copy_l2_parity"
$RunDir = Join-Path $BaseDir "run_${RunTimestamp}_${RunGuid}"
$HeaderDir = Join-Path $RunDir "headers\mixa_manager"
New-Item -ItemType Directory -Force -Path $HeaderDir | Out-Null

function Invoke-Cmd([string]$exe, [string]$argsStr, [string]$outLog, [string]$errLog) {
    & cmd /c "$exe $argsStr > `"$outLog`" 2> `"$errLog`""
    return $LASTEXITCODE
}

# Shared L2 runtime-support helper (ticket 20260914-003000): the runtime
# header/object generation trio, kept in one file so every runner uses
# the identical implementation rather than a pasted, driftable copy.
$InvokeCmdRef = { param($e, $a, $o, $er) Invoke-Cmd $e $a $o $er }

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

# ---- Step 0: translate the real headers this module's own real
# dependency chain needs, plus the L2 header. ----
Invoke-HeaderTrans "mixa_manager\mixa_dir.h.lm1" "mixa_dir.lm1.h"
Invoke-HeaderTrans "mixa_manager\mixa_dir_win32.h.lm1" "mixa_dir_win32.lm1.h"
Invoke-HeaderTrans "mixa_manager\mixa_selection_walk.h.lm1" "mixa_selection_walk.lm1.h"
Invoke-HeaderTrans "mixa_manager\mixa_fileio_win32.h.lm1" "mixa_fileio_win32.lm1.h"
Invoke-HeaderTrans "mixa_manager\mixa_fileio.h.lm1" "mixa_fileio.lm1.h"
Invoke-HeaderTrans "mixa_manager\mixa_copy.h.lm1" "mixa_copy.lm1.h"
Invoke-HeaderTrans "mixa_manager\mixa_file_manager.h.lm1" "mixa_file_manager.lm1.h"
Invoke-HeaderTrans "mixa_manager\mixa_fm_copy.h.lm1" "mixa_fm_copy.lm1.h"
Invoke-HeaderTrans "mixa_manager\mixa_fm_copy_l2.h.lm1" "mixa_fm_copy_l2.lm1.h"

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
$hExit = Invoke-Cmd $L1Trans "mixa_manager\tests\mixa_fm_copy_parity_harness.lm1 `"$harnessC`"" $hLog1 $hLog2
Pop-Location
if ($hExit -ne 0) { Get-Content $hLog2; throw "harness translation failed" }

$harnessO = Join-Path $RunDir "harness.o"
$hcLog1 = Join-Path $RunDir "harness_compile_stdout.log"
$hcLog2 = Join-Path $RunDir "harness_compile_stderr.log"
$hcExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" -c `"$harnessC`" -o `"$harnessO`"" $hcLog1 $hcLog2
if ($hcExit -ne 0) { Get-Content $hcLog2; throw "harness compile failed" }

# ---- Step 3: ALWAYS build + run the ORACLE-side harness. mixa_fm_
# copy.lm1's own translation unit already embeds mixa_file_manager/
# mixa_copy (see top comment), with l1trans's own predef dedup handling
# the shared dir_win32/selection bodies -- no separate real objects
# needed. ----
$oracleC = Join-Path $RunDir "mixa_fm_copy_oracle.c"
$ocLog1 = Join-Path $RunDir "oracle_trans_stdout.log"
$ocLog2 = Join-Path $RunDir "oracle_trans_stderr.log"
Push-Location $RepoRoot
$ocExit = Invoke-Cmd $L1Trans "mixa_manager\mixa_fm_copy.lm1 `"$oracleC`"" $ocLog1 $ocLog2
Pop-Location
if ($ocExit -ne 0) { Get-Content $ocLog2; throw "oracle mixa_fm_copy.lm1 translation failed" }

$oracleO = Join-Path $RunDir "mixa_fm_copy_oracle.o"
$occLog1 = Join-Path $RunDir "oracle_compile_stdout.log"
$occLog2 = Join-Path $RunDir "oracle_compile_stderr.log"
$occExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" -c `"$oracleC`" -o `"$oracleO`"" $occLog1 $occLog2
if ($occExit -ne 0) { Get-Content $occLog2; throw "oracle mixa_fm_copy.lm1 compile failed" }

$eventFifoO = Build-RealDep "mixa_event_fifo" "mixa_manager\mixa_event_fifo.lm1"
$backendTableO = Build-RealDep "mixa_backend_table" "mixa_manager\mixa_backend_table.lm1"
$backendHeadlessO = Build-RealDep "mixa_backend_headless" "mixa_manager\mixa_backend_headless.lm1"
$backendCtorsO = Build-RealDep "mixa_backend_ctors_headless" "mixa_manager\mixa_backend_ctors_headless.lm1"
$pumpO = Build-RealDep "mixa_pump" "mixa_manager\mixa_pump.lm1"

$oracleExe = Join-Path $RunDir "parity_oracle.exe"
$olLog1 = Join-Path $RunDir "oracle_link_stdout.log"
$olLog2 = Join-Path $RunDir "oracle_link_stderr.log"
$olExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" `"$harnessO`" `"$oracleO`" `"$eventFifoO`" `"$backendTableO`" `"$backendHeadlessO`" `"$backendCtorsO`" `"$pumpO`" -o `"$oracleExe`"" $olLog1 $olLog2
if ($olExit -ne 0) { Get-Content $olLog2; throw "oracle harness link failed" }

$OracleFixtureDir = Join-Path $RunDir "oracle_fixtures"
New-Item -ItemType Directory -Force -Path $OracleFixtureDir | Out-Null
Push-Location $RepoRoot
$oracleRunOut = Join-Path $RunDir "oracle_run_stdout.log"
$oracleRunErr = Join-Path $RunDir "oracle_run_stderr.log"
$oracleRunExit = Invoke-Cmd "`"$oracleExe`"" "`"$OracleFixtureDir`"" $oracleRunOut $oracleRunErr
Pop-Location
$OracleTrace = Get-Content -LiteralPath $oracleRunOut -Raw

# ---- Step 4: attempt the COMPLETE mixa_fm_copy.lm2 translation. ----
Push-Location $RepoRoot
$fcSrc = "mixa_manager\mixa_fm_copy.lm2"
$fcOut = Join-Path $RunDir "mixa_fm_copy_l2.lm1"
$fcStdout = Join-Path $RunDir "fc_stdout.log"
$fcStderr = Join-Path $RunDir "fc_stderr.log"
$env:L2_RUNTIME_ROOT = "stg/l1_baseline/l2src/"
$FcExit = Invoke-Cmd "`"$l2exe`"" "`"$fcSrc`" `"$fcOut`"" $fcStdout $fcStderr
Pop-Location

$FcStdoutText = if (Test-Path -LiteralPath $fcStdout) { Get-Content -LiteralPath $fcStdout -Raw } else { "" }
$FcStderrText = if (Test-Path -LiteralPath $fcStderr) { Get-Content -LiteralPath $fcStderr -Raw } else { "" }
$FcSourceHash = (Get-FileHash -LiteralPath (Join-Path $RepoRoot $fcSrc) -Algorithm SHA256).Hash
$RunnerHash = (Get-FileHash -LiteralPath $PSCommandPath -Algorithm SHA256).Hash

$Verdict = $null
$ExitCode = 1
$L2TraceText = ""
$DiffText = ""

$KnownBarrier = ($FcStderrText -match "unknown foreign type") -or ($FcStderrText -match "incompatible entry signature")

if ($FcExit -ne 0 -and $KnownBarrier) {
    $Verdict = "EXPECTED_CORE_BARRIER"
    $ExitCode = 2
} elseif ($FcExit -ne 0) {
    $Verdict = "UNEXPECTED_FAILURE"
    $ExitCode = 1
} else {
    Push-Location $RepoRoot
    $l2FcC = Join-Path $RunDir "mixa_fm_copy_l2.c"
    $l2ccLog1 = Join-Path $RunDir "l2fc_trans_stdout.log"
    $l2ccLog2 = Join-Path $RunDir "l2fc_trans_stderr.log"
    $l2ccExit = Invoke-Cmd $L1Trans "`"$fcOut`" `"$l2FcC`"" $l2ccLog1 $l2ccLog2
    Pop-Location
    if ($l2ccExit -ne 0) {
        Get-Content $l2ccLog2
        $Verdict = "UNEXPECTED_FAILURE"
        $ExitCode = 1
    } else {
        $L2Rt = Add-L2RuntimeSupport -L1Trans $L1Trans -L1Root $L1Root -RunDir $RunDir -InvokeCmd $InvokeCmdRef
        $l2FcO = Join-Path $RunDir "mixa_fm_copy_l2.o"
        $l2occLog1 = Join-Path $RunDir "l2fc_compile_stdout.log"
        $l2occLog2 = Join-Path $RunDir "l2fc_compile_stderr.log"
        $l2occExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" -I `"$($L2Rt.HeaderRoot)`" -I `"$($L2Rt.HeaderRoot)\stg\l1_baseline`" -I `"$L1Root`" -c `"$l2FcC`" -o `"$l2FcO`"" $l2occLog1 $l2occLog2
        if ($l2occExit -ne 0) {
            Get-Content $l2occLog2
            $Verdict = "UNEXPECTED_FAILURE"
            $ExitCode = 1
        } else {
            $linkDepO = Build-RealDep "mixa_fm_copy_l2_link" "mixa_manager\tests\mixa_fm_copy_l2_link.lm1"
            $l2Exe = Join-Path $RunDir "parity_l2.exe"
            $l2olLog1 = Join-Path $RunDir "l2fc_link_stdout.log"
            $l2olLog2 = Join-Path $RunDir "l2fc_link_stderr.log"
            $l2olExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" `"$harnessO`" `"$l2FcO`" `"$linkDepO`" `"$eventFifoO`" `"$backendTableO`" `"$backendHeadlessO`" `"$backendCtorsO`" `"$pumpO`" $($L2Rt.ObjList) -o `"$l2Exe`"" $l2olLog1 $l2olLog2
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
                $l2RunExit = Invoke-Cmd "`"$l2Exe`"" "`"$L2FixtureDir`"" $l2RunOut $l2RunErr
                Pop-Location
                $L2TraceText = Get-Content -LiteralPath $l2RunOut -Raw
                # Ticket 20260914-001000 (Fable): the oracle and L2 runs get
                # separate, independently populated fixture roots (by
                # design, per 20260913-235500 -- sharing one leaks state
                # between runs), so any check that prints an absolute path
                # will legitimately differ in ONLY that root between the
                # two traces even when both sides behave identically.
                # Normalize each trace's own known root to a fixed token
                # before comparing, so a real path is still checked (just
                # root-agnostically) rather than skipped.
                $NormOracleTrace = $OracleTrace -replace [regex]::Escape($OracleFixtureDir), "<FIXTURE_ROOT>"
                $NormL2Trace = $L2TraceText -replace [regex]::Escape($L2FixtureDir), "<FIXTURE_ROOT>"
                if ($l2RunExit -eq 0 -and $oracleRunExit -eq 0 -and $NormL2Trace -eq $NormOracleTrace) {
                    $Verdict = "PASS"
                    $ExitCode = 0
                } else {
                    $Verdict = "PARITY_FAILURE"
                    $DiffText = Compare-Object -ReferenceObject ($NormOracleTrace -split "`n") -DifferenceObject ($NormL2Trace -split "`n") | Out-String
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
FmCopy-L2-Header: mixa_manager\mixa_fm_copy_l2.h.lm1
FmCopy-L2-Source: $fcSrc
FmCopy-L2-Source-Sha256: $FcSourceHash
Harness-Source-Sha256: $((Get-FileHash -LiteralPath (Join-Path $RepoRoot "mixa_manager\tests\mixa_fm_copy_parity_harness.lm1") -Algorithm SHA256).Hash)
Runner-Sha256: $RunnerHash
Oracle-Run-Exit-Code: $oracleRunExit
Oracle-Trace:
$OracleTrace
FmCopy-Translate-Exit-Code: $FcExit
FmCopy-Translate-Stdout:
$FcStdoutText
FmCopy-Translate-Stderr:
$FcStderrText
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
        "EXPECTED_CORE_BARRIER: full mixa_fm_copy.lm2 translation stopped at an already-known barrier. This is NOT a pass -- the integrated frontend is not yet on main. The oracle-side harness DID run (see Oracle-Trace above)."
    }
    "UNEXPECTED_FAILURE" {
        "UNEXPECTED_FAILURE: translation or build failed with something OTHER than the already-known barriers. Inspect the logs under $RunDir."
    }
    "PARITY_FAILURE" {
        "PARITY_FAILURE: both implementations built and ran, but their traces differ (see Diff above) or one exited non-zero."
    }
    "PASS" {
        "PASS: full mixa_fm_copy.lm2 translated, built, and ran; its trace is byte-identical to the L1 oracle's own trace."
    }
}

exit $ExitCode
