@echo off
chcp 65001 >nul
cd /d "%~dp0"
where py >nul 2>nul && (py -3 dayz_toolkit.py %*) || (python dayz_toolkit.py %*)
if errorlevel 1 pause
