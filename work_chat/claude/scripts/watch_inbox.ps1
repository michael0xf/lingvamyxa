#Requires -Version 5.0
<#
.SYNOPSIS
  Claude inbox FileSystemWatcher with 30-minute fallback and health pulse.

.DESCRIPTION
  Monitors work_chat\claude\inbox for .txt request files using FileSystemWatcher.
  Batches events after 20 seconds of quiet, reconciles every 30 minutes,
  and publishes health pulse every 30 seconds.
  Uses a lifetime-held System.Threading.Mutex to prevent duplicate watchers.

.PARAMETER RootPath
  Workspace root (contains work_chat\claude).

.PARAMETER PollSeconds
  Fallback reconciliation interval (default 1800 = 30 minutes).

.PARAMETER QuietSeconds
  Event batching quiet threshold (default 20).

.PARAMETER HeartbeatSeconds
  Health pulse interval (default 30).
#>

param(
    [string]$RootPath = "C:\Nyasha_Planet\lingvamyxa",
    [int]$PollSeconds = 1800,
    [int]$QuietSeconds = 20,
    [int]$HeartbeatSeconds = 30
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

# === Configuration ===
$InboxPath = Join-Path $RootPath "work_chat\claude\inbox"
$SeenPath = Join-Path $RootPath "work_chat\claude\seen"
$OutboxPath = Join-Path $RootPath "work_chat\claude\outbox"
$WatchDir = Join-Path $RootPath "build\claude_watch"
$HeartbeatFile = Join-Path $WatchDir "heartbeat.txt"
$DeadlineFile = Join-Path $WatchDir "fallback_deadline.txt"
$LogFile = Join-Path $WatchDir "watcher.log"
$DeliveryStateFile = Join-Path $WatchDir "delivery_state.json"

# === Ensure directories exist ===
@($WatchDir, $InboxPath, $SeenPath, $OutboxPath) | ForEach-Object {
    if (-not (Test-Path $_)) { New-Item -ItemType Directory -Path $_ -Force | Out-Null }
}

# === Global shared state for event handling ===
# NOTE: Register-ObjectEvent -Action scriptblocks do NOT share $script: scope
# with the main script (verified empirically: a $script: write inside the
# action block is invisible to the main script's $script: read). Events are
# therefore consumed via Get-Event/Remove-Event polling in the main loop
# instead of Action callbacks, which sidesteps the scope boundary entirely.
$script:lastEventTime = [DateTime]::MinValue
$script:lastReconcile = [DateTime]::MinValue
$script:watcherErrorCount = 0
$runId = [Guid]::NewGuid().ToString("N").Substring(0,8)
$sidCreated = "ClaudeInboxCreated_$runId"
$sidChanged = "ClaudeInboxChanged_$runId"
$sidRenamed = "ClaudeInboxRenamed_$runId"
$sidError = "ClaudeInboxError_$runId"

# === Mutex for preventing duplicate launches (lifetime-held) ===
$script:mutex = $null

function AcquireMutex {
    try {
        $script:mutex = New-Object System.Threading.Mutex($false, "Claude_Inbox_Watcher_Mutex")
        $acquired = $script:mutex.WaitOne(5000)
        if ($acquired) {
            return $true
        } else {
            $script:mutex.Dispose()
            $script:mutex = $null
            return $false
        }
    } catch {
        return $false
    }
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
    $scriptPath = "C:\Nyasha_Planet\lingvamyxa\work_chat\claude\scripts\watch_inbox.ps1"

    $heartbeat = @{
        timestamp = $timestamp
        pid = $currentPid
        process_start_time = $creationTime
        script_path = $scriptPath
        state = $State
        fallback_due = if ($script:fallbackDue) { $script:fallbackDue.ToString("O") } else { "unknown" }
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
    # CRITICAL: [DateTime]::Parse() on a round-tripped "O"-format UTC string
    # ('...Z') silently converts to LOCAL time while keeping Kind=Local, and
    # DateTime comparison operators compare raw ticks WITHOUT normalizing for
    # Kind. On a UTC-3 machine this makes a persisted future UTC deadline
    # compare as already-past, firing fallback immediately on every restart.
    # Verified empirically. Fix: parse with RoundtripKind so Kind=Utc survives.
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

# === Durable deduplication: track which (basename -> hash) pairs were already
# reported in a prior fired/settled batch, so an unchanged backlog of old
# unseen/claimed tickets does not force an immediate re-fire on every restart.
# A ticket whose hash changes (content edited) is treated as new again.
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
    param($ScanResult, $PriorState)
    $watch = @($ScanResult.unseen) + @($ScanResult.claimed | Where-Object { -not $ScanResult.requests[$_].outbox })
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
LogEvent "Watcher started (PID=$startPid, poll=$PollSeconds, quiet=$QuietSeconds, heartbeat=$HeartbeatSeconds)"

function DrainEvents {
    # Pulls all pending FSW events for THIS watcher's identifiers only and
    # advances quiet/fallback bookkeeping. Returns nothing; mutates script vars.
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

    # Subscribe WITHOUT -Action: events queue in PowerShell's global event store
    # and are consumed by polling Get-Event/Remove-Event in the main loop below.
    # This is the scope-safe pattern (see note above).
    Register-ObjectEvent -InputObject $watcher -EventName "Created" -SourceIdentifier $sidCreated | Out-Null
    Register-ObjectEvent -InputObject $watcher -EventName "Changed" -SourceIdentifier $sidChanged | Out-Null
    Register-ObjectEvent -InputObject $watcher -EventName "Renamed" -SourceIdentifier $sidRenamed | Out-Null
    Register-ObjectEvent -InputObject $watcher -EventName "Error" -SourceIdentifier $sidError | Out-Null

    $watcher.EnableRaisingEvents = $true

    # Scan inbox immediately (before waiting for events). Only force an
    # immediate fire-eligible state if something is actually new/changed
    # relative to the last reported batch (durable dedup) -- an unchanged
    # old backlog must not force an instant re-fire on every restart.
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

    # Check heartbeat interval
    if ($now -ge $nextHeartbeat) {
        WriteHeartbeat "waiting"
        $nextHeartbeat = $now.AddSeconds($HeartbeatSeconds)
    }

    # Check if event batch has settled
    if ($script:eventFired -and $now -ge $script:quietDeadline) {
        $script:eventFired = $false
        WriteHeartbeat "settling"
        LogEvent "Event batch settled after $QuietSeconds`s quiet"
        $loop = $false
        break
    }

    # Check fallback deadline
    if ($now -ge $script:fallbackDue) {
        WriteHeartbeat "fallback"
        LogEvent "Fallback reconciliation deadline reached"
        $script:fallbackDue = $now.AddSeconds($PollSeconds)
        SaveFallbackDeadline
        $loop = $false
        break
    }

    # Calculate smart sleep: wake on next deadline
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
        timestamp = [DateTime]::UtcNow.ToString("O")
        uptime_seconds = ([DateTime]::UtcNow - $startTime).TotalSeconds
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

    # Update durable dedup state: record hashes of everything just reported so
    # an unchanged repeat of this exact backlog won't force-fire on next start.
    $newState = @{}
    foreach ($fn in (@($finalScan.unseen) + @($report.claimed_unfinished))) {
        $newState[$finalScan.requests[$fn].basename] = $finalScan.requests[$fn].hash
    }
    SaveDeliveryState $newState

    # Output compact result for Claude to parse
    Write-Output (ConvertTo-Json -InputObject $report -Compress)
    exit 0
} finally {
    # === Cleanup: only this watcher's own exact subscriptions (by run-scoped ID) ===
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
