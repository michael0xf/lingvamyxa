# Standalone owned helper only. Stable compiler is read-only; no Message build.
param([ValidateNotNullOrEmpty()][string[]]$Optimization = @('O2'))
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
if ((Get-FileHash -LiteralPath $compiler).Hash -ne $pin) { throw 'Stable compiler pin changed.' }
$runId = (Get-Date -Format 'yyyyMMdd_HHmmss_fff') + '_' + [guid]::NewGuid().ToString('N').Substring(0,8)
$run = Join-Path $repo "build/codex/msg_liveness/$runId"
$headers = Join-Path $run 'headers'
New-Item -ItemType Directory -Path (Join-Path $headers 'l2src') -Force | Out-Null
$sources = @($PSCommandPath, (Join-Path $PSScriptRoot 'lmx.h'),
    (Join-Path $PSScriptRoot 'lmx_msg_blocks.h.lm1'),
    (Join-Path $PSScriptRoot 'lmx_owned_ranges.h.lm1'),
    (Join-Path $PSScriptRoot 'lmx_msg_visit.h.lm1'),
    (Join-Path $PSScriptRoot 'lmx_msg_liveness.h.lm1'),
    (Join-Path $PSScriptRoot 'lmx_msg_liveness.lm1'),
    (Join-Path $PSScriptRoot 'tests/lmx_msg_liveness_selftest.lm1'))
$hashes = @{}
foreach ($source in $sources) { $hashes[$source] = (Get-FileHash -LiteralPath $source).Hash }
$evidence = [ordered]@{ compiler=$compiler; compilerSHA256=$pin; sources=$hashes; optimization=$Optimization; stages=@(); result='RUNNING' }
function Invoke-LivenessStage([string]$Name, [string]$Tool, [string[]]$NativeArgs) {
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
    $evidence.stages += [ordered]@{ name=$Name; tool=$Tool; arguments=$NativeArgs; exit=$code }
    if ($code -ne 0) {
        Get-Content -LiteralPath $stdout,$stderr
        throw "$Name failed: $code"
    }
}
try {
    $header = Join-Path $headers 'l2src/lmx_msg_liveness.lm1.h'
    $module = Join-Path $run 'lmx_msg_liveness.c'
    $test = Join-Path $run 'lmx_msg_liveness_selftest.c'
    foreach ($name in @('lmx_msg_blocks','lmx_owned_ranges','lmx_msg_visit','lmx_msg_liveness')) {
        Invoke-LivenessStage "header_$name" $compiler @("l2src/$name.h.lm1", (Join-Path $headers "l2src/$name.lm1.h"))
    }
    Invoke-LivenessStage 'module' $compiler @('l2src/lmx_msg_liveness.lm1',$module)
    Invoke-LivenessStage 'test' $compiler @('l2src/tests/lmx_msg_liveness_selftest.lm1',$test)
    $gcc = (Get-Command gcc -ErrorAction Stop).Source
    $nm = (Get-Command nm -ErrorAction Stop).Source
    Invoke-LivenessStage 'gcc_version' $gcc @('--version')
    $flags = @('-std=c99','-Wall','-Wextra','-Wpedantic','-Werror','-I',$baseline,'-I',$headers)
    foreach ($level in @($Optimization | ForEach-Object { $_.ToUpperInvariant() } | Select-Object -Unique)) {
        $obj = Join-Path $run "lmx_msg_liveness_$level.o"
        $exe = Join-Path $run "lmx_msg_liveness_$level.exe"
        Invoke-LivenessStage "object_$level" $gcc ($flags + @("-$level",'-c',$module,'-o',$obj))
        Invoke-LivenessStage "imports_$level" $nm @('-u',$obj)
        foreach ($line in (Get-Content -LiteralPath (Join-Path $run "imports_$level.stdout.txt"))) {
            if ($line.Trim()) { throw "Unexpected dependency: $line" }
        }
        Invoke-LivenessStage "symbols_$level" $nm @('--defined-only',$obj)
        foreach ($line in (Get-Content -LiteralPath (Join-Path $run "symbols_$level.stdout.txt"))) {
            # PE/COFF section labels exist even for empty .bss/.data sections;
            # they are not mutable C variables. All actual data symbols fail.
            if ($line -match '^\s*[0-9a-fA-F]+\s+[bBdDgGsSC]\s+' -and $line -notmatch '\s+\.(bss|data|sbss|sdata)\s*$') { throw "Mutable module storage: $line" }
        }
        Invoke-LivenessStage "compile_$level" $gcc ($flags + @("-$level",$obj,$test,'-o',$exe))
        Invoke-LivenessStage "run_$level" $exe @()
        $result = Get-Content -LiteralPath (Join-Path $run "run_$level.stdout.txt") -Raw
        if ($result -notmatch '(?m)^liveness checks=\d+ failures=0\s*$') { throw "Unexpected result: $result" }
        Write-Output "$level $($result.Trim())"
    }
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
