# L2 units: Lmx core, registered-range classification, typed service pools.
# Build root is this file's parent, matching tests\l1\*.ps1 convention.
# Picks the L1 generation with L1_GEN (default gen2).
$ErrorActionPreference = "Stop"
Set-Location (Join-Path $PSScriptRoot "..")

$gen = "gen2"
if ($env:L1_GEN -and $env:L1_GEN.Trim().Length -gt 0) { $gen = $env:L1_GEN.Trim() }
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

foreach ($unit in @("lmx_selftest", "lmx_pool_selftest", "lmx_chars_selftest", "lmx_ref_selftest", "lmx_branch_selftest", "lmx_own_selftest", "lmx_size_selftest")) {
    $src = "l2src\$unit.lm1"
    $c = Join-Path $out "$unit.c"
    $exe = Join-Path $out "$unit.exe"

    & $trans $src $c
    if ($LASTEXITCODE -ne 0) { throw "$gen translate failed: $src" }

    & gcc -std=c99 -Wall -Wextra -Wpedantic @guards -I . $c -o $exe 2>&1 |
        Tee-Object -FilePath (Join-Path $log "$unit.gcc.log") | Out-Null
    if ($LASTEXITCODE -ne 0) {
        Get-Content (Join-Path $log "$unit.gcc.log")
        throw "$gen gcc failed: $c"
    }

    & $exe
    if ($LASTEXITCODE -ne 0) { throw "$gen $unit failed" }
}
"l2 lmx $gen ok"
