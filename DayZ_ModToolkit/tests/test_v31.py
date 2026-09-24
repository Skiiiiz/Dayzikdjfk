# -*- coding: utf-8 -*-
"""Тесты версии 3.1: EDDS/LZ4/атлас, p3d, rvmat, requiredAddons, папка миссии, автоисправление,
запуск и логи, редактор types.xml, проверка обновлений."""

import os
import random
import struct
import subprocess
import sys
import time
from pathlib import Path

import numpy as np
import pytest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))

from dztk import checks, edds, fixes, launch, mission, p3d, update  # noqa: E402
from dztk import typesedit as te  # noqa: E402

CLI = [sys.executable, str(ROOT / "dayz_toolkit.py"), "--cli"]
DAYZ_CE = os.environ.get("DAYZ_CE", "")
EDDS_SAMPLES = os.environ.get("EDDS_SAMPLES", "")


def codes(issues, level=None):
    return {i.code for i in issues if level is None or i.level == level}


def run_cli(*args):
    env = dict(os.environ, DZTK_LANG="ru")
    return subprocess.run(CLI + [str(a) for a in args], capture_output=True, env=env, text=True, encoding="utf-8")


# ---------------------------------------------------------------- LZ4 / EDDS / атлас

_LZ4 = [b"", b"a", b"abcd" * 1000, bytes(random.Random(1).getrandbits(8) for _ in range(20000)),
        bytes(random.Random(2).choice(b"xyz") for _ in range(70000)), bytes(65536 * 2)]


@pytest.mark.parametrize("data", _LZ4, ids=[f"lz4_{i}" for i in range(len(_LZ4))])
def test_lz4_roundtrip(data):
    c = edds.lz4_compress_block(data)
    out = bytearray()
    edds.lz4_decompress_block(c, out, len(data))
    assert bytes(out) == data
    try:
        import lz4.block as ref
    except ImportError:
        return
    if data:
        assert ref.decompress(c, uncompressed_size=len(data)) == data


@pytest.mark.parametrize("fmt", ["dxt1", "dxt5"])
def test_edds_roundtrip(tmp_path, fmt):
    y, x = np.mgrid[0:128, 0:256]
    img = np.zeros((128, 256, 4), np.uint8)
    img[..., 0] = x
    img[..., 1] = y * 2
    img[..., 2] = 90
    img[..., 3] = 255 if fmt == "dxt1" else (x % 256)
    data, used = edds.encode_edds(img, fmt)
    assert used == fmt.upper() or used.lower() == fmt
    info, mips = edds.parse(data)
    assert (info.width, info.height) == (256, 128)
    assert len(mips) == 9                      # 256x128 ... 1x1
    p = tmp_path / "t.edds"
    p.write_bytes(data)
    rgba, _ = edds.read_edds(p)
    assert rgba.shape == (128, 256, 4)
    assert np.abs(rgba[..., :3].astype(int) - img[..., :3]).mean() < 6
    raw, _ = edds.encode_edds(img, fmt, compress=False)
    assert len(data) < len(raw)
    assert checks.check_edds(p) == []


@pytest.mark.skipif(not EDDS_SAMPLES, reason="нет образцов .edds (переменная EDDS_SAMPLES)")
def test_edds_reference_samples():
    files = list(Path(EDDS_SAMPLES).rglob("*.edds"))
    assert files
    for f in files:
        rgba, info = edds.read_edds(f)
        assert rgba.shape[:2] == (info.height, info.width)


def test_atlas(tmp_path):
    from PIL import Image
    src = tmp_path / "icons"
    src.mkdir()
    for i, (w, h) in enumerate([(64, 64), (32, 32), (100, 20), (16, 48)]):
        Image.new("RGBA", (w, h), (i * 50, 100, 200, 255)).save(src / f"icon_{i}.png")
    r = run_cli("atlas", src, "-o", tmp_path / "out", "--name", "myicons", "--prefix", "MyMod/gui/imagesets")
    assert r.returncode == 0, r.stderr
    tex = tmp_path / "out/myicons.edds"
    iset = (tmp_path / "out/myicons.imageset").read_text(encoding="utf-8")
    rgba, info = edds.read_edds(tex)
    assert info.width & (info.width - 1) == 0 and info.height & (info.height - 1) == 0
    assert iset.count("ImageSetDefClass") == 4 and "MyMod/gui/imagesets/myicons.edds" in iset
    assert checks.check_paths([str(tmp_path / "out")]).count("error") == 0


# ---------------------------------------------------------------- p3d

def _mlod(texture: str, material: str) -> bytes:
    out = b"MLOD" + struct.pack("<II", 257, 1)
    out += b"P3DM" + struct.pack("<6I", 0x1C, 0x100, 3, 1, 1, 0)
    for pt in ((0, 0, 0), (1, 0, 0), (0, 1, 0)):
        out += struct.pack("<3fI", *pt, 0)
    out += struct.pack("<3f", 0, 0, 1)
    out += struct.pack("<I", 3) + b"".join(struct.pack("<IIff", i, 0, 0, 0) for i in range(4)) + struct.pack("<I", 0)
    out += texture.encode() + b"\0" + material.encode() + b"\0"
    out += b"TAGG" + b"\x01" + b"#EndOfFile#\0" + struct.pack("<I", 0)
    return out + struct.pack("<f", 0.0)


def test_p3d_refs(tmp_path):
    mod = tmp_path / "MyMod"
    (mod / "data").mkdir(parents=True)
    (mod / "config.cpp").write_text(
        'class CfgPatches { class MyMod { units[]={}; weapons[]={}; requiredVersion=0.1; '
        'requiredAddons[]={"DZ_Data"}; }; };\n', encoding="utf-8")
    (mod / "data/box_co.paa").write_bytes(b"")
    p = mod / "data/box.p3d"
    p.write_bytes(_mlod(r"MyMod\data\box_co.paa", r"MyMod\data\box.rvmat"))
    info = p3d.read_p3d(p)
    assert info.kind == "MLOD" and info.lods == [0.0]
    assert info.textures == {r"MyMod\data\box_co.paa"} and info.materials == {r"MyMod\data\box.rvmat"}
    rep = checks.check_paths([str(mod)])
    missing = [i for i in rep.issues if i.code == "missing-file"]
    assert len(missing) == 1 and "box.rvmat" in missing[0].message
    p.write_bytes(_mlod("a", "b")[:-30])
    assert "p3d" in codes(checks.check_p3d(p))


# ---------------------------------------------------------------- rvmat, requiredAddons

def test_rvmat_and_required_addons(tmp_path):
    mod = tmp_path / "MyMod"
    (mod / "data").mkdir(parents=True)
    (mod / "config.cpp").write_text(
        'class CfgPatches {\n class MyMod_A { units[]={}; weapons[]={}; requiredVersion=0.1; '
        'requiredAddons[]={"DZ_Data","MyMod_B"}; };\n'
        ' class MyMod_B { units[]={}; weapons[]={}; requiredVersion=0.1; requiredAddons[]={"MyMod_A"}; };\n'
        ' class MyMod_C { units[]={}; weapons[]={}; requiredVersion=0.1; requiredAddons[]={"MyMod_C"}; };\n};\n',
        encoding="utf-8")
    (mod / "data/mat.rvmat").write_text(
        'ambient[]={1,1,1,1};\nclass Stage1 { texture="MyMod\\data\\nohq.paa"; };\n'
        'class Stage2 { texture="#(argb,8,8,3)color(0.5,0.5,0.5,1,DT)"; };\n', encoding="utf-8")
    rep = checks.check_paths([str(mod)])
    got = codes(rep.issues)
    assert {"required-self", "required-cycle"} <= got
    assert any(i.code == "missing-file" and "nohq.paa" in i.message for i in rep.issues)
    (mod / "data/bad.rvmat").write_text("class Stage1 { texture=\"x.paa\";\n", encoding="utf-8")
    assert any(i.level == "error" for i in checks.check_rvmat(mod / "data/bad.rvmat"))


# ---------------------------------------------------------------- миссия

def _mission(root: Path):
    (root / "db").mkdir(parents=True)
    (root / "custom").mkdir()
    (root / "cfgeconomycore.xml").write_text(
        '<economycore>\n<ce folder="custom">\n<file name="my_types.xml" type="types"/>\n'
        '<file name="nothere.xml" type="types"/>\n</ce>\n</economycore>\n', encoding="utf-8")
    (root / "cfglimitsdefinition.xml").write_text(
        '<lists><categories><category name="weapons"/></categories><tags><tag name="floor"/></tags>'
        '<usageflags><usage name="Military"/></usageflags><valueflags><value name="Tier1"/></valueflags></lists>',
        encoding="utf-8")
    (root / "db/types.xml").write_text(
        '<types>\n<type name="AKM">\n<nominal>5</nominal><min>2</min><category name="weapons"/>'
        '<usage name="Military"/><value name="Tier1"/>\n</type>\n'
        '<type name="Bad">\n<category name="weapon"/>\n<usage name="Military"/>\n</type>\n</types>\n',
        encoding="utf-8")
    (root / "custom/my_types.xml").write_text(
        '<types>\n<type name="AKM"><nominal>1</nominal></type>\n</types>\n', encoding="utf-8")
    (root / "db/events.xml").write_text(
        '<events><event name="VehicleX"><children><child type="NoSuchCar"/></children></event></events>',
        encoding="utf-8")
    (root / "cfgeventgroups.xml").write_text('<eventgroupdef><group name="G1"/></eventgroupdef>', encoding="utf-8")
    (root / "cfgeventspawns.xml").write_text(
        '<eventposdef><event name="VehicleX"><pos x="1" z="1" group="G2"/></event></eventposdef>', encoding="utf-8")
    (root / "cfgrandompresets.xml").write_text('<randompresets><cargo name="p1"><item name="AKM"/></cargo>'
                                               '</randompresets>', encoding="utf-8")
    (root / "cfgspawnabletypes.xml").write_text(
        '<spawnabletypes><type name="AKM"><cargo preset="p2"/></type></spawnabletypes>', encoding="utf-8")
    (root / "cfggameplay.json").write_text('{"WorldsData": {"objectSpawnersArr": ["custom/spawn.json"]}}',
                                           encoding="utf-8")


def test_mission_cross_checks(tmp_path):
    root = tmp_path / "dayzOffline.test"
    _mission(root)
    assert mission.is_mission_dir(root)
    iss = mission.check_mission(root)
    got = codes(iss)
    assert {"mission-ce", "mission-limits", "mission-dup-type", "mission-event-child", "mission-group",
            "mission-preset", "mission-gameplay-file"} <= got
    assert any("weapon" in i.message for i in iss if i.code == "mission-limits")
    rep = checks.check_paths([str(root)])
    assert "mission-group" in codes(rep.issues)


@pytest.mark.skipif(not DAYZ_CE, reason="нет официальных миссий (переменная DAYZ_CE)")
def test_official_missions_have_no_errors():
    dirs = [d for d in Path(DAYZ_CE).iterdir() if d.is_dir() and mission.is_mission_dir(d)]
    assert dirs
    for d in dirs:
        errors = [i.format() for i in mission.check_mission(d) if i.level == "error"]
        assert errors == [], d.name


# ---------------------------------------------------------------- автоисправление

def _broken_mod(root: Path) -> Path:
    mod = root / "FixMod"
    (mod / "Scripts/4_World").mkdir(parents=True)
    (mod / "sounds").mkdir()
    (mod / "config.cpp").write_text(
        'class CfgPatches\n{\n\tclass FixMod\n\t{\n\t\tunits[] = {};\n\t\tweapons[] = {};\n'
        '\t\trequiredVersion = 0.1;\n\t\trequiredAddons[] = {"DZ_Data"};\n\t};\n};\n'
        'class CfgMods\n{\n\tclass FixMod\n\t{\n\t\ttype = "mod";\n\t\tclass defs\n\t\t{\n'
        '\t\t\tclass worldScriptModule\n\t\t\t{\n\t\t\t\tvalue = "";\n'
        '\t\t\t\tfiles[] = {"FixMod/Scripts/4_World"};\n\t\t\t}\n\t\t};\n\t};\n};\n'
        'class CfgVehicles\n{\n\tclass MyJacket: Clothing_Base\n\t{\n\t\tscope = 2;\n'
        '\t\tdisplayName = "$STR_FixMod_Jacket";\n\t};\n};\n'
        'class CfgSoundShaders\n{\n\tclass Shot_SoundShader\n\t{\n'
        '\t\tsamples[] = {{"FixMod\\sounds\\Выстрел АК", 1}};\n\t};\n};\n', encoding="utf-8")
    (mod / "stringtable.csv").write_text('"Language","original","english"\n"STR_FixMod_Title","Title"\n',
                                         encoding="utf-8")
    (mod / "Scripts/4_World/a.c").write_text(
        'class FixModHelper\n{\n\tstring Title()\n\t{\n\t\treturn "#STR_FixMod_Title";\n\t}\n}\n', encoding="utf-8")
    (mod / "sounds/Выстрел АК.ogg").write_bytes(b"OggS" + bytes(100))
    return mod


def test_autofix(tmp_path):
    mod = _broken_mod(tmp_path)
    rep = checks.check_paths([str(mod)])
    got = codes(rep.issues)
    assert {"undefined-base", "class-semicolon", "missing-string", "filename", "csv"} <= got
    plan = fixes.plan(mod, rep.issues)
    kinds = [f.kind for f in plan]
    assert {"base", "semicolon", "strings", "rename", "csv"} <= set(kinds)
    assert kinds[-1] == "rename"
    ctx = fixes.apply(mod, plan)
    assert ctx.backup is not None and (ctx.backup / "config.cpp").is_file()
    text = (mod / "config.cpp").read_text(encoding="utf-8")
    assert "class Clothing_Base;" in text and "vystrel_ak" in text.lower()
    assert (mod / "sounds/vystrel_ak.ogg").is_file()
    after = checks.check_paths([str(mod)])
    left = {i.code for i in after.issues if i.level in ("error", "warning")} & \
        {"undefined-base", "class-semicolon", "missing-string", "filename", "csv"}
    assert left == set(), [i.format() for i in after.issues]
    assert after.count("error") == 0


def test_autofix_cli(tmp_path):
    mod = _broken_mod(tmp_path)
    r = run_cli("fix", mod)
    assert r.returncode == 0 and "--apply" in r.stdout
    assert "class Clothing_Base;" not in (mod / "config.cpp").read_text(encoding="utf-8")
    r = run_cli("fix", mod, "--apply", "--no-backup")
    assert r.returncode == 0 and "ошибок 0" in r.stdout
    assert not list(tmp_path.glob("FixMod_backup_*"))


# ---------------------------------------------------------------- запуск и логи

SCRIPT_LOG = """ 9:41:30.100 SCRIPT       : Creating script module 'Game'
 9:41:33.587 SCRIPT       (E): Can't compile "World" script module!

FixMod/Scripts/4_World/a.c(5): Undefined variable 'bar'
 9:41:40.001 SCRIPT    (W): @"scripts/3_Game/foo.c,33": Possible variable name conflict 'x'
 9:42:01.587 SCRIPT       (E): NULL pointer to instance
Class:      'PlayerBase'
Function: 'OnScheduledTick'
Stack trace:
FixMod/Scripts/4_World/a.c:3 Function Title
scripts/4_World/entities/manbase/playerbase.c:4412 Function OnTick

 9:42:02.587 SCRIPT       (E): NULL pointer to instance
Class:      'PlayerBase'
Function: 'OnScheduledTick'
Stack trace:
FixMod/Scripts/4_World/a.c:3 Function Title
scripts/4_World/entities/manbase/playerbase.c:4412 Function OnTick

"""

RPT = """ 9:40:00 Updating base class ->Foo, by bar/config.bin/CfgVehicles/X/
 9:40:01 Warning Message: No entry 'bin\\config.bin/CfgVehicles/MyJacket.scope'.
 9:40:02 ErrorMessage: Cannot open object FixMod\\data\\box.p3d
"""


def test_parse_logs(tmp_path):
    e = launch.parse_log(SCRIPT_LOG, "script_x.log")
    assert [x.level for x in e] == ["error", "error", "warning", "error"]
    assert (e[1].file, e[1].line) == ("FixMod/Scripts/4_World/a.c", 5)
    assert (e[2].file, e[2].line) == ("scripts/3_Game/foo.c", 33)
    assert e[3].line == 3 and len(e[3].stack) == 2 and "2" in e[3].message   # повтор схлопнут
    r = launch.parse_log(RPT, "DayZ_x64.RPT")
    assert [x.level for x in r] == ["warning", "error"] and "box.p3d" in r[1].message

    mod = _broken_mod(tmp_path)
    assert launch.resolve("FixMod/Scripts/4_World/a.c", [mod]) == mod / "Scripts/4_World/a.c"
    assert launch.resolve("fixmod/scripts/4_world/A.C", [mod]) == mod / "Scripts/4_World/a.c"
    assert launch.resolve("scripts/3_Game/foo.c", [mod]) is None


def test_log_tail_and_cli(tmp_path):
    prof = tmp_path / "profiles"
    prof.mkdir()
    log = prof / "script_2026-01-01_10-00-00.log"
    log.write_text(SCRIPT_LOG[:200], encoding="utf-8")
    tail = launch.LogTail([prof], since=time.time() - 60)
    first = tail.poll()
    with open(log, "a", encoding="utf-8") as fh:
        fh.write(SCRIPT_LOG[200:])
    second = tail.poll() + tail.poll()
    assert len(first) + len(second) >= 3
    assert launch.find_logs([prof]) == [log]
    r = run_cli("logs", prof, "--mods", str(_broken_mod(tmp_path)))
    assert r.returncode == 1 and "Undefined variable" in r.stdout and "a.c:5" in r.stdout


def test_launch_commands(tmp_path):
    game = tmp_path / "DayZ"
    game.mkdir()
    (game / "DayZ_x64.exe").write_bytes(b"")
    (game / "DayZServer_x64.exe").write_bytes(b"")
    (game / "serverDZ.cfg").write_text("", encoding="utf-8")
    mod = tmp_path / "@MyMod"
    (mod / "addons").mkdir(parents=True)
    o = launch.LaunchOptions(game_dir=str(game), mods=[str(mod), "@CF"], server_mods=["@Admin"],
                             mission="mpmissions\\dayzOffline.chernarusplus", profiles="profiles")
    c = launch.client_command(o)
    s = launch.server_command(o)
    assert c[0].endswith("DayZ_x64.exe") and f"-mod={mod};@CF" in c and "-filePatching" in c
    assert "-connect=127.0.0.1" in c and "-port=2302" in c
    assert s[0].endswith("DayZServer_x64.exe") and "-config=serverDZ.cfg" in s and "-serverMod=@Admin" in s
    assert "-mission=mpmissions\\dayzOffline.chernarusplus" in s and "-dologs" in s
    assert launch.check_options(o, True) == [] and launch.check_options(o, False) == []
    o.mods.append(str(tmp_path / "@Missing"))
    assert launch.check_options(o, False)
    r = run_cli("launch", "--game", game, "--server-dir", game, "--mod", mod, "--dry-run")
    assert r.returncode == 0 and "DayZServer_x64.exe" in r.stdout and "DayZ_x64.exe" in r.stdout


# ---------------------------------------------------------------- редактор types.xml

TYPES = """<?xml version="1.0" encoding="UTF-8" standalone="yes" ?>
<types>
    <!-- оружие -->
    <type name="AKM">
        <nominal>4</nominal>
        <lifetime>7200</lifetime>
        <restock>3600</restock>
        <min>2</min>
        <quantmin>30</quantmin>
        <quantmax>80</quantmax>
        <cost>100</cost>
        <flags count_in_cargo="0" count_in_hoarder="0" count_in_map="1" count_in_player="0" crafted="0" deloot="0"/>
        <category name="weapons"/>
        <usage name="Military"/>
        <value name="Tier4"/>
    </type>
    <!--<type name="Old"><nominal>1</nominal></type>-->
    <type name="Apple">
        <nominal>20</nominal>
        <min>10</min>
        <category name="food"/>
        <usage name="Town"/>
        <usage name="Village"/>
    </type>
    <type name="Empty"/>
</types>
"""


def test_types_editor(tmp_path):
    p = tmp_path / "types.xml"
    p.write_text(TYPES, encoding="utf-8")
    tf = te.load(p)
    assert [r.name for r in tf.rows] == ["AKM", "Apple", "Empty"]
    assert te.to_text(tf) == TYPES                                # без правок — байт в байт
    weapons = [r for r in tf.rows if te.matches(r, category="weapons")]
    assert [r.name for r in weapons] == ["AKM"]
    assert [r.name for r in tf.rows if te.matches(r, usage="village")] == ["Apple"]
    assert [r.name for r in tf.rows if te.matches(r, name="a*")] == ["AKM", "Apple"]
    assert te.bulk(tf.rows, "nominal", "mul", "1.5") == 2
    assert te.bulk(weapons, "usage", "addlist", "Town, @Custom") == 1
    assert te.bulk(tf.rows, "count_in_player", "set", "1") == 3
    te.set_value(tf.rows[1], "cost", "")
    with pytest.raises(te.TypesError):
        te.set_value(tf.rows[0], "nominal", "abc")
    with pytest.raises(te.TypesError):
        te.set_value(tf.rows[0], "category", "a, b")
    te.save(tf)
    text = p.read_text(encoding="utf-8")
    assert "<!-- оружие -->" in text and '<!--<type name="Old"><nominal>1</nominal></type>-->' in text
    assert "<nominal>6</nominal>" in text and "<nominal>30</nominal>" in text
    assert '<usage name="Town"/>' in text and '<usage user="Custom"/>' in text
    tf2 = te.load(p)
    akm, apple, empty = tf2.rows
    assert akm.lists["usage"] == ["Military", "Town"] and akm.user["usage"] == ["Custom"]
    assert akm.values["count_in_player"] == "1" and empty.values["count_in_player"] == "1"
    assert akm.values["lifetime"] == "7200" and "cost" not in apple.values
    # строки, которые не менялись, остались как были
    assert text.count("\n") >= TYPES.count("\n")
    apple.values["min"] = "50"
    assert "min больше nominal" in te.validate(apple)


def test_types_editor_cli(tmp_path):
    p = tmp_path / "types.xml"
    p.write_text(TYPES, encoding="utf-8")
    r = run_cli("typesedit", p, "--category", "weapons", "--mul", "nominal=2", "--add-list", "tag=floor")
    assert r.returncode == 0, r.stderr
    tf = te.load(p)
    assert tf.rows[0].values["nominal"] == "8" and tf.rows[0].lists["tag"] == ["floor"]
    assert tf.rows[1].values["nominal"] == "20"
    r = run_cli("typesedit", p, "--set", "bogus=1")
    assert r.returncode == 1


# ---------------------------------------------------------------- обновления

def test_update_check():
    assert update.is_newer("3.1.0", "3.0.9") and update.is_newer("3.0.10", "3.0.9")
    assert not update.is_newer("3.0.0", "3.0.0") and not update.is_newer("2.9", "3.0.0")
    info = update.from_release({"name": "DayZ Mod Toolkit 9.9.9 (Windows)", "html_url": "https://x",
                                "assets": [{"name": "DayZModToolkit.exe", "browser_download_url": "https://x/e"}]})
    assert info.version == "9.9.9" and info.newer and info.download == "https://x/e"
    assert update.from_release({"name": "DayZ Mod Toolkit (Windows)", "body": ""}) is None
    assert not update.due({"check_updates": False})
    assert update.due({"update_checked": 0}) and not update.due({"update_checked": time.time()})
