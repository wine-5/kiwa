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

		/// @brief 画面の背景（暗く落として枡と液体を浮かび上がらせる）
		inline constexpr core::utility::Color BACKGROUND{ 26, 28, 34 };

		/// @brief 枡が置かれている台
		inline constexpr core::utility::Color TABLE{ 96, 84, 70 };

		// ========== 枡 ==========

		/// @brief 枡の奥の板（一番明るい面）
		inline constexpr core::utility::Color MASU_BACK{ 176, 131, 87 };

		/// @brief 枡の左の板
		inline constexpr core::utility::Color MASU_LEFT{ 150, 111, 73 };

		/// @brief 枡の右の板（陰になる側）
		inline constexpr core::utility::Color MASU_RIGHT{ 122, 90, 59 };

		/// @brief 枡の底板
		inline constexpr core::utility::Color MASU_FLOOR{ 138, 102, 67 };

		// ========== 液体 ==========

		/// @brief 枡に注がれた液体
		inline constexpr core::utility::Color LIQUID{ 217, 164, 65 };

		/// @brief こぼれて台に広がったぶん（枡の中より沈んだ色）
		inline constexpr core::utility::Color LIQUID_SPILLED{ 150, 105, 52 };

		/// @brief 水面のひとかわ（本体より明るくして面を見せる）
		inline constexpr core::utility::Color LIQUID_SURFACE{ 236, 190, 96 };

		/// @brief 水面の照り（光を足すように重ねる）
		inline constexpr core::utility::Color LIQUID_SHINE{ 255, 246, 214 };

		/// @brief 広がる波紋
		inline constexpr core::utility::Color RIPPLE{ 250, 226, 165 };

		/// @brief 注がれている筋
		inline constexpr core::utility::Color STREAM{ 240, 221, 154 };

		/// @brief 跳ねたしずく
		inline constexpr core::utility::Color DROPLET{ 246, 214, 130 };

		// ========== 印 ==========

		/// @brief こぼれる際（きわ）を示す線
		inline constexpr core::utility::Color LIMIT_LINE{ 156, 43, 43 };

		// ========== 文字 ==========

		/// @brief 主となる文言
		inline constexpr core::utility::Color TEXT_PRIMARY{ 244, 236, 216 };

		/// @brief 添えの文言（操作の案内など）
		inline constexpr core::utility::Color TEXT_SUB{ 152, 142, 124 };
	} // namespace palette
} // namespace game::constant
