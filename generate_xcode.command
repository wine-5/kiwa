#!/bin/sh
#
# Xcode プロジェクトを作って開く（Mac で使う）
#
# Finder でこのファイルをダブルクリックすれば動く。
# DXライブラリ iOS 版の場所だけ、下の DXLIB_IOS_DIR を自分の置き場所に直すこと。

set -e

# ---- ここだけ自分の環境に合わせる ----
DXLIB_IOS_DIR="$HOME/Downloads/DxLib_iOS"

cd "$(dirname "$0")"

if [ ! -d "$DXLIB_IOS_DIR" ]; then
	echo "DXライブラリ iOS 版が見つかりません: $DXLIB_IOS_DIR"
	echo "generate_xcode.command を開いて、DXLIB_IOS_DIR を置き場所に直してください。"
	exit 1
fi

if ! command -v cmake > /dev/null 2>&1; then
	echo "cmake がありません。先に入れてください: brew install cmake"
	exit 1
fi

cmake -G Xcode -B build-ios \
	-DCMAKE_SYSTEM_NAME=iOS \
	-DDXLIB_IOS_DIR="$DXLIB_IOS_DIR"

echo ""
echo "できました。Xcode を開きます。"
echo "  ターゲット Kiwa を選び、Signing & Capabilities で Team を設定してから実行してください。"

open build-ios/Kiwa.xcodeproj
