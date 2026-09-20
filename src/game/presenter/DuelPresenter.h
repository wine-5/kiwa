#pragma once
#include "game/model/Duel.h"
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
		 * @param seed 先攻と器を決める乱数の種
		 */
		DuelPresenter(game::view::IPourView& view, core::iface::IInputProvider& input, unsigned int seed);

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
			Draw,      // 花月の札で先攻を決めている
			Ready,     // 手番の側が注ぎ始めるのを待っている
			Pouring,   // 注いでいる
			RoundOver, // その局の決着がついている
			MatchOver, // 試合の決着がついている
		};

		/**
		 * @brief 札を引く場面から始める（先攻を引き直す）
		 */
		void beginDraw();

		/**
		 * @brief 新しい器を引いて次の局を始める
		 */
		void beginRound();

		/**
		 * @brief 器の種類を見た目の種類へ読み替える
		 * @param vessel 器の種類
		 * @return 見た目の種類
		 */
		[[nodiscard]] static game::view::VesselLook lookOf(model::VesselType vessel) noexcept;

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
		[[nodiscard]] static std::string nameOf(model::Player player);

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

		/// @brief 札を返してから経った時間（秒）
		float m_revealedTime{ 0.0f };

		/// @brief 先攻と器を決める乱数
		std::mt19937 m_random;
	};
} // namespace game::presenter
