#include "game/scene/TitleScene.h"
#include "core/interface/IInputProvider.h"
#include "core/interface/IRenderer.h"
#include "core/interface/IResourceManager.h"
#include "core/interface/IScreen.h"
#include "game/constant/Fonts.h"
#include "game/constant/Palette.h"
#include "game/constant/Sounds.h"
#include "game/model/Npc.h"
#include <array>
#include <cmath>
#include <string>

namespace
{
	using core::utility::Vector2;
	namespace font = game::constant::font;
	namespace palette = game::constant::palette;

	/// @brief 題字の大きさ
	///
	/// 用意した題字の絵は墨で描いてあり、暗い画面では沈んで見えない。
	/// ここは同じ毛筆の書体で白く書く（絵を使うのは明るい地を敷いてからにする）
	constexpr int TITLE_FONT_SIZE{ 132 };

	/// @brief 題字を置く高さ（画面の高さに対する割合）
	constexpr float TITLE_Y{ 0.21f };

	/// @brief 副題を置く高さ（画面の高さに対する割合）
	constexpr float SUBTITLE_Y{ 0.33f };

	/// @brief 「二人で注ぎ合う」の並び順
	constexpr int MODE_TWO_PLAYERS{ 0 };

	/// @brief 「終わる」の並び順
	constexpr int MODE_QUIT{ 2 };

	/// @brief 茶室の間の大きさ
	constexpr float AMBIENCE_VOLUME{ 0.35f };

	/// @brief タイトルの曲の大きさ
	constexpr float BGM_VOLUME{ 0.55f };

	/// @brief 選ばせる並びを置く高さ（画面の高さに対する割合）
	///
	/// 後ろの茶室と重ならない、手前の畳が空いているところに置く
	constexpr float LIST_Y{ 0.74f };

	/// @brief 一人で打つときに選べる打ち手（弱い順に並べる）
	constexpr std::array<game::model::NpcType, 3> STRENGTHS{
		game::model::NpcType::Monk,
		game::model::NpcType::Performer,
		game::model::NpcType::Samurai,
	};

	/**
	 * @brief 強さのひと言を返す
	 * @param type 打ち手
	 * @return ひと言（「弱・慎重」など）
	 */
	std::string noteOf(game::model::NpcType type)
	{
		switch (type)
		{
		case game::model::NpcType::Monk:
			return "弱・慎重";
		case game::model::NpcType::Performer:
			return "中・気まぐれ";
		case game::model::NpcType::Samurai:
			return "強・豪胆";
		default:
			return "";
		}
	}
} // namespace

namespace game::scene
{
	TitleScene::TitleScene(const SceneContext& context)
	    : m_context{ context }, m_backdrop{ context.renderer3D, context.renderer, context.camera,
		                                    context.modelRenderer, context.resource, context.screen }
	{
		// 茶室の間は場面をまたいで鳴り続ける（切れると場が死ぬ）
		const int ambience{ m_context.resource.loadSound(game::constant::sound::AMBIENCE_TEAROOM) };
		m_context.resource.setVolume(ambience, AMBIENCE_VOLUME);
		m_context.resource.playLoop(ambience);

		namespace sound = game::constant::sound;
		m_bgm = m_context.resource.loadSound(sound::BGM_TITLE);
		m_cursorSound = m_context.resource.loadSound(sound::SE_CURSOR);
		m_decideSound = m_context.resource.loadSound(sound::SE_DECIDE);
		m_backSound = m_context.resource.loadSound(sound::SE_BACK);

		m_context.resource.setVolume(m_bgm, BGM_VOLUME);
		m_context.resource.playLoop(m_bgm);

		m_titleFont = m_context.resource.loadFont(font::HEADING_FAMILY, TITLE_FONT_SIZE);
		m_headingFont = m_context.resource.loadFont(font::HEADING_FAMILY, font::HEADING_SIZE);
		m_bodyFont = m_context.resource.loadFont(font::BODY_FAMILY, font::BODY_SIZE);
	}

	TitleScene::~TitleScene()
	{
		m_context.resource.stopSound(m_bgm);
	}

	view::ChoiceList::Content TitleScene::buildContent() const
	{
		view::ChoiceList::Content content{};
		content.isActive = true;
		content.selected = m_index;
		content.centerY = LIST_Y;

		if (m_step == Step::Mode)
		{
			content.items = { "二人で注ぎ合う", "一人で注ぎ合う", "終わる" };
			content.notes = { "向かい合って", "二の手は任せる", "" };
			return content;
		}

		for (const model::NpcType type : STRENGTHS)
		{
			content.items.emplace_back(model::npcOf(type).name);
			content.notes.emplace_back(noteOf(type));
		}

		content.items.emplace_back("戻る");
		content.notes.emplace_back("");
		return content;
	}

	void TitleScene::decide()
	{
		if (m_step == Step::Mode)
		{
			// 二人で打つなら、そのまま対局へ。一人なら続けて強さを選ばせる
			if (m_index == MODE_TWO_PLAYERS)
			{
				m_context.setup.npc = model::NpcType::None;
				m_context.changeScene(SceneType::InGame);
				return;
			}

			if (m_index == MODE_QUIT)
			{
				m_context.quitGame();
				return;
			}

			m_step = Step::Strength;
			m_index = 0;
			return;
		}

		if (m_index >= static_cast<int>(STRENGTHS.size()))
		{
			m_step = Step::Mode;
			m_index = 0;
			return;
		}

		m_context.setup.npc = STRENGTHS[m_index];
		m_context.changeScene(SceneType::InGame);
	}

	void TitleScene::update(float deltaTime)
	{
		m_backdrop.update(deltaTime);

		const view::ChoiceList::Content content{ buildContent() };
		const auto count{ static_cast<int>(content.items.size()) };

		// マウスを乗せたものへ指を移す。押せることが動きで分かる。
		// ただし動かしたときだけ。置いたままだと、キーで選んでも引き戻されてしまう
		const int hovered{ m_choices.hitTest(m_context.screen, m_context.input.getMousePosition()) };
		const int previousIndex{ m_index };

		if (hovered >= 0 && m_context.input.isMouseMoved())
			m_index = hovered;

		// 十字キーでも選べるようにしておく。
		// 押された印は必ず全部消費する（残すと次の場面へ持ち越されてしまう）
		const bool isNext{ m_context.input.consumeKeyPress(core::input::KeyCode::Down) };
		const bool isPrevious{ m_context.input.consumeKeyPress(core::input::KeyCode::Up) };
		const bool isSkip{ m_context.input.consumeKeyPress(core::input::KeyCode::Space) };
		const bool isBack{ m_context.input.consumeKeyPress(core::input::KeyCode::Left) };
		const bool isDecided{ m_context.input.consumeKeyPress(core::input::KeyCode::Enter) ||
			                  m_context.input.consumeKeyPress(core::input::KeyCode::Right) };

		if (isNext || isSkip)
			m_index = (m_index + 1) % count;

		if (isPrevious)
			m_index = (m_index + count - 1) % count;

		// 指しているものが変わったら、それが分かる音を鳴らす
		if (m_index != previousIndex)
			m_context.resource.playSe(m_cursorSound);

		// 左で一つ前の段へ戻る（強さを選んでいる途中で選び直せるように）
		if (isBack && m_step == Step::Strength)
		{
			m_context.resource.playSe(m_backSound);
			m_step = Step::Mode;
			m_index = 1;
			return;
		}

		const bool isClicked{ m_context.input.isMouseLeftPressed() && hovered >= 0 };
		if (isClicked || isDecided)
		{
			// 「戻る」を選んだときは決めた音にしない
			const bool isBackItem{ m_step == Step::Strength &&
				                   m_index >= static_cast<int>(STRENGTHS.size()) };
			m_context.resource.playSe(isBackItem ? m_backSound : m_decideSound);

			decide();
			return;
		}

		m_choices.update(deltaTime, buildContent());
	}

	void TitleScene::draw()
	{
		m_backdrop.draw();
	}

	void TitleScene::drawOverlay()
	{
		m_backdrop.drawOverlay();

		const float centerX{ m_context.screen.getWidth() * 0.5f };
		const float height{ static_cast<float>(m_context.screen.getHeight()) };

		m_context.renderer.setFont(m_titleFont);
		m_context.renderer.drawTextCentered(Vector2{ centerX, height * TITLE_Y }, "際",
		                                    palette::TEXT_PRIMARY);

		m_context.renderer.setFont(m_bodyFont);
		m_context.renderer.drawTextCentered(Vector2{ centerX, height * SUBTITLE_Y },
		                                    "-KIWA-", palette::TEXT_SUB);

		m_choices.draw(m_context.renderer, m_context.screen,
		               view::ChoiceList::Resources{ m_headingFont, m_bodyFont, font::HEADING_SIZE,
			                                        font::BODY_SIZE });
	}
} // namespace game::scene
