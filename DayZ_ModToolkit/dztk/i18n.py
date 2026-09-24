# -*- coding: utf-8 -*-
"""Локализация: исходные строки программы на русском, перевод на английский — в i18n_en.py.

Язык выбирается до импорта остальных модулей (строки-константы переводятся при импорте):
переменная окружения DZTK_LANG, ключ --lang, настройка «lang» или язык системы.
"""

from __future__ import annotations

import locale
import os
import sys
from typing import Dict, Optional

LANGUAGES = {"ru": "Русский", "en": "English"}
_lang = "ru"
_catalog: Dict[str, str] = {}


def system_language() -> str:
    try:
        if os.name == "nt":
            import ctypes
            lang_id = ctypes.windll.kernel32.GetUserDefaultUILanguage() & 0x3FF  # type: ignore[attr-defined]
            return "ru" if lang_id in (0x19, 0x22, 0x23) else "en"   # русский, украинский, белорусский
        code = (locale.getlocale()[0] or os.environ.get("LANG", "")).lower()
        return "ru" if code.startswith(("ru", "uk", "be")) else "en"
    except Exception:
        return "en"


def set_language(lang: Optional[str]) -> str:
    global _lang, _catalog
    lang = (lang or "").lower()
    if lang not in LANGUAGES:
        lang = system_language()
    _lang = lang
    if lang == "en":
        from .i18n_en import EN
        _catalog = EN
    else:
        _catalog = {}
    return _lang


def language() -> str:
    return _lang


def tr(text: str) -> str:
    """Переводит строку (для русского возвращает её как есть)."""
    if not _catalog:
        return text
    return _catalog.get(text, text)


def init_from_environment(argv=None, settings_lang: str = "") -> str:
    """Определяет язык: --lang в аргументах > DZTK_LANG > настройка > язык системы."""
    argv = list(sys.argv[1:] if argv is None else argv)
    lang = ""
    if "--lang" in argv:
        i = argv.index("--lang")
        if i + 1 < len(argv):
            lang = argv[i + 1]
    lang = lang or os.environ.get("DZTK_LANG", "") or settings_lang
    return set_language(lang)
