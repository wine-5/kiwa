#include "infrastructure/debug/ScriptedInput.h"

namespace infrastructure::debug
{
	void ScriptedInput::addHold(core::input::KeyCode keyCode, int fromFrame, int toFrame)
	{
		m_steps.push_back(Step{ keyCode, fromFrame, toFrame });
	}

	void ScriptedInput::captureFrameInput()
	{
		m_inner.captureFrameInput();
		++m_frame;

		m_scriptedKeys.fill(false);
		for (const Step& step : m_steps)
		{
			if (m_frame >= step.fromFrame && m_frame < step.toFrame)
				m_scriptedKeys[static_cast<std::size_t>(step.keyCode)] = true;
		}

		// 押された瞬間は、誰かが受け取るまで覚えておく（本物と同じ扱いにする）
		for (std::size_t i{ 0 }; i < KEY_COUNT; ++i)
		{
			if (m_scriptedKeys[i] && !m_previousScriptedKeys[i])
				m_pendingPresses[i] = true;
		}
	}

	void ScriptedInput::updatePreviousState()
	{
		m_inner.updatePreviousState();
		m_previousScriptedKeys = m_scriptedKeys;
	}

	bool ScriptedInput::isKeyDown(core::input::KeyCode keyCode) const
	{
		return m_inner.isKeyDown(keyCode) || m_scriptedKeys[static_cast<std::size_t>(keyCode)];
	}

	bool ScriptedInput::isKeyPressed(core::input::KeyCode keyCode) const
	{
		const std::size_t index{ static_cast<std::size_t>(keyCode) };
		const bool scripted{ m_scriptedKeys[index] && !m_previousScriptedKeys[index] };
		return m_inner.isKeyPressed(keyCode) || scripted;
	}

	bool ScriptedInput::consumeKeyPress(core::input::KeyCode keyCode)
	{
		const std::size_t index{ static_cast<std::size_t>(keyCode) };
		const bool scripted{ m_pendingPresses[index] };
		m_pendingPresses[index] = false;

		// 台本と本物のどちらで押されていても、一度ぶんとして扱う
		return m_inner.consumeKeyPress(keyCode) || scripted;
	}

	void ScriptedInput::clearPendingPresses()
	{
		m_inner.clearPendingPresses();
		m_pendingPresses.fill(false);
	}

	bool ScriptedInput::isKeyReleased(core::input::KeyCode keyCode) const
	{
		return m_inner.isKeyReleased(keyCode);
	}

	core::utility::Vector2 ScriptedInput::getMousePosition() const
	{
		return m_inner.getMousePosition();
	}

	bool ScriptedInput::isMouseLeftDown() const
	{
		return m_inner.isMouseLeftDown();
	}

	bool ScriptedInput::isMouseLeftPressed() const
	{
		return m_inner.isMouseLeftPressed();
	}
} // namespace infrastructure::debug
