# run_port_parser.ps1 -- parser-in-L2 port acceptance gate.
#
# l1src/parser.lm1 (the oracle) is never edited. For each landed stage
# this builds a Ref/Port pair against tests/p0_tree_contract's 36
# *.lmx inputs, twice over -- once through p0_meta_dump.c (the
# structural-metadata driver tests/p0_tree_contract itself uses) and
# once through l2src/p0_dump_driver.c (this port's own driver for
# l1src/parser.lm1's lm_p0_dump_alloc/dump_run/dump_node family, which
# p0_meta_dump.c never calls -- see that file's own header comment):
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
$dumpDriverSrc = Join-Path $PSScriptRoot "p0_dump_driver.c"
if (-not (Test-Path $dumpDriverSrc)) { throw "missing $dumpDriverSrc" }

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

# Runs every tests/p0_tree_contract/*.lmx input through both
# executables and emits one mismatch line per input that diverged
# (stdout or exit code); no output means all agree. $Label distinguishes
# per-golden output file names between the two drivers within the same
# stage directory. A plain loop, not Where-Object: inside a filter block
# Write-Output becomes the filter's value and the lines never printed.
function Test-GoldensBetween([string]$ExeRef, [string]$ExePort, [string]$StageOut, [string]$Label) {
    $files = Get-ChildItem $goldenDir -Filter "*.lmx" | Sort-Object Name
    foreach ($file in $files) {
        $refOut = Join-Path $StageOut ($file.BaseName + ".$Label.ref.out")
        $portOut = Join-Path $StageOut ($file.BaseName + ".$Label.port.out")
        # cmd /c, not Start-Process -Wait, which costs ~1 s per launch
        # (l2src/PORT_PARSER_TIMING.txt). Only stdout is redirected, as before.
        cmd /c "`"$ExeRef`" `"$($file.FullName)`" > `"$refOut`""
        $refCode = $LASTEXITCODE
        cmd /c "`"$ExePort`" `"$($file.FullName)`" > `"$portOut`""
        $portCode = $LASTEXITCODE
        if ($refCode -ne $portCode) {
            "$Label EXIT MISMATCH $($file.BaseName): ref=$refCode port=$portCode"
        } elseif ((Get-Content -Raw $refOut) -ne (Get-Content -Raw $portOut)) {
            "$Label OUTPUT MISMATCH $($file.BaseName)"
        }
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
    "lmx_msg_path_storage", "lmx_msg_roots_stale",
    "lmx_msg_slots", "lmx_msg_storage", "lmx_msg_visit", "lmx_owned_ranges",
    "lmx_value_owned"
)
foreach ($n in $rtHeaderNames) {
    $hdrOut = Join-Path $rtHeaderDir "$n.lm1.h"
    cmd /c "`"$l1trans`" l2src\$n.h.lm1 `"$hdrOut`" > `"$(Join-Path $log "hdr_$n.log")`" 2>&1"
    if ($LASTEXITCODE -ne 0) { throw "l1trans failed building header $n" }
}
$rtObjDir = Join-Path $out "l2rt_objs"
New-Item -ItemType Directory -Force -Path $rtObjDir | Out-Null
$rtModuleNames = $rtHeaderNames + @("lmx_message")
$rtObjs = @()
foreach ($n in $rtModuleNames) {
    $objOut = Join-Path $rtObjDir "$n.o"
    $rtObjs += $objOut
    $cOut = Join-Path $rtObjDir "$n.c"
    cmd /c "`"$l1trans`" l2src\$n.lm1 `"$cOut`" > `"$(Join-Path $log "trans_$n.log")`" 2>&1"
    if ($LASTEXITCODE -ne 0) { throw "l1trans failed translating $n.lm1" }
    Invoke-Gcc @("-std=c99", "-w", "-I", $rtHeaderRoot, "-I", (Get-Location), "-c", $cOut, "-o", $objOut) (Join-Path $log "compile_$n.log")
}
foreach ($n in @("lmx_message_host", "lmx_message_exec")) {
    $objOut = Join-Path $rtObjDir "$n.o"
    $rtObjs += $objOut
    Invoke-Gcc @("-std=c99", "-w", "-I", $rtHeaderRoot, "-I", (Get-Location), "-c", "l2src\$n.c", "-o", $objOut) (Join-Path $log "compile_$n.log")
}
# Stage 3c-2: the runtime links the L2 runtime units (l2src/l2units_build.ps1;
# today lmx_sched_record.lm2, which lmx_message calls from 3c-2b on).
. l2src/l2units_build.ps1
$rtObjs += @(Build-L2RuntimeUnits -L1Trans $l1trans -Out (Join-Path $out 'l2units') -IncludeDirs @($rtHeaderRoot) -CFlags '-std=c99 -w -I .')

# ---- p0_dump_driver.c's leak-balance counter (p0_counted_calloc/
#      realloc/free -- e2, 2026-09-14, Slice 4). Built once, plain, with
#      NONE of the calloc/realloc/free redirects below -- see that file's
#      own header comment for why. ----
$allocCounterObj = Join-Path $rtObjDir "p0_dump_alloc_counter.o"
if (-not (Test-Path $allocCounterObj)) {
    Invoke-Gcc @("-std=c99", "-w", "-c", "l2src\p0_dump_alloc_counter.c", "-o", $allocCounterObj) (Join-Path $log "compile_p0_dump_alloc_counter.log")
}
# calloc/realloc/free redirect for the dump-driver executables only (not
# p0_meta_dump): every c.calloc/c.realloc/c.free call site inlined from
# l1src/own.lm1 into lm1/build/parser.lm1.c -- Ref's pristine copy and
# every stage's own patched copy alike -- routes through the counted
# wrappers, plus p0_dump_driver.c's own free(dump) of the buffer handed
# back to it. Applied identically to Ref and Port so a normal run
# balances to 0 on both; only a real wrong-owner/wrong-length bug in a
# ported dump function makes them differ.
$allocCounterDefines = @("-Dcalloc=p0_counted_calloc", "-Drealloc=p0_counted_realloc", "-Dfree=p0_counted_free")

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
            "lm_p0_node_kind_class_name", "lm_p0_free_node",
            "lm_p0_new_structure", "lm_p0_new_frame", "lm_p0_new_node"
        )
        # Stage b's 26 plus all 5 of parser.lm1's node/structure/frame
        # allocation functions, l2src/parser_alloc_port.lm2.
        # lm_p0_new_structure/lm_p0_new_frame/lm_p0_new_node landed
        # once d6's Stage B step 3 (f7390ded/fb7ab80c) admitted
        # document\tree_arena field access.
    },
    @{
        Name = "c_parser_dump_slice1"
        Units = @("l2src\parser_text_port.lm2", "l2src\parser_scan_port.lm2", "l2src\parser_alloc_port.lm2", "l2src\parser_dump_port.lm2")
        Headers = @("l2src\parser_text_port_l2.h.lm1", "l2src\parser_scan_port_l2.h.lm1", "l2src\parser_alloc_port_l2.h.lm1", "l2src\parser_dump_port_l2.h.lm1")
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
            "lm_p0_node_kind_class_name", "lm_p0_free_node",
            "lm_p0_new_structure", "lm_p0_new_frame", "lm_p0_new_node",
            "lm_p0_dump_append", "lm_p0_dump_append_cstr", "lm_p0_dump_reserve",
            "lm_p0_dump_append_size", "lm_p0_dump_append_field_count_line",
            "lm_p0_dump_indent", "lm_p0_dump_text"
        )
        # Stage c's 28 plus 7 leaf text-buffer primitives of parser.lm1's
        # dump-printer family (e2's ordering, 2026-09-14: leaves first,
        # then frame/stack, then run/node, then new/take_data/delete/
        # alloc), l2src/parser_dump_port.lm2.
    },
    @{
        Name = "c_parser_dump_slice2"
        Units = @("l2src\parser_text_port.lm2", "l2src\parser_scan_port.lm2", "l2src\parser_alloc_port.lm2", "l2src\parser_dump_port.lm2")
        Headers = @("l2src\parser_text_port_l2.h.lm1", "l2src\parser_scan_port_l2.h.lm1", "l2src\parser_alloc_port_l2.h.lm1", "l2src\parser_dump_port_l2.h.lm1")
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
            "lm_p0_node_kind_class_name", "lm_p0_free_node",
            "lm_p0_new_structure", "lm_p0_new_frame", "lm_p0_new_node",
            "lm_p0_dump_append", "lm_p0_dump_append_cstr", "lm_p0_dump_reserve",
            "lm_p0_dump_append_size", "lm_p0_dump_append_field_count_line",
            "lm_p0_dump_indent", "lm_p0_dump_text",
            "lm_p0_dump_frame_new", "lm_p0_dump_push_frame", "lm_p0_dump_push_node",
            "lm_p0_dump_push_structure", "lm_p0_dump_push_trailer", "lm_p0_dump_stack_delete",
            "lm_p0_dump_stack_new"
        )
        # Slice 1's 35 plus all 7 of the dump-frame/stack family:
        # p0_dump_frame_new, p0_dump_push_frame, p0_dump_push_node,
        # p0_dump_push_structure, p0_dump_push_trailer,
        # p0_dump_stack_delete, p0_dump_stack_new. The last one needed
        # d6's aec85b3b (a bare function name as a value); landed once
        # that fix confirmed.
    },
    @{
        Name = "c_parser_dump_run"
        Units = @("l2src\parser_text_port.lm2", "l2src\parser_scan_port.lm2", "l2src\parser_alloc_port.lm2", "l2src\parser_dump_port.lm2")
        Headers = @("l2src\parser_text_port_l2.h.lm1", "l2src\parser_scan_port_l2.h.lm1", "l2src\parser_alloc_port_l2.h.lm1", "l2src\parser_dump_port_l2.h.lm1")
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
            "lm_p0_node_kind_class_name", "lm_p0_free_node",
            "lm_p0_new_structure", "lm_p0_new_frame", "lm_p0_new_node",
            "lm_p0_dump_append", "lm_p0_dump_append_cstr", "lm_p0_dump_reserve",
            "lm_p0_dump_append_size", "lm_p0_dump_append_field_count_line",
            "lm_p0_dump_indent", "lm_p0_dump_text",
            "lm_p0_dump_frame_new", "lm_p0_dump_push_frame", "lm_p0_dump_push_node",
            "lm_p0_dump_push_structure", "lm_p0_dump_push_trailer", "lm_p0_dump_stack_delete",
            "lm_p0_dump_stack_new",
            "lm_p0_dump_run"
        )
        # Slice 2's 42 plus p0_dump_run, the traversal state machine
        # (Slice 3, e2's ordering). Calls p0_node_kind_class_name (a
        # Stage c function in a different unit, parser_alloc_port.lm2)
        # through a new cross-unit prototype: entry in this file's own
        # header, same shape as Stage b's cross-stage dependency on
        # Stage a's p0_is_horizontal_space.
    },
    @{
        Name = "c_parser_dump_node"
        Units = @("l2src\parser_text_port.lm2", "l2src\parser_scan_port.lm2", "l2src\parser_alloc_port.lm2", "l2src\parser_dump_port.lm2")
        Headers = @("l2src\parser_text_port_l2.h.lm1", "l2src\parser_scan_port_l2.h.lm1", "l2src\parser_alloc_port_l2.h.lm1", "l2src\parser_dump_port_l2.h.lm1")
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
            "lm_p0_node_kind_class_name", "lm_p0_free_node",
            "lm_p0_new_structure", "lm_p0_new_frame", "lm_p0_new_node",
            "lm_p0_dump_append", "lm_p0_dump_append_cstr", "lm_p0_dump_reserve",
            "lm_p0_dump_append_size", "lm_p0_dump_append_field_count_line",
            "lm_p0_dump_indent", "lm_p0_dump_text",
            "lm_p0_dump_frame_new", "lm_p0_dump_push_frame", "lm_p0_dump_push_node",
            "lm_p0_dump_push_structure", "lm_p0_dump_push_trailer", "lm_p0_dump_stack_delete",
            "lm_p0_dump_stack_new",
            "lm_p0_dump_run", "lm_p0_dump_node"
        )
        # Prior stage's 44 plus p0_dump_node, the traversal orchestrator
        # (creates the stack, pushes the root node, runs the state
        # machine, tears the stack down) -- Slice 3 complete.
    },
    @{
        Name = "c_parser_dump_new"
        Units = @("l2src\parser_text_port.lm2", "l2src\parser_scan_port.lm2", "l2src\parser_alloc_port.lm2", "l2src\parser_dump_port.lm2")
        Headers = @("l2src\parser_text_port_l2.h.lm1", "l2src\parser_scan_port_l2.h.lm1", "l2src\parser_alloc_port_l2.h.lm1", "l2src\parser_dump_port_l2.h.lm1")
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
            "lm_p0_node_kind_class_name", "lm_p0_free_node",
            "lm_p0_new_structure", "lm_p0_new_frame", "lm_p0_new_node",
            "lm_p0_dump_append", "lm_p0_dump_append_cstr", "lm_p0_dump_reserve",
            "lm_p0_dump_append_size", "lm_p0_dump_append_field_count_line",
            "lm_p0_dump_indent", "lm_p0_dump_text",
            "lm_p0_dump_frame_new", "lm_p0_dump_push_frame", "lm_p0_dump_push_node",
            "lm_p0_dump_push_structure", "lm_p0_dump_push_trailer", "lm_p0_dump_stack_delete",
            "lm_p0_dump_stack_new",
            "lm_p0_dump_run", "lm_p0_dump_node", "lm_p0_dump_new"
        )
        # Prior stage's 46 plus p0_dump_new, the Dump-struct allocator --
        # Slice 4 begins.
    },
    @{
        Name = "c_parser_dump_take_data"
        Units = @("l2src\parser_text_port.lm2", "l2src\parser_scan_port.lm2", "l2src\parser_alloc_port.lm2", "l2src\parser_dump_port.lm2")
        Headers = @("l2src\parser_text_port_l2.h.lm1", "l2src\parser_scan_port_l2.h.lm1", "l2src\parser_alloc_port_l2.h.lm1", "l2src\parser_dump_port_l2.h.lm1")
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
            "lm_p0_node_kind_class_name", "lm_p0_free_node",
            "lm_p0_new_structure", "lm_p0_new_frame", "lm_p0_new_node",
            "lm_p0_dump_append", "lm_p0_dump_append_cstr", "lm_p0_dump_reserve",
            "lm_p0_dump_append_size", "lm_p0_dump_append_field_count_line",
            "lm_p0_dump_indent", "lm_p0_dump_text",
            "lm_p0_dump_frame_new", "lm_p0_dump_push_frame", "lm_p0_dump_push_node",
            "lm_p0_dump_push_structure", "lm_p0_dump_push_trailer", "lm_p0_dump_stack_delete",
            "lm_p0_dump_stack_new",
            "lm_p0_dump_run", "lm_p0_dump_node", "lm_p0_dump_new", "lm_p0_dump_take_data"
        )
        # Prior stage's 47 plus p0_dump_take_data, which detaches the
        # finished buffer from the Dump struct.
    },
    @{
        Name = "c_parser_dump_delete"
        Units = @("l2src\parser_text_port.lm2", "l2src\parser_scan_port.lm2", "l2src\parser_alloc_port.lm2", "l2src\parser_dump_port.lm2")
        Headers = @("l2src\parser_text_port_l2.h.lm1", "l2src\parser_scan_port_l2.h.lm1", "l2src\parser_alloc_port_l2.h.lm1", "l2src\parser_dump_port_l2.h.lm1")
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
            "lm_p0_node_kind_class_name", "lm_p0_free_node",
            "lm_p0_new_structure", "lm_p0_new_frame", "lm_p0_new_node",
            "lm_p0_dump_append", "lm_p0_dump_append_cstr", "lm_p0_dump_reserve",
            "lm_p0_dump_append_size", "lm_p0_dump_append_field_count_line",
            "lm_p0_dump_indent", "lm_p0_dump_text",
            "lm_p0_dump_frame_new", "lm_p0_dump_push_frame", "lm_p0_dump_push_node",
            "lm_p0_dump_push_structure", "lm_p0_dump_push_trailer", "lm_p0_dump_stack_delete",
            "lm_p0_dump_stack_new",
            "lm_p0_dump_run", "lm_p0_dump_node", "lm_p0_dump_new", "lm_p0_dump_take_data",
            "lm_p0_dump_delete"
        )
        # Prior stage's 48 plus p0_dump_delete, which frees the Dump
        # struct (and, when take_data wasn't called first, its buffer).
    },
    @{
        Name = "c_parser_dump_alloc"
        Units = @("l2src\parser_text_port.lm2", "l2src\parser_scan_port.lm2", "l2src\parser_alloc_port.lm2", "l2src\parser_dump_port.lm2")
        Headers = @("l2src\parser_text_port_l2.h.lm1", "l2src\parser_scan_port_l2.h.lm1", "l2src\parser_alloc_port_l2.h.lm1", "l2src\parser_dump_port_l2.h.lm1")
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
            "lm_p0_node_kind_class_name", "lm_p0_free_node",
            "lm_p0_new_structure", "lm_p0_new_frame", "lm_p0_new_node",
            "lm_p0_dump_append", "lm_p0_dump_append_cstr", "lm_p0_dump_reserve",
            "lm_p0_dump_append_size", "lm_p0_dump_append_field_count_line",
            "lm_p0_dump_indent", "lm_p0_dump_text",
            "lm_p0_dump_frame_new", "lm_p0_dump_push_frame", "lm_p0_dump_push_node",
            "lm_p0_dump_push_structure", "lm_p0_dump_push_trailer", "lm_p0_dump_stack_delete",
            "lm_p0_dump_stack_new",
            "lm_p0_dump_run", "lm_p0_dump_node", "lm_p0_dump_new", "lm_p0_dump_take_data",
            "lm_p0_dump_delete", "lm_p0_dump_alloc"
        )
        # Prior stage's 49 plus p0_dump_alloc, the public entry point --
        # dump-printer family complete. p0_dump_driver.c's own call to
        # lm_p0_dump_alloc gets redirected the same way every internal
        # oracle call site does, since it's compiled in the same gcc
        # invocation as the patched oracle copy.
    },
    @{
        Name = "d_lifecycle"
        Units = @("l2src\parser_text_port.lm2", "l2src\parser_scan_port.lm2", "l2src\parser_alloc_port.lm2", "l2src\parser_dump_port.lm2", "l2src\parser_stack_stream_port.lm2")
        Headers = @("l2src\parser_text_port_l2.h.lm1", "l2src\parser_scan_port_l2.h.lm1", "l2src\parser_alloc_port_l2.h.lm1", "l2src\parser_dump_port_l2.h.lm1", "l2src\parser_stack_stream_port_l2.h.lm1")
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
            "lm_p0_node_kind_class_name", "lm_p0_free_node",
            "lm_p0_new_structure", "lm_p0_new_frame", "lm_p0_new_node",
            "lm_p0_dump_append", "lm_p0_dump_append_cstr", "lm_p0_dump_reserve",
            "lm_p0_dump_append_size", "lm_p0_dump_append_field_count_line",
            "lm_p0_dump_indent", "lm_p0_dump_text",
            "lm_p0_dump_frame_new", "lm_p0_dump_push_frame", "lm_p0_dump_push_node",
            "lm_p0_dump_push_structure", "lm_p0_dump_push_trailer", "lm_p0_dump_stack_delete",
            "lm_p0_dump_stack_new",
            "lm_p0_dump_run", "lm_p0_dump_node", "lm_p0_dump_new", "lm_p0_dump_take_data",
            "lm_p0_dump_delete", "lm_p0_dump_alloc",
            "lm_p0_stack_free", "lm_p0_stack_free_any", "lm_p0_stack_new", "lm_p0_stack_delete",
            "lm_p0_pending_delimiter_new", "lm_p0_stream_event_new", "lm_p0_stream_event_new_copy",
            "lm_p0_stream_event_delete", "lm_p0_stream_event_is_tail_cutter",
            "lm_p0_pending_delimiter_clear", "lm_p0_pending_delimiter_delete",
            "lm_p0_pending_mix_new", "lm_p0_pending_mix_free", "lm_p0_pending_mix_free_any",
            "lm_p0_pending_mix_delete", "lm_p0_disabled_state_new", "lm_p0_disabled_state_delete"
        )
        # Prior stage's 50 plus Stage d Slice 1 (lifecycle): 17 struct
        # lifecycle functions for LmP0Stack/LmP0PendingDelimiter/
        # LmP0StreamEvent/LmP0PendingMix/LmP0DisabledState, plus the
        # stream_event_is_tail_cutter predicate. New unit,
        # l2src/parser_stack_stream_port.lm2.
    },
    @{
        Name = "d_stack_geometry"
        Units = @("l2src\parser_text_port.lm2", "l2src\parser_scan_port.lm2", "l2src\parser_alloc_port.lm2", "l2src\parser_dump_port.lm2", "l2src\parser_stack_stream_port.lm2")
        Headers = @("l2src\parser_text_port_l2.h.lm1", "l2src\parser_scan_port_l2.h.lm1", "l2src\parser_alloc_port_l2.h.lm1", "l2src\parser_dump_port_l2.h.lm1", "l2src\parser_stack_stream_port_l2.h.lm1")
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
            "lm_p0_node_kind_class_name", "lm_p0_free_node",
            "lm_p0_new_structure", "lm_p0_new_frame", "lm_p0_new_node",
            "lm_p0_dump_append", "lm_p0_dump_append_cstr", "lm_p0_dump_reserve",
            "lm_p0_dump_append_size", "lm_p0_dump_append_field_count_line",
            "lm_p0_dump_indent", "lm_p0_dump_text",
            "lm_p0_dump_frame_new", "lm_p0_dump_push_frame", "lm_p0_dump_push_node",
            "lm_p0_dump_push_structure", "lm_p0_dump_push_trailer", "lm_p0_dump_stack_delete",
            "lm_p0_dump_stack_new",
            "lm_p0_dump_run", "lm_p0_dump_node", "lm_p0_dump_new", "lm_p0_dump_take_data",
            "lm_p0_dump_delete", "lm_p0_dump_alloc",
            "lm_p0_stack_free", "lm_p0_stack_free_any", "lm_p0_stack_new", "lm_p0_stack_delete",
            "lm_p0_pending_delimiter_new", "lm_p0_stream_event_new", "lm_p0_stream_event_new_copy",
            "lm_p0_stream_event_delete", "lm_p0_stream_event_is_tail_cutter",
            "lm_p0_pending_delimiter_clear", "lm_p0_pending_delimiter_delete",
            "lm_p0_pending_mix_new", "lm_p0_pending_mix_free", "lm_p0_pending_mix_free_any",
            "lm_p0_pending_mix_delete", "lm_p0_disabled_state_new", "lm_p0_disabled_state_delete",
            "lm_p0_stack_ensure", "lm_p0_stack_truncate_deeper", "lm_p0_stack_top_level",
            "lm_p0_stack_level_is_trailer_body", "lm_p0_stack_collapse_soft_to_event"
        )
        # Prior stage's 67 plus Stage d Slice 2 (stack geometry): 5
        # functions operating on an existing LmP0Stack's
        # parents/owners/hard arrays.
    },
    @{
        Name = "d_lineage_trailer"
        Units = @("l2src\parser_text_port.lm2", "l2src\parser_scan_port.lm2", "l2src\parser_alloc_port.lm2", "l2src\parser_dump_port.lm2", "l2src\parser_stack_stream_port.lm2")
        Headers = @("l2src\parser_text_port_l2.h.lm1", "l2src\parser_scan_port_l2.h.lm1", "l2src\parser_alloc_port_l2.h.lm1", "l2src\parser_dump_port_l2.h.lm1", "l2src\parser_stack_stream_port_l2.h.lm1")
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
            "lm_p0_node_kind_class_name", "lm_p0_free_node",
            "lm_p0_new_structure", "lm_p0_new_frame", "lm_p0_new_node",
            "lm_p0_dump_append", "lm_p0_dump_append_cstr", "lm_p0_dump_reserve",
            "lm_p0_dump_append_size", "lm_p0_dump_append_field_count_line",
            "lm_p0_dump_indent", "lm_p0_dump_text",
            "lm_p0_dump_frame_new", "lm_p0_dump_push_frame", "lm_p0_dump_push_node",
            "lm_p0_dump_push_structure", "lm_p0_dump_push_trailer", "lm_p0_dump_stack_delete",
            "lm_p0_dump_stack_new",
            "lm_p0_dump_run", "lm_p0_dump_node", "lm_p0_dump_new", "lm_p0_dump_take_data",
            "lm_p0_dump_delete", "lm_p0_dump_alloc",
            "lm_p0_stack_free", "lm_p0_stack_free_any", "lm_p0_stack_new", "lm_p0_stack_delete",
            "lm_p0_pending_delimiter_new", "lm_p0_stream_event_new", "lm_p0_stream_event_new_copy",
            "lm_p0_stream_event_delete", "lm_p0_stream_event_is_tail_cutter",
            "lm_p0_pending_delimiter_clear", "lm_p0_pending_delimiter_delete",
            "lm_p0_pending_mix_new", "lm_p0_pending_mix_free", "lm_p0_pending_mix_free_any",
            "lm_p0_pending_mix_delete", "lm_p0_disabled_state_new", "lm_p0_disabled_state_delete",
            "lm_p0_stack_ensure", "lm_p0_stack_truncate_deeper", "lm_p0_stack_top_level",
            "lm_p0_stack_level_is_trailer_body", "lm_p0_stack_collapse_soft_to_event",
            "lm_p0_node_keeps_source_child_level", "lm_p0_node_child_structure",
            "lm_p0_structure_last_colon_frame", "lm_p0_stack_install_node_lineage",
            "lm_p0_stack_ensure_root_level_alias", "lm_p0_stack_open_implicit_anonymous",
            "lm_p0_node_trailer_slot", "lm_p0_attach_trailer", "lm_p0_pending_delimiter_set"
        )
        # Prior stage's 72 plus Stage d Slice 3 (lineage/trailer-attach):
        # 9 functions. Two cross-unit prototype: entries added (p0_new_
        # node/p0_free_node, already ported in parser_alloc_port.lm2);
        # two still-oracle-native prototype: entries added (lm_p0_
        # append_field, lm_p0_new_trailer).
    },
    @{
        Name = "d_trailer_resolve"
        Units = @("l2src\parser_text_port.lm2", "l2src\parser_scan_port.lm2", "l2src\parser_alloc_port.lm2", "l2src\parser_dump_port.lm2", "l2src\parser_stack_stream_port.lm2")
        Headers = @("l2src\parser_text_port_l2.h.lm1", "l2src\parser_scan_port_l2.h.lm1", "l2src\parser_alloc_port_l2.h.lm1", "l2src\parser_dump_port_l2.h.lm1", "l2src\parser_stack_stream_port_l2.h.lm1")
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
            "lm_p0_node_kind_class_name", "lm_p0_free_node",
            "lm_p0_new_structure", "lm_p0_new_frame", "lm_p0_new_node",
            "lm_p0_dump_append", "lm_p0_dump_append_cstr", "lm_p0_dump_reserve",
            "lm_p0_dump_append_size", "lm_p0_dump_append_field_count_line",
            "lm_p0_dump_indent", "lm_p0_dump_text",
            "lm_p0_dump_frame_new", "lm_p0_dump_push_frame", "lm_p0_dump_push_node",
            "lm_p0_dump_push_structure", "lm_p0_dump_push_trailer", "lm_p0_dump_stack_delete",
            "lm_p0_dump_stack_new",
            "lm_p0_dump_run", "lm_p0_dump_node", "lm_p0_dump_new", "lm_p0_dump_take_data",
            "lm_p0_dump_delete", "lm_p0_dump_alloc",
            "lm_p0_stack_free", "lm_p0_stack_free_any", "lm_p0_stack_new", "lm_p0_stack_delete",
            "lm_p0_pending_delimiter_new", "lm_p0_stream_event_new", "lm_p0_stream_event_new_copy",
            "lm_p0_stream_event_delete", "lm_p0_stream_event_is_tail_cutter",
            "lm_p0_pending_delimiter_clear", "lm_p0_pending_delimiter_delete",
            "lm_p0_pending_mix_new", "lm_p0_pending_mix_free", "lm_p0_pending_mix_free_any",
            "lm_p0_pending_mix_delete", "lm_p0_disabled_state_new", "lm_p0_disabled_state_delete",
            "lm_p0_stack_ensure", "lm_p0_stack_truncate_deeper", "lm_p0_stack_top_level",
            "lm_p0_stack_level_is_trailer_body", "lm_p0_stack_collapse_soft_to_event",
            "lm_p0_node_keeps_source_child_level", "lm_p0_node_child_structure",
            "lm_p0_structure_last_colon_frame", "lm_p0_stack_install_node_lineage",
            "lm_p0_stack_ensure_root_level_alias", "lm_p0_stack_open_implicit_anonymous",
            "lm_p0_node_trailer_slot", "lm_p0_attach_trailer", "lm_p0_pending_delimiter_set",
            "lm_p0_stream_resolve_pending_delimiter", "lm_p0_parse_trailer_item"
        )
        # Prior stage's 81 plus Stage d Slice 4 (trailer-item parse +
        # delimiter resolve): 2 functions. Header gains three cross-unit
        # entries (p0_is_horizontal_space/p0_is_field_separator/p0_
        # starts_python_string, already ported in parser_text_port.lm2)
        # and six still-oracle-native entries (lm_p0_find_colon, lm_p0_
        # starts_c_prefixed_quote, lm_p0_scan_c_prefixed_quote, lm_p0_
        # scan_c_char, lm_p0_scan_quoted, lm_p0_parse_fields_into --
        # Stage e, not yet ported).
    },
    @{
        Name = "d_event_apply"
        Units = @("l2src\parser_text_port.lm2", "l2src\parser_scan_port.lm2", "l2src\parser_alloc_port.lm2", "l2src\parser_dump_port.lm2", "l2src\parser_stack_stream_port.lm2")
        Headers = @("l2src\parser_text_port_l2.h.lm1", "l2src\parser_scan_port_l2.h.lm1", "l2src\parser_alloc_port_l2.h.lm1", "l2src\parser_dump_port_l2.h.lm1", "l2src\parser_stack_stream_port_l2.h.lm1")
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
            "lm_p0_node_kind_class_name", "lm_p0_free_node",
            "lm_p0_new_structure", "lm_p0_new_frame", "lm_p0_new_node",
            "lm_p0_dump_append", "lm_p0_dump_append_cstr", "lm_p0_dump_reserve",
            "lm_p0_dump_append_size", "lm_p0_dump_append_field_count_line",
            "lm_p0_dump_indent", "lm_p0_dump_text",
            "lm_p0_dump_frame_new", "lm_p0_dump_push_frame", "lm_p0_dump_push_node",
            "lm_p0_dump_push_structure", "lm_p0_dump_push_trailer", "lm_p0_dump_stack_delete",
            "lm_p0_dump_stack_new",
            "lm_p0_dump_run", "lm_p0_dump_node", "lm_p0_dump_new", "lm_p0_dump_take_data",
            "lm_p0_dump_delete", "lm_p0_dump_alloc",
            "lm_p0_stack_free", "lm_p0_stack_free_any", "lm_p0_stack_new", "lm_p0_stack_delete",
            "lm_p0_pending_delimiter_new", "lm_p0_stream_event_new", "lm_p0_stream_event_new_copy",
            "lm_p0_stream_event_delete", "lm_p0_stream_event_is_tail_cutter",
            "lm_p0_pending_delimiter_clear", "lm_p0_pending_delimiter_delete",
            "lm_p0_pending_mix_new", "lm_p0_pending_mix_free", "lm_p0_pending_mix_free_any",
            "lm_p0_pending_mix_delete", "lm_p0_disabled_state_new", "lm_p0_disabled_state_delete",
            "lm_p0_stack_ensure", "lm_p0_stack_truncate_deeper", "lm_p0_stack_top_level",
            "lm_p0_stack_level_is_trailer_body", "lm_p0_stack_collapse_soft_to_event",
            "lm_p0_node_keeps_source_child_level", "lm_p0_node_child_structure",
            "lm_p0_structure_last_colon_frame", "lm_p0_stack_install_node_lineage",
            "lm_p0_stack_ensure_root_level_alias", "lm_p0_stack_open_implicit_anonymous",
            "lm_p0_node_trailer_slot", "lm_p0_attach_trailer", "lm_p0_pending_delimiter_set",
            "lm_p0_stream_resolve_pending_delimiter", "lm_p0_parse_trailer_item",
            "lm_p0_pending_mix_push", "lm_p0_stream_apply_item_event", "lm_p0_stream_apply_mix_event",
            "lm_p0_stream_apply_event", "lm_p0_pending_mix_flush", "lm_p0_stream_block_string_level"
        )
        # Prior stage's 83 plus Stage d Slice 5 (event application core):
        # 6 functions -- Stage d complete. Header gains four more
        # still-oracle-native entries (lm_p0_trailer_role_accepts_target,
        # lm_p0_validate_dash_fence_line, lm_p0_document_register_lazy_
        # text, lm_p0_dash_fence_status_after_comment_trim). p0_stream_
        # apply_item_event's own trailer_role local uses a documented
        # `int:` stopgap for the foreign int-alias LmP0TrailerRole --
        # see l2src/tests/repro_foreign_int_alias_local.lm2, sent to d6.
    }
)

$parserSrc = "lm1\build\parser.lm1.c"
if (-not (Test-Path $parserSrc)) { throw "missing $parserSrc" }

# ---- Reference builds: pristine oracle, untouched, both drivers ----
$exeRef = Join-Path $out "p0_meta_dump_ref.exe"
Invoke-Gcc @("-std=c99", "-Wall", "-Wextra", "-DLM_THREAD_PROVIDER=LM_THREAD_PROVIDER_SINGLE", "-I", ".", "-I", "lm1\build", "-o", $exeRef, $dumpSrc, $parserSrc) (Join-Path $log "ref.gcc.log")
$exeDumpRef = Join-Path $out "p0_dump_driver_ref.exe"
Invoke-Gcc (@("-std=c99", "-Wall", "-Wextra", "-DLM_THREAD_PROVIDER=LM_THREAD_PROVIDER_SINGLE") + $allocCounterDefines +
            @("-I", ".", "-I", "lm1\build", "-o", $exeDumpRef, $dumpDriverSrc, $parserSrc, $allocCounterObj)) (Join-Path $log "dumpref.gcc.log")

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
    #    to strip. Every unit also carries its own copy of l2trans's
    #    shared immut-query support boilerplate (l2_fnv1a64,
    #    l2_immut_query_fill, l2_hash_compare, l2_hash_compare_q) --
    #    identical, auto-generated, no per-unit customization -- which
    #    collides at link time once two or more units are linked
    #    together; stripped here from every unit but the first. --
    $sharedBoilerplateNames = @("l2_fnv1a64", "l2_immut_query_fill", "l2_hash_compare_q", "l2_hash_compare")
    $unitFixedCs = @()
    $unitIdx = 0
    foreach ($unit in $stage.Units) {
        $unitIdx++
        $unitStem = [IO.Path]::GetFileNameWithoutExtension($unit)
        $unitLm1 = Join-Path $stageOut "$unitStem.lm1"
        $unitC = Join-Path $stageOut "$unitStem.c"
        cmd /c "`"$l2exe`" `"$unit`" `"$unitLm1`" > `"$(Join-Path $log "$($stage.Name)_${unitStem}_l2trans.log")`" 2>&1"
        if ($LASTEXITCODE -ne 0) { throw "l2trans failed: $unit" }
        cmd /c "`"$l1trans`" `"$unitLm1`" `"$unitC`" > `"$(Join-Path $log "$($stage.Name)_${unitStem}_l1trans.log")`" 2>&1"
        if ($LASTEXITCODE -ne 0) { throw "l1trans failed: $unitLm1" }
        if ($unitIdx -gt 1) {
            $unitFixedC = Join-Path $stageOut "$unitStem`_dedup.c"
            $namePattern = ($sharedBoilerplateNames | ForEach-Object { [regex]::Escape($_) }) -join "|"
            $bpPattern = "^\S.*\b($namePattern)\("
            $skip = $false
            $keptLines = New-Object System.Collections.Generic.List[string]
            foreach ($line in (Get-Content -LiteralPath $unitC)) {
                if (-not $skip -and $line -match $bpPattern) { $skip = $true; continue }
                if ($skip -and $line -eq "}") { $skip = $false; continue }
                if (-not $skip) { $keptLines.Add($line) }
            }
            [IO.File]::WriteAllLines($unitFixedC, $keptLines)
            $unitFixedCs += $unitFixedC
        } else {
            $unitFixedCs += $unitC
        }
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

    # -- Build & link the Port executables: each driver + patched
    #    oracle + every unit landed through this stage + the runtime
    #    trio. --
    $exePort = Join-Path $stageOut "p0_meta_dump_port.exe"
    $gccArgs = @("-std=c99", "-Wall", "-Wextra", "-Werror=incompatible-pointer-types", "-Werror=implicit-function-declaration",
                 "-DLM_THREAD_PROVIDER=LM_THREAD_PROVIDER_SINGLE") + $defines.ToArray() +
               @("-I", ".", "-I", "lm1\build", "-I", $rtHeaderRoot, "-o", $exePort, $dumpSrc, $patchedC) + $unitFixedCs + $rtObjs
    Invoke-Gcc $gccArgs (Join-Path $log "$($stage.Name).link.log")

    $exeDumpPort = Join-Path $stageOut "p0_dump_driver_port.exe"
    $gccArgs = @("-std=c99", "-Wall", "-Wextra", "-Werror=incompatible-pointer-types", "-Werror=implicit-function-declaration",
                 "-DLM_THREAD_PROVIDER=LM_THREAD_PROVIDER_SINGLE") + $defines.ToArray() + $allocCounterDefines +
               @("-I", ".", "-I", "lm1\build", "-I", $rtHeaderRoot, "-o", $exeDumpPort, $dumpDriverSrc, $patchedC) + $unitFixedCs + $rtObjs + @($allocCounterObj)
    Invoke-Gcc $gccArgs (Join-Path $log "$($stage.Name).dumplink.log")

    # -- Every golden must produce byte-identical output on both,
    #    through both drivers. --
    $n = (Get-ChildItem $goldenDir -Filter "*.lmx").Count
    $mismatchLines = @(Test-GoldensBetween $exeRef $exePort $stageOut "meta") + @(Test-GoldensBetween $exeDumpRef $exeDumpPort $stageOut "dump")
    if ($mismatchLines.Count -gt 0) {
        $mismatchLines | ForEach-Object { Write-Output $_ }
        throw "stage $($stage.Name): $($mismatchLines.Count) mismatches across $n goldens x 2 drivers"
    }
    Write-Output "stage $($stage.Name) ok: $n/$n goldens identical (ref vs port), both drivers"
}

Write-Output "run_port_parser ok"
