# Runtime-module parity runner: lmx_merge_owned (review-chat lane, 2026-09-14).
#
# Run the merge selftest (the graph gate's: operand order, body fields,
# cross-operand aliases through one map, eternal operands as terminals,
# container as the result's node, refusals, and a sweep that fails every
# counted allocation once) against the handwritten L1 implementation and
# against one generated from L2 source by the CURRENT translator, and require
# the two to agree exactly, twice.
#
# The generated half is a LIBRARY UNIT (7d7ec87c) exporting the two ABI names
# by real symbol: no runtime module calls merge and a unit's open path never
# merges, so nothing reaches the wrappers before they are published. The
# handwritten merge object is kept OUT of the parity link; the unmodified
# selftest resolves lmx_merge_owned and lmx_merge_add_width to the generated
# code or does not link at all.
#
# Allocation counting follows the graph gate: the graph-lane objects and the
# generated unit are compiled with -Dmalloc=lmx_test_malloc
# -Dfree=lmx_test_free. The unit's open path allocates through those same
# constructors once per process, on the first call: a C driver makes that
# first call BEFORE the selftest's main, with arguments merge refuses, then
# zeroes the counters, so the selftest starts from the reference's state.
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
$out = Join-Path $baseline "build/port_merge_owned/$stamp"
$hdrs = Join-Path $out 'hdrs'
$support = Join-Path $out 'support'
$instr = Join-Path $out 'instrumented'
New-Item -ItemType Directory -Force -Path $out, (Join-Path $hdrs 'l2src'), $support, $instr | Out-Null

$cflags = '-std=c99 -Wall -Wextra -Wpedantic -I . -Werror=incompatible-pointer-types -Werror=discarded-qualifiers -Werror=implicit-function-declaration -Werror=implicit-int'
$counted = '-Dmalloc=lmx_test_malloc -Dfree=lmx_test_free'

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
# 1. Headers and objects: the full runtime set plain, the graph-lane set a
#    second time with the counting allocator (the graph gate's split). The
#    handwritten merge exists in both forms and is used only by the reference.
# ---------------------------------------------------------------------------
$names = @('lmx_msg_blocks', 'lmx_owned_ranges', 'lmx_msg_storage', 'lmx_msg_mail_chain', 'lmx_msg_visit', 'lmx_msg_liveness', 'lmx_msg_history_owned', 'lmx_msg_roots_stale', 'lmx_branch_owned', 'lmx_value_owned', 'lmx_chars_owned', 'lmx_array_owned', 'lmx_array_ref_owned', 'lmx_graph_copy_owned', 'lmx_merge_owned', 'lmx_message_graph_copy')
$instrNames = @('lmx_graph_copy_owned', 'lmx_merge_owned', 'lmx_branch_owned', 'lmx_value_owned', 'lmx_chars_owned', 'lmx_array_owned', 'lmx_array_ref_owned', 'lmx_msg_blocks')
$plainNames = @('lmx_owned_ranges', 'lmx_msg_storage')

$moduleC = @{}
foreach ($name in $names) {
    Step "header_$name" (Invoke-Native ((Q $l1trans) + " l2src/$name.h.lm1 " + (Q (Join-Path $hdrs "l2src/$name.lm1.h"))) (Join-Path $out "header_$name.log")) (Join-Path $out "header_$name.log")
    $source = Join-Path $support "$name.c"
    Step "module_$name" (Invoke-Native ((Q $l1trans) + " l2src/$name.lm1 " + (Q $source)) (Join-Path $out "module_$name.log")) (Join-Path $out "module_$name.log")
    $moduleC[$name] = $source
}
$messageSource = Join-Path $support 'lmx_message.c'
Step 'module_lmx_message' (Invoke-Native ((Q $l1trans) + ' l2src/lmx_message.lm1 ' + (Q $messageSource)) (Join-Path $out 'module_lmx_message.log')) (Join-Path $out 'module_lmx_message.log')

$plainObjs = @()
foreach ($name in $names) {
    if ($instrNames -contains $name) { continue }
    $obj = Join-Path $support ($name + '.o')
    $glog = Join-Path $out ("plain_$name.gcc.log")
    Step "plain_$name" (Invoke-Native ("gcc $cflags -I " + (Q $hdrs) + ' -c ' + (Q $moduleC[$name]) + ' -o ' + (Q $obj)) $glog) $glog
    $plainObjs += $obj
}
foreach ($extra in @(@{ n = 'lmx_message'; s = $messageSource }, @{ n = 'lmx_message_host'; s = 'l2src/lmx_message_host.c' }, @{ n = 'lmx_message_exec'; s = 'l2src/lmx_message_exec.c' })) {
    $obj = Join-Path $support ($extra.n + '.o')
    $glog = Join-Path $out ('plain_' + $extra.n + '.gcc.log')
    Step ('plain_' + $extra.n) (Invoke-Native ("gcc $cflags -I " + (Q $hdrs) + ' -c ' + (Q $extra.s) + ' -o ' + (Q $obj)) $glog) $glog
    $plainObjs += $obj
}
# Stage 3c-2a: the production runtime includes the L2 runtime units
# (l2src/l2units_build.ps1; today lmx_sched_record.lm2, profile: runtime).
. l2src/l2units_build.ps1
$plainObjs += @(Build-L2RuntimeUnits -L1Trans $l1trans -Out (Join-Path $out 'l2units') -IncludeDirs @($hdrs) -CFlags $cflags)

$countedObjs = @()
$nativeCountedMerge = $null
foreach ($name in $instrNames) {
    $obj = Join-Path $instr ($name + '.o')
    $glog = Join-Path $out ("counted_$name.gcc.log")
    Step "counted_$name" (Invoke-Native ("gcc $cflags $counted -I " + (Q $hdrs) + ' -c ' + (Q $moduleC[$name]) + ' -o ' + (Q $obj)) $glog) $glog
    if ($name -eq 'lmx_merge_owned') { $nativeCountedMerge = $obj } else { $countedObjs += $obj }
}
if (-not $nativeCountedMerge) { throw 'the native counted merge object was not built' }
$gateObjs = @()
foreach ($name in $plainNames) { $gateObjs += (Join-Path $support ($name + '.o')) }

# ---------------------------------------------------------------------------
# 2. Reference run: the graph gate's merge selftest, linked the gate's way.
# ---------------------------------------------------------------------------
$selfSrc = 'l2src/tests/lmx_merge_selftest.lm1'
$selfC = Join-Path $out 'selftest.c'
Step 'selftest_translate' (Invoke-Native ((Q $l1trans) + ' ' + $selfSrc + ' ' + (Q $selfC)) (Join-Path $out 'selftest.translate.log')) (Join-Path $out 'selftest.translate.log')

$refExe = Join-Path $out 'reference.exe'
$refLog = Join-Path $out 'reference.stdout.txt'
$refLink = (@($nativeCountedMerge) + $countedObjs + $gateObjs | ForEach-Object { Q $_ }) -join ' '
Step 'reference_compile' (Invoke-Native ("gcc $cflags -I " + (Q $hdrs) + ' ' + (Q $selfC) + ' ' + $refLink + ' -o ' + (Q $refExe)) (Join-Path $out 'reference.gcc.log')) (Join-Path $out 'reference.gcc.log')
$refExit = Invoke-Native (Q $refExe) $refLog
$refOut = (Get-Content -LiteralPath $refLog -Raw)
if ($refExit -ne 0) { throw "the handwritten merge's own selftest failed: exit $refExit`n$refOut" }
if ($refOut -notmatch 'merge selftest: (\d+) checks, 0 failures') { throw "reference selftest did not report zero failures: $refOut" }
$refChecks = [int]$Matches[1]
if ($refOut -notmatch 'allocation failure positions exercised: (\d+)') { throw "reference selftest did not run the allocation-failure sweep: $refOut" }
$refPositions = [int]$Matches[1]
$ev.reference = [ordered]@{ exit = $refExit; stdout = $refOut.Trim(); checks = $refChecks; failurePositions = $refPositions }

# ---------------------------------------------------------------------------
# 3. The translator from THIS checkout, and the library unit it emits.
# ---------------------------------------------------------------------------
$l2c = Join-Path $out 'l2trans.c'
$l2exe = Join-Path $out 'l2trans.exe'
Step 'l2trans_translate' (Invoke-Native ((Q $l1trans) + ' l2src/l2trans.lm1 ' + (Q $l2c)) (Join-Path $out 'l2trans.translate.log')) (Join-Path $out 'l2trans.translate.log')
Step 'l2trans_compile' (Invoke-Native ("gcc $cflags -I lm1/build " + (Q $l2c) + ' l2src/lmx_poll_stub.c -o ' + (Q $l2exe)) (Join-Path $out 'l2trans.gcc.log')) (Join-Path $out 'l2trans.gcc.log')

$lm2 = 'l2src/lmx_merge_owned.lm2'
$ev.sourceSHA256 = (Get-FileHash -LiteralPath $lm2).Hash
$gen = Join-Path $out 'generated.lm1'
Step 'l2trans_module' (Invoke-Native ((Q $l2exe) + ' ' + (Q $lm2) + ' ' + (Q $gen)) (Join-Path $out 'module.l2trans.log')) (Join-Path $out 'module.l2trans.log')
$genText = [IO.File]::ReadAllText((Resolve-Path -LiteralPath $gen).ProviderPath).Replace("`r`n", "`n")

if ($genText -notmatch 'define: l2_program_entry l2_u[0-9A-F]{16}_entry') { throw 'the generated unit is not a library unit' }
foreach ($sig in @(
    'fn: lmx_merge_add_width \(@: size_t total; @: Lmx x\) int',
    'fn: lmx_merge_owned \(@@: Lmx operands; size_t: count; @: Lmx body; @: Lmx container; @: LmxOwnedRange src_ranges; @: LmxOwnedRange eternal_ranges; @: LmxOwnedRange method_ranges; @: char dst_chars; @@: LmxMsgBlock dst_blocks; @@: LmxOwnedRange dst_ranges; @@: Lmx out\) int')) {
    if ($genText -notmatch $sig) { throw "the public signature is missing or changed: $sig" }
}
# One copy operation over all ordinary roots; the two root lists allocated
# with malloc (never calloc, so the counted pair stays a pair), zeroed
# explicitly, released on every path; one storage-move admission; one
# disposal on failure; the container stored as the result's node.
if (([regex]::Matches($genText, 'lmx_graph_copy_many_owned\(')).Count -ne 1) { throw 'merge does not copy through exactly one operation' }
if (([regex]::Matches($genText, 'c\.malloc\(')).Count -ne 2) { throw 'merge does not allocate exactly its two root lists' }
if (([regex]::Matches($genText, 'c\.free\(')).Count -ne 4) { throw 'merge does not release its root lists on every path (four sites)' }
if (([regex]::Matches($genText, 'c\.memset\(')).Count -ne 2) { throw 'merge does not zero both root lists explicitly' }
if ($genText -match 'c\.(calloc|realloc)\(') { throw 'merge uses an allocator the handwritten module does not' }
if (([regex]::Matches($genText, 'lmx_msg_storage_move_all\(')).Count -ne 1) { throw 'merge does not admit through exactly one storage move' }
if (([regex]::Matches($genText, 'lmx_msg_blocks_dispose_all\(')).Count -ne 1) { throw 'merge does not dispose the prepared storage on failure exactly once' }
if ($genText -notmatch '(?m)^\s*[a-z_0-9]+\\node: ') { throw 'merge does not set the result node' }
if ($genText -notmatch 'c\.INT_MAX') { throw 'the width guard against INT_MAX is missing' }

# ---------------------------------------------------------------------------
# 4. The parity binary: the UNMODIFIED selftest, its main renamed, driven by a
#    C main that opens the unit first and zeroes the counters; the generated
#    unit compiled with the counting allocator; the handwritten merge absent.
# ---------------------------------------------------------------------------
$genC = Join-Path $out 'generated.c'
Step 'generated_translate' (Invoke-Native ((Q $l1trans) + ' ' + (Q $gen) + ' ' + (Q $genC)) (Join-Path $out 'generated.translate.log')) (Join-Path $out 'generated.translate.log')
$genObj = Join-Path $out 'generated.o'
Step 'generated_compile' (Invoke-Native ("gcc $cflags $counted -I " + (Q $hdrs) + ' -I lm1/build -c ' + (Q $genC) + ' -o ' + (Q $genObj)) (Join-Path $out 'generated.gcc.log')) (Join-Path $out 'generated.gcc.log')

$nmLog = Join-Path $out 'generated.nm.txt'
Step 'generated_nm' (Invoke-Native ('nm --defined-only ' + (Q $genObj)) $nmLog) $nmLog
$nmText = [IO.File]::ReadAllText($nmLog)
foreach ($sym in @('lmx_merge_owned', 'lmx_merge_add_width')) {
    if ($nmText -notmatch ('(?m)^\S+\s+T\s+_?' + $sym + '\s*$')) { throw "the generated object does not define $sym" }
}

$driverC = Join-Path $out 'parity_driver.c'
@'
#include <stdio.h>
#include "l2src/lmx_merge_owned.lm1.h"
extern int alloc_calls;
extern int alloc_live;
extern int alloc_fail_at;
extern int alloc_failed;
int merge_selftest_main(void);
int main(void) {
    Lmx *out = (Lmx *)1;
    LmxMsgBlock *blocks = 0;
    LmxOwnedRange *ranges = 0;
    int warm;
    /* First call: the library unit opens and builds its graph through the
       counted constructors; merge itself refuses a null operand list before
       touching out. */
    warm = lmx_merge_owned(0, 0, 0, 0, 0, 0, 0, 0, &blocks, &ranges, &out);
    fprintf(stderr, "warm-up status=%d allocations=%d live=%d\n", warm, alloc_calls, alloc_live);
    if (warm != LMX_MERGE_INVALID || out != (Lmx *)1 || blocks != 0 || ranges != 0) {
        return 90;
    }
    alloc_calls = 0;
    alloc_live = 0;
    alloc_fail_at = 0;
    alloc_failed = 0;
    return merge_selftest_main();
}
'@.Replace("`r`n", "`n") | Set-Content -LiteralPath $driverC -Encoding ascii -NoNewline

$selfDrivenObj = Join-Path $out 'selftest_driven.o'
Step 'selftest_driven_compile' (Invoke-Native ("gcc $cflags -Dmain=merge_selftest_main -I " + (Q $hdrs) + ' -c ' + (Q $selfC) + ' -o ' + (Q $selfDrivenObj)) (Join-Path $out 'selftest_driven.gcc.log')) (Join-Path $out 'selftest_driven.gcc.log')
$driverObj = Join-Path $out 'parity_driver.o'
Step 'driver_compile' (Invoke-Native ("gcc $cflags -I " + (Q $hdrs) + ' -c ' + (Q $driverC) + ' -o ' + (Q $driverObj)) (Join-Path $out 'driver.gcc.log')) (Join-Path $out 'driver.gcc.log')

$parityExe = Join-Path $out 'parity.exe'
$parityLink = (@($driverObj, $selfDrivenObj, $genObj) + $countedObjs + $plainObjs | ForEach-Object { Q $_ }) -join ' '
Step 'parity_link' (Invoke-Native ("gcc $cflags $parityLink -o " + (Q $parityExe)) (Join-Path $out 'parity.gcc.log')) (Join-Path $out 'parity.gcc.log')

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
    if ($errText -notmatch 'warm-up status=(-?\d+) allocations=(\d+) live=(\d+)') { throw "parity run $i did not report the warm-up: $errText" }
    if ([int]$Matches[2] -lt 1) { throw "parity run ${i}: the warm-up allocated nothing, so the unit did not open through the counted constructors" }
    if ($exit -ne $refExit) { throw "parity run $i exit $exit, reference exit $refExit`n$text`n$errText" }
    if ($text.Trim() -ne $refOut.Trim()) { throw "parity run $i stdout differs from the reference`nreference: $($refOut.Trim())`nparity   : $($text.Trim())" }
}
if ($runs[0].stdout -ne $runs[1].stdout -or $runs[0].stderr -ne $runs[1].stderr) { throw 'the two parity runs disagree' }
$ev.parity = $runs
$ev.generatedSHA256 = (Get-FileHash -LiteralPath $gen).Hash
$ev.l2transSHA256 = (Get-FileHash -LiteralPath $l2exe).Hash

$evPath = Join-Path $out 'evidence.json'
$ev | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath $evPath -Encoding utf8
Write-Output "lmx_merge_owned parity PASS: $refChecks checks, 0 failures, $refPositions allocation-failure positions, reference and generated agree on both runs, real symbols from the generated unit; evidence $evPath"
