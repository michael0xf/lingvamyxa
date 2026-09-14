# Real L1-vs-L2 parity gate for mixa_app_controller (ticket 20260913-
# 151000) -- the top-level integration seam this whole segment's own
# module-by-module L2 ports were building toward.
#
# MixaAppController is genuinely opaque (its real public header, mixa_
# app_controller.h, exposes no field at all), so the harness itself
# needs no predef/forward-declare machinery: it is compiled ONCE,
# purely against the real, unmodified public header plus the real
# headless backend -- the same include line the existing, already-
# accepted mixa_app_controller_e2e_selftest.lm1 already uses -- and
# linked twice, once against the real oracle object and once against
# the L2-generated object, both alongside the SAME 15 other real
# dependency objects (the exact link list run_app_controller_e2e_
# selftest.ps1 already uses: fmpanel, highlight, event_fifo, backend_
# table, backend_headless, backend_ctors_headless, pump, console_
# window, file_win32, app_path, help, cmdline, cmdline_dispatch,
# process_marker, process_win32).
#
#   0. ABI parity gate: dedicated MixaAppController probe pair (both
#      sides via l1trans-generated C headers, since neither the real
#      nor the L2 header is hand-written plain C).
#   1. ALWAYS builds and runs the ORACLE-side harness (real headless
#      backend, real spawned "echo"/"ping" children, real console file).
#   2. Attempts to translate the COMPLETE mixa_app_controller.lm2.
#      - fails with an already-known barrier -> EXPECTED_CORE_BARRIER
#        (exit 2). NOT a pass.
#      - fails with any OTHER diagnostic -> UNEXPECTED_FAILURE (exit 1).
#      - SUCCEEDS -> builds/links/runs the L2-side harness and diffs its
#        stdout against the oracle trace byte-for-byte. PASS (exit 0)
#        only on an exact match, else PARITY_FAILURE (exit 1).
#
# mixa_app_controller.h/_impl.h/.lm1 and every real dependency listed
# above are the parity oracle (or real, unmodified dependencies) and
# are never touched. Nothing under stg/l1_baseline is modified, only
# read. Every input is built fresh in a unique run directory -- no
# stale objects.
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
$LinkLibs = "-lshell32"

$RunTimestamp = (Get-Date -Format "yyyyMMdd_HHmmss_fff")
$RunGuid = [GUID]::NewGuid().ToString().Substring(0, 8)
$BaseDir = Join-Path $RepoRoot "build\mixa\claude\mixa_app_controller_l2_parity"
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
Invoke-HeaderTrans "mixa_manager\mixa_dir_win32.h.lm1" "mixa_dir_win32.lm1.h"
Invoke-HeaderTrans "mixa_manager\mixa_dir.h.lm1" "mixa_dir.lm1.h"
Invoke-HeaderTrans "mixa_manager\mixa_selection_walk.h.lm1" "mixa_selection_walk.lm1.h"
Invoke-HeaderTrans "mixa_manager\mixa_fileio_win32.h.lm1" "mixa_fileio_win32.lm1.h"
Invoke-HeaderTrans "mixa_manager\mixa_fileio.h.lm1" "mixa_fileio.lm1.h"
Invoke-HeaderTrans "mixa_manager\mixa_copy.h.lm1" "mixa_copy.lm1.h"
Invoke-HeaderTrans "mixa_manager\mixa_file_manager.h.lm1" "mixa_file_manager.lm1.h"
Invoke-HeaderTrans "mixa_manager\mixa_fm_copy.h.lm1" "mixa_fm_copy.lm1.h"
Invoke-HeaderTrans "mixa_manager\mixa_console_window.h.lm1" "mixa_console_window.lm1.h"
Invoke-HeaderTrans "mixa_manager\mixa_process_marker.h.lm1" "mixa_process_marker.lm1.h"
Invoke-HeaderTrans "mixa_manager\mixa_app_controller_l2.h.lm1" "mixa_app_controller_l2.lm1.h"

# ---- Step 0.5: ABI parity gate (dedicated probe pair). ----
$abiRealExe = Join-Path $RunDir "abi_probe_real.exe"
$abiRealLog1 = Join-Path $RunDir "abi_probe_real_compile_stdout.log"
$abiRealLog2 = Join-Path $RunDir "abi_probe_real_compile_stderr.log"
$abiRealExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" `"$RepoRoot\mixa_manager\tests\mixa_app_controller_abi_probe_real.c`" -o `"$abiRealExe`"" $abiRealLog1 $abiRealLog2
if ($abiRealExit -ne 0) { Get-Content $abiRealLog2; throw "ABI probe (real header) compile failed" }

$abiL2Exe = Join-Path $RunDir "abi_probe_l2.exe"
$abiL2Log1 = Join-Path $RunDir "abi_probe_l2_compile_stdout.log"
$abiL2Log2 = Join-Path $RunDir "abi_probe_l2_compile_stderr.log"
$abiL2Exit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" `"$RepoRoot\mixa_manager\tests\mixa_app_controller_abi_probe_l2.c`" -o `"$abiL2Exe`"" $abiL2Log1 $abiL2Log2
if ($abiL2Exit -ne 0) { Get-Content $abiL2Log2; throw "ABI probe (L2 header) compile failed" }

$abiRealOut = Join-Path $RunDir "abi_probe_real_run.log"
$abiRealRunExit = Invoke-Cmd "`"$abiRealExe`"" "" $abiRealOut (Join-Path $RunDir "abi_probe_real_run_stderr.log")
$abiL2Out = Join-Path $RunDir "abi_probe_l2_run.log"
$abiL2RunExit = Invoke-Cmd "`"$abiL2Exe`"" "" $abiL2Out (Join-Path $RunDir "abi_probe_l2_run_stderr.log")
if ($abiRealRunExit -ne 0 -or $abiL2RunExit -ne 0) { throw "ABI probe run failed (real exit $abiRealRunExit, L2 exit $abiL2RunExit)" }

$AbiRealText = Get-Content -LiteralPath $abiRealOut -Raw
$AbiL2Text = Get-Content -LiteralPath $abiL2Out -Raw
if ($AbiRealText -ne $AbiL2Text) {
    $abiDiff = Compare-Object -ReferenceObject ($AbiRealText -split "`n") -DifferenceObject ($AbiL2Text -split "`n") | Out-String
    $AbiSummary = "ABI_MISMATCH: real-header probe output:`n$AbiRealText`nL2-header probe output:`n$AbiL2Text`nDiff:`n$abiDiff"
    Set-Content -LiteralPath (Join-Path $RunDir "run_summary.txt") -Value $AbiSummary
    $AbiSummary
    "Run directory: $RunDir"
    "ABI_MISMATCH: struct layout drift detected -- see the diff above. The harness was NOT built or run."
    exit 1
}

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

# ---- Step 2: build the harness object (compiled ONCE -- real, plain-C
# public header only, works unchanged against either link target) and
# the 15 real dependency objects shared by both sides. ----
Push-Location $RepoRoot
$harnessC = Join-Path $RunDir "harness.c"
$hLog1 = Join-Path $RunDir "harness_trans_stdout.log"
$hLog2 = Join-Path $RunDir "harness_trans_stderr.log"
$hExit = Invoke-Cmd $L1Trans "mixa_manager\tests\mixa_app_controller_parity_harness.lm1 `"$harnessC`"" $hLog1 $hLog2
Pop-Location
if ($hExit -ne 0) { Get-Content $hLog2; throw "harness translation failed" }

$harnessO = Join-Path $RunDir "harness.o"
$hcLog1 = Join-Path $RunDir "harness_compile_stdout.log"
$hcLog2 = Join-Path $RunDir "harness_compile_stderr.log"
$hcExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" -c `"$harnessC`" -o `"$harnessO`"" $hcLog1 $hcLog2
if ($hcExit -ne 0) { Get-Content $hcLog2; throw "harness compile failed" }

$eventFifoO = Build-RealDep "mixa_event_fifo" "mixa_manager\mixa_event_fifo.lm1"
$backendTableO = Build-RealDep "mixa_backend_table" "mixa_manager\mixa_backend_table.lm1"
$backendHeadlessO = Build-RealDep "mixa_backend_headless" "mixa_manager\mixa_backend_headless.lm1"
$backendCtorsO = Build-RealDep "mixa_backend_ctors_headless" "mixa_manager\mixa_backend_ctors_headless.lm1"
$pumpO = Build-RealDep "mixa_pump" "mixa_manager\mixa_pump.lm1"
$consoleWindowO = Build-RealDep "mixa_console_window" "mixa_manager\mixa_console_window.lm1"
$fileWin32O = Build-RealDep "mixa_file_win32" "mixa_manager\mixa_file_win32.lm1"
$appPathO = Build-RealDep "mixa_app_path" "mixa_manager\mixa_app_path.lm1"
$helpO = Build-RealDep "mixa_help" "mixa_manager\mixa_help.lm1"
$cmdlineO = Build-RealDep "mixa_cmdline" "mixa_manager\mixa_cmdline.lm1"
$cmdlineDispatchO = Build-RealDep "mixa_cmdline_dispatch" "mixa_manager\mixa_cmdline_dispatch.lm1"
$processMarkerO = Build-RealDep "mixa_process_marker" "mixa_manager\mixa_process_marker.lm1"
$processWin32O = Build-RealDep "mixa_process_win32" "mixa_manager\mixa_process_win32.lm1"
$fmpanelO = Build-RealDep "mixa_app_fmpanel" "mixa_manager\mixa_app_fmpanel.lm1"
$highlightO = Build-RealDep "mixa_highlight" "mixa_manager\mixa_highlight.lm1"

$SharedDepsO = @($fmpanelO, $highlightO, $eventFifoO, $backendTableO, $backendHeadlessO, $backendCtorsO, $pumpO, $consoleWindowO, $fileWin32O, $appPathO, $helpO, $cmdlineO, $cmdlineDispatchO, $processMarkerO, $processWin32O)

# ---- Step 3: ALWAYS build + run the ORACLE-side harness. mixa_app_
# controller.lm1's own translation unit already embeds the full mixa_
# fm_copy.lm1 body (mixa_app_loop_*/mixa_fm_copy_here_action) via its
# own predef chain -- the 15 shared objects above supply every OTHER
# real dependency it calls but does not embed. ----
$oracleC = Join-Path $RunDir "mixa_app_controller_oracle.c"
$ocLog1 = Join-Path $RunDir "oracle_trans_stdout.log"
$ocLog2 = Join-Path $RunDir "oracle_trans_stderr.log"
Push-Location $RepoRoot
$ocExit = Invoke-Cmd $L1Trans "mixa_manager\mixa_app_controller.lm1 `"$oracleC`"" $ocLog1 $ocLog2
Pop-Location
if ($ocExit -ne 0) { Get-Content $ocLog2; throw "oracle mixa_app_controller.lm1 translation failed" }

$oracleO = Join-Path $RunDir "mixa_app_controller_oracle.o"
$occLog1 = Join-Path $RunDir "oracle_compile_stdout.log"
$occLog2 = Join-Path $RunDir "oracle_compile_stderr.log"
$occExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" -c `"$oracleC`" -o `"$oracleO`"" $occLog1 $occLog2
if ($occExit -ne 0) { Get-Content $occLog2; throw "oracle mixa_app_controller.lm1 compile failed" }

$oracleExe = Join-Path $RunDir "parity_oracle.exe"
$olLog1 = Join-Path $RunDir "oracle_link_stdout.log"
$olLog2 = Join-Path $RunDir "oracle_link_stderr.log"
$oracleLinkArgs = @("$GccStd", "-I", "`"$RepoRoot`"", "-I", "`"$RunDir\headers`"", "`"$harnessO`"", "`"$oracleO`"") + ($SharedDepsO | ForEach-Object { "`"$_`"" }) + @("-o", "`"$oracleExe`"", $LinkLibs)
$olExit = Invoke-Cmd "gcc" ($oracleLinkArgs -join " ") $olLog1 $olLog2
if ($olExit -ne 0) { Get-Content $olLog2; throw "oracle harness link failed" }

$OracleFixtureDir = Join-Path $RunDir "oracle_fixtures"
New-Item -ItemType Directory -Force -Path $OracleFixtureDir | Out-Null
Push-Location $RepoRoot
$oracleRunOut = Join-Path $RunDir "oracle_run_stdout.log"
$oracleRunErr = Join-Path $RunDir "oracle_run_stderr.log"
$oracleRunExit = Invoke-Cmd "`"$oracleExe`"" "`"$OracleFixtureDir`"" $oracleRunOut $oracleRunErr
Pop-Location
$OracleTrace = Get-Content -LiteralPath $oracleRunOut -Raw

# ---- Step 4: attempt the COMPLETE mixa_app_controller.lm2 translation. ----
Push-Location $RepoRoot
$acSrc = "mixa_manager\mixa_app_controller.lm2"
$acOut = Join-Path $RunDir "mixa_app_controller_l2.lm1"
$acStdout = Join-Path $RunDir "ac_stdout.log"
$acStderr = Join-Path $RunDir "ac_stderr.log"
$env:L2_RUNTIME_ROOT = "stg/l1_baseline/l2src/"
$AcExit = Invoke-Cmd "`"$l2exe`"" "`"$acSrc`" `"$acOut`"" $acStdout $acStderr
Pop-Location

$AcStdoutText = if (Test-Path -LiteralPath $acStdout) { Get-Content -LiteralPath $acStdout -Raw } else { "" }
$AcStderrText = if (Test-Path -LiteralPath $acStderr) { Get-Content -LiteralPath $acStderr -Raw } else { "" }
$AcSourceHash = (Get-FileHash -LiteralPath (Join-Path $RepoRoot $acSrc) -Algorithm SHA256).Hash
$RunnerHash = (Get-FileHash -LiteralPath $PSCommandPath -Algorithm SHA256).Hash

$Verdict = $null
$ExitCode = 1
$L2TraceText = ""
$DiffText = ""

# "unknown foreign type" covers BOTH this module's own first-encountered
# variant (a bare @: void RETURN type, at main_alloc -- a NEW trigger
# shape, isolated via a dedicated minimal .lm2 fixture, see the port
# note) and the already-familiar first-custom-struct-parameter variant
# every prior module this segment hit.
$KnownBarrier = ($AcStderrText -match "unknown foreign type") -or ($AcStderrText -match "incompatible entry signature")

if ($AcExit -ne 0 -and $KnownBarrier) {
    $Verdict = "EXPECTED_CORE_BARRIER"
    $ExitCode = 2
} elseif ($AcExit -ne 0) {
    $Verdict = "UNEXPECTED_FAILURE"
    $ExitCode = 1
} else {
    Push-Location $RepoRoot
    $l2AcC = Join-Path $RunDir "mixa_app_controller_l2.c"
    $l2ccLog1 = Join-Path $RunDir "l2ac_trans_stdout.log"
    $l2ccLog2 = Join-Path $RunDir "l2ac_trans_stderr.log"
    $l2ccExit = Invoke-Cmd $L1Trans "`"$acOut`" `"$l2AcC`"" $l2ccLog1 $l2ccLog2
    Pop-Location
    if ($l2ccExit -ne 0) {
        Get-Content $l2ccLog2
        $Verdict = "UNEXPECTED_FAILURE"
        $ExitCode = 1
    } else {
        $L2Rt = Add-L2RuntimeSupport -L1Trans $L1Trans -L1Root $L1Root -RunDir $RunDir -InvokeCmd $InvokeCmdRef
        $l2AcO = Join-Path $RunDir "mixa_app_controller_l2.o"
        $l2occLog1 = Join-Path $RunDir "l2ac_compile_stdout.log"
        $l2occLog2 = Join-Path $RunDir "l2ac_compile_stderr.log"
        $l2occExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" -I `"$($L2Rt.HeaderRoot)`" -I `"$($L2Rt.HeaderRoot)\stg\l1_baseline`" -I `"$L1Root`" -c `"$l2AcC`" -o `"$l2AcO`"" $l2occLog1 $l2occLog2
        if ($l2occExit -ne 0) {
            Get-Content $l2occLog2
            $Verdict = "UNEXPECTED_FAILURE"
            $ExitCode = 1
        } else {
            $l2Exe = Join-Path $RunDir "parity_l2.exe"
            $l2olLog1 = Join-Path $RunDir "l2ac_link_stdout.log"
            $l2olLog2 = Join-Path $RunDir "l2ac_link_stderr.log"
            $l2LinkArgs = @("$GccStd", "-I", "`"$RepoRoot`"", "-I", "`"$RunDir\headers`"", "`"$harnessO`"", "`"$l2AcO`"") + ($SharedDepsO | ForEach-Object { "`"$_`"" }) + @($L2Rt.ObjList, "-o", "`"$l2Exe`"", $LinkLibs)
            $l2olExit = Invoke-Cmd "gcc" ($l2LinkArgs -join " ") $l2olLog1 $l2olLog2
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
AppController-L2-Header: mixa_manager\mixa_app_controller_l2.h.lm1
AppController-L2-Source: $acSrc
AppController-L2-Source-Sha256: $AcSourceHash
Harness-Source-Sha256: $((Get-FileHash -LiteralPath (Join-Path $RepoRoot "mixa_manager\tests\mixa_app_controller_parity_harness.lm1") -Algorithm SHA256).Hash)
Runner-Sha256: $RunnerHash
Oracle-Run-Exit-Code: $oracleRunExit
Oracle-Trace:
$OracleTrace
AppController-Translate-Exit-Code: $AcExit
AppController-Translate-Stdout:
$AcStdoutText
AppController-Translate-Stderr:
$AcStderrText
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
        "EXPECTED_CORE_BARRIER: full mixa_app_controller.lm2 translation stopped at an already-known barrier. This is NOT a pass -- the integrated frontend is not yet on main. The oracle-side harness DID run (see Oracle-Trace above)."
    }
    "UNEXPECTED_FAILURE" {
        "UNEXPECTED_FAILURE: translation or build failed with something OTHER than the already-known barriers. Inspect the logs under $RunDir."
    }
    "PARITY_FAILURE" {
        "PARITY_FAILURE: both implementations built and ran, but their traces differ (see Diff above) or one exited non-zero."
    }
    "PASS" {
        "PASS: full mixa_app_controller.lm2 translated, built, and ran; its trace is byte-identical to the L1 oracle's own trace."
    }
}

exit $ExitCode
