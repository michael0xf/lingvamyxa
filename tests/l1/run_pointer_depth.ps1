$ErrorActionPreference = "Stop"
Set-Location (Join-Path $PSScriptRoot "..\..")

$gen = "gen2"
if ($env:L1_GEN -and $env:L1_GEN.Trim().Length -gt 0) { $gen = $env:L1_GEN.Trim() }
$translator = "build\l1trans\$gen\l1trans.exe"
$out = "build\obj\l1trans\$gen"
$bin = "build\l1trans\$gen"
New-Item -ItemType Directory -Force -Path $out, $bin | Out-Null
if (-not (Test-Path -LiteralPath $translator)) { throw "missing $translator" }

$source = "tests\l1\pointer_depth_n.lm1"
$c = Join-Path $out "pointer_depth_n.c"
& $translator $source $c
if ($LASTEXITCODE -ne 0) { throw "pointer-depth unit translation failed" }
$text = [IO.File]::ReadAllText((Join-Path (Get-Location) $c))
foreach ($needle in @("int *** value", "int *** ppp", "int ***slots[2]", "* * * casted", "***(casted) = 11")) {
    if ($text.IndexOf($needle) -lt 0) { throw "generated C missing '$needle'" }
}
$exe = Join-Path $bin "pointer_depth_n.exe"
cmd /c "gcc -std=c99 -Wall -Wextra -Wpedantic -Werror=incompatible-pointer-types -Werror=discarded-qualifiers -Werror=implicit-function-declaration -Werror=implicit-int -o $exe $c"
if ($LASTEXITCODE -ne 0) { throw "pointer-depth unit compile failed" }
& $exe
if ($LASTEXITCODE -ne 0) { throw "pointer-depth unit run failed: $LASTEXITCODE" }

$headerSource = "tests\l1\pointer_depth_n.h.lm1"
$header = Join-Path $out "pointer_depth_n.h"
& $translator $headerSource $header
if ($LASTEXITCODE -ne 0) { throw "pointer-depth header translation failed" }
$headerText = [IO.File]::ReadAllText((Join-Path (Get-Location) $header))
foreach ($needle in @("int *** value", "int *** (*PointerDepthFn)(int *** value)", "int *** pointer_depth_header_identity(int *** value)")) {
    if ($headerText.IndexOf($needle) -lt 0) { throw "generated header missing '$needle'" }
}

Write-Output "l1trans $gen arbitrary pointer depth ok"
