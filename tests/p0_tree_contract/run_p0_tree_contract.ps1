# Compare root gen2 printTree to pinned 620db86 goldens in this directory.
# Does not regenerate goldens. Versioned new-P0 rejects are listed below.
$ErrorActionPreference = "Stop"
Set-Location (Join-Path $PSScriptRoot "..\..")
$here = "tests\p0_tree_contract"
$pt = "build\l1trans\gen2\printTree.exe"
if (-not (Test-Path $pt)) { throw "missing $pt" }

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

# stem -> pinned exit and exact stderr (trimmed). printTree always exits 1 on
# parse failure (not the P0 code). C_nested_short is rejected by both 620 and
# current P0. return_colon_* are versioned: 620 accepts, current P0 rejects.
$expectReject = @{
    "C_nested_short" = @{
        Exit = 1
        Diag = "P0 parse error 13 at 2:5: source level increase must be one step"
    }
    "return_colon_empty_trailer" = @{
        Exit = 1
        Diag = "P0 parse error 32 at 1:1: empty colon Frame is not allowed"
    }
    "return_colon_comment_trailer" = @{
        Exit = 1
        Diag = "P0 parse error 32 at 1:1: empty colon Frame is not allowed"
    }
}

function Test-OrdinalEq([string]$a, [string]$b) {
    return [string]::Compare($a, $b, [System.StringComparison]::Ordinal) -eq 0
}

$n = 0; $ok = 0; $rej = 0
try {
Get-ChildItem $here -Filter "*.lmx" | Sort-Object Name | ForEach-Object {
    $stem = $_.BaseName
    $src = $_.FullName
    $gold = Join-Path $here ($stem + ".tree.txt")
    $out = Join-Path "build\p0_tree_contract" ($stem + ".root.out")
    $err = Join-Path "build\p0_tree_contract" ($stem + ".root.err")
    New-Item -ItemType Directory -Force -Path "build\p0_tree_contract" | Out-Null
    $p = Start-Process -FilePath (Join-Path (Get-Location) $pt) -ArgumentList $src -NoNewWindow -Wait -PassThru -RedirectStandardOutput (Join-Path (Get-Location) $out) -RedirectStandardError (Join-Path (Get-Location) $err)
    $n++
    if ($expectReject.ContainsKey($stem)) {
        $spec = $expectReject[$stem]
        if ($p.ExitCode -ne $spec.Exit) { throw "$stem exit $($p.ExitCode) expected $($spec.Exit)" }
        $etext = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $err)).Replace("`r`n", "`n").Trim()
        if (-not (Test-OrdinalEq $etext $spec.Diag)) { throw "$stem diag:`n got=$etext`n want=$($spec.Diag)" }
        $rej++; return
    }
    if ($p.ExitCode -ne 0) {
        $etext = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $err))
        throw "$stem unexpected reject $($p.ExitCode) $etext"
    }
    if (-not (Test-Path $gold)) { throw "missing golden $gold" }
    $g = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $gold)).Replace("`r`n","`n").TrimEnd()
    $got = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $out)).Replace("`r`n","`n").TrimEnd()
    if (-not (Test-OrdinalEq $got $g)) { throw "$stem tree mismatch`n--- gold ---`n$g`n--- root ---`n$got" }
    $ok++
}
Write-Output "p0_tree_contract root printTree ok n=$n match=$ok expected_reject=$rej"
} finally {
    Restore-HostedRegistryEnv
}
