#!/usr/bin/env python3
"""Генератор .layout файлов меню Boogy_BattlePass (палитра S.T.A.L.K.E.R. PDA, как в SM_PartyMod).

Запуск: python3 TOOLS/gen_layouts.py  (из корня Boogy_BattlePass)
"""
import os

OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "GUI", "layouts")

BG = "0.0566 0.0484 0.0401 0.9608"
BG_DARK = "0.0428 0.0366 0.0304 0.9608"
ROW = "0.1755 0.15 0.1245 0.85"
ROW_DARK = "0.1 0.086 0.071 0.9"
BTN = "0.2964 0.2533 0.2103 1"
COPPER = "0.7214 0.4508 0.2002 1"
COPPER_DIM = "0.7214 0.4508 0.2002 0.55"
AMBER = "0.9098 0.7098 0.2706 1"
TEXT = "0.936 0.9 0.855 1"
MUTED = "0.702 0.651 0.5725 1"
FONT = "gui/fonts/sdf_MetronBook24"


def ind(level):
    return " " * level


def panel(name, x, y, w, h, color, level, ignore=True, visible=True, children=None):
    s = f"{ind(level)}PanelWidgetClass {name} {{\n"
    if not visible:
        s += f"{ind(level+1)}visible 0\n"
    if ignore:
        s += f"{ind(level+1)}ignorepointer 1\n"
    s += f"{ind(level+1)}color {color}\n"
    s += f"{ind(level+1)}position {x} {y}\n{ind(level+1)}size {w} {h}\n"
    s += f"{ind(level+1)}hexactpos 1\n{ind(level+1)}vexactpos 1\n{ind(level+1)}hexactsize 1\n{ind(level+1)}vexactsize 1\n"
    s += f"{ind(level+1)}style rover_sim_colorable\n"
    if children:
        s += f"{ind(level+1)}{{\n" + "".join(children) + f"{ind(level+1)}}}\n"
    s += f"{ind(level)}}}\n"
    return s


def text(name, x, y, w, h, value, size, color, level, halign="left", valign="center", visible=True, multiline=False):
    cls = "MultilineTextWidgetClass" if multiline else "TextWidgetClass"
    s = f"{ind(level)}{cls} {name} {{\n"
    if not visible:
        s += f"{ind(level+1)}visible 0\n"
    s += f"{ind(level+1)}ignorepointer 1\n{ind(level+1)}color {color}\n"
    s += f"{ind(level+1)}position {x} {y}\n{ind(level+1)}size {w} {h}\n"
    s += f"{ind(level+1)}hexactpos 1\n{ind(level+1)}vexactpos 1\n{ind(level+1)}hexactsize 1\n{ind(level+1)}vexactsize 1\n"
    s += f"{ind(level+1)}text \"{value}\"\n{ind(level+1)}font \"{FONT}\"\n"
    s += f"{ind(level+1)}\"exact text\" 1\n{ind(level+1)}\"exact text size\" {size}\n"
    s += f"{ind(level+1)}\"text valign\" {valign}\n{ind(level+1)}\"text halign\" {halign}\n"
    s += f"{ind(level)}}}\n"
    return s


def button(name, x, y, w, h, value, level, size=14, visible=True):
    """Кнопка = подложка <name>Bg + прозрачная ButtonWidget <name> с текстом."""
    s = panel(name + "Bg", x, y, w, h, BTN, level, visible=visible)
    s += f"{ind(level)}ButtonWidgetClass {name} {{\n"
    if not visible:
        s += f"{ind(level+1)}visible 0\n"
    s += f"{ind(level+1)}color 1 1 0.95 1\n"
    s += f"{ind(level+1)}position {x} {y}\n{ind(level+1)}size {w} {h}\n"
    s += f"{ind(level+1)}hexactpos 1\n{ind(level+1)}vexactpos 1\n{ind(level+1)}hexactsize 1\n{ind(level+1)}vexactsize 1\n"
    s += f"{ind(level+1)}style EmptyHighlight\n{ind(level+1)}text \"{value}\"\n"
    s += f"{ind(level+1)}font \"{FONT}\"\n{ind(level+1)}\"exact text\" 1\n{ind(level+1)}\"exact text size\" {size}\n"
    s += f"{ind(level)}}}\n"
    return s


def root_frame(name, w, h, children, fullscreen=False):
    s = f"FrameWidgetClass {name} {{\n"
    if fullscreen:
        s += " position 0 0\n size 1 1\n hexactpos 0\n vexactpos 0\n hexactsize 0\n vexactsize 0\n priority 60\n"
    else:
        s += f" position 0 0\n size {w} {h}\n hexactpos 1\n vexactpos 1\n hexactsize 1\n vexactsize 1\n"
    s += " {\n" + "".join(children) + " }\n}\n"
    return s


def menu():
    W, H = 1100, 680
    L = 3
    inner = [
        panel("TopAccent", 0, 0, W, 3, COPPER, L),
        panel("HeaderBg", 0, 3, W, 94, BG_DARK, L),
        text("TitleText", 24, 12, 420, 30, "БОЕВОЙ ПРОПУСК", 24, AMBER, L),
        text("SeasonName", 24, 44, 540, 22, "", 16, TEXT, L),
        text("SeasonTimer", 24, 68, 540, 20, "", 13, MUTED, L),
        text("LevelText", 600, 12, 360, 28, "", 18, TEXT, L),
        panel("XpBarBg", 600, 46, 360, 14, ROW_DARK, L),
        panel("XpBarFill", 600, 46, 0, 14, COPPER, L),
        text("XpText", 600, 64, 360, 20, "", 13, MUTED, L),
        text("PremiumBadge", 972, 12, 110, 28, "", 15, AMBER, L, halign="right"),
        text("CoinsText", 972, 44, 110, 22, "", 14, TEXT, L, halign="right"),
        button("CloseButton", 1062, 62, 28, 28, "X", L),
        button("TabRewards", 24, 104, 160, 32, "НАГРАДЫ", L),
        button("TabQuests", 190, 104, 160, 32, "ЗАДАНИЯ", L),
        button("TabShop", 356, 104, 160, 32, "МАГАЗИН", L),
        button("AdminReload", 716, 104, 170, 32, "ПЕРЕЗАГРУЗИТЬ", L, size=12, visible=False),
        button("ClaimAll", 892, 104, 184, 32, "ЗАБРАТЬ ВСЁ", L),
        panel("TabLine", 24, 140, 1052, 2, COPPER_DIM, L),
        f"{ind(L)}ScrollWidgetClass ContentScroll {{\n"
        f"{ind(L+1)}position 24 148\n{ind(L+1)}size 1052 486\n"
        f"{ind(L+1)}hexactpos 1\n{ind(L+1)}vexactpos 1\n{ind(L+1)}hexactsize 1\n{ind(L+1)}vexactsize 1\n"
        f"{ind(L+1)}\"Scrollbar V\" 1\n"
        f"{ind(L+1)}{{\n"
        f"{ind(L+2)}FrameWidgetClass Content {{\n"
        f"{ind(L+3)}position 0 0\n{ind(L+3)}size 1036 486\n"
        f"{ind(L+3)}hexactpos 1\n{ind(L+3)}vexactpos 1\n{ind(L+3)}hexactsize 1\n{ind(L+3)}vexactsize 1\n"
        f"{ind(L+2)}}}\n"
        f"{ind(L+1)}}}\n"
        f"{ind(L)}}}\n",
        text("EmptyText", 24, 360, 1052, 40, "", 16, MUTED, L, halign="center", visible=False),
        text("FooterText", 24, 644, 1052, 26, "", 13, MUTED, L),
    ]
    window = panel("Window", 0, 0, W, H, BG, 1, ignore=False, children=inner)
    # центрирование окна
    window = window.replace(f"{ind(2)}position 0 0\n{ind(2)}size {W} {H}\n",
                            f"{ind(2)}position 0 0\n{ind(2)}size {W} {H}\n{ind(2)}halign center_ref\n{ind(2)}valign center_ref\n", 1)
    dim = f"{ind(1)}PanelWidgetClass Dim {{\n{ind(2)}ignorepointer 1\n{ind(2)}color 0 0 0 0.55\n{ind(2)}position 0 0\n{ind(2)}size 1 1\n{ind(2)}hexactpos 0\n{ind(2)}vexactpos 0\n{ind(2)}hexactsize 0\n{ind(2)}vexactsize 0\n{ind(2)}style rover_sim_colorable\n{ind(1)}}}\n"
    return root_frame("BBPMenuRoot", 0, 0, [dim, window], fullscreen=True)


def level_row():
    L = 1
    return root_frame("BBPLevelRow", 1036, 76, [
        panel("RowBg", 0, 0, 1036, 72, ROW, L),
        panel("LevelBox", 0, 0, 72, 72, ROW_DARK, L),
        panel("LevelAccent", 0, 0, 4, 72, COPPER, L),
        text("LevelNum", 4, 6, 68, 40, "1", 26, TEXT, L, halign="center"),
        text("LevelXp", 4, 46, 68, 20, "", 11, MUTED, L, halign="center"),
        text("FreeLabel", 88, 6, 380, 18, "БЕСПЛАТНО", 11, MUTED, L),
        text("FreeRewards", 88, 24, 380, 42, "", 14, TEXT, L, valign="top", multiline=True),
        button("FreeClaim", 474, 22, 116, 30, "ЗАБРАТЬ", L, size=12),
        panel("Divider", 604, 8, 2, 56, COPPER_DIM, L),
        text("PremiumLabel", 618, 6, 280, 18, "ПРЕМИУМ", 11, AMBER, L),
        text("PremiumRewards", 618, 24, 280, 42, "", 14, TEXT, L, valign="top", multiline=True),
        button("PremiumClaim", 906, 22, 120, 30, "ЗАБРАТЬ", L, size=12),
    ])


def quest_row():
    L = 1
    return root_frame("BBPQuestRow", 1036, 74, [
        panel("RowBg", 0, 0, 1036, 70, ROW, L),
        panel("RowAccent", 0, 0, 4, 70, COPPER, L),
        text("QuestTag", 16, 4, 560, 18, "", 11, MUTED, L),
        text("QuestName", 16, 22, 560, 22, "", 16, TEXT, L),
        text("QuestDesc", 16, 44, 560, 22, "", 12, MUTED, L),
        panel("ProgressBg", 592, 18, 250, 12, ROW_DARK, L),
        panel("ProgressFill", 592, 18, 0, 12, COPPER, L),
        text("ProgressText", 592, 34, 250, 20, "", 13, TEXT, L, halign="center"),
        text("RewardText", 856, 8, 170, 24, "", 13, AMBER, L, halign="right"),
        text("StatusText", 856, 36, 170, 24, "", 12, MUTED, L, halign="right"),
    ])


def case_row():
    L = 1
    return root_frame("BBPCaseRow", 1036, 124, [
        panel("RowBg", 0, 0, 1036, 120, ROW, L),
        panel("RowAccent", 0, 0, 4, 120, AMBER, L),
        text("CaseName", 16, 8, 700, 24, "", 18, TEXT, L),
        text("CaseDesc", 16, 32, 700, 20, "", 12, MUTED, L),
        text("CaseContents", 16, 54, 700, 62, "", 12, TEXT, L, valign="top", multiline=True),
        text("CasePrice", 730, 8, 294, 24, "", 15, AMBER, L, halign="right"),
        text("CaseOwned", 730, 34, 294, 20, "", 13, MUTED, L, halign="right"),
        button("BuyButton", 740, 74, 136, 32, "КУПИТЬ", L, size=13),
        button("OpenButton", 888, 74, 136, 32, "ОТКРЫТЬ", L, size=13),
    ])


def main():
    os.makedirs(OUT, exist_ok=True)
    files = {
        "BBP_Menu.layout": menu(),
        "BBP_LevelRow.layout": level_row(),
        "BBP_QuestRow.layout": quest_row(),
        "BBP_CaseRow.layout": case_row(),
    }
    for name, content in files.items():
        with open(os.path.join(OUT, name), "w", encoding="utf-8", newline="\n") as f:
            f.write(content)
        print("written", name)


if __name__ == "__main__":
    main()
