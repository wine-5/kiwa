#pragma once
#include <algorithm>
#include <cmath>

namespace game::view::cup
{
	/**
	 * @brief 湯呑の内側の寸法
	 *
	 * 原点は湯呑の底の中心。モデル（tools/make_models.py の make_yunomi）の
	 * 内側の形に合わせてある。液体を描くものがここを見る
	 */

	/// @brief 内側の底の高さ
	inline constexpr float FLOOR_TOP{ 0.075f };

	/// @brief 口の高さ
	inline constexpr float RIM_HEIGHT{ 0.60f };

	/// @brief 内側の底の半径
	inline constexpr float FLOOR_RADIUS{ 0.175f };

	/// @brief 口のところの内半径
	inline constexpr float RIM_RADIUS{ 0.408f };

	/// @brief 液体が入る高さ
	inline constexpr float INNER_HEIGHT{ RIM_HEIGHT - FLOOR_TOP };

	/**
	 * @brief その高さでの内側の半径を返す
	 *
	 * 湯呑は口へ向かって開いているが、広がり方は一定ではなく腰のあたりで急に開く。
	 * 直線で近似するとモデルの内壁との間に隙間ができ、器が空に見えてしまうので、
	 * モデル（make_yunomi の内側の断面）に合わせて平方根で近似する
	 * @param height 高さ
	 * @return 内側の半径
	 */
	inline float radiusAt(float height) noexcept
	{
		const float t{ std::clamp((height - FLOOR_TOP) / INNER_HEIGHT, 0.0f, 1.0f) };
		return FLOOR_RADIUS + (RIM_RADIUS - FLOOR_RADIUS) * std::sqrt(t);
	}

	/**
	 * @brief 嵩の割合から液面の高さを求める
	 * @param ratio 嵩（0.0〜1.0）
	 * @return 液面の高さ
	 */
	inline constexpr float surfaceHeight(float ratio) noexcept
	{
		return FLOOR_TOP + INNER_HEIGHT * ratio;
	}
} // namespace game::view::cup
