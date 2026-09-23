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
# iOS 版は「Debug-iphoneos」「Release-iphonesimulator」のように、
# 構成と実機/シミュレータの組み合わせごとにフォルダが分かれている。
# どれを使うかはビルドのたびに変わるので、Xcode に決めさせる（後述）。
# ここでは、その入れ物になっているフォルダと、リンクするライブラリの名前だけを拾う
file(GLOB_RECURSE DXLIB_HEADER_CANDIDATES
    "${DXLIB_IOS_DIR}/DxLib.h"
    "${DXLIB_IOS_DIR}/*/DxLib.h")

if(NOT DXLIB_HEADER_CANDIDATES)
    message(FATAL_ERROR
        "${DXLIB_IOS_DIR} の中に DxLib.h が見つかりません。
"
        "  展開先を間違えていないか確かめてください（find で探せます）：
"
        "    find ~/Downloads -maxdepth 6 -name DxLib.h")
endif()

# 実機向けのフォルダを目印にして、その親を入れ物とみなす
set(DXLIB_BASE_DIR "")

foreach(header ${DXLIB_HEADER_CANDIDATES})
    get_filename_component(header_dir "${header}" DIRECTORY)
    get_filename_component(header_dir_name "${header_dir}" NAME)

    if(header_dir_name MATCHES "-iphoneos$")
        get_filename_component(DXLIB_BASE_DIR "${header_dir}" DIRECTORY)
        set(DXLIB_SAMPLE_DIR "${header_dir}")
        break()
    endif()
endforeach()

if(DXLIB_BASE_DIR)
    # 構成ごとのフォルダを Xcode に選ばせる。この2つの記号はそのまま Xcode へ渡る
    set(DXLIB_SEARCH_PATH "${DXLIB_BASE_DIR}/$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)")
    message(STATUS "DXライブラリ: ${DXLIB_BASE_DIR} （構成ごとに切り替え）")
else()
    # 構成で分かれていない並びのときは、見つけたところをそのまま使う
    list(GET DXLIB_HEADER_CANDIDATES 0 DXLIB_HEADER)
    get_filename_component(DXLIB_SAMPLE_DIR "${DXLIB_HEADER}" DIRECTORY)
    set(DXLIB_SEARCH_PATH "${DXLIB_SAMPLE_DIR}")
    message(STATUS "DXライブラリ: ${DXLIB_SAMPLE_DIR}")
endif()

# リンクするライブラリの名前は、実物から拾う（版によって顔ぶれが変わるため）
file(GLOB DXLIB_LIBRARY_FILES "${DXLIB_SAMPLE_DIR}/*.a")

if(NOT DXLIB_LIBRARY_FILES)
    message(FATAL_ERROR
        "${DXLIB_SAMPLE_DIR} の中に .a（ライブラリ）が見つかりません。
"
        "  iOS 版ではなく Windows 版を展開していないか確かめてください。")
endif()

set(DXLIB_LINK_FLAGS "")

foreach(library ${DXLIB_LIBRARY_FILES})
    get_filename_component(library_name "${library}" NAME_WE)
    string(REGEX REPLACE "^lib" "" library_name "${library_name}")
    list(APPEND DXLIB_LINK_FLAGS "-l${library_name}")
endforeach()

list(LENGTH DXLIB_LINK_FLAGS DXLIB_LIBRARY_COUNT)
message(STATUS "DXライブラリのライブラリ: ${DXLIB_LIBRARY_COUNT} 個")

# ---- ソース ----
#
# Windows 専用の入口とフォント登録は外す。中身（core / game）は触らずそのまま使う
file(GLOB_RECURSE KIWA_SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.cpp")
list(FILTER KIWA_SOURCES EXCLUDE REGEX "/src/platform/windows/")

list(LENGTH KIWA_SOURCES KIWA_SOURCE_COUNT)

if(KIWA_SOURCE_COUNT EQUAL 0)
    message(FATAL_ERROR
        "${CMAKE_SOURCE_DIR}/src の中に .cpp が一つも見つかりません。
"
        "  展開したフォルダの中で ls を打ち、src がフォルダとして見えるか確かめてください。")
endif()

message(STATUS "ソース: ${KIWA_SOURCE_COUNT} 個")

add_executable(Kiwa MACOSX_BUNDLE ${KIWA_SOURCES})

target_include_directories(Kiwa PRIVATE "${CMAKE_SOURCE_DIR}/src")

# ---- DXライブラリと、それが要る枠組み ----
#
# ライブラリは上で見つけたものをそのまま渡す（名前を決め打ちにしない）。
# 枠組みの顔ぶれは版によって変わるので、足りなければここへ足す
target_link_libraries(Kiwa PRIVATE
    ${DXLIB_LINK_FLAGS}

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

    # DXライブラリの置き場所。構成（Debug/Release）と実機かシミュレータかで
    # フォルダが変わるので、Xcode の記号のまま渡して向こうに選ばせる
    XCODE_ATTRIBUTE_HEADER_SEARCH_PATHS "${DXLIB_SEARCH_PATH}"
    XCODE_ATTRIBUTE_LIBRARY_SEARCH_PATHS "${DXLIB_SEARCH_PATH}"
)
