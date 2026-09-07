# L1 define / ifndef-default -> C99 object-like #define.
# CWD = repo root. Isolated under build\l1trans\<gen>.
$ErrorActionPreference = "Stop"
Set-Location (Join-Path $PSScriptRoot "..\..")

$gen = "gen0"
if ($env:L1_GEN -and $env:L1_GEN.Trim().Length -gt 0) { $gen = $env:L1_GEN.Trim() }
$l1trans = "build\l1trans\$gen\l1trans.exe"
$obj = "build\obj\l1trans\$gen"
$bin = "build\l1trans\$gen"
$log = Join-Path "build\l1trans\logs" $gen
$script:defineLog = Join-Path $log "define.log"
$cflagsStr = "-std=c99 -Wall -Wextra -Wpedantic -I . -Werror=incompatible-pointer-types -Werror=discarded-qualifiers -Werror=implicit-function-declaration -Werror=implicit-int"

New-Item -ItemType Directory -Force -Path $obj, $bin, $log | Out-Null
Set-Content -LiteralPath $script:defineLog -Value "$(Get-Date -Format o) define start gen=$gen"

function Write-D([string]$m) { Add-Content -LiteralPath $script:defineLog -Value "$(Get-Date -Format o) $m" }

if (-not (Test-Path $l1trans)) { throw "missing $l1trans" }

function Translate([string]$name) {
    $src = "tests\l1\$name.lm1"
    $cpath = Join-Path $obj ($name + ".c")
    $cpathB = Join-Path $obj ($name + "_b.c")
    Write-D "BEGIN translate $src"
    & $l1trans $src $cpath
    if ($LASTEXITCODE -ne 0) { throw "translate failed $name $LASTEXITCODE" }
    & $l1trans $src $cpathB
    if ($LASTEXITCODE -ne 0) { throw "translate b failed $name" }
    $h1 = (Get-FileHash -LiteralPath $cpath).Hash
    $h2 = (Get-FileHash -LiteralPath $cpathB).Hash
    if ($h1 -ne $h2) { throw "C differ $name" }
    Write-D "C $name sha256=$h1"
    return $cpath
}

function Build-Run([string]$name, [string]$cpath, [string]$defs, [int]$expect) {
    $exepath = Join-Path $bin ($name + $defs.Replace("=", "").Replace(" ", "_") + ".exe")
    $gccLog = Join-Path $log ("gcc_" + $name + $defs.Replace("=", "").Replace(" ", "_") + ".log")
    $runOut = Join-Path $log ("run_" + $name + $defs.Replace("=", "").Replace(" ", "_") + ".stdout")
    $runErr = Join-Path $log ("run_" + $name + $defs.Replace("=", "").Replace(" ", "_") + ".stderr")
    $defPart = ""
    if ($defs.Length -gt 0) { $defPart = " " + $defs }
    Write-D "CMD gcc $cflagsStr$defPart -o $exepath $cpath"
    cmd /c "gcc $cflagsStr$defPart -o $exepath $cpath > $gccLog 2>&1"
    if ($LASTEXITCODE -ne 0) {
        Get-Content -LiteralPath $gccLog | Select-Object -Last 30
        throw "gcc failed $name defs=$defs"
    }
    cmd /c "$exepath > $runOut 2> $runErr"
    if ($LASTEXITCODE -ne $expect) { throw "run $name defs=$defs exit $LASTEXITCODE want $expect" }
    Write-D "EXIT run $name defs=$defs $LASTEXITCODE"
}

$valueC = Translate "define_value"
$valueText = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $valueC))
if ($valueText.IndexOf("#define VALUE 7") -lt 0) { throw "missing #define VALUE 7" }
if ($valueText.IndexOf("#if VALUE") -lt 0) { throw "missing #if VALUE" }
if ($valueText.IndexOf("#define INNER 4") -lt 0 -or $valueText.IndexOf("#define INNER 2") -lt 0) { throw "missing INNER defines in branches" }
if ($valueText.IndexOf("#if FLAG") -lt 0) { throw "missing #if FLAG around INNER" }
Build-Run "define_value" $valueC "-DFLAG=1" 12
Build-Run "define_value" $valueC "" 10

$ifndefC = Translate "define_ifndef"
$ifndefText = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $ifndefC))
if ($ifndefText.IndexOf("#ifndef VALUE") -lt 0) { throw "missing #ifndef VALUE" }
if ($ifndefText.IndexOf("#define VALUE 7") -lt 0) { throw "missing #define VALUE 7 in ifndef-default" }
if ($ifndefText.IndexOf("#endif") -lt 0) { throw "missing #endif for ifndef-default" }
Build-Run "define_ifndef" $ifndefC "" 7
Build-Run "define_ifndef" $ifndefC "-DVALUE=0" 0
Build-Run "define_ifndef" $ifndefC "-DVALUE=9" 9

$emptyC = Translate "define_empty"
$emptyText = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $emptyC))
if ($emptyText.IndexOf("#define PRESENT 1") -ge 0) { throw "empty define added implicit 1" }
if ($emptyText -notmatch '#define PRESENT\r?\n') { throw "missing empty #define PRESENT" }
if ($emptyText.IndexOf("defined(PRESENT)") -lt 0) { throw "missing defined(PRESENT)" }
Build-Run "define_empty" $emptyC "" 1

$tokC = Translate "define_tokens"
$tokText = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $tokC))
if ($tokText.IndexOf('#define ALIAS VALUE') -lt 0) { throw "missing alias #define ALIAS VALUE" }
if ($tokText.IndexOf('#define MSG "ok"') -lt 0) { throw 'missing quoted string token MSG "ok"' }
if ($tokText.IndexOf("#define CH 'A'") -lt 0) { throw "missing char token CH 'A'" }
if ($tokText.IndexOf("#define EQ =") -lt 0) { throw "missing EQ = token (not executed)" }
if ($tokText.IndexOf("#define EQ ==") -ge 0) { throw "EQ rewritten to ==" }
if ($tokText.IndexOf("#define ADD 3 + 4") -lt 0) { throw "missing backtick numeric ADD 3 + 4" }
if ($tokText.IndexOf("#define UNUSED (void)") -lt 0) { throw "missing backtick (void) token (not executed)" }
if ($tokText.IndexOf("->") -ge 0) { throw "backslash rewritten to ->" }
if ($tokText -match '#define \w+\(') { throw "function-like macro emitted" }
# 72 = ALIAS+CH. MSG bytes/NUL fail as 20/21/22; ADD fail as 30.
Build-Run "define_tokens" $tokC "" 72

function Negative([string]$name, [string]$diag) {
    $src = "tests\l1\$name.lm1"
    $cpath = Join-Path $obj ($name + ".c")
    $err = Join-Path $log ($name + ".err")
    Write-D "BEGIN negative $src"
    if (Test-Path -LiteralPath $cpath) { Remove-Item -LiteralPath $cpath }
    cmd /c "$l1trans $src $cpath > $log\$name.stdout 2> $err"
    if ($LASTEXITCODE -eq 0) { throw "expected translate failure: $src" }
    $errText = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $err))
    if ($errText.IndexOf($diag) -lt 0) { throw "missing diagnostic '$diag': $errText" }
    if (Test-Path -LiteralPath $cpath) { throw "failed translate created $cpath" }
    Write-D "EXIT negative $src $LASTEXITCODE diagnostic ok"
}

Negative "invalid_define_noname" "define receiver expects macro name as first atom"
Negative "invalid_define_quoted_name" "define receiver expects macro name as identifier atom"
Negative "invalid_define_nonatom" "define receiver expects atom tokens"
Negative "invalid_define_end" "end target does not match close target"
Negative "invalid_lm1_define_l2" "reserved L1 name"
Negative "invalid_lm1_define_repl_c" "reserved L1 name"

Write-D "define ok"
Write-Output "l1trans $gen define ok"
