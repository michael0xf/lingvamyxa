# Core acceptance runner: the model's end-to-end scenario (section 36).
#
# Builds the production runtime (no test defines: every module, the executor
# and host C, lmx_message) from this checkout with the pinned L1 translator,
# translates l2src/tests/lmx_model_scenario36_selftest.lm1, links, and runs
# it twice; both runs must exit 0, report zero failures and agree byte for
# byte. L2_RUNTIME_PLAN_20260914.md names this test the acceptance of the
# core (decision 15); the L1 selftests are regression evidence, not this.
#
# -MessageSource points the build at another lmx_message.lm1 (a tripwire
# copy mutated by line number); evidence.json records it and the PASS line
# says SCRATCH. Nothing here replaces or deletes any module.

[CmdletBinding()]
param(
    [string]$TranslatorPath,
    [ValidateRange(1, 3600)][int]$TestTimeoutSeconds = 120,
    [string]$MessageSource = 'l2src/lmx_message.lm1',
    [string[]]$Tests = @('lmx_model_scenario36_selftest', 'lmx_msg_delivery_selftest', 'lmx_model_checks_19_29_6_selftest', 'lmx_model_liveness_33_selftest')
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

if (-not (Test-Path -LiteralPath $MessageSource)) { throw "missing lmx_message source $MessageSource" }
$messageLm1 = (Resolve-Path -LiteralPath $MessageSource).ProviderPath
$tracked = (Resolve-Path -LiteralPath 'l2src/lmx_message.lm1').ProviderPath

$stamp = (Get-Date).ToString('yyyyMMdd_HHmmss_fff')
$out = Join-Path $baseline "build/model_scenario36/$stamp"
$hdrs = Join-Path $out 'hdrs'
$support = Join-Path $out 'support'
$work = Join-Path $out 'work'
New-Item -ItemType Directory -Force -Path $out, (Join-Path $hdrs 'l2src/tests'), $support, $work | Out-Null

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

$ev = [ordered]@{ stamp = $stamp; baseline = $baseline; translator = $l1trans; translatorSHA256 = $l1transHash; messageSource = $messageLm1; messageSourceSHA256 = (Get-FileHash -LiteralPath $messageLm1).Hash; testSHA256 = (Get-FileHash -LiteralPath 'l2src/tests/lmx_model_scenario36_selftest.lm1').Hash }

# ---------------------------------------------------------------------------
# 1. The production runtime: every module, lmx_message, executor and host C,
#    compiled without any test define.
# ---------------------------------------------------------------------------
$names = @('lmx_msg_blocks', 'lmx_owned_ranges', 'lmx_msg_storage', 'lmx_msg_path_storage', 'lmx_msg_slots', 'lmx_msg_mail_chain', 'lmx_msg_sched_ready', 'lmx_msg_visit', 'lmx_msg_liveness', 'lmx_msg_history_owned', 'lmx_msg_roots_stale', 'lmx_branch_owned', 'lmx_value_owned', 'lmx_chars_owned', 'lmx_array_owned', 'lmx_array_ref_owned', 'lmx_graph_copy_owned', 'lmx_merge_owned', 'lmx_message_graph_copy')
$sources = @('l2src/lmx_message_host.c', 'l2src/lmx_message_exec.c')
foreach ($name in $names) {
    Step "header_$name" (Invoke-Native ((Q $l1trans) + " l2src/$name.h.lm1 " + (Q (Join-Path $hdrs "l2src/$name.lm1.h"))) (Join-Path $out "header_$name.log")) (Join-Path $out "header_$name.log")
    $source = Join-Path $support "$name.c"
    Step "module_$name" (Invoke-Native ((Q $l1trans) + " l2src/$name.lm1 " + (Q $source)) (Join-Path $out "module_$name.log")) (Join-Path $out "module_$name.log")
    $sources += $source
}
$messageSourceC = Join-Path $support 'lmx_message.c'
Step 'module_lmx_message' (Invoke-Native ((Q $l1trans) + ' ' + (Q $messageLm1) + ' ' + (Q $messageSourceC)) (Join-Path $out 'module_lmx_message.log')) (Join-Path $out 'module_lmx_message.log')
$sources += $messageSourceC
Step 'header_scenario' (Invoke-Native ((Q $l1trans) + ' l2src/tests/lmx_model_scenario36.h.lm1 ' + (Q (Join-Path $hdrs 'l2src/tests/lmx_model_scenario36.lm1.h'))) (Join-Path $out 'header_scenario.log')) (Join-Path $out 'header_scenario.log')

$objs = @()
foreach ($source in $sources) {
    $stem = [IO.Path]::GetFileNameWithoutExtension($source)
    $obj = Join-Path $support ($stem + '.o')
    $glog = Join-Path $out ($stem + '.gcc.log')
    Step "compile_$stem" (Invoke-Native ("gcc $cflags -I " + (Q $hdrs) + ' -I lm1/build -c ' + (Q $source) + ' -o ' + (Q $obj)) $glog) $glog
    $objs += $obj
}
$objList = ($objs | ForEach-Object { Q $_ }) -join ' '

# ---------------------------------------------------------------------------
# 2. The core tests, each translated, linked against the same runtime, and
#    run twice. Every test prints one summary line "<name>: N checks, 0
#    failures" and exits 0; a FAIL line anywhere is red.
# ---------------------------------------------------------------------------
$results = [ordered]@{}
foreach ($test in $Tests) {
    $src = "l2src/tests/$test.lm1"
    if (-not (Test-Path -LiteralPath $src)) { throw "missing core test $src" }
    $ev["testSHA256_$test"] = (Get-FileHash -LiteralPath $src).Hash
    $testC = Join-Path $out "$test.c"
    Step "translate_$test" (Invoke-Native ((Q $l1trans) + ' ' + $src + ' ' + (Q $testC)) (Join-Path $out "$test.translate.log")) (Join-Path $out "$test.translate.log")
    $testObj = Join-Path $out "$test.o"
    Step "compile_$test" (Invoke-Native ("gcc $cflags -I " + (Q $hdrs) + ' -I lm1/build -c ' + (Q $testC) + ' -o ' + (Q $testObj)) (Join-Path $out "$test.gcc.log")) (Join-Path $out "$test.gcc.log")
    $exe = Join-Path $out "$test.exe"
    Step "link_$test" (Invoke-Native ("gcc $cflags " + (Q $testObj) + ' ' + $objList + ' -o ' + (Q $exe)) (Join-Path $out "$test.link.log")) (Join-Path $out "$test.link.log")

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
        if ($text -notmatch '(?m)^[a-z0-9 ]+: (\d+) checks, 0 failures$') { throw "$test run $i did not report zero failures: $text" }
        $checks = [int]$Matches[1]
        if ($text -match '(?m)^FAIL ') { throw "$test run $i printed a FAIL line: $text" }
    }
    if ($runs[0].stdout -ne $runs[1].stdout -or $runs[0].stderr -ne $runs[1].stderr) { throw "$test`: the two runs disagree" }
    $results[$test] = [ordered]@{ checks = $checks; runs = $runs }
}
$ev.results = $results

$evPath = Join-Path $out 'evidence.json'
$ev | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $evPath -Encoding utf8
$scratchNote = ''
if ($messageLm1 -ne $tracked) { $scratchNote = " [SCRATCH: lmx_message source $messageLm1; not evidence for the tracked runtime]" }
$summary = ($results.Keys | ForEach-Object { "$_ $($results[$_].checks)/0" }) -join ', '
Write-Output "core tests PASS: $summary, two runs each agree, production runtime; evidence $evPath$scratchNote"
