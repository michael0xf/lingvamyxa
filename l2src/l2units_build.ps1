# l2units_build.ps1 -- the L2 runtime units of the production runtime.
#
# Stage 3c-2a (L2_RUNTIME_PLAN_20260914.md): the production runtime is the L1
# modules plus every L2 unit under l2src whose first line is `profile:
# runtime` (today lmx_sched_record.lm2). Dot-source this file and call
# Build-L2RuntimeUnits once per run; it returns the object paths to append to
# the runtime link. Behaviour-neutral until exec.c reads a unit (3c-2b): the
# symbols are linked and unused.
#
# What it does, per run, into -Out:
#   1. builds l2trans.exe from l2src/l2trans.lm1 with the pinned l1trans
#      (the runner has already verified the pin), as run_sched_record.ps1;
#   2. for each unit: its .h.lm1 header (if present) to <IncludeDirs[0]>/
#      l2src/<stem>.lm1.h so the unit's own predef resolves; l2trans to a
#      generated lm1; two checks (a library unit, no escape poll: a
#      runtime-profile unit must not poll); l1trans to C; gcc -c.
#
# The function reads l2src/, l1src/ and lm1/build relative to the current
# location, so run it from a stg/l1_baseline directory. A runner that builds
# from a git-archive snapshot of the selected core (run_msg_send_local,
# run_msg_family_handoff) must:
#   - archive stg/l1_baseline/l1src (l2trans.lm1 predefs l1src/parser.lm1)
#     and stg/l1_baseline/lm1/build (gcc -I lm1/build finds p0.lm1.h), not
#     only stg/l1_baseline/l2src;
#   - call it under Push-Location to the snapshot's stg/l1_baseline;
#   - pass -I . in -CFlags, so the unit's l2src/ includes resolve there, and
#     the runner's generated headers dir first in -IncludeDirs;
#   - give each optimization level its own -Out (l2units_<level>), with
#     -<level> in -CFlags, since the objects are linked per level.
# run_sched_record is the exception: it builds the unit it tests and links
# that object itself, so it does not call this function (a second copy would
# be a duplicate definition in its link).
# Native calls go through cmd /c with a log, never through PowerShell's
# stderr (PS 5.1 under $ErrorActionPreference Stop treats stderr as failure).

function Invoke-L2Native([string]$Command, [string]$Log) {
    cmd /c "$Command > `"$Log`" 2>&1"
    if ($LASTEXITCODE -ne 0) {
        Get-Content -LiteralPath $Log
        throw "l2units: failed ($Log): $Command"
    }
}

function Build-L2RuntimeUnits([string]$L1Trans, [string]$Out, [string[]]$IncludeDirs, [string]$CFlags, [string]$Gcc = 'gcc', [string[]]$Exclude = @()) {
    # A runtime unit is an L2 unit with no L1 twin: lmx_sched_record.lm2 yes,
    # lmx_message.lm2 no (the parity mirror of lmx_message.lm1, measured by
    # run_port_message, never linked beside the L1 module it mirrors).
    # -Exclude names stems a runner builds itself (run_sched_record's unit under
    # test), so the link holds one copy of each unit.
    $units = @(Get-ChildItem -LiteralPath 'l2src' -File -Filter 'lmx_*.lm2' | Where-Object {
        (Get-Content -LiteralPath $_.FullName -TotalCount 1).Trim() -eq 'profile: runtime' -and
        -not (Test-Path -LiteralPath (Join-Path 'l2src' ([IO.Path]::GetFileNameWithoutExtension($_.Name) + '.lm1'))) -and
        ($Exclude -notcontains [IO.Path]::GetFileNameWithoutExtension($_.Name))
    } | Sort-Object Name)
    if ($units.Count -eq 0) { return @() }
    New-Item -ItemType Directory -Force -Path $Out | Out-Null
    $q = { param($s) '"' + $s + '"' }
    $incArgs = ($IncludeDirs | ForEach-Object { '-I ' + (& $q $_) }) -join ' '
    $l2c = Join-Path $Out 'l2trans.c'
    $l2exe = Join-Path $Out 'l2trans.exe'
    Invoke-L2Native ((& $q $L1Trans) + ' l2src/l2trans.lm1 ' + (& $q $l2c)) (Join-Path $Out 'l2trans.translate.log')
    Invoke-L2Native ("$Gcc $CFlags -I lm1/build " + (& $q $l2c) + ' l2src/lmx_poll_stub.c -o ' + (& $q $l2exe)) (Join-Path $Out 'l2trans.gcc.log')
    $objs = @()
    foreach ($unit in $units) {
        $stem = [IO.Path]::GetFileNameWithoutExtension($unit.Name)
        $header = Join-Path 'l2src' ($stem + '.h.lm1')
        if (Test-Path -LiteralPath $header) {
            $hdrDir = Join-Path $IncludeDirs[0] 'l2src'
            New-Item -ItemType Directory -Force -Path $hdrDir | Out-Null
            Invoke-L2Native ((& $q $L1Trans) + ' ' + (& $q $header) + ' ' + (& $q (Join-Path $hdrDir ($stem + '.lm1.h')))) (Join-Path $Out ($stem + '.header.log'))
        }
        $gen = Join-Path $Out ($stem + '.generated.lm1')
        Invoke-L2Native ((& $q $l2exe) + ' ' + (& $q $unit.FullName) + ' ' + (& $q $gen)) (Join-Path $Out ($stem + '.l2trans.log'))
        $genText = [IO.File]::ReadAllText((Resolve-Path -LiteralPath $gen).ProviderPath).Replace("`r`n", "`n")
        if ($genText -notmatch 'define: l2_program_entry l2_u[0-9A-F]{16}_entry') { throw "l2units: $stem is not a library unit" }
        if ($genText -match 'lmx_msg_poll_escape\(') { throw "l2units: the runtime-profile unit $stem emitted an escape poll" }
        $c = Join-Path $Out ($stem + '.generated.c')
        Invoke-L2Native ((& $q $L1Trans) + ' ' + (& $q $gen) + ' ' + (& $q $c)) (Join-Path $Out ($stem + '.translate.log'))
        $obj = Join-Path $Out ($stem + '.generated.o')
        Invoke-L2Native ("$Gcc $CFlags $incArgs -I lm1/build -c " + (& $q $c) + ' -o ' + (& $q $obj)) (Join-Path $Out ($stem + '.gcc.log'))
        $objs += $obj
    }
    return $objs
}
