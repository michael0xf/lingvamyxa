# Runtime-module parity runner: lmx_owned_ranges (Fable lane, 2026-09-13).
#
# Same question as the three runners before it: run the module's own selftest
# against the handwritten L1 implementation and against one generated from L2
# source by the CURRENT translator, and require the two to agree exactly.
#
# WHAT IS DIFFERENT TODAY: the generated half cannot be built yet. The L2 source
# is complete and correct, but the translator is missing constructs this module
# needs, and Codex owns that compiler slice (ticket 20260913-095000: do not edit
# l2trans.lm1 for this ticket). So the runner has two verdicts:
#
#   PENDING -- the L1 oracle passes, and l2trans rejects the .lm2 at EXACTLY the
#              construct recorded below. That is the honest state while the
#              compiler slice is someone else's work in progress.
#   PASS    -- l2trans accepts the .lm2 and the generated implementation agrees
#              with the oracle on both runs.
#
# The pending rejection is PINNED, so this runner fails in both directions: it
# fails if the oracle breaks, and it fails if the rejection moves to a different
# construct or disappears without the parity half being enabled. It cannot
# quietly keep reporting PENDING once the support lands.
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
$out = Join-Path $baseline "build/port_owned_ranges/$stamp"
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
    @{ name = 'LMX_OWNED_RANGES_OK'; value = 0 }
    @{ name = 'LMX_OWNED_RANGES_INVALID'; value = 1 }
)
$headerText = [IO.File]::ReadAllText((Resolve-Path -LiteralPath 'l2src/lmx_owned_ranges.h.lm1').ProviderPath)
foreach ($d in $defines) {
    $m = [regex]::Match($headerText, '(?m)^define:\s+' + [regex]::Escape($d.name) + '\s+(-?\d+)\s*$')
    if (-not $m.Success) { throw "l2src/lmx_owned_ranges.h.lm1 no longer defines $($d.name)" }
    if ([int]$m.Groups[1].Value -ne $d.value) { throw "$($d.name) is $($m.Groups[1].Value), but l2src/lmx_owned_ranges.lm2 spells it $($d.value)" }
}

# ---------------------------------------------------------------------------
# 1. The ORACLE: the handwritten module and its own selftest, unchanged.
#    This module allocates nothing and frees nothing -- verified below on the
#    generated object, not assumed -- so it has no allocation-failure path of
#    its own and its selftest wraps no allocator.
# ---------------------------------------------------------------------------
$header = Join-Path $hdrs 'l2src/lmx_owned_ranges.lm1.h'
$module = Join-Path $out 'lmx_owned_ranges.c'
$selfC = Join-Path $out 'selftest.c'
Step 'header' (Invoke-Native ((Q $l1trans) + ' l2src/lmx_owned_ranges.h.lm1 ' + (Q $header)) (Join-Path $out 'header.log')) (Join-Path $out 'header.log')
Step 'module' (Invoke-Native ((Q $l1trans) + ' l2src/lmx_owned_ranges.lm1 ' + (Q $module)) (Join-Path $out 'module.log')) (Join-Path $out 'module.log')
Step 'selftest_translate' (Invoke-Native ((Q $l1trans) + ' l2src/tests/lmx_owned_ranges_selftest.lm1 ' + (Q $selfC)) (Join-Path $out 'selftest.translate.log')) (Join-Path $out 'selftest.translate.log')

$refExe = Join-Path $out 'reference.exe'
$refLog = Join-Path $out 'reference.stdout.txt'
Step 'reference_compile' (Invoke-Native ("gcc $cflags -I " + (Q $hdrs) + ' ' + (Q $selfC) + ' ' + (Q $module) + ' -o ' + (Q $refExe)) (Join-Path $out 'reference.gcc.log')) (Join-Path $out 'reference.gcc.log')
$refExit = Invoke-Native (Q $refExe) $refLog
$refOut = (Get-Content -LiteralPath $refLog -Raw)
if ($refExit -ne 0) { throw "the handwritten module's own selftest failed: exit $refExit`n$refOut" }
if ($refOut -notmatch 'owned ranges checks=(\d+) failures=0') { throw "reference selftest did not report zero failures: $refOut" }
$refChecks = [int]$Matches[1]
$ev.reference = [ordered]@{ exit = $refExit; stdout = $refOut.Trim(); checks = $refChecks }

# ---------------------------------------------------------------------------
# 2. The translator from THIS checkout, and the L2 source it is given.
# ---------------------------------------------------------------------------
$l2c = Join-Path $out 'l2trans.c'
$l2exe = Join-Path $out 'l2trans.exe'
Step 'l2trans_translate' (Invoke-Native ((Q $l1trans) + ' l2src/l2trans.lm1 ' + (Q $l2c)) (Join-Path $out 'l2trans.translate.log')) (Join-Path $out 'l2trans.translate.log')
Step 'l2trans_compile' (Invoke-Native ("gcc $cflags -I lm1/build " + (Q $l2c) + ' l2src/lmx_poll_stub.c -o ' + (Q $l2exe)) (Join-Path $out 'l2trans.gcc.log')) (Join-Path $out 'l2trans.gcc.log')

$lm2 = 'l2src/lmx_owned_ranges.lm2'
$ev.sourceSHA256 = (Get-FileHash -LiteralPath $lm2).Hash
$gen = Join-Path $out 'generated.lm1'
$genLog = Join-Path $out 'module.l2trans.log'
$genExit = Invoke-Native ((Q $l2exe) + ' ' + (Q $lm2) + ' ' + (Q $gen)) $genLog
$genMsg = (Get-Content -LiteralPath $genLog -Raw)
if ($null -eq $genMsg) { $genMsg = '' }

# The construct the translator stops at TODAY, pinned. Codex owns the compiler
# slice that lifts it (ticket 20260913-095000); this runner must not pretend the
# port is done, and must not keep saying PENDING once it is.
$pendingLine = 20
$pendingText = 'incompatible entry signature'
$pendingWhat = 'a single-pointer formal of a foreign struct: @: LmxOwnedRange node'

if ($genExit -ne 0) {
    $m = [regex]::Match($genMsg, 'lmx_owned_ranges\.lm2:(\d+):(\d+): (.+?)\s*$', 'Multiline')
    if (-not $m.Success) { throw "l2trans rejected the source in a way this runner cannot read:`n$genMsg" }
    $line = [int]$m.Groups[1].Value
    $text = $m.Groups[3].Value.Trim()
    if ($line -ne $pendingLine -or $text -ne $pendingText) {
        throw "the pending rejection moved: expected ${pendingText} at line $pendingLine ($pendingWhat), got '$text' at line $line. Update the pin, or the remaining work list, whichever is now true."
    }
    $ev.pending = [ordered]@{ line = $line; message = $text; construct = $pendingWhat; translatorOutput = $genMsg.Trim() }
    $evPath = Join-Path $out 'evidence.json'
    $ev | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath $evPath -Encoding utf8
    Write-Output "lmx_owned_ranges parity PENDING: oracle PASS ($refChecks checks, 0 failures); l2trans stops at ${lm2}:${pendingLine} '$pendingText' -- $pendingWhat. Evidence $evPath"
    exit 0
}

throw "l2trans now ACCEPTS $lm2. The pending pin at line $pendingLine is stale: enable the parity half of this runner (splice, redirect, two runs, compare with the oracle) and delete the pin."
