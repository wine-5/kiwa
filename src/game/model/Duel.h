#pragma once
#include "game/model/Player.h"
#include "game/model/PourMatch.h"
#include "game/model/Vessel.h"

namespace game::model
{
	/**
	 * @brief 二人で交互に注ぐ勝負のルール
	 *
	 * 交互に注ぎ、こぼした方が負け。手番を渡すには最低量を注がなければならない
	 * （注がずに渡せてしまうと勝負にならないため）。
	 *
	 * 際は器の口そのもの（常に満杯）で、隠された数字ではない。代わりに毎局
	 * 器が変わり、口まで満たすのに要る量が変わる。どれだけ注げるかの見当は
	 * 器の大きさから立てることになる。
	 * PourMatch と同じく、時間も乱数も描画も知らない。器は外から渡される
	 */
	class Duel
	{
	  public:
		Duel() = default;

		/**
		 * @brief 試合を始める（勝敗を0に戻し、先攻を決める）
		 * @param firstPlayer 先攻の側
		 */
		void startMatch(Player firstPlayer) noexcept;

		/**
		 * @brief 新しい器で一局始める
		 * @details 先攻の側から注ぎ始める
		 * @param vessel この局で使う器
		 */
		void startRound(VesselType vessel) noexcept;

		/**
		 * @brief いまの手番が注ぐ
		 * @details こぼした時点でその局は終わり、相手の勝ちになる
		 * @param amount 注ぐ嵩
		 */
		void pour(float amount) noexcept;

		/**
		 * @brief 手番を相手へ渡す
		 * @details 最低量を注いでいない間は渡せない（呼んでも何も起こらない）
		 */
		void endTurn() noexcept;

		/**
		 * @brief 手番を渡せるかを返す
		 * @return 渡せるならtrue
		 */
		[[nodiscard]] bool canEndTurn() const noexcept;

		/**
		 * @brief この局の決着がついたかを返す
		 * @return ついているならtrue
		 */
		[[nodiscard]] bool isRoundOver() const noexcept
		{
			return m_isRoundOver;
		}

		/**
		 * @brief いまの手番を返す
		 * @return 手番の側
		 */
		[[nodiscard]] Player getCurrentPlayer() const noexcept
		{
			return m_currentPlayer;
		}

		/**
		 * @brief この局の勝者を返す
		 * @details 決着がついていない間の値に意味はない
		 * @return 勝った側
		 */
		[[nodiscard]] Player getWinner() const noexcept
		{
			return returnOpponent(m_loser);
		}

		/**
		 * @brief こぼした側を返す
		 * @details 決着がついていない間の値に意味はない
		 * @return こぼした側
		 */
		[[nodiscard]] Player getLoser() const noexcept
		{
			return m_loser;
		}

		/**
		 * @brief 試合の決着がついたかを返す
		 * @return どちらかが先取数に届いていればtrue
		 */
		[[nodiscard]] bool isMatchOver() const noexcept;

		/**
		 * @brief 試合に勝った側を返す
		 * @details 決着がついていない間の値に意味はない
		 * @return 勝った側
		 */
		[[nodiscard]] Player getMatchWinner() const noexcept;

		/**
		 * @brief 試合に必要な勝ち数を返す
		 * @return 先取する本数
		 */
		[[nodiscard]] static int getTargetWins() noexcept;

		/**
		 * @brief 勝った回数を返す
		 * @param player 数える側
		 * @return 勝った回数
		 */
		[[nodiscard]] int getScore(Player player) const noexcept
		{
			return player == Player::One ? m_scoreOne : m_scoreTwo;
		}

		/**
		 * @brief いま注がれている嵩を返す
		 * @return 嵩（0.0〜1.0）
		 */
		[[nodiscard]] float getAmount() const noexcept
		{
			return m_match.getAmount();
		}

		/**
		 * @brief この局で使っている器を返す
		 * @return 器の種類
		 */
		[[nodiscard]] VesselType getVessel() const noexcept
		{
			return m_vessel;
		}

		/**
		 * @brief この手番で注いだ嵩を返す
		 * @return 嵩（0.0〜1.0）
		 */
		[[nodiscard]] float getTurnAmount() const noexcept
		{
			return m_turnAmount;
		}

		/**
		 * @brief こぼしたかどうかを返す
		 * @return こぼしているならtrue
		 */
		[[nodiscard]] bool isOverflowed() const noexcept
		{
			return m_match.isOverflowed();
		}

	  private:
		/// @brief 注がれた嵩と際
		PourMatch m_match{ 1.0f };

		/// @brief この局で使っている器
		VesselType m_vessel{ VesselType::Yunomi };

		/// @brief いまの手番
		Player m_currentPlayer{ Player::One };

		/// @brief 先攻の側（毎局この側から注ぎ始める）
		Player m_firstPlayer{ Player::One };

		/// @brief この手番で注いだ嵩
		float m_turnAmount{ 0.0f };

		/// @brief 決着がついたか
		bool m_isRoundOver{ false };

		/// @brief こぼした側
		Player m_loser{ Player::Two };

		/// @brief 一の手が勝った回数
		int m_scoreOne{ 0 };

		/// @brief 二の手が勝った回数
		int m_scoreTwo{ 0 };
	};
} // namespace game::model
