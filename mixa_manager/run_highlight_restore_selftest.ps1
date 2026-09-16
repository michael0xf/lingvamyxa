# run_highlight_restore_selftest.ps1
# Isolated runner for mixa_highlight_restore_selftest
# Output and logs are placed under build/mixa/antigravity/highlight_restore/
#
# Parameters (defaulting to stable/current tools; override for failure-injection testing):
#   -TranslatorPath  : absolute path to the L1 translator binary
#   -GccPath         : absolute path to the gcc binary

param(
    [string]$TranslatorPath = "",
    [string]$GccPath = ""
)

$ErrorActionPreference = "Stop"
Set-Location (Join-Path $PSScriptRoot "..")

$artDir = "build\mixa\antigravity\highlight_restore"
$logDir = Join-Path $artDir "logs"
$negDir = Join-Path $artDir "negative"

# Create log directory first so we can always write summaries
New-Item -ItemType Directory -Force -Path $artDir, $logDir, $negDir | Out-Null

$src = "mixa_manager\tests\mixa_highlight_restore_selftest.lm1"
$c = Join-Path $artDir "mixa_highlight_restore_selftest.c"
$exe = Join-Path $artDir "mixa_highlight_restore_selftest.exe"
$compileLog = Join-Path $logDir "compile.log"
$testLog = Join-Path $logDir "test_stdout.log"
$runSummaryLog = Join-Path $logDir "run_summary.log"

$expectedHash = "65D5A5ED127CA1BAEBDD1D500A5B74CEEA63EC1985EAC52EDEF28EFEB261C936"
$defaultTrans = "stg\l1_baseline\build\l1trans\gen2\l1trans.exe"
$trans = if ($TranslatorPath -ne "") { $TranslatorPath } else { $defaultTrans }

# Initialize stage tracking before try
$currentStage = "preflight"
$failureExitCode = 1

# Helper: write FAILED summary
function Write-FailedSummary([string]$stage, [int]$exitCode, [string]$detail,
    [string]$transHash, [string]$srcHash, [string]$rnrHash, [string]$hlLm1Hash, [string]$hlHHash) {
    $ts = [DateTime]::UtcNow.ToString("o")
    $text = "Status: FAILED`nFailed-At: $ts`nStage: $stage`nExit-Code: $exitCode`nTranslator: $trans`nCompiler-SHA256: $transHash`nTestSource-SHA256: $srcHash`nRunner-SHA256: $rnrHash`nMixaHighlight-LM1-SHA256: $hlLm1Hash`nMixaHighlight-H-SHA256: $hlHHash`nDetail: $detail`n"
    [System.IO.File]::WriteAllText($runSummaryLog, $text, [System.Text.UTF8Encoding]::new($false))
}

# ----- Stage: preflight -----
# Write IN_PROGRESS immediately (before any hashing that might fail)
$startTimeUtc = [DateTime]::UtcNow.ToString("o")
$initText = "Status: IN_PROGRESS`nStage: preflight`nStarted-At: $startTimeUtc`nTranslator: $trans`n"
[System.IO.File]::WriteAllText($runSummaryLog, $initText, [System.Text.UTF8Encoding]::new($false))
[System.IO.File]::WriteAllText($compileLog, "pending`n", [System.Text.UTF8Encoding]::new($false))
[System.IO.File]::WriteAllText($testLog, "pending`n", [System.Text.UTF8Encoding]::new($false))

$transHash = "unknown"
$testSrcHash = "unknown"
$runnerHash = "unknown"
$hlLm1Hash = "unknown"
$hlHHash = "unknown"

try {
    # Resolve and verify translator
    if (-not (Test-Path -LiteralPath $trans)) {
        $failureExitCode = 1
        throw "Translator not found: $trans"
    }
    $transHash = (Get-FileHash -LiteralPath $trans -Algorithm SHA256).Hash
    Write-Host "Translator: $trans"
    Write-Host "Translator SHA256: $transHash"
    if ($transHash -ne $expectedHash) {
        $failureExitCode = 1
        throw "Compiler hash mismatch: expected $expectedHash got $transHash"
    }

    # Verify gcc can be launched
    $gccExePath = if ($GccPath -ne "") { $GccPath } else { "" }
    if ($gccExePath -eq "") {
        $gccCmd = Get-Command -Name gcc -CommandType Application -ErrorAction Stop
        $gccExePath = $gccCmd.Source
    } else {
        if (-not (Test-Path -LiteralPath $gccExePath)) {
            $failureExitCode = 1
            throw "gcc not found at: $gccExePath"
        }
    }
    Write-Host "gcc: $gccExePath"

    # Hash inputs
    $testSrcHash = (Get-FileHash -LiteralPath $src -Algorithm SHA256).Hash
    $runnerHash  = (Get-FileHash -LiteralPath "mixa_manager\run_highlight_restore_selftest.ps1" -Algorithm SHA256).Hash
    $hlLm1Hash   = (Get-FileHash -LiteralPath "mixa_manager\mixa_highlight.lm1" -Algorithm SHA256).Hash
    $hlHHash     = (Get-FileHash -LiteralPath "mixa_manager\mixa_highlight.h"   -Algorithm SHA256).Hash

    # Update IN_PROGRESS with known hashes
    $inprog = "Status: IN_PROGRESS`nStage: translation`nStarted-At: $startTimeUtc`nTranslator: $trans`nCompiler-SHA256: $transHash`nTestSource-SHA256: $testSrcHash`nRunner-SHA256: $runnerHash`nMixaHighlight-LM1-SHA256: $hlLm1Hash`nMixaHighlight-H-SHA256: $hlHHash`n"
    [System.IO.File]::WriteAllText($runSummaryLog, $inprog, [System.Text.UTF8Encoding]::new($false))

    # ----- Stage: translation -----
    $currentStage = "translation"
    Write-Host "Translating $src -> $c"
    $transProc = Start-Process -FilePath $trans -ArgumentList $src, $c -Wait -PassThru -NoNewWindow -RedirectStandardOutput "$logDir\_trans_stdout.tmp" -RedirectStandardError "$logDir\_trans_stderr.tmp"
    $transRc = $transProc.ExitCode
    if ($transRc -ne 0) {
        $failureExitCode = $transRc
        $transErr = Get-Content "$logDir\_trans_stderr.tmp" -Raw -ErrorAction SilentlyContinue
        if ($null -eq $transErr) { $transErrText = "" } else { $transErrText = $transErr }
        throw "Translation failed: exit $transRc"
    }
    Remove-Item "$logDir\_trans_stdout.tmp","$logDir\_trans_stderr.tmp" -ErrorAction SilentlyContinue

    # ----- Stage: compilation -----
    $currentStage = "compilation"
    $guards = @("-Werror=incompatible-pointer-types","-Werror=discarded-qualifiers","-Werror=implicit-function-declaration","-Werror=implicit-int")
    Write-Host "Compiling $c -> $exe"

    [System.IO.File]::WriteAllText($runSummaryLog, ($inprog -replace "Stage: translation","Stage: compilation"), [System.Text.UTF8Encoding]::new($false))

    $gccArgs = @("-std=c99","-Wall","-Wextra","-Wpedantic") + $guards + @("-I",".",$c,"-o",$exe)
    $gccProc = Start-Process -FilePath $gccExePath -ArgumentList $gccArgs -Wait -PassThru -NoNewWindow -RedirectStandardOutput "$logDir\_gcc_stdout.tmp" -RedirectStandardError "$logDir\_gcc_stderr.tmp"
    $gccRc = $gccProc.ExitCode
    $gccStderr = Get-Content "$logDir\_gcc_stderr.tmp" -Raw -ErrorAction SilentlyContinue
    $gccStdout = Get-Content "$logDir\_gcc_stdout.tmp" -Raw -ErrorAction SilentlyContinue
    if ($null -eq $gccStdout) { $gccStdoutStr = "" } else { $gccStdoutStr = $gccStdout }
    if ($null -eq $gccStderr) { $gccStderrStr = "" } else { $gccStderrStr = $gccStderr }
    $compileText = ($gccStdoutStr + $gccStderrStr).Trim()
    if ($compileText -eq "") { $compileText = "gcc compiled with 0 warnings/errors" }
    [System.IO.File]::WriteAllText($compileLog, $compileText + "`n", [System.Text.UTF8Encoding]::new($false))
    Remove-Item "$logDir\_gcc_stdout.tmp","$logDir\_gcc_stderr.tmp" -ErrorAction SilentlyContinue
    if ($compileText -ne "gcc compiled with 0 warnings/errors") { Write-Host $compileText }

    if ($gccRc -ne 0) {
        $failureExitCode = $gccRc
        throw "gcc failed: exit $gccRc (see $compileLog)"
    }

    # ----- Stage: execution -----
    $currentStage = "execution"
    Write-Host "Running test $exe"
    [System.IO.File]::WriteAllText($runSummaryLog, ($inprog -replace "Stage: translation","Stage: execution"), [System.Text.UTF8Encoding]::new($false))

    $testProc = Start-Process -FilePath $exe -Wait -PassThru -NoNewWindow -RedirectStandardOutput "$logDir\_test_stdout.tmp" -RedirectStandardError "$logDir\_test_stderr.tmp"
    $testRc = $testProc.ExitCode
    $testStdout = Get-Content "$logDir\_test_stdout.tmp" -Raw -ErrorAction SilentlyContinue
    $testStderr = Get-Content "$logDir\_test_stderr.tmp" -Raw -ErrorAction SilentlyContinue
    if ($null -eq $testStdout) { $testStdoutStr = "" } else { $testStdoutStr = $testStdout }
    if ($null -eq $testStderr) { $testStderrStr = "" } else { $testStderrStr = $testStderr }
    $testText = ($testStdoutStr + $testStderrStr).Trim()
    [System.IO.File]::WriteAllText($testLog, $testText + "`n", [System.Text.UTF8Encoding]::new($false))
    Remove-Item "$logDir\_test_stdout.tmp","$logDir\_test_stderr.tmp" -ErrorAction SilentlyContinue
    Write-Host $testText

    if ($testRc -ne 0) {
        $failureExitCode = $testRc
        throw "Test failed: exit $testRc (see $testLog)"
    }

    # ----- SUCCESS -----
    $completedTimeUtc = [DateTime]::UtcNow.ToString("o")
    $resultLine = ""
    if (-not [System.String]::IsNullOrEmpty($testText)) {
        $lines = $testText.Split("`n")
        $matched = $lines | Where-Object { $_ -match "mixa_highlight_restore_selftest:" }
        if ($null -ne $matched) {
            if ($matched -is [array]) {
                $resultLine = $matched[-1].Trim()
            } else {
                $resultLine = $matched.Trim()
            }
        }
    }
    $successText = "Status: SUCCESS`nCompleted-At: $completedTimeUtc`nTranslator: $trans`nCompiler-SHA256: $transHash`nTestSource: $src`nTestSource-SHA256: $testSrcHash`nRunner-SHA256: $runnerHash`nMixaHighlight-LM1-SHA256: $hlLm1Hash`nMixaHighlight-H-SHA256: $hlHHash`nTranslation: SUCCESS`nCompilation: SUCCESS (gcc exit 0)`nExecution: SUCCESS (exit 0)`nCompile-Log: $compileLog`nTest-Output-Log: $testLog`nTest-Result: $resultLine`n"
    [System.IO.File]::WriteAllText($runSummaryLog, $successText, [System.Text.UTF8Encoding]::new($false))
    Write-Host ""
    Write-Host "All stages completed successfully."

} catch {
    $errMsg = $_.Exception.Message
    if ([System.String]::IsNullOrEmpty($errMsg)) { $errMsg = "$_" }
    # Use tracked stage and exit code, not overwriting with generic caught-exception
    Write-FailedSummary $currentStage $failureExitCode $errMsg $transHash $testSrcHash $runnerHash $hlLm1Hash $hlHHash
    # Save run_summary to negative directory for failure analysis
    $negSummaryPath = Join-Path $negDir "run_summary.log"
    Copy-Item -LiteralPath $runSummaryLog -Destination $negSummaryPath -ErrorAction Stop
    Write-Host "ERROR: $errMsg"
    exit $failureExitCode
}
