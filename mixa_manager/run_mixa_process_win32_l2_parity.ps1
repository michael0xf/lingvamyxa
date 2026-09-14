# Real L1-vs-L2 parity gate for mixa_process_win32 (copy-chain ticket
# 20260913-185600, module 4 of 4, the last module in the standing
# list). No dedicated ABI probe pair, and NO L1 header unit to predef
# at all: mixa_process_win32.h has no .h.lm1 counterpart anywhere in
# this codebase -- real, plain, hand-written C (the same shape as
# mixa_pump.h before it). MixaProcess is fully opaque to every caller
# (never dereferenced via `\` from outside mixa_process_win32.lm1
# itself), so even the established `\`-defers-to-gcc pattern is not
# needed here -- callers only ever hold and pass the pointer.
#
#   1. ALWAYS builds and runs the ORACLE-side harness against a real
#      small fixture executable (mixa_process_fixture.c, plain C, the
#      same fixture the existing accepted mixa_process_selftest.lm1
#      already uses, built fresh here rather than reused stale):
#      argument/null/overflow guards; a real spawn failure (invalid
#      cwd) distinct from an ordinary started child, with a 20x repeat
#      proving no partial-spawn handle leak; an unknown command started
#      as a real shell child (not a spawn failure); real stdout+stderr
#      merge with arrival order preserved; a quiet-running child's
#      immediate zero-byte read then a real exit code; a stdin-
#      disabled child's real pipe EOF (not an invalid handle) and a
#      refused write; a stdin-enabled real echo round trip; the exact
#      native exit code preserved verbatim; kill genuinely terminating
#      both the direct child and a spawned descendant (a real marker
#      file proof); idempotent close; and a 10x real spawn/wait/close
#      lifecycle stress.
#   2. Attempts to translate the COMPLETE mixa_process_win32.lm2.
#      - fails with an already-known barrier -> EXPECTED_CORE_BARRIER
#        (exit 2). NOT a pass.
#      - fails with any OTHER diagnostic -> UNEXPECTED_FAILURE (exit 1).
#      - SUCCEEDS -> builds/links/runs the L2-side harness and diffs its
#        stdout against the oracle trace byte-for-byte. PASS (exit 0)
#        only on an exact match, else PARITY_FAILURE (exit 1).
#
# mixa_process_win32.h/.lm1 are the parity oracle and are never
# touched. mixa_process_fixture.c (plain C, test-owned, already
# accepted) is compiled fresh, never modified. Nothing under stg/
# l1_baseline is modified, only read. Every input is built fresh in a
# unique run directory -- no stale objects.
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
$BaseDir = Join-Path $RepoRoot "build\mixa\claude\mixa_process_win32_l2_parity"
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

# ---- Step 0: translate the L2 header (nothing else to translate --
# mixa_process_win32.h is plain C, and the L2 header predefs nothing). ----
Invoke-HeaderTrans "mixa_manager\mixa_process_win32_l2.h.lm1" "mixa_process_win32_l2.lm1.h"

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

# ---- Step 2: build the real fixture executable (plain C, no L1
# involved -- same source the existing accepted mixa_process_selftest.
# lm1 already uses). ----
$FixtureSource = Join-Path $RepoRoot "mixa_manager\tests\mixa_process_fixture.c"
if (-not (Test-Path -LiteralPath $FixtureSource)) {
    throw "missing real fixture source: $FixtureSource"
}
$FixtureExe = Join-Path $RunDir "mixa_process_fixture.exe"
$FixtureStdout = Join-Path $RunDir "fixture_compile_stdout.log"
$FixtureStderr = Join-Path $RunDir "fixture_compile_stderr.log"
$FixtureArgs = "-std=c99 -Wall -Wextra -Wpedantic -I `"$RepoRoot`" `"$FixtureSource`" -o `"$FixtureExe`""
$FixtureExit = Invoke-Cmd "gcc" $FixtureArgs $FixtureStdout $FixtureStderr
if ($FixtureExit -ne 0) { Get-Content $FixtureStderr; throw "fixture compile failed" }
$FixtureSourceHash = (Get-FileHash -LiteralPath $FixtureSource -Algorithm SHA256).Hash

# ---- Step 3: build the harness object (compiled ONCE). ----
Push-Location $RepoRoot
$harnessC = Join-Path $RunDir "harness.c"
$hLog1 = Join-Path $RunDir "harness_trans_stdout.log"
$hLog2 = Join-Path $RunDir "harness_trans_stderr.log"
$hExit = Invoke-Cmd $L1Trans "mixa_manager\tests\mixa_process_win32_parity_harness.lm1 `"$harnessC`"" $hLog1 $hLog2
Pop-Location
if ($hExit -ne 0) { Get-Content $hLog2; throw "harness translation failed" }

$harnessO = Join-Path $RunDir "harness.o"
$hcLog1 = Join-Path $RunDir "harness_compile_stdout.log"
$hcLog2 = Join-Path $RunDir "harness_compile_stderr.log"
$hcExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" -c `"$harnessC`" -o `"$harnessO`"" $hcLog1 $hcLog2
if ($hcExit -ne 0) { Get-Content $hcLog2; throw "harness compile failed" }

# ---- Step 4: ALWAYS build + run the ORACLE-side harness. ----
$oracleC = Join-Path $RunDir "mixa_process_win32_oracle.c"
$ocLog1 = Join-Path $RunDir "oracle_trans_stdout.log"
$ocLog2 = Join-Path $RunDir "oracle_trans_stderr.log"
Push-Location $RepoRoot
$ocExit = Invoke-Cmd $L1Trans "mixa_manager\mixa_process_win32.lm1 `"$oracleC`"" $ocLog1 $ocLog2
Pop-Location
if ($ocExit -ne 0) { Get-Content $ocLog2; throw "oracle mixa_process_win32.lm1 translation failed" }

$oracleO = Join-Path $RunDir "mixa_process_win32_oracle.o"
$occLog1 = Join-Path $RunDir "oracle_compile_stdout.log"
$occLog2 = Join-Path $RunDir "oracle_compile_stderr.log"
$occExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" -c `"$oracleC`" -o `"$oracleO`"" $occLog1 $occLog2
if ($occExit -ne 0) { Get-Content $occLog2; throw "oracle mixa_process_win32.lm1 compile failed" }

$oracleExe = Join-Path $RunDir "parity_oracle.exe"
$olLog1 = Join-Path $RunDir "oracle_link_stdout.log"
$olLog2 = Join-Path $RunDir "oracle_link_stderr.log"
$olExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" `"$harnessO`" `"$oracleO`" -o `"$oracleExe`"" $olLog1 $olLog2
if ($olExit -ne 0) { Get-Content $olLog2; throw "oracle harness link failed" }

$OracleFixtureDir = Join-Path $RunDir "oracle_fixtures"
New-Item -ItemType Directory -Force -Path $OracleFixtureDir | Out-Null
Push-Location $RepoRoot
$oracleRunOut = Join-Path $RunDir "oracle_run_stdout.log"
$oracleRunErr = Join-Path $RunDir "oracle_run_stderr.log"
$oracleRunExit = Invoke-Cmd "`"$oracleExe`"" "`"$OracleFixtureDir`" `"$FixtureExe`"" $oracleRunOut $oracleRunErr
Pop-Location
$OracleTrace = Get-Content -LiteralPath $oracleRunOut -Raw

# ---- Step 5: attempt the COMPLETE mixa_process_win32.lm2 translation. ----
Push-Location $RepoRoot
$pwSrc = "mixa_manager\mixa_process_win32.lm2"
$pwOut = Join-Path $RunDir "mixa_process_win32_l2.lm1"
$pwStdout = Join-Path $RunDir "pw_stdout.log"
$pwStderr = Join-Path $RunDir "pw_stderr.log"
$PwExit = Invoke-Cmd "`"$l2exe`"" "`"$pwSrc`" `"$pwOut`"" $pwStdout $pwStderr
Pop-Location

$PwStdoutText = if (Test-Path -LiteralPath $pwStdout) { Get-Content -LiteralPath $pwStdout -Raw } else { "" }
$PwStderrText = if (Test-Path -LiteralPath $pwStderr) { Get-Content -LiteralPath $pwStderr -Raw } else { "" }
$PwSourceHash = (Get-FileHash -LiteralPath (Join-Path $RepoRoot $pwSrc) -Algorithm SHA256).Hash
$RunnerHash = (Get-FileHash -LiteralPath $PSCommandPath -Algorithm SHA256).Hash

$Verdict = $null
$ExitCode = 1
$L2TraceText = ""
$DiffText = ""

$KnownBarrier = ($PwStderrText -match "unknown foreign type") -or ($PwStderrText -match "incompatible entry signature")

if ($PwExit -ne 0 -and $KnownBarrier) {
    $Verdict = "EXPECTED_CORE_BARRIER"
    $ExitCode = 2
} elseif ($PwExit -ne 0) {
    $Verdict = "UNEXPECTED_FAILURE"
    $ExitCode = 1
} else {
    Push-Location $RepoRoot
    $l2PwC = Join-Path $RunDir "mixa_process_win32_l2.c"
    $l2ccLog1 = Join-Path $RunDir "l2pw_trans_stdout.log"
    $l2ccLog2 = Join-Path $RunDir "l2pw_trans_stderr.log"
    $l2ccExit = Invoke-Cmd $L1Trans "`"$pwOut`" `"$l2PwC`"" $l2ccLog1 $l2ccLog2
    Pop-Location
    if ($l2ccExit -ne 0) {
        Get-Content $l2ccLog2
        $Verdict = "UNEXPECTED_FAILURE"
        $ExitCode = 1
    } else {
        $l2PwO = Join-Path $RunDir "mixa_process_win32_l2.o"
        $l2occLog1 = Join-Path $RunDir "l2pw_compile_stdout.log"
        $l2occLog2 = Join-Path $RunDir "l2pw_compile_stderr.log"
        $l2occExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" -c `"$l2PwC`" -o `"$l2PwO`"" $l2occLog1 $l2occLog2
        if ($l2occExit -ne 0) {
            Get-Content $l2occLog2
            $Verdict = "UNEXPECTED_FAILURE"
            $ExitCode = 1
        } else {
            $l2Exe = Join-Path $RunDir "parity_l2.exe"
            $l2olLog1 = Join-Path $RunDir "l2pw_link_stdout.log"
            $l2olLog2 = Join-Path $RunDir "l2pw_link_stderr.log"
            $l2olExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" `"$harnessO`" `"$l2PwO`" -o `"$l2Exe`"" $l2olLog1 $l2olLog2
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
                $l2RunExit = Invoke-Cmd "`"$l2Exe`"" "`"$L2FixtureDir`" `"$FixtureExe`"" $l2RunOut $l2RunErr
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
ProcessWin32-L2-Header: mixa_manager\mixa_process_win32_l2.h.lm1
ProcessWin32-L2-Source: $pwSrc
ProcessWin32-L2-Source-Sha256: $PwSourceHash
Fixture-Source-Sha256: $FixtureSourceHash
Harness-Source-Sha256: $((Get-FileHash -LiteralPath (Join-Path $RepoRoot "mixa_manager\tests\mixa_process_win32_parity_harness.lm1") -Algorithm SHA256).Hash)
Runner-Sha256: $RunnerHash
Oracle-Run-Exit-Code: $oracleRunExit
Oracle-Trace:
$OracleTrace
ProcessWin32-Translate-Exit-Code: $PwExit
ProcessWin32-Translate-Stdout:
$PwStdoutText
ProcessWin32-Translate-Stderr:
$PwStderrText
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
        "EXPECTED_CORE_BARRIER: full mixa_process_win32.lm2 translation stopped at an already-known barrier. This is NOT a pass -- the integrated frontend is not yet on main. The oracle-side harness DID run (see Oracle-Trace above)."
    }
    "UNEXPECTED_FAILURE" {
        "UNEXPECTED_FAILURE: translation or build failed with something OTHER than the already-known barriers. Inspect the logs under $RunDir."
    }
    "PARITY_FAILURE" {
        "PARITY_FAILURE: both implementations built and ran, but their traces differ (see Diff above) or one exited non-zero."
    }
    "PASS" {
        "PASS: full mixa_process_win32.lm2 translated, built, and ran; its trace is byte-identical to the L1 oracle's own trace."
    }
}

exit $ExitCode
