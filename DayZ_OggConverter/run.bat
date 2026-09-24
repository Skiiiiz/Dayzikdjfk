@echo off
chcp 65001 >nul
cd /d "%~dp0"
where py >nul 2>nul && (py -3 dayz_ogg_converter.py %*) || (python dayz_ogg_converter.py %*)
if errorlevel 1 pause
