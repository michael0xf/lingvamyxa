# Runtime-module parity runner: lmx_msg_slots (Fable lane, 2026-09-13).
#
# Same question as run_port_msg_storage.ps1: run the module's own selftest
# against the handwritten L1 implementation and against one generated from L2
# source by the CURRENT translator, and require the two to agree exactly.
#
# One difference from the storage runner, and it is the interesting one.
# Nothing in the runtime calls these two operations while a graph is being
# built -- the only non-test callers are two forwarders in lmx_message_exec.c
# -- so here the wrappers can EXPORT THE REAL SYMBOLS. The storage module
# could not: building the unit graph is itself a storage move, so its wrappers
# would have been called before the entry published the callables they select.
#
# That makes the proof stronger here: the selftest is unmodified and links
# against lmx_msg_slots_n and lmx_msg_slots_at by name, exactly as any other
# caller would. The wrappers still count their calls, and separately count any
# call that arrives BEFORE publication, so neither a silent native fallback nor
# a hidden early call can pass unnoticed.
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
$out = Join-Path $baseline "build/port_msg_slots/$stamp"
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
# 1. Support objects. lmx_msg_slots is built too, but kept OUT of the parity
#    link: there the generated implementation is the only definition.
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
$nativeSlotsObj = $null
foreach ($source in $sources) {
    $stem = [IO.Path]::GetFileNameWithoutExtension($source)
    $obj = Join-Path $support ($stem + '.o')
    $glog = Join-Path $out ($stem + '.gcc.log')
    Step "compile_$stem" (Invoke-Native ("gcc $cflags -I " + (Q $hdrs) + ' -c ' + (Q $source) + ' -o ' + (Q $obj)) $glog) $glog
    if ($stem -eq 'lmx_msg_slots') { $nativeSlotsObj = $obj } else { $objs += $obj }
}
if (-not $nativeSlotsObj) { throw 'the native slots object was not built' }
# Stage 3c-2a: the production runtime includes the L2 runtime units
# (l2src/l2units_build.ps1; today lmx_sched_record.lm2, profile: runtime).
. l2src/l2units_build.ps1
$objs += @(Build-L2RuntimeUnits -L1Trans $l1trans -Out (Join-Path $out 'l2units') -IncludeDirs @($hdrs) -CFlags $cflags)
$objList = ($objs | ForEach-Object { Q $_ }) -join ' '

# ---------------------------------------------------------------------------
# 2. Reference run: the existing selftest against the handwritten module.
# ---------------------------------------------------------------------------
$selfSrc = 'l2src/tests/lmx_msg_slots_selftest.lm1'
$selfC = Join-Path $out 'selftest.c'
Step 'selftest_translate' (Invoke-Native ((Q $l1trans) + ' ' + $selfSrc + ' ' + (Q $selfC)) (Join-Path $out 'selftest.translate.log')) (Join-Path $out 'selftest.translate.log')

$refExe = Join-Path $out 'reference.exe'
$refLog = Join-Path $out 'reference.stdout.txt'
Step 'reference_compile' (Invoke-Native ("gcc $cflags -I " + (Q $hdrs) + ' ' + (Q $selfC) + ' ' + (Q $nativeSlotsObj) + ' ' + $objList + ' -o ' + (Q $refExe)) (Join-Path $out 'reference.gcc.log')) (Join-Path $out 'reference.gcc.log')
$refExit = Invoke-Native (Q $refExe) $refLog
$refOut = (Get-Content -LiteralPath $refLog -Raw)
if ($refExit -ne 0) { throw "the handwritten module's own selftest failed: exit $refExit`n$refOut" }
if ($refOut -notmatch 'slots checks=(\d+) failures=0') { throw "reference selftest did not report zero failures: $refOut" }
$refChecks = [int]$Matches[1]
$ev.reference = [ordered]@{ exit = $refExit; stdout = $refOut.Trim(); checks = $refChecks }

# ---------------------------------------------------------------------------
# 3. The translator from THIS checkout, and the L2 source it compiles.
# ---------------------------------------------------------------------------
$l2c = Join-Path $out 'l2trans.c'
$l2exe = Join-Path $out 'l2trans.exe'
Step 'l2trans_translate' (Invoke-Native ((Q $l1trans) + ' l2src/l2trans.lm1 ' + (Q $l2c)) (Join-Path $out 'l2trans.translate.log')) (Join-Path $out 'l2trans.translate.log')
Step 'l2trans_compile' (Invoke-Native ("gcc $cflags -I lm1/build " + (Q $l2c) + ' l2src/lmx_poll_stub.c -o ' + (Q $l2exe)) (Join-Path $out 'l2trans.gcc.log')) (Join-Path $out 'l2trans.gcc.log')

$lm2 = 'l2src/lmx_msg_slots.lm2'
$ev.sourceSHA256 = (Get-FileHash -LiteralPath $lm2).Hash
$gen = Join-Path $out 'generated.lm1'
Step 'l2trans_module' (Invoke-Native ((Q $l2exe) + ' ' + (Q $lm2) + ' ' + (Q $gen)) (Join-Path $out 'module.l2trans.log')) (Join-Path $out 'module.l2trans.log')
$genText = [IO.File]::ReadAllText((Resolve-Path -LiteralPath $gen).ProviderPath).Replace("`r`n", "`n")

# The lowering itself, before anything runs.
if ($genText -notmatch '(?m)^fn: l2_m0 \(@: Lmx node; const: @\(LmxMsgRuntime l2_p0_0\)\) int') { throw 'the count signature did not survive translation' }
if ($genText -notmatch '(?m)^fn: l2_m1 \(@: Lmx node; const: @\(LmxMsgRuntime l2_p1_0\); int: l2_p1_1\) LmxMsgAddr') { throw 'the enumeration signature did not survive translation' }
# The count is the owner's cached field, never a traversal.
if ($genText -notmatch 'return: l2_p0_0\\n') { throw 'the count does not return the cached field' }
$m0 = [regex]::Match($genText, '(?ms)^fn: l2_m0 \(.*?end: l2_m0')
if (-not $m0.Success) { throw 'the count method was not emitted' }
if ($m0.Value -match 'while:' -or $m0.Value -match 'alloc_next') { throw 'the count traverses the list' }
# The enumeration follows alloc_next and nothing else. root, first_child and
# next_sibling may point anywhere and must not drive it.
$m1 = [regex]::Match($genText, '(?ms)^fn: l2_m1 \(.*?end: l2_m1')
if (-not $m1.Success) { throw 'the enumeration method was not emitted' }
if ($m1.Value -notmatch 'm: l2_p1_0\\slots') { throw 'the enumeration does not start at the owner slot head' }
if ($m1.Value -notmatch 'm: m\\alloc_next') { throw 'the enumeration does not advance along alloc_next' }
if ($m1.Value -match '\\root|\\first_child|\\next_sibling') { throw 'the enumeration reads a graph link' }
if ($m1.Value -notmatch 'return: m\\addr') { throw 'the enumeration does not return the record address' }
# The cursor is a const pointee: a read-only walk, as the handwritten module
# declares it.
if ($m1.Value -notmatch '(?m)^    const: @\(LmxMsg m\)\s*$') { throw 'the cursor is not a const LmxMsg local' }
# Neither operation allocates, so there is no allocation-failure path of its
# own to exercise.
if ($genText -match 'c\.(malloc|calloc|realloc)\(') { throw 'the generated module allocates; the handwritten one does not' }

# ---------------------------------------------------------------------------
# 4. Splice: publish the two selected callables, then export the real symbols.
# ---------------------------------------------------------------------------
$exports = @'
prototype:
    fn: lmx_slots_selftest_main () int
end: prototype

# The two selected callables of the generated unit. A method's reserved
# argument is the SELECTED occurrence, never the enclosing container.
@: Lmx l2_port_n 0
@: Lmx l2_port_at 0
int: l2_port_calls 0
int: l2_port_early 0

# These ARE lmx_msg_slots_n and lmx_msg_slots_at for everything linked here.
# Nothing calls them while the graph is built, so they can be the real symbols;
# l2_port_early counts any call that proves otherwise.
fn: lmx_msg_slots_n (const: @(LmxMsgRuntime rt)) int
    if: l2_port_n = 0
        l2_port_early: l2_port_early + 1
        return: 0
    l2_port_calls: l2_port_calls + 1
    return: l2_m0(l2_port_n, rt)

fn: lmx_msg_slots_at (const: @(LmxMsgRuntime rt); int: i) LmxMsgAddr
    if: l2_port_at = 0
        l2_port_early: l2_port_early + 1
        return: 0U
    l2_port_calls: l2_port_calls + 1
    return: l2_m1(l2_port_at, rt, i)

external:
'@
$exports = $exports.Replace("`r`n", "`n")

$entryMark = "`nexternal:`n    # Synthetic entry adapter"
if ($genText.IndexOf($entryMark) -lt 0) { throw 'the generated entry adapter was not found' }
$driveText = $genText.Replace($entryMark, "`n" + $exports + "    # Synthetic entry adapter")

$entryLocals = "        @: Lmx unit 0`n"
if ($driveText.IndexOf($entryLocals) -lt 0) { throw 'the generated entry locals were not found' }
$driveText = $driveText.Replace($entryLocals, $entryLocals + "        int: process_result 0`n")

$tail = @'
        if: c.lmx_msg_poll_escape() != 0
            return: 0
        return: 0
    end: l2_program_entry
'@
$tail = $tail.Replace("`r`n", "`n")
if ($driveText.IndexOf($tail) -lt 0) { throw 'the generated entry tail was not found' }
$drive = @'
        l2_port_n: lmx_branch_struct_known(unit, 0U)
        l2_port_at: lmx_branch_struct_known(unit, 1U)
        if: l2_port_n = 0 || l2_port_at = 0
            return: 1
        process_result: c.lmx_slots_selftest_main()
        c.fprintf(c.stderr, "port calls=%d early=%d\n", l2_port_calls, l2_port_early)
        return: process_result
    end: l2_program_entry
'@
$driveText = $driveText.Replace($tail, $drive.Replace("`r`n", "`n"))

$driveLm1 = Join-Path $out 'parity_drive.lm1'
[IO.File]::WriteAllText($driveLm1, $driveText)
$driveC = Join-Path $out 'parity_drive.c'
Step 'drive_translate' (Invoke-Native ((Q $l1trans) + ' ' + (Q $driveLm1) + ' ' + (Q $driveC)) (Join-Path $out 'drive.translate.log')) (Join-Path $out 'drive.translate.log')

# The selftest is UNMODIFIED apart from its main becoming the driven body: it
# links against the real symbol names, exactly as any other caller would.
$parityExe = Join-Path $out 'parity.exe'
Step 'selftest_driven_compile' (Invoke-Native ("gcc $cflags -Dmain=lmx_slots_selftest_main -I " + (Q $hdrs) + ' -c ' + (Q $selfC) + ' -o ' + (Q (Join-Path $out 'selftest_driven.o'))) (Join-Path $out 'selftest_driven.gcc.log')) (Join-Path $out 'selftest_driven.gcc.log')
Step 'parity_link' (Invoke-Native ("gcc $cflags -I " + (Q $hdrs) + ' -I lm1/build ' + (Q $driveC) + ' ' + (Q (Join-Path $out 'selftest_driven.o')) + ' ' + $objList + ' -o ' + (Q $parityExe)) (Join-Path $out 'parity.gcc.log')) (Join-Path $out 'parity.gcc.log')

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
    if ($errText -notmatch 'port calls=(\d+) early=(\d+)') { throw "parity run $i did not report how many calls reached the generated methods: $errText" }
    $calls = [int]$Matches[1]
    $early = [int]$Matches[2]
    # Pinned: the number of calls the selftest makes to the two operations.
    if ($calls -ne 279) { throw "$calls calls reached the generated methods, expected 279" }
    # and nothing called them before the entry published the callables.
    if ($early -ne 0) { throw "$early calls arrived before publication: this module has the bootstrap problem after all" }
}
if ($runs[0].stdout -ne $runs[1].stdout -or $runs[0].stderr -ne $runs[1].stderr) { throw 'the two parity runs disagree' }
$ev.parity = $runs
$ev.generatedSHA256 = (Get-FileHash -LiteralPath $gen).Hash
$ev.driveSHA256 = (Get-FileHash -LiteralPath $driveLm1).Hash
$ev.l2transSHA256 = (Get-FileHash -LiteralPath $l2exe).Hash

$evPath = Join-Path $out 'evidence.json'
$ev | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath $evPath -Encoding utf8
Write-Output "lmx_msg_slots parity PASS: $refChecks checks, 0 failures, reference and generated agree on both runs; evidence $evPath"
