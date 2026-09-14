# Real L1-vs-L2 parity gate for mixa_backend_win32 (share-and-backends
# ticket 20260913-192700, resumed after the runtime-headers detour). No
# dedicated ABI probe pair, and no L1 header unit to predef at all:
# mixa_backend_win32.h has no .h.lm1 counterpart anywhere in this
# codebase -- real, plain, hand-written C. MixaWin32's own fields are
# dereferenced via `\` with no L1-level struct: declaration, proven
# safe by the established "`\` defers to gcc" finding.
#
# Uses the explicit "win32-hidden" table (ticket 20260913-015530) -- a
# real HWND/DC/message loop opened with SW_HIDE, the same already-
# accepted, already-automated (run_mixa.ps1's own regression sweep)
# unattended route the existing mixa_backend_win32_selftest.lm1 already
# uses. This gate touches the real system clipboard (the same already-
# accepted behavior that existing selftest already exercises) and
# never opens a visible window.
#
#   1. ALWAYS builds and runs the ORACLE-side harness (open() argument/
#      metric validation, a real open with font-native even metrics,
#      a real present()/frame_at() pixel round trip, real PostMessage-
#      driven input drained through poll() in real order, a real
#      clipboard round trip, real glyph rasterization with cache
#      identity, the three injectable GDI-error fault paths, and
#      close()/null-argument safety).
#   2. Attempts to translate the COMPLETE mixa_backend_win32.lm2.
#      - fails with an already-known barrier -> EXPECTED_CORE_BARRIER
#        (exit 2). NOT a pass.
#      - fails with any OTHER diagnostic -> UNEXPECTED_FAILURE (exit 1).
#      - SUCCEEDS -> builds/links/runs the L2-side harness and diffs its
#        stdout against the oracle trace byte-for-byte. PASS (exit 0)
#        only on an exact match, else PARITY_FAILURE (exit 1).
#
# mixa_backend_win32.h/.lm1 are the parity oracle and are never
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
$GccLibs = "-lgdi32 -luser32 -lkernel32"

$RunTimestamp = (Get-Date -Format "yyyyMMdd_HHmmss_fff")
$RunGuid = [GUID]::NewGuid().ToString().Substring(0, 8)
$BaseDir = Join-Path $RepoRoot "build\mixa\claude\mixa_backend_win32_l2_parity"
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

# ---- Step 0: translate the L2 header (mixa_backend_win32.h is plain
# C, no other real headers to translate). ----
Invoke-HeaderTrans "mixa_manager\mixa_backend_win32_l2.h.lm1" "mixa_backend_win32_l2.lm1.h"

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
$hExit = Invoke-Cmd $L1Trans "mixa_manager\tests\mixa_backend_win32_parity_harness.lm1 `"$harnessC`"" $hLog1 $hLog2
Pop-Location
if ($hExit -ne 0) { Get-Content $hLog2; throw "harness translation failed" }

$harnessO = Join-Path $RunDir "harness.o"
$hcLog1 = Join-Path $RunDir "harness_compile_stdout.log"
$hcLog2 = Join-Path $RunDir "harness_compile_stderr.log"
$hcExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" -c `"$harnessC`" -o `"$harnessO`"" $hcLog1 $hcLog2
if ($hcExit -ne 0) { Get-Content $hcLog2; throw "harness compile failed" }

# ---- Step 3: build the shared dependency set once (used by BOTH
# oracle and L2 sides -- mixa_backend_win32.lm1 predefs nothing). ----
$eventFifoO = Build-RealDep "mixa_event_fifo" "mixa_manager\mixa_event_fifo.lm1"
$backendTableO = Build-RealDep "mixa_backend_table" "mixa_manager\mixa_backend_table.lm1"
$backendCtorsO = Build-RealDep "mixa_backend_ctors_win32" "mixa_manager\mixa_backend_ctors_win32.lm1"
$backendHeadlessO = Build-RealDep "mixa_backend_headless" "mixa_manager\mixa_backend_headless.lm1"

# ---- Step 4: ALWAYS build + run the ORACLE-side harness. ----
$oracleC = Join-Path $RunDir "mixa_backend_win32_oracle.c"
$ocLog1 = Join-Path $RunDir "oracle_trans_stdout.log"
$ocLog2 = Join-Path $RunDir "oracle_trans_stderr.log"
Push-Location $RepoRoot
$ocExit = Invoke-Cmd $L1Trans "mixa_manager\mixa_backend_win32.lm1 `"$oracleC`"" $ocLog1 $ocLog2
Pop-Location
if ($ocExit -ne 0) { Get-Content $ocLog2; throw "oracle mixa_backend_win32.lm1 translation failed" }

$oracleO = Join-Path $RunDir "mixa_backend_win32_oracle.o"
$occLog1 = Join-Path $RunDir "oracle_compile_stdout.log"
$occLog2 = Join-Path $RunDir "oracle_compile_stderr.log"
$occExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" -c `"$oracleC`" -o `"$oracleO`"" $occLog1 $occLog2
if ($occExit -ne 0) { Get-Content $occLog2; throw "oracle mixa_backend_win32.lm1 compile failed" }

$oracleExe = Join-Path $RunDir "parity_oracle.exe"
$olLog1 = Join-Path $RunDir "oracle_link_stdout.log"
$olLog2 = Join-Path $RunDir "oracle_link_stderr.log"
$olExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" `"$harnessO`" `"$oracleO`" `"$eventFifoO`" `"$backendTableO`" `"$backendCtorsO`" `"$backendHeadlessO`" -o `"$oracleExe`" $GccLibs" $olLog1 $olLog2
if ($olExit -ne 0) { Get-Content $olLog2; throw "oracle harness link failed" }

Push-Location $RepoRoot
$oracleRunOut = Join-Path $RunDir "oracle_run_stdout.log"
$oracleRunErr = Join-Path $RunDir "oracle_run_stderr.log"
$oracleRunExit = Invoke-Cmd "`"$oracleExe`"" "" $oracleRunOut $oracleRunErr
Pop-Location
$OracleTrace = Get-Content -LiteralPath $oracleRunOut -Raw

# ---- Step 5: attempt the COMPLETE mixa_backend_win32.lm2 translation. ----
Push-Location $RepoRoot
$bwSrc = "mixa_manager\mixa_backend_win32.lm2"
$bwOut = Join-Path $RunDir "mixa_backend_win32_l2.lm1"
$bwStdout = Join-Path $RunDir "bw_stdout.log"
$bwStderr = Join-Path $RunDir "bw_stderr.log"
$BwExit = Invoke-Cmd "`"$l2exe`"" "`"$bwSrc`" `"$bwOut`"" $bwStdout $bwStderr
Pop-Location

$BwStdoutText = if (Test-Path -LiteralPath $bwStdout) { Get-Content -LiteralPath $bwStdout -Raw } else { "" }
$BwStderrText = if (Test-Path -LiteralPath $bwStderr) { Get-Content -LiteralPath $bwStderr -Raw } else { "" }
$BwSourceHash = (Get-FileHash -LiteralPath (Join-Path $RepoRoot $bwSrc) -Algorithm SHA256).Hash
$RunnerHash = (Get-FileHash -LiteralPath $PSCommandPath -Algorithm SHA256).Hash

$Verdict = $null
$ExitCode = 1
$L2TraceText = ""
$DiffText = ""

$KnownBarrier = ($BwStderrText -match "unknown foreign type") -or ($BwStderrText -match "incompatible entry signature") -or ($BwStderrText -match "unsupported own array declaration")

if ($BwExit -ne 0 -and $KnownBarrier) {
    $Verdict = "EXPECTED_CORE_BARRIER"
    $ExitCode = 2
} elseif ($BwExit -ne 0) {
    $Verdict = "UNEXPECTED_FAILURE"
    $ExitCode = 1
} else {
    Push-Location $RepoRoot
    $l2BwC = Join-Path $RunDir "mixa_backend_win32_l2.c"
    $l2ccLog1 = Join-Path $RunDir "l2bw_trans_stdout.log"
    $l2ccLog2 = Join-Path $RunDir "l2bw_trans_stderr.log"
    $l2ccExit = Invoke-Cmd $L1Trans "`"$bwOut`" `"$l2BwC`"" $l2ccLog1 $l2ccLog2
    Pop-Location
    if ($l2ccExit -ne 0) {
        Get-Content $l2ccLog2
        $Verdict = "UNEXPECTED_FAILURE"
        $ExitCode = 1
    } else {
        # Generated L2 code spells its runtime `#include`s using the same
        # root-relative path L2_RUNTIME_ROOT gave the predef (Fable's own
        # run_graph_abi.ps1 shape, ticket 20260913-200800).
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
        $l2BwO = Join-Path $RunDir "mixa_backend_win32_l2.o"
        $l2occLog1 = Join-Path $RunDir "l2bw_compile_stdout.log"
        $l2occLog2 = Join-Path $RunDir "l2bw_compile_stderr.log"
        $l2occExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" -I `"$L2RuntimeHeaderRoot`" -c `"$l2BwC`" -o `"$l2BwO`"" $l2occLog1 $l2occLog2
        if ($l2occExit -ne 0) {
            Get-Content $l2occLog2
            $Verdict = "UNEXPECTED_FAILURE"
            $ExitCode = 1
        } else {
            $l2Exe = Join-Path $RunDir "parity_l2.exe"
            $l2olLog1 = Join-Path $RunDir "l2bw_link_stdout.log"
            $l2olLog2 = Join-Path $RunDir "l2bw_link_stderr.log"
            $l2olExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" `"$harnessO`" `"$l2BwO`" `"$eventFifoO`" `"$backendTableO`" `"$backendCtorsO`" `"$backendHeadlessO`" -o `"$l2Exe`" $GccLibs" $l2olLog1 $l2olLog2
            if ($l2olExit -ne 0) {
                Get-Content $l2olLog2
                $Verdict = "UNEXPECTED_FAILURE"
                $ExitCode = 1
            } else {
                Push-Location $RepoRoot
                $l2RunOut = Join-Path $RunDir "l2_run_stdout.log"
                $l2RunErr = Join-Path $RunDir "l2_run_stderr.log"
                $l2RunExit = Invoke-Cmd "`"$l2Exe`"" "" $l2RunOut $l2RunErr
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
BackendWin32-L2-Header: mixa_manager\mixa_backend_win32_l2.h.lm1
BackendWin32-L2-Source: $bwSrc
BackendWin32-L2-Source-Sha256: $BwSourceHash
Harness-Source-Sha256: $((Get-FileHash -LiteralPath (Join-Path $RepoRoot "mixa_manager\tests\mixa_backend_win32_parity_harness.lm1") -Algorithm SHA256).Hash)
Runner-Sha256: $RunnerHash
Oracle-Run-Exit-Code: $oracleRunExit
Oracle-Trace:
$OracleTrace
BackendWin32-Translate-Exit-Code: $BwExit
BackendWin32-Translate-Stdout:
$BwStdoutText
BackendWin32-Translate-Stderr:
$BwStderrText
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
        "EXPECTED_CORE_BARRIER: full mixa_backend_win32.lm2 translation stopped at an already-known barrier. This is NOT a pass -- the integrated frontend is not yet on main. The oracle-side harness DID run (see Oracle-Trace above)."
    }
    "UNEXPECTED_FAILURE" {
        "UNEXPECTED_FAILURE: translation or build failed with something OTHER than the already-known barriers. Inspect the logs under $RunDir."
    }
    "PARITY_FAILURE" {
        "PARITY_FAILURE: both implementations built and ran, but their traces differ (see Diff above) or one exited non-zero."
    }
    "PASS" {
        "PASS: full mixa_backend_win32.lm2 translated, built, and ran; its trace is byte-identical to the L1 oracle's own trace."
    }
}

exit $ExitCode
