# -*- coding: utf-8 -*-
"""Пишет version_info.txt для PyInstaller (--version-file) из dztk.APP_VERSION."""
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))
from dztk import APP_NAME, APP_VERSION  # noqa: E402

v = tuple(int(x) for x in APP_VERSION.split(".")) + (0,) * (4 - len(APP_VERSION.split(".")))
TEMPLATE = f"""VSVersionInfo(
  ffi=FixedFileInfo(filevers={v}, prodvers={v}, mask=0x3f, flags=0x0, OS=0x40004, fileType=0x1,
                    subtype=0x0, date=(0, 0)),
  kids=[
    StringFileInfo([StringTable('040904B0', [
      StringStruct('FileDescription', '{APP_NAME}'),
      StringStruct('ProductName', '{APP_NAME}'),
      StringStruct('FileVersion', '{APP_VERSION}'),
      StringStruct('ProductVersion', '{APP_VERSION}'),
      StringStruct('OriginalFilename', 'DayZModToolkit.exe'),
      StringStruct('InternalName', 'DayZModToolkit'),
      StringStruct('LegalCopyright', 'DayZ Mod Toolkit')])]),
    VarFileInfo([VarStruct('Translation', [1033, 1200])])
  ]
)
"""
(ROOT / "version_info.txt").write_text(TEMPLATE, encoding="utf-8")
print("version_info.txt", APP_VERSION)
