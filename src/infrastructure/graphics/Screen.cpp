#include "infrastructure/graphics/Screen.h"
#include "DxLib.h"

namespace infrastructure::graphics
{
	Screen::Screen(int width, int height) noexcept : m_width{ width }, m_height{ height }
	{
	}

	void Screen::setBackgroundColor(const core::utility::Color& color) noexcept
	{
		SetBackgroundColor(color.r, color.g, color.b);
	}
} // namespace infrastructure::graphics
