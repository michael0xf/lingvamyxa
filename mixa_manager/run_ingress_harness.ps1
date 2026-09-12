# App-only LMX_MSG_HOST_INGRESS_V0 foreign-thread harness (Windows).
# Pins: mixa_manager/vendor/lmx_msg_host_ingress_v0 (no live l2src dependency).
# Translator: stable stg\l1_baseline\build\l1trans\gen2\l1trans.exe (full SHA256).
$ErrorActionPreference = "Stop"
Set-Location (Join-Path $PSScriptRoot "..")

$ExpectedTransSha = "65D5A5ED127CA1BAEBDD1D500A5B74CEEA63EC1985EAC52EDEF28EFEB261C936"

$trans = "stg\l1_baseline\build\l1trans\gen2\l1trans.exe"
if ($env:MIXA_L1TRANS -and $env:MIXA_L1TRANS.Trim().Length -gt 0) {
    $trans = $env:MIXA_L1TRANS.Trim()
}
if (-not (Test-Path -LiteralPath $trans)) {
    throw "missing stable L1 translator: $trans"
}

$transHash = (Get-FileHash -LiteralPath $trans -Algorithm SHA256).Hash.ToUpperInvariant()
"ingress harness translator: $trans"
"ingress harness translator sha256: $transHash"
if ($transHash -ne $ExpectedTransSha) {
    throw "stable translator sha256 mismatch: expected $ExpectedTransSha got $transHash"
}
"ingress harness translator full hash ok"

$pinInc = "mixa_manager\vendor\lmx_msg_host_ingress_v0"
$manifestPath = Join-Path $pinInc "MANIFEST.txt"
if (-not (Test-Path -LiteralPath $manifestPath)) {
    throw "missing pin manifest: $manifestPath"
}

# Verify pinned files against recorded MANIFEST sha256 BEFORE build.
$ExpectedPins = @(
    @{ Rel = "l2src\lmx_message.h";           Sha = "F70E182DFE123C8B4E6822C1DE315094B9D8F88653351FA46A3F8466644CC01D" },
    @{ Rel = "l2src\lmx_message.lm1";         Sha = "49EB0DB16902F593E65C094C16BBBE797EC5E2FD9310B89E90CBB271F54AFE3D" },
    @{ Rel = "l2src\lmx_message_host.c";      Sha = "2A6909030D24EB8969B7B90712D2B52C96BB668B6368C382ADDC2AC97CF73053" },
    @{ Rel = "l2src\lmx_message_host.h";      Sha = "B5F4EC3FB88A4CCAEE02C45A0CA3BB54B38782F6929C22A35DECD1577BDF5708" },
    @{ Rel = "LMX_MSG_HOST_INGRESS_V0.txt";   Sha = "2CB1F7354A53AC7F87BD17E3739D7B314534F4C45E280F167FC89595224EBD68" }
)
foreach ($pin in $ExpectedPins) {
    $path = Join-Path $pinInc $pin.Rel
    if (-not (Test-Path -LiteralPath $path)) {
        throw "missing pin file: $path"
    }
    $sha = (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash.ToUpperInvariant()
    if ($sha -ne $pin.Sha) {
        throw "pin sha256 mismatch for $($pin.Rel): expected $($pin.Sha) got $sha"
    }
    "ingress pin ok $($pin.Rel)"
}
"ingress manifest pin verification ok (immutable; no live-core includes)"

$pinLm1 = Join-Path $pinInc "l2src\lmx_message.lm1"
$hostC = Join-Path $pinInc "l2src\lmx_message_host.c"
$harnessC = "mixa_manager\tests\mixa_ingress_host_harness.c"
if (-not (Test-Path -LiteralPath $pinLm1)) { throw "missing pin: $pinLm1" }
if (-not (Test-Path -LiteralPath $hostC)) { throw "missing pin: $hostC" }
if (-not (Test-Path -LiteralPath $harnessC)) { throw "missing harness: $harnessC" }

$out = "build\mixa"
$log = "build\mixa\logs"
New-Item -ItemType Directory -Force -Path $out, $log | Out-Null

$guards = @(
    "-Werror=incompatible-pointer-types", "-Werror=discarded-qualifiers",
    "-Werror=implicit-function-declaration", "-Werror=implicit-int"
)
$inc = @("-I", $pinInc)

# Translate pinned portable message unit (production path; no exec).
$msgC = Join-Path $out "lmx_message_pin.c"
& $trans $pinLm1 $msgC
if ($LASTEXITCODE -ne 0) { throw "translate failed: $pinLm1" }

$msgO = Join-Path $out "lmx_message_pin.o"
$hostO = Join-Path $out "lmx_message_host_pin.o"
$harnessO = Join-Path $out "mixa_ingress_host_harness.o"
$exe = Join-Path $out "mixa_ingress_host_harness.exe"

$prev = $ErrorActionPreference
$ErrorActionPreference = "Continue"
& gcc -std=c99 -Wall -Wextra -Wpedantic @guards @inc -c $msgC -o $msgO 2>&1 |
    Tee-Object -FilePath (Join-Path $log "lmx_message_pin.gcc.log") | Out-Null
$gccRc = $LASTEXITCODE
$ErrorActionPreference = $prev
if ($gccRc -ne 0) {
    Get-Content (Join-Path $log "lmx_message_pin.gcc.log")
    throw "gcc -c failed: $msgC"
}

# Production host: WITHOUT -DLMX_MSG_HOST_TEST
$prev = $ErrorActionPreference
$ErrorActionPreference = "Continue"
& gcc -std=c99 -Wall -Wextra -Wpedantic @guards @inc -c $hostC -o $hostO 2>&1 |
    Tee-Object -FilePath (Join-Path $log "lmx_message_host_pin.gcc.log") | Out-Null
$gccRc = $LASTEXITCODE
$ErrorActionPreference = $prev
if ($gccRc -ne 0) {
    Get-Content (Join-Path $log "lmx_message_host_pin.gcc.log")
    throw "gcc -c failed: $hostC"
}

# nm: exit must succeed; production host stub get_nomem only; no test setters / mutable data.
$nmLog = Join-Path $log "lmx_message_host_pin.nm.txt"
$prev = $ErrorActionPreference
$ErrorActionPreference = "Continue"
$nmOut = & nm $hostO 2>&1
$nmRc = $LASTEXITCODE
$ErrorActionPreference = $prev
$nmOut | Tee-Object -FilePath $nmLog | Out-Null
if ($nmRc -ne 0) {
    throw "nm failed exit=$nmRc on $hostO"
}
$nmText = ($nmOut | Out-String)
if ($nmText -match "lmx_msg_host_test_set_") {
    throw "production host.o exports lmx_msg_host_test_set_* (must build without LMX_MSG_HOST_TEST)"
}
if ($nmText -match "[BbDd]\s+lmx_msg_host_test_nomem\b") {
    throw "production host.o has mutable test_nomem data symbol"
}
if ($nmText -notmatch "lmx_msg_host_test_get_nomem") {
    throw "production host.o missing immutable get_nomem stub"
}
"ingress nm check ok (nm exit=0; get_nomem stub; no test_set_*; no mutable test_nomem)"

$prev = $ErrorActionPreference
$ErrorActionPreference = "Continue"
& gcc -std=c99 -Wall -Wextra -Wpedantic @guards @inc -c $harnessC -o $harnessO 2>&1 |
    Tee-Object -FilePath (Join-Path $log "mixa_ingress_host_harness.gcc.log") | Out-Null
$gccRc = $LASTEXITCODE
$ErrorActionPreference = $prev
if ($gccRc -ne 0) {
    Get-Content (Join-Path $log "mixa_ingress_host_harness.gcc.log")
    throw "gcc -c failed: $harnessC"
}

$prev = $ErrorActionPreference
$ErrorActionPreference = "Continue"
& gcc -std=c99 -Wall -Wextra -Wpedantic @guards @inc $msgO $hostO $harnessO -o $exe -lkernel32 2>&1 |
    Tee-Object -FilePath (Join-Path $log "mixa_ingress_host_harness.link.log") | Out-Null
$gccRc = $LASTEXITCODE
$ErrorActionPreference = $prev
if ($gccRc -ne 0) {
    Get-Content (Join-Path $log "mixa_ingress_host_harness.link.log")
    throw "gcc link failed: mixa_ingress_host_harness"
}

$runLog = Join-Path $log "mixa_ingress_host_harness.run.log"
# Native stderr under $ErrorActionPreference=Stop wraps each line in a
# terminating NativeCommandError even when the exit code is exactly what's
# expected (PowerShell 5.1 behavior on `2>&1` redirection of a native exe).
# Both invocations below can legitimately write diagnostics to stderr on
# their own expected exit path, so both are run under Continue and checked
# by $LASTEXITCODE explicitly, matching the existing pattern in run_mixa.ps1
# (see its gcc invocations around lines 74-80/115-120).
$prevEap = $ErrorActionPreference
$ErrorActionPreference = "Continue"
& $exe 2>&1 | Tee-Object -FilePath $runLog
$exeRc = $LASTEXITCODE
$ErrorActionPreference = $prevEap
if ($exeRc -ne 0) {
    throw "mixa_ingress_host_harness failed exit=$exeRc"
}
"mixa ingress host harness ok"

# Failure-path safety: deterministic join-timeout injection (harness-only).
# This case is EXPECTED to exit 1 and print a diagnostic ("quiescence
# incomplete - leaving runtime (no delete while poster reachable)") to
# stderr -- that diagnostic is the evidence being tested for, not an error
# in the runner itself.
$failLog = Join-Path $log "mixa_ingress_host_harness.force_join_timeout.run.log"
$failEv = Join-Path $log "mixa_ingress_host_harness.fail_keep_runtime.txt"
if (Test-Path -LiteralPath $failEv) { Remove-Item -LiteralPath $failEv -Force }
$prevEap = $ErrorActionPreference
$ErrorActionPreference = "Continue"
& $exe --force-join-timeout 2>&1 | Tee-Object -FilePath $failLog
$failRc = $LASTEXITCODE
$ErrorActionPreference = $prevEap
if ($failRc -ne 1) {
    throw "force-join-timeout expected exit=1 got=$failRc"
}
$failStderrText = Get-Content -LiteralPath $failLog -Raw
if ($failStderrText -notmatch "quiescence incomplete") {
    throw "force-join-timeout: expected 'quiescence incomplete' diagnostic not found in captured output"
}
"mixa ingress host harness force-join-timeout expected-failure diagnostic captured ok"
if (-not (Test-Path -LiteralPath $failEv)) {
    throw "missing fail_keep_runtime evidence: $failEv"
}
$failText = Get-Content -LiteralPath $failEv -Raw
if ($failText -notmatch "fail_keep_runtime=1") { throw "fail_keep_runtime evidence missing marker" }
if ($failText -notmatch "owner_local_only=1") { throw "fail_keep_runtime evidence missing owner_local_only" }
if ($failText -notmatch "no_close_posted_event=1") { throw "fail_keep_runtime evidence missing no_close_posted_event" }
if ($failText -match "foreign_posted=") { throw "fail_keep_runtime evidence must not inspect poster fields" }
"mixa ingress fail_keep_runtime path ok"

# Without this, the script's own exit code falls through to whatever
# $LASTEXITCODE happens to be at the end -- which is 1, the EXPECTED exit
# code of the force-join-timeout sub-test above, not a script failure. That
# residue was silently reported as this whole harness failing to any caller
# (like run_mixa.ps1) checking $LASTEXITCODE after invoking this script.
exit 0
