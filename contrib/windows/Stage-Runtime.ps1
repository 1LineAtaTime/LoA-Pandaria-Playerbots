[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)][string]$BuildDirectory,
    [ValidateSet('Debug', 'Release', 'RelWithDebInfo', 'MinSizeRel')]
    [string]$Configuration = 'RelWithDebInfo',
    [string]$DestinationDirectory,
    [string[]]$DependencyDirectory = @()
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
$buildPath = (Resolve-Path -LiteralPath $BuildDirectory).Path
$binaryPath = Join-Path $buildPath "bin\$Configuration"
if (-not (Test-Path -LiteralPath $binaryPath -PathType Container)) {
    throw "Build output directory does not exist: $binaryPath"
}
if (-not $DestinationDirectory) {
    $DestinationDirectory = Join-Path $buildPath "stage\$Configuration"
}
$destinationPath = [IO.Path]::GetFullPath($DestinationDirectory)
[void](New-Item -ItemType Directory -Path $destinationPath -Force)

# Read generated build settings, without executing or importing cache contents.
$cache = @{}
foreach ($line in Get-Content -LiteralPath (Join-Path $buildPath 'CMakeCache.txt')) {
    if ($line -match '^([^#/:][^:]*):[^=]+=(.*)$') {
        $cache[$Matches[1]] = $Matches[2]
    }
}
$searchDirectories = @($binaryPath) + $DependencyDirectory
if ($cache.ContainsKey('MYSQL_LIBRARY')) {
    $clientLibraryDirectory = Split-Path -Parent $cache['MYSQL_LIBRARY']
    $searchDirectories += $clientLibraryDirectory
    $searchDirectories += Join-Path (Split-Path -Parent $clientLibraryDirectory) 'bin'
}
if ($cache.ContainsKey('OPENSSL_INCLUDE_DIR')) {
    $sslRoot = Split-Path -Parent $cache['OPENSSL_INCLUDE_DIR']
    $searchDirectories += $sslRoot
    $searchDirectories += Join-Path $sslRoot 'bin'
}
$searchDirectories = @($searchDirectories | Where-Object { $_ -and (Test-Path -LiteralPath $_ -PathType Container) } | Select-Object -Unique)
$dumpbin = $null
if ($cache.ContainsKey('CMAKE_LINKER')) {
    $candidate = Join-Path (Split-Path -Parent $cache['CMAKE_LINKER']) 'dumpbin.exe'
    if (Test-Path -LiteralPath $candidate -PathType Leaf) { $dumpbin = $candidate }
}
if (-not $dumpbin) {
    $command = Get-Command dumpbin.exe -ErrorAction SilentlyContinue
    if ($command) { $dumpbin = $command.Source }
}
if (-not $dumpbin) { throw 'Cannot find MSVC dumpbin.exe; use a configured Visual Studio build directory.' }

$queue = [Collections.Generic.Queue[string]]::new()
$visited = [Collections.Generic.HashSet[string]]::new([StringComparer]::OrdinalIgnoreCase)
$testPrograms = @('battlepay_accounting_tests', 'auction_item_filter_tests', 'database_connector_probe', 'database_transaction_probe', 'database_version_tests')
$artifacts = @(Get-ChildItem -LiteralPath $binaryPath -File | Where-Object {
    ($_.Extension -in '.exe', '.dll', '.pdb' -or $_.Name.EndsWith('.conf.dist')) -and
    $_.BaseName -notin $testPrograms
})
if (-not @($artifacts | Where-Object Extension -eq '.exe').Count) { throw 'No executables have been built.' }
foreach ($artifact in $artifacts) {
    $target = Join-Path $destinationPath $artifact.Name
    if ($artifact.FullName -ne $target) { Copy-Item -LiteralPath $artifact.FullName -Destination $target -Force }
    if ($artifact.Extension -in '.exe', '.dll') { $queue.Enqueue($artifact.FullName) }
}
$sourcePath = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
foreach ($config in @('worldserver', 'authserver')) {
    $template = Join-Path $sourcePath "src/server/$config/$config.conf.dist"
    Copy-Item -LiteralPath $template -Destination (Join-Path $destinationPath "$config.conf.dist") -Force
}
foreach ($notice in @('COPYING.md', 'THANKS.md')) {
    $noticePath = Join-Path $sourcePath $notice
    if (Test-Path -LiteralPath $noticePath -PathType Leaf) {
        Copy-Item -LiteralPath $noticePath -Destination (Join-Path $destinationPath $notice) -Force
    }
}
foreach ($launcher in @('Start-Worldserver.cmd', 'Start-Authserver.cmd')) {
    Copy-Item -LiteralPath (Join-Path $PSScriptRoot $launcher) -Destination (Join-Path $destinationPath $launcher) -Force
}
$playerbotsConfig = Join-Path $sourcePath 'modules/mod_playerbots/config/playerbots.conf.dist'
if (Test-Path -LiteralPath $playerbotsConfig -PathType Leaf) {
    Copy-Item -LiteralPath $playerbotsConfig -Destination (Join-Path $destinationPath 'playerbots.conf.dist') -Force
}
while ($queue.Count) {
    $binary = $queue.Dequeue()
    if (-not $visited.Add([IO.Path]::GetFileName($binary))) { continue }
    $dependencyOutput = & $dumpbin /nologo /dependents $binary
    if ($LASTEXITCODE -ne 0) { throw "dumpbin failed for $binary" }
    foreach ($line in $dependencyOutput) {
        if ($line -notmatch '^\s+([A-Za-z0-9_.+-]+\.dll)\s*$') { continue }
        $dllName = $Matches[1]
        if ($dllName -match '^(api-ms-|ext-ms-)') { continue }
        $dependency = $null
        foreach ($directory in $searchDirectories) {
            $candidate = Join-Path $directory $dllName
            if (Test-Path -LiteralPath $candidate -PathType Leaf) { $dependency = $candidate; break }
        }
        if (-not $dependency) {
            # Resolve configured third-party libraries before consulting System32:
            # some OpenSSL installers also place their DLLs in that directory.
            # Only Windows / installed runtime dependencies are omitted here.
            if (Test-Path -LiteralPath (Join-Path "$env:WINDIR\System32" $dllName)) { continue }
            throw "Cannot resolve $dllName required by $binary. Supply -DependencyDirectory with its installed directory."
        }
        $target = Join-Path $destinationPath $dllName
        if ($dependency -ne $target) { Copy-Item -LiteralPath $dependency -Destination $target -Force }
        $queue.Enqueue($dependency)
    }
}

$manifest = @(Get-ChildItem -LiteralPath $destinationPath -File | Where-Object Name -ne 'manifest.json' | ForEach-Object {
    [ordered]@{ name = $_.Name; bytes = $_.Length; sha256 = (Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash.ToLowerInvariant() }
})
$manifest | ConvertTo-Json -Depth 3 | Set-Content -LiteralPath (Join-Path $destinationPath 'manifest.json') -Encoding utf8
Write-Output "Staged $($manifest.Count) files in $destinationPath"
Write-Output 'Install the Microsoft Visual C++ 2015-2022 x64 Redistributable on the destination host.'
