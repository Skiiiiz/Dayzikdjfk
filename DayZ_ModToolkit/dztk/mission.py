# -*- coding: utf-8 -*-
"""Перекрёстная проверка папки миссии сервера (Central Economy).

Отдельные XML-файлы проверяет checks.py; здесь — связи между ними:
  * types.xml: category/usage/value/tag (и user-наборы) объявлены в cfglimitsdefinition(user).xml;
  * cfgeconomycore.xml: папки и файлы <ce> существуют, их types/events/spawnabletypes учитываются;
  * один и тот же тип не описан в нескольких файлах types;
  * events.xml: дочерние типы событий есть в types.xml;
  * cfgeventspawns.xml: группы есть в cfgeventgroups.xml, события есть в events.xml;
  * cfgspawnabletypes.xml: пресеты есть в cfgrandompresets.xml, типы и предметы есть в types.xml;
  * cfggameplay.json: указанные файлы существуют.
Правила подобраны так, чтобы официальные миссии Bohemia (DayZ-Central-Economy) проходили без ошибок.
"""

from __future__ import annotations

from .i18n import tr

import json
import re
import xml.etree.ElementTree as ET
from pathlib import Path
from typing import Dict, List, Optional, Set, Tuple

from .cfg import Issue

LIMIT_SECTIONS = {"category": "categories", "tag": "tags", "usage": "usageflags", "value": "valueflags"}


def is_mission_dir(path: Path) -> bool:
    return (path / "cfgeconomycore.xml").is_file() and (path / "db").is_dir()


def _parse(path: Path) -> Tuple[Optional[ET.Element], str]:
    try:
        text = path.read_bytes().decode("utf-8-sig", "replace")
        root = ET.fromstring(text.encode("utf-8") if text.lstrip().startswith("<?xml") else text)
        return root, text
    except (OSError, ET.ParseError):
        return None, ""   # синтаксические ошибки уже сообщает checks.check_xml


def _line_of(text: str, tag: str, name: str) -> int:
    m = re.search(r"<%s\s+name\s*=\s*\"%s\"" % (re.escape(tag), re.escape(name)), text)
    return text.count("\n", 0, m.start()) + 1 if m else 0


def _find(root: Path, name: str) -> Optional[Path]:
    """Файл миссии без учёта регистра имени."""
    p = root / name
    if p.is_file():
        return p
    low = name.lower()
    for f in root.iterdir():
        if f.name.lower() == low and f.is_file():
            return f
    return None


def check_mission(root: Path) -> List[Issue]:
    root = Path(root)
    issues: List[Issue] = []

    def add(level, msg, file, line=0, code="mission"):
        issues.append(Issue(level, msg, str(file), line, 0, code))

    # --- набор файлов (db + <ce>) ---
    files: Dict[str, List[Path]] = {"types": [], "events": [], "spawnabletypes": []}
    for kind, rel in (("types", "db/types.xml"), ("events", "db/events.xml"),
                      ("spawnabletypes", "cfgspawnabletypes.xml")):
        p = root / rel
        if p.is_file():
            files[kind].append(p)
    core_path = root / "cfgeconomycore.xml"
    core, core_text = _parse(core_path)
    if core is not None:
        for ce in core.findall("ce"):
            folder = ce.get("folder") or ""
            d = root / folder
            if not d.is_dir():
                add("error", tr("cfgeconomycore.xml: папка <ce folder=\"{0}\"> не найдена").format(folder), core_path,
                    _line_of(core_text, "ce folder", folder) or 0, "mission-ce")
                continue
            for f in ce.findall("file"):
                fp = d / (f.get("name") or "")
                ftype = (f.get("type") or "").lower()
                if not fp.is_file():
                    add("error", tr("cfgeconomycore.xml: файл {0}/{1} не найден").format(folder, f.get("name")),
                        core_path, 0, "mission-ce")
                elif ftype in files:
                    files[ftype].append(fp)

    # --- справочники cfglimitsdefinition ---
    limits: Dict[str, Set[str]] = {}
    users: Dict[str, Set[str]] = {}
    lim_path = _find(root, "cfglimitsdefinition.xml")
    if lim_path:
        lim, _ = _parse(lim_path)
        if lim is not None:
            for tag, sec in LIMIT_SECTIONS.items():
                node = lim.find(sec)
                if node is not None:
                    limits[sec] = {x.get("name") for x in node if x.get("name")}
    user_path = _find(root, "cfglimitsdefinitionuser.xml")
    if user_path:
        usr, _ = _parse(user_path)
        if usr is not None:
            for sec in ("usageflags", "valueflags"):
                node = usr.find(sec)
                if node is not None:
                    users[sec] = {u.get("name") for u in node if u.get("name")}

    # --- types ---
    type_names: Set[str] = set()
    type_origin: Dict[str, Tuple[Path, int]] = {}
    for tp in files["types"]:
        troot, text = _parse(tp)
        if troot is None:
            continue
        for t in troot.findall("type"):
            name = t.get("name") or ""
            key = name.lower()
            line = _line_of(text, "type", name)
            if key in type_origin and type_origin[key][0] != tp:
                prev, pline = type_origin[key]
                add("warning", tr("тип {0} описан и в {1} (строка {2}) — будет действовать одно из описаний").format(
                    name, prev.name, pline), tp, line, "mission-dup-type")
            type_origin.setdefault(key, (tp, line))
            type_names.add(key)
            if not limits:
                continue
            for c in t:
                sec = LIMIT_SECTIONS.get(c.tag)
                if not sec:
                    continue
                if c.get("name") is not None and sec in limits and c.get("name") not in limits[sec]:
                    add("error", tr("'{0}': <{1} name=\"{2}\"> не объявлен в cfglimitsdefinition.xml").format(
                        name, c.tag, c.get("name")), tp, line, "mission-limits")
                if c.get("user") is not None and c.get("user") not in users.get(sec, set()):
                    add("error", tr("'{0}': <{1} user=\"{2}\"> не объявлен в cfglimitsdefinitionuser.xml").format(
                        name, c.tag, c.get("user")), tp, line, "mission-limits")

    # --- events ---
    event_names: Set[str] = set()
    for ep in files["events"]:
        eroot, text = _parse(ep)
        if eroot is None:
            continue
        for e in eroot.findall("event"):
            ename = e.get("name") or ""
            event_names.add(ename)
            for c in e.iter("child"):
                ct = c.get("type") or ""
                if type_names and ct and ct.lower() not in type_names:
                    add("warning", tr("событие {0}: тип {1} отсутствует в types.xml").format(ename, ct), ep,
                        _line_of(text, "event", ename), "mission-event-child")

    spawns_path = _find(root, "cfgeventspawns.xml")
    groups_path = _find(root, "cfgeventgroups.xml")
    groups: Optional[Set[str]] = None
    if groups_path:
        groot, _ = _parse(groups_path)
        if groot is not None:
            groups = {g.get("name") for g in groot.findall("group")}
    if spawns_path:
        sroot, text = _parse(spawns_path)
        if sroot is not None:
            for ev in sroot.findall("event"):
                en = ev.get("name") or ""
                if event_names and en not in event_names:
                    add("info", tr("cfgeventspawns.xml: позиции для события {0}, которого нет в events.xml").format(en),
                        spawns_path, _line_of(text, "event", en), "mission-spawn-event")
                for p in ev.iter("pos"):
                    g = p.get("group")
                    if g and groups is not None and g not in groups:
                        add("error", tr("cfgeventspawns.xml: группа {0} не найдена в cfgeventgroups.xml").format(g),
                            spawns_path, _line_of(text, "event", en), "mission-group")

    # --- spawnabletypes и пресеты ---
    presets: Optional[Set[str]] = None
    preset_path = _find(root, "cfgrandompresets.xml")
    if preset_path:
        proot, ptext = _parse(preset_path)
        if proot is not None:
            presets = {c.get("name") for c in proot}
            for c in proot:
                for it in c:
                    iname = it.get("name") or ""
                    if type_names and iname and iname.lower() not in type_names:
                        add("info", tr("cfgrandompresets.xml: предмет {0} (пресет {1}) отсутствует в types.xml").format(
                            iname, c.get("name")), preset_path, 0, "mission-preset-item")
    for sp in files["spawnabletypes"]:
        sroot, text = _parse(sp)
        if sroot is None:
            continue
        for t in sroot.findall("type"):
            name = t.get("name") or ""
            line = _line_of(text, "type", name)
            if type_names and name.lower() not in type_names:
                add("info", tr("{0}: тип отсутствует в types.xml (нормально для предметов, которые появляются только "
                               "как навесное)").format(name), sp, line, "mission-spawnable-type")
            for x in t.iter():
                pr = x.get("preset")
                if pr and presets is not None and pr not in presets:
                    add("error", tr("{0}: пресет {1} не найден в cfgrandompresets.xml").format(name, pr), sp, line,
                        "mission-preset")
                if x.tag == "item":
                    iname = x.get("name") or ""
                    if type_names and iname and iname.lower() not in type_names:
                        add("info", tr("{0}: предмет {1} отсутствует в types.xml").format(name, iname), sp, line,
                            "mission-spawnable-item")

    # --- cfggameplay.json: ссылки на файлы ---
    gp = _find(root, "cfggameplay.json")
    if gp:
        try:
            data = json.loads(gp.read_bytes().decode("utf-8-sig"))
        except (ValueError, OSError):
            data = None

        def walk(v):
            if isinstance(v, dict):
                for x in v.values():
                    yield from walk(x)
            elif isinstance(v, list):
                for x in v:
                    yield from walk(x)
            elif isinstance(v, str) and v.lower().endswith((".json", ".xml")):
                yield v

        for ref in walk(data):
            p = root / ref.replace("\\", "/").lstrip("/")
            if not p.is_file():
                add("error", tr("cfggameplay.json: файл {0} не найден в папке миссии").format(ref), gp, 0,
                    "mission-gameplay-file")
    return issues
