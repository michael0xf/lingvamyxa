param(
    [Parameter(Mandatory = $true)]
    [string]$TableTool
)

Set-StrictMode -Version 2.0
$ErrorActionPreference = 'Stop'

$tableToolPath = (Resolve-Path -LiteralPath $TableTool).Path
$projectRoot = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..')).Path
$fixture = Join-Path $projectRoot 'tests/fixtures/serializer_source_marker_roundtrip.lmx'
$testDir = Join-Path ([System.IO.Path]::GetTempPath()) ('lm-table-source-marker-roundtrip-' + [System.Guid]::NewGuid().ToString('N'))
$utf8NoBom = New-Object System.Text.UTF8Encoding($false)

function Invoke-TableCapture {
    param(
        [Parameter(Mandatory = $true)]
        [string]$InputPath,
        [Parameter(Mandatory = $true)]
        [string]$OutputPath
    )

    $startInfo = New-Object System.Diagnostics.ProcessStartInfo
    $startInfo.FileName = $tableToolPath
    $startInfo.Arguments = '"' + $InputPath.Replace('"', '\"') + '"'
    $startInfo.WorkingDirectory = $projectRoot
    $startInfo.UseShellExecute = $false
    $startInfo.CreateNoWindow = $true
    $startInfo.RedirectStandardOutput = $true
    $startInfo.RedirectStandardError = $true

    $process = New-Object System.Diagnostics.Process
    $process.StartInfo = $startInfo
    try {
        if (-not $process.Start()) {
            throw "could not start table formatter: $tableToolPath"
        }
        $standardOutput = $process.StandardOutput.ReadToEnd()
        $standardError = $process.StandardError.ReadToEnd()
        $process.WaitForExit()
        if ($process.ExitCode -ne 0) {
            throw "table formatter failed with exit $($process.ExitCode): $standardError"
        }
    }
    finally {
        $process.Dispose()
    }

    [System.IO.File]::WriteAllText($OutputPath, $standardOutput, $utf8NoBom)
}

function Assert-TableRejects {
    param(
        [Parameter(Mandatory = $true)]
        [string]$InputPath
    )

    $startInfo = New-Object System.Diagnostics.ProcessStartInfo
    $startInfo.FileName = $tableToolPath
    $startInfo.Arguments = '"' + $InputPath.Replace('"', '\"') + '"'
    $startInfo.WorkingDirectory = $projectRoot
    $startInfo.UseShellExecute = $false
    $startInfo.CreateNoWindow = $true
    $startInfo.RedirectStandardOutput = $true
    $startInfo.RedirectStandardError = $true

    $process = New-Object System.Diagnostics.Process
    $process.StartInfo = $startInfo
    try {
        if (-not $process.Start()) {
            throw "could not start table formatter: $tableToolPath"
        }
        $standardOutput = $process.StandardOutput.ReadToEnd()
        $standardError = $process.StandardError.ReadToEnd()
        $process.WaitForExit()
        if ($process.ExitCode -ne 1) {
            throw "invalid table source-marker fixture expected exit 1, got $($process.ExitCode): $InputPath`n$standardOutput$standardError"
        }
    }
    finally {
        $process.Dispose()
    }
}

New-Item -ItemType Directory -Path $testDir | Out-Null
try {
    $first = Join-Path $testDir 'first.lmx'
    $second = Join-Path $testDir 'second.lmx'

    Invoke-TableCapture -InputPath $fixture -OutputPath $first
    Invoke-TableCapture -InputPath $first -OutputPath $second

    $firstText = [System.IO.File]::ReadAllText($first)
    $secondText = [System.IO.File]::ReadAllText($second)
    if ($firstText -cne $secondText) {
        throw 'table source-marker output changed on the second serialization pass'
    }
    if ([regex]::Matches($firstText, '(?m)^    source\r?$').Count -ne 1) {
        throw 'table source-marker output must contain exactly one source marker'
    }
    if ($firstText -notmatch '(?m)^table:\r?\n    source\r?\n    name: marked\r?$') {
        throw 'marked table did not preserve its leading source marker'
    }
    if ($firstText -notmatch '(?m)^table:\r?\n    name: unmarked\r?$') {
        throw 'unmarked table was not preserved'
    }
    if ($firstText -match '(?m)^table:\r?\n    source\r?\n    name: unmarked\r?$') {
        throw 'unmarked table acquired a source marker'
    }

    foreach ($invalidFixture in @(
        (Join-Path $projectRoot 'tests/fixtures/parser_registry_source_table_marker_order_invalid.lm2'),
        (Join-Path $projectRoot 'tests/fixtures/parser_registry_source_table_marker_duplicate_invalid.lm2')
    )) {
        Assert-TableRejects -InputPath $invalidFixture
    }
}
finally {
    Remove-Item -LiteralPath $testDir -Recurse -Force -ErrorAction SilentlyContinue
}
