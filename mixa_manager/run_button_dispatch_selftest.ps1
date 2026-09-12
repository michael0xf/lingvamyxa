# Button-hit dispatch selftest runner (ticket 20260912-045346's "button/
# nested-failure UI" next step). Patterned from run_app_window_selftest.ps1
# with the same stable-compiler pin; two headers now need standalone
# pre-translation (mixa_app_window.h.lm1, then the new
# mixa_button_dispatch.h.lm1 which predefs it) so the main unit's generated
# C finds both #include'd companions, mirroring run_share_native_smoke.ps1's
# own multi-header precedent.

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
$TestSource = Join-Path $RepoRoot "mixa_manager\tests\mixa_button_dispatch_selftest.lm1"
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
$BaseDir = Join-Path $RepoRoot "build\mixa\claude\button_dispatch"
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
    $HeaderDir = Join-Path $RunDir "headers\mixa_manager"
    New-Item -ItemType Directory -Path $HeaderDir -Force | Out-Null

    $AppWindowHeaderSource = Join-Path $RepoRoot "mixa_manager\mixa_app_window.h.lm1"
    $AppWindowHeaderOut = Join-Path $HeaderDir "mixa_app_window.lm1.h"
    $AppWindowHeaderTransStdout = Join-Path $LogDir "app_window_header_trans_stdout.log"
    $AppWindowHeaderTransStderr = Join-Path $LogDir "app_window_header_trans_stderr.log"
    $AppWindowHeaderTransExitFile = Join-Path $LogDir "app_window_header_trans_exit.txt"
    $Stage = "app-window-header-translation"
    $AppWindowHeaderTransProc = Start-Process -FilePath $Compiler -ArgumentList $AppWindowHeaderSource, $AppWindowHeaderOut -Wait -PassThru -NoNewWindow -WorkingDirectory $RepoRoot -RedirectStandardOutput $AppWindowHeaderTransStdout -RedirectStandardError $AppWindowHeaderTransStderr
    $AppWindowHeaderTransRc = $AppWindowHeaderTransProc.ExitCode
    Set-Content -LiteralPath $AppWindowHeaderTransExitFile -Value $AppWindowHeaderTransRc
    if ($AppWindowHeaderTransRc -ne 0) {
        $Reason = "App window header translation failed with exit $AppWindowHeaderTransRc"
        throw $Reason
    }

    $DispatchHeaderSource = Join-Path $RepoRoot "mixa_manager\mixa_button_dispatch.h.lm1"
    $DispatchHeaderOut = Join-Path $HeaderDir "mixa_button_dispatch.lm1.h"
    $DispatchHeaderTransStdout = Join-Path $LogDir "dispatch_header_trans_stdout.log"
    $DispatchHeaderTransStderr = Join-Path $LogDir "dispatch_header_trans_stderr.log"
    $DispatchHeaderTransExitFile = Join-Path $LogDir "dispatch_header_trans_exit.txt"
    $Stage = "dispatch-header-translation"
    $DispatchHeaderTransProc = Start-Process -FilePath $Compiler -ArgumentList $DispatchHeaderSource, $DispatchHeaderOut -Wait -PassThru -NoNewWindow -WorkingDirectory $RepoRoot -RedirectStandardOutput $DispatchHeaderTransStdout -RedirectStandardError $DispatchHeaderTransStderr
    $DispatchHeaderTransRc = $DispatchHeaderTransProc.ExitCode
    Set-Content -LiteralPath $DispatchHeaderTransExitFile -Value $DispatchHeaderTransRc
    if ($DispatchHeaderTransRc -ne 0) {
        $Reason = "Dispatch header translation failed with exit $DispatchHeaderTransRc"
        throw $Reason
    }

    $TransOut = Join-Path $RunDir "mixa_button_dispatch_selftest.c"
    $ExeOut = Join-Path $RunDir "mixa_button_dispatch_selftest.exe"
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
    $TestProc = Start-Process -FilePath $ExeOut -Wait -PassThru -NoNewWindow -WorkingDirectory $RepoRoot -RedirectStandardOutput $TestStdout -RedirectStandardError $TestStderr
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
    $AppWindowHeaderHashFile = Join-Path $LogDir "app_window_header_hash.txt"
    $AppWindowImplHashFile = Join-Path $LogDir "app_window_impl_hash.txt"
    $DispatchHeaderHashFile = Join-Path $LogDir "dispatch_header_hash.txt"
    $DispatchImplHashFile = Join-Path $LogDir "dispatch_impl_hash.txt"
    $TestSourceHashFile = Join-Path $LogDir "test_source_hash.txt"
    $RunnerHashFile = Join-Path $LogDir "runner_hash.txt"

    try {
        if (Test-Path -LiteralPath $Compiler -PathType Leaf) {
            Set-Content -LiteralPath $CompilerHashFile -Value $ActualCompilerHash
        }
        $AppWindowHeaderPath = Join-Path $RepoRoot "mixa_manager\mixa_app_window.h.lm1"
        if (Test-Path -LiteralPath $AppWindowHeaderPath -PathType Leaf) {
            Set-Content -LiteralPath $AppWindowHeaderHashFile -Value ((Get-FileHash -LiteralPath $AppWindowHeaderPath -Algorithm SHA256).Hash)
        }
        $AppWindowImplPath = Join-Path $RepoRoot "mixa_manager\mixa_app_window.lm1"
        if (Test-Path -LiteralPath $AppWindowImplPath -PathType Leaf) {
            Set-Content -LiteralPath $AppWindowImplHashFile -Value ((Get-FileHash -LiteralPath $AppWindowImplPath -Algorithm SHA256).Hash)
        }
        $DispatchHeaderPath = Join-Path $RepoRoot "mixa_manager\mixa_button_dispatch.h.lm1"
        if (Test-Path -LiteralPath $DispatchHeaderPath -PathType Leaf) {
            Set-Content -LiteralPath $DispatchHeaderHashFile -Value ((Get-FileHash -LiteralPath $DispatchHeaderPath -Algorithm SHA256).Hash)
        }
        $DispatchImplPath = Join-Path $RepoRoot "mixa_manager\mixa_button_dispatch.lm1"
        if (Test-Path -LiteralPath $DispatchImplPath -PathType Leaf) {
            Set-Content -LiteralPath $DispatchImplHashFile -Value ((Get-FileHash -LiteralPath $DispatchImplPath -Algorithm SHA256).Hash)
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
App-Window-Header-Hash-File: $AppWindowHeaderHashFile
App-Window-Impl-Hash-File: $AppWindowImplHashFile
Dispatch-Header-Hash-File: $DispatchHeaderHashFile
Dispatch-Impl-Hash-File: $DispatchImplHashFile
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
