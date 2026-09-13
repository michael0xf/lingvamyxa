# Graph ABI conformance runner (Fable lane, 2026-09-12).
#
# One private translator build from THIS checkout's l2trans.lm1, one Message
# support object set built from THIS checkout's lmx.h / graph modules, then:
#   1. the direct L1 selftest of the void * child-slot representation;
#   2. every historical positive L2 fixture (Invoke-Positive / Invoke-Leaf /
#      Invoke-LeafOut lines of run_l2trans.ps1) translated L2 -> L1 -> C -> exe
#      and executed against the exit code / stdout that runner already pins,
#      with the generated L1 checked for the slot API and against the retired
#      inline-child spellings.
# The stable compiler is used read-only and pin-checked. Evidence goes to an
# immutable run directory with source/artifact hashes. Splice drives with
# handwritten inline-child bodies are NOT run here; they are listed as
# migration work in FABLE_GRAPH_ABI.txt.
#
# Native tools run through cmd /c with merged stdout/stderr logs: under
# PowerShell 5.1 a redirected native stderr line becomes a terminating
# NativeCommandError when $ErrorActionPreference is Stop, so the frozen
# parser's pre-existing -Wunused-parameter warnings must never reach the
# PowerShell error stream.
param(
    [string]$TranslatorPath = '',
    [string]$EvidenceRoot = '',
    [switch]$SelftestOnly
)
$ErrorActionPreference = 'Stop'
$baseline = Split-Path -Parent $PSScriptRoot
$repo = Split-Path -Parent (Split-Path -Parent $baseline)
Set-Location $baseline

$pin = '65D5A5ED127CA1BAEBDD1D500A5B74CEEA63EC1985EAC52EDEF28EFEB261C936'
if (-not $TranslatorPath) { $TranslatorPath = 'build/l1trans/gen2/l1trans.exe' }
if (-not (Test-Path -LiteralPath $TranslatorPath)) { throw "missing translator: $TranslatorPath" }
$transHash = (Get-FileHash -LiteralPath $TranslatorPath).Hash
if ($transHash -ne $pin) { throw "translator $TranslatorPath hash $transHash is not the stable pin" }
$l1trans = (Resolve-Path -LiteralPath $TranslatorPath).ProviderPath

if (-not $EvidenceRoot) { $EvidenceRoot = Join-Path $repo 'build/fable/graph_abi' }
$stamp = Get-Date -Format 'yyyyMMdd_HHmmss_fff'
$run = Join-Path $EvidenceRoot ('run_' + $stamp + '_' + [guid]::NewGuid().ToString('N').Substring(0, 8))
New-Item -ItemType Directory -Force -Path $run | Out-Null
$out = Join-Path $baseline "build/graph_abi/$stamp"
$supportDir = Join-Path $out 'message_support'
$hdrs = Join-Path $supportDir 'headers'
New-Item -ItemType Directory -Force -Path $out, (Join-Path $hdrs 'l2src') | Out-Null

$cflags = '-std=c99 -Wall -Wextra -Wpedantic -I . -Werror=incompatible-pointer-types -Werror=discarded-qualifiers -Werror=implicit-function-declaration -Werror=implicit-int'

$ev = [ordered]@{
    result = 'RUNNING'
    checkout = $repo
    gitHead = (git -C $repo rev-parse HEAD 2>$null)
    translator = $l1trans
    translatorSHA256 = $transHash
    runner = $PSCommandPath
    runnerSHA256 = (Get-FileHash -LiteralPath $PSCommandPath).Hash
    sources = [ordered]@{}
    stages = @()
    selftest = [ordered]@{}
    fixtures = @()
    fixturesPassed = 0
    fixturesFailed = 0
}
$owned = @('l2src/lmx.h', 'l2src/lmx_branch_owned.h.lm1', 'l2src/lmx_branch_owned.lm1',
    'l2src/lmx_value_owned.h.lm1', 'l2src/lmx_value_owned.lm1', 'l2src/lmx_chars_owned.lm1',
    'l2src/lmx_array_owned.lm1', 'l2src/l2trans.lm1', 'l2src/tests/lmx_graph_abi_selftest.lm1',
    'l2src/lmx_graph_copy_owned.h.lm1', 'l2src/lmx_graph_copy_owned.lm1', 'l2src/tests/lmx_graph_copy_selftest.lm1',
    'l2src/lmx_merge_owned.h.lm1', 'l2src/lmx_merge_owned.lm1', 'l2src/tests/lmx_merge_selftest.lm1',
    'l2src/lmx_message_graph_copy.h.lm1', 'l2src/lmx_message_graph_copy.lm1', 'l2src/tests/lmx_message_graph_copy_selftest.lm1',
    'l2src/lmx_message.lm1', 'l2src/lmx_message.h', 'l2src/lmx_message_exec.c')
foreach ($s in $owned) { $ev.sources[$s] = (Get-FileHash -LiteralPath $s).Hash }

function Invoke-Native([string]$commandLine, [string]$logPath) {
    # Merged stdout+stderr into $logPath; exit code returned, never thrown by PS.
    cmd /c "$commandLine > `"$logPath`" 2>&1"
    return $LASTEXITCODE
}
function Q([string]$p) { return '"' + $p + '"' }
function Stage([string]$name, [int]$code, [string]$logPath = '') {
    $script:ev.stages += [ordered]@{ name = $name; exit = $code }
    if ($code -ne 0) {
        if ($logPath -and (Test-Path -LiteralPath $logPath)) { Get-Content -LiteralPath $logPath | Select-Object -First 40 | ForEach-Object { Write-Host $_ } }
        throw "$name exit $code"
    }
}
function Save-Evidence([string]$result) {
    $script:ev.result = $result
    $script:ev.finishedAt = [DateTime]::UtcNow.ToString('o')
    ($script:ev | ConvertTo-Json -Depth 6) | Set-Content -LiteralPath (Join-Path $run 'evidence.json') -Encoding UTF8
}

$retired = 'lmx_branch_child_known\(|leaf\\data|kid\\data|l2_xp\\data|_from\\data|_leaf\\data|leaf\\node|@: Lmx l2_xp|@: Lmx l2_q\d+_from|@: Lmx l2_a\d+_leaf'

try {
    # Translator build from this checkout.
    $l2c = Join-Path $out 'l2trans.c'
    $l2exe = Join-Path $out 'l2trans.exe'
    Stage 'l2trans_translate' (Invoke-Native ((Q $l1trans) + ' l2src/l2trans.lm1 ' + (Q $l2c)) (Join-Path $run 'l2trans.translate.log')) (Join-Path $run 'l2trans.translate.log')
    Stage 'l2trans_compile' (Invoke-Native ("gcc $cflags -I lm1/build " + (Q $l2c) + ' l2src/lmx_poll_stub.c -o ' + (Q $l2exe)) (Join-Path $run 'l2trans.gcc.log')) (Join-Path $run 'l2trans.gcc.log')
    $ev.l2transSHA256 = (Get-FileHash -LiteralPath $l2exe).Hash

    # Message support objects: the set run_lmx.ps1's Exec suite links (the
    # shorter list in run_l2trans.ps1 predates the liveness/history/stale
    # modules that lmx_message_exec.c now includes).
    $names = @('lmx_msg_blocks', 'lmx_owned_ranges', 'lmx_msg_storage', 'lmx_msg_path_storage', 'lmx_msg_slots', 'lmx_msg_mail_chain', 'lmx_msg_sched_ready', 'lmx_msg_visit', 'lmx_msg_liveness', 'lmx_msg_history_owned', 'lmx_msg_roots_stale', 'lmx_branch_owned', 'lmx_value_owned', 'lmx_chars_owned', 'lmx_array_owned', 'lmx_array_ref_owned', 'lmx_graph_copy_owned', 'lmx_merge_owned', 'lmx_message_graph_copy')
    $sources = @('l2src/lmx_message_host.c', 'l2src/lmx_message_exec.c')
    foreach ($name in $names) {
        Stage "header_$name" (Invoke-Native ((Q $l1trans) + " l2src/$name.h.lm1 " + (Q (Join-Path $hdrs "l2src/$name.lm1.h"))) (Join-Path $run "header_$name.log")) (Join-Path $run "header_$name.log")
        $source = Join-Path $supportDir "$name.c"
        Stage "module_$name" (Invoke-Native ((Q $l1trans) + " l2src/$name.lm1 " + (Q $source)) (Join-Path $run "module_$name.log")) (Join-Path $run "module_$name.log")
        $sources += $source
    }
    $messageSource = Join-Path $supportDir 'lmx_message.c'
    Stage 'module_lmx_message' (Invoke-Native ((Q $l1trans) + ' l2src/lmx_message.lm1 ' + (Q $messageSource)) (Join-Path $run 'module_lmx_message.log')) (Join-Path $run 'module_lmx_message.log')
    $sources += $messageSource
    $objs = @()
    $supportWarnings = 0
    foreach ($source in $sources) {
        $obj = Join-Path $supportDir ([IO.Path]::GetFileNameWithoutExtension($source) + '.o')
        $glog = Join-Path $run ([IO.Path]::GetFileNameWithoutExtension($source) + '.gcc.log')
        Stage ('compile_' + [IO.Path]::GetFileNameWithoutExtension($source)) (Invoke-Native ("gcc $cflags -I " + (Q $hdrs) + ' -c ' + (Q $source) + ' -o ' + (Q $obj)) $glog) $glog
        $supportWarnings += @(Get-Content -LiteralPath $glog | Where-Object { $_ -match 'warning:' }).Count
        $objs += $obj
    }
    $ev.supportObjects = @($objs | ForEach-Object { [ordered]@{ path = $_; sha256 = (Get-FileHash -LiteralPath $_).Hash } })
    $ev.supportWarnings = $supportWarnings
    $objList = ($objs | ForEach-Object { Q $_ }) -join ' '

    # 1. Direct selftest.
    $selfSrc = 'l2src/tests/lmx_graph_abi_selftest.lm1'
    $selfC = Join-Path $out 'lmx_graph_abi_selftest.c'
    $selfExe = Join-Path $out 'lmx_graph_abi_selftest.exe'
    Stage 'selftest_translate' (Invoke-Native ((Q $l1trans) + ' ' + $selfSrc + ' ' + (Q $selfC)) (Join-Path $run 'selftest.translate.log')) (Join-Path $run 'selftest.translate.log')
    Stage 'selftest_compile' (Invoke-Native ("gcc $cflags -I " + (Q $hdrs) + ' ' + (Q $selfC) + ' ' + $objList + ' -o ' + (Q $selfExe)) (Join-Path $run 'selftest.gcc.log')) (Join-Path $run 'selftest.gcc.log')
    $selfLog = Join-Path $run 'selftest.stdout.txt'
    $selfExit = Invoke-Native (Q $selfExe) $selfLog
    $selfStdout = (Get-Content -LiteralPath $selfLog -Raw)
    $ev.selftest = [ordered]@{ exit = $selfExit; stdout = $selfStdout.Trim(); cSHA256 = (Get-FileHash -LiteralPath $selfC).Hash; gccWarnings = @(Get-Content (Join-Path $run 'selftest.gcc.log') | Where-Object { $_ -match 'warning:' }).Count }
    Stage 'selftest_run' $selfExit $selfLog
    if ($selfStdout -notmatch 'graph abi selftest: \d+ checks, 0 failures') { throw 'selftest did not report zero failures' }

    # 1b. Used-graph copy selftest. The graph-lane objects it links are rebuilt
    # with the test allocator substituted per object (a different
    # instrumentation configuration, never shared with production objects), so
    # every allocation and release of the copier and the owned constructors is
    # counted and can be failed deterministically.
    $instrDir = Join-Path $supportDir 'instrumented'
    New-Item -ItemType Directory -Force -Path $instrDir | Out-Null
    $instrNames = @('lmx_graph_copy_owned', 'lmx_merge_owned', 'lmx_branch_owned', 'lmx_value_owned', 'lmx_chars_owned', 'lmx_array_owned', 'lmx_array_ref_owned', 'lmx_msg_blocks')
    $plainNames = @('lmx_owned_ranges', 'lmx_msg_storage')
    $copyObjs = @()
    foreach ($name in $instrNames) {
        $obj = Join-Path $instrDir "$name.o"
        $glog = Join-Path $run "instrumented_$name.gcc.log"
        Stage "instrumented_$name" (Invoke-Native ("gcc $cflags -Dmalloc=lmx_test_malloc -Dfree=lmx_test_free -I " + (Q $hdrs) + ' -c ' + (Q (Join-Path $supportDir "$name.c")) + ' -o ' + (Q $obj)) $glog) $glog
        $copyObjs += $obj
    }
    foreach ($name in $plainNames) { $copyObjs += (Join-Path $supportDir "$name.o") }
    $copySrc = 'l2src/tests/lmx_graph_copy_selftest.lm1'
    $copyC = Join-Path $out 'lmx_graph_copy_selftest.c'
    $copyExe = Join-Path $out 'lmx_graph_copy_selftest.exe'
    Stage 'copy_selftest_translate' (Invoke-Native ((Q $l1trans) + ' ' + $copySrc + ' ' + (Q $copyC)) (Join-Path $run 'copy_selftest.translate.log')) (Join-Path $run 'copy_selftest.translate.log')
    $copyObjList = ($copyObjs | ForEach-Object { Q $_ }) -join ' '
    Stage 'copy_selftest_compile' (Invoke-Native ("gcc $cflags -I " + (Q $hdrs) + ' ' + (Q $copyC) + ' ' + $copyObjList + ' -o ' + (Q $copyExe)) (Join-Path $run 'copy_selftest.gcc.log')) (Join-Path $run 'copy_selftest.gcc.log')
    $copyLog = Join-Path $run 'copy_selftest.stdout.txt'
    $copyExit = Invoke-Native (Q $copyExe) $copyLog
    $copyStdout = (Get-Content -LiteralPath $copyLog -Raw)
    $ev.copySelftest = [ordered]@{ exit = $copyExit; stdout = $copyStdout.Trim(); cSHA256 = (Get-FileHash -LiteralPath $copyC).Hash; sourceSHA256 = (Get-FileHash -LiteralPath $copySrc).Hash; gccWarnings = @(Get-Content (Join-Path $run 'copy_selftest.gcc.log') | Where-Object { $_ -match 'warning:' }).Count }
    Stage 'copy_selftest_run' $copyExit $copyLog
    if ($copyStdout -notmatch 'graph copy selftest: \d+ checks, 0 failures') { throw 'copy selftest did not report zero failures' }

    # 1c. Runtime merge over the same copier, same instrumented objects.
    $mergeSrc = 'l2src/tests/lmx_merge_selftest.lm1'
    $mergeC = Join-Path $out 'lmx_merge_selftest.c'
    $mergeExe = Join-Path $out 'lmx_merge_selftest.exe'
    Stage 'merge_selftest_translate' (Invoke-Native ((Q $l1trans) + ' ' + $mergeSrc + ' ' + (Q $mergeC)) (Join-Path $run 'merge_selftest.translate.log')) (Join-Path $run 'merge_selftest.translate.log')
    Stage 'merge_selftest_compile' (Invoke-Native ("gcc $cflags -I " + (Q $hdrs) + ' ' + (Q $mergeC) + ' ' + $copyObjList + ' -o ' + (Q $mergeExe)) (Join-Path $run 'merge_selftest.gcc.log')) (Join-Path $run 'merge_selftest.gcc.log')
    $mergeLog = Join-Path $run 'merge_selftest.stdout.txt'
    $mergeExit = Invoke-Native (Q $mergeExe) $mergeLog
    $mergeStdout = (Get-Content -LiteralPath $mergeLog -Raw)
    $ev.mergeSelftest = [ordered]@{ exit = $mergeExit; stdout = $mergeStdout.Trim(); cSHA256 = (Get-FileHash -LiteralPath $mergeC).Hash; sourceSHA256 = (Get-FileHash -LiteralPath $mergeSrc).Hash }
    Stage 'merge_selftest_run' $mergeExit $mergeLog
    if ($mergeStdout -notmatch 'merge selftest: \d+ checks, 0 failures') { throw 'merge selftest did not report zero failures' }

    # 1c. Message publication seam: graph remains null until the complete copy
    # and its storage are admitted to the destination owner.
    $msgCopySrc = 'l2src/tests/lmx_message_graph_copy_selftest.lm1'
    $msgCopyC = Join-Path $out 'lmx_message_graph_copy_selftest.c'
    $msgCopyExe = Join-Path $out 'lmx_message_graph_copy_selftest.exe'
    Stage 'message_copy_selftest_translate' (Invoke-Native ((Q $l1trans) + ' ' + $msgCopySrc + ' ' + (Q $msgCopyC)) (Join-Path $run 'message_copy_selftest.translate.log')) (Join-Path $run 'message_copy_selftest.translate.log')
    Stage 'message_copy_selftest_compile' (Invoke-Native ("gcc $cflags -I " + (Q $hdrs) + ' ' + (Q $msgCopyC) + ' ' + $objList + ' -o ' + (Q $msgCopyExe)) (Join-Path $run 'message_copy_selftest.gcc.log')) (Join-Path $run 'message_copy_selftest.gcc.log')
    $msgCopyLog = Join-Path $run 'message_copy_selftest.stdout.txt'
    $msgCopyExit = Invoke-Native (Q $msgCopyExe) $msgCopyLog
    $msgCopyStdout = (Get-Content -LiteralPath $msgCopyLog -Raw)
    $ev.messageCopySelftest = [ordered]@{ exit = $msgCopyExit; stdout = $msgCopyStdout.Trim(); cSHA256 = (Get-FileHash -LiteralPath $msgCopyC).Hash; sourceSHA256 = (Get-FileHash -LiteralPath $msgCopySrc).Hash; gccWarnings = @(Get-Content (Join-Path $run 'message_copy_selftest.gcc.log') | Where-Object { $_ -match 'warning:' }).Count }
    Stage 'message_copy_selftest_run' $msgCopyExit $msgCopyLog
    if ($msgCopyStdout -notmatch 'message graph copy selftest: \d+ checks, 0 failures') { throw 'message graph copy selftest did not report zero failures' }

    if (-not $SelftestOnly) {
        # 2. Historical positive fixtures with the expectations run_l2trans.ps1 pins.
        $runnerText = Get-Content -LiteralPath (Join-Path $PSScriptRoot 'run_l2trans.ps1') -Raw
        $cases = @([regex]::Matches($runnerText, '(?m)^Invoke-(Positive|Leaf|LeafOut) "([^"]+)" "([^"]+)" (\d+)(?: "([^"]*)")?(?: "((?:[^"`]|`n)*)")?') | ForEach-Object {
            $kind = $_.Groups[1].Value
            $stdout = $null
            if ($kind -eq 'LeafOut') { $stdout = $_.Groups[6].Value.Replace('`n', "`n") }
            [pscustomobject]@{ kind = $kind; source = $_.Groups[2].Value.Replace('\', '/'); stem = $_.Groups[3].Value; expect = [int]$_.Groups[4].Value; stdout = $stdout }
        })
        if ($cases.Count -lt 50) { throw "unexpectedly few historical cases parsed: $($cases.Count)" }
        $ev.historicalCaseCount = $cases.Count
        # Focused fixtures for the independent: const: immutable branch stage.
        # They are not historical, so they are appended rather than pinned in
        # run_l2trans.ps1, which is not this lane's file.
        $cases += [pscustomobject]@{ kind = 'Positive'; source = 'l2src/tests/unit_eternal_branch.lm2'; stem = 'unit_eternal_branch'; expect = 0; stdout = $null }
        $cases += [pscustomobject]@{ kind = 'Positive'; source = 'l2src/tests/unit_eternal_two.lm2'; stem = 'unit_eternal_two'; expect = 0; stdout = $null }
        # 70 declarations: proof that the branch table grows rather than being
        # capped. Any reintroduced fixed limit below 70 fails here.
        $cases += [pscustomobject]@{ kind = 'Positive'; source = 'l2src/tests/unit_eternal_many.lm2'; stem = 'unit_eternal_many'; expect = 0; stdout = $null }
        # Reverse-declared 65-call chain: hidden lexical type information moves
        # only one edge per fixed-point pass. This rejects the retired guard<32
        # implementation without imposing a new depth cap.
        $cases += [pscustomobject]@{ kind = 'Positive'; source = 'l2src/tests/unit_dyn_chain65.lm2'; stem = 'unit_dyn_chain65'; expect = 0; stdout = $null }
        # 19.17 merge lowered at its execution site, with a second merge in an
        # untaken branch so the site is proved to be a site and not a preamble.
        $cases += [pscustomobject]@{ kind = 'Positive'; source = 'l2src/tests/unit_merge_site.lm2'; stem = 'unit_merge_site'; expect = 0; stdout = $null }
        # The current Message is a compiler-selected dynamic input. Entry passes
        # it to m, and m performs the merge in that Message arena.
        $cases += [pscustomobject]@{ kind = 'Positive'; source = 'l2src/tests/unit_merge_in_method.lm2'; stem = 'unit_merge_in_method'; expect = 0; stdout = $null }
        # The 19.17 example: ordinary named Structures declared at unit level,
        # merged with a result body, plus one declared and never used.
        $cases += [pscustomobject]@{ kind = 'Positive'; source = 'l2src/tests/unit_named_struct.lm2'; stem = 'unit_named_struct'; expect = 0; stdout = $null }
        # char fields and nested/reference Structure fields: the shapes the
        # flat-field emitter could not produce. Shape holds a char, an inline
        # nested Structure two levels deep and a reference to an earlier
        # declaration, and is merged so all of it is deep-copied.
        $cases += [pscustomobject]@{ kind = 'Positive'; source = 'l2src/tests/unit_named_nested.lm2'; stem = 'unit_named_nested'; expect = 0; stdout = $null }
        # Aliases and cycles: a forward reference, a self reference, a mutual
        # cycle and a reference to a nested declaration, merged so the common
        # copy map has to carry all of them.
        $cases += [pscustomobject]@{ kind = 'Positive'; source = 'l2src/tests/unit_named_alias.lm2'; stem = 'unit_named_alias'; expect = 0; stdout = $null }
        # Source field paths and post-merge independence: A keeps its own
        # values while the same paths on the merged result are rewritten.
        $cases += [pscustomobject]@{ kind = 'Positive'; source = 'l2src/tests/unit_field_path.lm2'; stem = 'unit_field_path'; expect = 0; stdout = $null }
        # The same paths inside METHODS, where the program unit is the
        # callable's lexical parent rather than a name in scope.
        $cases += [pscustomobject]@{ kind = 'Positive'; source = 'l2src/tests/unit_field_path_method.lm2'; stem = 'unit_field_path_method'; expect = 0; stdout = $null }
        # A callable Structure selected through a path, including one rooted
        # at a merge result: the selected occurrence is the reserved argument.
        $cases += [pscustomobject]@{ kind = 'Positive'; source = 'l2src/tests/unit_merged_callable.lm2'; stem = 'unit_merged_callable'; expect = 0; stdout = $null }
        # A qualified branch with a full immutable graph body: size_t and char
        # leaves, a nested Structure, a self reference, a second branch, and an
        # ordinary Structure holding a reference to the first.
        $cases += [pscustomobject]@{ kind = 'Positive'; source = 'l2src/tests/unit_eternal_shape.lm2'; stem = 'unit_eternal_shape'; expect = 0; stdout = $null }
        # Array fields in an ordinary declaration and inside a qualified
        # branch, merged: the ordinary ones are copied, the admitted one is not.
        $cases += [pscustomobject]@{ kind = 'Positive'; source = 'l2src/tests/unit_array_field.lm2'; stem = 'unit_array_field'; expect = 0; stdout = $null }
        # Empty is a typed Array descriptor with len=0/data=0, not null and not
        # a special Structure representation.
        $cases += [pscustomobject]@{ kind = 'Positive'; source = 'l2src/tests/unit_array_empty.lm2'; stem = 'unit_array_empty'; expect = 0; stdout = $null }
        # A reference from one branch into a NESTED entry of another: I claimed
        # this worked when reporting the branch slice and had not written the
        # fixture, so here it is.
        $cases += [pscustomobject]@{ kind = 'Positive'; source = 'l2src/tests/unit_eternal_xref.lm2'; stem = 'unit_eternal_xref'; expect = 0; stdout = $null }
        # A merge copies a callable occurrence. Its method both owns a mutable
        # field and performs a merge, so all three selected-callable calls prove
        # the Message/result/throw ABI without conflating A.M and R.M storage.
        $cases += [pscustomobject]@{ kind = 'Positive'; source = 'l2src/tests/unit_throwing_callable.lm2'; stem = 'unit_throwing_callable'; expect = 0; stdout = $null }
        # An explicit argument stays activation-local until an executed
        # same-name bind publishes that same variable as an own field.
        $cases += [pscustomobject]@{ kind = 'Positive'; source = 'l2src/tests/unit_arg_own_bind.lm2'; stem = 'unit_arg_own_bind'; expect = 0; stdout = $null }
        # Recursion keeps one published callable occurrence while every C
        # activation owns its cache and dirty flag.
        $cases += [pscustomobject]@{ kind = 'Positive'; source = 'l2src/tests/unit_recursion.lm2'; stem = 'unit_recursion'; expect = 0; stdout = $null }
        foreach ($case in $cases) {
            $rec = [ordered]@{ stem = $case.stem; kind = $case.kind; source = $case.source; expectExit = $case.expect; status = 'RUNNING' }
            try {
                if (-not (Test-Path -LiteralPath $case.source)) { throw "missing fixture $($case.source)" }
                $rec.sourceSHA256 = (Get-FileHash -LiteralPath $case.source).Hash
                $lm1 = Join-Path $out ($case.stem + '.lm1')
                $cpath = Join-Path $out ($case.stem + '.c')
                $exe = Join-Path $out ($case.stem + '.exe')
                $code = Invoke-Native ((Q $l2exe) + ' ' + (Q $case.source) + ' ' + (Q $lm1)) (Join-Path $out ($case.stem + '.l2trans.log'))
                if ($code -ne 0) { throw "l2trans exit $code" }
                $text = [IO.File]::ReadAllText((Resolve-Path -LiteralPath $lm1).ProviderPath)
                if ($text -match $retired) { throw 'generated L1 still spells the retired inline-child layout' }
                if ($text -match 'fn: l2_program_entry' -and $text -notmatch 'lmx_branch_slot_known\(' -and $text -notmatch 'lmx_branch_store_known\(') { throw 'graph unit L1 lacks the slot API' }
                # Every callable occurrence is an ordinary Structure whose slot 0
                # holds the shared METHOD descriptor, and a call passes that
                # Structure, never the enclosing container. A flat unit lowering
                # would show neither (SPEC 21.8, model 40).
                if ($text -match 'fn: l2_program_entry') {
                    if ($case.stem -eq 'unit_dyn_chain65' -and $text -notmatch 'fn: l2_m64 \(@: Lmx node; char: l2_p64_0\) int') { throw 'hidden char did not reach method 64 across the long call chain' }
                    $structs = [regex]::Matches($text, 'leaf: lmx_struct_new_owned\(unit,').Count
                    $atZero = [regex]::Matches($text, 'lmx_branch_store_known\(leaf, 0U, \(cast: \(@: void\) rec\)\)').Count
                    $rec.callableStructures = $structs
                    if ($structs -eq 0 -or $structs -lt $atZero) { throw "callable Structures $structs but $atZero descriptors at slot 0" }
                    # No call may hand over the enclosing container. The flat
                    # lowering spelled exactly this and nothing else does.
                    $flat = [regex]::Matches($text, 'l2_m\d+\((unit|node)[,)]').Count
                    if ($flat -ne 0) { throw "$flat call(s) still pass the enclosing container as the reserved argument" }
                    # The first Message holds the translation-known METHOD
                    # descriptor array, built once and never appended (9.1.4).
                    if ($text -notmatch 'lmx_array_ref_new_positive_owned\(c\.LMX_TYPE_ARRAY_OF_METHOD,') { throw 'generated entry lacks the root METHOD descriptor array' }
                    $rec.methodArrayRefs = [regex]::Matches($text, 'l2_method_refs\[\d+U\]: rec').Count
                    if ($rec.methodArrayRefs -ne $atZero) { throw "descriptor array filled $($rec.methodArrayRefs) times but $atZero records exist" }
                    # The retention array is separate from the descriptor array,
                    # holds only qualified branch roots, and each root is built
                    # with node = 0, which is what independent means (9.1.4).
                    $roots = [regex]::Matches($text, 'l2_nsp\[\d+\]: lmx_node_new_owned\(').Count
                    $stored = [regex]::Matches($text, 'l2_branch_refs\[\d+U\]: l2_nsp\[\d+\]').Count
                    $rec.eternalRoots = $roots
                    if ($roots -ne $stored) { throw "$roots eternal roots but $stored retention entries" }
                    if ($roots -gt 0) {
                        if ($text -notmatch 'lmx_array_ref_new_positive_owned\(c\.LMX_TYPE_ARRAY_OF_LMX,') { throw 'eternal roots without the ARRAY_OF_LMX retention array' }
                        # Twice per root: at construction, and again after the
                        # declaration-site reference is stored, since storing a
                        # reference must not reparent an independent branch.
                        if ([regex]::Matches($text, 'if: l2_nsp\[\d+\]\\node != 0').Count -lt (2 * $roots)) { throw 'an eternal root is not checked for a zero lexical root before and after the declaration-site store' }
                        if ([regex]::Matches($text, 'l2_ebr\d+: l2_nsp\[\d+\]').Count -ne $roots) { throw 'an eternal branch has no declaration-site binding in the unit graph' }
                        if ($text -match 'l2_branch_refs\[\d+U\]: rec') { throw 'a METHOD descriptor was stored in the retention array' }
                        if ($case.stem -eq 'unit_eternal_many' -and $roots -ne 70) { throw "growth fixture produced $roots roots, not 70" }
                        if ($text -match 'l2_nsp\[\d+\]: lmx_node_new_owned\(@ (?!process_message\\blocks)') { throw 'a qualified branch is not allocated from the first Message arena' }
                        if ($text -notmatch 'lmx_owned_ranges_find\(process_message\\ranges,') { throw 'no check that a qualified branch still classifies in the owner ranges' }
                        if ([regex]::Matches($text, 'c\.lmx_msg_bootstrap_eternal_admit\(process_message,').Count -lt (2 * $roots)) { throw 'each qualified root and every owned range inside it must be admitted through the Message classifier' }
                        if ($text -notmatch 'c\.lmx_msg_eternal_ranges\(process_message\)') { throw 'the eternal set is not read from the Message' }
                        if ($text -match '(?m)^@: LmxOwnedRange ' -or $text -match 'l2_eternal_ranges_get') { throw 'a file-scope eternal classifier reappeared' }
                    }
                    if ($case.stem -eq 'unit_merge_site') {
                        # Three merges in the settled result-bearing form, one of
                        # them in a branch that is never taken, so this proves a
                        # SITE and not a preamble.
                        $merges = [regex]::Matches($text, 'c\.lmx_merge_owned\(l2_mops,').Count
                        if ($merges -ne 3) { throw "merge fixture emitted $merges merge calls, not 3" }
                        # Every result is bound under its source name, and one
                        # merge takes a previously bound ORDINARY result as its
                        # operand, which is what exercises real copying.
                        if ([regex]::Matches($text, 'lmx_branch_store_known\(unit, \d+U, \(cast: \(@: void\) l2_mresult\)\)').Count -ne $merges) { throw 'a merge result is not bound under its name' }
                        if ($text -notmatch 'l2_mops\[0U\]: lmx_branch_struct_known\(unit, \d+U\)') { throw 'no merge takes an ordinary bound result as an operand' }
                        # Failure lowering is NOT the declared throw ABI and must
                        # not pretend to be: no global channel, no null result.
                        if ($text -match 'l2_throw') { throw 'the withdrawn throw local reappeared' }
                        if ($text -notmatch 'l2_mstatus != 0 \|\| l2_mresult = 0') { throw 'a merge failure is not stopped before publication' }
                        # One merge carries a result body, built as a Structure
                        # and handed to the helper, and the others pass none.
                        if ($text -notmatch 'lmx_branch_open_owned\(l2_mbody,') { throw 'the merge result body is not built' }
                        if ([regex]::Matches($text, 'l2_mbody: 0').Count -lt 2) { throw 'a merge without a body does not clear the body operand' }
                        # The generated program checks the SHAPE it was promised,
                        # so a merge that succeeds but produces the wrong width
                        # fails the fixture instead of passing silently.
                        $shapes = [regex]::Matches($text, 'if: l2_mresult\\len != (\d+)')
                        if ($shapes.Count -ne $merges) { throw "$($shapes.Count) result shape checks for $merges merges" }
                        $widths = @($shapes | ForEach-Object { [int]$_.Groups[1].Value })
                        if (($widths -join ',') -ne '3,4,1') { throw "predicted widths $($widths -join ',') are not 3,4,1" }
                        # and the CONTENT: a shape check alone would pass a copy
                        # that moved the right number of wrong values.
                        $vals = @([regex]::Matches($text, 'lmx_size_value_known\(l2_mxp\[0\]\) != (\d+)U') | ForEach-Object { $_.Groups[1].Value })
                        if (($vals -join ',') -ne '3,5,7,3,7,3,11,3,3,3') { throw "checked values $($vals -join ',') are not 3,5,7,3,7,3,11,3,3,3" }
                        if ($text -match 'lmx_size_store_known\(l2_mxp\[0\], 0U\)') { throw 'a merge body field stores zero: the literal suffix was dropped' }
                    }
                    if ($case.stem -eq 'unit_merge_in_method') {
                        if ($text -notmatch 'fn: l2_m0 \(@: Lmx node; @: LmxMsg process_message; @: int l2_out_result; @@: Lmx l2_out_throw\) int') { throw 'merge method lacks Message/result/throw ABI' }
                        if ($text -notmatch 'fn: l2_m2 \(@: Lmx node; @: LmxMsg process_message; @: int l2_out_result; @@: Lmx l2_out_throw\) int') { throw 'caller did not acquire transitive Message/result/throw ABI' }
                        if ($text -notmatch 'fn: l2_m3 \(@: Lmx node; @: LmxMsg process_message; @@: Lmx l2_out_throw\) int') { throw 'throwing sub has an incorrect typed-output ABI' }
                        if ($text -notmatch 'l2_mstatus: c\.lmx_merge_owned\(l2_mops, 1U, l2_mbody, node\\node, process_message\\ranges,') { throw 'method merge does not use its dynamic Message and lexical unit' }
                        if ($text -notmatch 'l2_ts\d+: l2_m0\(lmx_branch_struct_known\(node\\node, \d+U\), process_message, @ l2_t\d+, @ l2_te\d+\)') { throw 'throwing caller does not forward Message and typed outputs to the merge method' }
                        if ($text -notmatch 'l2_ts\d+: l2_m2\(lmx_branch_struct_known\(unit, \d+U\), process_message, @ l2_t\d+, @ l2_te\d+\)') { throw 'entry does not pass Message and typed outputs to the transitive caller' }
                        if ($text -notmatch 'l2_ts\d+: l2_m3\(lmx_branch_struct_known\(node\\node, \d+U\), process_message, @ l2_te\d+\)') { throw 'throwing sub call does not forward Message and throw output' }
                        if ($text -notmatch 'l2_out_throw\[0\]: node') { throw 'method merge failure does not publish its failure graph' }
                        if ([regex]::Matches($text, '@@: void l2_myp 0').Count -lt 3) { throw 'merge-capable methods lack the pointer comparison temporary' }
                        if ($text -match '(?m)^    l2_myp: lmx_branch_slot_known\(l2_ebr\d+,') { throw 'a method refers to an entry-local eternal alias instead of its lexical unit' }
                        $methodSigs = @([regex]::Matches($text, 'rec\\sig: (\d+)U') | ForEach-Object { $_.Groups[1].Value })
                        if ($methodSigs.Count -ne 4 -or $methodSigs[0] -ne $methodSigs[2] -or $methodSigs[0] -eq $methodSigs[1] -or $methodSigs[0] -eq $methodSigs[3]) { throw "METHOD.sig does not encode the closed throw/result contract: $($methodSigs -join ',')" }
                    }
                    if ($case.stem -eq 'unit_throwing_callable') {
                        if ($text -notmatch 'fn: l2_m0 \(@: Lmx node; @: LmxMsg process_message; @: int l2_out_result; @@: Lmx l2_out_throw\) int') { throw 'throwing callable lacks the Message/result/throw ABI' }
                        if ($text -match 'lmx_msg_from_node|process_message: node') { throw 'the Message was inferred from the lexical node instead of passed as a dynamic input' }
                        if ($text -notmatch 'l2_mstatus: c\.lmx_merge_owned\(l2_mops, 1U, l2_mbody, node\\node, process_message\\ranges,') { throw 'callable merge does not use its lexical unit and dynamic Message' }
                        $typedCalls = [regex]::Matches($text, 'l2_ts\d+: l2_m0\(l2_pst, process_message, @ l2_t\d+, @ l2_te\d+\)').Count
                        if ($typedCalls -ne 3) { throw "$typedCalls calls use the selected callable and typed outputs, not 3" }
                        if ([regex]::Matches($text, 'if: l2_ts\d+ != 0\s+return: 70').Count -ne 3) { throw 'a throwing call does not branch before reading its normal result' }
                        foreach ($name in @('a1','r1','a2')) {
                            if ($text -notmatch "if: l2_ts\d+ != 0\s+return: 70\s+${name}: l2_t\d+") { throw "$name reads its normal result before the status branch" }
                        }
                        $selectedRoots = @([regex]::Matches($text, 'l2_pst: lmx_branch_struct_known\(unit, (\d+)U\)') | ForEach-Object { $_.Groups[1].Value } | Sort-Object -Unique)
                        if ($selectedRoots.Count -ne 2) { throw "calls reached $($selectedRoots.Count) enclosing roots, not A and R" }
                        $mk = [regex]::Matches($text, 'lmx_method_new_owned\(').Count
                        if ($mk -ne $rec.methodArrayRefs) { throw "$mk METHOD descriptors constructed for $($rec.methodArrayRefs) known methods" }
                        if ($text -match 'strcmp|lmx_name|l2_field_names') { throw 'the copied callable was resolved by name at run time' }
                    }
                    if ($case.stem -eq 'unit_arg_own_bind') {
                        if ($text -notmatch 'l2_p0_0: l2_p0_0 \+ 10U') { throw 'the bind does not write the argument variable itself' }
                        if ($text -notmatch 'lmx_size_store_known\(l2_q\d+_from\[0\], l2_p0_0\)') { throw 'the checkpoint does not publish the argument variable' }
                        if ($text -match 'l2_p\d+_\d+: lmx_size_value_known') { throw 'an argument is preloaded from its graph slot' }
                        if ($text -match 'l2_p\d+_\d+: lmx_int_value_known|l2_p\d+_\d+: \(cast: \(char\) lmx_char_value_known') { throw 'an argument is preloaded from its graph slot' }
                        if ($text -match '(?m)^\s+int: l2_q\d+_dirty 1') { throw 'an argument is marked dirty before its bind executes' }
                        if ($text -notmatch '(?m)^\s+if: l2_p\d+_1 != 0U\r?\n(\s+.*\r?\n)*?\s+l2_q\d+_dirty: 1') { throw 'a conditional bind does not set dirty inside its own branch' }
                        $pure = [regex]::Match($text, '(?ms)^fn: l2_m1 \(.*?end: l2_m1')
                        if (-not $pure.Success) { throw 'the unbound method was not emitted' }
                        if ($pure.Value -match 'l2_q\d+_dirty|l2_q\d+_from') { throw 'return use alone created an own field' }
                    }
                    if ($case.stem -eq 'unit_recursion') {
                        if ($text -notmatch 'l2_m0\(lmx_branch_struct_known\(node\\node, \d+U\), ') { throw 'direct recursion does not pass the selected callable' }
                        if ($text -notmatch 'l2_m1\(l2_pst, ') { throw 'path recursion does not pass the selected callable' }
                        if ($text -match '(?m)^@: \w+ l2_q\d+|(?m)^int: l2_q\d+_dirty') { throw 'an own cache or dirty flag escaped its activation' }
                        if ([regex]::Matches($text, '(?m)^    int: l2_q\d+_dirty 0').Count -lt 4) { throw 'recursive methods lack activation-local dirty flags' }
                        foreach ($m in [regex]::Matches($text, 'l2_t\d+: l2_m\d+\([^\r\n]*\r?\n(?<next>[^\r\n]*)')) {
                            if ($m.Groups['next'].Value -match 'l2_q\d+: lmx_\w+_value_known') { throw 'an own cache is reloaded after a recursive call' }
                        }
                        if ($text -notmatch 'l2_q\d+_dirty: 0\r?\n(\s+[^\r\n]*\r?\n)*?\s+l2_t\d+: l2_m\d+\(') { throw 'the checkpoint does not precede recursion' }
                        if ($text -match 'strcmp|lmx_name|l2_field_names') { throw 'a recursive callable was resolved by name at run time' }
                        if ($text -notmatch 'l2_ts\d+: l2_m\d+\(lmx_branch_struct_known\(node\\node, \d+U\), l2_q\d+, process_message, @ l2_t\d+, @ l2_te\d+\)') { throw 'recursive throw call does not forward the full ABI' }
                        if ($text -notmatch 'l2_ts\d+: l2_m\d+\([^\r\n]*\r?\n\s+if: l2_ts\d+ != 0') { throw 'recursive status is not checked before the result' }
                    }
                    if ($case.stem -eq 'unit_throw_transitive') {
                        if ([regex]::Matches($text, '(?m)^fn: l2_m\d+ \(@: Lmx node; @: LmxMsg process_message; @: int l2_out_result; @@: Lmx l2_out_throw\) int').Count -ne 2) { throw 'the throw dependency is not transitive' }
                        if ($text -notmatch 'l2_out_throw\[0\]: l2_th\d+\r?\n\s+return: 1') { throw 'a caller does not forward the failure graph' }
                        if ($text -notmatch '(?m)^\s+if: l2_st\d+ != 0\r?\n\s+return: 92\r?$') { throw 'the entry does not stop on a failed call' }
                        if ($text -match 'l2_out_throw\[0\]: l2_th(\d+)\r?\n\s+\w+: l2_t\1') { throw 'a normal result is read on the failure path' }
                    }
                    if ($case.stem -eq 'unit_eternal_xref') {
                        # F's field holds E's NESTED entry, and storing it does
                        # not reparent that entry: its node is still E.
                        if ($text -notmatch 'lmx_branch_store_known\(l2_nsp\[2\], 0U, \(cast: \(@: void\) l2_nsp\[1\]\)\)') { throw 'the cross-branch nested reference did not resolve' }
                        if ($text -notmatch 'if: l2_nsp\[1\]\\node != l2_nsp\[0\]') { throw 'a referenced nested entry is not checked against reparenting' }
                        # Both branch roots still have no lexical parent.
                        if ([regex]::Matches($text, 'l2_nsp\[\d+\]: lmx_node_new_owned\(').Count -ne 2) { throw 'the two qualified roots are not built with a zero lexical root' }
                        # A path may read through a nested entry of a branch.
                        if ($text -notmatch 'l2_pst: lmx_branch_struct_known\(l2_pst, 0U\)') { throw 'the path does not step into the nested entry' }
                    }
                    if ($case.stem -eq 'unit_array_field') {
                        # The emitted constructor needs its emitted predef under
                        # the SAME condition: without it the record pointer is
                        # truncated to int and lands outside every owned range.
                        if ($text -notmatch 'predef: "l2src/lmx_array_owned\.h\.lm1"') { throw 'an array field is built without the array prototype in scope' }
                        # One record per declared array, with the declared
                        # element type and count.
                        if ($text -notmatch 'lmx_array_new_owned\(c\.LMX_TYPE_ARRAY_OF_INT, 4U,') { throw 'the ordinary int array is not built with its declared count' }
                        if ($text -notmatch 'lmx_array_new_owned\(c\.LMX_TYPE_ARRAY_OF_CHAR, 2U,') { throw 'the ordinary char array is not built with its declared count' }
                        if ($text -notmatch 'lmx_array_new_owned\(c\.LMX_TYPE_ARRAY_OF_INT, 3U,') { throw 'the qualified array is not built with its declared count' }
                        # Inside a branch BOTH ranges are admitted: the record
                        # and the backing it addresses.
                        if ($text -notmatch 'c\.lmx_msg_bootstrap_eternal_admit\(process_message, \(cast: \(@: LmxArrayDesc\) slot\[0\]\)\\data\)') { throw 'an array backing inside a branch is not admitted' }
                        # merge copies an ordinary array -- new record, new
                        # backing, same length -- and keeps the admitted one.
                        if ([regex]::Matches($text, '(?m)\s+return: 91').Count -lt 6) { throw 'the copied arrays are not checked for record, length and backing' }
                        if ([regex]::Matches($text, '(?m)\s+return: 80').Count -lt 2) { throw 'the admitted branch children are not checked for address identity' }
                        if ($text -notmatch 'if: l2_mresult\\len != 5') { throw 'the merged width is not E two plus Holder three' }
                    }
                    if ($case.stem -eq 'unit_array_empty') {
                        $emptyInt = [regex]::Matches($text, 'lmx_array_new_owned\(c\.LMX_TYPE_ARRAY_OF_INT, 0U,').Count
                        $emptyChar = [regex]::Matches($text, 'lmx_array_new_owned\(c\.LMX_TYPE_ARRAY_OF_CHAR, 0U,').Count
                        if ($emptyInt -ne 3 -or $emptyChar -ne 2) { throw "empty constructors int=$emptyInt char=$emptyChar, expected 3/2" }
                        if ($text -match 'lmx_array_new_positive_owned\([^\r\n]*, 0U,') { throw 'empty source array still uses the positive-only constructor' }
                        if ($text -notmatch 'if: \(cast: \(@: LmxArrayDesc\) slot\[0\]\)\\data != 0 && c\.lmx_msg_bootstrap_eternal_admit') { throw 'empty eternal array does not guard absent backing admission' }
                    }
                    if ($case.stem -eq 'unit_eternal_shape') {
                        # Two branches, each a root with node = 0 built in the
                        # Message arena and bound under its reserved child.
                        if ([regex]::Matches($text, 'l2_nsp\[\d+\]: lmx_node_new_owned\(').Count -ne 2) { throw 'the two qualified roots are not built with a zero lexical root' }
                        # Nested Structures inside a branch keep ordinary
                        # internal node links: only the ROOT has none.
                        if ($text -notmatch 'l2_nsp\[1\]: lmx_struct_new_owned\(l2_nsp\[0\],') { throw 'a nested Structure inside a branch is not a child of its parent' }
                        if ($text -notmatch 'if: l2_nsp\[1\]\\node != l2_nsp\[0\]') { throw 'a nested Structure inside a branch does not check its internal node link' }
                        # char uses the pointer cell, not inline storage.
                        if ($text -notmatch 'slot\[0\]: lmx_char_cell_known\(process_chars, 101\)') { throw 'a char field inside a branch is not a pointer cell' }
                        # An admitted branch survives merge BY ADDRESS -- both as
                        # an operand and as a reference held by an ordinary
                        # Structure -- while an ordinary mutable cell is copied.
                        if ([regex]::Matches($text, '(?m)\s+return: 80').Count -lt 6) { throw 'the admitted branch addresses are not all checked through merge' }
                        if ($text -notmatch '(?m)\s+return: 90') { throw 'no check that an ordinary mutable cell is copied rather than shared' }
                        if ($text -notmatch 'l2_mxp\[0\] != \(cast: \(@: void\) lmx_branch_struct_known\(unit, \d+U\)\)') { throw 'a held reference to a branch is not checked for identity after merge' }
                        # Receiving one branch exposes nothing else: the width is
                        # E's five children plus Holder's two, and the second
                        # branch F contributes nothing.
                        if ($text -notmatch 'if: l2_mresult\\len != 7') { throw 'the merged width is not E five plus Holder two' }
                        # The retention array and ordinary storage stay out of
                        # the eternal classification.
                        if ($text -notmatch 'lmx_owned_ranges_find\(c\.lmx_msg_eternal_ranges\(process_message\), \(cast: \(@: void\) l2_branches\)\) != 0') { throw 'the retention array is not checked against eternal classification' }
                    }
                    if ($case.stem -eq 'unit_merged_callable') {
                        # The reserved argument is the SELECTED callable, not
                        # the unit child and not the enclosing Structure.
                        $sel = [regex]::Matches($text, '(?m)l2_m\d+\(l2_pst\)').Count
                        if ($sel -ne 5) { throw "$sel calls pass the selected callable, not 5" }
                        if ($text -match 'l2_m\d+\(l2_mresult\)') { throw 'a call passed the enclosing merge result as the reserved argument' }
                        # Two occurrences are reached: A's callable field and
                        # the copy inside R, through different roots.
                        $roots = @([regex]::Matches($text, 'l2_pst: lmx_branch_struct_known\(unit, (\d+)U\)') | ForEach-Object { $_.Groups[1].Value } | Sort-Object -Unique)
                        if ($roots.Count -ne 2) { throw "callable paths reached $($roots.Count) roots, not 2" }
                        # The copy is a DIFFERENT occurrence, its child 0 is the
                        # SAME descriptor terminal, and its own cell is its own.
                        if ($text -notmatch '(?m)\s+return: 77') { throw 'no check that the copied callable is a distinct occurrence' }
                        if ($text -notmatch '(?m)\s+return: 78') { throw 'no check that the METHOD descriptor is shared by address' }
                        if ($text -notmatch '(?m)\s+return: 79') { throw 'no check that the copied own cell is distinct' }
                        # No descriptor clone and no runtime name table.
                        # One shared descriptor per known method, built in the
                        # entry. The invariant is that nothing else allocates
                        # one, so the constructor count must equal the number
                        # of descriptor array fills and no more.
                        $mk = [regex]::Matches($text, 'lmx_method_new_owned\(').Count
                        if ($mk -ne $rec.methodArrayRefs) { throw "$mk METHOD descriptors constructed for $($rec.methodArrayRefs) known methods" }
                        if ($text -match 'strcmp|lmx_name|l2_field_names') { throw 'a callable was resolved by name at run time' }
                        # The callable field stores the method's own Structure
                        # and does not reparent it.
                        if ($text -notmatch 'lmx_branch_store_known\(l2_nsp\[\d+\], \d+U, \(cast: \(@: void\) lmx_branch_struct_known\(unit, \d+U\)\)\)') { throw 'the callable field does not store the method Structure' }
                    }
                    if ($case.stem -eq 'unit_field_path_method') {
                        # Inside a method the root is node\node, the callable's
                        # lexical parent. The entry still reaches it as 'unit'.
                        if ($text -match '(?m)^    l2_pst: lmx_branch_struct_known\(unit,') { throw 'a method reaches the unit by name' }
                        $inm = [regex]::Matches($text, 'l2_pst: lmx_branch_struct_known\(node\\node, (\d+)U\)')
                        if ($inm.Count -lt 8) { throw "$($inm.Count) in-method path roots, fewer than the fixture writes" }
                        # A declaration and a bound result are DIFFERENT unit
                        # children. They were both child 0 while the reserved
                        # bases were still computed after the method bodies.
                        $roots = @($inm | ForEach-Object { $_.Groups[1].Value } | Sort-Object -Unique)
                        if ($roots.Count -ne 2) { throw "in-method paths reached $($roots.Count) distinct roots, not 2" }
                        if ($roots -contains '0') { throw 'an in-method path root is child 0: the reserved bases were not known yet' }
                        # No second category: the Message is not derived from
                        # node and no hidden per-path state appears.
                        if ($text -match 'lmx_msg_from_node|l2_path_msg') { throw 'the Message was derived from node' }
                        if ($text -match 'strcmp|lmx_name|l2_field_names') { throw 'a field path was resolved by name at run time' }
                        # A loaded own field is published like any other: a
                        # cached value plus a dirty flag at the checkpoint.
                        if ($text -notmatch 'l2_q\d+_dirty: 1') { throw 'a field path load does not mark its own field dirty' }
                        # char assignment still REBINDS the pointer cell.
                        if ($text -notmatch 'l2_pxp\[0\]: lmx_char_rebind_known\(l2_pxp\[0\], 122\)') { throw 'a char field path in a method does not rebind its pointer cell' }
                    }
                    if ($case.stem -eq 'unit_field_path') {
                        # Every step of a path is a translator-known child
                        # index. No name survives into the generated program,
                        # and no new runtime name or descriptor table appears.
                        if ($text -match 'strcmp|lmx_name|l2_field_names') { throw 'a field path was resolved by name at run time' }
                        $walks = [regex]::Matches($text, 'l2_pst: lmx_branch_struct_known\(unit, \d+U\)').Count
                        if ($walks -ne 9) { throw "$walks path walks, not 9" }
                        # A nested path takes a second step from the Structure
                        # it just reached, not from the unit again.
                        if ([regex]::Matches($text, 'l2_pst: lmx_branch_struct_known\(l2_pst, \d+U\)').Count -ne 3) { throw 'a nested field path does not step through its parent' }
                        # size_t assignment writes the addressed cell IN PLACE.
                        if ([regex]::Matches($text, 'lmx_size_store_known\(l2_pxp\[0\], \d+U\)').Count -ne 2) { throw 'a size_t field path does not store in place' }
                        # char assignment REBINDS the pointer cell; the table
                        # bytes are never written.
                        if ($text -notmatch 'l2_pxp\[0\]: lmx_char_rebind_known\(l2_pxp\[0\], 122\)') { throw 'a char field path does not rebind its pointer cell' }
                        if ($text -match 'lmx_char_cell_known\(process_chars, 122\)') { throw 'a char assignment allocated a new cell instead of rebinding' }
                        # The result and the operand are reached through
                        # DIFFERENT unit children, which is what makes the
                        # independence checks meaningful.
                        $roots = @([regex]::Matches($text, 'l2_pst: lmx_branch_struct_known\(unit, (\d+)U\)') | ForEach-Object { $_.Groups[1].Value } | Sort-Object -Unique)
                        if ($roots.Count -ne 2) { throw "field paths reached $($roots.Count) distinct roots, not 2" }
                    }
                    if ($case.stem -eq 'unit_named_alias') {
                        # Every reference resolves, in all four directions, and
                        # none of them reparents its target.
                        if ($text -notmatch 'lmx_branch_store_known\(l2_nsp\[0\], 1U, \(cast: \(@: void\) l2_nsp\[1\]\)\)') { throw 'the forward reference did not resolve' }
                        if ($text -notmatch 'lmx_branch_store_known\(l2_nsp\[0\], 2U, \(cast: \(@: void\) l2_nsp\[0\]\)\)') { throw 'the self reference did not resolve' }
                        if ($text -notmatch 'lmx_branch_store_known\(l2_nsp\[1\], 1U, \(cast: \(@: void\) l2_nsp\[0\]\)\)') { throw 'the mutual cycle did not close' }
                        if ($text -notmatch 'lmx_branch_store_known\(l2_nsp\[1\], 2U, \(cast: \(@: void\) l2_nsp\[3\]\)\)') { throw 'the reference to a nested declaration did not resolve' }
                        # A reference to a nested entry checks the target's own
                        # parent, not the unit.
                        if ($text -notmatch 'if: l2_nsp\[3\]\\node != l2_nsp\[2\]') { throw 'a nested reference target is not checked against its own parent' }
                        # and the copy map carries the aliases through merge:
                        # both directions of the cycle plus the self loop.
                        $alias = [regex]::Matches($text, 'l2_malias: lmx_branch_struct_known\(l2_mresult, \d+U\)').Count
                        if ($alias -ne 3) { throw "$alias alias checks through the copy map, not 3" }
                    }
                    if ($case.stem -eq 'unit_named_nested') {
                        # A char field is a pointer cell into the Message char
                        # table, not a string descriptor and not inline storage.
                        # The table needs its prototype in scope: without the
                        # predef the pointer is truncated to int and the cell
                        # lands outside every owned range.
                        if ($text -notmatch 'predef: "l2src/lmx_chars_owned\.h\.lm1"') { throw 'the char table is used without its prototype in scope' }
                        $chars = @([regex]::Matches($text, 'slot\[0\]: lmx_char_cell_known\(process_chars, (\d+)\)') | ForEach-Object { $_.Groups[1].Value })
                        if (($chars -join ',') -ne '83,111') { throw "char cells $($chars -join ',') are not 83,111" }
                        # Four entries: two unit-level declarations and two
                        # nested ones, built parent first.
                        $entries = [regex]::Matches($text, 'l2_nsp\[\d+\]: lmx_struct_new_owned\(').Count
                        if ($entries -ne 4) { throw "$entries named-Structure entries, not 4" }
                        # Only unit-level declarations are unit children; a
                        # nested one is a child of its parent and its node is
                        # that parent, checked in the generated program.
                        if ([regex]::Matches($text, 'l2_nsp\[\d+\]: lmx_struct_new_owned\(unit,').Count -ne 2) { throw 'a nested entry was built as a child of the unit' }
                        if ([regex]::Matches($text, 'if: l2_nsp\[\d+\]\\node != l2_nsp\[\d+\]').Count -ne 2) { throw 'a nested entry does not check its node' }
                        # A reference field shares a pointer WITHOUT reparenting
                        # the target, which the generated program checks.
                        if ($text -notmatch 'if: l2_nsp\[0\]\\node != unit') { throw 'a stored reference is not checked against reparenting' }
                        # Source field order is preserved and the child count is
                        # fixed: Shape is 4 wide with the char first and the
                        # nested Structure second.
                        if ($text -notmatch 'lmx_branch_open_owned\(l2_nsp\[1\], 4U,') { throw 'Shape does not have its four declared fields' }
                        if ($text -notmatch 'lmx_branch_store_known\(l2_nsp\[1\], 1U, \(cast: \(@: void\) l2_nsp\[2\]\)\)') { throw 'the nested Structure is not the second field of Shape' }
                        # Merge takes both declarations plus a body field: the
                        # width is the sum of the operands' OWN children, which
                        # is what the stale-operand bug used to get wrong.
                        if ($text -notmatch 'if: l2_mresult\\len != 7') { throw 'the merged width of a named operand is not 2 + 4 + 1' }
                        # and the char survives the copy as a char.
                        if ($text -notmatch 'lmx_char_value_known\(l2_mxp\[0\]\) != 83') { throw 'the merged char field is not checked' }
                    }
                    # A for scope belongs to the method that hosts its own
                    # fields, so it is never reached through the unit from
                    # inside that method.
                    if ($text -match 'l2_h\d+: lmx_branch_struct_known\(unit,') { throw 'a for scope is still reached through the unit' }
                }
                # Every checkpoint failure must reach the turn diagnostic root
                # first; a bare abort would end the whole process instead of
                # this Message's turn (SPEC 19.13, Codex review 112535).
                $aborts = [regex]::Matches($text, '(?m)^\s*c\.abort\(\)\s*$').Count
                $escapes = [regex]::Matches($text, '(?m)^\s*if: c\.lmx_msg_poll_abort\(\) != 0\s*$').Count
                $rec.checkpointAborts = $aborts
                if ($aborts -ne $escapes) { throw "generated L1 has $aborts abort(s) but $escapes diagnostic-root escapes" }
                $rec.l1SHA256 = (Get-FileHash -LiteralPath $lm1).Hash
                $code = Invoke-Native ((Q $l1trans) + ' ' + (Q $lm1) + ' ' + (Q $cpath)) (Join-Path $out ($case.stem + '.l1trans.log'))
                if ($code -ne 0) { throw "l1trans exit $code" }
                $rec.cSHA256 = (Get-FileHash -LiteralPath $cpath).Hash
                $ctext = [IO.File]::ReadAllText((Resolve-Path -LiteralPath $cpath).ProviderPath)
                $flags = $cflags
                if ($ctext.IndexOf('l1src/p0.lm1.h') -ge 0) { $flags += ' -I lm1/build' }
                $support = 'l2src/lmx_poll_stub.c'
                if ($ctext.Contains('"l2src/lmx_message.h"')) { $support = $objList; $flags += ' -I ' + (Q $hdrs) }
                $code = Invoke-Native ("gcc $flags " + (Q $cpath) + ' ' + $support + ' -o ' + (Q $exe)) (Join-Path $out ($case.stem + '.gcc.log'))
                if ($code -ne 0) { throw "gcc exit $code" }
                $stdoutPath = Join-Path $out ($case.stem + '.stdout.txt')
                $actualExit = Invoke-Native (Q $exe) $stdoutPath
                $actualStdout = ''
                if (Test-Path -LiteralPath $stdoutPath) { $actualStdout = (Get-Content -LiteralPath $stdoutPath -Raw); if ($null -eq $actualStdout) { $actualStdout = '' } }
                $rec.actualExit = $actualExit
                if ($actualExit -ne $case.expect) { throw "exit $actualExit expected $($case.expect)" }
                if ($null -ne $case.stdout) {
                    $want = $case.stdout.Replace("`r`n", "`n").TrimEnd("`n")
                    $got = $actualStdout.Replace("`r`n", "`n").TrimEnd("`n")
                    if ($got -ne $want) { throw "stdout '$got' expected '$want'" }
                    $rec.stdout = $got
                }
                $rec.status = 'PASS'
                $ev.fixturesPassed++
            } catch {
                $rec.status = 'FAIL'
                $rec.error = "$_"
                $ev.fixturesFailed++
                Write-Host ("FAIL " + $case.stem + ": " + $_)
            }
            $ev.fixtures += $rec
        }
        if ($ev.fixturesFailed -ne 0) { throw "$($ev.fixturesFailed) of $($cases.Count) historical fixtures failed" }
        # 3. Negative qualifier cases. Only the full independent: const:
        # immutable chain carries the eternal sharing contract, so a partial
        # chain must be refused rather than quietly retained, and a bare
        # const: immutable declaration at unit level stays unsupported.
        $negatives = @(
            @{ name = 'no_const';       body = "independent:`n    (): E`n        size_t: e 7U`n    end: E`nend: independent`n";                                   expect = 'independent branch requires const' }
            @{ name = 'no_immutable';   body = "independent:`n    const:`n        (): E`n            size_t: e 7U`n        end: E`n    end: const`nend: independent`n"; expect = 'independent branch requires immutable' }
            @{ name = 'no_independent'; body = "const:`n    immutable:`n        (): E`n            size_t: e 7U`n        end: E`n    end: immutable`nend: const`n";    expect = 'unsupported body' }
            @{ name = 'primitive';      body = "independent: const: immutable: size_t: e 7U`n";                                                                       expect = 'independent qualifies Structure construction' }
            @{ name = 'bad_child';      body = "independent:`n    const:`n        immutable:`n            (): E`n                char: e 7U`n            end: E`n        end: immutable`n    end: const`nend: independent`n"; expect = 'a char field needs a quoted single character' }
            # Qualification admits an address range; it cannot make a mutable
            # target eternal, and it cannot be spent on a callable.
            @{ name = 'et_escape';   body = "independent:`n    const:`n        immutable:`n            (): E`n                M: leak`n            end: E`n        end: immutable`n    end: const`nend: independent`n`nM:`n    size_t: x 1U`nend: M`n"; expect = 'an eternal branch cannot reference mutable storage' }
            @{ name = 'et_callable'; body = "independent:`n    const:`n        immutable:`n            (): E`n                fn: m`n            end: E`n        end: immutable`n    end: const`nend: independent`n"; expect = 'an eternal branch cannot hold a callable field' }
            @{ name = 'et_update';   body = "independent:`n    const:`n        immutable:`n            (): E`n                size_t: a 7U`n            end: E`n        end: immutable`n    end: const`nend: independent`n"; tail = "    E`\a: 9U`n"; expect = 'an eternal branch field cannot be updated' }
            @{ name = 'et_nested_update'; body = "independent:`n    const:`n        immutable:`n            (): E`n                (): inner`n                    size_t: b 1U`n                end: inner`n            end: E`n        end: immutable`n    end: const`nend: independent`n"; tail = "    E`\inner`\b: 9U`n"; expect = 'an eternal branch field cannot be updated' }
            @{ name = 'et_unknown';  body = "independent:`n    const:`n        immutable:`n            (): E`n                Q: r`n            end: E`n        end: immutable`n    end: const`nend: independent`n"; expect = 'unknown nested Structure reference' }
            @{ name = 'et_no_body';  body = "independent:`n    const:`n        immutable:`n            (): E`n            end: E`n        end: immutable`n    end: const`nend: independent`n"; expect = 'eternal branch takes a name and a body' }
            # Array fields reuse the L2 own-array spelling, and each way of
            # getting it wrong names itself.
            @{ name = 'ar_bad_elem'; body = "A:`n    []: size_t xs 3`nend: A`n"; expect = 'an array field element is int or char' }
            @{ name = 'ar_overflow'; body = "A:`n    []: int xs 999999999999999999999999999999999999`nend: A`n"; expect = 'an array field needs a count' }
            @{ name = 'ar_shape';    body = "A:`n    []: int xs`nend: A`n"; expect = 'an array field needs a type, a name and a count' }
            @{ name = 'ar_no_count'; body = "A:`n    []: int xs q`nend: A`n"; expect = 'an array field needs a count' }
            @{ name = 'ar_update';   body = "A:`n    []: int xs 3`n    size_t: n 1U`nend: A`n"; tail = "    A`\xs: 5U`n"; expect = 'a field path must end at a primitive field' }
            # merge lowering: every refusal reports its own cause.
            @{ name = 'merge_unknown';   body = "independent:`n    const:`n        immutable:`n            (): E`n                size_t: e 7U`n            end: E`n        end: immutable`n    end: const`nend: independent`n"; tail = "    Z: merge: Q`n"; expect = 'unknown merge operand' }
            @{ name = 'merge_bad_field'; body = "independent:`n    const:`n        immutable:`n            (): E`n                size_t: e 7U`n            end: E`n        end: immutable`n    end: const`nend: independent`n"; tail = "    Z: merge: E`n        char: f 4U`n    end: merge`n"; expect = 'unsupported merge result body field' }
            # Named Structures: every refusal names its own cause, and a
            # malformed reserved head stays an error rather than becoming one.
            @{ name = 'ns_duplicate';  body = "A:`n    size_t: x 1U`nend: A`n`nA:`n    size_t: y 2U`nend: A`n"; expect = 'duplicate named Structure' }
            @{ name = 'ns_collide';    body = "independent:`n    const:`n        immutable:`n            (): E`n                size_t: e 7U`n            end: E`n        end: immutable`n    end: const`nend: independent`n`nE:`n    size_t: x 1U`nend: E`n"; expect = 'named Structure collides with a qualified branch' }
            # An unrecognized head is read as a reference, so a THREE-part field
            # fails on shape and a two-part one on the unknown name.
            @{ name = 'ns_bad_field';  body = "A:`n    int: x 1U`nend: A`n"; expect = 'a Structure reference field needs a name' }
            @{ name = 'ns_bad_char';   body = "A:`n    char: x 1U`nend: A`n"; expect = 'a char field needs a quoted single character' }
            # A reference may only name a COMPLETED earlier unit-level
            # declaration: unknown, forward, self and nested targets are all
            # refused, so no reference can close a cycle through the unit graph.
            @{ name = 'ns_unknown_ref'; body = "A:`n    Q: q`nend: A`n"; expect = 'unknown nested Structure reference' }
            @{ name = 'ns_ambiguous';   body = "A:`n    (): dup`n        size_t: x 1U`n    end: dup`nend: A`n`nB:`n    (): dup`n        size_t: y 2U`n    end: dup`n    dup: r`nend: B`n"; expect = 'ambiguous nested Structure reference' }
            @{ name = 'ns_nested_end';  body = "A:`n    (): origin`n        size_t: x 1U`n    end: deep`nend: A`n"; expect = 'end target does not match close target' }
            # Field paths: every refusal names its own cause, and a path is
            # only lowered where the unit is addressable.
            @{ name = 'fp_unknown_field'; body = "A:`n    size_t: x 1U`n    (): inner`n        size_t: deep 2U`n    end: inner`nend: A`n"; tail = "    A`\zzz: 5U`n"; expect = 'unknown field path segment' }
            @{ name = 'fp_through_prim';  body = "A:`n    size_t: x 1U`n    (): inner`n        size_t: deep 2U`n    end: inner`nend: A`n"; tail = "    A`\x`\y: 5U`n"; expect = 'field path goes through a slot with no Structure' }
            @{ name = 'fp_ends_struct';   body = "A:`n    size_t: x 1U`n    (): inner`n        size_t: deep 2U`n    end: inner`nend: A`n"; tail = "    A`\inner: 5U`n"; expect = 'a field path must end at a primitive field' }
            @{ name = 'fp_bad_value';     body = "A:`n    size_t: x 1U`n    (): inner`n        size_t: deep 2U`n    end: inner`nend: A`n"; tail = "    A`\x: 'q'`n"; expect = 'unsupported field path value' }
            @{ name = 'fp_load_type';     body = "A:`n    size_t: x 1U`n    (): inner`n        size_t: deep 2U`n    end: inner`nend: A`n"; tail = "    char: c`n    c: A`\x`n"; expect = 'a size_t field needs a size_t local' }
            @{ name = 'fp_unknown_root';  body = "A:`n    size_t: x 1U`n    (): inner`n        size_t: deep 2U`n    end: inner`nend: A`n"; tail = "    Q`\x: 5U`n"; expect = 'unknown field path root' }
            # Calls through a path. The parse tree of "A`\x: 5U" and
            # "A`\x(5U)" is identical, so only a callable leaf or an EMPTY
            # argument list marks a call; those are the cases with a cause.
            @{ name = 'mc_non_callable'; body = "A:`n    fn: m`n    size_t: x 1U`nend: A`n"; tail = "    A`\x()`n"; expect = 'a call path must end at a callable field' }
            @{ name = 'mc_unknown_seg';  body = "A:`n    fn: m`n    size_t: x 1U`nend: A`n"; tail = "    A`\Z()`n"; expect = 'unknown field path segment' }
            @{ name = 'mc_unknown_fn';   body = "A:`n    fn: Q`nend: A`n"; expect = 'unknown callable field' }
            @{ name = 'mc_bad_shape';    body = "A:`n    fn: m 3U`nend: A`n"; expect = 'a callable field needs a method name' }
            @{ name = 'ns_bad_end';    body = "A:`n    size_t: x 1U`nend: B`n"; expect = 'end target does not match close target' }
        )
        $ev.negatives = @()
        foreach ($neg in $negatives) {
            $src = Join-Path $out ("neg_" + $neg.name + ".lm2")
            $mbody = ''
            if ($neg.method) { $mbody = $neg.method }
            $tail = ''
            if ($neg.tail) { $tail = $neg.tail }
            [IO.File]::WriteAllText($src, $neg.body + "fn: m () int`n" + $mbody + "    return: 0`nend: m`nfn: main () int`n" + $tail + "    return: 0`nend: main`n")
            $log = Join-Path $out ("neg_" + $neg.name + ".l2trans.log")
            Invoke-Native ((Q $l2exe) + ' ' + (Q $src) + ' ' + (Q (Join-Path $out ("neg_" + $neg.name + ".lm1")))) $log | Out-Null
            $text = Get-Content -LiteralPath $log -Raw
            if ($text -notmatch [regex]::Escape($neg.expect)) { throw "negative case $($neg.name) did not report '$($neg.expect)'" }
            if (Test-Path -LiteralPath (Join-Path $out ("neg_" + $neg.name + ".lm1"))) { throw "negative case $($neg.name) produced output" }
            $ev.negatives += $neg.name
            Write-Host "OK negative $($neg.name)"
        }
    }
    Save-Evidence 'PASS'
    Write-Host "graph ABI runner PASS: selftest '$($ev.selftest.stdout)', copy selftest '$($ev.copySelftest.stdout)', merge selftest '$($ev.mergeSelftest.stdout)', fixtures $($ev.fixturesPassed)/$($ev.fixturesPassed + $ev.fixturesFailed), support warnings $supportWarnings; evidence $run"
    exit 0
} catch {
    $ev.error = "$_"
    Save-Evidence 'FAIL'
    Write-Host "graph ABI runner FAIL: $_ ; evidence $run"
    exit 1
}
