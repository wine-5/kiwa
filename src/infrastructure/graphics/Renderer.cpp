#include "infrastructure/graphics/Renderer.h"
#include "DxLib.h"

namespace
{
	/**
	 * @brief Color を DxLib の色コードへ変換する
	 * @param color 変換する色
	 * @return DxLib の色コード
	 */
	unsigned int toDxColor(const core::utility::Color& color)
	{
		return GetColor(color.r, color.g, color.b);
	}
} // namespace

namespace infrastructure::graphics
{
	void Renderer::drawRect(const core::utility::Vector2& position, const core::utility::Vector2& size,
	                        const core::utility::Color& color, bool isFilled)
	{
		DrawBox(static_cast<int>(position.x), static_cast<int>(position.y),
		        static_cast<int>(position.x + size.x), static_cast<int>(position.y + size.y), toDxColor(color),
		        isFilled ? TRUE : FALSE);
	}

	void Renderer::drawCircle(const core::utility::Vector2& center, float radius,
	                          const core::utility::Color& color, bool isFilled)
	{
		DrawCircle(static_cast<int>(center.x), static_cast<int>(center.y), static_cast<int>(radius),
		           toDxColor(color), isFilled ? TRUE : FALSE);
	}

	void Renderer::drawLine(const core::utility::Vector2& from, const core::utility::Vector2& to,
	                        const core::utility::Color& color)
	{
		DrawLine(static_cast<int>(from.x), static_cast<int>(from.y), static_cast<int>(to.x),
		         static_cast<int>(to.y), toDxColor(color));
	}

	void Renderer::drawText(const core::utility::Vector2& position, const std::string& text,
	                        const core::utility::Color& color)
	{
		DrawString(static_cast<int>(position.x), static_cast<int>(position.y), text.c_str(), toDxColor(color));
	}

	void Renderer::drawTextCentered(const core::utility::Vector2& center, const std::string& text,
	                                const core::utility::Color& color)
	{
		const int width{ GetDrawStringWidth(text.c_str(), static_cast<int>(text.length())) };
		const int height{ GetFontSize() };
		DrawString(static_cast<int>(center.x) - width / 2, static_cast<int>(center.y) - height / 2,
		           text.c_str(), toDxColor(color));
	}

	void Renderer::drawTexture(int handle, const core::utility::Vector2& position)
	{
		DrawGraph(static_cast<int>(position.x), static_cast<int>(position.y), handle, TRUE);
	}

	void Renderer::drawTextureRotated(int handle, const core::utility::Vector2& center, float scale,
	                                  float angleRadian)
	{
		DrawRotaGraph(static_cast<int>(center.x), static_cast<int>(center.y), static_cast<double>(scale),
		              static_cast<double>(angleRadian), handle, TRUE);
	}
} // namespace infrastructure::graphics
