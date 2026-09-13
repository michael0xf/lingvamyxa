param(
    [string]$Source = (Join-Path $PSScriptRoot 'l2trans.lm1'),
    [string]$Destination = (Join-Path $PSScriptRoot 'l2trans.lm2')
)

$ErrorActionPreference = 'Stop'

function Rename-NodeFormalToken([string]$Line) {
    $out = [Text.StringBuilder]::new()
    $quoted = $false
    $escaped = $false
    for ($i = 0; $i -lt $Line.Length; ++$i) {
        $ch = $Line[$i]
        if ($quoted) {
            [void]$out.Append($ch)
            if ($escaped) { $escaped = $false }
            elseif ($ch -eq '\') { $escaped = $true }
            elseif ($ch -eq '"') { $quoted = $false }
            continue
        }
        if ($ch -eq '#') { [void]$out.Append($Line.Substring($i)); break }
        if ($ch -eq '"') { $quoted = $true; [void]$out.Append($ch); continue }
        if ($i + 4 -le $Line.Length -and $Line.Substring($i, 4) -ceq 'node') {
            $before = if ($i -eq 0) { '' } else { [string]$Line[$i - 1] }
            $after = if ($i + 4 -ge $Line.Length) { '' } else { [string]$Line[$i + 4] }
            $beforeIdent = $before -cmatch '[A-Za-z0-9_]' -or $before -ceq '\'
            $afterIdent = $after -cmatch '[A-Za-z0-9_]'
            if (-not $beforeIdent -and -not $afterIdent) {
                [void]$out.Append('p0_node')
                $i += 3
                continue
            }
        }
        [void]$out.Append($ch)
    }
    return $out.ToString()
}

function Rename-FormalToken([string]$Line, [string]$From, [string]$To) {
    $out = [Text.StringBuilder]::new()
    $quoted = $false
    $escaped = $false
    for ($i = 0; $i -lt $Line.Length; ++$i) {
        $ch = $Line[$i]
        if ($quoted) {
            [void]$out.Append($ch)
            if ($escaped) { $escaped = $false }
            elseif ($ch -eq '\') { $escaped = $true }
            elseif ($ch -eq '"') { $quoted = $false }
            continue
        }
        if ($ch -eq '#') { [void]$out.Append($Line.Substring($i)); break }
        if ($ch -eq '"') { $quoted = $true; [void]$out.Append($ch); continue }
        if ($i + $From.Length -le $Line.Length -and $Line.Substring($i, $From.Length) -ceq $From) {
            $before = if ($i -eq 0) { '' } else { [string]$Line[$i - 1] }
            $after = if ($i + $From.Length -ge $Line.Length) { '' } else { [string]$Line[$i + $From.Length] }
            $beforeIdent = $before -cmatch '[A-Za-z0-9_]' -or $before -ceq '\'
            $afterIdent = $after -cmatch '[A-Za-z0-9_]'
            $isHeadKeyword = $i -eq 0 -and $after -ceq ':'
            if (-not $beforeIdent -and -not $afterIdent -and -not $isHeadKeyword) {
                [void]$out.Append($To)
                $i += $From.Length - 1
                continue
            }
        }
        [void]$out.Append($ch)
    }
    return $out.ToString()
}

$sourcePath = (Resolve-Path -LiteralPath $Source).ProviderPath
$lines = [IO.File]::ReadAllLines($sourcePath)
$body = [Collections.Generic.List[string]]::new()
$inPrototype = $false
$prototypeBlocks = 0
$inMain = $false
$mainWrappers = 0
$arrayDeclarations = 0
$constDeclarations = 0
$constReturns = 0
$constPointerInitializers = 0
$curMiInitializers = 0
$localMinusOneInitializers = 0
$renameNodeFormal = $false
$renamedNodeFormals = 0
$renameFnFormal = $false
$renamedFnFormals = 0

for ($i = 0; $i -lt $lines.Count; ++$i) {
    $line = $lines[$i]

    if (-not $inPrototype -and $line -ceq 'prototype:') {
        $inPrototype = $true
        ++$prototypeBlocks
        continue
    }
    if ($inPrototype) {
        if ($line -ceq 'end: prototype') { $inPrototype = $false }
        continue
    }

    if (-not $inMain -and $line -ceq 'external:' -and
        $i + 1 -lt $lines.Count -and $lines[$i + 1] -cmatch '^    fn: main \(') {
        $inMain = $true
        ++$mainWrappers
        continue
    }
    if ($inMain -and $line -ceq 'end: external') {
        $inMain = $false
        continue
    }
    if ($inMain) {
        if (-not $line.StartsWith('    ', [StringComparison]::Ordinal)) {
            throw "main wrapper contains an unindented line: $($i + 1)"
        }
        $line = $line.Substring(4)
    }

    if ($line -cmatch '^fn: [A-Za-z_][A-Za-z0-9_]* .*\bnode\b') {
        $renameNodeFormal = $true
        ++$renamedNodeFormals
    }
    elseif ($line -cmatch '^(fn|sub): ') {
        $renameNodeFormal = $false
    }
    if ($renameNodeFormal) { $line = Rename-NodeFormalToken $line }

    if ($line -cmatch '^fn: [A-Za-z_][A-Za-z0-9_]* .*\bfn\b') {
        $renameFnFormal = $true
        ++$renamedFnFormals
    }
    elseif ($line -cmatch '^(fn|sub): ') {
        $renameFnFormal = $false
    }
    if ($renameFnFormal) { $line = Rename-FormalToken $line 'fn' 'p0_fn' }

    if ($line -ceq 'int: l2_cur_mi 0 - 1') {
        $line = 'int: l2_cur_mi 0'
        ++$curMiInitializers
    }

    if ($line -cmatch '^(\s+)int: ([A-Za-z_][A-Za-z0-9_]*) 0 - 1$') {
        [void]$body.Add('    ' + $Matches[1] + 'int: ' + $Matches[2] + ' 0')
        [void]$body.Add('    ' + $Matches[1] + $Matches[2] + ': 0 - 1')
        ++$localMinusOneInitializers
        continue
    }

    if ($line -cmatch '^(\s*)c\.array: \[(\d+)\]: char ([A-Za-z_][A-Za-z0-9_]*)$') {
        $line = $Matches[1] + '[]: char ' + $Matches[3] + ' ' + $Matches[2]
        ++$arrayDeclarations
    }
    elseif ($line -cmatch '^(\s*)c\.array: \[\]: char ([A-Za-z_][A-Za-z0-9_]*) (\d+)$') {
        $line = $Matches[1] + '[]: char ' + $Matches[2] + ' ' + $Matches[3]
        ++$arrayDeclarations
    }

    if ($line -cmatch '^(\s*)const: (@+): ([A-Za-z_][A-Za-z0-9_]*) ([A-Za-z_][A-Za-z0-9_]*)(.*)$') {
        $line = $Matches[1] + 'const: ' + $Matches[2] + '(' + $Matches[3] + ' ' + $Matches[4] + $Matches[5] + ')'
        ++$constDeclarations
    }
    elseif ($line -cmatch '^(.*\)) const: (@+): ([A-Za-z_][A-Za-z0-9_]*)$') {
        $line = $Matches[1] + ' const: ' + $Matches[2] + '(' + $Matches[3] + ')'
        ++$constReturns
    }
    elseif ($line -cmatch '^(\s*)const: (@+)\(([^()]*)\) (.+)$') {
        $line = $Matches[1] + 'const: ' + $Matches[2] + '(' + $Matches[3] + ' ' + $Matches[4] + ')'
        ++$constPointerInitializers
    }

    [void]$body.Add('    ' + $line)
}

if ($inPrototype) { throw 'unterminated top-level prototype block' }
if ($inMain) { throw 'unterminated final external main wrapper' }
if ($prototypeBlocks -ne 1) { throw "expected one top-level prototype block, found $prototypeBlocks" }
if ($mainWrappers -ne 1) { throw "expected one final external main wrapper, found $mainWrappers" }
if ($curMiInitializers -ne 1) { throw "expected one l2_cur_mi initializer, found $curMiInitializers" }
if ($localMinusOneInitializers -ne 15) { throw "expected 15 local minus-one initializers, found $localMinusOneInitializers" }
if ($renamedNodeFormals -ne 24) { throw "expected 24 node-formal functions, found $renamedNodeFormals" }
if ($renamedFnFormals -ne 3) { throw "expected three fn-formal functions, found $renamedFnFormals" }
if ($constPointerInitializers -ne 5) { throw "expected five const pointer initializers, found $constPointerInitializers" }
if ($arrayDeclarations -eq 0 -or $constDeclarations -eq 0 -or $constReturns -eq 0) {
    throw 'expected L1-only array, const declaration and const return spellings'
}

$output = [Collections.Generic.List[string]]::new()
[void]$output.Add('L2:')
foreach ($line in $body) { [void]$output.Add($line) }
[void]$output.Add('end: L2')

$destinationPath = [IO.Path]::GetFullPath($Destination)
[IO.Directory]::CreateDirectory([IO.Path]::GetDirectoryName($destinationPath)) | Out-Null
[IO.File]::WriteAllText($destinationPath, (($output -join "`n") + "`n"), [Text.UTF8Encoding]::new($false))
Write-Output "l2trans L2 source written: $destinationPath arrays=$arrayDeclarations constDecls=$constDeclarations constReturns=$constReturns constPtrInits=$constPointerInitializers localMinusOne=$localMinusOneInitializers nodeFormals=$renamedNodeFormals fnFormals=$renamedFnFormals"
