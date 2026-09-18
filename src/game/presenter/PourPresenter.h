#pragma once
#include "game/model/PourMatch.h"
#include <random>

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
	 * @brief 注ぎ勝負の進行役
	 *
	 * 入力を読んで Model へ伝え、Model の状態を View へ渡す。描画の手段は知らないので、
	 * View を差し替えれば画面を出さずにこの進行をそのまま動かせる
	 */
	class PourPresenter
	{
	  public:
		/**
		 * @brief PourPresenter のコンストラクタ
		 * @param view 映す先
		 * @param input 入力の取得
		 * @param seed 際を決める乱数の種
		 */
		PourPresenter(game::view::IPourView& view, core::iface::IInputProvider& input, unsigned int seed);

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
			Ready,  // 注ぎ始めるのを待っている
			Pouring, // 注いでいる
			Judged, // 注ぎ終えて結果が出ている
		};

		/**
		 * @brief 新しい際を引いて注ぎ直す
		 */
		void beginRound();

		/**
		 * @brief 注ぐのをやめて結果を出す
		 */
		void finishRound();

		/**
		 * @brief いまの状態を View へ渡す
		 */
		void pushToView();

		/**
		 * @brief 結果の文言を返す
		 * @return 文言
		 */
		[[nodiscard]] std::string buildResultMessage() const;

		game::view::IPourView& m_view;
		core::iface::IInputProvider& m_input;

		model::PourMatch m_match;
		Phase m_phase{ Phase::Ready };

		/// @brief 際を決める乱数
		std::mt19937 m_random;
	};
} // namespace game::presenter
