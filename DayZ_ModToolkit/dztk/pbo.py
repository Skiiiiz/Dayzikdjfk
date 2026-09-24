# -*- coding: utf-8 -*-
"""Архивы PBO: упаковка, распаковка, просмотр и сборка мода.

Формат PBO:
  заголовки записей: asciiz имя | u32 метод | u32 исходный_размер | u32 резерв | u32 время | u32 размер_данных
    первая запись — расширение заголовка: имя "", метод 'Vers' (0x56657273), затем пары asciiz ключ/значение
    (prefix, product, ...) и пустая строка; последняя запись — пустое имя и нули;
  данные файлов подряд;
  0x00 + SHA1 всего предыдущего содержимого.
Метод 'Cprs' (0x43707273) — старое сжатие LZSS (поддерживается при распаковке).
"""

from __future__ import annotations

from dztk.i18n import tr

import fnmatch
import hashlib
import os
import struct
from dataclasses import dataclass, field
from pathlib import Path
from typing import Callable, Dict, List, Optional, Tuple

MIME_VERS = 0x56657273
MIME_CPRS = 0x43707273
MIME_ENCO = 0x456E6372

DEFAULT_EXCLUDE = ("*.psd;*.xcf;*.blend;*.blend1;*.bak;*.tmp;*.log;*.pbo;*.bisign;*.bikey;*.biprivatekey;"
                   ".git;.svn;.vs;.vscode;.idea;__pycache__;Thumbs.db;desktop.ini;$PBOPREFIX$*;"
                   "_PBO_PROPERTIES.txt;*.md")
ALWAYS_EXCLUDE = ("*.biprivatekey", "*.pbo")


class PBOError(ValueError):
    pass


@dataclass
class Entry:
    name: str                  # путь внутри PBO через '\'
    method: int = 0
    original_size: int = 0
    timestamp: int = 0
    data_size: int = 0
    offset: int = 0            # смещение данных в файле

    @property
    def size(self) -> int:
        return self.original_size or self.data_size


@dataclass
class PBO:
    properties: Dict[str, str] = field(default_factory=dict)
    entries: List[Entry] = field(default_factory=list)
    checksum: bytes = b""
    path: Optional[Path] = None

    @property
    def prefix(self) -> str:
        return self.properties.get("prefix", "")

    def raw(self, e: Entry) -> bytes:
        with open(self.path, "rb") as f:
            f.seek(e.offset)
            return f.read(e.data_size)

    def read(self, e: Entry) -> bytes:
        raw = self.raw(e)
        if e.method == MIME_CPRS or (e.original_size and e.original_size != e.data_size and e.method != MIME_ENCO):
            return lzss_decompress(raw, e.original_size)
        if e.method == MIME_ENCO:
            raise PBOError(tr("{0}: файл зашифрован (защищённый PBO) — распаковка невозможна").format(e.name))
        return raw


# --------------------------------------------------------------------------------------
# Чтение
# --------------------------------------------------------------------------------------

def _asciiz(d: bytes, p: int) -> Tuple[str, int]:
    e = d.find(b"\x00", p)
    if e < 0:
        raise PBOError(tr("повреждённый заголовок (нет конца строки)"))
    raw = d[p:e]
    try:
        s = raw.decode("utf-8")
    except UnicodeDecodeError:
        s = raw.decode("cp1251", "replace")
    return s, e + 1


def read_pbo(path) -> PBO:
    """Читает заголовок PBO (данные файлов читаются по требованию — подходит для больших архивов)."""
    path = Path(path)
    total = path.stat().st_size
    chunk = 1 << 16
    while True:
        with open(path, "rb") as f:
            head = f.read(chunk)
        try:
            return _parse_header(head, path, total)
        except PBOError:
            if len(head) >= total:
                raise
            chunk *= 4


def _parse_header(d: bytes, path: Path, total: int) -> PBO:
    pbo = PBO(path=path)
    p = 0
    first = True
    while True:
        name, p = _asciiz(d, p)
        if p + 20 > len(d):
            raise PBOError(tr("обрезанный заголовок PBO"))
        method, orig, _res, ts, size = struct.unpack_from("<5I", d, p)
        p += 20
        if first and name == "" and method == MIME_VERS:
            while True:
                k, p = _asciiz(d, p)
                if k == "":
                    break
                v, p = _asciiz(d, p)
                pbo.properties[k] = v
            first = False
            continue
        first = False
        if name == "":
            break
        pbo.entries.append(Entry(name, method, orig, ts, size))
    off = p
    for e in pbo.entries:
        e.offset = off
        off += e.data_size
    if off > total:
        raise PBOError(tr("данные файлов выходят за конец PBO (архив повреждён)"))
    if total >= off + 21:
        with open(path, "rb") as f:
            f.seek(off)
            tail = f.read(21)
        if tail[:1] == b"\x00":
            pbo.checksum = tail[1:21]
    return pbo


def verify_checksum(path) -> Optional[bool]:
    pbo = read_pbo(path)
    if not pbo.checksum:
        return None
    end = Path(path).stat().st_size - 21
    sha = hashlib.sha1()
    with open(path, "rb") as f:
        left = end
        while left > 0:
            buf = f.read(min(left, 1 << 20))
            if not buf:
                break
            sha.update(buf)
            left -= len(buf)
    return sha.digest() == pbo.checksum


def lzss_decompress(src: bytes, out_len: int) -> bytes:
    out = bytearray()
    i = 0
    n = len(src)
    while len(out) < out_len and i < n:
        flags = src[i]
        i += 1
        for bit in range(8):
            if len(out) >= out_len or i >= n:
                break
            if flags & (1 << bit):
                out.append(src[i])
                i += 1
            else:
                if i + 1 >= n:
                    raise PBOError(tr("повреждённые LZSS-данные"))
                b1, b2 = src[i], src[i + 1]
                i += 2
                rpos = b1 | ((b2 & 0xF0) << 4)
                rlen = (b2 & 0x0F) + 3
                pos = len(out) - rpos
                for k in range(rlen):
                    if len(out) >= out_len:
                        break
                    out.append(0x20 if pos + k < 0 else out[pos + k])
    if len(out) != out_len:
        raise PBOError(tr("LZSS: ожидалось {0} байт, получено {1}").format(out_len, len(out)))
    return bytes(out)


def unpack(src, dst_dir, derapify: bool = True,
           on_file: Optional[Callable[[str, int, int], None]] = None) -> Tuple[Path, PBO]:
    """Распаковывает PBO в dst_dir/<имя_pbo>. Возвращает (папка, PBO)."""
    from . import rap
    from .cfg import to_text
    src = Path(src)
    pbo = read_pbo(src)
    root = Path(dst_dir) / src.stem
    root.mkdir(parents=True, exist_ok=True)
    total = len(pbo.entries)
    root_res = root.resolve()
    for i, e in enumerate(pbo.entries, 1):
        rel = e.name.replace("\\", "/").lstrip("/")
        target = (root / rel).resolve()
        try:
            target.relative_to(root_res)
        except ValueError:
            raise PBOError(tr("недопустимый путь в архиве: {0}").format(e.name))
        target.parent.mkdir(parents=True, exist_ok=True)
        data = pbo.read(e)
        target.write_bytes(data)
        if e.timestamp:
            try:
                os.utime(target, (e.timestamp, e.timestamp))
            except OSError:
                pass
        if derapify and target.name.lower() == "config.bin" and rap.is_rapified(data):
            cpp = target.with_suffix(".cpp")
            if not cpp.exists():
                cpp.write_text(to_text(rap.read_rap(data)).replace("\n", "\r\n"), encoding="utf-8", newline="")
        if on_file:
            on_file(e.name, i, total)
    if pbo.prefix:
        (root / "$PBOPREFIX$").write_text(pbo.prefix + "\n", encoding="utf-8")
    return root, pbo


# --------------------------------------------------------------------------------------
# Запись
# --------------------------------------------------------------------------------------

def _z(s: str) -> bytes:
    return s.encode("utf-8") + b"\x00"


def write_pbo(dst, files: List[Tuple[str, bytes, int]], properties: Dict[str, str]) -> bytes:
    """files: [(имя_внутри_pbo через '\\', данные, время)]. Возвращает SHA1-контрольную сумму."""
    header = bytearray(_z("") + struct.pack("<5I", MIME_VERS, 0, 0, 0, 0))
    for k, v in properties.items():
        if v:
            header += _z(k) + _z(v)
    header += b"\x00"
    for name, data, ts in files:
        header += _z(name) + struct.pack("<5I", 0, len(data), 0, ts & 0xFFFFFFFF, len(data))
    header += _z("") + struct.pack("<5I", 0, 0, 0, 0, 0)
    sha = hashlib.sha1(header)
    dst = Path(dst)
    dst.parent.mkdir(parents=True, exist_ok=True)
    tmp = dst.with_name(dst.name + ".part")
    with open(tmp, "wb") as f:
        f.write(header)
        for _, data, _ in files:
            f.write(data)
            sha.update(data)
        digest = sha.digest()
        f.write(b"\x00" + digest)
    os.replace(tmp, dst)
    return digest


def _excluded(rel: str, patterns: List[str]) -> bool:
    parts = rel.split("/")
    for pat in patterns:
        pat = pat.strip()
        if not pat:
            continue
        if fnmatch.fnmatch(parts[-1].lower(), pat.lower()) or fnmatch.fnmatch(rel.lower(), pat.lower()):
            return True
        if any(fnmatch.fnmatch(p.lower(), pat.lower()) for p in parts[:-1]):
            return True
    return False


@dataclass
class BuildOptions:
    prefix: str = ""                   # пусто = из $PBOPREFIX$/_PBO_PROPERTIES.txt/имени папки
    exclude: str = DEFAULT_EXCLUDE
    rapify: bool = True                # config.cpp -> config.bin
    check: bool = True                 # проверка ошибок перед сборкой
    stop_on_errors: bool = True
    convert_images: bool = False       # PNG/TGA без парного .paa -> .paa
    mod_layout: bool = True            # @Мод/Addons/имя.pbo + @Мод/Keys
    private_key: str = ""              # .biprivatekey для подписи
    sign_version: int = 3


@dataclass
class BuildResult:
    ok: bool
    pbo: Optional[Path] = None
    message: str = ""
    issues: list = field(default_factory=list)
    files: int = 0
    signature: Optional[Path] = None
    bikey: Optional[Path] = None


def collect_files(src: Path, opts: BuildOptions) -> Tuple[List[Tuple[str, Path]], List[str]]:
    patterns = [p for p in opts.exclude.split(";") if p.strip()] + list(ALWAYS_EXCLUDE)
    files, skipped = [], []
    for p in sorted(src.rglob("*"), key=lambda x: x.as_posix().lower()):
        if not p.is_file():
            continue
        rel = p.relative_to(src).as_posix()
        if _excluded(rel, patterns):
            skipped.append(rel)
            continue
        files.append((rel, p))
    return files, skipped


def build(src, out_dir, opts: Optional[BuildOptions] = None, log: Optional[Callable[[str, str], None]] = None,
          ffmpeg: Optional[str] = None) -> BuildResult:
    """Собирает папку мода в PBO: проверка -> бинаризация конфигов -> упаковка -> подпись."""
    from . import cfg, checks, paa, rap, sign

    opts = opts or BuildOptions()
    log = log or (lambda msg, level="": None)
    src = Path(src).resolve()
    if not src.is_dir():
        return BuildResult(False, message=tr("папка не найдена: {0}").format(src))
    prefix = (opts.prefix or checks.detect_prefix(src)).strip().strip("\\/").replace("/", "\\")
    name = prefix.split("\\")[-1] or src.name

    # 1. проверка
    issues = []
    if opts.check:
        log(tr("Проверка ошибок..."), "")
        rep = checks.check_paths([str(src)], ffmpeg)
        issues = rep.issues
        errors = [i for i in issues if i.level == "error"]
        for i in issues:
            if i.level != "info":
                log("  " + i.format(), i.level)
        if errors and opts.stop_on_errors:
            return BuildResult(False, message=tr("сборка остановлена: ошибок {0} (исправьте или отключите остановку при ошибках)").format(len(errors)), issues=issues)

    # 2. сбор файлов
    files, skipped = collect_files(src, opts)
    names = {rel.lower() for rel, _ in files}
    payload: List[Tuple[str, bytes, int]] = []
    for rel, p in files:
        low = rel.lower()
        ts = int(p.stat().st_mtime)
        if opts.rapify and p.name.lower() == "config.cpp":
            try:
                root, _ = cfg.parse_config(p, include_dirs=[src])
            except cfg.ConfigError as e:
                return BuildResult(False, message=tr("ошибка в {0}: {1}").format(rel, e), issues=issues + [e.issue])
            bin_rel = rel[:-4] + ".bin"
            payload.append((bin_rel.replace("/", "\\"), rap.write_rap(root), ts))
            log(tr("  {0} -> {1} (бинаризован)").format(rel, Path(bin_rel).name), "")
            continue
        if opts.rapify and p.name.lower() == "config.bin" and (low[:-4] + ".cpp") in names:
            continue  # будет заменён бинаризованным config.cpp
        if opts.rapify and p.suffix.lower() == ".hpp" and any(n.endswith("config.cpp") for n in names):
            continue  # #include-файлы уже встроены в config.bin
        if opts.convert_images and p.suffix.lower() in (".png", ".tga") and (low[:-4] + ".paa") not in names:
            try:
                data, fmt = paa.encode_paa(paa.load_image(p), paa.fmt_from_suffix(p.stem) or "auto")
                payload.append(((rel[:-4] + ".paa").replace("/", "\\"), data, ts))
                log(f"  {rel} -> {Path(rel).stem}.paa ({fmt})", "")
                continue
            except Exception as e:
                log(tr("  {0}: не сконвертирован в PAA ({1})").format(rel, e), "warning")
        payload.append((rel.replace("/", "\\"), p.read_bytes(), ts))

    if not payload:
        return BuildResult(False, message=tr("нет файлов для упаковки"))

    out_dir = Path(out_dir)
    if opts.mod_layout:
        mod_root = out_dir / ("@" + name)
        pbo_path = mod_root / "Addons" / f"{name}.pbo"
    else:
        mod_root = out_dir
        pbo_path = out_dir / f"{name}.pbo"
    props = {"prefix": prefix, "product": "dayz ugc"}
    write_pbo(pbo_path, payload, props)
    size = pbo_path.stat().st_size
    log(tr("PBO: {0} ({1} файлов, {2:.2f} МБ, prefix={3})").format(pbo_path, len(payload), size / 1048576, prefix), "")
    if skipped:
        log(tr("  пропущено по фильтру: {0} ({1}{2})").format(len(skipped), ', '.join(skipped[:5]), '...' if len(skipped) > 5 else ''), "")

    res = BuildResult(True, pbo_path, tr("готово"), issues, len(payload))
    # 3. подпись
    if opts.private_key:
        key = sign.PrivateKey.load(opts.private_key)
        res.signature = sign.sign_pbo(pbo_path, key, opts.sign_version)
        if opts.mod_layout:
            keys_dir = mod_root / "Keys"
            keys_dir.mkdir(parents=True, exist_ok=True)
            res.bikey = keys_dir / f"{key.authority}.bikey"
            res.bikey.write_bytes(key.public().to_bytes())
        log(tr("Подпись: {0}").format(res.signature.name) + (tr(", ключ: Keys/{0}").format(res.bikey.name) if res.bikey else ""), "")
    return res
