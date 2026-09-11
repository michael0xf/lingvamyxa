# FILE_SEAM section 4 dir seam selftest runner (Windows).
# Translator: stable stg/l1_baseline/build/l1trans/gen2/l1trans.exe (full SHA256).
$ErrorActionPreference = "Stop"
Set-Location (Join-Path $PSScriptRoot "..")

$RepoRoot = (Get-Location).Path
$CompilerHash = "65D5A5ED127CA1BAEBDD1D500A5B74CEEA63EC1985EAC52EDEF28EFEB261C936"

$RunTimestamp = (Get-Date -Format "yyyyMMdd_HHmmss_fff")
$RunGuid = [GUID]::NewGuid().ToString().Substring(0, 8)
$BaseDir = Join-Path (Join-Path (Join-Path (Join-Path $RepoRoot "build") "mixa") "opencode") "dir_seam"
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

$Win32HeaderSource = Join-Path $RepoRoot "mixa_manager\mixa_dir_win32.h.lm1"
$PortableHeaderSource = Join-Path $RepoRoot "mixa_manager\mixa_dir.h.lm1"
$ModuleSource = Join-Path $RepoRoot "mixa_manager\mixa_dir_win32.lm1"
$TestSource = Join-Path $RepoRoot "mixa_manager\tests\mixa_dir_selftest.lm1"
$RunnerSource = $PSCommandPath

Add-Content -LiteralPath $HashReport -Value "win32_header_src: $((Get-FileHash -LiteralPath $Win32HeaderSource -Algorithm SHA256).Hash) $Win32HeaderSource"
Add-Content -LiteralPath $HashReport -Value "portable_header_src: $((Get-FileHash -LiteralPath $PortableHeaderSource -Algorithm SHA256).Hash) $PortableHeaderSource"
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

# Stage 1a: translate win32 concrete header (defines struct MixaDir)
$Win32HeaderOut = Join-Path $HeaderDir "mixa_dir_win32.lm1.h"
Set-Content -LiteralPath $StatusFile -Value "stage win32_header_trans: launching"
$Result = Invoke-Stage "win32_header_trans" $Compiler @("mixa_manager\mixa_dir_win32.h.lm1", $Win32HeaderOut) $RepoRoot
if (Test-Path -LiteralPath $Win32HeaderOut) {
    Add-Content -LiteralPath $HashReport -Value "win32_header_gen: $((Get-FileHash -LiteralPath $Win32HeaderOut -Algorithm SHA256).Hash) $Win32HeaderOut"
}
if (-not $Result.Launched) {
    Set-Content -LiteralPath $StatusFile -Value "FAILED stage win32_header_trans: launch error: $($Result.Error)"
    Write-Output "STAGE LAUNCH FAILED: win32_header_trans: $($Result.Error)"
    Write-Output "Run dir: $RunDir"
    exit 1
}
if ($Result.ExitCode -ne 0) {
    Set-Content -LiteralPath $StatusFile -Value "FAILED stage win32_header_trans exit=$($Result.ExitCode)"
    Write-Output "STAGE FAILED: win32_header_trans exit=$($Result.ExitCode)"
    Write-Output "Run dir: $RunDir"
    exit 1
}

# Stage 1b: translate portable header (prototypes; references MixaDir opaquely)
$PortableHeaderOut = Join-Path $HeaderDir "mixa_dir.lm1.h"
Set-Content -LiteralPath $StatusFile -Value "stage portable_header_trans: launching"
$Result = Invoke-Stage "portable_header_trans" $Compiler @("mixa_manager\mixa_dir.h.lm1", $PortableHeaderOut) $RepoRoot
if (Test-Path -LiteralPath $PortableHeaderOut) {
    Add-Content -LiteralPath $HashReport -Value "portable_header_gen: $((Get-FileHash -LiteralPath $PortableHeaderOut -Algorithm SHA256).Hash) $PortableHeaderOut"
}
if (-not $Result.Launched) {
    Set-Content -LiteralPath $StatusFile -Value "FAILED stage portable_header_trans: launch error: $($Result.Error)"
    Write-Output "STAGE LAUNCH FAILED: portable_header_trans: $($Result.Error)"
    Write-Output "Run dir: $RunDir"
    exit 1
}
if ($Result.ExitCode -ne 0) {
    Set-Content -LiteralPath $StatusFile -Value "FAILED stage portable_header_trans exit=$($Result.ExitCode)"
    Write-Output "STAGE FAILED: portable_header_trans exit=$($Result.ExitCode)"
    Write-Output "Run dir: $RunDir"
    exit 1
}

# Stage 2: translate test (predefs both headers + module; single TU)
$TestC = Join-Path $RunDir "mixa_dir_selftest.c"
Set-Content -LiteralPath $StatusFile -Value "stage test_trans: launching"
$Result = Invoke-Stage "test_trans" $Compiler @("mixa_manager\tests\mixa_dir_selftest.lm1", $TestC) $RepoRoot
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

# Stage 3: compile
$Exe = Join-Path $RunDir "mixa_dir_selftest.exe"
$CompileStdout = Join-Path $LogDir "compile_stdout.log"
$CompileStderr = Join-Path $LogDir "compile_stderr.log"
$CompileExitFile = Join-Path $LogDir "compile_exit.txt"
$CompileCmdFile = Join-Path $LogDir "compile_command.txt"
$GccTool = if ($env:MIXA_TEST_GCC_OVERRIDE) { $env:MIXA_TEST_GCC_OVERRIDE } else { "gcc" }
$GccArgs = @("-std=c99","-Wall","-Wextra","-Wpedantic","-Werror=incompatible-pointer-types","-Werror=discarded-qualifiers","-Werror=implicit-function-declaration","-Werror=implicit-int","-I",".","-I",(Join-Path $RunDir "headers"),$TestC,"-o",$Exe)
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

# Stage 4: run (clean fixture dir first so reruns are deterministic)
$FixtureDir = Join-Path $RepoRoot "build\mixa\tmp_dir_seam"
if (Test-Path -LiteralPath $FixtureDir) {
    Remove-Item -LiteralPath $FixtureDir -Recurse -Force -ErrorAction Stop
}
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