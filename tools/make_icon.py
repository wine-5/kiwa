"""アプリのアイコンを作る。

同梱の毛筆フォントで「際」を一字書き、和紙色の地に置いた絵を PNG で書き出し、
それをまとめて .ico にする。小さく表示されたときに潰れないよう、
字は太めに、余白は広めに取る。

    python tools/make_icon.py

書き出し先
    assets/icon/kiwa.png   … 見て確かめる用（1024px）
    assets/icon/kiwa.ico   … 実行ファイルに埋める用（256〜16px）
"""

import os

import numpy as np
from PIL import Image, ImageDraw, ImageFont

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUTPUT_DIR = os.path.join(ROOT, "assets", "icon")
FONT_PATH = os.path.join(ROOT, "assets", "fonts", "KouzanMouhitu.ttf")

# 下絵の大きさ。ここから縮めて各寸法を作る
BASE_SIZE = 1024

# 和紙の地と墨の色（game/constant/Palette.h と揃えてある）
PAPER = (232, 224, 203)
INK = (28, 26, 24)

# 縁に敷く濃い色（暗い背景でも輪郭が分かるように）
EDGE = (46, 26, 22)

# .ico に入れる寸法
ICON_SIZES = [256, 128, 64, 48, 32, 16]


def smooth(t):
    """なめらかに補間するための重み。"""
    return t * t * t * (t * (t * 6.0 - 15.0) + 10.0)


def paper_grain(size, seed):
    """和紙の斑。ごく薄く重ねて、べた塗りに見えないようにする。"""
    rng = np.random.default_rng(seed)

    cells = 16
    grid = rng.random((cells, cells))

    coords = np.arange(size) * cells / size
    index0 = np.floor(coords).astype(int) % cells
    index1 = (index0 + 1) % cells
    weight = smooth(coords - np.floor(coords))

    row = grid[:, index0] * (1.0 - weight) + grid[:, index1] * weight
    return row[index0, :] * (1.0 - weight)[:, None] + row[index1, :] * weight[:, None]


def make_icon(size):
    """アイコンの絵を作る。"""
    base = np.zeros((size, size, 3), dtype=np.float64)
    base[..., 0], base[..., 1], base[..., 2] = PAPER

    # 紙の斑
    base += ((paper_grain(size, seed=17) - 0.5) * 18.0)[:, :, None]

    # 四隅を軽く落として、紙に当たる灯りを作る
    axis = np.linspace(-1.0, 1.0, size)
    x, y = np.meshgrid(axis, axis)
    radius = np.sqrt(x * x + y * y) / np.sqrt(2.0)
    base *= (1.0 - np.clip(radius - 0.25, 0.0, 1.0) * 0.22)[:, :, None]

    image = Image.fromarray(np.clip(base, 0, 255).astype(np.uint8))
    draw = ImageDraw.Draw(image)

    # 縁取り。暗い背景に置いたときに、紙の四角が沈まないようにする
    border = max(2, size // 64)
    draw.rectangle([0, 0, size - 1, size - 1], outline=EDGE, width=border)

    # 「際」を一字。小さく表示されても読めるよう、枠いっぱいに書く
    font_size = int(size * 0.82)
    font = ImageFont.truetype(FONT_PATH, font_size)
    box = draw.textbbox((0, 0), "際", font=font)

    left = (size - (box[2] - box[0])) // 2 - box[0]
    top = (size - (box[3] - box[1])) // 2 - box[1]
    draw.text((left, top), "際", font=font, fill=INK)

    return image


def main():
    os.makedirs(OUTPUT_DIR, exist_ok=True)

    image = make_icon(BASE_SIZE)

    png_path = os.path.join(OUTPUT_DIR, "kiwa.png")
    image.save(png_path)
    print("書き出し:", png_path)

    # .ico は複数の寸法をまとめて持つ。小さいほうは個別に作り直して潰れを抑える
    frames = [make_icon(size) for size in ICON_SIZES]
    ico_path = os.path.join(OUTPUT_DIR, "kiwa.ico")
    frames[0].save(ico_path, format="ICO",
                   sizes=[(size, size) for size in ICON_SIZES],
                   append_images=frames[1:])
    print("書き出し:", ico_path)


main()
