#include "game/presenter/PourPresenter.h"
#include "core/interface/IInputProvider.h"
#include "game/view/IPourView.h"
#include <string>

namespace
{
	/// @brief 1秒間に注がれる嵩
	constexpr float POUR_RATE{ 0.30f };

	/// @brief 際が現れうる下限
	constexpr float LIMIT_MIN{ 0.45f };

	/// @brief 際が現れうる上限
	constexpr float LIMIT_MAX{ 0.95f };

	/**
	 * @brief 割合を百分率の文字列にする
	 * @param ratio 割合（0.0〜1.0）
	 * @return 「82%」のような文字列
	 */
	std::string toPercent(float ratio)
	{
		return std::to_string(static_cast<int>(ratio * 100.0f + 0.5f)) + "%";
	}
} // namespace

namespace game::presenter
{
	PourPresenter::PourPresenter(game::view::IPourView& view, core::iface::IInputProvider& input,
	                             unsigned int seed)
	    : m_view{ view }, m_input{ input }, m_match{ LIMIT_MAX }, m_random{ seed }
	{
		beginRound();
	}

	void PourPresenter::update(float deltaTime)
	{
		switch (m_phase)
		{
		case Phase::Ready:
			if (m_input.isKeyDown(core::input::KeyCode::Space))
				m_phase = Phase::Pouring;
			break;

		case Phase::Pouring:
			if (!m_input.isKeyDown(core::input::KeyCode::Space))
			{
				finishRound();
				break;
			}
			m_match.pour(POUR_RATE * deltaTime);
			if (m_match.isOverflowed())
				finishRound();
			break;

		case Phase::Judged:
			if (m_input.isKeyPressed(core::input::KeyCode::R))
				beginRound();
			break;
		}

		pushToView();
	}

	void PourPresenter::beginRound()
	{
		std::uniform_real_distribution<float> distribution{ LIMIT_MIN, LIMIT_MAX };
		m_match.reset(distribution(m_random));
		m_phase = Phase::Ready;
	}

	void PourPresenter::finishRound()
	{
		m_phase = Phase::Judged;
	}

	void PourPresenter::pushToView()
	{
		m_view.showAmount(m_match.getAmount());
		m_view.showLimit(m_match.getLimit(), true); // 際を見せる練習の段階
		m_view.showPouring(m_phase == Phase::Pouring);
		m_view.showOverflowed(m_match.isOverflowed());

		switch (m_phase)
		{
		case Phase::Ready:
			m_view.showMessage("際（きわ）を狙って注げ");
			m_view.showPrompt("スペースキーを押している間だけ注がれる");
			break;

		case Phase::Pouring:
			m_view.showMessage("嵩 " + toPercent(m_match.getAmount()));
			m_view.showPrompt("離すと止まる");
			break;

		case Phase::Judged:
			m_view.showMessage(buildResultMessage());
			m_view.showPrompt("R でもう一度");
			break;
		}
	}

	std::string PourPresenter::buildResultMessage() const
	{
		const std::string figures{ "  嵩 " + toPercent(m_match.getAmount()) + " / 際 " +
			                       toPercent(m_match.getLimit()) };

		switch (m_match.judge())
		{
		case model::Judgement::Overflow:
			return "こぼした" + figures;
		case model::Judgement::Perfect:
			return "ぴったり。見事な際" + figures;
		case model::Judgement::Excellent:
			return "あと少しで際" + figures;
		case model::Judgement::Good:
			return "悪くない" + figures;
		case model::Judgement::Poor:
		default:
			return "まだ遠い" + figures;
		}
	}
} // namespace game::presenter
