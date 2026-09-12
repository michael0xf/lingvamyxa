# Candidate-only integration of authoritative L2 scanners into frozen L1 parsing.
# Each caller supplies a dedicated scanner Message; no global/TLS owner bridge.
param([string]$OracleEvidence)
$ErrorActionPreference='Stop'
$rootBaseline=Split-Path -Parent $PSScriptRoot
$repo=Split-Path -Parent (Split-Path -Parent $rootBaseline)
$compilerRun=Join-Path $repo 'build/codex/l2_nested_continue/20260912_012000/run_012133_172'
$compilerProof=Get-Content (Join-Path $compilerRun 'evidence.json') -Raw | ConvertFrom-Json
$l2exe=Join-Path $compilerRun 'source/stg/l1_baseline/build/nested_control/l2trans.exe'
$l1trans=$compilerProof.compiler
$pin='65D5A5ED127CA1BAEBDD1D500A5B74CEEA63EC1985EAC52EDEF28EFEB261C936'
if((Get-FileHash $l1trans).Hash -ne $pin) {throw 'Stable compiler pin mismatch'}
$l2key=(Resolve-Path $l2exe).Path
if((Get-FileHash $l2exe).Hash -ne $compilerProof.artifacts.$l2key) {throw 'Saved L2 compiler changed'}
if((Get-FileHash (Join-Path $PSScriptRoot 'l2trans.lm1')).Hash -ne $compilerProof.sources.'l2trans.lm1') {throw 'Saved compiler source no longer current'}
$objects=@($compilerProof.reusedObjects.PSObject.Properties | ForEach-Object Name)
foreach($obj in $objects) {if((Get-FileHash $obj).Hash -ne $compilerProof.reusedObjects.$obj){throw "Changed runtime object $obj"}}
$headers=Join-Path (Split-Path -Parent $objects[0]) 'headers'
$run=Join-Path $repo ('build/codex/candidate_c_scanners/'+(Get-Date -Format 'yyyyMMdd_HHmmss_fff'))
$sourceRoot=Join-Path $run 'source'
New-Item -ItemType Directory -Path $sourceRoot -Force | Out-Null
git -C $repo archive --format=zip "--output=$run/core.zip" eaac7c5 -- stg/l1_baseline/l1src stg/l1_baseline/l2src stg/l1_baseline/lm1/build/l1src/p0.lm1.h
if($LASTEXITCODE -ne 0){throw 'Archive failed'}
Expand-Archive -LiteralPath "$run/core.zip" -DestinationPath $sourceRoot
$work=Join-Path $sourceRoot 'stg/l1_baseline'
foreach($file in $compilerProof.sources.PSObject.Properties.Name){
    $saved=Join-Path $compilerRun "source/stg/l1_baseline/l2src/$file"
    if((Get-FileHash $saved).Hash -ne $compilerProof.sources.$file){throw "Changed saved compiler overlay $file"}
    Copy-Item -LiteralPath $saved -Destination (Join-Path $work "l2src/$file")
}
$out='build/c_scanners'
New-Item -ItemType Directory -Path (Join-Path $work $out) -Force | Out-Null
$evidence=[ordered]@{result='FAIL';stages=@();compiler=$l1trans;compilerSHA256=$pin;l2Compiler=$l2exe;compilerEvidence=(Join-Path $compilerRun 'evidence.json');snapshotOverlays=$compilerProof.sources;sources=@{};reusedObjects=$compilerProof.reusedObjects}
$inputs=@('parser_c_quoted.lm2','parser_c_surface.lm2','parser_text_predicates.lm2','tests/l2_c_scanners_parse_driver.lm1','run_candidate_c_scanners.ps1')
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
    $found=[regex]::Matches($text,'(?ms)^fn: '+[regex]::Escape($name)+'\b.*?(?=^fn: |\z)')
    if($found.Count -ne 1){throw "Missing/duplicate L2 definition $name"}
    return $found[0].Value
}
$oldLocation=Get-Location
try {
    Set-Location $work
    $q=(Get-Content 'l2src/parser_c_quoted.lm2' -Raw).Replace("\r\n","\n")
    $unit=$q.Substring(0,$q.IndexOf('fn: main ()'))
    $pred=Get-Content 'l2src/parser_text_predicates.lm2' -Raw
    foreach($name in @('lm_p0_is_horizontal_space','lm_p0_is_field_space','lm_p0_is_field_separator')){$unit+=Definition $pred $name}
    $surface=Get-Content 'l2src/parser_c_surface.lm2' -Raw
    $unit+=$surface.Substring(0,$surface.IndexOf('fn: main ()'))
    # Observe real L2 entry, including calls between L2 helpers. Counting
    # only L1 adapters misses helpers reached entirely inside the L2 unit.
    $hitNames=@('lm_p0_scan_c_quoted_token','lm_p0_starts_c_prefixed_quote','lm_p0_scan_c_char_token','lm_p0_scan_c_prefixed_quote_token','lm_p0_starts_c_surface_atom','lm_p0_is_c_surface_top_boundary','lm_p0_scan_c_sizeof_surface_atom','lm_p0_scan_c_surface_atom')
    foreach($i in 0..7){
        $header=[regex]::Match($unit,'(?m)^fn: '+$hitNames[$i]+'[^\r\n]*\r?\n')
        if(-not $header.Success){throw 'Missing instrumented L2 entry'}
        $unit=$unit.Insert($header.Index+$header.Length,'    scanner_hit('+$i+')'+[char]10)
    }
    $hit="fn: scanner_hit (int: which) int"+[char]10
    foreach($i in 0..7){$hit+="    int: hit$i"+[char]10}
    foreach($i in 0..7){$hit+="    if: which = $i"+[char]10+"        hit"+$i+": hit$i + 1"+[char]10}
    $hit+="    return: "+((0..7|ForEach-Object{"hit$_"}) -join ' + ')+[char]10+"end: scanner_hit"+[char]10
    $unit+=$hit+"fn: main () int"+[char]10+"    return: 0"+[char]10+"end: main"+[char]10
    [IO.File]::WriteAllText((Join-Path $work "$out/scanners.lm2"),$unit)
    & $l2exe "$out/scanners.lm2" "$out/scanners.lm1" *> "$out/scanners.translate.log"
    Check 'scanners_L2_to_L1'
    $generated=Get-Content "$out/scanners.lm1" -Raw
    foreach($i in 0..7){
        $slot=16+$i
        if($generated -notmatch ('# const: @\(char l2_own'+$slot+'\) "hit'+$i+'"')){throw 'Hit-counter layout changed'}
    }
    # Call the authentic generated entry directly with the caller's Message.
    # Its unused outer main is renamed only at native object compilation.
    & $l1trans "$out/scanners.lm1" "$out/scanners.c" *> "$out/scanners.c.log"
    Check 'scanners_L1_to_C'
    $cflags=@('-std=c99','-Wall','-Wextra','-Wpedantic','-I','.', '-I','lm1/build','-I',$headers,'-Werror=incompatible-pointer-types','-Werror=discarded-qualifiers','-Werror=implicit-function-declaration','-Werror=implicit-int')
    $evidence.cflags=$cflags
    & gcc @cflags -Dmain=l2_scanners_unused_main -c "$out/scanners.c" -o "$out/scanners.o" *> "$out/scanners.o.log"
    Check 'scanners_object'
    $parser=(Get-Content 'l1src/parser.lm1' -Raw).Replace("$([char]13)$([char]10)",[string][char]10)
    $adapters=@(
        @{name='lm_p0_scan_c_quoted_token';method=2;ret='size_t';args='text, end_index, quote_index'},
        @{name='lm_p0_starts_c_prefixed_quote';method=3;ret='int';args='text, end_index, start'},
        @{name='lm_p0_scan_c_char_token';method=4;ret='size_t';args='text, end_index, start'},
        @{name='lm_p0_scan_c_prefixed_quote_token';method=5;ret='size_t';args='text, end_index, start'},
        @{name='lm_p0_starts_c_surface_atom';method=9;ret='int';args='text, end_index, start'},
        @{name='lm_p0_is_c_surface_top_boundary';method=10;ret='int';args='value'},
        @{name='lm_p0_scan_c_sizeof_surface_atom';method=11;ret='size_t';args='text, end_index, start'},
        @{name='lm_p0_scan_c_surface_atom';method=12;ret='size_t';args='text, end_index, start'}
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
    $extra='include: "l2src/lmx.h"'+[char]10+'prototype:'+[char]10+'    fn: l2_m13 (@: Lmx unit; int: which) int'+[char]10
    $index=0
    foreach($adapter in $adapters){
        $name='ctx_'+$adapter.name
        $pattern='(?ms)^fn: '+$name+'\b.*?^end: '+$name+'\r?$'
        $matches=[regex]::Matches($candidate,$pattern)
        if($matches.Count -ne 1){throw "Missing adapter definition $name"}
        $header=($matches[0].Value -split '\n')[0]
        $extra+='    '+($header -replace ('^fn: '+$name),('fn: l2_m'+$adapter.method))+[char]10
        $body=$header+[char]10+'    return: c.l2_m'+$adapter.method+'(scanner_unit, '+$adapter.args+')'+[char]10+'end: '+$name
        $candidate=$candidate.Remove($matches[0].Index,$matches[0].Length).Insert($matches[0].Index,$body)
        $index++
    }
    $candidate=$extra+'end: prototype'+[char]10+$candidate
    [IO.File]::WriteAllText((Join-Path $work "$out/parser_candidate.lm1"),$candidate)
    foreach($flavor in @('oracle','candidate')){
        if($flavor -eq 'oracle' -and $OracleEvidence){
            $cached=Get-Content $OracleEvidence -Raw | ConvertFrom-Json
            if($cached.compilerSHA256 -ne $pin -or ($cached.snapshotOverlays | ConvertTo-Json -Compress) -ne ($compilerProof.sources | ConvertTo-Json -Compress)){throw 'Oracle cache profile changed'}
            foreach($source in $cached.sources.PSObject.Properties){
                if($source.Name -match '[\\/]l1src[\\/]' -and (Get-FileHash $source.Name).Hash -ne $source.Value){throw 'Oracle cache frozen source changed'}
            }
            $cachedRoot=Join-Path (Split-Path -Parent $OracleEvidence) 'source/stg/l1_baseline'
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
        & gcc @cflags -c "$out/parser_$flavor.c" -o "$out/parser_$flavor.o" *> "$out/parser_$flavor.o.log"
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
    & $l1trans 'l2src/tests/l2_c_scanners_parse_driver.lm1' "$out/driver.c" *> "$out/driver.translate.log"
    Check 'driver_L1_to_C'
    & gcc @cflags "$out/driver.c" "$out/scanners.o" "$out/parser_oracle.o" "$out/parser_candidate_namespaced.o" @objects '-Wl,--wrap=free' '-Wl,--wrap=lmx_node_new_owned' '-Wl,--wrap=lmx_method_new_owned' -o "$out/driver.exe" *> "$out/driver.gcc.log"
    Check 'driver_link'
    & "$out/driver.exe" *> "$out/driver.run.log"
    Check 'native_parity_context_cleanup'
    $result=Get-Content "$out/driver.run.log" -Raw
    if($result -notmatch 'candidate scanner parity cases=(\d+) freed=(\d+) PASS'){throw 'Missing native proof'}
    $evidence.cases=[int]$Matches[1];$evidence.freed=[int]$Matches[2]
    $evidence.result='PASS'
    Write-Output $result.Trim()
} finally {
    Set-Location $oldLocation
    $evidence.artifacts=@{}
    Get-ChildItem (Join-Path $work $out) -File -Recurse | ForEach-Object{$evidence.artifacts[$_.FullName]=(Get-FileHash $_.FullName).Hash}
    foreach($obj in $objects){if((Get-FileHash $obj).Hash -ne $compilerProof.reusedObjects.$obj){throw "Changed reused object $obj"}}
    $evidence | ConvertTo-Json -Depth 7 | Set-Content "$run/evidence.json" -Encoding utf8
    Write-Output "Candidate scanner evidence: $run result=$($evidence.result)"
}
