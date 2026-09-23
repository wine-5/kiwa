# 際 -KIWA- を Xcode で組む

Mac でこのフォルダを開いて、iOS 向けに組むための手順です。
Windows 版と**同じソースをそのまま使います**（分かれているのは入口とフォント登録だけ）。

> **このフォルダを Xcode で直接開くことはできません。**
> `.xcodeproj` はまだ入っていないので、先に CMake に作らせます。
> **ターミナルで下の3行を打つのが確実です**（`generate_xcode.command` をダブルクリックする手も
> ありますが、Zip で受け取ったファイルは macOS が実行を止めるので、ひと手間要ります。
> 「3. うまくいかないときに見るところ」を参照）。

---

## 1. 用意するもの

| もの | 備考 |
|---|---|
| Mac | Xcode 15 以降が動くもの |
| Xcode | App Store から |
| CMake | `brew install cmake`（3.20 以降） |
| **DXライブラリ iOS 版** | 公式（https://dxlib.xsrv.jp/dxdload.html）の「**ＤＸライブラリ iOS版**」（約78MB）を落として展開しておく。Windows 版とは別物なので注意 |
| Apple Developer Program | 実機とストアに出すために必要（年額あり） |

DXライブラリ iOS 版は**このフォルダには入っていません**。容量が大きく、版も変わるためです。
展開したら、**その場所（展開してできたフォルダ）**をあとで渡します。
中の並びは版によって違いますが、こちらで `DxLib.h` と `.a` を探しに行くので、
**展開先のフォルダを渡せば足ります**。どこに展開したか分からなくなったら、次で探せます。

```sh
find ~/Downloads -name DxLib.h -maxdepth 5
```

渡すのは、**展開してできた一番上のフォルダ**（例：`~/Downloads/DxLib_iOS`）で構いません。

iOS 版は中がこう分かれています。

```
DxLib_iOS/
  プロジェクトに追加すべきファイル_iOS用/
    Debug-iphoneos/            実機・デバッグ用
    Release-iphoneos/          実機・製品用
    Debug-iphonesimulator/     シミュレータ・デバッグ用
    Release-iphonesimulator/   シミュレータ・製品用
```

**どれを使うかはビルドのたびに変わる**ので、こちらでは決め打ちにせず、
Xcode に選ばせる形（`$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)`）で渡しています。
実機とシミュレータを行き来しても、そのまま通ります。

---

## 2. Xcode プロジェクトを作る

`.xcodeproj` を手で書くのではなく、CMake に作らせます。
ソースが増えても並びを直す必要がなく、取りこぼしも起きません。

```sh
cd ~/Downloads/Kiwa_IOS          # このフォルダを置いた場所

# DxLib.h がどこにあるか確かめる
find ~/Downloads -maxdepth 6 -name DxLib.h

# 出てきたパスのフォルダ部分を、そのまま次の行へ貼る
DXLIB=~/Downloads/DxLib_iOS

cmake -G Xcode -B build-ios -DCMAKE_SYSTEM_NAME=iOS -DDXLIB_IOS_DIR="$DXLIB"

open build-ios/Kiwa.xcodeproj
```

> `< >` で囲んだ書き方をそのまま打つと、ターミナルが記号として読んでしまい
> `parse error` になります。**山かっこは書かず、実際のパスに置き換えてください**。

あとは Xcode で

1. ターゲット `Kiwa` を選ぶ
2. **Signing & Capabilities** で Team を選ぶ（Bundle Identifier は `com.example.kiwa` から自分のものへ）
3. 実機か Simulator を選んで ⌘R

---

## 3. うまくいかないときに見るところ

### 「マルウェアが含まれていないことを検証できませんでした」と出る

`generate_xcode.command` をダブルクリックしたときに出ます。Zip で受け取ったファイルには
macOS が印を付けており、そのままでは実行させてくれません（中身の問題ではありません）。

**ターミナルで直に打つのが手っ取り早い**ですが、どうしてもダブルクリックで使いたければ
一度だけ次を実行してください。

```sh
xattr -dr com.apple.quarantine ~/Downloads/Kiwa_IOS
chmod +x ~/Downloads/Kiwa_IOS/generate_xcode.command
```

Windows で作った Zip は実行の権限も落ちるので、`chmod +x` も要ります。

### `iphoneos is not an iOS SDK` と出る

CMake が iOS の SDK を見つけられていません。**Xcode 本体ではなくコマンドラインツールのほうを
向いている**のが、たいていの原因です（`-G Xcode` には Xcode 本体が要ります）。

```sh
xcode-select -p
```

これが `/Library/Developer/CommandLineTools` を返したら、Xcode 本体へ向け直します。

```sh
sudo xcode-select -s /Applications/Xcode.app/Contents/Developer
sudo xcodebuild -license accept          # 初回だけ聞かれる
xcrun --sdk iphoneos --show-sdk-path     # パスが返れば直っている
```

最後の行がパスを返すようになってから、`build-ios` を作り直してください。

```sh
rm -rf build-ios
cmake -G Xcode -B build-ios -DCMAKE_SYSTEM_NAME=iOS -DDXLIB_IOS_DIR=~/Downloads/DxLib_iOS
```

（`xcode-select -p` の時点で Xcode が入っていないようなら、App Store から入れてください）

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
