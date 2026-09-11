# Pure L1 mailbox chain helper. No native runtime or translator self-build.
param(
    [string]$CoreCommit = 'a5643e4d46a89479862944ba4a20a5621be65f94',
    [ValidateSet('O0', 'O2')][string[]]$Optimization = @('O2')
)
$ErrorActionPreference = 'Stop'
$baseline = Split-Path -Parent $PSScriptRoot
$repo = Split-Path -Parent (Split-Path -Parent $baseline)
$compiler = Join-Path $baseline 'build/l1trans/gen2/l1trans.exe'
$pin = '65D5A5ED127CA1BAEBDD1D500A5B74CEEA63EC1985EAC52EDEF28EFEB261C936'
if ((Get-FileHash -LiteralPath $compiler).Hash -ne $pin) { throw 'Stable compiler pin changed.' }
$runId = (Get-Date -Format 'yyyyMMdd_HHmmss_fff') + '_' + [guid]::NewGuid().ToString('N').Substring(0,8)
$run = Join-Path $repo "build/codex/msg_mail_chain/$runId"
$snapshot = Join-Path $run 'source'
$headers = Join-Path $run 'headers'
New-Item -ItemType Directory -Path $snapshot, (Join-Path $headers 'l2src') -Force | Out-Null
$ownNames = @('lmx_msg_mail_chain.h.lm1', 'lmx_msg_mail_chain.lm1', 'tests/lmx_msg_mail_chain_selftest.lm1')
$sources = @($ownNames | ForEach-Object { Join-Path $PSScriptRoot $_ }) + @($PSCommandPath)
$hashes = @{}
foreach ($file in $sources) { $hashes[$file] = (Get-FileHash -LiteralPath $file).Hash }
$stageWorkingDir = $repo
$evidence = [ordered]@{ compiler = $compiler; compilerSHA256 = $pin; requestedCore = $CoreCommit; optimization = $Optimization; sources = $hashes; stages = @(); result = 'RUNNING' }
function Invoke-ChainStage([string]$Name, [string]$Tool, [string[]]$NativeArgs) {
    $quoted = ($NativeArgs | ForEach-Object { '"' + $_ + '"' }) -join ' '
    $stdout = Join-Path $run "$Name.stdout.txt"
    $stderr = Join-Path $run "$Name.stderr.txt"
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
    Invoke-ChainStage 'resolve_core' $git @('rev-parse', '--verify', '--end-of-options', "$CoreCommit^{commit}")
    $revision = (Get-Content -LiteralPath (Join-Path $run 'resolve_core.stdout.txt') -Raw).Trim()
    if ($revision -notmatch '^[0-9a-f]{40}$') { throw 'Expected full commit hash.' }
    $evidence.coreCommit = $revision
    $coreFiles = @('lmx_message.h', 'lmx_msg_blocks.h.lm1', 'lmx_owned_ranges.h.lm1', 'lmx_msg_storage.h.lm1', 'lmx_msg_path_storage.h.lm1')
    $paths = @($coreFiles | ForEach-Object { "stg/l1_baseline/l2src/$_" })
    $archive = Join-Path $run 'core_headers.zip'
    Invoke-ChainStage 'archive_core' $git (@('archive', '--format=zip', "--output=$archive", $revision, '--') + $paths)
    Expand-Archive -LiteralPath $archive -DestinationPath $snapshot
    $stageWorkingDir = Join-Path $snapshot 'stg/l1_baseline'
    New-Item -ItemType Directory -Path (Join-Path $stageWorkingDir 'l2src/tests') -Force | Out-Null
    foreach ($name in $ownNames) {
        Copy-Item -LiteralPath (Join-Path $PSScriptRoot $name) -Destination (Join-Path $stageWorkingDir "l2src/$name")
    }
    $snapshotHashes = @{}
    foreach ($name in ($coreFiles + $ownNames)) {
        $file = Join-Path $stageWorkingDir "l2src/$name"
        $snapshotHashes[$file] = (Get-FileHash -LiteralPath $file).Hash
    }
    $evidence.snapshotSources = $snapshotHashes
    foreach ($name in @('lmx_msg_blocks', 'lmx_owned_ranges', 'lmx_msg_storage', 'lmx_msg_path_storage', 'lmx_msg_mail_chain')) {
        Invoke-ChainStage "header_$name" $compiler @("l2src/$name.h.lm1", (Join-Path $headers "l2src/$name.lm1.h"))
    }
    $module = Join-Path $run 'lmx_msg_mail_chain.c'
    $test = Join-Path $run 'lmx_msg_mail_chain_selftest.c'
    Invoke-ChainStage 'module' $compiler @('l2src/lmx_msg_mail_chain.lm1', $module)
    Invoke-ChainStage 'test' $compiler @('l2src/tests/lmx_msg_mail_chain_selftest.lm1', $test)
    $gcc = (Get-Command gcc -ErrorAction Stop).Source
    $nm = (Get-Command nm -ErrorAction Stop).Source
    $flags = @('-std=c99', '-Wall', '-Wextra', '-Wpedantic', '-Werror', '-I', $headers, '-I', $stageWorkingDir)
    Invoke-ChainStage 'gcc_version' $gcc @('--version')
    foreach ($level in @($Optimization | Select-Object -Unique)) {
        $obj = Join-Path $run "mail_chain_$level.o"
        $exe = Join-Path $run "mail_chain_$level.exe"
        Invoke-ChainStage "object_$level" $gcc ($flags + @("-$level", '-c', $module, '-o', $obj))
        Invoke-ChainStage "imports_$level" $nm @('-u', $obj)
        if (-not [string]::IsNullOrWhiteSpace((Get-Content -LiteralPath (Join-Path $run "imports_$level.stdout.txt") -Raw))) { throw 'Production helper has unexpected imports.' }
        Invoke-ChainStage "symbols_$level" $nm @('--defined-only', $obj)
        $symbols = Get-Content -LiteralPath (Join-Path $run "symbols_$level.stdout.txt") -Raw
        if ($symbols -match '(?m)^\S+\s+[BbCcDdGgSs]\s+(?!\.)\S+') { throw 'Mutable data in production helper.' }
        Invoke-ChainStage "compile_$level" $gcc ($flags + @("-$level", $obj, $test, '-o', $exe))
        Invoke-ChainStage "run_$level" $exe @()
        $result = Get-Content -LiteralPath (Join-Path $run "run_$level.stdout.txt") -Raw
        if ($result -notmatch '(?m)^mail_chain checks=26 failures=0\s*$') { throw "Unexpected result: $result" }
        Write-Output "$level $($result.Trim())"
    }
    foreach ($file in $sources) {
        if ((Get-FileHash -LiteralPath $file).Hash -ne $hashes[$file]) { throw "Source changed: $file" }
    }
    foreach ($file in $snapshotHashes.Keys) {
        if ((Get-FileHash -LiteralPath $file).Hash -ne $snapshotHashes[$file]) { throw "Snapshot changed: $file" }
    }
    if ((Get-FileHash -LiteralPath $compiler).Hash -ne $pin) { throw 'Compiler changed.' }
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
