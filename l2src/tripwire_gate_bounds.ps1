# Tripwire for the two changes made on 2026-09-16 after the c_scanners gate held the
# chain for 28 minutes: the runner's whole-step bound, and run_gates requiring every
# row's own marker line. Each part is run twice -- once against the fixed file and once
# against the same file at 2d8f2b6a -- because a check that only ever passes proves
# nothing about what it catches. Part 1 and Part 2 are independent; -Part selects one.
#
# Part 1: the runner is killed by its own bound, with a NONZERO exit code.
#   fixed    + LMX_C_SCANNERS_BOUND_S=5  -> dead well inside the observation window
#   2d8f2b6a + LMX_C_SCANNERS_BOUND_S=5  -> still running when the window closes
# Part 2: a row whose runner exits 0 without printing its marker.
#   fixed run_gates    -> that row reads FAIL and the chain goes red
#   2d8f2b6a run_gates -> the same row reads PASS  (the hole this change closes)
# Part 3 (added with the per-row bounds): a row whose selftest never returns, bound 10 s.
#   fixed run_gates    -> red within bound + 30 s, naming that row as a timeout, and no
#                         process of the hung selftest left behind
#   d3bde8b5 run_gates, the last one without row bounds ($rowBase) -> still hung when that window closes
# 'both' is kept as the old name for running every part; 'all' says the same thing.
param([ValidateSet('1','2','3','both','all')][string]$Part='all',
      [int]$ObserveSeconds=40)
if ($Part -eq 'both') { $Part = 'all' }
$ErrorActionPreference='Stop'
$repo=Split-Path -Parent $PSScriptRoot
$base='2d8f2b6a'
$tmp=Join-Path $env:TEMP ('tripwire_gate_bounds_'+(Get-Date -Format 'yyyyMMdd_HHmmss'))
New-Item -ItemType Directory -Path $tmp -Force | Out-Null
$failures=@()
function Note($m){ Write-Output ("  "+$m) }

$rowBase='d3bde8b5'
# Shared by parts 2 and 3: a copy of a run_gates text with rows injected FIRST, and -- when the
# text has a per-row bound table -- a bound for each injected row, so the copy does not stop on
# "no row bound". A text from before the table (the controls) gets rows only.
function New-GatesCopy([string]$fromText,[string]$dest,[string[]]$rows,[string[]]$bounds){
    $lines=$fromText -split "`r?`n"
    $out=New-Object System.Collections.Generic.List[string]
    $done=$false
    foreach($l in $lines){
        $out.Add($l)
        if(-not $done -and $l -match '^\$gates = @\('){ foreach($r in $rows){ $out.Add($r) }; $done=$true }
        elseif($l -match '^\$rowBoundSeconds = @\{'){ foreach($b in $bounds){ $out.Add($b) } }
    }
    if(-not $done){ throw 'tripwire: did not find the $gates array to inject into' }
    Set-Content -LiteralPath $dest -Encoding utf8 -Value $out
}

if ($Part -eq '1' -or $Part -eq 'all') {
    Write-Output 'PART 1 -- the runner dies by its own bound, nonzero'
    # Launched the way run_gates:126 launches a gate, so the exit code travels the same
    # path (cmd /c -> powershell -File) that run_gates scores.
    function Invoke-Bounded([string]$scriptPath,[string]$label){
        $log=Join-Path $tmp ($label+'.log')
        $t0=Get-Date
        $env:LMX_C_SCANNERS_BOUND_S='5'
        try {
            $p=Start-Process -FilePath 'cmd' -NoNewWindow -PassThru -ArgumentList '/c',("powershell -NoProfile -ExecutionPolicy Bypass -File `"$scriptPath`" > `"$log`" 2>&1")
            $null=$p.Handle
            $exited=$p.WaitForExit($ObserveSeconds*1000)
            $wall=[int]((Get-Date)-$t0).TotalSeconds
            if(-not $exited){ cmd /c "taskkill /PID $($p.Id) /T /F >nul 2>&1" }
            [pscustomobject]@{Exited=$exited;Wall=$wall;Code=$(if($exited){$p.ExitCode}else{$null})}
        } finally { Remove-Item Env:LMX_C_SCANNERS_BOUND_S -ErrorAction SilentlyContinue }
    }
    $fixed=Invoke-Bounded (Join-Path $PSScriptRoot 'run_candidate_c_scanners.ps1') 'fixed'
    Note ("fixed:    exited={0} wall={1}s exit={2}" -f $fixed.Exited,$fixed.Wall,$fixed.Code)
    if(-not $fixed.Exited){ $failures+='the bound did not end the runner inside the window' }
    elseif($fixed.Code -eq 0){ $failures+="the runner ended with exit 0 -- run_gates would record PASS" }
    $ctlPath=Join-Path $tmp 'run_candidate_c_scanners.base.ps1'
    & git -C $repo show ("{0}:l2src/run_candidate_c_scanners.ps1" -f $base) | Set-Content -LiteralPath $ctlPath -Encoding utf8
    # The control must live beside L1_PIN.txt and run_l2trans.ps1: it resolves both
    # through $PSScriptRoot. Run it from l2src under a name the tree does not ship.
    $ctlInTree=Join-Path $PSScriptRoot 'tripwire_control_base_runner.ps1'
    Copy-Item -LiteralPath $ctlPath -Destination $ctlInTree -Force
    try {
        $ctl=Invoke-Bounded $ctlInTree 'control'
        Note ("control ({0}): exited={1} wall={2}s exit={3}" -f $base,$ctl.Exited,$ctl.Wall,$ctl.Code)
        if($ctl.Exited -and $ctl.Code -ne 0){
            $failures+='the control also ended nonzero inside the window: part 1 proves nothing about the bound'
        }
    } finally { Remove-Item -LiteralPath $ctlInTree -Force -ErrorAction SilentlyContinue }
}

if ($Part -eq '2' -or $Part -eq 'all') {
    Write-Output 'PART 2 -- a row that exits 0 without its marker'
    # A runner that exits 0 and never prints the marker: precisely the shape a hung or
    # silently-dead gate leaves behind.
    $silent=Join-Path $PSScriptRoot 'tripwire_silent_runner.ps1'
    Set-Content -LiteralPath $silent -Encoding utf8 -Value @(
        '# Written by tripwire_gate_bounds.ps1; exits 0 and never prints its marker.',
        'Write-Output "tripwire: this runner did some work and said nothing about it"',
        'exit 0')
    # A second injected row that always fails, so BOTH runs stop after two rows. Without
    # it the control is the expensive one: at 2d8f2b6a the silent row reads PASS, the
    # chain carries on through all 30 gates, and it reaches the very gate that hangs.
    $stopper=Join-Path $PSScriptRoot 'tripwire_stop_runner.ps1'
    Set-Content -LiteralPath $stopper -Encoding utf8 -Value @(
        '# Written by tripwire_gate_bounds.ps1; fails on purpose to end the chain.',
        'Write-Output "tripwire: stopping the chain after the measured row"',
        'exit 1')
    $injected = @(
        "    @('tripwire_marker', 'tripwire_silent_runner.ps1', '', 'MARKER THAT IS NEVER PRINTED'),",
        "    @('tripwire_stop', 'tripwire_stop_runner.ps1', '', 'tripwire: stopping the chain after the measured row'),")
    $injectedBounds = @('    tripwire_marker = 60; tripwire_stop = 60')
    function Read-Row([string]$gatesPath,[string]$label){
        $logDir=Join-Path $tmp ('gates_'+$label)
        $out=& cmd /c "powershell -NoProfile -ExecutionPolicy Bypass -File `"$gatesPath`" -LogDir `"$logDir`" 2>&1"
        $line=@($out | Where-Object { $_ -match '^tripwire_marker\s' })
        if(-not $line.Count){ throw ("tripwire: no tripwire_marker row in the {0} run" -f $label) }
        $line[-1]
    }
    # Injected as the FIRST row, so the chain stops there and neither run costs a gate.
    $fixedCopy=Join-Path $PSScriptRoot 'tripwire_gates_fixed.ps1'
    $baseCopy=Join-Path $PSScriptRoot 'tripwire_gates_base.ps1'
    try {
        New-GatesCopy (Get-Content -LiteralPath (Join-Path $PSScriptRoot 'run_gates.ps1') -Raw) $fixedCopy $injected $injectedBounds
        New-GatesCopy ((& git -C $repo show ("{0}:l2src/run_gates.ps1" -f $base)) -join "`n") $baseCopy $injected $injectedBounds
        $rFixed=Read-Row $fixedCopy 'fixed'
        $rBase=Read-Row $baseCopy 'base'
        Note ("fixed:            "+$rFixed.Trim())
        Note ("control ($base): "+$rBase.Trim())
        if($rFixed -notmatch 'FAIL'){ $failures+='the marker rule did not turn the silent row red' }
        if($rBase -match 'FAIL'){ $failures+="the silent row was already red at $base -- part 2 proves nothing about the rule" }
    } finally {
        foreach($f in @($silent,$stopper,$fixedCopy,$baseCopy)){ Remove-Item -LiteralPath $f -Force -ErrorAction SilentlyContinue }
    }
}

if ($Part -eq '3' -or $Part -eq 'all') {
    Write-Output 'PART 3 -- a row whose selftest never returns'
    $bound=10
    # A tag unique to this run, so a leftover of an earlier run cannot pass for this one.
    $tag='TRIPWIRE_ROW_HANG_'+(Get-Date -Format 'HHmmssfff')
    $hang=Join-Path $PSScriptRoot 'tripwire_hang_runner.ps1'
    # The runner does what family_handoff's did on the bad tree: it starts its selftest and
    # waits on it forever. The selftest is a separate process, one level below the runner, so
    # "killed down to the selftest" is a claim about a process the row did not start directly.
    Set-Content -LiteralPath $hang -Encoding utf8 -Value @(
        '# Written by tripwire_gate_bounds.ps1; its selftest never returns.',
        'Write-Output "tripwire: starting a selftest that never returns"',
        ('& powershell -NoProfile -Command "Start-Sleep -Seconds 900 # {0}"' -f $tag),
        'exit 0')
    $rows3=@("    @('tripwire_hang', 'tripwire_hang_runner.ps1', '', 'MARKER THAT IS NEVER PRINTED'),")
    $bounds3=@("    tripwire_hang = $bound")
    function Get-HangCount { @(Get-CimInstance Win32_Process | Where-Object { $_.CommandLine -match $tag -and $_.CommandLine -notmatch 'Get-CimInstance' }).Count }
    function Stop-Hang { Get-CimInstance Win32_Process | Where-Object { $_.CommandLine -match $tag -and $_.CommandLine -notmatch 'Get-CimInstance' } | ForEach-Object { Stop-Process -Id $_.ProcessId -Force -ErrorAction SilentlyContinue } }
    function Invoke-Chain([string]$gatesPath,[string]$label,[int]$waitSeconds){
        $out=Join-Path $tmp ("part3_$label.out")
        $t0=Get-Date
        $p=Start-Process -FilePath 'cmd' -NoNewWindow -PassThru -ArgumentList '/c',("powershell -NoProfile -ExecutionPolicy Bypass -File `"$gatesPath`" -LogDir `"$(Join-Path $tmp "gates3_$label")`" > `"$out`" 2>&1")
        $null=$p.Handle
        $exited=$p.WaitForExit($waitSeconds*1000)
        $wall=[int]((Get-Date)-$t0).TotalSeconds
        $alive=Get-HangCount
        if(-not $exited){ cmd /c "taskkill /PID $($p.Id) /T /F >nul 2>&1"; $null=$p.WaitForExit(10000) }
        $row=@(); if(Test-Path $out){ $row=@(Get-Content $out | Where-Object { $_ -match '^tripwire_hang\s' }) }
        [pscustomobject]@{Exited=$exited;Wall=$wall;Row=$(if($row.Count){$row[-1].Trim()}else{'(no tripwire_hang row)'});AliveAtWindowEnd=$alive}
    }
    $fixedCopy3=Join-Path $PSScriptRoot 'tripwire_gates_fixed3.ps1'
    $baseCopy3=Join-Path $PSScriptRoot 'tripwire_gates_base3.ps1'
    try {
        New-GatesCopy (Get-Content -LiteralPath (Join-Path $PSScriptRoot 'run_gates.ps1') -Raw) $fixedCopy3 $rows3 $bounds3
        New-GatesCopy ((& git -C $repo show ("{0}:l2src/run_gates.ps1" -f $rowBase)) -join "`n") $baseCopy3 $rows3 $bounds3
        $window=$bound+30
        $f3=Invoke-Chain $fixedCopy3 'fixed' $window
        Start-Sleep -Seconds 3
        $left=Get-HangCount
        Note ("fixed:             exited={0} wall={1}s leftover={2} :: {3}" -f $f3.Exited,$f3.Wall,$left,$f3.Row)
        if(-not $f3.Exited){ $failures+="the fixed chain was still running after bound + 30 s ($window s)" }
        if($f3.Row -notmatch 'FAIL timeout after \d+s \(row bound 10s\)'){ $failures+='the fixed chain did not name tripwire_hang as a timeout' }
        if($left -ne 0){ $failures+="$left process(es) of the hung selftest survived the row bound" }
        Stop-Hang
        $c3=Invoke-Chain $baseCopy3 'control' $window
        Note ("control ($rowBase): exited={0} wall={1}s selftest alive at window end={2} :: {3}" -f $c3.Exited,$c3.Wall,$c3.AliveAtWindowEnd,$c3.Row)
        if($c3.Exited){ $failures+="the control chain ended inside $window s too -- part 3 proves nothing about the row bound" }
        if($c3.AliveAtWindowEnd -lt 1){ $failures+='the control selftest was not running at window end -- part 3 proves nothing about the kill' }
    } finally {
        Stop-Hang
        foreach($f in @($hang,$fixedCopy3,$baseCopy3)){ Remove-Item -LiteralPath $f -Force -ErrorAction SilentlyContinue }
    }
}

Write-Output ''
if($failures.Count){
    foreach($f in $failures){ Write-Output ("TRIPWIRE FAIL: "+$f) }
    throw ("tripwire_gate_bounds: {0} failure(s); scratch {1}" -f $failures.Count,$tmp)
}
Write-Output ("tripwire_gate_bounds: PASS (part $Part); scratch $tmp")
