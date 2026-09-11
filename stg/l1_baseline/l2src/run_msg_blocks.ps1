# Independent storage-module verification. Never rebuild/promote the shared L1
# compiler or write the Message developer's build/l2 outputs.
param()
$ErrorActionPreference = 'Stop'
$baseline = Split-Path -Parent $PSScriptRoot
$repo = Split-Path -Parent (Split-Path -Parent $baseline)
$compiler = Join-Path $baseline 'build/l1trans/gen2/l1trans.exe'
$pin = '65D5A5ED127CA1BAEBDD1D500A5B74CEEA63EC1985EAC52EDEF28EFEB261C936'
if ((Get-FileHash -LiteralPath $compiler).Hash -ne $pin) {
    throw 'Stable compiler pin changed; reconcile before running this test.'
}
$runId = (Get-Date -Format 'yyyyMMdd_HHmmss_fff') + '_' + [guid]::NewGuid().ToString('N').Substring(0,8)
$run = Join-Path $repo "build/codex/msg_blocks/$runId"
$headers = Join-Path $run 'headers'
New-Item -ItemType Directory -Path (Join-Path $headers 'l2src') -Force | Out-Null
$sources = @(
    (Join-Path $PSScriptRoot 'lmx_msg_blocks.h.lm1'),
    (Join-Path $PSScriptRoot 'lmx_msg_blocks.lm1'),
    (Join-Path $PSScriptRoot 'tests/lmx_msg_blocks_selftest.lm1'),
    $PSCommandPath
)
$sourceHashes = @{}
foreach ($file in $sources) { $sourceHashes[$file] = (Get-FileHash -LiteralPath $file).Hash }
$evidence = [ordered]@{ compiler = $compiler; compilerSHA256 = $pin; sources = $sourceHashes; stages = @(); result = 'RUNNING' }
function Invoke-BlockStage([string]$Name, [string]$Tool, [string[]]$NativeArgs) {
    # Arguments are controlled paths/flags, contain no literal double quotes.
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
    $header = Join-Path $headers 'l2src/lmx_msg_blocks.lm1.h'
    $module = Join-Path $run 'lmx_msg_blocks.c'
    $test = Join-Path $run 'lmx_msg_blocks_selftest.c'
    Invoke-BlockStage 'header' $compiler @('l2src/lmx_msg_blocks.h.lm1', $header)
    Invoke-BlockStage 'module' $compiler @('l2src/lmx_msg_blocks.lm1', $module)
    Invoke-BlockStage 'test' $compiler @('l2src/tests/lmx_msg_blocks_selftest.lm1', $test)
    $gcc = (Get-Command gcc -ErrorAction Stop).Source
    $nm = (Get-Command nm -ErrorAction Stop).Source
    Invoke-BlockStage 'gcc_version' $gcc @('--version')
    $flags = @('-std=c99', '-Wall', '-Wextra', '-Wpedantic', '-Werror', '-I', $baseline, '-I', $headers)
    $obj = Join-Path $run 'lmx_msg_blocks.o'
    Invoke-BlockStage 'object' $gcc ($flags + @('-c', $module, '-o', $obj))
    Invoke-BlockStage 'imports' $nm @('-u', $obj)
    $imports = Get-Content -LiteralPath (Join-Path $run 'imports.stdout.txt')
    foreach ($line in $imports) {
        if ($line.Trim() -and $line -notmatch '^\s*U\s+_?free\s*$') {
            throw "Unexpected storage-module dependency: $line"
        }
    }
    foreach ($level in @('O0', 'O2')) {
        $exe = Join-Path $run "lmx_msg_blocks_$level.exe"
        Invoke-BlockStage "compile_$level" $gcc ($flags + @("-$level", $module, $test, '-Wl,--wrap=free', '-o', $exe))
        Invoke-BlockStage "run_$level" $exe @()
        $result = Get-Content -LiteralPath (Join-Path $run "run_$level.stdout.txt") -Raw
        if ($result -notmatch 'blocks checks=115 failures=0 frees=139 callbacks=2') {
            throw "Unexpected selftest result: $result"
        }
        Write-Output "$level $($result.Trim())"
    }
    foreach ($file in $sources) {
        if ((Get-FileHash -LiteralPath $file).Hash -ne $sourceHashes[$file]) {
            throw "Source changed during verification: $file"
        }
    }
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
