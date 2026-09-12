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
$script:eventQueue = @()
$script:watcherError = $null
$script:lastEventTime = [DateTime]::MinValue
$script:lastReconcile = [DateTime]::MinValue

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
    if (Test-Path $DeadlineFile) {
        try {
            $content = Get-Content $DeadlineFile -Raw
            if ($content) {
                $script:fallbackDue = [DateTime]::Parse($content)
                return
            }
        } catch {}
    }
    $script:fallbackDue = [DateTime]::UtcNow.AddSeconds($PollSeconds)
}

function SaveFallbackDeadline {
    Set-Content -Path $DeadlineFile -Value $script:fallbackDue.ToString("O") -Encoding UTF8
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
$nextHeartbeat = $startTime.AddSeconds($HeartbeatSeconds)
$quietDeadline = [DateTime]::MinValue
$eventFired = $false

$startPid = $PID
LogEvent "Watcher started (PID=$startPid, poll=$PollSeconds, quiet=$QuietSeconds, heartbeat=$HeartbeatSeconds)"

# === FileSystemWatcher setup ===
$watcher = New-Object System.IO.FileSystemWatcher
$watcher.Path = $InboxPath
$watcher.Filter = "*.txt"
$watcher.IncludeSubdirectories = $false
$watcher.NotifyFilter = [System.IO.NotifyFilters]::FileName -bor [System.IO.NotifyFilters]::LastWrite -bor [System.IO.NotifyFilters]::Size

# Event handlers
$onEvent = {
    if ($Event.SourceEventArgs.ChangeType -in @("Created", "Changed", "Renamed")) {
        $script:eventQueue += @{
            ChangeType = $Event.SourceEventArgs.ChangeType
            Name = $Event.SourceEventArgs.Name
            Time = [DateTime]::UtcNow
        }
        $script:eventFired = $true
        $script:quietDeadline = [DateTime]::UtcNow.AddSeconds($QuietSeconds)
        $script:lastEventTime = [DateTime]::UtcNow
    }
}

$onError = {
    $script:watcherError = $Event.SourceEventArgs.GetException()
    LogEvent "FSW Error: $($script:watcherError.Message)"
    $script:eventFired = $true
    $script:quietDeadline = [DateTime]::UtcNow.AddSeconds($QuietSeconds)
}

$createdId = Register-ObjectEvent -InputObject $watcher -EventName "Created" -Action $onEvent -ErrorAction SilentlyContinue | Select-Object -ExpandProperty Name
$changedId = Register-ObjectEvent -InputObject $watcher -EventName "Changed" -Action $onEvent -ErrorAction SilentlyContinue | Select-Object -ExpandProperty Name
$renamedId = Register-ObjectEvent -InputObject $watcher -EventName "Renamed" -Action $onEvent -ErrorAction SilentlyContinue | Select-Object -ExpandProperty Name
$errorId = Register-ObjectEvent -InputObject $watcher -EventName "Error" -Action $onError -ErrorAction SilentlyContinue | Select-Object -ExpandProperty Name

$watcher.EnableRaisingEvents = $true

# Scan inbox immediately (before waiting for events)
$scanResult = ScanInbox
if ($scanResult.unseen.Count -gt 0) {
    $script:eventFired = $true
    $script:quietDeadline = [DateTime]::UtcNow.AddSeconds($QuietSeconds)
    LogEvent "Startup scan found $($scanResult.unseen.Count) unseen request(s)"
}

WriteHeartbeat "waiting"
SaveFallbackDeadline

# === Main loop: wait for settle or fallback ===
$loop = $true
while ($loop) {
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
        LogEvent "Event batch settled. Processing $($script:eventQueue.Count) queued events"
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
}

LogEvent "Firing with $(($report.unseen).Count) unseen, $(($report.claimed_unfinished).Count) interrupted claims"

# Output compact result for Claude to parse
Write-Output (ConvertTo-Json -InputObject $report -Compress)

# === Cleanup: only this watcher's subscriptions ===
if ($createdId) { Unregister-Event -SourceIdentifier $createdId -ErrorAction SilentlyContinue }
if ($changedId) { Unregister-Event -SourceIdentifier $changedId -ErrorAction SilentlyContinue }
if ($renamedId) { Unregister-Event -SourceIdentifier $renamedId -ErrorAction SilentlyContinue }
if ($errorId) { Unregister-Event -SourceIdentifier $errorId -ErrorAction SilentlyContinue }
$watcher.Dispose()
ReleaseMutex

exit 0
