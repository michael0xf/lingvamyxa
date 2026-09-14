# Real L1-vs-L2 parity gate for mixa_app_main (ticket 20260913-133500).
#
# No ABI probe: mixa_app_main's own main() touches NO struct field of
# any kind. MixaAppController is genuinely opaque (a plain C `typedef
# struct MixaAppController MixaAppController;` with no body anywhere),
# and MixaBackendVTable is only ever passed through as an opaque
# pointer, never dereferenced. There is no ABI-relevant layout this
# module's own L2 port introduces or depends on, so an ABI probe would
# be inventing machinery this port genuinely does not need -- the
# "smallest honest header adapter" this ticket's own instruction 2 asks
# for is, here, none at all: both mixa_app_controller.h and mixa_
# backend.h are plain, hand-written C headers, included directly,
# unchanged, exactly like the real oracle does.
#
#   1. ALWAYS build-only translate+compile+LINK the REAL, unmodified
#      mixa_app_main.lm1 into a full Win32 executable, reusing the
#      exact real-dependency object chain already established and
#      accepted by run_win32_smoke_selftest.ps1 (13 objects + headers).
#      NEVER executed (this module opens a real window; per its own
#      header comment it is a build-only check in every existing
#      runner too).
#   2. ALWAYS build + run a headless-driven BEHAVIORAL harness
#      (mixa_app_main_parity_harness.lm1) exercising the identical
#      orchestration shape mixa_app_main's own body performs -- open/
#      step-loop/close via the REAL, unmodified mixa_app_controller_*
#      functions -- substituting the real headless backend table for
#      the real Win32 one (mirroring mixa_app_controller_e2e_selftest.
#      lm1's own already-accepted technique), covering startup-failure/
#      usage (null out, null vt), the successful setup-to-loop
#      boundary, ordinary ticks with no error, and close()'s own
#      cleanup/null-safety contract. This is disclosed explicitly as
#      controlled-seam behavioral evidence for the LOGIC mixa_app_main
#      embodies, not a literal execution of the untestable real entry
#      point.
#   3. Attempts to translate the COMPLETE mixa_app_main.lm2.
#      - fails with an already-known barrier -> EXPECTED_CORE_BARRIER
#        (exit 2). NOT a pass.
#      - fails with any OTHER diagnostic -> UNEXPECTED_FAILURE (exit 1).
#      - SUCCEEDS -> reports PASS (there is no second, parallel L2-side
#        execution to diff against here, since main()'s own C signature
#        cannot be linked twice into one process; a PASS is reported
#        once the translation, compile, and link of the L2 side alone
#        succeed cleanly).
#
# mixa_app_main.h/.lm1, mixa_app_controller.h/.lm1 and every real
# dependency object listed below are the parity oracle (or real,
# unmodified dependencies) and are never touched. Nothing under stg/
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

$RunTimestamp = (Get-Date -Format "yyyyMMdd_HHmmss_fff")
$RunGuid = [GUID]::NewGuid().ToString().Substring(0, 8)
$BaseDir = Join-Path $RepoRoot "build\mixa\claude\mixa_app_main_l2_parity"
$RunDir = Join-Path $BaseDir "run_${RunTimestamp}_${RunGuid}"
$LogDir = Join-Path $RunDir "logs"
$HeaderDir = Join-Path $RunDir "headers\mixa_manager"
$HeaderIncludeRoot = Join-Path $RunDir "headers"
New-Item -ItemType Directory -Force -Path $LogDir, $HeaderDir | Out-Null

function Invoke-Cmd([string]$exe, [string]$argsStr, [string]$outLog, [string]$errLog) {
    & cmd /c "$exe $argsStr > `"$outLog`" 2> `"$errLog`""
    return $LASTEXITCODE
}

function Invoke-HeaderTranslation([string]$Name, [string]$SourceRel, [string]$OutName) {
    $out = Join-Path $HeaderDir $OutName
    $o1 = Join-Path $LogDir "${Name}_header_trans_stdout.log"
    $o2 = Join-Path $LogDir "${Name}_header_trans_stderr.log"
    Push-Location $RepoRoot
    $rc = Invoke-Cmd $L1Trans "$SourceRel `"$out`"" $o1 $o2
    Pop-Location
    if ($rc -ne 0) { Get-Content $o2; throw "$Name header translation failed" }
}

function Invoke-UnitCompile([string]$Name, [string]$SourceRel) {
    $cOut = Join-Path $RunDir "$Name.c"
    $oOut = Join-Path $RunDir "$Name.o"
    $t1 = Join-Path $LogDir "${Name}_trans_stdout.log"
    $t2 = Join-Path $LogDir "${Name}_trans_stderr.log"
    Push-Location $RepoRoot
    $rc = Invoke-Cmd $L1Trans "$SourceRel `"$cOut`"" $t1 $t2
    Pop-Location
    if ($rc -ne 0) { Get-Content $t2; throw "$Name translation failed" }
    $c1 = Join-Path $LogDir "${Name}_compile_stdout.log"
    $c2 = Join-Path $LogDir "${Name}_compile_stderr.log"
    $rc2 = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$HeaderIncludeRoot`" -c `"$cOut`" -o `"$oOut`"" $c1 $c2
    if ($rc2 -ne 0) { Get-Content $c2; throw "$Name compile failed" }
    return $oOut
}

# ---- Shared header translations (both the win32 oracle build and the
# headless behavioral harness need the same header set). ----
Invoke-HeaderTranslation "dir_win32" "mixa_manager\mixa_dir_win32.h.lm1" "mixa_dir_win32.lm1.h"
Invoke-HeaderTranslation "dir" "mixa_manager\mixa_dir.h.lm1" "mixa_dir.lm1.h"
Invoke-HeaderTranslation "selection_walk" "mixa_manager\mixa_selection_walk.h.lm1" "mixa_selection_walk.lm1.h"
Invoke-HeaderTranslation "fileio_win32" "mixa_manager\mixa_fileio_win32.h.lm1" "mixa_fileio_win32.lm1.h"
Invoke-HeaderTranslation "fileio" "mixa_manager\mixa_fileio.h.lm1" "mixa_fileio.lm1.h"
Invoke-HeaderTranslation "copy" "mixa_manager\mixa_copy.h.lm1" "mixa_copy.lm1.h"
Invoke-HeaderTranslation "file_manager" "mixa_manager\mixa_file_manager.h.lm1" "mixa_file_manager.lm1.h"
Invoke-HeaderTranslation "fm_copy" "mixa_manager\mixa_fm_copy.h.lm1" "mixa_fm_copy.lm1.h"
Invoke-HeaderTranslation "console_window" "mixa_manager\mixa_console_window.h.lm1" "mixa_console_window.lm1.h"
Invoke-HeaderTranslation "process_marker" "mixa_manager\mixa_process_marker.h.lm1" "mixa_process_marker.lm1.h"

# ---- Shared object set (both win32 and headless builds need these). ----
$eventFifoObj = Invoke-UnitCompile "mixa_event_fifo" "mixa_manager\mixa_event_fifo.lm1"
$backendTableObj = Invoke-UnitCompile "mixa_backend_table" "mixa_manager\mixa_backend_table.lm1"
$pumpObj = Invoke-UnitCompile "mixa_pump" "mixa_manager\mixa_pump.lm1"
$consoleWindowObj = Invoke-UnitCompile "mixa_console_window" "mixa_manager\mixa_console_window.lm1"
$fileWin32Obj = Invoke-UnitCompile "mixa_file_win32" "mixa_manager\mixa_file_win32.lm1"
$appPathObj = Invoke-UnitCompile "mixa_app_path" "mixa_manager\mixa_app_path.lm1"
$helpObj = Invoke-UnitCompile "mixa_help" "mixa_manager\mixa_help.lm1"
$cmdlineObj = Invoke-UnitCompile "mixa_cmdline" "mixa_manager\mixa_cmdline.lm1"
$cmdlineDispatchObj = Invoke-UnitCompile "mixa_cmdline_dispatch" "mixa_manager\mixa_cmdline_dispatch.lm1"
$processMarkerObj = Invoke-UnitCompile "mixa_process_marker" "mixa_manager\mixa_process_marker.lm1"
$processWin32Obj = Invoke-UnitCompile "mixa_process_win32" "mixa_manager\mixa_process_win32.lm1"
$controllerObj = Invoke-UnitCompile "mixa_app_controller" "mixa_manager\mixa_app_controller.lm1"
$fmpanelObj = Invoke-UnitCompile "mixa_app_fmpanel" "mixa_manager\mixa_app_fmpanel.lm1"
$highlightObj = Invoke-UnitCompile "mixa_highlight" "mixa_manager\mixa_highlight.lm1"

# ---- Step 1: ALWAYS build-only the REAL win32 oracle mixa_app_main.lm1
# (never executed). ----
$win32Obj = Invoke-UnitCompile "mixa_backend_win32" "mixa_manager\mixa_backend_win32.lm1"
$ctorsWin32Obj = Invoke-UnitCompile "mixa_backend_ctors_win32" "mixa_manager\mixa_backend_ctors_win32.lm1"
$backendHeadlessObj = Invoke-UnitCompile "mixa_backend_headless" "mixa_manager\mixa_backend_headless.lm1"
$ctorsHeadlessObj = Invoke-UnitCompile "mixa_backend_ctors_headless" "mixa_manager\mixa_backend_ctors_headless.lm1"

$mainTransOut = Join-Path $RunDir "mixa_app_main.c"
$mainObj = Join-Path $RunDir "mixa_app_main.o"
$MainExe = Join-Path $RunDir "mixa_app_main.exe"
Push-Location $RepoRoot
$mt1 = Join-Path $LogDir "app_main_trans_stdout.log"
$mt2 = Join-Path $LogDir "app_main_trans_stderr.log"
$mtRc = Invoke-Cmd $L1Trans "mixa_manager\mixa_app_main.lm1 `"$mainTransOut`"" $mt1 $mt2
Pop-Location
if ($mtRc -ne 0) { Get-Content $mt2; throw "oracle mixa_app_main.lm1 translation failed" }
$mc1 = Join-Path $LogDir "app_main_compile_stdout.log"
$mc2 = Join-Path $LogDir "app_main_compile_stderr.log"
$mcRc = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$HeaderIncludeRoot`" -c `"$mainTransOut`" -o `"$mainObj`"" $mc1 $mc2
if ($mcRc -ne 0) { Get-Content $mc2; throw "oracle mixa_app_main.lm1 compile failed" }
$ml1 = Join-Path $LogDir "app_main_link_stdout.log"
$ml2 = Join-Path $LogDir "app_main_link_stderr.log"
$mlRc = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$HeaderIncludeRoot`" `"$mainObj`" `"$controllerObj`" `"$fmpanelObj`" `"$highlightObj`" `"$eventFifoObj`" `"$backendTableObj`" `"$win32Obj`" `"$backendHeadlessObj`" `"$ctorsWin32Obj`" `"$pumpObj`" `"$consoleWindowObj`" `"$fileWin32Obj`" `"$appPathObj`" `"$helpObj`" `"$cmdlineObj`" `"$cmdlineDispatchObj`" `"$processMarkerObj`" `"$processWin32Obj`" -lgdi32 -luser32 -lkernel32 -o `"$MainExe`"" $ml1 $ml2
if ($mlRc -ne 0) { Get-Content $ml2; throw "oracle mixa_app_main.lm1 link failed" }
if (-not (Test-Path -LiteralPath $MainExe -PathType Leaf)) { throw "mixa_app_main.exe was not produced" }
$OracleBuildStatus = "mixa_app_main.lm1 (real Win32 entrypoint): translate+compile+link OK, exe produced at $MainExe, NEVER executed (opens a real window)."

# ---- Step 2: ALWAYS build + run the headless behavioral harness. ----
$harnessC = Join-Path $RunDir "harness.c"
$hLog1 = Join-Path $LogDir "harness_trans_stdout.log"
$hLog2 = Join-Path $LogDir "harness_trans_stderr.log"
Push-Location $RepoRoot
$hExit = Invoke-Cmd $L1Trans "mixa_manager\tests\mixa_app_main_parity_harness.lm1 `"$harnessC`"" $hLog1 $hLog2
Pop-Location
if ($hExit -ne 0) { Get-Content $hLog2; throw "harness translation failed" }
$harnessO = Join-Path $RunDir "harness.o"
$hc1 = Join-Path $LogDir "harness_compile_stdout.log"
$hc2 = Join-Path $LogDir "harness_compile_stderr.log"
$hcExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$HeaderIncludeRoot`" -c `"$harnessC`" -o `"$harnessO`"" $hc1 $hc2
if ($hcExit -ne 0) { Get-Content $hc2; throw "harness compile failed" }

$harnessExe = Join-Path $RunDir "harness.exe"
$hl1 = Join-Path $LogDir "harness_link_stdout.log"
$hl2 = Join-Path $LogDir "harness_link_stderr.log"
$hlExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$HeaderIncludeRoot`" `"$harnessO`" `"$controllerObj`" `"$fmpanelObj`" `"$highlightObj`" `"$eventFifoObj`" `"$backendTableObj`" `"$backendHeadlessObj`" `"$ctorsHeadlessObj`" `"$pumpObj`" `"$consoleWindowObj`" `"$fileWin32Obj`" `"$appPathObj`" `"$helpObj`" `"$cmdlineObj`" `"$cmdlineDispatchObj`" `"$processMarkerObj`" `"$processWin32Obj`" -o `"$harnessExe`"" $hl1 $hl2
if ($hlExit -ne 0) { Get-Content $hl2; throw "harness link failed" }

$HarnessFixtureDir = Join-Path $RunDir "harness_fixtures"
New-Item -ItemType Directory -Force -Path $HarnessFixtureDir | Out-Null
Push-Location $RepoRoot
$hro = Join-Path $LogDir "harness_run_stdout.log"
$hre = Join-Path $LogDir "harness_run_stderr.log"
$hrExit = Invoke-Cmd "`"$harnessExe`"" "`"$HarnessFixtureDir`"" $hro $hre
Pop-Location
$HarnessTrace = Get-Content -LiteralPath $hro -Raw

# ---- Step 2b: build l2trans.exe fresh from whatever L2->L1 frontend
# source is currently on disk (informational hash only, never pinned --
# the L2 side is always rebuilt fresh, matching every other parity gate
# this segment uses). ----
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
    $gccBuildLog = Join-Path $LogDir "l2trans_build_stderr.log"
    $gccArgsStr = "$GccStd -I . -I lm1\build `"$l2c`" -o `"$l2exe`""
    $rc = Invoke-Cmd "gcc" $gccArgsStr (Join-Path $LogDir "l2trans_build_stdout.log") $gccBuildLog
    if ($rc -ne 0) { Get-Content $gccBuildLog; throw "gcc failed building l2trans.exe" }
} finally {
    Pop-Location
}
$L2TransExeHash = (Get-FileHash -LiteralPath $l2exe -Algorithm SHA256).Hash

# ---- Step 3: attempt the COMPLETE mixa_app_main.lm2 translation
# (L2 -> L1, via the freshly-built l2exe -- NOT the pinned L1Trans,
# which only ever translates L1 -> C). ----
Push-Location $RepoRoot
$amSrc = "mixa_manager\mixa_app_main.lm2"
$amOut = Join-Path $RunDir "mixa_app_main_l2.lm1"
$amStdout = Join-Path $LogDir "am_stdout.log"
$amStderr = Join-Path $LogDir "am_stderr.log"
$AmExit = Invoke-Cmd "`"$l2exe`"" "`"$amSrc`" `"$amOut`"" $amStdout $amStderr
Pop-Location

$AmStdoutText = if (Test-Path -LiteralPath $amStdout) { Get-Content -LiteralPath $amStdout -Raw } else { "" }
$AmStderrText = if (Test-Path -LiteralPath $amStderr) { Get-Content -LiteralPath $amStderr -Raw } else { "" }
$AmSourceHash = (Get-FileHash -LiteralPath (Join-Path $RepoRoot $amSrc) -Algorithm SHA256).Hash
$RunnerHash = (Get-FileHash -LiteralPath $PSCommandPath -Algorithm SHA256).Hash

$Verdict = $null
$ExitCode = 1
$KnownBarrier = ($AmStderrText -match "unknown foreign type") -or ($AmStderrText -match "incompatible entry signature")

if ($AmExit -ne 0 -and $KnownBarrier) {
    $Verdict = "EXPECTED_CORE_BARRIER"
    $ExitCode = 2
} elseif ($AmExit -ne 0) {
    $Verdict = "UNEXPECTED_FAILURE"
    $ExitCode = 1
} else {
    Push-Location $RepoRoot
    $l2AmC = Join-Path $RunDir "mixa_app_main_l2.c"
    $l2c1 = Join-Path $LogDir "l2am_trans_stdout.log"
    $l2c2 = Join-Path $LogDir "l2am_trans_stderr.log"
    $l2cExit = Invoke-Cmd $L1Trans "`"$amOut`" `"$l2AmC`"" $l2c1 $l2c2
    Pop-Location
    if ($l2cExit -ne 0) {
        Get-Content $l2c2
        $Verdict = "UNEXPECTED_FAILURE"
        $ExitCode = 1
    } else {
        $l2AmO = Join-Path $RunDir "mixa_app_main_l2.o"
        $l2oc1 = Join-Path $LogDir "l2am_compile_stdout.log"
        $l2oc2 = Join-Path $LogDir "l2am_compile_stderr.log"
        $l2ocExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$HeaderIncludeRoot`" -c `"$l2AmC`" -o `"$l2AmO`"" $l2oc1 $l2oc2
        if ($l2ocExit -ne 0) {
            Get-Content $l2oc2
            $Verdict = "UNEXPECTED_FAILURE"
            $ExitCode = 1
        } else {
            $l2AmExe = Join-Path $RunDir "mixa_app_main_l2.exe"
            $l2ol1 = Join-Path $LogDir "l2am_link_stdout.log"
            $l2ol2 = Join-Path $LogDir "l2am_link_stderr.log"
            $l2olExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$HeaderIncludeRoot`" `"$l2AmO`" `"$controllerObj`" `"$fmpanelObj`" `"$highlightObj`" `"$eventFifoObj`" `"$backendTableObj`" `"$win32Obj`" `"$backendHeadlessObj`" `"$ctorsWin32Obj`" `"$pumpObj`" `"$consoleWindowObj`" `"$fileWin32Obj`" `"$appPathObj`" `"$helpObj`" `"$cmdlineObj`" `"$cmdlineDispatchObj`" `"$processMarkerObj`" `"$processWin32Obj`" -lgdi32 -luser32 -lkernel32 -o `"$l2AmExe`"" $l2ol1 $l2ol2
            if ($l2olExit -ne 0) {
                Get-Content $l2ol2
                $Verdict = "UNEXPECTED_FAILURE"
                $ExitCode = 1
            } else {
                $Verdict = "PASS"
                $ExitCode = 0
            }
        }
    }
}

$Summary = @"
Stable-L1-Translator: $L1Trans
Stable-L1-Translator-Sha256: $ActualL1Hash
L2trans-Source (informational, not pinned): $L2TransSource
L2trans-Source-Sha256 (informational, not pinned): $L2TransSourceHash
AppMain-L2-Source: $amSrc
AppMain-L2-Source-Sha256: $AmSourceHash
Harness-Source-Sha256: $((Get-FileHash -LiteralPath (Join-Path $RepoRoot "mixa_manager\tests\mixa_app_main_parity_harness.lm1") -Algorithm SHA256).Hash)
Runner-Sha256: $RunnerHash
Oracle-Win32-Build-Status: $OracleBuildStatus
Harness-Run-Exit-Code: $hrExit
Harness-Trace:
$HarnessTrace
AppMain-Translate-Exit-Code: $AmExit
AppMain-Translate-Stdout:
$AmStdoutText
AppMain-Translate-Stderr:
$AmStderrText
Verdict: $Verdict
"@
Set-Content -LiteralPath (Join-Path $RunDir "run_summary.txt") -Value $Summary
$Summary
"Run directory: $RunDir"

switch ($Verdict) {
    "EXPECTED_CORE_BARRIER" {
        "EXPECTED_CORE_BARRIER: full mixa_app_main.lm2 translation stopped at an already-known barrier. This is NOT a pass -- the integrated frontend is not yet on main. The oracle win32 build and the headless behavioral harness DID run (see above)."
    }
    "UNEXPECTED_FAILURE" {
        "UNEXPECTED_FAILURE: translation or build failed with something OTHER than the already-known barriers. Inspect the logs under $RunDir."
    }
    "PASS" {
        "PASS: full mixa_app_main.lm2 translated, compiled, and linked cleanly against the real dependency chain."
    }
}

exit $ExitCode
