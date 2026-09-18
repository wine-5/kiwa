#pragma once
#include <numbers>

namespace core::utility::math
{
	/**
	 * @brief 数学で使う定数
	 *
	 * 円周率は標準の <numbers> が持っているので、自分で桁を書き写さずそれを包む。
	 * 書き写すと桁を間違えても気付けないため
	 */

	/// @brief 円周率
	inline constexpr float PI{ std::numbers::pi_v<float> };

	/// @brief 円周（一周のラジアン）
	inline constexpr float TWO_PI{ PI * 2.0f };

	/// @brief 直角（ラジアン）
	inline constexpr float HALF_PI{ PI * 0.5f };

	/**
	 * @brief 度をラジアンに直す
	 * @param degree 角度（度）
	 * @return 角度（ラジアン）
	 */
	inline constexpr float toRadian(float degree) noexcept
	{
		return degree * (PI / 180.0f);
	}

	/**
	 * @brief ラジアンを度に直す
	 * @param radian 角度（ラジアン）
	 * @return 角度（度）
	 */
	inline constexpr float toDegree(float radian) noexcept
	{
		return radian * (180.0f / PI);
	}
} // namespace core::utility::math
