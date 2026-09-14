# Runtime-module parity runner: lmx_msg_storage (Fable lane, 2026-09-13).
#
# The question this answers is narrow and observable. Take the module's own
# existing selftest, which spells the exported names and the header defines,
# and run it twice: once against the handwritten L1 implementation, and once
# against an implementation generated from L2 source by the CURRENT translator
# in this checkout. If the two runs do not agree exactly -- stdout and exit --
# the port is not a port.
#
# The generated program is closed: its methods are l2_m{i} and its entry builds
# the unit graph. So the parity binary splices that entry, saving the two
# selected callables, and exports the module's real symbols as wrappers that
# call them. That is the existing mangled-entry/splice-drive mechanism, not a
# new one, and it is why this runner does not need a module emission mode.
#
# Nothing here replaces or deletes the L1 module. This slice establishes the
# verified source path; the clean bootstrap consumes it later.

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
$out = Join-Path $baseline "build/port_msg_storage/$stamp"
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
# 1. Support objects. The same set the graph gate links. lmx_msg_storage is
#    built too, but kept OUT of the parity link so the generated
#    implementation is the only definition of those two symbols there.
# ---------------------------------------------------------------------------
$names = @('lmx_msg_blocks', 'lmx_owned_ranges', 'lmx_msg_storage', 'lmx_msg_path_storage', 'lmx_msg_slots', 'lmx_msg_mail_chain', 'lmx_msg_visit', 'lmx_msg_liveness', 'lmx_msg_history_owned', 'lmx_msg_roots_stale', 'lmx_branch_owned', 'lmx_value_owned', 'lmx_chars_owned', 'lmx_array_owned', 'lmx_array_ref_owned', 'lmx_graph_copy_owned', 'lmx_merge_owned', 'lmx_message_graph_copy')
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
$nativeStorageObj = $null
foreach ($source in $sources) {
    $stem = [IO.Path]::GetFileNameWithoutExtension($source)
    $obj = Join-Path $support ($stem + '.o')
    $glog = Join-Path $out ($stem + '.gcc.log')
    Step "compile_$stem" (Invoke-Native ("gcc $cflags -I " + (Q $hdrs) + ' -c ' + (Q $source) + ' -o ' + (Q $obj)) $glog) $glog
    if ($stem -eq 'lmx_msg_storage') { $nativeStorageObj = $obj } else { $objs += $obj }
}
if (-not $nativeStorageObj) { throw 'the native storage object was not built' }
# Stage 3c-2a: the production runtime includes the L2 runtime units
# (l2src/l2units_build.ps1; today lmx_sched_record.lm2, profile: runtime).
. l2src/l2units_build.ps1
$objs += @(Build-L2RuntimeUnits -L1Trans $l1trans -Out (Join-Path $out 'l2units') -IncludeDirs @($hdrs) -CFlags $cflags)
$objList = ($objs | ForEach-Object { Q $_ }) -join ' '

# ---------------------------------------------------------------------------
# 2. Reference run: the existing selftest against the handwritten module.
# ---------------------------------------------------------------------------
$selfSrc = 'l2src/tests/lmx_msg_storage_selftest.lm1'
$selfC = Join-Path $out 'selftest.c'
Step 'selftest_translate' (Invoke-Native ((Q $l1trans) + ' ' + $selfSrc + ' ' + (Q $selfC)) (Join-Path $out 'selftest.translate.log')) (Join-Path $out 'selftest.translate.log')

$refExe = Join-Path $out 'reference.exe'
$refLog = Join-Path $out 'reference.stdout.txt'
Step 'reference_compile' (Invoke-Native ("gcc $cflags -I " + (Q $hdrs) + ' ' + (Q $selfC) + ' ' + (Q $nativeStorageObj) + ' ' + $objList + ' -o ' + (Q $refExe)) (Join-Path $out 'reference.gcc.log')) (Join-Path $out 'reference.gcc.log')
$refExit = Invoke-Native (Q $refExe) $refLog
$refOut = (Get-Content -LiteralPath $refLog -Raw)
if ($refExit -ne 0) { throw "the handwritten module's own selftest failed: exit $refExit`n$refOut" }
if ($refOut -notmatch 'storage checks=(\d+) failures=0') { throw "reference selftest did not report zero failures: $refOut" }
$refChecks = [int]$Matches[1]
# Pinned: the number of calls the selftest makes to the two module
# operations. A drop means a call site stopped being redirected.
$expectedPortCalls = 31
$ev.reference = [ordered]@{ exit = $refExit; stdout = $refOut.Trim(); checks = $refChecks }

# ---------------------------------------------------------------------------
# 3. The translator from THIS checkout, and the L2 source it compiles.
# ---------------------------------------------------------------------------
$l2c = Join-Path $out 'l2trans.c'
$l2exe = Join-Path $out 'l2trans.exe'
Step 'l2trans_translate' (Invoke-Native ((Q $l1trans) + ' l2src/l2trans.lm1 ' + (Q $l2c)) (Join-Path $out 'l2trans.translate.log')) (Join-Path $out 'l2trans.translate.log')
Step 'l2trans_compile' (Invoke-Native ("gcc $cflags -I lm1/build " + (Q $l2c) + ' l2src/lmx_poll_stub.c -o ' + (Q $l2exe)) (Join-Path $out 'l2trans.gcc.log')) (Join-Path $out 'l2trans.gcc.log')

$lm2 = 'l2src/lmx_msg_storage.lm2'
$ev.sourceSHA256 = (Get-FileHash -LiteralPath $lm2).Hash
# L2 has no define form, so the .lm2 spells 0 and 1 where the handwritten
# module spells the header names. The selftest pins the OK/INVALID contract
# behaviourally, but nothing would catch a header whose numbering changed
# under a source file that can no longer refer to it -- so the identity is
# checked here, against the headers themselves.
$defines = @(
    @{ header = 'l2src/lmx_msg_storage.h.lm1';  name = 'LMX_MSG_STORAGE_OK';    value = 0 }
    @{ header = 'l2src/lmx_msg_storage.h.lm1';  name = 'LMX_MSG_STORAGE_INVALID'; value = 1 }
    @{ header = 'l2src/lmx_msg_blocks.h.lm1';   name = 'LMX_MSG_BLOCKS_OK';     value = 0 }
    @{ header = 'l2src/lmx_msg_blocks.h.lm1';   name = 'LMX_MSG_BLOCKS_INVALID'; value = 1 }
    @{ header = 'l2src/lmx_owned_ranges.h.lm1'; name = 'LMX_OWNED_RANGES_OK';   value = 0 }
    @{ header = 'l2src/lmx_owned_ranges.h.lm1'; name = 'LMX_OWNED_RANGES_INVALID'; value = 1 }
)
foreach ($d in $defines) {
    $headerText = [IO.File]::ReadAllText((Resolve-Path -LiteralPath $d.header).ProviderPath)
    $m = [regex]::Match($headerText, '(?m)^define:\s+' + [regex]::Escape($d.name) + '\s+(-?\d+)\s*$')
    if (-not $m.Success) { throw "$($d.header) no longer defines $($d.name)" }
    if ([int]$m.Groups[1].Value -ne $d.value) { throw "$($d.name) is $($m.Groups[1].Value), but l2src/lmx_msg_storage.lm2 spells it $($d.value)" }
}
$gen = Join-Path $out 'generated.lm1'
Step 'l2trans_module' (Invoke-Native ((Q $l2exe) + ' ' + (Q $lm2) + ' ' + (Q $gen)) (Join-Path $out 'module.l2trans.log')) (Join-Path $out 'module.l2trans.log')
$genText = [IO.File]::ReadAllText((Resolve-Path -LiteralPath $gen).ProviderPath).Replace("`r`n", "`n")

# The lowering itself, before anything is run. Two methods, the four head-slot
# formals intact, the four foreign list operations called as themselves, and
# no allocation: this module allocates nothing, which is why it has no
# allocation-failure path of its own to exercise.
if ($genText -notmatch '(?m)^fn: l2_m0 \(@: Lmx node; @@: LmxMsgBlock l2_p0_0; @@: LmxOwnedRange l2_p0_1; @@: LmxMsgBlock l2_p0_2; @@: LmxOwnedRange l2_p0_3\) int') { throw 'the preflight signature did not survive translation' }
if ($genText -notmatch '(?m)^fn: l2_m1 \(@: Lmx node; @@: LmxMsgBlock l2_p1_0; @@: LmxOwnedRange l2_p1_1; @@: LmxMsgBlock l2_p1_2; @@: LmxOwnedRange l2_p1_3\) int') { throw 'the move signature did not survive translation' }
foreach ($fn in @('c.lmx_msg_blocks_can_move', 'c.lmx_owned_ranges_can_move', 'c.lmx_msg_blocks_move_all', 'c.lmx_owned_ranges_move_all')) {
    if ($genText -notmatch ([regex]::Escape($fn) + '\(l2_p\d+_\d+, l2_p\d+_\d+\)')) { throw "$fn is not called on two head slots" }
}
if ($genText -match 'c\.(malloc|calloc|realloc|free)\(') { throw 'the generated module allocates; the handwritten one does not' }
# move_all reaches its preflight as a METHOD call, not as a copy of it.
if ($genText -notmatch 'l2_t\d+: l2_m0\(lmx_branch_struct_known\(node\\node, \d+U\), l2_p1_0, l2_p1_1, l2_p1_2, l2_p1_3\)') { throw 'move_all does not call the preflight method through its lexical unit' }

# ---------------------------------------------------------------------------
# 4. Splice: keep the generated unit construction, export the real symbols as
#    wrappers over the selected callables, and let the existing selftest be
#    the body of the entry.
# ---------------------------------------------------------------------------
$exports = @'
prototype:
    fn: lmx_storage_selftest_main () int
end: prototype

# The two selected callables of the generated unit. A method's reserved
# argument is the SELECTED occurrence, never the enclosing container, so the
# wrappers below pass exactly what the entry stored here.
@: Lmx l2_port_can 0
@: Lmx l2_port_move 0
int: l2_port_calls 0

# The selftest's two call sites are redirected to these by -D at compile time,
# so every check it makes goes through the GENERATED implementation. The
# runtime's own graph construction keeps calling the handwritten module by its
# real name -- it has to, since building this unit is what publishes the
# callables these wrappers select.
fn: port_can_move (@@: LmxMsgBlock dst_blocks; @@: LmxOwnedRange dst_ranges; @@: LmxMsgBlock src_blocks; @@: LmxOwnedRange src_ranges) int
    if: l2_port_can = 0
        return: 1
    l2_port_calls: l2_port_calls + 1
    return: l2_m0(l2_port_can, dst_blocks, dst_ranges, src_blocks, src_ranges)

fn: port_move_all (@@: LmxMsgBlock dst_blocks; @@: LmxOwnedRange dst_ranges; @@: LmxMsgBlock src_blocks; @@: LmxOwnedRange src_ranges) int
    if: l2_port_move = 0
        return: 1
    l2_port_calls: l2_port_calls + 1
    return: l2_m1(l2_port_move, dst_blocks, dst_ranges, src_blocks, src_ranges)

external:
'@
$exports = $exports.Replace("`r`n", "`n")

$entryMark = "`nexternal:`n    # Synthetic entry adapter"
if ($genText.IndexOf($entryMark) -lt 0) { throw 'the generated entry adapter was not found' }
$driveText = $genText.Replace($entryMark, "`n" + $exports + "    # Synthetic entry adapter")

$tail = @'
        if: c.lmx_msg_poll_escape() != 0
            return: 0
        return: 0
    end: l2_program_body
'@
$tail = $tail.Replace("`r`n", "`n")
if ($driveText.IndexOf($tail) -lt 0) { throw 'the generated entry tail was not found' }
$entryLocals = "        @: Lmx unit 0`n"
if ($driveText.IndexOf($entryLocals) -lt 0) { throw 'the generated entry locals were not found' }
$driveText = $driveText.Replace($entryLocals, $entryLocals + "        int: process_result 0`n")
$drive = @'
        l2_port_can: lmx_branch_struct_known(unit, 0U)
        l2_port_move: lmx_branch_struct_known(unit, 1U)
        if: l2_port_can = 0 || l2_port_move = 0
            return: 1
        process_result: c.lmx_storage_selftest_main()
        c.fprintf(c.stderr, "port calls=%d\n", l2_port_calls)
        return: process_result
    end: l2_program_body
'@
$driveText = $driveText.Replace($tail, $drive.Replace("`r`n", "`n"))

$driveLm1 = Join-Path $out 'parity_drive.lm1'
[IO.File]::WriteAllText($driveLm1, $driveText)
$driveC = Join-Path $out 'parity_drive.c'
Step 'drive_translate' (Invoke-Native ((Q $l1trans) + ' ' + (Q $driveLm1) + ' ' + (Q $driveC)) (Join-Path $out 'drive.translate.log')) (Join-Path $out 'drive.translate.log')

# The selftest keeps its own main only as the driven body.
$parityExe = Join-Path $out 'parity.exe'
Step 'parity_compile' (Invoke-Native ("gcc $cflags -Dmain=lmx_storage_selftest_main -Dlmx_msg_storage_can_move=port_can_move -Dlmx_msg_storage_move_all=port_move_all -I " + (Q $hdrs) + ' -c ' + (Q $selfC) + ' -o ' + (Q (Join-Path $out 'selftest_driven.o'))) (Join-Path $out 'selftest_driven.gcc.log')) (Join-Path $out 'selftest_driven.gcc.log')
Step 'parity_link' (Invoke-Native ("gcc $cflags -I " + (Q $hdrs) + ' -I lm1/build ' + (Q $driveC) + ' ' + (Q (Join-Path $out 'selftest_driven.o')) + ' ' + (Q $nativeStorageObj) + ' ' + $objList + ' -o ' + (Q $parityExe)) (Join-Path $out 'parity.gcc.log')) (Join-Path $out 'parity.gcc.log')

# ---------------------------------------------------------------------------
# 5. Two runs, so a pass that depends on run order or on leftover state shows.
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
    # A silent fallback to the handwritten module would pass every check above
    # and prove nothing, so the count of calls that actually reached the
    # generated methods is part of the acceptance.
    if ($errText -notmatch 'port calls=(\d+)') { throw "parity run $i did not report how many calls reached the generated methods: $errText" }
    if ([int]$Matches[1] -ne $expectedPortCalls) { throw "$($Matches[1]) calls reached the generated methods, expected $expectedPortCalls" }
}
if ($runs[0].stdout -ne $runs[1].stdout -or $runs[0].stderr -ne $runs[1].stderr) { throw 'the two parity runs disagree' }
$ev.parity = $runs
$ev.generatedSHA256 = (Get-FileHash -LiteralPath $gen).Hash
$ev.driveSHA256 = (Get-FileHash -LiteralPath $driveLm1).Hash
$ev.l2transSHA256 = (Get-FileHash -LiteralPath $l2exe).Hash

$evPath = Join-Path $out 'evidence.json'
$ev | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath $evPath -Encoding utf8
Write-Output "lmx_msg_storage parity PASS: $refChecks checks, 0 failures, reference and generated agree on both runs; evidence $evPath"
