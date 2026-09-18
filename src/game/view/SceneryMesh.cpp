#include "game/view/SceneryMesh.h"
#include <algorithm>
#include <cmath>

namespace
{
	using core::utility::Color;
	using core::utility::mixed;
	using core::utility::toChannel;
	using core::utility::Vector3;
	using core::utility::Vertex3D;

	/// @brief 台の木目の貼り具合
	constexpr float TABLE_UV_SCALE{ 0.32f };

	/// @brief 台の半分の幅
	constexpr float TABLE_HALF{ 7.0f };

	/// @brief 台を何分割するか（明暗をなめらかに乗せるため）
	constexpr int TABLE_DIVISIONS{ 28 };

	/// @brief 光だまりの広さ
	constexpr float LIGHT_POOL_RADIUS{ 1.15f };

	/// @brief 光だまりの外の明るさ
	constexpr float LIGHT_POOL_FLOOR{ 0.12f };

	/// @brief 影が広がる半径
	constexpr float SHADOW_RADIUS{ 0.8f };

	/// @brief 影の濃さ
	constexpr float SHADOW_STRENGTH{ 0.78f };

	/// @brief 影を何分割するか
	constexpr int SHADOW_DIVISIONS{ 24 };

	/**
	 * @brief 格子状の面を組む
	 * @param vertices 頂点の出力先
	 * @param indices 三角形の出力先
	 * @param half 面の半分の幅
	 * @param divisions 分割数
	 * @param y 高さ
	 * @param shade 位置から頂点を作る処理
	 */
	template <typename Shade>
	void appendGrid(std::vector<Vertex3D>& vertices, std::vector<unsigned short>& indices, float half,
	                int divisions, float y, Shade shade)
	{
		const int points{ divisions + 1 };
		const auto base{ static_cast<unsigned short>(vertices.size()) };
		const float span{ half * 2.0f };

		for (int iz{ 0 }; iz < points; ++iz)
		{
			for (int ix{ 0 }; ix < points; ++ix)
			{
				const float u{ static_cast<float>(ix) / divisions };
				const float v{ static_cast<float>(iz) / divisions };
				const float x{ -half + span * u };
				const float z{ -half + span * v };

				Vertex3D vertex{};
				vertex.position = Vector3{ x, y, z };
				vertex.normal = Vector3{ 0.0f, 1.0f, 0.0f };
				shade(vertex, x, z);
				vertices.push_back(vertex);
			}
		}

		for (int iz{ 0 }; iz < divisions; ++iz)
		{
			for (int ix{ 0 }; ix < divisions; ++ix)
			{
				const auto topLeft{ static_cast<unsigned short>(base + iz * points + ix) };
				const auto topRight{ static_cast<unsigned short>(topLeft + 1) };
				const auto bottomLeft{ static_cast<unsigned short>(topLeft + points) };
				const auto bottomRight{ static_cast<unsigned short>(bottomLeft + 1) };

				indices.push_back(topLeft);
				indices.push_back(bottomLeft);
				indices.push_back(topRight);

				indices.push_back(topRight);
				indices.push_back(bottomLeft);
				indices.push_back(bottomRight);
			}
		}
	}

	/// @brief 明かりの位置（行灯のつもり。面の中でも距離で明るさが変わる）
	constexpr Vector3 LAMP_POSITION{ 0.95f, 1.75f, -0.85f };

	/// @brief 明かりが届く距離のめやす
	constexpr float LAMP_RANGE{ 1.6f };

	/// @brief 明かりが当たらないところの明るさ
	constexpr float AMBIENT{ 0.26f };

	/// @brief 台や周りから回り込む光の強さ（上を向いた面ほど受ける）
	constexpr float BOUNCE{ 0.16f };

	/// @brief 手前から当てる弱い補助光の向き（写真のレフ板にあたるもの）
	constexpr Vector3 FILL_DIRECTION{ 0.15f, 0.55f, -0.82f };

	/// @brief 補助光の強さ
	constexpr float FILL{ 0.22f };

	/**
	 * @brief その点の明るさを返す
	 *
	 * 色は木目のテクスチャが持っているので、頂点側は陰影に徹する。
	 * 面ごとに一色だと板が平らな紙に見えるため、明かりからの距離でも変える
	 * @param position 座標
	 * @param normal 面の向き
	 * @return 明るさを表す灰色
	 */
	Color shadePoint(const Vector3& position, const Vector3& normal) noexcept
	{
		const Vector3 toLamp{ LAMP_POSITION - position };
		const float distance{ toLamp.length() };
		const Vector3 direction{ toLamp.normalized() };

		// 遠いほど暗い（距離の二乗で落ちる）
		const float falloff{ 1.0f / (1.0f + (distance / LAMP_RANGE) * (distance / LAMP_RANGE)) };
		const float diffuse{ std::max(0.0f, normal.dot(direction)) * falloff * 1.85f };

		// 光は一度きりでは終わらない。周りから回り込むぶんを足して陰を沈ませすぎない
		const float bounce{ BOUNCE * (0.5f + 0.5f * normal.y) };

		// 手前からの弱い補助光。これが無いと陰の側が潰れて形が読めなくなる
		const float fill{ FILL * std::max(0.0f, normal.dot(FILL_DIRECTION)) };

		const int level{ toChannel(255.0f * (AMBIENT + diffuse + bounce + fill)) };
		return Color{ level, level, level };
	}
} // namespace

namespace game::view
{
	void SceneryMesh::appendBox(std::vector<core::utility::Vertex3D>& vertices,
	                            std::vector<unsigned short>& indices,
	                            const core::utility::Vector3& minCorner,
	                            const core::utility::Vector3& maxCorner, float uvScale)
	{
		/**
		 * @brief 直方体の1面ぶん
		 */
		struct Face
		{
			Vector3 normal;
			Vector3 corners[4]; // 左下・右下・右上・左上の順
			float uWidth;       // テクスチャを貼る向きの長さ
			float vHeight;
		};

		const Vector3& a{ minCorner };
		const Vector3& b{ maxCorner };
		const float sizeX{ b.x - a.x };
		const float sizeY{ b.y - a.y };
		const float sizeZ{ b.z - a.z };

		const Face faces[]{
			// 手前（-Z）
			{ Vector3{ 0.0f, 0.0f, -1.0f },
			  { Vector3{ a.x, a.y, a.z }, Vector3{ b.x, a.y, a.z }, Vector3{ b.x, b.y, a.z },
			    Vector3{ a.x, b.y, a.z } },
			  sizeX, sizeY },
			// 奥（+Z）
			{ Vector3{ 0.0f, 0.0f, 1.0f },
			  { Vector3{ b.x, a.y, b.z }, Vector3{ a.x, a.y, b.z }, Vector3{ a.x, b.y, b.z },
			    Vector3{ b.x, b.y, b.z } },
			  sizeX, sizeY },
			// 左（-X）
			{ Vector3{ -1.0f, 0.0f, 0.0f },
			  { Vector3{ a.x, a.y, b.z }, Vector3{ a.x, a.y, a.z }, Vector3{ a.x, b.y, a.z },
			    Vector3{ a.x, b.y, b.z } },
			  sizeZ, sizeY },
			// 右（+X）
			{ Vector3{ 1.0f, 0.0f, 0.0f },
			  { Vector3{ b.x, a.y, a.z }, Vector3{ b.x, a.y, b.z }, Vector3{ b.x, b.y, b.z },
			    Vector3{ b.x, b.y, a.z } },
			  sizeZ, sizeY },
			// 上（+Y）
			{ Vector3{ 0.0f, 1.0f, 0.0f },
			  { Vector3{ a.x, b.y, a.z }, Vector3{ b.x, b.y, a.z }, Vector3{ b.x, b.y, b.z },
			    Vector3{ a.x, b.y, b.z } },
			  sizeX, sizeZ },
			// 下（-Y）
			{ Vector3{ 0.0f, -1.0f, 0.0f },
			  { Vector3{ a.x, a.y, b.z }, Vector3{ b.x, a.y, b.z }, Vector3{ b.x, a.y, a.z },
			    Vector3{ a.x, a.y, a.z } },
			  sizeX, sizeZ },
		};

		// 面は細かく割る。1枚の三角形では明かりの減衰が乗らず、板が平らな紙に見える
		constexpr int FACE_DIVISIONS{ 6 };
		constexpr int FACE_POINTS{ FACE_DIVISIONS + 1 };

		for (const Face& face : faces)
		{
			const auto base{ static_cast<unsigned short>(vertices.size()) };

			for (int iv{ 0 }; iv < FACE_POINTS; ++iv)
			{
				for (int iu{ 0 }; iu < FACE_POINTS; ++iu)
				{
					const float u{ static_cast<float>(iu) / FACE_DIVISIONS };
					const float v{ static_cast<float>(iv) / FACE_DIVISIONS };

					// 四隅から双一次で内側の点を作る
					const Vector3 bottom{ face.corners[0] + (face.corners[1] - face.corners[0]) * u };
					const Vector3 top{ face.corners[3] + (face.corners[2] - face.corners[3]) * u };
					const Vector3 position{ bottom + (top - bottom) * v };

					Vertex3D vertex{};
					vertex.position = position;
					vertex.normal = face.normal;
					vertex.color = shadePoint(position, face.normal);
					vertex.u = u * face.uWidth * uvScale;
					vertex.v = (1.0f - v) * face.vHeight * uvScale;
					vertices.push_back(vertex);
				}
			}

			for (int iv{ 0 }; iv < FACE_DIVISIONS; ++iv)
			{
				for (int iu{ 0 }; iu < FACE_DIVISIONS; ++iu)
				{
					const auto corner{ static_cast<unsigned short>(base + iv * FACE_POINTS + iu) };
					const auto right{ static_cast<unsigned short>(corner + 1) };
					const auto above{ static_cast<unsigned short>(corner + FACE_POINTS) };
					const auto diagonal{ static_cast<unsigned short>(above + 1) };

					indices.push_back(corner);
					indices.push_back(diagonal);
					indices.push_back(right);

					indices.push_back(corner);
					indices.push_back(above);
					indices.push_back(diagonal);
				}
			}
		}
	}

	void SceneryMesh::buildTable(std::vector<core::utility::Vertex3D>& vertices,
	                             std::vector<unsigned short>& indices)
	{
		appendGrid(vertices, indices, TABLE_HALF, TABLE_DIVISIONS, 0.0f,
		           [](Vertex3D& vertex, float x, float z) {
			           // 明かりからの距離で落とし、遠くは闇へ沈める
			           const Color lit{ shadePoint(vertex.position, vertex.normal) };
			           const float distance{ std::sqrt(x * x + z * z) / LIGHT_POOL_RADIUS };
			           const float pool{ LIGHT_POOL_FLOOR +
				                         (1.0f - LIGHT_POOL_FLOOR) * std::exp(-distance * distance) };

			           vertex.color = core::utility::scaled(lit, pool);
			           vertex.u = x * TABLE_UV_SCALE;
			           vertex.v = z * TABLE_UV_SCALE;
		           });
	}

	void SceneryMesh::buildShadow(std::vector<core::utility::Vertex3D>& vertices,
	                              std::vector<unsigned short>& indices)
	{
		appendGrid(vertices, indices, SHADOW_RADIUS, SHADOW_DIVISIONS, 0.004f,
		           [](Vertex3D& vertex, float x, float z) {
			           // 真下が濃く、外へ向かってぼやけて消える
			           const float distance{ std::sqrt(x * x + z * z) / SHADOW_RADIUS };
			           const float density{ std::exp(-distance * distance * 3.2f) };

			           vertex.color = Color{ 0, 0, 0 };
			           vertex.alpha = SHADOW_STRENGTH * density;
		           });
	}
} // namespace game::view
