# run_serve.ps1
# Hidden launcher for `opencode serve`. Bound to loopback only (127.0.0.1).
# Output is redirected to logs\serve.log by the caller.
param([int]$Port = 4097)

$Root = "C:\Nyasha_Planet\lingvamyxa\work_chat\opencode"
$env:OPENCODE_SERVER_PASSWORD = (Get-Content -LiteralPath (Join-Path $Root "server_password.txt") -Raw).Trim()
$logFile = Join-Path $Root "logs\serve.log"
try {
    & opencode serve --port $Port --hostname 127.0.0.1 *> $logFile
} catch {
    Add-Content -LiteralPath $logFile -Value ("[run_serve] failed: " + $_.Exception.Message) -Encoding utf8
}