# Isolated L1 helper verification. No Message native sources or shared outputs.
param()
$ErrorActionPreference = 'Stop'
$baseline = Split-Path -Parent $PSScriptRoot
$repo = Split-Path -Parent (Split-Path -Parent $baseline)
$compiler = Join-Path $baseline 'build/l1trans/gen2/l1trans.exe'
$pin = '65D5A5ED127CA1BAEBDD1D500A5B74CEEA63EC1985EAC52EDEF28EFEB261C936'
if ((Get-FileHash -LiteralPath $compiler).Hash -ne $pin) { throw 'Stable compiler pin changed.' }
$runId = (Get-Date -Format 'yyyyMMdd_HHmmss_fff') + '_' + [guid]::NewGuid().ToString('N').Substring(0,8)
$run = Join-Path $repo "build/codex/msg_path_storage/$runId"
$headers = Join-Path $run 'headers'
New-Item -ItemType Directory -Path (Join-Path $headers 'l2src') -Force | Out-Null
$sources = @(
    (Join-Path $PSScriptRoot 'lmx_msg_path_storage.h.lm1'),
    (Join-Path $PSScriptRoot 'lmx_msg_path_storage.lm1'),
    (Join-Path $PSScriptRoot 'tests/lmx_msg_path_storage_selftest.lm1'),
    $PSCommandPath
)
$hashes = @{}
foreach ($file in $sources) { $hashes[$file] = (Get-FileHash -LiteralPath $file).Hash }
$evidence = [ordered]@{ compiler = $compiler; compilerSHA256 = $pin; sources = $hashes; stages = @(); result = 'RUNNING' }
function Invoke-PathStage([string]$Name, [string]$Tool, [string[]]$NativeArgs) {
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
    $module = Join-Path $run 'lmx_msg_path_storage.c'
    $test = Join-Path $run 'lmx_msg_path_storage_selftest.c'
    Invoke-PathStage 'header' $compiler @('l2src/lmx_msg_path_storage.h.lm1', (Join-Path $headers 'l2src/lmx_msg_path_storage.lm1.h'))
    Invoke-PathStage 'module' $compiler @('l2src/lmx_msg_path_storage.lm1', $module)
    Invoke-PathStage 'test' $compiler @('l2src/tests/lmx_msg_path_storage_selftest.lm1', $test)
    $gcc = (Get-Command gcc -ErrorAction Stop).Source
    $nm = (Get-Command nm -ErrorAction Stop).Source
    $flags = @('-std=c99', '-Wall', '-Wextra', '-Wpedantic', '-Werror', '-I', $headers)
    Invoke-PathStage 'gcc_version' $gcc @('--version')
    foreach ($level in @('O0', 'O2')) {
        $obj = Join-Path $run "path_$level.o"
        $exe = Join-Path $run "path_$level.exe"
        Invoke-PathStage "object_$level" $gcc ($flags + @("-$level", '-c', $module, '-o', $obj))
        Invoke-PathStage "imports_$level" $nm @('-u', $obj)
        $imports = @(Get-Content -LiteralPath (Join-Path $run "imports_$level.stdout.txt") |
            Where-Object { -not [string]::IsNullOrWhiteSpace($_) } |
            ForEach-Object { ($_ -split '\s+')[-1] })
        if ($imports.Count -ne 1 -or $imports[0] -ne 'realloc') { throw "Unexpected imports: $imports" }
        Invoke-PathStage "symbols_$level" $nm @('--defined-only', $obj)
        $symbols = Get-Content -LiteralPath (Join-Path $run "symbols_$level.stdout.txt") -Raw
        if ($symbols -match '(?m)^\S+\s+[BbCcDdGgSs]\s+(?!\.)\S+') { throw 'Mutable data in production helper.' }
        Invoke-PathStage "compile_$level" $gcc ($flags + @("-$level", $obj, $test, '-Wl,--wrap=realloc', '-o', $exe))
        Invoke-PathStage "run_$level" $exe @()
        $result = Get-Content -LiteralPath (Join-Path $run "run_$level.stdout.txt") -Raw
        if ($result -notmatch '(?m)^path storage (checks=541 failures=0 allocations=11 size_guard_checks=0|checks=543 failures=0 allocations=11 size_guard_checks=2)\s*$') { throw "Unexpected test result: $result" }
        Write-Output "$level $($result.Trim())"
    }
    foreach ($file in $sources) {
        if ((Get-FileHash -LiteralPath $file).Hash -ne $hashes[$file]) { throw "Source changed: $file" }
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
