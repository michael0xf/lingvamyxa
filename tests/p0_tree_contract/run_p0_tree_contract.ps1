# Compare root gen2 printTree to pinned 620db86 goldens in this directory.
# Does not regenerate goldens. C_nested_short is an expected reject.
$ErrorActionPreference = "Stop"
Set-Location (Join-Path $PSScriptRoot "..\..")
$here = "tests\p0_tree_contract"
$pt = "build\l1trans\gen2\printTree.exe"
if (-not (Test-Path $pt)) { throw "missing $pt" }
$n = 0; $ok = 0; $rej = 0
Get-ChildItem $here -Filter "*.lmx" | Sort-Object Name | ForEach-Object {
    $stem = $_.BaseName
    $src = $_.FullName
    $gold = Join-Path $here ($stem + ".tree.txt")
    $out = Join-Path "build\p0_tree_contract" ($stem + ".root.out")
    $err = Join-Path "build\p0_tree_contract" ($stem + ".root.err")
    New-Item -ItemType Directory -Force -Path "build\p0_tree_contract" | Out-Null
    $p = Start-Process -FilePath (Join-Path (Get-Location) $pt) -ArgumentList $src -NoNewWindow -Wait -PassThru -RedirectStandardOutput (Join-Path (Get-Location) $out) -RedirectStandardError (Join-Path (Get-Location) $err)
    $n++
    if ($stem -eq "C_nested_short") {
        if ($p.ExitCode -eq 0) { throw "$stem expected reject" }
        $etext = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $err))
        if ($etext -notmatch "P0 parse error 13 at 2:5") { throw "$stem diag: $etext" }
        $rej++; return
    }
    if ($p.ExitCode -ne 0) { throw "$stem unexpected reject $($p.ExitCode)" }
    if (-not (Test-Path $gold)) { throw "missing golden $gold" }
    $g = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $gold)).Replace("`r`n","`n").TrimEnd()
    $got = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $out)).Replace("`r`n","`n").TrimEnd()
    if ($got -ne $g) { throw "$stem tree mismatch`n--- gold ---`n$g`n--- root ---`n$got" }
    $ok++
}
Write-Output "p0_tree_contract root printTree ok n=$n match=$ok expected_reject=$rej"
