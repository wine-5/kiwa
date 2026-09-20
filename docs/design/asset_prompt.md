# UI アセット：出来たものと、まだ要るもの

アセットシートから切り出した12点は `assets/textures/ui/` に入っている。
切り出しは `python tools/cut_ui_assets.py` でやり直せる（元の絵は `assets/source/`）。

## 出来ているもの

| 名前 | 大きさ | 画面で使う大きさ | 足りているか |
|---|---|---|---|
| `card_first` | 210×301 | 210×315 | ○（単体でもらった高解像度のもの） |
| `card_second` | 134×223 | 210×315 | **×** 先攻の札と大きさも形も揃っていない |
| `card_back` | 141×225 | 210×315 | **×** 同上 |
| `title_logo` | 300×229 | 720×320 | **×** 引き伸ばすとぼける |
| `result_scroll` | 536×373 | 760×420 | △ 少し足りない |
| `turn_plate` | 520×87 | 520×110 | ○ |
| `score_plate` | 287×75 | 300×80 | ○ |
| `emblem_one` / `emblem_two` | 128×124 / 128×122 | 128×128 | ○ |
| `key_cap_space` / `key_cap_enter` | 200×57 / 140×59 | 200×72 / 140×72 | ○ |
| `seal_victory` | 180×180 | 180×180 | ○ |

## まだ要るもの（4点）

下の `---` から下をそのままコピーして ChatGPT に渡す。**先攻の札の画像も一緒に添える**こと。

---

# Request

Thank you — the asset sheet and the single 先攻 card you made are exactly the style I want.
I now need **4 more PNG files** to finish the set. Please keep the identical art direction.

**The most important thing:** the first three must look like they came from the same deck as the
先攻 card you already made (attached). Same paper, same stains, same ink rule, same corner
ornaments, same proportions — only the centre changes.

## Technical requirements

- **Transparent background (PNG with alpha).** No grey backdrop baked in; torn and blotted edges
  must fade into alpha, never into grey.
- **Exact pixel sizes** as listed. Do not pad, letterbox or square-crop.
- One item per file, no captions, no file names drawn into the image.
- Straight-on and flat. No perspective, no drop shadow.

**Palette:** ink black `#1C1A18`, washi off-white `#E8E0CB`, straw `#C6BE8E`,
celadon `#78857F`, lacquer brown `#2E1A16`, vermilion `#9C2B2B`, tea green `#96AC3E`.

## The files

1. `card_front_second` — **840×1260**. The attached 先攻 card, redrawn with **後攻** in the centre
   instead. Everything else must match the attached card as closely as you can: the game
   cross-fades between the two, so any shift in the paper or the border will be visible as a jump.
2. `card_back` — **840×1260**. The same card seen face down. No characters. In the centre, a
   single *enso* — a circle drawn in one brush stroke, with a visible dry-brush break and a gap
   where the stroke does not close — in soft, thin ink. Low contrast, but clearly hand-drawn.
3. `title_logo` — **1440×640**. The title **際** as one huge brush character with **-KIWA-**
   underneath, exactly as on the asset sheet, but drawn at this full size. The single character
   should look written in one breath, with a dry-brush tail.
4. `result_scroll` — **1520×840**. The hanging scroll from the sheet, at full size: wooden rods
   top and bottom, patterned silk border, old and slightly stained. The paper panel in the middle
   must be **completely empty** — the winner's name is written there by the game.

If anything is ambiguous, choose the quieter, more restrained option.
