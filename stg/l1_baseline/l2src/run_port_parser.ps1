# run_port_parser.ps1 -- parser-in-L2 port acceptance gate.
#
# l1src/parser.lm1 (the oracle) is never edited. For each landed stage
# this builds two p0_meta_dump executables against the SAME goldens:
#   Ref  -- the pristine, tracked lm1/build/parser.lm1.c, untouched.
#   Port -- a disposable copy of that same generated C with the
#           stage's ported functions' definitions renamed aside
#           (name -> name__l1, dead code) and every internal call to
#           them redirected, via -D<name>=p0_<suffix>, to the L2
#           unit's own p0_* implementation. Internal cross-calls
#           inside the oracle (e.g. is_field_space calling
#           is_horizontal_space) get redirected the same way, since
#           the macro applies to the whole translation unit.
#
# Mechanism proof and rationale: reported to lingvamyxa-e2, 2026-09-14
# (objcopy --weaken-symbol does not override on this PE/COFF
# toolchain -- both definitions survive as separate symbols; this
# rename+macro approach does, confirmed by a positive falsifier -- the
# L2 side's printf fires at internal oracle call sites -- and a
# negative control -- the unwired pristine build gives an identical
# result with no override calls).
#
$ErrorActionPreference = "Stop"
Set-Location (Join-Path $PSScriptRoot "..")   # -> stg/l1_baseline

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..\..\..")
$goldenDir = Join-Path $repoRoot "tests\p0_tree_contract"
$dumpSrc = Join-Path $goldenDir "p0_meta_dump.c"
if (-not (Test-Path $dumpSrc)) { throw "missing $dumpSrc" }

$pin = (Get-Content (Join-Path $PSScriptRoot "L1_PIN.txt")).Trim()
$l1trans = Join-Path (Get-Location) "build\l1trans\gen2\l1trans.exe"
if (-not (Test-Path $l1trans)) { throw "missing pinned l1trans: $l1trans" }
$gotPin = (Get-FileHash -LiteralPath $l1trans -Algorithm SHA256).Hash
if ($gotPin -ne $pin) { throw "pinned l1trans hash mismatch: got $gotPin want $pin" }

$out = "build\port_parser"
$log = Join-Path $out "log"
New-Item -ItemType Directory -Force -Path $out | Out-Null
New-Item -ItemType Directory -Force -Path $log | Out-Null

function Invoke-Gcc([string[]]$GccArgs, [string]$LogPath) {
    $argStr = ($GccArgs | ForEach-Object { "`"$_`"" }) -join " "
    cmd /c "gcc $argStr > `"$LogPath`" 2>&1"
    if ($LASTEXITCODE -ne 0) {
        Get-Content $LogPath | Select-Object -Last 60
        throw "gcc failed (see $LogPath)"
    }
}

# ---- l2trans.exe (built fresh each run from l2src/l2trans.lm1) ----
$l2c = Join-Path $out "l2trans.c"
$l2exe = Join-Path $out "l2trans.exe"
cmd /c "`"$l1trans`" l2src\l2trans.lm1 `"$l2c`" > `"$(Join-Path $log 'l2trans_self.log')`" 2>&1"
if ($LASTEXITCODE -ne 0) { throw "l1trans failed translating l2trans.lm1" }
Invoke-Gcc @("-std=c99", "-w", "-DLM_THREAD_PROVIDER=LM_THREAD_PROVIDER_SINGLE", "-I", ".", "-I", "lm1\build", "-o", $l2exe, $l2c) (Join-Path $log "l2trans_self.gcc.log")

# ---- runtime-trio: 18 generated L2 headers + 19 lmx_* + 2 message host/exec ----
$rtHeaderRoot = Join-Path $out "l2rt_headers"
$rtHeaderDir = Join-Path $rtHeaderRoot "l2src"
New-Item -ItemType Directory -Force -Path $rtHeaderDir | Out-Null
$rtHeaderNames = @(
    "lmx_array_owned", "lmx_array_ref_owned", "lmx_branch_owned", "lmx_chars_owned",
    "lmx_graph_copy_owned", "lmx_message_graph_copy", "lmx_msg_blocks",
    "lmx_msg_history_owned", "lmx_msg_liveness", "lmx_msg_mail_chain",
    "lmx_msg_path_storage", "lmx_msg_roots_stale", "lmx_msg_sched_ready",
    "lmx_msg_slots", "lmx_msg_storage", "lmx_msg_visit", "lmx_owned_ranges",
    "lmx_value_owned"
)
foreach ($n in $rtHeaderNames) {
    $hdrOut = Join-Path $rtHeaderDir "$n.lm1.h"
    if (-not (Test-Path $hdrOut)) {
        cmd /c "`"$l1trans`" l2src\$n.h.lm1 `"$hdrOut`" > `"$(Join-Path $log "hdr_$n.log")`" 2>&1"
        if ($LASTEXITCODE -ne 0) { throw "l1trans failed building header $n" }
    }
}
$rtObjDir = Join-Path $out "l2rt_objs"
New-Item -ItemType Directory -Force -Path $rtObjDir | Out-Null
$rtModuleNames = $rtHeaderNames + @("lmx_message")
$rtObjs = @()
foreach ($n in $rtModuleNames) {
    $objOut = Join-Path $rtObjDir "$n.o"
    $rtObjs += $objOut
    if (Test-Path $objOut) { continue }
    $cOut = Join-Path $rtObjDir "$n.c"
    cmd /c "`"$l1trans`" l2src\$n.lm1 `"$cOut`" > `"$(Join-Path $log "trans_$n.log")`" 2>&1"
    if ($LASTEXITCODE -ne 0) { throw "l1trans failed translating $n.lm1" }
    Invoke-Gcc @("-std=c99", "-w", "-I", $rtHeaderRoot, "-I", (Get-Location), "-c", $cOut, "-o", $objOut) (Join-Path $log "compile_$n.log")
}
foreach ($n in @("lmx_message_host", "lmx_message_exec")) {
    $objOut = Join-Path $rtObjDir "$n.o"
    $rtObjs += $objOut
    if (Test-Path $objOut) { continue }
    Invoke-Gcc @("-std=c99", "-w", "-I", $rtHeaderRoot, "-I", (Get-Location), "-c", "l2src\$n.c", "-o", $objOut) (Join-Path $log "compile_$n.log")
}

# ---- Stage table ----
# Each stage is CUMULATIVE: Units and Funcs carry forward everything
# landed in earlier stages plus what's new, since a later unit's own
# calls into an earlier stage's p0_* functions (e.g. Stage b's
# p0_line_rest_is_horizontal_space calling Stage a's
# p0_is_horizontal_space) need that earlier unit's real object code
# linked in, and the oracle-side redirect set has to cover every
# ported name so no stale internal call reaches the un-redirected
# oracle body.
# Funcs: lm_p0_* oracle names this stage redirects to the L2 units'
# p0_* equivalents.
$Stages = @(
    @{
        Name = "a_parser_text"
        Units = @("l2src\parser_text_port.lm2")
        Headers = @("l2src\parser_text_port_l2.h.lm1")
        Funcs = @(
            "lm_p0_text_equals", "lm_p0_identifier_payload",
            "lm_p0_is_horizontal_space", "lm_p0_is_line_break", "lm_p0_line_break_width_at",
            "lm_p0_is_field_space", "lm_p0_is_field_separator", "lm_p0_is_short_form_separator",
            "lm_p0_is_quoted_token_boundary", "lm_p0_starts_python_string", "lm_p0_is_decimal_digit",
            "lm_p0_copy_bytes", "lm_p0_text_view_new_cstr", "lm_p0_text_view_delete", "lm_p0_text_from_cstr"
        )
        # All 15 of l1src/parser_text.lm1's functions. p0_text_equals and
        # p0_identifier_payload (const array-of-one struct formals) were
        # blocked by l2trans's "formal type 4" gap until d6's f29800c4.
    },
    @{
        Name = "b_parser_scan"
        Units = @("l2src\parser_text_port.lm2", "l2src\parser_scan_port.lm2")
        Headers = @("l2src\parser_text_port_l2.h.lm1", "l2src\parser_scan_port_l2.h.lm1")
        Funcs = @(
            "lm_p0_text_equals", "lm_p0_identifier_payload",
            "lm_p0_is_horizontal_space", "lm_p0_is_line_break", "lm_p0_line_break_width_at",
            "lm_p0_is_field_space", "lm_p0_is_field_separator", "lm_p0_is_short_form_separator",
            "lm_p0_is_quoted_token_boundary", "lm_p0_starts_python_string", "lm_p0_is_decimal_digit",
            "lm_p0_copy_bytes", "lm_p0_text_view_new_cstr", "lm_p0_text_view_delete", "lm_p0_text_from_cstr",
            "lm_p0_indent_tab_column", "lm_p0_scan_indent_column", "lm_p0_visual_column_between",
            "lm_p0_count_line_breaks", "lm_p0_position_in_slice", "lm_p0_advance_layout_line",
            "lm_p0_index_is_line_start", "lm_p0_line_rest_is_horizontal_space", "lm_p0_find_physical_line_end",
            "lm_p0_scan_layout_prefix", "lm_p0_layout_prefix_is_deeper"
        )
        # Stage a's 15 plus 11 new scanning primitives from l1src/parser.lm1
        # itself (position/line/indent/layout-prefix), l2src/parser_scan_port.lm2.
    },
    @{
        Name = "c_parser_alloc"
        Units = @("l2src\parser_text_port.lm2", "l2src\parser_scan_port.lm2", "l2src\parser_alloc_port.lm2")
        Headers = @("l2src\parser_text_port_l2.h.lm1", "l2src\parser_scan_port_l2.h.lm1", "l2src\parser_alloc_port_l2.h.lm1")
        Funcs = @(
            "lm_p0_text_equals", "lm_p0_identifier_payload",
            "lm_p0_is_horizontal_space", "lm_p0_is_line_break", "lm_p0_line_break_width_at",
            "lm_p0_is_field_space", "lm_p0_is_field_separator", "lm_p0_is_short_form_separator",
            "lm_p0_is_quoted_token_boundary", "lm_p0_starts_python_string", "lm_p0_is_decimal_digit",
            "lm_p0_copy_bytes", "lm_p0_text_view_new_cstr", "lm_p0_text_view_delete", "lm_p0_text_from_cstr",
            "lm_p0_indent_tab_column", "lm_p0_scan_indent_column", "lm_p0_visual_column_between",
            "lm_p0_count_line_breaks", "lm_p0_position_in_slice", "lm_p0_advance_layout_line",
            "lm_p0_index_is_line_start", "lm_p0_line_rest_is_horizontal_space", "lm_p0_find_physical_line_end",
            "lm_p0_scan_layout_prefix", "lm_p0_layout_prefix_is_deeper",
            "lm_p0_node_kind_class_name", "lm_p0_free_node"
        )
        # Stage b's 26 plus 2 of parser.lm1's node/structure/frame
        # allocation functions, l2src/parser_alloc_port.lm2.
        # lm_p0_new_structure/lm_p0_new_frame/lm_p0_new_node are NOT
        # here yet: blocked on the LmP0Document field-access gap
        # (reported to d6, 2026-09-14), see that unit's own header
        # comment.
    }
)

$parserSrc = "lm1\build\parser.lm1.c"
if (-not (Test-Path $parserSrc)) { throw "missing $parserSrc" }

# ---- Reference build: pristine oracle, untouched ----
$exeRef = Join-Path $out "p0_meta_dump_ref.exe"
Invoke-Gcc @("-std=c99", "-Wall", "-Wextra", "-DLM_THREAD_PROVIDER=LM_THREAD_PROVIDER_SINGLE", "-I", ".", "-I", "lm1\build", "-o", $exeRef, $dumpSrc, $parserSrc) (Join-Path $log "ref.gcc.log")

foreach ($stage in $Stages) {
    Write-Output "== stage $($stage.Name) =="
    $stageOut = Join-Path $out $stage.Name
    New-Item -ItemType Directory -Force -Path $stageOut | Out-Null

    # -- this stage's own predef'd header(s), generated into the same
    #    directory the runtime-trio headers live in, so #include
    #    "l2src/<name>.lm1.h" resolves via the existing -I. --
    foreach ($h in $stage.Headers) {
        $hName = [IO.Path]::GetFileNameWithoutExtension([IO.Path]::GetFileNameWithoutExtension($h))
        $hOut = Join-Path $rtHeaderDir "$hName.lm1.h"
        cmd /c "`"$l1trans`" `"$h`" `"$hOut`" > `"$(Join-Path $log "$($stage.Name)_hdr_$hName.log")`" 2>&1"
        if ($LASTEXITCODE -ne 0) { throw "l1trans failed building stage header $h" }
    }

    # -- L2 units: .lm2 -> .lm1 -> C. Each unit's lm_own_* calls
    #    (predef'd prototype: only, per d6's 4abf4fba) resolve
    #    externally to the oracle's real own.lm1 -- no fallback body
    #    to strip. --
    $unitFixedCs = @()
    foreach ($unit in $stage.Units) {
        $unitStem = [IO.Path]::GetFileNameWithoutExtension($unit)
        $unitLm1 = Join-Path $stageOut "$unitStem.lm1"
        $unitC = Join-Path $stageOut "$unitStem.c"
        cmd /c "`"$l2exe`" `"$unit`" `"$unitLm1`" > `"$(Join-Path $log "$($stage.Name)_${unitStem}_l2trans.log")`" 2>&1"
        if ($LASTEXITCODE -ne 0) { throw "l2trans failed: $unit" }
        cmd /c "`"$l1trans`" `"$unitLm1`" `"$unitC`" > `"$(Join-Path $log "$($stage.Name)_${unitStem}_l1trans.log")`" 2>&1"
        if ($LASTEXITCODE -ne 0) { throw "l1trans failed: $unitLm1" }
        $unitFixedCs += $unitC
    }

    # -- Patched oracle copy: rename each ported function's definition
    #    aside, capture its exact signature line as a fresh prototype
    #    inserted right after the first #include (before ANY internal
    #    call site -- some are earlier in the file than the last
    #    #include), and -D-redirect every remaining reference. --
    $patchedC = Join-Path $stageOut "parser_patched.c"
    $srcLines = Get-Content -LiteralPath $parserSrc
    $protos = New-Object System.Collections.Generic.List[string]
    $defines = New-Object System.Collections.Generic.List[string]
    foreach ($fn in $stage.Funcs) {
        $defLine = $srcLines | Where-Object { $_ -match "^\S.*\b$([regex]::Escape($fn))\(" } | Select-Object -First 1
        if (-not $defLine) { throw "no column-0 definition found for $fn in $parserSrc" }
        $protos.Add(($defLine.TrimEnd().TrimEnd(";") + ";"))
        $suffix = $fn.Substring(6)  # strip "lm_p0_"
        $defines.Add("-D$fn=p0_$suffix")
        $anchor = "^\S.*\b$([regex]::Escape($fn))\("
        for ($i = 0; $i -lt $srcLines.Count; $i++) {
            if ($srcLines[$i] -match $anchor) {
                $srcLines[$i] = [regex]::Replace($srcLines[$i], "\b$([regex]::Escape($fn))\b", "${fn}__l1")
            }
        }
    }
    $firstIncludeIdx = 0
    for ($i = 0; $i -lt $srcLines.Count; $i++) {
        if ($srcLines[$i] -match '^#include') { $firstIncludeIdx = $i; break }
    }
    $finalLines = New-Object System.Collections.Generic.List[string]
    for ($i = 0; $i -le $firstIncludeIdx; $i++) { $finalLines.Add($srcLines[$i]) }
    foreach ($p in $protos) { $finalLines.Add($p) }
    for ($i = $firstIncludeIdx + 1; $i -lt $srcLines.Count; $i++) { $finalLines.Add($srcLines[$i]) }
    [IO.File]::WriteAllLines($patchedC, $finalLines)

    # -- Build & link the Port executable: dump driver + patched
    #    oracle + every unit landed through this stage + the runtime
    #    trio. --
    $exePort = Join-Path $stageOut "p0_meta_dump_port.exe"
    $gccArgs = @("-std=c99", "-Wall", "-Wextra", "-Werror=incompatible-pointer-types", "-Werror=implicit-function-declaration",
                 "-DLM_THREAD_PROVIDER=LM_THREAD_PROVIDER_SINGLE") + $defines.ToArray() +
               @("-I", ".", "-I", "lm1\build", "-I", $rtHeaderRoot, "-o", $exePort, $dumpSrc, $patchedC) + $unitFixedCs + $rtObjs
    Invoke-Gcc $gccArgs (Join-Path $log "$($stage.Name).link.log")

    # -- Every golden must produce byte-identical output on both. --
    $mismatches = 0
    $n = 0
    Get-ChildItem $goldenDir -Filter "*.lmx" | Sort-Object Name | ForEach-Object {
        $n++
        $refOut = Join-Path $stageOut ($_.BaseName + ".ref.out")
        $portOut = Join-Path $stageOut ($_.BaseName + ".port.out")
        $pRef = Start-Process -FilePath $exeRef -ArgumentList $_.FullName -NoNewWindow -Wait -PassThru -RedirectStandardOutput $refOut
        $pPort = Start-Process -FilePath $exePort -ArgumentList $_.FullName -NoNewWindow -Wait -PassThru -RedirectStandardOutput $portOut
        if ($pRef.ExitCode -ne $pPort.ExitCode) {
            Write-Output "EXIT MISMATCH $($_.BaseName): ref=$($pRef.ExitCode) port=$($pPort.ExitCode)"
            $script:mismatches++
        } elseif ((Get-Content -Raw $refOut) -ne (Get-Content -Raw $portOut)) {
            Write-Output "OUTPUT MISMATCH $($_.BaseName)"
            $script:mismatches++
        }
    }
    if ($mismatches -gt 0) { throw "stage $($stage.Name): $mismatches/$n goldens diverged" }
    Write-Output "stage $($stage.Name) ok: $n/$n goldens identical (ref vs port)"
}

Write-Output "run_port_parser ok"
