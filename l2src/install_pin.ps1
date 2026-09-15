# Install the pinned L1 translator into a tree's build\l1trans\gen2 and gen3.
#
# gate.ps1 and tests/l1/run_gen.ps1 rebuild gen2 (and run_gen gen3) over the pinned
# binary, and gcc output here is not byte-reproducible, so a pinned runner after any
# gate.ps1 run needs the pin installed again. The hash is the tree's own
# l2src/L1_PIN.txt; the binary defaults to build\pin_<hash>\l1trans.exe under the main
# checkout (the first entry of git worktree list). Refuses with exit 2, before any
# copy, when the source's SHA256 is not the pin, and with exit 2 when a copy does not
# re-hash to the pin.
#
#   powershell -NoProfile -ExecutionPolicy Bypass -File l2src/install_pin.ps1 [-Tree <dir>] [-Source <exe>]
param([string]$Tree, [string]$Source)
$ErrorActionPreference = 'Continue'
if (-not $Tree) { $Tree = Join-Path $PSScriptRoot '..' }
if (-not (Test-Path -LiteralPath $Tree -PathType Container)) { Write-Output ('install_pin REFUSED: no tree ' + $Tree); exit 2 }
$Tree = (Resolve-Path -LiteralPath $Tree).Path
$pinFile = Join-Path $Tree 'l2src\L1_PIN.txt'
if (-not (Test-Path -LiteralPath $pinFile)) { Write-Output ('install_pin REFUSED: no ' + $pinFile); exit 2 }
$pin = (Get-Content -LiteralPath $pinFile -TotalCount 1).Trim().ToUpperInvariant()
if ($pin -notmatch '^[0-9A-F]{64}$') { Write-Output ('install_pin REFUSED: ' + $pinFile + ' is not 64 hex: ' + $pin); exit 2 }
if (-not $Source) {
    $main = ((git -C $Tree worktree list --porcelain) | Where-Object { $_ -like 'worktree *' } | Select-Object -First 1) -replace '^worktree ', ''
    if (-not $main) { Write-Output ('install_pin REFUSED: no main checkout from git worktree list in ' + $Tree); exit 2 }
    $Source = Join-Path $main ('build\pin_' + $pin + '\l1trans.exe')
}
if (-not (Test-Path -LiteralPath $Source)) { Write-Output ('install_pin REFUSED: source missing: ' + $Source); exit 2 }
$srcHash = (Get-FileHash -LiteralPath $Source -Algorithm SHA256).Hash
if ($srcHash -ne $pin) { Write-Output ('install_pin REFUSED: source ' + $Source + ' sha256 ' + $srcHash + ' is not L1_PIN ' + $pin + '; nothing copied'); exit 2 }
foreach ($gen in 'gen2', 'gen3') {
    $dir = Join-Path $Tree ('build\l1trans\' + $gen)
    New-Item -ItemType Directory -Force -Path $dir | Out-Null
    $dst = Join-Path $dir 'l1trans.exe'
    $before = if (Test-Path -LiteralPath $dst) { (Get-FileHash -LiteralPath $dst -Algorithm SHA256).Hash.Substring(0, 8) } else { 'absent' }
    Copy-Item -LiteralPath $Source -Destination $dst -Force
    $after = if (Test-Path -LiteralPath $dst) { (Get-FileHash -LiteralPath $dst -Algorithm SHA256).Hash } else { 'absent' }
    if ($after -ne $pin) { Write-Output ('install_pin REFUSED: ' + $dst + ' re-hashes to ' + $after + ', not L1_PIN ' + $pin); exit 2 }
    Write-Output ('install_pin ' + $gen + ': ' + $before + ' -> ' + $pin.Substring(0, 8))
}
Write-Output ('install_pin ok: ' + $Tree + ' gen2 and gen3 = L1_PIN ' + $pin.Substring(0, 8) + ' from ' + $Source)
exit 0
