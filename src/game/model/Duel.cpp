#include "game/model/Duel.h"

namespace
{
	/// @brief 手番を渡すために注がなければならない最低の嵩
	///
	/// 「注がずに渡す」を防ぐためだけのもので、我慢を強いるためのものではない。
	/// 一瞬だけ押した（更新1回ぶん注いだ）なら渡せる大きさにしてある。
	/// いちばん大きい茶碗でも更新1回で 0.0027 ほど進むので、それを下回る値を置く
	constexpr float MINIMUM_TURN_AMOUNT{ 0.0025f };

	/// @brief 試合に必要な勝ち数
	constexpr int TARGET_WINS{ 3 };

	/// @brief こぼれ始める嵩
	///
	/// 際は器の口そのもの。隠された数字ではなく、常に「縁いっぱい」が限界になる
	constexpr float RIM_LIMIT{ 1.0f };
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

	void Duel::startRound(VesselType vessel) noexcept
	{
		m_vessel = vessel;
		m_match.reset(RIM_LIMIT);
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
