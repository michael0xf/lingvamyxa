#Requires -Version 5.0
<#
.SYNOPSIS
  Watch a COLLEAGUE's outbox (their replies to us) with FileSystemWatcher, a
  30-minute fallback, a health pulse, and a peer-silence clock.

.DESCRIPTION
  One-shot helper for work_chat\<Peer>\outbox. The lead chat drops tickets into
  the peer's inbox and needs to wake when the peer publishes a reply there.
  The peer's own watcher (work_chat\<Peer>\scripts\watch_inbox.ps1) is NOT
  touched: it watches their inbox for our tickets; this one watches their
  outbox for their replies. Separate state directory, separate named mutex.

  Fires when:
    - a .txt in the peer's outbox is NEW or CHANGED relative to the last
      report (content hash, so a timestamp-only touch is not a reply);
    - the persisted PollSeconds fallback deadline arrives;
    - the peer is SILENT: the newest ticket in their inbox is newer than the
      newest reply in their outbox, and has been for more than SilentSeconds.
      A file watcher cannot notice that someone STOPPED -- silence produces no
      event -- so absence of a reply is made an event here (WAKING_CLAUDE.txt
      section 5). Reported once per outstanding ticket.

  Prints one compact JSON report and exits; the calling session re-arms it.
  A lifetime-held named mutex rejects a duplicate instance with exit code 1.
  Derived from watch_inbox.ps1 in this directory.

.PARAMETER RootPath
  Workspace root (contains work_chat\<Peer>).

.PARAMETER Peer
  The colleague's channel directory under work_chat (default: claude).

.PARAMETER StateDir
  Runtime state/log directory, OUTSIDE the watched tree
  (default: <RootPath>\build\fable_peer_<Peer>_watch).

.PARAMETER MutexName
  Global named mutex for single-instance ownership
  (default: Fable_Peer_<Peer>_Outbox_Mutex).

.PARAMETER PollSeconds
  Fallback reconciliation interval (default 1800 = 30 minutes).

.PARAMETER QuietSeconds
  Event batching quiet threshold (default 20).

.PARAMETER HeartbeatSeconds
  Health pulse interval (default 30).

.PARAMETER SilentSeconds
  How long the newest ticket may stay unanswered before PEER SILENT fires
  (default 1800). Tune to the peer's normal turnaround.
#>

param(
    [string]$RootPath = "C:\Nyasha_Planet\lingvamyxa",
    [string]$Peer = "claude",
    [string]$StateDir = "",
    [string]$MutexName = "",
    [int]$PollSeconds = 1800,
    [int]$QuietSeconds = 20,
    [int]$HeartbeatSeconds = 30,
    [int]$SilentSeconds = 1800
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

# === Configuration ===
$PeerPath = Join-Path $RootPath ("work_chat\" + $Peer)
$OutboxPath = Join-Path $PeerPath "outbox"
$InboxPath = Join-Path $PeerPath "inbox"
if (-not $StateDir) { $StateDir = Join-Path $RootPath ("build\fable_peer_" + $Peer + "_watch") }
if (-not $MutexName) { $MutexName = "Fable_Peer_" + $Peer + "_Outbox_Mutex" }
$WatchDir = $StateDir
$HeartbeatFile = Join-Path $WatchDir "heartbeat.txt"
$DeadlineFile = Join-Path $WatchDir "fallback_deadline.txt"
$LogFile = Join-Path $WatchDir "watcher.log"
$DeliveryStateFile = Join-Path $WatchDir "delivery_state.json"
$SilentStateFile = Join-Path $WatchDir "silent_reported.txt"
$ScriptPathForHeartbeat = $PSCommandPath

if (-not (Test-Path $OutboxPath)) { throw "peer outbox does not exist: $OutboxPath" }
if (-not (Test-Path $WatchDir)) { New-Item -ItemType Directory -Path $WatchDir -Force | Out-Null }

# === State ===
$script:lastEventTime = [DateTime]::MinValue
$script:lastReconcile = [DateTime]::MinValue
$script:watcherErrorCount = 0
$script:fallbackDue = [DateTime]::MinValue
$script:eventFired = $false
$script:quietDeadline = [DateTime]::MinValue
$runId = [Guid]::NewGuid().ToString("N").Substring(0,8)
$sidCreated = "FablePeerOutboxCreated_$runId"
$sidChanged = "FablePeerOutboxChanged_$runId"
$sidRenamed = "FablePeerOutboxRenamed_$runId"
$sidError = "FablePeerOutboxError_$runId"

# === Mutex (lifetime-held) ===
$script:mutex = $null

function AcquireMutex {
    try { $script:mutex = New-Object System.Threading.Mutex($false, $MutexName) } catch { return $false }
    $acquired = $false
    try { $acquired = $script:mutex.WaitOne(5000) }
    catch [System.Threading.AbandonedMutexException] { $acquired = $true; LogEvent "Acquired ABANDONED mutex $MutexName" }
    catch { $acquired = $false }
    if ($acquired) { return $true }
    $script:mutex.Dispose(); $script:mutex = $null
    return $false
}

function ReleaseMutex {
    if ($script:mutex) {
        try { $script:mutex.ReleaseMutex(); $script:mutex.Dispose() } catch {}
        $script:mutex = $null
    }
}

function LogEvent {
    param([string]$Message)
    $timestamp = [DateTime]::UtcNow.ToString("yyyy-MM-dd HH:mm:ss.fff")
    Add-Content -Path $LogFile -Value "[$timestamp] $Message" -Encoding UTF8 -ErrorAction SilentlyContinue
}

function WriteHeartbeat {
    param([string]$State)
    $process = Get-Process -Id $PID
    $heartbeat = @{
        timestamp = [DateTime]::UtcNow.ToString("O")
        pid = $PID
        process_start_time = $process.StartTime.ToUniversalTime().ToString("O")
        script_path = $ScriptPathForHeartbeat
        peer = $Peer
        watched = $OutboxPath
        state = $State
        fallback_due = if ($script:fallbackDue -ne [DateTime]::MinValue) { $script:fallbackDue.ToString("O") } else { "unknown" }
        last_event = if ($script:lastEventTime -ne [DateTime]::MinValue) { $script:lastEventTime.ToString("O") } else { "(none)" }
        last_reconcile = if ($script:lastReconcile -ne [DateTime]::MinValue) { $script:lastReconcile.ToString("O") } else { "(none)" }
        quiet_settle_seconds = $QuietSeconds
        heartbeat_seconds = $HeartbeatSeconds
        poll_seconds = $PollSeconds
        silent_seconds = $SilentSeconds
    }
    Set-Content -Path $HeartbeatFile -Value ($heartbeat | ConvertTo-Json -Compress) -Encoding UTF8
}

function GetFileHash256 {
    param([string]$FilePath)
    try { return (Get-FileHash -Path $FilePath -Algorithm SHA256 -ErrorAction SilentlyContinue).Hash } catch { return "" }
}

# Every completed reply in the peer's outbox, by basename, with its content hash.
function ScanOutbox {
    $replies = @{}
    Get-ChildItem $OutboxPath -Filter "*.txt" -ErrorAction SilentlyContinue |
        Where-Object { -not $_.PSIsContainer } | ForEach-Object {
        $replies[$_.BaseName] = @{ path = $_.FullName; hash = (GetFileHash256 $_.FullName); mtime = $_.LastWriteTimeUtc }
    }
    return $replies
}

function LoadDeliveryState {
    if (Test-Path $DeliveryStateFile) {
        try {
            $raw = Get-Content $DeliveryStateFile -Raw -ErrorAction Stop
            if ($raw) {
                $obj = $raw | ConvertFrom-Json
                $map = @{}
                $obj.PSObject.Properties | ForEach-Object { $map[$_.Name] = $_.Value }
                return $map
            }
        } catch {}
    }
    return @{}
}

function SaveDeliveryState { param($Map); ($Map | ConvertTo-Json -Compress) | Set-Content -Path $DeliveryStateFile -Encoding UTF8 }

# Replies that are new or changed since the last report.
function NewReplies {
    param($Replies, $PriorState)
    $new = @()
    foreach ($bn in ($Replies.Keys | Sort-Object)) {
        if (-not $PriorState.ContainsKey($bn) -or $PriorState[$bn] -ne $Replies[$bn].hash) { $new += $bn }
    }
    return $new
}

function LoadFallbackDeadline {
    if (Test-Path $DeadlineFile) {
        try {
            $content = (Get-Content $DeadlineFile -Raw).Trim()
            if ($content) {
                $parsed = [DateTime]::Parse($content, [System.Globalization.CultureInfo]::InvariantCulture, [System.Globalization.DateTimeStyles]::RoundtripKind)
                $script:fallbackDue = $parsed.ToUniversalTime()
                return
            }
        } catch {}
    }
    $script:fallbackDue = [DateTime]::UtcNow.AddSeconds($PollSeconds)
}

function SaveFallbackDeadline { Set-Content -Path $DeadlineFile -Value $script:fallbackDue.ToString("O") -Encoding UTF8 }

# The peer-silence clock. Newest ticket we sent (their inbox) vs newest reply
# they published (their outbox). Reported once per outstanding ticket basename.
function PeerSilence {
    $newestIn = Get-ChildItem $InboxPath -Filter "*.txt" -ErrorAction SilentlyContinue | Where-Object { -not $_.PSIsContainer } | Sort-Object LastWriteTimeUtc | Select-Object -Last 1
    $newestOut = Get-ChildItem $OutboxPath -Filter "*.txt" -ErrorAction SilentlyContinue | Where-Object { -not $_.PSIsContainer } | Sort-Object LastWriteTimeUtc | Select-Object -Last 1
    $result = @{ silent = $false; newest_inbox = ""; newest_outbox = ""; outstanding_seconds = 0 }
    if ($newestIn) { $result.newest_inbox = $newestIn.Name }
    if ($newestOut) { $result.newest_outbox = $newestOut.Name }
    if (-not $newestIn) { return $result }
    # answered: a reply with the same basename exists, or any reply is newer
    $answered = (Test-Path (Join-Path $OutboxPath $newestIn.Name)) -or ($newestOut -and $newestOut.LastWriteTimeUtc -ge $newestIn.LastWriteTimeUtc)
    if ($answered) { return $result }
    $age = ([DateTime]::UtcNow - $newestIn.LastWriteTimeUtc).TotalSeconds
    $result.outstanding_seconds = [Math]::Round($age)
    if ($age -lt $SilentSeconds) { return $result }
    $already = ""
    if (Test-Path $SilentStateFile) { $already = (Get-Content $SilentStateFile -Raw -ErrorAction SilentlyContinue).Trim() }
    if ($already -eq $newestIn.Name) { return $result }
    $result.silent = $true
    return $result
}

# === Main ===
if (-not (AcquireMutex)) {
    Write-Output ("{""error"":""already running"",""mutex"":""" + $MutexName + """}")
    exit 1
}
trap { ReleaseMutex; Write-Output "Error: $_"; exit 1 }

$startTime = [DateTime]::UtcNow
LoadFallbackDeadline
$deliveryState = LoadDeliveryState
$nextHeartbeat = $startTime.AddSeconds($HeartbeatSeconds)
$watcher = $null
$reason = ""
LogEvent "Peer-outbox watcher started (PID=$PID, peer=$Peer, poll=$PollSeconds, quiet=$QuietSeconds, heartbeat=$HeartbeatSeconds, silent=$SilentSeconds)"

function DrainEvents {
    $any = $false
    foreach ($sid in @($sidCreated, $sidChanged, $sidRenamed)) {
        $evts = Get-Event -SourceIdentifier $sid -ErrorAction SilentlyContinue
        foreach ($e in $evts) {
            $any = $true
            LogEvent "FSW $($e.SourceEventArgs.ChangeType): $($e.SourceEventArgs.Name)"
            Remove-Event -EventIdentifier $e.EventIdentifier
        }
    }
    $errEvts = Get-Event -SourceIdentifier $sidError -ErrorAction SilentlyContinue
    foreach ($e in $errEvts) {
        $any = $true
        $script:watcherErrorCount += 1
        LogEvent "FSW Error (count=$($script:watcherErrorCount)): $($e.SourceEventArgs.GetException().Message)"
        Remove-Event -EventIdentifier $e.EventIdentifier
    }
    if ($any) {
        $script:eventFired = $true
        $script:quietDeadline = [DateTime]::UtcNow.AddSeconds($QuietSeconds)
        $script:lastEventTime = [DateTime]::UtcNow
    }
}

try {
    $watcher = New-Object System.IO.FileSystemWatcher
    $watcher.Path = $OutboxPath
    $watcher.Filter = "*.txt"
    $watcher.IncludeSubdirectories = $false
    $watcher.NotifyFilter = [System.IO.NotifyFilters]::FileName -bor [System.IO.NotifyFilters]::LastWrite -bor [System.IO.NotifyFilters]::Size
    Register-ObjectEvent -InputObject $watcher -EventName "Created" -SourceIdentifier $sidCreated | Out-Null
    Register-ObjectEvent -InputObject $watcher -EventName "Changed" -SourceIdentifier $sidChanged | Out-Null
    Register-ObjectEvent -InputObject $watcher -EventName "Renamed" -SourceIdentifier $sidRenamed | Out-Null
    Register-ObjectEvent -InputObject $watcher -EventName "Error" -SourceIdentifier $sidError | Out-Null
    $watcher.EnableRaisingEvents = $true

    # Startup scan: fire only for replies new/changed since the last report.
    $scan = ScanOutbox
    $startupNew = @(NewReplies $scan $deliveryState)
    if ($startupNew.Count -gt 0) {
        $script:eventFired = $true
        $script:quietDeadline = [DateTime]::UtcNow.AddSeconds($QuietSeconds)
        LogEvent "Startup scan found $($startupNew.Count) new/changed replies"
    } else {
        LogEvent "Startup scan: $($scan.Count) replies, none new/changed since last report"
    }

    WriteHeartbeat "waiting"
    SaveFallbackDeadline

    $loop = $true
    while ($loop) {
        DrainEvents
        $now = [DateTime]::UtcNow
        if ($now -ge $nextHeartbeat) {
            WriteHeartbeat "waiting"
            $nextHeartbeat = $now.AddSeconds($HeartbeatSeconds)
            # the silence clock ticks with the pulse, not with file events
            $silence = PeerSilence
            if ($silence.silent) {
                LogEvent "PEER SILENT: $($silence.newest_inbox) unanswered for $($silence.outstanding_seconds)s"
                $reason = "peer_silent"
                $loop = $false
                break
            }
        }
        if ($script:eventFired -and $now -ge $script:quietDeadline) {
            $script:eventFired = $false
            $settle = ScanOutbox
            if ((@(NewReplies $settle $deliveryState)).Count -gt 0) {
                WriteHeartbeat "settling"
                LogEvent "Event batch settled after ${QuietSeconds}s quiet (new/changed replies present)"
                $reason = "reply"
                $loop = $false
                break
            } else {
                WriteHeartbeat "waiting"
                LogEvent "Event batch settled but no new/changed reply (touch-only noise); continuing"
            }
        }
        if ($now -ge $script:fallbackDue) {
            WriteHeartbeat "fallback"
            LogEvent "Fallback reconciliation deadline reached"
            $script:fallbackDue = $now.AddSeconds($PollSeconds)
            SaveFallbackDeadline
            $reason = "fallback"
            $loop = $false
            break
        }
        $eventDeadline = if ($script:eventFired) { $script:quietDeadline } else { [DateTime]::MaxValue }
        $minDeadline = ($nextHeartbeat, $eventDeadline, $script:fallbackDue | Sort-Object)[0]
        $sleepMs = 500
        if ($minDeadline -ne [DateTime]::MaxValue) {
            $msUntil = [Math]::Max(100, [int](($minDeadline - $now).TotalMilliseconds))
            $sleepMs = [Math]::Min($msUntil, 500)
        }
        Start-Sleep -Milliseconds $sleepMs
    }

    $watcher.EnableRaisingEvents = $false
    $final = ScanOutbox
    $newList = @(NewReplies $final $deliveryState)
    $silence = PeerSilence
    $script:lastReconcile = [DateTime]::UtcNow
    WriteHeartbeat "fired"
    SaveFallbackDeadline

    if ($reason -eq "peer_silent") { Set-Content -Path $SilentStateFile -Value $silence.newest_inbox -Encoding UTF8 }

    $report = @{
        peer = $Peer
        reason = $reason
        timestamp = [DateTime]::UtcNow.ToString("O")
        uptime_seconds = [Math]::Round(([DateTime]::UtcNow - $startTime).TotalSeconds, 1)
        new_count = $newList.Count
        new = @($newList)
        newest_inbox = $silence.newest_inbox
        newest_outbox = $silence.newest_outbox
        peer_silent = ($reason -eq "peer_silent")
        outstanding_seconds = $silence.outstanding_seconds
        watcher_error_count = $script:watcherErrorCount
    }
    LogEvent "Firing: reason=$reason new=$($newList.Count) silent=$($report.peer_silent)"

    $newState = @{}
    foreach ($bn in $final.Keys) { $newState[$bn] = $final[$bn].hash }
    SaveDeliveryState $newState

    Write-Output (ConvertTo-Json -InputObject $report -Compress)
    exit 0
} finally {
    foreach ($sid in @($sidCreated, $sidChanged, $sidRenamed, $sidError)) {
        Unregister-Event -SourceIdentifier $sid -ErrorAction SilentlyContinue
        Get-Event -SourceIdentifier $sid -ErrorAction SilentlyContinue | Remove-Event -ErrorAction SilentlyContinue
    }
    if ($watcher) { try { $watcher.EnableRaisingEvents = $false } catch {}; $watcher.Dispose() }
    ReleaseMutex
}
