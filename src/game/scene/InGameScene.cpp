#include "game/scene/InGameScene.h"
#include "core/interface/IInputProvider.h"
#include "core/interface/IAudioPlayer.h"
#include "core/interface/IResourceManager.h"
#include "game/constant/Sounds.h"
#include <random>

namespace
{
	/// @brief 茶室の間の大きさ
	constexpr float AMBIENCE_VOLUME{ 0.35f };

	/// @brief 対局中の曲の大きさ
	constexpr float BGM_VOLUME{ 0.5f };
} // namespace

namespace game::scene
{
	InGameScene::InGameScene(const SceneContext& context)
	    : m_context{ context },
	      m_view{ context.renderer3D,   context.renderer, context.camera, context.modelRenderer,
		          context.resource,      context.audio,    context.screen },
	      m_presenter{ m_view, context.input, context.setup, std::random_device{}() }
	{
		namespace sound = game::constant::sound;
		m_ambience = m_context.resource.loadSound(sound::AMBIENCE_TEAROOM);
		m_bgm = m_context.resource.loadSound(sound::BGM_DUEL_CALM);

		// 間は薄く、曲はその下。注ぐ音の邪魔をしない大きさに置く
		m_context.audio.setVolume(m_ambience, AMBIENCE_VOLUME);
		m_context.audio.setVolume(m_bgm, BGM_VOLUME);
		m_context.audio.playLoop(m_ambience);
		m_context.audio.playLoop(m_bgm);
	}

	InGameScene::~InGameScene()
	{
		// 対局を離れたら曲は止める。間（環境音）はそのまま鳴らし続ける
		m_context.audio.stopSound(m_bgm);
	}

	void InGameScene::update(float deltaTime)
	{
		m_presenter.update(deltaTime);

		// 先取数に届いたら決着。結末は設定に書き戻されているので、あとはリザルトが読む
		if (m_presenter.isMatchDecided())
			m_context.changeScene(SceneType::Result);

		// Presenter が渡したあとに View の動き（液面の揺れなど）を進める
		m_view.update(deltaTime);
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
