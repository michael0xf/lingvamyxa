# wait_doorbell.ps1
# Helper executed as an Antigravity background task to wake the agent upon incoming work.
param(
    [string]$Root = "C:\Nyasha_Planet\lingvamyxa\work_chat\antigravity",
    [int]$FallbackIntervalSeconds = 1800,
    [int]$TimeoutSeconds = 0
)

$ErrorActionPreference = "Continue"

if (Test-Path -LiteralPath (Join-Path $Root 'DISABLED.txt')) {
    Write-Output 'Antigravity monitoring is disabled by the user.'
    exit 0
}

$inbox = Join-Path $Root "inbox"
$seen = Join-Path $Root "seen"
$outbox = Join-Path $Root "outbox"
$doorbell = Join-Path $Root ".doorbell"

function Get-Eligible {
    if (-not (Test-Path -LiteralPath $inbox)) { return @() }
    $items = @(Get-ChildItem -LiteralPath $inbox -Filter "*.txt" -File -ErrorAction SilentlyContinue | Sort-Object Name)
    $res = @()
    foreach ($i in $items) {
        $name = $i.Name
        if ($name.StartsWith(".")) { continue }
        $s = Join-Path $seen $name
        $o = Join-Path $outbox $name
        if (-not (Test-Path -LiteralPath $s) -and -not (Test-Path -LiteralPath $o)) {
            $res += $name
        }
    }
    return $res
}

# 1. Startup check: was doorbell already signaled?
if (Test-Path -LiteralPath $doorbell) {
    Start-Sleep -Milliseconds 50
    $pending = Get-Eligible
    if ($pending.Count -gt 0) {
        Remove-Item -LiteralPath $doorbell -Force -ErrorAction SilentlyContinue
        Write-Output "READY: $($pending -join ', ')"
        exit 0
    } else {
        # Doorbell was stale (tickets already handled)
        Remove-Item -LiteralPath $doorbell -Force -ErrorAction SilentlyContinue
    }
}

# 2. Event-driven waiting loop
$lastFallbackScan = [DateTime]::UtcNow
$deadline = if ($TimeoutSeconds -gt 0) { [DateTime]::UtcNow.AddSeconds($TimeoutSeconds) } else { [DateTime]::MaxValue }

while ([DateTime]::UtcNow -lt $deadline -and -not (Test-Path -LiteralPath (Join-Path $Root 'DISABLED.txt'))) {
    Start-Sleep -Milliseconds 500

    # Notification check: watch_antigravity signals $doorbell after 20s quiet settling
    if (Test-Path -LiteralPath $doorbell) {
        Start-Sleep -Milliseconds 50
        Remove-Item -LiteralPath $doorbell -Force -ErrorAction SilentlyContinue
        $tickets = Get-Eligible
        if ($tickets.Count -gt 0) {
            Write-Output "DOORBELL: $($tickets -join ', ')"
            exit 0
        }
    }

    # Fallback recovery scan: runs every $FallbackIntervalSeconds (default 1800s = 30m)
    # An empty scan produces NO notifications and does NOT exit / wake the AI.
    $now = [DateTime]::UtcNow
    if (($now - $lastFallbackScan).TotalSeconds -ge $FallbackIntervalSeconds) {
        $lastFallbackScan = $now
        $safetyTickets = Get-Eligible
        if ($safetyTickets.Count -gt 0) {
            Write-Output "FALLBACK: $($safetyTickets -join ', ')"
            exit 0
        }
    }
}

# 3. Timeout expiration (only reached if explicit TimeoutSeconds > 0 was specified)
$finalTickets = Get-Eligible
if ($finalTickets.Count -gt 0) {
    Write-Output "TIMEOUT_ELIGIBLE: $($finalTickets -join ', ')"
} else {
    Write-Output "TIMEOUT_IDLE"
}
exit 0
