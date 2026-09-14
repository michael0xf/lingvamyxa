# App-only FILE_SEAM section 3.5 selftest (Windows).
# Translator: stable stg\l1_baseline\build\l1trans\gen2\l1trans.exe (full SHA256).
$ErrorActionPreference = "Stop"
. (Join-Path $PSScriptRoot "lib_l2_runtime_support.ps1")
$ExpectedTransSha = Get-L1Pin -L1Root (Join-Path $PSScriptRoot "..\stg\l1_baseline")
Set-Location (Join-Path $PSScriptRoot "..")

$trans = "stg\l1_baseline\build\l1trans\gen2\l1trans.exe"
if ($env:MIXA_L1TRANS -and $env:MIXA_L1TRANS.Trim().Length -gt 0) {
    $trans = $env:MIXA_L1TRANS.Trim()
}
if (-not (Test-Path -LiteralPath $trans)) {
    throw "missing stable L1 translator: $trans"
}

$transHash = (Get-FileHash -LiteralPath $trans -Algorithm SHA256).Hash.ToUpperInvariant()
"file selftest translator: $trans"
"file selftest translator sha256: $transHash"
if ($transHash -ne $ExpectedTransSha) {
    throw "stable translator sha256 mismatch: expected $ExpectedTransSha got $transHash"
}
"file selftest translator full hash ok (stable65D5)"

$out = "build\mixa"
$log = "build\mixa\logs"
New-Item -ItemType Directory -Force -Path $out, $log | Out-Null

# Fresh fixture/evidence (preserve unrelated logs).
$fix = @(
    "build\mixa\tmp_file_seam\console_ascii.log",
    "build\mixa\logs\mixa_file_selftest.evidence.txt",
    "build\mixa\logs\mixa_file_selftest.run.log",
    "build\mixa\logs\mixa_file_selftest.gcc.log"
)
foreach ($f in $fix) {
    if (Test-Path -LiteralPath $f) { Remove-Item -LiteralPath $f -Force }
}

$guards = @(
    "-Werror=incompatible-pointer-types", "-Werror=discarded-qualifiers",
    "-Werror=implicit-function-declaration", "-Werror=implicit-int"
)

$src = "mixa_manager\tests\mixa_file_selftest.lm1"
$cFile = Join-Path $out "mixa_file_selftest.c"
$exe = Join-Path $out "mixa_file_selftest.exe"

& $trans $src $cFile
if ($LASTEXITCODE -ne 0) { throw "translate failed: $src" }

$prev = $ErrorActionPreference
$ErrorActionPreference = "Continue"
& gcc -std=c99 -Wall -Wextra -Wpedantic @guards -I . $cFile -o $exe -lkernel32 2>&1 |
    Tee-Object -FilePath (Join-Path $log "mixa_file_selftest.gcc.log") | Out-Null
$gccRc = $LASTEXITCODE
$ErrorActionPreference = $prev
if ($gccRc -ne 0) {
    Get-Content (Join-Path $log "mixa_file_selftest.gcc.log")
    throw "gcc failed: $cFile"
}

$runLog = Join-Path $log "mixa_file_selftest.run.log"
& $exe 2>&1 | Tee-Object -FilePath $runLog
if ($LASTEXITCODE -ne 0) {
    throw "mixa_file_selftest failed exit=$LASTEXITCODE"
}
$ev = Join-Path $log "mixa_file_selftest.evidence.txt"
if (-not (Test-Path -LiteralPath $ev)) {
    throw "missing file selftest evidence: $ev"
}
"mixa file selftest ok"
