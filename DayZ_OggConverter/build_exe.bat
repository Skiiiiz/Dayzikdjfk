@echo off
chcp 65001 >nul
cd /d "%~dp0"
echo === Сборка DayZOggConverter.exe (PyInstaller) ===
py -3 -m pip install --upgrade pyinstaller || python -m pip install --upgrade pyinstaller
set ADD=
if exist ffmpeg.exe set ADD=--add-binary "ffmpeg.exe;."
py -3 -m PyInstaller --noconfirm --onefile --windowed --name DayZOggConverter %ADD% dayz_ogg_converter.py ^
 || python -m PyInstaller --noconfirm --onefile --windowed --name DayZOggConverter %ADD% dayz_ogg_converter.py
echo.
echo Готово: dist\DayZOggConverter.exe
if not exist ffmpeg.exe echo ВНИМАНИЕ: ffmpeg.exe не найден рядом со скриптом - положите его рядом с .exe вручную.
pause
