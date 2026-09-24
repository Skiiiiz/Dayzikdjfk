# -*- coding: utf-8 -*-
"""Ключи и подписи Bohemia Interactive: .biprivatekey, .bikey, .bisign (версии 2 и 3).

Алгоритм совпадает с DSSignFile (так же реализован в HEMTT/armake):
  hash1 = SHA1-контрольная сумма из конца PBO;
  namehash = SHA1(имена файлов в нижнем регистре, по алфавиту, без пустых файлов);
  filehash = SHA1(содержимое файлов с «хешируемыми» расширениями) или "nothing"/"gnihton";
  hash2 = SHA1(hash1 + namehash + prefix\\), hash3 = SHA1(filehash + namehash + prefix\\);
  подпись = RSA(PKCS#1 v1.5 + SHA1 DigestInfo) закрытым ключом, числа хранятся little-endian.
"""

from __future__ import annotations

from dztk.i18n import tr

import hashlib
import random
import struct
from dataclasses import dataclass
from pathlib import Path
from typing import Tuple

from .pbo import PBO, read_pbo

V2_SKIP = {"paa", "jpg", "p3d", "tga", "rvmat", "lip", "ogg", "wss", "png", "rtm", "pac", "fxy", "wrp"}
V3_HASH = {"sqf", "inc", "bikb", "ext", "fsm", "sqm", "hpp", "cfg", "sqs", "h", "sqfc"}

BLOB_PUBLIC = 0x0206
BLOB_PRIVATE = 0x0207
CALG_RSA_SIGN = 0x2400


class SignError(ValueError):
    pass


def _le(n: int, size: int) -> bytes:
    return n.to_bytes(size, "little")


def _from_le(b: bytes) -> int:
    return int.from_bytes(b, "little")


def _z(s: str) -> bytes:
    return s.encode("utf-8") + b"\x00"


def _read_z(d: bytes, p: int) -> Tuple[str, int]:
    e = d.find(b"\x00", p)
    if e < 0:
        raise SignError(tr("повреждённый файл ключа"))
    return d[p:e].decode("utf-8", "replace"), e + 1


# --------------------------------------------------------------------------------------
# Ключи
# --------------------------------------------------------------------------------------

@dataclass
class PublicKey:
    authority: str
    n: int
    e: int = 65537
    bits: int = 1024

    def to_bytes(self) -> bytes:
        size = self.bits // 8
        return (_z(self.authority) + struct.pack("<III", size + 20, BLOB_PUBLIC, CALG_RSA_SIGN) + b"RSA1" +
                struct.pack("<II", self.bits, self.e) + _le(self.n, size))

    @classmethod
    def from_bytes(cls, d: bytes, p: int = 0) -> Tuple["PublicKey", int]:
        authority, p = _read_z(d, p)
        _len, blob, alg = struct.unpack_from("<III", d, p)
        p += 12
        if d[p:p + 4] != b"RSA1":
            raise SignError(tr("это не открытый ключ BI (.bikey)"))
        bits, e = struct.unpack_from("<II", d, p + 4)
        p += 12
        size = bits // 8
        n = _from_le(d[p:p + size])
        return cls(authority, n, e, bits), p + size

    @classmethod
    def load(cls, path) -> "PublicKey":
        return cls.from_bytes(Path(path).read_bytes())[0]


@dataclass
class PrivateKey:
    authority: str
    n: int
    e: int
    d: int
    p: int
    q: int
    bits: int = 1024

    def public(self) -> PublicKey:
        return PublicKey(self.authority, self.n, self.e, self.bits)

    def to_bytes(self) -> bytes:
        size, half = self.bits // 8, self.bits // 16
        dp, dq = self.d % (self.p - 1), self.d % (self.q - 1)
        qinv = pow(self.q, -1, self.p)
        body = (b"RSA2" + struct.pack("<II", self.bits, self.e) + _le(self.n, size) + _le(self.p, half) +
                _le(self.q, half) + _le(dp, half) + _le(dq, half) + _le(qinv, half) + _le(self.d, size))
        return _z(self.authority) + struct.pack("<III", len(body) + 8, BLOB_PRIVATE, CALG_RSA_SIGN) + body

    @classmethod
    def from_bytes(cls, d: bytes) -> "PrivateKey":
        authority, p = _read_z(d, 0)
        p += 12
        if d[p:p + 4] != b"RSA2":
            raise SignError(tr("это не закрытый ключ BI (.biprivatekey)"))
        bits, e = struct.unpack_from("<II", d, p + 4)
        p += 12
        size, half = bits // 8, bits // 16
        n = _from_le(d[p:p + size]); p += size
        pp = _from_le(d[p:p + half]); p += half
        q = _from_le(d[p:p + half]); p += half
        p += 3 * half
        dd = _from_le(d[p:p + size])
        return cls(authority, n, e, dd, pp, q, bits)

    @classmethod
    def load(cls, path) -> "PrivateKey":
        return cls.from_bytes(Path(path).read_bytes())


def _is_probable_prime(n: int, rng: random.SystemRandom, rounds: int = 40) -> bool:
    if n < 2:
        return False
    for sp in (2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37):
        if n % sp == 0:
            return n == sp
    d, r = n - 1, 0
    while d % 2 == 0:
        d //= 2
        r += 1
    for _ in range(rounds):
        a = rng.randrange(2, n - 2)
        x = pow(a, d, n)
        if x in (1, n - 1):
            continue
        for _ in range(r - 1):
            x = pow(x, 2, n)
            if x == n - 1:
                break
        else:
            return False
    return True


def _gen_prime(bits: int, rng: random.SystemRandom, e: int) -> int:
    while True:
        c = rng.getrandbits(bits) | (1 << (bits - 1)) | (1 << (bits - 2)) | 1
        if c % e != 1 and _is_probable_prime(c, rng):
            return c


def generate_key(authority: str, bits: int = 1024) -> PrivateKey:
    """Создаёт пару ключей. authority — имя ключа (обычно ник/название мода, латиницей)."""
    if not authority or any(c in authority for c in '\\/:*?"<>| '):
        raise SignError(tr("имя ключа должно быть непустым, без пробелов и символов \\ / : * ? \" < > |"))
    rng = random.SystemRandom()
    e = 65537
    while True:
        p = _gen_prime(bits // 2, rng, e)
        q = _gen_prime(bits // 2, rng, e)
        if p == q:
            continue
        n = p * q
        if n.bit_length() != bits:
            continue
        phi = (p - 1) * (q - 1)
        try:
            d = pow(e, -1, phi)
        except ValueError:
            continue
        if p < q:
            p, q = q, p
        return PrivateKey(authority, n, e, d, p, q, bits)


def create_keys(authority: str, out_dir, bits: int = 1024) -> Tuple[Path, Path]:
    key = generate_key(authority, bits)
    out = Path(out_dir)
    out.mkdir(parents=True, exist_ok=True)
    priv, pub = out / f"{authority}.biprivatekey", out / f"{authority}.bikey"
    if priv.exists():
        raise SignError(tr("{0} уже существует — не перезаписываю закрытый ключ").format(priv.name))
    priv.write_bytes(key.to_bytes())
    pub.write_bytes(key.public().to_bytes())
    return priv, pub


# --------------------------------------------------------------------------------------
# Подпись
# --------------------------------------------------------------------------------------

def _hashes(pbo: PBO, version: int) -> Tuple[bytes, bytes, bytes]:
    if not pbo.checksum:
        raise SignError(tr("у PBO нет контрольной суммы в конце — пересоберите его"))
    files = sorted(((e.name.lower(), e) for e in pbo.entries), key=lambda x: x[0])
    names = hashlib.sha1()
    content = hashlib.sha1()
    hashed = False
    for low, e in files:
        if e.data_size == 0:
            continue
        names.update(low.encode("utf-8"))
        ext = low.rsplit(".", 1)[-1] if "." in low.rsplit("\\", 1)[-1] else ""
        take = (ext not in V2_SKIP) if version == 2 else (ext in V3_HASH)
        if take:
            content.update(pbo.read(e))
            hashed = True
    if not hashed:
        content.update(b"nothing" if version == 2 else b"gnihton")
    namehash, filehash = names.digest(), content.digest()
    prefix = pbo.prefix
    tail = (prefix.encode("utf-8") + (b"" if prefix.endswith("\\") else b"\\")) if prefix else b""
    hash1 = pbo.checksum
    hash2 = hashlib.sha1(hash1 + namehash + tail).digest()
    hash3 = hashlib.sha1(filehash + namehash + tail).digest()
    return hash1, hash2, hash3


def _pad(h: bytes, size: int) -> int:
    digest_info = bytes.fromhex("3021300906052b0e03021a05000414")
    body = b"\x00\x01" + b"\xff" * (size - 3 - len(digest_info) - len(h)) + b"\x00" + digest_info + h
    return int.from_bytes(body, "big")


def sign_pbo(pbo_path, key: PrivateKey, version: int = 3) -> Path:
    pbo_path = Path(pbo_path)
    pbo = read_pbo(pbo_path)
    size = key.bits // 8
    sigs = [pow(_pad(h, size), key.d, key.n) for h in _hashes(pbo, version)]
    out = key.public().to_bytes()
    out += struct.pack("<I", size) + _le(sigs[0], size)
    out += struct.pack("<I", version)
    out += struct.pack("<I", size) + _le(sigs[1], size)
    out += struct.pack("<I", size) + _le(sigs[2], size)
    dst = pbo_path.with_name(f"{pbo_path.name}.{key.authority}.bisign")
    dst.write_bytes(out)
    return dst


def verify_pbo(pbo_path, bisign_path, bikey_path=None) -> Tuple[bool, str]:
    """Проверяет подпись. Если bikey не указан — берётся ключ из самой подписи."""
    d = Path(bisign_path).read_bytes()
    key, p = PublicKey.from_bytes(d)
    if bikey_path:
        trusted = PublicKey.load(bikey_path)
        if (trusted.n, trusted.e) != (key.n, key.e):
            return False, tr("подпись сделана другим ключом ({0}), а не {1}").format(key.authority, trusted.authority)
        key = trusted
    size = key.bits // 8
    sig = []
    ln = struct.unpack_from("<I", d, p)[0]; p += 4
    sig.append(_from_le(d[p:p + ln])); p += ln
    version = struct.unpack_from("<I", d, p)[0]; p += 4
    for _ in range(2):
        ln = struct.unpack_from("<I", d, p)[0]; p += 4
        sig.append(_from_le(d[p:p + ln])); p += ln
    pbo = read_pbo(pbo_path)
    for i, (s, h) in enumerate(zip(sig, _hashes(pbo, version)), 1):
        if pow(s, key.e, key.n) != _pad(h, size):
            what = {1: tr("содержимое PBO изменено"), 2: tr("имена файлов или prefix изменены"),
                    3: tr("файлы или prefix изменены")}[i]
            return False, tr("подпись не совпадает (hash{0}): {1}").format(i, what)
    return True, tr("подпись верна (v{0}, ключ {1})").format(version, key.authority)
