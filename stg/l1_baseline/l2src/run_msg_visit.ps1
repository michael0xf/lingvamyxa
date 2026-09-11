# Standalone owned helper only. Stable compiler is read-only; no Message build.
param([ValidateNotNullOrEmpty()][ValidateSet('O0','O2')][string[]]$Optimization = @('O2'))
$ErrorActionPreference = 'Stop'
$baseline = Split-Path -Parent $PSScriptRoot
$repo = Split-Path -Parent (Split-Path -Parent $baseline)
$compiler = Join-Path $baseline 'build/l1trans/gen2/l1trans.exe'
$pin = '65D5A5ED127CA1BAEBDD1D500A5B74CEEA63EC1985EAC52EDEF28EFEB261C936'
if ((Get-FileHash -LiteralPath $compiler).Hash -ne $pin) { throw 'Stable compiler pin changed.' }
$runId = (Get-Date -Format 'yyyyMMdd_HHmmss_fff') + '_' + [guid]::NewGuid().ToString('N').Substring(0,8)
$run = Join-Path $repo "build/codex/msg_visit/$runId"
$headers = Join-Path $run 'headers'
New-Item -ItemType Directory -Path (Join-Path $headers 'l2src') -Force | Out-Null
$sources = @($PSCommandPath, (Join-Path $PSScriptRoot 'lmx.h'),
    (Join-Path $PSScriptRoot 'lmx_msg_visit.h.lm1'),
    (Join-Path $PSScriptRoot 'lmx_msg_visit.lm1'),
    (Join-Path $PSScriptRoot 'tests/lmx_msg_visit_selftest.lm1'))
$hashes = @{}
foreach ($source in $sources) { $hashes[$source] = (Get-FileHash -LiteralPath $source).Hash }
$evidence = [ordered]@{ compiler=$compiler; compilerSHA256=$pin; sources=$hashes; optimization=$Optimization; stages=@(); result='RUNNING' }
function Invoke-VisitStage([string]$Name, [string]$Tool, [string[]]$NativeArgs) {
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
    $header = Join-Path $headers 'l2src/lmx_msg_visit.lm1.h'
    $module = Join-Path $run 'lmx_msg_visit.c'
    $test = Join-Path $run 'lmx_msg_visit_selftest.c'
    Invoke-VisitStage 'header' $compiler @('l2src/lmx_msg_visit.h.lm1',$header)
    Invoke-VisitStage 'module' $compiler @('l2src/lmx_msg_visit.lm1',$module)
    Invoke-VisitStage 'test' $compiler @('l2src/tests/lmx_msg_visit_selftest.lm1',$test)
    $gcc = (Get-Command gcc -ErrorAction Stop).Source
    $nm = (Get-Command nm -ErrorAction Stop).Source
    Invoke-VisitStage 'gcc_version' $gcc @('--version')
    $flags = @('-std=c99','-Wall','-Wextra','-Wpedantic','-Werror','-I',$baseline,'-I',$headers)
    foreach ($level in @($Optimization | ForEach-Object { $_.ToUpperInvariant() } | Select-Object -Unique)) {
        $obj = Join-Path $run "lmx_msg_visit_$level.o"
        $exe = Join-Path $run "lmx_msg_visit_$level.exe"
        Invoke-VisitStage "object_$level" $gcc ($flags + @("-$level",'-c',$module,'-o',$obj))
        Invoke-VisitStage "imports_$level" $nm @('-u',$obj)
        foreach ($line in (Get-Content -LiteralPath (Join-Path $run "imports_$level.stdout.txt"))) {
            if ($line.Trim() -and $line -notmatch '^\s*U\s+_?(free|realloc)\s*$') { throw "Unexpected dependency: $line" }
        }
        Invoke-VisitStage "symbols_$level" $nm @('--defined-only',$obj)
        foreach ($line in (Get-Content -LiteralPath (Join-Path $run "symbols_$level.stdout.txt"))) {
            # PE/COFF section labels exist even for empty .bss/.data sections;
            # they are not mutable C variables. All actual data symbols fail.
            if ($line -match '^\s*[0-9a-fA-F]+\s+[bBdDgGsSC]\s+' -and $line -notmatch '\s+\.(bss|data|sbss|sdata)\s*$') { throw "Mutable module storage: $line" }
        }
        Invoke-VisitStage "compile_$level" $gcc ($flags + @("-$level",$obj,$test,'-Wl,--wrap=free','-Wl,--wrap=realloc','-o',$exe))
        Invoke-VisitStage "run_$level" $exe @()
        $result = Get-Content -LiteralPath (Join-Path $run "run_$level.stdout.txt") -Raw
        if ($result -notmatch '(?m)^visit checks=\d+ failures=0 frees=3 realloc_calls=\d+\s*$') { throw "Unexpected result: $result" }
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
