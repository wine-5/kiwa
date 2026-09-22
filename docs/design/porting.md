# 別の OS へ移すときの手順

いずれ iOS（App Store）へ出すことを見越して、**OS に依るところを一箇所へ寄せてある**。
その地図と、移すときに手を入れる場所をまとめる。

## いまの依存のかたち

```
platform → infrastructure → game → core
```

| 層 | ファイル数 | DxLib | OS の API |
|---|---|---|---|
| core | 20 | なし | なし |
| game | 59 | なし | なし |
| infrastructure | 22 | 9ファイル | なし |
| platform | 5 | 入口のみ | Windows のみ |

このほかに、ルートの `Application` と `Bootstrap` が DxLib を触る（ループと起動）。

**遊びの中身（core + game の79ファイル）は、どの OS でもそのまま動く。**
差し替えるのは infrastructure と platform、それとルートの2ファイルだけになっている。

## OS ごとに分かれている場所

### 1. 入口

- `src/platform/windows/WindowsMain.cpp` … `WinMain` があるだけ
- `src/Bootstrap.{h,cpp}` … DxLib を起こしてゲームを回し、後片付けまで

- `src/platform/ios/IosMain.cpp` … iOS の入口（用意済み）

**入口は OS ごとに別ファイルを置き、そこから `bootstrap::run()` を呼ぶ。**
DXライブラリの iOS 版は、通常の `main` ではなく **`int ios_main(void)`** を呼ぶ決まりなので、
その形で用意してある。`Bootstrap` には OS ごとの違いを持ち込まないこと。

### 2. フォントの登録

同梱した ttf を名前で呼べるようにする手順は OS ごとにまったく違う。

- `src/core/interface/IFontInstaller.h` … 口だけを決めたもの
- `src/platform/windows/WindowsFontInstaller.{h,cpp}` … `AddFontResourceExW`（FR_PRIVATE）
- `src/platform/PlatformFactory.cpp` … **どの OS のどれを作るかを決める唯一の場所**

iOS は同梱フォントをアプリの持ち物として扱えるので、`NullFontInstaller`（何もしない）で
済む見込み。足りなければ `IosFontInstaller` を足して `PlatformFactory` に一行加える。

### 3. DxLib を直に触る9ファイル

`infrastructure/graphics`（6）、`infrastructure/input`（1）、`infrastructure/resource`（1）、
`infrastructure/debug/FrameCapture`（1）。**DxLib の iOS 版でほぼそのまま通る見込み**で、
通らない関数があればそのファイルだけ直す。

## iOS へ移すときに残っている課題

1. **操作** — 画面に触れる実装（`infrastructure/input/TouchInputProvider`）を用意してある。
   指の位置をマウスの位置として、触れていることを注ぐキーとして渡すので、
   **画面のどこを触ってもいまの手番の人が注ぎ、並びは直接押して選べる**。
   DXライブラリでは触った指がマウスの関数では取れないため、触れた指を数える関数から読んでいる。
   実機で触り心地を見て、注ぐ場所を限る（画面の下半分だけにするなど）かどうかを決める
2. **音の形式** — mp3 が読めるかを確かめる。怪しければ ogg / wav へ変換する
3. **画面の比** — 1280×720 で組んであり、HUD は画面サイズに対する割合で置いている。
   端末の比が違うときに端が切れないかを見る
4. **ビルド** — vcxproj ではなく Xcode プロジェクトを用意する。
   ソースの一覧は `KasaGameJam.vcxproj` を見れば分かる。
   - テンプレートは iOS → App、Language は **Objective-C**、Interface は Storyboard
   - `Header Search Paths` と `Library Search Paths` に DXライブラリ iOS 版の場所を足す
   - `Other Linker Flags` に `-lDxLib_iOS` などを足す
   - **`src/platform/windows/` はターゲットに含めない**（Windows 専用のため）

## 足すときの約束

- **`#if defined(_WIN32)` のような分岐は `PlatformFactory.cpp` だけに置く。**
  あちこちに散ると、移すときに探し回ることになる
- game 層と core 層には、OS の API も DxLib も**絶対に持ち込まない**
