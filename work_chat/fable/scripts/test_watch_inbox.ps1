#Requires -Version 5.0
<#
.SYNOPSIS
  Accelerated, isolated acceptance tests for watch_inbox.ps1 (INBOX_WATCHERS.md
  section 6, items 1 and 3-10). Item 2 (an actual session wake) cannot be
  proven by a script; it is proven by arming the production watcher and
  observing the background-task notification in the session.

.DESCRIPTION
  Uses a throwaway root under build\fable_watch\selftest_<stamp>\ with its own
  work_chat\fable\{inbox,outbox,seen}, its own state directory and a TEST
  mutex name, so nothing touches the production channel or the claude channel.
  Intervals are shortened (quiet 3s, fallback 20s, pulse 2s); the results are
  accelerated evidence, not a real 30-minute run.

  Exit code 0 when every check passes, 1 otherwise. A report is written to
  <test root>\report.txt.
#>

param(
    [string]$RepoRoot = "C:\Nyasha_Planet\lingvamyxa"
)

$ErrorActionPreference = "Stop"

$Script = Join-Path $RepoRoot "work_chat\fable\scripts\watch_inbox.ps1"
$Stamp = (Get-Date).ToUniversalTime().ToString("yyyyMMdd_HHmmss")
$TestRoot = Join-Path $RepoRoot ("build\fable_watch\selftest_" + $Stamp)
$Root = Join-Path $TestRoot "root"
$Inbox = Join-Path $Root "work_chat\fable\inbox"
$Seen = Join-Path $Root "work_chat\fable\seen"
$Outbox = Join-Path $Root "work_chat\fable\outbox"
$State = Join-Path $TestRoot "state"
$Mutex = "Fable_Inbox_Watcher_Mutex_TEST_" + $Stamp
$LogFile = Join-Path $State "watcher.log"
$DeadlineFile = Join-Path $State "fallback_deadline.txt"
$ReportFile = Join-Path $TestRoot "report.txt"

$script:results = @()
$script:started = @()

function Check {
    param([string]$Name, [bool]$Cond, [string]$Detail = "")
    $tag = if ($Cond) { "PASS" } else { "FAIL" }
    $line = "[$tag] $Name" + $(if ($Detail) { " -- $Detail" } else { "" })
    $script:results += $line
    Write-Host $line
}

function Start-Watcher {
    param([int]$Poll, [int]$Quiet, [int]$Hb, [string]$OutFile, [string]$MutexName = $Mutex)
    $args = @(
        "-NoProfile", "-ExecutionPolicy", "Bypass", "-File", $Script,
        "-RootPath", $Root, "-Channel", "fable", "-StateDir", $State,
        "-MutexName", $MutexName,
        "-PollSeconds", $Poll, "-QuietSeconds", $Quiet, "-HeartbeatSeconds", $Hb
    )
    $p = Start-Process -FilePath "powershell.exe" -ArgumentList $args -PassThru -NoNewWindow `
        -RedirectStandardOutput $OutFile -RedirectStandardError ($OutFile + ".err")
    $null = $p.Handle   # cache the handle so ExitCode is readable after exit
    $script:started += $p
    return $p
}

function Read-Heartbeat {
    $raw = Get-Content (Join-Path $State "heartbeat.txt") -Raw
    return ($raw | ConvertFrom-Json)
}

function Read-Report {
    param([string]$OutFile)
    $raw = (Get-Content $OutFile -Raw).Trim()
    return ($raw | ConvertFrom-Json)
}

function Parse-Utc {
    param([string]$S)
    return [DateTime]::Parse($S, [System.Globalization.CultureInfo]::InvariantCulture, [System.Globalization.DateTimeStyles]::RoundtripKind).ToUniversalTime()
}

function Log-Text { if (Test-Path $LogFile) { Get-Content $LogFile -Raw } else { "" } }

try {
    New-Item -ItemType Directory -Path $Inbox, $Seen, $Outbox, $State -Force | Out-Null
    Write-Host "Test root: $TestRoot"
    Write-Host "Mutex:     $Mutex"

    # ---------------------------------------------------------------- A
    # T8 two advancing pulses, T9 duplicate rejected, T1 one batch for a burst
    $outA = Join-Path $TestRoot "A.out"
    $pA = Start-Watcher -Poll 120 -Quiet 3 -Hb 2 -OutFile $outA
    Start-Sleep -Seconds 4
    $hb1 = Read-Heartbeat
    Check "A: heartbeat state is waiting" ($hb1.state -eq "waiting") $hb1.state
    Check "A: heartbeat pid is the watcher" ($hb1.pid -eq $pA.Id) "hb=$($hb1.pid) proc=$($pA.Id)"
    Check "A: heartbeat script path is the fable script" ($hb1.script_path -eq $Script) $hb1.script_path
    Start-Sleep -Seconds 5
    $hb2 = Read-Heartbeat
    Check "T8: two advancing health pulses with no file activity" ((Parse-Utc $hb2.timestamp) -gt (Parse-Utc $hb1.timestamp)) "$($hb1.timestamp) -> $($hb2.timestamp)"

    $outDup = Join-Path $TestRoot "dup.out"
    $pDup = Start-Watcher -Poll 120 -Quiet 3 -Hb 2 -OutFile $outDup
    $dupExited = $pDup.WaitForExit(15000)
    Check "T9: duplicate launch exits" $dupExited
    Check "T9: duplicate exit code is 1" ($dupExited -and $pDup.ExitCode -eq 1) "exit=$($pDup.ExitCode)"
    Check "T9: duplicate reports already running" ((Get-Content $outDup -Raw) -match "already running")
    Check "T9: original instance still running" (-not $pA.HasExited)

    $t1 = Join-Path $Inbox "20260912-100001"
    $t2 = Join-Path $Inbox "20260912-100002"
    Set-Content -Path ($t1 + ".tmp") -Value "burst one" -Encoding UTF8
    Set-Content -Path ($t2 + ".tmp") -Value "burst two" -Encoding UTF8
    Rename-Item -Path ($t1 + ".tmp") -NewName "20260912-100001.txt"
    Rename-Item -Path ($t2 + ".tmp") -NewName "20260912-100002.txt"
    $aExited = $pA.WaitForExit(30000)
    Check "T1: watcher fired after the burst" $aExited
    if ($aExited) {
        $rep = Read-Report $outA
        Check "T1: exit code 0" ($pA.ExitCode -eq 0) "exit=$($pA.ExitCode)"
        Check "T1: ONE batch contains both IDs" (($rep.unseen -contains "20260912-100001.txt") -and ($rep.unseen -contains "20260912-100002.txt") -and ($rep.unseen_count -eq 2)) ("unseen=" + ($rep.unseen -join ","))
        $hb3 = Read-Heartbeat
        Check "T1: heartbeat state is fired" ($hb3.state -eq "fired") $hb3.state
        Check "T1: settle line logged" ((Log-Text) -match "Event batch settled after 3s quiet \(new/changed work present\)")
    }

    # ---------------------------------------------------------------- B
    # T3 incomplete .tmp ignored, T4 touch-only no duplicate work,
    # T7 accelerated fallback still fires with FSW delivering nothing actionable
    Remove-Item $DeadlineFile -ErrorAction SilentlyContinue
    $outB = Join-Path $TestRoot "B.out"
    $pB = Start-Watcher -Poll 20 -Quiet 3 -Hb 2 -OutFile $outB
    Start-Sleep -Seconds 3
    Set-Content -Path (Join-Path $Inbox "20260912-100003.tmp") -Value "incomplete" -Encoding UTF8
    Start-Sleep -Seconds 2
    (Get-Item ($t1 + ".txt")).LastWriteTime = Get-Date   # mtime-only touch, bytes unchanged
    $bExited = $pB.WaitForExit(45000)
    Check "T7: fallback fires with no actionable events" $bExited
    if ($bExited) {
        $rep = Read-Report $outB
        $log = Log-Text
        Check "T7: exit code 0" ($pB.ExitCode -eq 0) "exit=$($pB.ExitCode)"
        Check "T7: did not fire early (uptime >= 15s)" ($rep.uptime_seconds -ge 15) "uptime=$($rep.uptime_seconds)"
        Check "T7: fallback line logged" ($log -match "Fallback reconciliation deadline reached")
        Check "T4: touch-only settle did not fire (noise line logged)" ($log -match "touch-only noise")
        Check "T3: .tmp never reported" (-not ($rep.unseen -contains "20260912-100003.tmp"))
        Check "T3: no FSW event for the .tmp" (-not ($log -match "100003"))
        Check "T4: unchanged backlog still listed, not treated as new" ($rep.unseen_count -eq 2)
    }

    # ---------------------------------------------------------------- C
    # T5 startup scan finds a request that arrived while down,
    # T6 interrupted claim (seen, no outbox) is reported, completed is separated
    Remove-Item $DeadlineFile -ErrorAction SilentlyContinue
    Set-Content -Path (Join-Path $Inbox "20260912-100004.txt") -Value "arrived while down" -Encoding UTF8
    New-Item -ItemType File -Path (Join-Path $Seen "20260912-100001.txt") | Out-Null
    New-Item -ItemType File -Path (Join-Path $Seen "20260912-100002.txt") | Out-Null
    Set-Content -Path (Join-Path $Outbox "20260912-100002.txt") -Value "reply" -Encoding UTF8
    $outC = Join-Path $TestRoot "C.out"
    $pC = Start-Watcher -Poll 60 -Quiet 3 -Hb 2 -OutFile $outC
    $cExited = $pC.WaitForExit(20000)
    Check "T5: startup scan fires for a request present at launch" $cExited
    if ($cExited) {
        $rep = Read-Report $outC
        Check "T5: new request reported" ($rep.unseen -contains "20260912-100004.txt") ("unseen=" + ($rep.unseen -join ","))
        Check "T6: interrupted claim reported (seen, no reply)" ($rep.claimed_unfinished -contains "20260912-100001.txt") ("claimed_unfinished=" + ($rep.claimed_unfinished -join ","))
        Check "T6: completed ticket reported separately" (($rep.completed -contains "20260912-100002.txt") -and -not ($rep.claimed_unfinished -contains "20260912-100002.txt"))
        Check "T5: startup-scan line logged" ((Log-Text) -match "Startup scan found new/changed work")
    }

    # ---------------------------------------------------------------- D
    # Restart with an unchanged backlog must not re-fire; the persisted
    # fallback deadline survives the restart with Kind=Utc intact.
    $persisted = (Get-Content $DeadlineFile -Raw).Trim()
    $outD = Join-Path $TestRoot "D.out"
    $pD = Start-Watcher -Poll 60 -Quiet 3 -Hb 2 -OutFile $outD
    Start-Sleep -Seconds 8
    Check "D: unchanged backlog does not re-fire on restart (8s)" (-not $pD.HasExited)
    $hbD = Read-Heartbeat
    Check "D: persisted fallback deadline reused after restart" ($hbD.fallback_due -eq $persisted) "hb=$($hbD.fallback_due) file=$persisted"
    Check "D: persisted deadline is in the future (no immediate fallback)" ((Parse-Utc $hbD.fallback_due) -gt [DateTime]::UtcNow)

    # ---------------------------------------------------------------- E
    # Force-stop (the documented wake procedure) then relaunch: the abandoned
    # mutex must be re-acquired, not reported as "already running".
    Stop-Process -Id $pD.Id -Force
    Start-Sleep -Seconds 1
    $outE = Join-Path $TestRoot "E.out"
    $pE = Start-Watcher -Poll 60 -Quiet 3 -Hb 2 -OutFile $outE
    Start-Sleep -Seconds 5
    Check "E: relaunch after force-stop is running (abandoned mutex handled)" (-not $pE.HasExited) ("exited=" + $pE.HasExited + " out=" + $(if (Test-Path $outE) { (Get-Content $outE -Raw) } else { "" }))
    $hbE = Read-Heartbeat
    Check "E: heartbeat pid is the relaunched instance" ($hbE.pid -eq $pE.Id) "hb=$($hbE.pid) proc=$($pE.Id)"
    Stop-Process -Id $pE.Id -Force
} finally {
    foreach ($p in $script:started) {
        try { if (-not $p.HasExited) { Stop-Process -Id $p.Id -Force } } catch {}
    }
    $fails = @($script:results | Where-Object { $_ -like "[FAIL]*" }).Count
    $total = $script:results.Count
    $summary = "watch_inbox selftest: $total checks, $fails failures (test root: $TestRoot)"
    $script:results + @("", $summary) | Set-Content -Path $ReportFile -Encoding UTF8
    Write-Host ""
    Write-Host $summary
    if ($fails -ne 0) { exit 1 } else { exit 0 }
}
