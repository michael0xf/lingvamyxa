# Copy-into-the-view integration selftest runner (ticket 20260912-
# 121231): the accepted mixa_copy operation driven as a real user-
# triggered action through the file-manager view's own borrowed
# MixaSelection, with the view refreshed through its own transactional
# replacement path exactly when the copy's destination is the directory
# currently being looked at. Fixtures live under this run's own
# directory and are rebuilt from scratch each run; every mutation this
# test performs stays inside that fixture root, verified before any
# write. No user files.

param()

$ErrorActionPreference = "Stop"

$RepoRoot = Split-Path -Parent $PSScriptRoot
$MixaManagerDir = Join-Path $RepoRoot "mixa_manager"

$Stage = "preflight"
$Status = "FAILED"
$Reason = ""
$Compiler = ""
$ActualCompilerHash = ""
$CompilerHash = "65D5A5ED127CA1BAEBDD1D500A5B74CEEA63EC1985EAC52EDEF28EFEB261C936"
$RunDir = ""
$LogDir = ""
$TestSource = Join-Path $RepoRoot "mixa_manager\tests\mixa_fm_copy_selftest.lm1"
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
$BaseDir = Join-Path $RepoRoot "build\mixa\claude\fm_copy"
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

    $Stage = "header-translation"
    Invoke-HeaderTranslation -Name "dir_win32" -SourceRel "mixa_manager\mixa_dir_win32.h.lm1" -OutName "mixa_dir_win32.lm1.h"
    Invoke-HeaderTranslation -Name "dir" -SourceRel "mixa_manager\mixa_dir.h.lm1" -OutName "mixa_dir.lm1.h"
    Invoke-HeaderTranslation -Name "selection_walk" -SourceRel "mixa_manager\mixa_selection_walk.h.lm1" -OutName "mixa_selection_walk.lm1.h"
    Invoke-HeaderTranslation -Name "fileio_win32" -SourceRel "mixa_manager\mixa_fileio_win32.h.lm1" -OutName "mixa_fileio_win32.lm1.h"
    Invoke-HeaderTranslation -Name "fileio" -SourceRel "mixa_manager\mixa_fileio.h.lm1" -OutName "mixa_fileio.lm1.h"
    Invoke-HeaderTranslation -Name "copy" -SourceRel "mixa_manager\mixa_copy.h.lm1" -OutName "mixa_copy.lm1.h"
    Invoke-HeaderTranslation -Name "file_manager" -SourceRel "mixa_manager\mixa_file_manager.h.lm1" -OutName "mixa_file_manager.lm1.h"
    Invoke-HeaderTranslation -Name "fm_copy" -SourceRel "mixa_manager\mixa_fm_copy.h.lm1" -OutName "mixa_fm_copy.lm1.h"

    # mixa_fm_copy.h.lm1 now includes mixa_pump.h (ticket 20260912-131700's
    # own top-level drain/dispatch addition), and mixa_fm_copy_here_pump_
    # dispatch's body calls mixa_pump_drain/mixa_pump_next, which in turn
    # need mixa_event_fifo and mixa_backend_table (mixa_backend_poll) --
    # all undefined unless linked in too, even though this suite never
    # calls any of it. Same minimal headless-only stack run_mixa.ps1 links
    # for mixa_pump_selftest, translated separately here and passed into
    # the same single gcc invocation below.
    $PumpUnits = @(
        @{ Name = "mixa_event_fifo"; Src = "mixa_manager\mixa_event_fifo.lm1" },
        @{ Name = "mixa_backend_table"; Src = "mixa_manager\mixa_backend_table.lm1" },
        @{ Name = "mixa_backend_headless"; Src = "mixa_manager\mixa_backend_headless.lm1" },
        @{ Name = "mixa_backend_ctors_headless"; Src = "mixa_manager\mixa_backend_ctors_headless.lm1" },
        @{ Name = "mixa_pump"; Src = "mixa_manager\mixa_pump.lm1" }
    )
    $PumpTransOuts = @()
    foreach ($pu in $PumpUnits) {
        $pOut = Join-Path $RunDir ($pu.Name + ".c")
        $pStdout = Join-Path $LogDir ($pu.Name + "_trans_stdout.log")
        $pStderr = Join-Path $LogDir ($pu.Name + "_trans_stderr.log")
        $pExitFile = Join-Path $LogDir ($pu.Name + "_trans_exit.txt")
        $pProc = Start-Process -FilePath $Compiler -ArgumentList $pu.Src, $pOut -Wait -PassThru -NoNewWindow -WorkingDirectory $RepoRoot -RedirectStandardOutput $pStdout -RedirectStandardError $pStderr
        $pRc = $pProc.ExitCode
        Set-Content -LiteralPath $pExitFile -Value $pRc
        if ($pRc -ne 0) {
            throw "$($pu.Name) translation failed with exit $pRc"
        }
        $PumpTransOuts += $pOut
    }

    $TransOut = Join-Path $RunDir "mixa_fm_copy_selftest.c"
    $ExeOut = Join-Path $RunDir "mixa_fm_copy_selftest.exe"
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

    $GCC = "gcc.exe"
    $CompileStdout = Join-Path $LogDir "compile.log"
    $CompileStderr = Join-Path $LogDir "compile_stderr.log"
    $CompileExitFile = Join-Path $LogDir "compile_exit.txt"
    $HeaderIncludeRoot = Join-Path $RunDir "headers"
    $Stage = "compilation"

    $GccArgs = @("-std=c99","-Wall","-Wextra","-Wpedantic","-Werror=incompatible-pointer-types","-Werror=discarded-qualifiers","-Werror=implicit-function-declaration","-Werror=implicit-int","-I",".","-I",$HeaderIncludeRoot,$TransOut) + $PumpTransOuts + @("-o",$ExeOut)
    $GccProc = Start-Process -FilePath $GCC -ArgumentList $GccArgs -Wait -PassThru -NoNewWindow -WorkingDirectory $RepoRoot -RedirectStandardOutput $CompileStdout -RedirectStandardError $CompileStderr
    $CompileRc = $GccProc.ExitCode
    Set-Content -LiteralPath $CompileExitFile -Value $CompileRc
    if ($CompileRc -ne 0) {
        $Reason = "Compilation failed with exit $CompileRc"
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
        "dir_win32_header" = "mixa_manager\mixa_dir_win32.h.lm1"
        "dir_win32_impl" = "mixa_manager\mixa_dir_win32.lm1"
        "dir_header" = "mixa_manager\mixa_dir.h.lm1"
        "selection_header" = "mixa_manager\mixa_selection.h"
        "selection_impl" = "mixa_manager\mixa_selection.lm1"
        "selection_walk_header" = "mixa_manager\mixa_selection_walk.h.lm1"
        "selection_walk_impl" = "mixa_manager\mixa_selection_walk.lm1"
        "fileio_win32_header" = "mixa_manager\mixa_fileio_win32.h.lm1"
        "fileio_win32_impl" = "mixa_manager\mixa_fileio_win32.lm1"
        "fileio_header" = "mixa_manager\mixa_fileio.h.lm1"
        "copy_header" = "mixa_manager\mixa_copy.h.lm1"
        "copy_impl" = "mixa_manager\mixa_copy.lm1"
        "file_manager_header" = "mixa_manager\mixa_file_manager.h.lm1"
        "file_manager_impl" = "mixa_manager\mixa_file_manager.lm1"
        "fm_copy_header" = "mixa_manager\mixa_fm_copy.h.lm1"
        "fm_copy_impl" = "mixa_manager\mixa_fm_copy.lm1"
        "pump_header" = "mixa_manager\mixa_pump.h"
        "pump_impl" = "mixa_manager\mixa_pump.lm1"
        "event_fifo_header" = "mixa_manager\mixa_event_fifo.h"
        "event_fifo_impl" = "mixa_manager\mixa_event_fifo.lm1"
        "backend_header" = "mixa_manager\mixa_backend.h"
        "backend_table_impl" = "mixa_manager\mixa_backend_table.lm1"
        "backend_headless_header" = "mixa_manager\mixa_backend_headless.h"
        "backend_headless_impl" = "mixa_manager\mixa_backend_headless.lm1"
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
