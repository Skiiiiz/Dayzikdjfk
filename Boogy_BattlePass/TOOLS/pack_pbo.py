#!/usr/bin/env python3
"""Упаковка Boogy_BattlePass в PBO без бинаризации (config.cpp остаётся текстом — DayZ его читает).

Запуск из корня репозитория:
    python3 Boogy_BattlePass/TOOLS/pack_pbo.py [выходная_папка]
Результат: <выход>/@Boogy_BattlePass/addons/Boogy_BattlePass.pbo
Подпись (.bisign) делается отдельно DSSignFile из DayZ Tools.
"""
import hashlib, os, struct, sys, time

MOD = "Boogy_BattlePass"
SRC = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
EXCLUDE_DIRS = {"Docs", "Examples", "TOOLS", "__pycache__"}
EXCLUDE_FILES = {"README.md"}


def collect():
    files = []
    for root, dirs, names in os.walk(SRC):
        dirs[:] = sorted(d for d in dirs if d not in EXCLUDE_DIRS and not d.startswith("."))
        for n in sorted(names):
            if root == SRC and n in EXCLUDE_FILES:
                continue
            full = os.path.join(root, n)
            rel = os.path.relpath(full, SRC).replace("/", "\\")
            files.append((rel, full))
    return files


def pack(out_dir):
    files = collect()
    ts = int(time.time())
    header = bytearray()
    # product entry
    header += b"\0" + struct.pack("<5I", 0x56657273, 0, 0, 0, 0)
    header += b"prefix\0" + MOD.encode() + b"\0\0"
    blobs = []
    for rel, full in files:
        data = open(full, "rb").read()
        header += rel.encode("utf-8") + b"\0" + struct.pack("<5I", 0, len(data), 0, ts, len(data))
        blobs.append(data)
    header += b"\0" + struct.pack("<5I", 0, 0, 0, 0, 0)
    body = bytes(header) + b"".join(blobs)
    body += b"\0" + hashlib.sha1(body).digest()

    addons = os.path.join(out_dir, "@" + MOD, "addons")
    os.makedirs(addons, exist_ok=True)
    path = os.path.join(addons, MOD + ".pbo")
    with open(path, "wb") as f:
        f.write(body)
    return path, files


if __name__ == "__main__":
    out = sys.argv[1] if len(sys.argv) > 1 else os.path.join(SRC, "..", "build")
    p, fl = pack(out)
    print(f"{p}  ({os.path.getsize(p)} bytes, {len(fl)} files)")
    for rel, _ in fl:
        print("  " + rel)
