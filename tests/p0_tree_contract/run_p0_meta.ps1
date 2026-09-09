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
function Normalize-ColonBit([string]$t) { return $t.Replace("colon_bit=1", "colon_bit=0") }

$expectCurReject = @{
    "C_nested_short" = "REJECT code=13 line=2 col=5 msg=source level increase must be one step"
    "return_colon_empty_trailer" = "REJECT code=32 line=1 col=1 msg=empty colon Frame is not allowed"
    "return_colon_comment_trailer" = "REJECT code=32 line=1 col=1 msg=empty colon Frame is not allowed"
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
    if ($writeGolden) {
        [IO.File]::WriteAllText((Join-Path (Get-Location) $gold), $got620)
        $wrote++
    }
    if (-not (Test-Path $gold)) { throw "missing 620 golden $gold (set P0_META_WRITE_GOLDEN=1 once)" }
    $want = [IO.File]::ReadAllText((Join-Path (Get-Location) $gold)).Replace("`r`n","`n")
    if (-not (Test-OrdinalEq $got620 $want)) { throw "$stem 620 dump != golden" }

    $pCur = Start-Process -FilePath (Join-Path (Get-Location) $exeCur) -ArgumentList $src -NoNewWindow -Wait -PassThru -RedirectStandardOutput (Join-Path (Get-Location) $gCur) -RedirectStandardError (Join-Path (Get-Location) $eCur)
    $gotCur = [IO.File]::ReadAllText((Join-Path (Get-Location) $gCur)).Replace("`r`n","`n")
    if ($expectCurReject.ContainsKey($stem)) {
        $line = ($gotCur -split "`n" | Where-Object { $_ -like "REJECT *" } | Select-Object -First 1)
        if (-not $line) { throw "$stem current expected reject, got:`n$gotCur" }
        if (-not (Test-OrdinalEq $line.Trim() $expectCurReject[$stem])) {
            throw "$stem current reject mismatch`n got=$line`n want=$($expectCurReject[$stem])"
        }
        # 620 golden for these may be accept (empty-colon) or same reject (C_nested_short)
        $ok++; return
    }
    $normWant = Normalize-ColonBit $want
    $normCur = Normalize-ColonBit $gotCur
    if (-not (Test-OrdinalEq $normCur $normWant)) {
        throw "$stem current metadata != 620 golden (colon_bit normalized)`n--- 620 ---`n$want`n--- cur ---`n$gotCur"
    }
    $ok++
}

# Comparator sensitivity: ordinal, not case-fold; a mutated golden must fail.
$probe = [IO.File]::ReadAllText((Join-Path (Get-Location) (Join-Path $here "A_inline_f_abc.meta.txt"))).Replace("`r`n","`n")
$mut = $probe + "MUTATION"
if (Test-OrdinalEq $probe $mut) { throw "comparator failed to see appended mutation" }
$folded = $probe.ToUpperInvariant()
if ($folded -cne $probe) {
    if (Test-OrdinalEq $probe $folded) { throw "comparator is case-insensitive" }
}

Write-Output "p0_meta ok n=$n compared=$ok goldens_written=$wrote"
