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
$BaseDir = Join-Path $RepoRoot "build\mixa\claude\mixa_selection_l2_parity"
$RunDir = Join-Path $BaseDir "run_${RunTimestamp}_${RunGuid}"
$HeaderRoot = Join-Path $RunDir "headers"
$HeaderDir = Join-Path $HeaderRoot "mixa_manager"
$CoreHeaderDir = Join-Path $HeaderRoot "l2src"
$StageRoot = Join-Path $RunDir "source"
$StageCoreDir = Join-Path $StageRoot "l2src"
$StageManagerDir = Join-Path $StageRoot "mixa_manager"
New-Item -ItemType Directory -Force -Path $HeaderDir, $CoreHeaderDir, $StageCoreDir, $StageManagerDir | Out-Null

function Invoke-Cmd([string]$exe, [string]$argsStr, [string]$outLog, [string]$errLog) {
    # Established fix (ticket 20260913-063500, re-applied 075100/081200):
    # route through cmd /c so PowerShell's own NativeCommandError
    # machinery under EAP=Stop never wraps a native process's stderr.
    & cmd /c "$exe $argsStr > `"$outLog`" 2> `"$errLog`""
    return $LASTEXITCODE
}

# ---- Step 0: stage and translate the complete predef closure used by the
# generated library L1.  l1trans resolves predef paths from its working
# directory and generated C keeps the same relative paths for *.lm1.h, so a
# build-local mirror gives both phases one stable root without mutating the
# repository or creating a junction. ----
$CoreHeaderUnits = @(
    "lmx_msg_blocks.h.lm1",
    "lmx_owned_ranges.h.lm1",
    "lmx_msg_storage.h.lm1",
    "lmx_msg_path_storage.h.lm1",
    "lmx_msg_slots.h.lm1",
    "lmx_msg_mail_chain.h.lm1",
    "lmx_msg_sched_ready.h.lm1",
    "lmx_msg_visit.h.lm1",
    "lmx_msg_liveness.h.lm1",
    "lmx_chars_owned.h.lm1",
    "lmx_array_owned.h.lm1",
    "lmx_array_ref_owned.h.lm1",
    "lmx_branch_owned.h.lm1",
    "lmx_value_owned.h.lm1",
    "lmx_msg_history_owned.h.lm1",
    "lmx_msg_roots_stale.h.lm1",
    "lmx_graph_copy_owned.h.lm1",
    "lmx_message_graph_copy.h.lm1"
)
foreach ($unit in $CoreHeaderUnits) {
    Copy-Item -LiteralPath (Join-Path $L1Root "l2src\$unit") -Destination (Join-Path $StageCoreDir $unit)
}

Copy-Item -LiteralPath (Join-Path $RepoRoot "mixa_manager\mixa_selection_l2.h.lm1") -Destination (Join-Path $StageManagerDir "mixa_selection_l2.h.lm1")

Push-Location $StageRoot
$hdrLog1 = Join-Path $RunDir "header_trans_stdout.log"
$hdrLog2 = Join-Path $RunDir "header_trans_stderr.log"
$hdrOut = Join-Path $HeaderDir "mixa_selection_l2.lm1.h"
$hdrExit = Invoke-Cmd $L1Trans "mixa_manager\mixa_selection_l2.h.lm1 `"$hdrOut`"" $hdrLog1 $hdrLog2
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

# Library wrappers execute through the real Message runtime.  Build every
# runtime translation unit afresh into this invocation's directory: accepting
# objects from build/l2 would make the parity result depend on stale state.
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
    $unitOutLog = Join-Path $RunDir ("runtime_" + $stem + ".translate.stdout.log")
    $unitErrLog = Join-Path $RunDir ("runtime_" + $stem + ".translate.stderr.log")
    Push-Location $L1Root
    $unitExit = Invoke-Cmd $L1Trans "l2src\$unit `"$generatedC`"" $unitOutLog $unitErrLog
    Pop-Location
    if ($unitExit -ne 0) { Get-Content $unitErrLog; throw "core runtime translation failed: $unit" }
    $compileOut = Join-Path $RunDir ("runtime_" + $stem + ".compile.stdout.log")
    $compileErr = Join-Path $RunDir ("runtime_" + $stem + ".compile.stderr.log")
    $compileExit = Invoke-Cmd "gcc" "$GccStd -I `"$L1Root`" -I `"$L1Root\lm1\build`" -I `"$HeaderRoot`" -c `"$generatedC`" -o `"$generatedO`"" $compileOut $compileErr
    if ($compileExit -ne 0) { Get-Content $compileErr; throw "core runtime compile failed: $unit" }
    $CoreRuntimeObjects += $generatedO
}
foreach ($native in @("lmx_message_host.c", "lmx_message_exec.c")) {
    $stem = $native -replace '\.c$', ''
    $nativeO = Join-Path $RunDir ("runtime_" + $stem + "_native.o")
    $compileOut = Join-Path $RunDir ("runtime_" + $stem + "_native.compile.stdout.log")
    $compileErr = Join-Path $RunDir ("runtime_" + $stem + "_native.compile.stderr.log")
    $nativePath = Join-Path $L1Root "l2src\$native"
    $compileExit = Invoke-Cmd "gcc" "$GccStd -I `"$L1Root`" -I `"$L1Root\lm1\build`" -I `"$HeaderRoot`" -c `"$nativePath`" -o `"$nativeO`"" $compileOut $compileErr
    if ($compileExit -ne 0) { Get-Content $compileErr; throw "core native runtime compile failed: $native" }
    $CoreRuntimeObjects += $nativeO
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
$hcExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$L1Root`" -I `"$HeaderRoot`" -c `"$harnessC`" -o `"$harnessO`"" $hcLog1 $hcLog2
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
    Push-Location $StageRoot
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
        $l2occExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$L1Root`" -I `"$HeaderRoot`" -Dmalloc=test_malloc -Dcalloc=test_calloc -c `"$l2SelC`" -o `"$l2SelO`"" $l2occLog1 $l2occLog2
        if ($l2occExit -ne 0) {
            Get-Content $l2occLog2
            $Verdict = "UNEXPECTED_FAILURE"
            $ExitCode = 1
        } else {
            $l2Exe = Join-Path $RunDir "parity_l2.exe"
            $l2olLog1 = Join-Path $RunDir "l2sel_link_stdout.log"
            $l2olLog2 = Join-Path $RunDir "l2sel_link_stderr.log"
            $runtimeObjectArgs = ($CoreRuntimeObjects | ForEach-Object { '"' + $_ + '"' }) -join ' '
            $l2olExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$L1Root`" -I `"$HeaderRoot`" `"$harnessO`" `"$l2SelO`" `"$faultO`" $runtimeObjectArgs -o `"$l2Exe`"" $l2olLog1 $l2olLog2
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
