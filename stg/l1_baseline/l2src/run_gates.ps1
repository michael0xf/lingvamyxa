# Core gate chain: the gates run for every core commit, in order, each as its
# own powershell -File child. A gate's verdict is its exit code. The chain
# stops at the first non-zero exit and prints one summary block.
#
# Run it on a clean tree after the commit: run_msg_sched_ready archives the
# committed HEAD, the other gates read the working tree.
#
#   powershell -NoProfile -ExecutionPolicy Bypass -File stg/l1_baseline/l2src/run_gates.ps1
#   ... -SchedRecordSource <copy.lm2>   run_sched_record against another source
#   ... -L2MessageRoot                  also run the opt-in l2_message_root gate, last
#   ... -FamilyRelease17                also run the decision 17 family release
#                                       chain test (red first until the orphan
#                                       step lands (scenario 4))
param(
    [string]$SchedRecordSource,
    [switch]$L2MessageRoot,
    [switch]$FamilyRelease17,
    [string]$LogDir
)
$ErrorActionPreference = 'Continue'
$baseline = Split-Path -Parent $PSScriptRoot
Set-Location $baseline
$pinned = Join-Path $baseline 'build\l1trans\gen2\l1trans.exe'
if (-not $LogDir) { $LogDir = Join-Path $baseline ('build\gates\' + (Get-Date -Format 'yyyyMMdd_HHmmss')) }
# cmd's redirect into a missing directory never starts the child.
New-Item -ItemType Directory -Force -Path $LogDir | Out-Null

$head = (git rev-parse HEAD) -join ''
$dirty = @(git status --porcelain -uno).Count
$pin = (Get-Content -LiteralPath (Join-Path $PSScriptRoot 'L1_PIN.txt') -TotalCount 1).Trim()
$pinHash = (Get-FileHash -LiteralPath $pinned).Hash
$header = "gates HEAD $head dirty=$dirty pin=$($pinHash.Substring(0, 8)) pin_matches_L1_PIN=$($pinHash -eq $pin) logs $LogDir"
Write-Output $header

# Name, runner, arguments, the runner's own result line.
$gates = @(
    @('port_message', 'run_port_message.ps1', "-TranslatorPath `"$pinned`"", 'lmx_message parity PASS'),
    @('scenario36', 'run_model_scenario36.ps1', '', 'core tests PASS'),
    @('sched_record', 'run_sched_record.ps1', $(if ($SchedRecordSource) { "-SourcePath `"$SchedRecordSource`"" } else { '' }), 'sched record'),
    @('lmx_message', 'run_lmx.ps1', '-Suite Message', 'selected=Message'),
    @('history', 'run_lmx_msg_history_owned.ps1', '', 'history checks='),
    @('roots_stale', 'run_lmx_msg_roots_stale.ps1', '', 'stale checks='),
    @('visit', 'run_msg_visit.ps1', '', 'visit checks='),
    @('liveness', 'run_msg_liveness.ps1', '', 'liveness checks='),
    @('sched_ready', 'run_msg_sched_ready.ps1', '-CoreCommit HEAD', 'sched_ready checks='),
    @('send_local', 'run_msg_send_local.ps1', '', 'send local checks='),
    @('family_handoff', 'run_msg_family_handoff.ps1', '', 'family handoff checks=')
)
if ($L2MessageRoot) { $gates += , @('l2_message_root', 'run_l2_message_root.ps1', '', 'Historical catalog audit PASS') }
if ($FamilyRelease17) { $gates += , @('family_release_17', 'run_model_scenario36.ps1', '-Tests lmx_model_family_release_17_selftest', 'family release 17') }

# PowerShell's rendering of a thrown error around the runner's own text.
$decoration = '^\s*(At line:|At [A-Za-z]:\\|\+ |CategoryInfo|FullyQualifiedErrorId|~+\s*$)'
$rows = @()
$red = $false
$chainStarted = Get-Date
foreach ($g in $gates) {
    $name = $g[0]
    $log = Join-Path $LogDir "$name.log"
    if ($red) {
        $rows += '{0,-17} not run' -f $name
        continue
    }
    $started = Get-Date
    cmd /c "powershell -NoProfile -ExecutionPolicy Bypass -File l2src\$($g[1]) $($g[2]) > `"$log`" 2>&1"
    $code = $LASTEXITCODE
    $seconds = [int]((Get-Date) - $started).TotalSeconds
    $lines = @(Get-Content -LiteralPath $log | Where-Object { $_.Trim() -and $_ -notmatch $decoration })
    $own = @($lines | Where-Object { $_ -match [regex]::Escape($g[3]) })
    $verdict = if ($own.Count) { $own[-1].Trim() } elseif ($lines.Count) { $lines[-1].Trim() } else { '(empty log)' }
    $evidenceLines = @($lines | Where-Object { $_ -match '(?i)\bevidence:?\s+\S' })
    $evidence = if ($evidenceLines.Count) { ([regex]::Match($evidenceLines[-1], '(?i)\bevidence:?\s+(.+?)\s*$')).Groups[1].Value } else { '-' }
    $state = if ($code -eq 0) { 'PASS' } else { "FAIL exit=$code" }
    if ($evidence -ne '-' -and $verdict.Contains($evidence)) {
        $rows += '{0,-17} {1} {2}s | {3} | log {4}' -f $name, $state, $seconds, $verdict, $log
    } else {
        $rows += '{0,-17} {1} {2}s | {3} | evidence {4} | log {5}' -f $name, $state, $seconds, $verdict, $evidence, $log
    }
    Write-Output $rows[-1]
    if ($code -ne 0) {
        $red = $true
        $stoppedAt = $name
    }
}
$chainSeconds = [int]((Get-Date) - $chainStarted).TotalSeconds

Write-Output ''
Write-Output '==== gate summary ===='
Write-Output $header
$rows | ForEach-Object { Write-Output $_ }
if ($red) {
    Write-Output "gates RED: stopped at $stoppedAt after ${chainSeconds}s"
    exit 1
}
Write-Output "gates GREEN: $($gates.Count) of $($gates.Count) in ${chainSeconds}s"
exit 0
