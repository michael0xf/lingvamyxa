# Narrow L2 -> L1 -> C -> exe smoke. Build root is this file's parent.
# Artifacts stay under build\l2trans. Does not run native finalize, gate, or run_lmx.
$ErrorActionPreference = "Stop"
Set-Location (Join-Path $PSScriptRoot "..")

$gen = "gen2"
if ($env:L1_GEN -and $env:L1_GEN.Trim().Length -gt 0) { $gen = $env:L1_GEN.Trim() }
$l1trans = "build\l1trans\$gen\l1trans.exe"
if (-not (Test-Path -LiteralPath $l1trans)) {
    throw "missing L1 translator: $l1trans (run tests\l1\run_gen.ps1 first)"
}

$out = "build\l2trans"
$log = "build\l1trans\logs\$gen"
New-Item -ItemType Directory -Force -Path $out, $log | Out-Null

$guards = @(
    "-Werror=incompatible-pointer-types", "-Werror=discarded-qualifiers",
    "-Werror=implicit-function-declaration", "-Werror=implicit-int"
)
$cflags = @("-std=c99", "-Wall", "-Wextra", "-Wpedantic", "-I", ".") + $guards

function Invoke-Gcc([string]$cpath, [string]$exe, [string]$glog) {
    $flagStr = ($cflags -join " ")
    cmd /c "gcc $flagStr `"$cpath`" -o `"$exe`" > `"$glog`" 2>&1"
    if ($LASTEXITCODE -ne 0) {
        Get-Content $glog
        throw "gcc failed: $cpath"
    }
}

$l2c = Join-Path $out "l2trans.c"
$l2exe = Join-Path $out "l2trans.exe"
& $l1trans "l2src\l2trans.lm1" $l2c
if ($LASTEXITCODE -ne 0) { throw "l1trans failed: l2src\l2trans.lm1" }
Invoke-Gcc $l2c $l2exe (Join-Path $log "l2trans.gcc.log")

function Clear-Case([string]$stem) {
    foreach ($ext in @(".lm1", ".c", ".exe", ".err", ".stdout")) {
        $p = Join-Path $out ($stem + $ext)
        if (Test-Path -LiteralPath $p) { Remove-Item -LiteralPath $p -Force }
    }
}

function Invoke-Positive([string]$src, [string]$stem, [int]$expect, [string]$lit) {
    Clear-Case $stem
    $lm1 = Join-Path $out ($stem + ".lm1")
    $cpath = Join-Path $out ($stem + ".c")
    $exe = Join-Path $out ($stem + ".exe")
    $err = Join-Path $out ($stem + ".err")
    cmd /c "`"$l2exe`" `"$src`" `"$lm1`" 2> `"$err`""
    if ($LASTEXITCODE -ne 0) {
        Get-Content $err
        throw "l2trans failed: $src"
    }
    $text = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $lm1))
    $needle = "return: $lit"
    if ($text.IndexOf($needle) -lt 0) {
        throw "generated L1 missing '$needle' in $lm1"
    }
    & $l1trans $lm1 $cpath
    if ($LASTEXITCODE -ne 0) { throw "l1trans failed: $lm1" }
    $ctext = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $cpath))
    if ($ctext.IndexOf("return $lit;") -lt 0 -and $ctext.IndexOf("return $lit ;") -lt 0) {
        if ($ctext -notmatch ("return\s+" + [regex]::Escape($lit) + "\s*;")) {
            throw "generated C missing return $lit in $cpath"
        }
    }
    Invoke-Gcc $cpath $exe (Join-Path $log "$stem.gcc.log")
    & $exe
    if ($LASTEXITCODE -ne $expect) {
        throw "$stem exe exit $($LASTEXITCODE) expected $expect"
    }
}

function Invoke-Negative([string]$src, [string]$stem, [string]$needle) {
    # Planted .lm1/.c/.exe are text markers, not a real executable.
    # This checks nonzero l2trans, no later stages, destination unchanged.
    # It does not run or clear a previous real exe.
    $lm1 = Join-Path $out ($stem + ".lm1")
    $cpath = Join-Path $out ($stem + ".c")
    $exe = Join-Path $out ($stem + ".exe")
    $err = Join-Path $out ($stem + ".err")
    Clear-Case $stem
    $marker = "OLD-OUTPUT-MUST-NOT-BECOME-SUCCESS`n"
    [System.IO.File]::WriteAllText((Join-Path (Get-Location) $lm1), $marker)
    [System.IO.File]::WriteAllText((Join-Path (Get-Location) $cpath), $marker)
    [System.IO.File]::WriteAllText((Join-Path (Get-Location) $exe), $marker)
    cmd /c "`"$l2exe`" `"$src`" `"$lm1`" 2> `"$err`""
    if ($LASTEXITCODE -eq 0) {
        throw "expected l2trans failure: $src"
    }
    $etext = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $err))
    if ($etext.IndexOf($needle) -lt 0) {
        throw "missing diagnostic '$needle' in $err : $etext"
    }
    $got = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $lm1))
    if ($got -ne $marker) {
        throw "negative $stem mutated destination lm1"
    }
    if (-not (Test-Path -LiteralPath $cpath)) { throw "negative $stem lost planted c" }
    $cgot = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $cpath))
    if ($cgot -ne $marker) { throw "negative $stem mutated planted c" }
    $egot = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $exe))
    if ($egot -ne $marker) { throw "negative $stem mutated planted exe marker" }
}

function Invoke-Puts([string]$src, [string]$stem, [int]$expect, [string]$want) {
    Clear-Case $stem
    $lm1 = Join-Path $out ($stem + ".lm1")
    $cpath = Join-Path $out ($stem + ".c")
    $exe = Join-Path $out ($stem + ".exe")
    $err = Join-Path $out ($stem + ".err")
    $captured = Join-Path $out ($stem + ".stdout")
    cmd /c "`"$l2exe`" `"$src`" `"$lm1`" 2> `"$err`""
    if ($LASTEXITCODE -ne 0) {
        Get-Content $err
        throw "l2trans failed: $src"
    }
    $text = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $lm1))
    if ($text.IndexOf("c.puts:") -lt 0) {
        throw "generated L1 missing c.puts in $lm1"
    }
    if ($text.IndexOf("include:") -lt 0) {
        throw "generated L1 missing include in $lm1"
    }
    & $l1trans $lm1 $cpath
    if ($LASTEXITCODE -ne 0) { throw "l1trans failed: $lm1" }
    Invoke-Gcc $cpath $exe (Join-Path $log "$stem.gcc.log")
    cmd /c "`"$exe`" > `"$captured`" 2> `"$err`""
    if ($LASTEXITCODE -ne $expect) {
        throw "$stem exe exit $($LASTEXITCODE) expected $expect"
    }
    $got = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $captured))
    $got = $got -replace "`r`n", "`n"
    if ($got -ne $want) {
        throw "$stem stdout mismatch got=[$got] want=[$want]"
    }
}

function Invoke-AdmitEmit([string]$src, [string]$stem, [string]$lit) {
    # Admission + emission only. Do not use OS exit status for large ints.
    Clear-Case $stem
    $lm1 = Join-Path $out ($stem + ".lm1")
    $cpath = Join-Path $out ($stem + ".c")
    $exe = Join-Path $out ($stem + ".exe")
    $err = Join-Path $out ($stem + ".err")
    cmd /c "`"$l2exe`" `"$src`" `"$lm1`" 2> `"$err`""
    if ($LASTEXITCODE -ne 0) {
        Get-Content $err
        throw "l2trans failed: $src"
    }
    $text = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $lm1))
    $needle = "return: $lit"
    if ($text.IndexOf($needle) -lt 0) {
        throw "generated L1 missing '$needle' in $lm1"
    }
    & $l1trans $lm1 $cpath
    if ($LASTEXITCODE -ne 0) { throw "l1trans failed: $lm1" }
    $ctext = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $cpath))
    if ($ctext -notmatch ("return\s+" + [regex]::Escape($lit) + "\s*;")) {
        throw "generated C missing return $lit in $cpath"
    }
    Invoke-Gcc $cpath $exe (Join-Path $log "$stem.gcc.log")
}

Invoke-Positive "l2src\tests\entry_return0.lm2" "entry_return0" 0 "0"
Invoke-Positive "l2src\tests\entry_return7.lm2" "entry_return7" 7 "7"

$lm0 = [System.IO.File]::ReadAllText((Join-Path (Get-Location) (Join-Path $out "entry_return0.lm1")))
$lm7 = [System.IO.File]::ReadAllText((Join-Path (Get-Location) (Join-Path $out "entry_return7.lm1")))
if ($lm0 -eq $lm7) { throw "return 0 and return 7 produced identical L1" }
$c0 = [System.IO.File]::ReadAllText((Join-Path (Get-Location) (Join-Path $out "entry_return0.c")))
$c7 = [System.IO.File]::ReadAllText((Join-Path (Get-Location) (Join-Path $out "entry_return7.c")))
if ($c0 -eq $c7) { throw "return 0 and return 7 produced identical C" }

Invoke-Negative "l2src\tests\entry_bad_body.lm2" "entry_bad_body" "unsupported body"
Invoke-Negative "l2src\tests\entry_bad_sig.lm2" "entry_bad_sig" "incompatible entry signature"
Invoke-Negative "l2src\tests\entry_two_main.lm2" "entry_two_main" "several main"
Invoke-Negative "l2src\tests\entry_overflow.lm2" "entry_overflow" "return literal not representable as int"
Invoke-AdmitEmit "l2src\tests\entry_int_max.lm2" "entry_int_max" "2147483647"

Invoke-Puts "l2src\tests\entry_puts_hello.lm2" "entry_puts_hello" 0 "Hello`n"
Invoke-Puts "l2src\tests\entry_puts_seq.lm2" "entry_puts_seq" 0 "one`ntwo`n"
Invoke-Puts "l2src\tests\entry_puts_empty.lm2" "entry_puts_empty" 0 "`n"
Invoke-Puts "l2src\tests\entry_puts_esc.lm2" "entry_puts_esc" 0 "a`"b\c`n"
Invoke-Puts "l2src\tests\entry_puts_nl.lm2" "entry_puts_nl" 0 "x`ny`n"
Invoke-Negative "l2src\tests\entry_puts_bad_arg.lm2" "entry_puts_bad_arg" "unsupported argument"
Invoke-Negative "l2src\tests\entry_puts_extra_arg.lm2" "entry_puts_extra_arg" "extra argument"
Invoke-Negative "l2src\tests\entry_puts_after_return.lm2" "entry_puts_after_return" "unsupported body"
Invoke-Negative "l2src\tests\entry_puts_nested.lm2" "entry_puts_nested" "unsupported argument"

function Invoke-Leaf([string]$src, [string]$stem, [int]$expect, [string]$name) {
    Clear-Case $stem
    $lm1 = Join-Path $out ($stem + ".lm1")
    $cpath = Join-Path $out ($stem + ".c")
    $exe = Join-Path $out ($stem + ".exe")
    $err = Join-Path $out ($stem + ".err")
    cmd /c "`"$l2exe`" `"$src`" `"$lm1`" 2> `"$err`""
    if ($LASTEXITCODE -ne 0) {
        Get-Content $err
        throw "l2trans failed: $src"
    }
    $text = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $lm1))
    if ($text.IndexOf("fn: $name") -lt 0) { throw "$stem L1 missing fn: $name" }
    if ($text.IndexOf("@: Lmx") -lt 0) { throw "$stem L1 missing Lmx node" }
    if ($text.IndexOf("LmxMethod") -lt 0) { throw "$stem L1 missing method record" }
    if ($text.IndexOf("lmx_classify") -lt 0) { throw "$stem L1 missing classify" }
    if ($text -notmatch ([regex]::Escape($name) + "\((leaf|unit|node)")) {
        throw "$stem L1 missing typed call"
    }
    & $l1trans $lm1 $cpath
    if ($LASTEXITCODE -ne 0) { throw "l1trans failed: $lm1" }
    $ctext = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $cpath))
    if ($ctext.IndexOf("Lmx *node") -lt 0 -and $ctext.IndexOf("Lmx* node") -lt 0) {
        if ($ctext -notmatch "Lmx\s*\*\s*node") { throw "$stem C missing node parameter" }
    }
    Invoke-Gcc $cpath $exe (Join-Path $log "$stem.gcc.log")
    & $exe
    if ($LASTEXITCODE -ne $expect) {
        throw "$stem exe exit $($LASTEXITCODE) expected $expect"
    }
}

Invoke-Leaf "l2src\tests\add.lm2" "add" 0 "add"
Invoke-Leaf "l2src\tests\entry_sum.lm2" "entry_sum" 0 "sum"
Invoke-Leaf "l2src\tests\entry_add_ret.lm2" "entry_add_ret" 5 "add"
Invoke-Leaf "l2src\tests\entry_plus.lm2" "entry_plus" 0 "plus"
Invoke-Leaf "l2src\tests\entry_swap_formals.lm2" "entry_swap_formals" 0 "add"

# Intern algorithm is l2_intern_prove in l2trans.lm1 (translation fails if intern lies).
# This harness only checks that the intern table was emitted; it is not the equality proof.
function Get-LeafContract([string]$stem) {
    $t = [System.IO.File]::ReadAllText((Join-Path (Get-Location) (Join-Path $out ($stem + ".lm1"))))
    if ($t.IndexOf("Runtime: no memcmp") -lt 0) { throw "$stem missing honest runtime no-memcmp" }
    if ($t.IndexOf("id 1 :=") -lt 0) { throw "$stem missing intern mapping id 1 :=" }
    if ($t.IndexOf("Closed-unit devirtualization") -lt 0 -and $t.IndexOf("Closed-singleton devirtualization") -lt 0) {
        throw "$stem missing devirtualization proof comment"
    }
    $f0 = $null; $f1 = $null
    if ($t -match 'l2_sig_f0\) "([^"]*)"') { $f0 = $Matches[1] }
    if ($t -match 'l2_sig_f1\) "([^"]*)"') { $f1 = $Matches[1] }
    if (-not $f0 -or -not $f1) { throw "$stem missing interned formal-name mapping" }
    if ($t -notmatch 'l2_intern_id (\d+)U') { throw "$stem missing l2_intern_id" }
    $id = [int]$Matches[1]
    if ($t -notmatch 'l2_intern_again (\d+)U') { throw "$stem missing l2_intern_again" }
    $again = [int]$Matches[1]
    if ($again -ne $id) { throw "$stem intern again $again != id $id" }
    if ($t -notmatch 'l2_intern_swap (\d+)U') { throw "$stem missing l2_intern_swap" }
    $sw = [int]$Matches[1]
    if ($t -notmatch 'l2_intern_probe (\d+)U') { throw "$stem missing l2_intern_probe" }
    $probe = [int]$Matches[1]
    if ($t -notmatch 'rec\\sig: (\d+)U') { throw "$stem missing rec.sig intern id" }
    $rec = [int]$Matches[1]
    if ($rec -ne $id) { throw "$stem rec.sig $rec != intern id $id" }
    [pscustomobject]@{ Id = $id; Again = $again; Swap = $sw; Probe = $probe; Formals = "$f0|$f1" }
}
$cAdd = Get-LeafContract "add"
$cPlus = Get-LeafContract "entry_plus"
$cSum = Get-LeafContract "entry_sum"
$cSwapF = Get-LeafContract "entry_swap_formals"
if ($cAdd.Formals -ne "a|b") { throw "add formals $($cAdd.Formals)" }
if ($cAdd.Probe -ne $cAdd.Id) { throw "add intern probe must memcmp-equal a|b" }
if ($cAdd.Swap -eq $cAdd.Id) { throw "add intern must distinguish swapped formals" }
if ($cPlus.Formals -ne $cAdd.Formals) { throw "renaming method changed contract" }
if ($cPlus.Id -ne $cAdd.Id) { throw "plus intern id differed from add" }
if ($cSum.Formals -eq $cAdd.Formals) { throw "sum x,y must differ from add a,b" }
if ($cSum.Probe -eq $cSum.Id) { throw "sum intern must not equal probe a|b" }
if ($cSwapF.Formals -ne "b|a") { throw "swap formals $($cSwapF.Formals)" }
if ($cSwapF.Formals -eq $cAdd.Formals) { throw "swapped formals must differ from a,b" }
if ($cSwapF.Probe -eq $cSwapF.Id) { throw "swap intern must not equal probe a|b" }

Invoke-Leaf "l2src\tests\unit_chain.lm2" "unit_chain" 0 "wrap"
Invoke-Leaf "l2src\tests\unit_forward.lm2" "unit_forward" 0 "wrap"
Invoke-Leaf "l2src\tests\unit_if.lm2" "unit_if" 7 "max"
Invoke-Leaf "l2src\tests\unit_contracts.lm2" "unit_contracts" 0 "add"
$uc = [System.IO.File]::ReadAllText((Join-Path (Get-Location) (Join-Path $out "unit_contracts.lm1")))
$sigs = [regex]::Matches($uc, 'rec\\sig: (\d+)U') | ForEach-Object { [int]$_.Groups[1].Value }
if ($sigs.Count -lt 4) { throw "unit_contracts expected 4 rec.sig intern ids, got $($sigs.Count)" }
if ($sigs[0] -ne $sigs[1]) { throw "add and plus same formals must share intern id" }
if ($sigs[2] -eq $sigs[0]) { throw "sum x,y must intern differently from add a,b" }
if ($sigs[3] -eq $sigs[0]) { throw "swap b,a must intern differently from add a,b" }
if ($uc.IndexOf("plus(unit") -lt 0 -and $uc -notmatch 'plus\(unit') { throw "unit_contracts missing plus call" }

Invoke-Negative "l2src\tests\unit_dup_def.lm2" "unit_dup_def" "duplicate definition"
Invoke-Negative "l2src\tests\unit_dup_formal.lm2" "unit_dup_formal" "duplicate formal"
Invoke-Negative "l2src\tests\unit_loop.lm2" "unit_loop" "unsupported loop"
Invoke-Negative "l2src\tests\unit_rec.lm2" "unit_rec" "unsupported recursion"
Invoke-Negative "l2src\tests\unit_cycle.lm2" "unit_cycle" "unsupported recursion"
Invoke-Negative "l2src\tests\entry_unknown_method.lm2" "entry_unknown_method" "unknown method"
Invoke-Negative "l2src\tests\entry_bad_arity.lm2" "entry_bad_arity" "incompatible entry signature"
Invoke-Negative "l2src\tests\entry_unresolved.lm2" "entry_unresolved" "unresolved name"
Invoke-Negative "l2src\tests\entry_trailer.lm2" "entry_trailer" "unsupported trailer"

"l2trans $gen ok"
