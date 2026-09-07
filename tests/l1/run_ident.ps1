# L1 quoted identifiers: outer backticks strip in name positions.
# CWD = repo root. Isolated under build\l1trans\<gen>.
$ErrorActionPreference = "Stop"
Set-Location (Join-Path $PSScriptRoot "..\..")

$gen = "gen0"
if ($env:L1_GEN -and $env:L1_GEN.Trim().Length -gt 0) { $gen = $env:L1_GEN.Trim() }
$l1trans = "build\l1trans\$gen\l1trans.exe"
$obj = "build\obj\l1trans\$gen"
$bin = "build\l1trans\$gen"
$log = Join-Path "build\l1trans\logs" $gen
$script:identLog = Join-Path $log "ident.log"
$cflagsStr = "-std=c99 -Wall -Wextra -Wpedantic -I . -Werror=incompatible-pointer-types -Werror=discarded-qualifiers -Werror=implicit-function-declaration -Werror=implicit-int"

New-Item -ItemType Directory -Force -Path $obj, $bin, $log | Out-Null
Set-Content -LiteralPath $script:identLog -Value "$(Get-Date -Format o) ident start gen=$gen"

function Write-I([string]$m) { Add-Content -LiteralPath $script:identLog -Value "$(Get-Date -Format o) $m" }

if (-not (Test-Path $l1trans)) { throw "missing $l1trans" }

function Translate([string]$name) {
    $src = "tests\l1\$name.lm2"
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

function Build-Run([string]$name, [string]$cpath, [int]$expect) {
    $exepath = Join-Path $bin ($name + ".exe")
    $gccLog = Join-Path $log ("gcc_" + $name + ".log")
    $runOut = Join-Path $log ("run_" + $name + ".stdout")
    $runErr = Join-Path $log ("run_" + $name + ".stderr")
    Write-I "CMD gcc $cflagsStr -o $exepath $cpath"
    cmd /c "gcc $cflagsStr -o $exepath $cpath > $gccLog 2>&1"
    if ($LASTEXITCODE -ne 0) {
        Get-Content -LiteralPath $gccLog | Select-Object -Last 30
        throw "gcc failed $name"
    }
    cmd /c "$exepath > $runOut 2> $runErr"
    if ($LASTEXITCODE -ne $expect) { throw "run $name exit $LASTEXITCODE want $expect" }
    Write-I "EXIT run $name $LASTEXITCODE"
}

$aliasC = Translate "ident_alias"
$aliasT = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $aliasC))
if ($aliasT.IndexOf("int main_helper(void)") -lt 0) { throw "missing unquoted main_helper" }
if ($aliasT.IndexOf("int main(void)") -lt 0) { throw "missing unquoted main" }
if ($aliasT.IndexOf("int value = 40") -lt 0) { throw "missing unquoted value" }
if ($aliasT.IndexOf("``") -ge 0 -or $aliasT.Contains([char]96)) { throw "quoted name leaked into C" }
Build-Run "ident_alias" $aliasC 0

$fnC = Translate "ident_fn"
$fnT = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $fnC))
if ($fnT.IndexOf("int add1(int n)") -lt 0) { throw "missing add1 proto/def" }
if ($fnT.Contains([char]96)) { throw "quoted fn/param leaked into C" }
Build-Run "ident_fn" $fnC 0

$varC = Translate "ident_vars"
$varT = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $varC))
if ($varT.IndexOf("int x = 5") -lt 0) { throw "missing x" }
if ($varT.IndexOf("int y = 10") -lt 0) { throw "missing repeated y" }
if ($varT.IndexOf("int * p") -lt 0 -and $varT.IndexOf("int *p") -lt 0) { throw "missing pointer p" }
if ($varT.IndexOf("int * q") -lt 0 -and $varT.IndexOf("int *q") -lt 0) { throw "missing inherited pointer q" }
if ($varT.IndexOf("int xs[") -lt 0) { throw "missing array xs" }
if ($varT.Contains([char]96)) { throw "quoted var leaked into C" }
Build-Run "ident_vars" $varC 0

$strC = Translate "ident_string"
$strT = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $strC))
if ($strT.IndexOf('"a`b"') -lt 0 -and $strT.IndexOf('"a``b"') -lt 0) {
    if ($strT.IndexOf([char]96) -lt 0) { throw "string backtick was stripped" }
}
Build-Run "ident_string" $strC 0

Write-I "ident ok"
Write-Output "l1trans $gen ident ok"
