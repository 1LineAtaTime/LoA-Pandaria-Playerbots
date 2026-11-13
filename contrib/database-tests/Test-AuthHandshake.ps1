param(
    [Parameter(Mandatory)][string]$ServerExe,
    [Parameter(Mandatory)][string]$MySqlExe,
    [ValidateSet(33555,33556)][int]$DatabasePort = 33555,
    [string]$PythonExe = 'C:\Program Files\Python310\python.exe'
)
# Explicitly restricted to the isolated matrix instances, never port 3306.
$ErrorActionPreference = 'Stop'
$source = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '../..')).Path
$server = (Resolve-Path -LiteralPath $ServerExe).Path
$mysql = (Resolve-Path -LiteralPath $MySqlExe).Path
$python = (Resolve-Path -LiteralPath $PythonExe).Path
$run = Join-Path $source "out/build/vs2022-x64/auth-handshake/$DatabasePort"
New-Item -ItemType Directory -Path $run -Force | Out-Null
$authPort = $DatabasePort + 4700
$configuration = Get-Content -LiteralPath (Join-Path $source 'src/server/authserver/authserver.conf.dist') -Raw
$configuration = $configuration -replace '(?m)^RealmServerPort\s*=.*', "RealmServerPort = $authPort"
$configuration = $configuration -replace '(?m)^BindIP\s*=.*', 'BindIP = "127.0.0.1"'
$configuration = $configuration -replace '(?m)^LoginDatabaseInfo\s*=.*', "LoginDatabaseInfo = `"127.0.0.1;$DatabasePort;root;;mop_test_auth`""
$configuration = $configuration -replace '(?m)^Updates.EnableDatabases\s*=.*', 'Updates.EnableDatabases = 0'
$configuration = $configuration -replace '(?m)^Updates.AutoSetup\s*=.*', 'Updates.AutoSetup = 0'
$configPath = Join-Path $run 'isolated handshake config.conf'
$configuration | Set-Content -LiteralPath $configPath -Encoding utf8

# Disposable credentials, created exclusively in mop_test_auth on the fixed
# matrix ports. Reset their SRP verifier so authserver must initialize it.
$sql = "INSERT INTO account(username,sha_pass_hash,expansion,dp) VALUES('MOPAUTHTEST',SHA1('MOPAUTHTEST:ISOLATEDTESTPASS'),4,0) ON DUPLICATE KEY UPDATE sha_pass_hash=VALUES(sha_pass_hash),v='',s='',failed_logins=0;"
& $mysql --protocol=TCP --host=127.0.0.1 "--port=$DatabasePort" --user=root --database=mop_test_auth "--execute=$sql"
if ($LASTEXITCODE -ne 0) { throw 'Could not create isolated authentication fixture.' }

$process = $null
try {
    $process = Start-Process -FilePath $server -ArgumentList @('--config', ('"' + $configPath + '"')) -WorkingDirectory $run -WindowStyle Hidden -PassThru -RedirectStandardOutput (Join-Path $run 'server.stdout.log') -RedirectStandardError (Join-Path $run 'server.stderr.log')
    $ready = $false
    for ($attempt=0; $attempt -lt 40; $attempt++) {
        if ($process.HasExited) { throw "Isolated authserver exited with $($process.ExitCode)." }
        $client = [Net.Sockets.TcpClient]::new()
        try { $client.Connect('127.0.0.1', $authPort); $ready=$true }
        catch { Start-Sleep -Milliseconds 250 }
        finally { $client.Dispose() }
        if ($ready) { break }
    }
    if (-not $ready) { throw 'Isolated authentication listener did not start.' }

    $headless = Join-Path $source 'contrib/headless-client/headless.py'
    $successLog = Join-Path $run 'valid-password.log'
    & $python $headless --host 127.0.0.1 --auth-port $authPort --account MOPAUTHTEST --password ISOLATEDTESTPASS --cmd test 2>&1 | ForEach-Object { "$_" -replace ' K=[0-9a-f.]+', ' K=[redacted]' } | Tee-Object -FilePath $successLog
    if ($LASTEXITCODE -ne 0) { throw 'Valid SRP6 authentication failed.' }
    & $python $headless --host 127.0.0.1 --auth-port $authPort --account MOPAUTHTEST --password WRONGTESTPASS --cmd test *> (Join-Path $run 'invalid-password.log')
    if ($LASTEXITCODE -eq 0) { throw 'Invalid password was accepted.' }
    if ((Get-Content -LiteralPath (Join-Path $run 'invalid-password.log') -Raw) -notmatch 'AUTH_LOGON_PROOF rejected|AUTH_LOGON_CHALLENGE rejected') {
        throw 'Invalid-password test failed without a protocol rejection; inspect the log.'
    }
    Write-Output "PASS: MySQL/MariaDB port $DatabasePort; valid 5.4.8 SRP6 login accepted, invalid password rejected. Logs: $run"
}
finally {
    # Stop only the bounded test process created above. Never touch other servers.
    if ($process -and -not $process.HasExited) { $process.Kill(); $process.WaitForExit() }
}
