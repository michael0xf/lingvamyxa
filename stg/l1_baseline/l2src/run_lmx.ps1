# L2 units: Lmx core, registered-range classification, typed service pools.
# Build root is this file's parent, matching tests\l1\*.ps1 convention.
# Picks the L1 generation with L1_GEN (default gen2).
$ErrorActionPreference = "Stop"
Set-Location (Join-Path $PSScriptRoot "..")

$gen = "gen2"
if ($env:L1_GEN -and $env:L1_GEN.Trim().Length -gt 0) { $gen = $env:L1_GEN.Trim() }
$trans = "build\l1trans\$gen\l1trans.exe"
if (-not (Test-Path -LiteralPath $trans)) {
    throw "missing L1 translator: $trans (run tests\l1\run_gen.ps1 first)"
}

$out = "build\l2"
$log = "build\l1trans\logs\$gen"
New-Item -ItemType Directory -Force -Path $out, $log, (Join-Path $out "headers\l2src") | Out-Null
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
$blkInc = Join-Path $out "headers"
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

$guards = @(
    "-Werror=incompatible-pointer-types", "-Werror=discarded-qualifiers",
    "-Werror=implicit-function-declaration", "-Werror=implicit-int"
)

foreach ($unit in @("lmx_selftest", "lmx_pool_selftest", "lmx_chars_selftest", "lmx_ref_selftest", "lmx_branch_selftest", "lmx_own_selftest", "lmx_size_selftest", "lmx_dec_selftest", "lmx_message_selftest")) {
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
        $extra = @("l2src\lmx_message_host.c", "l2src\lmx_message_exec.c", $blkC, $rngC, $stgC, $pathC, $slotsC)
        $defs = @("-DLMX_MSG_HOST_TEST")
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
& $trans "l2src\lmx_message.lm1" $msgC
if ($LASTEXITCODE -ne 0) { throw "$gen translate failed: l2src\lmx_message.lm1" }
$prodHostO = Join-Path $out "lmx_message_host_prod.o"
$gstr = ($guards -join " ")
cmd /c "gcc -std=c99 -Wall -Wextra -Wpedantic $gstr -I . -I lm1/build -I `"$blkInc`" -c l2src\lmx_message_host.c -o `"$prodHostO`" > `"$(Join-Path $log 'lmx_message_host_prod.gcc.log')`" 2>&1"
if ($LASTEXITCODE -ne 0) {
    Get-Content (Join-Path $log "lmx_message_host_prod.gcc.log")
    throw "$gen gcc failed: production host.o"
}
$prodNm = & nm --defined-only $prodHostO 2>&1 | Out-String
if ($LASTEXITCODE -ne 0) { throw "nm failed on production host.o" }
if ($prodNm -match 'lmx_msg_host_test_set_') { throw "production host.o exports test setters" }
if ($prodNm -cmatch '(?m)\s[A-Z]\s+lmx_msg_host_test_nomem\s*$') { throw "production host.o has mutable test_nomem" }
$prodExecO = Join-Path $out "lmx_message_exec_prod.o"
cmd /c "gcc -std=c99 -Wall -Wextra -Wpedantic $gstr -I . -I lm1/build -I `"$blkInc`" -c l2src\lmx_message_exec.c -o `"$prodExecO`" > `"$(Join-Path $log 'lmx_message_exec_prod.gcc.log')`" 2>&1"
if ($LASTEXITCODE -ne 0) {
    Get-Content (Join-Path $log "lmx_message_exec_prod.gcc.log")
    throw "$gen gcc failed: production exec.o"
}
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
$hostExe = Join-Path $out "lmx_message_host_selftest.exe"
cmd /c "gcc -std=c99 -Wall -Wextra -Wpedantic $gstr -I . -I lm1/build -I `"$blkInc`" -DLMX_MSG_HOST_TEST l2src\lmx_message_host_selftest.c `"$msgC`" l2src\lmx_message_host.c l2src\lmx_message_exec.c `"$blkC`" `"$rngC`" `"$stgC`" `"$pathC`" `"$slotsC`" -o `"$hostExe`" > `"$(Join-Path $log 'lmx_message_host_selftest.gcc.log')`" 2>&1"
if ($LASTEXITCODE -ne 0) {
    Get-Content (Join-Path $log "lmx_message_host_selftest.gcc.log")
    throw "$gen gcc failed: lmx_message_host_selftest"
}
& $hostExe
if ($LASTEXITCODE -ne 0) { throw "$gen lmx_message_host_selftest failed" }
$execExe = Join-Path $out "lmx_message_exec_selftest.exe"
cmd /c "gcc -std=c99 -Wall -Wextra -Wpedantic $gstr -I . -I lm1/build -I `"$blkInc`" -DLMX_MSG_EXEC_TEST l2src\lmx_message_exec_selftest.c `"$msgC`" l2src\lmx_message_host.c l2src\lmx_message_exec.c `"$blkC`" `"$rngC`" `"$stgC`" `"$pathC`" `"$slotsC`" -o `"$execExe`" > `"$(Join-Path $log 'lmx_message_exec_selftest.gcc.log')`" 2>&1"
if ($LASTEXITCODE -ne 0) {
    Get-Content (Join-Path $log "lmx_message_exec_selftest.gcc.log")
    throw "$gen gcc failed: lmx_message_exec_selftest"
}
$execOut = Join-Path $log "lmx_message_exec_selftest.stdout.txt"
$execErr = Join-Path $log "lmx_message_exec_selftest.stderr.txt"
$p = Start-Process -FilePath (Join-Path (Get-Location) $execExe) -WorkingDirectory (Get-Location) -Wait -PassThru -NoNewWindow -RedirectStandardOutput $execOut -RedirectStandardError $execErr
$p.ExitCode.ToString() | Set-Content -LiteralPath (Join-Path $log "lmx_message_exec_selftest.exit.txt") -Encoding ascii
if ($p.ExitCode -ne 0) {
    Get-Content -LiteralPath $execOut -ErrorAction SilentlyContinue
    Get-Content -LiteralPath $execErr -ErrorAction SilentlyContinue
    throw "$gen lmx_message_exec_selftest failed exit=$($p.ExitCode)"
}

# L2 loop cancelled through Message control: own-thread map_child and parent-thread sched_step.
$l2exe = "build\l2trans\l2trans.exe"
$l2c = "build\l2trans\l2trans.c"
$needL2 = -not (Test-Path -LiteralPath $l2exe)
if (-not $needL2) {
    if ((Get-Item -LiteralPath "l2src\l2trans.lm1").LastWriteTime -gt (Get-Item -LiteralPath $l2exe).LastWriteTime) {
        $needL2 = $true
    }
}
if ($needL2) {
    New-Item -ItemType Directory -Force -Path "build\l2trans" | Out-Null
    & $trans "l2src\l2trans.lm1" $l2c
    if ($LASTEXITCODE -ne 0) { throw "$gen translate failed: l2src\l2trans.lm1" }
    & gcc -std=c99 -Wall -Wextra -Wpedantic @guards -I . -I lm1/build $l2c -o $l2exe 2>&1 |
        Tee-Object -FilePath (Join-Path $log "l2trans.gcc.log") | Out-Null
    if ($LASTEXITCODE -ne 0) {
        Get-Content (Join-Path $log "l2trans.gcc.log")
        throw "$gen gcc failed: l2trans"
    }
}
$spinLm2 = "l2src\tests\cancel_spin.lm2"
$spinLm1 = "build\l2trans\cancel_spin.lm1"
$spinC = "build\l2trans\cancel_spin.c"
$spinObj = "build\l2trans\cancel_spin_nomain.o"
$spinExe = "build\l2trans\cancel_spin_host.exe"
$spinS = Join-Path $log "cancel_spin_m0.s"
& $l2exe $spinLm2 $spinLm1
if ($LASTEXITCODE -ne 0) { throw "$gen l2trans failed: $spinLm2" }
& $trans $spinLm1 $spinC
if ($LASTEXITCODE -ne 0) { throw "$gen l1trans failed: $spinLm1" }
$gstr = ($guards -join " ")
$spinSlog = Join-Path $log "cancel_spin_m0.s.log"
cmd /c "gcc -std=c99 -Wall -Wextra -Wpedantic $gstr -I . -O2 -S `"$spinC`" -o `"$spinS`" > `"$spinSlog`" 2>&1"
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
cmd /c "gcc -std=c99 -Wall -Wextra -Wpedantic $gstr -I . -O2 -Dmain=cancel_spin_l2_main -c `"$spinC`" -o `"$spinObj`" > `"$spinOlog`" 2>&1"
if ($LASTEXITCODE -ne 0) {
    Get-Content $spinOlog
    throw "$gen gcc failed: $spinC nomain"
}
$spinGlog = Join-Path $log "cancel_spin_host.gcc.log"
cmd /c "gcc -std=c99 -Wall -Wextra -Wpedantic $gstr -I . -I lm1/build -I `"$blkInc`" `"$spinObj`" l2src\tests\cancel_spin_host.c `"$msgC`" l2src\lmx_message_host.c l2src\lmx_message_exec.c `"$blkC`" `"$rngC`" `"$stgC`" `"$pathC`" `"$slotsC`" -o `"$spinExe`" > `"$spinGlog`" 2>&1"
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

"l2 lmx $gen ok"
$suiteLog = Join-Path $log "lmx_suite.log"
$toolHash = (Get-FileHash -Algorithm SHA256 (Join-Path (Get-Location) $trans)).Hash
Copy-Item -LiteralPath $execOut -Destination (Join-Path $log "lmx_message_ctx.stdout.txt") -Force
Copy-Item -LiteralPath $execErr -Destination (Join-Path $log "lmx_message_ctx.stderr.txt") -Force
$p.ExitCode.ToString() | Set-Content -LiteralPath (Join-Path $log "lmx_message_ctx.exit.txt") -Encoding ascii
@(
    "cmd=l2src\run_lmx.ps1"
    "input=l2src\lmx_message_exec_selftest.c + translated lmx_message.lm1 + lmx_message_host.c + lmx_message_exec.c"
    "L1_GEN=$gen"
    "l1trans=$trans"
    "l1trans_sha256=$toolHash"
    "exe=$execExe"
    "cwd=$(Get-Location)"
    "streams=lmx_message_exec_selftest.stdout.txt / lmx_message_exec_selftest.stderr.txt (raw process, copied to lmx_message_ctx.*)"
) | Set-Content -LiteralPath (Join-Path $log "lmx_message_ctx.meta.txt") -Encoding utf8
$evPath = Join-Path $log "lmx_message_host_selftest.evidence.txt"
$ev = @()
if (Test-Path -LiteralPath $evPath) { $ev = Get-Content -LiteralPath $evPath }
$ev2Path = Join-Path $log "lmx_message_exec_selftest.evidence.txt"
if (Test-Path -LiteralPath $ev2Path) { $ev = $ev + (Get-Content -LiteralPath $ev2Path) }
@(
    "cmd=l2src\run_lmx.ps1"
    "L1_GEN=$gen"
    "l1trans=$trans"
    "l1trans_sha256=$toolHash"
    "banner=l2 lmx $gen ok"
    "exit=0"
    "host_selftest_stdout=lmx_message_host ok (see evidence)"
    "exec_selftest_stdout=lmx_message_exec ok (see evidence)"
    "cancel_spin_stdout=$((Get-Content -LiteralPath $spinOut -Raw).Trim())"
    "cancel_spin_stderr=$((Get-Content -LiteralPath $spinErr -Raw).Trim())"
    "cancel_spin_exit=$($sp.ExitCode)"
) + $ev | Set-Content -LiteralPath $suiteLog -Encoding utf8
$hashLines = @(
    "fresh hashes $(Get-Date -Format o)"
)
foreach ($hp in @(
    "l2src\lmx.h",
    "l2src\lmx_poll_stub.c",
    "l2src\lmx_message.lm1",
    "l2src\lmx_message.h",
    "l2src\lmx_message_exec.c",
    "l2src\lmx_message_exec_selftest.c",
    "l2src\lmx_message_selftest.lm1",
    "l2src\l2trans.lm1",
    "l2src\tests\cancel_spin.lm2",
    "l2src\tests\cancel_spin_host.c",
    "l2src\run_lmx.ps1",
    "build\l2trans\cancel_spin.lm1",
    "build\l2trans\cancel_spin.c",
    "build\l2\lmx_message_selftest.exe",
    "build\l2\lmx_message_exec_selftest.exe",
    "build\l2trans\cancel_spin_host.exe"
)) {
    if (Test-Path -LiteralPath $hp) {
        $hashLines += "$((Get-FileHash -Algorithm SHA256 (Join-Path (Get-Location) $hp)).Hash.ToLower())  $hp"
    }
}
$hashLines += "exit_message=0"
$hashLines += "exit_exec=0"
$hashLines += "exit_cancel_spin=$($sp.ExitCode)"
$hashLines | Set-Content -LiteralPath (Join-Path $log "lmx_message_ctx.hashes.txt") -Encoding ascii
