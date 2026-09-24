# -*- coding: utf-8 -*-
"""Проверка скриптов Enforce Script (.c) на типичные синтаксические ошибки.

Это не компилятор, а быстрый анализатор, который ловит то, из-за чего DayZ
не запускается или падает с непонятной ошибкой:
  * незакрытые строки и комментарии;
  * несбалансированные (), [], {} — с указанием, где открыта парная скобка;
  * несбалансированные #ifdef/#ifndef/#else/#endif;
  * пропущенная ';' в конце оператора или ',' в enum;
  * пустое тело после if/while/for (лишняя ';');
  * присваивание '=' вместо сравнения '==' в условии;
  * повторное объявление класса в одном файле.
"""

from __future__ import annotations

import re
from dataclasses import dataclass
from pathlib import Path
from typing import List, Optional

from .cfg import Issue, read_text

KEYWORDS_CONT = {
    # после этих слов оператор продолжается на следующей строке
    "return", "else", "new", "class", "extends", "modded", "ref", "autoptr", "const", "static",
    "override", "private", "protected", "proto", "native", "external", "owned", "out", "inout",
    "notnull", "event", "volatile", "typedef", "enum", "sealed", "reference", "local", "case",
    "delete", "thread", "func", "typename", "auto", "void", "do", "switch", "if", "while", "for",
    "foreach", "goto", "vararg", "Obsolete",
}
CONTROL = {"if", "while", "for", "foreach", "switch"}
STATEMENT_END = {";", "{", "}", ":"}
OPERATORS = sorted([
    "<<=", ">>=", "==", "!=", "<=", ">=", "&&", "||", "++", "--", "+=", "-=", "*=", "/=", "%=",
    "&=", "|=", "^=", "<<", ">>", "::",
], key=len, reverse=True)


@dataclass
class Tok:
    kind: str   # ident | number | string | op | eof
    text: str
    line: int
    col: int


class _Lexer:
    def __init__(self, text: str, file: str, issues: List[Issue]):
        self.s = text
        self.file = file
        self.issues = issues

    def run(self) -> List[Tok]:
        s, n = self.s, len(self.s)
        toks: List[Tok] = []
        i, line, lstart = 0, 1, 0
        pp_stack: List[tuple] = []
        at_line_start = True
        while i < n:
            ch = s[i]
            if ch == "\n":
                line += 1
                lstart = i + 1
                i += 1
                at_line_start = True
                continue
            if ch in " \t\r\f\v":
                i += 1
                continue
            col = i - lstart + 1
            if ch == "#" and at_line_start:
                j = s.find("\n", i)
                j = n if j < 0 else j
                directive = s[i:j].strip()
                m = re.match(r"#\s*(\w+)", directive)
                d = m.group(1) if m else ""
                if d in ("ifdef", "ifndef", "if"):
                    pp_stack.append((d, line, col))
                elif d == "else":
                    if not pp_stack:
                        self.issues.append(Issue("error", "#else без #ifdef/#ifndef", self.file, line, col,
                                                 "preprocessor"))
                elif d == "endif":
                    if not pp_stack:
                        self.issues.append(Issue("error", "#endif без #ifdef/#ifndef", self.file, line, col,
                                                 "preprocessor"))
                    else:
                        pp_stack.pop()
                elif d not in ("define", "undef", "include", "else", "elif", "pragma", "line"):
                    self.issues.append(Issue("warning", f"неизвестная директива препроцессора '{directive}'",
                                             self.file, line, col, "preprocessor"))
                i = j
                continue
            at_line_start = False
            if s.startswith("//", i):
                j = s.find("\n", i)
                i = n if j < 0 else j
                continue
            if s.startswith("/*", i):
                j = s.find("*/", i + 2)
                if j < 0:
                    self.issues.append(Issue("error", "незакрытый комментарий /* (нет */)", self.file, line, col,
                                             "comment"))
                    return toks + [Tok("eof", "", line, col)]
                chunk = s[i:j + 2]
                nl = chunk.count("\n")
                if nl:
                    line += nl
                    lstart = i + chunk.rfind("\n") + 1
                i = j + 2
                continue
            if ch in "\"'":
                j = i + 1
                closed = False
                while j < n:
                    c = s[j]
                    if c == "\\":
                        j += 2
                        continue
                    if c == ch:
                        closed = True
                        break
                    if c == "\n":
                        break
                    j += 1
                if not closed:
                    self.issues.append(Issue("error", f"незакрытая строка (нет закрывающей {ch})", self.file,
                                             line, col, "string"))
                    j = s.find("\n", i)
                    j = n if j < 0 else j
                    toks.append(Tok("string", s[i:j], line, col))
                    i = j
                    continue
                toks.append(Tok("string", s[i:j + 1], line, col))
                i = j + 1
                continue
            if ch.isdigit() or (ch == "." and i + 1 < n and s[i + 1].isdigit()):
                m = re.compile(r"0[xX][0-9A-Fa-f]+|\d*\.?\d+(?:[eE][+-]?\d+)?[fF]?").match(s, i)
                toks.append(Tok("number", m.group(0), line, col))
                i = m.end()
                continue
            if ch.isalpha() or ch == "_":
                m = re.compile(r"[A-Za-z0-9_]+").match(s, i)
                toks.append(Tok("ident", m.group(0), line, col))
                i = m.end()
                continue
            for op in OPERATORS:
                if s.startswith(op, i):
                    toks.append(Tok("op", op, line, col))
                    i += len(op)
                    break
            else:
                toks.append(Tok("op", ch, line, col))
                i += 1
        for d, l, c in pp_stack:
            self.issues.append(Issue("error", f"#{d} без закрывающего #endif", self.file, l, c, "preprocessor"))
        toks.append(Tok("eof", "", line, 1))
        return toks


PAIRS = {"(": ")", "[": "]", "{": "}"}
CLOSERS = {v: k for k, v in PAIRS.items()}


def check_script(path, text: Optional[str] = None) -> List[Issue]:
    path = Path(path)
    file = str(path)
    issues: List[Issue] = []
    if text is None:
        try:
            text = read_text(path)
        except OSError as e:
            return [Issue("error", f"не удалось прочитать файл: {e}", file)]
    toks = _Lexer(text, file, issues).run()

    # --- баланс скобок ---
    stack: List[tuple] = []  # (tok, kind, control)
    brace_kind: List[str] = []
    balanced = True
    for idx, t in enumerate(toks):
        if t.kind != "op":
            continue
        if t.text in PAIRS:
            stack.append((t, idx))
        elif t.text in CLOSERS:
            if not stack:
                issues.append(Issue("error", f"лишняя закрывающая скобка '{t.text}'", file, t.line, t.col,
                                    "brackets"))
                balanced = False
                continue
            open_t, _ = stack[-1]
            if PAIRS[open_t.text] != t.text:
                issues.append(Issue("error", f"'{t.text}' не соответствует '{open_t.text}' "
                                             f"(открыта в строке {open_t.line})", file, t.line, t.col, "brackets"))
                balanced = False
                # попытка восстановиться: ищем подходящую открывающую ниже по стеку
                for k in range(len(stack) - 1, -1, -1):
                    if PAIRS[stack[k][0].text] == t.text:
                        del stack[k:]
                        break
                continue
            stack.pop()
    for open_t, _ in stack:
        issues.append(Issue("error", f"не закрыта скобка '{open_t.text}' (нет '{PAIRS[open_t.text]}')",
                            file, open_t.line, open_t.col, "brackets"))
        balanced = False

    if balanced:
        issues += _statement_checks(toks, file)
    issues += _class_checks(toks, file)
    issues.sort(key=lambda i: (i.line, i.col))
    return issues


def _brace_context(toks: List[Tok], i: int) -> str:
    """Определяет тип блока, открываемого '{' с индексом i."""
    j = i - 1
    if j < 0:
        return "code"
    p = toks[j]
    if p.text in ("=", ",", "return", "(", "{") and p.kind in ("op", "ident"):
        if p.text == "{":
            # вложенный блок: если внешний — инициализатор, это тоже инициализатор
            return "nested"
        return "init"
    # ищем начало заголовка: назад до ; { } на том же уровне
    k = j
    depth = 0
    while k >= 0:
        t = toks[k]
        if t.kind == "op" and t.text in (")", "]"):
            depth += 1
        elif t.kind == "op" and t.text in ("(", "["):
            depth -= 1
        elif depth == 0 and t.kind == "op" and t.text in (";", "{", "}"):
            break
        k -= 1
    header = [t.text for t in toks[k + 1:i]]
    if "enum" in header:
        return "enum"
    if "class" in header and "(" not in header:
        return "class"
    return "code"


def _statement_checks(toks: List[Tok], file: str) -> List[Issue]:
    issues: List[Issue] = []
    ctx: List[str] = ["class"]           # верхний уровень файла ведёт себя как тело класса
    parens: List[tuple] = []             # (idx_open, is_control, keyword)
    control_close = set()                # индексы ')' закрывающих условие if/while/...
    stmt_start = True
    stmt_first_idx = 0

    for i, t in enumerate(toks):
        if t.kind == "eof":
            break
        prev = toks[i - 1] if i else None

        # --- проверка пропущенной ';' / ',' при переходе на новую строку ---
        if prev is not None and t.line > prev.line and not parens and ctx:
            kind = ctx[-1]
            prev_ok = (prev.kind in ("ident", "number", "string") and prev.text not in KEYWORDS_CONT) or \
                      (prev.kind == "op" and prev.text in (")", "]") and (i - 1) not in control_close)
            if prev.kind == "string" and (len(prev.text) < 2 or prev.text[-1] != prev.text[0]):
                prev_ok = False  # незакрытая строка — ошибка уже выдана
            if prev.kind == "op" and prev.text == "]" and toks[stmt_first_idx].text == "[":
                prev_ok = False  # атрибут [Attr()] перед объявлением
            next_ok = (t.kind == "ident" and t.text not in ("else", "extends", "catch")) or \
                      (t.kind == "op" and t.text == "}")
            if prev_ok and next_ok:
                if kind in ("code", "class"):
                    issues.append(Issue("error", "пропущена ';' в конце строки", file, prev.line,
                                        prev.col + len(prev.text), "missing-semicolon"))
                elif kind == "enum" and t.text != "}":
                    issues.append(Issue("error", "в enum пропущена ',' между значениями", file, prev.line,
                                        prev.col + len(prev.text), "missing-comma"))

        if t.kind == "op":
            if t.text == "(":
                kw = prev.text if prev is not None and prev.kind == "ident" else ""
                parens.append((i, kw in CONTROL and not parens, kw))
            elif t.text == ")" and parens:
                open_idx, is_ctrl, kw = parens.pop()
                if is_ctrl:
                    control_close.add(i)
                    nxt = toks[i + 1] if i + 1 < len(toks) else None
                    if nxt is not None and nxt.text == ";" and kw in ("if", "while", "for", "foreach"):
                        issues.append(Issue("warning", f"';' сразу после {kw}(...) — тело условия/цикла пустое",
                                            file, nxt.line, nxt.col, "empty-body"))
                    if kw in ("if", "while"):
                        _check_assign_in_cond(toks, open_idx, i, kw, file, issues)
            elif t.text == "{":
                if parens:
                    continue
                c = _brace_context(toks, i)
                if c == "nested":
                    c = "init" if ctx and ctx[-1] in ("init",) else "code"
                ctx.append(c)
                stmt_start = True
                stmt_first_idx = i + 1
                continue
            elif t.text == "}":
                if not parens and len(ctx) > 1:
                    ctx.pop()
                stmt_start = True
                stmt_first_idx = i + 1
                continue
            elif t.text in (";", ":") and not parens:
                stmt_start = True
                stmt_first_idx = i + 1
                continue
        if stmt_start:
            stmt_first_idx = i
            stmt_start = False
    return issues


def _check_assign_in_cond(toks: List[Tok], a: int, b: int, kw: str, file: str, issues: List[Issue]) -> None:
    depth = 0
    for k in range(a + 1, b):
        t = toks[k]
        if t.kind == "op" and t.text in "([{" and len(t.text) == 1:
            depth += 1
        elif t.kind == "op" and t.text in ")]}" and len(t.text) == 1:
            depth -= 1
        elif depth == 0 and t.kind == "op" and t.text == "=":
            issues.append(Issue("warning", f"присваивание '=' в условии {kw}(...) — возможно, нужно '=='",
                                file, t.line, t.col, "assign-in-condition"))
            return


def _class_checks(toks: List[Tok], file: str) -> List[Issue]:
    issues: List[Issue] = []
    seen = {}
    depth = 0
    for i, t in enumerate(toks):
        if t.kind == "op" and t.text == "{":
            depth += 1
        elif t.kind == "op" and t.text == "}":
            depth -= 1
        elif t.kind == "ident" and t.text == "class" and depth == 0:
            if i + 1 < len(toks) and toks[i + 1].kind == "ident":
                name = toks[i + 1].text
                modded = i > 0 and toks[i - 1].text == "modded"
                # предварительное объявление 'class X;' не считается
                nxt = toks[i + 2] if i + 2 < len(toks) else None
                if nxt is not None and nxt.text == ";":
                    continue
                key = (name, modded)
                if key in seen and not modded:
                    issues.append(Issue("error", f"класс '{name}' объявлен в файле повторно "
                                                 f"(первый раз в строке {seen[key]})", file, toks[i + 1].line,
                                        toks[i + 1].col, "duplicate-class"))
                seen.setdefault(key, toks[i + 1].line)
    return issues
