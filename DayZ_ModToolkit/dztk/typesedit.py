# -*- coding: utf-8 -*-
"""Табличная правка types.xml: чтение, фильтр, массовые изменения, сохранение.

Файл правится на уровне текста: меняются только значения внутри изменённых <type>, поэтому
комментарии, порядок, отступы и закомментированные записи сохраняются как были.
"""

from __future__ import annotations

from .i18n import tr

import fnmatch
import re
import xml.etree.ElementTree as ET
from dataclasses import dataclass, field
from pathlib import Path
from typing import Dict, Iterable, List, Optional, Tuple

NUM_FIELDS = ("nominal", "lifetime", "restock", "min", "quantmin", "quantmax", "cost")
FLAG_FIELDS = ("count_in_cargo", "count_in_hoarder", "count_in_map", "count_in_player", "crafted", "deloot")
LIST_FIELDS = ("category", "usage", "value", "tag")
ALL_FIELDS = NUM_FIELDS + FLAG_FIELDS + LIST_FIELDS

_TYPE = re.compile(r"<type\s+name\s*=\s*\"([^\"]*)\"\s*>(.*?)</type\s*>|<type\s+name\s*=\s*\"([^\"]*)\"\s*/>", re.S)
_COMMENT = re.compile(r"<!--.*?-->", re.S)


class TypesError(ValueError):
    pass


@dataclass
class TypeRow:
    name: str
    values: Dict[str, str] = field(default_factory=dict)    # числа и флаги (строки как в файле)
    lists: Dict[str, List[str]] = field(default_factory=dict)  # category/usage/value/tag
    user: Dict[str, List[str]] = field(default_factory=dict)   # usage/value с user="..."
    span: Tuple[int, int] = (0, 0)
    changed: set = field(default_factory=set)

    @property
    def dirty(self) -> bool:
        return bool(self.changed)

    def get(self, key: str) -> str:
        if key == "name":
            return self.name
        if key in LIST_FIELDS:
            return ", ".join(self.lists.get(key, []) + ["@" + u for u in self.user.get(key, [])])
        return self.values.get(key, "")

    def num(self, key: str) -> Optional[float]:
        try:
            return float(self.values.get(key, ""))
        except ValueError:
            return None


@dataclass
class TypesFile:
    path: Optional[Path]
    text: str
    rows: List[TypeRow]

    @property
    def dirty(self) -> bool:
        return any(r.dirty for r in self.rows)


def _parse_body(name: str, body: str) -> TypeRow:
    r = TypeRow(name)
    try:
        el = ET.fromstring(f"<type>{body}</type>")
    except ET.ParseError as e:
        raise TypesError(tr("тип {0}: ошибка XML ({1})").format(name, e))
    for c in el:
        if c.tag in NUM_FIELDS:
            r.values[c.tag] = (c.text or "").strip()
        elif c.tag == "flags":
            for k in FLAG_FIELDS:
                if c.get(k) is not None:
                    r.values[k] = c.get(k)
        elif c.tag in LIST_FIELDS:
            if c.get("name") is not None:
                r.lists.setdefault(c.tag, []).append(c.get("name"))
            if c.get("user") is not None:
                r.user.setdefault(c.tag, []).append(c.get("user"))
    return r


def parse(text: str, path=None) -> TypesFile:
    rows = []
    masked = _COMMENT.sub(lambda m: " " * len(m.group(0)), text)   # закомментированные типы не трогаем
    for m in _TYPE.finditer(masked):
        name = m.group(1) if m.group(1) is not None else m.group(3)
        body = text[m.start(2):m.end(2)] if m.group(2) is not None else ""
        row = _parse_body(name, _COMMENT.sub("", body))
        row.span = (m.start(), m.end())
        rows.append(row)
    if not rows and "<types" not in masked:
        raise TypesError(tr("это не types.xml: нет элемента <types>"))
    return TypesFile(Path(path) if path else None, text, rows)


def load(path) -> TypesFile:
    return parse(Path(path).read_bytes().decode("utf-8-sig", "replace"), path)


# --------------------------------------------------------------------------------------
# Фильтр и массовые правки
# --------------------------------------------------------------------------------------

def matches(row: TypeRow, name: str = "", category: str = "", usage: str = "", value: str = "",
            tag: str = "") -> bool:
    if name:
        pat = name if any(ch in name for ch in "*?[") else f"*{name}*"
        if not fnmatch.fnmatch(row.name.lower(), pat.lower()):
            return False
    for key, want in (("category", category), ("usage", usage), ("value", value), ("tag", tag)):
        if want:
            have = [x.lower() for x in row.lists.get(key, []) + row.user.get(key, [])]
            if want == "-":
                if have:
                    return False
            elif want.lower() not in have:
                return False
    return True


def _fmt(v: float) -> str:
    return str(int(round(v)))


def set_value(row: TypeRow, key: str, value: str) -> bool:
    """Установить поле. Для списков — значения через запятую (@имя — user-набор)."""
    value = value.strip()
    if key == "name":
        if value and value != row.name:
            row.name = value
            row.changed.add("name")
            return True
        return False
    if key in LIST_FIELDS:
        items = [x.strip() for x in value.split(",") if x.strip()]
        names = [x for x in items if not x.startswith("@")]
        users = [x[1:] for x in items if x.startswith("@")]
        if key == "category" and len(names) > 1:
            raise TypesError(tr("у типа может быть только одна категория"))
        if names == row.lists.get(key, []) and users == row.user.get(key, []):
            return False
        row.lists[key], row.user[key] = names, users
        row.changed.add(key)
        return True
    if key not in NUM_FIELDS + FLAG_FIELDS:
        raise TypesError(tr("неизвестное поле {0}").format(key))
    if value and not re.fullmatch(r"-?\d+", value):
        raise TypesError(tr("{0}: нужно целое число, а не «{1}»").format(key, value))
    if key in FLAG_FIELDS and value not in ("", "0", "1"):
        raise TypesError(tr("{0}: флаг может быть только 0 или 1").format(key))
    if row.values.get(key, "") == value:
        return False
    if value:
        row.values[key] = value
    else:
        row.values.pop(key, None)
    row.changed.add("flags" if key in FLAG_FIELDS else key)
    return True


def bulk(rows: Iterable[TypeRow], key: str, op: str, arg: str) -> int:
    """op: set | mul | add | addlist | remlist. Возвращает число изменённых типов."""
    n = 0
    for r in rows:
        if op == "set":
            ch = set_value(r, key, arg)
        elif op in ("mul", "add"):
            if key not in NUM_FIELDS:
                raise TypesError(tr("умножать и прибавлять можно только числовые поля"))
            cur = r.num(key)
            if cur is None or (key in ("quantmin", "quantmax") and cur < 0):
                continue
            new = cur * float(arg) if op == "mul" else cur + float(arg)
            if key in ("nominal", "min", "lifetime", "restock", "cost"):
                new = max(0, new)
            if key in ("quantmin", "quantmax"):
                new = min(100, max(0, new))
            ch = set_value(r, key, _fmt(new))
        elif op in ("addlist", "remlist"):
            if key not in LIST_FIELDS:
                raise TypesError(tr("добавлять и убирать можно только category/usage/value/tag"))
            cur = r.get(key)
            items = [x.strip() for x in cur.split(",") if x.strip()]
            if op == "addlist":
                items += [x for x in (y.strip() for y in arg.split(",")) if x and x not in items]
            else:
                rm = {x.strip().lower() for x in arg.split(",")}
                items = [x for x in items if x.lower() not in rm and x.lstrip("@").lower() not in rm]
            ch = set_value(r, key, ", ".join(items))
        else:
            raise TypesError(tr("неизвестная операция {0}").format(op))
        n += bool(ch)
    return n


def validate(row: TypeRow) -> List[str]:
    """Логические ошибки значений (как в проверке types.xml)."""
    out = []
    nom, mn = row.num("nominal"), row.num("min")
    if nom is not None and mn is not None and nom > 0 and mn > nom:
        out.append(tr("min больше nominal"))
    qmin, qmax = row.num("quantmin"), row.num("quantmax")
    if qmin is not None and qmax is not None and qmin > qmax >= 0:
        out.append(tr("quantmin больше quantmax"))
    if (qmin is not None and qmin >= 0) != (qmax is not None and qmax >= 0) and qmin is not None and qmax is not None:
        out.append(tr("quantmin и quantmax должны быть оба -1 или оба заданы"))
    return out


# --------------------------------------------------------------------------------------
# Запись
# --------------------------------------------------------------------------------------

def _indent_of(text: str, pos: int) -> str:
    ls = text.rfind("\n", 0, pos) + 1
    return re.match(r"[ \t]*", text[ls:pos]).group(0)


def render_type(row: TypeRow, indent: str = "    ", step: str = "    ") -> str:
    i2 = indent + step
    lines = [f'{indent}<type name="{_esc(row.name)}">']
    for k in NUM_FIELDS:
        if k in row.values:
            lines.append(f"{i2}<{k}>{row.values[k]}</{k}>")
    if any(k in row.values for k in FLAG_FIELDS):
        attrs = " ".join(f'{k}="{row.values[k]}"' for k in FLAG_FIELDS if k in row.values)
        lines.append(f"{i2}<flags {attrs}/>")
    for k in LIST_FIELDS:
        for v in row.lists.get(k, []):
            lines.append(f'{i2}<{k} name="{_esc(v)}"/>')
        for v in row.user.get(k, []):
            lines.append(f'{i2}<{k} user="{_esc(v)}"/>')
    lines.append(f"{indent}</type>")
    return "\n".join(lines)


def _esc(s: str) -> str:
    return s.replace("&", "&amp;").replace('"', "&quot;").replace("<", "&lt;")


def _rewrite(block: str, row: TypeRow, nl: str, ind: str, step: str) -> str:
    """Меняет в исходном тексте <type> только изменённые поля."""
    if "<type" in block and "/>" in block[:block.find(">") + 1]:
        return render_type(row, "", step).replace("\n", nl + ind)       # <type name="x"/> — пустой
    i2 = ind + step
    for key in sorted(row.changed):
        if key == "name":
            block = re.sub(r'(<type\s+name\s*=\s*")[^"]*(")', lambda m: m.group(1) + _esc(row.name) + m.group(2),
                           block, count=1)
            continue
        if key == "flags":
            new = ""
            if any(k in row.values for k in FLAG_FIELDS):
                new = "<flags " + " ".join(f'{k}="{row.values[k]}"' for k in FLAG_FIELDS if k in row.values) + "/>"
            block = _replace_elems(block, r"<flags\b[^>]*/>|<flags\b[^>]*>.*?</flags\s*>", [new] if new else [],
                                   nl, i2)
            continue
        if key in NUM_FIELDS:
            new = [f"<{key}>{row.values[key]}</{key}>"] if key in row.values else []
            block = _replace_elems(block, r"<%s\s*>.*?</%s\s*>|<%s\s*/>" % (key, key, key), new, nl, i2)
            continue
        new = [f'<{key} name="{_esc(v)}"/>' for v in row.lists.get(key, [])] + \
              [f'<{key} user="{_esc(v)}"/>' for v in row.user.get(key, [])]
        block = _replace_elems(block, r"<%s\b[^>]*/>|<%s\b[^>]*>.*?</%s\s*>" % (key, key, key), new, nl, i2)
    return block


def _replace_elems(block: str, pattern: str, new: List[str], nl: str, indent: str) -> str:
    """Заменяет все вхождения элемента на new (на месте первого; нет — перед </type>)."""
    rx = re.compile(r"[ \t]*(?:%s)[ \t]*(?:\r?\n)?" % pattern, re.S)
    found = list(rx.finditer(block))
    text = nl.join(indent + x for x in new) + (nl if new else "")
    if found:
        first = found[0]
        out = block[:first.start()] + text
        pos = first.end()
        for m in found[1:]:
            out += block[pos:m.start()]
            pos = m.end()
        return out + block[pos:]
    if not new:
        return block
    end = block.rfind("</type")
    ls = block.rfind("\n", 0, end) + 1
    if block[ls:end].strip():          # </type> на одной строке с содержимым
        return block[:end] + nl + text + block[end:]
    return block[:ls] + text + block[ls:]


def to_text(tf: TypesFile) -> str:
    text = tf.text
    nl = "\r\n" if "\r\n" in text else "\n"
    step = "\t" if re.search(r"\n\t+<", text) else "    "
    out, pos = [], 0
    for r in tf.rows:
        if not r.dirty:
            continue
        a, b = r.span
        out.append(text[pos:a])
        out.append(_rewrite(text[a:b], r, nl, _indent_of(text, a), step))
        pos = b
    out.append(text[pos:])
    return "".join(out)


def save(tf: TypesFile, path=None) -> Path:
    path = Path(path or tf.path)
    data = to_text(tf)
    raw_bom = tf.path is not None and tf.path.is_file() and tf.path.read_bytes().startswith(b"\xef\xbb\xbf")
    path.write_bytes((b"\xef\xbb\xbf" if raw_bom else b"") + data.encode("utf-8"))
    new = parse(data, path)
    tf.text, tf.rows, tf.path = new.text, new.rows, path
    return path
