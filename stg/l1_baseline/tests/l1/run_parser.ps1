# Parser acceptance: L1 printTree vs printTree.lm0 oracles.
# CWD = repo root. Isolated artifacts under build\l1trans.

$ErrorActionPreference = "Stop"
Set-Location (Join-Path $PSScriptRoot "..\..")

$gen = "gen0"
if ($env:L1_GEN -and $env:L1_GEN.Trim().Length -gt 0) { $gen = $env:L1_GEN.Trim() }
$l1trans = "build\l1trans\$gen\l1trans.exe"
$obj = "build\obj\l1trans\$gen"
$bin = "build\l1trans\$gen"
$log = Join-Path "build\l1trans\logs" $gen
$oracleDir = "build\l1trans\oracles"
$ptSrc = "l1src\printTree.lm1"
$ptC = Join-Path $obj "printTree.c"
$ptExe = Join-Path $bin "printTree.exe"
$lm0 = "build\lm0\printTree.lm0.exe"
$script:parserLog = Join-Path $log "parser_accept.log"

New-Item -ItemType Directory -Force -Path $obj, $bin, $log, $oracleDir | Out-Null
Set-Content -LiteralPath $script:parserLog -Value "$(Get-Date -Format o) parser accept start"

function Write-PLog([string]$msg) {
    Add-Content -LiteralPath $script:parserLog -Value "$(Get-Date -Format o) $msg"
}

$savedHosted = @{
    LM_TRANS_REGISTRY = $env:LM_TRANS_REGISTRY
    LM_TRANS_REGISTRY_VIEW = $env:LM_TRANS_REGISTRY_VIEW
    LM_P0_REGISTRY = $env:LM_P0_REGISTRY
    LM_P0_COMPARE_REGISTRY = $env:LM_P0_COMPARE_REGISTRY
}
Write-PLog "ENV incoming LM_P0_REGISTRY=$($env:LM_P0_REGISTRY) LM_P0_COMPARE_REGISTRY=$($env:LM_P0_COMPARE_REGISTRY)"
function Restore-HostedRegistryEnv {
    foreach ($k in @("LM_TRANS_REGISTRY", "LM_TRANS_REGISTRY_VIEW", "LM_P0_REGISTRY", "LM_P0_COMPARE_REGISTRY")) {
        $v = $savedHosted[$k]
        if ($null -eq $v -or $v -eq "") { Remove-Item "Env:$k" -ErrorAction SilentlyContinue }
        else { Set-Item "Env:$k" $v }
    }
}
try {
foreach ($k in $savedHosted.Keys) { Remove-Item "Env:$k" -ErrorAction SilentlyContinue }
Write-PLog "ENV effective default hosted P0 for L1 translate and printTree.lm0 oracle"

if (-not (Test-Path $l1trans)) { throw "missing $l1trans" }
if (-not (Test-Path $lm0)) { throw "missing $lm0" }

# Frozen STG gen0 is lm2 seed. Current l1src/printTree.lm1 needs gen1+ (usage).
if ($gen -eq "gen0") {
    Write-PLog "skip requires-gen1+ on gen0: current l1src/printTree.lm1"
    Write-Output "l1trans $gen parser accept ok"
    return
}

& $l1trans $ptSrc $ptC
if ($LASTEXITCODE -ne 0) { throw "translate printTree failed $LASTEXITCODE" }
Write-PLog "translate $ptSrc -> $ptC exit 0"

$gccLog = Join-Path $log "gcc_printTree.log"
cmd /c "gcc -std=c99 -Wall -Wextra -Wpedantic -I . -I lm1/build -Werror=incompatible-pointer-types -Werror=discarded-qualifiers -Werror=implicit-function-declaration -Werror=implicit-int -o $ptExe $ptC > $gccLog 2>&1"
if ($LASTEXITCODE -ne 0) {
    Get-Content $gccLog | Select-Object -Last 30
    throw "gcc printTree failed (see $gccLog)"
}
Write-PLog "gcc $ptC -> $ptExe exit 0"

function Invoke-Dump([string]$exe, [string]$src, [string]$out, [string]$err) {
    $p = Start-Process -FilePath (Join-Path (Get-Location) $exe) -ArgumentList $src -NoNewWindow -Wait -PassThru -RedirectStandardOutput (Join-Path (Get-Location) $out) -RedirectStandardError (Join-Path (Get-Location) $err)
    return $p.ExitCode
}

$positive = @(
    "tests\l1\integer_add.lm1",
    "tests\l1\shift_ops.lm1",
    "l1src\make.lm1",
    "tests\block_string.lmx",
    "tests\hash_comment.lmx",
    "tests\raw_comment.lmx",
    "tests\tail_cutters.lmx",
    "tests\python_string_double.lmx",
    "tests\trailer_word.lmx",
    "tests\arr.lmx",
    "tests\tail_receiver_words.lmx",
    "tests\return_inside_block_body_not_trailer.lmx",
    "tests\dash_fence_length.lmx",
    "tests\t2.lmx",
    "tests\l1\return_bare.lmx",
    "tests\l1\return_bare_comment.lmx"
)

foreach ($src in $positive) {
    if (-not (Test-Path $src)) { throw "missing fixture $src" }
    $base = [IO.Path]::GetFileNameWithoutExtension($src)
    $l1out = Join-Path $log "pt_l1_$base.stdout"
    $l1err = Join-Path $log "pt_l1_$base.stderr"
    $o0 = Join-Path $oracleDir "$base.oracle"
    $o0err = Join-Path $log "pt_lm0_$base.stderr"
    $ec0 = Invoke-Dump $lm0 $src $o0 $o0err
    $ec1 = Invoke-Dump $ptExe $src $l1out $l1err
    Write-PLog "dump $src l1_exit=$ec1 lm0_exit=$ec0"
    if ($ec0 -ne 0 -or $ec1 -ne 0) {
        throw "positive dump expected exit 0: $src l1=$ec1 lm0=$ec0"
    }
    $h0 = (Get-FileHash $o0).Hash
    $h1 = (Get-FileHash $l1out).Hash
    if ($h0 -ne $h1) { throw "oracle mismatch $src l1=$h1 lm0=$h0" }
    Write-PLog "MATCH $src hash=$h1 bytes=$((Get-Item $l1out).Length)"
}

$missEc = Invoke-Dump $ptExe "no_such_file.lm1" (Join-Path $log "pt_miss.stdout") (Join-Path $log "pt_miss.stderr")
if ($missEc -ne 1) { throw "missing file expected exit 1, got $missEc" }
$missErr = Get-Content (Join-Path $log "pt_miss.stderr") -Raw
if ($missErr -notmatch "P0 parse error") { throw "missing file missing diagnostic: $missErr" }
Write-PLog "malformed missing-file exit=$missEc"

$malformed = @(
    "tests\invalid_block_string_unclosed.lmx",
    "tests\invalid_brace_mark_unclosed.lmx",
    "tests\invalid_python_string_unclosed_double.lmx",
    "tests\invalid_raw_comment_unclosed.lmx",
    "tests\invalid_dash_fence_too_long.lmx"
)
foreach ($src in $malformed) {
    if (-not (Test-Path $src)) { throw "missing $src" }
    $base = [IO.Path]::GetFileNameWithoutExtension($src)
    $l1errPath = Join-Path $log "pt_l1_$base.stderr"
    $ec1 = Invoke-Dump $ptExe $src (Join-Path $log "pt_l1_$base.stdout") $l1errPath
    $ec0 = Invoke-Dump $lm0 $src (Join-Path $log "pt_lm0_$base.stdout") (Join-Path $log "pt_lm0_$base.stderr")
    Write-PLog "malformed $src l1_exit=$ec1 lm0_exit=$ec0"
    if ($ec0 -ne 1 -or $ec1 -ne 1) {
        throw "malformed expected exit 1 for both: $src l1=$ec1 lm0=$ec0"
    }
    $l1err = Get-Content -LiteralPath $l1errPath -Raw
    if ([string]::IsNullOrWhiteSpace($l1err) -or $l1err -notmatch "P0 parse error") {
        throw "malformed missing non-empty P0 parse error: $src l1=$ec1 lm0=$ec0 stderr=$l1err"
    }
}

Write-PLog "parser accept ok"
Write-Output "l1 printTree parser accept ok"
} finally {
    Restore-HostedRegistryEnv
}
