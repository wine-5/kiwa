#include "infrastructure/input/InputProvider.h"
#include "DxLib.h"
#include <cmath>

namespace
{
	/**
	 * @brief 独自のキーコードを DxLib のキーコードへ変換する
	 * @param keyCode 独自のキーコード
	 * @return DxLib のキーコード（対応が無ければ -1）
	 */
	int toDxKeyCode(core::input::KeyCode keyCode)
	{
		using core::input::KeyCode;
		switch (keyCode)
		{
		case KeyCode::W:
			return KEY_INPUT_W;
		case KeyCode::A:
			return KEY_INPUT_A;
		case KeyCode::S:
			return KEY_INPUT_S;
		case KeyCode::D:
			return KEY_INPUT_D;
		case KeyCode::Up:
			return KEY_INPUT_UP;
		case KeyCode::Down:
			return KEY_INPUT_DOWN;
		case KeyCode::Left:
			return KEY_INPUT_LEFT;
		case KeyCode::Right:
			return KEY_INPUT_RIGHT;
		case KeyCode::Space:
			return KEY_INPUT_SPACE;
		case KeyCode::Enter:
			return KEY_INPUT_RETURN;
		case KeyCode::Escape:
			return KEY_INPUT_ESCAPE;
		case KeyCode::Shift:
			return KEY_INPUT_LSHIFT;
		case KeyCode::R:
			return KEY_INPUT_R;
		case KeyCode::Z:
			return KEY_INPUT_Z;
		case KeyCode::X:
			return KEY_INPUT_X;
		case KeyCode::F1:
			return KEY_INPUT_F1;
		default:
			return -1;
		}
	}
} // namespace

namespace infrastructure::input
{
	void InputProvider::captureFrameInput()
	{
		// GetHitKeyStateAll は全キーの状態を一度に取れるため、キーごとに問い合わせるより軽い
		char keyStates[256]{};
		GetHitKeyStateAll(keyStates);

		for (std::size_t i{ 0 }; i < KEY_COUNT; ++i)
		{
			const int dxKeyCode{ toDxKeyCode(static_cast<core::input::KeyCode>(i)) };
			m_currentKeys[i] = dxKeyCode >= 0 && keyStates[dxKeyCode] != 0;

			// 押された瞬間は、誰かが受け取るまで覚えておく
			if (m_currentKeys[i] && !m_previousKeys[i])
				m_pendingPresses[i] = true;
		}

		int mouseX{ 0 };
		int mouseY{ 0 };
		GetMousePoint(&mouseX, &mouseY);
		m_currentMousePosition =
		    core::utility::Vector2{ static_cast<float>(mouseX), static_cast<float>(mouseY) };

		m_currentMouseLeft = (GetMouseInput() & MOUSE_INPUT_LEFT) != 0;
	}

	void InputProvider::updatePreviousState()
	{
		m_previousKeys = m_currentKeys;
		m_previousMouseLeft = m_currentMouseLeft;
		m_previousMousePosition = m_currentMousePosition;
	}

	bool InputProvider::isKeyDown(core::input::KeyCode keyCode) const
	{
		return m_currentKeys[static_cast<std::size_t>(keyCode)];
	}

	bool InputProvider::isKeyPressed(core::input::KeyCode keyCode) const
	{
		const std::size_t index{ static_cast<std::size_t>(keyCode) };
		return m_currentKeys[index] && !m_previousKeys[index];
	}

	bool InputProvider::isKeyReleased(core::input::KeyCode keyCode) const
	{
		const std::size_t index{ static_cast<std::size_t>(keyCode) };
		return !m_currentKeys[index] && m_previousKeys[index];
	}

	bool InputProvider::consumeKeyPress(core::input::KeyCode keyCode)
	{
		const std::size_t index{ static_cast<std::size_t>(keyCode) };
		if (!m_pendingPresses[index])
			return false;

		m_pendingPresses[index] = false;
		return true;
	}

	void InputProvider::clearPendingPresses()
	{
		m_pendingPresses.fill(false);
	}

	core::utility::Vector2 InputProvider::getMousePosition() const
	{
		return m_currentMousePosition;
	}

	bool InputProvider::isMouseMoved() const
	{
		// 手が触れていない微動を拾わないよう、わずかな差は動いていないものとして扱う
		constexpr float THRESHOLD{ 1.5f };
		return std::abs(m_currentMousePosition.x - m_previousMousePosition.x) > THRESHOLD ||
		       std::abs(m_currentMousePosition.y - m_previousMousePosition.y) > THRESHOLD;
	}

	bool InputProvider::isMouseLeftDown() const
	{
		return m_currentMouseLeft;
	}

	bool InputProvider::isMouseLeftPressed() const
	{
		return m_currentMouseLeft && !m_previousMouseLeft;
	}
} // namespace infrastructure::input
