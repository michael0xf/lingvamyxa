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

foreach ($unit in @("lmx_selftest", "lmx_pool_selftest", "lmx_chars_selftest", "lmx_ref_selftest", "lmx_branch_selftest", "lmx_own_selftest", "lmx_size_selftest", "lmx_message_selftest")) {
    $src = "l2src\$unit.lm1"
    $c = Join-Path $out "$unit.c"
    $exe = Join-Path $out "$unit.exe"

    & $trans $src $c
    if ($LASTEXITCODE -ne 0) { throw "$gen translate failed: $src" }

    $extra = @()
    $defs = @()
    if ($unit -eq "lmx_message_selftest") {
        $extra = @("l2src\lmx_message_host.c")
        $defs = @("-DLMX_MSG_HOST_TEST")
    }
    & gcc -std=c99 -Wall -Wextra -Wpedantic @guards -I . -I lm1/build @defs $c @extra -o $exe 2>&1 |
        Tee-Object -FilePath (Join-Path $log "$unit.gcc.log") | Out-Null
    if ($LASTEXITCODE -ne 0) {
        Get-Content (Join-Path $log "$unit.gcc.log")
        throw "$gen gcc failed: $c"
    }

    & $exe
    if ($LASTEXITCODE -ne 0) { throw "$gen $unit failed" }
}
$msgC = Join-Path $out "lmx_message.c"
& $trans "l2src\lmx_message.lm1" $msgC
if ($LASTEXITCODE -ne 0) { throw "$gen translate failed: l2src\lmx_message.lm1" }
$prodHostO = Join-Path $out "lmx_message_host_prod.o"
& gcc -std=c99 -Wall -Wextra -Wpedantic @guards -I . -I lm1/build -c "l2src\lmx_message_host.c" -o $prodHostO 2>&1 |
    Tee-Object -FilePath (Join-Path $log "lmx_message_host_prod.gcc.log") | Out-Null
if ($LASTEXITCODE -ne 0) { throw "$gen gcc failed: production host.o" }
$prodNm = & nm --defined-only $prodHostO 2>&1 | Out-String
if ($prodNm -match 'lmx_msg_host_test_set_') { throw "production host.o exports test setters" }
if ($prodNm -cmatch '(?m)\s[A-Z]\s+lmx_msg_host_test_nomem\s*$') { throw "production host.o has mutable test_nomem" }
$hostExe = Join-Path $out "lmx_message_host_selftest.exe"
& gcc -std=c99 -Wall -Wextra -Wpedantic @guards -I . -I lm1/build -DLMX_MSG_HOST_TEST "l2src\lmx_message_host_selftest.c" $msgC "l2src\lmx_message_host.c" -o $hostExe 2>&1 |
    Tee-Object -FilePath (Join-Path $log "lmx_message_host_selftest.gcc.log") | Out-Null
if ($LASTEXITCODE -ne 0) {
    Get-Content (Join-Path $log "lmx_message_host_selftest.gcc.log")
    throw "$gen gcc failed: lmx_message_host_selftest"
}
& $hostExe
if ($LASTEXITCODE -ne 0) { throw "$gen lmx_message_host_selftest failed" }
"l2 lmx $gen ok"
$suiteLog = Join-Path $log "lmx_suite.log"
$toolHash = (Get-FileHash -Algorithm SHA256 (Join-Path (Get-Location) $trans)).Hash
$evPath = Join-Path $log "lmx_message_host_selftest.evidence.txt"
$ev = @()
if (Test-Path -LiteralPath $evPath) { $ev = Get-Content -LiteralPath $evPath }
@(
    "cmd=l2src\run_lmx.ps1"
    "L1_GEN=$gen"
    "l1trans=$trans"
    "l1trans_sha256=$toolHash"
    "banner=l2 lmx $gen ok"
    "exit=0"
    "host_selftest_stdout=lmx_message_host ok (see evidence)"
) + $ev | Set-Content -LiteralPath $suiteLog -Encoding utf8
