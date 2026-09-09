# Mixa Manager: TextRect + OverlayRect/composite + headless backend + pump + selection + draw + Win32 backend selftests.
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

# Record what produced this run. TWO SEPARATE IDENTITIES, and no conclusion
# drawn from them - the link between them is not something this script can
# establish.
#
# Three versions of these lines have now been wrong, each in the same way: I
# made them CONCLUDE something. First a "generated-source fingerprint" that went
# stale and reported "unchanged" across a translator swap. Then a warning that a
# binary newer than its source meant it was not built from it - which is
# backwards, since compiling a source is exactly what makes a binary newer than
# it, so that would fire on every normal build.
#
# Provenance needs a build manifest linking a binary to a source. There is none
# here, so the honest output is two hashes and the timestamps as plain facts.
# Whether they correspond is decided elsewhere, from the promoted revision, not
# inferred in a test runner.
$trackedC = "stg/l1_baseline/lm1/build/l1trans.lm1.c"
"translator: $trans"
"translator binary sha256: " + (Get-FileHash -LiteralPath $trans -Algorithm SHA256).Hash
"translator binary mtime:  " + (Get-Item -LiteralPath $trans).LastWriteTime.ToString('yyyy-MM-dd HH:mm:ss')
if ($env:MIXA_L1TRANS -and $env:MIXA_L1TRANS.Trim().Length -gt 0) {
    "tracked source: (not checked - MIXA_L1TRANS override in use)"
} elseif (Test-Path -LiteralPath $trackedC) {
    "tracked source sha256:    " + (Get-FileHash -LiteralPath $trackedC -Algorithm SHA256).Hash
    "tracked source mtime:     " + (Get-Item -LiteralPath $trackedC).LastWriteTime.ToString('yyyy-MM-dd HH:mm:ss')
    "  (two identities; this runner does not establish that one produced the other)"
} else {
    "tracked source: (absent)"
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
    "mixa_draw_selftest",
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
"mixa draw ok"
if ($win32Ok) {
    "mixa win32 backend ok"
} elseif ($win32Skipped) {
    # already printed skip line
} else {
    throw "mixa win32 backend unit did not run"
}