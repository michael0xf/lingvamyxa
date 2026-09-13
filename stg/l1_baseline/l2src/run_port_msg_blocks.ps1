# Runtime-module parity runner: lmx_msg_blocks (Fable lane, 2026-09-13).
#
# Same question as the runners before it: run the module's own selftest against
# the handwritten L1 implementation and against one generated from L2 source by
# the CURRENT translator, and require the two to agree exactly.
#
# WHAT IS DIFFERENT TODAY: the generated half cannot be built yet. The L2 source
# is complete, but the translator is missing nine constructs this module needs
# and Codex owns that compiler slice (ticket 20260913-095000: do not edit
# l2trans.lm1 for this ticket). So the runner has two verdicts, PENDING and
# PASS, and the pending rejection is PINNED so it fails in both directions: if
# the oracle breaks, and if the rejection moves or disappears without the parity
# half being enabled.
#
# WHEN THE PARITY HALF IS ENABLED, two things are already measured and must be
# built in:
#   - this module has the BOOTSTRAP PROBLEM. Every arena allocation calls
#     lmx_msg_storage_move_all (lmx_branch_owned.lm1:61 and the same line in
#     lmx_value_owned, lmx_chars_owned and both array modules), which calls
#     lmx_msg_blocks_can_move and lmx_msg_blocks_move_all. So the generated
#     methods would be called while the unit graph that holds them is still
#     being built. Redirect the SELFTEST's call sites with -D, as
#     run_port_msg_storage.ps1 and run_port_msg_path_storage.ps1 do; do not
#     export the real symbols.
#   - the selftest wraps free (--wrap=free) and counts both the default
#     disposal and the custom disposer, so the drive must take and REPORT the
#     counters the runtime's own graph construction spends, the way
#     run_port_msg_path_storage.ps1 does, rather than silently clearing them.
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
# 0. The status literals ARE the header's defines.
# ---------------------------------------------------------------------------
$defines = @(
    @{ name = 'LMX_MSG_BLOCKS_OK'; value = 0 }
    @{ name = 'LMX_MSG_BLOCKS_INVALID'; value = 1 }
)
$headerText = [IO.File]::ReadAllText((Resolve-Path -LiteralPath 'l2src/lmx_msg_blocks.h.lm1').ProviderPath)
foreach ($d in $defines) {
    $m = [regex]::Match($headerText, '(?m)^define:\s+' + [regex]::Escape($d.name) + '\s+(-?\d+)\s*$')
    if (-not $m.Success) { throw "l2src/lmx_msg_blocks.h.lm1 no longer defines $($d.name)" }
    if ([int]$m.Groups[1].Value -ne $d.value) { throw "$($d.name) is $($m.Groups[1].Value), but l2src/lmx_msg_blocks.lm2 spells it $($d.value)" }
}

# ---------------------------------------------------------------------------
# 1. The ORACLE: the handwritten module and its own selftest, with the
#    allocator wrapped exactly as the module's own runner does it. The failure
#    and disposer coverage is the point of this selftest, so the counts are
#    checked, not just the verdict.
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
# If the wrap is not in force nothing is counted, and the disposal half of the
# test proves nothing.
if ($refFrees -lt 1 -or $refCallbacks -lt 1) { throw "frees=$refFrees callbacks=${refCallbacks}: the free wrap is not in force, so disposal is unverified" }
$ev.reference = [ordered]@{ exit = $refExit; stdout = $refOut.Trim(); checks = $refChecks; frees = $refFrees; callbacks = $refCallbacks }

# ---------------------------------------------------------------------------
# 2. The translator from THIS checkout, and the L2 source it is given.
# ---------------------------------------------------------------------------
$l2c = Join-Path $out 'l2trans.c'
$l2exe = Join-Path $out 'l2trans.exe'
Step 'l2trans_translate' (Invoke-Native ((Q $l1trans) + ' l2src/l2trans.lm1 ' + (Q $l2c)) (Join-Path $out 'l2trans.translate.log')) (Join-Path $out 'l2trans.translate.log')
Step 'l2trans_compile' (Invoke-Native ("gcc $cflags -I lm1/build " + (Q $l2c) + ' l2src/lmx_poll_stub.c -o ' + (Q $l2exe)) (Join-Path $out 'l2trans.gcc.log')) (Join-Path $out 'l2trans.gcc.log')

$lm2 = 'l2src/lmx_msg_blocks.lm2'
$ev.sourceSHA256 = (Get-FileHash -LiteralPath $lm2).Hash
$gen = Join-Path $out 'generated.lm1'
$genLog = Join-Path $out 'module.l2trans.log'
$genExit = Invoke-Native ((Q $l2exe) + ' ' + (Q $lm2) + ' ' + (Q $gen)) $genLog
$genMsg = (Get-Content -LiteralPath $genLog -Raw)
if ($null -eq $genMsg) { $genMsg = '' }

# The construct the translator stops at TODAY, pinned. The other eight are
# listed in 20260913-094000 and are reached only once this one is lifted, so
# this pin moves forward as the compiler slice lands rather than all at once.
$pendingLine = 18
$pendingText = 'unknown foreign type'
$pendingWhat = 'a single-pointer formal of a foreign struct: @: LmxMsgBlock head'

if ($genExit -ne 0) {
    $m = [regex]::Match($genMsg, 'lmx_msg_blocks\.lm2:(\d+):(\d+): (.+?)\s*$', 'Multiline')
    if (-not $m.Success) { throw "l2trans rejected the source in a way this runner cannot read:`n$genMsg" }
    $line = [int]$m.Groups[1].Value
    $text = $m.Groups[3].Value.Trim()
    if ($line -ne $pendingLine -or $text -ne $pendingText) {
        throw "the pending rejection moved: expected ${pendingText} at line $pendingLine ($pendingWhat), got '$text' at line $line. Update the pin, or the remaining work list, whichever is now true."
    }
    $ev.pending = [ordered]@{ line = $line; message = $text; construct = $pendingWhat; translatorOutput = $genMsg.Trim() }
    $evPath = Join-Path $out 'evidence.json'
    $ev | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath $evPath -Encoding utf8
    Write-Output "lmx_msg_blocks parity PENDING: oracle PASS ($refChecks checks, 0 failures, $refFrees frees, $refCallbacks disposer callbacks); l2trans stops at ${lm2}:${pendingLine} '$pendingText' -- $pendingWhat. Evidence $evPath"
    exit 0
}

throw "l2trans now ACCEPTS $lm2. The pending pin at line $pendingLine is stale: enable the parity half of this runner (redirect the selftest call sites, take and report the wrapped-free counters, two runs, compare with the oracle) and delete the pin."
