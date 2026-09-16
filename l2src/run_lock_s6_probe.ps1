# Lock removal S6 acceptance probe (design d6/lock-removal, S6 section; the coordinator's ruling, 2026-09-15).
# S6-1 deletes the executor lock and its holds, LmxMsgExec's non-atomic shared flags and the host lock.
# S6-2 deletes refs (retain/release/Interlocked) and the runtime's slot list, on 5.6's agreed wording.
# This runner prints six counts over tracked l2src sources and builds nothing. It never joins a gate; it
# stays on its own branch (d6/lock-s6-red) and is merged onto an S6 branch only to measure.
# -Part 1 exits 0 when the four S6-1 counts are 0; -Part 2 when the two S6-2 counts are 0; -Part all (the
# default) when all six are 0. Every pattern is a literal substring; comment text counts; a line that also
# contains "__atomic_" is not counted for the flag counts.
#
#   S6-1
#   exec_lock_calls  lines naming "lmx_msg_exec_lock(" or "lmx_msg_exec_unlock(" in tracked l2src *.c *.h *.lm1
#                    *.lm2 (tests included).
#   exec_lock_decl   lmx_message_exec.c lines naming "e->lock", plus a "lock;" field in the body of struct
#                    LmxMsgExec.
#   host_lock        lines naming "lmx_msg_host_lock(" or "lmx_msg_host_unlock(" in the same files, plus
#                    lmx_message_host.c lines naming "h->lock" or "h->shutting_down" without "__atomic_", plus a
#                    "lock;" field in the body of struct LmxMsgHostSync.
#   exec_fields      lines in lmx_message_exec.c naming "->nworkers", "->contexts_live", "->stopping",
#                    "->stopped", "->no_retire" or "->unbound_held" without "__atomic_".
#   S6-2
#   refs             lines naming "lmx_msg_endp_retain(", "lmx_msg_endp_release(", "lmx_msg_endp_refs(",
#                    "->refs" or "\refs" in the same files (the base's two InterlockedCompareExchange loops on &m->refs
#                    match "->refs"; the bare ICE needle was dropped 2026-09-16: it counted 178 selftest spin-cell
#                    lines at the base, never a holder count), plus a "refs;" field in
#                    the body of struct LmxMsg (lmx_message.h).
#   runtime_lists    "slots;" and " n;" fields in the body of struct LmxMsgRuntime (lmx_message.h), plus lines
#                    naming "->slots", "\slots", "->alloc_next" or "\alloc_next" in the same files.
param([ValidateSet('1', '2', 'all')][string]$Part = 'all')
$ErrorActionPreference = 'Stop'
Set-Location (Join-Path $PSScriptRoot '..')

function Get-ProbeText([string]$rel) {
    return [IO.File]::ReadAllText((Resolve-Path -LiteralPath $rel).ProviderPath).Replace("`r`n", "`n")
}

function Get-ProbeLines([string[]]$files, [string[]]$needles, [switch]$SkipAtomic) {
    $hits = @()
    foreach ($f in $files) {
        $n = 0
        foreach ($line in ((Get-ProbeText $f) -split "`n")) {
            if ($SkipAtomic -and $line.Contains('__atomic_')) { continue }
            foreach ($needle in $needles) {
                if ($line.Contains($needle)) { $n += 1; break }
            }
        }
        if ($n -gt 0) { $hits += [pscustomobject]@{ File = $f; Count = $n } }
    }
    return $hits
}

function Get-StructFieldCount([string]$rel, [string]$open, [string]$close, [string[]]$fieldTails) {
    $t = Get-ProbeText $rel
    $n = 0
    $inside = $false
    foreach ($line in ($t -split "`n")) {
        if (-not $inside) { if ($line.Contains($open)) { $inside = $true }; continue }
        if ($line.Contains($close)) { $inside = $false; continue }
        $trim = $line.Trim()
        foreach ($tail in $fieldTails) {
            if ($trim.EndsWith($tail) -and -not $trim.StartsWith('/*') -and -not $trim.StartsWith('*')) { $n += 1; break }
        }
    }
    return $n
}

function Get-Sum($hits) { $s = 0; foreach ($h in $hits) { $s += $h.Count }; return $s }

$files = @(& git ls-files -- 'l2src/*.c' 'l2src/*.h' 'l2src/*.lm1' 'l2src/*.lm2')
if ($LASTEXITCODE -ne 0 -or $files.Count -lt 10) { throw "git ls-files listed only $($files.Count) l2src sources" }

$hExecCalls = Get-ProbeLines $files @('lmx_msg_exec_lock(', 'lmx_msg_exec_unlock(')
$hExecDecl = Get-ProbeLines @('l2src/lmx_message_exec.c') @('e->lock')
$execDeclField = Get-StructFieldCount 'l2src/lmx_message_exec.c' 'typedef struct LmxMsgExec {' '} LmxMsgExec;' @(' lock;')
$hHostCalls = Get-ProbeLines $files @('lmx_msg_host_lock(', 'lmx_msg_host_unlock(')
$hHostInner = Get-ProbeLines @('l2src/lmx_message_host.c') @('h->lock', 'h->shutting_down') -SkipAtomic
$hostField = Get-StructFieldCount 'l2src/lmx_message_host.c' 'typedef struct LmxMsgHostSync {' '} LmxMsgHostSync;' @(' lock;')
$hExecFields = Get-ProbeLines @('l2src/lmx_message_exec.c') @('->nworkers', '->contexts_live', '->stopping', '->stopped', '->no_retire', '->unbound_held') -SkipAtomic
$hRefs = Get-ProbeLines $files @('lmx_msg_endp_retain(', 'lmx_msg_endp_release(', 'lmx_msg_endp_refs(', '->refs', '\refs')
$refsField = Get-StructFieldCount 'l2src/lmx_message.h' 'typedef struct LmxMsg {' '} LmxMsg;' @(' refs;')
$listFields = Get-StructFieldCount 'l2src/lmx_message.h' 'struct LmxMsgRuntime {' '};' @('slots;', ' n;')
$hLists = Get-ProbeLines $files @('->slots', '\slots', '->alloc_next', '\alloc_next')

$execLockCalls = Get-Sum $hExecCalls
$execLockDecl = (Get-Sum $hExecDecl) + $execDeclField
$hostLock = (Get-Sum $hHostCalls) + (Get-Sum $hHostInner) + $hostField
$execFields = Get-Sum $hExecFields
$refs = (Get-Sum $hRefs) + $refsField
$runtimeLists = $listFields + (Get-Sum $hLists)

"S6 probe at $((& git rev-parse --short HEAD) -join '')"
function Show-Hits([string]$label, $hits, [string]$extra) {
    "${label}:$extra"
    foreach ($h in $hits) { "  $($h.File) : $($h.Count)" }
}
Show-Hits 'exec_lock_calls by file' $hExecCalls ''
Show-Hits 'exec_lock_decl by file' $hExecDecl " (LmxMsgExec lock field: $execDeclField)"
Show-Hits 'host_lock calls by file' $hHostCalls ''
Show-Hits 'host_lock inside lmx_message_host.c' $hHostInner " (LmxMsgHostSync lock field: $hostField)"
Show-Hits 'exec_fields by file' $hExecFields ''
Show-Hits 'refs by file' $hRefs " (LmxMsg refs field: $refsField)"
Show-Hits 'runtime_lists by file' $hLists " (LmxMsgRuntime slots/n fields: $listFields)"
"S6 probe: exec_lock_calls=$execLockCalls exec_lock_decl=$execLockDecl host_lock=$hostLock exec_fields=$execFields | refs=$refs runtime_lists=$runtimeLists"
$g1 = ($execLockCalls -eq 0 -and $execLockDecl -eq 0 -and $hostLock -eq 0 -and $execFields -eq 0)
$g2 = ($refs -eq 0 -and $runtimeLists -eq 0)
if ($g1) { 'S6-1 GREEN' } else { 'S6-1 RED' }
if ($g2) { 'S6-2 GREEN' } else { 'S6-2 RED' }
if ($Part -eq '1') { if ($g1) { exit 0 } else { exit 1 } }
if ($Part -eq '2') { if ($g2) { exit 0 } else { exit 1 } }
if ($g1 -and $g2) { exit 0 }
exit 1
