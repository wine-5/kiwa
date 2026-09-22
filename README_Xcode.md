# 際 -KIWA- を Xcode で組む

Mac でこのフォルダを開いて、iOS 向けに組むための手順です。
Windows 版と**同じソースをそのまま使います**（分かれているのは入口とフォント登録だけ）。

> **このフォルダを Xcode で直接開くことはできません。**
> `.xcodeproj` はまだ入っていないので、先に CMake に作らせます。
> いちばん簡単なのは、Mac で **`generate_xcode.command` をダブルクリック**することです
> （中の `DXLIB_IOS_DIR` を、DXライブラリ iOS 版を置いた場所に直してから）。

---

## 1. 用意するもの

| もの | 備考 |
|---|---|
| Mac | Xcode 15 以降が動くもの |
| Xcode | App Store から |
| CMake | `brew install cmake`（3.20 以降） |
| **DXライブラリ iOS 版** | 公式（https://dxlib.xsrv.jp/dxdload.html）から落として展開しておく |
| Apple Developer Program | 実機とストアに出すために必要（年額あり） |

DXライブラリ iOS 版は**このフォルダには入っていません**。容量が大きく、版も変わるためです。
展開した場所（`DxLib.h` があるところ）をあとで渡します。

---

## 2. Xcode プロジェクトを作る

`.xcodeproj` を手で書くのではなく、CMake に作らせます。
ソースが増えても並びを直す必要がなく、取りこぼしも起きません。

```sh
cd ~/Desktop/Kiwa_AppStore

cmake -G Xcode -B build-ios \
  -DCMAKE_SYSTEM_NAME=iOS \
  -DDXLIB_IOS_DIR=$HOME/Downloads/DxLib_iOS

open build-ios/Kiwa.xcodeproj
```

あとは Xcode で

1. ターゲット `Kiwa` を選ぶ
2. **Signing & Capabilities** で Team を選ぶ（Bundle Identifier は `com.example.kiwa` から自分のものへ）
3. 実機か Simulator を選んで ⌘R

---

## 3. うまくいかないときに見るところ

### ライブラリが見つからない / リンクが通らない

`CMakeLists.txt` の `target_link_libraries` に並べてあるライブラリ名と枠組みは、
**DXライブラリ iOS 版の版によって変わります**。公式の手引き

- Xcode 15.2 版 https://dxlib.xsrv.jp/use/dxuse_xcode15_2_iOS.html

にある `Other Linker Flags` の並びに合わせて、`CMakeLists.txt` を直してください。

### 起動はするが、絵や音が出ない（素材が見つからない）

`assets/` はバンドルの `Resources/assets/...` へ、**階層を保ったまま**入るようにしてあります。
それでも見つからない場合は、**実行時のカレントディレクトリがバンドルの中を指していない**のが原因です。

直す場所は一箇所です。`src/infrastructure/resource/ResourceManager.cpp` の
`loadTexture` / `loadSound` / `loadModel` が受け取るパスの頭に、バンドルの場所を足してください。
パスの文字列は `src/game/constant/` にまとめてあるので、**コードの他の場所は触らずに済みます**。

### 入口

DXライブラリの iOS 版は、通常の `main` ではなく **`int ios_main(void)`** を呼びます。
`src/platform/ios/IosMain.cpp` に用意済みで、中身は Windows と同じ `bootstrap::run()` を呼ぶだけです。

---

## 4. まだ手を入れていないこと（要対応）

### 操作（触り心地は実機で見てほしい）

画面に触れる実装（`src/infrastructure/input/TouchInputProvider`）は**用意済み**で、
iOS では自動でそちらが使われます（`PlatformFactory` が選びます）。

| 触れたこと | 読み替え先 |
|---|---|
| 指の位置 | マウスの位置（並びや札を直接押して選べる） |
| 触れている | マウスの左ボタン |
| 触れている | 注ぐキー（画面のどこを触っても、いまの手番の人が注ぐ） |

注ぐキーは二人とも同じなので、一台を二人で囲む遊び方もそのまま動きます。

実機で触ってみて、次のあたりは調整が要るかもしれません。

- 注げる場所を画面の下半分だけに限る（メニューを押したつもりで注いでしまう場合）
- 休みへ入る札の当たりを広げる（指では小さい場合）

### 音

mp3 を使っています。iOS 版で読めるかを最初に確かめてください。
駄目なら ogg か wav に変換し、`src/game/constant/Sounds.h` の拡張子を直します。

### 画面の比

1280×720 で組んであり、HUD は画面サイズに対する割合で置いています。
端末の比が違うときに端が切れないかを見てください。

### アイコンと出品物

- アイコン … `assets/icon/kiwa.png`（1024px）があります。Xcode の Assets に入れてください
- スクリーンショット、年齢レーティング、プライバシー情報は App Store Connect で登録します

---

## 5. 中身の地図

```
src/core          どの OS でも動く（決まりごと、数学、口の定義）
src/game          遊びの中身（Model / View / Presenter、シーン）
src/infrastructure DxLib を使う描画・入力・資源・音
src/platform      OS ごとのもの
  ├ ios/          iOS の入口
  ├ windows/      Windows の入口とフォント登録（Xcode では組まれません）
  └ PlatformFactory.cpp   OS ごとの分かれ道はここ一箇所だけ
assets            絵・音・モデル・書体
```

`src/core` と `src/game`（80ファイル）は**そのまま動きます**。
手を入れるのは `src/infrastructure` と `src/platform` だけです。
詳しくは `docs/porting.md` を見てください。
