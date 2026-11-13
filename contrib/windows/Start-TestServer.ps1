$ErrorActionPreference = 'Stop'
$env:PSModulePath += [IO.Path]::PathSeparator + (Join-Path $PSHOME 'Modules')
$projectRoot = (Resolve-Path (Join-Path $PSScriptRoot '../..')).Path
$runtime = Join-Path $projectRoot 'exports'
$reportFolder = Join-Path $projectRoot 'out/qa-automation/server'
[IO.Directory]::CreateDirectory($reportFolder) | Out-Null
$report = [ordered]@{Status='STARTING';Runtime=$runtime;Processes=@();StartedUTC=[DateTime]::UtcNow.ToString('o')}
$mutex = New-Object Threading.Mutex($false,'Local\MoPPlayerbotsSingleTestServer')
$acquired = $false
Push-Location $projectRoot
try {
    try { $acquired=$mutex.WaitOne(0) } catch [Threading.AbandonedMutexException] { $acquired=$true }
    if (-not $acquired) { throw 'Server startup is already running. No second pair was started.' }
    python contrib/quest-qa/regular_runtime.py
    if ($LASTEXITCODE -ne 0) { throw 'Existing test runtime validation failed. No configuration was replaced.' }
    foreach ($name in @('authserver','worldserver')) {
        $expected=Join-Path $runtime ($name+'.exe')
        $foreign=@(Get-CimInstance Win32_Process -Filter ("name='"+$name+".exe'") | Where-Object {$_.ExecutablePath -ne $expected})
        if ($foreign.Count) { throw 'Another auth/world instance is running. Close it first; this launcher will not start a duplicate pair.' }
    }
    foreach ($server in @(@{Name='authserver';Port=3724},@{Name='worldserver';Port=8085})) {
        $exe=Join-Path $runtime ($server.Name+'.exe')
        $existing=@(Get-CimInstance Win32_Process -Filter ("name='"+$server.Name+".exe'") | Where-Object {$_.ExecutablePath -eq $exe})
        if ($existing.Count -gt 1) { throw ('Multiple '+$server.Name+' instances already exist.') }
        if ($existing.Count) { $serverPid=$existing[0].ProcessId }
        else {
            if (Get-NetTCPConnection -State Listen -LocalPort $server.Port -ErrorAction SilentlyContinue) { throw ('Port '+$server.Port+' is occupied; no process was stopped.') }
            $process=Start-Process -FilePath $exe -ArgumentList '-c',('"'+(Join-Path $runtime ($server.Name+'.conf'))+'"') -WorkingDirectory $runtime -WindowStyle Normal -PassThru
            $serverPid=$process.Id
        }
        $deadline=[DateTime]::UtcNow.AddMinutes(3)
        do {
            $ready=Get-NetTCPConnection -State Listen -LocalPort $server.Port -ErrorAction SilentlyContinue | Where-Object {$_.OwningProcess -eq $serverPid}
            if ($ready) { break }
            if (-not (Get-Process -Id $serverPid -ErrorAction SilentlyContinue)) { throw ($server.Name+' exited during startup. Inspect its diagnostic log.') }
            Start-Sleep -Seconds 1
        } while ([DateTime]::UtcNow -lt $deadline)
        if (-not $ready) { throw ($server.Name+' did not become ready within three minutes.') }
        $report.Processes+=@{Name=$server.Name;Pid=$serverPid;Port=$server.Port}
        Write-Host ($server.Name+' ready, PID '+$serverPid)
    }
    $report.Status='READY'
    Write-Host 'One authserver and one worldserver are ready. Run-QA.cmd uses this same pair.'
    exit 0
} catch {
    $report.Status='BLOCKED';$report.Reason=$_.Exception.Message
    Write-Host ('Startup blocked: '+$_.Exception.Message)
    exit 1
} finally {
    $report.FinishedUTC=[DateTime]::UtcNow.ToString('o')
    [IO.File]::WriteAllText((Join-Path $reportFolder 'latest.json'),($report | ConvertTo-Json -Depth 6))
    if ($acquired) { $mutex.ReleaseMutex() }
    $mutex.Dispose();Pop-Location
}
