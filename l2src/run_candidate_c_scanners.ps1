# Candidate-only integration of authoritative L2 scanners into frozen L1 parsing.
# Each caller supplies a dedicated scanner Message; no global/TLS owner bridge.
param([string]$OracleEvidence,[switch]$FocusedBracket)
$ErrorActionPreference='Stop'
$rootBaseline=Split-Path -Parent $PSScriptRoot
$repo=$rootBaseline
# Everything this gate measures comes from the tree: the pinned L1 compiler, the
# current l2trans, HEAD's l1src/l2src/p0.lm1.h with the listed inputs copied from
# the working tree, and runtime objects built from the tree by run_l2trans's own
# builder (routed through the L2 runtime units). It used to archive eaac7c5,
# overlay saved compiler sources from one machine's build/codex directory and
# borrow runtime objects from build/fable/graph_abi evidence.
$l1trans=Join-Path $rootBaseline 'build/l1trans/gen2/l1trans.exe'
$pin = (Get-Content -LiteralPath (Join-Path $PSScriptRoot 'L1_PIN.txt') -TotalCount 1).Trim()
if ($pin -notmatch '^[0-9A-F]{64}$') { throw "L1_PIN.txt must hold one 64-hex SHA256, got 'pin=$pin'" }
if((Get-FileHash $l1trans).Hash -ne $pin) {throw 'Stable compiler pin mismatch'}
$run=Join-Path $repo ('build/codex/candidate_c_scanners/'+(Get-Date -Format 'yyyyMMdd_HHmmss_fff'))
$sourceRoot=Join-Path $run 'source'
New-Item -ItemType Directory -Path $sourceRoot -Force | Out-Null
# One bound for the whole runner, not for one child of it. 2026-09-16: the archive
# bound below fired at 120029 ms, taskkilled its child, wrote result=FAIL and threw --
# and this runner still did not exit for 28 minutes, holding the gate chain with
# nothing of its own alive. The bound covered the child; nothing covered the runner.
# The watchdog is a detached process holding a HANDLE on this one, so it can never
# kill a reused pid, and it exits by itself the moment this runner exits -- there is
# no disarm, because the hang to be covered happens after the last finally can run.
# Kill() terminates with exit code -1, and run_gates scores any nonzero code FAIL, so
# a bounded kill records this gate red; an exit that merely happened could read green.
function Start-RunnerBound([int]$Seconds) {
    $watch='$p=[System.Diagnostics.Process]::GetProcessById('+$PID+');$null=$p.Handle;if(-not $p.WaitForExit('+($Seconds*1000)+')){$p.Kill()}'
    Start-Process -FilePath 'powershell' -WindowStyle Hidden -PassThru -ArgumentList @('-NoProfile','-NonInteractive','-ExecutionPolicy','Bypass','-Command',$watch)
}
# 600 s is not a round number picked for comfort: this gate finished in 22, 22 and 28
# seconds in the three most recent complete chains (build/gates/20260915_212132,
# _211217, _190756, whose whole 31-row chains spanned 455-493 s), so the bound is over
# twenty times the measured run and cannot redden a healthy gate on a loaded machine --
# while still ending a hang in ten minutes rather than the twenty-eight of 2026-09-16.
# LMX_C_SCANNERS_BOUND_S exists so the tripwire can prove this bound fires without
# waiting out the real one; unset, the gate keeps its own bound.
$stepBoundSeconds=if($env:LMX_C_SCANNERS_BOUND_S){[int]$env:LMX_C_SCANNERS_BOUND_S}else{600}
$null=Start-RunnerBound $stepBoundSeconds
function Get-TreeRuntimeObjects {
    # A private scope: run_l2trans's variables ($out, $cflags, ...) stay inside.
    . (Join-Path $PSScriptRoot 'run_l2trans.ps1') -BuildOnly -OutputDirectory 'build/c_scanners_runtime' -TranslatorPath $l1trans
    # Its object paths are relative to the repo root; this gate compiles and
    # links from inside its snapshot, so make them rooted.
    @(Get-L2MessageObjects) | ForEach-Object { if ([IO.Path]::IsPathRooted($_)) { $_ } else { Join-Path $rootBaseline $_ } }
}
$objects=@(Get-TreeRuntimeObjects)
if($objects.Count -eq 0){throw 'No runtime objects built'}
$headers=Join-Path (Split-Path -Parent $objects[0]) 'headers'
$runtimeObjectHashes=[ordered]@{}
foreach($obj in $objects){$runtimeObjectHashes[$obj]=(Get-FileHash $obj).Hash}
# Build the current frontend with the pinned stable L1 compiler.
$currentL2C=Join-Path $run 'l2trans.current.c'
$currentL2Exe=Join-Path $run 'l2trans.current.exe'
$savedLocation=Get-Location
try {
    Set-Location $rootBaseline
    & $l1trans 'l2src/l2trans.lm1' $currentL2C *> (Join-Path $run 'l2trans.current.translate.log')
    if($LASTEXITCODE -ne 0){throw 'Current l2trans L1-to-C failed'}
    $prevEap=$ErrorActionPreference
    $ErrorActionPreference='Continue'
    & gcc -std=c99 -Wall -Wextra -Wpedantic -I . -I lm1/build $currentL2C l2src/lmx_poll_stub.c -o $currentL2Exe *> (Join-Path $run 'l2trans.current.gcc.log')
    $ErrorActionPreference=$prevEap
    if($LASTEXITCODE -ne 0){throw 'Current l2trans compile failed'}
} finally {
    Set-Location $savedLocation
}
$l2exe=$currentL2Exe
# The archive has hung intermittently (GATE_ARCHIVE_HANG.txt): run it as a child with its own output
# files and a 120 s bound, so a stall fails this gate with its name instead of holding the chain.
$archiveOut=Join-Path $run 'archive_core.stdout.txt'
$archiveErr=Join-Path $run 'archive_core.stderr.txt'
$archiveArgs=@('-C',"`"$repo`"",'archive','--format=zip',"`"--output=$run/core.zip`"",'HEAD','--','l1src','l2src','lm1/build/l1src/p0.lm1.h')
$archiveStart=Get-Date
$archive=Start-Process -FilePath 'git' -ArgumentList $archiveArgs -NoNewWindow -PassThru -RedirectStandardOutput $archiveOut -RedirectStandardError $archiveErr
# Read the handle now, so ExitCode is still valid after the process has exited.
$null=$archive.Handle
if(-not $archive.WaitForExit(120000)){
    $archiveMs=[int]((Get-Date)-$archiveStart).TotalMilliseconds
    $timeoutLine=('Archive timed out after 120 s (git pid {0}, started {1:HH:mm:ss.fff})' -f $archive.Id,$archiveStart)
    # cmd /c: under EAP Stop, taskkill's stderr (a process already gone) would throw here instead.
    cmd /c "taskkill /PID $($archive.Id) /T /F >nul 2>&1"
    [ordered]@{result='FAIL';failure=$timeoutLine;archiveMs=$archiveMs;archiveStdout=$archiveOut;archiveStderr=$archiveErr} | ConvertTo-Json | Set-Content "$run/evidence.json" -Encoding utf8
    # This is exactly where 2026-09-16's hold sat: the bound had fired, the child was
    # killed, evidence said FAIL -- and the process was still alive 28 minutes later.
    # Print the verdict text ourselves, because the error rendering may never reach the
    # log if the exit is what hangs, then arm a short bound so this runner dies with a
    # nonzero code within seconds of its own throw instead of holding the chain.
    Write-Output $timeoutLine
    $null=Start-RunnerBound 20
    throw ($timeoutLine+'; evidence '+$run)
}
$archive.WaitForExit()
$archiveMs=[int]((Get-Date)-$archiveStart).TotalMilliseconds
if($archive.ExitCode -ne 0){
    Get-Content -LiteralPath $archiveOut,$archiveErr
    throw ('Archive failed with exit {0}' -f $archive.ExitCode)
}
Expand-Archive -LiteralPath "$run/core.zip" -DestinationPath $sourceRoot
$work=$sourceRoot
$out='build/c_scanners'
New-Item -ItemType Directory -Path (Join-Path $work $out) -Force | Out-Null
$evidence=[ordered]@{result='FAIL';stages=@();compiler=$l1trans;compilerSHA256=$pin;l2Compiler=$l2exe;coreCommit=((git -C $repo rev-parse HEAD) -join '');archiveMs=$archiveMs;currentL2TranslatorSourceSHA256=(Get-FileHash (Join-Path $PSScriptRoot 'l2trans.lm1')).Hash;sources=@{};reusedObjects=$runtimeObjectHashes}
$inputs=@('parser_c_quoted.lm2','parser_c_surface.lm2','parser_text_predicates.lm2','parser_position.lm2','parser_c_quote_diagnostics.lm2','parser_python_string.lm2','parser_python_diagnostics.lm2','parser_quoted_diagnostics.lm2','parser_dash_fence.lm2','parser_matching_paren.lm2','parser_matching_bracket.lm2','lmx.h','lmx_message.h','lmx_array_ref_owned.h.lm1','tests/l2_c_scanners_parse_driver.lm1','run_candidate_c_scanners.ps1')
foreach($file in $inputs){
    $src=Join-Path $PSScriptRoot $file
    Copy-Item -LiteralPath $src -Destination (Join-Path $work "l2src/$file")
    $evidence.sources[$src]=(Get-FileHash $src).Hash
}
foreach($file in @('parser.lm1','parser_text.lm1','own.lm1')){
    $src=Join-Path $rootBaseline "l1src/$file"
    $evidence.sources[$src]=(Get-FileHash $src).Hash
    if((Get-FileHash (Join-Path $work "l1src/$file")).Hash -ne $evidence.sources[$src]){throw "Frozen source drift $file"}
}
function Check([string]$name){
    $evidence.stages+=@{name=$name;exit=$LASTEXITCODE}
    if($LASTEXITCODE -ne 0){throw "$name exit $LASTEXITCODE"}
}
function Definition([string]$text,[string]$name){
    $found=[regex]::Matches($text,'(?ms)^(?:fn|sub): '+[regex]::Escape($name)+'\b.*?(?=^(?:fn|sub): |\z)')
    if($found.Count -ne 1){throw "Missing/duplicate L2 definition $name"}
    return $found[0].Value
}
$oldLocation=Get-Location
try {
    Set-Location $work
    & $l1trans 'l2src/lmx_array_ref_owned.h.lm1' 'l2src/lmx_array_ref_owned.lm1.h' *> "$out/lmx_array_ref_owned.header.log"
    Check 'current_array_ref_header'
    $q=(Get-Content 'l2src/parser_c_quoted.lm2' -Raw).Replace("\r\n","\n")
    $unit=$q.Substring(0,$q.IndexOf('fn: main ()'))
    $pred=Get-Content 'l2src/parser_text_predicates.lm2' -Raw
    foreach($name in @('lm_p0_is_horizontal_space','lm_p0_is_field_space','lm_p0_is_field_separator')){$unit+=Definition $pred $name}
    $surface=Get-Content 'l2src/parser_c_surface.lm2' -Raw
    $unit+=$surface.Substring(0,$surface.IndexOf('fn: main ()'))
    # Observe real L2 entry, including calls between L2 helpers. Counting
    # only L1 adapters misses helpers reached entirely inside the L2 unit.
    $hitNames=@('lm_p0_scan_c_quoted_token','lm_p0_starts_c_prefixed_quote','lm_p0_scan_c_char_token','lm_p0_scan_c_prefixed_quote_token','lm_p0_starts_c_surface_atom','lm_p0_is_c_surface_top_boundary','lm_p0_scan_c_sizeof_surface_atom','lm_p0_scan_c_surface_atom')
    $diagnostics=Get-Content 'l2src/parser_c_quote_diagnostics.lm2' -Raw
    $hitNames+=@('lm_p0_scan_c_char','lm_p0_scan_c_prefixed_quote')
    $hitNames+=@('lm_p0_is_line_break','lm_p0_line_break_width_at','lm_p0_is_horizontal_space','lm_p0_is_field_space','lm_p0_is_field_separator','lm_p0_position_in_slice')
    $hitNames+=@('lm_p0_starts_python_string','lm_p0_find_python_string_end','lm_p0_skip_python_string_unchecked','lm_p0_scan_python_string')
    $hitNames+=@('lm_p0_scan_quoted','lm_p0_require_quoted_token_boundary','lm_p0_is_quoted_token_boundary')
    $hitNames+=@('lm_p0_find_matching_paren')
    $hitNames+=@('lm_p0_scan_brace_mark_unchecked','lm_p0_find_matching_bracket')
    $hit="fn: scanner_hit (int: which) int"+[char]10
    foreach($i in 0..25){$hit+="    int: hit$i"+[char]10}
    foreach($i in 0..25){$hit+="    if: which = $i"+[char]10+"        hit"+$i+": hit$i + 1"+[char]10}
    $hit+="    return: 0"+[char]10+"end: scanner_hit"+[char]10
    $unit+=$hit+(Definition (Get-Content 'l2src/parser_position.lm2' -Raw) 'lm_p0_position_in_slice')
    $unit+=$diagnostics
    $python=Get-Content 'l2src/parser_python_string.lm2' -Raw
    foreach($name in @('lm_p0_starts_python_string','lm_p0_find_python_string_end')){$unit+=Definition $python $name}
    $unit+=(Get-Content 'l2src/parser_python_diagnostics.lm2' -Raw)
    $unit+=Definition $pred 'lm_p0_is_quoted_token_boundary'
    $unit+=(Get-Content 'l2src/parser_quoted_diagnostics.lm2' -Raw)
    $unit+=(Get-Content 'l2src/parser_matching_paren.lm2' -Raw)
    $brace=Get-Content 'l2src/parser_dash_fence.lm2' -Raw
    foreach($name in @('lm_p0_index_is_line_start','lm_p0_find_physical_line_end','lm_p0_line_rest_is_horizontal_space','lm_p0_match_block_string_fence_line','lm_p0_match_raw_comment_fence_line','lm_p0_skip_fence_block_unchecked')){$unit+=Definition $brace $name}
    $braceScan=Definition $brace 'lm_p0_scan_brace_mark_unchecked'
    # The current diagnostic-aware L2 Python helper exposes its temporary end
    # slot explicitly; adapt the older brace scanner body without changing its
    # frozen behavior.
    $braceScan=$braceScan.Replace('@: int closed) size_t','@: int closed; @: size_t python_end) size_t')
    $braceScan=$braceScan.Replace('lm_p0_skip_python_string_unchecked(text, length, i)','lm_p0_skip_python_string_unchecked(text, length, i, python_end)')
    $unit+=$braceScan
    $unit+=(Get-Content 'l2src/parser_matching_bracket.lm2' -Raw)+"fn: main () int"+[char]10+"    return: 0"+[char]10+"end: main"+[char]10
    foreach($i in 0..25){
        $headersFound=[regex]::Matches($unit,'(?m)^(?:fn|sub): '+$hitNames[$i]+'\s*\([^\r\n]*\r?\n')
        if($headersFound.Count -ne 1){throw 'Missing/duplicate instrumented L2 entry'}
        $header=$headersFound[0]
        $unit=$unit.Insert($header.Index+$header.Length,'    scanner_hit('+$i+')'+[char]10)
    }
    [IO.File]::WriteAllText((Join-Path $work "$out/scanners.lm2"),$unit)
    & $l2exe "$out/scanners.lm2" "$out/scanners.lm1" *> "$out/scanners.translate.log"
    Check 'scanners_L2_to_L1'
    $generated=Get-Content "$out/scanners.lm1" -Raw
    foreach($i in 0..25){
        $slot=16+$i
        if($generated -notmatch ('# const: @\(char l2_own'+$slot+'\) "hit'+$i+'"')){throw 'Hit-counter layout changed'}
    }
    # Call the authentic generated entry directly with the caller's Message.
    # Its unused outer main is renamed only at native object compilation.
    & $l1trans "$out/scanners.lm1" "$out/scanners.c" *> "$out/scanners.c.log"
    Check 'scanners_L1_to_C'
    $cflags=@('-std=c99','-Wall','-Wextra','-Wpedantic','-I','.', '-I','lm1/build','-I',$headers,'-Werror=incompatible-pointer-types','-Werror=discarded-qualifiers','-Werror=implicit-function-declaration','-Werror=implicit-int')
    $evidence.cflags=$cflags
    # Keep the generated diagnostic helper private to this scanner object;
    # the oracle and namespaced candidate retain their own frozen helpers.
    # gcc warnings on stderr must not become terminating NativeCommandError.
    $prevEap=$ErrorActionPreference
    $ErrorActionPreference='Continue'
    & gcc @cflags -Dmain=l2_scanners_unused_main -Dlm_p0_set_diagnostic=l2_scanners_set_diagnostic -c "$out/scanners.c" -o "$out/scanners.o" *> "$out/scanners.o.log"
    $ErrorActionPreference=$prevEap
    Check 'scanners_object'
    $parser=(Get-Content 'l1src/parser.lm1' -Raw).Replace("$([char]13)$([char]10)",[string][char]10)
    # Five helpers live in the parser_text predef, not parser.lm1. Inline its
    # frozen function bodies into the isolated copy so both files' transitive
    # callers receive an explicit unit; never edit the shared predef.
    $textSupport=Get-Content 'l1src/parser_text.lm1' -Raw
    $textSupport=$textSupport.Substring($textSupport.IndexOf('fn: '))
    $parser=$parser.Replace('predef: "l1src/parser_text.lm1"','')
    $parser=$parser.Insert($parser.IndexOf('end: prototype')+14,[char]10+$textSupport+[char]10)
    $adapters=@(
        @{name='lm_p0_scan_c_quoted_token';method=2;ret='size_t';args='text, end_index, quote_index'},
        @{name='lm_p0_starts_c_prefixed_quote';method=3;ret='int';args='text, end_index, start'},
        @{name='lm_p0_scan_c_char_token';method=4;ret='size_t';args='text, end_index, start'},
        @{name='lm_p0_scan_c_prefixed_quote_token';method=5;ret='size_t';args='text, end_index, start'},
        @{name='lm_p0_starts_c_surface_atom';method=9;ret='int';args='text, end_index, start'},
        @{name='lm_p0_is_c_surface_top_boundary';method=10;ret='int';args='value'},
        @{name='lm_p0_scan_c_sizeof_surface_atom';method=11;ret='size_t';args='text, end_index, start'},
        @{name='lm_p0_scan_c_surface_atom';method=12;ret='size_t';args='text, end_index, start'},
        @{name='lm_p0_scan_c_char';method=15;ret='int';args='document, text, length, index, line, base_column';scratch=@('diagnostic_line','diagnostic_column')},
        @{name='lm_p0_scan_c_prefixed_quote';method=16;ret='int';args='document, text, length, index, line, base_column';scratch=@('diagnostic_line','diagnostic_column')},
        @{name='lm_p0_is_line_break';method=0;ret='int';args='value'},
        @{name='lm_p0_line_break_width_at';method=1;ret='size_t';args='source, length, index'},
        @{name='lm_p0_is_horizontal_space';method=6;ret='int';args='value'},
        @{name='lm_p0_is_field_space';method=7;ret='int';args='value'},
        @{name='lm_p0_is_field_separator';method=8;ret='int';args='value'},
        @{name='lm_p0_position_in_slice';method=14;ret='void';args='text, length, index, base_line, base_column, out_line, out_column'},
        @{name='lm_p0_starts_python_string';method=17;ret='int';args='text, length, index'},
        @{name='lm_p0_find_python_string_end';method=18;ret='int';args='text, length, start, out_end'},
        @{name='lm_p0_skip_python_string_unchecked';method=19;ret='size_t';args='text, length, start';scratch=@('end_index')},
        @{name='lm_p0_scan_python_string';method=20;ret='int';args='document, text, length, index, line, base_column';scratch=@('end_index','diagnostic_line','diagnostic_column')},
        @{name='lm_p0_is_quoted_token_boundary';method=21;ret='int';args='value'},
        @{name='lm_p0_scan_quoted';method=22;ret='int';args='document, text, length, index, quote, line, base_column';scratch=@('end_index','diagnostic_line','diagnostic_column')},
        @{name='lm_p0_require_quoted_token_boundary';method=23;ret='int';args='document, text, length, index, line, column';scratch=@('diagnostic_line','diagnostic_column')},
        @{name='lm_p0_find_matching_paren';method=24;ret='int';args='document, text, length, open_index, line, base_column, close_index';scratch=@('cursor','end_index','diagnostic_line','diagnostic_column')}
        @{name='lm_p0_scan_brace_mark_unchecked';method=31;ret='size_t';args='text, length, start, closed';scratch=@('python_end')},
        @{name='lm_p0_find_matching_bracket';method=32;ret='int';args='document, text, length, open_index, line, base_column, close_index';scratch=@('cursor','end_index',@{name='closed';type='int'},'diagnostic_line','diagnostic_column')}
    )
    $definitions=[regex]::Matches($parser.Substring($parser.IndexOf('end: prototype')+14),'(?ms)^(?:    )?(?:fn|sub): (\w+)\b.*?(?=^(?:    )?(?:fn|sub): |\z)')
    $routed=[Collections.Generic.HashSet[string]]::new()
    foreach($adapter in $adapters){[void]$routed.Add($adapter.name)}
    do {
        $changed=$false
        foreach($def in $definitions){
            $name=$def.Groups[1].Value
            if($routed.Contains($name)){continue}
            foreach($callee in @($routed)){
                if($def.Value -match ('\b'+$callee+'\(')){[void]$routed.Add($name);$changed=$true;break}
            }
        }
    }while($changed)
    $evidence.routedFunctions=@($routed|Sort-Object)
    $names=(@($routed|Sort-Object -Descending)|ForEach-Object{[regex]::Escape($_)}) -join '|'
    # Preserve quoted strings/comments; rewrite only headers, end labels and calls.
    $candidateLines=foreach($line in ($parser -split '\n')){
        if($line -match ('^(\s*(?:fn|sub): )('+ $names +')\s*\(')){
            $line=[regex]::Replace($line,'^(\s*(?:fn|sub): )('+ $names +')\s*\(','$1ctx_$2 (@: Lmx scanner_unit; ')
        } elseif($line -match ('^(\s*end: )('+ $names +')\s*$')){
            $line=[regex]::Replace($line,'^(\s*end: )('+ $names +')\s*$','$1ctx_$2')
        } else {
            $token='"(?:\\.|[^"\\])*"|''(?:\\.|[^''\\])*''|#[^\r\n]*|(?<call>\b(?:'+$names+')\s*\()'
            $line=[regex]::Replace($line,$token,{
                param($m)
                if(-not $m.Groups['call'].Success){return $m.Value}
                return 'ctx_'+$m.Value.TrimEnd('(').TrimEnd()+'(scanner_unit, '
            })
        }
        $line
    }
    $candidate=$candidateLines -join [char]10
    $extra='include: "l2src/lmx.h"'+[char]10+'prototype:'+[char]10+'    fn: lmx_branch_struct_known (@: Lmx parent; size_t: index) @: Lmx'+[char]10+'    fn: l2_m13 (@: Lmx unit; int: which) int'+[char]10
    $index=0
    foreach($adapter in $adapters){
        $name='ctx_'+$adapter.name
        # Frozen text helpers also use top-level return trailers without end.
        $pattern='(?ms)^(?:fn|sub): '+$name+'\b.*?(?=^(?:fn|sub): |\z)'
        $matches=[regex]::Matches($candidate,$pattern)
        if($matches.Count -ne 1){throw "Missing adapter definition $name"}
        $header=($matches[0].Value -split '\n')[0]
        $kind='fn'
        if($adapter.ret -eq 'void'){$kind='sub'}
        $prototype=$header -replace ('^'+$kind+': '+$name),($kind+': l2_m'+$adapter.method)
        $locals=''
        $actuals=$adapter.args
        if($adapter.scratch){
            foreach($scratch in $adapter.scratch){
                $scratchName=$scratch
                $scratchType='size_t'
                if($scratch -is [Collections.IDictionary]){$scratchName=$scratch.name;$scratchType=$scratch.type}
                $prototype=$prototype.Insert($prototype.LastIndexOf(')'),'; @: '+$scratchType+' '+$scratchName)
                $locals+='    '+$scratchType+': '+$scratchName+[char]10
                $actuals+=', @ '+$scratchName
            }
        }
        $extra+='    '+$prototype+[char]10
        $callPrefix='    return: '
        if($kind -eq 'sub'){$callPrefix='    '}
        # A generated method receives its callable Structure M.  M.node is the
        # lexical unit used by calls to sibling methods; passing the unit itself
        # aliases unrelated child slots as own-cache fields.
        $receiver='c.lmx_branch_struct_known(scanner_unit, '+$adapter.method+'U)'
        $body=$header+[char]10+$locals+$callPrefix+'c.l2_m'+$adapter.method+'('+$receiver+', '+$actuals+')'+[char]10+'end: '+$name+[char]10+[char]10
        $candidate=$candidate.Remove($matches[0].Index,$matches[0].Length).Insert($matches[0].Index,$body)
        $index++
    }
    $candidate=$extra+'end: prototype'+[char]10+$candidate
    [IO.File]::WriteAllText((Join-Path $work "$out/parser_candidate.lm1"),$candidate)
    foreach($flavor in @('oracle','candidate')){
        if($flavor -eq 'oracle' -and $OracleEvidence){
            $cached=Get-Content $OracleEvidence -Raw | ConvertFrom-Json
            if($cached.compilerSHA256 -ne $pin){throw 'Oracle cache profile changed'}
            foreach($source in $cached.sources.PSObject.Properties){
                if($source.Name -match '[\\/]l1src[\\/]' -and (Get-FileHash $source.Name).Hash -ne $source.Value){throw 'Oracle cache frozen source changed'}
            }
            $cachedRoot=Join-Path (Split-Path -Parent $OracleEvidence) 'source'
            $cachedScript=Get-Content (Join-Path $cachedRoot 'l2src/run_candidate_c_scanners.ps1') -Raw
            $currentScript=Get-Content $PSCommandPath -Raw
            if([regex]::Match($cachedScript,'(?m)^    \$cflags=.*$').Value -ne [regex]::Match($currentScript,'(?m)^    \$cflags=.*$').Value){throw 'Oracle cache C flags changed'}
            foreach($ext in @('o','c')){
                $artifact=(Resolve-Path (Join-Path $cachedRoot "$out/parser_oracle.$ext")).Path
                if((Get-FileHash $artifact).Hash -ne $cached.artifacts.$artifact){throw 'Oracle cache artifact changed'}
                Copy-Item -LiteralPath $artifact -Destination "$out/parser_oracle.$ext"
            }
            $evidence.oracleReuse=$OracleEvidence
            continue
        }
        $input='l1src/parser.lm1'
        if($flavor -eq 'candidate'){$input="$out/parser_candidate.lm1"}
        & $l1trans $input "$out/parser_$flavor.c" *> "$out/parser_$flavor.translate.log"
        Check "$($flavor)_L1_to_C"
        $prevEap=$ErrorActionPreference
        $ErrorActionPreference='Continue'
        & gcc @cflags -c "$out/parser_$flavor.c" -o "$out/parser_$flavor.o" *> "$out/parser_$flavor.o.log"
        $ErrorActionPreference=$prevEap
        Check "$($flavor)_object"
    }
    # Namespace the candidate's defined globals (including its frozen support
    # functions), preserving undefined foreign and real-L2 method references.
    $symbols=& nm --defined-only --extern-only "$out/parser_candidate.o"
    Check 'candidate_symbols'
    $renames=@($symbols|ForEach-Object{if($_ -match '^\S+\s+[A-Z]\s+(\S+)$'){$Matches[1]+' candidate_'+$Matches[1]}})
    if($renames.Count -lt 100){throw 'Unexpected candidate symbol inventory'}
    [IO.File]::WriteAllLines((Join-Path $work "$out/candidate.symbols"),$renames)
    & objcopy "--redefine-syms=$out/candidate.symbols" "$out/parser_candidate.o" "$out/parser_candidate_namespaced.o"
    Check 'candidate_namespace'
    # A hand-written L1 prototype of the generated entry adapter links by name
    # against its C definition, so a stale shape compiles, links and hands the
    # adapter garbage (found on the stage 5 (a) merge: a one-argument
    # l2_program_entry(owner) call ran as status 1). Every prototype-block
    # declaration of it in the tree's tests must have the adapter's shape.
    foreach($lm1 in @(Get-ChildItem -LiteralPath (Join-Path $PSScriptRoot 'tests') -Recurse -Filter '*.lm1')){
        $inPrototype=$false
        $lineNo=0
        foreach($line in [IO.File]::ReadAllLines($lm1.FullName)){
            $lineNo++
            if($line -match '^prototype:'){$inPrototype=$true;continue}
            if($line -match '^end: prototype'){$inPrototype=$false;continue}
            if($inPrototype -and $line -match '^\s+fn: l2_program_entry \(' -and $line -notmatch '^\s+fn: l2_program_entry \(@: LmxMsg process_message; @: int result(; int: argc; @@: char argv)?\) int\s*$'){
                throw "stale l2_program_entry prototype at $($lm1.FullName):$lineNo (the adapter is (@: LmxMsg process_message; @: int result)): $($line.Trim())"
            }
        }
    }
    & $l1trans 'l2src/tests/l2_c_scanners_parse_driver.lm1' "$out/driver.c" *> "$out/driver.translate.log"
    Check 'driver_L1_to_C'
    $prevEap=$ErrorActionPreference
    $ErrorActionPreference='Continue'
    & gcc @cflags "$out/driver.c" "$out/scanners.o" "$out/parser_oracle.o" "$out/parser_candidate_namespaced.o" @objects '-Wl,--wrap=free' '-Wl,--wrap=lmx_node_new_owned' '-Wl,--wrap=lmx_method_new_owned' -o "$out/driver.exe" *> "$out/driver.gcc.log"
    $ErrorActionPreference=$prevEap
    Check 'driver_link'
    $savedFocused=$env:LMX_FOCUSED_BRACKET
    if($FocusedBracket){$env:LMX_FOCUSED_BRACKET='1'}else{Remove-Item Env:LMX_FOCUSED_BRACKET -ErrorAction SilentlyContinue}
    try {
        & "$out/driver.exe" *> "$out/driver.run.log"
    } finally {
        if($null -eq $savedFocused){Remove-Item Env:LMX_FOCUSED_BRACKET -ErrorAction SilentlyContinue}else{$env:LMX_FOCUSED_BRACKET=$savedFocused}
    }
    Check 'native_parity_context_cleanup'
    $result=Get-Content "$out/driver.run.log" -Raw
    if($FocusedBracket){
        if($result -notmatch 'candidate bracket parity cases=(\d+) PASS'){throw 'Missing focused bracket proof'}
        $evidence.cases=[int]$Matches[1];$evidence.focus='matching_bracket'
    } else {
        if($result -notmatch 'candidate scanner parity cases=(\d+) freed=(\d+) PASS'){throw 'Missing native proof'}
        $evidence.cases=[int]$Matches[1];$evidence.freed=[int]$Matches[2]
    }
    $evidence.result='PASS'
    Write-Output $result.Trim()
} finally {
    Set-Location $oldLocation
    $evidence.artifacts=@{}
    Get-ChildItem (Join-Path $work $out) -File -Recurse | ForEach-Object{$evidence.artifacts[$_.FullName]=(Get-FileHash $_.FullName).Hash}
    foreach($obj in $objects){if((Get-FileHash $obj).Hash -ne $runtimeObjectHashes[$obj]){throw "Changed reused object $obj"}}
    $evidence | ConvertTo-Json -Depth 7 | Set-Content "$run/evidence.json" -Encoding utf8
    Write-Output "Candidate scanner evidence: $run result=$($evidence.result)"
}
