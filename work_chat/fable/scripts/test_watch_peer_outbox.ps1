#Requires -Version 5.0
<#
.SYNOPSIS
  Accelerated, isolated acceptance tests for watch_peer_outbox.ps1.

.DESCRIPTION
  Throwaway root under build\fable_peer_watch_selftest_<stamp>\ with its own
  work_chat\tpeer\{inbox,outbox}, own state directory and a TEST mutex name, so
  nothing touches the production channels. Intervals are shortened (quiet 3s,
  fallback 20s, pulse 2s, silence 8s); results are accelerated evidence, not a
  real 30-minute run. Exit 0 when every check passes.
#>
param([string]$RepoRoot = "C:\Nyasha_Planet\lingvamyxa")
$ErrorActionPreference = "Stop"

$Script = Join-Path $RepoRoot "work_chat\fable\scripts\watch_peer_outbox.ps1"
$Stamp = (Get-Date).ToUniversalTime().ToString("yyyyMMdd_HHmmss")
$TestRoot = Join-Path $RepoRoot ("build\fable_peer_watch_selftest_" + $Stamp)
$Root = Join-Path $TestRoot "root"
$Inbox = Join-Path $Root "work_chat\tpeer\inbox"
$Outbox = Join-Path $Root "work_chat\tpeer\outbox"
$State = Join-Path $TestRoot "state"
$Mutex = "Fable_Peer_Outbox_TEST_" + $Stamp
New-Item -ItemType Directory -Force -Path $Inbox, $Outbox, $State | Out-Null

$results = @()
function Check([string]$name, [bool]$ok, [string]$detail = "") {
    $script:results += [pscustomobject]@{ name = $name; ok = $ok; detail = $detail }
    Write-Output (("[{0}] {1}{2}" -f ($(if ($ok) { "PASS" } else { "FAIL" }), $name, $(if ($detail) { " -- $detail" } else { "" }))))
}
function Start-Watcher([int]$silent = 8, [int]$poll = 20) {
    $args = @("-NoProfile", "-ExecutionPolicy", "Bypass", "-File", $Script, "-RootPath", $Root, "-Peer", "tpeer", "-StateDir", $State, "-MutexName", $Mutex, "-PollSeconds", $poll, "-QuietSeconds", 3, "-HeartbeatSeconds", 2, "-SilentSeconds", $silent)
    $out = Join-Path $TestRoot ("w_" + [Guid]::NewGuid().ToString("N").Substring(0, 6) + ".out")
    $p = Start-Process powershell.exe -ArgumentList $args -RedirectStandardOutput $out -PassThru -WindowStyle Hidden
    return @{ proc = $p; out = $out }
}
function Wait-Report($w, [int]$timeoutSec) {
    $w.proc.WaitForExit($timeoutSec * 1000) | Out-Null
    if (-not $w.proc.HasExited) { return $null }
    $txt = (Get-Content $w.out -Raw -ErrorAction SilentlyContinue)
    if (-not $txt) { return $null }
    $line = ($txt -split "`n" | Where-Object { $_.Trim().StartsWith("{") } | Select-Object -Last 1)
    if (-not $line) { return $null }
    return ($line | ConvertFrom-Json)
}

# T1: a reply published as .tmp then renamed to .txt fires with reason=reply.
$w = Start-Watcher
Start-Sleep -Seconds 3
$hb = Get-Content (Join-Path $State "heartbeat.txt") -Raw | ConvertFrom-Json
Check "A: heartbeat state waiting" ($hb.state -eq "waiting") $hb.state
Check "A: heartbeat pid is the watcher" ($hb.pid -eq $w.proc.Id) "hb=$($hb.pid) proc=$($w.proc.Id)"
$tmp = Join-Path $Outbox "20260101-000001.tmp"
Set-Content -Path $tmp -Value "reply one" -Encoding UTF8
Rename-Item -Path $tmp -NewName "20260101-000001.txt"
$r = Wait-Report $w 25
Check "T1: fired on a published reply" ($null -ne $r) "exit=$($w.proc.ExitCode)"
if ($r) {
    Check "T1: reason is reply" ($r.reason -eq "reply") $r.reason
    Check "T1: the reply id is reported" (@($r.new) -contains "20260101-000001") ($r.new -join ",")
}

# T2: nothing new -> the fallback fires, with no new replies.
$w = Start-Watcher -poll 12
$r = Wait-Report $w 30
Check "T2: fallback fired" ($null -ne $r -and $r.reason -eq "fallback") "$($r.reason) uptime=$($r.uptime_seconds)"
if ($r) { Check "T2: an unchanged reply is not reported again" ($r.new_count -eq 0) "new=$($r.new_count)" }

# T3: a touch that does not change bytes is not a reply.
$w = Start-Watcher -poll 12
Start-Sleep -Seconds 2
(Get-Item (Join-Path $Outbox "20260101-000001.txt")).LastWriteTime = Get-Date
$r = Wait-Report $w 30
Check "T3: touch-only does not fire as a reply" ($null -ne $r -and $r.reason -eq "fallback") $r.reason

# T4: the peer goes silent: a newer ticket in their inbox with no reply.
Set-Content -Path (Join-Path $Inbox "20260101-000002.txt") -Value "ticket two" -Encoding UTF8
$w = Start-Watcher -silent 6 -poll 60
$r = Wait-Report $w 30
Check "T4: peer silence fires" ($null -ne $r -and $r.reason -eq "peer_silent") "$($r.reason) outstanding=$($r.outstanding_seconds)s"
if ($r) { Check "T4: names the outstanding ticket" ($r.newest_inbox -eq "20260101-000002.txt") $r.newest_inbox }
# ...and only once per outstanding ticket
$w = Start-Watcher -silent 6 -poll 12
$r = Wait-Report $w 30
Check "T4b: the same silence is not reported twice" ($null -ne $r -and $r.reason -eq "fallback") $r.reason
# a reply to that ticket clears the silence
$w = Start-Watcher -silent 6 -poll 60
Start-Sleep -Seconds 2
Set-Content -Path (Join-Path $Outbox "20260101-000002.txt") -Value "reply two" -Encoding UTF8
$r = Wait-Report $w 25
Check "T4c: the reply wakes with reason=reply" ($null -ne $r -and $r.reason -eq "reply") $r.reason

# T5: a duplicate launch exits 1 and leaves the original running. The
# persisted fallback deadline from the short-poll runs above is cleared first,
# otherwise the original fires it within seconds -- which is the intended
# keep-the-deadline-across-re-arms behaviour, not a defect.
Remove-Item (Join-Path $State "fallback_deadline.txt") -ErrorAction SilentlyContinue
$w = Start-Watcher -poll 30
Start-Sleep -Seconds 2
$dup = Start-Watcher -poll 30
$dup.proc.WaitForExit(10000) | Out-Null
$dupTxt = (Get-Content $dup.out -Raw -ErrorAction SilentlyContinue)
# PassThru process objects do not always expose ExitCode after WaitForExit in
# PowerShell 5.1; the duplicate must have EXITED (not taken over), and the
# next check reads the reason it printed.
Check "T5: duplicate launch exits" ($dup.proc.HasExited) "exited=$($dup.proc.HasExited)"
Check "T5: duplicate reports already running" ([bool]("$dupTxt" -match "already running")) ("$dupTxt".Trim())
Check "T5: original still running" (-not $w.proc.HasExited)
Stop-Process -Id $w.proc.Id -Force -ErrorAction SilentlyContinue

$fails = @($results | Where-Object { -not $_.ok }).Count
Write-Output ""
Write-Output ("watch_peer_outbox selftest: {0} checks, {1} failures (test root: {2})" -f $results.Count, $fails, $TestRoot)
exit $(if ($fails -eq 0) { 0 } else { 1 })
