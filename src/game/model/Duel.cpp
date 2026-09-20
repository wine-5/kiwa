#include "game/model/Duel.h"

namespace
{
	/// @brief 手番を渡すために注がなければならない最低の嵩
	constexpr float MINIMUM_TURN_AMOUNT{ 0.04f };

	/// @brief 試合に必要な勝ち数
	constexpr int TARGET_WINS{ 3 };
} // namespace

namespace game::model
{
	void Duel::startMatch(Player firstPlayer) noexcept
	{
		m_firstPlayer = firstPlayer;
		m_scoreOne = 0;
		m_scoreTwo = 0;
		m_isRoundOver = false;
	}

	void Duel::startRound(float limit) noexcept
	{
		m_match.reset(limit);
		m_turnAmount = 0.0f;
		m_currentPlayer = m_firstPlayer;
		m_isRoundOver = false;
	}

	bool Duel::isMatchOver() const noexcept
	{
		return m_scoreOne >= TARGET_WINS || m_scoreTwo >= TARGET_WINS;
	}

	Player Duel::getMatchWinner() const noexcept
	{
		return m_scoreOne >= m_scoreTwo ? Player::One : Player::Two;
	}

	int Duel::getTargetWins() noexcept
	{
		return TARGET_WINS;
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
