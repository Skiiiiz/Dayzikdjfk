# -*- coding: utf-8 -*-
"""Генерация записей types.xml из config.cpp/config.bin мода, слияние и сортировка types.xml.

Берутся классы со scope = 2 из CfgVehicles, CfgWeapons и CfgMagazines (scope ищется и по
цепочке наследования внутри файла). Категория и стартовые значения подбираются по имени класса
и его базовым классам — это заготовка, значения стоит подогнать под свой сервер.
"""

from __future__ import annotations

from dztk.i18n import tr

import re
import xml.etree.ElementTree as ET
from dataclasses import dataclass, field
from pathlib import Path
from typing import Dict, List, Tuple

from . import cfg, rap

SECTIONS = ("cfgvehicles", "cfgweapons", "cfgmagazines")

PRESETS: Dict[str, dict] = {
    "weapons":    dict(nominal=3, min=1, lifetime=28800, restock=1800, usage=["Military"]),
    "magazines":  dict(nominal=5, min=2, lifetime=28800, restock=1800, usage=["Military"], category="weapons"),
    "clothes":    dict(nominal=10, min=5, lifetime=28800, restock=0, usage=["Town", "Village"]),
    "food":       dict(nominal=10, min=5, lifetime=14400, restock=0, usage=["Town", "Village"]),
    "tools":      dict(nominal=8, min=4, lifetime=14400, restock=0, usage=["Industrial", "Town"]),
    "containers": dict(nominal=5, min=2, lifetime=3888000, restock=0, usage=["Village", "Farm"]),
    "explosives": dict(nominal=2, min=1, lifetime=14400, restock=1800, usage=["Military"]),
}

RULES: List[Tuple[str, Tuple[str, ...]]] = [
    ("explosives", ("grenade", "explosive", "claymore", "landmine", "plastic_explosive", "bomb")),
    ("clothes", ("clothing", "_top", "_pants", "_shoes", "boots", "_vest", "helmet", "_gloves", "_mask",
                 "_hat", "_cap", "jacket", "shirt", "backpack", "_bag", "armband", "belt", "glasses")),
    ("food", ("edible", "food", "drink", "canned", "_can", "bottle", "fruit", "meat", "_soda")),
    ("containers", ("container", "tent", "barrel", "crate", "_case", "chest", "storage", "_box")),
    ("tools", ("tool", "knife", "axe", "saw", "hammer", "shovel", "pliers", "wrench", "light", "radio")),
]


@dataclass
class Item:
    name: str
    section: str
    chain: List[str] = field(default_factory=list)
    category: str = "tools"

    def preset(self) -> dict:
        return PRESETS.get(self.category, PRESETS["tools"])


def _load_root(path: Path) -> cfg.ClassNode:
    data = path.read_bytes()
    if rap.is_rapified(data):
        return rap.read_rap(data)
    root, _ = cfg.parse_config(path)
    return root


def _value(node: cfg.ClassNode, name: str):
    for e in node.entries:
        if isinstance(e, cfg.ValueNode) and e.name.lower() == name:
            return e.value
    return None


def items_from_config(root: cfg.ClassNode) -> List[Item]:
    items: List[Item] = []
    for sec in root.entries:
        if not (isinstance(sec, cfg.ClassNode) and sec.name.lower() in SECTIONS and not sec.extern):
            continue
        by_name = {c.name.lower(): c for c in sec.entries if isinstance(c, cfg.ClassNode)}
        for c in sec.entries:
            if not isinstance(c, cfg.ClassNode) or c.extern:
                continue
            chain, scope, cur, seen = [c.name], None, c, set()
            while cur is not None and cur.name.lower() not in seen:
                seen.add(cur.name.lower())
                if scope is None:
                    scope = _value(cur, "scope")
                if not cur.base:
                    break
                chain.append(cur.base)
                cur = by_name.get(cur.base.lower())
            try:
                if int(float(scope)) != 2:
                    continue
            except (TypeError, ValueError):
                continue
            it = Item(c.name, sec.name, chain)
            it.category = guess_category(it)
            items.append(it)
    return items


def guess_category(it: Item) -> str:
    sec = it.section.lower()
    if sec == "cfgmagazines":
        return "magazines"
    if sec == "cfgweapons":
        return "weapons"
    text = " ".join(x.lower() for x in it.chain)
    if any(k in text for k in ("rifle", "pistol", "weapon_base", "shotgun", "_smg")):
        return "weapons"
    for cat, keys in RULES:
        if any(k in text for k in keys):
            return cat
    return "tools"


def type_xml(it: Item, indent: str = "    ") -> str:
    p = it.preset()
    cat = p.get("category", it.category)
    lines = [f'{indent}<type name="{it.name}">',
             f"{indent * 2}<nominal>{p['nominal']}</nominal>",
             f"{indent * 2}<lifetime>{p['lifetime']}</lifetime>",
             f"{indent * 2}<restock>{p['restock']}</restock>",
             f"{indent * 2}<min>{p['min']}</min>",
             f"{indent * 2}<quantmin>-1</quantmin>",
             f"{indent * 2}<quantmax>-1</quantmax>",
             f"{indent * 2}<cost>100</cost>",
             f'{indent * 2}<flags count_in_cargo="0" count_in_hoarder="0" count_in_map="1" count_in_player="0" '
             f'crafted="0" deloot="0"/>',
             f'{indent * 2}<category name="{cat}"/>']
    lines += [f'{indent * 2}<usage name="{u}"/>' for u in p["usage"]]
    lines.append(f"{indent}</type>")
    return "\n".join(lines)


def generate(items: List[Item]) -> str:
    body = "\n".join(type_xml(i) for i in items)
    return (tr("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\" ?>\n<!-- Сгенерировано DayZ Mod Toolkit: это заготовка, проверьте nominal/min/lifetime/usage -->\n<types>\n") + body + ("\n" if body else "") + "</types>\n")


def existing_names(text: str) -> set:
    return {m.group(1).lower() for m in re.finditer(r'<type\s+name\s*=\s*"([^"]+)"', text)}


def merge(existing_text: str, items: List[Item]) -> Tuple[str, int]:
    """Добавляет в существующий types.xml недостающие типы (без переформатирования остального)."""
    have = existing_names(existing_text)
    new = [i for i in items if i.name.lower() not in have]
    if not new:
        return existing_text, 0
    pos = existing_text.rfind("</types>")
    if pos < 0:
        raise ValueError(tr("в файле нет закрывающего </types>"))
    block = tr("    <!-- добавлено DayZ Mod Toolkit -->\n") + "\n".join(type_xml(i) for i in new) + "\n"
    return existing_text[:pos] + block + existing_text[pos:], len(new)


def sort_types(text: str) -> str:
    """Сортирует <type> по имени (комментарии внутри <types> не сохраняются)."""
    root = ET.fromstring(text.encode("utf-8") if text.lstrip().startswith("<?xml") else text)
    types = sorted(root.findall("type"), key=lambda t: (t.get("name") or "").lower())
    for t in list(root):
        root.remove(t)
    out = ['<?xml version="1.0" encoding="UTF-8" standalone="yes" ?>', "<types>"]
    for t in types:
        out.append(f'    <type name="{t.get("name")}">')
        for ch in t:
            attrs = "".join(f' {k}="{v}"' for k, v in ch.attrib.items())
            if (ch.text or "").strip():
                out.append(f"        <{ch.tag}{attrs}>{ch.text.strip()}</{ch.tag}>")
            else:
                out.append(f"        <{ch.tag}{attrs}/>")
        out.append("    </type>")
    out.append("</types>")
    return "\n".join(out) + "\n"


def from_paths(paths: List[str]) -> List[Item]:
    """Собирает предметы из файлов config.cpp/config.bin или папок модов."""
    items: List[Item] = []
    seen = set()
    for raw in paths:
        p = Path(raw)
        files = [f for f in p.rglob("*") if f.name.lower() in ("config.cpp", "config.bin")] if p.is_dir() else [p]
        # если в папке есть и cpp, и bin — берём cpp
        cpps = {f.parent for f in files if f.name.lower() == "config.cpp"}
        for f in sorted(files):
            if f.name.lower() == "config.bin" and f.parent in cpps:
                continue
            for it in items_from_config(_load_root(f)):
                if it.name.lower() not in seen:
                    seen.add(it.name.lower())
                    items.append(it)
    return items
