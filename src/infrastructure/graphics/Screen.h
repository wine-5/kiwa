#pragma once
#include "core/base/NonCopyable.h"
#include "core/interface/IScreen.h"

namespace infrastructure::graphics
{
	/**
	 * @brief DxLib のウィンドウを扱う IScreen の実装
	 */
	class Screen final : public core::iface::IScreen, private core::base::NonCopyable
	{
	  public:
		/**
		 * @brief Screen のコンストラクタ
		 * @param width 画面の幅（ピクセル）
		 * @param height 画面の高さ（ピクセル）
		 */
		Screen(int width, int height) noexcept;

		[[nodiscard]] int getWidth() const noexcept override
		{
			return m_width;
		}

		[[nodiscard]] int getHeight() const noexcept override
		{
			return m_height;
		}

		void setBackgroundColor(const core::utility::Color& color) noexcept override;

	  private:
		int m_width{ 0 };
		int m_height{ 0 };
	};
} // namespace infrastructure::graphics
