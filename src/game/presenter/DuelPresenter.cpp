#include "game/presenter/DuelPresenter.h"
#include "core/interface/IInputProvider.h"
#include "game/view/IPourView.h"

namespace
{
	/// @brief 1秒間に注がれる量（湯呑一杯を 1.0 とした量）
	///
	/// 注ぐ速さは器によらず一定。大きい器ほど満ちるまでに時間がかかる
	constexpr float POUR_VOLUME_RATE{ 0.30f };

	/// @brief 相手（NPC）が注ぎ止めるまでにかかる、狙いの手前の余裕
	///
	/// 狙いちょうどで止めようとすると、更新の刻みぶん行き過ぎる。
	/// 人が指を離すときの遅れにも当たるので、少しだけ残して止める
	constexpr float NPC_STOP_MARGIN{ 0.001f };

	/// @brief 札を返してから対局へ移るまでの間（秒）
	///
	/// 返す動き・役の振り分け・読む間を合わせた長さ
	constexpr float REVEAL_HOLD{ 3.0f };
} // namespace

namespace game::presenter
{
	DuelPresenter::DuelPresenter(game::view::IPourView& view, core::iface::IInputProvider& input,
	                             model::NpcType npc, unsigned int seed)
	    : m_view{ view }, m_input{ input }, m_npc{ npc }, m_random{ seed }
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
				updateCardDraw();
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

		case Phase::TurnPassing:
			// 告げ終わるまでは誰も注げない。誰の番になったのかを見てから始めさせる
			m_callTime += deltaTime;
			if (m_callTime >= game::view::TurnCall::READABLE_TIME)
				enterReady();
			break;

		case Phase::Ready:
			if (isNpcTurn())
			{
				updateNpc(deltaTime);
				break;
			}

			if (m_input.isKeyDown(key))
				m_phase = Phase::Pouring;
			break;

		case Phase::Pouring:
			if (isNpcTurn())
			{
				updateNpc(deltaTime);
				break;
			}

			if (!m_input.isKeyDown(key))
			{
				// こぼしたなら、手を離したところで決着を見せる
				if (m_duel.isRoundOver())
					endRound();
				else
					finishTurn();
				break;
			}

			// こぼれてからも、押している間はあふれ続ける（注ぐのをやめるのは自分で決める）
			pourFor(deltaTime);
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

	void DuelPresenter::enterReady()
	{
		// 相手は手番をもらってから少し迷う。すぐ注ぎ始めると機械に見える
		m_thinkTime = 0.0f;
		m_phase = Phase::Ready;
	}

	bool DuelPresenter::isNpcTurn() const noexcept
	{
		return m_duel.getCurrentPlayer() == model::Player::Two &&
		       model::npcOf(m_npc).isPresent();
	}

	void DuelPresenter::updateNpc(float deltaTime)
	{
		const model::Npc& npc{ model::npcOf(m_npc) };

		if (m_phase == Phase::Ready)
		{
			m_thinkTime += deltaTime;
			if (m_thinkTime < npc.hesitation)
				return;

			// 迷い終わったところで、この手番でどこまで注ぐかを決める。
			// 震えは外から渡す（Model に乱数を持ち込まないため）
			std::uniform_real_distribution<float> noise{ -1.0f, 1.0f };
			m_npcAim = model::decideAim(npc, m_duel.getAmount(),
			                                 model::Duel::getMinimumTurnAmount(), noise(m_random));
			m_phase = Phase::Pouring;
			return;
		}

		// こぼしてしまったら、そこで手を引く
		if (m_duel.isRoundOver())
		{
			endRound();
			return;
		}

		// 狙いまで注いだら手を引く。
		// ただし渡せる量に届くまでは引かない。引いても手番が渡らず、
		// 決め直しては止まるだけの堂々巡りになる
		if (m_duel.canEndTurn() && m_duel.getAmount() >= m_npcAim - NPC_STOP_MARGIN)
		{
			finishTurn();
			return;
		}

		pourFor(deltaTime);
	}

	void DuelPresenter::pourFor(float deltaTime)
	{
		// 注ぐ速さは一定なので、器が大きいほど嵩の上がり方は緩やかになる
		m_duel.pour(POUR_VOLUME_RATE * deltaTime / model::vesselOf(m_duel.getVessel()).capacity);
	}

	void DuelPresenter::endRound()
	{
		// 注ぎながら押していたぶんを持ち越すと、決着がすぐ飛ばされてしまう
		m_input.clearPendingPresses();
		m_phase = Phase::RoundOver;
	}

	void DuelPresenter::finishTurn()
	{
		// 最低量に届いていなければ手番は渡らない。人ならもう一度押せば続きから注げる
		const model::Player previous{ m_duel.getCurrentPlayer() };
		m_duel.endTurn();

		// 渡ったときだけ告げる（届かず押し直すときに出しては紛らわしい）
		if (m_duel.getCurrentPlayer() != previous)
		{
			beginTurnCall();
			return;
		}

		enterReady();
	}

	void DuelPresenter::beginDraw()
	{
		m_input.clearPendingPresses();

		// 二枚のうち一枚が「先攻」。どちらに伏せるかは引く前に決めておく
		std::uniform_int_distribution<int> coin{ 0, 1 };
		m_firstCardSide = coin(m_random);

		m_firstPlayer = model::Player::One;
		m_cardHighlight = 0;
		m_cardPicked = -1;
		m_isCardRevealed = false;
		m_revealedTime = 0.0f;
		m_phase = Phase::Draw;
	}

	void DuelPresenter::updateCardDraw()
	{
		// 押された印は必ず全部消費する（残すと次の場面へ持ち越されてしまう）
		const bool isLeft{ m_input.consumeKeyPress(core::input::KeyCode::Left) };
		const bool isRight{ m_input.consumeKeyPress(core::input::KeyCode::Right) };
		const bool isSwapped{ m_input.consumeKeyPress(core::input::KeyCode::Space) };
		const bool isDecided{ m_input.consumeKeyPress(core::input::KeyCode::Enter) };

		if (isLeft)
			m_cardHighlight = 0;

		if (isRight)
			m_cardHighlight = 1;

		if (isSwapped)
			m_cardHighlight = 1 - m_cardHighlight;

		// 札に触れているならそちらを指す。そのまま押せば引ける
		const int hovered{ m_view.hitTestCard(m_input.getMousePosition()) };
		if (hovered >= 0)
			m_cardHighlight = hovered;

		const bool isClicked{ m_input.isMouseLeftPressed() && hovered >= 0 };
		if (!isClicked && !isDecided)
			return;

		// 引いた札が「先攻」なら、引いた一の手が先に注ぐ
		m_cardPicked = m_cardHighlight;
		m_firstPlayer = m_cardPicked == m_firstCardSide ? model::Player::One : model::Player::Two;
		m_isCardRevealed = true;
		m_revealedTime = 0.0f;
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

		// 局の初手も、誰から始まるのかを告げてから始める
		beginTurnCall();
	}

	void DuelPresenter::beginTurnCall()
	{
		++m_turnSerial;
		m_callTime = 0.0f;
		m_thinkTime = 0.0f;
		m_phase = Phase::TurnPassing;
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

	std::string DuelPresenter::nameOf(model::Player player) const
	{
		// 二の手を相手（NPC）が打つなら、その呼び名で通す
		if (player == model::Player::Two)
			return model::npcOf(m_npc).name;

		return "一の手";
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
		// 相手を選ぶ場面と札を引く場面では、前の勝負の残りが見えないよう器を空にしておく。
		// こぼした場面では縁を越えた状態を見せる
		const bool isDrawing{ m_phase == Phase::Draw };
		m_view.showAmount(isDrawing               ? 0.0f
		                  : m_duel.isOverflowed() ? 1.0f
		                                          : m_duel.getAmount());
		m_view.showVessel(lookOf(m_duel.getVessel()));
		m_view.showPouring(m_phase == Phase::Pouring);
		m_view.showOverflowed(!isDrawing && m_duel.isOverflowed());
		m_view.showTurn(buildTurnLabel());
		// 一の手は左、二の手は右。告知もその側から出すと、どちらの番か動きで分かる
		m_view.showTurnCall(m_turnSerial, nameOf(m_duel.getCurrentPlayer()) + " の番",
		                    m_duel.getCurrentPlayer() == model::Player::One);
		m_view.showScore(buildScoreLabel());
		// 札を引くのは一の手。引いた札が「先攻」なら一の手が先、「後攻」なら二の手が先
		const bool isFirstCard{ m_firstPlayer == model::Player::One };
		m_view.showCardDraw(m_phase == Phase::Draw, m_cardHighlight, m_cardPicked, m_isCardRevealed,
		                    isFirstCard,
		                    nameOf(model::Player::One) + "  " + (isFirstCard ? "先攻" : "後攻"),
		                    nameOf(model::Player::Two) + "  " + (isFirstCard ? "後攻" : "先攻"));

		const std::string keyName{ m_duel.getCurrentPlayer() == model::Player::One ? "スペース"
			                                                                      : "Enter" };

		// 相手が打っている間は、こちらの操作の案内を出さず、何をしているかを見せる
		const std::string waitPrompt{ isNpcTurn()
			                              ? nameOf(model::Player::Two) + " が思案している"
			                              : keyName + " を押している間だけ注がれる" };

		switch (m_phase)
		{
		case Phase::Draw:
			if (!m_isCardRevealed)
			{
				m_view.showMessage("折据（おりすえ）から札を引く");
				m_view.showPrompt("←→ で選び、Enter で引く　　札を押してもよい");
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

		case Phase::TurnPassing:
			// 告げている間は下の案内を伏せる。中央の告知だけに目を向けさせる
			m_view.showMessage("");
			m_view.showPrompt("");
			break;

		case Phase::Ready:
			if (m_duel.getTurnAmount() > 0.0f && !m_duel.canEndTurn())
			{
				m_view.showMessage("まだ渡せない。もう少し注げ");
				m_view.showPrompt(waitPrompt);
				break;
			}

			// 嵩は数字で出さない。目で見て決めるのがこの勝負の要
			m_view.showMessage(model::vesselOf(m_duel.getVessel()).name);
			m_view.showPrompt(waitPrompt);
			break;

		case Phase::Pouring:
			m_view.showMessage(model::vesselOf(m_duel.getVessel()).name);
			m_view.showPrompt(isNpcTurn() ? nameOf(model::Player::Two) + " が注いでいる"
			                              : "離せば手番を渡す");
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
