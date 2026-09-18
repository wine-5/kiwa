"""assets/model/ の 3D モデルを生成する。

DxLib が読める MQO 形式（テキスト）で書き出す。回転体・掃引・トーラスを
組み合わせて、急須・湯呑・茶托を作る。

    python tools/make_models.py

生成した .mqo は Metasequoia でも Blender（MQO 読み込みアドオン）でも開けるので、
手で直したい場合はそちらで編集してもよい。
単位はゲーム内のワールド単位に合わせてある（枡の外寸の半分が 1.0）。
"""

import math
import os

OUTPUT_DIR = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "assets", "model")


class Mesh:
    """頂点と面の入れ物。面は材質番号を持つ。"""

    def __init__(self):
        self.vertices = []
        self.faces = []  # (indices, material)

    def add_vertex(self, x, y, z):
        self.vertices.append((x, y, z))
        return len(self.vertices) - 1

    def add_face(self, indices, material):
        self.faces.append((list(indices), material))

    def extend(self, other):
        offset = len(self.vertices)
        self.vertices.extend(other.vertices)
        for indices, material in other.faces:
            self.faces.append([index + offset for index in indices])
            self.faces[-1] = ([index + offset for index in indices], material)


def revolve(profile, segments, material, closed_bottom=False, closed_top=False):
    """断面（半径・高さの並び）を Y 軸まわりに回して回転体を作る。"""
    mesh = Mesh()
    rings = []

    for radius, height in profile:
        ring = []
        for i in range(segments):
            angle = 2.0 * math.pi * i / segments
            ring.append(mesh.add_vertex(math.cos(angle) * radius, height, math.sin(angle) * radius))
        rings.append(ring)

    for r in range(len(rings) - 1):
        lower = rings[r]
        upper = rings[r + 1]
        for i in range(segments):
            j = (i + 1) % segments
            # MQO は面の並び順で表裏が決まる。外を向くようにこの順で並べる
            mesh.add_face([upper[i], upper[j], lower[j], lower[i]], material)

    if closed_bottom:
        center = mesh.add_vertex(0.0, profile[0][1], 0.0)
        for i in range(segments):
            j = (i + 1) % segments
            mesh.add_face([center, rings[0][i], rings[0][j]], material)

    if closed_top:
        center = mesh.add_vertex(0.0, profile[-1][1], 0.0)
        for i in range(segments):
            j = (i + 1) % segments
            mesh.add_face([center, rings[-1][j], rings[-1][i]], material)

    return mesh


def sweep(path, radii, segments, material, cap_start=False, cap_end=False):
    """経路に沿って筒を作る（注ぎ口や取っ手に使う）。"""
    mesh = Mesh()
    rings = []

    for index, (point, radius) in enumerate(zip(path, radii)):
        # 進む向きを求め、それに直交する二本の軸で円を描く
        if index == 0:
            direction = _sub(path[1], path[0])
        elif index == len(path) - 1:
            direction = _sub(path[-1], path[-2])
        else:
            direction = _sub(path[index + 1], path[index - 1])

        forward = _normalize(direction)
        helper = (0.0, 0.0, 1.0) if abs(forward[1]) > 0.9 else (0.0, 1.0, 0.0)
        side = _normalize(_cross(helper, forward))
        up = _cross(forward, side)

        ring = []
        for i in range(segments):
            angle = 2.0 * math.pi * i / segments
            offset = _add(_scale(side, math.cos(angle) * radius), _scale(up, math.sin(angle) * radius))
            position = _add(point, offset)
            ring.append(mesh.add_vertex(*position))
        rings.append(ring)

    for r in range(len(rings) - 1):
        lower = rings[r]
        upper = rings[r + 1]
        for i in range(segments):
            j = (i + 1) % segments
            mesh.add_face([upper[i], upper[j], lower[j], lower[i]], material)

    if cap_start:
        center = mesh.add_vertex(*path[0])
        for i in range(segments):
            j = (i + 1) % segments
            mesh.add_face([center, rings[0][i], rings[0][j]], material)

    if cap_end:
        center = mesh.add_vertex(*path[-1])
        for i in range(segments):
            j = (i + 1) % segments
            mesh.add_face([center, rings[-1][j], rings[-1][i]], material)

    return mesh


def _add(a, b):
    return (a[0] + b[0], a[1] + b[1], a[2] + b[2])


def _sub(a, b):
    return (a[0] - b[0], a[1] - b[1], a[2] - b[2])


def _scale(a, s):
    return (a[0] * s, a[1] * s, a[2] * s)


def _cross(a, b):
    return (a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0])


def _normalize(a):
    length = math.sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2])
    if length <= 0.0:
        return (0.0, 1.0, 0.0)
    return (a[0] / length, a[1] / length, a[2] / length)


def _bezier(p0, p1, p2, t):
    """2次ベジエ曲線上の点を返す。"""
    inverse = 1.0 - t
    return (
        inverse * inverse * p0[0] + 2.0 * inverse * t * p1[0] + t * t * p2[0],
        inverse * inverse * p0[1] + 2.0 * inverse * t * p1[1] + t * t * p2[1],
        inverse * inverse * p0[2] + 2.0 * inverse * t * p1[2] + t * t * p2[2],
    )


def smooth_profile(points, subdivisions):
    """断面の角を丸める（何度か中点を取って均す）。"""
    result = list(points)
    for _ in range(subdivisions):
        refined = [result[0]]
        for i in range(len(result) - 1):
            a = result[i]
            b = result[i + 1]
            refined.append((a[0] * 0.75 + b[0] * 0.25, a[1] * 0.75 + b[1] * 0.25))
            refined.append((a[0] * 0.25 + b[0] * 0.75, a[1] * 0.25 + b[1] * 0.75))
        refined.append(result[-1])
        result = refined
    return result


def make_dobin():
    """土瓶。丸い胴・短い注ぎ口・蓋・上に渡した弦（つる）。

    弦は籐を巻いた意匠にするため、胴とは別の材質にする。
    """
    mesh = Mesh()
    segments = 56

    # 胴。腰が張って上でつぼまる、平たい丸
    body = smooth_profile([
        (0.00, 0.00), (0.20, 0.00), (0.32, 0.02), (0.46, 0.09),
        (0.57, 0.20), (0.62, 0.32), (0.61, 0.43), (0.54, 0.53),
        (0.44, 0.60), (0.37, 0.635), (0.355, 0.645),
    ], 2)
    mesh.extend(revolve(body, segments, 0, closed_bottom=True))

    # 口の縁（内へ折り返して蓋を受ける）
    rim = [(0.355, 0.645), (0.335, 0.655), (0.325, 0.62)]
    mesh.extend(revolve(rim, segments, 0))

    # 蓋。浅い皿につまみを載せる
    lid = smooth_profile([
        (0.00, 0.735), (0.12, 0.730), (0.24, 0.712), (0.32, 0.678), (0.355, 0.652), (0.355, 0.640),
    ], 2)
    mesh.extend(revolve(lid, segments, 0))

    knob = smooth_profile([
        (0.00, 0.735), (0.035, 0.740), (0.052, 0.772), (0.045, 0.806), (0.026, 0.822), (0.00, 0.826),
    ], 2)
    mesh.extend(revolve(knob, segments, 0))

    # 注ぎ口。短く、先が斜めに切れている
    spout_path = [
        (0.50, 0.30, 0.0), (0.62, 0.36, 0.0), (0.74, 0.43, 0.0),
        (0.84, 0.50, 0.0), (0.90, 0.545, 0.0),
    ]
    spout_radii = [0.125, 0.105, 0.085, 0.068, 0.058]
    mesh.extend(sweep(spout_path, spout_radii, 24, 0))

    # 弦を受ける耳。肩の左右に立てる
    for side in (-1.0, 1.0):
        lug_path = [
            (side * 0.46, 0.50, 0.0), (side * 0.50, 0.58, 0.0), (side * 0.50, 0.66, 0.0),
        ]
        mesh.extend(sweep(lug_path, [0.055, 0.045, 0.038], 12, 0, cap_start=True, cap_end=True))

    # 弦。耳から耳へ、蓋の上を跨いで弧を描く
    handle_path = []
    handle_radii = []
    for i in range(26):
        t = i / 25.0
        angle = math.pi * t
        handle_path.append((-math.cos(angle) * 0.50, 0.64 + math.sin(angle) * 0.52, 0.0))
        handle_radii.append(0.040)
    mesh.extend(sweep(handle_path, handle_radii, 14, 1, cap_start=True, cap_end=True))

    return mesh


def make_yunomi():
    """湯呑。腰から口へ向かって開く、取っ手のない碗。"""
    mesh = Mesh()
    segments = 56

    outer = smooth_profile([
        (0.00, 0.00), (0.17, 0.00), (0.21, 0.015), (0.25, 0.07),
        (0.31, 0.20), (0.37, 0.36), (0.41, 0.50), (0.425, 0.575), (0.428, 0.60),
    ], 2)
    mesh.extend(revolve(outer, segments, 0, closed_bottom=True))

    # 口縁は色を変える（写真の湯呑は縁だけ土の色が出ている）
    lip = [(0.428, 0.60), (0.420, 0.607), (0.408, 0.603)]
    mesh.extend(revolve(lip, segments, 2))

    # 内側。底は厚く取る
    inner = smooth_profile([
        (0.408, 0.603), (0.395, 0.52), (0.35, 0.36), (0.285, 0.18),
        (0.22, 0.085), (0.17, 0.075), (0.00, 0.07),
    ], 2)
    mesh.extend(revolve(inner, segments, 0))

    return mesh


def make_saucer():
    """茶托。縁が立ち上がった浅い皿。"""
    mesh = Mesh()
    segments = 48

    profile = smooth_profile([
        (0.00, 0.00), (0.30, 0.00), (0.40, 0.015), (0.50, 0.05),
        (0.55, 0.09), (0.56, 0.11), (0.545, 0.105), (0.50, 0.075),
        (0.40, 0.045), (0.30, 0.035), (0.00, 0.032),
    ], 2)
    mesh.extend(revolve(profile, segments, 0, closed_bottom=True))

    return mesh


MATERIAL_TEMPLATES = {
    # 青磁のような、やや緑がかった灰色の釉薬。つやを強めに出す
    "celadon": '"celadon" shader(3) col(0.470 0.530 0.500 1.000) dif(0.820) amb(0.420) emi(0.000) spc(0.900) power(42.00)',
    # 弦に巻いた籐（濃い茶）
    "cord": '"cord" shader(3) col(0.280 0.170 0.150 1.000) dif(0.780) amb(0.380) emi(0.000) spc(0.350) power(16.00)',
    # 口縁に出る土の色
    "clay": '"clay" shader(3) col(0.620 0.570 0.470 1.000) dif(0.820) amb(0.420) emi(0.000) spc(0.300) power(14.00)',
    "lacquer": '"lacquer" shader(3) col(0.260 0.140 0.100 1.000) dif(0.820) amb(0.400) emi(0.000) spc(0.600) power(34.00)',
}


def write_mqo(path, name, mesh, materials, facet=60.0):
    """MQO 形式で書き出す。"""
    lines = []
    lines.append("Metasequoia Document")
    lines.append("Format Text Ver 1.0")
    lines.append("")
    lines.append("Scene {")
    lines.append("\tpos 0.0000 0.0000 1500.0000")
    lines.append("\tlookat 0.0000 0.0000 0.0000")
    lines.append("\thead -0.5000")
    lines.append("\tpich 0.5000")
    lines.append("\tortho 0")
    lines.append("\tzoom2 5.0000")
    lines.append("\tamb 0.250 0.250 0.250")
    lines.append("}")
    lines.append("Material %d {" % len(materials))
    for material in materials:
        lines.append("\t" + MATERIAL_TEMPLATES[material])
    lines.append("}")

    lines.append('Object "%s" {' % name)
    lines.append("\tdepth 0")
    lines.append("\tfolding 0")
    lines.append("\tscale 1.000000 1.000000 1.000000")
    lines.append("\trotation 0.000000 0.000000 0.000000")
    lines.append("\ttranslation 0.000000 0.000000 0.000000")
    lines.append("\tvisible 15")
    lines.append("\tlocking 0")
    lines.append("\tshading 1")
    lines.append("\tfacet %.1f" % facet)
    lines.append("\tcolor 0.898 0.498 0.698")
    lines.append("\tcolor_type 0")

    lines.append("\tvertex %d {" % len(mesh.vertices))
    for x, y, z in mesh.vertices:
        lines.append("\t\t%.5f %.5f %.5f" % (x, y, z))
    lines.append("\t}")

    lines.append("\tface %d {" % len(mesh.faces))
    for indices, material in mesh.faces:
        listed = " ".join(str(index) for index in indices)
        lines.append("\t\t%d V(%s) M(%d)" % (len(indices), listed, material))
    lines.append("\t}")
    lines.append("}")
    lines.append("Eof")

    os.makedirs(OUTPUT_DIR, exist_ok=True)
    full = os.path.join(OUTPUT_DIR, path)
    with open(full, "w", encoding="utf-8", newline="\r\n") as handle:
        handle.write("\n".join(lines) + "\n")
    print("書き出し: %s  (頂点 %d / 面 %d)" % (full, len(mesh.vertices), len(mesh.faces)))


def main():
    write_mqo("dobin.mqo", "dobin", make_dobin(), ["celadon", "cord"])
    write_mqo("yunomi.mqo", "yunomi", make_yunomi(), ["celadon", "cord", "clay"])
    write_mqo("saucer.mqo", "saucer", make_saucer(), ["lacquer"])


main()
