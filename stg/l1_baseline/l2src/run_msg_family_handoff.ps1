# Public-API L1 integration test against an immutable private source snapshot.
# It never compiles Grok's active working files or writes shared build outputs.
param(
    [string]$CoreCommit = '57b590f4f30636fc49ddc0f44afdd2a2ebcab186',
    [ValidateNotNullOrEmpty()][ValidateSet('O0', 'O2')][string[]]$Optimization = @('O2'),
    [ValidateSet('Family', 'UnrootedAdopt')][string]$Scenario = 'Family'
)
$ErrorActionPreference = 'Stop'
$baseline = Split-Path -Parent $PSScriptRoot
$repo = Split-Path -Parent (Split-Path -Parent $baseline)
$compiler = Join-Path $baseline 'build/l1trans/gen2/l1trans.exe'
$pin = '65D5A5ED127CA1BAEBDD1D500A5B74CEEA63EC1985EAC52EDEF28EFEB261C936'
if ((Get-FileHash -LiteralPath $compiler).Hash -ne $pin) { throw 'Stable compiler pin changed.' }
$testSource = Join-Path $PSScriptRoot 'tests/lmx_msg_family_handoff_selftest.lm1'
if ($Scenario -eq 'UnrootedAdopt') {
    $testSource = Join-Path $PSScriptRoot 'tests/lmx_msg_adopt_unrooted_selftest.lm1'
}
$testHash = (Get-FileHash -LiteralPath $testSource).Hash
$runnerHash = (Get-FileHash -LiteralPath $PSCommandPath).Hash
$runId = (Get-Date -Format 'yyyyMMdd_HHmmss_fff') + '_' + [guid]::NewGuid().ToString('N').Substring(0,8)
$run = Join-Path $repo "build/codex/msg_family_handoff/$runId"
$snapshot = Join-Path $run 'source'
$headers = Join-Path $run 'headers'
New-Item -ItemType Directory -Path $snapshot, (Join-Path $headers 'l2src') -Force | Out-Null
$stageWorkingDir = $repo
$evidence = [ordered]@{ compiler = $compiler; compilerSHA256 = $pin; requestedCore = $CoreCommit; optimization = $Optimization; scenario = $Scenario; testSource = $testSource; testSHA256 = $testHash; runnerSHA256 = $runnerHash; stages = @(); result = 'RUNNING' }
function Invoke-FamilyStage([string]$Name, [string]$Tool, [string[]]$NativeArgs) {
    $quoted = ($NativeArgs | ForEach-Object { '"' + $_ + '"' }) -join ' '
    $stdout = Join-Path $run "$Name.stdout.txt"
    $stderr = Join-Path $run "$Name.stderr.txt"
    # Windows PowerShell rejects an explicitly empty ArgumentList.
    $argumentOption = @{}
    if ($NativeArgs.Count -gt 0) { $argumentOption.ArgumentList = $quoted }
    $p = Start-Process -FilePath $Tool @argumentOption -WorkingDirectory $stageWorkingDir `
        -WindowStyle Hidden -Wait -PassThru -RedirectStandardOutput $stdout -RedirectStandardError $stderr
    $evidence.stages += [ordered]@{ name = $Name; tool = $Tool; arguments = $NativeArgs; exit = $p.ExitCode }
    if ($p.ExitCode -ne 0) {
        Get-Content -LiteralPath $stdout, $stderr
        throw "$Name failed with exit $($p.ExitCode)"
    }
}
try {
    $git = (Get-Command git -ErrorAction Stop).Source
    Invoke-FamilyStage 'resolve_core' $git @('rev-parse', '--verify', '--end-of-options', "$CoreCommit^{commit}")
    $revision = (Get-Content -LiteralPath (Join-Path $run 'resolve_core.stdout.txt') -Raw).Trim()
    if ($revision -notmatch '^[0-9a-f]{40}$') { throw 'Expected a resolved full commit hash.' }
    $evidence.coreCommit = $revision
    $files = @('lmx.h', 'lmx_message.h', 'lmx_message.lm1', 'lmx_message_host.h',
        'lmx_message_host.c', 'lmx_message_exec.h', 'lmx_message_exec.c',
        'lmx_msg_blocks.h.lm1', 'lmx_msg_blocks.lm1', 'lmx_owned_ranges.h.lm1',
        'lmx_owned_ranges.lm1', 'lmx_msg_storage.h.lm1', 'lmx_msg_storage.lm1',
        'lmx_msg_path_storage.h.lm1', 'lmx_msg_path_storage.lm1',
        'lmx_msg_slots.h.lm1', 'lmx_msg_slots.lm1',
        'lmx_msg_mail_chain.h.lm1', 'lmx_msg_mail_chain.lm1',
        'lmx_msg_sched_ready.h.lm1', 'lmx_msg_sched_ready.lm1')
    $paths = @($files | ForEach-Object { "stg/l1_baseline/l2src/$_" })
    $archive = Join-Path $run 'core.zip'
    Invoke-FamilyStage 'archive_core' $git (@('archive', '--format=zip', "--output=$archive", $revision, '--') + $paths)
    Expand-Archive -LiteralPath $archive -DestinationPath $snapshot
    $stageWorkingDir = Join-Path $snapshot 'stg/l1_baseline'
    $coreHashes = @{}
    foreach ($file in $files) {
        $path = Join-Path $stageWorkingDir "l2src/$file"
        $coreHashes[$path] = (Get-FileHash -LiteralPath $path).Hash
    }
    $evidence.coreSources = $coreHashes
    $modules = @()
    foreach ($name in @('lmx_msg_blocks', 'lmx_owned_ranges', 'lmx_msg_storage', 'lmx_msg_path_storage', 'lmx_msg_slots', 'lmx_msg_mail_chain', 'lmx_msg_sched_ready')) {
        Invoke-FamilyStage "header_$name" $compiler @("l2src/$name.h.lm1", (Join-Path $headers "l2src/$name.lm1.h"))
        $module = Join-Path $run "$name.c"
        Invoke-FamilyStage "module_$name" $compiler @("l2src/$name.lm1", $module)
        $modules += $module
    }
    $messageC = Join-Path $run 'lmx_message.c'
    Invoke-FamilyStage 'message' $compiler @('l2src/lmx_message.lm1', $messageC)
    $testC = Join-Path $run 'family_handoff.c'
    Invoke-FamilyStage 'test' $compiler @($testSource, $testC)
    $gcc = (Get-Command gcc -ErrorAction Stop).Source
    Invoke-FamilyStage 'gcc_version' $gcc @('--version')
    $flags = @('-std=c99', '-Wall', '-Wextra', '-Wpedantic', '-Werror=incompatible-pointer-types',
        '-Werror=discarded-qualifiers', '-Werror=implicit-function-declaration', '-Werror=implicit-int',
        '-I', $headers, '-I', $stageWorkingDir)
    $native = @((Join-Path $stageWorkingDir 'l2src/lmx_message_host.c'), (Join-Path $stageWorkingDir 'l2src/lmx_message_exec.c'))
    foreach ($level in @($Optimization | ForEach-Object { $_.ToUpperInvariant() } | Select-Object -Unique)) {
        $exe = Join-Path $run "family_handoff_$level.exe"
        $testObj = Join-Path $run "family_handoff_$level.o"
        Invoke-FamilyStage "compile_test_$level" $gcc ($flags + @('-Werror', "-$level", '-c', $testC, '-o', $testObj))
        Invoke-FamilyStage "compile_$level" $gcc ($flags + @("-$level", $testObj, $messageC) + $modules + $native + @('-Wl,--wrap=free', '-o', $exe))
        Invoke-FamilyStage "run_$level" $exe @()
        $result = Get-Content -LiteralPath (Join-Path $run "run_$level.stdout.txt") -Raw
        $expected = '(?m)^family handoff checks=53 failures=0 watched_frees=3\s*$'
        if ($Scenario -eq 'UnrootedAdopt') {
            $expected = '(?m)^adopt unrooted checks=\d+ failures=0 owned_frees=1\s*$'
        }
        if ($result -notmatch $expected) { throw "Unexpected test result: $result" }
        Write-Output "$level $($result.Trim())"
    }
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
