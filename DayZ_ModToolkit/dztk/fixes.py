# -*- coding: utf-8 -*-
"""Автоисправление типовых проблем мода (по результатам проверки).

Что исправляется:
  * необъявленный базовый класс в config.cpp -> строка «class X;» перед наследником;
  * нет ';' после '}' класса -> добавляется ';';
  * ключ #STR_... отсутствует в stringtable.csv -> добавляется строка-заготовка;
  * пробелы/кириллица в именах файлов -> файл переименовывается, ссылки во всех текстовых файлах мода
    (скрипты, конфиги, layout, imageset, xml, json, rvmat) исправляются.
Перед изменением каждый файл копируется в резервную папку рядом с модом.
"""

from __future__ import annotations

from .i18n import tr

import csv
import io
import re
import shutil
import time
from dataclasses import dataclass, field
from pathlib import Path
from typing import Callable, Dict, List, Optional

from .cfg import Issue, read_text
from .common import sanitize_name

TEXT_EXT = {".c", ".cpp", ".hpp", ".h", ".layout", ".imageset", ".styles", ".xml", ".json", ".rvmat", ".csv",
            ".txt", ".ext", ".sqm"}


@dataclass
class Fix:
    kind: str
    description: str
    file: str
    apply: Callable[["FixContext"], None]
    enabled: bool = True
    line: int = 0


@dataclass
class FixContext:
    root: Path
    backup: Optional[Path]
    changed: List[str] = field(default_factory=list)

    def backup_file(self, p: Path) -> None:
        if self.backup is None or not p.exists():
            return
        rel = p.resolve().relative_to(self.root.resolve())
        dst = self.backup / rel
        if not dst.exists():
            dst.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(p, dst)

    def write(self, p: Path, text: str) -> None:
        self.backup_file(p)
        raw = p.read_bytes()
        newline = "\r\n" if b"\r\n" in raw else "\n"
        bom = raw.startswith(b"\xef\xbb\xbf")
        data = text.replace("\n", newline).encode("utf-8")
        p.write_bytes((b"\xef\xbb\xbf" if bom else b"") + data)
        if str(p) not in self.changed:
            self.changed.append(str(p))


def _lines(p: Path) -> List[str]:
    return read_text(p).split("\n")


# --------------------------------------------------------------------------------------
# Отдельные исправления
# --------------------------------------------------------------------------------------

def _fix_undefined_base(issue: Issue) -> Optional[Fix]:
    m = re.search(r"'([^']+)' для '([^']+)'", issue.message) or re.search(r"'([^']+)' for '([^']+)'", issue.message)
    if not m or not issue.file.lower().endswith((".cpp", ".hpp")) or not issue.line:
        return None
    base, cls = m.group(1), m.group(2)
    path, line = Path(issue.file), issue.line

    def apply(ctx: FixContext) -> None:
        lines = _lines(path)
        i = line - 1
        if i >= len(lines) or not re.search(r"\bclass\s+%s\b" % re.escape(cls), lines[i]):
            # файл уже менялся — ищем объявление заново
            found = [k for k, t in enumerate(lines) if re.search(r"\bclass\s+%s\s*:\s*%s\b" % (re.escape(cls),
                                                                                              re.escape(base)), t)]
            if not found:
                return
            i = found[0]
        if re.search(r"\bclass\s+%s\s*;" % re.escape(base), "\n".join(lines[max(0, i - 50):i])):
            return   # уже добавлено для соседнего класса
        indent = re.match(r"\s*", lines[i]).group(0)
        # несколько классов на одной строке — вставляем перед строкой
        lines.insert(i, f"{indent}class {base};")
        ctx.write(path, "\n".join(lines))

    return Fix("base", tr("{0}: добавить «class {1};» перед классом {2}").format(path.name, base, cls), str(path),
               apply, line=line)


def _fix_class_semicolon(issue: Issue) -> Optional[Fix]:
    if not issue.file.lower().endswith((".cpp", ".hpp")) or not issue.line:
        return None
    path, line, col = Path(issue.file), issue.line, issue.col

    def apply(ctx: FixContext) -> None:
        lines = _lines(path)
        # ';' ставится сразу после последней '}' перед местом, где парсер её ждал
        li, ci = line - 1, max(0, col - 1)
        while li >= 0:
            seg = lines[li][:ci] if li == line - 1 else lines[li]
            k = seg.rfind("}")
            if k >= 0:
                lines[li] = lines[li][:k + 1] + ";" + lines[li][k + 1:]
                ctx.write(path, "\n".join(lines))
                return
            li -= 1

    return Fix("semicolon", tr("{0}:{1}: поставить ';' после '}}' класса").format(path.name, line), str(path), apply,
               line=line * 1000 + col)


def _fix_missing_strings(keys: List[str], table: Path) -> Fix:
    def apply(ctx: FixContext) -> None:
        text = read_text(table)
        rows = list(csv.reader(io.StringIO(text)))
        ncols = len(rows[0]) if rows else 2
        have = {r[0].strip().lower() for r in rows[1:] if r}
        buf = io.StringIO()
        w = csv.writer(buf, quoting=csv.QUOTE_ALL, lineterminator="\n")
        for k in keys:
            if k.lower() not in have:
                w.writerow([k] + ["TODO " + k] * (ncols - 1))
        add = buf.getvalue()
        if add:
            ctx.write(table, text.rstrip("\n") + "\n" + add)

    return Fix("strings", tr("stringtable.csv: добавить заготовки для {0} ключей ({1})").format(
        len(keys), ", ".join(keys[:4]) + ("..." if len(keys) > 4 else "")), str(table), apply)


def _fix_csv_columns(table: Path) -> Fix:
    def apply(ctx: FixContext) -> None:
        text = read_text(table)
        rows = list(csv.reader(io.StringIO(text)))
        if not rows:
            return
        n = len(rows[0])
        changed = False
        for r in rows[1:]:
            if r and len(r) < n:
                r += [r[1] if len(r) > 1 else r[0]] * (n - len(r))   # пустые языки — текстом из original
                changed = True
        if changed:
            buf = io.StringIO()
            csv.writer(buf, quoting=csv.QUOTE_ALL, lineterminator="\n").writerows(rows)
            ctx.write(table, buf.getvalue())

    return Fix("csv", tr("{0}: дополнить строки с недостающими колонками текстом из original").format(table.name),
               str(table), apply)


def _ref_variants(prefix: str, rel_old: str, rel_new: str) -> List[tuple]:
    """Варианты записи пути в файлах мода: с префиксом, со слешами / и \\, с экранированием для скриптов."""
    out = []
    for with_ext in (True, False):
        o = rel_old if with_ext else rel_old.rsplit(".", 1)[0]
        n = rel_new if with_ext else rel_new.rsplit(".", 1)[0]
        for pref in ([prefix] if prefix else []) + [""]:
            po = (pref.replace("\\", "/").strip("/") + "/" if pref else "") + o
            pn = (pref.replace("\\", "/").strip("/") + "/" if pref else "") + n
            if not pref and "/" not in o:
                continue   # голое имя файла без папок заменять опасно
            out.append((po, pn))
            out.append((po.replace("/", "\\"), pn.replace("/", "\\")))
            out.append((po.replace("/", "\\\\"), pn.replace("/", "\\\\")))
    # длинные варианты раньше коротких
    return sorted(set(out), key=lambda x: -len(x[0]))


def _fix_rename(root: Path, prefix: str, path: Path) -> Optional[Fix]:
    rel = path.relative_to(root).as_posix()
    parts = rel.split("/")
    new_parts = [sanitize_name(Path(p).stem) + Path(p).suffix.lower() if i == len(parts) - 1 else
                 (sanitize_name(p) if (" " in p or re.search(r"[^\x00-\x7F]", p)) else p)
                 for i, p in enumerate(parts)]
    new_rel = "/".join(new_parts)
    if new_rel == rel:
        return None

    def apply(ctx: FixContext) -> None:
        src = root / rel
        dst = root / new_rel
        if not src.exists() or dst.exists():
            return
        # ссылки во всех текстовых файлах
        variants = _ref_variants(prefix, rel, new_rel)
        for f in root.rglob("*"):
            if not f.is_file() or f.suffix.lower() not in TEXT_EXT:
                continue
            if ctx.backup is not None and ctx.backup in f.parents:
                continue
            text = read_text(f)
            new = text
            for o, n in variants:
                new = re.sub(re.escape(o), lambda _m, n=n: n, new, flags=re.I)
            if new != text:
                ctx.write(f, new)
        ctx.backup_file(src)
        dst.parent.mkdir(parents=True, exist_ok=True)
        src.rename(dst)
        ctx.changed.append(f"{rel} -> {new_rel}")

    return Fix("rename", tr("переименовать {0} -> {1} и исправить ссылки").format(rel, new_rel), str(path), apply)


# --------------------------------------------------------------------------------------
# План и применение
# --------------------------------------------------------------------------------------

def plan(root, issues: List[Issue]) -> List[Fix]:
    from .checks import detect_prefix
    root = Path(root).resolve()
    fixes: List[Fix] = []
    seen = set()
    missing_keys: List[str] = []
    for i in issues:
        key = (i.code, i.file, i.line, i.message)
        if key in seen:
            continue
        seen.add(key)
        fx = None
        if i.code == "undefined-base":
            fx = _fix_undefined_base(i)
        elif i.code == "class-semicolon":
            fx = _fix_class_semicolon(i)
        elif i.code == "missing-string":
            m = re.search(r"#(STR_[A-Za-z0-9_]+)", i.message)
            if m and m.group(1) not in missing_keys:
                missing_keys.append(m.group(1))
        elif i.code == "csv" and ("колонок" in i.message or "columns" in i.message) and \
                not any(f.kind == "csv" for f in fixes):
            fx = _fix_csv_columns(Path(i.file))
        elif i.code == "filename":
            p = Path(i.file)
            if p.exists():
                fx = _fix_rename(root, detect_prefix(root), p)
        if fx is not None:
            fixes.append(fx)
    if missing_keys:
        table = next((p for p in root.rglob("stringtable.csv")), None)
        if table is not None:
            fixes.append(_fix_missing_strings(missing_keys, table))
    # одинаковые «class X;» для одного файла и базы — один раз
    uniq, out = set(), []
    for f in fixes:
        k = (f.kind, f.description) if f.kind == "base" else (id(f),)
        if k not in uniq:
            uniq.add(k)
            out.append(f)
    # переименования — последними (после правок содержимого)
    return sorted(out, key=lambda f: f.kind == "rename")


def apply(root, fixes: List[Fix], backup: bool = True) -> FixContext:
    root = Path(root).resolve()
    bdir = root.parent / f"{root.name}_backup_{time.strftime('%Y%m%d_%H%M%S')}" if backup else None
    ctx = FixContext(root, bdir)
    # ';' не меняют число строк — их первыми (снизу вверх), затем вставки «class X;» снизу вверх
    semis = sorted((f for f in fixes if f.enabled and f.kind == "semicolon"), key=lambda f: -_line_hint(f))
    bases = sorted((f for f in fixes if f.enabled and f.kind == "base"), key=lambda f: -_line_hint(f))
    for f in semis + bases:
        f.apply(ctx)
    for f in fixes:
        if f.enabled and f.kind not in ("base", "semicolon"):
            f.apply(ctx)
    return ctx


def _line_hint(f: Fix) -> int:
    return f.line
