# start_channel.ps1
# Starts the OpenCode channel background stack (serve + watcher), hidden.
# Root: C:\Nyasha_Planet\lingvamyxa\work_chat\opencode\scripts
param(
    [string]$Root = "C:\Nyasha_Planet\lingvamyxa\work_chat\opencode",
    [int]$Port = 4097,
    [switch]$NoServe
)

$ErrorActionPreference = "Continue"

if (Test-Path -LiteralPath (Join-Path $Root 'monitoring.disabled')) {
    Write-Output 'OpenCode monitoring explicitly disabled; channel not rearmed.'
    return
}

# Password for the loopback server (basic auth). Generate once, reuse afterwards.
$pwFile = Join-Path $Root "server_password.txt"
if (-not (Test-Path -LiteralPath $pwFile)) {
    $pw = "oc-" + (-join ((48..57)+(65..90)+(97..122) | Get-Random -Count 24 | ForEach-Object {[char]$_}))
    Set-Content -LiteralPath $pwFile -Value $pw -Encoding ascii -NoNewline
}

function Start-Hidden([string]$file, [string[]]$arguments, [string]$workDir) {
    $psi = New-Object System.Diagnostics.ProcessStartInfo
    $psi.FileName = "powershell.exe"
    $psi.Arguments = "-NoProfile -ExecutionPolicy Bypass -WindowStyle Hidden -File `"$file`" " + ($arguments -join " ")
    $psi.WorkingDirectory = $workDir
    $psi.UseShellExecute = $false
    $psi.CreateNoWindow = $true
    return ([System.Diagnostics.Process]::Start($psi)).Id
}

# 1) Headless opencode server (loopback only, basic auth, shared data dir)
$servePidFile = Join-Path $Root "serve.pid"
$serveRunning = $false
if (Test-Path -LiteralPath $servePidFile) {
    $sp = [int](Get-Content -LiteralPath $servePidFile -Raw).Trim()
    $proc = Get-CimInstance Win32_Process -Filter "ProcessId = $sp" -ErrorAction SilentlyContinue
    if ($proc -and $proc.CommandLine -like "*opencode serve*$Port*") { $serveRunning = $true }
}
if (-not $serveRunning -and -not $NoServe) {
    $logDir = Join-Path $Root "logs"
    if (-not (Test-Path -LiteralPath $logDir)) { New-Item -ItemType Directory -Path $logDir -Force | Out-Null }
    $launcher = Join-Path $PSScriptRoot "run_serve.ps1"
    $p = Start-Hidden $launcher @($Port) "C:\Nyasha_Planet\lingvamyxa"
    Set-Content -LiteralPath $servePidFile -Value $p -Encoding ascii
    Write-Output "serve: launcher PID $p started (opencode serve on 127.0.0.1:$Port)"
} elseif ($NoServe) {
    Write-Output "serve: skipped (-NoServe)"
} else {
    Write-Output "serve: already running (PID from $servePidFile)"
}

# 2) Watcher
$watchScript = Join-Path $PSScriptRoot "watch_opencode.ps1"
$watcherPidFile = Join-Path $Root "watcher.pid"
$watcherRunning = $false
if (Test-Path -LiteralPath $watcherPidFile) {
    $wp = [int](Get-Content -LiteralPath $watcherPidFile -Raw).Trim()
    $proc = Get-CimInstance Win32_Process -Filter "ProcessId = $wp" -ErrorAction SilentlyContinue
    if ($proc -and $proc.CommandLine -like "*watch_opencode.ps1*") { $watcherRunning = $true }
}
if (-not $watcherRunning) {
    $p = Start-Hidden $watchScript @($Root) $Root
    Write-Output "watcher: started (launcher PID $p)"
    Start-Sleep -Seconds 3
    if (Test-Path -LiteralPath $watcherPidFile) {
        Write-Output "watcher: pid file = $((Get-Content -LiteralPath $watcherPidFile -Raw).Trim())"
    } else {
        Write-Output "watcher: pid file not created yet; check watcher.log"
    }
} else {
    Write-Output "watcher: already running (PID $wp)"
}
