#pragma once
#include "game/model/Duel.h"
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
	 * 際は毎局ここで引く（Model に乱数を持ち込まないため）
	 */
	class DuelPresenter
	{
	  public:
		/**
		 * @brief DuelPresenter のコンストラクタ
		 * @param view 映す先
		 * @param input 入力の取得
		 * @param seed 際を決める乱数の種
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
			Ready,     // 手番の側が注ぎ始めるのを待っている
			Pouring,   // 注いでいる
			RoundOver, // 決着がついている
		};

		/**
		 * @brief 新しい際を引いて次の局を始める
		 */
		void beginRound();

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
		Phase m_phase{ Phase::Ready };

		/// @brief 際を決める乱数
		std::mt19937 m_random;
	};
} // namespace game::presenter
