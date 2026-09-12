# Pure storage helper: no Message executor, compiler rebuild or stable writes.
param()
$ErrorActionPreference = 'Stop'
$baseline = Split-Path -Parent $PSScriptRoot
$repo = Split-Path -Parent (Split-Path -Parent $baseline)
$compiler = Join-Path $baseline 'build/l1trans/gen2/l1trans.exe'
$pin = '65D5A5ED127CA1BAEBDD1D500A5B74CEEA63EC1985EAC52EDEF28EFEB261C936'
if ((Get-FileHash -LiteralPath $compiler).Hash -ne $pin) { throw 'Stable compiler pin mismatch' }
$runId = (Get-Date -Format 'yyyyMMdd_HHmmss_fff') + '_' + [guid]::NewGuid().ToString('N').Substring(0,8)
$run = Join-Path $repo "build/codex/array_ref_owned/$runId"
$headers = Join-Path $run 'headers'
New-Item -ItemType Directory -Path (Join-Path $headers 'l2src') -Force | Out-Null
$names = @('lmx_msg_blocks','lmx_owned_ranges','lmx_msg_storage','lmx_array_ref_owned')
$sources = @($PSCommandPath, (Join-Path $PSScriptRoot 'lmx.h'), (Join-Path $PSScriptRoot 'tests/lmx_array_ref_owned_selftest.lm1'))
foreach ($name in $names) {
    $sources += Join-Path $PSScriptRoot "$name.h.lm1"
    $sources += Join-Path $PSScriptRoot "$name.lm1"
}
$hashes = @{}
foreach ($source in $sources) { $hashes[$source] = (Get-FileHash -LiteralPath $source).Hash }
$gcc = (Get-Command gcc -ErrorAction Stop).Source
$nm = (Get-Command nm -ErrorAction Stop).Source
$flags = @('-std=c99','-Wall','-Wextra','-Wpedantic','-Werror','-O2','-I',$baseline,'-I',$headers)
$evidence = [ordered]@{result='RUNNING'; compiler=$compiler; compilerSHA256=$pin; gcc=$gcc; gccSHA256=(Get-FileHash -LiteralPath $gcc).Hash; options=$flags; sources=$hashes; stages=@()}
function Invoke-ArrayStage([string]$Name, [string]$Tool, [string[]]$NativeArgs) {
    $options = @{}
    if ($NativeArgs.Count -gt 0) { $options.ArgumentList = ($NativeArgs | ForEach-Object { '"' + $_ + '"' }) -join ' ' }
    $stdout = Join-Path $run "$Name.stdout.txt"
    $stderr = Join-Path $run "$Name.stderr.txt"
    $process = Start-Process -FilePath $Tool @options -WorkingDirectory $baseline -WindowStyle Hidden -PassThru -RedirectStandardOutput $stdout -RedirectStandardError $stderr
    $processHandle = $process.Handle
    if (-not $process.WaitForExit(60000)) {
        $process.Kill()
        $process.WaitForExit()
        throw "$Name timed out; stopped only this launched child"
    }
    $process.WaitForExit()
    $evidence.stages += [ordered]@{name=$Name; tool=$Tool; arguments=$NativeArgs; exit=$process.ExitCode}
    if ($process.ExitCode -ne 0) {
        Get-Content -LiteralPath $stdout,$stderr
        throw "$Name failed: $($process.ExitCode)"
    }
}
try {
    $objects = @()
    foreach ($name in $names) {
        Invoke-ArrayStage "header_$name" $compiler @("l2src/$name.h.lm1", (Join-Path $headers "l2src/$name.lm1.h"))
        $module = Join-Path $run "$name.c"
        $obj = Join-Path $run "$name.o"
        Invoke-ArrayStage "translate_$name" $compiler @("l2src/$name.lm1",$module)
        Invoke-ArrayStage "compile_$name" $gcc ($flags + @('-c',$module,'-o',$obj))
        $objects += $obj
    }
    $charObject = Join-Path $run 'lmx_array_ref_owned.o'
    Invoke-ArrayStage 'imports' $nm @('-u',$charObject)
    foreach ($line in (Get-Content -LiteralPath (Join-Path $run 'imports.stdout.txt'))) {
        if ($line.Trim() -and $line -notmatch '\bU\s+(malloc|free|memset|lmx_msg_storage_move_all|lmx_owned_ranges_remove|lmx_msg_blocks_dispose_all)\s*$') { throw "Unexpected array dependency: $line" }
    }
    Invoke-ArrayStage 'symbols' $nm @('--defined-only',$charObject)
    foreach ($line in (Get-Content -LiteralPath (Join-Path $run 'symbols.stdout.txt'))) {
        if ($line -match '^\s*[0-9a-fA-F]+\s+[bBdDgGsSC]\s+' -and $line -notmatch '\s+\.(bss|data|sbss|sdata)\s*$') { throw "Global array storage: $line" }
    }
    $test = Join-Path $run 'lmx_array_ref_owned_selftest.c'
    $exe = Join-Path $run 'lmx_array_ref_owned_selftest.exe'
    Invoke-ArrayStage 'translate_test' $compiler @('l2src/tests/lmx_array_ref_owned_selftest.lm1',$test)
    Invoke-ArrayStage 'link_test' $gcc ($flags + $objects + @($test,'-Wl,--wrap=malloc','-Wl,--wrap=free','-o',$exe))
    Invoke-ArrayStage 'run_test' $exe @()
    $result = Get-Content -LiteralPath (Join-Path $run 'run_test.stdout.txt') -Raw
    if ($result -notmatch '^array_ref_owned checks=\d+ failures=0 allocations=(\d+) releases=(\d+)\s*$' -or $Matches[1] -ne $Matches[2]) { throw "Unexpected test result: $result" }
    Write-Output $result.Trim()
    foreach ($source in $sources) {
        if ((Get-FileHash -LiteralPath $source).Hash -ne $hashes[$source]) { throw "Source changed: $source" }
    }
    if ((Get-FileHash -LiteralPath $compiler).Hash -ne $pin -or (Get-FileHash -LiteralPath $gcc).Hash -ne $evidence.gccSHA256) { throw 'Toolchain changed during verification' }
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
