[CmdletBinding()]
param([Parameter(Mandatory=$true)][string]$ConfigPath,
      [ValidateSet('world','auth')][string]$ServerKind = 'world')
$ErrorActionPreference = 'Stop'
$path = (Resolve-Path -LiteralPath $ConfigPath).Path
$text = [IO.File]::ReadAllText($path)
$backup = "$path.before-diagnostic-logs-$(Get-Date -Format yyyyMMdd-HHmmss-fff)-$([guid]::NewGuid().ToString('N').Substring(0,8)).bak"
Copy-Item -LiteralPath $path -Destination $backup
$settings = [ordered]@{
    'Appender.Console' = '1,4,2'
    'Appender.Server' = '2,2,23,Server.log,w,52428800'
    'Appender.DBErrors' = '2,2,23,DBErrors.log,w,52428800'
    'Appender.Errors' = '2,5,23,Errors.txt,w,52428800'
    'Appender.ScriptsConsole' = '1,4,2'
    'Logger.root' = '3,Console Server Errors'
    'Logger.server' = '3,Console Server Errors'
    'Logger.scripts' = '3,ScriptsConsole Server Errors'
}
$errorAppender = 'Errors'
if ($ServerKind -eq 'auth') {
    $errorAppender = 'AuthErrors'
    $settings = [ordered]@{
        'Appender.Console' = '1,4,2'
        'Appender.Auth' = '2,3,23,Auth.log,w,52428800'
        'Appender.AuthErrors' = '2,5,23,AuthErrors.txt,w,52428800'
        'Appender.AuthStatus' = '1,3,2'
        'Logger.root' = '3,Console Auth AuthErrors'
        'Logger.server.authserver' = '3,Console Auth AuthErrors'
        'Logger.server.authserver.lifecycle' = '3,AuthStatus Auth AuthErrors'
        'Logger.sql.sql' = '5,Auth AuthErrors'
    }
}
foreach ($key in $settings.Keys) {
    $pattern = '(?m)^' + [regex]::Escape($key) + '\s*=.*$'
    $line = "$key=$($settings[$key])"
    if ([regex]::IsMatch($text,$pattern)) { $text = [regex]::Replace($text,$pattern,$line) }
    else { $text += "`r`n$line`r`n" }
}
# Loggers override ancestor appenders, so attach the central error file to each
# active logger. Keep diagnostic destinations; suppress trace/debug chatter.
$text = [regex]::Replace($text, '(?m)^(Logger\.[^=\r\n]+\s*=\s*\d+\s*,)([^\r\n#]*)(.*)$', {
    param($match)
    $destinations = $match.Groups[2].Value.Trim()
    if ($destinations -notmatch ('(^|\s)' + [regex]::Escape($errorAppender) + '(\s|$)')) { $destinations += " $errorAppender" }
    $prefix = $match.Groups[1].Value
    $prefix = [regex]::Replace($prefix, '=\s*[12]\s*,', '=3,')
    $prefix + $destinations + $match.Groups[3].Value
})
$text = [regex]::Replace($text, '\r*\n', "`r`n")
[IO.File]::WriteAllText($path,$text,[Text.UTF8Encoding]::new($false))
Write-Output 'Diagnostic logging configured. Apply on the next normal restart or config reload. The previous config was backed up.'
