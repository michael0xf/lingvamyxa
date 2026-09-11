# Isolated paired storage handoff verification. Shared stable/compiler and Grok
# build outputs are read-only. Every generated artifact belongs to this run.
param()
$ErrorActionPreference = 'Stop'
$baseline = Split-Path -Parent $PSScriptRoot
$repo = Split-Path -Parent (Split-Path -Parent $baseline)
$compiler = Join-Path $baseline 'build/l1trans/gen2/l1trans.exe'
$pin = '65D5A5ED127CA1BAEBDD1D500A5B74CEEA63EC1985EAC52EDEF28EFEB261C936'
if ((Get-FileHash -LiteralPath $compiler).Hash -ne $pin) { throw 'Stable compiler pin changed.' }
$runId = (Get-Date -Format 'yyyyMMdd_HHmmss_fff') + '_' + [guid]::NewGuid().ToString('N').Substring(0,8)
$run = Join-Path $repo "build/codex/msg_storage/$runId"
$headers = Join-Path $run 'headers'
New-Item -ItemType Directory -Path (Join-Path $headers 'l2src') -Force | Out-Null
$sources = @(
    (Join-Path $PSScriptRoot 'lmx_msg_blocks.h.lm1'),
    (Join-Path $PSScriptRoot 'lmx_msg_blocks.lm1'),
    (Join-Path $PSScriptRoot 'lmx_msg_storage.h.lm1'),
    (Join-Path $PSScriptRoot 'lmx_msg_storage.lm1'),
    (Join-Path $PSScriptRoot 'lmx_owned_ranges.h.lm1'),
    (Join-Path $PSScriptRoot 'lmx_owned_ranges.lm1'),
    (Join-Path $PSScriptRoot 'tests/lmx_msg_storage_selftest.lm1'),
    $PSCommandPath
)
$hashes = @{}
foreach ($file in $sources) { $hashes[$file] = (Get-FileHash -LiteralPath $file).Hash }
$evidence = [ordered]@{ compiler = $compiler; compilerSHA256 = $pin; sources = $hashes; stages = @(); result = 'RUNNING' }
function Invoke-StorageStage([string]$Name, [string]$Tool, [string[]]$NativeArgs) {
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
    $modules = @()
    $gcc = (Get-Command gcc -ErrorAction Stop).Source
    $nm = (Get-Command nm -ErrorAction Stop).Source
    $flags = @('-std=c99', '-Wall', '-Wextra', '-Wpedantic', '-Werror', '-I', $headers)
    foreach ($name in @('lmx_msg_blocks', 'lmx_owned_ranges', 'lmx_msg_storage')) {
        $header = Join-Path $headers "l2src/$name.lm1.h"
        $module = Join-Path $run "$name.c"
        Invoke-StorageStage "header_$name" $compiler @("l2src/$name.h.lm1", $header)
        Invoke-StorageStage "module_$name" $compiler @("l2src/$name.lm1", $module)
        $modules += $module
        $obj = Join-Path $run "$name.o"
        Invoke-StorageStage "object_$name" $gcc ($flags + @('-c', $module, '-o', $obj))
        Invoke-StorageStage "imports_$name" $nm @('-u', $obj)
        $imports = @(Get-Content -LiteralPath (Join-Path $run "imports_$name.stdout.txt") |
            Where-Object { -not [string]::IsNullOrWhiteSpace($_) } |
            ForEach-Object { ($_ -split '\s+')[-1] })
        $allowed = @()
        if ($name -eq 'lmx_msg_blocks') { $allowed = @('free') }
        if ($name -eq 'lmx_msg_storage') {
            $allowed = @('lmx_msg_blocks_can_move', 'lmx_msg_blocks_move_all',
                'lmx_owned_ranges_can_move', 'lmx_owned_ranges_move_all')
        }
        foreach ($symbol in $imports) {
            if ($symbol -notin $allowed) { throw "Unexpected import in ${name}: $symbol" }
        }
        Invoke-StorageStage "symbols_$name" $nm @('--defined-only', $obj)
        $symbols = Get-Content -LiteralPath (Join-Path $run "symbols_$name.stdout.txt") -Raw
        if ($symbols -match '(?m)^\S+\s+[BbCcDdGgSs]\s+(?!\.)\S+') { throw "Mutable static/global data in $name" }
    }
    $test = Join-Path $run 'lmx_msg_storage_selftest.c'
    Invoke-StorageStage 'test' $compiler @('l2src/tests/lmx_msg_storage_selftest.lm1', $test)
    Invoke-StorageStage 'gcc_version' $gcc @('--version')
    foreach ($level in @('O0', 'O2')) {
        $exe = Join-Path $run "lmx_msg_storage_$level.exe"
        Invoke-StorageStage "compile_$level" $gcc ($flags + @("-$level") + $modules + @($test, '-o', $exe))
        Invoke-StorageStage "run_$level" $exe @()
        $result = Get-Content -LiteralPath (Join-Path $run "run_$level.stdout.txt") -Raw
        if ($result -notmatch 'storage checks=77 failures=0') { throw "Unexpected selftest result: $result" }
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
