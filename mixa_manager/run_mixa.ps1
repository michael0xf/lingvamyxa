# Mixa Manager: TextRect + OverlayRect/composite + headless backend selftests.
# Build root is the repository root (parent of mixa_manager).
# Translator is the STABLE L1 under stg\l1_baseline, not the live tree.
$ErrorActionPreference = "Stop"
Set-Location (Join-Path $PSScriptRoot "..")

$trans = "stg\l1_baseline\build\l1trans\gen2\l1trans.exe"
if ($env:MIXA_L1TRANS -and $env:MIXA_L1TRANS.Trim().Length -gt 0) {
    $trans = $env:MIXA_L1TRANS.Trim()
}
if (-not (Test-Path -LiteralPath $trans)) {
    throw "missing stable L1 translator: $trans (produce via stg\l1_baseline\gate.ps1)"
}

$out = "build\mixa"
$log = "build\mixa\logs"
New-Item -ItemType Directory -Force -Path $out, $log | Out-Null

$guards = @(
    "-Werror=incompatible-pointer-types", "-Werror=discarded-qualifiers",
    "-Werror=implicit-function-declaration", "-Werror=implicit-int"
)

$units = @(
    "mixa_core_selftest",
    "mixa_overlay_selftest",
    "mixa_backend_selftest"
)

foreach ($unit in $units) {
    $src = "mixa_manager\tests\$unit.lm1"
    $c = Join-Path $out "$unit.c"
    $exe = Join-Path $out "$unit.exe"

    & $trans $src $c
    if ($LASTEXITCODE -ne 0) { throw "translate failed: $src" }

    & gcc -std=c99 -Wall -Wextra -Wpedantic @guards -I . $c -o $exe 2>&1 |
        Tee-Object -FilePath (Join-Path $log "$unit.gcc.log") | Out-Null
    if ($LASTEXITCODE -ne 0) {
        Get-Content (Join-Path $log "$unit.gcc.log")
        throw "gcc failed: $c"
    }

    & $exe
    if ($LASTEXITCODE -ne 0) { throw "$unit failed" }
}

"mixa core ok"
"mixa overlay ok"
"mixa backend ok"
