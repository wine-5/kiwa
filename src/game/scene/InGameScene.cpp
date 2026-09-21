#include "game/scene/InGameScene.h"
#include "core/interface/IInputProvider.h"
#include "core/interface/IRenderer.h"
#include "core/interface/IAudioPlayer.h"
#include "core/interface/IResourceManager.h"
#include "core/interface/IScreen.h"
#include "game/constant/Fonts.h"
#include "game/constant/Palette.h"
#include "game/constant/Sounds.h"
#include "game/constant/UiTextures.h"
#include "game/view/Plate.h"
#include <cmath>
#include <random>

namespace
{
	using core::utility::Vector2;
	namespace font = game::constant::font;
	namespace palette = game::constant::palette;

	/// @brief 茶室の間の大きさ
	constexpr float AMBIENCE_VOLUME{ 0.35f };

	/// @brief 対局中の曲の大きさ
	constexpr float BGM_VOLUME{ 0.5f };

	/// @brief 休みへ入る札を置く場所（画面の左下から）
	constexpr Vector2 PAUSE_BUTTON_MARGIN{ 104.0f, 58.0f };

	/// @brief 休みへ入る札の大きさ
	constexpr Vector2 PAUSE_BUTTON_SIZE{ 148.0f, 52.0f };

	/// @brief 休みの並びを置く高さ（画面の高さに対する割合）
	constexpr float PAUSE_LIST_Y{ 0.52f };

	/// @brief 休みの並びの中身
	constexpr int PAUSE_RESUME{ 0 };
	constexpr int PAUSE_SOUND{ 1 };
	constexpr int PAUSE_TITLE{ 2 };
	constexpr int PAUSE_QUIT{ 3 };
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
		m_cursorSound = m_context.resource.loadSound(sound::SE_CURSOR);
		m_decideSound = m_context.resource.loadSound(sound::SE_DECIDE);
		m_backSound = m_context.resource.loadSound(sound::SE_BACK);

		m_plateTexture = m_context.resource.loadTexture(constant::ui::SCORE_PLATE);
		m_headingFont = m_context.resource.loadFont(font::HEADING_FAMILY, font::HEADING_SIZE);
		m_bodyFont = m_context.resource.loadFont(font::BODY_FAMILY, font::BODY_SIZE);

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

	view::ChoiceList::Content InGameScene::buildPauseContent() const
	{
		view::ChoiceList::Content content{};
		content.isActive = m_isPaused;
		content.selected = m_pauseIndex;
		content.centerY = PAUSE_LIST_Y;
		content.items = { "続ける", m_context.audio.isMuted() ? "音を出す" : "音を消す",
			              "タイトルへ", "終わる" };
		content.notes = { "", m_context.audio.isMuted() ? "いまは消している" : "", "", "" };
		return content;
	}

	void InGameScene::openPause()
	{
		m_isPaused = true;
		m_pauseIndex = PAUSE_RESUME;
		m_context.input.clearPendingPresses();
		m_context.audio.playSe(m_decideSound);
	}

	void InGameScene::closePause()
	{
		m_isPaused = false;

		// 休んでいる間に押していたぶんを持ち越すと、戻った瞬間に注がれてしまう
		m_context.input.clearPendingPresses();
		m_context.audio.playSe(m_backSound);
	}

	void InGameScene::decidePause()
	{
		switch (m_pauseIndex)
		{
		case PAUSE_SOUND:
			m_context.audio.setMuted(!m_context.audio.isMuted());
			m_context.audio.playSe(m_decideSound);
			break;

		case PAUSE_TITLE:
			m_context.audio.playSe(m_decideSound);
			m_context.changeScene(SceneType::Title);
			break;

		case PAUSE_QUIT:
			m_context.quitGame();
			break;

		case PAUSE_RESUME:
		default:
			closePause();
			break;
		}
	}

	void InGameScene::updatePause(float deltaTime)
	{
		// 押された印は必ず全部消費する（残すと戻ったときに持ち越されてしまう）
		const bool isNext{ m_context.input.consumeKeyPress(core::input::KeyCode::Down) };
		const bool isPrevious{ m_context.input.consumeKeyPress(core::input::KeyCode::Up) };
		const bool isDecided{ m_context.input.consumeKeyPress(core::input::KeyCode::Enter) ||
			                  m_context.input.consumeKeyPress(core::input::KeyCode::Space) };
		const bool isClosed{ m_context.input.consumeKeyPress(core::input::KeyCode::Escape) };

		const int count{ static_cast<int>(buildPauseContent().items.size()) };
		const int previousIndex{ m_pauseIndex };

		if (isNext)
			m_pauseIndex = (m_pauseIndex + 1) % count;

		if (isPrevious)
			m_pauseIndex = (m_pauseIndex + count - 1) % count;

		// カーソルを動かしたときだけ、そちらを指す
		const int hovered{ m_pauseMenu.hitTest(m_context.screen, m_context.input.getMousePosition()) };
		if (hovered >= 0 && m_context.input.isMouseMoved())
			m_pauseIndex = hovered;

		if (m_pauseIndex != previousIndex)
			m_context.audio.playSe(m_cursorSound);

		m_pauseMenu.update(deltaTime, buildPauseContent());

		if (isClosed)
		{
			closePause();
			return;
		}

		if (isDecided || (m_context.input.isMouseLeftPressed() && hovered >= 0))
			decidePause();
	}

	bool InGameScene::updatePauseButton()
	{
		const Vector2 center{ PAUSE_BUTTON_MARGIN.x,
			                  m_context.screen.getHeight() - PAUSE_BUTTON_MARGIN.y };
		const Vector2 mouse{ m_context.input.getMousePosition() };

		const bool isHovered{ std::abs(mouse.x - center.x) <= PAUSE_BUTTON_SIZE.x * 0.5f &&
			                  std::abs(mouse.y - center.y) <= PAUSE_BUTTON_SIZE.y * 0.5f };

		return isHovered && m_context.input.isMouseLeftPressed();
	}

	void InGameScene::update(float deltaTime)
	{
		if (m_isPaused)
		{
			updatePause(deltaTime);
			return;
		}

		// 休みは Escape でも、左下の札を押しても入れる
		if (m_context.input.consumeKeyPress(core::input::KeyCode::Escape) || updatePauseButton())
		{
			openPause();
			return;
		}

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

		const float width{ static_cast<float>(m_context.screen.getWidth()) };
		const float height{ static_cast<float>(m_context.screen.getHeight()) };

		if (m_isPaused)
		{
			// 対局を覆って、休みの並びだけを見せる（薄く透かす手が無いので塗りつぶす）
			m_context.renderer.drawRect(Vector2{ 0.0f, 0.0f }, Vector2{ width, height },
			                            palette::BACKGROUND);

			m_context.renderer.setFont(m_headingFont);
			m_context.renderer.drawTextCentered(Vector2{ width * 0.5f, height * 0.3f }, "休み",
			                                    palette::TEXT_PRIMARY);

			m_pauseMenu.draw(m_context.renderer, m_context.screen,
			                 view::ChoiceList::Resources{ m_headingFont, m_bodyFont,
				                                          font::HEADING_SIZE, font::BODY_SIZE });
			return;
		}

		// 休みへ入る札。押せることが分かるよう、隅に小さく置く
		const Vector2 center{ PAUSE_BUTTON_MARGIN.x, height - PAUSE_BUTTON_MARGIN.y };
		view::Plate::draw(m_context.renderer, m_plateTexture, center, PAUSE_BUTTON_SIZE.x, 0.8f);

		m_context.renderer.setFont(m_bodyFont);
		m_context.renderer.drawTextCentered(center, "休み（Esc）", palette::INK);
	}
} // namespace game::scene
