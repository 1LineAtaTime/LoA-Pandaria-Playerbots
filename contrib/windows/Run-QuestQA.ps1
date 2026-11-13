[CmdletBinding()]
param([ValidateRange(1,8)][int]$Workers=4,[string]$Select="",[switch]$Force)
$ErrorActionPreference='Stop'
$questRoot=Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
Push-Location -LiteralPath $questRoot
try {
    python -m unittest discover -s contrib/quest-qa -p 'test_*.py'
    if ($LASTEXITCODE -ne 0) { throw 'Quest QA infrastructure tests failed' }
    python -m unittest discover -s contrib/headless-client -p 'test_qa*.py'
    if ($LASTEXITCODE -ne 0) { throw 'Headless client tests failed' }
    $questArgs=@('contrib/quest-qa/campaign.py','--workers',"$Workers")
    if ($Select) {$questArgs+=@('--select',$Select)}
    if ($Force) {$questArgs+='--force'}
    python @questArgs
    $questCampaignExit=$LASTEXITCODE
    python contrib/quest-qa/report.py
    if ($LASTEXITCODE -ne 0) { throw 'Quest QA report export failed' }
    if ($questCampaignExit -ne 0) { throw 'Gameplay cases failed or were blocked; inspect the durable results' }
} finally { Pop-Location }
