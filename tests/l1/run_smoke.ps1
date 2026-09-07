# l1trans gen0 smoke. CWD = repo root.
# Does not rewrite tests/trans_* harness. Generated C/binaries stay under
# build\obj\l1trans\gen0 and build\l1trans\gen0 (already gitignored).

$ErrorActionPreference = "Stop"
Set-Location (Join-Path $PSScriptRoot "..\..")

$gen = "gen0"
if ($env:L1_GEN -and $env:L1_GEN.Trim().Length -gt 0) { $gen = $env:L1_GEN.Trim() }
$l1trans = "build\l1trans\$gen\l1trans.exe"
$obj = "build\obj\l1trans\$gen"
$bin = "build\l1trans\$gen"
$log = Join-Path "build\l1trans\logs" $gen
$script:smokeLog = Join-Path $log "smoke.log"
$cflags = @("-std=c99", "-Wall", "-Wextra", "-Wpedantic", "-I", ".", "-Werror=incompatible-pointer-types", "-Werror=discarded-qualifiers", "-Werror=implicit-function-declaration", "-Werror=implicit-int")
$cflagsStr = ($cflags -join " ")

function Invoke-Translate([string]$src, [string]$dst) {
    & $l1trans $src $dst
    if ($LASTEXITCODE -ne 0) {
        throw "translate failed ($LASTEXITCODE): $src"
    }
}

function Invoke-TranslateFail([string]$src, [string]$dst, [string]$err, [string]$needle) {
    cmd /c "$l1trans $src $dst 2> $err"
    if ($LASTEXITCODE -eq 0) {
        throw "expected translate failure: $src"
    }
    $text = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $err))
    if ($text.IndexOf($needle) -lt 0) {
        throw "missing diagnostic '$needle' in $err : $text"
    }
}

function Assert-CHas([string]$cpath, [string]$needle) {
    $text = Get-Content $cpath -Raw
    if ($text.IndexOf($needle) -lt 0) {
        throw "missing '$needle' in $cpath"
    }
}

function Assert-CLacks([string]$cpath, [string]$needle) {
    $text = Get-Content $cpath -Raw
    if ($text.IndexOf($needle) -ge 0) {
        throw "unexpected '$needle' in $cpath"
    }
}

function Invoke-PreserveFail([string]$src, [string]$dst, [string]$err, [string]$needle) {
    $full = Join-Path (Get-Location) $dst
    $marker = "PRESERVE-DESTINATION`n"
    [System.IO.File]::WriteAllText($full, $marker)
    Invoke-TranslateFail $src $dst $err $needle
    $got = [System.IO.File]::ReadAllText($full)
    if ($got -ne $marker) {
        throw "destination mutated on failed translate: $dst"
    }
    Remove-Item -LiteralPath $full -Force
    Invoke-TranslateFail $src $dst $err $needle
    if (Test-Path -LiteralPath $full) {
        throw "failed translate created destination: $dst"
    }
    if (Test-Path -LiteralPath ($full + ".tmp")) {
        throw "failed translate left temp: $dst.tmp"
    }
}

function Write-Log([string]$msg) {
    $line = "$(Get-Date -Format o) $msg"
    Add-Content -LiteralPath $script:smokeLog -Value $line
}

function Invoke-Logged([string]$title, [scriptblock]$cmd) {
    Write-Log "BEGIN $title"
    $out = & $cmd 2>&1 | Out-String
    $ec = $LASTEXITCODE
    Write-Log "EXIT $ec"
    if ($out) { Write-Log $out.TrimEnd() }
    return $ec
}

function Invoke-CcRun([string]$cpath, [string]$exepath, [int]$expectExit, [string]$expectOut) {
    $gccLog = Join-Path $log ("gcc_" + [IO.Path]::GetFileNameWithoutExtension($exepath) + ".log")
    $runOut = Join-Path $log ("run_" + [IO.Path]::GetFileNameWithoutExtension($exepath) + ".stdout")
    $runErr = Join-Path $log ("run_" + [IO.Path]::GetFileNameWithoutExtension($exepath) + ".stderr")
    cmd /c "gcc $cflagsStr -o $exepath $cpath > $gccLog 2>&1"
    if ($LASTEXITCODE -ne 0) {
        throw "gcc failed: $cpath (see $gccLog)"
    }
    Write-Log "gcc $cpath -> $exepath exit 0"
    $psi = New-Object System.Diagnostics.ProcessStartInfo
    $psi.FileName = (Join-Path (Get-Location) $exepath)
    $psi.UseShellExecute = $false
    $psi.RedirectStandardOutput = $true
    $psi.RedirectStandardError = $true
    $psi.CreateNoWindow = $true
    $proc = New-Object System.Diagnostics.Process
    $proc.StartInfo = $psi
    [void]$proc.Start()
    $ms = New-Object System.IO.MemoryStream
    $proc.StandardOutput.BaseStream.CopyTo($ms)
    $errText = $proc.StandardError.ReadToEnd()
    $proc.WaitForExit()
    $ec = $proc.ExitCode
    $bytes = $ms.ToArray()
    [System.IO.File]::WriteAllBytes((Join-Path (Get-Location) $runOut), $bytes)
    [System.IO.File]::WriteAllText((Join-Path (Get-Location) $runErr), $errText)
    Write-Log "run $exepath exit $ec stdout_bytes=$($bytes.Length)"
    if ($ec -ne $expectExit) {
        throw "run exit $ec expected $expectExit : $exepath"
    }
    if ([string]::IsNullOrEmpty($expectOut)) {
        if ($bytes.Length -ne 0) {
            throw "expected empty stdout for $exepath, got $($bytes.Length) bytes"
        }
    } else {
        $expectBytes = [System.Text.Encoding]::ASCII.GetBytes($expectOut)
        $expectCrlf = $null
        if ($expectOut.EndsWith("`n") -and -not $expectOut.EndsWith("`r`n")) {
            $expectCrlf = [System.Text.Encoding]::ASCII.GetBytes($expectOut.Substring(0, $expectOut.Length - 1) + "`r`n")
        }
        $match = $false
        foreach ($exp in @($expectBytes, $expectCrlf)) {
            if ($null -eq $exp) { continue }
            if ($bytes.Length -ne $exp.Length) { continue }
            $same = $true
            for ($i = 0; $i -lt $bytes.Length; $i++) {
                if ($bytes[$i] -ne $exp[$i]) { $same = $false; break }
            }
            if ($same) { $match = $true; break }
        }
        $hex = ($bytes | ForEach-Object { $_.ToString("X2") }) -join " "
        Write-Log "stdout raw $hex"
        if (-not $match) {
            throw "stdout raw mismatch for $exepath bytes=$hex"
        }
    }
}

function Assert-ByteIdentical([string]$a, [string]$b) {
    $ha = (Get-FileHash $a).Hash
    $hb = (Get-FileHash $b).Hash
    if ($ha -ne $hb) {
        throw "C not byte-identical: $a vs $b"
    }
}

New-Item -ItemType Directory -Force -Path $obj, $bin, $log | Out-Null
if (-not (Test-Path -LiteralPath $l1trans)) {
    throw "missing $l1trans"
}
$srcInfo = Get-Item "lm2\l1trans.lm2"
$exeInfo = Get-Item $l1trans
if ($srcInfo.LastWriteTimeUtc -gt $exeInfo.LastWriteTimeUtc) {
    throw "stale l1trans.exe (source $($srcInfo.LastWriteTimeUtc) newer than exe $($exeInfo.LastWriteTimeUtc))"
}
Set-Content -LiteralPath $script:smokeLog -Value "$(Get-Date -Format o) smoke start src=$($srcInfo.LastWriteTimeUtc) exe=$($exeInfo.LastWriteTimeUtc) hash_src=$((Get-FileHash lm2\l1trans.lm2).Hash) hash_exe=$((Get-FileHash $l1trans).Hash)"

Invoke-Translate "tests\l1\integer_add.lm2" "$obj\integer_add.c"
Invoke-Translate "tests\l1\integer_add.lm2" "$obj\integer_add_b.c"
Assert-ByteIdentical "$obj\integer_add.c" "$obj\integer_add_b.c"
Invoke-CcRun "$obj\integer_add.c" "$bin\integer_add.exe" 0 $null

Invoke-Translate "tests\l1\include_printf.lm2" "$obj\include_printf.c"
Invoke-Translate "tests\l1\include_printf.lm2" "$obj\include_printf_b.c"
Assert-ByteIdentical "$obj\include_printf.c" "$obj\include_printf_b.c"
$inc = Get-Content "$obj\include_printf.c" -Raw
if ($inc.Contains('#include "<stdio.h>"')) {
    throw "include lowering still quoted angle header"
}
if (-not $inc.Contains("#include <stdio.h>")) {
    throw "include lowering missing #include <stdio.h>"
}
Invoke-CcRun "$obj\include_printf.c" "$bin\include_printf.exe" 0 "include-ok`n"

Invoke-Translate "tests\l1\for_loop.lm2" "$obj\for_loop.c"
Invoke-Translate "tests\l1\for_loop.lm2" "$obj\for_loop_b.c"
Assert-ByteIdentical "$obj\for_loop.c" "$obj\for_loop_b.c"
Invoke-CcRun "$obj\for_loop.c" "$bin\for_loop.exe" 0 $null

Invoke-Translate "tests\l1\array_1d.lm2" "$obj\array_1d.c"
Invoke-Translate "tests\l1\array_1d.lm2" "$obj\array_1d_b.c"
Assert-ByteIdentical "$obj\array_1d.c" "$obj\array_1d_b.c"
Invoke-CcRun "$obj\array_1d.c" "$bin\array_1d.exe" 0 $null

Invoke-Translate "tests\l1\array_2d.lm2" "$obj\array_2d.c"
Invoke-Translate "tests\l1\array_2d.lm2" "$obj\array_2d_b.c"
Assert-ByteIdentical "$obj\array_2d.c" "$obj\array_2d_b.c"
Assert-CHas "$obj\array_2d.c" "int matrix[2][2]"
Assert-CLacks "$obj\array_2d.c" "int matrix[2*2]"
Invoke-CcRun "$obj\array_2d.c" "$bin\array_2d.exe" 0 $null

Invoke-Translate "tests\l1\unary_call.lm2" "$obj\unary_call.c"
Invoke-Translate "tests\l1\unary_call.lm2" "$obj\unary_call_b.c"
Assert-ByteIdentical "$obj\unary_call.c" "$obj\unary_call_b.c"
Assert-CHas "$obj\unary_call.c" "add2(-a, b)"
Invoke-CcRun "$obj\unary_call.c" "$bin\unary_call.exe" 0 $null

Invoke-Translate "tests\l1\field_follow.lm2" "$obj\field_follow.c"
Invoke-Translate "tests\l1\field_follow.lm2" "$obj\field_follow_b.c"
Assert-ByteIdentical "$obj\field_follow.c" "$obj\field_follow_b.c"
Assert-CHas "$obj\field_follow.c" "->"
Assert-CHas "$obj\field_follow.c" "p->length"
Assert-CLacks "$obj\field_follow.c" "return p \\"
Assert-CLacks "$obj\field_follow.c" "return p\\"
Invoke-CcRun "$obj\field_follow.c" "$bin\field_follow.exe" 0 $null

Invoke-Translate "tests\l1\ptr_depth.lm2" "$obj\ptr_depth.c"
Assert-CHas "$obj\ptr_depth.c" "int **"
Invoke-CcRun "$obj\ptr_depth.c" "$bin\ptr_depth.exe" 0 $null

Invoke-Translate "tests\l1\own_oom.lm2" "$obj\own_oom.c"
Invoke-CcRun "$obj\own_oom.c" "$bin\own_oom.exe" 0 $null

Invoke-Translate "tests\l1\init_expr.lm2" "$obj\init_expr.c"
Invoke-Translate "tests\l1\init_expr.lm2" "$obj\init_expr_b.c"
Assert-ByteIdentical "$obj\init_expr.c" "$obj\init_expr_b.c"
Assert-CHas "$obj\init_expr.c" "int total = 1 + 2;"
Assert-CLacks "$obj\init_expr.c" "int total = total"
Invoke-CcRun "$obj\init_expr.c" "$bin\init_expr.exe" 0 $null

Invoke-Translate "tests\l1\fn_dedent.lm2" "$obj\fn_dedent.c"
Invoke-Translate "tests\l1\fn_dedent.lm2" "$obj\fn_dedent_b.c"
Assert-ByteIdentical "$obj\fn_dedent.c" "$obj\fn_dedent_b.c"
Invoke-CcRun "$obj\fn_dedent.c" "$bin\fn_dedent.exe" 0 $null

Invoke-Translate "tests\l1\prototype.lm2" "$obj\prototype.c"
Invoke-Translate "tests\l1\prototype.lm2" "$obj\prototype_b.c"
Assert-ByteIdentical "$obj\prototype.c" "$obj\prototype_b.c"
Assert-CHas "$obj\prototype.c" "int proto(int n);"
Assert-CLacks "$obj\prototype.c" "int proto(int n)`r`n{"
Assert-CLacks "$obj\prototype.c" "int proto(int n)`n{"
Invoke-CcRun "$obj\prototype.c" "$bin\prototype.exe" 0 $null

Invoke-Translate "tests\l1\sub_void.lm2" "$obj\sub_void.c"
Invoke-Translate "tests\l1\sub_void.lm2" "$obj\sub_void_b.c"
Assert-ByteIdentical "$obj\sub_void.c" "$obj\sub_void_b.c"
Assert-CHas "$obj\sub_void.c" "void helper(int x)"
Assert-CHas "$obj\sub_void.c" "x = x + 1;"
Invoke-CcRun "$obj\sub_void.c" "$bin\sub_void.exe" 0 $null

Invoke-Translate "tests\l1\shift_ops.lm2" "$obj\shift_ops.c"
Invoke-Translate "tests\l1\shift_ops.lm2" "$obj\shift_ops_b.c"
Assert-ByteIdentical "$obj\shift_ops.c" "$obj\shift_ops_b.c"
Assert-CHas "$obj\shift_ops.c" "<<"
Assert-CHas "$obj\shift_ops.c" ">>"
Assert-CLacks "$obj\shift_ops.c" "< <"
Assert-CLacks "$obj\shift_ops.c" "> >"
Invoke-CcRun "$obj\shift_ops.c" "$bin\shift_ops.exe" 0 $null

Invoke-PreserveFail "tests\l1\invalid_no_l1.lm2" "$obj\invalid_no_l1.c" "$log\invalid_no_l1.err" "missing L1 body"
Invoke-PreserveFail "tests\l1\invalid_nested_l1.lm2" "$obj\invalid_nested_l1.c" "$log\invalid_nested_l1.err" "missing L1 body"
Invoke-PreserveFail "tests\l1\invalid_unsupported.lm2" "$obj\invalid_unsupported.c" "$log\invalid_unsupported.err" "unsupported L1 form"
Invoke-PreserveFail "tests\l1\invalid_import_missing.lm2" "$obj\invalid_import_missing.c" "$log\invalid_import_missing.err" "cannot read import"
Invoke-PreserveFail "tests\l1\invalid_import_cycle.lm2" "$obj\invalid_import_cycle.c" "$log\invalid_import_cycle.err" "import cycle"
Invoke-PreserveFail "tests\l1\invalid_array_extent.lm2" "$obj\invalid_array_extent.c" "$log\invalid_array_extent.err" "array missing extent"

$dirDest = Join-Path $obj "publish_fail.c"
if (Test-Path -LiteralPath $dirDest) { Remove-Item -LiteralPath $dirDest -Recurse -Force }
New-Item -ItemType Directory -Force -Path $dirDest | Out-Null
cmd /c "$l1trans tests\l1\integer_add.lm2 $dirDest 2> $log\publish_fail.err"
if ($LASTEXITCODE -eq 0) {
    throw "expected publish failure over directory destination"
}
if (-not (Test-Path -LiteralPath $dirDest -PathType Container)) {
    throw "directory destination was destroyed"
}
if (Test-Path -LiteralPath ($dirDest + ".tmp")) {
    throw "publish failure left temp"
}
Write-Log "publish-fail over directory preserved dest, exit $LASTEXITCODE"

Invoke-Translate "tests\l1\import_add.lm2" "$obj\import_add.c"
Invoke-Translate "tests\l1\import_add.lm2" "$obj\import_add_b.c"
Assert-ByteIdentical "$obj\import_add.c" "$obj\import_add_b.c"
Assert-CHas "$obj\import_add.c" "int add(int a, int b)"
Invoke-CcRun "$obj\import_add.c" "$bin\import_add.exe" 0 $null

Invoke-Translate "tests\l1\predef_hoist.lm2" "$obj\predef_hoist.c"
Invoke-Translate "tests\l1\predef_hoist.lm2" "$obj\predef_hoist_b.c"
Assert-ByteIdentical "$obj\predef_hoist.c" "$obj\predef_hoist_b.c"
Assert-CHas "$obj\predef_hoist.c" "int add(int a, int b)"
Invoke-CcRun "$obj\predef_hoist.c" "$bin\predef_hoist.exe" 0 $null

Invoke-Translate "tests\l1\import_repeat.lm2" "$obj\import_repeat.c"
Invoke-Translate "tests\l1\import_repeat.lm2" "$obj\import_repeat_b.c"
Assert-ByteIdentical "$obj\import_repeat.c" "$obj\import_repeat_b.c"
Invoke-CcRun "$obj\import_repeat.c" "$bin\import_repeat.exe" 0 $null

Invoke-Translate "tests\l1\import_diamond.lm2" "$obj\import_diamond.c"
Invoke-Translate "tests\l1\import_diamond.lm2" "$obj\import_diamond_b.c"
Assert-ByteIdentical "$obj\import_diamond.c" "$obj\import_diamond_b.c"
Invoke-CcRun "$obj\import_diamond.c" "$bin\import_diamond.exe" 0 $null

Invoke-Translate "tests\l1\os_select.lm2" "$obj\os_select.c"
Invoke-Translate "tests\l1\os_select.lm2" "$obj\os_select_b.c"
Assert-ByteIdentical "$obj\os_select.c" "$obj\os_select_b.c"
Assert-CHas "$obj\os_select.c" "#ifdef _WIN32"
Invoke-CcRun "$obj\os_select.c" "$bin\os_select.exe" 0 $null

Invoke-Translate "tests\l1\repro_call_index.lm2" "$obj\repro_call_index.c"
Invoke-Translate "tests\l1\repro_call_index.lm2" "$obj\repro_call_index_b.c"
Assert-ByteIdentical "$obj\repro_call_index.c" "$obj\repro_call_index_b.c"
Assert-CHas "$obj\repro_call_index.c" "argv[1]"
Assert-CLacks "$obj\repro_call_index.c" "[,"
Invoke-CcRun "$obj\repro_call_index.c" "$bin\repro_call_index.exe" 1 $null

Invoke-Translate "tests\l1\repro_call_binop.lm2" "$obj\repro_call_binop.c"
Invoke-Translate "tests\l1\repro_call_binop.lm2" "$obj\repro_call_binop_b.c"
Assert-ByteIdentical "$obj\repro_call_binop.c" "$obj\repro_call_binop_b.c"
Assert-CHas "$obj\repro_call_binop.c" "char buffer[32];"
Assert-CHas "$obj\repro_call_binop.c" "memcpy(buffer + 0,"
Assert-CLacks "$obj\repro_call_binop.c" "buffer[] = {32}"
Invoke-CcRun "$obj\repro_call_binop.c" "$bin\repro_call_binop.exe" 0 $null

Write-Log "smoke ok"
Write-Output "l1trans gen0 smoke ok"
