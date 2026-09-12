# Isolated L1 stale bookkeeping check; no scheduler/runtime build.
param()
$ErrorActionPreference = 'Stop'
$baseline = Split-Path -Parent $PSScriptRoot
$repo = Split-Path -Parent (Split-Path -Parent $baseline)
$compiler = Join-Path $baseline 'build/l1trans/gen2/l1trans.exe'
$pin = '65D5A5ED127CA1BAEBDD1D500A5B74CEEA63EC1985EAC52EDEF28EFEB261C936'
if ((Get-FileHash -LiteralPath $compiler).Hash -ne $pin) { throw 'Stable compiler pin changed.' }
$runId = (Get-Date -Format 'yyyyMMdd_HHmmss_fff') + '_' + [guid]::NewGuid().ToString('N').Substring(0,8)
$run = Join-Path $repo "build/codex/msg_roots_stale/$runId"
$headers = Join-Path $run 'headers'
New-Item -ItemType Directory -Path (Join-Path $headers 'l2src') -Force | Out-Null
$headerUnits = @('lmx_msg_blocks','lmx_owned_ranges','lmx_msg_storage','lmx_msg_path_storage','lmx_msg_roots_stale')
$sources = @($PSCommandPath, (Join-Path $PSScriptRoot 'lmx.h'), (Join-Path $PSScriptRoot 'lmx_message.h'),
    (Join-Path $PSScriptRoot 'lmx_owned_ranges.lm1'),
    (Join-Path $PSScriptRoot 'lmx_msg_roots_stale.lm1'),
    (Join-Path $PSScriptRoot 'tests/lmx_msg_roots_stale_selftest.lm1'))
foreach ($unit in $headerUnits) { $sources += Join-Path $PSScriptRoot "$unit.h.lm1" }
$hashes = @{}
foreach ($source in $sources) { $hashes[$source] = (Get-FileHash -LiteralPath $source).Hash }
$evidence = [ordered]@{ compiler=$compiler; compilerSHA256=$pin; sources=$hashes; optimization='O2'; stages=@(); result='RUNNING' }
function Invoke-StaleStage([string]$Name, [string]$Tool, [string[]]$NativeArgs) {
    $options = @{}
    if ($NativeArgs.Count -gt 0) { $options.ArgumentList = ($NativeArgs | ForEach-Object { '"' + $_ + '"' }) -join ' ' }
    $stdout = Join-Path $run "$Name.stdout.txt"
    $stderr = Join-Path $run "$Name.stderr.txt"
    $process = Start-Process -FilePath $Tool @options -WorkingDirectory $baseline -WindowStyle Hidden -Wait -PassThru -RedirectStandardOutput $stdout -RedirectStandardError $stderr
    $evidence.stages += [ordered]@{ name=$Name; tool=$Tool; arguments=$NativeArgs; exit=$process.ExitCode }
    if ($process.ExitCode -ne 0) {
        Get-Content -LiteralPath $stdout,$stderr
        throw "$Name failed: $($process.ExitCode)"
    }
}
try {
    foreach ($unit in $headerUnits) {
        Invoke-StaleStage "header_$unit" $compiler @("l2src/$unit.h.lm1", (Join-Path $headers "l2src/$unit.lm1.h"))
    }
    $module = Join-Path $run 'lmx_msg_roots_stale.c'
    $test = Join-Path $run 'lmx_msg_roots_stale_selftest.c'
    $ranges = Join-Path $run 'lmx_owned_ranges.c'
    Invoke-StaleStage 'module' $compiler @('l2src/lmx_msg_roots_stale.lm1',$module)
    Invoke-StaleStage 'test' $compiler @('l2src/tests/lmx_msg_roots_stale_selftest.lm1',$test)
    Invoke-StaleStage 'ranges' $compiler @('l2src/lmx_owned_ranges.lm1',$ranges)
    $gcc = (Get-Command gcc -ErrorAction Stop).Source
    $nm = (Get-Command nm -ErrorAction Stop).Source
    Invoke-StaleStage 'gcc_version' $gcc @('--version')
    $flags = @('-std=c99','-O2','-Wall','-Wextra','-Wpedantic','-Werror','-I',$headers,'-I',$baseline)
    $obj = Join-Path $run 'stale.o'
    $exe = Join-Path $run 'stale.exe'
    Invoke-StaleStage 'object' $gcc ($flags + @('-c',$module,'-o',$obj))
    Invoke-StaleStage 'imports' $nm @('-u',$obj)
    foreach ($line in (Get-Content -LiteralPath (Join-Path $run 'imports.stdout.txt'))) {
        if ($line.Trim() -and $line -notmatch '^\s*U\s+_?(free|lmx_owned_ranges_find)\s*$') { throw "Unexpected dependency: $line" }
    }
    Invoke-StaleStage 'symbols' $nm @('--defined-only',$obj)
    foreach ($line in (Get-Content -LiteralPath (Join-Path $run 'symbols.stdout.txt'))) {
        if ($line -match '^\s*[0-9a-fA-F]+\s+[bBdDgGsSC]\s+' -and $line -notmatch '\s+\.(bss|data|sbss|sdata)\s*$') { throw "Mutable module storage: $line" }
    }
    # Scope free instrumentation to the helper object, excluding CRT/fixture.
    $faultObj = Join-Path $run 'stale_fault.o'
    Invoke-StaleStage 'fault_object' $gcc ($flags + @('-Dfree=lmx_stale_test_free','-c',$module,'-o',$faultObj))
    Invoke-StaleStage 'compile' $gcc ($flags + @($faultObj,$ranges,$test,'-o',$exe))
    Invoke-StaleStage 'run' $exe @()
    $result = Get-Content -LiteralPath (Join-Path $run 'run.stdout.txt') -Raw
    if ($result -notmatch '(?m)^stale checks=\d+ failures=0 frees=8 mask=255\s*$') { throw "Unexpected result: $result" }
    Write-Output $result.Trim()
    foreach ($source in $sources) {
        if ((Get-FileHash -LiteralPath $source).Hash -ne $hashes[$source]) { throw "Source changed: $source" }
    }
    if ((Get-FileHash -LiteralPath $compiler).Hash -ne $pin) { throw 'Compiler changed during verification.' }
    $evidence.artifacts = @{}
    Get-ChildItem -LiteralPath $run -Recurse -File | ForEach-Object { $evidence.artifacts[$_.FullName] = (Get-FileHash -LiteralPath $_.FullName).Hash }
    $evidence.result = 'PASS'
} catch {
    $evidence.result = 'FAIL'
    $evidence.error = $_.Exception.Message
    throw
} finally {
    $evidence | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath (Join-Path $run 'evidence.json') -Encoding UTF8
    Write-Output "Evidence: $run"
}
