# Real L1-vs-L2 parity gate for mixa_share_win32 (share chain ticket
# 20260913-192700, module 4 of 6). No dedicated ABI probe pair: this
# module declares no new struct of its own beyond what mixa_share_
# win32.h.lm1 already declares, reached via the REAL header predef'd
# directly, identically on both the real oracle and this L2 port.
#
# Scope, stated honestly (see the harness's own top comment for the
# full rationale): this module's real full round trip needs a live
# native window and drives the real OS Share UI -- that is exactly
# what the existing, already-accepted mixa_share_native_smoke.lm1
# already covers (manual/native only, requiring the Windows 10 SDK's
# own plain-C WinRT headers, deliberately NOT part of run_mixa.ps1's
# automated regression). This gate instead covers every DETERMINISTIC,
# non-UI-dependent piece of this module's own real logic: real Win32
# file/directory/missing classification, the real owner-thread guard,
# and the plain-C COM object mechanics (QueryInterface/AddRef/Release,
# including the real free_counter lifetime proof) for both COM handler
# object types this module hands out to WinRT.
#
# This module needs the Windows 10 SDK's plain-C WinRT headers (not
# bundled with MinGW) to compile at all (mixa_share_win32.h.lm1's own
# `<windows.applicationmodel.datatransfer.h>` include) -- the same
# -idirafter route and link libraries (-lruntimeobject -luser32 -lole32)
# the existing run_share_native_smoke.ps1 already uses, reused
# identically here.
#
#   1. ALWAYS builds and runs the ORACLE-side harness.
#   2. Attempts to translate the COMPLETE mixa_share_win32.lm2.
#      - fails with an already-known barrier -> EXPECTED_CORE_BARRIER
#        (exit 2). NOT a pass.
#      - fails with any OTHER diagnostic -> UNEXPECTED_FAILURE (exit 1).
#      - SUCCEEDS -> builds/links/runs the L2-side harness and diffs its
#        stdout against the oracle trace byte-for-byte. PASS (exit 0)
#        only on an exact match, else PARITY_FAILURE (exit 1).
#
# mixa_share_win32.h.lm1/.lm1 and mixa_share.h.lm1 are the parity
# oracle (or real, unmodified dependencies) and are never touched.
# Nothing under stg/l1_baseline is modified, only read. Every input is
# built fresh in a unique run directory -- no stale objects. No real
# window is ever opened and no real Share UI is ever invoked by this
# gate.
param(
    [string]$WindowsSdkIncludeRoot = "C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0"
)
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

$SdkWinrtDir = Join-Path $WindowsSdkIncludeRoot "winrt"
if (-not (Test-Path -LiteralPath $SdkWinrtDir)) {
    throw "Windows SDK winrt include dir not found: $SdkWinrtDir (pass -WindowsSdkIncludeRoot to override)"
}

$guards = @(
    "-Werror=incompatible-pointer-types", "-Werror=discarded-qualifiers",
    "-Werror=implicit-function-declaration", "-Werror=implicit-int"
)
$GccStd = "-std=c99 -Wall -Wextra -Wpedantic $($guards -join ' ')"
$GccLibs = "-lruntimeobject -luser32 -lole32"

$RunTimestamp = (Get-Date -Format "yyyyMMdd_HHmmss_fff")
$RunGuid = [GUID]::NewGuid().ToString().Substring(0, 8)
$BaseDir = Join-Path $RepoRoot "build\mixa\claude\mixa_share_win32_l2_parity"
$RunDir = Join-Path $BaseDir "run_${RunTimestamp}_${RunGuid}"
$HeaderDir = Join-Path $RunDir "headers\mixa_manager"
New-Item -ItemType Directory -Force -Path $HeaderDir | Out-Null

function Invoke-Cmd([string]$exe, [string]$argsStr, [string]$outLog, [string]$errLog) {
    & cmd /c "$exe $argsStr > `"$outLog`" 2> `"$errLog`""
    return $LASTEXITCODE
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

function Invoke-HeaderTrans([string]$SrcRel, [string]$OutName) {
    Push-Location $RepoRoot
    $out = Join-Path $HeaderDir $OutName
    $o1 = Join-Path $RunDir "hdr_${OutName}_stdout.log"
    $o2 = Join-Path $RunDir "hdr_${OutName}_stderr.log"
    $rc = Invoke-Cmd $L1Trans "$SrcRel `"$out`"" $o1 $o2
    Pop-Location
    if ($rc -ne 0) { Get-Content $o2; throw "$SrcRel header translation failed" }
}

# ---- Step 0: translate the real headers this module's own real
# dependency chain needs, plus the L2 header. ----
Invoke-HeaderTrans "mixa_manager\mixa_share_win32.h.lm1" "mixa_share_win32.lm1.h"
Invoke-HeaderTrans "mixa_manager\mixa_share.h.lm1" "mixa_share.lm1.h"
Invoke-HeaderTrans "mixa_manager\mixa_share_win32_l2.h.lm1" "mixa_share_win32_l2.lm1.h"

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

# ---- Step 2: build the harness object (compiled ONCE). ----
Push-Location $RepoRoot
$harnessC = Join-Path $RunDir "harness.c"
$hLog1 = Join-Path $RunDir "harness_trans_stdout.log"
$hLog2 = Join-Path $RunDir "harness_trans_stderr.log"
$hExit = Invoke-Cmd $L1Trans "mixa_manager\tests\mixa_share_win32_parity_harness.lm1 `"$harnessC`"" $hLog1 $hLog2
Pop-Location
if ($hExit -ne 0) { Get-Content $hLog2; throw "harness translation failed" }

$harnessO = Join-Path $RunDir "harness.o"
$hcLog1 = Join-Path $RunDir "harness_compile_stdout.log"
$hcLog2 = Join-Path $RunDir "harness_compile_stderr.log"
$hcExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" -idirafter `"$SdkWinrtDir`" -c `"$harnessC`" -o `"$harnessO`"" $hcLog1 $hcLog2
if ($hcExit -ne 0) { Get-Content $hcLog2; throw "harness compile failed" }

# ---- Step 3: ALWAYS build + run the ORACLE-side harness. mixa_share_
# win32.lm1 predefs ONLY headers -- no separate real dependency objects
# needed. ----
$oracleC = Join-Path $RunDir "mixa_share_win32_oracle.c"
$ocLog1 = Join-Path $RunDir "oracle_trans_stdout.log"
$ocLog2 = Join-Path $RunDir "oracle_trans_stderr.log"
Push-Location $RepoRoot
$ocExit = Invoke-Cmd $L1Trans "mixa_manager\mixa_share_win32.lm1 `"$oracleC`"" $ocLog1 $ocLog2
Pop-Location
if ($ocExit -ne 0) { Get-Content $ocLog2; throw "oracle mixa_share_win32.lm1 translation failed" }

$oracleO = Join-Path $RunDir "mixa_share_win32_oracle.o"
$occLog1 = Join-Path $RunDir "oracle_compile_stdout.log"
$occLog2 = Join-Path $RunDir "oracle_compile_stderr.log"
$occExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" -idirafter `"$SdkWinrtDir`" -c `"$oracleC`" -o `"$oracleO`"" $occLog1 $occLog2
if ($occExit -ne 0) { Get-Content $occLog2; throw "oracle mixa_share_win32.lm1 compile failed" }

$shareDepO_oracle = Build-RealDep "mixa_share_dep_oracle" "mixa_manager\mixa_share.lm1"

$oracleExe = Join-Path $RunDir "parity_oracle.exe"
$olLog1 = Join-Path $RunDir "oracle_link_stdout.log"
$olLog2 = Join-Path $RunDir "oracle_link_stderr.log"
$olExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" `"$harnessO`" `"$oracleO`" `"$shareDepO_oracle`" -o `"$oracleExe`" $GccLibs" $olLog1 $olLog2
if ($olExit -ne 0) { Get-Content $olLog2; throw "oracle harness link failed" }

$OracleFixtureDir = Join-Path $RunDir "oracle_fixtures"
New-Item -ItemType Directory -Force -Path $OracleFixtureDir | Out-Null
Push-Location $RepoRoot
$oracleRunOut = Join-Path $RunDir "oracle_run_stdout.log"
$oracleRunErr = Join-Path $RunDir "oracle_run_stderr.log"
$oracleRunExit = Invoke-Cmd "`"$oracleExe`"" "`"$OracleFixtureDir`"" $oracleRunOut $oracleRunErr
Pop-Location
$OracleTrace = Get-Content -LiteralPath $oracleRunOut -Raw

# ---- Step 4: attempt the COMPLETE mixa_share_win32.lm2 translation. ----
Push-Location $RepoRoot
$swSrc = "mixa_manager\mixa_share_win32.lm2"
$swOut = Join-Path $RunDir "mixa_share_win32_l2.lm1"
$swStdout = Join-Path $RunDir "sw_stdout.log"
$swStderr = Join-Path $RunDir "sw_stderr.log"
$SwExit = Invoke-Cmd "`"$l2exe`"" "`"$swSrc`" `"$swOut`"" $swStdout $swStderr
Pop-Location

$SwStdoutText = if (Test-Path -LiteralPath $swStdout) { Get-Content -LiteralPath $swStdout -Raw } else { "" }
$SwStderrText = if (Test-Path -LiteralPath $swStderr) { Get-Content -LiteralPath $swStderr -Raw } else { "" }
$SwSourceHash = (Get-FileHash -LiteralPath (Join-Path $RepoRoot $swSrc) -Algorithm SHA256).Hash
$RunnerHash = (Get-FileHash -LiteralPath $PSCommandPath -Algorithm SHA256).Hash

$Verdict = $null
$ExitCode = 1
$L2TraceText = ""
$DiffText = ""

$KnownBarrier = ($SwStderrText -match "unknown foreign type") -or ($SwStderrText -match "incompatible entry signature") -or ($SwStderrText -match "unsupported own array declaration")

if ($SwExit -ne 0 -and $KnownBarrier) {
    $Verdict = "EXPECTED_CORE_BARRIER"
    $ExitCode = 2
} elseif ($SwExit -ne 0) {
    $Verdict = "UNEXPECTED_FAILURE"
    $ExitCode = 1
} else {
    Push-Location $RepoRoot
    $l2SwC = Join-Path $RunDir "mixa_share_win32_l2.c"
    $l2ccLog1 = Join-Path $RunDir "l2sw_trans_stdout.log"
    $l2ccLog2 = Join-Path $RunDir "l2sw_trans_stderr.log"
    $l2ccExit = Invoke-Cmd $L1Trans "`"$swOut`" `"$l2SwC`"" $l2ccLog1 $l2ccLog2
    Pop-Location
    if ($l2ccExit -ne 0) {
        Get-Content $l2ccLog2
        $Verdict = "UNEXPECTED_FAILURE"
        $ExitCode = 1
    } else {
        $l2SwO = Join-Path $RunDir "mixa_share_win32_l2.o"
        $l2occLog1 = Join-Path $RunDir "l2sw_compile_stdout.log"
        $l2occLog2 = Join-Path $RunDir "l2sw_compile_stderr.log"
        $l2occExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" -idirafter `"$SdkWinrtDir`" -c `"$l2SwC`" -o `"$l2SwO`"" $l2occLog1 $l2occLog2
        if ($l2occExit -ne 0) {
            Get-Content $l2occLog2
            $Verdict = "UNEXPECTED_FAILURE"
            $ExitCode = 1
        } else {
            $shareDepO_l2 = Build-RealDep "mixa_share_dep_l2" "mixa_manager\mixa_share.lm1"
            $l2Exe = Join-Path $RunDir "parity_l2.exe"
            $l2olLog1 = Join-Path $RunDir "l2sw_link_stdout.log"
            $l2olLog2 = Join-Path $RunDir "l2sw_link_stderr.log"
            $l2olExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" `"$harnessO`" `"$l2SwO`" `"$shareDepO_l2`" -o `"$l2Exe`" $GccLibs" $l2olLog1 $l2olLog2
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
ShareWin32-L2-Header: mixa_manager\mixa_share_win32_l2.h.lm1
ShareWin32-L2-Source: $swSrc
ShareWin32-L2-Source-Sha256: $SwSourceHash
Harness-Source-Sha256: $((Get-FileHash -LiteralPath (Join-Path $RepoRoot "mixa_manager\tests\mixa_share_win32_parity_harness.lm1") -Algorithm SHA256).Hash)
Runner-Sha256: $RunnerHash
Oracle-Run-Exit-Code: $oracleRunExit
Oracle-Trace:
$OracleTrace
ShareWin32-Translate-Exit-Code: $SwExit
ShareWin32-Translate-Stdout:
$SwStdoutText
ShareWin32-Translate-Stderr:
$SwStderrText
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
        "EXPECTED_CORE_BARRIER: full mixa_share_win32.lm2 translation stopped at an already-known barrier. This is NOT a pass -- the integrated frontend is not yet on main. The oracle-side harness DID run (see Oracle-Trace above)."
    }
    "UNEXPECTED_FAILURE" {
        "UNEXPECTED_FAILURE: translation or build failed with something OTHER than the already-known barriers. Inspect the logs under $RunDir."
    }
    "PARITY_FAILURE" {
        "PARITY_FAILURE: both implementations built and ran, but their traces differ (see Diff above) or one exited non-zero."
    }
    "PASS" {
        "PASS: full mixa_share_win32.lm2 translated, built, and ran; its trace is byte-identical to the L1 oracle's own trace."
    }
}

exit $ExitCode
