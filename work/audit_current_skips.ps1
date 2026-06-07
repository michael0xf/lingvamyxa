$ErrorActionPreference = 'Continue'

$printTree = 'build/lm0/printTree.lm0.exe'
$trans = 'build/lm0/trans.lm0.exe'
$make = 'build/lm0/make.lm0.exe'
$parserLib = 'build/lm0/libparser.lm0.a'
$ownLib = 'build/lm0/libown.lm0.a'

New-Item -ItemType Directory -Force 'build/obj/skip-audit-current' | Out-Null

$parserTests = @(
    'if_else_dotted_empty_body_gap_probe.lmx'
)

$transTests = @(
    'trans_callable_force_argument.lm2',
    'trans_callable_lazy_bind.lm2',
    'trans_contextual_literal_none_ok.lmx',
    'trans_default_arguments.lm2',
    'trans_double_semicolon_params.lm2',
    'trans_fn_descriptor_only.lm2',
    'trans_l4_abi_receivers.lm2'
)

foreach ($name in $parserTests) {
    $src = Join-Path 'tests' $name
    $out = Join-Path 'build/obj/skip-audit-current' ($name + '.printTree.out.txt')
    & $printTree $src *> $out
    $code = $LASTEXITCODE
    Write-Output "PARSER`t$name`tcode=$code"
    if ($code -ne 0) {
        Get-Content $out | Select-Object -First 6 | ForEach-Object { Write-Output "  $_" }
    }
}

foreach ($name in $transTests) {
    $src = Join-Path 'tests' $name
    $base = [System.IO.Path]::GetFileNameWithoutExtension($name)
    $cPath = Join-Path 'build/obj/skip-audit-current' ($base + '.c')
    $exePath = Join-Path 'build/obj/skip-audit-current' ($base + '.exe')
    $transOut = Join-Path 'build/obj/skip-audit-current' ($base + '.trans.out.txt')
    $linkOut = Join-Path 'build/obj/skip-audit-current' ($base + '.link.out.txt')
    $runOut = Join-Path 'build/obj/skip-audit-current' ($base + '.run.out.txt')

    & $trans $src $cPath *> $transOut
    $tcode = $LASTEXITCODE
    Write-Output "TRANS`t$name`ttrans=$tcode"
    if ($tcode -ne 0) {
        Get-Content $transOut | Select-Object -First 8 | ForEach-Object { Write-Output "  $_" }
        continue
    }

    & $make 'link' '-std=c99' '-Wall' '-Wextra' '-Wpedantic' '-Ilm1' $cPath $parserLib $ownLib '-o' $exePath *> $linkOut
    $lcode = $LASTEXITCODE
    Write-Output "LINK`t$name`tlink=$lcode"
    if ($lcode -ne 0) {
        Get-Content $linkOut | Select-Object -First 14 | ForEach-Object { Write-Output "  $_" }
        continue
    }

    & (Resolve-Path -LiteralPath $exePath).Path *> $runOut
    $rcode = $LASTEXITCODE
    Write-Output "RUN`t$name`trun=$rcode"
    if ($rcode -ne 0) {
        Get-Content $runOut | Select-Object -First 8 | ForEach-Object { Write-Output "  $_" }
    }
}
