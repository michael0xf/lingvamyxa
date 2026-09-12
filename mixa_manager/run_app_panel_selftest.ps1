# Composed App/shortcut button panel selftest runner (ticket
# 20260912-055917's "real App/shortcut button integration"). Patterned
# from run_app_selftest.ps1's own fixture-creation (real .lnk via
# WScript.Shell, harmless fixture exe) combined with run_share_button_
# selftest.ps1's own multi-header-translation helper.

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
$TestSource = Join-Path $RepoRoot "mixa_manager\tests\mixa_app_panel_selftest.lm1"
$FixtureSource = Join-Path $RepoRoot "mixa_manager\tests\mixa_app_fixture_invoke.lm1"
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
$BaseDir = Join-Path $RepoRoot "build\mixa\claude\app_panel"
$RunDir = Join-Path $BaseDir "run_${RunTimestamp}_${RunGuid}"
$LogDir = Join-Path $RunDir "logs"

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

    # Translate and compile the L1 fixture exe (needed by .lnk fixtures) --
    # same harmless marker-writing target run_app_selftest.ps1 already uses.
    $FixtureDir = Join-Path $RunDir "fixtures"
    New-Item -ItemType Directory -Path $FixtureDir -Force | Out-Null
    $FixtureExe = Join-Path $FixtureDir "mixa_app_fixture_invoke.exe"
    $FixtureTransC = Join-Path $FixtureDir "mixa_app_fixture_invoke.c"
    $FixtureTransStdout = Join-Path $LogDir "fixture_trans_stdout.log"
    $FixtureTransStderr = Join-Path $LogDir "fixture_trans_stderr.log"
    $FixtureTransExitFile = Join-Path $LogDir "fixture_trans_exit.txt"
    $Stage = "fixture-translation"

    $FixtureTransProc = Start-Process -FilePath $Compiler -ArgumentList $FixtureSource, $FixtureTransC -Wait -PassThru -NoNewWindow -WorkingDirectory $RepoRoot -RedirectStandardOutput $FixtureTransStdout -RedirectStandardError $FixtureTransStderr
    $FixtureTransRc = $FixtureTransProc.ExitCode
    Set-Content -LiteralPath $FixtureTransExitFile -Value $FixtureTransRc
    if ($FixtureTransRc -ne 0) {
        $Reason = "Fixture translation failed with exit $FixtureTransRc"
        throw $Reason
    }

    $FixtureCompileStdout = Join-Path $LogDir "fixture_compile.log"
    $FixtureCompileStderr = Join-Path $LogDir "fixture_compile_stderr.log"
    $FixtureGcc = "gcc.exe"
    $FixtureGccArgs = @("-std=c99","-Wall","-Wextra","-O2",$FixtureTransC,"-o",$FixtureExe)
    $FixtureCompileProc = Start-Process -FilePath $FixtureGcc -ArgumentList $FixtureGccArgs -Wait -PassThru -NoNewWindow -WorkingDirectory $RepoRoot -RedirectStandardOutput $FixtureCompileStdout -RedirectStandardError $FixtureCompileStderr
    $FixtureCompileRc = $FixtureCompileProc.ExitCode
    if ($FixtureCompileRc -ne 0) {
        $Reason = "Fixture compile failed with exit $FixtureCompileRc"
        throw $Reason
    }

    $HeaderDir = Join-Path $RunDir "headers\mixa_manager"
    New-Item -ItemType Directory -Path $HeaderDir -Force | Out-Null

    $Stage = "header-translation"
    Invoke-HeaderTranslation -Name "app_win32" -SourceRel "mixa_manager\mixa_app_win32.h.lm1" -OutName "mixa_app_win32.lm1.h"
    Invoke-HeaderTranslation -Name "app" -SourceRel "mixa_manager\mixa_app.h.lm1" -OutName "mixa_app.lm1.h"
    Invoke-HeaderTranslation -Name "app_window" -SourceRel "mixa_manager\mixa_app_window.h.lm1" -OutName "mixa_app_window.lm1.h"
    Invoke-HeaderTranslation -Name "app_panel" -SourceRel "mixa_manager\mixa_app_panel.h.lm1" -OutName "mixa_app_panel.lm1.h"

    $TransOut = Join-Path $RunDir "mixa_app_panel_selftest.c"
    $ExeOut = Join-Path $RunDir "mixa_app_panel_selftest.exe"
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

    # Compile test (links shell32 for ShellExecuteExW/SHGetFileInfoW, same
    # as run_app_selftest.ps1).
    $GCC = "gcc.exe"
    $CompileStdout = Join-Path $LogDir "compile.log"
    $CompileStderr = Join-Path $LogDir "compile_stderr.log"
    $CompileExitFile = Join-Path $LogDir "compile_exit.txt"
    $HeaderIncludeRoot = Join-Path $RunDir "headers"
    $Stage = "compilation"

    $GccArgs = @("-std=c99","-Wall","-Wextra","-Wpedantic","-Werror=incompatible-pointer-types","-Werror=discarded-qualifiers","-Werror=implicit-function-declaration","-Werror=implicit-int","-I",".","-I",$HeaderIncludeRoot,$TransOut,"-o",$ExeOut,"-lshell32")
    $GccProc = Start-Process -FilePath $GCC -ArgumentList $GccArgs -Wait -PassThru -NoNewWindow -WorkingDirectory $RepoRoot -RedirectStandardOutput $CompileStdout -RedirectStandardError $CompileStderr
    $CompileRc = $GccProc.ExitCode
    Set-Content -LiteralPath $CompileExitFile -Value $CompileRc
    if ($CompileRc -ne 0) {
        $Reason = "Compilation failed with exit $CompileRc"
        throw $Reason
    }

    # Create the two real launch fixtures under the private fixture dir --
    # same WScript.Shell approach as run_app_selftest.ps1's own "valid.lnk",
    # each pointing at the same harmless fixture exe with its own -marker.
    $Stage = "fixture-creation"
    $Wsh = New-Object -ComObject WScript.Shell

    $LnkA = Join-Path $FixtureDir "ab.lnk"
    $MarkerA = Join-Path $FixtureDir "marker_a.txt"
    $ScA = $Wsh.CreateShortcut($LnkA)
    $ScA.TargetPath = $FixtureExe
    $ScA.Arguments = "-marker `"$MarkerA`""
    $ScA.WorkingDirectory = $FixtureDir
    $ScA.Save()

    $LnkB = Join-Path $FixtureDir "cd.lnk"
    $MarkerB = Join-Path $FixtureDir "marker_b.txt"
    $ScB = $Wsh.CreateShortcut($LnkB)
    $ScB.TargetPath = $FixtureExe
    $ScB.Arguments = "-marker `"$MarkerB`""
    $ScB.WorkingDirectory = $FixtureDir
    $ScB.Save()

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
    $RunnerHashFile = Join-Path $LogDir "runner_hash.txt"
    $TestSourceHashFile = Join-Path $LogDir "test_source_hash.txt"
    $FixtureSourceHashFile = Join-Path $LogDir "fixture_source_hash.txt"
    $HashFiles = @{
        "app_win32_header" = "mixa_manager\mixa_app_win32.h.lm1"
        "app_win32_impl" = "mixa_manager\mixa_app_win32.lm1"
        "app_header" = "mixa_manager\mixa_app.h.lm1"
        "app_window_header" = "mixa_manager\mixa_app_window.h.lm1"
        "app_window_impl" = "mixa_manager\mixa_app_window.lm1"
        "app_panel_header" = "mixa_manager\mixa_app_panel.h.lm1"
        "app_panel_impl" = "mixa_manager\mixa_app_panel.lm1"
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
        if (Test-Path -LiteralPath $FixtureSource -PathType Leaf) {
            Set-Content -LiteralPath $FixtureSourceHashFile -Value ((Get-FileHash -LiteralPath $FixtureSource -Algorithm SHA256).Hash)
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
FixtureSource-Hash-File: $FixtureSourceHashFile
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
