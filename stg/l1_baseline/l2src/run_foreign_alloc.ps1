# Focused foreign ABI test; no Message runtime or translator self-build.
$ErrorActionPreference = 'Stop'
$baseline = Split-Path -Parent $PSScriptRoot
$repo = Split-Path -Parent (Split-Path -Parent $baseline)
$compiler = Join-Path $baseline 'build/l1trans/gen2/l1trans.exe'
$pin = '65D5A5ED127CA1BAEBDD1D500A5B74CEEA63EC1985EAC52EDEF28EFEB261C936'
if ((Get-FileHash -LiteralPath $compiler).Hash -ne $pin) { throw 'Stable compiler pin mismatch' }
$run = Join-Path $repo ('build/codex/foreign_alloc/' + (Get-Date -Format 'yyyyMMdd_HHmmss_fff') + '_' + [guid]::NewGuid().ToString('N').Substring(0,8))
New-Item -ItemType Directory -Path $run -Force | Out-Null
$sources = @($PSCommandPath, (Join-Path $PSScriptRoot 'l2_foreign_alloc.lm1'), (Join-Path $PSScriptRoot 'tests/l2_foreign_alloc_selftest.lm1'), (Join-Path $baseline 'lm1/build/l1src/p0.lm1.h'))
$hashes = @{}
foreach ($source in $sources) { $hashes[$source] = (Get-FileHash -LiteralPath $source).Hash }
$gcc = (Get-Command gcc -ErrorAction Stop).Source
$nm = (Get-Command nm -ErrorAction Stop).Source
$flags = @('-std=c99','-Wall','-Wextra','-Wpedantic','-Werror','-O2','-I',(Join-Path $baseline 'lm1/build'))
$evidence = [ordered]@{result='RUNNING'; compiler=$compiler; compilerSHA256=$pin; gcc=$gcc; gccSHA256=(Get-FileHash -LiteralPath $gcc).Hash; options=$flags; sources=$hashes; stages=@()}
function Invoke-ForeignStage([string]$Name, [string]$Tool, [string[]]$NativeArgs) {
    $arguments = ($NativeArgs | ForEach-Object { '"' + $_ + '"' }) -join ' '
    $options = @{}
    if ($arguments) { $options.ArgumentList = $arguments }
    $stdout = Join-Path $run "$Name.stdout.txt"
    $stderr = Join-Path $run "$Name.stderr.txt"
    $process = Start-Process -FilePath $Tool @options -WorkingDirectory $baseline -WindowStyle Hidden -PassThru -RedirectStandardOutput $stdout -RedirectStandardError $stderr
    $handle = $process.Handle
    if (-not $process.WaitForExit(60000)) {
        $process.Kill()
        $process.WaitForExit()
        throw "$Name timed out; stopped only this launched child"
    }
    $process.WaitForExit()
    $evidence.stages += @{name=$Name; tool=$Tool; arguments=$NativeArgs; exit=$process.ExitCode}
    if ($process.ExitCode -ne 0) { Get-Content -LiteralPath $stdout,$stderr; throw "$Name failed: $($process.ExitCode)" }
}
try {
    Invoke-ForeignStage 'translate_module' $compiler @('l2src/l2_foreign_alloc.lm1', "$run/module.c")
    Invoke-ForeignStage 'compile_module' $gcc ($flags + @('-c',"$run/module.c",'-o',"$run/module.o"))
    Invoke-ForeignStage 'imports' $nm @('-u',"$run/module.o")
    foreach ($line in (Get-Content -LiteralPath "$run/imports.stdout.txt")) {
        if ($line.Trim() -and $line -notmatch '\bU\s+(calloc|realloc|free|memcpy)\s*$') { throw "Unexpected foreign dependency: $line" }
    }
    Invoke-ForeignStage 'symbols' $nm @('--defined-only',"$run/module.o")
    $functions = @()
    foreach ($line in (Get-Content -LiteralPath "$run/symbols.stdout.txt")) {
        if ($line -match '^\s*[0-9a-fA-F]+\s+[bBdDgGsSC]\s+' -and $line -notmatch '\s+\.(bss|data|sbss|sdata)\s*$') { throw "Global foreign state: $line" }
        if ($line -match '\sT\s+(\w+)\s*$') { $functions += $Matches[1] }
    }
    if (($functions | Sort-Object) -join ',' -ne 'lm_own_copy_bytes,lm_own_delete,lm_own_new_zero,lm_own_resize') { throw 'Unexpected adapter API' }
    Invoke-ForeignStage 'translate_test' $compiler @('l2src/tests/l2_foreign_alloc_selftest.lm1',"$run/test.c")
    Invoke-ForeignStage 'link_test' $gcc ($flags + @("$run/test.c","$run/module.o",'-Wl,--wrap=calloc','-Wl,--wrap=realloc','-Wl,--wrap=free','-o',"$run/test.exe"))
    Invoke-ForeignStage 'run_test' "$run/test.exe" @()
    $result = Get-Content -LiteralPath "$run/run_test.stdout.txt" -Raw
    if ($result -notmatch '^foreign_alloc checks=\d+ failures=0 live=0\s*$') { throw "Unexpected result: $result" }
    Write-Output $result.Trim()
    foreach ($source in $sources) {
        if ((Get-FileHash -LiteralPath $source).Hash -ne $hashes[$source]) { throw "Source changed: $source" }
    }
    if ((Get-FileHash -LiteralPath $compiler).Hash -ne $pin -or (Get-FileHash -LiteralPath $gcc).Hash -ne $evidence.gccSHA256) { throw 'Toolchain changed during verification' }
    $evidence.artifacts = @{}
    Get-ChildItem -LiteralPath $run -File | ForEach-Object { $evidence.artifacts[$_.FullName] = (Get-FileHash -LiteralPath $_.FullName).Hash }
    $evidence.result = 'PASS'
} catch {
    $evidence.result = 'FAIL'
    $evidence.error = $_.Exception.Message
    throw
} finally {
    $evidence | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath "$run/evidence.json" -Encoding UTF8
    Write-Output "Evidence: $run"
}
