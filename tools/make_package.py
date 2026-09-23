"""配布用のフォルダを Zip に固める。

Windows 標準の Compress-Archive は、Zip の中のパスを `\\` で書いてしまう。
Windows では展開できるが、**Mac や Linux では階層として扱われず**、
`src\\Application.cpp` という名前のファイルが一つできるだけになる。
（Xcode 側で「ソースが無い」と言われる原因になった）

ここでは区切りを `/` に直して書き込み、書いたあとに確かめる。

    python tools/make_package.py <固めたいフォルダ> <出力する.zip>

例
    python tools/make_package.py C:/Users/me/Desktop/Kiwa_IOS C:/Users/me/Desktop/Kiwa_IOS.zip
"""

import os
import sys
import zipfile


def zip_folder(folder, zip_path):
    """フォルダの中身を Zip に固める（中の区切りは / にする）。"""
    root = os.path.abspath(folder)

    with zipfile.ZipFile(zip_path, "w", zipfile.ZIP_DEFLATED) as archive:
        for base, directories, files in os.walk(root):
            # 並びを決めておくと、作り直したときの差分が読みやすい
            directories.sort()
            files.sort()

            for name in files:
                full = os.path.join(base, name)
                relative = os.path.relpath(full, root).replace("\\", "/")
                archive.write(full, relative)

    verify(zip_path)


def verify(zip_path):
    """区切りが正しく書けているかを確かめる。"""
    with zipfile.ZipFile(zip_path) as archive:
        names = archive.namelist()

    wrong = [name for name in names if "\\" in name]

    print("固めました:", zip_path)
    print("  中のファイル:", len(names), "件")

    if wrong:
        print("  区切りが誤っているもの:", len(wrong), "件 ← Mac で展開できません")
        sys.exit(1)

    print("  区切り: すべて / で書けています")


def main():
    if len(sys.argv) != 3:
        print(__doc__)
        sys.exit(1)

    zip_folder(sys.argv[1], sys.argv[2])


main()
