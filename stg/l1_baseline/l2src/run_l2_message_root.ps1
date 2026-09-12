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
$rootOwned = @('l2trans.lm1', 'run_l2trans.ps1', 'tests/l2_message_root_driver.lm1', 'lmx_value_owned.h.lm1', 'lmx_value_owned.lm1', 'l2_text_hash.lm1', 'lmx_chars_owned.h.lm1', 'lmx_chars_owned.lm1', 'l2_foreign_alloc.lm1', 'lmx_array_owned.h.lm1', 'lmx_array_owned.lm1', 'tests/unit_own_array_int.lm2', 'tests/unit_own_array_index.lm2', 'tests/unit_own_array_char_index.lm2', 'tests/unit_own_array_length.lm2', 'tests/unit_for_own_arrays.lm2')
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
    & $l2exe 'l2src/tests/unit_own_array_int.lm2' "$out/array_entry.lm1" *> "$out/array_entry.translate.log"
    Assert-RootExit 'array_entry_L2_to_L1'
    $arrayL1 = Get-Content -LiteralPath "$out/array_entry.lm1" -Raw
    if ([regex]::Matches($arrayL1, 'lmx_array_new_positive_owned\(').Count -ne 2 -or $arrayL1 -notmatch 'LMX_TYPE_ARRAY_OF_INT, 3U' -or $arrayL1 -notmatch 'LMX_TYPE_ARRAY_OF_CHAR, 4U') { throw 'Missing source array constructors or decimal extents' }
    if ($arrayL1 -match 'l2_q\d+(_dirty|_from)?\b|lmx_chars_new_owned|c\.array:') { throw 'Own arrays emitted as scalar caches, intern table or C-local storage' }
    & $l1trans "$out/array_entry.lm1" "$out/array_entry.c" *> "$out/array_entry.c.log"
    Assert-RootExit 'array_entry_L1_to_C'
    & gcc @cflags -I "$out/message_support/headers" -Dmain=l2_array_main -Dl2_program_entry=l2_array_entry -Dl2_m0=l2_array_m0 -c "$out/array_entry.c" -o "$out/array_entry.o" *> "$out/array_entry.o.log"
    Assert-RootExit 'array_entry_object'
    & $l2exe 'l2src/tests/unit_own_array_index.lm2' "$out/array_index.lm1" *> "$out/array_index.translate.log"
    Assert-RootExit 'array_index_L2_to_L1'
    & $l1trans "$out/array_index.lm1" "$out/array_index.c" *> "$out/array_index.c.log"
    Assert-RootExit 'array_index_L1_to_C'
    & gcc @cflags -I "$out/message_support/headers" -Dmain=l2_array_index_main -Dl2_program_entry=l2_array_index_entry -Dl2_m0=l2_array_index_m0 -c "$out/array_index.c" -o "$out/array_index.o" *> "$out/array_index.o.log"
    Assert-RootExit 'array_index_object'
    $indexSource = Get-Content -LiteralPath 'l2src/tests/unit_own_array_index.lm2' -Raw
    $indexPrograms = [ordered]@{
        first_fixture = @{source=$indexSource.Replace('    buf[02]: z', '').Replace('return: buf[0] + buf[1] + buf[2]', 'return: buf[0]'); expected=7}
        cell_to_cell = @{source=$indexSource.Replace('buf[02]: z', 'buf[02]: buf[0] + z'); expected=23}
        ccall = @{source=$indexSource.Replace('    return: buf[0]', ('    c.printf: "%d " buf[2]' + [char]10 + '    return: buf[0]')); expected=16; stdout='9 '}
    }
    foreach ($case in $indexPrograms.Keys) {
        $programCase = $indexPrograms[$case]
        [IO.File]::WriteAllText((Join-Path $rootWork "$out/index_$case.lm2"), $programCase.source)
        & $l2exe "$out/index_$case.lm2" "$out/index_$case.lm1" *> "$out/index_$case.translate.log"
        Assert-RootExit "index_${case}_L2_to_L1"
        & $l1trans "$out/index_$case.lm1" "$out/index_$case.c" *> "$out/index_$case.c.log"
        Assert-RootExit "index_${case}_L1_to_C"
        Invoke-Gcc "$out/index_$case.c" "$out/index_$case.exe" "$out/index_$case.gcc.log"
        & "$out/index_$case.exe" *> "$out/index_$case.run.log"
        $rootEvidence.stages += @{name="index_${case}_run";exit=$LASTEXITCODE;expected=$programCase.expected}
        if ($LASTEXITCODE -ne $programCase.expected) { throw "Incorrect array index $case result" }
        if ($programCase.ContainsKey('stdout') -and (Get-Content "$out/index_$case.run.log" -Raw).Trim() -ne $programCase.stdout.Trim()) { throw "Incorrect array index $case stdout" }
    }
    $indexInvalid = [ordered]@{
        store_dynamic = $indexSource.Replace('buf[000]:', 'buf[z]:')
        store_negative = $indexSource.Replace('buf[000]:', 'buf[-1]:')
        store_limit = $indexSource.Replace('buf[000]:', 'buf[3]:')
        store_overflow = $indexSource.Replace('buf[000]:', 'buf[184467440737095516160]:')
        store_suffix = $indexSource.Replace('buf[000]:', 'buf[0U]:')
        load_dynamic = $indexSource.Replace('return: buf[0]', 'return: buf[z]')
        load_negative = $indexSource.Replace('return: buf[0]', 'return: buf[-1]')
        load_limit = $indexSource.Replace('return: buf[0]', 'return: buf[3]')
        load_overflow = $indexSource.Replace('return: buf[0]', 'return: buf[184467440737095516160]')
        load_suffix = $indexSource.Replace('return: buf[0]', 'return: buf[0U]')
        rank_two = $indexSource.Replace('return: buf[0]', 'return: buf[0][1]')
        reference_type = $indexSource.Replace('[]: int buf', '[]: Lmx buf')
        element_address = $indexSource.Replace('return: buf[0]', 'return: @ buf[0]')
        view = $indexSource.Replace('return: buf[0]', 'return: buf[0:2]')
    }
    foreach ($case in $indexInvalid.Keys) {
        [IO.File]::WriteAllText((Join-Path $rootWork "$out/index_invalid_$case.lm2"), $indexInvalid[$case])
        & $l2exe "$out/index_invalid_$case.lm2" "$out/index_invalid_$case.lm1" *> "$out/index_invalid_$case.log"
        $rootEvidence.stages += @{name="index_invalid_$case";exit=$LASTEXITCODE;expected=1}
        if ($LASTEXITCODE -ne 1 -or (Test-Path "$out/index_invalid_$case.lm1")) { throw "Unsupported array index $case accepted/published" }
    }
    & $l2exe 'l2src/tests/unit_own_array_char_index.lm2' "$out/array_char_index.lm1" *> "$out/array_char_index.translate.log"
    Assert-RootExit 'array_char_index_L2_to_L1'
    $charIndexL1 = Get-Content "$out/array_char_index.lm1" -Raw
    if ($charIndexL1 -notmatch '@: char l2_a\d+_data' -or $charIndexL1 -match 'lmx_char_rebind_known|lmx_chars_new_owned|c\.array:|l2_q\d+(_dirty|_from)?\b') { throw 'CHAR indexing lost mutable byte storage contract' }
    & $l1trans "$out/array_char_index.lm1" "$out/array_char_index.c" *> "$out/array_char_index.c.log"
    Assert-RootExit 'array_char_index_L1_to_C'
    & gcc @cflags -I "$out/message_support/headers" -Dmain=l2_array_char_index_main -Dl2_program_entry=l2_array_char_index_entry -Dl2_m0=l2_array_char_index_m0 -c "$out/array_char_index.c" -o "$out/array_char_index.o" *> "$out/array_char_index.o.log"
    Assert-RootExit 'array_char_index_object'
    $charIndexSource = Get-Content -LiteralPath 'l2src/tests/unit_own_array_char_index.lm2' -Raw
    $charInvalid = [ordered]@{
        store_dynamic=$charIndexSource.Replace('letters[000]:', 'letters[z]:')
        store_negative=$charIndexSource.Replace('letters[000]:', 'letters[-1]:')
        store_limit=$charIndexSource.Replace('letters[000]:', 'letters[3]:')
        load_dynamic=$charIndexSource.Replace('return: letters[0]', 'return: letters[z]')
        load_negative=$charIndexSource.Replace('return: letters[0]', 'return: letters[-1]')
        load_limit=$charIndexSource.Replace('return: letters[0]', 'return: letters[3]')
    }
    foreach ($case in $charInvalid.Keys) {
        [IO.File]::WriteAllText((Join-Path $rootWork "$out/char_index_invalid_$case.lm2"), $charInvalid[$case])
        & $l2exe "$out/char_index_invalid_$case.lm2" "$out/char_index_invalid_$case.lm1" *> "$out/char_index_invalid_$case.log"
        $rootEvidence.stages += @{name="char_index_invalid_$case";exit=$LASTEXITCODE;expected=1}
        if ($LASTEXITCODE -ne 1 -or (Test-Path "$out/char_index_invalid_$case.lm1")) { throw "Unsupported CHAR index $case accepted/published" }
    }
    $charCcall = $charIndexSource.Replace('    return: letters[0]', ('    c.printf: "%d " letters[2]' + [char]10 + '    return: letters[0]'))
    [IO.File]::WriteAllText((Join-Path $rootWork "$out/char_index_ccall.lm2"), $charCcall)
    & $l2exe "$out/char_index_ccall.lm2" "$out/char_index_ccall.lm1" *> "$out/char_index_ccall.translate.log"
    Assert-RootExit 'char_index_ccall_L2_to_L1'
    & $l1trans "$out/char_index_ccall.lm1" "$out/char_index_ccall.c" *> "$out/char_index_ccall.c.log"
    Assert-RootExit 'char_index_ccall_L1_to_C'
    Invoke-Gcc "$out/char_index_ccall.c" "$out/char_index_ccall.exe" "$out/char_index_ccall.gcc.log"
    & "$out/char_index_ccall.exe" *> "$out/char_index_ccall.run.log"
    $rootEvidence.stages += @{name='char_index_ccall_run';exit=$LASTEXITCODE;expected=127}
    if ($LASTEXITCODE -ne 127 -or (Get-Content "$out/char_index_ccall.run.log" -Raw).Trim() -ne '62') { throw 'CHAR vararg promotion changed value' }
    & $l2exe 'l2src/tests/unit_own_array_length.lm2' "$out/array_length.lm1" *> "$out/array_length.translate.log"
    Assert-RootExit 'array_length_L2_to_L1'
    $lengthL1 = Get-Content "$out/array_length.lm1" -Raw
    if ([regex]::Matches($lengthL1, 'l2_t\d+: l2_a\d+_desc\\len').Count -ne 2 -or $lengthL1 -notmatch 'size_t: l2_t\d+') { throw 'Array length did not read descriptor size_t len' }
    & $l1trans "$out/array_length.lm1" "$out/array_length.c" *> "$out/array_length.c.log"
    Assert-RootExit 'array_length_L1_to_C'
    & gcc @cflags -I "$out/message_support/headers" -Dmain=l2_array_length_main -Dl2_program_entry=l2_array_length_entry -Dl2_m0=l2_array_length_m0 -c "$out/array_length.c" -o "$out/array_length.o" *> "$out/array_length.o.log"
    Assert-RootExit 'array_length_object'
    $lengthSource = Get-Content -LiteralPath 'l2src/tests/unit_own_array_length.lm2' -Raw
    $lengthInvalid = [ordered]@{
        no_argument=$lengthSource.Replace('length(buf)', 'length()')
        too_many=$lengthSource.Replace('length(buf)', 'length(buf, letters)')
        scalar=$lengthSource.Replace('length(buf)', 'length(z)')
        unknown=$lengthSource.Replace('length(buf)', 'length(missing)')
        element=$lengthSource.Replace('length(buf)', 'length(buf[0])')
        address=$lengthSource.Replace('length(buf)', 'length(@ buf)')
    }
    foreach ($case in $lengthInvalid.Keys) {
        [IO.File]::WriteAllText((Join-Path $rootWork "$out/length_invalid_$case.lm2"), $lengthInvalid[$case])
        & $l2exe "$out/length_invalid_$case.lm2" "$out/length_invalid_$case.lm1" *> "$out/length_invalid_$case.log"
        $rootEvidence.stages += @{name="length_invalid_$case";exit=$LASTEXITCODE;expected=1}
        if ($LASTEXITCODE -ne 1 -or (Test-Path "$out/length_invalid_$case.lm1")) { throw "Unsupported array length $case accepted/published" }
    }
    $lengthShadow = @'
fn: length (int: z) int
    return: z + 2
end: length
fn: main () int
    return: length(7)
end: main
'@
    [IO.File]::WriteAllText((Join-Path $rootWork "$out/length_shadow.lm2"), $lengthShadow)
    & $l2exe "$out/length_shadow.lm2" "$out/length_shadow.lm1" *> "$out/length_shadow.translate.log"
    Assert-RootExit 'length_shadow_L2_to_L1'
    & $l1trans "$out/length_shadow.lm1" "$out/length_shadow.c" *> "$out/length_shadow.c.log"
    Assert-RootExit 'length_shadow_L1_to_C'
    Invoke-Gcc "$out/length_shadow.c" "$out/length_shadow.exe" "$out/length_shadow.gcc.log"
    & "$out/length_shadow.exe" *> "$out/length_shadow.run.log"
    $rootEvidence.stages += @{name='length_shadow_run';exit=$LASTEXITCODE;expected=9}
    if ($LASTEXITCODE -ne 9) { throw 'Declared source length method lost normal resolution' }
    & $l2exe 'l2src/tests/unit_for_own_arrays.lm2' "$out/for_arrays.lm1" *> "$out/for_arrays.translate.log"
    Assert-RootExit 'for_arrays_L2_to_L1'
    $forL1 = Get-Content "$out/for_arrays.lm1" -Raw
    if ($forL1 -notmatch 'kid\\data: lmx_array_new_positive_owned' -or $forL1 -notmatch 'l2_a\d+_leaf: lmx_branch_child_known\(l2_h\d+,' -or [regex]::Matches($forL1, 'l2_t\d+: l2_a\d+_desc\\len').Count -ne 2) { throw 'For arrays did not use host children/live lengths' }
    & $l1trans "$out/for_arrays.lm1" "$out/for_arrays.c" *> "$out/for_arrays.c.log"
    Assert-RootExit 'for_arrays_L1_to_C'
    & gcc @cflags -I "$out/message_support/headers" -Dmain=l2_for_array_main -Dl2_program_entry=l2_for_array_entry -Dl2_m0=l2_for_array_m0 -c "$out/for_arrays.c" -o "$out/for_arrays.o" *> "$out/for_arrays.o.log"
    Assert-RootExit 'for_arrays_object'
    $forSource = Get-Content -LiteralPath 'l2src/tests/unit_for_own_arrays.lm2' -Raw
    # A second host prevents accidentally hard-coding the first for node.
    $nestedForLines = [System.Collections.Generic.List[string]]::new()
    $inForBody = $false
    foreach ($line in ($forSource -split '\r?\n')) {
        if ($line -eq '    for: int(i, 0) (i < 2) i++') {
            $nestedForLines.Add('    for: int(outer, 0) (outer < 2) outer++')
            $inForBody = $true
        }
        if ($inForBody) { $nestedForLines.Add('    ' + $line) } else { $nestedForLines.Add($line) }
        if ($inForBody -and $line -eq '    end: for') {
            $nestedForLines.Add('    end: for')
            $inForBody = $false
        }
    }
    [IO.File]::WriteAllText((Join-Path $rootWork "$out/for_arrays_nested.lm2"), ($nestedForLines -join [char]10))
    & $l2exe "$out/for_arrays_nested.lm2" "$out/for_arrays_nested.lm1" *> "$out/for_arrays_nested.translate.log"
    Assert-RootExit 'for_arrays_nested_L2_to_L1'
    & $l1trans "$out/for_arrays_nested.lm1" "$out/for_arrays_nested.c" *> "$out/for_arrays_nested.c.log"
    Assert-RootExit 'for_arrays_nested_L1_to_C'
    Invoke-Gcc "$out/for_arrays_nested.c" "$out/for_arrays_nested.exe" "$out/for_arrays_nested.gcc.log"
    & "$out/for_arrays_nested.exe" *> "$out/for_arrays_nested.run.log"
    $rootEvidence.stages += @{name='for_arrays_nested_run';exit=$LASTEXITCODE;expected=155}
    if ($LASTEXITCODE -ne 155) { throw 'Nested for arrays lost host identity or persistent values' }
    $forBad = [ordered]@{
        zero=$forSource.Replace('int buf 003','int buf 0')
        dynamic_extent=$forSource.Replace('int buf 003','int buf z')
        dynamic_index=$forSource.Replace('buf[0]: buf[0] + z','buf[i]: buf[0] + z')
        out_of_bounds=$forSource.Replace('buf[2]: i','buf[3]: i')
        reference=$forSource.Replace('int buf 003','Lmx buf 3')
        escape=$forSource.Replace('return: total','return: buf')
        scalar_rebind=$forSource.Replace('buf[0]: buf[0] + z','buf: 7')
        if_inside_for=$forSource.Replace('        []: int buf 003', ("        if: z" + [char]10 + "            []: int buf 003" + [char]10 + "        ---"))
        header_array=$forSource.Replace('for: int(i, 0)', 'for: [](int buf 3)')
    }
    foreach ($case in $forBad.Keys) {
        [IO.File]::WriteAllText((Join-Path $rootWork "$out/for_arrays_bad_$case.lm2"), $forBad[$case])
        & $l2exe "$out/for_arrays_bad_$case.lm2" "$out/for_arrays_bad_$case.lm1" *> "$out/for_arrays_bad_$case.log"
        $rootEvidence.stages += @{name="for_arrays_bad_$case";exit=$LASTEXITCODE;expected=1}
        if ($LASTEXITCODE -ne 1 -or (Test-Path "$out/for_arrays_bad_$case.lm1")) { throw "Unsupported for array $case accepted/published" }
    }
    # Sweep compiler allocations for the new host/array combination. Reuse
    # the one translator and preserve existing output on every injected OOM.
    $priorForFail = $env:L2_FAIL_MALLOC
    $priorForLog = $env:L2_ALLOC_LOG
    try {
        $env:L2_FAIL_MALLOC = $null
        $env:L2_ALLOC_LOG = "$out/for_arrays.alloc"
        & $l2exe 'l2src/tests/unit_for_own_arrays.lm2' "$out/for_arrays_probe.lm1" *> "$out/for_arrays_probe.log"
        Assert-RootExit 'for_arrays_alloc_probe'
        $forAllocText = Get-Content $env:L2_ALLOC_LOG -Raw
        if ($forAllocText -notmatch '^n=(\d+) free=\d+ live=0 ') { throw 'For array compiler metadata leaked' }
        $forAllocations = [int]$Matches[1]
        for ($fault=1; $fault -le $forAllocations; $fault++) {
            $destFor = "$out/for_arrays_oom_$fault.lm1"
            [IO.File]::WriteAllText((Join-Path $rootWork $destFor), 'PRESERVE_EXISTING_OUTPUT')
            $env:L2_FAIL_MALLOC = [string]$fault
            $env:L2_ALLOC_LOG = "$out/for_arrays_oom_$fault.alloc"
            & $l2exe 'l2src/tests/unit_for_own_arrays.lm2' $destFor *> "$out/for_arrays_oom_$fault.log"
            $rootEvidence.stages += @{name="for_arrays_oom_$fault";exit=$LASTEXITCODE;expected=1}
            if ($LASTEXITCODE -ne 1 -or (Get-Content $destFor -Raw) -ne 'PRESERVE_EXISTING_OUTPUT') { throw "For array compiler OOM $fault changed output" }
            if ((Get-Content $env:L2_ALLOC_LOG -Raw) -notmatch ' live=0 .*fail_kind=[1-9]') { throw "For array compiler OOM $fault leaked or missed fault" }
        }
        $rootEvidence.forArrayAllocationFailures = $forAllocations
    } finally {
        $env:L2_FAIL_MALLOC = $priorForFail
        $env:L2_ALLOC_LOG = $priorForLog
    }
    $arraySource = Get-Content -LiteralPath 'l2src/tests/unit_own_array_int.lm2' -Raw
    $nlArray = [string][char]10
    # Force own-metadata growth with live array extent entries, then sweep its
    # allocator failures using the existing compiler hook. Same translator and
    # support objects; this does not rebuild any helper or current core.
    $arrayGrowth = $arraySource.Replace('    return: 0', ((5..8 | ForEach-Object { "    []: int extra$_ $_" }) -join $nlArray) + $nlArray + '    return: 0')
    [IO.File]::WriteAllText((Join-Path $rootWork "$out/array_growth.lm2"), $arrayGrowth)
    $priorArrayFail = $env:L2_FAIL_MALLOC
    $priorArrayLog = $env:L2_ALLOC_LOG
    try {
        $env:L2_FAIL_MALLOC = $null
        $env:L2_ALLOC_LOG = "$out/array_growth.alloc"
        & $l2exe "$out/array_growth.lm2" "$out/array_growth.lm1" *> "$out/array_growth.translate.log"
        Assert-RootExit 'array_growth_L2_to_L1'
        $allocText = Get-Content -LiteralPath "$out/array_growth.alloc" -Raw
        if ($allocText -notmatch '^n=(\d+) free=\d+ live=0 ') { throw 'Array metadata growth leaked' }
        $arrayAllocationCount = [int]$Matches[1]
        $grownL1 = Get-Content -LiteralPath "$out/array_growth.lm1" -Raw
        foreach ($extent in 3..8) {
            if ($grownL1 -notmatch "LMX_TYPE_ARRAY_OF_(INT|CHAR), ${extent}U") { throw "Array metadata lost extent $extent on growth" }
        }
        & $l1trans "$out/array_growth.lm1" "$out/array_growth.c" *> "$out/array_growth.c.log"
        Assert-RootExit 'array_growth_L1_to_C'
        Invoke-Gcc "$out/array_growth.c" "$out/array_growth.exe" "$out/array_growth.gcc.log"
        & "$out/array_growth.exe" *> "$out/array_growth.run.log"
        Assert-RootExit 'array_growth_run'
        for ($fault = 1; $fault -le $arrayAllocationCount; $fault++) {
            $destArray = "$out/array_oom_$fault.lm1"
            [IO.File]::WriteAllText((Join-Path $rootWork $destArray), 'PRESERVE_EXISTING_OUTPUT')
            $env:L2_FAIL_MALLOC = [string]$fault
            $env:L2_ALLOC_LOG = "$out/array_oom_$fault.alloc"
            & $l2exe "$out/array_growth.lm2" $destArray *> "$out/array_oom_$fault.log"
            $rootEvidence.stages += @{name="array_metadata_oom_$fault";exit=$LASTEXITCODE;expected=1}
            if ($LASTEXITCODE -ne 1 -or (Get-Content -LiteralPath $destArray -Raw) -ne 'PRESERVE_EXISTING_OUTPUT') { throw "Compiler array OOM $fault succeeded or replaced output" }
            if ((Get-Content -LiteralPath $env:L2_ALLOC_LOG -Raw) -notmatch ' live=0 .*fail_kind=[1-9]') { throw "Compiler array OOM $fault leaks or was not injected" }
        }
        $rootEvidence.arrayMetadataAllocationFailures = $arrayAllocationCount
    } finally {
        $env:L2_FAIL_MALLOC = $priorArrayFail
        $env:L2_ALLOC_LOG = $priorArrayLog
    }
    $invalidArrays = [ordered]@{
        zero = $arraySource.Replace('int buf 003','int buf 0')
        negative = $arraySource.Replace('int buf 003','int buf -1')
        overflow = $arraySource.Replace('int buf 003','int buf 184467440737095516160')
        dynamic = $arraySource.Replace('int buf 003','int buf z')
        initialized = $arraySource.Replace('int buf 003','int buf 3 1 2 3')
        reference = $arraySource.Replace('int buf 003','Lmx buf 3')
        scalar_read = $arraySource.Replace('return: 0','return: buf')
        address = $arraySource.Replace('return: 0','return: @ buf')
        path_read = $arraySource.Replace('return: 0','return: node\buf')
        path_store = $arraySource.Replace('return: 0', ('node\buf: 7' + $nlArray + '    return: 0'))
        indexed_store = $arraySource.Replace('return: 0', ('buf[z]: 7' + $nlArray + '    return: 0'))
        scalar_store = $arraySource.Replace('return: 0', ('buf: 7' + $nlArray + '    return: 0'))
        duplicate = $arraySource.Replace('[]: char letters 4','[]: int buf 3')
        entry_body = (@('fn: main () int','    []: int buf 3','    return: 0','end: main','') -join $nlArray)
        nested = $arraySource.Replace('    []: int buf 003', (@('    if: z','        []: int buf 3','    ---') -join $nlArray))
    }
    foreach ($case in $invalidArrays.Keys) {
        [IO.File]::WriteAllText((Join-Path $rootWork "$out/array_invalid_$case.lm2"), $invalidArrays[$case])
        & $l2exe "$out/array_invalid_$case.lm2" "$out/array_invalid_$case.lm1" *> "$out/array_invalid_$case.log"
        $rootEvidence.stages += @{name="array_invalid_$case";exit=$LASTEXITCODE;expected=1}
        if ($LASTEXITCODE -ne 1 -or (Test-Path "$out/array_invalid_$case.lm1")) { throw "Unsupported array $case accepted/published" }
    }
    & $l1trans 'l2src/tests/l2_message_root_driver.lm1' "$out/driver.c" *> "$out/driver.translate.log"
    Assert-RootExit 'driver_translate'
    $rootWrap = @('lmx_msg_runtime_new','lmx_msg_create','lmx_msg_find','lmx_msg_set_graph','lmx_msg_runtime_delete','lmx_branch_open_owned','lmx_node_new_owned','lmx_method_new_owned','lmx_int_new_owned','lmx_size_new_owned','lmx_chars_new_owned','lmx_array_new_positive_owned','malloc','free') | ForEach-Object { "-Wl,--wrap=$_" }
    Invoke-Gcc "$out/driver.c" "$out/driver.exe" "$out/driver.gcc.log" (@("$out/program.o", "$out/char_entry.o", "$out/array_entry.o", "$out/array_index.o", "$out/array_char_index.o", "$out/array_length.o", "$out/for_arrays.o", '-Werror') + $rootWrap)
    $rootEvidence.stages += @{name='driver_link_real_message'; exit=0}
    $rootObjects = @(Get-L2MessageObjects)
    if ($rootObjects.Count -ne 15) { throw 'Unexpected Message object set' }
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
    foreach ($mode in 0..45) {
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
        lm_own_alloc_fails: 1
        text: l2_m1(unit, "forced failure")
        c.printf("%d\n", text = 0)
        lm_own_alloc_fails: 0
'@ "1 1`n1 2`n1`n0 1`n1"
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
