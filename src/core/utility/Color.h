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

	/**
	 * @brief 0〜255 に収めて整数にする
	 * @param value 元の値
	 * @return 収めた値
	 */
	[[nodiscard]] constexpr int toChannel(float value) noexcept
	{
		return value < 0.0f ? 0 : (value > 255.0f ? 255 : static_cast<int>(value));
	}

	/**
	 * @brief 明るさを掛けた色を返す
	 * @param color 元の色
	 * @param scale 掛ける倍率
	 * @return 明るさを掛けた色
	 */
	[[nodiscard]] constexpr Color scaled(const Color& color, float scale) noexcept
	{
		return Color{ toChannel(color.r * scale), toChannel(color.g * scale),
			          toChannel(color.b * scale) };
	}

	/**
	 * @brief 二つの色を混ぜる
	 * @param from 混ぜる前の色
	 * @param to 混ぜ込む色
	 * @param t 混ぜる割合（0.0〜1.0）
	 * @return 混ぜた色
	 */
	[[nodiscard]] constexpr Color mixed(const Color& from, const Color& to, float t) noexcept
	{
		return Color{ toChannel(from.r + (to.r - from.r) * t), toChannel(from.g + (to.g - from.g) * t),
			          toChannel(from.b + (to.b - from.b) * t) };
	}

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
