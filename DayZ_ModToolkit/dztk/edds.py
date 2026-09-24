# -*- coding: utf-8 -*-
"""Текстуры .edds (Enfusion DDS): чтение, запись и сборка атласа иконок с .imageset.

Формат .edds: обычный заголовок DDS (128 байт + 20 байт DX10 при FourCC "DX10"),
затем таблица блоков mip-уровней от меньшего к большему: 4 байта тег ("COPY" или "LZ4 ") + u32 размер,
затем данные блоков в том же порядке.
Блок LZ4: u32 размер распакованных данных, затем части: u24 сжатый размер + u8 флаги (0x80 — последняя часть)
и сжатый LZ4-блок; предыдущие распакованные данные служат словарём.

Пиксели декодирует Pillow (DXT1/3/5, BC4/5/7, RGBA): .edds пересобирается в обычный DDS.
Запись: DXT1/DXT5 (как PAA) с полной цепочкой mip-уровней, крупные уровни сжимаются LZ4.
"""

from __future__ import annotations

from .i18n import tr

import io
import struct
from dataclasses import dataclass, field
from pathlib import Path
from typing import List, Optional, Tuple

import numpy as np

DDS_MAGIC = b"DDS "
DDSD_FLAGS = 0x1 | 0x2 | 0x4 | 0x1000 | 0x20000 | 0x80000   # caps, height, width, pixelformat, mipmapcount, linearsize
DDPF_FOURCC = 0x4
DDSCAPS_TEXTURE, DDSCAPS_COMPLEX, DDSCAPS_MIPMAP = 0x1000, 0x8, 0x400000
LZ4_CHUNK = 0x10000


class EDDSError(ValueError):
    pass


# --------------------------------------------------------------------------------------
# LZ4 (блочный формат)
# --------------------------------------------------------------------------------------

def lz4_decompress_block(src: bytes, out: bytearray, limit: int) -> None:
    """Распаковывает один LZ4-блок, дописывая в out (out может содержать словарь — прошлые данные)."""
    i, n = 0, len(src)
    while i < n:
        token = src[i]
        i += 1
        lit = token >> 4
        if lit == 15:
            while True:
                b = src[i]
                i += 1
                lit += b
                if b != 255:
                    break
        out += src[i:i + lit]
        i += lit
        if i >= n:
            break
        off = src[i] | (src[i + 1] << 8)
        i += 2
        if off == 0 or off > len(out):
            raise EDDSError(tr("LZ4: неверное смещение"))
        ml = token & 15
        if ml == 15:
            while True:
                b = src[i]
                i += 1
                ml += b
                if b != 255:
                    break
        ml += 4
        start = len(out) - off
        if off >= ml:
            out += out[start:start + ml]
        else:
            for k in range(ml):
                out.append(out[start + k])
        if len(out) > limit:
            raise EDDSError(tr("LZ4: данных больше, чем заявлено"))


def lz4_compress_block(data: bytes) -> bytes:
    """Простое жадное сжатие в формат LZ4-блока (совместимо с любым LZ4-декодером)."""
    n = len(data)
    out = bytearray()
    if n < 13:
        _lz4_last_literals(out, data, 0, n)
        return bytes(out)
    table = {}
    anchor, i = 0, 0
    limit = n - 12          # последние 5 байт — литералы, совпадения не ближе 12 байт к концу
    mv = data
    while i < limit:
        seq = mv[i:i + 4]
        ref = table.get(seq)
        table[seq] = i
        if ref is None or i - ref > 0xFFFF:
            i += 1
            continue
        # длина совпадения
        ml = 4
        maxl = n - 5 - i
        while ml < maxl and mv[ref + ml] == mv[i + ml]:
            ml += 1
        lit = i - anchor
        token_pos = len(out)
        out.append(0)
        tok = (min(lit, 15) << 4)
        if lit >= 15:
            r = lit - 15
            while r >= 255:
                out.append(255)
                r -= 255
            out.append(r)
        out += mv[anchor:i]
        off = i - ref
        out += bytes((off & 255, off >> 8))
        m = ml - 4
        tok |= min(m, 15)
        if m >= 15:
            r = m - 15
            while r >= 255:
                out.append(255)
                r -= 255
            out.append(r)
        out[token_pos] = tok
        i += ml
        anchor = i
    _lz4_last_literals(out, data, anchor, n)
    return bytes(out)


def _lz4_last_literals(out: bytearray, data: bytes, anchor: int, n: int) -> None:
    lit = n - anchor
    out.append(min(lit, 15) << 4)
    if lit >= 15:
        r = lit - 15
        while r >= 255:
            out.append(255)
            r -= 255
        out.append(r)
    out += data[anchor:n]


# --------------------------------------------------------------------------------------
# Чтение
# --------------------------------------------------------------------------------------

@dataclass
class EDDSInfo:
    width: int
    height: int
    mipmaps: int
    fourcc: str
    dxgi: int = 0
    blocks: List[Tuple[str, int]] = field(default_factory=list)   # от меньшего mip к большему
    header: bytes = b""

    @property
    def format_name(self) -> str:
        names = {71: "BC1", 72: "BC1 sRGB", 74: "BC2", 75: "BC2 sRGB", 77: "BC3", 78: "BC3 sRGB", 80: "BC4",
                 83: "BC5", 95: "BC6H", 98: "BC7", 99: "BC7 sRGB", 28: "RGBA8", 29: "RGBA8 sRGB",
                 87: "BGRA8", 88: "BGRX8", 91: "BGRA8 sRGB", 93: "BGRX8 sRGB"}
        return self.fourcc if self.fourcc != "DX10" else names.get(self.dxgi, f"DXGI {self.dxgi}")


def parse(data: bytes) -> Tuple[EDDSInfo, List[bytes]]:
    """Возвращает (информация, данные mip-уровней от большего к меньшему)."""
    if data[:4] != DDS_MAGIC or len(data) < 128:
        raise EDDSError(tr("это не .edds/.dds (нет сигнатуры DDS)"))
    h, w, _pitch, _depth, mips = struct.unpack_from("<5I", data, 12)
    fourcc = data[84:88].decode("ascii", "replace")
    p = 128
    dxgi = 0
    if fourcc == "DX10":
        dxgi = struct.unpack_from("<I", data, 128)[0]
        p = 148
    header = data[:p]
    info = EDDSInfo(w, h, max(1, mips), fourcc, dxgi, header=header)
    while data[p:p + 4] in (b"COPY", b"LZ4 "):
        info.blocks.append((data[p:p + 4].decode().strip(), struct.unpack_from("<I", data, p + 4)[0]))
        p += 8
    if not info.blocks:
        raise EDDSError(tr("в .edds нет таблицы блоков COPY/LZ4 (возможно, это обычный .dds)"))
    raw: List[bytes] = []
    for kind, size in info.blocks:
        blk = data[p:p + size]
        if len(blk) != size:
            raise EDDSError(tr("блок {0} выходит за конец файла").format(kind))
        p += size
        if kind == "COPY":
            raw.append(blk)
            continue
        total = struct.unpack_from("<I", blk, 0)[0]
        out = bytearray()
        q = 4
        while q < len(blk):
            csz = blk[q] | (blk[q + 1] << 8) | (blk[q + 2] << 16)
            flags = blk[q + 3]
            q += 4
            lz4_decompress_block(blk[q:q + csz], out, total)
            q += csz
            if flags & 0x80:
                break
        if len(out) != total:
            raise EDDSError(tr("LZ4: ожидалось {0} байт, получено {1}").format(total, len(out)))
        raw.append(bytes(out))
    if p != len(data):
        raise EDDSError(tr("после данных осталось {0} лишних байт").format(len(data) - p))
    raw.reverse()
    return info, raw


def to_dds(data: bytes) -> bytes:
    info, mips = parse(data)
    return info.header + b"".join(mips)


def read_edds(path) -> Tuple[np.ndarray, EDDSInfo]:
    from PIL import Image
    data = Path(path).read_bytes()
    info, mips = parse(data)
    w, h = info.width, info.height
    raw_formats = {87: (2, 1, 0, 3), 91: (2, 1, 0, 3), 88: (2, 1, 0, -1), 93: (2, 1, 0, -1),
                   28: (0, 1, 2, 3), 29: (0, 1, 2, 3)}
    if info.fourcc == "DX10" and info.dxgi in raw_formats:
        order = raw_formats[info.dxgi]
        px = np.frombuffer(mips[0], np.uint8)[:w * h * 4].reshape(h, w, 4)
        rgba = px[..., [order[0], order[1], order[2], order[3] if order[3] >= 0 else 3]].copy()
        if order[3] < 0:
            rgba[..., 3] = 255
        return rgba, info
    hdr = bytearray(info.header)
    struct.pack_into("<I", hdr, 28, 1)          # Pillow читает только первый уровень
    try:
        im = Image.open(io.BytesIO(bytes(hdr) + mips[0]))
        im.load()
    except Exception as e:
        raise EDDSError(tr("формат {0} не удалось декодировать: {1}").format(info.format_name, e))
    return np.asarray(im.convert("RGBA")).copy(), info


def edds_to_image(src, dst) -> EDDSInfo:
    from PIL import Image
    rgba, info = read_edds(src)
    Path(dst).parent.mkdir(parents=True, exist_ok=True)
    Image.fromarray(rgba, "RGBA").save(dst)
    return info


# --------------------------------------------------------------------------------------
# Запись
# --------------------------------------------------------------------------------------

def _dds_header(w: int, h: int, mips: int, fourcc: bytes, linear: int) -> bytes:
    hdr = bytearray(128)
    hdr[0:4] = DDS_MAGIC
    struct.pack_into("<7I", hdr, 4, 124, DDSD_FLAGS, h, w, linear, 0, mips)
    struct.pack_into("<2I", hdr, 76, 32, DDPF_FOURCC)
    hdr[84:88] = fourcc
    struct.pack_into("<I", hdr, 108, DDSCAPS_TEXTURE | DDSCAPS_COMPLEX | DDSCAPS_MIPMAP)
    return bytes(hdr)


def _lz4_block(raw: bytes) -> bytes:
    out = bytearray(struct.pack("<I", len(raw)))
    chunks = [raw[i:i + LZ4_CHUNK] for i in range(0, len(raw), LZ4_CHUNK)] or [b""]
    for k, ch in enumerate(chunks):
        c = lz4_compress_block(ch)
        flag = 0x80 if k == len(chunks) - 1 else 0
        out += len(c).to_bytes(3, "little") + bytes((flag,)) + c
    return bytes(out)


def encode_edds(img: np.ndarray, fmt: str = "auto", compress: bool = True) -> Tuple[bytes, str]:
    from . import paa
    h, w = img.shape[:2]
    has_alpha = bool((img[..., 3] < 255).any())
    if fmt == "auto":
        fmt = "dxt5" if has_alpha else "dxt1"
    dxt5 = fmt == "dxt5"
    mips = paa.build_mipmaps(img, full_chain=True)
    raw = [paa.encode_dxt(m, dxt5) for m in mips]
    blocks: List[Tuple[bytes, bytes]] = []
    for r in reversed(raw):                     # от меньшего к большему
        if compress and len(r) >= 1024:
            z = _lz4_block(r)
            if len(z) < len(r):
                blocks.append((b"LZ4 ", z))
                continue
        blocks.append((b"COPY", r))
    out = bytearray(_dds_header(w, h, len(mips), b"DXT5" if dxt5 else b"DXT1", len(raw[0])))
    for tag, b in blocks:
        out += tag + struct.pack("<I", len(b))
    for _tag, b in blocks:
        out += b
    return bytes(out), fmt.upper()


def image_to_edds(src, dst, fmt: str = "auto", resize: str = "error") -> Tuple[int, int, str]:
    from PIL import Image
    from . import paa
    img = paa.load_image(src)
    h, w = img.shape[:2]
    if not (paa.is_pow2(w) and paa.is_pow2(h)):
        if resize == "nearest":
            img = np.asarray(Image.fromarray(img, "RGBA").resize((paa.nearest_pow2(w), paa.nearest_pow2(h)),
                                                                 Image.LANCZOS))
        else:
            raise EDDSError(tr("размер {0}x{1} не степень двойки (включите масштабирование)").format(w, h))
    data, used = encode_edds(img, fmt)
    Path(dst).parent.mkdir(parents=True, exist_ok=True)
    Path(dst).write_bytes(data)
    return img.shape[1], img.shape[0], used


# --------------------------------------------------------------------------------------
# Атлас иконок + .imageset
# --------------------------------------------------------------------------------------

@dataclass
class Placed:
    name: str
    x: int
    y: int
    w: int
    h: int


def pack(sizes: List[Tuple[str, int, int]], padding: int = 2, max_side: int = 4096) -> Tuple[int, int, List[Placed]]:
    """Полочная упаковка (от высоких к низким) в атлас со сторонами-степенями двойки."""
    items = sorted(sizes, key=lambda s: (-s[2], -s[1], s[0]))
    area = sum((w + padding) * (h + padding) for _, w, h in items)
    side = 64
    while side * side < area:
        side *= 2
    widest = max((w for _, w, _h in items), default=1) + padding
    while side < widest:
        side *= 2
    while side <= max_side:
        for width, height in ((side, side // 2), (side, side)):
            placed, x, y, row_h = [], 0, 0, 0
            ok = True
            for name, w, h in items:
                if x + w + padding > width:
                    x, y = 0, y + row_h
                    row_h = 0
                if y + h + padding > height:
                    ok = False
                    break
                placed.append(Placed(name, x, y, w, h))
                x += w + padding
                row_h = max(row_h, h + padding)
            if ok:
                return width, height, placed
        side *= 2
    raise EDDSError(tr("картинки не помещаются в атлас {0}x{0}").format(max_side))


def imageset_text(name: str, texture_path: str, width: int, height: int, placed: List[Placed]) -> str:
    lines = ["ImageSetClass {", f' Name "{name}"', f" RefSize {width} {height}", " Textures {",
             "  ImageSetTextureClass {", "   mpix 1", f'   path "{texture_path}"', "  }", " }", " Images {"]
    for p in placed:
        lines += [f"  ImageSetDefClass {p.name} {{", f'   Name "{p.name}"', f"   Pos {p.x} {p.y}",
                  f"   Size {p.w} {p.h}", "   Flags 0", "  }"]
    lines += [" }", "}"]
    return "\n".join(lines) + "\n"


def build_atlas(images: List[Path], out_dir, name: str, texture_prefix: str, fmt: str = "edds",
                padding: int = 2) -> Tuple[Path, Path, List[Placed]]:
    """Собирает картинки в атлас (.edds или .paa) и пишет .imageset. texture_prefix — путь к папке в PBO."""
    from PIL import Image
    from . import common, paa
    ims = []
    for f in images:
        with Image.open(f) as im:
            ims.append((common.sanitize_name(Path(f).stem), im.convert("RGBA").copy()))
    names = [n for n, _ in ims]
    if len(set(names)) != len(names):
        raise EDDSError(tr("одинаковые имена картинок после очистки имён: {0}").format(
            ", ".join(sorted({n for n in names if names.count(n) > 1}))))
    width, height, placed = pack([(n, im.width, im.height) for n, im in ims], padding)
    atlas = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    by_name = dict(ims)
    for p in placed:
        atlas.paste(by_name[p.name], (p.x, p.y))
    arr = np.asarray(atlas)
    out_dir = Path(out_dir)
    out_dir.mkdir(parents=True, exist_ok=True)
    ext = ".edds" if fmt == "edds" else ".paa"
    tex = out_dir / f"{name}{ext}"
    if fmt == "edds":
        tex.write_bytes(encode_edds(arr, "dxt5")[0])
    else:
        tex.write_bytes(paa.encode_paa(arr, "dxt5")[0])
    tpath = (texture_prefix.strip("\\/").replace("\\", "/") + "/" if texture_prefix.strip() else "") + tex.name
    iset = out_dir / f"{name}.imageset"
    iset.write_text(imageset_text(name, tpath, width, height, placed), encoding="utf-8")
    return tex, iset, placed
