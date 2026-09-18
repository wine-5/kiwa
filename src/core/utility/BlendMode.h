#pragma once

namespace core::utility
{
	/**
	 * @brief 描画の重ね方
	 */
	enum class BlendMode
	{
		None,  // そのまま描く（不透明）
		Alpha, // 透かして重ねる
		Add,   // 光を足すように重ねる（照りや輝きに使う）
	};
} // namespace core::utility
