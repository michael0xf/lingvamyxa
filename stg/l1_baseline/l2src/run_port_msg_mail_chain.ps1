# Runtime-module parity runner: lmx_msg_mail_chain (review-chat lane, 2026-09-14).
#
# Run the module's own selftest against the handwritten L1 implementation and
# against one generated from L2 source by the CURRENT translator, and require
# the two to agree exactly, twice.
#
# The generated half is a LIBRARY UNIT (7d7ec87c): no entry, module-unique
# internals, public wrappers carrying the source signatures. No splice.
#
# Like lmx_msg_slots and unlike storage/blocks/ranges, this module can EXPORT
# THE REAL SYMBOLS: nothing in the runtime calls the mailbox chain operations
# while a unit graph is being built. The only non-test callers are the mailbox
# wrappers in lmx_message_exec.c, reached on send/take, never from
# l2_library_open. So the selftest is UNMODIFIED, links by name, and the
# handwritten object is kept OUT of the parity link: the generated code is the
# only definition of lmx_msg_mail_chain_empty/_n/_take the test can reach. If
# the runtime did call one of them during graph construction, the generated
# wrapper would re-enter l2_library_open and the run would not print the
# reference line -- a visible failure, not a silent fallback.
#
# The module allocates, copies, frees and locks nothing; take is O(1). Those
# are asserted of the generated L1 below, not taken on trust.
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

# The pin lives in ONE place. Whatever translator was selected, it must be the
# pinned one; a wrong hash or a malformed pin file refuses the run.
$pinFile = Join-Path $PSScriptRoot 'L1_PIN.txt'
$pin = (Get-Content -LiteralPath $pinFile -TotalCount 1).Trim()
if ($pin -notmatch '^[0-9A-F]{64}$') { throw "l2src/L1_PIN.txt must hold one 64-hex SHA256, got '$pin'" }
$l1transHash = (Get-FileHash -LiteralPath $l1trans).Hash
if ($l1transHash -ne $pin) { throw "translator $l1trans is $l1transHash, but l2src/L1_PIN.txt pins $pin" }

$stamp = (Get-Date).ToString('yyyyMMdd_HHmmss_fff')
$out = Join-Path $baseline "build/port_msg_mail_chain/$stamp"
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
# 1. Support objects: the runtime set the graph gate links. The handwritten
#    lmx_msg_mail_chain is built too, but kept OUT of the parity link.
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
$nativeObj = $null
foreach ($source in $sources) {
    $stem = [IO.Path]::GetFileNameWithoutExtension($source)
    $obj = Join-Path $support ($stem + '.o')
    $glog = Join-Path $out ($stem + '.gcc.log')
    Step "compile_$stem" (Invoke-Native ("gcc $cflags -I " + (Q $hdrs) + ' -c ' + (Q $source) + ' -o ' + (Q $obj)) $glog) $glog
    if ($stem -eq 'lmx_msg_mail_chain') { $nativeObj = $obj } else { $objs += $obj }
}
if (-not $nativeObj) { throw 'the native mail_chain object was not built' }
$objList = ($objs | ForEach-Object { Q $_ }) -join ' '

# ---------------------------------------------------------------------------
# 2. Reference run: the existing selftest against the handwritten module. The
#    module is self-contained (no runtime), so the reference links only the
#    two of them, as l2src/run_msg_mail_chain.ps1 does.
# ---------------------------------------------------------------------------
$selfSrc = 'l2src/tests/lmx_msg_mail_chain_selftest.lm1'
$selfC = Join-Path $out 'selftest.c'
Step 'selftest_translate' (Invoke-Native ((Q $l1trans) + ' ' + $selfSrc + ' ' + (Q $selfC)) (Join-Path $out 'selftest.translate.log')) (Join-Path $out 'selftest.translate.log')
$selfObj = Join-Path $out 'selftest.o'
Step 'selftest_compile' (Invoke-Native ("gcc $cflags -I " + (Q $hdrs) + ' -c ' + (Q $selfC) + ' -o ' + (Q $selfObj)) (Join-Path $out 'selftest.gcc.log')) (Join-Path $out 'selftest.gcc.log')

$refExe = Join-Path $out 'reference.exe'
$refLog = Join-Path $out 'reference.stdout.txt'
Step 'reference_link' (Invoke-Native ("gcc $cflags " + (Q $selfObj) + ' ' + (Q $nativeObj) + ' -o ' + (Q $refExe)) (Join-Path $out 'reference.gcc.log')) (Join-Path $out 'reference.gcc.log')
$refExit = Invoke-Native (Q $refExe) $refLog
$refOut = (Get-Content -LiteralPath $refLog -Raw)
if ($refExit -ne 0) { throw "the handwritten module's own selftest failed: exit $refExit`n$refOut" }
if ($refOut -notmatch 'mail_chain checks=(\d+) failures=0') { throw "reference selftest did not report zero failures: $refOut" }
$refChecks = [int]$Matches[1]
$ev.reference = [ordered]@{ exit = $refExit; stdout = $refOut.Trim(); checks = $refChecks }

# ---------------------------------------------------------------------------
# 3. The translator from THIS checkout, and the library unit it emits.
# ---------------------------------------------------------------------------
$l2c = Join-Path $out 'l2trans.c'
$l2exe = Join-Path $out 'l2trans.exe'
Step 'l2trans_translate' (Invoke-Native ((Q $l1trans) + ' l2src/l2trans.lm1 ' + (Q $l2c)) (Join-Path $out 'l2trans.translate.log')) (Join-Path $out 'l2trans.translate.log')
Step 'l2trans_compile' (Invoke-Native ("gcc $cflags -I lm1/build " + (Q $l2c) + ' l2src/lmx_poll_stub.c -o ' + (Q $l2exe)) (Join-Path $out 'l2trans.gcc.log')) (Join-Path $out 'l2trans.gcc.log')

$lm2 = 'l2src/lmx_msg_mail_chain.lm2'
$ev.sourceSHA256 = (Get-FileHash -LiteralPath $lm2).Hash
$gen = Join-Path $out 'generated.lm1'
Step 'l2trans_module' (Invoke-Native ((Q $l2exe) + ' ' + (Q $lm2) + ' ' + (Q $gen)) (Join-Path $out 'module.l2trans.log')) (Join-Path $out 'module.l2trans.log')
$genText = [IO.File]::ReadAllText((Resolve-Path -LiteralPath $gen).ProviderPath).Replace("`r`n", "`n")

# It is a library unit, and its public signatures are the module's own ABI
# names, exactly as the L1 header declares them.
if ($genText -notmatch 'define: l2_program_entry l2_u[0-9A-F]{16}_entry') { throw 'the generated unit is not a library unit' }
foreach ($sig in @(
    'fn: lmx_msg_mail_chain_empty \(const: @\(LmxMsgCopy head\)\) int',
    'fn: lmx_msg_mail_chain_n \(const: @\(LmxMsgCopy head\)\) int',
    '(fn|sub): lmx_msg_mail_chain_take \(@@: LmxMsgCopy head; @@: LmxMsgCopy tail; @@: LmxMsgCopy out\)')) {
    if ($genText -notmatch $sig) { throw "the public signature is missing or changed: $sig" }
}
# No allocation, copy, free or lock anywhere in the unit: the contract of
# LMX_MSG_MAIL_CHAIN.txt, checked on the emitted text.
if ($genText -match 'c\.(malloc|calloc|realloc|free|memcpy|memmove)\(') { throw 'the generated unit allocates, copies or frees; the handwritten one does not' }
# The count follows next and nothing else.
$mN = [regex]::Match($genText, '(?ms)^fn: l2_u[0-9A-F]{16}_m1 \(.*?^end: l2_u[0-9A-F]{16}_m1')
if (-not $mN.Success) { throw 'the count method was not emitted' }
if ($mN.Value -notmatch 'while:') { throw 'the count does not loop' }
if ($mN.Value -notmatch '\\next') { throw 'the count does not follow next' }
# take is O(1): no loop, no link followed, three raw stores through the cells.
$mT = [regex]::Match($genText, '(?ms)^(fn|sub): l2_u[0-9A-F]{16}_m2 \(.*?^end: l2_u[0-9A-F]{16}_m2')
if (-not $mT.Success) { throw 'the take method was not emitted' }
if ($mT.Value -match 'while:|\\next') { throw 'take visits the chain' }
if (([regex]::Matches($mT.Value, '(?m)^\s*\\l2_p2_[012]: ')).Count -ne 3) { throw 'take does not store exactly three cells' }

# ---------------------------------------------------------------------------
# 4. The parity binary: the UNMODIFIED selftest linked by name against the
#    generated unit and the runtime, WITHOUT the handwritten object.
# ---------------------------------------------------------------------------
$genC = Join-Path $out 'generated.c'
Step 'generated_translate' (Invoke-Native ((Q $l1trans) + ' ' + (Q $gen) + ' ' + (Q $genC)) (Join-Path $out 'generated.translate.log')) (Join-Path $out 'generated.translate.log')
$genObj = Join-Path $out 'generated.o'
Step 'generated_compile' (Invoke-Native ("gcc $cflags -I " + (Q $hdrs) + ' -I lm1/build -c ' + (Q $genC) + ' -o ' + (Q $genObj)) (Join-Path $out 'generated.gcc.log')) (Join-Path $out 'generated.gcc.log')

# The three ABI symbols are DEFINED by the generated object. With the native
# object absent from the link, this is what the selftest will call.
$nmLog = Join-Path $out 'generated.nm.txt'
Step 'generated_nm' (Invoke-Native ('nm --defined-only ' + (Q $genObj)) $nmLog) $nmLog
$nmText = [IO.File]::ReadAllText($nmLog)
foreach ($sym in @('lmx_msg_mail_chain_empty', 'lmx_msg_mail_chain_n', 'lmx_msg_mail_chain_take')) {
    if ($nmText -notmatch ('(?m)^\S+\s+T\s+_?' + $sym + '\s*$')) { throw "the generated object does not define $sym" }
}

$parityExe = Join-Path $out 'parity.exe'
Step 'parity_link' (Invoke-Native ("gcc $cflags " + (Q $selfObj) + ' ' + (Q $genObj) + ' ' + $objList + ' -o ' + (Q $parityExe)) (Join-Path $out 'parity.gcc.log')) (Join-Path $out 'parity.gcc.log')

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
}
if ($runs[0].stdout -ne $runs[1].stdout -or $runs[0].stderr -ne $runs[1].stderr) { throw 'the two parity runs disagree' }
$ev.parity = $runs
$ev.generatedSHA256 = (Get-FileHash -LiteralPath $gen).Hash
$ev.l2transSHA256 = (Get-FileHash -LiteralPath $l2exe).Hash

$evPath = Join-Path $out 'evidence.json'
$ev | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath $evPath -Encoding utf8
Write-Output "lmx_msg_mail_chain parity PASS: $refChecks checks, 0 failures, reference and generated agree on both runs, real symbols from the generated unit; evidence $evPath"
