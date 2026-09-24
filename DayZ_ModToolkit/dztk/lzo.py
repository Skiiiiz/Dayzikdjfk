# -*- coding: utf-8 -*-
"""Распаковка LZO1X (используется для сжатых mip-уровней в PAA)."""

from __future__ import annotations

from dztk.i18n import tr


class LZOError(ValueError):
    pass


def decompress(src: bytes, out_len: int, start: int = 0) -> tuple:
    """Распаковывает LZO1X-поток.

    Возвращает (данные, число_прочитанных_байт). Поток заканчивается маркером
    конца (M4-совпадение с нулевым смещением), поэтому длина входа заранее не нужна.
    """
    dst = bytearray()
    ip = start
    n = len(src)

    def byte() -> int:
        nonlocal ip
        if ip >= n:
            raise LZOError(tr("неожиданный конец LZO-потока"))
        b = src[ip]
        ip += 1
        return b

    def literals(count: int) -> None:
        nonlocal ip
        if ip + count > n:
            raise LZOError(tr("неожиданный конец LZO-потока (литералы)"))
        dst.extend(src[ip:ip + count])
        ip += count

    def copy_match(dist_back: int, count: int) -> None:
        pos = len(dst) - dist_back
        if pos < 0:
            raise LZOError(tr("ссылка LZO за пределы уже распакованных данных"))
        if dist_back >= count:
            dst.extend(dst[pos:pos + count])
        else:  # перекрывающееся копирование
            for i in range(count):
                dst.append(dst[pos + i])

    def long_len(t: int, base: int) -> int:
        if t == 0:
            t = base
            while True:
                b = byte()
                if b != 0:
                    return t + b
                t += 255
        return t

    # Состояния: 'loop' — ждём литерал/совпадение, 'first' — сразу после литералов,
    # 'match' — t уже прочитан и это совпадение.
    t = src[ip] if ip < n else 0
    state = "loop"
    if t > 17:
        ip += 1
        t -= 17
        if t < 4:
            literals(t)
            t = byte()
            state = "match"
        else:
            literals(t)
            state = "first"

    while True:
        if state == "loop":
            t = byte()
            if t >= 16:
                state = "match"
                continue
            t = long_len(t, 15)
            literals(t + 3)
            state = "first"
            continue

        if state == "first":
            t = byte()
            if t >= 16:
                state = "match"
                continue
            b = byte()
            copy_match(1 + 0x0800 + (t >> 2) + (b << 2), 3)
        else:  # match
            if t >= 64:
                b = byte()
                copy_match(1 + ((t >> 2) & 7) + (b << 3), (t >> 5) - 1 + 2)
            elif t >= 32:
                t = long_len(t & 31, 31)
                b0, b1 = byte(), byte()
                copy_match(1 + ((b0 | (b1 << 8)) >> 2), t + 2)
            elif t >= 16:
                high = (t & 8) << 11
                t = long_len(t & 7, 7)
                b0, b1 = byte(), byte()
                dist = high + ((b0 | (b1 << 8)) >> 2)
                if dist == 0:
                    break  # конец потока
                copy_match(dist + 0x4000, t + 2)
            else:
                b = byte()
                copy_match(1 + (t >> 2) + (b << 2), 2)

        # match_done: младшие 2 бита предпоследнего байта = число хвостовых литералов
        t = src[ip - 2] & 3
        if t == 0:
            state = "loop"
            continue
        literals(t)
        t = byte()
        state = "match"

    if len(dst) != out_len:
        raise LZOError(tr("LZO: ожидалось {0} байт, получено {1}").format(out_len, len(dst)))
    return bytes(dst), ip - start


# --------------------------------------------------------------------------------------
# Сжатие LZO1X-1 (порт lzo1x_1_compress из minilzo; поток совместим с любым LZO1X-декодером)
# --------------------------------------------------------------------------------------

_D_BITS = 14
_D_MASK = (1 << _D_BITS) - 1
_M2_MAX_LEN, _M2_MAX_OFFSET = 8, 0x0800
_M3_MAX_LEN, _M3_MAX_OFFSET = 33, 0x4000
_M4_MAX_LEN = 9
_M3_MARKER, _M4_MARKER = 32, 16


def _emit_literals(out: bytearray, src, start: int, t: int, first: bool) -> None:
    if t == 0:
        return
    if first and t <= 238:
        out.append(17 + t)
    elif t <= 3:
        out[-2] |= t
    elif t <= 18:
        out.append(t - 3)
    else:
        tt = t - 18
        out.append(0)
        while tt > 255:
            tt -= 255
            out.append(0)
        out.append(tt)
    out += src[start:start + t]


def _do_compress(src, base: int, in_len: int, out: bytearray, ti: int) -> int:
    """Сжимает блок src[base:base+in_len]. Возвращает число «хвостовых» литералов."""
    in_end = base + in_len
    ip_end = base + in_len - 20
    dict_ = [0] * (1 << _D_BITS)
    ip = base
    ii = ip
    ip += 4 - ti if ti < 4 else 0
    frombytes = int.from_bytes
    while True:
        # literal:
        ip += 1 + ((ip - ii) >> 5)
        while True:  # next:
            if ip >= ip_end:
                return in_end - (ii - ti)
            dv = frombytes(src[ip:ip + 4], "little")
            dindex = ((0x1824429D * dv) & 0xFFFFFFFF) >> (32 - _D_BITS) & _D_MASK
            m_pos = base + dict_[dindex]
            dict_[dindex] = ip - base
            if dv != frombytes(src[m_pos:m_pos + 4], "little"):
                break  # -> literal
            ii -= ti
            ti = 0
            t = ip - ii
            if t:
                if t <= 3:
                    out[-2] |= t
                elif t <= 16:
                    out.append(t - 3)
                else:
                    if t <= 18:
                        out.append(t - 3)
                    else:
                        tt = t - 18
                        out.append(0)
                        while tt > 255:
                            tt -= 255
                            out.append(0)
                        out.append(tt)
                out += src[ii:ip]
            # длина совпадения (сравнение блоками для скорости)
            m_len = 4
            limit = ip_end - ip
            while m_len < limit:
                step = min(32, limit - m_len)
                a = src[ip + m_len:ip + m_len + step]
                b = src[m_pos + m_len:m_pos + m_len + step]
                if a == b:
                    m_len += step
                    continue
                k = 0
                while a[k] == b[k]:
                    k += 1
                m_len += k
                break
            m_off = ip - m_pos
            ip += m_len
            ii = ip
            if m_len <= _M2_MAX_LEN and m_off <= _M2_MAX_OFFSET:
                m_off -= 1
                out.append(((m_len - 1) << 5) | ((m_off & 7) << 2))
                out.append(m_off >> 3)
            elif m_off <= _M3_MAX_OFFSET:
                m_off -= 1
                if m_len <= _M3_MAX_LEN:
                    out.append(_M3_MARKER | (m_len - 2))
                else:
                    m_len -= _M3_MAX_LEN
                    out.append(_M3_MARKER)
                    while m_len > 255:
                        m_len -= 255
                        out.append(0)
                    out.append(m_len)
                out.append((m_off << 2) & 0xFF)
                out.append((m_off >> 6) & 0xFF)
            else:
                m_off -= 0x4000
                if m_len <= _M4_MAX_LEN:
                    out.append(_M4_MARKER | ((m_off >> 11) & 8) | (m_len - 2))
                else:
                    m_len -= _M4_MAX_LEN
                    out.append(_M4_MARKER | ((m_off >> 11) & 8))
                    while m_len > 255:
                        m_len -= 255
                        out.append(0)
                    out.append(m_len)
                out.append((m_off << 2) & 0xFF)
                out.append((m_off >> 6) & 0xFF)
            # goto next


def compress(data: bytes) -> bytes:
    """Сжатие LZO1X-1."""
    src = bytes(data)
    out = bytearray()
    in_len = len(src)
    ip = 0
    l = in_len
    t = 0
    while l > 20:
        ll = min(l, 49152)
        t = _do_compress(src, ip, ll, out, t)
        ip += ll
        l -= ll
    t += l
    if t > 0:
        start = in_len - t
        _emit_literals(out, src, start, t, first=(len(out) == 0))
    out += bytes((_M4_MARKER | 1, 0, 0))
    return bytes(out)
