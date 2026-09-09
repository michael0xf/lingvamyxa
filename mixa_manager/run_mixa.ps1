# Mixa Manager: TextRect + OverlayRect/composite + headless backend + pump selftests.
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

# Record WHICH translator produced this run. mixa_manager builds against a
# translator it does not own, and that translator is going to be REPLACED rather
# than merely updated: the new parser did not match the old working version, so
# a parity corpus is being built against lingvamyxa_old_worked_version and
# ported forward. When the floor moves, a green run says nothing unless we can
# see that it moved.
#
# Hash the GENERATED C, not the .exe. Measured 2026-09-09: rebuilding from
# identical source gives a different .exe hash every time, so the binary is
# noise. The gen2 C is the self-hosting fixed point the gate proves - gen1, gen2
# and gen3 are byte-identical - so it changes when the translator's BEHAVIOUR
# changes and not merely when someone rebuilt it.
$fixedPoint = "stg/l1_baseline/build/obj/l1trans/gen2/l1trans.c"
"translator: $trans"
if ($env:MIXA_L1TRANS -and $env:MIXA_L1TRANS.Trim().Length -gt 0) {
    "translator fixed point: (not checked - MIXA_L1TRANS override in use)"
} elseif (Test-Path -LiteralPath $fixedPoint) {
    "translator fixed point: " + (Get-FileHash -LiteralPath $fixedPoint -Algorithm SHA256).Hash
} else {
    "translator fixed point: (absent - run stg\l1_baseline\gate.ps1)"
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
    "mixa_backend_selftest",
    "mixa_pump_selftest"
)

foreach ($unit in $units) {
    $src = "mixa_manager\tests\$unit.lm1"
    $c = Join-Path $out "$unit.c"
    $exe = Join-Path $out "$unit.exe"

    & $trans $src $c
    if ($LASTEXITCODE -ne 0) { throw "translate failed: $src" }

    # Native gcc warnings on stderr must not trip $ErrorActionPreference Stop.
    $prev = $ErrorActionPreference
    $ErrorActionPreference = "Continue"
    & gcc -std=c99 -Wall -Wextra -Wpedantic @guards -I . $c -o $exe 2>&1 |
        Tee-Object -FilePath (Join-Path $log "$unit.gcc.log") | Out-Null
    $gccRc = $LASTEXITCODE
    $ErrorActionPreference = $prev
    if ($gccRc -ne 0) {
        Get-Content (Join-Path $log "$unit.gcc.log")
        throw "gcc failed: $c"
    }

    & $exe
    if ($LASTEXITCODE -ne 0) { throw "$unit failed" }
}

"mixa core ok"
"mixa overlay ok"
"mixa backend ok"
"mixa pump ok"
