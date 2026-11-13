param(
    [Parameter(Mandatory=$true)][string]$MySqlExecutable,
    [Parameter(Mandatory=$true)][ValidateSet(33555,33556)][int]$Port
)
$ErrorActionPreference = 'Stop'
$client = (Resolve-Path -LiteralPath $MySqlExecutable).Path
$database = 'mop_test_playerbots'
$base = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '../../modules/mod_playerbots/data/sql/playerbots/base')).Path

function Invoke-Client([string]$sql) {
    $start = [Diagnostics.ProcessStartInfo]::new($client)
    $start.UseShellExecute = $false
    $start.RedirectStandardOutput = $true
    $start.RedirectStandardError = $true
    foreach ($argument in @('--no-defaults', '--protocol=TCP', '--host=127.0.0.1', "--port=$Port", '--user=root', "--database=$database", '--batch', '--skip-column-names', "--execute=$sql")) {
        $start.ArgumentList.Add($argument)
    }
    $process = [Diagnostics.Process]::Start($start)
    $output = $process.StandardOutput.ReadToEnd()
    $errors = $process.StandardError.ReadToEnd()
    $process.WaitForExit()
    if ($process.ExitCode -ne 0) { throw "Fixture SQL failed: $errors" }
    return $output.Trim()
}

# The upstream combined dump contains only a subset of the tables used by the
# current prepared statements. Build a schema-only fixture from missing per-table
# base files; never run DROP TABLE or data INSERTs, and never target other ports.
$existing = [Collections.Generic.HashSet[string]]::new([StringComparer]::OrdinalIgnoreCase)
foreach ($table in ((Invoke-Client 'SHOW TABLES') -split '\r?\n')) {
    if ($table) { [void]$existing.Add($table) }
}
$ignore = @('playerbots_database_full', 'updates', 'updates_include', 'version_db_playerbots')
foreach ($file in (Get-ChildItem -LiteralPath $base -Filter '*.sql' | Sort-Object Name)) {
    if ($file.BaseName -in $ignore) { continue }
    $reader = [IO.StreamReader]::new($file.FullName)
    $builder = [Text.StringBuilder]::new()
    $started = $false
    try {
        while (($line = $reader.ReadLine()) -ne $null) {
            if (-not $started) {
                if ($line -notmatch '^\s*CREATE TABLE') { continue }
                $started = $true
            }
            [void]$builder.AppendLine($line)
            if ($line -match ';\s*$') { break }
        }
    } finally { $reader.Dispose() }
    $ddl = $builder.ToString()
    if ($ddl -notmatch '^CREATE TABLE') { throw "No CREATE TABLE in $($file.Name)" }
    $nameMatch = [regex]::Match($ddl, '(?i)^CREATE\s+TABLE\s+(?:IF\s+NOT\s+EXISTS\s+)?`?([A-Za-z0-9_]+)`?')
    if (-not $nameMatch.Success) { throw "Cannot read table name in $($file.Name)" }
    $tableName = $nameMatch.Groups[1].Value
    if ($existing.Contains($tableName)) { continue }
    [void](Invoke-Client $ddl)
    [void]$existing.Add($tableName)
    Write-Output "Created disposable schema fixture $tableName"
}
