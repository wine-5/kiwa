#include "game/scene/SceneManager.h"
#include "game/scene/SceneFactory.h"
#include "core/interface/IResourceManager.h"
#include "game/constant/Sounds.h"

namespace game::scene
{
	SceneManager::SceneManager(core::iface::IRenderer& renderer, core::iface::IRenderer3D& renderer3D,
	                           core::iface::ICamera& camera, core::iface::IModelRenderer& modelRenderer,
	                           core::iface::IInputProvider& input, core::iface::IResourceManager& resource,
	                           core::iface::IScreen& screen, model::MatchSetup& setup,
	                           std::function<void()> quitGame)
	    : m_context{ renderer,
		             renderer3D,
		             camera,
		             modelRenderer,
		             input,
		             resource,
		             screen,
		             setup,
		             [this](SceneType sceneType) { changeScene(sceneType); },
		             std::move(quitGame) }
	{
	}

	void SceneManager::start(SceneType sceneType)
	{
		m_transition.load(m_context.resource);
		m_sceneChangeSound = m_context.resource.loadSound(constant::sound::SE_SCENE_CHANGE);

		m_currentSceneType = sceneType;
		m_currentScene = SceneFactory::create(sceneType, m_context);
		m_currentScene->onEnter();
	}

	void SceneManager::changeScene(SceneType sceneType)
	{
		// すでに襖が動いているなら、行き先だけ差し替える
		if (!m_pendingSceneType.has_value())
		{
			m_transition.begin();
			m_context.resource.playSe(m_sceneChangeSound);
		}

		m_pendingSceneType = sceneType;
	}

	void SceneManager::update(float deltaTime)
	{
		m_transition.update(deltaTime);

		// 入れ替えるのは襖が閉じ切った瞬間。開いたときには次の場面になっている
		if (m_transition.consumeClosedMoment())
			applyPendingChange();

		if (m_currentScene)
			m_currentScene->update(deltaTime);
	}

	void SceneManager::draw()
	{
		if (m_currentScene)
			m_currentScene->draw();
	}

	void SceneManager::drawOverlay()
	{
		if (m_currentScene)
			m_currentScene->drawOverlay();

		// 襖はいちばん手前。場面が描いたものをすべて覆う
		m_transition.draw(m_context.renderer, m_context.screen);
	}

	void SceneManager::applyPendingChange()
	{
		if (!m_pendingSceneType.has_value())
			return;

		const SceneType nextSceneType{ *m_pendingSceneType };
		m_pendingSceneType.reset();

		if (m_currentScene)
			m_currentScene->onExit();

		m_currentSceneType = nextSceneType;
		m_currentScene = SceneFactory::create(nextSceneType, m_context);
		m_currentScene->onEnter();
	}
} // namespace game::scene
