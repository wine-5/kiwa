#include "game/scene/InGameScene.h"
#include "core/interface/IInputProvider.h"
#include "core/interface/IRenderer.h"
#include "core/interface/IScreen.h"

namespace
{
	/// @brief 動作確認用の四角の1辺の長さ（ピクセル）
	constexpr float BOX_SIZE{ 48.0f };

	/// @brief 動作確認用の四角の移動速度（ピクセル毎秒）
	constexpr float MOVE_SPEED{ 320.0f };
} // namespace

namespace game::scene
{
	InGameScene::InGameScene(const SceneContext& context) : m_context{ context }
	{
		m_position = core::utility::Vector2{ m_context.screen.getWidth() * 0.5f,
			                                 m_context.screen.getHeight() * 0.5f };
	}

	void InGameScene::update(float deltaTime)
	{
		core::utility::Vector2 direction{};
		if (m_context.input.isKeyDown(core::input::KeyCode::A))
			direction.x -= 1.0f;
		if (m_context.input.isKeyDown(core::input::KeyCode::D))
			direction.x += 1.0f;
		if (m_context.input.isKeyDown(core::input::KeyCode::W))
			direction.y -= 1.0f;
		if (m_context.input.isKeyDown(core::input::KeyCode::S))
			direction.y += 1.0f;

		// 斜めが速くならないよう長さを1に揃えてから速度を掛ける
		m_position += direction.normalized() * (MOVE_SPEED * deltaTime);

		if (m_context.input.isKeyPressed(core::input::KeyCode::Enter))
			m_context.changeScene(SceneType::Result);
	}

	void InGameScene::draw()
	{
		m_context.renderer.drawRect(
		    core::utility::Vector2{ m_position.x - BOX_SIZE * 0.5f, m_position.y - BOX_SIZE * 0.5f },
		    core::utility::Vector2{ BOX_SIZE, BOX_SIZE }, core::utility::colors::BLUE);

		m_context.renderer.drawText(core::utility::Vector2{ 16.0f, 16.0f }, "WASD で移動 / Enter でリザルトへ",
		                            core::utility::colors::GRAY);
	}
} // namespace game::scene
