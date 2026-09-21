#pragma once
#include "game/model/Npc.h"
#include "game/model/Player.h"

namespace game::model
{
	/**
	 * @brief 一つの対局の、始め方と終わり方
	 *
	 * タイトルで選んだことを対局へ運び、対局の結末をリザルトへ運ぶための入れ物。
	 * シーンをまたいで残るので Application が持ち、必要なシーンへ参照で手渡す
	 */
	struct MatchSetup
	{
		/// @brief 二の手を誰が打つか（None なら二人で打つ）
		NpcType npc{ NpcType::None };

		/// @brief 試合に勝った側（決着がつくまでの値に意味はない）
		Player winner{ Player::One };

		/// @brief 一の手が取った数
		int oneWins{ 0 };

		/// @brief 二の手が取った数
		int twoWins{ 0 };
	};
} // namespace game::model
