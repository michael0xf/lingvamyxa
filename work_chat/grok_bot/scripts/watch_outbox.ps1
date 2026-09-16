#Requires -Version 5.0
<#
.SYNOPSIS
  Grok Bot outbox FileSystemWatcher + 30-minute reconcile.
.DESCRIPTION
  Watches work_chat\grok_bot\outbox for new *.txt (not in seen\).
  Batches after QuietSeconds, reconciles every PollSeconds.
  Optionally POSTs to Grok Bot webhook routine so the agent wakes immediately
  (fill webhookUrl + webhookSenderKey in watch_outbox_config.json from the routine panel).
  Mutex prevents duplicate watchers.
#>
param(
  [string]$ConfigPath = (Join-Path $PSScriptRoot "watch_outbox_config.json")
)
$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest
$cfg = Get-Content -LiteralPath $ConfigPath -Raw -Encoding UTF8 | ConvertFrom-Json
$WatchPath = $cfg.watchPath
$SeenPath = $cfg.seenPath
$WatchDir = $cfg.watchDir
$PollSeconds = [int]$cfg.pollSeconds
$QuietSeconds = [int]$cfg.quietSeconds
$HeartbeatSeconds = [int]$cfg.heartbeatSeconds
$MutexName = [string]$cfg.mutexName
$WebhookUrl = [string]$cfg.webhookUrl
$WebhookKey = [string]$cfg.webhookSenderKey
$HeartbeatFile = Join-Path $WatchDir "heartbeat.txt"
$LogFile = Join-Path $WatchDir "watcher.log"
$PendingFile = Join-Path $WatchDir "pending_new.txt"

@($WatchDir, $WatchPath, $SeenPath) | ForEach-Object {
  if (-not (Test-Path $_)) { New-Item -ItemType Directory -Path $_ -Force | Out-Null }
}

$script:mutex = $null
function AcquireMutex {
  $script:mutex = New-Object System.Threading.Mutex($false, ("Local\" + $MutexName))
  if (-not $script:mutex.WaitOne(0)) {
    $script:mutex.Dispose(); $script:mutex = $null; return $false
  }
  return $true
}
function ReleaseMutex {
  if ($script:mutex) {
    try { $script:mutex.ReleaseMutex(); $script:mutex.Dispose() } catch {}
    $script:mutex = $null
  }
}
function Log-Line([string]$msg) {
  $line = ("{0:o} {1}" -f [DateTime]::UtcNow, $msg)
  Add-Content -LiteralPath $LogFile -Value $line -Encoding UTF8
}
function Write-Heartbeat([string]$state) {
  $obj = @{
    timestamp = [DateTime]::UtcNow.ToString("o")
    pid = $PID
    state = $state
    outbox = $WatchPath
    webhookConfigured = [bool]($WebhookUrl -and $WebhookUrl.Trim().Length -gt 0)
  }
  Set-Content -LiteralPath $HeartbeatFile -Value ($obj | ConvertTo-Json -Compress) -Encoding UTF8
}
function Get-NewBasenames {
  Get-ChildItem -LiteralPath $WatchPath -Filter "*.txt" -File -ErrorAction SilentlyContinue |
    Where-Object { $_.Name -ne ".gitkeep" -and -not (Test-Path (Join-Path $SeenPath $_.Name)) } |
    Sort-Object Name |
    ForEach-Object { $_.Name }
}
function Invoke-WebhookWake([string[]]$names) {
  if (-not $WebhookUrl -or $WebhookUrl.Trim().Length -eq 0) {
    Log-Line "webhook skipped (empty webhookUrl); 30m Grok Bot cron is safety net"
    return
  }
  try {
    $headers = @{}
    if ($WebhookKey -and $WebhookKey.Trim().Length -gt 0) {
      $headers["Authorization"] = "Bearer $WebhookKey"
      $headers["X-Webhook-Key"] = $WebhookKey
    }
    $body = @{ source = "grok_bot_outbox_fsw"; new = @($names); at = [DateTime]::UtcNow.ToString("o") } | ConvertTo-Json -Compress
    Invoke-RestMethod -Method Post -Uri $WebhookUrl -Headers $headers -Body $body -ContentType "application/json; charset=utf-8" | Out-Null
    Log-Line ("webhook ok names=" + ($names -join ","))
  } catch {
    Log-Line ("webhook fail: " + $_.Exception.Message)
  }
}
function Flush-New {
  $names = @(Get-NewBasenames)
  if ($names.Count -eq 0) { return }
  # The pending file is the live signal; a reply is marked seen only after it is
  # written, so a flush that dies mid-way reports it again instead of losing it.
  # seenPath must NOT be the bot's own seen\ directory: there a basename means
  # "the bot took that request", and writing our markers there would corrupt its
  # claim bookkeeping.
  Set-Content -LiteralPath $PendingFile -Value ($names -join "`n") -Encoding UTF8
  foreach ($n in $names) { New-Item -ItemType File -Path (Join-Path $SeenPath $n) -Force | Out-Null }
  Log-Line ("NEW " + ($names -join ","))
  Invoke-WebhookWake $names
}

# The watcher is built by a function rather than inline, because the 30-minute
# reconcile proves it is still alive and rebuilds it in place when it is not: a
# disposed or muted FileSystemWatcher raises no events and looks exactly like a
# quiet channel, which is a failure this channel cannot afford.
function New-OutboxWatcher {
  if ($script:watcher) {
    try { $script:watcher.EnableRaisingEvents = $false; $script:watcher.Dispose() } catch {}
    $script:watcher = $null
  }
  Get-EventSubscriber -ErrorAction SilentlyContinue |
    Where-Object { $_.SourceIdentifier -like "GrokBotOutbox*" } |
    Unregister-Event -Force -ErrorAction SilentlyContinue
  $w = New-Object System.IO.FileSystemWatcher $WatchPath, "*.txt"
  $w.IncludeSubdirectories = $false
  $w.NotifyFilter = [IO.NotifyFilters]"FileName, LastWrite, Size"
  # A reply is written in chunks, and every chunk is an event: with the default
  # 8 KB buffer a large answer (305 KB was the first one seen) overflows it and
  # the events are DROPPED silently, so the flush never runs. 64 KB is the
  # documented maximum for this filter set; the Error event is registered and
  # logged, so an overflow is visible rather than silent, and the 30-minute
  # reconcile remains the backstop.
  $w.InternalBufferSize = 65536
  $script:sidC = "GrokBotOutboxCreated_" + [Guid]::NewGuid().ToString("N")
  $script:sidH = "GrokBotOutboxChanged_" + [Guid]::NewGuid().ToString("N")
  $script:sidR = "GrokBotOutboxRenamed_" + [Guid]::NewGuid().ToString("N")
  $script:sidE = "GrokBotOutboxError_" + [Guid]::NewGuid().ToString("N")
  Register-ObjectEvent -InputObject $w -EventName Created -SourceIdentifier $script:sidC | Out-Null
  Register-ObjectEvent -InputObject $w -EventName Changed -SourceIdentifier $script:sidH | Out-Null
  Register-ObjectEvent -InputObject $w -EventName Renamed -SourceIdentifier $script:sidR | Out-Null
  Register-ObjectEvent -InputObject $w -EventName Error -SourceIdentifier $script:sidE | Out-Null
  $w.EnableRaisingEvents = $true
  return $w
}

# The reconcile's second duty: prove the watcher is still watching. Each state
# checked here is one in which no event will ever arrive again while the channel
# looks quiet -- never built, muted (EnableRaisingEvents false), or the watched
# directory gone or renamed. The rebuild keeps the run's state: the seen set and
# the flush bookkeeping live outside the watcher object.
function Test-OutboxWatcher {
  $why = ""
  if (-not $script:watcher) { $why = "not built" }
  elseif (-not $script:watcher.EnableRaisingEvents) { $why = "muted (EnableRaisingEvents false)" }
  elseif (-not (Test-Path -LiteralPath $WatchPath)) { $why = "watch path missing: $WatchPath" }
  if ($why.Length -eq 0) { Log-Line "fsw alive check ok"; return }
  Log-Line ("fsw DEAD: " + $why + " -- rebuilding")
  try { $script:watcher = New-OutboxWatcher; Log-Line "fsw rebuilt" }
  catch { Log-Line ("fsw rebuild failed: " + $_.Exception.Message) }
}

if (-not (AcquireMutex)) { throw "Grok Bot outbox watcher already running" }
try {
  Log-Line "start pid=$PID"
  Write-Heartbeat "start"
  $script:watcher = $null
  $script:watcher = New-OutboxWatcher

  $lastEvent = [DateTime]::MinValue
  $lastReconcile = [DateTime]::UtcNow
  $nextPulse = [DateTime]::UtcNow
  $pending = $false

  # Startup baseline: replies already in outbox\ were answered before this
  # watcher existed -- the channel's history, not news. They are marked seen
  # WITHOUT being reported, so the first flush does not wake the consumer with
  # the whole backlog; only what arrives from now on is a new reply. The count
  # is logged, so a silent baseline is never mistaken for a quiet channel.
  $baseline = @(Get-ChildItem -LiteralPath $WatchPath -Filter "*.txt" -File -ErrorAction SilentlyContinue |
    Where-Object { $_.Name -ne ".gitkeep" -and -not (Test-Path (Join-Path $SeenPath $_.Name)) })
  foreach ($f in $baseline) { New-Item -ItemType File -Path (Join-Path $SeenPath $f.Name) -Force | Out-Null }
  Log-Line ("baseline " + @($baseline).Count + " existing outbox file(s) marked seen")

  # startup reconcile
  Flush-New
  $lastReconcile = [DateTime]::UtcNow

  while ($true) {
    $now = [DateTime]::UtcNow
    while ($true) {
      $ev = Get-Event -ErrorAction SilentlyContinue | Select-Object -First 1
      if (-not $ev) { break }
      if ($ev.SourceIdentifier -eq $script:sidE) { Log-Line "watcher error event (a dropped event, likely a buffer overflow): flushing what is on disk" }
      Remove-Event -EventIdentifier $ev.EventIdentifier -ErrorAction SilentlyContinue
      $lastEvent = $now
      $pending = $true
    }
    if ($pending -and $lastEvent -ne [DateTime]::MinValue -and ($now - $lastEvent).TotalSeconds -ge $QuietSeconds) {
      Flush-New
      $pending = $false
    }
    if (($now - $lastReconcile).TotalSeconds -ge $PollSeconds) {
      Flush-New
      # The 30-minute tick proves the watcher itself, not only the directory:
      # a dead FSW is silence, and silence is what a quiet channel looks like.
      Test-OutboxWatcher
      $lastReconcile = $now
      Write-Heartbeat "reconcile"
    }
    if ($now -ge $nextPulse) {
      Write-Heartbeat "pulse"
      $nextPulse = $now.AddSeconds($HeartbeatSeconds)
    }
    Start-Sleep -Milliseconds 500
  }
} finally {
  Get-EventSubscriber -ErrorAction SilentlyContinue | Unregister-Event -Force -ErrorAction SilentlyContinue
  if ($script:watcher) { $script:watcher.EnableRaisingEvents = $false; $script:watcher.Dispose() }
  ReleaseMutex
  Write-Heartbeat "stopped"
  Log-Line "stop"
}
