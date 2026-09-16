# Stage AD acceptance probe (design d6/lock-removal, AD section, 9232d6c4; ticket from the coordinator,
# 2026-09-16). AD deletes every resolution of an id by scanning -- lmx_msg_find, find_tree, dest_from_src and
# self_or_find's find fallback -- deletes the id-taking lmx_msg_exec_adopt_mark/dispose_mark (no caller), and
# retires LmxMsg.parent as a duplicate of parent_msg.
# This runner prints counts over tracked l2src sources and builds nothing. It never joins a gate; it stays on
# its own branch (d6/lock-ad-red) and is merged onto an AD branch only to measure. Exit 0 when every count is
# 0. Every pattern is a literal substring except the parent field read, which is a regex so that parent_msg
# is not counted; comment text counts (the S6 probe's rule: a comment that spells a deleted name is reworded).
#
#   find            lines naming "msg_find(" or "msg_find (" (lm1 lmx_msg_find, lm2 msg_find, the C
#                   declaration and every call; msg_live_find and msg_find_tree do not contain either needle).
#   find_tree       lines naming "msg_find_tree(" or "msg_find_tree (".
#   dest_from_src   lines naming "msg_dest_from_src(" or "msg_dest_from_src (".
#   self_or_find_fallback  lines inside the body of self_or_find (lm1 lmx_msg_self_or_find, lm2
#                   msg_self_or_find) that name "msg_find(" -- informational; already inside find's count.
#   id_marks        lines naming "lmx_msg_exec_adopt_mark(" or "lmx_msg_exec_dispose_mark(" (the _msg forms
#                   are not matched: "_mark_msg(" does not contain "_mark(").
#   parent_field    a " parent;" field in the body of struct LmxMsg (lmx_message.h), plus lines matching
#                   (->|\\)parent\b -- a receiver-qualified read or write of the field on any receiver.
$ErrorActionPreference = 'Stop'
Set-Location (Join-Path $PSScriptRoot '..')

function Get-ProbeText([string]$rel) {
    return [IO.File]::ReadAllText((Resolve-Path -LiteralPath $rel).ProviderPath).Replace("`r`n", "`n")
}

function Get-ProbeLines([string[]]$files, [string[]]$needles) {
    $hits = @()
    foreach ($f in $files) {
        $n = 0
        foreach ($line in ((Get-ProbeText $f) -split "`n")) {
            foreach ($needle in $needles) {
                if ($line.Contains($needle)) { $n += 1; break }
            }
        }
        if ($n -gt 0) { $hits += [pscustomobject]@{ File = $f; Count = $n } }
    }
    return $hits
}

function Get-RegexLines([string[]]$files, [string]$pattern) {
    $hits = @()
    $re = [regex]$pattern
    foreach ($f in $files) {
        $n = 0
        foreach ($line in ((Get-ProbeText $f) -split "`n")) {
            if ($re.IsMatch($line)) { $n += 1 }
        }
        if ($n -gt 0) { $hits += [pscustomobject]@{ File = $f; Count = $n } }
    }
    return $hits
}

function Get-StructFieldCount([string]$rel, [string]$open, [string]$close, [string[]]$fieldTails) {
    $n = 0
    $inside = $false
    foreach ($line in ((Get-ProbeText $rel) -split "`n")) {
        if (-not $inside) { if ($line.Contains($open)) { $inside = $true }; continue }
        if ($line.Contains($close)) { $inside = $false; continue }
        $trim = $line.Trim()
        foreach ($tail in $fieldTails) {
            if ($trim.EndsWith($tail) -and -not $trim.StartsWith('/*') -and -not $trim.StartsWith('*')) { $n += 1; break }
        }
    }
    return $n
}

# Lines naming msg_find( between a self_or_find definition head and the next top-level definition.
function Get-FallbackCount([string[]]$files) {
    $n = 0
    foreach ($f in $files) {
        $inside = $false
        foreach ($line in ((Get-ProbeText $f) -split "`n")) {
            if ($line -match '^(fn|sub): (lmx_)?msg_self_or_find ') { $inside = $true; continue }
            if ($inside -and $line -match '^(fn|sub|end): ') { if ($line -notmatch '^end: ') { $inside = $false } else { $inside = $false; continue } }
            if ($inside -and $line.Contains('msg_find(')) { $n += 1 }
        }
    }
    return $n
}

function Get-Sum($hits) { $s = 0; foreach ($h in $hits) { $s += $h.Count }; return $s }

$files = @(& git ls-files -- 'l2src/*.c' 'l2src/*.h' 'l2src/*.lm1' 'l2src/*.lm2')
if ($LASTEXITCODE -ne 0 -or $files.Count -lt 10) { throw "git ls-files listed only $($files.Count) l2src sources" }

$hFind = Get-ProbeLines $files @('msg_find(', 'msg_find (')
$hTree = Get-ProbeLines $files @('msg_find_tree(', 'msg_find_tree (')
$hDest = Get-ProbeLines $files @('msg_dest_from_src(', 'msg_dest_from_src (')
$fallback = Get-FallbackCount @('l2src/lmx_message.lm1', 'l2src/lmx_message.lm2')
$hMarks = Get-ProbeLines $files @('lmx_msg_exec_adopt_mark(', 'lmx_msg_exec_dispose_mark(')
$parentDecl = Get-StructFieldCount 'l2src/lmx_message.h' 'typedef struct LmxMsg {' '} LmxMsg;' @(' parent;')
$hParent = Get-RegexLines $files '(->|\\)parent\b'

$find = Get-Sum $hFind
$tree = Get-Sum $hTree
$dest = Get-Sum $hDest
$marks = Get-Sum $hMarks
$parent = $parentDecl + (Get-Sum $hParent)

"AD probe at $((& git rev-parse --short HEAD) -join '')"
function Show-Hits([string]$label, $hits, [string]$extra) {
    "${label}:$extra"
    foreach ($h in $hits) { "  $($h.File) : $($h.Count)" }
}
Show-Hits 'find by file' $hFind " (of which inside self_or_find's body: $fallback)"
Show-Hits 'find_tree by file' $hTree ''
Show-Hits 'dest_from_src by file' $hDest ''
Show-Hits 'id_marks by file' $hMarks ''
Show-Hits 'parent_field by file' $hParent " (LmxMsg parent field: $parentDecl)"
"AD probe: find=$find find_tree=$tree dest_from_src=$dest self_or_find_fallback=$fallback id_marks=$marks parent_field=$parent"
if ($find -eq 0 -and $tree -eq 0 -and $dest -eq 0 -and $fallback -eq 0 -and $marks -eq 0 -and $parent -eq 0) {
    'AD GREEN'
    exit 0
}
'AD RED'
exit 1
