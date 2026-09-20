#include "game/presenter/DuelPresenter.h"
#include "core/interface/IInputProvider.h"
#include "game/view/IPourView.h"

namespace
{
	/// @brief 1秒間に注がれる量（湯呑一杯を 1.0 とした量）
	///
	/// 注ぐ速さは器によらず一定。大きい器ほど満ちるまでに時間がかかる
	constexpr float POUR_VOLUME_RATE{ 0.30f };

	/// @brief 札を返してから対局へ移るまでの間（秒）
	///
	/// 返す動き・役の振り分け・読む間を合わせた長さ
	constexpr float REVEAL_HOLD{ 3.0f };

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
		beginDraw();
	}

	void DuelPresenter::update(float deltaTime)
	{
		const core::input::KeyCode key{ keyFor(m_duel.getCurrentPlayer()) };

		switch (m_phase)
		{
		case Phase::Draw:
			if (!m_isCardRevealed)
			{
				if (isAnyKeyPressed())
					m_isCardRevealed = true;
				break;
			}

			// 札を返したあとは少し置いてから対局へ移る（読む間を作る）
			m_revealedTime += deltaTime;
			if (m_revealedTime >= REVEAL_HOLD)
			{
				m_duel.startMatch(m_firstPlayer);
				beginRound();
			}
			break;

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

			// 注ぐ速さは一定なので、器が大きいほど嵩の上がり方は緩やかになる
			m_duel.pour(POUR_VOLUME_RATE * deltaTime /
			            model::vesselOf(m_duel.getVessel()).capacity);
			if (m_duel.isRoundOver())
			{
				// 注ぎながら押していたぶんを持ち越すと、決着がすぐ飛ばされてしまう
				m_input.clearPendingPresses();
				m_phase = Phase::RoundOver;
			}
			break;

		case Phase::RoundOver:
			if (isAnyKeyPressed())
				beginRound();
			break;

		case Phase::MatchOver:
			if (isAnyKeyPressed())
				beginDraw();
			break;
		}

		pushToView();
	}

	void DuelPresenter::beginDraw()
	{
		m_input.clearPendingPresses();

		// 折据から引いた札で先攻を決める。引き当てるまで結果は伏せておく
		std::uniform_int_distribution<int> coin{ 0, 1 };
		m_firstPlayer = coin(m_random) == 0 ? model::Player::One : model::Player::Two;

		m_isCardRevealed = false;
		m_revealedTime = 0.0f;
		m_phase = Phase::Draw;
	}

	void DuelPresenter::beginRound()
	{
		if (m_duel.isMatchOver())
		{
			m_input.clearPendingPresses();
			m_phase = Phase::MatchOver;
			return;
		}

		// 器は毎局引き直す。際は常に口いっぱいなので、
		// 難しさは「その器がどれだけ入るか」で変わる
		std::uniform_int_distribution<std::size_t> pick{ 0, model::VESSELS.size() - 1 };
		m_duel.startRound(model::VESSELS[pick(m_random)].type);
		m_phase = Phase::Ready;
	}

	bool DuelPresenter::isAnyKeyPressed()
	{
		// どちらか一方でも消費されれば成立とする（両方消費しておかないと、
		// もう片方の押下が次の場面へ持ち越されてしまう）
		const bool space{ m_input.consumeKeyPress(core::input::KeyCode::Space) };
		const bool enter{ m_input.consumeKeyPress(core::input::KeyCode::Enter) };
		return space || enter;
	}

	core::input::KeyCode DuelPresenter::keyFor(model::Player player) noexcept
	{
		return player == model::Player::One ? core::input::KeyCode::Space : core::input::KeyCode::Enter;
	}

	game::view::VesselLook DuelPresenter::lookOf(model::VesselType vessel) noexcept
	{
		switch (vessel)
		{
		case model::VesselType::Guinomi:
			return game::view::VesselLook::Guinomi;
		case model::VesselType::Sobachoko:
			return game::view::VesselLook::Sobachoko;
		case model::VesselType::Chawan:
			return game::view::VesselLook::Chawan;
		case model::VesselType::Yunomi:
		default:
			return game::view::VesselLook::Yunomi;
		}
	}

	std::string DuelPresenter::nameOf(model::Player player)
	{
		return player == model::Player::One ? "一の手" : "二の手";
	}

	std::string DuelPresenter::buildTurnLabel() const
	{
		switch (m_phase)
		{
		case Phase::Draw:
			return m_isCardRevealed ? nameOf(m_firstPlayer) + " の先攻" : "先攻を決める";
		case Phase::RoundOver:
			return nameOf(m_duel.getWinner()) + " の勝ち";
		case Phase::MatchOver:
			return nameOf(m_duel.getMatchWinner()) + " の勝ち";
		default:
			return nameOf(m_duel.getCurrentPlayer()) + " の番";
		}
	}

	std::string DuelPresenter::buildScoreLabel() const
	{
		// 見出しとぶつからないよう短く保つ。先取数は決着の場面で伝える
		return "一 " + std::to_string(m_duel.getScore(model::Player::One)) + " - " +
		       std::to_string(m_duel.getScore(model::Player::Two)) + " 二";
	}

	void DuelPresenter::pushToView()
	{
		// 札を引く場面では、前の勝負の残りが見えないよう器を空にしておく。
		// こぼした場面では縁を越えた状態を見せる
		const bool isDrawing{ m_phase == Phase::Draw };
		m_view.showAmount(isDrawing               ? 0.0f
		                  : m_duel.isOverflowed() ? 1.0f
		                                          : m_duel.getAmount());
		m_view.showVessel(lookOf(m_duel.getVessel()));
		m_view.showPouring(m_phase == Phase::Pouring);
		m_view.showOverflowed(!isDrawing && m_duel.isOverflowed());
		m_view.showTurn(buildTurnLabel());
		m_view.showScore(buildScoreLabel());
		// 札を引くのは一の手。引いた札が「先攻」なら一の手が先、「後攻」なら二の手が先
		const bool isFirstCard{ m_firstPlayer == model::Player::One };
		m_view.showCardDraw(m_phase == Phase::Draw, m_isCardRevealed, isFirstCard,
		                    nameOf(model::Player::One) + "  " + (isFirstCard ? "先攻" : "後攻"),
		                    nameOf(model::Player::Two) + "  " + (isFirstCard ? "後攻" : "先攻"));

		const std::string keyName{ m_duel.getCurrentPlayer() == model::Player::One ? "スペース"
			                                                                      : "Enter" };

		switch (m_phase)
		{
		case Phase::Draw:
			if (!m_isCardRevealed)
			{
				m_view.showMessage("折据（おりすえ）から札を引く");
				m_view.showPrompt("どちらかのキーで引く");
				break;
			}

			m_view.showMessage(nameOf(m_firstPlayer) + " が先に注ぐ");
			m_view.showPrompt("");
			break;

		case Phase::MatchOver:
			m_view.showMessage(std::to_string(model::Duel::getTargetWins()) + "本先取  " +
			                   nameOf(m_duel.getMatchWinner()) + " の勝ち");
			m_view.showPrompt("どちらかのキーでもう一番");
			break;

		case Phase::Ready:
			if (m_duel.getTurnAmount() > 0.0f && !m_duel.canEndTurn())
			{
				m_view.showMessage("まだ渡せない。もう少し注げ");
				m_view.showPrompt(keyName + " を押している間だけ注がれる");
				break;
			}

			m_view.showMessage(std::string{ model::vesselOf(m_duel.getVessel()).name } + "　嵩 " +
			                   toPercent(m_duel.getAmount()));
			m_view.showPrompt(keyName + " を押している間だけ注がれる");
			break;

		case Phase::Pouring:
			m_view.showMessage(std::string{ model::vesselOf(m_duel.getVessel()).name } + "　嵩 " +
			                   toPercent(m_duel.getAmount()));
			m_view.showPrompt("離せば手番を渡す");
			break;

		case Phase::RoundOver:
			m_view.showMessage(nameOf(m_duel.getLoser()) + " がこぼした  " +
			                   model::vesselOf(m_duel.getVessel()).name + " は縁まで満ちた");
			m_view.showPrompt(m_duel.isMatchOver() ? "どちらかのキーで結果へ"
			                                       : "どちらかのキーで次の勝負へ");
			break;
		}
	}
} // namespace game::presenter
