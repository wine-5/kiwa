#include "game/view/TurnCall.h"
#include "core/interface/IRenderer.h"
#include "core/interface/IScreen.h"
#include "core/utility/Easing.h"
#include "game/constant/Palette.h"
#include <algorithm>

namespace
{
	using core::utility::Easing;
	using core::utility::Vector2;
	namespace palette = game::constant::palette;

	/// @brief 寄ってくるまでの時間（秒）
	constexpr float ENTER_TIME{ 0.26f };

	/// @brief 見せている時間（秒）
	constexpr float HOLD_TIME{ 0.62f };

	/// @brief 引いていく時間（秒）
	constexpr float LEAVE_TIME{ 0.34f };

	/// @brief 告知を置く高さ（画面の高さに対する割合）
	constexpr float CALL_Y{ 0.33f };

	/// @brief 寄ってくるときの横の助走（画面の幅に対する割合）
	constexpr float RUN_UP{ 0.22f };

	/// @brief 引くときに持ち上がる高さ
	constexpr float LEAVE_RISE{ 26.0f };

	/// @brief 名前の左右に引く墨の線の長さ
	constexpr float RULE_LENGTH{ 150.0f };

	/// @brief 墨の線の太さ
	constexpr float RULE_THICKNESS{ 2.0f };

	/// @brief 名前と線のあいだ
	constexpr float RULE_GAP{ 34.0f };

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
		return core::utility::mixed(palette::BACKGROUND, color,
		                            std::clamp(strength, 0.0f, 1.0f));
	}
} // namespace

namespace game::view
{
	void TurnCall::update(float deltaTime, const Content& content)
	{
		// 通し番号が変わったときだけ告げ直す。同じ番のあいだは何度渡されても動かない
		if (content.serial != m_content.serial)
			m_time = 0.0f;

		m_content = content;

		if (m_time >= 0.0f)
			m_time += deltaTime;
	}

	void TurnCall::draw(core::iface::IRenderer& renderer, core::iface::IScreen& screen,
	                    int fontHandle, int fontSize) const
	{
		constexpr float TOTAL{ ENTER_TIME + HOLD_TIME + LEAVE_TIME };
		if (m_time < 0.0f || m_time > TOTAL || m_content.name.empty())
			return;

		const float width{ static_cast<float>(screen.getWidth()) };
		const float height{ static_cast<float>(screen.getHeight()) };

		// 寄る→留まる→引く。留まっている間は動かさず、読む間を作る
		const float enter{ Easing::easeOut(m_time / ENTER_TIME) };
		const float leave{ Easing::easeIn(std::max(0.0f, m_time - ENTER_TIME - HOLD_TIME) /
		                                  LEAVE_TIME) };

		// 自分の側から滑り込ませる。どちら側の手番かが動きだけで分かる
		const float runUp{ width * RUN_UP * (m_content.isFromLeft ? -1.0f : 1.0f) };
		const float x{ width * 0.5f + runUp * (1.0f - enter) };
		const float y{ height * CALL_Y - LEAVE_RISE * leave };

		const float strength{ enter * (1.0f - leave) };

		// 名前の左右へ墨の線を引く。線が伸びきると名前が据わって見える
		const float ruleLength{ RULE_LENGTH * enter * (1.0f - leave) };
		const core::utility::Color ruleColor{ faded(palette::TEXT_SUB, strength * 0.9f) };

		// 線が名前に重ならないよう、字数から見当をつけて外へ逃がす
		const float gap{ estimateWidth(m_content.name, fontSize) * 0.5f + RULE_GAP };

		for (int side{ 0 }; side < 2; ++side)
		{
			const float direction{ side == 0 ? -1.0f : 1.0f };
			const float from{ x + direction * gap };

			renderer.drawRect(Vector2{ std::min(from, from + direction * ruleLength),
				                       y - RULE_THICKNESS * 0.5f },
			                  Vector2{ ruleLength, RULE_THICKNESS }, ruleColor);
		}

		renderer.setFont(fontHandle);
		renderer.drawTextCentered(Vector2{ x, y }, m_content.name,
		                          faded(palette::TEXT_PRIMARY, strength));
	}
} // namespace game::view
