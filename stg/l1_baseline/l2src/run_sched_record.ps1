# Core acceptance runner for stage 3c-1: the parent's scheduler record, an L2
# unit written directly in L2 (no L1 oracle). Builds the production runtime
# from this checkout with the pinned L1 translator, builds l2trans from the
# tree, translates l2src/lmx_sched_record.lm2 (REAL symbols, profile: runtime,
# no escape poll), links the L1 selftest against it and the runtime, runs it
# twice. -SourcePath points at another copy of the unit (a tripwire);
# evidence.json records it and the PASS line says SCRATCH.

[CmdletBinding()]
param(
    [string]$TranslatorPath,
    [ValidateRange(1, 3600)][int]$TestTimeoutSeconds = 120,
    [string]$SourcePath = 'l2src/lmx_sched_record.lm2'
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

$baseline = Split-Path -Parent $PSScriptRoot
Set-Location $baseline
if (-not $TranslatorPath) { $TranslatorPath = 'build/l1trans/gen2/l1trans.exe' }
if (-not (Test-Path -LiteralPath $TranslatorPath)) { throw "missing L1 translator $TranslatorPath" }
$l1trans = (Resolve-Path -LiteralPath $TranslatorPath).ProviderPath

$pinFile = Join-Path $PSScriptRoot 'L1_PIN.txt'
$pin = (Get-Content -LiteralPath $pinFile -TotalCount 1).Trim()
if ($pin -notmatch '^[0-9A-F]{64}$') { throw "l2src/L1_PIN.txt must hold one 64-hex SHA256, got '$pin'" }
$l1transHash = (Get-FileHash -LiteralPath $l1trans).Hash
if ($l1transHash -ne $pin) { throw "translator $l1trans is $l1transHash, but l2src/L1_PIN.txt pins $pin" }

if (-not (Test-Path -LiteralPath $SourcePath)) { throw "missing L2 source $SourcePath" }
$lm2 = (Resolve-Path -LiteralPath $SourcePath).ProviderPath
$tracked = (Resolve-Path -LiteralPath 'l2src/lmx_sched_record.lm2').ProviderPath

$stamp = (Get-Date).ToString('yyyyMMdd_HHmmss_fff')
$out = Join-Path $baseline "build/sched_record/$stamp"
$hdrs = Join-Path $out 'hdrs'
$support = Join-Path $out 'support'
$work = Join-Path $out 'work'
New-Item -ItemType Directory -Force -Path $out, (Join-Path $hdrs 'l2src'), $support, $work | Out-Null

$cflags = '-std=c99 -Wall -Wextra -Wpedantic -I . -Werror=incompatible-pointer-types -Werror=discarded-qualifiers -Werror=implicit-function-declaration -Werror=implicit-int'

function Q([string]$p) { '"' + $p + '"' }
function Invoke-Native([string]$command, [string]$log) {
    cmd /c "$command > `"$log`" 2>&1"
    return $LASTEXITCODE
}
function Step([string]$name, [int]$code, [string]$log) {
    if ($code -ne 0) {
        $tail = if (Test-Path -LiteralPath $log) { (Get-Content -LiteralPath $log -Raw) } else { '' }
        throw "$name exit $code`n$tail"
    }
}
function Invoke-Timed([string]$exe, [string]$stdoutPath, [string]$stderrPath) {
    $proc = Start-Process -FilePath $exe -WorkingDirectory $work -WindowStyle Hidden -PassThru `
        -RedirectStandardOutput $stdoutPath -RedirectStandardError $stderrPath
    $null = $proc.Handle
    if (-not $proc.WaitForExit($TestTimeoutSeconds * 1000)) {
        $proc.Kill()
        $proc.WaitForExit()
        throw "$exe timed out after $TestTimeoutSeconds seconds; see $stdoutPath / $stderrPath"
    }
    $proc.WaitForExit()
    $proc.Refresh()
    if ($null -eq $proc.ExitCode) { throw "$exe exited without an observable exit code" }
    return [int]$proc.ExitCode
}

$ev = [ordered]@{ stamp = $stamp; baseline = $baseline; translator = $l1trans; translatorSHA256 = $l1transHash; source = $lm2; sourceSHA256 = (Get-FileHash -LiteralPath $lm2).Hash; testSHA256 = (Get-FileHash -LiteralPath 'l2src/tests/lmx_sched_record_selftest.lm1').Hash }

# 1. The production runtime.
$names = @('lmx_msg_blocks', 'lmx_owned_ranges', 'lmx_msg_storage', 'lmx_msg_path_storage', 'lmx_msg_slots', 'lmx_msg_mail_chain', 'lmx_msg_sched_ready', 'lmx_msg_visit', 'lmx_msg_liveness', 'lmx_msg_history_owned', 'lmx_msg_roots_stale', 'lmx_branch_owned', 'lmx_value_owned', 'lmx_chars_owned', 'lmx_array_owned', 'lmx_array_ref_owned', 'lmx_graph_copy_owned', 'lmx_merge_owned', 'lmx_message_graph_copy')
$sources = @('l2src/lmx_message_host.c', 'l2src/lmx_message_exec.c')
foreach ($name in $names) {
    Step "header_$name" (Invoke-Native ((Q $l1trans) + " l2src/$name.h.lm1 " + (Q (Join-Path $hdrs "l2src/$name.lm1.h"))) (Join-Path $out "header_$name.log")) (Join-Path $out "header_$name.log")
    $source = Join-Path $support "$name.c"
    Step "module_$name" (Invoke-Native ((Q $l1trans) + " l2src/$name.lm1 " + (Q $source)) (Join-Path $out "module_$name.log")) (Join-Path $out "module_$name.log")
    $sources += $source
}
$messageSource = Join-Path $support 'lmx_message.c'
Step 'module_lmx_message' (Invoke-Native ((Q $l1trans) + ' l2src/lmx_message.lm1 ' + (Q $messageSource)) (Join-Path $out 'module_lmx_message.log')) (Join-Path $out 'module_lmx_message.log')
$sources += $messageSource
Step 'header_sched_record' (Invoke-Native ((Q $l1trans) + ' l2src/lmx_sched_record.h.lm1 ' + (Q (Join-Path $hdrs 'l2src/lmx_sched_record.lm1.h'))) (Join-Path $out 'header_sched_record.log')) (Join-Path $out 'header_sched_record.log')

$objs = @()
foreach ($source in $sources) {
    $stem = [IO.Path]::GetFileNameWithoutExtension($source)
    $obj = Join-Path $support ($stem + '.o')
    $glog = Join-Path $out ($stem + '.gcc.log')
    Step "compile_$stem" (Invoke-Native ("gcc $cflags -I " + (Q $hdrs) + ' -I lm1/build -c ' + (Q $source) + ' -o ' + (Q $obj)) $glog) $glog
    $objs += $obj
}
$objList = ($objs | ForEach-Object { Q $_ }) -join ' '

# 2. The translator from this checkout and the unit it emits.
$l2c = Join-Path $out 'l2trans.c'
$l2exe = Join-Path $out 'l2trans.exe'
Step 'l2trans_translate' (Invoke-Native ((Q $l1trans) + ' l2src/l2trans.lm1 ' + (Q $l2c)) (Join-Path $out 'l2trans.translate.log')) (Join-Path $out 'l2trans.translate.log')
Step 'l2trans_compile' (Invoke-Native ("gcc $cflags -I lm1/build " + (Q $l2c) + ' l2src/lmx_poll_stub.c -o ' + (Q $l2exe)) (Join-Path $out 'l2trans.gcc.log')) (Join-Path $out 'l2trans.gcc.log')
$gen = Join-Path $out 'generated.lm1'
Step 'l2trans_module' (Invoke-Native ((Q $l2exe) + ' ' + (Q $lm2) + ' ' + (Q $gen)) (Join-Path $out 'module.l2trans.log')) (Join-Path $out 'module.l2trans.log')
$genText = [IO.File]::ReadAllText((Resolve-Path -LiteralPath $gen).ProviderPath).Replace("`r`n", "`n")
$srcText = [IO.File]::ReadAllText($lm2).Replace("`r`n", "`n")
if ($genText -notmatch 'define: l2_program_entry l2_u[0-9A-F]{16}_entry') { throw 'the generated unit is not a library unit' }
if ($srcText -notmatch '(?m)^profile: runtime$') { throw 'lmx_sched_record.lm2 must declare profile: runtime' }
if ($genText -match 'lmx_msg_poll_escape\(') { throw 'a runtime-profile unit emitted an escape poll' }
$symbols = @('lmx_sched_record_new', 'lmx_sched_record_cursor', 'lmx_sched_record_set_cursor', 'lmx_sched_record_policy', 'lmx_sched_record_set_policy')
foreach ($s in $symbols) {
    if ($genText -notmatch ('(?m)^    fn: ' + [regex]::Escape($s) + ' \(')) { throw "the public wrapper is missing: $s" }
}
# The record's storage is the owner's arena and nothing else: every
# allocation goes through the owner's blocks and ranges, the record is two
# owned cells and no Array or list (decision 18: nothing is appended to a
# parent's structure from another lane), and the record is retained as a root.
if (([regex]::Matches($genText, 'c\.(malloc|calloc|realloc)\(')).Count -ne 0) { throw 'the unit allocates outside the owner arena' }
if ($genText -match 'lmx_array_new_positive_owned\(|lmx_array_new_owned\(') { throw 'the record must not hold an Array (decision 18)' }
if ($genText -notmatch 'lmx_unsigned_new_owned\(' -or $genText -notmatch 'lmx_int_new_owned\(') { throw 'the record is not two owned cells' }
if ($genText -notmatch 'c\.lmx_msg_root_attach\(') { throw 'the record is not retained as a root of the owner' }

$genC = Join-Path $out 'generated.c'
Step 'generated_translate' (Invoke-Native ((Q $l1trans) + ' ' + (Q $gen) + ' ' + (Q $genC)) (Join-Path $out 'generated.translate.log')) (Join-Path $out 'generated.translate.log')
$genObj = Join-Path $out 'generated.o'
Step 'generated_compile' (Invoke-Native ("gcc $cflags -I " + (Q $hdrs) + ' -I lm1/build -c ' + (Q $genC) + ' -o ' + (Q $genObj)) (Join-Path $out 'generated.gcc.log')) (Join-Path $out 'generated.gcc.log')
$nmLog = Join-Path $out 'generated.nm.txt'
Step 'generated_nm' (Invoke-Native ('nm --defined-only ' + (Q $genObj)) $nmLog) $nmLog
$nmText = [IO.File]::ReadAllText($nmLog)
foreach ($s in $symbols) {
    if ($nmText -notmatch ('(?m)^\S+\s+T\s+_?' + $s + '\s*$')) { throw "the generated object does not define $s" }
}

# 3. The selftest, twice.
$test = 'lmx_sched_record_selftest'
$testC = Join-Path $out "$test.c"
Step 'test_translate' (Invoke-Native ((Q $l1trans) + " l2src/tests/$test.lm1 " + (Q $testC)) (Join-Path $out "$test.translate.log")) (Join-Path $out "$test.translate.log")
$testObj = Join-Path $out "$test.o"
Step 'test_compile' (Invoke-Native ("gcc $cflags -I " + (Q $hdrs) + ' -I lm1/build -c ' + (Q $testC) + ' -o ' + (Q $testObj)) (Join-Path $out "$test.gcc.log")) (Join-Path $out "$test.gcc.log")
$exe = Join-Path $out "$test.exe"
Step 'test_link' (Invoke-Native ("gcc $cflags " + (Q $testObj) + ' ' + (Q $genObj) + ' ' + $objList + ' -o ' + (Q $exe)) (Join-Path $out "$test.link.log")) (Join-Path $out "$test.link.log")
$runs = @()
$checks = 0
foreach ($i in 1, 2) {
    $log = Join-Path $out "$test.$i.stdout.txt"
    $err = Join-Path $out "$test.$i.stderr.txt"
    $exit = Invoke-Timed $exe $log $err
    $text = [IO.File]::ReadAllText($log).Replace("`r`n", "`n")
    $errText = [IO.File]::ReadAllText($err).Replace("`r`n", "`n")
    $runs += [ordered]@{ exit = $exit; stdout = $text.Trim(); stderr = $errText.Trim() }
    if ($exit -ne 0) { throw "$test run $i exit $exit`n$text`n$errText" }
    if ($text -notmatch '(?m)^sched record: (\d+) checks, 0 failures$') { throw "$test run $i did not report zero failures: $text" }
    $checks = [int]$Matches[1]
    if ($errText -match 'library open failed') { throw "$test run $i reported a library open failure: $errText" }
}
if ($runs[0].stdout -ne $runs[1].stdout -or $runs[0].stderr -ne $runs[1].stderr) { throw 'the two runs disagree' }
$ev.runs = $runs
$ev.checks = $checks
$ev.generatedSHA256 = (Get-FileHash -LiteralPath $gen).Hash
$evPath = Join-Path $out 'evidence.json'
$ev | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath $evPath -Encoding utf8
$scratchNote = ''
if ($lm2 -ne $tracked) { $scratchNote = " [SCRATCH: source $lm2; not evidence for the tracked unit]" }
Write-Output "sched record PASS: $checks checks, 0 failures, two runs agree, unit real symbols, no escape poll; evidence $evPath$scratchNote"
