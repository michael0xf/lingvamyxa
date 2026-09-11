# Nested same-app missing-function window selftest runner
# Patterned from existing isolated test runners with stable compiler hash

param()

$ErrorActionPreference = "Stop"

# $PSScriptRoot is .../lingvamyxa/mixa_manager; one parent is the repo root
$RepoRoot = Split-Path -Parent $PSScriptRoot
$MixaManagerDir = Join-Path $RepoRoot "mixa_manager"

# Initialize ALL variables and paths first (before any work or errors)
$Stage = "preflight"
$Status = "FAILED"
$Reason = ""
$Compiler = ""
$ActualCompilerHash = ""
$CompilerHash = "65D5A5ED127CA1BAEBDD1D500A5B74CEEA63EC1985EAC52EDEF28EFEB261C936"
$RunDir = ""
$LogDir = ""
$TestSource = Join-Path $RepoRoot "mixa_manager\tests\mixa_app_window_selftest.lm1"
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

# Assert expected directory structure
if (-not (Test-Path $MixaManagerDir)) {
    Write-Error "Repository structure invalid; mixa_manager not found at $MixaManagerDir"
    exit 1
}

# Create unique run directory BEFORE any work (timestamp + GUID)
$RunTimestamp = (Get-Date -Format "yyyyMMdd_HHmmss_fff")
$RunGuid = [GUID]::NewGuid().ToString().Substring(0, 8)
$BaseDir = Join-Path $RepoRoot "build\mixa\opencode\app_window"
$RunDir = Join-Path $BaseDir "run_${RunTimestamp}_${RunGuid}"
$LogDir = Join-Path $RunDir "logs"

try {
    if (-not (New-Item -ItemType Directory -Path $LogDir -Force)) {
        $Reason = "Failed to create run directory: $LogDir"
        throw $Reason
    }
    
    # Now verify compiler (AFTER LogDir created)
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
    
    # Prepare header directory structure
    $HeaderDir = Join-Path $RunDir "headers\mixa_manager"
    New-Item -ItemType Directory -Path $HeaderDir -Force | Out-Null
    
    # Translate header file separately
    Write-Host "Translating header $($RepoRoot)\mixa_manager\mixa_app_window.h.lm1"
    $Stage = "header-translation"
    
    $HeaderSource = Join-Path $RepoRoot "mixa_manager\mixa_app_window.h.lm1"
    $HeaderOut = Join-Path $HeaderDir "mixa_app_window.lm1.h"
    $HeaderTransStdout = Join-Path $LogDir "header_trans_stdout.log"
    $HeaderTransStderr = Join-Path $LogDir "header_trans_stderr.log"
    $HeaderTransExitFile = Join-Path $LogDir "header_trans_exit.txt"
    
    $HeaderTransProc = Start-Process -FilePath $Compiler -ArgumentList $HeaderSource, $HeaderOut -Wait -PassThru -NoNewWindow -WorkingDirectory $RepoRoot -RedirectStandardOutput $HeaderTransStdout -RedirectStandardError $HeaderTransStderr
    $HeaderTransRc = $HeaderTransProc.ExitCode
    Set-Content -LiteralPath $HeaderTransExitFile -Value $HeaderTransRc
    
    if ($HeaderTransRc -ne 0) {
        $Reason = "Header translation failed with exit $HeaderTransRc"
        throw $Reason
    }
    
    $TransOut = Join-Path $RunDir "mixa_app_window_selftest.c"
    $ExeOut = Join-Path $RunDir "mixa_app_window_selftest.exe"
    $TransStdout = Join-Path $LogDir "trans_stdout.log"
    $TransStderr = Join-Path $LogDir "trans_stderr.log"
    $TransExitFile = Join-Path $LogDir "trans_exit.txt"
    
    Write-Host "Translating $TestSource"
    $Stage = "translation"
    
    $TransProc = Start-Process -FilePath $Compiler -ArgumentList $TestSource, $TransOut -Wait -PassThru -NoNewWindow -WorkingDirectory $RepoRoot -RedirectStandardOutput $TransStdout -RedirectStandardError $TransStderr
    $TransRc = $TransProc.ExitCode
    Set-Content -LiteralPath $TransExitFile -Value $TransRc
    
    if ($TransRc -ne 0) {
        $Reason = "Translation failed with exit $TransRc"
        throw $Reason
    }

    Write-Host "Compiling $TransOut"
    $Stage = "compilation"
    
    $GCC = "gcc.exe"
    $CompileStdout = Join-Path $LogDir "compile.log"
    $CompileStderr = Join-Path $LogDir "compile_stderr.log"
    $CompileExitFile = Join-Path $LogDir "compile_exit.txt"
    $HeaderIncludeRoot = Join-Path $RunDir "headers"
    
    $GccArgs = @("-std=c99","-Wall","-Wextra","-Wpedantic","-Werror=incompatible-pointer-types","-Werror=discarded-qualifiers","-Werror=implicit-function-declaration","-Werror=implicit-int","-I",".","-I",$HeaderIncludeRoot,$TransOut,"-o",$ExeOut)
    $GccProc = Start-Process -FilePath $GCC -ArgumentList $GccArgs -Wait -PassThru -NoNewWindow -WorkingDirectory $RepoRoot -RedirectStandardOutput $CompileStdout -RedirectStandardError $CompileStderr
    $CompileRc = $GccProc.ExitCode
    Set-Content -LiteralPath $CompileExitFile -Value $CompileRc
    
    if ($CompileRc -ne 0) {
        $Reason = "Compilation failed with exit $CompileRc"
        throw $Reason
    }

    Write-Host "Running test $ExeOut"
    $Stage = "execution"
    
    $TestStdout = Join-Path $LogDir "test_stdout.log"
    $TestStderr = Join-Path $LogDir "test_stderr.log"
    $TestExitFile = Join-Path $LogDir "test_exit.txt"
    
    # Run with explicit working directory and exit capture
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
    # Guard entire evidence-writing section: check if LogDir exists first
    if (-not (Test-Path -LiteralPath $LogDir -PathType Container)) {
        # Directory creation failed; report original reason and exit
        Write-Error "Run directory creation failed. Original reason: $Reason"
        exit 1
    }
    
    # Generate run summary (only if LogDir valid)
    $CompilerHashFile = Join-Path $LogDir "compiler_hash.txt"
    $HeaderHashFile = Join-Path $LogDir "header_hash.txt"
    $ImplHashFile = Join-Path $LogDir "impl_hash.txt"
    $TestSourceHashFile = Join-Path $LogDir "test_source_hash.txt"
    $RunnerHashFile = Join-Path $LogDir "runner_hash.txt"
    
    try {
        # Hash available files (with existence check)
        if (Test-Path -LiteralPath $Compiler -PathType Leaf) {
            Set-Content -LiteralPath $CompilerHashFile -Value $ActualCompilerHash
        }
        
        $HeaderPath = Join-Path $RepoRoot "mixa_manager\mixa_app_window.h.lm1"
        if (Test-Path -LiteralPath $HeaderPath -PathType Leaf) {
            Set-Content -LiteralPath $HeaderHashFile -Value ((Get-FileHash -LiteralPath $HeaderPath -Algorithm SHA256).Hash)
        }
        
        $ImplPath = Join-Path $RepoRoot "mixa_manager\mixa_app_window.lm1"
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
Compiler: $Compiler
Compiler-Hash-File: $CompilerHashFile
Header-Hash-File: $HeaderHashFile
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
"@
        
        $SummaryFile = Join-Path $LogDir "run_summary.txt"
        Set-Content -LiteralPath $SummaryFile -Value $Summary
        
        Write-Host ""
        Write-Host "Summary saved to $SummaryFile"
        Write-Host "Run directory: $RunDir"
        Write-Host "Status: $Status"
        
    } catch {
        # Evidence-write failure: set FAILED, preserve original reason, return 1
        Write-Error "Failed to write evidence: $_. Original failure: $Reason"
        $Status = "FAILED"
        exit 1
    }
}

# Exit with proper code based on status
if ($Status -eq "SUCCESS") {
    exit 0
} else {
    exit 1
}