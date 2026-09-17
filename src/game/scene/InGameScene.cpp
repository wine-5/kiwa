#include "game/scene/InGameScene.h"
#include "core/interface/IInputProvider.h"
#include <random>

namespace game::scene
{
	InGameScene::InGameScene(const SceneContext& context)
	    : m_context{ context }, m_view{ context.renderer3D, context.renderer, context.camera, context.screen },
	      m_presenter{ m_view, context.input, std::random_device{}() }
	{
	}

	void InGameScene::update(float deltaTime)
	{
		m_presenter.update(deltaTime);
	}

	void InGameScene::draw()
	{
		m_view.draw();
	}
} // namespace game::scene
