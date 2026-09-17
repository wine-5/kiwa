#include "game/scene/TitleScene.h"
#include "core/interface/IInputProvider.h"
#include "core/interface/IRenderer.h"
#include "core/interface/IScreen.h"
#include <cmath>

namespace
{
	/// @brief 「PRESS ENTER」が1回点滅する周期（秒）
	constexpr float BLINK_CYCLE{ 1.2f };
} // namespace

namespace game::scene
{
	TitleScene::TitleScene(const SceneContext& context) : m_context{ context }
	{
	}

	void TitleScene::update(float deltaTime)
	{
		m_elapsedTime += deltaTime;

		if (m_context.input.isKeyPressed(core::input::KeyCode::Enter))
			m_context.changeScene(SceneType::InGame);
	}

	void TitleScene::draw()
	{
		const float centerX{ m_context.screen.getWidth() * 0.5f };
		const float centerY{ m_context.screen.getHeight() * 0.5f };

		m_context.renderer.drawTextCentered(core::utility::Vector2{ centerX, centerY - 40.0f }, "かさ ゲームジャム",
		                                    core::utility::colors::WHITE);

		// 点滅させて「入力を待っている」ことを伝える
		if (std::fmod(m_elapsedTime, BLINK_CYCLE) < BLINK_CYCLE * 0.5f)
			m_context.renderer.drawTextCentered(core::utility::Vector2{ centerX, centerY + 20.0f },
			                                    "PRESS ENTER", core::utility::colors::GRAY);
	}
} // namespace game::scene
