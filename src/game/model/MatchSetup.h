#pragma once
#include "game/model/Npc.h"

namespace game::model
{
	/**
	 * @brief 一つの対局をどう始めるか
	 *
	 * タイトルで選んだことを対局へ運ぶための入れ物。シーンをまたいで残るので
	 * Application が持ち、必要なシーンへ参照で手渡す
	 */
	struct MatchSetup
	{
		/// @brief 二の手を誰が打つか（None なら二人で打つ）
		NpcType npc{ NpcType::None };
	};
} // namespace game::model
