#include "game/scene/InGameScene.h"
#include "core/interface/IInputProvider.h"
#include <random>

namespace game::scene
{
	InGameScene::InGameScene(const SceneContext& context)
	    : m_context{ context },
	      m_view{ context.renderer3D, context.renderer,      context.camera,
		          context.modelRenderer, context.resource, context.screen },
	      m_presenter{ m_view, context.input, std::random_device{}() }
	{
	}

	void InGameScene::update(float deltaTime)
	{
		m_presenter.update(deltaTime);

		// Presenter が渡したあとに View の動き（液面の揺れなど）を進める
		m_view.advance(deltaTime);
	}

	void InGameScene::draw()
	{
		m_view.draw();
	}

	void InGameScene::drawOverlay()
	{
		m_view.drawOverlay();
	}
} // namespace game::scene
