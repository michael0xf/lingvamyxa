# One private translator build, one Message object set, one generated program.
param([string]$CoreCommit = 'c557908')
$ErrorActionPreference = 'Stop'
$rootBaseline = Split-Path -Parent $PSScriptRoot
$rootRepo = Split-Path -Parent (Split-Path -Parent $rootBaseline)
$rootCompiler = Join-Path $rootBaseline 'build/l1trans/gen2/l1trans.exe'
$rootPin = '65D5A5ED127CA1BAEBDD1D500A5B74CEEA63EC1985EAC52EDEF28EFEB261C936'
if ((Get-FileHash -LiteralPath $rootCompiler).Hash -ne $rootPin) { throw 'Stable compiler pin mismatch' }
$rootRun = Join-Path $rootRepo ('build/codex/l2_message_root/' + (Get-Date -Format 'yyyyMMdd_HHmmss_fff') + '_' + [guid]::NewGuid().ToString('N').Substring(0,8))
$rootSnapshot = Join-Path $rootRun 'source'
New-Item -ItemType Directory -Path $rootSnapshot -Force | Out-Null
$rootOwned = @('l2trans.lm1', 'run_l2trans.ps1', 'tests/l2_message_root_driver.lm1', 'lmx_value_owned.h.lm1', 'lmx_value_owned.lm1')
$rootEvidence = [ordered]@{result='RUNNING'; compiler=$rootCompiler; compilerSHA256=$rootPin; owned=@{}; stages=@()}
$rootEvidence.runnerSHA256 = (Get-FileHash -LiteralPath $PSCommandPath).Hash
$rootOldLocation = Get-Location
function Assert-RootExit([string]$Stage) {
    $rootEvidence.stages += @{name=$Stage; exit=$LASTEXITCODE}
    if ($LASTEXITCODE -ne 0) { throw "$Stage exit $LASTEXITCODE" }
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
    if ($rootL1 -notmatch 'fn: l2_program_entry' -or $rootL1 -notmatch 'lmx_msg_set_graph\(process_message, unit\)' -or $rootL1 -notmatch 'lmx_msg_runtime_delete\(process_runtime\)') { throw 'Missing generated entry lifecycle' }
    if ($rootL1 -notmatch 'lmx_branch_open_owned' -or $rootL1 -match 'lmx_branch_open\(|lmx_branch_child\(') { throw 'Generated unit still uses legacy branch admission/access' }
    if ($rootL1 -notmatch 'lmx_node_new_owned' -or $rootL1 -notmatch 'lmx_method_new_owned' -or $rootL1 -match 'c\.malloc\(c\.sizeof\(c\.Lmx(Method)?\)\)|lmx_range_register\(|lmx_classify\(leaf') { throw 'Root/METHOD storage still uses raw or global admission' }
    & $l1trans "$out/program.lm1" "$out/program.c" *> "$out/program.c.log"
    Assert-RootExit 'L1_to_C'
    $rootObjects = @(Get-L2MessageObjects)
    & gcc @cflags -I "$out/message_support/headers" -Dmain=l2_generated_main -c "$out/program.c" -o "$out/program.o" *> "$out/program.o.log"
    Assert-RootExit 'program_object'
    & $l1trans 'l2src/tests/l2_message_root_driver.lm1' "$out/driver.c" *> "$out/driver.translate.log"
    Assert-RootExit 'driver_translate'
    $rootWrap = @('lmx_msg_runtime_new','lmx_msg_create','lmx_msg_find','lmx_msg_set_graph','lmx_msg_runtime_delete','lmx_branch_open_owned','lmx_node_new_owned','lmx_method_new_owned','lmx_int_new_owned','lmx_size_new_owned','malloc','free') | ForEach-Object { "-Wl,--wrap=$_" }
    Invoke-Gcc "$out/driver.c" "$out/driver.exe" "$out/driver.gcc.log" (@("$out/program.o", '-Werror') + $rootWrap)
    $rootEvidence.stages += @{name='driver_link_real_message'; exit=0}
    $rootObjects = @(Get-L2MessageObjects)
    if ($rootObjects.Count -ne 13) { throw 'Unexpected Message object set' }
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
    foreach ($mode in 0..17) {
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
    if ([regex]::Matches($rootNested, 'lmx_branch_open_owned\(').Count -ne 2 -or $rootNested -match 'lmx_branch_child\(') { throw 'Nested branches not on owned/proven-layout path' }
    & $l1trans "$out/nested.lm1" "$out/nested.c" *> "$out/nested.c.log"
    Assert-RootExit 'nested_L1_to_C'
    Invoke-Gcc "$out/nested.c" "$out/nested.exe" "$out/nested.gcc.log"
    & "$out/nested.exe" *> "$out/nested.run.log"
    Assert-RootExit 'nested_run'
    if ((Get-Content -LiteralPath "$out/nested.run.log" -Raw).Replace("`r`n", "`n").Trim() -ne "0`n9") { throw 'Nested dirty-only field history changed' }
    foreach ($obj in $rootObjects) {
        if ((Get-FileHash -LiteralPath $obj).Hash -ne $rootBefore[$obj]) { throw "Nested case rebuilt cached object: $obj" }
    }
    Write-Output 'root entry nested branch PASS (same Message object cache)'
    # Existing regressions share this translator and the same support objects.
    function Invoke-RootPrimitiveCase([string]$stem, [string]$body, [string]$expected) {
        & $l2exe "l2src/tests/$stem.lm2" "$out/$stem.lm1" *> "$out/$stem.translate.log"
        Assert-RootExit "${stem}_L2_to_L1"
        $text = Get-Content -LiteralPath "$out/$stem.lm1" -Raw
        if ($text -match '\blmx_(int|size)_(init|take|value|store)\(') { throw "$stem retains legacy primitive operations" }
        if ($body) {
            $tail = "`n        return: 0`n    end: main`nend: external`n"
            $text = New-L2DriveText $text ($body + $tail)
            [IO.File]::WriteAllText((Join-Path (Get-Location) "$out/$stem.lm1"), $text)
        }
        & $l1trans "$out/$stem.lm1" "$out/$stem.c" *> "$out/$stem.c.log"
        Assert-RootExit "${stem}_L1_to_C"
        $cText = Get-Content -LiteralPath "$out/$stem.c" -Raw
        if ($cText -match '\blmx_(int|size)_pool\b') { throw "$stem includes an unnecessary legacy primitive pool" }
        Invoke-Gcc "$out/$stem.c" "$out/$stem.exe" "$out/$stem.gcc.log"
        & "$out/$stem.exe" *> "$out/$stem.run.log"
        Assert-RootExit "${stem}_run"
        $actual = (Get-Content -LiteralPath "$out/$stem.run.log" -Raw).Replace("`r`n", "`n").Trim()
        if ($actual -ne $expected) { throw "$stem output '$actual' expected '$expected'" }
        foreach ($obj in $rootObjects) {
            if ((Get-FileHash -LiteralPath $obj).Hash -ne $rootBefore[$obj]) { throw "$stem rebuilt support object: $obj" }
        }
        Write-Output "$stem PASS (same translator and Message object cache)"
    }
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
        @: void source lmx_size_take()
        if: source = 0
            return: 2
        if: lmx_size_store(source, 9U) != 0
            return: 3
        l2_m0(unit, lmx_size_value(source))
        leaf: lmx_branch_child(unit, 0U)
        c.printf("%zu %zu\n", lmx_size_value(source), lmx_size_value(leaf\data))
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
