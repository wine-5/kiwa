#pragma once
#include <algorithm>
#include <array>

namespace game::model
{
	/**
	 * @brief 二の手を打つ者
	 *
	 * None は「人がもう一人で打つ」ことを指す。この場合 NPC はいない
	 */
	enum class NpcType
	{
		None,      ///< 人が打つ（二人で打つ）
		Monk,      ///< 老僧。慎重
		Samurai,   ///< 武士。豪胆
		Performer, ///< 旅芸人。気まぐれ
	};

	/**
	 * @brief NPC の性格
	 *
	 * 際（器の縁）は誰にも見えているので、違いは「どこで手を止めるか」に出る。
	 * 数字を持つだけで、時間も乱数も知らない
	 */
	struct Npc
	{
		/// @brief 誰が打つか
		NpcType type{ NpcType::None };

		/// @brief 呼び名
		const char* name{ "二の手" };

		/// @brief 狙う嵩（ここまで注いで手を止める）
		float aim{ 0.0f };

		/// @brief 手の震え（狙いからどれだけずれるか）
		float tremor{ 0.0f };

		/// @brief 迷う時間（秒）。長いほど考え込んで見える
		float hesitation{ 0.0f };

		/**
		 * @brief NPC が打つかを返す
		 * @return NPC が打つならtrue（人が打つなら false）
		 */
		[[nodiscard]] constexpr bool isPresent() const noexcept
		{
			return type != NpcType::None;
		}
	};

	/// @brief 選べる打ち手の一覧（先頭は人が打つ場合）
	inline constexpr std::array<Npc, 4> NPCS{ {
		{ NpcType::None, "二の手", 0.0f, 0.0f, 0.0f },
		{ NpcType::Monk, "老僧", 0.62f, 0.05f, 1.20f },
		{ NpcType::Samurai, "武士", 0.90f, 0.07f, 0.45f },
		{ NpcType::Performer, "旅芸人", 0.78f, 0.18f, 0.80f },
	} };

	/**
	 * @brief 種類から打ち手を引く
	 * @param type 誰が打つか
	 * @return その打ち手
	 */
	[[nodiscard]] inline constexpr const Npc& npcOf(NpcType type) noexcept
	{
		for (const Npc& npc : NPCS)
		{
			if (npc.type == type)
				return npc;
		}
		return NPCS[0];
	}

	/**
	 * @brief その手番で狙う嵩を決める
	 *
	 * 狙いは性格ごとの嵩だが、そのぶれは外から渡してもらう（乱数を持ち込まないため）。
	 * すでに狙いを越えて注がれているときは、渡すのに要る最低限だけ足して手を引く。
	 * 際を越えれば自分の負けなので、越える判断は「震え」の結果としてだけ起こる
	 * @param npc 打ち手
	 * @param amount いま注がれている嵩
	 * @param minimum 手番を渡すのに要る最低の嵩
	 * @param noise 震えのぶれ（-1.0〜1.0）
	 * @return 手を止める嵩
	 */
	[[nodiscard]] inline float decideAim(const Npc& npc, float amount, float minimum,
	                                     float noise) noexcept
	{
		const float aim{ npc.aim + npc.tremor * std::clamp(noise, -1.0f, 1.0f) };

		// 狙いまで届いていれば、そこで止める。越えていれば最低限だけ注いで渡す
		return std::max(aim, amount + minimum);
	}
} // namespace game::model
