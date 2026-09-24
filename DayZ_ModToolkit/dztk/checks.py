# -*- coding: utf-8 -*-
"""Проверка файлов мода DayZ на ошибки.

Поддерживаемые файлы:
  config.cpp            — синтаксис, базовые классы, дубликаты, CfgPatches
  config.bin            — целостность бинарного конфига
  *.c                   — синтаксис Enforce Script (см. enforce.py)
  *.layout, *.imageset  — скобки и строки
  *.xml                 — корректность XML + смысловые проверки types.xml, events.xml,
                          cfgspawnabletypes.xml, globals.xml, cfgeventspawns.xml
  *.json                — синтаксис JSON, повторяющиеся ключи
  stringtable.csv       — структура, дубликаты ключей, кодировка
  *.paa                 — структура, размеры (степень двойки), mip-уровни
  *.ogg                 — кодек Vorbis, частота (нужен ffmpeg)

Проверки по всему моду: ссылки на несуществующие файлы (.paa/.ogg/.layout/...),
ключи #STR_ без перевода, папки скриптов из CfgMods, «плохие» имена файлов.
"""

from __future__ import annotations

import csv
import io
import json
import re
import xml.etree.ElementTree as ET
from dataclasses import dataclass, field
from pathlib import Path
from typing import Callable, Dict, Iterable, List, Optional, Set, Tuple

from . import cfg, enforce, paa, rap
from .cfg import Issue

REF_EXTENSIONS = (".paa", ".edds", ".ogg", ".wss", ".layout", ".imageset", ".rvmat", ".p3d", ".xml",
                  ".styles", ".ptc", ".emat", ".anm", ".asi", ".json", ".c", ".tga", ".png")
VANILLA_ROOTS = ("dz/", "gui/", "scripts/", "graphics/", "system/", "core/", "sound/", "dayz/")
CHECK_EXTENSIONS = {".c", ".cpp", ".bin", ".layout", ".imageset", ".xml", ".json", ".csv", ".paa", ".ogg",
                    ".styles", ".hpp", ".h"}
SKIP_DIRS = {".git", "__pycache__", "node_modules", ".vs", ".idea"}

_STR_KEY = re.compile(r"#(STR_[A-Za-z0-9_]+)")


@dataclass
class ModContext:
    """Сведения о моде для перекрёстных проверок."""
    root: Path
    prefix: str = ""
    index: Dict[str, Path] = field(default_factory=dict)       # относительный путь (lower, '/') -> файл
    dirs: Set[str] = field(default_factory=set)
    refs: List[Tuple[str, str, Issue]] = field(default_factory=list)  # (путь, варианты расширений, место)
    dir_refs: List[Tuple[str, Issue]] = field(default_factory=list)
    str_refs: List[Tuple[str, Issue]] = field(default_factory=list)
    str_keys: Set[str] = field(default_factory=set)
    has_stringtable: bool = False

    @classmethod
    def build(cls, root: Path) -> "ModContext":
        root = root.resolve()
        ctx = cls(root=root, prefix=detect_prefix(root))
        for p in root.rglob("*"):
            if any(part in SKIP_DIRS for part in p.relative_to(root).parts):
                continue
            rel = p.relative_to(root).as_posix().lower()
            if p.is_dir():
                ctx.dirs.add(rel)
            else:
                ctx.index[rel] = p
        return ctx

    def resolve(self, ref: str) -> Optional[str]:
        """Путь из мода -> относительный путь внутри папки мода (или None для внешних/ванильных)."""
        r = ref.strip().replace("\\", "/").lstrip("/").lower()
        while "//" in r:
            r = r.replace("//", "/")
        if not r or r.startswith(VANILLA_ROOTS) or ":" in r.split("/")[0]:
            return None
        pref = self.prefix.replace("\\", "/").strip("/").lower()
        if pref and (r == pref or r.startswith(pref + "/")):
            return r[len(pref) + 1:]
        first = r.split("/")[0]
        if first == self.root.name.lower():
            return r[len(first) + 1:]
        return None


def detect_prefix(root: Path) -> str:
    for name in ("$PBOPREFIX$", "$PBOPREFIX$.txt", "$prefix$"):
        p = root / name
        if p.is_file():
            txt = p.read_text(encoding="utf-8", errors="replace").strip()
            for line in txt.splitlines():
                line = line.strip()
                if line and "=" not in line:
                    return line
                if line.lower().startswith("prefix="):
                    return line.split("=", 1)[1].strip()
    p = root / "_PBO_PROPERTIES.txt"
    if p.is_file():
        for line in p.read_text(encoding="utf-8", errors="replace").splitlines():
            if line.lower().startswith("prefix="):
                return line.split("=", 1)[1].strip()
    return root.name


def find_mod_root(path: Path) -> Path:
    """Поднимается от файла вверх до папки с config.cpp/config.bin/$PBOPREFIX$."""
    p = path.resolve()
    cur = p if p.is_dir() else p.parent
    for c in [cur] + list(cur.parents):
        for marker in ("config.cpp", "config.bin", "$PBOPREFIX$", "_PBO_PROPERTIES.txt"):
            if (c / marker).exists():
                return c
    return cur


def _add_ref(ctx: Optional[ModContext], value: str, where: Issue, exts: str = "") -> None:
    if ctx is None:
        return
    v = value.strip()
    low = v.lower()
    if exts:
        ctx.refs.append((v, exts, where))
    elif low.endswith(REF_EXTENSIONS) and ("/" in v or "\\" in v):
        ctx.refs.append((v, "", where))
    for m in _STR_KEY.finditer(v):
        if not m.group(1).endswith("_"):   # "#STR_MOD_" — префикс, а не ключ
            ctx.str_refs.append((m.group(1), where))


# --------------------------------------------------------------------------------------
# config.cpp / config.bin
# --------------------------------------------------------------------------------------

def check_config_cpp(path: Path, ctx: Optional[ModContext] = None) -> List[Issue]:
    try:
        root, issues = cfg.parse_config(path, include_dirs=[ctx.root] if ctx else None)
    except cfg.ConfigError as e:
        return [e.issue]
    issues = issues + cfg.semantic_check(root, str(path))
    _collect_cfg_refs(root, str(path), ctx)
    return issues


def _collect_cfg_refs(root: cfg.ClassNode, file: str, ctx: Optional[ModContext]) -> None:
    if ctx is None:
        return
    for ppath, value, node in cfg.iter_strings(root):
        where = Issue("warning", "", node.file or file, node.line)
        lp = ppath.lower()
        if "cfgsoundshaders" in lp and lp.endswith("/samples[]") or ("cfgsounds/" in lp and lp.endswith("/sound[]")):
            if value and not value.lower().endswith((".ogg", ".wss", ".wav")) and ("\\" in value or "/" in value):
                _add_ref(ctx, value, where, exts=".ogg|.wss")
                continue
        if "cfgmods" in lp and lp.endswith("/files[]"):
            ctx.dir_refs.append((value, where))
            continue
        _add_ref(ctx, value, where)


def check_config_bin(path: Path, ctx: Optional[ModContext] = None) -> List[Issue]:
    data = path.read_bytes()
    if not rap.is_rapified(data):
        return [Issue("error", "файл не является бинарным конфигом (нет сигнатуры raP)", str(path))]
    try:
        root = rap.read_rap(data)
    except Exception as e:
        return [Issue("error", f"повреждённый config.bin: {e}", str(path))]
    issues = [i for i in cfg.semantic_check(root, str(path)) if i.code not in ("undefined-base",)]
    for i in issues:
        i.line = 0
    _collect_cfg_refs(root, str(path), ctx)
    return issues


# --------------------------------------------------------------------------------------
# Скрипты
# --------------------------------------------------------------------------------------

def _unescape_enforce(s: str) -> str:
    return re.sub(r"\\(.)", lambda m: {"n": "\n", "t": "\t"}.get(m.group(1), m.group(1)), s)


def check_script(path: Path, ctx: Optional[ModContext] = None) -> List[Issue]:
    text = cfg.read_text(path)
    issues = enforce.check_script(path, text)
    if ctx is not None:
        lexer_issues: List[Issue] = []
        for t in enforce._Lexer(text, str(path), lexer_issues).run():
            if t.kind == "string" and len(t.text) >= 2 and t.text[0] == '"' and t.text[-1] == '"':
                _add_ref(ctx, _unescape_enforce(t.text[1:-1]), Issue("warning", "", str(path), t.line, t.col))
    return issues


# --------------------------------------------------------------------------------------
# Layout / imageset
# --------------------------------------------------------------------------------------

def check_layout(path: Path, ctx: Optional[ModContext] = None) -> List[Issue]:
    file = str(path)
    text = cfg.read_text(path)
    issues: List[Issue] = []
    stack: List[Tuple[int, int]] = []
    for ln, line in enumerate(text.split("\n"), 1):
        i = 0
        while i < len(line):
            ch = line[i]
            if ch == '"':
                j = line.find('"', i + 1)
                if j < 0:
                    issues.append(Issue("error", "незакрытая строка (нет закрывающей \")", file, ln, i + 1, "string"))
                    break
                value = line[i + 1:j]
                _add_ref(ctx, value, Issue("warning", "", file, ln, i + 1))
                i = j + 1
                continue
            if line.startswith("//", i):
                break
            if ch == "{":
                stack.append((ln, i + 1))
            elif ch == "}":
                if not stack:
                    issues.append(Issue("error", "лишняя закрывающая скобка '}'", file, ln, i + 1, "brackets"))
                else:
                    stack.pop()
            i += 1
    for ln, col in stack:
        issues.append(Issue("error", "не закрыта скобка '{'", file, ln, col, "brackets"))
    if path.suffix.lower() == ".layout" and text.strip():
        first = text.strip().split(None, 1)[0]
        if not first.endswith("WidgetClass"):
            issues.append(Issue("warning", f"layout начинается с '{first}', ожидался класс виджета (...WidgetClass)",
                                file, 1, 1, "layout"))
    return issues


# --------------------------------------------------------------------------------------
# XML
# --------------------------------------------------------------------------------------

def _xml_lines(text: str) -> Dict[str, int]:
    """Номер строки для <type name="X"> / <event name="X"> (ElementTree строки не хранит)."""
    res: Dict[str, int] = {}
    for m in re.finditer(r"<(\w+)\s+name\s*=\s*\"([^\"]*)\"", text):
        res.setdefault(f"{m.group(1)}:{m.group(2)}", text.count("\n", 0, m.start()) + 1)
    return res


def _int(el: Optional[ET.Element], attr: Optional[str] = None) -> Optional[str]:
    if el is None:
        return None
    return el.get(attr) if attr else (el.text or "").strip()


def _num(v: Optional[str], kind=int):
    try:
        return kind(v)
    except (TypeError, ValueError):
        return None


TYPES_CHILDREN = {"nominal", "lifetime", "restock", "min", "quantmin", "quantmax", "cost", "flags", "category",
                  "usage", "value", "tag"}
TYPES_FLAGS = ("count_in_cargo", "count_in_hoarder", "count_in_map", "count_in_player", "crafted", "deloot")


def _check_types_xml(root: ET.Element, file: str, lines: Dict[str, int]) -> List[Issue]:
    issues: List[Issue] = []
    seen: Dict[str, int] = {}
    for t in root.findall("type"):
        name = t.get("name")
        ln = lines.get(f"type:{name}", 0)
        if not name:
            issues.append(Issue("error", "<type> без атрибута name", file, ln, 0, "types"))
            continue
        key = name.lower()
        if key in seen:
            issues.append(Issue("warning", f"тип '{name}' объявлен повторно (строка {seen[key]}) — "
                                           f"действовать будет последнее объявление", file, ln, 0, "types-dup"))
        seen[key] = ln
        for child in t:
            if child.tag not in TYPES_CHILDREN:
                issues.append(Issue("warning", f"'{name}': неизвестный тег <{child.tag}>", file, ln, 0, "types"))
        vals = {}
        for tag in ("nominal", "lifetime", "restock", "min", "quantmin", "quantmax", "cost"):
            el = t.find(tag)
            if el is None:
                continue
            v = _num(_int(el))
            if v is None:
                issues.append(Issue("error", f"'{name}': <{tag}> должно быть целым числом, сейчас '{_int(el)}'",
                                    file, ln, 0, "types"))
            else:
                vals[tag] = v
        if "nominal" in vals and "min" in vals and vals["min"] > vals["nominal"]:
            issues.append(Issue("error", f"'{name}': min ({vals['min']}) больше nominal ({vals['nominal']})",
                                file, ln, 0, "types"))
        for q in ("quantmin", "quantmax"):
            if q in vals and not (vals[q] == -1 or 0 <= vals[q] <= 100):
                issues.append(Issue("error", f"'{name}': {q} должен быть -1 или от 0 до 100", file, ln, 0, "types"))
        if vals.get("quantmin", -1) != -1 and vals.get("quantmax", -1) != -1 and vals["quantmin"] > vals["quantmax"]:
            issues.append(Issue("error", f"'{name}': quantmin больше quantmax", file, ln, 0, "types"))
        if (vals.get("quantmin", -1) == -1) != (vals.get("quantmax", -1) == -1):
            issues.append(Issue("warning", f"'{name}': quantmin и quantmax должны быть оба -1 или оба заданы",
                                file, ln, 0, "types"))
        if vals.get("nominal", 0) > 0 and vals.get("lifetime", 1) <= 0:
            issues.append(Issue("warning", f"'{name}': nominal > 0, но lifetime = {vals.get('lifetime')}",
                                file, ln, 0, "types"))
        if vals.get("nominal", 0) > 0 and t.find("usage") is None and t.find("value") is None:
            issues.append(Issue("info", f"'{name}': nominal > 0, но нет ни <usage>, ни <value> — "
                                        f"предмет может не появляться", file, ln, 0, "types"))
        fl = t.find("flags")
        if fl is not None:
            for a, v in fl.attrib.items():
                if a not in TYPES_FLAGS:
                    issues.append(Issue("warning", f"'{name}': неизвестный флаг {a}", file, ln, 0, "types"))
                elif v not in ("0", "1"):
                    issues.append(Issue("error", f"'{name}': флаг {a} должен быть 0 или 1", file, ln, 0, "types"))
        for tag in ("category", "usage", "value", "tag"):
            for el in t.findall(tag):
                if not el.get("name"):
                    issues.append(Issue("error", f"'{name}': <{tag}> без атрибута name", file, ln, 0, "types"))
    return issues


def _check_events_xml(root: ET.Element, file: str, lines: Dict[str, int]) -> List[Issue]:
    issues: List[Issue] = []
    seen = set()
    for ev in root.findall("event"):
        name = ev.get("name") or ""
        ln = lines.get(f"event:{name}", 0)
        if not name:
            issues.append(Issue("error", "<event> без атрибута name", file, ln, 0, "events"))
        if name.lower() in seen:
            issues.append(Issue("warning", f"событие '{name}' объявлено повторно", file, ln, 0, "events"))
        seen.add(name.lower())
        vals = {}
        for tag in ("nominal", "min", "max", "lifetime", "restock", "saferadius", "distanceradius",
                    "cleanupradius", "active"):
            el = ev.find(tag)
            if el is not None:
                v = _num(_int(el))
                if v is None:
                    issues.append(Issue("error", f"'{name}': <{tag}> должно быть целым числом", file, ln, 0, "events"))
                else:
                    vals[tag] = v
        if "min" in vals and "max" in vals and vals["min"] > vals["max"]:
            issues.append(Issue("error", f"'{name}': min больше max", file, ln, 0, "events"))
        if "active" in vals and vals["active"] not in (0, 1):
            issues.append(Issue("error", f"'{name}': active должен быть 0 или 1", file, ln, 0, "events"))
        ch = ev.find("children")
        if ch is not None:
            for c in ch.findall("child"):
                if not c.get("type"):
                    issues.append(Issue("error", f"'{name}': <child> без type", file, ln, 0, "events"))
                lo, hi = _num(c.get("min")), _num(c.get("max"))
                if lo is not None and hi is not None and lo > hi:
                    issues.append(Issue("error", f"'{name}': у child {c.get('type')} min больше max",
                                        file, ln, 0, "events"))
    return issues


def _check_spawnable_xml(root: ET.Element, file: str, lines: Dict[str, int]) -> List[Issue]:
    issues: List[Issue] = []
    for t in root.findall("type"):
        name = t.get("name") or ""
        ln = lines.get(f"type:{name}", 0)
        for el in t.iter():
            ch = el.get("chance")
            if ch is not None:
                v = _num(ch, float)
                if v is None or not 0 <= v <= 1:
                    issues.append(Issue("error", f"'{name}': chance='{ch}' должен быть от 0 до 1 (<{el.tag}>)",
                                        file, ln, 0, "spawnable"))
            if el.tag == "item" and not el.get("name"):
                issues.append(Issue("error", f"'{name}': <item> без name", file, ln, 0, "spawnable"))
    return issues


def _check_globals_xml(root: ET.Element, file: str, lines: Dict[str, int]) -> List[Issue]:
    issues: List[Issue] = []
    for v in root.findall("var"):
        name, typ, val = v.get("name"), v.get("type"), v.get("value")
        ln = lines.get(f"var:{name}", 0)
        if not name or typ is None or val is None:
            issues.append(Issue("error", "<var> должен иметь name, type и value", file, ln, 0, "globals"))
            continue
        if typ == "0" and _num(val) is None:
            issues.append(Issue("error", f"'{name}': type=0 (целое), но value='{val}'", file, ln, 0, "globals"))
        if typ == "1" and _num(val, float) is None:
            issues.append(Issue("error", f"'{name}': type=1 (дробное), но value='{val}'", file, ln, 0, "globals"))
    return issues


XML_RULES: Dict[str, Tuple[str, Callable]] = {
    "types": ("types", _check_types_xml),
    "events": ("events", _check_events_xml),
    "spawnabletypes": ("spawnabletypes", _check_spawnable_xml),
    "variables": ("variables", _check_globals_xml),
}


def check_xml(path: Path, ctx: Optional[ModContext] = None) -> List[Issue]:
    file = str(path)
    raw = path.read_bytes()
    issues: List[Issue] = []
    try:
        text = raw.decode("utf-8-sig")
    except UnicodeDecodeError:
        issues.append(Issue("warning", "файл не в UTF-8", file, 0, 0, "encoding"))
        text = raw.decode("cp1251", "replace")
    try:
        root = ET.fromstring(text.encode("utf-8") if text.lstrip().startswith("<?xml") else text)
    except ET.ParseError as e:
        line, col = getattr(e, "position", (0, 0))
        msg = str(e).split(":")[0]
        hints = {
            "mismatched tag": "несовпадающий закрывающий тег",
            "not well-formed (invalid token)": "недопустимый символ (например, & без &amp; или лишняя <)",
            "unclosed token": "незакрытый тег или атрибут",
            "no element found": "файл обрывается: не закрыт корневой тег",
            "junk after document element": "лишнее содержимое после корневого тега",
            "duplicate attribute": "повторяющийся атрибут",
            "syntax error": "синтаксическая ошибка",
        }
        return [Issue("error", f"XML: {hints.get(msg, msg)}", file, line, col + 1, "xml")]
    lines = _xml_lines(text)
    for tag, (expected, fn) in XML_RULES.items():
        if root.tag == expected:
            issues += fn(root, file, lines)
    if ctx is not None:
        for el in root.iter():
            for a, v in el.attrib.items():
                pos = text.find(f'"{v}"')
                ln = text.count("\n", 0, pos) + 1 if pos >= 0 else 0
                _add_ref(ctx, v, Issue("warning", "", file, ln))
    return issues


# --------------------------------------------------------------------------------------
# JSON
# --------------------------------------------------------------------------------------

def check_json(path: Path, ctx: Optional[ModContext] = None) -> List[Issue]:
    file = str(path)
    raw = path.read_bytes()
    try:
        text = raw.decode("utf-8-sig")
    except UnicodeDecodeError:
        return [Issue("error", "файл не в UTF-8", file, 0, 0, "encoding")]
    issues: List[Issue] = []

    def hook(pairs):
        keys = set()
        for k, _ in pairs:
            if k in keys:
                issues.append(Issue("warning", f"ключ \"{k}\" повторяется в одном объекте", file, 0, 0, "json-dup"))
            keys.add(k)
        return dict(pairs)

    try:
        json.loads(text, object_pairs_hook=hook)
    except json.JSONDecodeError as e:
        msg = e.msg
        tr = {
            "Expecting ',' delimiter": "пропущена запятая",
            "Expecting property name enclosed in double quotes": "ожидалось имя в двойных кавычках "
                                                                 "(лишняя запятая перед } или одинарные кавычки?)",
            "Expecting value": "ожидалось значение (лишняя запятая, комментарий или пропущено значение)",
            "Expecting ':' delimiter": "пропущено двоеточие",
            "Unterminated string starting at": "незакрытая строка",
            "Extra data": "лишние данные после конца JSON",
            "Invalid control character at": "управляющий символ внутри строки",
        }
        if re.search(r"^\s*//|/\*", text, re.M):
            msg_extra = " (комментарии в JSON не допускаются)"
        else:
            msg_extra = ""
        return [Issue("error", f"JSON: {tr.get(msg, msg)}{msg_extra}", file, e.lineno, e.colno, "json")]
    return issues


# --------------------------------------------------------------------------------------
# stringtable.csv
# --------------------------------------------------------------------------------------

def check_stringtable(path: Path, ctx: Optional[ModContext] = None) -> List[Issue]:
    file = str(path)
    raw = path.read_bytes()
    issues: List[Issue] = []
    try:
        text = raw.decode("utf-8-sig")
    except UnicodeDecodeError:
        return [Issue("error", "stringtable.csv должен быть в кодировке UTF-8", file, 0, 0, "encoding")]
    rows = []
    reader = csv.reader(io.StringIO(text), strict=True)
    try:
        for row in reader:
            rows.append((reader.line_num, row))
    except csv.Error as e:
        return [Issue("error", f"CSV: {e} (проверьте кавычки: внутри строки кавычка пишется как \"\")",
                      file, reader.line_num, 0, "csv")]
    if not rows:
        return [Issue("error", "файл пустой", file)]
    header = rows[0][1]
    if not header or header[0].strip().lower() != "language":
        issues.append(Issue("error", "первая колонка заголовка должна называться \"Language\"", file, 1, 1, "csv"))
    ncols = len(header)
    seen: Dict[str, int] = {}
    keys = set()
    for ln, row in rows[1:]:
        if not row or all(not c.strip() for c in row):
            continue
        if len(row) != ncols:
            issues.append(Issue("error", f"колонок {len(row)}, а в заголовке {ncols}", file, ln, 0, "csv"))
        key = row[0].strip()
        if not key:
            issues.append(Issue("error", "пустой ключ строки", file, ln, 1, "csv"))
            continue
        if key != row[0]:
            issues.append(Issue("warning", f"пробелы вокруг ключа '{key}'", file, ln, 1, "csv"))
        if key.lower() in seen:
            issues.append(Issue("warning", f"ключ '{key}' повторяется (строка {seen[key.lower()]})",
                                file, ln, 1, "csv-dup"))
        seen[key.lower()] = ln
        keys.add(key)
        if len(row) > 1 and not row[1].strip():
            issues.append(Issue("warning", f"'{key}': пустой текст в колонке original", file, ln, 2, "csv"))
    if ctx is not None:
        ctx.has_stringtable = True
        ctx.str_keys |= {k.lower() for k in keys}
    return issues


# --------------------------------------------------------------------------------------
# PAA / OGG
# --------------------------------------------------------------------------------------

def check_paa(path: Path, ctx: Optional[ModContext] = None) -> List[Issue]:
    file = str(path)
    try:
        info = paa.parse(path.read_bytes(), load_all=True)
    except Exception as e:
        return [Issue("error", f"повреждённый PAA: {e}", file, 0, 0, "paa")]
    issues: List[Issue] = []
    w, h = info.width, info.height
    if not (paa.is_pow2(w) and paa.is_pow2(h)):
        issues.append(Issue("error", f"размер {w}x{h}: стороны должны быть степенью двойки", file, 0, 0, "paa"))
    if max(w, h) > 4096:
        issues.append(Issue("warning", f"очень большая текстура {w}x{h}", file, 0, 0, "paa"))
    if len(info.mipmaps) == 1 and max(w, h) > 4:
        issues.append(Issue("warning", "нет mip-уровней — текстура будет мерцать вдали", file, 0, 0, "paa"))
    pw, ph = w, h
    for m in info.mipmaps[1:]:
        if (m.width, m.height) != (max(1, pw // 2), max(1, ph // 2)):
            issues.append(Issue("error", f"нарушена цепочка mip-уровней: {pw}x{ph} -> {m.width}x{m.height}",
                                file, 0, 0, "paa"))
            break
        pw, ph = m.width, m.height
    expect = paa.fmt_from_suffix(path.stem)
    if expect == "dxt1" and info.type == paa.TYPE_DXT5:
        issues.append(Issue("info", "суффикс _co обычно означает DXT1 без прозрачности, а файл DXT5",
                            file, 0, 0, "paa"))
    if expect == "dxt5" and info.type == paa.TYPE_DXT1 and path.stem.lower().endswith("_ca"):
        issues.append(Issue("warning", "суффикс _ca означает текстуру с прозрачностью (DXT5), а файл DXT1",
                            file, 0, 0, "paa"))
    return issues


def check_ogg(path: Path, ctx: Optional[ModContext] = None, ffmpeg: Optional[str] = None) -> List[Issue]:
    file = str(path)
    data = path.read_bytes()[:64]
    if not data.startswith(b"OggS"):
        return [Issue("error", "это не OGG-файл (нет сигнатуры OggS) — возможно, MP3 с переименованным "
                               "расширением", file, 0, 0, "ogg")]
    if not ffmpeg:
        return []
    from .common import run_ffmpeg
    p = run_ffmpeg([ffmpeg, "-hide_banner", "-i", str(path)])
    m = re.search(r"Audio:\s*(\w+),\s*(\d+)\s*Hz,\s*([\w.]+)", p.stderr or "")
    if not m:
        return [Issue("error", "не удалось прочитать аудиопоток", file, 0, 0, "ogg")]
    codec, rate, ch = m.group(1), int(m.group(2)), m.group(3)
    issues: List[Issue] = []
    if codec != "vorbis":
        issues.append(Issue("error", f"кодек {codec}: DayZ воспроизводит OGG только с Vorbis", file, 0, 0, "ogg"))
    if rate not in (22050, 44100, 48000):
        issues.append(Issue("warning", f"частота {rate} Гц — рекомендуется 44100", file, 0, 0, "ogg"))
    if ch not in ("mono", "stereo"):
        issues.append(Issue("warning", f"каналы: {ch} — используйте моно (3D) или стерео (2D)", file, 0, 0, "ogg"))
    return issues


# --------------------------------------------------------------------------------------
# Диспетчер
# --------------------------------------------------------------------------------------

def checker_for(path: Path) -> Optional[Callable]:
    name = path.name.lower()
    ext = path.suffix.lower()
    if name == "config.cpp":
        return check_config_cpp
    if name == "config.bin" or (ext == ".bin" and path.read_bytes()[:4] == rap.MAGIC):
        return check_config_bin
    if name == "stringtable.csv":
        return check_stringtable
    return {
        ".c": check_script, ".layout": check_layout, ".imageset": check_layout, ".styles": check_layout,
        ".xml": check_xml, ".json": check_json, ".paa": check_paa, ".ogg": check_ogg,
    }.get(ext)


def check_file(path: Path, ctx: Optional[ModContext] = None, ffmpeg: Optional[str] = None) -> List[Issue]:
    fn = checker_for(path)
    if fn is None:
        return []
    try:
        if fn is check_ogg:
            return fn(path, ctx, ffmpeg)
        return fn(path, ctx)
    except Exception as e:  # защита от неожиданных сбоев анализатора
        return [Issue("error", f"не удалось проверить файл: {e}", str(path), 0, 0, "internal")]


def _bad_name(rel: str) -> Optional[str]:
    if " " in rel:
        return "пробел в пути"
    if re.search(r"[^\x00-\x7F]", rel):
        return "не-латинские символы в пути"
    return None


def cross_checks(ctx: ModContext) -> List[Issue]:
    issues: List[Issue] = []
    # ссылки на файлы
    reported = set()
    for ref, exts, where in ctx.refs:
        rel = ctx.resolve(ref)
        if rel is None:
            continue
        candidates = [rel] if not exts else [rel + e for e in exts.split("|")]
        if any(c in ctx.index for c in candidates):
            continue
        key = (rel, where.file, where.line)
        if key in reported:
            continue
        reported.add(key)
        issues.append(Issue("warning", f"файл не найден: {ref}" + (f" ({exts.replace('|', ' или ')})" if exts else ""),
                            where.file, where.line, where.col, "missing-file"))
    for ref, where in ctx.dir_refs:
        rel = ctx.resolve(ref)
        if rel is None:
            continue
        if rel.rstrip("/") not in ctx.dirs:
            issues.append(Issue("error", f"папка скриптов не найдена: {ref}", where.file, where.line, 0,
                                "missing-dir"))
    # ключи локализации
    if ctx.has_stringtable and ctx.str_keys:
        prefixes = {"_".join(k.split("_")[:2]) + "_" for k in ctx.str_keys if k.count("_") >= 2}
        missing: Dict[str, Issue] = {}
        for key, where in ctx.str_refs:
            k = key.lower()
            if k in ctx.str_keys:
                continue
            if not any(k.startswith(p) for p in prefixes):
                continue  # вероятно, ключ из ванильной игры
            missing.setdefault(k, Issue("warning", f"ключ #{key} отсутствует в stringtable.csv",
                                        where.file, where.line, where.col, "missing-string"))
        issues += list(missing.values())
    # имена файлов
    for rel, p in ctx.index.items():
        if p.suffix.lower() in CHECK_EXTENSIONS or p.suffix.lower() in (".p3d", ".rvmat", ".edds", ".wss"):
            real = p.relative_to(ctx.root).as_posix()
            why = _bad_name(real)
            if why:
                issues.append(Issue("warning", f"{why}: {real} — в PBO и путях DayZ используйте латиницу без "
                                               f"пробелов", str(p), 0, 0, "filename"))
    return issues


@dataclass
class CheckReport:
    files: int = 0
    issues: List[Issue] = field(default_factory=list)

    def count(self, level: str) -> int:
        return sum(1 for i in self.issues if i.level == level)


def check_paths(paths: Iterable[str], ffmpeg: Optional[str] = None,
                on_file: Optional[Callable[[Path, List[Issue], int, int], None]] = None,
                cross: bool = True) -> CheckReport:
    """Проверяет файлы и папки. Для папок выполняются также перекрёстные проверки мода."""
    report = CheckReport()
    groups: List[Tuple[Optional[ModContext], List[Path]]] = []
    for raw in paths:
        p = Path(raw)
        if p.is_dir():
            ctx = ModContext.build(p) if cross else None
            files = [f for f in sorted(p.rglob("*"))
                     if f.is_file() and not any(part in SKIP_DIRS for part in f.relative_to(p).parts)
                     and checker_for(f) is not None]
            groups.append((ctx, files))
        elif p.is_file():
            groups.append((None, [p]))
    total = sum(len(g[1]) for g in groups)
    done = 0
    for ctx, files in groups:
        # stringtable первым: ключи нужны для перекрёстной проверки
        files = sorted(files, key=lambda f: f.name.lower() != "stringtable.csv")
        for f in files:
            iss = check_file(f, ctx, ffmpeg)
            report.issues += iss
            report.files += 1
            done += 1
            if on_file:
                on_file(f, iss, done, total)
        if ctx is not None:
            report.issues += cross_checks(ctx)
    return report
