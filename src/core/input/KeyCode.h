#pragma once

namespace core::input
{
	/**
	 * @brief DxLib に依存しない独自のキーコード
	 *
	 * 末尾の Count は配列の要素数として使うため、必ず最後に置く
	 */
	enum class KeyCode
	{
		W,
		A,
		S,
		D,
		Up,
		Down,
		Left,
		Right,
		Space,
		Enter,
		Escape,
		Shift,
		R,
		Z,
		X,
		F1,

		Count, // 要素数（列挙の最後に置くこと）
	};
} // namespace core::input
