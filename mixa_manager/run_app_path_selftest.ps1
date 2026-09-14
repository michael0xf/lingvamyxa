# App path selftest runner (ticket 20260912-233811): exact, checked path
# construction for the real native entrypoint -- the unit that replaced
# mixa_app_main.lm1's former fixed char[1040] root / char[1100] console
# path buffers. Pure in-process byte checks: this opens no window, spawns
# no process and touches no file, so it is safe in an automated suite in
# a way the entrypoint itself never can be. Single translation unit (the
# test predefs mixa_app_path.lm1; its declarations come from the plain C
# header mixa_app_path.h via include:, so no header-translation stage is
# needed). No user files.

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
$TestSource = Join-Path $RepoRoot "mixa_manager\tests\mixa_app_path_selftest.lm1"
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
$BaseDir = Join-Path $RepoRoot "build\mixa\claude\app_path"
$RunDir = Join-Path $BaseDir "run_${RunTimestamp}_${RunGuid}"
$LogDir = Join-Path $RunDir "logs"

try {
    if (-not (New-Item -ItemType Directory -Path $LogDir -Force)) {
        $Reason = "Failed to create run directory: $LogDir"
        throw $Reason
    }

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

    $TransOut = Join-Path $RunDir "mixa_app_path_selftest.c"
    $ExeOut = Join-Path $RunDir "mixa_app_path_selftest.exe"
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

    $GccArgs = @("-std=c99","-Wall","-Wextra","-Wpedantic","-Werror=incompatible-pointer-types","-Werror=discarded-qualifiers","-Werror=implicit-function-declaration","-Werror=implicit-int","-I",".",$TransOut,"-o",$ExeOut)
    $GccProc = Start-Process -FilePath "gcc.exe" -ArgumentList $GccArgs -Wait -PassThru -NoNewWindow -WorkingDirectory $RepoRoot -RedirectStandardOutput $CompileStdout -RedirectStandardError $CompileStderr
    $CompileRc = $GccProc.ExitCode
    Set-Content -LiteralPath $CompileExitFile -Value $CompileRc
    if ($CompileRc -ne 0) {
        Get-Content $CompileStderr
        $Reason = "Compilation failed with exit $CompileRc"
        throw $Reason
    }

    $Stage = "execution"
    $TestStdout = Join-Path $LogDir "test_stdout.log"
    $TestStderr = Join-Path $LogDir "test_stderr.log"
    $TestExitFile = Join-Path $LogDir "test_exit.txt"

    $TestProc = Start-Process -FilePath $ExeOut -Wait -PassThru -NoNewWindow -WorkingDirectory $RepoRoot -RedirectStandardOutput $TestStdout -RedirectStandardError $TestStderr
    $TestExitCode = $TestProc.ExitCode
    Set-Content -LiteralPath $TestExitFile -Value $TestExitCode
    if ($TestExitCode -ne 0) {
        Get-Content $TestStdout
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
        "app_path_header" = "mixa_manager\mixa_app_path.h"
        "app_path_impl" = "mixa_manager\mixa_app_path.lm1"
        "app_main_impl" = "mixa_manager\mixa_app_main.lm1"
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
