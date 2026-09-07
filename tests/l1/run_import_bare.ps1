# L1 source-relative bare import:/predef: filenames.
# CWD = repo root unless a case explicitly uses another directory.
$ErrorActionPreference = "Stop"
Set-Location (Join-Path $PSScriptRoot "..\..")

$gen = "gen0"
if ($env:L1_GEN -and $env:L1_GEN.Trim().Length -gt 0) { $gen = $env:L1_GEN.Trim() }
$root = (Get-Location).Path
$l1trans = Join-Path $root "build\l1trans\$gen\l1trans.exe"
$obj = Join-Path $root "build\obj\l1trans\$gen"
$bin = Join-Path $root "build\l1trans\$gen"
$log = Join-Path $root "build\l1trans\logs\$gen"
$script:impLog = Join-Path $log "import_bare.log"
$cflagsStr = "-std=c99 -Wall -Wextra -Wpedantic -I . -Werror=incompatible-pointer-types -Werror=discarded-qualifiers -Werror=implicit-function-declaration -Werror=implicit-int"

New-Item -ItemType Directory -Force -Path $obj, $bin, $log | Out-Null
Set-Content -LiteralPath $script:impLog -Value "$(Get-Date -Format o) import_bare start gen=$gen"

function Write-I([string]$m) { Add-Content -LiteralPath $script:impLog -Value "$(Get-Date -Format o) $m" }

if (-not (Test-Path $l1trans)) { throw "missing $l1trans" }

$drvC = Join-Path $root "tests\l1\bare_drive_resolve.c"
$drvExe = Join-Path $bin "bare_drive_resolve.exe"
$drvGcc = Join-Path $log "gcc_bare_drive_resolve.log"
Write-I "CMD gcc $cflagsStr -o $drvExe $drvC"
cmd /c "gcc $cflagsStr -o `"$drvExe`" `"$drvC`" > `"$drvGcc`" 2>&1"
if ($LASTEXITCODE -ne 0) {
    Get-Content -LiteralPath $drvGcc | Select-Object -Last 30
    throw "gcc failed bare_drive_resolve"
}
cmd /c "`"$drvExe`""
if ($LASTEXITCODE -ne 0) { throw "bare_drive_resolve exit $LASTEXITCODE" }
Write-I "EXIT run bare_drive_resolve 0"

$genC = Join-Path $obj "l1trans.c"
$genText = [System.IO.File]::ReadAllText($genC)
if ($genText.IndexOf("source_path[1] == 58") -lt 0) { throw "generated l1trans.c missing drive-prefix test" }
if ($genText.IndexOf("memcpy(buf, source_path, 2U)") -lt 0) { throw "generated l1trans.c missing drive-prefix memcpy 2U" }

function Translate-Src([string]$src, [string]$name) {
    $cpath = Join-Path $obj ($name + ".c")
    $cpathB = Join-Path $obj ($name + "_b.c")
    Write-I "BEGIN translate $src"
    & $l1trans $src $cpath
    if ($LASTEXITCODE -ne 0) { throw "translate failed $src $LASTEXITCODE" }
    & $l1trans $src $cpathB
    if ($LASTEXITCODE -ne 0) { throw "translate b failed $src" }
    $h1 = (Get-FileHash -LiteralPath $cpath).Hash
    $h2 = (Get-FileHash -LiteralPath $cpathB).Hash
    if ($h1 -ne $h2) { throw "C differ $name" }
    Write-I "C $name sha256=$h1"
    return $cpath
}

function Build-Run([string]$name, [string]$cpath, [int]$expect) {
    $exepath = Join-Path $bin ($name + ".exe")
    $gccLog = Join-Path $log ("gcc_" + $name + ".log")
    $runOut = Join-Path $log ("run_" + $name + ".stdout")
    $runErr = Join-Path $log ("run_" + $name + ".stderr")
    Write-I "CMD gcc $cflagsStr -o $exepath $cpath"
    cmd /c "gcc $cflagsStr -o $exepath $cpath > $gccLog 2>&1"
    if ($LASTEXITCODE -ne 0) {
        Get-Content -LiteralPath $gccLog | Select-Object -Last 30
        throw "gcc failed $name"
    }
    cmd /c "$exepath > $runOut 2> $runErr"
    if ($LASTEXITCODE -ne $expect) { throw "run $name exit $LASTEXITCODE want $expect" }
    Write-I "EXIT run $name $LASTEXITCODE"
}

$spaceSrc = "tests\l1\bare space\import_bare.lm2"
$sc = Translate-Src $spaceSrc "import_bare"
$st = [System.IO.File]::ReadAllText($sc)
if ($st.IndexOf("int add_one(void)") -lt 0) { throw "missing add_one" }
Build-Run "import_bare" $sc 0

Push-Location (Join-Path $root "tests\l1")
try {
    $cwdC = Join-Path $obj "import_bare_cwd.c"
    $cwdB = Join-Path $obj "import_bare_cwd_b.c"
    $trans = $l1trans
    Write-I "BEGIN translate cwd=tests\l1 bare space\import_bare.lm2"
    & $trans "bare space\import_bare.lm2" $cwdC
    if ($LASTEXITCODE -ne 0) { throw "cwd translate failed $LASTEXITCODE" }
    & $trans "bare space\import_bare.lm2" $cwdB
    if ($LASTEXITCODE -ne 0) { throw "cwd translate b failed" }
    $h1 = (Get-FileHash -LiteralPath $cwdC).Hash
    $h2 = (Get-FileHash -LiteralPath $cwdB).Hash
    if ($h1 -ne $h2) { throw "C differ import_bare_cwd" }
    Write-I "C import_bare_cwd sha256=$h1"
} finally {
    Pop-Location
}
$cwdT = [System.IO.File]::ReadAllText((Join-Path $obj "import_bare_cwd.c"))
if ($cwdT.IndexOf("int add_one(void)") -lt 0) { throw "cwd missing add_one" }
Build-Run "import_bare_cwd" (Join-Path $obj "import_bare_cwd.c") 0

$pc = Translate-Src "tests\l1\bare space\import_bare_predef.lm2" "import_bare_predef"
Build-Run "import_bare_predef" $pc 0

$rc = Translate-Src "tests\l1\bare space\import_bare_repeat.lm2" "import_bare_repeat"
$rt = [System.IO.File]::ReadAllText($rc)
$addCount = ([regex]::Matches($rt, "int add_one\(void\)")).Count
if ($addCount -ne 1) { throw "repeat import emitted add_one $addCount times" }
Build-Run "import_bare_repeat" $rc 0

$dc = Translate-Src "tests\l1\import_bare_dirs.lm2" "import_bare_dirs"
$dt = [System.IO.File]::ReadAllText($dc)
if ($dt.IndexOf("int ha(void)") -lt 0) { throw "missing ha from bare_a/helper" }
if ($dt.IndexOf("int hb(void)") -lt 0) { throw "missing hb from bare_b/helper" }
if ($dt.IndexOf("return 11") -lt 0) { throw "missing ha value 11" }
if ($dt.IndexOf("return 22") -lt 0) { throw "missing hb value 22" }
Build-Run "import_bare_dirs" $dc 0

function Negative-Preserve([string]$src, [string]$name, [string]$diag) {
    $cpath = Join-Path $obj ($name + ".c")
    $err = Join-Path $log ($name + ".err")
    Write-I "BEGIN negative $src"
    if (Test-Path -LiteralPath $cpath) { Remove-Item -LiteralPath $cpath }
    $marker = "PRESERVE-DESTINATION`n"
    [System.IO.File]::WriteAllText($cpath, $marker)
    $fullSrc = Join-Path $root $src
    $p1 = Start-Process -FilePath $l1trans -ArgumentList @("`"$fullSrc`"", "`"$cpath`"") -NoNewWindow -Wait -PassThru -RedirectStandardOutput (Join-Path $log ($name + ".stdout")) -RedirectStandardError $err
    if ($p1.ExitCode -eq 0) { throw "expected translate failure: $src" }
    $got = [System.IO.File]::ReadAllText($cpath)
    if ($got -ne $marker) { throw "destination mutated on failed translate: $cpath" }
    Remove-Item -LiteralPath $cpath -Force
    $p2 = Start-Process -FilePath $l1trans -ArgumentList @("`"$fullSrc`"", "`"$cpath`"") -NoNewWindow -Wait -PassThru -RedirectStandardOutput (Join-Path $log ($name + ".stdout")) -RedirectStandardError $err
    if ($p2.ExitCode -eq 0) { throw "expected translate failure: $src" }
    $errText = [System.IO.File]::ReadAllText($err)
    if ($errText.IndexOf($diag) -lt 0) { throw "missing diagnostic '$diag': $errText" }
    if (Test-Path -LiteralPath $cpath) { throw "failed translate created $cpath" }
    Write-I "EXIT negative $src $($p2.ExitCode) diagnostic ok"
}

Negative-Preserve "tests\l1\bare space\invalid_import_bare_missing.lm2" "invalid_import_bare_missing" "cannot read import"
Negative-Preserve "tests\l1\invalid_import_bare_cycle.lm2" "invalid_import_bare_cycle" "import cycle"

Write-I "import_bare ok"
Write-Output "l1trans $gen import_bare ok"
