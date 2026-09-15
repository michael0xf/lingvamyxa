# Core acceptance runner for stage 5 step (a): a process entry runs in its root
# Message's own turn. Builds the production runtime from this checkout with the
# pinned L1 translator (the L1 modules, lmx_message_host.c, lmx_message_exec.c
# and the L2 runtime units through l2units_build), builds l2trans from the
# tree, emits a main-mode unit (tests/entry_turn_main.lm2) and a library unit
# (tests/entry_turn_lib.lm2), checks the emitted entry's shape, links the L1
# selftest tests/lmx_entry_turn_selftest.lm1 against all of it, runs it twice.

[CmdletBinding()]
param(
    [string]$TranslatorPath,
    [ValidateRange(1, 3600)][int]$TestTimeoutSeconds = 120
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

$stamp = (Get-Date).ToString('yyyyMMdd_HHmmss_fff')
$out = Join-Path $baseline "build/entry_turn/$stamp"
$hdrs = Join-Path $out 'hdrs'
$support = Join-Path $out 'support'
$work = Join-Path $out 'work'
New-Item -ItemType Directory -Force -Path $out, (Join-Path $hdrs 'l2src'), $support, $work | Out-Null

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

$test = 'lmx_entry_turn_selftest'
$ev = [ordered]@{ stamp = $stamp; baseline = $baseline; translator = $l1trans; translatorSHA256 = $l1transHash; testSHA256 = (Get-FileHash -LiteralPath "l2src/tests/$test.lm1").Hash }

# 1. The production runtime.
$names = @('lmx_msg_blocks', 'lmx_owned_ranges', 'lmx_msg_storage', 'lmx_msg_path_storage', 'lmx_msg_slots', 'lmx_msg_mail_chain', 'lmx_msg_sched_ready', 'lmx_msg_visit', 'lmx_msg_liveness', 'lmx_msg_history_owned', 'lmx_msg_roots_stale', 'lmx_branch_owned', 'lmx_value_owned', 'lmx_chars_owned', 'lmx_array_owned', 'lmx_array_ref_owned', 'lmx_graph_copy_owned', 'lmx_merge_owned', 'lmx_message_graph_copy') | Where-Object { Test-Path -LiteralPath "l2src/$_.lm1" }
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
    Step "compile_$stem" (Invoke-Native ("gcc $cflags -I " + (Q $hdrs) + ' -I lm1/build -c ' + (Q $source) + ' -o ' + (Q $obj)) $glog) $glog
    $objs += $obj
}
. (Join-Path $PSScriptRoot 'l2units_build.ps1')
$objs += @(Build-L2RuntimeUnits -L1Trans $l1trans -Out (Join-Path $out 'l2units') -IncludeDirs @($hdrs) -CFlags $cflags)
$objList = ($objs | ForEach-Object { Q $_ }) -join ' '

# 2. The translator from this checkout and the two units it emits.
$l2c = Join-Path $out 'l2trans.c'
$l2exe = Join-Path $out 'l2trans.exe'
Step 'l2trans_translate' (Invoke-Native ((Q $l1trans) + ' l2src/l2trans.lm1 ' + (Q $l2c)) (Join-Path $out 'l2trans.translate.log')) (Join-Path $out 'l2trans.translate.log')
Step 'l2trans_compile' (Invoke-Native ("gcc $cflags -I lm1/build " + (Q $l2c) + ' l2src/lmx_poll_stub.c -o ' + (Q $l2exe)) (Join-Path $out 'l2trans.gcc.log')) (Join-Path $out 'l2trans.gcc.log')
$unitObjs = @()
foreach ($unit in @(@{ stem = 'entry_turn_main'; define = '-Dmain=entry_turn_main' }, @{ stem = 'entry_turn_lib'; define = '' }, @{ stem = 'entry_turn_abort'; define = '-Dmain=entry_turn_abort_main -Dl2_program_entry=entry_turn_abort_entry -Dl2_program_body=entry_turn_abort_body -Dl2_program_turn=entry_turn_abort_turn -Dl2_m0=entry_turn_abort_m0' })) {
    $gen = Join-Path $out ($unit.stem + '.lm1')
    Step ($unit.stem + '_l2trans') (Invoke-Native ((Q $l2exe) + ' l2src/tests/' + $unit.stem + '.lm2 ' + (Q $gen)) (Join-Path $out ($unit.stem + '.l2trans.log'))) (Join-Path $out ($unit.stem + '.l2trans.log'))
    $text = [IO.File]::ReadAllText((Resolve-Path -LiteralPath $gen).ProviderPath).Replace("`r`n", "`n")
    # The body is called by its adapter alone and the adapter by its turn alone;
    # a definition is spelled `fn: name (` with a space, so these count calls.
    if ([regex]::Matches($text, 'l2_program_body\(').Count -ne 1) { throw "$($unit.stem): the entry body is called outside its adapter" }
    if ([regex]::Matches($text, 'l2_program_entry\(').Count -ne 1) { throw "$($unit.stem): the entry adapter is called outside its turn" }
    if ($text -notmatch 'if: c\.lmx_msg_exec_holding_turn\(process_message\\owner_rt, process_message\\addr\) = 0') { throw "$($unit.stem): the entry adapter does not refuse outside its Message's turn" }
    if ($unit.stem -ne 'entry_turn_lib') {
        if ($text -match 'define: l2_program_entry l2_u') { throw "$($unit.stem) was emitted as a library unit" }
        if ($text -notmatch 'c\.lmx_msg_run_entry_turn\(process_runtime, process_addr, l2_program_turn, ') { throw 'the generated main does not reach its entry through the bootstrap' }
    } else {
        if ($text -notmatch 'define: l2_program_entry l2_u[0-9A-F]{16}_entry' -or $text -notmatch 'define: l2_program_turn l2_u[0-9A-F]{16}_turn') { throw 'entry_turn_lib is not a library unit with a module-unique turn' }
        if ($text -notmatch 'c\.lmx_msg_run_entry_turn\(l2_library_runtime, process_addr, l2_program_turn, ') { throw 'l2_library_open does not reach its entry through the bootstrap' }
    }
    $genC = Join-Path $out ($unit.stem + '.c')
    Step ($unit.stem + '_translate') (Invoke-Native ((Q $l1trans) + ' ' + (Q $gen) + ' ' + (Q $genC)) (Join-Path $out ($unit.stem + '.translate.log'))) (Join-Path $out ($unit.stem + '.translate.log'))
    $genObj = Join-Path $out ($unit.stem + '.o')
    Step ($unit.stem + '_compile') (Invoke-Native ("gcc $cflags $($unit.define) -I " + (Q $hdrs) + ' -I lm1/build -c ' + (Q $genC) + ' -o ' + (Q $genObj)) (Join-Path $out ($unit.stem + '.gcc.log'))) (Join-Path $out ($unit.stem + '.gcc.log'))
    $unitObjs += $genObj
    if ($unit.stem -eq 'entry_turn_abort') {
        # The same program as a process: its exit status is main's result.
        $abortObj = Join-Path $out 'entry_turn_abort.process.o'
        Step 'entry_turn_abort_process_compile' (Invoke-Native ("gcc $cflags -I " + (Q $hdrs) + ' -I lm1/build -c ' + (Q $genC) + ' -o ' + (Q $abortObj)) (Join-Path $out 'entry_turn_abort.process.gcc.log')) (Join-Path $out 'entry_turn_abort.process.gcc.log')
        $abortExe = Join-Path $out 'entry_turn_abort.exe'
        $abortProcessObj = $abortObj
    }
}

# 3. The selftest, twice.
$testC = Join-Path $out "$test.c"
Step 'test_translate' (Invoke-Native ((Q $l1trans) + " l2src/tests/$test.lm1 " + (Q $testC)) (Join-Path $out "$test.translate.log")) (Join-Path $out "$test.translate.log")
$testObj = Join-Path $out "$test.o"
Step 'test_compile' (Invoke-Native ("gcc $cflags -I " + (Q $hdrs) + ' -I lm1/build -c ' + (Q $testC) + ' -o ' + (Q $testObj)) (Join-Path $out "$test.gcc.log")) (Join-Path $out "$test.gcc.log")
$exe = Join-Path $out "$test.exe"
Step 'test_link' (Invoke-Native ("gcc $cflags -Wl,--wrap=lmx_msg_runtime_delete " + (Q $testObj) + ' ' + (($unitObjs | ForEach-Object { Q $_ }) -join ' ') + ' ' + $objList + ' -o ' + (Q $exe)) (Join-Path $out "$test.link.log")) (Join-Path $out "$test.link.log")
# The aborting program as its own process: exit status 1, no crash.
Step 'abort_link' (Invoke-Native ("gcc $cflags " + (Q $abortProcessObj) + ' ' + $objList + ' -o ' + (Q $abortExe)) (Join-Path $out 'entry_turn_abort.link.log')) (Join-Path $out 'entry_turn_abort.link.log')
$abortExit = Invoke-Timed $abortExe (Join-Path $out 'entry_turn_abort.stdout.txt') (Join-Path $out 'entry_turn_abort.stderr.txt')
if ($abortExit -ne 1) { throw "the aborting program's process exit status is $abortExit, not 1 (main's result after the turn root took the abort)" }
$ev.abortProcessExit = $abortExit
$runs = @()
$checks = 0
foreach ($i in 1, 2) {
    $log = Join-Path $out "$test.$i.stdout.txt"
    $err = Join-Path $out "$test.$i.stderr.txt"
    $exit = Invoke-Timed $exe $log $err
    $text = [IO.File]::ReadAllText($log).Replace("`r`n", "`n")
    $errText = [IO.File]::ReadAllText($err).Replace("`r`n", "`n")
    $runs += [ordered]@{ exit = $exit; stdout = $text.Trim(); stderr = $errText.Trim() }
    if ($exit -ne 0) { throw "$test run $i exit $exit`n$text`n$errText" }
    if ($text -notmatch '(?m)^entry turn: (\d+) checks, 0 failures$') { throw "$test run $i did not report zero failures: $text" }
    $checks = [int]$Matches[1]
    if ($errText -match 'library open failed') { throw "$test run $i reported a library open failure: $errText" }
}
if ($runs[0].stdout -ne $runs[1].stdout -or $runs[0].stderr -ne $runs[1].stderr) { throw 'the two runs disagree' }
$ev.runs = $runs
$ev.checks = $checks
$evPath = Join-Path $out 'evidence.json'
$ev | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath $evPath -Encoding utf8
Write-Output "entry turn PASS: $checks checks, 0 failures, two runs agree, body and adapter each called once, aborting program exits 1; evidence $evPath"
