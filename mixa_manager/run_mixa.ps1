# Mixa Manager: TextRect + OverlayRect/composite + headless backend + pump + selection + draw + tiles/buttons + Win32 backend selftests.
# Build root is the repository root (parent of mixa_manager).
# Translator is the STABLE L1 under stg\l1_baseline, not the live tree.
#
# Profiles:
#   default on Windows_NT: headless + Win32 (ctors_win32), full suite
#   MIXA_HEADLESS_ONLY=1 : headless-only link (ctors_headless), no Win32 objects/libs
#   non-Windows          : headless-only (same as MIXA_HEADLESS_ONLY)
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

$wantWin32 = ($env:OS -eq "Windows_NT") -and -not (
    $env:MIXA_HEADLESS_ONLY -and $env:MIXA_HEADLESS_ONLY.Trim() -match '^(1|true|yes)$'
)
if ($wantWin32) {
    "mixa profile: windows (headless+win32)"
} else {
    "mixa profile: headless-only"
}

$units = @(
    "mixa_core_selftest",
    "mixa_overlay_selftest",
    "mixa_selection_selftest",
    "mixa_draw_selftest",
    "mixa_highlight_selftest",
    "mixa_pointer_selftest",
    "mixa_tiles_selftest",
    "mixa_buttons_selftest"
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

# ---- Portable backend stack: event_fifo + table + headless + per-build ctors ----
$backendOk = $false
$pumpOk = $false
$win32Ok = $false
$win32Skipped = -not $wantWin32
$tableOk = $false

$libUnits = @(
    @{ Name = "mixa_event_fifo"; Src = "mixa_manager\mixa_event_fifo.lm1" },
    @{ Name = "mixa_backend_table"; Src = "mixa_manager\mixa_backend_table.lm1" },
    @{ Name = "mixa_backend_headless"; Src = "mixa_manager\mixa_backend_headless.lm1" }
)
if ($wantWin32) {
    $libUnits += @{ Name = "mixa_backend_win32"; Src = "mixa_manager\mixa_backend_win32.lm1" }
    $libUnits += @{ Name = "mixa_backend_ctors"; Src = "mixa_manager\mixa_backend_ctors_win32.lm1" }
} else {
    $libUnits += @{ Name = "mixa_backend_ctors"; Src = "mixa_manager\mixa_backend_ctors_headless.lm1" }
}

$libObjs = @()
foreach ($lu in $libUnits) {
    $cFile = Join-Path $out ($lu.Name + ".c")
    $oFile = Join-Path $out ($lu.Name + ".o")
    & $trans $lu.Src $cFile
    if ($LASTEXITCODE -ne 0) { throw "translate failed: $($lu.Src)" }
    $prev = $ErrorActionPreference
    $ErrorActionPreference = "Continue"
    & gcc -std=c99 -Wall -Wextra -Wpedantic @guards -I . -c $cFile -o $oFile 2>&1 |
        Tee-Object -FilePath (Join-Path $log ($lu.Name + ".gcc.log")) | Out-Null
    $gccRc = $LASTEXITCODE
    $ErrorActionPreference = $prev
    if ($gccRc -ne 0) {
        Get-Content (Join-Path $log ($lu.Name + ".gcc.log"))
        throw "gcc -c failed: $cFile"
    }
    $libObjs += $oFile
}

function Invoke-MixaLinkedSelftest {
    param(
        [string]$Name,
        [string]$Src,
        [string[]]$Objs,
        [string[]]$LinkLibs
    )
    $cFile = Join-Path $out ($Name + ".c")
    $oFile = Join-Path $out ($Name + ".o")
    $exe = Join-Path $out ($Name + ".exe")
    & $trans $Src $cFile
    if ($LASTEXITCODE -ne 0) { throw "translate failed: $Src" }
    $prev = $ErrorActionPreference
    $ErrorActionPreference = "Continue"
    & gcc -std=c99 -Wall -Wextra -Wpedantic @guards -I . -c $cFile -o $oFile 2>&1 |
        Tee-Object -FilePath (Join-Path $log ($Name + ".gcc.log")) | Out-Null
    $gccRc = $LASTEXITCODE
    $ErrorActionPreference = $prev
    if ($gccRc -ne 0) {
        Get-Content (Join-Path $log ($Name + ".gcc.log"))
        throw "gcc -c failed: $cFile"
    }
    $prev = $ErrorActionPreference
    $ErrorActionPreference = "Continue"
    & gcc -std=c99 -Wall -Wextra -Wpedantic @guards -I . @Objs $oFile -o $exe @LinkLibs 2>&1 |
        Tee-Object -FilePath (Join-Path $log ($Name + ".link.log")) | Out-Null
    $gccRc = $LASTEXITCODE
    $ErrorActionPreference = $prev
    if ($gccRc -ne 0) {
        Get-Content (Join-Path $log ($Name + ".link.log"))
        throw "gcc link failed: $Name"
    }
    & $exe
    if ($LASTEXITCODE -ne 0) { throw "$Name failed" }
}

# Link libs: only needed when this build includes the Win32 object.
$linkLibs = @()
if ($wantWin32) {
    $linkLibs = @("-lgdi32", "-luser32", "-lkernel32")
}
Invoke-MixaLinkedSelftest -Name "mixa_backend_selftest" -Src "mixa_manager\tests\mixa_backend_selftest.lm1" -Objs $libObjs -LinkLibs $linkLibs
$backendOk = $true
Invoke-MixaLinkedSelftest -Name "mixa_pump_selftest" -Src "mixa_manager\tests\mixa_pump_selftest.lm1" -Objs $libObjs -LinkLibs $linkLibs
$pumpOk = $true
Invoke-MixaLinkedSelftest -Name "mixa_backend_table_selftest" -Src "mixa_manager\tests\mixa_backend_table_selftest.lm1" -Objs $libObjs -LinkLibs $linkLibs
$tableOk = $true
Invoke-MixaLinkedSelftest -Name "mixa_pointer_glyph_selftest" -Src "mixa_manager\tests\mixa_pointer_glyph_selftest.lm1" -Objs $libObjs -LinkLibs $linkLibs

if ($wantWin32) {
    Invoke-MixaLinkedSelftest -Name "mixa_pointer_glyph_diag_selftest" -Src "mixa_manager\tests\mixa_pointer_glyph_diag_selftest.lm1" -Objs $libObjs -LinkLibs @("-lgdi32", "-luser32", "-lkernel32")
    Invoke-MixaLinkedSelftest -Name "mixa_backend_win32_selftest" -Src "mixa_manager\tests\mixa_backend_win32_selftest.lm1" -Objs $libObjs -LinkLibs @("-lgdi32", "-luser32", "-lkernel32")
    $win32Ok = $true
    # Isolated host-ingress harness (pinned vendor); does not rewrite backend poll.
    & (Join-Path $PSScriptRoot "run_ingress_harness.ps1")
    if ($LASTEXITCODE -ne 0) { throw "mixa ingress host harness failed" }
    "mixa ingress host harness ok"
} else {
    "mixa win32 backend skipped (headless-only profile)"
}

"mixa core ok"
"mixa overlay ok"
if (-not $backendOk) { throw "mixa backend unit did not run" }
"mixa backend ok"
if (-not $pumpOk) { throw "mixa pump unit did not run" }
"mixa pump ok"
"mixa selection ok"
"mixa draw ok"
"mixa pointer ok"
"mixa tiles ok"
"mixa buttons ok"
if ($win32Ok) {
    "mixa win32 backend ok"
} elseif ($win32Skipped) {
    # skip already printed
} else {
    throw "mixa win32 backend unit did not run"
}
if (-not $tableOk) { throw "mixa backend table unit did not run" }
"mixa backend table ok"
