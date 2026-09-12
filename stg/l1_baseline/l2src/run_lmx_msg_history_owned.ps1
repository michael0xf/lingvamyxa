# Isolated L1 history bookkeeping check; no scheduler/runtime build.
param()
$ErrorActionPreference = 'Stop'
$baseline = Split-Path -Parent $PSScriptRoot
$repo = Split-Path -Parent (Split-Path -Parent $baseline)
$compiler = Join-Path $baseline 'build/l1trans/gen2/l1trans.exe'
$pin = '65D5A5ED127CA1BAEBDD1D500A5B74CEEA63EC1985EAC52EDEF28EFEB261C936'
if ((Get-FileHash -LiteralPath $compiler).Hash -ne $pin) { throw 'Stable compiler pin changed.' }
$runId = (Get-Date -Format 'yyyyMMdd_HHmmss_fff') + '_' + [guid]::NewGuid().ToString('N').Substring(0,8)
$run = Join-Path $repo "build/codex/msg_history_owned/$runId"
$headers = Join-Path $run 'headers'
New-Item -ItemType Directory -Path (Join-Path $headers 'l2src') -Force | Out-Null
$headerUnits = @('lmx_msg_blocks','lmx_owned_ranges','lmx_msg_storage','lmx_msg_path_storage','lmx_msg_history_owned')
$sources = @($PSCommandPath, (Join-Path $PSScriptRoot 'lmx.h'), (Join-Path $PSScriptRoot 'lmx_message.h'),
    (Join-Path $PSScriptRoot 'lmx_owned_ranges.lm1'),
    (Join-Path $PSScriptRoot 'lmx_msg_history_owned.lm1'),
    (Join-Path $PSScriptRoot 'tests/lmx_msg_history_owned_selftest.lm1'))
foreach ($unit in $headerUnits) { $sources += Join-Path $PSScriptRoot "$unit.h.lm1" }
$hashes = @{}
foreach ($source in $sources) { $hashes[$source] = (Get-FileHash -LiteralPath $source).Hash }
$evidence = [ordered]@{ compiler=$compiler; compilerSHA256=$pin; sources=$hashes; optimization='O2'; stages=@(); result='RUNNING' }
function Invoke-HistoryStage([string]$Name, [string]$Tool, [string[]]$NativeArgs) {
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
        Invoke-HistoryStage "header_$unit" $compiler @("l2src/$unit.h.lm1", (Join-Path $headers "l2src/$unit.lm1.h"))
    }
    $module = Join-Path $run 'lmx_msg_history_owned.c'
    $test = Join-Path $run 'lmx_msg_history_owned_selftest.c'
    $ranges = Join-Path $run 'lmx_owned_ranges.c'
    Invoke-HistoryStage 'module' $compiler @('l2src/lmx_msg_history_owned.lm1',$module)
    Invoke-HistoryStage 'test' $compiler @('l2src/tests/lmx_msg_history_owned_selftest.lm1',$test)
    Invoke-HistoryStage 'ranges' $compiler @('l2src/lmx_owned_ranges.lm1',$ranges)
    $gcc = (Get-Command gcc -ErrorAction Stop).Source
    $nm = (Get-Command nm -ErrorAction Stop).Source
    Invoke-HistoryStage 'gcc_version' $gcc @('--version')
    $flags = @('-std=c99','-O2','-Wall','-Wextra','-Wpedantic','-Werror','-I',$headers,'-I',$baseline)
    $obj = Join-Path $run 'history.o'
    $exe = Join-Path $run 'history.exe'
    Invoke-HistoryStage 'object' $gcc ($flags + @('-c',$module,'-o',$obj))
    Invoke-HistoryStage 'imports' $nm @('-u',$obj)
    foreach ($line in (Get-Content -LiteralPath (Join-Path $run 'imports.stdout.txt'))) {
        if ($line.Trim() -and $line -notmatch '^\s*U\s+_?(free|malloc|lmx_owned_ranges_find)\s*$') { throw "Unexpected dependency: $line" }
    }
    Invoke-HistoryStage 'symbols' $nm @('--defined-only',$obj)
    foreach ($line in (Get-Content -LiteralPath (Join-Path $run 'symbols.stdout.txt'))) {
        if ($line -match '^\s*[0-9a-fA-F]+\s+[bBdDgGsSC]\s+' -and $line -notmatch '\s+\.(bss|data|sbss|sdata)\s*$') { throw "Mutable module storage: $line" }
    }
    # Scope fault substitution to the helper object. Global linker wrapping
    # also catches MinGW CRT malloc calls during test startup/setup.
    $faultObj = Join-Path $run 'history_fault.o'
    Invoke-HistoryStage 'fault_object' $gcc ($flags + @('-Dmalloc=lmx_history_test_malloc','-Dfree=lmx_history_test_free','-c',$module,'-o',$faultObj))
    Invoke-HistoryStage 'compile' $gcc ($flags + @($faultObj,$ranges,$test,'-o',$exe))
    Invoke-HistoryStage 'run' $exe @()
    $result = Get-Content -LiteralPath (Join-Path $run 'run.stdout.txt') -Raw
    if ($result -notmatch '(?m)^history checks=\d+ failures=0 live=0 malloc_calls=\d+ frees=\d+\s*$') { throw "Unexpected result: $result" }
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
