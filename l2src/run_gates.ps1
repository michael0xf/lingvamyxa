# Core gate chain: the gates run for every core commit, in order, each as its
# own powershell -File child. A gate's verdict is its exit code; a gate that
# names a forbidden line also needs its own result line in the log and no
# forbidden line. The chain stops at the first red gate and prints one summary
# block.
#
# Run it on a clean tree after the commit: run_msg_send_local and
# run_msg_family_handoff archive the committed HEAD, the other gates read the
# working tree.
#
#   powershell -NoProfile -ExecutionPolicy Bypass -File l2src/run_gates.ps1
#   ... -L2MessageRoot                  also run the opt-in l2_message_root gate, last
#
# The default set (RUNNER_HAZARDS (e), 6f's decision): the ten core gates,
# then the entry turn, the 18 port parity
# runners and the graph ABI runner. run_l2trans, run_port_parser and
# run_l2_message_root stay outside the defaults.
#
# The L1 module selftest runners stay opt-in. No port parity runner duplicates
# its L1 runner exactly: a parity runner compiles the selftest with no -O level
# and -Werror on four warnings only, compares reference and generated output,
# runs nm on the generated object only, and pins none of the L1 runner's counts.
#   port_array_owned, port_array_ref_owned, port_chars_owned: not exact;
#     run_array_owned, run_array_ref_owned and run_chars_owned add -O2 -Werror,
#     nm imports and symbols of the module object, allocations equal releases.
#   port_msg_blocks: not exact; run_msg_blocks pins checks=143 frees=147
#     callbacks=2 at each requested level and checks the module's imports.
#   port_mail_chain: not exact; run_msg_mail_chain pins checks=26 at each level
#     and requires no imports.
#   port_path_storage: not exact; run_msg_path_storage builds O0 and O2, requires
#     exactly the realloc import and pins 541/543 checks with allocations=11.
#   port_slots: not exact; run_msg_slots builds O0 and O2, requires no imports,
#     pins checks=278. port_slots' reference build compiles the same selftest
#     against the L1 module, so it covers run_msg_slots' compile; run_msg_slots
#     stays opt-in (6f, after 6cb55982 broke both unnoticed).
#   port_storage: not exact; run_msg_storage builds O0 and O2, checks the
#     module imports, pins checks=77.
#   port_owned_ranges: not exact; run_owned_ranges builds O0 and O2, requires no
#     imports and no mutable data, pins checks=439.
#   port_history, port_roots_stale, port_visit, port_liveness: their L1 runners
#     (history, roots_stale, visit, liveness) are defaults already.
#   port_branch_owned, port_value_owned, port_graph_copy, port_merge_owned,
#     port_msg_graph_copy: their selftests (graph_abi, graph_copy, merge,
#     message_graph_copy) run inside graph_abi, a default.
#   run_foreign_alloc has no port parity runner.
param(
    [switch]$L2MessageRoot,
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

# Name, runner, arguments, the runner's own result line, and optionally a
# forbidden line: a gate with one is red on exit 0 when its own line is missing
# or the forbidden line appears.
$gates = @(
    # decision 18 lane oracle with the no-turn tripwire, S0
    @('lane_oracle', 'run_port_message.ps1', "-TranslatorPath `"$pinned`" -LaneCheck", 'lmx_message parity PASS', 'LANE WRITE FAIL'),
    @('scenario36', 'run_model_scenario36.ps1', '', 'core tests PASS'),
    @('lmx_message', 'run_lmx.ps1', '-Suite Message', 'selected=Message'),
    @('history', 'run_lmx_msg_history_owned.ps1', '', 'history checks='),
    @('roots_stale', 'run_lmx_msg_roots_stale.ps1', '', 'stale checks='),
    @('visit', 'run_msg_visit.ps1', '', 'visit checks='),
    @('liveness', 'run_msg_liveness.ps1', '', 'liveness checks='),
    @('send_local', 'run_msg_send_local.ps1', '', 'send local checks='),
    @('family_handoff', 'run_msg_family_handoff.ps1', '', 'family handoff checks='),
    @('c_scanners', 'run_candidate_c_scanners.ps1', '', 'candidate scanner parity cases='),
    @('entry_turn', 'run_entry_turn.ps1', '', 'entry turn PASS'),
    @('port_array_owned', 'run_port_array_owned.ps1', '', 'lmx_array_owned parity PASS'),
    @('port_array_ref_owned', 'run_port_array_ref_owned.ps1', '', 'lmx_array_ref_owned parity PASS'),
    @('port_branch_owned', 'run_port_branch_owned.ps1', '', 'lmx_branch_owned parity PASS'),
    @('port_chars_owned', 'run_port_chars_owned.ps1', '', 'lmx_chars_owned parity PASS'),
    @('port_graph_copy', 'run_port_graph_copy_owned.ps1', '', 'lmx_graph_copy_owned parity PASS'),
    @('port_merge_owned', 'run_port_merge_owned.ps1', '', 'lmx_merge_owned parity PASS'),
    @('port_msg_graph_copy', 'run_port_message_graph_copy.ps1', '', 'lmx_message_graph_copy parity PASS'),
    @('port_msg_blocks', 'run_port_msg_blocks.ps1', '', 'lmx_msg_blocks parity PASS'),
    @('port_history', 'run_port_msg_history_owned.ps1', '', 'lmx_msg_history_owned parity PASS'),
    @('port_liveness', 'run_port_msg_liveness.ps1', '', 'lmx_msg_liveness parity PASS'),
    @('port_mail_chain', 'run_port_msg_mail_chain.ps1', '', 'lmx_msg_mail_chain parity PASS'),
    @('port_path_storage', 'run_port_msg_path_storage.ps1', '', 'lmx_msg_path_storage parity PASS'),
    @('port_roots_stale', 'run_port_msg_roots_stale.ps1', '', 'lmx_msg_roots_stale parity PASS'),
    @('port_slots', 'run_port_msg_slots.ps1', '', 'lmx_msg_slots parity PASS'),
    @('port_storage', 'run_port_msg_storage.ps1', '', 'lmx_msg_storage parity PASS'),
    @('port_visit', 'run_port_msg_visit.ps1', '', 'lmx_msg_visit parity PASS'),
    @('port_owned_ranges', 'run_port_owned_ranges.ps1', '', 'lmx_owned_ranges parity PASS'),
    @('port_value_owned', 'run_port_value_owned.ps1', '', 'lmx_value_owned parity PASS'),
    @('graph_abi', 'run_graph_abi.ps1', '', 'graph ABI runner PASS')
)
if ($L2MessageRoot) { $gates += , @('l2_message_root', 'run_l2_message_root.ps1', '', 'Historical catalog audit PASS') }

# PowerShell's rendering of a thrown error around the runner's own text.
$decoration = '^\s*(At line:|At [A-Za-z]:\\|\+ |CategoryInfo|FullyQualifiedErrorId|~+\s*$)'
$rows = @()
$red = $false
$chainStarted = Get-Date
foreach ($g in $gates) {
    $name = $g[0]
    $log = Join-Path $LogDir "$name.log"
    if ($red) {
        $rows += '{0,-20} not run' -f $name
        continue
    }
    $started = Get-Date
    cmd /c "powershell -NoProfile -ExecutionPolicy Bypass -File l2src\$($g[1]) $($g[2]) > `"$log`" 2>&1"
    $code = $LASTEXITCODE
    $seconds = [int]((Get-Date) - $started).TotalSeconds
    $raw = @(Get-Content -LiteralPath $log)
    $lines = @($raw | Where-Object { $_.Trim() -and $_ -notmatch $decoration })
    $own = @($lines | Where-Object { $_ -match [regex]::Escape($g[3]) })
    $verdict = if ($own.Count) { $own[-1].Trim() } elseif ($lines.Count) { $lines[-1].Trim() } else { '(empty log)' }
    $evidenceLines = @($lines | Where-Object { $_ -match '(?i)\bevidence:?\s+\S' })
    $evidence = if ($evidenceLines.Count) { ([regex]::Match($evidenceLines[-1], '(?i)\bevidence:?\s+(.+?)\s*$')).Groups[1].Value } else { '-' }
    $state = if ($code -eq 0) { 'PASS' } else { "FAIL exit=$code" }
    if ($g.Count -gt 4 -and $g[4]) {
        # Counted over the raw log: a stderr line rendered as an error record
        # still carries the text.
        $forbidden = @($raw | Where-Object { $_ -match [regex]::Escape($g[4]) }).Count
        if ($code -eq 0 -and $own.Count -eq 0) {
            $state = "FAIL exit=0 no '$($g[3])' line"
            $code = 1
        } elseif ($code -eq 0 -and $forbidden -ne 0) {
            $state = "FAIL exit=0 '$($g[4])' x$forbidden"
            $code = 1
        }
    }
    if ($evidence -ne '-' -and $verdict.Contains($evidence)) {
        $rows += '{0,-20} {1} {2}s | {3} | log {4}' -f $name, $state, $seconds, $verdict, $log
    } else {
        $rows += '{0,-20} {1} {2}s | {3} | evidence {4} | log {5}' -f $name, $state, $seconds, $verdict, $evidence, $log
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
