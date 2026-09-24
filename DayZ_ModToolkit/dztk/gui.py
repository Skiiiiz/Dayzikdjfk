# -*- coding: utf-8 -*-
"""Графический интерфейс DayZ Mod Toolkit (tkinter)."""

from __future__ import annotations

import base64
import io
import os
import shutil
import subprocess
import sys
import threading
from dataclasses import asdict
from pathlib import Path
from typing import Callable, Dict, List, Optional

import tkinter as tk
from tkinter import filedialog, messagebox, ttk

from . import APP_NAME, APP_VERSION, audio, checks, convert, paa
from .cfg import Issue
from .common import check_vorbis, find_ffmpeg, human_size, load_settings_dict, open_folder, save_settings_dict

MUTED = "#6b6b6b"
LEVEL_NAMES = {"error": "Ошибка", "warning": "Предупр.", "info": "Инфо"}


def _num(var, default):
    try:
        return var.get()
    except (tk.TclError, ValueError):
        return default


# --------------------------------------------------------------------------------------
# Общие элементы
# --------------------------------------------------------------------------------------

class FileList(ttk.LabelFrame):
    """Список входных файлов/папок с колонками «исходник → результат → статус»."""

    def __init__(self, parent, app: "App", title: str, filetypes, on_change: Callable[[], None],
                 on_select: Optional[Callable[[Optional[Path]], None]] = None):
        super().__init__(parent, text=title, padding=6)
        self.app = app
        self.items: List[str] = []
        self.filetypes = filetypes
        self.on_change = on_change
        self.on_select = on_select
        self.columnconfigure(0, weight=1)
        self.rowconfigure(1, weight=1)

        bar = ttk.Frame(self)
        bar.grid(row=0, column=0, columnspan=2, sticky="ew", pady=(0, 4))
        ttk.Button(bar, text="+ Файлы", command=self.add_files).pack(side="left")
        ttk.Button(bar, text="+ Папка", command=self.add_folder).pack(side="left", padx=4)
        ttk.Button(bar, text="Убрать", command=self.remove_selected).pack(side="left")
        ttk.Button(bar, text="Очистить", command=self.clear).pack(side="left", padx=4)

        self.tree = ttk.Treeview(self, columns=("src", "dst", "status"), show="headings", selectmode="extended")
        for col, text, w in (("src", "Исходный файл", 200), ("dst", "Результат", 150), ("status", "Статус", 110)):
            self.tree.heading(col, text=text)
            self.tree.column(col, width=w, minwidth=60, stretch=col != "status")
        self.tree.grid(row=1, column=0, sticky="nsew")
        sb = ttk.Scrollbar(self, orient="vertical", command=self.tree.yview)
        sb.grid(row=1, column=1, sticky="ns")
        self.tree.configure(yscrollcommand=sb.set)
        self.tree.tag_configure("ok", foreground="#2e7d32")
        self.tree.tag_configure("err", foreground="#c62828")
        self.tree.bind("<<TreeviewSelect>>", self._selected)

    def add_paths(self, paths):
        for p in paths:
            if p and p not in self.items:
                self.items.append(p)
        self.on_change()

    def add_files(self):
        self.add_paths(filedialog.askopenfilenames(title="Выберите файлы", filetypes=self.filetypes))

    def add_folder(self):
        d = filedialog.askdirectory(title="Выберите папку")
        if d:
            self.add_paths([d])

    def remove_selected(self):
        sel = set(self.tree.selection())
        if not sel:
            return
        keep: List[str] = []
        for item in self.items:
            p = Path(item)
            if p.is_file() and str(p.resolve()) in sel:
                continue
            if p.is_dir():
                inner = [str(r) for r in self.tree.get_children() if r.startswith(str(p.resolve()))]
                if any(r in sel for r in inner):
                    keep += [r for r in inner if r not in sel]
                    continue
            keep.append(item)
        self.items = keep
        self.on_change()

    def clear(self):
        self.items = []
        self.on_change()

    def show(self, rows):
        """rows: [(src Path, root Path|None, dst_text)]"""
        self.tree.delete(*self.tree.get_children())
        for src, root, dst in rows:
            shown = str(src.relative_to(root.parent)) if root is not None else src.name
            iid = str(src.resolve())
            if not self.tree.exists(iid):
                self.tree.insert("", "end", iid=iid, values=(shown, dst, "ожидает"))

    def set_status(self, src: Path, text: str, ok: Optional[bool] = None):
        iid = str(src.resolve())
        if self.tree.exists(iid):
            self.tree.set(iid, "status", text)
            self.tree.item(iid, tags=(() if ok is None else ("ok" if ok else "err"),))
            self.tree.see(iid)

    def _selected(self, _e=None):
        if self.on_select:
            sel = self.tree.selection()
            self.on_select(Path(sel[0]) if sel else None)


def labeled(parent, row: int, label: str, widget, hint: str = ""):
    ttk.Label(parent, text=label).grid(row=row, column=0, sticky="w", pady=2, padx=(0, 6))
    widget.grid(row=row, column=1, sticky="ew", pady=2)
    if hint:
        ttk.Label(parent, text=hint, foreground=MUTED).grid(row=row, column=2, sticky="w", padx=4)


def dir_picker(parent, var: tk.StringVar, title: str, on_pick: Optional[Callable] = None):
    f = ttk.Frame(parent)
    ttk.Entry(f, textvariable=var).pack(side="left", fill="x", expand=True)

    def pick():
        d = filedialog.askdirectory(title=title)
        if d:
            var.set(d)
            if on_pick:
                on_pick()
    ttk.Button(f, text="...", width=3, command=pick).pack(side="left")
    return f


class Tab(ttk.Frame):
    title = ""

    def __init__(self, nb: ttk.Notebook, app: "App"):
        super().__init__(nb, padding=6)
        self.app = app
        nb.add(self, text=self.title)

    def save(self) -> dict:
        return {}

    def accepts(self, path: Path) -> bool:
        return False

    def add_paths(self, paths: List[str]) -> None:
        pass


# --------------------------------------------------------------------------------------
# Вкладка «Звук»
# --------------------------------------------------------------------------------------

class AudioTab(Tab):
    title = "  Звук → OGG  "

    def __init__(self, nb, app):
        super().__init__(nb, app)
        s = audio.ConvertSettings.from_dict(app.settings.get("audio", {}))
        self.columnconfigure(0, weight=3)
        self.columnconfigure(1, weight=2)
        self.rowconfigure(0, weight=1)
        exts = " ".join("*" + e for e in sorted(audio.INPUT_EXTENSIONS))
        self.files = FileList(self, app, "Аудиофайлы", [("Аудио", exts), ("Все файлы", "*.*")], self.refresh)
        self.files.grid(row=0, column=0, sticky="nsew", padx=(0, 6))

        V = self.v = {}
        V["preset"] = tk.StringVar(value=audio.DEFAULT_PRESET)
        V["channels"] = tk.StringVar(value="Моно" if s.channels == 1 else "Стерео")
        V["rate"] = tk.StringVar(value=str(s.sample_rate))
        V["quality"] = tk.IntVar(value=s.quality)
        V["volume"] = tk.DoubleVar(value=s.volume_db)
        V["norm"] = tk.StringVar(value=audio.NORMALIZE_MODES.get(s.normalize, audio.NORMALIZE_MODES["none"]))
        V["peak"] = tk.DoubleVar(value=s.peak_target_db)
        V["lufs"] = tk.DoubleVar(value=s.loudness_target)
        V["trim"] = tk.BooleanVar(value=s.trim_silence)
        V["fin"] = tk.DoubleVar(value=s.fade_in)
        V["fout"] = tk.DoubleVar(value=s.fade_out)
        V["cstart"] = tk.DoubleVar(value=s.cut_start)
        V["cdur"] = tk.DoubleVar(value=s.cut_duration)
        V["out"] = tk.StringVar(value=s.output_dir)
        V["keep"] = tk.BooleanVar(value=s.keep_structure)
        V["sanitize"] = tk.BooleanVar(value=s.sanitize_names)
        V["prefix"] = tk.StringVar(value=s.name_prefix)
        V["overwrite"] = tk.BooleanVar(value=s.overwrite)
        V["threads"] = tk.IntVar(value=s.threads)
        V["gencfg"] = tk.BooleanVar(value=s.generate_config)
        V["mod"] = tk.StringVar(value=s.mod_name)
        V["spath"] = tk.StringVar(value=s.sound_path)
        V["cpref"] = tk.StringVar(value=s.class_prefix)
        V["spatial"] = tk.BooleanVar(value=s.spatial)
        V["loop"] = tk.BooleanVar(value=s.loop)
        V["range"] = tk.IntVar(value=s.range)
        V["svol"] = tk.DoubleVar(value=s.shader_volume)

        nb = ttk.Notebook(self)
        nb.grid(row=0, column=1, sticky="nsew")

        ta = ttk.Frame(nb, padding=8)
        ta.columnconfigure(1, weight=1)
        nb.add(ta, text="Аудио")
        cb = ttk.Combobox(ta, textvariable=V["preset"], values=list(audio.PRESETS), state="readonly")
        cb.bind("<<ComboboxSelected>>", self.apply_preset)
        labeled(ta, 0, "Пресет", cb)
        labeled(ta, 1, "Каналы", ttk.Combobox(ta, textvariable=V["channels"], values=["Моно", "Стерео"],
                                              state="readonly", width=10), "3D = моно")
        labeled(ta, 2, "Частота, Гц", ttk.Combobox(ta, textvariable=V["rate"],
                                                  values=["22050", "32000", "44100", "48000"], state="readonly",
                                                  width=10))
        qf = ttk.Frame(ta)
        q_lbl = ttk.Label(qf, width=3, text=str(V["quality"].get()))
        qs = ttk.Scale(qf, from_=-1, to=10, orient="horizontal",
                       command=lambda v: (V["quality"].set(int(round(float(v)))),
                                          q_lbl.configure(text=str(V["quality"].get()))))
        qs.set(V["quality"].get())
        qs.pack(side="left", fill="x", expand=True)
        q_lbl.pack(side="left")
        labeled(ta, 3, "Качество Vorbis", qf, "5–7")
        labeled(ta, 4, "Усиление, дБ", ttk.Spinbox(ta, textvariable=V["volume"], from_=-30, to=30, increment=0.5,
                                                  width=8))
        labeled(ta, 5, "Нормализация", ttk.Combobox(ta, textvariable=V["norm"],
                                                   values=list(audio.NORMALIZE_MODES.values()), state="readonly"))
        labeled(ta, 6, "Пик, dBFS", ttk.Spinbox(ta, textvariable=V["peak"], from_=-20, to=0, increment=0.5, width=8))
        labeled(ta, 7, "Громкость, LUFS", ttk.Spinbox(ta, textvariable=V["lufs"], from_=-40, to=-5, increment=1,
                                                     width=8))
        ttk.Checkbutton(ta, text="Обрезать тишину в начале/конце", variable=V["trim"]).grid(
            row=8, column=0, columnspan=3, sticky="w", pady=2)
        labeled(ta, 9, "Fade-in, с", ttk.Spinbox(ta, textvariable=V["fin"], from_=0, to=30, increment=0.1, width=8))
        labeled(ta, 10, "Fade-out, с", ttk.Spinbox(ta, textvariable=V["fout"], from_=0, to=30, increment=0.1,
                                                  width=8))
        labeled(ta, 11, "Начать с, с", ttk.Spinbox(ta, textvariable=V["cstart"], from_=0, to=36000, increment=0.5,
                                                  width=8))
        labeled(ta, 12, "Длительность, с", ttk.Spinbox(ta, textvariable=V["cdur"], from_=0, to=36000,
                                                      increment=0.5, width=8), "0 = целиком")

        tf = ttk.Frame(nb, padding=8)
        tf.columnconfigure(1, weight=1)
        nb.add(tf, text="Вывод")
        labeled(tf, 0, "Папка вывода", dir_picker(tf, V["out"], "Папка для OGG", self.refresh))
        ttk.Label(tf, text="Пусто = подпапка «ogg» в исходной папке", foreground=MUTED).grid(
            row=1, column=0, columnspan=3, sticky="w")
        ttk.Checkbutton(tf, text="Сохранять структуру подпапок", variable=V["keep"], command=self.refresh).grid(
            row=2, column=0, columnspan=3, sticky="w", pady=2)
        ttk.Checkbutton(tf, text="Безопасные имена (латиница, a-z0-9_)", variable=V["sanitize"],
                        command=self.refresh).grid(row=3, column=0, columnspan=3, sticky="w", pady=2)
        pe = ttk.Entry(tf, textvariable=V["prefix"])
        pe.bind("<FocusOut>", lambda e: self.refresh())
        labeled(tf, 4, "Префикс имени", pe)
        ttk.Checkbutton(tf, text="Перезаписывать существующие", variable=V["overwrite"]).grid(
            row=5, column=0, columnspan=3, sticky="w", pady=2)
        labeled(tf, 6, "Потоков", ttk.Spinbox(tf, textvariable=V["threads"], from_=1, to=32, increment=1, width=6))

        tc = ttk.Frame(nb, padding=8)
        tc.columnconfigure(1, weight=1)
        nb.add(tc, text="config.cpp")
        ttk.Checkbutton(tc, text="Генерировать config.cpp", variable=V["gencfg"]).grid(
            row=0, column=0, columnspan=3, sticky="w", pady=2)
        labeled(tc, 1, "Имя мода", ttk.Entry(tc, textvariable=V["mod"]))
        labeled(tc, 2, "Путь в PBO", ttk.Entry(tc, textvariable=V["spath"]))
        ttk.Label(tc, text=r"напр. MyMod\sounds — куда положите .ogg", foreground=MUTED).grid(
            row=3, column=0, columnspan=3, sticky="w")
        labeled(tc, 4, "Префикс классов", ttk.Entry(tc, textvariable=V["cpref"]))
        ttk.Checkbutton(tc, text="3D (spatial = 1)", variable=V["spatial"]).grid(row=5, column=0, columnspan=3,
                                                                              sticky="w", pady=2)
        ttk.Checkbutton(tc, text="Зациклить (loop = 1)", variable=V["loop"]).grid(row=6, column=0, columnspan=3,
                                                                               sticky="w", pady=2)
        labeled(tc, 7, "Дальность, м", ttk.Spinbox(tc, textvariable=V["range"], from_=1, to=5000, increment=10,
                                                  width=8))
        labeled(tc, 8, "Громкость", ttk.Spinbox(tc, textvariable=V["svol"], from_=0, to=5, increment=0.1, width=8))
        ttk.Button(tc, text="Предпросмотр", command=self.preview_config).grid(row=9, column=0, sticky="w", pady=8)

        self.run_btn = ttk.Button(self, text="▶ Конвертировать звук", command=self.start)
        self.run_btn.grid(row=1, column=1, sticky="e", pady=(6, 0))
        app.run_buttons.append(self.run_btn)

    def collect(self) -> audio.ConvertSettings:
        V = self.v
        norm_key = next((k for k, v in audio.NORMALIZE_MODES.items() if v == V["norm"].get()), "none")
        return audio.ConvertSettings(
            channels=1 if V["channels"].get() == "Моно" else 2,
            sample_rate=int(V["rate"].get() or 44100),
            quality=int(_num(V["quality"], 6)),
            volume_db=float(_num(V["volume"], 0.0)),
            normalize=norm_key,
            peak_target_db=float(_num(V["peak"], -1.0)),
            loudness_target=float(_num(V["lufs"], -16.0)),
            trim_silence=V["trim"].get(),
            fade_in=max(0.0, float(_num(V["fin"], 0.0))),
            fade_out=max(0.0, float(_num(V["fout"], 0.0))),
            cut_start=max(0.0, float(_num(V["cstart"], 0.0))),
            cut_duration=max(0.0, float(_num(V["cdur"], 0.0))),
            output_dir=V["out"].get().strip(),
            keep_structure=V["keep"].get(),
            sanitize_names=V["sanitize"].get(),
            name_prefix=V["prefix"].get().strip(),
            overwrite=V["overwrite"].get(),
            threads=max(1, int(_num(V["threads"], 2))),
            generate_config=V["gencfg"].get(),
            mod_name=V["mod"].get().strip() or "MyMod",
            sound_path=V["spath"].get().strip(),
            class_prefix=V["cpref"].get().strip(),
            spatial=V["spatial"].get(),
            loop=V["loop"].get(),
            range=int(_num(V["range"], 50)),
            shader_volume=float(_num(V["svol"], 1.0)),
            ffmpeg_path=self.app.v_ffmpeg.get().strip(),
        )

    def save(self) -> dict:
        return asdict(self.collect())

    def apply_preset(self, *_):
        p = audio.PRESETS.get(self.v["preset"].get())
        if not p:
            return
        self.v["channels"].set("Моно" if p["channels"] == 1 else "Стерео")
        self.v["rate"].set(str(p["sample_rate"]))
        self.v["quality"].set(p["quality"])
        self.v["spatial"].set(p["spatial"])
        self.v["loop"].set(p["loop"])
        self.v["range"].set(p["range"])

    def jobs(self):
        cur = self.collect()
        return cur, audio.plan_jobs(audio.collect_inputs(self.files.items, cur.output_dir), cur)

    def refresh(self):
        cur, jobs = self.jobs()
        base = audio.output_root([audio.JobResult(j, True) for j in jobs], cur)
        rows = []
        for j in jobs:
            try:
                dst = str(j.dst.relative_to(base)) if base else j.dst.name
            except ValueError:
                dst = j.dst.name
            rows.append((j.src, j.root, dst))
        self.files.show(rows)
        self.app.status(f"Звук: файлов в очереди {len(jobs)}")

    def accepts(self, path: Path) -> bool:
        return path.suffix.lower() in audio.INPUT_EXTENSIONS and path.suffix.lower() != ".ogg"

    def add_paths(self, paths):
        self.files.add_paths(paths)

    def preview_config(self):
        cur, jobs = self.jobs()
        self.app.show_text("Предпросмотр config.cpp",
                           audio.generate_config([audio.JobResult(j, True) for j in jobs], cur).replace("\r\n", "\n"))

    def start(self):
        cur, jobs = self.jobs()
        ffmpeg = find_ffmpeg(cur.ffmpeg_path)
        if not ffmpeg:
            messagebox.showerror(APP_NAME, "ffmpeg не найден. Укажите путь на вкладке «Справка».")
            return
        if not check_vorbis(ffmpeg):
            messagebox.showerror(APP_NAME, f"ffmpeg собран без кодека libvorbis:\n{ffmpeg}")
            return
        if not jobs:
            messagebox.showinfo(APP_NAME, "Добавьте аудиофайлы.")
            return
        self.app.log(f"Звук: {len(jobs)} файл(ов), {'моно' if cur.channels == 1 else 'стерео'}, "
                     f"{cur.sample_rate} Гц, q={cur.quality}")
        for j in jobs:
            self.files.set_status(j.src, "в работе")

        def on_result(r: audio.JobResult, done: int, total: int):
            def ui():
                self.files.set_status(r.job.src, "✔ готово" if r.ok else "✖ ошибка", r.ok)
                self.app.progress(done, total)
                if r.ok:
                    info = human_size(r.size) + (f", {r.duration:.2f} c" if r.duration else "")
                    self.app.log(f"OK   {r.job.src.name} -> {r.job.dst}  ({info})")
                else:
                    self.app.log(f"ERR  {r.job.src.name}: {r.message}", "error")
            self.app.ui(ui)

        def work():
            results = audio.convert_all(jobs, cur, ffmpeg, on_result, self.app.cancel)
            cfg_path = audio.write_config(results, cur) if cur.generate_config else None
            out = audio.output_root(results, cur) or (jobs[0].dst.parent if jobs else None)
            ok = sum(r.ok for r in results)
            msgs = [f"config.cpp: {cfg_path}"] if cfg_path else []
            return out, f"Звук: готово {ok} из {len(results)}", msgs

        self.app.run_async(work, len(jobs))


# --------------------------------------------------------------------------------------
# Вкладка «Текстуры»
# --------------------------------------------------------------------------------------

class TextureTab(Tab):
    title = "  Текстуры PAA  "

    def __init__(self, nb, app):
        super().__init__(nb, app)
        st = app.settings.get("textures", {})
        self.columnconfigure(0, weight=3)
        self.columnconfigure(1, weight=2)
        self.rowconfigure(0, weight=1)
        img_types = " ".join("*" + e for e in sorted(paa.IMAGE_EXTENSIONS | {".paa"}))
        self.files = FileList(self, app, "Картинки и PAA", [("Картинки и PAA", img_types), ("Все файлы", "*.*")],
                              self.refresh, self.preview)
        self.files.grid(row=0, column=0, sticky="nsew", padx=(0, 6))

        right = ttk.Frame(self)
        right.grid(row=0, column=1, sticky="nsew")
        right.columnconfigure(0, weight=1)
        right.rowconfigure(2, weight=1)

        self.v_mode = tk.StringVar(value=st.get("mode", "to_paa"))
        self.v_fmt = tk.StringVar(value=st.get("fmt", "auto"))
        self.v_resize = tk.BooleanVar(value=st.get("resize", False))
        self.v_out = tk.StringVar(value=st.get("out", ""))
        self.v_img = tk.StringVar(value=st.get("img", "png"))
        self.v_overwrite = tk.BooleanVar(value=st.get("overwrite", True))

        mf = ttk.LabelFrame(right, text="Направление", padding=8)
        mf.grid(row=0, column=0, sticky="ew")
        ttk.Radiobutton(mf, text="Картинки (PNG/TGA/JPG/BMP) → PAA", variable=self.v_mode, value="to_paa",
                        command=self.refresh).pack(anchor="w")
        ttk.Radiobutton(mf, text="PAA → картинки", variable=self.v_mode, value="from_paa",
                        command=self.refresh).pack(anchor="w")

        of = ttk.LabelFrame(right, text="Параметры", padding=8)
        of.grid(row=1, column=0, sticky="ew", pady=6)
        of.columnconfigure(1, weight=1)
        labeled(of, 0, "Формат PAA", ttk.Combobox(of, textvariable=self.v_fmt, values=["auto", "dxt1", "dxt5"],
                                                 state="readonly", width=8), "auto: по прозрачности/_ca/_co")
        ttk.Checkbutton(of, text="Подгонять размер к степени двойки", variable=self.v_resize).grid(
            row=1, column=0, columnspan=3, sticky="w", pady=2)
        labeled(of, 2, "PAA → формат", ttk.Combobox(of, textvariable=self.v_img, values=["png", "tga"],
                                                   state="readonly", width=8), "", )
        labeled(of, 3, "Папка вывода", dir_picker(of, self.v_out, "Папка вывода", self.refresh))
        ttk.Label(of, text="Пусто = рядом с исходным файлом", foreground=MUTED).grid(row=4, column=0, columnspan=3,
                                                                                  sticky="w")
        ttk.Checkbutton(of, text="Перезаписывать существующие", variable=self.v_overwrite).grid(
            row=5, column=0, columnspan=3, sticky="w", pady=2)

        pf = ttk.LabelFrame(right, text="Предпросмотр", padding=6)
        pf.grid(row=2, column=0, sticky="nsew")
        self.preview_lbl = ttk.Label(pf, anchor="center", text="Выберите файл в списке")
        self.preview_lbl.pack(fill="both", expand=True)
        self.preview_info = ttk.Label(pf, foreground=MUTED)
        self.preview_info.pack()
        self._photo = None

        self.run_btn = ttk.Button(self, text="▶ Конвертировать текстуры", command=self.start)
        self.run_btn.grid(row=1, column=1, sticky="e", pady=(6, 0))
        app.run_buttons.append(self.run_btn)

    def save(self):
        return {"mode": self.v_mode.get(), "fmt": self.v_fmt.get(), "resize": self.v_resize.get(),
                "out": self.v_out.get(), "img": self.v_img.get(), "overwrite": self.v_overwrite.get()}

    def tasks(self):
        to_paa = self.v_mode.get() == "to_paa"
        exts = paa.IMAGE_EXTENSIONS if to_paa else {".paa"}
        inputs = convert.collect(self.files.items, exts)
        suffix = ".paa" if to_paa else "." + self.v_img.get()
        return inputs, convert.plan(inputs, self.v_out.get().strip(), suffix)

    def refresh(self):
        inputs, tasks = self.tasks()
        self.files.show([(t.src, root, t.dst.name) for (src, root), t in zip(inputs, tasks)])
        self.app.status(f"Текстуры: файлов в очереди {len(tasks)}")

    def accepts(self, path: Path) -> bool:
        return path.suffix.lower() in paa.IMAGE_EXTENSIONS or path.suffix.lower() == ".paa"

    def add_paths(self, paths):
        if paths and all(Path(p).suffix.lower() == ".paa" for p in paths):
            self.v_mode.set("from_paa")
        self.files.add_paths(paths)

    def preview(self, path: Optional[Path]):
        if path is None or not path.is_file():
            return
        try:
            from PIL import Image
            if path.suffix.lower() == ".paa":
                rgba, info = paa.read_paa(path)
                im = Image.fromarray(rgba, "RGBA")
                desc = f"{info.width}x{info.height}, {info.type_name}, mip-уровней: {len(info.mipmaps)}"
            else:
                im = Image.open(path).convert("RGBA")
                w, h = im.size
                pow2 = paa.is_pow2(w) and paa.is_pow2(h)
                desc = f"{w}x{h}" + ("" if pow2 else " — не степень двойки!")
            im.thumbnail((180, 180))
            bg = Image.new("RGBA", im.size, (60, 60, 70, 255))
            for y in range(0, im.size[1], 16):          # «шахматка» под прозрачностью
                for x in range(0, im.size[0], 16):
                    if (x // 16 + y // 16) % 2:
                        bg.paste((90, 90, 100, 255), (x, y, x + 16, y + 16))
            bg.alpha_composite(im)
            buf = io.BytesIO()
            bg.convert("RGB").save(buf, "PNG")
            self._photo = tk.PhotoImage(data=base64.b64encode(buf.getvalue()))
            self.preview_lbl.configure(image=self._photo, text="")
            self.preview_info.configure(text=desc)
        except Exception as e:
            self._photo = None
            self.preview_lbl.configure(image="", text=f"Не удалось открыть:\n{e}")
            self.preview_info.configure(text="")

    def start(self):
        _, tasks = self.tasks()
        if not tasks:
            messagebox.showinfo(APP_NAME, "Добавьте картинки или PAA-файлы (проверьте направление).")
            return
        if self.v_mode.get() == "to_paa":
            fn = convert.image_to_paa_task(self.v_fmt.get(), "nearest" if self.v_resize.get() else "error",
                                           self.v_overwrite.get())
        else:
            fn = convert.paa_to_image_task(self.v_overwrite.get())
        self.app.log(f"Текстуры: {len(tasks)} файл(ов)")
        for t in tasks:
            self.files.set_status(t.src, "в работе")

        def on_result(r: convert.Result, done, total):
            def ui():
                self.files.set_status(r.task.src, "✔ " + r.message if r.ok else "✖ ошибка", r.ok)
                self.app.progress(done, total)
                self.app.log(("OK   " if r.ok else "ERR  ") + f"{r.task.src.name} -> {r.task.dst}  ({r.message})",
                             None if r.ok else "error")
            self.app.ui(ui)

        def work():
            res = convert.run(tasks, fn, max(1, min(4, os.cpu_count() or 2)), on_result, self.app.cancel)
            ok = sum(r.ok for r in res)
            out = Path(self.v_out.get()) if self.v_out.get().strip() else tasks[0].dst.parent
            return out, f"Текстуры: готово {ok} из {len(res)}", []

        self.app.run_async(work, len(tasks))


# --------------------------------------------------------------------------------------
# Вкладка «Конфиги»
# --------------------------------------------------------------------------------------

class ConfigTab(Tab):
    title = "  config.cpp ⇄ bin  "

    def __init__(self, nb, app):
        super().__init__(nb, app)
        st = app.settings.get("configs", {})
        self.columnconfigure(0, weight=3)
        self.columnconfigure(1, weight=2)
        self.rowconfigure(0, weight=1)
        self.files = FileList(self, app, "Конфиги (.cpp → .bin, .bin → .cpp)",
                              [("Конфиги", "*.cpp *.bin"), ("Все файлы", "*.*")], self.refresh)
        self.files.grid(row=0, column=0, sticky="nsew", padx=(0, 6))

        self.v_out = tk.StringVar(value=st.get("out", ""))
        self.v_overwrite = tk.BooleanVar(value=st.get("overwrite", False))
        self.v_check = tk.BooleanVar(value=st.get("check", True))
        of = ttk.LabelFrame(self, text="Параметры", padding=8)
        of.grid(row=0, column=1, sticky="new")
        of.columnconfigure(1, weight=1)
        ttk.Label(of, wraplength=330, justify="left", text=(
            "Направление определяется автоматически:\n"
            "• config.cpp → config.bin (бинаризация, как в Addon Builder);\n"
            "• config.bin → config.cpp (распаковка чужого мода для изучения).")).grid(
            row=0, column=0, columnspan=3, sticky="w", pady=(0, 6))
        labeled(of, 1, "Папка вывода", dir_picker(of, self.v_out, "Папка вывода", self.refresh))
        ttk.Label(of, text="Пусто = рядом с исходным файлом", foreground=MUTED).grid(row=2, column=0, columnspan=3,
                                                                                  sticky="w")
        ttk.Checkbutton(of, text="Перезаписывать существующие", variable=self.v_overwrite).grid(
            row=3, column=0, columnspan=3, sticky="w", pady=2)
        ttk.Checkbutton(of, text="Не бинаризовать при ошибках (базовые классы и т.п.)",
                        variable=self.v_check).grid(row=4, column=0, columnspan=3, sticky="w", pady=2)

        self.run_btn = ttk.Button(self, text="▶ Преобразовать", command=self.start)
        self.run_btn.grid(row=1, column=1, sticky="e", pady=(6, 0))
        app.run_buttons.append(self.run_btn)

    def save(self):
        return {"out": self.v_out.get(), "overwrite": self.v_overwrite.get(), "check": self.v_check.get()}

    def tasks(self):
        inputs = convert.collect(self.files.items, {".cpp", ".bin"})
        inputs = [(f, r) for f, r in inputs if f.suffix.lower() == ".cpp" or f.read_bytes()[:4] == b"\x00raP"]
        return inputs, convert.plan(inputs, self.v_out.get().strip(), "", convert.config_dst_name)

    def refresh(self):
        inputs, tasks = self.tasks()
        self.files.show([(t.src, root, t.dst.name) for (src, root), t in zip(inputs, tasks)])
        self.app.status(f"Конфиги: файлов в очереди {len(tasks)}")

    def accepts(self, path: Path) -> bool:
        return path.suffix.lower() in (".cpp", ".bin")

    def add_paths(self, paths):
        self.files.add_paths(paths)

    def start(self):
        _, tasks = self.tasks()
        if not tasks:
            messagebox.showinfo(APP_NAME, "Добавьте config.cpp или config.bin.")
            return
        fn = convert.config_task(self.v_overwrite.get(), self.v_check.get())

        def on_result(r: convert.Result, done, total):
            def ui():
                self.files.set_status(r.task.src, "✔ " + r.message.split(",")[0] if r.ok else "✖ ошибка", r.ok)
                self.app.progress(done, total)
                self.app.log(("OK   " if r.ok else "ERR  ") + f"{r.task.src} -> {r.task.dst.name}  ({r.message})",
                             None if r.ok else "error")
                for i in r.details or []:
                    if i.level != "info":
                        self.app.log("     " + i.format(), i.level)
            self.app.ui(ui)

        def work():
            res = convert.run(tasks, fn, 1, on_result, self.app.cancel)
            ok = sum(r.ok for r in res)
            return tasks[0].dst.parent, f"Конфиги: готово {ok} из {len(res)}", []

        self.app.run_async(work, len(tasks))


# --------------------------------------------------------------------------------------
# Вкладка «Проверка»
# --------------------------------------------------------------------------------------

class CheckTab(Tab):
    title = "  Проверка ошибок  "

    def __init__(self, nb, app):
        super().__init__(nb, app)
        st = app.settings.get("check", {})
        self.columnconfigure(0, weight=1)
        self.rowconfigure(1, weight=1)
        self.paths: List[str] = list(st.get("paths", []))
        self.issues: List[Issue] = []

        top = ttk.Frame(self)
        top.grid(row=0, column=0, sticky="ew")
        top.columnconfigure(0, weight=1)
        self.path_list = tk.Listbox(top, height=3, activestyle="none")
        self.path_list.grid(row=0, column=0, rowspan=2, sticky="ew")
        bf = ttk.Frame(top)
        bf.grid(row=0, column=1, sticky="n", padx=6)
        ttk.Button(bf, text="+ Папка мода", command=self.add_folder).pack(fill="x")
        ttk.Button(bf, text="+ Файлы", command=self.add_files).pack(fill="x", pady=2)
        ttk.Button(bf, text="Убрать", command=self.remove).pack(fill="x")

        of = ttk.Frame(top)
        of.grid(row=0, column=2, sticky="n")
        self.v_cross = tk.BooleanVar(value=st.get("cross", True))
        self.v_err = tk.BooleanVar(value=True)
        self.v_warn = tk.BooleanVar(value=True)
        self.v_info = tk.BooleanVar(value=st.get("info", False))
        ttk.Checkbutton(of, text="Ссылки между файлами", variable=self.v_cross).pack(anchor="w")
        ttk.Checkbutton(of, text="Ошибки", variable=self.v_err, command=self.fill).pack(anchor="w")
        ttk.Checkbutton(of, text="Предупреждения", variable=self.v_warn, command=self.fill).pack(anchor="w")
        ttk.Checkbutton(of, text="Замечания", variable=self.v_info, command=self.fill).pack(anchor="w")

        rf = ttk.Frame(self)
        rf.grid(row=1, column=0, sticky="nsew", pady=6)
        rf.columnconfigure(0, weight=1)
        rf.rowconfigure(0, weight=1)
        self.tree = ttk.Treeview(rf, columns=("level", "file", "line", "msg"), show="headings")
        for col, text, w, st_ in (("level", "Уровень", 80, False), ("file", "Файл", 260, True),
                                  ("line", "Стр.", 55, False), ("msg", "Сообщение", 520, True)):
            self.tree.heading(col, text=text)
            self.tree.column(col, width=w, stretch=st_, anchor="w" if col != "line" else "e")
        self.tree.grid(row=0, column=0, sticky="nsew")
        sb = ttk.Scrollbar(rf, orient="vertical", command=self.tree.yview)
        sb.grid(row=0, column=1, sticky="ns")
        self.tree.configure(yscrollcommand=sb.set)
        self.tree.tag_configure("error", foreground="#c62828")
        self.tree.tag_configure("warning", foreground="#b26a00")
        self.tree.tag_configure("info", foreground=MUTED)
        self.tree.bind("<Double-1>", self.open_selected)

        bottom = ttk.Frame(self)
        bottom.grid(row=2, column=0, sticky="ew")
        self.summary = ttk.Label(bottom, text="Добавьте папку мода (где лежит config.cpp) или отдельные файлы.")
        self.summary.pack(side="left")
        self.run_btn = ttk.Button(bottom, text="▶ Проверить", command=self.start)
        self.run_btn.pack(side="right")
        ttk.Button(bottom, text="Сохранить отчёт", command=self.export).pack(side="right", padx=6)
        app.run_buttons.append(self.run_btn)
        self._refresh_list()

    def save(self):
        return {"paths": self.paths, "cross": self.v_cross.get(), "info": self.v_info.get()}

    def accepts(self, path: Path) -> bool:
        return path.is_dir() or checks.checker_for(path) is not None

    def add_paths(self, paths):
        for p in paths:
            if p and p not in self.paths:
                self.paths.append(p)
        self._refresh_list()

    def _refresh_list(self):
        self.path_list.delete(0, "end")
        for p in self.paths:
            self.path_list.insert("end", p)

    def add_folder(self):
        d = filedialog.askdirectory(title="Папка мода")
        if d:
            self.add_paths([d])

    def add_files(self):
        self.add_paths(filedialog.askopenfilenames(title="Файлы для проверки", filetypes=[
            ("Файлы DayZ", "*.cpp *.bin *.c *.layout *.imageset *.xml *.json *.csv *.paa *.ogg"),
            ("Все файлы", "*.*")]))

    def remove(self):
        for i in reversed(self.path_list.curselection()):
            del self.paths[i]
        self._refresh_list()

    def fill(self):
        self.tree.delete(*self.tree.get_children())
        show = {"error": self.v_err.get(), "warning": self.v_warn.get(), "info": self.v_info.get()}
        order = {"error": 0, "warning": 1, "info": 2}
        for n, i in enumerate(sorted(self.issues, key=lambda x: (order.get(x.level, 3), x.file, x.line))):
            if not show.get(i.level, True):
                continue
            f = i.file
            for p in self.paths:
                if f.startswith(str(Path(p))):
                    f = os.path.relpath(f, str(Path(p).parent))
                    break
            self.tree.insert("", "end", iid=str(n), values=(LEVEL_NAMES.get(i.level, i.level), f,
                                                            i.line or "", i.message), tags=(i.level,))
        self._sorted = sorted(self.issues, key=lambda x: (order.get(x.level, 3), x.file, x.line))

    def open_selected(self, _e=None):
        sel = self.tree.selection()
        if not sel:
            return
        i = self._sorted[int(sel[0])]
        if not i.file or not Path(i.file).exists():
            return
        code = shutil.which("code")
        try:
            if code and i.line:
                subprocess.Popen([code, "-g", f"{i.file}:{i.line}:{max(1, i.col)}"])
            else:
                open_folder(Path(i.file))
        except Exception:
            pass

    def export(self):
        if not self.issues:
            messagebox.showinfo(APP_NAME, "Сначала выполните проверку.")
            return
        f = filedialog.asksaveasfilename(title="Сохранить отчёт", defaultextension=".txt",
                                         filetypes=[("Текст", "*.txt")], initialfile="dayz_check_report.txt")
        if f:
            Path(f).write_text("\n".join(i.format() for i in self._sorted) + "\n" + self.summary.cget("text") + "\n",
                               encoding="utf-8")
            self.app.log(f"Отчёт сохранён: {f}")

    def start(self):
        if not self.paths:
            messagebox.showinfo(APP_NAME, "Добавьте папку мода или файлы для проверки.")
            return
        ffmpeg = find_ffmpeg(self.app.v_ffmpeg.get().strip())
        paths = list(self.paths)
        cross = self.v_cross.get()
        self.issues = []
        self.fill()
        self.app.log(f"Проверка: {', '.join(paths)}")

        def on_file(f: Path, iss, done, total):
            self.app.ui(lambda: self.app.progress(done, total))

        def work():
            rep = checks.check_paths(paths, ffmpeg, on_file, cross)

            def ui():
                self.issues = rep.issues
                self.fill()
                txt = (f"Проверено файлов: {rep.files}.  Ошибок: {rep.count('error')},  "
                       f"предупреждений: {rep.count('warning')},  замечаний: {rep.count('info')}")
                self.summary.configure(text=txt, foreground="#c62828" if rep.count("error") else "#2e7d32")
            self.app.ui(ui)
            return None, (f"Проверка: ошибок {rep.count('error')}, предупреждений {rep.count('warning')}"), []

        self.app.run_async(work, 1)


# --------------------------------------------------------------------------------------
# Вкладка «Справка»
# --------------------------------------------------------------------------------------

HELP = """DayZ Mod Toolkit — набор инструментов для моддера DayZ. Работает без установки.

ЗВУК → OGG
  MP3/WAV/FLAC/M4A/... → OGG Vorbis. Для 3D-звуков (в мире) — моно, 44100 Гц.
  Может сгенерировать config.cpp с CfgSoundShaders/CfgSoundSets.

ТЕКСТУРЫ PAA
  PNG/TGA/JPG/BMP → PAA (DXT1/DXT5 с mip-уровнями) и PAA → PNG/TGA для правки.
  Стороны текстуры должны быть степенью двойки (256, 512, 1024, 2048...).
  Формат «auto»: есть прозрачность или суффикс _ca → DXT5, иначе (и для _co) → DXT1.

CONFIG.CPP ⇄ CONFIG.BIN
  Бинаризация конфига (как Addon Builder/CfgConvert) с проверкой ошибок,
  и обратная распаковка config.bin в читаемый config.cpp.

ПРОВЕРКА ОШИБОК
  Укажите папку мода (где config.cpp) — будут проверены:
  • config.cpp: синтаксис (пропущенные ; и }; ), необъявленные базовые классы, дубликаты, CfgPatches;
  • скрипты .c: скобки, строки, пропущенные ; и запятые в enum, #ifdef/#endif, '=' вместо '==';
  • .layout/.imageset: скобки и строки;  XML (types.xml, events.xml, cfgspawnabletypes.xml, globals.xml);
  • JSON, stringtable.csv, PAA (размеры, mip-уровни), OGG (кодек Vorbis);
  • ссылки на несуществующие файлы, отсутствующие ключи #STR_, папки скриптов из CfgMods,
    пробелы и кириллица в именах файлов.
  Двойной щелчок по строке открывает файл (в VS Code — сразу на нужной строке).

Проверка скриптов — это быстрый анализатор, а не компилятор DayZ: он ловит типичные
синтаксические ошибки, но не проверяет типы и существование методов.
"""


class HelpTab(Tab):
    title = "  Справка  "

    def __init__(self, nb, app):
        super().__init__(nb, app)
        self.columnconfigure(0, weight=1)
        self.rowconfigure(0, weight=1)
        t = tk.Text(self, wrap="word", font=("Segoe UI", 10), relief="flat", padx=10, pady=8)
        t.insert("1.0", HELP)
        t.configure(state="disabled")
        t.grid(row=0, column=0, sticky="nsew")
        ff = ttk.LabelFrame(self, text="ffmpeg (для звука и проверки .ogg)", padding=6)
        ff.grid(row=1, column=0, sticky="ew", pady=(6, 0))
        e = ttk.Frame(ff)
        e.pack(fill="x")
        ttk.Entry(e, textvariable=app.v_ffmpeg).pack(side="left", fill="x", expand=True)

        def pick():
            f = filedialog.askopenfilename(title="Укажите ffmpeg", filetypes=[("ffmpeg", "ffmpeg*"), ("Все", "*.*")])
            if f:
                app.v_ffmpeg.set(f)
        ttk.Button(e, text="...", width=3, command=pick).pack(side="left")
        found = find_ffmpeg(app.v_ffmpeg.get())
        ttk.Label(ff, foreground=MUTED, text=f"Сейчас используется: {found or 'не найден'}  "
                                             f"(пусто = встроенный/автопоиск)").pack(anchor="w")


# --------------------------------------------------------------------------------------
# Приложение
# --------------------------------------------------------------------------------------

class App:
    def __init__(self, root: tk.Tk, initial: List[str]):
        self.root = root
        self.settings = load_settings_dict()
        self.cancel = threading.Event()
        self.busy = False
        self.run_buttons: List[ttk.Button] = []
        self.last_out: Optional[Path] = None
        self.v_ffmpeg = tk.StringVar(value=self.settings.get("ffmpeg", ""))

        root.title(f"{APP_NAME} {APP_VERSION}")
        root.geometry("1100x760")
        root.minsize(900, 620)
        style = ttk.Style()
        try:
            style.theme_use("clam")
        except tk.TclError:
            pass
        style.configure("Green.Horizontal.TProgressbar", background="#5a8f3c", troughcolor="#c9c6bd")
        style.configure("TNotebook.Tab", padding=(10, 4))

        main = ttk.Frame(root, padding=8)
        main.pack(fill="both", expand=True)
        main.columnconfigure(0, weight=1)
        main.rowconfigure(0, weight=1)

        self.nb = ttk.Notebook(main)
        self.nb.grid(row=0, column=0, sticky="nsew")
        self.tabs: Dict[str, Tab] = {
            "audio": AudioTab(self.nb, self),
            "textures": TextureTab(self.nb, self),
            "configs": ConfigTab(self.nb, self),
            "check": CheckTab(self.nb, self),
            "help": HelpTab(self.nb, self),
        }

        logf = ttk.LabelFrame(main, text="Журнал", padding=4)
        logf.grid(row=1, column=0, sticky="ew", pady=(6, 0))
        logf.columnconfigure(0, weight=1)
        self.log_text = tk.Text(logf, height=7, wrap="word", state="disabled", font=("Consolas", 9))
        self.log_text.grid(row=0, column=0, sticky="ew")
        lsb = ttk.Scrollbar(logf, orient="vertical", command=self.log_text.yview)
        lsb.grid(row=0, column=1, sticky="ns")
        self.log_text.configure(yscrollcommand=lsb.set)
        self.log_text.tag_configure("error", foreground="#c62828")
        self.log_text.tag_configure("warning", foreground="#b26a00")

        bottom = ttk.Frame(main)
        bottom.grid(row=2, column=0, sticky="ew", pady=(6, 0))
        bottom.columnconfigure(0, weight=1)
        self.pbar = ttk.Progressbar(bottom, mode="determinate", style="Green.Horizontal.TProgressbar")
        self.pbar.grid(row=0, column=0, sticky="ew")
        self.stop_btn = ttk.Button(bottom, text="■ Стоп", state="disabled", command=self.cancel.set)
        self.stop_btn.grid(row=0, column=1, padx=6)
        self.open_btn = ttk.Button(bottom, text="Открыть папку", state="disabled",
                                   command=lambda: self.last_out and open_folder(self.last_out))
        self.open_btn.grid(row=0, column=2)
        self.status_var = tk.StringVar(value="Готов к работе.")
        ttk.Label(bottom, textvariable=self.status_var).grid(row=1, column=0, columnspan=3, sticky="w", pady=(4, 0))

        tab_idx = {"audio": 0, "textures": 1, "configs": 2, "check": 3}
        self.nb.select(tab_idx.get(self.settings.get("tab", "audio"), 0))
        if initial:
            self.route(initial)
        self.log(f"{APP_NAME} {APP_VERSION}")
        ff = find_ffmpeg(self.v_ffmpeg.get())
        self.log(f"ffmpeg: {ff}" if ff else "ffmpeg не найден — конвертация звука недоступна (см. «Справка»).",
                 None if ff else "warning")
        root.protocol("WM_DELETE_WINDOW", self.on_close)

    def route(self, paths: List[str]) -> None:
        """Распределяет файлы, переданные при запуске (перетаскивание на .exe), по вкладкам."""
        buckets: Dict[str, List[str]] = {}
        for p in paths:
            path = Path(p)
            for key in ("audio", "textures", "configs", "check"):
                tab = self.tabs[key]
                if (path.is_dir() and key == "check") or (path.is_file() and tab.accepts(path)):
                    buckets.setdefault(key, []).append(p)
                    break
        for key, items in buckets.items():
            self.tabs[key].add_paths(items)
        if buckets:
            self.nb.select(list(self.tabs).index(next(iter(buckets))))

    # --- обслуживание ---
    def ui(self, fn: Callable[[], None]) -> None:
        self.root.after(0, fn)

    def log(self, text: str, level: Optional[str] = None) -> None:
        self.log_text.configure(state="normal")
        self.log_text.insert("end", text + "\n", (level,) if level else ())
        self.log_text.see("end")
        self.log_text.configure(state="disabled")

    def status(self, text: str) -> None:
        self.status_var.set(text)

    def progress(self, done: int, total: int) -> None:
        self.pbar.configure(maximum=max(1, total), value=done)
        self.status(f"{done} / {total}")

    def show_text(self, title: str, text: str) -> None:
        win = tk.Toplevel(self.root)
        win.title(title)
        win.geometry("720x600")
        t = tk.Text(win, wrap="none", font=("Consolas", 10))
        t.pack(fill="both", expand=True)
        t.insert("1.0", text)

        def copy():
            self.root.clipboard_clear()
            self.root.clipboard_append(t.get("1.0", "end-1c"))
        ttk.Button(win, text="Копировать", command=copy).pack(pady=4)

    def run_async(self, work: Callable[[], tuple], total: int) -> None:
        if self.busy:
            return
        self.busy = True
        self.cancel.clear()
        for b in self.run_buttons:
            b.configure(state="disabled")
        self.stop_btn.configure(state="normal")
        self.open_btn.configure(state="disabled")
        self.pbar.configure(maximum=max(1, total), value=0)
        self.save_settings()

        def runner():
            try:
                out, msg, extra = work()
                err = None
            except Exception as e:  # pragma: no cover
                out, msg, extra, err = None, "", [], e

            def done():
                self.busy = False
                for b in self.run_buttons:
                    b.configure(state="normal")
                self.stop_btn.configure(state="disabled")
                if err is not None:
                    self.log(f"Ошибка: {err}", "error")
                    self.status("Ошибка")
                    return
                for m in extra:
                    self.log(m)
                if self.cancel.is_set():
                    msg2 = msg + " (остановлено)"
                else:
                    msg2 = msg
                self.log(msg2)
                self.status(msg2)
                if out:
                    self.last_out = Path(out)
                    self.open_btn.configure(state="normal")
            self.ui(done)

        threading.Thread(target=runner, daemon=True).start()

    def save_settings(self) -> None:
        data = {k: t.save() for k, t in self.tabs.items() if k != "help"}
        data["ffmpeg"] = self.v_ffmpeg.get()
        try:
            data["tab"] = list(self.tabs)[self.nb.index(self.nb.select())]
        except Exception:
            pass
        save_settings_dict(data)

    def on_close(self) -> None:
        self.save_settings()
        self.cancel.set()
        self.root.destroy()


def gui(initial: Optional[List[str]] = None) -> int:
    root = tk.Tk()
    App(root, initial or [])
    root.mainloop()
    return 0
