# Real L1-vs-L2 parity gate for mixa_audio_button (ticket 20260913-
# 232000/231500 Part 2, module 6 of 8). Fully self-contained: no OS or
# native dependency of any kind, no L1 header unit predef beyond the
# module's own real header.
#
# GENUINE NEW OBSERVATION, distinct from every other module this
# session: mixa_audio_button.lm2 translated ALONE (Step 4 below) does
# NOT hit the closed-type-allowlist at all. It fails instead with
# "missing main", because every other module's own body contains at
# least one function whose signature trips a per-function check before
# l2trans would ever get far enough to notice there is no main() in the
# file -- this module's seven functions all take a plain (@: void) --
# a universally-supported void pointer, not a foreign struct -- so
# nothing in THEM trips anything. Confirmed by an isolated scratch
# probe: a single trivial function with no custom types, alone with no
# main, gets the identical "missing main". This is a structural
# artifact of testing a pure-library fragment in isolation (this module
# has no main of its own; it is always consumed by a caller), not a
# rejection of this module's own code -- so it is classified separately
# below as NO_MAIN_IN_ISOLATION, never folded into EXPECTED_CORE_
# BARRIER (which specifically means "the closed-type checker rejected
# something").
#
#   1. ALWAYS builds and runs the ORACLE-side harness.
#   2. Attempts to translate the COMPLETE mixa_audio_button.lm2 ALONE.
#      - "missing main" -> NO_MAIN_IN_ISOLATION (exit 3). Not the usual
#        barrier; see above.
#      - fails with an already-known barrier -> EXPECTED_CORE_BARRIER
#        (exit 2). NOT a pass.
#      - fails with any OTHER diagnostic -> UNEXPECTED_FAILURE (exit 1).
#      - SUCCEEDS -> builds/links/runs the L2-side harness and diffs its
#        stdout against the oracle trace byte-for-byte. PASS (exit 0)
#        only on an exact match, else PARITY_FAILURE (exit 1).
#
# mixa_audio_button.lm1 and mixa_audio_button.h.lm1 are the parity
# oracle and are never touched. Nothing under stg/l1_baseline is
# modified, only read. Every input is built fresh in a unique run
# directory -- no stale objects.
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
$BaseDir = Join-Path $RepoRoot "build\mixa\claude\mixa_audio_button_l2_parity"
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

# ---- Step 0: translate the real header, then the L2 header unit. ----
Invoke-HeaderTrans "mixa_manager\mixa_audio_win32.h.lm1" "mixa_audio_win32.lm1.h"
Invoke-HeaderTrans "mixa_manager\mixa_audio.h.lm1" "mixa_audio.lm1.h"
Invoke-HeaderTrans "mixa_manager\mixa_audio_button.h.lm1" "mixa_audio_button.lm1.h"
Invoke-HeaderTrans "mixa_manager\mixa_audio_button_l2.h.lm1" "mixa_audio_button_l2.lm1.h"

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
$hExit = Invoke-Cmd $L1Trans "mixa_manager\tests\mixa_audio_button_parity_harness.lm1 `"$harnessC`"" $hLog1 $hLog2
Pop-Location
if ($hExit -ne 0) { Get-Content $hLog2; throw "harness translation failed" }

$harnessO = Join-Path $RunDir "harness.o"
$hcLog1 = Join-Path $RunDir "harness_compile_stdout.log"
$hcLog2 = Join-Path $RunDir "harness_compile_stderr.log"
$hcExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" -c `"$harnessC`" -o `"$harnessO`"" $hcLog1 $hcLog2
if ($hcExit -ne 0) { Get-Content $hcLog2; throw "harness compile failed" }

# ---- Step 3: ALWAYS build + run the ORACLE-side harness. ----
$oracleC = Join-Path $RunDir "mixa_audio_button_oracle.c"
$ocLog1 = Join-Path $RunDir "oracle_trans_stdout.log"
$ocLog2 = Join-Path $RunDir "oracle_trans_stderr.log"
Push-Location $RepoRoot
$ocExit = Invoke-Cmd $L1Trans "mixa_manager\mixa_audio_button.lm1 `"$oracleC`"" $ocLog1 $ocLog2
Pop-Location
if ($ocExit -ne 0) { Get-Content $ocLog2; throw "oracle mixa_audio_button.lm1 translation failed" }

$oracleO = Join-Path $RunDir "mixa_audio_button_oracle.o"
$occLog1 = Join-Path $RunDir "oracle_compile_stdout.log"
$occLog2 = Join-Path $RunDir "oracle_compile_stderr.log"
$occExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" -c `"$oracleC`" -o `"$oracleO`"" $occLog1 $occLog2
if ($occExit -ne 0) { Get-Content $occLog2; throw "oracle mixa_audio_button.lm1 compile failed" }

$oracleExe = Join-Path $RunDir "parity_oracle.exe"
$olLog1 = Join-Path $RunDir "oracle_link_stdout.log"
$olLog2 = Join-Path $RunDir "oracle_link_stderr.log"
$olExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" `"$harnessO`" `"$oracleO`" -o `"$oracleExe`"" $olLog1 $olLog2
if ($olExit -ne 0) { Get-Content $olLog2; throw "oracle harness link failed" }

$oracleRunOut = Join-Path $RunDir "oracle_run_stdout.log"
$oracleRunErr = Join-Path $RunDir "oracle_run_stderr.log"
$oracleRunExit = Invoke-Cmd "`"$oracleExe`"" "" $oracleRunOut $oracleRunErr
$OracleTrace = Get-Content -LiteralPath $oracleRunOut -Raw

# ---- Step 4: attempt the COMPLETE mixa_audio_button.lm2 translation
# ALONE (no harness predef -- this is what isolates the "missing main"
# finding; see the header note above). ----
Push-Location $RepoRoot
$abSrc = "mixa_manager\mixa_audio_button.lm2"
$abOut = Join-Path $RunDir "mixa_audio_button_l2.lm1"
$abStdout = Join-Path $RunDir "ab_stdout.log"
$abStderr = Join-Path $RunDir "ab_stderr.log"
$AbExit = Invoke-Cmd "`"$l2exe`"" "`"$abSrc`" `"$abOut`"" $abStdout $abStderr
Pop-Location

$AbStdoutText = if (Test-Path -LiteralPath $abStdout) { Get-Content -LiteralPath $abStdout -Raw } else { "" }
$AbStderrText = if (Test-Path -LiteralPath $abStderr) { Get-Content -LiteralPath $abStderr -Raw } else { "" }
$AbSourceHash = (Get-FileHash -LiteralPath (Join-Path $RepoRoot $abSrc) -Algorithm SHA256).Hash
$RunnerHash = (Get-FileHash -LiteralPath $PSCommandPath -Algorithm SHA256).Hash

$Verdict = $null
$ExitCode = 1
$L2TraceText = ""
$DiffText = ""

$KnownBarrier = ($AbStderrText -match "unknown foreign type") -or ($AbStderrText -match "incompatible entry signature") -or ($AbStderrText -match "unsupported own array declaration")
$NoMain = ($AbStderrText -match "missing main")

if ($AbExit -ne 0 -and $NoMain) {
    $Verdict = "NO_MAIN_IN_ISOLATION"
    $ExitCode = 3
} elseif ($AbExit -ne 0 -and $KnownBarrier) {
    $Verdict = "EXPECTED_CORE_BARRIER"
    $ExitCode = 2
} elseif ($AbExit -ne 0) {
    $Verdict = "UNEXPECTED_FAILURE"
    $ExitCode = 1
} else {
    Push-Location $RepoRoot
    $l2AbC = Join-Path $RunDir "mixa_audio_button_l2.c"
    $l2ccLog1 = Join-Path $RunDir "l2ab_trans_stdout.log"
    $l2ccLog2 = Join-Path $RunDir "l2ab_trans_stderr.log"
    $l2ccExit = Invoke-Cmd $L1Trans "`"$abOut`" `"$l2AbC`"" $l2ccLog1 $l2ccLog2
    Pop-Location
    if ($l2ccExit -ne 0) {
        Get-Content $l2ccLog2
        $Verdict = "UNEXPECTED_FAILURE"
        $ExitCode = 1
    } else {
        $l2AbO = Join-Path $RunDir "mixa_audio_button_l2.o"
        $l2occLog1 = Join-Path $RunDir "l2ab_compile_stdout.log"
        $l2occLog2 = Join-Path $RunDir "l2ab_compile_stderr.log"
        $l2occExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" -c `"$l2AbC`" -o `"$l2AbO`"" $l2occLog1 $l2occLog2
        if ($l2occExit -ne 0) {
            Get-Content $l2occLog2
            $Verdict = "UNEXPECTED_FAILURE"
            $ExitCode = 1
        } else {
            $l2Exe = Join-Path $RunDir "parity_l2.exe"
            $l2olLog1 = Join-Path $RunDir "l2ab_link_stdout.log"
            $l2olLog2 = Join-Path $RunDir "l2ab_link_stderr.log"
            $l2olExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" `"$harnessO`" `"$l2AbO`" -o `"$l2Exe`"" $l2olLog1 $l2olLog2
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
AudioButton-L2-Header: mixa_manager\mixa_audio_button_l2.h.lm1
AudioButton-L2-Source: $abSrc
AudioButton-L2-Source-Sha256: $AbSourceHash
Harness-Source-Sha256: $((Get-FileHash -LiteralPath (Join-Path $RepoRoot "mixa_manager\tests\mixa_audio_button_parity_harness.lm1") -Algorithm SHA256).Hash)
Runner-Sha256: $RunnerHash
Oracle-Run-Exit-Code: $oracleRunExit
Oracle-Trace:
$OracleTrace
AudioButton-Translate-Exit-Code: $AbExit
AudioButton-Translate-Stdout:
$AbStdoutText
AudioButton-Translate-Stderr:
$AbStderrText
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
    "NO_MAIN_IN_ISOLATION" {
        "NO_MAIN_IN_ISOLATION: mixa_audio_button.lm2 translated ALONE has no function named main anywhere in its file/predef closure, and l2trans requires one to exist before it will process a file at all. None of this module's own seven functions trips the closed-type-allowlist on their own (confirmed by an isolated scratch probe with a trivial no-custom-type function alone in a mainless file, which fails identically). This is NOT the same as EXPECTED_CORE_BARRIER -- it says nothing about whether this module's own code is clean; it only reflects that testing an entry-point-less library fragment in isolation was never how l2trans expects to be invoked. The oracle-side harness DID run (see Oracle-Trace above)."
    }
    "EXPECTED_CORE_BARRIER" {
        "EXPECTED_CORE_BARRIER: full mixa_audio_button.lm2 translation stopped at an already-known barrier. This is NOT a pass -- the integrated frontend is not yet on main. The oracle-side harness DID run (see Oracle-Trace above)."
    }
    "UNEXPECTED_FAILURE" {
        "UNEXPECTED_FAILURE: translation or build failed with something OTHER than the already-known barriers or the no-main condition. Inspect the logs under $RunDir."
    }
    "PARITY_FAILURE" {
        "PARITY_FAILURE: both implementations built and ran, but their traces differ (see Diff above) or one exited non-zero."
    }
    "PASS" {
        "PASS: full mixa_audio_button.lm2 translated, built, and ran; its trace is byte-identical to the L1 oracle's own trace."
    }
}

exit $ExitCode
