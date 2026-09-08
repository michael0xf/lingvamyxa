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
    $src = "tests\l1\$name.lm1"
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

$declC = Translate "scalar_fixed_decl"
$declText = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $declC))
if ($declText.IndexOf("uint8_t a") -lt 0) { throw "missing uint8_t a" }
if ($declText.IndexOf("uint16_t b") -lt 0) { throw "missing uint16_t b" }
if ($declText.IndexOf("uint32_t c") -lt 0) { throw "missing uint32_t c" }
if ($declText.IndexOf("uint64_t d") -lt 0) { throw "missing uint64_t d" }
if ($declText.IndexOf("int8_t e") -lt 0) { throw "missing int8_t e" }
if ($declText.IndexOf("int16_t f") -lt 0) { throw "missing int16_t f" }
if ($declText.IndexOf("int32_t g") -lt 0) { throw "missing int32_t g" }
if ($declText.IndexOf("int64_t h") -lt 0) { throw "missing int64_t h" }
if ($declText.IndexOf("#include <stdint.h>") -lt 0) { throw "missing stdint.h" }
Build-Run "scalar_fixed_decl" $declC 0

$hashC = Translate "scalar_fixed_hash"
$hashText = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $hashC))
if ($hashText.IndexOf("uint64_t guard_hash") -lt 0) { throw "missing uint64_t guard_hash" }
if ($hashText.IndexOf("uint8_t byte") -lt 0) { throw "missing uint8_t byte" }
if ($hashText.IndexOf("1099511628211ULL") -lt 0) { throw "missing FNV prime" }
if ($hashText.IndexOf("0x5eeaf5dacf966d85ULL") -lt 0) { throw "missing independent forward oracle" }
if ($hashText.IndexOf("0x67f171f418d3a1c1ULL") -lt 0) { throw "missing independent reverse oracle" }
Build-Run "scalar_fixed_hash" $hashC 0

$signedC = Translate "scalar_fixed_signed"
$signedText = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $signedC))
if ($signedText.IndexOf("int8_t take_i8(int8_t x)") -lt 0) { throw "missing signed i8 param/return" }
if ($signedText.IndexOf("int32_t take_i32(int32_t x)") -lt 0) { throw "missing signed i32 param/return" }
if ($signedText.IndexOf("const int16_t k") -lt 0) { throw "missing const int16_t" }
if ($signedText.IndexOf("int8_t * p") -lt 0 -and $signedText.IndexOf("int8_t *p") -lt 0) { throw "missing i8 pointer" }
if ($signedText.IndexOf("int64_t xs[") -lt 0) { throw "missing i64 array" }
if ($signedText.IndexOf("uint64_t xs[") -ge 0) { throw "i64 array mapped unsigned" }
if ($signedText -notmatch '(?<![A-Za-z0-9_])int64_t xs\[') { throw "i64 array token is not signed int64_t" }
if ($signedText.IndexOf("(int8_t)") -lt 0) { throw "missing i8 cast" }
if ($signedText.IndexOf("uint8_t take_i8") -ge 0) { throw "i8 mapped unsigned" }
if ($signedText.IndexOf("uint32_t take_i32") -ge 0) { throw "i32 mapped unsigned" }
Build-Run "scalar_fixed_signed" $signedC 0

$parenC = Translate "scalar_fixed_paren"
$parenText = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $parenC))
if ($parenText.IndexOf("uint32_t v") -lt 0) { throw "missing uint32_t v declaration" }
if ($parenText.IndexOf("(1 + 1) * 3") -lt 0) { throw "missing grouped (1 + 1) * 3" }
Build-Run "scalar_fixed_paren" $parenC 0

$fuC = Translate "scalar_fixed_unit_for"
$fuText = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $fuC))
if ($fuText.IndexOf("uint64_t g") -lt 0) { throw "missing unit uint64_t g" }
if ($fuText.IndexOf("uint32_t sum") -lt 0) { throw "missing uint32_t sum" }
if ($fuText.IndexOf("uint32_t i = 1") -lt 0) { throw "for-init did not map u32" }
Build-Run "scalar_fixed_unit_for" $fuC 0

Write-S "scalar ok"
Write-Output "l1trans $gen scalar ok"
