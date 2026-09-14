# Runtime-module parity runner: lmx_msg_path_storage (Fable lane, 2026-09-13).
#
# Same question as the storage and slots runners: run the module's own selftest
# against the handwritten L1 implementation and against one generated from L2
# source by the CURRENT translator, and require the two to agree exactly.
#
# This module has the BOOTSTRAP PROBLEM, which I measured before writing a line
# of the runner rather than discovering it as a crash: lmx_msg_assign_path
# calls lmx_msg_path_grow from inside lmx_msg_create, so the runtime grows a
# placement path while creating the very Message whose graph the generated
# callables live in. Exporting the real symbol would call the generated method
# before the entry had published it. So, as for storage, only the SELFTEST's
# call site is redirected (-D) and the runtime keeps the handwritten module.
#
# That redirection is also why the allocation counters need a reset: the
# runtime's own path_grow during Message creation goes through the selftest's
# __wrap_realloc and would otherwise shift every absolute count in the test.
# The drive reports what it reset, on stderr, so the number is visible rather
# than swallowed.
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
$out = Join-Path $baseline "build/port_msg_path_storage/$stamp"
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

$ev = [ordered]@{ stamp = $stamp; baseline = $baseline; translator = $l1trans }

# ---------------------------------------------------------------------------
# 0. The status literals and the chunk size ARE the header's defines. L2 has no
#    define form, so the identity is checked here, against the header itself,
#    before anything is built. The chunk size matters more than a status code
#    does: a status is observable through the selftest, a chunk size is not.
# ---------------------------------------------------------------------------
$defines = @(
    @{ name = 'LMX_MSG_PATH_OK'; value = 0 }
    @{ name = 'LMX_MSG_PATH_INVALID'; value = 2 }
    @{ name = 'LMX_MSG_PATH_NOMEM'; value = 3 }
    @{ name = 'LMX_MSG_PATH_STORAGE_CHUNK'; value = 4 }
)
$headerText = [IO.File]::ReadAllText((Resolve-Path -LiteralPath 'l2src/lmx_msg_path_storage.h.lm1').ProviderPath)
foreach ($d in $defines) {
    $m = [regex]::Match($headerText, '(?m)^define:\s+' + [regex]::Escape($d.name) + '\s+(-?\d+)\s*$')
    if (-not $m.Success) { throw "l2src/lmx_msg_path_storage.h.lm1 no longer defines $($d.name)" }
    if ([int]$m.Groups[1].Value -ne $d.value) { throw "$($d.name) is $($m.Groups[1].Value), but l2src/lmx_msg_path_storage.lm2 spells it $($d.value)" }
}

# ---------------------------------------------------------------------------
# 1. Support objects. The generated program builds a unit graph, so it needs
#    the whole Message runtime. lmx_msg_path_storage is built too and STAYS in
#    the link: the runtime calls it while creating the Message.
# ---------------------------------------------------------------------------
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

$objs = @()
$nativePathObj = $null
foreach ($source in $sources) {
    $stem = [IO.Path]::GetFileNameWithoutExtension($source)
    $obj = Join-Path $support ($stem + '.o')
    $glog = Join-Path $out ($stem + '.gcc.log')
    Step "compile_$stem" (Invoke-Native ("gcc $cflags -I " + (Q $hdrs) + ' -c ' + (Q $source) + ' -o ' + (Q $obj)) $glog) $glog
    if ($stem -eq 'lmx_msg_path_storage') { $nativePathObj = $obj }
    $objs += $obj
}
if (-not $nativePathObj) { throw 'the native path storage object was not built' }
# Stage 3c-2a: the production runtime includes the L2 runtime units
# (l2src/l2units_build.ps1; today lmx_sched_record.lm2, profile: runtime).
. l2src/l2units_build.ps1
$objs += @(Build-L2RuntimeUnits -L1Trans $l1trans -Out (Join-Path $out 'l2units') -IncludeDirs @($hdrs) -CFlags $cflags)
$objList = ($objs | ForEach-Object { Q $_ }) -join ' '

# ---------------------------------------------------------------------------
# 2. Reference run: the existing selftest against the handwritten module, with
#    the allocator wrapped exactly as its own runner does it.
# ---------------------------------------------------------------------------
$selfSrc = 'l2src/tests/lmx_msg_path_storage_selftest.lm1'
$selfC = Join-Path $out 'selftest.c'
Step 'selftest_translate' (Invoke-Native ((Q $l1trans) + ' ' + $selfSrc + ' ' + (Q $selfC)) (Join-Path $out 'selftest.translate.log')) (Join-Path $out 'selftest.translate.log')

$refExe = Join-Path $out 'reference.exe'
$refLog = Join-Path $out 'reference.stdout.txt'
Step 'reference_compile' (Invoke-Native ("gcc $cflags -I " + (Q $hdrs) + ' ' + (Q $selfC) + ' ' + (Q $nativePathObj) + ' -Wl,--wrap=realloc -o ' + (Q $refExe)) (Join-Path $out 'reference.gcc.log')) (Join-Path $out 'reference.gcc.log')
$refExit = Invoke-Native (Q $refExe) $refLog
$refOut = (Get-Content -LiteralPath $refLog -Raw)
if ($refExit -ne 0) { throw "the handwritten module's own selftest failed: exit $refExit`n$refOut" }
if ($refOut -notmatch 'path storage checks=(\d+) failures=0 allocations=(\d+) size_guard_checks=(\d+)') { throw "reference selftest did not report zero failures: $refOut" }
$refChecks = [int]$Matches[1]
$refAllocs = [int]$Matches[2]
$refGuards = [int]$Matches[3]
$ev.reference = [ordered]@{ exit = $refExit; stdout = $refOut.Trim(); checks = $refChecks; allocations = $refAllocs; sizeGuardChecks = $refGuards }
# The allocation-failure coverage is the point of this module's selftest: if
# the wrap is not in force, nothing ever fails and the whole NOMEM half of the
# test is unreachable.
if ($refAllocs -lt 11) { throw "only $refAllocs allocations reached the wrapped allocator: the failure injection is not in force" }

# ---------------------------------------------------------------------------
# 3. The translator from THIS checkout, and the L2 source it compiles.
# ---------------------------------------------------------------------------
$l2c = Join-Path $out 'l2trans.c'
$l2exe = Join-Path $out 'l2trans.exe'
Step 'l2trans_translate' (Invoke-Native ((Q $l1trans) + ' l2src/l2trans.lm1 ' + (Q $l2c)) (Join-Path $out 'l2trans.translate.log')) (Join-Path $out 'l2trans.translate.log')
Step 'l2trans_compile' (Invoke-Native ("gcc $cflags -I lm1/build " + (Q $l2c) + ' l2src/lmx_poll_stub.c -o ' + (Q $l2exe)) (Join-Path $out 'l2trans.gcc.log')) (Join-Path $out 'l2trans.gcc.log')

$lm2 = 'l2src/lmx_msg_path_storage.lm2'
$ev.sourceSHA256 = (Get-FileHash -LiteralPath $lm2).Hash
$gen = Join-Path $out 'generated.lm1'
Step 'l2trans_module' (Invoke-Native ((Q $l2exe) + ' ' + (Q $lm2) + ' ' + (Q $gen)) (Join-Path $out 'module.l2trans.log')) (Join-Path $out 'module.l2trans.log')
$genText = [IO.File]::ReadAllText((Resolve-Path -LiteralPath $gen).ProviderPath).Replace("`r`n", "`n")

# The lowering itself, before anything runs.
if ($genText -notmatch '(?m)^fn: l2_m0 \(@: Lmx node; @@: unsigned l2_p0_0; @: int l2_p0_1; int: l2_p0_2\) int') { throw 'the owner slot signature did not survive translation' }
$grow = [regex]::Match($genText, '(?ms)^fn: l2_m0 \(.*?end: l2_m0')
if (-not $grow.Success) { throw 'the grow method was not emitted' }
# Reads and writes go through the owner's slots, not through copies.
# The owner slot may be spelled either way -- `x[0]` or the strict prefix
# raw load `\x`. What is asserted is that the capacity is read THROUGH the
# slot, not from a copy, so both spellings pass and anything else fails.
if ($grow.Value -notmatch 'if: (l2_p0_1\[0\]|\\l2_p0_1) >= l2_p0_2') { throw 'the capacity is not read through the owner slot' }
if ($grow.Value -notmatch '(?m)^\s+(l2_p0_0\[0\]|\\l2_p0_0): grown\s*$') { throw 'the grown buffer is not published through the owner slot' }
if ($grow.Value -notmatch '(?m)^\s+(l2_p0_1\[0\]|\\l2_p0_1): l2_q\d+\s*$') { throw 'the new capacity is not published through the owner slot' }
# This buffer is NOT arena storage. Exactly one foreign allocation, and no
# graph or Message allocator anywhere near it.
if ([regex]::Matches($grow.Value, 'c\.realloc\(').Count -ne 1) { throw 'the grow method does not perform exactly one realloc' }
# Accessors of the callable's own fields are the ordinary method prologue;
# what must never appear is an arena or graph ALLOCATOR.
if ($grow.Value -match '_new_owned\(|_open_owned\(|lm_own_|c\.malloc\(|c\.calloc\(') { throw 'the private placement-path buffer reached a graph or arena allocator' }
# The overflow guard survives as written: a doubling refusal and a byte-size
# refusal, both before the allocator.
if ($grow.Value -notmatch 'if: l2_q\d+ > 2147483647 / 2') { throw 'the doubling overflow guard was rewritten' }
if ($grow.Value -notmatch '\(cast: \(size_t\) -1\)') { throw 'the size_t byte-size guard was rewritten' }
if ($grow.Value -notmatch 'c\.sizeof\(unsigned\)') { throw 'the element size is not sizeof(unsigned)' }
# Failure atomicity, read off the lowering: every refusal returns before either
# owner slot is written. The two publishing lines must be the last statements.
$pub = [regex]::Match($grow.Value, '(?ms)(l2_p0_0\[0\]|\\l2_p0_0): grown.*$')
if (-not $pub.Success) { throw 'the publication was not found' }
if ($pub.Value -match 'return: 3') { throw 'a NOMEM refusal is reachable after the buffer has been published' }

# ---------------------------------------------------------------------------
# 4. Splice: publish the selected callable, redirect the selftest's call site.
# ---------------------------------------------------------------------------
$exports = @'
prototype:
    fn: lmx_path_selftest_main () int
    fn: port_take_allocations () int
end: prototype

# The selected callable of the generated unit.
@: Lmx l2_port_grow 0
int: l2_port_calls 0
int: l2_port_early 0

# The selftest's one call site is redirected here by -D. The runtime keeps
# calling the handwritten module by its real name: lmx_msg_assign_path grows a
# placement path from inside lmx_msg_create, before this unit exists at all.
fn: port_path_grow (@@: unsigned path; @: int path_cap; int: need) int
    if: l2_port_grow = 0
        l2_port_early: l2_port_early + 1
        return: 2
    l2_port_calls: l2_port_calls + 1
    return: l2_m0(l2_port_grow, path, path_cap, need)

external:
'@
$exports = $exports.Replace("`r`n", "`n")

$entryMark = "`nexternal:`n    # Synthetic entry adapter"
if ($genText.IndexOf($entryMark) -lt 0) { throw 'the generated entry adapter was not found' }
$driveText = $genText.Replace($entryMark, "`n" + $exports + "    # Synthetic entry adapter")

$entryLocals = "        @: Lmx unit 0`n"
if ($driveText.IndexOf($entryLocals) -lt 0) { throw 'the generated entry locals were not found' }
$driveText = $driveText.Replace($entryLocals, $entryLocals + "        int: process_result 0`n        int: pre_allocations 0`n")

$tail = @'
        if: c.lmx_msg_poll_escape() != 0
            return: 0
        return: 0
    end: l2_program_entry
'@
$tail = $tail.Replace("`r`n", "`n")
if ($driveText.IndexOf($tail) -lt 0) { throw 'the generated entry tail was not found' }
$drive = @'
        l2_port_grow: lmx_branch_struct_known(unit, 0U)
        if: l2_port_grow = 0
            return: 1
        pre_allocations: c.port_take_allocations()
        process_result: c.lmx_path_selftest_main()
        c.fprintf(c.stderr, "port calls=%d early=%d pre_allocations=%d\n", l2_port_calls, l2_port_early, pre_allocations)
        return: process_result
    end: l2_program_entry
'@
$driveText = $driveText.Replace($tail, $drive.Replace("`r`n", "`n"))

$driveLm1 = Join-Path $out 'parity_drive.lm1'
[IO.File]::WriteAllText($driveLm1, $driveText)
$driveC = Join-Path $out 'parity_drive.c'
Step 'drive_translate' (Invoke-Native ((Q $l1trans) + ' ' + (Q $driveLm1) + ' ' + (Q $driveC)) (Join-Path $out 'drive.translate.log')) (Join-Path $out 'drive.translate.log')

# Instrumentation, in a generated build artifact only: the runtime's own
# path_grow during Message creation goes through the selftest's wrapped
# allocator, so the counters are taken and cleared before the test begins, and
# what was taken is reported rather than discarded.
$instrC = Join-Path $out 'port_instr.c'
@'
#include <stddef.h>
extern int allocations;
extern size_t requested_bytes;
int port_take_allocations(void);
int port_take_allocations(void)
{
    int n = allocations;
    allocations = 0;
    requested_bytes = 0U;
    return n;
}
'@ | Set-Content -LiteralPath $instrC -Encoding ascii

$parityExe = Join-Path $out 'parity.exe'
Step 'selftest_driven_compile' (Invoke-Native ("gcc $cflags -Dmain=lmx_path_selftest_main -Dlmx_msg_path_grow=port_path_grow -I " + (Q $hdrs) + ' -c ' + (Q $selfC) + ' -o ' + (Q (Join-Path $out 'selftest_driven.o'))) (Join-Path $out 'selftest_driven.gcc.log')) (Join-Path $out 'selftest_driven.gcc.log')
Step 'instr_compile' (Invoke-Native ("gcc $cflags -c " + (Q $instrC) + ' -o ' + (Q (Join-Path $out 'port_instr.o'))) (Join-Path $out 'port_instr.gcc.log')) (Join-Path $out 'port_instr.gcc.log')
Step 'parity_link' (Invoke-Native ("gcc $cflags -I " + (Q $hdrs) + ' -I lm1/build ' + (Q $driveC) + ' ' + (Q (Join-Path $out 'selftest_driven.o')) + ' ' + (Q (Join-Path $out 'port_instr.o')) + ' ' + $objList + ' -Wl,--wrap=realloc -o ' + (Q $parityExe)) (Join-Path $out 'parity.gcc.log')) (Join-Path $out 'parity.gcc.log')

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
    if ($exit -ne $refExit) { throw "parity run $i exit $exit, reference exit $refExit`n$text`n$errText" }
    if ($text.Trim() -ne $refOut.Trim()) { throw "parity run $i stdout differs from the reference`nreference: $($refOut.Trim())`nparity   : $($text.Trim())" }
    # A silent native fallback would pass everything above and prove nothing.
    if ($errText -notmatch 'port calls=(\d+) early=(\d+) pre_allocations=(\d+)') { throw "parity run $i did not report how many calls reached the generated method: $errText" }
    $calls = [int]$Matches[1]
    $early = [int]$Matches[2]
    $pre = [int]$Matches[3]
    # Pinned: the number of calls the selftest makes to the one operation.
    if ($calls -ne 19) { throw "$calls calls reached the generated method, expected 19" }
    # The redirect is what makes the early calls harmless; if one ever reached
    # the generated method it would have run before the unit existed.
    if ($early -ne 0) { throw "$early calls arrived before publication" }
    # The runtime grows exactly one placement path while creating the Message.
    if ($pre -ne 1) { throw "the runtime made $pre wrapped allocations before the test, expected 1" }
}
if ($runs[0].stdout -ne $runs[1].stdout -or $runs[0].stderr -ne $runs[1].stderr) { throw 'the two parity runs disagree' }
$ev.parity = $runs
$ev.generatedSHA256 = (Get-FileHash -LiteralPath $gen).Hash
$ev.driveSHA256 = (Get-FileHash -LiteralPath $driveLm1).Hash
$ev.l2transSHA256 = (Get-FileHash -LiteralPath $l2exe).Hash

$evPath = Join-Path $out 'evidence.json'
$ev | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath $evPath -Encoding utf8
Write-Output "lmx_msg_path_storage parity PASS: $refChecks checks, 0 failures, $refAllocs wrapped allocations, $refGuards size-guard checks, reference and generated agree on both runs; evidence $evPath"
