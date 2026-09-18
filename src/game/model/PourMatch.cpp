#include "game/model/PourMatch.h"

namespace
{
	/// @brief ぴったりと認める際からの隔たり
	constexpr float PERFECT_GAP{ 0.01f };

	/// @brief かなり近いと認める際からの隔たり
	constexpr float EXCELLENT_GAP{ 0.03f };

	/// @brief 近いと認める際からの隔たり
	constexpr float GOOD_GAP{ 0.08f };
} // namespace

namespace game::model
{
	PourMatch::PourMatch(float limit) noexcept : m_limit{ limit }
	{
	}

	void PourMatch::pour(float amount) noexcept
	{
		if (m_isOverflowed)
			return;

		m_amount += amount;
		if (m_amount > m_limit)
			m_isOverflowed = true;
	}

	void PourMatch::reset(float limit) noexcept
	{
		m_limit = limit;
		m_amount = 0.0f;
		m_isOverflowed = false;
	}

	Judgement PourMatch::judge() const noexcept
	{
		if (m_isOverflowed)
			return Judgement::Overflow;

		const float gap{ getRemaining() };
		if (gap <= PERFECT_GAP)
			return Judgement::Perfect;
		if (gap <= EXCELLENT_GAP)
			return Judgement::Excellent;
		if (gap <= GOOD_GAP)
			return Judgement::Good;
		return Judgement::Poor;
	}
} // namespace game::model
