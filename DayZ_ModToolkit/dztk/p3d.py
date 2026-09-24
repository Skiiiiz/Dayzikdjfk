# -*- coding: utf-8 -*-
"""Модели .p3d: список используемых текстур и материалов.

MLOD (небинаризованные модели из Object Builder) разбираются полностью:
  "MLOD" u32 версия u32 число_LOD, затем LOD: "P3DM"/"SP3X" u32 major u32 minor u32 точек u32 нормалей
  u32 граней u32 флаги | точки (x,y,z,флаги) | нормали (x,y,z) | грани: u32 число_вершин, 4 вершины
  (u32 точка, u32 нормаль, f32 u, f32 v), u32 флаги, asciiz текстура, asciiz материал | "TAGG"-секции | f32 разрешение.
ODOL (бинаризованные) — извлекаются строки путей к .paa/.rvmat (полный разбор ODOL не нужен для проверки ссылок).
"""

from __future__ import annotations

from .i18n import tr

import re
import struct
from dataclasses import dataclass, field
from pathlib import Path
from typing import List, Set

DEP_EXT = (".paa", ".rvmat", ".tga", ".png", ".edds", ".jpg")
_ODOL_STR = re.compile(rb"[A-Za-z0-9_\-\\/.\s]{4,260}?\.(?:paa|rvmat|tga|png|edds|jpg)(?=\x00)", re.I)


class P3DError(ValueError):
    pass


@dataclass
class P3DInfo:
    kind: str                      # MLOD | ODOL
    version: int = 0
    lods: List[float] = field(default_factory=list)
    textures: Set[str] = field(default_factory=set)
    materials: Set[str] = field(default_factory=set)

    @property
    def dependencies(self) -> Set[str]:
        return self.textures | self.materials


def _cstr(d: bytes, p: int):
    e = d.find(b"\x00", p)
    if e < 0:
        raise P3DError(tr("обрезанная строка"))
    return d[p:e].decode("utf-8", "replace"), e + 1


def read_p3d(path) -> P3DInfo:
    d = Path(path).read_bytes()
    sig = d[:4]
    if sig == b"ODOL":
        info = P3DInfo("ODOL", struct.unpack_from("<I", d, 4)[0] if len(d) >= 8 else 0)
        for m in _ODOL_STR.finditer(d):
            s = m.group(0).decode("ascii", "replace").strip()
            if s.startswith("#"):
                continue
            (info.materials if s.lower().endswith(".rvmat") else info.textures).add(s)
        return info
    if sig != b"MLOD":
        raise P3DError(tr("неизвестный тип модели {0!r} (ожидалось MLOD или ODOL)").format(sig))
    version, nlods = struct.unpack_from("<II", d, 4)
    info = P3DInfo("MLOD", version)
    p = 12
    for _i in range(nlods):
        tag = d[p:p + 4]
        sp3x = tag == b"SP3X"
        if tag not in (b"P3DM", b"SP3X"):
            raise P3DError(tr("LOD {0}: неизвестная сигнатура {1!r}").format(_i, tag))
        _maj, _min, npts, nnorm, nfaces, _flags = struct.unpack_from("<6I", d, p + 4)
        p += 28 + npts * 16 + nnorm * 12
        for _f in range(nfaces):
            if sp3x:
                tex = d[p:p + 32].split(b"\x00", 1)[0].decode("utf-8", "replace")
                p += 32
                nv = struct.unpack_from("<I", d, p)[0]
                p += 4 + 4 * 16 + 4
                mat = ""
            else:
                nv = struct.unpack_from("<I", d, p)[0]
                p += 4 + 4 * 16 + 4
                tex, p = _cstr(d, p)
                mat, p = _cstr(d, p)
            if nv not in (3, 4):
                raise P3DError(tr("LOD {0}: грань с {1} вершинами — файл повреждён").format(_i, nv))
            if tex and not tex.startswith("#"):
                info.textures.add(tex)
            if mat and not mat.startswith("#"):
                info.materials.add(mat)
        if d[p:p + 4] != b"TAGG":
            raise P3DError(tr("LOD {0}: нет секции TAGG — файл повреждён").format(_i))
        p += 4
        while True:
            if sp3x:
                name = d[p:p + 64].split(b"\x00", 1)[0].decode("utf-8", "replace")
                p += 64
            else:
                p += 1
                name, p = _cstr(d, p)
            size = struct.unpack_from("<I", d, p)[0]
            p += 4 + size
            if name == "#EndOfFile#":
                break
        info.lods.append(struct.unpack_from("<f", d, p)[0])
        p += 4
    return info
