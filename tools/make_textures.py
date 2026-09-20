"""assets/textures/ のテクスチャを生成する。

画像ファイルを外から持ってこずに済ませるため、木目と水面の細かな凹凸を
計算で作って PNG に書き出す。実行すると assets/textures/ の中身を作り直す。

    python tools/make_textures.py

いずれも上下左右がつながる（タイル状に並べても継ぎ目が出ない）ように作っている。
"""

import os

import numpy as np
from PIL import Image

OUTPUT_DIR = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "assets", "textures")
SIZE = 1024


def smooth(t):
    """なめらかに補間するための重み。"""
    return t * t * t * (t * (t * 6.0 - 15.0) + 10.0)


def value_noise(size, cells, rng):
    """格子状の乱数をなめらかにつないだ、上下左右がつながる雑音を返す。"""
    grid = rng.random((cells, cells))

    coords = np.arange(size) * cells / size
    index0 = np.floor(coords).astype(int) % cells
    index1 = (index0 + 1) % cells
    weight = smooth(coords - np.floor(coords))

    # 横方向に補間してから縦方向に補間する
    row0 = grid[:, index0] * (1.0 - weight) + grid[:, index1] * weight
    col0 = row0[index0, :] * (1.0 - weight)[:, None] + row0[index1, :] * weight[:, None]
    return col0


def fractal_noise(size, cells, octaves, rng):
    """粗さの違う雑音を重ねたもの。"""
    total = np.zeros((size, size), dtype=np.float64)
    amplitude = 1.0
    weight = 0.0

    for i in range(octaves):
        total += value_noise(size, cells * (2 ** i), rng) * amplitude
        weight += amplitude
        amplitude *= 0.5

    return total / weight


def stretched_noise(size, cells_x, cells_y, rng):
    """縦横で粗さの違う雑音（木の導管のような細長い模様に使う）。"""
    grid = rng.random((cells_y, cells_x))

    x = np.arange(size) * cells_x / size
    y = np.arange(size) * cells_y / size
    ix0 = np.floor(x).astype(int) % cells_x
    ix1 = (ix0 + 1) % cells_x
    iy0 = np.floor(y).astype(int) % cells_y
    iy1 = (iy0 + 1) % cells_y
    wx = smooth(x - np.floor(x))
    wy = smooth(y - np.floor(y))

    row = grid[:, ix0] * (1.0 - wx) + grid[:, ix1] * wx
    return row[iy0, :] * (1.0 - wy)[:, None] + row[iy1, :] * wy[:, None]


def make_wood(size, seed, light, dark, ring_count, pore_strength, roughness, warp_amount, pore_cells):
    """木目を作る。

    年輪をうねらせ、導管の筋と木肌のざらつきを重ねる。
    """
    rng = np.random.default_rng(seed)

    v = np.linspace(0.0, 1.0, size, endpoint=False)[:, None] * np.ones((1, size))

    # 年輪はまっすぐ走らない。雑音で座標をゆがめてから縞にする
    warp = fractal_noise(size, 3, 4, rng) - 0.5
    rings = 0.5 + 0.5 * np.sin((v * ring_count + warp * warp_amount) * 2.0 * np.pi)
    rings = rings ** 0.5

    # 導管は木目と同じ向き（横）に走る。縦に走らせると布地のように見えてしまう
    pores = stretched_noise(size, 8, pore_cells, rng)
    pores = np.clip((pores - 0.52) * 3.2, 0.0, 1.0)

    # 木肌のざらつき
    grit = fractal_noise(size, 64, 3, rng) - 0.5

    shade = rings - pores * pore_strength + grit * roughness
    shade = np.clip(shade, 0.0, 1.0)[:, :, None]

    light = np.array(light, dtype=np.float64)
    dark = np.array(dark, dtype=np.float64)
    color = dark + (light - dark) * shade

    # 場所ごとの色味のばらつき（一枚板に見えないように）
    tint = (fractal_noise(size, 2, 3, rng) - 0.5)[:, :, None]
    color = color * (1.0 + tint * 0.10)

    return np.clip(color, 0, 255).astype(np.uint8)


def make_water_normal(size, seed):
    """水面の細かな凹凸を、法線マップとして作る。

    大きなうねりの上に重ねる細部で、これがないと水面がつるりとしてしまう。
    """
    rng = np.random.default_rng(seed)

    height = fractal_noise(size, 8, 5, rng)
    height += stretched_noise(size, 24, 10, rng) * 0.35
    height /= 1.35

    # 高さの傾きから法線を求める（端はつながるように np.roll を使う）
    strength = 3.2
    dx = (np.roll(height, -1, axis=1) - np.roll(height, 1, axis=1)) * strength
    dy = (np.roll(height, -1, axis=0) - np.roll(height, 1, axis=0)) * strength

    normal = np.stack([-dx, -dy, np.ones_like(height)], axis=-1)
    normal /= np.linalg.norm(normal, axis=-1, keepdims=True)

    return ((normal * 0.5 + 0.5) * 255.0).astype(np.uint8)


def make_card(text, size, rng_seed):
    """花月の札。和紙色の地に、太い明朝で縦に字を刷る。"""
    from PIL import ImageDraw, ImageFont

    width, height = size
    paper = Image.new("RGB", (width, height), (232, 224, 203))
    draw = ImageDraw.Draw(paper)

    # 和紙の繊維らしさ（ごく薄い斑）
    rng = np.random.default_rng(rng_seed)
    grain = fractal_noise(max(width, height), 24, 3, rng)[:height, :width]
    speckle = (grain - 0.5) * 16.0
    base = np.asarray(paper, dtype=np.float64) + speckle[:, :, None]

    paper = Image.fromarray(np.clip(base, 0, 255).astype(np.uint8))
    draw = ImageDraw.Draw(paper)

    # 縁の罫
    margin = int(width * 0.10)
    draw.rectangle([margin, margin, width - margin, height - margin],
                   outline=(120, 96, 74), width=max(2, width // 90))

    # 字は縦に一字ずつ置く
    # 同梱した毛筆のフォントを使う（画面の見出しと同じ書体で揃える）
    font_path = os.path.join(os.path.dirname(OUTPUT_DIR), "fonts", "KouzanMouhitu.ttf")
    font_size = int(width * 0.50)
    font = ImageFont.truetype(font_path, font_size)

    total = len(text) * font_size + (len(text) - 1) * int(font_size * 0.12)
    y = (height - total) // 2
    for character in text:
        box = draw.textbbox((0, 0), character, font=font)
        x = (width - (box[2] - box[0])) // 2 - box[0]
        draw.text((x, y - box[1]), character, font=font, fill=(38, 32, 28))
        y += font_size + int(font_size * 0.12)

    return np.asarray(paper)


def make_card_back(size, rng_seed):
    """札の裏。無地に近い和紙に、小さな丸紋をひとつ。"""
    from PIL import ImageDraw

    width, height = size
    rng = np.random.default_rng(rng_seed)
    grain = fractal_noise(max(width, height), 24, 3, rng)[:height, :width]

    base = np.zeros((height, width, 3), dtype=np.float64)
    base[..., 0] = 214
    base[..., 1] = 203
    base[..., 2] = 180
    base += ((grain - 0.5) * 18.0)[:, :, None]

    paper = Image.fromarray(np.clip(base, 0, 255).astype(np.uint8))
    draw = ImageDraw.Draw(paper)

    radius = int(width * 0.16)
    center = (width // 2, height // 2)
    draw.ellipse([center[0] - radius, center[1] - radius, center[0] + radius, center[1] + radius],
                 outline=(150, 124, 98), width=max(2, width // 80))

    return np.asarray(paper)


def make_tatami(size, seed):
    """畳表（い草の織り目）。細い横筋が詰んで走り、ところどころ色が振れる。"""
    rng = np.random.default_rng(seed)

    # 織り目は等間隔の横筋。細かいので、筋の山と谷で明暗を作る
    v = np.linspace(0.0, 1.0, size, endpoint=False)[:, None] * np.ones((1, size))
    lines = 0.5 + 0.5 * np.sin(v * 44.0 * 2.0 * np.pi)
    weave = lines ** 1.6

    # い草は一本ずつ色が違う。筋の番号ごとに明るさを振る
    strand_index = np.floor(v * 44.0).astype(int)
    strand_shade = rng.random(44 + 1)[strand_index % 44]

    # 縦方向の繊維の流れ
    fiber = stretched_noise(size, 220, 6, rng)

    # 経糸（たていと）で締めた筋が、一定間隔で縦に入る
    u = np.linspace(0.0, 1.0, size, endpoint=False)[None, :] * np.ones((size, 1))
    warp = 0.5 + 0.5 * np.sin(u * 6.0 * 2.0 * np.pi)
    warp = np.clip((warp - 0.86) * 6.0, 0.0, 1.0)

    shade = (0.74 + 0.26 * weave) * (0.88 + 0.24 * strand_shade)
    shade -= fiber * 0.10
    shade -= warp * 0.10
    shade = np.clip(shade, 0.0, 1.2)[:, :, None]

    light = np.array((206, 196, 142), dtype=np.float64)
    dark = np.array((150, 142, 96), dtype=np.float64)
    color = dark + (light - dark) * np.clip(shade, 0.0, 1.0)

    # 日に焼けた斑（新しい畳ほど緑が強く、焼けると黄色くなる）
    tint = (fractal_noise(size, 3, 3, rng) - 0.5)[:, :, None]
    color = color * (1.0 + tint * 0.12)
    color[..., 1] *= 1.0 + 0.04 * tint[..., 0]

    return np.clip(color, 0, 255).astype(np.uint8)


def save(image, name):
    os.makedirs(OUTPUT_DIR, exist_ok=True)
    path = os.path.join(OUTPUT_DIR, name)
    Image.fromarray(image).save(path)
    print("書き出し:", path)


def blur(image, radius):
    """箱ぼかし。ピントの外れた背景を作るのに使う。"""
    result = image.astype(np.float64)
    for axis in (0, 1):
        accumulated = np.zeros_like(result)
        for offset in range(-radius, radius + 1):
            accumulated += np.roll(result, offset, axis=axis)
        result = accumulated / (2 * radius + 1)
    return np.clip(result, 0, 255).astype(np.uint8)


def make_vignette(size):
    """周辺減光。四隅を落として中央へ目を集める。黒＋アルファで作る。"""
    axis = np.linspace(-1.0, 1.0, size)
    x, y = np.meshgrid(axis, axis)
    radius = np.sqrt(x * x + y * y) / np.sqrt(2.0)

    alpha = np.clip((radius - 0.35) / 0.65, 0.0, 1.0) ** 1.6
    rgba = np.zeros((size, size, 4), dtype=np.uint8)
    rgba[..., 3] = (alpha * 235).astype(np.uint8)
    return rgba


def make_grain(size, seed):
    """粒状感。写真のフィルム粒子のように、ごく薄く重ねて使う。"""
    rng = np.random.default_rng(seed)
    noise = rng.normal(0.5, 0.16, (size, size))
    noise = np.clip(noise, 0.0, 1.0)

    rgba = np.zeros((size, size, 4), dtype=np.uint8)
    rgba[..., 0] = rgba[..., 1] = rgba[..., 2] = (noise * 255).astype(np.uint8)
    rgba[..., 3] = 255
    return rgba


def main():
    # 枡は白木。明るく、年輪は細かい
    save(make_wood(SIZE, seed=20260918, light=(228, 201, 160), dark=(176, 140, 96),
                   ring_count=12.0, pore_strength=0.24, roughness=0.09,
                   warp_amount=0.8, pore_cells=220), "wood_masu.png")

    # 床は茶室に合わせて畳表にする。器の近くは焦点が合うので、ぼかしはごく軽く
    save(blur(make_tatami(SIZE, seed=404), 1), "tatami.png")

    save(make_water_normal(SIZE, seed=31415), "water_normal.png")
    save(make_vignette(512), "vignette.png")
    save(make_grain(512, seed=99), "grain.png")

    # 花月の札
    card_size = (256, 384)
    save(make_card("先攻", card_size, rng_seed=11), "card_first.png")
    save(make_card("後攻", card_size, rng_seed=12), "card_second.png")
    save(make_card_back(card_size, rng_seed=13), "card_back.png")


main()
