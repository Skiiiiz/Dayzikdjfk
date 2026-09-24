# -*- coding: utf-8 -*-
"""Парсер config.cpp (формат конфигов Bohemia Interactive), препроцессор и запись обратно в текст.

Используется для:
  * проверки синтаксиса config.cpp (ошибки с номером строки и столбца);
  * семантических проверок (необъявленные базовые классы, дубликаты и т.п.);
  * бинаризации (config.cpp -> config.bin) и обратного преобразования.
"""

from __future__ import annotations

from dztk.i18n import tr

import re
import struct
from dataclasses import dataclass, field
from pathlib import Path
from typing import Dict, List, Optional, Tuple, Union

Value = Union[str, int, float, list]


# --------------------------------------------------------------------------------------
# Диагностика
# --------------------------------------------------------------------------------------

@dataclass
class Issue:
    level: str          # "error" | "warning" | "info"
    message: str
    file: str = ""
    line: int = 0
    col: int = 0
    code: str = ""

    def format(self) -> str:
        loc = self.file
        if self.line:
            loc += f":{self.line}" + (f":{self.col}" if self.col else "")
        lvl = {"error": tr("ОШИБКА"), "warning": tr("ПРЕДУПРЕЖДЕНИЕ"), "info": tr("ИНФО")}.get(self.level, self.level)
        return f"{loc}: {lvl}: {self.message}" if loc else f"{lvl}: {self.message}"


class ConfigError(Exception):
    def __init__(self, message: str, file: str = "", line: int = 0, col: int = 0):
        super().__init__(message)
        self.issue = Issue("error", message, file, line, col, "syntax")

    def __str__(self) -> str:
        return self.issue.format()


# --------------------------------------------------------------------------------------
# AST
# --------------------------------------------------------------------------------------

@dataclass
class Node:
    name: str
    line: int = 0
    file: str = ""


@dataclass
class ClassNode(Node):
    base: Optional[str] = None
    entries: List[Node] = field(default_factory=list)
    extern: bool = False          # class X;


@dataclass
class ValueNode(Node):
    value: Value = ""
    quoted: bool = True


@dataclass
class ArrayNode(Node):
    value: list = field(default_factory=list)
    append: bool = False          # name[] += {...};


@dataclass
class DeleteNode(Node):
    pass


# --------------------------------------------------------------------------------------
# Препроцессор
# --------------------------------------------------------------------------------------

@dataclass
class SrcLine:
    text: str
    file: str
    line: int


_IDENT = re.compile(r"[A-Za-z_][A-Za-z0-9_]*")


class Preprocessor:
    """Упрощённый препроцессор: #include, #define (с параметрами), #undef, #ifdef/#ifndef/#else/#endif."""

    def __init__(self, include_dirs: Optional[List[Path]] = None, defines: Optional[Dict[str, str]] = None):
        self.include_dirs = include_dirs or []
        self.macros: Dict[str, Tuple[Optional[List[str]], str]] = {k: (None, v) for k, v in (defines or {}).items()}
        self.issues: List[Issue] = []
        self._depth = 0

    def run(self, path: Path, text: Optional[str] = None) -> List[SrcLine]:
        if text is None:
            text = read_text(path)
        out: List[SrcLine] = []
        self._process(path, text, out)
        return out

    def _process(self, path: Path, text: str, out: List[SrcLine]) -> None:
        self._depth += 1
        if self._depth > 32:
            raise ConfigError(tr("слишком глубокая вложенность #include"), str(path))
        fname = str(path)
        text = _strip_comments(text, fname)
        lines = text.split("\n")
        # склейка строк с '\' в конце (только для директив и макросов)
        stack: List[bool] = []   # активность веток
        i = 0
        while i < len(lines):
            raw = lines[i]
            lineno = i + 1
            i += 1
            stripped = raw.strip()
            if stripped.startswith("#"):
                while stripped.endswith("\\") and i < len(lines):
                    stripped = stripped[:-1] + " " + lines[i].strip()
                    i += 1
                    out.append(SrcLine("", fname, i))
                active = all(stack)
                m = re.match(r"#\s*(\w+)\s*(.*)$", stripped)
                if not m:
                    out.append(SrcLine("", fname, lineno))
                    continue
                d, rest = m.group(1), m.group(2).strip()
                if d in ("ifdef", "ifndef"):
                    name = rest.split()[0] if rest else ""
                    cond = name in self.macros
                    stack.append(cond if d == "ifdef" else not cond)
                elif d == "if":
                    self.issues.append(Issue("warning", tr("#if не вычисляется, ветка считается истинной"),
                                             fname, lineno))
                    stack.append(True)
                elif d == "else":
                    if not stack:
                        raise ConfigError(tr("#else без #ifdef"), fname, lineno)
                    stack[-1] = not stack[-1]
                elif d == "endif":
                    if not stack:
                        raise ConfigError(tr("#endif без #ifdef"), fname, lineno)
                    stack.pop()
                elif not active:
                    pass
                elif d == "define":
                    self._define(rest, fname, lineno)
                elif d == "undef":
                    self.macros.pop(rest.split()[0] if rest else "", None)
                elif d == "include":
                    self._include(rest, path, fname, lineno, out)
                    continue
                else:
                    self.issues.append(Issue("warning", tr("неизвестная директива #{0}").format(d), fname, lineno))
                out.append(SrcLine("", fname, lineno))
                continue
            if not all(stack):
                out.append(SrcLine("", fname, lineno))
                continue
            out.append(SrcLine(self._expand(raw, fname, lineno), fname, lineno))
        if stack:
            raise ConfigError(tr("не закрыт #ifdef/#ifndef (нет #endif)"), fname, len(lines))
        self._depth -= 1

    def _define(self, rest: str, fname: str, lineno: int) -> None:
        m = re.match(r"([A-Za-z_]\w*)(\(([^)]*)\))?\s*(.*)$", rest)
        if not m:
            raise ConfigError(tr("неверный #define"), fname, lineno)
        params = [p.strip() for p in m.group(3).split(",")] if m.group(2) else None
        if params == [""]:
            params = []
        self.macros[m.group(1)] = (params, m.group(4))

    def _include(self, rest: str, cur: Path, fname: str, lineno: int, out: List[SrcLine]) -> None:
        m = re.match(r'["<](.+?)[">]', rest)
        if not m:
            raise ConfigError(tr("неверный #include"), fname, lineno)
        inc = m.group(1).replace("\\", "/")
        candidates = [cur.parent / inc] + [d / inc.lstrip("/") for d in self.include_dirs]
        for c in candidates:
            if c.is_file():
                self._process(c, read_text(c), out)
                return
        if m.group(1).startswith(("\\", "/")):
            # абсолютный путь P:\... (данные игры или другого мода) — проверить нечем
            self.issues.append(Issue("info", tr("внешний #include \"{0}\" пропущен (файл не в моде)").format(m.group(1)),
                                     fname, lineno, 0, "include-external"))
        else:
            self.issues.append(Issue("error", tr("не найден файл #include \"{0}\"").format(m.group(1)), fname, lineno, 0,
                                     "include"))
        out.append(SrcLine("", fname, lineno))

    def _expand(self, text: str, fname: str, lineno: int, depth: int = 0) -> str:
        if not self.macros or depth > 16:
            return text
        parts = _split_strings(text)
        changed = False
        res = []
        for is_str, chunk in parts:
            if is_str:
                res.append(chunk)
                continue
            out, pos = [], 0
            for m in _IDENT.finditer(chunk):
                name = m.group(0)
                if m.start() < pos or name not in self.macros:
                    continue
                params, body = self.macros[name]
                if params is None:
                    out.append(chunk[pos:m.start()])
                    out.append(body)
                    pos = m.end()
                    changed = True
                else:
                    j = m.end()
                    while j < len(chunk) and chunk[j] == " ":
                        j += 1
                    if j >= len(chunk) or chunk[j] != "(":
                        continue
                    args, end = _parse_macro_args(chunk, j)
                    if args is None:
                        continue
                    b = body
                    for p, a in zip(params, args):
                        val = a.strip()
                        b = re.sub(r"##\s*\b%s\b|\b%s\b\s*##" % (re.escape(p), re.escape(p)), lambda _m: val, b)
                        b = re.sub(r"#\b%s\b" % re.escape(p), lambda _m: '"%s"' % val, b)
                        b = re.sub(r"\b%s\b" % re.escape(p), lambda _m: val, b)
                    b = b.replace("##", "")
                    out.append(chunk[pos:m.start()])
                    out.append(b)
                    pos = end
                    changed = True
            out.append(chunk[pos:])
            res.append("".join(out))
        text = "".join(res)
        return self._expand(text, fname, lineno, depth + 1) if changed else text


def _parse_macro_args(s: str, j: int):
    depth, args, cur = 0, [], []
    k = j
    while k < len(s):
        ch = s[k]
        if ch == "(":
            depth += 1
            if depth > 1:
                cur.append(ch)
        elif ch == ")":
            depth -= 1
            if depth == 0:
                args.append("".join(cur))
                return args, k + 1
            cur.append(ch)
        elif ch == "," and depth == 1:
            args.append("".join(cur))
            cur = []
        else:
            cur.append(ch)
        k += 1
    return None, j


def _split_strings(text: str) -> List[Tuple[bool, str]]:
    """Делит строку на куски вне/внутри "строковых литералов"."""
    res, i, start = [], 0, 0
    while i < len(text):
        if text[i] == '"':
            res.append((False, text[start:i]))
            j = i + 1
            while j < len(text):
                if text[j] == '"':
                    if j + 1 < len(text) and text[j + 1] == '"':
                        j += 2
                        continue
                    break
                j += 1
            res.append((True, text[i:j + 1]))
            i = start = j + 1
            continue
        i += 1
    res.append((False, text[start:]))
    return res


def _strip_comments(text: str, fname: str) -> str:
    """Удаляет // и /* */ комментарии, сохраняя переводы строк (для номеров строк)."""
    out = []
    i, n = 0, len(text)
    line = 1
    while i < n:
        ch = text[i]
        if ch == '"':
            j = i + 1
            while j < n:
                if text[j] == '"':
                    if j + 1 < n and text[j + 1] == '"':
                        j += 2
                        continue
                    break
                if text[j] == "\n":
                    break
                j += 1
            out.append(text[i:j + 1])
            i = j + 1
            continue
        if text.startswith("//", i):
            j = text.find("\n", i)
            i = n if j < 0 else j
            continue
        if text.startswith("/*", i):
            j = text.find("*/", i + 2)
            if j < 0:
                raise ConfigError(tr("незакрытый комментарий /*"), fname, line)
            chunk = text[i:j + 2]
            out.append("\n" * chunk.count("\n"))
            line += chunk.count("\n")
            i = j + 2
            continue
        if ch == "\n":
            line += 1
        out.append(ch)
        i += 1
    return "".join(out)


def read_text(path: Path) -> str:
    data = Path(path).read_bytes()
    if data.startswith(b"\xef\xbb\xbf"):
        data = data[3:]
    try:
        return data.decode("utf-8").replace("\r\n", "\n").replace("\r", "\n")
    except UnicodeDecodeError:
        return data.decode("cp1251", "replace").replace("\r\n", "\n").replace("\r", "\n")


# --------------------------------------------------------------------------------------
# Лексер
# --------------------------------------------------------------------------------------

@dataclass
class Tok:
    kind: str     # ident | number | string | sym | eof
    text: str
    file: str
    line: int
    col: int
    value: Value = None


_NUM = re.compile(r"[+-]?(0[xX][0-9A-Fa-f]+|(\d+\.?\d*|\.\d+)([eE][+-]?\d+)?)")


def tokenize(lines: List[SrcLine]) -> List[Tok]:
    toks: List[Tok] = []
    for sl in lines:
        s = sl.text
        i, n = 0, len(s)
        while i < n:
            ch = s[i]
            if ch in " \t\f\v":
                i += 1
                continue
            col = i + 1
            if ch == '"':
                j, buf = i + 1, []
                closed = False
                while j < n:
                    if s[j] == '"':
                        if j + 1 < n and s[j + 1] == '"':
                            buf.append('"')
                            j += 2
                            continue
                        closed = True
                        break
                    buf.append(s[j])
                    j += 1
                if not closed:
                    raise ConfigError(tr("незакрытая строка (нет закрывающей \")"), sl.file, sl.line, col)
                toks.append(Tok("string", s[i:j + 1], sl.file, sl.line, col, "".join(buf)))
                i = j + 1
                continue
            if s.startswith("+=", i):
                toks.append(Tok("sym", "+=", sl.file, sl.line, col))
                i += 2
                continue
            if ch in "{}[];:=,":
                toks.append(Tok("sym", ch, sl.file, sl.line, col))
                i += 1
                continue
            m = _NUM.match(s, i)
            if m and (m.end() >= n or not (s[m.end()].isalnum() or s[m.end()] in "_\\/.")):
                t = m.group(0)
                try:
                    v: Value = int(t, 16) if "x" in t.lower() else (float(t) if any(c in t for c in ".eE") else int(t))
                except ValueError:
                    v = t
                toks.append(Tok("number", t, sl.file, sl.line, col, v))
                i = m.end()
                continue
            if ch.isalpha() or ch == "_":
                m = re.compile(r"[A-Za-z0-9_]+").match(s, i)
                toks.append(Tok("ident", m.group(0), sl.file, sl.line, col))
                i = m.end()
                continue
            # прочие символы — часть «голых» значений (пути, выражения)
            toks.append(Tok("other", ch, sl.file, sl.line, col))
            i += 1
    last = lines[-1] if lines else SrcLine("", "", 0)
    toks.append(Tok("eof", "", last.file, last.line, 1))
    return toks


# --------------------------------------------------------------------------------------
# Парсер
# --------------------------------------------------------------------------------------

class Parser:
    def __init__(self, toks: List[Tok]):
        self.toks = toks
        self.i = 0
        self.issues: List[Issue] = []

    def peek(self, k: int = 0) -> Tok:
        return self.toks[min(self.i + k, len(self.toks) - 1)]

    def next(self) -> Tok:
        t = self.toks[self.i]
        self.i = min(self.i + 1, len(self.toks) - 1)
        return t

    def error(self, msg: str, t: Optional[Tok] = None):
        t = t or self.peek()
        raise ConfigError(msg, t.file, t.line, t.col)

    def expect(self, text: str, what: str = "") -> Tok:
        t = self.peek()
        if t.kind == "sym" and t.text == text:
            return self.next()
        found = tr("конец файла") if t.kind == "eof" else f"'{t.text}'"
        self.error(tr("ожидалось '{0}'{1}, найдено {2}").format(text, ' ' + what if what else '', found), t)

    def parse(self) -> ClassNode:
        root = ClassNode("", line=0)
        root.entries = self.parse_body(top=True)
        return root

    def parse_body(self, top: bool = False) -> List[Node]:
        entries: List[Node] = []
        while True:
            t = self.peek()
            if t.kind == "eof":
                if not top:
                    self.error(tr("неожиданный конец файла: не хватает '};'"), t)
                return entries
            if t.kind == "sym" and t.text == "}":
                if top:
                    self.error(tr("лишняя закрывающая скобка '}'"), t)
                return entries
            if t.kind == "sym" and t.text == ";":
                self.issues.append(Issue("warning", tr("лишняя ';'"), t.file, t.line, t.col, "extra-semicolon"))
                self.next()
                continue
            entries.append(self.parse_entry())

    def parse_entry(self) -> Node:
        t = self.peek()
        if t.kind != "ident":
            self.error(tr("ожидалось имя параметра или 'class', найдено '{0}'").format(t.text), t)
        if t.text == "class":
            return self.parse_class()
        if t.text == "delete":
            self.next()
            name = self.next()
            if name.kind != "ident":
                self.error(tr("после 'delete' ожидалось имя класса"), name)
            self.expect(";", tr("после delete"))
            return DeleteNode(name.text, name.line, name.file)
        if t.text == "enum":
            return self.parse_enum()
        return self.parse_property()

    def parse_class(self) -> ClassNode:
        self.next()  # class
        name = self.next()
        if name.kind != "ident":
            self.error(tr("после 'class' ожидалось имя класса"), name)
        node = ClassNode(name.text, name.line, name.file)
        t = self.peek()
        if t.kind == "sym" and t.text == ";":
            self.next()
            node.extern = True
            return node
        if t.kind == "sym" and t.text == ":":
            self.next()
            b = self.next()
            if b.kind != "ident":
                self.error(tr("после ':' ожидалось имя базового класса"), b)
            node.base = b.text
        t = self.peek()
        if not (t.kind == "sym" and t.text == "{"):
            found = tr("конец файла") if t.kind == "eof" else f"'{t.text}'"
            self.error(tr("после 'class {0}' ожидалось '{{', ':' или ';', найдено {1}").format(node.name, found), t)
        self.next()
        node.entries = self.parse_body()
        self.expect("}")
        t = self.peek()
        if t.kind == "sym" and t.text == ";":
            self.next()
        elif t.kind == "sym" and t.text == "}" or t.kind == "eof" or (t.kind == "ident" and t.text == "class"):
            # инструменты BI это прощают (так написано и в официальных примерах DayZ)
            self.issues.append(Issue("info", tr("после '}}' класса {0} нет ';' (правильно '}};')").format(node.name),
                                     t.file, t.line, t.col, "class-semicolon"))
        else:
            self.error(tr("после '}}' класса {0} нужна ';' (пишется '}};')").format(node.name), t)
        return node

    def parse_enum(self) -> Node:
        t = self.next()
        self.expect("{", tr("после enum"))
        while not (self.peek().kind == "sym" and self.peek().text == "}"):
            if self.peek().kind == "eof":
                self.error(tr("незакрытый enum"))
            self.next()
        self.next()
        self.expect(";", tr("после enum"))
        self.issues.append(Issue("info", tr("enum пропущен (не влияет на проверку)"), t.file, t.line))
        return ValueNode("__enum__", t.line, t.file, 0)

    def parse_property(self) -> Node:
        name = self.next()
        is_array = False
        t = self.peek()
        if t.kind == "sym" and t.text == "[":
            self.next()
            self.expect("]", tr("в объявлении массива {0}[]").format(name.text))
            is_array = True
        t = self.peek()
        if t.kind == "sym" and t.text == "+=":
            if not is_array:
                self.error(tr("'+=' можно применять только к массиву ({0}[] += {{...}})").format(name.text), t)
            self.next()
            arr = self.parse_array()
            self.expect(";", tr("после массива {0}[]").format(name.text))
            return ArrayNode(name.text, name.line, name.file, arr, append=True)
        if not (t.kind == "sym" and t.text == "="):
            found = tr("конец файла") if t.kind == "eof" else f"'{t.text}'"
            hint = tr(" (возможно, пропущена ';' в предыдущей строке)") if t.line != name.line else ""
            self.error(tr("после '{0}' ожидалось '='{1}, найдено {2}").format(name.text, hint, found), t)
        self.next()
        if is_array:
            t = self.peek()
            if not (t.kind == "sym" and t.text == "{"):
                self.error(tr("массив {0}[] должен присваиваться в фигурных скобках: {{...}}").format(name.text), t)
            arr = self.parse_array()
            self.expect(";", tr("после массива {0}[]").format(name.text))
            return ArrayNode(name.text, name.line, name.file, arr)
        t = self.peek()
        if t.kind == "sym" and t.text == "{":
            self.error(tr("'{0}' — массив, объявите его как {1}[] = {{...}};").format(name.text, name.text), t)
        value, quoted = self.parse_scalar(end=";")
        t = self.peek()
        if not (t.kind == "sym" and t.text == ";"):
            found = tr("конец файла") if t.kind == "eof" else f"'{t.text}'"
            self.error(tr("после значения '{0}' нужна ';', найдено {1}").format(name.text, found), t)
        self.next()
        node = ValueNode(name.text, name.line, name.file, value, quoted)
        if not quoted and isinstance(value, str):
            self.issues.append(Issue("info", tr("значение '{0}' без кавычек: {1} — лучше взять в кавычки").format(name.text, value), name.file, name.line, 0, "unquoted"))
        return node

    def parse_scalar(self, end: str) -> Tuple[Value, bool]:
        t = self.peek()
        if t.kind == "string":
            self.next()
            nt = self.peek()
            if nt.kind not in ("sym", "eof"):
                self.error(tr("лишнее после строки: '{0}' (строки в конфиге экранируют кавычки как \"\")").format(nt.text), nt)
            return t.value, True
        if t.kind == "number":
            nt = self.peek(1)
            if nt.kind == "sym" and nt.text in (";", ",", "}"):
                self.next()
                return t.value, True
        # «голое» значение до ; , или }
        parts, line = [], t.line
        first = t
        prev_end = None
        while True:
            t = self.peek()
            if t.kind == "eof" or (t.kind == "sym" and t.text in (";", ",", "}", "{")):
                break
            if t.line != line:
                self.error(tr("значение не закрыто: пропущена ';'") if end == ";" else
                           tr("в массиве пропущена ',' или '}'"), t)
            if prev_end is not None and t.col > prev_end and first.kind == "number":
                self.error(tr("пропущена ',' между значениями массива") if end == "}" else
                           tr("лишнее после числа {0}: '{1}' (пропущена ';'?)").format(first.text, t.text), t)
            parts.append(t.text)
            prev_end = t.col + len(t.text)
            self.next()
        if not parts:
            self.error(tr("пропущено значение"), t)
        return " ".join(parts) if len(parts) > 1 and all(p.isalnum() for p in parts) else "".join(parts), False

    def parse_array(self) -> list:
        self.expect("{")
        items: list = []
        while True:
            t = self.peek()
            if t.kind == "sym" and t.text == "}":
                self.next()
                return items
            if t.kind == "eof":
                self.error(tr("незакрытый массив: нет '}'"), t)
            if t.kind == "sym" and t.text == "{":
                items.append(self.parse_array())
            else:
                v, _ = self.parse_scalar(end="}")
                items.append(v)
            t = self.peek()
            if t.kind == "sym" and t.text == ",":
                self.next()
                t2 = self.peek()
                if t2.kind == "sym" and t2.text == "}":
                    self.issues.append(Issue("info", tr("лишняя запятая перед '}' в массиве"),
                                             t.file, t.line, t.col, "trailing-comma"))
                continue
            if t.kind == "sym" and t.text == "}":
                continue
            found = tr("конец файла") if t.kind == "eof" else f"'{t.text}'"
            self.error(tr("в массиве ожидалась ',' или '}}', найдено {0}").format(found), t)


# --------------------------------------------------------------------------------------
# Загрузка и проверки
# --------------------------------------------------------------------------------------

def parse_config(path, text: Optional[str] = None, include_dirs: Optional[List[Path]] = None
                 ) -> Tuple[ClassNode, List[Issue]]:
    """Разбирает config.cpp. Бросает ConfigError при синтаксической ошибке."""
    path = Path(path)
    pp = Preprocessor(include_dirs)
    lines = pp.run(path, text)
    parser = Parser(tokenize(lines))
    root = parser.parse()
    return root, pp.issues + parser.issues


class _Ctx:
    def __init__(self, node: ClassNode, parent: Optional["_Ctx"]):
        self.node = node
        self.parent = parent
        self.scope: Dict[str, ClassNode] = {}   # классы, объявленные выше в этой области


def semantic_check(root: ClassNode, file: str = "") -> List[Issue]:
    issues: List[Issue] = []
    ctx_of: Dict[int, _Ctx] = {}

    def members(node: ClassNode, name: str, seen: set, exclude: Optional[ClassNode] = None) -> str:
        """Ищет класс name среди членов node и его базовых классов: found | unknown | missing."""
        if id(node) in seen:
            return "missing"
        seen.add(id(node))
        for e in node.entries:
            if isinstance(e, ClassNode) and e.name.lower() == name and e is not exclude:
                return "found"
        if not node.base:
            return "unknown" if node.extern else "missing"
        ctx = ctx_of.get(id(node))
        base = resolve(ctx.parent if ctx else None, node.base.lower(), exclude=node)
        if base is None:
            return "unknown"
        return members(base, name, seen)

    def resolve(ctx: Optional[_Ctx], name: str, exclude: Optional[ClassNode] = None) -> Optional[ClassNode]:
        c = ctx
        while c is not None:
            n = c.scope.get(name)
            if n is not None and n is not exclude:
                return n
            c = c.parent
        return None

    def lookup(ctx: _Ctx, name: str, exclude: ClassNode) -> str:
        c: Optional[_Ctx] = ctx
        state = "missing"
        while c is not None:
            n = c.scope.get(name)
            if n is not None and n is not exclude:
                return "found"
            if c.node.base:
                base = resolve(c.parent, c.node.base.lower(), exclude=c.node)
                r = members(base, name, set(), exclude) if base is not None else "unknown"
                if r == "found":
                    return "found"
                if r == "unknown":
                    state = "unknown"
            c = c.parent
        return state

    def walk(ctx: _Ctx) -> None:
        props: Dict[str, Node] = {}
        for e in ctx.node.entries:
            if isinstance(e, ClassNode):
                key = e.name.lower()
                if e.extern:
                    ctx.scope.setdefault(key, e)
                    continue
                if e.base:
                    st = lookup(ctx, e.base.lower(), exclude=e)
                    if st == "missing":
                        issues.append(Issue(
                            "error", tr("базовый класс '{0}' для '{1}' не объявлен — добавьте выше 'class {2};'").format(e.base, e.name, e.base), e.file or file, e.line, 0,
                            "undefined-base"))
                    elif st == "unknown":
                        issues.append(Issue(
                            "info", tr("базовый класс '{0}' для '{1}' должен прийти из внешнего аддона (здесь не объявлен)").format(e.base, e.name), e.file or file, e.line, 0, "external-base"))
                prev = ctx.scope.get(key)
                if prev is not None and not prev.extern:
                    issues.append(Issue("error", tr("класс '{0}' уже определён в этой области (строка {1})").format(e.name, prev.line), e.file or file, e.line, 0,
                                        "duplicate-class"))
                child = _Ctx(e, ctx)
                ctx_of[id(e)] = child
                ctx.scope[key] = e
                walk(child)
            elif isinstance(e, (ValueNode, ArrayNode)):
                if e.name == "__enum__" or (isinstance(e, ArrayNode) and e.append):
                    continue
                key = e.name.lower()
                if key in props:
                    issues.append(Issue("warning", tr("параметр '{0}' задан повторно (строка {1})").format(e.name, props[key].line), e.file or file, e.line, 0,
                                        "duplicate-property"))
                props[key] = e

    rctx = _Ctx(root, None)
    ctx_of[id(root)] = rctx
    walk(rctx)

    # CfgPatches
    patches = [e for e in root.entries if isinstance(e, ClassNode) and e.name.lower() == "cfgpatches"]
    has_classes = any(isinstance(e, ClassNode) for e in root.entries)
    if has_classes and not patches:
        issues.append(Issue("warning", tr("нет класса CfgPatches — аддон не будет зарегистрирован"), file, 1, 0,
                            "no-cfgpatches"))
    for p in patches:
        for c in p.entries:
            if isinstance(c, ClassNode) and not c.extern:
                names = {x.name.lower() for x in c.entries}
                for req in ("units", "weapons", "requiredaddons"):
                    if req not in names:
                        issues.append(Issue("warning" if req == "requiredaddons" else "info",
                                            tr("в CfgPatches/{0} нет {1}[]").format(c.name, req),
                                            c.file or file, c.line, 0, "cfgpatches"))
                for x in c.entries:
                    if x.name.lower() in ("units", "weapons", "requiredaddons") and not isinstance(x, ArrayNode):
                        issues.append(Issue("error", tr("CfgPatches/{0}: {1} должен быть массивом []").format(c.name, x.name),
                                            x.file or file, x.line, 0, "cfgpatches"))
    return issues


def iter_strings(node: ClassNode, path: str = ""):
    """Обходит все строковые значения: (путь_к_параметру, строка, узел)."""
    for e in node.entries:
        if isinstance(e, ClassNode):
            yield from iter_strings(e, f"{path}/{e.name}" if path else e.name)
        elif isinstance(e, ValueNode) and isinstance(e.value, str):
            yield f"{path}/{e.name}", e.value, e
        elif isinstance(e, ArrayNode):
            stack = [e.value]
            while stack:
                arr = stack.pop()
                for v in arr:
                    if isinstance(v, list):
                        stack.append(v)
                    elif isinstance(v, str):
                        yield f"{path}/{e.name}[]", v, e


# --------------------------------------------------------------------------------------
# Запись в текст
# --------------------------------------------------------------------------------------

def format_float(v: float) -> str:
    """Кратчайшее представление, которое даёт тот же float32."""
    f32 = struct.unpack("<f", struct.pack("<f", v))[0]
    for p in range(1, 10):
        s = "%.*g" % (p, f32)
        if struct.pack("<f", float(s)) == struct.pack("<f", f32):
            break
    if "e" not in s and "." not in s and "inf" not in s and "nan" not in s:
        s += ".0" if abs(f32) >= 1e15 else ""
    return s


def format_value(v: Value) -> str:
    if isinstance(v, list):
        return "{" + ", ".join(format_value(x) for x in v) + "}"
    if isinstance(v, bool):
        return "1" if v else "0"
    if isinstance(v, int):
        return str(v)
    if isinstance(v, float):
        s = format_float(v)
        return s
    return '"' + str(v).replace('"', '""') + '"'


def to_text(root: ClassNode, indent: str = "\t") -> str:
    out: List[str] = []

    def emit(cls: ClassNode, level: int) -> None:
        pad = indent * level
        for e in cls.entries:
            if isinstance(e, ClassNode):
                if e.extern:
                    out.append(f"{pad}class {e.name};")
                    continue
                head = f"{pad}class {e.name}" + (f": {e.base}" if e.base else "")
                if not e.entries:
                    out.append(head + " {};")
                    continue
                out.append(head)
                out.append(pad + "{")
                emit(e, level + 1)
                out.append(pad + "};")
            elif isinstance(e, DeleteNode):
                out.append(f"{pad}delete {e.name};")
            elif isinstance(e, ArrayNode):
                op = "+=" if e.append else "="
                out.append(f"{pad}{e.name}[] {op} {format_value(e.value)};")
            elif isinstance(e, ValueNode):
                if e.name == "__enum__":
                    continue
                out.append(f"{pad}{e.name} = {format_value(e.value)};")

    emit(root, 0)
    return "\n".join(out) + "\n"
