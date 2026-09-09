# L1 self-build: gen0 seed -> gen1 -> gen2 -> gen3 check.
# L1 builder (build_l1) drives generate_all / compile_generated_tools / second pass.
# Isolated under build\l1trans\gen*. Does not touch live lm0/lm1.
$ErrorActionPreference = "Stop"
Set-Location (Join-Path $PSScriptRoot "..\..")

$log = "build\l1trans\logs"
New-Item -ItemType Directory -Force -Path $log,
    "build\obj\l1trans\gen1", "build\l1trans\gen1",
    "build\obj\l1trans\gen2", "build\l1trans\gen2",
    "build\obj\l1trans\gen3", "build\l1trans\gen3" | Out-Null
$script:genLog = Join-Path $log "gen_accept.log"
Set-Content -LiteralPath $script:genLog -Value "$(Get-Date -Format o) gen accept start"

function Write-G([string]$m) { Add-Content -LiteralPath $script:genLog -Value "$(Get-Date -Format o) $m" }

function Invoke-LoggedExe([string]$title, [string]$exe, [string[]]$argv) {
    $stdout = Join-Path $log ($title + ".stdout")
    $stderr = Join-Path $log ($title + ".stderr")
    $exeFull = Join-Path (Get-Location) $exe
    $outFull = Join-Path (Get-Location) $stdout
    $errFull = Join-Path (Get-Location) $stderr
    $quoted = foreach ($a in $argv) {
        if ($a -match '[ \t"]') { '"' + ($a -replace '"', '\"') + '"' } else { $a }
    }
    $argLine = [string]::Join(' ', [string[]]$quoted)
    Write-G "BEGIN $title exe=$exe args=$argLine"
    $p = Start-Process -FilePath $exeFull -ArgumentList $argLine -WorkingDirectory (Get-Location).Path -NoNewWindow -Wait -PassThru -RedirectStandardOutput $outFull -RedirectStandardError $errFull
    Write-G "EXIT $title $($p.ExitCode) stdout=$stdout stderr=$stderr"
    if ($p.ExitCode -ne 0) {
        if (Test-Path -LiteralPath $errFull) { Get-Content -LiteralPath $errFull | Select-Object -Last 30 }
        throw "$title failed $($p.ExitCode) (see $stdout / $stderr)"
    }
}

# Native published-C bootstrap: compile the tracked l1trans.lm1.c with gcc.
# This is the supported route for current l1src (including immutable:).
# Hosted gen0 from run_seed.ps1 / lm2 remains a separate historical seed;
# it is not used to parse current l1src and is not required here.
New-Item -ItemType Directory -Force -Path "build\l1trans\boot" | Out-Null
$bootC = "lm1\build\l1trans.lm1.c"
if (-not (Test-Path $bootC)) { throw "missing tracked bootstrap $bootC" }
cmd /c "gcc -std=c99 -Wall -Wextra -Wpedantic -I . -I lm1/build -Werror=incompatible-pointer-types -Werror=discarded-qualifiers -Werror=implicit-function-declaration -Werror=implicit-int -o build\l1trans\boot\l1trans.exe $bootC > $log\gcc_boot.log 2>&1"
if ($LASTEXITCODE -ne 0) { throw "native bootstrap gcc failed (see $log\gcc_boot.log)" }
$seed = "build\l1trans\boot\l1trans.exe"
Write-G "native boot exe ok hash=$((Get-FileHash $seed).Hash) from $bootC"

$savedHosted = @{
    LM_TRANS_REGISTRY = $env:LM_TRANS_REGISTRY
    LM_TRANS_REGISTRY_VIEW = $env:LM_TRANS_REGISTRY_VIEW
    LM_P0_REGISTRY = $env:LM_P0_REGISTRY
    LM_P0_COMPARE_REGISTRY = $env:LM_P0_COMPARE_REGISTRY
}
Write-G "ENV incoming LM_P0_REGISTRY=$($env:LM_P0_REGISTRY) LM_P0_COMPARE_REGISTRY=$($env:LM_P0_COMPARE_REGISTRY) LM_TRANS_REGISTRY=$($env:LM_TRANS_REGISTRY) LM_TRANS_REGISTRY_VIEW=$($env:LM_TRANS_REGISTRY_VIEW)"
function Restore-HostedRegistryEnv {
    foreach ($k in @("LM_TRANS_REGISTRY", "LM_TRANS_REGISTRY_VIEW", "LM_P0_REGISTRY", "LM_P0_COMPARE_REGISTRY")) {
        $v = $savedHosted[$k]
        if ($null -eq $v -or $v -eq "") { Remove-Item "Env:$k" -ErrorAction SilentlyContinue }
        else { Set-Item "Env:$k" $v }
    }
}
try {
foreach ($k in $savedHosted.Keys) { Remove-Item "Env:$k" -ErrorAction SilentlyContinue }
Write-G "ENV effective default hosted P0/trans registry for gen0 and nested run_parser/run_smoke"

Write-G "BEGIN gen1 translate"
& $seed "l1src\l1trans.lm1" "build\obj\l1trans\gen1\l1trans.c"
if ($LASTEXITCODE -ne 0) { throw "gen1 translate failed" }
cmd /c "gcc -std=c99 -Wall -Wextra -Wpedantic -I . -I lm1/build -Werror=incompatible-pointer-types -Werror=discarded-qualifiers -Werror=implicit-function-declaration -Werror=implicit-int -o build\l1trans\gen1\l1trans.exe build\obj\l1trans\gen1\l1trans.c > $log\gcc_gen1.log 2>&1"
if ($LASTEXITCODE -ne 0) { throw "gen1 gcc failed" }
Write-G "gen1 exe ok hash=$((Get-FileHash build\l1trans\gen1\l1trans.exe).Hash)"

Write-G "BEGIN gen2 translate"
& "build\l1trans\gen1\l1trans.exe" "l1src\l1trans.lm1" "build\obj\l1trans\gen2\l1trans.c"
if ($LASTEXITCODE -ne 0) { throw "gen2 translate failed" }
cmd /c "gcc -std=c99 -Wall -Wextra -Wpedantic -I . -I lm1/build -Werror=incompatible-pointer-types -Werror=discarded-qualifiers -Werror=implicit-function-declaration -Werror=implicit-int -o build\l1trans\gen2\l1trans.exe build\obj\l1trans\gen2\l1trans.c > $log\gcc_gen2.log 2>&1"
if ($LASTEXITCODE -ne 0) { throw "gen2 gcc failed" }
Write-G "gen2 exe ok hash=$((Get-FileHash build\l1trans\gen2\l1trans.exe).Hash)"

$h1 = (Get-FileHash "build\obj\l1trans\gen1\l1trans.c").Hash
$h2 = (Get-FileHash "build\obj\l1trans\gen2\l1trans.c").Hash
Write-G "gen1_c=$h1"
Write-G "gen2_c=$h2"
if ($h1 -ne $h2) { throw "gen1/gen2 C differ" }

Write-G "BEGIN gen3 check from gen2"
& "build\l1trans\gen2\l1trans.exe" "l1src\l1trans.lm1" "build\obj\l1trans\gen3\l1trans.c"
if ($LASTEXITCODE -ne 0) { throw "gen3 translate failed" }
$h3 = (Get-FileHash "build\obj\l1trans\gen3\l1trans.c").Hash
Write-G "gen3_c=$h3"
if ($h3 -ne $h2) { throw "gen2/gen3 C differ" }
cmd /c "gcc -std=c99 -Wall -Wextra -Wpedantic -I . -I lm1/build -Werror=incompatible-pointer-types -Werror=discarded-qualifiers -Werror=implicit-function-declaration -Werror=implicit-int -o build\l1trans\gen3\l1trans.exe build\obj\l1trans\gen3\l1trans.c > $log\gcc_gen3.log 2>&1"
if ($LASTEXITCODE -ne 0) { throw "gen3 gcc failed" }
& "build\l1trans\gen3\l1trans.exe" "tests\l1\integer_add.lm1" "build\obj\l1trans\gen3\integer_add.c"
if ($LASTEXITCODE -ne 0) { throw "gen3 integer_add translate failed" }
cmd /c "gcc -std=c99 -o build\l1trans\gen3\integer_add.exe build\obj\l1trans\gen3\integer_add.c"
if ($LASTEXITCODE -ne 0) { throw "gen3 integer_add gcc failed" }
& "build\l1trans\gen3\integer_add.exe"
if ($LASTEXITCODE -ne 0) { throw "gen3 integer_add run failed $LASTEXITCODE" }
Write-G "gen3 integer_add run exit 0"

Write-G "BEGIN L1 builder selftest + bootstrap gen3 tools"
& "build\l1trans\gen2\l1trans.exe" "l1src\build_l1.lm1" "build\obj\l1trans\gen2\build_l1.c"
if ($LASTEXITCODE -ne 0) { throw "build_l1 translate failed" }
cmd /c "gcc -std=c99 -Wall -Wextra -Wpedantic -I . -I lm1/build -Werror=incompatible-pointer-types -Werror=discarded-qualifiers -Werror=implicit-function-declaration -Werror=implicit-int -o build\l1trans\gen2\build_l1.exe build\obj\l1trans\gen2\build_l1.c > $log\gcc_build_l1.log 2>&1"
if ($LASTEXITCODE -ne 0) { throw "build_l1 gcc failed" }
Invoke-LoggedExe "build_l1_selftest" "build\l1trans\gen2\build_l1.exe" @("selftest")

$spaceRoot = "build\l1trans\space dir"
$spaceSeed = Join-Path $spaceRoot "seed"
$spaceObj = Join-Path $spaceRoot "obj"
$spaceBin = Join-Path $spaceRoot "bin"
New-Item -ItemType Directory -Force -Path $spaceSeed, $spaceObj, $spaceBin | Out-Null
Copy-Item "build\l1trans\gen2\l1trans.exe" (Join-Path $spaceSeed "l1trans.exe") -Force
Invoke-LoggedExe "build_l1_space" "build\l1trans\gen2\build_l1.exe" @(
    (Join-Path $spaceSeed "l1trans.exe"), $spaceObj, $spaceBin)
$spaceOut = Get-Content -LiteralPath (Join-Path $log "build_l1_space.stdout") -Raw
if ($spaceOut -notmatch [regex]::Escape('"build\l1trans\space dir\seed\l1trans.exe"')) {
    throw "spaced builder stdout missing quoted seed tool"
}
if ($spaceOut -notmatch [regex]::Escape('"build\l1trans\space dir\bin\l1trans.exe"')) {
    throw "spaced builder stdout missing quoted second-pass tool"
}
if (-not (Test-Path -LiteralPath (Join-Path $spaceBin "l1trans.exe"))) {
    throw "spaced builder did not produce bin l1trans.exe"
}
Write-G "spaced builder route exit 0 (seed+obj+bin with spaces, second pass quoted)"

Invoke-LoggedExe "build_l1_bootstrap_gen3" "build\l1trans\gen2\build_l1.exe" @(
    "build\l1trans\gen2\l1trans.exe", "build\obj\l1trans\gen3", "build\l1trans\gen3")

$env:L1_GEN = "gen2"
powershell -NoProfile -ExecutionPolicy Bypass -File tests\l1\run_smoke.ps1
if ($LASTEXITCODE -ne 0) { throw "gen2 smoke failed" }
Write-G "gen2 smoke ok"

powershell -NoProfile -ExecutionPolicy Bypass -File tests\l1\run_parser.ps1
if ($LASTEXITCODE -ne 0) { throw "gen2 parser accept failed" }
Write-G "gen2 parser accept ok"
Write-Output "l1trans gen1/gen2/gen3 accept ok"
} finally {
    Restore-HostedRegistryEnv
}
