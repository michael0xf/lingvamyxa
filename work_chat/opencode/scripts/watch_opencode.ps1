# watch_opencode.ps1
# Persistent background watcher helper for the OpenCode channel.
# C:\Nyasha_Planet\lingvamyxa\work_chat\opencode\
#
# - Immediate detection via FileSystemWatcher (Created/Renamed/Changed/Error)
# - 20-second quiet-window coalescing, batch notification
# - Independent fallback scan at startup and every 30 minutes (configurable)
# - Heartbeat updated ~once per minute regardless of events/agent activity
# - Single-instance protection: named mutex + .pid with process-identity check
# - Delivery into the PINNED opencode session via headless server REST API
#   (127.0.0.1 only, basic auth). Serializes on busy session.

param(
    [string]$Root = "C:\Nyasha_Planet\lingvamyxa\work_chat\opencode",
    [string]$SessionId = "ses_f72b606b8ffeAROE5aE5JDdwlh",
    [string]$HostName = "127.0.0.1",
    [int]$Port = 4097,
    [int]$QuietSeconds = 20,
    [int]$FallbackIntervalSeconds = 1800,
    [int]$HeartbeatIntervalSeconds = 60,
    [int]$BusyRetrySeconds = 15,
    [int]$MaxBusyMinutes = 10,
    [int]$EvTimeout = 1
)

$ErrorActionPreference = "Continue"

if (Test-Path -LiteralPath (Join-Path $Root 'monitoring.disabled')) {
    Write-Output 'OpenCode monitoring explicitly disabled; watcher not started.'
    return
}

$inbox   = Join-Path $Root "inbox"
$seen    = Join-Path $Root "seen"
$outbox  = Join-Path $Root "outbox"
$heartbeat = Join-Path $Root "heartbeat"
$pidFile = Join-Path $Root "watcher.pid"
$logFile = Join-Path $Root "watcher.log"
$pwFile  = Join-Path $Root "server_password.txt"
$site    = "http://${HostName}:${Port}"

$script:AuthHeader = $null

function Write-Log([string]$msg) {
    $stamp = [DateTime]::UtcNow.ToString("yyyy-MM-ddTHH:mm:ss.fffZ")
    $line = "$stamp [PID $PID] $msg"
    try {
        Add-Content -LiteralPath $logFile -Value $line -Encoding utf8
    } catch { }
}

# ---------- Single instance (named mutex + pid identity) ----------
$mutexName = "opencode_watcher_lingvamyxa"
$mutex = New-Object System.Threading.Mutex($false, $mutexName)
$hasMutex = $false
try { $hasMutex = $mutex.WaitOne(0, $false) } catch { $hasMutex = $false }
if (-not $hasMutex) { $mutex.Dispose(); exit 0 }

if (Test-Path -LiteralPath $pidFile) {
    $existingPidStr = (Get-Content -LiteralPath $pidFile -Raw -ErrorAction SilentlyContinue)
    if ($existingPidStr -match '^\s*(\d+)\s*$') {
        $existingPid = [int]$matches[1]
        if ($existingPid -ne $PID) {
            $existingProc = Get-CimInstance Win32_Process -Filter "ProcessId = $existingPid" -ErrorAction SilentlyContinue
            if ($existingProc -and $existingProc.CommandLine -like "*watch_opencode*") {
                Write-Log "Duplicate instance detected (PID $existingPid alive). Exiting."
                exit 0
            }
        }
    }
}
$PID | Set-Content -LiteralPath $pidFile -Encoding ascii

$scriptPath = $MyInvocation.MyCommand.Path
if (-not $scriptPath) { $scriptPath = "C:\Nyasha_Planet\lingvamyxa\work_chat\opencode\scripts\watch_opencode.ps1" }
$startedAtUtc = [DateTime]::UtcNow.ToString("yyyy-MM-ddTHH:mm:ssZ")
$eventsHandled = 0
$lastEventUtc = "none"

function Get-HttpHeaders {
    if ($null -eq $script:AuthHeader) {
        if (Test-Path -LiteralPath $pwFile) {
            $pw = (Get-Content -LiteralPath $pwFile -Raw -ErrorAction SilentlyContinue).Trim()
            if ($pw) {
                $auth = "opencode:$pw"
                $b64 = [Convert]::ToBase64String([Text.Encoding]::UTF8.GetBytes($auth))
                $script:AuthHeader = @{ Authorization = "Basic $b64" }
            } else {
                $script:AuthHeader = @{}
            }
        } else {
            $script:AuthHeader = @{}
        }
    }
    return $script:AuthHeader
}

function Update-Heartbeat([string]$status) {
    $nowUtc = [DateTime]::UtcNow.ToString("yyyy-MM-ddTHH:mm:ssZ")
    $hbText = @(
        "alive $nowUtc $status"
        "pid=$PID started=$startedAtUtc script=$scriptPath"
        "status=healthy events_handled=$eventsHandled last_event=$lastEventUtc"
        "session=$SessionId endpoint=$site"
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

# Returns: $true = busy, $false = idle, $null = cannot determine (server/HTTP error)
function Test-SessionBusy {
    try {
        # An errored/completed assistant message may lack finish. It is not a
        # live turn. Use server execution state, never historical message shape.
        $states = Invoke-RestMethod -Uri "$site/session/status" -Headers (Get-HttpHeaders) -Method GET -UseBasicParsing -TimeoutSec 20
        if ($null -eq $states) { return $null }
        $entry = $states.PSObject.Properties[$SessionId]
        if ($null -eq $entry) { return $false }
        if ($entry.Value.type -in @('busy', 'retry')) { return $true }
        if ($entry.Value.type -eq 'idle') { return $false }
        return $null
    } catch {
        Write-Log "Busy-check HTTP error: $($_.Exception.Message)"
        return $null
    }
}

function Send-Notification([string[]]$tickets) {
    $ticketList = ($tickets -join "`n    ")
    $nowUtc = [DateTime]::UtcNow.ToString("yyyy-MM-ddTHH:mm:ssZ")
    $text = @"
# opencode-inbox-notification
Inbound ticket(s) detected on the OpenCode channel (work_chat\opencode) at ${nowUtc}:

    $ticketList

Read C:\Nyasha_Planet\lingvamyxa\work_chat\opencode\ASSIGNMENT.txt and PROTOCOL.txt and follow them exactly.
For EACH ticket id listed above:
  1. Atomically claim it by creating seen\<id> (if seen\<id> already exists, skip that ticket: it is claimed or already done).
  2. Read the ENTIRE inbox\<id> file as the request (UTF-8).
  3. Perform only the work the ticket authorizes.
  4. Write the complete reply to outbox\<id>.tmp, flush, then atomically rename to outbox\<id>.txt (same basename).
Never delete or modify inbox files. Never overwrite an existing outbox reply silently.
Do not modify other agents' channels or their watchers, and do not stop their processes.
Reply in chat with a short summary; keep work minimal and authorized.
"@
    try {
        # Read on each dispatch: Codex can select the next turn's model without
        # restarting this helper or interrupting the active agent session.
        $selection = Get-Content -LiteralPath (Join-Path $Root 'model-selection.json') -Raw -ErrorAction Stop | ConvertFrom-Json -ErrorAction Stop
        if ([string]::IsNullOrWhiteSpace($selection.providerID) -or [string]::IsNullOrWhiteSpace($selection.modelID)) {
            throw 'Missing providerID/modelID in model-selection.json; refusing implicit model fallback.'
        }
        $body = @{ parts = @(@{ type = "text"; text = $text }); model = @{ providerID = $selection.providerID; modelID = $selection.modelID } }
        if ($selection.variant -and $selection.variant -ne 'default') { $body.variant = $selection.variant }
        $payload = $body | ConvertTo-Json -Depth 6 -Compress
        $resp = Invoke-WebRequest -Uri "$site/session/$SessionId/prompt_async" -Headers (Get-HttpHeaders) -Method POST -ContentType "application/json; charset=utf-8" -Body $payload -UseBasicParsing -TimeoutSec 30
        Write-Log "Notification delivered to session $SessionId using $($selection.providerID)/$($selection.modelID), variant=$($selection.variant) (HTTP $($resp.StatusCode)) for tickets: $($tickets -join ', ')"
        return $true
    } catch {
        Write-Log "Notification delivery FAILED for tickets $($tickets -join ', '): $($_.Exception.Message)"
        return $false
    }
}

Write-Log "Starting OpenCode watcher helper. PID=$PID, inbox=$inbox, session=$SessionId, endpoint=$site, fallback=$FallbackIntervalSeconds s"

# ---------- FileSystemWatcher (subscribed BEFORE initial scan) ----------
$fsw = New-Object System.IO.FileSystemWatcher
$fsw.Path = (Resolve-Path -LiteralPath $inbox).Path
$fsw.Filter = "*"
$fsw.IncludeSubdirectories = $false
$fsw.NotifyFilter = [IO.NotifyFilters]::FileName -bor [IO.NotifyFilters]::LastWrite -bor [IO.NotifyFilters]::Size
$fsw.InternalBufferSize = 65536

Register-ObjectEvent -InputObject $fsw -EventName Created -SourceIdentifier "OC_FSW_Created" | Out-Null
Register-ObjectEvent -InputObject $fsw -EventName Renamed -SourceIdentifier "OC_FSW_Renamed" | Out-Null
Register-ObjectEvent -InputObject $fsw -EventName Changed -SourceIdentifier "OC_FSW_Changed" | Out-Null
Register-ObjectEvent -InputObject $fsw -EventName Error   -SourceIdentifier "OC_FSW_Error"   | Out-Null

$fsw.EnableRaisingEvents = $true
Write-Log "FileSystemWatcher enabled on $inbox (Created/Renamed/Changed/Error)"

# ---------- State ----------
$script:notified = @{}
$deliveryPending = @()
$lastHeartbeat = [DateTime]::UtcNow
$lastFallbackScan = [DateTime]::UtcNow
$pendingEvent = $false
$quietDeadline = [DateTime]::MinValue
$lastDeliveryAttempt = [DateTime]::MinValue
$busySince = [DateTime]::MinValue
$busyLoggedAt = [DateTime]::MinValue

function Add-PendingNew([string[]]$eligibleList) {
    $added = @()
    foreach ($name in $eligibleList) {
        if (-not $script:notified.ContainsKey($name)) {
            $script:deliveryPending += $name
            $added += $name
        }
    }
    # keep unique + sorted
    $script:deliveryPending = @($script:deliveryPending | Sort-Object -Unique)
    return $added
}

# ---------- Initial startup scan ----------
$initial = Get-EligibleTickets
if ($initial.Count -gt 0) {
    $added = Add-PendingNew $initial
    Write-Log "Initial scan found $($initial.Count) eligible ticket(s): $($initial -join ', ') -> pending delivery"
} else {
    Write-Log "Initial scan complete: inbox empty/clean"
}
$initStatus = if ($deliveryPending.Count -gt 0) { "active (initial tickets pending: $($deliveryPending.Count))" } else { "idle (inbox empty)" }
Update-Heartbeat $initStatus

# ---------- Main loop ----------
try {
    while ($true) {
        $now = [DateTime]::UtcNow

        # Heartbeat ~every minute
        if (($now - $lastHeartbeat).TotalSeconds -ge $HeartbeatIntervalSeconds) {
            $status = if ($deliveryPending.Count -gt 0) { "pending delivery ($($deliveryPending.Count) ticket(s))" } else { "idle (inbox empty)" }
            Update-Heartbeat $status
            $lastHeartbeat = [DateTime]::UtcNow
        }

        # Fallback scan (30 min by default; short in isolated test config)
        if (($now - $lastFallbackScan).TotalSeconds -ge $FallbackIntervalSeconds) {
            $fb = Get-EligibleTickets
            if ($fb.Count -gt 0) {
                $added = Add-PendingNew $fb
                if ($added.Count -gt 0) {
                    Write-Log "Fallback scan found new eligible ticket(s): $($added -join ', ')"
                } else {
                    Write-Log "Fallback scan: $($fb.Count) eligible but all already notified; nothing new"
                }
            } else {
                Write-Log "Fallback scan complete: no new work"
            }
            $lastFallbackScan = [DateTime]::UtcNow
        }

        # 20s-quiet settling after relevant FSW events
        if ($pendingEvent -and ($now -ge $quietDeadline)) {
            Write-Log "Quiet window elapsed after filesystem events; scanning settled inbox."
            $pendingEvent = $false
            $settled = Get-EligibleTickets
            if ($settled.Count -gt 0) {
                $added = Add-PendingNew $settled
                if ($added.Count -gt 0) {
                    $eventsHandled += $added.Count
                    $lastEventUtc = [DateTime]::UtcNow.ToString("yyyy-MM-ddTHH:mm:ssZ")
                    Write-Log "Batch of $($added.Count) ticket(s) after quiet: $($added -join ', ')"
                } else {
                    Write-Log "Settling complete: eligible but already notified (temporary or already claimed)"
                }
            } else {
                Write-Log "Settling complete: no eligible tickets (temporary files or already claimed/done)"
            }
        }

        # Delivery: retain until session free / server reachable
        if ($deliveryPending.Count -gt 0 -and ($now - $lastDeliveryAttempt).TotalSeconds -ge $BusyRetrySeconds) {
            # A user-driven turn may have claimed queued work in the meantime.
            $deliveryPending = @($deliveryPending | Where-Object {
                -not (Test-Path -LiteralPath (Join-Path $seen $_)) -and
                -not (Test-Path -LiteralPath (Join-Path $outbox $_))
            })
            if ($deliveryPending.Count -eq 0) { continue }
            $lastDeliveryAttempt = [DateTime]::UtcNow
            $busy = Test-SessionBusy
            if ($busy -eq $false) {
                if (Send-Notification $deliveryPending) {
                    foreach ($name in $deliveryPending) { $script:notified[$name] = $true }
                    $deliveryPending = @()
                    $busySince = [DateTime]::MinValue
                }
                # on delivery failure, keep pending; retried next interval
            } elseif ($busy -eq $true) {
                if ($busySince -eq [DateTime]::MinValue) { $busySince = [DateTime]::UtcNow }
                $busyForMin = ($now - $busySince).TotalMinutes
                if (($now - $busyLoggedAt).TotalSeconds -ge 300) {
                    Write-Log "Session $SessionId busy for $([math]::Round($busyForMin,1)) min; retaining $($deliveryPending.Count) ticket(s): $($deliveryPending -join ', ')"
                    $busyLoggedAt = [DateTime]::UtcNow
                }
                # A long turn is still busy: do not force a second invocation
                # into the same session based only on elapsed time.
            } else {
                # $busy -eq $null (HTTP error) -> retain, retry later; throttled log
                if (($now - $busyLoggedAt).TotalSeconds -ge 300) {
                    Write-Log "Cannot reach server at $site for delivery; retaining $($deliveryPending.Count) ticket(s). Will retry."
                    $busyLoggedAt = [DateTime]::UtcNow
                }
            }
        }

        # Compute wait time until next important deadline
        $waitTimeout = $EvTimeout
        if ($pendingEvent -and $quietDeadline -gt [DateTime]::UtcNow) {
            $rem = ($quietDeadline - [DateTime]::UtcNow).TotalSeconds
            $waitTimeout = [Math]::Min(1.0, [Math]::Max(0.1, $rem))
        }

        $ev = Wait-Event -Timeout $waitTimeout

        if ($null -ne $ev) {
            $sourceId = $ev.SourceIdentifier
            $evArgs = $ev.SourceEventArgs

            if ($sourceId -eq "OC_FSW_Error") {
                Write-Log "WARNING: FileSystemWatcher Error event (buffer overflow or IO error). Rescanning immediately."
                Remove-Event -EventIdentifier $ev.EventIdentifier -ErrorAction SilentlyContinue
                while ($errEv = Get-Event -SourceIdentifier "OC_FSW_Error" -ErrorAction SilentlyContinue) {
                    Remove-Event -EventIdentifier $errEv.EventIdentifier -ErrorAction SilentlyContinue
                }
                $errTickets = Get-EligibleTickets
                if ($errTickets.Count -gt 0) {
                    $added = Add-PendingNew $errTickets
                    if ($added.Count -gt 0) { Write-Log "Post-error rescan found ticket(s): $($added -join ', ')" }
                    $lastDeliveryAttempt = [DateTime]::MinValue
                } else {
                    Write-Log "Post-error rescan complete: no new work"
                }
                continue
            }

            $name = $evArgs.Name
            $changeType = $evArgs.ChangeType
            Remove-Event -EventIdentifier $ev.EventIdentifier -ErrorAction SilentlyContinue

            # Drain the rest of the queue THIS cycle: process every event in the batch
            $otherEvents = @(Get-Event -ErrorAction SilentlyContinue)
            $batchNames = @()
            foreach ($oe in $otherEvents) {
                Remove-Event -EventIdentifier $oe.EventIdentifier -ErrorAction SilentlyContinue
                if ($oe.SourceIdentifier -eq "OC_FSW_Error") {
                    Write-Log "WARNING: FileSystemWatcher Error event in queue (buffer overflow). Rescanning immediately."
                    $errTickets = Get-EligibleTickets
                    if ($errTickets.Count -gt 0) {
                        $added = Add-PendingNew $errTickets
                        if ($added.Count -gt 0) { Write-Log "Post-error rescan found ticket(s): $($added -join ', ')" }
                    }
                    $lastDeliveryAttempt = [DateTime]::MinValue
                    continue
                }
                if ($oe.SourceEventArgs.Name) { $batchNames += $oe.SourceEventArgs.Name }
                if ($oe.SourceIdentifier -eq "OC_FSW_Renamed" -and $oe.SourceEventArgs.OldName) {
                    $batchNames += "renamed: $($oe.SourceEventArgs.OldName) -> $($oe.SourceEventArgs.Name)"
                }
            }
            if ($name) { $batchNames = @($name) + $batchNames }

            $anyRelevant = $false
            foreach ($n in $batchNames) {
                if (-not $n) { continue }
                $isRelevant = $false
                if ($n -like "*->*") {
                    # rename marker of old->new
                    $newPart = ($n -split " -> ")[-1]
                    if ($newPart.EndsWith(".txt", [System.StringComparison]::OrdinalIgnoreCase) -and -not $newPart.StartsWith(".")) { $isRelevant = $true }
                } elseif ($n.EndsWith(".txt", [System.StringComparison]::OrdinalIgnoreCase) -and -not $n.StartsWith(".")) {
                    $isRelevant = $true
                }
                if ($isRelevant) {
                    $anyRelevant = $true
                    if ($changeType -or ($name -eq $n)) {
                        Write-Log "FSW event: $changeType on $n"
                    }
                }
            }
            if (-not $anyRelevant -and $changeType) {
                Write-Log "FSW event (non-relevant): $changeType on $name"
            }
            if ($anyRelevant) {
                $pendingEvent = $true
                $quietDeadline = [DateTime]::UtcNow.AddSeconds($QuietSeconds)
            }
        }
    }
} finally {
    Write-Log "Watcher shutting down."
    Unregister-Event -SourceIdentifier "OC_FSW_Created" -ErrorAction SilentlyContinue
    Unregister-Event -SourceIdentifier "OC_FSW_Renamed" -ErrorAction SilentlyContinue
    Unregister-Event -SourceIdentifier "OC_FSW_Changed" -ErrorAction SilentlyContinue
    Unregister-Event -SourceIdentifier "OC_FSW_Error"   -ErrorAction SilentlyContinue
    if ($fsw) {
        $fsw.EnableRaisingEvents = $false
        $fsw.Dispose()
    }
    if ($hasMutex) {
        try { $mutex.ReleaseMutex() } catch { }
    }
    if (Test-Path -LiteralPath $pidFile) {
        $p = (Get-Content -LiteralPath $pidFile -Raw -ErrorAction SilentlyContinue).Trim()
        if ($p -eq "$PID") {
            Remove-Item -LiteralPath $pidFile -Force -ErrorAction SilentlyContinue
        }
    }
}
