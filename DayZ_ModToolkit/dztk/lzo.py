# -*- coding: utf-8 -*-
"""Распаковка LZO1X (используется для сжатых mip-уровней в PAA)."""

from __future__ import annotations


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
            raise LZOError("неожиданный конец LZO-потока")
        b = src[ip]
        ip += 1
        return b

    def literals(count: int) -> None:
        nonlocal ip
        if ip + count > n:
            raise LZOError("неожиданный конец LZO-потока (литералы)")
        dst.extend(src[ip:ip + count])
        ip += count

    def copy_match(dist_back: int, count: int) -> None:
        pos = len(dst) - dist_back
        if pos < 0:
            raise LZOError("ссылка LZO за пределы уже распакованных данных")
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
        raise LZOError(f"LZO: ожидалось {out_len} байт, получено {len(dst)}")
    return bytes(dst), ip - start
