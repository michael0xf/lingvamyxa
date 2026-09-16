# Runtime-module parity runner: lmx_msg_liveness (review-chat lane, 2026-09-14).
#
# Run the module's own selftest against the handwritten L1 implementation and
# against one generated from L2 source by the CURRENT translator, and require
# the two to agree exactly, twice.
#
# The generated half is a LIBRARY UNIT (7d7ec87c) exporting the three ABI
# names by real symbol: the callers are the collector's sweep in
# lmx_message.lm1, at a collection boundary, never while a unit's graph is
# being built. The handwritten object is kept OUT of the parity link.
#
# The selftest keeps no allocation counters. A C driver opens the unit with
# real calls first: a stack block, a pointer inside it and one just past it.
#
# Nothing here replaces or deletes the L1 module.

[CmdletBinding()]
param([string]$TranslatorPath)

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

$stamp = (Get-Date).ToString('yyyyMMdd_HHmmss_fff')
$out = Join-Path $baseline "build/port_msg_liveness/$stamp"
$hdrs = Join-Path $out 'hdrs'
$support = Join-Path $out 'support'
New-Item -ItemType Directory -Force -Path $out, (Join-Path $hdrs 'l2src'), $support | Out-Null

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

$ev = [ordered]@{ stamp = $stamp; baseline = $baseline; translator = $l1trans; translatorSHA256 = $l1transHash }

$names = @('lmx_msg_blocks', 'lmx_owned_ranges', 'lmx_msg_storage', 'lmx_msg_mail_chain', 'lmx_msg_visit', 'lmx_msg_liveness', 'lmx_msg_history_owned', 'lmx_msg_roots_stale', 'lmx_branch_owned', 'lmx_value_owned', 'lmx_chars_owned', 'lmx_array_owned', 'lmx_array_ref_owned', 'lmx_graph_copy_owned', 'lmx_merge_owned', 'lmx_message_graph_copy')
$referenceNames = @('lmx_msg_blocks', 'lmx_owned_ranges', 'lmx_msg_visit', 'lmx_msg_liveness')
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

$objs = @()
$objByName = @{}
$nativeObj = $null
foreach ($source in $sources) {
    $stem = [IO.Path]::GetFileNameWithoutExtension($source)
    $obj = Join-Path $support ($stem + '.o')
    $glog = Join-Path $out ($stem + '.gcc.log')
    Step "compile_$stem" (Invoke-Native ("gcc $cflags -I " + (Q $hdrs) + ' -c ' + (Q $source) + ' -o ' + (Q $obj)) $glog) $glog
    $objByName[$stem] = $obj
    if ($stem -eq 'lmx_msg_liveness') { $nativeObj = $obj } else { $objs += $obj }
}
if (-not $nativeObj) { throw 'the native liveness object was not built' }
# Stage 3c-2a: the production runtime includes the L2 runtime units
# (l2src/l2units_build.ps1; today lmx_sched_record.lm2, profile: runtime).
. l2src/l2units_build.ps1
$objs += @(Build-L2RuntimeUnits -L1Trans $l1trans -Out (Join-Path $out 'l2units') -IncludeDirs @($hdrs) -CFlags $cflags)
$objList = ($objs | ForEach-Object { Q $_ }) -join ' '
$referenceObjs = ($referenceNames | ForEach-Object { Q $objByName[$_] }) -join ' '

$selfSrc = 'l2src/tests/lmx_msg_liveness_selftest.lm1'
$selfC = Join-Path $out 'selftest.c'
Step 'selftest_translate' (Invoke-Native ((Q $l1trans) + ' ' + $selfSrc + ' ' + (Q $selfC)) (Join-Path $out 'selftest.translate.log')) (Join-Path $out 'selftest.translate.log')
$selfObj = Join-Path $out 'selftest.o'
Step 'selftest_compile' (Invoke-Native ("gcc $cflags -I " + (Q $hdrs) + ' -c ' + (Q $selfC) + ' -o ' + (Q $selfObj)) (Join-Path $out 'selftest.gcc.log')) (Join-Path $out 'selftest.gcc.log')

$refExe = Join-Path $out 'reference.exe'
$refLog = Join-Path $out 'reference.stdout.txt'
Step 'reference_link' (Invoke-Native ("gcc $cflags " + (Q $selfObj) + ' ' + $referenceObjs + ' -o ' + (Q $refExe)) (Join-Path $out 'reference.gcc.log')) (Join-Path $out 'reference.gcc.log')
$refExit = Invoke-Native (Q $refExe) $refLog
$refOut = (Get-Content -LiteralPath $refLog -Raw)
if ($refExit -ne 0) { throw "the handwritten module's own selftest failed: exit $refExit`n$refOut" }
if ($refOut -notmatch 'liveness checks=(\d+) failures=0') { throw "reference selftest did not report zero failures: $refOut" }
$refChecks = [int]$Matches[1]
$ev.reference = [ordered]@{ exit = $refExit; stdout = $refOut.Trim(); checks = $refChecks }

$l2c = Join-Path $out 'l2trans.c'
$l2exe = Join-Path $out 'l2trans.exe'
Step 'l2trans_translate' (Invoke-Native ((Q $l1trans) + ' l2src/l2trans.lm1 ' + (Q $l2c)) (Join-Path $out 'l2trans.translate.log')) (Join-Path $out 'l2trans.translate.log')
Step 'l2trans_compile' (Invoke-Native ("gcc $cflags -I lm1/build " + (Q $l2c) + ' l2src/lmx_poll_stub.c -o ' + (Q $l2exe)) (Join-Path $out 'l2trans.gcc.log')) (Join-Path $out 'l2trans.gcc.log')

$lm2 = 'l2src/lmx_msg_liveness.lm2'
$ev.sourceSHA256 = (Get-FileHash -LiteralPath $lm2).Hash
$gen = Join-Path $out 'generated.lm1'
Step 'l2trans_module' (Invoke-Native ((Q $l2exe) + ' ' + (Q $lm2) + ' ' + (Q $gen)) (Join-Path $out 'module.l2trans.log')) (Join-Path $out 'module.l2trans.log')
$genText = [IO.File]::ReadAllText((Resolve-Path -LiteralPath $gen).ProviderPath).Replace("`r`n", "`n")

if ($genText -notmatch 'define: l2_program_entry l2_u[0-9A-F]{16}_entry') { throw 'the generated unit is not a library unit' }
foreach ($sig in @(
    'fn: lmx_msg_ptr_in_block \(const: @\(void p\); const: @\(LmxMsgBlock block\)\) int',
    'fn: lmx_msg_block_has_range \(const: @\(LmxOwnedRange ranges\); const: @\(LmxMsgBlock block\)\) int',
    'fn: lmx_msg_block_is_live \(const: @\(Lmx graph\); const: @\(LmxOwnedRange ranges\); const: @\(LmxMsgBlock block\); const: @\(LmxVisit seen\)\) int')) {
    if ($genText -notmatch $sig) { throw "the public signature is missing or changed: $sig" }
}
# Read-only predicates: no allocator, no store into the owner's storage; the
# membership test is a subtraction, not a base+n comparison.
if ($genText -match 'c\.(malloc|calloc|realloc|free)\(') { throw 'the unit allocates or frees; the handwritten module does not' }
if ($genText -notmatch 'uintptr_t') { throw 'the membership test is not done on uintptr_t' }

$genC = Join-Path $out 'generated.c'
Step 'generated_translate' (Invoke-Native ((Q $l1trans) + ' ' + (Q $gen) + ' ' + (Q $genC)) (Join-Path $out 'generated.translate.log')) (Join-Path $out 'generated.translate.log')
$genObj = Join-Path $out 'generated.o'
Step 'generated_compile' (Invoke-Native ("gcc $cflags -I " + (Q $hdrs) + ' -I lm1/build -c ' + (Q $genC) + ' -o ' + (Q $genObj)) (Join-Path $out 'generated.gcc.log')) (Join-Path $out 'generated.gcc.log')

$nmLog = Join-Path $out 'generated.nm.txt'
Step 'generated_nm' (Invoke-Native ('nm --defined-only ' + (Q $genObj)) $nmLog) $nmLog
$nmText = [IO.File]::ReadAllText($nmLog)
foreach ($sym in @('lmx_msg_ptr_in_block', 'lmx_msg_block_has_range', 'lmx_msg_block_is_live')) {
    if ($nmText -notmatch ('(?m)^\S+\s+T\s+_?' + $sym + '\s*$')) { throw "the generated object does not define $sym" }
}

$driverC = Join-Path $out 'parity_driver.c'
@'
#include <stdio.h>
#include <string.h>
#include "l2src/lmx_msg_liveness.lm1.h"
int liveness_selftest_main(void);
int main(void) {
    char storage[64];
    LmxMsgBlock block;
    memset(&block, 0, sizeof block);
    block.base = storage;
    block.n = sizeof storage;
    /* First call: the library unit opens. */
    if (lmx_msg_ptr_in_block(storage + 3, &block) != 1) { fprintf(stderr, "warm-up: inside\n"); return 90; }
    if (lmx_msg_ptr_in_block(storage + sizeof storage, &block) != 0) { fprintf(stderr, "warm-up: past the end\n"); return 91; }
    if (lmx_msg_ptr_in_block(0, &block) != 0 || lmx_msg_block_has_range(0, &block) != 0) { fprintf(stderr, "warm-up: null guards\n"); return 92; }
    fprintf(stderr, "warm-up ok\n");
    return liveness_selftest_main();
}
'@.Replace("`r`n", "`n") | Set-Content -LiteralPath $driverC -Encoding ascii -NoNewline

$selfDrivenObj = Join-Path $out 'selftest_driven.o'
Step 'selftest_driven_compile' (Invoke-Native ("gcc $cflags -Dmain=liveness_selftest_main -I " + (Q $hdrs) + ' -c ' + (Q $selfC) + ' -o ' + (Q $selfDrivenObj)) (Join-Path $out 'selftest_driven.gcc.log')) (Join-Path $out 'selftest_driven.gcc.log')
$driverObj = Join-Path $out 'parity_driver.o'
Step 'driver_compile' (Invoke-Native ("gcc $cflags -I " + (Q $hdrs) + ' -c ' + (Q $driverC) + ' -o ' + (Q $driverObj)) (Join-Path $out 'driver.gcc.log')) (Join-Path $out 'driver.gcc.log')

$parityExe = Join-Path $out 'parity.exe'
Step 'parity_link' (Invoke-Native ("gcc $cflags " + (Q $driverObj) + ' ' + (Q $selfDrivenObj) + ' ' + (Q $genObj) + ' ' + $objList + ' -o ' + (Q $parityExe)) (Join-Path $out 'parity.gcc.log')) (Join-Path $out 'parity.gcc.log')

$runs = @()
foreach ($i in 1, 2) {
    $log = Join-Path $out "parity.$i.stdout.txt"
    $err = Join-Path $out "parity.$i.stderr.txt"
    cmd /c "`"$parityExe`" > `"$log`" 2> `"$err`""
    $exit = $LASTEXITCODE
    $text = [IO.File]::ReadAllText($log)
    $errText = [IO.File]::ReadAllText($err)
    $runs += [ordered]@{ exit = $exit; stdout = $text.Trim(); stderr = $errText.Trim() }
    if ($errText -notmatch 'warm-up ok') { throw "parity run $i did not open the unit through real calls: $errText" }
    if ($errText -match 'library open failed') { throw "parity run $i reported a library open failure: $errText" }
    if ($exit -ne $refExit) { throw "parity run $i exit $exit, reference exit $refExit`n$text`n$errText" }
    if ($text.Trim() -ne $refOut.Trim()) { throw "parity run $i stdout differs from the reference`nreference: $($refOut.Trim())`nparity   : $($text.Trim())" }
}
if ($runs[0].stdout -ne $runs[1].stdout -or $runs[0].stderr -ne $runs[1].stderr) { throw 'the two parity runs disagree' }
$ev.parity = $runs
$ev.generatedSHA256 = (Get-FileHash -LiteralPath $gen).Hash
$ev.l2transSHA256 = (Get-FileHash -LiteralPath $l2exe).Hash

$evPath = Join-Path $out 'evidence.json'
$ev | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath $evPath -Encoding utf8
Write-Output "lmx_msg_liveness parity PASS: $refChecks checks, 0 failures, reference and generated agree on both runs, real symbols from the generated unit; evidence $evPath"
