# -*- coding: utf-8 -*-
"""Конвертация звука в OGG Vorbis для DayZ и генерация CfgSoundShaders/CfgSoundSets."""

from __future__ import annotations

from dztk.i18n import tr

import os
import re
import threading
from concurrent.futures import ThreadPoolExecutor, as_completed
from dataclasses import dataclass
from pathlib import Path
from typing import Callable, Iterable, List, Optional

from . import APP_NAME, APP_VERSION
from .common import class_safe, is_within as _is_within, probe_duration, run_ffmpeg, sanitize_name

INPUT_EXTENSIONS = {".mp3", ".wav", ".flac", ".m4a", ".aac", ".wma", ".ogg", ".opus", ".webm", ".mp4"}

# --------------------------------------------------------------------------------------
# Пресеты
# --------------------------------------------------------------------------------------

PRESETS = {
    tr("3D звук (моно)"): dict(channels=1, sample_rate=44100, quality=6, spatial=True, loop=False, range=50),
    tr("Музыка / радио (стерео)"): dict(channels=2, sample_rate=44100, quality=7, spatial=False, loop=False, range=100),
    tr("UI / 2D звук (стерео)"): dict(channels=2, sample_rate=44100, quality=6, spatial=False, loop=False, range=10),
    tr("Эмбиент, петля (моно)"): dict(channels=1, sample_rate=44100, quality=5, spatial=True, loop=True, range=150),
    tr("Выстрел / громкий 3D (моно)"): dict(channels=1, sample_rate=44100, quality=7, spatial=True, loop=False, range=800),
}
DEFAULT_PRESET = tr("3D звук (моно)")

NORMALIZE_MODES = {
    "none": tr("Без нормализации"),
    "peak": tr("По пику (dBFS)"),
    "loudness": tr("По громкости (LUFS, EBU R128)"),
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
        return JobResult(job, False, tr("отменено"))
    if job.dst.resolve() == job.src.resolve():
        return JobResult(job, False, tr("исходный файл совпадает с результатом — укажите другую папку вывода"))
    if job.dst.exists() and not s.overwrite:
        return JobResult(job, True, tr("пропущен (уже существует)"), size=job.dst.stat().st_size)

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
        return JobResult(job, False, err[-1] if err else tr("ffmpeg завершился с кодом {0}").format(p.returncode))

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
                r = JobResult(futs[fut], False, tr("ошибка: {0}").format(e))
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
    w(tr("// Сгенерировано: %s %s") % (APP_NAME, APP_VERSION))
    w(tr("// Путь к звукам в PBO: %s") % (base_path or tr("<не задан>")))
    w(tr("// Воспроизведение в скрипте, например:"))
    w(tr("//   SEffectManager.PlaySound(\"%s_<имя>_SoundSet\", GetPosition());") % pref)
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
