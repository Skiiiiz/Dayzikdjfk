# -*- coding: utf-8 -*-
"""Бинарные конфиги (rapified, «raP»): config.bin <-> config.cpp.

Формат:
  "\\0raP" u32 0 u32 8 u32 смещение_enum
  тело класса: asciiz имя_базового | varint число_записей | записи | u32 конец_поддерева,
               затем тела вложенных классов (в глубину)
  записи: 0 класс (asciiz имя, u32 смещение тела) | 1 значение (u8 подтип, asciiz имя, данные)
          2 массив (asciiz имя, массив) | 3 внешний класс (asciiz) | 4 delete (asciiz)
          5 массив += (u32 флаги, asciiz имя, массив)
  массив: varint число | элементы (u8 тип: 0 строка, 1 float, 2 int32, 3 вложенный массив, 4 переменная)
  в конце: u32 число_enum | (asciiz имя, u32 значение)*
"""

from __future__ import annotations

import struct
from pathlib import Path
from typing import List, Optional, Tuple

from .cfg import ArrayNode, ClassNode, DeleteNode, Node, ValueNode

MAGIC = b"\x00raP"


class RapError(ValueError):
    pass


def is_rapified(data: bytes) -> bool:
    return data[:4] == MAGIC


# --------------------------------------------------------------------------------------
# Чтение
# --------------------------------------------------------------------------------------

class _Reader:
    def __init__(self, data: bytes):
        self.d = data

    def u8(self, p: int) -> Tuple[int, int]:
        if p >= len(self.d):
            raise RapError("неожиданный конец файла")
        return self.d[p], p + 1

    def u32(self, p: int) -> Tuple[int, int]:
        if p + 4 > len(self.d):
            raise RapError("неожиданный конец файла")
        return struct.unpack_from("<I", self.d, p)[0], p + 4

    def i32(self, p: int) -> Tuple[int, int]:
        return struct.unpack_from("<i", self.d, p)[0], p + 4

    def f32(self, p: int) -> Tuple[float, int]:
        return struct.unpack_from("<f", self.d, p)[0], p + 4

    def asciiz(self, p: int) -> Tuple[str, int]:
        e = self.d.find(b"\x00", p)
        if e < 0:
            raise RapError("незавершённая строка")
        raw = self.d[p:e]
        try:
            s = raw.decode("utf-8")
        except UnicodeDecodeError:
            s = raw.decode("cp1251", "replace")
        return s, e + 1

    def varint(self, p: int) -> Tuple[int, int]:
        v, shift = 0, 0
        while True:
            b, p = self.u8(p)
            v |= (b & 0x7F) << shift
            if not b & 0x80:
                return v, p
            shift += 7
            if shift > 35:
                raise RapError("неверное сжатое число")

    def array(self, p: int, depth: int = 0) -> Tuple[list, int]:
        if depth > 64:
            raise RapError("слишком глубокая вложенность массивов")
        n, p = self.varint(p)
        out: list = []
        for _ in range(n):
            t, p = self.u8(p)
            if t == 0 or t == 4:
                v, p = self.asciiz(p)
            elif t == 1:
                v, p = self.f32(p)
            elif t == 2:
                v, p = self.i32(p)
            elif t == 3:
                v, p = self.array(p, depth + 1)
            elif t == 6:
                v = struct.unpack_from("<q", self.d, p)[0]
                p += 8
            else:
                raise RapError(f"неизвестный тип элемента массива {t} (смещение {p - 1})")
            out.append(v)
        return out, p

    def body(self, p: int, node: ClassNode, depth: int = 0) -> None:
        if depth > 128:
            raise RapError("слишком глубокая вложенность классов")
        base, p = self.asciiz(p)
        node.base = base or None
        n, p = self.varint(p)
        children: List[Tuple[ClassNode, int]] = []
        for _ in range(n):
            t, p = self.u8(p)
            if t == 0:
                name, p = self.asciiz(p)
                off, p = self.u32(p)
                c = ClassNode(name)
                node.entries.append(c)
                children.append((c, off))
            elif t == 1:
                sub, p = self.u8(p)
                name, p = self.asciiz(p)
                if sub == 0 or sub == 4:
                    v, p = self.asciiz(p)
                elif sub == 1:
                    v, p = self.f32(p)
                elif sub == 2:
                    v, p = self.i32(p)
                elif sub == 6:
                    v = struct.unpack_from("<q", self.d, p)[0]
                    p += 8
                else:
                    raise RapError(f"неизвестный подтип значения {sub} у '{name}'")
                node.entries.append(ValueNode(name, value=v))
            elif t == 2:
                name, p = self.asciiz(p)
                arr, p = self.array(p)
                node.entries.append(ArrayNode(name, value=arr))
            elif t == 3:
                name, p = self.asciiz(p)
                node.entries.append(ClassNode(name, extern=True))
            elif t == 4:
                name, p = self.asciiz(p)
                node.entries.append(DeleteNode(name))
            elif t == 5:
                _, p = self.u32(p)
                name, p = self.asciiz(p)
                arr, p = self.array(p)
                node.entries.append(ArrayNode(name, value=arr, append=True))
            else:
                raise RapError(f"неизвестный тип записи {t} (смещение {p - 1})")
        for c, off in children:
            if off >= len(self.d):
                raise RapError(f"класс {c.name}: смещение {off} за пределами файла")
            self.body(off, c, depth + 1)


def read_rap(data: bytes) -> ClassNode:
    if not is_rapified(data):
        raise RapError("это не бинаризованный конфиг (нет сигнатуры raP)")
    r = _Reader(data)
    root = ClassNode("")
    r.body(16, root)
    root.base = None
    return root


def derapify_file(src, dst=None) -> Path:
    from .cfg import to_text
    src = Path(src)
    root = read_rap(src.read_bytes())
    dst = Path(dst) if dst else src.with_suffix(".cpp")
    dst.parent.mkdir(parents=True, exist_ok=True)
    dst.write_text(to_text(root).replace("\n", "\r\n"), encoding="utf-8", newline="")
    return dst


# --------------------------------------------------------------------------------------
# Запись
# --------------------------------------------------------------------------------------

def _varint(v: int) -> bytes:
    out = bytearray()
    while True:
        b = v & 0x7F
        v >>= 7
        if v:
            out.append(b | 0x80)
        else:
            out.append(b)
            return bytes(out)


def _z(s: str) -> bytes:
    b = s.encode("utf-8")
    if b"\x00" in b:
        raise RapError("строка содержит нулевой символ")
    return b + b"\x00"


def _scalar_elem(v) -> bytes:
    if isinstance(v, list):
        return b"\x03" + _array(v)
    if isinstance(v, bool):
        v = int(v)
    if isinstance(v, int):
        if -2 ** 31 <= v < 2 ** 31:
            return b"\x02" + struct.pack("<i", v)
        return b"\x01" + struct.pack("<f", float(v))
    if isinstance(v, float):
        return b"\x01" + struct.pack("<f", v)
    return b"\x00" + _z(str(v))


def _array(arr: list) -> bytes:
    return _varint(len(arr)) + b"".join(_scalar_elem(x) for x in arr)


def write_rap(root: ClassNode) -> bytes:
    out = bytearray(MAGIC + struct.pack("<III", 0, 8, 0))

    def write_body(node: ClassNode) -> None:
        out.extend(_z(node.base or ""))
        entries = [e for e in node.entries if not (isinstance(e, ValueNode) and e.name == "__enum__")]
        out.extend(_varint(len(entries)))
        patches: List[Tuple[int, ClassNode]] = []
        for e in entries:
            if isinstance(e, ClassNode):
                if e.extern:
                    out.extend(b"\x03" + _z(e.name))
                else:
                    out.extend(b"\x00" + _z(e.name))
                    patches.append((len(out), e))
                    out.extend(b"\x00\x00\x00\x00")
            elif isinstance(e, DeleteNode):
                out.extend(b"\x04" + _z(e.name))
            elif isinstance(e, ArrayNode):
                if e.append:
                    out.extend(b"\x05" + struct.pack("<I", 1) + _z(e.name) + _array(e.value))
                else:
                    out.extend(b"\x02" + _z(e.name) + _array(e.value))
            elif isinstance(e, ValueNode):
                v = e.value
                if isinstance(v, bool):
                    v = int(v)
                if isinstance(v, int) and -2 ** 31 <= v < 2 ** 31:
                    out.extend(b"\x01\x02" + _z(e.name) + struct.pack("<i", v))
                elif isinstance(v, (int, float)):
                    out.extend(b"\x01\x01" + _z(e.name) + struct.pack("<f", float(v)))
                else:
                    out.extend(b"\x01\x00" + _z(e.name) + _z(str(v)))
        end_pos = len(out)
        out.extend(b"\x00\x00\x00\x00")   # смещение конца поддерева этого класса
        for pos, c in patches:
            struct.pack_into("<I", out, pos, len(out))
            write_body(c)
        struct.pack_into("<I", out, end_pos, len(out))

    write_body(root)
    struct.pack_into("<I", out, 12, len(out))
    out.extend(struct.pack("<I", 0))  # enum: 0 записей
    return bytes(out)


def rapify_file(src, dst=None, include_dirs=None) -> Tuple[Path, list]:
    """config.cpp -> config.bin. Возвращает (путь, предупреждения). Бросает ConfigError."""
    from .cfg import parse_config
    src = Path(src)
    root, issues = parse_config(src, include_dirs=include_dirs)
    dst = Path(dst) if dst else src.with_suffix(".bin")
    dst.parent.mkdir(parents=True, exist_ok=True)
    dst.write_bytes(write_rap(root))
    return dst, issues
