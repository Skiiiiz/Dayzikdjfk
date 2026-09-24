# -*- coding: utf-8 -*-
"""Индекс классов игры (и других модов) для проверок «по игре».

Источник — любая из папок:
  * папка установки DayZ (читаются dta/*.pbo и Addons/*.pbo прямо из архивов);
  * распакованные данные (P:\\scripts, P:\\DZ) — файлы .c и config.cpp/config.bin;
  * папки других модов, от которых зависит ваш мод.

Проверки, которые включаются с индексом:
  * modded class X — класс X должен существовать;
  * class A extends B — класс B должен существовать;
  * override у метода — метод должен быть у одного из родительских классов;
  * в config.cpp: внешний класс (class X;) в CfgVehicles/CfgWeapons/... должен быть в игре или моде.
"""

from __future__ import annotations

from dztk.i18n import tr

import hashlib
import json
import time
from dataclasses import dataclass, field
from pathlib import Path
from typing import Callable, Dict, Iterable, List, Optional, Set, Tuple

from . import cfg, enforce, rap
from .cfg import Issue

NOT_METHOD = {"if", "while", "for", "foreach", "switch", "return", "new", "delete", "super", "this", "thread",
              "sizeof", "typename", "else", "case", "Cast", "CastTo"}
INDEX_VERSION = 2


@dataclass
class ClassInfo:
    name: str
    base: str = ""
    methods: Set[str] = field(default_factory=set)
    defined: bool = False       # есть не-modded объявление


@dataclass
class ScriptDecl:
    """Объявление класса в проверяемом моде (для сообщений об ошибках)."""
    name: str
    base: str
    modded: bool
    file: str
    line: int
    overrides: List[Tuple[str, int, int]] = field(default_factory=list)   # (метод, строка, столбец)
    methods: Set[str] = field(default_factory=set)


def parse_script(text: str, file: str = "") -> Tuple[List[ScriptDecl], Set[str]]:
    """Находит классы (с базами, методами и override) и имена typedef/enum в тексте скрипта."""
    toks = enforce._Lexer(text, file, []).run()
    decls: List[ScriptDecl] = []
    types: Set[str] = set()
    depth = 0
    cur: Optional[ScriptDecl] = None
    cur_depth = -1
    paren = 0
    pending_bases: Dict[str, List[str]] = {}   # class X extends A (без тела, ветка #ifdef) -> A
    n = len(toks)
    i = 0
    while i < n:
        t = toks[i]
        if t.kind == "op":
            if t.text == "{":
                depth += 1
            elif t.text == "}":
                depth -= 1
                if cur is not None and depth < cur_depth:
                    cur = None
            elif t.text == "(":
                paren += 1
            elif t.text == ")":
                paren = max(0, paren - 1)
            i += 1
            continue
        if t.kind != "ident":
            i += 1
            continue
        if depth == 0 and t.text == "class" and i + 1 < n and toks[i + 1].kind == "ident":
            name = toks[i + 1].text
            modded = i > 0 and toks[i - 1].text == "modded"
            j = i + 2
            # шаблонные параметры class X<Class T>
            if j < n and toks[j].text == "<":
                k = 1
                j += 1
                while j < n and k:
                    k += {"<": 1, ">": -1, ">>": -2}.get(toks[j].text, 0)
                    j += 1
            base = ""
            if j < n and toks[j].text in ("extends", ":") and j + 1 < n and toks[j + 1].kind == "ident":
                base = toks[j + 1].text
                j += 2
            # пропускаем до '{' или ';' (объявление вперёд)
            while j < n and toks[j].text not in ("{", ";") and not (toks[j].kind == "ident" and toks[j].text == "class"):
                j += 1
            if j < n and toks[j].text == "{":
                alts = pending_bases.pop(name.lower(), [])
                if alts and base:
                    base = "|".join(dict.fromkeys(alts + [base]))
                cur = ScriptDecl(name, base, modded, file, toks[i + 1].line)
                decls.append(cur)
                cur_depth = depth + 1
            elif base and j < n and toks[j].kind == "ident" and toks[j].text == "class":
                pending_bases.setdefault(name.lower(), []).append(base)
            i = j
            continue
        if depth == 0 and t.text in ("typedef", "enum"):
            if t.text == "enum" and i + 1 < n and toks[i + 1].kind == "ident":
                types.add(toks[i + 1].text)
            elif t.text == "typedef":
                j = i + 1
                while j < n and toks[j].text != ";":
                    j += 1
                if toks[j - 1].kind == "ident":
                    types.add(toks[j - 1].text)
            i += 1
            continue
        # объявление метода внутри класса: <тип> Имя(
        if (cur is not None and depth == cur_depth and paren == 0 and i + 1 < n and toks[i + 1].text == "("
                and t.text not in NOT_METHOD and i > 0):
            prev = toks[i - 1]
            if (prev.kind == "ident" and prev.text not in NOT_METHOD) or prev.text in (">", ">>", "]"):
                cur.methods.add(t.text)
                # есть ли override среди модификаторов объявления
                k = i - 1
                while k >= 0 and toks[k].text not in (";", "{", "}"):
                    if toks[k].text == "override":
                        cur.overrides.append((t.text, t.line, t.col))
                        break
                    k -= 1
        i += 1
    return decls, types


@dataclass
class Index:
    classes: Dict[str, ClassInfo] = field(default_factory=dict)      # имя (lower) -> информация
    types: Set[str] = field(default_factory=set)                      # typedef/enum (lower)
    config_classes: Dict[str, Set[str]] = field(default_factory=dict) # CfgXXX (lower) -> имена (lower)
    sources: List[str] = field(default_factory=list)
    files: int = 0

    def add_script(self, text: str, file: str = "") -> None:
        decls, types = parse_script(text, file)
        self.types |= {x.lower() for x in types}
        for d in decls:
            key = d.name.lower()
            ci = self.classes.get(key)
            if ci is None:
                ci = self.classes[key] = ClassInfo(d.name)
            if not d.modded:
                ci.defined = True
                if d.base:
                    ci.base = d.base
            ci.methods |= {m.lower() for m in d.methods}
        self.files += 1

    def add_config(self, root: cfg.ClassNode) -> None:
        for top in root.entries:
            if isinstance(top, cfg.ClassNode) and not top.extern and top.name.lower().startswith("cfg"):
                names = self.config_classes.setdefault(top.name.lower(), set())
                for c in top.entries:
                    if isinstance(c, cfg.ClassNode) and not c.extern:
                        names.add(c.name.lower())
        self.files += 1

    def knows_class(self, name: str) -> bool:
        def one(k: str) -> bool:
            return (k in self.classes and self.classes[k].defined) or k in self.types
        return any(one(k.strip().lower()) for k in name.split("|"))

    def has_method(self, cls: str, method: str, own: bool = True) -> Optional[bool]:
        """Есть ли метод у класса или его предков (все классы неявно наследуют Class).
        None — цепочка уходит в неизвестный класс."""
        m = method.lower()
        seen: Set[str] = set()
        unknown = False
        todo = [(k.strip().lower(), True) for k in cls.split("|")]
        while todo:
            k, first = todo.pop()
            if not k or k in seen:
                continue
            seen.add(k)
            ci = self.classes.get(k)
            if ci is None:
                unknown = True
                continue
            if (own or not first) and m in ci.methods:
                return True
            if ci.base:
                todo += [(b.strip().lower(), False) for b in ci.base.split("|")]
            elif k != "class":
                todo.append(("class", False))
        return None if unknown else False

    # --- сохранение ---
    def to_json(self) -> dict:
        return {"v": INDEX_VERSION, "sources": self.sources, "files": self.files,
                "types": sorted(self.types),
                "classes": {k: [c.name, c.base, sorted(c.methods), c.defined] for k, c in self.classes.items()},
                "config": {k: sorted(v) for k, v in self.config_classes.items()}}

    @classmethod
    def from_json(cls, d: dict) -> "Index":
        idx = cls(sources=d.get("sources", []), files=d.get("files", 0), types=set(d.get("types", [])))
        for k, (name, base, methods, defined) in d.get("classes", {}).items():
            idx.classes[k] = ClassInfo(name, base, set(methods), defined)
        idx.config_classes = {k: set(v) for k, v in d.get("config", {}).items()}
        return idx

    def merged(self, other: "Index") -> "Index":
        res = Index.from_json(self.to_json())
        for k, c in other.classes.items():
            ci = res.classes.setdefault(k, ClassInfo(c.name))
            ci.methods |= c.methods
            if c.defined:
                ci.defined = True
                ci.base = c.base or ci.base
        res.types |= other.types
        for k, v in other.config_classes.items():
            res.config_classes.setdefault(k, set()).update(v)
        return res


# --------------------------------------------------------------------------------------
# Построение
# --------------------------------------------------------------------------------------

def _signature(paths: List[Path]) -> str:
    h = hashlib.sha1()
    for p in paths:
        h.update(str(p.resolve()).encode())
        for f in sorted(p.rglob("*.pbo"))[:4000] if p.is_dir() else []:
            st = f.stat()
            h.update(f"{f.name}{st.st_size}{int(st.st_mtime)}".encode())
        if p.is_dir():
            h.update(str(sum(1 for _ in p.rglob("*.c"))).encode())
    h.update(str(INDEX_VERSION).encode())
    return h.hexdigest()


def build_index(sources: Iterable[str], on_progress: Optional[Callable[[str], None]] = None) -> Index:
    from . import pbo as pbomod
    idx = Index()
    say = on_progress or (lambda s: None)
    for raw in sources:
        root = Path(raw)
        if not root.exists():
            say(tr("нет папки: {0}").format(root))
            continue
        idx.sources.append(str(root))
        # 1) PBO-архивы игры/модов
        pbos = sorted(root.rglob("*.pbo")) if root.is_dir() else ([root] if root.suffix.lower() == ".pbo" else [])
        for f in pbos:
            try:
                arc = pbomod.read_pbo(f)
            except Exception as e:
                say(tr("пропущен {0}: {1}").format(f.name, e))
                continue
            n = 0
            for e in arc.entries:
                low = e.name.lower()
                try:
                    if low.endswith(".c"):
                        idx.add_script(arc.read(e).decode("utf-8", "replace"), f"{f.name}:{e.name}")
                        n += 1
                    elif low.endswith("config.bin"):
                        idx.add_config(rap.read_rap(arc.read(e)))
                        n += 1
                except Exception:
                    continue
            if n:
                say(tr("{0}: {1} файлов").format(f.name, n))
        # 2) распакованные файлы
        if root.is_dir():
            for f in root.rglob("*"):
                if not f.is_file():
                    continue
                low = f.name.lower()
                try:
                    if low.endswith(".c"):
                        idx.add_script(cfg.read_text(f), str(f))
                    elif low == "config.cpp":
                        node, _ = cfg.parse_config(f)
                        idx.add_config(node)
                    elif low == "config.bin":
                        idx.add_config(rap.read_rap(f.read_bytes()))
                except Exception:
                    continue
    say(tr("Индекс: классов скриптов {0}, конфиг-разделов {1}, файлов {2}").format(len(idx.classes), len(idx.config_classes), idx.files))
    return idx


def cache_path() -> Path:
    from .common import settings_file
    return settings_file().parent / "vanilla_index.json"


def load_index(sources: List[str], on_progress: Optional[Callable[[str], None]] = None,
               use_cache: bool = True) -> Index:
    """Строит индекс или берёт из кэша, если источники не менялись."""
    paths = [Path(s) for s in sources if s]
    sig = _signature(paths)
    cp = cache_path()
    if use_cache and cp.is_file():
        try:
            d = json.loads(cp.read_text(encoding="utf-8"))
            if d.get("sig") == sig:
                return Index.from_json(d["index"])
        except Exception:
            pass
    t = time.time()
    idx = build_index([str(p) for p in paths], on_progress)
    if on_progress:
        on_progress(tr("построено за {0:.1f} с").format(time.time() - t))
    try:
        cp.parent.mkdir(parents=True, exist_ok=True)
        cp.write_text(json.dumps({"sig": sig, "index": idx.to_json()}), encoding="utf-8")
    except OSError:
        pass
    return idx


# --------------------------------------------------------------------------------------
# Проверки
# --------------------------------------------------------------------------------------

CONFIG_ROOTS = ("cfgvehicles", "cfgweapons", "cfgmagazines", "cfgammo", "cfgnonaivehicles", "cfgsoundshaders",
                "cfgsoundsets", "cfgslots", "cfgworlds", "cfgsurfaces")


def check_mod_scripts(script_files: List[Path], game: Index) -> List[Issue]:
    """Проверяет скрипты мода против индекса игры (мод тоже добавляется в индекс)."""
    mod = Index()
    decls: List[ScriptDecl] = []
    for f in script_files:
        text = cfg.read_text(f)
        d, types = parse_script(text, str(f))
        decls += d
        mod.add_script(text, str(f))
    full = game.merged(mod)
    issues: List[Issue] = []
    for d in decls:
        if d.modded and not full.knows_class(d.name):
            issues.append(Issue("error", tr("modded class {0}: такого класса нет ни в игре, ни в моде").format(d.name),
                                d.file, d.line, 0, "unknown-class"))
            continue
        if d.base and not full.knows_class(d.base):
            issues.append(Issue("error", tr("класс {0} наследуется от неизвестного класса {1}").format(d.name, d.base.split('|')[-1]),
                                d.file, d.line, 0, "unknown-base"))
            continue
        for m, line, col in d.overrides:
            ml = m.lower()
            if d.modded:
                # override в modded-классе относится к методу исходного класса (или его предков)
                found = game.has_method(d.name, m, own=True)
                if found is not True and any(o is not d and o.name.lower() == d.name.lower()
                                             and ml in {x.lower() for x in o.methods} for o in decls):
                    found = True
            else:
                found = full.has_method(d.base or "Class", m, own=True)
            if found is False:
                issues.append(Issue("error", tr("{0}.{1}: помечен override, но у родительских классов нет метода {2} (опечатка или метод удалён в новой версии игры)").format(d.name, m, m),
                                    d.file, line, col, "bad-override"))
    return issues


def check_config_externs(root: cfg.ClassNode, file: str, game: Index) -> List[Issue]:
    issues: List[Issue] = []
    mod_defined: Dict[str, Set[str]] = {}
    for top in root.entries:
        if isinstance(top, cfg.ClassNode) and not top.extern:
            mod_defined[top.name.lower()] = {c.name.lower() for c in top.entries
                                             if isinstance(c, cfg.ClassNode) and not c.extern}
    for top in root.entries:
        if not (isinstance(top, cfg.ClassNode) and top.name.lower() in CONFIG_ROOTS):
            continue
        known = game.config_classes.get(top.name.lower())
        if not known:
            continue   # в индексе нет этого раздела — проверять не по чему
        for c in top.entries:
            if isinstance(c, cfg.ClassNode) and c.extern:
                k = c.name.lower()
                if k not in known and k not in mod_defined.get(top.name.lower(), set()):
                    issues.append(Issue("warning", tr("{0}: класс {1} не найден в игре — опечатка или нужен другой мод в requiredAddons").format(top.name, c.name), c.file or file,
                                        c.line, 0, "unknown-extern"))
    return issues
