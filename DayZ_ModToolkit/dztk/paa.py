# -*- coding: utf-8 -*-
"""Чтение и запись текстур PAA (формат Bohemia Interactive, используется в DayZ).

Поддерживается:
  * чтение DXT1/DXT5 (в т.ч. mip-уровни, сжатые LZO), ARGB8888, ARGB4444, ARGB1555, AI88;
  * запись DXT1 (с 1-битной прозрачностью) и DXT5 с полной цепочкой mip-уровней;
    крупные mip-уровни сжимаются LZO, как это делает ImageToPAA.

Структура файла:
  u16 тип | теги "GGAT"+имя(4)+u32 длина+данные | u16 размер палитры (+палитра) |
  mip-уровни: u16 ширина (бит 0x8000 = LZO), u16 высота, u24 размер, данные | u16 0,u16 0,u16 0
"""

from __future__ import annotations

from dztk.i18n import tr

import struct
from dataclasses import dataclass, field
from pathlib import Path
from typing import Dict, List, Optional, Tuple

import numpy as np

from . import lzo

TYPE_DXT1 = 0xFF01
TYPE_DXT2 = 0xFF02
TYPE_DXT3 = 0xFF03
TYPE_DXT4 = 0xFF04
TYPE_DXT5 = 0xFF05
TYPE_ARGB4444 = 0x4444
TYPE_ARGB1555 = 0x1555
TYPE_AI88 = 0x8080
TYPE_ARGB8888 = 0x8888

TYPE_NAMES = {
    TYPE_DXT1: "DXT1", TYPE_DXT2: "DXT2", TYPE_DXT3: "DXT3", TYPE_DXT4: "DXT4", TYPE_DXT5: "DXT5",
    TYPE_ARGB4444: "ARGB4444", TYPE_ARGB1555: "ARGB1555", TYPE_AI88: "AI88", TYPE_ARGB8888: "ARGB8888",
}

IMAGE_EXTENSIONS = {".png", ".jpg", ".jpeg", ".tga", ".bmp", ".tif", ".tiff", ".gif", ".webp"}


class PAAError(ValueError):
    pass


@dataclass
class Mipmap:
    width: int
    height: int
    offset: int
    size: int
    lzo: bool
    data: bytes = b""


@dataclass
class PAAInfo:
    type: int
    tags: Dict[str, bytes] = field(default_factory=dict)
    mipmaps: List[Mipmap] = field(default_factory=list)

    @property
    def type_name(self) -> str:
        return TYPE_NAMES.get(self.type, hex(self.type))

    @property
    def width(self) -> int:
        return self.mipmaps[0].width if self.mipmaps else 0

    @property
    def height(self) -> int:
        return self.mipmaps[0].height if self.mipmaps else 0


# --------------------------------------------------------------------------------------
# Разбор файла
# --------------------------------------------------------------------------------------

def _data_size(ptype: int, w: int, h: int) -> int:
    if ptype == TYPE_DXT1:
        return max(1, (w + 3) // 4) * max(1, (h + 3) // 4) * 8
    if ptype in (TYPE_DXT2, TYPE_DXT3, TYPE_DXT4, TYPE_DXT5):
        return max(1, (w + 3) // 4) * max(1, (h + 3) // 4) * 16
    if ptype == TYPE_ARGB8888:
        return w * h * 4
    return w * h * 2


def parse(data: bytes, load_all: bool = False) -> PAAInfo:
    """Разбирает PAA. Распаковывает данные первого mip (или всех при load_all)."""
    if len(data) < 4:
        raise PAAError(tr("файл слишком короткий"))
    ptype = struct.unpack_from("<H", data, 0)[0]
    if ptype not in TYPE_NAMES:
        raise PAAError(tr("неизвестный тип PAA 0x{0:04X}").format(ptype))
    info = PAAInfo(type=ptype)
    p = 2
    while data[p:p + 4] == b"GGAT":
        if p + 12 > len(data):
            raise PAAError(tr("обрезанный тег"))
        name = data[p + 4:p + 8][::-1].decode("ascii", "replace")  # 'CGVA' -> 'AVGC'
        ln = struct.unpack_from("<I", data, p + 8)[0]
        info.tags[name] = data[p + 12:p + 12 + ln]
        p += 12 + ln
    if p + 2 > len(data):
        raise PAAError(tr("нет палитры"))
    pal = struct.unpack_from("<H", data, p)[0]
    p += 2 + pal * 3

    first = True
    while p + 4 <= len(data):
        w, h = struct.unpack_from("<HH", data, p)
        if w == 0 or h == 0:
            break
        if p + 7 > len(data):
            raise PAAError(tr("обрезанный заголовок mip-уровня"))
        size = data[p + 4] | (data[p + 5] << 8) | (data[p + 6] << 16)
        is_lzo = bool(w & 0x8000)
        real_w = w & 0x7FFF
        mip = Mipmap(real_w, h, p + 7, size, is_lzo)
        if p + 7 + size > len(data):
            raise PAAError(tr("mip {0}x{1}: данные выходят за конец файла").format(real_w, h))
        if first or load_all:
            raw = data[p + 7:p + 7 + size]
            expected = _data_size(ptype, real_w, h)
            if is_lzo:
                raw, _ = lzo.decompress(raw, expected)
            elif len(raw) != expected:
                raise PAAError(tr("mip {0}x{1}: размер {2}, ожидалось {3} (сжатие LZSS не поддерживается)").format(real_w, h, len(raw), expected))
            mip.data = raw
        info.mipmaps.append(mip)
        first = False
        p += 7 + size
    if not info.mipmaps:
        raise PAAError(tr("в файле нет mip-уровней"))
    return info


# --------------------------------------------------------------------------------------
# Декодирование
# --------------------------------------------------------------------------------------

def _unpack565(c: np.ndarray) -> np.ndarray:
    r = ((c >> 11) & 31).astype(np.uint32)
    g = ((c >> 5) & 63).astype(np.uint32)
    b = (c & 31).astype(np.uint32)
    return np.stack([(r * 255 + 15) // 31, (g * 255 + 31) // 63, (b * 255 + 15) // 31], axis=-1).astype(np.int32)


def _color_block_palette(c0: np.ndarray, c1: np.ndarray, four_always: bool) -> Tuple[np.ndarray, np.ndarray]:
    """Палитра 4 цвета RGBA для каждого блока. Возвращает (N,4,4)."""
    p0, p1 = _unpack565(c0), _unpack565(c1)
    four = (c0 > c1) | four_always
    p2 = np.where(four[:, None], (2 * p0 + p1) // 3, (p0 + p1) // 2)
    p3 = np.where(four[:, None], (p0 + 2 * p1) // 3, 0)
    pal = np.stack([p0, p1, p2, p3], axis=1)
    alpha = np.full(pal.shape[:2], 255, np.int32)
    alpha[:, 3] = np.where(four, 255, 0)
    return np.concatenate([pal, alpha[..., None]], axis=-1), four


def _decode_dxt(raw: bytes, w: int, h: int, dxt5: bool) -> np.ndarray:
    bw, bh = max(1, (w + 3) // 4), max(1, (h + 3) // 4)
    nb = bw * bh
    bsize = 16 if dxt5 else 8
    arr = np.frombuffer(raw, np.uint8)[:nb * bsize].reshape(nb, bsize)
    col = arr[:, 8:] if dxt5 else arr
    c0 = col[:, 0].astype(np.uint16) | (col[:, 1].astype(np.uint16) << 8)
    c1 = col[:, 2].astype(np.uint16) | (col[:, 3].astype(np.uint16) << 8)
    bits = (col[:, 4].astype(np.uint32) | (col[:, 5].astype(np.uint32) << 8) |
            (col[:, 6].astype(np.uint32) << 16) | (col[:, 7].astype(np.uint32) << 24))
    idx = (bits[:, None] >> (2 * np.arange(16, dtype=np.uint32))) & 3
    pal, _ = _color_block_palette(c0, c1, dxt5)
    px = np.take_along_axis(pal, idx[..., None].astype(np.int64), axis=1)  # (N,16,4)

    if dxt5:
        a0 = arr[:, 0].astype(np.int32)
        a1 = arr[:, 1].astype(np.int32)
        abits = np.zeros(nb, np.uint64)
        for i in range(6):
            abits |= arr[:, 2 + i].astype(np.uint64) << np.uint64(8 * i)
        aidx = ((abits[:, None] >> (np.uint64(3) * np.arange(16, dtype=np.uint64))) & np.uint64(7)).astype(np.int64)
        k = np.arange(8)
        eight = (a0 > a1)[:, None]
        pal8 = ((7 - k[None, 1:7]) * a0[:, None] + k[None, 1:7] * a1[:, None]) // 7
        pal6 = ((5 - k[None, 1:5]) * a0[:, None] + k[None, 1:5] * a1[:, None]) // 5
        apal = np.zeros((nb, 8), np.int32)
        apal[:, 0], apal[:, 1] = a0, a1
        apal[:, 2:8] = np.where(eight, pal8, np.concatenate([pal6, np.zeros((nb, 1), np.int32),
                                                              np.full((nb, 1), 255, np.int32)], axis=1))
        px[..., 3] = np.take_along_axis(apal, aidx, axis=1)

    img = px.reshape(bh, bw, 4, 4, 4).transpose(0, 2, 1, 3, 4).reshape(bh * 4, bw * 4, 4)
    return img[:h, :w].astype(np.uint8)


def _decode_raw(ptype: int, raw: bytes, w: int, h: int) -> np.ndarray:
    if ptype == TYPE_ARGB8888:
        a = np.frombuffer(raw, np.uint8).reshape(h, w, 4)  # B,G,R,A
        return a[..., [2, 1, 0, 3]].copy()
    v = np.frombuffer(raw, "<u2").reshape(h, w).astype(np.uint32)
    if ptype == TYPE_ARGB4444:
        ch = [(v >> 8) & 15, (v >> 4) & 15, v & 15, (v >> 12) & 15]
        return np.stack([c * 17 for c in ch], -1).astype(np.uint8)
    if ptype == TYPE_ARGB1555:
        r, g, b = (v >> 10) & 31, (v >> 5) & 31, v & 31
        a = np.where(v >> 15, 255, 0)
        return np.stack([(r * 255) // 31, (g * 255) // 31, (b * 255) // 31, a], -1).astype(np.uint8)
    if ptype == TYPE_AI88:
        i, a = v & 255, v >> 8
        return np.stack([i, i, i, a], -1).astype(np.uint8)
    raise PAAError(tr("декодирование {0} не поддерживается").format(TYPE_NAMES.get(ptype)))


def decode_mip(info: PAAInfo, mip: Mipmap) -> np.ndarray:
    if info.type == TYPE_DXT1:
        return _decode_dxt(mip.data, mip.width, mip.height, False)
    if info.type in (TYPE_DXT3, TYPE_DXT2):
        raise PAAError(tr("DXT2/DXT3 не поддерживаются"))
    if info.type in (TYPE_DXT4, TYPE_DXT5):
        return _decode_dxt(mip.data, mip.width, mip.height, True)
    return _decode_raw(info.type, mip.data, mip.width, mip.height)


def read_paa(path) -> Tuple[np.ndarray, PAAInfo]:
    """Возвращает (RGBA uint8 массив HxWx4 первого mip-уровня, информация)."""
    info = parse(Path(path).read_bytes())
    return decode_mip(info, info.mipmaps[0]), info


def paa_to_image(src, dst) -> PAAInfo:
    from PIL import Image
    rgba, info = read_paa(src)
    Path(dst).parent.mkdir(parents=True, exist_ok=True)
    Image.fromarray(rgba, "RGBA").save(dst)
    return info


# --------------------------------------------------------------------------------------
# Кодирование
# --------------------------------------------------------------------------------------

def _to_blocks(img: np.ndarray) -> Tuple[np.ndarray, int, int]:
    h, w = img.shape[:2]
    bh, bw = max(1, (h + 3) // 4), max(1, (w + 3) // 4)
    ph, pw = bh * 4, bw * 4
    if (ph, pw) != (h, w):
        img = np.pad(img, ((0, ph - h), (0, pw - w), (0, 0)), mode="edge")
    blocks = img.reshape(bh, 4, bw, 4, 4).transpose(0, 2, 1, 3, 4).reshape(bh * bw, 16, 4)
    return blocks, bw, bh


def _pack565(rgb: np.ndarray) -> np.ndarray:
    rgb = np.clip(np.rint(rgb), 0, 255).astype(np.int32)
    r = (rgb[..., 0] * 31 + 127) // 255
    g = (rgb[..., 1] * 63 + 127) // 255
    b = (rgb[..., 2] * 31 + 127) // 255
    return ((r << 11) | (g << 5) | b).astype(np.uint16)


def _encode_color(blocks: np.ndarray, allow_alpha: bool) -> np.ndarray:
    """Кодирует цветовую часть DXT-блоков. blocks: (N,16,4) uint8 -> (N,8) uint8."""
    n = blocks.shape[0]
    rgb = blocks[..., :3].astype(np.float32)
    transparent = (blocks[..., 3] < 128) if allow_alpha else np.zeros(blocks.shape[:2], bool)
    has_t = transparent.any(axis=1)
    weight = (~transparent).astype(np.float32)
    wsum = np.maximum(weight.sum(axis=1, keepdims=True), 1.0)
    mean = (rgb * weight[..., None]).sum(axis=1) / wsum
    centered = (rgb - mean[:, None, :]) * weight[..., None]
    cov = np.einsum("nki,nkj->nij", centered, centered)
    axis = np.ones((n, 3), np.float32)
    for _ in range(8):  # степенной метод: главная ось цветов блока
        axis = np.einsum("nij,nj->ni", cov, axis)
        norm = np.linalg.norm(axis, axis=1, keepdims=True)
        axis = np.where(norm > 1e-6, axis / np.maximum(norm, 1e-12), np.float32(0.57735))
    proj = np.einsum("nki,ni->nk", rgb - mean[:, None, :], axis)
    big = np.float32(1e9)
    pmax = np.where(transparent, -big, proj).max(axis=1)
    pmin = np.where(transparent, big, proj).min(axis=1)
    pmax = np.where(has_t & transparent.all(axis=1), 0, pmax)
    pmin = np.where(has_t & transparent.all(axis=1), 0, pmin)
    # небольшая «вставка» концов внутрь диапазона уменьшает ошибку
    inset = (pmax - pmin) / 16.0
    e0 = mean + axis * (pmax - inset)[:, None]
    e1 = mean + axis * (pmin + inset)[:, None]
    c0, c1 = _pack565(e0), _pack565(e1)

    # 4-цветный режим требует c0 > c1, 3-цветный (с прозрачностью) — c0 <= c1
    four = ~has_t
    swap = np.where(four, c0 < c1, c0 > c1)
    c0, c1 = np.where(swap, c1, c0), np.where(swap, c0, c1)
    equal4 = four & (c0 == c1)

    pal, _ = _color_block_palette(c0, c1, False)
    pal_rgb = pal[..., :3].astype(np.float32)
    dist = ((rgb[:, :, None, :] - pal_rgb[:, None, :, :]) ** 2).sum(-1)  # (N,16,4)
    dist[:, :, 3] = np.where(has_t[:, None], np.inf, dist[:, :, 3])
    idx = dist.argmin(axis=2).astype(np.uint32)
    idx = np.where(transparent, 3, idx)
    idx = np.where(equal4[:, None], 0, idx)

    bits = (idx << (2 * np.arange(16, dtype=np.uint32))).sum(axis=1).astype(np.uint32)
    out = np.zeros((n, 8), np.uint8)
    out[:, 0], out[:, 1] = c0 & 255, c0 >> 8
    out[:, 2], out[:, 3] = c1 & 255, c1 >> 8
    for i in range(4):
        out[:, 4 + i] = (bits >> (8 * i)) & 255
    return out


def _encode_alpha(blocks: np.ndarray) -> np.ndarray:
    n = blocks.shape[0]
    a = blocks[..., 3].astype(np.int32)
    a0, a1 = a.max(axis=1), a.min(axis=1)
    k = np.arange(8)
    pal = np.zeros((n, 8), np.int32)
    pal[:, 0], pal[:, 1] = a0, a1
    pal[:, 2:] = ((7 - k[None, 1:7]) * a0[:, None] + k[None, 1:7] * a1[:, None]) // 7
    idx = np.abs(a[:, :, None] - pal[:, None, :]).argmin(axis=2).astype(np.uint64)
    idx = np.where((a0 == a1)[:, None], np.uint64(0), idx)
    bits = (idx << (np.uint64(3) * np.arange(16, dtype=np.uint64))).sum(axis=1, dtype=np.uint64)
    out = np.zeros((n, 8), np.uint8)
    out[:, 0], out[:, 1] = a0, a1
    for i in range(6):
        out[:, 2 + i] = ((bits >> np.uint64(8 * i)) & np.uint64(255)).astype(np.uint8)
    return out


def encode_dxt(img: np.ndarray, dxt5: bool) -> bytes:
    blocks, _, _ = _to_blocks(img)
    if dxt5:
        return np.concatenate([_encode_alpha(blocks), _encode_color(blocks, False)], axis=1).tobytes()
    return _encode_color(blocks, True).tobytes()


def is_pow2(v: int) -> bool:
    return v > 0 and (v & (v - 1)) == 0


def nearest_pow2(v: int, limit: int = 8192) -> int:
    p = 1
    while p * 2 <= v:
        p *= 2
    if p * 2 <= limit and (p * 2 - v) < (v - p):
        p *= 2
    return max(4, min(p, limit))


def _tag(name: str, payload: bytes) -> bytes:
    return b"GGAT" + name[::-1].encode("ascii") + struct.pack("<I", len(payload)) + payload


def build_mipmaps(img: np.ndarray, min_size: int = 4, full_chain: bool = False) -> List[np.ndarray]:
    """Уровни детализации; full_chain — до 1x1 по обеим сторонам (как в DDS), иначе до min_size по меньшей."""
    from PIL import Image
    mips = [img]
    cur = Image.fromarray(img, "RGBA")
    w, h = cur.size
    while (w > 1 or h > 1) if full_chain else (w > min_size and h > min_size):
        w, h = max(1, w // 2), max(1, h // 2)
        cur = cur.resize((w, h), Image.BOX)
        mips.append(np.asarray(cur))
    return mips


def encode_paa(img: np.ndarray, fmt: str = "auto", compress: bool = True) -> Tuple[bytes, str]:
    """RGBA-изображение (HxWx4 uint8, стороны — степени двойки) -> байты PAA."""
    h, w = img.shape[:2]
    if not (is_pow2(w) and is_pow2(h)):
        raise PAAError(tr("размер {0}x{1}: стороны текстуры должны быть степенью двойки").format(w, h))
    if w > 0x7FFF or h > 0x7FFF:
        raise PAAError(tr("слишком большая текстура"))
    alpha = img[..., 3]
    has_alpha = bool((alpha < 255).any())
    if fmt == "auto":
        fmt = "dxt5" if has_alpha else "dxt1"
    dxt5 = fmt == "dxt5"
    ptype = TYPE_DXT5 if dxt5 else TYPE_DXT1

    mips = build_mipmaps(img)
    mip_bytes = [encode_dxt(m, dxt5) for m in mips]
    # как ImageToPAA: крупные mip-уровни (ширина от 256) сжимаются LZO, флаг 0x8000 в ширине
    mip_lzo = [False] * len(mips)
    if compress:
        for i, (m, b) in enumerate(zip(mips, mip_bytes)):
            if m.shape[1] >= 256:
                c = lzo.compress(b)
                if len(c) < len(b):
                    mip_bytes[i], mip_lzo[i] = c, True

    avg = img.reshape(-1, 4).mean(axis=0)
    avg_bgra = bytes(int(round(x)) for x in (avg[2], avg[1], avg[0], avg[3]))
    tags = _tag("AVGC", avg_bgra) + _tag("MAXC", b"\xff\xff\xff\xff")
    if dxt5 and has_alpha:
        tags += _tag("FLAG", struct.pack("<I", 1))   # интерполированная альфа
    elif has_alpha:
        tags += _tag("FLAG", struct.pack("<I", 2))   # 1-битная прозрачность
    offs_len = 12 + 64
    header_len = 2 + len(tags) + offs_len + 2
    offsets, pos = [], header_len
    for m, b in zip(mips, mip_bytes):
        offsets.append(pos)
        pos += 7 + len(b)
    offsets = (offsets + [0] * 16)[:16]
    out = bytearray(struct.pack("<H", ptype))
    out += tags
    out += _tag("OFFS", struct.pack("<16I", *offsets))
    out += b"\x00\x00"  # палитра отсутствует
    for m, b, z in zip(mips, mip_bytes, mip_lzo):
        mh, mw = m.shape[:2]
        out += struct.pack("<HH", mw | (0x8000 if z else 0), mh) + len(b).to_bytes(3, "little") + b
    out += b"\x00" * 6
    return bytes(out), fmt.upper()


def load_image(path) -> np.ndarray:
    from PIL import Image
    with Image.open(path) as im:
        im.seek(0)
        return np.asarray(im.convert("RGBA")).copy()


def fmt_from_suffix(stem: str) -> Optional[str]:
    """Формат по суффиксу имени по правилам ImageToPAA: _co -> DXT1, _ca/_nohq/_smdi/_as -> DXT5."""
    s = stem.lower()
    if s.endswith(("_co", "_mc", "_mco", "_dt", "_sky", "_lco")):
        return "dxt1"
    if s.endswith(("_ca", "_nohq", "_smdi", "_as", "_nopx", "_ads", "_mask")):
        return "dxt5"
    return None


def image_to_paa(src, dst, fmt: str = "auto", resize: str = "error") -> Tuple[int, int, str]:
    """Конвертирует картинку в PAA.

    resize: 'error' — ошибка при размере не степени двойки, 'nearest' — масштабировать к ближайшей.
    Возвращает (ширина, высота, формат).
    """
    from PIL import Image
    img = load_image(src)
    h, w = img.shape[:2]
    if not (is_pow2(w) and is_pow2(h)):
        if resize == "nearest":
            nw, nh = nearest_pow2(w), nearest_pow2(h)
            img = np.asarray(Image.fromarray(img, "RGBA").resize((nw, nh), Image.LANCZOS))
        else:
            raise PAAError(tr("размер {0}x{1} не степень двойки (включите масштабирование)").format(w, h))
    if fmt == "auto":
        fmt = fmt_from_suffix(Path(src).stem) or "auto"
    data, used = encode_paa(img, fmt)
    Path(dst).parent.mkdir(parents=True, exist_ok=True)
    Path(dst).write_bytes(data)
    return img.shape[1], img.shape[0], used
