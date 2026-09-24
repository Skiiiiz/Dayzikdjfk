@echo off
chcp 65001 >nul
cd /d "%~dp0"
echo === Сборка DayZModToolkit.exe (PyInstaller) ===
py -3 -m pip install --upgrade pyinstaller -r requirements.txt || python -m pip install --upgrade pyinstaller -r requirements.txt
set ADD=
if exist ffmpeg.exe set ADD=--add-binary "ffmpeg.exe;."
py -3 -m PyInstaller --noconfirm --onefile --windowed --name DayZModToolkit --exclude-module imageio_ffmpeg %ADD% dayz_toolkit.py ^
 || python -m PyInstaller --noconfirm --onefile --windowed --name DayZModToolkit --exclude-module imageio_ffmpeg %ADD% dayz_toolkit.py
echo.
echo Готово: dist\DayZModToolkit.exe
if not exist ffmpeg.exe echo ВНИМАНИЕ: ffmpeg.exe не найден рядом со скриптом - звук работать не будет без ffmpeg.
pause
