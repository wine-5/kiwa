# CLAUDE.md

このファイルは Claude AI がこのリポジトリで作業する際に参照するコンテキストです。

---

## プロジェクト概要

テーマ「かさ」のゲームジャム作品。DxLib を使用した Windows 専用アプリケーション。

ゲームの内容（ジャンル・ルール）は未定。決まるまでは基盤のみを整備する。
学習目的でアーキテクチャを選ぶため、Game 層の設計方針はゲーム内容の決定後に定める。

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
- ソースは **UTF-8（BOM 付き）**。DxLib 側も `SetUseCharCodeFormat(DX_CHARCODEFORMAT_UTF8)` で
  UTF-8 に揃えてあるため、日本語の文字列リテラルをそのまま描画できる

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
