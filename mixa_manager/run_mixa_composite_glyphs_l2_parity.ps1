# Real L1-vs-L2 parity gate for mixa_composite_glyphs (ticket
# 20260913-110000). Same structure as run_mixa_composite_l2_parity.ps1
# (103000). Reuses mixa_tiles_l2.h.lm1 and the tiles ABI probe pair.
#   0. ABI parity gate: reuses mixa_tiles_abi_probe_real.c/_l2.c (the
#      MixaGlyph/MixaBackend chain has no byte-sized fields of its own;
#      MixaBackend is intentionally a single opaque `void*` field
#      rather than a full MixaBackendVTable declaration -- verified via
#      a dedicated one-off sizeof/offsetof check before writing the
#      source, recorded in mixa_composite_glyphs_l2_port.txt).
#   1. ALWAYS builds and runs the ORACLE-side harness, linking the real
#      mixa_backend_table.o (for mixa_backend_glyph's own real dispatch
#      body) plus a tiny test-only ctors stub (mixa_composite_glyphs_
#      ctors_stub.lm1) so the link does not have to pull in an entire
#      real backend's own dependency chain (event fifo, console window,
#      etc.) this harness never exercises.
#   2. Translates the COMPLETE mixa_composite_glyphs.lm2, builds and links
#      the L2-side harness, and diffs its stdout against the oracle trace
#      byte-for-byte. Any translation/build failure is a failure; PASS is
#      possible only on exact behavioral parity.
#
# mixa_composite_glyphs.lm1/mixa_backend.h/mixa_backend_table.lm1/mixa_
# core.h/mixa_tiles.lm1/mixa_text_rect.lm1/mixa_overlay.h are the parity
# oracle and are never touched. Nothing under stg/l1_baseline is
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
$BaseDir = Join-Path $RepoRoot "build\mixa\claude\mixa_composite_glyphs_l2_parity"
$RunDir = Join-Path $BaseDir "run_${RunTimestamp}_${RunGuid}"
$HeaderRoot = Join-Path $RunDir "headers"
$HeaderDir = Join-Path $HeaderRoot "mixa_manager"
$HeaderTestsDir = Join-Path $HeaderDir "tests"
$CoreHeaderDir = Join-Path $HeaderRoot "l2src"
$StageRoot = Join-Path $RunDir "source"
$StageCoreDir = Join-Path $StageRoot "l2src"
$StageManagerDir = Join-Path $StageRoot "mixa_manager"
$StageManagerTestsDir = Join-Path $StageManagerDir "tests"
New-Item -ItemType Directory -Force -Path $HeaderTestsDir, $CoreHeaderDir, $StageCoreDir, $StageManagerDir, $StageManagerTestsDir | Out-Null

function Invoke-Cmd([string]$exe, [string]$argsStr, [string]$outLog, [string]$errLog) {
    & cmd /c "$exe $argsStr > `"$outLog`" 2> `"$errLog`""
    return $LASTEXITCODE
}

# ---- Step 0: stage the complete generated-library predef closure. ----
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
foreach ($unit in @("mixa_tiles_l2.h.lm1", "mixa_composite_glyphs_l2.h.lm1")) {
    Copy-Item -LiteralPath (Join-Path $RepoRoot "mixa_manager\$unit") -Destination (Join-Path $StageManagerDir $unit)
}
Copy-Item -LiteralPath (Join-Path $RepoRoot "mixa_manager\tests\mixa_composite_glyphs_testvtable.h.lm1") -Destination (Join-Path $StageManagerTestsDir "mixa_composite_glyphs_testvtable.h.lm1")

Push-Location $StageRoot
$hdrLog1 = Join-Path $RunDir "header_trans_stdout.log"
$hdrLog2 = Join-Path $RunDir "header_trans_stderr.log"
$tilesHdrOut = Join-Path $HeaderDir "mixa_tiles_l2.lm1.h"
$hdrExit = Invoke-Cmd $L1Trans "mixa_manager\mixa_tiles_l2.h.lm1 `"$tilesHdrOut`"" $hdrLog1 $hdrLog2
if ($hdrExit -ne 0) { Pop-Location; throw "tiles header translation failed: see $hdrLog2" }
$cgHdrOut = Join-Path $HeaderDir "mixa_composite_glyphs_l2.lm1.h"
$hdrExit2 = Invoke-Cmd $L1Trans "mixa_manager\mixa_composite_glyphs_l2.h.lm1 `"$cgHdrOut`"" $hdrLog1 $hdrLog2
if ($hdrExit2 -ne 0) { Pop-Location; throw "composite_glyphs header translation failed: see $hdrLog2" }
$vtHdrOut = Join-Path $HeaderTestsDir "mixa_composite_glyphs_testvtable.lm1.h"
$hdrExit3 = Invoke-Cmd $L1Trans "mixa_manager\tests\mixa_composite_glyphs_testvtable.h.lm1 `"$vtHdrOut`"" $hdrLog1 $hdrLog2
Pop-Location
if ($hdrExit3 -ne 0) { throw "testvtable header translation failed: see $hdrLog2" }
foreach ($unit in $CoreHeaderUnits) {
    $unitOut = Join-Path $CoreHeaderDir ($unit -replace '\.h\.lm1$', '.lm1.h')
    Push-Location $StageRoot
    $unitExit = Invoke-Cmd $L1Trans "l2src\$unit `"$unitOut`"" (Join-Path $RunDir "runtime_header_$unit.stdout.log") (Join-Path $RunDir "runtime_header_$unit.stderr.log")
    Pop-Location
    if ($unitExit -ne 0) { throw "core header translation failed: $unit" }
}

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
    $compileExit = Invoke-Cmd "gcc" "$GccStd -I `"$L1Root`" -I `"$L1Root\lm1\build`" -I `"$HeaderRoot`" -c `"$(Join-Path $L1Root "l2src\$native")`" -o `"$nativeO`"" (Join-Path $RunDir "runtime_$stem.native.stdout.log") (Join-Path $RunDir "runtime_$stem.native.stderr.log")
    if ($compileExit -ne 0) { throw "core native runtime compile failed: $native" }
    $CoreRuntimeObjects += $nativeO
}

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

# ---- Step 2: build the harness object + the real mixa_backend_table.o
# + the test-only ctors stub (avoids pulling a real backend's own full
# dependency chain). ----
Push-Location $RepoRoot
$harnessC = Join-Path $RunDir "harness.c"
$hLog1 = Join-Path $RunDir "harness_trans_stdout.log"
$hLog2 = Join-Path $RunDir "harness_trans_stderr.log"
$hExit = Invoke-Cmd $L1Trans "mixa_manager\tests\mixa_composite_glyphs_parity_harness.lm1 `"$harnessC`"" $hLog1 $hLog2
Pop-Location
if ($hExit -ne 0) { Get-Content $hLog2; throw "harness translation failed" }

$harnessO = Join-Path $RunDir "harness.o"
$hcLog1 = Join-Path $RunDir "harness_compile_stdout.log"
$hcLog2 = Join-Path $RunDir "harness_compile_stderr.log"
$hcExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" -c `"$harnessC`" -o `"$harnessO`"" $hcLog1 $hcLog2
if ($hcExit -ne 0) { Get-Content $hcLog2; throw "harness compile failed" }

Push-Location $RepoRoot
$btC = Join-Path $RunDir "mixa_backend_table.c"
$btLog1 = Join-Path $RunDir "bt_trans_stdout.log"
$btLog2 = Join-Path $RunDir "bt_trans_stderr.log"
$btExit = Invoke-Cmd $L1Trans "mixa_manager\mixa_backend_table.lm1 `"$btC`"" $btLog1 $btLog2
Pop-Location
if ($btExit -ne 0) { Get-Content $btLog2; throw "mixa_backend_table.lm1 translation failed" }

$btO = Join-Path $RunDir "mixa_backend_table.o"
$btcLog1 = Join-Path $RunDir "bt_compile_stdout.log"
$btcLog2 = Join-Path $RunDir "bt_compile_stderr.log"
$btcExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -c `"$btC`" -o `"$btO`"" $btcLog1 $btcLog2
if ($btcExit -ne 0) { Get-Content $btcLog2; throw "mixa_backend_table.lm1 compile failed" }

Push-Location $RepoRoot
$csC = Join-Path $RunDir "ctors_stub.c"
$csLog1 = Join-Path $RunDir "cs_trans_stdout.log"
$csLog2 = Join-Path $RunDir "cs_trans_stderr.log"
$csExit = Invoke-Cmd $L1Trans "mixa_manager\tests\mixa_composite_glyphs_ctors_stub.lm1 `"$csC`"" $csLog1 $csLog2
Pop-Location
if ($csExit -ne 0) { Get-Content $csLog2; throw "ctors stub translation failed" }

$csO = Join-Path $RunDir "ctors_stub.o"
$cscLog1 = Join-Path $RunDir "cs_compile_stdout.log"
$cscLog2 = Join-Path $RunDir "cs_compile_stderr.log"
$cscExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -c `"$csC`" -o `"$csO`"" $cscLog1 $cscLog2
if ($cscExit -ne 0) { Get-Content $cscLog2; throw "ctors stub compile failed" }

# The L2 unit deliberately sees only ABI headers for the lower manager
# layers.  Link their accepted L1 implementation as an independently-built
# dependency object.  mixa_tiles.lm1 predefines mixa_text_rect.lm1, so this
# single object supplies both the tile and rectangle symbols without duplicate
# definitions.
Push-Location $RepoRoot
$tilesImplC = Join-Path $RunDir "mixa_tiles_dependency.c"
$tilesImplExit = Invoke-Cmd $L1Trans "mixa_manager\mixa_tiles.lm1 `"$tilesImplC`"" (Join-Path $RunDir "tiles_dependency_translate_stdout.log") (Join-Path $RunDir "tiles_dependency_translate_stderr.log")
Pop-Location
if ($tilesImplExit -ne 0) { throw "mixa_tiles.lm1 dependency translation failed" }

$tilesImplO = Join-Path $RunDir "mixa_tiles_dependency.o"
$tilesImplCompileExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -c `"$tilesImplC`" -o `"$tilesImplO`"" (Join-Path $RunDir "tiles_dependency_compile_stdout.log") (Join-Path $RunDir "tiles_dependency_compile_stderr.log")
if ($tilesImplCompileExit -ne 0) { throw "mixa_tiles.lm1 dependency compile failed" }

# ---- Step 3: ALWAYS build + run the ORACLE-side harness. ----
$oracleC = Join-Path $RunDir "mixa_composite_glyphs_oracle.c"
$ocLog1 = Join-Path $RunDir "oracle_trans_stdout.log"
$ocLog2 = Join-Path $RunDir "oracle_trans_stderr.log"
Push-Location $RepoRoot
$ocExit = Invoke-Cmd $L1Trans "mixa_manager\mixa_composite_glyphs.lm1 `"$oracleC`"" $ocLog1 $ocLog2
Pop-Location
if ($ocExit -ne 0) { Get-Content $ocLog2; throw "oracle mixa_composite_glyphs.lm1 translation failed" }

$oracleO = Join-Path $RunDir "mixa_composite_glyphs_oracle.o"
$occLog1 = Join-Path $RunDir "oracle_compile_stdout.log"
$occLog2 = Join-Path $RunDir "oracle_compile_stderr.log"
$occExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -c `"$oracleC`" -o `"$oracleO`"" $occLog1 $occLog2
if ($occExit -ne 0) { Get-Content $occLog2; throw "oracle mixa_composite_glyphs.lm1 compile failed" }

$oracleExe = Join-Path $RunDir "parity_oracle.exe"
$olLog1 = Join-Path $RunDir "oracle_link_stdout.log"
$olLog2 = Join-Path $RunDir "oracle_link_stderr.log"
$olExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" `"$harnessO`" `"$oracleO`" `"$btO`" `"$csO`" -o `"$oracleExe`"" $olLog1 $olLog2
if ($olExit -ne 0) { Get-Content $olLog2; throw "oracle harness link failed" }

$oracleRunOut = Join-Path $RunDir "oracle_run_stdout.log"
$oracleRunErr = Join-Path $RunDir "oracle_run_stderr.log"
$oracleRunExit = Invoke-Cmd "`"$oracleExe`"" "" $oracleRunOut $oracleRunErr
$OracleTrace = Get-Content -LiteralPath $oracleRunOut -Raw

# ---- Step 4: attempt the COMPLETE mixa_composite_glyphs.lm2 translation. ----
Push-Location $RepoRoot
$cgSrc = "mixa_manager\mixa_composite_glyphs.lm2"
$cgOut = Join-Path $RunDir "mixa_composite_glyphs_l2.lm1"
$cgStdout = Join-Path $RunDir "cg_stdout.log"
$cgStderr = Join-Path $RunDir "cg_stderr.log"
$CgExit = Invoke-Cmd "`"$l2exe`"" "`"$cgSrc`" `"$cgOut`"" $cgStdout $cgStderr
Pop-Location

$CgStdoutText = if (Test-Path -LiteralPath $cgStdout) { Get-Content -LiteralPath $cgStdout -Raw } else { "" }
$CgStderrText = if (Test-Path -LiteralPath $cgStderr) { Get-Content -LiteralPath $cgStderr -Raw } else { "" }
$CgSourceHash = (Get-FileHash -LiteralPath (Join-Path $RepoRoot $cgSrc) -Algorithm SHA256).Hash
$RunnerHash = (Get-FileHash -LiteralPath $PSCommandPath -Algorithm SHA256).Hash

$Verdict = $null
$ExitCode = 1
$L2TraceText = ""
$DiffText = ""

if ($CgExit -ne 0) {
    $Verdict = "UNEXPECTED_FAILURE"
    $ExitCode = 1
} else {
    Push-Location $StageRoot
    $l2CgC = Join-Path $RunDir "mixa_composite_glyphs_l2.c"
    $l2ccLog1 = Join-Path $RunDir "l2cg_trans_stdout.log"
    $l2ccLog2 = Join-Path $RunDir "l2cg_trans_stderr.log"
    $l2ccExit = Invoke-Cmd $L1Trans "`"$cgOut`" `"$l2CgC`"" $l2ccLog1 $l2ccLog2
    Pop-Location
    if ($l2ccExit -ne 0) {
        Get-Content $l2ccLog2
        $Verdict = "UNEXPECTED_FAILURE"
        $ExitCode = 1
    } else {
        $l2CgO = Join-Path $RunDir "mixa_composite_glyphs_l2.o"
        $l2occLog1 = Join-Path $RunDir "l2cg_compile_stdout.log"
        $l2occLog2 = Join-Path $RunDir "l2cg_compile_stderr.log"
        $l2occExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$L1Root`" -I `"$L1Root\lm1\build`" -I `"$HeaderRoot`" -c `"$l2CgC`" -o `"$l2CgO`"" $l2occLog1 $l2occLog2
        if ($l2occExit -ne 0) {
            Get-Content $l2occLog2
            $Verdict = "UNEXPECTED_FAILURE"
            $ExitCode = 1
        } else {
            $l2Exe = Join-Path $RunDir "parity_l2.exe"
            $l2olLog1 = Join-Path $RunDir "l2cg_link_stdout.log"
            $l2olLog2 = Join-Path $RunDir "l2cg_link_stderr.log"
            $runtimeObjectArgs = ($CoreRuntimeObjects | ForEach-Object { '"' + $_ + '"' }) -join ' '
            $l2olExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$L1Root`" -I `"$HeaderRoot`" `"$harnessO`" `"$l2CgO`" `"$tilesImplO`" `"$btO`" `"$csO`" $runtimeObjectArgs -o `"$l2Exe`"" $l2olLog1 $l2olLog2
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
CompositeGlyphs-L2-Header: mixa_manager\mixa_composite_glyphs_l2.h.lm1 (chains mixa_manager\mixa_tiles_l2.h.lm1, reused from 414f7a7f)
CompositeGlyphs-L2-Source: $cgSrc
CompositeGlyphs-L2-Source-Sha256: $CgSourceHash
Harness-Source-Sha256: $((Get-FileHash -LiteralPath (Join-Path $RepoRoot "mixa_manager\tests\mixa_composite_glyphs_parity_harness.lm1") -Algorithm SHA256).Hash)
Runner-Sha256: $RunnerHash
Oracle-Run-Exit-Code: $oracleRunExit
Oracle-Trace:
$OracleTrace
CompositeGlyphs-Translate-Exit-Code: $CgExit
CompositeGlyphs-Translate-Stdout:
$CgStdoutText
CompositeGlyphs-Translate-Stderr:
$CgStderrText
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
    "UNEXPECTED_FAILURE" {
        "UNEXPECTED_FAILURE: translation or build failed. Inspect the logs under $RunDir."
    }
    "PARITY_FAILURE" {
        "PARITY_FAILURE: both implementations built and ran, but their traces differ (see Diff above) or one exited non-zero."
    }
    "PASS" {
        "PASS: full mixa_composite_glyphs.lm2 translated, built, and ran; its trace is byte-identical to the L1 oracle's own trace."
    }
}

exit $ExitCode
