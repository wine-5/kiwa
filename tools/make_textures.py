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


def save(image, name):
    os.makedirs(OUTPUT_DIR, exist_ok=True)
    path = os.path.join(OUTPUT_DIR, name)
    Image.fromarray(image).save(path)
    print("書き出し:", path)


def main():
    # 枡は白木。明るく、年輪は細かい
    save(make_wood(SIZE, seed=20260918, light=(228, 201, 160), dark=(176, 140, 96),
                   ring_count=12.0, pore_strength=0.24, roughness=0.09,
                   warp_amount=0.8, pore_cells=220), "wood_masu.png")

    # 台は使い込まれた濃い木。年輪は粗い
    save(make_wood(SIZE, seed=771, light=(126, 104, 84), dark=(72, 56, 42),
                   ring_count=11.0, pore_strength=0.34, roughness=0.14,
                   warp_amount=1.8, pore_cells=180), "wood_table.png")

    save(make_water_normal(SIZE, seed=31415), "water_normal.png")


main()
