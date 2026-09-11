# Targeted legacy EXEC OOM fixture only; one private binary, two executions.
# No full regression, translator rebuild, or colleague active-source build.
param([string]$CoreCommit = 'b89c01cfd22c3542df371a06774ed5b79b539ed0')
$ErrorActionPreference = 'Stop'
$baseline = Split-Path -Parent $PSScriptRoot
$repo = Split-Path -Parent (Split-Path -Parent $baseline)
$compiler = Join-Path $baseline 'build/l1trans/gen2/l1trans.exe'
$pin = '65D5A5ED127CA1BAEBDD1D500A5B74CEEA63EC1985EAC52EDEF28EFEB261C936'
if ((Get-FileHash -LiteralPath $compiler).Hash -ne $pin) { throw 'Stable compiler pin changed.' }
$testSource = Join-Path $PSScriptRoot 'lmx_message_exec_selftest.c'
$testHash = (Get-FileHash -LiteralPath $testSource).Hash
$runnerHash = (Get-FileHash -LiteralPath $PSCommandPath).Hash
$runId = (Get-Date -Format 'yyyyMMdd_HHmmss_fff') + '_' + [guid]::NewGuid().ToString('N').Substring(0,8)
$run = Join-Path $repo "build/codex/msg_exec_oom/$runId"
$snapshot = Join-Path $run 'source'
$headers = Join-Path $run 'headers'
New-Item -ItemType Directory -Path $snapshot, (Join-Path $headers 'l2src') -Force | Out-Null
$stageWorkingDir = $repo
$evidence = [ordered]@{ compiler = $compiler; compilerSHA256 = $pin; requestedCore = $CoreCommit; testSHA256 = $testHash; runnerSHA256 = $runnerHash; stages = @(); result = 'RUNNING' }
function Invoke-OomStage([string]$Name, [string]$Tool, [string[]]$NativeArgs, [int]$ExpectedExit = 0) {
    $quoted = ($NativeArgs | ForEach-Object { '"' + $_ + '"' }) -join ' '
    $stdout = Join-Path $run "$Name.stdout.txt"
    $stderr = Join-Path $run "$Name.stderr.txt"
    $p = Start-Process -FilePath $Tool -ArgumentList $quoted -WorkingDirectory $stageWorkingDir `
        -WindowStyle Hidden -PassThru -RedirectStandardOutput $stdout -RedirectStandardError $stderr
    # The process handle belongs to this invocation in a unique private directory.
    # Only this newly launched process is terminated if its bounded stage hangs.
    $timedOut = -not $p.WaitForExit(30000)
    if ($timedOut) { $p.Kill(); $p.WaitForExit() }
    $p.Refresh()
    $evidence.stages += [ordered]@{ name = $Name; tool = $Tool; arguments = $NativeArgs; exit = $p.ExitCode; expectedExit = $ExpectedExit; timedOut = $timedOut }
    if ($timedOut) { throw "$Name exceeded its 30-second stage timeout." }
    if ($p.ExitCode -ne $ExpectedExit) {
        Get-Content -LiteralPath $stdout, $stderr
        throw "$Name failed with exit $($p.ExitCode)"
    }
}
try {
    $git = (Get-Command git -ErrorAction Stop).Source
    Invoke-OomStage 'resolve_core' $git @('rev-parse', '--verify', '--end-of-options', "$CoreCommit^{commit}")
    $revision = (Get-Content -LiteralPath (Join-Path $run 'resolve_core.stdout.txt') -Raw).Trim()
    if ($revision -notmatch '^[0-9a-f]{40}$') { throw 'Expected a resolved full commit hash.' }
    $evidence.coreCommit = $revision
    $files = @('lmx.h', 'lmx_message.h', 'lmx_message.lm1', 'lmx_message_host.h',
        'lmx_message_host.c', 'lmx_message_exec.h', 'lmx_message_exec.c',
        'lmx_msg_blocks.h.lm1', 'lmx_msg_blocks.lm1', 'lmx_owned_ranges.h.lm1',
        'lmx_owned_ranges.lm1', 'lmx_msg_storage.h.lm1', 'lmx_msg_storage.lm1',
        'lmx_msg_path_storage.h.lm1', 'lmx_msg_path_storage.lm1',
        'lmx_msg_slots.h.lm1', 'lmx_msg_slots.lm1')
    $paths = @($files | ForEach-Object { "stg/l1_baseline/l2src/$_" })
    $archive = Join-Path $run 'core.zip'
    Invoke-OomStage 'archive_core' $git (@('archive', '--format=zip', "--output=$archive", $revision, '--') + $paths)
    Expand-Archive -LiteralPath $archive -DestinationPath $snapshot
    $stageWorkingDir = Join-Path $snapshot 'stg/l1_baseline'
    $coreHashes = @{}
    foreach ($file in $files) {
        $path = Join-Path $stageWorkingDir "l2src/$file"
        $coreHashes[$path] = (Get-FileHash -LiteralPath $path).Hash
    }
    $evidence.coreSources = $coreHashes
    $modules = @()
    foreach ($name in @('lmx_msg_blocks', 'lmx_owned_ranges', 'lmx_msg_storage', 'lmx_msg_path_storage', 'lmx_msg_slots')) {
        Invoke-OomStage "header_$name" $compiler @("l2src/$name.h.lm1", (Join-Path $headers "l2src/$name.lm1.h"))
        $module = Join-Path $run "$name.c"
        Invoke-OomStage "module_$name" $compiler @("l2src/$name.lm1", $module)
        $modules += $module
    }
    $messageC = Join-Path $run 'lmx_message.c'
    Invoke-OomStage 'message' $compiler @('l2src/lmx_message.lm1', $messageC)
    $testC = Join-Path $run 'lmx_message_exec_selftest.c'
    Copy-Item -LiteralPath $testSource -Destination $testC
    $gcc = (Get-Command gcc -ErrorAction Stop).Source
    Invoke-OomStage 'gcc_version' $gcc @('--version')
    $flags = @('-std=c99', '-Wall', '-Wextra', '-Wpedantic', '-Werror=incompatible-pointer-types',
        '-Werror=discarded-qualifiers', '-Werror=implicit-function-declaration', '-Werror=implicit-int',
        '-I', $headers, '-I', $stageWorkingDir)
    $native = @((Join-Path $stageWorkingDir 'l2src/lmx_message_host.c'), (Join-Path $stageWorkingDir 'l2src/lmx_message_exec.c'))
    $exe = Join-Path $run 'exec_oom.exe'
    Invoke-OomStage 'compile_exec_oom' $gcc ($flags + @('-O0', '-DLMX_MSG_EXEC_TEST', $testC, $messageC) + $modules + $native + @('-o', $exe))
    Invoke-OomStage 'forced_cleanup_failure' $exe @('--oom-cleanup-failure') 1
    $forcedOut = Get-Content -LiteralPath (Join-Path $run 'forced_cleanup_failure.stdout.txt') -Raw
    $forcedErr = Get-Content -LiteralPath (Join-Path $run 'forced_cleanup_failure.stderr.txt') -Raw
    if ($forcedErr -notmatch 'forced oom cleanup failure' -or $forcedErr -notmatch 'oom cleanup complete failed=1') { throw 'Missing forced failure/cleanup evidence.' }
    if ($forcedOut -match 'exec oom-only ok' -or $forcedErr -match '(?m)^boot$|mass 70|cpu-busy start') { throw 'Forced mode ran unrelated scenarios or claimed success.' }
    Invoke-OomStage 'normal_oom' $exe @('--oom-only')
    $result = Get-Content -LiteralPath (Join-Path $run 'normal_oom.stdout.txt') -Raw
    $normalErr = Get-Content -LiteralPath (Join-Path $run 'normal_oom.stderr.txt') -Raw
    if ($result -notmatch '(?m)^exec oom-only ok count=12 hits=[1-9][0-9]* scan=1 fifo=10,20\s*$') { throw "Unexpected OOM result: $result" }
    if ($normalErr -notmatch 'oom cleanup complete failed=0' -or $normalErr -match '(?m)^boot$|mass 70|cpu-busy start') { throw 'Normal targeted cleanup/scope mismatch.' }
    Write-Output 'forced cleanup: expected exit 1, cleanup completed without timeout'
    Write-Output $result.Trim()
    foreach ($path in $coreHashes.Keys) {
        if ((Get-FileHash -LiteralPath $path).Hash -ne $coreHashes[$path]) { throw "Snapshot source changed: $path" }
    }
    if ((Get-FileHash -LiteralPath $compiler).Hash -ne $pin) { throw 'Compiler changed during verification.' }
    if ((Get-FileHash -LiteralPath $testSource).Hash -ne $testHash -or (Get-FileHash -LiteralPath $PSCommandPath).Hash -ne $runnerHash) { throw 'Test or runner changed during verification.' }
    $evidence.artifacts = @{}
    Get-ChildItem -LiteralPath $run -File | ForEach-Object { $evidence.artifacts[$_.Name] = (Get-FileHash -LiteralPath $_.FullName).Hash }
    $evidence.result = 'PASS'
} catch {
    $evidence.result = 'FAIL'
    $evidence.error = $_.Exception.Message
    throw
} finally {
    $evidence | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath (Join-Path $run 'evidence.json') -Encoding UTF8
    Write-Output "Evidence: $run"
}
