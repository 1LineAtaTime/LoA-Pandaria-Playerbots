@echo off
title MoP Worldserver
cd /d "%~dp0"
start "MoP Worldserver" /D "%~dp0" "%~dp0worldserver.exe" -c "%~dp0worldserver.conf"
exit /b
