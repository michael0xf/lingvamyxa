#Requires -Version 5.0
<#
.SYNOPSIS
  Claude inbox FileSystemWatcher with 30-minute fallback and health pulse.

.DESCRIPTION
  Monitors work_chat\claude\inbox for .txt request files using FileSystemWatcher.
  Batches events after 20 seconds of quiet, reconciles every 30 minutes,
  and publishes health pulse every 30 seconds.

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
$MutexFile = Join-Path $WatchDir "mutex.lock"
$LogFile = Join-Path $WatchDir "watcher.log"

# === Ensure directories exist ===
@($WatchDir, $InboxPath, $SeenPath, $OutboxPath) | ForEach-Object {
    if (-not (Test-Path $_)) { New-Item -ItemType Directory -Path $_ -Force | Out-Null }
}

# === Mutex for preventing duplicate launches ===
function AcquireMutex {
    $maxWait = 5
    $waited = 0
    while ($waited -lt $maxWait) {
        try {
            # Try to create lock file exclusively
            $fs = [System.IO.File]::Open($MutexFile,
                [System.IO.FileMode]::CreateNew,
                [System.IO.FileAccess]::Write)
            $fs.Close()
            return $true
        } catch {
            if ($waited -eq 0) {
                Write-Output "Another watcher already running (mutex held)"
            }
            Start-Sleep -Seconds 1
            $waited++
        }
    }
    return $false
}

function ReleaseMutex {
    try { Remove-Item $MutexFile -ErrorAction SilentlyContinue } catch {}
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
        fallback_due = $fallbackDue.ToString("O")
        last_event = $lastEventTime.ToString("O") -replace '0001-01-01.*', "(none)"
        last_reconcile = $lastReconcile.ToString("O") -replace '0001-01-01.*', "(none)"
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
$fallbackDue = $startTime.AddSeconds($PollSeconds)
$nextHeartbeat = $startTime.AddSeconds($HeartbeatSeconds)
$quietDeadline = [DateTime]::MinValue
$eventFired = $false
$lastEventTime = [DateTime]::MinValue
$lastReconcile = [DateTime]::MinValue

$startPid = $PID
LogEvent "Watcher started (PID=$startPid, poll=$PollSeconds, quiet=$QuietSeconds, heartbeat=$HeartbeatSeconds)"

# === FileSystemWatcher setup ===
$watcher = New-Object System.IO.FileSystemWatcher
$watcher.Path = $InboxPath
$watcher.Filter = "*.txt"
$watcher.IncludeSubdirectories = $false
$watcher.NotifyFilter = [System.IO.NotifyFilters]::FileName

# Event handlers
$onEvent = {
    # Only track Created, Changed, Renamed; ignore Error here
    if ($Event.SourceEventArgs.ChangeType -in @("Created", "Changed", "Renamed")) {
        $global:eventFired = $true
        $global:quietDeadline = [DateTime]::UtcNow.AddSeconds($QuietSeconds)
        $global:lastEventTime = [DateTime]::UtcNow
    }
}

$onError = {
    $global:eventError = $Event.SourceEventArgs.GetException()
    LogEvent "FSW Error: $($global:eventError.Message)"
}

Register-ObjectEvent -InputObject $watcher -EventName "Created" -Action $onEvent | Out-Null
Register-ObjectEvent -InputObject $watcher -EventName "Changed" -Action $onEvent | Out-Null
Register-ObjectEvent -InputObject $watcher -EventName "Renamed" -Action $onEvent | Out-Null
Register-ObjectEvent -InputObject $watcher -EventName "Error" -Action $onError | Out-Null

$watcher.EnableRaisingEvents = $true

# Scan inbox immediately (before waiting for events)
$scanResult = ScanInbox
if ($scanResult.unseen.Count -gt 0) {
    $eventFired = $true
    $quietDeadline = [DateTime]::UtcNow.AddSeconds($QuietSeconds)
    LogEvent "Startup scan found $($scanResult.unseen.Count) unseen request(s)"
}

WriteHeartbeat "waiting"

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
    if ($eventFired -and $now -ge $quietDeadline) {
        $eventFired = $false
        WriteHeartbeat "settling"
        LogEvent "Event batch settled after $(($now - $quietDeadline.AddSeconds(-$QuietSeconds)).TotalSeconds)s quiet"
        $loop = $false
        break
    }

    # Check fallback deadline
    if ($now -ge $fallbackDue) {
        WriteHeartbeat "fallback"
        LogEvent "Fallback reconciliation deadline reached"
        $loop = $false
        break
    }

    # Calculate smart sleep: wake on next deadline
    $eventDeadline = if ($eventFired) { $quietDeadline } else { [DateTime]::MaxValue }
    $minDeadline = ($nextHeartbeat, $eventDeadline, $fallbackDue | Sort-Object)[0]

    if ($minDeadline -ne [DateTime]::MaxValue) {
        $msUntil = [Math]::Max(100, [int](($minDeadline - $now).TotalMilliseconds))
        $sleepMs = [Math]::Min($msUntil, 500)
    }

    Start-Sleep -Milliseconds $sleepMs
}

# === Reconcile and report ===
$watcher.EnableRaisingEvents = $false

$finalScan = ScanInbox
$lastReconcile = [DateTime]::UtcNow
WriteHeartbeat "fired"

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
}

LogEvent "Firing with $(($report.unseen).Count) unseen, $(($report.claimed_unfinished).Count) interrupted claims"

# Output compact result for Claude to parse
Write-Output (ConvertTo-Json -InputObject $report -Compress)

# Cleanup
Unregister-Event -SourceIdentifier * -ErrorAction SilentlyContinue
$watcher.Dispose()
ReleaseMutex

exit 0
