#include "game/presenter/DuelPresenter.h"
#include "core/interface/IInputProvider.h"
#include "game/view/IPourView.h"

namespace
{
	/// @brief 1秒間に注がれる嵩
	constexpr float POUR_RATE{ 0.30f };

	/// @brief 際が現れうる下限
	constexpr float LIMIT_MIN{ 0.55f };

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
	DuelPresenter::DuelPresenter(game::view::IPourView& view, core::iface::IInputProvider& input,
	                             unsigned int seed)
	    : m_view{ view }, m_input{ input }, m_random{ seed }
	{
		beginRound();
	}

	void DuelPresenter::update(float deltaTime)
	{
		const core::input::KeyCode key{ keyFor(m_duel.getCurrentPlayer()) };

		switch (m_phase)
		{
		case Phase::Ready:
			if (m_input.isKeyDown(key))
				m_phase = Phase::Pouring;
			break;

		case Phase::Pouring:
			if (!m_input.isKeyDown(key))
			{
				// 最低量に届いていなければ手番は渡らない。もう一度押せば続きから注げる
				m_duel.endTurn();
				m_phase = Phase::Ready;
				break;
			}

			m_duel.pour(POUR_RATE * deltaTime);
			if (m_duel.isRoundOver())
				m_phase = Phase::RoundOver;
			break;

		case Phase::RoundOver:
			if (m_input.isKeyPressed(core::input::KeyCode::Space) ||
			    m_input.isKeyPressed(core::input::KeyCode::Enter))
				beginRound();
			break;
		}

		pushToView();
	}

	void DuelPresenter::beginRound()
	{
		std::uniform_real_distribution<float> distribution{ LIMIT_MIN, LIMIT_MAX };
		m_duel.startRound(distribution(m_random));
		m_phase = Phase::Ready;
	}

	core::input::KeyCode DuelPresenter::keyFor(model::Player player) noexcept
	{
		return player == model::Player::One ? core::input::KeyCode::Space : core::input::KeyCode::Enter;
	}

	std::string DuelPresenter::nameOf(model::Player player)
	{
		return player == model::Player::One ? "一の手" : "二の手";
	}

	std::string DuelPresenter::buildTurnLabel() const
	{
		if (m_phase == Phase::RoundOver)
			return nameOf(m_duel.getWinner()) + " の勝ち";

		return nameOf(m_duel.getCurrentPlayer()) + " の番";
	}

	std::string DuelPresenter::buildScoreLabel() const
	{
		return "一の手 " + std::to_string(m_duel.getScore(model::Player::One)) + " - " +
		       std::to_string(m_duel.getScore(model::Player::Two)) + " 二の手";
	}

	void DuelPresenter::pushToView()
	{
		// こぼした場面では器は縁まで満ちて見えるべきなので、絵の上では満杯にする。
		// 際の値は勝負の都合で低いこともあるが、それは数字の話で見た目とは別
		m_view.showAmount(m_duel.isOverflowed() ? 1.0f : m_duel.getAmount());
		m_view.showLimit(m_duel.getLimit(), false); // 際は見せない。それがこの勝負の要
		m_view.showPouring(m_phase == Phase::Pouring);
		m_view.showOverflowed(m_duel.isOverflowed());
		m_view.showTurn(buildTurnLabel());
		m_view.showScore(buildScoreLabel());

		const std::string keyName{ m_duel.getCurrentPlayer() == model::Player::One ? "スペース"
			                                                                      : "Enter" };

		switch (m_phase)
		{
		case Phase::Ready:
			if (m_duel.getTurnAmount() > 0.0f && !m_duel.canEndTurn())
			{
				m_view.showMessage("まだ渡せない。もう少し注げ");
				m_view.showPrompt(keyName + " を押している間だけ注がれる");
				break;
			}

			m_view.showMessage("嵩 " + toPercent(m_duel.getAmount()));
			m_view.showPrompt(keyName + " を押している間だけ注がれる");
			break;

		case Phase::Pouring:
			m_view.showMessage("嵩 " + toPercent(m_duel.getAmount()));
			m_view.showPrompt("離せば手番を渡す");
			break;

		case Phase::RoundOver:
			m_view.showMessage(nameOf(m_duel.getLoser()) + " がこぼした  際 " +
			                   toPercent(m_duel.getLimit()));
			m_view.showPrompt("どちらかのキーで次の勝負へ");
			break;
		}
	}
} // namespace game::presenter
