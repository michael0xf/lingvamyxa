# Public-API L1 integration test against an immutable private source snapshot.
# It never compiles Grok's active working files or writes shared build outputs.
#
# Revived 2026-09-14 (d6): the runner hard-coded the file and module list of
# core 57b590f4, so any newer core failed at the `message` stage
# ("cannot read import l2src/lmx_msg_liveness.h.lm1"). The snapshot is now the
# whole stg/l1_baseline/l2src subtree of the selected revision, and the
# modules are every lmx_*.lm1 with a matching .h.lm1 in that revision: the
# production runtime run_model_scenario36.ps1 builds. This is the model's
# section 34 (family handoff) coverage.
param(
    [string]$CoreCommit = 'HEAD',
    [ValidateNotNullOrEmpty()][string[]]$Optimization = @('O2'),
    [ValidateSet('Family', 'UnrootedAdopt')][string]$Scenario = 'Family'
)
$ErrorActionPreference = 'Stop'
# powershell -File passes "-Optimization O0,O2" as the single string "O0,O2",
# which a ValidateSet rejected; split it and accept only O0 and O2.
$requested = @($Optimization | ForEach-Object { $_ -split ',' } | ForEach-Object { $_.Trim() } | Where-Object { $_ })
if ($requested.Count -eq 0) { throw '-Optimization needs O0, O2 or O0,O2.' }
foreach ($level in $requested) {
    if (@('O0', 'O2') -notcontains $level) { throw "-Optimization '$level' is not O0 or O2." }
}
$Optimization = $requested
$baseline = Split-Path -Parent $PSScriptRoot
$repo = Split-Path -Parent (Split-Path -Parent $baseline)
$compiler = Join-Path $baseline 'build/l1trans/gen2/l1trans.exe'
$pin = (Get-Content -LiteralPath (Join-Path $PSScriptRoot 'L1_PIN.txt') -TotalCount 1).Trim()
if ($pin -notmatch '^[0-9A-F]{64}$') { throw "L1_PIN.txt must hold one 64-hex SHA256, got 'pin=$pin'" }
if ((Get-FileHash -LiteralPath $compiler).Hash -ne $pin) { throw 'Stable compiler pin changed.' }
$testSource = Join-Path $PSScriptRoot 'tests/lmx_msg_family_handoff_selftest.lm1'
if ($Scenario -eq 'UnrootedAdopt') {
    $testSource = Join-Path $PSScriptRoot 'tests/lmx_msg_adopt_unrooted_selftest.lm1'
}
$testHash = (Get-FileHash -LiteralPath $testSource).Hash
$runnerHash = (Get-FileHash -LiteralPath $PSCommandPath).Hash
$runId = (Get-Date -Format 'yyyyMMdd_HHmmss_fff') + '_' + [guid]::NewGuid().ToString('N').Substring(0,8)
$run = Join-Path $repo "build/codex/msg_family_handoff/$runId"
$snapshot = Join-Path $run 'source'
$headers = Join-Path $run 'headers'
New-Item -ItemType Directory -Path $snapshot, (Join-Path $headers 'l2src') -Force | Out-Null
$stageWorkingDir = $repo
$evidence = [ordered]@{ compiler = $compiler; compilerSHA256 = $pin; requestedCore = $CoreCommit; optimization = $Optimization; scenario = $Scenario; testSource = $testSource; testSHA256 = $testHash; runnerSHA256 = $runnerHash; stages = @(); result = 'RUNNING' }
function Invoke-FamilyStage([string]$Name, [string]$Tool, [string[]]$NativeArgs) {
    $quoted = ($NativeArgs | ForEach-Object { '"' + $_ + '"' }) -join ' '
    $stdout = Join-Path $run "$Name.stdout.txt"
    $stderr = Join-Path $run "$Name.stderr.txt"
    # Windows PowerShell rejects an explicitly empty ArgumentList.
    $argumentOption = @{}
    if ($NativeArgs.Count -gt 0) { $argumentOption.ArgumentList = $quoted }
    $p = Start-Process -FilePath $Tool @argumentOption -WorkingDirectory $stageWorkingDir `
        -WindowStyle Hidden -Wait -PassThru -RedirectStandardOutput $stdout -RedirectStandardError $stderr
    $evidence.stages += [ordered]@{ name = $Name; tool = $Tool; arguments = $NativeArgs; exit = $p.ExitCode }
    if ($p.ExitCode -ne 0) {
        Get-Content -LiteralPath $stdout, $stderr
        throw "$Name failed with exit $($p.ExitCode)"
    }
}
try {
    $git = (Get-Command git -ErrorAction Stop).Source
    Invoke-FamilyStage 'resolve_core' $git @('rev-parse', '--verify', '--end-of-options', "$CoreCommit^{commit}")
    $revision = (Get-Content -LiteralPath (Join-Path $run 'resolve_core.stdout.txt') -Raw).Trim()
    if ($revision -notmatch '^[0-9a-f]{40}$') { throw 'Expected a resolved full commit hash.' }
    $evidence.coreCommit = $revision
    # Inspect the selected immutable revision, never the live checkout.
    $subtree = 'stg/l1_baseline/l2src'
    Invoke-FamilyStage 'list_core' $git @('ls-tree', '--name-only', $revision, '--', "$subtree/")
    $listed = @(Get-Content -LiteralPath (Join-Path $run 'list_core.stdout.txt') | Where-Object { $_ } | ForEach-Object { ($_ -split '/')[-1] })
    $fixed = @('lmx.h', 'lmx_message.h', 'lmx_message.lm1', 'lmx_message_host.h', 'lmx_message_host.c', 'lmx_message_exec.h', 'lmx_message_exec.c')
    foreach ($file in $fixed) {
        if ($listed -notcontains $file) { throw "Selected core has no $subtree/$file." }
    }
    # The production runtime: every module with both a header and a body.
    $moduleNames = @($listed | Where-Object { $_ -match '^lmx_[a-z0-9_]+\.h\.lm1$' } | ForEach-Object { $_.Substring(0, $_.Length - '.h.lm1'.Length) } |
        Where-Object { $listed -contains "$_.lm1" } | Sort-Object)
    if ($moduleNames.Count -eq 0) { throw 'Selected core has no runtime modules.' }
    $evidence.modules = $moduleNames
    $archive = Join-Path $run 'core.zip'
    # l1src and lm1/build: l2trans.lm1 predefs l1src/parser.lm1 and its C
    # includes lm1/build, and the L2 runtime units need l2trans.
    Invoke-FamilyStage 'archive_core' $git @('archive', '--format=zip', "--output=$archive", $revision, '--', $subtree, 'stg/l1_baseline/l1src', 'stg/l1_baseline/lm1/build')
    Expand-Archive -LiteralPath $archive -DestinationPath $snapshot
    $stageWorkingDir = Join-Path $snapshot 'stg/l1_baseline'
    $files = @($fixed) + @($moduleNames | ForEach-Object { "$_.h.lm1"; "$_.lm1" })
    $coreHashes = @{}
    foreach ($file in $files) {
        $path = Join-Path $stageWorkingDir "l2src/$file"
        $coreHashes[$path] = (Get-FileHash -LiteralPath $path).Hash
    }
    $evidence.coreSources = $coreHashes
    $modules = @()
    foreach ($name in $moduleNames) {
        Invoke-FamilyStage "header_$name" $compiler @("l2src/$name.h.lm1", (Join-Path $headers "l2src/$name.lm1.h"))
        $module = Join-Path $run "$name.c"
        Invoke-FamilyStage "module_$name" $compiler @("l2src/$name.lm1", $module)
        $modules += $module
    }
    $messageC = Join-Path $run 'lmx_message.c'
    Invoke-FamilyStage 'message' $compiler @('l2src/lmx_message.lm1', $messageC)
    $testC = Join-Path $run 'family_handoff.c'
    Invoke-FamilyStage 'test' $compiler @($testSource, $testC)
    $gcc = (Get-Command gcc -ErrorAction Stop).Source
    Invoke-FamilyStage 'gcc_version' $gcc @('--version')
    . (Join-Path $PSScriptRoot 'l2units_build.ps1')
    $flags = @('-std=c99', '-Wall', '-Wextra', '-Wpedantic', '-Werror=incompatible-pointer-types',
        '-Werror=discarded-qualifiers', '-Werror=implicit-function-declaration', '-Werror=implicit-int',
        '-I', $headers, '-I', $stageWorkingDir)
    $native = @((Join-Path $stageWorkingDir 'l2src/lmx_message_host.c'), (Join-Path $stageWorkingDir 'l2src/lmx_message_exec.c'))
    foreach ($level in @($Optimization | ForEach-Object { $_.ToUpperInvariant() } | Select-Object -Unique)) {
        $exe = Join-Path $run "family_handoff_$level.exe"
        $testObj = Join-Path $run "family_handoff_$level.o"
        Invoke-FamilyStage "compile_test_$level" $gcc ($flags + @('-Werror', "-$level", '-c', $testC, '-o', $testObj))
        # Stage 3c-2a: the production runtime includes the L2 runtime units of
        # the snapshot (l2units_build.ps1; today lmx_sched_record.lm2).
        Push-Location $stageWorkingDir
        $unitObjs = @(Build-L2RuntimeUnits -L1Trans $compiler -Out (Join-Path $run "l2units_$level") -IncludeDirs @($headers) -CFlags "-std=c99 -Wall -Wextra -Wpedantic -$level -I ." -Gcc $gcc)
        Pop-Location
        $evidence.stages += [ordered]@{ name = "l2units_$level"; objects = $unitObjs }
        Invoke-FamilyStage "compile_$level" $gcc ($flags + @("-$level", $testObj, $messageC) + $modules + $native + $unitObjs + @('-Wl,--wrap=free', '-o', $exe))
        Invoke-FamilyStage "run_$level" $exe @()
        $result = Get-Content -LiteralPath (Join-Path $run "run_$level.stdout.txt") -Raw
        $expected = '(?m)^family handoff checks=62 failures=0 watched_frees=4\s*$'
        if ($Scenario -eq 'UnrootedAdopt') {
            $expected = '(?m)^adopt unrooted checks=\d+ failures=0 owned_frees=2 retained_interior=PASS\s*$'
        }
        if ($result -notmatch $expected) { throw "Unexpected test result: $result" }
        Write-Output "$level $($result.Trim())"
    }
    foreach ($path in $coreHashes.Keys) {
        if ((Get-FileHash -LiteralPath $path).Hash -ne $coreHashes[$path]) { throw "Snapshot source changed: $path" }
    }
    if ((Get-FileHash -LiteralPath $compiler).Hash -ne $pin) { throw 'Compiler changed during verification.' }
    if ((Get-FileHash -LiteralPath $testSource).Hash -ne $testHash -or (Get-FileHash -LiteralPath $PSCommandPath).Hash -ne $runnerHash) { throw 'Test or runner changed during verification.' }
    $evidence.artifacts = @{}
    Get-ChildItem -LiteralPath $run -File | ForEach-Object { $evidence.artifacts[$_.Name] = (Get-FileHash -LiteralPath $_.FullName).Hash }
    $evidence.result = 'PASS'
} catch {
    $evidence.result = 'FAIL'
    $evidence.error = $_.Exception.Message
    throw
} finally {
    $evidence | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath (Join-Path $run 'evidence.json') -Encoding UTF8
    Write-Output "Evidence: $run"
}
