# Start Grok Bot inbox FSW (one instance; mutex).
$here = Split-Path -Parent $MyInvocation.MyCommand.Path
Start-Process -FilePath "powershell.exe" -ArgumentList @(
  "-NoProfile","-ExecutionPolicy","Bypass","-File",(Join-Path $here "watch_inbox.ps1")
) -WindowStyle Minimized
Write-Host "Started grok_bot inbox watcher (minimized). Check build\grok_bot_watch\heartbeat.txt"
