# L1 switch/case/default and labels+goto -> C99.
# CWD = repo root. Isolated under build\l1trans\<gen>.
$ErrorActionPreference = "Stop"
Set-Location (Join-Path $PSScriptRoot "..\..")

$gen = "gen0"
if ($env:L1_GEN -and $env:L1_GEN.Trim().Length -gt 0) { $gen = $env:L1_GEN.Trim() }
$l1trans = "build\l1trans\$gen\l1trans.exe"
$obj = "build\obj\l1trans\$gen"
$bin = "build\l1trans\$gen"
$log = Join-Path "build\l1trans\logs" $gen
$script:ctlLog = Join-Path $log "control.log"
$cflagsStr = "-std=c99 -Wall -Wextra -Wpedantic -pedantic-errors -I . -Werror=incompatible-pointer-types -Werror=discarded-qualifiers -Werror=implicit-function-declaration -Werror=implicit-int"

New-Item -ItemType Directory -Force -Path $obj, $bin, $log | Out-Null
Set-Content -LiteralPath $script:ctlLog -Value "$(Get-Date -Format o) control start gen=$gen"

function Write-C([string]$m) { Add-Content -LiteralPath $script:ctlLog -Value "$(Get-Date -Format o) $m" }

if (-not (Test-Path $l1trans)) { throw "missing $l1trans" }

function Translate([string]$name) {
    $src = "tests\l1\$name.lm1"
    $cpath = Join-Path $obj ($name + ".c")
    $cpathB = Join-Path $obj ($name + "_b.c")
    Write-C "BEGIN translate $src"
    & $l1trans $src $cpath
    if ($LASTEXITCODE -ne 0) { throw "translate failed $name $LASTEXITCODE" }
    & $l1trans $src $cpathB
    if ($LASTEXITCODE -ne 0) { throw "translate b failed $name" }
    $h1 = (Get-FileHash -LiteralPath $cpath).Hash
    $h2 = (Get-FileHash -LiteralPath $cpathB).Hash
    if ($h1 -ne $h2) { throw "C differ $name" }
    Write-C "C $name sha256=$h1"
    return $cpath
}

function Build-Run([string]$name, [string]$cpath, [int]$expect) {
    $exepath = Join-Path $bin ($name + ".exe")
    $gccLog = Join-Path $log ("gcc_" + $name + ".log")
    $runOut = Join-Path $log ("run_" + $name + ".stdout")
    $runErr = Join-Path $log ("run_" + $name + ".stderr")
    Write-C "CMD gcc $cflagsStr -o $exepath $cpath"
    cmd /c "gcc $cflagsStr -o $exepath $cpath > $gccLog 2>&1"
    if ($LASTEXITCODE -ne 0) {
        Get-Content -LiteralPath $gccLog | Select-Object -Last 30
        throw "gcc failed $name"
    }
    cmd /c "$exepath > $runOut 2> $runErr"
    if ($LASTEXITCODE -ne $expect) { throw "run $name exit $LASTEXITCODE want $expect" }
    Write-C "EXIT run $name $LASTEXITCODE"
}

function Negative([string]$name, [string]$diag) {
    $src = "tests\l1\$name.lm1"
    $cpath = Join-Path $obj ($name + ".c")
    $err = Join-Path $log ($name + ".err")
    Write-C "BEGIN negative $src"
    if (Test-Path -LiteralPath $cpath) { Remove-Item -LiteralPath $cpath }
    cmd /c "$l1trans $src $cpath > $log\$name.stdout 2> $err"
    if ($LASTEXITCODE -eq 0) { throw "expected translate failure: $src" }
    $errText = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $err))
    if ($errText.IndexOf($diag) -lt 0) { throw "missing diagnostic '$diag': $errText" }
    if (Test-Path -LiteralPath $cpath) { throw "failed translate created $cpath" }
    Write-C "EXIT negative $src $LASTEXITCODE diagnostic ok"
}

$swC = Translate "control_switch"
$swT = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $swC))
if ($swT.IndexOf("switch (x)") -lt 0) { throw "missing switch (x)" }
if ($swT.IndexOf("case 1: ;") -lt 0) { throw "missing case 1 null statement" }
if ($swT.IndexOf("case 2: ;") -lt 0) { throw "missing case 2 null statement" }
if ($swT.IndexOf("default: ;") -lt 0) { throw "missing default null statement" }
if ($swT.IndexOf("break;") -lt 0) { throw "missing break" }
$idx1 = $swT.IndexOf("case 1:")
$idx2 = $swT.IndexOf("case 2:")
$mid = $swT.Substring($idx1, $idx2 - $idx1)
if ($mid.IndexOf("break;") -ge 0) { throw "implicit break between case 1 and case 2" }
if ($swT.IndexOf("switch (2)") -lt 0) { throw "missing nested switch (2)" }
Build-Run "control_switch" $swC 0

$gC = Translate "control_goto"
$gT = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $gC))
if ($gT.IndexOf("goto done;") -lt 0) { throw "missing forward goto done" }
if ($gT.IndexOf("done: ;") -lt 0) { throw "missing C99 label done" }
if ($gT.IndexOf("goto loop;") -lt 0) { throw "missing backward goto loop" }
if ($gT.IndexOf("loop: ;") -lt 0) { throw "missing C99 label loop" }
Build-Run "control_goto" $gC 0

$sdC = Translate "control_switch_decl"
$sdT = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $sdC))
if ($sdT.IndexOf("case 1: ;") -lt 0) { throw "missing case 1 before declaration" }
if ($sdT.IndexOf("default: ;") -lt 0) { throw "missing default before declaration" }
if ($sdT.IndexOf("int a = 1") -lt 0) { throw "missing case declaration" }
if ($sdT.IndexOf("int b = 2") -lt 0) { throw "missing default declaration" }
Build-Run "control_switch_decl" $sdC 0

Negative "invalid_switch_dup_default" "switch has duplicate default"
Negative "invalid_switch_case_out" "case/default outside switch"
Negative "invalid_switch_default_out" "case/default outside switch"
Negative "invalid_switch_end" "end target does not match close target"
Negative "invalid_switch_case_empty" "case expects an expression"
Negative "invalid_control_label_c" "reserved L1 name"
Negative "invalid_control_goto_l1" "reserved L1 name"

Write-C "control ok"
Write-Output "l1trans $gen control ok"
