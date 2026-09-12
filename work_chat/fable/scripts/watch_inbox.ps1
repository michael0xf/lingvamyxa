#Requires -Version 5.0
<#
.SYNOPSIS
  Fable inbox FileSystemWatcher with 30-minute fallback and health pulse.

.DESCRIPTION
  One-shot helper for the work_chat\<Channel>\inbox drop directory.
  Watches for completed .txt request files with FileSystemWatcher, batches a
  burst after QuietSeconds of quiet, fires at the latest at the persisted
  PollSeconds fallback deadline, and refreshes a health pulse every
  HeartbeatSeconds while waiting. Prints one compact JSON report and exits;
  the calling session re-arms it. A lifetime-held named mutex rejects a
  duplicate instance with exit code 1.

  Adapted from work_chat\claude\scripts\watch_inbox.ps1 with the channel,
  state directory, mutex name and script path made parameters so that
  isolated tests and a second chat cannot collide with the claude channel.

.PARAMETER RootPath
  Workspace root (contains work_chat\<Channel>).

.PARAMETER Channel
  Subdirectory of work_chat to serve (default: fable).

.PARAMETER StateDir
  Runtime state/log directory, OUTSIDE the watched tree
  (default: <RootPath>\build\<Channel>_watch).

.PARAMETER MutexName
  Global named mutex for single-instance ownership
  (default: Fable_Inbox_Watcher_Mutex).

.PARAMETER PollSeconds
  Fallback reconciliation interval (default 1800 = 30 minutes).

.PARAMETER QuietSeconds
  Event batching quiet threshold (default 20).

.PARAMETER HeartbeatSeconds
  Health pulse interval (default 30).
#>

param(
    [string]$RootPath = "C:\Nyasha_Planet\lingvamyxa",
    [string]$Channel = "fable",
    [string]$StateDir = "",
    [string]$MutexName = "Fable_Inbox_Watcher_Mutex",
    [int]$PollSeconds = 1800,
    [int]$QuietSeconds = 20,
    [int]$HeartbeatSeconds = 30
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

# === Configuration ===
$ChannelPath = Join-Path $RootPath ("work_chat\" + $Channel)
$InboxPath = Join-Path $ChannelPath "inbox"
$SeenPath = Join-Path $ChannelPath "seen"
$OutboxPath = Join-Path $ChannelPath "outbox"
if (-not $StateDir) { $StateDir = Join-Path $RootPath ("build\" + $Channel + "_watch") }
$WatchDir = $StateDir
$HeartbeatFile = Join-Path $WatchDir "heartbeat.txt"
$DeadlineFile = Join-Path $WatchDir "fallback_deadline.txt"
$LogFile = Join-Path $WatchDir "watcher.log"
$DeliveryStateFile = Join-Path $WatchDir "delivery_state.json"
$ScriptPathForHeartbeat = $PSCommandPath

# === Ensure directories exist ===
@($WatchDir, $InboxPath, $SeenPath, $OutboxPath) | ForEach-Object {
    if (-not (Test-Path $_)) { New-Item -ItemType Directory -Path $_ -Force | Out-Null }
}

# === Global shared state for event handling ===
# Register-ObjectEvent -Action scriptblocks do NOT share $script: scope with
# the main script. Events are therefore consumed via Get-Event/Remove-Event
# polling in the main loop instead of Action callbacks.
$script:lastEventTime = [DateTime]::MinValue
$script:lastReconcile = [DateTime]::MinValue
$script:watcherErrorCount = 0
$script:fallbackDue = [DateTime]::MinValue
$runId = [Guid]::NewGuid().ToString("N").Substring(0,8)
$sidCreated = "FableInboxCreated_$runId"
$sidChanged = "FableInboxChanged_$runId"
$sidRenamed = "FableInboxRenamed_$runId"
$sidError = "FableInboxError_$runId"

# === Mutex for preventing duplicate launches (lifetime-held) ===
$script:mutex = $null

function AcquireMutex {
    try {
        $script:mutex = New-Object System.Threading.Mutex($false, $MutexName)
    } catch {
        return $false
    }
    $acquired = $false
    try {
        $acquired = $script:mutex.WaitOne(5000)
    } catch [System.Threading.AbandonedMutexException] {
        # The previous owner exited without releasing while THIS instance was
        # already waiting on the same mutex object. The OS hands the mutex to
        # this thread together with the exception, so ownership IS acquired
        # here; treating it as failure would report "already running" for a
        # dead owner. Note the ordinary kill-then-relaunch case does not reach
        # this branch: when the killed process held the only handle, the named
        # mutex is destroyed and the relaunch simply creates a fresh one
        # (verified by test_watch_inbox.ps1 scenario E, which logs nothing
        # here). This catch covers the concurrent-waiter race only.
        $acquired = $true
        LogEvent "Acquired ABANDONED mutex $MutexName (previous instance exited without releasing)"
    } catch {
        $acquired = $false
    }
    if ($acquired) {
        return $true
    }
    $script:mutex.Dispose()
    $script:mutex = $null
    return $false
}

function ReleaseMutex {
    if ($script:mutex) {
        try {
            $script:mutex.ReleaseMutex()
            $script:mutex.Dispose()
        } catch {}
        $script:mutex = $null
    }
}

function WriteHeartbeat {
    param([string]$State)

    $currentPid = $PID
    $process = Get-Process -Id $currentPid
    $creationTime = $process.StartTime.ToUniversalTime().ToString("O")
    $timestamp = [DateTime]::UtcNow.ToString("O")

    $heartbeat = @{
        timestamp = $timestamp
        pid = $currentPid
        process_start_time = $creationTime
        script_path = $ScriptPathForHeartbeat
        channel = $Channel
        state = $State
        fallback_due = if ($script:fallbackDue -ne [DateTime]::MinValue) { $script:fallbackDue.ToString("O") } else { "unknown" }
        last_event = if ($script:lastEventTime -ne [DateTime]::MinValue) { $script:lastEventTime.ToString("O") } else { "(none)" }
        last_reconcile = if ($script:lastReconcile -ne [DateTime]::MinValue) { $script:lastReconcile.ToString("O") } else { "(none)" }
        quiet_settle_seconds = $QuietSeconds
        heartbeat_seconds = $HeartbeatSeconds
        poll_seconds = $PollSeconds
    }

    $content = $heartbeat | ConvertTo-Json -Compress
    Set-Content -Path $HeartbeatFile -Value $content -Encoding UTF8
}

# === Event deduplication by content hash ===
function GetFileHash {
    param([string]$FilePath)
    try {
        $hash = (Get-FileHash -Path $FilePath -Algorithm SHA256 -ErrorAction SilentlyContinue).Hash
        return $hash
    } catch {
        return ""
    }
}

function ScanInbox {
    $requests = @{}
    $unseen = @()
    $claimed = @()
    $completed = @()

    Get-ChildItem $InboxPath -Filter "*.txt" -ErrorAction SilentlyContinue |
        Where-Object { -not $_.PSIsContainer } | ForEach-Object {

        $filename = $_.Name
        $basename = $_.BaseName
        $seenMarker = Join-Path $SeenPath "$basename.txt"
        $outboxFile = Join-Path $OutboxPath "$basename.txt"

        $hash = GetFileHash $_.FullName
        $requests[$filename] = @{
            path = $_.FullName
            hash = $hash
            basename = $basename
            seen = (Test-Path $seenMarker)
            outbox = (Test-Path $outboxFile)
        }

        if ($requests[$filename].seen) {
            $claimed += $filename
            if ($requests[$filename].outbox) {
                $completed += $filename
            }
        } else {
            $unseen += $filename
        }
    }

    return @{
        requests = $requests
        unseen = @($unseen | Sort-Object)
        claimed = @($claimed | Sort-Object)
        completed = @($completed | Sort-Object)
    }
}

function LogEvent {
    param([string]$Message)
    $timestamp = [DateTime]::UtcNow.ToString("yyyy-MM-dd HH:mm:ss.fff")
    $logLine = "[$timestamp] $Message"
    Add-Content -Path $LogFile -Value $logLine -Encoding UTF8 -ErrorAction SilentlyContinue
}

function LoadFallbackDeadline {
    # [DateTime]::Parse() on a round-tripped "O"-format UTC string silently
    # converts to LOCAL time while keeping Kind=Local, and DateTime comparison
    # compares raw ticks without normalizing for Kind. Parse with RoundtripKind
    # so Kind=Utc survives; otherwise a persisted future deadline compares as
    # already past on a UTC-3 machine and fires on every restart.
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

function SaveFallbackDeadline {
    Set-Content -Path $DeadlineFile -Value $script:fallbackDue.ToString("O") -Encoding UTF8
}

# === Durable deduplication: basename -> content hash of every ticket in the
# last reported batch, so an unchanged backlog does not force an immediate
# re-fire on every restart. A ticket whose content changes counts as new.
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

function SaveDeliveryState {
    param($Map)
    ($Map | ConvertTo-Json -Compress) | Set-Content -Path $DeliveryStateFile -Encoding UTF8
}

function HasNewOrChangedWork {
    # Tracks EVERY ticket's content hash, including completed ones (seen +
    # outbox present): if a completed ticket's inbox content later changes,
    # that registers as new work. Distinguishing an ACK/WORKING reply from
    # DONE is the session's judgment during reconciliation, not string
    # matching here.
    param($ScanResult, $PriorState)
    $watch = @($ScanResult.unseen) + @($ScanResult.claimed)
    foreach ($fn in $watch) {
        $basename = $ScanResult.requests[$fn].basename
        $hash = $ScanResult.requests[$fn].hash
        if (-not $PriorState.ContainsKey($basename) -or $PriorState[$basename] -ne $hash) {
            return $true
        }
    }
    return $false
}

# === Main logic ===
if (-not (AcquireMutex)) {
    Write-Output ("{""error"":""already running"",""mutex"":""" + $MutexName + """}")
    exit 1
}

trap {
    ReleaseMutex
    Write-Output "Error: $_"
    exit 1
}

$startTime = [DateTime]::UtcNow
LoadFallbackDeadline
$deliveryState = LoadDeliveryState
$nextHeartbeat = $startTime.AddSeconds($HeartbeatSeconds)
$quietDeadline = [DateTime]::MinValue
$eventFired = $false
$watcher = $null

$startPid = $PID
LogEvent "Watcher started (PID=$startPid, channel=$Channel, poll=$PollSeconds, quiet=$QuietSeconds, heartbeat=$HeartbeatSeconds)"

function DrainEvents {
    # Pulls all pending FSW events for THIS watcher's identifiers only and
    # advances quiet/fallback bookkeeping.
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
        LogEvent "FSW Error event received (count=$($script:watcherErrorCount)): $($e.SourceEventArgs.GetException().Message)"
        Remove-Event -EventIdentifier $e.EventIdentifier
    }
    if ($any) {
        $script:eventFired = $true
        $script:quietDeadline = [DateTime]::UtcNow.AddSeconds($QuietSeconds)
        $script:lastEventTime = [DateTime]::UtcNow
    }
}

try {
    # === FileSystemWatcher setup ===
    $watcher = New-Object System.IO.FileSystemWatcher
    $watcher.Path = $InboxPath
    $watcher.Filter = "*.txt"
    $watcher.IncludeSubdirectories = $false
    $watcher.NotifyFilter = [System.IO.NotifyFilters]::FileName -bor [System.IO.NotifyFilters]::LastWrite -bor [System.IO.NotifyFilters]::Size

    # Subscribe WITHOUT -Action: events queue in PowerShell's event store and
    # are consumed by Get-Event/Remove-Event in the main loop.
    Register-ObjectEvent -InputObject $watcher -EventName "Created" -SourceIdentifier $sidCreated | Out-Null
    Register-ObjectEvent -InputObject $watcher -EventName "Changed" -SourceIdentifier $sidChanged | Out-Null
    Register-ObjectEvent -InputObject $watcher -EventName "Renamed" -SourceIdentifier $sidRenamed | Out-Null
    Register-ObjectEvent -InputObject $watcher -EventName "Error" -SourceIdentifier $sidError | Out-Null

    $watcher.EnableRaisingEvents = $true

    # Scan inbox immediately (do not rely on historical events). Fire only if
    # something is actually new/changed relative to the last reported batch.
    $scanResult = ScanInbox
    if (HasNewOrChangedWork -ScanResult $scanResult -PriorState $deliveryState) {
        $script:eventFired = $true
        $script:quietDeadline = [DateTime]::UtcNow.AddSeconds($QuietSeconds)
        LogEvent "Startup scan found new/changed work ($($scanResult.unseen.Count) unseen total)"
    } else {
        LogEvent "Startup scan: $($scanResult.unseen.Count) unseen total, none new/changed since last report"
    }

    WriteHeartbeat "waiting"
    SaveFallbackDeadline

    # === Main loop: wait for settle or fallback ===
    $loop = $true
    while ($loop) {
        DrainEvents
        $now = [DateTime]::UtcNow
        $sleepMs = 500

        if ($now -ge $nextHeartbeat) {
            WriteHeartbeat "waiting"
            $nextHeartbeat = $now.AddSeconds($HeartbeatSeconds)
        }

        # A raw FSW event (e.g. an mtime-only touch) counts toward the quiet
        # window, but firing is gated on HasNewOrChangedWork against the
        # last-reported hashes: touch-only noise resets quietly.
        if ($script:eventFired -and $now -ge $script:quietDeadline) {
            $script:eventFired = $false
            $settleScan = ScanInbox
            if (HasNewOrChangedWork -ScanResult $settleScan -PriorState $deliveryState) {
                WriteHeartbeat "settling"
                LogEvent "Event batch settled after $QuietSeconds`s quiet (new/changed work present)"
                $loop = $false
                break
            } else {
                WriteHeartbeat "waiting"
                LogEvent "Event batch settled after $QuietSeconds`s quiet but no new/changed work (touch-only noise); continuing to wait"
            }
        }

        if ($now -ge $script:fallbackDue) {
            WriteHeartbeat "fallback"
            LogEvent "Fallback reconciliation deadline reached"
            $script:fallbackDue = $now.AddSeconds($PollSeconds)
            SaveFallbackDeadline
            $loop = $false
            break
        }

        $eventDeadline = if ($script:eventFired) { $script:quietDeadline } else { [DateTime]::MaxValue }
        $minDeadline = ($nextHeartbeat, $eventDeadline, $script:fallbackDue | Sort-Object)[0]

        if ($minDeadline -ne [DateTime]::MaxValue) {
            $msUntil = [Math]::Max(100, [int](($minDeadline - $now).TotalMilliseconds))
            $sleepMs = [Math]::Min($msUntil, 500)
        }

        Start-Sleep -Milliseconds $sleepMs
    }

    # === Reconcile and report ===
    $watcher.EnableRaisingEvents = $false

    $finalScan = ScanInbox
    $script:lastReconcile = [DateTime]::UtcNow
    WriteHeartbeat "fired"
    SaveFallbackDeadline

    $report = @{
        channel = $Channel
        timestamp = [DateTime]::UtcNow.ToString("O")
        uptime_seconds = [Math]::Round(([DateTime]::UtcNow - $startTime).TotalSeconds, 1)
        unseen_count = $finalScan.unseen.Count
        unseen = $finalScan.unseen
        claimed_count = $finalScan.claimed.Count
        claimed_unfinished = @($finalScan.claimed | Where-Object {
            -not $finalScan.requests[$_].outbox
        })
        completed_count = $finalScan.completed.Count
        completed = $finalScan.completed
        watcher_error_count = $script:watcherErrorCount
    }

    LogEvent "Firing with $(($report.unseen).Count) unseen, $(($report.claimed_unfinished).Count) interrupted claims, $($script:watcherErrorCount) FSW errors"

    $newState = @{}
    foreach ($fn in (@($finalScan.unseen) + @($finalScan.claimed))) {
        $newState[$finalScan.requests[$fn].basename] = $finalScan.requests[$fn].hash
    }
    SaveDeliveryState $newState

    Write-Output (ConvertTo-Json -InputObject $report -Compress)
    exit 0
} finally {
    foreach ($sid in @($sidCreated, $sidChanged, $sidRenamed, $sidError)) {
        Unregister-Event -SourceIdentifier $sid -ErrorAction SilentlyContinue
        Get-Event -SourceIdentifier $sid -ErrorAction SilentlyContinue | Remove-Event -ErrorAction SilentlyContinue
    }
    if ($watcher) {
        try { $watcher.EnableRaisingEvents = $false } catch {}
        $watcher.Dispose()
    }
    ReleaseMutex
}
