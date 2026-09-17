#include "game/scene/SceneManager.h"
#include "game/scene/SceneFactory.h"

namespace game::scene
{
	SceneManager::SceneManager(core::iface::IRenderer& renderer, core::iface::IRenderer3D& renderer3D,
	                           core::iface::ICamera& camera, core::iface::IInputProvider& input,
	                           core::iface::IResourceManager& resource, core::iface::IScreen& screen)
	    : m_context{ renderer,  renderer3D, camera,
		             input,     resource,   screen,
		             [this](SceneType sceneType) { changeScene(sceneType); } }
	{
	}

	void SceneManager::start(SceneType sceneType)
	{
		m_currentSceneType = sceneType;
		m_currentScene = SceneFactory::create(sceneType, m_context);
		m_currentScene->onEnter();
	}

	void SceneManager::changeScene(SceneType sceneType)
	{
		m_pendingSceneType = sceneType;
	}

	void SceneManager::update(float deltaTime)
	{
		applyPendingChange();

		if (m_currentScene)
			m_currentScene->update(deltaTime);
	}

	void SceneManager::draw()
	{
		if (m_currentScene)
			m_currentScene->draw();
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
