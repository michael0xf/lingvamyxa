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

"l2trans $gen ok"
