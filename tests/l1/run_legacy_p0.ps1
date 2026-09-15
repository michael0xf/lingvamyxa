# Legacy P0 syntax corpus (recursive old *.lmx + tests/l1/return_bare*.lmx).
# Manifest pins expected_exit. Compared: the committed printTree.lm0 goldens
# (tests\l1\goldens\printTree.lm0, generated once from the lm2 chain's printTree.lm0.exe;
# see its README.txt) and the root gen2 printTree: exit, full stdout, and full P0
# code/line/column (ONE ROOT: the STG slice and its printTree are gone). A disagreement with the goldens is a question, never a
# regeneration. This is PARSE agreement, not consumer/emitter proof.

$ErrorActionPreference = "Stop"
Set-Location (Join-Path $PSScriptRoot "..\..")

$gen = "gen2"
if ($env:L1_GEN -and $env:L1_GEN.Trim().Length -gt 0) { $gen = $env:L1_GEN.Trim() }

$rootPt = "build\l1trans\$gen\printTree.exe"
$goldenDir = "tests\l1\goldens\printTree.lm0"
$manifest = "tests\l1\legacy_p0_manifest.txt"
$outDir = "build\l1trans\legacy_p0"
$log = Join-Path "build\l1trans\logs" $gen
New-Item -ItemType Directory -Force -Path $outDir, $log | Out-Null

if (-not (Test-Path -LiteralPath $rootPt)) { throw "missing $rootPt" }
if (-not (Test-Path -LiteralPath $goldenDir)) { throw "missing golden directory $goldenDir" }
if (-not (Test-Path -LiteralPath $manifest)) { throw "missing $manifest" }

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
foreach ($k in $savedHosted.Keys) { Remove-Item "Env:$k" -ErrorAction SilentlyContinue }

function Invoke-Dump([string]$exe, [string]$src, [string]$stdout, [string]$stderr) {
    $p = Start-Process -FilePath (Join-Path (Get-Location) $exe) -ArgumentList $src -NoNewWindow -Wait -PassThru -RedirectStandardOutput (Join-Path (Get-Location) $stdout) -RedirectStandardError (Join-Path (Get-Location) $stderr)
    return $p.ExitCode
}

function Get-GoldenPath([string]$key, [string]$ext, [string]$src) {
    $p = Join-Path $goldenDir ($key + $ext)
    if (-not (Test-Path -LiteralPath $p)) { throw "missing golden $p for $src" }
    return $p
}

function Get-Diag([string]$errPath) {
    if (-not (Test-Path -LiteralPath $errPath)) { return "" }
    $t = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $errPath)).Replace("`r`n", "`n").Trim()
    return $t
}

function Get-P0Loc([string]$t) {
    if ($t -match "P0 parse error (\d+) at (\d+):(\d+)") {
        return "$($Matches[1])@$($Matches[2]):$($Matches[3])"
    }
    return ""
}

$n = 0
$accept = 0
$reject = 0
$old = 0
$extra = 0
$added = 0
try {
Get-Content -LiteralPath $manifest | ForEach-Object {
    $line = $_.Trim()
    if ($line.Length -eq 0 -or $line.StartsWith("#")) { return }
    $parts = $line -split "\s+", 3
    if ($parts.Count -lt 2) { throw "bad manifest line: $line" }
    $src = $parts[0]
    $expect = [int]$parts[1]
    $note = ""
    if ($parts.Count -ge 3) { $note = $parts[2] }
    $rootExpect = $expect
    if ($note -match "root_exit=(\d+)") { $rootExpect = [int]$Matches[1] }
    # A manifest override records an intentional current-parser change against
    # the frozen lm0 oracle.
    $rootDiag = ""
    if ($note -match "root_diag=(\d+@\d+:\d+)") { $rootDiag = $Matches[1] }
    if (-not (Test-Path -LiteralPath $src)) { throw "missing fixture $src" }
    $base = ($src -replace "[\\/]", "_")
    $o0 = Get-GoldenPath $base ".stdout" $src
    $oR = Join-Path $outDir "$base.root.out"
    $eR = Join-Path $outDir "$base.root.err"
    $ec0 = [int]((Get-Content -LiteralPath (Get-GoldenPath $base ".exit" $src) -TotalCount 1).Trim())
    $ecR = Invoke-Dump $rootPt $src $oR $eR
    if ($ec0 -ne $expect) { throw "$src golden exit $ec0 expected $expect ($note)" }
    if ($ecR -ne $rootExpect) { throw "$src root printTree exit $ecR expected $rootExpect" }
    if ($rootExpect -ne $expect) {
        if ($rootExpect -eq 0) { throw "$src root_exit override must be a reject" }
        $dR = Get-Diag $eR
        $locR = Get-P0Loc $dR
        if ($rootDiag.Length -gt 0) {
            if ($locR -ne $rootDiag) { throw "$src root diag $locR expected $rootDiag : $dR" }
        } elseif ($locR -notmatch "^32@") {
            throw "$src root empty-colon diag expected code 32: $dR"
        }
        $reject++
    } elseif ($expect -eq 0) {
        $h0 = (Get-FileHash -LiteralPath $o0).Hash
        $hR = (Get-FileHash $oR).Hash
        if ($hR -ne $h0) { throw "stdout mismatch root vs golden: $src ($o0)" }
        $accept++
    } else {
        $dR = Get-Diag $eR
        if ($dR -notmatch "P0 parse error") { throw "root missing P0 parse error: $src : $dR" }
        $loc0 = (Get-Content -LiteralPath (Get-GoldenPath $base ".p0" $src) -TotalCount 1).Trim()
        if ($loc0 -notmatch "^\d+@\d+:\d+$") { throw "golden P0 location malformed: $src : $loc0" }
        if ((Get-P0Loc $dR) -ne $loc0) { throw "root diag parity: $src golden=$loc0 root=$dR" }
        $reject++
    }
    if ($src.StartsWith("tests\l1\")) { $extra++ }
    $isAdded = 0
    if ($src.StartsWith("tests\l1\")) { $isAdded = 1 }
    if ($src.StartsWith("tests\p0_tree_contract\")) { $isAdded = 1 }
    if ($src -eq "tests\invalid_empty_colon_standalone.lmx" -or $src -eq "tests\invalid_empty_colon_nested.lmx" -or $src -eq "tests\ok_colon_vertical_body.lmx" -or $src -eq "tests\ok_empty_compact_frame.lmx") { $isAdded = 1 }
    if ($isAdded -ne 0) { $added++ } else { $old++ }
    $n++
}

Write-Output "legacy P0 corpus ok n=$n old119=$old added=$added extra_l1=$extra accept=$accept reject=$reject oracle=goldens/printTree.lm0 root=$gen"
} finally {
    Restore-HostedRegistryEnv
}
