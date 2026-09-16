# Start Grok Bot outbox FSW + 30-minute reconcile (one instance; mutex).
$here = Split-Path -Parent $MyInvocation.MyCommand.Path
Start-Process -FilePath "powershell.exe" -ArgumentList @(
  "-NoProfile","-ExecutionPolicy","Bypass","-File",(Join-Path $here "watch_outbox.ps1")
) -WindowStyle Minimized
Write-Host "Started grok_bot outbox watcher (minimized). Check build\grok_bot_outbox_watch\heartbeat.txt"
