# Calculator syntax selftest runner.
# Pattern read (not edited) from run_console_window_selftest.ps1: isolated run
# dir, stage exits, hashes, raw stdout/stderr separated, launch-failure guarded.

param()

$ErrorActionPreference = "Stop"

$RepoRoot = Split-Path -Parent $PSScriptRoot
$CompilerHash = "65D5A5ED127CA1BAEBDD1D500A5B74CEEA63EC1985EAC52EDEF28EFEB261C936"

$RunTimestamp = (Get-Date -Format "yyyyMMdd_HHmmss_fff")
$RunGuid = [GUID]::NewGuid().ToString().Substring(0, 8)
$BaseDir = Join-Path (Join-Path (Join-Path (Join-Path $RepoRoot "build") "mixa") "opencode") "calculator_syntax"
$RunDir = Join-Path $BaseDir "run_${RunTimestamp}_${RunGuid}"
$LogDir = Join-Path $RunDir "logs"
$HeaderDir = Join-Path (Join-Path $RunDir "headers") "mixa_manager"

New-Item -ItemType Directory -Path $LogDir -Force | Out-Null
New-Item -ItemType Directory -Path $HeaderDir -Force | Out-Null

$StatusFile = Join-Path $LogDir "status.txt"
$HashReport = Join-Path $LogDir "source_hashes.txt"
New-Item -ItemType File -Path $HashReport -Force | Out-Null

$Compiler = Join-Path $RepoRoot "stg\l1_baseline\build\l1trans\gen2\l1trans.exe"
if (-not (Test-Path -LiteralPath $Compiler)) {
    Set-Content -LiteralPath $StatusFile -Value "FAILED preflight: missing compiler $Compiler (no process launched)"
    Write-Output "MISSING COMPILER: $Compiler"
    Write-Output "Run dir: $RunDir"
    exit 1
}
$ActualCompilerHash = (Get-FileHash -LiteralPath $Compiler -Algorithm SHA256).Hash
Add-Content -LiteralPath $HashReport -Value "compiler: $ActualCompilerHash $Compiler"
if ($ActualCompilerHash -ne $CompilerHash) {
    Set-Content -LiteralPath $StatusFile -Value "FAILED preflight: compiler hash mismatch expected=$CompilerHash actual=$ActualCompilerHash (no process launched)"
    Write-Output "COMPILER HASH MISMATCH: expected $CompilerHash got $ActualCompilerHash"
    Write-Output "Run dir: $RunDir"
    exit 1
}

$HeaderSource = Join-Path $RepoRoot "mixa_manager\mixa_calculator_syntax.h.lm1"
$ModuleSource = Join-Path $RepoRoot "mixa_manager\mixa_calculator_syntax.lm1"
$TestSource = Join-Path $RepoRoot "mixa_manager\tests\mixa_calculator_syntax_selftest.lm1"
$RunnerSource = $PSCommandPath

Add-Content -LiteralPath $HashReport -Value "header_src: $((Get-FileHash -LiteralPath $HeaderSource -Algorithm SHA256).Hash) $HeaderSource"
Add-Content -LiteralPath $HashReport -Value "module_src: $((Get-FileHash -LiteralPath $ModuleSource -Algorithm SHA256).Hash) $ModuleSource"
Add-Content -LiteralPath $HashReport -Value "test_src:   $((Get-FileHash -LiteralPath $TestSource -Algorithm SHA256).Hash) $TestSource"
Add-Content -LiteralPath $HashReport -Value "runner_src: $((Get-FileHash -LiteralPath $RunnerSource -Algorithm SHA256).Hash) $RunnerSource"

$GccVersionLog = Join-Path $LogDir "gcc_version.log"
& gcc --version > $GccVersionLog 2>&1
Add-Content -LiteralPath $HashReport -Value "gcc_identity: $((Get-Content -LiteralPath $GccVersionLog -TotalCount 1))"

Set-Content -LiteralPath $StatusFile -Value "preflight OK"

function Invoke-Stage($Name, $FilePath, $ArgList, $WorkDir) {
    $StdOut = Join-Path $LogDir "${Name}_stdout.log"
    $StdErr = Join-Path $LogDir "${Name}_stderr.log"
    $ExitFile = Join-Path $LogDir "${Name}_exit.txt"
    $CmdFile = Join-Path $LogDir "${Name}_command.txt"
    Set-Content -LiteralPath $CmdFile -Value "tool: $FilePath`nargs: $($ArgList -join ' ')`ncwd: $WorkDir"
    try {
        $Proc = Start-Process -FilePath $FilePath -ArgumentList $ArgList -Wait -PassThru -NoNewWindow -WorkingDirectory $WorkDir -RedirectStandardOutput $StdOut -RedirectStandardError $StdErr -ErrorAction Stop
        $Rc = $Proc.ExitCode
        Set-Content -LiteralPath $ExitFile -Value $Rc
        return @{ Launched = $true; ExitCode = $Rc; Error = $null }
    } catch {
        $ExMsg = $_.Exception.Message
        Set-Content -LiteralPath $ExitFile -Value "NOT_LAUNCHED"
        Set-Content -LiteralPath $StdErr -Value "launch failure (process never started): $ExMsg"
        return @{ Launched = $false; ExitCode = $null; Error = $ExMsg }
    }
}

# Stage 1: translate header
$HeaderOut = Join-Path $HeaderDir "mixa_calculator_syntax.lm1.h"
Set-Content -LiteralPath $StatusFile -Value "stage header_trans: launching"
$Result = Invoke-Stage "header_trans" $Compiler @("mixa_manager\mixa_calculator_syntax.h.lm1", $HeaderOut) $RepoRoot
if (Test-Path -LiteralPath $HeaderOut) {
    Add-Content -LiteralPath $HashReport -Value "header_gen: $((Get-FileHash -LiteralPath $HeaderOut -Algorithm SHA256).Hash) $HeaderOut"
}
if (-not $Result.Launched) {
    Set-Content -LiteralPath $StatusFile -Value "FAILED stage header_trans: launch error: $($Result.Error)"
    Write-Output "STAGE LAUNCH FAILED: header_trans: $($Result.Error)"
    Write-Output "Run dir: $RunDir"
    exit 1
}
if ($Result.ExitCode -ne 0) {
    Set-Content -LiteralPath $StatusFile -Value "FAILED stage header_trans exit=$($Result.ExitCode)"
    Write-Output "STAGE FAILED: header_trans exit=$($Result.ExitCode)"
    Write-Output "Run dir: $RunDir"
    exit 1
}

# Stage 2: translate module
$ModuleC = Join-Path $RunDir "mixa_calculator_syntax.c"
Set-Content -LiteralPath $StatusFile -Value "stage module_trans: launching"
$Result = Invoke-Stage "module_trans" $Compiler @("mixa_manager\mixa_calculator_syntax.lm1", $ModuleC) $RepoRoot
if (Test-Path -LiteralPath $ModuleC) {
    Add-Content -LiteralPath $HashReport -Value "module_gen: $((Get-FileHash -LiteralPath $ModuleC -Algorithm SHA256).Hash) $ModuleC"
}
if (-not $Result.Launched) {
    Set-Content -LiteralPath $StatusFile -Value "FAILED stage module_trans: launch error: $($Result.Error)"
    Write-Output "STAGE LAUNCH FAILED: module_trans: $($Result.Error)"
    Write-Output "Run dir: $RunDir"
    exit 1
}
if ($Result.ExitCode -ne 0) {
    Set-Content -LiteralPath $StatusFile -Value "FAILED stage module_trans exit=$($Result.ExitCode)"
    Write-Output "STAGE FAILED: module_trans exit=$($Result.ExitCode)"
    Write-Output "Run dir: $RunDir"
    exit 1
}

# Stage 3: translate test (predefs header+module; single TU)
$TestC = Join-Path $RunDir "mixa_calculator_syntax_selftest.c"
Set-Content -LiteralPath $StatusFile -Value "stage test_trans: launching"
$Result = Invoke-Stage "test_trans" $Compiler @("mixa_manager\tests\mixa_calculator_syntax_selftest.lm1", $TestC) $RepoRoot
if (Test-Path -LiteralPath $TestC) {
    Add-Content -LiteralPath $HashReport -Value "test_gen: $((Get-FileHash -LiteralPath $TestC -Algorithm SHA256).Hash) $TestC"
}
if (-not $Result.Launched) {
    Set-Content -LiteralPath $StatusFile -Value "FAILED stage test_trans: launch error: $($Result.Error)"
    Write-Output "STAGE LAUNCH FAILED: test_trans: $($Result.Error)"
    Write-Output "Run dir: $RunDir"
    exit 1
}
if ($Result.ExitCode -ne 0) {
    Set-Content -LiteralPath $StatusFile -Value "FAILED stage test_trans exit=$($Result.ExitCode)"
    Write-Output "STAGE FAILED: test_trans exit=$($Result.ExitCode)"
    Write-Output "Run dir: $RunDir"
    exit 1
}

# Stage 4: compile
$Exe = Join-Path $RunDir "mixa_calculator_syntax_selftest.exe"
$CompileStdout = Join-Path $LogDir "compile_stdout.log"
$CompileStderr = Join-Path $LogDir "compile_stderr.log"
$CompileExitFile = Join-Path $LogDir "compile_exit.txt"
$CompileCmdFile = Join-Path $LogDir "compile_command.txt"
$GccTool = if ($env:MIXA_TEST_GCC_OVERRIDE) { $env:MIXA_TEST_GCC_OVERRIDE } else { "gcc" }
$GccArgs = @("-std=c99","-Wall","-Wextra","-Wpedantic","-Werror=incompatible-pointer-types","-I",".","-I",(Join-Path $RunDir "headers"),$TestC,"-o",$Exe)
Set-Content -LiteralPath $CompileCmdFile -Value "tool: $GccTool`nargs: $($GccArgs -join ' ')`ncwd: $RepoRoot"
Set-Content -LiteralPath $StatusFile -Value "stage compile: launching"
try {
    $CompileProc = Start-Process -FilePath $GccTool -ArgumentList $GccArgs -Wait -PassThru -NoNewWindow -WorkingDirectory $RepoRoot -RedirectStandardOutput $CompileStdout -RedirectStandardError $CompileStderr -ErrorAction Stop
    $CompileRc = $CompileProc.ExitCode
    Set-Content -LiteralPath $CompileExitFile -Value $CompileRc
} catch {
    $ExMsg = $_.Exception.Message
    Set-Content -LiteralPath $CompileExitFile -Value "NOT_LAUNCHED"
    Set-Content -LiteralPath $CompileStderr -Value "launch failure (process never started): $ExMsg"
    Set-Content -LiteralPath $StatusFile -Value "FAILED stage compile: launch error: $ExMsg"
    Write-Output "STAGE LAUNCH FAILED: compile (tool='$GccTool'): $ExMsg"
    Write-Output "Run dir: $RunDir"
    exit 1
}
if ($CompileRc -ne 0) {
    Set-Content -LiteralPath $StatusFile -Value "FAILED stage compile exit=$CompileRc"
    Write-Output "STAGE FAILED: compile exit=$CompileRc"
    Write-Output "--- compile stderr ---"
    Get-Content -LiteralPath $CompileStderr
    Write-Output "Run dir: $RunDir"
    exit 1
}
$ExeHash = (Get-FileHash -LiteralPath $Exe -Algorithm SHA256).Hash
Add-Content -LiteralPath $HashReport -Value "exe: $ExeHash $Exe"

# Stage 5: run
$RunStdout = Join-Path $LogDir "run_stdout.log"
$RunStderr = Join-Path $LogDir "run_stderr.log"
$RunExitFile = Join-Path $LogDir "run_exit.txt"
Set-Content -LiteralPath $StatusFile -Value "stage run: launching"
try {
    $RunProc = Start-Process -FilePath $Exe -Wait -PassThru -NoNewWindow -RedirectStandardOutput $RunStdout -RedirectStandardError $RunStderr -ErrorAction Stop
    $RunRc = $RunProc.ExitCode
    Set-Content -LiteralPath $RunExitFile -Value $RunRc
} catch {
    $ExMsg = $_.Exception.Message
    Set-Content -LiteralPath $RunExitFile -Value "NOT_LAUNCHED"
    Set-Content -LiteralPath $RunStderr -Value "launch failure (process never started): $ExMsg"
    Set-Content -LiteralPath $StatusFile -Value "FAILED stage run: launch error: $ExMsg"
    Write-Output "STAGE LAUNCH FAILED: run: $ExMsg"
    Write-Output "Run dir: $RunDir"
    exit 1
}
if ($RunRc -eq 0) {
    Set-Content -LiteralPath $StatusFile -Value "COMPLETED all stages exit=0"
} else {
    Set-Content -LiteralPath $StatusFile -Value "FAILED stage run exit=$RunRc"
}

Write-Output "--- run stdout ---"
Get-Content -LiteralPath $RunStdout
Write-Output "--- run stderr ---"
Get-Content -LiteralPath $RunStderr
Write-Output ""
Write-Output "Run dir: $RunDir"
Write-Output "Test exit: $RunRc"
exit $RunRc
