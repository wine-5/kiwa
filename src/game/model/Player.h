#pragma once

namespace game::model
{
	/**
	 * @brief 対局する二人
	 */
	enum class Player
	{
		One, // 一の手
		Two, // 二の手
	};

	/**
	 * @brief 相手を返す
	 * @param player 一方
	 * @return もう一方
	 */
	[[nodiscard]] constexpr Player returnOpponent(Player player) noexcept
	{
		return player == Player::One ? Player::Two : Player::One;
	}
} // namespace game::model
