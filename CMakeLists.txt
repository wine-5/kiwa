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

# ---- ソース ----
#
# Windows 専用の入口とフォント登録は外す。中身（core / game）は触らずそのまま使う
file(GLOB_RECURSE KIWA_SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.cpp")
list(FILTER KIWA_SOURCES EXCLUDE REGEX "/src/platform/windows/")

add_executable(Kiwa MACOSX_BUNDLE ${KIWA_SOURCES})

target_include_directories(Kiwa PRIVATE
    "${CMAKE_SOURCE_DIR}/src"
    "${DXLIB_IOS_DIR}"
)

# ---- DXライブラリと、それが要る枠組み ----
#
# ライブラリ名と枠組みの顔ぶれは DXライブラリ iOS 版の版によって変わる。
# 合わないときは公式の手引き（README_Xcode.md にリンクあり）に合わせて直すこと
target_link_directories(Kiwa PRIVATE "${DXLIB_IOS_DIR}")
target_link_libraries(Kiwa PRIVATE
    DxLib_iOS
    DxUseCLib_iOS
    jpeg_iOS
    png_iOS
    zlib_iOS
    tiff_iOS
    theora_iOS
    vorbis_iOS
    ogg_iOS
    bullet_iOS
    opusfile_iOS
    opus_iOS

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
