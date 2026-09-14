# Real L1-vs-L2 parity gate for mixa_audio_win32 (ticket 20260913-
# 232000/231500 Part 2, module 8 of 8 -- the final audio-chain module):
# the real Windows MCI/winmm backend. Reuses the existing accepted
# mixa_audio_native_selftest.lm1's own real-device pattern: a fixture
# WAV (8000 Hz, 8-bit mono, ~8s of digital silence -- never audible,
# already an accepted, automated real-device test in this codebase),
# with a path deliberately containing a space to exercise MCI command
# quoting. Each side gets its own independently built WAV fixture
# (ticket 20260913-235500's own lesson: never share one fixture root
# between the oracle and L2 runs), and the trace comparison normalizes
# each side's own fixture root to a fixed token before comparing
# (ticket 20260914-001000).
#
#   1. ALWAYS builds and runs the ORACLE-side harness against its own
#      real WAV fixture.
#   2. Attempts to translate the COMPLETE mixa_audio_win32.lm2.
#      - fails with an already-known barrier -> EXPECTED_CORE_BARRIER
#        (exit 2). NOT a pass.
#      - fails with any OTHER diagnostic -> UNEXPECTED_FAILURE (exit 1).
#      - SUCCEEDS -> builds/links/runs the L2-side harness against its
#        own independently built WAV fixture and diffs its NORMALIZED
#        stdout against the oracle's NORMALIZED trace. PASS (exit 0)
#        only on an exact match, else PARITY_FAILURE (exit 1).
#
# mixa_audio_win32.lm1/.h.lm1 and the real, unmodified mixa_audio.h.lm1
# are the parity oracle and are never touched. Nothing under stg/
# l1_baseline is modified, only read. Every input is built fresh in a
# unique run directory -- no stale objects.
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
$GccLibs = "-lwinmm"

$RunTimestamp = (Get-Date -Format "yyyyMMdd_HHmmss_fff")
$RunGuid = [GUID]::NewGuid().ToString().Substring(0, 8)
# Short names from here down are deliberate, not a style choice: real
# MCI ("open ... type waveaudio ...") has a genuine path-length limit
# for the quoted device path, confirmed empirically -- a 123-character
# full WAV path (the existing accepted run_audio_native_selftest.ps1's
# own naming) opens fine; the same WAV, same exe, at 137 characters
# (this module's original naming: mixa_audio_win32_l2_parity\run_<ts>_
# <guid>\oracle_fixtures\) fails MCI's own "open" command with
# MIXA_AUDIO_ERR_BACKEND, and a 125-character path succeeds. Every
# other module's own naming never reaches this because none of them do
# a REAL MCI open of a REAL device path.
$BaseDir = Join-Path $RepoRoot "build\mixa\claude\aw32l2p"
$RunDir = Join-Path $BaseDir "r${RunTimestamp}_${RunGuid}"
$HeaderDir = Join-Path $RunDir "headers\mixa_manager"
New-Item -ItemType Directory -Force -Path $HeaderDir | Out-Null

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

function Invoke-HeaderTrans([string]$SrcRel, [string]$OutName) {
    Push-Location $RepoRoot
    $out = Join-Path $HeaderDir $OutName
    $o1 = Join-Path $RunDir "hdr_${OutName}_stdout.log"
    $o2 = Join-Path $RunDir "hdr_${OutName}_stderr.log"
    $rc = Invoke-Cmd $L1Trans "$SrcRel `"$out`"" $o1 $o2
    Pop-Location
    if ($rc -ne 0) { Get-Content $o2; throw "$SrcRel header translation failed" }
}

function New-AudioWavFixture([string]$Dir) {
    # Same recipe as run_audio_native_selftest.ps1: 8000 Hz, 8-bit PCM
    # mono, ~8s of near-silence (constant 0x7F). Path deliberately
    # contains a space.
    $FixtureWav = Join-Path $Dir "My Sound 8s.wav"
    $SampleRate = 8000
    $Seconds = 8
    $DataLen = $SampleRate * $Seconds
    $Enc = [System.Text.Encoding]::ASCII
    $Ms = New-Object System.IO.MemoryStream
    $W = New-Object System.IO.BinaryWriter($Ms)
    $W.Write($Enc.GetBytes("RIFF"))
    $W.Write([int](36 + $DataLen))
    $W.Write($Enc.GetBytes("WAVE"))
    $W.Write($Enc.GetBytes("fmt "))
    $W.Write([int]16)
    $W.Write([int16]1)
    $W.Write([int16]1)
    $W.Write([int]$SampleRate)
    $W.Write([int]$SampleRate)
    $W.Write([int16]1)
    $W.Write([int16]8)
    $W.Write($Enc.GetBytes("data"))
    $W.Write([int]$DataLen)
    $Data = New-Object byte[] $DataLen
    for ($i = 0; $i -lt $DataLen; $i++) { $Data[$i] = [byte]0x7F }
    $W.Write($Data)
    $W.Flush()
    [System.IO.File]::WriteAllBytes($FixtureWav, $Ms.ToArray())
    if (-not (Test-Path -LiteralPath $FixtureWav)) {
        throw "Fixture WAV not created: $FixtureWav"
    }
}

# ---- Step 0: translate the real headers, then the L2 header unit. ----
Invoke-HeaderTrans "mixa_manager\mixa_audio_win32.h.lm1" "mixa_audio_win32.lm1.h"
Invoke-HeaderTrans "mixa_manager\mixa_audio.h.lm1" "mixa_audio.lm1.h"
Invoke-HeaderTrans "mixa_manager\mixa_audio_win32_l2.h.lm1" "mixa_audio_win32_l2.lm1.h"

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
$hExit = Invoke-Cmd $L1Trans "mixa_manager\tests\mixa_audio_win32_parity_harness.lm1 `"$harnessC`"" $hLog1 $hLog2
Pop-Location
if ($hExit -ne 0) { Get-Content $hLog2; throw "harness translation failed" }

$harnessO = Join-Path $RunDir "harness.o"
$hcLog1 = Join-Path $RunDir "harness_compile_stdout.log"
$hcLog2 = Join-Path $RunDir "harness_compile_stderr.log"
$hcExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" -c `"$harnessC`" -o `"$harnessO`"" $hcLog1 $hcLog2
if ($hcExit -ne 0) { Get-Content $hcLog2; throw "harness compile failed" }

# ---- Step 3: ALWAYS build + run the ORACLE-side harness, against its
# own independently built WAV fixture. ----
$OracleFixtureDir = Join-Path $RunDir "of"
New-Item -ItemType Directory -Force -Path $OracleFixtureDir | Out-Null
New-AudioWavFixture -Dir $OracleFixtureDir

$oracleC = Join-Path $RunDir "mixa_audio_win32_oracle.c"
$ocLog1 = Join-Path $RunDir "oracle_trans_stdout.log"
$ocLog2 = Join-Path $RunDir "oracle_trans_stderr.log"
Push-Location $RepoRoot
$ocExit = Invoke-Cmd $L1Trans "mixa_manager\mixa_audio_win32.lm1 `"$oracleC`"" $ocLog1 $ocLog2
Pop-Location
if ($ocExit -ne 0) { Get-Content $ocLog2; throw "oracle mixa_audio_win32.lm1 translation failed" }

$oracleO = Join-Path $RunDir "mixa_audio_win32_oracle.o"
$occLog1 = Join-Path $RunDir "oracle_compile_stdout.log"
$occLog2 = Join-Path $RunDir "oracle_compile_stderr.log"
$occExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" -c `"$oracleC`" -o `"$oracleO`"" $occLog1 $occLog2
if ($occExit -ne 0) { Get-Content $occLog2; throw "oracle mixa_audio_win32.lm1 compile failed" }

$oracleExe = Join-Path $RunDir "parity_oracle.exe"
$olLog1 = Join-Path $RunDir "oracle_link_stdout.log"
$olLog2 = Join-Path $RunDir "oracle_link_stderr.log"
$olExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" `"$harnessO`" `"$oracleO`" -o `"$oracleExe`" $GccLibs" $olLog1 $olLog2
if ($olExit -ne 0) { Get-Content $olLog2; throw "oracle harness link failed" }

Push-Location $RepoRoot
$oracleRunOut = Join-Path $RunDir "oracle_run_stdout.log"
$oracleRunErr = Join-Path $RunDir "oracle_run_stderr.log"
$oracleRunExit = Invoke-Cmd "`"$oracleExe`"" "`"$OracleFixtureDir`"" $oracleRunOut $oracleRunErr
Pop-Location
$OracleTrace = Get-Content -LiteralPath $oracleRunOut -Raw

# ---- Step 4: attempt the COMPLETE mixa_audio_win32.lm2 translation. ----
Push-Location $RepoRoot
$awSrc = "mixa_manager\mixa_audio_win32.lm2"
$awOut = Join-Path $RunDir "mixa_audio_win32_l2.lm1"
$awStdout = Join-Path $RunDir "aw_stdout.log"
$awStderr = Join-Path $RunDir "aw_stderr.log"
$AwExit = Invoke-Cmd "`"$l2exe`"" "`"$awSrc`" `"$awOut`"" $awStdout $awStderr
Pop-Location

$AwStdoutText = if (Test-Path -LiteralPath $awStdout) { Get-Content -LiteralPath $awStdout -Raw } else { "" }
$AwStderrText = if (Test-Path -LiteralPath $awStderr) { Get-Content -LiteralPath $awStderr -Raw } else { "" }
$AwSourceHash = (Get-FileHash -LiteralPath (Join-Path $RepoRoot $awSrc) -Algorithm SHA256).Hash
$RunnerHash = (Get-FileHash -LiteralPath $PSCommandPath -Algorithm SHA256).Hash

$Verdict = $null
$ExitCode = 1
$L2TraceText = ""
$DiffText = ""

$KnownBarrier = ($AwStderrText -match "unknown foreign type") -or ($AwStderrText -match "incompatible entry signature") -or ($AwStderrText -match "unsupported own array declaration")

if ($AwExit -ne 0 -and $KnownBarrier) {
    $Verdict = "EXPECTED_CORE_BARRIER"
    $ExitCode = 2
} elseif ($AwExit -ne 0) {
    $Verdict = "UNEXPECTED_FAILURE"
    $ExitCode = 1
} else {
    Push-Location $RepoRoot
    $l2AwC = Join-Path $RunDir "mixa_audio_win32_l2.c"
    $l2ccLog1 = Join-Path $RunDir "l2aw_trans_stdout.log"
    $l2ccLog2 = Join-Path $RunDir "l2aw_trans_stderr.log"
    $l2ccExit = Invoke-Cmd $L1Trans "`"$awOut`" `"$l2AwC`"" $l2ccLog1 $l2ccLog2
    Pop-Location
    if ($l2ccExit -ne 0) {
        Get-Content $l2ccLog2
        $Verdict = "UNEXPECTED_FAILURE"
        $ExitCode = 1
    } else {
        $l2AwO = Join-Path $RunDir "mixa_audio_win32_l2.o"
        $l2occLog1 = Join-Path $RunDir "l2aw_compile_stdout.log"
        $l2occLog2 = Join-Path $RunDir "l2aw_compile_stderr.log"
        $l2occExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" -c `"$l2AwC`" -o `"$l2AwO`"" $l2occLog1 $l2occLog2
        if ($l2occExit -ne 0) {
            Get-Content $l2occLog2
            $Verdict = "UNEXPECTED_FAILURE"
            $ExitCode = 1
        } else {
            $l2Exe = Join-Path $RunDir "parity_l2.exe"
            $l2olLog1 = Join-Path $RunDir "l2aw_link_stdout.log"
            $l2olLog2 = Join-Path $RunDir "l2aw_link_stderr.log"
            $l2olExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" `"$harnessO`" `"$l2AwO`" -o `"$l2Exe`" $GccLibs" $l2olLog1 $l2olLog2
            if ($l2olExit -ne 0) {
                Get-Content $l2olLog2
                $Verdict = "UNEXPECTED_FAILURE"
                $ExitCode = 1
            } else {
                $L2FixtureDir = Join-Path $RunDir "lf"
                New-Item -ItemType Directory -Force -Path $L2FixtureDir | Out-Null
                New-AudioWavFixture -Dir $L2FixtureDir
                Push-Location $RepoRoot
                $l2RunOut = Join-Path $RunDir "l2_run_stdout.log"
                $l2RunErr = Join-Path $RunDir "l2_run_stderr.log"
                $l2RunExit = Invoke-Cmd "`"$l2Exe`"" "`"$L2FixtureDir`"" $l2RunOut $l2RunErr
                Pop-Location
                $L2TraceText = Get-Content -LiteralPath $l2RunOut -Raw
                # Ticket 20260914-001000: normalize each side's own
                # fixture root before comparing, since the two sides use
                # separate, independently populated roots.
                $Norm = Get-NormalizedParityTraces -OracleTrace $OracleTrace -OracleRoot $OracleFixtureDir -L2Trace $L2TraceText -L2Root $L2FixtureDir
                if ($l2RunExit -eq 0 -and $oracleRunExit -eq 0 -and $Norm.L2 -eq $Norm.Oracle) {
                    $Verdict = "PASS"
                    $ExitCode = 0
                } else {
                    $Verdict = "PARITY_FAILURE"
                    $DiffText = Compare-Object -ReferenceObject ($Norm.Oracle -split "`n") -DifferenceObject ($Norm.L2 -split "`n") | Out-String
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
AudioWin32-L2-Header: mixa_manager\mixa_audio_win32_l2.h.lm1
AudioWin32-L2-Source: $awSrc
AudioWin32-L2-Source-Sha256: $AwSourceHash
Harness-Source-Sha256: $((Get-FileHash -LiteralPath (Join-Path $RepoRoot "mixa_manager\tests\mixa_audio_win32_parity_harness.lm1") -Algorithm SHA256).Hash)
Runner-Sha256: $RunnerHash
Oracle-Fixture-Directory: $OracleFixtureDir
Oracle-Run-Exit-Code: $oracleRunExit
Oracle-Trace:
$OracleTrace
AudioWin32-Translate-Exit-Code: $AwExit
AudioWin32-Translate-Stdout:
$AwStdoutText
AudioWin32-Translate-Stderr:
$AwStderrText
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
        "EXPECTED_CORE_BARRIER: full mixa_audio_win32.lm2 translation stopped at an already-known barrier. This is NOT a pass -- the integrated frontend is not yet on main. The oracle-side harness DID run against a real fixture WAV through the real MCI backend (see Oracle-Trace above)."
    }
    "UNEXPECTED_FAILURE" {
        "UNEXPECTED_FAILURE: translation or build failed with something OTHER than the already-known barriers. Inspect the logs under $RunDir."
    }
    "PARITY_FAILURE" {
        "PARITY_FAILURE: both implementations built and ran, but their (fixture-root-normalized) traces differ or one exited non-zero. See Diff above."
    }
    "PASS" {
        "PASS: full mixa_audio_win32.lm2 translated, built, and ran; its (fixture-root-normalized) trace is byte-identical to the L1 oracle's own trace."
    }
}

exit $ExitCode
