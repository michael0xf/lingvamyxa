# App/shortcut seam (ClearShell "App" button) selftest runner
# Patterned from run_app_window_selftest.ps1 with stable compiler hash.
# Fixtures are created under this run's private dir (real .lnk files via
# WScript.Shell, .url INI text, a junction) plus a harmless fixture exe that
# only writes a marker file; the test never launches anything else.

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
$FixtureDir = ""
$TestSource = Join-Path $RepoRoot "mixa_manager\tests\mixa_app_selftest.lm1"
$FixtureSource = Join-Path $RepoRoot "mixa_manager\tests\mixa_app_fixture_invoke.c"
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
$BaseDir = Join-Path $RepoRoot "build\mixa\opencode\app_seam"
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

    # Compile the launch fixture exe first (needed by .lnk fixtures).
    $FixtureDir = Join-Path $RunDir "fixtures"
    New-Item -ItemType Directory -Path $FixtureDir -Force | Out-Null
    $FixtureExe = Join-Path $FixtureDir "mixa_app_fixture_invoke.exe"
    $FixtureCompileStdout = Join-Path $LogDir "fixture_compile.log"
    $FixtureCompileStderr = Join-Path $LogDir "fixture_compile_stderr.log"
    $FixtureGcc = "gcc.exe"
    $FixtureGccArgs = @("-std=c99","-Wall","-Wextra","-O2",$FixtureSource,"-o",$FixtureExe)
    $FixtureCompileProc = Start-Process -FilePath $FixtureGcc -ArgumentList $FixtureGccArgs -Wait -PassThru -NoNewWindow -WorkingDirectory $RepoRoot -RedirectStandardOutput $FixtureCompileStdout -RedirectStandardError $FixtureCompileStderr
    $FixtureCompileRc = $FixtureCompileProc.ExitCode
    if ($FixtureCompileRc -ne 0) {
        $Reason = "Fixture compile failed with exit $FixtureCompileRc"
        throw $Reason
    }

    # Prepare header directory structure
    $HeaderDir = Join-Path $RunDir "headers\mixa_manager"
    New-Item -ItemType Directory -Path $HeaderDir -Force | Out-Null

    # Translate concrete header file separately
    $HeaderSource = Join-Path $RepoRoot "mixa_manager\mixa_app_win32.h.lm1"
    $HeaderOut = Join-Path $HeaderDir "mixa_app_win32.lm1.h"
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

    # Translate portable header file separately
    $PortableHeaderSource = Join-Path $RepoRoot "mixa_manager\mixa_app.h.lm1"
    $PortableHeaderOut = Join-Path $HeaderDir "mixa_app.lm1.h"
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

    # Translate test (predefs headers + module; single TU)
    $TransOut = Join-Path $RunDir "mixa_app_selftest.c"
    $ExeOut = Join-Path $RunDir "mixa_app_selftest.exe"
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

    # Compile test (links shell32 for ShellExecuteExW/SHGetFileInfoW)
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

    # Create launch fixtures under the private fixture dir.
    $Stage = "fixture-creation"
    $Wsh = New-Object -ComObject WScript.Shell

    $ValidLnk = Join-Path $FixtureDir "valid.lnk"
    $ValidSc = $Wsh.CreateShortcut($ValidLnk)
    $ValidSc.TargetPath = $FixtureExe
    $MarkerPath = Join-Path $FixtureDir "mixa_app_invoked.marker"
    $ValidSc.Arguments = "-marker `"$MarkerPath`""
    $ValidSc.WorkingDirectory = $FixtureDir
    $ValidSc.Description = "app seam launch fixture"
    $ValidSc.Save()

    # Unicode + space shortcut name; built from char codes so the runner stays
    # pure ASCII (PowerShell 5.1 reads .ps1 without BOM as ANSI).
    $Name56 = "sp ace-" + [string][char]0x0444 + [string][char]0x0430 + [string][char]0x0439 + [string][char]0x043B
    $SpaceLnk = Join-Path $FixtureDir ($Name56 + ".lnk")
    $SpaceSc = $Wsh.CreateShortcut($SpaceLnk)
    $SpaceSc.TargetPath = $FixtureExe
    $SpaceSc.Arguments = "-marker `"$MarkerPath`""
    $SpaceSc.WorkingDirectory = $FixtureDir
    $SpaceSc.Save()

    $BrokenLnk = Join-Path $FixtureDir "broken.lnk"
    $BrokenSc = $Wsh.CreateShortcut($BrokenLnk)
    $BrokenSc.TargetPath = Join-Path $FixtureDir "no_such_target_xyz.exe"
    $BrokenSc.WorkingDirectory = $FixtureDir
    $BrokenSc.Save()

    # .url (INI text), plain file, directory "dir.lnk".
    Set-Content -LiteralPath (Join-Path $FixtureDir "web.url") -Value "[InternetShortcut]`nURL=https://example.com/"
    Set-Content -LiteralPath (Join-Path $FixtureDir "plain.txt") -Value "not a shortcut"
    New-Item -ItemType Directory -Path (Join-Path $FixtureDir "dir.lnk") -Force | Out-Null

    # Junction (reparse point) -> recognized as FS_LINK, never a launcher.
    $JTarget = Join-Path $FixtureDir "jtarget"
    New-Item -ItemType Directory -Path $JTarget -Force | Out-Null
    $JunctionProc = Start-Process -FilePath "cmd.exe" -ArgumentList @("/c","mklink","/J","js_link","jtarget") -Wait -PassThru -NoNewWindow -WorkingDirectory $FixtureDir
    if ($JunctionProc.ExitCode -ne 0) {
        $Reason = "mklink /J failed with exit $($JunctionProc.ExitCode)"
        throw $Reason
    }

    # Dirs for listing tests: appdir (populated by the selftest), emptydir,
    # corruptdir with a readable-but-corrupt .link file.
    New-Item -ItemType Directory -Path (Join-Path $FixtureDir "appdir") -Force | Out-Null
    New-Item -ItemType Directory -Path (Join-Path $FixtureDir "emptydir") -Force | Out-Null
    $CorruptDir = Join-Path $FixtureDir "corruptdir"
    New-Item -ItemType Directory -Path $CorruptDir -Force | Out-Null
    Set-Content -LiteralPath (Join-Path $CorruptDir "bad.link") -Value "this is not a mixa-app-link-v1 entry"

    $Stage = "execution"
    $TestStdout = Join-Path $LogDir "test_stdout.log"
    $TestStderr = Join-Path $LogDir "test_stderr.log"
    $TestExitFile = Join-Path $LogDir "test_exit.txt"

    # Run with the fixture dir as argv[1], explicit cwd and exit capture.
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
    # Guard entire evidence-writing section: check if LogDir exists first
    if (-not (Test-Path -LiteralPath $LogDir -PathType Container)) {
        Write-Error "Run directory creation failed. Original reason: $Reason"
        exit 1
    }

    $CompilerHashFile = Join-Path $LogDir "compiler_hash.txt"
    $ConcreteHeaderHashFile = Join-Path $LogDir "concrete_header_hash.txt"
    $PortableHeaderHashFile = Join-Path $LogDir "portable_header_hash.txt"
    $ImplHashFile = Join-Path $LogDir "impl_hash.txt"
    $TestSourceHashFile = Join-Path $LogDir "test_source_hash.txt"
    $FixtureSourceHashFile = Join-Path $LogDir "fixture_source_hash.txt"
    $RunnerHashFile = Join-Path $LogDir "runner_hash.txt"

    try {
        if (Test-Path -LiteralPath $Compiler -PathType Leaf) {
            Set-Content -LiteralPath $CompilerHashFile -Value $ActualCompilerHash
        }
        $ConcreteHeaderPath = Join-Path $RepoRoot "mixa_manager\mixa_app_win32.h.lm1"
        if (Test-Path -LiteralPath $ConcreteHeaderPath -PathType Leaf) {
            Set-Content -LiteralPath $ConcreteHeaderHashFile -Value ((Get-FileHash -LiteralPath $ConcreteHeaderPath -Algorithm SHA256).Hash)
        }
        $PortableHeaderPath = Join-Path $RepoRoot "mixa_manager\mixa_app.h.lm1"
        if (Test-Path -LiteralPath $PortableHeaderPath -PathType Leaf) {
            Set-Content -LiteralPath $PortableHeaderHashFile -Value ((Get-FileHash -LiteralPath $PortableHeaderPath -Algorithm SHA256).Hash)
        }
        $ImplPath = Join-Path $RepoRoot "mixa_manager\mixa_app_win32.lm1"
        if (Test-Path -LiteralPath $ImplPath -PathType Leaf) {
            Set-Content -LiteralPath $ImplHashFile -Value ((Get-FileHash -LiteralPath $ImplPath -Algorithm SHA256).Hash)
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
Concrete-Header-Hash-File: $ConcreteHeaderHashFile
Portable-Header-Hash-File: $PortableHeaderHashFile
Impl-Hash-File: $ImplHashFile
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

# Exit with proper code based on status
if ($Status -eq "SUCCESS") {
    exit 0
} else {
    exit 1
}