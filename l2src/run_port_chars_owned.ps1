# Runtime-module parity runner: lmx_chars_owned (review-chat lane, 2026-09-14).
#
# Run the module's own selftest against the handwritten L1 implementation and
# against one generated from L2 source by the CURRENT translator, and require
# the two to agree exactly, twice.
#
# The generated half is a LIBRARY UNIT (7d7ec87c). It cannot take the
# exported ABI names, and the reason is measured: Message creation calls
# lmx_chars_new_owned (lmx_message.lm1:1034), and a unit's open path creates a
# Message, so a wrapper carrying the real name would re-enter l2_library_open
# before the unit's graph exists. So the unit names its operations chars_* /
# char_*, the SELFTEST is redirected onto them with -D, and the runtime keeps
# the handwritten module, which is also what lets l2_library_open build this
# unit's own graph. Same shape as lmx_msg_blocks and lmx_owned_ranges.
#
# The selftest instruments malloc/free with --wrap and counts only while its
# `tracking` flag is set. The unit's open path allocates on the first call;
# a C driver makes that first call BEFORE the selftest's main, with tracking
# still off, so the open costs the counters nothing. The warm-up is a real
# call (non-null result, then disposed), which also proves the open succeeded
# rather than returning the wrapper's zero.
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
$out = Join-Path $baseline "build/port_chars_owned/$stamp"
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

# Every exported operation is redirected, or the test would quietly keep
# measuring the handwritten module.
$redirects = @(
    @{ abi = 'lmx_chars_new_owned'; unit = 'chars_new_owned' }
    @{ abi = 'lmx_char_cell_known'; unit = 'char_cell_known' }
    @{ abi = 'lmx_char_rebind_known'; unit = 'char_rebind_known' }
)
$redirect = ($redirects | ForEach-Object { '-D' + $_.abi + '=' + $_.unit }) -join ' '

# ---------------------------------------------------------------------------
# 1. Support objects: the full runtime set the unit's open path needs. The
#    handwritten lmx_chars_owned stays in every link; it is what the runtime
#    calls by the real names.
# ---------------------------------------------------------------------------
$names = @('lmx_msg_blocks', 'lmx_owned_ranges', 'lmx_msg_storage', 'lmx_msg_path_storage', 'lmx_msg_mail_chain', 'lmx_msg_visit', 'lmx_msg_liveness', 'lmx_msg_history_owned', 'lmx_msg_roots_stale', 'lmx_branch_owned', 'lmx_value_owned', 'lmx_chars_owned', 'lmx_array_owned', 'lmx_array_ref_owned', 'lmx_graph_copy_owned', 'lmx_merge_owned', 'lmx_message_graph_copy')
$referenceNames = @('lmx_msg_blocks', 'lmx_owned_ranges', 'lmx_msg_storage', 'lmx_chars_owned')
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
# Stage 3c-2a: the production runtime includes the L2 runtime units
# (l2src/l2units_build.ps1; today lmx_sched_record.lm2, profile: runtime).
. l2src/l2units_build.ps1
$objs += @(Build-L2RuntimeUnits -L1Trans $l1trans -Out (Join-Path $out 'l2units') -IncludeDirs @($hdrs) -CFlags $cflags)
$objList = ($objs | ForEach-Object { Q $_ }) -join ' '
$referenceObjs = ($referenceNames | ForEach-Object { Q $objByName[$_] }) -join ' '

# ---------------------------------------------------------------------------
# 2. Reference run: the existing selftest against the handwritten module,
#    linked as l2src/run_chars_owned.ps1 links it.
# ---------------------------------------------------------------------------
$selfSrc = 'l2src/tests/lmx_chars_owned_selftest.lm1'
$selfC = Join-Path $out 'selftest.c'
Step 'selftest_translate' (Invoke-Native ((Q $l1trans) + ' ' + $selfSrc + ' ' + (Q $selfC)) (Join-Path $out 'selftest.translate.log')) (Join-Path $out 'selftest.translate.log')

$refExe = Join-Path $out 'reference.exe'
$refLog = Join-Path $out 'reference.stdout.txt'
Step 'reference_compile' (Invoke-Native ("gcc $cflags -I " + (Q $hdrs) + ' ' + (Q $selfC) + ' ' + $referenceObjs + " $wrap -o " + (Q $refExe)) (Join-Path $out 'reference.gcc.log')) (Join-Path $out 'reference.gcc.log')
$refExit = Invoke-Native (Q $refExe) $refLog
$refOut = (Get-Content -LiteralPath $refLog -Raw)
if ($refExit -ne 0) { throw "the handwritten module's own selftest failed: exit $refExit`n$refOut" }
if ($refOut -notmatch 'chars_owned checks=(\d+) failures=0 allocations=(\d+) releases=(\d+)') { throw "reference selftest did not report zero failures: $refOut" }
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

$lm2 = 'l2src/lmx_chars_owned.lm2'
$ev.sourceSHA256 = (Get-FileHash -LiteralPath $lm2).Hash
$gen = Join-Path $out 'generated.lm1'
Step 'l2trans_module' (Invoke-Native ((Q $l2exe) + ' ' + (Q $lm2) + ' ' + (Q $gen)) (Join-Path $out 'module.l2trans.log')) (Join-Path $out 'module.l2trans.log')
$genText = [IO.File]::ReadAllText((Resolve-Path -LiteralPath $gen).ProviderPath).Replace("`r`n", "`n")

if ($genText -notmatch 'define: l2_program_entry l2_u[0-9A-F]{16}_entry') { throw 'the generated unit is not a library unit' }
foreach ($sig in @(
    'fn: chars_new_owned \(@@: LmxMsgBlock blocks; @@: LmxOwnedRange ranges\) @: char',
    'fn: char_cell_known \(@: char table; int: value\) @: void',
    'fn: char_rebind_known \(@: void cell; int: value\) @: void')) {
    if ($genText -notmatch $sig) { throw "the public signature is missing or changed: $sig" }
}
# The table lives behind its own range record: the offset is sizeof the record,
# not a literal; two allocations (the block node, the payload) and three
# releases (the two failure paths and the admission failure); no calloc,
# realloc or memset; the bytes are filled by a loop.
if ($genText -notmatch 'c\.sizeof\(c\.LmxOwnedRange\)') { throw 'the table offset is not sizeof the range record' }
if (([regex]::Matches($genText, 'c\.malloc\(')).Count -ne 2) { throw 'the unit does not allocate exactly twice' }
if (([regex]::Matches($genText, 'c\.free\(')).Count -ne 3) { throw 'the unit does not have exactly three release sites' }
if ($genText -match 'c\.(calloc|realloc|memset)\(') { throw 'the unit uses an allocator the handwritten module does not' }
$m0 = [regex]::Match($genText, '(?ms)^fn: (l2_u[0-9A-F]{16}_m0) \(.*?^end: \1')
if (-not $m0.Success) { throw 'the constructor method was not emitted' }
if ($m0.Value -notmatch 'while:') { throw 'the constructor does not fill the table by a loop' }
if ($m0.Value -notmatch 'lmx_msg_storage_move_all\(') { throw 'the constructor does not admit through storage move' }
# cell and rebind never write.
foreach ($mi in 1, 2) {
    $m = [regex]::Match($genText, "(?ms)^fn: (l2_u[0-9A-F]{16}_m$mi) \(.*?^end: \1")
    if (-not $m.Success) { throw "method $mi was not emitted" }
    if ($m.Value -match '(?m)^\s*[a-z_\\]+\[[^\]]+\]: ') { throw "method $mi stores into the table" }
}

# ---------------------------------------------------------------------------
# 4. The parity binary: the selftest with its call sites redirected onto the
#    unit's names and its main renamed; a C driver that opens the unit first
#    with a real call; the generated unit; the full runtime including the
#    handwritten module under its real names; malloc/free wrapped as in the
#    reference.
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
#include "l2src/lmx_chars_owned.lm1.h"
#include "l2src/lmx_msg_blocks.lm1.h"
int chars_selftest_main(void);
int main(void) {
    LmxMsgBlock *blocks = 0;
    LmxOwnedRange *ranges = 0;
    char *table;
    /* First call: the library unit opens and builds its graph while the
       selftest's tracking flag is still off. A real call, so a wrapper zero
       from a failed open cannot pass for a refusal. */
    table = lmx_chars_new_owned(&blocks, &ranges);
    if (table == 0 || blocks == 0 || ranges == 0) {
        fprintf(stderr, "warm-up failed\n");
        return 90;
    }
    if (lmx_char_cell_known(table, 65) != (void *)(table + 65)) {
        fprintf(stderr, "warm-up cell mismatch\n");
        return 91;
    }
    lmx_msg_blocks_dispose_all(&blocks);
    fprintf(stderr, "warm-up ok\n");
    return chars_selftest_main();
}
'@.Replace("`r`n", "`n") | Set-Content -LiteralPath $driverC -Encoding ascii -NoNewline

$selfDrivenObj = Join-Path $out 'selftest_driven.o'
Step 'selftest_driven_compile' (Invoke-Native ("gcc $cflags $redirect -Dmain=chars_selftest_main -I " + (Q $hdrs) + ' -c ' + (Q $selfC) + ' -o ' + (Q $selfDrivenObj)) (Join-Path $out 'selftest_driven.gcc.log')) (Join-Path $out 'selftest_driven.gcc.log')
$driverObj = Join-Path $out 'parity_driver.o'
# The driver is compiled with the same redirects, so its lmx_chars_* calls
# reach the unit and its header prototypes rename with them.
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
    if ($errText -notmatch 'warm-up ok') { throw "parity run $i did not open the unit through a real call: $errText" }
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
Write-Output "lmx_chars_owned parity PASS: $refChecks checks, 0 failures, $($ev.reference.allocations) counted allocations released, reference and generated agree on both runs, selftest redirected onto the unit's names; evidence $evPath"
