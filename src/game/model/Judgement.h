#pragma once

namespace game::model
{
	/**
	 * @brief 注ぎ終えたときの出来ばえ
	 *
	 * 際（きわ）にどれだけ近づけたかを表す。Overflow はこぼした状態
	 */
	enum class Judgement
	{
		Overflow, // こぼした
		Poor,     // まだ遠い
		Good,     // 近い
		Excellent, // かなり近い
		Perfect,  // ぴったり
	};
} // namespace game::model
