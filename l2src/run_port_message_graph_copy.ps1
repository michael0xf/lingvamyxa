# Runtime-module parity runner: lmx_message_graph_copy (review-chat lane, 2026-09-14).
#
# Run the module's selftest (the graph gate's message-copy test: install into
# an empty Message, refusal of a non-empty one, eternal and METHOD terminals
# across Messages, child creation from a copied graph) against the handwritten
# L1 implementation and against one generated from L2 source by the CURRENT
# translator, and require the two to agree exactly, twice.
#
# The generated half is a LIBRARY UNIT (7d7ec87c) exporting the ABI name by
# real symbol: the only runtime caller is Message creation WITH an init graph
# (lmx_message.lm1:1043), and a unit's open path creates its Message without
# one, so nothing reaches the wrapper before it is published. The handwritten
# object is kept OUT of the parity link.
#
# The selftest keeps no allocation counters, so the driver's warm-up (a
# refused call that opens the unit) needs no reset.
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
$out = Join-Path $baseline "build/port_message_graph_copy/$stamp"
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

# ---------------------------------------------------------------------------
# 1. Support objects: the full runtime set. The handwritten
#    lmx_message_graph_copy is built too, but kept OUT of the parity link.
# ---------------------------------------------------------------------------
$names = @('lmx_msg_blocks', 'lmx_owned_ranges', 'lmx_msg_storage', 'lmx_msg_mail_chain', 'lmx_msg_visit', 'lmx_msg_liveness', 'lmx_msg_history_owned', 'lmx_msg_roots_stale', 'lmx_branch_owned', 'lmx_value_owned', 'lmx_chars_owned', 'lmx_array_owned', 'lmx_array_ref_owned', 'lmx_graph_copy_owned', 'lmx_merge_owned', 'lmx_message_graph_copy')
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
$nativeObj = $null
foreach ($source in $sources) {
    $stem = [IO.Path]::GetFileNameWithoutExtension($source)
    $obj = Join-Path $support ($stem + '.o')
    $glog = Join-Path $out ($stem + '.gcc.log')
    Step "compile_$stem" (Invoke-Native ("gcc $cflags -I " + (Q $hdrs) + ' -c ' + (Q $source) + ' -o ' + (Q $obj)) $glog) $glog
    if ($stem -eq 'lmx_message_graph_copy') { $nativeObj = $obj } else { $objs += $obj }
}
if (-not $nativeObj) { throw 'the native message_graph_copy object was not built' }
# Stage 3c-2a: the production runtime includes the L2 runtime units
# (l2src/l2units_build.ps1; today lmx_sched_record.lm2, profile: runtime).
. l2src/l2units_build.ps1
$objs += @(Build-L2RuntimeUnits -L1Trans $l1trans -Out (Join-Path $out 'l2units') -IncludeDirs @($hdrs) -CFlags $cflags)
$objList = ($objs | ForEach-Object { Q $_ }) -join ' '

# ---------------------------------------------------------------------------
# 2. Reference run: the selftest against the handwritten module, linked as the
#    graph gate links it (the full runtime set).
# ---------------------------------------------------------------------------
$selfSrc = 'l2src/tests/lmx_message_graph_copy_selftest.lm1'
$selfC = Join-Path $out 'selftest.c'
Step 'selftest_translate' (Invoke-Native ((Q $l1trans) + ' ' + $selfSrc + ' ' + (Q $selfC)) (Join-Path $out 'selftest.translate.log')) (Join-Path $out 'selftest.translate.log')
$selfObj = Join-Path $out 'selftest.o'
Step 'selftest_compile' (Invoke-Native ("gcc $cflags -I " + (Q $hdrs) + ' -c ' + (Q $selfC) + ' -o ' + (Q $selfObj)) (Join-Path $out 'selftest.gcc.log')) (Join-Path $out 'selftest.gcc.log')

$refExe = Join-Path $out 'reference.exe'
$refLog = Join-Path $out 'reference.stdout.txt'
Step 'reference_link' (Invoke-Native ("gcc $cflags " + (Q $selfObj) + ' ' + (Q $nativeObj) + ' ' + $objList + ' -o ' + (Q $refExe)) (Join-Path $out 'reference.gcc.log')) (Join-Path $out 'reference.gcc.log')
$refExit = Invoke-Native (Q $refExe) $refLog
$refOut = (Get-Content -LiteralPath $refLog -Raw)
if ($refExit -ne 0) { throw "the handwritten module's own selftest failed: exit $refExit`n$refOut" }
if ($refOut -notmatch 'message graph copy selftest: (\d+) checks, 0 failures') { throw "reference selftest did not report zero failures: $refOut" }
$refChecks = [int]$Matches[1]
$ev.reference = [ordered]@{ exit = $refExit; stdout = $refOut.Trim(); checks = $refChecks }

# ---------------------------------------------------------------------------
# 3. The translator from THIS checkout, and the library unit it emits.
# ---------------------------------------------------------------------------
$l2c = Join-Path $out 'l2trans.c'
$l2exe = Join-Path $out 'l2trans.exe'
Step 'l2trans_translate' (Invoke-Native ((Q $l1trans) + ' l2src/l2trans.lm1 ' + (Q $l2c)) (Join-Path $out 'l2trans.translate.log')) (Join-Path $out 'l2trans.translate.log')
Step 'l2trans_compile' (Invoke-Native ("gcc $cflags -I lm1/build " + (Q $l2c) + ' l2src/lmx_poll_stub.c -o ' + (Q $l2exe)) (Join-Path $out 'l2trans.gcc.log')) (Join-Path $out 'l2trans.gcc.log')

$lm2 = 'l2src/lmx_message_graph_copy.lm2'
$ev.sourceSHA256 = (Get-FileHash -LiteralPath $lm2).Hash
$gen = Join-Path $out 'generated.lm1'
Step 'l2trans_module' (Invoke-Native ((Q $l2exe) + ' ' + (Q $lm2) + ' ' + (Q $gen)) (Join-Path $out 'module.l2trans.log')) (Join-Path $out 'module.l2trans.log')
$genText = [IO.File]::ReadAllText((Resolve-Path -LiteralPath $gen).ProviderPath).Replace("`r`n", "`n")

if ($genText -notmatch 'define: l2_program_entry l2_u[0-9A-F]{16}_entry') { throw 'the generated unit is not a library unit' }
if ($genText -notmatch 'fn: lmx_msg_graph_copy_install \(@: LmxMsg dest; @: Lmx source; @: LmxOwnedRange src_ranges; @: LmxOwnedRange eternal_ranges; @: LmxOwnedRange method_ranges; @: char dst_chars\) int') { throw 'the public signature is missing or changed' }
# One copy into the destination's OWN heads, the graph published only after,
# nothing allocated or freed here.
# Scoped to the install method itself: the unit's own entry adapter also
# publishes ITS graph through lmx_msg_set_graph, and a text-wide match would
# be satisfied by that line alone (measured: a tripwire that deleted the
# install's publish line still passed the text-wide check).
$m0 = [regex]::Match($genText, '(?ms)^fn: (l2_u[0-9A-F]{16}_m0) \(@: Lmx node; @: LmxMsg l2_p0_0;.*?^end: \1')
if (-not $m0.Success) { throw 'the install method was not emitted as method 0 with the destination as its first formal' }
if (([regex]::Matches($m0.Value, 'lmx_graph_copy_owned\(')).Count -ne 1) { throw 'install does not copy through exactly one call' }
if ($m0.Value -notmatch '(?m)^\s+c\.lmx_msg_set_graph\(l2_p0_0, ') { throw 'install does not publish the copy into the destination through lmx_msg_set_graph' }
$copyAt = $m0.Value.IndexOf('lmx_graph_copy_owned(')
$setAt = $m0.Value.IndexOf('c.lmx_msg_set_graph(l2_p0_0, ')
if ($setAt -lt $copyAt) { throw 'install publishes the graph before the copy' }
if ($genText -match 'c\.(malloc|calloc|realloc|free)\(') { throw 'install allocates or frees; the handwritten module does not' }

# ---------------------------------------------------------------------------
# 4. The parity binary: the UNMODIFIED selftest by name, the generated unit,
#    the runtime WITHOUT the handwritten object, a driver that opens the unit
#    with a refused call first.
# ---------------------------------------------------------------------------
$genC = Join-Path $out 'generated.c'
Step 'generated_translate' (Invoke-Native ((Q $l1trans) + ' ' + (Q $gen) + ' ' + (Q $genC)) (Join-Path $out 'generated.translate.log')) (Join-Path $out 'generated.translate.log')
$genObj = Join-Path $out 'generated.o'
Step 'generated_compile' (Invoke-Native ("gcc $cflags -I " + (Q $hdrs) + ' -I lm1/build -c ' + (Q $genC) + ' -o ' + (Q $genObj)) (Join-Path $out 'generated.gcc.log')) (Join-Path $out 'generated.gcc.log')

$nmLog = Join-Path $out 'generated.nm.txt'
Step 'generated_nm' (Invoke-Native ('nm --defined-only ' + (Q $genObj)) $nmLog) $nmLog
$nmText = [IO.File]::ReadAllText($nmLog)
if ($nmText -notmatch '(?m)^\S+\s+T\s+_?lmx_msg_graph_copy_install\s*$') { throw 'the generated object does not define lmx_msg_graph_copy_install' }

$driverC = Join-Path $out 'parity_driver.c'
@'
#include <stdio.h>
#include "l2src/lmx_message_graph_copy.lm1.h"
int copy_selftest_main(void);
int main(void) {
    int warm;
    /* First call: the library unit opens; install refuses a null destination. */
    warm = lmx_msg_graph_copy_install(0, 0, 0, 0, 0, 0);
    if (warm != LMX_MSG_INVALID) {
        fprintf(stderr, "warm-up returned %d\n", warm);
        return 90;
    }
    fprintf(stderr, "warm-up ok\n");
    return copy_selftest_main();
}
'@.Replace("`r`n", "`n") | Set-Content -LiteralPath $driverC -Encoding ascii -NoNewline

$selfDrivenObj = Join-Path $out 'selftest_driven.o'
Step 'selftest_driven_compile' (Invoke-Native ("gcc $cflags -Dmain=copy_selftest_main -I " + (Q $hdrs) + ' -c ' + (Q $selfC) + ' -o ' + (Q $selfDrivenObj)) (Join-Path $out 'selftest_driven.gcc.log')) (Join-Path $out 'selftest_driven.gcc.log')
$driverObj = Join-Path $out 'parity_driver.o'
Step 'driver_compile' (Invoke-Native ("gcc $cflags -I " + (Q $hdrs) + ' -c ' + (Q $driverC) + ' -o ' + (Q $driverObj)) (Join-Path $out 'driver.gcc.log')) (Join-Path $out 'driver.gcc.log')

$parityExe = Join-Path $out 'parity.exe'
Step 'parity_link' (Invoke-Native ("gcc $cflags " + (Q $driverObj) + ' ' + (Q $selfDrivenObj) + ' ' + (Q $genObj) + ' ' + $objList + ' -o ' + (Q $parityExe)) (Join-Path $out 'parity.gcc.log')) (Join-Path $out 'parity.gcc.log')

# ---------------------------------------------------------------------------
# 5. Two runs, so a pass that depends on run order or leftover state shows.
# ---------------------------------------------------------------------------
$runs = @()
foreach ($i in 1, 2) {
    $log = Join-Path $out "parity.$i.stdout.txt"
    $err = Join-Path $out "parity.$i.stderr.txt"
    cmd /c "`"$parityExe`" > `"$log`" 2> `"$err`""
    $exit = $LASTEXITCODE
    $text = [IO.File]::ReadAllText($log)
    $errText = [IO.File]::ReadAllText($err)
    $runs += [ordered]@{ exit = $exit; stdout = $text.Trim(); stderr = $errText.Trim() }
    if ($errText -notmatch 'warm-up ok') { throw "parity run $i did not open the unit: $errText" }
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
Write-Output "lmx_message_graph_copy parity PASS: $refChecks checks, 0 failures, reference and generated agree on both runs, real symbol from the generated unit; evidence $evPath"
