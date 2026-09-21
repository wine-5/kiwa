#include "game/scene/ResultScene.h"
#include "core/interface/IInputProvider.h"
#include "core/interface/IRenderer.h"
#include "core/interface/IAudioPlayer.h"
#include "core/interface/IResourceManager.h"
#include "core/interface/IScreen.h"
#include "core/utility/Easing.h"
#include "game/constant/Fonts.h"
#include "game/constant/Palette.h"
#include "game/constant/Sounds.h"
#include "game/constant/UiTextures.h"
#include "game/model/Npc.h"
#include <algorithm>

namespace
{
	using core::utility::Easing;
	using core::utility::Vector2;
	namespace font = game::constant::font;
	namespace palette = game::constant::palette;

	/// @brief 勝者の名を書く書体の大きさ
	constexpr int WINNER_FONT_SIZE{ 84 };

	/// @brief 掛軸が下り切るまでの時間（秒）
	constexpr float SCROLL_TIME{ 0.9f };

	/// @brief 名が浮かび上がるまでの間（秒）
	constexpr float NAME_DELAY{ 0.7f };

	/// @brief 名が浮かび上がる時間（秒）
	constexpr float NAME_TIME{ 0.7f };

	/// @brief 落款が押されるまでの間（秒）
	constexpr float SEAL_DELAY{ 1.6f };

	/// @brief 落款が押される時間（秒）
	constexpr float SEAL_TIME{ 0.22f };

	/// @brief 押す前に落款が浮いている高さの倍率
	constexpr float SEAL_LIFT{ 1.7f };

	/// @brief 掛軸の幅
	constexpr float SCROLL_WIDTH{ 720.0f };

	/// @brief 掛軸を置く高さ（画面の高さに対する割合）
	constexpr float SCROLL_Y{ 0.48f };

	/// @brief 落款の大きさ
	constexpr float SEAL_SIZE{ 104.0f };

	/// @brief リザルトの曲の大きさ
	constexpr float BGM_VOLUME{ 0.5f };

	/// @brief 案内が出るまでの間（秒）
	constexpr float PROMPT_DELAY{ 2.1f };

	/// @brief 締めの音が鳴るまでの間（秒）。落款が押し切ったすぐあと
	constexpr float MATCH_WIN_DELAY{ SEAL_DELAY + SEAL_TIME + 0.1f };

	/**
	 * @brief 背景へ溶かして濃さを変える
	 * @param color もとの色
	 * @param strength 濃さ（0.0で見えない、1.0でそのまま）
	 * @return 溶かした色
	 */
	core::utility::Color faded(const core::utility::Color& color, float strength)
	{
		return core::utility::mixed(palette::BACKGROUND, color, std::clamp(strength, 0.0f, 1.0f));
	}
} // namespace

namespace game::scene
{
	ResultScene::~ResultScene()
	{
		m_context.audio.stopSound(m_bgm);
	}

	ResultScene::ResultScene(const SceneContext& context)
	    : m_context{ context }, m_backdrop{ context.renderer3D, context.renderer, context.camera,
		                                    context.modelRenderer, context.resource, context.screen }
	{
		m_scrollTexture = m_context.resource.loadTexture(constant::ui::RESULT_SCROLL);
		m_sealTexture = m_context.resource.loadTexture(constant::ui::SEAL_VICTORY);
		namespace sound = game::constant::sound;
		m_winSound = m_context.resource.loadSound(sound::SE_ROUND_WIN);
		m_scrollSound = m_context.resource.loadSound(sound::SE_SCROLL_OPEN);
		m_sealSound = m_context.resource.loadSound(sound::SE_SEAL_STAMP);
		m_matchWinSound = m_context.resource.loadSound(sound::SE_MATCH_WIN);
		m_bgm = m_context.resource.loadSound(sound::BGM_RESULT_MATCH);

		// 短い締めをひとつ置いてから、余韻の曲を流す
		m_context.audio.playSe(m_context.resource.loadSound(sound::BGM_RESULT_ROUND));
		m_context.audio.playSe(m_scrollSound);

		m_context.audio.setVolume(m_bgm, BGM_VOLUME);
		m_context.audio.playLoop(m_bgm);

		m_winnerFont = m_context.resource.loadFont(font::HEADING_FAMILY, WINNER_FONT_SIZE);
		m_bodyFont = m_context.resource.loadFont(font::BODY_FAMILY, font::BODY_SIZE);
	}

	std::string ResultScene::buildWinnerName() const
	{
		// 二の手を NPC が打っていたなら、その呼び名で称える
		if (m_context.setup.winner == model::Player::Two)
			return model::npcOf(m_context.setup.npc).name;

		return "一の手";
	}

	void ResultScene::update(float deltaTime)
	{
		const float previous{ m_elapsedTime };
		m_elapsedTime += deltaTime;
		m_backdrop.update(deltaTime);

		// 掛軸が下り切って名が浮かぶところで、勝ちの音を一度だけ
		if (previous < NAME_DELAY && m_elapsedTime >= NAME_DELAY)
			m_context.audio.playSe(m_winSound);

		// 落款が紙を打つ瞬間と、そのあとの締め
		if (previous < SEAL_DELAY && m_elapsedTime >= SEAL_DELAY)
			m_context.audio.playSe(m_sealSound);

		if (previous < MATCH_WIN_DELAY && m_elapsedTime >= MATCH_WIN_DELAY)
			m_context.audio.playSe(m_matchWinSound);

		// 押し切るまでは受け付けない。見せ切ってから次へ進ませる
		if (m_elapsedTime < PROMPT_DELAY)
		{
			m_context.input.clearPendingPresses();
			return;
		}

		if (m_context.input.consumeKeyPress(core::input::KeyCode::Enter) ||
		    m_context.input.consumeKeyPress(core::input::KeyCode::Space) ||
		    m_context.input.isMouseLeftPressed())
			m_context.changeScene(SceneType::Title);
	}

	void ResultScene::draw()
	{
		m_backdrop.draw();
	}

	void ResultScene::drawOverlay()
	{
		m_backdrop.drawOverlay();

		const float width{ static_cast<float>(m_context.screen.getWidth()) };
		const float height{ static_cast<float>(m_context.screen.getHeight()) };
		const float centerX{ width * 0.5f };

		const Vector2 source{ m_context.renderer.getTextureSize(m_scrollTexture) };
		if (source.x <= 0.0f)
			return;

		// 掛軸は上から下りてくる。下り切ってから名が浮かぶ
		const float drop{ Easing::easeOut(m_elapsedTime / SCROLL_TIME) };
		const float scrollHeight{ SCROLL_WIDTH * (source.y / source.x) };
		const float centerY{ height * SCROLL_Y };
		const float top{ centerY - scrollHeight * 0.5f - (1.0f - drop) * height * 0.6f };

		m_context.renderer.drawTextureStretched(m_scrollTexture,
		                                        Vector2{ centerX - SCROLL_WIDTH * 0.5f, top },
		                                        Vector2{ SCROLL_WIDTH, scrollHeight }, drop);

		const float nameStrength{ Easing::easeOut((m_elapsedTime - NAME_DELAY) / NAME_TIME) };
		if (nameStrength > 0.0f)
		{
			m_context.renderer.setFont(m_winnerFont);
			m_context.renderer.drawTextCentered(Vector2{ centerX, top + scrollHeight * 0.44f },
			                                    buildWinnerName() + " の勝ち",
			                                    faded(palette::INK, nameStrength));
		}

		// 落款は上から降りてきて、最後に押し当てられる
		const float stamp{ Easing::easeIn((m_elapsedTime - SEAL_DELAY) / SEAL_TIME) };
		if (stamp > 0.0f)
		{
			const float lift{ SEAL_SIZE * SEAL_LIFT * (1.0f - stamp) };
			const float size{ SEAL_SIZE * (1.0f + (SEAL_LIFT - 1.0f) * (1.0f - stamp)) };

			m_context.renderer.drawTextureStretched(
			    m_sealTexture,
			    Vector2{ centerX + SCROLL_WIDTH * 0.27f - size * 0.5f,
				         top + scrollHeight * 0.72f - size * 0.5f - lift },
			    Vector2{ size, size }, std::min(1.0f, stamp * 1.6f));
		}

		if (m_elapsedTime < PROMPT_DELAY)
			return;

		m_context.renderer.setFont(m_bodyFont);
		m_context.renderer.drawTextCentered(Vector2{ centerX, height - 72.0f },
		                                    "Enter でタイトルへ", palette::TEXT_SUB);
	}
} // namespace game::scene
