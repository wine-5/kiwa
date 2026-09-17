#pragma once
#include "core/base/NonCopyable.h"
#include "core/interface/IInputProvider.h"
#include <array>
#include <cstddef>

namespace infrastructure::input
{
	/**
	 * @brief DxLib のキーボード・マウスを扱う IInputProvider の実装
	 *
	 * キー状態はフレーム頭に1度だけ読み取り、その内容を1フレーム保持する
	 */
	class InputProvider final : public core::iface::IInputProvider, private core::base::NonCopyable
	{
	  public:
		InputProvider() = default;

		void captureFrameInput() override;

		void updatePreviousState() override;

		[[nodiscard]] bool isKeyDown(core::input::KeyCode keyCode) const override;

		[[nodiscard]] bool isKeyPressed(core::input::KeyCode keyCode) const override;

		[[nodiscard]] bool isKeyReleased(core::input::KeyCode keyCode) const override;

		[[nodiscard]] core::utility::Vector2 getMousePosition() const override;

		[[nodiscard]] bool isMouseLeftDown() const override;

		[[nodiscard]] bool isMouseLeftPressed() const override;

	  private:
		/// @brief 扱うキーの数
		static constexpr std::size_t KEY_COUNT{ static_cast<std::size_t>(core::input::KeyCode::Count) };

		/// @brief このフレームのキー状態
		std::array<bool, KEY_COUNT> m_currentKeys{};

		/// @brief 前フレームのキー状態
		std::array<bool, KEY_COUNT> m_previousKeys{};

		/// @brief このフレームのマウス左ボタンの状態
		bool m_currentMouseLeft{ false };

		/// @brief 前フレームのマウス左ボタンの状態
		bool m_previousMouseLeft{ false };
	};
} // namespace infrastructure::input
