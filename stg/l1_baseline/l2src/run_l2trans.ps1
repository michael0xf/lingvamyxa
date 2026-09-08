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
if ($sc -notmatch '1 \|\| div0\(' -and $sc.IndexOf("1 || div0(") -lt 0) { throw "unit_sc must inline right-hand call for short-circuit" }

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
    $text = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $lm1))
    $cut = $text.LastIndexOf("external:")
    if ($cut -lt 0) { throw "predicates L1 missing external main" }
    $head = $text.Substring(0, $cut)
    $drive = @"
external:
    fn: main () int
        @: Lmx unit 0
        @: Lmx leaf 0
        @: LmxMethod rec 0
        int: v 0
        if: lmx_ranges_init(8U) != 0
            return: 1
        unit: (cast: (@: Lmx) c.malloc(c.sizeof(c.Lmx)))
        if: unit = 0
            return: 1
        lmx_cell_init(unit, 0, 0)
        if: lmx_branch_open(unit, 7U) != 0
            return: 1
        rec: (cast: (@: LmxMethod) c.malloc(c.sizeof(c.LmxMethod)))
        if: rec = 0
            return: 1
        rec\addr: (cast: (LmxEntry) lm_p0_is_horizontal_space)
        rec\sig: 1U
        if: lmx_range_register((cast: (@: void) rec), (cast: (@: void) (rec + 1)), c.sizeof(c.LmxMethod), c.LMX_KIND_METHOD, c.LMX_TYPE_METHOD) != 0
            return: 1
        leaf: lmx_branch_child(unit, 0U)
        leaf\data: (cast: (@: void) rec)
        while: v < 128
            c.printf("%d%d%d%d%d%d%d\n", lm_p0_is_horizontal_space(unit, (cast: (char) v)), lm_p0_is_line_break(unit, (cast: (char) v)), lm_p0_is_field_space(unit, (cast: (char) v)), lm_p0_is_field_separator(unit, (cast: (char) v)), lm_p0_is_short_form_separator(unit, (cast: (char) v)), lm_p0_is_quoted_token_boundary(unit, (cast: (char) v)), lm_p0_is_decimal_digit(unit, (cast: (char) v)))
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

"l2trans $gen ok"
