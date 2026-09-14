# Show the real Copy Here outcome in the production panel (ticket
# 20260913-041656). Drives the SAME production orchestration -- mixa_
# app_controller.h/.lm1, the identical code the real Win32 entrypoint
# uses, now also reporting mixa_fm_copy_here_action's own real status to
# the panel -- through the existing headless backend with a real
# temporary fixture directory. No window opens anywhere in this run;
# this is a real, executed test, not a build-only check. Fixtures live
# under this run's own directory. No user files.

param()

$ErrorActionPreference = "Stop"

$RepoRoot = Split-Path -Parent $PSScriptRoot
$MixaManagerDir = Join-Path $RepoRoot "mixa_manager"

$Stage = "preflight"
$Status = "FAILED"
$Reason = ""
$Compiler = ""
$ActualCompilerHash = ""
. (Join-Path $PSScriptRoot "lib_l2_runtime_support.ps1")
$CompilerHash = Get-L1Pin -L1Root (Join-Path $RepoRoot "stg\l1_baseline")
$RunDir = ""
$LogDir = ""
$TestSource = Join-Path $RepoRoot "mixa_manager\tests\mixa_app_fmpanel_copyresult_e2e_selftest.lm1"

if (-not (Test-Path $MixaManagerDir)) {
    Write-Error "Repository structure invalid; mixa_manager not found at $MixaManagerDir"
    exit 1
}

function Invoke-HeaderTranslation {
    param([string]$Name, [string]$SourceRel, [string]$OutName)
    $src = Join-Path $RepoRoot $SourceRel
    $out = Join-Path $HeaderDir $OutName
    $stdoutFile = Join-Path $LogDir "${Name}_header_trans_stdout.log"
    $stderrFile = Join-Path $LogDir "${Name}_header_trans_stderr.log"
    $exitFile = Join-Path $LogDir "${Name}_header_trans_exit.txt"
    $proc = Start-Process -FilePath $Compiler -ArgumentList $src, $out -Wait -PassThru -NoNewWindow -WorkingDirectory $RepoRoot -RedirectStandardOutput $stdoutFile -RedirectStandardError $stderrFile
    $rc = $proc.ExitCode
    Set-Content -LiteralPath $exitFile -Value $rc
    if ($rc -ne 0) { throw "$Name header translation failed with exit $rc" }
}

function Invoke-UnitCompile {
    param([string]$Name, [string]$SourceRel, [string]$HeaderIncludeRoot)
    $src = Join-Path $RepoRoot $SourceRel
    $cOut = Join-Path $RunDir ($Name + ".c")
    $oOut = Join-Path $RunDir ($Name + ".o")
    $transStdout = Join-Path $LogDir ($Name + "_trans_stdout.log")
    $transStderr = Join-Path $LogDir ($Name + "_trans_stderr.log")
    $transExitFile = Join-Path $LogDir ($Name + "_trans_exit.txt")
    $proc = Start-Process -FilePath $Compiler -ArgumentList $src, $cOut -Wait -PassThru -NoNewWindow -WorkingDirectory $RepoRoot -RedirectStandardOutput $transStdout -RedirectStandardError $transStderr
    $rc = $proc.ExitCode
    Set-Content -LiteralPath $transExitFile -Value $rc
    if ($rc -ne 0) { throw "$Name translation failed with exit $rc" }
    $compileStdout = Join-Path $LogDir ($Name + "_compile_stdout.log")
    $compileStderr = Join-Path $LogDir ($Name + "_compile_stderr.log")
    $compileExitFile = Join-Path $LogDir ($Name + "_compile_exit.txt")
    $gccArgs = @("-std=c99","-Wall","-Wextra","-Wpedantic","-Werror=incompatible-pointer-types","-Werror=discarded-qualifiers","-Werror=implicit-function-declaration","-Werror=implicit-int","-I",".","-I",$HeaderIncludeRoot,"-c",$cOut,"-o",$oOut)
    $gccProc = Start-Process -FilePath "gcc.exe" -ArgumentList $gccArgs -Wait -PassThru -NoNewWindow -WorkingDirectory $RepoRoot -RedirectStandardOutput $compileStdout -RedirectStandardError $compileStderr
    $gccRc = $gccProc.ExitCode
    Set-Content -LiteralPath $compileExitFile -Value $gccRc
    if ($gccRc -ne 0) { Get-Content $compileStderr; throw "$Name compilation failed with exit $gccRc" }
    return $oOut
}

$RunTimestamp = (Get-Date -Format "yyyyMMdd_HHmmss_fff")
$RunGuid = [GUID]::NewGuid().ToString().Substring(0, 8)
$BaseDir = Join-Path $RepoRoot "build\mixa\claude\app_fmpanel_copyresult_e2e"
$RunDir = Join-Path $BaseDir "run_${RunTimestamp}_${RunGuid}"
$LogDir = Join-Path $RunDir "logs"
$FixtureDir = Join-Path $RunDir "fixtures"

try {
    if (-not (New-Item -ItemType Directory -Path $LogDir -Force)) {
        $Reason = "Failed to create run directory: $LogDir"
        throw $Reason
    }
    New-Item -ItemType Directory -Path $FixtureDir -Force | Out-Null

    $Compiler = Join-Path $RepoRoot "stg\l1_baseline\build\l1trans\gen2\l1trans.exe"
    if (-not (Test-Path $Compiler)) { $Reason = "Compiler not found: $Compiler"; throw $Reason }
    $ActualCompilerHash = (Get-FileHash -LiteralPath $Compiler -Algorithm SHA256).Hash
    if ($ActualCompilerHash -ne $CompilerHash) {
        $Reason = "Compiler hash mismatch: expected $CompilerHash, got $ActualCompilerHash"
        throw $Reason
    }
    $Stage = "ready"

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
    $backendCtorsObj = Invoke-UnitCompile -Name "mixa_backend_ctors_headless" -SourceRel "mixa_manager\mixa_backend_ctors_headless.lm1" -HeaderIncludeRoot $HeaderIncludeRoot
    $pumpObj = Invoke-UnitCompile -Name "mixa_pump" -SourceRel "mixa_manager\mixa_pump.lm1" -HeaderIncludeRoot $HeaderIncludeRoot
    $consoleWindowObj = Invoke-UnitCompile -Name "mixa_console_window" -SourceRel "mixa_manager\mixa_console_window.lm1" -HeaderIncludeRoot $HeaderIncludeRoot
    $fileWin32Obj = Invoke-UnitCompile -Name "mixa_file_win32" -SourceRel "mixa_manager\mixa_file_win32.lm1" -HeaderIncludeRoot $HeaderIncludeRoot
    $appPathObj = Invoke-UnitCompile -Name "mixa_app_path" -SourceRel "mixa_manager\mixa_app_path.lm1" -HeaderIncludeRoot $HeaderIncludeRoot
    $helpObj = Invoke-UnitCompile -Name "mixa_help" -SourceRel "mixa_manager\mixa_help.lm1" -HeaderIncludeRoot $HeaderIncludeRoot
    $cmdlineObj = Invoke-UnitCompile -Name "mixa_cmdline" -SourceRel "mixa_manager\mixa_cmdline.lm1" -HeaderIncludeRoot $HeaderIncludeRoot
    $cmdlineDispatchObj = Invoke-UnitCompile -Name "mixa_cmdline_dispatch" -SourceRel "mixa_manager\mixa_cmdline_dispatch.lm1" -HeaderIncludeRoot $HeaderIncludeRoot
    $processMarkerObj = Invoke-UnitCompile -Name "mixa_process_marker" -SourceRel "mixa_manager\mixa_process_marker.lm1" -HeaderIncludeRoot $HeaderIncludeRoot
    $processWin32Obj = Invoke-UnitCompile -Name "mixa_process_win32" -SourceRel "mixa_manager\mixa_process_win32.lm1" -HeaderIncludeRoot $HeaderIncludeRoot
    $controllerObj = Invoke-UnitCompile -Name "mixa_app_controller" -SourceRel "mixa_manager\mixa_app_controller.lm1" -HeaderIncludeRoot $HeaderIncludeRoot
    $fmpanelObj = Invoke-UnitCompile -Name "mixa_app_fmpanel" -SourceRel "mixa_manager\mixa_app_fmpanel.lm1" -HeaderIncludeRoot $HeaderIncludeRoot
    $highlightObj = Invoke-UnitCompile -Name "mixa_highlight" -SourceRel "mixa_manager\mixa_highlight.lm1" -HeaderIncludeRoot $HeaderIncludeRoot

    $TransOut = Join-Path $RunDir "mixa_app_fmpanel_copyresult_e2e_selftest.c"
    $TestObj = Join-Path $RunDir "mixa_app_fmpanel_copyresult_e2e_selftest.o"
    $ExeOut = Join-Path $RunDir "mixa_app_fmpanel_copyresult_e2e_selftest.exe"
    $TransStdout = Join-Path $LogDir "trans_stdout.log"
    $TransStderr = Join-Path $LogDir "trans_stderr.log"
    $TransExitFile = Join-Path $LogDir "trans_exit.txt"
    $Stage = "translation"
    $TransProc = Start-Process -FilePath $Compiler -ArgumentList $TestSource, $TransOut -Wait -PassThru -NoNewWindow -WorkingDirectory $RepoRoot -RedirectStandardOutput $TransStdout -RedirectStandardError $TransStderr
    $TransRc = $TransProc.ExitCode
    Set-Content -LiteralPath $TransExitFile -Value $TransRc
    if ($TransRc -ne 0) { $Reason = "Translation failed with exit $TransRc"; throw $Reason }

    $CompileStdout = Join-Path $LogDir "compile.log"
    $CompileStderr = Join-Path $LogDir "compile_stderr.log"
    $CompileExitFile = Join-Path $LogDir "compile_exit.txt"
    $Stage = "compilation"
    $GccArgs = @("-std=c99","-Wall","-Wextra","-Wpedantic","-Werror=incompatible-pointer-types","-Werror=discarded-qualifiers","-Werror=implicit-function-declaration","-Werror=implicit-int","-I",".","-I",$HeaderIncludeRoot,"-c",$TransOut,"-o",$TestObj)
    $GccProc = Start-Process -FilePath "gcc.exe" -ArgumentList $GccArgs -Wait -PassThru -NoNewWindow -WorkingDirectory $RepoRoot -RedirectStandardOutput $CompileStdout -RedirectStandardError $CompileStderr
    $CompileRc = $GccProc.ExitCode
    Set-Content -LiteralPath $CompileExitFile -Value $CompileRc
    if ($CompileRc -ne 0) { Get-Content $CompileStderr; $Reason = "Compilation failed with exit $CompileRc"; throw $Reason }

    $Stage = "link"
    $LinkStdout = Join-Path $LogDir "link_stdout.log"
    $LinkStderr = Join-Path $LogDir "link_stderr.log"
    $LinkExitFile = Join-Path $LogDir "link_exit.txt"
    $LinkArgs = @("-std=c99","-Wall","-Wextra","-Wpedantic","-I",".","-I",$HeaderIncludeRoot,$TestObj,$controllerObj,$fmpanelObj,$highlightObj,$eventFifoObj,$backendTableObj,$backendHeadlessObj,$backendCtorsObj,$pumpObj,$consoleWindowObj,$fileWin32Obj,$appPathObj,$helpObj,$cmdlineObj,$cmdlineDispatchObj,$processMarkerObj,$processWin32Obj,"-o",$ExeOut)
    $LinkProc = Start-Process -FilePath "gcc.exe" -ArgumentList $LinkArgs -Wait -PassThru -NoNewWindow -WorkingDirectory $RepoRoot -RedirectStandardOutput $LinkStdout -RedirectStandardError $LinkStderr
    $LinkRc = $LinkProc.ExitCode
    Set-Content -LiteralPath $LinkExitFile -Value $LinkRc
    if ($LinkRc -ne 0) { Get-Content $LinkStderr; $Reason = "Link failed with exit $LinkRc"; throw $Reason }

    $Stage = "execution"
    $TestStdout = Join-Path $LogDir "test_stdout.log"
    $TestStderr = Join-Path $LogDir "test_stderr.log"
    $TestExitFile = Join-Path $LogDir "test_exit.txt"
    $TestProc = Start-Process -FilePath $ExeOut -ArgumentList $FixtureDir -Wait -PassThru -NoNewWindow -WorkingDirectory $RepoRoot -RedirectStandardOutput $TestStdout -RedirectStandardError $TestStderr
    $TestExitCode = $TestProc.ExitCode
    Set-Content -LiteralPath $TestExitFile -Value $TestExitCode
    if ($TestExitCode -ne 0) { Get-Content $TestStdout; Get-Content $TestStderr; $Reason = "Test execution failed with exit $TestExitCode"; throw $Reason }
    Get-Content $TestStdout

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
    $TestSourceHashFile = Join-Path $LogDir "test_source_hash.txt"
    $HashFiles = @{
        "app_controller_header" = "mixa_manager\mixa_app_controller.h"
        "app_controller_impl_header" = "mixa_manager\mixa_app_controller_impl.h"
        "app_controller_impl" = "mixa_manager\mixa_app_controller.lm1"
        "fmpanel_header" = "mixa_manager\mixa_app_fmpanel.h"
        "fmpanel_impl_header" = "mixa_manager\mixa_app_fmpanel_impl.h"
        "fmpanel_impl" = "mixa_manager\mixa_app_fmpanel.lm1"
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
        if (Test-Path -LiteralPath $TestSource -PathType Leaf) {
            Set-Content -LiteralPath $TestSourceHashFile -Value ((Get-FileHash -LiteralPath $TestSource -Algorithm SHA256).Hash)
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
TestSource-Hash-File: $TestSourceHashFile
Runner-Hash-File: $RunnerHashFile
Test-Stdout: $TestStdout
Test-Stderr: $TestStderr
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

if ($Status -eq "SUCCESS") { exit 0 } else { exit 1 }
