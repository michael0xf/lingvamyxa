# Runtime-module parity runner: lmx_array_owned (review-chat lane, 2026-09-14).
#
# Run the module's own selftest against the handwritten L1 implementation and
# against one generated from L2 source by the CURRENT translator, and require
# the two to agree exactly, twice.
#
# The generated half is a LIBRARY UNIT (7d7ec87c). It cannot take the exported
# ABI names: a unit's open path builds its arrays through the array
# constructors, so a wrapper under a real name would re-enter l2_library_open
# before the unit's graph exists. The unit drops the lmx_ prefix, the
# SELFTEST is redirected onto the two public names with -D, the runtime keeps
# the handwritten module under the real names, and nm proves the generated
# object defines the unit names and none of the ABI names.
#
# The selftest instruments malloc/free with --wrap and counts only while its
# `tracking` flag is set. A C driver opens the unit with real calls BEFORE the
# selftest's main, with tracking still off: an empty int Array (descriptor,
# no backing), a positive one (zeroed backing), an unknown type refused, then
# disposed.
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
$out = Join-Path $baseline "build/port_array_owned/$stamp"
$hdrs = Join-Path $out 'hdrs'
$support = Join-Path $out 'support'
New-Item -ItemType Directory -Force -Path $out, (Join-Path $hdrs 'l2src'), $support | Out-Null

$cflags = '-std=c99 -Wall -Wextra -Wpedantic -I . -Werror=incompatible-pointer-types -Werror=discarded-qualifiers -Werror=implicit-function-declaration -Werror=implicit-int'
$wrap = '-Wl,--wrap=malloc -Wl,--wrap=free'

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

$redirects = @(
    @{ abi = 'lmx_array_new_owned'; unit = 'array_new_owned' }
    @{ abi = 'lmx_array_new_positive_owned'; unit = 'array_new_positive_owned' }
)
$redirect = ($redirects | ForEach-Object { '-D' + $_.abi + '=' + $_.unit }) -join ' '

# ---------------------------------------------------------------------------
# 1. Support objects: the full runtime set; the handwritten module stays.
# ---------------------------------------------------------------------------
$names = @('lmx_msg_blocks', 'lmx_owned_ranges', 'lmx_msg_storage', 'lmx_msg_path_storage', 'lmx_msg_slots', 'lmx_msg_mail_chain', 'lmx_msg_sched_ready', 'lmx_msg_visit', 'lmx_msg_liveness', 'lmx_msg_history_owned', 'lmx_msg_roots_stale', 'lmx_branch_owned', 'lmx_value_owned', 'lmx_chars_owned', 'lmx_array_owned', 'lmx_array_ref_owned', 'lmx_graph_copy_owned', 'lmx_merge_owned', 'lmx_message_graph_copy')
$referenceNames = @('lmx_msg_blocks', 'lmx_owned_ranges', 'lmx_msg_storage', 'lmx_array_owned', 'lmx_value_owned')
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
foreach ($source in $sources) {
    $stem = [IO.Path]::GetFileNameWithoutExtension($source)
    $obj = Join-Path $support ($stem + '.o')
    $glog = Join-Path $out ($stem + '.gcc.log')
    Step "compile_$stem" (Invoke-Native ("gcc $cflags -I " + (Q $hdrs) + ' -c ' + (Q $source) + ' -o ' + (Q $obj)) $glog) $glog
    $objs += $obj
    $objByName[$stem] = $obj
}
$objList = ($objs | ForEach-Object { Q $_ }) -join ' '
$referenceObjs = ($referenceNames | ForEach-Object { Q $objByName[$_] }) -join ' '

# ---------------------------------------------------------------------------
# 2. Reference run, linked as l2src/run_array_owned.ps1 links it.
# ---------------------------------------------------------------------------
$selfSrc = 'l2src/tests/lmx_array_owned_selftest.lm1'
$selfC = Join-Path $out 'selftest.c'
Step 'selftest_translate' (Invoke-Native ((Q $l1trans) + ' ' + $selfSrc + ' ' + (Q $selfC)) (Join-Path $out 'selftest.translate.log')) (Join-Path $out 'selftest.translate.log')

$refExe = Join-Path $out 'reference.exe'
$refLog = Join-Path $out 'reference.stdout.txt'
Step 'reference_compile' (Invoke-Native ("gcc $cflags -I " + (Q $hdrs) + ' ' + (Q $selfC) + ' ' + $referenceObjs + " $wrap -o " + (Q $refExe)) (Join-Path $out 'reference.gcc.log')) (Join-Path $out 'reference.gcc.log')
$refExit = Invoke-Native (Q $refExe) $refLog
$refOut = (Get-Content -LiteralPath $refLog -Raw)
if ($refExit -ne 0) { throw "the handwritten module's own selftest failed: exit $refExit`n$refOut" }
if ($refOut -notmatch 'array_owned checks=(\d+) failures=0 allocations=(\d+) releases=(\d+)') { throw "reference selftest did not report zero failures: $refOut" }
$refChecks = [int]$Matches[1]
if ([int]$Matches[2] -ne [int]$Matches[3]) { throw "reference allocations $($Matches[2]) and releases $($Matches[3]) differ" }
if ([int]$Matches[2] -lt 1) { throw 'the malloc wrap is not in force: the reference counted no allocations' }
$ev.reference = [ordered]@{ exit = $refExit; stdout = $refOut.Trim(); checks = $refChecks; allocations = [int]$Matches[2] }

# ---------------------------------------------------------------------------
# 3. The translator from THIS checkout, and the library unit it emits.
# ---------------------------------------------------------------------------
$l2c = Join-Path $out 'l2trans.c'
$l2exe = Join-Path $out 'l2trans.exe'
Step 'l2trans_translate' (Invoke-Native ((Q $l1trans) + ' l2src/l2trans.lm1 ' + (Q $l2c)) (Join-Path $out 'l2trans.translate.log')) (Join-Path $out 'l2trans.translate.log')
Step 'l2trans_compile' (Invoke-Native ("gcc $cflags -I lm1/build " + (Q $l2c) + ' l2src/lmx_poll_stub.c -o ' + (Q $l2exe)) (Join-Path $out 'l2trans.gcc.log')) (Join-Path $out 'l2trans.gcc.log')

$lm2 = 'l2src/lmx_array_owned.lm2'
$ev.sourceSHA256 = (Get-FileHash -LiteralPath $lm2).Hash
$gen = Join-Path $out 'generated.lm1'
Step 'l2trans_module' (Invoke-Native ((Q $l2exe) + ' ' + (Q $lm2) + ' ' + (Q $gen)) (Join-Path $out 'module.l2trans.log')) (Join-Path $out 'module.l2trans.log')
$genText = [IO.File]::ReadAllText((Resolve-Path -LiteralPath $gen).ProviderPath).Replace("`r`n", "`n")

if ($genText -notmatch 'define: l2_program_entry l2_u[0-9A-F]{16}_entry') { throw 'the generated unit is not a library unit' }
foreach ($sig in @(
    'fn: array_new_owned \(int: type; size_t: count; @@: LmxMsgBlock blocks; @@: LmxOwnedRange ranges\) @: LmxArrayDesc',
    'fn: array_new_positive_owned \(int: type; size_t: count; @@: LmxMsgBlock blocks; @@: LmxOwnedRange ranges\) @: LmxArrayDesc')) {
    if ($genText -notmatch $sig) { throw "the public signature is missing or changed: $sig" }
}
# One preparation path (the range record in front of a zeroed payload), two
# allocations and one release site in it, one memset, one storage-move
# admission, one loop (retiring prepared ranges on discard), no calloc or
# realloc; every element stride from a C type or a probe, never a literal;
# the descriptor's len and data are set before admission.
if ($genText -notmatch 'c\.sizeof\(c\.LmxOwnedRange\)') { throw 'the record offset is not sizeof the range record' }
if ($genText -notmatch 'c\.sizeof\(c\.LmxArrayDesc\)') { throw 'the descriptor size is not sizeof the record' }
foreach ($sz in @('c\.sizeof\(c\.char\)', 'c\.sizeof\(c\.int\)', 'c\.sizeof\(c\.size_t\)')) { if ($genText -notmatch $sz) { throw "an element stride is not taken from its C type: $sz" } }
if (([regex]::Matches($genText, 'c\.malloc\(')).Count -ne 2) { throw 'the unit does not allocate exactly twice' }
if (([regex]::Matches($genText, 'c\.free\(')).Count -ne 1) { throw 'the unit does not have exactly one direct release site' }
if (([regex]::Matches($genText, 'c\.memset\(')).Count -ne 1) { throw 'the payload is not zeroed exactly once' }
if (([regex]::Matches($genText, 'lmx_msg_storage_move_all\(')).Count -ne 1) { throw 'the unit does not admit through exactly one storage move' }
if (([regex]::Matches($genText, '(?m)^\s*while:')).Count -ne 1) { throw 'the unit does not have exactly one loop (the discard retirement)' }
if ($genText -match 'c\.(calloc|realloc)\(') { throw 'the unit uses an allocator the handwritten module does not' }
if ($genText -notmatch 'lmx_owned_ranges_remove\(' -or $genText -notmatch 'lmx_msg_blocks_dispose_all\(') { throw 'discard does not retire ranges then dispose blocks' }

# ---------------------------------------------------------------------------
# 4. The parity binary.
# ---------------------------------------------------------------------------
$genC = Join-Path $out 'generated.c'
Step 'generated_translate' (Invoke-Native ((Q $l1trans) + ' ' + (Q $gen) + ' ' + (Q $genC)) (Join-Path $out 'generated.translate.log')) (Join-Path $out 'generated.translate.log')
$genObj = Join-Path $out 'generated.o'
Step 'generated_compile' (Invoke-Native ("gcc $cflags -I " + (Q $hdrs) + ' -I lm1/build -c ' + (Q $genC) + ' -o ' + (Q $genObj)) (Join-Path $out 'generated.gcc.log')) (Join-Path $out 'generated.gcc.log')

$nmLog = Join-Path $out 'generated.nm.txt'
Step 'generated_nm' (Invoke-Native ('nm --defined-only ' + (Q $genObj)) $nmLog) $nmLog
$nmText = [IO.File]::ReadAllText($nmLog)
foreach ($r in $redirects) {
    if ($nmText -notmatch ('(?m)^\S+\s+T\s+_?' + $r.unit + '\s*$')) { throw "the generated object does not define $($r.unit)" }
    if ($nmText -match ('(?m)^\S+\s+T\s+_?' + $r.abi + '\s*$')) { throw "the generated object defines the ABI name $($r.abi); the runtime's definition would be duplicated" }
}

$driverC = Join-Path $out 'parity_driver.c'
@'
#include <stdio.h>
#include "l2src/lmx_array_owned.lm1.h"
#include "l2src/lmx_msg_blocks.lm1.h"
int array_selftest_main(void);
int main(void) {
    LmxMsgBlock *blocks = 0;
    LmxOwnedRange *ranges = 0;
    LmxArrayDesc *empty;
    LmxArrayDesc *three;
    int *cells;
    /* Redirected onto the generated unit; the first call opens the library
       while the selftest's tracking flag is still off. */
    empty = lmx_array_new_owned(LMX_TYPE_ARRAY_OF_INT, 0, &blocks, &ranges);
    if (empty == 0 || empty->len != 0 || empty->data != 0) { fprintf(stderr, "warm-up: empty array\n"); return 90; }
    three = lmx_array_new_owned(LMX_TYPE_ARRAY_OF_INT, 3, &blocks, &ranges);
    if (three == 0 || three->len != 3 || three->data == 0) { fprintf(stderr, "warm-up: positive array\n"); return 91; }
    cells = (int *)three->data;
    if (cells[0] != 0 || cells[1] != 0 || cells[2] != 0) { fprintf(stderr, "warm-up: backing not zeroed\n"); return 92; }
    if (lmx_array_new_owned(LMX_TYPE_ARRAY_OF_ULONG - 1000, 1, &blocks, &ranges) != 0) { fprintf(stderr, "warm-up: unknown type not refused\n"); return 93; }
    if (lmx_array_new_positive_owned(LMX_TYPE_ARRAY_OF_INT, 0, &blocks, &ranges) != 0) { fprintf(stderr, "warm-up: positive constructor accepted zero\n"); return 94; }
    lmx_msg_blocks_dispose_all(&blocks);
    fprintf(stderr, "warm-up ok\n");
    return array_selftest_main();
}
'@.Replace("`r`n", "`n") | Set-Content -LiteralPath $driverC -Encoding ascii -NoNewline

$selfDrivenObj = Join-Path $out 'selftest_driven.o'
Step 'selftest_driven_compile' (Invoke-Native ("gcc $cflags $redirect -Dmain=array_selftest_main -I " + (Q $hdrs) + ' -c ' + (Q $selfC) + ' -o ' + (Q $selfDrivenObj)) (Join-Path $out 'selftest_driven.gcc.log')) (Join-Path $out 'selftest_driven.gcc.log')
$driverObj = Join-Path $out 'parity_driver.o'
Step 'driver_compile' (Invoke-Native ("gcc $cflags $redirect -I " + (Q $hdrs) + ' -c ' + (Q $driverC) + ' -o ' + (Q $driverObj)) (Join-Path $out 'driver.gcc.log')) (Join-Path $out 'driver.gcc.log')

$parityExe = Join-Path $out 'parity.exe'
Step 'parity_link' (Invoke-Native ("gcc $cflags " + (Q $driverObj) + ' ' + (Q $selfDrivenObj) + ' ' + (Q $genObj) + ' ' + $objList + " $wrap -o " + (Q $parityExe)) (Join-Path $out 'parity.gcc.log')) (Join-Path $out 'parity.gcc.log')

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
Write-Output "lmx_array_owned parity PASS: $refChecks checks, 0 failures, $($ev.reference.allocations) counted allocations released, reference and generated agree on both runs, selftest redirected onto the unit's names; evidence $evPath"
