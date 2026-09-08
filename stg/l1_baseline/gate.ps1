# Full gate for this build root, in one command.
#
# The order matters and is the reason this script exists: C bootstrap, then the
# gen0 seed, then the self-build to its fixed point, then every suite on both
# gen0 and gen2.
#
# The L2 suite is last and is deliberately just one more suite. l2src does not
# self-build - it is L2 written in L1, so self-build does not apply to it
# directly. Here it acts as a test inside the self-build of the working L1 tool,
# and a red l2 line says as much about L1 as it does about l2src.
#
# Run this before every commit of code. Commit and push together, never one
# without the other. Docs go straight to commit and push without waiting here.
#
# Two cmd traps this script has already fallen into, kept in mind here so the
# next edit does not reintroduce them: cmd /c does not inherit PowerShell's
# Set-Location, so every step cd's itself; and `set L1_GEN=gen0 && ...` puts
# "gen0 " with a trailing space into the variable, so the assignment is quoted.
#
# Every step is run through cmd with its own redirection. Redirecting a native
# command's stderr inside Windows PowerShell 5.1 wraps each line in an
# ErrorRecord and fails the step even when the exe returned zero, which is how
# the first version of this script managed to fail on gcc warnings.

$ErrorActionPreference = "Stop"
Set-Location $PSScriptRoot

$logDir = "build\l1trans\logs\gate"
New-Item -ItemType Directory -Force -Path $logDir | Out-Null
$failed = @()

function Step([string]$name, [string]$command) {
    $safe = ($name -replace '[^A-Za-z0-9]+', '_')
    $log = Join-Path $logDir "$safe.log"
    $t0 = Get-Date
    cmd /c "cd /d `"$PSScriptRoot`" && $command > `"$log`" 2>&1"
    $ec = $LASTEXITCODE
    $dt = [int]((Get-Date) - $t0).TotalSeconds
    $mark = if ($ec -eq 0) { "ok  " } else { "FAIL" }
    "{0} {1,-32} {2,4}s" -f $mark, $name, $dt
    if ($ec -ne 0) {
        $script:failed += $name
        Get-Content $log -Tail 12
    }
}

$ps = "powershell -NoProfile -ExecutionPolicy Bypass -File"
$suites = @("run_smoke","run_parser","run_expr","run_ifdef","run_define","run_scalar",
            "run_decl_repeat","run_ident","run_c_array","run_control","run_import_bare")

"gate: $PSScriptRoot"
Step "buildCore.lm0.bat" ".\buildCore.lm0.bat"
Step "run_seed"          "$ps tests\l1\run_seed.ps1"
Step "run_gen"           "$ps tests\l1\run_gen.ps1"

foreach ($gen in @("gen0","gen2")) {
    foreach ($s in $suites) {
        Step "$gen $s" "set `"L1_GEN=$gen`" && $ps tests\l1\$s.ps1"
    }
    Step "$gen l2 run_lmx" "set `"L1_GEN=$gen`" && $ps l2src\run_lmx.ps1"
}

if ($failed.Count -ne 0) {
    "gate: FAILED - " + ($failed -join ", ")
    exit 1
}
"gate: all green"
exit 0
