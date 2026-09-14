# Real Win32 mixa_app_main entrypoint smoke test (ticket 20260913-013609).
#
# run_app_loop_selftest.ps1 already builds/links the real mixa_app_main.exe
# against the real Win32 backend but never executes it (BUILD-ONLY). This
# runner reuses that IDENTICAL build sequence (same units, same headers,
# same stable pinned translator) and then actually launches the resulting
# executable, driving it from a separate, out-of-process native C harness
# (tests/mixa_win32_smoke_harness.c) that opens no window of its own: it
# spawns the real exe, finds its real HWND, and drives it purely through
# PostMessage(WM_CHAR/WM_KEYDOWN/WM_CLOSE) -- the same events a real
# keyboard and window manager would generate -- while observing results
# through the SAME production console file the app itself writes. No
# second orchestration path; the harness contains no app logic.

param()

$ErrorActionPreference = "Stop"

$RepoRoot = Split-Path -Parent $PSScriptRoot
$MixaManagerDir = Join-Path $RepoRoot "mixa_manager"

if (-not (Test-Path $MixaManagerDir)) {
    Write-Error "Repository structure invalid; mixa_manager not found at $MixaManagerDir"
    exit 1
}

if ($env:OS -ne "Windows_NT") {
    Write-Host "win32 smoke selftest skipped (non-Windows profile)"
    exit 0
}

$Stage = "preflight"
$Status = "FAILED"
$Reason = ""
$Compiler = ""
$ActualCompilerHash = ""
. (Join-Path $PSScriptRoot "lib_l2_runtime_support.ps1")
$CompilerHash = Get-L1Pin -L1Root (Join-Path $RepoRoot "stg\l1_baseline")
$HarnessSource = Join-Path $RepoRoot "mixa_manager\tests\mixa_win32_smoke_harness.c"
$HarnessStdout = ""
$HarnessStderr = ""
$HarnessExitFile = ""
$MainExe = ""
$HarnessExe = ""

$RunTimestamp = (Get-Date -Format "yyyyMMdd_HHmmss_fff")
$RunGuid = [GUID]::NewGuid().ToString().Substring(0, 8)
$BaseDir = Join-Path $RepoRoot "build\mixa\claude\win32_smoke"
$RunDir = Join-Path $BaseDir "run_${RunTimestamp}_${RunGuid}"
$LogDir = Join-Path $RunDir "logs"
$FixtureDir = Join-Path $RunDir "fixtures"

function Invoke-HeaderTranslation {
    param(
        [string]$Name,
        [string]$SourceRel,
        [string]$OutName
    )
    $src = Join-Path $RepoRoot $SourceRel
    $out = Join-Path $HeaderDir $OutName
    $stdoutFile = Join-Path $LogDir "${Name}_header_trans_stdout.log"
    $stderrFile = Join-Path $LogDir "${Name}_header_trans_stderr.log"
    $exitFile = Join-Path $LogDir "${Name}_header_trans_exit.txt"
    $proc = Start-Process -FilePath $Compiler -ArgumentList $src, $out -Wait -PassThru -NoNewWindow -WorkingDirectory $RepoRoot -RedirectStandardOutput $stdoutFile -RedirectStandardError $stderrFile
    $rc = $proc.ExitCode
    Set-Content -LiteralPath $exitFile -Value $rc
    if ($rc -ne 0) {
        throw "$Name header translation failed with exit $rc"
    }
}

function Invoke-UnitCompile {
    param(
        [string]$Name,
        [string]$SourceRel,
        [string]$HeaderIncludeRoot
    )
    $src = Join-Path $RepoRoot $SourceRel
    $cOut = Join-Path $RunDir ($Name + ".c")
    $oOut = Join-Path $RunDir ($Name + ".o")
    $transStdout = Join-Path $LogDir ($Name + "_trans_stdout.log")
    $transStderr = Join-Path $LogDir ($Name + "_trans_stderr.log")
    $transExitFile = Join-Path $LogDir ($Name + "_trans_exit.txt")
    $proc = Start-Process -FilePath $Compiler -ArgumentList $src, $cOut -Wait -PassThru -NoNewWindow -WorkingDirectory $RepoRoot -RedirectStandardOutput $transStdout -RedirectStandardError $transStderr
    $rc = $proc.ExitCode
    Set-Content -LiteralPath $transExitFile -Value $rc
    if ($rc -ne 0) {
        throw "$Name translation failed with exit $rc"
    }
    $compileStdout = Join-Path $LogDir ($Name + "_compile_stdout.log")
    $compileStderr = Join-Path $LogDir ($Name + "_compile_stderr.log")
    $compileExitFile = Join-Path $LogDir ($Name + "_compile_exit.txt")
    $gccArgs = @("-std=c99","-Wall","-Wextra","-Wpedantic","-Werror=incompatible-pointer-types","-Werror=discarded-qualifiers","-Werror=implicit-function-declaration","-Werror=implicit-int","-I",".","-I",$HeaderIncludeRoot,"-c",$cOut,"-o",$oOut)
    $gccProc = Start-Process -FilePath "gcc.exe" -ArgumentList $gccArgs -Wait -PassThru -NoNewWindow -WorkingDirectory $RepoRoot -RedirectStandardOutput $compileStdout -RedirectStandardError $compileStderr
    $gccRc = $gccProc.ExitCode
    Set-Content -LiteralPath $compileExitFile -Value $gccRc
    if ($gccRc -ne 0) {
        Get-Content $compileStderr
        throw "$Name compilation failed with exit $gccRc"
    }
    return $oOut
}

try {
    if (-not (New-Item -ItemType Directory -Path $LogDir -Force)) {
        $Reason = "Failed to create run directory: $LogDir"
        throw $Reason
    }
    New-Item -ItemType Directory -Path $FixtureDir -Force | Out-Null

    $Compiler = Join-Path $RepoRoot "stg\l1_baseline\build\l1trans\gen2\l1trans.exe"
    if (-not (Test-Path $Compiler)) {
        $Reason = "Compiler not found: $Compiler"
        throw $Reason
    }
    $ActualCompilerHash = (Get-FileHash -LiteralPath $Compiler -Algorithm SHA256).Hash
    if ($ActualCompilerHash -ne $CompilerHash) {
        $Reason = "Compiler hash mismatch: expected $CompilerHash, got $ActualCompilerHash"
        throw $Reason
    }

    $HeaderDir = Join-Path $RunDir "headers\mixa_manager"
    New-Item -ItemType Directory -Path $HeaderDir -Force | Out-Null
    $HeaderIncludeRoot = Join-Path $RunDir "headers"

    $Stage = "header-translation"
    Invoke-HeaderTranslation -Name "dir_win32" -SourceRel "mixa_manager\mixa_dir_win32.h.lm1" -OutName "mixa_dir_win32.lm1.h"
    Invoke-HeaderTranslation -Name "dir" -SourceRel "mixa_manager\mixa_dir.h.lm1" -OutName "mixa_dir.lm1.h"
    Invoke-HeaderTranslation -Name "selection_walk" -SourceRel "mixa_manager\mixa_selection_walk.h.lm1" -OutName "mixa_selection_walk.lm1.h"
    Invoke-HeaderTranslation -Name "fileio_win32" -SourceRel "mixa_manager\mixa_fileio_win32.h.lm1" -OutName "mixa_fileio_win32.lm1.h"
    Invoke-HeaderTranslation -Name "fileio" -SourceRel "mixa_manager\mixa_fileio.h.lm1" -OutName "mixa_fileio.lm1.h"
    Invoke-HeaderTranslation -Name "copy" -SourceRel "mixa_manager\mixa_copy.h.lm1" -OutName "mixa_copy.lm1.h"
    Invoke-HeaderTranslation -Name "file_manager" -SourceRel "mixa_manager\mixa_file_manager.h.lm1" -OutName "mixa_file_manager.lm1.h"
    Invoke-HeaderTranslation -Name "fm_copy" -SourceRel "mixa_manager\mixa_fm_copy.h.lm1" -OutName "mixa_fm_copy.lm1.h"
    Invoke-HeaderTranslation -Name "console_window" -SourceRel "mixa_manager\mixa_console_window.h.lm1" -OutName "mixa_console_window.lm1.h"
    Invoke-HeaderTranslation -Name "process_marker" -SourceRel "mixa_manager\mixa_process_marker.h.lm1" -OutName "mixa_process_marker.lm1.h"

    $Stage = "unit-compile"
    $eventFifoObj = Invoke-UnitCompile -Name "mixa_event_fifo" -SourceRel "mixa_manager\mixa_event_fifo.lm1" -HeaderIncludeRoot $HeaderIncludeRoot
    $backendTableObj = Invoke-UnitCompile -Name "mixa_backend_table" -SourceRel "mixa_manager\mixa_backend_table.lm1" -HeaderIncludeRoot $HeaderIncludeRoot
    $backendHeadlessObj = Invoke-UnitCompile -Name "mixa_backend_headless" -SourceRel "mixa_manager\mixa_backend_headless.lm1" -HeaderIncludeRoot $HeaderIncludeRoot
    $pumpObj = Invoke-UnitCompile -Name "mixa_pump" -SourceRel "mixa_manager\mixa_pump.lm1" -HeaderIncludeRoot $HeaderIncludeRoot
    $consoleWindowObj = Invoke-UnitCompile -Name "mixa_console_window" -SourceRel "mixa_manager\mixa_console_window.lm1" -HeaderIncludeRoot $HeaderIncludeRoot
    $fileWin32Obj = Invoke-UnitCompile -Name "mixa_file_win32" -SourceRel "mixa_manager\mixa_file_win32.lm1" -HeaderIncludeRoot $HeaderIncludeRoot
    $win32Obj = Invoke-UnitCompile -Name "mixa_backend_win32" -SourceRel "mixa_manager\mixa_backend_win32.lm1" -HeaderIncludeRoot $HeaderIncludeRoot
    $ctorsWin32Obj = Invoke-UnitCompile -Name "mixa_backend_ctors_win32" -SourceRel "mixa_manager\mixa_backend_ctors_win32.lm1" -HeaderIncludeRoot $HeaderIncludeRoot
    $appPathObj = Invoke-UnitCompile -Name "mixa_app_path" -SourceRel "mixa_manager\mixa_app_path.lm1" -HeaderIncludeRoot $HeaderIncludeRoot
    $helpObj = Invoke-UnitCompile -Name "mixa_help" -SourceRel "mixa_manager\mixa_help.lm1" -HeaderIncludeRoot $HeaderIncludeRoot
    $cmdlineObj = Invoke-UnitCompile -Name "mixa_cmdline" -SourceRel "mixa_manager\mixa_cmdline.lm1" -HeaderIncludeRoot $HeaderIncludeRoot
    $cmdlineDispatchObj = Invoke-UnitCompile -Name "mixa_cmdline_dispatch" -SourceRel "mixa_manager\mixa_cmdline_dispatch.lm1" -HeaderIncludeRoot $HeaderIncludeRoot
    $processMarkerObj = Invoke-UnitCompile -Name "mixa_process_marker" -SourceRel "mixa_manager\mixa_process_marker.lm1" -HeaderIncludeRoot $HeaderIncludeRoot
    $processWin32Obj = Invoke-UnitCompile -Name "mixa_process_win32" -SourceRel "mixa_manager\mixa_process_win32.lm1" -HeaderIncludeRoot $HeaderIncludeRoot
    $controllerObj = Invoke-UnitCompile -Name "mixa_app_controller" -SourceRel "mixa_manager\mixa_app_controller.lm1" -HeaderIncludeRoot $HeaderIncludeRoot
    # First visible file-manager panel (ticket 20260913-032000):
    # mixa_app_controller.lm1 now calls into it, so every build of
    # $controllerObj needs it linked too.
    $fmpanelObj = Invoke-UnitCompile -Name "mixa_app_fmpanel" -SourceRel "mixa_manager\mixa_app_fmpanel.lm1" -HeaderIncludeRoot $HeaderIncludeRoot
    $highlightObj = Invoke-UnitCompile -Name "mixa_highlight" -SourceRel "mixa_manager\mixa_highlight.lm1" -HeaderIncludeRoot $HeaderIncludeRoot

    $mainTransOut = Join-Path $RunDir "mixa_app_main.c"
    $mainObj = Join-Path $RunDir "mixa_app_main.o"
    $MainExe = Join-Path $RunDir "mixa_app_main.exe"
    $mainTransStdout = Join-Path $LogDir "app_main_trans_stdout.log"
    $mainTransStderr = Join-Path $LogDir "app_main_trans_stderr.log"
    $mainTransExitFile = Join-Path $LogDir "app_main_trans_exit.txt"
    $Stage = "app-main-translate"
    $mainProc = Start-Process -FilePath $Compiler -ArgumentList "mixa_manager\mixa_app_main.lm1", $mainTransOut -Wait -PassThru -NoNewWindow -WorkingDirectory $RepoRoot -RedirectStandardOutput $mainTransStdout -RedirectStandardError $mainTransStderr
    $mainTransRc = $mainProc.ExitCode
    Set-Content -LiteralPath $mainTransExitFile -Value $mainTransRc
    if ($mainTransRc -ne 0) {
        throw "mixa_app_main translation failed with exit $mainTransRc"
    }

    $mainCompileStdout = Join-Path $LogDir "app_main_compile_stdout.log"
    $mainCompileStderr = Join-Path $LogDir "app_main_compile_stderr.log"
    $mainCompileExitFile = Join-Path $LogDir "app_main_compile_exit.txt"
    $Stage = "app-main-compile"
    $mainGccArgs = @("-std=c99","-Wall","-Wextra","-Wpedantic","-Werror=incompatible-pointer-types","-Werror=discarded-qualifiers","-Werror=implicit-function-declaration","-Werror=implicit-int","-I",".","-I",$HeaderIncludeRoot,"-c",$mainTransOut,"-o",$mainObj)
    $mainGccProc = Start-Process -FilePath "gcc.exe" -ArgumentList $mainGccArgs -Wait -PassThru -NoNewWindow -WorkingDirectory $RepoRoot -RedirectStandardOutput $mainCompileStdout -RedirectStandardError $mainCompileStderr
    $mainCompileRc = $mainGccProc.ExitCode
    Set-Content -LiteralPath $mainCompileExitFile -Value $mainCompileRc
    if ($mainCompileRc -ne 0) {
        Get-Content $mainCompileStderr
        throw "mixa_app_main compilation failed with exit $mainCompileRc"
    }

    $mainLinkStdout = Join-Path $LogDir "app_main_link_stdout.log"
    $mainLinkStderr = Join-Path $LogDir "app_main_link_stderr.log"
    $mainLinkExitFile = Join-Path $LogDir "app_main_link_exit.txt"
    $Stage = "app-main-link"
    $mainLinkArgs = @("-std=c99","-Wall","-Wextra","-Wpedantic","-I",".","-I",$HeaderIncludeRoot,$mainObj,$controllerObj,$fmpanelObj,$highlightObj,$eventFifoObj,$backendTableObj,$win32Obj,$backendHeadlessObj,$ctorsWin32Obj,$pumpObj,$consoleWindowObj,$fileWin32Obj,$appPathObj,$helpObj,$cmdlineObj,$cmdlineDispatchObj,$processMarkerObj,$processWin32Obj,"-lgdi32","-luser32","-lkernel32","-o",$MainExe)
    $mainLinkProc = Start-Process -FilePath "gcc.exe" -ArgumentList $mainLinkArgs -Wait -PassThru -NoNewWindow -WorkingDirectory $RepoRoot -RedirectStandardOutput $mainLinkStdout -RedirectStandardError $mainLinkStderr
    $mainLinkRc = $mainLinkProc.ExitCode
    Set-Content -LiteralPath $mainLinkExitFile -Value $mainLinkRc
    if ($mainLinkRc -ne 0) {
        Get-Content $mainLinkStderr
        throw "mixa_app_main link failed with exit $mainLinkRc"
    }
    if (-not (Test-Path -LiteralPath $MainExe -PathType Leaf)) {
        throw "mixa_app_main.exe was not produced"
    }

    # Build the external, non-L1 driving harness -- plain gcc, no translation.
    $Stage = "harness-compile"
    $HarnessObj = Join-Path $RunDir "mixa_win32_smoke_harness.o"
    $HarnessExe = Join-Path $RunDir "mixa_win32_smoke_harness.exe"
    $harnessCompileStdout = Join-Path $LogDir "harness_compile_stdout.log"
    $harnessCompileStderr = Join-Path $LogDir "harness_compile_stderr.log"
    $harnessCompileExitFile = Join-Path $LogDir "harness_compile_exit.txt"
    $harnessGccArgs = @("-std=c99","-Wall","-Wextra","-Wpedantic","-c",$HarnessSource,"-o",$HarnessObj)
    $harnessGccProc = Start-Process -FilePath "gcc.exe" -ArgumentList $harnessGccArgs -Wait -PassThru -NoNewWindow -WorkingDirectory $RepoRoot -RedirectStandardOutput $harnessCompileStdout -RedirectStandardError $harnessCompileStderr
    $harnessGccRc = $harnessGccProc.ExitCode
    Set-Content -LiteralPath $harnessCompileExitFile -Value $harnessGccRc
    if ($harnessGccRc -ne 0) {
        Get-Content $harnessCompileStderr
        throw "harness compilation failed with exit $harnessGccRc"
    }

    $harnessLinkStdout = Join-Path $LogDir "harness_link_stdout.log"
    $harnessLinkStderr = Join-Path $LogDir "harness_link_stderr.log"
    $harnessLinkExitFile = Join-Path $LogDir "harness_link_exit.txt"
    $Stage = "harness-link"
    $harnessLinkArgs = @("-std=c99","-Wall","-Wextra","-Wpedantic",$HarnessObj,"-luser32","-lkernel32","-o",$HarnessExe)
    $harnessLinkProc = Start-Process -FilePath "gcc.exe" -ArgumentList $harnessLinkArgs -Wait -PassThru -NoNewWindow -WorkingDirectory $RepoRoot -RedirectStandardOutput $harnessLinkStdout -RedirectStandardError $harnessLinkStderr
    $harnessLinkRc = $harnessLinkProc.ExitCode
    Set-Content -LiteralPath $harnessLinkExitFile -Value $harnessLinkRc
    if ($harnessLinkRc -ne 0) {
        Get-Content $harnessLinkStderr
        throw "harness link failed with exit $harnessLinkRc"
    }

    $Stage = "execution"
    $HarnessStdout = Join-Path $LogDir "harness_stdout.log"
    $HarnessStderr = Join-Path $LogDir "harness_stderr.log"
    $HarnessExitFile = Join-Path $LogDir "harness_exit.txt"
    # This run's own scenarios take real, bounded wall-clock time (a few
    # real ping commands totalling well under a minute); the outer
    # timeout below is a TEST timeout only, never a semantic runtime cap
    # inside the harness itself (every internal wait is its own bounded
    # poll on observable state, see mixa_win32_smoke_harness.c).
    $harnessProc = Start-Process -FilePath $HarnessExe -ArgumentList $MainExe, $FixtureDir -Wait -PassThru -NoNewWindow -WorkingDirectory $RepoRoot -RedirectStandardOutput $HarnessStdout -RedirectStandardError $HarnessStderr
    $harnessExitCode = $harnessProc.ExitCode
    Set-Content -LiteralPath $HarnessExitFile -Value $harnessExitCode
    if ($harnessExitCode -ne 0) {
        Get-Content $HarnessStdout
        Get-Content $HarnessStderr
        $Reason = "Harness execution failed with exit $harnessExitCode"
        throw $Reason
    }
    Get-Content $HarnessStdout

    $Stage = "complete"
    $Status = "SUCCESS"

} catch {
    Write-Host "ERROR at stage $Stage : $_"
    $Status = "FAILED"
    $Reason = $_
} finally {
    if (-not (Test-Path -LiteralPath $LogDir -PathType Container)) {
        Write-Error "Run directory creation failed. Original reason: $Reason"
        exit 1
    }

    $CompilerHashFile = Join-Path $LogDir "compiler_hash.txt"
    $RunnerHashFile = Join-Path $LogDir "runner_hash.txt"
    $HarnessSourceHashFile = Join-Path $LogDir "harness_source_hash.txt"
    $HashFiles = @{
        "app_main_impl" = "mixa_manager\mixa_app_main.lm1"
        "app_controller_header" = "mixa_manager\mixa_app_controller.h"
        "app_controller_impl_header" = "mixa_manager\mixa_app_controller_impl.h"
        "app_controller_impl" = "mixa_manager\mixa_app_controller.lm1"
        "backend_win32_header" = "mixa_manager\mixa_backend_win32.h"
        "backend_win32_impl" = "mixa_manager\mixa_backend_win32.lm1"
    }

    try {
        if (Test-Path -LiteralPath $Compiler -PathType Leaf) {
            Set-Content -LiteralPath $CompilerHashFile -Value $ActualCompilerHash
        }
        $HashLines = @()
        foreach ($key in $HashFiles.Keys) {
            $p = Join-Path $RepoRoot $HashFiles[$key]
            $hf = Join-Path $LogDir "${key}_hash.txt"
            if (Test-Path -LiteralPath $p -PathType Leaf) {
                $h = (Get-FileHash -LiteralPath $p -Algorithm SHA256).Hash
                Set-Content -LiteralPath $hf -Value $h
                $HashLines += "$key-Hash-File: $hf"
            }
        }
        if (Test-Path -LiteralPath $HarnessSource -PathType Leaf) {
            Set-Content -LiteralPath $HarnessSourceHashFile -Value ((Get-FileHash -LiteralPath $HarnessSource -Algorithm SHA256).Hash)
        }
        Set-Content -LiteralPath $RunnerHashFile -Value ((Get-FileHash -LiteralPath $PSCommandPath -Algorithm SHA256).Hash)

        $Summary = @"
Status: $Status
Stage: $Stage
Reason: $Reason
Completed-At: $((Get-Date).ToUniversalTime().ToString("o"))
Run-Directory: $RunDir
Fixture-Directory: $FixtureDir
Compiler: $Compiler
Compiler-Hash-File: $CompilerHashFile
$($HashLines -join "`n")
HarnessSource-Hash-File: $HarnessSourceHashFile
Runner-Hash-File: $RunnerHashFile
Main-Exe: $MainExe
Harness-Exe: $HarnessExe
Harness-Stdout: $HarnessStdout
Harness-Stderr: $HarnessStderr
Harness-Exit-File: $HarnessExitFile
"@

        $SummaryFile = Join-Path $LogDir "run_summary.txt"
        Set-Content -LiteralPath $SummaryFile -Value $Summary

        Write-Host ""
        Write-Host "Summary saved to $SummaryFile"
        Write-Host "Run directory: $RunDir"
        Write-Host "Status: $Status"

    } catch {
        Write-Error "Failed to write evidence: $_. Original failure: $Reason"
        $Status = "FAILED"
        exit 1
    }
}

if ($Status -eq "SUCCESS") {
    exit 0
} else {
    exit 1
}
