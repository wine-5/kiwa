#pragma once
#include "core/base/NonCopyable.h"
#include "core/interface/IInputProvider.h"
#include <array>
#include <cstddef>

namespace infrastructure::input
{
	/**
	 * @brief 画面に触れる操作を、いまのゲームの操作へ読み替える実装
	 *
	 * 携帯の端末にはキーボードもマウスも無い。DXライブラリでも、触った指は
	 * マウスの関数では取れず、触れた指を数える関数から取ることになっている。
	 * そこでここが指の位置と数を読み、ゲームが知っている言葉へ直す。
	 *
	 * - 指の位置 … マウスの位置として渡す（並びを押して選ぶところがそのまま動く）
	 * - 触れている … マウスの左ボタンとして渡す
	 * - 触れている … 注ぐキー（スペース）としても渡す
	 *
	 * 注ぐのは二人とも同じキーなので、**画面のどこを触っても、いまの手番の人が注ぐ**
	 * ことになる。二人で一台を囲む遊び方も、一人で遊ぶ場合もこれで足りる
	 */
	class TouchInputProvider final : public core::iface::IInputProvider,
	                                 private core::base::NonCopyable
	{
	  public:
		TouchInputProvider() = default;

		void captureFrameInput() override;

		void updatePreviousState() override;

		[[nodiscard]] bool isKeyDown(core::input::KeyCode keyCode) const override;

		[[nodiscard]] bool isKeyPressed(core::input::KeyCode keyCode) const override;

		[[nodiscard]] bool isKeyReleased(core::input::KeyCode keyCode) const override;

		bool consumeKeyPress(core::input::KeyCode keyCode) override;

		void clearPendingPresses() override;

		[[nodiscard]] core::utility::Vector2 getMousePosition() const override;

		[[nodiscard]] bool isMouseMoved() const override;

		[[nodiscard]] bool isMouseLeftDown() const override;

		[[nodiscard]] bool isMouseLeftPressed() const override;

	  private:
		/// @brief キーの数
		static constexpr std::size_t KEY_COUNT{ static_cast<std::size_t>(
			core::input::KeyCode::Count) };

		/// @brief いまのフレームで押されているとみなすキー
		std::array<bool, KEY_COUNT> m_currentKeys{};

		/// @brief 1フレーム前に押されていたとみなすキー
		std::array<bool, KEY_COUNT> m_previousKeys{};

		/// @brief 押された瞬間を、まだ誰も受け取っていないか
		std::array<bool, KEY_COUNT> m_pendingPresses{};

		/// @brief いま触れている場所
		core::utility::Vector2 m_currentPosition{};

		/// @brief 1フレーム前に触れていた場所
		core::utility::Vector2 m_previousPosition{};

		/// @brief いま触れているか
		bool m_isTouching{ false };

		/// @brief 1フレーム前に触れていたか
		bool m_wasTouching{ false };
	};
} // namespace infrastructure::input
