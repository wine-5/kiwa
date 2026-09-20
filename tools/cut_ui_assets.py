"""assets/source/ の下絵から、UI の画像を1枚ずつ切り出す。

生成AIに描いてもらった絵は、灰色の地の上に素材が並び、それぞれの下に名前と寸法が
刷られた「アセットシート」の形で届く。そのままでは使えないので、ここで

    1. 並んでいる素材を位置で切り出し
    2. 灰色の地を透明に置き換え（にじんだ縁は半透明にする）
    3. 下に刷られた説明書きを落とし、余白を詰める

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

# ここまでの色の違いは地とみなす（透明にする）
BACKGROUND_TOLERANCE = 8.0

# ここまで離れていれば素材とみなす（不透明にする）。
# 間は半透明にして、にじんだ縁や毛筆のかすれをそのまま写し取る
SOLID_DISTANCE = 30.0

# 素材の行とみなす横幅の割合（いちばん広い行に対して）
ART_WIDTH_RATIO = 0.7

# 素材の端（角の丸みやちぎれた縁）とみなす横幅の割合
ART_EDGE_RATIO = 0.06

# 説明書きを探すために、枠の下へ余分に見る行数
CAPTION_SEARCH = 24

# 余白を詰めるときに、素材の縁とみなす不透明度
EDGE_THRESHOLD = 24

# 素材の形がひとつながりかどうか。
# 紙や器物のように外形が閉じているものは、中を塗りつぶして不透明にする。
# 墨の字や印のように中に隙間があるものは、隙間を埋めずにそのまま残す
CLOSED = True
OPEN = False

# シート上の位置（左, 上, 幅, 高さ）と、外形の扱いと、画面で使うときの大きさ
SHEET_ITEMS = [
	("card_second",   (197,  51, 132, 261), CLOSED, (210, 315)),
	("card_back",     (357,  51, 140, 241), CLOSED, (210, 315)),
	("title_logo",    (589,  56, 290, 218), OPEN,   (720, 320)),
	("turn_plate",    ( 34, 404, 530, 101), CLOSED, (520, 110)),
	("score_plate",   (609, 415, 285,  93), CLOSED, (300,  80)),
	("emblem_one",    ( 49, 526, 134, 149), CLOSED, (128, 128)),
	("emblem_two",    (257, 529, 136, 145), CLOSED, (128, 128)),
	("key_cap_space", (447, 558, 266,  95), CLOSED, (200,  72)),
	("key_cap_enter", (748, 558, 180,  95), CLOSED, (140,  72)),
	("result_scroll", (976,  57, 535, 393), CLOSED, (760, 420)),
	("seal_victory",  (996, 519, 196, 206), OPEN,   (180, 180)),
]

# 1枚で届いた絵。地がすでに透明なので、切り出しだけでよい
SINGLE_ITEMS = [
	("card_first", "card_first.png", (210, 315)),
]


def to_alpha(rgb, is_closed):
	"""灰色の地を透明にした RGBA を返す。

	にじんだ縁をそのまま切ると、灰色を含んだ輪郭が残ってしまう。地の色が
	分かっているので、どれだけ混ざっているかから不透明度を割り出し、混ざった
	ぶんを差し引いて元の色を取り戻す。

	外形が閉じている素材は、中を塗りつぶして不透明にする。そうしないと、
	地と近い色で描かれたところ（薄墨の円相など）が透けてしまう。
	"""
	distance = np.abs(rgb - SHEET_BACKGROUND).max(axis=2)
	alpha = np.clip((distance - BACKGROUND_TOLERANCE) / (SOLID_DISTANCE - BACKGROUND_TOLERANCE),
	                0.0, 1.0)

	if is_closed:
		outline = ndimage.binary_closing(distance > SOLID_DISTANCE, np.ones((5, 5)))
		alpha = np.maximum(alpha, ndimage.binary_fill_holes(outline).astype(np.float32))

	# 地の色が混ざったぶんを戻す
	safe = np.maximum(alpha, 1.0 / 255.0)[:, :, None]
	color = (rgb - SHEET_BACKGROUND * (1.0 - safe)) / safe

	rgba = np.zeros(rgb.shape[:2] + (4,), dtype=np.uint8)
	rgba[..., :3] = np.clip(color, 0, 255).astype(np.uint8)
	rgba[..., 3] = (alpha * 255).astype(np.uint8)
	return rgba


def art_band(rgb):
	"""素材が写っている行の範囲を返す。

	シートには素材の下に、名前と寸法が細い字で刷られている。素材と字のあいだには
	必ず地だけの行が挟まるので、

	    1. いちばん横に広い行がまとまって続くところを素材の胴とみなし
	    2. そこから上下へ、地だけの行に当たるまで広げる

	という順で範囲を決める。角の丸みやちぎれた縁のように、端へ行くほど細くなる
	形でも切り落とさずに済む。
	"""
	distance = np.abs(rgb - SHEET_BACKGROUND).max(axis=2)
	widths = (distance > SOLID_DISTANCE).sum(axis=1)
	if widths.max() == 0:
		return 0, rgb.shape[0]

	# 胴とみなせる行がいちばん長く続くところを探す
	wide = widths >= widths.max() * ART_WIDTH_RATIO
	best = (0, 0, 0)
	start = None
	for y, value in enumerate(np.append(wide, False)):
		if value and start is None:
			start = y
		elif not value and start is not None:
			if y - start > best[0]:
				best = (y - start, start, y)
			start = None

	top, bottom = best[1], best[2]

	# 地だけの行に当たるまで外へ広げる
	edge = max(1.0, widths.max() * ART_EDGE_RATIO)
	while top > 0 and widths[top - 1] >= edge:
		top -= 1
	while bottom < len(widths) and widths[bottom] >= edge:
		bottom += 1
	return top, bottom


def trim(image, threshold):
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

	for name, box, is_closed, size in SHEET_ITEMS:
		left, top, width, height = box
		# にじんだ縁を拾えるよう、枠より少し広く切る
		margin = 8
		below = CAPTION_SEARCH if is_closed else 0
		region = np.asarray(sheet.crop((left - margin, top - margin,
		                                left + width + margin,
		                                top + height + margin + below))).astype(np.float32)

		# 紙や器物は下に刷られた説明書きを切り離してから地を透明にする。
		# 墨の字や印は行ごとの幅がばらばらで見分けられないので、枠をそのまま使う
		if is_closed:
			top_row, bottom_row = art_band(region)
			region = region[top_row:bottom_row]

		cut = Image.fromarray(to_alpha(region, is_closed))

		save(fit(trim(cut, EDGE_THRESHOLD), size), name)

	for name, file_name, size in SINGLE_ITEMS:
		image = Image.open(os.path.join(SOURCE_DIR, file_name)).convert("RGBA")
		save(fit(trim(image, EDGE_THRESHOLD), size), name)


main()
