# Real L1-vs-L2 parity gate for mixa_app_win32 (dependency-order
# continuation, per Fable's own "then mixa_app_win32" instruction from
# ticket 20260913-200800, after the share-and-backends ticket's own
# extended scope -- mixa_backend_win32 and the ctors pair -- completed).
#
#   1. ALWAYS builds and runs the ORACLE-side harness against the same
#      real fixture set as the existing accepted mixa_app_selftest.lm1
#      (run_app_selftest.ps1's own real .lnk/.url/junction/fixture-exe
#      fixtures, reused verbatim -- same shape, same harmless marker-
#      writing fixture exe, same real invoke pattern).
#   2. Attempts to translate the COMPLETE mixa_app_win32.lm2.
#      - fails with an already-known barrier -> EXPECTED_CORE_BARRIER
#        (exit 2). NOT a pass.
#      - fails with any OTHER diagnostic -> UNEXPECTED_FAILURE (exit 1).
#      - SUCCEEDS -> builds/links/runs the L2-side harness and diffs its
#        stdout against the oracle trace byte-for-byte. PASS (exit 0)
#        only on an exact match, else PARITY_FAILURE (exit 1).
#
# mixa_app_win32.lm1, mixa_app_win32.h.lm1, and mixa_app.h.lm1 are the
# parity oracle and are never touched. Nothing under stg/l1_baseline is
# modified, only read. Every input is built fresh in a unique run
# directory -- no stale objects.
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
$GccLibs = "-lshell32"

$RunTimestamp = (Get-Date -Format "yyyyMMdd_HHmmss_fff")
$RunGuid = [GUID]::NewGuid().ToString().Substring(0, 8)
$BaseDir = Join-Path $RepoRoot "build\mixa\claude\mixa_app_win32_l2_parity"
$RunDir = Join-Path $BaseDir "run_${RunTimestamp}_${RunGuid}"
$HeaderDir = Join-Path $RunDir "headers\mixa_manager"
$FixtureDir = Join-Path $RunDir "fixtures"
New-Item -ItemType Directory -Force -Path $HeaderDir | Out-Null
New-Item -ItemType Directory -Force -Path $FixtureDir | Out-Null

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

# ---- Step 0: translate the two real L1 headers (concrete + portable),
# then the L2 header unit that predefs both of them. ----
Invoke-HeaderTrans "mixa_manager\mixa_app_win32.h.lm1" "mixa_app_win32.lm1.h"
Invoke-HeaderTrans "mixa_manager\mixa_app.h.lm1" "mixa_app.lm1.h"
Invoke-HeaderTrans "mixa_manager\mixa_app_win32_l2.h.lm1" "mixa_app_win32_l2.lm1.h"

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

# ---- Step 2: translate + compile the L1 fixture exe (used by .lnk fixtures). ----
$FixtureSource = Join-Path $RepoRoot "mixa_manager\tests\mixa_app_fixture_invoke.lm1"
$FixtureExe = Join-Path $FixtureDir "mixa_app_fixture_invoke.exe"
Push-Location $RepoRoot
$fixC = Join-Path $RunDir "mixa_app_fixture_invoke.c"
$fLog1 = Join-Path $RunDir "fixture_trans_stdout.log"
$fLog2 = Join-Path $RunDir "fixture_trans_stderr.log"
$fExit = Invoke-Cmd $L1Trans "mixa_manager\tests\mixa_app_fixture_invoke.lm1 `"$fixC`"" $fLog1 $fLog2
Pop-Location
if ($fExit -ne 0) { Get-Content $fLog2; throw "fixture exe translation failed" }
$fcLog1 = Join-Path $RunDir "fixture_compile_stdout.log"
$fcLog2 = Join-Path $RunDir "fixture_compile_stderr.log"
$fcExit = Invoke-Cmd "gcc" "-std=c99 -Wall -Wextra -O2 `"$fixC`" -o `"$FixtureExe`"" $fcLog1 $fcLog2
if ($fcExit -ne 0) { Get-Content $fcLog2; throw "fixture exe compile failed" }

# ---- Step 3: build the harness object (compiled ONCE). ----
Push-Location $RepoRoot
$harnessC = Join-Path $RunDir "harness.c"
$hLog1 = Join-Path $RunDir "harness_trans_stdout.log"
$hLog2 = Join-Path $RunDir "harness_trans_stderr.log"
$hExit = Invoke-Cmd $L1Trans "mixa_manager\tests\mixa_app_win32_parity_harness.lm1 `"$harnessC`"" $hLog1 $hLog2
Pop-Location
if ($hExit -ne 0) { Get-Content $hLog2; throw "harness translation failed" }

$harnessO = Join-Path $RunDir "harness.o"
$hcLog1 = Join-Path $RunDir "harness_compile_stdout.log"
$hcLog2 = Join-Path $RunDir "harness_compile_stderr.log"
$hcExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" -c `"$harnessC`" -o `"$harnessO`"" $hcLog1 $hcLog2
if ($hcExit -ne 0) { Get-Content $hcLog2; throw "harness compile failed" }

# ---- Step 4: real fixture creation (same shape as run_app_selftest.ps1). ----
$Wsh = New-Object -ComObject WScript.Shell

$ValidLnk = Join-Path $FixtureDir "valid.lnk"
$ValidSc = $Wsh.CreateShortcut($ValidLnk)
$ValidSc.TargetPath = $FixtureExe
$MarkerPath = Join-Path $FixtureDir "mixa_app_invoked.marker"
$ValidSc.Arguments = "-marker `"$MarkerPath`""
$ValidSc.WorkingDirectory = $FixtureDir
$ValidSc.Description = "app win32 clean-L2 parity launch fixture"
$ValidSc.Save()

$Name56 = "sp ace-" + [string][char]0x0444 + [string][char]0x0430 + [string][char]0x0439 + [string][char]0x043B
$SpaceLnk = Join-Path $FixtureDir ($Name56 + ".lnk")
$SpaceSc = $Wsh.CreateShortcut($SpaceLnk)
$SpaceSc.TargetPath = $FixtureExe
$SpaceSc.Arguments = "-marker `"$MarkerPath`""
$SpaceSc.WorkingDirectory = $FixtureDir
$SpaceSc.Save()

$BrokenLnk = Join-Path $FixtureDir "broken.lnk"
$BrokenSc = $Wsh.CreateShortcut($BrokenLnk)
$BrokenSc.TargetPath = Join-Path $FixtureDir "no_such_target_xyz.exe"
$BrokenSc.WorkingDirectory = $FixtureDir
$BrokenSc.Save()

Set-Content -LiteralPath (Join-Path $FixtureDir "web.url") -Value "[InternetShortcut]`nURL=https://example.com/"
Set-Content -LiteralPath (Join-Path $FixtureDir "plain.txt") -Value "not a shortcut"
New-Item -ItemType Directory -Path (Join-Path $FixtureDir "dir.lnk") -Force | Out-Null

$JTarget = Join-Path $FixtureDir "jtarget"
New-Item -ItemType Directory -Path $JTarget -Force | Out-Null
$JunctionProc = Start-Process -FilePath "cmd.exe" -ArgumentList @("/c", "mklink", "/J", "js_link", "jtarget") -Wait -PassThru -NoNewWindow -WorkingDirectory $FixtureDir
if ($JunctionProc.ExitCode -ne 0) { throw "mklink /J failed with exit $($JunctionProc.ExitCode)" }

New-Item -ItemType Directory -Path (Join-Path $FixtureDir "appdir") -Force | Out-Null
New-Item -ItemType Directory -Path (Join-Path $FixtureDir "emptydir") -Force | Out-Null
$CorruptDir = Join-Path $FixtureDir "corruptdir"
New-Item -ItemType Directory -Path $CorruptDir -Force | Out-Null
Set-Content -LiteralPath (Join-Path $CorruptDir "bad.link") -Value "this is not a mixa-app-link-v1 entry"

$TinyDir = Join-Path $FixtureDir "tinyfiles"
New-Item -ItemType Directory -Path $TinyDir -Force | Out-Null
[IO.File]::WriteAllBytes((Join-Path $TinyDir "empty.link"), [byte[]]@())
[IO.File]::WriteAllBytes((Join-Path $TinyDir "onebyte.link"), [System.Text.Encoding]::ASCII.GetBytes("m"))
[IO.File]::WriteAllBytes((Join-Path $TinyDir "sixteen.link"), [System.Text.Encoding]::ASCII.GetBytes("mixa-app-link-v1"))

# ---- Step 5: ALWAYS build + run the ORACLE-side harness. ----
$oracleC = Join-Path $RunDir "mixa_app_win32_oracle.c"
$ocLog1 = Join-Path $RunDir "oracle_trans_stdout.log"
$ocLog2 = Join-Path $RunDir "oracle_trans_stderr.log"
Push-Location $RepoRoot
$ocExit = Invoke-Cmd $L1Trans "mixa_manager\mixa_app_win32.lm1 `"$oracleC`"" $ocLog1 $ocLog2
Pop-Location
if ($ocExit -ne 0) { Get-Content $ocLog2; throw "oracle mixa_app_win32.lm1 translation failed" }

$oracleO = Join-Path $RunDir "mixa_app_win32_oracle.o"
$occLog1 = Join-Path $RunDir "oracle_compile_stdout.log"
$occLog2 = Join-Path $RunDir "oracle_compile_stderr.log"
$occExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" -c `"$oracleC`" -o `"$oracleO`"" $occLog1 $occLog2
if ($occExit -ne 0) { Get-Content $occLog2; throw "oracle mixa_app_win32.lm1 compile failed" }

$oracleExe = Join-Path $RunDir "parity_oracle.exe"
$olLog1 = Join-Path $RunDir "oracle_link_stdout.log"
$olLog2 = Join-Path $RunDir "oracle_link_stderr.log"
$olExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" `"$harnessO`" `"$oracleO`" -o `"$oracleExe`" $GccLibs" $olLog1 $olLog2
if ($olExit -ne 0) { Get-Content $olLog2; throw "oracle harness link failed" }

Push-Location $RepoRoot
$oracleRunOut = Join-Path $RunDir "oracle_run_stdout.log"
$oracleRunErr = Join-Path $RunDir "oracle_run_stderr.log"
$oracleRunExit = Invoke-Cmd "`"$oracleExe`"" "`"$FixtureDir`"" $oracleRunOut $oracleRunErr
Pop-Location
$OracleTrace = Get-Content -LiteralPath $oracleRunOut -Raw

# ---- Step 6: attempt the COMPLETE mixa_app_win32.lm2 translation. ----
Push-Location $RepoRoot
$awSrc = "mixa_manager\mixa_app_win32.lm2"
$awOut = Join-Path $RunDir "mixa_app_win32_l2.lm1"
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
    $l2AwC = Join-Path $RunDir "mixa_app_win32_l2.c"
    $l2ccLog1 = Join-Path $RunDir "l2aw_trans_stdout.log"
    $l2ccLog2 = Join-Path $RunDir "l2aw_trans_stderr.log"
    $l2ccExit = Invoke-Cmd $L1Trans "`"$awOut`" `"$l2AwC`"" $l2ccLog1 $l2ccLog2
    Pop-Location
    if ($l2ccExit -ne 0) {
        Get-Content $l2ccLog2
        $Verdict = "UNEXPECTED_FAILURE"
        $ExitCode = 1
    } else {
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
        $l2AwO = Join-Path $RunDir "mixa_app_win32_l2.o"
        $l2occLog1 = Join-Path $RunDir "l2aw_compile_stdout.log"
        $l2occLog2 = Join-Path $RunDir "l2aw_compile_stderr.log"
        $l2occExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" -I `"$L2RuntimeHeaderRoot`" -c `"$l2AwC`" -o `"$l2AwO`"" $l2occLog1 $l2occLog2
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
L2trans-Source (informational, not pinned): $L2TransSource
L2trans-Source-Sha256 (informational, not pinned): $L2TransSourceHash
L2trans-Exe-Sha256 (rebuilt fresh this run, not a stable artifact): $L2TransExeHash
AppWin32-L2-Header: mixa_manager\mixa_app_win32_l2.h.lm1
AppWin32-L2-Source: $awSrc
AppWin32-L2-Source-Sha256: $AwSourceHash
Harness-Source-Sha256: $((Get-FileHash -LiteralPath (Join-Path $RepoRoot "mixa_manager\tests\mixa_app_win32_parity_harness.lm1") -Algorithm SHA256).Hash)
Runner-Sha256: $RunnerHash
Fixture-Directory: $FixtureDir
Oracle-Run-Exit-Code: $oracleRunExit
Oracle-Trace:
$OracleTrace
AppWin32-Translate-Exit-Code: $AwExit
AppWin32-Translate-Stdout:
$AwStdoutText
AppWin32-Translate-Stderr:
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
        "EXPECTED_CORE_BARRIER: full mixa_app_win32.lm2 translation stopped at an already-known barrier. This is NOT a pass -- the integrated frontend is not yet on main. The oracle-side harness DID run (see Oracle-Trace above)."
    }
    "UNEXPECTED_FAILURE" {
        "UNEXPECTED_FAILURE: translation or build failed with something OTHER than the already-known barriers. Inspect the logs under $RunDir."
    }
    "PARITY_FAILURE" {
        "PARITY_FAILURE: both implementations built and ran, but their traces differ (see Diff above) or one exited non-zero."
    }
    "PASS" {
        "PASS: full mixa_app_win32.lm2 translated, built, and ran; its trace is byte-identical to the L1 oracle's own trace."
    }
}

exit $ExitCode
