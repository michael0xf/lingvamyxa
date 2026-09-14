# Real L1-vs-L2 parity gate for mixa_button_dispatch (ticket
# 20260913-105000). Same structure as run_mixa_highlight_l2_parity.ps1
# (104000). Reuses mixa_tiles_l2.h.lm1 (chained via mixa_button_
# dispatch_l2.h.lm1) and the tiles ABI probe pair.
#   0. ABI parity gate: reuses mixa_tiles_abi_probe_real.c/_l2.c (the
#      MixaButton/MixaButtonLine/MixaButtonPanel/MixaButtonAction/
#      MixaAppWindow/MixaAppWindowStack chain has no byte-sized fields
#      of its own -- every field is int/size_t/unsigned/pointer -- so no
#      dedicated probe pair was written for it; its layout was verified
#      directly via a one-off sizeof/offsetof check before writing the
#      source, recorded in mixa_button_dispatch_l2_port.txt).
#   1. ALWAYS builds and runs the ORACLE-side harness.
#   2. Attempts to translate the COMPLETE mixa_button_dispatch.lm2.
#      - fails with an already-known barrier -> EXPECTED_CORE_BARRIER
#        (exit 2). NOT a pass.
#      - fails with any OTHER diagnostic -> UNEXPECTED_FAILURE (exit 1).
#      - SUCCEEDS -> builds/links/runs the L2-side harness (linking the
#        L2 button_dispatch object + the harness + the REAL, UNCHANGED
#        mixa_app_window.o -- which itself already contains mixa_
#        buttons.lm1's/mixa_draw.lm1's/mixa_text_rect.lm1's own real
#        bodies via its own predef chain, so it alone supplies every
#        real function this module calls) and diffs its stdout against
#        the oracle trace byte-for-byte. PASS (exit 0) only on an exact
#        match, else PARITY_FAILURE (exit 1).
#
# mixa_button_dispatch.lm1/mixa_buttons.h/mixa_app_window.lm1/mixa_core.
# h/mixa_draw.lm1/mixa_text_rect.lm1 are the parity oracle and are never
# touched. Nothing under stg/l1_baseline is modified, only read.
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
$BaseDir = Join-Path $RepoRoot "build\mixa\claude\mixa_button_dispatch_l2_parity"
$RunDir = Join-Path $BaseDir "run_${RunTimestamp}_${RunGuid}"
$HeaderDir = Join-Path $RunDir "headers\mixa_manager"
New-Item -ItemType Directory -Force -Path $HeaderDir | Out-Null

function Invoke-Cmd([string]$exe, [string]$argsStr, [string]$outLog, [string]$errLog) {
    & cmd /c "$exe $argsStr > `"$outLog`" 2> `"$errLog`""
    return $LASTEXITCODE
}

# ---- Step 0: translate the shared header-units once (tiles, chained,
# then this module's own). ----
Push-Location $RepoRoot
$hdrLog1 = Join-Path $RunDir "header_trans_stdout.log"
$hdrLog2 = Join-Path $RunDir "header_trans_stderr.log"
$tilesHdrOut = Join-Path $HeaderDir "mixa_tiles_l2.lm1.h"
$hdrExit = Invoke-Cmd $L1Trans "mixa_manager\mixa_tiles_l2.h.lm1 `"$tilesHdrOut`"" $hdrLog1 $hdrLog2
if ($hdrExit -ne 0) { Pop-Location; throw "tiles header translation failed: see $hdrLog2" }
$bdHdrOut = Join-Path $HeaderDir "mixa_button_dispatch_l2.lm1.h"
$hdrExit2 = Invoke-Cmd $L1Trans "mixa_manager\mixa_button_dispatch_l2.h.lm1 `"$bdHdrOut`"" $hdrLog1 $hdrLog2
Pop-Location
if ($hdrExit2 -ne 0) { throw "button_dispatch header translation failed: see $hdrLog2" }

# ---- Step 0.5: ABI parity gate (reused tiles probes). ----
$abiRealExe = Join-Path $RunDir "abi_probe_real.exe"
$abiRealLog1 = Join-Path $RunDir "abi_probe_real_compile_stdout.log"
$abiRealLog2 = Join-Path $RunDir "abi_probe_real_compile_stderr.log"
$abiRealExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" `"$RepoRoot\mixa_manager\tests\mixa_tiles_abi_probe_real.c`" -o `"$abiRealExe`"" $abiRealLog1 $abiRealLog2
if ($abiRealExit -ne 0) { Get-Content $abiRealLog2; throw "ABI probe (real headers) compile failed" }

$abiL2Exe = Join-Path $RunDir "abi_probe_l2.exe"
$abiL2Log1 = Join-Path $RunDir "abi_probe_l2_compile_stdout.log"
$abiL2Log2 = Join-Path $RunDir "abi_probe_l2_compile_stderr.log"
$abiL2Exit = Invoke-Cmd "gcc" "$GccStd -I `"$RunDir\headers`" `"$RepoRoot\mixa_manager\tests\mixa_tiles_abi_probe_l2.c`" -o `"$abiL2Exe`"" $abiL2Log1 $abiL2Log2
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

# ---- Step 2: build the harness object + the real mixa_app_window.o
# (which already contains mixa_buttons.lm1's/mixa_draw.lm1's/mixa_text_
# rect.lm1's own real bodies via its own predef chain). ----
Push-Location $RepoRoot
$harnessC = Join-Path $RunDir "harness.c"
$hLog1 = Join-Path $RunDir "harness_trans_stdout.log"
$hLog2 = Join-Path $RunDir "harness_trans_stderr.log"
$hExit = Invoke-Cmd $L1Trans "mixa_manager\tests\mixa_button_dispatch_parity_harness.lm1 `"$harnessC`"" $hLog1 $hLog2
Pop-Location
if ($hExit -ne 0) { Get-Content $hLog2; throw "harness translation failed" }

$harnessO = Join-Path $RunDir "harness.o"
$hcLog1 = Join-Path $RunDir "harness_compile_stdout.log"
$hcLog2 = Join-Path $RunDir "harness_compile_stderr.log"
$hcExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" -c `"$harnessC`" -o `"$harnessO`"" $hcLog1 $hcLog2
if ($hcExit -ne 0) { Get-Content $hcLog2; throw "harness compile failed" }

# ---- Step 2.5: translate the ORACLE-side predef headers (mixa_button_
# dispatch.h.lm1 -> mixa_button_dispatch.lm1.h, mixa_app_window.h.lm1 ->
# mixa_app_window.lm1.h) into their own dir -- a DIFFERENT, unrelated
# header set from the L2-specific "headers/" one above (different
# filenames, real production field layouts, no MixaCell/MixaTextRect
# redefinition risk since these headers reach them via plain C
# `include:`, not predef:). Needed by BOTH the mixa_app_window.o compile
# below (its own generated .c includes mixa_app_window.lm1.h directly)
# and the oracle mixa_button_dispatch.lm1 compile further down. ----
$OracleHeaderDir = Join-Path $RunDir "oracle_headers\mixa_manager"
New-Item -ItemType Directory -Force -Path $OracleHeaderDir | Out-Null
Push-Location $RepoRoot
$bdHdrOut2 = Join-Path $OracleHeaderDir "mixa_button_dispatch.lm1.h"
$ohExit1 = Invoke-Cmd $L1Trans "mixa_manager\mixa_button_dispatch.h.lm1 `"$bdHdrOut2`"" $hdrLog1 $hdrLog2
if ($ohExit1 -ne 0) { Pop-Location; throw "oracle button_dispatch header translation failed: see $hdrLog2" }
$awHdrOut = Join-Path $OracleHeaderDir "mixa_app_window.lm1.h"
$ohExit2 = Invoke-Cmd $L1Trans "mixa_manager\mixa_app_window.h.lm1 `"$awHdrOut`"" $hdrLog1 $hdrLog2
Pop-Location
if ($ohExit2 -ne 0) { throw "oracle app_window header translation failed: see $hdrLog2" }

Push-Location $RepoRoot
$awC = Join-Path $RunDir "mixa_app_window.c"
$awLog1 = Join-Path $RunDir "aw_trans_stdout.log"
$awLog2 = Join-Path $RunDir "aw_trans_stderr.log"
$awExit = Invoke-Cmd $L1Trans "mixa_manager\mixa_app_window.lm1 `"$awC`"" $awLog1 $awLog2
Pop-Location
if ($awExit -ne 0) { Get-Content $awLog2; throw "mixa_app_window.lm1 translation failed" }

$awO = Join-Path $RunDir "mixa_app_window.o"
$awcLog1 = Join-Path $RunDir "aw_compile_stdout.log"
$awcLog2 = Join-Path $RunDir "aw_compile_stderr.log"
$awcExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\oracle_headers`" -c `"$awC`" -o `"$awO`"" $awcLog1 $awcLog2
if ($awcExit -ne 0) { Get-Content $awcLog2; throw "mixa_app_window.lm1 compile failed" }

# ---- Step 3: ALWAYS build + run the ORACLE-side harness. ----
$oracleC = Join-Path $RunDir "mixa_button_dispatch_oracle.c"
$ocLog1 = Join-Path $RunDir "oracle_trans_stdout.log"
$ocLog2 = Join-Path $RunDir "oracle_trans_stderr.log"
Push-Location $RepoRoot
$ocExit = Invoke-Cmd $L1Trans "mixa_manager\mixa_button_dispatch.lm1 `"$oracleC`"" $ocLog1 $ocLog2
Pop-Location
if ($ocExit -ne 0) { Get-Content $ocLog2; throw "oracle mixa_button_dispatch.lm1 translation failed" }

$oracleO = Join-Path $RunDir "mixa_button_dispatch_oracle.o"
$occLog1 = Join-Path $RunDir "oracle_compile_stdout.log"
$occLog2 = Join-Path $RunDir "oracle_compile_stderr.log"
$occExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\oracle_headers`" -c `"$oracleC`" -o `"$oracleO`"" $occLog1 $occLog2
if ($occExit -ne 0) { Get-Content $occLog2; throw "oracle mixa_button_dispatch.lm1 compile failed" }

$oracleExe = Join-Path $RunDir "parity_oracle.exe"
$olLog1 = Join-Path $RunDir "oracle_link_stdout.log"
$olLog2 = Join-Path $RunDir "oracle_link_stderr.log"
$olExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" `"$harnessO`" `"$oracleO`" -o `"$oracleExe`"" $olLog1 $olLog2
if ($olExit -ne 0) { Get-Content $olLog2; throw "oracle harness link failed" }

$oracleRunOut = Join-Path $RunDir "oracle_run_stdout.log"
$oracleRunErr = Join-Path $RunDir "oracle_run_stderr.log"
$oracleRunExit = Invoke-Cmd "`"$oracleExe`"" "" $oracleRunOut $oracleRunErr
$OracleTrace = Get-Content -LiteralPath $oracleRunOut -Raw

# ---- Step 4: attempt the COMPLETE mixa_button_dispatch.lm2 translation. ----
Push-Location $RepoRoot
$bdSrc = "mixa_manager\mixa_button_dispatch.lm2"
$bdOut = Join-Path $RunDir "mixa_button_dispatch_l2.lm1"
$bdStdout = Join-Path $RunDir "bd_stdout.log"
$bdStderr = Join-Path $RunDir "bd_stderr.log"
$BdExit = Invoke-Cmd "`"$l2exe`"" "`"$bdSrc`" `"$bdOut`"" $bdStdout $bdStderr
Pop-Location

$BdStdoutText = if (Test-Path -LiteralPath $bdStdout) { Get-Content -LiteralPath $bdStdout -Raw } else { "" }
$BdStderrText = if (Test-Path -LiteralPath $bdStderr) { Get-Content -LiteralPath $bdStderr -Raw } else { "" }
$BdSourceHash = (Get-FileHash -LiteralPath (Join-Path $RepoRoot $bdSrc) -Algorithm SHA256).Hash
$RunnerHash = (Get-FileHash -LiteralPath $PSCommandPath -Algorithm SHA256).Hash

$Verdict = $null
$ExitCode = 1
$L2TraceText = ""
$DiffText = ""

$KnownBarrier = ($BdStderrText -match "unknown foreign type") -or ($BdStderrText -match "incompatible entry signature")

if ($BdExit -ne 0 -and $KnownBarrier) {
    $Verdict = "EXPECTED_CORE_BARRIER"
    $ExitCode = 2
} elseif ($BdExit -ne 0) {
    $Verdict = "UNEXPECTED_FAILURE"
    $ExitCode = 1
} else {
    Push-Location $RepoRoot
    $l2BdC = Join-Path $RunDir "mixa_button_dispatch_l2.c"
    $l2bcLog1 = Join-Path $RunDir "l2bd_trans_stdout.log"
    $l2bcLog2 = Join-Path $RunDir "l2bd_trans_stderr.log"
    $l2bcExit = Invoke-Cmd $L1Trans "`"$bdOut`" `"$l2BdC`"" $l2bcLog1 $l2bcLog2
    Pop-Location
    if ($l2bcExit -ne 0) {
        Get-Content $l2bcLog2
        $Verdict = "UNEXPECTED_FAILURE"
        $ExitCode = 1
    } else {
        $l2BdO = Join-Path $RunDir "mixa_button_dispatch_l2.o"
        $l2occLog1 = Join-Path $RunDir "l2bd_compile_stdout.log"
        $l2occLog2 = Join-Path $RunDir "l2bd_compile_stderr.log"
        $l2occExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" -c `"$l2BdC`" -o `"$l2BdO`"" $l2occLog1 $l2occLog2
        if ($l2occExit -ne 0) {
            Get-Content $l2occLog2
            $Verdict = "UNEXPECTED_FAILURE"
            $ExitCode = 1
        } else {
            $l2Exe = Join-Path $RunDir "parity_l2.exe"
            $l2olLog1 = Join-Path $RunDir "l2bd_link_stdout.log"
            $l2olLog2 = Join-Path $RunDir "l2bd_link_stderr.log"
            $l2olExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" `"$harnessO`" `"$l2BdO`" `"$awO`" -o `"$l2Exe`"" $l2olLog1 $l2olLog2
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
ButtonDispatch-L2-Header: mixa_manager\mixa_button_dispatch_l2.h.lm1 (chains mixa_manager\mixa_tiles_l2.h.lm1, reused from 414f7a7f)
ButtonDispatch-L2-Source: $bdSrc
ButtonDispatch-L2-Source-Sha256: $BdSourceHash
Harness-Source-Sha256: $((Get-FileHash -LiteralPath (Join-Path $RepoRoot "mixa_manager\tests\mixa_button_dispatch_parity_harness.lm1") -Algorithm SHA256).Hash)
Runner-Sha256: $RunnerHash
Oracle-Run-Exit-Code: $oracleRunExit
Oracle-Trace:
$OracleTrace
ButtonDispatch-Translate-Exit-Code: $BdExit
ButtonDispatch-Translate-Stdout:
$BdStdoutText
ButtonDispatch-Translate-Stderr:
$BdStderrText
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
        "EXPECTED_CORE_BARRIER: full mixa_button_dispatch.lm2 translation stopped at an already-known barrier. This is NOT a pass -- the integrated frontend is not yet on main. The oracle-side harness DID run (see Oracle-Trace above)."
    }
    "UNEXPECTED_FAILURE" {
        "UNEXPECTED_FAILURE: translation or build failed with something OTHER than the already-known barriers. Inspect the logs under $RunDir."
    }
    "PARITY_FAILURE" {
        "PARITY_FAILURE: both implementations built and ran, but their traces differ (see Diff above) or one exited non-zero."
    }
    "PASS" {
        "PASS: full mixa_button_dispatch.lm2 translated, built, and ran; its trace is byte-identical to the L1 oracle's own trace."
    }
}

exit $ExitCode
