# -*- coding: utf-8 -*-
"""Пакетные конвертеры: картинки <-> PAA, config.cpp <-> config.bin."""

from __future__ import annotations

from dztk.i18n import tr

import threading
from concurrent.futures import ThreadPoolExecutor, as_completed
from dataclasses import dataclass
from pathlib import Path
from typing import Callable, Iterable, List, Optional, Set, Tuple

from . import cfg, paa, rap

CONFIG_EXTENSIONS = {".cpp", ".bin", ".hpp"}


@dataclass
class Task:
    src: Path
    dst: Path


@dataclass
class Result:
    task: Task
    ok: bool
    message: str = ""
    details: Optional[list] = None   # предупреждения (Issue)


def collect(paths: Iterable[str], exts: Set[str], skip_dirs: Iterable[Path] = ()) -> List[Tuple[Path, Optional[Path]]]:
    out, seen = [], set()
    skip = [Path(d).resolve() for d in skip_dirs]
    for raw in paths:
        p = Path(raw)
        items = []
        if p.is_dir():
            items = [(f, p) for f in sorted(p.rglob("*")) if f.is_file() and f.suffix.lower() in exts]
        elif p.is_file() and p.suffix.lower() in exts:
            items = [(p, None)]
        for f, root in items:
            r = f.resolve()
            if r in seen or any(_within(r, s) for s in skip):
                continue
            seen.add(r)
            out.append((f, root))
    return out


def _within(p: Path, parent: Path) -> bool:
    try:
        p.relative_to(parent)
        return True
    except ValueError:
        return False


def plan(inputs: List[Tuple[Path, Optional[Path]]], out_dir: str, new_suffix: str,
         name_fn: Optional[Callable[[Path], str]] = None) -> List[Task]:
    tasks = []
    for src, root in inputs:
        name = name_fn(src) if name_fn else src.stem + new_suffix
        if out_dir:
            base = Path(out_dir)
            if root is not None:
                base = base.joinpath(*src.parent.relative_to(root).parts)
        else:
            base = src.parent
        tasks.append(Task(src, base / name))
    return tasks


def run(tasks: List[Task], fn: Callable[[Task], Result], threads: int = 4,
        on_result: Optional[Callable[[Result, int, int], None]] = None,
        cancel: Optional[threading.Event] = None) -> List[Result]:
    results: List[Result] = []

    def wrapped(t: Task) -> Result:
        if cancel is not None and cancel.is_set():
            return Result(t, False, tr("отменено"))
        try:
            return fn(t)
        except Exception as e:
            return Result(t, False, str(e))

    with ThreadPoolExecutor(max_workers=max(1, threads)) as ex:
        futs = [ex.submit(wrapped, t) for t in tasks]
        for f in as_completed(futs):
            r = f.result()
            results.append(r)
            if on_result:
                on_result(r, len(results), len(tasks))
    order = {id(t): i for i, t in enumerate(tasks)}
    results.sort(key=lambda r: order.get(id(r.task), 0))
    return results


# --------------------------------------------------------------------------------------
# Текстуры
# --------------------------------------------------------------------------------------

def image_to_paa_task(fmt: str = "auto", resize: str = "error", overwrite: bool = True):
    def fn(t: Task) -> Result:
        if t.dst.exists() and not overwrite:
            return Result(t, True, tr("пропущен (уже существует)"))
        w, h, used = paa.image_to_paa(t.src, t.dst, fmt, resize)
        return Result(t, True, f"{w}x{h} {used}")
    return fn


def paa_to_image_task(overwrite: bool = True):
    def fn(t: Task) -> Result:
        if t.dst.exists() and not overwrite:
            return Result(t, True, tr("пропущен (уже существует)"))
        info = paa.paa_to_image(t.src, t.dst)
        return Result(t, True, f"{info.width}x{info.height} {info.type_name}")
    return fn


# --------------------------------------------------------------------------------------
# Конфиги
# --------------------------------------------------------------------------------------

def config_task(overwrite: bool = False, check: bool = True):
    """Бинаризует .cpp или разбирает .bin — направление по содержимому файла."""
    def fn(t: Task) -> Result:
        if t.dst.exists() and not overwrite:
            return Result(t, False, tr("{0} уже существует (включите перезапись)").format(t.dst.name))
        data = t.src.read_bytes()
        if rap.is_rapified(data):
            root = rap.read_rap(data)
            t.dst.parent.mkdir(parents=True, exist_ok=True)
            t.dst.write_text(cfg.to_text(root).replace("\n", "\r\n"), encoding="utf-8", newline="")
            return Result(t, True, "bin -> cpp")
        try:
            root, issues = cfg.parse_config(t.src)
        except cfg.ConfigError as e:
            return Result(t, False, e.issue.format(), [e.issue])
        if check:
            issues = issues + cfg.semantic_check(root, str(t.src))
            errors = [i for i in issues if i.level == "error"]
            if errors:
                return Result(t, False, tr("не бинаризован, ошибок: {0}").format(len(errors)), issues)
        t.dst.parent.mkdir(parents=True, exist_ok=True)
        t.dst.write_bytes(rap.write_rap(root))
        warn = [i for i in issues if i.level == "warning"]
        return Result(t, True, "cpp -> bin" + (tr(", предупреждений: {0}").format(len(warn)) if warn else ""), issues)
    return fn


def config_dst_name(src: Path) -> str:
    try:
        is_bin = rap.is_rapified(src.read_bytes()[:4])
    except OSError:
        is_bin = src.suffix.lower() == ".bin"
    return src.stem + (".cpp" if is_bin else ".bin")
