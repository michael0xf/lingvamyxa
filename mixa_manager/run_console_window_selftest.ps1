# Console view selftest runner (ticket 20260912-140500). The pre-existing
# untracked runner under this name assumed a different test signature (a
# single pinned binary fixture file as argv[1], with a hash-pinned
# mixa_file_win32.lm1 that predates this ticket's own real console-view
# logic) -- replaced with this version to match the ACTUAL selftest built
# here, which builds its own fixtures under a caller-supplied directory,
# the same convention every other selftest in this codebase already uses.
# The console read/wrap/cursor byte-window MODEL itself was not replaced,
# only this stale runner script.
#
# Fixtures live under this run's own directory and are rebuilt from
# scratch each run; every mutation this test performs stays inside that
# fixture root, verified before any write. No user files.

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
$TestSource = Join-Path $RepoRoot "mixa_manager\tests\mixa_console_window_selftest.lm1"
$TransOut = ""
$ExeOut = ""
$TransStdout = ""
$TransStderr = ""
$TransExitFile = ""
$CompileStdout = ""
$CompileStderr = ""
$CompileExitFile = ""
$TestStdout = ""
$TestStderr = ""
$TestExitFile = ""

if (-not (Test-Path $MixaManagerDir)) {
    Write-Error "Repository structure invalid; mixa_manager not found at $MixaManagerDir"
    exit 1
}

$RunTimestamp = (Get-Date -Format "yyyyMMdd_HHmmss_fff")
$RunGuid = [GUID]::NewGuid().ToString().Substring(0, 8)
$BaseDir = Join-Path $RepoRoot "build\mixa\claude\console_window"
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
    $Stage = "ready"

    $HeaderDir = Join-Path $RunDir "headers\mixa_manager"
    New-Item -ItemType Directory -Path $HeaderDir -Force | Out-Null
    $HeaderIncludeRoot = Join-Path $RunDir "headers"

    $Stage = "header-translation"
    Invoke-HeaderTranslation -Name "console_window" -SourceRel "mixa_manager\mixa_console_window.h.lm1" -OutName "mixa_console_window.lm1.h"

    $Stage = "unit-compile"
    # Portable headless backend stack, linked alongside the test -- same
    # units run_mixa.ps1 links for mixa_pump_selftest.
    $eventFifoObj = Invoke-UnitCompile -Name "mixa_event_fifo" -SourceRel "mixa_manager\mixa_event_fifo.lm1" -HeaderIncludeRoot $HeaderIncludeRoot
    $backendTableObj = Invoke-UnitCompile -Name "mixa_backend_table" -SourceRel "mixa_manager\mixa_backend_table.lm1" -HeaderIncludeRoot $HeaderIncludeRoot
    $backendHeadlessObj = Invoke-UnitCompile -Name "mixa_backend_headless" -SourceRel "mixa_manager\mixa_backend_headless.lm1" -HeaderIncludeRoot $HeaderIncludeRoot
    $backendCtorsObj = Invoke-UnitCompile -Name "mixa_backend_ctors_headless" -SourceRel "mixa_manager\mixa_backend_ctors_headless.lm1" -HeaderIncludeRoot $HeaderIncludeRoot
    $backendObjs = @($eventFifoObj, $backendTableObj, $backendHeadlessObj, $backendCtorsObj)

    $TransOut = Join-Path $RunDir "mixa_console_window_selftest.c"
    $TestObj = Join-Path $RunDir "mixa_console_window_selftest.o"
    $ExeOut = Join-Path $RunDir "mixa_console_window_selftest.exe"
    $TransStdout = Join-Path $LogDir "trans_stdout.log"
    $TransStderr = Join-Path $LogDir "trans_stderr.log"
    $TransExitFile = Join-Path $LogDir "trans_exit.txt"
    $Stage = "translation"

    $TransProc = Start-Process -FilePath $Compiler -ArgumentList $TestSource, $TransOut -Wait -PassThru -NoNewWindow -WorkingDirectory $RepoRoot -RedirectStandardOutput $TransStdout -RedirectStandardError $TransStderr
    $TransRc = $TransProc.ExitCode
    Set-Content -LiteralPath $TransExitFile -Value $TransRc
    if ($TransRc -ne 0) {
        $Reason = "Translation failed with exit $TransRc"
        throw $Reason
    }

    $CompileStdout = Join-Path $LogDir "compile.log"
    $CompileStderr = Join-Path $LogDir "compile_stderr.log"
    $CompileExitFile = Join-Path $LogDir "compile_exit.txt"
    $Stage = "compilation"

    $GccArgs = @("-std=c99","-Wall","-Wextra","-Wpedantic","-Werror=incompatible-pointer-types","-Werror=discarded-qualifiers","-Werror=implicit-function-declaration","-Werror=implicit-int","-I",".","-I",$HeaderIncludeRoot,"-c",$TransOut,"-o",$TestObj)
    $GccProc = Start-Process -FilePath "gcc.exe" -ArgumentList $GccArgs -Wait -PassThru -NoNewWindow -WorkingDirectory $RepoRoot -RedirectStandardOutput $CompileStdout -RedirectStandardError $CompileStderr
    $CompileRc = $GccProc.ExitCode
    Set-Content -LiteralPath $CompileExitFile -Value $CompileRc
    if ($CompileRc -ne 0) {
        Get-Content $CompileStderr
        $Reason = "Compilation failed with exit $CompileRc"
        throw $Reason
    }

    $Stage = "link"
    $LinkStdout = Join-Path $LogDir "link_stdout.log"
    $LinkStderr = Join-Path $LogDir "link_stderr.log"
    $LinkExitFile = Join-Path $LogDir "link_exit.txt"
    $LinkArgs = @("-std=c99","-Wall","-Wextra","-Wpedantic","-I",".","-I",$HeaderIncludeRoot,$TestObj) + $backendObjs + @("-o",$ExeOut)
    $LinkProc = Start-Process -FilePath "gcc.exe" -ArgumentList $LinkArgs -Wait -PassThru -NoNewWindow -WorkingDirectory $RepoRoot -RedirectStandardOutput $LinkStdout -RedirectStandardError $LinkStderr
    $LinkRc = $LinkProc.ExitCode
    Set-Content -LiteralPath $LinkExitFile -Value $LinkRc
    if ($LinkRc -ne 0) {
        Get-Content $LinkStderr
        $Reason = "Link failed with exit $LinkRc"
        throw $Reason
    }

    $Stage = "execution"
    $TestStdout = Join-Path $LogDir "test_stdout.log"
    $TestStderr = Join-Path $LogDir "test_stderr.log"
    $TestExitFile = Join-Path $LogDir "test_exit.txt"

    $TestProc = Start-Process -FilePath $ExeOut -ArgumentList $FixtureDir -Wait -PassThru -NoNewWindow -WorkingDirectory $RepoRoot -RedirectStandardOutput $TestStdout -RedirectStandardError $TestStderr
    $TestExitCode = $TestProc.ExitCode
    Set-Content -LiteralPath $TestExitFile -Value $TestExitCode
    if ($TestExitCode -ne 0) {
        $Reason = "Test execution failed with exit $TestExitCode"
        throw $Reason
    }

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
        "console_window_header" = "mixa_manager\mixa_console_window.h.lm1"
        "console_window_impl" = "mixa_manager\mixa_console_window.lm1"
        "file_header" = "mixa_manager\mixa_file.h"
        "file_win32_header" = "mixa_manager\mixa_file_win32.h"
        "file_win32_impl" = "mixa_manager\mixa_file_win32.lm1"
        "event_fifo_header" = "mixa_manager\mixa_event_fifo.h"
        "event_fifo_impl" = "mixa_manager\mixa_event_fifo.lm1"
        "backend_header" = "mixa_manager\mixa_backend.h"
        "backend_headless_header" = "mixa_manager\mixa_backend_headless.h"
        "backend_headless_impl" = "mixa_manager\mixa_backend_headless.lm1"
        "backend_table_impl" = "mixa_manager\mixa_backend_table.lm1"
        "backend_ctors_headless_impl" = "mixa_manager\mixa_backend_ctors_headless.lm1"
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
Translation-Exit-File: $TransExitFile
Compilation-Exit-File: $CompileExitFile
Execution-Exit-File: $TestExitFile
Compile-Log: $CompileStdout
Compile-Stderr: $CompileStderr
Test-Stdout: $TestStdout
Test-Stderr: $TestStderr
Trans-Stdout: $TransStdout
Trans-Stderr: $TransStderr
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
