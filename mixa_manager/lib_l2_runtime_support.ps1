# The single-source-of-truth pinned L1 hash (lingvamyxa-d6's amendment,
# ticket following lingvamyxa-e2's review, 20260914-various): both the
# core lane (21 runners under stg/l1_baseline/l2src) and this lane's 52
# parity runners used to each paste the same 64-hex literal separately.
# Now both read stg/l1_baseline/l2src/L1_PIN.txt, one line, nothing
# else -- a pin promotion is a one-line change to that file instead of
# 73 separate edits. $L1Root is each runner's own already-resolved
# stg\l1_baseline path.
function Get-L1Pin {
    param(
        [Parameter(Mandatory=$true)][string]$L1Root
    )
    $pinPath = Join-Path $L1Root "l2src\L1_PIN.txt"
    if (-not (Test-Path -LiteralPath $pinPath)) {
        throw "missing L1 pin file: $pinPath"
    }
    $pin = (Get-Content -LiteralPath $pinPath -Raw).Trim()
    if ($pin -notmatch '^[0-9A-Fa-f]{64}$') {
        throw "L1 pin file $pinPath does not contain exactly 64 hex characters: '$pin'"
    }
    return $pin
}

# Verifies the stable L1 translator at $L1Trans exists and matches the
# pin from Get-L1Pin, throwing the same messages every runner's own
# inline check used to throw. Returns the actual hash (callers assign
# it to $ActualL1Hash, which several runners' own $Summary here-strings
# already interpolate).
function Assert-PinnedL1Translator {
    param(
        [Parameter(Mandatory=$true)][string]$L1Trans,
        [Parameter(Mandatory=$true)][string]$L1Root
    )
    $pin = Get-L1Pin -L1Root $L1Root
    if (-not (Test-Path -LiteralPath $L1Trans)) {
        throw "missing stable L1 translator: $L1Trans"
    }
    $actualHash = (Get-FileHash -LiteralPath $L1Trans -Algorithm SHA256).Hash
    if ($actualHash -ne $pin) {
        throw "stable L1 translator hash mismatch: expected $pin got $actualHash"
    }
    return $actualHash
}

# Shared L2 runtime-support helper for the clean-L2 parity runners
# (ticket 20260914-003000, Fable's own request: "put the three pieces
# where they are shared than paste them file by file"). Dot-source this
# file, then call Add-L2RuntimeSupport once the L2-side translation of
# a module has SUCCEEDED, right before compiling the generated .c.
#
# The three pieces, always applied together as one unit (the bug this
# file exists to prevent: several runners built after the first six had
# the header/object generation block copy-pasted WITHOUT the
# $env:L2_RUNTIME_ROOT line next to it -- silently spelling every
# runtime #include one path shape away from what the generated .c
# actually expects, "l2src/..." instead of "stg/l1_baseline/l2src/...",
# exactly the failure ticket 20260914-003000 reported for fm_copy):
#   1. $env:L2_RUNTIME_ROOT, set on the l2trans invocation that
#      generates the module's own translated .lm1 -- this is what makes
#      l2trans spell its generated #include lines with the
#      "stg/l1_baseline/l2src/..." shape in the first place, so it MUST
#      be set before that translation call, not just before compiling
#      afterward. Callers are responsible for setting it before their
#      own l2trans invocation; this file only documents that ordering
#      requirement, since the env var's effect is on translation, which
#      happens before this helper is ever called.
#   2. The generated L2 runtime headers (18 lmx_* header units,
#      translated from their real .h.lm1 sources under cwd=$L1Root, the
#      same cwd reasoning as the module headers themselves -- these
#      cross-import each other with paths relative to stg/l1_baseline).
#   3. The generated L2 runtime object set (the same 19 modules plus
#      the two plain, hand-written C files run_graph_abi.ps1 itself
#      builds as "message support" objects) -- the real graph/Message
#      runtime symbols (lmx_msg_create, lmx_branch_struct_known, etc.)
#      the generated L2 code actually calls.
#
# Returns a hashtable: @{ HeaderRoot = <path>; ObjList = <string> }.
# HeaderRoot is the -I path resolving the generated runtime headers
# (plus their own sibling-header and real-lmx.h resolution -- add BOTH
# "$HeaderRoot" and "$HeaderRoot\stg\l1_baseline" and "$L1Root" as -I
# flags on the module's own L2-side compile, exactly as the existing
# runners already do). ObjList is a ready-to-splice, already-quoted
# string of every runtime object path, for the final L2-side link line.
function Add-L2RuntimeSupport {
    param(
        [Parameter(Mandatory=$true)][string]$L1Trans,
        [Parameter(Mandatory=$true)][string]$L1Root,
        [Parameter(Mandatory=$true)][string]$RunDir,
        [Parameter(Mandatory=$true)][scriptblock]$InvokeCmd
    )

    $L2RuntimeHeaderRoot = Join-Path $RunDir "l2rt_headers"
    $L2RuntimeHeaderTree = Join-Path $L2RuntimeHeaderRoot "stg\l1_baseline\l2src"
    New-Item -ItemType Directory -Force -Path $L2RuntimeHeaderTree | Out-Null
    $L2RuntimeNames = @('lmx_array_owned','lmx_array_ref_owned','lmx_branch_owned','lmx_chars_owned','lmx_graph_copy_owned','lmx_message_graph_copy','lmx_msg_blocks','lmx_msg_history_owned','lmx_msg_liveness','lmx_msg_mail_chain','lmx_msg_path_storage','lmx_msg_roots_stale','lmx_msg_sched_ready','lmx_msg_slots','lmx_msg_storage','lmx_msg_visit','lmx_owned_ranges','lmx_value_owned')

    Push-Location $L1Root
    try {
        foreach ($rtName in $L2RuntimeNames) {
            $rtOut = Join-Path $L2RuntimeHeaderTree "$rtName.lm1.h"
            $rtLog1 = Join-Path $RunDir "l2rt_${rtName}_stdout.log"
            $rtLog2 = Join-Path $RunDir "l2rt_${rtName}_stderr.log"
            $rtExit = & $InvokeCmd $L1Trans "l2src\$rtName.h.lm1 `"$rtOut`"" $rtLog1 $rtLog2
            if ($rtExit -ne 0) { Get-Content $rtLog2; throw "L2 runtime header $rtName translation failed" }
        }
    } finally {
        Pop-Location
    }

    $L2RuntimeObjDir = Join-Path $RunDir "l2rt_objs"
    New-Item -ItemType Directory -Force -Path $L2RuntimeObjDir | Out-Null
    $L2RuntimeModuleNames = $L2RuntimeNames + @('lmx_message')
    $L2RuntimeObjs = @()

    Push-Location $L1Root
    try {
        foreach ($rtName in $L2RuntimeModuleNames) {
            $rtSrcC = Join-Path $L2RuntimeObjDir "$rtName.c"
            $rtTLog1 = Join-Path $RunDir "l2rtobj_${rtName}_trans_stdout.log"
            $rtTLog2 = Join-Path $RunDir "l2rtobj_${rtName}_trans_stderr.log"
            $rtTExit = & $InvokeCmd $L1Trans "l2src\$rtName.lm1 `"$rtSrcC`"" $rtTLog1 $rtTLog2
            if ($rtTExit -ne 0) { Pop-Location; Get-Content $rtTLog2; throw "L2 runtime module $rtName translation failed" }
            $rtObj = Join-Path $L2RuntimeObjDir "$rtName.o"
            $rtCLog1 = Join-Path $RunDir "l2rtobj_${rtName}_compile_stdout.log"
            $rtCLog2 = Join-Path $RunDir "l2rtobj_${rtName}_compile_stderr.log"
            $rtCArgs = "-std=c99 -Wall -Wextra -Wpedantic -Werror=incompatible-pointer-types -Werror=discarded-qualifiers -Werror=implicit-function-declaration -Werror=implicit-int -I `"$L2RuntimeHeaderRoot\stg\l1_baseline`" -I `"$L1Root`" -c `"$rtSrcC`" -o `"$rtObj`""
            $rtCExit = & $InvokeCmd "gcc" $rtCArgs $rtCLog1 $rtCLog2
            if ($rtCExit -ne 0) { Pop-Location; Get-Content $rtCLog2; throw "L2 runtime module $rtName compile failed" }
            $L2RuntimeObjs += $rtObj
        }
        foreach ($plainName in @('lmx_message_host', 'lmx_message_exec')) {
            $plainObj = Join-Path $L2RuntimeObjDir "$plainName.o"
            $plainCLog1 = Join-Path $RunDir "l2rtobj_${plainName}_compile_stdout.log"
            $plainCLog2 = Join-Path $RunDir "l2rtobj_${plainName}_compile_stderr.log"
            $plainCArgs = "-std=c99 -Wall -Wextra -Wpedantic -Werror=incompatible-pointer-types -Werror=discarded-qualifiers -Werror=implicit-function-declaration -Werror=implicit-int -I `"$L2RuntimeHeaderRoot\stg\l1_baseline`" -I `"$L1Root`" -c `"l2src\$plainName.c`" -o `"$plainObj`""
            $plainCExit = & $InvokeCmd "gcc" $plainCArgs $plainCLog1 $plainCLog2
            if ($plainCExit -ne 0) { Pop-Location; Get-Content $plainCLog2; throw "L2 runtime support $plainName compile failed" }
            $L2RuntimeObjs += $plainObj
        }
    } finally {
        Pop-Location
    }

    $L2RuntimeObjList = ($L2RuntimeObjs | ForEach-Object { '"' + $_ + '"' }) -join ' '
    return @{ HeaderRoot = $L2RuntimeHeaderRoot; ObjList = $L2RuntimeObjList }
}

# Shared fixture-root trace normalization (ticket 20260914-various,
# Fable's own request in the runner-uniformity ticket following ticket
# 20260913-235500 / 20260914-001000). The oracle and L2 runs get
# separate, independently populated fixture roots by design -- sharing
# one leaks state between the two runs (a marker file, a real invoke
# side effect, an entry a previous run already created) and invalidates
# whichever side reads it second. But that means any check that prints
# an absolute path will legitimately differ in ONLY that root between
# the two traces even when both sides behave identically (app_panel's
# own PARITY_FAILURE before ticket 20260914-001000's fix). This
# normalizes each trace's own known root to a fixed token before
# comparing, so a real path is still checked for real, root-
# agnostically, rather than either skipped or falsely flagged.
#
# Returns a hashtable: @{ Oracle = <normalized oracle trace>; L2 =
# <normalized L2 trace> }. Compare those two strings directly (and diff
# them on mismatch) instead of the raw traces.
function Get-NormalizedParityTraces {
    param(
        [Parameter(Mandatory=$true)][string]$OracleTrace,
        [Parameter(Mandatory=$true)][string]$OracleRoot,
        [Parameter(Mandatory=$true)][string]$L2Trace,
        [Parameter(Mandatory=$true)][string]$L2Root
    )
    $normOracle = $OracleTrace -replace [regex]::Escape($OracleRoot), "<FIXTURE_ROOT>"
    $normL2 = $L2Trace -replace [regex]::Escape($L2Root), "<FIXTURE_ROOT>"
    return @{ Oracle = $normOracle; L2 = $normL2 }
}
