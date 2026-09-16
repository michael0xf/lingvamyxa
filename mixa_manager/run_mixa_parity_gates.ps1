# Gates the ten mixa L2 parity runners with a known, accepted state
# today, per MIXA_L2_PARITY_GATE.txt's own proposal (accepted by
# lingvamyxa-6f). app_controller and audio_mp3 stay OUTSIDE this table
# until each has its own accepted diagnosis -- not silently folded in.
#
# Pass means a runner's own ACTUAL verdict this run equals its own
# ExpectVerdict below -- not "PASS" unconditionally. backend_ctors_win32
# and backend_win32 are expected to STAY at EXPECTED_CORE_BARRIER;
# app_window is expected to STAY at PARITY_FAILURE (the l2trans
# int/pointer defect diagnosed in commit 2b4df6ad, lingvamyxa-6f's own
# ticket after S6-2 lands). Drifting to a DIFFERENT verdict, or the
# same verdict for a different reason, is a gate failure to report, not
# to silently absorb -- this whole thread's own "a further red is its
# own finding" rule.
#
# ExpectVerdict values below are dated 2026-09-15, set from that day's
# cold measurements (MIXA_L2_PARITY_GATE.txt). Any future change to an
# ExpectVerdict value must be its own dated comment on that row, never
# a silent edit -- so a reader can see when and why the accepted state
# moved.
#
# Runner invocation follows this tree's own established pitfall (this
# session's own memory, and every existing runner's own Invoke-Cmd
# function, copied verbatim below): PS 5.1 under
# $ErrorActionPreference="Stop" turns a native process's own stderr
# into a terminating NativeCommandError, so every child runner here is
# launched exactly the way every OTHER runner in this tree launches
# gcc/l1trans -- `& cmd /c "$exe $argsStr > out 2> err"; $LASTEXITCODE`
# -- run inside a background job so a per-runner timeout can be
# enforced (Wait-Job -Timeout) without changing that proven shape.
$ErrorActionPreference = "Stop"

$RepoRoot = Split-Path -Parent $PSScriptRoot
$RunTimestamp = (Get-Date -Format "yyyyMMdd_HHmmss_fff")
$RunGuid = [GUID]::NewGuid().ToString().Substring(0, 8)
$LogDir = Join-Path $RepoRoot "build\mixa\claude\parity_gates\run_${RunTimestamp}_${RunGuid}"
New-Item -ItemType Directory -Force -Path $LogDir | Out-Null

# script     = runner file, relative to mixa_manager
# args       = extra arguments (dispatcher-driven modules need -Module)
# expect     = the verdict this runner is accepted to produce today
# timeoutSec = per-runner wall-clock cap, at least 2x this runner's own
#              cold time as measured 2026-09-15 (MIXA_L2_PARITY_GATE.txt)
$Gates = @(
    @{ Script = "run_mixa_app_panel_l2_parity.ps1"; Args = @(); Expect = "PASS"; TimeoutSec = 75 }
    @{ Script = "run_mixa_app_win32_l2_parity.ps1"; Args = @(); Expect = "PASS"; TimeoutSec = 80 }
    @{ Script = "run_mixa_backend_ctors_win32_l2_parity.ps1"; Args = @(); Expect = "EXPECTED_CORE_BARRIER"; TimeoutSec = 30 }
    @{ Script = "run_mixa_backend_win32_l2_parity.ps1"; Args = @(); Expect = "EXPECTED_CORE_BARRIER"; TimeoutSec = 30 }
    @{ Script = "run_mixa_l2_parity.ps1"; Args = @("-Module", "app_window"); Expect = "PARITY_FAILURE"; TimeoutSec = 60 }
    @{ Script = "run_mixa_app_fmpanel_l2_parity.ps1"; Args = @(); Expect = "PASS"; TimeoutSec = 80 }
    @{ Script = "run_mixa_app_path_l2_parity.ps1"; Args = @(); Expect = "PASS"; TimeoutSec = 45 }
    @{ Script = "run_mixa_composite_glyphs_l2_parity.ps1"; Args = @(); Expect = "PASS"; TimeoutSec = 45 }
    @{ Script = "run_mixa_selection_l2_parity.ps1"; Args = @(); Expect = "PASS"; TimeoutSec = 45 }
    @{ Script = "run_mixa_tiles_l2_parity.ps1"; Args = @(); Expect = "PASS"; TimeoutSec = 45 }
    # 2026-09-16, commit 4014bec8: audio_mp3's scratch second-translator
    # mechanism retired (l1trans's old fixed import cap it worked around
    # is gone from the current pin); re-measured cold at PASS, 23.1s.
    @{ Script = "run_mixa_audio_mp3_l2_parity.ps1"; Args = @(); Expect = "PASS"; TimeoutSec = 60 }
)

function Get-Verdict([string]$LogPath) {
    if (-not (Test-Path -LiteralPath $LogPath)) { return "NO_OUTPUT" }
    $text = Get-Content -LiteralPath $LogPath -Raw
    $m = [regex]::Matches($text, '(?m)^Verdict:\s*(\S+)')
    if ($m.Count -eq 0) { return "NO_VERDICT_LINE" }
    return $m[$m.Count - 1].Groups[1].Value
}

function Invoke-GateRunner([string]$RepoRoot, [string]$ScriptRel, [string[]]$ExtraArgs, [int]$TimeoutSec, [string]$LogPath) {
    $job = Start-Job -ScriptBlock {
        param($RepoRoot, $ScriptRel, $ExtraArgs, $LogPath)
        Set-Location $RepoRoot
        $argsStr = "-NoProfile -File `"mixa_manager\$ScriptRel`""
        foreach ($a in $ExtraArgs) { $argsStr += " $a" }
        & cmd /c "powershell $argsStr > `"$LogPath`" 2>&1"
        return $LASTEXITCODE
    } -ArgumentList $RepoRoot, $ScriptRel, $ExtraArgs, $LogPath
    $done = Wait-Job -Job $job -Timeout $TimeoutSec
    if ($null -eq $done) {
        Stop-Job -Job $job -ErrorAction SilentlyContinue
        Remove-Job -Job $job -Force -ErrorAction SilentlyContinue
        return @{ TimedOut = $true; ExitCode = -1 }
    }
    $exitCode = Receive-Job -Job $job
    Remove-Job -Job $job -Force -ErrorAction SilentlyContinue
    return @{ TimedOut = $false; ExitCode = $exitCode }
}

$Rows = @()
foreach ($gate in $Gates) {
    $stem = [IO.Path]::GetFileNameWithoutExtension($gate.Script)
    $tag = if ($gate.Args.Count -gt 0) { "$stem`_$($gate.Args[1])" } else { $stem }
    $logPath = Join-Path $LogDir "$tag.log"
    $sw = [System.Diagnostics.Stopwatch]::StartNew()
    $result = Invoke-GateRunner -RepoRoot $RepoRoot -ScriptRel $gate.Script -ExtraArgs $gate.Args -TimeoutSec $gate.TimeoutSec -LogPath $logPath
    $sw.Stop()
    $elapsed = [math]::Round($sw.Elapsed.TotalSeconds, 1)
    $actual = if ($result.TimedOut) { "TIMEOUT" } else { Get-Verdict $logPath }
    $ok = ($actual -eq $gate.Expect)
    $displayName = if ($gate.Args.Count -gt 0) { "$($gate.Script) $($gate.Args -join ' ')" } else { $gate.Script }
    $Rows += [PSCustomObject]@{
        Runner   = $displayName
        Expect   = $gate.Expect
        Actual   = $actual
        Seconds  = $elapsed
        Ok       = $ok
        LogPath  = $logPath
    }
    if ($ok) {
        "OK    $displayName -- $actual (${elapsed}s)"
    } else {
        "MISMATCH $displayName -- expected $($gate.Expect), got $actual (${elapsed}s) -- log: $logPath"
    }
}

$okCount = ($Rows | Where-Object { $_.Ok }).Count
$total = $Rows.Count
$totalSeconds = [math]::Round((($Rows | Measure-Object -Property Seconds -Sum).Sum), 1)

""
"mixa parity gates: $okCount of $total as expected (total ${totalSeconds}s)"
"Log directory: $LogDir"

if ($okCount -eq $total) { exit 0 } else { exit 1 }
