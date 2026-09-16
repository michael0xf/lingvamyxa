# stop_channel.ps1
# Graceful shutdown of the OpenCode channel background stack (watcher + serve).
# Stops ONLY processes that provably belong to this channel (command-line match).
param(
    [string]$Root = "C:\Nyasha_Planet\lingvamyxa\work_chat\opencode",
    [switch]$Force
)

$ErrorActionPreference = "Continue"

function Stop-Mine([string]$pidFile, [string]$needle) {
    if (-not (Test-Path -LiteralPath $pidFile)) { Write-Output "no pid file $pidFile; nothing to stop"; return }
    $raw = (Get-Content -LiteralPath $pidFile -Raw -ErrorAction SilentlyContinue).Trim()
    if ($raw -notmatch '^\d+$') { Write-Output "pid file $pidFile has no valid PID; skipping"; return }
    $p = [int]$raw
    $proc = Get-CimInstance Win32_Process -Filter "ProcessId = $p" -ErrorAction SilentlyContinue
    if (-not $proc) { Write-Output "PID $p ($pidFile) not running; removing stale file"; Remove-Item -LiteralPath $pidFile -Force -ErrorAction SilentlyContinue; return }
    if ($proc.CommandLine -like "*$needle*") {
        if ($Force) {
            Stop-Process -Id $p -Force -ErrorAction SilentlyContinue
            Write-Output "stopped $needle PID $p (forced)"
        } else {
            # graceful: ask watcher/serve to stop via Stop-Process (SIGTERM-ish on Windows)
            $proc | ForEach-Object { }
            Stop-Process -Id $p -Force -ErrorAction SilentlyContinue
            Write-Output "stopped $needle PID $p"
        }
        Start-Sleep -Milliseconds 500
        Remove-Item -LiteralPath $pidFile -Force -ErrorAction SilentlyContinue
    } else {
        Write-Output "PID $p does not match channel command line ($needle); NOT stopping (foreign process)"
    }
}

Stop-Mine (Join-Path $Root "watcher.pid") "watch_opencode.ps1"
Stop-Mine (Join-Path $Root "serve.pid") "opencode serve"
Write-Output "channel stop complete"