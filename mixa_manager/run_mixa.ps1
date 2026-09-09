# Mixa Manager: TextRect + OverlayRect/composite + headless backend + pump + selection + Win32 backend selftests.
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

# Record what produced this run. mixa_manager builds against a translator it
# does not own and cannot pin, so any change to that floor arrives here as a
# suite result. Two DIFFERENT facts are printed and neither implies the other:
#
#   binary       SHA256 of the executable actually invoked. This is the
#                identity of that concrete binary. Measured 2026-09-09: a
#                rebuild from identical source gives a different hash every
#                time, so it does not survive rebuilds and is not a behaviour
#                comparison.
#
#   source       SHA256 of the generated gen2 l1trans.c, a GENERATED-SOURCE
#                FINGERPRINT. It is not a behavioural hash: formatting can move
#                it without changing behaviour, and compiler version or flags
#                can change behaviour without moving it. It also does not prove
#                the binary above was built from it - it only sits beside it in
#                the same build tree. Its presence is not evidence that a fixed
#                point was verified in this run; the gate proves that, not this
#                script.
#
# Use them as breadcrumbs for "did the floor move", not as proof of identity.
$genSource = "stg/l1_baseline/build/obj/l1trans/gen2/l1trans.c"
"translator: $trans"
"translator binary sha256: " + (Get-FileHash -LiteralPath $trans -Algorithm SHA256).Hash
if ($env:MIXA_L1TRANS -and $env:MIXA_L1TRANS.Trim().Length -gt 0) {
    "generated-source fingerprint: (not applicable - MIXA_L1TRANS override in use)"
} elseif (Test-Path -LiteralPath $genSource) {
    "generated-source fingerprint: " + (Get-FileHash -LiteralPath $genSource -Algorithm SHA256).Hash
} else {
    "generated-source fingerprint: (absent - produce via the baseline gate)"
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
    "mixa_pump_selftest",
    "mixa_selection_selftest",
    "mixa_backend_win32_selftest"
)

$win32Ok = $false
$win32Skipped = $false

foreach ($unit in $units) {
    if ($unit -eq "mixa_backend_win32_selftest") {
        if ($env:OS -ne "Windows_NT") {
            "mixa win32 backend skipped (not Windows)"
            $win32Skipped = $true
            continue
        }
    }

    $src = "mixa_manager\tests\$unit.lm1"
    $c = Join-Path $out "$unit.c"
    $exe = Join-Path $out "$unit.exe"

    & $trans $src $c
    if ($LASTEXITCODE -ne 0) { throw "translate failed: $src" }

    # Native gcc warnings on stderr must not trip $ErrorActionPreference Stop.
    $prev = $ErrorActionPreference
    $ErrorActionPreference = "Continue"
    $linkLibs = @()
    if ($unit -eq "mixa_backend_win32_selftest") {
        $linkLibs = @("-lgdi32", "-luser32", "-lkernel32")
    }
    & gcc -std=c99 -Wall -Wextra -Wpedantic @guards -I . $c -o $exe @linkLibs 2>&1 |
        Tee-Object -FilePath (Join-Path $log "$unit.gcc.log") | Out-Null
    $gccRc = $LASTEXITCODE
    $ErrorActionPreference = $prev
    if ($gccRc -ne 0) {
        Get-Content (Join-Path $log "$unit.gcc.log")
        throw "gcc failed: $c"
    }

    & $exe
    if ($LASTEXITCODE -ne 0) { throw "$unit failed" }
    if ($unit -eq "mixa_backend_win32_selftest") {
        $win32Ok = $true
    }
}

"mixa core ok"
"mixa overlay ok"
"mixa backend ok"
"mixa pump ok"
"mixa selection ok"
if ($win32Ok) {
    "mixa win32 backend ok"
} elseif ($win32Skipped) {
    # already printed skip line
} else {
    throw "mixa win32 backend unit did not run"
}