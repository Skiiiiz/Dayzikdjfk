# -*- coding: utf-8 -*-
"""Автотесты DayZ Mod Toolkit. Реальные файлы берутся из мода SM_PartyMod в этом репозитории."""

import shutil
import subprocess
import sys
from pathlib import Path

import numpy as np
import pytest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))

from dztk import audio, cfg, checks, convert, enforce, paa, rap  # noqa: E402
from dztk.common import find_ffmpeg  # noqa: E402

MOD = ROOT.parent / "SM_PartyMod"
needs_mod = pytest.mark.skipif(not MOD.is_dir(), reason="нет SM_PartyMod")


def codes(issues, level=None):
    return [i.code for i in issues if level is None or i.level == level]


# ---------------------------------------------------------------- PAA

@needs_mod
def test_read_all_real_paa():
    files = sorted(MOD.rglob("*.paa"))
    assert len(files) >= 40
    for f in files:
        info = paa.parse(f.read_bytes(), load_all=True)       # включая LZO-сжатые mip-уровни
        for m in info.mipmaps:
            img = paa.decode_mip(info, m)
            assert img.shape == (m.height, m.width, 4)


@needs_mod
def test_paa_roundtrip_quality():
    img, info = paa.read_paa(MOD / "GUI/logo/logo.paa")
    data, fmt = paa.encode_paa(img, "auto")
    assert fmt == "DXT5"
    info2 = paa.parse(data, load_all=True)
    assert (info2.width, info2.height) == (info.width, info.height)
    assert info2.mipmaps[-1].width == 4                     # цепочка до 4x4
    dec = paa.decode_mip(info2, info2.mipmaps[0])
    mse = ((dec[..., :3].astype(float) - img[..., :3]) ** 2).mean()
    assert 10 * np.log10(255 ** 2 / mse) > 35
    assert np.abs(dec[..., 3].astype(int) - img[..., 3]).mean() < 2


def test_paa_dxt1_alpha_and_pow2(tmp_path):
    from PIL import Image
    img = np.zeros((64, 128, 4), np.uint8)
    img[..., 0] = 200
    img[..., 3] = 255
    img[:, :32, 3] = 0                                      # прозрачная полоса
    Image.fromarray(img).save(tmp_path / "a.png")
    w, h, fmt = paa.image_to_paa(tmp_path / "a.png", tmp_path / "a.paa", "dxt1")
    rgba, info = paa.read_paa(tmp_path / "a.paa")
    assert info.type_name == "DXT1" and (w, h) == (128, 64)
    assert rgba[:, :32, 3].max() == 0 and rgba[:, 40:, 3].min() == 255
    assert info.tags["FLAG"] == b"\x02\x00\x00\x00"

    Image.fromarray(np.zeros((100, 60, 4), np.uint8)).save(tmp_path / "b.png")
    with pytest.raises(paa.PAAError):
        paa.image_to_paa(tmp_path / "b.png", tmp_path / "b.paa")
    w, h, _ = paa.image_to_paa(tmp_path / "b.png", tmp_path / "b.paa", resize="nearest")
    assert paa.is_pow2(w) and paa.is_pow2(h)


def test_suffix_format():
    assert paa.fmt_from_suffix("wall_co") == "dxt1"
    assert paa.fmt_from_suffix("glass_ca") == "dxt5"
    assert paa.fmt_from_suffix("icon") is None


# ---------------------------------------------------------------- config.bin / config.cpp

@needs_mod
def test_rap_roundtrip_is_byte_identical(tmp_path):
    data = (MOD / "config.bin").read_bytes()
    root = rap.read_rap(data)
    text = cfg.to_text(root)
    (tmp_path / "config.cpp").write_text(text, encoding="utf-8")
    root2, issues = cfg.parse_config(tmp_path / "config.cpp")
    assert rap.write_rap(root2) == data
    assert not [i for i in issues + cfg.semantic_check(root2) if i.level == "error"]


def parse_text(tmp_path, text):
    p = tmp_path / "config.cpp"
    p.write_text(text, encoding="utf-8")
    return cfg.parse_config(p)


@pytest.mark.parametrize("text, fragment", [
    ("class A\n{\n\tx = 1\n\ty = 2;\n};\n", "';'"),
    ("class A\n{\n\tx = 1;\n}\nclass B {};\n", "};"),
    ("class A\n{\n\tx = 1;\n", "конец файла"),
    ("class A { arr = {1,2}; };\n", "arr[]"),
    ("class A { s = \"abc; };\n", "незакрытая строка"),
    ("class A { x[] = {1, 2 3}; };\n", "','"),
    ("/* comment\nclass A {};\n", "комментарий"),
    ("#ifdef X\nclass A {};\n", "#endif"),
])
def test_cfg_syntax_errors(tmp_path, text, fragment):
    with pytest.raises(cfg.ConfigError) as e:
        parse_text(tmp_path, text)
    assert fragment in str(e.value)
    assert e.value.issue.line > 0


def test_cfg_semantics(tmp_path):
    text = """
class CfgPatches { class M { units[] = {}; weapons[] = {}; requiredAddons[] = {"DZ_Data"}; }; };
class CfgVehicles
{
    class Inventory_Base;
    class Car_Base;
    class Ok: Inventory_Base { scope = 2; scope = 1; };
    class Bad: Clothing_Base {};
    class Ok {};
    class MyCar: Car_Base { class Turrets: Turrets {}; };
    class Child: Ok { class Inner: Inner {}; };
};
"""
    root, _ = parse_text(tmp_path, text)
    iss = cfg.semantic_check(root)
    msgs = [i.message for i in iss if i.code == "undefined-base"]
    assert any("Clothing_Base" in m for m in msgs)
    assert any("'Inner'" in m for m in msgs)
    assert not any("Turrets" in m for m in msgs)            # база внешняя — это не ошибка
    assert "duplicate-class" in codes(iss)
    assert "duplicate-property" in codes(iss)


def test_cfg_preprocessor(tmp_path):
    (tmp_path / "inc.hpp").write_text("class Included { v = VALUE; };\n", encoding="utf-8")
    text = '#define VALUE 42\n#define QUOTE(x) #x\n#include "inc.hpp"\nclass A { p = QUOTE(MyMod\\data); };\n'
    root, issues = parse_text(tmp_path, text)
    inc = next(e for e in root.entries if e.name == "Included")
    assert inc.entries[0].value == 42
    a = next(e for e in root.entries if e.name == "A")
    assert a.entries[0].value == "MyMod\\data"


# ---------------------------------------------------------------- Enforce Script

@needs_mod
def test_scripts_no_false_positives():
    files = sorted(MOD.rglob("*.c"))
    assert files
    for f in files:
        assert enforce.check_script(f) == [], f


def test_script_errors():
    src = """class A extends B
{
    int m_X
    void F(int a)
    {
        if (a = 1)
            Print("x");
        while (a > 0);
        Foo(a)
    }
}
enum E
{
    ONE,
    TWO
    THREE
}
"""
    iss = enforce.check_script("a.c", src)
    got = {(i.line, i.code) for i in iss}
    assert (3, "missing-semicolon") in got
    assert (6, "assign-in-condition") in got
    assert (8, "empty-body") in got
    assert (9, "missing-semicolon") in got
    assert (15, "missing-comma") in got


def test_script_brackets():
    iss = enforce.check_script("c.c", "class A\n{\n    void F()\n    {\n        Print((1);\n    }\n}\n")
    assert iss and iss[0].code == "brackets" and "строке 5" in iss[0].message
    iss = enforce.check_script("d.c", "class A\n{\n    void F() {\n}\n")
    assert any(i.code == "brackets" and i.line == 2 for i in iss)


def test_script_valid_constructs():
    src = """[CfgMod("x")]
class A
{
    ref array<string> m_List = {"a", "b",
        "c"};
    void F()
    {
        if (m_List)
            Print("ok");
        else
            Print("no");
        int x = 1 +
            2;
        string s = "a" + "b"
            + "c";
        m_List
            .Insert("d");
        switch (x) { case 1: Print(1); break; default: break; }
    }
}
"""
    assert enforce.check_script("b.c", src) == []


# ---------------------------------------------------------------- прочие файлы

def test_types_xml(tmp_path):
    p = tmp_path / "types.xml"
    p.write_text("""<?xml version="1.0" encoding="UTF-8"?>
<types>
    <type name="AKM"><nominal>5</nominal><lifetime>3600</lifetime><min>10</min>
        <quantmin>-1</quantmin><quantmax>-1</quantmax><flags count_in_cargo="2"/><usage name="Military"/></type>
    <type name="AKM"><nominal>x</nominal></type>
</types>""", encoding="utf-8")
    c = [i.message for i in checks.check_xml(p)]
    assert any("min (10) больше nominal" in m for m in c)
    assert any("count_in_cargo" in m for m in c)
    assert any("повторно" in m for m in c)
    assert any("целым числом" in m for m in c)


def test_bad_xml_json_csv(tmp_path):
    (tmp_path / "a.xml").write_text("<a><b></a>", encoding="utf-8")
    i = checks.check_xml(tmp_path / "a.xml")
    assert i[0].level == "error" and i[0].line == 1
    (tmp_path / "a.json").write_text('{\n "a": 1,\n}\n', encoding="utf-8")
    i = checks.check_json(tmp_path / "a.json")
    assert i[0].level == "error" and i[0].line == 3
    (tmp_path / "stringtable.csv").write_text(
        '"Language","original","english"\n"STR_A","a","a"\n"STR_A","b"\n', encoding="utf-8")
    c = [x.message for x in checks.check_stringtable(tmp_path / "stringtable.csv")]
    assert any("колонок 2" in m for m in c) and any("повторяется" in m for m in c)


@needs_mod
def test_clean_mod_has_no_problems():
    rep = checks.check_paths([str(MOD)])
    assert rep.files > 50
    assert [i.format() for i in rep.issues if i.level in ("error", "warning")] == []


@needs_mod
def test_broken_mod_detected(tmp_path):
    mod = tmp_path / "SM_PartyMod"
    shutil.copytree(MOD, mod)
    (mod / "GUI/icons/nav_tops.paa").unlink()
    shutil.rmtree(mod / "Scripts/4_World")
    loc = mod / "Scripts/3_Game/SM_PartyLoc.c"
    loc.write_text(loc.read_text(encoding="utf-8").replace("KEY_LENGTH = 14;", "KEY_LENGTH = 14", 1),
                   encoding="utf-8")
    st = mod / "stringtable.csv"
    st.write_text(st.read_text(encoding="utf-8").replace('"STR_SMP_00756"', '"STR_SMP_X0756"'), encoding="utf-8")
    rep = checks.check_paths([str(mod)])
    got = codes(rep.issues)
    assert "missing-semicolon" in got
    assert "missing-dir" in got
    assert "missing-string" in got
    assert any(i.code == "missing-file" and "nav_tops.paa" in i.message for i in rep.issues)


# ---------------------------------------------------------------- звук

ffmpeg = find_ffmpeg()
needs_ffmpeg = pytest.mark.skipif(not ffmpeg, reason="нет ffmpeg")


@needs_ffmpeg
def test_audio_and_ogg_check(tmp_path):
    src = tmp_path / "Тест Звук.mp3"
    subprocess.run([ffmpeg, "-loglevel", "error", "-f", "lavfi", "-i", "sine=f=440:d=1", "-ac", "2", str(src)],
                   check=True)
    s = audio.ConvertSettings(output_dir=str(tmp_path / "out"), mod_name="T", sound_path="T\\sounds")
    res = audio.convert_all(audio.plan_jobs(audio.collect_inputs([str(src)]), s), s, ffmpeg)
    assert res[0].ok and res[0].job.dst.name == "test_zvuk.ogg"
    assert checks.check_ogg(res[0].job.dst, None, ffmpeg) == []
    fake = tmp_path / "fake.ogg"
    shutil.copy(src, fake)
    assert checks.check_ogg(fake, None, ffmpeg)[0].level == "error"


# ---------------------------------------------------------------- CLI

@needs_mod
def test_cli_commands(tmp_path):
    exe = [sys.executable, str(ROOT / "dayz_toolkit.py"), "--cli"]
    r = subprocess.run(exe + ["derapify", str(MOD / "config.bin"), "-o", str(tmp_path)], capture_output=True)
    assert r.returncode == 0
    r = subprocess.run(exe + ["rapify", str(tmp_path / "config.cpp"), "-o", str(tmp_path / "b")], capture_output=True)
    assert r.returncode == 0
    assert (tmp_path / "b/config.bin").read_bytes() == (MOD / "config.bin").read_bytes()
    r = subprocess.run(exe + ["png", str(MOD / "GUI/pings"), "-o", str(tmp_path / "png")], capture_output=True)
    assert r.returncode == 0 and len(list((tmp_path / "png").glob("*.png"))) == 15
    r = subprocess.run(exe + ["paa", str(tmp_path / "png"), "-o", str(tmp_path / "paa")], capture_output=True)
    assert r.returncode == 0 and len(list((tmp_path / "paa").glob("*.paa"))) == 15
    r = subprocess.run(exe + ["check", str(MOD)], capture_output=True)
    assert r.returncode == 0
