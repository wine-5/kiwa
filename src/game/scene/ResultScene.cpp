#include "game/scene/ResultScene.h"
#include "core/interface/IInputProvider.h"
#include "core/interface/IRenderer.h"
#include "core/interface/IScreen.h"

namespace game::scene
{
	ResultScene::ResultScene(const SceneContext& context) : m_context{ context }
	{
	}

	void ResultScene::update([[maybe_unused]] float deltaTime)
	{
		if (m_context.input.isKeyPressed(core::input::KeyCode::Enter))
			m_context.changeScene(SceneType::Title);
	}

	void ResultScene::draw()
	{
		const float centerX{ m_context.screen.getWidth() * 0.5f };
		const float centerY{ m_context.screen.getHeight() * 0.5f };

		m_context.renderer.drawTextCentered(core::utility::Vector2{ centerX, centerY - 20.0f }, "RESULT",
		                                    core::utility::colors::WHITE);
		m_context.renderer.drawTextCentered(core::utility::Vector2{ centerX, centerY + 20.0f },
		                                    "Enter でタイトルへ", core::utility::colors::GRAY);
	}
} // namespace game::scene
