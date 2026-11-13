param([string]$ReportDirectory = 'out/qa/latest', [switch]$AuditOnly, [string]$Suite = '')
$ErrorActionPreference = 'Stop'
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '../..')).Path
Push-Location -LiteralPath $repoRoot
try {
    $qaArgs = @('contrib/headless-client/qa_run.py', '--report-dir', $ReportDirectory)
    if ($Suite) { $qaArgs += @('--suite', $Suite) }
    if ($AuditOnly) { $qaArgs += '--audit-only' }
    else {
        $expectedExe = Join-Path $repoRoot 'exports/worldserver.exe'
        $servers = @(Get-Process worldserver -ErrorAction SilentlyContinue | Where-Object { $_.Path -eq $expectedExe })
        if ($servers.Count -ne 1) { throw 'Start exactly one worldserver from this repository exports folder before running gameplay QA.' }
        $qaArgs += @('--world-pid', [string]$servers[0].Id)
    }
    & python @qaArgs
    $qaExitCode = $LASTEXITCODE
}
finally { Pop-Location }
exit $qaExitCode
