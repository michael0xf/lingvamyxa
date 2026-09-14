# Real L1-vs-L2 parity gate for mixa_share_action (share chain ticket
# 20260913-192700, module 2 of 6). No dedicated ABI probe pair, and no
# L1 header unit of its own to predef: mixa_share_action.lm1 has no
# .h.lm1 counterpart anywhere in this codebase -- its single public
# function (mixa_share_action_send) is declared nowhere except by
# direct forward-declare in whatever calls it. The L2 header predefs
# the REAL mixa_share.h.lm1 directly (for MixaShareVTable/
# MixaShareRequest) and reaches MixaSelection via a plain `include:`
# of mixa_selection.h, proven safe by the established "`\` defers to
# gcc" finding from ticket 20260913-151000 (mixa_app_controller).
#
# Dependency-linking note: mixa_share_action.lm1 predefs the FULL
# bodies of mixa_selection.lm1 and mixa_share.lm1 directly in ONE
# translation unit (neither of those further predefs the other's body,
# so no dedup risk there) -- so the ORACLE side needs no separate real
# objects. The L2 header instead predefs only mixa_share.h.lm1 (header-
# only) and plain-includes mixa_selection.h -- so the L2 side
# separately links the real, unmodified mixa_share.lm1 and mixa_
# selection.lm1 (no duplication risk between them either).
#
#   1. ALWAYS builds and runs the ORACLE-side harness (a real fixture
#      tree: 2 real files plus 1 individually-selected "directory"
#      entry popped/deselected in original relative order despite a
#      backward internal walk, the directory entry staying selected
#      and counted as skipped, exactly one real share request begun
#      over the two files; nothing selected proceeding as a real text-
#      only share; and a selected file that has since disappeared
#      aborting the whole call with the real probe status, leaving the
#      selection fully untouched).
#   2. Attempts to translate the COMPLETE mixa_share_action.lm2.
#      - fails with an already-known barrier -> EXPECTED_CORE_BARRIER
#        (exit 2). NOT a pass.
#      - fails with any OTHER diagnostic -> UNEXPECTED_FAILURE (exit 1).
#      - SUCCEEDS -> builds/links/runs the L2-side harness and diffs its
#        stdout against the oracle trace byte-for-byte. PASS (exit 0)
#        only on an exact match, else PARITY_FAILURE (exit 1).
#
# mixa_share_action.lm1 and every real dependency (mixa_share.h.lm1/
# .lm1, mixa_share_win32.h.lm1, mixa_selection.h/.lm1) are the parity
# oracle (or real, unmodified dependencies) and are never touched.
# Nothing under stg/l1_baseline is modified, only read. Every input is
# built fresh in a unique run directory -- no stale objects.
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
$BaseDir = Join-Path $RepoRoot "build\mixa\claude\mixa_share_action_l2_parity"
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

# ---- Step 0: translate the real headers this module's own real
# dependency chain needs, plus the L2 header. ----
Invoke-HeaderTrans "mixa_manager\mixa_share_win32.h.lm1" "mixa_share_win32.lm1.h"
Invoke-HeaderTrans "mixa_manager\mixa_share.h.lm1" "mixa_share.lm1.h"
Invoke-HeaderTrans "mixa_manager\mixa_share_action_l2.h.lm1" "mixa_share_action_l2.lm1.h"

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
$hExit = Invoke-Cmd $L1Trans "mixa_manager\tests\mixa_share_action_parity_harness.lm1 `"$harnessC`"" $hLog1 $hLog2
Pop-Location
if ($hExit -ne 0) { Get-Content $hLog2; throw "harness translation failed" }

$harnessO = Join-Path $RunDir "harness.o"
$hcLog1 = Join-Path $RunDir "harness_compile_stdout.log"
$hcLog2 = Join-Path $RunDir "harness_compile_stderr.log"
$hcExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" -c `"$harnessC`" -o `"$harnessO`"" $hcLog1 $hcLog2
if ($hcExit -ne 0) { Get-Content $hcLog2; throw "harness compile failed" }

# ---- Step 3: ALWAYS build + run the ORACLE-side harness. mixa_share_
# action.lm1's own translation unit already embeds mixa_selection.lm1/
# mixa_share.lm1 (see top comment) -- no separate real objects needed. ----
$oracleC = Join-Path $RunDir "mixa_share_action_oracle.c"
$ocLog1 = Join-Path $RunDir "oracle_trans_stdout.log"
$ocLog2 = Join-Path $RunDir "oracle_trans_stderr.log"
Push-Location $RepoRoot
$ocExit = Invoke-Cmd $L1Trans "mixa_manager\mixa_share_action.lm1 `"$oracleC`"" $ocLog1 $ocLog2
Pop-Location
if ($ocExit -ne 0) { Get-Content $ocLog2; throw "oracle mixa_share_action.lm1 translation failed" }

$oracleO = Join-Path $RunDir "mixa_share_action_oracle.o"
$occLog1 = Join-Path $RunDir "oracle_compile_stdout.log"
$occLog2 = Join-Path $RunDir "oracle_compile_stderr.log"
$occExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" -c `"$oracleC`" -o `"$oracleO`"" $occLog1 $occLog2
if ($occExit -ne 0) { Get-Content $occLog2; throw "oracle mixa_share_action.lm1 compile failed" }

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
$oracleRunExit = Invoke-Cmd "`"$oracleExe`"" "`"$OracleFixtureDir`"" $oracleRunOut $oracleRunErr
Pop-Location
$OracleTrace = Get-Content -LiteralPath $oracleRunOut -Raw

# ---- Step 4: attempt the COMPLETE mixa_share_action.lm2 translation. ----
Push-Location $RepoRoot
$saSrc = "mixa_manager\mixa_share_action.lm2"
$saOut = Join-Path $RunDir "mixa_share_action_l2.lm1"
$saStdout = Join-Path $RunDir "sa_stdout.log"
$saStderr = Join-Path $RunDir "sa_stderr.log"
$SaExit = Invoke-Cmd "`"$l2exe`"" "`"$saSrc`" `"$saOut`"" $saStdout $saStderr
Pop-Location

$SaStdoutText = if (Test-Path -LiteralPath $saStdout) { Get-Content -LiteralPath $saStdout -Raw } else { "" }
$SaStderrText = if (Test-Path -LiteralPath $saStderr) { Get-Content -LiteralPath $saStderr -Raw } else { "" }
$SaSourceHash = (Get-FileHash -LiteralPath (Join-Path $RepoRoot $saSrc) -Algorithm SHA256).Hash
$RunnerHash = (Get-FileHash -LiteralPath $PSCommandPath -Algorithm SHA256).Hash

$Verdict = $null
$ExitCode = 1
$L2TraceText = ""
$DiffText = ""

$KnownBarrier = ($SaStderrText -match "unknown foreign type") -or ($SaStderrText -match "incompatible entry signature")

if ($SaExit -ne 0 -and $KnownBarrier) {
    $Verdict = "EXPECTED_CORE_BARRIER"
    $ExitCode = 2
} elseif ($SaExit -ne 0) {
    $Verdict = "UNEXPECTED_FAILURE"
    $ExitCode = 1
} else {
    Push-Location $RepoRoot
    $l2SaC = Join-Path $RunDir "mixa_share_action_l2.c"
    $l2ccLog1 = Join-Path $RunDir "l2sa_trans_stdout.log"
    $l2ccLog2 = Join-Path $RunDir "l2sa_trans_stderr.log"
    $l2ccExit = Invoke-Cmd $L1Trans "`"$saOut`" `"$l2SaC`"" $l2ccLog1 $l2ccLog2
    Pop-Location
    if ($l2ccExit -ne 0) {
        Get-Content $l2ccLog2
        $Verdict = "UNEXPECTED_FAILURE"
        $ExitCode = 1
    } else {
        $l2SaO = Join-Path $RunDir "mixa_share_action_l2.o"
        $l2occLog1 = Join-Path $RunDir "l2sa_compile_stdout.log"
        $l2occLog2 = Join-Path $RunDir "l2sa_compile_stderr.log"
        $l2occExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" -c `"$l2SaC`" -o `"$l2SaO`"" $l2occLog1 $l2occLog2
        if ($l2occExit -ne 0) {
            Get-Content $l2occLog2
            $Verdict = "UNEXPECTED_FAILURE"
            $ExitCode = 1
        } else {
            $shareO = Build-RealDep "mixa_share" "mixa_manager\mixa_share.lm1"
            $selectionO = Build-RealDep "mixa_selection" "mixa_manager\mixa_selection.lm1"
            $l2Exe = Join-Path $RunDir "parity_l2.exe"
            $l2olLog1 = Join-Path $RunDir "l2sa_link_stdout.log"
            $l2olLog2 = Join-Path $RunDir "l2sa_link_stderr.log"
            $l2olExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" `"$harnessO`" `"$l2SaO`" `"$shareO`" `"$selectionO`" -o `"$l2Exe`"" $l2olLog1 $l2olLog2
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
ShareAction-L2-Header: mixa_manager\mixa_share_action_l2.h.lm1
ShareAction-L2-Source: $saSrc
ShareAction-L2-Source-Sha256: $SaSourceHash
Harness-Source-Sha256: $((Get-FileHash -LiteralPath (Join-Path $RepoRoot "mixa_manager\tests\mixa_share_action_parity_harness.lm1") -Algorithm SHA256).Hash)
Runner-Sha256: $RunnerHash
Oracle-Run-Exit-Code: $oracleRunExit
Oracle-Trace:
$OracleTrace
ShareAction-Translate-Exit-Code: $SaExit
ShareAction-Translate-Stdout:
$SaStdoutText
ShareAction-Translate-Stderr:
$SaStderrText
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
        "EXPECTED_CORE_BARRIER: full mixa_share_action.lm2 translation stopped at an already-known barrier. This is NOT a pass -- the integrated frontend is not yet on main. The oracle-side harness DID run (see Oracle-Trace above)."
    }
    "UNEXPECTED_FAILURE" {
        "UNEXPECTED_FAILURE: translation or build failed with something OTHER than the already-known barriers. Inspect the logs under $RunDir."
    }
    "PARITY_FAILURE" {
        "PARITY_FAILURE: both implementations built and ran, but their traces differ (see Diff above) or one exited non-zero."
    }
    "PASS" {
        "PASS: full mixa_share_action.lm2 translated, built, and ran; its trace is byte-identical to the L1 oracle's own trace."
    }
}

exit $ExitCode
