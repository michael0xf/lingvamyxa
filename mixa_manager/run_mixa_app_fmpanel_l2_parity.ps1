# Real L1-vs-L2 parity gate for mixa_app_fmpanel (ticket
# 20260913-150500). Own dedicated ABI probe pair (mixa_app_fmpanel_abi_
# probe_real.c/_l2.c -- real side compiles against the genuine plain-C
# mixa_app_fmpanel_impl.h directly). This module predefs two REAL
# manager headers (mixa_file_manager.h.lm1, mixa_selection_walk.h.lm1),
# so the harness needs real mixa_file_manager.o (for mixa_fm_*) and
# real mixa_dir_win32.o (mixa_file_manager.lm1's own transitive
# dependency) plus mixa_draw.o (for mixa_rect_open/mixa_draw_fill, also
# transitively containing mixa_text_rect.lm1's own real body) and mixa_
# highlight.o (for mixa_highlight_init/set/clear/release, called
# directly by mixa_app_fmpanel itself) linked alongside the harness and
# the oracle/L2 object.
#   0. ABI parity gate: dedicated MixaAppFmPanel probe pair.
#   1. ALWAYS builds and runs the ORACLE-side harness (real temp-
#      directory-backed MixaFm, real MixaTextRect, real file deletion).
#   2. Attempts to translate the COMPLETE mixa_app_fmpanel.lm2.
#      - fails with an already-known barrier -> EXPECTED_CORE_BARRIER
#        (exit 2). NOT a pass.
#      - fails with any OTHER diagnostic -> UNEXPECTED_FAILURE (exit 1).
#      - SUCCEEDS -> builds/links/runs the L2-side harness and diffs its
#        stdout against the oracle trace byte-for-byte. PASS (exit 0)
#        only on an exact match, else PARITY_FAILURE (exit 1).
#
# mixa_app_fmpanel.h/_impl.h/.lm1 and its real dependencies (mixa_file_
# manager.h.lm1/.lm1, mixa_selection_walk.h.lm1/.lm1, mixa_dir.h.lm1/
# mixa_dir_win32.h.lm1/.lm1, mixa_draw.h/.lm1, mixa_highlight.h/.lm1,
# mixa_selection.h/.lm1) are the parity oracle (or real, unmodified
# dependencies) and are never touched. Nothing under stg/l1_baseline is
# modified, only read. Every input is built fresh in a unique run
# directory -- no stale objects.
param(
    [string]$L2TranslatorPath = "",
    [string]$L2L1TranslatorPath = ""
)
$ErrorActionPreference = "Stop"

$RepoRoot = Split-Path -Parent $PSScriptRoot
$L1Root = Join-Path $RepoRoot "stg\l1_baseline"
$L1Trans = Join-Path $L1Root "build\l1trans\gen2\l1trans.exe"
. (Join-Path $PSScriptRoot "lib_l2_runtime_support.ps1")
$ExpectedL1Hash = Get-L1Pin -L1Root $L1Root

$ActualL1Hash = Assert-PinnedL1Translator -L1Trans $L1Trans -L1Root $L1Root
$L2L1Trans = if ($L2L1TranslatorPath) {
    (Resolve-Path -LiteralPath $L2L1TranslatorPath).Path
} else {
    (Resolve-Path -LiteralPath (Join-Path $RepoRoot "build\l1trans\gen3\l1trans.exe")).Path
}
$L2L1TransHash = (Get-FileHash -LiteralPath $L2L1Trans -Algorithm SHA256).Hash

$guards = @(
    "-Werror=incompatible-pointer-types", "-Werror=discarded-qualifiers",
    "-Werror=implicit-function-declaration", "-Werror=implicit-int"
)
$GccStd = "-std=c99 -Wall -Wextra -Wpedantic $($guards -join ' ')"
$LinkLibs = "-lkernel32"

$RunTimestamp = (Get-Date -Format "yyyyMMdd_HHmmss_fff")
$RunGuid = [GUID]::NewGuid().ToString().Substring(0, 8)
$BaseDir = Join-Path $RepoRoot "build\mixa\claude\mixa_app_fmpanel_l2_parity"
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

# ---- Step 0: translate the L2 header and its own real predef chain. ----
Invoke-HeaderTrans "mixa_manager\mixa_app_fmpanel_l2.h.lm1" "mixa_app_fmpanel_l2.lm1.h"
Invoke-HeaderTrans "mixa_manager\mixa_file_manager.h.lm1" "mixa_file_manager.lm1.h"
Invoke-HeaderTrans "mixa_manager\mixa_selection_walk.h.lm1" "mixa_selection_walk.lm1.h"
Invoke-HeaderTrans "mixa_manager\mixa_highlight_l2.h.lm1" "mixa_highlight_l2.lm1.h"
Invoke-HeaderTrans "mixa_manager\mixa_tiles_l2.h.lm1" "mixa_tiles_l2.lm1.h"
Invoke-HeaderTrans "mixa_manager\mixa_dir.h.lm1" "mixa_dir.lm1.h"
Invoke-HeaderTrans "mixa_manager\mixa_dir_win32.h.lm1" "mixa_dir_win32.lm1.h"
Invoke-HeaderTrans "mixa_manager\mixa_fm_copy.h.lm1" "mixa_fm_copy.lm1.h"
Invoke-HeaderTrans "mixa_manager\mixa_copy.h.lm1" "mixa_copy.lm1.h"
Invoke-HeaderTrans "mixa_manager\mixa_fileio_win32.h.lm1" "mixa_fileio_win32.lm1.h"
Invoke-HeaderTrans "mixa_manager\mixa_fileio.h.lm1" "mixa_fileio.lm1.h"

# ---- Step 0.5: ABI parity gate (dedicated probe pair). ----
$abiRealExe = Join-Path $RunDir "abi_probe_real.exe"
$abiRealLog1 = Join-Path $RunDir "abi_probe_real_compile_stdout.log"
$abiRealLog2 = Join-Path $RunDir "abi_probe_real_compile_stderr.log"
$abiRealExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" `"$RepoRoot\mixa_manager\tests\mixa_app_fmpanel_abi_probe_real.c`" -o `"$abiRealExe`"" $abiRealLog1 $abiRealLog2
if ($abiRealExit -ne 0) { Get-Content $abiRealLog2; throw "ABI probe (real header) compile failed" }

$abiL2Exe = Join-Path $RunDir "abi_probe_l2.exe"
$abiL2Log1 = Join-Path $RunDir "abi_probe_l2_compile_stdout.log"
$abiL2Log2 = Join-Path $RunDir "abi_probe_l2_compile_stderr.log"
$abiL2Exit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" `"$RepoRoot\mixa_manager\tests\mixa_app_fmpanel_abi_probe_l2.c`" -o `"$abiL2Exe`"" $abiL2Log1 $abiL2Log2
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

# ---- Step 1: select a verified l2trans.exe or build one fresh. ----
$L2TransSourceRel = "l2src\l2trans.lm1"
$L2TransSource = Join-Path $L1Root $L2TransSourceRel
if (-not (Test-Path -LiteralPath $L2TransSource)) {
    throw "missing L2 frontend source (read-only, not modified by this script): $L2TransSource"
}
$L2TransSourceHash = (Get-FileHash -LiteralPath $L2TransSource -Algorithm SHA256).Hash

$l2exe = if ($L2TranslatorPath) {
    (Resolve-Path -LiteralPath $L2TranslatorPath).Path
} else {
    Join-Path $RunDir "l2trans.exe"
}
if (-not $L2TranslatorPath) {
    Push-Location $L1Root
    try {
        $l2c = Join-Path $RunDir "l2trans.c"
        & $L2L1Trans $L2TransSourceRel $l2c
        if ($LASTEXITCODE -ne 0) { throw "l1trans failed translating l2trans.lm1 itself" }
        $gccBuildLog = Join-Path $RunDir "l2trans_build_stderr.log"
        $gccArgsStr = "$GccStd -I . -I lm1\build `"$l2c`" -o `"$l2exe`""
        $rc = Invoke-Cmd "gcc" $gccArgsStr (Join-Path $RunDir "l2trans_build_stdout.log") $gccBuildLog
        if ($rc -ne 0) { Get-Content $gccBuildLog; throw "gcc failed building l2trans.exe" }
    } finally {
        Pop-Location
    }
}
$L2TransExeHash = (Get-FileHash -LiteralPath $l2exe -Algorithm SHA256).Hash

# ---- Step 2: build the harness object + the real dependency objects. ----
Push-Location $RepoRoot
$harnessC = Join-Path $RunDir "harness.c"
$hLog1 = Join-Path $RunDir "harness_trans_stdout.log"
$hLog2 = Join-Path $RunDir "harness_trans_stderr.log"
$hExit = Invoke-Cmd $L1Trans "mixa_manager\tests\mixa_app_fmpanel_parity_harness.lm1 `"$harnessC`"" $hLog1 $hLog2
Pop-Location
if ($hExit -ne 0) { Get-Content $hLog2; throw "harness translation failed" }

$harnessO = Join-Path $RunDir "harness.o"
$hcLog1 = Join-Path $RunDir "harness_compile_stdout.log"
$hcLog2 = Join-Path $RunDir "harness_compile_stderr.log"
$hcExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" -c `"$harnessC`" -o `"$harnessO`"" $hcLog1 $hcLog2
if ($hcExit -ne 0) { Get-Content $hcLog2; throw "harness compile failed" }

# mixa_fm_copy.lm1 predefs the FULL body of mixa_file_manager.lm1 and
# mixa_copy.lm1 (which itself predefs the full body of mixa_selection_
# walk.lm1); mixa_file_manager.lm1 in turn predefs the full bodies of
# mixa_dir_win32.lm1 and mixa_selection.lm1. l1trans dedupes a predef
# chain within ONE translation unit, so building mixa_fm_copy.lm1 alone
# yields a SINGLE object containing mixa_fm_*, mixa_dir_*, mixa_
# selection_*, mixa_selection_walk, and mixa_copy_* exactly once --
# this mirrors how the real mixa_app_controller.lm1 resolves the exact
# same symbol set (via its own predef of mixa_fm_copy.lm1) in the
# existing run_app_fmpanel_e2e_selftest.ps1, which never separately
# compiles mixa_file_manager.lm1/mixa_dir_win32.lm1/mixa_selection_
# walk.lm1 as their own objects for exactly this reason -- doing so
# would duplicate-define these symbols across multiple objects.
$fmCopyO = Build-RealDep "mixa_fm_copy" "mixa_manager\mixa_fm_copy.lm1"
$drawO = Build-RealDep "mixa_draw" "mixa_manager\mixa_draw.lm1"
$highlightO = Build-RealDep "mixa_highlight" "mixa_manager\mixa_highlight.lm1"
# mixa_fm_copy.lm1 itself drives copy progress through the real
# mixa_pump_*/mixa_backend_* seam (same as mixa_app_controller.lm1's
# own real link list in run_app_fmpanel_e2e_selftest.ps1).
$pumpO = Build-RealDep "mixa_pump" "mixa_manager\mixa_pump.lm1"
$backendTableO = Build-RealDep "mixa_backend_table" "mixa_manager\mixa_backend_table.lm1"
$backendHeadlessO = Build-RealDep "mixa_backend_headless" "mixa_manager\mixa_backend_headless.lm1"
$backendCtorsO = Build-RealDep "mixa_backend_ctors_headless" "mixa_manager\mixa_backend_ctors_headless.lm1"
$eventFifoO = Build-RealDep "mixa_event_fifo" "mixa_manager\mixa_event_fifo.lm1"

# ---- Step 3: ALWAYS build + run the ORACLE-side harness. ----
$oracleC = Join-Path $RunDir "mixa_app_fmpanel_oracle.c"
$ocLog1 = Join-Path $RunDir "oracle_trans_stdout.log"
$ocLog2 = Join-Path $RunDir "oracle_trans_stderr.log"
Push-Location $RepoRoot
$ocExit = Invoke-Cmd $L1Trans "mixa_manager\mixa_app_fmpanel.lm1 `"$oracleC`"" $ocLog1 $ocLog2
Pop-Location
if ($ocExit -ne 0) { Get-Content $ocLog2; throw "oracle mixa_app_fmpanel.lm1 translation failed" }

$oracleO = Join-Path $RunDir "mixa_app_fmpanel_oracle.o"
$occLog1 = Join-Path $RunDir "oracle_compile_stdout.log"
$occLog2 = Join-Path $RunDir "oracle_compile_stderr.log"
$occExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" -c `"$oracleC`" -o `"$oracleO`"" $occLog1 $occLog2
if ($occExit -ne 0) { Get-Content $occLog2; throw "oracle mixa_app_fmpanel.lm1 compile failed" }

$oracleExe = Join-Path $RunDir "parity_oracle.exe"
$olLog1 = Join-Path $RunDir "oracle_link_stdout.log"
$olLog2 = Join-Path $RunDir "oracle_link_stderr.log"
$olExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" `"$harnessO`" `"$oracleO`" `"$fmCopyO`" `"$drawO`" `"$highlightO`" `"$pumpO`" `"$backendTableO`" `"$backendHeadlessO`" `"$backendCtorsO`" `"$eventFifoO`" -o `"$oracleExe`" $LinkLibs" $olLog1 $olLog2
if ($olExit -ne 0) { Get-Content $olLog2; throw "oracle harness link failed" }

$OracleFixtureDir = Join-Path $RunDir "oracle_fixtures"
New-Item -ItemType Directory -Force -Path $OracleFixtureDir | Out-Null
Push-Location $RepoRoot
$oracleRunOut = Join-Path $RunDir "oracle_run_stdout.log"
$oracleRunErr = Join-Path $RunDir "oracle_run_stderr.log"
$oracleRunExit = Invoke-Cmd "`"$oracleExe`"" "`"$OracleFixtureDir`"" $oracleRunOut $oracleRunErr
Pop-Location
$OracleTrace = Get-Content -LiteralPath $oracleRunOut -Raw

# ---- Step 4: attempt the COMPLETE mixa_app_fmpanel.lm2 translation. ----
Push-Location $RepoRoot
$fpSrc = "mixa_manager\mixa_app_fmpanel.lm2"
$fpOut = Join-Path $RunDir "mixa_app_fmpanel_l2.lm1"
$fpStdout = Join-Path $RunDir "fp_stdout.log"
$fpStderr = Join-Path $RunDir "fp_stderr.log"
$FpExit = Invoke-Cmd "`"$l2exe`"" "`"$fpSrc`" `"$fpOut`"" $fpStdout $fpStderr
Pop-Location

$FpStdoutText = if (Test-Path -LiteralPath $fpStdout) { Get-Content -LiteralPath $fpStdout -Raw } else { "" }
$FpStderrText = if (Test-Path -LiteralPath $fpStderr) { Get-Content -LiteralPath $fpStderr -Raw } else { "" }
$FpSourceHash = (Get-FileHash -LiteralPath (Join-Path $RepoRoot $fpSrc) -Algorithm SHA256).Hash
$RunnerHash = (Get-FileHash -LiteralPath $PSCommandPath -Algorithm SHA256).Hash

$Verdict = $null
$ExitCode = 1
$L2TraceText = ""
$DiffText = ""

$KnownBarrier = ($FpStderrText -match "unknown foreign type") -or ($FpStderrText -match "incompatible entry signature")

if ($FpExit -ne 0 -and $KnownBarrier) {
    $Verdict = "EXPECTED_CORE_BARRIER"
    $ExitCode = 2
} elseif ($FpExit -ne 0) {
    $Verdict = "UNEXPECTED_FAILURE"
    $ExitCode = 1
} else {
    # Generated L1 imports both the staged core (`l2src/...`) and manager
    # headers (`mixa_manager/...`). Give the translator one disposable root
    # containing directory junctions to both trees.
    $l2ImportLink = Join-Path $RunDir 'l2src'
    $managerImportLink = Join-Path $RunDir 'mixa_manager'
    if (-not (Test-Path -LiteralPath $l2ImportLink)) {
        New-Item -ItemType Junction -Path $l2ImportLink -Target (Join-Path $L1Root 'l2src') | Out-Null
    }
    if (-not (Test-Path -LiteralPath $managerImportLink)) {
        New-Item -ItemType Junction -Path $managerImportLink -Target (Join-Path $RepoRoot 'mixa_manager') | Out-Null
    }
    Push-Location $RunDir
    $l2FpC = Join-Path $RunDir "mixa_app_fmpanel_l2.c"
    $l2ccLog1 = Join-Path $RunDir "l2fp_trans_stdout.log"
    $l2ccLog2 = Join-Path $RunDir "l2fp_trans_stderr.log"
    $l2ccExit = Invoke-Cmd $L2L1Trans "`"$fpOut`" `"$l2FpC`"" $l2ccLog1 $l2ccLog2
    Pop-Location
    if ($l2ccExit -ne 0) {
        Get-Content $l2ccLog2
        $Verdict = "UNEXPECTED_FAILURE"
        $ExitCode = 1
    } else {
        $coreHeaderDir = Join-Path $RunDir 'headers\l2src'
        New-Item -ItemType Directory -Force -Path $coreHeaderDir | Out-Null
        $coreNames = @(
            'lmx_msg_blocks', 'lmx_owned_ranges', 'lmx_msg_storage',
            'lmx_msg_path_storage', 'lmx_msg_slots', 'lmx_msg_mail_chain',
            'lmx_msg_sched_ready', 'lmx_msg_visit', 'lmx_msg_liveness',
            'lmx_msg_history_owned', 'lmx_msg_roots_stale',
            'lmx_branch_owned', 'lmx_value_owned', 'lmx_chars_owned',
            'lmx_array_owned', 'lmx_array_ref_owned', 'lmx_graph_copy_owned',
            'lmx_merge_owned', 'lmx_message_graph_copy')
        foreach ($coreHeader in $coreNames) {
            $coreHeaderOut = Join-Path $coreHeaderDir ($coreHeader + '.lm1.h')
            $coreHeaderStdout = Join-Path $RunDir ($coreHeader + '_header_stdout.log')
            $coreHeaderStderr = Join-Path $RunDir ($coreHeader + '_header_stderr.log')
            Push-Location $RunDir
            $coreHeaderExit = Invoke-Cmd $L2L1Trans "`"l2src\$coreHeader.h.lm1`" `"$coreHeaderOut`"" $coreHeaderStdout $coreHeaderStderr
            Pop-Location
            if ($coreHeaderExit -ne 0) {
                Get-Content $coreHeaderStderr
                throw "core header translation failed: $coreHeader"
            }
        }
        # A generated library owns a real Message graph, so its parity executable
        # must link the same runtime object set as the graph ABI gate.
        $coreObjectDir = Join-Path $RunDir 'core_objects'
        New-Item -ItemType Directory -Force -Path $coreObjectDir | Out-Null
        $coreObjects = @()
        foreach ($coreName in $coreNames + @('lmx_message')) {
            $coreSource = Join-Path $coreObjectDir ($coreName + '.c')
            $coreObject = Join-Path $coreObjectDir ($coreName + '.o')
            $coreSourceStdout = Join-Path $RunDir ($coreName + '_source_stdout.log')
            $coreSourceStderr = Join-Path $RunDir ($coreName + '_source_stderr.log')
            Push-Location $RunDir
            $coreSourceExit = Invoke-Cmd $L2L1Trans "`"l2src\$coreName.lm1`" `"$coreSource`"" $coreSourceStdout $coreSourceStderr
            Pop-Location
            if ($coreSourceExit -ne 0) {
                Get-Content $coreSourceStderr
                throw "core source translation failed: $coreName"
            }
            $coreCompileStdout = Join-Path $RunDir ($coreName + '_compile_stdout.log')
            $coreCompileStderr = Join-Path $RunDir ($coreName + '_compile_stderr.log')
            $coreCompileExit = Invoke-Cmd 'gcc' "$GccStd -I `"$L1Root`" -I `"$RunDir\headers`" -c `"$coreSource`" -o `"$coreObject`"" $coreCompileStdout $coreCompileStderr
            if ($coreCompileExit -ne 0) {
                Get-Content $coreCompileStderr
                throw "core source compile failed: $coreName"
            }
            $coreObjects += $coreObject
        }
        foreach ($coreCName in @('lmx_message_host', 'lmx_message_exec')) {
            $coreCSource = Join-Path $L1Root ("l2src\$coreCName.c")
            $coreCObject = Join-Path $coreObjectDir ($coreCName + '.o')
            $coreCStdout = Join-Path $RunDir ($coreCName + '_compile_stdout.log')
            $coreCStderr = Join-Path $RunDir ($coreCName + '_compile_stderr.log')
            $coreCExit = Invoke-Cmd 'gcc' "$GccStd -I `"$L1Root`" -I `"$RunDir\headers`" -c `"$coreCSource`" -o `"$coreCObject`"" $coreCStdout $coreCStderr
            if ($coreCExit -ne 0) {
                Get-Content $coreCStderr
                throw "core C compile failed: $coreCName"
            }
            $coreObjects += $coreCObject
        }
        $coreObjectArgs = ($coreObjects | ForEach-Object { '"' + $_ + '"' }) -join ' '
        $l2FpO = Join-Path $RunDir "mixa_app_fmpanel_l2.o"
        $l2occLog1 = Join-Path $RunDir "l2fp_compile_stdout.log"
        $l2occLog2 = Join-Path $RunDir "l2fp_compile_stderr.log"
        $l2occExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$L1Root`" -I `"$RunDir\headers`" -c `"$l2FpC`" -o `"$l2FpO`"" $l2occLog1 $l2occLog2
        if ($l2occExit -ne 0) {
            Get-Content $l2occLog2
            $Verdict = "UNEXPECTED_FAILURE"
            $ExitCode = 1
        } else {
            $l2Exe = Join-Path $RunDir "parity_l2.exe"
            $l2olLog1 = Join-Path $RunDir "l2fp_link_stdout.log"
            $l2olLog2 = Join-Path $RunDir "l2fp_link_stderr.log"
            $l2olExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" `"$harnessO`" `"$l2FpO`" `"$fmCopyO`" `"$drawO`" `"$highlightO`" `"$pumpO`" `"$backendTableO`" `"$backendHeadlessO`" `"$backendCtorsO`" `"$eventFifoO`" $coreObjectArgs -o `"$l2Exe`" $LinkLibs" $l2olLog1 $l2olLog2
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
L2-output-L1-Translator: $L2L1Trans
L2-output-L1-Translator-Sha256: $L2L1TransHash
L2trans-Source (informational, not pinned): $L2TransSource
L2trans-Source-Sha256 (informational, not pinned): $L2TransSourceHash
L2trans-Exe-Sha256 (rebuilt fresh this run, not a stable artifact): $L2TransExeHash
AppFmPanel-L2-Header: mixa_manager\mixa_app_fmpanel_l2.h.lm1
AppFmPanel-L2-Source: $fpSrc
AppFmPanel-L2-Source-Sha256: $FpSourceHash
Harness-Source-Sha256: $((Get-FileHash -LiteralPath (Join-Path $RepoRoot "mixa_manager\tests\mixa_app_fmpanel_parity_harness.lm1") -Algorithm SHA256).Hash)
Runner-Sha256: $RunnerHash
Oracle-Run-Exit-Code: $oracleRunExit
Oracle-Trace:
$OracleTrace
AppFmPanel-Translate-Exit-Code: $FpExit
AppFmPanel-Translate-Stdout:
$FpStdoutText
AppFmPanel-Translate-Stderr:
$FpStderrText
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
        "EXPECTED_CORE_BARRIER: full mixa_app_fmpanel.lm2 translation stopped at an already-known barrier. This is NOT a pass -- the integrated frontend is not yet on main. The oracle-side harness DID run (see Oracle-Trace above)."
    }
    "UNEXPECTED_FAILURE" {
        "UNEXPECTED_FAILURE: translation or build failed with something OTHER than the already-known barriers. Inspect the logs under $RunDir."
    }
    "PARITY_FAILURE" {
        "PARITY_FAILURE: both implementations built and ran, but their traces differ (see Diff above) or one exited non-zero."
    }
    "PASS" {
        "PASS: full mixa_app_fmpanel.lm2 translated, built, and ran; its trace is byte-identical to the L1 oracle's own trace."
    }
}

exit $ExitCode
