#include "game/scene/TitleScene.h"
#include "core/interface/IInputProvider.h"
#include "core/interface/IRenderer.h"
#include "core/interface/IResourceManager.h"
#include "core/interface/IScreen.h"
#include "game/constant/Fonts.h"
#include "game/constant/Palette.h"
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

	/// @brief 選ばせる並びを置く高さ（画面の高さに対する割合）
	constexpr float LIST_Y{ 0.58f };

	/// @brief 案内が1回点滅する周期（秒）
	constexpr float BLINK_CYCLE{ 1.6f };

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
	TitleScene::TitleScene(const SceneContext& context) : m_context{ context }
	{
		m_titleFont = m_context.resource.loadFont(font::HEADING_FAMILY, TITLE_FONT_SIZE);
		m_headingFont = m_context.resource.loadFont(font::HEADING_FAMILY, font::HEADING_SIZE);
		m_bodyFont = m_context.resource.loadFont(font::BODY_FAMILY, font::BODY_SIZE);
	}

	view::ChoiceList::Content TitleScene::buildContent() const
	{
		view::ChoiceList::Content content{};
		content.isActive = true;
		content.selected = m_index;
		content.centerY = LIST_Y;

		if (m_step == Step::Mode)
		{
			content.items = { "二人で打つ", "一人で打つ" };
			content.notes = { "向かい合って", "二の手は任せる" };
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
			if (m_index == 0)
			{
				m_context.setup.npc = model::NpcType::None;
				m_context.changeScene(SceneType::InGame);
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
		m_elapsedTime += deltaTime;

		const view::ChoiceList::Content content{ buildContent() };
		const auto count{ static_cast<int>(content.items.size()) };

		// マウスを乗せたものへ指を移す。押せることが動きで分かる
		const int hovered{ m_choices.hitTest(m_context.screen, m_context.input.getMousePosition()) };
		if (hovered >= 0)
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

		// 左で一つ前の段へ戻る（強さを選んでいる途中で選び直せるように）
		if (isBack && m_step == Step::Strength)
		{
			m_step = Step::Mode;
			m_index = 1;
			return;
		}

		const bool isClicked{ m_context.input.isMouseLeftPressed() && hovered >= 0 };
		if (isClicked || isDecided)
		{
			decide();
			return;
		}

		m_choices.update(deltaTime, buildContent());
	}

	void TitleScene::draw()
	{
	}

	void TitleScene::drawOverlay()
	{
		const float centerX{ m_context.screen.getWidth() * 0.5f };
		const float height{ static_cast<float>(m_context.screen.getHeight()) };

		m_context.renderer.setFont(m_titleFont);
		m_context.renderer.drawTextCentered(Vector2{ centerX, height * TITLE_Y }, "際",
		                                    palette::TEXT_PRIMARY);

		m_context.renderer.setFont(m_bodyFont);
		m_context.renderer.drawTextCentered(Vector2{ centerX, height * SUBTITLE_Y },
		                                    "-KIWA-　注ぎ勝負", palette::TEXT_SUB);

		m_choices.draw(m_context.renderer, m_context.screen,
		               view::ChoiceList::Resources{ m_headingFont, m_bodyFont });

		// 押せることを点滅で伝える
		if (std::fmod(m_elapsedTime, BLINK_CYCLE) < BLINK_CYCLE * 0.62f)
		{
			m_context.renderer.setFont(m_bodyFont);
			m_context.renderer.drawTextCentered(Vector2{ centerX, height - 96.0f },
			                                    "↑↓ で選び、Enter で決める　　クリックでも選べる",
			                                    palette::TEXT_SUB);
		}
	}
} // namespace game::scene
