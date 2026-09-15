# Independent storage-module verification. Never rebuild/promote the shared L1
# compiler or write the Message developer's build/l2 outputs.
param(
    [ValidateNotNullOrEmpty()][string[]]$Optimization = @('O2')
)
$ErrorActionPreference = 'Stop'
# powershell -File passes "-Optimization O0,O2" as the single string "O0,O2",
# which a ValidateSet rejected; split it and accept only O0 and O2.
$requested = @($Optimization | ForEach-Object { $_ -split ',' } | ForEach-Object { $_.Trim() } | Where-Object { $_ })
if ($requested.Count -eq 0) { throw '-Optimization needs O0, O2 or O0,O2.' }
foreach ($level in $requested) {
    if (@('O0', 'O2') -notcontains $level) { throw "-Optimization '$level' is not O0 or O2." }
}
$Optimization = $requested
$baseline = Split-Path -Parent $PSScriptRoot
$repo = $baseline
$compiler = Join-Path $baseline 'build/l1trans/gen2/l1trans.exe'
$pin = (Get-Content -LiteralPath (Join-Path $PSScriptRoot 'L1_PIN.txt') -TotalCount 1).Trim()
if ($pin -notmatch '^[0-9A-F]{64}$') { throw "L1_PIN.txt must hold one 64-hex SHA256, got 'pin=$pin'" }
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
$evidence = [ordered]@{ compiler = $compiler; compilerSHA256 = $pin; sources = $sourceHashes; optimization = $Optimization; stages = @(); result = 'RUNNING' }
function Invoke-BlockStage([string]$Name, [string]$Tool, [string[]]$NativeArgs) {
    # Arguments are controlled paths/flags, contain no literal double quotes.
    $quoted = ($NativeArgs | ForEach-Object { '"' + $_ + '"' }) -join ' '
    $stdout = Join-Path $run "$Name.stdout.txt"
    $stderr = Join-Path $run "$Name.stderr.txt"
    # cmd /c, not Start-Process -Wait, which costs about 1 s per launch.
    Push-Location -LiteralPath $baseline
    try {
        cmd /c "`"$Tool`" $quoted > `"$stdout`" 2> `"$stderr`""
        $code = $LASTEXITCODE
    } finally {
        Pop-Location
    }
    $evidence.stages += [ordered]@{ name = $Name; tool = $Tool; arguments = $NativeArgs; exit = $code }
    if ($code -ne 0) {
        Get-Content -LiteralPath $stdout, $stderr
        throw "$Name failed with exit $code"
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
    foreach ($level in @($Optimization | ForEach-Object { $_.ToUpperInvariant() } | Select-Object -Unique)) {
        $exe = Join-Path $run "lmx_msg_blocks_$level.exe"
        Invoke-BlockStage "compile_$level" $gcc ($flags + @("-$level", $module, $test, '-Wl,--wrap=free', '-o', $exe))
        Invoke-BlockStage "run_$level" $exe @()
        $result = Get-Content -LiteralPath (Join-Path $run "run_$level.stdout.txt") -Raw
        if ($result -notmatch '(?m)^blocks checks=143 failures=0 frees=147 callbacks=2\s*$') {
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
