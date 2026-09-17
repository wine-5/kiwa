#pragma once
#include "core/base/NonCopyable.h"
#include "core/interface/IRenderer.h"

namespace infrastructure::graphics
{
	/**
	 * @brief DxLib を使った IRenderer の実装
	 */
	class Renderer final : public core::iface::IRenderer, private core::base::NonCopyable
	{
	  public:
		Renderer() = default;

		void drawRect(const core::utility::Vector2& position, const core::utility::Vector2& size,
		              const core::utility::Color& color, bool isFilled = true) override;

		void drawCircle(const core::utility::Vector2& center, float radius, const core::utility::Color& color,
		                bool isFilled = true) override;

		void drawLine(const core::utility::Vector2& from, const core::utility::Vector2& to,
		              const core::utility::Color& color) override;

		void drawText(const core::utility::Vector2& position, const std::string& text,
		              const core::utility::Color& color) override;

		void drawTextCentered(const core::utility::Vector2& center, const std::string& text,
		                      const core::utility::Color& color) override;

		void drawTexture(int handle, const core::utility::Vector2& position) override;

		void drawTextureRotated(int handle, const core::utility::Vector2& center, float scale,
		                        float angleRadian) override;
	};
} // namespace infrastructure::graphics
