# Slice equality (Mikhail 2026-09-15: stg/l1_baseline is refreshed from the root now;
# the two build roots stay until the self-build is achieved).
#
# 1. Every tracked file under stg/l1_baseline/l1src equals the root's l1src file of
#    the same relative path, by git blob id, and neither side has a tracked file the
#    other lacks (6f 2026-09-15: every tracked file, no exclusions; a stray file is red).
# 2. The slice's committed generated C equals its regeneration: gcc of the ROOT's
#    committed lm1/build/l1trans.lm1.c (the committed fixed point once
#    run_self_build is green) regenerates the eight files of buildCore.lm1's map
#    from stg/l1_baseline (sources given relative to that directory, as buildCore
#    runs them); each must match stg/l1_baseline/lm1/build by blob id.
#
#   powershell -NoProfile -ExecutionPolicy Bypass -File stg/l1_baseline/l2src/run_slice_equal.ps1
param([string]$OutDir)
$ErrorActionPreference = 'Continue'
$repo = (Resolve-Path (Join-Path $PSScriptRoot '..\..\..')).Path
$slice = Join-Path $repo 'stg\l1_baseline'
if (-not $OutDir) { $OutDir = Join-Path $repo ('build\slice_equal\' + (Get-Date -Format 'yyyyMMdd_HHmmss_fff')) }
$map = @(
    @('l1src/p0.h.lm1', 'lm1/build/l1src/p0.lm1.h'),
    @('l1src/own.lm1', 'lm1/build/own.lm1.c'),
    @('l1src/parser.lm1', 'lm1/build/parser.lm1.c'),
    @('l1src/l1trans.lm1', 'lm1/build/l1trans.lm1.c'),
    @('l1src/printTree.lm1', 'lm1/build/printTree.lm1.c'),
    @('l1src/finalize.lm1', 'lm1/build/finalize.lm1.c'),
    @('l1src/make.lm1', 'lm1/build/make.lm1.c'),
    @('l1src/buildCore.lm1', 'lm1/build/buildCore.lm1.c')
)
$flags = '-std=c99 -Wall -Wextra -Wpedantic -Werror=incompatible-pointer-types -Werror=discarded-qualifiers -Werror=implicit-function-declaration -Werror=implicit-int'
foreach ($k in 'LM_TRANS_REGISTRY', 'LM_TRANS_REGISTRY_VIEW', 'LM_P0_REGISTRY', 'LM_P0_COMPARE_REGISTRY') { Remove-Item ('Env:' + $k) -ErrorAction SilentlyContinue }
$logs = Join-Path $OutDir 'logs'
foreach ($d in 'bin', 'regen\lm1\build\l1src', 'logs') { New-Item -ItemType Directory -Force -Path (Join-Path $OutDir $d) | Out-Null }
$started = Get-Date
Set-Location $repo
$head = (git rev-parse HEAD) -join ''
Write-Output ('slice equality on ' + $head.Substring(0, 8) + '; evidence ' + $OutDir)
$red = @()
function Get-SliceShort([string]$Blob) { if ($Blob) { return $Blob.Substring(0, 8) } else { return 'absent' } }

# 1. Source equality over every tracked file (working tree, hashed as git would store them).
function Get-SliceTracked([string]$Prefix) {
    $h = @{}
    foreach ($p in @(git ls-files -- $Prefix)) {
        $h[$p.Substring($Prefix.Length)] = if (Test-Path -LiteralPath $p) { (git hash-object ('--path=' + $p) -- $p) -join '' } else { '' }
    }
    return $h
}
$rootSrc = Get-SliceTracked 'l1src/'
$sliceSrc = Get-SliceTracked 'stg/l1_baseline/l1src/'
$srcSame = 0
foreach ($k in @($rootSrc.Keys + $sliceSrc.Keys | Sort-Object -Unique)) {
    if (-not $sliceSrc.ContainsKey($k)) { $red += ('source l1src/' + $k + ' is tracked at the root and not in the slice'); continue }
    if (-not $rootSrc.ContainsKey($k)) { $red += ('source l1src/' + $k + ' is tracked in the slice and not at the root'); continue }
    if ($rootSrc[$k] -eq $sliceSrc[$k] -and $rootSrc[$k]) { $srcSame++ } else { $red += ('source l1src/' + $k + ' differs: root ' + (Get-SliceShort $rootSrc[$k]) + ', slice ' + (Get-SliceShort $sliceSrc[$k])) }
}
$srcTotal = @($rootSrc.Keys + $sliceSrc.Keys | Sort-Object -Unique).Count

# 2. The slice's generated C against its regeneration by the root's committed translator.
$exe = Join-Path $OutDir 'bin\l1trans.exe'
cmd /c "gcc $flags -I . -I lm1/build -o `"$exe`" lm1\build\l1trans.lm1.c > `"$logs\root_l1trans.gcc.log`" 2>&1"
if ($LASTEXITCODE -ne 0 -or -not (Test-Path -LiteralPath $exe)) { $red += ('gcc of the root lm1/build/l1trans.lm1.c exit ' + $LASTEXITCODE) }
$genSame = 0
if (Test-Path -LiteralPath $exe) {
    Set-Location $slice
    foreach ($e in $map) {
        $out = Join-Path $OutDir ('regen\' + ($e[1] -replace '/', '\'))
        cmd /c "`"$exe`" $($e[0]) `"$out`" > `"$logs\$(Split-Path -Leaf $e[1]).log`" 2>&1"
        if ($LASTEXITCODE -ne 0 -or -not (Test-Path -LiteralPath $out)) { $red += ('translate stg/l1_baseline/' + $e[0] + ' exit ' + $LASTEXITCODE); continue }
        $committed = 'stg/l1_baseline/' + $e[1]
        $cBlob = (git -C $repo hash-object ('--path=' + $committed) -- (Join-Path $repo ($committed -replace '/', '\'))) -join ''
        $rBlob = (git -C $repo hash-object ('--path=' + $committed) -- $out) -join ''
        if ($cBlob -and $cBlob -eq $rBlob) { $genSame++ } else { $red += ('generated ' + $committed + ' ' + (Get-SliceShort $cBlob) + ' differs from its regeneration ' + (Get-SliceShort $rBlob)) }
    }
    Set-Location $repo
}
$sec = [int]((Get-Date) - $started).TotalSeconds
foreach ($r in $red) { Write-Output ('RED ' + $r) }
if ($red.Count) {
    Write-Output ('slice equal FAIL: sources ' + $srcSame + ' of ' + $srcTotal + ' equal to the root, generated C ' + $genSame + ' of 8 equal to its regeneration, in ' + $sec + 's; evidence ' + $OutDir)
    exit 1
}
Write-Output ('slice equal PASS: sources ' + $srcSame + ' of ' + $srcTotal + ' equal to the root, generated C 8 of 8 equal to its regeneration, in ' + $sec + 's; evidence ' + $OutDir)
exit 0
