# -*- coding: utf-8 -*-
"""Консольный режим.

    DayZModToolkit --cli audio    <файлы/папки> [опции]   звук -> OGG
    DayZModToolkit --cli paa      <картинки>    [опции]   PNG/TGA/JPG -> PAA
    DayZModToolkit --cli png      <paa>         [опции]   PAA -> PNG
    DayZModToolkit --cli rapify   <config.cpp>  [опции]   config.cpp -> config.bin
    DayZModToolkit --cli derapify <config.bin>  [опции]   config.bin -> config.cpp
    DayZModToolkit --cli check    <файлы/папки> [опции]   проверка на ошибки

Без подкоманды (--cli <файлы>) выполняется конвертация звука, как в прежних версиях.
"""

from __future__ import annotations

import argparse
import os
import sys
from pathlib import Path
from typing import List

from . import APP_NAME, APP_VERSION, checks, convert, paa
from .audio import (NORMALIZE_MODES, PRESETS, ConvertSettings, JobResult, collect_inputs, convert_all,
                    plan_jobs, write_config)
from .common import check_vorbis, find_ffmpeg, human_size

SUBCOMMANDS = ("audio", "paa", "png", "rapify", "derapify", "check")


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
                stream.reconfigure(errors="replace")
            except Exception:
                pass




def cli_audio(argv: List[str]) -> int:
    ap = argparse.ArgumentParser(
        prog="DayZModToolkit audio",
        description=f"{APP_NAME} {APP_VERSION}: конвертация MP3/WAV/... в OGG Vorbis для DayZ.")
    ap.add_argument("inputs", nargs="+", help="файлы и/или папки (папки обходятся рекурсивно)")
    ap.add_argument("-o", "--output", default="", help="папка вывода (по умолчанию: подпапка 'ogg' рядом с исходником)")
    ap.add_argument("-p", "--preset", choices=list(PRESETS), help="пресет настроек")
    ap.add_argument("--mono", action="store_true", help="моно (для 3D звуков)")
    ap.add_argument("--stereo", action="store_true", help="стерео (музыка/UI)")
    ap.add_argument("-r", "--rate", type=int, choices=[22050, 32000, 44100, 48000], help="частота дискретизации")
    ap.add_argument("-q", "--quality", type=int, choices=range(-1, 11), metavar="{-1..10}", help="качество Vorbis")
    ap.add_argument("--volume", type=float, default=0.0, help="усиление, дБ")
    ap.add_argument("--normalize", choices=list(NORMALIZE_MODES), default="none")
    ap.add_argument("--peak", type=float, default=-1.0, help="цель пиковой нормализации, dBFS")
    ap.add_argument("--lufs", type=float, default=-16.0, help="цель нормализации громкости, LUFS")
    ap.add_argument("--trim-silence", action="store_true", help="обрезать тишину в начале и конце")
    ap.add_argument("--fade-in", type=float, default=0.0)
    ap.add_argument("--fade-out", type=float, default=0.0)
    ap.add_argument("--start", type=float, default=0.0, help="начать с N секунд")
    ap.add_argument("--duration", type=float, default=0.0, help="взять только N секунд")
    ap.add_argument("--no-sanitize", action="store_true", help="не переименовывать файлы в безопасные имена")
    ap.add_argument("--flat", action="store_true", help="не сохранять структуру подпапок")
    ap.add_argument("--prefix", default="", help="префикс имени файлов")
    ap.add_argument("--no-overwrite", action="store_true", help="пропускать уже существующие файлы")
    ap.add_argument("-j", "--threads", type=int, default=ConvertSettings().threads)
    ap.add_argument("--config", action="store_true", help="сгенерировать config.cpp")
    ap.add_argument("--mod", default="MyMod", help="имя мода (CfgPatches)")
    ap.add_argument("--sound-path", default="", help=r"путь к звукам в PBO, напр. MyMod\sounds")
    ap.add_argument("--class-prefix", default="", help="префикс классов SoundShader/SoundSet")
    ap.add_argument("--range", type=int, help="дальность слышимости, м")
    ap.add_argument("--loop", action="store_true", help="loop = 1 в SoundSet")
    ap.add_argument("--no-spatial", action="store_true", help="2D звук (spatial = 0)")
    ap.add_argument("--ffmpeg", default="", help="путь к ffmpeg")
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
        print("ОШИБКА: ffmpeg не найден. Положите ffmpeg.exe рядом с программой, добавьте в PATH "
              "или установите: pip install imageio-ffmpeg", file=sys.stderr)
        return 2
    if not check_vorbis(ffmpeg):
        print(f"ОШИБКА: ffmpeg ({ffmpeg}) собран без libvorbis.", file=sys.stderr)
        return 2

    inputs = collect_inputs(a.inputs, s.output_dir)
    if not inputs:
        print("Нет подходящих аудиофайлов.", file=sys.stderr)
        return 1
    jobs = plan_jobs(inputs, s)
    print(f"ffmpeg: {ffmpeg}")
    print(f"Файлов: {len(jobs)} | {'моно' if s.channels == 1 else 'стерео'}, {s.sample_rate} Гц, q={s.quality}")

    def on_result(r: JobResult, done: int, total: int) -> None:
        status = "OK " if r.ok else "ERR"
        extra = f"{human_size(r.size)}, {r.duration:.2f} c" if r.ok and r.duration else r.message
        print(f"[{done}/{total}] {status} {r.job.src.name} -> {r.job.dst}  ({extra})", flush=True)

    results = convert_all(jobs, s, ffmpeg, on_result)
    ok = sum(1 for r in results if r.ok)
    print(f"Готово: {ok} из {len(results)}")
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
    ap = argparse.ArgumentParser(prog="DayZModToolkit paa", description="Картинки (PNG/TGA/JPG/BMP) -> PAA.")
    ap.add_argument("inputs", nargs="+", help="картинки и/или папки")
    ap.add_argument("-o", "--output", default="", help="папка вывода (по умолчанию рядом с исходником)")
    ap.add_argument("-f", "--format", choices=["auto", "dxt1", "dxt5"], default="auto",
                    help="auto: DXT5 при прозрачности или по суффиксу _ca/_co, иначе DXT1")
    ap.add_argument("--resize", action="store_true", help="масштабировать до степени двойки")
    ap.add_argument("--no-overwrite", action="store_true")
    ap.add_argument("-j", "--threads", type=int, default=max(1, min(4, os.cpu_count() or 2)))
    a = ap.parse_args(argv)
    tasks = convert.plan(convert.collect(a.inputs, paa.IMAGE_EXTENSIONS), a.output, ".paa")
    if not tasks:
        print("Нет подходящих картинок.", file=sys.stderr)
        return 1
    res = convert.run(tasks, convert.image_to_paa_task(a.format, "nearest" if a.resize else "error",
                                                         not a.no_overwrite), a.threads, _print_result)
    ok = sum(r.ok for r in res)
    print(f"Готово: {ok} из {len(res)}")
    return 0 if ok == len(res) else 1


def cli_png(argv: List[str]) -> int:
    ap = argparse.ArgumentParser(prog="DayZModToolkit png", description="PAA -> PNG/TGA.")
    ap.add_argument("inputs", nargs="+", help="файлы .paa и/или папки")
    ap.add_argument("-o", "--output", default="")
    ap.add_argument("--tga", action="store_true", help="сохранять в TGA вместо PNG")
    ap.add_argument("--no-overwrite", action="store_true")
    ap.add_argument("-j", "--threads", type=int, default=max(1, min(4, os.cpu_count() or 2)))
    a = ap.parse_args(argv)
    tasks = convert.plan(convert.collect(a.inputs, {".paa"}), a.output, ".tga" if a.tga else ".png")
    if not tasks:
        print("Нет файлов .paa.", file=sys.stderr)
        return 1
    res = convert.run(tasks, convert.paa_to_image_task(not a.no_overwrite), a.threads, _print_result)
    ok = sum(r.ok for r in res)
    print(f"Готово: {ok} из {len(res)}")
    return 0 if ok == len(res) else 1


def cli_config(argv: List[str], mode: str) -> int:
    to_bin = mode == "rapify"
    ap = argparse.ArgumentParser(prog=f"DayZModToolkit {mode}",
                                 description="config.cpp -> config.bin" if to_bin else "config.bin -> config.cpp")
    ap.add_argument("inputs", nargs="+")
    ap.add_argument("-o", "--output", default="", help="папка вывода (по умолчанию рядом с исходником)")
    ap.add_argument("--overwrite", action="store_true", help="перезаписывать существующий файл")
    if to_bin:
        ap.add_argument("--no-check", action="store_true", help="не останавливаться на смысловых ошибках")
    a = ap.parse_args(argv)
    exts = {".cpp"} if to_bin else {".bin"}
    inputs = convert.collect(a.inputs, exts)
    tasks = convert.plan(inputs, a.output, ".bin" if to_bin else ".cpp")
    if not tasks:
        print("Нет подходящих файлов.", file=sys.stderr)
        return 1
    res = convert.run(tasks, convert.config_task(a.overwrite, not getattr(a, "no_check", False)), 1, _print_result)
    ok = sum(r.ok for r in res)
    print(f"Готово: {ok} из {len(res)}")
    return 0 if ok == len(res) else 1


def cli_check(argv: List[str]) -> int:
    ap = argparse.ArgumentParser(prog="DayZModToolkit check",
                                 description="Проверка мода/файлов на синтаксические и смысловые ошибки.")
    ap.add_argument("inputs", nargs="+", help="папка мода и/или отдельные файлы")
    ap.add_argument("--errors-only", action="store_true", help="показывать только ошибки")
    ap.add_argument("--no-info", action="store_true", help="скрыть информационные сообщения")
    ap.add_argument("--no-cross", action="store_true", help="без проверки ссылок между файлами")
    ap.add_argument("--report", default="", help="сохранить отчёт в файл")
    ap.add_argument("--ffmpeg", default="", help="путь к ffmpeg (для проверки .ogg)")
    a = ap.parse_args(argv)
    ffmpeg = find_ffmpeg(a.ffmpeg)
    rep = checks.check_paths(a.inputs, ffmpeg, cross=not a.no_cross)
    shown = [i for i in rep.issues
             if not (a.errors_only and i.level != "error") and not (a.no_info and i.level == "info")]
    lines = [i.format() for i in shown]
    summary = (f"Проверено файлов: {rep.files}. Ошибок: {rep.count('error')}, "
               f"предупреждений: {rep.count('warning')}, замечаний: {rep.count('info')}.")
    for ln in lines:
        print(ln)
    print(summary)
    if a.report:
        Path(a.report).write_text("\n".join(lines + [summary]) + "\n", encoding="utf-8")
        print(f"Отчёт: {a.report}")
    return 1 if rep.count("error") else 0


def cli(argv: List[str]) -> int:
    _setup_console()
    if argv and argv[0] in ("-h", "--help"):
        print(__doc__)
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
        return cli_check(rest)
    return cli_audio(argv)
