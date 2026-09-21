#include "game/view/ChoiceList.h"
#include "core/interface/IRenderer.h"
#include "core/interface/IScreen.h"
#include "core/utility/Easing.h"
#include "game/constant/Palette.h"
#include <algorithm>
#include <cmath>
#include <string>

namespace
{
	using core::utility::Easing;
	using core::utility::Vector2;
	namespace palette = game::constant::palette;

	/// @brief 出てくる速さ
	constexpr float APPEAR_RATE{ 5.0f };

	/// @brief 印が選ばれたものへ移る速さ
	constexpr float MARKER_RATE{ 14.0f };

	/// @brief 行の間
	constexpr float LINE_STEP{ 54.0f };

	/// @brief 出てくるときの下からの寄り
	constexpr float RISE{ 34.0f };

	/// @brief 選ばれている行に引く墨の線の長さ
	constexpr float RULE_LENGTH{ 68.0f };

	/// @brief 線と字のあいだ
	constexpr float RULE_GAP{ 34.0f };

	/// @brief 線と添え書きのあいだ
	constexpr float NOTE_GAP{ 24.0f };

	/// @brief 線の太さ
	constexpr float RULE_THICKNESS{ 2.0f };

	/// @brief 押せるところの横幅（行の中心からの左右）
	constexpr float HIT_WIDTH{ 260.0f };

	/**
	 * @brief 文字列の見た目の幅を見積もる
	 *
	 * 書体の幅を測る手だてが無いので、字数から見当をつける。全角はほぼ
	 * 文字の大きさぶん、半角はその半分として数える
	 * @param text 文字列（UTF-8）
	 * @param fontSize 文字の大きさ
	 * @return 幅の見積もり
	 */
	float estimateWidth(const std::string& text, int fontSize)
	{
		float count{ 0.0f };
		for (const char character : text)
		{
			const auto byte{ static_cast<unsigned char>(character) };

			// UTF-8 の続きのバイト（10xxxxxx）は数えない
			if ((byte & 0xC0) == 0x80)
				continue;

			count += byte < 0x80 ? 0.5f : 1.0f;
		}
		return count * static_cast<float>(fontSize);
	}

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

namespace game::view
{
	float ChoiceList::selectedGap(const Resources& resources) const
	{
		// 選ばれている字は毛筆で大きい。その幅を跨いだところから罫を引く
		const int index{ std::clamp(m_content.selected, 0,
			                        static_cast<int>(m_content.items.size()) - 1) };
		return estimateWidth(m_content.items[index], resources.headingSize) * 0.5f + RULE_GAP;
	}

	int ChoiceList::hitTest(core::iface::IScreen& screen,
	                        const core::utility::Vector2& position) const
	{
		if (!m_content.isActive || m_content.items.empty())
			return -1;

		const float centerX{ screen.getWidth() * 0.5f };
		const auto count{ static_cast<int>(m_content.items.size()) };
		const float top{ screen.getHeight() * m_content.centerY - LINE_STEP * (count - 1) * 0.5f };

		if (std::abs(position.x - centerX) > HIT_WIDTH)
			return -1;

		for (int i{ 0 }; i < count; ++i)
		{
			// 行と行のあいだに隙間を作らない。取りこぼすと押しても反応しないように見える
			if (std::abs(position.y - (top + LINE_STEP * i)) <= LINE_STEP * 0.5f)
				return i;
		}
		return -1;
	}

	void ChoiceList::update(float deltaTime, const Content& content)
	{
		m_content = content;
		m_time += deltaTime;

		if (!content.isActive)
		{
			m_appear = 0.0f;
			m_marker = static_cast<float>(content.selected);
			return;
		}

		m_appear = Easing::approach(m_appear, 1.0f, APPEAR_RATE, deltaTime);

		// 印は選ばれたものへ滑って移る。飛ぶと、どこから来たのか分からなくなる
		m_marker = Easing::approach(m_marker, static_cast<float>(content.selected), MARKER_RATE,
		                            deltaTime);
	}

	void ChoiceList::draw(core::iface::IRenderer& renderer, core::iface::IScreen& screen,
	                      const Resources& resources) const
	{
		if (!m_content.isActive || m_content.items.empty() || m_appear <= 0.01f)
			return;

		const float centerX{ screen.getWidth() * 0.5f };
		const float height{ static_cast<float>(screen.getHeight()) };
		const auto count{ static_cast<int>(m_content.items.size()) };

		const float appear{ Easing::easeOut(m_appear) };
		const float top{ height * m_content.centerY - LINE_STEP * (count - 1) * 0.5f +
			             RISE * (1.0f - appear) };

		for (int i{ 0 }; i < count; ++i)
		{
			const bool isSelected{ i == m_content.selected };
			const float y{ top + LINE_STEP * i };

			// 選ばれているものだけ毛筆で大きく、そのほかは沈めて置く
			renderer.setFont(isSelected ? resources.headingFont : resources.bodyFont);
			renderer.drawTextCentered(Vector2{ centerX, y }, m_content.items[i],
			                          faded(isSelected ? palette::TEXT_PRIMARY : palette::TEXT_SUB,
			                                appear * (isSelected ? 1.0f : 0.45f)));

			if (!isSelected || i >= static_cast<int>(m_content.notes.size()))
				continue;

			// 添え書きは罫のさらに外側へ。字幅に合わせて逃がさないと罫と重なる
			const float noteLeft{ centerX + selectedGap(resources) + RULE_LENGTH + NOTE_GAP };

			renderer.setFont(resources.bodyFont);
			renderer.drawText(Vector2{ noteLeft, y - resources.bodySize * 0.5f },
			                  m_content.notes[i], faded(palette::TEXT_SUB, appear * 0.8f));
		}

		// 印は行から行へ滑る。明滅させて、いま動かせるものだと分かるようにする
		const float markerY{ top + LINE_STEP * m_marker };
		const float pulse{ 0.7f + 0.3f * std::sin(m_time * 4.0f) };
		const core::utility::Color ruleColor{ faded(palette::TEXT_SUB, appear * pulse) };

		const float gap{ selectedGap(resources) };

		for (int side{ 0 }; side < 2; ++side)
		{
			const float direction{ side == 0 ? -1.0f : 1.0f };
			const float from{ centerX + direction * gap };

			renderer.drawRect(Vector2{ std::min(from, from + direction * RULE_LENGTH),
				                       markerY - RULE_THICKNESS * 0.5f },
			                  Vector2{ RULE_LENGTH, RULE_THICKNESS }, ruleColor);
		}
	}
} // namespace game::view
