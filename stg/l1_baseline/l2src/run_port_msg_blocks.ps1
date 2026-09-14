# Runtime-module parity runner: lmx_msg_blocks (Fable lane, 2026-09-13).
#
# Run the module's own selftest against the handwritten L1 implementation and
# against one generated from L2 source by the CURRENT translator, and require
# the two to agree exactly, twice.
#
# The generated half is a LIBRARY UNIT (7d7ec87c): no entry, module-unique
# internals, public wrappers carrying the source signatures. No splice.
#
# It cannot take the exported ABI names, and the reason is measured: every
# arena allocation calls lmx_msg_storage_move_all, which calls
# lmx_msg_blocks_can_move and lmx_msg_blocks_move_all. A library unit named
# lmx_msg_blocks_* would call l2_library_open on first use, which allocates,
# which calls it again. So the unit names its operations blocks_* and the
# SELFTEST is redirected onto them; the runtime keeps the handwritten module,
# which is also what lets l2_library_open build this unit's own graph.
#
# This module DISPOSES: it calls the disposer a node carries, or free. The
# selftest wraps free and counts both the default disposal and the custom
# callback, so those counts are part of the comparison rather than decoration.
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

$stamp = (Get-Date).ToString('yyyyMMdd_HHmmss_fff')
$out = Join-Path $baseline "build/port_msg_blocks/$stamp"
$hdrs = Join-Path $out 'hdrs'
New-Item -ItemType Directory -Force -Path $out, (Join-Path $hdrs 'l2src') | Out-Null

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

$ev = [ordered]@{ stamp = $stamp; baseline = $baseline; translator = $l1trans }

# ---------------------------------------------------------------------------
# 0. The status literals ARE the header's defines. L2 has no define form, so
#    the identity is checked here before anything is built.
# ---------------------------------------------------------------------------
$defines = @(
    @{ name = 'LMX_MSG_BLOCKS_OK'; value = 0 }
    @{ name = 'LMX_MSG_BLOCKS_INVALID'; value = 1 }
)
# The selftest calls the exported ABI names; the library unit defines its own.
# Every exported operation must be redirected, or the test would quietly keep
# measuring the handwritten module.
$defineNames = @(
    @{ abi = 'lmx_msg_blocks_push'; unit = 'blocks_push' }
    @{ abi = 'lmx_msg_blocks_remove'; unit = 'blocks_remove' }
    @{ abi = 'lmx_msg_blocks_can_move'; unit = 'blocks_can_move' }
    @{ abi = 'lmx_msg_blocks_move_all'; unit = 'blocks_move_all' }
    @{ abi = 'lmx_msg_blocks_dispose_all'; unit = 'blocks_dispose_all' }
)
$headerText = [IO.File]::ReadAllText((Resolve-Path -LiteralPath 'l2src/lmx_msg_blocks.h.lm1').ProviderPath)
foreach ($d in $defines) {
    $m = [regex]::Match($headerText, '(?m)^define:\s+' + [regex]::Escape($d.name) + '\s+(-?\d+)\s*$')
    if (-not $m.Success) { throw "l2src/lmx_msg_blocks.h.lm1 no longer defines $($d.name)" }
    if ([int]$m.Groups[1].Value -ne $d.value) { throw "$($d.name) is $($m.Groups[1].Value), but l2src/lmx_msg_blocks.lm2 spells it $($d.value)" }
}

# ---------------------------------------------------------------------------
# 1. The ORACLE: the handwritten module and its own selftest, unchanged.
#    This module allocates nothing and frees nothing -- verified below on the
#    generated object, not assumed -- so it has no allocation-failure path of
#    its own and its selftest wraps no allocator.
# ---------------------------------------------------------------------------
$header = Join-Path $hdrs 'l2src/lmx_msg_blocks.lm1.h'
$module = Join-Path $out 'lmx_msg_blocks.c'
$selfC = Join-Path $out 'selftest.c'
Step 'header' (Invoke-Native ((Q $l1trans) + ' l2src/lmx_msg_blocks.h.lm1 ' + (Q $header)) (Join-Path $out 'header.log')) (Join-Path $out 'header.log')
Step 'module' (Invoke-Native ((Q $l1trans) + ' l2src/lmx_msg_blocks.lm1 ' + (Q $module)) (Join-Path $out 'module.log')) (Join-Path $out 'module.log')
Step 'selftest_translate' (Invoke-Native ((Q $l1trans) + ' l2src/tests/lmx_msg_blocks_selftest.lm1 ' + (Q $selfC)) (Join-Path $out 'selftest.translate.log')) (Join-Path $out 'selftest.translate.log')

$refExe = Join-Path $out 'reference.exe'
$refLog = Join-Path $out 'reference.stdout.txt'
Step 'reference_compile' (Invoke-Native ("gcc $cflags -I " + (Q $hdrs) + ' ' + (Q $selfC) + ' ' + (Q $module) + ' -Wl,--wrap=free -o ' + (Q $refExe)) (Join-Path $out 'reference.gcc.log')) (Join-Path $out 'reference.gcc.log')
$refExit = Invoke-Native (Q $refExe) $refLog
$refOut = (Get-Content -LiteralPath $refLog -Raw)
if ($refExit -ne 0) { throw "the handwritten module's own selftest failed: exit $refExit`n$refOut" }
if ($refOut -notmatch 'blocks checks=(\d+) failures=0 frees=(\d+) callbacks=(\d+)') { throw "reference selftest did not report zero failures: $refOut" }
$refChecks = [int]$Matches[1]
$refFrees = [int]$Matches[2]
$refCallbacks = [int]$Matches[3]
# Without the wrap nothing is counted and the disposal half proves nothing.
if ($refFrees -lt 1 -or $refCallbacks -lt 1) { throw "frees=$refFrees callbacks=${refCallbacks}: the free wrap is not in force, so disposal is unverified" }
$ev.reference = [ordered]@{ exit = $refExit; stdout = $refOut.Trim(); checks = $refChecks; frees = $refFrees; callbacks = $refCallbacks }

# ---------------------------------------------------------------------------
# 2. The translator from THIS checkout, and the library unit it emits.
# ---------------------------------------------------------------------------
$l2c = Join-Path $out 'l2trans.c'
$l2exe = Join-Path $out 'l2trans.exe'
Step 'l2trans_translate' (Invoke-Native ((Q $l1trans) + ' l2src/l2trans.lm1 ' + (Q $l2c)) (Join-Path $out 'l2trans.translate.log')) (Join-Path $out 'l2trans.translate.log')
Step 'l2trans_compile' (Invoke-Native ("gcc $cflags -I lm1/build " + (Q $l2c) + ' l2src/lmx_poll_stub.c -o ' + (Q $l2exe)) (Join-Path $out 'l2trans.gcc.log')) (Join-Path $out 'l2trans.gcc.log')

$lm2 = 'l2src/lmx_msg_blocks.lm2'
$ev.sourceSHA256 = (Get-FileHash -LiteralPath $lm2).Hash
$gen = Join-Path $out 'generated.lm1'
Step 'l2trans_module' (Invoke-Native ((Q $l2exe) + ' ' + (Q $lm2) + ' ' + (Q $gen)) (Join-Path $out 'module.l2trans.log')) (Join-Path $out 'module.l2trans.log')
$genText = [IO.File]::ReadAllText((Resolve-Path -LiteralPath $gen).ProviderPath).Replace("`r`n", "`n")

# It is a library unit, and its public signatures are the module's own.
if ($genText -notmatch 'define: l2_program_entry l2_u[0-9A-F]{16}_entry') { throw 'the generated unit is not a library unit' }
foreach ($sig in @(
    'fn: blocks_tail \(@: LmxMsgBlock head; @@: LmxMsgBlock tail\) int',
    'fn: blocks_push \(@@: LmxMsgBlock head; @: LmxMsgBlock item\) int',
    'fn: blocks_remove \(@@: LmxMsgBlock head; @: LmxMsgBlock item\) int',
    'fn: blocks_can_move \(@@: LmxMsgBlock dst; @@: LmxMsgBlock src\) int',
    'fn: blocks_move_all \(@@: LmxMsgBlock dst; @@: LmxMsgBlock src\) int',
    'fn: blocks_dispose_all \(@@: LmxMsgBlock head\) int')) {
    if ($genText -notmatch $sig) { throw "the public signature is missing or changed: $sig" }
}
# The disposer is read out of the node and CALLED. An assignment here would be
# the same parse tree as the call, which is why it is declared with its value.
if ($genText -notmatch 'LmxMsgBlockDisposer: disposer item\\dispose') { throw 'the disposer is not taken from the node it belongs to' }
if ($genText -notmatch 'disposer\(item\\base, item\\n\)') { throw 'the disposer is not called with the payload it owns' }
if ($genText -notmatch 'c\.free\(item\\base\)') { throw 'the default disposal does not free the payload' }
if ($genText -notmatch 'c\.free\(item\)') { throw 'the node itself is not freed' }
# The list is detached BEFORE any disposer runs, so a repeated cleanup sees an
# empty list rather than freed nodes.
$dispose = [regex]::Match($genText, '(?ms)^fn: l2_u[0-9A-F]{16}_m5 \(.*?^end: l2_u[0-9A-F]{16}_m5')
if (-not $dispose.Success) { throw 'the disposal method was not emitted' }
# Either spelling of the owner slot detaches the list: the legacy `x[0]: 0`
# or the strict prefix raw store `\x: 0`. The property asserted below is the
# ORDER -- detach before the first disposal -- not the syntax.
$clearMatch = [regex]::Match($dispose.Value, '(l2_p5_0\[0\]|\\l2_p5_0): 0')
$clear = if ($clearMatch.Success) { $clearMatch.Index } else { -1 }
$firstFree = $dispose.Value.IndexOf('c.free(')
if ($clear -lt 0 -or $firstFree -lt 0 -or $clear -gt $firstFree) { throw 'the owner list is not detached before the first disposal' }
# Cycles are refused before anything is written.
if ($genText -notmatch 'break') { throw 'the cycle detector lost its early exit' }

# ---------------------------------------------------------------------------
# 3. The parity binary: the GENERATED implementation under the selftest, with
#    the selftest's call sites redirected onto the library unit's names. The
#    handwritten module stays linked for the runtime that builds the unit.
# ---------------------------------------------------------------------------
$genC = Join-Path $out 'generated.c'
Step 'generated_translate' (Invoke-Native ((Q $l1trans) + ' ' + (Q $gen) + ' ' + (Q $genC)) (Join-Path $out 'generated.translate.log')) (Join-Path $out 'generated.translate.log')

# The generated unit builds a Message-owned graph on first use, so it needs the
# Message runtime the graph gate links.
$names = @('lmx_msg_blocks', 'lmx_owned_ranges', 'lmx_msg_storage', 'lmx_msg_path_storage', 'lmx_msg_slots', 'lmx_msg_mail_chain', 'lmx_msg_sched_ready', 'lmx_msg_visit', 'lmx_msg_liveness', 'lmx_msg_history_owned', 'lmx_msg_roots_stale', 'lmx_branch_owned', 'lmx_value_owned', 'lmx_chars_owned', 'lmx_array_owned', 'lmx_array_ref_owned', 'lmx_graph_copy_owned', 'lmx_merge_owned', 'lmx_message_graph_copy')
$sources = @('l2src/lmx_message_host.c', 'l2src/lmx_message_exec.c')
foreach ($name in $names) {
    if ($name -ne 'lmx_msg_blocks') {
        Step "header_$name" (Invoke-Native ((Q $l1trans) + " l2src/$name.h.lm1 " + (Q (Join-Path $hdrs "l2src/$name.lm1.h"))) (Join-Path $out "header_$name.log")) (Join-Path $out "header_$name.log")
    }
    $src = Join-Path $out "$name.c"
    Step "module_$name" (Invoke-Native ((Q $l1trans) + " l2src/$name.lm1 " + (Q $src)) (Join-Path $out "module_$name.log")) (Join-Path $out "module_$name.log")
    $sources += $src
}
$messageSource = Join-Path $out 'lmx_message.c'
Step 'module_lmx_message' (Invoke-Native ((Q $l1trans) + ' l2src/lmx_message.lm1 ' + (Q $messageSource)) (Join-Path $out 'module_lmx_message.log')) (Join-Path $out 'module_lmx_message.log')
$sources += $messageSource

$objs = @()
foreach ($src in $sources) {
    $stem = [IO.Path]::GetFileNameWithoutExtension($src)
    $obj = Join-Path $out ($stem + '.o')
    $glog = Join-Path $out ($stem + '.gcc.log')
    Step "compile_$stem" (Invoke-Native ("gcc $cflags -I " + (Q $hdrs) + ' -c ' + (Q $src) + ' -o ' + (Q $obj)) $glog) $glog
    $objs += $obj
}
# Stage 3c-2a: the production runtime includes the L2 runtime units
# (l2src/l2units_build.ps1; today lmx_sched_record.lm2, profile: runtime).
. l2src/l2units_build.ps1
$objs += @(Build-L2RuntimeUnits -L1Trans $l1trans -Out (Join-Path $out 'l2units') -IncludeDirs @($hdrs) -CFlags $cflags)
$objList = ($objs | ForEach-Object { Q $_ }) -join ' '

$redirect = ($defineNames | ForEach-Object { '-D' + $_.abi + '=' + $_.unit }) -join ' '
$parityExe = Join-Path $out 'parity.exe'
Step 'generated_compile' (Invoke-Native ("gcc $cflags -I " + (Q $hdrs) + ' -I lm1/build -c ' + (Q $genC) + ' -o ' + (Q (Join-Path $out 'generated.o'))) (Join-Path $out 'generated.gcc.log')) (Join-Path $out 'generated.gcc.log')
Step 'selftest_driven_compile' (Invoke-Native ("gcc $cflags $redirect -I " + (Q $hdrs) + ' -c ' + (Q $selfC) + ' -o ' + (Q (Join-Path $out 'selftest_driven.o'))) (Join-Path $out 'selftest_driven.gcc.log')) (Join-Path $out 'selftest_driven.gcc.log')
Step 'parity_link' (Invoke-Native ("gcc $cflags -I " + (Q $hdrs) + ' ' + (Q (Join-Path $out 'selftest_driven.o')) + ' ' + (Q (Join-Path $out 'generated.o')) + ' ' + $objList + ' -Wl,--wrap=free -o ' + (Q $parityExe)) (Join-Path $out 'parity.gcc.log')) (Join-Path $out 'parity.gcc.log')

# ---------------------------------------------------------------------------
# 4. Two runs, so a pass that depends on run order or leftover state shows.
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
    if ($exit -ne $refExit) { throw "parity run $i exit $exit, reference exit $refExit`n$text`n$errText" }
    if ($text.Trim() -ne $refOut.Trim()) { throw "parity run $i stdout differs from the reference`nreference: $($refOut.Trim())`nparity   : $($text.Trim())" }
}
if ($runs[0].stdout -ne $runs[1].stdout) { throw 'the two parity runs disagree' }
$ev.parity = $runs
$ev.generatedSHA256 = (Get-FileHash -LiteralPath $gen).Hash
$ev.l2transSHA256 = (Get-FileHash -LiteralPath $l2exe).Hash

$evPath = Join-Path $out 'evidence.json'
$ev | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath $evPath -Encoding utf8
Write-Output "lmx_msg_blocks parity PASS: $refChecks checks, 0 failures, $refFrees frees, $refCallbacks disposer callbacks, reference and generated agree on both runs; evidence $evPath"
