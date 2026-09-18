#pragma once
#include "core/utility/Color.h"

namespace game::constant
{
	/**
	 * @brief 作品全体の配色
	 *
	 * 生成りと木の色を基調にした和風の配色をここへ集める。
	 * 複数の画面で使う色だけを置き、ひとつの View でしか使わない色はその View に置く
	 */
	namespace palette
	{
		// ========== 場 ==========

		/// @brief 画面の背景（ほぼ闇。写真のように被写体だけを浮かび上がらせる）
		inline constexpr core::utility::Color BACKGROUND{ 12, 11, 10 };


		// ========== 液体 ==========

		/// @brief 枡に注がれた液体
		inline constexpr core::utility::Color LIQUID{ 198, 138, 38 };

		/// @brief こぼれて台に広がったぶん（枡の中より沈んだ色）
		inline constexpr core::utility::Color LIQUID_SPILLED{ 150, 105, 52 };

		/// @brief 水面のひとかわ（本体より明るくして面を見せる）
		inline constexpr core::utility::Color LIQUID_SURFACE{ 226, 174, 74 };

		/// @brief 水面の照り（光を足すように重ねる）
		inline constexpr core::utility::Color LIQUID_SHINE{ 255, 246, 214 };

		/// @brief 広がる波紋
		inline constexpr core::utility::Color RIPPLE{ 250, 226, 165 };

		/// @brief 注がれている筋
		inline constexpr core::utility::Color STREAM{ 240, 221, 154 };

		/// @brief 跳ねたしずく
		inline constexpr core::utility::Color DROPLET{ 246, 214, 130 };

		// ========== 印 ==========

		/// @brief こぼれる際（きわ）を示す線（漆で引いた細い線のつもり）
		inline constexpr core::utility::Color LIMIT_LINE{ 112, 30, 26 };

		// ========== 文字 ==========

		/// @brief 主となる文言
		inline constexpr core::utility::Color TEXT_PRIMARY{ 244, 236, 216 };

		/// @brief 添えの文言（操作の案内など）
		inline constexpr core::utility::Color TEXT_SUB{ 152, 142, 124 };
	} // namespace palette
} // namespace game::constant
