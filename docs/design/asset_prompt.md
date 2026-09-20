# UI アセット生成用プロンプト（ChatGPT へ渡す英文）

アセットシートで絵柄は決まったので、ここからは**実際に使う10点を個別の透過PNG**で
もらうための指示文。`---` から下をそのままコピーして使う。

前回のシートから外したもの（使わないので生成しなくてよい）

| 外したもの | 理由 |
|---|---|
| `title_subtitle_plate` | `turn_plate` を流用すれば足りる |
| `gauge_frame` / `gauge_fill` | 際（こぼれる量）は伏せるのが要。残量計は設計と矛盾する。嵩は3Dの液面そのもので見せる |
| `ink_splash_transition` | 場面転換の演出がまだない。入れるときに改めて頼む |
| `overlay_washi` / `overlay_vignette` | `tools/make_textures.py` で生成済み（`grain.png` / `vignette.png`） |

---

# Request

Thank you — the style on the asset sheet you made is exactly right. Please keep that
**identical** art direction, and now export the following **10 items as individual PNG files**.

The sheet was for approving the look; these files are the ones that actually go into the game,
so the technical requirements below matter more than anything else.

---

## 1. What the game is (for context)

**Title:** 際 -KIWA-　(read "kiwa" — "the brink, the very edge")

Two players kneel across a single cup of green tea on a tatami floor in a dark tea room, lit by
one small warm lamp. They take turns pouring tea from a celadon teapot into that one cup. The cup
has a **hidden** limit that changes every round — nobody knows how much it can take before it
spills. On your turn you must pour at least a little; you cannot pass. **Whoever makes the tea
spill loses the round.** First to three rounds wins. Before the match, the players draw a lot to
decide who pours first.

The feeling is stillness and restraint with rising tension — a tea ceremony, not a party game.

---

## 2. Technical requirements (please follow exactly)

- **Transparent background (PNG with alpha).** The grey backdrop of the asset sheet must not be
  baked in. Soft, torn or splattered edges must fade into *alpha*, never into grey.
- **Exact pixel sizes** as listed. These are 2× the size used on a 1280×720 screen, so the
  proportions must be kept precisely — please do not letterbox, pad or square-crop them.
- **One item per file**, centred, with only a few pixels of empty margin so nothing is clipped.
- **No labels.** Do not draw the file name, size or any caption into the image.
- Straight-on and flat. No perspective, no drop shadow onto the background, no mock-up scene.
- Where the list says "leave empty", the area must be genuinely blank — the game draws the text
  itself with a brush font, and baked-in text would collide with it.

**Palette (unchanged):** ink black `#1C1A18`, washi off-white `#E8E0CB`, straw `#C6BE8E`,
celadon `#78857F`, lacquer brown `#2E1A16`, vermilion `#9C2B2B`, tea green `#96AC3E`.

---

## 3. The 10 files

### A. Lot-drawing cards （花月の札）

Drawn face down at the start of the match, then flipped to decide who pours first. All three must
be **exactly the same size and the same paper**, because the game cross-fades between them.

1. `card_front_first` — **420×630**. Washi card, thin ink rule inset from the edge, the two
   characters **先攻** written vertically in the centre in thick brush calligraphy.
2. `card_front_second` — **420×630**. The same card with **後攻**.
3. `card_back` — **420×630**. The same paper face down. Please make the centre mark stronger than
   on the sheet: a real *enso* drawn in one brush stroke, with a visible dry-brush break and an
   unclosed gap, in soft ink — not a clean grey ring.

### B. Title

4. `title_logo` — **1440×640**. The title **際** as one huge brush character with **-KIWA-**
   underneath in a thin, restrained style, exactly as on the sheet. This one was perfect — please
   just re-export it at this size on transparency.

### C. In-game UI

5. `turn_plate` — **1040×220**. A *tanzaku* paper strip that holds the current player's name.
   Two changes from the sheet: please **remove the metal rivets**, and keep the **middle 60% of
   the strip plain and uniform** (only the left and right ends decorated), because the game
   stretches the middle to fit the text. The paper area must be empty.
6. `score_plate` — **600×160**. The same idea, smaller, same rules: plain stretchable middle,
   no rivets, no text.
7. `emblem_player_one` — **256×256**. Circular *kamon*-style emblem: one bold brush ring with a
   single horizontal stroke 一 inside.
8. `emblem_player_two` — **256×256**. The matching emblem with two strokes 二 inside. The pair
   must stay readable when shrunk to 32×32, so keep the ring thick and the inner strokes bold.

### D. Result

9. `result_scroll` — **1520×840**. A hanging scroll (*kakejiku*) seen flat and straight on, with
   plain wooden rods top and bottom, old and slightly stained. The paper panel in the middle must
   be **completely empty** — the winner's name is written there by the game.
10. `seal_victory` — **360×360**. The vermilion seal with **勝**, in the rough blotted style of a
    real stone seal. The sheet version is good; please export it with the red ink alone on
    transparency, keeping the broken, uneven edges of the stamp.

---

## 4. Delivery

Individual PNG files at the exact sizes above, named exactly as listed, each with a transparent
background. No contact sheet this time.

If anything is ambiguous, choose the quieter, more restrained option.
