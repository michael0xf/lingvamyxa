# L1 ifdef -> C99 #if/#elif/#else/#endif.
# CWD = repo root. Isolated under build\l1trans\<gen>.
$ErrorActionPreference = "Stop"
Set-Location (Join-Path $PSScriptRoot "..\..")

$gen = "gen0"
if ($env:L1_GEN -and $env:L1_GEN.Trim().Length -gt 0) { $gen = $env:L1_GEN.Trim() }
$l1trans = "build\l1trans\$gen\l1trans.exe"
$obj = "build\obj\l1trans\$gen"
$bin = "build\l1trans\$gen"
$log = Join-Path "build\l1trans\logs" $gen
$script:ifdefLog = Join-Path $log "ifdef.log"
$cflagsStr = "-std=c99 -Wall -Wextra -Wpedantic -I . -I lm1/build -Werror=incompatible-pointer-types -Werror=discarded-qualifiers -Werror=implicit-function-declaration -Werror=implicit-int"

New-Item -ItemType Directory -Force -Path $obj, $bin, $log | Out-Null
Set-Content -LiteralPath $script:ifdefLog -Value "$(Get-Date -Format o) ifdef start gen=$gen"

function Write-I([string]$m) { Add-Content -LiteralPath $script:ifdefLog -Value "$(Get-Date -Format o) $m" }

if (-not (Test-Path $l1trans)) { throw "missing $l1trans" }

function Translate([string]$name) {
    $src = "tests\l1\$name.lm1"
    $cpath = Join-Path $obj ($name + ".c")
    $cpathB = Join-Path $obj ($name + "_b.c")
    Write-I "BEGIN translate $src"
    & $l1trans $src $cpath
    if ($LASTEXITCODE -ne 0) { throw "translate failed $name $LASTEXITCODE" }
    & $l1trans $src $cpathB
    if ($LASTEXITCODE -ne 0) { throw "translate b failed $name" }
    $h1 = (Get-FileHash -LiteralPath $cpath).Hash
    $h2 = (Get-FileHash -LiteralPath $cpathB).Hash
    if ($h1 -ne $h2) { throw "C differ $name" }
    Write-I "C $name sha256=$h1"
    return $cpath
}

function Build-Run([string]$name, [string]$cpath, [string]$defs, [int]$expect) {
    $exepath = Join-Path $bin ($name + $defs.Replace("=", "").Replace(" ", "_") + ".exe")
    $gccLog = Join-Path $log ("gcc_" + $name + $defs.Replace("=", "").Replace(" ", "_") + ".log")
    $runOut = Join-Path $log ("run_" + $name + $defs.Replace("=", "").Replace(" ", "_") + ".stdout")
    $runErr = Join-Path $log ("run_" + $name + $defs.Replace("=", "").Replace(" ", "_") + ".stderr")
    $defPart = ""
    if ($defs.Length -gt 0) { $defPart = " " + $defs }
    Write-I "CMD gcc $cflagsStr$defPart -o $exepath $cpath"
    cmd /c "gcc $cflagsStr$defPart -o $exepath $cpath > $gccLog 2>&1"
    if ($LASTEXITCODE -ne 0) {
        Get-Content -LiteralPath $gccLog | Select-Object -Last 30
        throw "gcc failed $name defs=$defs"
    }
    cmd /c "$exepath > $runOut 2> $runErr"
    if ($LASTEXITCODE -ne $expect) { throw "run $name defs=$defs exit $LASTEXITCODE want $expect" }
    Write-I "EXIT run $name defs=$defs $LASTEXITCODE"
}

$flagC = Translate "ifdef_flag"
$flagText = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $flagC))
if ($flagText.IndexOf("#if FLAG") -lt 0) { throw "missing #if FLAG" }
if ($flagText.IndexOf("#ifdef FLAG") -ge 0) { throw "used #ifdef FLAG instead of #if" }
if ($flagText.IndexOf("#else") -lt 0 -or $flagText.IndexOf("#endif") -lt 0) { throw "missing #else/#endif" }
Build-Run "ifdef_flag" $flagC "" 0
Build-Run "ifdef_flag" $flagC "-DFLAG=0" 0
Build-Run "ifdef_flag" $flagC "-DFLAG=1" 1

$multiC = Translate "ifdef_multi"
$multiText = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $multiC))
if ($multiText.IndexOf("#if A") -lt 0 -or $multiText.IndexOf("#elif B") -lt 0) { throw "missing #if A/#elif B" }
if ($multiText.IndexOf("#if 0") -lt 0) { throw "missing inactive #if 0" }
Build-Run "ifdef_multi" $multiC "-DA=1 -DB=1" 1
Build-Run "ifdef_multi" $multiC "-DB=1" 2
Build-Run "ifdef_multi" $multiC "" 3

$nestC = Translate "ifdef_nested"
$nestText = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $nestC))
if ($nestText.IndexOf("defined(FLAG) && !defined(OTHER)") -lt 0) { throw "condition rewritten" }
if ($nestText.IndexOf("FLAG == 2 || FLAG == 3") -lt 0) { throw "missing quoted || condition" }
Build-Run "ifdef_nested" $nestC "-DFLAG" 1
Build-Run "ifdef_nested" $nestC "-DFLAG=2 -DOTHER" 2
Build-Run "ifdef_nested" $nestC "" 0

$ctxC = Translate "ifdef_ctx"
$ctxText = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $ctxC))
if ($ctxText.IndexOf("#include <stdio.h>") -lt 0) { throw "missing include in FLAG branch" }
Build-Run "ifdef_ctx" $ctxC "-DFLAG=1" 4
Build-Run "ifdef_ctx" $ctxC "" 5

function Negative([string]$name, [string]$diag) {
    $src = "tests\l1\$name.lm1"
    $cpath = Join-Path $obj ($name + ".c")
    $err = Join-Path $log ($name + ".err")
    Write-I "BEGIN negative $src"
    if (Test-Path -LiteralPath $cpath) { Remove-Item -LiteralPath $cpath }
    cmd /c "$l1trans $src $cpath > $log\$name.stdout 2> $err"
    if ($LASTEXITCODE -eq 0) { throw "expected translate failure: $src" }
    $errText = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $err))
    if ($errText.IndexOf($diag) -lt 0) { throw "missing diagnostic '$diag': $errText" }
    if (Test-Path -LiteralPath $cpath) { throw "failed translate created $cpath" }
    Write-I "EXIT negative $src $LASTEXITCODE diagnostic ok"
}

Negative "invalid_ifdef_dup_default" "ifdef receiver has duplicate default branch"
Negative "invalid_ifdef_after_default" "ifdef default branch must be last"
Negative "invalid_ifdef_nonframe" "ifdef receiver expects condition/default frames"
Negative "invalid_ifdef_empty_cond" "ifdef branch expects a non-empty condition"
Negative "invalid_ifdef_end_outer" "end target does not match close target"
Negative "invalid_ifdef_end_branch" "end target does not match close target"
Negative "invalid_ifdef_end_count" "end trailer expects exactly one target name"
# Root gen0 binary may still be an older P0; gen2 has empty-colon P0.
if ($gen -eq "gen0") {
    Negative "invalid_ifdef_end_empty" "end target does not match close target"
} else {
    Negative "invalid_ifdef_end_empty" "empty colon Frame is not allowed"
}
Negative "invalid_lm1_ifdef_l3" "reserved L1 name"

Write-I "ifdef ok"
Write-Output "l1trans $gen ifdef ok"
