# One private translator build, one Message object set, one generated program.
param([string]$CoreCommit = 'be4bbdd')
$ErrorActionPreference = 'Stop'
$rootBaseline = Split-Path -Parent $PSScriptRoot
$rootRepo = Split-Path -Parent (Split-Path -Parent $rootBaseline)
$rootCompiler = Join-Path $rootBaseline 'build/l1trans/gen2/l1trans.exe'
$rootPin = '65D5A5ED127CA1BAEBDD1D500A5B74CEEA63EC1985EAC52EDEF28EFEB261C936'
if ((Get-FileHash -LiteralPath $rootCompiler).Hash -ne $rootPin) { throw 'Stable compiler pin mismatch' }
$rootRun = Join-Path $rootRepo ('build/codex/l2_message_root/' + (Get-Date -Format 'yyyyMMdd_HHmmss_fff') + '_' + [guid]::NewGuid().ToString('N').Substring(0,8))
$rootSnapshot = Join-Path $rootRun 'source'
New-Item -ItemType Directory -Path $rootSnapshot -Force | Out-Null
$rootOwned = @('l2trans.lm1', 'run_l2trans.ps1', 'tests/l2_message_root_driver.lm1')
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
    & $l1trans "$out/program.lm1" "$out/program.c" *> "$out/program.c.log"
    Assert-RootExit 'L1_to_C'
    $rootObjects = @(Get-L2MessageObjects)
    & gcc @cflags -I "$out/message_support/headers" -Dmain=l2_generated_main -c "$out/program.c" -o "$out/program.o" *> "$out/program.o.log"
    Assert-RootExit 'program_object'
    & $l1trans 'l2src/tests/l2_message_root_driver.lm1' "$out/driver.c" *> "$out/driver.translate.log"
    Assert-RootExit 'driver_translate'
    $rootWrap = @('lmx_msg_runtime_new','lmx_msg_create','lmx_msg_find','lmx_msg_set_graph','lmx_msg_runtime_delete','malloc') | ForEach-Object { "-Wl,--wrap=$_" }
    Invoke-Gcc "$out/driver.c" "$out/driver.exe" "$out/driver.gcc.log" (@("$out/program.o", '-Werror') + $rootWrap)
    $rootEvidence.stages += @{name='driver_link_real_message'; exit=0}
    $rootObjects = @(Get-L2MessageObjects)
    if ($rootObjects.Count -ne 11) { throw 'Unexpected Message object set' }
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
    foreach ($mode in 0..5) {
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
