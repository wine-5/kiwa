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

	/// @brief 伏せた二枚を左右へ離す幅
	constexpr float SPREAD{ 132.0f };

	/// @brief 引いた札が中央へ寄る速さ
	constexpr float GATHER_RATE{ 4.2f };

	/// @brief 引かれなかった札が逃げていく距離の倍率
	constexpr float LEAVE_SPREAD{ 1.9f };

	/// @brief 指している札が持ち上がる高さ
	constexpr float HIGHLIGHT_RISE{ 16.0f };

	/// @brief 指している札が大きく見える割合
	constexpr float HIGHLIGHT_SCALE{ 1.05f };

	/// @brief 札を置く高さ（画面の高さに対する割合）
	constexpr float CARD_Y{ 0.46f };
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
			m_gather = 0.0f;
			m_hasFlipped = false;
			return;
		}

		m_appear = Easing::approach(m_appear, 1.0f, APPEAR_RATE, deltaTime);

		// 引いた札が中央へ寄り、寄り切ってから返る。
		// 寄る前に返し始めると、どちらを引いたのか分からなくなる
		if (content.picked >= 0)
			m_gather = Easing::approach(m_gather, 1.0f, GATHER_RATE, deltaTime);

		const bool canFlip{ content.isRevealed && m_gather > 0.72f };
		m_flip = Easing::approach(m_flip, canFlip ? 1.0f : 0.0f, FLIP_RATE, deltaTime);

		// 役の振り分けは、札が返り切ってから始める
		if (m_flip > 0.92f)
			m_announce = Easing::approach(m_announce, 1.0f, ANNOUNCE_RATE, deltaTime);
	}

	bool CardDraw::consumeFlipMoment()
	{
		// 裏と表が入れ替わるのは、潰れ切った半ば。そこで一度だけ知らせる
		if (m_hasFlipped || m_flip < 0.5f)
			return false;

		m_hasFlipped = true;
		return true;
	}

	int CardDraw::hitTest(core::iface::IScreen& screen, const core::utility::Vector2& position) const
	{
		// 引いたあとは押せない
		if (!m_content.isActive || m_content.picked >= 0)
			return -1;

		const float centerX{ screen.getWidth() * 0.5f };
		const float centerY{ screen.getHeight() * CARD_Y };

		for (int i{ 0 }; i < 2; ++i)
		{
			const float x{ centerX + (i == 0 ? -SPREAD : SPREAD) };

			if (std::abs(position.x - x) <= CARD_WIDTH * 0.5f &&
			    std::abs(position.y - centerY) <= CARD_HEIGHT * 0.5f)
				return i;
		}
		return -1;
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

		const float appear{ Easing::easeOut(m_appear) };
		const float announce{ Easing::easeOut(m_announce) };
		const float gather{ Easing::easeOut(m_gather) };

		// 下から現れ、役を告げたら上へ退く
		const float sway{ m_content.picked < 0 ? std::sin(m_time * 2.2f) * IDLE_SWAY : 0.0f };
		const float baseY{ height * CARD_Y + (1.0f - appear) * 180.0f - announce * ANNOUNCE_RISE +
			               sway };

		for (int i{ 0 }; i < 2; ++i)
		{
			const float side{ i == 0 ? -1.0f : 1.0f };
			const bool isPicked{ i == m_content.picked };
			const bool isWaiting{ m_content.picked < 0 };

			// 引いた札は中央へ寄り、引かれなかった札は外へ逃げて消える
			const float x{ isPicked ? width * 0.5f + side * SPREAD * (1.0f - gather)
			                        : width * 0.5f + side * SPREAD *
			                                             (1.0f + (LEAVE_SPREAD - 1.0f) * gather) };
			const float alpha{ isPicked || isWaiting ? appear : appear * (1.0f - gather) };
			if (alpha <= 0.01f)
				continue;

			// まだ引いていない間は、指している札を持ち上げて大きく見せる
			const bool isHighlighted{ isWaiting && i == m_content.highlighted };
			const float scale{ (isHighlighted ? HIGHLIGHT_SCALE : 1.0f) *
				               (1.0f - announce * ANNOUNCE_SHRINK) };
			const float y{ baseY - (isHighlighted ? HIGHLIGHT_RISE : 0.0f) };

			// 返る途中は横に潰れて見える。潰れ切ったところで裏と表が入れ替わる
			const float flip{ isPicked ? m_flip : 0.0f };
			const float flipAngle{ std::clamp(flip, 0.0f, 1.0f) * core::utility::math::PI };
			const float cardWidth{ CARD_WIDTH * std::abs(std::cos(flipAngle)) * appear * scale };
			const float cardHeight{ CARD_HEIGHT * appear * scale };

			const int faceTexture{ m_content.isFirstCard ? resources.firstTexture
				                                         : resources.secondTexture };
			const int texture{ isPicked && flip >= 0.5f ? faceTexture : resources.backTexture };

			renderer.drawTextureStretched(
			    texture, Vector2{ x - cardWidth * 0.5f, y - cardHeight * 0.5f },
			    Vector2{ cardWidth, cardHeight }, alpha);
		}

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
