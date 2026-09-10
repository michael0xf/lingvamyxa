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
    $flags = [System.Collections.Generic.List[string]]::new()
    foreach ($f in $cflags) { [void]$flags.Add($f) }
    $src = ""
    if (Test-Path -LiteralPath $cpath) {
        $src = [IO.File]::ReadAllText((Join-Path (Get-Location) $cpath))
    }
    $wantGen = $src.IndexOf("l1src/p0.lm1.h") -ge 0
    if ($wantGen) { [void]$flags.Add("-I"); [void]$flags.Add("lm1/build") }
    $flagStr = ($flags -join " ")
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

function Invoke-FmtBuf {
    $obj = Join-Path $out "l2trans_nomain.o"
    $src = "l2src\tests\fmt_buf.c"
    $exe = Join-Path $out "fmt_buf.exe"
    $clog = Join-Path $log "fmt_buf_l2trans.gcc.log"
    $tlog = Join-Path $log "fmt_buf.gcc.log"
    $flags = [System.Collections.Generic.List[string]]::new()
    foreach ($f in $cflags) { [void]$flags.Add($f) }
    [void]$flags.Add("-I"); [void]$flags.Add("lm1/build")
    $flagStr = ($flags -join " ")
    cmd /c "gcc $flagStr -Dmain=l2trans_main -c `"$l2c`" -o `"$obj`" > `"$clog`" 2>&1"
    if ($LASTEXITCODE -ne 0) {
        Get-Content $clog
        throw "gcc failed: l2trans nomain object"
    }
    cmd /c "gcc $flagStr `"$src`" `"$obj`" -o `"$exe`" > `"$tlog`" 2>&1"
    if ($LASTEXITCODE -ne 0) {
        Get-Content $tlog
        throw "gcc failed: $src"
    }
    & $exe
    if ($LASTEXITCODE -ne 0) { throw "fmt_buf failed" }
}
Invoke-FmtBuf


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
Invoke-Positive "l2src\tests\entry_ret_tr.lm2" "entry_ret_tr" 0 "0"
Invoke-Puts "l2src\tests\entry_ret_tr_puts.lm2" "entry_ret_tr_puts" 0 "MUST PRINT`n"
Invoke-Puts "l2src\tests\entry_ret_tr_two.lm2" "entry_ret_tr_two" 0 "one`ntwo`n"
Invoke-Negative "l2src\tests\entry_ret_tr_bad.lm2" "entry_ret_tr_bad" "unsupported"

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
Invoke-Puts "l2src\tests\entry_puts_triple.lm2" "entry_puts_triple" 0 ('a"""b' + "`n")
Invoke-Puts "l2src\tests\entry_puts_triple_single.lm2" "entry_puts_triple_single" 0 ("a'''b" + "`n")
Invoke-Puts "l2src\tests\entry_puts_triple_runs.lm2" "entry_puts_triple_runs" 0 ('a"b""c"""d' + "`n")
Invoke-Puts "l2src\tests\entry_puts_triple_lead.lm2" "entry_puts_triple_lead" 0 ('"hello' + "`n")
Invoke-Puts "l2src\tests\entry_puts_triple_seven.lm2" "entry_puts_triple_seven" 0 ('"""x' + "`n")
Invoke-Puts "l2src\tests\entry_puts_triple_lead_sq.lm2" "entry_puts_triple_lead_sq" 0 ("'hello" + "`n")
Invoke-Puts "l2src\tests\entry_puts_triple_seven_sq.lm2" "entry_puts_triple_seven_sq" 0 ("'''x" + "`n")
$longXs = "x" * 100
Invoke-Puts "l2src\tests\entry_puts_triple_long.lm2" "entry_puts_triple_long" 0 ($longXs + "`n")
$longLm1 = [System.IO.File]::ReadAllText((Join-Path (Get-Location) "build\l2trans\entry_puts_triple_long.lm1"))
if ($longLm1.IndexOf($longXs) -lt 0) { throw "triple_long L1 truncated: missing 100 x payload" }
Invoke-Negative "l2src\tests\entry_puts_triple_fence4.lm2" "entry_puts_triple_fence4" "unterminated python-like string"
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

function Get-L2Call([string]$text, [int]$mi, [string[]]$vals) {
    $m = [regex]::Match($text, "fn: l2_m$mi \(@: Lmx node([^)]*)\)")
    if (-not $m.Success) { throw "missing prototype l2_m$mi" }
    $rest = $m.Groups[1].Value
    $n = @($rest.Split(';') | Where-Object { $_.Trim().Length -gt 0 }).Count
    $args = @("unit")
    $i = 0
    while ($i -lt $n) {
        if ($i -lt $vals.Count) { $args += $vals[$i] } else { $args += "0" }
        $i++
    }
    return "l2_m$mi(" + ($args -join ", ") + ")"
}

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
Invoke-Leaf "l2src\tests\unit_loop.lm2" "unit_loop" 1 "add"
Invoke-Negative "l2src\tests\unit_for.lm2" "unit_for" "unsupported loop"
Invoke-Negative "l2src\tests\unit_cont_out.lm2" "unit_cont_out" "unsupported loop"
Invoke-Negative "l2src\tests\unit_cont_frame.lm2" "unit_cont_frame" "unsupported loop"
Invoke-Negative "l2src\tests\unit_cont_colon.lm2" "unit_cont_colon" "empty colon Frame is not allowed"
Invoke-Negative "l2src\tests\unit_break.lm2" "unit_break" "unsupported loop"
Invoke-Negative "l2src\tests\unit_sz_idx.lm2" "unit_sz_idx" "unsupported index"
Invoke-Negative "l2src\tests\unit_sz_np.lm2" "unit_sz_np" "unsupported index"
Invoke-Leaf "l2src\tests\unit_sz_intp.lm2" "unit_sz_intp" 0 "add"
$szintp = [System.IO.File]::ReadAllText((Join-Path (Get-Location) (Join-Path $out "unit_sz_intp.lm1")))
if ($szintp -notmatch '@: int l2_p0_0') { throw "unit_sz_intp missing @: int formal" }
Invoke-Leaf "l2src\tests\unit_addr_take.lm2" "unit_addr_take" 0 "set_one"
$addrTake = [System.IO.File]::ReadAllText((Join-Path (Get-Location) (Join-Path $out "unit_addr_take.lm1")))
if ($addrTake.IndexOf("&l2_q") -ge 0) { throw "unit_addr_take must not take address of own cache" }
if ($addrTake -notmatch 'cast: \(@: size_t\) l2_q0_from') { throw "unit_addr_take must pass graph cell, not cache" }
$callPos = $addrTake.LastIndexOf("l2_m0(")
if ($callPos -lt 0) { throw "unit_addr_take missing set_one call" }
$afterCall = $addrTake.Substring($callPos)
if ($afterCall -match 'l2_q\d+\s*:\s*lmx_size_value') { throw "unit_addr_take must not reload own cache after call" }
if ($afterCall -match 'l2_q\d+\s*:\s*\(cast:') { throw "unit_addr_take must not reload own cache after call" }
$addrGot = Invoke-SpliceDrive "unit_addr_take" @"
        c.printf("%d\n", l2_m1(unit))
        c.printf("%d\n", (cast: int (lmx_size_value(lmx_branch_child(unit, 0U)\data))))
        return: 0
    end: main
end: external
"@
if ($addrGot -ne "0`n1`n") { throw "unit_addr_take expected bare 0 and graph 1 got=$addrGot" }
Invoke-Leaf "l2src\tests\unit_addr_arg.lm2" "unit_addr_arg" 0 "set_one"
$addrArg = [System.IO.File]::ReadAllText((Join-Path (Get-Location) (Join-Path $out "unit_addr_arg.lm1")))
if ($addrArg.IndexOf("&l2_q") -ge 0) { throw "unit_addr_arg must not take address of own cache" }
if ($addrArg -notmatch '@ l2_p1_0') { throw "unit_addr_arg must take address of the C argument cell" }
$addrArgGot = Invoke-SpliceDrive "unit_addr_arg" @"
        c.printf("%d\n", l2_m1(unit, 0U))
        return: 0
    end: main
end: external
"@
if ($addrArgGot -ne "1`n") { throw "unit_addr_arg go expected 1 got=$addrArgGot" }
Invoke-Negative "l2src\tests\unit_addr_depth.lm2" "unit_addr_depth" "unsupported address depth"
Invoke-Leaf "l2src\tests\unit_dash_emit.lm2" "unit_dash_emit" 0 "go"
$dashGot = Invoke-SpliceDrive "unit_dash_emit" @"
        c.printf("%d\n", l2_m0(unit, 3))
        c.printf("%d\n", l2_m0(unit, 0))
        return: 0
    end: main
end: external
"@
if ($dashGot -ne "4`n0`n") { throw "unit_dash_emit go expected 4 then 0 got=$dashGot" }
Invoke-Negative "l2src\tests\unit_rec.lm2" "unit_rec" "unsupported recursion"
Invoke-Negative "l2src\tests\unit_cycle.lm2" "unit_cycle" "unsupported recursion"
Invoke-Leaf "l2src\tests\unit_eight.lm2" "unit_eight" 0 "m7"
$e8 = [System.IO.File]::ReadAllText((Join-Path (Get-Location) (Join-Path $out "unit_eight.lm1")))
if ($e8.IndexOf("lmx_ranges_init(9U)") -lt 0) { throw "unit_eight must init 9 ranges (8 methods + children)" }
if ($e8.IndexOf("l2_p0_0") -lt 0) { throw "unit_eight missing hygienic formal l2_p0_0" }
Invoke-Leaf "l2src\tests\unit_nine.lm2" "unit_nine" 0 "m8"
$e9 = [System.IO.File]::ReadAllText((Join-Path (Get-Location) (Join-Path $out "unit_nine.lm1")))
if ($e9.IndexOf("fn: l2_m8") -lt 0) { throw "unit_nine missing 9th method" }

function New-MethodNSource([string]$path, [int]$n) {
    $i = 0
    $body = ""
    while ($i -lt $n) {
        $next = $i + 1
        if ($next -eq $n) {
            $body += "fn: m$i (int: a; int: b) int`n    return: m0(a, b)`nend: m$i`n"
        } else {
            $body += "fn: m$i (int: a; int: b) int`n    return: a + b`nend: m$i`n"
        }
        $i = $i + 1
    }
    $src = $body + "fn: main () int`n    return: 0`nend: main`n"
    [System.IO.File]::WriteAllText((Join-Path (Get-Location) $path), $src.Replace("`r`n", "`n"))
}
function New-HiddenNSource([string]$path, [int]$n) {
    $i = 0
    $last = $n - 1
    $body = ""
    while ($i -lt $n) {
        if ($i -eq 1) {
            $body += "fn: m$i (int: a; int: b) int`n    return: a + b + quote`nend: m$i`n"
        } elseif ($i -eq $last) {
            $body += "fn: m$i (int: a; int: b) int`n    return: m1(a, b)`nend: m$i`n"
        } else {
            $body += "fn: m$i (int: a; int: b) int`n    return: a + b`nend: m$i`n"
        }
        $i = $i + 1
    }
    $body += "fn: holder () int`n    char: quote`n    quote: 3`n    return: m$last(1, 2)`nend: holder`n"
    $body += "fn: main () int`n    return: 0`nend: main`n"
    [System.IO.File]::WriteAllText((Join-Path (Get-Location) $path), $body.Replace("`r`n", "`n"))
}
$m17 = Join-Path $out "unit_m17.lm2"
New-MethodNSource $m17 17
Invoke-Leaf $m17 "unit_m17" 0 "m16"
$t17 = [System.IO.File]::ReadAllText((Join-Path (Get-Location) (Join-Path $out "unit_m17.lm1")))
if ($t17 -notmatch 'fn: l2_m16') { throw "unit_m17 missing method 16" }
$c16 = Get-L2Call $t17 16 @("1", "2")
$d17 = Invoke-SpliceDrive "unit_m17" @"
        c.printf("%d\n", $c16)
        return: 0
    end: main
end: external
"@
if ($d17 -ne "3`n") { throw "unit_m17 must execute m16->m0: $d17" }
$m33 = Join-Path $out "unit_m33.lm2"
New-MethodNSource $m33 33
Invoke-Leaf $m33 "unit_m33" 0 "m32"
$t33 = [System.IO.File]::ReadAllText((Join-Path (Get-Location) (Join-Path $out "unit_m33.lm1")))
if ($t33 -notmatch 'fn: l2_m32') { throw "unit_m33 missing method 32" }
$c32 = Get-L2Call $t33 32 @("1", "2")
$d33 = Invoke-SpliceDrive "unit_m33" @"
        c.printf("%d\n", $c32)
        return: 0
    end: main
end: external
"@
if ($d33 -ne "3`n") { throw "unit_m33 must execute m32->m0: $d33" }

$cyc = Join-Path $out "unit_cycle32.lm2"
$cb = ""
$ci = 0
while ($ci -lt 33) {
    if ($ci -eq 16) { $cb += "fn: m16 (int: a; int: b) int`n    return: m32(a, b)`nend: m16`n" }
    elseif ($ci -eq 32) { $cb += "fn: m32 (int: a; int: b) int`n    return: m16(a, b)`nend: m32`n" }
    else { $cb += "fn: m$ci (int: a; int: b) int`n    return: a + b`nend: m$ci`n" }
    $ci++
}
[System.IO.File]::WriteAllText((Join-Path (Get-Location) $cyc), ($cb + "fn: main () int`n    return: 0`nend: main`n").Replace("`r`n", "`n"))
Invoke-Negative $cyc "unit_cycle32" "unsupported recursion"

$h17 = Join-Path $out "unit_hidden17.lm2"
New-HiddenNSource $h17 17
Invoke-Leaf $h17 "unit_hidden17" 0 "holder"
$th17 = [System.IO.File]::ReadAllText((Join-Path (Get-Location) (Join-Path $out "unit_hidden17.lm1"))).Replace("`r`n", "`n")
if ($th17 -notmatch 'fn: l2_m16') { throw "unit_hidden17 missing method 16" }
$hold17 = Get-L2Call $th17 17 @()
$dh17 = Invoke-SpliceDrive "unit_hidden17" @"
        c.printf("%d\n", $hold17)
        return: 0
    end: main
end: external
"@
if ($dh17 -ne "6`n") { throw "unit_hidden17 m16 must through-quote across 16: $dh17" }
$h33 = Join-Path $out "unit_hidden33.lm2"
New-HiddenNSource $h33 33
Invoke-Leaf $h33 "unit_hidden33" 0 "holder"
$th33 = [System.IO.File]::ReadAllText((Join-Path (Get-Location) (Join-Path $out "unit_hidden33.lm1"))).Replace("`r`n", "`n")
if ($th33 -notmatch 'fn: l2_m32') { throw "unit_hidden33 missing method 32" }
$hold33 = Get-L2Call $th33 33 @()
$dh33 = Invoke-SpliceDrive "unit_hidden33" @"
        c.printf("%d\n", $hold33)
        return: 0
    end: main
end: external
"@
if ($dh33 -ne "6`n") { throw "unit_hidden33 m32 must through-quote across 32: $dh33" }

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
Invoke-Leaf "l2src\tests\unit_ret_tr.lm2" "unit_ret_tr" 0 "add"
Invoke-Leaf "l2src\tests\unit_end_fn.lm2" "unit_end_fn" 0 "add"
Invoke-Leaf "l2src\tests\unit_end_dash.lm2" "unit_end_dash" 0 "add"
Invoke-Negative "l2src\tests\unit_end_wrong.lm2" "unit_end_wrong" "end target does not match close target"
Invoke-Leaf "l2src\tests\unit_ret_tr_own.lm2" "unit_ret_tr_own" 0 "m"
$rto = [System.IO.File]::ReadAllText((Join-Path (Get-Location) (Join-Path $out "unit_ret_tr_own.lm1"))).Replace("`r`n", "`n")
$callRto = Get-L2Call $rto 0 @()
$drto = Invoke-SpliceDrive "unit_ret_tr_own" @"
        c.printf("%d\n", $callRto)
        @: Lmx f 0
        f: lmx_branch_child(unit, 0U)
        c.printf("%d\n", lmx_char_value(f\data))
        return: 0
    end: main
end: external
"@
if ($drto -ne "65`n65`n") { throw "return-trailer must publish own: $drto" }
Invoke-Leaf "l2src\tests\unit_ret_tr_hid.lm2" "unit_ret_tr_hid" 0 "outer"
$rth = [System.IO.File]::ReadAllText((Join-Path (Get-Location) (Join-Path $out "unit_ret_tr_hid.lm1"))).Replace("`r`n", "`n")
$callRth = Get-L2Call $rth 1 @()
$drth = Invoke-SpliceDrive "unit_ret_tr_hid" @"
        c.printf("%d\n", $callRth)
        return: 0
    end: main
end: external
"@
if ($drth -ne "65`n") { throw "return-trailer must discover hidden: $drth" }
Invoke-Leaf "l2src\tests\unit_main_ret_tr.lm2" "unit_main_ret_tr" 0 "add"

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

Invoke-Leaf "l2src\tests\unit_sub_ret.lm2" "unit_sub_ret" 0 "bump"
$sr = [System.IO.File]::ReadAllText((Join-Path (Get-Location) (Join-Path $out "unit_sub_ret.lm1")))
if ($sr -notmatch '(?m)^\s+return$') { throw "unit_sub_ret missing void return" }
if ($sr.IndexOf("l2_q0_dirty") -lt 0) { throw "unit_sub_ret missing dirty checkpoint before void return" }
$dsr = Invoke-SpliceDrive "unit_sub_ret" @"
        @: Lmx f 0
        l2_m0(unit)
        l2_m0(unit)
        f: lmx_branch_child(unit, 0U)
        c.printf("%d\n", lmx_char_value(f\data))
        l2_m1(unit)
        f: lmx_branch_child(unit, 1U)
        c.printf("%d\n", lmx_char_value(f\data))
        l2_m2(unit, 1)
        f: lmx_branch_child(unit, 1U)
        c.printf("%d\n", lmx_char_value(f\data))
        l2_m2(unit, 0)
        f: lmx_branch_child(unit, 1U)
        c.printf("%d\n", lmx_char_value(f\data))
        return: 0
    end: main
end: external
"@
if ($dsr -ne "2`n65`n65`n66`n") { throw "sub void return publish/skip: $dsr" }
Invoke-Negative "l2src\tests\unit_sub_retval.lm2" "unit_sub_retval" "unsupported body"
Invoke-Negative "l2src\tests\unit_fn_noret.lm2" "unit_fn_noret" "unsupported body"

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

Invoke-Leaf "l2src\tests\unit_own5.lm2" "unit_own5" 0 "m"
$o5 = [System.IO.File]::ReadAllText((Join-Path (Get-Location) (Join-Path $out "unit_own5.lm1")))
if ($o5 -notmatch 'const: @\(char l2_own4\)') { throw "unit_own5 missing 5th OwnUsed" }
Invoke-Leaf "l2src\tests\unit_own6.lm2" "unit_own6" 0 "m"
$o6 = [System.IO.File]::ReadAllText((Join-Path (Get-Location) (Join-Path $out "unit_own6.lm1")))
if ($o6 -notmatch 'const: @\(char l2_own5\)') { throw "unit_own6 missing 6th OwnUsed" }
Invoke-Negative "l2src\tests\unit_own_clash.lm2" "unit_own_clash" "incompatible entry signature"

function New-Own33Source([string]$path) {
    $decls = @()
    $after = @()
    $i = 0
    while ($i -lt 33) {
        $n = "n" + $i.ToString("00")
        $decls += "    char: $n"
        $after += "    ${n}: 0"
        $i = $i + 1
    }
    $src = @"
fn: m (char: z) int
    z: n00
$($decls -join "`n")
    if: z != 0
        z: n00
    while: z != 0
        z: n15
        ---
$($after -join "`n")
    return: 0
end: m
fn: main () int
    return: 0
end: main
"@
    [System.IO.File]::WriteAllText((Join-Path (Get-Location) $path), $src.Replace("`r`n", "`n"))
}

$own33 = Join-Path $out "unit_own33.lm2"
New-Own33Source $own33
Invoke-Leaf $own33 "unit_own33" 0 "m"
$o33 = [System.IO.File]::ReadAllText((Join-Path (Get-Location) (Join-Path $out "unit_own33.lm1")))
if ($o33 -notmatch 'const: @\(char l2_own32\)') { throw "unit_own33 missing 33rd OwnUsed" }
if ($o33 -notmatch 'l2_q32') { throw "unit_own33 missing 33rd own cache" }

Invoke-Leaf "l2src\tests\unit_own32_pub.lm2" "unit_own32_pub" 0 "m"
$p32 = [System.IO.File]::ReadAllText((Join-Path (Get-Location) (Join-Path $out "unit_own32_pub.lm1"))).Replace("`r`n", "`n")
$callM = Get-L2Call $p32 0 @("0")
$callB = Get-L2Call $p32 1 @("9")
$callS = Get-L2Call $p32 2 @("0")
$d32 = Invoke-SpliceDrive "unit_own32_pub" @"
        c.printf("%d\n", $callM)
        @: Lmx f 0
        f: lmx_branch_child(unit, 0U)
        c.printf("%d\n", lmx_char_value(f\data))
        f: lmx_branch_child(unit, 32U)
        c.printf("%d\n", lmx_char_value(f\data))
        c.printf("%d\n", $callB)
        f: lmx_branch_child(unit, 32U)
        c.printf("%d\n", lmx_char_value(f\data))
        f: lmx_branch_child(unit, 0U)
        c.printf("%d\n", lmx_char_value(f\data))
        c.printf("%d\n", $callS)
        f: lmx_branch_child(unit, 32U)
        c.printf("%d\n", lmx_char_value(f\data))
        f: lmx_branch_child(unit, 0U)
        c.printf("%d\n", lmx_char_value(f\data))
        return: 0
    end: main
end: external
"@
if ($d32 -ne "0`n1`n77`n3`n3`n1`n65`n65`n1`n") { throw "n32 before/after decl and n00 unaliased: $d32" }

Invoke-Leaf "l2src\tests\unit_own_grow_alias.lm2" "unit_own_grow_alias" 0 "m"
$ga = [System.IO.File]::ReadAllText((Join-Path (Get-Location) (Join-Path $out "unit_own_grow_alias.lm1"))).Replace("`r`n", "`n")
$callG = Get-L2Call $ga 0 @("1")
$callL = Get-L2Call $ga 1 @("2")
$dga = Invoke-SpliceDrive "unit_own_grow_alias" @"
        c.printf("%d\n", $callG)
        @: Lmx f 0
        f: lmx_branch_child(unit, 0U)
        c.printf("%d\n", lmx_char_value(f\data))
        c.printf("%d\n", $callL)
        f: lmx_branch_child(unit, 0U)
        c.printf("%d\n", lmx_char_value(f\data))
        return: 0
    end: main
end: external
"@
if ($dga -ne "65`n65`n66`n66`n") { throw "grow must keep n00 alias on under-construction row: $dga" }

Invoke-Leaf "l2src\tests\unit_own_same_name.lm2" "unit_own_same_name" 0 "left"
$ownSame = [System.IO.File]::ReadAllText((Join-Path (Get-Location) (Join-Path $out "unit_own_same_name.lm1")))
if ($ownSame.IndexOf("l2_q0: 1U") -lt 0) { throw "unit_own_same_name left missing i=1" }
if ($ownSame.IndexOf("l2_q0: 2U") -lt 0) { throw "unit_own_same_name right missing i=2" }
Invoke-Leaf "l2src\tests\unit_own_meth.lm2" "unit_own_meth" 0 "m0"
$om = [System.IO.File]::ReadAllText((Join-Path (Get-Location) (Join-Path $out "unit_own_meth.lm1"))).Replace("`r`n", "`n")
if ($om -notmatch 'fn: l2_m4') { throw "unit_own_meth missing 5th method" }
$callOm = Get-L2Call $om 0 @()
$dom = Invoke-SpliceDrive "unit_own_meth" @"
        c.printf("%d\n", $callOm)
        @: Lmx f 0
        f: lmx_branch_child(unit, 0U)
        c.printf("%d\n", lmx_char_value(f\data))
        return: 0
    end: main
end: external
"@
if ($dom -ne "0`n1`n") { throw "method growth must keep first-method OwnUsed: $dom" }

Invoke-Leaf "l2src\tests\unit_bind.lm2" "unit_bind" 0 "bind"
$bg = [System.IO.File]::ReadAllText((Join-Path (Get-Location) (Join-Path $out "unit_bind.lm1"))).Replace("`r`n", "`n")
$bindFn = [regex]::Match($bg, 'fn: l2_m2[\s\S]*?end: l2_m2').Value
if ($bindFn.Length -lt 20) { throw "unit_bind missing mangled bind l2_m2" }
if ($bindFn -match 'char: l2_q') { throw "aliased own must not emit a separate l2_q value" }
if ($bindFn.IndexOf("lmx_char_value") -ge 0) { throw "aliased own must not load graph over the incoming argument" }
if ($bindFn.IndexOf("l2_p2_0") -lt 0) { throw "aliased own must use the parameter l2_p2_0" }
if ($bindFn.IndexOf("l2_q0_dirty: 1") -lt 0) { throw "aliased own assign after bind must dirty" }
if ($bindFn -notmatch 'l2_p2_0: 1') { throw "assign before own-decl must write the parameter" }
$passFn = [regex]::Match($bg, 'fn: l2_m1[\s\S]*?end: l2_m1').Value
if ($passFn.IndexOf("l2_q0_dirty") -ge 0) { throw "parameter without own bind must never dirty" }
if ($passFn.IndexOf("l2_p1_0: 65") -lt 0) { throw "parameter without own bind must still accept local assign" }
$dBind = Invoke-SpliceDrive "unit_bind" @"
        @: Lmx f 0
        l2_m1(unit, 90)
        f: lmx_branch_child(unit, 0U)
        c.printf("%d\n", lmx_char_value(f\data))
        l2_m6(unit, 90, 0)
        f: lmx_branch_child(unit, 0U)
        c.printf("%d\n", lmx_char_value(f\data))
        l2_m6(unit, 90, 1)
        f: lmx_branch_child(unit, 0U)
        c.printf("%d\n", lmx_char_value(f\data))
        l2_m2(unit, 90)
        f: lmx_branch_child(unit, 0U)
        c.printf("%d\n", lmx_char_value(f\data))
        l2_m3(unit, 77)
        f: lmx_branch_child(unit, 0U)
        c.printf("%d\n", lmx_char_value(f\data))
        l2_m4(unit, 1)
        f: lmx_branch_child(unit, 0U)
        c.printf("%d\n", lmx_char_value(f\data))
        l2_m5(unit, 1)
        f: lmx_branch_child(unit, 0U)
        c.printf("%d\n", lmx_char_value(f\data))
        return: 0
    end: main
end: external
"@
if ($dBind -ne "0`n0`n65`n66`n77`n66`n65`n") { throw "same-name bind graph got $dBind" }

Invoke-Leaf "l2src\tests\unit_bind_sz.lm2" "unit_bind_sz" 0 "m"
$szg = [System.IO.File]::ReadAllText((Join-Path (Get-Location) (Join-Path $out "unit_bind_sz.lm1")))
if ($szg.IndexOf("size_t: l2_q0") -ge 0) { throw "aliased size_t must not emit a separate l2_q value" }
if ($szg.IndexOf("lmx_size_value") -ge 0) { throw "aliased size_t must not load graph over the incoming argument" }
$dSz = Invoke-SpliceDrive "unit_bind_sz" @"
        @: Lmx f 0
        c.printf("%zu\n", l2_m0(unit, 9U))
        f: lmx_branch_child(unit, 0U)
        c.printf("%zu\n", lmx_size_value(f\data))
        return: 0
    end: main
end: external
"@
if ($dSz -ne "3`n3`n") { throw "size_t same-name bind got $dSz" }

Invoke-Negative "l2src\tests\unit_bind_ifdecl.lm2" "unit_bind_ifdecl" "unsupported own declaration"

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
    if ($text.IndexOf("l2_hash_eq_forced") -ge 0) { throw "views must not ship l2_hash_eq_forced in production helper" }
    if ($text.IndexOf("l2_immut_query_fill") -lt 0) { throw "views missing L2 make helper fill" }
    if ($text.IndexOf("l2_hash_compare_q") -lt 0) { throw "views missing production compare seam" }
    if ($text.IndexOf("const: @(L2ImmutQuery") -lt 0) { throw "views missing const L2ImmutQuery* formal" }
    if ($text.IndexOf("@: L2ImmutQuery") -lt 0) { throw "views missing mutable L2ImmutQuery* formal" }
    if ($text.IndexOf("l2src/l2_immut_query.h") -lt 0) { throw "views missing L2ImmutQuery header include" }
    if ($text.IndexOf("l2_hash_bind") -ge 0) { throw "views must not auto-bind a mutable payload" }
    if ($text.IndexOf("l1src/p0.lm1.h") -lt 0) { throw "views missing p0.lm1.h adapter include" }
    if ($text.IndexOf("l1src/p0.h`"") -ge 0 -or $text.IndexOf("l1src/p0.h>") -ge 0) { throw "views must not include leftover p0.h" }
    if ($text.IndexOf("const-pointee") -lt 0) { throw "views intern comment must distinguish const pointee" }
    $m2 = [regex]::Match($text, 'fn: l2_m2[\s\S]*?end: l2_m2').Value
    $m3 = [regex]::Match($text, 'fn: l2_m3[\s\S]*?end: l2_m3').Value
    if ($m2.IndexOf("l2_immut_query_fill") -lt 0) { throw "make method must call l2_immut_query_fill" }
    if ($m3.IndexOf("l2_hash_compare_q") -lt 0) { throw "equals_query must call production l2_hash_compare_q" }
    $tail = "        return: 0`n    end: main`nend: external"
    $pos = $text.LastIndexOf($tail)
    if ($pos -lt 0) { throw "views L1 missing generated main return" }
    $drive = @"
        @: LmP0Text t 0
        @: LmP0Text atom 0
        @: LmP0Text pay 0
        @: L2ImmutQuery q 0
        @: Lmx f 0
        c.array: [4]: char nbuf
        c.array: [6]: char mut
        t: (cast: (@: LmP0Text) c.malloc(c.sizeof(c.LmP0Text)))
        atom: (cast: (@: LmP0Text) c.malloc(c.sizeof(c.LmP0Text)))
        pay: (cast: (@: LmP0Text) c.malloc(c.sizeof(c.LmP0Text)))
        q: (cast: (@: L2ImmutQuery) c.malloc(c.sizeof(c.L2ImmutQuery)))
        if: t = 0 || atom = 0 || pay = 0 || q = 0
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
        c.printf("%d\n", l2_m3(unit, t, 0))
        c.printf("%d\n", l2_m2(unit, 0, q))
        c.printf("%d\n", l2_m3(unit, t, q))
        c.printf("%d\n", l2_m2(unit, "fn", q))
        t\data: "fn"
        t\length: 2U
        c.printf("%d\n", l2_m3(unit, t, q))
        t\data: "table"
        t\length: 5U
        c.printf("%d\n", l2_m3(unit, t, q))
        t\data: "fn"
        t\length: 2U
        c.printf("%d\n", l2_m3(unit, t, q))
        c.printf("%d\n", l2_m2(unit, "bbb", q))
        mut[0]: 97
        mut[1]: 97
        mut[2]: 97
        mut[3]: 0
        atom\data: mut
        atom\length: 3U
        c.printf("%d\n", l2_m1(unit, atom, pay))
        c.printf("%d\n", l2_m3(unit, pay, q))
        mut[0]: 98
        mut[1]: 98
        mut[2]: 98
        c.printf("%d\n", l2_m3(unit, pay, q))
        c.printf("%d\n", l2_m0(unit, pay, "bbb"))
        c.printf("%d\n", l2_m0(unit, pay, "aaa"))
        c.printf("%d\n", l2_m2(unit, "aaa", q))
        c.printf("%d\n", l2_m3(unit, pay, q))
        q\hash: l2_fnv1a64("bbb", 3U)
        t\data: "bbb"
        t\length: 3U
        c.printf("%d\n", l2_m3(unit, t, q))
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
    $want = "2,1,2,0,2,0,0,0,1,1,0,1,1,1,0,1,1,0,1,0,0"
    if ($got -ne $want) { throw "views extra own/query/mutation got=$got want=$want full=$b" }
    $drvText = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $drvLm1))
    $fnMake = [regex]::Matches($drvText, 'l2_m2\(unit, "fn"')
    $qCalls = [regex]::Matches($drvText, 'l2_m3\(unit,')
    if ($fnMake.Count -ne 1) { throw "prepared query 'fn' must be made once, got $($fnMake.Count)" }
    if ($qCalls.Count -lt 3) { throw "equals_query must be reused on the prepared query, got $($qCalls.Count)" }
}

Invoke-Views

Invoke-Negative "l2src\tests\unit_void_value.lm2" "unit_void_value" "incompatible entry signature"
Invoke-Negative "l2src\tests\unit_bad_sizeof.lm2" "unit_bad_sizeof" "unknown foreign type"

function Invoke-Heap {
    $refLm1 = Join-Path $out "heap_ref.lm1"
    $refC = Join-Path $out "heap_ref.c"
    $refExe = Join-Path $out "heap_ref.exe"
    $refOut = Join-Path $out "heap_ref.stdout"
    $refSrc = @"
predef: "l1src/parser_text.lm1"
include: "<stdio.h>" "<string.h>"
external:
    fn: main () int
        @: LmP0Text a 0
        @: LmP0Text b 0
        @: char p 0
        @: char q 0
        a: lm_p0_text_view_new_cstr("hello")
        c.printf("%d\n", a != 0)
        c.printf("%zu\n", a\length)
        c.printf("%d\n", c.strcmp(a\data, "hello") = 0)
        c.printf("%d\n", a\data = "hello")
        b: lm_p0_text_from_cstr("hello")
        c.printf("%d\n", b != 0)
        c.printf("%d\n", b = a)
        c.printf("%zu\n", b\length)
        lm_p0_text_view_delete(a)
        lm_p0_text_view_delete(b)
        c.printf("%d\n", c.strcmp("hello", "hello") = 0)
        a: lm_p0_text_view_new_cstr("")
        c.printf("%zu\n", a\length)
        c.printf("%d\n", a\data[0] = 0)
        lm_p0_text_view_delete(a)
        a: lm_p0_text_view_new_cstr(0)
        c.printf("%zu\n", a\length)
        c.printf("%d\n", a\data[0] = 0)
        lm_p0_text_view_delete(a)
        p: lm_p0_copy_bytes("ab", 2U)
        c.printf("%d\n", p != 0)
        c.printf("%d\n", p[0] = 97)
        c.printf("%d\n", p[1] = 98)
        c.printf("%d\n", p[2] = 0)
        q: lm_p0_copy_bytes("ab", 2U)
        c.printf("%d\n", q = p)
        lm_own_delete(p, 0)
        lm_own_delete(q, 0)
        p: lm_p0_copy_bytes(0, 0U)
        c.printf("%d\n", p != 0)
        c.printf("%d\n", p[0] = 0)
        lm_own_delete(p, 0)
        c.array: [4]: char nbuf
        nbuf[0]: 97
        nbuf[1]: 0
        nbuf[2]: 99
        nbuf[3]: 0
        p: lm_p0_copy_bytes(nbuf, 3U)
        c.printf("%d\n", p[0] = 97)
        c.printf("%d\n", p[1] = 0)
        c.printf("%d\n", p[2] = 99)
        c.printf("%d\n", p[3] = 0)
        lm_own_delete(p, 0)
        p: lm_p0_copy_bytes("x", (cast: (size_t) -1))
        c.printf("%d\n", p = 0)
        lm_own_alloc_fails: 1
        a: lm_p0_text_view_new_cstr("z")
        c.printf("%d\n", a = 0)
        lm_own_alloc_fails: 0
        lm_p0_text_view_delete(0)
        return: 0
    end: main
end: external
"@
    [System.IO.File]::WriteAllText((Join-Path (Get-Location) $refLm1), $refSrc.Replace("`r`n","`n"))
    & $l1trans $refLm1 $refC
    if ($LASTEXITCODE -ne 0) { throw "l1trans failed heap_ref" }
    Invoke-Gcc $refC $refExe (Join-Path $log "heap_ref.gcc.log")
    cmd /c "`"$refExe`" > `"$refOut`" 2> `"$(Join-Path $out 'heap_ref.err')`""
    if ($LASTEXITCODE -ne 0) { throw "heap_ref exe failed" }

    Invoke-Leaf "l2src\parser_text_heap.lm2" "parser_text_heap" 0 "lm_p0_copy_bytes"
    $lm1 = Join-Path $out "parser_text_heap.lm1"
    $text = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $lm1)).Replace("`r`n", "`n")
    if ($text -match 'fn: lm_p0_') { throw "heap must mangle method symbols" }
    if ($text.IndexOf("@: LmP0Text l2_s") -lt 0) { throw "heap missing address slot, not own cache" }
    if ($text.IndexOf("lmx_own_load") -ge 0) { throw "heap must not use lmx_own cache API" }
    if ($text.IndexOf("sub: l2_m2") -lt 0) { throw "heap delete must be sub l2_m2" }
    if ($text.IndexOf("c.sizeof(c.LmP0Text)") -lt 0) { throw "heap missing sizeof imported ABI" }
    if ($text.IndexOf("return: @") -ge 0) { throw "heap must return pointer value, not @slot" }
    if ($text.IndexOf("l1src/own.lm1") -lt 0) { throw "heap missing own.lm1 predef" }
    $tail = "        return: 0`n    end: main`nend: external"
    $pos = $text.LastIndexOf($tail)
    if ($pos -lt 0) { throw "heap L1 missing generated main return" }
    $drive = @"
        @: LmP0Text a 0
        @: LmP0Text b 0
        @: char p 0
        @: char q 0
        a: l2_m1(unit, "hello")
        c.printf("%d\n", a != 0)
        c.printf("%zu\n", a\length)
        c.printf("%d\n", c.strcmp(a\data, "hello") = 0)
        c.printf("%d\n", a\data = "hello")
        b: l2_m3(unit, "hello")
        c.printf("%d\n", b != 0)
        c.printf("%d\n", b = a)
        c.printf("%zu\n", b\length)
        l2_m2(unit, a)
        l2_m2(unit, b)
        c.printf("%d\n", c.strcmp("hello", "hello") = 0)
        a: l2_m1(unit, "")
        c.printf("%zu\n", a\length)
        c.printf("%d\n", a\data[0] = 0)
        l2_m2(unit, a)
        a: l2_m1(unit, 0)
        c.printf("%zu\n", a\length)
        c.printf("%d\n", a\data[0] = 0)
        l2_m2(unit, a)
        p: l2_m0(unit, "ab", 2U)
        c.printf("%d\n", p != 0)
        c.printf("%d\n", p[0] = 97)
        c.printf("%d\n", p[1] = 98)
        c.printf("%d\n", p[2] = 0)
        q: l2_m0(unit, "ab", 2U)
        c.printf("%d\n", q = p)
        lm_own_delete(p, 0)
        lm_own_delete(q, 0)
        p: l2_m0(unit, 0, 0U)
        c.printf("%d\n", p != 0)
        c.printf("%d\n", p[0] = 0)
        lm_own_delete(p, 0)
        c.array: [4]: char nbuf
        nbuf[0]: 97
        nbuf[1]: 0
        nbuf[2]: 99
        nbuf[3]: 0
        p: l2_m0(unit, nbuf, 3U)
        c.printf("%d\n", p[0] = 97)
        c.printf("%d\n", p[1] = 0)
        c.printf("%d\n", p[2] = 99)
        c.printf("%d\n", p[3] = 0)
        lm_own_delete(p, 0)
        p: l2_m0(unit, "x", (cast: (size_t) -1))
        c.printf("%d\n", p = 0)
        lm_own_alloc_fails: 1
        a: l2_m1(unit, "z")
        c.printf("%d\n", a = 0)
        lm_own_alloc_fails: 0
        l2_m2(unit, 0)
        return: 0
    end: main
end: external
"@
    $drvLm1 = Join-Path $out "heap_l2_drive.lm1"
    $drvC = Join-Path $out "heap_l2_drive.c"
    $drvExe = Join-Path $out "heap_l2_drive.exe"
    $drvOut = Join-Path $out "heap_l2_drive.stdout"
    [System.IO.File]::WriteAllText((Join-Path (Get-Location) $drvLm1), ($text.Substring(0, $pos) + $drive.Replace("`r`n","`n")))
    & $l1trans $drvLm1 $drvC
    if ($LASTEXITCODE -ne 0) { throw "l1trans failed heap_l2_drive" }
    Invoke-Gcc $drvC $drvExe (Join-Path $log "heap_l2_drive.gcc.log")
    cmd /c "`"$drvExe`" > `"$drvOut`" 2> `"$(Join-Path $out 'heap_l2_drive.err')`""
    if ($LASTEXITCODE -ne 0) { throw "heap_l2_drive exe failed" }
    $a = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $refOut)).Replace("`r`n","`n")
    $b = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $drvOut)).Replace("`r`n","`n")
    if ($a -ne $b) { throw "heap mismatch vs parser_text.lm1`nREF:`n$a`nL2:`n$b" }
}

Invoke-Heap

# Unit 12: hidden char/size_t through-args, closed direct calls.
# Types come from own/formal/call-site supplier, not from literal 65/3U.
# L2-to-L2 calls pass the shared unit pointer (l2_call_node). Distinct
# graph instances are a 21.8 harness second Structure, not a shared cell.

Invoke-Leaf "l2src\tests\unit_own_only.lm2" "unit_own_only" 0 "m"
$oo = [System.IO.File]::ReadAllText((Join-Path (Get-Location) (Join-Path $out "unit_own_only.lm1")))
if ($oo -notmatch 'fn: l2_m0 \(@: Lmx node; int: l2_p0_0\) int') { throw "own-only must keep declared arity, no hidden param" }
if ($oo -match 'char: l2_p0_1') { throw "own-only must not grow a hidden char parameter" }
if ($oo.IndexOf("l2_q0") -lt 0) { throw "own-only missing entry graph load" }

Invoke-Leaf "l2src\tests\unit_dyn_leaf.lm2" "unit_dyn_leaf" 0 "leaf"
$dl = [System.IO.File]::ReadAllText((Join-Path (Get-Location) (Join-Path $out "unit_dyn_leaf.lm1")))
if ($dl -notmatch 'fn: l2_m0 \(@: Lmx node; char: l2_p0_0\) int') { throw "leaf free-use must intern hidden char after declared arity 0" }
if ($dl -notmatch 'fn: l2_m1 \(@: Lmx node; int: l2_p1_0\) int') { throw "outer own-only must not grow hidden" }
if ($dl -notmatch 'l2_m0\(node, l2_q0\)') { throw "outer must pass own cache, not re-read graph at the call" }
$dleaf = Invoke-SpliceDrive "unit_dyn_leaf" @"
        @: Lmx f 0
        f: lmx_branch_child(unit, 0U)
        c.printf("%d\n", l2_m0(unit, 65))
        c.printf("%d\n", lmx_char_value(f\data))
        c.printf("%d\n", l2_m1(unit, 0))
        c.printf("%d\n", lmx_char_value(f\data))
        return: 0
    end: main
end: external
"@
# leaf native 65 -> 1, no source writeback (still 0); outer own publishes 65 then leaf reads it
if ($dleaf -ne "1`n0`n1`n65`n") { throw "dyn leaf hidden read/writeback: $dleaf" }

Invoke-Leaf "l2src\tests\unit_dyn_mid.lm2" "unit_dyn_mid" 0 "leaf"
$dm = [System.IO.File]::ReadAllText((Join-Path (Get-Location) (Join-Path $out "unit_dyn_mid.lm1")))
if ($dm -notmatch 'fn: l2_m1 \(@: Lmx node; char: l2_p1_0\) int') { throw "mid must forward hidden char without source mention" }
if ($dm -notmatch 'l2_m0\(node, l2_p1_0\)') { throw "mid must pass hidden cache to leaf" }
if ($dm -notmatch 'l2_m1\(node, l2_q0\)') { throw "outer must supply own cache through mid" }
$dmid = Invoke-SpliceDrive "unit_dyn_mid" @"
        c.printf("%d\n", l2_m2(unit, 0))
        return: 0
    end: main
end: external
"@
if ($dmid -ne "1`n") { throw "dyn mid chain: $dmid" }

Invoke-Leaf "l2src\tests\unit_dyn_predecl.lm2" "unit_dyn_predecl" 0 "leaf"
$dp = [System.IO.File]::ReadAllText((Join-Path (Get-Location) (Join-Path $out "unit_dyn_predecl.lm1")))
$leafFn = [regex]::Match($dp, 'fn: l2_m0[\s\S]*?end: l2_m0').Value
if ($leafFn -notmatch 'l2_p0_0: 1') { throw "predecl assign before bind must write hidden param" }
if ($leafFn -notmatch 'l2_p0_0: 66') { throw "predecl assign after bind must write the same param" }
$dpre = Invoke-SpliceDrive "unit_dyn_predecl" @"
        @: Lmx unit2 0
        @: Lmx f 0
        @: Lmx g 0
        unit2: (cast: (@: Lmx) c.malloc(c.sizeof(c.Lmx)))
        if: unit2 = 0
            return: 1
        lmx_cell_init(unit2, 0, 0)
        if: lmx_branch_open(unit2, 1U) != 0
            return: 1
        g: lmx_branch_child(unit2, 0U)
        if: g = 0
            return: 1
        g\data: lmx_char_cell(0)
        f: lmx_branch_child(unit, 0U)
        l2_m0(unit2, 65)
        c.printf("%d\n", lmx_char_value(f\data))
        c.printf("%d\n", lmx_char_value(g\data))
        return: 0
    end: main
end: external
"@
# distinct 21.8 nodes: unit stays 0; unit2 publishes 66 from hidden->own bind
if ($dpre -ne "0`n66`n") { throw "predecl distinct-instance publish: $dpre" }

Invoke-Leaf "l2src\tests\unit_dyn_fallback.lm2" "unit_dyn_fallback" 0 "leaf"
$dfb = [System.IO.File]::ReadAllText((Join-Path (Get-Location) (Join-Path $out "unit_dyn_fallback.lm1")))
if ($dfb -notmatch 'fn: l2_m0 \(@: Lmx node; char: l2_p0_0\) int') { throw "fallback leaf must free-use hidden char" }
if ($dfb -notmatch 'fn: l2_m1 \(@: Lmx node\) int') { throw "holder is own-only container, no hidden" }
if ($dfb -notmatch 'fn: l2_m2 \(@: Lmx node; char: l2_p2_0\) int') { throw "typed formal is the char supplier, not holder name-guess" }
if ($dfb -notmatch 'fn: l2_m3 \(@: Lmx node; char: l2_p3_0\) int') { throw "caller without source quote still has hidden ABI" }
if ($dfb -notmatch 'l2_m0\(node, l2_p3_0\)') { throw "caller must pass hidden cache, not reload graph" }
$dfall = Invoke-SpliceDrive "unit_dyn_fallback" @"
        @: Lmx f 0
        f: lmx_branch_child(unit, 0U)
        f\data: lmx_char_cell(65)
        c.printf("%d\n", l2_m3(unit, (cast: (char) lmx_char_value(f\data))))
        c.printf("%d\n", l2_m0(unit, 65))
        return: 0
    end: main
end: external
"@
# adapter loads callee-node child, then native C actual; not silent zero
if ($dfall -ne "1`n1`n") { throw "dyn fallback adapter+native: $dfall" }

Invoke-Leaf "l2src\tests\unit_dyn_early.lm2" "unit_dyn_early" 0 "leaf"
$dearly = Invoke-SpliceDrive "unit_dyn_early" @"
        @: Lmx f 0
        f: lmx_branch_child(unit, 0U)
        c.printf("%d\n", l2_m0(unit, 0, 65))
        c.printf("%d\n", lmx_char_value(f\data))
        c.printf("%d\n", l2_m0(unit, 1, 65))
        c.printf("%d\n", lmx_char_value(f\data))
        return: 0
    end: main
end: external
"@
if ($dearly -ne "0`n0`n0`n66`n") { throw "dyn early-return before bind: $dearly" }

Invoke-Leaf "l2src\tests\unit_dyn_sz.lm2" "unit_dyn_sz" 0 "leaf"
$dsz = [System.IO.File]::ReadAllText((Join-Path (Get-Location) (Join-Path $out "unit_dyn_sz.lm1")))
if ($dsz -notmatch 'fn: l2_m0 \(@: Lmx node; size_t: l2_p0_0\) int') { throw "size_t hidden must come from outer own, not from 3U as a language rule" }
$dszv = Invoke-SpliceDrive "unit_dyn_sz" @"
        c.printf("%d\n", l2_m1(unit, 0))
        return: 0
    end: main
end: external
"@
if ($dszv -ne "1`n") { throw "dyn size_t chain: $dszv" }

Invoke-Negative "l2src\tests\unit_dyn_miss.lm2" "unit_dyn_miss" "unresolved name"
Invoke-Negative "l2src\tests\unit_dyn_type.lm2" "unit_dyn_type" "incompatible entry signature"
Invoke-Leaf "l2src\tests\unit_dyn_cap.lm2" "unit_dyn_cap" 0 "outer"
$dcap = [System.IO.File]::ReadAllText((Join-Path (Get-Location) (Join-Path $out "unit_dyn_cap.lm1")))
if ($dcap -notmatch 'fn: l2_m0 \(@: Lmx node; char: l2_p0_0; char: l2_p0_1; char: l2_p0_2; char: l2_p0_3; char: l2_p0_4\) int') {
    throw "unit_dyn_cap leaf must intern five hidden char params"
}
$dcapv = Invoke-SpliceDrive "unit_dyn_cap" @"
        c.printf("%d\n", l2_m1(unit, 1))
        return: 0
    end: main
end: external
"@
if ($dcapv -ne "5`n") { throw "five hidden through-args: $dcapv" }

Invoke-Leaf "l2src\tests\unit_dyn_bool.lm2" "unit_dyn_bool" 0 "m"
$dbool = [System.IO.File]::ReadAllText((Join-Path (Get-Location) (Join-Path $out "unit_dyn_bool.lm1")))
if ($dbool -match '&& l2_m' -or $dbool -match '\|\| l2_m') { throw "hidden &&/|| must not inline a method call into C &&/||" }
if ($dbool -notmatch 'l2_m0\(node, l2_q0\)' -and $dbool -notmatch 'l2_m0\(node, l2_p') { throw "executed hidden &&/|| call must pass full ABI" }
if ($dbool -notmatch 'if: l2_t') { throw "hidden &&/|| must use guarded if so skipped RHS does not prep" }
$dbv = Invoke-SpliceDrive "unit_dyn_bool" @"
        @: Lmx f 0
        f: lmx_branch_child(unit, 0U)
        c.printf("%d\n", l2_m3(unit, 0))
        c.printf("%d\n", lmx_char_value(f\data))
        c.printf("%d\n", l2_m3(unit, 1))
        c.printf("%d\n", lmx_char_value(f\data))
        c.printf("%d\n", l2_m3(unit, 2))
        c.printf("%d\n", lmx_char_value(f\data))
        c.printf("%d\n", l2_m3(unit, 3))
        c.printf("%d\n", lmx_char_value(f\data))
        c.printf("%d\n", l2_m3(unit, 4))
        c.printf("%d\n", lmx_char_value(f\data))
        c.printf("%d\n", l2_m3(unit, 5))
        c.printf("%d\n", lmx_char_value(f\data))
        c.printf("%d\n", l2_m3(unit, 6))
        c.printf("%d\n", lmx_char_value(f\data))
        c.printf("%d\n", l2_m3(unit, 7))
        c.printf("%d\n", lmx_char_value(f\data))
        return: 0
    end: main
end: external
"@
# skip, skip, run, run, mid run, mid skip, wrap(leaf) run, wrap skip
if ($dbv -ne "0`n65`n1`n65`n1`n90`n1`n90`n1`n90`n0`n65`n1`n90`n0`n65`n") { throw "dyn bool hidden &&/||: $dbv" }

Invoke-Leaf "l2src\tests\unit_while.lm2" "unit_while" 0 "zero"
$wh = [System.IO.File]::ReadAllText((Join-Path (Get-Location) (Join-Path $out "unit_while.lm1")))
if ($wh -notmatch 'while: l2_t') { throw "unit_while must emit L1 while of a re-evaluated cond temp" }
$dwh = Invoke-SpliceDrive "unit_while" @"
        @: Lmx f 0
        c.printf("%d\n", l2_m1(unit, 0))
        c.printf("%d\n", l2_m2(unit, 0))
        c.printf("%d\n", l2_m3(unit, 3U))
        c.printf("%d\n", l2_m3(unit, 0U))
        c.printf("%d\n", l2_m5(unit, 1))
        c.printf("%d\n", l2_m5(unit, 0))
        return: 0
    end: main
end: external
"@
if ($dwh -ne "0`n1`n1`n1`n3`n0`n") { throw "unit_while zero/once/many/early: $dwh" }
$dhit = Invoke-SpliceDrive "unit_while" @"
        @: Lmx f 0
        l2_m6(unit)
        f: lmx_branch_child(unit, 0U)
        c.printf("%d\n", lmx_char_value(f\data))
        return: 0
    end: main
end: external
"@
if ($dhit -ne "4`n") { throw "unit_while last-false still calls bump: $dhit" }
$dgd = Invoke-SpliceDrive "unit_while" @"
        @: Lmx f 0
        l2_m4(unit, "abc", 3U)
        f: lmx_branch_child(unit, 0U)
        c.printf("%d\n", lmx_char_value(f\data))
        l2_m4(unit, "ab", 2U)
        f: lmx_branch_child(unit, 0U)
        c.printf("%d\n", lmx_char_value(f\data))
        return: 0
    end: main
end: external
"@
# first guarded 3 calls; second adds 2 more on the same unit
if ($dgd -ne "3`n5`n") { throw "unit_while shortcircuit bump vs length: $dgd" }

Invoke-Leaf "l2src\tests\unit_continue.lm2" "unit_continue" 0 "skip_tail"
$ct = [System.IO.File]::ReadAllText((Join-Path (Get-Location) (Join-Path $out "unit_continue.lm1")))
if ($ct -notmatch '(?m)^\s+continue$') { throw "unit_continue L1 must emit continue" }
$dct = Invoke-SpliceDrive "unit_continue" @"
        @: Lmx f 0
        c.printf("%d\n", l2_m1(unit, 1, 3))
        c.printf("%d\n", l2_m2(unit, 1, 3))
        c.printf("%d\n", l2_m3(unit))
        l2_m4(unit)
        f: lmx_branch_child(unit, 0U)
        c.printf("%d\n", lmx_char_value(f\data))
        c.printf("%d\n", l2_m5(unit, 1))
        f: lmx_branch_child(unit, 0U)
        c.printf("%d\n", lmx_char_value(f\data))
        return: 0
    end: main
end: external
"@
# skip_tail 3, nested_if 3, nested_while 4 (inner continue, outer still bumps), until hits 4, early 3 and hits 65
if ($dct -ne "3`n3`n4`n4`n3`n65`n") { throw "unit_continue skip/nested/until/early: $dct" }

function Invoke-PhysicalLine {
    $pred = [System.IO.File]::ReadAllText((Join-Path (Get-Location) "l2src\parser_text_predicates.lm2")).Replace("`r`n", "`n")
    $phys = [System.IO.File]::ReadAllText((Join-Path (Get-Location) "l2src\parser_physical_line.lm2")).Replace("`r`n", "`n")
    $h = ($pred -split "fn: lm_p0_is_horizontal_space")[1]
    $h = "fn: lm_p0_is_horizontal_space" + ($h -split "fn: lm_p0_is_line_break")[0]
    $b = ($pred -split "fn: lm_p0_is_line_break")[1]
    $b = "fn: lm_p0_is_line_break" + ($b -split "fn: lm_p0_is_field_space")[0]
    $ph = ($phys -split "fn: lm_p0_is_horizontal_space")[1]
    $ph = "fn: lm_p0_is_horizontal_space" + ($ph -split "fn: lm_p0_is_line_break")[0]
    $pb = ($phys -split "fn: lm_p0_is_line_break")[1]
    $pb = "fn: lm_p0_is_line_break" + ($pb -split "fn: lm_p0_index_is_line_start")[0]
    if ($h.Trim() -ne $ph.Trim()) { throw "physical_line is_horizontal_space must match parser_text_predicates.lm2" }
    if ($b.Trim() -ne $pb.Trim()) { throw "physical_line is_line_break must match parser_text_predicates.lm2" }
    $wref = [System.IO.File]::ReadAllText((Join-Path (Get-Location) "l2src\parser_text_line_break.lm2")).Replace("`r`n", "`n")
    $wref = ($wref -split "fn: lm_p0_line_break_width_at")[1]
    $wref = "fn: lm_p0_line_break_width_at" + ($wref -split "fn: main")[0]
    $wgot = ($phys -split "fn: lm_p0_line_break_width_at")[1]
    $wgot = "fn: lm_p0_line_break_width_at" + ($wgot -split "fn: lm_p0_count_line_breaks")[0]
    if ($wref.Trim() -ne $wgot.Trim()) { throw "physical_line line_break_width_at must match parser_text_line_break.lm2" }

    $refLm1 = Join-Path $out "pline_ref.lm1"
    $refC = Join-Path $out "pline_ref.c"
    $refExe = Join-Path $out "pline_ref.exe"
    $refOut = Join-Path $out "pline_ref.stdout"
    $refSrc = @"
predef: "l1src/parser.lm1"
include: "<stdio.h>"
external:
    fn: main () int
        c.printf("%d\n", lm_p0_index_is_line_start("x", 0U))
        c.printf("%d\n", lm_p0_index_is_line_start("x\ny", 0U))
        c.printf("%d\n", lm_p0_index_is_line_start("x\ny", 1U))
        c.printf("%d\n", lm_p0_index_is_line_start("x\ny", 2U))
        c.printf("%d\n", lm_p0_index_is_line_start("x\ry", 2U))
        c.printf("%zu\n", lm_p0_find_physical_line_end("", 0U, 0U))
        c.printf("%zu\n", lm_p0_find_physical_line_end("abc", 3U, 0U))
        c.printf("%zu\n", lm_p0_find_physical_line_end("abc", 3U, 3U))
        c.printf("%zu\n", lm_p0_find_physical_line_end("abc", 3U, 4U))
        c.printf("%zu\n", lm_p0_find_physical_line_end("ab\ncd", 5U, 0U))
        c.printf("%zu\n", lm_p0_find_physical_line_end("ab\ncd", 5U, 3U))
        c.printf("%zu\n", lm_p0_find_physical_line_end("ab\rcd", 5U, 0U))
        c.printf("%zu\n", lm_p0_find_physical_line_end("ab\r\ncd", 6U, 0U))
        c.printf("%zu\n", lm_p0_find_physical_line_end("a\0b\n", 4U, 0U))
        c.printf("%zu\n", lm_p0_find_physical_line_end("ab\ncd", 5U, 2U))
        c.printf("%d\n", lm_p0_line_rest_is_horizontal_space("", 0U, 0U))
        c.printf("%d\n", lm_p0_line_rest_is_horizontal_space("   ", 0U, 3U))
        c.printf("%d\n", lm_p0_line_rest_is_horizontal_space(" \t ", 0U, 3U))
        c.printf("%d\n", lm_p0_line_rest_is_horizontal_space(" a", 0U, 2U))
        c.printf("%d\n", lm_p0_line_rest_is_horizontal_space("x  ", 1U, 3U))
        c.printf("%d\n", lm_p0_line_rest_is_horizontal_space("x", 1U, 1U))
        c.printf("%zu\n", lm_p0_count_line_breaks("", 0U, 0U))
        c.printf("%zu\n", lm_p0_count_line_breaks("abc", 0U, 3U))
        c.printf("%zu\n", lm_p0_count_line_breaks("abc", 3U, 3U))
        c.printf("%zu\n", lm_p0_count_line_breaks("abc", 4U, 3U))
        c.printf("%zu\n", lm_p0_count_line_breaks("a\nb", 0U, 3U))
        c.printf("%zu\n", lm_p0_count_line_breaks("a\rb", 0U, 3U))
        c.printf("%zu\n", lm_p0_count_line_breaks("a\r\nb", 0U, 4U))
        c.printf("%zu\n", lm_p0_count_line_breaks("a\n\nb", 0U, 4U))
        c.printf("%zu\n", lm_p0_count_line_breaks("a\0b\n", 0U, 4U))
        c.printf("%zu\n", lm_p0_count_line_breaks("ab\ncd\n", 2U, 6U))
        return: 0
    end: main
end: external
"@
    [System.IO.File]::WriteAllText((Join-Path (Get-Location) $refLm1), $refSrc.Replace("`r`n","`n"))
    & $l1trans $refLm1 $refC
    if ($LASTEXITCODE -ne 0) { throw "l1trans failed pline_ref" }
    Invoke-Gcc $refC $refExe (Join-Path $log "pline_ref.gcc.log")
    cmd /c "`"$refExe`" > `"$refOut`" 2> `"$(Join-Path $out 'pline_ref.err')`""
    if ($LASTEXITCODE -ne 0) { throw "pline_ref exe failed" }

    Invoke-Leaf "l2src\parser_physical_line.lm2" "parser_physical_line" 0 "lm_p0_index_is_line_start"
    $lm1 = Join-Path $out "parser_physical_line.lm1"
    $text = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $lm1)).Replace("`r`n", "`n")
    if ($text -match 'fn: lm_p0_') { throw "physical_line must mangle method symbols" }
    if ($text -notmatch 'while: l2_t') { throw "physical_line must re-evaluate while cond each check" }
    if ($text -notmatch '(?m)^\s+continue$') { throw "count_line_breaks must emit continue" }
    $tail = "        return: 0`n    end: main`nend: external"
    $pos = $text.LastIndexOf($tail)
    if ($pos -lt 0) { throw "physical_line L1 missing generated main return" }
    $drive = @"
        c.printf("%d\n", l2_m2(unit, "x", 0U))
        c.printf("%d\n", l2_m2(unit, "x\ny", 0U))
        c.printf("%d\n", l2_m2(unit, "x\ny", 1U))
        c.printf("%d\n", l2_m2(unit, "x\ny", 2U))
        c.printf("%d\n", l2_m2(unit, "x\ry", 2U))
        c.printf("%zu\n", l2_m3(unit, "", 0U, 0U))
        c.printf("%zu\n", l2_m3(unit, "abc", 3U, 0U))
        c.printf("%zu\n", l2_m3(unit, "abc", 3U, 3U))
        c.printf("%zu\n", l2_m3(unit, "abc", 3U, 4U))
        c.printf("%zu\n", l2_m3(unit, "ab\ncd", 5U, 0U))
        c.printf("%zu\n", l2_m3(unit, "ab\ncd", 5U, 3U))
        c.printf("%zu\n", l2_m3(unit, "ab\rcd", 5U, 0U))
        c.printf("%zu\n", l2_m3(unit, "ab\r\ncd", 6U, 0U))
        c.printf("%zu\n", l2_m3(unit, "a\0b\n", 4U, 0U))
        c.printf("%zu\n", l2_m3(unit, "ab\ncd", 5U, 2U))
        c.printf("%d\n", l2_m4(unit, "", 0U, 0U))
        c.printf("%d\n", l2_m4(unit, "   ", 0U, 3U))
        c.printf("%d\n", l2_m4(unit, " \t ", 0U, 3U))
        c.printf("%d\n", l2_m4(unit, " a", 0U, 2U))
        c.printf("%d\n", l2_m4(unit, "x  ", 1U, 3U))
        c.printf("%d\n", l2_m4(unit, "x", 1U, 1U))
        c.printf("%zu\n", l2_m6(unit, "", 0U, 0U))
        c.printf("%zu\n", l2_m6(unit, "abc", 0U, 3U))
        c.printf("%zu\n", l2_m6(unit, "abc", 3U, 3U))
        c.printf("%zu\n", l2_m6(unit, "abc", 4U, 3U))
        c.printf("%zu\n", l2_m6(unit, "a\nb", 0U, 3U))
        c.printf("%zu\n", l2_m6(unit, "a\rb", 0U, 3U))
        c.printf("%zu\n", l2_m6(unit, "a\r\nb", 0U, 4U))
        c.printf("%zu\n", l2_m6(unit, "a\n\nb", 0U, 4U))
        c.printf("%zu\n", l2_m6(unit, "a\0b\n", 0U, 4U))
        c.printf("%zu\n", l2_m6(unit, "ab\ncd\n", 2U, 6U))
        return: 0
    end: main
end: external
"@
    $drvLm1 = Join-Path $out "pline_l2_drive.lm1"
    $drvC = Join-Path $out "pline_l2_drive.c"
    $drvExe = Join-Path $out "pline_l2_drive.exe"
    $drvOut = Join-Path $out "pline_l2_drive.stdout"
    [System.IO.File]::WriteAllText((Join-Path (Get-Location) $drvLm1), ($text.Substring(0, $pos) + $drive.Replace("`r`n","`n")))
    & $l1trans $drvLm1 $drvC
    if ($LASTEXITCODE -ne 0) { throw "l1trans failed pline_l2_drive" }
    Invoke-Gcc $drvC $drvExe (Join-Path $log "pline_l2_drive.gcc.log")
    cmd /c "`"$drvExe`" > `"$drvOut`" 2> `"$(Join-Path $out 'pline_l2_drive.err')`""
    if ($LASTEXITCODE -ne 0) { throw "pline_l2_drive exe failed" }
    $a = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $refOut)).Replace("`r`n","`n")
    $b = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $drvOut)).Replace("`r`n","`n")
    if ($a -ne $b) { throw "physical_line mismatch vs parser.lm1`nREF:`n$a`nL2:`n$b" }
}

Invoke-PhysicalLine

function Invoke-PythonString {
    $startsRef = [System.IO.File]::ReadAllText((Join-Path (Get-Location) "l2src\parser_text_starts_python.lm2")).Replace("`r`n", "`n")
    $py = [System.IO.File]::ReadAllText((Join-Path (Get-Location) "l2src\parser_python_string.lm2")).Replace("`r`n", "`n")
    $sr = ($startsRef -split "fn: lm_p0_starts_python_string")[1]
    $sr = "fn: lm_p0_starts_python_string" + ($sr -split "fn: main")[0]
    $sg = ($py -split "fn: lm_p0_starts_python_string")[1]
    $sg = "fn: lm_p0_starts_python_string" + ($sg -split "fn: lm_p0_find_python_string_end")[0]
    if ($sr.Trim() -ne $sg.Trim()) { throw "python_string starts_python_string must match parser_text_starts_python.lm2" }

    $cases = @'
        size_t: cell
        int: r
        cell: 99U
        r: lm_p0_find_python_string_end("", 0U, 0U, @ cell)
        c.printf("%d %zu\n", r, cell)
        cell: 99U
        r: lm_p0_find_python_string_end("x", 1U, 0U, @ cell)
        c.printf("%d %zu\n", r, cell)
        cell: 99U
        r: lm_p0_find_python_string_end("''", 2U, 0U, @ cell)
        c.printf("%d %zu\n", r, cell)
        cell: 99U
        r: lm_p0_find_python_string_end("\"abc\"", 5U, 0U, @ cell)
        c.printf("%d %zu\n", r, cell)
        cell: 99U
        r: lm_p0_find_python_string_end("\"\"\"abc\"\"\"", 9U, 0U, @ cell)
        c.printf("%d %zu\n", r, cell)
        cell: 99U
        r: lm_p0_find_python_string_end("'''abc'''", 9U, 0U, @ cell)
        c.printf("%d %zu\n", r, cell)
        cell: 99U
        r: lm_p0_find_python_string_end("\"\"\"abc", 6U, 0U, @ cell)
        c.printf("%d %zu\n", r, cell)
        cell: 99U
        r: lm_p0_find_python_string_end("\"\"\"a\"b\"\"\"", 9U, 0U, @ cell)
        c.printf("%d %zu\n", r, cell)
        cell: 99U
        r: lm_p0_find_python_string_end("\"\"\"a\"\"b\"\"\"", 10U, 0U, @ cell)
        c.printf("%d %zu\n", r, cell)
        cell: 99U
        r: lm_p0_find_python_string_end("\"\"\"a\"\"\"\"", 8U, 0U, @ cell)
        c.printf("%d %zu\n", r, cell)
        cell: 99U
        r: lm_p0_find_python_string_end("\"\"\"", 3U, 0U, @ cell)
        c.printf("%d %zu\n", r, cell)
        cell: 99U
        r: lm_p0_find_python_string_end("\"\"\"\"\"\"", 6U, 0U, @ cell)
        c.printf("%d %zu\n", r, cell)
        cell: 99U
        r: lm_p0_find_python_string_end("\"\"\"\"\"\"\"", 7U, 0U, @ cell)
        c.printf("%d %zu\n", r, cell)
        cell: 99U
        r: lm_p0_find_python_string_end("xx\"\"\"abc\"\"\"", 11U, 2U, @ cell)
        c.printf("%d %zu\n", r, cell)
        cell: 99U
        r: lm_p0_find_python_string_end("\"\"\"a\0b\"\"\"", 9U, 0U, @ cell)
        c.printf("%d %zu\n", r, cell)
'@

    $refLm1 = Join-Path $out "pystr_ref.lm1"
    $refC = Join-Path $out "pystr_ref.c"
    $refExe = Join-Path $out "pystr_ref.exe"
    $refOut = Join-Path $out "pystr_ref.stdout"
    $refSrc = @"
predef: "l1src/parser.lm1"
include: "<stdio.h>"
external:
    fn: main () int
$cases
        return: 0
    end: main
end: external
"@
    [System.IO.File]::WriteAllText((Join-Path (Get-Location) $refLm1), $refSrc.Replace("`r`n","`n"))
    & $l1trans $refLm1 $refC
    if ($LASTEXITCODE -ne 0) { throw "l1trans failed pystr_ref" }
    Invoke-Gcc $refC $refExe (Join-Path $log "pystr_ref.gcc.log")
    cmd /c "`"$refExe`" > `"$refOut`" 2> `"$(Join-Path $out 'pystr_ref.err')`""
    if ($LASTEXITCODE -ne 0) { throw "pystr_ref exe failed" }

    Invoke-Leaf "l2src\parser_python_string.lm2" "parser_python_string" 0 "lm_p0_find_python_string_end"
    $lm1 = Join-Path $out "parser_python_string.lm1"
    $text = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $lm1)).Replace("`r`n", "`n")
    if ($text -match 'fn: lm_p0_') { throw "python_string must mangle method symbols" }
    if ($text -notmatch 'l2_p\d+_\d+\[0\]:') { throw "find must store through size_t* [0]" }
    if ($text -notmatch '@: size_t l2_p') { throw "find must emit @: size_t formal" }
    if ($text -notmatch '(?m)^\s+continue$') { throw "find must emit continue" }
    $cpath = Join-Path $out "parser_python_string.c"
    $ctext = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $cpath)).Replace("`r`n", "`n")
    if ($ctext -notmatch '&&') { throw "inner while cond must keep C && short-circuit" }
    if ($ctext -match 'text\[i \+ run_length\]' -and $ctext -notmatch '&&') {
        throw "indexed inner load without &&"
    }

    $l2cases = $cases.Replace("lm_p0_find_python_string_end(", "l2_m1(unit, ")
    $tail = "        return: 0`n    end: main`nend: external"
    $pos = $text.LastIndexOf($tail)
    if ($pos -lt 0) { throw "python_string L1 missing generated main return" }
    $drive = @"
$l2cases
        return: 0
    end: main
end: external
"@
    $drvLm1 = Join-Path $out "pystr_l2_drive.lm1"
    $drvC = Join-Path $out "pystr_l2_drive.c"
    $drvExe = Join-Path $out "pystr_l2_drive.exe"
    $drvOut = Join-Path $out "pystr_l2_drive.stdout"
    [System.IO.File]::WriteAllText((Join-Path (Get-Location) $drvLm1), ($text.Substring(0, $pos) + $drive.Replace("`r`n","`n")))
    & $l1trans $drvLm1 $drvC
    if ($LASTEXITCODE -ne 0) { throw "l1trans failed pystr_l2_drive" }
    Invoke-Gcc $drvC $drvExe (Join-Path $log "pystr_l2_drive.gcc.log")
    cmd /c "`"$drvExe`" > `"$drvOut`" 2> `"$(Join-Path $out 'pystr_l2_drive.err')`""
    if ($LASTEXITCODE -ne 0) { throw "pystr_l2_drive exe failed" }
    $a = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $refOut)).Replace("`r`n","`n")
    $b = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $drvOut)).Replace("`r`n","`n")
    if ($a -ne $b) { throw "python_string find mismatch vs parser.lm1`nREF:`n$a`nL2:`n$b" }

    $fwdcases = $cases.Replace("lm_p0_find_python_string_end(", "l2_m2(unit, ")
    $fwdDrive = @"
$fwdcases
        return: 0
    end: main
end: external
"@
    $fwdLm1 = Join-Path $out "pystr_fwd_drive.lm1"
    $fwdC = Join-Path $out "pystr_fwd_drive.c"
    $fwdExe = Join-Path $out "pystr_fwd_drive.exe"
    $fwdOut = Join-Path $out "pystr_fwd_drive.stdout"
    [System.IO.File]::WriteAllText((Join-Path (Get-Location) $fwdLm1), ($text.Substring(0, $pos) + $fwdDrive.Replace("`r`n","`n")))
    & $l1trans $fwdLm1 $fwdC
    if ($LASTEXITCODE -ne 0) { throw "l1trans failed pystr_fwd_drive" }
    Invoke-Gcc $fwdC $fwdExe (Join-Path $log "pystr_fwd_drive.gcc.log")
    cmd /c "`"$fwdExe`" > `"$fwdOut`" 2> `"$(Join-Path $out 'pystr_fwd_drive.err')`""
    if ($LASTEXITCODE -ne 0) { throw "pystr_fwd_drive exe failed" }
    $c = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $fwdOut)).Replace("`r`n","`n")
    if ($b -ne $c) { throw "python_string fwd must match find`nFIND:`n$b`nFWD:`n$c" }

    $drvCtext = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $drvC)).Replace("`r`n", "`n")
    if ($drvCtext -notmatch '&&') { throw "pystr_l2_drive.c missing && short-circuit" }
}

Invoke-PythonString

Invoke-Leaf "l2src\tests\unit_paren_prec.lm2" "unit_paren_prec" 0 "grouped"
$dpp = Invoke-SpliceDrive "unit_paren_prec" @"
        c.printf("%d\n", l2_m0(unit, 2, 3, 4))
        c.printf("%d\n", l2_m1(unit, 2, 3, 4))
        return: 0
    end: main
end: external
"@
if ($dpp -ne "20`n14`n") { throw "paren prec grouped vs raw: $dpp" }

Invoke-Leaf "l2src\tests\unit_paren_sub.lm2" "unit_paren_sub" 0 "left"
$dps = Invoke-SpliceDrive "unit_paren_sub" @"
        c.printf("%d\n", l2_m0(unit, 10, 3, 2))
        c.printf("%d\n", l2_m1(unit, 10, 3, 2))
        return: 0
    end: main
end: external
"@
if ($dps -ne "5`n9`n") { throw "paren sub left vs right: $dps" }

Invoke-Leaf "l2src\tests\unit_paren_sib.lm2" "unit_paren_sib" 0 "sib"
$dsi = Invoke-SpliceDrive "unit_paren_sib" @"
        c.printf("%d\n", l2_m0(unit, 2, 3))
        return: 0
    end: main
end: external
"@
if ($dsi -ne "12`n") { throw "paren siblings: $dsi" }

Invoke-Leaf "l2src\tests\unit_paren_nest.lm2" "unit_paren_nest" 0 "nest"
$dn = Invoke-SpliceDrive "unit_paren_nest" @"
        c.printf("%d\n", l2_m0(unit, 4))
        return: 0
    end: main
end: external
"@
if ($dn -ne "13`n") { throw "paren nested: $dn" }

Invoke-Leaf "l2src\tests\unit_paren_rel.lm2" "unit_paren_rel" 0 "rel"
$dr = Invoke-SpliceDrive "unit_paren_rel" @"
        c.printf("%d\n", l2_m0(unit, 2, 4))
        c.printf("%d\n", l2_m0(unit, 3, 4))
        return: 0
    end: main
end: external
"@
if ($dr -ne "1`n0`n") { throw "paren relational: $dr" }

Invoke-Leaf "l2src\tests\unit_paren_and.lm2" "unit_paren_and" 0 "gated"
$da0 = Invoke-SpliceDrive "unit_paren_and" @"
        @: Lmx f 0
        l2_m1(unit, 5, 3)
        f: lmx_branch_child(unit, 0U)
        c.printf("%d\n", lmx_char_value(f\data))
        l2_m1(unit, 0, 3)
        f: lmx_branch_child(unit, 0U)
        c.printf("%d\n", lmx_char_value(f\data))
        return: 0
    end: main
end: external
"@
if ($da0 -ne "0`n2`n") { throw "paren shortcircuit bump: $da0" }

Invoke-Leaf "l2src\tests\unit_paren_call.lm2" "unit_paren_call" 0 "order"
$dc = Invoke-SpliceDrive "unit_paren_call" @"
        c.printf("%d\n", l2_m2(unit))
        return: 0
    end: main
end: external
"@
if ($dc -ne "12`n") { throw "paren call actual order: $dc" }
$oct = [System.IO.File]::ReadAllText((Join-Path (Get-Location) (Join-Path $out "unit_paren_call.lm1")))
if ($oct -notmatch 'l2_q\d+_dirty') { throw "paren call missing dirty checkpoint" }

Invoke-Negative "l2src\tests\unit_paren_long.lm2" "unit_paren_long" "expression too long"

Invoke-Leaf "l2src\tests\unit_arity5.lm2" "unit_arity5" 0 "sum5"
$da5 = Invoke-SpliceDrive "unit_arity5" @"
        c.printf("%d\n", l2_m0(unit, 1, 2, 3, 4, 5))
        c.printf("%d\n", l2_m0(unit, 5, 4, 3, 2, 1))
        c.printf("%d\n", l2_m1(unit, 1, 2, 3, 4, 5))
        return: 0
    end: main
end: external
"@
if ($da5 -ne "15`n15`n15`n") { throw "arity5 sum: $da5" }
$c5 = Get-LeafContract "unit_arity5"
if ($c5.Formals -ne "a|b") { throw "arity5 intern f0/f1 $($c5.Formals)" }
$t5 = [System.IO.File]::ReadAllText((Join-Path (Get-Location) (Join-Path $out "unit_arity5.lm1")))
if ($t5 -notmatch 'l2_p0_4') { throw "arity5 missing fifth formal" }
if ($t5 -notmatch 'l2_p1_4') { throw "arity5b missing fifth formal x" }
if ($t5.IndexOf("l2_p0_4") -lt 0) { throw "arity5 intern/emit fifth name" }
$sigs5 = [regex]::Matches($t5, 'rec\\sig: (\d+)U') | ForEach-Object { [int]$_.Groups[1].Value }
if ($sigs5.Count -lt 2) { throw "arity5 expected 2 rec.sig intern ids, got $($sigs5.Count)" }
if ($sigs5[0] -eq $sigs5[1]) { throw "arity5 late name e vs x must intern differently" }

Invoke-Leaf "l2src\tests\unit_arity8.lm2" "unit_arity8" 0 "order8"
$da8 = Invoke-SpliceDrive "unit_arity8" @"
        c.printf("%d\n", l2_m2(unit))
        c.printf("%d\n", l2_m1(unit, 1, 2, 3, 4, 5, 6, 7, 8))
        return: 0
    end: main
end: external
"@
if ($da8 -ne "1793`n1793`n") { throw "arity8 LTR pack: $da8" }
$t8 = [System.IO.File]::ReadAllText((Join-Path (Get-Location) (Join-Path $out "unit_arity8.lm1")))
if ($t8 -notmatch 'l2_p1_7') { throw "arity8 missing eighth formal" }

Invoke-Leaf "l2src\tests\unit_arity9.lm2" "unit_arity9" 0 "nine"
$da9 = Invoke-SpliceDrive "unit_arity9" @"
        c.printf("%d\n", l2_m0(unit, 1, 2, 3, 4, 5, 6, 7, 8, 9))
        c.printf("%d\n", l2_m1(unit, 1, 2, 3, 4, 5, 6, 7, 8, 9))
        return: 0
    end: main
end: external
"@
if ($da9 -ne "45`n10`n") { throw "arity9 sum/late-name: $da9" }
$t9 = [System.IO.File]::ReadAllText((Join-Path (Get-Location) (Join-Path $out "unit_arity9.lm1")))
if ($t9 -notmatch 'l2_p0_8') { throw "arity9 missing ninth formal" }
if ($t9 -notmatch 'l2_p1_8') { throw "arity9b missing ninth formal z" }
$sigs9 = [regex]::Matches($t9, 'rec\\sig: (\d+)U') | ForEach-Object { [int]$_.Groups[1].Value }
if ($sigs9.Count -lt 2) { throw "arity9 expected 2 rec.sig intern ids, got $($sigs9.Count)" }
if ($sigs9[0] -eq $sigs9[1]) { throw "arity9 late name i vs z must intern differently" }

Invoke-Leaf "l2src\tests\unit_arity5p.lm2" "unit_arity5p" 0 "write5"
$dp = Invoke-SpliceDrive "unit_arity5p" @"
        size_t: cell
        int: r
        cell: 99U
        r: l2_m0(unit, 1, 2, 3, 4, @ cell)
        c.printf("%d %zu\n", r, cell)
        cell: 99U
        r: l2_m1(unit, 1, 2, 3, 4, @ cell)
        c.printf("%d %zu\n", r, cell)
        cell: 99U
        l2_m2(unit, 2, 3, 4, 5, @ cell)
        c.printf("%zu\n", cell)
        return: 0
    end: main
end: external
"@
if ($dp -ne "1 10`n1 10`n14`n") { throw "arity5 pointer/sub: $dp" }

Invoke-Leaf "l2src\tests\unit_arity5h.lm2" "unit_arity5h" 0 "outer"
$dh = Invoke-SpliceDrive "unit_arity5h" @"
        c.printf("%d\n", l2_m1(unit))
        return: 0
    end: main
end: external
"@
if ($dh -ne "9`n") { throw "arity5 plus hidden: $dh" }

function New-Arity127Source([string]$path) {
    $names = 0..126 | ForEach-Object { "int: p$_" }
    $plist = [string]::Join("; ", $names)
    $acts = New-Object string[] 127
    for ($i = 0; $i -lt 127; $i++) { $acts[$i] = "0" }
    $acts[0] = "1"
    $acts[63] = "2"
    $acts[126] = "4"
    $call = "sum127(" + [string]::Join(", ", $acts) + ")"
    $src = "fn: sum127 ($plist) int`n    return: p0 + p63 + p126`nend: sum127`nfn: call127 () int`n    return: $call`nend: call127`nfn: main () int`n    return: 0`nend: main`n"
    [System.IO.File]::WriteAllText((Join-Path (Get-Location) $path), $src.Replace("`r`n", "`n"))
}

$ar127 = Join-Path $out "unit_arity127.lm2"
New-Arity127Source $ar127
Invoke-Leaf $ar127 "unit_arity127" 0 "call127"
$t127 = [System.IO.File]::ReadAllText((Join-Path (Get-Location) (Join-Path $out "unit_arity127.lm1")))
if ($t127 -notmatch 'l2_p0_126') { throw "arity127 missing 127th formal" }
if ($t127 -notmatch 'l2_m0\(node, 1') { throw "arity127 missing L2-source call with 127 actuals" }
$d127 = Invoke-SpliceDrive "unit_arity127" @"
        c.printf("%d\n", l2_m1(unit))
        return: 0
    end: main
end: external
"@
if ($d127 -ne "7`n") { throw "arity127 L2 call p0+p63+p126: $d127" }

function Invoke-FailMallocSrc([string]$src, [string]$tag, [int]$maxN) {
    $dir = Join-Path $out "fail_malloc"
    New-Item -ItemType Directory -Force -Path $dir | Out-Null
    $kinds = @{}
    $n = 1
    while ($n -le $maxN) {
        $case = Join-Path $dir ($tag + "_" + $n)
        $lm1 = $case + ".lm1"
        $cpath = $case + ".c"
        $exe = $case + ".exe"
        $err = $case + ".err"
        $alog = $case + ".alloc"
        $marker = "OLD-OUTPUT-MUST-NOT-BECOME-SUCCESS`n"
        [System.IO.File]::WriteAllText((Join-Path (Get-Location) $lm1), $marker)
        [System.IO.File]::WriteAllText((Join-Path (Get-Location) $cpath), $marker)
        [System.IO.File]::WriteAllText((Join-Path (Get-Location) $exe), $marker)
        $env:L2_FAIL_MALLOC = [string]$n
        $env:L2_ALLOC_LOG = [string]$alog
        cmd /c "`"$l2exe`" `"$src`" `"$lm1`" 2> `"$err`""
        $code = $LASTEXITCODE
        Remove-Item Env:L2_FAIL_MALLOC
        Remove-Item Env:L2_ALLOC_LOG
        if ($code -eq 0) {
            $n = $n + 1
            continue
        }
        $etext = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $err))
        if ($etext.IndexOf("out of memory") -lt 0 -and $etext.IndexOf("size overflow") -lt 0) {
            throw "fail-malloc $tag $n missing oom/overflow in $err : $etext"
        }
        $got = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $lm1))
        if ($got -ne $marker) { throw "fail-malloc $tag $n mutated dest lm1" }
        if (-not (Test-Path -LiteralPath $alog)) { throw "fail-malloc $tag $n missing alloc log" }
        $astat = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $alog)).Trim()
        if ($astat -notmatch 'live=(\d+).*fail_kind=(\d+)') { throw "fail-malloc $tag $n bad alloc log: $astat" }
        $live = [int]$Matches[1]
        $kind = [int]$Matches[2]
        if ($live -ne 0) { throw "fail-malloc $tag $n live=$live (partial row not freed) log=$astat" }
        if ($kind -lt 1) { throw "fail-malloc $tag $n fail_kind unset: $astat" }
        $kinds[$kind] = $n
        $n = $n + 1
    }
    return $kinds
}

$kDyn = Invoke-FailMallocSrc "l2src\tests\unit_dyn_cap.lm2" "dyn_cap" 48
$k127 = Invoke-FailMallocSrc (Join-Path $out "unit_arity127.lm2") "arity127" 64
$kOwn = Invoke-FailMallocSrc "l2src\tests\unit_own6.lm2" "own6" 48
$kMeth = Invoke-FailMallocSrc "l2src\tests\unit_nine.lm2" "nine" 48
$kOwnMeth = Invoke-FailMallocSrc "l2src\tests\unit_own_meth.lm2" "own_meth" 64
if (-not $kDyn.ContainsKey(1)) { throw "fail-malloc dyn_cap never hit formals (kind 1); got $($kDyn.Keys -join ',')" }
if (-not $kDyn.ContainsKey(2)) { throw "fail-malloc dyn_cap never hit hidden growth (kind 2); got $($kDyn.Keys -join ',')" }
if (-not $kDyn.ContainsKey(3)) { throw "fail-malloc dyn_cap never hit intern rows (kind 3); got $($kDyn.Keys -join ',')" }
if (-not $kDyn.ContainsKey(4) -and -not $k127.ContainsKey(4)) { throw "fail-malloc never hit call-actual vectors (kind 4)" }
if (-not $kOwn.ContainsKey(5)) { throw "fail-malloc own6 never hit OwnUsed growth (kind 5); got $($kOwn.Keys -join ',')" }
if (-not $kMeth.ContainsKey(6)) { throw "fail-malloc nine never hit method growth (kind 6); got $($kMeth.Keys -join ',')" }
if (-not $kOwnMeth.ContainsKey(6)) { throw "fail-malloc own_meth never hit method growth (kind 6) with own rows; got $($kOwnMeth.Keys -join ',')" }
$ev = Join-Path $out "fail_malloc\summary.txt"
$lines = @("dyn_cap kinds: " + (($kDyn.GetEnumerator() | Sort-Object Name | ForEach-Object { "$($_.Key)=$($_.Value)" }) -join " "))
$lines += "arity127 kinds: " + (($k127.GetEnumerator() | Sort-Object Name | ForEach-Object { "$($_.Key)=$($_.Value)" }) -join " ")
$lines += "own6 kinds: " + (($kOwn.GetEnumerator() | Sort-Object Name | ForEach-Object { "$($_.Key)=$($_.Value)" }) -join " ")
$lines += "nine kinds: " + (($kMeth.GetEnumerator() | Sort-Object Name | ForEach-Object { "$($_.Key)=$($_.Value)" }) -join " ")
$lines += "own_meth kinds: " + (($kOwnMeth.GetEnumerator() | Sort-Object Name | ForEach-Object { "$($_.Key)=$($_.Value)" }) -join " ")
[System.IO.File]::WriteAllLines((Join-Path (Get-Location) $ev), $lines)

function Invoke-VisualColumn {
    $cases = @'
        c.printf("%zu\n", lm_p0_indent_tab_column(0U))
        c.printf("%zu\n", lm_p0_indent_tab_column(1U))
        c.printf("%zu\n", lm_p0_indent_tab_column(7U))
        c.printf("%zu\n", lm_p0_indent_tab_column(8U))
        c.printf("%zu\n", lm_p0_indent_tab_column(9U))
        c.printf("%zu\n", lm_p0_visual_column_between("", 0U, 0U))
        c.printf("%zu\n", lm_p0_visual_column_between("abc", 0U, 3U))
        c.printf("%zu\n", lm_p0_visual_column_between("abc", 3U, 3U))
        c.printf("%zu\n", lm_p0_visual_column_between("abc", 4U, 3U))
        c.printf("%zu\n", lm_p0_visual_column_between("a\nb", 0U, 3U))
        c.printf("%zu\n", lm_p0_visual_column_between("a\rb", 0U, 3U))
        c.printf("%zu\n", lm_p0_visual_column_between("a\r\nb", 0U, 4U))
        c.printf("%zu\n", lm_p0_visual_column_between("a\r", 0U, 2U))
        c.printf("%zu\n", lm_p0_visual_column_between("a\r\n", 0U, 3U))
        c.printf("%zu\n", lm_p0_visual_column_between("\t", 0U, 1U))
        c.printf("%zu\n", lm_p0_visual_column_between("x\t", 0U, 2U))
        c.printf("%zu\n", lm_p0_visual_column_between("xxxxxxx\t", 0U, 8U))
        c.printf("%zu\n", lm_p0_visual_column_between("xxxxxxxx\t", 0U, 9U))
        c.printf("%zu\n", lm_p0_visual_column_between("a\tb\nc", 0U, 5U))
        c.printf("%zu\n", lm_p0_visual_column_between("ab\tcd", 2U, 5U))
        c.printf("%zu\n", lm_p0_visual_column_between("a\0b", 0U, 3U))
'@

    $refLm1 = Join-Path $out "vcol_ref.lm1"
    $refC = Join-Path $out "vcol_ref.c"
    $refExe = Join-Path $out "vcol_ref.exe"
    $refOut = Join-Path $out "vcol_ref.stdout"
    $refSrc = @"
predef: "l1src/parser.lm1"
include: "<stdio.h>"
external:
    fn: main () int
$cases
        return: 0
    end: main
end: external
"@
    [System.IO.File]::WriteAllText((Join-Path (Get-Location) $refLm1), $refSrc.Replace("`r`n","`n"))
    & $l1trans $refLm1 $refC
    if ($LASTEXITCODE -ne 0) { throw "l1trans failed vcol_ref" }
    Invoke-Gcc $refC $refExe (Join-Path $log "vcol_ref.gcc.log")
    cmd /c "`"$refExe`" > `"$refOut`" 2> `"$(Join-Path $out 'vcol_ref.err')`""
    if ($LASTEXITCODE -ne 0) { throw "vcol_ref exe failed" }

    Invoke-Leaf "l2src\parser_visual_column.lm2" "parser_visual_column" 0 "lm_p0_visual_column_between"
    $lm1 = Join-Path $out "parser_visual_column.lm1"
    $text = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $lm1)).Replace("`r`n", "`n")
    if ($text -match 'fn: lm_p0_') { throw "visual_column must mangle method symbols" }
    if ($text -notmatch '(?m)^\s+continue$') { throw "visual_column must emit continue" }
    if ($text -notmatch 'l2_m0\(') { throw "visual_column must call tab helper" }
    if ($text -notmatch 'l2_q\d+_dirty: 1') { throw "column own must dirty after assign" }
    if ($text -notmatch 'lmx_size_store') { throw "column own must checkpoint before tab helper call" }

    $l2cases = $cases.Replace("lm_p0_indent_tab_column(", "l2_m0(unit, ").Replace("lm_p0_visual_column_between(", "l2_m1(unit, ")
    $tail = "        return: 0`n    end: main`nend: external"
    $pos = $text.LastIndexOf($tail)
    if ($pos -lt 0) { throw "visual_column L1 missing generated main return" }
    $drive = @"
$l2cases
        return: 0
    end: main
end: external
"@
    $drvLm1 = Join-Path $out "vcol_l2_drive.lm1"
    $drvC = Join-Path $out "vcol_l2_drive.c"
    $drvExe = Join-Path $out "vcol_l2_drive.exe"
    $drvOut = Join-Path $out "vcol_l2_drive.stdout"
    [System.IO.File]::WriteAllText((Join-Path (Get-Location) $drvLm1), ($text.Substring(0, $pos) + $drive.Replace("`r`n","`n")))
    & $l1trans $drvLm1 $drvC
    if ($LASTEXITCODE -ne 0) { throw "l1trans failed vcol_l2_drive" }
    Invoke-Gcc $drvC $drvExe (Join-Path $log "vcol_l2_drive.gcc.log")
    cmd /c "`"$drvExe`" > `"$drvOut`" 2> `"$(Join-Path $out 'vcol_l2_drive.err')`""
    if ($LASTEXITCODE -ne 0) { throw "vcol_l2_drive exe failed" }
    $a = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $refOut)).Replace("`r`n","`n")
    $b = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $drvOut)).Replace("`r`n","`n")
    if ($a -ne $b) { throw "visual_column mismatch vs parser.lm1`nREF:`n$a`nL2:`n$b" }

    $ctext = [System.IO.File]::ReadAllText((Join-Path (Get-Location) (Join-Path $out "parser_visual_column.c"))).Replace("`r`n", "`n")
    if ($ctext -notmatch '&&') { throw "CRLF p+1 bound guard must stay C &&" }

    Invoke-Leaf "l2src\parser_physical_line.lm2" "parser_physical_line" 0 "lm_p0_index_is_line_start"
    $mtext = [System.IO.File]::ReadAllText((Join-Path (Get-Location) (Join-Path $out "parser_physical_line.lm1"))).Replace("`r`n", "`n")
    if ($mtext -notmatch 'fn: l2_m8') { throw "merged physical_line missing tab helper" }
    if ($mtext -notmatch 'fn: l2_m9') { throw "merged physical_line missing visual_column" }
    $ml2 = $cases.Replace("lm_p0_indent_tab_column(", "l2_m8(unit, ").Replace("lm_p0_visual_column_between(", "l2_m9(unit, ")
    $mpos = $mtext.LastIndexOf($tail)
    if ($mpos -lt 0) { throw "merged physical_line L1 missing main return" }
    $mdrv = Join-Path $out "vcol_merged_drive.lm1"
    $mdrvC = Join-Path $out "vcol_merged_drive.c"
    $mdrvExe = Join-Path $out "vcol_merged_drive.exe"
    $mdrvOut = Join-Path $out "vcol_merged_drive.stdout"
    [System.IO.File]::WriteAllText((Join-Path (Get-Location) $mdrv), ($mtext.Substring(0, $mpos) + ($ml2 + "`n        return: 0`n    end: main`nend: external").Replace("`r`n","`n")))
    & $l1trans $mdrv $mdrvC
    if ($LASTEXITCODE -ne 0) { throw "l1trans failed vcol_merged_drive" }
    Invoke-Gcc $mdrvC $mdrvExe (Join-Path $log "vcol_merged_drive.gcc.log")
    cmd /c "`"$mdrvExe`" > `"$mdrvOut`" 2> `"$(Join-Path $out 'vcol_merged_drive.err')`""
    if ($LASTEXITCODE -ne 0) { throw "vcol_merged_drive exe failed" }
    $mc = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $mdrvOut)).Replace("`r`n","`n")
    if ($a -ne $mc) { throw "merged physical_line visual_column mismatch vs parser.lm1`nREF:`n$a`nL2:`n$mc" }
}

Invoke-VisualColumn

function Invoke-ScanIndent {
    $cases = @'
        size_t: off
        size_t: col
        off: 99U
        col: 99U
        lm_p0_scan_indent_column("", 0U, 0U, @ off, @ col)
        c.printf("%zu %zu\n", off, col)
        off: 99U
        col: 99U
        lm_p0_scan_indent_column("abc", 0U, 3U, @ off, @ col)
        c.printf("%zu %zu\n", off, col)
        off: 99U
        col: 99U
        lm_p0_scan_indent_column("abc", 3U, 3U, @ off, @ col)
        c.printf("%zu %zu\n", off, col)
        off: 99U
        col: 99U
        lm_p0_scan_indent_column("abc", 4U, 3U, @ off, @ col)
        c.printf("%zu %zu\n", off, col)
        off: 99U
        col: 99U
        lm_p0_scan_indent_column("  x", 0U, 3U, @ off, @ col)
        c.printf("%zu %zu\n", off, col)
        off: 99U
        col: 99U
        lm_p0_scan_indent_column("\tx", 0U, 2U, @ off, @ col)
        c.printf("%zu %zu\n", off, col)
        off: 99U
        col: 99U
        lm_p0_scan_indent_column(" \tx", 0U, 3U, @ off, @ col)
        c.printf("%zu %zu\n", off, col)
        off: 99U
        col: 99U
        lm_p0_scan_indent_column("       \ty", 0U, 9U, @ off, @ col)
        c.printf("%zu %zu\n", off, col)
        off: 99U
        col: 99U
        lm_p0_scan_indent_column("        \ty", 0U, 10U, @ off, @ col)
        c.printf("%zu %zu\n", off, col)
        off: 99U
        col: 99U
        lm_p0_scan_indent_column("   ", 0U, 3U, @ off, @ col)
        c.printf("%zu %zu\n", off, col)
        off: 99U
        col: 99U
        lm_p0_scan_indent_column("x  y", 1U, 4U, @ off, @ col)
        c.printf("%zu %zu\n", off, col)
        off: 99U
        col: 99U
        lm_p0_scan_indent_column("\n  x", 0U, 4U, @ off, @ col)
        c.printf("%zu %zu\n", off, col)
        off: 99U
        col: 99U
        lm_p0_scan_indent_column("\r  x", 0U, 4U, @ off, @ col)
        c.printf("%zu %zu\n", off, col)
        off: 99U
        col: 99U
        lm_p0_scan_indent_column(" \0 x", 0U, 4U, @ off, @ col)
        c.printf("%zu %zu\n", off, col)
        off: 99U
        col: 99U
        lm_p0_scan_indent_column(0, 0U, 0U, @ off, @ col)
        c.printf("%zu %zu\n", off, col)
        off: 99U
        col: 99U
        lm_p0_scan_indent_column("\t\t", 0U, 2U, @ off, @ col)
        c.printf("%zu %zu\n", off, col)
'@

    $refLm1 = Join-Path $out "sind_ref.lm1"
    $refC = Join-Path $out "sind_ref.c"
    $refExe = Join-Path $out "sind_ref.exe"
    $refOut = Join-Path $out "sind_ref.stdout"
    $refSrc = @"
predef: "l1src/parser.lm1"
include: "<stdio.h>"
external:
    fn: main () int
$cases
        return: 0
    end: main
end: external
"@
    [System.IO.File]::WriteAllText((Join-Path (Get-Location) $refLm1), $refSrc.Replace("`r`n","`n"))
    & $l1trans $refLm1 $refC
    if ($LASTEXITCODE -ne 0) { throw "l1trans failed sind_ref" }
    Invoke-Gcc $refC $refExe (Join-Path $log "sind_ref.gcc.log")
    cmd /c "`"$refExe`" > `"$refOut`" 2> `"$(Join-Path $out 'sind_ref.err')`""
    if ($LASTEXITCODE -ne 0) { throw "sind_ref exe failed" }

    Invoke-Leaf "l2src\parser_scan_indent.lm2" "parser_scan_indent" 0 "lm_p0_scan_indent_column"
    $lm1 = Join-Path $out "parser_scan_indent.lm1"
    $text = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $lm1)).Replace("`r`n", "`n")
    if ($text -match 'fn: lm_p0_' -or $text -match 'sub: lm_p0_') { throw "scan_indent must mangle method symbols" }
    if ($text -notmatch 'sub: l2_m2') { throw "scan_indent missing mangled sub" }
    if ($text -notmatch '@: size_t l2_p2_3') { throw "scan_indent missing size_t* out_offset" }
    if ($text -notmatch '@: size_t l2_p2_4') { throw "scan_indent missing size_t* out_column" }
    if ($text -notmatch 'l2_p2_3\[0\]:') { throw "scan_indent missing out_offset[0] store" }
    if ($text -notmatch 'l2_p2_4\[0\]:') { throw "scan_indent missing out_column[0] store" }
    if ($text -notmatch '(?m)^\s+continue$') { throw "scan_indent must emit continue" }
    if ($text -notmatch 'l2_m1\(') { throw "scan_indent must call tab helper" }

    $l2cases = $cases.Replace("lm_p0_scan_indent_column(", "l2_m2(unit, ")
    $tail = "        return: 0`n    end: main`nend: external"
    $pos = $text.LastIndexOf($tail)
    if ($pos -lt 0) { throw "scan_indent L1 missing generated main return" }
    $drive = @"
$l2cases
        return: 0
    end: main
end: external
"@
    $drvLm1 = Join-Path $out "sind_l2_drive.lm1"
    $drvC = Join-Path $out "sind_l2_drive.c"
    $drvExe = Join-Path $out "sind_l2_drive.exe"
    $drvOut = Join-Path $out "sind_l2_drive.stdout"
    [System.IO.File]::WriteAllText((Join-Path (Get-Location) $drvLm1), ($text.Substring(0, $pos) + $drive.Replace("`r`n","`n")))
    & $l1trans $drvLm1 $drvC
    if ($LASTEXITCODE -ne 0) { throw "l1trans failed sind_l2_drive" }
    Invoke-Gcc $drvC $drvExe (Join-Path $log "sind_l2_drive.gcc.log")
    cmd /c "`"$drvExe`" > `"$drvOut`" 2> `"$(Join-Path $out 'sind_l2_drive.err')`""
    if ($LASTEXITCODE -ne 0) { throw "sind_l2_drive exe failed" }
    $a = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $refOut)).Replace("`r`n","`n")
    $b = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $drvOut)).Replace("`r`n","`n")
    if ($a -ne $b) { throw "scan_indent mismatch vs parser.lm1`nREF:`n$a`nL2:`n$b" }

    Invoke-Leaf "l2src\parser_physical_line.lm2" "parser_physical_line" 0 "lm_p0_index_is_line_start"
    $mtext = [System.IO.File]::ReadAllText((Join-Path (Get-Location) (Join-Path $out "parser_physical_line.lm1"))).Replace("`r`n", "`n")
    if ($mtext -notmatch 'sub: l2_m10') { throw "merged physical_line missing scan_indent" }
    $ml2 = $cases.Replace("lm_p0_scan_indent_column(", "l2_m10(unit, ")
    $mtail = "        return: 0`n    end: main`nend: external"
    $mpos = $mtext.LastIndexOf($mtail)
    if ($mpos -lt 0) { throw "merged physical_line L1 missing main return for scan" }
    $mdrv = Join-Path $out "sind_merged_drive.lm1"
    $mdrvC = Join-Path $out "sind_merged_drive.c"
    $mdrvExe = Join-Path $out "sind_merged_drive.exe"
    $mdrvOut = Join-Path $out "sind_merged_drive.stdout"
    [System.IO.File]::WriteAllText((Join-Path (Get-Location) $mdrv), ($mtext.Substring(0, $mpos) + ($ml2 + "`n        return: 0`n    end: main`nend: external").Replace("`r`n","`n")))
    & $l1trans $mdrv $mdrvC
    if ($LASTEXITCODE -ne 0) { throw "l1trans failed sind_merged_drive" }
    Invoke-Gcc $mdrvC $mdrvExe (Join-Path $log "sind_merged_drive.gcc.log")
    cmd /c "`"$mdrvExe`" > `"$mdrvOut`" 2> `"$(Join-Path $out 'sind_merged_drive.err')`""
    if ($LASTEXITCODE -ne 0) { throw "sind_merged_drive exe failed" }
    $mc = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $mdrvOut)).Replace("`r`n","`n")
    if ($a -ne $mc) { throw "merged physical_line scan_indent mismatch vs parser.lm1`nREF:`n$a`nL2:`n$mc" }
}

Invoke-ScanIndent

function Invoke-AdvanceLayout {
    $cases = @'
        size_t: off
        size_t: ln
        off: 99U
        ln: 1U
        lm_p0_advance_layout_line("abc", 3U, 0U, 3U, @ off, @ ln)
        c.printf("%zu %zu\n", off, ln)
        off: 99U
        ln: 1U
        lm_p0_advance_layout_line("ab\ncd", 5U, 0U, 2U, @ off, @ ln)
        c.printf("%zu %zu\n", off, ln)
        off: 99U
        ln: 1U
        lm_p0_advance_layout_line("ab\r\ncd", 6U, 0U, 2U, @ off, @ ln)
        c.printf("%zu %zu\n", off, ln)
        off: 99U
        ln: 1U
        lm_p0_advance_layout_line("ab\rcd", 5U, 0U, 2U, @ off, @ ln)
        c.printf("%zu %zu\n", off, ln)
        off: 99U
        ln: 5U
        lm_p0_advance_layout_line("a\nb\nc", 5U, 0U, 5U, @ off, @ ln)
        c.printf("%zu %zu\n", off, ln)
        off: 99U
        ln: 3U
        lm_p0_advance_layout_line("abc", 3U, 1U, 1U, @ off, @ ln)
        c.printf("%zu %zu\n", off, ln)
        off: 99U
        ln: 1U
        lm_p0_advance_layout_line("", 0U, 0U, 0U, @ off, @ ln)
        c.printf("%zu %zu\n", off, ln)
        off: 99U
        ln: 1U
        lm_p0_advance_layout_line("a\0b\n", 4U, 0U, 3U, @ off, @ ln)
        c.printf("%zu %zu\n", off, ln)
        off: 99U
        ln: 2U
        lm_p0_advance_layout_line("ab\ncd\n", 6U, 3U, 5U, @ off, @ ln)
        c.printf("%zu %zu\n", off, ln)
        off: 0U
        ln: 1U
        lm_p0_advance_layout_line("a\nb\n", 4U, 0U, 1U, @ off, @ ln)
        lm_p0_advance_layout_line("a\nb\n", 4U, 2U, 3U, @ off, @ ln)
        c.printf("%zu %zu\n", off, ln)
        off: 99U
        ln: 1U
        lm_p0_advance_layout_line("\r\n", 2U, 0U, 0U, @ off, @ ln)
        c.printf("%zu %zu\n", off, ln)
        off: 99U
        ln: 1U
        lm_p0_advance_layout_line("\r\n", 1U, 0U, 0U, @ off, @ ln)
        c.printf("%zu %zu\n", off, ln)
'@

    $refLm1 = Join-Path $out "adv_ref.lm1"
    $refC = Join-Path $out "adv_ref.c"
    $refExe = Join-Path $out "adv_ref.exe"
    $refOut = Join-Path $out "adv_ref.stdout"
    $refSrc = @"
predef: "l1src/parser.lm1"
include: "<stdio.h>"
external:
    fn: main () int
$cases
        return: 0
    end: main
end: external
"@
    [System.IO.File]::WriteAllText((Join-Path (Get-Location) $refLm1), $refSrc.Replace("`r`n","`n"))
    & $l1trans $refLm1 $refC
    if ($LASTEXITCODE -ne 0) { throw "l1trans failed adv_ref" }
    Invoke-Gcc $refC $refExe (Join-Path $log "adv_ref.gcc.log")
    cmd /c "`"$refExe`" > `"$refOut`" 2> `"$(Join-Path $out 'adv_ref.err')`""
    if ($LASTEXITCODE -ne 0) { throw "adv_ref exe failed" }

    Invoke-Leaf "l2src\parser_physical_line.lm2" "parser_physical_line" 0 "lm_p0_index_is_line_start"
    $lm1 = Join-Path $out "parser_physical_line.lm1"
    $text = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $lm1)).Replace("`r`n", "`n")
    if ($text -match 'fn: lm_p0_' -or $text -match 'sub: lm_p0_') { throw "advance_layout must mangle method symbols" }
    if ($text -notmatch 'sub: l2_m11') { throw "advance_layout missing mangled sub" }
    if ($text -notmatch '@: size_t l2_p11_4') { throw "advance_layout missing size_t* offset" }
    if ($text -notmatch '@: size_t l2_p11_5') { throw "advance_layout missing size_t* line" }
    if ($text -notmatch 'l2_p11_4\[0\]:') { throw "advance_layout missing offset[0] store" }
    if ($text -notmatch 'l2_p11_5\[0\]:') { throw "advance_layout missing line[0] store" }
    if ($text -notmatch 'l2_m6\(') { throw "advance_layout must call count_line_breaks" }
    if ($text -notmatch 'l2_m5\(') { throw "advance_layout must call line_break_width_at" }

    $l2cases = $cases.Replace("lm_p0_advance_layout_line(", "l2_m11(unit, ")
    $tail = "        return: 0`n    end: main`nend: external"
    $pos = $text.LastIndexOf($tail)
    if ($pos -lt 0) { throw "advance_layout L1 missing generated main return" }
    $drive = @"
$l2cases
        return: 0
    end: main
end: external
"@
    $drvLm1 = Join-Path $out "adv_l2_drive.lm1"
    $drvC = Join-Path $out "adv_l2_drive.c"
    $drvExe = Join-Path $out "adv_l2_drive.exe"
    $drvOut = Join-Path $out "adv_l2_drive.stdout"
    [System.IO.File]::WriteAllText((Join-Path (Get-Location) $drvLm1), ($text.Substring(0, $pos) + $drive.Replace("`r`n","`n")))
    & $l1trans $drvLm1 $drvC
    if ($LASTEXITCODE -ne 0) { throw "l1trans failed adv_l2_drive" }
    Invoke-Gcc $drvC $drvExe (Join-Path $log "adv_l2_drive.gcc.log")
    cmd /c "`"$drvExe`" > `"$drvOut`" 2> `"$(Join-Path $out 'adv_l2_drive.err')`""
    if ($LASTEXITCODE -ne 0) { throw "adv_l2_drive exe failed" }
    $a = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $refOut)).Replace("`r`n","`n")
    $b = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $drvOut)).Replace("`r`n","`n")
    if ($a -ne $b) { throw "advance_layout mismatch vs parser.lm1`nREF:`n$a`nL2:`n$b" }

    Invoke-Leaf "l2src\parser_physical_line.lm2" "parser_physical_line" 0 "lm_p0_index_is_line_start"
    $mtext = [System.IO.File]::ReadAllText((Join-Path (Get-Location) (Join-Path $out "parser_physical_line.lm1"))).Replace("`r`n", "`n")
    if ($mtext -notmatch 'sub: l2_m11') { throw "merged physical_line missing advance_layout" }
    $ml2 = $cases.Replace("lm_p0_advance_layout_line(", "l2_m11(unit, ")
    $mtail = "        return: 0`n    end: main`nend: external"
    $mpos = $mtext.LastIndexOf($mtail)
    if ($mpos -lt 0) { throw "merged physical_line L1 missing main return for advance" }
    $mdrv = Join-Path $out "adv_merged_drive.lm1"
    $mdrvC = Join-Path $out "adv_merged_drive.c"
    $mdrvExe = Join-Path $out "adv_merged_drive.exe"
    $mdrvOut = Join-Path $out "adv_merged_drive.stdout"
    [System.IO.File]::WriteAllText((Join-Path (Get-Location) $mdrv), ($mtext.Substring(0, $mpos) + ($ml2 + "`n        return: 0`n    end: main`nend: external").Replace("`r`n","`n")))
    & $l1trans $mdrv $mdrvC
    if ($LASTEXITCODE -ne 0) { throw "l1trans failed adv_merged_drive" }
    Invoke-Gcc $mdrvC $mdrvExe (Join-Path $log "adv_merged_drive.gcc.log")
    cmd /c "`"$mdrvExe`" > `"$mdrvOut`" 2> `"$(Join-Path $out 'adv_merged_drive.err')`""
    if ($LASTEXITCODE -ne 0) { throw "adv_merged_drive exe failed" }
    $mc = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $mdrvOut)).Replace("`r`n","`n")
    if ($a -ne $mc) { throw "merged physical_line advance_layout mismatch vs parser.lm1`nREF:`n$a`nL2:`n$mc" }
}

Invoke-AdvanceLayout


function Invoke-LineStart {
    $cases = @'
        c.printf("%d\n", lm_p0_index_is_line_start(0, 0U))
        c.printf("%d\n", lm_p0_index_is_line_start("", 0U))
        c.printf("%d\n", lm_p0_index_is_line_start("a", 0U))
        c.printf("%d\n", lm_p0_index_is_line_start("a", 1U))
        c.printf("%d\n", lm_p0_index_is_line_start("\na", 1U))
        c.printf("%d\n", lm_p0_index_is_line_start("\ra", 1U))
        c.printf("%d\n", lm_p0_index_is_line_start("\r\na", 1U))
        c.printf("%d\n", lm_p0_index_is_line_start("\r\na", 2U))
        c.printf("%d\n", lm_p0_index_is_line_start("a\nb", 2U))
        c.printf("%d\n", lm_p0_index_is_line_start("ab", 1U))
        c.printf("%d\n", lm_p0_index_is_line_start("a\0b", 1U))
        c.printf("%d\n", lm_p0_index_is_line_start("\n", 1U))
        c.printf("%d\n", lm_p0_line_rest_is_horizontal_space(0, 0U, 0U))
        c.printf("%d\n", lm_p0_line_rest_is_horizontal_space("", 0U, 0U))
        c.printf("%d\n", lm_p0_line_rest_is_horizontal_space("abc", 3U, 3U))
        c.printf("%d\n", lm_p0_line_rest_is_horizontal_space("abc", 4U, 3U))
        c.printf("%d\n", lm_p0_line_rest_is_horizontal_space("   ", 0U, 3U))
        c.printf("%d\n", lm_p0_line_rest_is_horizontal_space("\t\t", 0U, 2U))
        c.printf("%d\n", lm_p0_line_rest_is_horizontal_space("  x", 0U, 3U))
        c.printf("%d\n", lm_p0_line_rest_is_horizontal_space("  x", 0U, 2U))
        c.printf("%d\n", lm_p0_line_rest_is_horizontal_space("x  ", 1U, 3U))
        c.printf("%d\n", lm_p0_line_rest_is_horizontal_space("x y", 1U, 3U))
        c.printf("%d\n", lm_p0_line_rest_is_horizontal_space(" \0 ", 0U, 3U))
        c.printf("%d\n", lm_p0_line_rest_is_horizontal_space("\n  ", 0U, 3U))
'@

    $refLm1 = Join-Path $out "lstart_ref.lm1"
    $refC = Join-Path $out "lstart_ref.c"
    $refExe = Join-Path $out "lstart_ref.exe"
    $refOut = Join-Path $out "lstart_ref.stdout"
    $refSrc = @"
predef: "l1src/parser.lm1"
include: "<stdio.h>"
external:
    fn: main () int
$cases
        return: 0
    end: main
end: external
"@
    [System.IO.File]::WriteAllText((Join-Path (Get-Location) $refLm1), $refSrc.Replace("`r`n","`n"))
    & $l1trans $refLm1 $refC
    if ($LASTEXITCODE -ne 0) { throw "l1trans failed lstart_ref" }
    Invoke-Gcc $refC $refExe (Join-Path $log "lstart_ref.gcc.log")
    cmd /c "`"$refExe`" > `"$refOut`" 2> `"$(Join-Path $out 'lstart_ref.err')`""
    if ($LASTEXITCODE -ne 0) { throw "lstart_ref exe failed" }

    Invoke-Leaf "l2src\parser_line_start.lm2" "parser_line_start" 0 "lm_p0_index_is_line_start"
    $lm1 = Join-Path $out "parser_line_start.lm1"
    $text = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $lm1)).Replace("`r`n", "`n")
    if ($text -match 'fn: lm_p0_') { throw "line_start must mangle method symbols" }
    if ($text -notmatch 'fn: l2_m1') { throw "line_start missing index_is_line_start" }
    if ($text -notmatch 'fn: l2_m2') { throw "line_start missing line_rest_is_horizontal_space" }
    if ($text -notmatch 'if: l2_p1_1 = 0U') { throw "index_is_line_start must return before any index-1 read" }

    $l2cases = $cases.Replace("lm_p0_index_is_line_start(", "l2_m1(unit, ").Replace("lm_p0_line_rest_is_horizontal_space(", "l2_m2(unit, ")
    $tail = "        return: 0`n    end: main`nend: external"
    $pos = $text.LastIndexOf($tail)
    if ($pos -lt 0) { throw "line_start L1 missing generated main return" }
    $drive = @"
$l2cases
        return: 0
    end: main
end: external
"@
    $drvLm1 = Join-Path $out "lstart_l2_drive.lm1"
    $drvC = Join-Path $out "lstart_l2_drive.c"
    $drvExe = Join-Path $out "lstart_l2_drive.exe"
    $drvOut = Join-Path $out "lstart_l2_drive.stdout"
    [System.IO.File]::WriteAllText((Join-Path (Get-Location) $drvLm1), ($text.Substring(0, $pos) + $drive.Replace("`r`n","`n")))
    & $l1trans $drvLm1 $drvC
    if ($LASTEXITCODE -ne 0) { throw "l1trans failed lstart_l2_drive" }
    Invoke-Gcc $drvC $drvExe (Join-Path $log "lstart_l2_drive.gcc.log")
    cmd /c "`"$drvExe`" > `"$drvOut`" 2> `"$(Join-Path $out 'lstart_l2_drive.err')`""
    if ($LASTEXITCODE -ne 0) { throw "lstart_l2_drive exe failed" }
    $a = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $refOut)).Replace("`r`n","`n")
    $b = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $drvOut)).Replace("`r`n","`n")
    if ($a -ne $b) { throw "line_start mismatch vs parser.lm1`nREF:`n$a`nL2:`n$b" }
}

Invoke-LineStart

function Invoke-Position {
    $cases = @'
        size_t: line
        size_t: col
        c.printf("%zu\n", lm_p0_line_break_width_at(0, 0U, 0U))
        c.printf("%zu\n", lm_p0_line_break_width_at("", 0U, 0U))
        c.printf("%zu\n", lm_p0_line_break_width_at("x", 1U, 1U))
        c.printf("%zu\n", lm_p0_line_break_width_at("\n", 1U, 0U))
        c.printf("%zu\n", lm_p0_line_break_width_at("\r", 1U, 0U))
        c.printf("%zu\n", lm_p0_line_break_width_at("\r\n", 2U, 0U))
        c.printf("%zu\n", lm_p0_line_break_width_at("\r\n", 1U, 0U))
        c.printf("%zu\n", lm_p0_line_break_width_at("\r\n", 2U, 1U))
        c.printf("%zu\n", lm_p0_line_break_width_at("a\r\nb", 4U, 1U))
        c.printf("%zu\n", lm_p0_line_break_width_at("\t", 1U, 0U))
        c.printf("%zu\n", lm_p0_line_break_width_at("\0", 1U, 0U))
        line: 99U
        col: 99U
        lm_p0_position_in_slice(0, 0U, 0U, 1U, 1U, @ line, @ col)
        c.printf("%zu %zu\n", line, col)
        line: 99U
        col: 99U
        lm_p0_position_in_slice("", 0U, 0U, 1U, 1U, @ line, @ col)
        c.printf("%zu %zu\n", line, col)
        line: 99U
        col: 99U
        lm_p0_position_in_slice("ab", 2U, 5U, 1U, 1U, @ line, @ col)
        c.printf("%zu %zu\n", line, col)
        line: 99U
        col: 99U
        lm_p0_position_in_slice("a\r\nb", 4U, 0U, 1U, 1U, @ line, @ col)
        c.printf("%zu %zu\n", line, col)
        line: 99U
        col: 99U
        lm_p0_position_in_slice("a\r\nb", 4U, 1U, 1U, 1U, @ line, @ col)
        c.printf("%zu %zu\n", line, col)
        line: 99U
        col: 99U
        lm_p0_position_in_slice("a\r\nb", 4U, 2U, 1U, 1U, @ line, @ col)
        c.printf("%zu %zu\n", line, col)
        line: 99U
        col: 99U
        lm_p0_position_in_slice("a\r\nb", 4U, 3U, 1U, 1U, @ line, @ col)
        c.printf("%zu %zu\n", line, col)
        line: 99U
        col: 99U
        lm_p0_position_in_slice("a\r\nb", 4U, 4U, 1U, 1U, @ line, @ col)
        c.printf("%zu %zu\n", line, col)
        line: 99U
        col: 99U
        lm_p0_position_in_slice("a\nb", 3U, 2U, 1U, 1U, @ line, @ col)
        c.printf("%zu %zu\n", line, col)
        line: 99U
        col: 99U
        lm_p0_position_in_slice("a\rb", 3U, 2U, 1U, 1U, @ line, @ col)
        c.printf("%zu %zu\n", line, col)
        line: 99U
        col: 99U
        lm_p0_position_in_slice("a\r", 2U, 2U, 1U, 1U, @ line, @ col)
        c.printf("%zu %zu\n", line, col)
        line: 99U
        col: 99U
        lm_p0_position_in_slice("a\r\nb", 4U, 3U, 10U, 20U, @ line, @ col)
        c.printf("%zu %zu\n", line, col)
        line: 99U
        col: 99U
        lm_p0_position_in_slice("\t", 1U, 1U, 1U, 1U, @ line, @ col)
        c.printf("%zu %zu\n", line, col)
        line: 99U
        col: 99U
        lm_p0_position_in_slice("a\0b", 3U, 2U, 1U, 1U, @ line, @ col)
        c.printf("%zu %zu\n", line, col)
        line: 99U
        col: 99U
        lm_p0_position_in_slice("\r\n", 1U, 1U, 1U, 1U, @ line, @ col)
        c.printf("%zu %zu\n", line, col)
'@

    $refLm1 = Join-Path $out "pos_ref.lm1"
    $refC = Join-Path $out "pos_ref.c"
    $refExe = Join-Path $out "pos_ref.exe"
    $refOut = Join-Path $out "pos_ref.stdout"
    $refSrc = @"
predef: "l1src/parser.lm1"
include: "<stdio.h>"
external:
    fn: main () int
$cases
        return: 0
    end: main
end: external
"@
    [System.IO.File]::WriteAllText((Join-Path (Get-Location) $refLm1), $refSrc.Replace("`r`n","`n"))
    & $l1trans $refLm1 $refC
    if ($LASTEXITCODE -ne 0) { throw "l1trans failed pos_ref" }
    Invoke-Gcc $refC $refExe (Join-Path $log "pos_ref.gcc.log")
    cmd /c "`"$refExe`" > `"$refOut`" 2> `"$(Join-Path $out 'pos_ref.err')`""
    if ($LASTEXITCODE -ne 0) { throw "pos_ref exe failed" }

    Invoke-Leaf "l2src\parser_position.lm2" "parser_position" 0 "lm_p0_position_in_slice"
    $lm1 = Join-Path $out "parser_position.lm1"
    $text = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $lm1)).Replace("`r`n", "`n")
    if ($text -match 'fn: lm_p0_' -or $text -match 'sub: lm_p0_') { throw "position must mangle method symbols" }
    if ($text -notmatch 'fn: l2_m0') { throw "position missing line_break_width_at" }
    if ($text -notmatch 'sub: l2_m1') { throw "position missing mangled sub" }
    if ($text -notmatch '@: size_t l2_p1_5') { throw "position missing out_line" }
    if ($text -notmatch '@: size_t l2_p1_6') { throw "position missing out_column" }
    if ($text -notmatch 'l2_p1_5\[0\]:') { throw "position missing out_line[0] store" }
    if ($text -notmatch 'l2_p1_6\[0\]:') { throw "position missing out_column[0] store" }
    if ($text -notmatch '(?m)^\s+continue$') { throw "position must emit continue" }
    if ($text -notmatch 'l2_p1_2: l2_p1_1') { throw "position must clamp index to length" }
    if ($text -notmatch 'l2_m0\(node, l2_p1_0, l2_p1_2') { throw "position must pass clamped index as helper length, not full text length" }
    if ($text -match 'l2_m0\(node, l2_p1_0, l2_p1_1') { throw "position must not pass full text length to helper" }

    $l2cases = $cases.Replace("lm_p0_line_break_width_at(", "l2_m0(unit, ").Replace("lm_p0_position_in_slice(", "l2_m1(unit, ")
    $tail = "        return: 0`n    end: main`nend: external"
    $pos = $text.LastIndexOf($tail)
    if ($pos -lt 0) { throw "position L1 missing generated main return" }
    $drive = @"
$l2cases
        return: 0
    end: main
end: external
"@
    $drvLm1 = Join-Path $out "pos_l2_drive.lm1"
    $drvC = Join-Path $out "pos_l2_drive.c"
    $drvExe = Join-Path $out "pos_l2_drive.exe"
    $drvOut = Join-Path $out "pos_l2_drive.stdout"
    [System.IO.File]::WriteAllText((Join-Path (Get-Location) $drvLm1), ($text.Substring(0, $pos) + $drive.Replace("`r`n","`n")))
    & $l1trans $drvLm1 $drvC
    if ($LASTEXITCODE -ne 0) { throw "l1trans failed pos_l2_drive" }
    Invoke-Gcc $drvC $drvExe (Join-Path $log "pos_l2_drive.gcc.log")
    cmd /c "`"$drvExe`" > `"$drvOut`" 2> `"$(Join-Path $out 'pos_l2_drive.err')`""
    if ($LASTEXITCODE -ne 0) { throw "pos_l2_drive exe failed" }
    $a = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $refOut)).Replace("`r`n","`n")
    $b = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $drvOut)).Replace("`r`n","`n")
    if ($a -ne $b) { throw "position mismatch vs parser.lm1`nREF:`n$a`nL2:`n$b" }

    Invoke-Leaf "l2src\parser_physical_line.lm2" "parser_physical_line" 0 "lm_p0_index_is_line_start"
    $mlm1 = Join-Path $out "parser_physical_line.lm1"
    $mtext = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $mlm1)).Replace("`r`n", "`n")
    if ($mtext -notmatch 'sub: l2_m7') { throw "merged physical_line missing position_in_slice" }
    if ($mtext -notmatch 'l2_p7_2: l2_p7_1') { throw "merged position must clamp index to length" }
    if ($mtext -notmatch 'l2_m5\(node, l2_p7_0, l2_p7_2') { throw "merged position must pass clamped index as helper length" }
    if ($mtext -match 'l2_m5\(node, l2_p7_0, l2_p7_1') { throw "merged position must not pass full text length to helper" }
    if ($mtext -notmatch 'const: @\(char l2_own5\)') { throw "merged physical_line missing 6th OwnUsed" }
    $ml2cases = $cases.Replace("lm_p0_line_break_width_at(", "l2_m5(unit, ").Replace("lm_p0_position_in_slice(", "l2_m7(unit, ")
    $mpos = $mtext.LastIndexOf($tail)
    if ($mpos -lt 0) { throw "merged physical_line L1 missing generated main return" }
    $mdrvLm1 = Join-Path $out "pos_merged_drive.lm1"
    $mdrvC = Join-Path $out "pos_merged_drive.c"
    $mdrvExe = Join-Path $out "pos_merged_drive.exe"
    $mdrvOut = Join-Path $out "pos_merged_drive.stdout"
    [System.IO.File]::WriteAllText((Join-Path (Get-Location) $mdrvLm1), ($mtext.Substring(0, $mpos) + ($ml2cases + "`n        return: 0`n    end: main`nend: external").Replace("`r`n","`n")))
    & $l1trans $mdrvLm1 $mdrvC
    if ($LASTEXITCODE -ne 0) { throw "l1trans failed pos_merged_drive" }
    Invoke-Gcc $mdrvC $mdrvExe (Join-Path $log "pos_merged_drive.gcc.log")
    cmd /c "`"$mdrvExe`" > `"$mdrvOut`" 2> `"$(Join-Path $out 'pos_merged_drive.err')`""
    if ($LASTEXITCODE -ne 0) { throw "pos_merged_drive exe failed" }
    $c = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $mdrvOut)).Replace("`r`n","`n")
    if ($a -ne $c) { throw "merged physical_line position mismatch vs parser.lm1`nREF:`n$a`nL2:`n$c" }
}

Invoke-Position

function Invoke-TrailerRole {
    $pred = [System.IO.File]::ReadAllText((Join-Path (Get-Location) "l2src\parser_text_predicates.lm2")).Replace("`r`n", "`n")
    $tr = [System.IO.File]::ReadAllText((Join-Path (Get-Location) "l2src\parser_trailer_role.lm2")).Replace("`r`n", "`n")
    $h = ($pred -split "fn: lm_p0_is_horizontal_space")[1]
    $h = "fn: lm_p0_is_horizontal_space" + ($h -split "fn: lm_p0_is_line_break")[0]
    $th = ($tr -split "fn: lm_p0_is_horizontal_space")[1]
    $th = "fn: lm_p0_is_horizontal_space" + ($th -split "fn: lm_p0_text_has_prefix_name")[0]
    if ($h.Trim() -ne $th.Trim()) { throw "trailer_role is_horizontal_space must match parser_text_predicates.lm2" }

    $dash81 = "-" * 81
    $cases = @"
        c.printf("%d\n", lm_p0_text_has_prefix_name("end:", 4U, "end", 0))
        c.printf("%d\n", lm_p0_text_has_prefix_name("end", 3U, "end", 0))
        c.printf("%d\n", lm_p0_text_has_prefix_name("endif", 5U, "end", 0))
        c.printf("%d\n", lm_p0_text_has_prefix_name("return", 6U, "return", 1))
        c.printf("%d\n", lm_p0_text_has_prefix_name("return x", 8U, "return", 1))
        c.printf("%d\n", lm_p0_legacy_trailer_role("end:", 4U))
        c.printf("%d\n", lm_p0_legacy_trailer_role("end", 3U))
        c.printf("%d\n", lm_p0_legacy_trailer_role("return", 6U))
        c.printf("%d\n", lm_p0_legacy_trailer_role("until:", 6U))
        c.printf("%d\n", lm_p0_legacy_trailer_role("---", 3U))
        c.printf("%d\n", lm_p0_trailer_role("end:", 4U))
        c.printf("%d\n", lm_p0_trailer_role("end", 3U))
        c.printf("%d\n", lm_p0_trailer_role("return", 6U))
        c.printf("%d\n", lm_p0_trailer_role("return:", 7U))
        c.printf("%d\n", lm_p0_trailer_role("until:", 6U))
        c.printf("%d\n", lm_p0_trailer_role("---", 3U))
        c.printf("%d\n", lm_p0_trailer_role("----", 4U))
        c.printf("%d\n", lm_p0_trailer_role("--- ", 4U))
        c.printf("%d\n", lm_p0_trailer_role("--- #c", 6U))
        c.printf("%d\n", lm_p0_trailer_role("--", 2U))
        c.printf("%d\n", lm_p0_trailer_role("foo", 3U))
        c.printf("%d\n", lm_p0_trailer_role("", 0U))
        c.printf("%d\n", lm_p0_trailer_role("$dash81", 81U))
        c.printf("%d\n", lm_p0_trailer_role_from_payload("trailer.end"))
        c.printf("%d\n", lm_p0_trailer_role_from_payload("LM_P0_TRAILER_ROLE_RETURN"))
        c.printf("%d\n", lm_p0_trailer_role_from_payload("trailer.dash-cutter"))
        c.printf("%d\n", lm_p0_trailer_role_from_payload(0))
        c.printf("%d\n", lm_p0_trailer_role_is_tail_cutter(0))
        c.printf("%d\n", lm_p0_trailer_role_is_tail_cutter(2))
        c.printf("%s\n", lm_p0_trailer_role_payload(2))
        c.printf("%s\n", lm_p0_trailer_role_payload(1))
        c.printf("%d\n", lm_p0_trailer_role_payload(0) = 0)
"@

    $refLm1 = Join-Path $out "tr_ref.lm1"
    $refC = Join-Path $out "tr_ref.c"
    $refExe = Join-Path $out "tr_ref.exe"
    $refOut = Join-Path $out "tr_ref.stdout"
    $refSrc = @"
predef: "l1src/parser.lm1"
include: "<stdio.h>"
external:
    fn: main () int
$cases
        return: 0
    end: main
end: external
"@
    [System.IO.File]::WriteAllText((Join-Path (Get-Location) $refLm1), $refSrc.Replace("`r`n","`n"))
    & $l1trans $refLm1 $refC
    if ($LASTEXITCODE -ne 0) { throw "l1trans failed tr_ref" }
    Invoke-Gcc $refC $refExe (Join-Path $log "tr_ref.gcc.log")
    cmd /c "`"$refExe`" > `"$refOut`" 2> `"$(Join-Path $out 'tr_ref.err')`""
    if ($LASTEXITCODE -ne 0) { throw "tr_ref exe failed" }

    Invoke-Leaf "l2src\parser_trailer_role.lm2" "parser_trailer_role" 0 "lm_p0_is_horizontal_space"
    $lm1 = Join-Path $out "parser_trailer_role.lm1"
    $text = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $lm1)).Replace("`r`n", "`n")
    if ($text -match 'fn: lm_p0_' -or $text -match 'sub: lm_p0_') { throw "trailer_role must mangle method symbols" }
    if ($text -notmatch 'fn: l2_m6') { throw "trailer_role missing mangled lm_p0_trailer_role" }
    if ($text -notmatch 'LmP0TrailerRole') { throw "trailer_role L1 missing LmP0TrailerRole" }
    if ($text -notmatch 'predef: "l1src/p0.lm1.h"' -and $text -notmatch 'l1src/p0.lm1.h') { throw "trailer_role must predef p0.lm1.h" }

    $l2cases = $cases.
        Replace("lm_p0_text_has_prefix_name(", "l2_m1(unit, ").
        Replace("lm_p0_legacy_trailer_role(", "l2_m2(unit, ").
        Replace("lm_p0_trailer_role_from_payload(", "l2_m3(unit, ").
        Replace("lm_p0_trailer_role_payload(", "l2_m4(unit, ").
        Replace("lm_p0_trailer_role_is_tail_cutter(", "l2_m5(unit, ").
        Replace("lm_p0_trailer_role(", "l2_m6(unit, ")
    $tail = "        return: 0`n    end: main`nend: external"
    $pos = $text.LastIndexOf($tail)
    if ($pos -lt 0) { throw "trailer_role L1 missing generated main return" }
    $drive = @"
$l2cases
        return: 0
    end: main
end: external
"@
    $drvLm1 = Join-Path $out "tr_l2_drive.lm1"
    $drvC = Join-Path $out "tr_l2_drive.c"
    $drvExe = Join-Path $out "tr_l2_drive.exe"
    $drvOut = Join-Path $out "tr_l2_drive.stdout"
    [System.IO.File]::WriteAllText((Join-Path (Get-Location) $drvLm1), ($text.Substring(0, $pos) + $drive.Replace("`r`n","`n")))
    & $l1trans $drvLm1 $drvC
    if ($LASTEXITCODE -ne 0) { throw "l1trans failed tr_l2_drive" }
    Invoke-Gcc $drvC $drvExe (Join-Path $log "tr_l2_drive.gcc.log")
    cmd /c "`"$drvExe`" > `"$drvOut`" 2> `"$(Join-Path $out 'tr_l2_drive.err')`""
    if ($LASTEXITCODE -ne 0) { throw "tr_l2_drive exe failed" }
    $a = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $refOut)).Replace("`r`n","`n")
    $b = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $drvOut)).Replace("`r`n","`n")
    if ($a -ne $b) { throw "trailer_role mismatch vs parser.lm1`nREF:`n$a`nL2:`n$b" }
}

Invoke-TrailerRole

function Invoke-FenceLine {
    $pred = [System.IO.File]::ReadAllText((Join-Path (Get-Location) "l2src\parser_text_predicates.lm2")).Replace("`r`n", "`n")
    $ls = [System.IO.File]::ReadAllText((Join-Path (Get-Location) "l2src\parser_line_start.lm2")).Replace("`r`n", "`n")
    $fl = [System.IO.File]::ReadAllText((Join-Path (Get-Location) "l2src\parser_fence_line.lm2")).Replace("`r`n", "`n")
    $h = ($pred -split "fn: lm_p0_is_horizontal_space")[1]
    $h = "fn: lm_p0_is_horizontal_space" + ($h -split "fn: lm_p0_is_line_break")[0]
    $fh = ($fl -split "fn: lm_p0_is_horizontal_space")[1]
    $fh = "fn: lm_p0_is_horizontal_space" + ($fh -split "fn: lm_p0_line_rest_is_horizontal_space")[0]
    if ($h.Trim() -ne $fh.Trim()) { throw "fence_line is_horizontal_space must match parser_text_predicates.lm2" }
    $lr = ($ls -split "fn: lm_p0_line_rest_is_horizontal_space")[1]
    $lr = "fn: lm_p0_line_rest_is_horizontal_space" + ($lr -split "fn: main")[0]
    $flr = ($fl -split "fn: lm_p0_line_rest_is_horizontal_space")[1]
    $flr = "fn: lm_p0_line_rest_is_horizontal_space" + ($flr -split "fn: lm_p0_match_block_string_fence_line")[0]
    if ($lr.Trim() -ne $flr.Trim()) { throw "fence_line line_rest must match parser_line_start.lm2" }

    $eq80 = "=" * 80
    $eq81 = "=" * 81
    $st80 = "*" * 80
    $st81 = "*" * 81
    $cases = @"
        c.printf("%d\n", lm_p0_match_block_string_fence_line("===", 0U, 3U, 3U))
        c.printf("%d\n", lm_p0_match_block_string_fence_line("===  ", 0U, 5U, 3U))
        c.printf("%d\n", lm_p0_match_block_string_fence_line("===\t", 0U, 4U, 3U))
        c.printf("%d\n", lm_p0_match_block_string_fence_line("===x", 0U, 4U, 3U))
        c.printf("%d\n", lm_p0_match_block_string_fence_line("====", 0U, 4U, 3U))
        c.printf("%d\n", lm_p0_match_block_string_fence_line("==", 0U, 2U, 3U))
        c.printf("%d\n", lm_p0_match_block_string_fence_line("===", 0U, 3U, 2U))
        c.printf("%d\n", lm_p0_match_block_string_fence_line(" ===", 1U, 4U, 3U))
        c.printf("%d\n", lm_p0_match_block_string_fence_line("", 0U, 0U, 3U))
        c.printf("%d\n", lm_p0_match_block_string_fence_line("***", 0U, 3U, 3U))
        c.printf("%d\n", lm_p0_match_block_string_fence_line("$eq80", 0U, 80U, 80U))
        c.printf("%d\n", lm_p0_match_block_string_fence_line("$eq81", 0U, 81U, 81U))
        c.printf("%d\n", lm_p0_match_block_string_fence_line("$eq80", 0U, 80U, 79U))
        c.printf("%d\n", lm_p0_match_raw_comment_fence_line("***", 0U, 3U, 3U))
        c.printf("%d\n", lm_p0_match_raw_comment_fence_line("***  ", 0U, 5U, 3U))
        c.printf("%d\n", lm_p0_match_raw_comment_fence_line("***\t", 0U, 4U, 3U))
        c.printf("%d\n", lm_p0_match_raw_comment_fence_line("***x", 0U, 4U, 3U))
        c.printf("%d\n", lm_p0_match_raw_comment_fence_line("****", 0U, 4U, 3U))
        c.printf("%d\n", lm_p0_match_raw_comment_fence_line("**", 0U, 2U, 3U))
        c.printf("%d\n", lm_p0_match_raw_comment_fence_line("***", 0U, 3U, 2U))
        c.printf("%d\n", lm_p0_match_raw_comment_fence_line(" ***", 1U, 4U, 3U))
        c.printf("%d\n", lm_p0_match_raw_comment_fence_line("", 0U, 0U, 3U))
        c.printf("%d\n", lm_p0_match_raw_comment_fence_line("===", 0U, 3U, 3U))
        c.printf("%d\n", lm_p0_match_raw_comment_fence_line("$st80", 0U, 80U, 80U))
        c.printf("%d\n", lm_p0_match_raw_comment_fence_line("$st81", 0U, 81U, 81U))
"@

    $refLm1 = Join-Path $out "fence_ref.lm1"
    $refC = Join-Path $out "fence_ref.c"
    $refExe = Join-Path $out "fence_ref.exe"
    $refOut = Join-Path $out "fence_ref.stdout"
    $refSrc = @"
predef: "l1src/parser.lm1"
include: "<stdio.h>"
external:
    fn: main () int
$cases
        return: 0
    end: main
end: external
"@
    [System.IO.File]::WriteAllText((Join-Path (Get-Location) $refLm1), $refSrc.Replace("`r`n","`n"))
    & $l1trans $refLm1 $refC
    if ($LASTEXITCODE -ne 0) { throw "l1trans failed fence_ref" }
    Invoke-Gcc $refC $refExe (Join-Path $log "fence_ref.gcc.log")
    cmd /c "`"$refExe`" > `"$refOut`" 2> `"$(Join-Path $out 'fence_ref.err')`""
    if ($LASTEXITCODE -ne 0) { throw "fence_ref exe failed" }

    Invoke-Leaf "l2src\parser_fence_line.lm2" "parser_fence_line" 0 "lm_p0_is_horizontal_space"
    $lm1 = Join-Path $out "parser_fence_line.lm1"
    $text = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $lm1)).Replace("`r`n", "`n")
    if ($text -match 'fn: lm_p0_' -or $text -match 'sub: lm_p0_') { throw "fence_line must mangle method symbols" }
    if ($text -notmatch 'fn: l2_m2') { throw "fence_line missing match_block_string_fence_line" }
    if ($text -notmatch 'fn: l2_m3') { throw "fence_line missing match_raw_comment_fence_line" }

    $l2cases = $cases.
        Replace("lm_p0_match_block_string_fence_line(", "l2_m2(unit, ").
        Replace("lm_p0_match_raw_comment_fence_line(", "l2_m3(unit, ")
    $tail = "        return: 0`n    end: main`nend: external"
    $pos = $text.LastIndexOf($tail)
    if ($pos -lt 0) { throw "fence_line L1 missing generated main return" }
    $drive = @"
$l2cases
        return: 0
    end: main
end: external
"@
    $drvLm1 = Join-Path $out "fence_l2_drive.lm1"
    $drvC = Join-Path $out "fence_l2_drive.c"
    $drvExe = Join-Path $out "fence_l2_drive.exe"
    $drvOut = Join-Path $out "fence_l2_drive.stdout"
    [System.IO.File]::WriteAllText((Join-Path (Get-Location) $drvLm1), ($text.Substring(0, $pos) + $drive.Replace("`r`n","`n")))
    & $l1trans $drvLm1 $drvC
    if ($LASTEXITCODE -ne 0) { throw "l1trans failed fence_l2_drive" }
    Invoke-Gcc $drvC $drvExe (Join-Path $log "fence_l2_drive.gcc.log")
    cmd /c "`"$drvExe`" > `"$drvOut`" 2> `"$(Join-Path $out 'fence_l2_drive.err')`""
    if ($LASTEXITCODE -ne 0) { throw "fence_l2_drive exe failed" }
    $a = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $refOut)).Replace("`r`n","`n")
    $b = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $drvOut)).Replace("`r`n","`n")
    if ($a -ne $b) { throw "fence_line mismatch vs parser.lm1`nREF:`n$a`nL2:`n$b" }
}

Invoke-FenceLine

function Invoke-LayoutDeeper {
    $cases = @'
        c.printf("%d\n", lm_p0_layout_prefix_is_deeper(0U, 0U, 0U, 0U))
        c.printf("%d\n", lm_p0_layout_prefix_is_deeper(1U, 0U, 0U, 0U))
        c.printf("%d\n", lm_p0_layout_prefix_is_deeper(0U, 0U, 1U, 0U))
        c.printf("%d\n", lm_p0_layout_prefix_is_deeper(4U, 0U, 4U, 0U))
        c.printf("%d\n", lm_p0_layout_prefix_is_deeper(8U, 0U, 4U, 0U))
        c.printf("%d\n", lm_p0_layout_prefix_is_deeper(4U, 0U, 8U, 0U))
        c.printf("%d\n", lm_p0_layout_prefix_is_deeper(0U, 1U, 0U, 0U))
        c.printf("%d\n", lm_p0_layout_prefix_is_deeper(0U, 0U, 0U, 1U))
        c.printf("%d\n", lm_p0_layout_prefix_is_deeper(0U, 2U, 0U, 1U))
        c.printf("%d\n", lm_p0_layout_prefix_is_deeper(0U, 1U, 0U, 2U))
        c.printf("%d\n", lm_p0_layout_prefix_is_deeper(8U, 1U, 0U, 0U))
        c.printf("%d\n", lm_p0_layout_prefix_is_deeper(0U, 0U, 8U, 1U))
        c.printf("%d\n", lm_p0_layout_prefix_is_deeper(8U, 1U, 4U, 1U))
        c.printf("%d\n", lm_p0_layout_prefix_is_deeper(4U, 2U, 8U, 1U))
'@
    $refLm1 = Join-Path $out "layout_ref.lm1"
    $refC = Join-Path $out "layout_ref.c"
    $refExe = Join-Path $out "layout_ref.exe"
    $refOut = Join-Path $out "layout_ref.stdout"
    $refSrc = @"
predef: "l1src/parser.lm1"
include: "<stdio.h>"
external:
    fn: main () int
$cases
        return: 0
    end: main
end: external
"@
    [System.IO.File]::WriteAllText((Join-Path (Get-Location) $refLm1), $refSrc.Replace("`r`n","`n"))
    & $l1trans $refLm1 $refC
    if ($LASTEXITCODE -ne 0) { throw "l1trans failed layout_ref" }
    Invoke-Gcc $refC $refExe (Join-Path $log "layout_ref.gcc.log")
    cmd /c "`"$refExe`" > `"$refOut`" 2> `"$(Join-Path $out 'layout_ref.err')`""
    if ($LASTEXITCODE -ne 0) { throw "layout_ref exe failed" }

    Invoke-Leaf "l2src\parser_layout_deeper.lm2" "parser_layout_deeper" 0 "lm_p0_layout_prefix_is_deeper"
    $lm1 = Join-Path $out "parser_layout_deeper.lm1"
    $text = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $lm1)).Replace("`r`n", "`n")
    if ($text -match 'fn: lm_p0_') { throw "layout_deeper must mangle method symbols" }
    if ($text -notmatch 'fn: l2_m0') { throw "layout_deeper missing mangled method" }
    $l2cases = $cases.Replace("lm_p0_layout_prefix_is_deeper(", "l2_m0(unit, ")
    $tail = "        return: 0`n    end: main`nend: external"
    $pos = $text.LastIndexOf($tail)
    if ($pos -lt 0) { throw "layout_deeper L1 missing generated main return" }
    $drive = @"
$l2cases
        return: 0
    end: main
end: external
"@
    $drvLm1 = Join-Path $out "layout_l2_drive.lm1"
    $drvC = Join-Path $out "layout_l2_drive.c"
    $drvExe = Join-Path $out "layout_l2_drive.exe"
    $drvOut = Join-Path $out "layout_l2_drive.stdout"
    [System.IO.File]::WriteAllText((Join-Path (Get-Location) $drvLm1), ($text.Substring(0, $pos) + $drive.Replace("`r`n","`n")))
    & $l1trans $drvLm1 $drvC
    if ($LASTEXITCODE -ne 0) { throw "l1trans failed layout_l2_drive" }
    Invoke-Gcc $drvC $drvExe (Join-Path $log "layout_l2_drive.gcc.log")
    cmd /c "`"$drvExe`" > `"$drvOut`" 2> `"$(Join-Path $out 'layout_l2_drive.err')`""
    if ($LASTEXITCODE -ne 0) { throw "layout_l2_drive exe failed" }
    $a = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $refOut)).Replace("`r`n","`n")
    $b = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $drvOut)).Replace("`r`n","`n")
    if ($a -ne $b) { throw "layout_deeper mismatch vs parser.lm1`nREF:`n$a`nL2:`n$b" }
}

Invoke-LayoutDeeper

function Invoke-DashFence {
    $pred = [System.IO.File]::ReadAllText((Join-Path (Get-Location) "l2src\parser_text_predicates.lm2")).Replace("`r`n", "`n")
    $df = [System.IO.File]::ReadAllText((Join-Path (Get-Location) "l2src\parser_dash_fence.lm2")).Replace("`r`n", "`n")
    $h = ($pred -split "fn: lm_p0_is_horizontal_space")[1]
    $h = "fn: lm_p0_is_horizontal_space" + ($h -split "fn: lm_p0_is_line_break")[0]
    $dh = ($df -split "fn: lm_p0_is_horizontal_space")[1]
    $dh = "fn: lm_p0_is_horizontal_space" + ($dh -split "fn: lm_p0_is_line_break")[0]
    if ($h.Trim() -ne $dh.Trim()) { throw "dash_fence is_horizontal_space must match parser_text_predicates.lm2" }
    $lb = ($pred -split "fn: lm_p0_is_line_break")[1]
    $lb = "fn: lm_p0_is_line_break" + ($lb -split "fn: lm_p0_is_field_space")[0]
    $dlb = ($df -split "fn: lm_p0_is_line_break")[1]
    $dlb = "fn: lm_p0_is_line_break" + ($dlb -split "fn: lm_p0_scan_brace_mark_unchecked")[0]
    if ($lb.Trim() -ne $dlb.Trim()) { throw "dash_fence is_line_break must match parser_text_predicates.lm2" }

    $d80 = "-" * 80
    $d81 = "-" * 81
    # Status first, then print. Count is 0..81 so cast to int is the safe conversion.
    $cases = @"
        size_t: n
        int: st
        n: 0U
        st: lm_p0_dash_fence_status("---", 3U, @ n)
        c.printf("%d %d\n", st, (cast: int (n)))
        c.fflush(c.stdout)
        c.fflush(c.stdout)
        n: 0U
        st: lm_p0_dash_fence_status("----", 4U, @ n)
        c.printf("%d %d\n", st, (cast: int (n)))
        c.fflush(c.stdout)
        n: 0U
        st: lm_p0_dash_fence_status("--", 2U, @ n)
        c.printf("%d %d\n", st, (cast: int (n)))
        c.fflush(c.stdout)
        n: 0U
        st: lm_p0_dash_fence_status("-", 1U, @ n)
        c.printf("%d %d\n", st, (cast: int (n)))
        c.fflush(c.stdout)
        n: 0U
        st: lm_p0_dash_fence_status("", 0U, @ n)
        c.printf("%d %d\n", st, (cast: int (n)))
        c.fflush(c.stdout)
        n: 0U
        st: lm_p0_dash_fence_status("$d80", 80U, @ n)
        c.printf("%d %d\n", st, (cast: int (n)))
        c.fflush(c.stdout)
        n: 0U
        st: lm_p0_dash_fence_status("$d81", 81U, @ n)
        c.printf("%d %d\n", st, (cast: int (n)))
        c.fflush(c.stdout)
        n: 0U
        st: lm_p0_dash_fence_status("---   ", 6U, @ n)
        c.printf("%d %d\n", st, (cast: int (n)))
        c.fflush(c.stdout)
        n: 0U
        st: lm_p0_dash_fence_status("---`t", 4U, @ n)
        c.printf("%d %d\n", st, (cast: int (n)))
        c.fflush(c.stdout)
        n: 0U
        st: lm_p0_dash_fence_status("---#", 4U, @ n)
        c.printf("%d %d\n", st, (cast: int (n)))
        c.fflush(c.stdout)
        n: 0U
        st: lm_p0_dash_fence_status("--- # x", 7U, @ n)
        c.printf("%d %d\n", st, (cast: int (n)))
        c.fflush(c.stdout)
        n: 0U
        st: lm_p0_dash_fence_status("---x", 4U, @ n)
        c.printf("%d %d\n", st, (cast: int (n)))
        c.fflush(c.stdout)
        n: 0U
        st: lm_p0_dash_fence_status("--- x", 5U, @ n)
        c.printf("%d %d\n", st, (cast: int (n)))
        c.fflush(c.stdout)
        n: 0U
        st: lm_p0_dash_fence_status("--- {x}", 7U, @ n)
        c.printf("%d %d\n", st, (cast: int (n)))
        c.fflush(c.stdout)
        n: 0U
        st: lm_p0_dash_fence_status("--- {x} y", 9U, @ n)
        c.printf("%d %d\n", st, (cast: int (n)))
        c.fflush(c.stdout)
        n: 0U
        st: lm_p0_dash_fence_status("--- {x", 6U, @ n)
        c.printf("%d %d\n", st, (cast: int (n)))
        c.fflush(c.stdout)
        n: 0U
        st: lm_p0_dash_fence_status("--- {}", 6U, @ n)
        c.printf("%d %d\n", st, (cast: int (n)))
        c.fflush(c.stdout)
        n: 0U
        st: lm_p0_dash_fence_status("--- {a {b}}", 11U, @ n)
        c.printf("%d %d\n", st, (cast: int (n)))
        c.fflush(c.stdout)
        n: 0U
        st: lm_p0_dash_fence_status("""--- {"{"}""", 9U, @ n)
        c.printf("%d %d\n", st, (cast: int (n)))
        c.fflush(c.stdout)
        n: 0U
        st: lm_p0_dash_fence_status("""--- {"}"}""", 9U, @ n)
        c.printf("%d %d\n", st, (cast: int (n)))
        c.fflush(c.stdout)
        n: 0U
        st: lm_p0_dash_fence_status('''--- {"""{"""}''', 13U, @ n)
        c.printf("%d %d\n", st, (cast: int (n)))
        c.fflush(c.stdout)
        n: 0U
        st: lm_p0_dash_fence_status("--- {'''{'''}", 13U, @ n)
        c.printf("%d %d\n", st, (cast: int (n)))
        c.fflush(c.stdout)
        n: 0U
        st: lm_p0_dash_fence_status("--- {'}'}", 9U, @ n)
        c.printf("%d %d\n", st, (cast: int (n)))
        c.fflush(c.stdout)
        n: 0U
        st: lm_p0_dash_fence_status("""--- {#}`n}""", 8U, @ n)
        c.printf("%d %d\n", st, (cast: int (n)))
        c.fflush(c.stdout)
        n: 0U
        st: lm_p0_dash_fence_status("""--- {
===
}
===
}""", 16U, @ n)
        c.printf("%d %d\n", st, (cast: int (n)))
        c.fflush(c.stdout)
        n: 0U
        st: lm_p0_dash_fence_status("""--- {
***
}
***
}""", 16U, @ n)
        c.printf("%d %d\n", st, (cast: int (n)))
        c.fflush(c.stdout)
        n: 0U
        st: lm_p0_dash_fence_status("""--- {`r`n}""", 8U, @ n)
        c.printf("%d %d\n", st, (cast: int (n)))
        c.fflush(c.stdout)
"@
    # NONE=0 VALID=1 TOO_LONG=2 TRAILING=3 ; count is dash run length
    # Trailing 9 lines: quoted/triple/char/comment/fence/newline vs parser.lm1 REF.
    $want = "1 3`n1 4`n0 2`n0 1`n0 0`n1 80`n2 81`n1 3`n1 3`n1 3`n1 3`n3 3`n3 3`n1 3`n3 3`n3 3`n1 3`n1 3`n1 3`n1 3`n1 3`n1 3`n1 3`n3 3`n3 3`n3 3`n3 3`n"

    $refLm1 = Join-Path $out "dash_ref.lm1"
    $refC = Join-Path $out "dash_ref.c"
    $refExe = Join-Path $out "dash_ref.exe"
    $refOut = Join-Path $out "dash_ref.stdout"
    $refSrc = @"
predef: "l1src/parser.lm1"
include: "<stdio.h>"
external:
    fn: main () int
$cases
        return: 0
    end: main
end: external
"@
    [System.IO.File]::WriteAllText((Join-Path (Get-Location) $refLm1), $refSrc.Replace("`r`n","`n"))
    & $l1trans $refLm1 $refC
    if ($LASTEXITCODE -ne 0) { throw "l1trans failed dash_ref" }
    Invoke-Gcc $refC $refExe (Join-Path $log "dash_ref.gcc.log")
    cmd /c "`"$refExe`" > `"$refOut`" 2> `"$(Join-Path $out 'dash_ref.err')`""
    if ($LASTEXITCODE -ne 0) { throw "dash_ref exe failed" }

    Invoke-Leaf "l2src\parser_dash_fence.lm2" "parser_dash_fence" 0 "lm_p0_is_horizontal_space"
    $lm1 = Join-Path $out "parser_dash_fence.lm1"
    $text = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $lm1)).Replace("`r`n", "`n")
    if ($text -match 'fn: lm_p0_') { throw "dash_fence must mangle method symbols" }
    if ($text -notmatch 'fn: l2_m3') { throw "dash_fence missing mangled dash_fence_status" }

    $l2cases = $cases.Replace("lm_p0_dash_fence_status(", "l2_m3(unit, ")
    $tail = "        return: 0`n    end: main`nend: external"
    $pos = $text.LastIndexOf($tail)
    if ($pos -lt 0) { throw "dash_fence L1 missing generated main return" }
    $drive = @"
$l2cases
        return: 0
    end: main
end: external
"@
    $drvLm1 = Join-Path $out "dash_l2_drive.lm1"
    $drvC = Join-Path $out "dash_l2_drive.c"
    $drvExe = Join-Path $out "dash_l2_drive.exe"
    $drvOut = Join-Path $out "dash_l2_drive.stdout"
    [System.IO.File]::WriteAllText((Join-Path (Get-Location) $drvLm1), ($text.Substring(0, $pos) + $drive.Replace("`r`n","`n")))
    & $l1trans $drvLm1 $drvC
    if ($LASTEXITCODE -ne 0) { throw "l1trans failed dash_l2_drive" }
    Invoke-Gcc $drvC $drvExe (Join-Path $log "dash_l2_drive.gcc.log")
    $drvErr = Join-Path $out "dash_l2_drive.err"
    $drvEcFile = Join-Path $out "dash_l2_drive.exitcode"
    $drvFull = Join-Path (Get-Location) $drvExe
    $drvOutFull = Join-Path (Get-Location) $drvOut
    $drvErrFull = Join-Path (Get-Location) $drvErr
    $drvEcFull = Join-Path (Get-Location) $drvEcFile
    Remove-Item -LiteralPath $drvEcFile -Force -ErrorAction SilentlyContinue
    $psi = New-Object System.Diagnostics.ProcessStartInfo
    $psi.FileName = $drvFull
    $psi.WorkingDirectory = (Get-Location).Path
    $psi.UseShellExecute = $false
    $psi.RedirectStandardOutput = $true
    $psi.RedirectStandardError = $true
    $drvProc = New-Object System.Diagnostics.Process
    $drvProc.StartInfo = $psi
    [void]$drvProc.Start()
    $outTask = $drvProc.StandardOutput.ReadToEndAsync()
    $errTask = $drvProc.StandardError.ReadToEndAsync()
    if (-not $drvProc.WaitForExit(20000)) {
        try { $drvProc.Kill() } catch { }
        [void]$drvProc.WaitForExit(5000)
        throw "dash_l2_drive timed out after 20s pid=$($drvProc.Id). preserved hang_042800/"
    }
    $drvProc.WaitForExit()
    $drvEc = $drvProc.ExitCode
    Set-Content -LiteralPath $drvEcFile -Value "$drvEc" -Encoding ascii
    [System.IO.File]::WriteAllText($drvOutFull, $outTask.Result.Replace("`r`n", "`n"))
    [System.IO.File]::WriteAllText($drvErrFull, $errTask.Result)
    if ($null -eq $drvEc) { throw "dash_l2_drive finished but ExitCode was not available pid=$($drvProc.Id)" }
    if ($drvEc -ne 0) { throw "dash_l2_drive exe failed exit=$drvEc" }
    $a = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $refOut)).Replace("`r`n","`n")
    $b = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $drvOut)).Replace("`r`n","`n")
    if ($a -ne $want) { throw "dash_fence REF unexpected`nREF:`n$a`nWANT:`n$want" }
    if ($b -ne $want) { throw "dash_fence L2 unexpected`nL2:`n$b`nWANT:`n$want" }
    if ($a -ne $b) { throw "dash_fence mismatch vs parser.lm1`nREF:`n$a`nL2:`n$b" }
}

Invoke-DashFence

"l2trans $gen ok"
$suiteLog = Join-Path $log "l2trans_suite.log"
$toolHash = (Get-FileHash -Algorithm SHA256 (Join-Path (Get-Location) $l1trans)).Hash
@(
    "cmd=l2src\run_l2trans.ps1"
    "L1_GEN=$gen"
    "l1trans=$l1trans"
    "l1trans_sha256=$toolHash"
    "banner=l2trans $gen ok"
    "exit=0"
) | Set-Content -LiteralPath $suiteLog -Encoding utf8

