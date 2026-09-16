#Requires -Version 5.0
<#
.SYNOPSIS
  Grok Bot inbox FileSystemWatcher + 30-minute reconcile.
.DESCRIPTION
  Watches work_chat\grok_bot\inbox for new *.txt (not in seen\).
  Batches after QuietSeconds, reconciles every PollSeconds.
  Optionally POSTs to Grok Bot webhook routine so the agent wakes immediately
  (fill webhookUrl + webhookSenderKey in watch-config.json from the routine panel).
  Mutex prevents duplicate watchers.
#>
param(
  [string]$ConfigPath = (Join-Path $PSScriptRoot "watch-config.json")
)
$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest
$cfg = Get-Content -LiteralPath $ConfigPath -Raw -Encoding UTF8 | ConvertFrom-Json
$InboxPath = $cfg.inboxPath
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

@($WatchDir, $InboxPath, $SeenPath, $cfg.outboxPath) | ForEach-Object {
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
    inbox = $InboxPath
    webhookConfigured = [bool]($WebhookUrl -and $WebhookUrl.Trim().Length -gt 0)
  }
  Set-Content -LiteralPath $HeartbeatFile -Value ($obj | ConvertTo-Json -Compress) -Encoding UTF8
}
function Get-NewBasenames {
  Get-ChildItem -LiteralPath $InboxPath -Filter "*.txt" -File -ErrorAction SilentlyContinue |
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
    $body = @{ source = "grok_bot_inbox_fsw"; new = @($names); at = [DateTime]::UtcNow.ToString("o") } | ConvertTo-Json -Compress
    Invoke-RestMethod -Method Post -Uri $WebhookUrl -Headers $headers -Body $body -ContentType "application/json; charset=utf-8" | Out-Null
    Log-Line ("webhook ok names=" + ($names -join ","))
  } catch {
    Log-Line ("webhook fail: " + $_.Exception.Message)
  }
}
function Flush-New {
  $names = @(Get-NewBasenames)
  if ($names.Count -eq 0) { return }
  Set-Content -LiteralPath $PendingFile -Value ($names -join "`n") -Encoding UTF8
  Log-Line ("NEW " + ($names -join ","))
  Invoke-WebhookWake $names
}

if (-not (AcquireMutex)) { throw "Grok Bot inbox watcher already running" }
try {
  Log-Line "start pid=$PID"
  Write-Heartbeat "start"
  $watcher = New-Object System.IO.FileSystemWatcher $InboxPath, "*.txt"
  $watcher.IncludeSubdirectories = $false
  $watcher.NotifyFilter = [IO.NotifyFilters]"FileName, LastWrite, Size"
  $watcher.EnableRaisingEvents = $true
  $sidC = "GrokBotInboxCreated_" + [Guid]::NewGuid().ToString("N")
  $sidH = "GrokBotInboxChanged_" + [Guid]::NewGuid().ToString("N")
  $sidR = "GrokBotInboxRenamed_" + [Guid]::NewGuid().ToString("N")
  Register-ObjectEvent -InputObject $watcher -EventName Created -SourceIdentifier $sidC | Out-Null
  Register-ObjectEvent -InputObject $watcher -EventName Changed -SourceIdentifier $sidH | Out-Null
  Register-ObjectEvent -InputObject $watcher -EventName Renamed -SourceIdentifier $sidR | Out-Null

  $lastEvent = [DateTime]::MinValue
  $lastReconcile = [DateTime]::UtcNow
  $nextPulse = [DateTime]::UtcNow
  $pending = $false

  # startup reconcile
  Flush-New
  $lastReconcile = [DateTime]::UtcNow

  while ($true) {
    $now = [DateTime]::UtcNow
    while ($true) {
      $ev = Get-Event -ErrorAction SilentlyContinue | Select-Object -First 1
      if (-not $ev) { break }
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
  if ($watcher) { $watcher.EnableRaisingEvents = $false; $watcher.Dispose() }
  ReleaseMutex
  Write-Heartbeat "stopped"
  Log-Line "stop"
}
