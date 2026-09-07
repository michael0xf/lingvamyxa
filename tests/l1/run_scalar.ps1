# L1 float/double/ulong C scalar spellings.
# CWD = repo root. Isolated under build\l1trans\<gen>.
$ErrorActionPreference = "Stop"
Set-Location (Join-Path $PSScriptRoot "..\..")

$gen = "gen0"
if ($env:L1_GEN -and $env:L1_GEN.Trim().Length -gt 0) { $gen = $env:L1_GEN.Trim() }
$l1trans = "build\l1trans\$gen\l1trans.exe"
$obj = "build\obj\l1trans\$gen"
$bin = "build\l1trans\$gen"
$log = Join-Path "build\l1trans\logs" $gen
$script:scalarLog = Join-Path $log "scalar.log"
$cflagsStr = "-std=c99 -Wall -Wextra -Wpedantic -I . -Werror=incompatible-pointer-types -Werror=discarded-qualifiers -Werror=implicit-function-declaration -Werror=implicit-int"

New-Item -ItemType Directory -Force -Path $obj, $bin, $log | Out-Null
Set-Content -LiteralPath $script:scalarLog -Value "$(Get-Date -Format o) scalar start gen=$gen"

function Write-S([string]$m) { Add-Content -LiteralPath $script:scalarLog -Value "$(Get-Date -Format o) $m" }

if (-not (Test-Path $l1trans)) { throw "missing $l1trans" }

function Translate([string]$name) {
    $src = "tests\l1\$name.lm2"
    $cpath = Join-Path $obj ($name + ".c")
    $cpathB = Join-Path $obj ($name + "_b.c")
    Write-S "BEGIN translate $src"
    & $l1trans $src $cpath
    if ($LASTEXITCODE -ne 0) { throw "translate failed $name $LASTEXITCODE" }
    & $l1trans $src $cpathB
    if ($LASTEXITCODE -ne 0) { throw "translate b failed $name" }
    $h1 = (Get-FileHash -LiteralPath $cpath).Hash
    $h2 = (Get-FileHash -LiteralPath $cpathB).Hash
    if ($h1 -ne $h2) { throw "C differ $name" }
    Write-S "C $name sha256=$h1"
    return $cpath
}

function Build-Run([string]$name, [string]$cpath, [int]$expect) {
    $exepath = Join-Path $bin ($name + ".exe")
    $gccLog = Join-Path $log ("gcc_" + $name + ".log")
    $runOut = Join-Path $log ("run_" + $name + ".stdout")
    $runErr = Join-Path $log ("run_" + $name + ".stderr")
    Write-S "CMD gcc $cflagsStr -o $exepath $cpath"
    cmd /c "gcc $cflagsStr -o $exepath $cpath > $gccLog 2>&1"
    if ($LASTEXITCODE -ne 0) {
        Get-Content -LiteralPath $gccLog | Select-Object -Last 30
        throw "gcc failed $name"
    }
    cmd /c "$exepath > $runOut 2> $runErr"
    if ($LASTEXITCODE -ne $expect) { throw "run $name exit $LASTEXITCODE want $expect" }
    Write-S "EXIT run $name $LASTEXITCODE"
}

$printfC = Translate "scalar_printf"
$printfText = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $printfC))
if ($printfText.IndexOf("double double_value") -lt 0) { throw "missing double double_value" }
if ($printfText.IndexOf("ulong") -ge 0) { throw "raw ulong in printf fixture" }
Build-Run "scalar_printf" $printfC 0

$paramC = Translate "scalar_params"
$paramText = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $paramC))
if ($paramText.IndexOf("double take_d(double x)") -lt 0) { throw "missing double proto/def" }
if ($paramText.IndexOf("float take_f(float x)") -lt 0) { throw "missing float proto/def" }
if ($paramText.IndexOf("unsigned long take_u(unsigned long n)") -lt 0) { throw "missing ulong -> unsigned long" }
if ($paramText.IndexOf("ulong") -ge 0) { throw "raw ulong in params" }
Build-Run "scalar_params" $paramC 0

$ulC = Translate "scalar_ulong"
$ulText = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $ulC))
if ($ulText.IndexOf("unsigned long value") -lt 0) { throw "missing unsigned long value" }
if ($ulText.IndexOf("strtoul") -lt 0) { throw "missing strtoul" }
if ($ulText.IndexOf("ulong") -ge 0) { throw "raw ulong in ulong fixture" }
Build-Run "scalar_ulong" $ulC 0

$formsC = Translate "scalar_forms"
$formsText = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $formsC))
if ($formsText.IndexOf("const double d") -lt 0) { throw "missing const double" }
if ($formsText.IndexOf("float * fp") -lt 0 -and $formsText.IndexOf("float *fp") -lt 0) { throw "missing float pointer" }
if ($formsText.IndexOf("unsigned long xs[") -lt 0) { throw "missing unsigned long array" }
if ($formsText.IndexOf("(unsigned long)") -lt 0) { throw "missing unsigned long cast" }
if ($formsText.IndexOf("ulong") -ge 0) { throw "raw ulong in forms" }
Build-Run "scalar_forms" $formsC 0

$unitC = Translate "scalar_unit_for"
$unitText = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $unitC))
if ($unitText.IndexOf("double g") -lt 0) { throw "missing unit double g" }
if ($unitText.IndexOf("unsigned long sum") -lt 0) { throw "missing ulong sum declaration" }
if ($unitText.IndexOf("unsigned long i = 1") -lt 0) { throw "for-init did not stay a declaration" }
if ($unitText.IndexOf("ulong") -ge 0) { throw "raw ulong in unit/for" }
Build-Run "scalar_unit_for" $unitC 0

Write-S "scalar ok"
Write-Output "l1trans $gen scalar ok"
