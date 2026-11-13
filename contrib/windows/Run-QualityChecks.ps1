[CmdletBinding()]
param(
    [ValidateSet('targeted','full')][string]$Profile = 'targeted',
    [string]$RunDirectory = '',
    [string]$BackupPlan = '',
    [switch]$AuditOnly,
    [switch]$Build
)
$ErrorActionPreference = 'Stop'
$qualityRoot = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '../..')).Path
Push-Location -LiteralPath $qualityRoot
try {
    $qualityArguments = @('contrib/qa-automation/pipeline.py','--profile',$Profile)
    if ($RunDirectory) { $qualityArguments += @('--run-dir',$RunDirectory) }
    if ($BackupPlan) { $qualityArguments += @('--backup-plan',$BackupPlan) }
    if ($Build) { $qualityArguments += '--build' }
    if ($AuditOnly) { $qualityArguments += '--audit-only' }
    else {
        $qualityExpectedWorld = Join-Path $qualityRoot 'exports/worldserver.exe'
        $qualityServers = @(Get-Process worldserver -ErrorAction SilentlyContinue | Where-Object { $_.Path -eq $qualityExpectedWorld })
        if ($qualityServers.Count -ne 1) { throw 'Start exactly one worldserver from this exports folder.' }
        $qualityArguments += @('--world-pid',[string]$qualityServers[0].Id)
    }
    & python @qualityArguments
    $qualityExit = $LASTEXITCODE
}
finally { Pop-Location }
exit $qualityExit
