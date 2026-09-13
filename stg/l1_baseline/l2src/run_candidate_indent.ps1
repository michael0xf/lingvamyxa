# Candidate printTree: stripped parser.lm1 copy + L2 indent adapters.
# Frozen l1src/parser.lm1 and live gen2 binaries are not written.
param(
    [Parameter(Mandatory = $true)][string]$l1trans,
    [Parameter(Mandatory = $true)][string]$out,
    [Parameter(Mandatory = $true)][string]$log,
    [Parameter(Mandatory = $true)][string[]]$messageObjects
)
$ErrorActionPreference = "Stop"
$guards = @(
    "-Werror=incompatible-pointer-types", "-Werror=discarded-qualifiers",
    "-Werror=implicit-function-declaration", "-Werror=implicit-int"
)
$cflags = @("-std=c99", "-Wall", "-Wextra", "-Wpedantic", "-I", ".", "-I", "lm1/build", "-I", (Join-Path $out "message_support/headers")) + $guards
$messageLink = (($messageObjects | ForEach-Object { '"' + $_ + '"' }) -join ' ')

function Invoke-CandGcc([string]$cpath, [string]$exe, [string]$glog) {
    $flagStr = ($cflags -join " ")
    cmd /c "gcc $flagStr `"$cpath`" -o `"$exe`" > `"$glog`" 2>&1"
    if ($LASTEXITCODE -ne 0) {
        Get-Content $glog
        throw "gcc failed: $cpath"
    }
}

$replaced = @(
    "lm_p0_indent_stack_free",
    "lm_p0_indent_stack_free_any",
    "lm_p0_indent_stack_push",
    "lm_p0_indent_stack_init",
    "lm_p0_indent_stack_new_empty",
    "lm_p0_indent_stack_new",
    "lm_p0_indent_stack_delete",
    "lm_p0_indent_stack_copy",
    "lm_p0_indent_stack_clone",
    "lm_p0_indent_level_from_column",
    "lm_p0_scan_layout_prefix",
    "lm_p0_scan_registry_compact_atom_piece",
    "lm_p0_scan_c_quoted_token",
    "lm_p0_starts_c_prefixed_quote",
    "lm_p0_scan_c_char_token",
    "lm_p0_scan_c_prefixed_quote_token",
    "lm_p0_find_python_string_end",
    "lm_p0_skip_python_string_unchecked"
)

$frozenParser = Join-Path (Get-Location) "l1src\parser.lm1"
$srcParser = [System.IO.File]::ReadAllText($frozenParser).Replace("`r`n", "`n")
$frozenHash = (Get-FileHash -Algorithm SHA256 $frozenParser).Hash
$headParser = (git -C (Join-Path (Get-Location) "..\..") rev-parse "HEAD:stg/l1_baseline/l1src/parser.lm1").Trim()
$workParser = (git -C (Join-Path (Get-Location) "..\..") hash-object "stg/l1_baseline/l1src/parser.lm1").Trim()
if ($headParser -ne $workParser) { throw "frozen REF l1src/parser.lm1 is dirty; refuse to strip a moving original" }
foreach ($name in $replaced) {
    $all = [regex]::Matches($srcParser, "(?m)^(sub|fn): $([regex]::Escape($name))\b")
    if ($all.Count -eq 0) { throw "strip $name : definition start not found" }
    if ($all.Count -ne 1) { throw "strip $name : expected 1 definition, got $($all.Count)" }
    $startPat = "(?m)^(sub|fn): $([regex]::Escape($name))\b"
    $sm = [regex]::Match($srcParser, $startPat)
    if (-not $sm.Success) { throw "strip $name : definition start not found" }
    $rest = $srcParser.Substring($sm.Index)
    $endPat = "(?m)^end: $([regex]::Escape($name))\n"
    $em = [regex]::Match($rest, $endPat)
    $len = 0
    if ($em.Success) {
        $len = $em.Index + $em.Length
    } else {
        $nm = [regex]::Match($rest, "(?m)\n(?:fn|sub): ")
        if (-not $nm.Success) { throw "strip $name : no end: and no following fn/sub" }
        $len = $nm.Index + 1
    }
    $srcParser = $srcParser.Remove($sm.Index, $len)
}
foreach ($name in $replaced) {
    if ($srcParser -match "(?m)^(sub|fn): $([regex]::Escape($name))\b") {
        throw "strip $name : definition survived"
    }
    if ($srcParser -notmatch "(?m)^    (sub|fn): $([regex]::Escape($name))\b") {
        throw "strip $name : prototype missing after strip"
    }
}

$candParser = Join-Path $out "parser_candidate.lm1"
[System.IO.File]::WriteAllText((Join-Path (Get-Location) $candParser), $srcParser)

$pt = [System.IO.File]::ReadAllText((Join-Path (Get-Location) "l1src\printTree.lm1")).Replace("`r`n", "`n")
if ($pt -notmatch 'predef: "l1src/parser.lm1"') { throw "printTree.lm1 missing parser predef" }
$pt = $pt.Replace('predef: "l1src/parser.lm1"', 'predef: "build/l2trans/parser_candidate.lm1"')
$candPtLm1 = Join-Path $out "candidate_printTree.lm1"
[System.IO.File]::WriteAllText((Join-Path (Get-Location) $candPtLm1), $pt)

$candPtC = Join-Path $out "candidate_printTree.c"
& $l1trans $candPtLm1 $candPtC
if ($LASTEXITCODE -ne 0) { throw "l1trans failed candidate_printTree" }
$candCtext = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $candPtC)).Replace("`r`n", "`n")
foreach ($name in $replaced) {
    $defs = [regex]::Matches($candCtext, "(?m)^(?:void|int|size_t|LmP0IndentStack \*) $([regex]::Escape($name))\([^;{]*\)\s*\{")
    if ($defs.Count -ne 0) { throw "candidate C still defines $name ($($defs.Count))" }
    if ($candCtext -notmatch [regex]::Escape($name)) {
        throw "candidate C lost all mentions of $name (calls should remain)"
    }
}

$indentLm1 = Join-Path $out "parser_indent_stack.lm1"
if (-not (Test-Path -LiteralPath $indentLm1)) { throw "missing $indentLm1 (run Invoke-IndentStack first)" }
$ilm1 = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $indentLm1)).Replace("`r`n", "`n")
if ($ilm1 -notmatch '(?m)^external:') { throw "indent lm1 missing external" }
if ($ilm1 -notmatch '@: Lmx unit 0') { throw "indent lm1 missing unit local" }
$ilm1 = [regex]::Replace($ilm1, '(?<![A-Za-z0-9_])unit(?![A-Za-z0-9_])', 'l2_indent_unit')
$ilm1 = $ilm1.Replace("        @: Lmx l2_indent_unit 0`n", "")
$ilm1 = $ilm1.Replace("        c.lmx_msg_runtime_delete(process_runtime)`n", "")
$ilm1 = $ilm1.Replace("c.lmx_msg_poll_escape()", "0")
$eidx = $ilm1.IndexOf("`nexternal:")
if ($eidx -lt 0) { throw "indent lm1 hoist: external not found" }
$ilm1 = $ilm1.Insert($eidx + 1, "@: Lmx l2_indent_unit 0`n`n")
$bootLm1 = Join-Path $out "indent_stack_boot.lm1"
[System.IO.File]::WriteAllText((Join-Path (Get-Location) $bootLm1), $ilm1)
$bootC = Join-Path $out "indent_stack_boot.c"
& $l1trans $bootLm1 $bootC
if ($LASTEXITCODE -ne 0) { throw "l1trans failed indent_stack_boot" }

$bootO = Join-Path $out "indent_stack_boot.o"
$bootLog = Join-Path $log "indent_stack_boot.gcc.log"
$flagStr = ($cflags -join " ")
cmd /c "gcc $flagStr -Dmain=l2_indent_boot -c `"$bootC`" -o `"$bootO`" > `"$bootLog`" 2>&1"
if ($LASTEXITCODE -ne 0) { Get-Content $bootLog; throw "gcc failed indent_stack_boot.o" }

$nm = "nm"
$nmOut = & $nm --defined-only $bootO 2>&1 | Out-String
if ($LASTEXITCODE -ne 0) { throw "nm failed on indent_stack_boot.o" }
$keep = New-Object System.Collections.Generic.List[string]
foreach ($line in ($nmOut -split "`n")) {
    if ($line -match '\s([A-TV-Z])\s+(\S+)\s*$') {
        $sym = $Matches[2]
        if ($sym -like 'l2_*' -or $sym -like 'lmx_*' -or $sym -eq 'l2_indent_boot' -or $sym -eq 'l2_indent_unit') {
            [void]$keep.Add($sym)
        }
    }
}
if ($keep -notcontains 'l2_m10') { throw "indent_stack_boot.o missing l2_m10" }
if ($keep -notcontains 'l2_indent_boot') { throw "indent_stack_boot.o missing l2_indent_boot" }
$exp = Join-Path $out "indent_stack_exports.txt"
[System.IO.File]::WriteAllLines((Join-Path (Get-Location) $exp), ($keep | Select-Object -Unique))
$bootKeep = Join-Path $out "indent_stack_boot.keep.o"
cmd /c "objcopy --keep-global-symbols=`"$exp`" `"$bootO`" `"$bootKeep`" > `"$(Join-Path $log 'indent_stack_objcopy.log')`" 2>&1"
if ($LASTEXITCODE -ne 0) { throw "objcopy --keep-global-symbols failed" }

$abiC = "l2src\indent_stack_abi.c"
$abiO = Join-Path $out "indent_stack_abi.o"
$abiLog = Join-Path $log "indent_stack_abi.gcc.log"
cmd /c "gcc $flagStr -c `"$abiC`" -o `"$abiO`" > `"$abiLog`" 2>&1"
if ($LASTEXITCODE -ne 0) { Get-Content $abiLog; throw "gcc failed indent_stack_abi.o" }

$ptO = Join-Path $out "candidate_printTree.o"
$ptLog = Join-Path $log "candidate_printTree.gcc.log"
cmd /c "gcc $flagStr -c `"$candPtC`" -o `"$ptO`" > `"$ptLog`" 2>&1"
if ($LASTEXITCODE -ne 0) { Get-Content $ptLog; throw "gcc failed candidate_printTree.o" }

$candExe = Join-Path $out "candidate_printTree.exe"
$linkLog = Join-Path $log "candidate_printTree.link.log"
$layLm1 = Join-Path $out "parser_scan_layout_prefix.lm1"
if (-not (Test-Path -LiteralPath $layLm1)) { throw "missing $layLm1 (run Invoke-ScanLayoutPrefix first)" }
$llm1 = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $layLm1)).Replace("`r`n", "`n")
if ($llm1 -notmatch '(?m)^external:') { throw "layout lm1 missing external" }
if ($llm1 -notmatch '@: Lmx unit 0') { throw "layout lm1 missing unit local" }
$llm1 = [regex]::Replace($llm1, '(?<![A-Za-z0-9_])unit(?![A-Za-z0-9_])', 'l2_layout_unit')
$llm1 = $llm1.Replace("        @: Lmx l2_layout_unit 0`n", "")
$llm1 = $llm1.Replace("        c.lmx_msg_runtime_delete(process_runtime)`n", "")
$llm1 = $llm1.Replace("c.lmx_msg_poll_escape()", "0")
$leidx = $llm1.IndexOf("`nexternal:")
if ($leidx -lt 0) { throw "layout lm1 hoist: external not found" }
$llm1 = $llm1.Insert($leidx + 1, "@: Lmx l2_layout_unit 0`n`n")
$layBootLm1 = Join-Path $out "layout_prefix_boot.lm1"
[System.IO.File]::WriteAllText((Join-Path (Get-Location) $layBootLm1), $llm1)
$layBootC = Join-Path $out "layout_prefix_boot.c"
& $l1trans $layBootLm1 $layBootC
if ($LASTEXITCODE -ne 0) { throw "l1trans failed layout_prefix_boot" }
$layBootO = Join-Path $out "layout_prefix_boot.o"
$layBootLog = Join-Path $log "layout_prefix_boot.gcc.log"
cmd /c "gcc $flagStr -Dmain=l2_layout_boot -c `"$layBootC`" -o `"$layBootO`" > `"$layBootLog`" 2>&1"
if ($LASTEXITCODE -ne 0) { Get-Content $layBootLog; throw "gcc failed layout_prefix_boot.o" }
$layRen = Join-Path $out "layout_prefix_boot.ren.o"
cmd /c "objcopy --redefine-sym l2_m3=l2_layout_m3 `"$layBootO`" `"$layRen`" > `"$(Join-Path $log 'layout_prefix_redef.log')`" 2>&1"
if ($LASTEXITCODE -ne 0) { throw "objcopy --redefine-sym l2_m3 failed" }
$layKeepList = @("l2_layout_boot", "l2_layout_unit", "l2_layout_m3")
$layExp = Join-Path $out "layout_prefix_exports.txt"
[System.IO.File]::WriteAllLines((Join-Path (Get-Location) $layExp), $layKeepList)
$layKeep = Join-Path $out "layout_prefix_boot.keep.o"
cmd /c "objcopy --keep-global-symbols=`"$layExp`" `"$layRen`" `"$layKeep`" > `"$(Join-Path $log 'layout_prefix_objcopy.log')`" 2>&1"
if ($LASTEXITCODE -ne 0) { throw "objcopy layout keep-global failed" }
$layNm = & $nm --defined-only $layKeep 2>&1 | Out-String
if ($layNm -notmatch 'l2_layout_m3') { throw "layout keep.o missing l2_layout_m3" }
if ($layNm -cmatch '(?m)\sT\s+l2_m\d+\s*$') { throw "layout keep.o still exports raw l2_m*" }

$layAbiC = "l2src\layout_prefix_abi.c"
$layAbiO = Join-Path $out "layout_prefix_abi.o"
$layAbiLog = Join-Path $log "layout_prefix_abi.gcc.log"
cmd /c "gcc $flagStr -c `"$layAbiC`" -o `"$layAbiO`" > `"$layAbiLog`" 2>&1"
if ($LASTEXITCODE -ne 0) { Get-Content $layAbiLog; throw "gcc failed layout_prefix_abi.o" }

$regLm1 = Join-Path $out "parser_registry_compact.lm1"
if (-not (Test-Path -LiteralPath $regLm1)) { throw "missing $regLm1 (translate parser_registry_compact.lm2 first)" }
$rlm1 = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $regLm1)).Replace("`r`n", "`n")
if ($rlm1 -notmatch '(?m)^external:' -or $rlm1 -notmatch '@: Lmx unit 0') { throw "registry compact lm1 missing generated entry" }
$rlm1 = [regex]::Replace($rlm1, '(?<![A-Za-z0-9_])unit(?![A-Za-z0-9_])', 'l2_registry_unit')
$rlm1 = $rlm1.Replace("        @: Lmx l2_registry_unit 0`n", "")
$rlm1 = $rlm1.Replace("        c.lmx_msg_runtime_delete(process_runtime)`n", "")
$rlm1 = $rlm1.Replace("c.lmx_msg_poll_escape()", "0")
$reidx = $rlm1.IndexOf("`nexternal:")
if ($reidx -lt 0) { throw "registry compact hoist: external not found" }
$rlm1 = $rlm1.Insert($reidx + 1, "@: Lmx l2_registry_unit 0`n`n")
$regBootLm1 = Join-Path $out "registry_compact_boot.lm1"
[System.IO.File]::WriteAllText((Join-Path (Get-Location) $regBootLm1), $rlm1)
$regBootC = Join-Path $out "registry_compact_boot.c"
& $l1trans $regBootLm1 $regBootC
if ($LASTEXITCODE -ne 0) { throw "l1trans failed registry_compact_boot" }
$regBootO = Join-Path $out "registry_compact_boot.o"
$regBootLog = Join-Path $log "registry_compact_boot.gcc.log"
cmd /c "gcc $flagStr -Dmain=l2_registry_boot -c `"$regBootC`" -o `"$regBootO`" > `"$regBootLog`" 2>&1"
if ($LASTEXITCODE -ne 0) { Get-Content $regBootLog; throw "gcc failed registry_compact_boot.o" }
$regRen = Join-Path $out "registry_compact_boot.ren.o"
cmd /c "objcopy --redefine-sym l2_m0=l2_registry_m0 `"$regBootO`" `"$regRen`" > `"$(Join-Path $log 'registry_compact_redef.log')`" 2>&1"
if ($LASTEXITCODE -ne 0) { throw "objcopy --redefine-sym l2_m0 failed for registry compact" }
$regExp = Join-Path $out "registry_compact_exports.txt"
[System.IO.File]::WriteAllLines((Join-Path (Get-Location) $regExp), @("l2_registry_boot", "l2_registry_unit", "l2_registry_m0"))
$regKeep = Join-Path $out "registry_compact_boot.keep.o"
cmd /c "objcopy --keep-global-symbols=`"$regExp`" `"$regRen`" `"$regKeep`" > `"$(Join-Path $log 'registry_compact_objcopy.log')`" 2>&1"
if ($LASTEXITCODE -ne 0) { throw "objcopy registry compact keep-global failed" }
$regAbiC = "l2src\registry_compact_abi.c"
$regAbiO = Join-Path $out "registry_compact_abi.o"
$regAbiLog = Join-Path $log "registry_compact_abi.gcc.log"
cmd /c "gcc $flagStr -c `"$regAbiC`" -o `"$regAbiO`" > `"$regAbiLog`" 2>&1"
if ($LASTEXITCODE -ne 0) { Get-Content $regAbiLog; throw "gcc failed registry_compact_abi.o" }

$cqLm1 = Join-Path $out "parser_c_quoted.lm1"
if (-not (Test-Path -LiteralPath $cqLm1)) { throw "missing $cqLm1 (translate parser_c_quoted.lm2 first)" }
$cqlm1 = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $cqLm1)).Replace("`r`n", "`n")
if ($cqlm1 -notmatch '(?m)^external:' -or $cqlm1 -notmatch '@: Lmx unit 0') { throw "C quoted lm1 missing generated entry" }
$cqlm1 = [regex]::Replace($cqlm1, '(?<![A-Za-z0-9_])unit(?![A-Za-z0-9_])', 'l2_cquoted_unit')
$cqlm1 = $cqlm1.Replace("        @: Lmx l2_cquoted_unit 0`n", "")
$cqlm1 = $cqlm1.Replace("        c.lmx_msg_runtime_delete(process_runtime)`n", "")
$cqlm1 = $cqlm1.Replace("c.lmx_msg_poll_escape()", "0")
$cqidx = $cqlm1.IndexOf("`nexternal:")
if ($cqidx -lt 0) { throw "C quoted hoist: external not found" }
$cqlm1 = $cqlm1.Insert($cqidx + 1, "@: Lmx l2_cquoted_unit 0`n`n")
$cqBootLm1 = Join-Path $out "c_quoted_boot.lm1"
[System.IO.File]::WriteAllText((Join-Path (Get-Location) $cqBootLm1), $cqlm1)
$cqBootC = Join-Path $out "c_quoted_boot.c"
& $l1trans $cqBootLm1 $cqBootC
if ($LASTEXITCODE -ne 0) { throw "l1trans failed c_quoted_boot" }
$cqBootO = Join-Path $out "c_quoted_boot.o"
$cqBootLog = Join-Path $log "c_quoted_boot.gcc.log"
cmd /c "gcc $flagStr -Dmain=l2_cquoted_boot -c `"$cqBootC`" -o `"$cqBootO`" > `"$cqBootLog`" 2>&1"
if ($LASTEXITCODE -ne 0) { Get-Content $cqBootLog; throw "gcc failed c_quoted_boot.o" }
$cqRen = $cqBootO
foreach ($i in 0..5) {
    $next = Join-Path $out ("c_quoted_boot.ren" + $i + ".o")
    cmd /c "objcopy --redefine-sym l2_m$i=l2_cquoted_m$i `"$cqRen`" `"$next`" > `"$(Join-Path $log ('c_quoted_redef_' + $i + '.log'))`" 2>&1"
    if ($LASTEXITCODE -ne 0) { throw "objcopy redefine C quoted l2_m$i failed" }
    $cqRen = $next
}
$cqExp = Join-Path $out "c_quoted_exports.txt"
[System.IO.File]::WriteAllLines((Join-Path (Get-Location) $cqExp), @("l2_cquoted_boot", "l2_cquoted_unit") + @(0..5 | ForEach-Object { "l2_cquoted_m$_" }))
$cqKeep = Join-Path $out "c_quoted_boot.keep.o"
cmd /c "objcopy --keep-global-symbols=`"$cqExp`" `"$cqRen`" `"$cqKeep`" > `"$(Join-Path $log 'c_quoted_objcopy.log')`" 2>&1"
if ($LASTEXITCODE -ne 0) { throw "objcopy C quoted keep-global failed" }
$cqAbiC = "l2src\c_quoted_abi.c"
$cqAbiO = Join-Path $out "c_quoted_abi.o"
$cqAbiLog = Join-Path $log "c_quoted_abi.gcc.log"
cmd /c "gcc $flagStr -c `"$cqAbiC`" -o `"$cqAbiO`" > `"$cqAbiLog`" 2>&1"
if ($LASTEXITCODE -ne 0) { Get-Content $cqAbiLog; throw "gcc failed c_quoted_abi.o" }

$pyLm1 = Join-Path $out "parser_python_string.lm1"
if (-not (Test-Path -LiteralPath $pyLm1)) { throw "missing $pyLm1 (translate parser_python_string.lm2 first)" }
$pylm1 = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $pyLm1)).Replace("`r`n", "`n")
if ($pylm1 -notmatch '(?m)^external:' -or $pylm1 -notmatch '@: Lmx unit 0') { throw "Python string lm1 missing generated entry" }
$pylm1 = [regex]::Replace($pylm1, '(?<![A-Za-z0-9_])unit(?![A-Za-z0-9_])', 'l2_pystr_unit')
$pylm1 = $pylm1.Replace("        @: Lmx l2_pystr_unit 0`n", "")
$pylm1 = $pylm1.Replace("        c.lmx_msg_runtime_delete(process_runtime)`n", "")
$pylm1 = $pylm1.Replace("c.lmx_msg_poll_escape()", "0")
$pyidx = $pylm1.IndexOf("`nexternal:")
if ($pyidx -lt 0) { throw "Python string hoist: external not found" }
$pylm1 = $pylm1.Insert($pyidx + 1, "@: Lmx l2_pystr_unit 0`n`n")
$pyBootLm1 = Join-Path $out "python_string_boot.lm1"
[System.IO.File]::WriteAllText((Join-Path (Get-Location) $pyBootLm1), $pylm1)
$pyBootC = Join-Path $out "python_string_boot.c"
& $l1trans $pyBootLm1 $pyBootC
if ($LASTEXITCODE -ne 0) { throw "l1trans failed python_string_boot" }
$pyBootO = Join-Path $out "python_string_boot.o"
$pyBootLog = Join-Path $log "python_string_boot.gcc.log"
cmd /c "gcc $flagStr -Dmain=l2_pystr_boot -c `"$pyBootC`" -o `"$pyBootO`" > `"$pyBootLog`" 2>&1"
if ($LASTEXITCODE -ne 0) { Get-Content $pyBootLog; throw "gcc failed python_string_boot.o" }
$pyRen = $pyBootO
foreach ($i in 0..3) {
    $next = Join-Path $out ("python_string_boot.ren" + $i + ".o")
    cmd /c "objcopy --redefine-sym l2_m$i=l2_pystr_m$i `"$pyRen`" `"$next`" > `"$(Join-Path $log ('python_string_redef_' + $i + '.log'))`" 2>&1"
    if ($LASTEXITCODE -ne 0) { throw "objcopy redefine Python string l2_m$i failed" }
    $pyRen = $next
}
$pyExp = Join-Path $out "python_string_exports.txt"
[System.IO.File]::WriteAllLines((Join-Path (Get-Location) $pyExp), @("l2_pystr_boot", "l2_pystr_unit") + @(0..3 | ForEach-Object { "l2_pystr_m$_" }))
$pyKeep = Join-Path $out "python_string_boot.keep.o"
cmd /c "objcopy --keep-global-symbols=`"$pyExp`" `"$pyRen`" `"$pyKeep`" > `"$(Join-Path $log 'python_string_objcopy.log')`" 2>&1"
if ($LASTEXITCODE -ne 0) { throw "objcopy Python string keep-global failed" }
$pyAbiC = "l2src\python_string_abi.c"
$pyAbiO = Join-Path $out "python_string_abi.o"
$pyAbiLog = Join-Path $log "python_string_abi.gcc.log"
cmd /c "gcc $flagStr -c `"$pyAbiC`" -o `"$pyAbiO`" > `"$pyAbiLog`" 2>&1"
if ($LASTEXITCODE -ne 0) { Get-Content $pyAbiLog; throw "gcc failed python_string_abi.o" }

cmd /c "gcc $flagStr `"$ptO`" `"$bootKeep`" `"$abiO`" `"$layKeep`" `"$layAbiO`" `"$regKeep`" `"$regAbiO`" `"$cqKeep`" `"$cqAbiO`" `"$pyKeep`" `"$pyAbiO`" $messageLink -o `"$candExe`" > `"$linkLog`" 2>&1"
if ($LASTEXITCODE -ne 0) { Get-Content $linkLog; throw "link failed candidate_printTree" }

$ptNoMain = Join-Path $out "candidate_parser_nomain.o"
$ptNoLog = Join-Path $log "candidate_parser_nomain.gcc.log"
cmd /c "gcc $flagStr -Dmain=candidate_printTree_unused -c `"$candPtC`" -o `"$ptNoMain`" > `"$ptNoLog`" 2>&1"
if ($LASTEXITCODE -ne 0) { Get-Content $ptNoLog; throw "gcc failed candidate_parser_nomain.o" }
$probeC = "l2src\indent_parse_probe.c"
$probeExe = Join-Path $out "indent_parse_probe.exe"
$probeLog = Join-Path $log "indent_parse_probe.gcc.log"
cmd /c "gcc $flagStr `"$probeC`" `"$ptNoMain`" `"$bootKeep`" `"$abiO`" `"$layKeep`" `"$layAbiO`" `"$regKeep`" `"$regAbiO`" `"$cqKeep`" `"$cqAbiO`" `"$pyKeep`" `"$pyAbiO`" $messageLink -o `"$probeExe`" > `"$probeLog`" 2>&1"
if ($LASTEXITCODE -ne 0) { Get-Content $probeLog; throw "link failed indent_parse_probe" }
$probeOut = Join-Path $out "indent_parse_probe.stdout"
cmd /c "`"$probeExe`" > `"$probeOut`" 2> `"$(Join-Path $out 'indent_parse_probe.err')`""
if ($LASTEXITCODE -ne 0) { Get-Content (Join-Path $out "indent_parse_probe.err"); throw "indent_parse_probe failed" }
$probeText = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $probeOut)).Replace("`r`n", "`n").Trim()
if ($probeText -notmatch '^parse=0 indent_hits=[1-9][0-9]* layout_hits=[1-9][0-9]* registry_hits=[1-9][0-9]* cquoted_hits=[1-9][0-9]* pystr_hits=[1-9][0-9]*$') { throw "parse_bytes did not reach all L2 parser helpers: $probeText" }

$candHash = (Get-FileHash -Algorithm SHA256 (Join-Path (Get-Location) $candExe)).Hash
$stgPt = "build\l1trans\gen2\printTree.exe"
if (-not (Test-Path -LiteralPath $stgPt)) { throw "missing STG gen2 printTree $stgPt" }
$stgHash = (Get-FileHash -Algorithm SHA256 (Join-Path (Get-Location) $stgPt)).Hash
$lm0 = (Join-Path (Get-Location) "..\..\build\lm0\printTree.lm0.exe")
if (-not (Test-Path -LiteralPath $lm0)) { $lm0 = "C:\Nyasha_Planet\lingvamyxa\build\lm0\printTree.lm0.exe" }
$lm0Hash = "missing"
if (Test-Path -LiteralPath $lm0) { $lm0Hash = (Get-FileHash -Algorithm SHA256 $lm0).Hash }

$pinnedRev = "620db8612c32569c8dd507cca135d5d076144e9f"
$pinnedWant = "CB564AD6FF52F35E918FFBAE6A6E2E166147DADCAAE445F9D5A2526A77801EF3"
$pinnedExe = "C:\Nyasha_Planet\lingvamyxa_old_worked_version\build\p0_tree_contract\printTree_620db86.exe"
if (-not (Test-Path -LiteralPath $pinnedExe)) { throw "missing pinned 620 parser $pinnedExe" }
$pinnedHash = (Get-FileHash -Algorithm SHA256 $pinnedExe).Hash
if ($pinnedHash -ne $pinnedWant) { throw "pinned 620 printTree hash $pinnedHash want $pinnedWant" }
if ($lm0Hash -eq $pinnedWant) { throw "build/lm0/printTree.lm0.exe unexpectedly equals pinned 620 hash; do not treat it as the oracle by convenience" }

$expectReject = @{
    "C_nested_short" = @{ Exit = 1; Diag = "P0 parse error 13 at 2:5: source level increase must be one step" }
    "return_colon_empty_trailer" = @{ Exit = 1; Diag = "P0 parse error 32 at 1:1: empty colon Frame is not allowed" }
    "return_colon_comment_trailer" = @{ Exit = 1; Diag = "P0 parse error 32 at 1:1: empty colon Frame is not allowed" }
    "F_star_fence" = @{ Exit = 1; Diag = "P0 parse error 32 at 1:1: empty colon Frame is not allowed" }
    "invalid_eq_unclosed" = @{ Exit = 1; Diag = "P0 parse error 20 at 2:1: unterminated block string literal" }
    "invalid_triple_unclosed" = @{ Exit = 1; Diag = "P0 parse error 4 at 1:4: unterminated python-like string literal" }
}
$emptyColon = @("return_colon_empty_trailer", "return_colon_comment_trailer", "F_star_fence")
$repo = (Resolve-Path (Join-Path (Get-Location) "..\..")).Path
$corpusDir = Join-Path $repo "tests\p0_tree_contract"
$corpus = @(Get-ChildItem -LiteralPath $corpusDir -Filter "*.lmx" | Sort-Object Name | ForEach-Object { Join-Path "tests\p0_tree_contract" $_.Name })
$extra = @("tests\arr.lmx", "tests\tail_cutters.lmx")
foreach ($e in $extra) {
    if (Test-Path -LiteralPath (Join-Path $repo $e)) { $corpus += $e }
}

function Invoke-DumpExe([string]$exe, [string]$src, [string]$stdout, [string]$stderr) {
    $exeFull = $exe
    if (-not [IO.Path]::IsPathRooted($exeFull)) { $exeFull = Join-Path (Get-Location) $exe }
    $outFull = $stdout
    if (-not [IO.Path]::IsPathRooted($outFull)) { $outFull = Join-Path (Get-Location) $stdout }
    $errFull = $stderr
    if (-not [IO.Path]::IsPathRooted($errFull)) { $errFull = Join-Path (Get-Location) $stderr }
    $p = Start-Process -FilePath $exeFull -ArgumentList $src -NoNewWindow -Wait -PassThru -RedirectStandardOutput $outFull -RedirectStandardError $errFull
    return $p.ExitCode
}

function Read-Norm([string]$path) {
    $full = $path
    if (-not [IO.Path]::IsPathRooted($full)) { $full = Join-Path (Get-Location) $path }
    return [System.IO.File]::ReadAllText($full).Replace("`r`n", "`n")
}

$treeDir = Join-Path $out "candidate_trees"
New-Item -ItemType Directory -Force -Path $treeDir | Out-Null
$n = 0
$nReject = 0
$nMatch620 = 0
$nDeltaColon = 0
$nKnownTreeDelta = 0
foreach ($src in $corpus) {
    $rootSrc = Join-Path $repo $src
    if (-not (Test-Path -LiteralPath $rootSrc)) { throw "missing corpus $src at $rootSrc" }
    $stem = [IO.Path]::GetFileNameWithoutExtension($src)
    $cOut = Join-Path $treeDir ($stem + ".cand.out")
    $cErr = Join-Path $treeDir ($stem + ".cand.err")
    $sOut = Join-Path $treeDir ($stem + ".stg.out")
    $sErr = Join-Path $treeDir ($stem + ".stg.err")
    $ecC = Invoke-DumpExe $candExe $rootSrc $cOut $cErr
    $ecS = Invoke-DumpExe $stgPt $rootSrc $sOut $sErr
    $n++
    if ($ecC -ne $ecS) { throw "$src candidate exit $ecC stg $ecS" }
    $a = Read-Norm $cOut
    $b = Read-Norm $sOut
    $ae = (Read-Norm $cErr).Trim()
    $be = (Read-Norm $sErr).Trim()
    if ($a -ne $b) { throw "$src tree mismatch vs STG gen2 printTree" }
    if ($ae -ne $be) { throw "$src stderr mismatch vs STG gen2 printTree`ncand=$ae`nstg=$be" }
    if ($expectReject.ContainsKey($stem)) {
        $spec = $expectReject[$stem]
        if ($ecC -ne $spec.Exit) { throw "$src candidate exit $ecC expected $($spec.Exit)" }
        if ($ae -ne $spec.Diag) { throw "$src diag got=$ae want=$($spec.Diag)" }
        $nReject++
        if ($emptyColon -contains $stem) { $nDeltaColon++ }
        continue
    }
    if ($ecC -ne 0) { throw "$src unexpected reject $ecC $ae" }
    $gold = Join-Path (Split-Path -Parent $rootSrc) ($stem + ".tree.txt")
    if (Test-Path -LiteralPath $gold) {
        $g = (Read-Norm $gold).TrimEnd()
        $at = $a.TrimEnd()
        $bt = $b.TrimEnd()
        if ($bt -eq $g) {
            if ($at -ne $g) { throw "$src tree mismatch vs 620db86 golden that STG gen2 still matches (not absorbing)" }
            $nMatch620++
        } else {
            if ($at -ne $bt) { throw "$src new tree delta vs STG while STG already differs from 620 golden" }
            $nKnownTreeDelta++
        }
    }
    if ($src.StartsWith("tests\p0_tree_contract\")) {
        $pOut = Join-Path $treeDir ($stem + ".620.out")
        $pErr = Join-Path $treeDir ($stem + ".620.err")
        $ecP = Invoke-DumpExe $pinnedExe $rootSrc $pOut $pErr
        if (Test-Path -LiteralPath $gold) {
            $g = (Read-Norm $gold).TrimEnd()
            $ptxt = (Read-Norm $pOut).TrimEnd()
            if ($ecP -eq 0 -and $ptxt -ne $g) { throw "$src pinned 620 exe stdout != committed golden (do not regenerate)" }
        }
    }
}

$id = Join-Path $out "candidate_indent_id.txt"
@(
    "candidate_exe=$candExe"
    "candidate_sha256=$candHash"
    "stg_printTree=$stgPt"
    "stg_printTree_sha256=$stgHash"
    "lm0_printTree=$lm0"
    "lm0_printTree_sha256=$lm0Hash"
    "lm0_note=checkout dump client; not the pinned 620db86 oracle"
    "pinned_rev=$pinnedRev"
    "pinned_exe=$pinnedExe"
    "pinned_sha256=$pinnedHash"
    "frozen_parser_sha256=$frozenHash"
    "frozen_parser_git=$workParser"
    "probe=$probeText"
    "replaced=$($replaced -join ',')"
    "sources=l2src/parser_indent_stack.lm2; l2src/parser_scan_layout_prefix.lm2; l2src/parser_registry_compact.lm2; l2src/parser_c_quoted.lm2; l2src/parser_python_string.lm2; l2src/indent_stack_abi.c; l2src/layout_prefix_abi.c; l2src/registry_compact_abi.c; l2src/c_quoted_abi.c; l2src/python_string_abi.c; l2src/indent_parse_probe.c; build/l2trans/parser_candidate.lm1 (stripped copy of l1src/parser.lm1); l1src/printTree.lm1"
    "next_l1=lm_p0_parse_bytes/parse_file still L1 in the candidate TU; next unit document_init/scan remaining field-loop helpers"
    "corpus_total=$n accept=$($n - $nReject) expected_reject=$nReject empty_colon_delta=$nDeltaColon same_as_620_reject=$($nReject - $nDeltaColon) match620_accept=$nMatch620 known_tree_delta_vs_620=$nKnownTreeDelta extra_no_golden=$($n - $nReject - $nMatch620)"
) | Set-Content -LiteralPath (Join-Path (Get-Location) $id) -Encoding utf8
