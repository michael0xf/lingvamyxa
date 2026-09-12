# File-share seam (ClearShell "Send e-mail" -> native Share) selftest runner.
# Patterned from run_app_selftest.ps1 with the same stable-compiler pin.
# Deterministic fake backend only; no real OS share UI or COM in this stage.

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
$FixtureDir = ""
$TestSource = Join-Path $RepoRoot "mixa_manager\tests\mixa_share_selftest.lm1"
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
$BaseDir = Join-Path $RepoRoot "build\mixa\claude\share_seam"
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
    $FixtureDir = Join-Path $RunDir "fixtures"
    New-Item -ItemType Directory -Path $FixtureDir -Force | Out-Null

    # Header translation (concrete win32 stub, then portable contract).
    $HeaderDir = Join-Path $RunDir "headers\mixa_manager"
    New-Item -ItemType Directory -Path $HeaderDir -Force | Out-Null

    $HeaderSource = Join-Path $RepoRoot "mixa_manager\mixa_share_win32.h.lm1"
    $HeaderOut = Join-Path $HeaderDir "mixa_share_win32.lm1.h"
    $HeaderTransStdout = Join-Path $LogDir "header_trans_stdout.log"
    $HeaderTransStderr = Join-Path $LogDir "header_trans_stderr.log"
    $HeaderTransExitFile = Join-Path $LogDir "header_trans_exit.txt"
    $Stage = "header-translation"
    $HeaderTransProc = Start-Process -FilePath $Compiler -ArgumentList $HeaderSource, $HeaderOut -Wait -PassThru -NoNewWindow -WorkingDirectory $RepoRoot -RedirectStandardOutput $HeaderTransStdout -RedirectStandardError $HeaderTransStderr
    $HeaderTransRc = $HeaderTransProc.ExitCode
    Set-Content -LiteralPath $HeaderTransExitFile -Value $HeaderTransRc
    if ($HeaderTransRc -ne 0) {
        $Reason = "Header translation failed with exit $HeaderTransRc"
        throw $Reason
    }

    $PortableHeaderSource = Join-Path $RepoRoot "mixa_manager\mixa_share.h.lm1"
    $PortableHeaderOut = Join-Path $HeaderDir "mixa_share.lm1.h"
    $PortableHeaderTransStdout = Join-Path $LogDir "portable_header_trans_stdout.log"
    $PortableHeaderTransStderr = Join-Path $LogDir "portable_header_trans_stderr.log"
    $PortableHeaderTransExitFile = Join-Path $LogDir "portable_header_trans_exit.txt"
    $Stage = "portable-header-translation"
    $PortableHeaderTransProc = Start-Process -FilePath $Compiler -ArgumentList $PortableHeaderSource, $PortableHeaderOut -Wait -PassThru -NoNewWindow -WorkingDirectory $RepoRoot -RedirectStandardOutput $PortableHeaderTransStdout -RedirectStandardError $PortableHeaderTransStderr
    $PortableHeaderTransRc = $PortableHeaderTransProc.ExitCode
    Set-Content -LiteralPath $PortableHeaderTransExitFile -Value $PortableHeaderTransRc
    if ($PortableHeaderTransRc -ne 0) {
        $Reason = "Portable header translation failed with exit $PortableHeaderTransRc"
        throw $Reason
    }

    # Translate test (predefs headers + mixa_share.lm1 controller + fake
    # backend + main, all in one TU -- same single-file pattern as
    # run_app_selftest.ps1).
    $TransOut = Join-Path $RunDir "mixa_share_selftest.c"
    $ExeOut = Join-Path $RunDir "mixa_share_selftest.exe"
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
    $TestProc = Start-Process -FilePath $ExeOut -ArgumentList @($FixtureDir) -Wait -PassThru -NoNewWindow -WorkingDirectory $RepoRoot -RedirectStandardOutput $TestStdout -RedirectStandardError $TestStderr
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
    $ConcreteHeaderHashFile = Join-Path $LogDir "concrete_header_hash.txt"
    $PortableHeaderHashFile = Join-Path $LogDir "portable_header_hash.txt"
    $ImplHashFile = Join-Path $LogDir "impl_hash.txt"
    $TestSourceHashFile = Join-Path $LogDir "test_source_hash.txt"
    $RunnerHashFile = Join-Path $LogDir "runner_hash.txt"

    try {
        if (Test-Path -LiteralPath $Compiler -PathType Leaf) {
            Set-Content -LiteralPath $CompilerHashFile -Value $ActualCompilerHash
        }
        $ConcreteHeaderPath = Join-Path $RepoRoot "mixa_manager\mixa_share_win32.h.lm1"
        if (Test-Path -LiteralPath $ConcreteHeaderPath -PathType Leaf) {
            Set-Content -LiteralPath $ConcreteHeaderHashFile -Value ((Get-FileHash -LiteralPath $ConcreteHeaderPath -Algorithm SHA256).Hash)
        }
        $PortableHeaderPath = Join-Path $RepoRoot "mixa_manager\mixa_share.h.lm1"
        if (Test-Path -LiteralPath $PortableHeaderPath -PathType Leaf) {
            Set-Content -LiteralPath $PortableHeaderHashFile -Value ((Get-FileHash -LiteralPath $PortableHeaderPath -Algorithm SHA256).Hash)
        }
        $ImplPath = Join-Path $RepoRoot "mixa_manager\mixa_share.lm1"
        if (Test-Path -LiteralPath $ImplPath -PathType Leaf) {
            Set-Content -LiteralPath $ImplHashFile -Value ((Get-FileHash -LiteralPath $ImplPath -Algorithm SHA256).Hash)
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
Concrete-Header-Hash-File: $ConcreteHeaderHashFile
Portable-Header-Hash-File: $PortableHeaderHashFile
Impl-Hash-File: $ImplHashFile
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
Test-Args: ($FixtureDir)
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
