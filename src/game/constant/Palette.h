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

		/// @brief 器に注がれた液体（緑茶）
		inline constexpr core::utility::Color LIQUID{ 150, 172, 62 };

		/// @brief こぼれて台に広がったぶん（器の中より沈んだ色）
		inline constexpr core::utility::Color LIQUID_SPILLED{ 112, 126, 58 };

		/// @brief 畳に吸われた茶（染みの外側。ほとんど畳の色に近づく）
		inline constexpr core::utility::Color LIQUID_SOAKED{ 74, 74, 44 };

		/// @brief 水面のひとかわ（本体より明るくして面を見せる）
		inline constexpr core::utility::Color LIQUID_SURFACE{ 186, 206, 96 };

		/// @brief 水面の照り（光を足すように重ねる）
		inline constexpr core::utility::Color LIQUID_SHINE{ 250, 252, 228 };

		/// @brief 広がる波紋
		inline constexpr core::utility::Color RIPPLE{ 226, 238, 176 };

		/// @brief 注がれている筋
		inline constexpr core::utility::Color STREAM{ 214, 228, 150 };

		/// @brief 跳ねたしずく
		inline constexpr core::utility::Color DROPLET{ 220, 232, 156 };

		// ========== 印 ==========

		/// @brief こぼれる際（きわ）を示す線（漆で引いた細い線のつもり）
		inline constexpr core::utility::Color LIMIT_LINE{ 112, 30, 26 };

		// ========== 文字 ==========

		/// @brief 主となる文言
		inline constexpr core::utility::Color TEXT_PRIMARY{ 244, 236, 216 };

		/// @brief 墨（明るい紙の上に書く字。白い字では読めないため）
		inline constexpr core::utility::Color INK{ 28, 26, 24 };

		/// @brief 添えの文言（操作の案内など）
		inline constexpr core::utility::Color TEXT_SUB{ 152, 142, 124 };
	} // namespace palette
} // namespace game::constant
