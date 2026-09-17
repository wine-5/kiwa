#pragma once

namespace game::view::masu
{
	/**
	 * @brief 枡の寸法
	 *
	 * 原点は枡の底の中心。枡を描くものと、その中の液体を描くものの両方が使うため、
	 * どちらか一方に置かずここへ出している
	 */

	/// @brief 枡の外側の半分の幅
	inline constexpr float OUTER_HALF{ 1.0f };

	/// @brief 板の厚み
	inline constexpr float WALL_THICKNESS{ 0.12f };

	/// @brief 枡の高さ
	inline constexpr float HEIGHT{ 1.4f };

	/// @brief 内側の底の高さ
	inline constexpr float FLOOR_TOP{ 0.14f };

	/// @brief 内側の半分の幅
	inline constexpr float INNER_HALF{ OUTER_HALF - WALL_THICKNESS };

	/// @brief 内側に液体が入る高さ
	inline constexpr float INNER_HEIGHT{ HEIGHT - FLOOR_TOP };

	/**
	 * @brief 嵩の割合から液面の高さを求める
	 * @param ratio 嵩（0.0〜1.0）
	 * @return 液面の高さ
	 */
	inline constexpr float surfaceHeight(float ratio) noexcept
	{
		return FLOOR_TOP + INNER_HEIGHT * ratio;
	}
} // namespace game::view::masu
