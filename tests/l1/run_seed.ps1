# Stage-0 gen0 seed: lm2/l1trans.lm2 via existing trans.lm0 + hosted parser/own.
# Default hosted registry profile (env overrides cleared for trans.lm0 and seed smoke):
#   LM_TRANS_REGISTRY unset     -> lm2/trans_registry.lm2
#   LM_P0_REGISTRY unset        -> lm2/parser_registry.lm2
#   LM_TRANS_REGISTRY_VIEW unset -> view_mode 0 (legacy)
# Bootstrap files actually selected by that profile:
#   lm2/core.lm2, lm2/primitive.lm2, lm2/convert.lm2,
#   lm2/trans_registry.lm2, lm2/parser_registry.lm2
# convert_impl.lm2 is a convert-table impl path, not a bootstrap candidate.
# Does not run port_*.py. Does not touch lm1/build, live build/lm0, frozen/prev, R2.
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
        throw "missing external prerequisite ($role): $path"
    }
    $h = (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash
    $n = (Get-Item -LiteralPath $path).Length
    Write-S "PREREQ $role path=$path bytes=$n sha256=$h"
}

if (-not (Get-Command gcc -ErrorAction SilentlyContinue)) {
    throw "missing external prerequisite: gcc on PATH"
}
$gccExe = (Get-Command gcc).Source
$gccVer = (gcc --version 2>&1 | Select-Object -First 1 | Out-String).Trim()
Write-S "PREREQ gcc version=$gccVer"
Require-File $gccExe "gcc executable"

$trans = "build\lm0\trans.lm0.exe"
$libParser = "build\lm0\libparser.lm0.a"
$libOwn = "build\lm0\libown.lm0.a"
$src = "lm2\l1trans.lm2"
$abi = "lm2\parser_abi.lm2"
Require-File $trans "trans.lm0"
Require-File $libParser "libparser.lm0"
Require-File $libOwn "libown.lm0"
Require-File $src "seed source"
Require-File $abi "parser_abi predef"
if (-not (Test-Path -LiteralPath "lm1")) {
    throw "missing external prerequisite (include dir used by the working seed gcc): lm1"
}
Write-S "PREREQ include dir=lm1"

Require-File "lm2\core.lm2" "registry core (default candidate 0)"
Require-File "lm2\primitive.lm2" "registry primitive (required)"
Require-File "lm2\convert.lm2" "registry convert (required)"
Require-File "lm2\trans_registry.lm2" "trans_registry (default candidate 0)"
Require-File "lm2\parser_registry.lm2" "parser_registry (default candidate 0)"

Write-S "ENV incoming LM_TRANS_REGISTRY=$($env:LM_TRANS_REGISTRY)"
Write-S "ENV incoming LM_TRANS_REGISTRY_VIEW=$($env:LM_TRANS_REGISTRY_VIEW)"
Write-S "ENV incoming LM_P0_REGISTRY=$($env:LM_P0_REGISTRY)"
Write-S "ENV incoming LM_P0_COMPARE_REGISTRY=$($env:LM_P0_COMPARE_REGISTRY)"

$cSeed = Join-Path $stageObj "l1trans.c"
$exeSeed = Join-Path $stageBin "l1trans.exe"
$cFinal = Join-Path $pubObj "l1trans.c"
$exeFinal = Join-Path $pubBin "l1trans.exe"
if (Test-Path -LiteralPath $exeFinal) {
    Write-S "KEEP published gen0 exe sha256=$((Get-FileHash -LiteralPath $exeFinal).Hash) until new seed verifies"
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
$transOut = Join-Path $logDir "seed_trans.stdout"
$transErr = Join-Path $logDir "seed_trans.stderr"
$transCmd = "$trans $src $cSeed"
Write-S "CMD $transCmd"
try {
    foreach ($k in $savedHosted.Keys) { Remove-Item "Env:$k" -ErrorAction SilentlyContinue }
    Write-S "ENV effective profile=default hosted for trans.lm0 and gen0 seed smoke: LM_TRANS_REGISTRY unset, LM_P0_REGISTRY unset, LM_TRANS_REGISTRY_VIEW unset (view_mode 0), LM_P0_COMPARE_REGISTRY unset (does not select the registry file)"
    cmd /c "$trans $src $cSeed > $transOut 2> $transErr"
    $ec = $LASTEXITCODE
Write-S "EXIT trans.lm0 $ec stdout=$transOut stderr=$transErr"
if ($ec -ne 0) {
    if (Test-Path -LiteralPath $transErr) { Get-Content -LiteralPath $transErr | Select-Object -Last 40 }
    throw "trans.lm0 failed $ec (published gen0 left in place)"
}
if (-not (Test-Path -LiteralPath $cSeed)) { throw "trans.lm0 exit 0 but missing $cSeed" }
Write-S "C seed sha256=$((Get-FileHash -LiteralPath $cSeed).Hash) bytes=$((Get-Item -LiteralPath $cSeed).Length)"

$gccLog = Join-Path $logDir "seed_gcc.log"
$gccCmd = "gcc -std=c99 -Wall -Wextra -Wpedantic -I lm1 -o $exeSeed $cSeed $libParser $libOwn"
Write-S "CMD $gccCmd"
cmd /c "$gccCmd > $gccLog 2>&1"
$ec = $LASTEXITCODE
Write-S "EXIT gcc $ec log=$gccLog"
if ($ec -ne 0) {
    Get-Content -LiteralPath $gccLog | Select-Object -Last 40
    throw "seed gcc failed $ec (published gen0 left in place)"
}
if (-not (Test-Path -LiteralPath $exeSeed)) { throw "seed gcc exit 0 but missing $exeSeed" }
Write-S "EXE seed sha256=$((Get-FileHash -LiteralPath $exeSeed).Hash) bytes=$((Get-Item -LiteralPath $exeSeed).Length)"

$addSrc = "tests\l1\integer_add.lm1"
Require-File $addSrc "integer_add source"
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
    throw "new seed failed to translate integer_add (published gen0 left in place)"
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
Write-S "EXE integer_add sha256=$((Get-FileHash -LiteralPath $addExe).Hash) bytes=$((Get-Item -LiteralPath $addExe).Length)"

$addRunOut = Join-Path $logDir "seed_integer_add_run.stdout"
$addRunErr = Join-Path $logDir "seed_integer_add_run.stderr"
Write-S "CMD $addExe"
cmd /c "$addExe > $addRunOut 2> $addRunErr"
$ec = $LASTEXITCODE
Write-S "EXIT seed integer_add run $ec stdout=$addRunOut stderr=$addRunErr"
if ($ec -ne 0) { throw "integer_add run failed $ec (published gen0 left in place)" }

Write-S "PUBLISH C $cSeed -> $cFinal (separate from exe; not an atomic pair)"
Copy-Item -LiteralPath $cSeed -Destination $cFinal -Force
Write-S "PUBLISH C sha256=$((Get-FileHash -LiteralPath $cFinal).Hash)"
Write-S "PUBLISH exe $exeSeed -> $exeFinal (separate from C; not an atomic pair)"
Copy-Item -LiteralPath $exeSeed -Destination $exeFinal -Force
Write-S "PUBLISH exe sha256=$((Get-FileHash -LiteralPath $exeFinal).Hash)"
Write-S "STAGING kept $stageObj $stageBin"
Write-S "seed ok"
Write-Output "l1trans gen0 seed ok"
} finally {
    Restore-HostedRegistryEnv
}
