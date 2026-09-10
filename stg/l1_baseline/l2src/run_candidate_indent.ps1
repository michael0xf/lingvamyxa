# Candidate printTree: stripped parser.lm1 copy + L2 indent adapters.
# Frozen l1src/parser.lm1 and live gen2 binaries are not written.
param(
    [Parameter(Mandatory = $true)][string]$l1trans,
    [Parameter(Mandatory = $true)][string]$out,
    [Parameter(Mandatory = $true)][string]$log
)
$ErrorActionPreference = "Stop"
$guards = @(
    "-Werror=incompatible-pointer-types", "-Werror=discarded-qualifiers",
    "-Werror=implicit-function-declaration", "-Werror=implicit-int"
)
$cflags = @("-std=c99", "-Wall", "-Wextra", "-Wpedantic", "-I", ".", "-I", "lm1/build") + $guards

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
    "lm_p0_scan_layout_prefix"
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
$ilm1 = $ilm1.Replace("external:`n    fn: main () int`n        @: Lmx unit 0`n", "@: Lmx l2_indent_unit 0`n`nexternal:`n    fn: main () int`n")
$eidx = $ilm1.LastIndexOf("`nexternal:")
if ($eidx -lt 0) { throw "indent lm1 hoist: external not found" }
$head = $ilm1.Substring(0, $eidx)
$tail = $ilm1.Substring($eidx)
$tail = [regex]::Replace($tail, '(?<![A-Za-z0-9_])unit(?![A-Za-z0-9_])', 'l2_indent_unit')
$bootLm1 = Join-Path $out "indent_stack_boot.lm1"
[System.IO.File]::WriteAllText((Join-Path (Get-Location) $bootLm1), ($head + $tail))
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
$llm1 = $llm1.Replace("external:`n    fn: main () int`n        @: Lmx unit 0`n", "@: Lmx l2_layout_unit 0`n`nexternal:`n    fn: main () int`n")
$leidx = $llm1.LastIndexOf("`nexternal:")
if ($leidx -lt 0) { throw "layout lm1 hoist: external not found" }
$lhead = $llm1.Substring(0, $leidx)
$ltail = $llm1.Substring($leidx)
$ltail = [regex]::Replace($ltail, '(?<![A-Za-z0-9_])unit(?![A-Za-z0-9_])', 'l2_layout_unit')
$layBootLm1 = Join-Path $out "layout_prefix_boot.lm1"
[System.IO.File]::WriteAllText((Join-Path (Get-Location) $layBootLm1), ($lhead + $ltail))
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

cmd /c "gcc $flagStr `"$ptO`" `"$bootKeep`" `"$abiO`" `"$layKeep`" `"$layAbiO`" -o `"$candExe`" > `"$linkLog`" 2>&1"
if ($LASTEXITCODE -ne 0) { Get-Content $linkLog; throw "link failed candidate_printTree" }

$ptNoMain = Join-Path $out "candidate_parser_nomain.o"
$ptNoLog = Join-Path $log "candidate_parser_nomain.gcc.log"
cmd /c "gcc $flagStr -Dmain=candidate_printTree_unused -c `"$candPtC`" -o `"$ptNoMain`" > `"$ptNoLog`" 2>&1"
if ($LASTEXITCODE -ne 0) { Get-Content $ptNoLog; throw "gcc failed candidate_parser_nomain.o" }
$probeC = "l2src\indent_parse_probe.c"
$probeExe = Join-Path $out "indent_parse_probe.exe"
$probeLog = Join-Path $log "indent_parse_probe.gcc.log"
cmd /c "gcc $flagStr `"$probeC`" `"$ptNoMain`" `"$bootKeep`" `"$abiO`" `"$layKeep`" `"$layAbiO`" -o `"$probeExe`" > `"$probeLog`" 2>&1"
if ($LASTEXITCODE -ne 0) { Get-Content $probeLog; throw "link failed indent_parse_probe" }
$probeOut = Join-Path $out "indent_parse_probe.stdout"
cmd /c "`"$probeExe`" > `"$probeOut`" 2> `"$(Join-Path $out 'indent_parse_probe.err')`""
if ($LASTEXITCODE -ne 0) { Get-Content (Join-Path $out "indent_parse_probe.err"); throw "indent_parse_probe failed" }
$probeText = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $probeOut)).Replace("`r`n", "`n").Trim()
if ($probeText -notmatch '^parse=0 indent_hits=[1-9][0-9]* layout_hits=[1-9]') { throw "parse_bytes did not reach L2 indent/layout: $probeText" }

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
    "sources=l2src/parser_indent_stack.lm2; l2src/parser_scan_layout_prefix.lm2; l2src/indent_stack_abi.c; l2src/layout_prefix_abi.c; l2src/indent_parse_probe.c; build/l2trans/parser_candidate.lm1 (stripped copy of l1src/parser.lm1); l1src/printTree.lm1"
    "next_l1=lm_p0_parse_bytes/parse_file still L1 in the candidate TU; next unit document_init/scan remaining field-loop helpers"
    "corpus_total=$n accept=$($n - $nReject) expected_reject=$nReject empty_colon_delta=$nDeltaColon same_as_620_reject=$($nReject - $nDeltaColon) match620_accept=$nMatch620 known_tree_delta_vs_620=$nKnownTreeDelta extra_no_golden=$($n - $nReject - $nMatch620)"
) | Set-Content -LiteralPath (Join-Path (Get-Location) $id) -Encoding utf8
