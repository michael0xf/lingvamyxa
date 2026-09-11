# Audio seam native smoke-test runner: portable controller + win32/MCI backend
# with a generated WAV fixture whose path contains spaces (MCI quoting check).
# Only digital-silence audio is generated; playback is ~8 s if it ever plays.
# Patterned from run_audio_selftest.ps1; links -lwinmm like the real usage.

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
$FixtureWav = ""
$TestSource = Join-Path $RepoRoot "mixa_manager\tests\mixa_audio_native_selftest.lm1"
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
$BaseDir = Join-Path $RepoRoot "build\mixa\opencode\audio_native_seam"
$RunDir = Join-Path $BaseDir "run_${RunTimestamp}_${RunGuid}"
$LogDir = Join-Path $RunDir "logs"
$FixtureDir = Join-Path $RunDir "fixtures"

try {
    if (-not (New-Item -ItemType Directory -Path $LogDir -Force)) {
        $Reason = "Failed to create run directory: $LogDir"
        throw $Reason
    }
    if (-not (New-Item -ItemType Directory -Path $FixtureDir -Force)) {
        $Reason = "Failed to create fixture directory: $FixtureDir"
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

    $ConcreteHeaderSource = Join-Path $RepoRoot "mixa_manager\mixa_audio_win32.h.lm1"
    $ConcreteHeaderOut = Join-Path $HeaderDir "mixa_audio_win32.lm1.h"
    $ConcreteHeaderTransStdout = Join-Path $LogDir "concrete_header_trans_stdout.log"
    $ConcreteHeaderTransStderr = Join-Path $LogDir "concrete_header_trans_stderr.log"
    $ConcreteHeaderTransExitFile = Join-Path $LogDir "concrete_header_trans_exit.txt"
    $Stage = "concrete-header-translation"

    $ConcreteHeaderProc = Start-Process -FilePath $Compiler -ArgumentList $ConcreteHeaderSource, $ConcreteHeaderOut -Wait -PassThru -NoNewWindow -WorkingDirectory $RepoRoot -RedirectStandardOutput $ConcreteHeaderTransStdout -RedirectStandardError $ConcreteHeaderTransStderr
    $ConcreteHeaderTransRc = $ConcreteHeaderProc.ExitCode
    Set-Content -LiteralPath $ConcreteHeaderTransExitFile -Value $ConcreteHeaderTransRc
    if ($ConcreteHeaderTransRc -ne 0) {
        $Reason = "Concrete header translation failed with exit $ConcreteHeaderTransRc"
        throw $Reason
    }

    $PortableHeaderSource = Join-Path $RepoRoot "mixa_manager\mixa_audio.h.lm1"
    $PortableHeaderOut = Join-Path $HeaderDir "mixa_audio.lm1.h"
    $PortableHeaderTransStdout = Join-Path $LogDir "portable_header_trans_stdout.log"
    $PortableHeaderTransStderr = Join-Path $LogDir "portable_header_trans_stderr.log"
    $PortableHeaderTransExitFile = Join-Path $LogDir "portable_header_trans_exit.txt"
    $Stage = "portable-header-translation"

    $PortableHeaderProc = Start-Process -FilePath $Compiler -ArgumentList $PortableHeaderSource, $PortableHeaderOut -Wait -PassThru -NoNewWindow -WorkingDirectory $RepoRoot -RedirectStandardOutput $PortableHeaderTransStdout -RedirectStandardError $PortableHeaderTransStderr
    $PortableHeaderTransRc = $PortableHeaderProc.ExitCode
    Set-Content -LiteralPath $PortableHeaderTransExitFile -Value $PortableHeaderTransRc
    if ($PortableHeaderTransRc -ne 0) {
        $Reason = "Portable header translation failed with exit $PortableHeaderTransRc"
        throw $Reason
    }

    $TransOut = Join-Path $RunDir "mixa_audio_native_selftest.c"
    $ExeOut = Join-Path $RunDir "mixa_audio_native_selftest.exe"
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

    $GccArgs = @("-std=c99","-Wall","-Wextra","-Wpedantic","-Werror=incompatible-pointer-types","-Werror=discarded-qualifiers","-Werror=implicit-function-declaration","-Werror=implicit-int","-I",".","-I",$HeaderIncludeRoot,$TransOut,"-o",$ExeOut,"-lwinmm")
    $GccProc = Start-Process -FilePath $GCC -ArgumentList $GccArgs -Wait -PassThru -NoNewWindow -WorkingDirectory $RepoRoot -RedirectStandardOutput $CompileStdout -RedirectStandardError $CompileStderr
    $CompileRc = $GccProc.ExitCode
    Set-Content -LiteralPath $CompileExitFile -Value $CompileRc
    if ($CompileRc -ne 0) {
        $Reason = "Compilation failed with exit $CompileRc"
        throw $Reason
    }

    # Generate the fixture WAV: 8000 Hz, 8-bit PCM mono, ~8 s of near-silence
    # (constant 0x7F). The path deliberately contains a space.
    $Stage = "fixture-creation"
    $FixtureWav = Join-Path $FixtureDir "My Sound 8s.wav"
    $SampleRate = 8000
    $Seconds = 8
    $DataLen = $SampleRate * $Seconds
    $Enc = [System.Text.Encoding]::ASCII
    $Ms = New-Object System.IO.MemoryStream
    $W = New-Object System.IO.BinaryWriter($Ms)
    $W.Write($Enc.GetBytes("RIFF"))
    $W.Write([int](36 + $DataLen))
    $W.Write($Enc.GetBytes("WAVE"))
    $W.Write($Enc.GetBytes("fmt "))
    $W.Write([int]16)
    $W.Write([int16]1)
    $W.Write([int16]1)
    $W.Write([int]$SampleRate)
    $W.Write([int]$SampleRate)
    $W.Write([int16]1)
    $W.Write([int16]8)
    $W.Write($Enc.GetBytes("data"))
    $W.Write([int]$DataLen)
    $Data = New-Object byte[] $DataLen
    for ($i = 0; $i -lt $DataLen; $i++) {
        $Data[$i] = [byte]0x7F
    }
    $W.Write($Data)
    $W.Flush()
    [System.IO.File]::WriteAllBytes($FixtureWav, $Ms.ToArray())
    if (-not (Test-Path -LiteralPath $FixtureWav)) {
        $Reason = "Fixture WAV not created: $FixtureWav"
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
    $TestSourceHashFile = Join-Path $LogDir "test_source_hash.txt"
    $RunnerHashFile = Join-Path $LogDir "runner_hash.txt"

    try {
        if (Test-Path -LiteralPath $Compiler -PathType Leaf) {
            Set-Content -LiteralPath $CompilerHashFile -Value $ActualCompilerHash
        }
        $ConcreteHeaderPath = Join-Path $RepoRoot "mixa_manager\mixa_audio_win32.h.lm1"
        if (Test-Path -LiteralPath $ConcreteHeaderPath -PathType Leaf) {
            Set-Content -LiteralPath $ConcreteHeaderHashFile -Value ((Get-FileHash -LiteralPath $ConcreteHeaderPath -Algorithm SHA256).Hash)
        }
        $PortableHeaderPath = Join-Path $RepoRoot "mixa_manager\mixa_audio.h.lm1"
        if (Test-Path -LiteralPath $PortableHeaderPath -PathType Leaf) {
            Set-Content -LiteralPath $PortableHeaderHashFile -Value ((Get-FileHash -LiteralPath $PortableHeaderPath -Algorithm SHA256).Hash)
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
Fixture-Wav: $FixtureWav
Compiler: $Compiler
Compiler-Hash-File: $CompilerHashFile
Concrete-Header-Hash-File: $ConcreteHeaderHashFile
Portable-Header-Hash-File: $PortableHeaderHashFile
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