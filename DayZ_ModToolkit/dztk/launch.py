# -*- coding: utf-8 -*-
"""Запуск DayZ / DayZ Server с модами и разбор логов.

Командная строка собирается так, как её собирают DayZ Launcher и DayZ Tools Workbench:
  DayZ_x64.exe  -mod=C:\\@A;C:\\@B -filePatching -profiles=<папка> -connect=127.0.0.1 -port=2302
  DayZServer_x64.exe -config=serverDZ.cfg -mod=... -serverMod=... -profiles=<папка> -port=2302
                     -dologs -adminlog -netlog -freezecheck -filePatching
С -filePatching BattlEye не работает, поэтому запускается DayZ_x64.exe, а не DayZ_BE.exe.

Логи (script_*.log, crash_*.log, *.RPT, error_*.log) ищутся в папке профилей; для клиента
по умолчанию это %LOCALAPPDATA%\\DayZ. Разбор выделяет ошибки компиляции (файл(строка): текст),
ошибки скриптов (SCRIPT (E): ...) со стеком вызовов, предупреждения и ошибки движка из RPT.
"""

from __future__ import annotations

from .i18n import tr

import os
import re
import subprocess
from dataclasses import dataclass, field
from pathlib import Path
from typing import Dict, Iterable, List, Optional, Sequence, Tuple

CLIENT_EXE = ("DayZ_x64.exe", "DayZ.exe")
SERVER_EXE = ("DayZServer_x64.exe", "DayZServer.exe")
LOG_PATTERNS = ("script*.log", "crash*.log", "error*.log", "*.RPT", "*.rpt", "*.ADM")


# --------------------------------------------------------------------------------------
# Поиск игры
# --------------------------------------------------------------------------------------

def _steam_libraries() -> List[Path]:
    roots: List[Path] = []
    if os.name == "nt":
        try:
            import winreg  # type: ignore
            for hive, key in ((winreg.HKEY_LOCAL_MACHINE, r"SOFTWARE\WOW6432Node\Valve\Steam"),
                              (winreg.HKEY_LOCAL_MACHINE, r"SOFTWARE\Valve\Steam"),
                              (winreg.HKEY_CURRENT_USER, r"SOFTWARE\Valve\Steam")):
                try:
                    with winreg.OpenKey(hive, key) as k:
                        for name in ("InstallPath", "SteamPath"):
                            try:
                                roots.append(Path(winreg.QueryValueEx(k, name)[0]))
                            except OSError:
                                pass
                except OSError:
                    pass
        except ImportError:
            pass
        for env in ("ProgramFiles(x86)", "ProgramFiles"):
            if os.environ.get(env):
                roots.append(Path(os.environ[env]) / "Steam")
    else:
        roots += [Path.home() / ".steam/steam", Path.home() / ".local/share/Steam"]
    libs: List[Path] = []
    for r in roots:
        if r not in libs:
            libs.append(r)
        vdf = r / "steamapps" / "libraryfolders.vdf"
        try:
            for m in re.finditer(r'"path"\s+"([^"]+)"', vdf.read_text(encoding="utf-8", errors="replace")):
                p = Path(m.group(1).replace("\\\\", "\\"))
                if p not in libs:
                    libs.append(p)
        except OSError:
            pass
    return libs


def find_game_dirs() -> Dict[str, Optional[Path]]:
    """Папки DayZ и DayZServer из библиотек Steam (если установлены)."""
    found: Dict[str, Optional[Path]] = {"client": None, "server": None}
    for lib in _steam_libraries():
        for kind, name, exes in (("client", "DayZ", CLIENT_EXE), ("server", "DayZServer", SERVER_EXE)):
            d = lib / "steamapps" / "common" / name
            if found[kind] is None and any((d / e).is_file() for e in exes):
                found[kind] = d
    return found


def find_exe(folder, exes: Sequence[str]) -> Optional[Path]:
    folder = Path(folder)
    if folder.is_file():
        return folder
    for e in exes:
        if (folder / e).is_file():
            return folder / e
    return None


def default_client_profiles() -> Path:
    base = os.environ.get("LOCALAPPDATA")
    return Path(base) / "DayZ" if base else Path.home() / "DayZ"


# --------------------------------------------------------------------------------------
# Командная строка
# --------------------------------------------------------------------------------------

@dataclass
class LaunchOptions:
    game_dir: str = ""
    server_dir: str = ""
    mods: List[str] = field(default_factory=list)          # клиент и сервер
    server_mods: List[str] = field(default_factory=list)   # только сервер (-serverMod)
    profiles: str = ""                                      # профили сервера
    client_profiles: str = ""
    server_config: str = "serverDZ.cfg"
    mission: str = ""                                       # -mission=mpmissions\\dayzOffline.chernarusplus
    port: int = 2302
    file_patching: bool = True
    windowed: bool = True
    connect: bool = True                                    # клиент подключается к локальному серверу
    name: str = ""                                          # имя игрока (-name=)
    extra_client: str = ""
    extra_server: str = ""


def _mod_arg(mods: Iterable[str]) -> str:
    return ";".join(str(Path(m)) if (os.sep in m or "/" in m) else m for m in mods if m.strip())


def _split(extra: str) -> List[str]:
    import shlex
    try:
        return shlex.split(extra, posix=os.name != "nt")
    except ValueError:
        return extra.split()


def client_command(o: LaunchOptions) -> List[str]:
    exe = find_exe(o.game_dir, CLIENT_EXE) if o.game_dir else None
    cmd = [str(exe or Path(o.game_dir or ".") / CLIENT_EXE[0])]
    if o.mods:
        cmd.append("-mod=" + _mod_arg(o.mods))
    if o.file_patching:
        cmd.append("-filePatching")
    if o.windowed:
        cmd.append("-window")
    if o.client_profiles:
        cmd.append("-profiles=" + str(Path(o.client_profiles)))
    if o.connect:
        cmd += ["-connect=127.0.0.1", f"-port={o.port}"]
    if o.name:
        cmd.append("-name=" + o.name)
    cmd += ["-nosplash", "-noPause", "-noBenchmark", "-dologs", "-scriptDebug=true"]
    return cmd + _split(o.extra_client)


def server_command(o: LaunchOptions) -> List[str]:
    folder = o.server_dir or o.game_dir
    exe = find_exe(folder, SERVER_EXE) if folder else None
    cmd = [str(exe or Path(folder or ".") / SERVER_EXE[0])]
    if o.server_config:
        cmd.append("-config=" + o.server_config)
    if o.mods:
        cmd.append("-mod=" + _mod_arg(o.mods))
    if o.server_mods:
        cmd.append("-serverMod=" + _mod_arg(o.server_mods))
    if o.mission:
        cmd.append("-mission=" + o.mission)
    cmd.append("-profiles=" + str(Path(o.profiles or "profiles")))
    cmd.append(f"-port={o.port}")
    if o.file_patching:
        cmd.append("-filePatching")
    cmd += ["-dologs", "-adminlog", "-netlog", "-freezecheck"]
    return cmd + _split(o.extra_server)


def check_options(o: LaunchOptions, server: bool) -> List[str]:
    """Проблемы, из-за которых запуск не сработает (для показа до запуска)."""
    problems = []
    folder = (o.server_dir or o.game_dir) if server else o.game_dir
    exes = SERVER_EXE if server else CLIENT_EXE
    if not folder or not find_exe(folder, exes):
        problems.append(tr("не найден {0} (укажите папку {1})").format(exes[0], "DayZServer" if server else "DayZ"))
    for m in list(o.mods) + (list(o.server_mods) if server else []):
        p = Path(m)
        if (p.is_absolute() or os.sep in m or "/" in m) and not p.is_dir():
            problems.append(tr("папка мода не найдена: {0}").format(m))
        elif p.is_dir() and not (p / "addons").is_dir() and not (p / "Addons").is_dir():
            problems.append(tr("в папке мода нет addons: {0}").format(m))
    if server and folder and o.server_config and not (Path(folder) / o.server_config).is_file() \
            and not Path(o.server_config).is_file():
        problems.append(tr("нет файла конфигурации сервера: {0}").format(o.server_config))
    return problems


def start(cmd: List[str]) -> subprocess.Popen:
    exe = Path(cmd[0])
    kw = {}
    if os.name == "nt":
        kw["creationflags"] = getattr(subprocess, "CREATE_NEW_PROCESS_GROUP", 0)
    return subprocess.Popen(cmd, cwd=str(exe.parent) if exe.parent.is_dir() else None, **kw)


# --------------------------------------------------------------------------------------
# Логи
# --------------------------------------------------------------------------------------

@dataclass
class LogEntry:
    level: str               # error | warning | info
    message: str
    file: str = ""           # путь из лога (как в игре, напр. MyMod/scripts/4_World/x.c)
    line: int = 0
    time: str = ""
    log: str = ""            # файл лога
    log_line: int = 0
    stack: List[Tuple[str, int, str]] = field(default_factory=list)   # (файл, строка, функция)

    @property
    def where(self) -> str:
        return f"{self.file}:{self.line}" if self.file else ""


_TIME = r"(?:(?P<time>\d{1,2}:\d{2}:\d{2}(?:\.\d+)?)\s+)?"
_COMPILE = re.compile(_TIME + r"(?:SCRIPT\s*(?:\((?:E|W)\))?\s*:\s*)?(?P<file>[\w@$\-./\\ ]+?\.c)\((?P<line>\d+)\)\s*:\s*(?P<msg>.+)")
_SCRIPT = re.compile(_TIME + r"SCRIPT\s*\((?P<lvl>E|W)\)\s*:\s*(?P<msg>.*)")
_SCRIPT_AT = re.compile(r"@\"(?P<file>[^\"]+?\.c),(?P<line>\d+)\"\s*:?\s*(?P<msg>.*)")
_STACK = re.compile(r"^\s*(?P<file>[\w@$\-./\\]+?\.c):(?P<line>\d+)(?:\s+Function\s+(?P<func>[\w:<>~.]+))?\s*$")
_STACK2 = re.compile(r"^\s*(?P<func>[\w:<>~.]+)\s+\((?P<file>[\w@$\-./\\ ]+?\.c)\s*:\s*(?P<line>\d+)\)\s*$")
_RPT_ERR = re.compile(_TIME + r"(?P<msg>(?:ErrorMessage|Error|ERROR|Critical|Cannot open|Cannot load|Can't open|"
                      r"Can't load|No entry|Unhandled exception|Crash|Fatal|FATAL)\b.*)")
_RPT_WARN = re.compile(_TIME + r"(?P<msg>(?:Warning Message|Warning|WARNING|Missing|Unknown|Undefined|"
                       r"Updating base class|Conflicting addon)\b.*)")
_NOISE = re.compile(r"(?:Updating base class .*->)|(?:^\s*$)")


def parse_log(text: str, log_name: str = "") -> List[LogEntry]:
    """Ошибки и предупреждения из текста лога DayZ (script/crash/RPT)."""
    out: List[LogEntry] = []
    last: Optional[LogEntry] = None
    in_stack = False
    is_rpt = log_name.lower().endswith(".rpt")
    for n, raw in enumerate(text.splitlines(), 1):
        line = raw.rstrip()
        s = line.strip()
        if not s:
            in_stack = False
            continue
        if last is not None and (s.startswith("Stack trace:") or s.startswith("Function:") or
                                 s.startswith("Class:") or s.startswith("Reason:")):
            in_stack = True
            if s.startswith(("Class:", "Function:", "Reason:")):
                last.message += " | " + re.sub(r"\s+", " ", s)
            continue
        if in_stack and last is not None:
            m = _STACK2.match(line) or _STACK.match(line)
            if m:
                f, ln = m.group("file").strip(), int(m.group("line"))
                last.stack.append((f, ln, m.group("func") or ""))
                if not last.file:
                    last.file, last.line = f, ln
                continue
            in_stack = False
        m = _COMPILE.match(s)
        if m:
            msg = m.group("msg").strip()
            lvl = "warning" if re.match(r"(?i)warning|possible", msg) else "error"
            last = LogEntry(lvl, msg, m.group("file").strip(), int(m.group("line")), m.group("time") or "",
                            log_name, n)
            out.append(last)
            continue
        m = _SCRIPT.match(s)
        if m:
            msg = m.group("msg").strip()
            e = LogEntry("error" if m.group("lvl") == "E" else "warning", msg, time=m.group("time") or "",
                         log=log_name, log_line=n)
            at = _SCRIPT_AT.search(msg)
            if at:
                e.file, e.line = at.group("file"), int(at.group("line"))
                e.message = at.group("msg").strip() or msg
            # «Can't compile ... script module» — само место ошибки на следующей строке, отдельной записью
            out.append(e)
            last = e
            continue
        if is_rpt or log_name.lower().startswith(("crash", "error")):
            if _NOISE.search(s):
                continue
            m = _RPT_ERR.match(s)
            if m:
                last = LogEntry("error", m.group("msg").strip(), time=m.group("time") or "", log=log_name,
                                log_line=n)
                out.append(last)
                continue
            m = _RPT_WARN.match(s)
            if m and is_rpt:
                last = LogEntry("warning", m.group("msg").strip(), time=m.group("time") or "", log=log_name,
                                log_line=n)
                out.append(last)
                continue
            if log_name.lower().startswith("crash") and last is None:
                last = LogEntry("error", s, log=log_name, log_line=n)
                out.append(last)
    return _dedupe(out)


def _dedupe(entries: List[LogEntry]) -> List[LogEntry]:
    """Одинаковые ошибки (каждый кадр в цикле) — одной записью с числом повторов."""
    seen: Dict[Tuple[str, str, str, int], LogEntry] = {}
    out: List[LogEntry] = []
    counts: Dict[int, int] = {}
    for e in entries:
        key = (e.level, e.message, e.file, e.line)
        if key in seen:
            counts[id(seen[key])] = counts.get(id(seen[key]), 1) + 1
            continue
        seen[key] = e
        out.append(e)
    for e in out:
        c = counts.get(id(e))
        if c:
            e.message += tr(" (повторов: {0})").format(c)
    return out


def find_logs(folders: Iterable, newest_only: bool = True) -> List[Path]:
    """Файлы логов в папках профилей; с newest_only — последний лог каждого вида."""
    files: List[Path] = []
    for d in folders:
        d = Path(d)
        if not d.is_dir():
            continue
        for pat in LOG_PATTERNS:
            files += [p for p in d.glob(pat) if p.is_file()]
    files = sorted(set(files), key=lambda p: p.stat().st_mtime, reverse=True)
    if not newest_only:
        return files
    kinds, out = set(), []
    for p in files:
        k = (p.parent, re.sub(r"[\d_\-]+", "", p.stem.lower()), p.suffix.lower())
        if k not in kinds:
            kinds.add(k)
            out.append(p)
    return out


def read_log(path: Path) -> str:
    data = Path(path).read_bytes()
    if data.startswith(b"\xff\xfe"):
        return data.decode("utf-16", "replace")
    return data.decode("utf-8", "replace")


class LogTail:
    """Следит за логами в папках профилей и отдаёт новые записи (для вкладки «Запуск»)."""

    def __init__(self, folders: Iterable, since: float = 0.0):
        self.folders = [Path(f) for f in folders]
        self.since = since
        self.pos: Dict[Path, int] = {}
        self.pending: Dict[Path, str] = {}

    def poll(self) -> List[LogEntry]:
        out: List[LogEntry] = []
        for p in find_logs(self.folders, newest_only=False):
            try:
                st = p.stat()
            except OSError:
                continue
            if st.st_mtime < self.since and p not in self.pos:
                continue
            start = self.pos.get(p, 0)
            if st.st_size < start:
                start = 0
            if st.st_size == start:
                continue
            with open(p, "rb") as fh:
                fh.seek(start)
                chunk = fh.read()
            self.pos[p] = start + len(chunk)
            text = self.pending.pop(p, "") + chunk.decode("utf-8", "replace")
            # незаконченную запись (стек ещё пишется) оставляем до следующего раза
            cut = text.rfind("\n\n")
            if cut < 0 or cut < len(text) - 4000:
                cut = text.rfind("\n")
            if cut >= 0 and cut + 1 < len(text):
                self.pending[p] = text[cut + 1:]
                text = text[:cut + 1]
            out += parse_log(text, p.name)
        return out


# --------------------------------------------------------------------------------------
# Сопоставление путей из лога с файлами мода
# --------------------------------------------------------------------------------------

def resolve(path: str, mod_roots: Iterable) -> Optional[Path]:
    """Путь из лога (MyMod/scripts/4_World/x.c) -> файл на диске в одном из модов."""
    from .checks import detect_prefix
    norm = path.replace("\\", "/").lstrip("/").lower()
    if not norm:
        return None
    for root in mod_roots:
        root = Path(root)
        if not root.is_dir():
            continue
        cands = []
        prefix = detect_prefix(root).replace("\\", "/").strip("/").lower()
        if prefix and norm.startswith(prefix + "/"):
            cands.append(norm[len(prefix) + 1:])
        if norm.startswith(root.name.lower() + "/"):
            cands.append(norm[len(root.name) + 1:])
        cands.append(norm)
        for c in cands:
            p = _ci_path(root, c)
            if p is not None:
                return p
    return None


def _ci_path(root: Path, rel: str) -> Optional[Path]:
    cur = root
    for part in rel.split("/"):
        if not part:
            continue
        nxt = cur / part
        if not nxt.exists():
            try:
                nxt = next((c for c in cur.iterdir() if c.name.lower() == part.lower()), None)
            except OSError:
                return None
            if nxt is None:
                return None
        cur = nxt
    return cur if cur.is_file() else None
