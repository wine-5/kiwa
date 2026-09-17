#pragma once
#include "game/model/Judgement.h"

namespace game::model
{
	/**
	 * @brief 枡へ注いだ嵩と、こぼれる際（きわ）を持つルールの本体
	 *
	 * 時間も乱数も描画も知らない。外から「どれだけ注いだか」を渡されて嵩を増やし、
	 * いまどういう状態かを答えるだけ。こうしておくと画面を出さずにルールを試せる。
	 * 嵩は枡の満杯を 1.0 とした割合で扱う
	 */
	class PourMatch
	{
	  public:
		/**
		 * @brief PourMatch のコンストラクタ
		 * @param limit こぼれ始める嵩（0.0〜1.0）
		 */
		explicit PourMatch(float limit) noexcept;

		/**
		 * @brief 注ぐ
		 * @details こぼした後はそれ以上増えない（際を越えた事実だけが残る）
		 * @param amount 注ぐ嵩
		 */
		void pour(float amount) noexcept;

		/**
		 * @brief 新しい際で注ぎ直す
		 * @param limit こぼれ始める嵩（0.0〜1.0）
		 */
		void reset(float limit) noexcept;

		/**
		 * @brief いま注がれている嵩を返す
		 * @return 嵩（0.0〜1.0）
		 */
		[[nodiscard]] float getAmount() const noexcept
		{
			return m_amount;
		}

		/**
		 * @brief こぼれ始める嵩を返す
		 * @return 際の嵩（0.0〜1.0）
		 */
		[[nodiscard]] float getLimit() const noexcept
		{
			return m_limit;
		}

		/**
		 * @brief 際まであとどれだけ残っているかを返す
		 * @return 残りの嵩（こぼしている場合は負の値）
		 */
		[[nodiscard]] float getRemaining() const noexcept
		{
			return m_limit - m_amount;
		}

		/**
		 * @brief こぼしたかどうかを返す
		 * @return こぼしているならtrue
		 */
		[[nodiscard]] bool isOverflowed() const noexcept
		{
			return m_isOverflowed;
		}

		/**
		 * @brief いまの嵩の出来ばえを返す
		 * @return 出来ばえ
		 */
		[[nodiscard]] Judgement judge() const noexcept;

	  private:
		/// @brief こぼれ始める嵩（0.0〜1.0）
		float m_limit{ 1.0f };

		/// @brief いま注がれている嵩（0.0〜1.0）
		float m_amount{ 0.0f };

		/// @brief 際を越えたか
		bool m_isOverflowed{ false };
	};
} // namespace game::model
