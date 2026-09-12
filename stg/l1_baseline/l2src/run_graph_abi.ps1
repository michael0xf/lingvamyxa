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
    $names = @('lmx_msg_blocks', 'lmx_owned_ranges', 'lmx_msg_storage', 'lmx_msg_path_storage', 'lmx_msg_slots', 'lmx_msg_mail_chain', 'lmx_msg_sched_ready', 'lmx_msg_visit', 'lmx_msg_liveness', 'lmx_msg_history_owned', 'lmx_msg_roots_stale', 'lmx_branch_owned', 'lmx_value_owned', 'lmx_chars_owned', 'lmx_array_owned', 'lmx_array_ref_owned', 'lmx_graph_copy_owned')
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
    $instrNames = @('lmx_graph_copy_owned', 'lmx_branch_owned', 'lmx_value_owned', 'lmx_chars_owned', 'lmx_array_owned', 'lmx_array_ref_owned', 'lmx_msg_blocks')
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
                    $roots = [regex]::Matches($text, 'l2_ebr: lmx_node_new_owned\(').Count
                    $stored = [regex]::Matches($text, 'l2_branch_refs\[\d+U\]: l2_ebr').Count
                    $rec.eternalRoots = $roots
                    if ($roots -ne $stored) { throw "$roots eternal roots but $stored retention entries" }
                    if ($roots -gt 0) {
                        if ($text -notmatch 'lmx_array_ref_new_positive_owned\(c\.LMX_TYPE_ARRAY_OF_LMX,') { throw 'eternal roots without the ARRAY_OF_LMX retention array' }
                        # Twice per root: at construction, and again after the
                        # declaration-site reference is stored, since storing a
                        # reference must not reparent an independent branch.
                        if ([regex]::Matches($text, 'if: l2_ebr\\node != 0').Count -ne (2 * $roots)) { throw 'an eternal root is not checked for a zero lexical root before and after the declaration-site store' }
                        if ([regex]::Matches($text, 'lmx_branch_store_known\(unit, \d+U, \(cast: \(@: void\) l2_ebr\)\)').Count -ne $roots) { throw 'an eternal branch has no declaration-site reference in the unit graph' }
                        if ($text -match 'l2_branch_refs\[\d+U\]: rec') { throw 'a METHOD descriptor was stored in the retention array' }
                        if ($case.stem -eq 'unit_eternal_many' -and $roots -ne 70) { throw "growth fixture produced $roots roots, not 70" }
                        # The first Message stays the sole storage OWNER of every
                        # qualified branch; the eternal list is a non-owning
                        # classifier over that same storage. A branch allocated
                        # from any other arena would break single teardown.
                        if ($text -match 'l2_ebr: lmx_node_new_owned\(@ (?!process_message\\blocks)') { throw 'a qualified branch is not allocated from the first Message arena' }
                        if ($text -notmatch 'lmx_owned_ranges_find\(process_message\\ranges,') { throw 'no check that a qualified branch still classifies in the owner ranges' }
                    }
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
            @{ name = 'bad_child';      body = "independent:`n    const:`n        immutable:`n            (): E`n                char: e 7U`n            end: E`n        end: immutable`n    end: const`nend: independent`n"; expect = 'unsupported eternal branch child' }
        )
        $ev.negatives = @()
        foreach ($neg in $negatives) {
            $src = Join-Path $out ("neg_" + $neg.name + ".lm2")
            [IO.File]::WriteAllText($src, $neg.body + "fn: m () int`n    return: 0`nend: m`nfn: main () int`n    return: 0`nend: main`n")
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
    Write-Host "graph ABI runner PASS: selftest '$($ev.selftest.stdout)', copy selftest '$($ev.copySelftest.stdout)', fixtures $($ev.fixturesPassed)/$($ev.fixturesPassed + $ev.fixturesFailed), support warnings $supportWarnings; evidence $run"
    exit 0
} catch {
    $ev.error = "$_"
    Save-Evidence 'FAIL'
    Write-Host "graph ABI runner FAIL: $_ ; evidence $run"
    exit 1
}
