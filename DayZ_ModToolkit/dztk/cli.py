# -*- coding: utf-8 -*-
"""Консольный режим.

    DayZModToolkit --cli audio    <файлы/папки> [опции]   звук -> OGG
    DayZModToolkit --cli paa      <картинки>    [опции]   PNG/TGA/JPG -> PAA
    DayZModToolkit --cli png      <paa>         [опции]   PAA -> PNG
    DayZModToolkit --cli rapify   <config.cpp>  [опции]   config.cpp -> config.bin
    DayZModToolkit --cli derapify <config.bin>  [опции]   config.bin -> config.cpp
    DayZModToolkit --cli check    <файлы/папки> [опции]   проверка на ошибки (--game, --watch)
    DayZModToolkit --cli pack     <папка мода>  [опции]   сборка PBO (проверка, бинаризация, подпись)
    DayZModToolkit --cli unpack   <pbo>         [опции]   распаковка PBO
    DayZModToolkit --cli keygen   <имя_ключа>   [опции]   создать .biprivatekey и .bikey
    DayZModToolkit --cli sign     <pbo>  --key <файл.biprivatekey>
    DayZModToolkit --cli verify   <pbo>  [--key <файл.bikey>]
    DayZModToolkit --cli types    <config/папка мода> [опции]   записи types.xml из конфига

Без подкоманды (--cli <файлы>) выполняется конвертация звука, как в прежних версиях.
"""

from __future__ import annotations

from dztk.i18n import tr

import argparse
import os
import sys
from pathlib import Path
from typing import List

from . import APP_NAME, APP_VERSION, checks, convert, paa
from .audio import (NORMALIZE_MODES, PRESETS, ConvertSettings, JobResult, collect_inputs, convert_all,
                    plan_jobs, write_config)
from .common import check_vorbis, find_ffmpeg, human_size

USAGE = """Консольный режим.

    DayZModToolkit --cli audio    <файлы/папки> [опции]   звук -> OGG
    DayZModToolkit --cli paa      <картинки>    [опции]   PNG/TGA/JPG -> PAA
    DayZModToolkit --cli png      <paa>         [опции]   PAA -> PNG
    DayZModToolkit --cli rapify   <config.cpp>  [опции]   config.cpp -> config.bin
    DayZModToolkit --cli derapify <config.bin>  [опции]   config.bin -> config.cpp
    DayZModToolkit --cli check    <файлы/папки> [опции]   проверка на ошибки (--game, --watch)
    DayZModToolkit --cli pack     <папка мода>  [опции]   сборка PBO (проверка, бинаризация, подпись)
    DayZModToolkit --cli unpack   <pbo>         [опции]   распаковка PBO
    DayZModToolkit --cli keygen   <имя_ключа>   [опции]   создать .biprivatekey и .bikey
    DayZModToolkit --cli sign     <pbo>  --key <файл.biprivatekey>
    DayZModToolkit --cli verify   <pbo>  [--key <файл.bikey>]
    DayZModToolkit --cli types    <config/папка мода> [опции]   записи types.xml из конфига

Без подкоманды (--cli <файлы>) выполняется конвертация звука, как в прежних версиях.
"""

SUBCOMMANDS = ("audio", "paa", "png", "rapify", "derapify", "check", "pack", "unpack", "keygen", "sign", "verify",
               "types")


def _setup_console() -> None:
    """Готовит вывод для консольного режима.

    Оконный .exe (PyInstaller --windowed) не имеет консоли: подключаемся к консоли,
    из которой его запустили. Символы, которых нет в кодировке консоли, заменяются,
    а не роняют программу.
    """
    if os.name == "nt" and sys.stdout is None:
        try:
            import ctypes
            kernel32 = ctypes.windll.kernel32  # type: ignore[attr-defined]
            if kernel32.AttachConsole(-1):  # ATTACH_PARENT_PROCESS
                enc = f"cp{kernel32.GetConsoleOutputCP()}"
                sys.stdout = open("CONOUT$", "w", encoding=enc, errors="replace")
                sys.stderr = sys.stdout
                print()
        except Exception:
            pass
    for stream in (sys.stdout, sys.stderr):
        if stream is not None and hasattr(stream, "reconfigure"):
            try:
                if not stream.isatty():
                    # вывод в файл или канал: UTF-8, иначе Windows испортит кириллицу (cp1252)
                    stream.reconfigure(encoding="utf-8", errors="replace")
                else:
                    stream.reconfigure(errors="replace")
            except Exception:
                pass




def cli_audio(argv: List[str]) -> int:
    ap = argparse.ArgumentParser(
        prog="DayZModToolkit audio",
        description=tr("{0} {1}: конвертация MP3/WAV/... в OGG Vorbis для DayZ.").format(APP_NAME, APP_VERSION))
    ap.add_argument("inputs", nargs="+", help=tr("файлы и/или папки (папки обходятся рекурсивно)"))
    ap.add_argument("-o", "--output", default="", help=tr("папка вывода (по умолчанию: подпапка 'ogg' рядом с исходником)"))
    ap.add_argument("-p", "--preset", choices=list(PRESETS), help=tr("пресет настроек"))
    ap.add_argument("--mono", action="store_true", help=tr("моно (для 3D звуков)"))
    ap.add_argument("--stereo", action="store_true", help=tr("стерео (музыка/UI)"))
    ap.add_argument("-r", "--rate", type=int, choices=[22050, 32000, 44100, 48000], help=tr("частота дискретизации"))
    ap.add_argument("-q", "--quality", type=int, choices=range(-1, 11), metavar="{-1..10}", help=tr("качество Vorbis"))
    ap.add_argument("--volume", type=float, default=0.0, help=tr("усиление, дБ"))
    ap.add_argument("--normalize", choices=list(NORMALIZE_MODES), default="none")
    ap.add_argument("--peak", type=float, default=-1.0, help=tr("цель пиковой нормализации, dBFS"))
    ap.add_argument("--lufs", type=float, default=-16.0, help=tr("цель нормализации громкости, LUFS"))
    ap.add_argument("--trim-silence", action="store_true", help=tr("обрезать тишину в начале и конце"))
    ap.add_argument("--fade-in", type=float, default=0.0)
    ap.add_argument("--fade-out", type=float, default=0.0)
    ap.add_argument("--start", type=float, default=0.0, help=tr("начать с N секунд"))
    ap.add_argument("--duration", type=float, default=0.0, help=tr("взять только N секунд"))
    ap.add_argument("--no-sanitize", action="store_true", help=tr("не переименовывать файлы в безопасные имена"))
    ap.add_argument("--flat", action="store_true", help=tr("не сохранять структуру подпапок"))
    ap.add_argument("--prefix", default="", help=tr("префикс имени файлов"))
    ap.add_argument("--no-overwrite", action="store_true", help=tr("пропускать уже существующие файлы"))
    ap.add_argument("-j", "--threads", type=int, default=ConvertSettings().threads)
    ap.add_argument("--config", action="store_true", help=tr("сгенерировать config.cpp"))
    ap.add_argument("--mod", default="MyMod", help=tr("имя мода (CfgPatches)"))
    ap.add_argument("--sound-path", default="", help=tr("путь к звукам в PBO, напр. MyMod\\sounds"))
    ap.add_argument("--class-prefix", default="", help=tr("префикс классов SoundShader/SoundSet"))
    ap.add_argument("--range", type=int, help=tr("дальность слышимости, м"))
    ap.add_argument("--loop", action="store_true", help=tr("loop = 1 в SoundSet"))
    ap.add_argument("--no-spatial", action="store_true", help=tr("2D звук (spatial = 0)"))
    ap.add_argument("--ffmpeg", default="", help=tr("путь к ffmpeg"))
    a = ap.parse_args(argv)

    s = ConvertSettings()
    if a.preset:
        p = PRESETS[a.preset]
        s.channels, s.sample_rate, s.quality = p["channels"], p["sample_rate"], p["quality"]
        s.spatial, s.loop, s.range = p["spatial"], p["loop"], p["range"]
    if a.mono:
        s.channels = 1
    if a.stereo:
        s.channels = 2
    if a.rate:
        s.sample_rate = a.rate
    if a.quality is not None:
        s.quality = a.quality
    s.volume_db, s.normalize = a.volume, a.normalize
    s.peak_target_db, s.loudness_target = a.peak, a.lufs
    s.trim_silence, s.fade_in, s.fade_out = a.trim_silence, a.fade_in, a.fade_out
    s.cut_start, s.cut_duration = a.start, a.duration
    s.output_dir, s.keep_structure = a.output, not a.flat
    s.sanitize_names, s.name_prefix = not a.no_sanitize, a.prefix
    s.overwrite, s.threads = not a.no_overwrite, a.threads
    s.generate_config, s.mod_name = a.config, a.mod
    s.sound_path = a.sound_path or f"{a.mod}\\sounds"
    s.class_prefix = a.class_prefix or a.mod
    if a.range is not None:
        s.range = a.range
    if a.loop:
        s.loop = True
    if a.no_spatial:
        s.spatial = False
    s.ffmpeg_path = a.ffmpeg

    ffmpeg = find_ffmpeg(s.ffmpeg_path)
    if not ffmpeg:
        print(tr("ОШИБКА: ffmpeg не найден. Положите ffmpeg.exe рядом с программой, добавьте в PATH или установите: pip install imageio-ffmpeg"), file=sys.stderr)
        return 2
    if not check_vorbis(ffmpeg):
        print(tr("ОШИБКА: ffmpeg ({0}) собран без libvorbis.").format(ffmpeg), file=sys.stderr)
        return 2

    inputs = collect_inputs(a.inputs, s.output_dir)
    if not inputs:
        print(tr("Нет подходящих аудиофайлов."), file=sys.stderr)
        return 1
    jobs = plan_jobs(inputs, s)
    print(f"ffmpeg: {ffmpeg}")
    print(tr("Файлов: {0} | {1}, {2} Гц, q={3}").format(len(jobs), tr('моно') if s.channels == 1 else tr('стерео'), s.sample_rate, s.quality))

    def on_result(r: JobResult, done: int, total: int) -> None:
        status = "OK " if r.ok else "ERR"
        extra = f"{human_size(r.size)}, {r.duration:.2f} c" if r.ok and r.duration else r.message
        print(f"[{done}/{total}] {status} {r.job.src.name} -> {r.job.dst}  ({extra})", flush=True)

    results = convert_all(jobs, s, ffmpeg, on_result)
    ok = sum(1 for r in results if r.ok)
    print(tr("Готово: {0} из {1}").format(ok, len(results)))
    if s.generate_config:
        cfg_path = write_config(results, s)
        if cfg_path:
            print(f"config.cpp: {cfg_path}")
    return 0 if ok == len(results) else 1




def _print_result(r: convert.Result, done: int, total: int) -> None:
    status = "OK " if r.ok else "ERR"
    print(f"[{done}/{total}] {status} {r.task.src} -> {r.task.dst.name}  ({r.message})", flush=True)
    if r.details:
        for i in r.details:
            if i.level != "info":
                print("      " + i.format(), flush=True)


def cli_paa(argv: List[str]) -> int:
    ap = argparse.ArgumentParser(prog="DayZModToolkit paa", description=tr("Картинки (PNG/TGA/JPG/BMP) -> PAA."))
    ap.add_argument("inputs", nargs="+", help=tr("картинки и/или папки"))
    ap.add_argument("-o", "--output", default="", help=tr("папка вывода (по умолчанию рядом с исходником)"))
    ap.add_argument("-f", "--format", choices=["auto", "dxt1", "dxt5"], default="auto",
                    help=tr("auto: DXT5 при прозрачности или по суффиксу _ca/_co, иначе DXT1"))
    ap.add_argument("--resize", action="store_true", help=tr("масштабировать до степени двойки"))
    ap.add_argument("--no-overwrite", action="store_true")
    ap.add_argument("-j", "--threads", type=int, default=max(1, min(4, os.cpu_count() or 2)))
    a = ap.parse_args(argv)
    tasks = convert.plan(convert.collect(a.inputs, paa.IMAGE_EXTENSIONS), a.output, ".paa")
    if not tasks:
        print(tr("Нет подходящих картинок."), file=sys.stderr)
        return 1
    res = convert.run(tasks, convert.image_to_paa_task(a.format, "nearest" if a.resize else "error",
                                                         not a.no_overwrite), a.threads, _print_result)
    ok = sum(r.ok for r in res)
    print(tr("Готово: {0} из {1}").format(ok, len(res)))
    return 0 if ok == len(res) else 1


def cli_png(argv: List[str]) -> int:
    ap = argparse.ArgumentParser(prog="DayZModToolkit png", description="PAA -> PNG/TGA.")
    ap.add_argument("inputs", nargs="+", help=tr("файлы .paa и/или папки"))
    ap.add_argument("-o", "--output", default="")
    ap.add_argument("--tga", action="store_true", help=tr("сохранять в TGA вместо PNG"))
    ap.add_argument("--no-overwrite", action="store_true")
    ap.add_argument("-j", "--threads", type=int, default=max(1, min(4, os.cpu_count() or 2)))
    a = ap.parse_args(argv)
    tasks = convert.plan(convert.collect(a.inputs, {".paa"}), a.output, ".tga" if a.tga else ".png")
    if not tasks:
        print(tr("Нет файлов .paa."), file=sys.stderr)
        return 1
    res = convert.run(tasks, convert.paa_to_image_task(not a.no_overwrite), a.threads, _print_result)
    ok = sum(r.ok for r in res)
    print(tr("Готово: {0} из {1}").format(ok, len(res)))
    return 0 if ok == len(res) else 1


def cli_config(argv: List[str], mode: str) -> int:
    to_bin = mode == "rapify"
    ap = argparse.ArgumentParser(prog=f"DayZModToolkit {mode}",
                                 description="config.cpp -> config.bin" if to_bin else "config.bin -> config.cpp")
    ap.add_argument("inputs", nargs="+")
    ap.add_argument("-o", "--output", default="", help=tr("папка вывода (по умолчанию рядом с исходником)"))
    ap.add_argument("--overwrite", action="store_true", help=tr("перезаписывать существующий файл"))
    if to_bin:
        ap.add_argument("--no-check", action="store_true", help=tr("не останавливаться на смысловых ошибках"))
    a = ap.parse_args(argv)
    exts = {".cpp"} if to_bin else {".bin"}
    inputs = convert.collect(a.inputs, exts)
    tasks = convert.plan(inputs, a.output, ".bin" if to_bin else ".cpp")
    if not tasks:
        print(tr("Нет подходящих файлов."), file=sys.stderr)
        return 1
    res = convert.run(tasks, convert.config_task(a.overwrite, not getattr(a, "no_check", False)), 1, _print_result)
    ok = sum(r.ok for r in res)
    print(tr("Готово: {0} из {1}").format(ok, len(res)))
    return 0 if ok == len(res) else 1


def cli_check(argv: List[str]) -> int:
    ap = argparse.ArgumentParser(prog="DayZModToolkit check",
                                 description=tr("Проверка мода/файлов на синтаксические и смысловые ошибки."))
    ap.add_argument("inputs", nargs="+", help=tr("папка мода и/или отдельные файлы"))
    ap.add_argument("--errors-only", action="store_true", help=tr("показывать только ошибки"))
    ap.add_argument("--no-info", action="store_true", help=tr("скрыть информационные сообщения"))
    ap.add_argument("--no-cross", action="store_true", help=tr("без проверки ссылок между файлами"))
    ap.add_argument("--report", default="", help=tr("сохранить отчёт в файл"))
    ap.add_argument("--ffmpeg", default="", help=tr("путь к ffmpeg (для проверки .ogg)"))
    ap.add_argument("--game", action="append", default=[],
                    help=tr("папка DayZ (или P:\\scripts, или другой мод) — проверки modded/override по игре; можно указать несколько раз"))
    ap.add_argument("--watch", action="store_true", help=tr("следить за изменениями и перепроверять"))
    a = ap.parse_args(argv)
    ffmpeg = find_ffmpeg(a.ffmpeg)
    game = None
    if a.game:
        from . import vanilla
        game = vanilla.load_index(a.game, lambda m: print("  " + m))
    if a.watch:
        return _watch(a, ffmpeg, game)
    return _check_once(a, ffmpeg, game)


def _watch(a, ffmpeg, game) -> int:
    import time
    last = None
    print(tr("Режим наблюдения: Ctrl+C для выхода."))
    try:
        while True:
            snap = checks.snapshot(a.inputs)
            if snap != last:
                last = snap
                print("\n=== " + time.strftime("%H:%M:%S") + " ===")
                _check_once(a, ffmpeg, game)
            time.sleep(1.5)
    except KeyboardInterrupt:
        return 0


def _check_once(a, ffmpeg, game) -> int:
    rep = checks.check_paths(a.inputs, ffmpeg, cross=not a.no_cross, game=game)
    shown = [i for i in rep.issues
             if not (a.errors_only and i.level != "error") and not (a.no_info and i.level == "info")]
    lines = [i.format() for i in shown]
    summary = (tr("Проверено файлов: {0}. Ошибок: {1}, предупреждений: {2}, замечаний: {3}.").format(rep.files, rep.count('error'), rep.count('warning'), rep.count('info')))
    for ln in lines:
        print(ln)
    print(summary)
    if a.report:
        Path(a.report).write_text("\n".join(lines + [summary]) + "\n", encoding="utf-8")
        print(tr("Отчёт: {0}").format(a.report))
    return 1 if rep.count("error") else 0


def cli_pack(argv: List[str]) -> int:
    from . import pbo
    ap = argparse.ArgumentParser(prog="DayZModToolkit pack", description=tr("Сборка папки мода в PBO."))
    ap.add_argument("source", help=tr("папка мода (где лежит config.cpp)"))
    ap.add_argument("-o", "--output", default="", help=tr("куда положить результат (по умолчанию рядом с папкой)"))
    ap.add_argument("--prefix", default="", help=tr("prefix внутри PBO (по умолчанию из $PBOPREFIX$ или имени папки)"))
    ap.add_argument("--key", default="", help=tr("подписать закрытым ключом .biprivatekey"))
    ap.add_argument("--no-check", action="store_true", help=tr("не проверять ошибки перед сборкой"))
    ap.add_argument("--force", action="store_true", help=tr("собирать даже при найденных ошибках"))
    ap.add_argument("--no-rapify", action="store_true", help=tr("не бинаризовать config.cpp"))
    ap.add_argument("--convert-images", action="store_true", help=tr("PNG/TGA без парного .paa конвертировать в PAA"))
    ap.add_argument("--flat", action="store_true", help=tr("класть .pbo прямо в папку вывода (без @Мод/Addons)"))
    ap.add_argument("--exclude", default=pbo.DEFAULT_EXCLUDE, help=tr("маски исключений через ;"))
    a = ap.parse_args(argv)
    src = Path(a.source)
    opts = pbo.BuildOptions(prefix=a.prefix, exclude=a.exclude, rapify=not a.no_rapify, check=not a.no_check,
                            stop_on_errors=not a.force, convert_images=a.convert_images, mod_layout=not a.flat,
                            private_key=a.key)
    res = pbo.build(src, a.output or src.resolve().parent, opts, lambda m, lvl="": print(m), find_ffmpeg())
    if not res.ok:
        print(tr("ОШИБКА: {0}").format(res.message), file=sys.stderr)
        return 1
    print(tr("Готово: {0}").format(res.pbo))
    return 0


def cli_unpack(argv: List[str]) -> int:
    from . import pbo
    ap = argparse.ArgumentParser(prog="DayZModToolkit unpack", description=tr("Распаковка PBO."))
    ap.add_argument("inputs", nargs="+", help=tr("файлы .pbo или папки с ними"))
    ap.add_argument("-o", "--output", default="", help=tr("папка вывода (по умолчанию рядом с PBO)"))
    ap.add_argument("--no-derapify", action="store_true", help=tr("не создавать config.cpp из config.bin"))
    a = ap.parse_args(argv)
    files = [f for f, _ in convert.collect(a.inputs, {".pbo"})]
    if not files:
        print(tr("Нет файлов .pbo."), file=sys.stderr)
        return 1
    rc = 0
    for f in files:
        try:
            out, arc = pbo.unpack(f, a.output or f.parent, not a.no_derapify)
            print(tr("OK  {0} -> {1}  ({2} файлов, prefix={3})").format(f.name, out, len(arc.entries), arc.prefix or '-'))
        except Exception as e:
            print(f"ERR {f.name}: {e}")
            rc = 1
    return rc


def cli_keygen(argv: List[str]) -> int:
    from . import sign
    ap = argparse.ArgumentParser(prog="DayZModToolkit keygen", description=tr("Создание пары ключей BI."))
    ap.add_argument("name", help=tr("имя ключа (латиница, без пробелов), напр. MyMod"))
    ap.add_argument("-o", "--output", default=".", help=tr("папка для ключей"))
    a = ap.parse_args(argv)
    try:
        priv, pub = sign.create_keys(a.name, a.output)
    except sign.SignError as e:
        print(tr("ОШИБКА: {0}").format(e), file=sys.stderr)
        return 1
    print(tr("Закрытый ключ (никому не передавайте!): {0}").format(priv))
    print(tr("Открытый ключ (кладётся в Keys мода и на сервер): {0}").format(pub))
    return 0


def cli_sign(argv: List[str]) -> int:
    from . import sign
    ap = argparse.ArgumentParser(prog="DayZModToolkit sign", description=tr("Подпись PBO."))
    ap.add_argument("inputs", nargs="+", help=tr("файлы .pbo или папки"))
    ap.add_argument("--key", required=True, help=tr("закрытый ключ .biprivatekey"))
    ap.add_argument("--version", type=int, choices=[2, 3], default=3)
    a = ap.parse_args(argv)
    key = sign.PrivateKey.load(a.key)
    for f, _ in convert.collect(a.inputs, {".pbo"}):
        print(f"OK  {sign.sign_pbo(f, key, a.version)}")
    return 0


def cli_verify(argv: List[str]) -> int:
    from . import sign
    ap = argparse.ArgumentParser(prog="DayZModToolkit verify", description=tr("Проверка подписей PBO."))
    ap.add_argument("inputs", nargs="+", help=tr("файлы .pbo или папки"))
    ap.add_argument("--key", default="", help=tr("доверенный открытый ключ .bikey"))
    a = ap.parse_args(argv)
    rc = 0
    for f, _ in convert.collect(a.inputs, {".pbo"}):
        sigs = sorted(f.parent.glob(f.name + ".*.bisign"))
        if not sigs:
            print(tr("ERR {0}: нет файла .bisign").format(f.name))
            rc = 1
        for s_ in sigs:
            ok, msg = sign.verify_pbo(f, s_, a.key or None)
            print(("OK  " if ok else "ERR ") + f"{s_.name}: {msg}")
            rc |= 0 if ok else 1
    return rc


def cli_types(argv: List[str]) -> int:
    from . import typesgen
    ap = argparse.ArgumentParser(prog="DayZModToolkit types",
                                 description=tr("Записи types.xml для предметов мода (scope = 2)."))
    ap.add_argument("inputs", nargs="+", help=tr("config.cpp/config.bin или папки модов"))
    ap.add_argument("-o", "--output", default="", help=tr("куда записать (по умолчанию вывод в консоль)"))
    ap.add_argument("--merge", default="", help=tr("дополнить существующий types.xml недостающими типами"))
    ap.add_argument("--sort", action="store_true", help=tr("отсортировать результат по имени"))
    a = ap.parse_args(argv)
    items = typesgen.from_paths(a.inputs)
    if not items:
        print(tr("Не найдено предметов со scope = 2."), file=sys.stderr)
        return 1
    if a.merge:
        text, n = typesgen.merge(Path(a.merge).read_text(encoding="utf-8-sig"), items)
        print(tr("Добавлено типов: {0} из {1}").format(n, len(items)), file=sys.stderr)
    else:
        text = typesgen.generate(items)
    if a.sort:
        text = typesgen.sort_types(text)
    if a.output:
        Path(a.output).write_text(text, encoding="utf-8")
        print(tr("Записано: {0} ({1} предметов)").format(a.output, len(items)))
    else:
        print(text)
    return 0


def cli(argv: List[str]) -> int:
    _setup_console()
    if argv and argv[0] in ("-h", "--help"):
        print(tr(USAGE))
        return 0
    if argv and argv[0] in ("-V", "--version"):
        print(f"{APP_NAME} {APP_VERSION}")
        return 0
    if argv and argv[0] in SUBCOMMANDS:
        cmd, rest = argv[0], argv[1:]
        if cmd == "audio":
            return cli_audio(rest)
        if cmd == "paa":
            return cli_paa(rest)
        if cmd == "png":
            return cli_png(rest)
        if cmd in ("rapify", "derapify"):
            return cli_config(rest, cmd)
        handlers = {"check": cli_check, "pack": cli_pack, "unpack": cli_unpack, "keygen": cli_keygen,
                    "sign": cli_sign, "verify": cli_verify, "types": cli_types}
        return handlers[cmd](rest)
    return cli_audio(argv)
