#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
DayZ OGG Converter
==================

Конвертер MP3 (и WAV/FLAC/M4A/AAC/WMA/OPUS/OGG) в OGG Vorbis, пригодный для DayZ,
плюс генератор config.cpp (CfgSoundShaders / CfgSoundSets) под сконвертированные файлы.

Запуск:
    python dayz_ogg_converter.py              -> графический интерфейс
    python dayz_ogg_converter.py --help       -> консольный режим (см. справку)

Требуется ffmpeg (ищется рядом с программой, в PATH или через пакет imageio-ffmpeg).
"""

from __future__ import annotations

import argparse
import json
import os
import re
import shutil
import subprocess
import sys
import threading
from concurrent.futures import ThreadPoolExecutor, as_completed
from dataclasses import asdict, dataclass, field
from pathlib import Path
from typing import Callable, Iterable, List, Optional

APP_NAME = "DayZ OGG Converter"
APP_VERSION = "1.0.0"

INPUT_EXTENSIONS = {".mp3", ".wav", ".flac", ".m4a", ".aac", ".wma", ".ogg", ".opus", ".webm", ".mp4"}

# --------------------------------------------------------------------------------------
# Пресеты
# --------------------------------------------------------------------------------------

PRESETS = {
    "3D звук (моно)": dict(channels=1, sample_rate=44100, quality=6, spatial=True, loop=False, range=50),
    "Музыка / радио (стерео)": dict(channels=2, sample_rate=44100, quality=7, spatial=False, loop=False, range=100),
    "UI / 2D звук (стерео)": dict(channels=2, sample_rate=44100, quality=6, spatial=False, loop=False, range=10),
    "Эмбиент, петля (моно)": dict(channels=1, sample_rate=44100, quality=5, spatial=True, loop=True, range=150),
    "Выстрел / громкий 3D (моно)": dict(channels=1, sample_rate=44100, quality=7, spatial=True, loop=False, range=800),
}
DEFAULT_PRESET = "3D звук (моно)"

NORMALIZE_MODES = {
    "none": "Без нормализации",
    "peak": "По пику (dBFS)",
    "loudness": "По громкости (LUFS, EBU R128)",
}


# --------------------------------------------------------------------------------------
# Настройки
# --------------------------------------------------------------------------------------

@dataclass
class ConvertSettings:
    # аудио
    channels: int = 1                 # 1 = моно (3D позиционный звук), 2 = стерео
    sample_rate: int = 44100
    quality: int = 6                  # качество Vorbis -1..10 (≈ 6 -> ~192 кбит/с стерео)
    volume_db: float = 0.0            # ручное усиление, дБ
    normalize: str = "none"           # none | peak | loudness
    peak_target_db: float = -1.0
    loudness_target: float = -16.0    # LUFS
    trim_silence: bool = False
    silence_threshold_db: float = -50.0
    fade_in: float = 0.0              # сек
    fade_out: float = 0.0             # сек
    cut_start: float = 0.0            # сек, откуда начинать
    cut_duration: float = 0.0         # сек, 0 = до конца
    # файлы
    output_dir: str = ""
    keep_structure: bool = True
    sanitize_names: bool = True
    name_prefix: str = ""
    overwrite: bool = True
    threads: int = max(1, min(4, (os.cpu_count() or 2)))
    # config.cpp
    generate_config: bool = True
    mod_name: str = "MyMod"
    sound_path: str = r"MyMod\sounds"  # путь внутри PBO (без P:\)
    class_prefix: str = "MyMod"
    spatial: bool = True
    loop: bool = False
    range: int = 50
    shader_volume: float = 1.0
    # ffmpeg
    ffmpeg_path: str = ""

    @classmethod
    def from_dict(cls, data: dict) -> "ConvertSettings":
        known = {f for f in cls.__dataclass_fields__}
        return cls(**{k: v for k, v in data.items() if k in known})


def settings_file() -> Path:
    base = os.environ.get("APPDATA") or os.path.join(Path.home(), ".config")
    return Path(base) / "DayZOggConverter" / "settings.json"


def load_settings() -> ConvertSettings:
    try:
        return ConvertSettings.from_dict(json.loads(settings_file().read_text(encoding="utf-8")))
    except Exception:
        return ConvertSettings()


def save_settings(s: ConvertSettings) -> None:
    try:
        p = settings_file()
        p.parent.mkdir(parents=True, exist_ok=True)
        p.write_text(json.dumps(asdict(s), ensure_ascii=False, indent=2), encoding="utf-8")
    except Exception:
        pass


# --------------------------------------------------------------------------------------
# ffmpeg
# --------------------------------------------------------------------------------------

def app_dir() -> Path:
    if getattr(sys, "frozen", False):  # PyInstaller
        return Path(sys.executable).resolve().parent
    return Path(__file__).resolve().parent


def find_ffmpeg(custom: str = "") -> Optional[str]:
    """Ищет ffmpeg: указанный путь -> рядом с программой -> PATH -> imageio-ffmpeg."""
    exe = "ffmpeg.exe" if os.name == "nt" else "ffmpeg"
    candidates = []
    if custom:
        candidates.append(Path(custom))
    candidates += [app_dir() / exe, app_dir() / "ffmpeg" / exe, app_dir() / "ffmpeg" / "bin" / exe]
    if getattr(sys, "frozen", False) and hasattr(sys, "_MEIPASS"):
        candidates.append(Path(sys._MEIPASS) / exe)  # type: ignore[attr-defined]
    for c in candidates:
        if c.is_file():
            return str(c)
    found = shutil.which("ffmpeg")
    if found:
        return found
    try:
        import imageio_ffmpeg  # type: ignore
        return imageio_ffmpeg.get_ffmpeg_exe()
    except Exception:
        return None


def _popen_kwargs() -> dict:
    kw: dict = dict(stdout=subprocess.PIPE, stderr=subprocess.PIPE, stdin=subprocess.DEVNULL)
    if os.name == "nt":
        kw["creationflags"] = getattr(subprocess, "CREATE_NO_WINDOW", 0)
    return kw


def run_ffmpeg(args: List[str]) -> subprocess.CompletedProcess:
    p = subprocess.run(args, **_popen_kwargs())
    p.stdout = p.stdout.decode("utf-8", "replace") if isinstance(p.stdout, bytes) else p.stdout
    p.stderr = p.stderr.decode("utf-8", "replace") if isinstance(p.stderr, bytes) else p.stderr
    return p


def check_vorbis(ffmpeg: str) -> bool:
    p = run_ffmpeg([ffmpeg, "-hide_banner", "-encoders"])
    return "libvorbis" in (p.stdout or "")


def probe_duration(ffmpeg: str, path: Path) -> Optional[float]:
    p = run_ffmpeg([ffmpeg, "-hide_banner", "-i", str(path)])
    m = re.search(r"Duration:\s*(\d+):(\d+):(\d+(?:\.\d+)?)", p.stderr or "")
    if not m:
        return None
    h, mi, se = m.groups()
    return int(h) * 3600 + int(mi) * 60 + float(se)


# --------------------------------------------------------------------------------------
# Имена файлов
# --------------------------------------------------------------------------------------

_TRANSLIT = {
    "а": "a", "б": "b", "в": "v", "г": "g", "д": "d", "е": "e", "ё": "e", "ж": "zh", "з": "z",
    "и": "i", "й": "y", "к": "k", "л": "l", "м": "m", "н": "n", "о": "o", "п": "p", "р": "r",
    "с": "s", "т": "t", "у": "u", "ф": "f", "х": "h", "ц": "ts", "ч": "ch", "ш": "sh", "щ": "sch",
    "ъ": "", "ы": "y", "ь": "", "э": "e", "ю": "yu", "я": "ya",
    "і": "i", "ї": "yi", "є": "ye", "ґ": "g",
}


def sanitize_name(name: str) -> str:
    """Делает имя безопасным для DayZ/PBO: латиница, нижний регистр, цифры и '_'."""
    name = name.lower()
    name = "".join(_TRANSLIT.get(ch, ch) for ch in name)
    name = re.sub(r"[^a-z0-9_]+", "_", name)
    name = re.sub(r"_+", "_", name).strip("_")
    if not name:
        name = "sound"
    if name[0].isdigit():
        name = "s_" + name
    return name


def class_safe(name: str) -> str:
    """Имя, пригодное для класса в config.cpp."""
    n = re.sub(r"[^A-Za-z0-9_]+", "_", name)
    if not n or n[0].isdigit():
        n = "_" + n
    return n


# --------------------------------------------------------------------------------------
# Конвертация
# --------------------------------------------------------------------------------------

@dataclass
class Job:
    src: Path
    dst: Path
    root: Optional[Path] = None


@dataclass
class JobResult:
    job: Job
    ok: bool
    message: str = ""
    duration: Optional[float] = None
    size: int = 0


def _is_within(path: Path, parent: Path) -> bool:
    try:
        path.resolve().relative_to(parent.resolve())
        return True
    except ValueError:
        return False


def collect_inputs(paths: Iterable[str], output_dir: str = "") -> List[tuple]:
    """Возвращает список (файл, корневая_папка_или_None).

    Из обхода папок исключаются папки вывода (указанная и «ogg» по умолчанию),
    чтобы уже сконвертированные файлы не попадали в очередь повторно.
    """
    out = []
    seen = set()
    for raw in paths:
        p = Path(raw)
        if p.is_dir():
            excluded = [p / "ogg"] + ([Path(output_dir)] if output_dir else [])
            for f in sorted(p.rglob("*")):
                if any(_is_within(f, e) for e in excluded):
                    continue
                if f.is_file() and f.suffix.lower() in INPUT_EXTENSIONS:
                    key = str(f.resolve())
                    if key not in seen:
                        seen.add(key)
                        out.append((f, p))
        elif p.is_file() and p.suffix.lower() in INPUT_EXTENSIONS:
            key = str(p.resolve())
            if key not in seen:
                seen.add(key)
                out.append((p, None))
    return out


def plan_jobs(inputs: List[tuple], s: ConvertSettings) -> List[Job]:
    jobs: List[Job] = []
    used = set()
    for src, root in inputs:
        stem = src.stem
        stem = sanitize_name(stem) if s.sanitize_names else stem
        if s.name_prefix:
            stem = (sanitize_name(s.name_prefix) if s.sanitize_names else s.name_prefix) + "_" + stem

        if s.output_dir:
            out_dir = Path(s.output_dir)
        elif root is not None:
            out_dir = root / "ogg"
        else:
            out_dir = src.parent / "ogg"
        if s.keep_structure and root is not None:
            rel = src.parent.relative_to(root)
            parts = [sanitize_name(x) if s.sanitize_names else x for x in rel.parts]
            out_dir = out_dir.joinpath(*parts) if parts else out_dir

        dst = out_dir / f"{stem}.ogg"
        n = 2
        while str(dst).lower() in used:
            dst = out_dir / f"{stem}_{n}.ogg"
            n += 1
        used.add(str(dst).lower())
        jobs.append(Job(src=src, dst=dst, root=root))
    return jobs


def _detect_peak(ffmpeg: str, src: Path, pre_filters: List[str], input_opts: List[str]) -> Optional[float]:
    af = ",".join(pre_filters + ["volumedetect"])
    p = run_ffmpeg([ffmpeg, "-hide_banner", "-nostats", *input_opts, "-i", str(src),
                    "-vn", "-af", af, "-f", "null", "-"])
    m = re.search(r"max_volume:\s*(-?[\d.]+)\s*dB", p.stderr or "")
    return float(m.group(1)) if m else None


def build_filters(s: ConvertSettings) -> List[str]:
    """Фильтры до нормализации (обрезка тишины, фейды)."""
    f: List[str] = []
    if s.trim_silence:
        th = f"{s.silence_threshold_db}dB"
        f += [f"silenceremove=start_periods=1:start_threshold={th}:start_silence=0.05",
              "areverse",
              f"silenceremove=start_periods=1:start_threshold={th}:start_silence=0.05",
              "areverse"]
    if s.fade_in > 0:
        f.append(f"afade=t=in:st=0:d={s.fade_in}")
    if s.fade_out > 0:
        f += ["areverse", f"afade=t=in:st=0:d={s.fade_out}", "areverse"]
    return f


def convert_one(ffmpeg: str, job: Job, s: ConvertSettings,
                cancel: Optional[threading.Event] = None) -> JobResult:
    if cancel is not None and cancel.is_set():
        return JobResult(job, False, "отменено")
    if job.dst.resolve() == job.src.resolve():
        return JobResult(job, False, "исходный файл совпадает с результатом — укажите другую папку вывода")
    if job.dst.exists() and not s.overwrite:
        return JobResult(job, True, "пропущен (уже существует)", size=job.dst.stat().st_size)

    job.dst.parent.mkdir(parents=True, exist_ok=True)

    input_opts: List[str] = []
    if s.cut_start > 0:
        input_opts += ["-ss", f"{s.cut_start}"]
    if s.cut_duration > 0:
        input_opts += ["-t", f"{s.cut_duration}"]

    filters = build_filters(s)
    gain = float(s.volume_db or 0.0)

    if s.normalize == "peak":
        peak = _detect_peak(ffmpeg, job.src, filters, input_opts)
        if peak is not None:
            gain += s.peak_target_db - peak
        filters_all = filters + ([f"volume={gain:.2f}dB"] if abs(gain) > 0.005 else [])
    elif s.normalize == "loudness":
        filters_all = filters + [f"loudnorm=I={s.loudness_target}:TP=-1.5:LRA=11"]
        if abs(gain) > 0.005:
            filters_all.append(f"volume={gain:.2f}dB")
    else:
        filters_all = filters + ([f"volume={gain:.2f}dB"] if abs(gain) > 0.005 else [])

    # после loudnorm частота может смениться -> принудительно задаём нужную
    filters_all.append(f"aresample={s.sample_rate}")

    tmp = job.dst.with_name(job.dst.stem + ".part.ogg")
    cmd = [ffmpeg, "-hide_banner", "-nostdin", "-y", "-loglevel", "error",
           *input_opts, "-i", str(job.src),
           "-vn", "-sn", "-dn", "-map", "0:a:0", "-map_metadata", "-1",
           "-af", ",".join(filters_all),
           "-ac", str(s.channels), "-ar", str(s.sample_rate),
           "-c:a", "libvorbis", "-q:a", str(s.quality),
           "-f", "ogg", str(tmp)]
    p = run_ffmpeg(cmd)
    if p.returncode != 0 or not tmp.exists() or tmp.stat().st_size == 0:
        try:
            tmp.unlink()
        except OSError:
            pass
        err = (p.stderr or "").strip().splitlines()
        return JobResult(job, False, err[-1] if err else f"ffmpeg завершился с кодом {p.returncode}")

    os.replace(tmp, job.dst)
    dur = probe_duration(ffmpeg, job.dst)
    return JobResult(job, True, "OK", duration=dur, size=job.dst.stat().st_size)


def convert_all(jobs: List[Job], s: ConvertSettings, ffmpeg: str,
                on_result: Optional[Callable[[JobResult, int, int], None]] = None,
                cancel: Optional[threading.Event] = None) -> List[JobResult]:
    results: List[JobResult] = []
    total = len(jobs)
    with ThreadPoolExecutor(max_workers=max(1, int(s.threads))) as ex:
        futs = {ex.submit(convert_one, ffmpeg, j, s, cancel): j for j in jobs}
        for fut in as_completed(futs):
            try:
                r = fut.result()
            except Exception as e:  # pragma: no cover
                r = JobResult(futs[fut], False, f"ошибка: {e}")
            results.append(r)
            if on_result:
                on_result(r, len(results), total)
    order = {id(j): i for i, j in enumerate(jobs)}
    results.sort(key=lambda r: order.get(id(r.job), 0))
    return results


# --------------------------------------------------------------------------------------
# Генерация config.cpp
# --------------------------------------------------------------------------------------

def _fmt_num(v: float) -> str:
    return ("%g" % float(v))


def output_root(results: List[JobResult], s: ConvertSettings) -> Optional[Path]:
    """Общая папка вывода: от неё считаются пути звуков в config.cpp."""
    if s.output_dir:
        return Path(s.output_dir)
    parents = [str(r.job.dst.parent) for r in results if r.ok]
    if not parents:
        return None
    try:
        return Path(os.path.commonpath(parents))
    except ValueError:  # разные диски
        return None


def generate_config(results: List[JobResult], s: ConvertSettings) -> str:
    ok = [r for r in results if r.ok]
    mod = class_safe(s.mod_name or "MyMod")
    pref = class_safe(s.class_prefix or mod)
    base_path = (s.sound_path or "").strip().strip("\\/").replace("/", "\\")
    out_root = output_root(results, s)

    lines: List[str] = []
    w = lines.append
    w("// Сгенерировано: %s %s" % (APP_NAME, APP_VERSION))
    w("// Путь к звукам в PBO: %s" % (base_path or "<не задан>"))
    w("// Воспроизведение в скрипте, например:")
    w("//   SEffectManager.PlaySound(\"%s_<имя>_SoundSet\", GetPosition());" % pref)
    w("")
    w("class CfgPatches")
    w("{")
    w("\tclass %s_Sounds" % mod)
    w("\t{")
    w("\t\tunits[] = {};")
    w("\t\tweapons[] = {};")
    w("\t\trequiredVersion = 0.1;")
    w("\t\trequiredAddons[] = {\"DZ_Data\", \"DZ_Sounds_Effects\"};")
    w("\t};")
    w("};")
    w("")
    w("class CfgSoundShaders")
    w("{")
    w("\tclass %s_SoundShader_Base" % pref)
    w("\t{")
    w("\t\tsamples[] = {};")
    w("\t\tfrequency = 1;")
    w("\t\trange = %d;" % int(s.range))
    w("\t\tvolume = %s;" % _fmt_num(s.shader_volume))
    w("\t};")

    names = []
    for r in ok:
        dst = r.job.dst
        if out_root is not None:
            try:
                rel = dst.relative_to(out_root)
            except ValueError:
                rel = Path(dst.name)
        else:
            rel = Path(dst.name)
        rel_noext = "\\".join(rel.with_suffix("").parts)
        sample = f"{base_path}\\{rel_noext}" if base_path else rel_noext
        name = class_safe("_".join(rel.with_suffix("").parts))
        names.append(name)
        dur = f"  // {r.duration:.2f} c" if r.duration else ""
        w("\tclass %s_%s_SoundShader: %s_SoundShader_Base%s" % (pref, name, pref, dur))
        w("\t{")
        w("\t\tsamples[] = {{\"%s\", 1}};" % sample.replace('"', ""))
        w("\t};")
    w("};")
    w("")
    w("class CfgSoundSets")
    w("{")
    w("\tclass %s_SoundSet_Base" % pref)
    w("\t{")
    if s.spatial:
        w("\t\tsound3DProcessingType = \"character3DProcessingType\";")
        w("\t\tvolumeCurve = \"characterAttenuationCurve\";")
        w("\t\tspatial = 1;")
    else:
        w("\t\tspatial = 0;")
    w("\t\tdoppler = 0;")
    w("\t\tloop = %d;" % (1 if s.loop else 0))
    w("\t};")
    for name in names:
        w("\tclass %s_%s_SoundSet: %s_SoundSet_Base" % (pref, name, pref))
        w("\t{")
        w("\t\tsoundShaders[] = {\"%s_%s_SoundShader\"};" % (pref, name))
        w("\t};")
    w("};")
    w("")
    return "\r\n".join(lines)


def write_config(results: List[JobResult], s: ConvertSettings) -> Optional[Path]:
    ok = [r for r in results if r.ok]
    if not ok:
        return None
    target_dir = output_root(results, s) or ok[0].job.dst.parent
    target_dir.mkdir(parents=True, exist_ok=True)
    path = target_dir / "config.cpp"
    path.write_text(generate_config(results, s), encoding="utf-8")
    return path


# --------------------------------------------------------------------------------------
# Утилиты
# --------------------------------------------------------------------------------------

def human_size(n: int) -> str:
    size = float(n)
    for unit in ("Б", "КБ", "МБ", "ГБ"):
        if size < 1024 or unit == "ГБ":
            return f"{size:.0f} {unit}" if unit == "Б" else f"{size:.1f} {unit}"
        size /= 1024
    return f"{n} Б"


def open_folder(path: Path) -> None:
    try:
        if os.name == "nt":
            os.startfile(str(path))  # type: ignore[attr-defined]
        elif sys.platform == "darwin":
            subprocess.Popen(["open", str(path)])
        else:
            subprocess.Popen(["xdg-open", str(path)])
    except Exception:
        pass


# --------------------------------------------------------------------------------------
# Консольный режим
# --------------------------------------------------------------------------------------

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


def cli(argv: List[str]) -> int:
    _setup_console()
    ap = argparse.ArgumentParser(
        prog="dayz_ogg_converter",
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
        cfg = write_config(results, s)
        if cfg:
            print(f"config.cpp: {cfg}")
    return 0 if ok == len(results) else 1


# --------------------------------------------------------------------------------------
# Графический интерфейс
# --------------------------------------------------------------------------------------

def gui() -> int:
    import tkinter as tk
    from tkinter import filedialog, messagebox, ttk

    s = load_settings()

    root = tk.Tk()
    root.title(f"{APP_NAME} {APP_VERSION}")
    root.geometry("1000x720")
    root.minsize(860, 600)
    style = ttk.Style()
    try:
        style.theme_use("clam")
    except tk.TclError:
        pass
    style.configure("Green.Horizontal.TProgressbar", background="#5a8f3c", troughcolor="#c9c6bd")

    files: List[str] = []
    cancel_event = threading.Event()
    state = {"busy": False}

    # ---- переменные ----
    v_preset = tk.StringVar(value=DEFAULT_PRESET)
    v_channels = tk.StringVar(value="Моно" if s.channels == 1 else "Стерео")
    v_rate = tk.StringVar(value=str(s.sample_rate))
    v_quality = tk.IntVar(value=s.quality)
    v_volume = tk.DoubleVar(value=s.volume_db)
    v_norm = tk.StringVar(value=NORMALIZE_MODES.get(s.normalize, NORMALIZE_MODES["none"]))
    v_peak = tk.DoubleVar(value=s.peak_target_db)
    v_lufs = tk.DoubleVar(value=s.loudness_target)
    v_trim = tk.BooleanVar(value=s.trim_silence)
    v_fin = tk.DoubleVar(value=s.fade_in)
    v_fout = tk.DoubleVar(value=s.fade_out)
    v_cstart = tk.DoubleVar(value=s.cut_start)
    v_cdur = tk.DoubleVar(value=s.cut_duration)
    v_out = tk.StringVar(value=s.output_dir)
    v_keep = tk.BooleanVar(value=s.keep_structure)
    v_sanitize = tk.BooleanVar(value=s.sanitize_names)
    v_prefix = tk.StringVar(value=s.name_prefix)
    v_overwrite = tk.BooleanVar(value=s.overwrite)
    v_threads = tk.IntVar(value=s.threads)
    v_gencfg = tk.BooleanVar(value=s.generate_config)
    v_mod = tk.StringVar(value=s.mod_name)
    v_spath = tk.StringVar(value=s.sound_path)
    v_cpref = tk.StringVar(value=s.class_prefix)
    v_spatial = tk.BooleanVar(value=s.spatial)
    v_loop = tk.BooleanVar(value=s.loop)
    v_range = tk.IntVar(value=s.range)
    v_svol = tk.DoubleVar(value=s.shader_volume)
    v_ffmpeg = tk.StringVar(value=s.ffmpeg_path)
    v_status = tk.StringVar(value="Добавьте файлы или папки.")

    def num(var, default):
        try:
            return var.get()
        except (tk.TclError, ValueError):
            return default

    def collect() -> ConvertSettings:
        norm_key = next((k for k, v in NORMALIZE_MODES.items() if v == v_norm.get()), "none")
        return ConvertSettings(
            channels=1 if v_channels.get() == "Моно" else 2,
            sample_rate=int(v_rate.get() or 44100),
            quality=int(num(v_quality, 6)),
            volume_db=float(num(v_volume, 0.0)),
            normalize=norm_key,
            peak_target_db=float(num(v_peak, -1.0)),
            loudness_target=float(num(v_lufs, -16.0)),
            trim_silence=v_trim.get(),
            fade_in=max(0.0, float(num(v_fin, 0.0))),
            fade_out=max(0.0, float(num(v_fout, 0.0))),
            cut_start=max(0.0, float(num(v_cstart, 0.0))),
            cut_duration=max(0.0, float(num(v_cdur, 0.0))),
            output_dir=v_out.get().strip(),
            keep_structure=v_keep.get(),
            sanitize_names=v_sanitize.get(),
            name_prefix=v_prefix.get().strip(),
            overwrite=v_overwrite.get(),
            threads=max(1, int(num(v_threads, 2))),
            generate_config=v_gencfg.get(),
            mod_name=v_mod.get().strip() or "MyMod",
            sound_path=v_spath.get().strip(),
            class_prefix=v_cpref.get().strip(),
            spatial=v_spatial.get(),
            loop=v_loop.get(),
            range=int(num(v_range, 50)),
            shader_volume=float(num(v_svol, 1.0)),
            ffmpeg_path=v_ffmpeg.get().strip(),
        )

    def apply_preset(*_):
        p = PRESETS.get(v_preset.get())
        if not p:
            return
        v_channels.set("Моно" if p["channels"] == 1 else "Стерео")
        v_rate.set(str(p["sample_rate"]))
        v_quality.set(p["quality"])
        v_spatial.set(p["spatial"])
        v_loop.set(p["loop"])
        v_range.set(p["range"])

    # ---- раскладка ----
    main = ttk.Frame(root, padding=8)
    main.pack(fill="both", expand=True)
    main.columnconfigure(0, weight=3)
    main.columnconfigure(1, weight=2)
    main.rowconfigure(0, weight=1)

    # левая часть: список файлов
    left = ttk.LabelFrame(main, text="Файлы", padding=6)
    left.grid(row=0, column=0, sticky="nsew", padx=(0, 6))
    left.columnconfigure(0, weight=1)
    left.rowconfigure(1, weight=1)

    btns = ttk.Frame(left)
    btns.grid(row=0, column=0, sticky="ew", pady=(0, 4))

    tree = ttk.Treeview(left, columns=("src", "dst", "status"), show="headings", selectmode="extended")
    tree.heading("src", text="Исходный файл")
    tree.heading("dst", text="Результат")
    tree.heading("status", text="Статус")
    tree.column("src", width=240)
    tree.column("dst", width=190)
    tree.column("status", width=90, anchor="center", stretch=False)
    tree.grid(row=1, column=0, sticky="nsew")
    sb = ttk.Scrollbar(left, orient="vertical", command=tree.yview)
    sb.grid(row=1, column=1, sticky="ns")
    tree.configure(yscrollcommand=sb.set)

    def refresh_tree():
        tree.delete(*tree.get_children())
        cur = collect()
        inputs = collect_inputs(files, cur.output_dir)
        jobs = plan_jobs(inputs, cur)
        base = output_root([JobResult(x, True) for x in jobs], cur)
        for j in jobs:
            shown = str(j.src.relative_to(j.root.parent)) if j.root is not None else j.src.name
            dst_shown = str(j.dst.relative_to(base)) if base and _is_within(j.dst, base) else j.dst.name
            tree.insert("", "end", iid=str(j.src.resolve()), values=(shown, dst_shown, "ожидает"))
        v_status.set(f"Файлов в очереди: {len(jobs)}")

    def add_files():
        sel = filedialog.askopenfilenames(
            title="Выберите аудиофайлы",
            filetypes=[("Аудио", " ".join("*" + e for e in sorted(INPUT_EXTENSIONS))), ("Все файлы", "*.*")])
        for f in sel:
            if f not in files:
                files.append(f)
        refresh_tree()

    def add_folder():
        d = filedialog.askdirectory(title="Выберите папку со звуками")
        if d and d not in files:
            files.append(d)
        refresh_tree()

    def remove_selected():
        sel = set(tree.selection())
        if not sel:
            return
        keep = []
        for f in files:
            if Path(f).is_file() and str(Path(f).resolve()) in sel:
                continue
            keep.append(f)
        # файлы из выбранных папок исключаем, добавляя остальные файлы папки поштучно
        final: List[str] = []
        for f in keep:
            if Path(f).is_dir():
                inner = [str(x) for x, _ in collect_inputs([f], v_out.get().strip())]
                if any(str(Path(x).resolve()) in sel for x in inner):
                    final += [x for x in inner if str(Path(x).resolve()) not in sel]
                    continue
            final.append(f)
        files[:] = final
        refresh_tree()

    def clear_all():
        files.clear()
        refresh_tree()

    ttk.Button(btns, text="+ Файлы", command=add_files).pack(side="left")
    ttk.Button(btns, text="+ Папка", command=add_folder).pack(side="left", padx=4)
    ttk.Button(btns, text="Удалить", command=remove_selected).pack(side="left")
    ttk.Button(btns, text="Очистить", command=clear_all).pack(side="left", padx=4)

    # лог
    logf = ttk.LabelFrame(left, text="Журнал", padding=4)
    logf.grid(row=2, column=0, columnspan=2, sticky="nsew", pady=(6, 0))
    logf.columnconfigure(0, weight=1)
    log = tk.Text(logf, height=8, wrap="word", state="disabled", font=("Consolas", 9))
    log.grid(row=0, column=0, sticky="nsew")
    lsb = ttk.Scrollbar(logf, orient="vertical", command=log.yview)
    lsb.grid(row=0, column=1, sticky="ns")
    log.configure(yscrollcommand=lsb.set)

    def write_log(text: str):
        log.configure(state="normal")
        log.insert("end", text + "\n")
        log.see("end")
        log.configure(state="disabled")

    # правая часть: настройки
    nb = ttk.Notebook(main)
    nb.grid(row=0, column=1, sticky="nsew")

    def row(parent, r, label, widget, hint=""):
        ttk.Label(parent, text=label).grid(row=r, column=0, sticky="w", pady=2)
        widget.grid(row=r, column=1, sticky="ew", pady=2)
        if hint:
            ttk.Label(parent, text=hint, foreground="#777").grid(row=r, column=2, sticky="w", padx=4)

    # -- вкладка Аудио
    ta = ttk.Frame(nb, padding=8)
    ta.columnconfigure(1, weight=1)
    nb.add(ta, text="Аудио")
    cb = ttk.Combobox(ta, textvariable=v_preset, values=list(PRESETS), state="readonly")
    cb.bind("<<ComboboxSelected>>", apply_preset)
    row(ta, 0, "Пресет", cb)
    row(ta, 1, "Каналы", ttk.Combobox(ta, textvariable=v_channels, values=["Моно", "Стерео"], state="readonly"),
        "3D = моно")
    row(ta, 2, "Частота, Гц", ttk.Combobox(ta, textvariable=v_rate, values=["22050", "32000", "44100", "48000"],
                                           state="readonly"))
    qf = ttk.Frame(ta)
    q_lbl = ttk.Label(qf, width=3)
    qs = ttk.Scale(qf, from_=-1, to=10, orient="horizontal",
                   command=lambda v: (v_quality.set(int(round(float(v)))), q_lbl.configure(text=str(v_quality.get()))))
    qs.set(v_quality.get())
    q_lbl.configure(text=str(v_quality.get()))
    qs.pack(side="left", fill="x", expand=True)
    q_lbl.pack(side="left")
    row(ta, 3, "Качество Vorbis", qf, "5–7 оптимально")
    row(ta, 4, "Усиление, дБ", ttk.Spinbox(ta, textvariable=v_volume, from_=-30, to=30, increment=0.5))
    row(ta, 5, "Нормализация", ttk.Combobox(ta, textvariable=v_norm, values=list(NORMALIZE_MODES.values()),
                                            state="readonly"))
    row(ta, 6, "Пик, dBFS", ttk.Spinbox(ta, textvariable=v_peak, from_=-20, to=0, increment=0.5))
    row(ta, 7, "Громкость, LUFS", ttk.Spinbox(ta, textvariable=v_lufs, from_=-40, to=-5, increment=1))
    ttk.Checkbutton(ta, text="Обрезать тишину в начале/конце", variable=v_trim).grid(row=8, column=0, columnspan=3,
                                                                                    sticky="w", pady=2)
    row(ta, 9, "Fade-in, с", ttk.Spinbox(ta, textvariable=v_fin, from_=0, to=30, increment=0.1))
    row(ta, 10, "Fade-out, с", ttk.Spinbox(ta, textvariable=v_fout, from_=0, to=30, increment=0.1))
    row(ta, 11, "Начать с, с", ttk.Spinbox(ta, textvariable=v_cstart, from_=0, to=36000, increment=0.5))
    row(ta, 12, "Длительность, с", ttk.Spinbox(ta, textvariable=v_cdur, from_=0, to=36000, increment=0.5),
        "0 = целиком")

    # -- вкладка Файлы
    tf = ttk.Frame(nb, padding=8)
    tf.columnconfigure(1, weight=1)
    nb.add(tf, text="Вывод")
    of = ttk.Frame(tf)
    ttk.Entry(of, textvariable=v_out).pack(side="left", fill="x", expand=True)

    def pick_out():
        d = filedialog.askdirectory(title="Папка для OGG")
        if d:
            v_out.set(d)
            refresh_tree()
    ttk.Button(of, text="...", width=3, command=pick_out).pack(side="left")
    row(tf, 0, "Папка вывода", of)
    ttk.Label(tf, text="Пусто = подпапка «ogg» в исходной папке", foreground="#777").grid(
        row=1, column=0, columnspan=3, sticky="w")
    ttk.Checkbutton(tf, text="Сохранять структуру подпапок", variable=v_keep,
                    command=refresh_tree).grid(row=2, column=0, columnspan=3, sticky="w", pady=2)
    ttk.Checkbutton(tf, text="Безопасные имена (латиница, a-z0-9_)", variable=v_sanitize,
                    command=refresh_tree).grid(row=3, column=0, columnspan=3, sticky="w", pady=2)
    pe = ttk.Entry(tf, textvariable=v_prefix)
    pe.bind("<FocusOut>", lambda e: refresh_tree())
    row(tf, 4, "Префикс имени", pe)
    ttk.Checkbutton(tf, text="Перезаписывать существующие", variable=v_overwrite).grid(
        row=5, column=0, columnspan=3, sticky="w", pady=2)
    row(tf, 6, "Потоков", ttk.Spinbox(tf, textvariable=v_threads, from_=1, to=32, increment=1))
    ff = ttk.Frame(tf)
    ttk.Entry(ff, textvariable=v_ffmpeg).pack(side="left", fill="x", expand=True)

    def pick_ffmpeg():
        f = filedialog.askopenfilename(title="Укажите ffmpeg",
                                       filetypes=[("ffmpeg", "ffmpeg*"), ("Все файлы", "*.*")])
        if f:
            v_ffmpeg.set(f)
    ttk.Button(ff, text="...", width=3, command=pick_ffmpeg).pack(side="left")
    row(tf, 7, "ffmpeg", ff)
    ttk.Label(tf, text="Пусто = автопоиск (рядом с программой / PATH)", foreground="#777").grid(
        row=8, column=0, columnspan=3, sticky="w")

    # -- вкладка config.cpp
    tc = ttk.Frame(nb, padding=8)
    tc.columnconfigure(1, weight=1)
    nb.add(tc, text="config.cpp")
    ttk.Checkbutton(tc, text="Генерировать config.cpp", variable=v_gencfg).grid(
        row=0, column=0, columnspan=3, sticky="w", pady=2)
    row(tc, 1, "Имя мода", ttk.Entry(tc, textvariable=v_mod))
    row(tc, 2, "Путь в PBO", ttk.Entry(tc, textvariable=v_spath))
    ttk.Label(tc, text=r"напр. MyMod\sounds — куда положите .ogg", foreground="#777").grid(
        row=3, column=0, columnspan=3, sticky="w")
    row(tc, 4, "Префикс классов", ttk.Entry(tc, textvariable=v_cpref))
    ttk.Checkbutton(tc, text="3D (spatial = 1)", variable=v_spatial).grid(row=5, column=0, columnspan=3,
                                                                          sticky="w", pady=2)
    ttk.Checkbutton(tc, text="Зациклить (loop = 1)", variable=v_loop).grid(row=6, column=0, columnspan=3,
                                                                           sticky="w", pady=2)
    row(tc, 7, "Дальность, м", ttk.Spinbox(tc, textvariable=v_range, from_=1, to=5000, increment=10))
    row(tc, 8, "Громкость", ttk.Spinbox(tc, textvariable=v_svol, from_=0, to=5, increment=0.1))

    def preview_config():
        cur = collect()
        jobs = plan_jobs(collect_inputs(files, cur.output_dir), cur)
        fake = [JobResult(j, True) for j in jobs]
        win = tk.Toplevel(root)
        win.title("Предпросмотр config.cpp")
        win.geometry("700x600")
        t = tk.Text(win, wrap="none", font=("Consolas", 10))
        t.pack(fill="both", expand=True)
        t.insert("1.0", generate_config(fake, cur).replace("\r\n", "\n"))

        def copy():
            root.clipboard_clear()
            root.clipboard_append(t.get("1.0", "end-1c"))
        ttk.Button(win, text="Копировать", command=copy).pack(pady=4)
    ttk.Button(tc, text="Предпросмотр", command=preview_config).grid(row=9, column=0, columnspan=3,
                                                                     sticky="w", pady=8)

    # низ: прогресс и кнопки
    bottom = ttk.Frame(main)
    bottom.grid(row=1, column=0, columnspan=2, sticky="ew", pady=(8, 0))
    bottom.columnconfigure(0, weight=1)
    progress = ttk.Progressbar(bottom, mode="determinate", style="Green.Horizontal.TProgressbar")
    progress.grid(row=0, column=0, sticky="ew")
    btn_start = ttk.Button(bottom, text="▶ Конвертировать")
    btn_start.grid(row=0, column=1, padx=6)
    btn_stop = ttk.Button(bottom, text="■ Стоп", state="disabled", command=lambda: cancel_event.set())
    btn_stop.grid(row=0, column=2)
    btn_open = ttk.Button(bottom, text="Открыть папку", state="disabled")
    btn_open.grid(row=0, column=3, padx=(6, 0))
    ttk.Label(bottom, textvariable=v_status).grid(row=1, column=0, columnspan=4, sticky="w", pady=(4, 0))

    last_out = {"path": None}

    def start():
        if state["busy"]:
            return
        cur = collect()
        save_settings(cur)
        ffmpeg = find_ffmpeg(cur.ffmpeg_path)
        if not ffmpeg:
            messagebox.showerror(APP_NAME, "ffmpeg не найден.\n\nСкачайте ffmpeg (https://www.gyan.dev/ffmpeg/builds/) "
                                           "и положите ffmpeg.exe рядом с программой, либо укажите путь на вкладке «Вывод».")
            return
        if not check_vorbis(ffmpeg):
            messagebox.showerror(APP_NAME, f"ffmpeg собран без кодека libvorbis:\n{ffmpeg}")
            return
        jobs = plan_jobs(collect_inputs(files, cur.output_dir), cur)
        if not jobs:
            messagebox.showinfo(APP_NAME, "Нет файлов для конвертации.")
            return

        state["busy"] = True
        cancel_event.clear()
        btn_start.configure(state="disabled")
        btn_stop.configure(state="normal")
        btn_open.configure(state="disabled")
        progress.configure(maximum=len(jobs), value=0)
        for j in jobs:
            iid = str(j.src.resolve())
            if tree.exists(iid):
                tree.set(iid, "status", "в работе")
        write_log(f"ffmpeg: {ffmpeg}")
        write_log(f"Старт: {len(jobs)} файл(ов), {'моно' if cur.channels == 1 else 'стерео'}, "
                  f"{cur.sample_rate} Гц, q={cur.quality}")

        def on_result(r: JobResult, done: int, total: int):
            def ui():
                iid = str(r.job.src.resolve())
                if tree.exists(iid):
                    tree.set(iid, "status", "✔ готово" if r.ok else "✖ ошибка")
                progress.configure(value=done)
                v_status.set(f"{done} / {total}")
                if r.ok:
                    info = f"{human_size(r.size)}" + (f", {r.duration:.2f} c" if r.duration else "")
                    write_log(f"OK   {r.job.src.name} -> {r.job.dst}  ({info})")
                else:
                    write_log(f"ERR  {r.job.src.name}: {r.message}")
            root.after(0, ui)

        def worker():
            results = convert_all(jobs, cur, ffmpeg, on_result, cancel_event)
            cfg_path = write_config(results, cur) if cur.generate_config else None

            def done_ui():
                state["busy"] = False
                btn_start.configure(state="normal")
                btn_stop.configure(state="disabled")
                ok = sum(1 for r in results if r.ok)
                out = output_root(results, cur) or (jobs[0].dst.parent if jobs else None)
                last_out["path"] = out
                if out:
                    btn_open.configure(state="normal")
                if cfg_path:
                    write_log(f"config.cpp: {cfg_path}")
                msg = f"Готово: {ok} из {len(results)}" + (" (остановлено)" if cancel_event.is_set() else "")
                write_log(msg)
                v_status.set(msg)
            root.after(0, done_ui)

        threading.Thread(target=worker, daemon=True).start()

    btn_start.configure(command=start)
    btn_open.configure(command=lambda: last_out["path"] and open_folder(last_out["path"]))

    # приём файлов из аргументов (например, перетаскивание на .exe в Проводнике)
    for a in sys.argv[1:]:
        if Path(a).exists():
            files.append(a)
    refresh_tree()

    ff_found = find_ffmpeg(s.ffmpeg_path)
    write_log(f"{APP_NAME} {APP_VERSION}")
    write_log(f"ffmpeg: {ff_found}" if ff_found else "ВНИМАНИЕ: ffmpeg не найден — укажите путь на вкладке «Вывод».")

    def on_close():
        save_settings(collect())
        cancel_event.set()
        root.destroy()
    root.protocol("WM_DELETE_WINDOW", on_close)
    root.mainloop()
    return 0


# --------------------------------------------------------------------------------------

def main() -> int:
    args = sys.argv[1:]
    if "--cli" in args:
        return cli([a for a in args if a != "--cli"])
    # без аргументов или только пути (перетаскивание файлов на программу) -> GUI
    if not args or all(Path(a).exists() for a in args):
        try:
            return gui()
        except ImportError:
            if not args:
                print("tkinter недоступен. Используйте консольный режим: --help")
                return 1
    return cli(args)


if __name__ == "__main__":
    sys.exit(main())
