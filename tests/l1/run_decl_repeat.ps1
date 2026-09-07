# L1 repeated declaration heads (scalar and @/@@ pointers).
# CWD = repo root. Isolated under build\l1trans\<gen>.
$ErrorActionPreference = "Stop"
Set-Location (Join-Path $PSScriptRoot "..\..")

$gen = "gen0"
if ($env:L1_GEN -and $env:L1_GEN.Trim().Length -gt 0) { $gen = $env:L1_GEN.Trim() }
$l1trans = "build\l1trans\$gen\l1trans.exe"
$obj = "build\obj\l1trans\$gen"
$bin = "build\l1trans\$gen"
$log = Join-Path "build\l1trans\logs" $gen
$script:repLog = Join-Path $log "decl_repeat.log"
$cflagsStr = "-std=c99 -Wall -Wextra -Wpedantic -I . -Werror=incompatible-pointer-types -Werror=discarded-qualifiers -Werror=implicit-function-declaration -Werror=implicit-int"

New-Item -ItemType Directory -Force -Path $obj, $bin, $log | Out-Null
Set-Content -LiteralPath $script:repLog -Value "$(Get-Date -Format o) decl_repeat start gen=$gen"

function Write-R([string]$m) { Add-Content -LiteralPath $script:repLog -Value "$(Get-Date -Format o) $m" }

if (-not (Test-Path $l1trans)) { throw "missing $l1trans" }

function Translate([string]$name) {
    $src = "tests\l1\$name.lm2"
    $cpath = Join-Path $obj ($name + ".c")
    $cpathB = Join-Path $obj ($name + "_b.c")
    Write-R "BEGIN translate $src"
    & $l1trans $src $cpath
    if ($LASTEXITCODE -ne 0) { throw "translate failed $name $LASTEXITCODE" }
    & $l1trans $src $cpathB
    if ($LASTEXITCODE -ne 0) { throw "translate b failed $name" }
    $h1 = (Get-FileHash -LiteralPath $cpath).Hash
    $h2 = (Get-FileHash -LiteralPath $cpathB).Hash
    if ($h1 -ne $h2) { throw "C differ $name" }
    Write-R "C $name sha256=$h1"
    return $cpath
}

function Build-Run([string]$name, [string]$cpath, [int]$expect) {
    $exepath = Join-Path $bin ($name + ".exe")
    $gccLog = Join-Path $log ("gcc_" + $name + ".log")
    $runOut = Join-Path $log ("run_" + $name + ".stdout")
    $runErr = Join-Path $log ("run_" + $name + ".stderr")
    Write-R "CMD gcc $cflagsStr -o $exepath $cpath"
    cmd /c "gcc $cflagsStr -o $exepath $cpath > $gccLog 2>&1"
    if ($LASTEXITCODE -ne 0) {
        Get-Content -LiteralPath $gccLog | Select-Object -Last 30
        throw "gcc failed $name"
    }
    cmd /c "$exepath > $runOut 2> $runErr"
    if ($LASTEXITCODE -ne $expect) { throw "run $name exit $LASTEXITCODE want $expect" }
    Write-R "EXIT run $name $LASTEXITCODE"
}

$sc = Translate "decl_repeat_scalar"
$st = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $sc))
if ($st.IndexOf("int x = 5") -lt 0) { throw "missing int x = 5" }
if ($st.IndexOf("int y = 10") -lt 0) { throw "missing int y = 10" }
if ($st.IndexOf("int z = 15") -lt 0) { throw "missing int z = 15" }
Build-Run "decl_repeat_scalar" $sc 0

$pc = Translate "decl_repeat_paren"
$pt = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $pc))
if ($pt.IndexOf("int a = 1") -lt 0) { throw "missing int a = 1" }
if ($pt.IndexOf("int b = 2") -lt 0) { throw "missing int b = 2" }
if ($pt.IndexOf("int c = 3") -lt 0) { throw "missing int c = 3" }
Build-Run "decl_repeat_paren" $pc 0

$ptrC = Translate "decl_repeat_ptr"
$ptrT = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $ptrC))
if ($ptrT.IndexOf("int * px") -lt 0 -and $ptrT.IndexOf("int *px") -lt 0) { throw "missing int * px" }
if ($ptrT.IndexOf("int * py") -lt 0 -and $ptrT.IndexOf("int *py") -lt 0) { throw "missing inherited int * py" }
if ($ptrT.IndexOf("char * pc") -lt 0 -and $ptrT.IndexOf("char *pc") -lt 0) { throw "missing explicit char * pc" }
if ($ptrT.IndexOf("char ** ppa") -lt 0 -and $ptrT.IndexOf("char **ppa") -lt 0) { throw "missing char ** ppa" }
if ($ptrT.IndexOf("char ** ppb") -lt 0 -and $ptrT.IndexOf("char **ppb") -lt 0) { throw "missing inherited char ** ppb" }
if ($ptrT.IndexOf("int * pc") -ge 0 -or $ptrT.IndexOf("int *pc") -ge 0) { throw "char pc inherited int" }
Build-Run "decl_repeat_ptr" $ptrC 0

$ctxC = Translate "decl_repeat_ctx"
$ctxT = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $ctxC))
if ($ctxT.IndexOf("int a = 1") -lt 0 -or $ctxT.IndexOf("int b = 2") -lt 0) { throw "missing if-body repeats" }
if ($ctxT.IndexOf("int c = 3") -lt 0 -or $ctxT.IndexOf("int d = 4") -lt 0) { throw "missing while-body repeats" }
Build-Run "decl_repeat_ctx" $ctxC 0

function Negative([string]$name, [string]$diag) {
    $src = "tests\l1\$name.lm2"
    $cpath = Join-Path $obj ($name + ".c")
    $err = Join-Path $log ($name + ".err")
    Write-R "BEGIN negative $src"
    if (Test-Path -LiteralPath $cpath) { Remove-Item -LiteralPath $cpath }
    cmd /c "$l1trans $src $cpath > $log\$name.stdout 2> $err"
    if ($LASTEXITCODE -eq 0) { throw "expected translate failure: $src" }
    $errText = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $err))
    if ($errText.IndexOf($diag) -lt 0) { throw "missing diagnostic '$diag': $errText" }
    if (Test-Path -LiteralPath $cpath) { throw "failed translate created $cpath" }
    Write-R "EXIT negative $src $LASTEXITCODE diagnostic ok"
}

Negative "invalid_decl_repeat_reset" "unsupported statement atom"
Negative "invalid_decl_repeat_disabled" "unsupported statement atom"

Write-R "decl_repeat ok"
Write-Output "l1trans $gen decl_repeat ok"
