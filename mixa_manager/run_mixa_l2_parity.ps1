# Parameterized L1-vs-L2 parity gate (runner-uniformity ticket,
# lingvamyxa-d6's step 2: "run_mixa_l2_parity.ps1 -Module <name> for
# the 51, data-driven"). Replaces the per-module run_mixa_<X>_l2_
# parity.ps1 scripts one verified batch at a time -- see
# l2_parity_module_table.ps1 for the schema, the batch-by-batch
# rollout plan, and which modules are deliberately excluded (app_main,
# app_controller, audio_mp3) and why.
#
# Same gate every old per-module script implemented, now driven by
# $ModuleTable[$Module] instead of copy-pasted per file:
#   0. (optional) ABI parity probe pair.
#   1. ALWAYS build + run the ORACLE-side harness.
#   2. Attempt the COMPLETE mixa_<Module>.lm2 translation.
#      - known barrier -> EXPECTED_CORE_BARRIER (exit 2). NOT a pass.
#      - other failure -> UNEXPECTED_FAILURE (exit 1).
#      - success -> build/link/run the L2-side harness, diff its trace
#        against the oracle's byte-for-byte (normalized when the
#        module's fixture roots differ between the two sides). PASS
#        (exit 0) only on an exact match, else PARITY_FAILURE (exit 1).
#
# The module's own real .lm1/.h.lm1 files are the parity oracle and
# are never touched. Nothing under stg/l1_baseline is modified, only
# read. Every input is built fresh in a unique run directory.
param(
    [Parameter(Mandatory=$true)][string]$Module
)
$ErrorActionPreference = "Stop"

$RepoRoot = Split-Path -Parent $PSScriptRoot
$L1Root = Join-Path $RepoRoot "stg\l1_baseline"
$L1Trans = Join-Path $L1Root "build\l1trans\gen2\l1trans.exe"
. (Join-Path $PSScriptRoot "lib_l2_runtime_support.ps1")
. (Join-Path $PSScriptRoot "l2_parity_module_table.ps1")

if (-not $ModuleTable.ContainsKey($Module)) {
    throw "unknown module '$Module' -- not in l2_parity_module_table.ps1 (app_main, app_controller and audio_mp3 are deliberately excluded, see that file's header comment)"
}
$Cfg = $ModuleTable[$Module]

$ExpectedL1Hash = Get-L1Pin -L1Root $L1Root
$ActualL1Hash = Assert-PinnedL1Translator -L1Trans $L1Trans -L1Root $L1Root

$guards = @(
    "-Werror=incompatible-pointer-types", "-Werror=discarded-qualifiers",
    "-Werror=implicit-function-declaration", "-Werror=implicit-int"
)
$GccStd = "-std=c99 -Wall -Wextra -Wpedantic $($guards -join ' ')"

$RunTimestamp = (Get-Date -Format "yyyyMMdd_HHmmss_fff")
$RunGuid = [GUID]::NewGuid().ToString().Substring(0, 8)
$BaseDirName = if ($Cfg.ShortBaseDir) { $Cfg.ShortBaseDir } else { "l2p_$Module" }
$BaseDir = Join-Path $RepoRoot "build\mixa\claude\$BaseDirName"
$RunDir = Join-Path $BaseDir "r${RunTimestamp}_${RunGuid}"
$HeaderDir = Join-Path $RunDir "headers\mixa_manager"
New-Item -ItemType Directory -Force -Path $HeaderDir | Out-Null

function Invoke-Cmd([string]$exe, [string]$argsStr, [string]$outLog, [string]$errLog) {
    & cmd /c "$exe $argsStr > `"$outLog`" 2> `"$errLog`""
    return $LASTEXITCODE
}
$InvokeCmdRef = { param($e, $a, $o, $er) Invoke-Cmd $e $a $o $er }

function Invoke-HeaderTrans([string]$SrcRel, [string]$OutName) {
    Push-Location $RepoRoot
    $out = Join-Path $HeaderDir $OutName
    $o1 = Join-Path $RunDir "hdr_${OutName}_stdout.log"
    $o2 = Join-Path $RunDir "hdr_${OutName}_stderr.log"
    $rc = Invoke-Cmd $L1Trans "$SrcRel `"$out`"" $o1 $o2
    Pop-Location
    if ($rc -ne 0) { Get-Content $o2; throw "$SrcRel header translation failed" }
}

# $Spec is either a plain stem string (source defaults to
# mixa_manager\<stem>.lm1) or a hashtable @{ Stem = "..."; Src = "mixa_
# manager\tests\...\<name>.lm1" } for a dependency whose real source
# lives outside the plain mixa_manager\mixa_<stem>.lm1 shape -- e.g.
# fm_copy/fm_remove's own test-owned link-glue units that predef two
# real bodies together into one translation unit because they can't be
# compiled separately (see each module's own comment for why).
function Build-RealDep($Spec) {
    if ($Spec -is [string]) {
        $Stem = $Spec
        $SrcRel = "mixa_manager\$Stem.lm1"
    } else {
        $Stem = $Spec.Stem
        $SrcRel = $Spec.Src
    }
    Push-Location $RepoRoot
    $depC = Join-Path $RunDir "$Stem.c"
    $dLog1 = Join-Path $RunDir "${Stem}_trans_stdout.log"
    $dLog2 = Join-Path $RunDir "${Stem}_trans_stderr.log"
    $dExit = Invoke-Cmd $L1Trans "$SrcRel `"$depC`"" $dLog1 $dLog2
    Pop-Location
    if ($dExit -ne 0) { Get-Content $dLog2; throw "real $Stem translation failed" }
    $depO = Join-Path $RunDir "$Stem.o"
    $dcLog1 = Join-Path $RunDir "${Stem}_compile_stdout.log"
    $dcLog2 = Join-Path $RunDir "${Stem}_compile_stderr.log"
    $dcExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" -c `"$depC`" -o `"$depO`"" $dcLog1 $dcLog2
    if ($dcExit -ne 0) { Get-Content $dcLog2; throw "real $Stem compile failed" }
    return $depO
}

function New-AudioWavFixture([string]$Dir) {
    # Same recipe as run_audio_native_selftest.ps1 / the old audio_win32
    # runner: 8000 Hz, 8-bit PCM mono, ~8s of near-silence (0x7F). Path
    # deliberately contains a space.
    $FixtureWav = Join-Path $Dir "My Sound 8s.wav"
    $SampleRate = 8000
    $Seconds = 8
    $DataLen = $SampleRate * $Seconds
    $Enc = [System.Text.Encoding]::ASCII
    $Ms = New-Object System.IO.MemoryStream
    $W = New-Object System.IO.BinaryWriter($Ms)
    $W.Write($Enc.GetBytes("RIFF"))
    $W.Write([int](36 + $DataLen))
    $W.Write($Enc.GetBytes("WAVE"))
    $W.Write($Enc.GetBytes("fmt "))
    $W.Write([int]16)
    $W.Write([int16]1)
    $W.Write([int16]1)
    $W.Write([int]$SampleRate)
    $W.Write([int]$SampleRate)
    $W.Write([int16]1)
    $W.Write([int16]8)
    $W.Write($Enc.GetBytes("data"))
    $W.Write([int]$DataLen)
    $Data = New-Object byte[] $DataLen
    for ($i = 0; $i -lt $DataLen; $i++) { $Data[$i] = [byte]0x7F }
    $W.Write($Data)
    $W.Flush()
    [System.IO.File]::WriteAllBytes($FixtureWav, $Ms.ToArray())
    if (-not (Test-Path -LiteralPath $FixtureWav)) {
        throw "Fixture WAV not created: $FixtureWav"
    }
}

# ---- Step 0: header translation. ----
foreach ($h in $Cfg.HeaderTrans) {
    Invoke-HeaderTrans $h.Src $h.Out
}

# ---- Step 0.5: ABI parity gate (optional). ----
if ($Cfg.Probe) {
    $probeLibs = if ($Cfg.Probe.Libs) { " $($Cfg.Probe.Libs)" } else { "" }
    $abiRealExe = Join-Path $RunDir "abi_probe_real.exe"
    $abiRealLog1 = Join-Path $RunDir "abi_probe_real_compile_stdout.log"
    $abiRealLog2 = Join-Path $RunDir "abi_probe_real_compile_stderr.log"
    $abiRealExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" `"$RepoRoot\mixa_manager\$($Cfg.Probe.RealC)`" -o `"$abiRealExe`"$probeLibs" $abiRealLog1 $abiRealLog2
    if ($abiRealExit -ne 0) { Get-Content $abiRealLog2; throw "ABI probe (real headers) compile failed" }

    $abiL2Exe = Join-Path $RunDir "abi_probe_l2.exe"
    $abiL2Log1 = Join-Path $RunDir "abi_probe_l2_compile_stdout.log"
    $abiL2Log2 = Join-Path $RunDir "abi_probe_l2_compile_stderr.log"
    $abiL2Exit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" `"$RepoRoot\mixa_manager\$($Cfg.Probe.L2C)`" -o `"$abiL2Exe`"$probeLibs" $abiL2Log1 $abiL2Log2
    if ($abiL2Exit -ne 0) { Get-Content $abiL2Log2; throw "ABI probe (L2 header) compile failed" }

    $abiRealOut = Join-Path $RunDir "abi_probe_real_run.log"
    $abiRealRunExit = Invoke-Cmd "`"$abiRealExe`"" "" $abiRealOut (Join-Path $RunDir "abi_probe_real_run_stderr.log")
    $abiL2Out = Join-Path $RunDir "abi_probe_l2_run.log"
    $abiL2RunExit = Invoke-Cmd "`"$abiL2Exe`"" "" $abiL2Out (Join-Path $RunDir "abi_probe_l2_run_stderr.log")
    if ($abiRealRunExit -ne 0 -or $abiL2RunExit -ne 0) { throw "ABI probe run failed (real exit $abiRealRunExit, L2 exit $abiL2RunExit)" }

    $AbiRealText = Get-Content -LiteralPath $abiRealOut -Raw
    $AbiL2Text = Get-Content -LiteralPath $abiL2Out -Raw
    if ($AbiRealText -ne $AbiL2Text) {
        $abiDiff = Compare-Object -ReferenceObject ($AbiRealText -split "`n") -DifferenceObject ($AbiL2Text -split "`n") | Out-String
        $AbiSummary = "ABI_MISMATCH: real-header probe output:`n$AbiRealText`nL2-header probe output:`n$AbiL2Text`nDiff:`n$abiDiff"
        Set-Content -LiteralPath (Join-Path $RunDir "run_summary.txt") -Value $AbiSummary
        $AbiSummary
        "Run directory: $RunDir"
        "ABI_MISMATCH: struct layout drift detected -- see the diff above. The harness was NOT built or run."
        exit 1
    }
}

# ---- Step 1: build l2trans.exe fresh. ----
$L2TransSourceRel = "l2src\l2trans.lm1"
$L2TransSource = Join-Path $L1Root $L2TransSourceRel
if (-not (Test-Path -LiteralPath $L2TransSource)) {
    throw "missing L2 frontend source (read-only, not modified by this script): $L2TransSource"
}
$L2TransSourceHash = (Get-FileHash -LiteralPath $L2TransSource -Algorithm SHA256).Hash

$l2exe = Join-Path $RunDir "l2trans.exe"
Push-Location $L1Root
try {
    $l2c = Join-Path $RunDir "l2trans.c"
    & $L1Trans $L2TransSourceRel $l2c
    if ($LASTEXITCODE -ne 0) { throw "l1trans failed translating l2trans.lm1 itself" }
    $gccBuildLog = Join-Path $RunDir "l2trans_build_stderr.log"
    $gccArgsStr = "$GccStd -I . -I lm1\build `"$l2c`" -o `"$l2exe`""
    $rc = Invoke-Cmd "gcc" $gccArgsStr (Join-Path $RunDir "l2trans_build_stdout.log") $gccBuildLog
    if ($rc -ne 0) { Get-Content $gccBuildLog; throw "gcc failed building l2trans.exe" }
} finally {
    Pop-Location
}
$L2TransExeHash = (Get-FileHash -LiteralPath $l2exe -Algorithm SHA256).Hash

# ---- Step 2: build the harness object (compiled ONCE). ----
Push-Location $RepoRoot
$harnessC = Join-Path $RunDir "harness.c"
$hLog1 = Join-Path $RunDir "harness_trans_stdout.log"
$hLog2 = Join-Path $RunDir "harness_trans_stderr.log"
$hExit = Invoke-Cmd $L1Trans "mixa_manager\tests\mixa_${Module}_parity_harness.lm1 `"$harnessC`"" $hLog1 $hLog2
Pop-Location
if ($hExit -ne 0) { Get-Content $hLog2; throw "harness translation failed" }

$harnessO = Join-Path $RunDir "harness.o"
$hcLog1 = Join-Path $RunDir "harness_compile_stdout.log"
$hcLog2 = Join-Path $RunDir "harness_compile_stderr.log"
$hcExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" -c `"$harnessC`" -o `"$harnessO`"" $hcLog1 $hcLog2
if ($hcExit -ne 0) { Get-Content $hcLog2; throw "harness compile failed" }

# ---- Step 2.5: build the union of real dependency objects (oracle side
# and L2 side may need different, possibly overlapping, subsets). ----
function Get-DepStemName($Spec) { if ($Spec -is [string]) { $Spec } else { $Spec.Stem } }

$DepObjs = @{}
foreach ($spec in ($Cfg.OracleDeps + $Cfg.L2Deps)) {
    $stemName = Get-DepStemName $spec
    if (-not $DepObjs.ContainsKey($stemName)) {
        $DepObjs[$stemName] = Build-RealDep $spec
    }
}
$OracleDepObjs = $Cfg.OracleDeps | ForEach-Object { $DepObjs[(Get-DepStemName $_)] }
$L2DepObjArr = $Cfg.L2Deps | ForEach-Object { $DepObjs[(Get-DepStemName $_)] }
$OracleDepArgs = ($OracleDepObjs | ForEach-Object { "`"$_`"" }) -join ' '
$L2DepArgs = ($L2DepObjArr | ForEach-Object { "`"$_`"" }) -join ' '

$LinkLibsStr = if ($Cfg.LinkLibs) { " $($Cfg.LinkLibs)" } else { "" }

# ---- Fixture setup (oracle side only for now; the L2 side's own
# fixture, when separate, is created later inside the success branch). ----
$OracleFixtureArg = ""
$FixtureKind = $Cfg.Fixture
if ($FixtureKind -eq 'shared-single') {
    $FixtureDir = Join-Path $RunDir "fixtures"
    New-Item -ItemType Directory -Force -Path $FixtureDir | Out-Null
    $OracleFixtureArg = $FixtureDir
} elseif ($FixtureKind -eq 'shared-separate' -or $FixtureKind -eq 'bespoke-audio-wav') {
    $oracleSub = if ($Cfg.OracleFixtureSubdir) { $Cfg.OracleFixtureSubdir } else { "oracle_fixtures" }
    $OracleFixtureDir = Join-Path $RunDir $oracleSub
    New-Item -ItemType Directory -Force -Path $OracleFixtureDir | Out-Null
    if ($FixtureKind -eq 'bespoke-audio-wav') { New-AudioWavFixture -Dir $OracleFixtureDir }
    $OracleFixtureArg = $OracleFixtureDir
}

# ---- Step 3: ALWAYS build + run the ORACLE-side harness. ----
$oracleC = Join-Path $RunDir "${Module}_oracle.c"
$ocLog1 = Join-Path $RunDir "oracle_trans_stdout.log"
$ocLog2 = Join-Path $RunDir "oracle_trans_stderr.log"
Push-Location $RepoRoot
$ocExit = Invoke-Cmd $L1Trans "mixa_manager\mixa_${Module}.lm1 `"$oracleC`"" $ocLog1 $ocLog2
Pop-Location
if ($ocExit -ne 0) { Get-Content $ocLog2; throw "oracle mixa_$Module.lm1 translation failed" }

$oracleO = Join-Path $RunDir "${Module}_oracle.o"
$occLog1 = Join-Path $RunDir "oracle_compile_stdout.log"
$occLog2 = Join-Path $RunDir "oracle_compile_stderr.log"
$ExtraCompileFlags = if ($Cfg.ExtraCompileFlags) { " $($Cfg.ExtraCompileFlags)" } else { "" }
$occExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`"$ExtraCompileFlags -c `"$oracleC`" -o `"$oracleO`"" $occLog1 $occLog2
if ($occExit -ne 0) { Get-Content $occLog2; throw "oracle mixa_$Module.lm1 compile failed" }

$oracleExe = Join-Path $RunDir "parity_oracle.exe"
$olLog1 = Join-Path $RunDir "oracle_link_stdout.log"
$olLog2 = Join-Path $RunDir "oracle_link_stderr.log"
$olArgsStr = "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" `"$harnessO`" `"$oracleO`""
if ($OracleDepArgs) { $olArgsStr += " $OracleDepArgs" }
$olArgsStr += " -o `"$oracleExe`"$LinkLibsStr"
$olExit = Invoke-Cmd "gcc" $olArgsStr $olLog1 $olLog2
if ($olExit -ne 0) { Get-Content $olLog2; throw "oracle harness link failed" }

$oracleRunOut = Join-Path $RunDir "oracle_run_stdout.log"
$oracleRunErr = Join-Path $RunDir "oracle_run_stderr.log"
Push-Location $RepoRoot
$oracleRunExit = Invoke-Cmd "`"$oracleExe`"" "`"$OracleFixtureArg`"" $oracleRunOut $oracleRunErr
Pop-Location
$OracleTrace = Get-Content -LiteralPath $oracleRunOut -Raw

# ---- Step 4: attempt the COMPLETE mixa_<Module>.lm2 translation. ----
# L2_RUNTIME_ROOT must be set before THIS call (the .lm2 -> .lm1
# translation, l2trans.exe itself), not just before the later .lm1 ->
# C step -- it's what makes l2trans spell its generated #include lines
# "stg/l1_baseline/l2src/..." instead of "l2src/...". Getting this
# wrong reproduces the exact original bug this ticket's shared helper
# exists to prevent (see lib_l2_runtime_support.ps1's own header
# comment) -- caught by lingvamyxa-d6 running event_fifo/cmdline/pump
# through this engine on integration, where the success branch is
# actually reachable; main never exercises this line.
#
# Set unconditionally, not just when Cfg.RuntimeTrio: app_path/app_
# fmpanel/help's own success path (a separate $L2L1Trans-vs-cwd shape
# in their old scripts, per lingvamyxa-d6) needs this too even though
# they never link the runtime object set -- d6 confirmed setting it
# before translation is an equally valid fix to modeling their own
# cwd separately, and it is a no-op for any module whose generated
# code never references an l2src/lmx_* import in the first place.
$env:L2_RUNTIME_ROOT = "stg/l1_baseline/l2src/"
Push-Location $RepoRoot
$modSrc = "mixa_manager\mixa_${Module}.lm2"
$modOut = Join-Path $RunDir "${Module}_l2.lm1"
$modStdout = Join-Path $RunDir "mod_stdout.log"
$modStderr = Join-Path $RunDir "mod_stderr.log"
$ModExit = Invoke-Cmd "`"$l2exe`"" "`"$modSrc`" `"$modOut`"" $modStdout $modStderr
Pop-Location

$ModStdoutText = if (Test-Path -LiteralPath $modStdout) { Get-Content -LiteralPath $modStdout -Raw } else { "" }
$ModStderrText = if (Test-Path -LiteralPath $modStderr) { Get-Content -LiteralPath $modStderr -Raw } else { "" }
$ModSourceHash = (Get-FileHash -LiteralPath (Join-Path $RepoRoot $modSrc) -Algorithm SHA256).Hash
$RunnerHash = (Get-FileHash -LiteralPath $PSCommandPath -Algorithm SHA256).Hash
$TableHash = (Get-FileHash -LiteralPath (Join-Path $PSScriptRoot "l2_parity_module_table.ps1") -Algorithm SHA256).Hash

$Verdict = $null
$ExitCode = 1
$L2TraceText = ""
$DiffText = ""

$KnownBarrier = ($ModStderrText -match "unknown foreign type") -or ($ModStderrText -match "incompatible entry signature")
if ($Cfg.BarrierSet -eq 'UIA') {
    $KnownBarrier = $KnownBarrier -or ($ModStderrText -match "unsupported own array declaration")
}
# audio_button's own finding (its old runner's own header comment): a
# module translated ALONE, with no main() anywhere in its file/predef
# closure, trips l2trans's "every file needs a main" requirement before
# any of its own code is even considered -- NOT the same claim as
# EXPECTED_CORE_BARRIER (says nothing about the module's own code) and
# checked first since it can co-occur with stderr text that would
# otherwise also match a known-barrier substring.
$NoMain = ($ModStderrText -match "missing main")

if ($ModExit -ne 0 -and $NoMain) {
    $Verdict = "NO_MAIN_IN_ISOLATION"
    $ExitCode = 3
} elseif ($ModExit -ne 0 -and $KnownBarrier) {
    $Verdict = "EXPECTED_CORE_BARRIER"
    $ExitCode = 2
} elseif ($ModExit -ne 0) {
    $Verdict = "UNEXPECTED_FAILURE"
    $ExitCode = 1
} else {
    Push-Location $RepoRoot
    $l2ModC = Join-Path $RunDir "${Module}_l2.c"
    $l2ccLog1 = Join-Path $RunDir "l2mod_trans_stdout.log"
    $l2ccLog2 = Join-Path $RunDir "l2mod_trans_stderr.log"
    if ($Cfg.RuntimeTrio) { $env:L2_RUNTIME_ROOT = "stg/l1_baseline/l2src/" }
    $l2ccExit = Invoke-Cmd $L1Trans "`"$modOut`" `"$l2ModC`"" $l2ccLog1 $l2ccLog2
    Pop-Location
    if ($l2ccExit -ne 0) {
        Get-Content $l2ccLog2
        $Verdict = "UNEXPECTED_FAILURE"
        $ExitCode = 1
    } else {
        $l2ModO = Join-Path $RunDir "${Module}_l2.o"
        $l2occLog1 = Join-Path $RunDir "l2mod_compile_stdout.log"
        $l2occLog2 = Join-Path $RunDir "l2mod_compile_stderr.log"
        if ($Cfg.RuntimeTrio) {
            $L2Rt = Add-L2RuntimeSupport -L1Trans $L1Trans -L1Root $L1Root -RunDir $RunDir -InvokeCmd $InvokeCmdRef
            $l2occExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" -I `"$($L2Rt.HeaderRoot)`" -I `"$($L2Rt.HeaderRoot)\stg\l1_baseline`" -I `"$L1Root`"$ExtraCompileFlags -c `"$l2ModC`" -o `"$l2ModO`"" $l2occLog1 $l2occLog2
        } else {
            $l2occExit = Invoke-Cmd "gcc" "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`"$ExtraCompileFlags -c `"$l2ModC`" -o `"$l2ModO`"" $l2occLog1 $l2occLog2
        }
        if ($l2occExit -ne 0) {
            Get-Content $l2occLog2
            $Verdict = "UNEXPECTED_FAILURE"
            $ExitCode = 1
        } else {
            $l2Exe = Join-Path $RunDir "parity_l2.exe"
            $l2olLog1 = Join-Path $RunDir "l2mod_link_stdout.log"
            $l2olLog2 = Join-Path $RunDir "l2mod_link_stderr.log"
            $l2olArgsStr = "$GccStd -I `"$RepoRoot`" -I `"$RunDir\headers`" `"$harnessO`" `"$l2ModO`""
            if ($L2DepArgs) { $l2olArgsStr += " $L2DepArgs" }
            if ($Cfg.RuntimeTrio) { $l2olArgsStr += " $($L2Rt.ObjList)" }
            $l2olArgsStr += " -o `"$l2Exe`"$LinkLibsStr"
            $l2olExit = Invoke-Cmd "gcc" $l2olArgsStr $l2olLog1 $l2olLog2
            if ($l2olExit -ne 0) {
                Get-Content $l2olLog2
                $Verdict = "UNEXPECTED_FAILURE"
                $ExitCode = 1
            } else {
                $L2FixtureArg = ""
                if ($FixtureKind -eq 'shared-single') {
                    $L2FixtureArg = $FixtureDir
                } elseif ($FixtureKind -eq 'shared-separate' -or $FixtureKind -eq 'bespoke-audio-wav') {
                    $l2Sub = if ($Cfg.L2FixtureSubdir) { $Cfg.L2FixtureSubdir } else { "l2_fixtures" }
                    $L2FixtureDir = Join-Path $RunDir $l2Sub
                    New-Item -ItemType Directory -Force -Path $L2FixtureDir | Out-Null
                    if ($FixtureKind -eq 'bespoke-audio-wav') { New-AudioWavFixture -Dir $L2FixtureDir }
                    $L2FixtureArg = $L2FixtureDir
                }
                $l2RunOut = Join-Path $RunDir "l2_run_stdout.log"
                $l2RunErr = Join-Path $RunDir "l2_run_stderr.log"
                Push-Location $RepoRoot
                $l2RunExit = Invoke-Cmd "`"$l2Exe`"" "`"$L2FixtureArg`"" $l2RunOut $l2RunErr
                Pop-Location
                $L2TraceText = Get-Content -LiteralPath $l2RunOut -Raw

                if ($FixtureKind -eq 'shared-separate' -or $FixtureKind -eq 'bespoke-audio-wav') {
                    $Norm = Get-NormalizedParityTraces -OracleTrace $OracleTrace -OracleRoot $OracleFixtureDir -L2Trace $L2TraceText -L2Root $L2FixtureDir
                    $CompareOracle = $Norm.Oracle
                    $CompareL2 = $Norm.L2
                } else {
                    $CompareOracle = $OracleTrace
                    $CompareL2 = $L2TraceText
                }

                if ($l2RunExit -eq 0 -and $oracleRunExit -eq 0 -and $CompareL2 -eq $CompareOracle) {
                    $Verdict = "PASS"
                    $ExitCode = 0
                } else {
                    $Verdict = "PARITY_FAILURE"
                    $DiffText = Compare-Object -ReferenceObject ($CompareOracle -split "`n") -DifferenceObject ($CompareL2 -split "`n") | Out-String
                    $ExitCode = 1
                }
            }
        }
    }
}

$Summary = @"
Module: $Module
Stable-L1-Translator: $L1Trans
Stable-L1-Translator-Sha256: $ActualL1Hash
L2trans-Source (informational, not pinned): $L2TransSource
L2trans-Source-Sha256 (informational, not pinned): $L2TransSourceHash
L2trans-Exe-Sha256 (rebuilt fresh this run, not a stable artifact): $L2TransExeHash
Module-L2-Source: $modSrc
Module-L2-Source-Sha256: $ModSourceHash
Harness-Source-Sha256: $((Get-FileHash -LiteralPath (Join-Path $RepoRoot "mixa_manager\tests\mixa_${Module}_parity_harness.lm1") -Algorithm SHA256).Hash)
Runner-Sha256: $RunnerHash
Module-Table-Sha256: $TableHash
Oracle-Run-Exit-Code: $oracleRunExit
Oracle-Trace:
$OracleTrace
Module-Translate-Exit-Code: $ModExit
Module-Translate-Stdout:
$ModStdoutText
Module-Translate-Stderr:
$ModStderrText
L2-Trace:
$L2TraceText
Diff (oracle vs L2, empty if identical):
$DiffText
Verdict: $Verdict
"@
Set-Content -LiteralPath (Join-Path $RunDir "run_summary.txt") -Value $Summary
$Summary
"Run directory: $RunDir"

switch ($Verdict) {
    "NO_MAIN_IN_ISOLATION" {
        "NO_MAIN_IN_ISOLATION: mixa_$Module.lm2 translated ALONE has no function named main anywhere in its file/predef closure, and l2trans requires one to exist before it will process a file at all. This is NOT the same as EXPECTED_CORE_BARRIER -- it says nothing about whether this module's own code is clean; it only reflects that testing an entry-point-less library fragment in isolation was never how l2trans expects to be invoked. The oracle-side harness DID run (see Oracle-Trace above)."
    }
    "EXPECTED_CORE_BARRIER" {
        "EXPECTED_CORE_BARRIER: full mixa_$Module.lm2 translation stopped at an already-known barrier. This is NOT a pass -- the integrated frontend is not yet on main. The oracle-side harness DID run (see Oracle-Trace above)."
    }
    "UNEXPECTED_FAILURE" {
        "UNEXPECTED_FAILURE: translation or build failed with something OTHER than the already-known barriers. Inspect the logs under $RunDir."
    }
    "PARITY_FAILURE" {
        "PARITY_FAILURE: both implementations built and ran, but their traces differ (see Diff above) or one exited non-zero."
    }
    "PASS" {
        "PASS: full mixa_$Module.lm2 translated, built, and ran; its trace is byte-identical to the L1 oracle's own trace."
    }
}

exit $ExitCode
