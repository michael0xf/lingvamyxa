# Nested call / index / field / binop / unary / parens / cast expressions.
# CWD = repo root. Isolated under build\l1trans\<gen> and build\obj\l1trans\<gen>.
# Does not edit tests/l1/run_smoke.ps1.
$ErrorActionPreference = "Stop"
Set-Location (Join-Path $PSScriptRoot "..\..")

$gen = "gen0"
if ($env:L1_GEN -and $env:L1_GEN.Trim().Length -gt 0) { $gen = $env:L1_GEN.Trim() }
$l1trans = "build\l1trans\$gen\l1trans.exe"
$obj = "build\obj\l1trans\$gen"
$bin = "build\l1trans\$gen"
$log = Join-Path "build\l1trans\logs" $gen
$script:exprLog = Join-Path $log "expr.log"
$cflagsStr = "-std=c99 -Wall -Wextra -Wpedantic -I . -Werror=incompatible-pointer-types -Werror=discarded-qualifiers -Werror=implicit-function-declaration -Werror=implicit-int"

New-Item -ItemType Directory -Force -Path $obj, $bin, $log | Out-Null
Set-Content -LiteralPath $script:exprLog -Value "$(Get-Date -Format o) expr start gen=$gen"

function Write-E([string]$m) { Add-Content -LiteralPath $script:exprLog -Value "$(Get-Date -Format o) $m" }

if (-not (Test-Path $l1trans)) { throw "missing $l1trans" }

$cases = @(
    @{ Name = "expr_nested_call"; Has = "add(add(" },
    @{ Name = "expr_index_arg"; Has = "add(xs[1], xs[0])" },
    @{ Name = "expr_field_arg"; Has = "p->length" },
    @{ Name = "expr_binop_unary_arg"; Has = "add(1 + 2 * 3, -4)" },
    @{ Name = "expr_parens"; Has = "add((1 + 2) * 3, 1 + 2 * 3)" },
    @{ Name = "expr_cast"; Has = "(uchar)259" },
    @{ Name = "expr_index_call"; Has = "xs[pick(1)]" },
    @{ Name = "expr_cast_bound"; Has = "2 * ((uchar)128 + 128)" },
    @{ Name = "expr_deref_assign"; Has = "*(slot) = 1" },
    @{ Name = "expr_deref_read"; Has = "* p + 1" },
    @{ Name = "expr_deref_arg"; Has = "add(*(p), 1)" },
    @{ Name = "expr_deref_call"; Has = "*(getp())" },
    @{ Name = "expr_deref_mix"; Has = @(
        "add(*(getp()), 1)",
        "! * p",
        "4 != * p",
        "add(*(&value), 1)",
        "add(*(&*(p)), 1)",
        "add(*(&*(p)) + 1, 1)"
    )},
    @{ Name = "expr_inc_arg"; Has = @(
        "take(i--)",
        "take(--i)",
        "take(++i)",
        "take(i++)",
        "take(1 + --i)",
        "a + ++b",
        "take(- --i)",
        "take(+ ++i)"
    )},
    @{ Name = "expr_arg_segments"; Has = @(
        "sum4(a + b, c--, nested(++i, x | y), arr[index + 1])",
        "nested(i--, x & y)"
    )},
    @{ Name = "expr_index_inc"; Has = @(
        "take(xs[i++])",
        "take(xs[i--])"
    )},
    @{ Name = "expr_index_deref"; Has = @(
        "xs[* p]",
        "take(xs[*(p)])"
    )},
    @{ Name = "expr_str_triple_double"; Has = @(
        'double # not a comment\ncolon: stays data',
        'backslash is raw: \\\" \\n',
        'const char * t = "\?\?="'
    )},
    @{ Name = "expr_str_triple_single"; Has = @(
        "single # not a comment",
        'four \"\"\"\" five'
    )},
    @{ Name = "expr_str_triple_args"; Has = @(
        ") # not syntax",
        '"c"'
    )},
    @{ Name = "expr_str_fence"; Has = @(
        'alpha\n    beta',
        "=== literal content",
        "from-import",
        '\"\"\"abc\"\"\"',
        'eq = ("=")',
        'sl = ("\\")',
        'br = ("[")',
        'inc = ("++")',
        'tb = ("\0110")'
    )},
    @{ Name = "expr_str_quoted"; Has = @(
        '"hello"',
        "'A'"
    )}
)

foreach ($c in $cases) {
    $src = "tests\l1\$($c.Name).lm2"
    $cpath = Join-Path $obj ($c.Name + ".c")
    $cpathB = Join-Path $obj ($c.Name + "_b.c")
    $exepath = Join-Path $bin ($c.Name + ".exe")
    $gccLog = Join-Path $log ("gcc_" + $c.Name + ".log")
    $runOut = Join-Path $log ("run_" + $c.Name + ".stdout")
    $runErr = Join-Path $log ("run_" + $c.Name + ".stderr")

    Write-E "BEGIN translate $src"
    & $l1trans $src $cpath
    if ($LASTEXITCODE -ne 0) { throw "translate failed $($c.Name) $LASTEXITCODE" }
    & $l1trans $src $cpathB
    if ($LASTEXITCODE -ne 0) { throw "translate b failed $($c.Name)" }
    $h1 = (Get-FileHash -LiteralPath $cpath).Hash
    $h2 = (Get-FileHash -LiteralPath $cpathB).Hash
    if ($h1 -ne $h2) { throw "C differ $($c.Name)" }
    $ctext = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $cpath))
    $need = @($c.Has)
    foreach ($h in $need) {
        if ($ctext.IndexOf($h) -lt 0) { throw "missing '$h' in $cpath" }
    }
    Write-E "C $($c.Name) sha256=$h1 has=$($need -join '; ')"

    Write-E "CMD gcc $cflagsStr -o $exepath $cpath"
    cmd /c "gcc $cflagsStr -o $exepath $cpath > $gccLog 2>&1"
    if ($LASTEXITCODE -ne 0) {
        Get-Content -LiteralPath $gccLog | Select-Object -Last 30
        throw "gcc failed $($c.Name)"
    }
    Write-E "EXIT gcc $($c.Name) 0"
    cmd /c "$exepath > $runOut 2> $runErr"
    if ($LASTEXITCODE -ne 0) { throw "run $($c.Name) exit $LASTEXITCODE" }
    Write-E "EXIT run $($c.Name) 0"
}

$badSrc = "tests\l1\invalid_deref_target.lm2"
$badC = Join-Path $obj "invalid_deref_target.c"
$badErr = Join-Path $log "invalid_deref_target.err"
Write-E "BEGIN negative $badSrc"
cmd /c "$l1trans $badSrc $badC > $log\invalid_deref_target.stdout 2> $badErr"
if ($LASTEXITCODE -eq 0) { throw "expected translate failure: $badSrc" }
$errText = [System.IO.File]::ReadAllText((Join-Path (Get-Location) $badErr))
if ($errText.IndexOf("dereferenced assignment target expects a name") -lt 0) {
    throw "missing deref-target diagnostic: $errText"
}
if (Test-Path -LiteralPath $badC) { throw "failed translate created $badC" }
Write-E "EXIT negative $badSrc $($LASTEXITCODE) diagnostic ok"

Write-E "expr ok"
Write-Output "l1trans $gen expr ok"
