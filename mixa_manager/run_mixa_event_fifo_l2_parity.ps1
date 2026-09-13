# Real L1-vs-L2 parity gate for mixa_event_fifo (ticket 20260913-085300).
# Same structure as run_mixa_selection_l2_parity.ps1 (081200/083100):
#   1. ALWAYS builds and runs the ORACLE-side harness (real
#      mixa_event_fifo.lm1 + tests/mixa_event_fifo_parity_harness.lm1) --
#      "run all preparatory/oracle checks that are possible", independent
#      of L2 translation status.
#   2. Attempts to translate the COMPLETE mixa_event_fifo.lm2.
#      - fails with the already-known barrier diagnostic ("unknown
#        foreign type" -- no manager-owned struct type admission) ->
#        EXPECTED_CORE_BARRIER (exit 2). NOT a pass.
#      - fails with any OTHER diagnostic -> UNEXPECTED_FAILURE (exit 1).
#      - SUCCEEDS -> builds/links/runs the L2-side harness against the
#        emitted object and diffs its stdout against the oracle trace
#        byte-for-byte. PASS (exit 0) only on an exact match with both
#        exiting 0, else PARITY_FAILURE (exit 1).
#
# mixa_event_fifo.h/mixa_event_fifo.lm1 are the parity oracle and are
# never touched. Nothing under stg/l1_baseline is modified, only read.
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
$BaseDir = Join-Path $RepoRoot "build\mixa\claude\mixa_event_fifo_l2_parity"
$RunDir = Join-Path $BaseDir "run_${RunTimestamp}_${RunGuid}"
$HeaderDir = Join-Path $RunDir "headers\mixa_manager"
New-Item -ItemType Directory -Force -Path $HeaderDir | Out-Null

function Invoke-Cmd([string]$exe, [string]$argsStr, [string]$outLog, [string]$errLog) {
    # Established fix (ticket 20260913-063500, re-applied 075100/081200/
    # 083100): route through cmd /c so PowerShell's own NativeCommandError
    # machinery under EAP=Stop never wraps a native process's stderr.
    & cmd /c "$exe $argsStr > `"$outLog`" 2> `"$errLog`""
    return $LASTEXITCODE
}

# ---- Step 0: translate the shared header-unit once. ----
Push-Location $RepoRoot
$hdrLog1 = Join-Path $RunDir "header_trans_stdout.log"
$hdrLog2 = Join-Path $RunDir "header_trans_stderr.log"
$hdrOut = Join-Path $HeaderDir "mixa_event_fifo_l2.lm1.h"
$hdrExit = Invoke-Cmd $L1Trans "mixa_manager\mixa_event_fifo_l2.h.lm1 `"$hdrOut`"" $hdrLog1 $hdrLog2
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

# ---- Step 1.5: ABI parity gate (ticket 20260913-090200). Before ever
# building or running the actual harness, compile two tiny standalone
# probes -- one against the REAL production headers, one against the L2
# predef'd header this port uses -- and diff their sizeof/offsetof
# report byte-for-byte. A silently drifted copied struct declaration
# must fail HERE, loudly and specifically, never surface only as garbage
# field values deep inside a scenario trace. ----
$abiRealExe = Join-Path $RunDir "abi_probe_real.exe"
$abiRealLog1 = Join-Path $RunDir "abi_probe_real_compile_stdout.log"
$abiRealLog2 = Join-Path $RunDir "abi_probe_real_compile_stderr.log"
$abiRealExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" `"$RepoRoot\mixa_manager\tests\mixa_event_fifo_abi_probe_real.c`" -o `"$abiRealExe`"" $abiRealLog1 $abiRealLog2
if ($abiRealExit -ne 0) { Get-Content $abiRealLog2; throw "ABI probe (real headers) compile failed" }

$abiL2Exe = Join-Path $RunDir "abi_probe_l2.exe"
$abiL2Log1 = Join-Path $RunDir "abi_probe_l2_compile_stdout.log"
$abiL2Log2 = Join-Path $RunDir "abi_probe_l2_compile_stderr.log"
$abiL2Exit = Invoke-Cmd "gcc" "$GccStd -I `"$RunDir\headers`" `"$RepoRoot\mixa_manager\tests\mixa_event_fifo_abi_probe_l2.c`" -o `"$abiL2Exe`"" $abiL2Log1 $abiL2Log2
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
    "ABI_MISMATCH: struct layout drift detected between mixa_event_fifo_l2.h.lm1 and the real headers -- see the diff above. Fix the header before anything else; the harness was not run."
    exit 1
}

# ---- Step 2: build the harness object (implementation-independent). ----
Push-Location $RepoRoot
$harnessC = Join-Path $RunDir "harness.c"
$hLog1 = Join-Path $RunDir "harness_trans_stdout.log"
$hLog2 = Join-Path $RunDir "harness_trans_stderr.log"
$hExit = Invoke-Cmd $L1Trans "mixa_manager\tests\mixa_event_fifo_parity_harness.lm1 `"$harnessC`"" $hLog1 $hLog2
Pop-Location
if ($hExit -ne 0) { Get-Content $hLog2; throw "harness translation failed" }

$harnessO = Join-Path $RunDir "harness.o"
$hcLog1 = Join-Path $RunDir "harness_compile_stdout.log"
$hcLog2 = Join-Path $RunDir "harness_compile_stderr.log"
$hcExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" -c `"$harnessC`" -o `"$harnessO`"" $hcLog1 $hcLog2
if ($hcExit -ne 0) { Get-Content $hcLog2; throw "harness compile failed" }

# ---- Step 3: ALWAYS build + run the ORACLE-side harness. ----
$oracleC = Join-Path $RunDir "mixa_event_fifo_oracle.c"
$ocLog1 = Join-Path $RunDir "oracle_trans_stdout.log"
$ocLog2 = Join-Path $RunDir "oracle_trans_stderr.log"
Push-Location $RepoRoot
$ocExit = Invoke-Cmd $L1Trans "mixa_manager\mixa_event_fifo.lm1 `"$oracleC`"" $ocLog1 $ocLog2
Pop-Location
if ($ocExit -ne 0) { Get-Content $ocLog2; throw "oracle mixa_event_fifo.lm1 translation failed" }

$oracleO = Join-Path $RunDir "mixa_event_fifo_oracle.o"
$occLog1 = Join-Path $RunDir "oracle_compile_stdout.log"
$occLog2 = Join-Path $RunDir "oracle_compile_stderr.log"
$occExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -c `"$oracleC`" -o `"$oracleO`"" $occLog1 $occLog2
if ($occExit -ne 0) { Get-Content $occLog2; throw "oracle mixa_event_fifo.lm1 compile failed" }

$oracleExe = Join-Path $RunDir "parity_oracle.exe"
$olLog1 = Join-Path $RunDir "oracle_link_stdout.log"
$olLog2 = Join-Path $RunDir "oracle_link_stderr.log"
$olExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" `"$harnessO`" `"$oracleO`" -o `"$oracleExe`"" $olLog1 $olLog2
if ($olExit -ne 0) { Get-Content $olLog2; throw "oracle harness link failed" }

$oracleRunOut = Join-Path $RunDir "oracle_run_stdout.log"
$oracleRunErr = Join-Path $RunDir "oracle_run_stderr.log"
$oracleRunExit = Invoke-Cmd "`"$oracleExe`"" "" $oracleRunOut $oracleRunErr
$OracleTrace = Get-Content -LiteralPath $oracleRunOut -Raw

# ---- Step 4: attempt the COMPLETE mixa_event_fifo.lm2 translation. ----
Push-Location $RepoRoot
$efSrc = "mixa_manager\mixa_event_fifo.lm2"
$efOut = Join-Path $RunDir "mixa_event_fifo_l2.lm1"
$efStdout = Join-Path $RunDir "eventfifo_stdout.log"
$efStderr = Join-Path $RunDir "eventfifo_stderr.log"
$env:L2_RUNTIME_ROOT = "stg/l1_baseline/l2src/"
$EfExit = Invoke-Cmd "`"$l2exe`"" "`"$efSrc`" `"$efOut`"" $efStdout $efStderr
Pop-Location

$EfStdoutText = if (Test-Path -LiteralPath $efStdout) { Get-Content -LiteralPath $efStdout -Raw } else { "" }
$EfStderrText = if (Test-Path -LiteralPath $efStderr) { Get-Content -LiteralPath $efStderr -Raw } else { "" }
$EfSourceHash = (Get-FileHash -LiteralPath (Join-Path $RepoRoot $efSrc) -Algorithm SHA256).Hash
$EfHeaderHash = (Get-FileHash -LiteralPath (Join-Path $RepoRoot "mixa_manager\mixa_event_fifo_l2.h.lm1") -Algorithm SHA256).Hash
$RunnerHash = (Get-FileHash -LiteralPath $PSCommandPath -Algorithm SHA256).Hash

$Verdict = $null
$ExitCode = 1
$L2TraceText = ""
$DiffText = ""

$KnownBarrier = ($EfStderrText -match "unknown foreign type") -or ($EfStderrText -match "incompatible entry signature")

if ($EfExit -ne 0 -and $KnownBarrier) {
    $Verdict = "EXPECTED_CORE_BARRIER"
    $ExitCode = 2
} elseif ($EfExit -ne 0) {
    $Verdict = "UNEXPECTED_FAILURE"
    $ExitCode = 1
} else {
    # ---- Step 5 (only reached once translation succeeds): build + run
    # the L2-side harness and diff byte-for-byte against the oracle trace. ----
    Push-Location $RepoRoot
    $l2EfC = Join-Path $RunDir "mixa_event_fifo_l2.c"
    $l2ecLog1 = Join-Path $RunDir "l2ef_trans_stdout.log"
    $l2ecLog2 = Join-Path $RunDir "l2ef_trans_stderr.log"
    $l2ecExit = Invoke-Cmd $L1Trans "`"$efOut`" `"$l2EfC`"" $l2ecLog1 $l2ecLog2
    Pop-Location
    if ($l2ecExit -ne 0) {
        Get-Content $l2ecLog2
        $Verdict = "UNEXPECTED_FAILURE"
        $ExitCode = 1
    } else {
        # Generated L2 code spells its runtime `#include`s using the same
        # root-relative path L2_RUNTIME_ROOT gave the predef (Fable's own
        # run_graph_abi.ps1 shape, ticket 20260913-200800): produce that
        # exact header tree inside this run's own directory and point -I
        # at its root, rather than at stg/l1_baseline itself (read-only).
        $L2RuntimeHeaderRoot = Join-Path $RunDir "l2rt_headers"
        $L2RuntimeHeaderTree = Join-Path $L2RuntimeHeaderRoot "stg\l1_baseline\l2src"
        New-Item -ItemType Directory -Force -Path $L2RuntimeHeaderTree | Out-Null
        $L2RuntimeNames = @('lmx_array_owned','lmx_array_ref_owned','lmx_branch_owned','lmx_chars_owned','lmx_graph_copy_owned','lmx_message_graph_copy','lmx_msg_blocks','lmx_msg_history_owned','lmx_msg_liveness','lmx_msg_mail_chain','lmx_msg_path_storage','lmx_msg_roots_stale','lmx_msg_sched_ready','lmx_msg_slots','lmx_msg_storage','lmx_msg_visit','lmx_owned_ranges','lmx_value_owned')
        foreach ($rtName in $L2RuntimeNames) {
            $rtOut = Join-Path $L2RuntimeHeaderTree "$rtName.lm1.h"
            $rtLog1 = Join-Path $RunDir "l2rt_${rtName}_stdout.log"
            $rtLog2 = Join-Path $RunDir "l2rt_${rtName}_stderr.log"
            $rtExit = Invoke-Cmd $L1Trans "stg\l1_baseline\l2src\$rtName.h.lm1 `"$rtOut`"" $rtLog1 $rtLog2
            if ($rtExit -ne 0) { Get-Content $rtLog2; throw "L2 runtime header $rtName translation failed" }
        }
        $l2EfO = Join-Path $RunDir "mixa_event_fifo_l2.o"
        $l2occLog1 = Join-Path $RunDir "l2ef_compile_stdout.log"
        $l2occLog2 = Join-Path $RunDir "l2ef_compile_stderr.log"
        $l2occExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" -I `"$L2RuntimeHeaderRoot`" -c `"$l2EfC`" -o `"$l2EfO`"" $l2occLog1 $l2occLog2
        if ($l2occExit -ne 0) {
            Get-Content $l2occLog2
            $Verdict = "UNEXPECTED_FAILURE"
            $ExitCode = 1
        } else {
            $l2Exe = Join-Path $RunDir "parity_l2.exe"
            $l2olLog1 = Join-Path $RunDir "l2ef_link_stdout.log"
            $l2olLog2 = Join-Path $RunDir "l2ef_link_stderr.log"
            $l2olExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" `"$harnessO`" `"$l2EfO`" -o `"$l2Exe`"" $l2olLog1 $l2olLog2
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
EventFifo-L2-Header: mixa_manager\mixa_event_fifo_l2.h.lm1
EventFifo-L2-Header-Sha256: $EfHeaderHash
EventFifo-L2-Source: $efSrc
EventFifo-L2-Source-Sha256: $EfSourceHash
Harness-Source-Sha256: $((Get-FileHash -LiteralPath (Join-Path $RepoRoot "mixa_manager\tests\mixa_event_fifo_parity_harness.lm1") -Algorithm SHA256).Hash)
Runner-Sha256: $RunnerHash
Oracle-Run-Exit-Code: $oracleRunExit
Oracle-Trace:
$OracleTrace
EventFifo-Translate-Exit-Code: $EfExit
EventFifo-Translate-Stdout:
$EfStdoutText
EventFifo-Translate-Stderr:
$EfStderrText
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
        "EXPECTED_CORE_BARRIER: full mixa_event_fifo.lm2 translation stopped at an already-known barrier (`"unknown foreign type`" or `"incompatible entry signature`" -- core struct-type/@@: return-type admission not yet landed). This is NOT a pass. The oracle-side harness DID run (see Oracle-Trace above) -- all preparatory/oracle checks that are currently possible have been exercised."
    }
    "UNEXPECTED_FAILURE" {
        "UNEXPECTED_FAILURE: translation or build failed with something OTHER than the already-known barriers. Inspect the logs under $RunDir -- this may be a further, distinct compiler barrier; do not invent a manager-side workaround for it."
    }
    "PARITY_FAILURE" {
        "PARITY_FAILURE: both implementations built and ran, but their traces differ (see Diff above) or one exited non-zero. This is a real behavioral divergence -- investigate before reporting anything as a pass."
    }
    "PASS" {
        "PASS: full mixa_event_fifo.lm2 translated, built, and ran; its trace is byte-identical to the L1 oracle's own trace across all scenarios."
    }
}

exit $ExitCode
