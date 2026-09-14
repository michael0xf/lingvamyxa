# Full gate for this build root, in one command.
#
# WHEN NOT TO RUN THIS. It REBUILDS the translator - buildCore, the gen0 seed,
# then gen1..gen3 - so it regenerates a tool other people are using. Run it only
# when something could affect L1 ITSELF.
#
# Working on mixa_manager is NOT such a case, and I ran it before every
# mixa_manager commit anyway, for weeks, out of a rule that was about a
# different situation. mixa_manager cannot affect the L1 self-build: it only
# needs a translator that already EXISTS, and run_mixa.ps1 merely READS the
# binary. So the gate cost four minutes each time and, worse, rewrote a shared
# tool underneath an integration in progress - which is exactly how I walked
# into Grok's uncommitted candidate on 2026-09-09.
#
# Rule: mixa_manager work is verified by run_mixa.ps1. This gate is for L1.
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

# SEED-LIMITED STEPS, decided 2026-09-13 together with the run_gen change.
#
# gen0 is built from lm2/l1trans.lm2 through the frozen old chain. It is a
# BOOTSTRAP artifact: nothing regenerates it, and it is 59 functions behind
# l1src/l1trans.lm1 (measured with l2src/tools_seed_drift.py). Running the
# current suites against it asks a frozen translator to accept the language as
# it is today, which it cannot and was never meant to.
#
# The three steps below fail on gen0 for exactly that reason, each with a
# recorded diagnostic:
#   gen0 run_decl_repeat  tests\l1\decl_repeat_ptr.lm1: unsupported statement
#                         atom -- the test was extended for arbitrary address
#                         depth, which the seed predates
#   gen0 run_ident        a quoted assignment target reaches C with its
#                         backticks -- the fix is in l1src, not in the seed
#   gen0 l2 run_lmx       l2src\lmx_msg_blocks.h.lm1: unsupported L1 form --
#                         a current runtime header the seed cannot parse.
#                         This one is red on origin/main as well.
#
# They are reported as SEED and do not fail the gate. Each one still RUNS, and
# each is required to pass on gen2, where it tests the translator that actually
# matters. If a step listed here ever passes on gen0 the gate says so, because
# that means the seed has caught up and the entry should be removed.
#
# This list is deliberately explicit and short. Adding to it is a decision, not
# a convenience: anything new here means the seed fell further behind, and that
# is worth an argument each time rather than a silent append.
$seedLimited = @("gen0 run_decl_repeat", "gen0 run_ident", "gen0 l2 run_lmx")

function Step([string]$name, [string]$command) {
    $safe = ($name -replace '[^A-Za-z0-9]+', '_')
    $log = Join-Path $logDir "$safe.log"
    $t0 = Get-Date
    cmd /c "cd /d `"$PSScriptRoot`" && $command > `"$log`" 2>&1"
    $ec = $LASTEXITCODE
    $dt = [int]((Get-Date) - $t0).TotalSeconds
    $seedOk = $seedLimited -contains $name
    $mark = if ($ec -eq 0) { "ok  " } elseif ($seedOk) { "SEED" } else { "FAIL" }
    "{0} {1,-32} {2,4}s" -f $mark, $name, $dt
    if ($ec -eq 0 -and $seedOk) {
        "     ^ this step passes on the seed now; remove it from `$seedLimited"
    }
    if ($ec -ne 0 -and -not $seedOk) {
        $script:failed += $name
        Get-Content $log -Tail 12
    }
    if ($ec -ne 0 -and $seedOk) {
        Get-Content $log -Tail 3
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
