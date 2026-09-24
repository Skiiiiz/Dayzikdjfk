# -*- coding: utf-8 -*-
"""Вкладки «Запуск» (игра/сервер + логи) и «Редактор types.xml»."""

from __future__ import annotations

from dztk.i18n import tr

import os
import shutil
import subprocess
import time
from pathlib import Path
from typing import List, Optional

import tkinter as tk
from tkinter import filedialog, messagebox, ttk

from . import APP_NAME, launch
from . import typesedit as te
from .common import open_folder
from .gui import LEVEL_NAMES, MUTED, Tab, labeled


def open_at(path: Path, line: int = 0) -> None:
    """Открыть файл на строке (VS Code / Notepad++), иначе программой по умолчанию."""
    code = shutil.which("code")
    npp = shutil.which("notepad++")
    try:
        if code:
            subprocess.Popen([code, "-g", f"{path}:{max(1, line)}"])
        elif npp:
            subprocess.Popen([npp, f"-n{max(1, line)}", str(path)])
        else:
            open_folder(path)
    except Exception:
        open_folder(path)


def _path_row(parent, var: tk.StringVar, title: str, file: bool = False):
    f = ttk.Frame(parent)
    ttk.Entry(f, textvariable=var).pack(side="left", fill="x", expand=True)

    def pick():
        d = filedialog.askopenfilename(title=title) if file else filedialog.askdirectory(title=title)
        if d:
            var.set(d)
    ttk.Button(f, text="...", width=3, command=pick).pack(side="left")
    return f


# --------------------------------------------------------------------------------------
# Вкладка «Запуск»
# --------------------------------------------------------------------------------------

class LaunchTab(Tab):
    title = tr("  Запуск  ")

    def __init__(self, nb, app):
        super().__init__(nb, app)
        st = app.settings.get("launch", {})
        self.columnconfigure(0, weight=1)
        self.rowconfigure(1, weight=1)
        self.procs: List[subprocess.Popen] = []
        self.entries: List[launch.LogEntry] = []
        self.tail: Optional[launch.LogTail] = None

        top = ttk.Frame(self)
        top.grid(row=0, column=0, sticky="ew")
        top.columnconfigure(1, weight=1)
        top.columnconfigure(3, weight=1)
        self.v_game = tk.StringVar(value=st.get("game", ""))
        self.v_server = tk.StringVar(value=st.get("server", ""))
        self.v_smods = tk.StringVar(value=st.get("server_mods", ""))
        self.v_mission = tk.StringVar(value=st.get("mission", ""))
        self.v_config = tk.StringVar(value=st.get("config", "serverDZ.cfg"))
        self.v_profiles = tk.StringVar(value=st.get("profiles", ""))
        self.v_cprofiles = tk.StringVar(value=st.get("client_profiles", ""))
        self.v_port = tk.StringVar(value=str(st.get("port", 2302)))
        self.v_fp = tk.BooleanVar(value=st.get("file_patching", True))
        self.v_win = tk.BooleanVar(value=st.get("windowed", True))
        self.v_errors = tk.BooleanVar(value=st.get("errors_only", False))
        self.v_follow = tk.BooleanVar(value=True)

        def row(r, c, label, widget):
            ttk.Label(top, text=label).grid(row=r, column=c, sticky="w", padx=(8 if c else 0, 6), pady=2)
            widget.grid(row=r, column=c + 1, sticky="ew", pady=2)

        row(0, 0, tr("Папка DayZ"), _path_row(top, self.v_game, tr("Папка DayZ")))
        row(0, 2, tr("Папка DayZServer"), _path_row(top, self.v_server, tr("Папка DayZServer")))
        row(1, 0, tr("Миссия (-mission)"), ttk.Entry(top, textvariable=self.v_mission))
        row(1, 2, tr("Конфиг сервера"), ttk.Entry(top, textvariable=self.v_config))
        row(2, 0, tr("Профили сервера"), _path_row(top, self.v_profiles, tr("Папка профилей сервера")))
        row(2, 2, tr("Профили клиента"), _path_row(top, self.v_cprofiles, tr("Папка профилей клиента")))
        row(3, 0, tr("Серверные моды"), ttk.Entry(top, textvariable=self.v_smods))
        pf = ttk.Frame(top)
        ttk.Entry(pf, textvariable=self.v_port, width=7).pack(side="left")
        ttk.Checkbutton(pf, text="-filePatching", variable=self.v_fp).pack(side="left", padx=8)
        ttk.Checkbutton(pf, text=tr("В окне"), variable=self.v_win).pack(side="left")
        ttk.Button(pf, text=tr("Найти в Steam"), command=self.detect).pack(side="right")
        row(3, 2, tr("Порт"), pf)

        mf = ttk.LabelFrame(top, text=tr("Моды (-mod), по порядку загрузки"), padding=4)
        mf.grid(row=4, column=0, columnspan=4, sticky="ew", pady=(4, 0))
        mf.columnconfigure(0, weight=1)
        self.mods = tk.Listbox(mf, height=3, activestyle="none")
        self.mods.grid(row=0, column=0, sticky="ew")
        for m in st.get("mods", []):
            self.mods.insert("end", m)
        mb = ttk.Frame(mf)
        mb.grid(row=0, column=1, sticky="n", padx=(6, 0))
        ttk.Button(mb, text=tr("+ Папка @мода"), command=self.add_mod).grid(row=0, column=0, sticky="ew")
        ttk.Button(mb, text=tr("Убрать"), command=self.remove_mod).grid(row=0, column=1, sticky="ew")
        ttk.Button(mb, text="↑", width=3, command=lambda: self.move(-1)).grid(row=1, column=0, sticky="w")
        ttk.Button(mb, text="↓", width=3, command=lambda: self.move(1)).grid(row=1, column=1, sticky="w")

        bf = ttk.Frame(top)
        bf.grid(row=5, column=0, columnspan=4, sticky="ew", pady=(4, 0))
        ttk.Button(bf, text=tr("▶ Сервер + игра"), command=lambda: self.run("both")).pack(side="left")
        ttk.Button(bf, text=tr("Сервер"), command=lambda: self.run("server")).pack(side="left", padx=4)
        ttk.Button(bf, text=tr("Игра"), command=lambda: self.run("client")).pack(side="left")
        ttk.Button(bf, text=tr("■ Остановить"), command=self.stop).pack(side="left", padx=4)
        ttk.Button(bf, text=tr("Командная строка"), command=self.show_cmd).pack(side="left")
        lf = ttk.Frame(top)
        lf.grid(row=6, column=0, columnspan=4, sticky="ew", pady=4)
        ttk.Checkbutton(lf, text=tr("Следить за логами"), variable=self.v_follow,
                        command=self.toggle_follow).pack(side="left")
        ttk.Checkbutton(lf, text=tr("Только ошибки"), variable=self.v_errors, command=self.fill).pack(side="left",
                                                                                                 padx=8)
        ttk.Button(lf, text=tr("Последние логи"), command=self.load_latest).pack(side="right")
        ttk.Button(lf, text=tr("Открыть папку логов"), command=self.open_logs).pack(side="right", padx=4)

        rf = ttk.Frame(self)
        rf.grid(row=1, column=0, sticky="nsew")
        rf.columnconfigure(0, weight=1)
        rf.rowconfigure(0, weight=1)
        self.tree = ttk.Treeview(rf, columns=("time", "level", "where", "msg"), show="headings")
        for col, text, w, s in (("time", tr("Время"), 90, False), ("level", tr("Уровень"), 75, False),
                                ("where", tr("Файл:строка"), 330, True), ("msg", tr("Сообщение"), 520, True)):
            self.tree.heading(col, text=text)
            self.tree.column(col, width=w, stretch=s, anchor="w")
        self.tree.grid(row=0, column=0, sticky="nsew")
        sb = ttk.Scrollbar(rf, orient="vertical", command=self.tree.yview)
        sb.grid(row=0, column=1, sticky="ns")
        self.tree.configure(yscrollcommand=sb.set)
        self.tree.tag_configure("error", foreground="#c62828")
        self.tree.tag_configure("warning", foreground="#b26a00")
        self.tree.tag_configure("info", foreground=MUTED)
        self.tree.bind("<Double-1>", self.open_selected)
        self.summary = ttk.Label(self, foreground=MUTED, text=tr(
            "Двойной щелчок — открыть файл мода на строке с ошибкой (исходники модов берутся из вкладки "
            "«Проверка ошибок» и списка модов)."))
        self.summary.grid(row=2, column=0, sticky="w", pady=(4, 0))

    # --- настройки ---
    def save(self):
        return {"game": self.v_game.get(), "server": self.v_server.get(), "server_mods": self.v_smods.get(),
                "mission": self.v_mission.get(), "config": self.v_config.get(), "profiles": self.v_profiles.get(),
                "client_profiles": self.v_cprofiles.get(), "port": self._port(), "file_patching": self.v_fp.get(),
                "windowed": self.v_win.get(), "errors_only": self.v_errors.get(),
                "mods": list(self.mods.get(0, "end"))}

    def _port(self) -> int:
        try:
            return int(self.v_port.get())
        except ValueError:
            return 2302

    def options(self, what: str) -> launch.LaunchOptions:
        return launch.LaunchOptions(
            game_dir=self.v_game.get().strip(), server_dir=self.v_server.get().strip(),
            mods=list(self.mods.get(0, "end")),
            server_mods=[m.strip() for m in self.v_smods.get().split(";") if m.strip()],
            profiles=self.v_profiles.get().strip() or str(Path(self.v_server.get() or ".") / "profiles"),
            client_profiles=self.v_cprofiles.get().strip(), server_config=self.v_config.get().strip(),
            mission=self.v_mission.get().strip(), port=self._port(), file_patching=self.v_fp.get(),
            windowed=self.v_win.get(), connect=what == "both")

    def log_dirs(self) -> List[Path]:
        o = self.options("both")
        dirs = [Path(o.client_profiles) if o.client_profiles else launch.default_client_profiles()]
        if o.profiles:
            p = Path(o.profiles)
            dirs.append(p if p.is_absolute() else Path(o.server_dir or o.game_dir or ".") / p)
        return dirs

    # --- моды ---
    def add_mod(self):
        d = filedialog.askdirectory(title=tr("Папка мода (@MyMod с addons внутри)"))
        if d:
            self.mods.insert("end", d)

    def remove_mod(self):
        for i in reversed(self.mods.curselection()):
            self.mods.delete(i)

    def move(self, delta: int):
        sel = self.mods.curselection()
        if not sel:
            return
        i = sel[0]
        j = i + delta
        if 0 <= j < self.mods.size():
            v = self.mods.get(i)
            self.mods.delete(i)
            self.mods.insert(j, v)
            self.mods.selection_set(j)

    def detect(self):
        found = launch.find_game_dirs()
        if found.get("client"):
            self.v_game.set(str(found["client"]))
        if found.get("server"):
            self.v_server.set(str(found["server"]))
        msg = ", ".join(f"{k}: {v}" for k, v in found.items() if v) or tr("DayZ в библиотеках Steam не найден.")
        self.app.log(msg)

    # --- запуск ---
    def commands(self, what: str):
        o = self.options(what)
        out = []
        if what in ("server", "both"):
            out.append(("server", launch.server_command(o), launch.check_options(o, True)))
        if what in ("client", "both"):
            out.append(("client", launch.client_command(o), launch.check_options(o, False)))
        return out

    def show_cmd(self):
        text = "\n\n".join(f"[{k}]\n{subprocess.list2cmdline(c)}" + "".join("\n  ! " + p for p in probs)
                           for k, c, probs in self.commands("both"))
        self.app.show_text(tr("Командная строка"), text)

    def run(self, what: str):
        cmds = self.commands(what)
        probs = [p for _, _, ps in cmds for p in ps]
        if probs and not messagebox.askyesno(APP_NAME, tr("Возможны проблемы:\n\n{0}\n\nВсё равно запустить?").format(
                "\n".join("• " + p for p in probs))):
            return
        self.app.save_settings()
        self.start_follow()
        for kind, cmd, _ in cmds:
            try:
                self.procs.append(launch.start(cmd))
                self.app.log(f"{kind}: {subprocess.list2cmdline(cmd)}")
            except OSError as e:
                messagebox.showerror(APP_NAME, tr("Не удалось запустить {0}: {1}").format(kind, e))
                return
            if kind == "server" and what == "both":
                self.app.status(tr("Сервер запускается, игра стартует через 10 секунд..."))
                rest = [c for c in cmds if c[0] == "client"]
                if rest:
                    self.after(10000, lambda c=rest[0][1]: self._start_client(c))
                break

    def _start_client(self, cmd):
        try:
            self.procs.append(launch.start(cmd))
            self.app.log(f"client: {subprocess.list2cmdline(cmd)}")
        except OSError as e:
            messagebox.showerror(APP_NAME, tr("Не удалось запустить {0}: {1}").format("client", e))

    def stop(self):
        for p in self.procs:
            if p.poll() is None:
                try:
                    p.terminate()
                except OSError:
                    pass
        self.procs.clear()
        self.app.status(tr("Остановлено."))

    # --- логи ---
    def toggle_follow(self):
        if self.v_follow.get() and self.procs:
            self.start_follow()

    def start_follow(self):
        self.tail = launch.LogTail(self.log_dirs(), since=time.time() - 5)
        self.entries = []
        self.fill()
        self._tick()

    def _tick(self):
        if self.tail is None or not self.v_follow.get():
            return
        new = self.tail.poll()
        if new:
            self.entries += new
            self.fill()
            errs = sum(1 for e in new if e.level == "error")
            if errs:
                self.app.log(tr("В логах новых ошибок: {0}").format(errs), "error")
        alive = any(p.poll() is None for p in self.procs)
        if alive or new:
            self.after(1000, self._tick)
        else:
            self.after(3000, self._tick)

    def load_latest(self):
        files = launch.find_logs(self.log_dirs())
        if not files:
            messagebox.showinfo(APP_NAME, tr("Логи не найдены в:\n{0}").format(
                "\n".join(str(d) for d in self.log_dirs())))
            return
        self.entries = []
        for f in files:
            try:
                self.entries += launch.parse_log(launch.read_log(f), f.name)
            except OSError:
                pass
        self.fill()
        self.app.status(tr("Логи: {0}, записей: {1}").format(", ".join(f.name for f in files), len(self.entries)))

    def open_logs(self):
        for d in self.log_dirs():
            if d.is_dir():
                open_folder(d)
                return

    def fill(self):
        self.tree.delete(*self.tree.get_children())
        self._shown = [e for e in self.entries if not self.v_errors.get() or e.level == "error"]
        for n, e in enumerate(self._shown):
            self.tree.insert("", "end", iid=str(n), values=(e.time, LEVEL_NAMES.get(e.level, e.level), e.where,
                                                            e.message), tags=(e.level,))
        if self._shown:
            self.tree.see(str(len(self._shown) - 1))

    def mod_roots(self) -> List[str]:
        roots = list(getattr(self.app.tabs.get("check"), "paths", []))
        roots += list(self.mods.get(0, "end"))
        return [r for r in roots if Path(r).is_dir()]

    def open_selected(self, _e=None):
        sel = self.tree.selection()
        if not sel:
            return
        e = self._shown[int(sel[0])]
        for f, ln in ([(e.file, e.line)] if e.file else []) + [(s[0], s[1]) for s in e.stack]:
            p = launch.resolve(f, self.mod_roots())
            if p:
                open_at(p, ln)
                return
        for d in self.log_dirs():
            p = d / e.log
            if p.is_file():
                open_at(p, e.log_line)
                return


# --------------------------------------------------------------------------------------
# Вкладка «Редактор types.xml»
# --------------------------------------------------------------------------------------

COLUMNS = ("name", "nominal", "min", "lifetime", "restock", "quantmin", "quantmax", "cost", "flags",
           "category", "usage", "value", "tag")
WIDTHS = {"name": 200, "flags": 70, "usage": 130, "value": 100, "tag": 60, "category": 80, "min": 50, "cost": 50}
OPS = (("set", tr("установить")), ("mul", tr("умножить на")), ("add", tr("прибавить")),
       ("addlist", tr("добавить в список")), ("remlist", tr("убрать из списка")))


def _flags(r: te.TypeRow) -> str:
    return "".join(r.values.get(k, "-") for k in te.FLAG_FIELDS)


class TypesEditTab(Tab):
    title = tr("  Редактор types  ")

    def __init__(self, nb, app):
        super().__init__(nb, app)
        st = app.settings.get("typesedit", {})
        self.columnconfigure(0, weight=1)
        self.rowconfigure(2, weight=1)
        self.tf: Optional[te.TypesFile] = None
        self.shown: List[te.TypeRow] = []
        self.sort_key, self.sort_rev = "", False
        self.editor: Optional[tk.Entry] = None

        ff = ttk.Frame(self)
        ff.grid(row=0, column=0, sticky="ew")
        ff.columnconfigure(1, weight=1)
        self.v_file = tk.StringVar(value=st.get("file", ""))
        ttk.Label(ff, text="types.xml").grid(row=0, column=0, padx=(0, 6))
        ttk.Entry(ff, textvariable=self.v_file).grid(row=0, column=1, sticky="ew")
        ttk.Button(ff, text=tr("Открыть..."), command=self.pick).grid(row=0, column=2, padx=4)
        ttk.Button(ff, text=tr("Перечитать"), command=self.reload).grid(row=0, column=3)
        ttk.Button(ff, text=tr("Сохранить"), command=self.save_file).grid(row=0, column=4, padx=4)
        ttk.Button(ff, text=tr("Сохранить как..."), command=self.save_as).grid(row=0, column=5)

        flt = ttk.Frame(self)
        flt.grid(row=1, column=0, sticky="ew", pady=4)
        self.v_name = tk.StringVar()
        self.v_cat = tk.StringVar()
        self.v_usage = tk.StringVar()
        self.v_value = tk.StringVar()
        self.v_tag = tk.StringVar()
        ttk.Label(flt, text=tr("Имя:")).pack(side="left")
        ttk.Entry(flt, textvariable=self.v_name, width=18).pack(side="left", padx=(2, 8))
        self.combos = {}
        for key, var, label in (("category", self.v_cat, tr("Категория:")), ("usage", self.v_usage, "usage:"),
                                ("value", self.v_value, "value:"), ("tag", self.v_tag, "tag:")):
            ttk.Label(flt, text=label).pack(side="left")
            cb = ttk.Combobox(flt, textvariable=var, width=12)
            cb.pack(side="left", padx=(2, 8))
            cb.bind("<<ComboboxSelected>>", lambda _e: self.fill())
            self.combos[key] = cb
        for v in (self.v_name, self.v_cat, self.v_usage, self.v_value, self.v_tag):
            v.trace_add("write", lambda *_: self.after_idle(self.fill))
        ttk.Button(flt, text=tr("Сбросить"), command=self.reset_filter).pack(side="left")

        tf = ttk.Frame(self)
        tf.grid(row=2, column=0, sticky="nsew")
        tf.columnconfigure(0, weight=1)
        tf.rowconfigure(0, weight=1)
        self.tree = ttk.Treeview(tf, columns=COLUMNS, show="headings", selectmode="extended")
        for c in COLUMNS:
            self.tree.heading(c, text=c, command=lambda c=c: self.sort_by(c))
            self.tree.column(c, width=WIDTHS.get(c, 72), stretch=c in ("name", "usage", "value"),
                             anchor="w" if c in ("name", "flags", "category", "usage", "value", "tag") else "e")
        self.tree.grid(row=0, column=0, sticky="nsew")
        sb = ttk.Scrollbar(tf, orient="vertical", command=self.tree.yview)
        sb.grid(row=0, column=1, sticky="ns")
        self.tree.configure(yscrollcommand=sb.set)
        self.tree.tag_configure("dirty", background="#fff4d6")
        self.tree.tag_configure("bad", foreground="#c62828")
        self.tree.bind("<Double-1>", self.edit_cell)

        bk = ttk.LabelFrame(self, text=tr("Массовая правка (к найденным или выделенным)"), padding=4)
        bk.grid(row=3, column=0, sticky="ew", pady=(4, 0))
        self.v_field = tk.StringVar(value="nominal")
        self.v_op = tk.StringVar(value=OPS[1][1])
        self.v_arg = tk.StringVar(value="2")
        self.v_sel_only = tk.BooleanVar(value=False)
        ttk.Combobox(bk, textvariable=self.v_field, values=te.ALL_FIELDS, width=16, state="readonly").pack(side="left")
        ttk.Combobox(bk, textvariable=self.v_op, values=[o[1] for o in OPS], width=18,
                     state="readonly").pack(side="left", padx=4)
        ttk.Entry(bk, textvariable=self.v_arg, width=20).pack(side="left")
        ttk.Checkbutton(bk, text=tr("только выделенные"), variable=self.v_sel_only).pack(side="left", padx=6)
        ttk.Button(bk, text=tr("Применить"), command=self.bulk).pack(side="left")
        self.status = ttk.Label(self, foreground=MUTED, wraplength=1000, justify="left", text=tr("Откройте types.xml. Двойной щелчок по ячейке — "
                                                               "правка; flags — 6 цифр (cargo hoarder map player "
                                                               "crafted deloot); списки — через запятую, @имя — user."))
        self.status.grid(row=4, column=0, sticky="w", pady=(4, 0))
        if self.v_file.get() and Path(self.v_file.get()).is_file():
            self.after(100, self.reload)

    def save(self):
        return {"file": self.v_file.get()}

    def accepts(self, path: Path) -> bool:
        return path.name.lower() == "types.xml"

    def add_paths(self, paths):
        self.v_file.set(paths[0])
        self.reload()

    # --- файл ---
    def _confirm_discard(self) -> bool:
        if self.tf is not None and self.tf.dirty:
            return messagebox.askyesno(APP_NAME, tr("Есть несохранённые изменения. Отбросить их?"))
        return True

    def pick(self):
        f = filedialog.askopenfilename(title="types.xml", filetypes=[("XML", "*.xml")])
        if f and self._confirm_discard():
            self.v_file.set(f)
            self.tf = None
            self.reload()

    def reload(self):
        if not self._confirm_discard():
            return
        try:
            self.tf = te.load(self.v_file.get())
        except (OSError, te.TypesError) as e:
            messagebox.showerror(APP_NAME, tr("Не удалось открыть: {0}").format(e))
            return
        for key, cb in self.combos.items():
            vals = sorted({x for r in self.tf.rows for x in r.lists.get(key, []) + r.user.get(key, [])},
                          key=str.lower)
            cb.configure(values=[""] + vals + ["-"])
        self.fill()

    def save_file(self, path=None):
        if self.tf is None:
            return
        try:
            out = te.save(self.tf, path)
        except OSError as e:
            messagebox.showerror(APP_NAME, tr("Не удалось сохранить: {0}").format(e))
            return
        self.v_file.set(str(out))
        self.app.log(tr("Сохранено: {0}").format(out))
        self.fill()

    def save_as(self):
        if self.tf is None:
            return
        f = filedialog.asksaveasfilename(title=tr("Сохранить types.xml"), defaultextension=".xml",
                                         initialfile="types.xml", filetypes=[("XML", "*.xml")])
        if f:
            self.save_file(f)

    # --- таблица ---
    def reset_filter(self):
        for v in (self.v_name, self.v_cat, self.v_usage, self.v_value, self.v_tag):
            v.set("")

    def _value(self, r: te.TypeRow, c: str) -> str:
        return _flags(r) if c == "flags" else r.get(c)

    def fill(self):
        if self.tf is None:
            return
        self.shown = [r for r in self.tf.rows if te.matches(r, self.v_name.get().strip(), self.v_cat.get(),
                                                            self.v_usage.get(), self.v_value.get(), self.v_tag.get())]
        if self.sort_key:
            def k(r):
                v = self._value(r, self.sort_key)
                try:
                    return (0, float(v), "")
                except ValueError:
                    return (1, 0.0, v.lower())
            self.shown.sort(key=k, reverse=self.sort_rev)
        self.tree.delete(*self.tree.get_children())
        for n, r in enumerate(self.shown):
            tags = (("dirty",) if r.dirty else ()) + (("bad",) if te.validate(r) else ())
            self.tree.insert("", "end", iid=str(n), values=[self._value(r, c) for c in COLUMNS], tags=tags)
        dirty = sum(1 for r in self.tf.rows if r.dirty)
        self.status.configure(text=tr("Показано {0} из {1}; изменено: {2}").format(len(self.shown), len(self.tf.rows),
                                                                                 dirty))

    def sort_by(self, c: str):
        self.sort_rev = not self.sort_rev if self.sort_key == c else False
        self.sort_key = c
        self.fill()

    def edit_cell(self, event):
        if self.editor is not None:
            self.editor.destroy()
        iid = self.tree.identify_row(event.y)
        col = self.tree.identify_column(event.x)
        if not iid or not col:
            return
        c = COLUMNS[int(col[1:]) - 1]
        r = self.shown[int(iid)]
        x, y, w, h = self.tree.bbox(iid, col)
        var = tk.StringVar(value=self._value(r, c))
        ent = ttk.Entry(self.tree, textvariable=var)
        ent.place(x=x, y=y, width=max(w, 80), height=h)
        ent.focus_set()
        ent.select_range(0, "end")
        self.editor = ent

        def commit(_e=None):
            val = var.get()
            try:
                if c == "flags":
                    digits = val.replace(" ", "")
                    if len(digits) != 6 or any(d not in "01" for d in digits):
                        raise te.TypesError(tr("flags: нужно 6 цифр 0/1"))
                    for k, d in zip(te.FLAG_FIELDS, digits):
                        te.set_value(r, k, d)
                else:
                    te.set_value(r, c, val)
            except te.TypesError as e:
                messagebox.showerror(APP_NAME, str(e))
                return
            cancel()
            self.tree.item(iid, values=[self._value(r, cc) for cc in COLUMNS],
                           tags=(("dirty",) if r.dirty else ()) + (("bad",) if te.validate(r) else ()))
            self.status.configure(text=tr("Показано {0} из {1}; изменено: {2}").format(
                len(self.shown), len(self.tf.rows), sum(1 for x in self.tf.rows if x.dirty)))

        def cancel(_e=None):
            ent.destroy()
            self.editor = None
        ent.bind("<Return>", commit)
        ent.bind("<Escape>", cancel)
        ent.bind("<FocusOut>", commit)

    def bulk(self):
        if self.tf is None:
            return
        op = next(k for k, label in OPS if label == self.v_op.get())
        rows = [self.shown[int(i)] for i in self.tree.selection()] if self.v_sel_only.get() else self.shown
        if not rows:
            return
        if len(rows) > 50 and not messagebox.askyesno(APP_NAME, tr("Изменить {0} типов?").format(len(rows))):
            return
        try:
            n = te.bulk(rows, self.v_field.get(), op, self.v_arg.get())
        except (te.TypesError, ValueError) as e:
            messagebox.showerror(APP_NAME, str(e))
            return
        self.app.log(tr("types.xml: изменено типов: {0}").format(n))
        self.fill()
