# Lock removal S5 acceptance probe (design d6/lock-removal, S5 section; the coordinator's ruling,
# 2026-09-15, after withdrawing "slots/n/next_addr into R0's record"). S5 deletes the dead retire
# queue, makes root_seq the constant 1, makes next_addr one order-free atomic counter (no owner lane),
# and collapses the root list to R0; slots and n go to S6 with refs. This runner prints four counts
# over tracked files and exits 0 only when all four are 0. It builds nothing and never joins a gate;
# it stays on its own branch (d6/lock-s5-red) and is merged onto the S5 branch only to measure.
#
#   retire_queue        lines naming the retire queue (retire_head, retire_tail, retire_next,
#                       retire_queued, lmx_msg_exec_flush_retire, lmx_msg_exec_retire_n,
#                       msg_exec_flush_retire) in tracked l2src *.c, *.h, *.lm1, *.lm2 files,
#                       comment text included.
#   root_seq            lines naming root_seq in l2src/lmx_message.h, lmx_message.lm1,
#                       lmx_message.lm2, lmx_message_exec.c, lmx_message_host.c.
#   next_addr_nonatomic lines naming next_addr in the same five files, except the declaration in
#                       struct LmxMsgRuntime, the one-time initialization before the runtime is shared
#                       (a line that is exactly "rt\next_addr: 1U" or "rt->next_addr = 1U;"), and lines
#                       that also contain "__atomic_" (the atomic fetch-add).
#   root_list_loops     lines matching the exact root-list walk and unlink patterns below, each a
#                       literal substring (not a regex), counted per file:
#                       lmx_message.lm1 and .lm2:  "fam: rt\root"  "ch: rt\root"  "rt\root: m\next_sibling"
#                       lmx_message_exec.c:        "for (ch = rt->root; ch != 0; ch = ch->next_sibling)"
#                                                  "drive_walk_list(rt, 0, rt->root)"
#                                                  "cur = rt->root;"  "rt->root = m->next_sibling;"
#                       Not counted: "m: rt\root" followed by m\first_child (R0's own child list) and the
#                       whole-tree walks from rt->root in rec_walk_locked / msg_find_any_locked, which stay
#                       valid with one root.
$ErrorActionPreference = 'Stop'
Set-Location (Join-Path $PSScriptRoot '..')

function Get-ProbeText([string]$rel) {
    return [IO.File]::ReadAllText((Resolve-Path -LiteralPath $rel).ProviderPath).Replace("`r`n", "`n")
}

function Get-ProbeLiteralCount([string]$text, [string]$needle) {
    $n = 0
    foreach ($line in ($text -split "`n")) {
        if ($line.Contains($needle)) { $n += 1 }
    }
    return $n
}

$core = @('l2src/lmx_message.h', 'l2src/lmx_message.lm1', 'l2src/lmx_message.lm2', 'l2src/lmx_message_exec.c',
    'l2src/lmx_message_host.c')

# 1. The retire queue.
$retireNames = @('retire_head', 'retire_tail', 'retire_next', 'retire_queued',
    'lmx_msg_exec_flush_retire', 'lmx_msg_exec_retire_n', 'msg_exec_flush_retire')
$files = @(& git ls-files -- 'l2src/*.c' 'l2src/*.h' 'l2src/*.lm1' 'l2src/*.lm2')
if ($LASTEXITCODE -ne 0 -or $files.Count -lt 10) { throw "git ls-files listed only $($files.Count) l2src sources" }
$retire = 0
$retireByFile = [ordered]@{}
foreach ($f in $files) {
    $t = Get-ProbeText $f
    $c = 0
    foreach ($line in ($t -split "`n")) {
        foreach ($name in $retireNames) {
            if ($line.Contains($name)) { $c += 1; break }
        }
    }
    if ($c -gt 0) { $retireByFile[$f] = $c; $retire += $c }
}

# 2. root_seq, and 3. non-atomic next_addr.
$hdr = Get-ProbeText 'l2src/lmx_message.h'
$sm = [regex]::Match($hdr, '(?s)\nstruct LmxMsgRuntime \{(.*?)\n\};')
if (-not $sm.Success) { throw 'struct LmxMsgRuntime body not found in l2src/lmx_message.h' }
$declLines = @(($sm.Groups[1].Value -split "`n") | Where-Object { $_.Contains('next_addr') } | ForEach-Object { $_.Trim() })
$rootSeq = 0
$rootSeqByFile = [ordered]@{}
$nextAddr = 0
$nextAddrLines = @()
foreach ($f in $core) {
    $t = Get-ProbeText $f
    $c = Get-ProbeLiteralCount $t 'root_seq'
    if ($c -gt 0) { $rootSeqByFile[$f] = $c; $rootSeq += $c }
    $k = 0
    foreach ($line in ($t -split "`n")) {
        if (-not $line.Contains('next_addr')) { continue }
        $trim = $line.Trim()
        if ($line.Contains('__atomic_')) { continue }
        if ($trim -eq 'rt\next_addr: 1U' -or $trim -eq 'rt->next_addr = 1U;') { continue }
        if ($f -eq 'l2src/lmx_message.h' -and $declLines -contains $trim) { continue }
        $k += 1
        $nextAddrLines += ('  {0}: {1}' -f $f, $trim)
    }
    $nextAddr += $k
}

# 4. Root-list loops, by exact pattern.
$lmPatterns = @('fam: rt\root', 'ch: rt\root', 'rt\root: m\next_sibling')
$cPatterns = @('for (ch = rt->root; ch != 0; ch = ch->next_sibling)', 'drive_walk_list(rt, 0, rt->root)',
    'cur = rt->root;', 'rt->root = m->next_sibling;')
$rootLoops = 0
$rootLines = @()
foreach ($f in @('l2src/lmx_message.lm1', 'l2src/lmx_message.lm2')) {
    $t = Get-ProbeText $f
    foreach ($p in $lmPatterns) {
        $c = Get-ProbeLiteralCount $t $p
        $rootLoops += $c
        $rootLines += ('  {0} "{1}": {2}' -f $f, $p, $c)
    }
}
$ct = Get-ProbeText 'l2src/lmx_message_exec.c'
foreach ($p in $cPatterns) {
    $c = Get-ProbeLiteralCount $ct $p
    $rootLoops += $c
    $rootLines += ('  l2src/lmx_message_exec.c "{0}": {1}' -f $p, $c)
}

"S5 probe at $((& git rev-parse --short HEAD) -join '')"
"retire_queue lines by file:"
foreach ($k in $retireByFile.Keys) { "  $k : $($retireByFile[$k])" }
"root_seq lines by file:"
foreach ($k in $rootSeqByFile.Keys) { "  $k : $($rootSeqByFile[$k])" }
"next_addr non-atomic lines:"
$nextAddrLines
"root_list_loops by pattern:"
$rootLines
"S5 probe: retire_queue=$retire root_seq=$rootSeq next_addr_nonatomic=$nextAddr root_list_loops=$rootLoops"
if ($retire -eq 0 -and $rootSeq -eq 0 -and $nextAddr -eq 0 -and $rootLoops -eq 0) {
    'S5 probe GREEN'
    exit 0
}
'S5 probe RED'
exit 1
