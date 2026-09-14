# Real L1-vs-L2 parity gate for mixa_tiles (ticket 20260913-093000).
# Same structure as run_mixa_event_fifo_l2_parity.ps1 (090200/085300):
#   0. ABI parity gate: compile+diff two tiny probes (real headers vs
#      the L2 predef'd header) before ever building the harness. Hard
#      failure (ABI_MISMATCH, exit 1) on any difference.
#   1. ALWAYS builds and runs the ORACLE-side harness.
#   2. Attempts to translate the COMPLETE mixa_tiles.lm2.
#      - fails with an already-known barrier diagnostic ("unknown
#        foreign type", "incompatible entry signature") ->
#        EXPECTED_CORE_BARRIER (exit 2). NOT a pass. Per ticket
#        20260913-093000's own instruction: the integrated frontend with
#        custom-struct-pointer/[]: size_t support is NOT YET on main, so
#        a refusal here is expected, not a source defect.
#      - fails with any OTHER diagnostic -> UNEXPECTED_FAILURE (exit 1).
#      - SUCCEEDS -> builds/links/runs the L2-side harness (linking the
#        L2 tiles object + the harness + the REAL, UNCHANGED
#        mixa_text_rect.o for mixa_cell_at's own real body -- "declare
#        then link", mixa_tiles_l2.h.lm1's own convention) and diffs its
#        stdout against the oracle trace byte-for-byte. PASS (exit 0)
#        only on an exact match, else PARITY_FAILURE (exit 1).
#
# mixa_tiles.h/mixa_tiles.lm1/mixa_core.h/mixa_text_rect.lm1 are the
# parity oracle and are never touched. Nothing under stg/l1_baseline is
# modified, only read.
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
$BaseDir = Join-Path $RepoRoot "build\mixa\claude\mixa_tiles_l2_parity"
$RunDir = Join-Path $BaseDir "run_${RunTimestamp}_${RunGuid}"
$HeaderRoot = Join-Path $RunDir "headers"
$HeaderDir = Join-Path $HeaderRoot "mixa_manager"
$CoreHeaderDir = Join-Path $HeaderRoot "l2src"
$StageRoot = Join-Path $RunDir "source"
$StageCoreDir = Join-Path $StageRoot "l2src"
$StageManagerDir = Join-Path $StageRoot "mixa_manager"
New-Item -ItemType Directory -Force -Path $HeaderDir, $CoreHeaderDir, $StageCoreDir, $StageManagerDir | Out-Null

function Invoke-Cmd([string]$exe, [string]$argsStr, [string]$outLog, [string]$errLog) {
    & cmd /c "$exe $argsStr > `"$outLog`" 2> `"$errLog`""
    return $LASTEXITCODE
}

# ---- Step 0: stage the generated library's complete predef closure. ----
$CoreHeaderUnits = @(
    "lmx_msg_blocks.h.lm1", "lmx_owned_ranges.h.lm1", "lmx_msg_storage.h.lm1",
    "lmx_msg_path_storage.h.lm1", "lmx_msg_slots.h.lm1", "lmx_msg_mail_chain.h.lm1",
    "lmx_msg_sched_ready.h.lm1", "lmx_msg_visit.h.lm1", "lmx_msg_liveness.h.lm1",
    "lmx_chars_owned.h.lm1", "lmx_array_owned.h.lm1", "lmx_array_ref_owned.h.lm1",
    "lmx_branch_owned.h.lm1", "lmx_value_owned.h.lm1", "lmx_msg_history_owned.h.lm1",
    "lmx_msg_roots_stale.h.lm1", "lmx_graph_copy_owned.h.lm1",
    "lmx_message_graph_copy.h.lm1"
)
foreach ($unit in $CoreHeaderUnits) {
    Copy-Item -LiteralPath (Join-Path $L1Root "l2src\$unit") -Destination (Join-Path $StageCoreDir $unit)
}
Copy-Item -LiteralPath (Join-Path $RepoRoot "mixa_manager\mixa_tiles_l2.h.lm1") -Destination (Join-Path $StageManagerDir "mixa_tiles_l2.h.lm1")

Push-Location $StageRoot
$hdrLog1 = Join-Path $RunDir "header_trans_stdout.log"
$hdrLog2 = Join-Path $RunDir "header_trans_stderr.log"
$hdrOut = Join-Path $HeaderDir "mixa_tiles_l2.lm1.h"
$hdrExit = Invoke-Cmd $L1Trans "mixa_manager\mixa_tiles_l2.h.lm1 `"$hdrOut`"" $hdrLog1 $hdrLog2
Pop-Location
if ($hdrExit -ne 0) { throw "header translation failed: see $hdrLog2" }
foreach ($unit in $CoreHeaderUnits) {
    $unitOut = Join-Path $CoreHeaderDir ($unit -replace '\.h\.lm1$', '.lm1.h')
    $unitOutLog = Join-Path $RunDir (($unit -replace '[^A-Za-z0-9_.-]', '_') + ".stdout.log")
    $unitErrLog = Join-Path $RunDir (($unit -replace '[^A-Za-z0-9_.-]', '_') + ".stderr.log")
    Push-Location $StageRoot
    $unitExit = Invoke-Cmd $L1Trans "l2src\$unit `"$unitOut`"" $unitOutLog $unitErrLog
    Pop-Location
    if ($unitExit -ne 0) { Get-Content $unitErrLog; throw "core header translation failed: $unit" }
}

# Generated library wrappers execute through a real, freshly built Message runtime.
$CoreImplUnits = @(
    "lmx_msg_blocks.lm1", "lmx_owned_ranges.lm1", "lmx_msg_storage.lm1",
    "lmx_msg_path_storage.lm1", "lmx_msg_slots.lm1", "lmx_msg_mail_chain.lm1",
    "lmx_msg_sched_ready.lm1", "lmx_msg_visit.lm1", "lmx_msg_liveness.lm1",
    "lmx_chars_owned.lm1", "lmx_array_owned.lm1", "lmx_array_ref_owned.lm1",
    "lmx_branch_owned.lm1", "lmx_value_owned.lm1", "lmx_msg_history_owned.lm1",
    "lmx_msg_roots_stale.lm1", "lmx_graph_copy_owned.lm1",
    "lmx_message_graph_copy.lm1", "lmx_message.lm1"
)
$CoreRuntimeObjects = @()
foreach ($unit in $CoreImplUnits) {
    $stem = $unit -replace '\.lm1$', ''
    $generatedC = Join-Path $RunDir ("runtime_" + $stem + ".c")
    $generatedO = Join-Path $RunDir ("runtime_" + $stem + ".o")
    Push-Location $L1Root
    $unitExit = Invoke-Cmd $L1Trans "l2src\$unit `"$generatedC`"" (Join-Path $RunDir "runtime_$stem.translate.stdout.log") (Join-Path $RunDir "runtime_$stem.translate.stderr.log")
    Pop-Location
    if ($unitExit -ne 0) { throw "core runtime translation failed: $unit" }
    $compileExit = Invoke-Cmd "gcc" "$GccStd -I `"$L1Root`" -I `"$L1Root\lm1\build`" -I `"$HeaderRoot`" -c `"$generatedC`" -o `"$generatedO`"" (Join-Path $RunDir "runtime_$stem.compile.stdout.log") (Join-Path $RunDir "runtime_$stem.compile.stderr.log")
    if ($compileExit -ne 0) { throw "core runtime compile failed: $unit" }
    $CoreRuntimeObjects += $generatedO
}
foreach ($native in @("lmx_message_host.c", "lmx_message_exec.c")) {
    $stem = $native -replace '\.c$', ''
    $nativeO = Join-Path $RunDir ("runtime_" + $stem + "_native.o")
    $nativePath = Join-Path $L1Root "l2src\$native"
    $compileExit = Invoke-Cmd "gcc" "$GccStd -I `"$L1Root`" -I `"$L1Root\lm1\build`" -I `"$HeaderRoot`" -c `"$nativePath`" -o `"$nativeO`"" (Join-Path $RunDir "runtime_$stem.native.stdout.log") (Join-Path $RunDir "runtime_$stem.native.stderr.log")
    if ($compileExit -ne 0) { throw "core native runtime compile failed: $native" }
    $CoreRuntimeObjects += $nativeO
}

# ---- Step 0.5: ABI parity gate. ----
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
    $AbiSummary = @"
ABI_MISMATCH: the L2 header's struct layout does not match the real
production headers. This is a hard failure -- the harness was NOT built
or run.
Real-header probe output:
$AbiRealText
L2-header probe output:
$AbiL2Text
Diff:
$abiDiff
"@
    Set-Content -LiteralPath (Join-Path $RunDir "run_summary.txt") -Value $AbiSummary
    $AbiSummary
    "Run directory: $RunDir"
    "ABI_MISMATCH: struct layout drift detected between mixa_tiles_l2.h.lm1 and the real headers -- see the diff above."
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

# ---- Step 2: build the harness + the real mixa_text_rect.o (linked in
# for mixa_cell_at's own real body on the L2 side; the ORACLE side
# already gets its own copy for free since mixa_tiles.lm1 predefs
# mixa_text_rect.lm1 directly, inlining the full source). ----
Push-Location $RepoRoot
$harnessC = Join-Path $RunDir "harness.c"
$hLog1 = Join-Path $RunDir "harness_trans_stdout.log"
$hLog2 = Join-Path $RunDir "harness_trans_stderr.log"
$hExit = Invoke-Cmd $L1Trans "mixa_manager\tests\mixa_tiles_parity_harness.lm1 `"$harnessC`"" $hLog1 $hLog2
Pop-Location
if ($hExit -ne 0) { Get-Content $hLog2; throw "harness translation failed" }

$harnessO = Join-Path $RunDir "harness.o"
$hcLog1 = Join-Path $RunDir "harness_compile_stdout.log"
$hcLog2 = Join-Path $RunDir "harness_compile_stderr.log"
$hcExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" -c `"$harnessC`" -o `"$harnessO`"" $hcLog1 $hcLog2
if ($hcExit -ne 0) { Get-Content $hcLog2; throw "harness compile failed" }

Push-Location $RepoRoot
$textRectC = Join-Path $RunDir "mixa_text_rect.c"
$trLog1 = Join-Path $RunDir "text_rect_trans_stdout.log"
$trLog2 = Join-Path $RunDir "text_rect_trans_stderr.log"
$trExit = Invoke-Cmd $L1Trans "mixa_manager\mixa_text_rect.lm1 `"$textRectC`"" $trLog1 $trLog2
Pop-Location
if ($trExit -ne 0) { Get-Content $trLog2; throw "mixa_text_rect.lm1 translation failed" }

$textRectO = Join-Path $RunDir "mixa_text_rect.o"
$trcLog1 = Join-Path $RunDir "text_rect_compile_stdout.log"
$trcLog2 = Join-Path $RunDir "text_rect_compile_stderr.log"
$trcExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -c `"$textRectC`" -o `"$textRectO`"" $trcLog1 $trcLog2
if ($trcExit -ne 0) { Get-Content $trcLog2; throw "mixa_text_rect.lm1 compile failed" }

# ---- Step 3: ALWAYS build + run the ORACLE-side harness. ----
$oracleC = Join-Path $RunDir "mixa_tiles_oracle.c"
$ocLog1 = Join-Path $RunDir "oracle_trans_stdout.log"
$ocLog2 = Join-Path $RunDir "oracle_trans_stderr.log"
Push-Location $RepoRoot
$ocExit = Invoke-Cmd $L1Trans "mixa_manager\mixa_tiles.lm1 `"$oracleC`"" $ocLog1 $ocLog2
Pop-Location
if ($ocExit -ne 0) { Get-Content $ocLog2; throw "oracle mixa_tiles.lm1 translation failed" }

$oracleO = Join-Path $RunDir "mixa_tiles_oracle.o"
$occLog1 = Join-Path $RunDir "oracle_compile_stdout.log"
$occLog2 = Join-Path $RunDir "oracle_compile_stderr.log"
$occExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -c `"$oracleC`" -o `"$oracleO`"" $occLog1 $occLog2
if ($occExit -ne 0) { Get-Content $occLog2; throw "oracle mixa_tiles.lm1 compile failed" }

$oracleExe = Join-Path $RunDir "parity_oracle.exe"
$olLog1 = Join-Path $RunDir "oracle_link_stdout.log"
$olLog2 = Join-Path $RunDir "oracle_link_stderr.log"
$olExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" `"$harnessO`" `"$oracleO`" -o `"$oracleExe`"" $olLog1 $olLog2
if ($olExit -ne 0) { Get-Content $olLog2; throw "oracle harness link failed" }

$oracleRunOut = Join-Path $RunDir "oracle_run_stdout.log"
$oracleRunErr = Join-Path $RunDir "oracle_run_stderr.log"
$oracleRunExit = Invoke-Cmd "`"$oracleExe`"" "" $oracleRunOut $oracleRunErr
$OracleTrace = Get-Content -LiteralPath $oracleRunOut -Raw

# ---- Step 4: attempt the COMPLETE mixa_tiles.lm2 translation. ----
Push-Location $RepoRoot
$tilesSrc = "mixa_manager\mixa_tiles.lm2"
$tilesOut = Join-Path $RunDir "mixa_tiles_l2.lm1"
$tilesStdout = Join-Path $RunDir "tiles_stdout.log"
$tilesStderr = Join-Path $RunDir "tiles_stderr.log"
$TilesExit = Invoke-Cmd "`"$l2exe`"" "`"$tilesSrc`" `"$tilesOut`"" $tilesStdout $tilesStderr
Pop-Location

$TilesStdoutText = if (Test-Path -LiteralPath $tilesStdout) { Get-Content -LiteralPath $tilesStdout -Raw } else { "" }
$TilesStderrText = if (Test-Path -LiteralPath $tilesStderr) { Get-Content -LiteralPath $tilesStderr -Raw } else { "" }
$TilesSourceHash = (Get-FileHash -LiteralPath (Join-Path $RepoRoot $tilesSrc) -Algorithm SHA256).Hash
$TilesHeaderHash = (Get-FileHash -LiteralPath (Join-Path $RepoRoot "mixa_manager\mixa_tiles_l2.h.lm1") -Algorithm SHA256).Hash
$RunnerHash = (Get-FileHash -LiteralPath $PSCommandPath -Algorithm SHA256).Hash

$Verdict = $null
$ExitCode = 1
$L2TraceText = ""
$DiffText = ""

$KnownBarrier = ($TilesStderrText -match "unknown foreign type") -or ($TilesStderrText -match "incompatible entry signature")

if ($TilesExit -ne 0 -and $KnownBarrier) {
    $Verdict = "EXPECTED_CORE_BARRIER"
    $ExitCode = 2
} elseif ($TilesExit -ne 0) {
    $Verdict = "UNEXPECTED_FAILURE"
    $ExitCode = 1
} else {
    Push-Location $StageRoot
    $l2TilesC = Join-Path $RunDir "mixa_tiles_l2.c"
    $l2tcLog1 = Join-Path $RunDir "l2tiles_trans_stdout.log"
    $l2tcLog2 = Join-Path $RunDir "l2tiles_trans_stderr.log"
    $l2tcExit = Invoke-Cmd $L1Trans "`"$tilesOut`" `"$l2TilesC`"" $l2tcLog1 $l2tcLog2
    Pop-Location
    if ($l2tcExit -ne 0) {
        Get-Content $l2tcLog2
        $Verdict = "UNEXPECTED_FAILURE"
        $ExitCode = 1
    } else {
        $l2TilesO = Join-Path $RunDir "mixa_tiles_l2.o"
        $l2occLog1 = Join-Path $RunDir "l2tiles_compile_stdout.log"
        $l2occLog2 = Join-Path $RunDir "l2tiles_compile_stderr.log"
        $l2occExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$L1Root`" -I `"$L1Root\lm1\build`" -I `"$HeaderRoot`" -c `"$l2TilesC`" -o `"$l2TilesO`"" $l2occLog1 $l2occLog2
        if ($l2occExit -ne 0) {
            Get-Content $l2occLog2
            $Verdict = "UNEXPECTED_FAILURE"
            $ExitCode = 1
        } else {
            $l2Exe = Join-Path $RunDir "parity_l2.exe"
            $l2olLog1 = Join-Path $RunDir "l2tiles_link_stdout.log"
            $l2olLog2 = Join-Path $RunDir "l2tiles_link_stderr.log"
            $runtimeObjectArgs = ($CoreRuntimeObjects | ForEach-Object { '"' + $_ + '"' }) -join ' '
            $l2olExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$L1Root`" -I `"$HeaderRoot`" `"$harnessO`" `"$l2TilesO`" `"$textRectO`" $runtimeObjectArgs -o `"$l2Exe`"" $l2olLog1 $l2olLog2
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
Tiles-L2-Header: mixa_manager\mixa_tiles_l2.h.lm1
Tiles-L2-Header-Sha256: $TilesHeaderHash
Tiles-L2-Source: $tilesSrc
Tiles-L2-Source-Sha256: $TilesSourceHash
Harness-Source-Sha256: $((Get-FileHash -LiteralPath (Join-Path $RepoRoot "mixa_manager\tests\mixa_tiles_parity_harness.lm1") -Algorithm SHA256).Hash)
Runner-Sha256: $RunnerHash
Oracle-Run-Exit-Code: $oracleRunExit
Oracle-Trace:
$OracleTrace
Tiles-Translate-Exit-Code: $TilesExit
Tiles-Translate-Stdout:
$TilesStdoutText
Tiles-Translate-Stderr:
$TilesStderrText
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
        "EXPECTED_CORE_BARRIER: full mixa_tiles.lm2 translation stopped at an already-known barrier. This is NOT a pass -- per ticket 20260913-093000, the integrated frontend with custom-struct-pointer/[]: size_t support is not yet on main, so this is expected. The oracle-side harness DID run (see Oracle-Trace above)."
    }
    "UNEXPECTED_FAILURE" {
        "UNEXPECTED_FAILURE: translation or build failed with something OTHER than the already-known barriers. Inspect the logs under $RunDir."
    }
    "PARITY_FAILURE" {
        "PARITY_FAILURE: both implementations built and ran, but their traces differ (see Diff above) or one exited non-zero."
    }
    "PASS" {
        "PASS: full mixa_tiles.lm2 translated, built, and ran; its trace is byte-identical to the L1 oracle's own trace."
    }
}

exit $ExitCode
