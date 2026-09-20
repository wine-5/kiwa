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

	void Renderer::setFont(int fontHandle)
	{
		m_fontHandle = fontHandle;
	}

	void Renderer::drawText(const core::utility::Vector2& position, const std::string& text,
	                        const core::utility::Color& color)
	{
		if (m_fontHandle < 0)
		{
			DrawString(static_cast<int>(position.x), static_cast<int>(position.y), text.c_str(),
			           toDxColor(color));
			return;
		}

		DrawStringToHandle(static_cast<int>(position.x), static_cast<int>(position.y), text.c_str(),
		                   toDxColor(color), m_fontHandle);
	}

	void Renderer::drawTextCentered(const core::utility::Vector2& center, const std::string& text,
	                                const core::utility::Color& color)
	{
		const int length{ static_cast<int>(text.length()) };

		if (m_fontHandle < 0)
		{
			const int width{ GetDrawStringWidth(text.c_str(), length) };
			const int height{ GetFontSize() };
			DrawString(static_cast<int>(center.x) - width / 2, static_cast<int>(center.y) - height / 2,
			           text.c_str(), toDxColor(color));
			return;
		}

		const int width{ GetDrawStringWidthToHandle(text.c_str(), length, m_fontHandle) };
		const int height{ GetFontSizeToHandle(m_fontHandle) };
		DrawStringToHandle(static_cast<int>(center.x) - width / 2,
		                   static_cast<int>(center.y) - height / 2, text.c_str(), toDxColor(color),
		                   m_fontHandle);
	}

	void Renderer::drawTexture(int handle, const core::utility::Vector2& position)
	{
		DrawGraph(static_cast<int>(position.x), static_cast<int>(position.y), handle, TRUE);
	}

	void Renderer::drawTextureStretched(int handle, const core::utility::Vector2& position,
	                                    const core::utility::Vector2& size, float alpha)
	{
		if (handle < 0)
			return;

		const int value{ static_cast<int>(alpha * 255.0f) };
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, value < 0 ? 0 : (value > 255 ? 255 : value));
		DrawExtendGraph(static_cast<int>(position.x), static_cast<int>(position.y),
		                static_cast<int>(position.x + size.x), static_cast<int>(position.y + size.y),
		                handle, TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	void Renderer::drawTextureRotated(int handle, const core::utility::Vector2& center, float scale,
	                                  float angleRadian)
	{
		DrawRotaGraph(static_cast<int>(center.x), static_cast<int>(center.y), static_cast<double>(scale),
		              static_cast<double>(angleRadian), handle, TRUE);
	}
} // namespace infrastructure::graphics
