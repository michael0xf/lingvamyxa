# l1trans smoke. CWD = repo root. L1_GEN selects the translator.
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
$cflags = @("-std=c99", "-Wall", "-Wextra", "-Wpedantic", "-I", ".", "-I", "lm1/build", "-Werror=incompatible-pointer-types", "-Werror=discarded-qualifiers", "-Werror=implicit-function-declaration", "-Werror=implicit-int")
$cflagsStr = ($cflags -join " ")

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
try {
foreach ($k in $savedHosted.Keys) { Remove-Item "Env:$k" -ErrorAction SilentlyContinue }

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

function Invoke-CcRun([string]$cpath, [string]$exepath, [int]$expectExit, [string]$expectOut, [string]$runArgs = "", [string]$runId = "") {
    $baseName = [IO.Path]::GetFileNameWithoutExtension($exepath)
    $gccLog = Join-Path $log ("gcc_" + $baseName + ".log")
    $runTag = $baseName
    if ($runId -and $runId.Trim().Length -gt 0) { $runTag = $baseName + "_" + $runId.Trim() }
    $runOut = Join-Path $log ("run_" + $runTag + ".stdout")
    $runErr = Join-Path $log ("run_" + $runTag + ".stderr")
    cmd /c "gcc $cflagsStr -o $exepath $cpath > $gccLog 2>&1"
    if ($LASTEXITCODE -ne 0) {
        throw "gcc failed: $cpath (see $gccLog)"
    }
    Write-Log "gcc $cpath -> $exepath exit 0"
    $psi = New-Object System.Diagnostics.ProcessStartInfo
    $psi.FileName = (Join-Path (Get-Location) $exepath)
    $psi.Arguments = $runArgs
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
    Write-Log "run $exepath args='$runArgs' id='$runId' exit $ec stdout_bytes=$($bytes.Length)"
    if ($ec -ne $expectExit) {
        throw "run exit $ec expected $expectExit : $exepath args='$runArgs'"
    }
    if ([string]::IsNullOrEmpty($expectOut)) {
        if ($bytes.Length -ne 0) {
            throw "expected empty stdout for $exepath, got $($bytes.Length) bytes"
        }
    } else {
        $expectBytes = [System.Text.Encoding]::ASCII.GetBytes($expectOut)
        $expectCrlf = $null
        $expectAllCrlf = $null
        if ($expectOut.EndsWith("`n") -and -not $expectOut.EndsWith("`r`n")) {
            $expectCrlf = [System.Text.Encoding]::ASCII.GetBytes($expectOut.Substring(0, $expectOut.Length - 1) + "`r`n")
            $expectAllCrlf = [System.Text.Encoding]::ASCII.GetBytes($expectOut.Replace("`n", "`r`n"))
        }
        $match = $false
        foreach ($exp in @($expectBytes, $expectCrlf, $expectAllCrlf)) {
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

Invoke-Translate "tests\l1\integer_add.lm1" "$obj\integer_add.c"
Invoke-Translate "tests\l1\integer_add.lm1" "$obj\integer_add_b.c"
Assert-ByteIdentical "$obj\integer_add.c" "$obj\integer_add_b.c"
Invoke-CcRun "$obj\integer_add.c" "$bin\integer_add.exe" 0 $null
Invoke-Translate "tests\l1\fn_close_name.lm1" "$obj\fn_close_name.c"
Assert-CHas "$obj\fn_close_name.c" "int add("
Invoke-CcRun "$obj\fn_close_name.c" "$bin\fn_close_name.exe" 0 $null
Invoke-Translate "tests\l1\fn_close_fn.lm1" "$obj\fn_close_fn.c"
Assert-CHas "$obj\fn_close_fn.c" "int add("
Invoke-CcRun "$obj\fn_close_fn.c" "$bin\fn_close_fn.exe" 0 $null
Invoke-Translate "tests\l1\fn_close_ret.lm1" "$obj\fn_close_ret.c"
Assert-CHas "$obj\fn_close_ret.c" "return a;"
Invoke-CcRun "$obj\fn_close_ret.c" "$bin\fn_close_ret.exe" 0 $null
Invoke-Translate "tests\l1\fn_close_dash.lm1" "$obj\fn_close_dash.c"
Assert-CHas "$obj\fn_close_dash.c" "int add("
Invoke-CcRun "$obj\fn_close_dash.c" "$bin\fn_close_dash.exe" 0 $null
Invoke-TranslateFail "tests\l1\invalid_fn_end_wrong.lm1" "$obj\invalid_fn_end_wrong.c" "$log\invalid_fn_end_wrong.err" "end target does not match close target"
Invoke-TranslateFail "tests\l1\invalid_fn_end_empty.lm1" "$obj\invalid_fn_end_empty.c" "$log\invalid_fn_end_empty.err" "empty colon Frame is not allowed"
Invoke-TranslateFail "tests\l1\invalid_empty_colon_noop.lm1" "$obj\invalid_empty_colon_noop.c" "$log\invalid_empty_colon_noop.err" "empty colon Frame is not allowed"
Invoke-Translate "tests\l1\fn_ret_tr_vertical.lm1" "$obj\fn_ret_tr_vertical.c"
Assert-CHas "$obj\fn_ret_tr_vertical.c" "return 5 + 5;"
Invoke-CcRun "$obj\fn_ret_tr_vertical.c" "$bin\fn_ret_tr_vertical.exe" 10 $null
Invoke-Translate "tests\l1\quote_run.lm1" "$obj\quote_run.c"
$quoteRunOut = (@('a"""b', "a'''b", 'x"y', 'x""y', 'x""""y', 'x"""""y', "a'b", 'a\b') -join "`n") + "`n"
Invoke-CcRun "$obj\quote_run.c" "$bin\quote_run.exe" 0 $quoteRunOut
Invoke-Translate "tests\l1\lm1_unit.lm1" "$obj\lm1_unit.c"
Assert-CHas "$obj\lm1_unit.c" "int main(void)"
Invoke-CcRun "$obj\lm1_unit.c" "$bin\lm1_unit.exe" 0 $null
Invoke-Translate "tests\l1\lm1_unit.LM1" "$obj\lm1_unit_upper.c"
Assert-ByteIdentical "$obj\lm1_unit.c" "$obj\lm1_unit_upper.c"
Invoke-Translate "tests\l1\lm1_import.lm1" "$obj\lm1_import.c"
Assert-CHas "$obj\lm1_import.c" "int lm1_mod_value(void)"
Invoke-CcRun "$obj\lm1_import.c" "$bin\lm1_import.exe" 0 $null
Invoke-Translate "tests\l1\ident_reserved_ok.lm1" "$obj\ident_reserved_ok.c"
Assert-CHas "$obj\ident_reserved_ok.c" "C_value"
Assert-CHas "$obj\ident_reserved_ok.c" "L1State"
Invoke-CcRun "$obj\ident_reserved_ok.c" "$bin\ident_reserved_ok.exe" 0 $null
Invoke-Translate "tests\l1\ident_c_proj.lm1" "$obj\ident_c_proj.c"
Assert-CHas "$obj\ident_c_proj.c" "return C;"
Assert-CLacks "$obj\ident_c_proj.c" "c.C"
Invoke-CcRun "$obj\ident_c_proj.c" "$bin\ident_c_proj.exe" 0 $null

Invoke-Translate "tests\l1\include_printf.lm1" "$obj\include_printf.c"
Invoke-Translate "tests\l1\include_printf.lm1" "$obj\include_printf_b.c"
Assert-ByteIdentical "$obj\include_printf.c" "$obj\include_printf_b.c"
$inc = Get-Content "$obj\include_printf.c" -Raw
if ($inc.Contains('#include "<stdio.h>"')) {
    throw "include lowering still quoted angle header"
}
if (-not $inc.Contains("#include <stdio.h>")) {
    throw "include lowering missing #include <stdio.h>"
}
Invoke-CcRun "$obj\include_printf.c" "$bin\include_printf.exe" 0 "include-ok`n"

Invoke-Translate "tests\l1\for_loop.lm1" "$obj\for_loop.c"
Invoke-Translate "tests\l1\for_loop.lm1" "$obj\for_loop_b.c"
Assert-ByteIdentical "$obj\for_loop.c" "$obj\for_loop_b.c"
Invoke-CcRun "$obj\for_loop.c" "$bin\for_loop.exe" 0 $null

Invoke-Translate "tests\l1\array_1d.lm1" "$obj\array_1d.c"
Invoke-Translate "tests\l1\array_1d.lm1" "$obj\array_1d_b.c"
Assert-ByteIdentical "$obj\array_1d.c" "$obj\array_1d_b.c"
Invoke-CcRun "$obj\array_1d.c" "$bin\array_1d.exe" 0 $null

Invoke-Translate "tests\l1\array_2d.lm1" "$obj\array_2d.c"
Invoke-Translate "tests\l1\array_2d.lm1" "$obj\array_2d_b.c"
Assert-ByteIdentical "$obj\array_2d.c" "$obj\array_2d_b.c"
Assert-CHas "$obj\array_2d.c" "int matrix[2][2]"
Assert-CLacks "$obj\array_2d.c" "int matrix[2*2]"
Invoke-CcRun "$obj\array_2d.c" "$bin\array_2d.exe" 0 $null

Invoke-Translate "tests\l1\unary_call.lm1" "$obj\unary_call.c"
Invoke-Translate "tests\l1\unary_call.lm1" "$obj\unary_call_b.c"
Assert-ByteIdentical "$obj\unary_call.c" "$obj\unary_call_b.c"
Assert-CHas "$obj\unary_call.c" "add2(-a, b)"
Invoke-CcRun "$obj\unary_call.c" "$bin\unary_call.exe" 0 $null

Invoke-Translate "tests\l1\field_follow.lm1" "$obj\field_follow.c"
Invoke-Translate "tests\l1\field_follow.lm1" "$obj\field_follow_b.c"
Assert-ByteIdentical "$obj\field_follow.c" "$obj\field_follow_b.c"
Assert-CHas "$obj\field_follow.c" "->"
Assert-CHas "$obj\field_follow.c" "p->length"
Assert-CLacks "$obj\field_follow.c" "return p \\"
Assert-CLacks "$obj\field_follow.c" "return p\\"
Invoke-CcRun "$obj\field_follow.c" "$bin\field_follow.exe" 0 $null

Invoke-Translate "tests\l1\ptr_depth.lm1" "$obj\ptr_depth.c"
Assert-CHas "$obj\ptr_depth.c" "int **"
Invoke-CcRun "$obj\ptr_depth.c" "$bin\ptr_depth.exe" 0 $null

Invoke-Translate "tests\l1\own_oom.lm1" "$obj\own_oom.c"
Invoke-CcRun "$obj\own_oom.c" "$bin\own_oom.exe" 0 $null

Invoke-Translate "tests\l1\init_expr.lm1" "$obj\init_expr.c"
Invoke-Translate "tests\l1\init_expr.lm1" "$obj\init_expr_b.c"
Assert-ByteIdentical "$obj\init_expr.c" "$obj\init_expr_b.c"
Assert-CHas "$obj\init_expr.c" "int total = 1 + 2;"
Assert-CLacks "$obj\init_expr.c" "int total = total"
Invoke-CcRun "$obj\init_expr.c" "$bin\init_expr.exe" 0 $null

Invoke-Translate "tests\l1\fn_dedent.lm1" "$obj\fn_dedent.c"
Invoke-Translate "tests\l1\fn_dedent.lm1" "$obj\fn_dedent_b.c"
Assert-ByteIdentical "$obj\fn_dedent.c" "$obj\fn_dedent_b.c"
Invoke-CcRun "$obj\fn_dedent.c" "$bin\fn_dedent.exe" 0 $null

Invoke-Translate "tests\l1\prototype.lm1" "$obj\prototype.c"
Invoke-Translate "tests\l1\prototype.lm1" "$obj\prototype_b.c"
Assert-ByteIdentical "$obj\prototype.c" "$obj\prototype_b.c"
Assert-CHas "$obj\prototype.c" "int proto(int n);"
Assert-CLacks "$obj\prototype.c" "int proto(int n)`r`n{"
Assert-CLacks "$obj\prototype.c" "int proto(int n)`n{"
Invoke-CcRun "$obj\prototype.c" "$bin\prototype.exe" 0 $null

Invoke-Translate "tests\l1\sub_void.lm1" "$obj\sub_void.c"
Invoke-Translate "tests\l1\sub_void.lm1" "$obj\sub_void_b.c"
Assert-ByteIdentical "$obj\sub_void.c" "$obj\sub_void_b.c"
Assert-CHas "$obj\sub_void.c" "void helper(int x)"
Assert-CHas "$obj\sub_void.c" "x = x + 1;"
Invoke-CcRun "$obj\sub_void.c" "$bin\sub_void.exe" 0 $null

Invoke-Translate "tests\l1\shift_ops.lm1" "$obj\shift_ops.c"
Invoke-Translate "tests\l1\shift_ops.lm1" "$obj\shift_ops_b.c"
Assert-ByteIdentical "$obj\shift_ops.c" "$obj\shift_ops_b.c"
Assert-CHas "$obj\shift_ops.c" "<<"
Assert-CHas "$obj\shift_ops.c" ">>"
Assert-CLacks "$obj\shift_ops.c" "< <"
Assert-CLacks "$obj\shift_ops.c" "> >"
Invoke-CcRun "$obj\shift_ops.c" "$bin\shift_ops.exe" 0 $null

Invoke-PreserveFail "tests\l1\invalid_no_l1.lm2" "$obj\invalid_no_l1.c" "$log\invalid_no_l1.err" "missing L1 body"
Invoke-PreserveFail "tests\l1\invalid_nested_l1.lm2" "$obj\invalid_nested_l1.c" "$log\invalid_nested_l1.err" "missing L1 body"
Invoke-PreserveFail "tests\l1\invalid_empty_lm1.lm1" "$obj\invalid_empty_lm1.c" "$log\invalid_empty_lm1.err" "empty implicit L1 body"
Invoke-PreserveFail "tests\l1\invalid_lm1_explicit_l1.lm1" "$obj\invalid_lm1_explicit_l1.c" "$log\invalid_lm1_explicit_l1.err" "reserved L1 name"
Invoke-PreserveFail "tests\l1\invalid_lm1_nested_l1.lm1" "$obj\invalid_lm1_nested_l1.c" "$log\invalid_lm1_nested_l1.err" "reserved L1 name"
Invoke-PreserveFail "tests\l1\invalid_lm1_l2.lm1" "$obj\invalid_lm1_l2.c" "$log\invalid_lm1_l2.err" "reserved L1 name"
Invoke-PreserveFail "tests\l1\invalid_lm1_l3.lm1" "$obj\invalid_lm1_l3.c" "$log\invalid_lm1_l3.err" "reserved L1 name"
Invoke-PreserveFail "tests\l1\invalid_lm1_c_top.lm1" "$obj\invalid_lm1_c_top.c" "$log\invalid_lm1_c_top.err" "reserved L1 name"
Invoke-PreserveFail "tests\l1\invalid_lm1_c_body.lm1" "$obj\invalid_lm1_c_body.c" "$log\invalid_lm1_c_body.err" "reserved L1 name"
Invoke-PreserveFail "tests\l1\invalid_lm1_atom.lm1" "$obj\invalid_lm1_atom.c" "$log\invalid_lm1_atom.err" "implicit L1 expects a statement frame"
Invoke-PreserveFail "tests\l1\invalid_lm1_win.lm1" "$obj\invalid_lm1_win.c" "$log\invalid_lm1_win.err" "unsupported L1 form"
Invoke-PreserveFail "tests\l1\invalid_lm1_default.lm1" "$obj\invalid_lm1_default.c" "$log\invalid_lm1_default.err" "unsupported L1 form"
Invoke-PreserveFail "tests\l1\invalid_lm1_external_atom.lm1" "$obj\invalid_lm1_external_atom.c" "$log\invalid_lm1_external_atom.err" "external without fn"
Invoke-PreserveFail "tests\l1\invalid_lm1_external_extra.lm1" "$obj\invalid_lm1_external_extra.c" "$log\invalid_lm1_external_extra.err" "unsupported L1 form"
Invoke-PreserveFail "tests\l1\invalid_lm1_ident_c.lm1" "$obj\invalid_lm1_ident_c.c" "$log\invalid_lm1_ident_c.err" "reserved L1 name"
Invoke-PreserveFail "tests\l1\invalid_lm1_fn_l1.lm1" "$obj\invalid_lm1_fn_l1.c" "$log\invalid_lm1_fn_l1.err" "reserved L1 name"
Invoke-PreserveFail "tests\l1\invalid_lm1_define_l2.lm1" "$obj\invalid_lm1_define_l2.c" "$log\invalid_lm1_define_l2.err" "reserved L1 name"
Invoke-PreserveFail "tests\l1\invalid_lm1_ifdef_l3.lm1" "$obj\invalid_lm1_ifdef_l3.c" "$log\invalid_lm1_ifdef_l3.err" "reserved L1 name"
Invoke-PreserveFail "tests\l1\invalid_lm1_ref_c.lm1" "$obj\invalid_lm1_ref_c.c" "$log\invalid_lm1_ref_c.err" "reserved L1 name"
Invoke-PreserveFail "tests\l1\invalid_lm1_field_c.lm1" "$obj\invalid_lm1_field_c.c" "$log\invalid_lm1_field_c.err" "reserved L1 name"
Invoke-PreserveFail "tests\l1\invalid_lm1_type_l1.lm1" "$obj\invalid_lm1_type_l1.c" "$log\invalid_lm1_type_l1.err" "reserved L1 name"
Invoke-PreserveFail "tests\l1\invalid_lm1_param_l2.lm1" "$obj\invalid_lm1_param_l2.c" "$log\invalid_lm1_param_l2.err" "reserved L1 name"
Invoke-PreserveFail "tests\l1\invalid_lm1_define_repl_c.lm1" "$obj\invalid_lm1_define_repl_c.c" "$log\invalid_lm1_define_repl_c.err" "reserved L1 name"
Invoke-PreserveFail "tests\l1\invalid_l1_wrong_ext.txt" "$obj\invalid_l1_wrong_ext.c" "$log\invalid_l1_wrong_ext.err" "l1trans expects a .lm1 or .lm2 source"
Invoke-PreserveFail "tests\l1\invalid_unsupported.lm1" "$obj\invalid_unsupported.c" "$log\invalid_unsupported.err" "unsupported L1 form"
Invoke-PreserveFail "tests\l1\invalid_import_missing.lm1" "$obj\invalid_import_missing.c" "$log\invalid_import_missing.err" "cannot read import"
Invoke-PreserveFail "tests\l1\invalid_import_cycle.lm1" "$obj\invalid_import_cycle.c" "$log\invalid_import_cycle.err" "import cycle"
Invoke-PreserveFail "tests\l1\invalid_array_extent.lm1" "$obj\invalid_array_extent.c" "$log\invalid_array_extent.err" "array missing extent"
# Requires gen1+: fixtures for l1src fixes not in lm2 seed (TASK5 + empty_sub + cast path).
# gen0 is built from lm2\l1trans.lm2 via run_seed; these checks would stay red forever on gen0.
if ($gen -eq "gen0") {
    Write-Log "skip requires-gen1+ on gen0: immutable, invalid_unknown_type, invalid_unknown_ctype, scalar_size_t_ok, empty_sub_ok, cast_multiword_ok, cast_uchar_alias_ok, cast_ptr_uchar_ok, param_multiword_ok, invalid_param_qualifier_extra, header units H1"
} else {
    Invoke-Translate "tests\l1\immutable_group.lm1" "$obj\immutable_group.c"
    Assert-CHas "$obj\immutable_group.c" "const int a = 2;"
    Assert-CHas "$obj\immutable_group.c" "const int b = 3;"
    Assert-CHas "$obj\immutable_group.c" "const char * s = "
    Assert-CHas "$obj\immutable_group.c" "const int c = 4;"
    Assert-CHas "$obj\immutable_group.c" "const int d = 5;"
    Assert-CLacks "$obj\immutable_group.c" "const int mut"
    Assert-CLacks "$obj\immutable_group.c" "const int e"
    Invoke-CcRun "$obj\immutable_group.c" "$bin\immutable_group.exe" 0 $null
    Invoke-PreserveFail "tests\l1\invalid_immutable_if.lm1" "$obj\invalid_immutable_if.c" "$log\invalid_immutable_if.err" "immutable does not qualify this form"
    Invoke-Translate "tests\l1\invalid_immutable_write.lm1" "$obj\invalid_immutable_write.c"
    Assert-CHas "$obj\invalid_immutable_write.c" "const int a = 1;"
    cmd /c "gcc $cflagsStr -o $bin\invalid_immutable_write.exe $obj\invalid_immutable_write.c > $log\gcc_invalid_immutable_write.log 2>&1"
    if ($LASTEXITCODE -eq 0) { throw "expected gcc failure writing immutable int" }
    Invoke-PreserveFail "tests\l1\invalid_unknown_type.lm1" "$obj\invalid_unknown_type.c" "$log\invalid_unknown_type.err" "unknown type name"
    Invoke-PreserveFail "tests\l1\invalid_unknown_ctype.lm1" "$obj\invalid_unknown_ctype.c" "$log\invalid_unknown_ctype.err" "unknown type name"
    Invoke-Translate "tests\l1\scalar_size_t_ok.lm1" "$obj\scalar_size_t_ok.c"
    Assert-CHas "$obj\scalar_size_t_ok.c" "size_t x = 0;"
    Invoke-CcRun "$obj\scalar_size_t_ok.c" "$bin\scalar_size_t_ok.exe" 0 $null
    Invoke-Translate "tests\l1\empty_sub_ok.lm1" "$obj\empty_sub_ok.c"
    Assert-CHas "$obj\empty_sub_ok.c" "void sample_entry(void)"
    Assert-CHas "$obj\empty_sub_ok.c" "sample_entry();"
    Assert-CLacks "$obj\empty_sub_ok.c" "void sample_entry(void);"
    Invoke-CcRun "$obj\empty_sub_ok.c" "$bin\empty_sub_ok.exe" 0 $null
    Invoke-Translate "tests\l1\cast_multiword_ok.lm1" "$obj\cast_multiword_ok.c"
    Assert-CHas "$obj\cast_multiword_ok.c" "(unsigned char)"
    Assert-CLacks "$obj\cast_multiword_ok.c" "charp"
    Invoke-CcRun "$obj\cast_multiword_ok.c" "$bin\cast_multiword_ok.exe" 0 $null
    Invoke-Translate "tests\l1\cast_uchar_alias_ok.lm1" "$obj\cast_uchar_alias_ok.c"
    Assert-CHas "$obj\cast_uchar_alias_ok.c" "(uchar)"
    Assert-CLacks "$obj\cast_uchar_alias_ok.c" "(unsigned char) ch"
    Invoke-CcRun "$obj\cast_uchar_alias_ok.c" "$bin\cast_uchar_alias_ok.exe" 0 $null
    Invoke-Translate "tests\l1\cast_ptr_uchar_ok.lm1" "$obj\cast_ptr_uchar_ok.c"
    Assert-CHas "$obj\cast_ptr_uchar_ok.c" "uchar *"
    Assert-CLacks "$obj\cast_ptr_uchar_ok.c" "unsigned char *"
    Invoke-CcRun "$obj\cast_ptr_uchar_ok.c" "$bin\cast_ptr_uchar_ok.exe" 0 $null
    Invoke-PreserveFail "tests\l1\invalid_param_qualifier_extra.lm1" "$obj\invalid_param_qualifier_extra.c" "$log\invalid_param_qualifier_extra.err" "qualifier parameter has extra fields"
    Invoke-Translate "tests\l1\param_multiword_ok.lm1" "$obj\param_multiword_ok.c"
    Assert-CHas "$obj\param_multiword_ok.c" "int mix(unsigned char * p, unsigned long n, const char * s)"
    Assert-CLacks "$obj\param_multiword_ok.c" "unsigned *char"
    Assert-CLacks "$obj\param_multiword_ok.c" "int mix(unsigned long"
    Invoke-CcRun "$obj\param_multiword_ok.c" "$bin\param_multiword_ok.exe" 0 $null

    # H1 header units (*.h.lm1) — gated gen1+
    New-Item -ItemType Directory -Force -Path "$obj\headers\hdr_dir_a", "$obj\headers\hdr_dir_b" | Out-Null
    Invoke-Translate "tests\l1\hdr_aggregate.h.lm1" "$obj\headers\hdr_aggregate.lm1.h"
    Assert-CHas "$obj\headers\hdr_aggregate.lm1.h" "typedef struct Point Point;"
    Assert-CHas "$obj\headers\hdr_aggregate.lm1.h" "struct Point {"
    Assert-CHas "$obj\headers\hdr_aggregate.lm1.h" "int x;"
    Assert-CHas "$obj\headers\hdr_aggregate.lm1.h" "size_t n;"
    Assert-CHas "$obj\headers\hdr_aggregate.lm1.h" "#ifndef LM_H_tests_2Fl1_2Fhdr_5Faggregate_2Eh_2Elm1"

    Invoke-Translate "tests\l1\hdr_selfptr.h.lm1" "$obj\headers\hdr_selfptr.lm1.h"
    Assert-CHas "$obj\headers\hdr_selfptr.lm1.h" "typedef struct Node Node;"
    Assert-CHas "$obj\headers\hdr_selfptr.lm1.h" "Node * next;"

    Invoke-Translate "tests\l1\hdr_mutual.h.lm1" "$obj\headers\hdr_mutual.lm1.h"
    Assert-CHas "$obj\headers\hdr_mutual.lm1.h" "typedef struct Left Left;"
    Assert-CHas "$obj\headers\hdr_mutual.lm1.h" "typedef struct Right Right;"
    Assert-CHas "$obj\headers\hdr_mutual.lm1.h" "Right * right;"
    Assert-CHas "$obj\headers\hdr_mutual.lm1.h" "Left * left;"

    Invoke-Translate "tests\l1\hdr_byval_order.h.lm1" "$obj\headers\hdr_byval_order.lm1.h"
    $byval = Get-Content "$obj\headers\hdr_byval_order.lm1.h" -Raw
    $innerAt = $byval.IndexOf("struct Inner {")
    $outerAt = $byval.IndexOf("struct Outer {")
    if ($innerAt -lt 0 -or $outerAt -lt 0 -or $innerAt -gt $outerAt) {
        throw "by-value dependency order not applied in hdr_byval_order.lm1.h"
    }

    Invoke-PreserveFail "tests\l1\invalid_hdr_byval_cycle.h.lm1" "$obj\headers\invalid_hdr_byval_cycle.lm1.h" "$log\invalid_hdr_byval_cycle.err" "by-value cycle between struct"

    Invoke-Translate "tests\l1\hdr_combo.h.lm1" "$obj\headers\hdr_combo.lm1.h"
    Assert-CHas "$obj\headers\hdr_combo.lm1.h" "typedef enum Style"
    Assert-CHas "$obj\headers\hdr_combo.lm1.h" "typedef int (*CellVisitor)"
    Assert-CHas "$obj\headers\hdr_combo.lm1.h" "int rect_open(Rect * rect, size_t rows, size_t cols);"
    # generated header compiles under C99 (project cflags)
    cmd /c "gcc $cflagsStr -c -o $obj\headers\hdr_combo_compile.o -x c $obj\headers\hdr_combo.lm1.h > $log\hdr_combo_compile.log 2>&1"
    if ($LASTEXITCODE -ne 0) { throw "generated hdr_combo.lm1.h failed to compile (see $log\hdr_combo_compile.log)" }

    # external C consumer includes generated header and compiles
    Copy-Item -LiteralPath "$obj\headers\hdr_combo.lm1.h" -Destination "$obj\hdr_combo.lm1.h" -Force
    cmd /c "gcc $cflagsStr -I $obj -o $bin\hdr_consumer.exe tests\l1\hdr_consumer.c > $log\hdr_consumer_gcc.log 2>&1"
    if ($LASTEXITCODE -ne 0) { throw "hdr_consumer.c failed to compile (see $log\hdr_consumer_gcc.log)" }
    Write-Log "hdr_consumer compile ok"

    Invoke-PreserveFail "tests\l1\invalid_hdr_union.h.lm1" "$obj\headers\invalid_hdr_union.lm1.h" "$log\invalid_hdr_union.err" "union"
    Invoke-PreserveFail "tests\l1\invalid_hdr_flmacro.h.lm1" "$obj\headers\invalid_hdr_flmacro.lm1.h" "$log\invalid_hdr_flmacro.err" "function-like macro"
    Invoke-PreserveFail "tests\l1\invalid_hdr_body.h.lm1" "$obj\headers\invalid_hdr_body.lm1.h" "$log\invalid_hdr_body.err" "executable body"
    Invoke-PreserveFail "tests\l1\invalid_hdr_unknown.h.lm1" "$obj\headers\invalid_hdr_unknown.lm1.h" "$log\invalid_hdr_unknown.err" "unsupported header unit form"

    Invoke-Translate "tests\l1\hdr_flmacro.h.lm1" "$obj\headers\hdr_flmacro.lm1.h"
    Assert-CHas "$obj\headers\hdr_flmacro.lm1.h" "#define LM_UNUSED(x)"
    Assert-CHas "$obj\headers\hdr_flmacro.lm1.h" "((void) (x))"
    cmd /c "gcc $cflagsStr -c -o $obj\headers\hdr_flmacro_compile.o -x c $obj\headers\hdr_flmacro.lm1.h > $log\hdr_flmacro_compile.log 2>&1"
    if ($LASTEXITCODE -ne 0) { throw "generated hdr_flmacro.lm1.h failed to compile (see $log\hdr_flmacro_compile.log)" }

    Invoke-Translate "tests\l1\hdr_alias.h.lm1" "$obj\headers\hdr_alias.lm1.h"
    Assert-CHas "$obj\headers\hdr_alias.lm1.h" "typedef unsigned char uchar;"
    Assert-CHas "$obj\headers\hdr_alias.lm1.h" "typedef int Kind;"
    Assert-CHas "$obj\headers\hdr_alias.lm1.h" "typedef unsigned Flags;"
    Assert-CHas "$obj\headers\hdr_alias.lm1.h" "Kind kind;"
    Assert-CHas "$obj\headers\hdr_alias.lm1.h" "char message[8];"
    Assert-CHas "$obj\headers\hdr_alias.lm1.h" "const char * data;"
    Assert-CHas "$obj\headers\hdr_alias.lm1.h" "uchar * bytes;"
    Assert-CHas "$obj\headers\hdr_alias.lm1.h" "void ** items;"
    Assert-CHas "$obj\headers\hdr_alias.lm1.h" "#define LM_UNUSED(x)"
    cmd /c "gcc $cflagsStr -c -o $obj\headers\hdr_alias_compile.o -x c $obj\headers\hdr_alias.lm1.h > $log\hdr_alias_compile.log 2>&1"
    if ($LASTEXITCODE -ne 0) { throw "generated hdr_alias.lm1.h failed to compile (see $log\hdr_alias_compile.log)" }

    Invoke-Translate "l1src\p0.h.lm1" "$obj\headers\p0.lm1.h"
    Assert-CHas "$obj\headers\p0.lm1.h" "#ifndef LM_H_l1src_2Fp0_2Eh_2Elm1"
    Assert-CHas "$obj\headers\p0.lm1.h" "#define LM_UNUSED(x)"
    Assert-CHas "$obj\headers\p0.lm1.h" "#define LM_P0_TRAILER_COLON 2U"
    Assert-CHas "$obj\headers\p0.lm1.h" "typedef unsigned char uchar;"
    Assert-CHas "$obj\headers\p0.lm1.h" "typedef int LmP0NodeKind;"
    Assert-CHas "$obj\headers\p0.lm1.h" "typedef unsigned LmP0TrailerFlags;"
    Assert-CHas "$obj\headers\p0.lm1.h" "char message[256];"
    Assert-CHas "$obj\headers\p0.lm1.h" "LmP0FieldParseLoopContinuation continuation;"
    Assert-CHas "$obj\headers\p0.lm1.h" "typedef void (*LmOwnDelete)"
    Assert-CHas "$obj\headers\p0.lm1.h" "int lm_p0_parse_string("
    cmd /c "gcc $cflagsStr -c -o $obj\headers\p0_compile.o -x c $obj\headers\p0.lm1.h > $log\p0_compile.log 2>&1"
    if ($LASTEXITCODE -ne 0) { throw "generated p0.lm1.h failed to compile (see $log\p0_compile.log)" }
    # same unit-root as cwd must not change guard/identity
    & $l1trans "--unit-root" (Get-Location).Path "l1src\p0.h.lm1" "$obj\headers\p0_unitroot.lm1.h"
    if ($LASTEXITCODE -ne 0) { throw "translate failed ($LASTEXITCODE): p0.h.lm1 --unit-root" }
    Assert-ByteIdentical "$obj\headers\p0.lm1.h" "$obj\headers\p0_unitroot.lm1.h"

    New-Item -ItemType Directory -Force -Path "$obj\headers\tests\l1\hdr_dir_a", "$obj\headers\tests\l1\hdr_dir_b", "$obj\headers\tests\l1\hdr_diamond" | Out-Null
    Copy-Item -LiteralPath "$obj\headers\hdr_combo.lm1.h" -Destination "$obj\headers\tests\l1\hdr_combo.lm1.h" -Force

    Invoke-Translate "tests\l1\hdr_predef_consumer.lm1" "$obj\hdr_predef_consumer.c"
    Assert-CHas "$obj\hdr_predef_consumer.c" "#include `"tests/l1/hdr_combo.lm1.h`""
    Assert-CLacks "$obj\hdr_predef_consumer.c" "struct Rect"
    Assert-CHas "$obj\hdr_predef_consumer.c" "int use_rect(Rect * rect)"
    cmd /c "gcc $cflagsStr -I $obj\headers -o $bin\hdr_predef_consumer.exe $obj\hdr_predef_consumer.c > $log\hdr_predef_consumer_gcc.log 2>&1"
    if ($LASTEXITCODE -ne 0) { throw "hdr_predef_consumer.c failed to compile (see $log\hdr_predef_consumer_gcc.log)" }

    # repeated predef of the same header (not a diamond)
    Invoke-Translate "tests\l1\hdr_predef_diamond.lm1" "$obj\hdr_predef_repeat.c"
    $pr = Get-Content "$obj\hdr_predef_repeat.c" -Raw
    $incCount = ([regex]::Matches($pr, '#include "tests/l1/hdr_combo.lm1.h"')).Count
    if ($incCount -ne 1) { throw "repeated predef must emit exactly one include, got $incCount" }
    cmd /c "gcc $cflagsStr -I $obj\headers -o $bin\hdr_predef_repeat.exe $obj\hdr_predef_repeat.c > $log\hdr_predef_repeat_gcc.log 2>&1"
    if ($LASTEXITCODE -ne 0) { throw "hdr_predef_repeat.c failed to compile (see $log\hdr_predef_repeat_gcc.log)" }

    Invoke-Translate "tests/l1/hdr_dir_a/core.h.lm1" "$obj\headers\tests\l1\hdr_dir_a\core.lm1.h"
    Invoke-Translate "tests/l1/hdr_dir_b/core.h.lm1" "$obj\headers\tests\l1\hdr_dir_b\core.lm1.h"
    Assert-CHas "$obj\headers\tests\l1\hdr_dir_a\core.lm1.h" "LM_H_tests_2Fl1_2Fhdr_5Fdir_5Fa_2Fcore_2Eh_2Elm1"
    Assert-CHas "$obj\headers\tests\l1\hdr_dir_b\core.lm1.h" "LM_H_tests_2Fl1_2Fhdr_5Fdir_5Fb_2Fcore_2Eh_2Elm1"
    Assert-CHas "$obj\headers\tests\l1\hdr_dir_a\core.lm1.h" "struct CoreA"
    Assert-CHas "$obj\headers\tests\l1\hdr_dir_b\core.lm1.h" "struct CoreB"

    Invoke-Translate "tests\l1\hdr_dir_both.lm1" "$obj\hdr_dir_both.c"
    Assert-CHas "$obj\hdr_dir_both.c" "#include `"tests/l1/hdr_dir_a/core.lm1.h`""
    Assert-CHas "$obj\hdr_dir_both.c" "#include `"tests/l1/hdr_dir_b/core.lm1.h`""
    Assert-CLacks "$obj\hdr_dir_both.c" "#include `"core.lm1.h`""
    cmd /c "gcc $cflagsStr -I $obj\headers -o $bin\hdr_dir_both.exe $obj\hdr_dir_both.c > $log\hdr_dir_both_gcc.log 2>&1"
    if ($LASTEXITCODE -ne 0) { throw "hdr_dir_both.c failed to compile (see $log\hdr_dir_both_gcc.log)" }

    Invoke-Translate "tests/l1/hdr_diamond/common.h.lm1" "$obj\headers\tests\l1\hdr_diamond\common.lm1.h"
    Invoke-Translate "tests/l1/hdr_diamond/left.h.lm1" "$obj\headers\tests\l1\hdr_diamond\left.lm1.h"
    Invoke-Translate "tests/l1/hdr_diamond/right.h.lm1" "$obj\headers\tests\l1\hdr_diamond\right.lm1.h"
    Invoke-Translate "tests/l1/hdr_diamond/holder.h.lm1" "$obj\headers\tests\l1\hdr_diamond\holder.lm1.h"
    Assert-CHas "$obj\headers\tests\l1\hdr_diamond\holder.lm1.h" "Shared s;"
    Assert-CHas "$obj\headers\tests\l1\hdr_diamond\holder.lm1.h" "HoldFn fn;"
    Assert-CHas "$obj\headers\tests\l1\hdr_diamond\holder.lm1.h" "typedef int (*HoldFn)(Shared s);"
    Assert-CLacks "$obj\headers\tests\l1\hdr_diamond\holder.lm1.h" "by-value cycle"
    cmd /c "gcc $cflagsStr -I $obj\headers -c -o $obj\headers\tests\l1\hdr_diamond\holder_compile.o -x c $obj\headers\tests\l1\hdr_diamond\holder.lm1.h > $log\hdr_holder_compile.log 2>&1"
    if ($LASTEXITCODE -ne 0) { throw "generated holder.lm1.h failed to compile (see $log\hdr_holder_compile.log)" }
    Assert-CHas "$obj\headers\tests\l1\hdr_diamond\left.lm1.h" "#include `"tests/l1/hdr_diamond/common.lm1.h`""
    Assert-CHas "$obj\headers\tests\l1\hdr_diamond\right.lm1.h" "#include `"tests/l1/hdr_diamond/common.lm1.h`""
    Invoke-Translate "tests\l1\hdr_diamond_root.lm1" "$obj\hdr_diamond_root.c"
    Assert-CHas "$obj\hdr_diamond_root.c" "#include `"tests/l1/hdr_diamond/left.lm1.h`""
    Assert-CHas "$obj\hdr_diamond_root.c" "#include `"tests/l1/hdr_diamond/right.lm1.h`""
    Assert-CLacks "$obj\hdr_diamond_root.c" "hdr_diamond/common.lm1.h"
    Assert-CHas "$obj\hdr_diamond_root.c" "Shared *"
    cmd /c "gcc $cflagsStr -I $obj\headers -o $bin\hdr_diamond_root.exe $obj\hdr_diamond_root.c > $log\hdr_diamond_root_gcc.log 2>&1"
    if ($LASTEXITCODE -ne 0) { throw "hdr_diamond_root.c failed to compile (see $log\hdr_diamond_root_gcc.log)" }

    Invoke-PreserveFail "tests/l1/hdr_cycle_a.h.lm1" "$obj\headers\hdr_cycle_a.lm1.h" "$log\hdr_cycle.err" "import cycle"

    Invoke-Translate "tests\l1\hdr_aggregate.h.lm1" "$obj\headers\hdr_aggregate_b.lm1.h"
    Assert-ByteIdentical "$obj\headers\hdr_aggregate.lm1.h" "$obj\headers\hdr_aggregate_b.lm1.h"

    # H1 correction: owned fnptr as struct field joins dependency ordering
    Invoke-Translate "tests\l1\hdr_fnptr_field.h.lm1" "$obj\headers\hdr_fnptr_field.lm1.h"
    $ff = Get-Content "$obj\headers\hdr_fnptr_field.lm1.h" -Raw
    $visitorAt = $ff.IndexOf("typedef int (*Visitor)")
    $walkerAt = $ff.IndexOf("struct Walker {")
    if ($visitorAt -lt 0 -or $walkerAt -lt 0 -or $visitorAt -gt $walkerAt) {
        throw "fnptr field order: Visitor typedef must precede Walker in hdr_fnptr_field.lm1.h"
    }
    Assert-CHas "$obj\headers\hdr_fnptr_field.lm1.h" "Visitor visit;"
    cmd /c "gcc $cflagsStr -c -o $obj\headers\hdr_fnptr_field_compile.o -x c $obj\headers\hdr_fnptr_field.lm1.h > $log\hdr_fnptr_field_compile.log 2>&1"
    if ($LASTEXITCODE -ne 0) { throw "generated hdr_fnptr_field.lm1.h failed to compile (see $log\hdr_fnptr_field_compile.log)" }

    Invoke-Translate "tests\l1\hdr_fnptr_field_before.h.lm1" "$obj\headers\hdr_fnptr_field_before.lm1.h"
    $fb = Get-Content "$obj\headers\hdr_fnptr_field_before.lm1.h" -Raw
    $visitorAt2 = $fb.IndexOf("typedef int (*Visitor)")
    $walkerAt2 = $fb.IndexOf("struct Walker {")
    if ($visitorAt2 -lt 0 -or $walkerAt2 -lt 0 -or $visitorAt2 -gt $walkerAt2) {
        throw "fnptr field order: Visitor typedef must precede Walker in hdr_fnptr_field_before.lm1.h"
    }
    cmd /c "gcc $cflagsStr -c -o $obj\headers\hdr_fnptr_field_before_compile.o -x c $obj\headers\hdr_fnptr_field_before.lm1.h > $log\hdr_fnptr_field_before_compile.log 2>&1"
    if ($LASTEXITCODE -ne 0) { throw "generated hdr_fnptr_field_before.lm1.h failed to compile (see $log\hdr_fnptr_field_before_compile.log)" }

    Invoke-Translate "tests\l1\hdr_fnptr_byval_arg.h.lm1" "$obj\headers\hdr_fnptr_byval_arg.lm1.h"
    $ba = Get-Content "$obj\headers\hdr_fnptr_byval_arg.lm1.h" -Raw
    $payloadAt = $ba.IndexOf("struct Payload {")
    $takeAt = $ba.IndexOf("typedef int (*TakePayload)")
    if ($payloadAt -lt 0 -or $takeAt -lt 0 -or $payloadAt -gt $takeAt) {
        throw "fnptr by-value arg: Payload definition must precede TakePayload typedef"
    }
    cmd /c "gcc $cflagsStr -c -o $obj\headers\hdr_fnptr_byval_arg_compile.o -x c $obj\headers\hdr_fnptr_byval_arg.lm1.h > $log\hdr_fnptr_byval_arg_compile.log 2>&1"
    if ($LASTEXITCODE -ne 0) { throw "generated hdr_fnptr_byval_arg.lm1.h failed to compile (see $log\hdr_fnptr_byval_arg_compile.log)" }

    Invoke-PreserveFail "tests\l1\invalid_hdr_fnptr_cycle.h.lm1" "$obj\headers\invalid_hdr_fnptr_cycle.lm1.h" "$log\invalid_hdr_fnptr_cycle.err" "typedef"
    $cyc = Get-Content "$log\invalid_hdr_fnptr_cycle.err" -Raw
    if ($cyc -match "struct Bfn") { throw "fnptr cycle diagnostic must not call typedef Bfn a struct" }
}
$dirDest = Join-Path $obj "publish_fail.c"
if (Test-Path -LiteralPath $dirDest) { Remove-Item -LiteralPath $dirDest -Recurse -Force }
New-Item -ItemType Directory -Force -Path $dirDest | Out-Null
cmd /c "$l1trans tests\l1\integer_add.lm1 $dirDest 2> $log\publish_fail.err"
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

Invoke-Translate "tests\l1\import_add.lm1" "$obj\import_add.c"
Invoke-Translate "tests\l1\import_add.lm1" "$obj\import_add_b.c"
Assert-ByteIdentical "$obj\import_add.c" "$obj\import_add_b.c"
Assert-CHas "$obj\import_add.c" "int add(int a, int b)"
Invoke-CcRun "$obj\import_add.c" "$bin\import_add.exe" 0 $null

Invoke-Translate "tests\l1\predef_hoist.lm1" "$obj\predef_hoist.c"
Invoke-Translate "tests\l1\predef_hoist.lm1" "$obj\predef_hoist_b.c"
Assert-ByteIdentical "$obj\predef_hoist.c" "$obj\predef_hoist_b.c"
Assert-CHas "$obj\predef_hoist.c" "int add(int a, int b)"
Invoke-CcRun "$obj\predef_hoist.c" "$bin\predef_hoist.exe" 0 $null

Invoke-Translate "tests\l1\import_repeat.lm1" "$obj\import_repeat.c"
Invoke-Translate "tests\l1\import_repeat.lm1" "$obj\import_repeat_b.c"
Assert-ByteIdentical "$obj\import_repeat.c" "$obj\import_repeat_b.c"
Invoke-CcRun "$obj\import_repeat.c" "$bin\import_repeat.exe" 0 $null

Invoke-Translate "tests\l1\import_diamond.lm1" "$obj\import_diamond.c"
Invoke-Translate "tests\l1\import_diamond.lm1" "$obj\import_diamond_b.c"
Assert-ByteIdentical "$obj\import_diamond.c" "$obj\import_diamond_b.c"
Invoke-CcRun "$obj\import_diamond.c" "$bin\import_diamond.exe" 0 $null

Invoke-Translate "tests\l1\os_select.lm1" "$obj\os_select.c"
Invoke-Translate "tests\l1\os_select.lm1" "$obj\os_select_b.c"
Assert-ByteIdentical "$obj\os_select.c" "$obj\os_select_b.c"
Assert-CHas "$obj\os_select.c" "#ifdef _WIN32"
Invoke-CcRun "$obj\os_select.c" "$bin\os_select.exe" 0 $null

Invoke-Translate "tests\l1\repro_call_index.lm1" "$obj\repro_call_index.c"
Invoke-Translate "tests\l1\repro_call_index.lm1" "$obj\repro_call_index_b.c"
Assert-ByteIdentical "$obj\repro_call_index.c" "$obj\repro_call_index_b.c"
Assert-CHas "$obj\repro_call_index.c" "argv[1]"
Assert-CLacks "$obj\repro_call_index.c" "[,"
# argc<2 -> 1; strcmp(argv[1],"ok")!=0 -> 1 for "bad", 0 for "ok"
Invoke-CcRun "$obj\repro_call_index.c" "$bin\repro_call_index.exe" 1 $null "" "noargs"
Invoke-CcRun "$obj\repro_call_index.c" "$bin\repro_call_index.exe" 0 $null "ok" "ok"
Invoke-CcRun "$obj\repro_call_index.c" "$bin\repro_call_index.exe" 1 $null "bad" "bad"

Invoke-Translate "tests\l1\repro_call_binop.lm1" "$obj\repro_call_binop.c"
Invoke-Translate "tests\l1\repro_call_binop.lm1" "$obj\repro_call_binop_b.c"
Assert-ByteIdentical "$obj\repro_call_binop.c" "$obj\repro_call_binop_b.c"
Assert-CHas "$obj\repro_call_binop.c" "char buffer[32];"
Assert-CHas "$obj\repro_call_binop.c" "memcpy(buffer + 0,"
Assert-CLacks "$obj\repro_call_binop.c" "buffer[] = {32}"
Invoke-CcRun "$obj\repro_call_binop.c" "$bin\repro_call_binop.exe" 0 $null

Write-Log "smoke ok"
Write-Output "l1trans $gen smoke ok"
} finally {
    Restore-HostedRegistryEnv
}
