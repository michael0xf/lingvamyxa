# Reference-first P0 metadata oracle.
# 620 goldens from tracked parser.lm1.c + own.lm1.c in the immutable old checkout.
# Does not modify that checkout. Pretty printTree is not the metadata oracle.
$ErrorActionPreference = "Stop"
Set-Location (Join-Path $PSScriptRoot "..\..")

$oldRoot = "C:\Nyasha_Planet\lingvamyxa_old_worked_version"
$oldParser = Join-Path $oldRoot "lm1\build\parser.lm1.c"
$oldOwn = Join-Path $oldRoot "lm1\build\own.lm1.c"
$wantParser = "A344B975BD541757AB1CCA89FDC04DD9467C17D041B9A42BC3BE09205E254DDE"
$wantOwn = "8EF21B1AB0561916762829C07B6AD45B7E3A98BC42CA1E1F48D830B6B2C783FA"
if (-not (Test-Path $oldParser)) { throw "missing 620 parser C: $oldParser" }
$hp = (Get-FileHash -LiteralPath $oldParser).Hash
$ho = (Get-FileHash -LiteralPath $oldOwn).Hash
if ($hp -ne $wantParser) { throw "620 parser.lm1.c hash $hp expected $wantParser" }
if ($ho -ne $wantOwn) { throw "620 own.lm1.c hash $ho expected $wantOwn" }

$outDir = "build\p0_tree_contract"
New-Item -ItemType Directory -Force -Path $outDir | Out-Null
$dumpSrc = "tests\p0_tree_contract\p0_meta_dump.c"
$exe620 = Join-Path $outDir "p0_meta_dump_620.exe"
$exeCur = Join-Path $outDir "p0_meta_dump_cur.exe"
$log620 = Join-Path $outDir "cc_620.log"
$logCur = Join-Path $outDir "cc_cur.log"

cmd /c "gcc -std=c99 -Wall -Wextra -DLM_THREAD_PROVIDER=LM_THREAD_PROVIDER_SINGLE -DP0_META_ABI_620 -o $exe620 $dumpSrc `"$oldParser`" `"$oldOwn`" > $log620 2>&1"
if ($LASTEXITCODE -ne 0) {
    Get-Content $log620 | Select-Object -Last 40
    throw "gcc 620 p0_meta_dump failed"
}
# Current parser.lm1.c already contains own (predef); do not also link own.lm1.c.
cmd /c "gcc -std=c99 -Wall -Wextra -DLM_THREAD_PROVIDER=LM_THREAD_PROVIDER_SINGLE -I . -I lm1/build -o $exeCur $dumpSrc lm1\build\parser.lm1.c > $logCur 2>&1"
if ($LASTEXITCODE -ne 0) {
    Get-Content $logCur | Select-Object -Last 40
    throw "gcc current p0_meta_dump failed"
}

function Test-OrdinalEq([string]$a, [string]$b) {
    return [string]::Compare($a, $b, [System.StringComparison]::Ordinal) -eq 0
}

# Current-only TRAILER_COLON bit, by exact trailer order (colon_bit= fields in dump order).
# Count alone is not identity: F_mixed_trailer is 0,1 (bare return then colon end).
$expectCurColonSeq = @{
    "A_header_then_body" = "1"
    "F_eq_fence" = "1"
    "F_eq_inner_longer" = "1"
    "F_eq_long_opener" = "1"
    "F_mixed_trailer" = "0,1"
    "F_star_long" = "1"
    "G_fn_nested_cut" = "1"
    "G_fn_nested_end" = "1,1"
    "return_colon_vertical_body" = "1"
}
$expect620Reject = @{
    "C_nested_short" = "REJECT code=13 line=2 col=5 msg=source level increase must be one step"
    "invalid_eq_unclosed" = "REJECT code=20 line=2 col=1 msg=unterminated block string literal"
    "invalid_triple_unclosed" = "REJECT code=4 line=1 col=4 msg=unterminated python-like string literal"
}
$expectCurReject = @{
    "C_nested_short" = "REJECT code=13 line=2 col=5 msg=source level increase must be one step"
    "return_colon_empty_trailer" = "REJECT code=32 line=1 col=1 msg=empty colon Frame is not allowed"
    "return_colon_comment_trailer" = "REJECT code=32 line=1 col=1 msg=empty colon Frame is not allowed"
    "F_star_fence" = "REJECT code=32 line=1 col=1 msg=empty colon Frame is not allowed"
    "invalid_eq_unclosed" = "REJECT code=20 line=2 col=1 msg=unterminated block string literal"
    "invalid_triple_unclosed" = "REJECT code=4 line=1 col=4 msg=unterminated python-like string literal"
}

function Get-RejectLine([string]$text) {
    return ($text -split "`n" | Where-Object { $_ -like "REJECT *" } | Select-Object -First 1)
}

function Assert-DumpShape([string]$stem, [string]$side, [int]$exit, [string]$stdout, [string]$stderr, [string]$expectReject) {
    $stPath = Join-Path $outDir ($stem + "." + $side + ".status.txt")
    $st = "exit=$exit expectReject=$expectReject stderr_len=$($stderr.Length)"
    [IO.File]::WriteAllText((Join-Path (Get-Location) $stPath), $st + "`n")
    if ($stderr.Trim().Length -gt 0) { throw "$stem $side unexpected stderr: $stderr" }
    if (-not $stdout.StartsWith("# p0-meta v1")) { throw "$stem $side dump missing banner (crash?): exit=$exit" }
    if ($expectReject) {
        if ($exit -ne 1) { throw "$stem $side expected reject exit 1, got $exit" }
        $line = Get-RejectLine $stdout
        if (-not $line) { throw "$stem $side expected REJECT line, exit=$exit" }
        if (-not (Test-OrdinalEq $line.Trim() $expectReject)) {
            throw "$stem $side reject mismatch`n got=$line`n want=$expectReject"
        }
    } else {
        if ($exit -ne 0) { throw "$stem $side expected accept exit 0, got $exit (crash vs reject)" }
        if (Get-RejectLine $stdout) { throw "$stem $side unexpected REJECT on accept fixture" }
    }
}

function Get-ColonSeq([string]$dump) {
    $bits = [regex]::Matches($dump, "colon_bit=([01])") | ForEach-Object { $_.Groups[1].Value }
    return [string]($bits -join ",")
}

function Assert-CurrentVs620([string]$stem, [string]$got620, [string]$gotCur) {
    if ($expectCurReject.ContainsKey($stem)) { return }
    $seq = Get-ColonSeq $gotCur
    $n1 = ([regex]::Matches($gotCur, "colon_bit=1")).Count
    $n620_1 = ([regex]::Matches($got620, "colon_bit=1")).Count
    if ($n620_1 -ne 0) { throw "$stem 620 golden unexpectedly has colon_bit=1" }
    if ($expectCurColonSeq.ContainsKey($stem)) {
        $wantSeq = [string]$expectCurColonSeq[$stem]
        if ($seq -ne $wantSeq) { throw "$stem current colon_bit seq '$seq' expected '$wantSeq'" }
        $norm = $gotCur.Replace("colon_bit=1", "colon_bit=0")
        if (-not (Test-OrdinalEq $norm $got620)) {
            throw "$stem current != 620 after versioned colon_bit seq $wantSeq`n--- 620 ---`n$got620`n--- cur ---`n$gotCur"
        }
    } else {
        if ($n1 -ne 0) { throw "$stem spurious current colon_bit=1" }
        if (-not (Test-OrdinalEq $gotCur $got620)) {
            throw "$stem current metadata != 620 golden`n--- 620 ---`n$got620`n--- cur ---`n$gotCur"
        }
    }
}

$here = "tests\p0_tree_contract"
$writeGolden = $env:P0_META_WRITE_GOLDEN -eq "1"
$n = 0; $ok = 0; $wrote = 0
Get-ChildItem $here -Filter "*.lmx" | Sort-Object Name | ForEach-Object {
    $stem = $_.BaseName
    $src = $_.FullName
    $gold = Join-Path $here ($stem + ".meta.txt")
    $g620 = Join-Path $outDir ($stem + ".620.meta")
    $gCur = Join-Path $outDir ($stem + ".cur.meta")
    $e620 = Join-Path $outDir ($stem + ".620.err")
    $eCur = Join-Path $outDir ($stem + ".cur.err")
    $n++
    $p620 = Start-Process -FilePath (Join-Path (Get-Location) $exe620) -ArgumentList $src -NoNewWindow -Wait -PassThru -RedirectStandardOutput (Join-Path (Get-Location) $g620) -RedirectStandardError (Join-Path (Get-Location) $e620)
    $got620 = [IO.File]::ReadAllText((Join-Path (Get-Location) $g620)).Replace("`r`n","`n")
    $err620 = [IO.File]::ReadAllText((Join-Path (Get-Location) $e620)).Replace("`r`n","`n")
    $rej620 = $null
    if ($expect620Reject.ContainsKey($stem)) { $rej620 = $expect620Reject[$stem] }
    elseif ($writeGolden -and (Get-RejectLine $got620)) { $rej620 = (Get-RejectLine $got620).Trim() }
    if ($writeGolden) {
        if ($p620.ExitCode -ne 0 -and $p620.ExitCode -ne 1) { throw "$stem 620 crash exit $($p620.ExitCode); not writing golden" }
        if (-not $got620.StartsWith("# p0-meta v1")) { throw "$stem 620 dump missing banner; not writing golden" }
        Assert-DumpShape $stem "620" $p620.ExitCode $got620 $err620 $rej620
        [IO.File]::WriteAllText((Join-Path (Get-Location) $gold), $got620)
        $wrote++
    } else {
        if (-not (Test-Path $gold)) { throw "missing 620 golden $gold" }
        Assert-DumpShape $stem "620" $p620.ExitCode $got620 $err620 $rej620
        $want = [IO.File]::ReadAllText((Join-Path (Get-Location) $gold)).Replace("`r`n","`n")
        if (-not (Test-OrdinalEq $got620 $want)) { throw "$stem 620 dump != golden" }
    }

    $pCur = Start-Process -FilePath (Join-Path (Get-Location) $exeCur) -ArgumentList $src -NoNewWindow -Wait -PassThru -RedirectStandardOutput (Join-Path (Get-Location) $gCur) -RedirectStandardError (Join-Path (Get-Location) $eCur)
    $gotCur = [IO.File]::ReadAllText((Join-Path (Get-Location) $gCur)).Replace("`r`n","`n")
    $errCur = [IO.File]::ReadAllText((Join-Path (Get-Location) $eCur)).Replace("`r`n","`n")
    $rejCur = $null
    if ($expectCurReject.ContainsKey($stem)) { $rejCur = $expectCurReject[$stem] }
    elseif ($expect620Reject.ContainsKey($stem)) { $rejCur = $expect620Reject[$stem] }
    Assert-DumpShape $stem "cur" $pCur.ExitCode $gotCur $errCur $rejCur
    if (-not $writeGolden) {
        $want = [IO.File]::ReadAllText((Join-Path (Get-Location) $gold)).Replace("`r`n","`n")
        Assert-CurrentVs620 $stem $want $gotCur
    }
    $ok++
}

# Sensitivity through the same admission path, without editing committed goldens.
$probeStem = "A_inline_f_abc"
$probe = [IO.File]::ReadAllText((Join-Path (Get-Location) (Join-Path $here ($probeStem + ".meta.txt")))).Replace("`r`n","`n")
function Expect-Mismatch([string]$label, [string]$mut) {
    $threw = $false
    try { Assert-CurrentVs620 ($probeStem + "_mut") $probe $mut } catch { $threw = $true }
    if (-not $threw) { throw "sensitivity missed $label" }
}
Expect-Mismatch "append" ($probe + "MUTATION")
Expect-Mismatch "kind" ($probe.Replace("kind=frame", "kind=atom"))
Expect-Mismatch "flags" ($probe.Replace("flags=0x0", "flags=0x8"))
Expect-Mismatch "span" ($probe.Replace("span=", "span=9:9+"))
Expect-Mismatch "spelling-bytes" ($probe.Replace("bytes=", "bytes=FF"))
Expect-Mismatch "null-vs-empty" ($probe.Replace("trailer=null", "trailer=yes"))
Expect-Mismatch "child-order" ($probe.Replace("[0]", "[9]"))
$hdr = [IO.File]::ReadAllText((Join-Path (Get-Location) (Join-Path $here "A_header_then_body.meta.txt"))).Replace("`r`n","`n")
$threw = $false
try { Assert-CurrentVs620 "A_header_then_body" $hdr $hdr } catch { $threw = $true }
if (-not $threw) { throw "sensitivity missed missing colon_bit on A_header_then_body" }
$threw = $false
try { Assert-CurrentVs620 "A_header_then_body" $hdr ($hdr + "trailer flags=0x0 colon_bit=1 body=null`n") } catch { $threw = $true }
if (-not $threw) { throw "sensitivity missed extra colon_bit count on A_header_then_body" }
$mixedGold = [IO.File]::ReadAllText((Join-Path (Get-Location) (Join-Path $here "F_mixed_trailer.meta.txt"))).Replace("`r`n","`n")
# Current identity is colon_bit seq 0,1 (bare return, then colon end). Build that dump from the 620 golden.
$mixedCur = $mixedGold
$endAt = $mixedCur.LastIndexOf("colon_bit=0")
if ($endAt -lt 0) { throw "F_mixed_trailer golden missing colon_bit=0" }
$mixedCur = $mixedCur.Remove($endAt, "colon_bit=0".Length).Insert($endAt, "colon_bit=1")
if ((Get-ColonSeq $mixedCur) -ne "0,1") { throw "relocation setup expected seq 0,1 got $(Get-ColonSeq $mixedCur)" }
$threw = $false
try { Assert-CurrentVs620 "F_mixed_trailer" $mixedGold $mixedCur } catch { $threw = $true }
if ($threw) { throw "F_mixed_trailer seq 0,1 must be admitted" }
$reloc = $mixedCur.Replace("colon_bit=0", "colon_bit=X").Replace("colon_bit=1", "colon_bit=0").Replace("colon_bit=X", "colon_bit=1")
if ((Get-ColonSeq $reloc) -ne "1,0") { throw "relocation mutation expected seq 1,0 got $(Get-ColonSeq $reloc)" }
if (([regex]::Matches($reloc, "colon_bit=1")).Count -ne 1) { throw "relocation must keep colon_bit=1 count 1" }
$threw = $false
try { Assert-CurrentVs620 "F_mixed_trailer" $mixedGold $reloc } catch { $threw = $true }
if (-not $threw) { throw "sensitivity missed same-count colon_bit relocation on F_mixed_trailer" }

Write-Output "p0_meta ok n=$n compared=$ok goldens_written=$wrote"
