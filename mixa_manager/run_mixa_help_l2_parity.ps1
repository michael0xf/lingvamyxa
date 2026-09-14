# Real L1-vs-L2 parity gate for mixa_help (ticket 20260913-145500).
# Own dedicated ABI probe pair (mixa_help_abi_probe_real.c/_l2.c --
# real side compiles against the genuine plain-C mixa_help.h directly).
# mixa_help.lm1/.lm2 are self-contained (no predef of any other manager
# module -- MixaFile/MixaEvent are both plain-C headers), but the
# harness needs real mixa_file_open/close/read/append bodies, so the
# runner separately compiles the real, unmodified mixa_file_win32.o and
# links it alongside the harness and the oracle/L2 object.
#   0. ABI parity gate: dedicated MixaHelpCtx probe pair.
#   1. ALWAYS builds and runs the ORACLE-side harness (real spawned-
#      free, file-backed console via the accepted mixa_file seam).
#   2. Attempts to translate the COMPLETE mixa_help.lm2.
#      - fails with an already-known barrier -> EXPECTED_CORE_BARRIER
#        (exit 2). NOT a pass.
#      - fails with any OTHER diagnostic -> UNEXPECTED_FAILURE (exit 1).
#      - SUCCEEDS -> builds/links/runs the L2-side harness and diffs its
#        stdout against the oracle trace byte-for-byte. PASS (exit 0)
#        only on an exact match, else PARITY_FAILURE (exit 1).
#
# mixa_help.h/mixa_help.lm1 and its real dependency (mixa_file_win32.h/
# .lm1) are the parity oracle (or real, unmodified dependency) and are
# never touched. Nothing under stg/l1_baseline is modified, only read.
# Every input is built fresh in a unique run directory -- no stale
# objects.
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
$LinkLibs = "-lkernel32"

$RunTimestamp = (Get-Date -Format "yyyyMMdd_HHmmss_fff")
$RunGuid = [GUID]::NewGuid().ToString().Substring(0, 8)
$BaseDir = Join-Path $RepoRoot "build\mixa\claude\mixa_help_l2_parity"
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
$l2HdrOut = Join-Path $HeaderDir "mixa_help_l2.lm1.h"
$hdrExit = Invoke-Cmd $L1Trans "mixa_manager\mixa_help_l2.h.lm1 `"$l2HdrOut`"" $hdrLog1 $hdrLog2
Pop-Location
if ($hdrExit -ne 0) { Get-Content $hdrLog2; throw "L2 help header translation failed" }

# ---- Step 0.5: ABI parity gate (dedicated probe pair). ----
$abiRealExe = Join-Path $RunDir "abi_probe_real.exe"
$abiRealLog1 = Join-Path $RunDir "abi_probe_real_compile_stdout.log"
$abiRealLog2 = Join-Path $RunDir "abi_probe_real_compile_stderr.log"
$abiRealExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" `"$RepoRoot\mixa_manager\tests\mixa_help_abi_probe_real.c`" -o `"$abiRealExe`"" $abiRealLog1 $abiRealLog2
if ($abiRealExit -ne 0) { Get-Content $abiRealLog2; throw "ABI probe (real header) compile failed" }

$abiL2Exe = Join-Path $RunDir "abi_probe_l2.exe"
$abiL2Log1 = Join-Path $RunDir "abi_probe_l2_compile_stdout.log"
$abiL2Log2 = Join-Path $RunDir "abi_probe_l2_compile_stderr.log"
$abiL2Exit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" `"$RepoRoot\mixa_manager\tests\mixa_help_abi_probe_l2.c`" -o `"$abiL2Exe`"" $abiL2Log1 $abiL2Log2
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

# ---- Step 2: build the harness object + the real mixa_file_win32.o
# (needed by the harness's own fixture helpers; not predef'd by help
# itself). ----
Push-Location $RepoRoot
$harnessC = Join-Path $RunDir "harness.c"
$hLog1 = Join-Path $RunDir "harness_trans_stdout.log"
$hLog2 = Join-Path $RunDir "harness_trans_stderr.log"
$hExit = Invoke-Cmd $L1Trans "mixa_manager\tests\mixa_help_parity_harness.lm1 `"$harnessC`"" $hLog1 $hLog2
Pop-Location
if ($hExit -ne 0) { Get-Content $hLog2; throw "harness translation failed" }

$harnessO = Join-Path $RunDir "harness.o"
$hcLog1 = Join-Path $RunDir "harness_compile_stdout.log"
$hcLog2 = Join-Path $RunDir "harness_compile_stderr.log"
$hcExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" -c `"$harnessC`" -o `"$harnessO`"" $hcLog1 $hcLog2
if ($hcExit -ne 0) { Get-Content $hcLog2; throw "harness compile failed" }

Push-Location $RepoRoot
$fwC = Join-Path $RunDir "mixa_file_win32.c"
$fwLog1 = Join-Path $RunDir "filewin32_trans_stdout.log"
$fwLog2 = Join-Path $RunDir "filewin32_trans_stderr.log"
$fwExit = Invoke-Cmd $L1Trans "mixa_manager\mixa_file_win32.lm1 `"$fwC`"" $fwLog1 $fwLog2
Pop-Location
if ($fwExit -ne 0) { Get-Content $fwLog2; throw "real mixa_file_win32.lm1 translation failed" }

$fwO = Join-Path $RunDir "mixa_file_win32.o"
$fwcLog1 = Join-Path $RunDir "filewin32_compile_stdout.log"
$fwcLog2 = Join-Path $RunDir "filewin32_compile_stderr.log"
$fwcExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -c `"$fwC`" -o `"$fwO`"" $fwcLog1 $fwcLog2
if ($fwcExit -ne 0) { Get-Content $fwcLog2; throw "real mixa_file_win32.lm1 compile failed" }

# ---- Step 3: ALWAYS build + run the ORACLE-side harness. ----
$oracleC = Join-Path $RunDir "mixa_help_oracle.c"
$ocLog1 = Join-Path $RunDir "oracle_trans_stdout.log"
$ocLog2 = Join-Path $RunDir "oracle_trans_stderr.log"
Push-Location $RepoRoot
$ocExit = Invoke-Cmd $L1Trans "mixa_manager\mixa_help.lm1 `"$oracleC`"" $ocLog1 $ocLog2
Pop-Location
if ($ocExit -ne 0) { Get-Content $ocLog2; throw "oracle mixa_help.lm1 translation failed" }

$oracleO = Join-Path $RunDir "mixa_help_oracle.o"
$occLog1 = Join-Path $RunDir "oracle_compile_stdout.log"
$occLog2 = Join-Path $RunDir "oracle_compile_stderr.log"
$occExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -c `"$oracleC`" -o `"$oracleO`"" $occLog1 $occLog2
if ($occExit -ne 0) { Get-Content $occLog2; throw "oracle mixa_help.lm1 compile failed" }

$oracleExe = Join-Path $RunDir "parity_oracle.exe"
$olLog1 = Join-Path $RunDir "oracle_link_stdout.log"
$olLog2 = Join-Path $RunDir "oracle_link_stderr.log"
$olExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" `"$harnessO`" `"$oracleO`" `"$fwO`" -o `"$oracleExe`" $LinkLibs" $olLog1 $olLog2
if ($olExit -ne 0) { Get-Content $olLog2; throw "oracle harness link failed" }

$OracleFixtureDir = Join-Path $RunDir "oracle_fixtures"
New-Item -ItemType Directory -Force -Path $OracleFixtureDir | Out-Null
Push-Location $RepoRoot
$oracleRunOut = Join-Path $RunDir "oracle_run_stdout.log"
$oracleRunErr = Join-Path $RunDir "oracle_run_stderr.log"
$oracleRunExit = Invoke-Cmd "`"$oracleExe`"" "`"$OracleFixtureDir`"" $oracleRunOut $oracleRunErr
Pop-Location
$OracleTrace = Get-Content -LiteralPath $oracleRunOut -Raw

# ---- Step 4: attempt the COMPLETE mixa_help.lm2 translation. ----
Push-Location $RepoRoot
$hlpSrc = "mixa_manager\mixa_help.lm2"
$hlpOut = Join-Path $RunDir "mixa_help_l2.lm1"
$hlpStdout = Join-Path $RunDir "hlp_stdout.log"
$hlpStderr = Join-Path $RunDir "hlp_stderr.log"
$HlpExit = Invoke-Cmd "`"$l2exe`"" "`"$hlpSrc`" `"$hlpOut`"" $hlpStdout $hlpStderr
Pop-Location

$HlpStdoutText = if (Test-Path -LiteralPath $hlpStdout) { Get-Content -LiteralPath $hlpStdout -Raw } else { "" }
$HlpStderrText = if (Test-Path -LiteralPath $hlpStderr) { Get-Content -LiteralPath $hlpStderr -Raw } else { "" }
$HlpSourceHash = (Get-FileHash -LiteralPath (Join-Path $RepoRoot $hlpSrc) -Algorithm SHA256).Hash
$RunnerHash = (Get-FileHash -LiteralPath $PSCommandPath -Algorithm SHA256).Hash

$Verdict = $null
$ExitCode = 1
$L2TraceText = ""
$DiffText = ""

$KnownBarrier = ($HlpStderrText -match "unknown foreign type") -or ($HlpStderrText -match "incompatible entry signature")

if ($HlpExit -ne 0 -and $KnownBarrier) {
    $Verdict = "EXPECTED_CORE_BARRIER"
    $ExitCode = 2
} elseif ($HlpExit -ne 0) {
    $Verdict = "UNEXPECTED_FAILURE"
    $ExitCode = 1
} else {
    Push-Location $RepoRoot
    $l2HlpC = Join-Path $RunDir "mixa_help_l2.c"
    $l2ccLog1 = Join-Path $RunDir "l2hlp_trans_stdout.log"
    $l2ccLog2 = Join-Path $RunDir "l2hlp_trans_stderr.log"
    $l2ccExit = Invoke-Cmd $L1Trans "`"$hlpOut`" `"$l2HlpC`"" $l2ccLog1 $l2ccLog2
    Pop-Location
    if ($l2ccExit -ne 0) {
        Get-Content $l2ccLog2
        $Verdict = "UNEXPECTED_FAILURE"
        $ExitCode = 1
    } else {
        $l2HlpO = Join-Path $RunDir "mixa_help_l2.o"
        $l2occLog1 = Join-Path $RunDir "l2hlp_compile_stdout.log"
        $l2occLog2 = Join-Path $RunDir "l2hlp_compile_stderr.log"
        $l2occExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" -c `"$l2HlpC`" -o `"$l2HlpO`"" $l2occLog1 $l2occLog2
        if ($l2occExit -ne 0) {
            Get-Content $l2occLog2
            $Verdict = "UNEXPECTED_FAILURE"
            $ExitCode = 1
        } else {
            $l2Exe = Join-Path $RunDir "parity_l2.exe"
            $l2olLog1 = Join-Path $RunDir "l2hlp_link_stdout.log"
            $l2olLog2 = Join-Path $RunDir "l2hlp_link_stderr.log"
            $l2olExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" `"$harnessO`" `"$l2HlpO`" `"$fwO`" -o `"$l2Exe`" $LinkLibs" $l2olLog1 $l2olLog2
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
Help-L2-Header: mixa_manager\mixa_help_l2.h.lm1 (self-contained, no chain)
Help-L2-Source: $hlpSrc
Help-L2-Source-Sha256: $HlpSourceHash
Harness-Source-Sha256: $((Get-FileHash -LiteralPath (Join-Path $RepoRoot "mixa_manager\tests\mixa_help_parity_harness.lm1") -Algorithm SHA256).Hash)
Runner-Sha256: $RunnerHash
Oracle-Run-Exit-Code: $oracleRunExit
Oracle-Trace:
$OracleTrace
Help-Translate-Exit-Code: $HlpExit
Help-Translate-Stdout:
$HlpStdoutText
Help-Translate-Stderr:
$HlpStderrText
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
        "EXPECTED_CORE_BARRIER: full mixa_help.lm2 translation stopped at an already-known barrier. This is NOT a pass -- the integrated frontend is not yet on main. The oracle-side harness DID run (see Oracle-Trace above)."
    }
    "UNEXPECTED_FAILURE" {
        "UNEXPECTED_FAILURE: translation or build failed with something OTHER than the already-known barriers. Inspect the logs under $RunDir."
    }
    "PARITY_FAILURE" {
        "PARITY_FAILURE: both implementations built and ran, but their traces differ (see Diff above) or one exited non-zero."
    }
    "PASS" {
        "PASS: full mixa_help.lm2 translated, built, and ran; its trace is byte-identical to the L1 oracle's own trace."
    }
}

exit $ExitCode
