# One private translator build, one Message object set, one generated program.
param([string]$CoreCommit = 'eaac7c5')
$ErrorActionPreference = 'Stop'
$rootBaseline = Split-Path -Parent $PSScriptRoot
$rootRepo = Split-Path -Parent (Split-Path -Parent $rootBaseline)
$rootCompiler = Join-Path $rootBaseline 'build/l1trans/gen2/l1trans.exe'
$rootPin = '65D5A5ED127CA1BAEBDD1D500A5B74CEEA63EC1985EAC52EDEF28EFEB261C936'
if ((Get-FileHash -LiteralPath $rootCompiler).Hash -ne $rootPin) { throw 'Stable compiler pin mismatch' }
$rootRun = Join-Path $rootRepo ('build/codex/l2_message_root/' + (Get-Date -Format 'yyyyMMdd_HHmmss_fff') + '_' + [guid]::NewGuid().ToString('N').Substring(0,8))
$rootSnapshot = Join-Path $rootRun 'source'
New-Item -ItemType Directory -Path $rootSnapshot -Force | Out-Null
$rootOwned = @('l2trans.lm1', 'run_l2trans.ps1', 'tests/l2_message_root_driver.lm1', 'lmx_value_owned.h.lm1', 'lmx_value_owned.lm1', 'l2_text_hash.lm1', 'lmx_chars_owned.h.lm1', 'lmx_chars_owned.lm1', 'l2_foreign_alloc.lm1')
$rootEvidence = [ordered]@{result='RUNNING'; compiler=$rootCompiler; compilerSHA256=$rootPin; owned=@{}; stages=@()}
$rootEvidence.runnerSHA256 = (Get-FileHash -LiteralPath $PSCommandPath).Hash
$rootOldLocation = Get-Location
function Assert-RootExit([string]$Stage) {
    $rootEvidence.stages += @{name=$Stage; exit=$LASTEXITCODE}
    if ($LASTEXITCODE -ne 0) { throw "$Stage exit $LASTEXITCODE" }
}
function Assert-RootSignatureDiagnostics([string]$Text, [string]$Stage, [switch]$GeneratedC) {
    $symbols = '\bl2_(sig_f[01]|intern_(id|again|swap|probe)|own\d+)\b'
    if ($GeneratedC) {
        if ($Text -match $symbols) { throw "$Stage retains runtime signature diagnostics" }
    } else {
        # Metadata remains inspectable by historical contract checks, but is
        # commentary only. Canonical interning is still checked by l2_intern_prove.
        foreach ($line in ($Text -split '\r?\n')) {
            if ($line -match $symbols -and $line -notmatch '^\s*#') { throw "$Stage retains executable signature diagnostics: $line" }
        }
        foreach ($name in @('sig_f0','sig_f1','intern_id','intern_again','intern_swap','intern_probe')) {
            if ($Text -notmatch "(?m)^# .*\bl2_$name\b") { throw "$Stage lost signature evidence l2_$name" }
        }
    }
}
try {
    Set-Location $rootRepo
    $rootRevision = (git rev-parse --verify --end-of-options "$CoreCommit^{commit}").Trim()
    Assert-RootExit 'resolve'
    if ($rootRevision -notmatch '^[0-9a-f]{40}$') { throw 'Expected immutable revision' }
    $rootEvidence.coreCommit = $rootRevision
    git archive --format=zip "--output=$rootRun/core.zip" $rootRevision -- stg/l1_baseline/l1src stg/l1_baseline/l2src stg/l1_baseline/lm1/build/l1src/p0.lm1.h
    Assert-RootExit 'archive'
    $rootEvidence.archiveSHA256 = (Get-FileHash -LiteralPath "$rootRun/core.zip").Hash
    Expand-Archive -LiteralPath "$rootRun/core.zip" -DestinationPath $rootSnapshot
    $rootWork = Join-Path $rootSnapshot 'stg/l1_baseline'
    foreach ($name in $rootOwned) {
        $path = Join-Path $PSScriptRoot $name
        $rootEvidence.owned[$path] = (Get-FileHash -LiteralPath $path).Hash
        Copy-Item -LiteralPath $path -Destination (Join-Path $rootWork "l2src/$name")
    }
    # Dot-source only the build/function prefix; the large historical suite is
    # deliberately excluded from this edit-loop checkpoint.
    . (Join-Path $rootWork 'l2src/run_l2trans.ps1') -BuildOnly -OutputDirectory 'build/root_entry' -TranslatorPath $rootCompiler
    $rootEvidence.stages += @{name='translator_build'; exit=0}
    & $l2exe 'l2src/tests/unit_bool_and.lm2' "$out/program.lm1" *> "$out/program.translate.log"
    Assert-RootExit 'L2_to_L1'
    $rootL1 = Get-Content -LiteralPath "$out/program.lm1" -Raw
    Assert-RootSignatureDiagnostics $rootL1 'program L1'
    if ($rootL1 -notmatch 'fn: l2_program_entry' -or $rootL1 -notmatch 'lmx_msg_set_graph\(process_message, unit\)' -or $rootL1 -notmatch 'lmx_msg_runtime_delete\(process_runtime\)') { throw 'Missing generated entry lifecycle' }
    if ($rootL1 -notmatch 'lmx_branch_open_owned' -or $rootL1 -match 'lmx_branch_open\(|lmx_branch_child\(') { throw 'Generated unit still uses legacy branch admission/access' }
    if ($rootL1 -notmatch 'lmx_node_new_owned' -or $rootL1 -notmatch 'lmx_method_new_owned' -or $rootL1 -match 'c\.malloc\(c\.sizeof\(c\.Lmx(Method)?\)\)|lmx_range_register\(|lmx_classify\(leaf') { throw 'Root/METHOD storage still uses raw or global admission' }
    if ($rootL1 -match 'lmx_ranges_init|predef: "l2src/lmx_(branch|pool|chars|size|int)\.lm1"') { throw 'Closed unit retains unused legacy dependency' }
    & $l1trans "$out/program.lm1" "$out/program.c" *> "$out/program.c.log"
    Assert-RootExit 'L1_to_C'
    $rootC = Get-Content -LiteralPath "$out/program.c" -Raw
    Assert-RootSignatureDiagnostics $rootC 'program C' -GeneratedC
    if ($rootC -match '\blmx_(range_table|ranges_init|chars_pool|int_pool|size_pool)\b') { throw 'Closed unit still defines or calls the legacy range/pool catalog' }
    $rootObjects = @(Get-L2MessageObjects)
    & gcc @cflags -I "$out/message_support/headers" -Dmain=l2_generated_main -c "$out/program.c" -o "$out/program.o" *> "$out/program.o.log"
    Assert-RootExit 'program_object'
    $charEntrySource = @'
fn: test () int
    char: x
    x: 65
    return: x
end: test
fn: main () int
    return: test()
end: main
'@
    [IO.File]::WriteAllText((Join-Path $rootWork 'l2src/tests/unit_char_entry.lm2'), $charEntrySource)
    & $l2exe 'l2src/tests/unit_char_entry.lm2' "$out/char_entry.lm1" *> "$out/char_entry.translate.log"
    Assert-RootExit 'char_entry_L2_to_L1'
    & $l1trans "$out/char_entry.lm1" "$out/char_entry.c" *> "$out/char_entry.c.log"
    Assert-RootExit 'char_entry_L1_to_C'
    & gcc @cflags -I "$out/message_support/headers" -Dmain=l2_char_main -Dl2_program_entry=l2_char_entry -Dl2_m0=l2_char_m0 -Dl2_m1=l2_char_m1 -c "$out/char_entry.c" -o "$out/char_entry.o" *> "$out/char_entry.o.log"
    Assert-RootExit 'char_entry_object'
    & $l1trans 'l2src/tests/l2_message_root_driver.lm1' "$out/driver.c" *> "$out/driver.translate.log"
    Assert-RootExit 'driver_translate'
    $rootWrap = @('lmx_msg_runtime_new','lmx_msg_create','lmx_msg_find','lmx_msg_set_graph','lmx_msg_runtime_delete','lmx_branch_open_owned','lmx_node_new_owned','lmx_method_new_owned','lmx_int_new_owned','lmx_size_new_owned','lmx_chars_new_owned','malloc','free') | ForEach-Object { "-Wl,--wrap=$_" }
    Invoke-Gcc "$out/driver.c" "$out/driver.exe" "$out/driver.gcc.log" (@("$out/program.o", "$out/char_entry.o", '-Werror') + $rootWrap)
    $rootEvidence.stages += @{name='driver_link_real_message'; exit=0}
    $rootObjects = @(Get-L2MessageObjects)
    if ($rootObjects.Count -ne 14) { throw 'Unexpected Message object set' }
    $rootBefore = @{}
    foreach ($obj in $rootObjects) { $rootBefore[$obj] = (Get-FileHash -LiteralPath $obj).Hash }
    # Ordinary generated-program linking exercises Invoke-Gcc's same cached
    # object path, not just the instrumented driver. No second support build.
    Invoke-Gcc "$out/program.c" "$out/program.exe" "$out/program.gcc.log"
    & "$out/program.exe"
    $rootEvidence.stages += @{name='normal_exit'; exit=$LASTEXITCODE; expected=1}
    if ($LASTEXITCODE -ne 1) { throw 'Generated method result changed' }
    foreach ($obj in $rootObjects) {
        if ((Get-FileHash -LiteralPath $obj).Hash -ne $rootBefore[$obj]) { throw "Cached object changed: $obj" }
    }
    foreach ($mode in 0..23) {
        & "$out/driver.exe" $mode *> "$out/mode_$mode.log"
        Assert-RootExit "mode_$mode"
        $line = Get-Content -LiteralPath "$out/mode_$mode.log" -Raw
        if ($line -notmatch "root entry mode=$mode checks=\d+ PASS") { throw "Missing mode $mode proof" }
        Write-Output $line.Trim()
    }
    $rootDrive = @'
        c.printf("%d\n", l2_m0(unit, 0))
        return: 0
    end: main
end: external
'@
    $rootSpliced = New-L2DriveText $rootL1 $rootDrive
    if ([regex]::Matches($rootSpliced, 'fn: main \(').Count -ne 1 -or [regex]::Matches($rootSpliced, 'lmx_msg_runtime_delete\(process_runtime\)').Count -ne 1) { throw 'Splice lost or duplicated outer Message lifecycle' }
    [IO.File]::WriteAllText((Join-Path (Get-Location) "$out/splice.lm1"), $rootSpliced)
    & $l1trans "$out/splice.lm1" "$out/splice.c" *> "$out/splice.translate.log"
    Assert-RootExit 'splice_translate'
    Invoke-Gcc "$out/splice.c" "$out/splice.exe" "$out/splice.gcc.log"
    & "$out/splice.exe" *> "$out/splice.run.log"
    Assert-RootExit 'splice_run'
    if ((Get-Content -LiteralPath "$out/splice.run.log" -Raw).Trim() -ne '7') { throw 'Spliced method call changed' }
    foreach ($obj in $rootObjects) {
        if ((Get-FileHash -LiteralPath $obj).Hash -ne $rootBefore[$obj]) { throw "Splice rebuilt cached object: $obj" }
    }
    Write-Output 'root entry splice PASS (same Message object cache)'
    # Exercise the other owned-open emitter site and cached field paths with
    # an existing nested-body regression; reuse this translator/object set.
    & $l2exe 'l2src/tests/unit_forj_again.lm2' "$out/nested.lm1" *> "$out/nested.translate.log"
    Assert-RootExit 'nested_L2_to_L1'
    $rootNested = Get-Content -LiteralPath "$out/nested.lm1" -Raw
    Assert-RootSignatureDiagnostics $rootNested 'nested L1'
    if ([regex]::Matches($rootNested, 'lmx_branch_open_owned\(').Count -ne 2 -or $rootNested -match 'lmx_branch_child\(') { throw 'Nested branches not on owned/proven-layout path' }
    & $l1trans "$out/nested.lm1" "$out/nested.c" *> "$out/nested.c.log"
    Assert-RootExit 'nested_L1_to_C'
    Assert-RootSignatureDiagnostics (Get-Content -LiteralPath "$out/nested.c" -Raw) 'nested C' -GeneratedC
    Invoke-Gcc "$out/nested.c" "$out/nested.exe" "$out/nested.gcc.log"
    & "$out/nested.exe" *> "$out/nested.run.log"
    Assert-RootExit 'nested_run'
    if ((Get-Content -LiteralPath "$out/nested.run.log" -Raw).Replace("`r`n", "`n").Trim() -ne "0`n9") { throw 'Nested dirty-only field history changed' }
    foreach ($obj in $rootObjects) {
        if ((Get-FileHash -LiteralPath $obj).Hash -ne $rootBefore[$obj]) { throw "Nested case rebuilt cached object: $obj" }
    }
    Write-Output 'root entry nested branch PASS (same Message object cache)'
    # Existing regressions share this translator and the same support objects.
    function Invoke-RootPrimitiveCase([string]$stem, [string]$body, [string]$expected, [bool]$hasChar = $false) {
        & $l2exe "l2src/tests/$stem.lm2" "$out/$stem.lm1" *> "$out/$stem.translate.log"
        Assert-RootExit "${stem}_L2_to_L1"
        $text = Get-Content -LiteralPath "$out/$stem.lm1" -Raw
        Assert-RootSignatureDiagnostics $text "$stem L1"
        if ($text -match '\blmx_(int|size)_(init|take|value|store)\(') { throw "$stem retains legacy primitive operations" }
        if ($text -match 'lmx_ranges_init|predef: "l2src/lmx_(branch|pool|chars|size|int)\.lm1"') { throw "$stem retains unused legacy imports/init" }
        if ($hasChar) {
            if ($text -notmatch 'lmx_chars_new_owned\(' -or $text -notmatch 'lmx_char_cell_known\(process_chars, 0\)') { throw "$stem missing owned char initialization" }
            if ($text -match '\blmx_char_value\(') { throw "$stem retains classified character reads" }
        }
        if ($body) {
            $tail = "`n        return: 0`n    end: main`nend: external`n"
            $text = New-L2DriveText $text ($body + $tail)
            [IO.File]::WriteAllText((Join-Path (Get-Location) "$out/$stem.lm1"), $text)
        }
        & $l1trans "$out/$stem.lm1" "$out/$stem.c" *> "$out/$stem.c.log"
        Assert-RootExit "${stem}_L1_to_C"
        $cText = Get-Content -LiteralPath "$out/$stem.c" -Raw
        Assert-RootSignatureDiagnostics $cText "$stem C" -GeneratedC
        if ($cText -match '\blm_own_(alloc_fails|ok_left|absorb_fails|should_fail|ptr_stack_\w+|absorb\w*)\b') { throw "$stem retains unused L1 allocator state or machinery" }
        if ($hasChar) {
            $methods = [regex]::Matches($cText, '(?ms)^\w+ l2_m\d+\([^\r\n;]*\)\r?\n\{.*?^\}')
            if ($methods.Count -eq 0) { throw "$stem missing generated method definitions" }
            foreach ($method in $methods) {
                if ($method.Value -match '\blmx_(char_value|type_of|classify)\(') { throw "$stem method retains classified read" }
            }
        }
        if ($cText -match '\blmx_(int|size)_pool\b') { throw "$stem includes an unnecessary legacy primitive pool" }
        if ($cText -match '\blmx_(range_table|ranges_init|chars_pool|chars_init|char_cell)\b') { throw "$stem retains a legacy catalog or char pool" }
        Invoke-Gcc "$out/$stem.c" "$out/$stem.exe" "$out/$stem.gcc.log"
        & "$out/$stem.exe" *> "$out/$stem.run.log"
        Assert-RootExit "${stem}_run"
        $rawOutput = Get-Content -LiteralPath "$out/$stem.run.log" -Raw
        $actual = ''
        if ($null -ne $rawOutput) { $actual = $rawOutput.Replace("`r`n", "`n").Trim() }
        if ($actual -ne $expected) { throw "$stem output '$actual' expected '$expected'" }
        foreach ($obj in $rootObjects) {
            if ((Get-FileHash -LiteralPath $obj).Hash -ne $rootBefore[$obj]) { throw "$stem rebuilt support object: $obj" }
        }
        Write-Output "$stem PASS (same translator and Message object cache)"
    }
    Copy-Item -LiteralPath (Join-Path $rootWork 'l2src/parser_text_heap.lm2') -Destination (Join-Path $rootWork 'l2src/tests/unit_text_heap.lm2')
    Invoke-RootPrimitiveCase 'unit_text_heap' @'
        @: char copy l2_m0(unit, "abc", 3U)
        @: LmP0Text text 0
        c.array: [4]: char source
        if: copy = 0
            return: 2
        c.printf("%d %d\n", c.memcmp(copy, "abc", 3U) = 0, copy[3] = 0)
        lm_own_delete(copy, 0)
        source[0]: 97
        source[1]: 98
        source[2]: 0
        text: l2_m1(unit, source)
        if: text = 0
            return: 3
        c.printf("%d %zu\n", text\data = source, text\length)
        source[0]: 99
        c.printf("%d\n", text\data[0] = 99)
        l2_m2(unit, text)
        text: l2_m3(unit, 0)
        if: text = 0
            return: 4
        c.printf("%zu %d\n", text\length, text\data[0] = 0)
        l2_m2(unit, text)
        l2_m2(unit, 0)
'@ "1 1`n1 2`n1`n0 1"
    $resizeSource = @'
fn: resize_probe () int
    @: size_t p
    @: size_t grown
    p: lm_own_new_zero(c.sizeof(c.size_t))
    if: p = 0
        return: 1
    lm_p0_indent_store(p, 17U)
    grown: lm_own_resize(p, 2U * c.sizeof(c.size_t))
    if: grown = 0
        lm_own_delete(p, 0)
        return: 2
    c.printf: "%zu\n" lm_p0_indent_load(grown)
    p: lm_own_resize(grown, 0U)
    if: p != 0
        return: 3
    return: 0
end: resize_probe
fn: main () int
    return: resize_probe()
end: main
'@
    $indentSource = Get-Content -LiteralPath (Join-Path $rootWork 'l2src/parser_indent_stack.lm2') -Raw
    $storeStart = $indentSource.IndexOf('fn: lm_p0_indent_store')
    $pushStart = $indentSource.IndexOf('fn: lm_p0_indent_stack_push')
    if ($storeStart -lt 0 -or $pushStart -le $storeStart) { throw 'Indent helper fixture boundaries changed' }
    [IO.File]::WriteAllText((Join-Path $rootWork 'l2src/tests/unit_foreign_resize.lm2'), $indentSource.Substring($storeStart, $pushStart - $storeStart) + $resizeSource)
    Invoke-RootPrimitiveCase 'unit_foreign_resize' '' '17'
    foreach ($stem in @('unit_text_heap','unit_foreign_resize')) {
        $text = Get-Content -LiteralPath "$out/$stem.lm1" -Raw
        if ($text -notmatch 'predef: "l2src/l2_foreign_alloc.lm1"' -or $text -match 'predef: "l1src/own.lm1"') { throw "$stem did not select the narrow foreign adapter" }
    }
    # Derive bounded cases from the archived real text-view source. No new
    # translator or support build, and no full parser gate in this checkpoint.
    $viewSource = Get-Content -LiteralPath (Join-Path $rootWork 'l2src/parser_text_views.lm2') -Raw
    $viewMain = "fn: main () int`n    return: 0`nend: main`n"
    $viewSource = $viewSource.Replace("`r`n", "`n")
    $queryStart = $viewSource.IndexOf('fn: lm_p0_immut_query_make')
    $compareStart = $viewSource.IndexOf('fn: lm_p0_text_equals_query')
    if ($queryStart -lt 0 -or $compareStart -lt $queryStart) { throw 'Text-view fixture boundaries changed' }
    [IO.File]::WriteAllText((Join-Path $rootWork 'l2src/tests/unit_p0_view.lm2'), $viewSource.Substring(0, $queryStart) + $viewMain)
    [IO.File]::WriteAllText((Join-Path $rootWork 'l2src/tests/unit_query_make.lm2'), $viewSource.Substring($queryStart, $compareStart - $queryStart) + $viewMain)
    [IO.File]::WriteAllText((Join-Path $rootWork 'l2src/tests/unit_text_views.lm2'), $viewSource)
    $viewChar = @'
fn: char_marker () int
    char: marker
    marker: 65
    return: marker
end: char_marker
'@
    [IO.File]::WriteAllText((Join-Path $rootWork 'l2src/tests/unit_text_views_char.lm2'), $viewSource.Replace('fn: main () int', $viewChar + "`nfn: main () int"))
    $p0Drive = @'
        @: LmP0Text t (cast: (@: LmP0Text) c.malloc(c.sizeof(c.LmP0Text)))
        @: LmP0Text pay (cast: (@: LmP0Text) c.malloc(c.sizeof(c.LmP0Text)))
        if: t = 0 || pay = 0
            c.free(t)
            c.free(pay)
            return: 2
        t\data: "hello"
        t\length: 5U
        c.printf("%d %d %d %d %d\n", l2_m0(unit, t, "hello"), l2_m0(unit, t, "Hello"), l2_m0(unit, t, "hell"), l2_m0(unit, 0, "hello"), l2_m0(unit, t, 0))
        t\data: "`xy`"
        t\length: 4U
        c.printf("%d\n", l2_m1(unit, t, pay))
        c.printf("%d %zu\n", pay\data = t\data + 1U, pay\length)
        c.free(pay)
        c.free(t)
'@
    Invoke-RootPrimitiveCase 'unit_p0_view' $p0Drive "1 0 0 0 0`n1`n1 2"
    $queryDrive = @'
        @: L2ImmutQuery q (cast: (@: L2ImmutQuery) c.malloc(c.sizeof(c.L2ImmutQuery)))
        c.array: [4]: char word
        if: q = 0
            return: 2
        word[0]: 102
        word[1]: 110
        word[2]: 0
        c.printf("%d\n", l2_m0(unit, word, 0))
        c.printf("%d\n", l2_m0(unit, 0, q))
        c.printf("%d %zu %d\n", q\data = 0, q\length, q\live)
        c.printf("%d\n", l2_m0(unit, word, q))
        c.printf("%d %zu %d %d\n", q\data = word, q\length, q\live, q\hash = l2_fnv1a64(word, 2U))
        c.free(q)
'@
    Invoke-RootPrimitiveCase 'unit_query_make' $queryDrive "0`n0`n1 0 0`n1`n1 2 1 1"
    $viewsDrive = @'
        @: LmP0Text t (cast: (@: LmP0Text) c.malloc(c.sizeof(c.LmP0Text)))
        @: L2ImmutQuery q (cast: (@: L2ImmutQuery) c.malloc(c.sizeof(c.L2ImmutQuery)))
        c.array: [4]: char bytes
        if: t = 0 || q = 0
            c.free(t)
            c.free(q)
            return: 2
        c.printf("%d\n", l2_m2(unit, 0, q))
        c.printf("%d %d\n", l2_m3(unit, 0, q), l2_m3(unit, t, 0))
        c.printf("%d\n", l2_m2(unit, "bbb", q))
        bytes[0]: 97
        bytes[1]: 97
        bytes[2]: 97
        bytes[3]: 0
        t\data: bytes
        t\length: 3U
        c.printf("%d\n", l2_m3(unit, t, q))
        bytes[0]: 98
        bytes[1]: 98
        bytes[2]: 98
        c.printf("%d\n", l2_m3(unit, t, q))
        t\length: 2U
        c.printf("%d\n", l2_m3(unit, t, q))
        t\length: 3U
        c.printf("%d\n", l2_m2(unit, "aaa", q))
        q\hash: l2_fnv1a64("bbb", 3U)
        c.printf("%d\n", l2_m3(unit, t, q))
        c.free(q)
        c.free(t)
'@
    $viewsExpected = "0`n0 0`n1`n0`n1`n0`n1`n0"
    Invoke-RootPrimitiveCase 'unit_text_views' $viewsDrive $viewsExpected
    Invoke-RootPrimitiveCase 'unit_text_views_char' ($viewsDrive + "`n        c.printf(`"%d\n`", l2_m4(unit))") ($viewsExpected + "`n65") $true
    Invoke-RootPrimitiveCase 'unit_forj_stale' '' "9`n9 42"
    Invoke-RootPrimitiveCase 'unit_node_path' '' '0 1'
    Invoke-RootPrimitiveCase 'unit_addr_take' @'
        c.printf("%d\n", l2_m1(unit))
        leaf: lmx_branch_child(unit, 0U)
        c.printf("%zu\n", lmx_size_value(leaf\data))
'@ "0`n1"
    Invoke-RootPrimitiveCase 'unit_bind_sz' @'
        c.printf("%zu\n", l2_m0(unit, 9U))
        leaf: lmx_branch_child(unit, 0U)
        c.printf("%zu\n", lmx_size_value(leaf\data))
'@ "3`n3"
    Invoke-RootPrimitiveCase 'unit_asgn_bind_sz' @'
        @: Lmx source lmx_node_new_owned(@ process_message\blocks, @ process_message\ranges)
        @: Lmx source_field 0
        if: source = 0
            return: 2
        if: lmx_branch_open_owned(source, 1U, @ process_message\blocks, @ process_message\ranges) != 0
            return: 3
        source_field: lmx_branch_child(source, 0U)
        source_field\data: lmx_size_take()
        if: lmx_size_store(source_field\data, 9U) != 0
            return: 4
        l2_m0(unit, lmx_size_value(source_field\data))
        leaf: lmx_branch_child(unit, 0U)
        c.printf("%zu %zu\n", lmx_size_value(source_field\data), lmx_size_value(leaf\data))
'@ '9 10'
    Invoke-RootPrimitiveCase 'unit_own_same_name' @'
        l2_m0(unit)
        l2_m1(unit)
        leaf: lmx_branch_child(unit, 0U)
        kid: lmx_branch_child(unit, 1U)
        if: leaf\data = kid\data
            return: 2
        c.printf("%zu %zu\n", lmx_size_value(leaf\data), lmx_size_value(kid\data))
        if: lmx_size_store(leaf\data, 2147483648U) != 0
            return: 3
        c.printf("%zu %zu\n", lmx_size_value(leaf\data), lmx_size_value(kid\data))
'@ "1 2`n2147483648 2"
    Invoke-RootPrimitiveCase 'unit_printf_char' '        l2_m0(unit)' '65' $true
    # Reuse the exact historical drives whose cell/branch setup changed with
    # removal of the last transitive legacy import. Also cover aliased writes.
    $historicalDrives = Get-Content -LiteralPath (Join-Path $rootWork 'l2src/run_l2trans.ps1') -Raw
    foreach ($case in @(
        @{variable='dAsgn'; stem='unit_asgn_bind'; expected="11`n10`n65`n10`n11`n10`n0`n21`n10"},
        @{variable='dpre'; stem='unit_dyn_predecl'; expected="0`n66"},
        @{variable='dBind'; stem='unit_bind'; expected="0`n0`n0`n66`n66`n66`n66"}
    )) {
        $pattern = '(?ms)^\$' + $case.variable + ' = Invoke-SpliceDrive "' + $case.stem + '" @"\r?\n(.*?)^"@'
        $match = [regex]::Match($historicalDrives, $pattern)
        if (-not $match.Success) { throw "Missing historical drive $($case.variable)" }
        $body = [regex]::Replace($match.Groups[1].Value, '(?s)\s*return: 0\s*end: main\s*end: external\s*$', '')
        Invoke-RootPrimitiveCase $case.stem $body $case.expected $true
    }
    Invoke-RootPrimitiveCase 'unit_own_early' @'
        l2_m0(unit, 0)
        leaf: lmx_branch_child(unit, 0U)
        c.printf("%d\n", lmx_char_value(leaf\data))
        l2_m0(unit, 1)
        c.printf("%d\n", lmx_char_value(leaf\data))
'@ "0`n65" $true
    Invoke-RootPrimitiveCase 'unit_own_clean' @'
        l2_m1(unit, 0)
        leaf: lmx_branch_child(unit, 0U)
        c.printf("%d\n", lmx_char_value(leaf\data))
'@ '66' $true
    Invoke-RootPrimitiveCase 'unit_own_dirty_rhs' @'
        l2_m1(unit, 0)
        leaf: lmx_branch_child(unit, 0U)
        kid: lmx_branch_child(unit, 1U)
        c.printf("%d\n%d\n", lmx_char_value(leaf\data), lmx_char_value(kid\data))
'@ "65`n88" $true
    # An explicit char path reads published state; never mutate interned cells.
    $charPathSource = @'
fn: test () int
    char: x
    x: 65
    c.printf: "%d\n" x
    c.printf: "%d\n" node\x
    return: 0
end: test
fn: main () int
    return: test()
end: main
'@
    [IO.File]::WriteAllText((Join-Path $rootWork 'l2src/tests/unit_char_known_path.lm2'), $charPathSource)
    Invoke-RootPrimitiveCase 'unit_char_known_path' '' "65`n65" $true
    $charPathL1 = Get-Content -LiteralPath "$out/unit_char_known_path.lm1" -Raw
    if ($charPathL1 -notmatch 'lmx_char_value_known\(l2_xp\\data\)' -or $charPathL1 -notmatch 'lmx_char_value_known\(l2_q\d+_from\\data\)') { throw 'Char explicit path/cache read was not exercised' }
    Invoke-RootPrimitiveCase 'unit_own5' '        l2_m0(unit)' ''
    Invoke-RootPrimitiveCase 'unit_own6' '        l2_m0(unit)' '' $true
    foreach ($case in @(@{stem='unit_own5'; names=@('a','b','c','d','e')}, @{stem='unit_own6'; names=@('i','ch','count','width','line','column')})) {
        $text = Get-Content -LiteralPath "$out/$($case.stem).lm1" -Raw
        $ownNames = [regex]::Matches($text, '(?m)^# const: @\(char l2_own(\d+)\) "([^"]*)"\r?$')
        if ($ownNames.Count -ne $case.names.Count) { throw "$($case.stem) own-name evidence count changed" }
        for ($i = 0; $i -lt $case.names.Count; $i++) {
            if ([int]$ownNames[$i].Groups[1].Value -ne $i -or $ownNames[$i].Groups[2].Value -ne $case.names[$i]) { throw "$($case.stem) own-name evidence/order changed at $i" }
        }
    }
    # Reuse existing formal-order/name fixtures. Translation itself runs the
    # canonical intern proof; no extra compiler build or runtime object set.
    $contracts = @{}
    foreach ($stem in @('add','entry_plus','entry_sum','entry_swap_formals')) {
        & $l2exe "l2src/tests/$stem.lm2" "$out/$stem.contract.lm1" *> "$out/$stem.contract.log"
        Assert-RootExit "${stem}_contract"
        $text = Get-Content -LiteralPath "$out/$stem.contract.lm1" -Raw
        Assert-RootSignatureDiagnostics $text "$stem contract"
        $contract = @{}
        foreach ($name in @('id','again','swap','probe')) {
            if ($text -notmatch "(?m)^# unsigned: l2_intern_$name (\d+)U$") { throw "$stem missing intern $name" }
            $contract[$name] = [int]$Matches[1]
        }
        foreach ($name in @('f0','f1')) {
            if ($text -notmatch ('(?m)^# const: @\(char l2_sig_' + $name + '\) "([^"]*)"$')) { throw "$stem missing formal $name" }
            $contract[$name] = $Matches[1]
        }
        if ($contract.id -ne $contract.again) { throw "$stem repeated contract changed identity" }
        if ($text -notmatch 'rec\\sig: (\d+)U' -or [int]$Matches[1] -ne $contract.id) { throw "$stem METHOD sig differs from intern evidence" }
        $contracts[$stem] = $contract
    }
    if ($contracts.add.f0 -ne 'a' -or $contracts.add.f1 -ne 'b' -or $contracts.add.probe -ne $contracts.add.id -or $contracts.add.swap -eq $contracts.add.id) { throw 'add canonical order/probe evidence changed' }
    if ($contracts.entry_plus.f0 -ne 'a' -or $contracts.entry_plus.f1 -ne 'b' -or $contracts.entry_plus.id -ne $contracts.add.id) { throw 'Method rename changed canonical signature' }
    if ($contracts.entry_sum.f0 -ne 'x' -or $contracts.entry_sum.f1 -ne 'y' -or $contracts.entry_sum.probe -eq $contracts.entry_sum.id) { throw 'Different formals collapsed to probe' }
    if ($contracts.entry_swap_formals.f0 -ne 'b' -or $contracts.entry_swap_formals.f1 -ne 'a' -or $contracts.entry_swap_formals.probe -eq $contracts.entry_swap_formals.id) { throw 'Swapped formal order collapsed to probe' }
    Write-Output 'signature diagnostic metadata PASS (no runtime diagnostic state)'
    foreach ($path in $rootEvidence.owned.Keys) {
        if ((Get-FileHash -LiteralPath $path).Hash -ne $rootEvidence.owned[$path]) { throw "Owned source changed: $path" }
    }
    if ((Get-FileHash -LiteralPath $rootCompiler).Hash -ne $rootPin) { throw 'Stable compiler changed' }
    if ((Get-FileHash -LiteralPath $PSCommandPath).Hash -ne $rootEvidence.runnerSHA256) { throw 'Runner changed' }
    $rootEvidence.artifacts = @{}
    Get-ChildItem -LiteralPath (Join-Path $rootWork $out) -File -Recurse | ForEach-Object { $rootEvidence.artifacts[$_.FullName] = (Get-FileHash -LiteralPath $_.FullName).Hash }
    $rootEvidence.result = 'PASS'
} catch {
    $rootEvidence.result = 'FAIL'
    $rootEvidence.error = $_.Exception.Message
    throw
} finally {
    Set-Location $rootOldLocation
    $rootEvidence | ConvertTo-Json -Depth 7 | Set-Content -LiteralPath "$rootRun/evidence.json" -Encoding utf8
    Write-Output "Evidence: $rootRun"
}
