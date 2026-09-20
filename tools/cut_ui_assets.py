"""assets/source/ の下絵から、UI の画像を1枚ずつ切り出す。

生成AIに描いてもらった絵は、灰色の地の上に素材が並んだ「アセットシート」の形で
届く。そのままでは使えないので、ここで

    1. 並んでいる素材を位置で切り出し
    2. 灰色の地を透明に置き換え（縁のぼけた部分は半透明にする）
    3. 余白を詰める

という手順で 1枚ずつの透過 PNG にして assets/textures/ui/ へ書き出す。

    python tools/cut_ui_assets.py

下絵を描き直してもらったら、SHEET_ITEMS の枠を直して実行し直す。
拡大はしない（元より大きくしてもぼけるだけなので）。
"""

import os

import numpy as np
from PIL import Image
from scipy import ndimage

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SOURCE_DIR = os.path.join(ROOT, "assets", "source")
OUTPUT_DIR = os.path.join(ROOT, "assets", "textures", "ui")

# シートの地の色。素材と地を見分ける基準にする
SHEET_BACKGROUND = np.array([93.5, 94.0, 93.0], dtype=np.float32)

# 地とみなす色の幅。これを超えて離れていれば素材とみなす
BACKGROUND_TOLERANCE = 14.0

# 縁のぼけを半透明に写し取る幅。大きいほど柔らかくつながる
EDGE_SOFTNESS = 46.0

# シート上の位置（左, 上, 幅, 高さ）と、画面で使うときの大きさ
SHEET_ITEMS = [
	("card_second",       (197,  51, 132, 261), (210, 315)),
	("card_back",         (357,  51, 140, 241), (210, 315)),
	("title_logo",        (589,  53, 288, 217), (720, 320)),
	("turn_plate",        ( 34, 404, 530, 101), (520, 110)),
	("score_plate",       (609, 415, 285,  93), (300,  80)),
	("emblem_one",        ( 49, 526, 134, 149), (128, 128)),
	("emblem_two",        (257, 529, 136, 145), (128, 128)),
	("key_cap_space",     (447, 558, 266,  95), (200,  72)),
	("key_cap_enter",     (748, 558, 180,  95), (140,  72)),
	("result_scroll",     (976,  57, 535, 393), (760, 420)),
	("seal_victory",      (996, 519, 196, 206), (180, 180)),
]

# 1枚で届いた絵。地が透明なので切り出しだけでよい
SINGLE_ITEMS = [
	("card_first", "card_first.png", (210, 315)),
]


def cut_background(rgb):
	"""灰色の地を透明にした RGBA を返す。

	縁がぼけている素材をそのまま切ると、灰色を含んだ輪郭が残ってしまう。
	地の色が分かっているので、どれだけ混ざっているかから不透明度を割り出し、
	混ざったぶんを差し引いて元の色を取り戻す。
	"""
	distance = np.abs(rgb - SHEET_BACKGROUND).max(axis=2)

	# 中身は塗りつぶして、穴が開かないようにする
	solid = ndimage.binary_opening(distance > BACKGROUND_TOLERANCE, np.ones((3, 3)))
	solid = ndimage.binary_fill_holes(ndimage.binary_closing(solid, np.ones((9, 9))))

	alpha = np.clip(distance / EDGE_SOFTNESS, 0.0, 1.0)
	alpha = np.where(solid, 1.0, alpha)

	# 地の色が混ざったぶんを戻す
	safe = np.maximum(alpha, 1.0 / 255.0)[:, :, None]
	color = (rgb - SHEET_BACKGROUND * (1.0 - safe)) / safe

	rgba = np.zeros(rgb.shape[:2] + (4,), dtype=np.uint8)
	rgba[..., :3] = np.clip(color, 0, 255).astype(np.uint8)
	rgba[..., 3] = (alpha * 255).astype(np.uint8)
	return rgba


def trim(image, threshold=6):
	"""まわりの透明な余白を詰める。"""
	alpha = np.asarray(image)[..., 3]
	rows = np.where(alpha.max(axis=1) > threshold)[0]
	columns = np.where(alpha.max(axis=0) > threshold)[0]
	if len(rows) == 0 or len(columns) == 0:
		return image
	return image.crop((columns[0], rows[0], columns[-1] + 1, rows[-1] + 1))


def fit(image, size):
	"""画面で使う大きさに合わせる。元より大きくはしない。"""
	scale = min(size[0] / image.width, size[1] / image.height, 1.0)
	if scale >= 1.0:
		return image
	return image.resize((max(1, round(image.width * scale)),
	                     max(1, round(image.height * scale))), Image.LANCZOS)


def save(image, name):
	os.makedirs(OUTPUT_DIR, exist_ok=True)
	path = os.path.join(OUTPUT_DIR, name + ".png")
	image.save(path)
	print("書き出し: %s (%d×%d)" % (path, image.width, image.height))


def main():
	sheet = Image.open(os.path.join(SOURCE_DIR, "ui_sheet.png")).convert("RGB")

	for name, box, size in SHEET_ITEMS:
		left, top, width, height = box
		# 縁のぼけを拾えるよう、枠より少し広く切ってから余白を詰める
		margin = 6
		region = sheet.crop((left - margin, top - margin,
		                     left + width + margin, top + height + margin))
		cut = Image.fromarray(cut_background(np.asarray(region).astype(np.float32)))
		save(fit(trim(cut), size), name)

	for name, file_name, size in SINGLE_ITEMS:
		image = Image.open(os.path.join(SOURCE_DIR, file_name)).convert("RGBA")
		save(fit(trim(image), size), name)


main()
