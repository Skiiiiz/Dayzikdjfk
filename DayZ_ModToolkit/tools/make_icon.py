# -*- coding: utf-8 -*-
"""Рисует иконку программы: assets/icon.png и assets/icon.ico (нужен Pillow)."""
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parents[1]
FONTS = ["/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", "C:/Windows/Fonts/arialbd.ttf",
         "/usr/share/fonts/truetype/liberation/LiberationSans-Bold.ttf"]


def font(size):
    for f in FONTS:
        if Path(f).is_file():
            return ImageFont.truetype(f, size)
    return ImageFont.load_default()


def draw(size=512):
    s = size
    im = Image.new("RGBA", (s, s), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    r = s // 7
    d.rounded_rectangle((s * 0.03, s * 0.03, s * 0.97, s * 0.97), r, fill=(38, 43, 38, 255),
                        outline=(122, 140, 90, 255), width=max(2, s // 40))
    # «DZ»
    f = font(int(s * 0.42))
    text = "DZ"
    bb = d.textbbox((0, 0), text, font=f)
    d.text(((s - (bb[2] - bb[0])) / 2 - bb[0], s * 0.14 - bb[1]), text, font=f, fill=(226, 220, 200, 255))
    # полоса-«лента» с галочкой: инструмент проверки
    d.rounded_rectangle((s * 0.16, s * 0.64, s * 0.84, s * 0.84), s // 20, fill=(90, 143, 60, 255))
    w = max(3, s // 22)
    d.line([(s * 0.36, s * 0.74), (s * 0.46, s * 0.81), (s * 0.66, s * 0.67)], fill=(255, 255, 255, 255),
           width=w, joint="curve")
    return im


if __name__ == "__main__":
    out = ROOT / "assets"
    out.mkdir(exist_ok=True)
    big = draw(512)
    big.resize((256, 256), Image.LANCZOS).save(out / "icon.png")
    big.save(out / "icon.ico", sizes=[(16, 16), (24, 24), (32, 32), (48, 48), (64, 64), (128, 128), (256, 256)])
    print("assets/icon.png, assets/icon.ico")
