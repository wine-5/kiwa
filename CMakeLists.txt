# 際 -KIWA- を iOS（App Store）向けに組むための定義
#
# Xcode プロジェクトは、これを元に CMake が作る。手で .xcodeproj を書くより、
# ソースが増えたときの取り回しが楽で、取りこぼしも起きにくい。
#
#   cmake -G Xcode -B build-ios -DCMAKE_SYSTEM_NAME=iOS -DDXLIB_IOS_DIR=<DXライブラリiOS版の場所>
#   open build-ios/Kiwa.xcodeproj
#
# 詳しい手順は README_Xcode.md を読むこと。

cmake_minimum_required(VERSION 3.20)

project(Kiwa VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# DXライブラリ iOS 版の場所。リポジトリには入っていないので、外から渡す
set(DXLIB_IOS_DIR "" CACHE PATH "DXライブラリ iOS 版を展開した場所（DxLib.h があるところ）")

if(NOT DXLIB_IOS_DIR)
    message(FATAL_ERROR
        "DXライブラリ iOS 版の場所を渡してください：cmake ... -DDXLIB_IOS_DIR=/path/to/DxLib_iOS")
endif()

if(NOT EXISTS "${DXLIB_IOS_DIR}")
    message(FATAL_ERROR
        "その場所がありません: ${DXLIB_IOS_DIR}
"
        "  DXライブラリ iOS 版を https://dxlib.xsrv.jp/dxdload.html から落として展開し、
"
        "  展開した場所を -DDXLIB_IOS_DIR= に渡してください。")
endif()

# ---- DXライブラリの中身を探す ----
#
# 版によって中の並びが変わるので、名前を決め打ちにせず探しに行く。
# ヘッダは DxLib.h があるところ、ライブラリは中にある .a を全部使う
file(GLOB_RECURSE DXLIB_HEADER_CANDIDATES "${DXLIB_IOS_DIR}/DxLib.h" "${DXLIB_IOS_DIR}/*/DxLib.h")

if(NOT DXLIB_HEADER_CANDIDATES)
    message(FATAL_ERROR
        "${DXLIB_IOS_DIR} の中に DxLib.h が見つかりません。
"
        "  展開先を間違えていないか確かめてください（find で探せます）：
"
        "    find ~/Downloads -name DxLib.h -maxdepth 5")
endif()

list(GET DXLIB_HEADER_CANDIDATES 0 DXLIB_HEADER)
get_filename_component(DXLIB_INCLUDE_DIR "${DXLIB_HEADER}" DIRECTORY)
message(STATUS "DXライブラリのヘッダ: ${DXLIB_INCLUDE_DIR}")

file(GLOB_RECURSE DXLIB_LIBRARIES "${DXLIB_IOS_DIR}/*.a")

if(NOT DXLIB_LIBRARIES)
    message(FATAL_ERROR
        "${DXLIB_IOS_DIR} の中に .a（ライブラリ）が見つかりません。
"
        "  iOS 版ではなく Windows 版を展開していないか確かめてください。")
endif()

list(LENGTH DXLIB_LIBRARIES DXLIB_LIBRARY_COUNT)
message(STATUS "DXライブラリのライブラリ: ${DXLIB_LIBRARY_COUNT} 個")

# ---- ソース ----
#
# Windows 専用の入口とフォント登録は外す。中身（core / game）は触らずそのまま使う
file(GLOB_RECURSE KIWA_SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.cpp")
list(FILTER KIWA_SOURCES EXCLUDE REGEX "/src/platform/windows/")

add_executable(Kiwa MACOSX_BUNDLE ${KIWA_SOURCES})

target_include_directories(Kiwa PRIVATE
    "${CMAKE_SOURCE_DIR}/src"
    "${DXLIB_INCLUDE_DIR}"
)

# ---- DXライブラリと、それが要る枠組み ----
#
# ライブラリは上で見つけたものをそのまま渡す（名前を決め打ちにしない）。
# 枠組みの顔ぶれは版によって変わるので、足りなければここへ足す
target_link_libraries(Kiwa PRIVATE
    ${DXLIB_LIBRARIES}

    "-framework UIKit"
    "-framework Foundation"
    "-framework CoreGraphics"
    "-framework QuartzCore"
    "-framework OpenGLES"
    "-framework GLKit"
    "-framework AVFoundation"
    "-framework AudioToolbox"
    "-framework CoreMedia"
    "-framework CoreVideo"
    "-framework CoreMotion"
    "-framework ImageIO"
    "-framework MediaPlayer"
    "-framework GameController"
)

# ---- 素材 ----
#
# assets 以下をそのままの形でバンドルへ入れる。
# 階層を保たないと、コードが書いているパス（assets/textures/... など）で開けない
file(GLOB_RECURSE KIWA_ASSETS CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/assets/*")

foreach(asset ${KIWA_ASSETS})
    file(RELATIVE_PATH relative "${CMAKE_SOURCE_DIR}" "${asset}")
    get_filename_component(relative_dir "${relative}" DIRECTORY)
    set_source_files_properties("${asset}" PROPERTIES
        MACOSX_PACKAGE_LOCATION "Resources/${relative_dir}")
endforeach()

target_sources(Kiwa PRIVATE ${KIWA_ASSETS})

# ---- バンドルの設定 ----
set_target_properties(Kiwa PROPERTIES
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/Info.plist.in"
    MACOSX_BUNDLE_BUNDLE_NAME "Kiwa"
    MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
    MACOSX_BUNDLE_GUI_IDENTIFIER "com.example.kiwa"
    XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET "13.0"
    XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2"
    XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++20"
    XCODE_ATTRIBUTE_ENABLE_BITCODE "NO"
)
