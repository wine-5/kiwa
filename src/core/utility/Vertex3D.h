#pragma once
#include "core/utility/Color.h"
#include "core/utility/Vector3.h"

namespace core::utility
{
	/**
	 * @brief 自前で組み立てるメッシュの頂点
	 *
	 * 箱や球のような出来合いの形では足りないもの（波打つ水面、流れる筋）を
	 * 三角形の集まりとして描くために使う。色は頂点ごとに持てるので、
	 * 陰影や照りをこちら側で計算して流し込める
	 */
	struct Vertex3D
	{
		/// @brief 座標
		Vector3 position{};

		/// @brief 法線（面の向き）
		Vector3 normal{ 0.0f, 1.0f, 0.0f };

		/// @brief 頂点の色
		Color color{};

		/// @brief 透明度（0.0で透明、1.0で不透明）
		/// @details 透かして重ねる設定のときだけ効く
		float alpha{ 1.0f };

		/// @brief テクスチャ座標
		float u{ 0.0f };
		float v{ 0.0f };
	};
} // namespace core::utility
