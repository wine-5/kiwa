#include "game/view/CardDraw.h"
#include "core/interface/IRenderer.h"
#include "core/interface/IScreen.h"
#include "core/utility/Easing.h"
#include "core/utility/MathConstants.h"
#include "game/constant/Palette.h"
#include <algorithm>
#include <cmath>

namespace
{
	using core::utility::Easing;
	using core::utility::Vector2;
	namespace palette = game::constant::palette;

	/// @brief 札が現れる速さ
	constexpr float APPEAR_RATE{ 3.4f };

	/// @brief 札が返る速さ
	constexpr float FLIP_RATE{ 2.6f };

	/// @brief 役を振り分ける速さ
	constexpr float ANNOUNCE_RATE{ 2.8f };

	/// @brief 札の幅
	constexpr float CARD_WIDTH{ 210.0f };

	/// @brief 札の高さ
	constexpr float CARD_HEIGHT{ 315.0f };

	/// @brief 伏せている間の上下の揺れ幅
	constexpr float IDLE_SWAY{ 6.0f };

	/// @brief 役を振り分けたあと、札が退く高さ
	constexpr float ANNOUNCE_RISE{ 92.0f };

	/// @brief 役を振り分けたあとの札の縮み具合
	constexpr float ANNOUNCE_SHRINK{ 0.34f };

	/// @brief 役を出す左右の位置（画面幅に対する割合）
	constexpr float SIDE_X{ 0.23f };
} // namespace

namespace game::view
{
	void CardDraw::update(float deltaTime, const Content& content)
	{
		m_content = content;
		m_time += deltaTime;

		if (!content.isActive)
		{
			m_appear = 0.0f;
			m_flip = 0.0f;
			m_announce = 0.0f;
			return;
		}

		m_appear = Easing::approach(m_appear, 1.0f, APPEAR_RATE, deltaTime);

		const float flipTarget{ content.isRevealed ? 1.0f : 0.0f };
		m_flip = Easing::approach(m_flip, flipTarget, FLIP_RATE, deltaTime);

		// 役の振り分けは、札が返り切ってから始める
		if (m_flip > 0.92f)
			m_announce = Easing::approach(m_announce, 1.0f, ANNOUNCE_RATE, deltaTime);
	}

	bool CardDraw::isVisible() const noexcept
	{
		return m_content.isActive && m_appear > 0.01f;
	}

	void CardDraw::draw(core::iface::IRenderer& renderer, core::iface::IScreen& screen,
	                    const Resources& resources) const
	{
		if (!isVisible())
			return;

		const float width{ static_cast<float>(screen.getWidth()) };
		const float height{ static_cast<float>(screen.getHeight()) };

		// 下から現れ、役を告げたら上へ退く
		const float appear{ Easing::easeOut(m_appear) };
		const float announce{ Easing::easeOut(m_announce) };
		const float sway{ m_flip < 0.05f ? std::sin(m_time * 2.2f) * IDLE_SWAY : 0.0f };
		const float centerY{ height * 0.46f + (1.0f - appear) * 180.0f - announce * ANNOUNCE_RISE +
			                 sway };

		// 返る途中は横に潰れて見える。潰れ切ったところで裏と表が入れ替わる
		const float flipAngle{ std::clamp(m_flip, 0.0f, 1.0f) * core::utility::math::PI };
		const float shrink{ 1.0f - announce * ANNOUNCE_SHRINK };
		const float cardWidth{ CARD_WIDTH * std::abs(std::cos(flipAngle)) * appear * shrink };
		const float cardHeight{ CARD_HEIGHT * appear * shrink };

		const bool showsFace{ m_flip >= 0.5f };
		const int faceTexture{ m_content.isFirstCard ? resources.firstTexture
			                                         : resources.secondTexture };
		const int texture{ showsFace ? faceTexture : resources.backTexture };

		renderer.drawTextureStretched(texture,
		                              Vector2{ width * 0.5f - cardWidth * 0.5f,
			                                   centerY - cardHeight * 0.5f },
		                              Vector2{ cardWidth, cardHeight }, appear);

		if (m_announce <= 0.01f)
			return;

		// 左右へ役を振り分ける。外から寄ってくることで「割り当てられた」と分かる
		renderer.setFont(resources.headingFont);

		const float slide{ (1.0f - announce) * 90.0f };
		const float labelY{ height * 0.62f };

		renderer.drawTextCentered(Vector2{ width * SIDE_X - slide, labelY }, m_content.leftLabel,
		                          palette::TEXT_PRIMARY);
		renderer.drawTextCentered(Vector2{ width * (1.0f - SIDE_X) + slide, labelY },
		                          m_content.rightLabel, palette::TEXT_PRIMARY);

		renderer.setFont(resources.bodyFont);
	}
} // namespace game::view
