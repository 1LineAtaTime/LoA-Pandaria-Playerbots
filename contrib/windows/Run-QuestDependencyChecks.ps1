[CmdletBinding()]
param(
    [string]$BaselineDirectory = 'doc/quest-dependency-audit-548',
    [string]$RunDirectory = ''
)
$ErrorActionPreference = 'Stop'
$questCheckRoot = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '../..')).Path
Push-Location -LiteralPath $questCheckRoot
try {
    if (-not $RunDirectory) {
        $questRunStamp = [DateTime]::UtcNow.ToString('yyyyMMddTHHmmssfffZ') + '-' + [Guid]::NewGuid().ToString('N').Substring(0,8)
        $RunDirectory = Join-Path 'out/quest-dependency-checks' $questRunStamp
    }
    # A fresh directory prevents an interrupted generator being mistaken for an old pass.
    if (Test-Path -LiteralPath $RunDirectory) { throw 'Use a new run directory; existing evidence is never overwritten.' }
    [void](New-Item -ItemType Directory -Path $RunDirectory)
    $questAuditOutput = Join-Path $RunDirectory 'dependencies'
    $questGraphOutput = Join-Path $RunDirectory 'graph'
    & python contrib/content-audit/quest_dependencies.py --output $questAuditOutput --artifacts $questGraphOutput
    if ($LASTEXITCODE -ne 0) { throw 'Dependency generation failed; no comparison accepted.' }
    $questComparisonPath = Join-Path $RunDirectory 'comparison.json'
    & python contrib/content-audit/compare_quest_audits.py $BaselineDirectory $questAuditOutput --report $questComparisonPath
    if ($LASTEXITCODE -ne 0) { throw "New dependency failures or removed quests; inspect $questComparisonPath" }
    Write-Output "Dependency regression check passed: $questComparisonPath"
}
finally { Pop-Location }
