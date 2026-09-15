# Self-build of the live L1 chain at the repository root (Mikhail 2026-09-15: the
# self-build's target is the root's l1src; one build root after the self-build).
#
# B0 is gcc of the root's committed lm1/build/l1trans.lm1.c. Pass 1: B0 regenerates
# the eight generated files of l1src/buildCore.lm1's lm_build_generate_all map into
# a stamped directory. B1 from pass 1's l1trans.lm1.c, pass 2; B2 from pass 2's,
# pass 3. Green is the fixed point (pass 3 == pass 2 byte for byte) and the
# committed generated C equal to that fixed point by git blob id (hash-object
# --path applies .gitattributes). B0's agreement with the committed C is reported
# and does not decide. Executables are compared by nothing: gcc output here is not
# byte-reproducible from identical C.
#
#   powershell -NoProfile -ExecutionPolicy Bypass -File l2src/run_self_build.ps1
param([string]$OutDir)
$ErrorActionPreference = 'Continue'
$repo = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
if (-not $OutDir) { $OutDir = Join-Path $repo ('build\self_build\' + (Get-Date -Format 'yyyyMMdd_HHmmss_fff')) }
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
foreach ($d in 'b0', 'b1', 'b2', 'pass1\lm1\build\l1src', 'pass2\lm1\build\l1src', 'pass3\lm1\build\l1src', 'logs') { New-Item -ItemType Directory -Force -Path (Join-Path $OutDir $d) | Out-Null }
Set-Location $repo
$started = Get-Date
$head = (git rev-parse HEAD) -join ''
$dirty = @(git status --porcelain -uno -- l1src lm1/build).Count
Write-Output ('self-build on ' + $head.Substring(0, 8) + '; tracked changes under l1src and lm1/build: ' + $dirty + '; evidence ' + $OutDir)
$script:red = @()

function Invoke-SelfBuildGcc([string]$Label, [string]$Include, [string]$Source, [string]$Exe) {
    $log = Join-Path $logs ($Label + '.gcc.log')
    cmd /c "gcc $flags -I . -I `"$Include`" -o `"$Exe`" `"$Source`" > `"$log`" 2>&1"
    $code = $LASTEXITCODE
    if ($code -ne 0 -or -not (Test-Path -LiteralPath $Exe)) { $script:red += ($Label + ' gcc exit ' + $code + ' (log ' + $log + ')') }
}
function Invoke-SelfBuildPass([string]$Label, [string]$Exe, [string]$Dir) {
    foreach ($e in $map) {
        $out = Join-Path $Dir ($e[1] -replace '/', '\')
        $log = Join-Path $logs ($Label + '.' + (Split-Path -Leaf $e[1]) + '.log')
        cmd /c "`"$Exe`" $($e[0]) `"$out`" > `"$log`" 2>&1"
        if ($LASTEXITCODE -ne 0 -or -not (Test-Path -LiteralPath $out)) { $script:red += ($Label + ' translate ' + $e[0] + ' exit ' + $LASTEXITCODE + ' (log ' + $log + ')') }
    }
}
function Get-SelfBuildBlob([string]$Committed, [string]$File) {
    if (-not (Test-Path -LiteralPath $File)) { return '' }
    return ((git hash-object ('--path=' + $Committed) -- $File) -join '')
}

$b0 = Join-Path $OutDir 'b0\l1trans.exe'
Invoke-SelfBuildGcc 'b0' 'lm1/build' 'lm1\build\l1trans.lm1.c' $b0
$p1 = Join-Path $OutDir 'pass1'; $p2 = Join-Path $OutDir 'pass2'; $p3 = Join-Path $OutDir 'pass3'
if (Test-Path -LiteralPath $b0) { Invoke-SelfBuildPass 'pass1' $b0 $p1 }
$b1 = Join-Path $OutDir 'b1\l1trans.exe'
if (Test-Path -LiteralPath (Join-Path $p1 'lm1\build\l1trans.lm1.c')) { Invoke-SelfBuildGcc 'b1' (Join-Path $p1 'lm1\build') (Join-Path $p1 'lm1\build\l1trans.lm1.c') $b1 }
if (Test-Path -LiteralPath $b1) { Invoke-SelfBuildPass 'pass2' $b1 $p2 }
$b2 = Join-Path $OutDir 'b2\l1trans.exe'
if (Test-Path -LiteralPath (Join-Path $p2 'lm1\build\l1trans.lm1.c')) { Invoke-SelfBuildGcc 'b2' (Join-Path $p2 'lm1\build') (Join-Path $p2 'lm1\build\l1trans.lm1.c') $b2 }
if (Test-Path -LiteralPath $b2) { Invoke-SelfBuildPass 'pass3' $b2 $p3 }

$fixed = 0; $committedEq = 0; $b0Eq = 0
foreach ($e in $map) {
    $rel = $e[1] -replace '/', '\'
    $f1 = Join-Path $p1 $rel; $f2 = Join-Path $p2 $rel; $f3 = Join-Path $p3 $rel
    $same23 = (Test-Path -LiteralPath $f2) -and (Test-Path -LiteralPath $f3) -and ((Get-FileHash -LiteralPath $f2).Hash -eq (Get-FileHash -LiteralPath $f3).Hash)
    if ($same23) { $fixed++ } else { $script:red += ('fixed point ' + $e[1] + ': pass 3 differs from pass 2') }
    $committedBlob = Get-SelfBuildBlob $e[1] $e[1]
    $fixedBlob = Get-SelfBuildBlob $e[1] $f2
    if ($committedBlob -and $committedBlob -eq $fixedBlob) { $committedEq++ } else {
        $line = ''
        if ((Test-Path -LiteralPath $e[1]) -and (Test-Path -LiteralPath $f2)) {
            $a = @(Get-Content -LiteralPath $e[1]); $b = @(Get-Content -LiteralPath $f2)
            for ($i = 0; $i -lt [Math]::Min($a.Count, $b.Count); $i++) { if ($a[$i] -ne $b[$i]) { $line = '; first difference at line ' + ($i + 1); break } }
            if (-not $line) { $line = '; ' + $a.Count + ' committed lines vs ' + $b.Count + ' regenerated' }
        }
        $script:red += ('committed ' + $e[1] + ' ' + $(if ($committedBlob) { $committedBlob.Substring(0, 8) } else { 'absent' }) + ' differs from the fixed point ' + $(if ($fixedBlob) { $fixedBlob.Substring(0, 8) } else { 'absent' }) + $line)
    }
    if ($committedBlob -and $committedBlob -eq (Get-SelfBuildBlob $e[1] $f1)) { $b0Eq++ }
}
$sec = [int]((Get-Date) - $started).TotalSeconds
Write-Output ('committed translator (B0) regenerates ' + $b0Eq + ' of 8 committed files unchanged (reported, not decisive)')
foreach ($r in $script:red) { Write-Output ('RED ' + $r) }
if ($script:red.Count) {
    Write-Output ('self-build FAIL: fixed point ' + $fixed + ' of 8 (pass 3 == pass 2), committed generated C ' + $committedEq + ' of 8 equal to the fixed point, in ' + $sec + 's; evidence ' + $OutDir)
    exit 1
}
Write-Output ('self-build PASS: fixed point 8 of 8 (pass 3 == pass 2), committed generated C 8 of 8 equal to the fixed point, in ' + $sec + 's; evidence ' + $OutDir)
exit 0
