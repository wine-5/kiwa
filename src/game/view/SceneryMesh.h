#pragma once
#include "core/utility/Color.h"
#include "core/utility/Vector3.h"
#include "core/utility/Vertex3D.h"
#include <vector>

namespace game::view
{
	/**
	 * @brief 枡と台をテクスチャ付きのメッシュとして組む
	 *
	 * 直方体を描く関数では木目を貼れないため、面ごとに頂点を並べて自前で組む。
	 * 面の向きに応じた明るさもここで頂点色に焼き込む
	 */
	class SceneryMesh
	{
	  public:
		/**
		 * @brief 枡を組む（底板と側板3枚）
		 * @param vertices 頂点の出力先
		 * @param indices 三角形の出力先
		 */
		static void buildMasu(std::vector<core::utility::Vertex3D>& vertices,
		                      std::vector<unsigned short>& indices);

		/**
		 * @brief 枡を載せる台を組む
		 *
		 * 一枚の平らな面として組み、枡の周りだけが明るくなるよう頂点で明暗をつける。
		 * 写真でいう「光だまり」で、周りが落ちるほど被写体が浮かび上がる
		 * @param vertices 頂点の出力先
		 * @param indices 三角形の出力先
		 */
		static void buildTable(std::vector<core::utility::Vertex3D>& vertices,
		                       std::vector<unsigned short>& indices);

		/**
		 * @brief 枡が台に落とす影を組む
		 *
		 * 中心ほど濃く、外へ向かって消えていく楕円。透かして重ねて使う
		 * @param vertices 頂点の出力先
		 * @param indices 三角形の出力先
		 */
		static void buildShadow(std::vector<core::utility::Vertex3D>& vertices,
		                        std::vector<unsigned short>& indices);

	  private:
		/**
		 * @brief 軸に沿った直方体を面ごとに組む
		 * @param vertices 頂点の出力先
		 * @param indices 三角形の出力先
		 * @param minCorner 小さい側の角
		 * @param maxCorner 大きい側の角
		 * @param uvScale テクスチャの貼り具合（1ワールド単位あたりの繰り返し数）
		 */
		static void appendBox(std::vector<core::utility::Vertex3D>& vertices,
		                      std::vector<unsigned short>& indices,
		                      const core::utility::Vector3& minCorner,
		                      const core::utility::Vector3& maxCorner, float uvScale);
	};
} // namespace game::view
