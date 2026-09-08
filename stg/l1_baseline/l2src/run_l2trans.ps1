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
    if ($text -notmatch 'fn: l2_m\d+') { throw "$stem L1 missing mangled method symbol" }
    if ($text.IndexOf("@: Lmx") -lt 0) { throw "$stem L1 missing Lmx node" }
    if ($text.IndexOf("LmxMethod") -lt 0) { throw "$stem L1 missing method record" }
    if ($text.IndexOf("lmx_classify") -lt 0) { throw "$stem L1 missing classify" }
    if ($text -notmatch 'fn: l2_m\d+ \(@: Lmx node' -and $text -notmatch 'l2_m\d+\((leaf|unit|node)') {
        throw "$stem L1 missing typed entry"
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
if ($uc -notmatch 'l2_m\d+\(unit') { throw "unit_contracts missing mangled typed call" }

Invoke-Negative "l2src\tests\unit_dup_def.lm2" "unit_dup_def" "duplicate definition"
Invoke-Negative "l2src\tests\unit_dup_formal.lm2" "unit_dup_formal" "duplicate formal"
Invoke-Negative "l2src\tests\unit_loop.lm2" "unit_loop" "unsupported loop"
Invoke-Negative "l2src\tests\unit_rec.lm2" "unit_rec" "unsupported recursion"
Invoke-Negative "l2src\tests\unit_cycle.lm2" "unit_cycle" "unsupported recursion"
Invoke-Leaf "l2src\tests\unit_eight.lm2" "unit_eight" 0 "m7"
$e8 = [System.IO.File]::ReadAllText((Join-Path (Get-Location) (Join-Path $out "unit_eight.lm1")))
if ($e8.IndexOf("lmx_ranges_init(9U)") -lt 0) { throw "unit_eight must init 9 ranges (8 methods + children)" }
if ($e8.IndexOf("l2_p0_0") -lt 0) { throw "unit_eight missing hygienic formal l2_p0_0" }
Invoke-Negative "l2src\tests\unit_nine.lm2" "unit_nine" "too many methods"
Invoke-Leaf "l2src\tests\unit_tempname.lm2" "unit_tempname" 0 "add"
$tn = [System.IO.File]::ReadAllText((Join-Path (Get-Location) (Join-Path $out "unit_tempname.lm1")))
if ($tn -match 'int: l2_t0;') { throw "unit_tempname leaked source formal l2_t0 into L1 params" }
if ($tn.IndexOf("l2_p0_0") -lt 0) { throw "unit_tempname missing mangled formal" }
if ($tn.IndexOf('l2_sig_f0) "l2_t0"') -lt 0) { throw "intern must keep source formal name l2_t0" }
Invoke-Negative "l2src\tests\unit_longname.lm2" "unit_longname" "name too long"
Invoke-Negative "l2src\tests\unit_deepif.lm2" "unit_deepif" "too deeply nested"
Invoke-Negative "l2src\tests\unit_node_formal.lm2" "unit_node_formal" "incompatible entry signature"
Invoke-Negative "l2src\tests\entry_unknown_method.lm2" "entry_unknown_method" "unknown method"
Invoke-Negative "l2src\tests\entry_bad_arity.lm2" "entry_bad_arity" "incompatible entry signature"
Invoke-Negative "l2src\tests\entry_unresolved.lm2" "entry_unresolved" "unresolved name"
Invoke-Negative "l2src\tests\entry_trailer.lm2" "entry_trailer" "unsupported trailer"

Invoke-Leaf "l2src\tests\unit_prec.lm2" "unit_prec" 1 "prec"
$pr = [System.IO.File]::ReadAllText((Join-Path (Get-Location) (Join-Path $out "unit_prec.lm1")))
if ($pr.IndexOf("||") -lt 0 -or $pr.IndexOf("&&") -lt 0) { throw "unit_prec missing &&/|| emission" }
Invoke-Leaf "l2src\tests\unit_sc.lm2" "unit_sc" 1 "div0"
$sc = [System.IO.File]::ReadAllText((Join-Path (Get-Location) (Join-Path $out "unit_sc.lm1")))
if ($sc -notmatch 'if: l2_t\d+ = 0' -and $sc -notmatch '1 \|\| l2_m\d+\(') { throw "unit_sc must skip RHS call via guarded if or C ||" }

function Invoke-PredAscii {
    $refLm1 = Join-Path $out "pred_ref.lm1"
    $refC = Join-Path $out "pred_ref.c"
    $refExe = Join-Path $out "pred_ref.exe"
    $refOut = Join-Path $out "pred_ref.stdout"
    $refSrc = @"
predef: "l1src/parser_text.lm1"
include: "<stdio.h>"
external:
    fn: main () int
        int: v 0
        while: v < 128
            c.printf("%d%d%d%d%d%d%d\n", lm_p0_is_horizontal_space((cast: (char) v)), lm_p0_is_line_break((cast: (char) v)), lm_p0_is_field_space((cast: (char) v)), lm_p0_is_field_separator((cast: (char) v)), lm_p0_is_short_form_separator((cast: (char) v)), lm_p0_is_quoted_token_boundary((cast: (char) v)), lm_p0_is_decimal_digit((cast: (char) v)))
            v: v + 1
        return: 0
    end: main
end: external
"@
    [System.IO.File]::WriteAllText((Join-Path (Get-Location) $refLm1), $refSrc.Replace("`r`n","`n"))
    & $l1trans $refLm1 $refC
    if ($LASTEXITCODE -ne 0) { throw "l1trans failed pred_ref (parser_text excerpt)" }
    Invoke-Gcc $refC $refExe (Join-Path $log "pred_ref.gcc.log")
    cmd /c "`"$refExe`" > `"$refOut`" 2> `"$(Join-Path $out 'pred_ref.err')`""
    if ($LASTEXITCODE -ne 0) { throw "pred_ref exe failed" }

    Invoke-Leaf "l2src\parser_text_predicates.lm2" "parser_text_predicates" 0 "lm_p0_is_horizontal_space"
    $lm1 = Join-Path $out "parser_text_predicates.lm1"
    $text = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $lm1)).Replace("`r`n", "`n")
    if ($text -match 'fn: lm_p0_is_') { throw "predicates must mangle method symbols, not emit source names as C symbols" }
    if ($text -notmatch 'rec\\addr: \(cast: \(LmxEntry\) l2_m0\)') { throw "predicates missing rec.addr l2_m0" }
    if ($text -notmatch 'rec\\addr: \(cast: \(LmxEntry\) l2_m6\)') { throw "predicates missing full 7-method graph" }
    $tail = "        return: 0`n    end: main`nend: external"
    $pos = $text.LastIndexOf($tail)
    if ($pos -lt 0) { throw "predicates L1 missing generated main return" }
    $head = $text.Substring(0, $pos)
    $drive = @"
        int: v 0
        while: v < 128
            c.printf("%d%d%d%d%d%d%d\n", l2_m0(unit, (cast: (char) v)), l2_m1(unit, (cast: (char) v)), l2_m2(unit, (cast: (char) v)), l2_m3(unit, (cast: (char) v)), l2_m4(unit, (cast: (char) v)), l2_m5(unit, (cast: (char) v)), l2_m6(unit, (cast: (char) v)))
            v: v + 1
        return: 0
    end: main
end: external
"@
    $drvLm1 = Join-Path $out "pred_l2_drive.lm1"
    $drvC = Join-Path $out "pred_l2_drive.c"
    $drvExe = Join-Path $out "pred_l2_drive.exe"
    $drvOut = Join-Path $out "pred_l2_drive.stdout"
    [System.IO.File]::WriteAllText((Join-Path (Get-Location) $drvLm1), ($head + $drive.Replace("`r`n","`n")))
    & $l1trans $drvLm1 $drvC
    if ($LASTEXITCODE -ne 0) { throw "l1trans failed pred_l2_drive" }
    Invoke-Gcc $drvC $drvExe (Join-Path $log "pred_l2_drive.gcc.log")
    cmd /c "`"$drvExe`" > `"$drvOut`" 2> `"$(Join-Path $out 'pred_l2_drive.err')`""
    if ($LASTEXITCODE -ne 0) { throw "pred_l2_drive exe failed" }
    $a = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $refOut)).Replace("`r`n","`n")
    $b = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $drvOut)).Replace("`r`n","`n")
    if ($a -ne $b) { throw "predicate 128-ASCII mismatch vs parser_text.lm1 reference" }
    $lines = $a.Split("`n") | Where-Object { $_ -ne "" }
    if ($lines.Count -ne 128) { throw "expected 128 ASCII lines, got $($lines.Count)" }
}

Invoke-PredAscii

Invoke-Leaf "l2src\tests\unit_malloc_name.lm2" "unit_malloc_name" 10 "malloc"
$mn = [System.IO.File]::ReadAllText((Join-Path (Get-Location) (Join-Path $out "unit_malloc_name.lm1")))
if ($mn -match 'fn: malloc \(') { throw "source name malloc must not be the C symbol" }
if ($mn -notmatch 'fn: l2_m0 ') { throw "malloc should be l2_m0" }

function Invoke-PublishFail {
    $dest = Join-Path $out "publish_fail.lm1"
    $bak = $dest + ".bak"
    $marker = "KEEP-THESE-BYTES-ON-FAILED-PUBLISH`n"
    [System.IO.File]::WriteAllText((Join-Path (Get-Location) $dest), $marker)
    if (Test-Path -LiteralPath $bak) { Remove-Item -LiteralPath $bak -Recurse -Force }
    New-Item -ItemType Directory -Path (Join-Path (Get-Location) $bak) | Out-Null
    cmd /c "`"$l2exe`" `"l2src\tests\add.lm2`" `"$dest`" 2> `"$(Join-Path $out 'publish_fail.err')`""
    if ($LASTEXITCODE -eq 0) { throw "publish_fail expected replace failure when .bak is a directory" }
    $got = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $dest))
    if ($got -ne $marker) { throw "failed publication mutated dest: $got" }
    Remove-Item -LiteralPath $bak -Recurse -Force

    $dest2 = Join-Path $out "publish_bak_file.lm1"
    $bak2 = $dest2 + ".bak"
    $sentinel = "FOREIGN-BAK-SENTINEL`n"
    [System.IO.File]::WriteAllText((Join-Path (Get-Location) $dest2), $marker)
    [System.IO.File]::WriteAllText((Join-Path (Get-Location) $bak2), $sentinel)
    cmd /c "`"$l2exe`" `"l2src\tests\add.lm2`" `"$dest2`" 2> `"$(Join-Path $out 'publish_bak_file.err')`""
    if ($LASTEXITCODE -ne 0) {
        Get-Content (Join-Path $out 'publish_bak_file.err')
        throw "existing regular dest.bak must not block unique-backup publication"
    }
    $kept = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $bak2))
    if ($kept -ne $sentinel) { throw "foreign dest.bak sentinel was mutated" }
    $newd = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $dest2))
    if ($newd -eq $marker) { throw "unique-backup publication left dest unchanged" }
    if ($newd.IndexOf("fn: l2_m") -lt 0) { throw "unique-backup dest is not generated L1" }
}

Invoke-PublishFail

function Invoke-LineBreakWidth {
    $refLm1 = Join-Path $out "lbw_ref.lm1"
    $refC = Join-Path $out "lbw_ref.c"
    $refExe = Join-Path $out "lbw_ref.exe"
    $refOut = Join-Path $out "lbw_ref.stdout"
    $refSrc = @"
predef: "l1src/parser_text.lm1"
include: "<stdio.h>"
external:
    fn: main () int
        c.printf("%zu\n", lm_p0_line_break_width_at("\n", 1U, 0U))
        c.printf("%zu\n", lm_p0_line_break_width_at("\r", 1U, 0U))
        c.printf("%zu\n", lm_p0_line_break_width_at("\r\n", 2U, 0U))
        c.printf("%zu\n", lm_p0_line_break_width_at("x", 1U, 0U))
        c.printf("%zu\n", lm_p0_line_break_width_at("\r\n", 2U, 2U))
        c.printf("%zu\n", lm_p0_line_break_width_at("", 0U, 0U))
        c.printf("%zu\n", lm_p0_line_break_width_at(0, 0U, 0U))
        c.printf("%zu\n", lm_p0_line_break_width_at("ab", 2U, 2U))
        c.printf("%zu\n", lm_p0_line_break_width_at("a\r\nb", 4U, 1U))
        return: 0
    end: main
end: external
"@
    [System.IO.File]::WriteAllText((Join-Path (Get-Location) $refLm1), $refSrc.Replace("`r`n","`n"))
    & $l1trans $refLm1 $refC
    if ($LASTEXITCODE -ne 0) { throw "l1trans failed lbw_ref" }
    Invoke-Gcc $refC $refExe (Join-Path $log "lbw_ref.gcc.log")
    cmd /c "`"$refExe`" > `"$refOut`" 2> `"$(Join-Path $out 'lbw_ref.err')`""
    if ($LASTEXITCODE -ne 0) { throw "lbw_ref exe failed" }

    Invoke-Leaf "l2src\parser_text_line_break.lm2" "parser_text_line_break" 0 "lm_p0_line_break_width_at"
    $lm1 = Join-Path $out "parser_text_line_break.lm1"
    $text = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $lm1)).Replace("`r`n", "`n")
    if ($text -match 'fn: lm_p0_line_break_width_at') { throw "line_break must mangle method symbols" }
    if ($text.IndexOf("const: @(char l2_p0_0)") -lt 0) { throw "line_break missing const char* formal" }
    if ($text.IndexOf("size_t: l2_p0_1") -lt 0) { throw "line_break missing size_t formal" }
    if ($text.IndexOf(") size_t") -lt 0) { throw "line_break missing size_t result" }
    $tail = "        return: 0`n    end: main`nend: external"
    $pos = $text.LastIndexOf($tail)
    if ($pos -lt 0) { throw "line_break L1 missing generated main return" }
    $head = $text.Substring(0, $pos)
    $drive = @"
        c.printf("%zu\n", l2_m0(unit, "\n", 1U, 0U))
        c.printf("%zu\n", l2_m0(unit, "\r", 1U, 0U))
        c.printf("%zu\n", l2_m0(unit, "\r\n", 2U, 0U))
        c.printf("%zu\n", l2_m0(unit, "x", 1U, 0U))
        c.printf("%zu\n", l2_m0(unit, "\r\n", 2U, 2U))
        c.printf("%zu\n", l2_m0(unit, "", 0U, 0U))
        c.printf("%zu\n", l2_m0(unit, 0, 0U, 0U))
        c.printf("%zu\n", l2_m0(unit, "ab", 2U, 2U))
        c.printf("%zu\n", l2_m0(unit, "a\r\nb", 4U, 1U))
        return: 0
    end: main
end: external
"@
    $drvLm1 = Join-Path $out "lbw_l2_drive.lm1"
    $drvC = Join-Path $out "lbw_l2_drive.c"
    $drvExe = Join-Path $out "lbw_l2_drive.exe"
    $drvOut = Join-Path $out "lbw_l2_drive.stdout"
    [System.IO.File]::WriteAllText((Join-Path (Get-Location) $drvLm1), ($head + $drive.Replace("`r`n","`n")))
    & $l1trans $drvLm1 $drvC
    if ($LASTEXITCODE -ne 0) { throw "l1trans failed lbw_l2_drive" }
    Invoke-Gcc $drvC $drvExe (Join-Path $log "lbw_l2_drive.gcc.log")
    cmd /c "`"$drvExe`" > `"$drvOut`" 2> `"$(Join-Path $out 'lbw_l2_drive.err')`""
    if ($LASTEXITCODE -ne 0) { throw "lbw_l2_drive exe failed" }
    $a = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $refOut)).Replace("`r`n","`n")
    $b = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $drvOut)).Replace("`r`n","`n")
    if ($a -ne $b) { throw "line_break_width_at mismatch vs parser_text.lm1`nREF:`n$a`nL2:`n$b" }
}

Invoke-LineBreakWidth

function Invoke-SpliceDrive([string]$stem, [string]$driveBody) {
    $lm1 = Join-Path $out ($stem + ".lm1")
    $text = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $lm1)).Replace("`r`n", "`n")
    $tail = "        return: 0`n    end: main`nend: external"
    $pos = $text.LastIndexOf($tail)
    if ($pos -lt 0) { throw "$stem L1 missing generated main return" }
    $drvLm1 = Join-Path $out ($stem + "_drive.lm1")
    $drvC = Join-Path $out ($stem + "_drive.c")
    $drvExe = Join-Path $out ($stem + "_drive.exe")
    $drvOut = Join-Path $out ($stem + "_drive.stdout")
    [System.IO.File]::WriteAllText((Join-Path (Get-Location) $drvLm1), ($text.Substring(0, $pos) + $driveBody.Replace("`r`n", "`n")))
    & $l1trans $drvLm1 $drvC
    if ($LASTEXITCODE -ne 0) { throw "l1trans failed $stem drive" }
    Invoke-Gcc $drvC $drvExe (Join-Path $log "$stem.drive.gcc.log")
    cmd /c "`"$drvExe`" > `"$drvOut`" 2> `"$(Join-Path $out ($stem + '_drive.err'))`""
    if ($LASTEXITCODE -ne 0) { throw "$stem drive exe failed" }
    return [System.IO.File]::ReadAllText((Join-Path (Get-Location) $drvOut)).Replace("`r`n", "`n")
}

Invoke-Leaf "l2src\tests\unit_own_early.lm2" "unit_own_early" 0 "m"
$oe = [System.IO.File]::ReadAllText((Join-Path (Get-Location) (Join-Path $out "unit_own_early.lm1")))
if ($oe.IndexOf("l2_q0_dirty") -lt 0) { throw "unit_own_early missing typed own cache" }
if ($oe.IndexOf("lmx_char_cell") -lt 0) { throw "unit_own_early missing all_chars publish" }
$d1 = Invoke-SpliceDrive "unit_own_early" @"
        l2_m0(unit, 0)
        @: Lmx f 0
        f: lmx_branch_child(unit, 0U)
        c.printf("%d\n", lmx_char_value(f\data))
        l2_m0(unit, 1)
        f: lmx_branch_child(unit, 0U)
        c.printf("%d\n", lmx_char_value(f\data))
        return: 0
    end: main
end: external
"@
if ($d1 -ne "0`n65`n") { throw "own early-return/assign published $d1" }

Invoke-Leaf "l2src\tests\unit_own_clean.lm2" "unit_own_clean" 0 "outer"
$d2 = Invoke-SpliceDrive "unit_own_clean" @"
        l2_m1(unit, 0)
        @: Lmx f 0
        f: lmx_branch_child(unit, 0U)
        c.printf("%d\n", lmx_char_value(f\data))
        return: 0
    end: main
end: external
"@
if ($d2 -ne "66`n") { throw "clean caller republished over explicit path write: $d2" }

Invoke-Leaf "l2src\tests\unit_own_eq.lm2" "unit_own_eq" 0 "outer"
$d3 = Invoke-SpliceDrive "unit_own_eq" @"
        l2_m1(unit, 0)
        @: Lmx f 0
        f: lmx_branch_child(unit, 0U)
        c.printf("%d\n", lmx_char_value(f\data))
        return: 0
    end: main
end: external
"@
if ($d3 -ne "0`n") { throw "equal assign must dirty and publish: $d3" }

Invoke-Leaf "l2src\tests\unit_own_lazy.lm2" "unit_own_lazy" 0 "m"
$d4 = Invoke-SpliceDrive "unit_own_lazy" @"
        l2_m2(unit, 0)
        @: Lmx f 0
        f: lmx_branch_child(unit, 0U)
        c.printf("%d\n", lmx_char_value(f\data))
        return: 0
    end: main
end: external
"@
if ($d4 -ne "0`n") { throw "lazy && evaluated RHS call: $d4" }
$lz = [System.IO.File]::ReadAllText((Join-Path (Get-Location) (Join-Path $out "unit_own_lazy.lm1")))
if ($lz -match 'l2_m1\(node, \(0 && l2_m0') { throw "unit_own_lazy still inlines nested call into C && actual" }

Invoke-Leaf "l2src\tests\unit_own_skip.lm2" "unit_own_skip" 0 "m"
$skg = [System.IO.File]::ReadAllText((Join-Path (Get-Location) (Join-Path $out "unit_own_skip.lm1")))
if ($skg -notmatch 'if: l2_t\d+' ) { throw "unit_own_skip must emit guarded if for lazy &&/||" }
$d5 = Invoke-SpliceDrive "unit_own_skip" @"
        l2_m2(unit, 0)
        l2_m2(unit, 1)
        l2_m2(unit, 2)
        l2_m2(unit, 3)
        l2_m2(unit, 4)
        @: Lmx f 0
        f: lmx_branch_child(unit, 0U)
        c.printf("%d\n", lmx_char_value(f\data))
        return: 0
    end: main
end: external
"@
if ($d5 -ne "0`n") { throw "0 && wrap(boom) / 1 || wrap(boom) must skip boom: $d5" }

Invoke-Leaf "l2src\tests\unit_own_dirty_rhs.lm2" "unit_own_dirty_rhs" 0 "m"
$d6 = Invoke-SpliceDrive "unit_own_dirty_rhs" @"
        l2_m1(unit, 0)
        @: Lmx f 0
        f: lmx_branch_child(unit, 0U)
        c.printf("%d\n", lmx_char_value(f\data))
        return: 0
    end: main
end: external
"@
if ($d6 -ne "88`n") { throw "dirty own must be published before executed RHS call: $d6" }

Invoke-Leaf "l2src\tests\unit_own_twoact.lm2" "unit_own_twoact" 0 "m"
$d7 = Invoke-SpliceDrive "unit_own_twoact" @"
        l2_m3(unit, 0)
        @: Lmx f 0
        f: lmx_branch_child(unit, 0U)
        c.printf("%d\n", lmx_char_value(f\data))
        return: 0
    end: main
end: external
"@
if ($d7 -ne "50`n") { throw "two actuals must be LTR once (expect b's 50): $d7" }

Invoke-Leaf "l2src\tests\unit_own_prec2.lm2" "unit_own_prec2" 0 "m"
$d8 = Invoke-SpliceDrive "unit_own_prec2" @"
        l2_m1(unit, 0)
        @: Lmx f 0
        f: lmx_branch_child(unit, 0U)
        c.printf("%d\n", lmx_char_value(f\data))
        l2_m1(unit, 1)
        f: lmx_branch_child(unit, 0U)
        c.printf("%d\n", lmx_char_value(f\data))
        l2_m1(unit, 2)
        f: lmx_branch_child(unit, 0U)
        c.printf("%d\n", lmx_char_value(f\data))
        return: 0
    end: main
end: external
"@
if ($d8 -ne "0`n0`n90`n") { throw "mixed &&/|| precedence skip/run: $d8" }

# C99 &&/|| yield int 0/1. Oracle is C, not this emitter.
Invoke-Leaf "l2src\tests\unit_bool_and.lm2" "unit_bool_and" 1 "f"
$ba = [System.IO.File]::ReadAllText((Join-Path (Get-Location) (Join-Path $out "unit_bool_and.lm1")))
if ($ba -notmatch '!= 0') { throw "unit_bool_and must normalize && to 0/1" }
Invoke-Leaf "l2src\tests\unit_bool_or.lm2" "unit_bool_or" 1 "f"

Invoke-Leaf "l2src\tests\unit_bool_skip.lm2" "unit_bool_skip" 0 "m"
$d9 = Invoke-SpliceDrive "unit_bool_skip" @"
        @: Lmx f 0
        c.printf("%d\n", l2_m1(unit, 0))
        f: lmx_branch_child(unit, 0U)
        c.printf("%d\n", lmx_char_value(f\data))
        c.printf("%d\n", l2_m1(unit, 1))
        f: lmx_branch_child(unit, 0U)
        c.printf("%d\n", lmx_char_value(f\data))
        c.printf("%d\n", l2_m1(unit, 2))
        f: lmx_branch_child(unit, 0U)
        c.printf("%d\n", lmx_char_value(f\data))
        return: 0
    end: main
end: external
"@
if ($d9 -ne "1`n0`n1`n0`n0`n0`n") { throw "7||boom / -1||boom / 0&&boom return/sideeffect: $d9" }

Invoke-Leaf "l2src\tests\unit_bool_mix.lm2" "unit_bool_mix" 0 "m"
$d10 = Invoke-SpliceDrive "unit_bool_mix" @"
        c.printf("%d\n", l2_m1(unit, 0))
        c.printf("%d\n", l2_m1(unit, 1))
        c.printf("%d\n", l2_m1(unit, 2))
        c.printf("%d\n", l2_m1(unit, 3))
        return: 0
    end: main
end: external
"@
# C: 1||0&&7 -> 1; 0||1&&7 -> 1; 1&&7 -> 1; 0&&7 -> 0
if ($d10 -ne "1`n1`n1`n0`n") { throw "nested mixed &&/|| C 0/1 results: $d10" }

Invoke-Leaf "l2src\tests\unit_sz_id.lm2" "unit_sz_id" 0 "id"
$sz = [System.IO.File]::ReadAllText((Join-Path (Get-Location) (Join-Path $out "unit_sz_id.lm1")))
if ($sz -notmatch 'size_t: l2_t') { throw "unit_sz_id wrap/id must keep size_t call temp" }
$wrapFn = [regex]::Match($sz, 'fn: l2_m1[\s\S]*?end: l2_m1').Value
if ($wrapFn -match 'int: l2_t') { throw "parenthesized size_t wrap must not copy into int temp:`n$wrapFn" }
$d11 = Invoke-SpliceDrive "unit_sz_id" @"
        c.printf("%zu\n", l2_m0(unit, 2147483648U))
        c.printf("%zu\n", l2_m1(unit, 2147483648U))
        return: 0
    end: main
end: external
"@
if ($d11 -ne "2147483648`n2147483648`n") { throw "size_t id/wrap lost high bits: $d11" }

function Invoke-StartsPython {
    $refLm1 = Join-Path $out "spy_ref.lm1"
    $refC = Join-Path $out "spy_ref.c"
    $refExe = Join-Path $out "spy_ref.exe"
    $refOut = Join-Path $out "spy_ref.stdout"
    $refSrc = @"
predef: "l1src/parser_text.lm1"
include: "<stdio.h>"
external:
    fn: main () int
        c.printf("%d\n", lm_p0_starts_python_string("'''", 3U, 0U))
        c.printf("%d\n", lm_p0_starts_python_string("\x22\x22\x22", 3U, 0U))
        c.printf("%d\n", lm_p0_starts_python_string("'", 1U, 0U))
        c.printf("%d\n", lm_p0_starts_python_string("''x", 3U, 0U))
        c.printf("%d\n", lm_p0_starts_python_string("x'''", 4U, 1U))
        c.printf("%d\n", lm_p0_starts_python_string("'''", 3U, 1U))
        c.printf("%d\n", lm_p0_starts_python_string("'''", 3U, 3U))
        c.printf("%d\n", lm_p0_starts_python_string("", 0U, 0U))
        c.printf("%d\n", lm_p0_starts_python_string("'\x22\x22", 3U, 0U))
        c.printf("%d\n", lm_p0_starts_python_string("abc", 3U, 0U))
        return: 0
    end: main
end: external
"@
    [System.IO.File]::WriteAllText((Join-Path (Get-Location) $refLm1), $refSrc.Replace("`r`n","`n"))
    & $l1trans $refLm1 $refC
    if ($LASTEXITCODE -ne 0) { throw "l1trans failed spy_ref" }
    Invoke-Gcc $refC $refExe (Join-Path $log "spy_ref.gcc.log")
    cmd /c "`"$refExe`" > `"$refOut`" 2> `"$(Join-Path $out 'spy_ref.err')`""
    if ($LASTEXITCODE -ne 0) { throw "spy_ref exe failed" }

    Invoke-Leaf "l2src\parser_text_starts_python.lm2" "parser_text_starts_python" 0 "lm_p0_starts_python_string"
    $lm1 = Join-Path $out "parser_text_starts_python.lm1"
    $text = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $lm1)).Replace("`r`n", "`n")
    if ($text -match 'fn: lm_p0_starts_python_string') { throw "starts_python must mangle method symbols" }
    if ($text.IndexOf("l2_q0") -lt 0) { throw "starts_python missing own char cache" }
    if ($text.IndexOf('l2_own0) "quote"') -lt 0) { throw "starts_python OwnUsed must keep source name quote" }
    $tail = "        return: 0`n    end: main`nend: external"
    $pos = $text.LastIndexOf($tail)
    if ($pos -lt 0) { throw "starts_python L1 missing generated main return" }
    $drive = @"
        c.printf("%d\n", l2_m0(unit, "'''", 3U, 0U))
        c.printf("%d\n", l2_m0(unit, "\x22\x22\x22", 3U, 0U))
        c.printf("%d\n", l2_m0(unit, "'", 1U, 0U))
        c.printf("%d\n", l2_m0(unit, "''x", 3U, 0U))
        c.printf("%d\n", l2_m0(unit, "x'''", 4U, 1U))
        c.printf("%d\n", l2_m0(unit, "'''", 3U, 1U))
        c.printf("%d\n", l2_m0(unit, "'''", 3U, 3U))
        c.printf("%d\n", l2_m0(unit, "", 0U, 0U))
        c.printf("%d\n", l2_m0(unit, "'\x22\x22", 3U, 0U))
        c.printf("%d\n", l2_m0(unit, "abc", 3U, 0U))
        @: Lmx f 0
        f: lmx_branch_child(unit, 0U)
        c.printf("%d\n", lmx_char_value(f\data))
        return: 0
    end: main
end: external
"@
    $drvLm1 = Join-Path $out "spy_l2_drive.lm1"
    $drvC = Join-Path $out "spy_l2_drive.c"
    $drvExe = Join-Path $out "spy_l2_drive.exe"
    $drvOut = Join-Path $out "spy_l2_drive.stdout"
    [System.IO.File]::WriteAllText((Join-Path (Get-Location) $drvLm1), ($text.Substring(0, $pos) + $drive.Replace("`r`n","`n")))
    & $l1trans $drvLm1 $drvC
    if ($LASTEXITCODE -ne 0) { throw "l1trans failed spy_l2_drive" }
    Invoke-Gcc $drvC $drvExe (Join-Path $log "spy_l2_drive.gcc.log")
    cmd /c "`"$drvExe`" > `"$drvOut`" 2> `"$(Join-Path $out 'spy_l2_drive.err')`""
    if ($LASTEXITCODE -ne 0) { throw "spy_l2_drive exe failed" }
    $a = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $refOut)).Replace("`r`n","`n")
    $b = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $drvOut)).Replace("`r`n","`n")
    $alines = $a.Split("`n") | Where-Object { $_ -ne "" }
    $blines = $b.Split("`n") | Where-Object { $_ -ne "" }
    if ($alines.Count -ne 10) { throw "spy_ref expected 10 lines, got $($alines.Count)" }
    if ($blines.Count -ne 11) { throw "spy_l2 expected 10 cases + published field, got $($blines.Count)" }
    for ($i = 0; $i -lt 10; $i++) {
        if ($alines[$i] -ne $blines[$i]) { throw "starts_python_string mismatch vs parser_text.lm1 at case $i ref=$($alines[$i]) l2=$($blines[$i])" }
    }
    if ($blines[10] -ne "97") { throw "published own field via generated child 0 must be last assigned quote (abc -> 97), got $($blines[10])" }
}

Invoke-StartsPython

Invoke-Negative "l2src\tests\unit_unknown_c.lm2" "unit_unknown_c" "unknown method"
Invoke-Negative "l2src\tests\unit_unknown_field.lm2" "unit_unknown_field" "unknown foreign field"
Invoke-Negative "l2src\tests\unit_unknown_type.lm2" "unit_unknown_type" "unknown foreign type"
Invoke-Negative "l2src\tests\unit_const_write.lm2" "unit_const_write" "const write"

function Invoke-Views {
    $refLm1 = Join-Path $out "views_ref.lm1"
    $refC = Join-Path $out "views_ref.c"
    $refExe = Join-Path $out "views_ref.exe"
    $refOut = Join-Path $out "views_ref.stdout"
    $refSrc = @"
predef: "l1src/parser_text.lm1"
include: "<stdio.h>" "<stdlib.h>"
external:
    fn: main () int
        @: LmP0Text t 0
        @: LmP0Text atom 0
        @: LmP0Text pay 0
        t: (cast: (@: LmP0Text) c.malloc(c.sizeof(c.LmP0Text)))
        atom: (cast: (@: LmP0Text) c.malloc(c.sizeof(c.LmP0Text)))
        pay: (cast: (@: LmP0Text) c.malloc(c.sizeof(c.LmP0Text)))
        if: t = 0 || atom = 0 || pay = 0
            return: 1
        t\data: "hello"
        t\length: 5U
        c.printf("%d\n", lm_p0_text_equals(t, "hello"))
        c.printf("%d\n", lm_p0_text_equals(t, "Hello"))
        c.printf("%d\n", lm_p0_text_equals(t, "hell"))
        c.printf("%d\n", lm_p0_text_equals(0, "hello"))
        c.printf("%d\n", lm_p0_text_equals(t, 0))
        t\data: ""
        t\length: 0U
        c.printf("%d\n", lm_p0_text_equals(t, ""))
        t\data: "x"
        t\length: 0U
        c.printf("%d\n", lm_p0_text_equals(t, ""))
        c.array: [4]: char nbuf
        nbuf[0]: 97
        nbuf[1]: 98
        nbuf[2]: 0
        nbuf[3]: 99
        t\data: nbuf
        t\length: 4U
        c.printf("%d\n", lm_p0_text_equals(t, "ab"))
        atom\data: "abc"
        atom\length: 3U
        c.printf("%d\n", lm_p0_identifier_payload(atom, pay))
        c.printf("%d\n", pay\data = atom\data)
        c.printf("%zu\n", pay\length)
        atom\data: "`xy`"
        atom\length: 4U
        c.printf("%d\n", lm_p0_identifier_payload(atom, pay))
        c.printf("%d\n", pay\data = atom\data + 1U)
        c.printf("%zu\n", pay\length)
        atom\data: "`x"
        atom\length: 2U
        c.printf("%d\n", lm_p0_identifier_payload(atom, pay))
        c.printf("%zu\n", pay\length)
        c.printf("%d\n", lm_p0_identifier_payload(0, pay))
        c.printf("%d\n", lm_p0_identifier_payload(atom, 0))
        atom\data: 0
        atom\length: 1U
        c.printf("%d\n", lm_p0_identifier_payload(atom, pay))
        return: 0
    end: main
end: external
"@
    [System.IO.File]::WriteAllText((Join-Path (Get-Location) $refLm1), $refSrc.Replace("`r`n","`n"))
    & $l1trans $refLm1 $refC
    if ($LASTEXITCODE -ne 0) { throw "l1trans failed views_ref" }
    Invoke-Gcc $refC $refExe (Join-Path $log "views_ref.gcc.log")
    cmd /c "`"$refExe`" > `"$refOut`" 2> `"$(Join-Path $out 'views_ref.err')`""
    if ($LASTEXITCODE -ne 0) { throw "views_ref exe failed" }

    Invoke-Leaf "l2src\parser_text_views.lm2" "parser_text_views" 0 "lm_p0_text_equals"
    $lm1 = Join-Path $out "parser_text_views.lm1"
    $text = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $lm1)).Replace("`r`n", "`n")
    if ($text -match 'fn: lm_p0_text_equals') { throw "views must mangle method symbols" }
    if ($text.IndexOf("const: @(LmP0Text") -lt 0) { throw "views missing const LmP0Text* formal" }
    if ($text.IndexOf("@: LmP0Text") -lt 0) { throw "views missing mutable LmP0Text* formal" }
    if ($text.IndexOf("size_t: l2_q0") -lt 0) { throw "views missing own size_t cache" }
    if ($text.IndexOf("lmx_size_take") -lt 0) { throw "views missing size_t pool take" }
    if ($text.IndexOf("l2_hash_eq") -lt 0) { throw "views missing hashed equals" }
    if ($text.IndexOf("l2_hash_bind") -ge 0) { throw "views must not auto-bind a mutable payload" }
    if ($text.IndexOf("l1src/p0.h") -lt 0) { throw "views missing p0.h adapter include" }
    if ($text.IndexOf("const-pointee") -lt 0) { throw "views intern comment must distinguish const pointee" }
    $tail = "        return: 0`n    end: main`nend: external"
    $pos = $text.LastIndexOf($tail)
    if ($pos -lt 0) { throw "views L1 missing generated main return" }
    $drive = @"
        @: LmP0Text t 0
        @: LmP0Text atom 0
        @: LmP0Text pay 0
        @: Lmx f 0
        c.array: [4]: char nbuf
        c.array: [6]: char mut
        t: (cast: (@: LmP0Text) c.malloc(c.sizeof(c.LmP0Text)))
        atom: (cast: (@: LmP0Text) c.malloc(c.sizeof(c.LmP0Text)))
        pay: (cast: (@: LmP0Text) c.malloc(c.sizeof(c.LmP0Text)))
        if: t = 0 || atom = 0 || pay = 0
            return: 1
        t\data: "hello"
        t\length: 5U
        c.printf("%d\n", l2_m0(unit, t, "hello"))
        c.printf("%d\n", l2_m0(unit, t, "Hello"))
        c.printf("%d\n", l2_m0(unit, t, "hell"))
        c.printf("%d\n", l2_m0(unit, 0, "hello"))
        c.printf("%d\n", l2_m0(unit, t, 0))
        t\data: ""
        t\length: 0U
        c.printf("%d\n", l2_m0(unit, t, ""))
        t\data: "x"
        t\length: 0U
        c.printf("%d\n", l2_m0(unit, t, ""))
        nbuf[0]: 97
        nbuf[1]: 98
        nbuf[2]: 0
        nbuf[3]: 99
        t\data: nbuf
        t\length: 4U
        c.printf("%d\n", l2_m0(unit, t, "ab"))
        atom\data: "abc"
        atom\length: 3U
        c.printf("%d\n", l2_m1(unit, atom, pay))
        c.printf("%d\n", pay\data = atom\data)
        c.printf("%zu\n", pay\length)
        atom\data: "`xy`"
        atom\length: 4U
        c.printf("%d\n", l2_m1(unit, atom, pay))
        c.printf("%d\n", pay\data = atom\data + 1U)
        c.printf("%zu\n", pay\length)
        atom\data: "`x"
        atom\length: 2U
        c.printf("%d\n", l2_m1(unit, atom, pay))
        c.printf("%zu\n", pay\length)
        c.printf("%d\n", l2_m1(unit, 0, pay))
        c.printf("%d\n", l2_m1(unit, atom, 0))
        atom\data: 0
        atom\length: 1U
        c.printf("%d\n", l2_m1(unit, atom, pay))
        f: lmx_branch_child(unit, 0U)
        c.printf("%zu\n", lmx_size_value(f\data))
        t\data: "ab"
        t\length: 2U
        c.printf("%d\n", l2_m0(unit, t, "ab"))
        f: lmx_branch_child(unit, 0U)
        c.printf("%zu\n", lmx_size_value(f\data))
        c.printf("%d\n", l2_m0(unit, 0, "zzzz"))
        f: lmx_branch_child(unit, 0U)
        c.printf("%zu\n", lmx_size_value(f\data))
        mut[0]: 97
        mut[1]: 97
        mut[2]: 97
        mut[3]: 0
        atom\data: mut
        atom\length: 3U
        c.printf("%d\n", l2_m1(unit, atom, pay))
        c.printf("%d\n", l2_m0(unit, pay, "aaa"))
        mut[0]: 98
        mut[1]: 98
        mut[2]: 98
        c.printf("%d\n", l2_m0(unit, pay, "bbb"))
        c.printf("%d\n", l2_m0(unit, pay, "aaa"))
        mut[0]: 99
        mut[1]: 99
        mut[2]: 99
        c.printf("%d\n", l2_m0(unit, pay, "ccc"))
        atom\data: "xyz"
        atom\length: 3U
        c.printf("%d\n", l2_m1(unit, atom, pay))
        c.printf("%d\n", l2_m0(unit, pay, "xyz"))
        c.printf("%d\n", l2_m0(unit, pay, "ccc"))
        c.printf("%d\n", l2_hash_eq_forced(pay, "aaa", 3U, 1ULL, 1ULL))
        return: 0
    end: main
end: external
"@
    $drvLm1 = Join-Path $out "views_l2_drive.lm1"
    $drvC = Join-Path $out "views_l2_drive.c"
    $drvExe = Join-Path $out "views_l2_drive.exe"
    $drvOut = Join-Path $out "views_l2_drive.stdout"
    [System.IO.File]::WriteAllText((Join-Path (Get-Location) $drvLm1), ($text.Substring(0, $pos) + $drive.Replace("`r`n","`n")))
    & $l1trans $drvLm1 $drvC
    if ($LASTEXITCODE -ne 0) { throw "l1trans failed views_l2_drive" }
    Invoke-Gcc $drvC $drvExe (Join-Path $log "views_l2_drive.gcc.log")
    cmd /c "`"$drvExe`" > `"$drvOut`" 2> `"$(Join-Path $out 'views_l2_drive.err')`""
    if ($LASTEXITCODE -ne 0) { throw "views_l2_drive exe failed" }
    $a = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $refOut)).Replace("`r`n","`n")
    $b = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $drvOut)).Replace("`r`n","`n")
    $alines = $a.Split("`n") | Where-Object { $_ -ne "" }
    $blines = $b.Split("`n") | Where-Object { $_ -ne "" }
    if ($alines.Count -ne 19) { throw "views_ref expected 19 lines, got $($alines.Count) : $a" }
    if ($blines.Count -lt 19) { throw "views_l2 shorter than L1 ref, got $($blines.Count) : $b" }
    for ($i = 0; $i -lt 19; $i++) {
        if ($alines[$i] -ne $blines[$i]) { throw "views mismatch vs parser_text.lm1 at $i ref=$($alines[$i]) l2=$($blines[$i])" }
    }
    $got = ($blines[19..($blines.Count - 1)] -join ",")
    $want = "2,1,2,0,2,1,1,1,0,1,1,1,0,0"
    if ($got -ne $want) { throw "views extra own/hash/mutation got=$got want=$want full=$b" }
}

Invoke-Views

"l2trans $gen ok"
