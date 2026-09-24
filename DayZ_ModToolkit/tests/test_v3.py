# -*- coding: utf-8 -*-
"""Тесты функций версии 3: LZO, PBO, подписи, types.xml, проверки по игре, перевод."""

import ast
import os
import random
import shutil
import subprocess
import sys
from pathlib import Path

import pytest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))

from dztk import checks, lzo, paa, pbo, sign, typesgen, vanilla  # noqa: E402
from dztk.i18n_en import EN  # noqa: E402

MOD = ROOT.parent / "SM_PartyMod"
needs_mod = pytest.mark.skipif(not MOD.is_dir(), reason="нет SM_PartyMod")


# ---------------------------------------------------------------- LZO

def _samples():
    rnd = random.Random(7)
    yield b""
    yield b"a"
    yield b"abcabcabcabc" * 3
    yield bytes(rnd.getrandbits(8) for _ in range(5000))
    yield bytes(rnd.choice(b"ab") for _ in range(70000))
    yield bytes(120000)
    yield b"hello world " * 5000


_SAMPLES = list(_samples())


@pytest.mark.parametrize("data", _SAMPLES, ids=[f"sample{i}_{len(d)}b" for i, d in enumerate(_SAMPLES)])
def test_lzo_roundtrip(data):
    c = lzo.compress(data)
    assert lzo.decompress(c, len(data))[0] == data
    try:
        import lzo as ref  # python-lzo (эталонная liblzo2), если установлен
    except ImportError:
        return
    if data:
        assert ref.decompress(c, False, len(data)) == data


@needs_mod
def test_paa_lzo_mips():
    img, _ = paa.read_paa(MOD / "GUI/logo/logo.paa")          # 512x512
    data, _ = paa.encode_paa(img)
    info = paa.parse(data, load_all=True)
    assert [m.lzo for m in info.mipmaps[:3]] == [True, True, False]
    raw, _ = paa.encode_paa(img, compress=False)
    assert len(data) < len(raw) / 3
    for m in info.mipmaps:
        paa.decode_mip(info, m)


# ---------------------------------------------------------------- PBO и подписи

@needs_mod
def test_pbo_pack_unpack_roundtrip(tmp_path):
    opts = pbo.BuildOptions(check=False, mod_layout=True)
    res = pbo.build(MOD, tmp_path, opts)
    assert res.ok and res.pbo.name == "SM_PartyMod.pbo"
    assert pbo.verify_checksum(res.pbo) is True
    arc = pbo.read_pbo(res.pbo)
    assert arc.prefix == "SM_PartyMod"
    out, _ = pbo.unpack(res.pbo, tmp_path / "unp")
    for rel, src in pbo.collect_files(MOD, opts)[0]:
        assert (out / rel).read_bytes() == src.read_bytes(), rel
    assert (out / "config.cpp").is_file()          # распакован из config.bin
    assert (out / "$PBOPREFIX$").read_text().strip() == "SM_PartyMod"


def test_pbo_rapify_and_excludes(tmp_path):
    src = tmp_path / "MyMod"
    (src / "data").mkdir(parents=True)
    (src / "config.cpp").write_text('class CfgPatches { class MyMod { units[] = {}; weapons[] = {}; '
                                    'requiredAddons[] = {}; }; };\n', encoding="utf-8")
    (src / "data" / "a.txt").write_text("x", encoding="utf-8")
    (src / "secret.biprivatekey").write_bytes(b"x")
    (src / "art.psd").write_bytes(b"x")
    res = pbo.build(src, tmp_path / "out", pbo.BuildOptions(check=True))
    assert res.ok
    names = {e.name for e in pbo.read_pbo(res.pbo).entries}
    assert names == {"config.bin", "data\\a.txt"}


def test_pbo_build_stops_on_errors(tmp_path):
    src = tmp_path / "Bad"
    src.mkdir()
    (src / "config.cpp").write_text("class CfgVehicles { class A: Missing_Base {}; };\n", encoding="utf-8")
    res = pbo.build(src, tmp_path / "out", pbo.BuildOptions())
    assert not res.ok and not (tmp_path / "out").exists()


def test_keys_sign_verify_and_tamper(tmp_path):
    priv, pub = sign.create_keys("TestKey", tmp_path)
    key = sign.PrivateKey.load(priv)
    assert key.to_bytes() == priv.read_bytes()
    assert key.public().to_bytes() == pub.read_bytes()
    assert key.p * key.q == key.n and key.bits == 1024
    with pytest.raises(sign.SignError):
        sign.create_keys("TestKey", tmp_path)               # закрытый ключ не перезаписывается

    files = [("config.cpp", b"class A {};", 0), ("scripts\\x.hpp", b"#define X 1", 0), ("data\\t.paa", b"P" * 50, 0)]
    target = tmp_path / "t.pbo"
    pbo.write_pbo(target, files, {"prefix": "MyMod"})
    sig = sign.sign_pbo(target, key)
    assert sig.name == "t.pbo.TestKey.bisign"
    assert sign.verify_pbo(target, sig, pub)[0]

    other_priv, other_pub = sign.create_keys("Other", tmp_path / "o")
    assert not sign.verify_pbo(target, sig, other_pub)[0]   # чужой ключ

    files[1] = ("scripts\\x.hpp", b"#define X 2", 0)          # подмена содержимого
    pbo.write_pbo(target, files, {"prefix": "MyMod"})
    ok, msg = sign.verify_pbo(target, sig, pub)
    assert not ok and "hash1" in msg


FIXTURES = os.environ.get("HEMTT_SIGNING_TESTS", "")


@pytest.mark.skipif(not FIXTURES, reason="нет эталонных файлов HEMTT (переменная HEMTT_SIGNING_TESTS)")
def test_signature_matches_reference(tmp_path):
    """Подпись должна совпадать байт в байт с эталоном (libs/signing/tests/ace_ai_3.15.2.69 из HEMTT)."""
    d = Path(FIXTURES)
    shutil.copy(d / "source.pbo", tmp_path / "source.pbo")
    key = sign.PrivateKey.load(d / "test.biprivatekey")
    out = sign.sign_pbo(tmp_path / "source.pbo", key, 3)
    assert out.read_bytes() == (d / "source.pbo.test.bisign").read_bytes()


# ---------------------------------------------------------------- types.xml

def test_types_generation(tmp_path):
    cfgp = tmp_path / "config.cpp"
    cfgp.write_text("""
class CfgVehicles {
  class Clothing; class Inventory_Base; class Edible_Base;
  class MyJacket_Base: Clothing { scope = 0; };
  class MyJacket_Red: MyJacket_Base { scope = 2; };
  class MyFood: Edible_Base { scope = 2; };
  class Hidden: Inventory_Base { scope = 1; };
};
class CfgWeapons { class Rifle_Base; class MyRifle: Rifle_Base { scope = 2; }; };
class CfgMagazines { class Magazine_Base; class Mag_My_30Rnd: Magazine_Base { scope = 2; }; };
""", encoding="utf-8")
    items = typesgen.from_paths([str(cfgp)])
    assert {(i.name, i.category) for i in items} == {("MyJacket_Red", "clothes"), ("MyFood", "food"),
                                                    ("MyRifle", "weapons"), ("Mag_My_30Rnd", "magazines")}
    out = tmp_path / "types.xml"
    out.write_text(typesgen.generate(items), encoding="utf-8")
    assert [i for i in checks.check_xml(out) if i.level != "info"] == []
    merged, n = typesgen.merge('<types>\n    <type name="MyFood"><nominal>1</nominal></type>\n</types>\n', items)
    assert n == 3 and merged.count("<type ") == 4
    s = typesgen.sort_types(merged)
    names = [l.split('"')[1] for l in s.splitlines() if "<type " in l]
    assert names == sorted(names, key=str.lower)


# ---------------------------------------------------------------- проверки по игре

GAME = {
    "enscript.c": "class Class\n{\n\tstring GetDebugName() { return \"\"; }\n}\nclass Managed {}\n",
    "entity.c": """class EntityAI: Managed
{
	void EEInit() {}
	bool CanPutInCargo(EntityAI parent) { return true; }
	proto native void SetHealth(float v);
}
typedef array<string> TStringArray;
#ifdef FEATURE_X
class Man extends Person
#else
class Man extends EntityAI
#endif
{
	void OnManInit() {}
}
class Person: EntityAI { void ObtainState() {} }
class Inventory_Base extends EntityAI {}
""",
}


def test_game_index_checks(tmp_path):
    game_dir = tmp_path / "game"
    game_dir.mkdir()
    for n, t in GAME.items():
        (game_dir / n).write_text(t, encoding="utf-8")
    game = vanilla.build_index([str(game_dir)])
    assert game.classes["man"].base == "Person|EntityAI"
    mod = tmp_path / "mod.c"
    mod.write_text("""modded class Man
{
	override void EEInit() {}
	override void ObtainState() {}
	override void OnManInit() {}
	override void Typo() {}
}
modded class NoSuchClass {}
class MyItem extends Inventory_Base
{
	override bool CanPutInCargo(EntityAI parent) { return false; }
	override string GetDebugName() { return "x"; }
	override void EEInitt() {}
}
class MyList extends TStringArray {}
class Broken extends Inventory_Basee {}
""", encoding="utf-8")
    iss = vanilla.check_mod_scripts([mod], game)
    got = sorted((i.code, i.line) for i in iss)
    assert got == [("bad-override", 6), ("bad-override", 13), ("unknown-base", 16), ("unknown-class", 8)]


DAYZ_SCRIPTS = os.environ.get("DAYZ_SCRIPTS", "")


@pytest.mark.skipif(not DAYZ_SCRIPTS, reason="нет ванильных скриптов (переменная DAYZ_SCRIPTS)")
def test_vanilla_scripts_no_false_positives():
    """Скрипты игры (github.com/BohemiaInteractive/DayZ-Script-Diff) не должны давать ошибок."""
    files = sorted(Path(DAYZ_SCRIPTS).rglob("*.c"))
    from dztk import enforce
    errors = [i.format() for f in files for i in enforce.check_script(f) if i.level != "info"]
    assert errors == []
    assert vanilla.check_mod_scripts(files, vanilla.Index()) == []
    if MOD.is_dir():
        game = vanilla.build_index([DAYZ_SCRIPTS])
        assert vanilla.check_mod_scripts(sorted(MOD.rglob("*.c")), game) == []


# ---------------------------------------------------------------- перевод

def test_every_string_is_translated():
    missing = []
    for f in list((ROOT / "dztk").glob("*.py")) + [ROOT / "dayz_toolkit.py"]:
        for n in ast.walk(ast.parse(f.read_text(encoding="utf-8"))):
            if (isinstance(n, ast.Call) and getattr(n.func, "id", "") == "tr" and n.args
                    and isinstance(n.args[0], ast.Constant) and n.args[0].value not in EN):
                missing.append(f"{f.name}: {n.args[0].value[:60]}")
    assert missing == []


@needs_mod
def test_english_cli():
    env = dict(os.environ, DZTK_LANG="en")
    r = subprocess.run([sys.executable, str(ROOT / "dayz_toolkit.py"), "--cli", "check", str(MOD)],
                       capture_output=True, env=env)
    out = r.stdout.decode("utf-8", "replace")
    assert r.returncode == 0 and "Files checked" in out
    r = subprocess.run([sys.executable, str(ROOT / "dayz_toolkit.py"), "--lang", "ru", "--cli", "check", str(MOD)],
                       capture_output=True, env=env)
    assert "Проверено файлов" in r.stdout.decode("utf-8", "replace")
