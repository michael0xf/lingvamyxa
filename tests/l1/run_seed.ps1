# gen0: gcc of the committed lm1\build\l1trans.lm1.c, the previous binary that always
# exists (Mikhail 2026-09-15). Nothing from lm2, no trans.lm0, no oldchain archives.
# The exe is staged, checked by translating, compiling and running
# tests\l1\integer_add.lm1, then published to build\l1trans\gen0 with its build record
# l1trans.sources.txt: one "path SHA256" line for l1src\l1trans.lm1 and one for
# lm1\build\l1trans.lm1.c, hashed before the build. run_smoke compares the current files
# against that record. A failed stage leaves the published gen0 in place.
$ErrorActionPreference = "Stop"
Set-Location (Join-Path $PSScriptRoot "..\..")

$runId = Get-Date -Format "yyyyMMdd-HHmmss"
$logDir = "build\l1trans\logs"
$pubObj = "build\obj\l1trans\gen0"
$pubBin = "build\l1trans\gen0"
$stageObj = Join-Path $pubObj ("seed-" + $runId)
$stageBin = Join-Path $pubBin ("seed-" + $runId)
New-Item -ItemType Directory -Force -Path $logDir, $pubObj, $pubBin, $stageObj, $stageBin | Out-Null
$script:seedLog = Join-Path $logDir "seed.log"
Set-Content -LiteralPath $script:seedLog -Value "$(Get-Date -Format o) seed start run=$runId"

function Write-S([string]$m) { Add-Content -LiteralPath $script:seedLog -Value "$(Get-Date -Format o) $m" }

function Require-File([string]$path, [string]$role) {
    if (-not (Test-Path -LiteralPath $path)) {
        throw "missing prerequisite ($role): $path"
    }
    $h = (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash
    $n = (Get-Item -LiteralPath $path).Length
    Write-S "PREREQ $role path=$path bytes=$n sha256=$h"
    return $h
}

if (-not (Get-Command gcc -ErrorAction SilentlyContinue)) {
    throw "missing prerequisite: gcc on PATH"
}
$gccExe = (Get-Command gcc).Source
$gccVer = (gcc --version 2>&1 | Select-Object -First 1 | Out-String).Trim()
Write-S "PREREQ gcc version=$gccVer"
$null = Require-File $gccExe "gcc executable"

$srcL1 = "l1src\l1trans.lm1"
$srcC = "lm1\build\l1trans.lm1.c"
$hashL1 = Require-File $srcL1 "L1 source of the translator"
$hashC = Require-File $srcC "committed generated C, the previous binary's source"

$cFinal = Join-Path $pubObj "l1trans.c"
$exeSeed = Join-Path $stageBin "l1trans.exe"
$exeFinal = Join-Path $pubBin "l1trans.exe"
$recordSeed = Join-Path $stageBin "l1trans.sources.txt"
$recordFinal = Join-Path $pubBin "l1trans.sources.txt"
if (Test-Path -LiteralPath $exeFinal) {
    Write-S "KEEP published gen0 exe sha256=$((Get-FileHash -LiteralPath $exeFinal).Hash) until the new one verifies"
}

$savedHosted = @{
    LM_TRANS_REGISTRY = $env:LM_TRANS_REGISTRY
    LM_TRANS_REGISTRY_VIEW = $env:LM_TRANS_REGISTRY_VIEW
    LM_P0_REGISTRY = $env:LM_P0_REGISTRY
    LM_P0_COMPARE_REGISTRY = $env:LM_P0_COMPARE_REGISTRY
}
function Restore-HostedRegistryEnv {
    foreach ($k in @("LM_TRANS_REGISTRY", "LM_TRANS_REGISTRY_VIEW", "LM_P0_REGISTRY", "LM_P0_COMPARE_REGISTRY")) {
        $v = $savedHosted[$k]
        if ($null -eq $v -or $v -eq "") { Remove-Item "Env:$k" -ErrorAction SilentlyContinue }
        else { Set-Item "Env:$k" $v }
    }
}
try {
    foreach ($k in $savedHosted.Keys) { Remove-Item "Env:$k" -ErrorAction SilentlyContinue }

$gccLog = Join-Path $logDir "seed_gcc.log"
$gccCmd = "gcc -std=c99 -Wall -Wextra -Wpedantic -I . -I lm1/build -Werror=incompatible-pointer-types -Werror=discarded-qualifiers -Werror=implicit-function-declaration -Werror=implicit-int -o $exeSeed $srcC"
Write-S "CMD $gccCmd"
cmd /c "$gccCmd > $gccLog 2>&1"
$ec = $LASTEXITCODE
Write-S "EXIT gcc $ec log=$gccLog"
if ($ec -ne 0) {
    Get-Content -LiteralPath $gccLog | Select-Object -Last 40
    throw "gen0 gcc failed $ec (published gen0 left in place)"
}
if (-not (Test-Path -LiteralPath $exeSeed)) { throw "gen0 gcc exit 0 but missing $exeSeed" }
Write-S "EXE seed sha256=$((Get-FileHash -LiteralPath $exeSeed).Hash) bytes=$((Get-Item -LiteralPath $exeSeed).Length)"

$addSrc = "tests\l1\integer_add.lm1"
$null = Require-File $addSrc "integer_add source"
$addC = Join-Path $stageObj "integer_add.c"
$addExe = Join-Path $stageBin "integer_add.exe"
$addOut = Join-Path $logDir "seed_integer_add.stdout"
$addErr = Join-Path $logDir "seed_integer_add.stderr"
Write-S "CMD $exeSeed $addSrc $addC"
cmd /c "$exeSeed $addSrc $addC > $addOut 2> $addErr"
$ec = $LASTEXITCODE
Write-S "EXIT seed integer_add translate $ec stdout=$addOut stderr=$addErr"
if ($ec -ne 0) {
    if (Test-Path -LiteralPath $addErr) { Get-Content -LiteralPath $addErr }
    throw "new gen0 failed to translate integer_add (published gen0 left in place)"
}
Write-S "C integer_add sha256=$((Get-FileHash -LiteralPath $addC).Hash) bytes=$((Get-Item -LiteralPath $addC).Length)"

$addGccLog = Join-Path $logDir "seed_integer_add_gcc.log"
$addGccCmd = "gcc -std=c99 -o $addExe $addC"
Write-S "CMD $addGccCmd"
cmd /c "$addGccCmd > $addGccLog 2>&1"
$ec = $LASTEXITCODE
Write-S "EXIT gcc integer_add $ec log=$addGccLog"
if ($ec -ne 0) {
    Get-Content -LiteralPath $addGccLog
    throw "integer_add gcc failed (published gen0 left in place)"
}

$addRunOut = Join-Path $logDir "seed_integer_add_run.stdout"
$addRunErr = Join-Path $logDir "seed_integer_add_run.stderr"
Write-S "CMD $addExe"
cmd /c "$addExe > $addRunOut 2> $addRunErr"
$ec = $LASTEXITCODE
Write-S "EXIT seed integer_add run $ec stdout=$addRunOut stderr=$addRunErr"
if ($ec -ne 0) { throw "integer_add run failed $ec (published gen0 left in place)" }

[IO.File]::WriteAllLines((Join-Path (Get-Location) $recordSeed), [string[]]@("$srcL1 $hashL1", "$srcC $hashC"))
Write-S "PUBLISH C $srcC -> $cFinal (the C gen0 was compiled from)"
Copy-Item -LiteralPath $srcC -Destination $cFinal -Force
Write-S "PUBLISH exe $exeSeed -> $exeFinal"
Copy-Item -LiteralPath $exeSeed -Destination $exeFinal -Force
Write-S "PUBLISH record $recordSeed -> $recordFinal"
Copy-Item -LiteralPath $recordSeed -Destination $recordFinal -Force
Write-S "PUBLISH exe sha256=$((Get-FileHash -LiteralPath $exeFinal).Hash); record $srcL1=$hashL1 $srcC=$hashC"
Write-S "STAGING kept $stageObj $stageBin"
Write-S "seed ok"
Write-Output "l1trans gen0 seed ok"
} finally {
    Restore-HostedRegistryEnv
}
