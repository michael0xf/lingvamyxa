# Real L1-vs-L2 parity gate for mixa_app_path (ticket 20260913-142800).
# Same structure as run_mixa_cmdline_l2_parity.ps1 (124500): fully self-
# contained, own dedicated ABI probe pair (mixa_app_path_abi_probe_
# real.c/_l2.c -- real side compiles against the genuine plain-C mixa_
# app_path.h directly), no other manager module linked, no Win32 API
# surface at all.
#   0. ABI parity gate: dedicated MixaAppPathFaultVTable probe pair.
#   1. ALWAYS builds and runs the ORACLE-side harness (real mixa_
#      app_path.lm1, unmodified).
#   2. Attempts to translate the COMPLETE mixa_app_path.lm2.
#      - fails with an already-known barrier -> EXPECTED_CORE_BARRIER
#        (exit 2). NOT a pass.
#      - fails with any OTHER diagnostic -> UNEXPECTED_FAILURE (exit 1).
#      - SUCCEEDS -> builds/links/runs the L2-side harness and diffs its
#        stdout against the oracle trace byte-for-byte. PASS (exit 0)
#        only on an exact match, else PARITY_FAILURE (exit 1).
#
# mixa_app_path.h/mixa_app_path.lm1 are the parity oracle and are never
# touched. Nothing under stg/l1_baseline is modified, only read. Every
# input is built fresh in a unique run directory -- no stale objects.
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

$RunTimestamp = (Get-Date -Format "yyyyMMdd_HHmmss_fff")
$RunGuid = [GUID]::NewGuid().ToString().Substring(0, 8)
$BaseDir = Join-Path $RepoRoot "build\mixa\claude\mixa_app_path_l2_parity"
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
$l2HdrOut = Join-Path $HeaderDir "mixa_app_path_l2.lm1.h"
$hdrExit = Invoke-Cmd $L1Trans "mixa_manager\mixa_app_path_l2.h.lm1 `"$l2HdrOut`"" $hdrLog1 $hdrLog2
Pop-Location
if ($hdrExit -ne 0) { Get-Content $hdrLog2; throw "L2 app_path header translation failed" }

# ---- Step 0.5: ABI parity gate (dedicated probe pair). ----
$abiRealExe = Join-Path $RunDir "abi_probe_real.exe"
$abiRealLog1 = Join-Path $RunDir "abi_probe_real_compile_stdout.log"
$abiRealLog2 = Join-Path $RunDir "abi_probe_real_compile_stderr.log"
$abiRealExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" `"$RepoRoot\mixa_manager\tests\mixa_app_path_abi_probe_real.c`" -o `"$abiRealExe`"" $abiRealLog1 $abiRealLog2
if ($abiRealExit -ne 0) { Get-Content $abiRealLog2; throw "ABI probe (real header) compile failed" }

$abiL2Exe = Join-Path $RunDir "abi_probe_l2.exe"
$abiL2Log1 = Join-Path $RunDir "abi_probe_l2_compile_stdout.log"
$abiL2Log2 = Join-Path $RunDir "abi_probe_l2_compile_stderr.log"
$abiL2Exit = Invoke-Cmd "gcc" "$GccStd -I `"$RunDir\headers`" `"$RepoRoot\mixa_manager\tests\mixa_app_path_abi_probe_l2.c`" -o `"$abiL2Exe`"" $abiL2Log1 $abiL2Log2
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

# ---- Step 1: select an already-verified L2 translator or build one fresh. ----
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
        & $L1Trans $L2TransSourceRel $l2c
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

# ---- Step 2: build the harness object (compiled once, linked twice). ----
Push-Location $RepoRoot
$harnessC = Join-Path $RunDir "harness.c"
$hLog1 = Join-Path $RunDir "harness_trans_stdout.log"
$hLog2 = Join-Path $RunDir "harness_trans_stderr.log"
$hExit = Invoke-Cmd $L1Trans "mixa_manager\tests\mixa_app_path_parity_harness.lm1 `"$harnessC`"" $hLog1 $hLog2
Pop-Location
if ($hExit -ne 0) { Get-Content $hLog2; throw "harness translation failed" }

$harnessO = Join-Path $RunDir "harness.o"
$hcLog1 = Join-Path $RunDir "harness_compile_stdout.log"
$hcLog2 = Join-Path $RunDir "harness_compile_stderr.log"
$hcExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" -c `"$harnessC`" -o `"$harnessO`"" $hcLog1 $hcLog2
if ($hcExit -ne 0) { Get-Content $hcLog2; throw "harness compile failed" }

# ---- Step 3: ALWAYS build + run the ORACLE-side harness. ----
$oracleC = Join-Path $RunDir "mixa_app_path_oracle.c"
$ocLog1 = Join-Path $RunDir "oracle_trans_stdout.log"
$ocLog2 = Join-Path $RunDir "oracle_trans_stderr.log"
Push-Location $RepoRoot
$ocExit = Invoke-Cmd $L1Trans "mixa_manager\mixa_app_path.lm1 `"$oracleC`"" $ocLog1 $ocLog2
Pop-Location
if ($ocExit -ne 0) { Get-Content $ocLog2; throw "oracle mixa_app_path.lm1 translation failed" }

$oracleO = Join-Path $RunDir "mixa_app_path_oracle.o"
$occLog1 = Join-Path $RunDir "oracle_compile_stdout.log"
$occLog2 = Join-Path $RunDir "oracle_compile_stderr.log"
$occExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -c `"$oracleC`" -o `"$oracleO`"" $occLog1 $occLog2
if ($occExit -ne 0) { Get-Content $occLog2; throw "oracle mixa_app_path.lm1 compile failed" }

$oracleExe = Join-Path $RunDir "parity_oracle.exe"
$olLog1 = Join-Path $RunDir "oracle_link_stdout.log"
$olLog2 = Join-Path $RunDir "oracle_link_stderr.log"
$olExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" `"$harnessO`" `"$oracleO`" -o `"$oracleExe`"" $olLog1 $olLog2
if ($olExit -ne 0) { Get-Content $olLog2; throw "oracle harness link failed" }

$oracleRunOut = Join-Path $RunDir "oracle_run_stdout.log"
$oracleRunErr = Join-Path $RunDir "oracle_run_stderr.log"
$oracleRunExit = Invoke-Cmd "`"$oracleExe`"" "" $oracleRunOut $oracleRunErr
$OracleTrace = Get-Content -LiteralPath $oracleRunOut -Raw

# ---- Step 4: attempt the COMPLETE mixa_app_path.lm2 translation. ----
Push-Location $RepoRoot
$apSrc = "mixa_manager\mixa_app_path.lm2"
$apOut = Join-Path $RunDir "mixa_app_path_l2.lm1"
$apStdout = Join-Path $RunDir "ap_stdout.log"
$apStderr = Join-Path $RunDir "ap_stderr.log"
$ApExit = Invoke-Cmd "`"$l2exe`"" "`"$apSrc`" `"$apOut`"" $apStdout $apStderr
Pop-Location

$ApStdoutText = if (Test-Path -LiteralPath $apStdout) { Get-Content -LiteralPath $apStdout -Raw } else { "" }
$ApStderrText = if (Test-Path -LiteralPath $apStderr) { Get-Content -LiteralPath $apStderr -Raw } else { "" }
$ApSourceHash = (Get-FileHash -LiteralPath (Join-Path $RepoRoot $apSrc) -Algorithm SHA256).Hash
$RunnerHash = (Get-FileHash -LiteralPath $PSCommandPath -Algorithm SHA256).Hash

$Verdict = $null
$ExitCode = 1
$L2TraceText = ""
$DiffText = ""

$KnownBarrier = ($ApStderrText -match "unknown foreign type") -or ($ApStderrText -match "incompatible entry signature")

if ($ApExit -ne 0 -and $KnownBarrier) {
    $Verdict = "EXPECTED_CORE_BARRIER"
    $ExitCode = 2
} elseif ($ApExit -ne 0) {
    $Verdict = "UNEXPECTED_FAILURE"
    $ExitCode = 1
} else {
    # l2trans emits repository-relative predef paths.  Recreate that read-only
    # import view inside this run directory so the generated unit can be
    # compiled without modifying the pinned staged tree.
    $runL2Src = Join-Path $RunDir "l2src"
    $runMixa = Join-Path $RunDir "mixa_manager"
    New-Item -ItemType Directory -Force -Path $runL2Src, $runMixa | Out-Null
    Get-ChildItem -LiteralPath (Join-Path $L1Root "l2src") -Filter "*.h.lm1" |
        Copy-Item -Destination $runL2Src
    Copy-Item -LiteralPath (Join-Path $RepoRoot "mixa_manager\mixa_app_path_l2.h.lm1") -Destination $runMixa

    Push-Location $RunDir
    try {
        Get-ChildItem -LiteralPath $runL2Src -Filter "*.h.lm1" | ForEach-Object {
            $compiledName = $_.Name.Substring(0, $_.Name.Length - ".h.lm1".Length) + ".lm1.h"
            & $L2L1Trans ("l2src\" + $_.Name) (Join-Path $runL2Src $compiledName)
            if ($LASTEXITCODE -ne 0) { throw "failed to compile L2 support header $($_.Name)" }
        }
    } finally {
        Pop-Location
    }

    Push-Location $RunDir
    $l2ApC = Join-Path $RunDir "mixa_app_path_l2.c"
    $l2ccLog1 = Join-Path $RunDir "l2ap_trans_stdout.log"
    $l2ccLog2 = Join-Path $RunDir "l2ap_trans_stderr.log"
    $l2ccExit = Invoke-Cmd $L2L1Trans "`"$apOut`" `"$l2ApC`"" $l2ccLog1 $l2ccLog2
    Pop-Location
    if ($l2ccExit -ne 0) {
        Get-Content $l2ccLog2
        $Verdict = "UNEXPECTED_FAILURE"
        $ExitCode = 1
    } else {
        $l2ApO = Join-Path $RunDir "mixa_app_path_l2.o"
        $l2occLog1 = Join-Path $RunDir "l2ap_compile_stdout.log"
        $l2occLog2 = Join-Path $RunDir "l2ap_compile_stderr.log"
        $l2occExit = Invoke-Cmd "gcc" "$GccStd -I `"$RunDir`" -I `"$RepoRoot`" -I `"$L1Root`" -I `"$RunDir\headers`" -c `"$l2ApC`" -o `"$l2ApO`"" $l2occLog1 $l2occLog2
        if ($l2occExit -ne 0) {
            Get-Content $l2occLog2
            $Verdict = "UNEXPECTED_FAILURE"
            $ExitCode = 1
        } else {
            $runtimeDir = Join-Path $RunDir "message_runtime"
            New-Item -ItemType Directory -Force -Path $runtimeDir | Out-Null
            $runtimeNames = @(
                "lmx_msg_blocks", "lmx_owned_ranges", "lmx_msg_storage", "lmx_msg_liveness",
                "lmx_msg_history_owned", "lmx_msg_roots_stale", "lmx_msg_path_storage",
                "lmx_msg_slots", "lmx_msg_mail_chain", "lmx_msg_sched_ready", "lmx_msg_visit",
                "lmx_branch_owned", "lmx_value_owned", "lmx_chars_owned", "lmx_array_owned",
                "lmx_array_ref_owned", "lmx_graph_copy_owned", "lmx_message_graph_copy"
            )
            $runtimeSources = @()
            Push-Location $L1Root
            try {
                foreach ($runtimeName in $runtimeNames) {
                    $runtimeC = Join-Path $runtimeDir ($runtimeName + ".c")
                    & $L2L1Trans ("l2src\" + $runtimeName + ".lm1") $runtimeC
                    if ($LASTEXITCODE -ne 0) { throw "failed to translate Message runtime source $runtimeName" }
                    $runtimeSources += $runtimeC
                }
                $messageC = Join-Path $runtimeDir "lmx_message.c"
                & $L2L1Trans "l2src\lmx_message.lm1" $messageC
                if ($LASTEXITCODE -ne 0) { throw "failed to translate Message runtime source lmx_message" }
                $runtimeSources += $messageC
            } finally {
                Pop-Location
            }
            $runtimeSources += (Join-Path $L1Root "l2src\lmx_message_host.c")
            $runtimeSources += (Join-Path $L1Root "l2src\lmx_message_exec.c")
            $runtimeObjects = @()
            foreach ($runtimeSource in $runtimeSources) {
                $runtimeObject = Join-Path $runtimeDir (([IO.Path]::GetFileNameWithoutExtension($runtimeSource)) + ".o")
                $runtimeCompileExit = Invoke-Cmd "gcc" "$GccStd -I `"$RunDir`" -I `"$L1Root`" -I `"$L1Root\lm1\build`" -c `"$runtimeSource`" -o `"$runtimeObject`"" (Join-Path $runtimeDir (([IO.Path]::GetFileName($runtimeSource)) + ".stdout.log")) (Join-Path $runtimeDir (([IO.Path]::GetFileName($runtimeSource)) + ".stderr.log"))
                if ($runtimeCompileExit -ne 0) { throw "failed to compile Message runtime source $runtimeSource" }
                $runtimeObjects += $runtimeObject
            }
            $l2Exe = Join-Path $RunDir "parity_l2.exe"
            $l2olLog1 = Join-Path $RunDir "l2ap_link_stdout.log"
            $l2olLog2 = Join-Path $RunDir "l2ap_link_stderr.log"
            $runtimeObjectArgs = ($runtimeObjects | ForEach-Object { '"' + $_ + '"' }) -join ' '
            $l2olExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" `"$harnessO`" `"$l2ApO`" $runtimeObjectArgs -o `"$l2Exe`"" $l2olLog1 $l2olLog2
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
L2-output-L1-Translator: $L2L1Trans
L2-output-L1-Translator-Sha256: $L2L1TransHash
L2trans-Source (informational, not pinned): $L2TransSource
L2trans-Source-Sha256 (informational, not pinned): $L2TransSourceHash
L2trans-Exe: $l2exe
L2trans-Exe-Sha256: $L2TransExeHash
AppPath-L2-Header: mixa_manager\mixa_app_path_l2.h.lm1 (self-contained, no chain)
AppPath-L2-Source: $apSrc
AppPath-L2-Source-Sha256: $ApSourceHash
Harness-Source-Sha256: $((Get-FileHash -LiteralPath (Join-Path $RepoRoot "mixa_manager\tests\mixa_app_path_parity_harness.lm1") -Algorithm SHA256).Hash)
Runner-Sha256: $RunnerHash
Oracle-Run-Exit-Code: $oracleRunExit
Oracle-Trace:
$OracleTrace
AppPath-Translate-Exit-Code: $ApExit
AppPath-Translate-Stdout:
$ApStdoutText
AppPath-Translate-Stderr:
$ApStderrText
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
        "EXPECTED_CORE_BARRIER: full mixa_app_path.lm2 translation stopped at an already-known barrier. This is NOT a pass -- the integrated frontend is not yet on main. The oracle-side harness DID run (see Oracle-Trace above)."
    }
    "UNEXPECTED_FAILURE" {
        "UNEXPECTED_FAILURE: translation or build failed with something OTHER than the already-known barriers. Inspect the logs under $RunDir."
    }
    "PARITY_FAILURE" {
        "PARITY_FAILURE: both implementations built and ran, but their traces differ (see Diff above) or one exited non-zero."
    }
    "PASS" {
        "PASS: full mixa_app_path.lm2 translated, built, and ran; its trace is byte-identical to the L1 oracle's own trace."
    }
}

exit $ExitCode
