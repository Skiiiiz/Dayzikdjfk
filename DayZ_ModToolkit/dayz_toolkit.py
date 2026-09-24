#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
DayZ Mod Toolkit
================

Конвертация файлов для DayZ и проверка модов на ошибки:
  * звук MP3/WAV/... -> OGG Vorbis (+ генерация CfgSoundShaders/CfgSoundSets);
  * текстуры PNG/TGA/JPG -> PAA и PAA -> PNG;
  * config.cpp -> config.bin и обратно;
  * проверка config.cpp, скриптов .c, layout, XML (types.xml и др.), JSON, stringtable.csv,
    PAA, OGG и ссылок между файлами мода.

Запуск:
    python dayz_toolkit.py                    -> графический интерфейс
    python dayz_toolkit.py <файлы/папки>      -> интерфейс с уже добавленными файлами
    python dayz_toolkit.py --cli --help       -> консольный режим
"""

import sys
from pathlib import Path


def main() -> int:
    args = sys.argv[1:]
    if args and args[0] == "--cli":
        from dztk.cli import cli
        return cli(args[1:])
    if "--cli" in args:  # совместимость: --cli в любом месте
        from dztk.cli import cli
        return cli([a for a in args if a != "--cli"])
    if not args or all(Path(a).exists() for a in args):
        try:
            from dztk.gui import gui
        except ImportError:
            print("tkinter недоступен. Используйте консольный режим: --cli --help")
            return 1
        return gui(args)
    from dztk.cli import cli
    return cli(args)


if __name__ == "__main__":
    sys.exit(main())
