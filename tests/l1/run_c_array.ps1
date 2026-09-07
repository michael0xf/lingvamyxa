# L1 c.array nested type-head and outer const wrapper (1D fixed extents).
# CWD = repo root. Isolated under build\l1trans\<gen>.
$ErrorActionPreference = "Stop"
Set-Location (Join-Path $PSScriptRoot "..\..")

$gen = "gen0"
if ($env:L1_GEN -and $env:L1_GEN.Trim().Length -gt 0) { $gen = $env:L1_GEN.Trim() }
$l1trans = "build\l1trans\$gen\l1trans.exe"
$obj = "build\obj\l1trans\$gen"
$bin = "build\l1trans\$gen"
$log = Join-Path "build\l1trans\logs" $gen
$script:arrLog = Join-Path $log "c_array.log"
$cflagsStr = "-std=c99 -Wall -Wextra -Wpedantic -I . -Werror=incompatible-pointer-types -Werror=discarded-qualifiers -Werror=implicit-function-declaration -Werror=implicit-int"

New-Item -ItemType Directory -Force -Path $obj, $bin, $log | Out-Null
Set-Content -LiteralPath $script:arrLog -Value "$(Get-Date -Format o) c_array start gen=$gen"

function Write-A([string]$m) { Add-Content -LiteralPath $script:arrLog -Value "$(Get-Date -Format o) $m" }

if (-not (Test-Path $l1trans)) { throw "missing $l1trans" }

function Translate([string]$name) {
    $src = "tests\l1\$name.lm2"
    $cpath = Join-Path $obj ($name + ".c")
    $cpathB = Join-Path $obj ($name + "_b.c")
    Write-A "BEGIN translate $src"
    & $l1trans $src $cpath
    if ($LASTEXITCODE -ne 0) { throw "translate failed $name $LASTEXITCODE" }
    & $l1trans $src $cpathB
    if ($LASTEXITCODE -ne 0) { throw "translate b failed $name" }
    $h1 = (Get-FileHash -LiteralPath $cpath).Hash
    $h2 = (Get-FileHash -LiteralPath $cpathB).Hash
    if ($h1 -ne $h2) { throw "C differ $name" }
    Write-A "C $name sha256=$h1"
    return $cpath
}

function Build-Run([string]$name, [string]$cpath, [int]$expect) {
    $exepath = Join-Path $bin ($name + ".exe")
    $gccLog = Join-Path $log ("gcc_" + $name + ".log")
    $runOut = Join-Path $log ("run_" + $name + ".stdout")
    $runErr = Join-Path $log ("run_" + $name + ".stderr")
    Write-A "CMD gcc $cflagsStr -o $exepath $cpath"
    cmd /c "gcc $cflagsStr -o $exepath $cpath > $gccLog 2>&1"
    if ($LASTEXITCODE -ne 0) {
        Get-Content -LiteralPath $gccLog | Select-Object -Last 30
        throw "gcc failed $name"
    }
    cmd /c "$exepath > $runOut 2> $runErr"
    if ($LASTEXITCODE -ne $expect) { throw "run $name exit $LASTEXITCODE want $expect" }
    Write-A "EXIT run $name $LASTEXITCODE"
}

$nc = Translate "c_array_nested"
$nt = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $nc))
if ($nt.IndexOf("const int top_values[3] = {2, 4, 6}") -lt 0) { throw "missing const int top_values[3]" }
if ($nt.IndexOf("int top_target[1] = {13}") -lt 0) { throw "missing int top_target[1]" }
if ($nt.IndexOf("int local_values[2] = {8, 9}") -lt 0) { throw "missing int local_values[2]" }
Build-Run "c_array_nested" $nc 0

$cc = Translate "c_array_const"
$ct = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $cc))
if ($ct.IndexOf("const int values[3] = {2, 4, 6}") -lt 0) { throw "missing const int values[3]" }
Build-Run "c_array_const" $cc 0

$cw = Translate "c_array_const_write"
$cwt = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $cw))
if ($cwt.IndexOf("const int values[1] = {7}") -lt 0) { throw "missing const int values[1]" }
$cwExe = Join-Path $bin "c_array_const_write.exe"
$cwGcc = Join-Path $log "gcc_c_array_const_write.log"
Write-A "CMD gcc compile-negative $cw"
cmd /c "gcc $cflagsStr -o $cwExe $cw > $cwGcc 2>&1"
if ($LASTEXITCODE -eq 0) { throw "expected gcc failure writing const int element" }
Write-A "EXIT gcc c_array_const_write $LASTEXITCODE (expected fail)"

$pc = Translate "c_array_ptr"
$pt = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $pc))
if ($pt.IndexOf("const char *names[2] = {") -lt 0) { throw "missing const char *names[2]" }
if ($pt.IndexOf("char *const names") -ge 0) { throw "emitted char *const names" }
if ($pt.IndexOf("const char *candidates[4];") -lt 0) { throw "missing const char *candidates[4]" }
Build-Run "c_array_ptr" $pc 0

function Negative([string]$name, [string]$diag) {
    $src = "tests\l1\$name.lm2"
    $cpath = Join-Path $obj ($name + ".c")
    $err = Join-Path $log ($name + ".err")
    Write-A "BEGIN negative $src"
    if (Test-Path -LiteralPath $cpath) { Remove-Item -LiteralPath $cpath }
    cmd /c "$l1trans $src $cpath > $log\$name.stdout 2> $err"
    if ($LASTEXITCODE -eq 0) { throw "expected translate failure: $src" }
    $errText = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $err))
    if ($errText.IndexOf($diag) -lt 0) { throw "missing diagnostic '$diag': $errText" }
    if (Test-Path -LiteralPath $cpath) { throw "failed translate created $cpath" }
    Write-A "EXIT negative $src $LASTEXITCODE diagnostic ok"
}

Negative "invalid_c_array_empty" "c.array missing contents"
Negative "invalid_c_array_noname" "c.array missing name"
Negative "invalid_c_array_const_arity" "const c.array expects exactly one bracket-head array declaration"

Write-A "c_array ok"
Write-Output "l1trans $gen c_array ok"
