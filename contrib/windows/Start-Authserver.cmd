@echo off
title MoP Authserver
cd /d "%~dp0"
start "MoP Authserver" /D "%~dp0" "%~dp0authserver.exe" -c "%~dp0authserver.conf"
exit /b
