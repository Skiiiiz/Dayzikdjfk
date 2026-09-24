# -*- coding: utf-8 -*-
"""Проверка новой версии через GitHub Releases (без автообновления — только уведомление со ссылкой)."""

from __future__ import annotations

from .i18n import tr

import json
import re
import time
import urllib.request
from dataclasses import dataclass
from typing import Optional, Tuple

from . import APP_NAME, APP_VERSION

REPO = "Skiiiiz/Dayzikdjfk"
TAG = "dayz-toolkit-latest"
API = f"https://api.github.com/repos/{REPO}/releases/tags/{TAG}"
PAGE = f"https://github.com/{REPO}/releases/tag/{TAG}"
INTERVAL = 24 * 3600


@dataclass
class UpdateInfo:
    version: str
    url: str
    download: str
    notes: str

    @property
    def newer(self) -> bool:
        return is_newer(self.version, APP_VERSION)


def parse_version(text: str) -> Optional[Tuple[int, ...]]:
    m = re.search(r"(\d+)\.(\d+)(?:\.(\d+))?", text or "")
    if not m:
        return None
    return tuple(int(x or 0) for x in m.groups())


def is_newer(remote: str, local: str) -> bool:
    r, l_ = parse_version(remote), parse_version(local)
    return bool(r and l_ and r > l_)


def from_release(data: dict) -> Optional[UpdateInfo]:
    ver = None
    for src in (data.get("name") or "", data.get("body") or ""):
        m = re.search(r"(?:version|версия|toolkit)\s*v?(\d+\.\d+(?:\.\d+)?)", src, re.I)
        if m:
            ver = m.group(1)
            break
    if not ver:
        return None
    exe = next((a.get("browser_download_url") for a in data.get("assets", [])
                if str(a.get("name", "")).lower().endswith(".exe")), "")
    return UpdateInfo(ver, data.get("html_url") or PAGE, exe or "", (data.get("body") or "").strip())


def fetch(timeout: float = 6.0) -> Optional[UpdateInfo]:
    req = urllib.request.Request(API, headers={"Accept": "application/vnd.github+json",
                                               "User-Agent": f"{APP_NAME}/{APP_VERSION}"})
    with urllib.request.urlopen(req, timeout=timeout) as r:
        return from_release(json.loads(r.read().decode("utf-8")))


def due(settings: dict) -> bool:
    if not settings.get("check_updates", True):
        return False
    return time.time() - float(settings.get("update_checked", 0) or 0) > INTERVAL


def message(info: Optional[UpdateInfo]) -> str:
    if info is None:
        return tr("Не удалось определить версию последнего выпуска.")
    if info.newer:
        return tr("Доступна новая версия {0} (у вас {1}): {2}").format(info.version, APP_VERSION, info.url)
    return tr("У вас последняя версия ({0}).").format(APP_VERSION)
