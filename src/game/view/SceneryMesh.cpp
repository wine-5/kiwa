#include "game/view/SceneryMesh.h"
#include "game/view/MasuGeometry.h"
#include <algorithm>
#include <cmath>

namespace
{
	using core::utility::Color;
	using core::utility::mixed;
	using core::utility::toChannel;
	using core::utility::Vector3;
	using core::utility::Vertex3D;
	namespace masu = game::view::masu;

	/// @brief 光が来る向き（水面と同じものを使い、陰影の向きを揃える）
	constexpr Vector3 LIGHT_DIRECTION{ 0.36f, 0.88f, -0.31f };

	/// @brief 木目の貼り具合（1ワールド単位あたりの繰り返し数）
	constexpr float WOOD_UV_SCALE{ 0.55f };

	/// @brief 台の木目の貼り具合
	constexpr float TABLE_UV_SCALE{ 0.16f };

	/// @brief 台の半分の幅
	constexpr float TABLE_HALF{ 7.0f };

	/// @brief 台を何分割するか（明暗をなめらかに乗せるため）
	constexpr int TABLE_DIVISIONS{ 28 };

	/// @brief 光だまりの広さ
	constexpr float LIGHT_POOL_RADIUS{ 2.3f };

	/// @brief 光だまりの外の明るさ
	constexpr float LIGHT_POOL_FLOOR{ 0.12f };

	/// @brief 影が広がる半径
	constexpr float SHADOW_RADIUS{ 1.7f };

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

	/**
	 * @brief 面の向きから明るさだけを返す
	 *
	 * 色は木目のテクスチャが持っているので、頂点側は陰影に徹する。
	 * ここで色を掛けると二重になって濁る
	 * @param normal 面の向き
	 * @return 明るさを表す灰色
	 */
	Color shadeFace(const Vector3& normal) noexcept
	{
		// 真上を向いた面が一番明るく、陰の側は落ちる
		const float brightness{ 0.52f + 0.48f * std::max(0.0f, normal.dot(LIGHT_DIRECTION)) };
		const int level{ toChannel(255.0f * brightness) };
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

		for (const Face& face : faces)
		{
			const auto base{ static_cast<unsigned short>(vertices.size()) };
			const Color shaded{ shadeFace(face.normal) };

			const float uvU[4]{ 0.0f, face.uWidth * uvScale, face.uWidth * uvScale, 0.0f };
			const float uvV[4]{ face.vHeight * uvScale, face.vHeight * uvScale, 0.0f, 0.0f };

			for (int i{ 0 }; i < 4; ++i)
			{
				Vertex3D vertex{};
				vertex.position = face.corners[i];
				vertex.normal = face.normal;
				vertex.color = shaded;
				vertex.u = uvU[i];
				vertex.v = uvV[i];
				vertices.push_back(vertex);
			}

			indices.push_back(base);
			indices.push_back(static_cast<unsigned short>(base + 2));
			indices.push_back(static_cast<unsigned short>(base + 1));

			indices.push_back(base);
			indices.push_back(static_cast<unsigned short>(base + 3));
			indices.push_back(static_cast<unsigned short>(base + 2));
		}
	}

	void SceneryMesh::buildMasu(std::vector<core::utility::Vertex3D>& vertices,
	                            std::vector<unsigned short>& indices)
	{
		// 底板
		appendBox(vertices, indices, Vector3{ -masu::OUTER_HALF, 0.0f, -masu::OUTER_HALF },
		          Vector3{ masu::OUTER_HALF, masu::FLOOR_TOP, masu::OUTER_HALF }, WOOD_UV_SCALE);

		// 側板は4枚とも組む。手前を省くと断面図のようになって、器に見えなくなる
		appendBox(vertices, indices, Vector3{ -masu::OUTER_HALF, 0.0f, -masu::OUTER_HALF },
		          Vector3{ masu::OUTER_HALF, masu::HEIGHT, -masu::INNER_HALF }, WOOD_UV_SCALE);
		appendBox(vertices, indices, Vector3{ -masu::OUTER_HALF, 0.0f, masu::INNER_HALF },
		          Vector3{ masu::OUTER_HALF, masu::HEIGHT, masu::OUTER_HALF }, WOOD_UV_SCALE);
		appendBox(vertices, indices, Vector3{ masu::INNER_HALF, 0.0f, -masu::OUTER_HALF },
		          Vector3{ masu::OUTER_HALF, masu::HEIGHT, masu::OUTER_HALF }, WOOD_UV_SCALE);
		appendBox(vertices, indices, Vector3{ -masu::OUTER_HALF, 0.0f, -masu::OUTER_HALF },
		          Vector3{ -masu::INNER_HALF, masu::HEIGHT, masu::OUTER_HALF }, WOOD_UV_SCALE);
	}

	void SceneryMesh::buildTable(std::vector<core::utility::Vertex3D>& vertices,
	                             std::vector<unsigned short>& indices)
	{
		appendGrid(vertices, indices, TABLE_HALF, TABLE_DIVISIONS, 0.0f,
		           [](Vertex3D& vertex, float x, float z) {
			           // 枡の周りだけを明るくし、外へ向かって闇へ落とす
			           const float distance{ std::sqrt(x * x + z * z) / LIGHT_POOL_RADIUS };
			           const float pool{ std::exp(-distance * distance) };
			           const float brightness{ LIGHT_POOL_FLOOR + (1.0f - LIGHT_POOL_FLOOR) * pool };

			           const int level{ toChannel(255.0f * brightness) };
			           vertex.color = Color{ level, level, level };
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
