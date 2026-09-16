# watch_antigravity.ps1
# Persistent background watcher helper for Antigravity channel
# C:\Nyasha_Planet\lingvamyxa\work_chat\antigravity\

param(
    [string]$Root = "C:\Nyasha_Planet\lingvamyxa\work_chat\antigravity",
    [int]$QuietSeconds = 20,
    [int]$FallbackIntervalSeconds = 1800,
    [int]$HeartbeatIntervalSeconds = 60
)

$ErrorActionPreference = "Continue"

if (Test-Path -LiteralPath (Join-Path $Root 'DISABLED.txt')) {
    Write-Output 'Antigravity monitoring is disabled by the user.'
    exit 0
}

$inbox = Join-Path $Root "inbox"
$seen = Join-Path $Root "seen"
$outbox = Join-Path $Root "outbox"
$heartbeat = Join-Path $Root "heartbeat"
$doorbell = Join-Path $Root ".doorbell"
$pidFile = Join-Path $Root ".pid"
$logFile = Join-Path $Root "watcher.log"

function Write-Log([string]$msg) {
    $stamp = [DateTime]::UtcNow.ToString("yyyy-MM-ddTHH:mm:ss.fffZ")
    $line = "$stamp [PID $PID] $msg"
    try {
        Add-Content -LiteralPath $logFile -Value $line -Encoding utf8
    } catch { }
}

# 1. Single-instance check
if (Test-Path -LiteralPath $pidFile) {
    $existingPidStr = (Get-Content -LiteralPath $pidFile -Raw -ErrorAction SilentlyContinue)
    if ($existingPidStr -match '^\s*(\d+)\s*$') {
        $existingPid = [int]$matches[1]
        if ($existingPid -ne $PID) {
            $existingProc = Get-CimInstance Win32_Process -Filter "ProcessId = $existingPid" -ErrorAction SilentlyContinue
            if ($existingProc -and $existingProc.CommandLine -like "*watch_antigravity.ps1*") {
                Write-Log "Duplicate instance detected (PID $existingPid is alive). Exiting."
                exit 0
            }
        }
    }
}
$PID | Set-Content -LiteralPath $pidFile -Encoding utf8

$scriptPath = $MyInvocation.MyCommand.Path
if (-not $scriptPath) { $scriptPath = "C:\Nyasha_Planet\lingvamyxa\work_chat\antigravity\watch_antigravity.ps1" }
$startedAtUtc = [DateTime]::UtcNow.ToString("yyyy-MM-ddTHH:mm:ssZ")
$eventsHandled = 0
$lastEventUtc = "none"

function Update-Heartbeat([string]$status) {
    $nowUtc = [DateTime]::UtcNow.ToString("yyyy-MM-ddTHH:mm:ssZ")
    $hbText = @(
        "alive $nowUtc $status"
        "pid=$PID started=$startedAtUtc script=$scriptPath"
        "status=healthy events_handled=$eventsHandled last_event=$lastEventUtc"
    ) -join "`n"
    try {
        [System.IO.File]::WriteAllText($heartbeat, $hbText + "`n", [System.Text.UTF8Encoding]::new($false))
    } catch {
        Write-Log "Failed to write heartbeat: $_"
    }
}

function Get-EligibleTickets {
    if (-not (Test-Path -LiteralPath $inbox)) { return @() }
    $items = @(Get-ChildItem -LiteralPath $inbox -Filter "*.txt" -File -ErrorAction SilentlyContinue | Sort-Object Name)
    $eligible = @()
    foreach ($item in $items) {
        $name = $item.Name
        if ($name.StartsWith(".")) { continue }
        $seenPath = Join-Path $seen $name
        $outboxPath = Join-Path $outbox $name
        if (-not (Test-Path -LiteralPath $seenPath) -and -not (Test-Path -LiteralPath $outboxPath)) {
            $eligible += $name
        }
    }
    return $eligible
}

function Signal-Doorbell([string[]]$tickets) {
    $ticketList = $tickets -join ","
    $nowUtc = [DateTime]::UtcNow.ToString("yyyy-MM-ddTHH:mm:ssZ")
    $content = "$nowUtc NEW $ticketList"
    try {
        [System.IO.File]::WriteAllText($doorbell, $content + "`n", [System.Text.UTF8Encoding]::new($false))
        Write-Log "Doorbell signalled for tickets: $ticketList"
    } catch {
        Write-Log "Failed to signal doorbell: $_"
    }
}

Write-Log "Starting Antigravity watcher helper. PID=$PID, Inbox=$inbox"

# 2. FileSystemWatcher setup
$fsw = New-Object System.IO.FileSystemWatcher
$fsw.Path = (Resolve-Path -LiteralPath $inbox).Path
$fsw.Filter = "*"
$fsw.IncludeSubdirectories = $false
$fsw.NotifyFilter = [IO.NotifyFilters]::FileName -bor [IO.NotifyFilters]::LastWrite -bor [IO.NotifyFilters]::Size
$fsw.InternalBufferSize = 65536

# Subscribe to events BEFORE initial directory scan
Register-ObjectEvent -InputObject $fsw -EventName Created -SourceIdentifier "AGY_FSW_Created" | Out-Null
Register-ObjectEvent -InputObject $fsw -EventName Renamed -SourceIdentifier "AGY_FSW_Renamed" | Out-Null
Register-ObjectEvent -InputObject $fsw -EventName Changed -SourceIdentifier "AGY_FSW_Changed" | Out-Null
Register-ObjectEvent -InputObject $fsw -EventName Error   -SourceIdentifier "AGY_FSW_Error"   | Out-Null

$fsw.EnableRaisingEvents = $true
Write-Log "Event subscriptions enabled on $inbox"

# 3. Initial directory scan on startup
$initialTickets = Get-EligibleTickets
if ($initialTickets.Count -gt 0) {
    Write-Log "Initial scan found $($initialTickets.Count) eligible tickets: $($initialTickets -join ', ')"
    Signal-Doorbell $initialTickets
    Update-Heartbeat "active (initial tickets pending: $($initialTickets.Count))"
} else {
    Write-Log "Initial scan complete: inbox empty/clean"
    Update-Heartbeat "idle (inbox empty)"
}

$lastHeartbeat = [DateTime]::UtcNow
$lastFallbackScan = [DateTime]::UtcNow
$pendingEvent = $false
$quietDeadline = [DateTime]::MinValue

try {
    while (-not (Test-Path -LiteralPath (Join-Path $Root 'DISABLED.txt'))) {
        $now = [DateTime]::UtcNow

        # 4. Heartbeat check (every ~60s)
        if (($now - $lastHeartbeat).TotalSeconds -ge $HeartbeatIntervalSeconds) {
            $eligible = Get-EligibleTickets
            $status = if ($eligible.Count -gt 0) { "pending work ($($eligible.Count) tickets)" } else { "idle (inbox empty)" }
            Update-Heartbeat $status
            $lastHeartbeat = [DateTime]::UtcNow
        }

        # 5. Fallback scan check (every 30m)
        if (($now - $lastFallbackScan).TotalSeconds -ge $FallbackIntervalSeconds) {
            Write-Log "Executing 30-minute fallback scan"
            $fallbackTickets = Get-EligibleTickets
            if ($fallbackTickets.Count -gt 0) {
                Write-Log "Fallback scan found $($fallbackTickets.Count) eligible tickets: $($fallbackTickets -join ', ')"
                Signal-Doorbell $fallbackTickets
            } else {
                Write-Log "Fallback scan complete: no new work"
            }
            $lastFallbackScan = [DateTime]::UtcNow
        }

        # Determine wait timeout: either towards quiet deadline or heartbeat interval
        $waitTimeout = 1.0
        if ($pendingEvent) {
            $remQuiet = ($quietDeadline - [DateTime]::UtcNow).TotalSeconds
            if ($remQuiet -le 0) {
                Write-Log "20 seconds of quiet elapsed. Scanning settled inbox."
                $pendingEvent = $false
                $settledTickets = Get-EligibleTickets
                if ($settledTickets.Count -gt 0) {
                    $eventsHandled += $settledTickets.Count
                    $lastEventUtc = [DateTime]::UtcNow.ToString("yyyy-MM-ddTHH:mm:ssZ")
                    Write-Log "Notifying batch of $($settledTickets.Count) tickets: $($settledTickets -join ', ')"
                    Signal-Doorbell $settledTickets
                } else {
                    Write-Log "Settling complete: no eligible tickets found (possibly temporary or already claimed)"
                }
                continue
            } else {
                $waitTimeout = [Math]::Min(1.0, [Math]::Max(0.1, $remQuiet))
            }
        }

        $ev = Wait-Event -Timeout $waitTimeout

        if ($null -ne $ev) {
            $sourceId = $ev.SourceIdentifier
            $evArgs = $ev.SourceEventArgs

            if ($sourceId -eq "AGY_FSW_Error") {
                Write-Log "WARNING: FileSystemWatcher Error event received (buffer overflow or IO error). Rescanning immediately."
                Remove-Event -EventIdentifier $ev.EventIdentifier -ErrorAction SilentlyContinue
                while ($errEv = Get-Event -SourceIdentifier "AGY_FSW_Error" -ErrorAction SilentlyContinue) {
                    Remove-Event -EventIdentifier $errEv.EventIdentifier -ErrorAction SilentlyContinue
                }
                $errTickets = Get-EligibleTickets
                if ($errTickets.Count -gt 0) {
                    Signal-Doorbell $errTickets
                }
                continue
            }

            $name = $evArgs.Name
            $changeType = $evArgs.ChangeType
            Remove-Event -EventIdentifier $ev.EventIdentifier -ErrorAction SilentlyContinue

            $otherEvents = @(Get-Event -ErrorAction SilentlyContinue)
            foreach ($oe in $otherEvents) {
                Remove-Event -EventIdentifier $oe.EventIdentifier -ErrorAction SilentlyContinue
            }

            $isRelevant = $false
            if ($name) {
                if ($name.EndsWith(".txt", [System.StringComparison]::OrdinalIgnoreCase)) {
                    $isRelevant = $true
                }
            }
            if ($sourceId -eq "AGY_FSW_Renamed") {
                $oldName = $evArgs.OldName
                if ($name -and $name.EndsWith(".txt", [System.StringComparison]::OrdinalIgnoreCase)) {
                    $isRelevant = $true
                    Write-Log "Rename detected: $oldName -> $name"
                }
            }

            if ($isRelevant) {
                Write-Log "FSW event: $changeType on $name. Resetting 20s quiet window."
                $pendingEvent = $true
                $quietDeadline = [DateTime]::UtcNow.AddSeconds($QuietSeconds)
            }
        }
    }
} finally {
    Write-Log "Watcher shutting down."
    Unregister-Event -SourceIdentifier "AGY_FSW_Created" -ErrorAction SilentlyContinue
    Unregister-Event -SourceIdentifier "AGY_FSW_Renamed" -ErrorAction SilentlyContinue
    Unregister-Event -SourceIdentifier "AGY_FSW_Changed" -ErrorAction SilentlyContinue
    Unregister-Event -SourceIdentifier "AGY_FSW_Error"   -ErrorAction SilentlyContinue
    if ($fsw) {
        $fsw.EnableRaisingEvents = $false
        $fsw.Dispose()
    }
    if (Test-Path -LiteralPath $pidFile) {
        $p = (Get-Content -LiteralPath $pidFile -Raw -ErrorAction SilentlyContinue).Trim()
        if ($p -eq "$PID") {
            Remove-Item -LiteralPath $pidFile -Force -ErrorAction SilentlyContinue
        }
    }
}
