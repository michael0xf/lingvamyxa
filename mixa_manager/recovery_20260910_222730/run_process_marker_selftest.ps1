# Process marker parser selftest runner
# Patterned from existing isolated test runners with stable compiler hash

param()

$ErrorActionPreference = "Stop"

$Root = Split-Path -Parent $PSScriptRoot
$Compiler = Join-Path $Root "stg\l1_baseline\build\l1trans\gen2\l1trans.exe"
$CompilerHash = "65D5A5ED127CA1BAEBDD1D500A5B74CEEA63EC1985EAC52EDEF28EFEB261C936"

if (-not (Test-Path $Compiler)) {
    Write-Host "ERROR: Compiler not found: $Compiler"
    exit 1
}

$ActualCompilerHash = (Get-FileHash -LiteralPath $Compiler -Algorithm SHA256).Hash
if ($ActualCompilerHash -ne $CompilerHash) {
    Write-Host "ERROR: Compiler hash mismatch"
    Write-Host "  Expected: $CompilerHash"
    Write-Host "  Actual:   $ActualCompilerHash"
    exit 1
}

$OutDir = Join-Path $Root "build\mixa\opencode\process_marker"
$LogDir = Join-Path $OutDir "logs"

if (Test-Path $OutDir) {
    Remove-Item -Recurse -Force $OutDir
}

New-Item -ItemType Directory -Path $LogDir | Out-Null

$TestSource = "mixa_manager\tests\mixa_process_marker_selftest.lm1"
$TransOut = Join-Path $OutDir "mixa_process_marker_selftest.c"
$ExeOut = Join-Path $OutDir "mixa_process_marker_selftest.exe"

Write-Host "Translating $TestSource"

$TransStdout = Join-Path $LogDir "_trans_stdout.tmp"
$TransStderr = Join-Path $LogDir "_trans_stderr.tmp"

$TransProc = Start-Process -FilePath $Compiler -ArgumentList $TestSource, $TransOut -Wait -PassThru -NoNewWindow -RedirectStandardOutput $TransStdout -RedirectStandardError $TransStderr
$TransRc = $TransProc.ExitCode

if ($TransRc -ne 0) {
    Write-Host "ERROR: Translation failed: exit $TransRc"
    if (Test-Path $TransStderr) {
        Get-Content $TransStderr -Raw | Write-Host
    }
    exit 1
}

Remove-Item $TransStdout, $TransStderr -ErrorAction SilentlyContinue

Write-Host "Compiling $TransOut"

$GCC = "gcc.exe"
$CompileStdout = Join-Path $LogDir "compile.log"

$GccArgs = @("-std=c99","-Wall","-Wextra","-Wpedantic","-Werror=incompatible-pointer-types","-Werror=discarded-qualifiers","-Werror=implicit-function-declaration","-Werror=implicit-int","-I",".",$TransOut,"-o",$ExeOut)
$GccStderr = Join-Path $LogDir "compile_stderr.log"
$GccProc = Start-Process -FilePath $GCC -ArgumentList $GccArgs -Wait -PassThru -NoNewWindow -RedirectStandardOutput $CompileStdout -RedirectStandardError $GccStderr

if ($GccProc.ExitCode -ne 0) {
    Write-Host "ERROR: Compilation failed: exit $($GccProc.ExitCode)"
    Get-Content $CompileStdout | Write-Host
    exit 1
}

Write-Host "Running test $ExeOut"

$TestStdout = Join-Path $LogDir "test_stdout.log"

& $ExeOut 2>&1 | Tee-Object -FilePath $TestStdout

$TestExitCode = $LASTEXITCODE

if ($TestExitCode -eq 0) {
    $TestOutput = Get-Content $TestStdout -Raw
    Write-Host ""
    Write-Host "Test PASSED"
} else {
    Write-Host ""
    Write-Host "ERROR: Test failed: exit $TestExitCode"
    exit 1
}

# Generate run summary
$Summary = @"
Status: SUCCESS
Completed-At: $((Get-Date).ToUniversalTime().ToString("o"))
Compiler: $Compiler
Compiler-SHA256: $CompilerHash
TestSource: $TestSource
TestSource-SHA256: $((Get-FileHash -LiteralPath $TestSource -Algorithm SHA256).Hash)
Translation: SUCCESS
Compilation: SUCCESS (gcc exit 0)
Execution: SUCCESS (exit 0)
Compile-Log: $CompileStdout
Test-Output-Log: $TestStdout
Test-Result: $TestOutput
"@

$SummaryFile = Join-Path $LogDir "run_summary.log"
Set-Content -LiteralPath $SummaryFile -Value $Summary

Write-Host ""
Write-Host "Summary saved to $SummaryFile"
