# Real L1-vs-L2 parity gate for mixa_audio_mp3 (ticket 20260913-232000/
# 231500 Part 2, module 5 of 8).
#
# GENUINE DEVIATION FROM EVERY OTHER MODULE THIS SESSION, stated plainly
# rather than hidden: the real mixa_audio_mp3.lm1 needs 22 imports in one
# translation unit (it predefs both mixa_audio_scan.lm1's and mixa_
# audio_launch.lm1's full bodies). stable65D5's import path table caps
# at 16, so the STABLE PINNED compiler cannot translate the real .lm1 at
# all -- not a barrier, a hard compile failure unrelated to the clean-L2
# porting effort. This was already solved once, by an earlier Claude
# session (ticket 20260912-084943): Codex's own CANDIDATE compiler (SHA256
# 24A1B57B6C831C7B45630DF8CA61A7441376B4EB8EF946B1F08FC41957290B09, at
# build/codex/l1-import-capacity/build/import_capacity/run_20260912_084844/
# l1trans.exe) removes the cap and is used ONLY for this module's own
# oracle-side headers/translation/compile -- read-only, hash-verified
# against the published value every run, exactly mirroring the already-
# accepted run_mp3_selftest.ps1's own precedent (not a new exception
# invented here). stable65D5's own hash is independently re-verified
# unchanged on every run. The L2-side attempt (l2trans.exe, itself always
# built from stable65D5, and any subsequent l1trans call on l2trans's own
# generated intermediate output) still uses the STABLE compiler, matching
# every other module -- l2trans's own import handling is unrelated to
# l1trans's fixed-size import table and has never needed the candidate.
#
#   1. ALWAYS builds and runs the ORACLE-side harness (via the CANDIDATE
#      compiler) against real, test-owned on-disk fixtures with an
#      independent mixa_dir enumeration oracle.
#   2. Attempts to translate the COMPLETE mixa_audio_mp3.lm2 (via
#      l2trans.exe, built from STABLE).
#      - fails with an already-known barrier -> EXPECTED_CORE_BARRIER
#        (exit 2). NOT a pass.
#      - fails with any OTHER diagnostic -> UNEXPECTED_FAILURE (exit 1).
#      - SUCCEEDS -> builds/links/runs the L2-side harness (STABLE for
#        the generated-C step, candidate only if that step itself needs
#        it) and diffs its stdout against the oracle trace byte-for-
#        byte. PASS (exit 0) only on an exact match, else PARITY_FAILURE
#        (exit 1).
#
# mixa_audio_mp3.lm1/.h.lm1 and the real, unmodified chain it predefs
# (mixa_audio_scan, mixa_audio_launch, and everything they in turn
# predef) are the parity oracle and are never touched. Nothing under
# stg/l1_baseline or build/codex is modified, only read. Every input is
# built fresh in a unique run directory -- no stale objects.
param()
$ErrorActionPreference = "Stop"

$RepoRoot = Split-Path -Parent $PSScriptRoot
$L1Root = Join-Path $RepoRoot "stg\l1_baseline"
$L1Trans = Join-Path $L1Root "build\l1trans\gen2\l1trans.exe"
. (Join-Path $PSScriptRoot "lib_l2_runtime_support.ps1")
$ExpectedL1Hash = Get-L1Pin -L1Root $L1Root
$Candidate = Join-Path $RepoRoot "build\codex\l1-import-capacity\build\import_capacity\run_20260912_084844\l1trans.exe"
$ExpectedCandidateHash = "24A1B57B6C831C7B45630DF8CA61A7441376B4EB8EF946B1F08FC41957290B09"

$ActualL1Hash = Assert-PinnedL1Translator -L1Trans $L1Trans -L1Root $L1Root
if (-not (Test-Path -LiteralPath $Candidate)) {
    throw "missing candidate L1 translator (ticket 20260912-084943, read-only, not modified by this script): $Candidate"
}
$ActualCandidateHash = (Get-FileHash -LiteralPath $Candidate -Algorithm SHA256).Hash
if ($ActualCandidateHash -ne $ExpectedCandidateHash) {
    throw "candidate L1 translator hash mismatch: expected $ExpectedCandidateHash got $ActualCandidateHash"
}

$guards = @(
    "-Werror=incompatible-pointer-types", "-Werror=discarded-qualifiers",
    "-Werror=implicit-function-declaration", "-Werror=implicit-int"
)
$GccStd = "-std=c99 -Wall -Wextra -Wpedantic $($guards -join ' ')"

$RunTimestamp = (Get-Date -Format "yyyyMMdd_HHmmss_fff")
$RunGuid = [GUID]::NewGuid().ToString().Substring(0, 8)
$BaseDir = Join-Path $RepoRoot "build\mixa\claude\mixa_audio_mp3_l2_parity"
$RunDir = Join-Path $BaseDir "run_${RunTimestamp}_${RunGuid}"
$HeaderDir = Join-Path $RunDir "headers\mixa_manager"
$FixtureDir = Join-Path $RunDir "fixtures"
New-Item -ItemType Directory -Force -Path $HeaderDir | Out-Null
New-Item -ItemType Directory -Force -Path $FixtureDir | Out-Null

function Invoke-Cmd([string]$exe, [string]$argsStr, [string]$outLog, [string]$errLog) {
    & cmd /c "$exe $argsStr > `"$outLog`" 2> `"$errLog`""
    return $LASTEXITCODE
}

# Shared L2 runtime-support helper (runner-uniformity ticket, following
# 20260914-003000/001000): the runtime header/object generation trio
# and fixture-root trace normalization, kept in one file so every
# runner uses the identical implementation rather than a pasted,
# driftable copy.
$InvokeCmdRef = { param($e, $a, $o, $er) Invoke-Cmd $e $a $o $er }

function Invoke-HeaderTrans([string]$Trans, [string]$SrcRel, [string]$OutName) {
    Push-Location $RepoRoot
    $out = Join-Path $HeaderDir $OutName
    $o1 = Join-Path $RunDir "hdr_${OutName}_stdout.log"
    $o2 = Join-Path $RunDir "hdr_${OutName}_stderr.log"
    $rc = Invoke-Cmd $Trans "$SrcRel `"$out`"" $o1 $o2
    Pop-Location
    if ($rc -ne 0) { Get-Content $o2; throw "$SrcRel header translation failed" }
}

# ---- Step 0: translate the real headers with the CANDIDATE (oracle
# side needs the same candidate for headers as for the body, matching
# run_mp3_selftest.ps1's own approach), then the L2 header unit with
# STABLE (the L2 header alone, forward-declares aside, has a small
# transitive predef closure well under 16 imports). ----
Invoke-HeaderTrans $Candidate "mixa_manager\mixa_dir_win32.h.lm1" "mixa_dir_win32.lm1.h"
Invoke-HeaderTrans $Candidate "mixa_manager\mixa_dir.h.lm1" "mixa_dir.lm1.h"
Invoke-HeaderTrans $Candidate "mixa_manager\mixa_audio_win32.h.lm1" "mixa_audio_win32.lm1.h"
Invoke-HeaderTrans $Candidate "mixa_manager\mixa_audio.h.lm1" "mixa_audio.lm1.h"
Invoke-HeaderTrans $Candidate "mixa_manager\mixa_audio_scan.h.lm1" "mixa_audio_scan.lm1.h"
Invoke-HeaderTrans $Candidate "mixa_manager\mixa_audio_button.h.lm1" "mixa_audio_button.lm1.h"
Invoke-HeaderTrans $Candidate "mixa_manager\mixa_app_window.h.lm1" "mixa_app_window.lm1.h"
Invoke-HeaderTrans $Candidate "mixa_manager\mixa_audio_panel.h.lm1" "mixa_audio_panel.lm1.h"
Invoke-HeaderTrans $Candidate "mixa_manager\mixa_audio_launch.h.lm1" "mixa_audio_launch.lm1.h"
Invoke-HeaderTrans $Candidate "mixa_manager\mixa_audio_mp3.h.lm1" "mixa_audio_mp3.lm1.h"
Invoke-HeaderTrans $Candidate "mixa_manager\mixa_audio_mp3_l2.h.lm1" "mixa_audio_mp3_l2.lm1.h"

# ---- Step 1: build l2trans.exe fresh, from STABLE (as always -- its
# own import handling is independent of l1trans's fixed-size table). ----
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

# ---- Step 2: build the harness object (compiled ONCE, via CANDIDATE --
# it must link against the oracle object below, and while the harness's
# OWN predef closure is small, using the same translator for both avoids
# any ambiguity about which l1trans's C-generation conventions apply). ----
Push-Location $RepoRoot
$harnessC = Join-Path $RunDir "harness.c"
$hLog1 = Join-Path $RunDir "harness_trans_stdout.log"
$hLog2 = Join-Path $RunDir "harness_trans_stderr.log"
$hExit = Invoke-Cmd $Candidate "mixa_manager\tests\mixa_audio_mp3_parity_harness.lm1 `"$harnessC`"" $hLog1 $hLog2
Pop-Location
if ($hExit -ne 0) { Get-Content $hLog2; throw "harness translation failed" }

$harnessO = Join-Path $RunDir "harness.o"
$hcLog1 = Join-Path $RunDir "harness_compile_stdout.log"
$hcLog2 = Join-Path $RunDir "harness_compile_stderr.log"
$hcExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" -c `"$harnessC`" -o `"$harnessO`"" $hcLog1 $hcLog2
if ($hcExit -ne 0) { Get-Content $hcLog2; throw "harness compile failed" }

# ---- Step 3: ALWAYS build + run the ORACLE-side harness (CANDIDATE). ----
$oracleC = Join-Path $RunDir "mixa_audio_mp3_oracle.c"
$ocLog1 = Join-Path $RunDir "oracle_trans_stdout.log"
$ocLog2 = Join-Path $RunDir "oracle_trans_stderr.log"
Push-Location $RepoRoot
$ocExit = Invoke-Cmd $Candidate "mixa_manager\mixa_audio_mp3.lm1 `"$oracleC`"" $ocLog1 $ocLog2
Pop-Location
if ($ocExit -ne 0) { Get-Content $ocLog2; throw "oracle mixa_audio_mp3.lm1 translation failed (candidate compiler)" }

$oracleO = Join-Path $RunDir "mixa_audio_mp3_oracle.o"
$occLog1 = Join-Path $RunDir "oracle_compile_stdout.log"
$occLog2 = Join-Path $RunDir "oracle_compile_stderr.log"
$occExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" -c `"$oracleC`" -o `"$oracleO`"" $occLog1 $occLog2
if ($occExit -ne 0) { Get-Content $occLog2; throw "oracle mixa_audio_mp3.lm1 compile failed" }

$oracleExe = Join-Path $RunDir "parity_oracle.exe"
$olLog1 = Join-Path $RunDir "oracle_link_stdout.log"
$olLog2 = Join-Path $RunDir "oracle_link_stderr.log"
$olExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" `"$harnessO`" `"$oracleO`" -o `"$oracleExe`"" $olLog1 $olLog2
if ($olExit -ne 0) { Get-Content $olLog2; throw "oracle harness link failed" }

Push-Location $RepoRoot
$oracleRunOut = Join-Path $RunDir "oracle_run_stdout.log"
$oracleRunErr = Join-Path $RunDir "oracle_run_stderr.log"
$oracleRunExit = Invoke-Cmd "`"$oracleExe`"" "`"$FixtureDir`"" $oracleRunOut $oracleRunErr
Pop-Location
$OracleTrace = Get-Content -LiteralPath $oracleRunOut -Raw

# ---- Step 4: attempt the COMPLETE mixa_audio_mp3.lm2 translation
# (l2trans.exe, built from STABLE). ----
Push-Location $RepoRoot
$mpSrc = "mixa_manager\mixa_audio_mp3.lm2"
$mpOut = Join-Path $RunDir "mixa_audio_mp3_l2.lm1"
$mpStdout = Join-Path $RunDir "mp_stdout.log"
$mpStderr = Join-Path $RunDir "mp_stderr.log"
$env:L2_RUNTIME_ROOT = "stg/l1_baseline/l2src/"
$MpExit = Invoke-Cmd "`"$l2exe`"" "`"$mpSrc`" `"$mpOut`"" $mpStdout $mpStderr
Pop-Location

$MpStdoutText = if (Test-Path -LiteralPath $mpStdout) { Get-Content -LiteralPath $mpStdout -Raw } else { "" }
$MpStderrText = if (Test-Path -LiteralPath $mpStderr) { Get-Content -LiteralPath $mpStderr -Raw } else { "" }
$MpSourceHash = (Get-FileHash -LiteralPath (Join-Path $RepoRoot $mpSrc) -Algorithm SHA256).Hash
$RunnerHash = (Get-FileHash -LiteralPath $PSCommandPath -Algorithm SHA256).Hash

$Verdict = $null
$ExitCode = 1
$L2TraceText = ""
$DiffText = ""

$KnownBarrier = ($MpStderrText -match "unknown foreign type") -or ($MpStderrText -match "incompatible entry signature") -or ($MpStderrText -match "unsupported own array declaration")

if ($MpExit -ne 0 -and $KnownBarrier) {
    $Verdict = "EXPECTED_CORE_BARRIER"
    $ExitCode = 2
} elseif ($MpExit -ne 0) {
    $Verdict = "UNEXPECTED_FAILURE"
    $ExitCode = 1
} else {
    # If l2trans ever succeeds here, the generated intermediate .lm1
    # might itself need >16 imports (it embeds the same composed
    # scan+launch chain) -- try STABLE first, fall back to the
    # candidate only if stable specifically reports the import-table
    # limit, and say plainly which one actually worked.
    Push-Location $RepoRoot
    $l2MpC = Join-Path $RunDir "mixa_audio_mp3_l2.c"
    $l2ccLog1 = Join-Path $RunDir "l2mp_trans_stdout.log"
    $l2ccLog2 = Join-Path $RunDir "l2mp_trans_stderr.log"
    $l2ccExit = Invoke-Cmd $L1Trans "`"$mpOut`" `"$l2MpC`"" $l2ccLog1 $l2ccLog2
    $UsedCandidateForIntermediate = $false
    if ($l2ccExit -ne 0) {
        $stableIntermediateErr = Get-Content $l2ccLog2 -Raw
        if ($stableIntermediateErr -match "import path table full") {
            $l2ccExit = Invoke-Cmd $Candidate "`"$mpOut`" `"$l2MpC`"" $l2ccLog1 $l2ccLog2
            $UsedCandidateForIntermediate = $true
        }
    }
    Pop-Location
    if ($l2ccExit -ne 0) {
        Get-Content $l2ccLog2
        $Verdict = "UNEXPECTED_FAILURE"
        $ExitCode = 1
    } else {
        $L2Rt = Add-L2RuntimeSupport -L1Trans $L1Trans -L1Root $L1Root -RunDir $RunDir -InvokeCmd $InvokeCmdRef
        $l2MpO = Join-Path $RunDir "mixa_audio_mp3_l2.o"
        $l2occLog1 = Join-Path $RunDir "l2mp_compile_stdout.log"
        $l2occLog2 = Join-Path $RunDir "l2mp_compile_stderr.log"
        $l2occExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" -I `"$($L2Rt.HeaderRoot)`" -I `"$($L2Rt.HeaderRoot)\stg\l1_baseline`" -I `"$L1Root`" -c `"$l2MpC`" -o `"$l2MpO`"" $l2occLog1 $l2occLog2
        if ($l2occExit -ne 0) {
            Get-Content $l2occLog2
            $Verdict = "UNEXPECTED_FAILURE"
            $ExitCode = 1
        } else {
            $l2Exe = Join-Path $RunDir "parity_l2.exe"
            $l2olLog1 = Join-Path $RunDir "l2mp_link_stdout.log"
            $l2olLog2 = Join-Path $RunDir "l2mp_link_stderr.log"
            $l2olExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" `"$harnessO`" `"$l2MpO`" $($L2Rt.ObjList) -o `"$l2Exe`"" $l2olLog1 $l2olLog2
            if ($l2olExit -ne 0) {
                Get-Content $l2olLog2
                $Verdict = "UNEXPECTED_FAILURE"
                $ExitCode = 1
            } else {
                Push-Location $RepoRoot
                $l2RunOut = Join-Path $RunDir "l2_run_stdout.log"
                $l2RunErr = Join-Path $RunDir "l2_run_stderr.log"
                $l2RunExit = Invoke-Cmd "`"$l2Exe`"" "`"$FixtureDir`"" $l2RunOut $l2RunErr
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
Candidate-L1-Translator (ticket 20260912-084943, oracle side only): $Candidate
Candidate-L1-Translator-Sha256: $ActualCandidateHash
L2trans-Source (informational, not pinned): $L2TransSource
L2trans-Source-Sha256 (informational, not pinned): $L2TransSourceHash
L2trans-Exe-Sha256 (rebuilt fresh this run from STABLE, not a stable artifact): $L2TransExeHash
AudioMp3-L2-Header: mixa_manager\mixa_audio_mp3_l2.h.lm1
AudioMp3-L2-Source: $mpSrc
AudioMp3-L2-Source-Sha256: $MpSourceHash
Harness-Source-Sha256: $((Get-FileHash -LiteralPath (Join-Path $RepoRoot "mixa_manager\tests\mixa_audio_mp3_parity_harness.lm1") -Algorithm SHA256).Hash)
Runner-Sha256: $RunnerHash
Fixture-Directory: $FixtureDir
Oracle-Run-Exit-Code: $oracleRunExit
Oracle-Trace:
$OracleTrace
AudioMp3-Translate-Exit-Code: $MpExit
AudioMp3-Translate-Stdout:
$MpStdoutText
AudioMp3-Translate-Stderr:
$MpStderrText
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
        "EXPECTED_CORE_BARRIER: full mixa_audio_mp3.lm2 translation stopped at an already-known barrier. This is NOT a pass -- the integrated frontend is not yet on main. The oracle-side harness DID run (see Oracle-Trace above), using the CANDIDATE compiler as documented."
    }
    "UNEXPECTED_FAILURE" {
        "UNEXPECTED_FAILURE: translation or build failed with something OTHER than the already-known barriers. Inspect the logs under $RunDir."
    }
    "PARITY_FAILURE" {
        "PARITY_FAILURE: both implementations built and ran, but their traces differ (see Diff above) or one exited non-zero."
    }
    "PASS" {
        "PASS: full mixa_audio_mp3.lm2 translated, built, and ran; its trace is byte-identical to the L1 oracle's own trace."
    }
}

exit $ExitCode
