# L2 unit: Lmx core and registered-range classification.
# Build root is this file's parent, matching tests\l1\*.ps1 convention.
# Picks the L1 generation with L1_GEN (default gen2).
$ErrorActionPreference = "Stop"
Set-Location (Join-Path $PSScriptRoot "..")

$gen = if ($env:L1_GEN) { $env:L1_GEN } else { "gen2" }
$trans = "build\l1trans\$gen\l1trans.exe"
if (-not (Test-Path -LiteralPath $trans)) {
    throw "missing L1 translator: $trans (run tests\l1\run_gen.ps1 first)"
}

$out = "build\l2"
$log = "build\l1trans\logs\$gen"
New-Item -ItemType Directory -Force -Path $out, $log | Out-Null

$guards = @(
    "-Werror=incompatible-pointer-types", "-Werror=discarded-qualifiers",
    "-Werror=implicit-function-declaration", "-Werror=implicit-int"
)

$src = "l2src\lmx_selftest.lm1"
$c = Join-Path $out "lmx_selftest.c"
$exe = Join-Path $out "lmx_selftest.exe"

& $trans $src $c
if ($LASTEXITCODE -ne 0) { throw "$gen translate failed: $src" }

& gcc -std=c99 -Wall -Wextra -Wpedantic @guards -I . $c -o $exe 2>&1 |
    Tee-Object -FilePath (Join-Path $log "lmx_gcc.log") | Out-Null
if ($LASTEXITCODE -ne 0) {
    Get-Content (Join-Path $log "lmx_gcc.log")
    throw "$gen gcc failed: $c"
}

& $exe
if ($LASTEXITCODE -ne 0) { throw "$gen lmx selftest failed" }
"l2 lmx range $gen ok"
