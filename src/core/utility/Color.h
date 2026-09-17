#pragma once

namespace core::utility
{
	/**
	 * @brief RGB の色
	 *
	 * DxLib の GetColor が返す int に Game 層が触れないようにするための型。
	 * 実際の変換は Infrastructure 層（Renderer）が行う
	 */
	struct Color
	{
		int r{ 255 };
		int g{ 255 };
		int b{ 255 };
	};

	namespace colors
	{
		inline constexpr Color WHITE{ 255, 255, 255 };
		inline constexpr Color BLACK{ 0, 0, 0 };
		inline constexpr Color RED{ 255, 64, 64 };
		inline constexpr Color GREEN{ 64, 255, 128 };
		inline constexpr Color BLUE{ 64, 160, 255 };
		inline constexpr Color GRAY{ 128, 128, 128 };
	} // namespace colors
} // namespace core::utility
