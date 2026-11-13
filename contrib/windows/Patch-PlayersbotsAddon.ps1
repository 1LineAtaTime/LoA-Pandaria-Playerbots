param([Parameter(Mandatory=$true)][string]$AddonDirectory)
$path = Join-Path $AddonDirectory 'Mini_frame.lua'
$text = [IO.File]::ReadAllText($path)
if ($text.Contains('SendChatMessage(".bot summon")')) { Write-Output 'Already patched'; exit 0 }
if (!$text.Contains('SendChatMessage(".summon")')) { throw 'Unrecognized addon summon button; no changes made.' }
$backup = "$path.before-mop-compatibility"
if (Test-Path -LiteralPath $backup) { throw "Backup already exists: $backup" }
Copy-Item -LiteralPath $path -Destination $backup
[IO.File]::WriteAllText($path, $text.Replace('SendChatMessage(".summon")','SendChatMessage(".bot summon")'), [Text.UTF8Encoding]::new($false))
Write-Output 'Patched only the companion summon button. Reload the client UI.'
