# Real L1-vs-L2 parity gate for mixa_app_panel (ticket 20260913-
# 152000). Own dedicated ABI probe pair (mixa_app_panel_abi_probe_
# real.c/_l2.c -- both sides compile against an l1trans-GENERATED C
# header, since unlike most modules this segment mixa_app_panel has no
# hand-written plain-C header at all: both its real header (mixa_app_
# panel.h.lm1) and its L2 header (mixa_app_panel_l2.h.lm1) are genuine
# L1 .h.lm1 units).
#
# Dependency-linking note: mixa_app_panel.lm1 predefs the FULL body of
# mixa_app_window.lm1 directly, which itself predefs the full bodies of
# mixa_buttons.lm1 (-> mixa_tiles.lm1) and mixa_draw.lm1 (-> mixa_text_
# rect.lm1) -- so the ORACLE side's own mixa_app_panel.lm1 translation
# unit already embeds app_window/buttons/draw/tiles/text_rect bodies
# and needs no separate objects for them. The L2 header instead predefs
# ONLY header-only chains (mixa_button_dispatch_l2.h.lm1, mixa_app.h.
# lm1) with no bodies at all, so the L2 side separately links the real,
# unmodified mixa_app_window.lm1 (which embeds buttons/draw/tiles/
# text_rect the same way) for those symbols. Both sides also link the
# real, unmodified mixa_app_win32.lm1 (mixa_app_list_*/mixa_app_entry_*
# /mixa_app_invoke -- neither header chain embeds this body).
#
#   0. ABI parity gate: dedicated MixaAppPanelEntry/MixaAppPanel probe
#      pair.
#   1. ALWAYS builds and runs the ORACLE-side harness (real .lnk
#      fixtures via WScript.Shell, real enumeration/invoke/marker
#      files, real layout rejection).
#   2. Attempts to translate the COMPLETE mixa_app_panel.lm2.
#      - fails with an already-known barrier -> EXPECTED_CORE_BARRIER
#        (exit 2). NOT a pass.
#      - fails with any OTHER diagnostic -> UNEXPECTED_FAILURE (exit 1).
#      - SUCCEEDS -> builds/links/runs the L2-side harness and diffs its
#        stdout against the oracle trace byte-for-byte. PASS (exit 0)
#        only on an exact match, else PARITY_FAILURE (exit 1).
#
# mixa_app_panel.h.lm1/.lm1 and every real dependency (mixa_app_window.
# h.lm1/.lm1, mixa_app.h.lm1, mixa_app_win32.h.lm1/.lm1, mixa_buttons.
# h/.lm1, mixa_draw.h/.lm1, mixa_tiles.h/.lm1, mixa_text_rect.h/.lm1)
# are the parity oracle (or real, unmodified dependencies) and are never
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
$LinkLibs = "-lshell32"

$RunTimestamp = (Get-Date -Format "yyyyMMdd_HHmmss_fff")
$RunGuid = [GUID]::NewGuid().ToString().Substring(0, 8)
$BaseDir = Join-Path $RepoRoot "build\mixa\claude\mixa_app_panel_l2_parity"
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

# ---- Step 0: translate the L2 header and its own real predef chain,
# plus the REAL header (for the ABI probe's own real side). ----
Invoke-HeaderTrans "mixa_manager\mixa_app_win32.h.lm1" "mixa_app_win32.lm1.h"
Invoke-HeaderTrans "mixa_manager\mixa_app.h.lm1" "mixa_app.lm1.h"
Invoke-HeaderTrans "mixa_manager\mixa_app_window.h.lm1" "mixa_app_window.lm1.h"
Invoke-HeaderTrans "mixa_manager\mixa_app_panel.h.lm1" "mixa_app_panel.lm1.h"
Invoke-HeaderTrans "mixa_manager\mixa_tiles_l2.h.lm1" "mixa_tiles_l2.lm1.h"
Invoke-HeaderTrans "mixa_manager\mixa_button_dispatch_l2.h.lm1" "mixa_button_dispatch_l2.lm1.h"
Invoke-HeaderTrans "mixa_manager\mixa_app_panel_l2.h.lm1" "mixa_app_panel_l2.lm1.h"

# ---- Step 0.5: ABI parity gate (dedicated probe pair). ----
$abiRealExe = Join-Path $RunDir "abi_probe_real.exe"
$abiRealLog1 = Join-Path $RunDir "abi_probe_real_compile_stdout.log"
$abiRealLog2 = Join-Path $RunDir "abi_probe_real_compile_stderr.log"
$abiRealExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" `"$RepoRoot\mixa_manager\tests\mixa_app_panel_abi_probe_real.c`" -o `"$abiRealExe`"" $abiRealLog1 $abiRealLog2
if ($abiRealExit -ne 0) { Get-Content $abiRealLog2; throw "ABI probe (real header) compile failed" }

$abiL2Exe = Join-Path $RunDir "abi_probe_l2.exe"
$abiL2Log1 = Join-Path $RunDir "abi_probe_l2_compile_stdout.log"
$abiL2Log2 = Join-Path $RunDir "abi_probe_l2_compile_stderr.log"
$abiL2Exit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" `"$RepoRoot\mixa_manager\tests\mixa_app_panel_abi_probe_l2.c`" -o `"$abiL2Exe`"" $abiL2Log1 $abiL2Log2
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

# ---- Step 2: build the harness object + the real dependency objects. ----
Push-Location $RepoRoot
$harnessC = Join-Path $RunDir "harness.c"
$hLog1 = Join-Path $RunDir "harness_trans_stdout.log"
$hLog2 = Join-Path $RunDir "harness_trans_stderr.log"
$hExit = Invoke-Cmd $L1Trans "mixa_manager\tests\mixa_app_panel_parity_harness.lm1 `"$harnessC`"" $hLog1 $hLog2
Pop-Location
if ($hExit -ne 0) { Get-Content $hLog2; throw "harness translation failed" }

$harnessO = Join-Path $RunDir "harness.o"
$hcLog1 = Join-Path $RunDir "harness_compile_stdout.log"
$hcLog2 = Join-Path $RunDir "harness_compile_stderr.log"
$hcExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" -c `"$harnessC`" -o `"$harnessO`"" $hcLog1 $hcLog2
if ($hcExit -ne 0) { Get-Content $hcLog2; throw "harness compile failed" }

# mixa_app_win32.lm1 (real body: mixa_app_list_*/mixa_app_entry_*/
# mixa_app_invoke) is needed by BOTH sides -- neither header chain
# embeds it.
$appWin32O = Build-RealDep "mixa_app_win32" "mixa_manager\mixa_app_win32.lm1"
# mixa_app_window.lm1 (real body: also embeds buttons/draw/tiles/
# text_rect transitively) is needed ONLY by the L2 side -- the oracle
# side's own mixa_app_panel.lm1 already embeds it via its own predef
# chain (see this file's own top comment).
$appWindowO = Build-RealDep "mixa_app_window" "mixa_manager\mixa_app_window.lm1"

# ---- Step 3: ALWAYS build + run the ORACLE-side harness. mixa_app_
# panel.lm1's own translation unit already embeds app_window/buttons/
# draw/tiles/text_rect via its own predef chain (see top comment);
# tests/mixa_app_panel_oracle_link.lm1 (test-owned, not the oracle
# itself) additionally predefs the real mixa_app_win32.lm1 body into
# the SAME translation unit -- both for its own stdio.h ordering and so
# mixa_app_invoke resolves -- so the oracle side needs only ONE real
# object linked alongside the harness. ----
$oracleC = Join-Path $RunDir "mixa_app_panel_oracle.c"
$ocLog1 = Join-Path $RunDir "oracle_trans_stdout.log"
$ocLog2 = Join-Path $RunDir "oracle_trans_stderr.log"
Push-Location $RepoRoot
$ocExit = Invoke-Cmd $L1Trans "mixa_manager\tests\mixa_app_panel_oracle_link.lm1 `"$oracleC`"" $ocLog1 $ocLog2
Pop-Location
if ($ocExit -ne 0) { Get-Content $ocLog2; throw "oracle mixa_app_panel.lm1 translation failed" }

$oracleO = Join-Path $RunDir "mixa_app_panel_oracle.o"
$occLog1 = Join-Path $RunDir "oracle_compile_stdout.log"
$occLog2 = Join-Path $RunDir "oracle_compile_stderr.log"
$occExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" -c `"$oracleC`" -o `"$oracleO`"" $occLog1 $occLog2
if ($occExit -ne 0) { Get-Content $occLog2; throw "oracle mixa_app_panel.lm1 compile failed" }

$oracleExe = Join-Path $RunDir "parity_oracle.exe"
$olLog1 = Join-Path $RunDir "oracle_link_stdout.log"
$olLog2 = Join-Path $RunDir "oracle_link_stderr.log"
$olExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" `"$harnessO`" `"$oracleO`" -o `"$oracleExe`" $LinkLibs" $olLog1 $olLog2
if ($olExit -ne 0) { Get-Content $olLog2; throw "oracle harness link failed" }

# Real .lnk fixtures (WScript.Shell), same approach as the existing
# accepted run_app_panel_selftest.ps1: two shortcuts to a harmless
# marker-writing fixture exe, one broken shortcut (ERR_LAUNCH).
$Stage = "fixture-creation"
$FixtureExeSrc = Join-Path $RepoRoot "mixa_manager\tests\mixa_app_fixture_invoke.lm1"
$FixtureExeC = Join-Path $RunDir "mixa_app_fixture_invoke.c"
$FixtureExe = Join-Path $FixtureDir "mixa_app_fixture_invoke.exe"
Push-Location $RepoRoot
$fixTransExit = Invoke-Cmd $L1Trans "mixa_manager\tests\mixa_app_fixture_invoke.lm1 `"$FixtureExeC`"" (Join-Path $RunDir "fixture_trans_stdout.log") (Join-Path $RunDir "fixture_trans_stderr.log")
Pop-Location
if ($fixTransExit -ne 0) { throw "fixture invoke translation failed" }
$fixCompileExit = Invoke-Cmd "gcc" "-std=c99 -Wall -Wextra -O2 `"$FixtureExeC`" -o `"$FixtureExe`"" (Join-Path $RunDir "fixture_compile_stdout.log") (Join-Path $RunDir "fixture_compile_stderr.log")
if ($fixCompileExit -ne 0) { throw "fixture invoke compile failed" }

$Wsh = New-Object -ComObject WScript.Shell
$LnkA = Join-Path $FixtureDir "ab.lnk"
$MarkerA = Join-Path $FixtureDir "marker_a.txt"
$ScA = $Wsh.CreateShortcut($LnkA)
$ScA.TargetPath = $FixtureExe
$ScA.Arguments = "-marker `"$MarkerA`""
$ScA.WorkingDirectory = $FixtureDir
$ScA.Save()

$LnkB = Join-Path $FixtureDir "cd.lnk"
$MarkerB = Join-Path $FixtureDir "marker_b.txt"
$ScB = $Wsh.CreateShortcut($LnkB)
$ScB.TargetPath = $FixtureExe
$ScB.Arguments = "-marker `"$MarkerB`""
$ScB.WorkingDirectory = $FixtureDir
$ScB.Save()

$LnkGhost = Join-Path $FixtureDir "ghost.lnk"
$ScGhost = $Wsh.CreateShortcut($LnkGhost)
$ScGhost.TargetPath = Join-Path $FixtureDir "no_such_target_xyz.exe"
$ScGhost.WorkingDirectory = $FixtureDir
$ScGhost.Save()

Push-Location $RepoRoot
$oracleRunOut = Join-Path $RunDir "oracle_run_stdout.log"
$oracleRunErr = Join-Path $RunDir "oracle_run_stderr.log"
$oracleRunExit = Invoke-Cmd "`"$oracleExe`"" "`"$RunDir`"" $oracleRunOut $oracleRunErr
Pop-Location
$OracleTrace = Get-Content -LiteralPath $oracleRunOut -Raw

# ---- Step 4: attempt the COMPLETE mixa_app_panel.lm2 translation. ----
Push-Location $RepoRoot
$apSrc = "mixa_manager\mixa_app_panel.lm2"
$apOut = Join-Path $RunDir "mixa_app_panel_l2.lm1"
$apStdout = Join-Path $RunDir "ap_stdout.log"
$apStderr = Join-Path $RunDir "ap_stderr.log"
$env:L2_RUNTIME_ROOT = "stg/l1_baseline/l2src/"
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
    Push-Location $RepoRoot
    $l2ApC = Join-Path $RunDir "mixa_app_panel_l2.c"
    $l2ccLog1 = Join-Path $RunDir "l2ap_trans_stdout.log"
    $l2ccLog2 = Join-Path $RunDir "l2ap_trans_stderr.log"
    $l2ccExit = Invoke-Cmd $L1Trans "`"$apOut`" `"$l2ApC`"" $l2ccLog1 $l2ccLog2
    Pop-Location
    if ($l2ccExit -ne 0) {
        Get-Content $l2ccLog2
        $Verdict = "UNEXPECTED_FAILURE"
        $ExitCode = 1
    } else {
        $L2Rt = Add-L2RuntimeSupport -L1Trans $L1Trans -L1Root $L1Root -RunDir $RunDir -InvokeCmd $InvokeCmdRef
        $l2ApO = Join-Path $RunDir "mixa_app_panel_l2.o"
        $l2occLog1 = Join-Path $RunDir "l2ap_compile_stdout.log"
        $l2occLog2 = Join-Path $RunDir "l2ap_compile_stderr.log"
        $l2occExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" -I `"$($L2Rt.HeaderRoot)`" -I `"$($L2Rt.HeaderRoot)\stg\l1_baseline`" -I `"$L1Root`" -c `"$l2ApC`" -o `"$l2ApO`"" $l2occLog1 $l2occLog2
        if ($l2occExit -ne 0) {
            Get-Content $l2occLog2
            $Verdict = "UNEXPECTED_FAILURE"
            $ExitCode = 1
        } else {
            $l2Exe = Join-Path $RunDir "parity_l2.exe"
            $l2olLog1 = Join-Path $RunDir "l2ap_link_stdout.log"
            $l2olLog2 = Join-Path $RunDir "l2ap_link_stderr.log"
            $l2olExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" `"$harnessO`" `"$l2ApO`" `"$appWindowO`" `"$appWin32O`" $($L2Rt.ObjList) -o `"$l2Exe`" $LinkLibs" $l2olLog1 $l2olLog2
            if ($l2olExit -ne 0) {
                Get-Content $l2olLog2
                $Verdict = "UNEXPECTED_FAILURE"
                $ExitCode = 1
            } else {
                # Ticket 20260913-235500 (Fable): the L2 run was handed a
                # freshly-created, EMPTY directory ($RunDir\l2run) as its
                # fixture root, while the oracle run above was handed
                # $RunDir itself -- the directory the harness's own
                # PowerShell-side fixture preparation (WScript.Shell .lnk
                # creation, the fixture exe) actually populated. That made
                # entry_create fail at the very first real operation
                # (MIXA_APP_ERR_MISSING), with every dependent check
                # following from that one absence -- not a behavioural
                # difference in the generated module, never actually
                # exercised.
                #
                # Simply reusing $RunDir for the L2 run too would create a
                # DIFFERENT bug: the oracle run's own execution creates
                # marker_a.txt/marker_b.txt as a side effect of its real
                # invoke, and populates apppaneldir/apppaneldir2 (which the
                # harness's own main() creates from argv[1] at runtime, not
                # this script) with real .link entries. Handing the L2 run
                # that SAME already-mutated directory would let its own
                # "marker appeared" checks pass on leftover files from the
                # oracle's run even if the L2 binary's own invoke never
                # fired. So the L2 run gets its own PRISTINE, independently
                # built fixture tree instead -- the identical static
                # fixtures (fixture exe, three .lnk shortcuts) built fresh
                # under $L2FixtureRoot, mirroring exactly what the oracle
                # got before its own run touched anything; the harness's
                # own main() builds apppaneldir/apppaneldir2/emptydir
                # itself from this root, same as it does for the oracle.
                $L2FixtureRoot = Join-Path $RunDir "l2run"
                $L2FixtureDir = Join-Path $L2FixtureRoot "fixtures"
                New-Item -ItemType Directory -Force -Path $L2FixtureDir | Out-Null
                $L2FixtureExe = Join-Path $L2FixtureDir "mixa_app_fixture_invoke.exe"
                Copy-Item -LiteralPath $FixtureExe -Destination $L2FixtureExe -Force

                $L2LnkA = Join-Path $L2FixtureDir "ab.lnk"
                $L2MarkerA = Join-Path $L2FixtureDir "marker_a.txt"
                $L2ScA = $Wsh.CreateShortcut($L2LnkA)
                $L2ScA.TargetPath = $L2FixtureExe
                $L2ScA.Arguments = "-marker `"$L2MarkerA`""
                $L2ScA.WorkingDirectory = $L2FixtureDir
                $L2ScA.Save()

                $L2LnkB = Join-Path $L2FixtureDir "cd.lnk"
                $L2MarkerB = Join-Path $L2FixtureDir "marker_b.txt"
                $L2ScB = $Wsh.CreateShortcut($L2LnkB)
                $L2ScB.TargetPath = $L2FixtureExe
                $L2ScB.Arguments = "-marker `"$L2MarkerB`""
                $L2ScB.WorkingDirectory = $L2FixtureDir
                $L2ScB.Save()

                $L2LnkGhost = Join-Path $L2FixtureDir "ghost.lnk"
                $L2ScGhost = $Wsh.CreateShortcut($L2LnkGhost)
                $L2ScGhost.TargetPath = Join-Path $L2FixtureDir "no_such_target_xyz.exe"
                $L2ScGhost.WorkingDirectory = $L2FixtureDir
                $L2ScGhost.Save()

                Push-Location $RepoRoot
                $l2RunOut = Join-Path $RunDir "l2_run_stdout.log"
                $l2RunErr = Join-Path $RunDir "l2_run_stderr.log"
                $l2RunExit = Invoke-Cmd "`"$l2Exe`"" "`"$L2FixtureRoot`"" $l2RunOut $l2RunErr
                Pop-Location
                $L2TraceText = Get-Content -LiteralPath $l2RunOut -Raw
                $Norm = Get-NormalizedParityTraces -OracleTrace $OracleTrace -OracleRoot $RunDir -L2Trace $L2TraceText -L2Root $L2FixtureRoot
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
AppPanel-L2-Header: mixa_manager\mixa_app_panel_l2.h.lm1
AppPanel-L2-Source: $apSrc
AppPanel-L2-Source-Sha256: $ApSourceHash
Harness-Source-Sha256: $((Get-FileHash -LiteralPath (Join-Path $RepoRoot "mixa_manager\tests\mixa_app_panel_parity_harness.lm1") -Algorithm SHA256).Hash)
Runner-Sha256: $RunnerHash
Oracle-Run-Exit-Code: $oracleRunExit
Oracle-Trace:
$OracleTrace
AppPanel-Translate-Exit-Code: $ApExit
AppPanel-Translate-Stdout:
$ApStdoutText
AppPanel-Translate-Stderr:
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
        "EXPECTED_CORE_BARRIER: full mixa_app_panel.lm2 translation stopped at an already-known barrier. This is NOT a pass -- the integrated frontend is not yet on main. The oracle-side harness DID run (see Oracle-Trace above)."
    }
    "UNEXPECTED_FAILURE" {
        "UNEXPECTED_FAILURE: translation or build failed with something OTHER than the already-known barriers. Inspect the logs under $RunDir."
    }
    "PARITY_FAILURE" {
        "PARITY_FAILURE: both implementations built and ran, but their traces differ (see Diff above) or one exited non-zero."
    }
    "PASS" {
        "PASS: full mixa_app_panel.lm2 translated, built, and ran; its trace is byte-identical to the L1 oracle's own trace."
    }
}

exit $ExitCode
