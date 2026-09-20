#include "game/model/Duel.h"

namespace
{
	/// @brief 手番を渡すために注がなければならない最低の嵩
	constexpr float MINIMUM_TURN_AMOUNT{ 0.04f };
} // namespace

namespace game::model
{
	void Duel::startRound(float limit) noexcept
	{
		m_match.reset(limit);
		m_turnAmount = 0.0f;

		// 負けた側から始める。先に注ぐほうが空の器に注げて安全なので、その一手を渡す
		if (m_isRoundOver)
			m_currentPlayer = m_loser;

		m_isRoundOver = false;
	}

	void Duel::pour(float amount) noexcept
	{
		if (m_isRoundOver)
			return;

		m_match.pour(amount);
		m_turnAmount += amount;

		if (!m_match.isOverflowed())
			return;

		m_isRoundOver = true;
		m_loser = m_currentPlayer;

		if (getWinner() == Player::One)
			++m_scoreOne;
		else
			++m_scoreTwo;
	}

	bool Duel::canEndTurn() const noexcept
	{
		return !m_isRoundOver && m_turnAmount >= MINIMUM_TURN_AMOUNT;
	}

	void Duel::endTurn() noexcept
	{
		if (!canEndTurn())
			return;

		m_currentPlayer = returnOpponent(m_currentPlayer);
		m_turnAmount = 0.0f;
	}
} // namespace game::model
