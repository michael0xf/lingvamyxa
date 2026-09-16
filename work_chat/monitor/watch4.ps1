# Robust poll-based watcher over 4 dirs: grok/opencode x inbox/outbox.
# Polling instead of FileSystemWatcher: no event-buffer overflow, no silent death.
# Emits: NEW <channel> <box> <basename>   /   UPD <channel> <box> <basename>

$root = 'C:\Nyasha_Planet\lingvamyxa\work_chat'
$targets = @(
    @{ Channel = 'grok';     Box = 'inbox'  },
    @{ Channel = 'grok';     Box = 'outbox' },
    @{ Channel = 'opencode'; Box = 'inbox'  },
    @{ Channel = 'opencode'; Box = 'outbox' }
)
$pollSec = 5

$state = @{}

function Get-Snapshot {
    param($dir)
    $out = @{}
    try {
        if (Test-Path -LiteralPath $dir) {
            foreach ($f in Get-ChildItem -LiteralPath $dir -File -Filter *.txt -ErrorAction Stop) {
                $out[$f.Name] = $f.LastWriteTimeUtc.Ticks
            }
        }
    } catch { }
    return $out
}

# Seed with what already exists so we don't replay the backlog.
foreach ($t in $targets) {
    $dir = Join-Path $root (Join-Path $t.Channel $t.Box)
    $key = "$($t.Channel)/$($t.Box)"
    $state[$key] = Get-Snapshot $dir
}

Write-Output "WATCH4_START pid=$PID poll=${pollSec}s dirs=$($targets.Count)"

while ($true) {
    foreach ($t in $targets) {
        $dir = Join-Path $root (Join-Path $t.Channel $t.Box)
        $key = "$($t.Channel)/$($t.Box)"
        try {
            $now = Get-Snapshot $dir
            $prev = $state[$key]
            foreach ($name in $now.Keys) {
                if (-not $prev.ContainsKey($name)) {
                    Write-Output "NEW $($t.Channel) $($t.Box) $name"
                } elseif ($prev[$name] -ne $now[$name]) {
                    Write-Output "UPD $($t.Channel) $($t.Box) $name"
                }
            }
            $state[$key] = $now
        } catch {
            # never die on a transient IO error
            Write-Output "WARN $key $($_.Exception.Message)"
        }
    }
    Start-Sleep -Seconds $pollSec
}
