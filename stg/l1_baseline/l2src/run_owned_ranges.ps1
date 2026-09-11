# Isolated owner-local classifier verification. Shared stable/compiler and Grok
# build outputs are read-only. Every generated artifact belongs to this run.
param()
$ErrorActionPreference = 'Stop'
$baseline = Split-Path -Parent $PSScriptRoot
$repo = Split-Path -Parent (Split-Path -Parent $baseline)
$compiler = Join-Path $baseline 'build/l1trans/gen2/l1trans.exe'
$pin = '65D5A5ED127CA1BAEBDD1D500A5B74CEEA63EC1985EAC52EDEF28EFEB261C936'
if ((Get-FileHash -LiteralPath $compiler).Hash -ne $pin) { throw 'Stable compiler pin changed.' }
$runId = (Get-Date -Format 'yyyyMMdd_HHmmss_fff') + '_' + [guid]::NewGuid().ToString('N').Substring(0,8)
$run = Join-Path $repo "build/codex/owned_ranges/$runId"
$headers = Join-Path $run 'headers'
New-Item -ItemType Directory -Path (Join-Path $headers 'l2src') -Force | Out-Null
$sources = @(
    (Join-Path $PSScriptRoot 'lmx_owned_ranges.h.lm1'),
    (Join-Path $PSScriptRoot 'lmx_owned_ranges.lm1'),
    (Join-Path $PSScriptRoot 'tests/lmx_owned_ranges_selftest.lm1'),
    $PSCommandPath
)
$hashes = @{}
foreach ($file in $sources) { $hashes[$file] = (Get-FileHash -LiteralPath $file).Hash }
$evidence = [ordered]@{ compiler = $compiler; compilerSHA256 = $pin; sources = $hashes; stages = @(); result = 'RUNNING' }
function Invoke-RangeStage([string]$Name, [string]$Tool, [string[]]$NativeArgs) {
    $quoted = ($NativeArgs | ForEach-Object { '"' + $_ + '"' }) -join ' '
    $stdout = Join-Path $run "$Name.stdout.txt"
    $stderr = Join-Path $run "$Name.stderr.txt"
    $p = Start-Process -FilePath $Tool -ArgumentList $quoted -WorkingDirectory $baseline `
        -WindowStyle Hidden -Wait -PassThru -RedirectStandardOutput $stdout -RedirectStandardError $stderr
    $evidence.stages += [ordered]@{ name = $Name; tool = $Tool; arguments = $NativeArgs; exit = $p.ExitCode }
    if ($p.ExitCode -ne 0) {
        Get-Content -LiteralPath $stdout, $stderr
        throw "$Name failed with exit $($p.ExitCode)"
    }
}
try {
    $header = Join-Path $headers 'l2src/lmx_owned_ranges.lm1.h'
    $module = Join-Path $run 'lmx_owned_ranges.c'
    $test = Join-Path $run 'lmx_owned_ranges_selftest.c'
    Invoke-RangeStage 'header' $compiler @('l2src/lmx_owned_ranges.h.lm1', $header)
    Invoke-RangeStage 'module' $compiler @('l2src/lmx_owned_ranges.lm1', $module)
    Invoke-RangeStage 'test' $compiler @('l2src/tests/lmx_owned_ranges_selftest.lm1', $test)
    $gcc = (Get-Command gcc -ErrorAction Stop).Source
    $nm = (Get-Command nm -ErrorAction Stop).Source
    Invoke-RangeStage 'gcc_version' $gcc @('--version')
    $flags = @('-std=c99', '-Wall', '-Wextra', '-Wpedantic', '-Werror', '-I', $headers)
    $obj = Join-Path $run 'lmx_owned_ranges.o'
    Invoke-RangeStage 'object' $gcc ($flags + @('-c', $module, '-o', $obj))
    Invoke-RangeStage 'imports' $nm @('-u', $obj)
    if (-not [string]::IsNullOrWhiteSpace((Get-Content -LiteralPath (Join-Path $run 'imports.stdout.txt') -Raw))) {
        throw 'Owner-local classifier unexpectedly depends on external runtime functions.'
    }
    Invoke-RangeStage 'symbols' $nm @('--defined-only', $obj)
    $symbols = Get-Content -LiteralPath (Join-Path $run 'symbols.stdout.txt') -Raw
    # COFF lists empty .bss/.data section symbols as well as actual objects.
    if ($symbols -match '(?m)^\S+\s+[BbCcDdGgSs]\s+(?!\.)\S+') { throw 'Classifier contains mutable static/global data.' }
    foreach ($level in @('O0', 'O2')) {
        $exe = Join-Path $run "lmx_owned_ranges_$level.exe"
        Invoke-RangeStage "compile_$level" $gcc ($flags + @("-$level", $module, $test, '-o', $exe))
        Invoke-RangeStage "run_$level" $exe @()
        $result = Get-Content -LiteralPath (Join-Path $run "run_$level.stdout.txt") -Raw
        if ($result -notmatch 'owned ranges checks=439 failures=0') { throw "Unexpected selftest result: $result" }
        Write-Output "$level $($result.Trim())"
    }
    foreach ($file in $sources) {
        if ((Get-FileHash -LiteralPath $file).Hash -ne $hashes[$file]) { throw "Source changed: $file" }
    }
    if ((Get-FileHash -LiteralPath $compiler).Hash -ne $pin) { throw 'Compiler changed during verification.' }
    $evidence.artifacts = @{}
    Get-ChildItem -LiteralPath $run -Recurse -File | ForEach-Object {
        $evidence.artifacts[$_.FullName] = (Get-FileHash -LiteralPath $_.FullName).Hash
    }
    $evidence.result = 'PASS'
} catch {
    $evidence.result = 'FAIL'
    $evidence.error = $_.Exception.Message
    throw
} finally {
    $evidence | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath (Join-Path $run 'evidence.json') -Encoding UTF8
    Write-Output "Evidence: $run"
}
