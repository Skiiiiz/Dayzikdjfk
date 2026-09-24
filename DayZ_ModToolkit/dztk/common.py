# -*- coding: utf-8 -*-
"""Общие утилиты: поиск ffmpeg, имена файлов, настройки."""

from __future__ import annotations

from dztk.i18n import tr

import json
import os
import re
import shutil
import subprocess
import sys
from pathlib import Path
from typing import List, Optional


def settings_file() -> Path:
    base = os.environ.get("APPDATA") or os.path.join(Path.home(), ".config")
    return Path(base) / "DayZModToolkit" / "settings.json"


def load_settings_dict() -> dict:
    try:
        return json.loads(settings_file().read_text(encoding="utf-8"))
    except Exception:
        return {}


def save_settings_dict(data: dict) -> None:
    try:
        p = settings_file()
        p.parent.mkdir(parents=True, exist_ok=True)
        p.write_text(json.dumps(data, ensure_ascii=False, indent=2), encoding="utf-8")
    except Exception:
        pass


# --------------------------------------------------------------------------------------
# ffmpeg
# --------------------------------------------------------------------------------------

def app_dir() -> Path:
    if getattr(sys, "frozen", False):  # PyInstaller
        return Path(sys.executable).resolve().parent
    return Path(__file__).resolve().parent.parent


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

def resource_path(rel: str) -> Path:
    """Файл из комплекта программы (в собранном .exe — из временной папки PyInstaller)."""
    base = Path(getattr(sys, "_MEIPASS", "")) if getattr(sys, "frozen", False) else app_dir()
    return base / rel


def is_within(path: Path, parent: Path) -> bool:
    try:
        path.resolve().relative_to(parent.resolve())
        return True
    except ValueError:
        return False


def human_size(n: int) -> str:
    size = float(n)
    units = (tr("Б"), tr("КБ"), tr("МБ"), tr("ГБ"))
    for unit in units:
        if size < 1024 or unit == units[-1]:
            return f"{size:.0f} {unit}" if unit == units[0] else f"{size:.1f} {unit}"
        size /= 1024
    return tr("{0} Б").format(n)


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

