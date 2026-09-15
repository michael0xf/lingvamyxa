# Stage 5 (d): every migrated test carries its own copy of `fn: turn_step_child`
# (a translated test cannot share a function body through the predef header).
# Each copy, up to the next blank line with CRLF normalised to LF, must hash the
# same. Prints each file with its hash and exits 1 unless at least one copy
# exists and all copies agree.
$sha = [Security.Cryptography.SHA256]::Create()
$rows = @()
foreach ($f in Get-ChildItem (Join-Path $PSScriptRoot 'tests') -Filter '*.lm1' | Sort-Object Name) {
    $lines = [IO.File]::ReadAllText($f.FullName).Replace("`r`n", "`n").Split("`n")
    $start = -1
    for ($i = 0; $i -lt $lines.Count; $i++) { if ($lines[$i] -match '^fn: turn_step_child \(') { $start = $i; break } }
    if ($start -lt 0) { continue }
    $end = $start
    while ($end + 1 -lt $lines.Count -and $lines[$end + 1].Trim() -ne '') { $end++ }
    $block = $lines[$start..$end] -join "`n"
    $hash = ([BitConverter]::ToString($sha.ComputeHash([Text.Encoding]::UTF8.GetBytes($block)))).Replace('-', '').Substring(0, 16)
    $rows += [pscustomobject]@{ File = $f.Name; Lines = $end - $start + 1; Hash = $hash }
}
$rows | ForEach-Object { "{0}  {1,2} lines  {2}" -f $_.Hash, $_.Lines, $_.File }
$distinct = @($rows | Select-Object -ExpandProperty Hash -Unique)
"turn_step_child copies: $($rows.Count); distinct hashes: $($distinct.Count)" + $(if ($distinct.Count -eq 1) { "; the hash: $($distinct[0])" } else { '' })
if ($rows.Count -eq 0 -or $distinct.Count -ne 1) { exit 1 }
exit 0
