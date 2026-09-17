#include "infrastructure/input/InputProvider.h"
#include "DxLib.h"

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
		}

		m_currentMouseLeft = (GetMouseInput() & MOUSE_INPUT_LEFT) != 0;
	}

	void InputProvider::updatePreviousState()
	{
		m_previousKeys = m_currentKeys;
		m_previousMouseLeft = m_currentMouseLeft;
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

	core::utility::Vector2 InputProvider::getMousePosition() const
	{
		int x{ 0 };
		int y{ 0 };
		GetMousePoint(&x, &y);
		return core::utility::Vector2{ static_cast<float>(x), static_cast<float>(y) };
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
