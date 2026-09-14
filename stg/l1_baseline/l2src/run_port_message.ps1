# Runtime-module parity runner: lmx_message (review-chat lane, 2026-09-14).
#
# Run the executor selftest (lmx_message_exec_selftest.c, the Exec suite of
# run_lmx.ps1 and the module's accepted test) against the handwritten L1
# implementation and against one generated from L2 source by the CURRENT
# translator, and require the two to agree, twice.
#
# The generated half is a LIBRARY UNIT (7d7ec87c). Its shape is REDIRECT:
# lmx_msg_runtime_new, lmx_msg_create, lmx_msg_find, lmx_msg_graph and
# lmx_msg_runtime_delete are the unit's own open path, so a wrapper under a
# real name would re-enter l2_library_open before the unit's graph exists.
# The unit drops the lmx_ prefix from every method, the SELFTEST and the
# driver are redirected onto those names with -D, the runtime keeps the
# handwritten module under the real names (the executor and host C call it
# there), and nm proves the generated object defines the unit names and
# none of the ABI names. The port is what the TEST's own call sites exercise;
# the executor's internal calls (self_or_find, exec_ready, end_turn,
# drive_tree, live_check, ...) keep reaching the handwritten module until
# the translation-time unit graph lands.
#
# A C driver opens the unit with real calls BEFORE the selftest's main: a
# runtime, a root Message with init bytes, find, state, child_n, path_n,
# then delete. It reports "warm-up ok" on stderr.
#
# The selftest prints wall-clock fields (ui_ms, cpu_busy_ui_ms, ui_step_ms,
# t0/t1/t2) and threads interleave its stderr; the comparison masks those
# numbers and compares stderr as sorted lines. The reference is run twice
# first and must agree with itself under the same mask, or the mask is
# reported as insufficient rather than trusted.
#
# -SourcePath / -ExtraSources / -ExtraIncludeDirs exist for scratch copies
# (a tripwire mutation, or a copy that shims a translator gap); evidence.json
# records the source path and hash, so a scratch run is never mistaken for
# the tracked one.
#
# Nothing here replaces or deletes the L1 module.

[CmdletBinding()]
param(
    [string]$TranslatorPath,
    [ValidateRange(1, 3600)][int]$TestTimeoutSeconds = 900,
    [string]$SourcePath = 'l2src/lmx_message.lm2',
    [string[]]$ExtraSources = @(),
    [string[]]$ExtraIncludeDirs = @(),
    [switch]$LaneCheck
)

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

if (-not (Test-Path -LiteralPath $SourcePath)) { throw "missing L2 source $SourcePath" }
$lm2 = (Resolve-Path -LiteralPath $SourcePath).ProviderPath

$stamp = (Get-Date).ToString('yyyyMMdd_HHmmss_fff')
$out = Join-Path $baseline "build/port_message/$stamp"
$hdrs = Join-Path $out 'hdrs'
$support = Join-Path $out 'support'
$work = Join-Path $out 'work'
New-Item -ItemType Directory -Force -Path $out, (Join-Path $hdrs 'l2src'), $support, (Join-Path $work 'build/l1trans/logs/gen2') | Out-Null

$cflags = '-std=c99 -Wall -Wextra -Wpedantic -I . -Werror=incompatible-pointer-types -Werror=discarded-qualifiers -Werror=implicit-function-declaration -Werror=implicit-int'
$testDefine = '-DLMX_MSG_EXEC_TEST'

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

# Run one selftest binary in the work directory with a timeout; returns exit code.
function Invoke-Timed([string]$exe, [string]$stdoutPath, [string]$stderrPath) {
    $proc = Start-Process -FilePath $exe -WorkingDirectory $work -WindowStyle Hidden -PassThru `
        -RedirectStandardOutput $stdoutPath -RedirectStandardError $stderrPath
    $null = $proc.Handle
    if (-not $proc.WaitForExit($TestTimeoutSeconds * 1000)) {
        $proc.Kill()
        $proc.WaitForExit()
        throw "$exe timed out after $TestTimeoutSeconds seconds; see $stdoutPath / $stderrPath"
    }
    $proc.WaitForExit()
    $proc.Refresh()
    if ($null -eq $proc.ExitCode) { throw "$exe exited without an observable exit code" }
    return [int]$proc.ExitCode
}

function Mask([string]$s) {
    $t = $s.Replace("`r`n", "`n")
    $t = [regex]::Replace($t, '\b(ui_ms|cpu_busy_ui_ms|ui_step_ms|t0|t1|t2)=\d+', '$1=#')
    # %p prints: "fail waiting cleanup seen=%p", "owned move orig=%p got=%p",
    # "exec owner-batch/dual ... root=%p".
    $t = [regex]::Replace($t, '\b(seen|orig|got|root)=[0-9A-Fa-f]{8,16}\b', '$1=#')
    # Two concurrent admissions land in either order ("m0_acc=7 admit=2,5
    # apply=2,5 ... concurrent=1" and "admit=5,2 apply=5,2" both occur).
    $t = [regex]::Replace($t, '\b(admit|apply)=\d+,\d+', '$1=#,#')
    # ctx_spawn_race: the case's bind races the stop the main thread issues
    # right after releasing it; the selftest accepts both outcomes (done=1,
    # workers=0) and prints the bind status, 0 or 2 (0c, 2026-09-14, one red
    # parity run out of two with the lane check on).
    $t = [regex]::Replace($t, '\bctx_spawn_race st=\d+', 'ctx_spawn_race st=#')
    return $t.Trim()
}
function SortedLines([string]$s) {
    return (((Mask $s) -split "`n") | Where-Object { $_ -ne '' } | Sort-Object) -join "`n"
}

$ev = [ordered]@{ stamp = $stamp; baseline = $baseline; translator = $l1trans; translatorSHA256 = $l1transHash; source = $lm2; sourceSHA256 = (Get-FileHash -LiteralPath $lm2).Hash; extraSources = @($ExtraSources); extraIncludeDirs = @($ExtraIncludeDirs) }

# ---------------------------------------------------------------------------
# 0. The unit's method names are the redirect set: every lmx_msg_* the
#    handwritten module defines must have a method, and every method must
#    correspond to a handwritten definition (a scratch copy may add helpers;
#    those are recorded, not refused).
# ---------------------------------------------------------------------------
$srcText = [IO.File]::ReadAllText($lm2).Replace("`r`n", "`n")
$unitNames = [regex]::Matches($srcText, '(?m)^(?:fn|sub): (msg_[a-z0-9_]+) \(') | ForEach-Object { $_.Groups[1].Value }
if ($unitNames.Count -lt 80) { throw "the L2 source defines only $($unitNames.Count) msg_* methods" }
$oracleText = [IO.File]::ReadAllText((Resolve-Path -LiteralPath 'l2src/lmx_message.lm1').ProviderPath).Replace("`r`n", "`n")
$oracleNames = [regex]::Matches($oracleText, '(?m)^(?:fn|sub): (lmx_msg_[a-z0-9_]+) \(') | ForEach-Object { $_.Groups[1].Value }
foreach ($name in $oracleNames) {
    if ($unitNames -notcontains $name.Substring(4)) { throw "the handwritten module defines $name, but the L2 source has no method $($name.Substring(4))" }
}
$extraMethods = @($unitNames | Where-Object { $oracleNames -notcontains ('lmx_' + $_) })
$ev.methods = $unitNames.Count
$ev.oracleFunctions = $oracleNames.Count
$ev.extraMethods = $extraMethods
$redirects = $unitNames | ForEach-Object { @{ abi = 'lmx_' + $_; unit = $_ } }
$redirect = ($redirects | ForEach-Object { '-D' + $_.abi + '=' + $_.unit }) -join ' '

# ---------------------------------------------------------------------------
# 1. Support objects: the Exec suite's list from run_lmx.ps1, every object
#    under -DLMX_MSG_EXEC_TEST, history with its counted malloc/free. The
#    handwritten lmx_message stays in every link under the real names.
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
foreach ($source in $sources) {
    $stem = [IO.Path]::GetFileNameWithoutExtension($source)
    $obj = Join-Path $support ($stem + '.o')
    $glog = Join-Path $out ($stem + '.gcc.log')
    $defs = $testDefine
    if ($stem -eq 'lmx_msg_history_owned') { $defs = "$testDefine -Dmalloc=lmx_msg_history_test_malloc -Dfree=lmx_msg_history_test_free" }
    Step "compile_$stem" (Invoke-Native ("gcc $cflags $defs -I " + (Q $hdrs) + ' -I lm1/build -c ' + (Q $source) + ' -o ' + (Q $obj)) $glog) $glog
    $objs += $obj
}
# Stage 3c-2a: the production runtime includes the L2 runtime units
# (l2src/l2units_build.ps1; today lmx_sched_record.lm2, profile: runtime).
. l2src/l2units_build.ps1
$objs += @(Build-L2RuntimeUnits -L1Trans $l1trans -Out (Join-Path $out 'l2units') -IncludeDirs @($hdrs) -CFlags $cflags)
$objList = ($objs | ForEach-Object { Q $_ }) -join ' '

# ---------------------------------------------------------------------------
# 2. Reference: the executor selftest against the handwritten modules, twice,
#    so the comparison mask is itself checked before it judges the port.
# ---------------------------------------------------------------------------
$selfSrc = 'l2src/lmx_message_exec_selftest.c'
$selfObj = Join-Path $out 'selftest.o'
Step 'selftest_compile' (Invoke-Native ("gcc $cflags $testDefine -Dmain=exec_selftest_main -I " + (Q $hdrs) + ' -I lm1/build -c ' + $selfSrc + ' -o ' + (Q $selfObj)) (Join-Path $out 'selftest.gcc.log')) (Join-Path $out 'selftest.gcc.log')
# The crash report (l2src/tests/lmx_exec_crash_report.c) prints the faulting
# thread, address and raw stack on an access violation; silent otherwise. The
# reference links its main variant; the parity driver installs it itself.
$crashSrc = 'l2src/tests/lmx_exec_crash_report.c'
$crashMainObj = Join-Path $out 'crash_report_main.o'
Step 'crash_report_main_compile' (Invoke-Native ("gcc $cflags -DLMX_EXEC_CRASH_REPORT_MAIN -c " + $crashSrc + ' -o ' + (Q $crashMainObj)) (Join-Path $out 'crash_report_main.gcc.log')) (Join-Path $out 'crash_report_main.gcc.log')
$crashObj = Join-Path $out 'crash_report.o'
Step 'crash_report_compile' (Invoke-Native ("gcc $cflags -c " + $crashSrc + ' -o ' + (Q $crashObj)) (Join-Path $out 'crash_report.gcc.log')) (Join-Path $out 'crash_report.gcc.log')
$refExe = Join-Path $out 'reference.exe'
Step 'reference_link' (Invoke-Native ("gcc $cflags " + (Q $crashMainObj) + ' ' + (Q $selfObj) + ' ' + $objList + ' -o ' + (Q $refExe)) (Join-Path $out 'reference.gcc.log')) (Join-Path $out 'reference.gcc.log')

# Decision 18 oracle: with -LaneCheck the selftest runs abort at the first
# write of a scheduler cell off its owner's lane (LANE WRITE FAIL site=...);
# opt-in until the executor keeps every such write on the owner's lane.
if ($LaneCheck) { $env:LMX_LANE_CHECK = '1' } else { Remove-Item Env:LMX_LANE_CHECK -ErrorAction SilentlyContinue }
$ev.laneCheck = [bool]$LaneCheck
$refRuns = @()
foreach ($i in 1, 2) {
    $log = Join-Path $out "reference.$i.stdout.txt"
    $err = Join-Path $out "reference.$i.stderr.txt"
    $exit = Invoke-Timed $refExe $log $err
    $text = [IO.File]::ReadAllText($log)
    $errText = [IO.File]::ReadAllText($err)
    if ($exit -ne 0) { throw "the executor selftest failed against the handwritten modules on run $i`: exit $exit`n$text`n$errText" }
    if ($text -notmatch '(?m)^lmx_message_exec ok ') { throw "reference run $i did not report lmx_message_exec ok: $text" }
    $refRuns += [ordered]@{ exit = $exit; stdout = $text.Trim(); stderr = $errText.Trim() }
}
if ((Mask $refRuns[0].stdout) -ne (Mask $refRuns[1].stdout)) { throw "the reference disagrees with itself on stdout under the mask; the mask is insufficient`n$($refRuns[0].stdout)`n$($refRuns[1].stdout)" }
if ((SortedLines $refRuns[0].stderr) -ne (SortedLines $refRuns[1].stderr)) { throw "the reference disagrees with itself on stderr lines under the mask; the mask is insufficient" }
$refOut = Mask $refRuns[0].stdout
$refErr = SortedLines $refRuns[0].stderr
$ev.reference = $refRuns

# ---------------------------------------------------------------------------
# 3. The translator from THIS checkout, and the library unit it emits.
# ---------------------------------------------------------------------------
$l2c = Join-Path $out 'l2trans.c'
$l2exe = Join-Path $out 'l2trans.exe'
Step 'l2trans_translate' (Invoke-Native ((Q $l1trans) + ' l2src/l2trans.lm1 ' + (Q $l2c)) (Join-Path $out 'l2trans.translate.log')) (Join-Path $out 'l2trans.translate.log')
Step 'l2trans_compile' (Invoke-Native ("gcc $cflags -I lm1/build " + (Q $l2c) + ' l2src/lmx_poll_stub.c -o ' + (Q $l2exe)) (Join-Path $out 'l2trans.gcc.log')) (Join-Path $out 'l2trans.gcc.log')

$gen = Join-Path $out 'generated.lm1'
Step 'l2trans_module' (Invoke-Native ((Q $l2exe) + ' ' + (Q $lm2) + ' ' + (Q $gen)) (Join-Path $out 'module.l2trans.log')) (Join-Path $out 'module.l2trans.log')
$genText = [IO.File]::ReadAllText((Resolve-Path -LiteralPath $gen).ProviderPath).Replace("`r`n", "`n")

if ($genText -notmatch 'define: l2_program_entry l2_u[0-9A-F]{16}_entry') { throw 'the generated unit is not a library unit' }
# A runtime-profile unit (decision 14, translator 73e01271): the executor's own
# core is called during an unwind, so it must carry no escape poll at all; the
# checkpoint diagnostics (poll_abort) stay. Falsified by removing `profile:
# runtime` from the source: every method then polls.
if ($srcText -notmatch '(?m)^profile: runtime$') { throw 'lmx_message.lm2 must declare profile: runtime' }
if ($genText -match 'lmx_msg_poll_escape\(') { throw 'a runtime-profile unit emitted an escape poll' }
if (([regex]::Matches($genText, 'c\.lmx_msg_poll_abort\(\)')).Count -lt 1) { throw 'the checkpoint diagnostics are missing from the unit' }
foreach ($r in $redirects) {
    if ($genText -notmatch ('(?m)^    (?:fn|sub): ' + [regex]::Escape($r.unit) + ' \(')) { throw "the public wrapper is missing: $($r.unit)" }
}
foreach ($sig in @(
    'fn: msg_create \(@: LmxMsgRuntime rt; LmxMsgAddr: parent; unsigned: create_id; const: @\(uchar init\); size_t: n; @: LmxMsgAddr out\) int',
    'fn: msg_send \(@: LmxMsgRuntime rt; LmxMsgAddr: from; LmxMsgAddr: to; const: @\(LmxMsgEnv env\)\) int',
    'fn: msg_end_turn \(@: LmxMsgRuntime rt; LmxMsgAddr: who; int: success\) int',
    'fn: msg_recv \(@: LmxMsgRuntime rt; LmxMsgAddr: who; @: LmxMsgEnv out\) int',
    'fn: msg_runtime_new \(\) @: LmxMsgRuntime',
    'fn: msg_exec_take_addr \(@: LmxMsgRuntime rt\) unsigned')) {
    if ($genText -notmatch $sig) { throw "the public signature is missing or changed: $sig" }
}

# Structural assertions, each scoped to ONE method body so a match in the
# unit's own open path cannot satisfy it. A method body is the text from its
# generated definition to the next top-level fn:/sub:.
function MethodBody([string]$unitName) {
    # The prototype block repeats every wrapper's first line; only the real
    # wrapper is followed by the open-path check, so anchor on that line and
    # allow only the wrapper's own few lines before its method call.
    $m = [regex]::Match($genText, '(?m)^    fn: ' + [regex]::Escape($unitName) + ' \(.*\n        if: l2_library_open\(\) != 0\n(?:.*\n){0,6}?        return: (l2_u[0-9A-F]{16}_m\d+)\(')
    if (-not $m.Success) {
        $m = [regex]::Match($genText, '(?m)^    sub: ' + [regex]::Escape($unitName) + ' \(.*\n        if: l2_library_open\(\) != 0\n(?:.*\n){0,6}?        (l2_u[0-9A-F]{16}_m\d+)\(')
    }
    if (-not $m.Success) { throw "cannot find the method behind the wrapper $unitName" }
    $method = $m.Groups[1].Value
    $body = [regex]::Match($genText, '(?ms)^(?:fn|sub): ' + [regex]::Escape($method) + ' \(.*?^end: ' + [regex]::Escape($method) + '$')
    if (-not $body.Success) { throw "cannot find the body of $method ($unitName)" }
    return $body.Value
}
# send stages exactly one transport copy through the outbox under the mail
# lock and releases the source endpoint once.
$sendBody = MethodBody 'msg_send'
if (([regex]::Matches($sendBody, 'c\.calloc\(')).Count -ne 1) { throw 'msg_send does not allocate exactly one copy' }
if (([regex]::Matches($sendBody, 'c\.lmx_msg_mail_lock\(')).Count -ne 1 -or ([regex]::Matches($sendBody, 'c\.lmx_msg_mail_unlock\(')).Count -ne 1) { throw 'msg_send does not take and release the mail lock exactly once' }
if ($sendBody -notmatch 'c\.LMX_MSG_STAGED') { throw 'msg_send does not report STAGED' }
# admit_one refuses a duplicate delivery record before it retains the
# destination, and a STOP closes the destination without an inbox copy.
$admitBody = MethodBody 'msg_admit_one'
if ($admitBody -notmatch 'c\.LMX_MSG_DUPLICATE') { throw 'msg_admit_one does not refuse duplicates' }
if ($admitBody -notmatch 'c\.LMX_MSG_KIND_STOP') { throw 'msg_admit_one does not recognise STOP' }
if ($admitBody -notmatch 'c\.lmx_msg_test_on_admit\(') { throw 'msg_admit_one does not report to the admission hook' }
# arena_collect marks graph, roots and both classifier sets before it sweeps.
$collectBody = MethodBody 'msg_arena_collect'
if (([regex]::Matches($collectBody, 'lmx_msg_visit_dispose\(')).Count -lt 4) { throw 'msg_arena_collect does not dispose the visit set on every exit' }
if ($collectBody -notmatch 'eternal_ranges' -or $collectBody -notmatch 'method_ranges') { throw 'msg_arena_collect does not mark both classifier sets' }
# end_turn on failure releases only uncommitted inactive children.
$endBody = MethodBody 'msg_end_turn'
if ($endBody -notmatch 'committed = 0 && [a-z_0-9\\]+state = c\.LMX_MSG_STATE_INACTIVE') { throw 'msg_end_turn does not release exactly the uncommitted inactive children' }
if ($endBody -notmatch 'c\.lmx_msg_after_outbox_xfer\(') { throw 'msg_end_turn does not report the outbox transfer' }

# ---------------------------------------------------------------------------
# 4. The parity binary.
# ---------------------------------------------------------------------------
$genC = Join-Path $out 'generated.c'
Step 'generated_translate' (Invoke-Native ((Q $l1trans) + ' ' + (Q $gen) + ' ' + (Q $genC)) (Join-Path $out 'generated.translate.log')) (Join-Path $out 'generated.translate.log')
$extraInc = ''
foreach ($d in $ExtraIncludeDirs) { $extraInc += ' -I ' + (Q ((Resolve-Path -LiteralPath $d).ProviderPath)) }
$genObj = Join-Path $out 'generated.o'
# Extra include directories come FIRST so a scratch experiment can shadow a
# header for the unit alone (the support objects never see them).
Step 'generated_compile' (Invoke-Native ("gcc$extraInc $cflags $testDefine -I " + (Q $hdrs) + ' -I lm1/build -c ' + (Q $genC) + ' -o ' + (Q $genObj)) (Join-Path $out 'generated.gcc.log')) (Join-Path $out 'generated.gcc.log')
$extraObjs = @()
foreach ($src in $ExtraSources) {
    $srcPath = (Resolve-Path -LiteralPath $src).ProviderPath
    $stem = [IO.Path]::GetFileNameWithoutExtension($srcPath)
    $obj = Join-Path $out ("extra_$stem.o")
    Step "extra_$stem" (Invoke-Native ("gcc $cflags $testDefine$extraInc -I " + (Q $hdrs) + ' -I lm1/build -c ' + (Q $srcPath) + ' -o ' + (Q $obj)) (Join-Path $out "extra_$stem.gcc.log")) (Join-Path $out "extra_$stem.gcc.log")
    $extraObjs += $obj
}
$extraObjList = ($extraObjs | ForEach-Object { Q $_ }) -join ' '

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
#include "l2src/lmx_message.h"
int exec_selftest_main(int argc, char **argv);
void lmx_exec_crash_report_install(void);
int main(int argc, char **argv) {
    LmxMsgRuntime *rt;
    LmxMsgAddr a = 0;
    static const uchar init[4] = { 'w', 'a', 'r', 'm' };
    lmx_exec_crash_report_install();
    /* Every call below is redirected onto the generated unit; the first one
       opens the library. */
    rt = lmx_msg_runtime_new();
    if (rt == 0) { fprintf(stderr, "warm-up: runtime_new\n"); return 90; }
    if (lmx_msg_create(rt, 0U, 7U, init, 4U, &a) != LMX_MSG_OK || a == 0) { fprintf(stderr, "warm-up: create\n"); return 91; }
    if (lmx_msg_find(rt, a) == 0 || lmx_msg_state(rt, a) != LMX_MSG_STATE_RUNNING) { fprintf(stderr, "warm-up: find/state\n"); return 92; }
    if (lmx_msg_child_n(rt, a) != 0 || lmx_msg_path_n(rt, a) != 1 || lmx_msg_inbox_n(rt, a) != 0) { fprintf(stderr, "warm-up: queries\n"); return 93; }
    if (lmx_msg_create(rt, 0U, 7U, init, 4U, &a) != LMX_MSG_OK) { fprintf(stderr, "warm-up: create again\n"); return 94; }
    if (lmx_msg_find(rt, 999U) != 0 || lmx_msg_state(rt, 999U) != -1) { fprintf(stderr, "warm-up: unknown address\n"); return 95; }
    lmx_msg_runtime_delete(rt);
    fprintf(stderr, "warm-up ok\n");
    return exec_selftest_main(argc, argv);
}
'@.Replace("`r`n", "`n") | Set-Content -LiteralPath $driverC -Encoding ascii -NoNewline

$selfDrivenObj = Join-Path $out 'selftest_driven.o'
Step 'selftest_driven_compile' (Invoke-Native ("gcc $cflags $testDefine $redirect -Dmain=exec_selftest_main -I " + (Q $hdrs) + ' -I lm1/build -c ' + $selfSrc + ' -o ' + (Q $selfDrivenObj)) (Join-Path $out 'selftest_driven.gcc.log')) (Join-Path $out 'selftest_driven.gcc.log')
$driverObj = Join-Path $out 'parity_driver.o'
Step 'driver_compile' (Invoke-Native ("gcc $cflags $testDefine $redirect -I " + (Q $hdrs) + ' -I lm1/build -c ' + (Q $driverC) + ' -o ' + (Q $driverObj)) (Join-Path $out 'driver.gcc.log')) (Join-Path $out 'driver.gcc.log')

$parityExe = Join-Path $out 'parity.exe'
Step 'parity_link' (Invoke-Native ("gcc $cflags " + (Q $crashObj) + ' ' + (Q $driverObj) + ' ' + (Q $selfDrivenObj) + ' ' + (Q $genObj) + ' ' + $extraObjList + ' ' + $objList + ' -o ' + (Q $parityExe)) (Join-Path $out 'parity.gcc.log')) (Join-Path $out 'parity.gcc.log')

# ---------------------------------------------------------------------------
# 5. Two runs, so a pass that depends on run order or leftover state shows.
# ---------------------------------------------------------------------------
$runs = @()
foreach ($i in 1, 2) {
    $log = Join-Path $out "parity.$i.stdout.txt"
    $err = Join-Path $out "parity.$i.stderr.txt"
    $exit = Invoke-Timed $parityExe $log $err
    $text = [IO.File]::ReadAllText($log)
    $errText = [IO.File]::ReadAllText($err)
    $runs += [ordered]@{ exit = $exit; stdout = $text.Trim(); stderr = $errText.Trim() }
    if ($errText -notmatch 'warm-up ok') { throw "parity run $i did not open the unit through real calls: $errText" }
    if ($errText -match 'library open failed') { throw "parity run $i reported a library open failure: $errText" }
    if ($exit -ne 0) { throw "parity run $i exit $exit, reference exit 0`n$text`n$errText" }
    if ((Mask $text) -ne $refOut) { throw "parity run $i stdout differs from the reference`nreference: $refOut`nparity   : $(Mask $text)" }
    $errLines = SortedLines ($errText -replace '(?m)^warm-up ok\n?', '')
    if ($errLines -ne $refErr) { throw "parity run $i stderr lines differ from the reference`n--- reference`n$refErr`n--- parity`n$errLines" }
}
$ev.parity = $runs
$ev.generatedSHA256 = (Get-FileHash -LiteralPath $gen).Hash
$ev.l2transSHA256 = (Get-FileHash -LiteralPath $l2exe).Hash

$evPath = Join-Path $out 'evidence.json'
$ev | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath $evPath -Encoding utf8
$scratchNote = ''
if ($lm2 -ne (Resolve-Path -LiteralPath 'l2src/lmx_message.lm2').ProviderPath -or $ExtraSources.Count -gt 0 -or $ExtraIncludeDirs.Count -gt 0) { $scratchNote = " [SCRATCH: source $lm2, extra objects $($ExtraSources.Count), shadow include dirs $($ExtraIncludeDirs.Count); not evidence for the tracked source as built]" }
Write-Output "lmx_message parity PASS: executor selftest reports lmx_message_exec ok, reference and generated agree on both runs (wall-clock fields masked, stderr as sorted lines), $($unitNames.Count) methods redirected; evidence $evPath$scratchNote"
