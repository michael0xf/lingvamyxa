# Native Windows Share smoke test runner (ClearShell "Send e-mail" ->
# native Share, ticket 20260911-210200). Real IDataTransferManagerInterop/
# DataTransferManager/DataRequested/ShowShareUIForWindow round trip against
# a real window -- distinct from and in addition to run_share_selftest.ps1's
# deterministic fake-backend test, not a replacement for it.
#
# Requires the Windows 10 SDK's plain-C WinRT headers (not bundled with
# MinGW) for the DataTransferManager/DataPackage/DataRequestedEventArgs
# interfaces; see mixa_share.txt for the feasibility research and why
# -idirafter (not -I) is required for that extra include path.

param(
    [string]$WindowsSdkIncludeRoot = "C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0"
)

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
$TestSource = Join-Path $RepoRoot "mixa_manager\tests\mixa_share_native_smoke.lm1"
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

$SdkWinrtDir = Join-Path $WindowsSdkIncludeRoot "winrt"
if (-not (Test-Path $SdkWinrtDir)) {
    Write-Error "Windows SDK winrt include dir not found: $SdkWinrtDir (pass -WindowsSdkIncludeRoot to override)"
    exit 1
}

$RunTimestamp = (Get-Date -Format "yyyyMMdd_HHmmss_fff")
$RunGuid = [GUID]::NewGuid().ToString().Substring(0, 8)
$BaseDir = Join-Path $RepoRoot "build\mixa\claude\share_native_smoke"
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

    # Header translation into this run's private header dir (concrete win32
    # unit, then portable contract) -- same pattern as run_share_selftest.ps1.
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

    # Test-only fake async-operation seam header (ticket 20260912-033900):
    # same two-argument l1trans invocation as the two headers above --
    # confirmed by direct reproduction that l1trans's generated C for the
    # main unit emits an #include for this predef'd header's own
    # "<basename>.lm1.h" companion (mirroring mixa_share_win32.lm1.h/
    # mixa_share.lm1.h above), which nothing else produces; the earlier
    # symptom was a gcc "file not found" on that #include, not an
    # l1trans-reported translation error, since l1trans itself does not
    # generate this companion as a side effect of the MAIN translation --
    # it must be pre-generated exactly like the other two.
    $FakesHeaderDir = Join-Path $RunDir "headers\mixa_manager\tests"
    New-Item -ItemType Directory -Path $FakesHeaderDir -Force | Out-Null
    $FakesHeaderSource = Join-Path $RepoRoot "mixa_manager\tests\mixa_share_native_smoke_fakes.h.lm1"
    $FakesHeaderOut = Join-Path $FakesHeaderDir "mixa_share_native_smoke_fakes.lm1.h"
    $FakesHeaderTransStdout = Join-Path $LogDir "fakes_header_trans_stdout.log"
    $FakesHeaderTransStderr = Join-Path $LogDir "fakes_header_trans_stderr.log"
    $FakesHeaderTransExitFile = Join-Path $LogDir "fakes_header_trans_exit.txt"
    $Stage = "fakes-header-translation"
    $FakesHeaderTransProc = Start-Process -FilePath $Compiler -ArgumentList $FakesHeaderSource, $FakesHeaderOut -Wait -PassThru -NoNewWindow -WorkingDirectory $RepoRoot -RedirectStandardOutput $FakesHeaderTransStdout -RedirectStandardError $FakesHeaderTransStderr
    $FakesHeaderTransRc = $FakesHeaderTransProc.ExitCode
    Set-Content -LiteralPath $FakesHeaderTransExitFile -Value $FakesHeaderTransRc
    if ($FakesHeaderTransRc -ne 0) {
        $Reason = "Fakes header translation failed with exit $FakesHeaderTransRc"
        throw $Reason
    }

    # Translate test (predefs headers + mixa_share.lm1 + mixa_share_win32.lm1
    # + main, all in one TU).
    $TransOut = Join-Path $RunDir "mixa_share_native_smoke.c"
    $ExeOut = Join-Path $RunDir "mixa_share_native_smoke.exe"
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

    # Compile: -idirafter (not -I) for the SDK winrt path so MinGW's own
    # compatible headers (roapi.h etc.) resolve first; the SDK path is only
    # consulted for what MinGW genuinely lacks (the WinRT interfaces
    # themselves). See mixa_share.txt for why plain -I breaks the build.
    $GCC = "gcc.exe"
    $CompileStdout = Join-Path $LogDir "compile.log"
    $CompileStderr = Join-Path $LogDir "compile_stderr.log"
    $CompileExitFile = Join-Path $LogDir "compile_exit.txt"
    $HeaderIncludeRoot = Join-Path $RunDir "headers"
    $Stage = "compilation"
    # Start-Process -ArgumentList (given a string array) does NOT quote
    # elements containing spaces -- it naively space-joins them, silently
    # splitting a path like "...Program Files (x86)..." into multiple
    # broken arguments. Confirmed by direct reproduction. Fix: wrap only
    # the argument(s) that may contain spaces in explicit literal quotes.
    $GccArgs = @("-std=c99","-Wall","-Wextra","-Wpedantic","-Werror=incompatible-pointer-types","-Werror=discarded-qualifiers","-Werror=implicit-function-declaration","-Werror=implicit-int","-I",".","-I","`"$HeaderIncludeRoot`"","-idirafter","`"$SdkWinrtDir`"","`"$TransOut`"","-o","`"$ExeOut`"","-lruntimeobject","-luser32","-lole32")
    $GccProc = Start-Process -FilePath $GCC -ArgumentList $GccArgs -Wait -PassThru -NoNewWindow -WorkingDirectory $RepoRoot -RedirectStandardOutput $CompileStdout -RedirectStandardError $CompileStderr
    $CompileRc = $GccProc.ExitCode
    Set-Content -LiteralPath $CompileExitFile -Value $CompileRc
    if ($CompileRc -ne 0) {
        $Reason = "Compilation failed with exit $CompileRc"
        throw $Reason
    }

    # Execution: real window, real WinRT round trip. Sends Escape near the
    # end to best-effort dismiss the OS share flyout; pass/fail is judged
    # only by the DataRequested-handed-off assertion inside the test, not
    # by whether the flyout was actually dismissed (see the .lm1's header
    # note on honest platform behavior).
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
    $ConcreteHeaderHashFile = Join-Path $LogDir "concrete_header_hash.txt"
    $PortableHeaderHashFile = Join-Path $LogDir "portable_header_hash.txt"
    $PortableImplHashFile = Join-Path $LogDir "portable_impl_hash.txt"
    $ImplHashFile = Join-Path $LogDir "impl_hash.txt"
    $FakesHeaderHashFile = Join-Path $LogDir "fakes_header_hash.txt"
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
        # mixa_share.lm1 (the portable IMPLEMENTATION, not just its header)
        # is part of this test's translation unit and holds real logic --
        # e.g. the use-after-free fix in mixa_share_destroy() -- that
        # ticket 20260912-004800 correctly pointed out was missing from
        # this evidence manifest even though it directly affects what the
        # native smoke test exercises.
        $PortableImplPath = Join-Path $RepoRoot "mixa_manager\mixa_share.lm1"
        if (Test-Path -LiteralPath $PortableImplPath -PathType Leaf) {
            Set-Content -LiteralPath $PortableImplHashFile -Value ((Get-FileHash -LiteralPath $PortableImplPath -Algorithm SHA256).Hash)
        }
        $ImplPath = Join-Path $RepoRoot "mixa_manager\mixa_share_win32.lm1"
        if (Test-Path -LiteralPath $ImplPath -PathType Leaf) {
            Set-Content -LiteralPath $ImplHashFile -Value ((Get-FileHash -LiteralPath $ImplPath -Algorithm SHA256).Hash)
        }
        if (Test-Path -LiteralPath $FakesHeaderSource -PathType Leaf) {
            Set-Content -LiteralPath $FakesHeaderHashFile -Value ((Get-FileHash -LiteralPath $FakesHeaderSource -Algorithm SHA256).Hash)
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
Windows-SDK-Winrt-Dir: $SdkWinrtDir
Compiler: $Compiler
Compiler-Hash-File: $CompilerHashFile
Concrete-Header-Hash-File: $ConcreteHeaderHashFile
Portable-Header-Hash-File: $PortableHeaderHashFile
Portable-Impl-Hash-File: $PortableImplHashFile
Impl-Hash-File: $ImplHashFile
Fakes-Header-Hash-File: $FakesHeaderHashFile
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
