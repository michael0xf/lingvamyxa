from pathlib import Path
p = Path(r"C:\Nyasha_Planet\lingvamyxa\mixa_manager\run_ingress_harness.ps1")
t = p.read_text(encoding="utf-8")
if "force-join-timeout" in t:
    print("already")
else:
    m = '"mixa ingress host harness ok"'
    if not t.endswith(m):
        raise SystemExit("end marker missing: " + repr(t[-60:]))
    add = m + """

# Failure-path safety: deterministic join-timeout injection (harness-only).
$failLog = Join-Path $log "mixa_ingress_host_harness.force_join_timeout.run.log"
$failEv = Join-Path $log "mixa_ingress_host_harness.fail_keep_runtime.txt"
if (Test-Path -LiteralPath $failEv) { Remove-Item -LiteralPath $failEv -Force }
& $exe --force-join-timeout 2>&1 | Tee-Object -FilePath $failLog
if ($LASTEXITCODE -ne 1) {
    throw "force-join-timeout expected exit=1 got=$LASTEXITCODE"
}
if (-not (Test-Path -LiteralPath $failEv)) {
    throw "missing fail_keep_runtime evidence: $failEv"
}
$failText = Get-Content -LiteralPath $failEv -Raw
if ($failText -notmatch "fail_keep_runtime=1") { throw "fail_keep_runtime evidence missing marker" }
if ($failText -notmatch "owner_local_only=1") { throw "fail_keep_runtime evidence missing owner_local_only" }
if ($failText -notmatch "no_close_posted_event=1") { throw "fail_keep_runtime evidence missing no_close_posted_event" }
if ($failText -match "foreign_posted=") { throw "fail_keep_runtime evidence must not inspect poster fields" }
"mixa ingress fail_keep_runtime path ok"
"""
    p.write_text(t[:-len(m)] + add, encoding="utf-8", newline="\n")
    print("runner2 patched")
