# L2 units: Lmx core, registered-range classification, typed service pools.
# Build root is this file's parent, matching tests\l1\*.ps1 convention.
# Picks the L1 generation with L1_GEN (default gen2).
param(
    [ValidateSet('Full', 'Core', 'Message', 'MessageApi', 'Host', 'Exec', 'Cancel', 'Production')]
    [string]$Suite = 'Full',
    [switch]$Plan,
    [ValidateRange(1, 3600)][int]$TestTimeoutSeconds = 120
)
$ErrorActionPreference = "Stop"
$selected = [ordered]@{
    Core = $Suite -in @('Full', 'Core')
    MessageApi = $Suite -in @('Full', 'Message', 'MessageApi')
    Host = $Suite -in @('Full', 'Message', 'Host')
    Exec = $Suite -in @('Full', 'Message', 'Exec')
    Production = $Suite -in @('Full', 'Message', 'Production')
    Cancel = $Suite -in @('Full', 'Cancel')
}
if ($Plan) {
    [ordered]@{ suite = $Suite; checks = @($selected.Keys | Where-Object { $selected[$_] }); rebuildL1 = $false } | ConvertTo-Json
    return
}
Set-Location (Join-Path $PSScriptRoot "..")

$gen = "gen2"
if ($env:L1_GEN -and $env:L1_GEN.Trim().Length -gt 0) { $gen = $env:L1_GEN.Trim() }
$trans = "build\l1trans\$gen\l1trans.exe"
if (-not (Test-Path -LiteralPath $trans)) {
    throw "missing L1 translator: $trans (run tests\l1\run_gen.ps1 first)"
}

$out = "build\l2"
$log = "build\l1trans\logs\$gen"
if ($Suite -ne 'Full') {
    $out = Join-Path $out "targeted\$gen\$Suite"
    $log = Join-Path $log "lmx_$Suite"
}
New-Item -ItemType Directory -Force -Path $out, $log, (Join-Path $out "headers\l2src") | Out-Null
$suiteLog = Join-Path $log 'lmx_suite.log'
@("suite=$Suite", "status=RUNNING", "L1_GEN=$gen") | Set-Content -LiteralPath $suiteLog -Encoding utf8
try {
$needsMessage = $selected.MessageApi -or $selected.Host -or $selected.Exec -or $selected.Production -or $selected.Cancel
$nativeCwd = (Get-Location).Path
if ($Suite -ne 'Full') { $nativeCwd = Join-Path (Get-Location) (Join-Path $out 'native_work') }
if ($selected.Host -or $selected.Exec) {
    # Legacy native fixtures write relative gen2 evidence; isolate it as well.
    New-Item -ItemType Directory -Force -Path (Join-Path $nativeCwd 'build/l1trans/logs/gen2') | Out-Null
}
function Invoke-LmxTest([string]$Exe, [string]$Name, [string]$WorkingDirectory) {
    $stdout = Join-Path $log "$Name.stdout.txt"
    $stderr = Join-Path $log "$Name.stderr.txt"
    $proc = Start-Process -FilePath (Resolve-Path -LiteralPath $Exe).Path -WorkingDirectory $WorkingDirectory `
        -WindowStyle Hidden -PassThru -RedirectStandardOutput $stdout -RedirectStandardError $stderr
    # PS5.1 must retain the native handle before a quick process exits.
    $null = $proc.Handle
    if (-not $proc.WaitForExit($TestTimeoutSeconds * 1000)) {
        # Only the process this invocation started; no PID lookup or broad kill.
        $proc.Kill()
        $proc.WaitForExit()
        'TIMEOUT' | Set-Content -LiteralPath (Join-Path $log "$Name.exit.txt") -Encoding ascii
        throw "$gen $Name timed out after $TestTimeoutSeconds seconds; see $stdout / $stderr"
    }
    $proc.WaitForExit()
    $proc.Refresh()
    if ($null -eq $proc.ExitCode) { throw "$Name exited without an observable exit code" }
    $proc.ExitCode.ToString() | Set-Content -LiteralPath (Join-Path $log "$Name.exit.txt") -Encoding ascii
    if ($proc.ExitCode -ne 0) {
        Get-Content -LiteralPath $stdout, $stderr
        throw "$gen $Name failed exit=$($proc.ExitCode)"
    }
    return $proc.ExitCode
}
$blkHdr = Join-Path $out "headers\l2src\lmx_msg_blocks.lm1.h"
$blkC = Join-Path $out "lmx_msg_blocks.c"
$rngHdr = Join-Path $out "headers\l2src\lmx_owned_ranges.lm1.h"
$rngC = Join-Path $out "lmx_owned_ranges.c"
$stgHdr = Join-Path $out "headers\l2src\lmx_msg_storage.lm1.h"
$stgC = Join-Path $out "lmx_msg_storage.c"
$pathHdr = Join-Path $out "headers\l2src\lmx_msg_path_storage.lm1.h"
$pathC = Join-Path $out "lmx_msg_path_storage.c"
$slotsHdr = Join-Path $out "headers\l2src\lmx_msg_slots.lm1.h"
$slotsC = Join-Path $out "lmx_msg_slots.c"
$mailHdr = Join-Path $out "headers\l2src\lmx_msg_mail_chain.lm1.h"
$mailC = Join-Path $out "lmx_msg_mail_chain.c"
$schedHdr = Join-Path $out "headers\l2src\lmx_msg_sched_ready.lm1.h"
$schedC = Join-Path $out "lmx_msg_sched_ready.c"
$visitHdr = Join-Path $out "headers\l2src\lmx_msg_visit.lm1.h"
$visitC = Join-Path $out "lmx_msg_visit.c"
$liveHdr = Join-Path $out "headers\l2src\lmx_msg_liveness.lm1.h"
$liveC = Join-Path $out "lmx_msg_liveness.c"
$charsHdr = Join-Path $out "headers\l2src\lmx_chars_owned.lm1.h"
$charsC = Join-Path $out "lmx_chars_owned.c"
$arrHdr = Join-Path $out "headers\l2src\lmx_array_owned.lm1.h"
$arrC = Join-Path $out "lmx_array_owned.c"
$arrRefHdr = Join-Path $out "headers\l2src\lmx_array_ref_owned.lm1.h"
$arrRefC = Join-Path $out "lmx_array_ref_owned.c"
$brHdr = Join-Path $out "headers\l2src\lmx_branch_owned.lm1.h"
$brC = Join-Path $out "lmx_branch_owned.c"
$valHdr = Join-Path $out "headers\l2src\lmx_value_owned.lm1.h"
$valC = Join-Path $out "lmx_value_owned.c"
$histHdr = Join-Path $out "headers\l2src\lmx_msg_history_owned.lm1.h"
$histC = Join-Path $out "lmx_msg_history_owned.c"
$staleHdr = Join-Path $out "headers\l2src\lmx_msg_roots_stale.lm1.h"
$staleC = Join-Path $out "lmx_msg_roots_stale.c"
$copyHdr = Join-Path $out "headers\l2src\lmx_graph_copy_owned.lm1.h"
$copyC = Join-Path $out "lmx_graph_copy_owned.c"
$msgCopyHdr = Join-Path $out "headers\l2src\lmx_message_graph_copy.lm1.h"
$msgCopyC = Join-Path $out "lmx_message_graph_copy.c"
$blkInc = Join-Path $out "headers"
if ($needsMessage) {
& $trans "l2src\lmx_msg_blocks.h.lm1" $blkHdr
if ($LASTEXITCODE -ne 0) { throw "$gen translate failed: lmx_msg_blocks.h.lm1" }
& $trans "l2src\lmx_msg_blocks.lm1" $blkC
if ($LASTEXITCODE -ne 0) { throw "$gen translate failed: lmx_msg_blocks.lm1" }
& $trans "l2src\lmx_owned_ranges.h.lm1" $rngHdr
if ($LASTEXITCODE -ne 0) { throw "$gen translate failed: lmx_owned_ranges.h.lm1" }
& $trans "l2src\lmx_owned_ranges.lm1" $rngC
if ($LASTEXITCODE -ne 0) { throw "$gen translate failed: lmx_owned_ranges.lm1" }
& $trans "l2src\lmx_msg_storage.h.lm1" $stgHdr
if ($LASTEXITCODE -ne 0) { throw "$gen translate failed: lmx_msg_storage.h.lm1" }
& $trans "l2src\lmx_msg_storage.lm1" $stgC
if ($LASTEXITCODE -ne 0) { throw "$gen translate failed: lmx_msg_storage.lm1" }
& $trans "l2src\lmx_msg_path_storage.h.lm1" $pathHdr
if ($LASTEXITCODE -ne 0) { throw "$gen translate failed: lmx_msg_path_storage.h.lm1" }
& $trans "l2src\lmx_msg_path_storage.lm1" $pathC
if ($LASTEXITCODE -ne 0) { throw "$gen translate failed: lmx_msg_path_storage.lm1" }
& $trans "l2src\lmx_msg_slots.h.lm1" $slotsHdr
if ($LASTEXITCODE -ne 0) { throw "$gen translate failed: lmx_msg_slots.h.lm1" }
& $trans "l2src\lmx_msg_slots.lm1" $slotsC
if ($LASTEXITCODE -ne 0) { throw "$gen translate failed: lmx_msg_slots.lm1" }
& $trans "l2src\lmx_msg_mail_chain.h.lm1" $mailHdr
if ($LASTEXITCODE -ne 0) { throw "$gen translate failed: lmx_msg_mail_chain.h.lm1" }
& $trans "l2src\lmx_msg_mail_chain.lm1" $mailC
if ($LASTEXITCODE -ne 0) { throw "$gen translate failed: lmx_msg_mail_chain.lm1" }
& $trans "l2src\lmx_msg_sched_ready.h.lm1" $schedHdr
if ($LASTEXITCODE -ne 0) { throw "$gen translate failed: lmx_msg_sched_ready.h.lm1" }
& $trans "l2src\lmx_msg_sched_ready.lm1" $schedC
if ($LASTEXITCODE -ne 0) { throw "$gen translate failed: lmx_msg_sched_ready.lm1" }
& $trans "l2src\lmx_msg_visit.h.lm1" $visitHdr
if ($LASTEXITCODE -ne 0) { throw "$gen translate failed: lmx_msg_visit.h.lm1" }
& $trans "l2src\lmx_msg_visit.lm1" $visitC
if ($LASTEXITCODE -ne 0) { throw "$gen translate failed: lmx_msg_visit.lm1" }
& $trans "l2src\lmx_msg_liveness.h.lm1" $liveHdr
if ($LASTEXITCODE -ne 0) { throw "$gen translate failed: lmx_msg_liveness.h.lm1" }
& $trans "l2src\lmx_msg_liveness.lm1" $liveC
if ($LASTEXITCODE -ne 0) { throw "$gen translate failed: lmx_msg_liveness.lm1" }
& $trans "l2src\lmx_chars_owned.h.lm1" $charsHdr
if ($LASTEXITCODE -ne 0) { throw "$gen translate failed: lmx_chars_owned.h.lm1" }
& $trans "l2src\lmx_chars_owned.lm1" $charsC
if ($LASTEXITCODE -ne 0) { throw "$gen translate failed: lmx_chars_owned.lm1" }
& $trans "l2src\lmx_array_owned.h.lm1" $arrHdr
if ($LASTEXITCODE -ne 0) { throw "$gen translate failed: lmx_array_owned.h.lm1" }
& $trans "l2src\lmx_array_owned.lm1" $arrC
if ($LASTEXITCODE -ne 0) { throw "$gen translate failed: lmx_array_owned.lm1" }
& $trans "l2src\lmx_array_ref_owned.h.lm1" $arrRefHdr
if ($LASTEXITCODE -ne 0) { throw "$gen translate failed: lmx_array_ref_owned.h.lm1" }
& $trans "l2src\lmx_array_ref_owned.lm1" $arrRefC
if ($LASTEXITCODE -ne 0) { throw "$gen translate failed: lmx_array_ref_owned.lm1" }
& $trans "l2src\lmx_branch_owned.h.lm1" $brHdr
if ($LASTEXITCODE -ne 0) { throw "$gen translate failed: lmx_branch_owned.h.lm1" }
& $trans "l2src\lmx_branch_owned.lm1" $brC
if ($LASTEXITCODE -ne 0) { throw "$gen translate failed: lmx_branch_owned.lm1" }
& $trans "l2src\lmx_value_owned.h.lm1" $valHdr
if ($LASTEXITCODE -ne 0) { throw "$gen translate failed: lmx_value_owned.h.lm1" }
& $trans "l2src\lmx_value_owned.lm1" $valC
if ($LASTEXITCODE -ne 0) { throw "$gen translate failed: lmx_value_owned.lm1" }
& $trans "l2src\lmx_msg_history_owned.h.lm1" $histHdr
if ($LASTEXITCODE -ne 0) { throw "$gen translate failed: lmx_msg_history_owned.h.lm1" }
& $trans "l2src\lmx_msg_history_owned.lm1" $histC
if ($LASTEXITCODE -ne 0) { throw "$gen translate failed: lmx_msg_history_owned.lm1" }
& $trans "l2src\lmx_msg_roots_stale.h.lm1" $staleHdr
if ($LASTEXITCODE -ne 0) { throw "$gen translate failed: lmx_msg_roots_stale.h.lm1" }
& $trans "l2src\lmx_msg_roots_stale.lm1" $staleC
if ($LASTEXITCODE -ne 0) { throw "$gen translate failed: lmx_msg_roots_stale.lm1" }
& $trans "l2src\lmx_graph_copy_owned.h.lm1" $copyHdr
if ($LASTEXITCODE -ne 0) { throw "$gen translate failed: lmx_graph_copy_owned.h.lm1" }
& $trans "l2src\lmx_graph_copy_owned.lm1" $copyC
if ($LASTEXITCODE -ne 0) { throw "$gen translate failed: lmx_graph_copy_owned.lm1" }
& $trans "l2src\lmx_message_graph_copy.h.lm1" $msgCopyHdr
if ($LASTEXITCODE -ne 0) { throw "$gen translate failed: lmx_message_graph_copy.h.lm1" }
& $trans "l2src\lmx_message_graph_copy.lm1" $msgCopyC
if ($LASTEXITCODE -ne 0) { throw "$gen translate failed: lmx_message_graph_copy.lm1" }
}

$guards = @(
    "-Werror=incompatible-pointer-types", "-Werror=discarded-qualifiers",
    "-Werror=implicit-function-declaration", "-Werror=implicit-int"
)

# Invocation-local cache only: no prior object is accepted from disk.
# Local headers are part of the key, so matching flags alone never suffice.
$objectCache = @{}
$objectEvidence = [System.Collections.Generic.List[object]]::new()
$gccPath = (Get-Command gcc -ErrorAction Stop).Source
$gccHash = (Get-FileHash -LiteralPath $gccPath).Hash
function Get-LmxObject([string]$Source, [string[]]$Defines = @()) {
    $options = @('-std=c99', '-Wall', '-Wextra', '-Wpedantic') + $guards + @('-I', '.', '-I', 'lm1/build', '-I', $blkInc) + $Defines
    $headerHashes = @()
    foreach ($dir in @('l2src', 'lm1/build', $blkInc)) {
        if (Test-Path -LiteralPath $dir) {
            $headerHashes += @(Get-ChildItem -LiteralPath $dir -Recurse -File -Filter '*.h' | Sort-Object FullName | ForEach-Object {
                $_.FullName + ':' + (Get-FileHash -LiteralPath $_.FullName).Hash
            })
        }
    }
    $identity = [ordered]@{
        source = (Resolve-Path -LiteralPath $Source).Path
        sha256 = (Get-FileHash -LiteralPath $Source).Hash
        compiler = $gccPath
        compilerSHA256 = $gccHash
        options = $options
        headers = $headerHashes
    }
    $sha = [Security.Cryptography.SHA256]::Create()
    try { $key = ([BitConverter]::ToString($sha.ComputeHash([Text.Encoding]::UTF8.GetBytes(($identity | ConvertTo-Json -Depth 5 -Compress))))).Replace('-', '').ToLowerInvariant() }
    finally { $sha.Dispose() }
    if ($objectCache.ContainsKey($key)) {
        $objectEvidence.Add([ordered]@{ key = $key; source = $Source; reused = $true; object = $objectCache[$key] })
        return $objectCache[$key]
    }
    $obj = Join-Path $out (([IO.Path]::GetFileNameWithoutExtension($Source)) + '_' + $key + '.o')
    $compileLog = Join-Path $log (([IO.Path]::GetFileNameWithoutExtension($Source)) + '_' + $key + '.gcc.log')
    $arguments = $options + @('-c', $Source, '-o', $obj)
    $quoted = ($arguments | ForEach-Object { '"' + $_ + '"' }) -join ' '
    cmd /c "`"$gccPath`" $quoted > `"$compileLog`" 2>&1"
    if ($LASTEXITCODE -ne 0) {
        Get-Content -LiteralPath $compileLog
        throw "$gen gcc failed: $Source"
    }
    $objectCache[$key] = $obj
    $objectEvidence.Add([ordered]@{ key = $key; source = $Source; reused = $false; object = $obj; identity = $identity })
    return $obj
}
function Get-LmxSupportObjects([string[]]$Defines = @(), [string[]]$HistoryDefines = $null) {
    foreach ($source in @('l2src/lmx_message_host.c', 'l2src/lmx_message_exec.c', $blkC, $rngC, $stgC, $pathC, $slotsC, $mailC, $schedC, $visitC, $liveC, $charsC, $arrC, $arrRefC, $brC, $valC, $copyC, $msgCopyC)) {
        Get-LmxObject $source $Defines
    }
    $histDefs = $Defines
    if ($null -ne $HistoryDefines) { $histDefs = $HistoryDefines }
    Get-LmxObject $histC $histDefs
    Get-LmxObject $staleC $Defines
}
$units = @()
if ($selected.Core) { $units += @('lmx_selftest', 'lmx_pool_selftest', 'lmx_chars_selftest', 'lmx_ref_selftest', 'lmx_branch_selftest', 'lmx_own_selftest', 'lmx_size_selftest', 'lmx_dec_selftest') }
if ($selected.MessageApi) { $units += 'lmx_message_selftest' }
foreach ($unit in $units) {
    $src = "l2src\$unit.lm1"
    $c = Join-Path $out "$unit.c"
    $exe = Join-Path $out "$unit.exe"

    & $trans $src $c
    if ($LASTEXITCODE -ne 0) { throw "$gen translate failed: $src" }

    $extra = @()
    $defs = @()
    $opt = @()
    $inc = @("-I", ".", "-I", "lm1/build")
    if ($unit -eq "lmx_message_selftest") {
        $defs = @("-DLMX_MSG_HOST_TEST")
        $extra = @(Get-LmxSupportObjects $defs)
        $inc += @("-I", $blkInc)
    }
    if ($unit -eq "lmx_dec_selftest") {
        $decSrc = Join-Path (Get-Location) "..\..\third_party\decNumber\decNumber-icu-368"
        $defs = @("-DDECNUMDIGITS=34")
        $opt = @("-O2")
        $inc += @("-I", $decSrc)
        $extra = @(
            (Join-Path $decSrc "decNumber.c"),
            (Join-Path $decSrc "decContext.c"),
            "l2src\lmx_dec.c"
        )
    }
    $glog = Join-Path $log "$unit.gcc.log"
    $incStr = ($inc -join " ")
    $defStr = ($defs -join " ")
    $gstr = ($guards -join " ")
    $extraStr = ($extra | ForEach-Object { '"' + $_ + '"' }) -join " "
    $optStr = ($opt -join " ")
    cmd /c "gcc -std=c99 -Wall -Wextra -Wpedantic $gstr $optStr $incStr $defStr `"$c`" $extraStr -o `"$exe`" > `"$glog`" 2>&1"
    if ($LASTEXITCODE -ne 0) {
        Get-Content $glog
        throw "$gen gcc failed: $c"
    }

    if ($unit -eq "lmx_dec_selftest" -or $unit -eq "lmx_message_selftest") {
        $capOut = Join-Path $log "$unit.stdout.txt"
        $capErr = Join-Path $log "$unit.stderr.txt"
        $p = Start-Process -FilePath (Join-Path (Get-Location) $exe) -WorkingDirectory (Get-Location) -Wait -PassThru -NoNewWindow -RedirectStandardOutput $capOut -RedirectStandardError $capErr
        $p.ExitCode.ToString() | Set-Content -LiteralPath (Join-Path $log "$unit.exit.txt") -Encoding ascii
        if ($p.ExitCode -ne 0) {
            Get-Content -LiteralPath $capOut -ErrorAction SilentlyContinue
            Get-Content -LiteralPath $capErr -ErrorAction SilentlyContinue
            throw "$gen $unit failed exit=$($p.ExitCode)"
        }
    } else {
        & $exe
        if ($LASTEXITCODE -ne 0) { throw "$gen $unit failed" }
    }
}
$msgC = Join-Path $out "lmx_message.c"
if ($selected.Host -or $selected.Exec -or $selected.Cancel) {
& $trans "l2src\lmx_message.lm1" $msgC
if ($LASTEXITCODE -ne 0) { throw "$gen translate failed: l2src\lmx_message.lm1" }
}
$gstr = ($guards -join " ")
if ($selected.Production) {
$prodHostO = Get-LmxObject 'l2src/lmx_message_host.c'
$prodNm = & nm --defined-only $prodHostO 2>&1 | Out-String
if ($LASTEXITCODE -ne 0) { throw "nm failed on production host.o" }
if ($prodNm -match 'lmx_msg_host_test_set_') { throw "production host.o exports test setters" }
if ($prodNm -cmatch '(?m)\s[A-Z]\s+lmx_msg_host_test_nomem\s*$') { throw "production host.o has mutable test_nomem" }
$prodExecO = Get-LmxObject 'l2src/lmx_message_exec.c'
$prodExecNm = & nm --defined-only $prodExecO 2>&1 | Out-String
if ($LASTEXITCODE -ne 0) { throw "nm failed on production exec.o" }
if ($prodExecNm -cmatch '(?m)\s[A-Z]\s+lmx_msg_exec_test_after_cleanup\s*$') { throw "production exec.o exports test cleanup hook" }
if ($prodExecNm -cmatch '(?m)\s[A-Z]\s+lmx_msg_exec_test_set_fail_grow\s*$') { throw "production exec.o exports test fail_grow setter" }
if ($prodExecNm -cmatch '(?m)\s[A-Z]\s+lmx_msg_exec_test_set_fail_ctx\s*$') { throw "production exec.o exports test fail_ctx setter" }
if ($prodExecNm -cmatch '(?m)\s[A-Z]\s+lmx_msg_exec_test_after_bind_add\s*$') { throw "production exec.o exports test after_bind_add hook" }
if ($prodExecNm -cmatch '(?m)\s[A-Z]\s+lmx_msg_exec_bind_n\s*$') { throw "production exec.o exports test bind_n" }
if ($prodExecNm -cmatch '(?m)\s[A-Z]\s+lmx_msg_exec_bind_aff\s*$') { throw "production exec.o exports test bind_aff" }
if ($prodExecNm -cmatch '(?m)\s[A-Z]\s+lmx_msg_exec_bind_has_worker\s*$') { throw "production exec.o exports test bind_has_worker" }
if ($prodExecNm -cmatch '(?m)\s[A-Z]\s+lmx_msg_exec_test_fail_hits\s*$') { throw "production exec.o exports test fail_hits" }
if ($prodExecNm -cmatch '(?m)\s[A-Z]\s+lmx_msg_exec_get_scan\s*$') { throw "production exec.o exports test get_scan" }
}
if ($selected.Host) {
$hostExe = Join-Path $out "lmx_message_host_selftest.exe"
$hostObjects = @((Get-LmxObject $msgC @('-DLMX_MSG_HOST_TEST'))) + @(Get-LmxSupportObjects @('-DLMX_MSG_HOST_TEST'))
$hostObjectStr = ($hostObjects | ForEach-Object { '"' + $_ + '"' }) -join ' '
cmd /c "gcc -std=c99 -Wall -Wextra -Wpedantic $gstr -I . -I lm1/build -I `"$blkInc`" -DLMX_MSG_HOST_TEST l2src\lmx_message_host_selftest.c $hostObjectStr -o `"$hostExe`" > `"$(Join-Path $log 'lmx_message_host_selftest.gcc.log')`" 2>&1"
if ($LASTEXITCODE -ne 0) {
    Get-Content (Join-Path $log "lmx_message_host_selftest.gcc.log")
    throw "$gen gcc failed: lmx_message_host_selftest"
}
$hostExit = Invoke-LmxTest $hostExe 'lmx_message_host_selftest' $nativeCwd
}
if ($selected.Exec) {
$execExe = Join-Path $out "lmx_message_exec_selftest.exe"
$execObjects = @((Get-LmxObject $msgC @('-DLMX_MSG_EXEC_TEST'))) + @(Get-LmxSupportObjects @('-DLMX_MSG_EXEC_TEST') @('-DLMX_MSG_EXEC_TEST', '-Dmalloc=lmx_msg_history_test_malloc', '-Dfree=lmx_msg_history_test_free'))
$execObjectStr = ($execObjects | ForEach-Object { '"' + $_ + '"' }) -join ' '
cmd /c "gcc -std=c99 -Wall -Wextra -Wpedantic $gstr -I . -I lm1/build -I `"$blkInc`" -DLMX_MSG_EXEC_TEST l2src\lmx_message_exec_selftest.c $execObjectStr -o `"$execExe`" > `"$(Join-Path $log 'lmx_message_exec_selftest.gcc.log')`" 2>&1"
if ($LASTEXITCODE -ne 0) {
    Get-Content (Join-Path $log "lmx_message_exec_selftest.gcc.log")
    throw "$gen gcc failed: lmx_message_exec_selftest"
}
$execOut = Join-Path $log "lmx_message_exec_selftest.stdout.txt"
$execErr = Join-Path $log "lmx_message_exec_selftest.stderr.txt"
$execExit = Invoke-LmxTest $execExe 'lmx_message_exec_selftest' $nativeCwd

$idxDir = Join-Path $out 'array_index_collect'
New-Item -ItemType Directory -Force -Path $idxDir | Out-Null
$idxL1 = Join-Path $idxDir 'array_index.lm1'
$idxC = Join-Path $idxDir 'array_index.c'
$idxO = Join-Path $idxDir 'array_index.o'
$idxExe = Join-Path $idxDir 'lmx_generated_array_index_collect.exe'
$trackedL1 = 'l2src\tests\generated\unit_own_array_index.lm1'
if (-not (Test-Path -LiteralPath $trackedL1)) {
    throw 'missing tracked generated L1 l2src/tests/generated/unit_own_array_index.lm1'
}
Copy-Item -LiteralPath $trackedL1 -Destination $idxL1 -Force
& $trans $idxL1 $idxC *> (Join-Path $idxDir 'array_index.c.log')
if ($LASTEXITCODE -ne 0) { throw "$gen array_index L1->C failed" }
cmd /c "gcc -std=c99 -Wall -Wextra -Wpedantic $gstr -I . -I lm1/build -I `"$blkInc`" -Dmain=l2_generated_main -c `"$idxC`" -o `"$idxO`" > `"$(Join-Path $idxDir 'array_index.o.log')`" 2>&1"
if ($LASTEXITCODE -ne 0) {
    Get-Content (Join-Path $idxDir 'array_index.o.log')
    throw "$gen gcc failed: generated array_index.c"
}
cmd /c "gcc -std=c99 -Wall -Wextra -Wpedantic $gstr -I . -I lm1/build -I `"$blkInc`" -DLMX_MSG_EXEC_TEST l2src\tests\lmx_generated_array_index_collect.c `"$idxO`" $execObjectStr -o `"$idxExe`" > `"$(Join-Path $log 'lmx_generated_array_index_collect.gcc.log')`" 2>&1"
if ($LASTEXITCODE -ne 0) {
    Get-Content (Join-Path $log 'lmx_generated_array_index_collect.gcc.log')
    throw "$gen gcc failed: lmx_generated_array_index_collect"
}
$idxExit = Invoke-LmxTest $idxExe 'lmx_generated_array_index_collect' $nativeCwd

$chDir = Join-Path $out 'array_char_index_collect'
New-Item -ItemType Directory -Force -Path $chDir | Out-Null
$chL1 = Join-Path $chDir 'array_char_index.lm1'
$chC = Join-Path $chDir 'array_char_index.c'
$chO = Join-Path $chDir 'array_char_index.o'
$chExe = Join-Path $chDir 'lmx_generated_array_char_index_collect.exe'
$trackedChL1 = 'l2src\tests\generated\unit_own_array_char_index.lm1'
if (-not (Test-Path -LiteralPath $trackedChL1)) {
    throw 'missing tracked generated L1 l2src/tests/generated/unit_own_array_char_index.lm1'
}
Copy-Item -LiteralPath $trackedChL1 -Destination $chL1 -Force
& $trans $chL1 $chC *> (Join-Path $chDir 'array_char_index.c.log')
if ($LASTEXITCODE -ne 0) { throw "$gen array_char_index L1->C failed" }
cmd /c "gcc -std=c99 -Wall -Wextra -Wpedantic $gstr -I . -I lm1/build -I `"$blkInc`" -Dmain=l2_generated_main -c `"$chC`" -o `"$chO`" > `"$(Join-Path $chDir 'array_char_index.o.log')`" 2>&1"
if ($LASTEXITCODE -ne 0) {
    Get-Content (Join-Path $chDir 'array_char_index.o.log')
    throw "$gen gcc failed: generated array_char_index.c"
}
cmd /c "gcc -std=c99 -Wall -Wextra -Wpedantic $gstr -I . -I lm1/build -I `"$blkInc`" -DLMX_MSG_EXEC_TEST l2src\tests\lmx_generated_array_char_index_collect.c `"$chO`" $execObjectStr -o `"$chExe`" > `"$(Join-Path $log 'lmx_generated_array_char_index_collect.gcc.log')`" 2>&1"
if ($LASTEXITCODE -ne 0) {
    Get-Content (Join-Path $log 'lmx_generated_array_char_index_collect.gcc.log')
    throw "$gen gcc failed: lmx_generated_array_char_index_collect"
}
$chExit = Invoke-LmxTest $chExe 'lmx_generated_array_char_index_collect' $nativeCwd

$forDir = Join-Path $out 'for_arrays_collect'
New-Item -ItemType Directory -Force -Path $forDir | Out-Null
$forL1 = Join-Path $forDir 'for_arrays.lm1'
$forC = Join-Path $forDir 'for_arrays.c'
$forO = Join-Path $forDir 'for_arrays.o'
$forExe = Join-Path $forDir 'lmx_generated_for_arrays_collect.exe'
$trackedForL1 = 'l2src\tests\generated\unit_for_own_arrays.lm1'
if (-not (Test-Path -LiteralPath $trackedForL1)) {
    throw 'missing tracked generated L1 l2src/tests/generated/unit_for_own_arrays.lm1'
}
Copy-Item -LiteralPath $trackedForL1 -Destination $forL1 -Force
& $trans $forL1 $forC *> (Join-Path $forDir 'for_arrays.c.log')
if ($LASTEXITCODE -ne 0) { throw "$gen for_arrays L1->C failed" }
cmd /c "gcc -std=c99 -Wall -Wextra -Wpedantic $gstr -I . -I lm1/build -I `"$blkInc`" -Dmain=l2_generated_main -c `"$forC`" -o `"$forO`" > `"$(Join-Path $forDir 'for_arrays.o.log')`" 2>&1"
if ($LASTEXITCODE -ne 0) {
    Get-Content (Join-Path $forDir 'for_arrays.o.log')
    throw "$gen gcc failed: generated for_arrays.c"
}
cmd /c "gcc -std=c99 -Wall -Wextra -Wpedantic $gstr -I . -I lm1/build -I `"$blkInc`" -DLMX_MSG_EXEC_TEST l2src\tests\lmx_generated_for_arrays_collect.c `"$forO`" $execObjectStr -o `"$forExe`" > `"$(Join-Path $log 'lmx_generated_for_arrays_collect.gcc.log')`" 2>&1"
if ($LASTEXITCODE -ne 0) {
    Get-Content (Join-Path $log 'lmx_generated_for_arrays_collect.gcc.log')
    throw "$gen gcc failed: lmx_generated_for_arrays_collect"
}
$forExit = Invoke-LmxTest $forExe 'lmx_generated_for_arrays_collect' $nativeCwd
}

# L2 loop cancelled through Message control: own-thread map_child and parent-thread sched_step.
if ($selected.Cancel) {
$l2out = 'build\l2trans'
if ($Suite -ne 'Full') { $l2out = Join-Path $out 'l2trans' }
$l2exe = Join-Path $l2out 'l2trans.exe'
$l2c = Join-Path $l2out 'l2trans.c'
$needL2 = -not (Test-Path -LiteralPath $l2exe)
if (-not $needL2) {
    if ((Get-Item -LiteralPath "l2src\l2trans.lm1").LastWriteTime -gt (Get-Item -LiteralPath $l2exe).LastWriteTime) {
        $needL2 = $true
    }
}
if ($needL2) {
    New-Item -ItemType Directory -Force -Path $l2out | Out-Null
    & $trans "l2src\l2trans.lm1" $l2c
    if ($LASTEXITCODE -ne 0) { throw "$gen translate failed: l2src\l2trans.lm1" }
    # Capture stderr natively: PS5.1 otherwise promotes GCC warnings to errors.
    $l2CompileLog = Join-Path $log 'l2trans.gcc.log'
    cmd /c "gcc -std=c99 -Wall -Wextra -Wpedantic $gstr -I . -I lm1/build `"$l2c`" -o `"$l2exe`" > `"$l2CompileLog`" 2>&1"
    if ($LASTEXITCODE -ne 0) {
        Get-Content (Join-Path $log "l2trans.gcc.log")
        throw "$gen gcc failed: l2trans"
    }
}
$spinLm2 = "l2src\tests\cancel_spin.lm2"
$spinLm1 = Join-Path $l2out 'cancel_spin.lm1'
$spinC = Join-Path $l2out 'cancel_spin.c'
$spinObj = Join-Path $l2out 'cancel_spin_nomain.o'
$spinExe = Join-Path $l2out 'cancel_spin_host.exe'
$spinS = Join-Path $log "cancel_spin_m0.s"
& $l2exe $spinLm2 $spinLm1
if ($LASTEXITCODE -ne 0) { throw "$gen l2trans failed: $spinLm2" }
& $trans $spinLm1 $spinC
if ($LASTEXITCODE -ne 0) { throw "$gen l1trans failed: $spinLm1" }
$gstr = ($guards -join " ")
$spinSlog = Join-Path $log "cancel_spin_m0.s.log"
cmd /c "gcc -std=c99 -Wall -Wextra -Wpedantic $gstr -I . -I `"$blkInc`" -O2 -S `"$spinC`" -o `"$spinS`" > `"$spinSlog`" 2>&1"
if ($LASTEXITCODE -ne 0) {
    Get-Content $spinSlog
    throw "$gen gcc -S failed: $spinC"
}
$spinText = [IO.File]::ReadAllText((Join-Path (Get-Location) $spinS))
if ($spinText -notmatch 'l2_m0:') {
    throw "$gen cancel_spin assembly missing l2_m0"
}
if ($spinText -notmatch 'movzbl') {
    throw "$gen cancel_spin -O2 site missing movzbl running load"
}
if ($spinText -match '(?s)l2_m0:.*?call\s+lmx_msg_poll_escape') {
    throw "$gen cancel_spin -O2 still calls poll_escape on the generated hot path"
}
@(
    "generated C: $spinC"
    "gcc -O2 -S (not host wrappers): $spinS"
    "hot site: movzbl of running; cold abort is lmx_msg_poll_abort, not a hot helper call"
) | Set-Content -LiteralPath (Join-Path $log "cancel_spin_m0.evidence.txt") -Encoding utf8
$spinOlog = Join-Path $log "cancel_spin_nomain.gcc.log"
cmd /c "gcc -std=c99 -Wall -Wextra -Wpedantic $gstr -I . -I `"$blkInc`" -O2 -Dmain=cancel_spin_l2_main -c `"$spinC`" -o `"$spinObj`" > `"$spinOlog`" 2>&1"
if ($LASTEXITCODE -ne 0) {
    Get-Content $spinOlog
    throw "$gen gcc failed: $spinC nomain"
}
$spinGlog = Join-Path $log "cancel_spin_host.gcc.log"
$cancelObjects = @((Get-LmxObject $msgC)) + @(Get-LmxSupportObjects)
$cancelObjectStr = ($cancelObjects | ForEach-Object { '"' + $_ + '"' }) -join ' '
cmd /c "gcc -std=c99 -Wall -Wextra -Wpedantic $gstr -I . -I lm1/build -I `"$blkInc`" `"$spinObj`" l2src\tests\cancel_spin_host.c $cancelObjectStr -o `"$spinExe`" > `"$spinGlog`" 2>&1"
if ($LASTEXITCODE -ne 0) {
    Get-Content $spinGlog
    throw "$gen gcc failed: cancel_spin_host"
}
$spinOut = Join-Path $log "cancel_spin_host.stdout.txt"
$spinErr = Join-Path $log "cancel_spin_host.stderr.txt"
$sp = Start-Process -FilePath (Join-Path (Get-Location) $spinExe) -WorkingDirectory (Get-Location) -Wait -PassThru -NoNewWindow -RedirectStandardOutput $spinOut -RedirectStandardError $spinErr
$sp.ExitCode.ToString() | Set-Content -LiteralPath (Join-Path $log "cancel_spin_host.exit.txt") -Encoding ascii
if ($sp.ExitCode -ne 0) {
    Get-Content -LiteralPath $spinOut -ErrorAction SilentlyContinue
    Get-Content -LiteralPath $spinErr -ErrorAction SilentlyContinue
    throw "$gen cancel_spin_host failed exit=$($sp.ExitCode)"
}
}

if ($Suite -eq 'Full') { $banner = "l2 lmx $gen ok" }
else { $banner = "l2 lmx $gen selected=$Suite ok (not a full suite)" }
$toolHash = (Get-FileHash -Algorithm SHA256 (Join-Path (Get-Location) $trans)).Hash
if ($selected.Exec) {
Copy-Item -LiteralPath $execOut -Destination (Join-Path $log "lmx_message_ctx.stdout.txt") -Force
Copy-Item -LiteralPath $execErr -Destination (Join-Path $log "lmx_message_ctx.stderr.txt") -Force
$execExit.ToString() | Set-Content -LiteralPath (Join-Path $log "lmx_message_ctx.exit.txt") -Encoding ascii
@(
    "cmd=l2src\run_lmx.ps1"
    "input=l2src\lmx_message_exec_selftest.c + translated lmx_message.lm1 + lmx_message_host.c + lmx_message_exec.c"
    "L1_GEN=$gen"
    "l1trans=$trans"
    "l1trans_sha256=$toolHash"
    "exe=$execExe"
    "cwd=$nativeCwd"
    "streams=lmx_message_exec_selftest.stdout.txt / lmx_message_exec_selftest.stderr.txt (raw process, copied to lmx_message_ctx.*)"
) | Set-Content -LiteralPath (Join-Path $log "lmx_message_ctx.meta.txt") -Encoding utf8
}
$evPath = Join-Path $nativeCwd 'build/l1trans/logs/gen2/lmx_message_host_selftest.evidence.txt'
$ev = @()
if ($selected.Host -and (Test-Path -LiteralPath $evPath)) { $ev = Get-Content -LiteralPath $evPath }
$ev2Path = Join-Path $nativeCwd 'build/l1trans/logs/gen2/lmx_message_exec_selftest.evidence.txt'
if ($selected.Exec -and (Test-Path -LiteralPath $ev2Path)) { $ev = $ev + (Get-Content -LiteralPath $ev2Path) }
$suiteEvidence = @(
    "cmd=l2src\run_lmx.ps1"
    "L1_GEN=$gen"
    "l1trans=$trans"
    "l1trans_sha256=$toolHash"
    "suite=$Suite"
    "checks=$(@($selected.Keys | Where-Object { $selected[$_] }) -join ',')"
    "banner=$banner"
    "status=PASS"
    "exit=0"
)
if ($selected.Host) { $suiteEvidence += 'host_selftest_exit=0' }
if ($selected.Exec) { $suiteEvidence += "exec_selftest_exit=$execExit" }
if ($selected.Cancel) { $suiteEvidence += @(
    "cancel_spin_stdout=$((Get-Content -LiteralPath $spinOut -Raw).Trim())"
    "cancel_spin_stderr=$((Get-Content -LiteralPath $spinErr -Raw).Trim())"
    "cancel_spin_exit=$($sp.ExitCode)"
) }
$suiteEvidence + $ev | Set-Content -LiteralPath $suiteLog -Encoding utf8
ConvertTo-Json -InputObject @($objectEvidence.ToArray()) -Depth 8 | Set-Content -LiteralPath (Join-Path $log 'lmx_objects.json') -Encoding utf8
$hashLines = @(
    "fresh hashes $(Get-Date -Format o)"
)
$hashPaths = @('l2src/lmx.h', 'l2src/run_lmx.ps1', $trans, $gccPath)
if ($needsMessage) { $hashPaths += @('l2src/lmx_message.lm1', 'l2src/lmx_message.h', 'l2src/lmx_message_exec.c', 'l2src/lmx_message_host.c') }
foreach ($unit in $units) { $hashPaths += @("l2src/$unit.lm1", (Join-Path $out "$unit.c"), (Join-Path $out "$unit.exe")) }
if ($selected.Host) { $hashPaths += @('l2src/lmx_message_host_selftest.c', $hostExe) }
if ($selected.Exec) {
    $hashPaths += @(
        'l2src/lmx_message_exec_selftest.c', $execExe,
        'l2src/tests/generated/unit_own_array_index.lm1',
        'l2src/tests/lmx_generated_array_index_collect.c',
        $idxL1, $idxC, $idxExe,
        'l2src/tests/generated/unit_own_array_char_index.lm1',
        'l2src/tests/lmx_generated_array_char_index_collect.c',
        $chL1, $chC, $chExe,
        'l2src/tests/generated/unit_for_own_arrays.lm1',
        'l2src/tests/lmx_generated_for_arrays_collect.c',
        $forL1, $forC, $forExe
    )
}
if ($selected.Cancel) { $hashPaths += @('l2src/l2trans.lm1', 'l2src/tests/cancel_spin.lm2', 'l2src/tests/cancel_spin_host.c', $spinLm1, $spinC, $spinExe) }
foreach ($hp in $hashPaths) {
    if (Test-Path -LiteralPath $hp) {
        $hashLines += "$((Get-FileHash -Algorithm SHA256 -LiteralPath $hp).Hash.ToLower())  $hp"
    }
}
if ($selected.MessageApi) { $hashLines += "exit_message=0" }
if ($selected.Exec) {
    $hashLines += "exit_exec=$execExit"
    $hashLines += "exit_generated_array_index=$idxExit"
    $hashLines += "exit_generated_array_char_index=$chExit"
    $hashLines += "exit_generated_for_arrays=$forExit"
}
if ($selected.Cancel) { $hashLines += "exit_cancel_spin=$($sp.ExitCode)" }
$hashLines += "selected_suite=$Suite"
$hashLines | Set-Content -LiteralPath (Join-Path $log "lmx_message_ctx.hashes.txt") -Encoding ascii
$banner
} catch {
    @("suite=$Suite", "status=FAILED", "L1_GEN=$gen", "error=$($_.Exception.Message)") | Set-Content -LiteralPath $suiteLog -Encoding utf8
    if ($null -ne $objectEvidence) {
        ConvertTo-Json -InputObject @($objectEvidence.ToArray()) -Depth 8 | Set-Content -LiteralPath (Join-Path $log 'lmx_objects.json') -Encoding utf8
    }
    throw
}
