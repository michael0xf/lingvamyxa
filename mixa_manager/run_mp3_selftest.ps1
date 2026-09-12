# MP3 composition selftest runner (ticket 20260912-084943): a real
# top-level button hit that scans a real directory through the accepted
# mixa_dir seam and opens the accepted audio panel over what it found.
#
# This runner uses the CANDIDATE compiler, not stable65D5. The composed
# unit needs 22 imports and stable65D5's import table caps at 16, which
# is exactly why this stage was blocked until now. stable65D5 is not
# modified, is verified unchanged on every run, and still builds both
# halves on its own through run_audio_scan_selftest.ps1 and
# run_audio_launch_selftest.ps1.
#
# Fixtures live under this run's own directory and are rebuilt from
# scratch each run. Deterministic fake audio device only: no real native
# playback and no user media.

param()

$ErrorActionPreference = "Stop"

$RepoRoot = Split-Path -Parent $PSScriptRoot
$MixaManagerDir = Join-Path $RepoRoot "mixa_manager"

$Stage = "preflight"
$Status = "FAILED"
$Reason = ""
$Compiler = ""
$ActualCompilerHash = ""
# CANDIDATE compiler from ticket 20260912-084943, not stable65D5. This
# unit needs 22 imports and stable65D5's import table caps at 16, so it
# cannot be built with the stable compiler at all -- that is the whole
# reason this stage was blocked. stable65D5 is NOT modified and is still
# hashed below so the evidence records both.
$CompilerHash = "24A1B57B6C831C7B45630DF8CA61A7441376B4EB8EF946B1F08FC41957290B09"
$StableHash = "65D5A5ED127CA1BAEBDD1D500A5B74CEEA63EC1985EAC52EDEF28EFEB261C936"
$ActualStableHash = ""
$RunDir = ""
$LogDir = ""
$TestSource = Join-Path $RepoRoot "mixa_manager\tests\mixa_audio_mp3_selftest.lm1"
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
$BaseDir = Join-Path $RepoRoot "build\mixa\claude\mp3"
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

    $Compiler = Join-Path $RepoRoot "build\codex\l1-import-capacity\build\import_capacity\run_20260912_084844\l1trans.exe"
    if (-not (Test-Path $Compiler)) {
        $Reason = "Candidate compiler not found: $Compiler"
        throw $Reason
    }
    $ActualCompilerHash = (Get-FileHash -LiteralPath $Compiler -Algorithm SHA256).Hash
    if ($ActualCompilerHash -ne $CompilerHash) {
        $Reason = "Candidate compiler hash mismatch: expected $CompilerHash, got $ActualCompilerHash"
        throw $Reason
    }

    # stable65D5 must still be exactly what it was. This run does not use
    # it, but a silently changed stable compiler is worth failing on.
    $StableCompiler = Join-Path $RepoRoot "stg\l1_baseline\build\l1trans\gen2\l1trans.exe"
    if (Test-Path $StableCompiler) {
        $ActualStableHash = (Get-FileHash -LiteralPath $StableCompiler -Algorithm SHA256).Hash
        if ($ActualStableHash -ne $StableHash) {
            $Reason = "stable65D5 changed: expected $StableHash, got $ActualStableHash"
            throw $Reason
        }
    }
    $Stage = "ready"

    $HeaderDir = Join-Path $RunDir "headers\mixa_manager"
    New-Item -ItemType Directory -Path $HeaderDir -Force | Out-Null

    $Stage = "header-translation"
    Invoke-HeaderTranslation -Name "dir_win32" -SourceRel "mixa_manager\mixa_dir_win32.h.lm1" -OutName "mixa_dir_win32.lm1.h"
    Invoke-HeaderTranslation -Name "dir" -SourceRel "mixa_manager\mixa_dir.h.lm1" -OutName "mixa_dir.lm1.h"
    Invoke-HeaderTranslation -Name "audio_win32" -SourceRel "mixa_manager\mixa_audio_win32.h.lm1" -OutName "mixa_audio_win32.lm1.h"
    Invoke-HeaderTranslation -Name "audio" -SourceRel "mixa_manager\mixa_audio.h.lm1" -OutName "mixa_audio.lm1.h"
    Invoke-HeaderTranslation -Name "audio_scan" -SourceRel "mixa_manager\mixa_audio_scan.h.lm1" -OutName "mixa_audio_scan.lm1.h"
    Invoke-HeaderTranslation -Name "audio_button" -SourceRel "mixa_manager\mixa_audio_button.h.lm1" -OutName "mixa_audio_button.lm1.h"
    Invoke-HeaderTranslation -Name "app_window" -SourceRel "mixa_manager\mixa_app_window.h.lm1" -OutName "mixa_app_window.lm1.h"
    Invoke-HeaderTranslation -Name "audio_panel" -SourceRel "mixa_manager\mixa_audio_panel.h.lm1" -OutName "mixa_audio_panel.lm1.h"
    Invoke-HeaderTranslation -Name "audio_launch" -SourceRel "mixa_manager\mixa_audio_launch.h.lm1" -OutName "mixa_audio_launch.lm1.h"
    Invoke-HeaderTranslation -Name "audio_mp3" -SourceRel "mixa_manager\mixa_audio_mp3.h.lm1" -OutName "mixa_audio_mp3.lm1.h"

    $TransOut = Join-Path $RunDir "mixa_audio_mp3_selftest.c"
    $ExeOut = Join-Path $RunDir "mixa_audio_mp3_selftest.exe"
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

    $GccArgs = @("-std=c99","-Wall","-Wextra","-Wpedantic","-Werror=incompatible-pointer-types","-Werror=discarded-qualifiers","-Werror=implicit-function-declaration","-Werror=implicit-int","-I",".","-I",$HeaderIncludeRoot,$TransOut,"-o",$ExeOut)
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
        "audio_win32_header" = "mixa_manager\mixa_audio_win32.h.lm1"
        "audio_win32_impl" = "mixa_manager\mixa_audio_win32.lm1"
        "audio_header" = "mixa_manager\mixa_audio.h.lm1"
        "audio_impl" = "mixa_manager\mixa_audio.lm1"
        "audio_scan_header" = "mixa_manager\mixa_audio_scan.h.lm1"
        "audio_scan_impl" = "mixa_manager\mixa_audio_scan.lm1"
        "audio_button_header" = "mixa_manager\mixa_audio_button.h.lm1"
        "audio_button_impl" = "mixa_manager\mixa_audio_button.lm1"
        "app_window_header" = "mixa_manager\mixa_app_window.h.lm1"
        "app_window_impl" = "mixa_manager\mixa_app_window.lm1"
        "audio_panel_header" = "mixa_manager\mixa_audio_panel.h.lm1"
        "audio_panel_impl" = "mixa_manager\mixa_audio_panel.lm1"
        "audio_launch_header" = "mixa_manager\mixa_audio_launch.h.lm1"
        "audio_launch_impl" = "mixa_manager\mixa_audio_launch.lm1"
        "audio_mp3_header" = "mixa_manager\mixa_audio_mp3.h.lm1"
        "audio_mp3_impl" = "mixa_manager\mixa_audio_mp3.lm1"
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
Compiler-Kind: candidate (ticket 20260912-084943), NOT stable65D5
Compiler-Hash: $ActualCompilerHash
Stable65D5-Hash: $ActualStableHash
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
