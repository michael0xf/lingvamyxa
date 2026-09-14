# Real L1-vs-L2 parity gate for mixa_cmdline (ticket 20260913-124500).
# Same structure as run_mixa_pointer_l2_parity.ps1 (113000): fully self-
# contained, own dedicated ABI probe pair (mixa_cmdline_abi_probe_
# real.c/_l2.c -- real side compiles against the genuine plain-C
# mixa_cmdline.h directly, unlike mixa_process_marker/mixa_console_
# window which have no separate plain-C production header), no other
# manager module linked, no Win32 API surface needed at all.
#   0. ABI parity gate: dedicated MixaCmdLineFaultVTable/MixaCmdLine
#      probe pair.
#   1. ALWAYS builds and runs the ORACLE-side harness (real mixa_
#      cmdline.lm1, unmodified).
#   2. Attempts to translate the COMPLETE mixa_cmdline.lm2.
#      - fails with an already-known barrier -> EXPECTED_CORE_BARRIER
#        (exit 2). NOT a pass.
#      - fails with any OTHER diagnostic -> UNEXPECTED_FAILURE (exit 1).
#      - SUCCEEDS -> builds/links/runs the L2-side harness and diffs its
#        stdout against the oracle trace byte-for-byte. PASS (exit 0)
#        only on an exact match, else PARITY_FAILURE (exit 1).
#
# mixa_cmdline.h/mixa_cmdline.lm1 are the parity oracle and are never
# touched. Nothing under stg/l1_baseline is modified, only read. Every
# input is built fresh in a unique run directory -- no stale objects.
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
$BaseDir = Join-Path $RepoRoot "build\mixa\claude\mixa_cmdline_l2_parity"
$RunDir = Join-Path $BaseDir "run_${RunTimestamp}_${RunGuid}"
$HeaderDir = Join-Path $RunDir "headers\mixa_manager"
New-Item -ItemType Directory -Force -Path $HeaderDir | Out-Null

function Invoke-Cmd([string]$exe, [string]$argsStr, [string]$outLog, [string]$errLog) {
    & cmd /c "$exe $argsStr > `"$outLog`" 2> `"$errLog`""
    return $LASTEXITCODE
}

# ---- Step 0: translate the L2 header unit. ----
Push-Location $RepoRoot
$hdrLog1 = Join-Path $RunDir "header_trans_stdout.log"
$hdrLog2 = Join-Path $RunDir "header_trans_stderr.log"
$l2HdrOut = Join-Path $HeaderDir "mixa_cmdline_l2.lm1.h"
$hdrExit = Invoke-Cmd $L1Trans "mixa_manager\mixa_cmdline_l2.h.lm1 `"$l2HdrOut`"" $hdrLog1 $hdrLog2
Pop-Location
if ($hdrExit -ne 0) { Get-Content $hdrLog2; throw "L2 cmdline header translation failed" }

# ---- Step 0.5: ABI parity gate (dedicated probe pair). ----
$abiRealExe = Join-Path $RunDir "abi_probe_real.exe"
$abiRealLog1 = Join-Path $RunDir "abi_probe_real_compile_stdout.log"
$abiRealLog2 = Join-Path $RunDir "abi_probe_real_compile_stderr.log"
$abiRealExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" `"$RepoRoot\mixa_manager\tests\mixa_cmdline_abi_probe_real.c`" -o `"$abiRealExe`"" $abiRealLog1 $abiRealLog2
if ($abiRealExit -ne 0) { Get-Content $abiRealLog2; throw "ABI probe (real header) compile failed" }

$abiL2Exe = Join-Path $RunDir "abi_probe_l2.exe"
$abiL2Log1 = Join-Path $RunDir "abi_probe_l2_compile_stdout.log"
$abiL2Log2 = Join-Path $RunDir "abi_probe_l2_compile_stderr.log"
$abiL2Exit = Invoke-Cmd "gcc" "$GccStd -I `"$RunDir\headers`" `"$RepoRoot\mixa_manager\tests\mixa_cmdline_abi_probe_l2.c`" -o `"$abiL2Exe`"" $abiL2Log1 $abiL2Log2
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

# ---- Step 2: build the harness object (compiled once, linked twice). ----
Push-Location $RepoRoot
$harnessC = Join-Path $RunDir "harness.c"
$hLog1 = Join-Path $RunDir "harness_trans_stdout.log"
$hLog2 = Join-Path $RunDir "harness_trans_stderr.log"
$hExit = Invoke-Cmd $L1Trans "mixa_manager\tests\mixa_cmdline_parity_harness.lm1 `"$harnessC`"" $hLog1 $hLog2
Pop-Location
if ($hExit -ne 0) { Get-Content $hLog2; throw "harness translation failed" }

$harnessO = Join-Path $RunDir "harness.o"
$hcLog1 = Join-Path $RunDir "harness_compile_stdout.log"
$hcLog2 = Join-Path $RunDir "harness_compile_stderr.log"
$hcExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" -c `"$harnessC`" -o `"$harnessO`"" $hcLog1 $hcLog2
if ($hcExit -ne 0) { Get-Content $hcLog2; throw "harness compile failed" }

# ---- Step 3: ALWAYS build + run the ORACLE-side harness. ----
$oracleC = Join-Path $RunDir "mixa_cmdline_oracle.c"
$ocLog1 = Join-Path $RunDir "oracle_trans_stdout.log"
$ocLog2 = Join-Path $RunDir "oracle_trans_stderr.log"
Push-Location $RepoRoot
$ocExit = Invoke-Cmd $L1Trans "mixa_manager\mixa_cmdline.lm1 `"$oracleC`"" $ocLog1 $ocLog2
Pop-Location
if ($ocExit -ne 0) { Get-Content $ocLog2; throw "oracle mixa_cmdline.lm1 translation failed" }

$oracleO = Join-Path $RunDir "mixa_cmdline_oracle.o"
$occLog1 = Join-Path $RunDir "oracle_compile_stdout.log"
$occLog2 = Join-Path $RunDir "oracle_compile_stderr.log"
$occExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -c `"$oracleC`" -o `"$oracleO`"" $occLog1 $occLog2
if ($occExit -ne 0) { Get-Content $occLog2; throw "oracle mixa_cmdline.lm1 compile failed" }

$oracleExe = Join-Path $RunDir "parity_oracle.exe"
$olLog1 = Join-Path $RunDir "oracle_link_stdout.log"
$olLog2 = Join-Path $RunDir "oracle_link_stderr.log"
$olExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" `"$harnessO`" `"$oracleO`" -o `"$oracleExe`"" $olLog1 $olLog2
if ($olExit -ne 0) { Get-Content $olLog2; throw "oracle harness link failed" }

$oracleRunOut = Join-Path $RunDir "oracle_run_stdout.log"
$oracleRunErr = Join-Path $RunDir "oracle_run_stderr.log"
$oracleRunExit = Invoke-Cmd "`"$oracleExe`"" "" $oracleRunOut $oracleRunErr
$OracleTrace = Get-Content -LiteralPath $oracleRunOut -Raw

# ---- Step 4: attempt the COMPLETE mixa_cmdline.lm2 translation. ----
Push-Location $RepoRoot
$cmdSrc = "mixa_manager\mixa_cmdline.lm2"
$cmdOut = Join-Path $RunDir "mixa_cmdline_l2.lm1"
$cmdStdout = Join-Path $RunDir "cmd_stdout.log"
$cmdStderr = Join-Path $RunDir "cmd_stderr.log"
$env:L2_RUNTIME_ROOT = "stg/l1_baseline/l2src/"
$CmdExit = Invoke-Cmd "`"$l2exe`"" "`"$cmdSrc`" `"$cmdOut`"" $cmdStdout $cmdStderr
Pop-Location

$CmdStdoutText = if (Test-Path -LiteralPath $cmdStdout) { Get-Content -LiteralPath $cmdStdout -Raw } else { "" }
$CmdStderrText = if (Test-Path -LiteralPath $cmdStderr) { Get-Content -LiteralPath $cmdStderr -Raw } else { "" }
$CmdSourceHash = (Get-FileHash -LiteralPath (Join-Path $RepoRoot $cmdSrc) -Algorithm SHA256).Hash
$RunnerHash = (Get-FileHash -LiteralPath $PSCommandPath -Algorithm SHA256).Hash

$Verdict = $null
$ExitCode = 1
$L2TraceText = ""
$DiffText = ""

$KnownBarrier = ($CmdStderrText -match "unknown foreign type") -or ($CmdStderrText -match "incompatible entry signature")

if ($CmdExit -ne 0 -and $KnownBarrier) {
    $Verdict = "EXPECTED_CORE_BARRIER"
    $ExitCode = 2
} elseif ($CmdExit -ne 0) {
    $Verdict = "UNEXPECTED_FAILURE"
    $ExitCode = 1
} else {
    Push-Location $RepoRoot
    $l2CmdC = Join-Path $RunDir "mixa_cmdline_l2.c"
    $l2ccLog1 = Join-Path $RunDir "l2cmd_trans_stdout.log"
    $l2ccLog2 = Join-Path $RunDir "l2cmd_trans_stderr.log"
    $l2ccExit = Invoke-Cmd $L1Trans "`"$cmdOut`" `"$l2CmdC`"" $l2ccLog1 $l2ccLog2
    Pop-Location
    if ($l2ccExit -ne 0) {
        Get-Content $l2ccLog2
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
        # These runtime headers cross-import each other with paths relative
        # to stg/l1_baseline itself, so l1trans must run from $L1Root or it
        # fails with "cannot read import l2src/....h.lm1" (confirmed by
        # direct reproduction). $rtOut is absolute, so the OUTPUT location
        # is unaffected by cwd.
        Push-Location $L1Root
        foreach ($rtName in $L2RuntimeNames) {
            $rtOut = Join-Path $L2RuntimeHeaderTree "$rtName.lm1.h"
            $rtLog1 = Join-Path $RunDir "l2rt_${rtName}_stdout.log"
            $rtLog2 = Join-Path $RunDir "l2rt_${rtName}_stderr.log"
            $rtExit = Invoke-Cmd $L1Trans "l2src\$rtName.h.lm1 `"$rtOut`"" $rtLog1 $rtLog2
            if ($rtExit -ne 0) { Pop-Location; Get-Content $rtLog2; throw "L2 runtime header $rtName translation failed" }
        }
        Pop-Location
        # The generated L2 code calls the real graph/Message runtime
        # (lmx_msg_create, lmx_branch_struct_known, lmx_int_new_owned, etc.
        # -- ticket 20260913-232000), which lives in the same 19 modules
        # run_graph_abi.ps1 itself builds as "message support" objects
        # (its own $names list plus lmx_message.lm1), plus two plain,
        # hand-written C files it also links (lmx_message_host.c,
        # lmx_message_exec.c). Built once per run, same cwd=$L1Root
        # reasoning as the headers above.
        $L2RuntimeObjDir = Join-Path $RunDir "l2rt_objs"
        New-Item -ItemType Directory -Force -Path $L2RuntimeObjDir | Out-Null
        $L2RuntimeModuleNames = $L2RuntimeNames + @('lmx_message')
        $L2RuntimeObjs = @()
        Push-Location $L1Root
        foreach ($rtName in $L2RuntimeModuleNames) {
            $rtSrcC = Join-Path $L2RuntimeObjDir "$rtName.c"
            $rtTLog1 = Join-Path $RunDir "l2rtobj_${rtName}_trans_stdout.log"
            $rtTLog2 = Join-Path $RunDir "l2rtobj_${rtName}_trans_stderr.log"
            $rtTExit = Invoke-Cmd $L1Trans "l2src\$rtName.lm1 `"$rtSrcC`"" $rtTLog1 $rtTLog2
            if ($rtTExit -ne 0) { Pop-Location; Get-Content $rtTLog2; throw "L2 runtime module $rtName translation failed" }
            $rtObj = Join-Path $L2RuntimeObjDir "$rtName.o"
            $rtCLog1 = Join-Path $RunDir "l2rtobj_${rtName}_compile_stdout.log"
            $rtCLog2 = Join-Path $RunDir "l2rtobj_${rtName}_compile_stderr.log"
            $rtCExit = Invoke-Cmd "gcc" "$GccStd -I `"$L2RuntimeHeaderRoot\stg\l1_baseline`" -I `"$L1Root`" -c `"$rtSrcC`" -o `"$rtObj`"" $rtCLog1 $rtCLog2
            if ($rtCExit -ne 0) { Pop-Location; Get-Content $rtCLog2; throw "L2 runtime module $rtName compile failed" }
            $L2RuntimeObjs += $rtObj
        }
        foreach ($plainName in @('lmx_message_host', 'lmx_message_exec')) {
            $plainObj = Join-Path $L2RuntimeObjDir "$plainName.o"
            $plainCLog1 = Join-Path $RunDir "l2rtobj_${plainName}_compile_stdout.log"
            $plainCLog2 = Join-Path $RunDir "l2rtobj_${plainName}_compile_stderr.log"
            $plainCExit = Invoke-Cmd "gcc" "$GccStd -I `"$L2RuntimeHeaderRoot\stg\l1_baseline`" -I `"$L1Root`" -c `"l2src\$plainName.c`" -o `"$plainObj`"" $plainCLog1 $plainCLog2
            if ($plainCExit -ne 0) { Pop-Location; Get-Content $plainCLog2; throw "L2 runtime support $plainName compile failed" }
            $L2RuntimeObjs += $plainObj
        }
        Pop-Location
        $L2RuntimeObjList = ($L2RuntimeObjs | ForEach-Object { '"' + $_ + '"' }) -join ' '
        $l2CmdO = Join-Path $RunDir "mixa_cmdline_l2.o"
        $l2occLog1 = Join-Path $RunDir "l2cmd_compile_stdout.log"
        $l2occLog2 = Join-Path $RunDir "l2cmd_compile_stderr.log"
        # Two extra -I paths: the generated headers themselves #include each
        # other and the real lmx.h using paths relative to stg/l1_baseline
        # (e.g. "l2src/lmx.h") rather than the "stg/l1_baseline/l2src/..."
        # spelling the OUTER generated .c uses for its own predef-driven
        # #include.
        $l2occExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" -I `"$L2RuntimeHeaderRoot`" -I `"$L2RuntimeHeaderRoot\stg\l1_baseline`" -I `"$L1Root`" -c `"$l2CmdC`" -o `"$l2CmdO`"" $l2occLog1 $l2occLog2
        if ($l2occExit -ne 0) {
            Get-Content $l2occLog2
            $Verdict = "UNEXPECTED_FAILURE"
            $ExitCode = 1
        } else {
            $l2Exe = Join-Path $RunDir "parity_l2.exe"
            $l2olLog1 = Join-Path $RunDir "l2cmd_link_stdout.log"
            $l2olLog2 = Join-Path $RunDir "l2cmd_link_stderr.log"
            $l2olExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" `"$harnessO`" `"$l2CmdO`" $L2RuntimeObjList -o `"$l2Exe`"" $l2olLog1 $l2olLog2
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
Cmdline-L2-Header: mixa_manager\mixa_cmdline_l2.h.lm1 (self-contained, no chain)
Cmdline-L2-Source: $cmdSrc
Cmdline-L2-Source-Sha256: $CmdSourceHash
Harness-Source-Sha256: $((Get-FileHash -LiteralPath (Join-Path $RepoRoot "mixa_manager\tests\mixa_cmdline_parity_harness.lm1") -Algorithm SHA256).Hash)
Runner-Sha256: $RunnerHash
Oracle-Run-Exit-Code: $oracleRunExit
Oracle-Trace:
$OracleTrace
Cmdline-Translate-Exit-Code: $CmdExit
Cmdline-Translate-Stdout:
$CmdStdoutText
Cmdline-Translate-Stderr:
$CmdStderrText
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
        "EXPECTED_CORE_BARRIER: full mixa_cmdline.lm2 translation stopped at an already-known barrier. This is NOT a pass -- the integrated frontend is not yet on main. The oracle-side harness DID run (see Oracle-Trace above)."
    }
    "UNEXPECTED_FAILURE" {
        "UNEXPECTED_FAILURE: translation or build failed with something OTHER than the already-known barriers. Inspect the logs under $RunDir."
    }
    "PARITY_FAILURE" {
        "PARITY_FAILURE: both implementations built and ran, but their traces differ (see Diff above) or one exited non-zero."
    }
    "PASS" {
        "PASS: full mixa_cmdline.lm2 translated, built, and ran; its trace is byte-identical to the L1 oracle's own trace."
    }
}

exit $ExitCode
