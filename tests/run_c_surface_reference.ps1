param(
    [string]$TranslatorPath = "build/lm0/trans.lm0.exe",
    [string]$OutputDirectory = "build/codex/c_surface_reference/gate"
)

$ErrorActionPreference = "Stop"
Set-Location (Join-Path $PSScriptRoot "..")

$cases = @(
    @{ Name="trans_c_surface_call_parens"; Sha="61D2F22808E23760D9E096EA2C4035B8DB1F3A70DD015E49B911AF3D37A34841"; Has=@("add(variable, wrap(add(2, node->value)))") },
    @{ Name="trans_c_surface_compact"; Sha="14DE390A6B8D22758F9C328ECBF794F96978A25FE11B7E3DE55D85F472A7CE4F"; Has=@("boxes[1].value = 7", "boxes[1].value != 7") },
    @{ Name="trans_c_surface_variable_expr"; Sha="EBCAAAA78DE2D3860E16CD59F2A9DD8EAD29E5D102DA976F4C4032A51F3D51E8"; Has=@("variable != 17") },
    @{ Name="trans_value_field_dot"; Sha="4A83A15BAEBE3DE38B846075F7DFED5C31CD9482A89FAF95BCCBC5DF60D1D3DF"; Has=@("c_box.value = 9", "c_box.value != 9") },
    @{ Name="trans_getenv_index_probe"; Sha="3C18E61F8FFA20BF0BD6EAC6D181D8D0090FD49E1630411F67C7635582C261A8"; Has=@("getenv(name)[0] == '0'") },
    @{ Name="trans_index_bracket_spaces"; Sha="4D53676A79E573AFCC8548D078D0697E8DD62E1FA6AE1F36B614B69F35BFC896"; Has=@("[index - 1] = 99", "[index - 1] != 99") },
    @{ Name="trans_index_semantic_nested"; Sha="7C7066EFC9D542401D74EB99E869EA39988E0E5D6056DA57092B029268968210"; Has=@("[offset(", ")[pick(", "] + pick(") },
    @{ Name="trans_nested_call_namespace"; Sha="E38BDB666A5DEAD134A00C1FF495CDAB156166B85C55DA4EAC094A143913B934"; Has=@("print(", ", 1, 2, get(") },
    @{ Name="trans_multiline_paren_expr"; Sha="54050D2D5F01DD0492B0F5F19B691D9B341585B47B5CC248DD0F62FAB146B82B"; Has=@("v = (1 + 1) * 3") }
)

if (-not (Test-Path -LiteralPath $TranslatorPath -PathType Leaf)) {
    throw "missing translator: $TranslatorPath"
}
New-Item -ItemType Directory -Force -Path $OutputDirectory | Out-Null

$cflags = @("-std=c99", "-Wall", "-Wextra", "-Wpedantic", "-Ilm1", "-Itests")
$libraries = @("build/lm0/libparser.lm0.a", "build/lm0/libown.lm0.a")
foreach ($library in $libraries) {
    if (-not (Test-Path -LiteralPath $library -PathType Leaf)) { throw "missing library: $library" }
}

foreach ($case in $cases) {
    $source = "tests/$($case.Name).lm2"
    $actualSha = (Get-FileHash -LiteralPath $source -Algorithm SHA256).Hash
    if ($actualSha -ne $case.Sha) {
        throw "$source changed from the old-worked-version reference: $actualSha"
    }

    $cPath = Join-Path $OutputDirectory "$($case.Name).c"
    $cPath2 = Join-Path $OutputDirectory "$($case.Name).second.c"
    $exePath = Join-Path $OutputDirectory "$($case.Name).exe"
    $transErr = Join-Path $OutputDirectory "$($case.Name).trans.err"
    $gccLog = Join-Path $OutputDirectory "$($case.Name).gcc.log"

    & $TranslatorPath $source $cPath 2> $transErr
    if ($LASTEXITCODE -ne 0) { throw "translation failed: $source" }
    & $TranslatorPath $source $cPath2 2>> $transErr
    if ($LASTEXITCODE -ne 0) { throw "second translation failed: $source" }
    if ((Get-FileHash -LiteralPath $cPath).Hash -ne (Get-FileHash -LiteralPath $cPath2).Hash) {
        throw "non-deterministic generated C: $source"
    }

    $generated = [IO.File]::ReadAllText((Resolve-Path -LiteralPath $cPath).Path)
    foreach ($needle in $case.Has) {
        if ($generated.IndexOf($needle, [StringComparison]::Ordinal) -lt 0) {
            throw "$source generated C missing: $needle"
        }
    }

    $previous = $ErrorActionPreference
    $ErrorActionPreference = "Continue"
    & gcc @cflags $cPath @libraries -o $exePath *> $gccLog
    $gccExit = $LASTEXITCODE
    $ErrorActionPreference = $previous
    if ($gccExit -ne 0) { Get-Content -LiteralPath $gccLog; throw "gcc failed: $source" }
    & $exePath
    if ($LASTEXITCODE -ne 0) { throw "$source executable failed: $LASTEXITCODE" }
}

Write-Output "C-surface reference gate passed: $($cases.Count) old-worked-version fixtures"
