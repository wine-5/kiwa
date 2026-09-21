#pragma once
#include "game/model/Duel.h"
#include "game/model/Npc.h"
#include "game/view/TurnCall.h"
#include "game/view/VesselLook.h"
#include "core/input/KeyCode.h"
#include <random>
#include <string>

namespace core::iface
{
	class IInputProvider; // 前方宣言
} // namespace core::iface

namespace game::view
{
	class IPourView; // 前方宣言
} // namespace game::view

namespace game::presenter
{
	/**
	 * @brief 二人で注ぎ合う勝負の進行役
	 *
	 * 手番の側の入力だけを読んで Model へ伝え、Model の状態を View へ渡す。
	 * 器は毎局ここで引く（Model に乱数を持ち込まないため）
	 */
	class DuelPresenter
	{
	  public:
		/**
		 * @brief DuelPresenter のコンストラクタ
		 * @param view 映す先
		 * @param input 入力の取得
		 * @param npc 二の手を誰が打つか（None なら二人で打つ）
		 * @param seed 先攻と器を決める乱数の種
		 */
		DuelPresenter(game::view::IPourView& view, core::iface::IInputProvider& input,
		              model::NpcType npc, unsigned int seed);

		/**
		 * @brief 進行を1ステップ進める
		 * @param deltaTime 進める時間（秒）
		 */
		void update(float deltaTime);

	  private:
		/**
		 * @brief 進行の段階
		 */
		enum class Phase
		{
			Draw,       // 花月の札で先攻を決めている
			TurnPassing, // 手番が移ったことを告げている（この間は注げない）
			Ready,      // 手番の側が注ぎ始めるのを待っている
			Pouring,   // 注いでいる
			RoundOver, // その局の決着がついている
			MatchOver, // 試合の決着がついている
		};

		/**
		 * @brief 札を引く場面から始める（先攻を引き直す）
		 */
		void beginDraw();

		/**
		 * @brief 注ぎ始めるのを待つ場面へ移る
		 */
		void enterReady();

		/**
		 * @brief いまの手番を相手（NPC）が打っているかを返す
		 * @return NPC の手番ならtrue
		 */
		[[nodiscard]] bool isNpcTurn() const noexcept;

		/**
		 * @brief 相手（NPC）の手を進める
		 * @param deltaTime 進める時間（秒）
		 */
		void updateNpc(float deltaTime);

		/**
		 * @brief この刻みぶん注ぐ
		 * @param deltaTime 進める時間（秒）
		 */
		void pourFor(float deltaTime);

		/**
		 * @brief 手番を終える（渡れば告知へ、渡らなければ待ちへ）
		 */
		void finishTurn();

		/**
		 * @brief その局の決着を見せる場面へ移る
		 */
		void endRound();

		/**
		 * @brief 新しい器を引いて次の局を始める
		 */
		void beginRound();

		/**
		 * @brief 相手の性格をひと言で返す
		 * @param opponent 相手
		 * @return ひと言（「慎重」など）
		 */
		[[nodiscard]] static std::string noteOf(const model::Npc& npc);

		/**
		 * @brief 器の種類を見た目の種類へ読み替える
		 * @param vessel 器の種類
		 * @return 見た目の種類
		 */
		[[nodiscard]] static game::view::VesselLook lookOf(model::VesselType vessel) noexcept;

		/**
		 * @brief 手番が移ったことを告げ始める
		 */
		void beginTurnCall();

		/**
		 * @brief 札を引く場面の操作を読む
		 */
		void updateCardDraw();

		/**
		 * @brief どちらかのキーが押されたかを返す
		 * @return 押されたならtrue
		 */
		[[nodiscard]] bool isAnyKeyPressed();

		/**
		 * @brief いまの状態を View へ渡す
		 */
		void pushToView();

		/**
		 * @brief その側が使うキーを返す
		 * @param player 手番の側
		 * @return キーコード
		 */
		[[nodiscard]] static core::input::KeyCode keyFor(model::Player player) noexcept;

		/**
		 * @brief その側の呼び名を返す
		 * @param player 手番の側
		 * @return 呼び名
		 */
		[[nodiscard]] std::string nameOf(model::Player player) const;

		/**
		 * @brief 手番の表示を作る
		 * @return 文言
		 */
		[[nodiscard]] std::string buildTurnLabel() const;

		/**
		 * @brief 勝敗の表示を作る
		 * @return 文言
		 */
		[[nodiscard]] std::string buildScoreLabel() const;

		game::view::IPourView& m_view;
		core::iface::IInputProvider& m_input;

		model::Duel m_duel{};
		Phase m_phase{ Phase::Draw };

		/// @brief 札が示した先攻の側
		model::Player m_firstPlayer{ model::Player::One };

		/// @brief 札を返したか
		bool m_isCardRevealed{ false };

		/// @brief いま指している札（0が左、1が右）
		int m_cardHighlight{ 0 };

		/// @brief 引いた札（まだ引いていなければ -1）
		int m_cardPicked{ -1 };

		/// @brief 「先攻」が伏せられている側（0が左、1が右）
		int m_firstCardSide{ 0 };

		/// @brief 札を返してから経った時間（秒）
		float m_revealedTime{ 0.0f };

		/// @brief 手番の通し番号（渡るたびに増やし、View はこれを見て告げ直す）
		int m_turnSerial{ 0 };

		/// @brief 手番を告げ始めてから経った時間（秒）
		float m_callTime{ 0.0f };

		/// @brief 二の手を誰が打つか
		model::NpcType m_npc{ model::NpcType::None };

		/// @brief 相手が迷っている時間（秒）
		float m_thinkTime{ 0.0f };

		/// @brief 相手がこの手番で狙う嵩
		float m_npcAim{ 0.0f };

		/// @brief 先攻と器を決める乱数
		std::mt19937 m_random;
	};
} // namespace game::presenter
