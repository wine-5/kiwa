#include "infrastructure/input/TouchInputProvider.h"
#include "DxLib.h"
#include <cmath>

namespace
{
	/// @brief 触れた指を、動いたとみなすまでの距離
	constexpr float MOVE_THRESHOLD{ 1.5f };

	/**
	 * @brief その操作を、触れることで代わりにできるかを返す
	 * @param keyCode 調べるキー
	 * @return 代われるなら true
	 */
	constexpr bool isTouchKey(core::input::KeyCode keyCode) noexcept
	{
		// 注ぐ（スペース）と、決める（Enter）を触れることで代える。
		// 選ぶのは並びを直接押せるので、十字キーは要らない
		return keyCode == core::input::KeyCode::Space || keyCode == core::input::KeyCode::Enter;
	}
} // namespace

namespace infrastructure::input
{
	void TouchInputProvider::captureFrameInput()
	{
		m_wasTouching = m_isTouching;
		m_previousPosition = m_currentPosition;

		// 触れている指のうち、最初の一本だけを見る。
		// この遊びは「押しているか、離したか」しか使わないため
		const int count{ GetTouchInputNum() };
		m_isTouching = count > 0;

		if (m_isTouching)
		{
			int x{ 0 };
			int y{ 0 };
			GetTouchInput(0, &x, &y, nullptr, nullptr);
			m_currentPosition =
			    core::utility::Vector2{ static_cast<float>(x), static_cast<float>(y) };
		}

		for (std::size_t i{ 0 }; i < KEY_COUNT; ++i)
		{
			m_previousKeys[i] = m_currentKeys[i];
			m_currentKeys[i] = m_isTouching && isTouchKey(static_cast<core::input::KeyCode>(i));

			// 押した瞬間は、誰かが受け取るまで覚えておく。
			// 更新は固定の刻みで回るため、押した瞬間を取りこぼすことがある
			if (m_currentKeys[i] && !m_previousKeys[i])
				m_pendingPresses[i] = true;
		}
	}

	void TouchInputProvider::updatePreviousState()
	{
		// 状態は captureFrameInput で進めているので、ここでは何もしない
	}

	bool TouchInputProvider::isKeyDown(core::input::KeyCode keyCode) const
	{
		return m_currentKeys[static_cast<std::size_t>(keyCode)];
	}

	bool TouchInputProvider::isKeyPressed(core::input::KeyCode keyCode) const
	{
		const auto index{ static_cast<std::size_t>(keyCode) };
		return m_currentKeys[index] && !m_previousKeys[index];
	}

	bool TouchInputProvider::isKeyReleased(core::input::KeyCode keyCode) const
	{
		const auto index{ static_cast<std::size_t>(keyCode) };
		return !m_currentKeys[index] && m_previousKeys[index];
	}

	bool TouchInputProvider::consumeKeyPress(core::input::KeyCode keyCode)
	{
		const auto index{ static_cast<std::size_t>(keyCode) };
		if (!m_pendingPresses[index])
			return false;

		m_pendingPresses[index] = false;
		return true;
	}

	void TouchInputProvider::clearPendingPresses()
	{
		m_pendingPresses.fill(false);
	}

	core::utility::Vector2 TouchInputProvider::getMousePosition() const
	{
		return m_currentPosition;
	}

	bool TouchInputProvider::isMouseMoved() const
	{
		// 触れた指は必ずどこかを指しているので、触れた瞬間も「動いた」とみなす。
		// そうしないと、指の下にある項目が選ばれない
		if (m_isTouching && !m_wasTouching)
			return true;

		return std::abs(m_currentPosition.x - m_previousPosition.x) > MOVE_THRESHOLD ||
		       std::abs(m_currentPosition.y - m_previousPosition.y) > MOVE_THRESHOLD;
	}

	bool TouchInputProvider::isMouseLeftDown() const
	{
		return m_isTouching;
	}

	bool TouchInputProvider::isMouseLeftPressed() const
	{
		return m_isTouching && !m_wasTouching;
	}
} // namespace infrastructure::input
