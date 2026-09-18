# CLAUDE.md

このファイルは Claude AI がこのリポジトリで作業する際に参照するコンテキストです。

---

## プロジェクト概要

テーマ「かさ」のゲームジャム作品「**際 -KIWA- 注ぎ勝負**」。
お題を「嵩」と読み替え、枡へ交互に注ぎ、こぼした方が負けという勝負にする。
DxLib を使用した Windows 専用アプリケーション。

詳細: [game_concept.md](docs/design/game_concept.md)

---

## アーキテクチャ

レイヤードアーキテクチャ。

```
platform → infrastructure → game → core
```

- **Core 層**: インターフェース定義・数学ユーティリティ・定数（DxLib に依存しない）
- **Game 層**: ゲームロジック・シーン
- **Infrastructure 層**: DxLib を使った描画・入力・リソース管理
- **Platform 層**: Windows API の処理（現時点では未使用）

**重要**: 内側の層が外側の層をインクルードしてはいけない。DxLib や Windows API は Game 層から直接触れない。

依存は Singleton やグローバルな取得口を使わず、`Application`（コンポジションルート）が
実体を所有してコンストラクタで注入する。シーンへは `SceneContext` にまとめて手渡す。

### Game 層の設計（MVP / Passive View）

| 役 | 置き場 | 責任 |
|---|---|---|
| Model | `game/model/` | ルールだけ。時間も乱数も描画も知らない |
| View | `game/view/` | 渡されたものを映すだけ。Model も入力も見ない |
| Presenter | `game/presenter/` | 入力を読んで Model へ伝え、Model の状態を View へ渡す |

View を差し替えれば画面を出さずに進行を動かせる状態を保つこと。
液面の揺れのような「見た目だけの動き」は View 側が自前の時間（`advance`）で持ち、
Model や Presenter には持ち込まない。

### 描画の流れ

1. `IPostEffect::begin()` で別の面へ切り替える
2. `IScene::draw()` … 3D を描く
3. `IPostEffect::end()` … 明るいところを滲ませて画面へ戻す
4. `IScene::drawOverlay()` … 文字や周辺減光を重ねる（滲ませたあとに描く）

3D は `IRenderer3D` 越しに描く。出来合いの形で足りないもの（波打つ水面・注ぎ筋）は
`drawTriangles` に頂点を渡して自前で組み、陰影も頂点色に焼き込む。
器や急須のように形の決まったものは `IModelRenderer` でモデルとして読む。

### メインループ

更新は固定タイムステップ（1/60秒）、描画はフレームに1回。
`ScreenFlip` が垂直同期を待つため描画回数はモニタ依存になるが、更新は実経過時間を
貯めて 1/60 秒ぶんずつ回すので、モニタが何Hzでもゲーム内時間の進み方は一定になる。

---

## 命名規則

| 対象 | 規則 | 例 |
|---|---|---|
| ファイル名 | PascalCase | `TitleScene.cpp`, `SceneManager.h` |
| フォルダ名 | 小文字 | `src/`, `game/`, `scene/` |
| クラス名 | PascalCase | `SceneManager` |
| インターフェース名 | `I` + PascalCase | `IInputProvider` |
| 関数名 | camelCase | `getWidth()`, `changeScene()` |
| ローカル変数 | camelCase | `deltaTime` |
| メンバ変数 | `m_` + camelCase | `m_currentScene` |
| 静的メンバ変数 | `s_` + camelCase | `s_instance` |
| 定数 / constexpr | UPPER_SNAKE_CASE | `RENDER_WIDTH`, `FIXED_TIME_STEP` |
| 列挙型（型名・値） | PascalCase | `SceneType::Title` |
| 名前空間 | 小文字 | `namespace game::scene` |

マクロは使用禁止。定数は `constexpr`、インクルードガードは `#pragma once` を使用。

---

## コーディング規則

- メンバ変数・ローカル変数は **Uniform Initialization（`{}`）** で初期化する
- **`if` の中身が1行の場合は `{}` を省略する**（複数行のときのみブロックにする）
- public 関数には **Doxygen コメント**（`@brief`, `@param`, `@return`）を記載する
- スマートポインタは `make_unique` / `make_shared` を使用する
- ソースは **UTF-8（BOM 付き）**。BOM は必須。付け忘れると MSVC がソースを CP932 として読み、
  日本語コメント中のバイトが行継続と解釈されて次の行の宣言ごと消え、原因の分かりにくい
  コンパイルエラーになる
- 実行時の文字コードは `/execution-charset:utf-8` と `SetUseCharCodeFormat(DX_CHARCODEFORMAT_UTF8)`
  で UTF-8 に揃えてある。`/utf-8`（ソース側も UTF-8 に固定）は使わないこと。
  Shift-JIS で書かれた `DxLib.h` のコメントが壊れて大量の警告が出る

---

## 素材の生成

素材は外から持ってこず、ツールで生成して `assets/` へ書き出す。パラメータを変えて
作り直せるので、見た目の調整はツール側を直す。

```
python tools/make_textures.py   # 木目・周辺減光・粒状感・水面の法線
python tools/make_models.py     # 急須・湯呑・茶托（MQO 形式）
```

モデルは MQO（テキスト形式）で書き出す。DxLib が確実に読めて、中身を目で追えるため。
Blender や Metasequoia で開いて手直しすることもできる。

---

## ビルド

- Visual Studio 2022 以降（PlatformToolset v143）、x64 のみ。C++20
- `KasaGameJam.sln` を開いてビルドする。出力は `build/x64/<Configuration>/`
- DxLib は `thirdparty/dxlib/` に同梱済み（インストール不要）
- コマンドラインからは次のとおり

```
MSBuild.exe KasaGameJam.sln /p:Configuration=Debug /p:Platform=x64
```

---

## コミット規則

- Conventional Commits の `<type>: <subject>` 形式、日本語・一行のみ。スコープは付けない
- 命令形・末尾のピリオドなし・50文字以内
- `Co-Authored-By` トレーラーは書かない
- 見た目だけの調整は `style:`、パラメータ調整は `chore:`（`fix:` は不具合修正に限る）
- 1コミット＝1つの小さな部品。土台とそれを使う振る舞いは別コミット。リファクタは独立コミット
- 実装 → 即コミット → 実装 → 即コミット → 最後にビルドチェック
- `git add` は自分が変更したファイルのみを明示列挙する（`git add -A` / `.` / `commit -a` は使わない）
- **push は絶対にしない**
